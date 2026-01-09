/*
 * Copyright (c) 2026 Noverse (Nohuto). All rights reserved.
 * Proprietary and confidential. Unauthorized copying or redistribution is strictly prohibited.
 */

#include "commands.h"

namespace nvcli
{
    namespace
    {
        int CmdMosaicCurrentAdapter(int argc, char **argv)
        {
            (void)argc;
            (void)argv;
            return CmdMosaicCurrent();
        }

        void PrintMosaicUsage()
        {
            PrintUsageGroup("mosaic");
        }
    }
    int CmdMosaicCaps(int argc, char **argv)
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

            NV_MOSAIC_CAPS caps = {};
            caps.version = NV_MOSAIC_CAPS_VER;
            NvAPI_Status status = NvAPI_Mosaic_GetSingleGpuMosaicCaps(handles[i], &caps);
            if (status != NVAPI_OK)
            {
                PrintNvapiError("  NvAPI_Mosaic_GetSingleGpuMosaicCaps failed", status);
                continue;
            }

            std::printf("  Mosaic caps:\n");
            std::printf("    supportsMosaic=%u supportsImmersiveGaming=%u supportsPassiveStereo=%u\n",
                caps.bSupportsMosaic ? 1 : 0,
                caps.bSupportsImmersiveGaming ? 1 : 0,
                caps.bSupportsPassiveStereo ? 1 : 0);
            std::printf("    supportsOverlap=%u supportsGaps=%u supportsUnevenGapping=%u\n",
                caps.bSupportsOverlap ? 1 : 0,
                caps.bSupportsGaps ? 1 : 0,
                caps.bSupportsUnevenGapping ? 1 : 0);
            std::printf("    supportsPerDisplaySettings=%u supportsPerDisplayRotation=%u supportsGlobalRotation=%u\n",
                caps.bSupportsPerDisplaySettings ? 1 : 0,
                caps.bSupportsPerDisplayRotation ? 1 : 0,
                caps.bSupportsGlobalRotation ? 1 : 0);
            std::printf("    supportsPixelShift=%u supports4QuadrantPixelShift=%u supportsPerDisplayMirroring=%u\n",
                caps.bSupportsPixelShift ? 1 : 0,
                caps.bSupports4QuadrantPixelShift ? 1 : 0,
                caps.bSupportsPerDisplayMirroring ? 1 : 0);
        }

        return 0;
    }

    int CmdMosaicSupported(int argc, char **argv)
    {
        NV_MOSAIC_TOPO_TYPE type = NV_MOSAIC_TOPO_TYPE_ALL;
        NvU32 limit = 10;
        for (int i = 0; i < argc; ++i)
        {
            if (std::strcmp(argv[i], "--type") == 0)
            {
                if (i + 1 >= argc)
                {
                    std::printf("Missing value for --type\n");
                    return 1;
                }
                std::string value = ToLowerAscii(argv[i + 1]);
                if (value == "all")
                {
                    type = NV_MOSAIC_TOPO_TYPE_ALL;
                }
                else if (value == "basic")
                {
                    type = NV_MOSAIC_TOPO_TYPE_BASIC;
                }
                else if (value == "passive-stereo")
                {
                    type = NV_MOSAIC_TOPO_TYPE_PASSIVE_STEREO;
                }
                else if (value == "scaled-clone")
                {
                    type = NV_MOSAIC_TOPO_TYPE_SCALED_CLONE;
                }
                else if (value == "passive-stereo-scaled-clone")
                {
                    type = NV_MOSAIC_TOPO_TYPE_PASSIVE_STEREO_SCALED_CLONE;
                }
                else
                {
                    std::printf("Unknown type: %s\n", argv[i + 1]);
                    return 1;
                }
                ++i;
                continue;
            }
            if (std::strcmp(argv[i], "--limit") == 0)
            {
                if (i + 1 >= argc)
                {
                    std::printf("Missing value for --limit\n");
                    return 1;
                }
                if (!ParseUint(argv[i + 1], &limit))
                {
                    std::printf("Invalid limit: %s\n", argv[i + 1]);
                    return 1;
                }
                ++i;
                continue;
            }
            std::printf("Unknown option: %s\n", argv[i]);
            return 1;
        }

        NV_MOSAIC_SUPPORTED_TOPO_INFO info = {};
        info.version = NVAPI_MOSAIC_SUPPORTED_TOPO_INFO_VER;
        NvAPI_Status status = NvAPI_Mosaic_GetSupportedTopoInfo(&info, type);
        if (status != NVAPI_OK)
        {
            PrintNvapiError("NvAPI_Mosaic_GetSupportedTopoInfo failed", status);
            return 1;
        }

        std::printf("Mosaic supported topologies (type=%s): %u\n",
            MosaicTopoTypeName(type),
            info.topoBriefsCount);
        for (NvU32 i = 0; i < info.topoBriefsCount; ++i)
        {
            const NV_MOSAIC_TOPO_BRIEF &brief = info.topoBriefs[i];
            std::printf("  topo=0x%08X (%s) enabled=%u possible=%u\n",
                brief.topo,
                MosaicTopoName(brief.topo),
                brief.enabled,
                brief.isPossible);
        }

        std::printf("Display settings: %u\n", info.displaySettingsCount);
        NvU32 displayLimit = info.displaySettingsCount < limit ? info.displaySettingsCount : limit;
        for (NvU32 i = 0; i < displayLimit; ++i)
        {
            const NV_MOSAIC_DISPLAY_SETTING &setting = info.displaySettings[i];
            std::printf("  [%u] %ux%u bpp=%u freq=%u rrx1k=%u\n",
                i,
                setting.width,
                setting.height,
                setting.bpp,
                setting.freq,
                setting.rrx1k);
        }
        if (info.displaySettingsCount > displayLimit)
        {
            std::printf("  ... (use --limit to show more)\n");
        }

        return 0;
    }

    int CmdMosaicCurrent()
    {
        NV_MOSAIC_TOPO_BRIEF topo = {};
        topo.version = NVAPI_MOSAIC_TOPO_BRIEF_VER;
        NV_MOSAIC_DISPLAY_SETTING display = {};
        display.version = NVAPI_MOSAIC_DISPLAY_SETTING_VER;
        NvS32 overlapX = 0;
        NvS32 overlapY = 0;

        NvAPI_Status status = NvAPI_Mosaic_GetCurrentTopo(&topo, &display, &overlapX, &overlapY);
        if (status != NVAPI_OK)
        {
            PrintNvapiError("NvAPI_Mosaic_GetCurrentTopo failed", status);
            return 1;
        }

        std::printf("Current mosaic topology:\n");
        std::printf("  topo=0x%08X (%s) enabled=%u possible=%u\n",
            topo.topo,
            MosaicTopoName(topo.topo),
            topo.enabled,
            topo.isPossible);
        std::printf("  display=%ux%u bpp=%u freq=%u rrx1k=%u overlapX=%d overlapY=%d\n",
            display.width,
            display.height,
            display.bpp,
            display.freq,
            display.rrx1k,
            overlapX,
            overlapY);
        return 0;
    }

    int CmdMosaicEnable(int argc, char **argv)
    {
        bool state = false;
        bool hasState = false;

        for (int i = 0; i < argc; ++i)
        {
            if (std::strcmp(argv[i], "--state") == 0)
            {
                if (i + 1 >= argc)
                {
                    std::printf("Missing value for --state\n");
                    return 1;
                }
                if (!ParseBoolValue(argv[i + 1], &state))
                {
                    std::printf("Invalid state: %s\n", argv[i + 1]);
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
            std::printf("Missing required --state 0|1\n");
            return 1;
        }

        std::printf("Mosaic enable request: %s\n", state ? "on" : "off");
        NvAPI_Status status = NvAPI_Mosaic_EnableCurrentTopo(state ? 1 : 0);
        if (status != NVAPI_OK)
        {
            PrintNvapiError("NvAPI_Mosaic_EnableCurrentTopo failed", status);
            return 1;
        }

        std::printf("Mosaic state updated.\n");
        return 0;
    }

    int CmdMosaicDisplayCaps(int argc, char **argv)
    {
        NvU32 limit = 10;
        for (int i = 0; i < argc; ++i)
        {
            if (std::strcmp(argv[i], "--limit") == 0)
            {
                if (i + 1 >= argc)
                {
                    std::printf("Missing value for --limit\n");
                    return 1;
                }
                if (!ParseUint(argv[i + 1], &limit))
                {
                    std::printf("Invalid limit: %s\n", argv[i + 1]);
                    return 1;
                }
                ++i;
                continue;
            }
            std::printf("Unknown option: %s\n", argv[i]);
            return 1;
        }

        NV_MOSAIC_DISPLAY_CAPS caps = {};
        caps.version = NV_MOSAIC_DISPLAY_CAPS_VER;
        NvAPI_Status status = NvAPI_Mosaic_GetDisplayCapabilities(&caps);
        if (status != NVAPI_OK)
        {
            PrintNvapiError("NvAPI_Mosaic_GetDisplayCapabilities failed", status);
            return 1;
        }

        std::printf("Mosaic display capabilities: %u settings\n", caps.displaySettingsCount);
        NvU32 displayLimit = caps.displaySettingsCount < limit ? caps.displaySettingsCount : limit;
        for (NvU32 i = 0; i < displayLimit; ++i)
        {
            const NV_MOSAIC_DISPLAY_SETTING &setting = caps.displaySettings[i];
            std::printf("  [%u] %ux%u bpp=%u freq=%u rrx1k=%u\n",
                i,
                setting.width,
                setting.height,
                setting.bpp,
                setting.freq,
                setting.rrx1k);
        }
        if (caps.displaySettingsCount > displayLimit)
        {
            std::printf("  ... (use --limit to show more)\n");
        }
        return 0;
    }

    int CmdMosaic(int argc, char **argv)
    {
        if (argc < 1)
        {
            PrintUsageGroup("mosaic");
            return 1;
        }

        static const SubcommandEntry kSubcommands[] = {
            {"caps", CmdMosaicCaps},
            {"supported", CmdMosaicSupported},
            {"current", CmdMosaicCurrentAdapter},
            {"enable", CmdMosaicEnable},
            {"display-caps", CmdMosaicDisplayCaps},
        };

        return DispatchSubcommand("mosaic", argc, argv, kSubcommands, sizeof(kSubcommands) / sizeof(kSubcommands[0]), PrintMosaicUsage);
}
}

