# Stereo Group

Covers the `nvapi-cli stereo` command group (`src/cli/stereo.cpp`). Many commands create a temporary D3D11 device and stereo handle via `NvAPI_Stereo_CreateHandleFromIUnknown`, they fail if no NVIDIA adapter is available. `--handle-index N` refers to a display handle index from `display list` and defaults to 0 when omitted. `--monitor-index N` refers to the Windows monitor enumeration order, `--id HEX` can be used instead and is omitted from blocks below for brevity. `--id HEX` (displayId) is required for `stereo caps monitor` and `stereo dongle status` but is omitted from blocks below for brevity.

```powershell
nvapi-cli stereo enable
nvapi-cli stereo disable
nvapi-cli stereo is-enabled
nvapi-cli stereo windowed get
nvapi-cli stereo windowed set --mode off|auto|persistent [--flags N]
nvapi-cli stereo windowed supported
nvapi-cli stereo caps get
nvapi-cli stereo caps internal
nvapi-cli stereo caps monitor --monitor-index N
nvapi-cli stereo info [--handle-index N]
nvapi-cli stereo app-info [--handle-index N]
nvapi-cli stereo mode-enum get
nvapi-cli stereo mode-enum set --command enable|disable [--data N]
nvapi-cli stereo accessory
nvapi-cli stereo dongle control --command NAME [--data N] [--handle-index N]
nvapi-cli stereo dongle status --param N
nvapi-cli stereo aegis --panel-id N
nvapi-cli stereo default-profile get
nvapi-cli stereo default-profile set --name NAME
nvapi-cli stereo profile create --type default|dx9|dx10
nvapi-cli stereo profile delete --type default|dx9|dx10
nvapi-cli stereo profile set --type default|dx9|dx10 --id convergence|frustum (--dword N|--float F)
nvapi-cli stereo profile delete-value --type default|dx9|dx10 --id convergence|frustum
nvapi-cli stereo driver-mode set --mode automatic|direct
nvapi-cli stereo activate
nvapi-cli stereo deactivate
nvapi-cli stereo is-activated
nvapi-cli stereo separation get
nvapi-cli stereo separation set --value PCT
nvapi-cli stereo separation inc
nvapi-cli stereo separation dec
nvapi-cli stereo convergence get
nvapi-cli stereo convergence set --value F
nvapi-cli stereo convergence inc
nvapi-cli stereo convergence dec
nvapi-cli stereo frustum get
nvapi-cli stereo frustum set --mode none|stretch|clear-edges
nvapi-cli stereo capture jpeg --quality 0-100
nvapi-cli stereo capture png
nvapi-cli stereo init-activation --flag immediate|delayed
nvapi-cli stereo trigger-activation
nvapi-cli stereo reverse-blit --enable 0|1
nvapi-cli stereo notify --hwnd HEX --message-id N
nvapi-cli stereo active-eye set --eye left|right|mono
nvapi-cli stereo eye-separation get
nvapi-cli stereo cursor supported
nvapi-cli stereo cursor get
nvapi-cli stereo cursor set --value PCT
nvapi-cli stereo surface get
nvapi-cli stereo surface set --mode auto|force-stereo|force-mono
nvapi-cli stereo debug last-draw
nvapi-cli stereo force-to-screen --enable 0|1
nvapi-cli stereo video-control --layout NAME --client-id N --enable 0|1
nvapi-cli stereo video-metadata --width W --height H --src HEX --dst HEX
nvapi-cli stereo handshake challenge
nvapi-cli stereo handshake response --guid GUID --response-hex HEX [--flags N]
nvapi-cli stereo handshake-trigger
nvapi-cli stereo handshake-message --enable 0|1
nvapi-cli stereo profile-name set --name NAME [--flags N]
nvapi-cli stereo diag
nvapi-cli stereo shader set --stage vs|ps --type f|i|b --start N --count N --mono PATH --left PATH --right PATH
nvapi-cli stereo shader get --stage vs|ps --type f|i|b --start N --count N --mono PATH --left PATH --right PATH
```

# Command Reference

## stereo enable/disable
Uses `NvAPI_Stereo_Enable` or `NvAPI_Stereo_Disable` to toggle global stereo support.

## stereo is-enabled
Uses `NvAPI_Stereo_IsEnabled` to report whether stereo is globally enabled.

