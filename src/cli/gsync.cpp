/*
 * Copyright (c) 2026 Noverse (Nohuto). All rights reserved.
 * Proprietary and confidential. Unauthorized copying or redistribution is strictly prohibited.
 */

#include "commands.h"

namespace nvcli
{
    int CmdGsyncSyncGet(int argc, char **argv);
    int CmdGsyncSyncSet(int argc, char **argv);
    int CmdGsyncSyncEnable(int argc, char **argv);
    int CmdGsyncSyncDisable(int argc, char **argv);
    int CmdGsyncControlGet(int argc, char **argv);
    int CmdGsyncControlSet(int argc, char **argv);

    namespace
    {
        int CmdGsyncListAdapter(int argc, char **argv)
        {
            (void)argc;
            (void)argv;
            return CmdGsyncList();
        }

        void PrintGsyncUsage()
        {
            PrintUsageGroup("gsync");
        }

        int CmdGsyncSyncDispatch(int argc, char **argv)
        {
            static const SubcommandEntry kSubcommands[] = {
                {"get", CmdGsyncSyncGet},
                {"set", CmdGsyncSyncSet},
                {"enable", CmdGsyncSyncEnable},
                {"disable", CmdGsyncSyncDisable},
            };

            return DispatchSubcommand("gsync sync", argc, argv, kSubcommands, sizeof(kSubcommands) / sizeof(kSubcommands[0]), PrintGsyncUsage);
        }

        const char *GsyncPolarityName(NVAPI_GSYNC_POLARITY polarity)
        {
            switch (polarity)
            {
            case NVAPI_GSYNC_POLARITY_RISING_EDGE:
                return "RISING";
            case NVAPI_GSYNC_POLARITY_FALLING_EDGE:
                return "FALLING";
            case NVAPI_GSYNC_POLARITY_BOTH_EDGES:
                return "BOTH";
            default:
                return "UNKNOWN";
            }
        }

        const char *GsyncVideoModeName(NVAPI_GSYNC_VIDEO_MODE mode)
        {
            switch (mode)
            {
            case NVAPI_GSYNC_VIDEO_MODE_NONE:
                return "NONE";
            case NVAPI_GSYNC_VIDEO_MODE_TTL:
                return "TTL";
            case NVAPI_GSYNC_VIDEO_MODE_NTSCPALSECAM:
                return "NTSCPALSECAM";
            case NVAPI_GSYNC_VIDEO_MODE_HDTV:
                return "HDTV";
            case NVAPI_GSYNC_VIDEO_MODE_COMPOSITE:
                return "COMPOSITE";
            default:
                return "UNKNOWN";
            }
        }

        const char *GsyncSyncSourceName(NVAPI_GSYNC_SYNC_SOURCE source)
        {
            switch (source)
            {
            case NVAPI_GSYNC_SYNC_SOURCE_VSYNC:
                return "VSYNC";
            case NVAPI_GSYNC_SYNC_SOURCE_HOUSESYNC:
                return "HOUSESYNC";
            default:
                return "UNKNOWN";
            }
        }

        const char *GsyncRj45IoName(NVAPI_GSYNC_RJ45_IO value)
        {
            switch (value)
            {
            case NVAPI_GSYNC_RJ45_OUTPUT:
                return "OUTPUT";
            case NVAPI_GSYNC_RJ45_INPUT:
                return "INPUT";
            case NVAPI_GSYNC_RJ45_UNUSED:
                return "UNUSED";
            default:
                return "UNKNOWN";
            }
        }

        bool ParseGsyncPolarity(const char *value, NVAPI_GSYNC_POLARITY *out)
        {
            if (!value || !out)
            {
                return false;
            }
            std::string lowered = ToLowerAscii(value);
            if (lowered == "rising")
            {
                *out = NVAPI_GSYNC_POLARITY_RISING_EDGE;
                return true;
            }
            if (lowered == "falling")
            {
                *out = NVAPI_GSYNC_POLARITY_FALLING_EDGE;
                return true;
            }
            if (lowered == "both")
            {
                *out = NVAPI_GSYNC_POLARITY_BOTH_EDGES;
                return true;
            }
            NvU32 numeric = 0;
            if (!ParseUint(value, &numeric))
            {
                return false;
            }
            *out = static_cast<NVAPI_GSYNC_POLARITY>(numeric);
            return true;
        }

