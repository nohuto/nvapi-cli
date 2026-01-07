# SLI Group

Covers the `nvapi-cli sli` command group (`src/cli/sli.cpp`). Many NVAPI structs are versioned in `nvapi.h`, it uses the latest version and falls back on older versions when required.

```
nvapi-cli sli status
nvapi-cli sli views
```

# Command Reference

## sli status
Uses `NvAPI_SYS_GetChipSetSliBondInfo` to print chipset bond info, then enumerates physical and logical GPUs to show SLI groupings via `NvAPI_EnumPhysicalGPUs`, `NvAPI_EnumLogicalGPUs`, and `NvAPI_GetPhysicalGPUsFromLogicalGPU`. It also lists connected display IDs per physical GPU using `NvAPI_GPU_GetConnectedDisplayIds`.

```c
// prints logical GPU group membership and connected display IDs
```

## sli views
Uses `NvAPI_GPU_GetSupportedSLIViews` to list supported SLI view modes for each GPU. The CLI prints both the numeric mode and a friendly name.