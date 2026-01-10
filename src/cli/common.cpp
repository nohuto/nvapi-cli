/*
 * Copyright (c) 2026 Noverse (Nohuto). All rights reserved.
 * Proprietary and confidential. Unauthorized copying or redistribution is strictly prohibited.
 */

#include "cli/common.h"

namespace nvcli
{
    const char *kToolName = "nvapi-cli";

    std::string ToLowerAscii(const char *value);

    std::string NvapiStatusString(NvAPI_Status status)
    {
        NvAPI_ShortString err = {0};
        if (NvAPI_GetErrorMessage(status, err) == NVAPI_OK)
        {
            return std::string(err);
        }
        return "NVAPI_ERROR";
    }

    void PrintNvapiError(const char *prefix, NvAPI_Status status)
    {
        std::printf("%s: %s (0x%08X)\n", prefix, NvapiStatusString(status).c_str(), status);
    }

    bool ParseUint(const char *text, NvU32 *out)
    {
        if (!text || !out)
        {
            return false;
        }
        char *end = NULL;
        unsigned long value = std::strtoul(text, &end, 0);
        if (end == text || *end != '\0')
        {
            return false;
        }
        *out = static_cast<NvU32>(value);
        return true;
    }

    bool ParseSrcDevicePair(const char *text, NvU32 *srcId, NvU32 *device)
    {
        if (!text || !srcId || !device)
        {
            return false;
        }
        const char *sep = std::strchr(text, ':');
        if (!sep)
        {
            return false;
        }
        std::string left(text, sep - text);
        std::string right(sep + 1);
        return ParseUint(left.c_str(), srcId) && ParseUint(right.c_str(), device);
    }

    bool GetDisplayHandleByIndex(NvU32 index, NvDisplayHandle *outHandle)
    {
        if (!outHandle)
        {
            return false;
        }
        NvDisplayHandle handle = NULL;
        NvU32 current = 0;
        while (NvAPI_EnumNvidiaDisplayHandle(current, &handle) == NVAPI_OK)
        {
            if (current == index)
            {
                *outHandle = handle;
                return true;
            }
            ++current;
        }
        return false;
    }

    const SubcommandEntry *FindSubcommand(const SubcommandEntry *entries, size_t count, const char *name)
    {
        if (!entries || count == 0 || !name)
        {
            return nullptr;
        }
        for (size_t i = 0; i < count; ++i)
        {
            if (std::strcmp(entries[i].name, name) == 0)
            {
                return &entries[i];
            }
        }
        return nullptr;
    }

    int DispatchSubcommand(const char *group, int argc, char **argv, const SubcommandEntry *entries, size_t count, void (*printUsage)())
    {
        if (argc < 1 || !argv || !argv[0])
        {
            if (printUsage)
            {
                printUsage();
            }
            else if (group)
            {
                std::printf("Missing %s subcommand.\n", group);
            }
            else
            {
                std::printf("Missing subcommand.\n");
            }
            return 1;
        }

        if (std::strcmp(argv[0], "help") == 0)
        {
            if (printUsage)
            {
                printUsage();
            }
            else
            {
                PrintUsage();
            }
            return 0;
        }

        const SubcommandEntry *entry = FindSubcommand(entries, count, argv[0]);
        if (!entry)
        {
            if (group)
            {
                std::printf("Unknown %s subcommand: %s\n", group, argv[0]);
            }
            else
            {
                std::printf("Unknown subcommand: %s\n", argv[0]);
            }
            if (printUsage)
            {
                printUsage();
            }
            return 1;
        }

        return entry->handler(argc - 1, argv + 1);
    }

    double KBToMiB(NvU32 kb)
    {
        return static_cast<double>(kb) / 1024.0;
    }

    double BytesToMiB(NvU64 bytes)
    {
        return static_cast<double>(bytes) / (1024.0 * 1024.0);
    }

    const char *BusTypeName(NV_GPU_BUS_TYPE type)
    {
        switch (type)
        {
        case NVAPI_GPU_BUS_TYPE_PCI:
            return "PCI";
        case NVAPI_GPU_BUS_TYPE_AGP:
            return "AGP";
        case NVAPI_GPU_BUS_TYPE_PCI_EXPRESS:
            return "PCIe";
        case NVAPI_GPU_BUS_TYPE_FPCI:
            return "FPCI";
        case NVAPI_GPU_BUS_TYPE_AXI:
            return "AXI";
        default:
            return "UNKNOWN";
        }
    }

    const char *RamTypeName(NV_GPU_RAM_TYPE type)
    {
        switch (type)
        {
        case NV_GPU_RAM_TYPE_SDRAM:
            return "SDRAM";
        case NV_GPU_RAM_TYPE_DDR1:
            return "DDR1";
        case NV_GPU_RAM_TYPE_DDR2:
            return "DDR2";
        case NV_GPU_RAM_TYPE_GDDR2:
            return "GDDR2";
        case NV_GPU_RAM_TYPE_GDDR3:
            return "GDDR3";
        case NV_GPU_RAM_TYPE_GDDR4:
            return "GDDR4";
        case NV_GPU_RAM_TYPE_DDR3:
            return "DDR3";
        case NV_GPU_RAM_TYPE_GDDR5:
            return "GDDR5";
        case NV_GPU_RAM_TYPE_LPDDR2:
            return "LPDDR2";
        case NV_GPU_RAM_TYPE_GDDR5X:
            return "GDDR5X";
        case NV_GPU_RAM_TYPE_HBM1:
            return "HBM1";
        case NV_GPU_RAM_TYPE_HBM2:
            return "HBM2";
        case NV_GPU_RAM_TYPE_SDDR4:
            return "SDDR4";
        case NV_GPU_RAM_TYPE_GDDR6:
            return "GDDR6";
        case NV_GPU_RAM_TYPE_GDDR6X:
            return "GDDR6X";
        default:
            return "UNKNOWN";
        }
    }

    const char *UtilizationDomainName(NvU32 domain)
    {
        switch (domain)
        {
        case NVAPI_GPU_UTILIZATION_DOMAIN_GPU:
            return "GPU";
        case NVAPI_GPU_UTILIZATION_DOMAIN_FB:
            return "FB";
        case NVAPI_GPU_UTILIZATION_DOMAIN_VID:
            return "VID";
        case NVAPI_GPU_UTILIZATION_DOMAIN_BUS:
            return "BUS";
        default:
            return "OTHER";
        }
    }

    const char *ClockDomainName(NvU32 domain)
    {
        switch (domain)
        {
        case NVAPI_GPU_PUBLIC_CLOCK_GRAPHICS:
            return "GRAPHICS";
        case NVAPI_GPU_PUBLIC_CLOCK_MEMORY:
            return "MEMORY";
        case NVAPI_GPU_PUBLIC_CLOCK_PROCESSOR:
            return "PROCESSOR";
        case NVAPI_GPU_PUBLIC_CLOCK_VIDEO:
            return "VIDEO";
        default:
            return "OTHER";
        }
    }

    const char *PstateClockTypeName(NV_GPU_PERF_PSTATE20_CLOCK_TYPE_ID type)
    {
        switch (type)
        {
        case NVAPI_GPU_PERF_PSTATE20_CLOCK_TYPE_SINGLE:
            return "SINGLE";
        case NVAPI_GPU_PERF_PSTATE20_CLOCK_TYPE_RANGE:
            return "RANGE";
        default:
            return "UNKNOWN";
        }
    }

    const char *ConnectorTypeName(NV_MONITOR_CONN_TYPE type)
    {
        switch (type)
        {
        case NV_MONITOR_CONN_TYPE_VGA:
            return "VGA";
        case NV_MONITOR_CONN_TYPE_COMPONENT:
            return "COMPONENT";
        case NV_MONITOR_CONN_TYPE_SVIDEO:
            return "SVIDEO";
        case NV_MONITOR_CONN_TYPE_HDMI:
            return "HDMI";
        case NV_MONITOR_CONN_TYPE_DVI:
            return "DVI";
        case NV_MONITOR_CONN_TYPE_LVDS:
            return "LVDS";
        case NV_MONITOR_CONN_TYPE_DP:
            return "DP";
        case NV_MONITOR_CONN_TYPE_COMPOSITE:
            return "COMPOSITE";
        case NV_MONITOR_CONN_TYPE_UNINITIALIZED:
            return "UNINITIALIZED";
        default:
            return "UNKNOWN";
        }
    }

    const char *PstateName(NV_GPU_PERF_PSTATE_ID pstate)
    {
        switch (pstate)
        {
        case NVAPI_GPU_PERF_PSTATE_P0:
            return "P0";
        case NVAPI_GPU_PERF_PSTATE_P1:
            return "P1";
        case NVAPI_GPU_PERF_PSTATE_P2:
            return "P2";
        case NVAPI_GPU_PERF_PSTATE_P3:
            return "P3";
        case NVAPI_GPU_PERF_PSTATE_P4:
            return "P4";
        case NVAPI_GPU_PERF_PSTATE_P5:
            return "P5";
        case NVAPI_GPU_PERF_PSTATE_P6:
            return "P6";
        case NVAPI_GPU_PERF_PSTATE_P7:
            return "P7";
        case NVAPI_GPU_PERF_PSTATE_P8:
            return "P8";
        case NVAPI_GPU_PERF_PSTATE_P9:
            return "P9";
        case NVAPI_GPU_PERF_PSTATE_P10:
            return "P10";
        case NVAPI_GPU_PERF_PSTATE_P11:
            return "P11";
        case NVAPI_GPU_PERF_PSTATE_P12:
            return "P12";
        case NVAPI_GPU_PERF_PSTATE_P13:
            return "P13";
        case NVAPI_GPU_PERF_PSTATE_P14:
            return "P14";
        case NVAPI_GPU_PERF_PSTATE_P15:
            return "P15";
        default:
            return "UNKNOWN";
        }
    }

