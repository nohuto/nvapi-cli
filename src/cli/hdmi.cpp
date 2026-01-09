/*
 * Copyright (c) 2026 Noverse (Nohuto). All rights reserved.
 * Proprietary and confidential. Unauthorized copying or redistribution is strictly prohibited.
 */

#include "commands.h"

namespace nvcli
{
    int CmdHdmiSupport(int argc, char **argv)
    {
        NvU32 handleIndex = 0;
        NvU32 outputId = 0;
        bool hasOutputId = false;

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
            if (std::strcmp(argv[i], "--output-id") == 0)
            {
                if (i + 1 >= argc)
                {
                    std::printf("Missing value for --output-id\n");
                    return 1;
                }
                if (!ParseUint(argv[i + 1], &outputId))
                {
                    std::printf("Invalid output id: %s\n", argv[i + 1]);
                    return 1;
                }
                hasOutputId = true;
                ++i;
                continue;
            }
            std::printf("Unknown option: %s\n", argv[i]);
            return 1;
        }

        NvDisplayHandle handle = NULL;
        if (!GetDisplayHandleByIndex(handleIndex, &handle))
        {
            std::printf("Display handle index %u not found.\n", handleIndex);
            return 1;
        }

        if (!hasOutputId)
        {
            NvAPI_Status status = NvAPI_GetAssociatedDisplayOutputId(handle, &outputId);
            if (status != NVAPI_OK)
            {
                PrintNvapiError("NvAPI_GetAssociatedDisplayOutputId failed", status);
                return 1;
            }
        }

        NV_HDMI_SUPPORT_INFO info = {};
        info.version = NV_HDMI_SUPPORT_INFO_VER;
        NvAPI_Status status = NvAPI_GetHDMISupportInfo(handle, outputId, &info);
        if (status != NVAPI_OK)
        {
            PrintNvapiError("NvAPI_GetHDMISupportInfo failed", status);
            return 1;
        }

