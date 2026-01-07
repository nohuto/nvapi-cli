# GSync Group

Covers the `nvapi-cli gsync` command group (`src/cli/gsync.cpp`). `--index N` is required for most GSync commands and is omitted from the command list and blocks below for brevity, get it from `gsync list`. `--display-id HEX` is required for `gsync sync set` and is omitted from the block below for brevity, get it from `gsync topo` or `display ids`. Many NVAPI structs are versioned in `nvapi.h`, the CLI uses the latest version and falls back on older versions when required.

```powershell
nvapi-cli gsync list
nvapi-cli gsync caps
nvapi-cli gsync topo
nvapi-cli gsync sync get
nvapi-cli gsync sync set --state master|slave|unsynced [--no-validate] [--send-start-event]
nvapi-cli gsync status [--gpu-index N]
nvapi-cli gsync control get
nvapi-cli gsync control set [--polarity rising|falling|both] [--video-mode none|ttl|ntsc|hdtv|composite]
   [--interval N] [--source vsync|housesync] [--interlace 0|1] [--sync-source-output 0|1]
```

# Command Reference

## gsync list
Uses `NvAPI_GSync_EnumSyncDevices` to enumerate connected G-Sync sync devices and their handles.

## gsync caps
Uses `NvAPI_GSync_QueryCapabilities` (`NV_GSYNC_CAPABILITIES`) to report board revision and timing capability limits.

```powershell
# no flags
# reports boardId, revision, maxSyncSkew, syncSkewResolution, maxStartDelay, startDelayResolution
```

## gsync topo
Uses `NvAPI_GSync_GetTopology` (`NV_GSYNC_GPU`, `NV_GSYNC_DISPLAY`) to list GPUs and displays attached to the sync device. Displays report whether they are masterable and their current sync state.

## gsync sync get
Uses `NvAPI_GSync_GetTopology` to report the current sync state (master/slave/unsynced) for all displays in the topology.

## gsync sync set
Uses `NvAPI_GSync_GetTopology` to build the current display list, updates the target display state, then calls `NvAPI_GSync_SetSyncStateSettings`. Flags map to `NV_SET_SYNC_FLAGS_*`.

```powershell
--state master|slave|unsynced # desired sync state for the target display
--no-validate # set NV_SET_SYNC_FLAGS_NO_VALIDATION
--send-start-event # set NV_SET_SYNC_FLAGS_SEND_START_EVENT
```

## gsync status
Uses `NvAPI_GSync_GetSyncStatus` (`NV_GSYNC_STATUS`) and `NvAPI_GSync_GetStatusParameters` (`NV_GSYNC_STATUS_PARAMS`) to report timing sync state and house sync parameters. If `--gpu-index` is omitted, the CLI uses the first GPU from the topology.

```powershell
--gpu-index N # optional GPU index override for sync status
```

## gsync control get
Uses `NvAPI_GSync_GetControlParameters` (`NV_GSYNC_CONTROL_PARAMS`) to read control settings such as polarity, video mode, interval, source, and sync skew/start delay.

## gsync control set
Uses `NvAPI_GSync_SetControlParameters` to update control settings, the CLI reads the current params first and then overwrites only the specified fields.

```powershell
--polarity rising|falling|both # sync edge selection
--video-mode none|ttl|ntsc|hdtv|composite # sync video mode
--interval N # pulses to wait between framelock signals
--source vsync|housesync # sync source selection
--interlace 0|1 # interlace mode
--sync-source-output 0|1 # house sync output mode (board-specific)
# sync-source-output is valid only for certain boards when source is VSYNC
```
