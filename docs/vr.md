# VR Group

Covers the `nvapi-cli vr` command group (`src/cli/vr.cpp`). `--vendor-id` is the vendor ID for the VR solution (as expected by NVAPI direct mode APIs). Direct mode handles are `NV_DIRECT_MODE_DISPLAY_HANDLE` structs with `displayId` and a vendor context pointer. `--context HEX` should be the pointer printed by `vr direct-mode list` (the `pDisplayContext` field).

```
nvapi-cli vr direct-mode enable --vendor-id ID
nvapi-cli vr direct-mode disable --vendor-id ID
nvapi-cli vr direct-mode list --vendor-id ID [--flag capable|enabled]
nvapi-cli vr direct-mode handle-from-id --display-id HEX
nvapi-cli vr direct-mode id-from-handle --display-id HEX --context HEX
```

# Command Reference

## vr direct-mode enable
Uses `NvAPI_DISP_EnableDirectMode` to enable VR direct mode for the vendor ID.

```c
--vendor-id ID // VR vendor identifier
// may return NVAPI_INVALID_DIRECT_MODE_DISPLAY if not allowlisted
```

## vr direct-mode disable
Uses `NvAPI_DISP_DisableDirectMode` to disable VR direct mode for the vendor ID.

```c
--vendor-id ID // VR vendor identifier
// may return NVAPI_INVALID_DIRECT_MODE_DISPLAY if not allowlisted
```

## vr direct-mode list
Uses `NvAPI_DISP_EnumerateDirectModeDisplays` to enumerate VR-capable or direct-mode-enabled displays for a vendor. The CLI calls the API twice (count, then populate).

```c
--vendor-id ID // VR vendor identifier
--flag capable|enabled // enumerate VR-capable or direct-mode-enabled displays
// uses NV_ENUM_DIRECTMODE_DISPLAY_FLAG
```

## vr direct-mode handle-from-id
Uses `NvAPI_DISP_GetDirectModeDisplayHandleFromDisplayId` to map a displayId to a direct mode display handle.

```c
--display-id HEX // displayId to map to a direct mode handle
```

## vr direct-mode id-from-handle
Uses `NvAPI_DISP_GetDisplayIdFromDirectModeDisplayHandle` to map a direct mode display handle back to a displayId.

```c
--display-id HEX // displayId field of the handle
--context HEX // context pointer field of the handle (pDisplayContext)
```
