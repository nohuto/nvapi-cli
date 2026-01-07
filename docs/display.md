# Display Group

Covers the `nvapi-cli display` command group (`src/cli/display_*.cpp`). `--id HEX` is required for most display commands and is omitted from the command list and blocks below for brevity, get it from `display ids` or from `display list` + `display id-from-handle`. `--handle-index N` refers to a display handle index from `display list`. `--index N` (used by `display ids`) is optional and is omitted from the blocks below for brevity, when omitted, the CLI enumerates all physical GPUs. `srcId:device` pairs for `display set` come from `display get`, the first path is used as the GPU primary view target.

```powershell
nvapi-cli display list
nvapi-cli display ids [--all] [--flags HEX]
nvapi-cli display edid [--flag default|raw|cooked|forced|inf|hw|tiles]
nvapi-cli display timing
nvapi-cli display get [--handle-index N]
nvapi-cli display set [--handle-index N] <srcId:device> [srcId:device ...]
nvapi-cli display custom list
nvapi-cli display custom try --width W --height H --refresh R [--depth BPP]
   [--type auto|cvt|cvt-rb|gtf|dmt|dmt-rb|eia861|analog-tv|nv-predefined] [--interlaced 0|1] [--hw-only 0|1]
   [--cea-id N] [--tv-format N] [--psf-id N]
nvapi-cli display custom save [--output-only 0|1] [--monitor-only 0|1]
nvapi-cli display custom delete
nvapi-cli display custom revert
nvapi-cli display monitor-caps [--type generic|hdmi-vsdb|hdmi-vcdb|all]
nvapi-cli display monitor-color-caps
nvapi-cli display scaling
nvapi-cli display scaling-override get
nvapi-cli display scaling-override set [--preferred MODE] [--force-override]
nvapi-cli display viewport get
nvapi-cli display viewport set [--x N] [--y N] [--w N] [--h N] [--lock 0|1] [--zoom PCT]
nvapi-cli display feature get
nvapi-cli display feature set [--pan-scan 0|1] [--gdi-primary 0|1]
nvapi-cli display wide-color get [--range xvycc]
nvapi-cli display wide-color set --enable 0|1 [--range xvycc]
nvapi-cli display bpc get
nvapi-cli display bpc set [--bpc default|6|8|10|12|16] [--dither 0|1] [--force-link 0|1] [--force-rg-div 0|1]
nvapi-cli display blanking get
nvapi-cli display blanking set --state 0|1 [--persist 0|1]
nvapi-cli display hdr caps
nvapi-cli display hdr session get
nvapi-cli display hdr session set --enable 0|1 [--expire SEC]
nvapi-cli display hdr color get
nvapi-cli display hdr color set [--mode MODE] [--format FORMAT] [--range RANGE] [--bpc BPC] [--os-hdr default|on|off]
nvapi-cli display id-by-name --name NAME
nvapi-cli display gdi-primary
nvapi-cli display handle-from-id
nvapi-cli display id-from-handle --handle-index N
```


# Command Reference

## display list
Uses `NvAPI_EnumNvidiaDisplayHandle`, `NvAPI_GetAssociatedNvidiaDisplayName`, and `NvAPI_GetAssociatedDisplayOutputId` to enumerate NVIDIA display handles, print names, and show the associated output ID when available.


## display ids
Uses `NvAPI_GPU_GetConnectedDisplayIds` (or `NvAPI_GPU_GetAllDisplayIds` with `--all`) to list display IDs and their connector/active/visible flags. Use flags to control cached vs uncached, SLI, lid state, or MST filtering.

```powershell
--all # call NvAPI_GPU_GetAllDisplayIds instead of GetConnectedDisplayIds
--flags HEX # NV_GPU_CONNECTED_IDS_FLAG_* bitmask (ignored with --all)
# flags include UNCACHED, SLI, LIDSTATE, FAKE, EXCLUDE_MST, SYMMETRIC, ORDERED
```

## display edid
Uses `NvAPI_GPU_GetEdidEx2` to retrieve EDID bytes for a displayId. The CLI allocates `NV_EDID_DATA_SIZE_MAX` (1024) bytes, prints the actual size, and dumps raw hex. With the `tiles` flag, the driver can return a concatenated EDID for tiled displays.

```powershell
--flag default|raw|cooked|forced|inf|hw|tiles # NV_EDID_FLAG selection
# EDID size can be up to NV_EDID_DATA_SIZE_MAX (1024)
# tiles combines EDIDs, use the base block extension count to split
```

## display timing
Uses `NvAPI_DISP_GetTimingInfo` (`NV_BACKEND_TIMING_INFO`) to list backend timings for a display. NVAPI currently returns timings present in EDID and uses a two-call pattern (count then data).

