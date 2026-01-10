/*
 * Copyright (c) 2026 Noverse (Nohuto). All rights reserved.
 * Proprietary and confidential. Unauthorized copying or redistribution is strictly prohibited.
 */

#include "cli/commands.h"

namespace nvcli
{
    bool ParseFloat(const char *text, float *out)
    {
        if (!text || !out)
        {
            return false;
        }
        char *end = NULL;
        float value = std::strtof(text, &end);
        if (end == text || *end != '\0')
        {
            return false;
        }
        *out = value;
        return true;
    }

    bool ParseInt(const char *text, NvS32 *out)
    {
        if (!text || !out)
        {
            return false;
        }
        char *end = NULL;
        long value = std::strtol(text, &end, 0);
        if (end == text || *end != '\0')
        {
            return false;
        }
        *out = static_cast<NvS32>(value);
        return true;
    }

    bool ParseVfeVarOverrideType(const char *value, NvU8 *out)
    {
        if (!value || !out)
        {
            return false;
        }
        if (std::strcmp(value, "none") == 0)
        {
            *out = NV_GPU_PERF_VFE_VAR_SINGLE_OVERRIDE_TYPE_NONE;
            return true;
        }
        if (std::strcmp(value, "value") == 0)
        {
            *out = NV_GPU_PERF_VFE_VAR_SINGLE_OVERRIDE_TYPE_VALUE;
            return true;
        }
        if (std::strcmp(value, "offset") == 0)
        {
            *out = NV_GPU_PERF_VFE_VAR_SINGLE_OVERRIDE_TYPE_OFFSET;
            return true;
        }
        if (std::strcmp(value, "scale") == 0)
        {
            *out = NV_GPU_PERF_VFE_VAR_SINGLE_OVERRIDE_TYPE_SCALE;
            return true;
        }
        NvU32 numeric = 0;
        if (!ParseUint(value, &numeric))
        {
            return false;
        }
        *out = static_cast<NvU8>(numeric);
        return true;
    }

    bool ParseVfeEquCompareFunc(const char *value, NvU8 *out)
    {
        if (!value || !out)
        {
            return false;
        }
        if (std::strcmp(value, "eq") == 0)
        {
            *out = NV_GPU_PERF_VFE_EQU_COMPARE_FUNCTION_EQUAL;
            return true;
        }
        if (std::strcmp(value, "gte") == 0)
        {
            *out = NV_GPU_PERF_VFE_EQU_COMPARE_FUNCTION_GREATER_EQ;
            return true;
        }
        if (std::strcmp(value, "gt") == 0)
        {
            *out = NV_GPU_PERF_VFE_EQU_COMPARE_FUNCTION_GREATER;
            return true;
        }
        NvU32 numeric = 0;
        if (!ParseUint(value, &numeric))
        {
            return false;
        }
        *out = static_cast<NvU8>(numeric);
        return true;
    }

    bool ParseVoltDomainType(const char *value, NvU8 *out)
    {
        if (!value || !out)
        {
            return false;
        }
        if (std::strcmp(value, "logic") == 0)
        {
            *out = NV_GPU_VOLT_VOLT_DOMAIN_LOGIC;
            return true;
        }
        if (std::strcmp(value, "sram") == 0)
        {
            *out = NV_GPU_VOLT_VOLT_DOMAIN_SRAM;
            return true;
        }
        if (std::strcmp(value, "msvdd") == 0)
        {
            *out = NV_GPU_VOLT_VOLT_DOMAIN_MSVDD;
            return true;
        }
        NvU32 numeric = 0;
        if (!ParseUint(value, &numeric))
        {
            return false;
        }
        *out = static_cast<NvU8>(numeric);
        return true;
    }

    bool ParseCsvFloat3(const char *text, NvF32 coeffs[3])
    {
        if (!text || !coeffs)
        {
            return false;
        }

        const char *cursor = text;
        for (int i = 0; i < 3; ++i)
        {
            char *end = NULL;
            coeffs[i] = std::strtof(cursor, &end);
            if (end == cursor)
            {
                return false;
            }
            if (i < 2)
            {
                if (*end != ',')
                {
                    return false;
                }
                cursor = end + 1;
            }
            else if (*end != '\0')
            {
                return false;
            }
        }

        return true;
    }

    NV_GPU_PERF_VFE_VAR_CONTROL_SINGLE *GetVfeVarSingleControl(NV_GPU_PERF_VFE_VAR_CONTROL_V1 &var)
    {
        switch (var.type)
        {
        case NV_GPU_PERF_VFE_VAR_TYPE_SINGLE:
            return &var.data.single;
        case NV_GPU_PERF_VFE_VAR_TYPE_SINGLE_FREQUENCY:
            return &var.data.singleFreq.super;
        case NV_GPU_PERF_VFE_VAR_TYPE_SINGLE_SENSED:
            return &var.data.sensed.super;
        case NV_GPU_PERF_VFE_VAR_TYPE_SINGLE_SENSED_FUSE:
        case NV_GPU_PERF_VFE_VAR_TYPE_SINGLE_SENSED_FUSE_20:
            return &var.data.sensedFuse.super.super;
        case NV_GPU_PERF_VFE_VAR_TYPE_SINGLE_SENSED_TEMP:
            return &var.data.sensedTemp.super.super;
        case NV_GPU_PERF_VFE_VAR_TYPE_SINGLE_VOLTAGE:
            return &var.data.singleVolt.super.super;
        default:
            return NULL;
        }
    }

    const NV_GPU_PERF_VFE_VAR_CONTROL_SINGLE *GetVfeVarSingleControl(const NV_GPU_PERF_VFE_VAR_CONTROL_V1 &var)
    {
        switch (var.type)
        {
        case NV_GPU_PERF_VFE_VAR_TYPE_SINGLE:
            return &var.data.single;
        case NV_GPU_PERF_VFE_VAR_TYPE_SINGLE_FREQUENCY:
            return &var.data.singleFreq.super;
        case NV_GPU_PERF_VFE_VAR_TYPE_SINGLE_SENSED:
            return &var.data.sensed.super;
        case NV_GPU_PERF_VFE_VAR_TYPE_SINGLE_SENSED_FUSE:
        case NV_GPU_PERF_VFE_VAR_TYPE_SINGLE_SENSED_FUSE_20:
            return &var.data.sensedFuse.super.super;
        case NV_GPU_PERF_VFE_VAR_TYPE_SINGLE_SENSED_TEMP:
            return &var.data.sensedTemp.super.super;
        case NV_GPU_PERF_VFE_VAR_TYPE_SINGLE_VOLTAGE:
            return &var.data.singleVolt.super.super;
        default:
            return NULL;
        }
    }

    int CmdGpuPstates20(int argc, char **argv)
    {
        NvU32 index = 0;
        bool hasIndex = false;
        if (!ParseGpuIndexArgs(argc, argv, &index, &hasIndex))
        {
            return 1;
        }

        std::vector<NvPhysicalGpuHandle> handles;
        std::vector<NvU32> indices;
        if (!CollectGpus(hasIndex, index, handles, indices))
        {
            return 1;
        }

        for (size_t i = 0; i < handles.size(); ++i)
        {
            PrintGpuHeader(indices[i], handles[i]);

            NV_GPU_PERF_PSTATES20_INFO info = {};
            info.version = NV_GPU_PERF_PSTATES20_INFO_VER;
            NvAPI_Status status = NvAPI_GPU_GetPstates20(handles[i], &info);
            if (status != NVAPI_OK)
            {
                PrintNvapiError("  NvAPI_GPU_GetPstates20 failed", status);
                continue;
            }

            std::printf("  Pstates20 editable=%u pstates=%u clocks=%u voltages=%u\n",
                info.bIsEditable ? 1 : 0,
                info.numPstates,
                info.numClocks,
                info.numBaseVoltages);

            for (NvU32 p = 0; p < info.numPstates; ++p)
            {
                const auto &pstate = info.pstates[p];
                std::printf("  %s editable=%u\n",
                    PstateName(pstate.pstateId),
                    pstate.bIsEditable ? 1 : 0);

                for (NvU32 c = 0; c < info.numClocks; ++c)
                {
                    const auto &clock = pstate.clocks[c];
                    if (clock.domainId == NVAPI_GPU_PUBLIC_CLOCK_UNDEFINED)
                    {
                        continue;
                    }

                    std::printf("    clock[%u] %s type=%s editable=%u\n",
                        c,
                        ClockDomainName(clock.domainId),
                        PstateClockTypeName(clock.typeId),
                        clock.bIsEditable ? 1 : 0);
                    if (clock.typeId == NVAPI_GPU_PERF_PSTATE20_CLOCK_TYPE_SINGLE)
                    {
                        std::printf("      freq=%u kHz\n", clock.data.single.freq_kHz);
                    }
                    else
                    {
                        std::printf("      freqRange=%u-%u kHz voltageDomain=%u minV=%u maxV=%u\n",
                            clock.data.range.minFreq_kHz,
                            clock.data.range.maxFreq_kHz,
                            clock.data.range.domainId,
                            clock.data.range.minVoltage_uV,
                            clock.data.range.maxVoltage_uV);
                    }
                    std::printf("      delta=%d kHz (min=%d max=%d)\n",
                        clock.freqDelta_kHz.value,
                        clock.freqDelta_kHz.valueRange.min,
                        clock.freqDelta_kHz.valueRange.max);
                }

                for (NvU32 v = 0; v < info.numBaseVoltages; ++v)
                {
                    const auto &volt = pstate.baseVoltages[v];
                    if (volt.domainId == NVAPI_GPU_PERF_VOLTAGE_INFO_DOMAIN_UNDEFINED)
                    {
                        continue;
                    }
                    std::printf("    voltage[%u] %s editable=%u base=%u uV delta=%d uV (min=%d max=%d)\n",
                        v,
                        VoltageDomainName(volt.domainId),
                        volt.bIsEditable ? 1 : 0,
                        volt.volt_uV,
                        volt.voltDelta_uV.value,
                        volt.voltDelta_uV.valueRange.min,
                        volt.voltDelta_uV.valueRange.max);
                }

                if (info.ov.numVoltages > 0)
                {
                    std::printf("    ov voltages: %u\n", info.ov.numVoltages);
                    for (NvU32 v = 0; v < info.ov.numVoltages; ++v)
                    {
                        const auto &ov = info.ov.voltages[v];
                        std::printf("      ov[%u] %s editable=%u base=%u uV delta=%d uV (min=%d max=%d)\n",
                            v,
                            VoltageDomainName(ov.domainId),
                            ov.bIsEditable ? 1 : 0,
                            ov.volt_uV,
                            ov.voltDelta_uV.value,
                            ov.voltDelta_uV.valueRange.min,
                            ov.voltDelta_uV.valueRange.max);
                    }
                }
            }
        }
        return 0;
    }

