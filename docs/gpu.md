# GPU Group

Covers the `nvapi-cli gpu` command group (`src/cli/gpu_*.cpp` & `src/cli/info.cpp`). `--index N` is optional on most commands and is omitted from the blocks below for brevity. When omitted, it enumerates all physical GPUs via `NvAPI_EnumPhysicalGPUs` and applies the command to each one. Many structures are versioned, it retries older versions when it receives `NVAPI_INCOMPATIBLE_STRUCT_VERSION`.

```
nvapi-cli gpu list
nvapi-cli gpu memory
nvapi-cli gpu clocks
nvapi-cli gpu utilization
nvapi-cli gpu pstate
nvapi-cli gpu pstates20
nvapi-cli gpu pstates20-set --pstate P0 --clock graphics|memory|processor|video --delta-khz N
nvapi-cli gpu pstates20-set --pstate P0 --voltage core --delta-uv N
nvapi-cli gpu pstates20-private
nvapi-cli gpu pstates20-private-set --pstate P0 --clock-id ID --delta-khz N
nvapi-cli gpu pstates20-private-set --pstate P0 --voltage-domain core|fb|cold-core|core-nominal|ID --delta-uv N
nvapi-cli gpu pstates20-private-set --pstate P0 --voltage-domain core|fb|cold-core|core-nominal|ID --target-uv N
nvapi-cli gpu bus
nvapi-cli gpu vbios
nvapi-cli gpu cooler
nvapi-cli gpu bar
nvapi-cli gpu ecc status
nvapi-cli gpu ecc errors [--raw]
nvapi-cli gpu ecc config
nvapi-cli gpu ecc reset --current 0|1 --aggregate 0|1
nvapi-cli gpu board mfg
nvapi-cli gpu pcie info
nvapi-cli gpu pcie switch-errors
nvapi-cli gpu pcie errors
nvapi-cli gpu pcie aer
nvapi-cli gpu power
nvapi-cli gpu power limit
nvapi-cli gpu power limit-set --limit 0-255|max [--flags HEX]
nvapi-cli gpu power monitor info
nvapi-cli gpu power monitor status
nvapi-cli gpu power device info
nvapi-cli gpu power device status
nvapi-cli gpu power capping info
nvapi-cli gpu power capping slowdown
nvapi-cli gpu power leakage info
nvapi-cli gpu power leakage status
nvapi-cli gpu vf tables
nvapi-cli gpu vf inject [--flags HEX] [--clk-domain ID --clk-khz N]
   [--volt-domain logic|sram|msvdd|ID --volt-rail N --volt-uv N --volt-min-uv N]
nvapi-cli gpu vpstates info
nvapi-cli gpu vpstates control [--original]
nvapi-cli gpu vpstates set --vpstate N (--clock N --target-mhz N [--min-eff-mhz N] | --group N --value N)
nvapi-cli gpu vfe-var info
nvapi-cli gpu vfe-var control
nvapi-cli gpu vfe-var set --var N --override-type none|value|offset|scale
   [--override-value F] [--temp-hyst-pos C] [--temp-hyst-neg C]
nvapi-cli gpu vfe-equ info
nvapi-cli gpu vfe-equ control
nvapi-cli gpu vfe-equ set --equ N (--compare-func eq|gte|gt --compare-crit F | --minmax min|max | --coeffs A,B,C)
nvapi-cli gpu perf-limits info
nvapi-cli gpu perf-limits status
nvapi-cli gpu perf-limits set --limit-id ID --type disabled|pstate|freq|vpstate [--pstate P0] [--point nom|min|max|mid]
   [--freq-khz N --domain ID] [--vpstate N]
nvapi-cli gpu voltage
nvapi-cli gpu voltage control-set --enable 0|1
nvapi-cli gpu thermal
nvapi-cli gpu thermal slowdown
nvapi-cli gpu thermal slowdown-set --state enabled|disabled
nvapi-cli gpu thermal sim [--sensor N]
nvapi-cli gpu thermal sim-set --sensor N --mode enabled|disabled [--temp C]
nvapi-cli gpu fan set --cooler N --level PCT [--policy manual|perf|temp-discrete|temp-cont|temp-cont-sw|default]
nvapi-cli gpu fan restore [--cooler N]
nvapi-cli gpu client-fan coolers info
nvapi-cli gpu client-fan coolers status
nvapi-cli gpu client-fan coolers control
nvapi-cli gpu client-fan coolers set --cooler N [--level PCT] [--enable 0|1] [--default]
nvapi-cli gpu client-fan policies info
nvapi-cli gpu client-fan policies status
nvapi-cli gpu client-fan policies control
nvapi-cli gpu client-fan policies set --policy N --fan-stop 0|1 [--default]
nvapi-cli gpu client-fan arbiters info
nvapi-cli gpu client-fan arbiters status
nvapi-cli gpu client-fan arbiters control
nvapi-cli gpu client-fan arbiters set --arbiter N --fan-stop 0|1
nvapi-cli gpu client-illum devices info
nvapi-cli gpu client-illum devices control
nvapi-cli gpu client-illum devices set --device N --sync 0|1 [--timestamp-ms N]
nvapi-cli gpu client-illum zones info
nvapi-cli gpu client-illum zones control
nvapi-cli gpu client-illum zones set --zone N --mode manual-rgb|manual-rgbw|manual-single|manual-color-fixed --brightness N [--r N --g N --b N --w N] [--default]
```

