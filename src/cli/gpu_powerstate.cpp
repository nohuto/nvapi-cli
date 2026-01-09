/*
 * Copyright (c) 2026 Noverse (Nohuto). All rights reserved.
 * Proprietary and confidential. Unauthorized copying or redistribution is strictly prohibited.
 */

#include "commands.h"

namespace nvcli
{
    namespace
    {
        const char *EventLevelName(NV_EVENT_LEVEL level)
        {
            switch (level)
            {
            case UNKNOWN_LEVEL:
                return "UNKNOWN";
            case NORMAL_LEVEL:
                return "NORMAL";
            case WARNING_LEVEL:
                return "WARNING";
            case CRITICAL_LEVEL:
                return "CRITICAL";
            default:
                return "UNKNOWN";
            }
        }

        const char *Gc6ControlOpName(NV_GPU_GC6_CONTROL_OP op)
        {
            switch (op)
            {
            case CLEAR_STATS:
                return "CLEAR_STATS";
            case SET_STATS_ENABLED:
                return "SET_STATS_ENABLED";
            case SET_STATS_DISABLED:
                return "SET_STATS_DISABLED";
            case IS_GC6_SUPPORTED:
                return "IS_GC6_SUPPORTED";
            case IS_GC6_ENABLED:
                return "IS_GC6_ENABLED";
            default:
                return "UNKNOWN";
            }
        }

        const char *Gc6ControlStatusName(NV_GPU_GC6_CONTROL_STATUS status)
        {
            switch (status)
            {
            case SUCCESS:
                return "SUCCESS";
            case FEATURE_DISABLED:
                return "FEATURE_DISABLED";
            case CANNOT_ENABLE_STATS:
                return "CANNOT_ENABLE_STATS";
            case STATS_DISABLED:
                return "STATS_DISABLED";
            case FAILED:
                return "FAILED";
            default:
                return "UNKNOWN";
            }
        }

        bool ParseGc6ControlOp(const char *value, NV_GPU_GC6_CONTROL_OP *out)
        {
            if (!value || !out)
            {
                return false;
            }
            std::string lowered = ToLowerAscii(value);
            if (lowered == "clear-stats")
            {
                *out = CLEAR_STATS;
                return true;
            }
            if (lowered == "enable-stats")
            {
                *out = SET_STATS_ENABLED;
                return true;
            }
            if (lowered == "disable-stats")
            {
                *out = SET_STATS_DISABLED;
                return true;
            }
            if (lowered == "supported")
            {
                *out = IS_GC6_SUPPORTED;
                return true;
            }
            if (lowered == "enabled")
            {
                *out = IS_GC6_ENABLED;
                return true;
            }
            return false;
        }

        const char *DeepIdleStateName(NV_DEEP_IDLE_STATE state)
        {
            switch (state)
            {
            case NV_DEEP_IDLE_ENABLE:
                return "ENABLED";
            case NV_DEEP_IDLE_DISABLE:
                return "DISABLED";
            case NV_DEEP_IDLE_NOT_SUPPORTED:
                return "NOT_SUPPORTED";
            default:
                return "UNKNOWN";
            }
        }

        bool ParseDeepIdleState(const char *value, NV_DEEP_IDLE_STATE *out)
        {
            if (!value || !out)
            {
                return false;
            }
            std::string lowered = ToLowerAscii(value);
            if (lowered == "enabled")
            {
                *out = NV_DEEP_IDLE_ENABLE;
                return true;
            }
            if (lowered == "disabled")
            {
                *out = NV_DEEP_IDLE_DISABLE;
                return true;
            }
            return false;
        }

        const char *DeepIdleStatsModeName(NV_DEEP_IDLE_STATISTICS_MODE mode)
        {
            switch (mode)
            {
            case NV_DEEP_IDLE_STATISTICS_MODE_NH:
                return "NH";
            case NV_DEEP_IDLE_STATISTICS_MODE_VE:
                return "VE";
            case NV_DEEP_IDLE_STATISTICS_MODE_SSC:
                return "SSC";
            case NV_DEEP_IDLE_STATISTICS_MODE_FO:
                return "FO";
            default:
                return "UNKNOWN";
            }
        }