    const char *CoolerControlName(NV_COOLER_CONTROL control)
    {
        switch (control)
        {
        case NVAPI_COOLER_CONTROL_TOGGLE:
            return "TOGGLE";
        case NVAPI_COOLER_CONTROL_VARIABLE:
            return "VARIABLE";
        default:
            return "NONE";
        }
    }

    const char *VoltageDomainName(NV_GPU_PERF_VOLTAGE_INFO_DOMAIN_ID domain)
    {
        switch (domain)
        {
        case NVAPI_GPU_PERF_VOLTAGE_INFO_DOMAIN_CORE:
            return "CORE";
        default:
            return "UNKNOWN";
        }
    }

    const char *PerfVoltageDomainName(NV_GPU_PERF_VOLTAGE_DOMAIN_ID domain)
    {
        switch (domain)
        {
        case NVAPI_GPU_PERF_VOLTAGE_DOMAIN_CORE:
            return "CORE";
        case NVAPI_GPU_PERF_VOLTAGE_DOMAIN_FB:
            return "FB";
        case NVAPI_GPU_PERF_VOLTAGE_DOMAIN_COLD_CORE:
            return "COLD_CORE";
        case NVAPI_GPU_PERF_VOLTAGE_DOMAIN_CORE_NOMINAL:
            return "CORE_NOMINAL";
        case NVAPI_GPU_PERF_VOLTAGE_DOMAIN_UNDEFINED:
            return "UNDEFINED";
        default:
            return "UNKNOWN";
        }
    }

    const char *VoltageEntryTypeName(NV_GPU_PSTATE20_VOLTAGE_ENTRY_TYPE type)
    {
        switch (type)
        {
        case NV_GPU_PSTATE20_VOLTAGE_ENTRY_TYPE_LOGICAL:
            return "LOGICAL";
        case NV_GPU_PSTATE20_VOLTAGE_ENTRY_TYPE_VDT:
            return "VDT";
        case NV_GPU_PSTATE20_VOLTAGE_ENTRY_TYPE_DELTA_ONLY:
            return "DELTA_ONLY";
        case NV_GPU_PSTATE30_VOLTAGE_ENTRY_TYPE_VFE:
            return "VFE";
        case NV_GPU_PSTATE30_VOLTAGE_ENTRY_TYPE_PSTATE:
            return "PSTATE";
        case NV_GPU_PSTATE30_VOLTAGE_ENTRY_TYPE_VPSTATE:
            return "VPSTATE";
        case NV_GPU_PSTATE30_VOLTAGE_ENTRY_TYPE_FREQUENCY:
            return "FREQUENCY";
        case NV_GPU_PSTATE20_VOLTAGE_ENTRY_TYPE_UNDEFINED:
            return "UNDEFINED";
        default:
            return "UNKNOWN";
        }
    }

    const char *ClockDomainIdName(NV_GPU_CLOCK_DOMAIN_ID domain)
    {
        switch (domain)
        {
        case NVAPI_GPU_CLOCK_DOMAIN_GPC:
            return "GPC";
        case NVAPI_GPU_CLOCK_DOMAIN_XBAR:
            return "XBAR";
        case NVAPI_GPU_CLOCK_DOMAIN_SYS:
            return "SYS";
        case NVAPI_GPU_CLOCK_DOMAIN_HUB:
            return "HUB";
        case NVAPI_GPU_CLOCK_DOMAIN_M:
            return "M";
        case NVAPI_GPU_CLOCK_DOMAIN_HOST:
            return "HOST";
        case NVAPI_GPU_CLOCK_DOMAIN_DISP:
            return "DISP";
        case NVAPI_GPU_CLOCK_DOMAIN_HOTCLK:
            return "HOTCLK";
        case NVAPI_GPU_CLOCK_DOMAIN_PCLK0:
            return "PCLK0";
        case NVAPI_GPU_CLOCK_DOMAIN_PCLK1:
            return "PCLK1";
        case NVAPI_GPU_CLOCK_DOMAIN_BYPCLK:
            return "BYPCLK";
        case NVAPI_GPU_CLOCK_DOMAIN_XCLK:
            return "XCLK";
        case NVAPI_GPU_CLOCK_DOMAIN_VPV:
            return "VPV";
        case NVAPI_GPU_CLOCK_DOMAIN_VPS:
            return "VPS";
        case NVAPI_GPU_CLOCK_DOMAIN_GPUCACHECLK:
            return "GPUCACHECLK";
        case NVAPI_GPU_CLOCK_DOMAIN_UNDEFINED:
            return "UNDEFINED";
        default:
            return "UNKNOWN";
        }
    }

    const char *ClockPstateUsageName(NV_GPU_CLOCK_INFO_DOMAIN_PSTATE_USAGE usage)
    {
        switch (usage)
        {
        case NV_GPU_CLOCK_PSTATE_USAGE_FIXED:
            return "FIXED";
        case NV_GPU_CLOCK_PSTATE_USAGE_PSTATE:
            return "PSTATE";
        case NV_GPU_CLOCK_PSTATE_USAGE_DECOUPLED:
            return "DECOUPLED";
        case NV_GPU_CLOCK_PSTATE_USAGE_RATIO:
            return "RATIO";
        default:
            return "UNKNOWN";
        }
    }

    const char *PerfLimitInputTypeName(NV_GPU_PERF_LIMIT_STATUS_INPUT_TYPE type)
    {
        switch (type)
        {
        case NVAPI_PERF_LIMIT_STATUS_INPUT_TYPE_DISABLED:
            return "DISABLED";
        case NVAPI_PERF_LIMIT_STATUS_INPUT_TYPE_PSTATE:
            return "PSTATE";
        case NVAPI_PERF_LIMIT_STATUS_INPUT_TYPE_FREQ:
            return "FREQ";
        case NVAPI_PERF_LIMIT_STATUS_INPUT_TYPE_VPSTATE:
            return "VPSTATE";
        case NVAPI_PERF_LIMIT_STATUS_INPUT_TYPE_VOLTAGE:
            return "VOLTAGE";
        case NVAPI_PERF_LIMIT_STATUS_INPUT_TYPE_VOLTAGE_EX:
            return "VOLTAGE_EX";
        case NVAPI_PERF_LIMIT_STATUS_INPUT_TYPE_VOLTAGE_3X:
            return "VOLTAGE_3X";
        default:
            return "UNKNOWN";
        }
    }

    const char *PerfLimitPstatePointName(NV_GPU_PERF_LIMIT_INPUT_DATA_PSTATE_POINT point)
    {
        switch (point)
        {
        case NV_GPU_PERF_LIMIT_INPUT_DATA_PSTATE_POINT_NOM:
            return "NOM";
        case NV_GPU_PERF_LIMIT_INPUT_DATA_PSTATE_POINT_MIN:
            return "MIN";
        case NV_GPU_PERF_LIMIT_INPUT_DATA_PSTATE_POINT_MAX:
            return "MAX";
        case NV_GPU_PERF_LIMIT_INPUT_DATA_PSTATE_POINT_MID:
            return "MID";
        default:
            return "UNKNOWN";
        }
    }

    const char *VpstateTypeName(NV_GPU_PERF_VPSTATE_TYPE type)
    {
        switch (type)
        {
        case NV_GPU_PERF_VPSTATE_TYPE_2X:
            return "2X";
        case NV_GPU_PERF_VPSTATE_TYPE_3X:
            return "3X";
        default:
            return "UNKNOWN";
        }
    }

    const char *VfeVarTypeName(NV_GPU_PERF_VFE_VAR_TYPE type)
    {
        switch (type)
        {
        case NV_GPU_PERF_VFE_VAR_TYPE_DERIVED:
            return "DERIVED";
        case NV_GPU_PERF_VFE_VAR_TYPE_DERIVED_PRODUCT:
            return "DERIVED_PRODUCT";
        case NV_GPU_PERF_VFE_VAR_TYPE_DERIVED_SUM:
            return "DERIVED_SUM";
        case NV_GPU_PERF_VFE_VAR_TYPE_SINGLE:
            return "SINGLE";
        case NV_GPU_PERF_VFE_VAR_TYPE_SINGLE_FREQUENCY:
            return "SINGLE_FREQUENCY";
        case NV_GPU_PERF_VFE_VAR_TYPE_SINGLE_SENSED:
            return "SINGLE_SENSED";
        case NV_GPU_PERF_VFE_VAR_TYPE_SINGLE_SENSED_FUSE:
            return "SINGLE_SENSED_FUSE";
        case NV_GPU_PERF_VFE_VAR_TYPE_SINGLE_SENSED_TEMP:
            return "SINGLE_SENSED_TEMP";
        case NV_GPU_PERF_VFE_VAR_TYPE_SINGLE_VOLTAGE:
            return "SINGLE_VOLTAGE";
        case NV_GPU_PERF_VFE_VAR_TYPE_SINGLE_SENSED_FUSE_BASE:
            return "SINGLE_SENSED_FUSE_BASE";
        case NV_GPU_PERF_VFE_VAR_TYPE_SINGLE_SENSED_FUSE_20:
            return "SINGLE_SENSED_FUSE_20";
        default:
            return "UNKNOWN";
        }
    }

