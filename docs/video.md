# Video Group

Covers the `nvapi-cli video` command group `src/cli/video.cpp`. `--handle-index N` refers to a display handle index from `display list` and is optional on all commands. The NVAPI video color control APIs are deprecated in release 455 and commonly return `NVAPI_NOT_SUPPORTED`.

```powershell
nvapi-cli video color get [--handle-index N]
nvapi-cli video color default [--handle-index N]
nvapi-cli video color set [--handle-index N] [--brightness N] [--contrast N] [--hue N] [--saturation N]
   [--color-temp N] [--yamma N] [--rgamma N] [--ggamma N] [--bgamma N] [--override|--use-app]
```

# Command Reference

## video color get
Uses `NvAPI_Video_ColorControl_Get` (`NV_VIDEO_COLORCONTROL_INFO`) to read current video color controls. The API exposes per control ranges and feature flags.

```powershell
--handle-index N # optional display handle index
# deprecated NVAPI call, may return NVAPI_NOT_SUPPORTED
```

## video color default
Uses `NvAPI_Video_ColorControl_GetDefault` to retrieve default color control values and ranges.

```powershell
--handle-index N # optional display handle index
# deprecated NVAPI call, may return NVAPI_NOT_SUPPORTED
```

## video color set
Uses `NvAPI_Video_ColorControl_Get` to read current values, applies the specified overrides, then calls `NvAPI_Video_ColorControl_Set`. The control setting toggles between app-controlled and NVAPI-controlled values.

```powershell
--handle-index N # optional display handle index
--brightness N --contrast N --hue N --saturation N # color controls
--color-temp N # color temperature control
--ygamma N --rgamma N --ggamma N --bgamma N # gamma controls
--override # NV_VIDEO_COLORCONTROL_SETTING_OVERRIDE_APP_CTRLS
--use-app # NV_VIDEO_COLORCONTROL_SETTING_USE_APP_CTRLS
# at least one value or control setting must be specified
# deprecated NVAPI calls, may return NVAPI_NOT_SUPPORTED
```