        bool ParseDeepIdleStatsMode(const char *value, NV_DEEP_IDLE_STATISTICS_MODE *out)
        {
            if (!value || !out)
            {
                return false;
            }
            std::string lowered = ToLowerAscii(value);
            if (lowered == "nh")
            {
                *out = NV_DEEP_IDLE_STATISTICS_MODE_NH;
                return true;
            }
            if (lowered == "ve")
            {
                *out = NV_DEEP_IDLE_STATISTICS_MODE_VE;
                return true;
            }
            if (lowered == "ssc")
            {
                *out = NV_DEEP_IDLE_STATISTICS_MODE_SSC;
                return true;
            }
            if (lowered == "fo")
            {
                *out = NV_DEEP_IDLE_STATISTICS_MODE_FO;
                return true;
            }
            return false;
        }

        const char *PstateFallbackName(NV_GPU_PERF_PSTATE_FALLBACK fallback)
        {
            switch (fallback)
            {
            case NVAPI_GPU_PERF_PSTATE_FALLBACK_RETURN_ERROR:
                return "ERROR";
            case NVAPI_GPU_PERF_PSTATE_FALLBACK_HIGHER_PERF:
                return "HIGHER";
            case NVAPI_GPU_PERF_PSTATE_FALLBACK_LOWER_PERF:
                return "LOWER";
            default:
                return "UNKNOWN";
            }
        }

        bool ParsePstateFallback(const char *value, NV_GPU_PERF_PSTATE_FALLBACK *out)
        {
            if (!value || !out)
            {
                return false;
            }
            std::string lowered = ToLowerAscii(value);
            if (lowered == "error")
            {
                *out = NVAPI_GPU_PERF_PSTATE_FALLBACK_RETURN_ERROR;
                return true;
            }
            if (lowered == "higher")
            {
                *out = NVAPI_GPU_PERF_PSTATE_FALLBACK_HIGHER_PERF;
                return true;
            }
            if (lowered == "lower")
            {
                *out = NVAPI_GPU_PERF_PSTATE_FALLBACK_LOWER_PERF;
                return true;
            }
            return false;
        }

        bool ParseForcePstateId(const char *value, NV_GPU_PERF_PSTATE_ID *out)
        {
            if (!value || !out)
            {
                return false;
            }
            std::string lowered = ToLowerAscii(value);
            if (lowered == "auto")
            {
                *out = NVAPI_GPU_PERF_PSTATE_UNDEFINED;
                return true;
            }
            return ParsePstateId(value, out);
        }
    }

    int CmdGpuDynamicPstatesSet(int argc, char **argv)
    {
        NvU32 index = 0;
        bool hasIndex = false;
        bool enable = false;
        bool hasEnable = false;

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
            if (std::strcmp(argv[i], "--enable") == 0)
            {
                if (i + 1 >= argc)
                {
                    std::printf("Missing value for --enable\n");
                    return 1;
                }
                if (!ParseBoolValue(argv[i + 1], &enable))
                {
                    std::printf("Invalid value for --enable: %s\n", argv[i + 1]);
                    return 1;
                }
                hasEnable = true;
                ++i;
                continue;
            }
            std::printf("Unknown option: %s\n", argv[i]);
            return 1;
        }