    int CmdGpuPstates20Private(int argc, char **argv)
    {
        NvU32 index = 0;
        bool hasIndex = false;
        if (!ParseGpuIndexArgs(argc, argv, &index, &hasIndex))
        {
            return 1;
        }

        std::vector<NvPhysicalGpuHandle> handles;
        std::vector<NvU32> indices;
        if (!CollectGpus(hasIndex, index, handles, indices))
        {
            return 1;
        }

        for (size_t i = 0; i < handles.size(); ++i)
        {
            PrintGpuHeader(indices[i], handles[i]);

            NV_GPU_PERF_PSTATES20_PRIVATE_INFO info = {};
            info.version = NV_GPU_PERF_PSTATES20_PRIVATE_INFO_VER;
            NvAPI_Status status = NvAPI_GPU_GetPstates20Private(handles[i], &info);
            if (status != NVAPI_OK)
            {
                PrintNvapiError("  NvAPI_GPU_GetPstates20Private failed", status);
                continue;
            }

            std::printf("  Pstates20 private editable=%u pstates=%u clocks=%u voltages=%u ov=%u\n",
                info.bIsEditable ? 1 : 0,
                info.numPstates,
                info.numClocks,
                info.numVoltages,
                info.ov.numVoltages);

            for (NvU32 p = 0; p < info.numPstates; ++p)
            {
                const auto &pstate = info.pstates[p];
                std::printf("  %s editable=%u\n",
                    PstateName(pstate.pstateId),
                    pstate.bIsEditable ? 1 : 0);

                for (NvU32 c = 0; c < info.numClocks; ++c)
                {
                    const auto &clock = pstate.clocks[c];
                    if (clock.domainId == NVAPI_GPU_CLOCK_DOMAIN_UNDEFINED)
                    {
                        continue;
                    }

                    std::printf("    clock[%u] id=%u (%s) usage=%s editable=%u\n",
                        c,
                        clock.domainId,
                        ClockDomainIdName(clock.domainId),
                        ClockPstateUsageName(clock.usage),
                        clock.bIsEditable ? 1 : 0);

                    switch (clock.usage)
                    {
                    case NV_GPU_CLOCK_PSTATE_USAGE_FIXED:
                        std::printf("      freq=%u kHz\n", clock.data.fixed.freq_kHz);
                        break;
                    case NV_GPU_CLOCK_PSTATE_USAGE_PSTATE:
                        std::printf("      freq=%u kHz\n", clock.data.pstate.freq_kHz);
                        break;
                    case NV_GPU_CLOCK_PSTATE_USAGE_DECOUPLED:
                        std::printf("      freqRange=%u-%u kHz voltageDomain=%s(%u) minV=%u maxV=%u\n",
                            clock.data.decoupled.minFreq_kHz,
                            clock.data.decoupled.maxFreq_kHz,
                            PerfVoltageDomainName(clock.data.decoupled.domainId),
                            clock.data.decoupled.domainId,
                            clock.data.decoupled.minVoltage_uV,
                            clock.data.decoupled.maxVoltage_uV);
                        break;
                    case NV_GPU_CLOCK_PSTATE_USAGE_RATIO:
                        std::printf("      freqRange=%u-%u kHz\n",
                            clock.data.ratio.minFreq_kHz,
                            clock.data.ratio.maxFreq_kHz);
                        break;
                    default:
                        break;
                    }

                    std::printf("      delta=%d kHz (min=%d max=%d)\n",
                        clock.freqDelta_kHz.value,
                        clock.freqDelta_kHz.valueRange.min,
                        clock.freqDelta_kHz.valueRange.max);
                }

                for (NvU32 v = 0; v < info.numVoltages; ++v)
                {
                    const auto &volt = pstate.voltages[v];
                    if (volt.domainId == NVAPI_GPU_PERF_VOLTAGE_DOMAIN_UNDEFINED)
                    {
                        continue;
                    }

                    std::printf("    voltage[%u] %s(%u) type=%s editable=%u volt=%u uV delta=%d uV (min=%d max=%d)\n",
                        v,
                        PerfVoltageDomainName(volt.domainId),
                        volt.domainId,
                        VoltageEntryTypeName(volt.type),
                        volt.bIsEditable ? 1 : 0,
                        volt.volt_uV,
                        volt.voltDelta_uV.value,
                        volt.voltDelta_uV.valueRange.min,
                        volt.voltDelta_uV.valueRange.max);

                    if (volt.type == NV_GPU_PSTATE20_VOLTAGE_ENTRY_TYPE_LOGICAL)
                    {
                        std::printf("      nominal=%u uV\n", volt.data.logical.nominal_uV);
                    }
                    else if (volt.type == NV_GPU_PSTATE20_VOLTAGE_ENTRY_TYPE_VDT)
                    {
                        std::printf("      vdtIndex=%u\n", volt.data.vdt.vdtIndex);
                    }
                }

                if (info.ov.numVoltages > 0)
                {
                    std::printf("    ov voltages: %u\n", info.ov.numVoltages);
                    for (NvU32 v = 0; v < info.ov.numVoltages; ++v)
                    {
                        const auto &ov = info.ov.voltages[v];
                        if (ov.domainId == NVAPI_GPU_PERF_VOLTAGE_DOMAIN_UNDEFINED)
                        {
                            continue;
                        }
                        std::printf("      ov[%u] %s(%u) type=%s editable=%u volt=%u uV delta=%d uV (min=%d max=%d)\n",
                            v,
                            PerfVoltageDomainName(ov.domainId),
                            ov.domainId,
                            VoltageEntryTypeName(ov.type),
                            ov.bIsEditable ? 1 : 0,
                            ov.volt_uV,
                            ov.voltDelta_uV.value,
                            ov.voltDelta_uV.valueRange.min,
                            ov.voltDelta_uV.valueRange.max);
                    }
                }
            }
        }

        return 0;
    }

    int CmdGpuPstates20PrivateSet(int argc, char **argv)
    {
        NvU32 index = 0;
        bool hasIndex = false;
        bool hasPstate = false;
        bool hasClock = false;
        bool hasClockDelta = false;
        bool hasVoltage = false;
        bool hasVoltageDelta = false;
        bool hasVoltageTarget = false;
        NV_GPU_PERF_PSTATE_ID pstateId = NVAPI_GPU_PERF_PSTATE_UNDEFINED;
        NV_GPU_CLOCK_DOMAIN_ID clockId = NVAPI_GPU_CLOCK_DOMAIN_UNDEFINED;
        NV_GPU_PERF_VOLTAGE_DOMAIN_ID voltageId = NVAPI_GPU_PERF_VOLTAGE_DOMAIN_UNDEFINED;
        NvS32 clockDelta = 0;
        NvS32 voltageDelta = 0;
        NvU32 voltageTarget = 0;

        for (int i = 0; i < argc; ++i)
        {
            if (std::strcmp(argv[i], "--index") == 0)
            {
                if (i + 1 >= argc || !ParseUint(argv[i + 1], &index))
                {
                    std::printf("Invalid GPU index.\n");
                    return 1;
                }
                hasIndex = true;
                ++i;
                continue;
            }
            if (std::strcmp(argv[i], "--pstate") == 0)
            {
                if (i + 1 >= argc || !ParsePstateId(argv[i + 1], &pstateId))
                {
                    std::printf("Invalid pstate.\n");
                    return 1;
                }
                hasPstate = true;
                ++i;
                continue;
            }
            if (std::strcmp(argv[i], "--clock-id") == 0)
            {
                NvU32 value = 0;
                if (i + 1 >= argc || !ParseUint(argv[i + 1], &value))
                {
                    std::printf("Invalid clock id.\n");
                    return 1;
                }
                clockId = static_cast<NV_GPU_CLOCK_DOMAIN_ID>(value);
                hasClock = true;
                ++i;
                continue;
            }
            if (std::strcmp(argv[i], "--delta-khz") == 0)
            {
                if (i + 1 >= argc)
                {
                    std::printf("Missing value for --delta-khz\n");
                    return 1;
                }
                clockDelta = static_cast<NvS32>(std::strtol(argv[i + 1], NULL, 0));
                hasClockDelta = true;
                ++i;
                continue;
            }
            if (std::strcmp(argv[i], "--voltage-domain") == 0)
            {
                if (i + 1 >= argc || !ParsePerfVoltageDomainId(argv[i + 1], &voltageId))
                {
                    std::printf("Invalid voltage domain.\n");
                    return 1;
                }
                hasVoltage = true;
                ++i;
                continue;
            }
            if (std::strcmp(argv[i], "--delta-uv") == 0)
            {
                if (i + 1 >= argc)
                {
                    std::printf("Missing value for --delta-uv\n");
                    return 1;
                }
                voltageDelta = static_cast<NvS32>(std::strtol(argv[i + 1], NULL, 0));
                hasVoltageDelta = true;
                ++i;
                continue;
            }
            if (std::strcmp(argv[i], "--target-uv") == 0)
            {
                if (i + 1 >= argc || !ParseUint(argv[i + 1], &voltageTarget))
                {
                    std::printf("Invalid target voltage.\n");
                    return 1;
                }
                hasVoltageTarget = true;
                ++i;
                continue;
            }
            std::printf("Unknown option: %s\n", argv[i]);
            return 1;
        }

        if (!hasPstate)
        {
            std::printf("Missing required --pstate\n");
            return 1;
        }
        if (hasClock != hasClockDelta)
        {
            std::printf("Both --clock-id and --delta-khz are required together.\n");
            return 1;
        }
        if (hasVoltage && hasVoltageDelta && hasVoltageTarget)
        {
            std::printf("Specify only one of --delta-uv or --target-uv.\n");
            return 1;
        }
        if (hasVoltage && !hasVoltageDelta && !hasVoltageTarget)
        {
            std::printf("Either --delta-uv or --target-uv is required for voltage changes.\n");
            return 1;
        }
        if (!hasClock && !hasVoltage)
        {
            std::printf("Specify at least --clock-id/--delta-khz or --voltage-domain with delta/target.\n");
            return 1;
        }

        std::vector<NvPhysicalGpuHandle> handles;
        std::vector<NvU32> indices;
        if (!CollectGpus(hasIndex, index, handles, indices))
        {
            return 1;
        }

        for (size_t i = 0; i < handles.size(); ++i)
        {
            PrintGpuHeader(indices[i], handles[i]);

            NV_GPU_PERF_PSTATES20_PRIVATE_INFO info = {};
            info.version = NV_GPU_PERF_PSTATES20_PRIVATE_INFO_VER;
            NvAPI_Status status = NvAPI_GPU_GetPstates20Private(handles[i], &info);
            if (status != NVAPI_OK)
            {
                PrintNvapiError("  NvAPI_GPU_GetPstates20Private failed", status);
                continue;
            }

            NV_GPU_PSTATE20_PRIVATE_CLOCK_ENTRY_V1 *clockEntry = NULL;
            NV_GPU_PSTATE20_VOLTAGE_ENTRY_V1 *voltEntry = NULL;
            bool foundPstate = false;
            NvU32 pstateIndex = 0;

            for (NvU32 p = 0; p < info.numPstates; ++p)
            {
                auto &pstate = info.pstates[p];
                if (pstate.pstateId != pstateId)
                {
                    continue;
                }
                foundPstate = true;
                pstateIndex = p;

                if (hasClock)
                {
                    for (NvU32 c = 0; c < info.numClocks; ++c)
                    {
                        if (pstate.clocks[c].domainId == clockId)
                        {
                            clockEntry = &pstate.clocks[c];
                            break;
                        }
                    }
                }
                if (hasVoltage)
                {
                    for (NvU32 v = 0; v < info.numVoltages; ++v)
                    {
                        if (pstate.voltages[v].domainId == voltageId)
                        {
                            voltEntry = &pstate.voltages[v];
                            break;
                        }
                    }
                }
                break;
            }

            if (!foundPstate)
            {
                std::printf("  Pstate not found: %s\n", PstateName(pstateId));
                continue;
            }
            if (hasClock && !clockEntry)
            {
                std::printf("  Clock domain not found in pstate.\n");
                continue;
            }
            if (hasVoltage && !voltEntry)
            {
                std::printf("  Voltage domain not found in pstate.\n");
                continue;
            }
            if (!info.bIsEditable)
            {
                std::printf("  Pstates20 private not editable on this GPU.\n");
                continue;
            }

            if (clockEntry)
            {
                if (!clockEntry->bIsEditable)
                {
                    std::printf("  Clock domain not editable.\n");
                    continue;
                }
                if (clockDelta < clockEntry->freqDelta_kHz.valueRange.min ||
                    clockDelta > clockEntry->freqDelta_kHz.valueRange.max)
                {
                    std::printf("  Clock delta out of range (%d to %d kHz).\n",
                        clockEntry->freqDelta_kHz.valueRange.min,
                        clockEntry->freqDelta_kHz.valueRange.max);
                    continue;
                }
                clockEntry->freqDelta_kHz.value = clockDelta;
            }

            if (voltEntry)
            {
                if (!voltEntry->bIsEditable)
                {
                    std::printf("  Voltage domain not editable.\n");
                    continue;
                }
                if (hasVoltageTarget)
                {
                    long long base = static_cast<long long>(voltEntry->volt_uV) -
                        static_cast<long long>(voltEntry->voltDelta_uV.value);
                    long long targetDelta = static_cast<long long>(voltageTarget) - base;
                    voltageDelta = static_cast<NvS32>(targetDelta);
                }

                if (voltageDelta < voltEntry->voltDelta_uV.valueRange.min ||
                    voltageDelta > voltEntry->voltDelta_uV.valueRange.max)
                {
                    std::printf("  Voltage delta out of range (%d to %d uV).\n",
                        voltEntry->voltDelta_uV.valueRange.min,
                        voltEntry->voltDelta_uV.valueRange.max);
                    continue;
                }
                voltEntry->voltDelta_uV.value = voltageDelta;
            }

            std::printf("  Pstates20 private set request: %s\n", PstateName(pstateId));
            if (clockEntry)
            {
                std::printf("    clock id=%u (%s) delta=%d kHz\n",
                    clockId,
                    ClockDomainIdName(clockId),
                    clockDelta);
            }
            if (voltEntry)
            {
                if (hasVoltageTarget)
                {
                    std::printf("    voltage %s target=%u uV delta=%d uV\n",
                        PerfVoltageDomainName(voltageId),
                        voltageTarget,
                        voltageDelta);
                }
                else
                {
                    std::printf("    voltage %s delta=%d uV\n",
                        PerfVoltageDomainName(voltageId),
                        voltageDelta);
                }
            }

            NV_GPU_PERF_PSTATES20_PRIVATE_INFO setInfo = {};
            setInfo.version = info.version;
            setInfo.bIsEditable = info.bIsEditable;
            setInfo.numPstates = 1;
            setInfo.numClocks = info.numClocks;
            setInfo.numVoltages = info.numVoltages;
            setInfo.pstates[0] = info.pstates[pstateIndex];

            for (NvU32 c = 0; c < setInfo.numClocks; ++c)
            {
                auto &entry = setInfo.pstates[0].clocks[c];
                if (!hasClock || entry.domainId != clockId)
                {
                    entry.domainId = NVAPI_GPU_CLOCK_DOMAIN_UNDEFINED;
                }
            }
            for (NvU32 v = 0; v < setInfo.numVoltages; ++v)
            {
                auto &entry = setInfo.pstates[0].voltages[v];
                if (!hasVoltage || entry.domainId != voltageId)
                {
                    entry.domainId = NVAPI_GPU_PERF_VOLTAGE_DOMAIN_UNDEFINED;
                }
            }

            status = NvAPI_GPU_SetPstates20Private(handles[i], &setInfo);
            if (status != NVAPI_OK)
            {
                PrintNvapiError("  NvAPI_GPU_SetPstates20Private failed", status);
                continue;
            }

            std::printf("  Pstates20 private updated.\n");
        }
        return 0;
    }

