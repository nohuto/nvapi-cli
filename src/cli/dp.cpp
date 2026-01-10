/*
 * Copyright (c) 2026 Noverse (Nohuto). All rights reserved.
 * Proprietary and confidential. Unauthorized copying or redistribution is strictly prohibited.
 */

#include "cli/commands.h"

namespace nvcli
{
    namespace
    {
        void PrintDpUsage()
        {
            PrintUsageGroup("dp");
        }

        const char *DpLinkRateName(NV_DP_LINK_RATE rate)
        {
            switch (rate)
            {
            case NV_DP_1_62GBPS:
                return "1.62";
            case NV_DP_2_70GBPS:
                return "2.70";
            case NV_DP_5_40GBPS:
                return "5.40";
            case NV_DP_8_10GBPS:
                return "8.10";
            case NV_EDP_2_16GBPS:
                return "2.16";
            case NV_EDP_2_43GBPS:
                return "2.43";
            case NV_EDP_3_24GBPS:
                return "3.24";
            case NV_EDP_4_32GBPS:
                return "4.32";
            default:
                return "UNKNOWN";
            }
        }

        const char *DpLaneCountName(NV_DP_LANE_COUNT lanes)
        {
            switch (lanes)
            {
            case NV_DP_1_LANE:
                return "1";
            case NV_DP_2_LANE:
                return "2";
            case NV_DP_4_LANE:
                return "4";
            default:
                return "UNKNOWN";
            }
        }

        const char *DpColorFormatName(NV_DP_COLOR_FORMAT format)
        {
            switch (format)
            {
            case NV_DP_COLOR_FORMAT_RGB:
                return "RGB";
            case NV_DP_COLOR_FORMAT_YCbCr422:
                return "YCbCr422";
            case NV_DP_COLOR_FORMAT_YCbCr444:
                return "YCbCr444";
            default:
                return "UNKNOWN";
            }
        }

        const char *DpDynamicRangeName(NV_DP_DYNAMIC_RANGE range)
        {
            switch (range)
            {
            case NV_DP_DYNAMIC_RANGE_VESA:
                return "VESA";
            case NV_DP_DYNAMIC_RANGE_CEA:
                return "CEA";
            default:
                return "UNKNOWN";
            }
        }

        const char *DpColorimetryName(NV_DP_COLORIMETRY colorimetry)
        {
            switch (colorimetry)
            {
            case NV_DP_COLORIMETRY_RGB:
                return "RGB";
            case NV_DP_COLORIMETRY_YCbCr_ITU601:
                return "ITU601";
            case NV_DP_COLORIMETRY_YCbCr_ITU709:
                return "ITU709";
            default:
                return "UNKNOWN";
            }
        }

        const char *DpBpcName(NV_DP_BPC bpc)
        {
            switch (bpc)
            {
            case NV_DP_BPC_DEFAULT:
                return "DEFAULT";
            case NV_DP_BPC_6:
                return "6";
            case NV_DP_BPC_8:
                return "8";
            case NV_DP_BPC_10:
                return "10";
            case NV_DP_BPC_12:
                return "12";
            case NV_DP_BPC_16:
                return "16";
            default:
                return "UNKNOWN";
            }
        }

        bool ParseDpLinkRate(const char *value, NV_DP_LINK_RATE *out)
        {
            if (!value || !out)
            {
                return false;
            }
            std::string lowered = ToLowerAscii(value);
            if (lowered == "1.62")
            {
                *out = NV_DP_1_62GBPS;
                return true;
            }
            if (lowered == "2.70")
            {
                *out = NV_DP_2_70GBPS;
                return true;
            }
            if (lowered == "5.40")
            {
                *out = NV_DP_5_40GBPS;
                return true;
            }
            if (lowered == "8.10")
            {
                *out = NV_DP_8_10GBPS;
                return true;
            }
            if (lowered == "2.16")
            {
                *out = NV_EDP_2_16GBPS;
                return true;
            }
            if (lowered == "2.43")
            {
                *out = NV_EDP_2_43GBPS;
                return true;
            }
            if (lowered == "3.24")
            {
                *out = NV_EDP_3_24GBPS;
                return true;
            }
            if (lowered == "4.32")
            {
                *out = NV_EDP_4_32GBPS;
                return true;
            }
            NvU32 numeric = 0;
            if (!ParseUint(value, &numeric))
            {
                return false;
            }
            *out = static_cast<NV_DP_LINK_RATE>(numeric);
            return true;
        }