    const char *VfeVarOverrideTypeName(NvU8 type)
    {
        switch (type)
        {
        case NV_GPU_PERF_VFE_VAR_SINGLE_OVERRIDE_TYPE_NONE:
            return "NONE";
        case NV_GPU_PERF_VFE_VAR_SINGLE_OVERRIDE_TYPE_VALUE:
            return "VALUE";
        case NV_GPU_PERF_VFE_VAR_SINGLE_OVERRIDE_TYPE_OFFSET:
            return "OFFSET";
        case NV_GPU_PERF_VFE_VAR_SINGLE_OVERRIDE_TYPE_SCALE:
            return "SCALE";
        default:
            return "UNKNOWN";
        }
    }

    const char *VfeEquTypeName(NV_GPU_PERF_VFE_EQU_TYPE type)
    {
        switch (type)
        {
        case NV_GPU_PERF_VFE_EQU_TYPE_COMPARE:
            return "COMPARE";
        case NV_GPU_PERF_VFE_EQU_TYPE_MINMAX:
            return "MINMAX";
        case NV_GPU_PERF_VFE_EQU_TYPE_QUADRATIC:
            return "QUADRATIC";
        case NV_GPU_PERF_VFE_EQU_TYPE_EQUATION_SCALAR:
            return "EQUATION_SCALAR";
        default:
            return "UNKNOWN";
        }
    }

    const char *VfeEquOutputTypeName(NvU8 type)
    {
        switch (type)
        {
        case NV_GPU_PERF_VFE_EQU_OUTPUT_TYPE_UNITLESS:
            return "UNITLESS";
        case NV_GPU_PERF_VFE_EQU_OUTPUT_TYPE_FREQ_MHZ:
            return "FREQ_MHZ";
        case NV_GPU_PERF_VFE_EQU_OUTPUT_TYPE_VOLT_UV:
            return "VOLT_UV";
        case NV_GPU_PERF_VFE_EQU_OUTPUT_TYPE_VF_GAIN:
            return "VF_GAIN";
        case NV_GPU_PERF_VFE_EQU_OUTPUT_TYPE_VOLT_DELTA_UV:
            return "VOLT_DELTA_UV";
        default:
            return "UNKNOWN";
        }
    }

    const char *VfeEquCompareFuncName(NvU8 func)
    {
        switch (func)
        {
        case NV_GPU_PERF_VFE_EQU_COMPARE_FUNCTION_EQUAL:
            return "EQ";
        case NV_GPU_PERF_VFE_EQU_COMPARE_FUNCTION_GREATER_EQ:
            return "GTE";
        case NV_GPU_PERF_VFE_EQU_COMPARE_FUNCTION_GREATER:
            return "GT";
        default:
            return "UNKNOWN";
        }
    }

    bool ParseGpuIndexArgs(int argc, char **argv, NvU32 *indexOut, bool *hasIndex)
    {
        if (!indexOut || !hasIndex)
        {
            return false;
        }
        *hasIndex = false;
        for (int i = 0; i < argc; ++i)
        {
            if (std::strcmp(argv[i], "--index") == 0)
            {
                if (i + 1 >= argc)
                {
                    std::printf("Missing value for --index\n");
                    return false;
                }
                if (!ParseUint(argv[i + 1], indexOut))
                {
                    std::printf("Invalid GPU index: %s\n", argv[i + 1]);
                    return false;
                }
                *hasIndex = true;
                ++i;
                continue;
            }
            std::printf("Unknown option: %s\n", argv[i]);
            return false;
        }
        return true;
    }

    bool ParseDisplayIdArg(int argc, char **argv, NvU32 *displayId)
    {
        if (!displayId)
        {
            return false;
        }
        bool found = false;
        for (int i = 0; i < argc; ++i)
        {
            if (std::strcmp(argv[i], "--id") == 0)
            {
                if (i + 1 >= argc)
                {
                    std::printf("Missing value for --id\n");
                    return false;
                }
                if (!ParseUint(argv[i + 1], displayId))
                {
                    std::printf("Invalid display id: %s\n", argv[i + 1]);
                    return false;
                }
                found = true;
                ++i;
                continue;
            }
            std::printf("Unknown option: %s\n", argv[i]);
            return false;
        }
        if (!found)
        {
            std::printf("Missing required --id\n");
            return false;
        }
        return true;
    }

    bool ParseDisplayIdsArgs(int argc, char **argv, NvU32 *gpuIndex, bool *hasIndex, bool *all, NvU32 *flags)
    {
        if (!gpuIndex || !hasIndex || !all || !flags)
        {
            return false;
        }
        *hasIndex = false;
        *all = false;
        *flags = 0;
        for (int i = 0; i < argc; ++i)
        {
            if (std::strcmp(argv[i], "--index") == 0)
            {
                if (i + 1 >= argc)
                {
                    std::printf("Missing value for --index\n");
                    return false;
                }
                if (!ParseUint(argv[i + 1], gpuIndex))
                {
                    std::printf("Invalid GPU index: %s\n", argv[i + 1]);
                    return false;
                }
                *hasIndex = true;
                ++i;
                continue;
            }
            if (std::strcmp(argv[i], "--all") == 0)
            {
                *all = true;
                continue;
            }
            if (std::strcmp(argv[i], "--flags") == 0)
            {
                if (i + 1 >= argc)
                {
                    std::printf("Missing value for --flags\n");
                    return false;
                }
                if (!ParseUint(argv[i + 1], flags))
                {
                    std::printf("Invalid flags: %s\n", argv[i + 1]);
                    return false;
                }
                ++i;
                continue;
            }
            std::printf("Unknown option: %s\n", argv[i]);
            return false;
        }
        return true;
    }

    bool ParseEdidFlagArg(int argc, char **argv, NV_EDID_FLAG *flag)
    {
        if (!flag)
        {
            return false;
        }
        *flag = NV_EDID_FLAG_DEFAULT;
        for (int i = 0; i < argc; ++i)
        {
            if (std::strcmp(argv[i], "--flag") == 0)
            {
                if (i + 1 >= argc)
                {
                    std::printf("Missing value for --flag\n");
                    return false;
                }
                const char *value = argv[i + 1];
                if (std::strcmp(value, "default") == 0)
                {
                    *flag = NV_EDID_FLAG_DEFAULT;
                }
                else if (std::strcmp(value, "raw") == 0)
                {
                    *flag = NV_EDID_FLAG_RAW;
                }
                else if (std::strcmp(value, "cooked") == 0)
                {
                    *flag = NV_EDID_FLAG_COOKED;
                }
                else if (std::strcmp(value, "forced") == 0)
                {
                    *flag = NV_EDID_FLAG_FORCED;
                }
                else if (std::strcmp(value, "inf") == 0)
                {
                    *flag = NV_EDID_FLAG_INF;
                }
                else if (std::strcmp(value, "hw") == 0)
                {
                    *flag = NV_EDID_FLAG_HW;
                }
                else if (std::strcmp(value, "tiles") == 0)
                {
                    *flag = NV_EDID_FLAG_TILES;
                }
                else
                {
                    NvU32 numeric = 0;
                    if (!ParseUint(value, &numeric))
                    {
                        std::printf("Invalid flag: %s\n", value);
                        return false;
                    }
                    *flag = static_cast<NV_EDID_FLAG>(numeric);
                }
                ++i;
                continue;
            }
        }
        return true;
    }

    bool ParseCoolerPolicy(const char *value, NV_COOLER_POLICY *policy)
    {
        if (!value || !policy)
        {
            return false;
        }
        if (std::strcmp(value, "manual") == 0)
        {
            *policy = NVAPI_COOLER_POLICY_MANUAL;
        }
        else if (std::strcmp(value, "perf") == 0)
        {
            *policy = NVAPI_COOLER_POLICY_PERF;
        }
        else if (std::strcmp(value, "temp-discrete") == 0)
        {
            *policy = NVAPI_COOLER_POLICY_TEMPERATURE_DISCRETE;
        }
        else if (std::strcmp(value, "temp-cont") == 0)
        {
            *policy = NVAPI_COOLER_POLICY_TEMPERATURE_CONTINUOUS;
        }
        else if (std::strcmp(value, "temp-cont-sw") == 0)
        {
            *policy = NVAPI_COOLER_POLICY_TEMPERATURE_CONTINUOUS_SW;
        }
        else if (std::strcmp(value, "default") == 0)
        {
            *policy = NVAPI_COOLER_POLICY_DEFAULT;
        }
        else
        {
            NvU32 numeric = 0;
            if (!ParseUint(value, &numeric))
            {
                return false;
            }
            *policy = static_cast<NV_COOLER_POLICY>(numeric);
        }
        return true;
    }