# Command Reference

## gpu list
Uses `NvAPI_EnumPhysicalGPUs`, `NvAPI_GPU_GetFullName`, `NvAPI_GPU_GetPCIIdentifiers` to enumerate physical GPU handles, print names and PCI IDs. It reports up to `NVAPI_MAX_PHYSICAL_GPUS` handles, the names and PCI IDs come from separate queries.

## gpu memory
Uses `NvAPI_GPU_GetMemoryInfo` (`NV_DISPLAY_DRIVER_MEMORY_INFO`), `NvAPI_GPU_GetPhysicalFrameBufferSize`, `NvAPI_GPU_GetVirtualFrameBufferSize`, `NvAPI_GPU_GetRamType`, `NvAPI_GPU_GetRamBusWidth`, `NvAPI_GPU_GetRamBankCount` to report memory usage and RAM characteristics. The memory info struct reports sizes in KB and newer versions add eviction size/count fields, so the version may need to be downgraded on older drivers.

```c
// values are reported in KB
// newer struct versions add eviction metrics
```

## gpu clocks
Uses `NvAPI_GPU_GetAllClockFrequencies` (`NV_GPU_CLOCK_FREQUENCIES`) to report current clock frequencies per public clock domain. Each entry is indexed by `NVAPI_GPU_PUBLIC_CLOCK_*` and includes a presence flag plus a kHz frequency, `ClockType` selects current/base/boost clocks (the CLI uses current).

```c
// domain index uses NVAPI_GPU_PUBLIC_CLOCK_*
// frequency is in kHz
```

## gpu utilization
Uses `NvAPI_GPU_GetDynamicPstatesInfoEx` (`NV_GPU_DYNAMIC_PSTATES_INFO_EX`) to show dynamic utilization percentages and whether dynamic Pstates are enabled. The struct exposes a dynamic Pstate enable flag and per-domain busy percentages for GPU/FB/VID/BUS over a one second window.

```c
// utilization is a 1 second busy percentage
// domains include GPU, FB, VID, BUS
```

## gpu pstate
Uses `NvAPI_GPU_GetCurrentPstate` to report current performance Pstate. Pstates are enumerated as `NV_GPU_PERF_PSTATE_P0` through `P15`, with P0 being the highest performance state.

## gpu pstates20
Uses `NvAPI_GPU_GetPstates20` (`NV_GPU_PERF_PSTATES20_INFO`) to dump Pstates20 tables, editability flags, and ranges. Pstates 2.0 exposes the editable clocks/voltages per state and notes which parameters support overrides, not every state exists on every GPU.

```c
// P0..P15 are possible, not all are present on every GPU
```

## gpu pstates20-set
Uses `NvAPI_GPU_GetPstates20`, `NvAPI_GPU_SetPstates20` (`NV_GPU_PERF_PSTATES20_INFO`) to apply Pstates20 deltas for clocks or base voltages. NvAPI sets only delta offsets from nominal values and only for fields marked [SET].