        bool ParseGsyncVideoMode(const char *value, NVAPI_GSYNC_VIDEO_MODE *out)
        {
            if (!value || !out)
            {
                return false;
            }
            std::string lowered = ToLowerAscii(value);
            if (lowered == "none")
            {
                *out = NVAPI_GSYNC_VIDEO_MODE_NONE;
                return true;
            }
            if (lowered == "ttl")
            {
                *out = NVAPI_GSYNC_VIDEO_MODE_TTL;
                return true;
            }
            if (lowered == "ntsc")
            {
                *out = NVAPI_GSYNC_VIDEO_MODE_NTSCPALSECAM;
                return true;
            }
            if (lowered == "hdtv")
            {
                *out = NVAPI_GSYNC_VIDEO_MODE_HDTV;
                return true;
            }
            if (lowered == "composite")
            {
                *out = NVAPI_GSYNC_VIDEO_MODE_COMPOSITE;
                return true;
            }
            NvU32 numeric = 0;
            if (!ParseUint(value, &numeric))
            {
                return false;
            }
            *out = static_cast<NVAPI_GSYNC_VIDEO_MODE>(numeric);
            return true;
        }

        bool ParseGsyncSyncSource(const char *value, NVAPI_GSYNC_SYNC_SOURCE *out)
        {
            if (!value || !out)
            {
                return false;
            }
            std::string lowered = ToLowerAscii(value);
            if (lowered == "vsync")
            {
                *out = NVAPI_GSYNC_SYNC_SOURCE_VSYNC;
                return true;
            }
            if (lowered == "housesync")
            {
                *out = NVAPI_GSYNC_SYNC_SOURCE_HOUSESYNC;
                return true;
            }
            NvU32 numeric = 0;
            if (!ParseUint(value, &numeric))
            {
                return false;
            }
            *out = static_cast<NVAPI_GSYNC_SYNC_SOURCE>(numeric);
            return true;
        }

        bool ParseGsyncDisplayState(const char *value, NVAPI_GSYNC_DISPLAY_SYNC_STATE *out)
        {
            if (!value || !out)
            {
                return false;
            }
            std::string lowered = ToLowerAscii(value);
            if (lowered == "unsynced")
            {
                *out = NVAPI_GSYNC_DISPLAY_SYNC_STATE_UNSYNCED;
                return true;
            }
            if (lowered == "slave")
            {
                *out = NVAPI_GSYNC_DISPLAY_SYNC_STATE_SLAVE;
                return true;
            }
            if (lowered == "master")
            {
                *out = NVAPI_GSYNC_DISPLAY_SYNC_STATE_MASTER;
                return true;
            }
            NvU32 numeric = 0;
            if (!ParseUint(value, &numeric))
            {
                return false;
            }
            *out = static_cast<NVAPI_GSYNC_DISPLAY_SYNC_STATE>(numeric);
            return true;
        }
    }

    int CmdGsyncList()
    {
        NvGSyncDeviceHandle handles[NVAPI_MAX_GSYNC_DEVICES] = {};
        NvU32 count = 0;
        NvAPI_Status status = NvAPI_GSync_EnumSyncDevices(handles, &count);
        if (status != NVAPI_OK)
        {
            PrintNvapiError("NvAPI_GSync_EnumSyncDevices failed", status);
            return 1;
        }

        std::printf("G-Sync devices: %u\n", count);
        for (NvU32 i = 0; i < count; ++i)
        {
            std::printf("  [%u] handle=0x%p\n", i, handles[i]);
        }
        return 0;
    }

    int CmdGsyncCaps(int argc, char **argv)
    {
        NvU32 index = 0;
        bool hasIndex = false;
        for (int i = 0; i < argc; ++i)
        {
            if (std::strcmp(argv[i], "--index") == 0)
            {
                if (i + 1 >= argc || !ParseUint(argv[i + 1], &index))
                {
                    std::printf("Invalid index.\n");
                    return 1;
                }
                hasIndex = true;
                ++i;
                continue;
            }
            std::printf("Unknown option: %s\n", argv[i]);
            return 1;
        }

        if (!hasIndex)
        {
            std::printf("Missing required --index\n");
            return 1;
        }

        NvGSyncDeviceHandle handle = NULL;
        if (!GetGsyncHandleByIndex(index, &handle))
        {
            return 1;
        }

        NV_GSYNC_CAPABILITIES caps = {};
        caps.version = NV_GSYNC_CAPABILITIES_VER;
        NvAPI_Status status = NvAPI_GSync_QueryCapabilities(handle, &caps);
        if (status != NVAPI_OK)
        {
            PrintNvapiError("NvAPI_GSync_QueryCapabilities failed", status);
            return 1;
        }

        std::printf("G-Sync caps: boardId=0x%08X revId=0x%08X revision=%u extendedRevision=%u\n",
            caps.boardId,
            caps.revId,
            caps.revision,
            caps.extendedRevision);
        std::printf("  capFlags=0x%08X\n", caps.capFlags);
        std::printf("  maxSyncSkew=%u syncSkewResolution=%u ns\n", caps.maxSyncSkew, caps.syncSkewResolution);
        std::printf("  maxStartDelay=%u startDelayResolution=%u ns\n", caps.maxStartDelay, caps.startDelayResolution);
        return 0;
    }