    bool ParsePstateId(const char *value, NV_GPU_PERF_PSTATE_ID *out)
    {
        if (!value || !out)
        {
            return false;
        }
        if ((value[0] == 'P' || value[0] == 'p') && value[1] != '\0')
        {
            NvU32 numeric = 0;
            if (!ParseUint(value + 1, &numeric))
            {
                return false;
            }
            if (numeric > NVAPI_GPU_PERF_PSTATE_P15)
            {
                return false;
            }
            *out = static_cast<NV_GPU_PERF_PSTATE_ID>(numeric);
            return true;
        }
        NvU32 numeric = 0;
        if (!ParseUint(value, &numeric))
        {
            return false;
        }
        if (numeric > NVAPI_GPU_PERF_PSTATE_P15)
        {
            return false;
        }
        *out = static_cast<NV_GPU_PERF_PSTATE_ID>(numeric);
        return true;
    }

    bool ParsePublicClockId(const char *value, NV_GPU_PUBLIC_CLOCK_ID *out)
    {
        if (!value || !out)
        {
            return false;
        }
        std::string lowered = ToLowerAscii(value);
        if (lowered == "graphics")
        {
            *out = NVAPI_GPU_PUBLIC_CLOCK_GRAPHICS;
            return true;
        }
        if (lowered == "memory")
        {
            *out = NVAPI_GPU_PUBLIC_CLOCK_MEMORY;
            return true;
        }
        if (lowered == "processor")
        {
            *out = NVAPI_GPU_PUBLIC_CLOCK_PROCESSOR;
            return true;
        }
        if (lowered == "video")
        {
            *out = NVAPI_GPU_PUBLIC_CLOCK_VIDEO;
            return true;
        }
        NvU32 numeric = 0;
        if (!ParseUint(value, &numeric))
        {
            return false;
        }
        *out = static_cast<NV_GPU_PUBLIC_CLOCK_ID>(numeric);
        return true;
    }

    bool ParseVoltageDomainId(const char *value, NV_GPU_PERF_VOLTAGE_INFO_DOMAIN_ID *out)
    {
        if (!value || !out)
        {
            return false;
        }
        std::string lowered = ToLowerAscii(value);
        if (lowered == "core")
        {
            *out = NVAPI_GPU_PERF_VOLTAGE_INFO_DOMAIN_CORE;
            return true;
        }
        NvU32 numeric = 0;
        if (!ParseUint(value, &numeric))
        {
            return false;
        }
        *out = static_cast<NV_GPU_PERF_VOLTAGE_INFO_DOMAIN_ID>(numeric);
        return true;
    }

    bool ParsePerfVoltageDomainId(const char *value, NV_GPU_PERF_VOLTAGE_DOMAIN_ID *out)
    {
        if (!value || !out)
        {
            return false;
        }
        std::string lowered = ToLowerAscii(value);
        if (lowered == "core")
        {
            *out = NVAPI_GPU_PERF_VOLTAGE_DOMAIN_CORE;
            return true;
        }
        if (lowered == "fb")
        {
            *out = NVAPI_GPU_PERF_VOLTAGE_DOMAIN_FB;
            return true;
        }
        if (lowered == "cold-core")
        {
            *out = NVAPI_GPU_PERF_VOLTAGE_DOMAIN_COLD_CORE;
            return true;
        }
        if (lowered == "core-nominal")
        {
            *out = NVAPI_GPU_PERF_VOLTAGE_DOMAIN_CORE_NOMINAL;
            return true;
        }
        NvU32 numeric = 0;
        if (!ParseUint(value, &numeric))
        {
            return false;
        }
        *out = static_cast<NV_GPU_PERF_VOLTAGE_DOMAIN_ID>(numeric);
        return true;
    }

    bool ParsePerfLimitInputType(const char *value, NV_GPU_PERF_LIMIT_STATUS_INPUT_TYPE *out)
    {
        if (!value || !out)
        {
            return false;
        }
        std::string lowered = ToLowerAscii(value);
        if (lowered == "disabled")
        {
            *out = NVAPI_PERF_LIMIT_STATUS_INPUT_TYPE_DISABLED;
            return true;
        }
        if (lowered == "pstate")
        {
            *out = NVAPI_PERF_LIMIT_STATUS_INPUT_TYPE_PSTATE;
            return true;
        }
        if (lowered == "freq")
        {
            *out = NVAPI_PERF_LIMIT_STATUS_INPUT_TYPE_FREQ;
            return true;
        }
        if (lowered == "vpstate")
        {
            *out = NVAPI_PERF_LIMIT_STATUS_INPUT_TYPE_VPSTATE;
            return true;
        }
        NvU32 numeric = 0;
        if (!ParseUint(value, &numeric))
        {
            return false;
        }
        *out = static_cast<NV_GPU_PERF_LIMIT_STATUS_INPUT_TYPE>(numeric);
        return true;
    }

    bool ParsePerfLimitPstatePoint(const char *value, NV_GPU_PERF_LIMIT_INPUT_DATA_PSTATE_POINT *out)
    {
        if (!value || !out)
        {
            return false;
        }
        std::string lowered = ToLowerAscii(value);
        if (lowered == "nominal")
        {
            *out = NV_GPU_PERF_LIMIT_INPUT_DATA_PSTATE_POINT_NOM;
            return true;
        }
        if (lowered == "min")
        {
            *out = NV_GPU_PERF_LIMIT_INPUT_DATA_PSTATE_POINT_MIN;
            return true;
        }
        if (lowered == "max")
        {
            *out = NV_GPU_PERF_LIMIT_INPUT_DATA_PSTATE_POINT_MAX;
            return true;
        }
        if (lowered == "mid")
        {
            *out = NV_GPU_PERF_LIMIT_INPUT_DATA_PSTATE_POINT_MID;
            return true;
        }
        NvU32 numeric = 0;
        if (!ParseUint(value, &numeric))
        {
            return false;
        }
        *out = static_cast<NV_GPU_PERF_LIMIT_INPUT_DATA_PSTATE_POINT>(numeric);
        return true;
    }