        bool ParseDpLaneCount(const char *value, NV_DP_LANE_COUNT *out)
        {
            if (!value || !out)
            {
                return false;
            }
            if (std::strcmp(value, "1") == 0)
            {
                *out = NV_DP_1_LANE;
                return true;
            }
            if (std::strcmp(value, "2") == 0)
            {
                *out = NV_DP_2_LANE;
                return true;
            }
            if (std::strcmp(value, "4") == 0)
            {
                *out = NV_DP_4_LANE;
                return true;
            }
            NvU32 numeric = 0;
            if (!ParseUint(value, &numeric))
            {
                return false;
            }
            *out = static_cast<NV_DP_LANE_COUNT>(numeric);
            return true;
        }

        bool ParseDpColorFormat(const char *value, NV_DP_COLOR_FORMAT *out)
        {
            if (!value || !out)
            {
                return false;
            }
            std::string lowered = ToLowerAscii(value);
            if (lowered == "rgb")
            {
                *out = NV_DP_COLOR_FORMAT_RGB;
                return true;
            }
            if (lowered == "ycbcr422")
            {
                *out = NV_DP_COLOR_FORMAT_YCbCr422;
                return true;
            }
            if (lowered == "ycbcr444")
            {
                *out = NV_DP_COLOR_FORMAT_YCbCr444;
                return true;
            }
            NvU32 numeric = 0;
            if (!ParseUint(value, &numeric))
            {
                return false;
            }
            *out = static_cast<NV_DP_COLOR_FORMAT>(numeric);
            return true;
        }

        bool ParseDpDynamicRange(const char *value, NV_DP_DYNAMIC_RANGE *out)
        {
            if (!value || !out)
            {
                return false;
            }
            std::string lowered = ToLowerAscii(value);
            if (lowered == "vesa")
            {
                *out = NV_DP_DYNAMIC_RANGE_VESA;
                return true;
            }
            if (lowered == "cea")
            {
                *out = NV_DP_DYNAMIC_RANGE_CEA;
                return true;
            }
            NvU32 numeric = 0;
            if (!ParseUint(value, &numeric))
            {
                return false;
            }
            *out = static_cast<NV_DP_DYNAMIC_RANGE>(numeric);
            return true;
        }

        bool ParseDpColorimetry(const char *value, NV_DP_COLORIMETRY *out)
        {
            if (!value || !out)
            {
                return false;
            }
            std::string lowered = ToLowerAscii(value);
            if (lowered == "rgb")
            {
                *out = NV_DP_COLORIMETRY_RGB;
                return true;
            }
            if (lowered == "itu601")
            {
                *out = NV_DP_COLORIMETRY_YCbCr_ITU601;
                return true;
            }
            if (lowered == "itu709")
            {
                *out = NV_DP_COLORIMETRY_YCbCr_ITU709;
                return true;
            }
            NvU32 numeric = 0;
            if (!ParseUint(value, &numeric))
            {
                return false;
            }
            *out = static_cast<NV_DP_COLORIMETRY>(numeric);
            return true;
        }

        bool ParseDpBpc(const char *value, NV_DP_BPC *out)
        {
            if (!value || !out)
            {
                return false;
            }
            std::string lowered = ToLowerAscii(value);
            if (lowered == "default")
            {
                *out = NV_DP_BPC_DEFAULT;
                return true;
            }
            if (lowered == "6")
            {
                *out = NV_DP_BPC_6;
                return true;
            }
            if (lowered == "8")
            {
                *out = NV_DP_BPC_8;
                return true;
            }
            if (lowered == "10")
            {
                *out = NV_DP_BPC_10;
                return true;
            }
            if (lowered == "12")
            {
                *out = NV_DP_BPC_12;
                return true;
            }
            if (lowered == "16")
            {
                *out = NV_DP_BPC_16;
                return true;
            }
            NvU32 numeric = 0;
            if (!ParseUint(value, &numeric))
            {
                return false;
            }
            *out = static_cast<NV_DP_BPC>(numeric);
            return true;
        }