```powershell
# timings reported are EDID-backed per nvapi.h
```

## display get
Uses `NvAPI_GetDisplaySettings` (`NV_DISP_PATH`) to list display paths for a display handle. Each path reports `srcID` and `device` values needed for `display set`.

```powershell
--handle-index N # display handle index from display list (defaults to 0)
# NV_DISP_PATH is limited to a single GPU
```

## display set
Uses `NvAPI_SetDisplaySettings` to apply display paths for a handle. The CLI expects `srcId:device` pairs and submits them as `NV_DISP_PATH` entries, the first path becomes the GPU primary view target.

```powershell
--handle-index N # display handle index from display list (defaults to 0)
srcId:device # path entry from display get (srcID and device mask)
# NV_DISP_PATH is limited to a single GPU
```

## display custom list
Uses `NvAPI_DISP_EnumCustomDisplay` (`NV_CUSTOM_DISPLAY`) to enumerate custom display timings for a displayId until `NVAPI_END_ENUMERATION`.


## display custom try
Uses `NvAPI_DISP_GetTiming` (`NV_TIMING_INPUT`) to calculate timings and `NvAPI_DISP_TryCustomDisplay` to apply a trial custom timing without saving. The command does not support `NV_TIMING_OVERRIDE_CUST` because it requires explicit timing fields.

```powershell
--width W --height H --refresh R # required for most timing types
--depth BPP # 0 means all 8/16/32 bpp per nvapi.h
--type auto|cvt|cvt-rb|gtf|dmt|dmt-rb|eia861|analog-tv|nv-predefined # NV_TIMING_OVERRIDE
--interlaced 0|1 # set NV_TIMING_FLAG::isInterlaced
--hw-only 0|1 # request hardware modeset only (no OS update)
--cea-id N # required for type eia861 (EIA/CEA 861 ID)
--tv-format N # required for type analog-tv
--psf-id N # required for type nv-predefined (PsF)
# trial applies to hardware only, use custom save to persist or custom revert to cancel
```

## display custom save
Uses `NvAPI_DISP_SaveCustomDisplay` to persist the current trial custom display configuration. This should be called right after a successful `display custom try`.

```powershell
--output-only 0|1 # apply only to same outputId
--monitor-only 0|1 # apply only to same EDID (or TV connector for analog)
# no effect if no trial custom display is active
```

## display custom delete
Uses `NvAPI_DISP_DeleteCustomDisplay` to delete a saved custom display entry. The CLI fetches the `NV_CUSTOM_DISPLAY` at the given index before deleting it, so you still need to supply the custom display index from `display custom list`.


## display custom revert
Uses `NvAPI_DISP_RevertCustomDisplayTrial` to restore the configuration changed by `display custom try`.

```powershell
# only meaningful after a trial custom display
```

## display monitor-caps
Uses `NvAPI_DISP_GetMonitorCapabilities` to query monitor capability blocks such as generic backend caps or HDMI VSDB/VCDB fields.

```powershell
--type generic|hdmi-vsdb|hdmi-vcdb|all # NV_MONITOR_CAPS_TYPE (all queries all three)
```

## display monitor-color-caps
Uses `NvAPI_DISP_GetMonitorColorCapabilities` to enumerate DP color formats and bit depths. The API uses a count-then-data flow and is DP-only per `nvapi.h`.

```powershell
# NVAPI_INVALID_DISPLAY_ID if not connected or not a DP panel
```

## display scaling
Uses `NvAPI_DISP_GetScalingCaps` to report scaling capabilities, default scaling, and current scaling. The CLI retries the v1 struct on `NVAPI_INCOMPATIBLE_STRUCT_VERSION`.

```powershell
# v1 lacks aspect-closest, center-closest, and integer scaling bits
```

## display scaling-override get
Uses `NvAPI_DISP_GetScalingCapsOverride` to read the current override caps. The driver may return empty caps if no override is set.


## display scaling-override set
Uses `NvAPI_DISP_SetScalingCapsOverride` to override scaling caps and optionally set a preferred scaling mode. The driver allows only one scaling cap bit to be set at a time.

```powershell
--preferred MODE # default|closest|native|centered|aspect|aspect-closest|centered-closest|integer|custom
--force-override
# driver limitation: only one cap bit should be set at a time
```

## display viewport get
Uses `NvAPI_DISP_GetViewPortInfo` to read the current viewport rectangle, lock state, and zoom. Zoom is stored as percent * 1000.

```powershell
# zoomValue = percent * 1000
```

## display viewport set
Uses `NvAPI_DISP_SetViewPortInfo` to update viewport fields based on `NV_VIEW_PORT_INFO_SET_*` flags.