    int CmdGsyncTopology(int argc, char **argv)
    {
        NvU32 index = 0;
        bool hasIndex = false;
        for (int i = 0; i < argc; ++i)
        {
            if (std::strcmp(argv[i], "--index") == 0)
            {
                if (i + 1 >= argc || !ParseUint(argv[i + 1], &index))
                {
                    std::printf("Invalid index.\n");
                    return 1;
                }
                hasIndex = true;
                ++i;
                continue;
            }
            std::printf("Unknown option: %s\n", argv[i]);
            return 1;
        }

        if (!hasIndex)
        {
            std::printf("Missing required --index\n");
            return 1;
        }

        NvGSyncDeviceHandle handle = NULL;
        if (!GetGsyncHandleByIndex(index, &handle))
        {
            return 1;
        }

        NvU32 gpuCount = 0;
        NvU32 displayCount = 0;
        NvAPI_Status status = NvAPI_GSync_GetTopology(handle, &gpuCount, NULL, &displayCount, NULL);
        if (status != NVAPI_OK)
        {
            PrintNvapiError("NvAPI_GSync_GetTopology failed", status);
            return 1;
        }
        if (gpuCount == 0 && displayCount == 0)
        {
            std::printf("No G-Sync topology entries found.\n");
            return 1;
        }

        std::vector<NV_GSYNC_GPU> gpus(gpuCount);
        std::vector<NV_GSYNC_DISPLAY> displays(displayCount);
        for (NvU32 i = 0; i < gpuCount; ++i)
        {
            gpus[i].version = NV_GSYNC_GPU_VER;
        }
        for (NvU32 i = 0; i < displayCount; ++i)
        {
            displays[i].version = NV_GSYNC_DISPLAY_VER;
        }

        status = NvAPI_GSync_GetTopology(handle, &gpuCount, gpus.data(), &displayCount, displays.data());
        if (status != NVAPI_OK)
        {
            PrintNvapiError("NvAPI_GSync_GetTopology failed", status);
            return 1;
        }

        std::printf("G-Sync topology: gpus=%u displays=%u\n", gpuCount, displayCount);
        for (NvU32 i = 0; i < gpuCount; ++i)
        {
            NvAPI_ShortString name = {0};
            NvAPI_GPU_GetFullName(gpus[i].hPhysicalGpu, name);
            std::printf("  GPU[%u] handle=0x%p name=%s connector=%s proxy=0x%p synced=%u\n",
                i,
                gpus[i].hPhysicalGpu,
                name,
                GsyncConnectorName(gpus[i].connector),
                gpus[i].hProxyPhysicalGpu,
                gpus[i].isSynced ? 1u : 0u);
        }

        for (NvU32 i = 0; i < displayCount; ++i)
        {
            std::printf("  display[%u] id=0x%08X state=%s masterable=%u\n",
                i,
                displays[i].displayId,
                GsyncDisplaySyncStateName(displays[i].syncState),
                displays[i].isMasterable ? 1u : 0u);
        }

        return 0;
    }

    int CmdGsyncSyncGet(int argc, char **argv)
    {
        NvU32 index = 0;
        bool hasIndex = false;

        for (int i = 0; i < argc; ++i)
        {
            if (std::strcmp(argv[i], "--index") == 0)
            {
                if (i + 1 >= argc || !ParseUint(argv[i + 1], &index))
                {
                    std::printf("Invalid index.\n");
                    return 1;
                }
                hasIndex = true;
                ++i;
                continue;
            }
            std::printf("Unknown option: %s\n", argv[i]);
            return 1;
        }

        if (!hasIndex)
        {
            std::printf("Missing required --index\n");
            return 1;
        }

        NvGSyncDeviceHandle handle = NULL;
        if (!GetGsyncHandleByIndex(index, &handle))
        {
            return 1;
        }

        NvU32 displayCount = 0;
        NvAPI_Status status = NvAPI_GSync_GetTopology(handle, NULL, NULL, &displayCount, NULL);
        if (status != NVAPI_OK)
        {
            PrintNvapiError("NvAPI_GSync_GetTopology failed", status);
            return 1;
        }
        if (displayCount == 0)
        {
            std::printf("No G-Sync displays found.\n");
            return 1;
        }

        std::vector<NV_GSYNC_DISPLAY> displays(displayCount);
        for (NvU32 i = 0; i < displayCount; ++i)
        {
            displays[i].version = NV_GSYNC_DISPLAY_VER;
        }

        status = NvAPI_GSync_GetTopology(handle, NULL, NULL, &displayCount, displays.data());
        if (status != NVAPI_OK)
        {
            PrintNvapiError("NvAPI_GSync_GetTopology failed", status);
            return 1;
        }

        std::printf("G-Sync displays: %u\n", displayCount);
        for (NvU32 i = 0; i < displayCount; ++i)
        {
            std::printf("  display[%u] id=0x%08X state=%s masterable=%u\n",
                i,
                displays[i].displayId,
                GsyncDisplaySyncStateName(displays[i].syncState),
                displays[i].isMasterable ? 1u : 0u);
        }

        return 0;
    }

