/*
 * Copyright (c) 2026 Noverse (Nohuto). All rights reserved.
 * Proprietary and confidential. Unauthorized copying or redistribution is strictly prohibited.
 */

#include "cli/commands.h"

namespace nvcli {
namespace {
bool ParseFloatValue(const char *value, float *out) {
  if (!value || !out) { return false; }
  char *end = NULL;
  float parsed = std::strtof(value, &end);
  if (end == value || *end != '\0') { return false; }
  *out = parsed;
  return true;
}

const char *TimingOverrideName(NV_TIMING_OVERRIDE type) {
  switch (type) {
  case NV_TIMING_OVERRIDE_CURRENT: return "CURRENT";
  case NV_TIMING_OVERRIDE_AUTO: return "AUTO";
  case NV_TIMING_OVERRIDE_EDID: return "EDID";
  case NV_TIMING_OVERRIDE_DMT: return "DMT";
  case NV_TIMING_OVERRIDE_DMT_RB: return "DMT_RB";
  case NV_TIMING_OVERRIDE_CVT: return "CVT";
  case NV_TIMING_OVERRIDE_CVT_RB: return "CVT_RB";
  case NV_TIMING_OVERRIDE_GTF: return "GTF";
  case NV_TIMING_OVERRIDE_EIA861: return "EIA861";
  case NV_TIMING_OVERRIDE_ANALOG_TV: return "ANALOG_TV";
  case NV_TIMING_OVERRIDE_CUST: return "CUSTOM";
  case NV_TIMING_OVERRIDE_NV_PREDEFINED: return "NV_PREDEFINED";
  case NV_TIMING_OVERRIDE_SDI: return "SDI";
  default: return "UNKNOWN";
  }
}

bool ParseTimingOverride(const char *value, NV_TIMING_OVERRIDE *out) {
  if (!value || !out) { return false; }
  std::string lowered = ToLowerAscii(value);
  if (lowered == "auto") {
    *out = NV_TIMING_OVERRIDE_AUTO;
    return true;
  }
  if (lowered == "edid") {
    *out = NV_TIMING_OVERRIDE_EDID;
    return true;
  }
  if (lowered == "dmt") {
    *out = NV_TIMING_OVERRIDE_DMT;
    return true;
  }
  if (lowered == "dmt-rb") {
    *out = NV_TIMING_OVERRIDE_DMT_RB;
    return true;
  }
  if (lowered == "cvt") {
    *out = NV_TIMING_OVERRIDE_CVT;
    return true;
  }
  if (lowered == "cvt-rb") {
    *out = NV_TIMING_OVERRIDE_CVT_RB;
    return true;
  }
  if (lowered == "gtf") {
    *out = NV_TIMING_OVERRIDE_GTF;
    return true;
  }
  if (lowered == "eia861") {
    *out = NV_TIMING_OVERRIDE_EIA861;
    return true;
  }
  if (lowered == "analog-tv") {
    *out = NV_TIMING_OVERRIDE_ANALOG_TV;
    return true;
  }
  if (lowered == "custom") {
    *out = NV_TIMING_OVERRIDE_CUST;
    return true;
  }
  if (lowered == "nv-predefined") {
    *out = NV_TIMING_OVERRIDE_NV_PREDEFINED;
    return true;
  }
  if (lowered == "sdi") {
    *out = NV_TIMING_OVERRIDE_SDI;
    return true;
  }
  NvU32 numeric = 0;
  if (!ParseUint(value, &numeric)) { return false; }
  *out = static_cast<NV_TIMING_OVERRIDE>(numeric);
  return true;
}

void PrintCustomDisplay(const NV_CUSTOM_DISPLAY &custom, NvU32 index) {
  double refresh = TimingRefreshHz(custom.timing);
  std::printf("  [%u] %ux%u depth=%u format=%u ratio=%.3f/%.3f hwOnly=%u\n", index, custom.width, custom.height,
              custom.depth, static_cast<unsigned>(custom.colorFormat), custom.xRatio, custom.yRatio,
              custom.hwModeSetOnly ? 1 : 0);
  std::printf("       timing: %ux%u %s pclk=%.3f MHz refresh=%.2f Hz\n", custom.timing.HVisible, custom.timing.VVisible,
              custom.timing.interlaced ? "interlaced" : "progressive", static_cast<double>(custom.timing.pclk) / 100.0,
              refresh);
}

bool BuildTiming(NvU32 displayId, NvU32 width, NvU32 height, float refresh, NV_TIMING_OVERRIDE type, bool interlaced,
                 bool hasCeaId, NvU32 ceaId, bool hasTvFormat, NvU32 tvFormat, bool hasPsfId, NvU32 psfId,
                 NV_TIMING *outTiming) {
  if (!outTiming) { return false; }

  NV_TIMING_INPUT input = {};
  input.version = NV_TIMING_INPUT_VER;
  input.flag.isInterlaced = interlaced ? 1 : 0;
  input.type = type;

  if (type == NV_TIMING_OVERRIDE_EIA861) {
    if (!hasCeaId) {
      std::printf("Missing --cea-id for timing type EIA861.\n");
      return false;
    }
    input.width = 0;
    input.height = 0;
    input.rr = 0.0f;
    input.flag.ceaId = static_cast<NvU32>(ceaId);
  } else if (type == NV_TIMING_OVERRIDE_ANALOG_TV) {
    if (!hasTvFormat) {
      std::printf("Missing --tv-format for timing type ANALOG_TV.\n");
      return false;
    }
    input.width = 0;
    input.height = 0;
    input.rr = 0.0f;
    input.flag.tvFormat = static_cast<NvU32>(tvFormat);
  } else if (type == NV_TIMING_OVERRIDE_NV_PREDEFINED) {
    if (!hasPsfId) {
      std::printf("Missing --psf-id for timing type NV_PREDEFINED.\n");
      return false;
    }
    input.width = 0;
    input.height = 0;
    input.rr = 0.0f;
    input.flag.nvPsfId = static_cast<NvU32>(psfId);
  } else {
    if (width == 0 || height == 0 || refresh <= 0.0f) {
      std::printf("Missing --width/--height/--refresh for timing calculation.\n");
      return false;
    }
    input.width = width;
    input.height = height;
    input.rr = refresh;
  }

  NvAPI_Status status = NvAPI_DISP_GetTiming(displayId, &input, outTiming);
  if (status != NVAPI_OK) {
    PrintNvapiError("NvAPI_DISP_GetTiming failed", status);
    return false;
  }

  return true;
}

bool BuildCustomDisplay(NvU32 displayId, NvU32 width, NvU32 height, float refresh, NvU32 depth, NV_TIMING_OVERRIDE type,
                        bool interlaced, bool hwOnly, bool hasCeaId, NvU32 ceaId, bool hasTvFormat, NvU32 tvFormat,
                        bool hasPsfId, NvU32 psfId, NV_CUSTOM_DISPLAY *outCustom) {
  if (!outCustom) { return false; }

  NV_TIMING timing = {};
  if (!BuildTiming(displayId, width, height, refresh, type, interlaced, hasCeaId, ceaId, hasTvFormat, tvFormat,
                   hasPsfId, psfId, &timing)) {
    return false;
  }

  if (width == 0) { width = timing.HVisible; }
  if (height == 0) { height = timing.VVisible; }

  std::memset(outCustom, 0, sizeof(*outCustom));
  outCustom->version = NV_CUSTOM_DISPLAY_VER;
  outCustom->width = width;
  outCustom->height = height;
  outCustom->depth = depth;
  outCustom->colorFormat = NV_FORMAT_UNKNOWN;
  outCustom->srcPartition.x = 0.0f;
  outCustom->srcPartition.y = 0.0f;
  outCustom->srcPartition.w = 1.0f;
  outCustom->srcPartition.h = 1.0f;
  outCustom->xRatio = 1.0f;
  outCustom->yRatio = 1.0f;
  outCustom->timing = timing;
  outCustom->hwModeSetOnly = hwOnly ? 1 : 0;
  return true;
}
} // namespace

int CmdDisplayCustomList(int argc, char **argv) {
  NvU32 displayId = 0;
  if (!ParseDisplayIdArg(argc, argv, &displayId)) { return 1; }

  NvU32 index = 0;
  bool any = false;
  while (true) {
    NV_CUSTOM_DISPLAY custom = {};
    custom.version = NV_CUSTOM_DISPLAY_VER;
    NvAPI_Status status = NvAPI_DISP_EnumCustomDisplay(displayId, index, &custom);
    if (status == NVAPI_END_ENUMERATION) { break; }
    if (status != NVAPI_OK) {
      PrintNvapiError("NvAPI_DISP_EnumCustomDisplay failed", status);
      return 1;
    }

    if (!any) { std::printf("Custom displays for 0x%08X:\n", displayId); }
    any = true;
    PrintCustomDisplay(custom, index);
    ++index;
  }

  if (!any) { std::printf("No custom displays found for 0x%08X.\n", displayId); }
  return 0;
}

int CmdDisplayCustomTry(int argc, char **argv) {
  NvU32 displayId = 0;
  bool hasDisplayId = false;
  NvU32 width = 0;
  NvU32 height = 0;
  float refresh = 0.0f;
  NvU32 depth = 0;
  NV_TIMING_OVERRIDE type = NV_TIMING_OVERRIDE_CVT_RB;
  bool interlaced = false;
  bool hwOnly = false;
  bool hasCeaId = false;
  NvU32 ceaId = 0;
  bool hasTvFormat = false;
  NvU32 tvFormat = 0;
  bool hasPsfId = false;
  NvU32 psfId = 0;

  for (int i = 0; i < argc; ++i) {
    if (std::strcmp(argv[i], "--id") == 0) {
      if (i + 1 >= argc || !ParseUint(argv[i + 1], &displayId)) {
        std::printf("Invalid display id.\n");
        return 1;
      }
      hasDisplayId = true;
      ++i;
      continue;
    }
    if (std::strcmp(argv[i], "--width") == 0) {
      if (i + 1 >= argc || !ParseUint(argv[i + 1], &width)) {
        std::printf("Invalid --width value.\n");
        return 1;
      }
      ++i;
      continue;
    }
    if (std::strcmp(argv[i], "--height") == 0) {
      if (i + 1 >= argc || !ParseUint(argv[i + 1], &height)) {
        std::printf("Invalid --height value.\n");
        return 1;
      }
      ++i;
      continue;
    }
    if (std::strcmp(argv[i], "--refresh") == 0) {
      if (i + 1 >= argc || !ParseFloatValue(argv[i + 1], &refresh)) {
        std::printf("Invalid --refresh value.\n");
        return 1;
      }
      ++i;
      continue;
    }
    if (std::strcmp(argv[i], "--depth") == 0) {
      if (i + 1 >= argc || !ParseUint(argv[i + 1], &depth)) {
        std::printf("Invalid --depth value.\n");
        return 1;
      }
      ++i;
      continue;
    }
    if (std::strcmp(argv[i], "--type") == 0) {
      if (i + 1 >= argc || !ParseTimingOverride(argv[i + 1], &type)) {
        std::printf("Invalid --type value.\n");
        return 1;
      }
      ++i;
      continue;
    }
    if (std::strcmp(argv[i], "--interlaced") == 0) {
      if (i + 1 >= argc || !ParseBoolValue(argv[i + 1], &interlaced)) {
        std::printf("Invalid --interlaced value.\n");
        return 1;
      }
      ++i;
      continue;
    }
    if (std::strcmp(argv[i], "--hw-only") == 0) {
      if (i + 1 >= argc || !ParseBoolValue(argv[i + 1], &hwOnly)) {
        std::printf("Invalid --hw-only value.\n");
        return 1;
      }
      ++i;
      continue;
    }
    if (std::strcmp(argv[i], "--cea-id") == 0) {
      if (i + 1 >= argc || !ParseUint(argv[i + 1], &ceaId)) {
        std::printf("Invalid --cea-id value.\n");
        return 1;
      }
      hasCeaId = true;
      ++i;
      continue;
    }
    if (std::strcmp(argv[i], "--tv-format") == 0) {
      if (i + 1 >= argc || !ParseUint(argv[i + 1], &tvFormat)) {
        std::printf("Invalid --tv-format value.\n");
        return 1;
      }
      hasTvFormat = true;
      ++i;
      continue;
    }
    if (std::strcmp(argv[i], "--psf-id") == 0) {
      if (i + 1 >= argc || !ParseUint(argv[i + 1], &psfId)) {
        std::printf("Invalid --psf-id value.\n");
        return 1;
      }
      hasPsfId = true;
      ++i;
      continue;
    }
    std::printf("Unknown option: %s\n", argv[i]);
    return 1;
  }

  if (!hasDisplayId) {
    std::printf("Missing required --id\n");
    return 1;
  }

  if (type == NV_TIMING_OVERRIDE_CUST) {
    std::printf("NV_TIMING_OVERRIDE_CUST requires explicit timing fields and is not supported by this command.\n");
    return 1;
  }

  NV_CUSTOM_DISPLAY custom = {};
  if (!BuildCustomDisplay(displayId, width, height, refresh, depth, type, interlaced, hwOnly, hasCeaId, ceaId,
                          hasTvFormat, tvFormat, hasPsfId, psfId, &custom)) {
    return 1;
  }

  double actualRefresh = TimingRefreshHz(custom.timing);
  std::printf("Custom display try: %ux%u@%.3fHz depth=%u type=%s interlaced=%u hwOnly=%u\n", custom.width,
              custom.height, actualRefresh, depth, TimingOverrideName(type), interlaced ? 1 : 0, hwOnly ? 1 : 0);

  NvU32 displayIds[1] = {displayId};
  NvAPI_Status status = NvAPI_DISP_TryCustomDisplay(displayIds, 1, &custom);
  if (status != NVAPI_OK) {
    PrintNvapiError("NvAPI_DISP_TryCustomDisplay failed", status);
    return 1;
  }

  std::printf("Custom display applied (trial).\n");
  return 0;
}

int CmdDisplayCustomSave(int argc, char **argv) {
  NvU32 displayId = 0;
  bool hasDisplayId = false;
  bool outputOnly = false;
  bool monitorOnly = false;

  for (int i = 0; i < argc; ++i) {
    if (std::strcmp(argv[i], "--id") == 0) {
      if (i + 1 >= argc || !ParseUint(argv[i + 1], &displayId)) {
        std::printf("Invalid display id.\n");
        return 1;
      }
      hasDisplayId = true;
      ++i;
      continue;
    }
    if (std::strcmp(argv[i], "--output-only") == 0) {
      if (i + 1 >= argc || !ParseBoolValue(argv[i + 1], &outputOnly)) {
        std::printf("Invalid --output-only value.\n");
        return 1;
      }
      ++i;
      continue;
    }
    if (std::strcmp(argv[i], "--monitor-only") == 0) {
      if (i + 1 >= argc || !ParseBoolValue(argv[i + 1], &monitorOnly)) {
        std::printf("Invalid --monitor-only value.\n");
        return 1;
      }
      ++i;
      continue;
    }
    std::printf("Unknown option: %s\n", argv[i]);
    return 1;
  }

  if (!hasDisplayId) {
    std::printf("Missing required --id\n");
    return 1;
  }

  std::printf("Custom display save: outputOnly=%u monitorOnly=%u\n", outputOnly ? 1 : 0, monitorOnly ? 1 : 0);

  NvU32 displayIds[1] = {displayId};
  NvAPI_Status status = NvAPI_DISP_SaveCustomDisplay(displayIds, 1, outputOnly ? 1 : 0, monitorOnly ? 1 : 0);
  if (status != NVAPI_OK) {
    PrintNvapiError("NvAPI_DISP_SaveCustomDisplay failed", status);
    return 1;
  }

  std::printf("Custom display saved.\n");
  return 0;
}

int CmdDisplayCustomDelete(int argc, char **argv) {
  NvU32 displayId = 0;
  bool hasDisplayId = false;
  NvU32 index = 0;
  bool hasIndex = false;

  for (int i = 0; i < argc; ++i) {
    if (std::strcmp(argv[i], "--id") == 0) {
      if (i + 1 >= argc || !ParseUint(argv[i + 1], &displayId)) {
        std::printf("Invalid display id.\n");
        return 1;
      }
      hasDisplayId = true;
      ++i;
      continue;
    }
    if (std::strcmp(argv[i], "--index") == 0) {
      if (i + 1 >= argc || !ParseUint(argv[i + 1], &index)) {
        std::printf("Invalid --index value.\n");
        return 1;
      }
      hasIndex = true;
      ++i;
      continue;
    }
    std::printf("Unknown option: %s\n", argv[i]);
    return 1;
  }

  if (!hasDisplayId || !hasIndex) {
    std::printf("Missing required --id and --index\n");
    return 1;
  }

  NV_CUSTOM_DISPLAY custom = {};
  custom.version = NV_CUSTOM_DISPLAY_VER;
  NvAPI_Status status = NvAPI_DISP_EnumCustomDisplay(displayId, index, &custom);
  if (status != NVAPI_OK) {
    PrintNvapiError("NvAPI_DISP_EnumCustomDisplay failed", status);
    return 1;
  }

  std::printf("Custom display delete index=%u:\n", index);
  PrintCustomDisplay(custom, index);

  NvU32 displayIds[1] = {displayId};
  status = NvAPI_DISP_DeleteCustomDisplay(displayIds, 1, &custom);
  if (status != NVAPI_OK) {
    PrintNvapiError("NvAPI_DISP_DeleteCustomDisplay failed", status);
    return 1;
  }

  std::printf("Custom display deleted.\n");
  return 0;
}

int CmdDisplayCustomRevert(int argc, char **argv) {
  NvU32 displayId = 0;
  bool hasDisplayId = false;

  for (int i = 0; i < argc; ++i) {
    if (std::strcmp(argv[i], "--id") == 0) {
      if (i + 1 >= argc || !ParseUint(argv[i + 1], &displayId)) {
        std::printf("Invalid display id.\n");
        return 1;
      }
      hasDisplayId = true;
      ++i;
      continue;
    }
    std::printf("Unknown option: %s\n", argv[i]);
    return 1;
  }

  if (!hasDisplayId) {
    std::printf("Missing required --id\n");
    return 1;
  }

  std::printf("Custom display revert trial for 0x%08X.\n", displayId);

  NvU32 displayIds[1] = {displayId};
  NvAPI_Status status = NvAPI_DISP_RevertCustomDisplayTrial(displayIds, 1);
  if (status != NVAPI_OK) {
    PrintNvapiError("NvAPI_DISP_RevertCustomDisplayTrial failed", status);
    return 1;
  }

  std::printf("Custom display trial reverted.\n");
  return 0;
}

int CmdDisplayCustom(int argc, char **argv) {
  if (argc < 1) {
    PrintUsageGroup("display");
    return 1;
  }

  if (std::strcmp(argv[0], "list") == 0) { return CmdDisplayCustomList(argc - 1, argv + 1); }
  if (std::strcmp(argv[0], "try") == 0) { return CmdDisplayCustomTry(argc - 1, argv + 1); }
  if (std::strcmp(argv[0], "save") == 0) { return CmdDisplayCustomSave(argc - 1, argv + 1); }
  if (std::strcmp(argv[0], "delete") == 0) { return CmdDisplayCustomDelete(argc - 1, argv + 1); }
  if (std::strcmp(argv[0], "revert") == 0) { return CmdDisplayCustomRevert(argc - 1, argv + 1); }

  std::printf("Unknown display custom subcommand: %s\n", argv[0]);
  return 1;
}
} // namespace nvcli