```c
--pstate P0 // Pstate to modify (e.g., P0)
--clock graphics|memory|processor|video --delta-khz N // set clock delta in kHz
--voltage core --delta-uv N // set voltage delta in uV
// --clock and --delta-khz must be provided together
// --voltage and --delta-uv must be provided together
// The CLI validates editability and delta ranges from nvapi.h value ranges
// only fields marked [SET] are required
```

## gpu pstates20-private
Uses `NvAPI_GPU_GetPstates20Private` (`NV_GPU_PERF_PSTATES20_PRIVATE_INFO`) to dump private Pstates20 tables, including per-domain voltage entries. This is the internal/extended Pstates 2.0 table that replaces deprecated Pstate APIs and includes additional voltage detail.

This API replaces the deprecated NvAPI_GPU_GetPstates, NvAPI_GPU_GetPstatesEx calls.

## gpu pstates20-private-set
Uses `NvAPI_GPU_GetPstates20Private`, `NvAPI_GPU_SetPstates20Private` (`NV_GPU_PERF_PSTATES20_PRIVATE_INFO`) to apply private Pstate clock or voltage deltas. Like the public setter, this applies delta offsets and only honors fields marked [SET].

```c
--pstate P0 // Pstate to modify
--clock-id ID --delta-khz N // set clock delta in kHz
--voltage-domain core|fb|cold-core|core-nominal|ID --delta-uv N // set voltage delta in uV
--voltage-domain core|fb|cold-core|core-nominal|ID --target-uv N // set absolute target in uV
// --delta-uv and --target-uv are mutually exclusive
// The CLI derives the correct delta when --target-uv is used
// only fields marked [SET] are required
```

## gpu bus
Uses `NvAPI_GPU_GetPCIIdentifiers`, `NvAPI_GPU_GetBusType`, `NvAPI_GPU_GetBusId`, `NvAPI_GPU_GetBusSlotId`, `NvAPI_GPU_GetIRQ` to report PCI identifiers, bus type, bus/slot IDs, and IRQ. The PCI identifiers include device, subsystem, revision, and external IDs, plus bus type, bus ID, slot ID, and IRQ.

## gpu vbios
Uses `NvAPI_GPU_GetVbiosVersionString`, `NvAPI_GPU_GetVbiosRevision`, `NvAPI_GPU_GetVbiosOEMRevision` to report VBIOS version and revision numbers. The VBIOS string is returned as an ASCII `NvAPI_ShortString`, while the revision fields are numeric identifiers.

## gpu cooler
Uses `NvAPI_GPU_GetCoolerSettings` (`NV_GPU_GETCOOLER_SETTINGS`), `NvAPI_GPU_GetTachReading` to report cooler settings, policies, and tachometer readings. The cooler array includes level limits, policy, target, and control type, tachometer data is reported separately.

## gpu bar
Uses `NvAPI_GPU_GetBarInfo` (`NV_GPU_BAR_INFO`) to report BAR sizes and offsets. BAR info returns a count and per-BAR size/offset in bytes, capped by `NV_GPU_MAX_BAR_COUNT`.

```c
// sizes/offsets are in bytes
// max bars is NV_GPU_MAX_BAR_COUNT
```

## gpu ecc status
Uses `NvAPI_GPU_GetECCStatusInfo` (`NV_GPU_ECC_STATUS_INFO`) to show ECC support and current configuration. ECC status reports support, enablement, and the active configuration mode.

## gpu ecc errors
Uses `NvAPI_GPU_GetECCErrorInfo` / `NvAPI_GPU_GetECCErrorInfoEx` (`NV_GPU_ECC_ERROR_INFO`, `NVAPI_GPU_ECC_STATUS_FLAGS_TYPE_RAW`) to show current and aggregate ECC error counters. Counters are provided for both current and aggregate single- and double-bit errors, with `--raw` selecting the raw counter path.

```c
--raw // use NvAPI_GPU_GetECCErrorInfoEx with RAW error type
// raw uses NVAPI_GPU_ECC_STATUS_FLAGS_TYPE_RAW
```

## gpu ecc config
Uses `NvAPI_GPU_GetECCConfigurationInfo` (`NV_GPU_ECC_CONFIGURATION_INFO`) to show stored and default ECC enable state.

## gpu ecc reset
Uses `NvAPI_GPU_ResetECCErrorInfo` to reset ECC error counters. This clears current and/or aggregate ECC counters depending on the flags passed.

