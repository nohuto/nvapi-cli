/*
 * Copyright (c) 2026 Noverse (Nohuto). All rights reserved.
 * Proprietary and confidential. Unauthorized copying or redistribution is strictly prohibited.
 */

#include "cli/commands.h"

namespace nvcli
{
    namespace
    {
        const char *HdrModeName(NV_HDR_MODE mode)
        {
            switch (mode)
            {
            case NV_HDR_MODE_OFF:
                return "OFF";
            case NV_HDR_MODE_UHDA:
                return "UHDA";
            case NV_HDR_MODE_UHDA_PASSTHROUGH:
                return "UHDA_PASSTHROUGH";
            case NV_HDR_MODE_DOLBY_VISION:
                return "DOLBY_VISION";
            case NV_HDR_MODE_EDR:
                return "EDR";
            case NV_HDR_MODE_SDR:
                return "SDR";
            case NV_HDR_MODE_UHDA_NB:
                return "UHDA_NB";
            default:
                return "UNKNOWN";
            }
        }

        bool ParseHdrMode(const char *value, NV_HDR_MODE *out)
        {
            if (!value || !out)
            {
                return false;
            }
            std::string lowered = ToLowerAscii(value);
            if (lowered == "off")
            {
                *out = NV_HDR_MODE_OFF;
                return true;
            }
            if (lowered == "uhda")
            {
                *out = NV_HDR_MODE_UHDA;
                return true;
            }
            if (lowered == "uhda-passthrough")
            {
                *out = NV_HDR_MODE_UHDA_PASSTHROUGH;
                return true;
            }
            if (lowered == "dolby-vision")
            {
                *out = NV_HDR_MODE_DOLBY_VISION;
                return true;
            }
            if (lowered == "edr")
            {
                *out = NV_HDR_MODE_EDR;
                return true;
            }
            if (lowered == "sdr")
            {
                *out = NV_HDR_MODE_SDR;
                return true;
            }
            if (lowered == "uhda-nb")
            {
                *out = NV_HDR_MODE_UHDA_NB;
                return true;
            }
            NvU32 numeric = 0;
            if (!ParseUint(value, &numeric))
            {
                return false;
            }
            *out = static_cast<NV_HDR_MODE>(numeric);
            return true;
        }

        const char *ColorFormatName(NV_COLOR_FORMAT format)
        {
            switch (format)
            {
            case NV_COLOR_FORMAT_RGB:
                return "RGB";
            case NV_COLOR_FORMAT_YUV422:
                return "YUV422";
            case NV_COLOR_FORMAT_YUV444:
                return "YUV444";
            case NV_COLOR_FORMAT_YUV420:
                return "YUV420";
            case NV_COLOR_FORMAT_AUTO:
                return "AUTO";
            case NV_COLOR_FORMAT_DEFAULT:
                return "DEFAULT";
            default:
                return "UNKNOWN";
            }
        }

        bool ParseColorFormat(const char *value, NV_COLOR_FORMAT *out)
        {
            if (!value || !out)
            {
                return false;
            }
            std::string lowered = ToLowerAscii(value);
            if (lowered == "rgb")
            {
                *out = NV_COLOR_FORMAT_RGB;
                return true;
            }
            if (lowered == "yuv422")
            {
                *out = NV_COLOR_FORMAT_YUV422;
                return true;
            }
            if (lowered == "yuv444")
            {
                *out = NV_COLOR_FORMAT_YUV444;
                return true;
            }
            if (lowered == "yuv420")
            {
                *out = NV_COLOR_FORMAT_YUV420;
                return true;
            }
            if (lowered == "auto")
            {
                *out = NV_COLOR_FORMAT_AUTO;
                return true;
            }
            if (lowered == "default")
            {
                *out = NV_COLOR_FORMAT_DEFAULT;
                return true;
            }
            NvU32 numeric = 0;
            if (!ParseUint(value, &numeric))
            {
                return false;
            }
            *out = static_cast<NV_COLOR_FORMAT>(numeric);
            return true;
        }

