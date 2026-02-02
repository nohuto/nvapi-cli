/*
 * Copyright (c) 2026 Noverse (Nohuto). All rights reserved.
 * Proprietary and confidential. Unauthorized copying or redistribution is strictly prohibited.
 */

#include "cli/commands.h"

namespace nvcli {
namespace {
bool ParseDoubleValue(const char *value, double *out) {
  if (!value || !out) { return false; }
  char *end = NULL;
  double parsed = std::strtod(value, &end);
  if (end == value || *end != '\0') { return false; }
  *out = parsed;
  return true;
}

const char *MonitorCapsTypeName(NV_MONITOR_CAPS_TYPE type) {
  switch (type) {
  case NV_MONITOR_CAPS_TYPE_HDMI_VSDB: return "HDMI_VSDB";
  case NV_MONITOR_CAPS_TYPE_HDMI_VCDB: return "HDMI_VCDB";
  case NV_MONITOR_CAPS_TYPE_GENERIC: return "GENERIC";
  default: return "UNKNOWN";
  }
}

bool ParseMonitorCapsType(const char *value, NV_MONITOR_CAPS_TYPE *out) {
  if (!value || !out) { return false; }
  std::string lowered = ToLowerAscii(value);
  if (lowered == "hdmi-vsdb") {
    *out = NV_MONITOR_CAPS_TYPE_HDMI_VSDB;
    return true;
  }
  if (lowered == "hdmi-vcdb") {
    *out = NV_MONITOR_CAPS_TYPE_HDMI_VCDB;
    return true;
  }
  if (lowered == "generic") {
    *out = NV_MONITOR_CAPS_TYPE_GENERIC;
    return true;
  }
  NvU32 numeric = 0;
  if (!ParseUint(value, &numeric)) { return false; }
  *out = static_cast<NV_MONITOR_CAPS_TYPE>(numeric);
  return true;
}

const char *DpColorFormatName(NV_DP_COLOR_FORMAT format) {
  switch (format) {
  case NV_DP_COLOR_FORMAT_RGB: return "RGB";
  case NV_DP_COLOR_FORMAT_YCbCr422: return "YCbCr422";
  case NV_DP_COLOR_FORMAT_YCbCr444: return "YCbCr444";
  default: return "UNKNOWN";
  }
}

const char *DpBpcName(NV_DP_BPC bpc) {
  switch (bpc) {
  case NV_DP_BPC_DEFAULT: return "DEFAULT";
  case NV_DP_BPC_6: return "6";
  case NV_DP_BPC_8: return "8";
  case NV_DP_BPC_10: return "10";
  case NV_DP_BPC_12: return "12";
  case NV_DP_BPC_16: return "16";
  default: return "UNKNOWN";
  }
}

const char *BpcName(NV_BPC bpc) {
  switch (bpc) {
  case NV_BPC_DEFAULT: return "default";
  case NV_BPC_6: return "6";
  case NV_BPC_8: return "8";
  case NV_BPC_10: return "10";
  case NV_BPC_12: return "12";
  case NV_BPC_16: return "16";
  default: return "unknown";
  }
}

bool ParseBpcValue(const char *value, NvU32 *out) {
  if (!value || !out) { return false; }
  std::string lowered = ToLowerAscii(value);
  if (lowered == "default") {
    *out = static_cast<NvU32>(NV_BPC_DEFAULT);
    return true;
  }
  if (lowered == "6") {
    *out = static_cast<NvU32>(NV_BPC_6);
    return true;
  }
  if (lowered == "8") {
    *out = static_cast<NvU32>(NV_BPC_8);
    return true;
  }
  if (lowered == "10") {
    *out = static_cast<NvU32>(NV_BPC_10);
    return true;
  }
  if (lowered == "12") {
    *out = static_cast<NvU32>(NV_BPC_12);
    return true;
  }
  if (lowered == "16") {
    *out = static_cast<NvU32>(NV_BPC_16);
    return true;
  }
  return false;
}

const char *ScalingName(NV_SCALING scaling) {
  switch (scaling) {
  case NV_SCALING_DEFAULT: return "DEFAULT";
  case NV_SCALING_GPU_SCALING_TO_CLOSEST: return "GPU_SCALING_TO_CLOSEST";
  case NV_SCALING_GPU_SCALING_TO_NATIVE: return "GPU_SCALING_TO_NATIVE";
  case NV_SCALING_GPU_SCANOUT_TO_NATIVE: return "GPU_SCANOUT_TO_NATIVE";
  case NV_SCALING_GPU_SCALING_TO_ASPECT_SCANOUT_TO_NATIVE: return "GPU_ASPECT_TO_NATIVE";
  case NV_SCALING_GPU_SCALING_TO_ASPECT_SCANOUT_TO_CLOSEST: return "GPU_ASPECT_TO_CLOSEST";
  case NV_SCALING_GPU_SCANOUT_TO_CLOSEST: return "GPU_SCANOUT_TO_CLOSEST";
  case NV_SCALING_GPU_INTEGER_ASPECT_SCALING: return "GPU_INTEGER_ASPECT";
  case NV_SCALING_CUSTOMIZED: return "CUSTOM";
  default: return "UNKNOWN";
  }
}

bool ParseScalingMode(const char *value, NV_SCALING *out) {
  if (!value || !out) { return false; }
  std::string lowered = ToLowerAscii(value);
  if (lowered == "default") {
    *out = NV_SCALING_DEFAULT;
    return true;
  }
  if (lowered == "closest") {
    *out = NV_SCALING_GPU_SCALING_TO_CLOSEST;
    return true;
  }
  if (lowered == "native") {
    *out = NV_SCALING_GPU_SCALING_TO_NATIVE;
    return true;
  }
  if (lowered == "centered") {
    *out = NV_SCALING_GPU_SCANOUT_TO_NATIVE;
    return true;
  }
  if (lowered == "aspect") {
    *out = NV_SCALING_GPU_SCALING_TO_ASPECT_SCANOUT_TO_NATIVE;
    return true;
  }
  if (lowered == "aspect-closest") {
    *out = NV_SCALING_GPU_SCALING_TO_ASPECT_SCANOUT_TO_CLOSEST;
    return true;
  }
  if (lowered == "centered-closest") {
    *out = NV_SCALING_GPU_SCANOUT_TO_CLOSEST;
    return true;
  }
  if (lowered == "integer") {
    *out = NV_SCALING_GPU_INTEGER_ASPECT_SCALING;
    return true;
  }
  if (lowered == "custom") {
    *out = NV_SCALING_CUSTOMIZED;
    return true;
  }
  NvU32 numeric = 0;
  if (!ParseUint(value, &numeric)) { return false; }
  *out = static_cast<NV_SCALING>(numeric);
  return true;
}

const char *WideColorRangeName(NV_WIDE_COLOR_RANGE range) {
  switch (range) {
  case NV_COLOR_xvYCC: return "xvYCC";
  case NV_COLOR_UNKNOWN: return "UNKNOWN";
  default: return "UNKNOWN";
  }
}

bool ParseWideColorRange(const char *value, NV_WIDE_COLOR_RANGE *out) {
  if (!value || !out) { return false; }
  std::string lowered = ToLowerAscii(value);
  if (lowered == "xvycc") {
    *out = NV_COLOR_xvYCC;
    return true;
  }
  if (lowered == "unknown") {
    *out = NV_COLOR_UNKNOWN;
    return true;
  }
  NvU32 numeric = 0;
  if (!ParseUint(value, &numeric)) { return false; }
  *out = static_cast<NV_WIDE_COLOR_RANGE>(numeric);
  return true;
}

void PrintScalingCapsV1(const NV_GET_SCALING_CAPS_V1 &caps) {
  std::printf("Scaling caps (v1):\n");
  std::printf("  gpuScaling=%u gpuAspect=%u monitorScaling=%u centerScaling=%u\n", caps.isGPUScalingAvailable ? 1 : 0,
              caps.isGPUFixedAspectRatioScalingAvailable ? 1 : 0, caps.isMonitorScalingAvailable ? 1 : 0,
              caps.isCenterScalingAvailable ? 1 : 0);
}

void PrintScalingCapsV2(const NV_GET_SCALING_CAPS_V2 &caps) {
  std::printf("Scaling caps (v2):\n");
  std::printf("  gpuFull=%u gpuAspect=%u monitorFull=%u center=%u\n", caps.isGPUScalingAvailable ? 1 : 0,
              caps.isGPUFixedAspectRatioScalingAvailable ? 1 : 0, caps.isMonitorScalingAvailable ? 1 : 0,
              caps.isCenterScalingAvailable ? 1 : 0);
  std::printf("  gpuAspectClosest=%u gpuCenterClosest=%u gpuInteger=%u\n",
              caps.isGPUScalingToAspectScanoutToClosest ? 1 : 0, caps.isGPUScanoutToClosest ? 1 : 0,
              caps.isGpuRepeatToAspectScanoutToNative ? 1 : 0);
  std::printf("  default=%s current=%s\n", ScalingName(caps.defaultScaling), ScalingName(caps.currentScaling));
}

bool ApplyForcedScalingCapsV1(NV_GET_SCALING_CAPS_V1 &caps, NV_SCALING preferred) {
  std::memset(&caps, 0, sizeof(caps));
  switch (preferred) {
  case NV_SCALING_GPU_SCALING_TO_CLOSEST: caps.isMonitorScalingAvailable = 1; return true;
  case NV_SCALING_GPU_SCALING_TO_NATIVE: caps.isGPUScalingAvailable = 1; return true;
  case NV_SCALING_GPU_SCANOUT_TO_NATIVE: caps.isCenterScalingAvailable = 1; return true;
  case NV_SCALING_GPU_SCALING_TO_ASPECT_SCANOUT_TO_NATIVE: caps.isGPUFixedAspectRatioScalingAvailable = 1; return true;
  case NV_SCALING_GPU_SCALING_TO_ASPECT_SCANOUT_TO_CLOSEST: caps.isMonitorScalingAvailable = 1; return true;
  case NV_SCALING_GPU_SCANOUT_TO_CLOSEST: caps.isCenterScalingAvailable = 1; return true;
  default: return false;
  }
}

bool ApplyForcedScalingCapsV2(NV_GET_SCALING_CAPS_V2 &caps, NV_SCALING preferred) {
  std::memset(&caps, 0, sizeof(caps));
  caps.defaultScaling = preferred;
  caps.currentScaling = preferred;
  switch (preferred) {
  case NV_SCALING_GPU_SCALING_TO_CLOSEST: caps.isMonitorScalingAvailable = 1; return true;
  case NV_SCALING_GPU_SCALING_TO_NATIVE: caps.isGPUScalingAvailable = 1; return true;
  case NV_SCALING_GPU_SCANOUT_TO_NATIVE: caps.isCenterScalingAvailable = 1; return true;
  case NV_SCALING_GPU_SCALING_TO_ASPECT_SCANOUT_TO_NATIVE: caps.isGPUFixedAspectRatioScalingAvailable = 1; return true;
  case NV_SCALING_GPU_SCALING_TO_ASPECT_SCANOUT_TO_CLOSEST: caps.isGPUScalingToAspectScanoutToClosest = 1; return true;
  case NV_SCALING_GPU_SCANOUT_TO_CLOSEST: caps.isGPUScanoutToClosest = 1; return true;
  case NV_SCALING_GPU_INTEGER_ASPECT_SCALING: caps.isGpuRepeatToAspectScanoutToNative = 1; return true;
  default: return false;
  }
}
} // namespace

int CmdDisplayMonitorCaps(int argc, char **argv) {
  NvU32 displayId = 0;
  bool hasDisplayId = false;
  bool hasType = false;
  bool allTypes = false;
  NV_MONITOR_CAPS_TYPE type = NV_MONITOR_CAPS_TYPE_GENERIC;

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
    if (std::strcmp(argv[i], "--type") == 0) {
      if (i + 1 >= argc) {
        std::printf("Missing value for --type\n");
        return 1;
      }
      std::string lowered = ToLowerAscii(argv[i + 1]);
      if (lowered == "all") {
        allTypes = true;
        hasType = true;
      } else if (!ParseMonitorCapsType(argv[i + 1], &type)) {
        std::printf("Invalid monitor caps type: %s\n", argv[i + 1]);
        return 1;
      } else {
        hasType = true;
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

  std::vector<NV_MONITOR_CAPS_TYPE> types;
  if (!hasType || allTypes) {
    types.push_back(NV_MONITOR_CAPS_TYPE_GENERIC);
    types.push_back(NV_MONITOR_CAPS_TYPE_HDMI_VSDB);
    types.push_back(NV_MONITOR_CAPS_TYPE_HDMI_VCDB);
  } else {
    types.push_back(type);
  }

  for (size_t i = 0; i < types.size(); ++i) {
    NV_MONITOR_CAPABILITIES caps = {};
    caps.version = NV_MONITOR_CAPABILITIES_VER;
    caps.size = static_cast<NvU16>(sizeof(caps));
    caps.infoType = types[i];

    NvAPI_Status status = NvAPI_DISP_GetMonitorCapabilities(displayId, &caps);
    if (status != NVAPI_OK) {
      PrintNvapiError("NvAPI_DISP_GetMonitorCapabilities failed", status);
      continue;
    }

    std::printf("Monitor caps type=%s connector=%s valid=%u\n",
                MonitorCapsTypeName(static_cast<NV_MONITOR_CAPS_TYPE>(caps.infoType)),
                ConnectorTypeName(static_cast<NV_MONITOR_CONN_TYPE>(caps.connectorType)), caps.bIsValidInfo ? 1 : 0);

    if (!caps.bIsValidInfo) { continue; }

    if (caps.infoType == NV_MONITOR_CAPS_TYPE_GENERIC) {
      const NV_MONITOR_CAPS_GENERIC &info = caps.data.caps;
      std::printf("  vrr=%u ulmb=%u trueGsync=%u rla=%u\n", info.supportVRR ? 1 : 0, info.supportULMB ? 1 : 0,
                  info.isTrueGsync ? 1 : 0, info.isRLACapable ? 1 : 0);
    } else if (caps.infoType == NV_MONITOR_CAPS_TYPE_HDMI_VSDB) {
      const NV_MONITOR_CAPS_VSDB &info = caps.data.vsdb;
      std::printf("  physAddr=%u.%u.%u.%u maxTmdsClock=%u\n", info.sourcePhysicalAddressA, info.sourcePhysicalAddressB,
                  info.sourcePhysicalAddressC, info.sourcePhysicalAddressD, info.maxTmdsClock);
      std::printf("  deepColor: ycbcr444=%u 30=%u 36=%u 48=%u ai=%u dualDvi=%u\n",
                  info.supportDeepColorYCbCr444 ? 1 : 0, info.supportDeepColor30bits ? 1 : 0,
                  info.supportDeepColor36bits ? 1 : 0, info.supportDeepColor48bits ? 1 : 0, info.supportAI ? 1 : 0,
                  info.supportDualDviOperation ? 1 : 0);
      std::printf("  cnc: graphics=%u photo=%u cinema=%u game=%u\n", info.cnc0SupportGraphicsTextContent ? 1 : 0,
                  info.cnc1SupportPhotoContent ? 1 : 0, info.cnc2SupportCinemaContent ? 1 : 0,
                  info.cnc3SupportGameContent ? 1 : 0);
      std::printf("  latency: has=%u interlaced=%u video=%u audio=%u\n", info.hasLatencyField ? 1 : 0,
                  info.hasInterlacedLatencyField ? 1 : 0, info.videoLatency, info.audioLatency);
      std::printf("  latencyInterlaced: video=%u audio=%u\n", info.interlacedVideoLatency, info.interlacedAudioLatency);
      std::printf("  vic: has=%u len=%u 3d: has=%u len=%u\n", info.hasVicEntries ? 1 : 0, info.hdmiVicLength,
                  info.has3dEntries ? 1 : 0, info.hdmi3dLength);
      if (info.hasVicEntries && info.hdmiVicLength > 0) {
        std::printf("  hdmiVic:");
        for (NvU8 j = 0; j < info.hdmiVicLength && j < sizeof(info.hdmi_vic); ++j) {
          std::printf(" %02X", info.hdmi_vic[j]);
        }
        std::printf("\n");
      }
      if (info.has3dEntries && info.hdmi3dLength > 0) {
        std::printf("  hdmi3d:");
        for (NvU8 j = 0; j < info.hdmi3dLength && j < sizeof(info.hdmi_3d); ++j) {
          std::printf(" %02X", info.hdmi_3d[j]);
        }
        std::printf("\n");
      }
    } else if (caps.infoType == NV_MONITOR_CAPS_TYPE_HDMI_VCDB) {
      const NV_MONITOR_CAPS_VCDB &info = caps.data.vcdb;
      std::printf("  quantYcc=%u quantRgb=%u scanPref=%u scanIT=%u scanCE=%u\n", info.quantizationRangeYcc,
                  info.quantizationRangeRgb, info.scanInfoPreferredVideoFormat, info.scanInfoITVideoFormats,
                  info.scanInfoCEVideoFormats);
    }
  }

  return 0;
}

int CmdDisplayMonitorColorCaps(int argc, char **argv) {
  NvU32 displayId = 0;
  if (!ParseDisplayIdArg(argc, argv, &displayId)) { return 1; }

  NvU32 count = 0;
  NvAPI_Status status = NvAPI_DISP_GetMonitorColorCapabilities(displayId, NULL, &count);
  if (status != NVAPI_OK && status != NVAPI_INSUFFICIENT_BUFFER) {
    PrintNvapiError("NvAPI_DISP_GetMonitorColorCapabilities failed", status);
    return 1;
  }

  if (count == 0) {
    std::printf("No monitor color caps reported.\n");
    return 0;
  }

  std::vector<NV_MONITOR_COLOR_CAPS> caps(count);
  for (NvU32 i = 0; i < count; ++i) { caps[i].version = NV_MONITOR_COLOR_CAPS_VER; }

  status = NvAPI_DISP_GetMonitorColorCapabilities(displayId, caps.data(), &count);
  if (status != NVAPI_OK && status != NVAPI_INSUFFICIENT_BUFFER) {
    PrintNvapiError("NvAPI_DISP_GetMonitorColorCapabilities failed", status);
    return 1;
  }

  std::printf("Monitor color caps: entries=%u\n", count);
  for (NvU32 i = 0; i < count; ++i) {
    std::printf("  [%u] format=%s bpc=%s\n", i, DpColorFormatName(caps[i].colorFormat),
                DpBpcName(caps[i].backendBitDepths));
  }
  return 0;
}

int CmdDisplayScaling(int argc, char **argv) {
  NvU32 displayId = 0;
  if (!ParseDisplayIdArg(argc, argv, &displayId)) { return 1; }

  NV_GET_SCALING_CAPS caps = {};
  caps.version = NV_GET_SCALING_CAPS_VER;
  NvAPI_Status status = NvAPI_DISP_GetScalingCaps(displayId, &caps);
  if (status == NVAPI_INCOMPATIBLE_STRUCT_VERSION) {
    NV_GET_SCALING_CAPS_V1 capsV1 = {};
    capsV1.version = NV_GET_SCALING_CAPS_VER1;
    status = NvAPI_DISP_GetScalingCaps(displayId, reinterpret_cast<NV_GET_SCALING_CAPS *>(&capsV1));
    if (status != NVAPI_OK) {
      PrintNvapiError("NvAPI_DISP_GetScalingCaps failed", status);
      return 1;
    }
    PrintScalingCapsV1(capsV1);
    return 0;
  }
  if (status != NVAPI_OK) {
    PrintNvapiError("NvAPI_DISP_GetScalingCaps failed", status);
    return 1;
  }

  PrintScalingCapsV2(reinterpret_cast<const NV_GET_SCALING_CAPS_V2 &>(caps));
  return 0;
}

int CmdDisplayScalingOverrideGet(int argc, char **argv) {
  NvU32 displayId = 0;
  if (!ParseDisplayIdArg(argc, argv, &displayId)) { return 1; }

  NV_GET_SCALING_CAPS caps = {};
  caps.version = NV_GET_SCALING_CAPS_VER;
  NvAPI_Status status = NvAPI_DISP_GetScalingCapsOverride(displayId, &caps);
  if (status == NVAPI_INCOMPATIBLE_STRUCT_VERSION) {
    NV_GET_SCALING_CAPS_V1 capsV1 = {};
    capsV1.version = NV_GET_SCALING_CAPS_VER1;
    status = NvAPI_DISP_GetScalingCapsOverride(displayId, reinterpret_cast<NV_GET_SCALING_CAPS *>(&capsV1));
    if (status != NVAPI_OK) {
      PrintNvapiError("NvAPI_DISP_GetScalingCapsOverride failed", status);
      return 1;
    }
    PrintScalingCapsV1(capsV1);
    return 0;
  }
  if (status != NVAPI_OK) {
    PrintNvapiError("NvAPI_DISP_GetScalingCapsOverride failed", status);
    return 1;
  }

  PrintScalingCapsV2(reinterpret_cast<const NV_GET_SCALING_CAPS_V2 &>(caps));
  return 0;
}

int CmdDisplayScalingOverrideSet(int argc, char **argv) {
  NvU32 displayId = 0;
  bool hasDisplayId = false;
  bool forceOverride = false;
  NV_SCALING preferred = NV_SCALING_DEFAULT;

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
    if (std::strcmp(argv[i], "--preferred") == 0) {
      if (i + 1 >= argc || !ParseScalingMode(argv[i + 1], &preferred)) {
        std::printf("Invalid preferred scaling mode.\n");
        return 1;
      }
      ++i;
      continue;
    }
    if (std::strcmp(argv[i], "--force-override") == 0) {
      forceOverride = true;
      continue;
    }
    std::printf("Unknown option: %s\n", argv[i]);
    return 1;
  }

  if (!hasDisplayId) {
    std::printf("Missing required --id\n");
    return 1;
  }

  if (forceOverride && preferred == NV_SCALING_DEFAULT) {
    std::printf("--force-override requires --preferred.\n");
    return 1;
  }

  bool usedV1 = false;
  bool capsFromOverride = false;
  NV_GET_SCALING_CAPS caps = {};
  caps.version = NV_GET_SCALING_CAPS_VER;
  NvAPI_Status status = NvAPI_DISP_GetScalingCapsOverride(displayId, &caps);
  if (status == NVAPI_INCOMPATIBLE_STRUCT_VERSION) {
    usedV1 = true;
  } else if (status == NVAPI_OK) {
    capsFromOverride = true;
  } else if (status != NVAPI_OK) {
    status = NvAPI_DISP_GetScalingCaps(displayId, &caps);
    if (status == NVAPI_INCOMPATIBLE_STRUCT_VERSION) {
      usedV1 = true;
    } else if (status != NVAPI_OK) {
      PrintNvapiError("NvAPI_DISP_GetScalingCapsOverride failed", status);
      return 1;
    }
  }

  if (usedV1) {
    NV_GET_SCALING_CAPS_V1 capsV1 = {};
    capsV1.version = NV_GET_SCALING_CAPS_VER1;
    status = NvAPI_DISP_GetScalingCapsOverride(displayId, reinterpret_cast<NV_GET_SCALING_CAPS *>(&capsV1));
    if (status != NVAPI_OK) {
      status = NvAPI_DISP_GetScalingCaps(displayId, reinterpret_cast<NV_GET_SCALING_CAPS *>(&capsV1));
      if (status != NVAPI_OK) {
        PrintNvapiError("NvAPI_DISP_GetScalingCapsOverride failed", status);
        return 1;
      }
    } else {
      bool emptyCaps = !capsV1.isGPUScalingAvailable && !capsV1.isGPUFixedAspectRatioScalingAvailable &&
                       !capsV1.isMonitorScalingAvailable && !capsV1.isCenterScalingAvailable;
      if (emptyCaps) {
        NV_GET_SCALING_CAPS_V1 fallback = {};
        fallback.version = NV_GET_SCALING_CAPS_VER1;
        status = NvAPI_DISP_GetScalingCaps(displayId, reinterpret_cast<NV_GET_SCALING_CAPS *>(&fallback));
        if (status == NVAPI_OK) {
          capsV1 = fallback;
        } else {
          PrintNvapiError("NvAPI_DISP_GetScalingCaps failed", status);
          return 1;
        }
      }
    }

    if (forceOverride) {
      if (!ApplyForcedScalingCapsV1(capsV1, preferred)) {
        std::printf("Force override is not supported for preferred scaling %s on caps v1.\n", ScalingName(preferred));
        return 1;
      }
      capsV1.version = NV_GET_SCALING_CAPS_VER1;
    }

    PrintScalingCapsV1(capsV1);
    std::printf("Preferred scaling=%s\n", ScalingName(preferred));

    status = NvAPI_DISP_SetScalingCapsOverride(displayId, reinterpret_cast<NV_GET_SCALING_CAPS *>(&capsV1), preferred);
    if (status != NVAPI_OK) {
      PrintNvapiError("NvAPI_DISP_SetScalingCapsOverride failed", status);
      return 1;
    }

    std::printf("Scaling override updated.\n");
    return 0;
  }

  NV_GET_SCALING_CAPS_V2 &capsV2 = reinterpret_cast<NV_GET_SCALING_CAPS_V2 &>(caps);
  if (capsFromOverride) {
    bool emptyCaps = !capsV2.isGPUScalingAvailable && !capsV2.isGPUFixedAspectRatioScalingAvailable &&
                     !capsV2.isMonitorScalingAvailable && !capsV2.isCenterScalingAvailable &&
                     !capsV2.isGPUScalingToAspectScanoutToClosest && !capsV2.isGPUScanoutToClosest &&
                     !capsV2.isGpuRepeatToAspectScanoutToNative;
    if (emptyCaps) {
      NV_GET_SCALING_CAPS fallback = {};
      fallback.version = NV_GET_SCALING_CAPS_VER;
      status = NvAPI_DISP_GetScalingCaps(displayId, &fallback);
      if (status == NVAPI_OK) {
        caps = fallback;
      } else {
        PrintNvapiError("NvAPI_DISP_GetScalingCaps failed", status);
        return 1;
      }
    }
  }

  if (forceOverride) {
    if (!ApplyForcedScalingCapsV2(capsV2, preferred)) {
      std::printf("Force override is not supported for preferred scaling %s.\n", ScalingName(preferred));
      return 1;
    }
    caps.version = NV_GET_SCALING_CAPS_VER;
  }

  PrintScalingCapsV2(capsV2);
  std::printf("Preferred scaling=%s\n", ScalingName(preferred));

  status = NvAPI_DISP_SetScalingCapsOverride(displayId, &caps, preferred);
  if (status != NVAPI_OK) {
    PrintNvapiError("NvAPI_DISP_SetScalingCapsOverride failed", status);
    return 1;
  }

  std::printf("Scaling override updated.\n");
  return 0;
}

int CmdDisplayScalingOverride(int argc, char **argv) {
  if (argc < 1) {
    PrintUsageGroup("display");
    return 1;
  }

  if (std::strcmp(argv[0], "get") == 0) { return CmdDisplayScalingOverrideGet(argc - 1, argv + 1); }
  if (std::strcmp(argv[0], "set") == 0) { return CmdDisplayScalingOverrideSet(argc - 1, argv + 1); }

  std::printf("Unknown display scaling-override subcommand: %s\n", argv[0]);
  return 1;
}

int CmdDisplayViewportGet(int argc, char **argv) {
  NvU32 displayId = 0;
  if (!ParseDisplayIdArg(argc, argv, &displayId)) { return 1; }

  NV_VIEW_PORT_INFO info = {};
  info.version = NV_VIEW_PORT_INFO_VER;
  NvAPI_Status status = NvAPI_DISP_GetViewPortInfo(displayId, &info);
  if (status != NVAPI_OK) {
    PrintNvapiError("NvAPI_DISP_GetViewPortInfo failed", status);
    return 1;
  }

  std::printf("Viewport: x=%u y=%u w=%u h=%u lock=%u\n", info.viewPort.x, info.viewPort.y, info.viewPort.w,
              info.viewPort.h, info.viewPortLockState ? 1 : 0);
  std::printf("  zoom=%.3f%% (raw=%u)\n", static_cast<double>(info.zoomValue) / 1000.0, info.zoomValue);
  return 0;
}

int CmdDisplayViewportSet(int argc, char **argv) {
  NvU32 displayId = 0;
  bool hasDisplayId = false;

  bool hasX = false;
  bool hasY = false;
  bool hasW = false;
  bool hasH = false;
  bool hasLock = false;
  bool hasZoom = false;
  NvU32 x = 0;
  NvU32 y = 0;
  NvU32 w = 0;
  NvU32 h = 0;
  bool lock = false;
  double zoom = 0.0;

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
    if (std::strcmp(argv[i], "--x") == 0) {
      if (i + 1 >= argc || !ParseUint(argv[i + 1], &x)) {
        std::printf("Invalid --x value.\n");
        return 1;
      }
      hasX = true;
      ++i;
      continue;
    }
    if (std::strcmp(argv[i], "--y") == 0) {
      if (i + 1 >= argc || !ParseUint(argv[i + 1], &y)) {
        std::printf("Invalid --y value.\n");
        return 1;
      }
      hasY = true;
      ++i;
      continue;
    }
    if (std::strcmp(argv[i], "--w") == 0) {
      if (i + 1 >= argc || !ParseUint(argv[i + 1], &w)) {
        std::printf("Invalid --w value.\n");
        return 1;
      }
      hasW = true;
      ++i;
      continue;
    }
    if (std::strcmp(argv[i], "--h") == 0) {
      if (i + 1 >= argc || !ParseUint(argv[i + 1], &h)) {
        std::printf("Invalid --h value.\n");
        return 1;
      }
      hasH = true;
      ++i;
      continue;
    }
    if (std::strcmp(argv[i], "--lock") == 0) {
      if (i + 1 >= argc || !ParseBoolValue(argv[i + 1], &lock)) {
        std::printf("Invalid --lock value.\n");
        return 1;
      }
      hasLock = true;
      ++i;
      continue;
    }
    if (std::strcmp(argv[i], "--zoom") == 0) {
      if (i + 1 >= argc || !ParseDoubleValue(argv[i + 1], &zoom)) {
        std::printf("Invalid --zoom value.\n");
        return 1;
      }
      hasZoom = true;
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

  if (!hasX && !hasY && !hasW && !hasH && !hasLock && !hasZoom) {
    std::printf("No viewport changes specified.\n");
    return 1;
  }

  NV_VIEW_PORT_INFO info = {};
  info.version = NV_VIEW_PORT_INFO_VER;
  NvAPI_Status status = NvAPI_DISP_GetViewPortInfo(displayId, &info);
  if (status != NVAPI_OK) {
    PrintNvapiError("NvAPI_DISP_GetViewPortInfo failed", status);
    return 1;
  }

  NvU32 setFlags = 0;
  if (hasX || hasY || hasW || hasH) {
    if (hasX) { info.viewPort.x = x; }
    if (hasY) { info.viewPort.y = y; }
    if (hasW) { info.viewPort.w = w; }
    if (hasH) { info.viewPort.h = h; }
    setFlags |= NV_VIEW_PORT_INFO_SET_VIEWPORT;
  }
  if (hasLock) {
    info.viewPortLockState = lock ? 1 : 0;
    setFlags |= NV_VIEW_PORT_INFO_SET_LOCK_STATE;
  }
  if (hasZoom) {
    if (zoom < 0.0) {
      std::printf("Zoom must be non-negative.\n");
      return 1;
    }
    info.zoomValue = static_cast<NvU32>(zoom * 1000.0 + 0.5);
    setFlags |= NV_VIEW_PORT_INFO_SET_ZOOM;
  }

  std::printf("Viewport set: x=%u y=%u w=%u h=%u lock=%u zoom=%.3f%%\n", info.viewPort.x, info.viewPort.y,
              info.viewPort.w, info.viewPort.h, info.viewPortLockState ? 1 : 0,
              static_cast<double>(info.zoomValue) / 1000.0);

  status = NvAPI_DISP_SetViewPortInfo(displayId, &info, setFlags);
  if (status != NVAPI_OK) {
    PrintNvapiError("NvAPI_DISP_SetViewPortInfo failed", status);
    return 1;
  }

  std::printf("Viewport updated.\n");
  return 0;
}

int CmdDisplayViewport(int argc, char **argv) {
  if (argc < 1) {
    PrintUsageGroup("display");
    return 1;
  }

  if (std::strcmp(argv[0], "get") == 0) { return CmdDisplayViewportGet(argc - 1, argv + 1); }
  if (std::strcmp(argv[0], "set") == 0) { return CmdDisplayViewportSet(argc - 1, argv + 1); }

  std::printf("Unknown display viewport subcommand: %s\n", argv[0]);
  return 1;
}

int CmdDisplayFeatureGet(int argc, char **argv) {
  NvU32 displayId = 0;
  if (!ParseDisplayIdArg(argc, argv, &displayId)) { return 1; }

  NV_DISPLAY_FEATURE_CONFIG config = {};
  config.version = NV_DISPLAY_FEATURE_CONFIG_VER;
  NvAPI_Status status = NvAPI_DISP_GetFeatureConfig(displayId, &config);
  if (status != NVAPI_OK) {
    PrintNvapiError("NvAPI_DISP_GetFeatureConfig failed", status);
    return 1;
  }

  std::printf("Feature config: panScan=%u gdiPrimarySync=%u\n", config.isPanAndScanEnabled ? 1 : 0,
              config.modulePresentSyncGDIPrimaryTarget ? 1 : 0);
  return 0;
}

int CmdDisplayFeatureSet(int argc, char **argv) {
  NvU32 displayId = 0;
  bool hasDisplayId = false;
  bool hasPanScan = false;
  bool panScan = false;
  bool hasGdiPrimary = false;
  bool gdiPrimary = false;

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
    if (std::strcmp(argv[i], "--pan-scan") == 0) {
      if (i + 1 >= argc || !ParseBoolValue(argv[i + 1], &panScan)) {
        std::printf("Invalid --pan-scan value.\n");
        return 1;
      }
      hasPanScan = true;
      ++i;
      continue;
    }
    if (std::strcmp(argv[i], "--gdi-primary") == 0) {
      if (i + 1 >= argc || !ParseBoolValue(argv[i + 1], &gdiPrimary)) {
        std::printf("Invalid --gdi-primary value.\n");
        return 1;
      }
      hasGdiPrimary = true;
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

  if (!hasPanScan && !hasGdiPrimary) {
    std::printf("No feature changes specified.\n");
    return 1;
  }

  NV_DISPLAY_FEATURE_CONFIG config = {};
  config.version = NV_DISPLAY_FEATURE_CONFIG_VER;
  NvAPI_Status status = NvAPI_DISP_GetFeatureConfig(displayId, &config);
  if (status != NVAPI_OK) {
    PrintNvapiError("NvAPI_DISP_GetFeatureConfig failed", status);
    return 1;
  }

  if (hasPanScan) { config.isPanAndScanEnabled = panScan ? 1 : 0; }
  if (hasGdiPrimary) { config.modulePresentSyncGDIPrimaryTarget = gdiPrimary ? 1 : 0; }

  std::printf("Feature set: panScan=%u gdiPrimarySync=%u\n", config.isPanAndScanEnabled ? 1 : 0,
              config.modulePresentSyncGDIPrimaryTarget ? 1 : 0);

  status = NvAPI_DISP_SetFeatureConfig(displayId, &config);
  if (status != NVAPI_OK) {
    PrintNvapiError("NvAPI_DISP_SetFeatureConfig failed", status);
    return 1;
  }

  std::printf("Feature config updated.\n");
  return 0;
}

int CmdDisplayFeature(int argc, char **argv) {
  if (argc < 1) {
    PrintUsageGroup("display");
    return 1;
  }

  if (std::strcmp(argv[0], "get") == 0) { return CmdDisplayFeatureGet(argc - 1, argv + 1); }
  if (std::strcmp(argv[0], "set") == 0) { return CmdDisplayFeatureSet(argc - 1, argv + 1); }

  std::printf("Unknown display feature subcommand: %s\n", argv[0]);
  return 1;
}

int CmdDisplayWideColorGet(int argc, char **argv) {
  NvU32 displayId = 0;
  bool hasDisplayId = false;
  NV_WIDE_COLOR_RANGE range = NV_COLOR_xvYCC;

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
    if (std::strcmp(argv[i], "--range") == 0) {
      if (i + 1 >= argc || !ParseWideColorRange(argv[i + 1], &range)) {
        std::printf("Invalid --range value.\n");
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

  NV_WIDE_COLOR_RANGE_SETTING setting = {};
  setting.version = NV_WIDE_COLOR_RANGE_SETTING_VER;
  setting.colorRange = range;
  NvAPI_Status status = NvAPI_DISP_GetWideColorRange(displayId, &setting);
  if (status != NVAPI_OK) {
    PrintNvapiError("NvAPI_DISP_GetWideColorRange failed", status);
    return 1;
  }

  std::printf("Wide color range: range=%s enable=%u\n", WideColorRangeName(setting.colorRange), setting.enable ? 1 : 0);
  return 0;
}

int CmdDisplayWideColorSet(int argc, char **argv) {
  NvU32 displayId = 0;
  bool hasDisplayId = false;
  bool hasEnable = false;
  bool enable = false;
  NV_WIDE_COLOR_RANGE range = NV_COLOR_xvYCC;

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
    if (std::strcmp(argv[i], "--enable") == 0) {
      if (i + 1 >= argc || !ParseBoolValue(argv[i + 1], &enable)) {
        std::printf("Invalid --enable value.\n");
        return 1;
      }
      hasEnable = true;
      ++i;
      continue;
    }
    if (std::strcmp(argv[i], "--range") == 0) {
      if (i + 1 >= argc || !ParseWideColorRange(argv[i + 1], &range)) {
        std::printf("Invalid --range value.\n");
        return 1;
      }
      ++i;
      continue;
    }
    std::printf("Unknown option: %s\n", argv[i]);
    return 1;
  }

  if (!hasDisplayId || !hasEnable) {
    std::printf("Missing required --id and --enable\n");
    return 1;
  }

  NV_WIDE_COLOR_RANGE_SETTING setting = {};
  setting.version = NV_WIDE_COLOR_RANGE_SETTING_VER;
  setting.colorRange = range;
  setting.enable = enable ? 1 : 0;

  std::printf("Wide color set: range=%s enable=%u\n", WideColorRangeName(setting.colorRange), setting.enable ? 1 : 0);

  NvAPI_Status status = NvAPI_DISP_SetWideColorRange(displayId, &setting);
  if (status != NVAPI_OK) {
    PrintNvapiError("NvAPI_DISP_SetWideColorRange failed", status);
    return 1;
  }

  std::printf("Wide color range updated.\n");
  return 0;
}

int CmdDisplayWideColor(int argc, char **argv) {
  if (argc < 1) {
    PrintUsageGroup("display");
    return 1;
  }

  if (std::strcmp(argv[0], "get") == 0) { return CmdDisplayWideColorGet(argc - 1, argv + 1); }
  if (std::strcmp(argv[0], "set") == 0) { return CmdDisplayWideColorSet(argc - 1, argv + 1); }

  std::printf("Unknown display wide-color subcommand: %s\n", argv[0]);
  return 1;
}

int CmdDisplayBpcGet(int argc, char **argv) {
  NvU32 displayId = 0;
  if (!ParseDisplayIdArg(argc, argv, &displayId)) { return 1; }

  NV_BPC_CONFIG config = {};
  config.version = NV_BPC_CONFIG_VER;
  config.displayId = displayId;
  config.cmd = NV_BPC_CONFIG_CMD_GET;
  NvAPI_Status status = NvAPI_DISP_BpcConfiguration(&config);
  if (status != NVAPI_OK) {
    PrintNvapiError("NvAPI_DISP_BpcConfiguration failed", status);
    return 1;
  }

  std::printf("BPC config: bpc=%s (raw=%u) ditherOff=%u forceLink=%u forceRGDiv=%u\n",
              BpcName(static_cast<NV_BPC>(config.bpc)), config.bpc, config.ditherOff ? 1 : 0,
              config.forceAtCurLinkConfig ? 1 : 0, config.forceRGDivMode ? 1 : 0);
  return 0;
}

int CmdDisplayBpcSet(int argc, char **argv) {
  NvU32 displayId = 0;
  bool hasDisplayId = false;
  bool hasBpc = false;
  NvU32 bpc = 0;
  bool hasDither = false;
  bool dither = false;
  bool hasForceLink = false;
  bool forceLink = false;
  bool hasForceRgDiv = false;
  bool forceRgDiv = false;

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
    if (std::strcmp(argv[i], "--bpc") == 0) {
      if (i + 1 >= argc || !ParseBpcValue(argv[i + 1], &bpc)) {
        std::printf("Invalid --bpc value. Expected default|6|8|10|12|16.\n");
        return 1;
      }
      hasBpc = true;
      ++i;
      continue;
    }
    if (std::strcmp(argv[i], "--dither") == 0) {
      if (i + 1 >= argc || !ParseBoolValue(argv[i + 1], &dither)) {
        std::printf("Invalid --dither value.\n");
        return 1;
      }
      hasDither = true;
      ++i;
      continue;
    }
    if (std::strcmp(argv[i], "--force-link") == 0) {
      if (i + 1 >= argc || !ParseBoolValue(argv[i + 1], &forceLink)) {
        std::printf("Invalid --force-link value.\n");
        return 1;
      }
      hasForceLink = true;
      ++i;
      continue;
    }
    if (std::strcmp(argv[i], "--force-rg-div") == 0) {
      if (i + 1 >= argc || !ParseBoolValue(argv[i + 1], &forceRgDiv)) {
        std::printf("Invalid --force-rg-div value.\n");
        return 1;
      }
      hasForceRgDiv = true;
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

  if (!hasBpc && !hasDither && !hasForceLink && !hasForceRgDiv) {
    std::printf("No BPC changes specified.\n");
    return 1;
  }

  NV_BPC_CONFIG config = {};
  config.version = NV_BPC_CONFIG_VER;
  config.displayId = displayId;
  config.cmd = NV_BPC_CONFIG_CMD_GET;
  NvAPI_Status status = NvAPI_DISP_BpcConfiguration(&config);
  if (status != NVAPI_OK) {
    PrintNvapiError("NvAPI_DISP_BpcConfiguration failed", status);
    return 1;
  }

  if (hasBpc) { config.bpc = bpc; }
  if (hasDither) { config.ditherOff = dither ? 0 : 1; }
  if (hasForceLink) { config.forceAtCurLinkConfig = forceLink ? 1 : 0; }
  if (hasForceRgDiv) { config.forceRGDivMode = forceRgDiv ? 1 : 0; }

  config.cmd = NV_BPC_CONFIG_CMD_SET;

  std::printf("BPC set: bpc=%s (raw=%u) ditherOff=%u forceLink=%u forceRGDiv=%u\n",
              BpcName(static_cast<NV_BPC>(config.bpc)), config.bpc, config.ditherOff ? 1 : 0,
              config.forceAtCurLinkConfig ? 1 : 0, config.forceRGDivMode ? 1 : 0);

  status = NvAPI_DISP_BpcConfiguration(&config);
  if (status != NVAPI_OK) {
    PrintNvapiError("NvAPI_DISP_BpcConfiguration failed", status);
    return 1;
  }

  std::printf("BPC configuration updated.\n");
  return 0;
}

int CmdDisplayBpc(int argc, char **argv) {
  if (argc < 1) {
    PrintUsageGroup("display");
    return 1;
  }

  if (std::strcmp(argv[0], "get") == 0) { return CmdDisplayBpcGet(argc - 1, argv + 1); }
  if (std::strcmp(argv[0], "set") == 0) { return CmdDisplayBpcSet(argc - 1, argv + 1); }

  std::printf("Unknown display bpc subcommand: %s\n", argv[0]);
  return 1;
}

int CmdDisplayBlankingGet(int argc, char **argv) {
  NvU32 displayId = 0;
  if (!ParseDisplayIdArg(argc, argv, &displayId)) { return 1; }

  NV_DISPLAY_BLANKING_INFO info = {};
  info.version = NV_DISPLAY_BLANKING_INFO_VER;
  NvAPI_Status status = NvAPI_DISP_GetDisplayBlankingState(displayId, &info);
  if (status != NVAPI_OK) {
    PrintNvapiError("NvAPI_DISP_GetDisplayBlankingState failed", status);
    return 1;
  }

  std::printf("Display blanking: state=%u persist=%u\n", info.blankingState ? 1 : 0,
              info.persistBlankingAcrossHotPlugUnplug ? 1 : 0);
  return 0;
}

int CmdDisplayBlankingSet(int argc, char **argv) {
  NvU32 displayId = 0;
  bool hasDisplayId = false;
  bool hasState = false;
  bool state = false;
  bool hasPersist = false;
  bool persist = false;

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
    if (std::strcmp(argv[i], "--state") == 0) {
      if (i + 1 >= argc || !ParseBoolValue(argv[i + 1], &state)) {
        std::printf("Invalid --state value.\n");
        return 1;
      }
      hasState = true;
      ++i;
      continue;
    }
    if (std::strcmp(argv[i], "--persist") == 0) {
      if (i + 1 >= argc || !ParseBoolValue(argv[i + 1], &persist)) {
        std::printf("Invalid --persist value.\n");
        return 1;
      }
      hasPersist = true;
      ++i;
      continue;
    }
    std::printf("Unknown option: %s\n", argv[i]);
    return 1;
  }

  if (!hasDisplayId || !hasState) {
    std::printf("Missing required --id and --state\n");
    return 1;
  }

  NV_DISPLAY_BLANKING_INFO info = {};
  info.version = NV_DISPLAY_BLANKING_INFO_VER;
  NvAPI_Status status = NvAPI_DISP_GetDisplayBlankingState(displayId, &info);
  if (status != NVAPI_OK) {
    PrintNvapiError("NvAPI_DISP_GetDisplayBlankingState failed", status);
    return 1;
  }

  info.blankingState = state ? 1 : 0;
  if (hasPersist) { info.persistBlankingAcrossHotPlugUnplug = persist ? 1 : 0; }

  std::printf("Display blanking set: state=%u persist=%u\n", info.blankingState ? 1 : 0,
              info.persistBlankingAcrossHotPlugUnplug ? 1 : 0);

  status = NvAPI_DISP_SetDisplayBlankingState(displayId, &info);
  if (status != NVAPI_OK) {
    PrintNvapiError("NvAPI_DISP_SetDisplayBlankingState failed", status);
    return 1;
  }

  std::printf("Display blanking updated.\n");
  return 0;
}

int CmdDisplayBlanking(int argc, char **argv) {
  if (argc < 1) {
    PrintUsageGroup("display");
    return 1;
  }

  if (std::strcmp(argv[0], "get") == 0) { return CmdDisplayBlankingGet(argc - 1, argv + 1); }
  if (std::strcmp(argv[0], "set") == 0) { return CmdDisplayBlankingSet(argc - 1, argv + 1); }

  std::printf("Unknown display blanking subcommand: %s\n", argv[0]);
  return 1;
}

int CmdDisplayIdByName(int argc, char **argv) {
  const char *name = NULL;
  for (int i = 0; i < argc; ++i) {
    if (std::strcmp(argv[i], "--name") == 0) {
      if (i + 1 >= argc) {
        std::printf("Missing value for --name\n");
        return 1;
      }
      name = argv[i + 1];
      ++i;
      continue;
    }
    std::printf("Unknown option: %s\n", argv[i]);
    return 1;
  }

  if (!name) {
    std::printf("Missing required --name\n");
    return 1;
  }

  NvU32 displayId = 0;
  NvAPI_Status status = NvAPI_DISP_GetDisplayIdByDisplayName(name, &displayId);
  if (status != NVAPI_OK) {
    PrintNvapiError("NvAPI_DISP_GetDisplayIdByDisplayName failed", status);
    return 1;
  }

  std::printf("Display name \"%s\" id=0x%08X\n", name, displayId);
  return 0;
}

int CmdDisplayGdiPrimary() {
  NvU32 displayId = 0;
  NvAPI_Status status = NvAPI_DISP_GetGDIPrimaryDisplayId(&displayId);
  if (status != NVAPI_OK) {
    PrintNvapiError("NvAPI_DISP_GetGDIPrimaryDisplayId failed", status);
    return 1;
  }

  std::printf("GDI primary display id=0x%08X\n", displayId);
  return 0;
}

int CmdDisplayHandleFromId(int argc, char **argv) {
  NvU32 displayId = 0;
  if (!ParseDisplayIdArg(argc, argv, &displayId)) { return 1; }

  NvDisplayHandle handle = NULL;
  NvAPI_Status status = NvAPI_DISP_GetDisplayHandleFromDisplayId(displayId, &handle);
  if (status != NVAPI_OK) {
    PrintNvapiError("NvAPI_DISP_GetDisplayHandleFromDisplayId failed", status);
    return 1;
  }

  std::printf("Display handle for id=0x%08X is 0x%p\n", displayId, handle);
  return 0;
}

int CmdDisplayIdFromHandle(int argc, char **argv) {
  NvU32 handleIndex = 0;
  bool hasHandleIndex = false;

  for (int i = 0; i < argc; ++i) {
    if (std::strcmp(argv[i], "--handle-index") == 0) {
      if (i + 1 >= argc || !ParseUint(argv[i + 1], &handleIndex)) {
        std::printf("Invalid handle index.\n");
        return 1;
      }
      hasHandleIndex = true;
      ++i;
      continue;
    }
    std::printf("Unknown option: %s\n", argv[i]);
    return 1;
  }

  if (!hasHandleIndex) {
    std::printf("Missing required --handle-index\n");
    return 1;
  }

  NvDisplayHandle handle = NULL;
  if (!GetDisplayHandleByIndex(handleIndex, &handle)) {
    std::printf("Display handle index %u not found.\n", handleIndex);
    return 1;
  }

  NvU32 displayId = 0;
  NvAPI_Status status = NvAPI_DISP_GetDisplayIdFromDisplayHandle(handle, &displayId);
  if (status != NVAPI_OK) {
    PrintNvapiError("NvAPI_DISP_GetDisplayIdFromDisplayHandle failed", status);
    return 1;
  }

  std::printf("Display id for handle index %u is 0x%08X\n", handleIndex, displayId);
  return 0;
}
} // namespace nvcli