```c
--current 0|1 // reset current counters
--aggregate 0|1 // reset aggregate counters
// At least one of --current or --aggregate is required
```

## gpu board mfg
Uses `NvAPI_GPU_ManufacturingInfo` (`NV_MANUFACTURING_INFO`) to print manufacturing ECID. Manufacturing info returns the ECID identifier used for board tracing.

## gpu pcie info
Uses `NvAPI_GPU_GetPCIEInfo` (`NV_PCIE_INFO`) to report PCIe link speed/width/generation for root, GPU, upstream, downstream, and board. PCIe info includes link speed/width, generation, and ASPM state for root, GPU, upstream, downstream, and board links.

## gpu pcie switch-errors
Uses `NvAPI_GPU_GetPCIELinkSwitchErrorInfo` (`NV_PCIELINK_SWITCH_ERROR_INFO`) to report link speed/width switch error counters. Switch error info reports separate counters for link speed and width transitions.

## gpu pcie errors
Uses `NvAPI_GPU_ClearPCIELinkErrorInfo` (`NV_PCIE_LINK_ERROR_INFO`) to read and clear PCIe link error masks. The API returns root/GPU link error masks and clears the counters as part of the query.

```c
// This call clears counters as part of the query in NVAPI
```

## gpu pcie aer
Uses `NvAPI_GPU_ClearPCIELinkAERInfo` to read and clear PCIe AER error mask. The API returns the AER error mask and clears it when queried.

```c
// This call clears counters as part of the query in NVAPI
```

## gpu power
Uses `NvAPI_GPU_GetPowerConnectorStatus` to report power connector count and connection masks. Power connector status includes a connector count and bitmasks for boot-time and current connections.

## gpu power limit
Uses `NvAPI_GPU_GetPerfLimit` to report the current perf limit and flags. The perf limit is a 0-255 value where `NV_GPU_PERF_LIMIT_MAX` represents no limit, flags describe the limit state.

```c
// NV_GPU_PERF_LIMIT_MAX means no limit
```

## gpu power limit-set
Uses `NvAPI_GPU_SetPerfLimit` to apply a perf limit percentage. The limit value is a byte (0-255) or `NV_GPU_PERF_LIMIT_MAX`, and the optional flags control how the driver applies it.

```c
--limit 0-255|max // perf limit value or max (no limit)
--flags HEX // optional NVAPI perf limit flags (see nvapi.h)
```

## gpu power monitor info
Uses `NvAPI_GPU_PowerMonitorGetInfo` (`NV_GPU_POWER_MONITOR_GET_INFO`) to report power monitor channels and supported rails. Info exposes a channel mask and per-channel metadata like rail and type, the mask tells which entries are valid.

```c
// channelMask indicates valid channels
```

## gpu power monitor status
Uses `NvAPI_GPU_PowerMonitorGetInfo`, `NvAPI_GPU_PowerMonitorGetStatus` (`NV_GPU_POWER_MONITOR_GET_STATUS`) to report power monitor measurements per channel. Status uses the channel mask from GetInfo and returns per-channel power/current/voltage/energy plus total GPU power.

```c
// uses channelMask from GetInfo
// includes total GPU power
```

## gpu power device info
Uses `NvAPI_GPU_PowerDeviceGetInfo` (`NV_GPU_POWER_DEVICE_GET_INFO`) to report available power devices and their rails. Info provides a device mask and per-device rail/type metadata.

## gpu power device status
Uses `NvAPI_GPU_PowerDeviceGetInfo`, `NvAPI_GPU_PowerDeviceGetStatus` (`NV_GPU_POWER_DEVICE_GET_STATUS`) to report power device telemetry providers. Status returns provider tuples for each masked device, including power/current/voltage readings.

```c
// uses the device mask from GetInfo
```

## gpu power capping info
Uses `NvAPI_GPU_PowerCappingGetInfo` (`NV_GPU_POWER_CAPPING_GET_INFO`) to report power capping configuration (deprecated in NVAPI). This API is marked deprecated in `nvapi.h` and commonly returns `NVAPI_NOT_SUPPORTED`.

```c
// deprecated in nvapi.h, may return NVAPI_NOT_SUPPORTED
```