    int CmdGsyncSyncSet(int argc, char **argv)
    {
        NvU32 index = 0;
        bool hasIndex = false;
        NvU32 displayId = 0;
        bool hasDisplayId = false;
        NVAPI_GSYNC_DISPLAY_SYNC_STATE state = NVAPI_GSYNC_DISPLAY_SYNC_STATE_UNSYNCED;
        bool hasState = false;
        NvU32 flags = 0;

        for (int i = 0; i < argc; ++i)
        {
            if (std::strcmp(argv[i], "--index") == 0)
            {
                if (i + 1 >= argc || !ParseUint(argv[i + 1], &index))
                {
                    std::printf("Invalid index.\n");
                    return 1;
                }
                hasIndex = true;
                ++i;
                continue;
            }
            if (std::strcmp(argv[i], "--display-id") == 0)
            {
                if (i + 1 >= argc || !ParseUint(argv[i + 1], &displayId))
                {
                    std::printf("Invalid display id.\n");
                    return 1;
                }
                hasDisplayId = true;
                ++i;
                continue;
            }
            if (std::strcmp(argv[i], "--state") == 0)
            {
                if (i + 1 >= argc || !ParseGsyncDisplayState(argv[i + 1], &state))
                {
                    std::printf("Invalid --state value.\n");
                    return 1;
                }
                hasState = true;
                ++i;
                continue;
            }
            if (std::strcmp(argv[i], "--no-validate") == 0)
            {
                flags |= NV_SET_SYNC_FLAGS_NO_VALIDATION;
                continue;
            }
            if (std::strcmp(argv[i], "--send-start-event") == 0)
            {
                flags |= NV_SET_SYNC_FLAGS_SEND_START_EVENT;
                continue;
            }
            std::printf("Unknown option: %s\n", argv[i]);
            return 1;
        }

        if (!hasIndex || !hasDisplayId || !hasState)
        {
            std::printf("Missing required --index, --display-id, and --state\n");
            return 1;
        }

        NvGSyncDeviceHandle handle = NULL;
        if (!GetGsyncHandleByIndex(index, &handle))
        {
            return 1;
        }

        NvU32 displayCount = 0;
        NvAPI_Status status = NvAPI_GSync_GetTopology(handle, NULL, NULL, &displayCount, NULL);
        if (status != NVAPI_OK)
        {
            PrintNvapiError("NvAPI_GSync_GetTopology failed", status);
            return 1;
        }
        if (displayCount == 0)
        {
            std::printf("No G-Sync displays found.\n");
            return 1;
        }

        std::vector<NV_GSYNC_DISPLAY> displays(displayCount);
        for (NvU32 i = 0; i < displayCount; ++i)
        {
            displays[i].version = NV_GSYNC_DISPLAY_VER;
        }

        status = NvAPI_GSync_GetTopology(handle, NULL, NULL, &displayCount, displays.data());
        if (status != NVAPI_OK)
        {
            PrintNvapiError("NvAPI_GSync_GetTopology failed", status);
            return 1;
        }

        bool found = false;
        for (NvU32 i = 0; i < displayCount; ++i)
        {
            if (displays[i].displayId == displayId)
            {
                displays[i].syncState = state;
                found = true;
                break;
            }
        }

        if (!found)
        {
            std::printf("DisplayId 0x%08X not found in G-Sync topology.\n", displayId);
            return 1;
        }

        std::printf("G-Sync sync state update: displayId=0x%08X state=%s flags=0x%08X\n",
            displayId,
            GsyncDisplaySyncStateName(state),
            flags);

        status = NvAPI_GSync_SetSyncStateSettings(displayCount, displays.data(), flags);
        if (status != NVAPI_OK)
        {
            PrintNvapiError("NvAPI_GSync_SetSyncStateSettings failed", status);
            return 1;
        }

        std::printf("G-Sync sync state updated.\n");
        return 0;
    }