    int CmdGpuPstates20Set(int argc, char **argv)
    {
        NvU32 index = 0;
        bool hasIndex = false;
        bool hasPstate = false;
        bool hasClock = false;
        bool hasClockDelta = false;
        bool hasVoltage = false;
        bool hasVoltageDelta = false;
        NV_GPU_PERF_PSTATE_ID pstateId = NVAPI_GPU_PERF_PSTATE_UNDEFINED;
        NV_GPU_PUBLIC_CLOCK_ID clockId = NVAPI_GPU_PUBLIC_CLOCK_UNDEFINED;
        NV_GPU_PERF_VOLTAGE_INFO_DOMAIN_ID voltageId = NVAPI_GPU_PERF_VOLTAGE_INFO_DOMAIN_UNDEFINED;
        NvS32 clockDelta = 0;
        NvS32 voltageDelta = 0;

        for (int i = 0; i < argc; ++i)
        {
            if (std::strcmp(argv[i], "--index") == 0)
            {
                if (i + 1 >= argc || !ParseUint(argv[i + 1], &index))
                {
                    std::printf("Invalid GPU index.\n");
                    return 1;
                }
                hasIndex = true;
                ++i;
                continue;
            }
            if (std::strcmp(argv[i], "--pstate") == 0)
            {
                if (i + 1 >= argc || !ParsePstateId(argv[i + 1], &pstateId))
                {
                    std::printf("Invalid pstate.\n");
                    return 1;
                }
                hasPstate = true;
                ++i;
                continue;
            }
            if (std::strcmp(argv[i], "--clock") == 0)
            {
                if (i + 1 >= argc || !ParsePublicClockId(argv[i + 1], &clockId))
                {
                    std::printf("Invalid clock domain.\n");
                    return 1;
                }
                hasClock = true;
                ++i;
                continue;
            }
            if (std::strcmp(argv[i], "--delta-khz") == 0)
            {
                if (i + 1 >= argc)
                {
                    std::printf("Missing value for --delta-khz\n");
                    return 1;
                }
                clockDelta = static_cast<NvS32>(std::strtol(argv[i + 1], NULL, 0));
                hasClockDelta = true;
                ++i;
                continue;
            }
            if (std::strcmp(argv[i], "--voltage") == 0)
            {
                if (i + 1 >= argc || !ParseVoltageDomainId(argv[i + 1], &voltageId))
                {
                    std::printf("Invalid voltage domain.\n");
                    return 1;
                }
                hasVoltage = true;
                ++i;
                continue;
            }
            if (std::strcmp(argv[i], "--delta-uv") == 0)
            {
                if (i + 1 >= argc)
                {
                    std::printf("Missing value for --delta-uv\n");
                    return 1;
                }
                voltageDelta = static_cast<NvS32>(std::strtol(argv[i + 1], NULL, 0));
                hasVoltageDelta = true;
                ++i;
                continue;
            }
            std::printf("Unknown option: %s\n", argv[i]);
            return 1;
        }

        if (!hasPstate)
        {
            std::printf("Missing required --pstate\n");
            return 1;
        }
        if (hasClock != hasClockDelta)
        {
            std::printf("Both --clock and --delta-khz are required together.\n");
            return 1;
        }
        if (hasVoltage != hasVoltageDelta)
        {
            std::printf("Both --voltage and --delta-uv are required together.\n");
            return 1;
        }
        if (!hasClock && !hasVoltage)
        {
            std::printf("Specify at least --clock/--delta-khz or --voltage/--delta-uv.\n");
            return 1;
        }

        std::vector<NvPhysicalGpuHandle> handles;
        std::vector<NvU32> indices;
        if (!CollectGpus(hasIndex, index, handles, indices))
        {
            return 1;
        }

        for (size_t i = 0; i < handles.size(); ++i)
        {
            PrintGpuHeader(indices[i], handles[i]);

            NV_GPU_PERF_PSTATES20_INFO info = {};
            info.version = NV_GPU_PERF_PSTATES20_INFO_VER;
            NvAPI_Status status = NvAPI_GPU_GetPstates20(handles[i], &info);
            if (status != NVAPI_OK)
            {
                PrintNvapiError("  NvAPI_GPU_GetPstates20 failed", status);
                continue;
            }

            NV_GPU_PSTATE20_CLOCK_ENTRY_V1 *clockEntry = NULL;
            NV_GPU_PSTATE20_BASE_VOLTAGE_ENTRY_V1 *voltEntry = NULL;
            bool foundPstate = false;
            NvU32 pstateIndex = 0;

            for (NvU32 p = 0; p < info.numPstates; ++p)
            {
                auto &pstate = info.pstates[p];
                if (pstate.pstateId != pstateId)
                {
                    continue;
                }
                foundPstate = true;
                pstateIndex = p;

                if (hasClock)
                {
                    for (NvU32 c = 0; c < info.numClocks; ++c)
                    {
                        if (pstate.clocks[c].domainId == clockId)
                        {
                            clockEntry = &pstate.clocks[c];
                            break;
                        }
                    }
                }
                if (hasVoltage)
                {
                    for (NvU32 v = 0; v < info.numBaseVoltages; ++v)
                    {
                        if (pstate.baseVoltages[v].domainId == voltageId)
                        {
                            voltEntry = &pstate.baseVoltages[v];
                            break;
                        }
                    }
                }
                break;
            }

            if (!foundPstate)
            {
                std::printf("  Pstate not found: %s\n", PstateName(pstateId));
                continue;
            }
            if (hasClock && !clockEntry)
            {
                std::printf("  Clock domain not found in pstate.\n");
                continue;
            }
            if (hasVoltage && !voltEntry)
            {
                std::printf("  Voltage domain not found in pstate.\n");
                continue;
            }
            if (!info.bIsEditable)
            {
                std::printf("  Pstates20 not editable on this GPU.\n");
                continue;
            }

            if (clockEntry)
            {
                if (!clockEntry->bIsEditable)
                {
                    std::printf("  Clock domain not editable.\n");
                    continue;
                }
                if (clockDelta < clockEntry->freqDelta_kHz.valueRange.min ||
                    clockDelta > clockEntry->freqDelta_kHz.valueRange.max)
                {
                    std::printf("  Clock delta out of range (%d to %d kHz).\n",
                        clockEntry->freqDelta_kHz.valueRange.min,
                        clockEntry->freqDelta_kHz.valueRange.max);
                    continue;
                }
                clockEntry->freqDelta_kHz.value = clockDelta;
            }

            if (voltEntry)
            {
                if (!voltEntry->bIsEditable)
                {
                    std::printf("  Voltage domain not editable.\n");
                    continue;
                }
                if (voltageDelta < voltEntry->voltDelta_uV.valueRange.min ||
                    voltageDelta > voltEntry->voltDelta_uV.valueRange.max)
                {
                    std::printf("  Voltage delta out of range (%d to %d uV).\n",
                        voltEntry->voltDelta_uV.valueRange.min,
                        voltEntry->voltDelta_uV.valueRange.max);
                    continue;
                }
                voltEntry->voltDelta_uV.value = voltageDelta;
            }

            std::printf("  Pstates20 set request: %s\n", PstateName(pstateId));
            if (clockEntry)
            {
                std::printf("    clock %s delta=%d kHz\n", ClockDomainName(clockId), clockDelta);
            }
            if (voltEntry)
            {
                std::printf("    voltage %s delta=%d uV\n", VoltageDomainName(voltageId), voltageDelta);
            }

            NV_GPU_PERF_PSTATES20_INFO setInfo = {};
            setInfo.version = info.version;
            setInfo.bIsEditable = info.bIsEditable;
            setInfo.numPstates = 1;
            setInfo.numClocks = info.numClocks;
            setInfo.numBaseVoltages = info.numBaseVoltages;
            setInfo.pstates[0] = info.pstates[pstateIndex];
            setInfo.ov.numVoltages = 0;

            for (NvU32 c = 0; c < setInfo.numClocks; ++c)
            {
                auto &entry = setInfo.pstates[0].clocks[c];
                if (!hasClock || entry.domainId != clockId)
                {
                    entry.domainId = NVAPI_GPU_PUBLIC_CLOCK_UNDEFINED;
                }
            }
            for (NvU32 v = 0; v < setInfo.numBaseVoltages; ++v)
            {
                auto &entry = setInfo.pstates[0].baseVoltages[v];
                if (!hasVoltage || entry.domainId != voltageId)
                {
                    entry.domainId = NVAPI_GPU_PERF_VOLTAGE_INFO_DOMAIN_UNDEFINED;
                }
            }

            status = NvAPI_GPU_SetPstates20(handles[i], &setInfo);
            if (status != NVAPI_OK)
            {
                PrintNvapiError("  NvAPI_GPU_SetPstates20 failed", status);
                continue;
            }

            std::printf("  Pstates20 updated.\n");
        }
        return 0;
    }

    int CmdGpuVfTables(int argc, char **argv)
    {
        NvU32 index = 0;
        bool hasIndex = false;
        if (!ParseGpuIndexArgs(argc, argv, &index, &hasIndex))
        {
            return 1;
        }

        std::vector<NvPhysicalGpuHandle> handles;
        std::vector<NvU32> indices;
        if (!CollectGpus(hasIndex, index, handles, indices))
        {
            return 1;
        }

        for (size_t i = 0; i < handles.size(); ++i)
        {
            PrintGpuHeader(indices[i], handles[i]);

            NV_GPU_PERF_VF_TABLES tables = {};
            tables.version = NV_GPU_PERF_VF_TABLES_VER;
            NvAPI_Status status = NvAPI_GPU_PerfVfTablesGetInfo(handles[i], &tables);
            if (status != NVAPI_OK)
            {
                PrintNvapiError("  NvAPI_GPU_PerfVfTablesGetInfo failed", status);
                continue;
            }

            std::printf("  VF tables: indexes=%u entries=%u\n", tables.numIndexes, tables.numEntries);
            for (NvU32 t = 0; t < tables.numIndexes; ++t)
            {
                const auto &entry = tables.indexes[t];
                std::printf("    index[%u] pstate=%s domain=%s range=%u-%u\n",
                    t,
                    PstateName(entry.pstateId),
                    ClockDomainIdName(entry.domainId),
                    entry.entryIndexFirst,
                    entry.entryIndexLast);
            }

            for (NvU32 e = 0; e < tables.numEntries; ++e)
            {
                const auto &entry = tables.entries[e];
                const char *stepLabel = NULL;
                if (entry.freqStepSizeKHz == NV_GPU_PERF_VF_ENTRIES_TABLE_STEP_SIZE_NONE)
                {
                    stepLabel = "none";
                }
                else if (entry.freqStepSizeKHz == NV_GPU_PERF_VF_ENTRIES_TABLE_STEP_SIZE_ALL)
                {
                    stepLabel = "all";
                }

                if (stepLabel)
                {
                    std::printf("    entry[%u] maxFreq=%u kHz step=%s pll=%u bypass=%u flags=0x%08X\n",
                        e,
                        entry.maxFreqKHz,
                        stepLabel,
                        entry.bForcePll ? 1 : 0,
                        entry.bForceBypass ? 1 : 0,
                        entry.flags);
                }
                else
                {
                    std::printf("    entry[%u] maxFreq=%u kHz step=%u kHz pll=%u bypass=%u flags=0x%08X\n",
                        e,
                        entry.maxFreqKHz,
                        entry.freqStepSizeKHz,
                        entry.bForcePll ? 1 : 0,
                        entry.bForceBypass ? 1 : 0,
                        entry.flags);
                }
                std::printf("      voltage domain=%s mV=%u vflags=0x%08X\n",
                    PerfVoltageDomainName(entry.voltage.domainId),
                    entry.voltage.mvolt,
                    entry.voltage.flags);
            }
        }
        return 0;
    }