## gpu power capping slowdown
Uses `NvAPI_GPU_PowerCappingSlowdownGetStatus` (`NV_GPU_POWER_CAPPING_SLOWDOWN_GET_STATUS`) to report power capping slowdown status (deprecated in NVAPI). This API is marked deprecated in `nvapi.h` and commonly returns `NVAPI_NOT_SUPPORTED`.

```c
// deprecated in nvapi.h, may return NVAPI_NOT_SUPPORTED
```

## gpu power leakage info
Uses `NvAPI_GPU_PowerLeakageGetInfo` (`NV_GPU_POWER_LEAKAGE_INFO_PARAMS`) to report leakage tables and types. Leakage info returns a mask of available leakages plus per-entry type data.

## gpu power leakage status
Uses `NvAPI_GPU_PowerLeakageGetStatus` (`NV_GPU_POWER_LEAKAGE_STATUS_PARAMS`) to report leakage status per voltage (deprecated in NVAPI). This API is deprecated in `nvapi.h`, so drivers often report `NVAPI_NOT_SUPPORTED`.

```c
// deprecated in nvapi.h, may return NVAPI_NOT_SUPPORTED
```

## gpu vf tables
Uses `NvAPI_GPU_PerfVfTablesGetInfo` (`NV_GPU_PERF_VF_TABLES`) to dump VF table entries per clock domain and voltage. VF tables map pstate/domain ranges to entries, and each entry carries a max frequency and voltage information.

```c
// entries include maxFreqKHz and voltage in mV
```

## gpu vf inject
Uses `NvAPI_GPU_PerfVfChangeInject` (`NV_GPU_PERF_VF_CHANGE_INJECT_PARAMS`) to inject a VF table change for a clock and/or voltage rail. The inject params let you supply a list of clock domains and/or voltage rails, and the voltage domain is one of `NV_GPU_VOLT_VOLT_DOMAIN_*`.

```c
--flags HEX // NVAPI VF inject flags (see nvapi.h)
--clk-domain ID --clk-khz N // set a clock domain frequency in kHz
--volt-domain logic|sram|msvdd|ID // voltage domain selector
--volt-rail N // voltage rail index (0-255)
--volt-uv N // target voltage in uV
--volt-min-uv N // minimum noise-unaware voltage in uV
// --clk-domain and --clk-khz must be provided together
// Voltage changes require --volt-domain, --volt-rail, and at least one of --volt-uv or
// --volt-min-uv
// voltage domain is NV_GPU_VOLT_VOLT_DOMAIN_*
```

## gpu vpstates info
Uses `NvAPI_GPU_PerfVpstatesGetInfo` (`NV_GPU_PERF_VPSTATES_INFO`) to dump VPSTATE definitions and associated clock groups. VPSTATE info is a static VBIOS table with a mask of valid indices and a mapping from logical names to indices.

```c
// static VBIOS table with a mask of valid indices
```

## gpu vpstates control
Uses `NvAPI_GPU_PerfVpstatesGetControl` (`NV_GPU_PERF_VPSTATES_CONTROL`) to show active VPSTATE control values. Control values are the VBIOS-defined settings that can be overridden via the control structure.

```c
--original // query original control values instead of current
```

## gpu vpstates set
Uses `NvAPI_GPU_PerfVpstatesGetControl`, `NvAPI_GPU_PerfVpstatesSetControl` (`NV_GPU_PERF_VPSTATES_CONTROL`) to set VPSTATE control values (2X group values or 3X clock targets). Set writes client-specified control values back into the VPSTATE table for the selected entries.

```c
--vpstate N // VPSTATE index to modify
--clock N --target-mhz N [--min-eff-mhz N] // set 3X clock target/min-eff MHz
--group N --value N // set 2X group value
// Choose either the clock path or the group path, not both
// --target-mhz and --min-eff-mhz must be <= 65535
// 2X uses group values, 3X uses clock targets
```

## gpu vfe-var info
Uses `NvAPI_GPU_PerfVfeVarGetInfo` (`NV_GPU_PERF_VFE_VARS_INFO`) to dump VFE variable metadata and ranges. VFE variable info describes variable types, ranges, and which entries are present.

## gpu vfe-var control
Uses `NvAPI_GPU_PerfVfeVarGetControl` (`NV_GPU_PERF_VFE_VARS_CONTROL`) to show current VFE variable overrides. Control exposes the current override state and any hysteresis parameters for supported variable types.

