# OGL Group

Covers the `nvapi-cli ogl` command group (`src/cli/ogl.cpp`). The CLI creates a hidden window and OpenGL context, makes it current, and then calls the NVAPI OpenGL expert APIs. `expert get/set` are per-context, `defaults-get/set` are global defaults for new contexts. Masks are bitmasks composed from `NVAPI_OGLEXPERT_DETAIL_*`, `NVAPI_OGLEXPERT_REPORT_*`, and `NVAPI_OGLEXPERT_OUTPUT_*` in `nvapi.h`.

```powershell
nvapi-cli ogl expert get
nvapi-cli ogl expert set --detail MASK --report MASK --output MASK
nvapi-cli ogl expert defaults-get
nvapi-cli ogl expert defaults-set --detail MASK --report MASK --output MASK
```

# Command Reference

## ogl expert get
Uses `NvAPI_OGL_ExpertModeGet` to query the current expert detail, report, and output masks for the active OpenGL context.

## ogl expert set
Uses `NvAPI_OGL_ExpertModeSet` to apply expert detail, report, and output masks for the active OpenGL context.

```powershell
--detail MASK # NVAPI_OGLEXPERT_DETAIL_* mask
--report MASK # NVAPI_OGLEXPERT_REPORT_* mask
--output MASK # NVAPI_OGLEXPERT_OUTPUT_* mask
```

## ogl expert defaults-get
Uses `NvAPI_OGL_ExpertModeDefaultsGet` to query the global default expert masks applied to new OpenGL contexts.


## ogl expert defaults-set
Uses `NvAPI_OGL_ExpertModeDefaultsSet` to update the global default expert masks applied to new OpenGL contexts.

```powershell
--detail MASK # NVAPI_OGLEXPERT_DETAIL_* mask
--report MASK # NVAPI_OGLEXPERT_REPORT_* mask
--output MASK # NVAPI_OGLEXPERT_OUTPUT_* mask
# output-to-callback is ignored by defaults-set per nvapi.h
```