    int CmdGsyncSyncEnable(int argc, char **argv)
    {
        NvU32 index = 0;
        bool hasIndex = false;
        NvU32 displayId = 0;
        bool hasDisplayId = false;
        NVAPI_GSYNC_DISPLAY_SYNC_STATE state = NVAPI_GSYNC_DISPLAY_SYNC_STATE_MASTER;
        bool hasState = false;

        for (int i = 0; i < argc; ++i)
        {
            if (std::strcmp(argv[i], "--index") == 0)
            {
                if (i + 1 >= argc || !ParseUint(argv[i + 1], &index))
                {
                    std::printf("Invalid index.\n");
                    return 1;
                }
                hasIndex = true;
                ++i;
                continue;
            }
            if (std::strcmp(argv[i], "--display-id") == 0)
            {
                if (i + 1 >= argc || !ParseUint(argv[i + 1], &displayId))
                {
                    std::printf("Invalid display id.\n");
                    return 1;
                }
                hasDisplayId = true;
                ++i;
                continue;
            }
            if (std::strcmp(argv[i], "--state") == 0)
            {
                if (i + 1 >= argc || !ParseGsyncDisplayState(argv[i + 1], &state))
                {
                    std::printf("Invalid --state value.\n");
                    return 1;
                }
                hasState = true;
                ++i;
                continue;
            }
            std::printf("Unknown option: %s\n", argv[i]);
            return 1;
        }

        if (!hasIndex || !hasDisplayId || !hasState)
        {
            std::printf("Missing required --index, --display-id, and/or --state\n");
            return 1;
        }
        if (state == NVAPI_GSYNC_DISPLAY_SYNC_STATE_UNSYNCED)
        {
            std::printf("--state must be master or slave for sync enable.\n");
            return 1;
        }

        NvGSyncDeviceHandle handle = NULL;
        if (!GetGsyncHandleByIndex(index, &handle))
        {
            return 1;
        }

        NvU32 displayCount = 0;
        NvAPI_Status status = NvAPI_GSync_GetTopology(handle, NULL, NULL, &displayCount, NULL);
        if (status != NVAPI_OK)
        {
            PrintNvapiError("NvAPI_GSync_GetTopology failed", status);
            return 1;
        }
        if (displayCount == 0)
        {
            std::printf("No G-Sync displays found.\n");
            return 1;
        }

        std::vector<NV_GSYNC_DISPLAY> displays(displayCount);
        for (NvU32 i = 0; i < displayCount; ++i)
        {
            displays[i].version = NV_GSYNC_DISPLAY_VER;
        }

        status = NvAPI_GSync_GetTopology(handle, NULL, NULL, &displayCount, displays.data());
        if (status != NVAPI_OK)
        {
            PrintNvapiError("NvAPI_GSync_GetTopology failed", status);
            return 1;
        }

        bool found = false;
        bool masterable = false;
        for (NvU32 i = 0; i < displayCount; ++i)
        {
            if (displays[i].displayId == displayId)
            {
                displays[i].syncState = state;
                masterable = displays[i].isMasterable != 0;
                found = true;
                break;
            }
        }

        if (!found)
        {
            std::printf("DisplayId 0x%08X not found in G-Sync topology.\n", displayId);
            return 1;
        }
        if (state == NVAPI_GSYNC_DISPLAY_SYNC_STATE_MASTER && !masterable)
        {
            std::printf("DisplayId 0x%08X is not masterable.\n", displayId);
            return 1;
        }

        std::printf("G-Sync sync state update: displayId=0x%08X state=%s\n",
            displayId,
            GsyncDisplaySyncStateName(state));

        status = NvAPI_GSync_SetSyncStateSettings(displayCount, displays.data(), 0);
        if (status != NVAPI_OK)
        {
            PrintNvapiError("NvAPI_GSync_SetSyncStateSettings failed", status);
            return 1;
        }

        std::printf("G-Sync sync state updated.\n");
        return 0;
    }