## gpu vfe-var set
Uses `NvAPI_GPU_PerfVfeVarGetControl`, `NvAPI_GPU_PerfVfeVarSetControl` (`NV_GPU_PERF_VFE_VARS_CONTROL`) to set VFE variable overrides and temperature hysteresis. Overrides are applied to a single variable, non-`none` override types require an explicit value.

```c
--var N // variable index
--override-type none|value|offset|scale // override type
--override-value F // required unless override-type is none
--temp-hyst-pos C, --temp-hyst-neg C // optional hysteresis for sensed-temp vars
// Hysteresis options are valid only for NV_GPU_PERF_VFE_VAR_TYPE_SINGLE_SENSED_TEMP
// override-type none does not require override-value
```

## gpu vfe-equ info
Uses `NvAPI_GPU_PerfVfeEquGetInfo` (`NV_GPU_PERF_VFE_EQUS_INFO`) to dump VFE equation metadata. VFE equation info describes equation types such as compare, min/max, or quadratic and their metadata.

## gpu vfe-equ control
Uses `NvAPI_GPU_PerfVfeEquGetControl` (`NV_GPU_PERF_VFE_EQUS_CONTROL`) to show current VFE equation control values. Control reports the active compare criteria, min/max selection, or coefficients depending on equation type.

## gpu vfe-equ set
Uses `NvAPI_GPU_PerfVfeEquGetControl`, `NvAPI_GPU_PerfVfeEquSetControl` (`NV_GPU_PERF_VFE_EQUS_CONTROL`) to update a VFE equation. Set updates only the data that matches the equation type and ignores incompatible modes.

```c
--equ N // equation index
--compare-func eq|gte|gt --compare-crit F // set compare equation
--minmax min|max // set min/max equation
--coeffs A,B,C // set quadratic coefficients
// Exactly one of compare/minmax/coeffs must be specified
// equation type must match compare/minmax/quadratic
```

## gpu perf-limits info
Uses `NvAPI_GPU_PerfLimitsGetInfo` (`NV_GPU_PERF_LIMITS_INFO`) to list perf limit IDs, names, and priorities. Perf limits are generic caps on clocks/performance and include named limit IDs and priority.

## gpu perf-limits status
Uses `NvAPI_GPU_PerfLimitsGetStatus` (`NV_GPU_PERF_LIMITS_STATUS`) to report current perf limit statuses and active inputs. Status reports whether each limit is enabled and which input type/value is driving it.

## gpu perf-limits set
Uses `NvAPI_GPU_PerfLimitsGetInfo`, `NvAPI_GPU_PerfLimitsSetStatus` (`NV_GPU_PERF_LIMITS_STATUS`) to set a perf limit input. A status struct with a single entry is passed back to set one limit with a chosen input type.

```c
--limit-id ID // limit ID (from perf-limits info)
--type disabled|pstate|freq|vpstate // input type
--pstate P0 // required for type pstate
--point nom|min|max|mid // optional for type pstate (defaults to nom)
--freq-khz N --domain ID // required for type freq
--vpstate N // required for type vpstate
// input type selects which fields are used
```

## gpu voltage
Uses `NvAPI_GPU_GetCoreVoltage`, `NvAPI_GPU_GetCoreVoltageControl`, `NvAPI_GPU_GetVoltageDomainsInfo`, `NvAPI_GPU_GetVoltageDomainsStatus`, `NvAPI_GPU_GetVoltages` to report current core voltage, voltage domains, and available levels. Core voltage is reported in mV, while domain info/status uses uV step sizes and current values, `NvAPI_GPU_GetVoltages` returns available voltage levels.

```c
// core voltage in mV, domain info uses uV
```

## gpu voltage control-set
Uses `NvAPI_GPU_SetCoreVoltageControl` to enable or disable core voltage control. The API toggles core voltage control and is marked for testing purposes in `nvapi.h`.

```c
--enable 0|1 // disable/enable control
// nvapi.h marks this as testing only
```

## gpu thermal
Uses `NvAPI_GPU_GetThermalSettings` (`NV_GPU_THERMAL_SETTINGS`) to report thermal sensor readings and limits. Thermal settings can target one sensor or all (`NVAPI_THERMAL_TARGET_ALL`), with sensors indexed 0..`NVAPI_MAX_THERMAL_SENSORS_PER_GPU-1`.

