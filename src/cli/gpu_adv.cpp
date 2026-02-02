/*
 * Copyright (c) 2026 Noverse (Nohuto). All rights reserved.
 * Proprietary and confidential. Unauthorized copying or redistribution is strictly prohibited.
 */

#include "cli/commands.h"

#include <memory>
#include <string>

namespace nvcli {
struct RawApiArgs {
  NvU32 index = 0;
  bool hasIndex = false;
  bool raw = false;
  const char *inPath = nullptr;
  const char *outPath = nullptr;
  bool hasDomain = false;
  NvU32 domain = 0;
  bool hasClassType = false;
  NvU32 classType = 0;
};

struct ApiSpec {
  const char *name;
  const char *desc;
  size_t size;
  NvU32 version;
  bool isSet;
  NvAPI_Status (*call)(NvPhysicalGpuHandle, void *);
  void (*prepare)(void *, const RawApiArgs &);
};

static bool ReadBinaryFile(const char *path, void *data, size_t size) {
  if (!path || !data || size == 0) { return false; }
  FILE *file = nullptr;
  if (fopen_s(&file, path, "rb") != 0 || !file) {
    std::printf("Failed to open input file: %s\n", path);
    return false;
  }
  if (fseek(file, 0, SEEK_END) != 0) {
    std::printf("Failed to seek input file: %s\n", path);
    std::fclose(file);
    return false;
  }
  long length = ftell(file);
  if (length < 0) {
    std::printf("Failed to read input file size: %s\n", path);
    std::fclose(file);
    return false;
  }
  if (static_cast<size_t>(length) != size) {
    std::printf("Input file size mismatch: %s (expected %zu bytes, got %ld)\n", path, size, length);
    std::fclose(file);
    return false;
  }
  std::rewind(file);
  size_t read = std::fread(data, 1, size, file);
  std::fclose(file);
  if (read != size) {
    std::printf("Failed to read input file: %s\n", path);
    return false;
  }
  return true;
}

static bool WriteBinaryFile(const char *path, const void *data, size_t size) {
  if (!path || !data || size == 0) { return false; }
  FILE *file = nullptr;
  if (fopen_s(&file, path, "wb") != 0 || !file) {
    std::printf("Failed to open output file: %s\n", path);
    return false;
  }
  size_t written = std::fwrite(data, 1, size, file);
  std::fclose(file);
  if (written != size) {
    std::printf("Failed to write output file: %s\n", path);
    return false;
  }
  return true;
}

static std::string AppendGpuIndexToPath(const char *path, NvU32 index) {
  std::string value(path);
  size_t slash = value.find_last_of("/\\");
  size_t dot = value.find_last_of('.');
  bool hasExt = dot != std::string::npos && (slash == std::string::npos || dot > slash);
  if (!hasExt) { return value + "_gpu" + std::to_string(index); }
  return value.substr(0, dot) + "_gpu" + std::to_string(index) + value.substr(dot);
}

static void DumpHex(const void *data, size_t size) {
  const unsigned char *bytes = static_cast<const unsigned char *>(data);
  for (size_t i = 0; i < size; ++i) {
    if (i % 16 == 0) { std::printf("    "); }
    std::printf("%02X ", bytes[i]);
    if (i % 16 == 15 || i + 1 == size) { std::printf("\n"); }
  }
}

static bool ParseRawApiArgs(int argc, char **argv, RawApiArgs *out) {
  if (!out) { return false; }
  for (int i = 0; i < argc; ++i) {
    if (std::strcmp(argv[i], "--index") == 0) {
      if (i + 1 >= argc) {
        std::printf("Missing value for --index\n");
        return false;
      }
      if (!ParseUint(argv[i + 1], &out->index)) {
        std::printf("Invalid GPU index: %s\n", argv[i + 1]);
        return false;
      }
      out->hasIndex = true;
      ++i;
      continue;
    }
    if (std::strcmp(argv[i], "--raw") == 0) {
      out->raw = true;
      continue;
    }
    if (std::strcmp(argv[i], "--in") == 0) {
      if (i + 1 >= argc) {
        std::printf("Missing value for --in\n");
        return false;
      }
      out->inPath = argv[i + 1];
      ++i;
      continue;
    }
    if (std::strcmp(argv[i], "--out") == 0) {
      if (i + 1 >= argc) {
        std::printf("Missing value for --out\n");
        return false;
      }
      out->outPath = argv[i + 1];
      ++i;
      continue;
    }
    if (std::strcmp(argv[i], "--domain") == 0) {
      if (i + 1 >= argc) {
        std::printf("Missing value for --domain\n");
        return false;
      }
      if (!ParseUint(argv[i + 1], &out->domain)) {
        std::printf("Invalid domain value: %s\n", argv[i + 1]);
        return false;
      }
      out->hasDomain = true;
      ++i;
      continue;
    }
    if (std::strcmp(argv[i], "--class") == 0) {
      if (i + 1 >= argc) {
        std::printf("Missing value for --class\n");
        return false;
      }
      if (!ParseUint(argv[i + 1], &out->classType)) {
        std::printf("Invalid class value: %s\n", argv[i + 1]);
        return false;
      }
      out->hasClassType = true;
      ++i;
      continue;
    }
    std::printf("Unknown option: %s\n", argv[i]);
    return false;
  }
  return true;
}

static void PrepareClockCounterAvgFreq(void *data, const RawApiArgs &args) {
  if (!args.hasDomain) { return; }
  auto *params = reinterpret_cast<NV_GPU_CLOCK_COUNTER_MEASURE_AVG_FREQ_PARAMS *>(data);
  params->clkDomain = static_cast<NV_GPU_CLOCK_DOMAIN_ID>(args.domain);
}

static void PrepareClockDomainFreqsEnum(void *data, const RawApiArgs &args) {
  if (!args.hasDomain) { return; }
  auto *params = reinterpret_cast<NV_GPU_CLOCK_CLK_DOMAIN_FREQS_ENUM *>(data);
  params->clkDomainIdx = static_cast<NvU8>(args.domain);
}

static void PrepareClockDomainRpc(void *data, const RawApiArgs &args) {
  if (!args.hasDomain && !args.hasClassType) { return; }
  auto *params = reinterpret_cast<NV_GPU_CLOCK_CLK_DOMAIN_RPC *>(data);
  if (args.hasDomain) { params->clkDomainIdx = args.domain; }
  if (args.hasClassType) { params->classType = static_cast<NvU8>(args.classType); }
}

static void PrepareClockDomainsFreqInfo(void *data, const RawApiArgs &args) {
  if (args.inPath) { return; }
  auto *params = reinterpret_cast<NV_GPU_CLOCK_CLK_DOMAINS_FREQ_INFO *>(data);
  params->numFreqInfos = NV_GPU_CLOCK_CLK_DOMAINS_MAX_V1;
}

static NvAPI_Status CallClockCounterMeasureAvgFreq(NvPhysicalGpuHandle gpu, void *data) {
  return NvAPI_GPU_ClockCounterMeasureAvgFreq(gpu,
                                              reinterpret_cast<NV_GPU_CLOCK_COUNTER_MEASURE_AVG_FREQ_PARAMS *>(data));
}

static NvAPI_Status CallClockClkDomainsGetControl(NvPhysicalGpuHandle gpu, void *data) {
  return NvAPI_GPU_ClockClkDomainsGetControl(gpu, reinterpret_cast<NV_GPU_CLOCK_CLK_DOMAINS_CONTROL *>(data));
}

static NvAPI_Status CallClockClkDomainsSetControl(NvPhysicalGpuHandle gpu, void *data) {
  return NvAPI_GPU_ClockClkDomainsSetControl(gpu, reinterpret_cast<NV_GPU_CLOCK_CLK_DOMAINS_CONTROL *>(data));
}

static NvAPI_Status CallClockClkDomainsGetInfo(NvPhysicalGpuHandle gpu, void *data) {
  return NvAPI_GPU_ClockClkDomainsGetInfo(gpu, reinterpret_cast<NV_GPU_CLOCK_CLK_DOMAINS_INFO *>(data));
}

static NvAPI_Status CallClockClkDomainsGetFreqInfo(NvPhysicalGpuHandle gpu, void *data) {
  return NvAPI_GPU_ClockClkDomainsGetFreqInfo(gpu, reinterpret_cast<NV_GPU_CLOCK_CLK_DOMAINS_FREQ_INFO *>(data));
}

static NvAPI_Status CallClockClkDomainFreqsEnum(NvPhysicalGpuHandle gpu, void *data) {
  return NvAPI_GPU_ClockClkDomainFreqsEnum(gpu, reinterpret_cast<NV_GPU_CLOCK_CLK_DOMAIN_FREQS_ENUM *>(data));
}

static NvAPI_Status CallClockClkDomainRpc(NvPhysicalGpuHandle gpu, void *data) {
  return NvAPI_GPU_ClockClkDomainRpc(gpu, reinterpret_cast<NV_GPU_CLOCK_CLK_DOMAIN_RPC *>(data));
}

static NvAPI_Status CallClockClkProgsGetInfo(NvPhysicalGpuHandle gpu, void *data) {
  return NvAPI_GPU_ClockClkProgsGetInfo(gpu, reinterpret_cast<NV_GPU_CLOCK_CLK_PROGS_INFO *>(data));
}

static NvAPI_Status CallClockClkProgsGetStatus(NvPhysicalGpuHandle gpu, void *data) {
  return NvAPI_GPU_ClockClkProgsGetStatus(gpu, reinterpret_cast<NV_GPU_CLOCK_CLK_PROGS_STATUS *>(data));
}

static NvAPI_Status CallClockClkProgsGetControl(NvPhysicalGpuHandle gpu, void *data) {
  return NvAPI_GPU_ClockClkProgsGetControl(gpu, reinterpret_cast<NV_GPU_CLOCK_CLK_PROGS_CONTROL *>(data));
}

static NvAPI_Status CallClockClkProgsSetControl(NvPhysicalGpuHandle gpu, void *data) {
  return NvAPI_GPU_ClockClkProgsSetControl(gpu, reinterpret_cast<NV_GPU_CLOCK_CLK_PROGS_CONTROL *>(data));
}

static NvAPI_Status CallClockClkEnumsGetInfo(NvPhysicalGpuHandle gpu, void *data) {
  return NvAPI_GPU_ClockClkEnumsGetInfo(gpu, reinterpret_cast<NV_GPU_CLOCK_CLK_ENUMS_INFO *>(data));
}

static NvAPI_Status CallClockClkVfRelsGetInfo(NvPhysicalGpuHandle gpu, void *data) {
  return NvAPI_GPU_ClockClkVfRelsGetInfo(gpu, reinterpret_cast<NV_GPU_CLOCK_CLK_VF_RELS_INFO *>(data));
}

static NvAPI_Status CallClockClkVfRelsGetStatus(NvPhysicalGpuHandle gpu, void *data) {
  return NvAPI_GPU_ClockClkVfRelsGetStatus(gpu, reinterpret_cast<NV_GPU_CLOCK_CLK_VF_RELS_STATUS *>(data));
}

static NvAPI_Status CallClockClkVfRelsGetControl(NvPhysicalGpuHandle gpu, void *data) {
  return NvAPI_GPU_ClockClkVfRelsGetControl(gpu, reinterpret_cast<NV_GPU_CLOCK_CLK_VF_RELS_CONTROL *>(data));
}

static NvAPI_Status CallClockClkVfRelsSetControl(NvPhysicalGpuHandle gpu, void *data) {
  return NvAPI_GPU_ClockClkVfRelsSetControl(gpu, reinterpret_cast<NV_GPU_CLOCK_CLK_VF_RELS_CONTROL *>(data));
}

static NvAPI_Status CallClockClkVfPointsGetInfo(NvPhysicalGpuHandle gpu, void *data) {
  return NvAPI_GPU_ClockClkVfPointsGetInfo(gpu, reinterpret_cast<NV_GPU_CLOCK_CLK_VF_POINTS_INFO *>(data));
}

static NvAPI_Status CallClockClkVfPointsGetStatus(NvPhysicalGpuHandle gpu, void *data) {
  return NvAPI_GPU_ClockClkVfPointsGetStatus(gpu, reinterpret_cast<NV_GPU_CLOCK_CLK_VF_POINTS_STATUS *>(data));
}

static NvAPI_Status CallClockClkVfPointsGetControl(NvPhysicalGpuHandle gpu, void *data) {
  return NvAPI_GPU_ClockClkVfPointsGetControl(gpu, reinterpret_cast<NV_GPU_CLOCK_CLK_VF_POINTS_CONTROL *>(data));
}

static NvAPI_Status CallClockClkVfPointsSetControl(NvPhysicalGpuHandle gpu, void *data) {
  return NvAPI_GPU_ClockClkVfPointsSetControl(gpu, reinterpret_cast<NV_GPU_CLOCK_CLK_VF_POINTS_CONTROL *>(data));
}

static NvAPI_Status CallClockClkPropRegimesGetInfo(NvPhysicalGpuHandle gpu, void *data) {
  return NvAPI_GPU_ClockClkPropRegimesGetInfo(gpu, reinterpret_cast<NV_GPU_CLOCK_CLK_PROP_REGIMES_INFO *>(data));
}

static NvAPI_Status CallClockClkPropRegimesGetControl(NvPhysicalGpuHandle gpu, void *data) {
  return NvAPI_GPU_ClockClkPropRegimesGetControl(gpu, reinterpret_cast<NV_GPU_CLOCK_CLK_PROP_REGIMES_CONTROL *>(data));
}

static NvAPI_Status CallClockClkPropRegimesSetControl(NvPhysicalGpuHandle gpu, void *data) {
  return NvAPI_GPU_ClockClkPropRegimesSetControl(gpu, reinterpret_cast<NV_GPU_CLOCK_CLK_PROP_REGIMES_CONTROL *>(data));
}

static NvAPI_Status CallClockClkPropTopsGetInfo(NvPhysicalGpuHandle gpu, void *data) {
  return NvAPI_GPU_ClockClkPropTopsGetInfo(gpu, reinterpret_cast<NV_GPU_CLOCK_CLK_PROP_TOPS_INFO *>(data));
}

static NvAPI_Status CallClockClkPropTopsGetStatus(NvPhysicalGpuHandle gpu, void *data) {
  return NvAPI_GPU_ClockClkPropTopsGetStatus(gpu, reinterpret_cast<NV_GPU_CLOCK_CLK_PROP_TOPS_STATUS *>(data));
}

static NvAPI_Status CallClockClkPropTopsGetControl(NvPhysicalGpuHandle gpu, void *data) {
  return NvAPI_GPU_ClockClkPropTopsGetControl(gpu, reinterpret_cast<NV_GPU_CLOCK_CLK_PROP_TOPS_CONTROL *>(data));
}

static NvAPI_Status CallClockClkPropTopsSetControl(NvPhysicalGpuHandle gpu, void *data) {
  return NvAPI_GPU_ClockClkPropTopsSetControl(gpu, reinterpret_cast<NV_GPU_CLOCK_CLK_PROP_TOPS_CONTROL *>(data));
}

static NvAPI_Status CallClockClkPropTopRelsGetInfo(NvPhysicalGpuHandle gpu, void *data) {
  return NvAPI_GPU_ClockClkPropTopRelsGetInfo(gpu, reinterpret_cast<NV_GPU_CLOCK_CLK_PROP_TOP_RELS_INFO *>(data));
}

static NvAPI_Status CallClockClkPropTopRelsGetControl(NvPhysicalGpuHandle gpu, void *data) {
  return NvAPI_GPU_ClockClkPropTopRelsGetControl(gpu, reinterpret_cast<NV_GPU_CLOCK_CLK_PROP_TOP_RELS_CONTROL *>(data));
}

static NvAPI_Status CallClockClkPropTopRelsSetControl(NvPhysicalGpuHandle gpu, void *data) {
  return NvAPI_GPU_ClockClkPropTopRelsSetControl(gpu, reinterpret_cast<NV_GPU_CLOCK_CLK_PROP_TOP_RELS_CONTROL *>(data));
}

static NvAPI_Status CallClockAdcDevicesGetInfo(NvPhysicalGpuHandle gpu, void *data) {
  return NvAPI_GPU_ClockAdcDevicesGetInfo(gpu, reinterpret_cast<NV_GPU_CLOCK_ADC_DEVICES_INFO *>(data));
}

static NvAPI_Status CallClockAdcDevicesGetStatus(NvPhysicalGpuHandle gpu, void *data) {
  return NvAPI_GPU_ClockAdcDevicesGetStatus(gpu, reinterpret_cast<NV_GPU_CLOCK_ADC_DEVICES_STATUS *>(data));
}

static NvAPI_Status CallClockAdcDevicesGetControl(NvPhysicalGpuHandle gpu, void *data) {
  return NvAPI_GPU_ClockAdcDevicesGetControl(gpu, reinterpret_cast<NV_GPU_CLOCK_ADC_DEVICES_CONTROL *>(data));
}

static NvAPI_Status CallClockAdcDevicesSetControl(NvPhysicalGpuHandle gpu, void *data) {
  return NvAPI_GPU_ClockAdcDevicesSetControl(gpu, reinterpret_cast<NV_GPU_CLOCK_ADC_DEVICES_CONTROL *>(data));
}

static NvAPI_Status CallClockNafllDevicesGetInfo(NvPhysicalGpuHandle gpu, void *data) {
  return NvAPI_GPU_ClockNafllDevicesGetInfo(gpu, reinterpret_cast<NV_GPU_CLOCK_NAFLL_DEVICES_INFO *>(data));
}

static NvAPI_Status CallClockNafllDevicesGetStatus(NvPhysicalGpuHandle gpu, void *data) {
  return NvAPI_GPU_ClockNafllDevicesGetStatus(gpu, reinterpret_cast<NV_GPU_CLOCK_NAFLL_DEVICES_STATUS *>(data));
}

static NvAPI_Status CallClockNafllDevicesGetControl(NvPhysicalGpuHandle gpu, void *data) {
  return NvAPI_GPU_ClockNafllDevicesGetControl(gpu, reinterpret_cast<NV_GPU_CLOCK_NAFLL_DEVICES_CONTROL *>(data));
}

static NvAPI_Status CallClockNafllDevicesSetControl(NvPhysicalGpuHandle gpu, void *data) {
  return NvAPI_GPU_ClockNafllDevicesSetControl(gpu, reinterpret_cast<NV_GPU_CLOCK_NAFLL_DEVICES_CONTROL *>(data));
}

static NvAPI_Status CallClockClkFreqControllersGetInfo(NvPhysicalGpuHandle gpu, void *data) {
  return NvAPI_GPU_ClockClkFreqControllerGetInfo(gpu, reinterpret_cast<NV_GPU_CLOCK_CLK_FREQ_CONTROLLERS_INFO *>(data));
}

static NvAPI_Status CallClockClkFreqControllersGetStatus(NvPhysicalGpuHandle gpu, void *data) {
  return NvAPI_GPU_ClockClkFreqControllerGetStatus(gpu,
                                                   reinterpret_cast<NV_GPU_CLOCK_CLK_FREQ_CONTROLLERS_STATUS *>(data));
}

static NvAPI_Status CallClockClkFreqControllersGetControl(NvPhysicalGpuHandle gpu, void *data) {
  return NvAPI_GPU_ClockClkFreqControllersGetControl(
      gpu, reinterpret_cast<NV_GPU_CLOCK_CLK_FREQ_CONTROLLERS_CONTROL *>(data));
}

static NvAPI_Status CallClockClkFreqControllersSetControl(NvPhysicalGpuHandle gpu, void *data) {
  return NvAPI_GPU_ClockClkFreqControllersSetControl(
      gpu, reinterpret_cast<NV_GPU_CLOCK_CLK_FREQ_CONTROLLERS_CONTROL *>(data));
}

static NvAPI_Status CallClockClkVoltControllersGetInfo(NvPhysicalGpuHandle gpu, void *data) {
  return NvAPI_GPU_ClockClkVoltControllerGetInfo(gpu, reinterpret_cast<NV_GPU_CLOCK_CLK_VOLT_CONTROLLERS_INFO *>(data));
}

static NvAPI_Status CallClockClkVoltControllersGetStatus(NvPhysicalGpuHandle gpu, void *data) {
  return NvAPI_GPU_ClockClkVoltControllerGetStatus(gpu,
                                                   reinterpret_cast<NV_GPU_CLOCK_CLK_VOLT_CONTROLLERS_STATUS *>(data));
}

static NvAPI_Status CallClockClkVoltControllersGetControl(NvPhysicalGpuHandle gpu, void *data) {
  return NvAPI_GPU_ClockClkVoltControllersGetControl(
      gpu, reinterpret_cast<NV_GPU_CLOCK_CLK_VOLT_CONTROLLERS_CONTROL *>(data));
}

static NvAPI_Status CallClockClkVoltControllersSetControl(NvPhysicalGpuHandle gpu, void *data) {
  return NvAPI_GPU_ClockClkVoltControllersSetControl(
      gpu, reinterpret_cast<NV_GPU_CLOCK_CLK_VOLT_CONTROLLERS_CONTROL *>(data));
}

static NvAPI_Status CallClockClientClkDomainsGetInfo(NvPhysicalGpuHandle gpu, void *data) {
  return NvAPI_GPU_ClockClientClkDomainsGetInfo(gpu, reinterpret_cast<PNV_GPU_CLOCK_CLIENT_CLK_DOMAINS_INFO>(data));
}

static NvAPI_Status CallClockClientClkVfPointsGetInfo(NvPhysicalGpuHandle gpu, void *data) {
  return NvAPI_GPU_ClockClientClkVfPointsGetInfo(gpu, reinterpret_cast<PNV_GPU_CLOCK_CLIENT_CLK_VF_POINTS_INFO>(data));
}

static NvAPI_Status CallClockClientClkVfPointsGetStatus(NvPhysicalGpuHandle gpu, void *data) {
  return NvAPI_GPU_ClockClientClkVfPointsGetStatus(gpu,
                                                   reinterpret_cast<PNV_GPU_CLOCK_CLIENT_CLK_VF_POINTS_STATUS>(data));
}

static NvAPI_Status CallClockClientClkVfPointsGetControl(NvPhysicalGpuHandle gpu, void *data) {
  return NvAPI_GPU_ClockClientClkVfPointsGetControl(gpu,
                                                    reinterpret_cast<PNV_GPU_CLOCK_CLIENT_CLK_VF_POINTS_CONTROL>(data));
}

static NvAPI_Status CallClockClientClkVfPointsSetControl(NvPhysicalGpuHandle gpu, void *data) {
  return NvAPI_GPU_ClockClientClkVfPointsSetControl(gpu,
                                                    reinterpret_cast<PNV_GPU_CLOCK_CLIENT_CLK_VF_POINTS_CONTROL>(data));
}

static NvAPI_Status CallClockPmumonClkDomainsGetSamples(NvPhysicalGpuHandle gpu, void *data) {
  return NvAPI_GPU_ClockPmumonClkDomainsGetSamples(
      gpu, reinterpret_cast<NV_GPU_CLOCK_PMUMON_CLK_DOMAINS_GET_SAMPLES *>(data));
}

static NvAPI_Status CallPowerPolicyGetInfo(NvPhysicalGpuHandle gpu, void *data) {
  return NvAPI_GPU_PowerPolicyGetInfo(gpu, reinterpret_cast<NV_GPU_POWER_POLICY_INFO_PARAMS *>(data));
}

static NvAPI_Status CallPowerPolicyGetStatus(NvPhysicalGpuHandle gpu, void *data) {
  return NvAPI_GPU_PowerPolicyGetStatus(gpu, reinterpret_cast<NV_GPU_POWER_POLICY_STATUS_PARAMS *>(data));
}

static NvAPI_Status CallPowerPolicyGetControl(NvPhysicalGpuHandle gpu, void *data) {
  return NvAPI_GPU_PowerPolicyGetControl(gpu, reinterpret_cast<NV_GPU_POWER_POLICY_CONTROL_PARAMS *>(data));
}

static NvAPI_Status CallPowerPolicySetControl(NvPhysicalGpuHandle gpu, void *data) {
  return NvAPI_GPU_PowerPolicySetControl(gpu, reinterpret_cast<NV_GPU_POWER_POLICY_CONTROL_PARAMS *>(data));
}

static NvAPI_Status CallClientPowerTopologyGetInfo(NvPhysicalGpuHandle gpu, void *data) {
  return NvAPI_GPU_ClientPowerTopologyGetInfo(gpu, reinterpret_cast<NV_GPU_CLIENT_POWER_TOPOLOGY_INFO *>(data));
}

static NvAPI_Status CallClientPowerTopologyGetStatus(NvPhysicalGpuHandle gpu, void *data) {
  return NvAPI_GPU_ClientPowerTopologyGetStatus(gpu, reinterpret_cast<NV_GPU_CLIENT_POWER_TOPOLOGY_STATUS *>(data));
}

static NvAPI_Status CallClientPowerPoliciesGetInfo(NvPhysicalGpuHandle gpu, void *data) {
  return NvAPI_GPU_ClientPowerPoliciesGetInfo(gpu, reinterpret_cast<NV_GPU_CLIENT_POWER_POLICIES_INFO *>(data));
}

static NvAPI_Status CallClientPowerPoliciesGetStatus(NvPhysicalGpuHandle gpu, void *data) {
  return NvAPI_GPU_ClientPowerPoliciesGetStatus(gpu, reinterpret_cast<NV_GPU_CLIENT_POWER_POLICIES_STATUS *>(data));
}

static NvAPI_Status CallClientPowerPoliciesSetStatus(NvPhysicalGpuHandle gpu, void *data) {
  return NvAPI_GPU_ClientPowerPoliciesSetStatus(gpu, reinterpret_cast<NV_GPU_CLIENT_POWER_POLICIES_STATUS *>(data));
}

static NvAPI_Status CallClientThermalPoliciesGetInfo(NvPhysicalGpuHandle gpu, void *data) {
  return NvAPI_GPU_ClientThermalPoliciesGetInfo(gpu, reinterpret_cast<NV_GPU_CLIENT_THERMAL_POLICIES_INFO *>(data));
}

static NvAPI_Status CallClientThermalPoliciesGetStatus(NvPhysicalGpuHandle gpu, void *data) {
  return NvAPI_GPU_ClientThermalPoliciesGetStatus(gpu, reinterpret_cast<NV_GPU_CLIENT_THERMAL_POLICIES_STATUS *>(data));
}

static NvAPI_Status CallClientThermalPoliciesSetStatus(NvPhysicalGpuHandle gpu, void *data) {
  return NvAPI_GPU_ClientThermalPoliciesSetStatus(gpu, reinterpret_cast<NV_GPU_CLIENT_THERMAL_POLICIES_STATUS *>(data));
}

static NvAPI_Status CallFanArbiterGetInfo(NvPhysicalGpuHandle gpu, void *data) {
  return NvAPI_GPU_FanArbiterGetInfo(gpu, reinterpret_cast<NV_GPU_FAN_ARBITER_INFO_PARAMS *>(data));
}

static NvAPI_Status CallFanArbiterGetStatus(NvPhysicalGpuHandle gpu, void *data) {
  return NvAPI_GPU_FanArbiterGetStatus(gpu, reinterpret_cast<NV_GPU_FAN_ARBITER_STATUS_PARAMS *>(data));
}

static NvAPI_Status CallFanCoolerGetInfo(NvPhysicalGpuHandle gpu, void *data) {
  return NvAPI_GPU_FanCoolerGetInfo(gpu, reinterpret_cast<NV_GPU_FAN_COOLER_INFO_PARAMS *>(data));
}

static NvAPI_Status CallFanCoolerGetStatus(NvPhysicalGpuHandle gpu, void *data) {
  return NvAPI_GPU_FanCoolerGetStatus(gpu, reinterpret_cast<NV_GPU_FAN_COOLER_STATUS_PARAMS *>(data));
}

static NvAPI_Status CallFanCoolerGetControl(NvPhysicalGpuHandle gpu, void *data) {
  return NvAPI_GPU_FanCoolerGetControl(gpu, reinterpret_cast<NV_GPU_FAN_COOLER_CONTROL_PARAMS *>(data));
}

static NvAPI_Status CallFanCoolerSetControl(NvPhysicalGpuHandle gpu, void *data) {
  return NvAPI_GPU_FanCoolerSetControl(gpu, reinterpret_cast<NV_GPU_FAN_COOLER_CONTROL_PARAMS *>(data));
}

static NvAPI_Status CallFanPolicyGetInfo(NvPhysicalGpuHandle gpu, void *data) {
  return NvAPI_GPU_FanPolicyGetInfo(gpu, reinterpret_cast<NV_GPU_FAN_POLICY_INFO_PARAMS *>(data));
}

static NvAPI_Status CallFanPolicyGetStatus(NvPhysicalGpuHandle gpu, void *data) {
  return NvAPI_GPU_FanPolicyGetStatus(gpu, reinterpret_cast<NV_GPU_FAN_POLICY_STATUS_PARAMS *>(data));
}

static NvAPI_Status CallFanPolicyGetControl(NvPhysicalGpuHandle gpu, void *data) {
  return NvAPI_GPU_FanPolicyGetControl(gpu, reinterpret_cast<NV_GPU_FAN_POLICY_CONTROL_PARAMS *>(data));
}

static NvAPI_Status CallFanPolicySetControl(NvPhysicalGpuHandle gpu, void *data) {
  return NvAPI_GPU_FanPolicySetControl(gpu, reinterpret_cast<NV_GPU_FAN_POLICY_CONTROL_PARAMS *>(data));
}

static NvAPI_Status CallFanTestGetInfo(NvPhysicalGpuHandle gpu, void *data) {
  return NvAPI_GPU_FanTestGetInfo(gpu, reinterpret_cast<NV_GPU_FAN_TEST_INFO_PARAMS *>(data));
}

static NvAPI_Status CallFanPmumonFanCoolersGetSamples(NvPhysicalGpuHandle gpu, void *data) {
  return NvAPI_GPU_FanPmumonFanCoolersGetSamples(gpu,
                                                 reinterpret_cast<NV_GPU_FAN_PMUMON_FAN_COOLERS_GET_SAMPLES *>(data));
}

static NvAPI_Status CallThermalPolicyGetInfo(NvPhysicalGpuHandle gpu, void *data) {
  return NvAPI_GPU_ThermalPolicyGetInfo(gpu, reinterpret_cast<NV_GPU_THERMAL_POLICY_INFO_PARAMS *>(data));
}

static NvAPI_Status CallThermalPolicyGetStatus(NvPhysicalGpuHandle gpu, void *data) {
  return NvAPI_GPU_ThermalPolicyGetStatus(gpu, reinterpret_cast<NV_GPU_THERMAL_POLICY_STATUS_PARAMS *>(data));
}

static NvAPI_Status CallThermalPolicyGetControl(NvPhysicalGpuHandle gpu, void *data) {
  return NvAPI_GPU_ThermalPolicyGetControl(gpu, reinterpret_cast<NV_GPU_THERMAL_POLICY_CONTROL_PARAMS *>(data));
}

static NvAPI_Status CallThermalPolicySetControl(NvPhysicalGpuHandle gpu, void *data) {
  return NvAPI_GPU_ThermalPolicySetControl(gpu, reinterpret_cast<NV_GPU_THERMAL_POLICY_CONTROL_PARAMS *>(data));
}

static NvAPI_Status CallThermChannelGetInfo(NvPhysicalGpuHandle gpu, void *data) {
  return NvAPI_GPU_ThermChannelGetInfo(gpu, reinterpret_cast<NV_GPU_THERMAL_THERM_CHANNEL_INFO_PARAMS *>(data));
}

static NvAPI_Status CallThermChannelGetStatus(NvPhysicalGpuHandle gpu, void *data) {
  return NvAPI_GPU_ThermChannelGetStatus(gpu, reinterpret_cast<NV_GPU_THERMAL_THERM_CHANNEL_STATUS_PARAMS *>(data));
}

static NvAPI_Status CallThermChannelGetControl(NvPhysicalGpuHandle gpu, void *data) {
  return NvAPI_GPU_ThermChannelGetControl(gpu, reinterpret_cast<NV_GPU_THERMAL_THERM_CHANNEL_CONTROL_PARAMS *>(data));
}

static NvAPI_Status CallThermChannelSetControl(NvPhysicalGpuHandle gpu, void *data) {
  return NvAPI_GPU_ThermChannelSetControl(gpu, reinterpret_cast<NV_GPU_THERMAL_THERM_CHANNEL_CONTROL_PARAMS *>(data));
}

static NvAPI_Status CallThermDeviceGetInfo(NvPhysicalGpuHandle gpu, void *data) {
  return NvAPI_GPU_ThermDeviceGetInfo(gpu, reinterpret_cast<NV_GPU_THERMAL_THERM_DEVICE_INFO_PARAMS *>(data));
}

static NvAPI_Status CallThermMonitorsGetInfo(NvPhysicalGpuHandle gpu, void *data) {
  return NvAPI_GPU_ThermMonitorsGetInfo(gpu, reinterpret_cast<NV_GPU_THERMAL_THERM_MONITORS_INFO *>(data));
}

static NvAPI_Status CallThermMonitorsGetStatus(NvPhysicalGpuHandle gpu, void *data) {
  return NvAPI_GPU_ThermMonitorsGetStatus(gpu, reinterpret_cast<NV_GPU_THERMAL_THERM_MONITORS_STATUS *>(data));
}

static NvAPI_Status CallThermHwFsSlowdownAmountGet(NvPhysicalGpuHandle gpu, void *data) {
  return NvAPI_GPU_ThermHwFsSlowdownAmountGet(gpu,
                                              reinterpret_cast<NV_GPU_THERMAL_HWFS_SLOWDOWN_AMOUNT_GET_PARAMS *>(data));
}

static NvAPI_Status CallThermalHwFsGetInfo(NvPhysicalGpuHandle gpu, void *data) {
  return NvAPI_GPU_ThermalHwFsGetInfo(gpu, reinterpret_cast<NV_GPU_THERMAL_HWFS_EVENT_SETTINGS_PARAMS *>(data));
}

static NvAPI_Status CallThermalHwFsSetInfo(NvPhysicalGpuHandle gpu, void *data) {
  return NvAPI_GPU_ThermalHwFsSetInfo(gpu, reinterpret_cast<NV_GPU_THERMAL_HWFS_EVENT_SETTINGS_PARAMS *>(data));
}

static NvAPI_Status CallThermalPmumonThermChannelsGetSamples(NvPhysicalGpuHandle gpu, void *data) {
  return NvAPI_GPU_ThermalPmumonThermChannelsGetSamples(
      gpu, reinterpret_cast<NV_GPU_THERMAL_PMUMON_THERM_CHANNELS_GET_SAMPLES *>(data));
}

static NvAPI_Status CallPerfPoliciesGetInfo(NvPhysicalGpuHandle gpu, void *data) {
  return NvAPI_GPU_PerfPoliciesGetInfo(gpu, reinterpret_cast<NV_GPU_PERF_POLICIES_INFO_PARAMS *>(data));
}

static NvAPI_Status CallPerfPoliciesGetStatus(NvPhysicalGpuHandle gpu, void *data) {
  return NvAPI_GPU_PerfPoliciesGetStatus(gpu, reinterpret_cast<NV_GPU_PERF_POLICIES_STATUS_PARAMS *>(data));
}

static NvAPI_Status CallPerfPmumonPerfPoliciesGetSamples(NvPhysicalGpuHandle gpu, void *data) {
  return NvAPI_GPU_PerfPmumonPerfPoliciesGetSamples(
      gpu, reinterpret_cast<NV_GPU_PERF_PMUMON_PERF_POLICIES_GET_SAMPLES *>(data));
}

static const ApiSpec kClockApiSpecs[] = {
    {"counter-avgfreq", "Clock counter average frequency", sizeof(NV_GPU_CLOCK_COUNTER_MEASURE_AVG_FREQ_PARAMS),
     NV_GPU_CLOCK_COUNTER_MEASURE_AVG_FREQ_PARAMS_VER, false, CallClockCounterMeasureAvgFreq,
     PrepareClockCounterAvgFreq},
    {"clk-domains-info", "CLK_DOMAIN info", sizeof(NV_GPU_CLOCK_CLK_DOMAINS_INFO), NV_GPU_CLOCK_CLK_DOMAINS_INFO_VER,
     false, CallClockClkDomainsGetInfo, nullptr},
    {"clk-domains-control", "CLK_DOMAIN control", sizeof(NV_GPU_CLOCK_CLK_DOMAINS_CONTROL),
     NV_GPU_CLOCK_CLK_DOMAINS_CONTROL_VER, false, CallClockClkDomainsGetControl, nullptr},
    {"clk-domains-set", "CLK_DOMAIN set control", sizeof(NV_GPU_CLOCK_CLK_DOMAINS_CONTROL),
     NV_GPU_CLOCK_CLK_DOMAINS_CONTROL_VER, true, CallClockClkDomainsSetControl, nullptr},
    {"clk-domains-freq-info", "CLK_DOMAIN frequency info", sizeof(NV_GPU_CLOCK_CLK_DOMAINS_FREQ_INFO),
     NV_GPU_CLOCK_CLK_DOMAINS_FREQ_INFO_VER, false, CallClockClkDomainsGetFreqInfo, PrepareClockDomainsFreqInfo},
    {"clk-domain-freqs", "CLK_DOMAIN frequency enumeration", sizeof(NV_GPU_CLOCK_CLK_DOMAIN_FREQS_ENUM),
     NV_GPU_CLOCK_CLK_DOMAIN_FREQS_ENUM_VER, false, CallClockClkDomainFreqsEnum, PrepareClockDomainFreqsEnum},
    {"clk-domain-rpc", "CLK_DOMAIN RPC", sizeof(NV_GPU_CLOCK_CLK_DOMAIN_RPC), NV_GPU_CLOCK_CLK_DOMAIN_RPC_VER, false,
     CallClockClkDomainRpc, PrepareClockDomainRpc},
    {"clk-progs-info", "CLK_PROG info", sizeof(NV_GPU_CLOCK_CLK_PROGS_INFO), NV_GPU_CLOCK_CLK_PROGS_INFO_VER, false,
     CallClockClkProgsGetInfo, nullptr},
    {"clk-progs-status", "CLK_PROG status", sizeof(NV_GPU_CLOCK_CLK_PROGS_STATUS), NV_GPU_CLOCK_CLK_PROGS_STATUS_VER,
     false, CallClockClkProgsGetStatus, nullptr},
    {"clk-progs-control", "CLK_PROG control", sizeof(NV_GPU_CLOCK_CLK_PROGS_CONTROL),
     NV_GPU_CLOCK_CLK_PROGS_CONTROL_VER, false, CallClockClkProgsGetControl, nullptr},
    {"clk-progs-set", "CLK_PROG set control", sizeof(NV_GPU_CLOCK_CLK_PROGS_CONTROL),
     NV_GPU_CLOCK_CLK_PROGS_CONTROL_VER, true, CallClockClkProgsSetControl, nullptr},
    {"clk-enums-info", "CLK enums info", sizeof(NV_GPU_CLOCK_CLK_ENUMS_INFO), NV_GPU_CLOCK_CLK_ENUMS_INFO_VER, false,
     CallClockClkEnumsGetInfo, nullptr},
    {"clk-vf-rels-info", "CLK VF relationships info", sizeof(NV_GPU_CLOCK_CLK_VF_RELS_INFO),
     NV_GPU_CLOCK_CLK_VF_RELS_INFO_VER, false, CallClockClkVfRelsGetInfo, nullptr},
    {"clk-vf-rels-status", "CLK VF relationships status", sizeof(NV_GPU_CLOCK_CLK_VF_RELS_STATUS),
     NV_GPU_CLOCK_CLK_VF_RELS_STATUS_VER, false, CallClockClkVfRelsGetStatus, nullptr},
    {"clk-vf-rels-control", "CLK VF relationships control", sizeof(NV_GPU_CLOCK_CLK_VF_RELS_CONTROL),
     NV_GPU_CLOCK_CLK_VF_RELS_CONTROL_VER, false, CallClockClkVfRelsGetControl, nullptr},
    {"clk-vf-rels-set", "CLK VF relationships set control", sizeof(NV_GPU_CLOCK_CLK_VF_RELS_CONTROL),
     NV_GPU_CLOCK_CLK_VF_RELS_CONTROL_VER, true, CallClockClkVfRelsSetControl, nullptr},
    {"clk-vf-points-info", "CLK VF points info", sizeof(NV_GPU_CLOCK_CLK_VF_POINTS_INFO),
     NV_GPU_CLOCK_CLK_VF_POINTS_INFO_VER, false, CallClockClkVfPointsGetInfo, nullptr},
    {"clk-vf-points-status", "CLK VF points status", sizeof(NV_GPU_CLOCK_CLK_VF_POINTS_STATUS),
     NV_GPU_CLOCK_CLK_VF_POINTS_STATUS_VER, false, CallClockClkVfPointsGetStatus, nullptr},
    {"clk-vf-points-control", "CLK VF points control", sizeof(NV_GPU_CLOCK_CLK_VF_POINTS_CONTROL),
     NV_GPU_CLOCK_CLK_VF_POINTS_CONTROL_VER, false, CallClockClkVfPointsGetControl, nullptr},
    {"clk-vf-points-set", "CLK VF points set control", sizeof(NV_GPU_CLOCK_CLK_VF_POINTS_CONTROL),
     NV_GPU_CLOCK_CLK_VF_POINTS_CONTROL_VER, true, CallClockClkVfPointsSetControl, nullptr},
    {"clk-prop-regimes-info", "CLK_PROP regimes info", sizeof(NV_GPU_CLOCK_CLK_PROP_REGIMES_INFO),
     NV_GPU_CLOCK_CLK_PROP_REGIMES_INFO_VER, false, CallClockClkPropRegimesGetInfo, nullptr},
    {"clk-prop-regimes-control", "CLK_PROP regimes control", sizeof(NV_GPU_CLOCK_CLK_PROP_REGIMES_CONTROL),
     NV_GPU_CLOCK_CLK_PROP_REGIMES_CONTROL_VER, false, CallClockClkPropRegimesGetControl, nullptr},
    {"clk-prop-regimes-set", "CLK_PROP regimes set control", sizeof(NV_GPU_CLOCK_CLK_PROP_REGIMES_CONTROL),
     NV_GPU_CLOCK_CLK_PROP_REGIMES_CONTROL_VER, true, CallClockClkPropRegimesSetControl, nullptr},
    {"clk-prop-tops-info", "CLK_PROP tops info", sizeof(NV_GPU_CLOCK_CLK_PROP_TOPS_INFO),
     NV_GPU_CLOCK_CLK_PROP_TOPS_INFO_VER, false, CallClockClkPropTopsGetInfo, nullptr},
    {"clk-prop-tops-status", "CLK_PROP tops status", sizeof(NV_GPU_CLOCK_CLK_PROP_TOPS_STATUS),
     NV_GPU_CLOCK_CLK_PROP_TOPS_STATUS_VER, false, CallClockClkPropTopsGetStatus, nullptr},
    {"clk-prop-tops-control", "CLK_PROP tops control", sizeof(NV_GPU_CLOCK_CLK_PROP_TOPS_CONTROL),
     NV_GPU_CLOCK_CLK_PROP_TOPS_CONTROL_VER, false, CallClockClkPropTopsGetControl, nullptr},
    {"clk-prop-tops-set", "CLK_PROP tops set control", sizeof(NV_GPU_CLOCK_CLK_PROP_TOPS_CONTROL),
     NV_GPU_CLOCK_CLK_PROP_TOPS_CONTROL_VER, true, CallClockClkPropTopsSetControl, nullptr},
    {"clk-prop-top-rels-info", "CLK_PROP top relationships info", sizeof(NV_GPU_CLOCK_CLK_PROP_TOP_RELS_INFO),
     NV_GPU_CLOCK_CLK_PROP_TOP_RELS_INFO_VER, false, CallClockClkPropTopRelsGetInfo, nullptr},
    {"clk-prop-top-rels-control", "CLK_PROP top relationships control", sizeof(NV_GPU_CLOCK_CLK_PROP_TOP_RELS_CONTROL),
     NV_GPU_CLOCK_CLK_PROP_TOP_RELS_CONTROL_VER, false, CallClockClkPropTopRelsGetControl, nullptr},
    {"clk-prop-top-rels-set", "CLK_PROP top relationships set control", sizeof(NV_GPU_CLOCK_CLK_PROP_TOP_RELS_CONTROL),
     NV_GPU_CLOCK_CLK_PROP_TOP_RELS_CONTROL_VER, true, CallClockClkPropTopRelsSetControl, nullptr},
    {"adc-devices-info", "ADC devices info", sizeof(NV_GPU_CLOCK_ADC_DEVICES_INFO), NV_GPU_CLOCK_ADC_DEVICES_INFO_VER,
     false, CallClockAdcDevicesGetInfo, nullptr},
    {"adc-devices-status", "ADC devices status", sizeof(NV_GPU_CLOCK_ADC_DEVICES_STATUS),
     NV_GPU_CLOCK_ADC_DEVICES_STATUS_VER, false, CallClockAdcDevicesGetStatus, nullptr},
    {"adc-devices-control", "ADC devices control", sizeof(NV_GPU_CLOCK_ADC_DEVICES_CONTROL),
     NV_GPU_CLOCK_ADC_DEVICES_CONTROL_VER, false, CallClockAdcDevicesGetControl, nullptr},
    {"adc-devices-set", "ADC devices set control", sizeof(NV_GPU_CLOCK_ADC_DEVICES_CONTROL),
     NV_GPU_CLOCK_ADC_DEVICES_CONTROL_VER, true, CallClockAdcDevicesSetControl, nullptr},
    {"nafll-devices-info", "NAFLL devices info", sizeof(NV_GPU_CLOCK_NAFLL_DEVICES_INFO),
     NV_GPU_CLOCK_NAFLL_DEVICES_INFO_VER, false, CallClockNafllDevicesGetInfo, nullptr},
    {"nafll-devices-status", "NAFLL devices status", sizeof(NV_GPU_CLOCK_NAFLL_DEVICES_STATUS),
     NV_GPU_CLOCK_NAFLL_DEVICES_STATUS_VER, false, CallClockNafllDevicesGetStatus, nullptr},
    {"nafll-devices-control", "NAFLL devices control", sizeof(NV_GPU_CLOCK_NAFLL_DEVICES_CONTROL),
     NV_GPU_CLOCK_NAFLL_DEVICES_CONTROL_VER, false, CallClockNafllDevicesGetControl, nullptr},
    {"nafll-devices-set", "NAFLL devices set control", sizeof(NV_GPU_CLOCK_NAFLL_DEVICES_CONTROL),
     NV_GPU_CLOCK_NAFLL_DEVICES_CONTROL_VER, true, CallClockNafllDevicesSetControl, nullptr},
    {"clk-freq-controllers-info", "CLK frequency controllers info", sizeof(NV_GPU_CLOCK_CLK_FREQ_CONTROLLERS_INFO),
     NV_GPU_CLOCK_CLK_FREQ_CONTROLLERS_INFO_VER, false, CallClockClkFreqControllersGetInfo, nullptr},
    {"clk-freq-controllers-status", "CLK frequency controllers status",
     sizeof(NV_GPU_CLOCK_CLK_FREQ_CONTROLLERS_STATUS), NV_GPU_CLOCK_CLK_FREQ_CONTROLLERS_STATUS_VER, false,
     CallClockClkFreqControllersGetStatus, nullptr},
    {"clk-freq-controllers-control", "CLK frequency controllers control",
     sizeof(NV_GPU_CLOCK_CLK_FREQ_CONTROLLERS_CONTROL), NV_GPU_CLOCK_CLK_FREQ_CONTROLLERS_CONTROL_VER, false,
     CallClockClkFreqControllersGetControl, nullptr},
    {"clk-freq-controllers-set", "CLK frequency controllers set control",
     sizeof(NV_GPU_CLOCK_CLK_FREQ_CONTROLLERS_CONTROL), NV_GPU_CLOCK_CLK_FREQ_CONTROLLERS_CONTROL_VER, true,
     CallClockClkFreqControllersSetControl, nullptr},
    {"clk-volt-controllers-info", "CLK voltage controllers info", sizeof(NV_GPU_CLOCK_CLK_VOLT_CONTROLLERS_INFO),
     NV_GPU_CLOCK_CLK_VOLT_CONTROLLERS_INFO_VER, false, CallClockClkVoltControllersGetInfo, nullptr},
    {"clk-volt-controllers-status", "CLK voltage controllers status", sizeof(NV_GPU_CLOCK_CLK_VOLT_CONTROLLERS_STATUS),
     NV_GPU_CLOCK_CLK_VOLT_CONTROLLERS_STATUS_VER, false, CallClockClkVoltControllersGetStatus, nullptr},
    {"clk-volt-controllers-control", "CLK voltage controllers control",
     sizeof(NV_GPU_CLOCK_CLK_VOLT_CONTROLLERS_CONTROL), NV_GPU_CLOCK_CLK_VOLT_CONTROLLERS_CONTROL_VER, false,
     CallClockClkVoltControllersGetControl, nullptr},
    {"clk-volt-controllers-set", "CLK voltage controllers set control",
     sizeof(NV_GPU_CLOCK_CLK_VOLT_CONTROLLERS_CONTROL), NV_GPU_CLOCK_CLK_VOLT_CONTROLLERS_CONTROL_VER, true,
     CallClockClkVoltControllersSetControl, nullptr},
    {"client-domains-info", "Client CLK domains info", sizeof(NV_GPU_CLOCK_CLIENT_CLK_DOMAINS_INFO),
     NV_GPU_CLOCK_CLIENT_CLK_DOMAINS_INFO_VER, false, CallClockClientClkDomainsGetInfo, nullptr},
    {"client-vf-info", "Client CLK VF points info", sizeof(NV_GPU_CLOCK_CLIENT_CLK_VF_POINTS_INFO),
     NV_GPU_CLOCK_CLIENT_CLK_VF_POINTS_INFO_VER, false, CallClockClientClkVfPointsGetInfo, nullptr},
    {"client-vf-status", "Client CLK VF points status", sizeof(NV_GPU_CLOCK_CLIENT_CLK_VF_POINTS_STATUS),
     NV_GPU_CLOCK_CLIENT_CLK_VF_POINTS_STATUS_VER, false, CallClockClientClkVfPointsGetStatus, nullptr},
    {"client-vf-control", "Client CLK VF points control", sizeof(NV_GPU_CLOCK_CLIENT_CLK_VF_POINTS_CONTROL),
     NV_GPU_CLOCK_CLIENT_CLK_VF_POINTS_CONTROL_VER, false, CallClockClientClkVfPointsGetControl, nullptr},
    {"client-vf-set", "Client CLK VF points set control", sizeof(NV_GPU_CLOCK_CLIENT_CLK_VF_POINTS_CONTROL),
     NV_GPU_CLOCK_CLIENT_CLK_VF_POINTS_CONTROL_VER, true, CallClockClientClkVfPointsSetControl, nullptr},
    {"pmumon-clk-domains-samples", "PMUMON CLK domain samples", sizeof(NV_GPU_CLOCK_PMUMON_CLK_DOMAINS_GET_SAMPLES),
     NV_GPU_CLOCK_PMUMON_CLK_DOMAINS_GET_SAMPLES_VER, false, CallClockPmumonClkDomainsGetSamples, nullptr},
};

static const ApiSpec kPowerApiSpecs[] = {
    {"power-policy-info", "Power policy info", sizeof(NV_GPU_POWER_POLICY_INFO_PARAMS),
     NV_GPU_POWER_POLICY_INFO_PARAMS_VER, false, CallPowerPolicyGetInfo, nullptr},
    {"power-policy-status", "Power policy status", sizeof(NV_GPU_POWER_POLICY_STATUS_PARAMS),
     NV_GPU_POWER_POLICY_STATUS_PARAMS_VER, false, CallPowerPolicyGetStatus, nullptr},
    {"power-policy-control", "Power policy control", sizeof(NV_GPU_POWER_POLICY_CONTROL_PARAMS),
     NV_GPU_POWER_POLICY_CONTROL_PARAMS_VER, false, CallPowerPolicyGetControl, nullptr},
    {"power-policy-set", "Power policy set control", sizeof(NV_GPU_POWER_POLICY_CONTROL_PARAMS),
     NV_GPU_POWER_POLICY_CONTROL_PARAMS_VER, true, CallPowerPolicySetControl, nullptr},
    {"client-power-topology-info", "Client power topology info", sizeof(NV_GPU_CLIENT_POWER_TOPOLOGY_INFO),
     NV_GPU_CLIENT_POWER_TOPOLOGY_INFO_VER, false, CallClientPowerTopologyGetInfo, nullptr},
    {"client-power-topology-status", "Client power topology status", sizeof(NV_GPU_CLIENT_POWER_TOPOLOGY_STATUS),
     NV_GPU_CLIENT_POWER_TOPOLOGY_STATUS_VER, false, CallClientPowerTopologyGetStatus, nullptr},
    {"client-power-policies-info", "Client power policies info", sizeof(NV_GPU_CLIENT_POWER_POLICIES_INFO),
     NV_GPU_CLIENT_POWER_POLICIES_INFO_VER, false, CallClientPowerPoliciesGetInfo, nullptr},
    {"client-power-policies-status", "Client power policies status", sizeof(NV_GPU_CLIENT_POWER_POLICIES_STATUS),
     NV_GPU_CLIENT_POWER_POLICIES_STATUS_VER, false, CallClientPowerPoliciesGetStatus, nullptr},
    {"client-power-policies-set", "Client power policies set status", sizeof(NV_GPU_CLIENT_POWER_POLICIES_STATUS),
     NV_GPU_CLIENT_POWER_POLICIES_STATUS_VER, true, CallClientPowerPoliciesSetStatus, nullptr},
    {"client-thermal-policies-info", "Client thermal policies info", sizeof(NV_GPU_CLIENT_THERMAL_POLICIES_INFO),
     NV_GPU_CLIENT_THERMAL_POLICIES_INFO_VER, false, CallClientThermalPoliciesGetInfo, nullptr},
    {"client-thermal-policies-status", "Client thermal policies status", sizeof(NV_GPU_CLIENT_THERMAL_POLICIES_STATUS),
     NV_GPU_CLIENT_THERMAL_POLICIES_STATUS_VER, false, CallClientThermalPoliciesGetStatus, nullptr},
    {"client-thermal-policies-set", "Client thermal policies set status", sizeof(NV_GPU_CLIENT_THERMAL_POLICIES_STATUS),
     NV_GPU_CLIENT_THERMAL_POLICIES_STATUS_VER, true, CallClientThermalPoliciesSetStatus, nullptr},
    {"perf-policies-info", "Perf policies info", sizeof(NV_GPU_PERF_POLICIES_INFO_PARAMS),
     NV_GPU_PERF_POLICIES_INFO_PARAMS_VER, false, CallPerfPoliciesGetInfo, nullptr},
    {"perf-policies-status", "Perf policies status", sizeof(NV_GPU_PERF_POLICIES_STATUS_PARAMS),
     NV_GPU_PERF_POLICIES_STATUS_PARAMS_VER, false, CallPerfPoliciesGetStatus, nullptr},
    {"perf-policies-samples", "Perf policy PMUMON samples", sizeof(NV_GPU_PERF_PMUMON_PERF_POLICIES_GET_SAMPLES),
     NV_GPU_PERF_PMUMON_PERF_POLICIES_GET_SAMPLES_VER, false, CallPerfPmumonPerfPoliciesGetSamples, nullptr},
};

static const ApiSpec kThermalApiSpecs[] = {
    {"fan-arbiter-info", "Fan arbiter info", sizeof(NV_GPU_FAN_ARBITER_INFO_PARAMS), NV_GPU_FAN_ARBITER_INFO_PARAMS_VER,
     false, CallFanArbiterGetInfo, nullptr},
    {"fan-arbiter-status", "Fan arbiter status", sizeof(NV_GPU_FAN_ARBITER_STATUS_PARAMS),
     NV_GPU_FAN_ARBITER_STATUS_PARAMS_VER, false, CallFanArbiterGetStatus, nullptr},
    {"fan-cooler-info", "Fan cooler info", sizeof(NV_GPU_FAN_COOLER_INFO_PARAMS), NV_GPU_FAN_COOLER_INFO_PARAMS_VER,
     false, CallFanCoolerGetInfo, nullptr},
    {"fan-cooler-status", "Fan cooler status", sizeof(NV_GPU_FAN_COOLER_STATUS_PARAMS),
     NV_GPU_FAN_COOLER_STATUS_PARAMS_VER, false, CallFanCoolerGetStatus, nullptr},
    {"fan-cooler-control", "Fan cooler control", sizeof(NV_GPU_FAN_COOLER_CONTROL_PARAMS),
     NV_GPU_FAN_COOLER_CONTROL_PARAMS_VER, false, CallFanCoolerGetControl, nullptr},
    {"fan-cooler-set", "Fan cooler set control", sizeof(NV_GPU_FAN_COOLER_CONTROL_PARAMS),
     NV_GPU_FAN_COOLER_CONTROL_PARAMS_VER, true, CallFanCoolerSetControl, nullptr},
    {"fan-policy-info", "Fan policy info", sizeof(NV_GPU_FAN_POLICY_INFO_PARAMS), NV_GPU_FAN_POLICY_INFO_PARAMS_VER,
     false, CallFanPolicyGetInfo, nullptr},
    {"fan-policy-status", "Fan policy status", sizeof(NV_GPU_FAN_POLICY_STATUS_PARAMS),
     NV_GPU_FAN_POLICY_STATUS_PARAMS_VER, false, CallFanPolicyGetStatus, nullptr},
    {"fan-policy-control", "Fan policy control", sizeof(NV_GPU_FAN_POLICY_CONTROL_PARAMS),
     NV_GPU_FAN_POLICY_CONTROL_PARAMS_VER, false, CallFanPolicyGetControl, nullptr},
    {"fan-policy-set", "Fan policy set control", sizeof(NV_GPU_FAN_POLICY_CONTROL_PARAMS),
     NV_GPU_FAN_POLICY_CONTROL_PARAMS_VER, true, CallFanPolicySetControl, nullptr},
    {"fan-test-info", "Fan test info", sizeof(NV_GPU_FAN_TEST_INFO_PARAMS), NV_GPU_FAN_TEST_INFO_PARAMS_VER, false,
     CallFanTestGetInfo, nullptr},
    {"fan-pmumon-samples", "Fan PMUMON samples", sizeof(NV_GPU_FAN_PMUMON_FAN_COOLERS_GET_SAMPLES),
     NV_GPU_FAN_PMUMON_FAN_COOLERS_GET_SAMPLES_VER, false, CallFanPmumonFanCoolersGetSamples, nullptr},
    {"thermal-policy-info", "Thermal policy info", sizeof(NV_GPU_THERMAL_POLICY_INFO_PARAMS),
     NV_GPU_THERMAL_POLICY_INFO_PARAMS_VER, false, CallThermalPolicyGetInfo, nullptr},
    {"thermal-policy-status", "Thermal policy status", sizeof(NV_GPU_THERMAL_POLICY_STATUS_PARAMS),
     NV_GPU_THERMAL_POLICY_STATUS_PARAMS_VER, false, CallThermalPolicyGetStatus, nullptr},
    {"thermal-policy-control", "Thermal policy control", sizeof(NV_GPU_THERMAL_POLICY_CONTROL_PARAMS),
     NV_GPU_THERMAL_POLICY_CONTROL_PARAMS_VER, false, CallThermalPolicyGetControl, nullptr},
    {"thermal-policy-set", "Thermal policy set control", sizeof(NV_GPU_THERMAL_POLICY_CONTROL_PARAMS),
     NV_GPU_THERMAL_POLICY_CONTROL_PARAMS_VER, true, CallThermalPolicySetControl, nullptr},
    {"therm-channel-info", "Thermal channel info", sizeof(NV_GPU_THERMAL_THERM_CHANNEL_INFO_PARAMS),
     NV_GPU_THERMAL_THERM_CHANNEL_INFO_PARAMS_VER, false, CallThermChannelGetInfo, nullptr},
    {"therm-channel-status", "Thermal channel status", sizeof(NV_GPU_THERMAL_THERM_CHANNEL_STATUS_PARAMS),
     NV_GPU_THERMAL_THERM_CHANNEL_STATUS_PARAMS_VER, false, CallThermChannelGetStatus, nullptr},
    {"therm-channel-control", "Thermal channel control", sizeof(NV_GPU_THERMAL_THERM_CHANNEL_CONTROL_PARAMS),
     NV_GPU_THERMAL_THERM_CHANNEL_CONTROL_PARAMS_VER, false, CallThermChannelGetControl, nullptr},
    {"therm-channel-set", "Thermal channel set control", sizeof(NV_GPU_THERMAL_THERM_CHANNEL_CONTROL_PARAMS),
     NV_GPU_THERMAL_THERM_CHANNEL_CONTROL_PARAMS_VER, true, CallThermChannelSetControl, nullptr},
    {"therm-device-info", "Thermal device info", sizeof(NV_GPU_THERMAL_THERM_DEVICE_INFO_PARAMS),
     NV_GPU_THERMAL_THERM_DEVICE_INFO_PARAMS_VER, false, CallThermDeviceGetInfo, nullptr},
    {"therm-monitors-info", "Thermal monitors info", sizeof(NV_GPU_THERMAL_THERM_MONITORS_INFO),
     NV_GPU_THERMAL_THERM_MONITORS_INFO_VER, false, CallThermMonitorsGetInfo, nullptr},
    {"therm-monitors-status", "Thermal monitors status", sizeof(NV_GPU_THERMAL_THERM_MONITORS_STATUS),
     NV_GPU_THERMAL_THERM_MONITORS_STATUS_VER, false, CallThermMonitorsGetStatus, nullptr},
    {"therm-hwfs-slowdown", "Thermal HWFS slowdown amount", sizeof(NV_GPU_THERMAL_HWFS_SLOWDOWN_AMOUNT_GET_PARAMS),
     NV_GPU_THERMAL_HWFS_SLOWDOWN_AMOUNT_GET_PARAMS_VER, false, CallThermHwFsSlowdownAmountGet, nullptr},
    {"thermal-hwfs-info", "Thermal HWFS event info", sizeof(NV_GPU_THERMAL_HWFS_EVENT_SETTINGS_PARAMS),
     NV_GPU_THERMAL_HWFS_EVENT_SETTINGS_PARAMS_VER, false, CallThermalHwFsGetInfo, nullptr},
    {"thermal-hwfs-set", "Thermal HWFS event set", sizeof(NV_GPU_THERMAL_HWFS_EVENT_SETTINGS_PARAMS),
     NV_GPU_THERMAL_HWFS_EVENT_SETTINGS_PARAMS_VER, true, CallThermalHwFsSetInfo, nullptr},
    {"thermal-pmumon-samples", "Thermal PMUMON samples", sizeof(NV_GPU_THERMAL_PMUMON_THERM_CHANNELS_GET_SAMPLES),
     NV_GPU_THERMAL_PMUMON_THERM_CHANNELS_GET_SAMPLES_VER, false, CallThermalPmumonThermChannelsGetSamples, nullptr},
};

static const ApiSpec *FindApiSpec(const ApiSpec *specs, size_t count, const char *name) {
  for (size_t i = 0; i < count; ++i) {
    if (std::strcmp(specs[i].name, name) == 0) { return &specs[i]; }
  }
  return nullptr;
}

static void PrintApiList(const ApiSpec *specs, size_t count) {
  for (size_t i = 0; i < count; ++i) { std::printf("  %s - %s\n", specs[i].name, specs[i].desc); }
}

static int RunRawApiCommand(const char *groupLabel, const ApiSpec *specs, size_t count, int argc, char **argv) {
  if (argc < 1 || std::strcmp(argv[0], "list") == 0) {
    std::printf("%s api list:\n", groupLabel);
    PrintApiList(specs, count);
    return 0;
  }

  const char *apiName = argv[0];
  RawApiArgs args;
  if (!ParseRawApiArgs(argc - 1, argv + 1, &args)) { return 1; }

  const ApiSpec *spec = FindApiSpec(specs, count, apiName);
  if (!spec) {
    std::printf("Unknown %s api: %s\n", groupLabel, apiName);
    PrintApiList(specs, count);
    return 1;
  }

  if (spec->isSet && !args.inPath) {
    std::printf("Missing required --in for %s api %s\n", groupLabel, apiName);
    return 1;
  }

  std::vector<NvPhysicalGpuHandle> handles;
  std::vector<NvU32> indices;
  if (!CollectGpus(args.hasIndex, args.index, handles, indices)) { return 1; }

  bool multiGpu = handles.size() > 1;
  for (size_t i = 0; i < handles.size(); ++i) {
    PrintGpuHeader(indices[i], handles[i]);

    std::unique_ptr<NvU8[]> buffer(new NvU8[spec->size]);
    std::memset(buffer.get(), 0, spec->size);
    if (args.inPath && !ReadBinaryFile(args.inPath, buffer.get(), spec->size)) { return 1; }

    *reinterpret_cast<NvU32 *>(buffer.get()) = spec->version;
    if (spec->prepare) { spec->prepare(buffer.get(), args); }

    NvAPI_Status status = spec->call(handles[i], buffer.get());
    if (status != NVAPI_OK) {
      PrintNvapiError("  NVAPI call failed", status);
      continue;
    }

    NvU32 version = *reinterpret_cast<NvU32 *>(buffer.get());
    std::printf("  %s ok (size=%zu version=0x%08X)\n", apiName, spec->size, version);

    if (args.outPath) {
      std::string outPath = multiGpu ? AppendGpuIndexToPath(args.outPath, indices[i]) : std::string(args.outPath);
      if (!WriteBinaryFile(outPath.c_str(), buffer.get(), spec->size)) { return 1; }
      std::printf("  Wrote output: %s\n", outPath.c_str());
    }

    const bool dumpHex = args.raw || (!spec->isSet && !args.outPath);
    if (dumpHex) { DumpHex(buffer.get(), spec->size); }
  }

  return 0;
}

static void PrintClockUsage() {
  std::printf("Clock API access:\n");
  std::printf("  %s gpu clock list\n", kToolName);
  std::printf("  %s gpu clock <name> [--index N] [--raw] [--out PATH] [--in PATH]\n", kToolName);
  std::printf("  %s gpu clock api list\n", kToolName);
  std::printf("  %s gpu clock api <name> [--index N] [--raw] [--out PATH] [--in PATH]\n", kToolName);
  std::printf("     [--domain N] [--class N]\n");
}

static void PrintPowerUsage() {
  std::printf("Power API access:\n");
  std::printf("  %s gpu power api list\n", kToolName);
  std::printf("  %s gpu power api <name> [--index N] [--raw] [--out PATH] [--in PATH]\n", kToolName);
}

static void PrintThermalUsage() {
  std::printf("Thermal API access:\n");
  std::printf("  %s gpu thermal api list\n", kToolName);
  std::printf("  %s gpu thermal api <name> [--index N] [--raw] [--out PATH] [--in PATH]\n", kToolName);
}

int CmdGpuClock(int argc, char **argv) {
  if (argc < 1) {
    PrintClockUsage();
    return 1;
  }
  if (std::strcmp(argv[0], "list") == 0) {
    PrintApiList(kClockApiSpecs, sizeof(kClockApiSpecs) / sizeof(kClockApiSpecs[0]));
    return 0;
  }
  if (std::strcmp(argv[0], "api") == 0) {
    return RunRawApiCommand("clock", kClockApiSpecs, sizeof(kClockApiSpecs) / sizeof(kClockApiSpecs[0]), argc - 1,
                            argv + 1);
  }
  return RunRawApiCommand("clock", kClockApiSpecs, sizeof(kClockApiSpecs) / sizeof(kClockApiSpecs[0]), argc, argv);
}

int CmdGpuPowerApi(int argc, char **argv) {
  if (argc < 1) {
    PrintPowerUsage();
    return 1;
  }
  if (std::strcmp(argv[0], "list") == 0) {
    PrintApiList(kPowerApiSpecs, sizeof(kPowerApiSpecs) / sizeof(kPowerApiSpecs[0]));
    return 0;
  }
  return RunRawApiCommand("power", kPowerApiSpecs, sizeof(kPowerApiSpecs) / sizeof(kPowerApiSpecs[0]), argc, argv);
}

int CmdGpuThermalApi(int argc, char **argv) {
  if (argc < 1) {
    PrintThermalUsage();
    return 1;
  }
  if (std::strcmp(argv[0], "list") == 0) {
    PrintApiList(kThermalApiSpecs, sizeof(kThermalApiSpecs) / sizeof(kThermalApiSpecs[0]));
    return 0;
  }
  return RunRawApiCommand("thermal", kThermalApiSpecs, sizeof(kThermalApiSpecs) / sizeof(kThermalApiSpecs[0]), argc,
                          argv);
}
} // namespace nvcli