    int CmdGpuVfInject(int argc, char **argv)
    {
        NvU32 index = 0;
        bool hasIndex = false;
        NvU32 clkDomain = 0;
        NvU32 clkFreqKHz = 0;
        bool hasClkDomain = false;
        bool hasClkFreq = false;
        NvU8 voltDomain = 0;
        NvU32 voltRail = 0;
        NvU32 voltUv = 0;
        NvU32 voltMinUv = 0;
        bool hasVoltDomain = false;
        bool hasVoltRail = false;
        bool hasVoltUv = false;
        bool hasVoltMin = false;
        NvU32 flags = 0;

        for (int i = 0; i < argc; ++i)
        {
            if (std::strcmp(argv[i], "--index") == 0)
            {
                if (i + 1 >= argc)
                {
                    std::printf("Missing value for --index\n");
                    return 1;
                }
                if (!ParseUint(argv[i + 1], &index))
                {
                    std::printf("Invalid GPU index: %s\n", argv[i + 1]);
                    return 1;
                }
                hasIndex = true;
                ++i;
                continue;
            }
            if (std::strcmp(argv[i], "--clk-domain") == 0)
            {
                if (i + 1 >= argc)
                {
                    std::printf("Missing value for --clk-domain\n");
                    return 1;
                }
                if (!ParseUint(argv[i + 1], &clkDomain))
                {
                    std::printf("Invalid clk domain: %s\n", argv[i + 1]);
                    return 1;
                }
                hasClkDomain = true;
                ++i;
                continue;
            }
            if (std::strcmp(argv[i], "--clk-khz") == 0)
            {
                if (i + 1 >= argc)
                {
                    std::printf("Missing value for --clk-khz\n");
                    return 1;
                }
                if (!ParseUint(argv[i + 1], &clkFreqKHz))
                {
                    std::printf("Invalid clk-khz: %s\n", argv[i + 1]);
                    return 1;
                }
                hasClkFreq = true;
                ++i;
                continue;
            }
            if (std::strcmp(argv[i], "--volt-domain") == 0)
            {
                if (i + 1 >= argc)
                {
                    std::printf("Missing value for --volt-domain\n");
                    return 1;
                }
                if (!ParseVoltDomainType(argv[i + 1], &voltDomain))
                {
                    std::printf("Invalid volt-domain: %s\n", argv[i + 1]);
                    return 1;
                }
                hasVoltDomain = true;
                ++i;
                continue;
            }
            if (std::strcmp(argv[i], "--volt-rail") == 0)
            {
                if (i + 1 >= argc)
                {
                    std::printf("Missing value for --volt-rail\n");
                    return 1;
                }
                if (!ParseUint(argv[i + 1], &voltRail))
                {
                    std::printf("Invalid volt-rail: %s\n", argv[i + 1]);
                    return 1;
                }
                hasVoltRail = true;
                ++i;
                continue;
            }
            if (std::strcmp(argv[i], "--volt-uv") == 0)
            {
                if (i + 1 >= argc)
                {
                    std::printf("Missing value for --volt-uv\n");
                    return 1;
                }
                if (!ParseUint(argv[i + 1], &voltUv))
                {
                    std::printf("Invalid volt-uv: %s\n", argv[i + 1]);
                    return 1;
                }
                hasVoltUv = true;
                ++i;
                continue;
            }
            if (std::strcmp(argv[i], "--volt-min-uv") == 0)
            {
                if (i + 1 >= argc)
                {
                    std::printf("Missing value for --volt-min-uv\n");
                    return 1;
                }
                if (!ParseUint(argv[i + 1], &voltMinUv))
                {
                    std::printf("Invalid volt-min-uv: %s\n", argv[i + 1]);
                    return 1;
                }
                hasVoltMin = true;
                ++i;
                continue;
            }
            if (std::strcmp(argv[i], "--flags") == 0)
            {
                if (i + 1 >= argc)
                {
                    std::printf("Missing value for --flags\n");
                    return 1;
                }
                if (!ParseUint(argv[i + 1], &flags))
                {
                    std::printf("Invalid flags: %s\n", argv[i + 1]);
                    return 1;
                }
                ++i;
                continue;
            }
            std::printf("Unknown option: %s\n", argv[i]);
            return 1;
        }

        if (hasClkDomain != hasClkFreq)
        {
            std::printf("Both --clk-domain and --clk-khz must be specified together.\n");
            return 1;
        }
        if ((hasVoltDomain || hasVoltRail || hasVoltUv || hasVoltMin) &&
            (!hasVoltDomain || !hasVoltRail || (!hasVoltUv && !hasVoltMin)))
        {
            std::printf("Voltage change requires --volt-domain, --volt-rail and at least one of --volt-uv/--volt-min-uv.\n");
            return 1;
        }
        if (voltRail > 0xFF)
        {
            std::printf("Volt rail index must be 0-255.\n");
            return 1;
        }
        if (!hasClkDomain && !hasVoltDomain && !hasVoltUv && !hasVoltMin)
        {
            std::printf("Specify at least one clock or voltage change.\n");
            return 1;
        }

        std::vector<NvPhysicalGpuHandle> handles;
        std::vector<NvU32> indices;
        if (!CollectGpus(hasIndex, index, handles, indices))
        {
            return 1;
        }

        for (size_t i = 0; i < handles.size(); ++i)
        {
            PrintGpuHeader(indices[i], handles[i]);

            NV_GPU_PERF_VF_CHANGE_INJECT_PARAMS params = {};
            params.version = NV_GPU_PERF_VF_CHANGE_PARAMS_VER;
            params.flags = static_cast<NvU8>(flags);

            if (hasClkDomain)
            {
                params.clkList.numDomains = 1;
                params.clkList.clkDomains[0].clkDomain = clkDomain;
                params.clkList.clkDomains[0].clkFreqKHz = clkFreqKHz;
                params.clkList.clkDomains[0].rsvd1 = 0;
                params.clkList.clkDomains[0].regimeId = 0;
                params.clkList.clkDomains[0].source = 0;
            }

            if (hasVoltDomain)
            {
                params.voltList.numRails = 1;
                params.voltList.rails[0].voltDomain = voltDomain;
                params.voltList.rails[0].railIdx = static_cast<NvU8>(voltRail);
                params.voltList.rails[0].voltageuV = hasVoltUv ? voltUv : 0;
                params.voltList.rails[0].voltageMinNoiseUnawareuV = hasVoltMin ? voltMinUv : 0;
            }

            std::printf("  VF inject request: flags=0x%02X\n", params.flags);
            if (hasClkDomain)
            {
                std::printf("    clock domain=%u freq=%u kHz\n", clkDomain, clkFreqKHz);
            }
            if (hasVoltDomain)
            {
                std::printf("    volt domain=%u rail=%u target=%u uV minNoise=%u uV\n",
                    voltDomain,
                    voltRail,
                    hasVoltUv ? voltUv : 0,
                    hasVoltMin ? voltMinUv : 0);
            }

            NvAPI_Status status = NvAPI_GPU_PerfVfChangeInject(handles[i], &params);
            if (status != NVAPI_OK)
            {
                PrintNvapiError("  NvAPI_GPU_PerfVfChangeInject failed", status);
                continue;
            }

            std::printf("  VF change injected.\n");
        }

        return 0;
    }

    int CmdGpuVpstatesInfo(int argc, char **argv)
    {
        NvU32 index = 0;
        bool hasIndex = false;
        if (!ParseGpuIndexArgs(argc, argv, &index, &hasIndex))
        {
            return 1;
        }

        std::vector<NvPhysicalGpuHandle> handles;
        std::vector<NvU32> indices;
        if (!CollectGpus(hasIndex, index, handles, indices))
        {
            return 1;
        }

        for (size_t i = 0; i < handles.size(); ++i)
        {
            PrintGpuHeader(indices[i], handles[i]);

            NV_GPU_PERF_VPSTATES_INFO info = {};
            info.version = NV_GPU_PERF_VPSTATES_INFO_VER;
            NvAPI_Status status = NvAPI_GPU_PerfVpstatesGetInfo(handles[i], &info);
            if (status != NVAPI_OK)
            {
                PrintNvapiError("  NvAPI_GPU_PerfVpstatesGetInfo failed", status);
                continue;
            }

            std::printf("  VPSTATES info:\n");
            for (NvU32 v = 0; v < NV_GPU_PERF_VPSTATE_MAX_V1; ++v)
            {
                if (!MaskE255Has(info.vpstatesMask, v))
                {
                    continue;
                }

                const auto &vp = info.vpstates[v];
                std::printf("  VPSTATE[%u] type=%s\n", v, VpstateTypeName(vp.type));

                if (vp.type == NV_GPU_PERF_VPSTATE_TYPE_2X)
                {
                    std::printf("    vPstateNum=%u reqPower=%u reqSlowdown=%u\n",
                        vp.data.v2x.vPstateNum,
                        vp.data.v2x.reqPower10mW,
                        vp.data.v2x.reqSlowdownPower10mW);
                    for (NvU32 g = 0; g < NV_GPU_PERF_VPSTATE_2X_DOMAIN_GROUP_NUM; ++g)
                    {
                        std::printf("    group[%u]=%u\n", g, vp.data.v2x.value[g]);
                    }
                }
                else if (vp.type == NV_GPU_PERF_VPSTATE_TYPE_3X)
                {
                    std::printf("    pstateIdx=%u\n", vp.data.v3x.pstateIdx);
                    bool anyClock = false;
                    for (NvU32 c = 0; c < NV_GPU_CLOCK_CLK_DOMAINS_MAX_V1; ++c)
                    {
                        const auto &clock = vp.data.v3x.clocks[c];
                        if (clock.targetFreqMHz == 0 && clock.minEffFreqMHz == 0)
                        {
                            continue;
                        }
                        anyClock = true;
                        std::printf("    clock[%u] target=%u MHz minEff=%u MHz\n",
                            c,
                            clock.targetFreqMHz,
                            clock.minEffFreqMHz);
                    }
                    if (!anyClock)
                    {
                        std::printf("    clocks: none\n");
                    }
                }
            }
        }

        return 0;
    }

    int CmdGpuVpstatesControl(int argc, char **argv)
    {
        NvU32 index = 0;
        bool hasIndex = false;
        bool original = false;

        for (int i = 0; i < argc; ++i)
        {
            if (std::strcmp(argv[i], "--index") == 0)
            {
                if (i + 1 >= argc)
                {
                    std::printf("Missing value for --index\n");
                    return 1;
                }
                if (!ParseUint(argv[i + 1], &index))
                {
                    std::printf("Invalid GPU index: %s\n", argv[i + 1]);
                    return 1;
                }
                hasIndex = true;
                ++i;
                continue;
            }
            if (std::strcmp(argv[i], "--original") == 0)
            {
                original = true;
                continue;
            }
            std::printf("Unknown option: %s\n", argv[i]);
            return 1;
        }

        std::vector<NvPhysicalGpuHandle> handles;
        std::vector<NvU32> indices;
        if (!CollectGpus(hasIndex, index, handles, indices))
        {
            return 1;
        }

        for (size_t i = 0; i < handles.size(); ++i)
        {
            PrintGpuHeader(indices[i], handles[i]);

            NV_GPU_PERF_VPSTATES_CONTROL control = {};
            control.version = NV_GPU_PERF_VPSTATES_CONTROL_VER;
            control.bOriginal = original ? NV_TRUE : NV_FALSE;
            NvAPI_Status status = NvAPI_GPU_PerfVpstatesGetControl(handles[i], &control);
            if (status != NVAPI_OK)
            {
                PrintNvapiError("  NvAPI_GPU_PerfVpstatesGetControl failed", status);
                continue;
            }

            std::printf("  VPSTATES control (original=%u):\n", control.bOriginal ? 1 : 0);
            for (NvU32 v = 0; v < NV_GPU_PERF_VPSTATE_MAX_V1; ++v)
            {
                if (!MaskE255Has(control.vpstatesMask, v))
                {
                    continue;
                }

                const auto &vp = control.vpstates[v];
                std::printf("  VPSTATE[%u] type=%s\n", v, VpstateTypeName(vp.type));
                if (vp.type == NV_GPU_PERF_VPSTATE_TYPE_2X)
                {
                    for (NvU32 g = 0; g < NV_GPU_PERF_VPSTATE_2X_DOMAIN_GROUP_NUM; ++g)
                    {
                        std::printf("    group[%u]=%u\n", g, vp.data.v2x.value[g]);
                    }
                }
                else if (vp.type == NV_GPU_PERF_VPSTATE_TYPE_3X)
                {
                    std::printf("    pstateIdx=%u\n", vp.data.v3x.pstateIdx);
                    bool anyClock = false;
                    for (NvU32 c = 0; c < NV_GPU_CLOCK_CLK_DOMAINS_MAX_V1; ++c)
                    {
                        const auto &clock = vp.data.v3x.clocks[c];
                        if (clock.targetFreqMHz == 0 && clock.minEffFreqMHz == 0)
                        {
                            continue;
                        }
                        anyClock = true;
                        std::printf("    clock[%u] target=%u MHz minEff=%u MHz\n",
                            c,
                            clock.targetFreqMHz,
                            clock.minEffFreqMHz);
                    }
                    if (!anyClock)
                    {
                        std::printf("    clocks: none\n");
                    }
                }
            }
        }

        return 0;
    }

