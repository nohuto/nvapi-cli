# SYS Group

Covers the `nvapi-cli sys` command group (`src/cli/sys.cpp`). `sys smp --default` returns the default SMP values instead of the current values.

```powershell
nvapi-cli sys gpu-count
nvapi-cli sys smp [--default]
nvapi-cli sys chipset-sli
```

# Command Reference

## sys gpu-count
Uses `NvAPI_SYS_GetGpuCount` (`NV_GPU_COUNT_DATA`) to report the total GPU count found on the bus and the number of active GPUs.

## sys smp
Uses `NvAPI_SYS_GetSMPInfo` (`NV_SMP_INFO`) to report SMP settings such as tilt angle, vertical FOV, and camera mode.

```powershell
--default # set NV_SMP_INFO::bDefault to fetch defaults
```

## sys chipset-sli
Uses `NvAPI_SYS_GetChipSetSliBondInfo` (`NV_CHIPSET_SLI_BOND_INFO`) to report the chipset SLI bond ID and its human-readable name (which may be empty).