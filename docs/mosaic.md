# Mosaic Group

Covers the `nvapi-cli mosaic` command group (`src/cli/mosaic.cpp`). `--index N` is optional for `mosaic caps` and is omitted from the blocks below for brevity, when omitted, the CLI enumerates all physical GPUs. Many NVAPI structs are versioned in `nvapi.h`, the CLI uses the latest version and falls back on older versions when required.

```powershell
nvapi-cli mosaic caps
nvapi-cli mosaic supported [--type all|basic|passive-stereo|scaled-clone|passive-stereo-scaled-clone] [--limit N]
nvapi-cli mosaic current
nvapi-cli mosaic enable --state 0|1
nvapi-cli mosaic display-caps [--limit N]
```

# Command Reference

## mosaic caps
Uses `NvAPI_Mosaic_GetSingleGpuMosaicCaps` (`NV_MOSAIC_CAPS`) to report Mosaic capabilities for each GPU. When multiple GPUs are enumerated, the CLI prints a header per GPU and the capability flags returned by NVAPI.

```powershell
# reports bSupportsMosaic, bSupportsImmersiveGaming, passive stereo, overlap/gap, pixel shift, mirroring
```

## mosaic supported
Uses `NvAPI_Mosaic_GetSupportedTopoInfo` (`NV_MOSAIC_SUPPORTED_TOPO_INFO`) to list supported topologies and common per-display settings. Topologies may be present but not currently possible, check `isPossible`. NVAPI notes that scaled-clone types are not supported at this time.

```powershell
--type all|basic|passive-stereo|scaled-clone|passive-stereo-scaled-clone # NV_MOSAIC_TOPO_TYPE filter
--limit N # limit the number of display settings printed (default 10)
# isPossible=0 means the topology exists but cannot be enabled with current hardware/config
```

## mosaic current
Uses `NvAPI_Mosaic_GetCurrentTopo` to report the current topology, per-display settings, and overlap values. If there is no active topology, NVAPI returns `NV_MOSAIC_TOPO_NONE`.

```powershell
# overlapX/overlapY are pixel overlaps between displays
```

## mosaic enable
Uses `NvAPI_Mosaic_EnableCurrentTopo` to enable or disable the current topology. Disabling preserves the topology so it can be re-enabled later.

```powershell
--state 0|1 # enable or disable the current topology
# enabling requires the current topology to be possible
```

## mosaic display-caps
Uses `NvAPI_Mosaic_GetDisplayCapabilities` (`NV_MOSAIC_DISPLAY_CAPS`) to retrieve common timing settings and problem flags. The CLI prints the display settings list and does not surface per-display problem flags.

```powershell
--limit N # limit the number of display settings printed (default 10)
# problemFlags include NO_EDID_AVAILABLE, MISMATCHED_OUTPUT_TYPE, NO_SLI_BRIDGE, etc.
```
