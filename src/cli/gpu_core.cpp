/*
 * Copyright (c) 2026 Noverse (Nohuto). All rights reserved.
 * Proprietary and confidential. Unauthorized copying or redistribution is strictly prohibited.
 */

#include "cli/commands.h"

namespace nvcli {
namespace {
const char *CoolerPolicyName(NV_COOLER_POLICY policy) {
  switch (policy) {
  case NVAPI_COOLER_POLICY_MANUAL: return "MANUAL";
  case NVAPI_COOLER_POLICY_PERF: return "PERF";
  case NVAPI_COOLER_POLICY_TEMPERATURE_DISCRETE: return "TEMP_DISCRETE";
  case NVAPI_COOLER_POLICY_TEMPERATURE_CONTINUOUS: return "TEMP_CONT";
  case NVAPI_COOLER_POLICY_TEMPERATURE_CONTINUOUS_SW: return "TEMP_CONT_SW";
  case NVAPI_COOLER_POLICY_DEFAULT: return "DEFAULT";
  default: return "UNKNOWN";
  }
}
} // namespace
int CmdGpuThermalSlowdown(int argc, char **argv) {
  NvU32 index = 0;
  bool hasIndex = false;
  if (!ParseGpuIndexArgs(argc, argv, &index, &hasIndex)) { return 1; }

  std::vector<NvPhysicalGpuHandle> handles;
  std::vector<NvU32> indices;
  if (!CollectGpus(hasIndex, index, handles, indices)) { return 1; }

  for (size_t i = 0; i < handles.size(); ++i) {
    PrintGpuHeader(indices[i], handles[i]);
    NV_GPU_THERMAL_SLOWDOWN state = NVAPI_GPU_THERMAL_SLOWDOWN_ENABLED;
    NvAPI_Status status = NvAPI_GPU_GetThermalSlowdownState(handles[i], &state);
    if (status == NVAPI_OK) {
      const char *label = (state == NVAPI_GPU_THERMAL_SLOWDOWN_DISABLED_ALL) ? "disabled" : "enabled";
      std::printf("  Thermal slowdown: %s\n", label);
    } else {
      PrintNvapiError("  NvAPI_GPU_GetThermalSlowdownState failed", status);
    }
  }
  return 0;
}

int CmdGpuThermalSlowdownSet(int argc, char **argv) {
  NvU32 index = 0;
  bool hasIndex = false;
  const char *stateValue = NULL;

  for (int i = 0; i < argc; ++i) {
    if (std::strcmp(argv[i], "--index") == 0) {
      if (i + 1 >= argc) {
        std::printf("Missing value for --index\n");
        return 1;
      }
      if (!ParseUint(argv[i + 1], &index)) {
        std::printf("Invalid GPU index: %s\n", argv[i + 1]);
        return 1;
      }
      hasIndex = true;
      ++i;
      continue;
    }
    if (std::strcmp(argv[i], "--state") == 0) {
      if (i + 1 >= argc) {
        std::printf("Missing value for --state\n");
        return 1;
      }
      stateValue = argv[i + 1];
      ++i;
      continue;
    }
    std::printf("Unknown option: %s\n", argv[i]);
    return 1;
  }

  if (!stateValue) {
    std::printf("Missing required --state enabled|disabled\n");
    return 1;
  }

  NV_GPU_THERMAL_SLOWDOWN state = NVAPI_GPU_THERMAL_SLOWDOWN_ENABLED;
  if (std::strcmp(stateValue, "enabled") == 0) {
    state = NVAPI_GPU_THERMAL_SLOWDOWN_ENABLED;
  } else if (std::strcmp(stateValue, "disabled") == 0) {
    state = NVAPI_GPU_THERMAL_SLOWDOWN_DISABLED_ALL;
  } else {
    std::printf("Invalid state: %s\n", stateValue);
    return 1;
  }

  std::vector<NvPhysicalGpuHandle> handles;
  std::vector<NvU32> indices;
  if (!CollectGpus(hasIndex, index, handles, indices)) { return 1; }

  for (size_t i = 0; i < handles.size(); ++i) {
    PrintGpuHeader(indices[i], handles[i]);
    NvAPI_Status status = NvAPI_GPU_SetThermalSlowdownState(handles[i], state);
    if (status == NVAPI_OK) {
      std::printf("  Thermal slowdown updated.\n");
    } else {
      PrintNvapiError("  NvAPI_GPU_SetThermalSlowdownState failed", status);
    }
  }
  return 0;
}

int CmdGpuThermalSimGet(int argc, char **argv) {
  NvU32 index = 0;
  bool hasIndex = false;
  NvU32 sensor = 0;
  bool hasSensor = false;

  for (int i = 0; i < argc; ++i) {
    if (std::strcmp(argv[i], "--index") == 0) {
      if (i + 1 >= argc) {
        std::printf("Missing value for --index\n");
        return 1;
      }
      if (!ParseUint(argv[i + 1], &index)) {
        std::printf("Invalid GPU index: %s\n", argv[i + 1]);
        return 1;
      }
      hasIndex = true;
      ++i;
      continue;
    }
    if (std::strcmp(argv[i], "--sensor") == 0) {
      if (i + 1 >= argc) {
        std::printf("Missing value for --sensor\n");
        return 1;
      }
      if (!ParseUint(argv[i + 1], &sensor)) {
        std::printf("Invalid sensor index: %s\n", argv[i + 1]);
        return 1;
      }
      hasSensor = true;
      ++i;
      continue;
    }
    std::printf("Unknown option: %s\n", argv[i]);
    return 1;
  }

  if (!hasSensor) { sensor = 0; }

  std::vector<NvPhysicalGpuHandle> handles;
  std::vector<NvU32> indices;
  if (!CollectGpus(hasIndex, index, handles, indices)) { return 1; }

  for (size_t i = 0; i < handles.size(); ++i) {
    PrintGpuHeader(indices[i], handles[i]);
    NV_GPU_THERMAL_SIMULATION_MODE mode = NVAPI_GPU_THERMAL_SIMULATION_DISABLED;
    NvAPI_Status status = NvAPI_GPU_GetThermalSimulationMode(handles[i], sensor, &mode);
    if (status == NVAPI_OK) {
      const char *label = (mode == NVAPI_GPU_THERMAL_SIMULATION_ENABLED) ? "enabled" : "disabled";
      std::printf("  Thermal simulation (sensor %u): %s\n", sensor, label);
    } else {
      PrintNvapiError("  NvAPI_GPU_GetThermalSimulationMode failed", status);
    }
  }
  return 0;
}

int CmdGpuThermalSimSet(int argc, char **argv) {
  NvU32 index = 0;
  bool hasIndex = false;
  NvU32 sensor = 0;
  bool hasSensor = false;
  const char *modeValue = NULL;
  NvU32 temperature = 0;
  bool hasTemp = false;

  for (int i = 0; i < argc; ++i) {
    if (std::strcmp(argv[i], "--index") == 0) {
      if (i + 1 >= argc) {
        std::printf("Missing value for --index\n");
        return 1;
      }
      if (!ParseUint(argv[i + 1], &index)) {
        std::printf("Invalid GPU index: %s\n", argv[i + 1]);
        return 1;
      }
      hasIndex = true;
      ++i;
      continue;
    }
    if (std::strcmp(argv[i], "--sensor") == 0) {
      if (i + 1 >= argc) {
        std::printf("Missing value for --sensor\n");
        return 1;
      }
      if (!ParseUint(argv[i + 1], &sensor)) {
        std::printf("Invalid sensor index: %s\n", argv[i + 1]);
        return 1;
      }
      hasSensor = true;
      ++i;
      continue;
    }
    if (std::strcmp(argv[i], "--mode") == 0) {
      if (i + 1 >= argc) {
        std::printf("Missing value for --mode\n");
        return 1;
      }
      modeValue = argv[i + 1];
      ++i;
      continue;
    }
    if (std::strcmp(argv[i], "--temp") == 0) {
      if (i + 1 >= argc) {
        std::printf("Missing value for --temp\n");
        return 1;
      }
      if (!ParseUint(argv[i + 1], &temperature)) {
        std::printf("Invalid temperature: %s\n", argv[i + 1]);
        return 1;
      }
      hasTemp = true;
      ++i;
      continue;
    }
    std::printf("Unknown option: %s\n", argv[i]);
    return 1;
  }

  if (!modeValue) {
    std::printf("Missing required --mode enabled|disabled\n");
    return 1;
  }
  if (!hasSensor) {
    std::printf("Missing required --sensor\n");
    return 1;
  }

  NV_GPU_THERMAL_SIMULATION_MODE mode = NVAPI_GPU_THERMAL_SIMULATION_DISABLED;
  if (std::strcmp(modeValue, "enabled") == 0) {
    mode = NVAPI_GPU_THERMAL_SIMULATION_ENABLED;
  } else if (std::strcmp(modeValue, "disabled") == 0) {
    mode = NVAPI_GPU_THERMAL_SIMULATION_DISABLED;
  } else {
    std::printf("Invalid mode: %s\n", modeValue);
    return 1;
  }

  if (mode == NVAPI_GPU_THERMAL_SIMULATION_ENABLED && !hasTemp) {
    std::printf("Missing required --temp when enabling thermal simulation\n");
    return 1;
  }

  std::vector<NvPhysicalGpuHandle> handles;
  std::vector<NvU32> indices;
  if (!CollectGpus(hasIndex, index, handles, indices)) { return 1; }

  for (size_t i = 0; i < handles.size(); ++i) {
    PrintGpuHeader(indices[i], handles[i]);
    NvAPI_Status status = NvAPI_GPU_SetThermalSimulationMode(handles[i], sensor, mode, temperature);
    if (status == NVAPI_OK) {
      std::printf("  Thermal simulation updated.\n");
    } else {
      PrintNvapiError("  NvAPI_GPU_SetThermalSimulationMode failed", status);
    }
  }
  return 0;
}

int CmdGpuFanSet(int argc, char **argv) {
  NvU32 index = 0;
  bool hasIndex = false;
  NvU32 coolerIndex = 0;
  bool hasCooler = false;
  NvU32 level = 0;
  bool hasLevel = false;
  NV_COOLER_POLICY policy = NVAPI_COOLER_POLICY_MANUAL;

  for (int i = 0; i < argc; ++i) {
    if (std::strcmp(argv[i], "--index") == 0) {
      if (i + 1 >= argc) {
        std::printf("Missing value for --index\n");
        return 1;
      }
      if (!ParseUint(argv[i + 1], &index)) {
        std::printf("Invalid GPU index: %s\n", argv[i + 1]);
        return 1;
      }
      hasIndex = true;
      ++i;
      continue;
    }
    if (std::strcmp(argv[i], "--cooler") == 0) {
      if (i + 1 >= argc) {
        std::printf("Missing value for --cooler\n");
        return 1;
      }
      if (!ParseUint(argv[i + 1], &coolerIndex)) {
        std::printf("Invalid cooler index: %s\n", argv[i + 1]);
        return 1;
      }
      hasCooler = true;
      ++i;
      continue;
    }
    if (std::strcmp(argv[i], "--level") == 0) {
      if (i + 1 >= argc) {
        std::printf("Missing value for --level\n");
        return 1;
      }
      if (!ParseUint(argv[i + 1], &level)) {
        std::printf("Invalid level: %s\n", argv[i + 1]);
        return 1;
      }
      hasLevel = true;
      ++i;
      continue;
    }
    if (std::strcmp(argv[i], "--policy") == 0) {
      if (i + 1 >= argc) {
        std::printf("Missing value for --policy\n");
        return 1;
      }
      if (!ParseCoolerPolicy(argv[i + 1], &policy)) {
        std::printf("Invalid policy: %s\n", argv[i + 1]);
        return 1;
      }
      ++i;
      continue;
    }
    std::printf("Unknown option: %s\n", argv[i]);
    return 1;
  }

  if (!hasCooler || !hasLevel) {
    std::printf("Missing required --cooler and/or --level\n");
    return 1;
  }
  if (level > 100) {
    std::printf("Level must be between 0 and 100\n");
    return 1;
  }

  std::vector<NvPhysicalGpuHandle> handles;
  std::vector<NvU32> indices;
  if (!CollectGpus(hasIndex, index, handles, indices)) { return 1; }

  for (size_t i = 0; i < handles.size(); ++i) {
    PrintGpuHeader(indices[i], handles[i]);
    NV_GPU_SETCOOLER_LEVEL levels = {};
    levels.version = NV_GPU_SETCOOLER_LEVEL_VER;
    levels.cooler[coolerIndex].currentLevel = level;
    levels.cooler[coolerIndex].currentPolicy = policy;
    NvAPI_Status status = NvAPI_GPU_SetCoolerLevels(handles[i], coolerIndex, &levels);
    if (status == NVAPI_OK) {
      std::printf("  Fan level updated.\n");
    } else {
      PrintNvapiError("  NvAPI_GPU_SetCoolerLevels failed", status);
    }
  }
  return 0;
}

int CmdGpuFanRestore(int argc, char **argv) {
  NvU32 index = 0;
  bool hasIndex = false;
  NvU32 coolerIndex = 0;
  bool hasCooler = false;

  for (int i = 0; i < argc; ++i) {
    if (std::strcmp(argv[i], "--index") == 0) {
      if (i + 1 >= argc) {
        std::printf("Missing value for --index\n");
        return 1;
      }
      if (!ParseUint(argv[i + 1], &index)) {
        std::printf("Invalid GPU index: %s\n", argv[i + 1]);
        return 1;
      }
      hasIndex = true;
      ++i;
      continue;
    }
    if (std::strcmp(argv[i], "--cooler") == 0) {
      if (i + 1 >= argc) {
        std::printf("Missing value for --cooler\n");
        return 1;
      }
      if (!ParseUint(argv[i + 1], &coolerIndex)) {
        std::printf("Invalid cooler index: %s\n", argv[i + 1]);
        return 1;
      }
      hasCooler = true;
      ++i;
      continue;
    }
    std::printf("Unknown option: %s\n", argv[i]);
    return 1;
  }

  std::vector<NvPhysicalGpuHandle> handles;
  std::vector<NvU32> indices;
  if (!CollectGpus(hasIndex, index, handles, indices)) { return 1; }

  for (size_t i = 0; i < handles.size(); ++i) {
    PrintGpuHeader(indices[i], handles[i]);
    NvAPI_Status status = NVAPI_OK;
    if (hasCooler) {
      NvU32 cooler = coolerIndex;
      status = NvAPI_GPU_RestoreCoolerSettings(handles[i], &cooler, 1);
    } else {
      status = NvAPI_GPU_RestoreCoolerSettings(handles[i], NULL, 0);
    }
    if (status == NVAPI_OK) {
      std::printf("  Fan settings restored.\n");
    } else {
      PrintNvapiError("  NvAPI_GPU_RestoreCoolerSettings failed", status);
    }
  }
  return 0;
}

int CmdGpuPowerLimitGet(int argc, char **argv) {
  NvU32 index = 0;
  bool hasIndex = false;
  if (!ParseGpuIndexArgs(argc, argv, &index, &hasIndex)) { return 1; }

  std::vector<NvPhysicalGpuHandle> handles;
  std::vector<NvU32> indices;
  if (!CollectGpus(hasIndex, index, handles, indices)) { return 1; }

  for (size_t i = 0; i < handles.size(); ++i) {
    PrintGpuHeader(indices[i], handles[i]);
    NvU8 limit = 0;
    NvU32 flags = 0;
    NvAPI_Status status = NvAPI_GPU_GetPerfLimit(handles[i], &limit, &flags);
    if (status == NVAPI_OK) {
      if (limit == NV_GPU_PERF_LIMIT_MAX) {
        std::printf("  Perf limit: max (no limit)\n");
      } else {
        std::printf("  Perf limit: %u\n", limit);
      }
      std::printf("  Perf limit flags: 0x%08X\n", flags);
    } else {
      PrintNvapiError("  NvAPI_GPU_GetPerfLimit failed", status);
    }
  }
  return 0;
}

int CmdGpuPowerLimitSet(int argc, char **argv) {
  NvU32 index = 0;
  bool hasIndex = false;
  NvU32 flags = 0;
  bool hasFlags = false;
  const char *limitValue = NULL;

  for (int i = 0; i < argc; ++i) {
    if (std::strcmp(argv[i], "--index") == 0) {
      if (i + 1 >= argc) {
        std::printf("Missing value for --index\n");
        return 1;
      }
      if (!ParseUint(argv[i + 1], &index)) {
        std::printf("Invalid GPU index: %s\n", argv[i + 1]);
        return 1;
      }
      hasIndex = true;
      ++i;
      continue;
    }
    if (std::strcmp(argv[i], "--limit") == 0) {
      if (i + 1 >= argc) {
        std::printf("Missing value for --limit\n");
        return 1;
      }
      limitValue = argv[i + 1];
      ++i;
      continue;
    }
    if (std::strcmp(argv[i], "--flags") == 0) {
      if (i + 1 >= argc) {
        std::printf("Missing value for --flags\n");
        return 1;
      }
      if (!ParseUint(argv[i + 1], &flags)) {
        std::printf("Invalid flags: %s\n", argv[i + 1]);
        return 1;
      }
      hasFlags = true;
      ++i;
      continue;
    }
    std::printf("Unknown option: %s\n", argv[i]);
    return 1;
  }

  if (!limitValue) {
    std::printf("Missing required --limit (0-255 or max)\n");
    return 1;
  }

  NvU8 limit = 0;
  if (std::strcmp(limitValue, "max") == 0) {
    limit = NV_GPU_PERF_LIMIT_MAX;
  } else {
    NvU32 parsed = 0;
    if (!ParseUint(limitValue, &parsed) || parsed > 0xFF) {
      std::printf("Invalid limit: %s\n", limitValue);
      return 1;
    }
    limit = static_cast<NvU8>(parsed);
  }

  std::vector<NvPhysicalGpuHandle> handles;
  std::vector<NvU32> indices;
  if (!CollectGpus(hasIndex, index, handles, indices)) { return 1; }

  for (size_t i = 0; i < handles.size(); ++i) {
    PrintGpuHeader(indices[i], handles[i]);
    NvU32 flagsValue = hasFlags ? flags : 0;
    NvAPI_Status status = NvAPI_GPU_SetPerfLimit(handles[i], limit, &flagsValue);
    if (status == NVAPI_OK) {
      std::printf("  Perf limit updated.\n");
    } else {
      PrintNvapiError("  NvAPI_GPU_SetPerfLimit failed", status);
    }
  }
  return 0;
}

int CmdGpuVoltage(int argc, char **argv) {
  NvU32 index = 0;
  bool hasIndex = false;
  if (!ParseGpuIndexArgs(argc, argv, &index, &hasIndex)) { return 1; }

  std::vector<NvPhysicalGpuHandle> handles;
  std::vector<NvU32> indices;
  if (!CollectGpus(hasIndex, index, handles, indices)) { return 1; }

  for (size_t i = 0; i < handles.size(); ++i) {
    PrintGpuHeader(indices[i], handles[i]);

    NvU32 mvolt = 0;
    NvAPI_Status status = NvAPI_GPU_GetCoreVoltage(handles[i], &mvolt);
    if (status == NVAPI_OK) {
      std::printf("  Core voltage: %u mV\n", mvolt);
    } else {
      PrintNvapiError("  NvAPI_GPU_GetCoreVoltage failed", status);
    }

    NvU32 controlEnabled = 0;
    status = NvAPI_GPU_GetCoreVoltageControl(handles[i], &controlEnabled);
    if (status == NVAPI_OK) {
      std::printf("  Core voltage control: %s\n", controlEnabled ? "enabled" : "disabled");
    } else {
      PrintNvapiError("  NvAPI_GPU_GetCoreVoltageControl failed", status);
    }

    NV_GPU_VOLTAGE_DOMAINS_INFO domainsInfo = {};
    domainsInfo.version = NV_GPU_VOLTAGE_DOMAINS_INFO_VER;
    domainsInfo.numDomains = 0;
    status = NvAPI_GPU_GetVoltageDomainsInfo(handles[i], &domainsInfo);
    if (status == NVAPI_OK) {
      std::printf("  Voltage domains (info): %u\n", domainsInfo.numDomains);
      for (NvU32 d = 0; d < domainsInfo.numDomains; ++d) {
        const auto &domain = domainsInfo.domains[d];
        std::printf("    %s(%u): step=%u uV\n", VoltageDomainName(domain.domainId), domain.domainId, domain.stepSizeuV);
      }
    } else {
      PrintNvapiError("  NvAPI_GPU_GetVoltageDomainsInfo failed", status);
    }

    NV_GPU_VOLTAGE_DOMAINS_STATUS domainsStatus = {};
    domainsStatus.version = NV_GPU_VOLTAGE_DOMAINS_STATUS_VER;
    domainsStatus.numDomains = 0;
    status = NvAPI_GPU_GetVoltageDomainsStatus(handles[i], &domainsStatus);
    if (status == NVAPI_OK) {
      std::printf("  Voltage domains (status): %u\n", domainsStatus.numDomains);
      for (NvU32 d = 0; d < domainsStatus.numDomains; ++d) {
        const auto &domain = domainsStatus.domains[d];
        std::printf("    %s(%u): %u uV (%.3f mV)\n", VoltageDomainName(domain.domainId), domain.domainId,
                    domain.voltageuV, static_cast<double>(domain.voltageuV) / 1000.0);
      }
    } else {
      PrintNvapiError("  NvAPI_GPU_GetVoltageDomainsStatus failed", status);
    }

    NV_GPU_PERF_VOLTAGES voltages = {};
    voltages.version = NV_GPU_PERF_VOLTAGES_VER;
    status = NvAPI_GPU_GetVoltages(handles[i], &voltages);
    if (status == NVAPI_OK) {
      std::printf("  Voltage levels (domains): %u\n", voltages.numDomains);
      for (NvU32 d = 0; d < voltages.numDomains; ++d) {
        const auto &domain = voltages.domains[d];
        std::printf("    %s(%u): levels=%u\n", VoltageDomainName(domain.domainId), domain.domainId, domain.numVoltages);
        NvU32 limit = domain.numVoltages;
        if (limit > 16) { limit = 16; }
        for (NvU32 v = 0; v < limit; ++v) { std::printf("      [%u] %u mV\n", v, domain.voltages[v].mvolt); }
        if (domain.numVoltages > limit) { std::printf("      ... %u more\n", domain.numVoltages - limit); }
      }
    } else {
      PrintNvapiError("  NvAPI_GPU_GetVoltages failed", status);
    }
  }

  return 0;
}

int CmdGpuVoltageControlSet(int argc, char **argv) {
  NvU32 index = 0;
  bool hasIndex = false;
  bool hasEnable = false;
  NvU32 enable = 0;

  for (int i = 0; i < argc; ++i) {
    if (std::strcmp(argv[i], "--index") == 0) {
      if (i + 1 >= argc) {
        std::printf("Missing value for --index\n");
        return 1;
      }
      if (!ParseUint(argv[i + 1], &index)) {
        std::printf("Invalid GPU index: %s\n", argv[i + 1]);
        return 1;
      }
      hasIndex = true;
      ++i;
      continue;
    }
    if (std::strcmp(argv[i], "--enable") == 0) {
      if (i + 1 >= argc) {
        std::printf("Missing value for --enable\n");
        return 1;
      }
      if (!ParseUint(argv[i + 1], &enable)) {
        std::printf("Invalid enable value: %s\n", argv[i + 1]);
        return 1;
      }
      hasEnable = true;
      ++i;
      continue;
    }
    std::printf("Unknown option: %s\n", argv[i]);
    return 1;
  }

  if (!hasEnable) {
    std::printf("Missing required --enable 0|1\n");
    return 1;
  }

  std::vector<NvPhysicalGpuHandle> handles;
  std::vector<NvU32> indices;
  if (!CollectGpus(hasIndex, index, handles, indices)) { return 1; }

  for (size_t i = 0; i < handles.size(); ++i) {
    PrintGpuHeader(indices[i], handles[i]);
    NvAPI_Status status = NvAPI_GPU_SetCoreVoltageControl(handles[i], enable ? 1 : 0);
    if (status == NVAPI_OK) {
      std::printf("  Core voltage control updated.\n");
    } else {
      PrintNvapiError("  NvAPI_GPU_SetCoreVoltageControl failed", status);
    }
  }
  return 0;
}

int CmdGpuReport(int argc, char **argv) {
  NvU32 index = 0;
  bool hasIndex = false;
  if (!ParseGpuIndexArgs(argc, argv, &index, &hasIndex)) { return 1; }

  std::vector<NvPhysicalGpuHandle> handles;
  std::vector<NvU32> indices;
  if (!CollectGpus(hasIndex, index, handles, indices)) { return 1; }

  for (size_t i = 0; i < handles.size(); ++i) {
    PrintGpuHeader(indices[i], handles[i]);
    PrintBusInfo(handles[i]);
    PrintVbiosInfo(handles[i]);
    PrintMemoryInfo(handles[i]);
    PrintBarInfo(handles[i]);
    PrintPowerInfo(handles[i]);
    PrintPstateInfo(handles[i]);
    PrintUtilizationInfo(handles[i]);
    PrintClockInfo(handles[i]);
    PrintCoolerInfo(handles[i]);
  }

  return 0;
}

int CmdGpuMemory(int argc, char **argv) {
  NvU32 index = 0;
  bool hasIndex = false;
  if (!ParseGpuIndexArgs(argc, argv, &index, &hasIndex)) { return 1; }

  std::vector<NvPhysicalGpuHandle> handles;
  std::vector<NvU32> indices;
  if (!CollectGpus(hasIndex, index, handles, indices)) { return 1; }

  for (size_t i = 0; i < handles.size(); ++i) {
    PrintGpuHeader(indices[i], handles[i]);
    PrintMemoryInfo(handles[i]);
  }
  return 0;
}

int CmdGpuClocks(int argc, char **argv) {
  NvU32 index = 0;
  bool hasIndex = false;
  if (!ParseGpuIndexArgs(argc, argv, &index, &hasIndex)) { return 1; }

  std::vector<NvPhysicalGpuHandle> handles;
  std::vector<NvU32> indices;
  if (!CollectGpus(hasIndex, index, handles, indices)) { return 1; }

  for (size_t i = 0; i < handles.size(); ++i) {
    PrintGpuHeader(indices[i], handles[i]);
    PrintClockInfo(handles[i]);
  }
  return 0;
}

int CmdGpuUtilization(int argc, char **argv) {
  NvU32 index = 0;
  bool hasIndex = false;
  if (!ParseGpuIndexArgs(argc, argv, &index, &hasIndex)) { return 1; }

  std::vector<NvPhysicalGpuHandle> handles;
  std::vector<NvU32> indices;
  if (!CollectGpus(hasIndex, index, handles, indices)) { return 1; }

  for (size_t i = 0; i < handles.size(); ++i) {
    PrintGpuHeader(indices[i], handles[i]);
    PrintUtilizationInfo(handles[i]);
  }
  return 0;
}

int CmdGpuPstate(int argc, char **argv) {
  NvU32 index = 0;
  bool hasIndex = false;
  if (!ParseGpuIndexArgs(argc, argv, &index, &hasIndex)) { return 1; }

  std::vector<NvPhysicalGpuHandle> handles;
  std::vector<NvU32> indices;
  if (!CollectGpus(hasIndex, index, handles, indices)) { return 1; }

  for (size_t i = 0; i < handles.size(); ++i) {
    PrintGpuHeader(indices[i], handles[i]);
    PrintPstateInfo(handles[i]);
  }
  return 0;
}

int CmdGpuBus(int argc, char **argv) {
  NvU32 index = 0;
  bool hasIndex = false;
  if (!ParseGpuIndexArgs(argc, argv, &index, &hasIndex)) { return 1; }

  std::vector<NvPhysicalGpuHandle> handles;
  std::vector<NvU32> indices;
  if (!CollectGpus(hasIndex, index, handles, indices)) { return 1; }

  for (size_t i = 0; i < handles.size(); ++i) {
    PrintGpuHeader(indices[i], handles[i]);
    PrintBusInfo(handles[i]);
  }
  return 0;
}

int CmdGpuVbios(int argc, char **argv) {
  NvU32 index = 0;
  bool hasIndex = false;
  if (!ParseGpuIndexArgs(argc, argv, &index, &hasIndex)) { return 1; }

  std::vector<NvPhysicalGpuHandle> handles;
  std::vector<NvU32> indices;
  if (!CollectGpus(hasIndex, index, handles, indices)) { return 1; }

  for (size_t i = 0; i < handles.size(); ++i) {
    PrintGpuHeader(indices[i], handles[i]);
    PrintVbiosInfo(handles[i]);
  }
  return 0;
}

int CmdGpuCooler(int argc, char **argv) {
  NvU32 index = 0;
  bool hasIndex = false;
  if (!ParseGpuIndexArgs(argc, argv, &index, &hasIndex)) { return 1; }

  std::vector<NvPhysicalGpuHandle> handles;
  std::vector<NvU32> indices;
  if (!CollectGpus(hasIndex, index, handles, indices)) { return 1; }

  for (size_t i = 0; i < handles.size(); ++i) {
    PrintGpuHeader(indices[i], handles[i]);
    PrintCoolerInfo(handles[i]);
  }
  return 0;
}

int CmdGpuCoolerPolicyGet(int argc, char **argv) {
  NvU32 index = 0;
  bool hasIndex = false;
  NvU32 coolerIndex = 0;
  bool hasCooler = false;
  NV_COOLER_POLICY policy = NVAPI_COOLER_POLICY_PERF;

  for (int i = 0; i < argc; ++i) {
    if (std::strcmp(argv[i], "--index") == 0) {
      if (i + 1 >= argc) {
        std::printf("Missing value for --index\n");
        return 1;
      }
      if (!ParseUint(argv[i + 1], &index)) {
        std::printf("Invalid GPU index: %s\n", argv[i + 1]);
        return 1;
      }
      hasIndex = true;
      ++i;
      continue;
    }
    if (std::strcmp(argv[i], "--cooler") == 0) {
      if (i + 1 >= argc) {
        std::printf("Missing value for --cooler\n");
        return 1;
      }
      if (!ParseUint(argv[i + 1], &coolerIndex)) {
        std::printf("Invalid cooler index: %s\n", argv[i + 1]);
        return 1;
      }
      hasCooler = true;
      ++i;
      continue;
    }
    if (std::strcmp(argv[i], "--policy") == 0) {
      if (i + 1 >= argc) {
        std::printf("Missing value for --policy\n");
        return 1;
      }
      if (!ParseCoolerPolicy(argv[i + 1], &policy)) {
        std::printf("Invalid cooler policy: %s\n", argv[i + 1]);
        return 1;
      }
      ++i;
      continue;
    }
    std::printf("Unknown option: %s\n", argv[i]);
    return 1;
  }

  if (!hasCooler) {
    std::printf("Missing required --cooler\n");
    return 1;
  }

  std::vector<NvPhysicalGpuHandle> handles;
  std::vector<NvU32> indices;
  if (!CollectGpus(hasIndex, index, handles, indices)) { return 1; }

  for (size_t i = 0; i < handles.size(); ++i) {
    PrintGpuHeader(indices[i], handles[i]);
    NV_GPU_COOLER_POLICY_TABLE table = {};
    table.version = NV_GPU_COOLER_POLICY_TABLE_VER;
    table.policy = policy;
    NvU32 count = 0;
    NvAPI_Status status = NvAPI_GPU_GetCoolerPolicyTable(handles[i], coolerIndex, &table, &count);
    if (status != NVAPI_OK) {
      PrintNvapiError("  NvAPI_GPU_GetCoolerPolicyTable failed", status);
      continue;
    }
    std::printf("  Cooler policy table: cooler=%u policy=%s(%u) count=%u\n", coolerIndex, CoolerPolicyName(policy),
                static_cast<unsigned>(policy), count);
    const NvU32 maxCount = (count > NVAPI_MAX_COOLER_LEVELS) ? NVAPI_MAX_COOLER_LEVELS : count;
    for (NvU32 level = 0; level < maxCount; ++level) {
      const auto &entry = table.policyCoolerLevel[level];
      std::printf("    levelId=%u current=%u default=%u\n", entry.levelId, entry.currentLevel, entry.defaultLevel);
    }
  }

  return 0;
}

int CmdGpuCoolerPolicySet(int argc, char **argv) {
  NvU32 index = 0;
  bool hasIndex = false;
  NvU32 coolerIndex = 0;
  bool hasCooler = false;
  NvU32 levelId = 0;
  bool hasLevelId = false;
  NvU32 level = 0;
  bool hasLevel = false;
  NV_COOLER_POLICY policy = NVAPI_COOLER_POLICY_PERF;

  for (int i = 0; i < argc; ++i) {
    if (std::strcmp(argv[i], "--index") == 0) {
      if (i + 1 >= argc) {
        std::printf("Missing value for --index\n");
        return 1;
      }
      if (!ParseUint(argv[i + 1], &index)) {
        std::printf("Invalid GPU index: %s\n", argv[i + 1]);
        return 1;
      }
      hasIndex = true;
      ++i;
      continue;
    }
    if (std::strcmp(argv[i], "--cooler") == 0) {
      if (i + 1 >= argc) {
        std::printf("Missing value for --cooler\n");
        return 1;
      }
      if (!ParseUint(argv[i + 1], &coolerIndex)) {
        std::printf("Invalid cooler index: %s\n", argv[i + 1]);
        return 1;
      }
      hasCooler = true;
      ++i;
      continue;
    }
    if (std::strcmp(argv[i], "--policy") == 0) {
      if (i + 1 >= argc) {
        std::printf("Missing value for --policy\n");
        return 1;
      }
      if (!ParseCoolerPolicy(argv[i + 1], &policy)) {
        std::printf("Invalid cooler policy: %s\n", argv[i + 1]);
        return 1;
      }
      ++i;
      continue;
    }
    if (std::strcmp(argv[i], "--level-id") == 0) {
      if (i + 1 >= argc) {
        std::printf("Missing value for --level-id\n");
        return 1;
      }
      if (!ParseUint(argv[i + 1], &levelId)) {
        std::printf("Invalid level id: %s\n", argv[i + 1]);
        return 1;
      }
      hasLevelId = true;
      ++i;
      continue;
    }
    if (std::strcmp(argv[i], "--level") == 0) {
      if (i + 1 >= argc) {
        std::printf("Missing value for --level\n");
        return 1;
      }
      if (!ParseUint(argv[i + 1], &level)) {
        std::printf("Invalid level: %s\n", argv[i + 1]);
        return 1;
      }
      hasLevel = true;
      ++i;
      continue;
    }
    std::printf("Unknown option: %s\n", argv[i]);
    return 1;
  }

  if (!hasCooler || !hasLevelId || !hasLevel) {
    std::printf("Missing required --cooler, --level-id, and/or --level\n");
    return 1;
  }

  std::vector<NvPhysicalGpuHandle> handles;
  std::vector<NvU32> indices;
  if (!CollectGpus(hasIndex, index, handles, indices)) { return 1; }

  for (size_t i = 0; i < handles.size(); ++i) {
    PrintGpuHeader(indices[i], handles[i]);
    NV_GPU_COOLER_POLICY_TABLE table = {};
    table.version = NV_GPU_COOLER_POLICY_TABLE_VER;
    table.policy = policy;
    NvU32 count = 0;
    NvAPI_Status status = NvAPI_GPU_GetCoolerPolicyTable(handles[i], coolerIndex, &table, &count);
    if (status != NVAPI_OK) {
      PrintNvapiError("  NvAPI_GPU_GetCoolerPolicyTable failed", status);
      continue;
    }

    bool found = false;
    const NvU32 maxCount = (count > NVAPI_MAX_COOLER_LEVELS) ? NVAPI_MAX_COOLER_LEVELS : count;
    for (NvU32 entry = 0; entry < maxCount; ++entry) {
      if (table.policyCoolerLevel[entry].levelId == levelId) {
        table.policyCoolerLevel[entry].currentLevel = level;
        found = true;
        break;
      }
    }
    if (!found) {
      std::printf("  Level id %u not found in policy table.\n", levelId);
      continue;
    }

    status = NvAPI_GPU_SetCoolerPolicyTable(handles[i], coolerIndex, &table, count);
    if (status != NVAPI_OK) {
      PrintNvapiError("  NvAPI_GPU_SetCoolerPolicyTable failed", status);
      continue;
    }

    std::printf("  Cooler policy updated: cooler=%u levelId=%u level=%u policy=%s(%u)\n", coolerIndex, levelId, level,
                CoolerPolicyName(policy), static_cast<unsigned>(policy));
  }

  return 0;
}

int CmdGpuCoolerPolicyRestore(int argc, char **argv) {
  NvU32 index = 0;
  bool hasIndex = false;
  NvU32 coolerIndex = 0;
  bool hasCooler = false;
  NV_COOLER_POLICY policy = NVAPI_COOLER_POLICY_PERF;

  for (int i = 0; i < argc; ++i) {
    if (std::strcmp(argv[i], "--index") == 0) {
      if (i + 1 >= argc) {
        std::printf("Missing value for --index\n");
        return 1;
      }
      if (!ParseUint(argv[i + 1], &index)) {
        std::printf("Invalid GPU index: %s\n", argv[i + 1]);
        return 1;
      }
      hasIndex = true;
      ++i;
      continue;
    }
    if (std::strcmp(argv[i], "--cooler") == 0) {
      if (i + 1 >= argc) {
        std::printf("Missing value for --cooler\n");
        return 1;
      }
      if (!ParseUint(argv[i + 1], &coolerIndex)) {
        std::printf("Invalid cooler index: %s\n", argv[i + 1]);
        return 1;
      }
      hasCooler = true;
      ++i;
      continue;
    }
    if (std::strcmp(argv[i], "--policy") == 0) {
      if (i + 1 >= argc) {
        std::printf("Missing value for --policy\n");
        return 1;
      }
      if (!ParseCoolerPolicy(argv[i + 1], &policy)) {
        std::printf("Invalid cooler policy: %s\n", argv[i + 1]);
        return 1;
      }
      ++i;
      continue;
    }
    std::printf("Unknown option: %s\n", argv[i]);
    return 1;
  }

  std::vector<NvPhysicalGpuHandle> handles;
  std::vector<NvU32> indices;
  if (!CollectGpus(hasIndex, index, handles, indices)) { return 1; }

  for (size_t i = 0; i < handles.size(); ++i) {
    PrintGpuHeader(indices[i], handles[i]);
    NvAPI_Status status = NVAPI_OK;
    if (hasCooler) {
      NvU32 cooler = coolerIndex;
      status = NvAPI_GPU_RestoreCoolerPolicyTable(handles[i], &cooler, 1, policy);
    } else {
      status = NvAPI_GPU_RestoreCoolerPolicyTable(handles[i], NULL, 0, policy);
    }
    if (status != NVAPI_OK) {
      PrintNvapiError("  NvAPI_GPU_RestoreCoolerPolicyTable failed", status);
      continue;
    }
    std::printf("  Cooler policy restored: policy=%s(%u)%s\n", CoolerPolicyName(policy), static_cast<unsigned>(policy),
                hasCooler ? " (cooler specified)" : "");
  }

  return 0;
}

int CmdGpuCoolerPolicy(int argc, char **argv) {
  if (argc < 1) {
    std::printf("Missing cooler-policy subcommand.\n");
    return 1;
  }
  if (std::strcmp(argv[0], "get") == 0) { return CmdGpuCoolerPolicyGet(argc - 1, argv + 1); }
  if (std::strcmp(argv[0], "set") == 0) { return CmdGpuCoolerPolicySet(argc - 1, argv + 1); }
  if (std::strcmp(argv[0], "restore") == 0) { return CmdGpuCoolerPolicyRestore(argc - 1, argv + 1); }
  std::printf("Unknown cooler-policy subcommand: %s\n", argv[0]);
  return 1;
}
int CmdGpuBar(int argc, char **argv) {
  NvU32 index = 0;
  bool hasIndex = false;
  if (!ParseGpuIndexArgs(argc, argv, &index, &hasIndex)) { return 1; }

  std::vector<NvPhysicalGpuHandle> handles;
  std::vector<NvU32> indices;
  if (!CollectGpus(hasIndex, index, handles, indices)) { return 1; }

  for (size_t i = 0; i < handles.size(); ++i) {
    PrintGpuHeader(indices[i], handles[i]);
    PrintBarInfo(handles[i]);
  }
  return 0;
}

int CmdGpuPower(int argc, char **argv) {
  NvU32 index = 0;
  bool hasIndex = false;
  if (!ParseGpuIndexArgs(argc, argv, &index, &hasIndex)) { return 1; }

  std::vector<NvPhysicalGpuHandle> handles;
  std::vector<NvU32> indices;
  if (!CollectGpus(hasIndex, index, handles, indices)) { return 1; }

  for (size_t i = 0; i < handles.size(); ++i) {
    PrintGpuHeader(indices[i], handles[i]);
    PrintPowerInfo(handles[i]);
  }
  return 0;
}

int CmdGpuList() {
  NvPhysicalGpuHandle gpus[NVAPI_MAX_PHYSICAL_GPUS] = {};
  NvU32 gpuCount = 0;
  NvAPI_Status status = NvAPI_EnumPhysicalGPUs(gpus, &gpuCount);
  if (status != NVAPI_OK) {
    PrintNvapiError("NvAPI_EnumPhysicalGPUs failed", status);
    return 1;
  }

  std::printf("Physical GPUs: %u\n", gpuCount);
  for (NvU32 i = 0; i < gpuCount; ++i) {
    NvAPI_ShortString name = {0};
    status = NvAPI_GPU_GetFullName(gpus[i], name);
    if (status != NVAPI_OK) {
      std::printf("  [%u] <name unavailable>\n", i);
      continue;
    }

    NvU32 deviceId = 0;
    NvU32 subSystemId = 0;
    NvU32 revisionId = 0;
    NvU32 extDeviceId = 0;
    status = NvAPI_GPU_GetPCIIdentifiers(gpus[i], &deviceId, &subSystemId, &revisionId, &extDeviceId);

    std::printf("  [%u] %s\n", i, name);
    if (status == NVAPI_OK) {
      std::printf("       PCI: device=0x%04X subsystem=0x%08X revision=0x%02X ext=0x%08X\n", deviceId, subSystemId,
                  revisionId, extDeviceId);
    }
  }
  return 0;
}

int CmdGpuThermal(int argc, char **argv) {
  NvU32 index = 0;
  bool hasIndex = false;
  if (!ParseGpuIndexArgs(argc, argv, &index, &hasIndex)) { return 1; }

  std::vector<NvPhysicalGpuHandle> handles;
  std::vector<NvU32> indices;
  if (!CollectGpus(hasIndex, index, handles, indices)) { return 1; }

  for (size_t i = 0; i < handles.size(); ++i) {
    NvAPI_ShortString name = {0};
    NvAPI_GPU_GetFullName(handles[i], name);

    NV_GPU_THERMAL_SETTINGS thermal = {0};
    thermal.version = NV_GPU_THERMAL_SETTINGS_VER;
    NvAPI_Status status = NvAPI_GPU_GetThermalSettings(handles[i], NVAPI_THERMAL_TARGET_ALL, &thermal);
    if (status != NVAPI_OK) {
      std::printf("GPU[%u] %s\n", indices[i], name);
      PrintNvapiError("  NvAPI_GPU_GetThermalSettings failed", status);
      continue;
    }

    std::printf("GPU[%u] %s\n", indices[i], name);
    for (NvU32 sensorIndex = 0; sensorIndex < thermal.count; ++sensorIndex) {
      const auto &sensor = thermal.sensor[sensorIndex];
      std::printf("  sensor[%u] target=%u current=%dC defaultMin=%dC defaultMax=%dC\n", sensorIndex, sensor.target,
                  sensor.currentTemp, sensor.defaultMinTemp, sensor.defaultMaxTemp);
    }
  }
  return 0;
}
} // namespace nvcli