        const char *DynamicRangeName(NV_DYNAMIC_RANGE range)
        {
            switch (range)
            {
            case NV_DYNAMIC_RANGE_VESA:
                return "VESA";
            case NV_DYNAMIC_RANGE_CEA:
                return "CEA";
            case NV_DYNAMIC_RANGE_AUTO:
                return "AUTO";
            default:
                return "UNKNOWN";
            }
        }

        bool ParseDynamicRange(const char *value, NV_DYNAMIC_RANGE *out)
        {
            if (!value || !out)
            {
                return false;
            }
            std::string lowered = ToLowerAscii(value);
            if (lowered == "vesa")
            {
                *out = NV_DYNAMIC_RANGE_VESA;
                return true;
            }
            if (lowered == "cea")
            {
                *out = NV_DYNAMIC_RANGE_CEA;
                return true;
            }
            if (lowered == "auto")
            {
                *out = NV_DYNAMIC_RANGE_AUTO;
                return true;
            }
            NvU32 numeric = 0;
            if (!ParseUint(value, &numeric))
            {
                return false;
            }
            *out = static_cast<NV_DYNAMIC_RANGE>(numeric);
            return true;
        }

        const char *BpcName(NV_BPC bpc)
        {
            switch (bpc)
            {
            case NV_BPC_DEFAULT:
                return "DEFAULT";
            case NV_BPC_6:
                return "6";
            case NV_BPC_8:
                return "8";
            case NV_BPC_10:
                return "10";
            case NV_BPC_12:
                return "12";
            case NV_BPC_16:
                return "16";
            default:
                return "UNKNOWN";
            }
        }

        bool ParseBpc(const char *value, NV_BPC *out)
        {
            if (!value || !out)
            {
                return false;
            }
            std::string lowered = ToLowerAscii(value);
            if (lowered == "default")
            {
                *out = NV_BPC_DEFAULT;
                return true;
            }
            if (lowered == "6")
            {
                *out = NV_BPC_6;
                return true;
            }
            if (lowered == "8")
            {
                *out = NV_BPC_8;
                return true;
            }
            if (lowered == "10")
            {
                *out = NV_BPC_10;
                return true;
            }
            if (lowered == "12")
            {
                *out = NV_BPC_12;
                return true;
            }
            if (lowered == "16")
            {
                *out = NV_BPC_16;
                return true;
            }
            NvU32 numeric = 0;
            if (!ParseUint(value, &numeric))
            {
                return false;
            }
            *out = static_cast<NV_BPC>(numeric);
            return true;
        }

        const char *OsHdrStateName(NV_OS_HDR_STATE state)
        {
            switch (state)
            {
            case NV_OS_HDR_DEFAULT:
                return "DEFAULT";
            case NV_OS_HDR_ON:
                return "ON";
            case NV_OS_HDR_OFF:
                return "OFF";
            default:
                return "UNKNOWN";
            }
        }

        bool ParseOsHdrState(const char *value, NV_OS_HDR_STATE *out)
        {
            if (!value || !out)
            {
                return false;
            }
            std::string lowered = ToLowerAscii(value);
            if (lowered == "default")
            {
                *out = NV_OS_HDR_DEFAULT;
                return true;
            }
            if (lowered == "on")
            {
                *out = NV_OS_HDR_ON;
                return true;
            }
            if (lowered == "off")
            {
                *out = NV_OS_HDR_OFF;
                return true;
            }
            NvU32 numeric = 0;
            if (!ParseUint(value, &numeric))
            {
                return false;
            }
            *out = static_cast<NV_OS_HDR_STATE>(numeric);
            return true;
        }

        void PrintHdrStaticMetadata(const NV_HDR_COLOR_DATA &data)
        {
            const auto &md = data.mastering_display_data;
            std::printf("  mastering: primaries=(%u,%u) (%u,%u) (%u,%u) white=(%u,%u)\n",
                md.displayPrimary_x0, md.displayPrimary_y0,
                md.displayPrimary_x1, md.displayPrimary_y1,
                md.displayPrimary_x2, md.displayPrimary_y2,
                md.displayWhitePoint_x, md.displayWhitePoint_y);
            std::printf("  mastering: maxLum=%u minLum=%u maxCLL=%u maxFALL=%u\n",
                md.max_display_mastering_luminance,
                md.min_display_mastering_luminance,
                md.max_content_light_level,
                md.max_frame_average_light_level);
        }