    int CmdGsyncSyncDisable(int argc, char **argv)
    {
        NvU32 index = 0;
        bool hasIndex = false;
        NvU32 displayId = 0;
        bool hasDisplayId = false;

        for (int i = 0; i < argc; ++i)
        {
            if (std::strcmp(argv[i], "--index") == 0)
            {
                if (i + 1 >= argc || !ParseUint(argv[i + 1], &index))
                {
                    std::printf("Invalid index.\n");
                    return 1;
                }
                hasIndex = true;
                ++i;
                continue;
            }
            if (std::strcmp(argv[i], "--display-id") == 0)
            {
                if (i + 1 >= argc || !ParseUint(argv[i + 1], &displayId))
                {
                    std::printf("Invalid display id.\n");
                    return 1;
                }
                hasDisplayId = true;
                ++i;
                continue;
            }
            std::printf("Unknown option: %s\n", argv[i]);
            return 1;
        }

        if (!hasIndex || !hasDisplayId)
        {
            std::printf("Missing required --index and/or --display-id\n");
            return 1;
        }

        NvGSyncDeviceHandle handle = NULL;
        if (!GetGsyncHandleByIndex(index, &handle))
        {
            return 1;
        }

        NvU32 displayCount = 0;
        NvAPI_Status status = NvAPI_GSync_GetTopology(handle, NULL, NULL, &displayCount, NULL);
        if (status != NVAPI_OK)
        {
            PrintNvapiError("NvAPI_GSync_GetTopology failed", status);
            return 1;
        }
        if (displayCount == 0)
        {
            std::printf("No G-Sync displays found.\n");
            return 1;
        }

        std::vector<NV_GSYNC_DISPLAY> displays(displayCount);
        for (NvU32 i = 0; i < displayCount; ++i)
        {
            displays[i].version = NV_GSYNC_DISPLAY_VER;
        }

        status = NvAPI_GSync_GetTopology(handle, NULL, NULL, &displayCount, displays.data());
        if (status != NVAPI_OK)
        {
            PrintNvapiError("NvAPI_GSync_GetTopology failed", status);
            return 1;
        }

        bool found = false;
        for (NvU32 i = 0; i < displayCount; ++i)
        {
            if (displays[i].displayId == displayId)
            {
                displays[i].syncState = NVAPI_GSYNC_DISPLAY_SYNC_STATE_UNSYNCED;
                found = true;
                break;
            }
        }

        if (!found)
        {
            std::printf("DisplayId 0x%08X not found in G-Sync topology.\n", displayId);
            return 1;
        }

        std::printf("G-Sync sync state update: displayId=0x%08X state=%s\n",
            displayId,
            GsyncDisplaySyncStateName(NVAPI_GSYNC_DISPLAY_SYNC_STATE_UNSYNCED));

        status = NvAPI_GSync_SetSyncStateSettings(displayCount, displays.data(), 0);
        if (status != NVAPI_OK)
        {
            PrintNvapiError("NvAPI_GSync_SetSyncStateSettings failed", status);
            return 1;
        }

        std::printf("G-Sync sync state updated.\n");
        return 0;
    }

    int CmdGsyncStatus(int argc, char **argv)
    {
        NvU32 index = 0;
        bool hasIndex = false;
        NvU32 gpuIndex = 0;
        bool hasGpuIndex = false;

        for (int i = 0; i < argc; ++i)
        {
            if (std::strcmp(argv[i], "--index") == 0)
            {
                if (i + 1 >= argc || !ParseUint(argv[i + 1], &index))
                {
                    std::printf("Invalid index.\n");
                    return 1;
                }
                hasIndex = true;
                ++i;
                continue;
            }
            if (std::strcmp(argv[i], "--gpu-index") == 0)
            {
                if (i + 1 >= argc || !ParseUint(argv[i + 1], &gpuIndex))
                {
                    std::printf("Invalid gpu index.\n");
                    return 1;
                }
                hasGpuIndex = true;
                ++i;
                continue;
            }
            std::printf("Unknown option: %s\n", argv[i]);
            return 1;
        }

        if (!hasIndex)
        {
            std::printf("Missing required --index\n");
            return 1;
        }

        NvGSyncDeviceHandle handle = NULL;
        if (!GetGsyncHandleByIndex(index, &handle))
        {
            return 1;
        }

        NvU32 gpuCount = 0;
        NvU32 displayCount = 0;
        NvAPI_Status status = NvAPI_GSync_GetTopology(handle, &gpuCount, NULL, &displayCount, NULL);
        if (status != NVAPI_OK)
        {
            PrintNvapiError("NvAPI_GSync_GetTopology failed", status);
            return 1;
        }
        if (gpuCount == 0)
        {
            std::printf("No G-Sync GPUs found.\n");
            return 1;
        }

        std::vector<NV_GSYNC_GPU> gpus(gpuCount);
        for (NvU32 i = 0; i < gpuCount; ++i)
        {
            gpus[i].version = NV_GSYNC_GPU_VER;
        }

        status = NvAPI_GSync_GetTopology(handle, &gpuCount, gpus.data(), &displayCount, NULL);
        if (status != NVAPI_OK)
        {
            PrintNvapiError("NvAPI_GSync_GetTopology failed", status);
            return 1;
        }

        NvPhysicalGpuHandle targetGpu = gpus[0].hPhysicalGpu;
        if (hasGpuIndex)
        {
            std::vector<NvPhysicalGpuHandle> handles;
            std::vector<NvU32> indices;
            if (!CollectGpus(true, gpuIndex, handles, indices))
            {
                return 1;
            }
            if (handles.empty())
            {
                std::printf("GPU index %u not found.\n", gpuIndex);
                return 1;
            }
            targetGpu = handles[0];
        }

        NV_GSYNC_STATUS statusInfo = {};
        statusInfo.version = NV_GSYNC_STATUS_VER;
        status = NvAPI_GSync_GetSyncStatus(handle, targetGpu, &statusInfo);
        if (status != NVAPI_OK)
        {
            PrintNvapiError("NvAPI_GSync_GetSyncStatus failed", status);
            return 1;
        }

        NV_GSYNC_STATUS_PARAMS statusParams = {};
        statusParams.version = NV_GSYNC_STATUS_PARAMS_VER;
        status = NvAPI_GSync_GetStatusParameters(handle, &statusParams);
        if (status != NVAPI_OK)
        {
            PrintNvapiError("NvAPI_GSync_GetStatusParameters failed", status);
            return 1;
        }

        std::printf("G-Sync status:\n");
        std::printf("  synced=%u stereoSynced=%u signalAvailable=%u\n",
            statusInfo.bIsSynced ? 1u : 0u,
            statusInfo.bIsStereoSynced ? 1u : 0u,
            statusInfo.bIsSyncSignalAvailable ? 1u : 0u);
        std::printf("  refreshRate=%u Hz houseSyncIncoming=%u Hz houseSync=%u\n",
            statusParams.refreshRate,
            statusParams.houseSyncIncoming,
            statusParams.bHouseSync ? 1u : 0u);
        std::printf("  internalSlave=%u\n", statusParams.bInternalSlave ? 1u : 0u);
        for (NvU32 i = 0; i < NVAPI_MAX_RJ45_PER_GSYNC; ++i)
        {
            std::printf("  rj45[%u] io=%s ethernet=%u\n",
                i,
                GsyncRj45IoName(statusParams.RJ45_IO[i]),
                statusParams.RJ45_Ethernet[i]);
        }

        return 0;
    }

