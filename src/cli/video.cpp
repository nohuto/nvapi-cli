/*
 * Copyright (c) 2026 Noverse (Nohuto). All rights reserved.
 * Proprietary and confidential. Unauthorized copying or redistribution is strictly prohibited.
 */

#include "cli/commands.h"

namespace nvcli {
int CmdVideoColorGet(NvDisplayHandle handle, bool useDefault) {
  (void)handle;
  NV_VIDEO_COLORCONTROL_INFO info = {};
  info.version = NV_VIDEO_COLORCONTROL_NVAPI_VER;
  NvAPI_Status status = NVAPI_NOT_SUPPORTED;
  if (useDefault) {
    // Deprecated in release 455: NvAPI_Video_ColorControl_GetDefault(handle, &info);
  } else {
    // Deprecated in release 455: NvAPI_Video_ColorControl_Get(handle, &info);
  }
  if (status != NVAPI_OK) {
    PrintNvapiError(useDefault ? "NvAPI_Video_ColorControl_GetDefault failed" : "NvAPI_Video_ColorControl_Get failed",
                    status);
    return 1;
  }

  std::printf("Video color control (%s):\n", useDefault ? "default" : "current");
  std::printf("  controlSetting=%u feature=%u caps=%u\n", info.colorControlSettings, info.colorFeature, info.colorCaps);
  std::printf("  brightness=%u (min=%u max=%u)\n", info.brightnessRangeValue.value, info.brightnessRangeValue.min,
              info.brightnessRangeValue.max);
  std::printf("  contrast=%u (min=%u max=%u)\n", info.contrastRangeValue.value, info.contrastRangeValue.min,
              info.contrastRangeValue.max);
  std::printf("  hue=%u (min=%u max=%u)\n", info.hueRangeValue.value, info.hueRangeValue.min, info.hueRangeValue.max);
  std::printf("  saturation=%u (min=%u max=%u)\n", info.saturationRangeValue.value, info.saturationRangeValue.min,
              info.saturationRangeValue.max);
  std::printf("  colorTemp=%u (min=%u max=%u) feature=%u caps=%u\n", info.colorTempRangeValue.value,
              info.colorTempRangeValue.min, info.colorTempRangeValue.max, info.colorTempFeature, info.colorTempCaps);
  std::printf("  gammaFeature=%u caps=%u\n", info.gammaFeature, info.gammaCaps);
  std::printf("  yGamma=%u (min=%u max=%u)\n", info.yGammaRangeValue.value, info.yGammaRangeValue.min,
              info.yGammaRangeValue.max);
  std::printf("  rgbGammaR=%u (min=%u max=%u)\n", info.rgbGammaRRangeValue.value, info.rgbGammaRRangeValue.min,
              info.rgbGammaRRangeValue.max);
  std::printf("  rgbGammaG=%u (min=%u max=%u)\n", info.rgbGammaGRangeValue.value, info.rgbGammaGRangeValue.min,
              info.rgbGammaGRangeValue.max);
  std::printf("  rgbGammaB=%u (min=%u max=%u)\n", info.rgbGammaBRangeValue.value, info.rgbGammaBRangeValue.min,
              info.rgbGammaBRangeValue.max);
  return 0;
}

int CmdVideoColorGet(int argc, char **argv, bool useDefault) {
  NvU32 handleIndex = 0;
  for (int i = 0; i < argc; ++i) {
    if (std::strcmp(argv[i], "--handle-index") == 0) {
      if (i + 1 >= argc) {
        std::printf("Missing value for --handle-index\n");
        return 1;
      }
      if (!ParseUint(argv[i + 1], &handleIndex)) {
        std::printf("Invalid handle index: %s\n", argv[i + 1]);
        return 1;
      }
      ++i;
      continue;
    }
    std::printf("Unknown option: %s\n", argv[i]);
    return 1;
  }

  NvDisplayHandle handle = NULL;
  if (!GetDisplayHandleByIndex(handleIndex, &handle)) {
    std::printf("Display handle index %u not found.\n", handleIndex);
    return 1;
  }

  return CmdVideoColorGet(handle, useDefault);
}

int CmdVideoColorSet(int argc, char **argv) {
  NvU32 handleIndex = 0;
  bool hasBrightness = false;
  bool hasContrast = false;
  bool hasHue = false;
  bool hasSaturation = false;
  bool hasTemp = false;
  bool hasYGamma = false;
  bool hasRGamma = false;
  bool hasGGamma = false;
  bool hasBGamma = false;
  bool hasControlSetting = false;
  NV_VIDEO_COLORCONTROL_SETTING controlSetting = NV_VIDEO_COLORCONTROL_SETTING_USE_APP_CTRLS;
  NvU32 brightness = 0;
  NvU32 contrast = 0;
  NvU32 hue = 0;
  NvU32 saturation = 0;
  NvU32 temp = 0;
  NvU32 yGamma = 0;
  NvU32 rGamma = 0;
  NvU32 gGamma = 0;
  NvU32 bGamma = 0;

  for (int i = 0; i < argc; ++i) {
    if (std::strcmp(argv[i], "--handle-index") == 0) {
      if (i + 1 >= argc) {
        std::printf("Missing value for --handle-index\n");
        return 1;
      }
      if (!ParseUint(argv[i + 1], &handleIndex)) {
        std::printf("Invalid handle index: %s\n", argv[i + 1]);
        return 1;
      }
      ++i;
      continue;
    }
    if (std::strcmp(argv[i], "--brightness") == 0) {
      if (i + 1 >= argc || !ParseUint(argv[i + 1], &brightness)) {
        std::printf("Invalid brightness.\n");
        return 1;
      }
      hasBrightness = true;
      ++i;
      continue;
    }
    if (std::strcmp(argv[i], "--contrast") == 0) {
      if (i + 1 >= argc || !ParseUint(argv[i + 1], &contrast)) {
        std::printf("Invalid contrast.\n");
        return 1;
      }
      hasContrast = true;
      ++i;
      continue;
    }
    if (std::strcmp(argv[i], "--hue") == 0) {
      if (i + 1 >= argc || !ParseUint(argv[i + 1], &hue)) {
        std::printf("Invalid hue.\n");
        return 1;
      }
      hasHue = true;
      ++i;
      continue;
    }
    if (std::strcmp(argv[i], "--saturation") == 0) {
      if (i + 1 >= argc || !ParseUint(argv[i + 1], &saturation)) {
        std::printf("Invalid saturation.\n");
        return 1;
      }
      hasSaturation = true;
      ++i;
      continue;
    }
    if (std::strcmp(argv[i], "--color-temp") == 0) {
      if (i + 1 >= argc || !ParseUint(argv[i + 1], &temp)) {
        std::printf("Invalid color temperature.\n");
        return 1;
      }
      hasTemp = true;
      ++i;
      continue;
    }
    if (std::strcmp(argv[i], "--ygamma") == 0) {
      if (i + 1 >= argc || !ParseUint(argv[i + 1], &yGamma)) {
        std::printf("Invalid yGamma.\n");
        return 1;
      }
      hasYGamma = true;
      ++i;
      continue;
    }
    if (std::strcmp(argv[i], "--rgamma") == 0) {
      if (i + 1 >= argc || !ParseUint(argv[i + 1], &rGamma)) {
        std::printf("Invalid rGamma.\n");
        return 1;
      }
      hasRGamma = true;
      ++i;
      continue;
    }
    if (std::strcmp(argv[i], "--ggamma") == 0) {
      if (i + 1 >= argc || !ParseUint(argv[i + 1], &gGamma)) {
        std::printf("Invalid gGamma.\n");
        return 1;
      }
      hasGGamma = true;
      ++i;
      continue;
    }
    if (std::strcmp(argv[i], "--bgamma") == 0) {
      if (i + 1 >= argc || !ParseUint(argv[i + 1], &bGamma)) {
        std::printf("Invalid bGamma.\n");
        return 1;
      }
      hasBGamma = true;
      ++i;
      continue;
    }
    if (std::strcmp(argv[i], "--override") == 0) {
      controlSetting = NV_VIDEO_COLORCONTROL_SETTING_OVERRIDE_APP_CTRLS;
      hasControlSetting = true;
      continue;
    }
    if (std::strcmp(argv[i], "--use-app") == 0) {
      controlSetting = NV_VIDEO_COLORCONTROL_SETTING_USE_APP_CTRLS;
      hasControlSetting = true;
      continue;
    }
    std::printf("Unknown option: %s\n", argv[i]);
    return 1;
  }

  if (!(hasBrightness || hasContrast || hasHue || hasSaturation || hasTemp || hasYGamma || hasRGamma || hasGGamma ||
        hasBGamma || hasControlSetting)) {
    std::printf("No values specified.\n");
    return 1;
  }

  NvDisplayHandle handle = NULL;
  if (!GetDisplayHandleByIndex(handleIndex, &handle)) {
    std::printf("Display handle index %u not found.\n", handleIndex);
    return 1;
  }

  NV_VIDEO_COLORCONTROL_INFO info = {};
  info.version = NV_VIDEO_COLORCONTROL_NVAPI_VER;
  NvAPI_Status status = NVAPI_NOT_SUPPORTED;
  // Deprecated in release 455: NvAPI_Video_ColorControl_Get(handle, &info);
  if (status != NVAPI_OK) {
    PrintNvapiError("NvAPI_Video_ColorControl_Get failed", status);
    return 1;
  }

  if (hasBrightness) info.brightnessRangeValue.value = brightness;
  if (hasContrast) info.contrastRangeValue.value = contrast;
  if (hasHue) info.hueRangeValue.value = hue;
  if (hasSaturation) info.saturationRangeValue.value = saturation;
  if (hasTemp) info.colorTempRangeValue.value = temp;
  if (hasYGamma) info.yGammaRangeValue.value = yGamma;
  if (hasRGamma) info.rgbGammaRRangeValue.value = rGamma;
  if (hasGGamma) info.rgbGammaGRangeValue.value = gGamma;
  if (hasBGamma) info.rgbGammaBRangeValue.value = bGamma;
  if (hasControlSetting) info.colorControlSettings = controlSetting;

  std::printf("Video color set request:\n");
  if (hasBrightness) std::printf("  brightness=%u\n", brightness);
  if (hasContrast) std::printf("  contrast=%u\n", contrast);
  if (hasHue) std::printf("  hue=%u\n", hue);
  if (hasSaturation) std::printf("  saturation=%u\n", saturation);
  if (hasTemp) std::printf("  color-temp=%u\n", temp);
  if (hasYGamma) std::printf("  ygamma=%u\n", yGamma);
  if (hasRGamma) std::printf("  rgamma=%u\n", rGamma);
  if (hasGGamma) std::printf("  ggamma=%u\n", gGamma);
  if (hasBGamma) std::printf("  bgamma=%u\n", bGamma);
  if (hasControlSetting) std::printf("  controlSetting=%u\n", controlSetting);

  status = NVAPI_NOT_SUPPORTED;
  // Deprecated in release 455: NvAPI_Video_ColorControl_Set(handle, &info);
  if (status != NVAPI_OK) {
    PrintNvapiError("NvAPI_Video_ColorControl_Set failed", status);
    return 1;
  }

  std::printf("Video color settings updated.\n");
  return 0;
}

int CmdVideo(int argc, char **argv) {
  if (argc < 1) {
    PrintUsageGroup("video");
    return 1;
  }
  if (std::strcmp(argv[0], "help") == 0) {
    PrintUsageGroup("video");
    return 0;
  }

  if (std::strcmp(argv[0], "color") == 0) {
    if (argc >= 2 && std::strcmp(argv[1], "get") == 0) { return CmdVideoColorGet(argc - 2, argv + 2, false); }
    if (argc >= 2 && std::strcmp(argv[1], "default") == 0) { return CmdVideoColorGet(argc - 2, argv + 2, true); }
    if (argc >= 2 && std::strcmp(argv[1], "set") == 0) { return CmdVideoColorSet(argc - 2, argv + 2); }
    std::printf("Unknown video color subcommand: %s\n", argc >= 2 ? argv[1] : "");
    return 1;
  }

  std::printf("Unknown video subcommand: %s\n", argv[0]);
  return 1;
}

const char *HdmiFrlRateName(NV_HDMI_FRL_RATE rate) {
  switch (rate) {
  case NV_HDMI_FRL_RATE_NONE: return "NONE";
  case NV_HDMI_FRL_RATE_3LANES_3GBPS: return "3LANES_3GBPS";
  case NV_HDMI_FRL_RATE_3LANES_6GBPS: return "3LANES_6GBPS";
  case NV_HDMI_FRL_RATE_4LANES_6GBPS: return "4LANES_6GBPS";
  case NV_HDMI_FRL_RATE_4LANES_8GBPS: return "4LANES_8GBPS";
  case NV_HDMI_FRL_RATE_4LANES_10GBPS: return "4LANES_10GBPS";
  case NV_HDMI_FRL_RATE_4LANES_12GBPS: return "4LANES_12GBPS";
  default: return "UNKNOWN";
  }
}
} // namespace nvcli