        std::printf("HDMI support:\n");
        std::printf("  gpuCapable=%u monHdmi=%u monAudio=%u monUnderscan=%u\n",
            info.isGpuHDMICapable ? 1 : 0,
            info.isMonHDMI ? 1 : 0,
            info.isMonBasicAudioCapable ? 1 : 0,
            info.isMonUnderscanCapable ? 1 : 0);
        std::printf("  rgb444=%u ycbcr444=%u ycbcr422=%u ycbcr420=%u\n",
            info.isMonRgb444Capable ? 1 : 0,
            info.isMonYCbCr444Capable ? 1 : 0,
            info.isMonYCbCr422Capable ? 1 : 0,
            info.isMonYCbCr420Capable ? 1 : 0);
        std::printf("  rgb444Current=%u ycbcr444Current=%u ycbcr422Current=%u ycbcr420Current=%u\n",
            info.isRgb444SupportedOnCurrentMode ? 1 : 0,
            info.isYCbCr444SupportedOnCurrentMode ? 1 : 0,
            info.isYCbCr422SupportedOnCurrentMode ? 1 : 0,
            info.isYCbCr420SupportedOnCurrentMode ? 1 : 0);
        std::printf("  bpc: 8=%u 10=%u 12=%u 16=%u\n",
            info.is8BPCSupported ? 1 : 0,
            info.is10BPCSupported ? 1 : 0,
            info.is12BPCSupported ? 1 : 0,
            info.is16BPCSupported ? 1 : 0);
        std::printf("  bpcCurrent: 8=%u 10=%u 12=%u 16=%u\n",
            info.is8BPCSupportedOnCurrentMode ? 1 : 0,
            info.is10BPCSupportedOnCurrentMode ? 1 : 0,
            info.is12BPCSupportedOnCurrentMode ? 1 : 0,
            info.is16BPCSupportedOnCurrentMode ? 1 : 0);
        std::printf("  hdrGamma=%u st2084=%u gpuFrl=%u edid861ExtRev=%u\n",
            info.isHdrGammaSupported ? 1 : 0,
            info.isST2084EotfSupported ? 1 : 0,
            info.isGpuFrlCapable ? 1 : 0,
            info.EDID861ExtRev);
        std::printf("  frl: maxMon=%s maxMonAssessed=%s maxMonDsc=%s maxGpu=%s\n",
            HdmiFrlRateName(info.maxMonFrlRate),
            HdmiFrlRateName(info.maxMonAsessedFrlRate),
            HdmiFrlRateName(info.maxMonDscFrlRate),
            HdmiFrlRateName(info.maxGpuFrlRate));
        return 0;
    }

    int CmdHdmiHdcpDiag(int argc, char **argv)
    {
        NvU32 index = 0;
        bool hasIndex = false;
        NvU32 displayId = 0;
        bool hasDisplayId = false;

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
            if (std::strcmp(argv[i], "--id") == 0)
            {
                if (i + 1 >= argc)
                {
                    std::printf("Missing value for --id\n");
                    return 1;
                }
                if (!ParseUint(argv[i + 1], &displayId))
                {
                    std::printf("Invalid display id: %s\n", argv[i + 1]);
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
            std::printf("Missing required --index and --id\n");
            return 1;
        }

        std::vector<NvPhysicalGpuHandle> handles;
        std::vector<NvU32> indices;
        if (!CollectGpus(true, index, handles, indices))
        {
            return 1;
        }

        NV_HDCP_HDMI_DIAGNOSTICS diag = {};
        diag.version = NV_GET_HDCP_HDMI_DIAGNOSTICS_VER;
        NvAPI_Status status = NvAPI_GetHdcpHdmiDiagnostics(handles[0], displayId, &diag);
        if (status != NVAPI_OK)
        {
            PrintNvapiError("NvAPI_GetHdcpHdmiDiagnostics failed", status);
            return 1;
        }

        std::printf("HDCP/HDMI diagnostics:\n");
        std::printf("  HDCP: gpuCapable=%u fusePresent=%u dfpCapable=%u dfpAllowed=%u dualLink=%u\n",
            diag.hdcp.bGpuCapable ? 1 : 0,
            diag.hdcp.bGpuFusePresent ? 1 : 0,
            diag.hdcp.bDfpCapable ? 1 : 0,
            diag.hdcp.bDfpAllowed ? 1 : 0,
            diag.hdcp.bDfpDualLink ? 1 : 0);
        std::printf("  HDCP: romError=%u downstreamChecksumError=%u\n",
            diag.hdcp.bRomError ? 1 : 0,
            diag.hdcp.bDownstreamChecksumError ? 1 : 0);
        std::printf("  HDMI: gpuCapable=%u dfpCapable=%u dfpAllowed=%u dualLink=%u\n",
            diag.hdmi.bGpuCapable ? 1 : 0,
            diag.hdmi.bDfpCapable ? 1 : 0,
            diag.hdmi.bDfpAllowed ? 1 : 0,
            diag.hdmi.bDfpDualLink ? 1 : 0);
        return 0;
    }

    int CmdHdmiStereoModes(int argc, char **argv)
    {
        NvU32 displayId = 0;
        bool hasDisplayId = false;
        NvU16 start = 0;
        NvU16 count = 16;
        bool passThrough = false;
        bool hasDimensions = false;
        NvU32 width = 0;
        NvU32 height = 0;
        NvU32 refresh = 0;

        for (int i = 0; i < argc; ++i)
        {
            if (std::strcmp(argv[i], "--id") == 0)
            {
                if (i + 1 >= argc)
                {
                    std::printf("Missing value for --id\n");
                    return 1;
                }
                if (!ParseUint(argv[i + 1], &displayId))
                {
                    std::printf("Invalid display id: %s\n", argv[i + 1]);
                    return 1;
                }
                hasDisplayId = true;
                ++i;
                continue;
            }
            if (std::strcmp(argv[i], "--start") == 0)
            {
                NvU32 temp = 0;
                if (i + 1 >= argc || !ParseUint(argv[i + 1], &temp))
                {
                    std::printf("Invalid start.\n");
                    return 1;
                }
                start = static_cast<NvU16>(temp);
                ++i;
                continue;
            }
            if (std::strcmp(argv[i], "--count") == 0)
            {
                NvU32 temp = 0;
                if (i + 1 >= argc || !ParseUint(argv[i + 1], &temp))
                {
                    std::printf("Invalid count.\n");
                    return 1;
                }
                count = static_cast<NvU16>(temp);
                ++i;
                continue;
            }
            if (std::strcmp(argv[i], "--pass-through") == 0)
            {
                passThrough = true;
                continue;
            }
            if (std::strcmp(argv[i], "--width") == 0)
            {
                if (i + 1 >= argc || !ParseUint(argv[i + 1], &width))
                {
                    std::printf("Invalid width.\n");
                    return 1;
                }
                hasDimensions = true;
                ++i;
                continue;
            }
            if (std::strcmp(argv[i], "--height") == 0)
            {
                if (i + 1 >= argc || !ParseUint(argv[i + 1], &height))
                {
                    std::printf("Invalid height.\n");
                    return 1;
                }
                hasDimensions = true;
                ++i;
                continue;
            }
            if (std::strcmp(argv[i], "--refresh") == 0)
            {
                if (i + 1 >= argc || !ParseUint(argv[i + 1], &refresh))
                {
                    std::printf("Invalid refresh.\n");
                    return 1;
                }
                hasDimensions = true;
                ++i;
                continue;
            }
            std::printf("Unknown option: %s\n", argv[i]);
            return 1;
        }

        if (!hasDisplayId)
        {
            std::printf("Missing required --id\n");
            return 1;
        }

        NV_HDMI_STEREO_MODES_LIST modes = {};
        modes.version = NV_HDMI_STEREO_MODES_LIST_VER;
        modes.displayId = displayId;
        modes.enumIndex = start;
        modes.count = count;
        modes.bPassThroughMode = passThrough ? 1 : 0;
        if (hasDimensions)
        {
            modes.bMatchDimension = 1;
            modes.bMatchRR = 1;
            modes.width = width;
            modes.height = height;
            modes.refreshRate = refresh;
        }

        NvAPI_Status status = NvAPI_DISP_EnumHDMIStereoModes(&modes);
        if (status != NVAPI_OK && status != NVAPI_END_ENUMERATION)
        {
            PrintNvapiError("NvAPI_DISP_EnumHDMIStereoModes failed", status);
            return 1;
        }

        std::printf("HDMI stereo modes: %u\n", modes.numberOfModes);
        for (NvU16 i = 0; i < modes.numberOfModes; ++i)
        {
            const NV_HDMI_STEREO_MODE &mode = modes.modeList[i];
            std::printf("  [%u] %ux%u rr=%u type=0x%02X\n",
                i,
                mode.HVisible,
                mode.VVisible,
                mode.rr,
                mode.stereoType);
        }
        return 0;
    }

    int CmdHdmiStereoGet(int argc, char **argv)
    {
        NvU32 displayId = 0;
        bool hasDisplayId = false;
        for (int i = 0; i < argc; ++i)
        {
            if (std::strcmp(argv[i], "--id") == 0)
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
        if (!hasDisplayId)
        {
            std::printf("Missing required --id\n");
            return 1;
        }

        NV_HDMI_STEREO_SETTINGS settings = {};
        settings.version = NV_HDMI_STEREO_SETTINGS_VER;
        NvAPI_Status status = NvAPI_DISP_GetHDMIStereoSettings(displayId, &settings);
        if (status != NVAPI_OK)
        {
            PrintNvapiError("NvAPI_DISP_GetHDMIStereoSettings failed", status);
            return 1;
        }

        std::printf("HDMI stereo settings: type=0x%02X\n", settings.type);
        return 0;
    }

    bool ParseHdmiStereoType(const char *value, NV_HDMI_STEREO_TYPE *outType)
    {
        if (!value || !outType)
        {
            return false;
        }
        std::string lowered = ToLowerAscii(value);
        if (lowered == "none")
        {
            *outType = NV_HDMI_STEREO_3D_NONE;
            return true;
        }
        if (lowered == "frame-packing")
        {
            *outType = NV_HDMI_STEREO_3D_FRAME_PACKING;
            return true;
        }
        if (lowered == "top-bottom")
        {
            *outType = NV_HDMI_STEREO_3D_TOP_BOTTOM;
            return true;
        }
        if (lowered == "side-by-side-full")
        {
            *outType = NV_HDMI_STEREO_3D_SIDE_BY_SIDE_FULL;
            return true;
        }
        if (lowered == "side-by-side-half")
        {
            *outType = NV_HDMI_STEREO_3D_SIDE_BY_SIDE_HALF_HORIZONTAIL;
            return true;
        }
        NvU32 numeric = 0;
        if (!ParseUint(value, &numeric))
        {
            return false;
        }
        *outType = static_cast<NV_HDMI_STEREO_TYPE>(numeric);
        return true;
    }

    int CmdHdmiStereoSet(int argc, char **argv)
    {
        NvU32 displayId = 0;
        bool hasDisplayId = false;
        NV_HDMI_STEREO_TYPE type = NV_HDMI_STEREO_3D_NONE;
        bool hasType = false;

        for (int i = 0; i < argc; ++i)
        {
            if (std::strcmp(argv[i], "--id") == 0)
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
            if (std::strcmp(argv[i], "--type") == 0)
            {
                if (i + 1 >= argc || !ParseHdmiStereoType(argv[i + 1], &type))
                {
                    std::printf("Invalid HDMI stereo type.\n");
                    return 1;
                }
                hasType = true;
                ++i;
                continue;
            }
            std::printf("Unknown option: %s\n", argv[i]);
            return 1;
        }

        if (!hasDisplayId || !hasType)
        {
            std::printf("Missing required --id and --type\n");
            return 1;
        }

        std::printf("HDMI stereo set: displayId=0x%08X type=0x%02X\n", displayId, type);
        NV_HDMI_STEREO_SETTINGS settings = {};
        settings.version = NV_HDMI_STEREO_SETTINGS_VER;
        settings.type = type;
        NvAPI_Status status = NvAPI_DISP_SetHDMIStereoSettings(displayId, &settings);
        if (status != NVAPI_OK)
        {
            PrintNvapiError("NvAPI_DISP_SetHDMIStereoSettings failed", status);
            return 1;
        }

        std::printf("HDMI stereo settings updated.\n");
        return 0;
    }

    int CmdHdmiAudioMute(int argc, char **argv)
    {
        NvU32 handleIndex = 0;
        NvU32 outputId = 0;
        bool hasOutputId = false;
        bool hasState = false;
        bool mute = false;

        for (int i = 0; i < argc; ++i)
        {
            if (std::strcmp(argv[i], "--handle-index") == 0)
            {
                if (i + 1 >= argc || !ParseUint(argv[i + 1], &handleIndex))
                {
                    std::printf("Invalid handle index.\n");
                    return 1;
                }
                ++i;
                continue;
            }
            if (std::strcmp(argv[i], "--output-id") == 0)
            {
                if (i + 1 >= argc || !ParseUint(argv[i + 1], &outputId))
                {
                    std::printf("Invalid output id.\n");
                    return 1;
                }
                hasOutputId = true;
                ++i;
                continue;
            }
            if (std::strcmp(argv[i], "--state") == 0)
            {
                if (i + 1 >= argc || !ParseBoolValue(argv[i + 1], &mute))
                {
                    std::printf("Invalid state.\n");
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

        NvDisplayHandle handle = NULL;
        if (!GetDisplayHandleByIndex(handleIndex, &handle))
        {
            std::printf("Display handle index %u not found.\n", handleIndex);
            return 1;
        }

        if (!hasOutputId)
        {
            NvAPI_Status status = NvAPI_GetAssociatedDisplayOutputId(handle, &outputId);
            if (status != NVAPI_OK)
            {
                PrintNvapiError("NvAPI_GetAssociatedDisplayOutputId failed", status);
                return 1;
            }
        }

        std::printf("HDMI audio mute: state=%s outputId=0x%08X\n", mute ? "on" : "off", outputId);
        NV_HDMI_AUDIO_INFO info = {};
        info.version = NV_HDMI_AUDIO_INFO_VER;
        info.nvHdmiAudioMute = mute ? NV_SET_HDMI_AUDIO_STREAM_MUTE_ON : NV_SET_HDMI_AUDIO_STREAM_MUTE_OFF;
        NvAPI_Status status = NvAPI_SetHDMIAudioStreamMute(handle, outputId, &info);
        if (status != NVAPI_OK)
        {
            PrintNvapiError("NvAPI_SetHDMIAudioStreamMute failed", status);
            return 1;
        }

        std::printf("HDMI audio mute updated.\n");
        return 0;
    }

    namespace
    {
        void PrintHdmiUsage();

        int CmdHdmiStereoDispatch(int argc, char **argv)
        {
            static const SubcommandEntry kStereoSubcommands[] = {
                {"modes", CmdHdmiStereoModes},
                {"get", CmdHdmiStereoGet},
                {"set", CmdHdmiStereoSet},
            };

            return DispatchSubcommand("hdmi stereo", argc, argv, kStereoSubcommands, sizeof(kStereoSubcommands) / sizeof(kStereoSubcommands[0]), PrintHdmiUsage);
        }

        void PrintHdmiUsage()
        {
            PrintUsageGroup("hdmi");
        }
    }

    int CmdHdmi(int argc, char **argv)
    {
        if (argc < 1)
        {
            PrintUsageGroup("hdmi");
            return 1;
        }

        static const SubcommandEntry kSubcommands[] = {
            {"support", CmdHdmiSupport},
            {"hdcp-diag", CmdHdmiHdcpDiag},
            {"stereo", CmdHdmiStereoDispatch},
            {"audio-mute", CmdHdmiAudioMute},
        };

        return DispatchSubcommand("hdmi", argc, argv, kSubcommands, sizeof(kSubcommands) / sizeof(kSubcommands[0]), PrintHdmiUsage);
}
}

