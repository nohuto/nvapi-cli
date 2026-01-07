/*
 * Copyright (c) 2026 Noverse (Nohuto). All rights reserved.
 * Proprietary and confidential. Unauthorized copying or redistribution is strictly prohibited.
 */

#include "commands.h"

namespace nvcli
{
    int CmdDisplayIds(int argc, char **argv)
    {
        NvU32 gpuIndex = 0;
        bool hasIndex = false;
        bool all = false;
        NvU32 flags = 0;
        if (!ParseDisplayIdsArgs(argc, argv, &gpuIndex, &hasIndex, &all, &flags))
        {
            return 1;
        }

        std::vector<NvPhysicalGpuHandle> handles;
        std::vector<NvU32> indices;
        if (!CollectGpus(hasIndex, gpuIndex, handles, indices))
        {
            return 1;
        }

        for (size_t i = 0; i < handles.size(); ++i)
        {
            PrintGpuHeader(indices[i], handles[i]);

            NvU32 count = 0;
            NvAPI_Status status = NVAPI_OK;
            if (all)
            {
                status = NvAPI_GPU_GetAllDisplayIds(handles[i], NULL, &count);
            }
            else
            {
                status = NvAPI_GPU_GetConnectedDisplayIds(handles[i], NULL, &count, flags);
            }
            if (status != NVAPI_OK)
            {
                PrintNvapiError("  NvAPI_GPU_GetDisplayIds failed", status);
                continue;
            }
            if (count == 0)
            {
                std::printf("  No display IDs found.\n");
                continue;
            }

            std::vector<NV_GPU_DISPLAYIDS> displayIds(count);
            for (NvU32 j = 0; j < count; ++j)
            {
                displayIds[j].version = NV_GPU_DISPLAYIDS_VER;
            }

            if (all)
            {
                status = NvAPI_GPU_GetAllDisplayIds(handles[i], displayIds.data(), &count);
            }
            else
            {
                status = NvAPI_GPU_GetConnectedDisplayIds(handles[i], displayIds.data(), &count, flags);
            }
            if (status != NVAPI_OK)
            {
                PrintNvapiError("  NvAPI_GPU_GetDisplayIds failed", status);
                continue;
            }

            for (NvU32 j = 0; j < count; ++j)
            {
                const NV_GPU_DISPLAYIDS &info = displayIds[j];
                std::printf("  displayId=0x%08X connector=%s active=%u connected=%u osVisible=%u\n",
                    info.displayId,
                    ConnectorTypeName(info.connectorType),
                    info.isActive,
                    info.isConnected,
                    info.isOSVisible);
            }
        }

        return 0;
    }

    int CmdDisplayEdid(int argc, char **argv)
    {
        NvU32 displayId = 0;
        if (!ParseDisplayIdArg(argc, argv, &displayId))
        {
            return 1;
        }

        NV_EDID_FLAG flag = NV_EDID_FLAG_DEFAULT;
        if (!ParseEdidFlagArg(argc, argv, &flag))
        {
            return 1;
        }

        NvU8 edidData[NV_EDID_DATA_SIZE_MAX] = {};
        NvU32 size = NV_EDID_DATA_SIZE_MAX;
        NvAPI_Status status = NvAPI_GPU_GetEdidEx2(displayId, &flag, edidData, &size);
        if (status != NVAPI_OK)
        {
            PrintNvapiError("NvAPI_GPU_GetEdidEx2 failed", status);
            return 1;
        }

        std::printf("EDID for displayId=0x%08X size=%u flag=%u\n", displayId, size, flag);
        if (size >= 128)
        {
            NvU8 extensionCount = edidData[0x7E];
            std::printf("Extensions: %u\n", extensionCount);
        }
        PrintHexBytes(edidData, size);
        return 0;
    }

    int CmdDisplayTiming(int argc, char **argv)
    {
        NvU32 displayId = 0;
        if (!ParseDisplayIdArg(argc, argv, &displayId))
        {
            return 1;
        }

        NvU32 count = 0;
        NvAPI_Status status = NvAPI_DISP_GetTimingInfo(displayId, &count, NULL);
        if (status != NVAPI_OK)
        {
            PrintNvapiError("NvAPI_DISP_GetTimingInfo failed", status);
            return 1;
        }
        if (count == 0)
        {
            std::printf("No timing info available.\n");
            return 0;
        }

        std::vector<NV_BACKEND_TIMING_INFO> timings(count);
        for (NvU32 i = 0; i < count; ++i)
        {
            timings[i].version = NV_BACKEND_TIMING_INFO_VER;
        }

        status = NvAPI_DISP_GetTimingInfo(displayId, &count, timings.data());
        if (status != NVAPI_OK)
        {
            PrintNvapiError("NvAPI_DISP_GetTimingInfo failed", status);
            return 1;
        }

        std::printf("Timing info for displayId=0x%08X entries=%u\n", displayId, count);
        for (NvU32 i = 0; i < count; ++i)
        {
            const NV_TIMING &timing = timings[i].timingInfo;
            double refresh = TimingRefreshHz(timing);
            std::printf("  [%u] %ux%u %s pclk=%.3f MHz refresh=%.2f Hz\n",
                i,
                timing.HVisible,
                timing.VVisible,
                timing.interlaced ? "interlaced" : "progressive",
                static_cast<double>(timing.pclk) / 100.0,
                refresh);
        }
        return 0;
}
}