    int CmdGpuVpstatesSet(int argc, char **argv)
    {
        NvU32 index = 0;
        bool hasIndex = false;
        NvU32 vpstateIndex = 0;
        bool hasVpstate = false;
        NvU32 clockIndex = 0;
        bool hasClock = false;
        NvU32 targetMHz = 0;
        bool hasTarget = false;
        NvU32 minEffMHz = 0;
        bool hasMinEff = false;
        NvU32 groupIndex = 0;
        bool hasGroup = false;
        NvU32 groupValue = 0;
        bool hasValue = false;

        for (int i = 0; i < argc; ++i)
        {
            if (std::strcmp(argv[i], "--index") == 0)
            {
                if (i + 1 >= argc)
                {
                    std::printf("Missing value for --index\n");
                    return 1;
                }
                if (!ParseUint(argv[i + 1], &index))
                {
                    std::printf("Invalid GPU index: %s\n", argv[i + 1]);
                    return 1;
                }
                hasIndex = true;
                ++i;
                continue;
            }
            if (std::strcmp(argv[i], "--vpstate") == 0)
            {
                if (i + 1 >= argc)
                {
                    std::printf("Missing value for --vpstate\n");
                    return 1;
                }
                if (!ParseUint(argv[i + 1], &vpstateIndex))
                {
                    std::printf("Invalid vpstate index: %s\n", argv[i + 1]);
                    return 1;
                }
                hasVpstate = true;
                ++i;
                continue;
            }
            if (std::strcmp(argv[i], "--clock") == 0)
            {
                if (i + 1 >= argc)
                {
                    std::printf("Missing value for --clock\n");
                    return 1;
                }
                if (!ParseUint(argv[i + 1], &clockIndex))
                {
                    std::printf("Invalid clock index: %s\n", argv[i + 1]);
                    return 1;
                }
                hasClock = true;
                ++i;
                continue;
            }
            if (std::strcmp(argv[i], "--target-mhz") == 0)
            {
                if (i + 1 >= argc)
                {
                    std::printf("Missing value for --target-mhz\n");
                    return 1;
                }
                if (!ParseUint(argv[i + 1], &targetMHz))
                {
                    std::printf("Invalid target-mhz: %s\n", argv[i + 1]);
                    return 1;
                }
                hasTarget = true;
                ++i;
                continue;
            }
            if (std::strcmp(argv[i], "--min-eff-mhz") == 0)
            {
                if (i + 1 >= argc)
                {
                    std::printf("Missing value for --min-eff-mhz\n");
                    return 1;
                }
                if (!ParseUint(argv[i + 1], &minEffMHz))
                {
                    std::printf("Invalid min-eff-mhz: %s\n", argv[i + 1]);
                    return 1;
                }
                hasMinEff = true;
                ++i;
                continue;
            }
            if (std::strcmp(argv[i], "--group") == 0)
            {
                if (i + 1 >= argc)
                {
                    std::printf("Missing value for --group\n");
                    return 1;
                }
                if (!ParseUint(argv[i + 1], &groupIndex))
                {
                    std::printf("Invalid group index: %s\n", argv[i + 1]);
                    return 1;
                }
                hasGroup = true;
                ++i;
                continue;
            }
            if (std::strcmp(argv[i], "--value") == 0)
            {
                if (i + 1 >= argc)
                {
                    std::printf("Missing value for --value\n");
                    return 1;
                }
                if (!ParseUint(argv[i + 1], &groupValue))
                {
                    std::printf("Invalid value: %s\n", argv[i + 1]);
                    return 1;
                }
                hasValue = true;
                ++i;
                continue;
            }
            std::printf("Unknown option: %s\n", argv[i]);
            return 1;
        }

        if (!hasVpstate)
        {
            std::printf("Missing required --vpstate\n");
            return 1;
        }

        const bool usesClock = hasClock || hasTarget || hasMinEff;
        const bool usesGroup = hasGroup || hasValue;
        if (usesClock == usesGroup)
        {
            std::printf("Specify either clock parameters (--clock/--target-mhz) or group parameters (--group/--value).\n");
            return 1;
        }
        if (usesClock && (!hasClock || !hasTarget))
        {
            std::printf("Clock updates require --clock and --target-mhz.\n");
            return 1;
        }
        if (usesGroup && (!hasGroup || !hasValue))
        {
            std::printf("Group updates require --group and --value.\n");
            return 1;
        }
        if (hasTarget && targetMHz > 0xFFFF)
        {
            std::printf("target-mhz must be <= 65535.\n");
            return 1;
        }
        if (hasMinEff && minEffMHz > 0xFFFF)
        {
            std::printf("min-eff-mhz must be <= 65535.\n");
            return 1;
        }

        std::vector<NvPhysicalGpuHandle> handles;
        std::vector<NvU32> indices;
        if (!CollectGpus(hasIndex, index, handles, indices))
        {
            return 1;
        }

        for (size_t i = 0; i < handles.size(); ++i)
        {
            PrintGpuHeader(indices[i], handles[i]);

            NV_GPU_PERF_VPSTATES_CONTROL control = {};
            control.version = NV_GPU_PERF_VPSTATES_CONTROL_VER;
            control.bOriginal = NV_FALSE;
            NvAPI_Status status = NvAPI_GPU_PerfVpstatesGetControl(handles[i], &control);
            if (status != NVAPI_OK)
            {
                PrintNvapiError("  NvAPI_GPU_PerfVpstatesGetControl failed", status);
                continue;
            }

            if (!MaskE255Has(control.vpstatesMask, vpstateIndex))
            {
                std::printf("  VPSTATE index not present: %u\n", vpstateIndex);
                continue;
            }

            auto &vp = control.vpstates[vpstateIndex];
            if (usesClock)
            {
                if (vp.type != NV_GPU_PERF_VPSTATE_TYPE_3X)
                {
                    std::printf("  VPSTATE[%u] is not 3X.\n", vpstateIndex);
                    continue;
                }
                if (clockIndex >= NV_GPU_CLOCK_CLK_DOMAINS_MAX_V1)
                {
                    std::printf("  Clock index out of range.\n");
                    continue;
                }
                vp.data.v3x.clocks[clockIndex].targetFreqMHz = static_cast<NvU16>(targetMHz);
                if (hasMinEff)
                {
                    vp.data.v3x.clocks[clockIndex].minEffFreqMHz = static_cast<NvU16>(minEffMHz);
                }
                std::printf("  VPSTATE set request: vpstate=%u clock=%u target=%u MHz minEff=%u MHz\n",
                    vpstateIndex,
                    clockIndex,
                    vp.data.v3x.clocks[clockIndex].targetFreqMHz,
                    vp.data.v3x.clocks[clockIndex].minEffFreqMHz);
            }
            else
            {
                if (vp.type != NV_GPU_PERF_VPSTATE_TYPE_2X)
                {
                    std::printf("  VPSTATE[%u] is not 2X.\n", vpstateIndex);
                    continue;
                }
                if (groupIndex >= NV_GPU_PERF_VPSTATE_2X_DOMAIN_GROUP_NUM)
                {
                    std::printf("  Group index out of range.\n");
                    continue;
                }
                vp.data.v2x.value[groupIndex] = groupValue;
                std::printf("  VPSTATE set request: vpstate=%u group=%u value=%u\n",
                    vpstateIndex,
                    groupIndex,
                    groupValue);
            }

            status = NvAPI_GPU_PerfVpstatesSetControl(handles[i], &control);
            if (status != NVAPI_OK)
            {
                PrintNvapiError("  NvAPI_GPU_PerfVpstatesSetControl failed", status);
                continue;
            }
            std::printf("  VPSTATE updated.\n");
        }

        return 0;
    }

    int CmdGpuVfeVarsInfo(int argc, char **argv)
    {
        NvU32 index = 0;
        bool hasIndex = false;
        if (!ParseGpuIndexArgs(argc, argv, &index, &hasIndex))
        {
            return 1;
        }

        std::vector<NvPhysicalGpuHandle> handles;
        std::vector<NvU32> indices;
        if (!CollectGpus(hasIndex, index, handles, indices))
        {
            return 1;
        }

        for (size_t i = 0; i < handles.size(); ++i)
        {
            PrintGpuHeader(indices[i], handles[i]);

            NV_GPU_PERF_VFE_VARS_INFO info = {};
            info.version = NV_GPU_PERF_VFE_VARS_INFO_VER;
            NvAPI_Status status = NvAPI_GPU_PerfVfeVarGetInfo(handles[i], &info);
            if (status != NVAPI_OK)
            {
                PrintNvapiError("  NvAPI_GPU_PerfVfeVarGetInfo failed", status);
                continue;
            }

            std::printf("  VFE vars info:\n");
            for (NvU32 v = 0; v < NV_GPU_PERF_VFE_VAR_MAX_V1; ++v)
            {
                if (!MaskE32Has(info.varsMask, v))
                {
                    continue;
                }

                const auto &var = info.vars[v];
                std::printf("  VAR[%u] type=%s\n", v, VfeVarTypeName(var.type));

                if (var.type == NV_GPU_PERF_VFE_VAR_TYPE_DERIVED_PRODUCT)
                {
                    std::printf("    product varIdx0=%u varIdx1=%u\n",
                        var.data.derivedProd.varIdx0,
                        var.data.derivedProd.varIdx1);
                }
                else if (var.type == NV_GPU_PERF_VFE_VAR_TYPE_DERIVED_SUM)
                {
                    std::printf("    sum varIdx0=%u varIdx1=%u\n",
                        var.data.derivedSum.varIdx0,
                        var.data.derivedSum.varIdx1);
                }
                else if (var.type == NV_GPU_PERF_VFE_VAR_TYPE_SINGLE_SENSED_TEMP)
                {
                    std::printf("    thermChannelIndex=%u\n", var.data.sensedTemp.thermChannelIndex);
                }
                else if (var.type == NV_GPU_PERF_VFE_VAR_TYPE_SINGLE_SENSED_FUSE_BASE)
                {
                    std::printf("    fuseVersion=%u fuseValue=%u fuseHw=%u\n",
                        var.data.sensedFuseBase.fuseVersion,
                        var.data.sensedFuseBase.fuseValueInteger,
                        var.data.sensedFuseBase.fuseValueHwInteger);
                }
                else if (var.type == NV_GPU_PERF_VFE_VAR_TYPE_SINGLE_SENSED_FUSE)
                {
                    std::printf("    fuseVersion=%u fuseValue=%u fuseHw=%u\n",
                        var.data.sensedFuse.super.fuseVersion,
                        var.data.sensedFuse.super.fuseValueInteger,
                        var.data.sensedFuse.super.fuseValueHwInteger);
                }
                else if (var.type == NV_GPU_PERF_VFE_VAR_TYPE_SINGLE_SENSED_FUSE_20)
                {
                    std::printf("    fuseVersion=%u fuseValue=%u fuseHw=%u\n",
                        var.data.sensedFuse20.super.fuseVersion,
                        var.data.sensedFuse20.super.fuseValueInteger,
                        var.data.sensedFuse20.super.fuseValueHwInteger);
                }
            }
        }

        return 0;
    }

    int CmdGpuVfeVarsControl(int argc, char **argv)
    {
        NvU32 index = 0;
        bool hasIndex = false;
        if (!ParseGpuIndexArgs(argc, argv, &index, &hasIndex))
        {
            return 1;
        }

        std::vector<NvPhysicalGpuHandle> handles;
        std::vector<NvU32> indices;
        if (!CollectGpus(hasIndex, index, handles, indices))
        {
            return 1;
        }

        for (size_t i = 0; i < handles.size(); ++i)
        {
            PrintGpuHeader(indices[i], handles[i]);

            NV_GPU_PERF_VFE_VARS_CONTROL control = {};
            control.version = NV_GPU_PERF_VFE_VARS_CONTROL_VER;
            NvAPI_Status status = NvAPI_GPU_PerfVfeVarGetControl(handles[i], &control);
            if (status != NVAPI_OK)
            {
                PrintNvapiError("  NvAPI_GPU_PerfVfeVarGetControl failed", status);
                continue;
            }

            std::printf("  VFE vars control (polling=%ums):\n", control.pollingPeriodms);
            for (NvU32 v = 0; v < NV_GPU_PERF_VFE_VAR_MAX_V1; ++v)
            {
                if (!MaskE32Has(control.varsMask, v))
                {
                    continue;
                }

                const auto &var = control.vars[v];
                std::printf("  VAR[%u] type=%s range=%g-%g\n",
                    v,
                    VfeVarTypeName(var.type),
                    var.outRangeMin,
                    var.outRangeMax);

                const NV_GPU_PERF_VFE_VAR_CONTROL_SINGLE *single = GetVfeVarSingleControl(var);
                if (single)
                {
                    std::printf("    override=%s value=%g\n",
                        VfeVarOverrideTypeName(single->overrideType),
                        single->overrideValue);
                }

                if (var.type == NV_GPU_PERF_VFE_VAR_TYPE_SINGLE_SENSED_TEMP)
                {
                    std::printf("    tempHystPos=%d tempHystNeg=%d\n",
                        var.data.sensedTemp.tempHysteresisPositive,
                        var.data.sensedTemp.tempHysteresisNegative);
                }
            }
        }

        return 0;
    }