```powershell
--x N --y N --w N --h N # viewport rectangle
--lock 0|1 # lock/unlock viewport
--zoom PCT # percent, stored as PCT * 1000
# zoom must be non-negative
```

## display feature get
Uses `NvAPI_DISP_GetFeatureConfig` to report pan-and-scan state and GDI primary sync state.


## display feature set
Uses `NvAPI_DISP_SetFeatureConfig` to change pan-and-scan or GDI primary sync state. When setting GDI primary sync, the displayId should point at the GDI primary display.

```powershell
--pan-scan 0|1 # pan-and-scan enable
--gdi-primary 0|1 # sync with GDI primary target
```

## display wide-color get
Uses `NvAPI_DISP_GetWideColorRange` to query wide color range state. NVAPI only supports xvYCC in this interface.

```powershell
--range xvycc # only xvYCC is supported
```

## display wide-color set
Uses `NvAPI_DISP_SetWideColorRange` to enable or disable xvYCC wide color mode.

```powershell
--enable 0|1 # enable or disable xvYCC
--range xvycc # only xvYCC is supported
```

## display bpc get
Uses `NvAPI_DISP_BpcConfiguration` with `NV_BPC_CONFIG_CMD_GET` to read current scanout bits-per-component and related flags.


## display bpc set
Uses `NvAPI_DISP_BpcConfiguration` with `NV_BPC_CONFIG_CMD_SET` to update BPC and related testing flags.

```powershell
--bpc default|6|8|10|12|16 # requested bits per component
--dither 0|1 # 1 enables dithering (ditherOff = 0)
--force-link 0|1 # force change at current link config (testing)
--force-rg-div 0|1 # force RG div mode (testing)
```

## display blanking get
Uses `NvAPI_DISP_GetDisplayBlankingState` to query blanking state. This feature is Quadro-only per `nvapi.h`.

```powershell
# Quadro-only, may return NVAPI_NOT_SUPPORTED on GeForce
```

## display blanking set
Uses `NvAPI_DISP_SetDisplayBlankingState` to blank or unblank a display and optionally persist across hotplug events.

```powershell
--state 0|1 # 1 blanks the display, 0 unblanks
--persist 0|1 # persist blanking across hotplug/unplug
# another process may lock blanking and return NVAPI_ACCESS_DENIED
```

## display hdr caps
Uses `NvAPI_Disp_GetHdrCapabilities` to report HDR capability flags and static metadata from the display.

```powershell
# returns HDR/EDR/Dolby Vision capability flags and display metadata
```

## display hdr session get
Uses `NvAPI_Disp_HdrSessionControl` with `NV_HDR_CONTROL_CMD_GET` to report HDR session and HDR state.


## display hdr session set
Uses `NvAPI_Disp_HdrSessionControl` with `NV_HDR_CONTROL_CMD_SET` to enable or disable the HDR session. `--expire` controls the valid time window when HDR can be enabled on displays without native HDR support.

```powershell
--enable 0|1 # enable or disable HDR session
--expire SEC # optional expiry time in seconds
```

## display hdr color get
Uses `NvAPI_Disp_HdrColorControl` with `NV_HDR_CMD_GET` to read HDR mode, color format, dynamic range, and BPC, plus static metadata.


## display hdr color set
Uses `NvAPI_Disp_HdrColorControl` with `NV_HDR_CMD_SET` to adjust HDR mode or color encoding parameters.

```powershell
--mode MODE # off|uhda|uhda-passthrough|dolby-vision|edr|sdr|uhda-nb
--format FORMAT # rgb|yuv422|yuv444|yuv420|auto|default
--range RANGE # vesa|cea|auto
--bpc BPC # 6|8|10|12|16|default
--os-hdr default|on|off # OS HDR state toggle (intended for control-panel use)
# Dolby Vision enforces specific format/range/bpc combinations
# HDR mode ignores format/range/bpc fields per nvapi.h guidance
```

## display id-by-name
Uses `NvAPI_DISP_GetDisplayIdByDisplayName` to map a Windows display name like `\\DISPLAY1` to a displayId. The display must be active, in clone/Surround, NVAPI returns the primary or top-left display.

```powershell
--name NAME # Windows display name (e.g., "\\DISPLAY1")
```

## display gdi-primary
Uses `NvAPI_DISP_GetGDIPrimaryDisplayId` to report the displayId of the GDI primary display. Returns `NVAPI_NVIDIA_DEVICE_NOT_FOUND` if the GDI primary is not on an NVIDIA GPU.


## display handle-from-id
Uses `NvAPI_DISP_GetDisplayHandleFromDisplayId` to map a displayId to a display handle.


## display id-from-handle
Uses `NvAPI_DISP_GetDisplayIdFromDisplayHandle` to map a display handle index to a displayId.

```powershell
--handle-index N # display handle index from display list
```