    int CmdGsyncControlGet(int argc, char **argv)
    {
        NvU32 index = 0;
        bool hasIndex = false;

        for (int i = 0; i < argc; ++i)
        {
            if (std::strcmp(argv[i], "--index") == 0)
            {
                if (i + 1 >= argc || !ParseUint(argv[i + 1], &index))
                {
                    std::printf("Invalid index.\n");
                    return 1;
                }
                hasIndex = true;
                ++i;
                continue;
            }
            std::printf("Unknown option: %s\n", argv[i]);
            return 1;
        }

        if (!hasIndex)
        {
            std::printf("Missing required --index\n");
            return 1;
        }

        NvGSyncDeviceHandle handle = NULL;
        if (!GetGsyncHandleByIndex(index, &handle))
        {
            return 1;
        }

        NV_GSYNC_CONTROL_PARAMS params = {};
        params.version = NV_GSYNC_CONTROL_PARAMS_VER;
        params.syncSkew.version = NV_GSYNC_DELAY_VER;
        params.startupDelay.version = NV_GSYNC_DELAY_VER;

        NvAPI_Status status = NvAPI_GSync_GetControlParameters(handle, &params);
        if (status != NVAPI_OK)
        {
            PrintNvapiError("NvAPI_GSync_GetControlParameters failed", status);
            return 1;
        }

        std::printf("G-Sync control parameters:\n");
        std::printf("  polarity=%s videoMode=%s interval=%u\n",
            GsyncPolarityName(params.polarity),
            GsyncVideoModeName(params.vmode),
            params.interval);
        std::printf("  source=%s interlace=%u syncSourceOutput=%u\n",
            GsyncSyncSourceName(params.source),
            params.interlaceMode ? 1u : 0u,
            params.syncSourceIsOutput ? 1u : 0u);
        std::printf("  skew=%d startDelay=%d\n", params.skew, params.startDelay);
        std::printf("  syncSkew lines=%u pixels=%u\n", params.syncSkew.numLines, params.syncSkew.numPixels);
        std::printf("  startupDelay lines=%u pixels=%u\n", params.startupDelay.numLines, params.startupDelay.numPixels);
        return 0;
    }