    int CmdGpuVfeVarsSet(int argc, char **argv)
    {
        NvU32 index = 0;
        bool hasIndex = false;
        NvU32 varIndex = 0;
        bool hasVar = false;

        NvU8 overrideType = 0;
        bool hasOverrideType = false;
        float overrideValue = 0.0f;
        bool hasOverrideValue = false;
        NvS32 tempHystPos = 0;
        NvS32 tempHystNeg = 0;
        bool hasTempHystPos = false;
        bool hasTempHystNeg = false;

        for (int i = 0; i < argc; ++i)
        {
            if (std::strcmp(argv[i], "--index") == 0)
            {
                if (i + 1 >= argc)
                {
                    std::printf("Missing value for --index\n");
                    return 1;
                }
                if (!ParseUint(argv[i + 1], &index))
                {
                    std::printf("Invalid GPU index: %s\n", argv[i + 1]);
                    return 1;
                }
                hasIndex = true;
                ++i;
                continue;
            }
            if (std::strcmp(argv[i], "--var") == 0)
            {
                if (i + 1 >= argc)
                {
                    std::printf("Missing value for --var\n");
                    return 1;
                }
                if (!ParseUint(argv[i + 1], &varIndex))
                {
                    std::printf("Invalid var index: %s\n", argv[i + 1]);
                    return 1;
                }
                hasVar = true;
                ++i;
                continue;
            }
            if (std::strcmp(argv[i], "--override-type") == 0)
            {
                if (i + 1 >= argc)
                {
                    std::printf("Missing value for --override-type\n");
                    return 1;
                }
                if (!ParseVfeVarOverrideType(argv[i + 1], &overrideType))
                {
                    std::printf("Invalid override-type: %s\n", argv[i + 1]);
                    return 1;
                }
                hasOverrideType = true;
                ++i;
                continue;
            }
            if (std::strcmp(argv[i], "--override-value") == 0)
            {
                if (i + 1 >= argc)
                {
                    std::printf("Missing value for --override-value\n");
                    return 1;
                }
                if (!ParseFloat(argv[i + 1], &overrideValue))
                {
                    std::printf("Invalid override-value: %s\n", argv[i + 1]);
                    return 1;
                }
                hasOverrideValue = true;
                ++i;
                continue;
            }
            if (std::strcmp(argv[i], "--temp-hyst-pos") == 0)
            {
                if (i + 1 >= argc)
                {
                    std::printf("Missing value for --temp-hyst-pos\n");
                    return 1;
                }
                if (!ParseInt(argv[i + 1], &tempHystPos))
                {
                    std::printf("Invalid temp-hyst-pos: %s\n", argv[i + 1]);
                    return 1;
                }
                hasTempHystPos = true;
                ++i;
                continue;
            }
            if (std::strcmp(argv[i], "--temp-hyst-neg") == 0)
            {
                if (i + 1 >= argc)
                {
                    std::printf("Missing value for --temp-hyst-neg\n");
                    return 1;
                }
                if (!ParseInt(argv[i + 1], &tempHystNeg))
                {
                    std::printf("Invalid temp-hyst-neg: %s\n", argv[i + 1]);
                    return 1;
                }
                hasTempHystNeg = true;
                ++i;
                continue;
            }
            std::printf("Unknown option: %s\n", argv[i]);
            return 1;
        }

        if (!hasVar || !hasOverrideType)
        {
            std::printf("Missing required --var and --override-type.\n");
            return 1;
        }
        if (overrideType != NV_GPU_PERF_VFE_VAR_SINGLE_OVERRIDE_TYPE_NONE && !hasOverrideValue)
        {
            std::printf("override-value is required when override-type is not none.\n");
            return 1;
        }

        std::vector<NvPhysicalGpuHandle> handles;
        std::vector<NvU32> indices;
        if (!CollectGpus(hasIndex, index, handles, indices))
        {
            return 1;
        }

        for (size_t i = 0; i < handles.size(); ++i)
        {
            PrintGpuHeader(indices[i], handles[i]);

            NV_GPU_PERF_VFE_VARS_CONTROL control = {};
            control.version = NV_GPU_PERF_VFE_VARS_CONTROL_VER;
            NvAPI_Status status = NvAPI_GPU_PerfVfeVarGetControl(handles[i], &control);
            if (status != NVAPI_OK)
            {
                PrintNvapiError("  NvAPI_GPU_PerfVfeVarGetControl failed", status);
                continue;
            }

            if (varIndex >= NV_GPU_PERF_VFE_VAR_MAX_V1 || !MaskE32Has(control.varsMask, varIndex))
            {
                std::printf("  Var index not present: %u\n", varIndex);
                continue;
            }

            auto &var = control.vars[varIndex];
            NV_GPU_PERF_VFE_VAR_CONTROL_SINGLE *single = GetVfeVarSingleControl(var);
            if (!single)
            {
                std::printf("  Var[%u] type %s does not support overrides.\n",
                    varIndex,
                    VfeVarTypeName(var.type));
                continue;
            }

            single->overrideType = overrideType;
            if (hasOverrideValue)
            {
                single->overrideValue = overrideValue;
            }

            if (hasTempHystPos || hasTempHystNeg)
            {
                if (var.type != NV_GPU_PERF_VFE_VAR_TYPE_SINGLE_SENSED_TEMP)
                {
                    std::printf("  Var[%u] is not a sensed temperature variable.\n", varIndex);
                    continue;
                }
                if (hasTempHystPos)
                {
                    var.data.sensedTemp.tempHysteresisPositive = tempHystPos;
                }
                if (hasTempHystNeg)
                {
                    var.data.sensedTemp.tempHysteresisNegative = tempHystNeg;
                }
            }

            std::printf("  VFE var set request: var=%u override=%s value=%g\n",
                varIndex,
                VfeVarOverrideTypeName(single->overrideType),
                single->overrideValue);

            status = NvAPI_GPU_PerfVfeVarSetControl(handles[i], &control);
            if (status != NVAPI_OK)
            {
                PrintNvapiError("  NvAPI_GPU_PerfVfeVarSetControl failed", status);
                continue;
            }

            std::printf("  VFE var updated.\n");
        }

        return 0;
    }

    int CmdGpuVfeEqusInfo(int argc, char **argv)
    {
        NvU32 index = 0;
        bool hasIndex = false;
        if (!ParseGpuIndexArgs(argc, argv, &index, &hasIndex))
        {
            return 1;
        }

        std::vector<NvPhysicalGpuHandle> handles;
        std::vector<NvU32> indices;
        if (!CollectGpus(hasIndex, index, handles, indices))
        {
            return 1;
        }

        for (size_t i = 0; i < handles.size(); ++i)
        {
            PrintGpuHeader(indices[i], handles[i]);

            NV_GPU_PERF_VFE_EQUS_INFO info = {};
            info.version = NV_GPU_PERF_VFE_EQUS_INFO_VER;
            NvAPI_Status status = NvAPI_GPU_PerfVfeEquGetInfo(handles[i], &info);
            if (status != NVAPI_OK)
            {
                PrintNvapiError("  NvAPI_GPU_PerfVfeEquGetInfo failed", status);
                continue;
            }

            std::printf("  VFE equations info:\n");
            for (NvU32 e = 0; e < NV_GPU_PERF_VFE_EQU_MAX_V1; ++e)
            {
                if (!MaskE255Has(info.equsMask, e))
                {
                    continue;
                }
                const auto &equ = info.equs[e];
                std::printf("  EQU[%u] type=%s varIdx=%u next=%u output=%s\n",
                    e,
                    VfeEquTypeName(equ.type),
                    equ.varIdx,
                    equ.equIdxNext,
                    VfeEquOutputTypeName(equ.outputType));
            }
        }

        return 0;
    }

    int CmdGpuVfeEqusControl(int argc, char **argv)
    {
        NvU32 index = 0;
        bool hasIndex = false;
        if (!ParseGpuIndexArgs(argc, argv, &index, &hasIndex))
        {
            return 1;
        }

        std::vector<NvPhysicalGpuHandle> handles;
        std::vector<NvU32> indices;
        if (!CollectGpus(hasIndex, index, handles, indices))
        {
            return 1;
        }

        for (size_t i = 0; i < handles.size(); ++i)
        {
            PrintGpuHeader(indices[i], handles[i]);

            NV_GPU_PERF_VFE_EQUS_CONTROL control = {};
            control.version = NV_GPU_PERF_VFE_EQUS_CONTROL_VER;
            NvAPI_Status status = NvAPI_GPU_PerfVfeEquGetControl(handles[i], &control);
            if (status != NVAPI_OK)
            {
                PrintNvapiError("  NvAPI_GPU_PerfVfeEquGetControl failed", status);
                continue;
            }

            std::printf("  VFE equations control:\n");
            for (NvU32 e = 0; e < NV_GPU_PERF_VFE_EQU_MAX_V1; ++e)
            {
                if (!MaskE255Has(control.equsMask, e))
                {
                    continue;
                }

                const auto &equ = control.equs[e];
                std::printf("  EQU[%u] type=%s range=%g-%g\n",
                    e,
                    VfeEquTypeName(equ.type),
                    equ.outRangeMin,
                    equ.outRangeMax);

                if (equ.type == NV_GPU_PERF_VFE_EQU_TYPE_COMPARE)
                {
                    std::printf("    compare func=%s crit=%g\n",
                        VfeEquCompareFuncName(equ.data.compare.funcId),
                        equ.data.compare.criteria);
                }
                else if (equ.type == NV_GPU_PERF_VFE_EQU_TYPE_MINMAX)
                {
                    std::printf("    minmax=%s\n", equ.data.minmax.bMax ? "max" : "min");
                }
                else if (equ.type == NV_GPU_PERF_VFE_EQU_TYPE_QUADRATIC)
                {
                    std::printf("    coeffs=%g,%g,%g\n",
                        equ.data.quadratic.coeffs[0],
                        equ.data.quadratic.coeffs[1],
                        equ.data.quadratic.coeffs[2]);
                }
            }
        }

        return 0;
    }