```c
// sensor index 0..NVAPI_MAX_THERMAL_SENSORS_PER_GPU-1 or NVAPI_THERMAL_TARGET_ALL
```

## gpu thermal slowdown
Uses `NvAPI_GPU_GetThermalSlowdownState` to report thermal slowdown state. Slowdown states include enabled and an all-disabled mode that shuts off HW/SW/Pstate slowdown.

## gpu thermal slowdown-set
Uses `NvAPI_GPU_SetThermalSlowdownState` to enable or disable thermal slowdown. The setter uses `NV_GPU_THERMAL_SLOWDOWN` and can disable all slowdown mechanisms at once.

```c
--state enabled|disabled // requested state
```

## gpu thermal sim
Uses `NvAPI_GPU_GetThermalSimulationMode` to report thermal simulation mode for a sensor. Simulation mode is reported per sensor and may be unsupported on some GPUs.

```c
--sensor N // sensor index (defaults to 0 when omitted)
```

## gpu thermal sim-set
Uses `NvAPI_GPU_SetThermalSimulationMode` to enable or disable thermal simulation for a sensor. When enabled, the selected sensor reports a constant simulated temperature in the 0-255 C range.

```c
--sensor N // sensor index
--mode enabled|disabled // requested mode
--temp C // required when enabling simulation
// Some drivers return NVAPI_PRIV_SEC_VIOLATION for this call
// temp range is 0-255 C
// simulation may be unsupported
```

## gpu fan set
Uses `NvAPI_GPU_SetCoolerLevels` (`NV_GPU_SETCOOLER_LEVEL`) to set a cooler level and policy. Cooler indices come from the cooler settings table and the level is a percent value applied to that cooler.

```c
--cooler N // cooler index
--level PCT // fan level 0-100
--policy manual|perf|temp-discrete|temp-cont|temp-cont-sw|default // cooler policy
// level is percent (0-100)
```

## gpu fan restore
Uses `NvAPI_GPU_RestoreCoolerSettings` to restore cooler settings to driver defaults. Passing a cooler index restores that cooler, while omitting it restores all coolers.

```c
--cooler N // optional cooler index (omit to restore all)
// omit --cooler to restore all
```

## gpu client-fan coolers info
Uses `NvAPI_GPU_ClientFanCoolersGetInfo` (`NV_GPU_CLIENT_FAN_COOLERS_INFO`) to report client fan cooler capabilities and RPM ranges. Client fan cooler info includes cooler IDs and supported RPM ranges.

## gpu client-fan coolers status
Uses `NvAPI_GPU_ClientFanCoolersGetStatus` (`NV_GPU_CLIENT_FAN_COOLERS_STATUS`) to report current client fan cooler status. Status reports current RPM plus configured target levels for each cooler.

## gpu client-fan coolers control
Uses `NvAPI_GPU_ClientFanCoolersGetControl` (`NV_GPU_CLIENT_FAN_COOLERS_CONTROL`) to report current client fan cooler control overrides. Control shows whether level simulation is active and what simulated level is set.

## gpu client-fan coolers set
Uses `NvAPI_GPU_ClientFanCoolersGetControl`, `NvAPI_GPU_ClientFanCoolersSetControl` (`NV_GPU_CLIENT_FAN_COOLERS_CONTROL`) to set or clear client fan cooler overrides. Set writes to the control structure and can reset defaults via `bDefault`.

```c
--cooler N // cooler index
--level PCT // target level 0-100
--enable 0|1 // enable/disable level simulation
--default // restore defaults
// If --level is provided without --enable, the CLI enables the override automatically
// Enabling requires --level
// --default resets to driver defaults
```

## gpu client-fan policies info
Uses `NvAPI_GPU_ClientFanPoliciesGetInfo` (`NV_GPU_CLIENT_FAN_POLICIES_INFO`) to report client fan policies. Policy info describes available policies and their capabilities.

## gpu client-fan policies status
Uses `NvAPI_GPU_ClientFanPoliciesGetStatus` (`NV_GPU_CLIENT_FAN_POLICIES_STATUS`) to report client fan policy status. Status returns the active policy state.