        void PrintHdrCapabilities(const NV_HDR_CAPABILITIES &caps)
        {
            std::printf("HDR caps:\n");
            std::printf("  hdrGamma=%u st2084=%u edr=%u sdrGamma=%u dolbyVision=%u expandDefault=%u\n",
                caps.isTraditionalHdrGammaSupported ? 1 : 0,
                caps.isST2084EotfSupported ? 1 : 0,
                caps.isEdrSupported ? 1 : 0,
                caps.isTraditionalSdrGammaSupported ? 1 : 0,
                caps.isDolbyVisionSupported ? 1 : 0,
                caps.driverExpandDefaultHdrParameters ? 1 : 0);
            std::printf("  staticMetadataId=%u\n", caps.static_metadata_descriptor_id);
            std::printf("  display primaries=(%u,%u) (%u,%u) (%u,%u) white=(%u,%u)\n",
                caps.display_data.displayPrimary_x0, caps.display_data.displayPrimary_y0,
                caps.display_data.displayPrimary_x1, caps.display_data.displayPrimary_y1,
                caps.display_data.displayPrimary_x2, caps.display_data.displayPrimary_y2,
                caps.display_data.displayWhitePoint_x, caps.display_data.displayWhitePoint_y);
            std::printf("  display luminance: max=%u min=%u maxFALL=%u\n",
                caps.display_data.desired_content_max_luminance,
                caps.display_data.desired_content_min_luminance,
                caps.display_data.desired_content_max_frame_average_luminance);
            std::printf("  dolbyVision: vsvdbVer=%u dmVer=%u 2160p60=%u yuv422_12b=%u globalDimming=%u\n",
                caps.dv_static_metadata.VSVDB_version,
                caps.dv_static_metadata.dm_version,
                caps.dv_static_metadata.supports_2160p60hz ? 1 : 0,
                caps.dv_static_metadata.supports_YUV422_12bit ? 1 : 0,
                caps.dv_static_metadata.supports_global_dimming ? 1 : 0);
        }
    }

    int CmdDisplayHdrCaps(int argc, char **argv)
    {
        NvU32 displayId = 0;
        if (!ParseDisplayIdArg(argc, argv, &displayId))
        {
            return 1;
        }

        NV_HDR_CAPABILITIES caps = {};
        caps.version = NV_HDR_CAPABILITIES_VER;
        NvAPI_Status status = NvAPI_Disp_GetHdrCapabilities(displayId, &caps);
        if (status != NVAPI_OK)
        {
            PrintNvapiError("NvAPI_Disp_GetHdrCapabilities failed", status);
            return 1;
        }

        PrintHdrCapabilities(caps);
        return 0;
    }

    int CmdDisplayHdrSessionGet(int argc, char **argv)
    {
        NvU32 displayId = 0;
        if (!ParseDisplayIdArg(argc, argv, &displayId))
        {
            return 1;
        }

        NV_HDR_SESSION_CONFIG_DATA data = {};
        data.version = NV_HDR_SESSION_CONFIG_DATA_VER;
        data.cmd = NV_HDR_CONTROL_CMD_GET;
        NvAPI_Status status = NvAPI_Disp_HdrSessionControl(displayId, &data);
        if (status != NVAPI_OK)
        {
            PrintNvapiError("NvAPI_Disp_HdrSessionControl failed", status);
            return 1;
        }

        std::printf("HDR session: enabled=%u hdrOn=%u expire=%u\n",
            data.bSessionState ? 1 : 0,
            data.bHDRState ? 1 : 0,
            data.sessionExpireTime);
        return 0;
    }

