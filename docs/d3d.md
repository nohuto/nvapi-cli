# D3D Group

Covers the `nvapi-cli d3d` command group (`src/cli/d3d.cpp`). The CLI creates a temporary D3D11 device on the first NVIDIA adapter it finds, commands fail if no NVIDIA adapter is available. `--swapchain` builds a hidden window + swapchain to obtain a surface handle, `--surface` supplies an existing `NVDX_ObjectHandle`. `--present` only matters with `--swapchain` and issues a `Present` before the VRR call, `--debug` writes `nvapi-cli_d3d_vrr.log`.

```powershell
nvapi-cli d3d vrr get [--swapchain] [--surface HANDLE] [--present] [--debug]
nvapi-cli d3d vrr set --state on|off [--swapchain] [--surface HANDLE] [--present] [--debug]
nvapi-cli d3d latency get [--out PATH] [--raw]
nvapi-cli d3d latency mark --frame ID --type TYPE
nvapi-cli d3d ansel set [--modifier none|ctrl|shift|alt] [--enable-key VK] [--feature NAME:STATE[:VK]]
```

# Command Reference

## d3d vrr get
Uses `NvAPI_D3D_GetVRRState` to query VRR enabled/requested state for a device/context and a primary surface handle.

```powershell
--swapchain # create a hidden swapchain and derive the surface handle
--surface HANDLE # use an existing NVDX_ObjectHandle instead of a swapchain
--present # call Present on the hidden swapchain before querying
--debug # log verbose steps to nvapi-cli_d3d_vrr.log
```

## d3d vrr set
Uses `NvAPI_D3D_SetVRRState` to enable or disable VRR for a device/context and primary surface handle.

```powershell
--state on|off # enable or disable VRR
--swapchain # create a hidden swapchain and derive the surface handle
--surface HANDLE # use an existing NVDX_ObjectHandle instead of a swapchain
--present # call Present on the hidden swapchain before setting
--debug # log verbose steps to nvapi-cli_d3d_vrr.log
```

## d3d latency get
Uses `NvAPI_D3D_GetLatency` (`NV_LATENCY_RESULT_PARAMS`) to retrieve latency reports. The CLI prints the most recent non zero frame report and can optionally dump the struct to a file or hex output.

```powershell
--out PATH # write raw NV_LATENCY_RESULT_PARAMS to a binary file
--raw # dump the full struct as hex
# use d3d latency mark with incrementing frame IDs for valid reports
```

## d3d latency mark
Uses `NvAPI_D3D_SetLatencyMarker` (`NV_LATENCY_MARKER_PARAMS`) to insert a latency marker for a frame ID.

```powershell
--frame ID # monotonically increasing frame ID
--type TYPE # simulation-start|simulation-end|rendersubmit-start|rendersubmit-end|present-start|present-end|input-sample|trigger-flash|pc-latency-ping
```

## d3d ansel set
Uses `NvAPI_D3D_ConfigureAnsel` (`NVAPI_ANSEL_CONFIGURATION_STRUCT`) to configure Ansel hotkeys and feature toggles. Features can be specified multiple times.

```powershell
--modifier none|ctrl|shift|alt # hotkey modifier for the Ansel enable key
--enable-key VK # virtual key code for enabling/disabling Ansel
--feature NAME:STATE[:VK] # NAME=black-and-white|hudless, STATE=enable|disable, optional per-feature VK
```