    std::string ToLowerAscii(const char *value)
    {
        std::string out = value ? value : "";
        std::transform(out.begin(), out.end(), out.begin(),
            [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
        return out;
    }

    bool ParseBoolValue(const char *value, bool *out)
    {
        if (!value || !out)
        {
            return false;
        }
        std::string lowered = ToLowerAscii(value);
        if (lowered == "1")
        {
            *out = true;
            return true;
        }
        if (lowered == "0")
        {
            *out = false;
            return true;
        }
        return false;
    }

    bool Utf8ToNvUnicode(const char *input, NvAPI_UnicodeString out)
    {
        if (!input || !out)
        {
            return false;
        }
        int needed = MultiByteToWideChar(CP_UTF8, 0, input, -1, NULL, 0);
        if (needed <= 0 || needed > static_cast<int>(NVAPI_UNICODE_STRING_MAX))
        {
            return false;
        }
        std::vector<wchar_t> buffer(static_cast<size_t>(needed));
        if (!MultiByteToWideChar(CP_UTF8, 0, input, -1, buffer.data(), needed))
        {
            return false;
        }
        std::memset(out, 0, sizeof(NvAPI_UnicodeString));
        std::memcpy(out, buffer.data(), static_cast<size_t>(needed) * sizeof(wchar_t));
        return true;
    }

    std::string NvUnicodeToUtf8(const NvAPI_UnicodeString value)
    {
        if (!value)
        {
            return std::string();
        }
        const wchar_t *wide = reinterpret_cast<const wchar_t *>(value);
        size_t length = 0;
        while (length < NVAPI_UNICODE_STRING_MAX && wide[length] != L'\0')
        {
            ++length;
        }
        int needed = WideCharToMultiByte(CP_UTF8, 0, wide, static_cast<int>(length), NULL, 0, NULL, NULL);
        if (needed <= 0)
        {
            return std::string();
        }
        std::string out(static_cast<size_t>(needed), '\0');
        WideCharToMultiByte(CP_UTF8, 0, wide, static_cast<int>(length), &out[0], needed, NULL, NULL);
        return out;
    }

    std::string GuidToString(const NvGUID &guid)
    {
        char buffer[64] = {};
        std::snprintf(buffer, sizeof(buffer),
            "{%08X-%04X-%04X-%02X%02X-%02X%02X%02X%02X%02X%02X}",
            guid.data1,
            guid.data2,
            guid.data3,
            guid.data4[0],
            guid.data4[1],
            guid.data4[2],
            guid.data4[3],
            guid.data4[4],
            guid.data4[5],
            guid.data4[6],
            guid.data4[7]);
        return std::string(buffer);
    }

    const char *TargetViewModeName(NV_TARGET_VIEW_MODE mode)
    {
        switch (mode)
        {
        case NV_VIEW_MODE_STANDARD:
            return "STANDARD";
        case NV_VIEW_MODE_CLONE:
            return "CLONE";
        case NV_VIEW_MODE_HSPAN:
            return "HSPAN";
        case NV_VIEW_MODE_VSPAN:
            return "VSPAN";
        case NV_VIEW_MODE_DUALVIEW:
            return "DUALVIEW";
        case NV_VIEW_MODE_MULTIVIEW:
            return "MULTIVIEW";
        default:
            return "UNKNOWN";
        }
    }

    const char *MosaicTopoTypeName(NV_MOSAIC_TOPO_TYPE type)
    {
        switch (type)
        {
        case NV_MOSAIC_TOPO_TYPE_ALL:
            return "ALL";
        case NV_MOSAIC_TOPO_TYPE_BASIC:
            return "BASIC";
        case NV_MOSAIC_TOPO_TYPE_PASSIVE_STEREO:
            return "PASSIVE_STEREO";
        case NV_MOSAIC_TOPO_TYPE_SCALED_CLONE:
            return "SCALED_CLONE";
        case NV_MOSAIC_TOPO_TYPE_PASSIVE_STEREO_SCALED_CLONE:
            return "PASSIVE_STEREO_SCALED_CLONE";
        default:
            return "UNKNOWN";
        }
    }

    const char *MosaicTopoName(NV_MOSAIC_TOPO topo)
    {
        switch (topo)
        {
        case NV_MOSAIC_TOPO_NONE:
            return "NONE";
        case NV_MOSAIC_TOPO_1x2_BASIC:
            return "1x2_BASIC";
        case NV_MOSAIC_TOPO_2x1_BASIC:
            return "2x1_BASIC";
        case NV_MOSAIC_TOPO_1x3_BASIC:
            return "1x3_BASIC";
        case NV_MOSAIC_TOPO_3x1_BASIC:
            return "3x1_BASIC";
        case NV_MOSAIC_TOPO_1x4_BASIC:
            return "1x4_BASIC";
        case NV_MOSAIC_TOPO_4x1_BASIC:
            return "4x1_BASIC";
        case NV_MOSAIC_TOPO_2x2_BASIC:
            return "2x2_BASIC";
        case NV_MOSAIC_TOPO_2x3_BASIC:
            return "2x3_BASIC";
        case NV_MOSAIC_TOPO_2x4_BASIC:
            return "2x4_BASIC";
        case NV_MOSAIC_TOPO_3x2_BASIC:
            return "3x2_BASIC";
        case NV_MOSAIC_TOPO_4x2_BASIC:
            return "4x2_BASIC";
        case NV_MOSAIC_TOPO_1x5_BASIC:
            return "1x5_BASIC";
        case NV_MOSAIC_TOPO_1x6_BASIC:
            return "1x6_BASIC";
        case NV_MOSAIC_TOPO_7x1_BASIC:
            return "7x1_BASIC";
        case NV_MOSAIC_TOPO_1x2_PASSIVE_STEREO:
            return "1x2_PASSIVE_STEREO";
        case NV_MOSAIC_TOPO_2x1_PASSIVE_STEREO:
            return "2x1_PASSIVE_STEREO";
        case NV_MOSAIC_TOPO_1x3_PASSIVE_STEREO:
            return "1x3_PASSIVE_STEREO";
        case NV_MOSAIC_TOPO_3x1_PASSIVE_STEREO:
            return "3x1_PASSIVE_STEREO";
        case NV_MOSAIC_TOPO_1x4_PASSIVE_STEREO:
            return "1x4_PASSIVE_STEREO";
        case NV_MOSAIC_TOPO_4x1_PASSIVE_STEREO:
            return "4x1_PASSIVE_STEREO";
        case NV_MOSAIC_TOPO_2x2_PASSIVE_STEREO:
            return "2x2_PASSIVE_STEREO";
        default:
            return "UNKNOWN";
        }
    }

    const char *GsyncDisplaySyncStateName(NVAPI_GSYNC_DISPLAY_SYNC_STATE state)
    {
        switch (state)
        {
        case NVAPI_GSYNC_DISPLAY_SYNC_STATE_UNSYNCED:
            return "UNSYNCED";
        case NVAPI_GSYNC_DISPLAY_SYNC_STATE_SLAVE:
            return "SLAVE";
        case NVAPI_GSYNC_DISPLAY_SYNC_STATE_MASTER:
            return "MASTER";
        default:
            return "UNKNOWN";
        }
    }

    const char *DpNodeTypeName(NV_DP_NODE_TYPE type)
    {
        switch (type)
        {
        case NV_DP_NODE_TYPE_DP:
            return "DP";
        case NV_DP_NODE_TYPE_HDMI:
            return "HDMI";
        case NV_DP_NODE_TYPE_DVI:
            return "DVI";
        case NV_DP_NODE_TYPE_VGA:
            return "VGA";
        case NV_DP_NODE_TYPE_UNKNOWN:
        default:
            return "UNKNOWN";
        }
    }

    bool CollectGpus(bool hasIndex, NvU32 index, std::vector<NvPhysicalGpuHandle> &handles, std::vector<NvU32> &indices)
    {
        NvPhysicalGpuHandle gpus[NVAPI_MAX_PHYSICAL_GPUS] = {};
        NvU32 gpuCount = 0;
        NvAPI_Status status = NvAPI_EnumPhysicalGPUs(gpus, &gpuCount);
        if (status != NVAPI_OK)
        {
            PrintNvapiError("NvAPI_EnumPhysicalGPUs failed", status);
            return false;
        }

        if (gpuCount == 0)
        {
            std::printf("No NVIDIA GPUs found.\n");
            return false;
        }

        if (hasIndex)
        {
            if (index >= gpuCount)
            {
                std::printf("GPU index %u out of range (0-%u).\n", index, gpuCount - 1);
                return false;
            }
            handles.push_back(gpus[index]);
            indices.push_back(index);
            return true;
        }

        for (NvU32 i = 0; i < gpuCount; ++i)
        {
            handles.push_back(gpus[i]);
            indices.push_back(i);
        }
        return true;
    }

    bool MaskE32Has(const NV_GPU_BOARDOBJGRP_MASK_E32 &mask, NvU32 index)
    {
        if (index >= NV_GPU_BOARDOBJGRP_E32_MAX_OBJECTS)
        {
            return false;
        }
        const NvU32 word = index / 32;
        const NvU32 bit = index % 32;
        const NvU32 *data = mask.super.pData;
        return (data[word] & (1u << bit)) != 0;
    }

    bool MaskE255Has(const NV_GPU_BOARDOBJGRP_MASK_E255 &mask, NvU32 index)
    {
        if (index >= NV_GPU_BOARDOBJGRP_E255_MAX_OBJECTS)
        {
            return false;
        }
        const NvU32 word = index / 32;
        const NvU32 bit = index % 32;
        const NvU32 *data = mask.super.pData;
        return (data[word] & (1u << bit)) != 0;
    }

    namespace
    {
        void PrintUsageSummary()
        {
            std::printf("Usage:\n");
            std::printf("  %s help [group]\n", kToolName);
            std::printf("  %s info\n", kToolName);
            std::printf("  %s <group> <command> [options]\n", kToolName);
            std::printf("    groups: gpu display mosaic sli gsync drs video hdmi dp pcf sys d3d ogl vr stereo\n");
            std::printf("\n");
            std::printf("Use \"%s help <group>\" or \"%s <group> help\" for details.\n", kToolName, kToolName);
            std::printf("Use \"%s help all\" for the full list.\n", kToolName);
            std::printf("\n");
        }

        void PrintUsageInfo()
        {
            std::printf("Info commands:\n");
            std::printf("  %s info\n", kToolName);
            std::printf("\n");
        }

        void PrintUsageGpu()
        {
            std::printf("GPU commands:\n");
            std::printf("  %s gpu list\n", kToolName);
            std::printf("  %s gpu memory [--index N]\n", kToolName);
            std::printf("  %s gpu clocks [--index N]\n", kToolName);
            std::printf("  %s gpu utilization [--index N]\n", kToolName);
            std::printf("  %s gpu dynamic-pstates-set [--index N] --enable 0|1\n", kToolName);
            std::printf("  %s gpu force-pstate [--index N] --pstate P0|auto [--fallback error|higher|lower]\n", kToolName);
            std::printf("  %s gpu force-pstate-ex [--index N] --pstate P0|auto [--fallback error|higher|lower] [--async 0|1]\n", kToolName);
            std::printf("  %s gpu pstate [--index N]\n", kToolName);
            std::printf("  %s gpu pstates20 [--index N]\n", kToolName);
            std::printf("  %s gpu pstates20-set [--index N] --pstate P0 (--clock graphics|memory|processor|video --delta-khz N | --voltage core --delta-uv N)\n", kToolName);
            std::printf("  %s gpu pstates20-private [--index N]\n", kToolName);
            std::printf("  %s gpu pstates20-private-set [--index N] --pstate P0 (--clock-id ID --delta-khz N | --voltage-domain core|fb|cold-core|core-nominal|ID (--delta-uv N | --target-uv N))\n", kToolName);
            std::printf("  %s gpu bus [--index N]\n", kToolName);
            std::printf("  %s gpu vbios [--index N]\n", kToolName);
            std::printf("  %s gpu cooler [--index N]\n", kToolName);
            std::printf("  %s gpu cooler-policy get [--index N] --cooler N [--policy manual|perf|temp-discrete|temp-cont|temp-cont-sw|default]\n", kToolName);
            std::printf("  %s gpu cooler-policy set [--index N] --cooler N --level-id N --level PCT [--policy manual|perf|temp-discrete|temp-cont|temp-cont-sw|default]\n", kToolName);
            std::printf("  %s gpu cooler-policy restore [--index N] [--cooler N] [--policy manual|perf|temp-discrete|temp-cont|temp-cont-sw|default]\n", kToolName);
            std::printf("  %s gpu bar [--index N]\n", kToolName);
            std::printf("  %s gpu ecc status [--index N]\n", kToolName);
            std::printf("  %s gpu ecc errors [--index N] [--raw]\n", kToolName);
            std::printf("  %s gpu ecc config [--index N]\n", kToolName);
            std::printf("  %s gpu ecc set [--index N] --enable 0|1 [--immediate 0|1] [--clear 0|1]\n", kToolName);
            std::printf("  %s gpu ecc reset [--index N] --current 0|1 --aggregate 0|1\n", kToolName);
            std::printf("  %s gpu board mfg [--index N]\n", kToolName);
            std::printf("  %s gpu pcie info [--index N]\n", kToolName);
            std::printf("  %s gpu pcie switch-errors [--index N]\n", kToolName);
            std::printf("  %s gpu pcie errors [--index N]\n", kToolName);
            std::printf("  %s gpu pcie aer [--index N]\n", kToolName);
            std::printf("  %s gpu pcie aspm-set [--index N] --enable 0|1\n", kToolName);
            std::printf("  %s gpu pcie width-set [--index N] --width N\n", kToolName);
            std::printf("  %s gpu pcie speed-set [--index N] --speed N\n", kToolName);
            std::printf("  %s gpu power [--index N]\n", kToolName);
            std::printf("  %s gpu gc6 control [--index N] --op clear-stats|enable-stats|disable-stats|supported|enabled\n", kToolName);
            std::printf("  %s gpu gc6 force-exit [--index N]\n", kToolName);
            std::printf("  %s gpu deep-idle state [--index N]\n", kToolName);
            std::printf("  %s gpu deep-idle set [--index N] --state enabled|disabled\n", kToolName);
            std::printf("  %s gpu deep-idle stats-mode [--index N] --mode nh|ve|ssc|fo [--reset 0|1]\n", kToolName);
            std::printf("  %s gpu deep-idle stats [--index N]\n", kToolName);
            std::printf("  %s gpu oc-scanner start [--index N]\n", kToolName);
            std::printf("  %s gpu oc-scanner stop [--index N]\n", kToolName);
            std::printf("  %s gpu oc-scanner revert [--index N]\n", kToolName);
            std::printf("  %s gpu power limit [--index N]\n", kToolName);
            std::printf("  %s gpu power limit-set [--index N] --limit 0-255|max [--flags HEX]\n", kToolName);
            std::printf("  %s gpu power monitor info [--index N]\n", kToolName);
            std::printf("  %s gpu power monitor status [--index N]\n", kToolName);
            std::printf("  %s gpu power device info [--index N]\n", kToolName);
            std::printf("  %s gpu power device status [--index N]\n", kToolName);
            std::printf("  %s gpu power capping info [--index N]\n", kToolName);
            std::printf("  %s gpu power capping slowdown [--index N]\n", kToolName);
            std::printf("  %s gpu power leakage info [--index N]\n", kToolName);
            std::printf("  %s gpu power leakage status [--index N]\n", kToolName);
            std::printf("  %s gpu vf tables [--index N]\n", kToolName);
            std::printf("  %s gpu vf inject [--index N] [--flags HEX] [--clk-domain ID --clk-khz N] [--volt-domain logic|sram|msvdd|ID --volt-rail N --volt-uv N --volt-min-uv N]\n", kToolName);
            std::printf("  %s gpu vpstates info [--index N]\n", kToolName);
            std::printf("  %s gpu vpstates control [--index N] [--original]\n", kToolName);
            std::printf("  %s gpu vpstates set [--index N] --vpstate N (--clock N --target-mhz N [--min-eff-mhz N] | --group N --value N)\n", kToolName);
            std::printf("  %s gpu vfe-var info [--index N]\n", kToolName);
            std::printf("  %s gpu vfe-var control [--index N]\n", kToolName);
            std::printf("  %s gpu vfe-var set [--index N] --var N --override-type none|value|offset|scale [--override-value F] [--temp-hyst-pos C] [--temp-hyst-neg C]\n", kToolName);
            std::printf("  %s gpu vfe-equ info [--index N]\n", kToolName);
            std::printf("  %s gpu vfe-equ control [--index N]\n", kToolName);
            std::printf("  %s gpu vfe-equ set [--index N] --equ N (--compare-func eq|gte|gt --compare-crit F | --minmax min|max | --coeffs A,B,C)\n", kToolName);
            std::printf("  %s gpu perf-limits info [--index N]\n", kToolName);
            std::printf("  %s gpu perf-limits status [--index N]\n", kToolName);
            std::printf("  %s gpu perf-limits set [--index N] --limit-id ID --type disabled|pstate|freq|vpstate [--pstate P0] [--point nominal|min|max|mid] [--freq-khz N --domain ID] [--vpstate N]\n", kToolName);
            std::printf("  %s gpu voltage [--index N]\n", kToolName);
            std::printf("  %s gpu voltage control-set [--index N] --enable 0|1\n", kToolName);
            std::printf("  %s gpu thermal [--index N]\n", kToolName);
            std::printf("  %s gpu thermal level [--index N]\n", kToolName);
            std::printf("  %s gpu thermal slowdown [--index N]\n", kToolName);
            std::printf("  %s gpu thermal slowdown-set [--index N] --state enabled|disabled\n", kToolName);
            std::printf("  %s gpu thermal sim [--index N] [--sensor N]\n", kToolName);
            std::printf("  %s gpu thermal sim-set [--index N] --sensor N --mode enabled|disabled [--temp C]\n", kToolName);
            std::printf("  %s gpu fan set [--index N] --cooler N --level PCT [--policy manual|perf|temp-discrete|temp-cont|temp-cont-sw|default]\n", kToolName);
            std::printf("  %s gpu fan restore [--index N] [--cooler N]\n", kToolName);
            std::printf("  %s gpu client-fan coolers info [--index N]\n", kToolName);
            std::printf("  %s gpu client-fan coolers status [--index N]\n", kToolName);
            std::printf("  %s gpu client-fan coolers control [--index N]\n", kToolName);
            std::printf("  %s gpu client-fan coolers set [--index N] --cooler N [--level PCT] [--enable 0|1] [--default]\n", kToolName);
            std::printf("  %s gpu client-fan policies info [--index N]\n", kToolName);
            std::printf("  %s gpu client-fan policies status [--index N]\n", kToolName);
            std::printf("  %s gpu client-fan policies control [--index N]\n", kToolName);
            std::printf("  %s gpu client-fan policies set [--index N] --policy N --fan-stop 0|1 [--default]\n", kToolName);
            std::printf("  %s gpu client-fan arbiters info [--index N]\n", kToolName);
            std::printf("  %s gpu client-fan arbiters status [--index N]\n", kToolName);
            std::printf("  %s gpu client-fan arbiters control [--index N]\n", kToolName);
            std::printf("  %s gpu client-fan arbiters set [--index N] --arbiter N --fan-stop 0|1\n", kToolName);
            std::printf("  %s gpu client-illum devices info [--index N]\n", kToolName);
            std::printf("  %s gpu client-illum devices control [--index N]\n", kToolName);
            std::printf("  %s gpu client-illum devices set [--index N] --device N --sync 0|1 [--timestamp-ms N]\n", kToolName);
            std::printf("  %s gpu client-illum zones info [--index N]\n", kToolName);
            std::printf("  %s gpu client-illum zones control [--index N]\n", kToolName);
            std::printf("  %s gpu client-illum zones set [--index N] --zone N --mode manual-rgb|manual-rgbw|manual-single|manual-color-fixed --brightness N [--r N --g N --b N --w N] [--default]\n", kToolName);
            std::printf("\n");
        }

        void PrintUsageDisplay()
        {
            std::printf("Display commands:\n");
            std::printf("  %s display list\n", kToolName);
            std::printf("  %s display ids [--index N] [--all] [--flags HEX]\n", kToolName);
            std::printf("  %s display edid --id HEX [--flag default|raw|cooked|forced|inf|hw|tiles]\n", kToolName);
            std::printf("  %s display timing --id HEX\n", kToolName);
            std::printf("  %s display get [--handle-index N]\n", kToolName);
            std::printf("  %s display set [--handle-index N] <srcId:device> [srcId:device ...]\n", kToolName);
            std::printf("  %s display custom list --id HEX\n", kToolName);
            std::printf("  %s display custom try --id HEX --width W --height H --refresh R [--depth BPP] [--type auto|cvt|cvt-rb|gtf|dmt|dmt-rb|eia861|analog-tv|nv-predefined] [--interlaced 0|1] [--hw-only 0|1] [--cea-id N] [--tv-format N] [--psf-id N]\n", kToolName);
            std::printf("  %s display custom save --id HEX [--output-only 0|1] [--monitor-only 0|1]\n", kToolName);
            std::printf("  %s display custom delete --id HEX --index N\n", kToolName);
            std::printf("  %s display custom revert --id HEX\n", kToolName);
            std::printf("  %s display monitor-caps --id HEX [--type generic|hdmi-vsdb|hdmi-vcdb|all]\n", kToolName);
            std::printf("  %s display monitor-color-caps --id HEX\n", kToolName);
            std::printf("  %s display scaling --id HEX\n", kToolName);
            std::printf("  %s display scaling-override get --id HEX\n", kToolName);
            std::printf("  %s display scaling-override set --id HEX [--preferred MODE] [--force-override]\n", kToolName);
            std::printf("  %s display viewport get --id HEX\n", kToolName);
            std::printf("  %s display viewport set --id HEX [--x N] [--y N] [--w N] [--h N] [--lock 0|1] [--zoom PCT]\n", kToolName);
            std::printf("  %s display feature get --id HEX\n", kToolName);
            std::printf("  %s display feature set --id HEX [--pan-scan 0|1] [--gdi-primary 0|1]\n", kToolName);
            std::printf("  %s display wide-color get --id HEX [--range xvycc]\n", kToolName);
            std::printf("  %s display wide-color set --id HEX --enable 0|1 [--range xvycc]\n", kToolName);
            std::printf("  %s display bpc get --id HEX\n", kToolName);
            std::printf("  %s display bpc set --id HEX [--bpc default|6|8|10|12|16] [--dither 0|1] [--force-link 0|1] [--force-rg-div 0|1]\n", kToolName);
            std::printf("  %s display blanking get --id HEX\n", kToolName);
            std::printf("  %s display blanking set --id HEX --state 0|1 [--persist 0|1]\n", kToolName);
            std::printf("  %s display hdr caps --id HEX\n", kToolName);
            std::printf("  %s display hdr session get --id HEX\n", kToolName);
            std::printf("  %s display hdr session set --id HEX --enable 0|1 [--expire SEC]\n", kToolName);
            std::printf("  %s display hdr color get --id HEX\n", kToolName);
            std::printf("  %s display hdr color set --id HEX [--mode MODE] [--format FORMAT] [--range RANGE] [--bpc BPC] [--os-hdr default|on|off]\n", kToolName);
            std::printf("  %s display id-by-name --name NAME\n", kToolName);
            std::printf("  %s display gdi-primary\n", kToolName);
            std::printf("  %s display handle-from-id --id HEX\n", kToolName);
            std::printf("  %s display id-from-handle --handle-index N\n", kToolName);
            std::printf("\n");
        }

        void PrintUsageMosaic()
        {
            std::printf("Mosaic commands:\n");
            std::printf("  %s mosaic caps [--index N]\n", kToolName);
            std::printf("  %s mosaic supported [--type all|basic|passive-stereo|scaled-clone|passive-stereo-scaled-clone] [--limit N]\n", kToolName);
            std::printf("  %s mosaic current\n", kToolName);
            std::printf("  %s mosaic enable --state 0|1\n", kToolName);
            std::printf("  %s mosaic display-caps [--limit N]\n", kToolName);
            std::printf("\n");
        }

        void PrintUsageSli()
        {
            std::printf("SLI commands:\n");
            std::printf("  %s sli status\n", kToolName);
            std::printf("  %s sli views [--index N]\n", kToolName);
            std::printf("\n");
        }

        void PrintUsageGsync()
        {
            std::printf("G-Sync commands:\n");
            std::printf("  %s gsync list\n", kToolName);
            std::printf("  %s gsync caps --index N\n", kToolName);
            std::printf("  %s gsync topo --index N\n", kToolName);
            std::printf("  %s gsync sync get --index N\n", kToolName);
            std::printf("  %s gsync sync set --index N --display-id HEX --state master|slave|unsynced [--no-validate] [--send-start-event]\n", kToolName);
            std::printf("  %s gsync sync enable --index N --display-id HEX --state master|slave\n", kToolName);
            std::printf("  %s gsync sync disable --index N --display-id HEX\n", kToolName);
            std::printf("  %s gsync status --index N [--gpu-index N]\n", kToolName);
            std::printf("  %s gsync control get --index N\n", kToolName);
            std::printf("  %s gsync control set --index N [--polarity rising|falling|both] [--video-mode none|ttl|ntsc|hdtv|composite] [--interval N] [--source vsync|housesync] [--interlace 0|1] [--sync-source-output 0|1]\n", kToolName);
            std::printf("\n");
        }

        void PrintUsageDrs()
        {
            std::printf("DRS commands:\n");
            std::printf("  %s drs profiles\n", kToolName);
            std::printf("  %s drs apps --profile NAME\n", kToolName);
            std::printf("  %s drs settings --profile NAME [--start N] [--limit N]\n", kToolName);
            std::printf("  %s drs setting get --profile NAME (--id ID|--name NAME)\n", kToolName);
            std::printf("  %s drs setting set --profile NAME (--id ID|--name NAME) --dword VALUE\n", kToolName);
            std::printf("  %s drs profile create --name NAME\n", kToolName);
            std::printf("  %s drs profile delete --name NAME\n", kToolName);
            std::printf("\n");
        }

        void PrintUsageVideo()
        {
            std::printf("Video commands:\n");
            std::printf("  %s video color get [--handle-index N]\n", kToolName);
            std::printf("  %s video color default [--handle-index N]\n", kToolName);
            std::printf("  %s video color set [--handle-index N] [--brightness N] [--contrast N] [--hue N] [--saturation N] [--color-temp N] [--ygamma N] [--rgamma N] [--ggamma N] [--bgamma N] [--override|--use-app]\n", kToolName);
            std::printf("\n");
        }

        void PrintUsageHdmi()
        {
            std::printf("HDMI commands:\n");
            std::printf("  %s hdmi support [--handle-index N] [--output-id HEX]\n", kToolName);
            std::printf("  %s hdmi hdcp-diag --index N --id HEX\n", kToolName);
            std::printf("  %s hdmi stereo modes --id HEX [--start N] [--count N] [--pass-through] [--width W --height H --refresh R]\n", kToolName);
            std::printf("  %s hdmi stereo get --id HEX\n", kToolName);
            std::printf("  %s hdmi stereo set --id HEX --type NAME|HEX\n", kToolName);
            std::printf("  %s hdmi audio-mute --handle-index N --state 0|1 [--output-id HEX]\n", kToolName);
            std::printf("\n");
        }

        void PrintUsageDp()
        {
            std::printf("DisplayPort commands:\n");
            std::printf("  %s dp info [--id HEX] [--handle-index N] [--output-id HEX]\n", kToolName);
            std::printf("  %s dp set [--id HEX] [--handle-index N] [--output-id HEX] [--link-rate RATE] [--lane-count N] [--format FORMAT] [--range RANGE] [--colorimetry MODE] [--bpc BPC] [--hpd 0|1] [--defer 0|1] [--chroma-lpf-off 0|1] [--dither-off 0|1] [--test-link-train 0|1] [--test-color-change 0|1]\n", kToolName);
            std::printf("  %s dp dongle [--index N] --output-id HEX\n", kToolName);
            std::printf("  %s dp topology --id HEX\n", kToolName);
            std::printf("\n");
        }

        void PrintUsagePcf()
        {
            std::printf("PCF commands:\n");
            std::printf("  %s pcf master info\n", kToolName);
            std::printf("  %s pcf master control\n", kToolName);
            std::printf("  %s pcf master status\n", kToolName);
            std::printf("  %s pcf master set --index N --bus-high N --bus-nominal N\n", kToolName);
            std::printf("\n");
        }

        void PrintUsageSys()
        {
            std::printf("SYS commands:\n");
            std::printf("  %s sys gpu-count\n", kToolName);
            std::printf("  %s sys smp [--default]\n", kToolName);
            std::printf("  %s sys chipset-sli\n", kToolName);
            std::printf("\n");
        }

        void PrintUsageD3d()
        {
            std::printf("D3D toolchain commands:\n");
            std::printf("  %s d3d vrr get [--swapchain] [--surface HANDLE] [--present] [--debug]\n", kToolName);
            std::printf("  %s d3d vrr set --state on|off [--swapchain] [--surface HANDLE] [--present] [--debug]\n", kToolName);
            std::printf("  %s d3d latency get [--out PATH] [--raw]\n", kToolName);
            std::printf("  %s d3d latency mark --frame ID --type TYPE\n", kToolName);
            std::printf("  %s d3d ansel set [--modifier none|ctrl|shift|alt] [--enable-key VK] [--feature NAME:STATE[:VK]]\n", kToolName);
            std::printf("\n");
        }

        void PrintUsageOgl()
        {
            std::printf("OpenGL commands:\n");
            std::printf("  %s ogl expert get\n", kToolName);
            std::printf("  %s ogl expert set --detail MASK --report MASK --output MASK\n", kToolName);
            std::printf("  %s ogl expert defaults-get\n", kToolName);
            std::printf("  %s ogl expert defaults-set --detail MASK --report MASK --output MASK\n", kToolName);
            std::printf("\n");
        }

        void PrintUsageVr()
        {
            std::printf("VR commands:\n");
            std::printf("  %s vr direct-mode enable --vendor-id ID\n", kToolName);
            std::printf("  %s vr direct-mode disable --vendor-id ID\n", kToolName);
            std::printf("  %s vr direct-mode list --vendor-id ID [--flag capable|enabled]\n", kToolName);
            std::printf("  %s vr direct-mode handle-from-id --display-id HEX\n", kToolName);
            std::printf("  %s vr direct-mode id-from-handle --display-id HEX --context HEX\n", kToolName);
            std::printf("\n");
        }

        void PrintUsageStereo()
        {
            std::printf("Stereo commands:\n");
            std::printf("  %s stereo enable\n", kToolName);
            std::printf("  %s stereo disable\n", kToolName);
            std::printf("  %s stereo is-enabled\n", kToolName);
            std::printf("  %s stereo windowed get\n", kToolName);
            std::printf("  %s stereo windowed set --mode off|auto|persistent [--flags N]\n", kToolName);
            std::printf("  %s stereo windowed supported\n", kToolName);
            std::printf("  %s stereo caps get|internal\n", kToolName);
            std::printf("  %s stereo caps monitor --monitor-index N | --id HEX\n", kToolName);
            std::printf("  %s stereo info [--handle-index N]\n", kToolName);
            std::printf("  %s stereo app-info [--handle-index N]\n", kToolName);
            std::printf("  %s stereo mode-enum get\n", kToolName);
            std::printf("  %s stereo mode-enum set --command enable|disable [--data N]\n", kToolName);
            std::printf("  %s stereo accessory\n", kToolName);
            std::printf("  %s stereo dongle control --command NAME [--data N] [--handle-index N]\n", kToolName);
            std::printf("  %s stereo dongle status --id HEX --param N\n", kToolName);
            std::printf("  %s stereo aegis --panel-id N\n", kToolName);
            std::printf("  %s stereo default-profile get\n", kToolName);
            std::printf("  %s stereo default-profile set --name NAME\n", kToolName);
            std::printf("  %s stereo profile create|delete --type default|dx9|dx10\n", kToolName);
            std::printf("  %s stereo profile set --type TYPE --id convergence|frustum (--dword N|--float F)\n", kToolName);
            std::printf("  %s stereo profile delete-value --type TYPE --id convergence|frustum\n", kToolName);
            std::printf("  %s stereo driver-mode set --mode automatic|direct\n", kToolName);
            std::printf("  %s stereo activate|deactivate|is-activated\n", kToolName);
            std::printf("  %s stereo separation get|set|inc|dec [--value PCT]\n", kToolName);
            std::printf("  %s stereo convergence get|set|inc|dec [--value F]\n", kToolName);
            std::printf("  %s stereo frustum get|set --mode none|stretch|clear-edges\n", kToolName);
            std::printf("  %s stereo capture jpeg --quality 0-100\n", kToolName);
            std::printf("  %s stereo capture png\n", kToolName);
            std::printf("  %s stereo init-activation --flag immediate|delayed\n", kToolName);
            std::printf("  %s stereo trigger-activation\n", kToolName);
            std::printf("  %s stereo reverse-blit --enable 0|1\n", kToolName);
            std::printf("  %s stereo notify --hwnd HEX --message-id N\n", kToolName);
            std::printf("  %s stereo active-eye set --eye left|right|mono\n", kToolName);
            std::printf("  %s stereo eye-separation get\n", kToolName);
            std::printf("  %s stereo cursor supported|get|set [--value PCT]\n", kToolName);
            std::printf("  %s stereo surface get|set --mode auto|force-stereo|force-mono\n", kToolName);
            std::printf("  %s stereo debug last-draw\n", kToolName);
            std::printf("  %s stereo force-to-screen --enable 0|1\n", kToolName);
            std::printf("  %s stereo video-control --layout NAME --client-id N --enable 0|1\n", kToolName);
            std::printf("  %s stereo video-metadata --width W --height H --src HEX --dst HEX\n", kToolName);
            std::printf("  %s stereo handshake challenge\n", kToolName);
            std::printf("  %s stereo handshake response --guid GUID --response-hex HEX\n", kToolName);
            std::printf("  %s stereo handshake-trigger\n", kToolName);
            std::printf("  %s stereo handshake-message --enable 0|1\n", kToolName);
            std::printf("  %s stereo profile-name set --name NAME [--flags N]\n", kToolName);
            std::printf("  %s stereo diag\n", kToolName);
            std::printf("  %s stereo shader set --stage vs|ps --type f|i|b --start N --count N --mono PATH --left PATH --right PATH\n", kToolName);
            std::printf("  %s stereo shader get --stage vs|ps --type f|i|b --start N --count N --mono PATH --left PATH --right PATH\n", kToolName);
            std::printf("\n");
        }

        void PrintUsageAll()
        {
            PrintUsageInfo();
            PrintUsageGpu();
            PrintUsageDisplay();
            PrintUsageMosaic();
            PrintUsageSli();
            PrintUsageGsync();
            PrintUsageDrs();
            PrintUsageVideo();
            PrintUsageHdmi();
            PrintUsageDp();
            PrintUsagePcf();
            PrintUsageSys();
            PrintUsageD3d();
            PrintUsageOgl();
            PrintUsageVr();
            PrintUsageStereo();
        }
    }

    void PrintUsageGroup(const char *group)
    {
        if (!group || !*group)
        {
            PrintUsageSummary();
            return;
        }

        std::string key = ToLowerAscii(group);
        if (key == "all")
        {
            PrintUsageAll();
            return;
        }
        if (key == "info")
        {
            PrintUsageInfo();
            return;
        }
        if (key == "gpu")
        {
            PrintUsageGpu();
            return;
        }
        if (key == "display")
        {
            PrintUsageDisplay();
            return;
        }
        if (key == "mosaic")
        {
            PrintUsageMosaic();
            return;
        }
        if (key == "sli")
        {
            PrintUsageSli();
            return;
        }
        if (key == "gsync")
        {
            PrintUsageGsync();
            return;
        }
        if (key == "drs")
        {
            PrintUsageDrs();
            return;
        }
        if (key == "video")
        {
            PrintUsageVideo();
            return;
        }
        if (key == "hdmi")
        {
            PrintUsageHdmi();
            return;
        }
        if (key == "dp")
        {
            PrintUsageDp();
            return;
        }
        if (key == "pcf")
        {
            PrintUsagePcf();
            return;
        }
        if (key == "sys")
        {
            PrintUsageSys();
            return;
        }
        if (key == "d3d")
        {
            PrintUsageD3d();
            return;
        }
        if (key == "ogl")
        {
            PrintUsageOgl();
            return;
        }
        if (key == "vr")
        {
            PrintUsageVr();
            return;
        }
        if (key == "stereo")
        {
            PrintUsageStereo();
            return;
        }

        std::printf("Unknown help group: %s\n", group);
        PrintUsageSummary();
    }

    void PrintUsage()
    {
        PrintUsageSummary();
    }

    const char *DrsSettingTypeName(NVDRS_SETTING_TYPE type)
    {
        switch (type)
        {
        case NVDRS_DWORD_TYPE:
            return "DWORD";
        case NVDRS_BINARY_TYPE:
            return "BINARY";
        case NVDRS_STRING_TYPE:
            return "STRING";
        case NVDRS_WSTRING_TYPE:
            return "WSTRING";
        default:
            return "UNKNOWN";
        }
    }

    void InitDrsSetting(NVDRS_SETTING *setting)
    {
        if (!setting)
        {
            return;
        }
        std::memset(setting, 0, sizeof(*setting));
        setting->version = NVDRS_SETTING_VER;
        setting->binaryPredefinedValue.valueLength = NVAPI_BINARY_DATA_MAX;
        setting->binaryCurrentValue.valueLength = NVAPI_BINARY_DATA_MAX;
    }

    void PrintDrsSetting(const NVDRS_SETTING &setting)
    {
        std::string name = NvUnicodeToUtf8(setting.settingName);
        std::printf("  0x%08X %s (%s) ",
            setting.settingId,
            name.empty() ? "<unnamed>" : name.c_str(),
            DrsSettingTypeName(setting.settingType));

        switch (setting.settingType)
        {
        case NVDRS_DWORD_TYPE:
            std::printf("value=0x%08X (%u)", setting.u32CurrentValue, setting.u32CurrentValue);
            break;
        case NVDRS_STRING_TYPE:
        case NVDRS_WSTRING_TYPE:
        {
            std::string value = NvUnicodeToUtf8(setting.wszCurrentValue);
            std::printf("value=\"%s\"", value.c_str());
            break;
        }
        case NVDRS_BINARY_TYPE:
        {
            const NvU32 length = setting.binaryCurrentValue.valueLength;
            std::printf("valueLen=%u data=", length);
            const NvU32 maxDump = 32;
            NvU32 dumpCount = length < maxDump ? length : maxDump;
            for (NvU32 i = 0; i < dumpCount; ++i)
            {
                std::printf("%02X", setting.binaryCurrentValue.valueData[i]);
            }
            if (length > maxDump)
            {
                std::printf("...");
            }
            break;
        }
        default:
            std::printf("value=<unsupported>");
            break;
        }
        std::printf("\n");
    }

    bool GetDrsProfileByName(NvDRSSessionHandle session, const char *name, NvDRSProfileHandle *outProfile)
    {
        if (!name || !outProfile)
        {
            return false;
        }
        NvAPI_UnicodeString wideName = {};
        if (!Utf8ToNvUnicode(name, wideName))
        {
            std::printf("Invalid profile name encoding.\n");
            return false;
        }
        NvAPI_Status status = NvAPI_DRS_FindProfileByName(session, wideName, outProfile);
        if (status != NVAPI_OK)
        {
            PrintNvapiError("NvAPI_DRS_FindProfileByName failed", status);
            return false;
        }
        return true;
    }

    bool GetDrsSettingIdByName(const char *name, NvU32 *outId)
    {
        if (!name || !outId)
        {
            return false;
        }
        NvAPI_UnicodeString wideName = {};
        if (!Utf8ToNvUnicode(name, wideName))
        {
            std::printf("Invalid setting name encoding.\n");
            return false;
        }
        NvAPI_Status status = NvAPI_DRS_GetSettingIdFromName(wideName, outId);
        if (status != NVAPI_OK)
        {
            PrintNvapiError("NvAPI_DRS_GetSettingIdFromName failed", status);
            return false;
        }
        return true;
    }
}