        if (!hasEnable)
        {
            std::printf("Missing required --enable\n");
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
            NvAPI_Status status = NvAPI_GPU_EnableDynamicPstates(handles[i], enable ? 1 : 0);
            if (status != NVAPI_OK)
            {
                PrintNvapiError("  NvAPI_GPU_EnableDynamicPstates failed", status);
                continue;
            }
            std::printf("  Dynamic Pstates: %s\n", enable ? "enabled" : "disabled");
        }
        return 0;
    }

    int CmdGpuPcieAspmSet(int argc, char **argv)
    {
        NvU32 index = 0;
        bool hasIndex = false;
        bool enable = false;
        bool hasEnable = false;

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
            if (std::strcmp(argv[i], "--enable") == 0)
            {
                if (i + 1 >= argc)
                {
                    std::printf("Missing value for --enable\n");
                    return 1;
                }
                if (!ParseBoolValue(argv[i + 1], &enable))
                {
                    std::printf("Invalid value for --enable: %s\n", argv[i + 1]);
                    return 1;
                }
                hasEnable = true;
                ++i;
                continue;
            }
            std::printf("Unknown option: %s\n", argv[i]);
            return 1;
        }

        if (!hasEnable)
        {
            std::printf("Missing required --enable\n");
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
            NV_GPU_ASPM_CONTROL_DATA data = {};
            data.version = NV_GPU_ASPM_CONTROL_DATA_VER;
            data.bEnable = enable ? 1 : 0;
            NvAPI_Status status = NvAPI_GPU_ControlASPM(handles[i], &data);
            if (status != NVAPI_OK)
            {
                PrintNvapiError("  NvAPI_GPU_ControlASPM failed", status);
                continue;
            }
            std::printf("  ASPM: %s\n", enable ? "enabled" : "disabled");
        }
        return 0;
    }

    int CmdGpuGc6Control(int argc, char **argv)
    {
        NvU32 index = 0;
        bool hasIndex = false;
        const char *opValue = NULL;
        NV_GPU_GC6_CONTROL_OP op = CLEAR_STATS;

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
            if (std::strcmp(argv[i], "--op") == 0)
            {
                if (i + 1 >= argc)
                {
                    std::printf("Missing value for --op\n");
                    return 1;
                }
                opValue = argv[i + 1];
                ++i;
                continue;
            }
            std::printf("Unknown option: %s\n", argv[i]);
            return 1;
        }

        if (!opValue)
        {
            std::printf("Missing required --op\n");
            return 1;
        }
        if (!ParseGc6ControlOp(opValue, &op))
        {
            std::printf("Invalid GC6 op: %s\n", opValue);
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
            NV_GPU_GC6_CONTROL control = {};
            control.version = NV_GPU_GC6_CONTROL_VER;
            control.controlOp = op;
            NvAPI_Status status = NvAPI_GPU_GC6Control(handles[i], &control);
            if (status != NVAPI_OK)
            {
                PrintNvapiError("  NvAPI_GPU_GC6Control failed", status);
                continue;
            }
            std::printf("  GC6 op=%s status=%s (%u)\n",
                Gc6ControlOpName(op),
                Gc6ControlStatusName(control.controlStatus),
                static_cast<unsigned>(control.controlStatus));
        }
        return 0;
    }

    int CmdGpuGc6ForceExit(int argc, char **argv)
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
            NvAPI_Status status = NvAPI_GPU_ForceGC6Exit(handles[i]);
            if (status != NVAPI_OK)
            {
                PrintNvapiError("  NvAPI_GPU_ForceGC6Exit failed", status);
                continue;
            }
            std::printf("  GC6 exit requested.\n");
        }
        return 0;
    }

    int CmdGpuGc6(int argc, char **argv)
    {
        if (argc < 1)
        {
            std::printf("Missing gc6 subcommand.\n");
            return 1;
        }
        if (std::strcmp(argv[0], "control") == 0)
        {
            return CmdGpuGc6Control(argc - 1, argv + 1);
        }
        if (std::strcmp(argv[0], "force-exit") == 0)
        {
            return CmdGpuGc6ForceExit(argc - 1, argv + 1);
        }
        std::printf("Unknown gc6 subcommand: %s\n", argv[0]);
        return 1;
    }

    int CmdGpuThermalLevel(int argc, char **argv)
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
            NV_EVENT_LEVEL level = UNKNOWN_LEVEL;
            NvAPI_Status status = NvAPI_GPU_GetCurrentThermalLevel(handles[i], &level);
            if (status != NVAPI_OK)
            {
                PrintNvapiError("  NvAPI_GPU_GetCurrentThermalLevel failed", status);
                continue;
            }
            std::printf("  Thermal level: %s (%u)\n", EventLevelName(level), static_cast<unsigned>(level));
        }
        return 0;
    }

    int CmdGpuDeepIdleState(int argc, char **argv)
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
            NvLogicalGpuHandle logical = NULL;
            NvAPI_Status status = NvAPI_GetLogicalGPUFromPhysicalGPU(handles[i], &logical);
            if (status != NVAPI_OK)
            {
                PrintNvapiError("  NvAPI_GetLogicalGPUFromPhysicalGPU failed", status);
                continue;
            }

            NV_DEEP_IDLE_STATE state = NV_DEEP_IDLE_NOT_SUPPORTED;
            status = NvAPI_GPU_GetDeepIdleState(logical, &state);
            if (status != NVAPI_OK)
            {
                PrintNvapiError("  NvAPI_GPU_GetDeepIdleState failed", status);
                continue;
            }
            std::printf("  Deep idle: %s (%d)\n", DeepIdleStateName(state), static_cast<int>(state));
        }

        return 0;
    }

    int CmdGpuDeepIdleSet(int argc, char **argv)
    {
        NvU32 index = 0;
        bool hasIndex = false;
        bool hasState = false;
        NV_DEEP_IDLE_STATE state = NV_DEEP_IDLE_DISABLE;

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
            if (std::strcmp(argv[i], "--state") == 0)
            {
                if (i + 1 >= argc)
                {
                    std::printf("Missing value for --state\n");
                    return 1;
                }
                if (!ParseDeepIdleState(argv[i + 1], &state))
                {
                    std::printf("Invalid deep idle state: %s\n", argv[i + 1]);
                    return 1;
                }
                hasState = true;
                ++i;
                continue;
            }
            std::printf("Unknown option: %s\n", argv[i]);
            return 1;
        }

        if (!hasState)
        {
            std::printf("Missing required --state\n");
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
            NvLogicalGpuHandle logical = NULL;
            NvAPI_Status status = NvAPI_GetLogicalGPUFromPhysicalGPU(handles[i], &logical);
            if (status != NVAPI_OK)
            {
                PrintNvapiError("  NvAPI_GetLogicalGPUFromPhysicalGPU failed", status);
                continue;
            }
            status = NvAPI_GPU_SetDeepIdleState(logical, state);
            if (status != NVAPI_OK)
            {
                PrintNvapiError("  NvAPI_GPU_SetDeepIdleState failed", status);
                continue;
            }
            std::printf("  Deep idle set: %s\n", DeepIdleStateName(state));
        }

        return 0;
    }

    int CmdGpuDeepIdleStatsMode(int argc, char **argv)
    {
        NvU32 index = 0;
        bool hasIndex = false;
        bool hasMode = false;
        bool reset = false;
        NV_DEEP_IDLE_STATISTICS_MODE mode = NV_DEEP_IDLE_STATISTICS_MODE_NH;

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
            if (std::strcmp(argv[i], "--mode") == 0)
            {
                if (i + 1 >= argc)
                {
                    std::printf("Missing value for --mode\n");
                    return 1;
                }
                if (!ParseDeepIdleStatsMode(argv[i + 1], &mode))
                {
                    std::printf("Invalid deep idle stats mode: %s\n", argv[i + 1]);
                    return 1;
                }
                hasMode = true;
                ++i;
                continue;
            }
            if (std::strcmp(argv[i], "--reset") == 0)
            {
                if (i + 1 >= argc)
                {
                    std::printf("Missing value for --reset\n");
                    return 1;
                }
                if (!ParseBoolValue(argv[i + 1], &reset))
                {
                    std::printf("Invalid value for --reset: %s\n", argv[i + 1]);
                    return 1;
                }
                ++i;
                continue;
            }
            std::printf("Unknown option: %s\n", argv[i]);
            return 1;
        }

        if (!hasMode)
        {
            std::printf("Missing required --mode\n");
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
            NvAPI_Status status = NvAPI_GPU_SetDeepIdleStatisticsMode(handles[i], mode, reset ? 1 : 0);
            if (status != NVAPI_OK)
            {
                PrintNvapiError("  NvAPI_GPU_SetDeepIdleStatisticsMode failed", status);
                continue;
            }
            std::printf("  Deep idle stats mode: %s reset=%u\n", DeepIdleStatsModeName(mode), reset ? 1u : 0u);
        }

        return 0;
    }

    int CmdGpuDeepIdleStats(int argc, char **argv)
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
            NV_DEEP_IDLE_STATISTICS stats = {};
            stats.version = NV_DEEP_IDLE_STATISTICS_VER;
            NvAPI_Status status = NvAPI_GPU_GetDeepIdleStatistics(handles[i], &stats);
            if (status != NVAPI_OK)
            {
                PrintNvapiError("  NvAPI_GPU_GetDeepIdleStatistics failed", status);
                continue;
            }
            std::printf("  Deep idle attempts=%u entries=%u exits=%u time=%u us\n",
                stats.attempts,
                stats.entries,
                stats.exits,
                stats.time);
            std::printf("  Max latency: entry=%u us exit=%u us\n",
                stats.maxEntryLatency,
                stats.maxExitLatency);
            std::printf("  VE stats: frames=%u vblankExits=%u deepL1Exits=%u\n",
                stats.veFrames,
                stats.veVblankExits,
                stats.veDeepL1Exits);
        }

        return 0;
    }

    int CmdGpuDeepIdle(int argc, char **argv)
    {
        if (argc < 1)
        {
            std::printf("Missing deep-idle subcommand.\n");
            return 1;
        }
        if (std::strcmp(argv[0], "state") == 0)
        {
            return CmdGpuDeepIdleState(argc - 1, argv + 1);
        }
        if (std::strcmp(argv[0], "set") == 0)
        {
            return CmdGpuDeepIdleSet(argc - 1, argv + 1);
        }
        if (std::strcmp(argv[0], "stats-mode") == 0)
        {
            return CmdGpuDeepIdleStatsMode(argc - 1, argv + 1);
        }
        if (std::strcmp(argv[0], "stats") == 0)
        {
            return CmdGpuDeepIdleStats(argc - 1, argv + 1);
        }
        std::printf("Unknown deep-idle subcommand: %s\n", argv[0]);
        return 1;
    }

    int CmdGpuForcePstate(int argc, char **argv)
    {
        NvU32 index = 0;
        bool hasIndex = false;
        bool hasPstate = false;
        NV_GPU_PERF_PSTATE_ID pstate = NVAPI_GPU_PERF_PSTATE_UNDEFINED;
        NV_GPU_PERF_PSTATE_FALLBACK fallback = NVAPI_GPU_PERF_PSTATE_FALLBACK_RETURN_ERROR;

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
            if (std::strcmp(argv[i], "--pstate") == 0)
            {
                if (i + 1 >= argc)
                {
                    std::printf("Missing value for --pstate\n");
                    return 1;
                }
                if (!ParseForcePstateId(argv[i + 1], &pstate))
                {
                    std::printf("Invalid pstate: %s\n", argv[i + 1]);
                    return 1;
                }
                hasPstate = true;
                ++i;
                continue;
            }
            if (std::strcmp(argv[i], "--fallback") == 0)
            {
                if (i + 1 >= argc)
                {
                    std::printf("Missing value for --fallback\n");
                    return 1;
                }
                if (!ParsePstateFallback(argv[i + 1], &fallback))
                {
                    std::printf("Invalid fallback: %s\n", argv[i + 1]);
                    return 1;
                }
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

        std::vector<NvPhysicalGpuHandle> handles;
        std::vector<NvU32> indices;
        if (!CollectGpus(hasIndex, index, handles, indices))
        {
            return 1;
        }

        for (size_t i = 0; i < handles.size(); ++i)
        {
            PrintGpuHeader(indices[i], handles[i]);
            NvAPI_Status status = NvAPI_GPU_SetForcePstate(handles[i], pstate, fallback);
            if (status != NVAPI_OK)
            {
                PrintNvapiError("  NvAPI_GPU_SetForcePstate failed", status);
                if (status == NVAPI_INVALID_USER_PRIVILEGE)
                {
                    std::printf("  Try running the terminal as Administrator.\n");
                }
                continue;
            }
            const char *pstateName = (pstate == NVAPI_GPU_PERF_PSTATE_UNDEFINED) ? "AUTO" : PstateName(pstate);
            std::printf("  Force pstate: %s fallback=%s\n", pstateName, PstateFallbackName(fallback));
        }

        return 0;
    }

    int CmdGpuForcePstateEx(int argc, char **argv)
    {
        NvU32 index = 0;
        bool hasIndex = false;
        bool hasPstate = false;
        bool async = false;
        NV_GPU_PERF_PSTATE_ID pstate = NVAPI_GPU_PERF_PSTATE_UNDEFINED;
        NV_GPU_PERF_PSTATE_FALLBACK fallback = NVAPI_GPU_PERF_PSTATE_FALLBACK_RETURN_ERROR;

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
            if (std::strcmp(argv[i], "--pstate") == 0)
            {
                if (i + 1 >= argc)
                {
                    std::printf("Missing value for --pstate\n");
                    return 1;
                }
                if (!ParseForcePstateId(argv[i + 1], &pstate))
                {
                    std::printf("Invalid pstate: %s\n", argv[i + 1]);
                    return 1;
                }
                hasPstate = true;
                ++i;
                continue;
            }
            if (std::strcmp(argv[i], "--fallback") == 0)
            {
                if (i + 1 >= argc)
                {
                    std::printf("Missing value for --fallback\n");
                    return 1;
                }
                if (!ParsePstateFallback(argv[i + 1], &fallback))
                {
                    std::printf("Invalid fallback: %s\n", argv[i + 1]);
                    return 1;
                }
                ++i;
                continue;
            }
            if (std::strcmp(argv[i], "--async") == 0)
            {
                if (i + 1 >= argc)
                {
                    std::printf("Missing value for --async\n");
                    return 1;
                }
                if (!ParseBoolValue(argv[i + 1], &async))
                {
                    std::printf("Invalid value for --async: %s\n", argv[i + 1]);
                    return 1;
                }
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

        std::vector<NvPhysicalGpuHandle> handles;
        std::vector<NvU32> indices;
        if (!CollectGpus(hasIndex, index, handles, indices))
        {
            return 1;
        }

        NvU32 flags = async ? NV_GPU_PERF_SET_FORCE_PSTATE_FLAGS_ASYNC : 0;
        for (size_t i = 0; i < handles.size(); ++i)
        {
            PrintGpuHeader(indices[i], handles[i]);
            NvAPI_Status status = NvAPI_GPU_SetForcePstateEx(handles[i], pstate, fallback, flags);
            if (status != NVAPI_OK)
            {
                PrintNvapiError("  NvAPI_GPU_SetForcePstateEx failed", status);
                if (status == NVAPI_INVALID_USER_PRIVILEGE)
                {
                    std::printf("  Try running the terminal as Administrator.\n");
                }
                continue;
            }
            const char *pstateName = (pstate == NVAPI_GPU_PERF_PSTATE_UNDEFINED) ? "AUTO" : PstateName(pstate);
            std::printf("  Force pstate: %s fallback=%s async=%u\n", pstateName, PstateFallbackName(fallback), async ? 1u : 0u);
        }

        return 0;
    }

}
