# PCF Group

Covers the `nvapi-cli pcf` command group (`src/cli/pcf.cpp`). PCF calls require initializing the platform driver interface via `NvAPI_InitializeEx(NV_PLATFORM_DRIVER)`. The CLI opens a short lived session per command and unloads the platform driver afterward. `--index N` is required for `pcf master set` and is omitted from the blocks below for brevity. PCF master objects are indexed within a 32-entry board object mask (`NV_PCF_BOARDOBJGRP_E32_MAX_OBJECTS`).

```
nvapi-cli pcf master info
nvapi-cli pcf master control
nvapi-cli pcf master status
nvapi-cli pcf master set --bus-high N --bus-nominal N
```

# Command Reference

## pcf master info
Uses `NvAPI_PCF_MasterGetInfo` (`NV_PCF_MASTER_INFO_PARAMS`) to enumerate PCF master objects and report their type-specific info. The CLI prints CPU frequency control bus ratios, CPU data collector metadata, WM2 platform fields, and CPU TDP limit metadata where available.

```c
// no flags
// requires NvAPI_InitializeEx(NV_PLATFORM_DRIVER)
```

## pcf master control
Uses `NvAPI_PCF_MasterGetControl` (`NV_PCF_MASTER_CONTROL_PARAMS`) to report current control values for each master object. The CLI prints CPU frequency control bus ratios, WM2 platform control values, and CPU data collector/TDP limit control fields when present.

```c
// no flags
// requires NvAPI_InitializeEx(NV_PLATFORM_DRIVER)
```

## pcf master status
Uses `NvAPI_PCF_MasterGetStatus` (`NV_PCF_MASTER_STATUS_PARAMS`) to report status for PCF masters. The CLI currently prints WM2 platform status, including CPU/GPU temperatures and system power, plus a running update counter.

```c
// no flags
// requires NvAPI_InitializeEx(NV_PLATFORM_DRIVER)
```

## pcf master set
Uses `NvAPI_PCF_MasterGetInfo` to validate the master index, `NvAPI_PCF_MasterGetControl` to load the current control, then updates CPU frequency control bus ratios and calls `NvAPI_PCF_MasterSetControl`.

Only masters of type `NV_PCF_MASTER_CPU_FREQUENCY_CONTROL_TYPE_ACPI_ECIO` are accepted. The CLI enforces 0-255 for bus ratio values and updates only the selected master index.

```c
--bus-high N // high bus ratio (0-255)
--bus-nominal N // nominal bus ratio (0-255)
// --index N required (omitted)
// at least one of --bus-high or --bus-nominal is required
```