    int CmdGpuVfeEqusSet(int argc, char **argv)
    {
        NvU32 index = 0;
        bool hasIndex = false;
        NvU32 equIndex = 0;
        bool hasEqu = false;

        NvU8 compareFunc = 0;
        bool hasCompare = false;
        float compareCrit = 0.0f;
        bool hasCompareCrit = false;

        bool hasMinMax = false;
        NvU8 minMaxIsMax = 0;

        NvF32 coeffs[3] = {};
        bool hasCoeffs = false;

        for (int i = 0; i < argc; ++i)
        {
            if (std::strcmp(argv[i], "--index") == 0)
            {
                if (i + 1 >= argc)
                {
                    std::printf("Missing value for --index\n");
                    return 1;
                }
                if (!ParseUint(argv[i + 1], &index))
                {
                    std::printf("Invalid GPU index: %s\n", argv[i + 1]);
                    return 1;
                }
                hasIndex = true;
                ++i;
                continue;
            }
            if (std::strcmp(argv[i], "--equ") == 0)
            {
                if (i + 1 >= argc)
                {
                    std::printf("Missing value for --equ\n");
                    return 1;
                }
                if (!ParseUint(argv[i + 1], &equIndex))
                {
                    std::printf("Invalid equ index: %s\n", argv[i + 1]);
                    return 1;
                }
                hasEqu = true;
                ++i;
                continue;
            }
            if (std::strcmp(argv[i], "--compare-func") == 0)
            {
                if (i + 1 >= argc)
                {
                    std::printf("Missing value for --compare-func\n");
                    return 1;
                }
                if (!ParseVfeEquCompareFunc(argv[i + 1], &compareFunc))
                {
                    std::printf("Invalid compare-func: %s\n", argv[i + 1]);
                    return 1;
                }
                hasCompare = true;
                ++i;
                continue;
            }
            if (std::strcmp(argv[i], "--compare-crit") == 0)
            {
                if (i + 1 >= argc)
                {
                    std::printf("Missing value for --compare-crit\n");
                    return 1;
                }
                if (!ParseFloat(argv[i + 1], &compareCrit))
                {
                    std::printf("Invalid compare-crit: %s\n", argv[i + 1]);
                    return 1;
                }
                hasCompareCrit = true;
                ++i;
                continue;
            }
            if (std::strcmp(argv[i], "--minmax") == 0)
            {
                if (i + 1 >= argc)
                {
                    std::printf("Missing value for --minmax\n");
                    return 1;
                }
                if (std::strcmp(argv[i + 1], "min") == 0)
                {
                    minMaxIsMax = 0;
                }
                else if (std::strcmp(argv[i + 1], "max") == 0)
                {
                    minMaxIsMax = 1;
                }
                else
                {
                    std::printf("Invalid minmax: %s\n", argv[i + 1]);
                    return 1;
                }
                hasMinMax = true;
                ++i;
                continue;
            }
            if (std::strcmp(argv[i], "--coeffs") == 0)
            {
                if (i + 1 >= argc)
                {
                    std::printf("Missing value for --coeffs\n");
                    return 1;
                }
                if (!ParseCsvFloat3(argv[i + 1], coeffs))
                {
                    std::printf("Invalid coeffs: %s\n", argv[i + 1]);
                    return 1;
                }
                hasCoeffs = true;
                ++i;
                continue;
            }
            std::printf("Unknown option: %s\n", argv[i]);
            return 1;
        }

        if (!hasEqu)
        {
            std::printf("Missing required --equ\n");
            return 1;
        }

        const int modeCount = (hasCompare ? 1 : 0) + (hasMinMax ? 1 : 0) + (hasCoeffs ? 1 : 0);
        if (modeCount != 1)
        {
            std::printf("Specify exactly one of --compare-func/--compare-crit, --minmax, or --coeffs.\n");
            return 1;
        }
        if (hasCompare && !hasCompareCrit)
        {
            std::printf("--compare-crit is required with --compare-func.\n");
            return 1;
        }

        std::vector<NvPhysicalGpuHandle> handles;
        std::vector<NvU32> indices;
        if (!CollectGpus(hasIndex, index, handles, indices))
        {
            return 1;
        }

        for (size_t i = 0; i < handles.size(); ++i)
        {
            PrintGpuHeader(indices[i], handles[i]);

            NV_GPU_PERF_VFE_EQUS_CONTROL control = {};
            control.version = NV_GPU_PERF_VFE_EQUS_CONTROL_VER;
            NvAPI_Status status = NvAPI_GPU_PerfVfeEquGetControl(handles[i], &control);
            if (status != NVAPI_OK)
            {
                PrintNvapiError("  NvAPI_GPU_PerfVfeEquGetControl failed", status);
                continue;
            }

            if (equIndex >= NV_GPU_PERF_VFE_EQU_MAX_V1 || !MaskE255Has(control.equsMask, equIndex))
            {
                std::printf("  EQU index not present: %u\n", equIndex);
                continue;
            }

            auto &equ = control.equs[equIndex];
            if (hasCompare)
            {
                if (equ.type != NV_GPU_PERF_VFE_EQU_TYPE_COMPARE)
                {
                    std::printf("  EQU[%u] type is not COMPARE.\n", equIndex);
                    continue;
                }
                equ.data.compare.funcId = compareFunc;
                equ.data.compare.criteria = compareCrit;
                std::printf("  VFE equ set request: equ=%u compare=%s crit=%g\n",
                    equIndex,
                    VfeEquCompareFuncName(compareFunc),
                    compareCrit);
            }
            else if (hasMinMax)
            {
                if (equ.type != NV_GPU_PERF_VFE_EQU_TYPE_MINMAX)
                {
                    std::printf("  EQU[%u] type is not MINMAX.\n", equIndex);
                    continue;
                }
                equ.data.minmax.bMax = minMaxIsMax;
                std::printf("  VFE equ set request: equ=%u minmax=%s\n",
                    equIndex,
                    minMaxIsMax ? "max" : "min");
            }
            else if (hasCoeffs)
            {
                if (equ.type != NV_GPU_PERF_VFE_EQU_TYPE_QUADRATIC)
                {
                    std::printf("  EQU[%u] type is not QUADRATIC.\n", equIndex);
                    continue;
                }
                equ.data.quadratic.coeffs[0] = coeffs[0];
                equ.data.quadratic.coeffs[1] = coeffs[1];
                equ.data.quadratic.coeffs[2] = coeffs[2];
                std::printf("  VFE equ set request: equ=%u coeffs=%g,%g,%g\n",
                    equIndex,
                    coeffs[0],
                    coeffs[1],
                    coeffs[2]);
            }

            status = NvAPI_GPU_PerfVfeEquSetControl(handles[i], &control);
            if (status != NVAPI_OK)
            {
                PrintNvapiError("  NvAPI_GPU_PerfVfeEquSetControl failed", status);
                continue;
            }

            std::printf("  VFE equation updated.\n");
        }

        return 0;
    }

    int CmdGpuVf(int argc, char **argv)
    {
        if (argc < 1)
        {
            PrintUsageGroup("gpu");
            return 1;
        }
        if (std::strcmp(argv[0], "help") == 0)
        {
            PrintUsageGroup("gpu");
            return 0;
        }
        if (std::strcmp(argv[0], "tables") == 0)
        {
            return CmdGpuVfTables(argc - 1, argv + 1);
        }
        if (std::strcmp(argv[0], "inject") == 0)
        {
            return CmdGpuVfInject(argc - 1, argv + 1);
        }
        std::printf("Unknown vf subcommand: %s\n", argv[0]);
        return 1;
    }

    int CmdGpuVpstates(int argc, char **argv)
    {
        if (argc < 1)
        {
            PrintUsageGroup("gpu");
            return 1;
        }
        if (std::strcmp(argv[0], "help") == 0)
        {
            PrintUsageGroup("gpu");
            return 0;
        }
        if (std::strcmp(argv[0], "info") == 0)
        {
            return CmdGpuVpstatesInfo(argc - 1, argv + 1);
        }
        if (std::strcmp(argv[0], "control") == 0)
        {
            return CmdGpuVpstatesControl(argc - 1, argv + 1);
        }
        if (std::strcmp(argv[0], "set") == 0)
        {
            return CmdGpuVpstatesSet(argc - 1, argv + 1);
        }
        std::printf("Unknown vpstates subcommand: %s\n", argv[0]);
        return 1;
    }

    int CmdGpuVfeVar(int argc, char **argv)
    {
        if (argc < 1)
        {
            PrintUsageGroup("gpu");
            return 1;
        }
        if (std::strcmp(argv[0], "help") == 0)
        {
            PrintUsageGroup("gpu");
            return 0;
        }
        if (std::strcmp(argv[0], "info") == 0)
        {
            return CmdGpuVfeVarsInfo(argc - 1, argv + 1);
        }
        if (std::strcmp(argv[0], "control") == 0)
        {
            return CmdGpuVfeVarsControl(argc - 1, argv + 1);
        }
        if (std::strcmp(argv[0], "set") == 0)
        {
            return CmdGpuVfeVarsSet(argc - 1, argv + 1);
        }
        std::printf("Unknown vfe-var subcommand: %s\n", argv[0]);
        return 1;
    }

    int CmdGpuVfeEqu(int argc, char **argv)
    {
        if (argc < 1)
        {
            PrintUsageGroup("gpu");
            return 1;
        }
        if (std::strcmp(argv[0], "help") == 0)
        {
            PrintUsageGroup("gpu");
            return 0;
        }
        if (std::strcmp(argv[0], "info") == 0)
        {
            return CmdGpuVfeEqusInfo(argc - 1, argv + 1);
        }
        if (std::strcmp(argv[0], "control") == 0)
        {
            return CmdGpuVfeEqusControl(argc - 1, argv + 1);
        }
        if (std::strcmp(argv[0], "set") == 0)
        {
            return CmdGpuVfeEqusSet(argc - 1, argv + 1);
        }
        std::printf("Unknown vfe-equ subcommand: %s\n", argv[0]);
        return 1;
    }

    int CmdGpuPerfLimitsInfo(int argc, char **argv)
    {
        NvU32 index = 0;
        bool hasIndex = false;
        if (!ParseGpuIndexArgs(argc, argv, &index, &hasIndex))
        {
            return 1;
        }

        std::vector<NvPhysicalGpuHandle> handles;
        std::vector<NvU32> indices;
        if (!CollectGpus(hasIndex, index, handles, indices))
        {
            return 1;
        }

        for (size_t i = 0; i < handles.size(); ++i)
        {
            PrintGpuHeader(indices[i], handles[i]);

            NV_GPU_PERF_LIMITS_INFO info = {};
            info.version = NV_GPU_PERF_LIMITS_INFO_VER;
            info.numLimits = 0;
            NvAPI_Status status = NvAPI_GPU_PerfLimitsGetInfo(handles[i], &info);
            if (status != NVAPI_OK)
            {
                PrintNvapiError("  NvAPI_GPU_PerfLimitsGetInfo failed", status);
                continue;
            }

            std::printf("  Perf limits info: %u\n", info.numLimits);
            for (NvU32 l = 0; l < info.numLimits; ++l)
            {
                const auto &limit = info.limits[l];
                char name[NV_GPU_PERF_LIMIT_INFO_NAME_MAX_LENGTH_V1 + 1] = {};
                std::memcpy(name, limit.szName, NV_GPU_PERF_LIMIT_INFO_NAME_MAX_LENGTH_V1);
                std::printf("    id=0x%08X flags=0x%08X priority=%u rm=0x%08X name=%s\n",
                    limit.limitId,
                    limit.flags,
                    limit.priority,
                    limit.rmLimitId,
                    name);
            }
        }

        return 0;
    }

    int CmdGpuPerfLimitsStatus(int argc, char **argv)
    {
        NvU32 index = 0;
        bool hasIndex = false;
        if (!ParseGpuIndexArgs(argc, argv, &index, &hasIndex))
        {
            return 1;
        }

        std::vector<NvPhysicalGpuHandle> handles;
        std::vector<NvU32> indices;
        if (!CollectGpus(hasIndex, index, handles, indices))
        {
            return 1;
        }

        for (size_t i = 0; i < handles.size(); ++i)
        {
            PrintGpuHeader(indices[i], handles[i]);

            std::vector<std::string> names(NVAPI_PERF_LIMIT_ID_MAX_LIMITS);
            NV_GPU_PERF_LIMITS_INFO info = {};
            info.version = NV_GPU_PERF_LIMITS_INFO_VER;
            info.numLimits = 0;
            NvAPI_Status infoStatus = NvAPI_GPU_PerfLimitsGetInfo(handles[i], &info);
            if (infoStatus == NVAPI_OK)
            {
                for (NvU32 l = 0; l < info.numLimits; ++l)
                {
                    const auto &limit = info.limits[l];
                    NvU32 limitId = static_cast<NvU32>(limit.limitId);
                    if (limitId < NVAPI_PERF_LIMIT_ID_MAX_LIMITS)
                    {
                        char name[NV_GPU_PERF_LIMIT_INFO_NAME_MAX_LENGTH_V1 + 1] = {};
                        std::memcpy(name, limit.szName, NV_GPU_PERF_LIMIT_INFO_NAME_MAX_LENGTH_V1);
                        names[limitId] = name;
                    }
                }
            }

            NV_GPU_PERF_LIMITS_STATUS status = {};
            status.version = NV_GPU_PERF_LIMITS_STATUS_VER;
            status.numLimits = 0;
            NvAPI_Status callStatus = NvAPI_GPU_PerfLimitsGetStatus(handles[i], &status);
            if (callStatus != NVAPI_OK)
            {
                PrintNvapiError("  NvAPI_GPU_PerfLimitsGetStatus failed", callStatus);
                continue;
            }

            std::printf("  Perf limits status: %u\n", status.numLimits);
            for (NvU32 l = 0; l < status.numLimits; ++l)
            {
                const auto &limit = status.limits[l];
                NvU32 limitId = static_cast<NvU32>(limit.limitId);
                const char *name = "";
                if (limitId < NVAPI_PERF_LIMIT_ID_MAX_LIMITS && !names[limitId].empty())
                {
                    name = names[limitId].c_str();
                }

                std::printf("    id=0x%08X enabled=%u type=%s value=%u clkDomain=%u %s\n",
                    limit.limitId,
                    limit.output.bEnabled ? 1 : 0,
                    PerfLimitInputTypeName(limit.input.type),
                    limit.output.value,
                    limit.output.decoupledClockId,
                    name);

                if (limit.input.type == NVAPI_PERF_LIMIT_STATUS_INPUT_TYPE_PSTATE)
                {
                    std::printf("      pstate=%s point=%s\n",
                        PstateName(limit.input.data.pstate.pstateId),
                        PerfLimitPstatePointName(limit.input.data.pstate.point));
                }
                else if (limit.input.type == NVAPI_PERF_LIMIT_STATUS_INPUT_TYPE_FREQ)
                {
                    std::printf("      freq=%u kHz domain=%u\n",
                        limit.input.data.freq.freqKHz,
                        limit.input.data.freq.domainId);
                }
                else if (limit.input.type == NVAPI_PERF_LIMIT_STATUS_INPUT_TYPE_VPSTATE)
                {
                    std::printf("      vpstate=%u\n", limit.input.data.vpstate.vpstate);
                }
            }
        }

        return 0;
    }