        bool ResolveDisplayPortTarget(bool hasDisplayId,
            NvU32 displayId,
            NvU32 handleIndex,
            bool hasOutputId,
            NvU32 outputId,
            NvDisplayHandle *handleOut,
            NvU32 *outputOut)
        {
            if (!handleOut || !outputOut)
            {
                return false;
            }
            if (hasDisplayId)
            {
                *handleOut = NULL;
                *outputOut = displayId;
                return true;
            }

            NvDisplayHandle handle = NULL;
            if (!GetDisplayHandleByIndex(handleIndex, &handle))
            {
                std::printf("Display handle index %u not found.\n", handleIndex);
                return false;
            }

            if (!hasOutputId)
            {
                NvAPI_Status status = NvAPI_GetAssociatedDisplayOutputId(handle, &outputId);
                if (status != NVAPI_OK)
                {
                    PrintNvapiError("NvAPI_GetAssociatedDisplayOutputId failed", status);
                    return false;
                }
            }

            *handleOut = handle;
            *outputOut = outputId;
            return true;
        }
    }

    int CmdDpInfo(int argc, char **argv)
    {
        NvU32 displayId = 0;
        bool hasDisplayId = false;
        NvU32 handleIndex = 0;
        NvU32 outputId = 0;
        bool hasOutputId = false;

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
            std::printf("Unknown option: %s\n", argv[i]);
            return 1;
        }

        NvDisplayHandle handle = NULL;
        NvU32 resolvedOutput = 0;
        if (!ResolveDisplayPortTarget(hasDisplayId, displayId, handleIndex, hasOutputId, outputId, &handle, &resolvedOutput))
        {
            return 1;
        }

        NV_DISPLAY_PORT_INFO info = {};
        info.version = NV_DISPLAY_PORT_INFO_VER;
        NvAPI_Status status = NvAPI_GetDisplayPortInfo(handle, resolvedOutput, &info);
        if (status != NVAPI_OK)
        {
            PrintNvapiError("NvAPI_GetDisplayPortInfo failed", status);
            return 1;
        }