## stereo windowed
Uses `NvAPI_Stereo_GetWindowedMode`, `NvAPI_Stereo_SetWindowedMode`, and `NvAPI_Stereo_IsWindowedModeSupported` to query or configure windowed stereo mode.

```powershell
--mode off|auto|persistent # required for windowed set
--flags N # optional windowed mode flags
```

## stereo caps
Uses `NvAPI_Stereo_GetStereoCaps` or `NvAPI_Stereo_GetStereoCapsInternal` for global capabilities, or `NvAPI_Stereo_GetStereoSupport` for a specific monitor.

```powershell
--monitor-index N # required for caps monitor (or use --id HEX)
# caps get/internal have no flags
```

## stereo info
Uses `NvAPI_Stereo_GetInfo` (`NV_STEREO_DIAG_INFO`) to report stereo status, per-head config flags, and emitter state for a display handle.

```powershell
--handle-index N # display handle index from display list (defaults to 0)
```

## stereo app-info
Uses `NvAPI_Stereo_GetAppInfo` (`NV_STEREO_DIAG_APP_INFO`) to report stereo-aware application entries for a display handle.

```powershell
--handle-index N # display handle index from display list (defaults to 0)
```

## stereo mode-enum
Uses `NvAPI_Stereo_ModeEnumControl` to query or modify stereo mode enumeration behavior.

```powershell
--command enable|disable # required for mode-enum set
--data N # optional control data
```

## stereo accessory
Uses `NvAPI_Stereo_IsAccessoryDisplayEnabled` to report whether the accessory display is enabled.


## stereo dongle
Uses `NvAPI_Stereo_Dongle_Status` to query a display's dongle status, or `NvAPI_Stereo_DongleControl` to send dongle control commands to a display handle.

```powershell
--command NAME # poll|supported|set-timing|activate|deactivate|aegis-surround|active-count|supported-device-independent
--data N # optional command data for dongle control
--handle-index N # display handle index for dongle control (defaults to 0)
--param N # required for dongle status
# dongle status also requires --id HEX (displayId)
```

## stereo aegis
Uses `NvAPI_Stereo_IsDisplayAegisDTType` to detect Aegis DT panels by panel ID.

```powershell
--panel-id N # panel identifier
```

## stereo default-profile
Uses `NvAPI_Stereo_GetDefaultProfile` and `NvAPI_Stereo_SetDefaultProfile` to manage the default stereo profile name.

```powershell
--name NAME # required for default-profile set
```

## stereo profile
Uses `NvAPI_Stereo_CreateConfigurationProfileRegistryKey`, `NvAPI_Stereo_DeleteConfigurationProfileRegistryKey`, `NvAPI_Stereo_SetConfigurationProfileValue`, and `NvAPI_Stereo_DeleteConfigurationProfileValue` to manage profile keys and values.

```powershell
--type default|dx9|dx10 # registry profile type
--id convergence|frustum # registry value ID
--dword N # dword value for set
--float F # float value for set
# choose either --dword or --float for profile set
```

## stereo driver-mode
Uses `NvAPI_Stereo_SetDriverMode` to select stereo driver mode.

```powershell
--mode automatic|direct # driver mode
```

## stereo activate/deactivate/is-activated
Uses `NvAPI_Stereo_Activate`, `NvAPI_Stereo_Deactivate`, or `NvAPI_Stereo_IsActivated` on a stereo handle created from a D3D device.


## stereo separation
Uses `NvAPI_Stereo_GetSeparation`, `NvAPI_Stereo_SetSeparation`, `NvAPI_Stereo_IncreaseSeparation`, and `NvAPI_Stereo_DecreaseSeparation` to control stereo separation.

```powershell
--value PCT # required for separation set (percent as float)
```

## stereo convergence
Uses `NvAPI_Stereo_GetConvergence`, `NvAPI_Stereo_SetConvergence`, `NvAPI_Stereo_IncreaseConvergence`, and `NvAPI_Stereo_DecreaseConvergence` to control convergence distance.

```powershell
--value F # required for convergence set (float)
```

## stereo frustum
Uses `NvAPI_Stereo_GetFrustumAdjustMode` and `NvAPI_Stereo_SetFrustumAdjustMode` to configure frustum adjustment.

