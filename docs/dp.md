# DP Group

Covers the `nvapi-cli dp` command group (`src/cli/dp.cpp`). `--id HEX` identifies a displayId (from `display ids`) and is omitted from the command list and blocks below for brevity. `--index N` selects a GPU index for `dp dongle` and is omitted from the blocks below for brevity. `--handle-index N` refers to a display handle index from `display list`. `--output-id HEX` is a display output bitmask, for `dp info` it is optional, for `dp set` it is required unless `--id` is used, and for `dp dongle` it is required.

```powershell
nvapi-cli dp info [--handle-index N] [--output-id HEX]
nvapi-cli dp set [--handle-index N] [--output-id HEX] [--link-rate RATE] [--lane-count N]
   [--format FORMAT] [--range RANGE] [--colorimetry MODE] [--bpc BPC] [--hpd 0|1] [--defer 0|1]
   [--chroma-lpf-off 0|1] [--dither-off 0|1] [--test-link-train 0|1] [--test-color-change 0|1]
nvapi-cli dp dongle --output-id HEX
nvapi-cli dp topology [--flags HEX]
```

# Command Reference

## dp info
Uses `NvAPI_GetDisplayPortInfo` (`NV_DISPLAY_PORT_INFO`) to report DPCD version, current and maximum link rates/lanes, current color format/range/colorimetry/BPC, and capability flags (DP, internal DP, color control support, and extended colorimetry support).

If you omit both `--id` and `--output-id`, the CLI uses `--handle-index` (default 0) and calls `NvAPI_GetAssociatedDisplayOutputId` to resolve the outputId.

```powershell
--handle-index N # display handle index from display list (defaults to 0)
--output-id HEX # output bitmask or displayId (optional when --id is used)
# output-id must be a single-bit mask when using legacy output IDs
```

## dp set
Uses `NvAPI_GetDisplayPortInfo` to pull the current link settings, then applies overrides via `NvAPI_SetDisplayPort` (`NV_DISPLAY_PORT_CONFIG`). At least one setting flag must be provided, the CLI rejects a call that specifies no changes.

```powershell
--handle-index N # display handle index from display list (defaults to 0)
--output-id HEX # output bitmask or displayId (required unless --id is provided)
--link-rate RATE # 1.62|2.70|5.40|8.10|2.16|2.43|3.24|4.32 or numeric enum
--lane-count N # 1|2|4 or numeric enum
--format FORMAT # rgb|ycbcr422|ycbcr444
--range RANGE # vesa|cea
--colorimetry MODE # rgb|itu601|itu709
--bpc BPC # default|6|8|10|12|16
--hpd 0|1 # set NV_DISPLAY_PORT_CONFIG::isHPD
--defer 0|1 # set NV_DISPLAY_PORT_CONFIG::isSetDeferred (requires OS modeset)
--chroma-lpf-off 0|1 # force chroma LPF off
--dither-off 0|1 # force dither off
--test-link-train 0|1 # testing mode: skip validation
--test-color-change 0|1 # testing mode: skip validation
# --id HEX can be used instead of --output-id to target a displayId
```

## dp dongle
Uses `NvAPI_GPU_Get_DisplayPort_DongleInfo` (`NV_NVAPI_GET_DP_DONGLE_INFO`) to report DP-to-DVI/HDMI/VGA conversion status and DMS dongle detection for a specific output mask.

```powershell
--output-id HEX # required output mask (single-bit display output ID)
```

## dp topology
Uses `NvAPI_GPU_QueryDPTopology` (`NV_DP_NODE_INFO`) to enumerate MST nodes behind a DP connector, including node type, GUID, flag bits (MST, video/audio sink, HDCP, loop/zombie), and branch port masks.

The CLI rejects `--flags` because this build does not expose DP topology flags.

```powershell
--flags HEX # not supported by this build, use only if your NVAPI supports DP topology flags
# --id HEX is required to specify the target displayId
```