        std::printf("DisplayPort info: outputId=0x%08X\n", resolvedOutput);
        std::printf("  dpcd=0x%08X maxRate=%s maxLanes=%s currentRate=%s currentLanes=%s\n",
            info.dpcd_ver,
            DpLinkRateName(info.maxLinkRate),
            DpLaneCountName(info.maxLaneCount),
            DpLinkRateName(info.curLinkRate),
            DpLaneCountName(info.curLaneCount));
        std::printf("  format=%s range=%s colorimetry=%s bpc=%s\n",
            DpColorFormatName(info.colorFormat),
            DpDynamicRangeName(info.dynamicRange),
            DpColorimetryName(info.colorimetry),
            DpBpcName(info.bpc));
        std::printf("  flags: isDp=%u internal=%u colorCtrl=%u\n",
            info.isDp ? 1 : 0,
            info.isInternalDp ? 1 : 0,
            info.isColorCtrlSupported ? 1 : 0);
        std::printf("  bpcSupported: 6=%u 8=%u 10=%u 12=%u 16=%u\n",
            info.is6BPCSupported ? 1 : 0,
            info.is8BPCSupported ? 1 : 0,
            info.is10BPCSupported ? 1 : 0,
            info.is12BPCSupported ? 1 : 0,
            info.is16BPCSupported ? 1 : 0);
        std::printf("  ycbcrSupported: 420=%u 422=%u 444=%u\n",
            info.isYCrCb420Supported ? 1 : 0,
            info.isYCrCb422Supported ? 1 : 0,
            info.isYCrCb444Supported ? 1 : 0);
        std::printf("  currentMode: rgb444=%u ycbcr444=%u ycbcr422=%u ycbcr420=%u\n",
            info.isRgb444SupportedOnCurrentMode ? 1 : 0,
            info.isYCbCr444SupportedOnCurrentMode ? 1 : 0,
            info.isYCbCr422SupportedOnCurrentMode ? 1 : 0,
            info.isYCbCr420SupportedOnCurrentMode ? 1 : 0);
        std::printf("  currentBpc: 6=%u 8=%u 10=%u 12=%u 16=%u\n",
            info.is6BPCSupportedOnCurrentMode ? 1 : 0,
            info.is8BPCSupportedOnCurrentMode ? 1 : 0,
            info.is10BPCSupportedOnCurrentMode ? 1 : 0,
            info.is12BPCSupportedOnCurrentMode ? 1 : 0,
            info.is16BPCSupportedOnCurrentMode ? 1 : 0);
        std::printf("  extColorimetry: xvYCC601=%u xvYCC709=%u sYCC601=%u adobeYCC601=%u adobeRGB=%u bt2020rgb=%u bt2020ycc=%u bt2020cycc=%u\n",
            info.isMonxvYCC601Capable ? 1 : 0,
            info.isMonxvYCC709Capable ? 1 : 0,
            info.isMonsYCC601Capable ? 1 : 0,
            info.isMonAdobeYCC601Capable ? 1 : 0,
            info.isMonAdobeRGBCapable ? 1 : 0,
            info.isMonBT2020RGBCapable ? 1 : 0,
            info.isMonBT2020YCCCapable ? 1 : 0,
            info.isMonBT2020cYCCCapable ? 1 : 0);
        return 0;
    }

    int CmdDpSet(int argc, char **argv)
    {
        NvU32 displayId = 0;
        bool hasDisplayId = false;
        NvU32 handleIndex = 0;
        NvU32 outputId = 0;
        bool hasOutputId = false;

        NV_DP_LINK_RATE linkRate = NV_DP_1_62GBPS;
        bool hasLinkRate = false;
        NV_DP_LANE_COUNT laneCount = NV_DP_1_LANE;
        bool hasLaneCount = false;
        NV_DP_COLOR_FORMAT colorFormat = NV_DP_COLOR_FORMAT_RGB;
        bool hasColorFormat = false;
        NV_DP_DYNAMIC_RANGE dynamicRange = NV_DP_DYNAMIC_RANGE_VESA;
        bool hasDynamicRange = false;
        NV_DP_COLORIMETRY colorimetry = NV_DP_COLORIMETRY_RGB;
        bool hasColorimetry = false;
        NV_DP_BPC bpc = NV_DP_BPC_DEFAULT;
        bool hasBpc = false;

        bool hasHpd = false;
        bool hpd = false;
        bool hasDefer = false;
        bool defer = false;
        bool hasChromaLpfOff = false;
        bool chromaLpfOff = false;
        bool hasDitherOff = false;
        bool ditherOff = false;
        bool hasTestLinkTrain = false;
        bool testLinkTrain = false;
        bool hasTestColorChange = false;
        bool testColorChange = false;

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
            if (std::strcmp(argv[i], "--link-rate") == 0)
            {
                if (i + 1 >= argc || !ParseDpLinkRate(argv[i + 1], &linkRate))
                {
                    std::printf("Invalid --link-rate value.\n");
                    return 1;
                }
                hasLinkRate = true;
                ++i;
                continue;
            }
            if (std::strcmp(argv[i], "--lane-count") == 0)
            {
                if (i + 1 >= argc || !ParseDpLaneCount(argv[i + 1], &laneCount))
                {
                    std::printf("Invalid --lane-count value.\n");
                    return 1;
                }
                hasLaneCount = true;
                ++i;
                continue;
            }
            if (std::strcmp(argv[i], "--format") == 0)
            {
                if (i + 1 >= argc || !ParseDpColorFormat(argv[i + 1], &colorFormat))
                {
                    std::printf("Invalid --format value.\n");
                    return 1;
                }
                hasColorFormat = true;
                ++i;
                continue;
            }
            if (std::strcmp(argv[i], "--range") == 0)
            {
                if (i + 1 >= argc || !ParseDpDynamicRange(argv[i + 1], &dynamicRange))
                {
                    std::printf("Invalid --range value.\n");
                    return 1;
                }
                hasDynamicRange = true;
                ++i;
                continue;
            }
            if (std::strcmp(argv[i], "--colorimetry") == 0)
            {
                if (i + 1 >= argc || !ParseDpColorimetry(argv[i + 1], &colorimetry))
                {
                    std::printf("Invalid --colorimetry value.\n");
                    return 1;
                }
                hasColorimetry = true;
                ++i;
                continue;
            }
            if (std::strcmp(argv[i], "--bpc") == 0)
            {
                if (i + 1 >= argc || !ParseDpBpc(argv[i + 1], &bpc))
                {
                    std::printf("Invalid --bpc value.\n");
                    return 1;
                }
                hasBpc = true;
                ++i;
                continue;
            }
            if (std::strcmp(argv[i], "--hpd") == 0)
            {
                if (i + 1 >= argc || !ParseBoolValue(argv[i + 1], &hpd))
                {
                    std::printf("Invalid --hpd value.\n");
                    return 1;
                }
                hasHpd = true;
                ++i;
                continue;
            }
            if (std::strcmp(argv[i], "--defer") == 0)
            {
                if (i + 1 >= argc || !ParseBoolValue(argv[i + 1], &defer))
                {
                    std::printf("Invalid --defer value.\n");
                    return 1;
                }
                hasDefer = true;
                ++i;
                continue;
            }
            if (std::strcmp(argv[i], "--chroma-lpf-off") == 0)
            {
                if (i + 1 >= argc || !ParseBoolValue(argv[i + 1], &chromaLpfOff))
                {
                    std::printf("Invalid --chroma-lpf-off value.\n");
                    return 1;
                }
                hasChromaLpfOff = true;
                ++i;
                continue;
            }
            if (std::strcmp(argv[i], "--dither-off") == 0)
            {
                if (i + 1 >= argc || !ParseBoolValue(argv[i + 1], &ditherOff))
                {
                    std::printf("Invalid --dither-off value.\n");
                    return 1;
                }
                hasDitherOff = true;
                ++i;
                continue;
            }
            if (std::strcmp(argv[i], "--test-link-train") == 0)
            {
                if (i + 1 >= argc || !ParseBoolValue(argv[i + 1], &testLinkTrain))
                {
                    std::printf("Invalid --test-link-train value.\n");
                    return 1;
                }
                hasTestLinkTrain = true;
                ++i;
                continue;
            }
            if (std::strcmp(argv[i], "--test-color-change") == 0)
            {
                if (i + 1 >= argc || !ParseBoolValue(argv[i + 1], &testColorChange))
                {
                    std::printf("Invalid --test-color-change value.\n");
                    return 1;
                }
                hasTestColorChange = true;
                ++i;
                continue;
            }
            std::printf("Unknown option: %s\n", argv[i]);
            return 1;
        }

        if (!hasDisplayId && !hasOutputId)
        {
            std::printf("Missing required --id or --output-id\n");
            return 1;
        }

        if (!hasLinkRate && !hasLaneCount && !hasColorFormat && !hasDynamicRange && !hasColorimetry && !hasBpc &&
            !hasHpd && !hasDefer && !hasChromaLpfOff && !hasDitherOff && !hasTestLinkTrain && !hasTestColorChange)
        {
            std::printf("No DisplayPort changes specified.\n");
            return 1;
        }

        NvDisplayHandle handle = NULL;
        NvU32 resolvedOutput = 0;
        if (!ResolveDisplayPortTarget(hasDisplayId, displayId, handleIndex, hasOutputId, outputId, &handle, &resolvedOutput))
        {
            return 1;
        }

        NV_DISPLAY_PORT_INFO info = {};
        info.version = NV_DISPLAY_PORT_INFO_VER;
        NvAPI_Status status = NvAPI_GetDisplayPortInfo(handle, resolvedOutput, &info);
        if (status != NVAPI_OK)
        {
            PrintNvapiError("NvAPI_GetDisplayPortInfo failed", status);
            return 1;
        }

        NV_DISPLAY_PORT_CONFIG config = {};
        config.version = NV_DISPLAY_PORT_CONFIG_VER;
        config.linkRate = info.curLinkRate;
        config.laneCount = info.curLaneCount;
        config.colorFormat = info.colorFormat;
        config.dynamicRange = info.dynamicRange;
        config.colorimetry = info.colorimetry;
        config.bpc = info.bpc;

        if (hasLinkRate)
        {
            config.linkRate = linkRate;
        }
        if (hasLaneCount)
        {
            config.laneCount = laneCount;
        }
        if (hasColorFormat)
        {
            config.colorFormat = colorFormat;
        }
        if (hasDynamicRange)
        {
            config.dynamicRange = dynamicRange;
        }
        if (hasColorimetry)
        {
            config.colorimetry = colorimetry;
        }
        if (hasBpc)
        {
            config.bpc = bpc;
        }
        if (hasHpd)
        {
            config.isHPD = hpd ? 1 : 0;
        }
        if (hasDefer)
        {
            config.isSetDeferred = defer ? 1 : 0;
        }
        if (hasChromaLpfOff)
        {
            config.isChromaLpfOff = chromaLpfOff ? 1 : 0;
        }
        if (hasDitherOff)
        {
            config.isDitherOff = ditherOff ? 1 : 0;
        }
        if (hasTestLinkTrain)
        {
            config.testLinkTrain = testLinkTrain ? 1 : 0;
        }
        if (hasTestColorChange)
        {
            config.testColorChange = testColorChange ? 1 : 0;
        }

        std::printf("DisplayPort set: linkRate=%s laneCount=%s format=%s range=%s colorimetry=%s bpc=%s\n",
            DpLinkRateName(config.linkRate),
            DpLaneCountName(config.laneCount),
            DpColorFormatName(config.colorFormat),
            DpDynamicRangeName(config.dynamicRange),
            DpColorimetryName(config.colorimetry),
            DpBpcName(config.bpc));
        std::printf("  flags: hpd=%u defer=%u chromaLpfOff=%u ditherOff=%u testLinkTrain=%u testColorChange=%u\n",
            config.isHPD ? 1 : 0,
            config.isSetDeferred ? 1 : 0,
            config.isChromaLpfOff ? 1 : 0,
            config.isDitherOff ? 1 : 0,
            config.testLinkTrain ? 1 : 0,
            config.testColorChange ? 1 : 0);

        status = NvAPI_SetDisplayPort(handle, resolvedOutput, &config);
        if (status != NVAPI_OK)
        {
            PrintNvapiError("NvAPI_SetDisplayPort failed", status);
            return 1;
        }

        std::printf("DisplayPort configuration updated.\n");
        return 0;
    }

    int CmdDpDongle(int argc, char **argv)
    {
        NvU32 index = 0;
        bool hasIndex = false;
        NvU32 outputId = 0;
        bool hasOutputId = false;

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
            std::printf("Unknown option: %s\n", argv[i]);
            return 1;
        }

        if (!hasOutputId)
        {
            std::printf("Missing required --output-id\n");
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
            NV_NVAPI_GET_DP_DONGLE_INFO info = {};
            info.version = NV_NVAPI_GET_DP_DONGLE_INFO_VER;
            info.input.displayMask = outputId;
            NvAPI_Status status = NvAPI_GPU_Get_DisplayPort_DongleInfo(handles[i], &info);
            if (status != NVAPI_OK)
            {
                PrintNvapiError("  NvAPI_GPU_Get_DisplayPort_DongleInfo failed", status);
                continue;
            }

            std::printf("  DP dongle outputId=0x%08X\n", outputId);
            std::printf("    isDP2DVI=%u isDP2HDMI=%u isDP2VGA=%u\n",
                info.output.isDP2DVI ? 1 : 0,
                info.output.isDP2HDMI ? 1 : 0,
                info.output.isDP2VGA ? 1 : 0);
            std::printf("    isDP2DVIActive=%u isDP2HDMIActive=%u\n",
                info.output.isDP2DVIActive ? 1 : 0,
                info.output.isDP2HDMIActive ? 1 : 0);
            std::printf("    isDMS592DVI=%u isDMS592VGA=%u\n",
                info.output.isDMS592DVI ? 1 : 0,
                info.output.isDMS592VGA ? 1 : 0);
        }
        return 0;
    }

    void PrintDpNodeInfo(const NV_DP_NODE_INFO &node)
    {
        std::printf("  node displayId=0x%08X type=%s guid=%s\n",
            node.displayId,
            DpNodeTypeName(node.nodeType),
            GuidToString(node.guid).c_str());

        std::printf("    flags: multi=%u video=%u audio=%u loop=%u redundant=%u mustDisc=%u zombie=%u cableOk=%u\n",
            node.flags.isMultistream ? 1 : 0,
            node.flags.isVideoSink ? 1 : 0,
            node.flags.isAudioSink ? 1 : 0,
            node.flags.isLoop ? 1 : 0,
            node.flags.isRedundant ? 1 : 0,
            node.flags.isMustDisconnect ? 1 : 0,
            node.flags.isZombie ? 1 : 0,
            node.flags.isCableOk ? 1 : 0);
        std::printf("    flags: powerSuspended=%u active=%u hdcpCapable=%u hdcpPath=%u hdcpActive=%u revoked=%u\n",
            node.flags.isPowerSuspended ? 1 : 0,
            node.flags.isActive ? 1 : 0,
            node.flags.isHdcpCapable ? 1 : 0,
            node.flags.isPathHdcpCapable ? 1 : 0,
            node.flags.isHdcpActive ? 1 : 0,
            node.flags.isRevoked ? 1 : 0);
        std::printf("    ports: valid=0x%04X input=0x%04X internal=0x%04X\n",
            node.branchDevicePortsInfo.validPortsMask,
            node.branchDevicePortsInfo.inputPortsMask,
            node.branchDevicePortsInfo.internalPortsMask);
    }

    int CmdDpTopology(int argc, char **argv)
    {
        NvU32 displayId = 0;
        bool hasDisplayId = false;
        NvU32 flags = 0;
        bool hasFlags = false;

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
            if (std::strcmp(argv[i], "--flags") == 0)
            {
                if (i + 1 >= argc || !ParseUint(argv[i + 1], &flags))
                {
                    std::printf("Invalid flags.\n");
                    return 1;
                }
                hasFlags = true;
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

        if (hasFlags)
        {
            std::printf("DP topology flags are not supported by this NVAPI build. Re-run without --flags.\n");
            return 1;
        }

        NvU32 nodeCount = NV_DP_MAX_TOPOLOGY_NODES;
        std::vector<NV_DP_NODE_INFO> nodes(nodeCount);
        for (NvU32 i = 0; i < nodeCount; ++i)
        {
            nodes[i].version = NV_DP_NODE_INFO_VER;
        }
        NvAPI_Status status = NvAPI_GPU_QueryDPTopology(displayId, nodes.data(), &nodeCount);
        if (status != NVAPI_OK)
        {
            PrintNvapiError("NvAPI_GPU_QueryDPTopology failed", status);
            return 1;
        }

        std::printf("DP topology nodes: %u\n", nodeCount);
        for (NvU32 i = 0; i < nodeCount; ++i)
        {
            PrintDpNodeInfo(nodes[i]);
        }
        return 0;
    }

    int CmdDp(int argc, char **argv)
    {
        if (argc < 1)
        {
            PrintUsageGroup("dp");
            return 1;
        }

        static const SubcommandEntry kSubcommands[] = {
            {"info", CmdDpInfo},
            {"set", CmdDpSet},
            {"dongle", CmdDpDongle},
            {"topology", CmdDpTopology},
        };

        return DispatchSubcommand("dp", argc, argv, kSubcommands, sizeof(kSubcommands) / sizeof(kSubcommands[0]), PrintDpUsage);
    }
}

