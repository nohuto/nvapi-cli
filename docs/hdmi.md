# HDMI Group

Covers the `nvapi-cli hdmi` command group (`src/cli/hdmi.cpp`). `--index N` is required for `hdmi hdcp-diag` and is omitted from the command list and blocks below for brevity. `--id HEX` is required for `hdmi hdcp-diag` and all `hdmi stereo` commands and is omitted from the command list and blocks below for brevity; get it from `display ids`. `--handle-index N` refers to a display handle index from `display list`. If `--output-id` is omitted, the CLI uses `NvAPI_GetAssociatedDisplayOutputId`.

```
nvapi-cli hdmi support [--handle-index N] [--output-id HEX]
nvapi-cli hdmi hdcp-diag
nvapi-cli hdmi stereo modes [--start N] [--count N] [--pass-through]
   [--width W --height H --refresh R]
nvapi-cli hdmi stereo get
nvapi-cli hdmi stereo set --type NAME|HEX
nvapi-cli hdmi audio-mute --handle-index N --state on|off [--output-id HEX]
```

# Command Reference

## hdmi support
Uses `NvAPI_GetHDMISupportInfo` (`NV_HDMI_SUPPORT_INFO`) to report HDMI capability flags for the GPU and monitor, including color format support, BPC support, HDR capabilities, and FRL rates.

```c
--handle-index N // optional display handle index
--output-id HEX // optional outputId or displayId; 0 uses associated output
// reports EDID 861 extension revision and FRL rates when available
```

## hdmi hdcp-diag
Uses `NvAPI_GetHdcpHdmiDiagnostics` (`NV_HDCP_HDMI_DIAGNOSTICS`) to report HDCP and HDMI diagnostic flags for a physical GPU and display.

```c
// no flags
// prints HDCP fuse/capability flags and HDMI capability flags
```

## hdmi stereo modes
Uses `NvAPI_DISP_EnumHDMIStereoModes` (`NV_HDMI_STEREO_MODES_LIST`) to enumerate HDMI stereo modes. The CLI can filter by dimensions or refresh rate and optionally request pass-through modes.

```c
--start N // enumeration start index (default 0)
--count N // maximum modes to return (default 16)
--pass-through // request pass-through capable modes
--width W --height H --refresh R // optional dimension/refresh filtering
```

## hdmi stereo get
Uses `NvAPI_DISP_GetHDMIStereoSettings` (`NV_HDMI_STEREO_SETTINGS`) to read the current stereo type.


## hdmi stereo set
Uses `NvAPI_DISP_SetHDMIStereoSettings` to apply a pass-through stereo mode. Use `none` to disable.

```c
--type NAME|HEX // none|frame-packing|top-bottom|side-by-side-full|side-by-side-half
// numeric type values are accepted for other NV_HDMI_STEREO_TYPE values
```

## hdmi audio-mute
Uses `NvAPI_SetHDMIAudioStreamMute` (`NV_HDMI_AUDIO_INFO`) to toggle HDMI audio mute for the target output.

```c
--handle-index N // display handle index
--state on|off // mute state
--output-id HEX // optional outputId; 0 uses associated output
```