```powershell
--mode none|stretch|clear-edges # required for frustum set
```

## stereo capture
Uses `NvAPI_Stereo_CaptureJpegImage` or `NvAPI_Stereo_CapturePngImage` to capture a stereo screenshot.

```powershell
--quality 0-100 # optional for jpeg (defaults to 90)
```

## stereo init-activation
Uses `NvAPI_Stereo_InitActivation` to configure immediate or delayed activation.

```powershell
--flag immediate|delayed # activation mode
```

## stereo trigger-activation
Uses `NvAPI_Stereo_Trigger_Activation` to trigger activation after delayed init.


## stereo reverse-blit
Uses `NvAPI_Stereo_ReverseStereoBlitControl` to enable or disable reverse stereo blit.

```powershell
--enable 0|1 # enable or disable reverse blit
```

## stereo notify
Uses `NvAPI_Stereo_SetNotificationMessage` to register a window handle and message ID for stereo notifications.

```powershell
--hwnd HEX # HWND value as hex
--message-id N # Windows message ID
```

## stereo active-eye
Uses `NvAPI_Stereo_SetActiveEye` to force the active eye for rendering.

```powershell
--eye left|right|mono # required
```

## stereo eye-separation
Uses `NvAPI_Stereo_GetEyeSeparation` to query physical eye separation in meters.


## stereo cursor
Uses `NvAPI_Stereo_Is3DCursorSupported`, `NvAPI_Stereo_GetCursorSeparation`, and `NvAPI_Stereo_SetCursorSeparation` to manage 3D cursor behavior.

```powershell
--value PCT # required for cursor set (percent as float)
```

## stereo surface
Uses `NvAPI_Stereo_GetSurfaceCreationMode` and `NvAPI_Stereo_SetSurfaceCreationMode` to control stereo surface creation policy.

```powershell
--mode auto|force-stereo|force-mono # required for surface set
```

## stereo debug last-draw
Uses `NvAPI_Stereo_Debug_WasLastDrawStereoized` to report whether the last draw call was stereoized.


## stereo force-to-screen
Uses `NvAPI_Stereo_ForceToScreenDepth` to force depth to the screen plane.

```powershell
--enable 0|1 # enable or disable force-to-screen depth
```

## stereo video-control
Uses `NvAPI_Stereo_SetVideoControl` (`NVAPI_STEREO_VIDEO_CONTROL`) to configure stereo video layout for a client ID.

```powershell
--layout NAME # left-right|right-left|top-bottom|bottom-top|mono
--client-id N # client identifier
--enable 0|1 # enable or disable stereo video
```

## stereo video-metadata
Uses `NvAPI_Stereo_SetVideoMetadata` (`NVAPI_STEREO_VIDEO_METADATA`) to describe stereo video buffers.

```powershell
--width W --height H # video dimensions
--src HEX --dst HEX # source/destination surface handles
```

## stereo handshake
Uses `NvAPI_Stereo_AppHandShake` to issue handshake challenge or response sequences.

```powershell
--guid GUID # required for handshake response
--response-hex HEX # required for handshake response, size NVAPI_STEREO_HANDSHAKE_RESPONSE_SIZE
--flags N # optional response flags
```

## stereo handshake-trigger
Uses `NvAPI_Stereo_HandShake_Trigger_Activation` to activate after a handshake.


## stereo handshake-message
Uses `NvAPI_Stereo_HandShake_Message_Control` to enable or disable handshake messaging.

```powershell
--enable 0|1 # enable or disable messages
```

## stereo profile-name
Uses `NvAPI_Stereo_SetProfileName` to set the active profile name for the current stereo handle.

```powershell
--name NAME # required
--flags N # optional flags for profile selection
```

## stereo diag
Uses `NvAPI_Stereo_GetStereoDiag` to retrieve KMD diagnostic data such as dongle active count and head config masks.


## stereo shader
Uses `NvAPI_Stereo_Set*ShaderConstant*` or `NvAPI_Stereo_Get*ShaderConstant*` to set or read shader constants for mono/left/right.

```powershell
--stage vs|ps # vertex or pixel shader stage
--type f|i|b # float, int, or bool constants
--start N --count N # start register and count
--mono PATH --left PATH --right PATH # binary files for mono/left/right data
# f/i use count*4 elements, b uses count elements
```