    int CmdDisplayHdrSessionSet(int argc, char **argv)
    {
        NvU32 displayId = 0;
        bool hasDisplayId = false;
        bool hasEnable = false;
        bool enable = false;
        NvU32 expire = 0;
        bool hasExpire = false;

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
            if (std::strcmp(argv[i], "--enable") == 0)
            {
                if (i + 1 >= argc || !ParseBoolValue(argv[i + 1], &enable))
                {
                    std::printf("Invalid --enable value.\n");
                    return 1;
                }
                hasEnable = true;
                ++i;
                continue;
            }
            if (std::strcmp(argv[i], "--expire") == 0)
            {
                if (i + 1 >= argc || !ParseUint(argv[i + 1], &expire))
                {
                    std::printf("Invalid --expire value.\n");
                    return 1;
                }
                hasExpire = true;
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
        if (!hasEnable)
        {
            std::printf("Missing required --enable\n");
            return 1;
        }

        NV_HDR_SESSION_CONFIG_DATA data = {};
        data.version = NV_HDR_SESSION_CONFIG_DATA_VER;
        data.cmd = NV_HDR_CONTROL_CMD_SET;
        data.bSessionState = enable ? 1 : 0;
        if (hasExpire)
        {
            data.sessionExpireTime = expire;
        }

        std::printf("HDR session set: enable=%u expire=%u\n",
            data.bSessionState ? 1 : 0,
            data.sessionExpireTime);

        NvAPI_Status status = NvAPI_Disp_HdrSessionControl(displayId, &data);
        if (status != NVAPI_OK)
        {
            PrintNvapiError("NvAPI_Disp_HdrSessionControl failed", status);
            return 1;
        }

        std::printf("HDR session updated.\n");
        return 0;
    }

    int CmdDisplayHdrSession(int argc, char **argv)
    {
        if (argc < 1)
        {
            PrintUsageGroup("display");
            return 1;
        }

        if (std::strcmp(argv[0], "get") == 0)
        {
            return CmdDisplayHdrSessionGet(argc - 1, argv + 1);
        }
        if (std::strcmp(argv[0], "set") == 0)
        {
            return CmdDisplayHdrSessionSet(argc - 1, argv + 1);
        }

        std::printf("Unknown display hdr session subcommand: %s\n", argv[0]);
        return 1;
    }

    int CmdDisplayHdrColorGet(int argc, char **argv)
    {
        NvU32 displayId = 0;
        if (!ParseDisplayIdArg(argc, argv, &displayId))
        {
            return 1;
        }

        NV_HDR_COLOR_DATA data = {};
        data.version = NV_HDR_COLOR_DATA_VER;
        data.cmd = NV_HDR_CMD_GET;
        NvAPI_Status status = NvAPI_Disp_HdrColorControl(displayId, &data);
        if (status != NVAPI_OK)
        {
            PrintNvapiError("NvAPI_Disp_HdrColorControl failed", status);
            return 1;
        }

        std::printf("HDR color: mode=%s format=%s range=%s bpc=%s osHdr=%s\n",
            HdrModeName(data.hdrMode),
            ColorFormatName(data.hdrColorFormat),
            DynamicRangeName(data.hdrDynamicRange),
            BpcName(data.hdrBpc),
            OsHdrStateName(data.osHdrMode));
        std::printf("  staticMetadataId=%u\n", data.static_metadata_descriptor_id);
        PrintHdrStaticMetadata(data);
        return 0;
    }

    int CmdDisplayHdrColorSet(int argc, char **argv)
    {
        NvU32 displayId = 0;
        bool hasDisplayId = false;
        bool hasMode = false;
        NV_HDR_MODE mode = NV_HDR_MODE_OFF;
        bool hasFormat = false;
        NV_COLOR_FORMAT format = NV_COLOR_FORMAT_DEFAULT;
        bool hasRange = false;
        NV_DYNAMIC_RANGE range = NV_DYNAMIC_RANGE_AUTO;
        bool hasBpc = false;
        NV_BPC bpc = NV_BPC_DEFAULT;
        bool hasOsHdr = false;
        NV_OS_HDR_STATE osHdr = NV_OS_HDR_DEFAULT;

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
            if (std::strcmp(argv[i], "--mode") == 0)
            {
                if (i + 1 >= argc || !ParseHdrMode(argv[i + 1], &mode))
                {
                    std::printf("Invalid --mode value.\n");
                    return 1;
                }
                hasMode = true;
                ++i;
                continue;
            }
            if (std::strcmp(argv[i], "--format") == 0)
            {
                if (i + 1 >= argc || !ParseColorFormat(argv[i + 1], &format))
                {
                    std::printf("Invalid --format value.\n");
                    return 1;
                }
                hasFormat = true;
                ++i;
                continue;
            }
            if (std::strcmp(argv[i], "--range") == 0)
            {
                if (i + 1 >= argc || !ParseDynamicRange(argv[i + 1], &range))
                {
                    std::printf("Invalid --range value.\n");
                    return 1;
                }
                hasRange = true;
                ++i;
                continue;
            }
            if (std::strcmp(argv[i], "--bpc") == 0)
            {
                if (i + 1 >= argc || !ParseBpc(argv[i + 1], &bpc))
                {
                    std::printf("Invalid --bpc value.\n");
                    return 1;
                }
                hasBpc = true;
                ++i;
                continue;
            }
            if (std::strcmp(argv[i], "--os-hdr") == 0)
            {
                if (i + 1 >= argc || !ParseOsHdrState(argv[i + 1], &osHdr))
                {
                    std::printf("Invalid --os-hdr value.\n");
                    return 1;
                }
                hasOsHdr = true;
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
        if (!hasMode && !hasFormat && !hasRange && !hasBpc && !hasOsHdr)
        {
            std::printf("No HDR color changes specified.\n");
            return 1;
        }

        NV_HDR_COLOR_DATA data = {};
        data.version = NV_HDR_COLOR_DATA_VER;
        data.cmd = NV_HDR_CMD_GET;
        NvAPI_Status status = NvAPI_Disp_HdrColorControl(displayId, &data);
        if (status != NVAPI_OK)
        {
            PrintNvapiError("NvAPI_Disp_HdrColorControl failed", status);
            return 1;
        }

        if (hasMode)
        {
            data.hdrMode = mode;
        }
        if (hasFormat)
        {
            data.hdrColorFormat = format;
        }
        if (hasRange)
        {
            data.hdrDynamicRange = range;
        }
        if (hasBpc)
        {
            data.hdrBpc = bpc;
        }
        if (hasOsHdr)
        {
            data.osHdrMode = osHdr;
        }

        data.cmd = NV_HDR_CMD_SET;

        std::printf("HDR color set: mode=%s format=%s range=%s bpc=%s osHdr=%s\n",
            HdrModeName(data.hdrMode),
            ColorFormatName(data.hdrColorFormat),
            DynamicRangeName(data.hdrDynamicRange),
            BpcName(data.hdrBpc),
            OsHdrStateName(data.osHdrMode));

        status = NvAPI_Disp_HdrColorControl(displayId, &data);
        if (status != NVAPI_OK)
        {
            PrintNvapiError("NvAPI_Disp_HdrColorControl failed", status);
            return 1;
        }

        std::printf("HDR color configuration updated.\n");
        return 0;
    }

    int CmdDisplayHdrColor(int argc, char **argv)
    {
        if (argc < 1)
        {
            PrintUsageGroup("display");
            return 1;
        }

        if (std::strcmp(argv[0], "get") == 0)
        {
            return CmdDisplayHdrColorGet(argc - 1, argv + 1);
        }
        if (std::strcmp(argv[0], "set") == 0)
        {
            return CmdDisplayHdrColorSet(argc - 1, argv + 1);
        }

        std::printf("Unknown display hdr color subcommand: %s\n", argv[0]);
        return 1;
    }

    int CmdDisplayHdr(int argc, char **argv)
    {
        if (argc < 1)
        {
            PrintUsageGroup("display");
            return 1;
        }

        if (std::strcmp(argv[0], "caps") == 0)
        {
            return CmdDisplayHdrCaps(argc - 1, argv + 1);
        }
        if (std::strcmp(argv[0], "session") == 0)
        {
            return CmdDisplayHdrSession(argc - 1, argv + 1);
        }
        if (std::strcmp(argv[0], "color") == 0)
        {
            return CmdDisplayHdrColor(argc - 1, argv + 1);
        }

        std::printf("Unknown display hdr subcommand: %s\n", argv[0]);
        return 1;
    }
}

