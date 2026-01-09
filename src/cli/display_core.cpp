/*
 * Copyright (c) 2026 Noverse (Nohuto). All rights reserved.
 * Proprietary and confidential. Unauthorized copying or redistribution is strictly prohibited.
 */

#include "commands.h"

namespace nvcli
{
    int CmdDisplayList()
    {
        NvU32 index = 0;
        NvDisplayHandle handle = NULL;
        while (NvAPI_EnumNvidiaDisplayHandle(index, &handle) == NVAPI_OK)
        {
            NvAPI_ShortString name = {0};
            NvAPI_Status status = NvAPI_GetAssociatedNvidiaDisplayName(handle, name);
            if (status != NVAPI_OK)
            {
                strncpy_s(name, sizeof(name), "<name unavailable>", _TRUNCATE);
            }

            NvU32 outputId = 0;
            status = NvAPI_GetAssociatedDisplayOutputId(handle, &outputId);
            if (status == NVAPI_OK)
            {
                std::printf("[%u] handle=0x%p name=%s output=0x%08X\n",
                    index, handle, name, outputId);
            }
            else
            {
                std::printf("[%u] handle=0x%p name=%s\n", index, handle, name);
            }
            ++index;
        }

        if (index == 0)
        {
            std::printf("No NVIDIA displays found.\n");
        }
        return 0;
    }

    int CmdDisplayGet(int argc, char **argv)
    {
        NvU32 handleIndex = 0;
        for (int i = 0; i < argc; ++i)
        {
            if (std::strcmp(argv[i], "--handle-index") == 0)
            {
                if (i + 1 >= argc)
                {
                    std::printf("Missing value for --handle-index\n");
                    return 1;
                }
                if (!ParseUint(argv[i + 1], &handleIndex))
                {
                    std::printf("Invalid handle index: %s\n", argv[i + 1]);
                    return 1;
                }
                ++i;
            }
            else
            {
                std::printf("Unknown option: %s\n", argv[i]);
                return 1;
            }
        }

        NvDisplayHandle handle = NULL;
        if (!GetDisplayHandleByIndex(handleIndex, &handle))
        {
            std::printf("Display handle index %u not found.\n", handleIndex);
            return 1;
        }

        NV_DISP_PATH paths[kMaxDisplayPaths] = {};
        for (NvU32 i = 0; i < kMaxDisplayPaths; ++i)
        {
            paths[i].version = NV_DISP_PATH_VER;
        }

        NvU32 pathCount = kMaxDisplayPaths;
        NvAPI_Status status = NvAPI_GetDisplaySettings(handle, paths, &pathCount);
        if (status != NVAPI_OK)
        {
            PrintNvapiError("NvAPI_GetDisplaySettings failed", status);
            return 1;
        }

        std::printf("Display paths: %u\n", pathCount);
        for (NvU32 i = 0; i < pathCount; ++i)
        {
            std::printf("  [%u] srcID=%u device=0x%08X\n", i, paths[i].srcID, paths[i].device);
        }
        return 0;
    }

    int CmdDisplaySet(int argc, char **argv)
    {
        NvU32 handleIndex = 0;
        std::vector<NV_DISP_PATH> paths;
        paths.reserve(kMaxDisplayPaths);

        for (int i = 0; i < argc; ++i)
        {
            if (std::strcmp(argv[i], "--handle-index") == 0)
            {
                if (i + 1 >= argc)
                {
                    std::printf("Missing value for --handle-index\n");
                    return 1;
                }
                if (!ParseUint(argv[i + 1], &handleIndex))
                {
                    std::printf("Invalid handle index: %s\n", argv[i + 1]);
                    return 1;
                }
                ++i;
                continue;
            }
            NvU32 srcId = 0;
            NvU32 device = 0;
            if (!ParseSrcDevicePair(argv[i], &srcId, &device))
            {
                std::printf("Invalid path format: %s (expected srcId:device)\n", argv[i]);
                return 1;
            }

            NV_DISP_PATH path = {};
            path.version = NV_DISP_PATH_VER;
            path.srcID = srcId;
            path.device = device;
            paths.push_back(path);
        }

        if (paths.empty())
        {
            std::printf("No display paths provided.\n");
            return 1;
        }

        NvDisplayHandle handle = NULL;
        if (!GetDisplayHandleByIndex(handleIndex, &handle))
        {
            std::printf("Display handle index %u not found.\n", handleIndex);
            return 1;
        }

        std::printf("Requested display paths:\n");
        for (size_t i = 0; i < paths.size(); ++i)
        {
            std::printf("  [%u] srcID=%u device=0x%08X\n",
                static_cast<unsigned>(i),
                paths[i].srcID,
                paths[i].device);
        }

        NvAPI_Status status = NvAPI_SetDisplaySettings(handle, &paths[0], static_cast<NvU32>(paths.size()));
        if (status != NVAPI_OK)
        {
            PrintNvapiError("NvAPI_SetDisplaySettings failed", status);
            return 1;
        }

        std::printf("Display settings applied.\n");
        return 0;
    }
}