    int CmdGsyncControlSet(int argc, char **argv)
    {
        NvU32 index = 0;
        bool hasIndex = false;
        bool hasPolarity = false;
        bool hasVideoMode = false;
        bool hasInterval = false;
        bool hasSource = false;
        bool hasInterlace = false;
        bool hasSyncSourceOutput = false;
        NVAPI_GSYNC_POLARITY polarity = NVAPI_GSYNC_POLARITY_RISING_EDGE;
        NVAPI_GSYNC_VIDEO_MODE videoMode = NVAPI_GSYNC_VIDEO_MODE_NONE;
        NVAPI_GSYNC_SYNC_SOURCE source = NVAPI_GSYNC_SYNC_SOURCE_VSYNC;
        NvU32 interval = 0;
        bool interlace = false;
        bool syncSourceOutput = false;

        for (int i = 0; i < argc; ++i)
        {
            if (std::strcmp(argv[i], "--index") == 0)
            {
                if (i + 1 >= argc || !ParseUint(argv[i + 1], &index))
                {
                    std::printf("Invalid index.\n");
                    return 1;
                }
                hasIndex = true;
                ++i;
                continue;
            }
            if (std::strcmp(argv[i], "--polarity") == 0)
            {
                if (i + 1 >= argc || !ParseGsyncPolarity(argv[i + 1], &polarity))
                {
                    std::printf("Invalid --polarity value.\n");
                    return 1;
                }
                hasPolarity = true;
                ++i;
                continue;
            }
            if (std::strcmp(argv[i], "--video-mode") == 0)
            {
                if (i + 1 >= argc || !ParseGsyncVideoMode(argv[i + 1], &videoMode))
                {
                    std::printf("Invalid --video-mode value.\n");
                    return 1;
                }
                hasVideoMode = true;
                ++i;
                continue;
            }
            if (std::strcmp(argv[i], "--interval") == 0)
            {
                if (i + 1 >= argc || !ParseUint(argv[i + 1], &interval))
                {
                    std::printf("Invalid --interval value.\n");
                    return 1;
                }
                hasInterval = true;
                ++i;
                continue;
            }
            if (std::strcmp(argv[i], "--source") == 0)
            {
                if (i + 1 >= argc || !ParseGsyncSyncSource(argv[i + 1], &source))
                {
                    std::printf("Invalid --source value.\n");
                    return 1;
                }
                hasSource = true;
                ++i;
                continue;
            }
            if (std::strcmp(argv[i], "--interlace") == 0)
            {
                if (i + 1 >= argc || !ParseBoolValue(argv[i + 1], &interlace))
                {
                    std::printf("Invalid --interlace value.\n");
                    return 1;
                }
                hasInterlace = true;
                ++i;
                continue;
            }
            if (std::strcmp(argv[i], "--sync-source-output") == 0)
            {
                if (i + 1 >= argc || !ParseBoolValue(argv[i + 1], &syncSourceOutput))
                {
                    std::printf("Invalid --sync-source-output value.\n");
                    return 1;
                }
                hasSyncSourceOutput = true;
                ++i;
                continue;
            }
            std::printf("Unknown option: %s\n", argv[i]);
            return 1;
        }

        if (!hasIndex)
        {
            std::printf("Missing required --index\n");
            return 1;
        }

        NvGSyncDeviceHandle handle = NULL;
        if (!GetGsyncHandleByIndex(index, &handle))
        {
            return 1;
        }

        NV_GSYNC_CONTROL_PARAMS params = {};
        params.version = NV_GSYNC_CONTROL_PARAMS_VER;
        params.syncSkew.version = NV_GSYNC_DELAY_VER;
        params.startupDelay.version = NV_GSYNC_DELAY_VER;

        NvAPI_Status status = NvAPI_GSync_GetControlParameters(handle, &params);
        if (status != NVAPI_OK)
        {
            PrintNvapiError("NvAPI_GSync_GetControlParameters failed", status);
            return 1;
        }

        if (hasPolarity)
        {
            params.polarity = polarity;
        }
        if (hasVideoMode)
        {
            params.vmode = videoMode;
        }
        if (hasInterval)
        {
            params.interval = interval;
        }
        if (hasSource)
        {
            params.source = source;
        }
        if (hasInterlace)
        {
            params.interlaceMode = interlace ? 1u : 0u;
        }
        if (hasSyncSourceOutput)
        {
            params.syncSourceIsOutput = syncSourceOutput ? 1u : 0u;
        }

        status = NvAPI_GSync_SetControlParameters(handle, &params);
        if (status != NVAPI_OK)
        {
            PrintNvapiError("NvAPI_GSync_SetControlParameters failed", status);
            return 1;
        }

        std::printf("G-Sync control parameters updated.\n");
        return 0;
    }

    int CmdGsyncControl(int argc, char **argv)
    {
        if (argc < 1)
        {
            PrintUsageGroup("gsync");
            return 1;
        }

        static const SubcommandEntry kSubcommands[] = {
            {"get", CmdGsyncControlGet},
            {"set", CmdGsyncControlSet},
        };

        return DispatchSubcommand("gsync control", argc, argv, kSubcommands, sizeof(kSubcommands) / sizeof(kSubcommands[0]), PrintGsyncUsage);
    }

    int CmdGsync(int argc, char **argv)
    {
        if (argc < 1)
        {
            PrintUsageGroup("gsync");
            return 1;
        }

        static const SubcommandEntry kSubcommands[] = {
            {"list", CmdGsyncListAdapter},
            {"caps", CmdGsyncCaps},
            {"topo", CmdGsyncTopology},
            {"sync", CmdGsyncSyncDispatch},
            {"status", CmdGsyncStatus},
            {"control", CmdGsyncControl},
        };

        return DispatchSubcommand("gsync", argc, argv, kSubcommands, sizeof(kSubcommands) / sizeof(kSubcommands[0]), PrintGsyncUsage);
    }
}