## gpu client-fan policies control
Uses `NvAPI_GPU_ClientFanPoliciesGetControl` (`NV_GPU_CLIENT_FAN_POLICIES_CONTROL`) to report client fan policy control values. Control exposes per-policy fan-stop enable flags.

## gpu client-fan policies set
Uses `NvAPI_GPU_ClientFanPoliciesGetControl`, `NvAPI_GPU_ClientFanPoliciesSetControl` (`NV_GPU_CLIENT_FAN_POLICIES_CONTROL`) to set fan-stop policy control. Set toggles the fan-stop feature on a policy or resets defaults.

```c
--policy N // policy index
--fan-stop 0|1 // enable or disable fan-stop
--default // restore defaults
// --default resets to driver defaults
```

## gpu client-fan arbiters info
Uses `NvAPI_GPU_ClientFanArbitersGetInfo` (`NV_GPU_CLIENT_FAN_ARBITERS_INFO`) to report client fan arbiters. Arbiter info lists available arbiters and indices.

## gpu client-fan arbiters status
Uses `NvAPI_GPU_ClientFanArbitersGetStatus` (`NV_GPU_CLIENT_FAN_ARBITERS_STATUS`) to report client fan arbiter status. Status shows whether fan-stop is active per arbiter.

## gpu client-fan arbiters control
Uses `NvAPI_GPU_ClientFanArbitersGetControl` (`NV_GPU_CLIENT_FAN_ARBITERS_CONTROL`) to report client fan arbiter control values. Control exposes per-arbiter fan-stop enable flags.

## gpu client-fan arbiters set
Uses `NvAPI_GPU_ClientFanArbitersGetControl`, `NvAPI_GPU_ClientFanArbitersSetControl` (`NV_GPU_CLIENT_FAN_ARBITERS_CONTROL`) to enable or disable fan-stop per arbiter. Set updates the arbiter's fan-stop enable setting.

```c
--arbiter N // arbiter index
--fan-stop 0|1 // enable or disable fan-stop
```

## gpu client-illum devices info
Uses `NvAPI_GPU_ClientIllumDevicesGetInfo` (`NV_GPU_CLIENT_ILLUM_DEVICE_INFO_PARAMS`) to report illumination devices and types. Illum device info lists device types and sync capability.

## gpu client-illum devices control
Uses `NvAPI_GPU_ClientIllumDevicesGetControl` (`NV_GPU_CLIENT_ILLUM_DEVICE_CONTROL_PARAMS`) to report illumination device control and sync settings. Control reports per-device sync state and timestamp.

## gpu client-illum devices set
Uses `NvAPI_GPU_ClientIllumDevicesGetControl`, `NvAPI_GPU_ClientIllumDevicesSetControl` (`NV_GPU_CLIENT_ILLUM_DEVICE_CONTROL_PARAMS`) to set device sync and timestamp. Set updates sync enable and optional timestamp for a device.

```c
--device N // device index
--sync 0|1 // enable or disable sync
--timestamp-ms N // optional timestamp override
```

## gpu client-illum zones info
Uses `NvAPI_GPU_ClientIllumZonesGetInfo` (`NV_GPU_CLIENT_ILLUM_ZONE_INFO_PARAMS`) to report illumination zones and locations. Zone info includes zone type and physical location.

## gpu client-illum zones control
Uses `NvAPI_GPU_ClientIllumZonesGetControl` (`NV_GPU_CLIENT_ILLUM_ZONE_CONTROL_PARAMS`) to report current zone control values. Control reports the current mode and zone parameters.

## gpu client-illum zones set
Uses `NvAPI_GPU_ClientIllumZonesGetControl`, `NvAPI_GPU_ClientIllumZonesSetControl` (`NV_GPU_CLIENT_ILLUM_ZONE_CONTROL_PARAMS`) to set manual illumination control for a zone or restore defaults. Set enforces manual control for RGB/RGBW/single/fixed zones when the mode matches the zone type.

```c
--zone N // zone index
--mode manual-rgb|manual-rgbw|manual-single|manual-color-fixed // manual mode
--brightness N // 0-100
--r N --g N --b N // required for manual-rgb
--r N --g N --b N --w N // required for manual-rgbw
--default // restore defaults
// mode must match the zone type
```