    int CmdGpuPerfLimitsSet(int argc, char **argv)
    {
        NvU32 index = 0;
        bool hasIndex = false;
        bool hasLimitId = false;
        bool hasType = false;
        NV_GPU_PERF_LIMIT_STATUS_INPUT_TYPE inputType = NVAPI_PERF_LIMIT_STATUS_INPUT_TYPE_DISABLED;
        NvU32 limitIdRaw = 0;
        NV_GPU_PERF_PSTATE_ID pstateId = NVAPI_GPU_PERF_PSTATE_UNDEFINED;
        NV_GPU_PERF_LIMIT_INPUT_DATA_PSTATE_POINT point = NV_GPU_PERF_LIMIT_INPUT_DATA_PSTATE_POINT_NOM;
        bool hasPstate = false;
        bool hasPoint = false;
        bool hasFreq = false;
        NvU32 freqKHz = 0;
        bool hasDomain = false;
        NvU32 domainId = 0;
        bool hasVpstate = false;
        NvU32 vpstate = 0;

        for (int i = 0; i < argc; ++i)
        {
            if (std::strcmp(argv[i], "--index") == 0)
            {
                if (i + 1 >= argc || !ParseUint(argv[i + 1], &index))
                {
                    std::printf("Invalid GPU index.\n");
                    return 1;
                }
                hasIndex = true;
                ++i;
                continue;
            }
            if (std::strcmp(argv[i], "--limit-id") == 0)
            {
                if (i + 1 >= argc || !ParseUint(argv[i + 1], &limitIdRaw))
                {
                    std::printf("Invalid limit id.\n");
                    return 1;
                }
                hasLimitId = true;
                ++i;
                continue;
            }
            if (std::strcmp(argv[i], "--type") == 0)
            {
                if (i + 1 >= argc || !ParsePerfLimitInputType(argv[i + 1], &inputType))
                {
                    std::printf("Invalid input type.\n");
                    return 1;
                }
                hasType = true;
                ++i;
                continue;
            }
            if (std::strcmp(argv[i], "--pstate") == 0)
            {
                if (i + 1 >= argc || !ParsePstateId(argv[i + 1], &pstateId))
                {
                    std::printf("Invalid pstate.\n");
                    return 1;
                }
                hasPstate = true;
                ++i;
                continue;
            }
            if (std::strcmp(argv[i], "--point") == 0)
            {
                if (i + 1 >= argc || !ParsePerfLimitPstatePoint(argv[i + 1], &point))
                {
                    std::printf("Invalid point.\n");
                    return 1;
                }
                hasPoint = true;
                ++i;
                continue;
            }
            if (std::strcmp(argv[i], "--freq-khz") == 0)
            {
                if (i + 1 >= argc || !ParseUint(argv[i + 1], &freqKHz))
                {
                    std::printf("Invalid freq.\n");
                    return 1;
                }
                hasFreq = true;
                ++i;
                continue;
            }
            if (std::strcmp(argv[i], "--domain") == 0)
            {
                if (i + 1 >= argc || !ParseUint(argv[i + 1], &domainId))
                {
                    std::printf("Invalid domain.\n");
                    return 1;
                }
                hasDomain = true;
                ++i;
                continue;
            }
            if (std::strcmp(argv[i], "--vpstate") == 0)
            {
                if (i + 1 >= argc || !ParseUint(argv[i + 1], &vpstate))
                {
                    std::printf("Invalid vpstate.\n");
                    return 1;
                }
                hasVpstate = true;
                ++i;
                continue;
            }
            std::printf("Unknown option: %s\n", argv[i]);
            return 1;
        }

        if (!hasLimitId || !hasType)
        {
            std::printf("Missing required --limit-id and --type\n");
            return 1;
        }

        if (inputType == NVAPI_PERF_LIMIT_STATUS_INPUT_TYPE_PSTATE && !hasPstate)
        {
            std::printf("--pstate is required for type pstate.\n");
            return 1;
        }
        if (inputType == NVAPI_PERF_LIMIT_STATUS_INPUT_TYPE_FREQ && (!hasFreq || !hasDomain))
        {
            std::printf("--freq-khz and --domain are required for type freq.\n");
            return 1;
        }
        if (inputType == NVAPI_PERF_LIMIT_STATUS_INPUT_TYPE_VPSTATE && !hasVpstate)
        {
            std::printf("--vpstate is required for type vpstate.\n");
            return 1;
        }

        std::vector<NvPhysicalGpuHandle> handles;
        std::vector<NvU32> indices;
        if (!CollectGpus(hasIndex, index, handles, indices))
        {
            return 1;
        }

        for (size_t i = 0; i < handles.size(); ++i)
        {
            PrintGpuHeader(indices[i], handles[i]);

            NvU32 rmLimitId = 0;
            NV_GPU_PERF_LIMITS_INFO info = {};
            info.version = NV_GPU_PERF_LIMITS_INFO_VER;
            info.numLimits = 0;
            NvAPI_Status infoStatus = NvAPI_GPU_PerfLimitsGetInfo(handles[i], &info);
            if (infoStatus == NVAPI_OK)
            {
                for (NvU32 l = 0; l < info.numLimits; ++l)
                {
                    if (static_cast<NvU32>(info.limits[l].limitId) == limitIdRaw)
                    {
                        rmLimitId = info.limits[l].rmLimitId;
                        break;
                    }
                }
            }

            NV_GPU_PERF_LIMITS_STATUS limits = {};
            limits.version = NV_GPU_PERF_LIMITS_STATUS_VER;
            limits.numLimits = 1;
            NV_GPU_PERF_LIMIT_STATUS_V6 &entry = limits.limits[0];
            entry.limitId = static_cast<NV_GPU_PERF_LIMIT_ID>(limitIdRaw);
            entry.rmLimitId = rmLimitId;
            entry.flags = 0;
            entry.input.flags = 0;
            entry.input.type = inputType;

            if (inputType == NVAPI_PERF_LIMIT_STATUS_INPUT_TYPE_PSTATE)
            {
                entry.input.data.pstate.flags = 0;
                entry.input.data.pstate.pstateId = pstateId;
                entry.input.data.pstate.point = hasPoint ? point : NV_GPU_PERF_LIMIT_INPUT_DATA_PSTATE_POINT_NOM;
            }
            else if (inputType == NVAPI_PERF_LIMIT_STATUS_INPUT_TYPE_FREQ)
            {
                entry.input.data.freq.flags = 0;
                entry.input.data.freq.freqKHz = freqKHz;
                entry.input.data.freq.domainId = static_cast<NV_GPU_CLOCK_DOMAIN_ID>(domainId);
            }
            else if (inputType == NVAPI_PERF_LIMIT_STATUS_INPUT_TYPE_VPSTATE)
            {
                entry.input.data.vpstate.flags = 0;
                entry.input.data.vpstate.vpstate = vpstate;
            }

            std::printf("  Perf limit set request: id=0x%08X type=%s\n",
                limitIdRaw,
                PerfLimitInputTypeName(inputType));
            if (inputType == NVAPI_PERF_LIMIT_STATUS_INPUT_TYPE_PSTATE)
            {
                std::printf("    pstate=%s point=%s\n",
                    PstateName(pstateId),
                    PerfLimitPstatePointName(entry.input.data.pstate.point));
            }
            else if (inputType == NVAPI_PERF_LIMIT_STATUS_INPUT_TYPE_FREQ)
            {
                std::printf("    freq=%u kHz domain=%u\n", freqKHz, domainId);
            }
            else if (inputType == NVAPI_PERF_LIMIT_STATUS_INPUT_TYPE_VPSTATE)
            {
                std::printf("    vpstate=%u\n", vpstate);
            }

            NvAPI_Status status = NvAPI_GPU_PerfLimitsSetStatus(handles[i], &limits);
            if (status != NVAPI_OK)
            {
                PrintNvapiError("  NvAPI_GPU_PerfLimitsSetStatus failed", status);
                continue;
            }

            std::printf("  Perf limit updated.\n");
        }

        return 0;
    }

    namespace
    {
        void PrintGpuUsage()
        {
            PrintUsageGroup("gpu");
        }

        int CmdGpuListAdapter(int argc, char **argv)
        {
            (void)argc;
            (void)argv;
            return CmdGpuList();
        }

        int CmdGpuPowerDispatch(int argc, char **argv)
        {
            if (argc > 0 && argv && (std::strcmp(argv[0], "help") == 0))
            {
                PrintUsageGroup("gpu");
                return 0;
            }
            static const SubcommandEntry kSubcommands[] = {
                {"limit", CmdGpuPowerLimitGet},
                {"limit-set", CmdGpuPowerLimitSet},
                {"monitor", CmdGpuPowerMonitor},
                {"device", CmdGpuPowerDevice},
                {"capping", CmdGpuPowerCapping},
                {"leakage", CmdGpuPowerLeakage},
            };
            const SubcommandEntry *entry = FindSubcommand(kSubcommands, sizeof(kSubcommands) / sizeof(kSubcommands[0]), argc > 0 ? argv[0] : nullptr);
            if (entry)
            {
                return entry->handler(argc - 1, argv + 1);
            }
            return CmdGpuPower(argc, argv);
        }

        int CmdGpuThermalDispatch(int argc, char **argv)
        {
            if (argc > 0 && argv && (std::strcmp(argv[0], "help") == 0))
            {
                PrintUsageGroup("gpu");
                return 0;
            }
            static const SubcommandEntry kSubcommands[] = {
                {"level", CmdGpuThermalLevel},
                {"slowdown", CmdGpuThermalSlowdown},
                {"slowdown-set", CmdGpuThermalSlowdownSet},
                {"sim", CmdGpuThermalSimGet},
                {"sim-set", CmdGpuThermalSimSet},
            };
            const SubcommandEntry *entry = FindSubcommand(kSubcommands, sizeof(kSubcommands) / sizeof(kSubcommands[0]), argc > 0 ? argv[0] : nullptr);
            if (entry)
            {
                return entry->handler(argc - 1, argv + 1);
            }
            return CmdGpuThermal(argc, argv);
        }

        int CmdGpuFanDispatch(int argc, char **argv)
        {
            static const SubcommandEntry kSubcommands[] = {
                {"set", CmdGpuFanSet},
                {"restore", CmdGpuFanRestore},
            };

            return DispatchSubcommand("fan", argc, argv, kSubcommands, sizeof(kSubcommands) / sizeof(kSubcommands[0]), PrintGpuUsage);
        }

        int CmdGpuVoltageDispatch(int argc, char **argv)
        {
            if (argc > 0 && argv && (std::strcmp(argv[0], "help") == 0))
            {
                PrintUsageGroup("gpu");
                return 0;
            }
            static const SubcommandEntry kSubcommands[] = {
                {"control-set", CmdGpuVoltageControlSet},
            };
            const SubcommandEntry *entry = FindSubcommand(kSubcommands, sizeof(kSubcommands) / sizeof(kSubcommands[0]), argc > 0 ? argv[0] : nullptr);
            if (entry)
            {
                return entry->handler(argc - 1, argv + 1);
            }
            return CmdGpuVoltage(argc, argv);
        }
    }

    int CmdGpuPerfLimits(int argc, char **argv)
    {
        if (argc < 1)
        {
            PrintUsageGroup("gpu");
            return 1;
        }

        static const SubcommandEntry kSubcommands[] = {
            {"info", CmdGpuPerfLimitsInfo},
            {"status", CmdGpuPerfLimitsStatus},
            {"set", CmdGpuPerfLimitsSet},
        };

        return DispatchSubcommand("perf-limits", argc, argv, kSubcommands, sizeof(kSubcommands) / sizeof(kSubcommands[0]), PrintGpuUsage);
    }

    int CmdGpu(int argc, char **argv)
    {
        if (argc < 1)
        {
            PrintUsageGroup("gpu");
            return 1;
        }

        static const SubcommandEntry kSubcommands[] = {
            {"list", CmdGpuListAdapter},
            {"memory", CmdGpuMemory},
            {"clocks", CmdGpuClocks},
            {"clock", CmdGpuClock},
            {"utilization", CmdGpuUtilization},
            {"dynamic-pstates-set", CmdGpuDynamicPstatesSet},
            {"force-pstate", CmdGpuForcePstate},
            {"force-pstate-ex", CmdGpuForcePstateEx},
            {"pstate", CmdGpuPstate},
            {"pstates20", CmdGpuPstates20},
            {"pstates20-set", CmdGpuPstates20Set},
            {"pstates20-private", CmdGpuPstates20Private},
            {"pstates20-private-set", CmdGpuPstates20PrivateSet},
            {"bus", CmdGpuBus},
            {"vbios", CmdGpuVbios},
            {"cooler", CmdGpuCooler},
            {"cooler-policy", CmdGpuCoolerPolicy},
            {"bar", CmdGpuBar},
            {"ecc", CmdGpuEcc},
            {"board", CmdGpuBoard},
            {"pcie", CmdGpuPcie},
            {"power", CmdGpuPowerDispatch},
            {"gc6", CmdGpuGc6},
            {"deep-idle", CmdGpuDeepIdle},
            {"oc-scanner", CmdGpuOcScanner},
            {"vf", CmdGpuVf},
            {"vpstates", CmdGpuVpstates},
            {"vfe-var", CmdGpuVfeVar},
            {"vfe-equ", CmdGpuVfeEqu},
            {"perf-limits", CmdGpuPerfLimits},
            {"voltage", CmdGpuVoltageDispatch},
            {"thermal", CmdGpuThermalDispatch},
            {"fan", CmdGpuFanDispatch},
            {"client-fan", CmdGpuClientFan},
            {"client-illum", CmdGpuClientIllum},
        };
        return DispatchSubcommand("gpu", argc, argv, kSubcommands, sizeof(kSubcommands) / sizeof(kSubcommands[0]), PrintGpuUsage);
    }
}

