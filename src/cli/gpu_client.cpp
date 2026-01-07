/*
 * Copyright (c) 2026 Noverse (Nohuto). All rights reserved.
 * Proprietary and confidential. Unauthorized copying or redistribution is strictly prohibited.
 */

#include "commands.h"

namespace nvcli
{
    namespace
    {
        bool HasBit(NvU32 mask, NvU32 index)
        {
            return (mask & (1u << index)) != 0;
        }

        void MaskE32Clear(NV_GPU_BOARDOBJGRP_MASK_E32 &mask)
        {
            std::memset(&mask, 0, sizeof(mask));
        }

        void MaskE32Set(NV_GPU_BOARDOBJGRP_MASK_E32 &mask, NvU32 index)
        {
            if (index >= NV_GPU_BOARDOBJGRP_E32_MAX_OBJECTS)
            {
                return;
            }
            const NvU32 word = index / 32;
            const NvU32 bit = index % 32;
            mask.super.pData[word] |= (1u << bit);
        }

        const char *ClientFanPolicyName(NV_GPU_CLIENT_FAN_POLICIES_POLICY_ID policyId)
        {
            switch (policyId)
            {
            case NV_GPU_CLIENT_FAN_POLICIES_POLICY_NONE:
                return "NONE";
            case NV_GPU_CLIENT_FAN_POLICIES_POLICY_GPU:
                return "GPU";
            case NV_GPU_CLIENT_FAN_POLICIES_POLICY_GPU_1:
                return "GPU_1";
            default:
                return "UNKNOWN";
            }
        }

        const char *ClientFanCoolerName(NV_GPU_CLIENT_FAN_COOLERS_COOLER_ID coolerId)
        {
            switch (coolerId)
            {
            case NV_GPU_CLIENT_FAN_COOLERS_COOLER_NONE:
                return "NONE";
            case NV_GPU_CLIENT_FAN_COOLERS_COOLER_GPU_0:
                return "GPU_0";
            case NV_GPU_CLIENT_FAN_COOLERS_COOLER_GPU_1:
                return "GPU_1";
            default:
                return "UNKNOWN";
            }
        }

        const char *PowerDeviceTypeName(NV_GPU_POWER_DEVICE_TYPE type)
        {
            switch (type)
            {
            case NV_GPU_POWER_DEVICE_TYPE_DISABLED:
                return "DISABLED";
            case NV_GPU_POWER_DEVICE_TYPE_INA219:
                return "INA219";
            case NV_GPU_POWER_DEVICE_TYPE_VT1165:
                return "VT1165";
            case NV_GPU_POWER_DEVICE_TYPE_CHL8112:
                return "CHL8112";
            case NV_GPU_POWER_DEVICE_TYPE_INA209:
                return "INA209";
            case NV_GPU_POWER_DEVICE_TYPE_INA3221:
                return "INA3221";
            case NV_GPU_POWER_DEVICE_TYPE_BA00:
                return "BA00";
            case NV_GPU_POWER_DEVICE_TYPE_BA10HW:
                return "BA10HW";
            case NV_GPU_POWER_DEVICE_TYPE_BA10SW:
                return "BA10SW";
            case NV_GPU_POWER_DEVICE_TYPE_BA11HW:
                return "BA11HW";
            case NV_GPU_POWER_DEVICE_TYPE_BA11SW:
                return "BA11SW";
            case NV_GPU_POWER_DEVICE_TYPE_BA12HW:
                return "BA12HW";
            case NV_GPU_POWER_DEVICE_TYPE_BA13HW:
                return "BA13HW";
            case NV_GPU_POWER_DEVICE_TYPE_BA14HW:
                return "BA14HW";
            case NV_GPU_POWER_DEVICE_TYPE_NCT3933U:
                return "NCT3933U";
            case NV_GPU_POWER_DEVICE_TYPE_GPUADC10:
                return "GPUADC10";
            case NV_GPU_POWER_DEVICE_TYPE_GPUADC11:
                return "GPUADC11";
            case NV_GPU_POWER_DEVICE_TYPE_BA15HW:
                return "BA15HW";
            case NV_GPU_POWER_DEVICE_TYPE_BA16HW:
                return "BA16HW";
            case NV_GPU_POWER_DEVICE_TYPE_BA20:
                return "BA20";
            case NV_GPU_POWER_DEVICE_TYPE_GPUADC13:
                return "GPUADC13";
            default:
                return "UNKNOWN";
            }
        }

        const char *PowerDeviceRailName(NV_GPU_POWER_DEVICE_POWER_RAIL rail)
        {
            switch (rail)
            {
            case NV_GPU_POWER_DEVICE_POWER_RAIL_DISABLED:
                return "DISABLED";
            case NV_GPU_POWER_DEVICE_POWER_RAIL_OUTPUT_NVVDD:
                return "OUTPUT_NVVDD";
            case NV_GPU_POWER_DEVICE_POWER_RAIL_OUTPUT_FBVDD:
                return "OUTPUT_FBVDD";
            case NV_GPU_POWER_DEVICE_POWER_RAIL_OUTPUT_FBVDDQ:
                return "OUTPUT_FBVDDQ";
            case NV_GPU_POWER_DEVICE_POWER_RAIL_OUTPUT_FBVDD_Q:
                return "OUTPUT_FBVDD_Q";
            case NV_GPU_POWER_DEVICE_POWER_RAIL_OUTPUT_PEXVDD:
                return "OUTPUT_PEXVDD";
            case NV_GPU_POWER_DEVICE_POWER_RAIL_OUTPUT_A3V3:
                return "OUTPUT_A3V3";
            case NV_GPU_POWER_DEVICE_POWER_RAIL_INPUT_EXT12V_8PIN0:
                return "INPUT_EXT12V_8PIN0";
            case NV_GPU_POWER_DEVICE_POWER_RAIL_INPUT_EXT12V_8PIN1:
                return "INPUT_EXT12V_8PIN1";
            case NV_GPU_POWER_DEVICE_POWER_RAIL_INPUT_EXT12V_6PIN0:
                return "INPUT_EXT12V_6PIN0";
            case NV_GPU_POWER_DEVICE_POWER_RAIL_INPUT_EXT12V_6PIN1:
                return "INPUT_EXT12V_6PIN1";
            case NV_GPU_POWER_DEVICE_POWER_RAIL_INPUT_PEX3V3:
                return "INPUT_PEX3V3";
            case NV_GPU_POWER_DEVICE_POWER_RAIL_INPUT_PEX12V:
                return "INPUT_PEX12V";
            default:
                return "UNKNOWN";
            }
        }

        const char *PowerChannelTypeName(NV_GPU_POWER_MONITOR_POWER_CHANNEL_TYPE type)
        {
            switch (type)
            {
            case NV_GPU_POWER_MONITOR_POWER_CHANNEL_TYPE_DEFAULT:
                return "DEFAULT";
            case NV_GPU_POWER_MONITOR_POWER_CHANNEL_TYPE_SUMMATION:
                return "SUMMATION";
            case NV_GPU_POWER_MONITOR_POWER_CHANNEL_TYPE_ESTIMATION:
                return "ESTIMATION";
            case NV_GPU_POWER_MONITOR_POWER_CHANNEL_TYPE_SLOW:
                return "SLOW";
            case NV_GPU_POWER_MONITOR_POWER_CHANNEL_TYPE_GEMINI_CORRECTION:
                return "GEMINI_CORRECTION";
            case NV_GPU_POWER_MONITOR_POWER_CHANNEL_TYPE_1X:
                return "1X";
            case NV_GPU_POWER_MONITOR_POWER_CHANNEL_TYPE_SENSOR:
                return "SENSOR";
            case NV_GPU_POWER_MONITOR_POWER_CHANNEL_TYPE_PSTATE_ESTIMATION_LUT:
                return "PSTATE_EST_LUT";
            case NV_GPU_POWER_MONITOR_POWER_CHANNEL_TYPE_SENSOR_CLIENT_ALIGNED:
                return "SENSOR_CLIENT_ALIGNED";
            default:
                return "UNKNOWN";
            }
        }

        const char *PowerChannelRailName(NV_GPU_POWER_CHANNEL_POWER_RAIL rail)
        {
            switch (rail)
            {
            case NV_GPU_POWER_CHANNEL_POWER_RAIL_UNKNOWN:
                return "UNKNOWN";
            case NV_GPU_POWER_CHANNEL_POWER_RAIL_OUTPUT_NVVDD:
                return "OUTPUT_NVVDD";
            case NV_GPU_POWER_CHANNEL_POWER_RAIL_OUTPUT_FBVDD:
                return "OUTPUT_FBVDD";
            case NV_GPU_POWER_CHANNEL_POWER_RAIL_OUTPUT_FBVDDQ:
                return "OUTPUT_FBVDDQ";
            case NV_GPU_POWER_CHANNEL_POWER_RAIL_OUTPUT_FBVDD_Q:
                return "OUTPUT_FBVDD_Q";
            case NV_GPU_POWER_CHANNEL_POWER_RAIL_OUTPUT_PEXVDD:
                return "OUTPUT_PEXVDD";
            case NV_GPU_POWER_CHANNEL_POWER_RAIL_OUTPUT_A3V3:
                return "OUTPUT_A3V3";
            case NV_GPU_POWER_CHANNEL_POWER_RAIL_OUTPUT_TOTAL_GPU:
                return "OUTPUT_TOTAL_GPU";
            case NV_GPU_POWER_CHANNEL_POWER_RAIL_INPUT_PEX12V:
                return "INPUT_PEX12V";
            case NV_GPU_POWER_CHANNEL_POWER_RAIL_INPUT_PEX3V3:
                return "INPUT_PEX3V3";
            case NV_GPU_POWER_CHANNEL_POWER_RAIL_INPUT_EXT12V_8PIN0:
                return "INPUT_EXT12V_8PIN0";
            case NV_GPU_POWER_CHANNEL_POWER_RAIL_INPUT_EXT12V_8PIN1:
                return "INPUT_EXT12V_8PIN1";
            case NV_GPU_POWER_CHANNEL_POWER_RAIL_INPUT_EXT12V_6PIN0:
                return "INPUT_EXT12V_6PIN0";
            case NV_GPU_POWER_CHANNEL_POWER_RAIL_INPUT_EXT12V_6PIN1:
                return "INPUT_EXT12V_6PIN1";
            case NV_GPU_POWER_CHANNEL_POWER_RAIL_INPUT_TOTAL_BOARD:
                return "INPUT_TOTAL_BOARD";
            case NV_GPU_POWER_CHANNEL_POWER_RAIL_INPUT_TOTAL_BOARD2:
                return "INPUT_TOTAL_BOARD2";
            default:
                return "UNKNOWN";
            }
        }

        const char *LeakageTypeName(NV_GPU_POWER_LEAKAGE_TYPE type)
        {
            switch (type)
            {
            case NV_GPU_POWER_LEAKAGE_TYPE_DTCS10:
                return "DTCS10";
            case NV_GPU_POWER_LEAKAGE_TYPE_DTCS11:
                return "DTCS11";
            case NV_GPU_POWER_LEAKAGE_TYPE_BA00_FIT:
                return "BA00_FIT";
            default:
                return "UNKNOWN";
            }
        }

        const char *IllumDeviceTypeName(NV_GPU_CLIENT_ILLUM_DEVICE_TYPE type)
        {
            switch (type)
            {
            case NV_GPU_CLIENT_ILLUM_DEVICE_TYPE_INVALID:
                return "INVALID";
            case NV_GPU_CLIENT_ILLUM_DEVICE_TYPE_MCUV10:
                return "MCUV10";
            case NV_GPU_CLIENT_ILLUM_DEVICE_TYPE_GPIO_PWM_RGBW_V10:
                return "GPIO_PWM_RGBW_V10";
            case NV_GPU_CLIENT_ILLUM_DEVICE_TYPE_GPIO_PWM_SINGLE_COLOR_V10:
                return "GPIO_PWM_SINGLE_COLOR_V10";
            default:
                return "UNKNOWN";
            }
        }

        const char *IllumZoneTypeName(NV_GPU_CLIENT_ILLUM_ZONE_TYPE type)
        {
            switch (type)
            {
            case NV_GPU_CLIENT_ILLUM_ZONE_TYPE_INVALID:
                return "INVALID";
            case NV_GPU_CLIENT_ILLUM_ZONE_TYPE_RGB:
                return "RGB";
            case NV_GPU_CLIENT_ILLUM_ZONE_TYPE_COLOR_FIXED:
                return "COLOR_FIXED";
            case NV_GPU_CLIENT_ILLUM_ZONE_TYPE_RGBW:
                return "RGBW";
            case NV_GPU_CLIENT_ILLUM_ZONE_TYPE_SINGLE_COLOR:
                return "SINGLE_COLOR";
            default:
                return "UNKNOWN";
            }
        }

        const char *IllumZoneLocationName(NV_GPU_CLIENT_ILLUM_ZONE_LOCATION location)
        {
            switch (location)
            {
            case NV_GPU_CLIENT_ILLUM_ZONE_LOCATION_GPU_TOP_0:
                return "GPU_TOP_0";
            case NV_GPU_CLIENT_ILLUM_ZONE_LOCATION_GPU_FRONT_0:
                return "GPU_FRONT_0";
            case NV_GPU_CLIENT_ILLUM_ZONE_LOCATION_GPU_BACK_0:
                return "GPU_BACK_0";
            case NV_GPU_CLIENT_ILLUM_ZONE_LOCATION_SLI_TOP_0:
                return "SLI_TOP_0";
            case NV_GPU_CLIENT_ILLUM_ZONE_LOCATION_INVALID:
                return "INVALID";
            default:
                return "UNKNOWN";
            }
        }

        const char *IllumCtrlModeName(NV_GPU_CLIENT_ILLUM_CTRL_MODE mode)
        {
            switch (mode)
            {
            case NV_GPU_CLIENT_ILLUM_CTRL_MODE_MANUAL:
                return "MANUAL";
            case NV_GPU_CLIENT_ILLUM_CTRL_MODE_PIECEWISE_LINEAR:
                return "PIECEWISE_LINEAR";
            default:
                return "UNKNOWN";
            }
        }
    }

    void PrintClientFanCoolersInfo(NvPhysicalGpuHandle handle)
    {
        NV_GPU_CLIENT_FAN_COOLERS_INFO info = {};
        info.version = NV_GPU_CLIENT_FAN_COOLERS_INFO_VER;
        NvAPI_Status status = NvAPI_GPU_ClientFanCoolersGetInfo(handle, &info);
        if (status != NVAPI_OK)
        {
            PrintNvapiError("  NvAPI_GPU_ClientFanCoolersGetInfo failed", status);
            return;
        }

        std::printf("  Client fan coolers supported: %u\n", info.bIsSupported ? 1 : 0);
        std::printf("  Client fan coolers: %u\n", info.numCoolers);
        for (NvU8 i = 0; i < info.numCoolers && i < NV_GPU_CLIENT_FAN_COOLERS_NUM_COOLERS_MAX; ++i)
        {
            const auto &cooler = info.coolers[i];
            std::printf("    cooler[%u]: id=%s(%u) tach=%u rpm=%u-%u\n",
                i,
                ClientFanCoolerName(cooler.coolerId),
                cooler.coolerId,
                cooler.bTachSupported ? 1u : 0u,
                cooler.rpmMin,
                cooler.rpmMax);
        }
    }

    void PrintClientFanCoolersStatus(NvPhysicalGpuHandle handle)
    {
        NV_GPU_CLIENT_FAN_COOLERS_STATUS statusData = {};
        statusData.version = NV_GPU_CLIENT_FAN_COOLERS_STATUS_VER;
        NvAPI_Status status = NvAPI_GPU_ClientFanCoolersGetStatus(handle, &statusData);
        if (status != NVAPI_OK)
        {
            PrintNvapiError("  NvAPI_GPU_ClientFanCoolersGetStatus failed", status);
            return;
        }

        std::printf("  Client fan cooler status: %u\n", statusData.numCoolers);
        for (NvU8 i = 0; i < statusData.numCoolers && i < NV_GPU_CLIENT_FAN_COOLERS_NUM_COOLERS_MAX; ++i)
        {
            const auto &cooler = statusData.coolers[i];
            std::printf("    cooler[%u]: id=%s(%u) rpm=%u level=%u-%u target=%u\n",
                i,
                ClientFanCoolerName(cooler.coolerId),
                cooler.coolerId,
                cooler.rpmCurr,
                cooler.levelMin,
                cooler.levelMax,
                cooler.levelTarget);
        }
    }

    void PrintClientFanCoolersControl(NvPhysicalGpuHandle handle)
    {
        NV_GPU_CLIENT_FAN_COOLERS_CONTROL control = {};
        control.version = NV_GPU_CLIENT_FAN_COOLERS_CONTROL_VER;
        NvAPI_Status status = NvAPI_GPU_ClientFanCoolersGetControl(handle, &control);
        if (status != NVAPI_OK)
        {
            PrintNvapiError("  NvAPI_GPU_ClientFanCoolersGetControl failed", status);
            return;
        }

        std::printf("  Client fan cooler control: %u\n", control.numCoolers);
        for (NvU8 i = 0; i < control.numCoolers && i < NV_GPU_CLIENT_FAN_COOLERS_NUM_COOLERS_MAX; ++i)
        {
            const auto &cooler = control.coolers[i];
            std::printf("    cooler[%u]: id=%s(%u) sim=%u%% active=%u\n",
                i,
                ClientFanCoolerName(cooler.coolerId),
                cooler.coolerId,
                cooler.levelSim,
                cooler.bLevelSimActive ? 1u : 0u);
        }
    }

    int CmdGpuClientFanCoolersSet(int argc, char **argv)
    {
        NvU32 index = 0;
        bool hasIndex = false;
        NvU32 coolerIndex = 0;
        bool hasCooler = false;
        NvU32 level = 0;
        bool hasLevel = false;
        bool enable = false;
        bool hasEnable = false;
        bool useDefault = false;

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
            if (std::strcmp(argv[i], "--cooler") == 0)
            {
                if (i + 1 >= argc)
                {
                    std::printf("Missing value for --cooler\n");
                    return 1;
                }
                if (!ParseUint(argv[i + 1], &coolerIndex))
                {
                    std::printf("Invalid cooler index: %s\n", argv[i + 1]);
                    return 1;
                }
                hasCooler = true;
                ++i;
                continue;
            }
            if (std::strcmp(argv[i], "--level") == 0)
            {
                if (i + 1 >= argc)
                {
                    std::printf("Missing value for --level\n");
                    return 1;
                }
                if (!ParseUint(argv[i + 1], &level))
                {
                    std::printf("Invalid level: %s\n", argv[i + 1]);
                    return 1;
                }
                hasLevel = true;
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
                    std::printf("Invalid enable value: %s\n", argv[i + 1]);
                    return 1;
                }
                hasEnable = true;
                ++i;
                continue;
            }
            if (std::strcmp(argv[i], "--default") == 0)
            {
                useDefault = true;
                continue;
            }
            std::printf("Unknown option: %s\n", argv[i]);
            return 1;
        }

        if (!useDefault)
        {
            if (!hasCooler)
            {
                std::printf("Missing required --cooler\n");
                return 1;
            }
            if (hasLevel && level > 100)
            {
                std::printf("Level must be between 0 and 100\n");
                return 1;
            }
            if (!hasEnable && hasLevel)
            {
                enable = true;
                hasEnable = true;
            }
            if (hasEnable && enable && !hasLevel)
            {
                std::printf("Missing required --level when enabling override\n");
                return 1;
            }
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
            NV_GPU_CLIENT_FAN_COOLERS_CONTROL control = {};
            control.version = NV_GPU_CLIENT_FAN_COOLERS_CONTROL_VER;
            NvAPI_Status status = NvAPI_GPU_ClientFanCoolersGetControl(handles[i], &control);
            if (status != NVAPI_OK)
            {
                PrintNvapiError("  NvAPI_GPU_ClientFanCoolersGetControl failed", status);
                continue;
            }

            if (useDefault)
            {
                control.bDefault = 1;
            }
            else
            {
                if (coolerIndex >= control.numCoolers)
                {
                    std::printf("  Cooler index %u out of range (0-%u)\n",
                        coolerIndex,
                        control.numCoolers ? control.numCoolers - 1 : 0);
                    continue;
                }

                auto &cooler = control.coolers[coolerIndex];
                if (hasEnable && !enable)
                {
                    cooler.bLevelSimActive = 0;
                    cooler.levelSim = 0;
                }
                else
                {
                    cooler.bLevelSimActive = 1;
                    cooler.levelSim = level;
                }
            }

            status = NvAPI_GPU_ClientFanCoolersSetControl(handles[i], &control);
            if (status == NVAPI_OK)
            {
                std::printf("  Client fan cooler control updated.\n");
            }
            else
            {
                PrintNvapiError("  NvAPI_GPU_ClientFanCoolersSetControl failed", status);
            }
        }
        return 0;
    }

    int CmdGpuClientFanCoolers(int argc, char **argv)
    {
        if (argc < 1)
        {
            std::printf("Missing client-fan coolers subcommand\n");
            return 1;
        }
        if (std::strcmp(argv[0], "info") == 0)
        {
            NvU32 index = 0;
            bool hasIndex = false;
            if (!ParseGpuIndexArgs(argc - 1, argv + 1, &index, &hasIndex))
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
                PrintClientFanCoolersInfo(handles[i]);
            }
            return 0;
        }
        if (std::strcmp(argv[0], "status") == 0)
        {
            NvU32 index = 0;
            bool hasIndex = false;
            if (!ParseGpuIndexArgs(argc - 1, argv + 1, &index, &hasIndex))
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
                PrintClientFanCoolersStatus(handles[i]);
            }
            return 0;
        }
        if (std::strcmp(argv[0], "control") == 0)
        {
            NvU32 index = 0;
            bool hasIndex = false;
            if (!ParseGpuIndexArgs(argc - 1, argv + 1, &index, &hasIndex))
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
                PrintClientFanCoolersControl(handles[i]);
            }
            return 0;
        }
        if (std::strcmp(argv[0], "set") == 0)
        {
            return CmdGpuClientFanCoolersSet(argc - 1, argv + 1);
        }

        std::printf("Unknown client-fan coolers subcommand: %s\n", argv[0]);
        return 1;
    }

    void PrintClientFanPoliciesInfo(NvPhysicalGpuHandle handle)
    {
        NV_GPU_CLIENT_FAN_POLICIES_INFO info = {};
        info.version = NV_GPU_CLIENT_FAN_POLICIES_INFO_VER;
        NvAPI_Status status = NvAPI_GPU_ClientFanPoliciesGetInfo(handle, &info);
        if (status != NVAPI_OK)
        {
            PrintNvapiError("  NvAPI_GPU_ClientFanPoliciesGetInfo failed", status);
            return;
        }

        std::printf("  Client fan policies supported: %u\n", info.isSupported ? 1 : 0);
        std::printf("  Client fan policies: %u\n", info.numPolicies);
        for (NvU8 i = 0; i < info.numPolicies && i < NV_GPU_CLIENT_FAN_POLICIES_NUM_POLICIES_MAX; ++i)
        {
            const auto &policy = info.policies[i];
            std::printf("    policy[%u]: id=%s(%u) idx=%u arbiterMask=0x%02X stopSupported=%u stopDefault=%u curveAdj=%u\n",
                i,
                ClientFanPolicyName(policy.policyId),
                policy.policyId,
                policy.policyIdx,
                policy.arbiterMask,
                policy.fanStopFeatureSupported ? 1u : 0u,
                policy.fanStopFeatureEnableDefault ? 1u : 0u,
                policy.fanCurveAdjSupported ? 1u : 0u);
        }
    }

    void PrintClientFanPoliciesStatus(NvPhysicalGpuHandle handle)
    {
        NV_GPU_CLIENT_FAN_POLICIES_STATUS statusData = {};
        statusData.version = NV_GPU_CLIENT_FAN_POLICIES_STATUS_VER;
        NvAPI_Status status = NvAPI_GPU_ClientFanPoliciesGetStatus(handle, &statusData);
        if (status != NVAPI_OK)
        {
            PrintNvapiError("  NvAPI_GPU_ClientFanPoliciesGetStatus failed", status);
            return;
        }

        std::printf("  Client fan policy status: %u\n", statusData.numPolicies);
        for (NvU8 i = 0; i < statusData.numPolicies && i < NV_GPU_CLIENT_FAN_POLICIES_NUM_POLICIES_MAX; ++i)
        {
            const auto &policy = statusData.policies[i];
            std::printf("    policy[%u]: id=%s(%u) stopActive=%u\n",
                i,
                ClientFanPolicyName(policy.policyId),
                policy.policyId,
                policy.fanStopFeatureActive ? 1u : 0u);
        }
    }

    void PrintClientFanPoliciesControl(NvPhysicalGpuHandle handle)
    {
        NV_GPU_CLIENT_FAN_POLICIES_CONTROL control = {};
        control.version = NV_GPU_CLIENT_FAN_POLICIES_CONTROL_VER;
        NvAPI_Status status = NvAPI_GPU_ClientFanPoliciesGetControl(handle, &control);
        if (status != NVAPI_OK)
        {
            PrintNvapiError("  NvAPI_GPU_ClientFanPoliciesGetControl failed", status);
            return;
        }

        std::printf("  Client fan policy control: %u\n", control.numPolicies);
        for (NvU8 i = 0; i < control.numPolicies && i < NV_GPU_CLIENT_FAN_POLICIES_NUM_POLICIES_MAX; ++i)
        {
            const auto &policy = control.policies[i];
            std::printf("    policy[%u]: id=%s(%u) idx=%u stopEnable=%u\n",
                i,
                ClientFanPolicyName(policy.policyId),
                policy.policyId,
                policy.policyIdx,
                policy.fanStopFeatureEnable ? 1u : 0u);
            for (NvU32 p = 0; p < NV_GPU_CLIENT_FAN_POLICY_IIR_TJ_FIXED_DUAL_SLOPE_PWM_FAN_CURVE_PTS; ++p)
            {
                const auto &pt = policy.fanCurvePts[p];
                const float tj = static_cast<float>(pt.fanTj) / 256.0f;
                const float pwm = static_cast<float>(pt.fanPwm) / 65536.0f;
                std::printf("      curve[%u]: tj=%.2fC pwm=%.2f%% rpm=%u\n",
                    p,
                    tj,
                    pwm,
                    pt.fanRpm);
            }
        }
    }

    int CmdGpuClientFanPoliciesSet(int argc, char **argv)
    {
        NvU32 index = 0;
        bool hasIndex = false;
        NvU32 policyIndex = 0;
        bool hasPolicy = false;
        bool fanStopEnable = false;
        bool hasFanStop = false;
        bool useDefault = false;

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
            if (std::strcmp(argv[i], "--policy") == 0)
            {
                if (i + 1 >= argc)
                {
                    std::printf("Missing value for --policy\n");
                    return 1;
                }
                if (!ParseUint(argv[i + 1], &policyIndex))
                {
                    std::printf("Invalid policy index: %s\n", argv[i + 1]);
                    return 1;
                }
                hasPolicy = true;
                ++i;
                continue;
            }
            if (std::strcmp(argv[i], "--fan-stop") == 0)
            {
                if (i + 1 >= argc)
                {
                    std::printf("Missing value for --fan-stop\n");
                    return 1;
                }
                if (!ParseBoolValue(argv[i + 1], &fanStopEnable))
                {
                    std::printf("Invalid fan-stop value: %s\n", argv[i + 1]);
                    return 1;
                }
                hasFanStop = true;
                ++i;
                continue;
            }
            if (std::strcmp(argv[i], "--default") == 0)
            {
                useDefault = true;
                continue;
            }
            std::printf("Unknown option: %s\n", argv[i]);
            return 1;
        }

        if (!useDefault)
        {
            if (!hasPolicy)
            {
                std::printf("Missing required --policy\n");
                return 1;
            }
            if (!hasFanStop)
            {
                std::printf("Missing required --fan-stop\n");
                return 1;
            }
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
            NV_GPU_CLIENT_FAN_POLICIES_CONTROL control = {};
            control.version = NV_GPU_CLIENT_FAN_POLICIES_CONTROL_VER;
            NvAPI_Status status = NvAPI_GPU_ClientFanPoliciesGetControl(handles[i], &control);
            if (status != NVAPI_OK)
            {
                PrintNvapiError("  NvAPI_GPU_ClientFanPoliciesGetControl failed", status);
                continue;
            }

            if (useDefault)
            {
                control.bDefault = 1;
            }
            else
            {
                if (policyIndex >= control.numPolicies)
                {
                    std::printf("  Policy index %u out of range (0-%u)\n",
                        policyIndex,
                        control.numPolicies ? control.numPolicies - 1 : 0);
                    continue;
                }
                auto &policy = control.policies[policyIndex];
                policy.fanStopFeatureEnable = fanStopEnable ? 1u : 0u;
            }

            status = NvAPI_GPU_ClientFanPoliciesSetControl(handles[i], &control);
            if (status == NVAPI_OK)
            {
                std::printf("  Client fan policy control updated.\n");
            }
            else
            {
                PrintNvapiError("  NvAPI_GPU_ClientFanPoliciesSetControl failed", status);
            }
        }
        return 0;
    }

    int CmdGpuClientFanPolicies(int argc, char **argv)
    {
        if (argc < 1)
        {
            std::printf("Missing client-fan policies subcommand\n");
            return 1;
        }
        if (std::strcmp(argv[0], "info") == 0)
        {
            NvU32 index = 0;
            bool hasIndex = false;
            if (!ParseGpuIndexArgs(argc - 1, argv + 1, &index, &hasIndex))
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
                PrintClientFanPoliciesInfo(handles[i]);
            }
            return 0;
        }
        if (std::strcmp(argv[0], "status") == 0)
        {
            NvU32 index = 0;
            bool hasIndex = false;
            if (!ParseGpuIndexArgs(argc - 1, argv + 1, &index, &hasIndex))
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
                PrintClientFanPoliciesStatus(handles[i]);
            }
            return 0;
        }
        if (std::strcmp(argv[0], "control") == 0)
        {
            NvU32 index = 0;
            bool hasIndex = false;
            if (!ParseGpuIndexArgs(argc - 1, argv + 1, &index, &hasIndex))
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
                PrintClientFanPoliciesControl(handles[i]);
            }
            return 0;
        }
        if (std::strcmp(argv[0], "set") == 0)
        {
            return CmdGpuClientFanPoliciesSet(argc - 1, argv + 1);
        }

        std::printf("Unknown client-fan policies subcommand: %s\n", argv[0]);
        return 1;
    }

    void PrintClientFanArbitersInfo(NvPhysicalGpuHandle handle)
    {
        NV_GPU_CLIENT_FAN_ARBITERS_INFO info = {};
        info.version = NV_GPU_CLIENT_FAN_ARBITERS_INFO_VER;
        NvAPI_Status status = NvAPI_GPU_ClientFanArbitersGetInfo(handle, &info);
        if (status != NVAPI_OK)
        {
            PrintNvapiError("  NvAPI_GPU_ClientFanArbitersGetInfo failed", status);
            return;
        }

        std::printf("  Client fan arbiters: %u\n", info.numArbiters);
        for (NvU8 i = 0; i < info.numArbiters && i < NV_GPU_CLIENT_FAN_ARBITERS_NUM_ARBITERS_MAX; ++i)
        {
            const auto &arbiter = info.arbiters[i];
            std::printf("    arbiter[%u]: idx=%u cooler=%s(%u) stopSupported=%u stopDefault=%u\n",
                i,
                arbiter.arbiterIdx,
                ClientFanCoolerName(arbiter.coolerId),
                arbiter.coolerId,
                arbiter.fanStopFeatureSupported ? 1u : 0u,
                arbiter.fanStopFeatureEnableDefault ? 1u : 0u);
        }
    }

    void PrintClientFanArbitersStatus(NvPhysicalGpuHandle handle)
    {
        NV_GPU_CLIENT_FAN_ARBITERS_STATUS statusData = {};
        statusData.version = NV_GPU_CLIENT_FAN_ARBITERS_STATUS_VER;
        NvAPI_Status status = NvAPI_GPU_ClientFanArbitersGetStatus(handle, &statusData);
        if (status != NVAPI_OK)
        {
            PrintNvapiError("  NvAPI_GPU_ClientFanArbitersGetStatus failed", status);
            return;
        }

        std::printf("  Client fan arbiter status: %u\n", statusData.numArbiters);
        for (NvU8 i = 0; i < statusData.numArbiters && i < NV_GPU_CLIENT_FAN_ARBITERS_NUM_ARBITERS_MAX; ++i)
        {
            const auto &arbiter = statusData.arbiters[i];
            std::printf("    arbiter[%u]: idx=%u stopActive=%u\n",
                i,
                arbiter.arbiterIdx,
                arbiter.fanStopFeatureActive ? 1u : 0u);
        }
    }

    void PrintClientFanArbitersControl(NvPhysicalGpuHandle handle)
    {
        NV_GPU_CLIENT_FAN_ARBITERS_CONTROL control = {};
        control.version = NV_GPU_CLIENT_FAN_ARBITERS_CONTROL_VER;
        NvAPI_Status status = NvAPI_GPU_ClientFanArbitersGetControl(handle, &control);
        if (status != NVAPI_OK)
        {
            PrintNvapiError("  NvAPI_GPU_ClientFanArbitersGetControl failed", status);
            return;
        }

        std::printf("  Client fan arbiter control: %u\n", control.numArbiters);
        for (NvU8 i = 0; i < control.numArbiters && i < NV_GPU_CLIENT_FAN_ARBITERS_NUM_ARBITERS_MAX; ++i)
        {
            const auto &arbiter = control.arbiters[i];
            std::printf("    arbiter[%u]: idx=%u stopEnable=%u\n",
                i,
                arbiter.arbiterIdx,
                arbiter.fanStopFeatureEnable ? 1u : 0u);
        }
    }

    int CmdGpuClientFanArbitersSet(int argc, char **argv)
    {
        NvU32 index = 0;
        bool hasIndex = false;
        NvU32 arbiterIndex = 0;
        bool hasArbiter = false;
        bool fanStopEnable = false;
        bool hasFanStop = false;

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
            if (std::strcmp(argv[i], "--arbiter") == 0)
            {
                if (i + 1 >= argc)
                {
                    std::printf("Missing value for --arbiter\n");
                    return 1;
                }
                if (!ParseUint(argv[i + 1], &arbiterIndex))
                {
                    std::printf("Invalid arbiter index: %s\n", argv[i + 1]);
                    return 1;
                }
                hasArbiter = true;
                ++i;
                continue;
            }
            if (std::strcmp(argv[i], "--fan-stop") == 0)
            {
                if (i + 1 >= argc)
                {
                    std::printf("Missing value for --fan-stop\n");
                    return 1;
                }
                if (!ParseBoolValue(argv[i + 1], &fanStopEnable))
                {
                    std::printf("Invalid fan-stop value: %s\n", argv[i + 1]);
                    return 1;
                }
                hasFanStop = true;
                ++i;
                continue;
            }
            std::printf("Unknown option: %s\n", argv[i]);
            return 1;
        }

        if (!hasArbiter)
        {
            std::printf("Missing required --arbiter\n");
            return 1;
        }
        if (!hasFanStop)
        {
            std::printf("Missing required --fan-stop\n");
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
            NV_GPU_CLIENT_FAN_ARBITERS_CONTROL control = {};
            control.version = NV_GPU_CLIENT_FAN_ARBITERS_CONTROL_VER;
            NvAPI_Status status = NvAPI_GPU_ClientFanArbitersGetControl(handles[i], &control);
            if (status != NVAPI_OK)
            {
                PrintNvapiError("  NvAPI_GPU_ClientFanArbitersGetControl failed", status);
                continue;
            }
            if (arbiterIndex >= control.numArbiters)
            {
                std::printf("  Arbiter index %u out of range (0-%u)\n",
                    arbiterIndex,
                    control.numArbiters ? control.numArbiters - 1 : 0);
                continue;
            }
            control.arbiters[arbiterIndex].fanStopFeatureEnable = fanStopEnable ? 1u : 0u;

            status = NvAPI_GPU_ClientFanArbitersSetControl(handles[i], &control);
            if (status == NVAPI_OK)
            {
                std::printf("  Client fan arbiter control updated.\n");
            }
            else
            {
                PrintNvapiError("  NvAPI_GPU_ClientFanArbitersSetControl failed", status);
            }
        }
        return 0;
    }

    int CmdGpuClientFanArbiters(int argc, char **argv)
    {
        if (argc < 1)
        {
            std::printf("Missing client-fan arbiters subcommand\n");
            return 1;
        }
        if (std::strcmp(argv[0], "info") == 0)
        {
            NvU32 index = 0;
            bool hasIndex = false;
            if (!ParseGpuIndexArgs(argc - 1, argv + 1, &index, &hasIndex))
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
                PrintClientFanArbitersInfo(handles[i]);
            }
            return 0;
        }
        if (std::strcmp(argv[0], "status") == 0)
        {
            NvU32 index = 0;
            bool hasIndex = false;
            if (!ParseGpuIndexArgs(argc - 1, argv + 1, &index, &hasIndex))
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
                PrintClientFanArbitersStatus(handles[i]);
            }
            return 0;
        }
        if (std::strcmp(argv[0], "control") == 0)
        {
            NvU32 index = 0;
            bool hasIndex = false;
            if (!ParseGpuIndexArgs(argc - 1, argv + 1, &index, &hasIndex))
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
                PrintClientFanArbitersControl(handles[i]);
            }
            return 0;
        }
        if (std::strcmp(argv[0], "set") == 0)
        {
            return CmdGpuClientFanArbitersSet(argc - 1, argv + 1);
        }

        std::printf("Unknown client-fan arbiters subcommand: %s\n", argv[0]);
        return 1;
    }

    void PrintClientIllumDevicesInfo(NvPhysicalGpuHandle handle)
    {
        NV_GPU_CLIENT_ILLUM_DEVICE_INFO_PARAMS info = {};
        info.version = NV_GPU_CLIENT_ILLUM_DEVICE_INFO_PARAMS_VER;
        NvAPI_Status status = NvAPI_GPU_ClientIllumDevicesGetInfo(handle, &info);
        if (status != NVAPI_OK)
        {
            PrintNvapiError("  NvAPI_GPU_ClientIllumDevicesGetInfo failed", status);
            return;
        }

        std::printf("  Illum devices: %u\n", info.numIllumDevices);
        for (NvU32 i = 0; i < info.numIllumDevices && i < NV_GPU_CLIENT_ILLUM_DEVICE_NUM_DEVICES_MAX; ++i)
        {
            const auto &device = info.devices[i];
            std::printf("    device[%u]: type=%s(%u) ctrlModeMask=0x%08X\n",
                i,
                IllumDeviceTypeName(device.type),
                device.type,
                device.ctrlModeMask);

            if (device.type == NV_GPU_CLIENT_ILLUM_DEVICE_TYPE_MCUV10)
            {
                std::printf("      i2cDevIdx=%u\n", device.data.mcuv10.i2cDevIdx);
            }
            else if (device.type == NV_GPU_CLIENT_ILLUM_DEVICE_TYPE_GPIO_PWM_RGBW_V10)
            {
                std::printf("      pins: r=%u g=%u b=%u w=%u\n",
                    device.data.gpioPwmRgbwv10.gpioPinRed,
                    device.data.gpioPwmRgbwv10.gpioPinGreen,
                    device.data.gpioPwmRgbwv10.gpioPinBlue,
                    device.data.gpioPwmRgbwv10.gpioPinWhite);
            }
            else if (device.type == NV_GPU_CLIENT_ILLUM_DEVICE_TYPE_GPIO_PWM_SINGLE_COLOR_V10)
            {
                std::printf("      pin: single=%u\n", device.data.gpioPwmSingleColorv10.gpioPinSingleColor);
            }
        }
    }

    void PrintClientIllumDevicesControl(NvPhysicalGpuHandle handle)
    {
        NV_GPU_CLIENT_ILLUM_DEVICE_CONTROL_PARAMS control = {};
        control.version = NV_GPU_CLIENT_ILLUM_DEVICE_CONTROL_PARAMS_VER;
        NvAPI_Status status = NvAPI_GPU_ClientIllumDevicesGetControl(handle, &control);
        if (status != NVAPI_OK)
        {
            PrintNvapiError("  NvAPI_GPU_ClientIllumDevicesGetControl failed", status);
            return;
        }

        std::printf("  Illum device control: %u\n", control.numIllumDevices);
        for (NvU32 i = 0; i < control.numIllumDevices && i < NV_GPU_CLIENT_ILLUM_DEVICE_NUM_DEVICES_MAX; ++i)
        {
            const auto &device = control.devices[i];
            std::printf("    device[%u]: type=%s(%u) sync=%u timestamp=%llu\n",
                i,
                IllumDeviceTypeName(device.type),
                device.type,
                device.syncData.bSync ? 1u : 0u,
                static_cast<unsigned long long>(device.syncData.timeStampms));
        }
    }

    int CmdGpuClientIllumDevicesSet(int argc, char **argv)
    {
        NvU32 index = 0;
        bool hasIndex = false;
        NvU32 deviceIndex = 0;
        bool hasDevice = false;
        bool sync = false;
        bool hasSync = false;
        NvU32 timestampMs = 0;
        bool hasTimestamp = false;

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
            if (std::strcmp(argv[i], "--device") == 0)
            {
                if (i + 1 >= argc)
                {
                    std::printf("Missing value for --device\n");
                    return 1;
                }
                if (!ParseUint(argv[i + 1], &deviceIndex))
                {
                    std::printf("Invalid device index: %s\n", argv[i + 1]);
                    return 1;
                }
                hasDevice = true;
                ++i;
                continue;
            }
            if (std::strcmp(argv[i], "--sync") == 0)
            {
                if (i + 1 >= argc)
                {
                    std::printf("Missing value for --sync\n");
                    return 1;
                }
                if (!ParseBoolValue(argv[i + 1], &sync))
                {
                    std::printf("Invalid sync value: %s\n", argv[i + 1]);
                    return 1;
                }
                hasSync = true;
                ++i;
                continue;
            }
            if (std::strcmp(argv[i], "--timestamp-ms") == 0)
            {
                if (i + 1 >= argc)
                {
                    std::printf("Missing value for --timestamp-ms\n");
                    return 1;
                }
                if (!ParseUint(argv[i + 1], &timestampMs))
                {
                    std::printf("Invalid timestamp: %s\n", argv[i + 1]);
                    return 1;
                }
                hasTimestamp = true;
                ++i;
                continue;
            }
            std::printf("Unknown option: %s\n", argv[i]);
            return 1;
        }

        if (!hasDevice)
        {
            std::printf("Missing required --device\n");
            return 1;
        }
        if (!hasSync)
        {
            std::printf("Missing required --sync\n");
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
            NV_GPU_CLIENT_ILLUM_DEVICE_CONTROL_PARAMS control = {};
            control.version = NV_GPU_CLIENT_ILLUM_DEVICE_CONTROL_PARAMS_VER;
            NvAPI_Status status = NvAPI_GPU_ClientIllumDevicesGetControl(handles[i], &control);
            if (status != NVAPI_OK)
            {
                PrintNvapiError("  NvAPI_GPU_ClientIllumDevicesGetControl failed", status);
                continue;
            }
            if (deviceIndex >= control.numIllumDevices)
            {
                std::printf("  Device index %u out of range (0-%u)\n",
                    deviceIndex,
                    control.numIllumDevices ? control.numIllumDevices - 1 : 0);
                continue;
            }
            auto &device = control.devices[deviceIndex];
            device.syncData.bSync = sync ? NV_TRUE : NV_FALSE;
            if (hasTimestamp)
            {
                device.syncData.timeStampms = static_cast<NvU64>(timestampMs);
            }
            status = NvAPI_GPU_ClientIllumDevicesSetControl(handles[i], &control);
            if (status == NVAPI_OK)
            {
                std::printf("  Illum device control updated.\n");
            }
            else
            {
                PrintNvapiError("  NvAPI_GPU_ClientIllumDevicesSetControl failed", status);
            }
        }
        return 0;
    }

    void PrintClientIllumZonesInfo(NvPhysicalGpuHandle handle)
    {
        NV_GPU_CLIENT_ILLUM_ZONE_INFO_PARAMS info = {};
        info.version = NV_GPU_CLIENT_ILLUM_ZONE_INFO_PARAMS_VER;
        NvAPI_Status status = NvAPI_GPU_ClientIllumZonesGetInfo(handle, &info);
        if (status != NVAPI_OK)
        {
            PrintNvapiError("  NvAPI_GPU_ClientIllumZonesGetInfo failed", status);
            return;
        }

        std::printf("  Illum zones: %u\n", info.numIllumZones);
        for (NvU32 i = 0; i < info.numIllumZones && i < NV_GPU_CLIENT_ILLUM_ZONE_NUM_ZONES_MAX; ++i)
        {
            const auto &zone = info.zones[i];
            std::printf("    zone[%u]: type=%s(%u) device=%u provider=%u location=%s(%u)\n",
                i,
                IllumZoneTypeName(zone.type),
                zone.type,
                zone.illumDeviceIdx,
                zone.provIdx,
                IllumZoneLocationName(zone.zoneLocation),
                zone.zoneLocation);
        }
    }

    void PrintClientIllumZonesControl(NvPhysicalGpuHandle handle)
    {
        NV_GPU_CLIENT_ILLUM_ZONE_CONTROL_PARAMS control = {};
        control.version = NV_GPU_CLIENT_ILLUM_ZONE_CONTROL_PARAMS_VER;
        NvAPI_Status status = NvAPI_GPU_ClientIllumZonesGetControl(handle, &control);
        if (status != NVAPI_OK)
        {
            PrintNvapiError("  NvAPI_GPU_ClientIllumZonesGetControl failed", status);
            return;
        }

        std::printf("  Illum zone control: %u\n", control.numIllumZonesControl);
        for (NvU32 i = 0; i < control.numIllumZonesControl && i < NV_GPU_CLIENT_ILLUM_ZONE_NUM_ZONES_MAX; ++i)
        {
            const auto &zone = control.zones[i];
            std::printf("    zone[%u]: type=%s(%u) mode=%s(%u)\n",
                i,
                IllumZoneTypeName(zone.type),
                zone.type,
                IllumCtrlModeName(zone.ctrlMode),
                zone.ctrlMode);

            if (zone.ctrlMode == NV_GPU_CLIENT_ILLUM_CTRL_MODE_MANUAL)
            {
                if (zone.type == NV_GPU_CLIENT_ILLUM_ZONE_TYPE_RGB)
                {
                    const auto &rgb = zone.data.rgb.data.manualRGB.rgbParams;
                    std::printf("      rgb=%u,%u,%u brightness=%u\n",
                        rgb.colorR, rgb.colorG, rgb.colorB, rgb.brightnessPct);
                }
                else if (zone.type == NV_GPU_CLIENT_ILLUM_ZONE_TYPE_RGBW)
                {
                    const auto &rgbw = zone.data.rgbw.data.manualRGBW.rgbwParams;
                    std::printf("      rgbw=%u,%u,%u,%u brightness=%u\n",
                        rgbw.colorR, rgbw.colorG, rgbw.colorB, rgbw.colorW, rgbw.brightnessPct);
                }
                else if (zone.type == NV_GPU_CLIENT_ILLUM_ZONE_TYPE_SINGLE_COLOR)
                {
                    const auto &single = zone.data.singleColor.data.manualSingleColor.singleColorParams;
                    std::printf("      brightness=%u\n", single.brightnessPct);
                }
                else if (zone.type == NV_GPU_CLIENT_ILLUM_ZONE_TYPE_COLOR_FIXED)
                {
                    const auto &fixed = zone.data.colorFixed.data.manualColorFixed.colorFixedParams;
                    std::printf("      brightness=%u\n", fixed.brightnessPct);
                }
            }
        }
    }

    int CmdGpuClientIllumZonesSet(int argc, char **argv)
    {
        NvU32 index = 0;
        bool hasIndex = false;
        NvU32 zoneIndex = 0;
        bool hasZone = false;
        std::string mode;
        bool hasMode = false;
        NvU32 brightness = 0;
        bool hasBrightness = false;
        NvU32 colorR = 0;
        NvU32 colorG = 0;
        NvU32 colorB = 0;
        NvU32 colorW = 0;
        bool hasR = false;
        bool hasG = false;
        bool hasB = false;
        bool hasW = false;
        bool useDefault = false;

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
            if (std::strcmp(argv[i], "--zone") == 0)
            {
                if (i + 1 >= argc)
                {
                    std::printf("Missing value for --zone\n");
                    return 1;
                }
                if (!ParseUint(argv[i + 1], &zoneIndex))
                {
                    std::printf("Invalid zone index: %s\n", argv[i + 1]);
                    return 1;
                }
                hasZone = true;
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
                mode = ToLowerAscii(argv[i + 1]);
                hasMode = true;
                ++i;
                continue;
            }
            if (std::strcmp(argv[i], "--brightness") == 0)
            {
                if (i + 1 >= argc)
                {
                    std::printf("Missing value for --brightness\n");
                    return 1;
                }
                if (!ParseUint(argv[i + 1], &brightness))
                {
                    std::printf("Invalid brightness: %s\n", argv[i + 1]);
                    return 1;
                }
                hasBrightness = true;
                ++i;
                continue;
            }
            if (std::strcmp(argv[i], "--r") == 0)
            {
                if (i + 1 >= argc)
                {
                    std::printf("Missing value for --r\n");
                    return 1;
                }
                if (!ParseUint(argv[i + 1], &colorR))
                {
                    std::printf("Invalid r: %s\n", argv[i + 1]);
                    return 1;
                }
                hasR = true;
                ++i;
                continue;
            }
            if (std::strcmp(argv[i], "--g") == 0)
            {
                if (i + 1 >= argc)
                {
                    std::printf("Missing value for --g\n");
                    return 1;
                }
                if (!ParseUint(argv[i + 1], &colorG))
                {
                    std::printf("Invalid g: %s\n", argv[i + 1]);
                    return 1;
                }
                hasG = true;
                ++i;
                continue;
            }
            if (std::strcmp(argv[i], "--b") == 0)
            {
                if (i + 1 >= argc)
                {
                    std::printf("Missing value for --b\n");
                    return 1;
                }
                if (!ParseUint(argv[i + 1], &colorB))
                {
                    std::printf("Invalid b: %s\n", argv[i + 1]);
                    return 1;
                }
                hasB = true;
                ++i;
                continue;
            }
            if (std::strcmp(argv[i], "--w") == 0)
            {
                if (i + 1 >= argc)
                {
                    std::printf("Missing value for --w\n");
                    return 1;
                }
                if (!ParseUint(argv[i + 1], &colorW))
                {
                    std::printf("Invalid w: %s\n", argv[i + 1]);
                    return 1;
                }
                hasW = true;
                ++i;
                continue;
            }
            if (std::strcmp(argv[i], "--default") == 0)
            {
                useDefault = true;
                continue;
            }
            std::printf("Unknown option: %s\n", argv[i]);
            return 1;
        }

        if (!useDefault)
        {
            if (!hasZone)
            {
                std::printf("Missing required --zone\n");
                return 1;
            }
            if (!hasMode)
            {
                std::printf("Missing required --mode\n");
                return 1;
            }
            if (!hasBrightness)
            {
                std::printf("Missing required --brightness\n");
                return 1;
            }
            if (brightness > 100)
            {
                std::printf("Brightness must be between 0 and 100\n");
                return 1;
            }
            if ((hasR && colorR > 255) || (hasG && colorG > 255) || (hasB && colorB > 255) || (hasW && colorW > 255))
            {
                std::printf("Color values must be between 0 and 255\n");
                return 1;
            }
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
            NV_GPU_CLIENT_ILLUM_ZONE_CONTROL_PARAMS control = {};
            control.version = NV_GPU_CLIENT_ILLUM_ZONE_CONTROL_PARAMS_VER;
            NvAPI_Status status = NvAPI_GPU_ClientIllumZonesGetControl(handles[i], &control);
            if (status != NVAPI_OK)
            {
                PrintNvapiError("  NvAPI_GPU_ClientIllumZonesGetControl failed", status);
                continue;
            }

            if (useDefault)
            {
                control.bDefault = 1;
            }
            else
            {
                if (zoneIndex >= control.numIllumZonesControl)
                {
                    std::printf("  Zone index %u out of range (0-%u)\n",
                        zoneIndex,
                        control.numIllumZonesControl ? control.numIllumZonesControl - 1 : 0);
                    continue;
                }

                auto &zone = control.zones[zoneIndex];
                zone.ctrlMode = NV_GPU_CLIENT_ILLUM_CTRL_MODE_MANUAL;

                if (mode == "manual-rgb")
                {
                    if (zone.type != NV_GPU_CLIENT_ILLUM_ZONE_TYPE_RGB)
                    {
                        std::printf("  Zone %u does not support RGB manual control.\n", zoneIndex);
                        continue;
                    }
                    if (!hasR || !hasG || !hasB)
                    {
                        std::printf("  Missing required --r/--g/--b for manual-rgb\n");
                        continue;
                    }
                    zone.data.rgb.data.manualRGB.rgbParams.colorR = static_cast<NvU8>(colorR);
                    zone.data.rgb.data.manualRGB.rgbParams.colorG = static_cast<NvU8>(colorG);
                    zone.data.rgb.data.manualRGB.rgbParams.colorB = static_cast<NvU8>(colorB);
                    zone.data.rgb.data.manualRGB.rgbParams.brightnessPct = static_cast<NvU8>(brightness);
                }
                else if (mode == "manual-rgbw")
                {
                    if (zone.type != NV_GPU_CLIENT_ILLUM_ZONE_TYPE_RGBW)
                    {
                        std::printf("  Zone %u does not support RGBW manual control.\n", zoneIndex);
                        continue;
                    }
                    if (!hasR || !hasG || !hasB || !hasW)
                    {
                        std::printf("  Missing required --r/--g/--b/--w for manual-rgbw\n");
                        continue;
                    }
                    zone.data.rgbw.data.manualRGBW.rgbwParams.colorR = static_cast<NvU8>(colorR);
                    zone.data.rgbw.data.manualRGBW.rgbwParams.colorG = static_cast<NvU8>(colorG);
                    zone.data.rgbw.data.manualRGBW.rgbwParams.colorB = static_cast<NvU8>(colorB);
                    zone.data.rgbw.data.manualRGBW.rgbwParams.colorW = static_cast<NvU8>(colorW);
                    zone.data.rgbw.data.manualRGBW.rgbwParams.brightnessPct = static_cast<NvU8>(brightness);
                }
                else if (mode == "manual-single")
                {
                    if (zone.type != NV_GPU_CLIENT_ILLUM_ZONE_TYPE_SINGLE_COLOR)
                    {
                        std::printf("  Zone %u does not support single-color manual control.\n", zoneIndex);
                        continue;
                    }
                    zone.data.singleColor.data.manualSingleColor.singleColorParams.brightnessPct = static_cast<NvU8>(brightness);
                }
                else if (mode == "manual-color-fixed")
                {
                    if (zone.type != NV_GPU_CLIENT_ILLUM_ZONE_TYPE_COLOR_FIXED)
                    {
                        std::printf("  Zone %u does not support color-fixed manual control.\n", zoneIndex);
                        continue;
                    }
                    zone.data.colorFixed.data.manualColorFixed.colorFixedParams.brightnessPct = static_cast<NvU8>(brightness);
                }
                else
                {
                    std::printf("  Unknown mode: %s\n", mode.c_str());
                    continue;
                }
            }

            status = NvAPI_GPU_ClientIllumZonesSetControl(handles[i], &control);
            if (status == NVAPI_OK)
            {
                std::printf("  Illum zone control updated.\n");
            }
            else
            {
                PrintNvapiError("  NvAPI_GPU_ClientIllumZonesSetControl failed", status);
            }
        }
        return 0;
    }

    int CmdGpuClientIllumDevices(int argc, char **argv)
    {
        if (argc < 1)
        {
            std::printf("Missing client-illum devices subcommand\n");
            return 1;
        }
        if (std::strcmp(argv[0], "info") == 0)
        {
            NvU32 index = 0;
            bool hasIndex = false;
            if (!ParseGpuIndexArgs(argc - 1, argv + 1, &index, &hasIndex))
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
                PrintClientIllumDevicesInfo(handles[i]);
            }
            return 0;
        }
        if (std::strcmp(argv[0], "control") == 0)
        {
            NvU32 index = 0;
            bool hasIndex = false;
            if (!ParseGpuIndexArgs(argc - 1, argv + 1, &index, &hasIndex))
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
                PrintClientIllumDevicesControl(handles[i]);
            }
            return 0;
        }
        if (std::strcmp(argv[0], "set") == 0)
        {
            return CmdGpuClientIllumDevicesSet(argc - 1, argv + 1);
        }

        std::printf("Unknown client-illum devices subcommand: %s\n", argv[0]);
        return 1;
    }

    int CmdGpuClientIllumZones(int argc, char **argv)
    {
        if (argc < 1)
        {
            std::printf("Missing client-illum zones subcommand\n");
            return 1;
        }
        if (std::strcmp(argv[0], "info") == 0)
        {
            NvU32 index = 0;
            bool hasIndex = false;
            if (!ParseGpuIndexArgs(argc - 1, argv + 1, &index, &hasIndex))
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
                PrintClientIllumZonesInfo(handles[i]);
            }
            return 0;
        }
        if (std::strcmp(argv[0], "control") == 0)
        {
            NvU32 index = 0;
            bool hasIndex = false;
            if (!ParseGpuIndexArgs(argc - 1, argv + 1, &index, &hasIndex))
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
                PrintClientIllumZonesControl(handles[i]);
            }
            return 0;
        }
        if (std::strcmp(argv[0], "set") == 0)
        {
            return CmdGpuClientIllumZonesSet(argc - 1, argv + 1);
        }

        std::printf("Unknown client-illum zones subcommand: %s\n", argv[0]);
        return 1;
    }

    void PrintPowerMonitorInfo(NvPhysicalGpuHandle handle)
    {
        NV_GPU_POWER_MONITOR_GET_INFO info = {};
        info.version = NV_GPU_POWER_MONITOR_GET_INFO_VER;
        NvAPI_Status status = NvAPI_GPU_PowerMonitorGetInfo(handle, &info);
        if (status != NVAPI_OK)
        {
            PrintNvapiError("  NvAPI_GPU_PowerMonitorGetInfo failed", status);
            return;
        }

        std::printf("  Power monitor supported: %u\n", info.bSupported);
        std::printf("  Sampling: period=%u ms count=%u\n", info.samplingPeriodms, info.sampleCount);
        std::printf("  Channel mask: 0x%08X\n", info.channelMask);
        std::printf("  Channel rel mask: 0x%08X\n", info.chRelMask);
        std::printf("  Total GPU mask: 0x%08X total channel idx=%u\n",
            info.totalGpuPowerChannelMask,
            info.totalGpuChannelIdx);

        for (NvU32 i = 0; i < NV_GPU_POWER_MONITOR_MAX_CHANNELS_V2; ++i)
        {
            if (!HasBit(info.channelMask, i))
            {
                continue;
            }
            const auto &channel = info.channels[i];
            std::printf("    channel[%u]: type=%s(%u) rail=%s(0x%02X) offset=%d mW limit=%u mW\n",
                i,
                PowerChannelTypeName(channel.type),
                channel.type,
                PowerChannelRailName(static_cast<NV_GPU_POWER_CHANNEL_POWER_RAIL>(channel.pwrRail)),
                channel.pwrRail,
                channel.pwrOffsetmW,
                channel.pwrLimitmW);
        }
    }

    void PrintPowerMonitorStatus(NvPhysicalGpuHandle handle)
    {
        NV_GPU_POWER_MONITOR_GET_INFO info = {};
        info.version = NV_GPU_POWER_MONITOR_GET_INFO_VER;
        NvAPI_Status status = NvAPI_GPU_PowerMonitorGetInfo(handle, &info);
        if (status != NVAPI_OK)
        {
            PrintNvapiError("  NvAPI_GPU_PowerMonitorGetInfo failed", status);
            return;
        }

        NV_GPU_POWER_MONITOR_GET_STATUS statusData = {};
        statusData.version = NV_GPU_POWER_MONITOR_GET_STATUS_VER;
        statusData.channelMask = info.channelMask;
        status = NvAPI_GPU_PowerMonitorGetStatus(handle, &statusData);
        if (status != NVAPI_OK)
        {
            PrintNvapiError("  NvAPI_GPU_PowerMonitorGetStatus failed", status);
            return;
        }

        std::printf("  Total GPU power: %u mW\n", statusData.totalGpuPowermW);
        for (NvU32 i = 0; i < NV_GPU_POWER_MONITOR_MAX_CHANNELS_V2; ++i)
        {
            if (!HasBit(statusData.channelMask, i))
            {
                continue;
            }
            const auto &channel = statusData.channels[i];
            std::printf("    channel[%u]: avg=%u min=%u max=%u mW curr=%u mA volt=%u uV energy=%llu mJ\n",
                i,
                channel.pwrAvgmW,
                channel.pwrMinmW,
                channel.pwrMaxmW,
                channel.currmA,
                channel.voltuV,
                static_cast<unsigned long long>(channel.energymJ));
            std::printf("      tuple: pwr=%u mW curr=%u mA volt=%u uV energy=%llu mJ\n",
                channel.tuple.pwrmW,
                channel.tuple.currmA,
                channel.tuple.voltuV,
                static_cast<unsigned long long>(channel.tuple.energymJ));
            std::printf("      polled: pwr=%u mW curr=%u mA volt=%u uV energy=%llu mJ\n",
                channel.tuplePolled.pwrmW,
                channel.tuplePolled.currmA,
                channel.tuplePolled.voltuV,
                static_cast<unsigned long long>(channel.tuplePolled.energymJ));
        }
    }

    void PrintPowerDeviceInfo(NvPhysicalGpuHandle handle)
    {
        NV_GPU_POWER_DEVICE_GET_INFO info = {};
        info.version = NV_GPU_POWER_DEVICE_GET_INFO_VER;
        NvAPI_Status status = NvAPI_GPU_PowerDeviceGetInfo(handle, &info);
        if (status != NVAPI_OK)
        {
            PrintNvapiError("  NvAPI_GPU_PowerDeviceGetInfo failed", status);
            return;
        }

        std::printf("  Power device mask: 0x%08X\n", info.pwrDeviceMask);
        for (NvU32 i = 0; i < NV_GPU_POWER_DEVICE_MAX_DEVICES_V1; ++i)
        {
            if (!HasBit(info.pwrDeviceMask, i))
            {
                continue;
            }
            const auto &device = info.devices[i];
            std::printf("    device[%u]: type=%s(%u) rail=%s(0x%02X)\n",
                i,
                PowerDeviceTypeName(static_cast<NV_GPU_POWER_DEVICE_TYPE>(device.type)),
                device.type,
                PowerDeviceRailName(static_cast<NV_GPU_POWER_DEVICE_POWER_RAIL>(device.powerRail)),
                device.powerRail);
        }
    }

    void PrintPowerDeviceStatus(NvPhysicalGpuHandle handle)
    {
        NV_GPU_POWER_DEVICE_GET_INFO info = {};
        info.version = NV_GPU_POWER_DEVICE_GET_INFO_VER;
        NvAPI_Status status = NvAPI_GPU_PowerDeviceGetInfo(handle, &info);
        if (status != NVAPI_OK)
        {
            PrintNvapiError("  NvAPI_GPU_PowerDeviceGetInfo failed", status);
            return;
        }

        NV_GPU_POWER_DEVICE_GET_STATUS statusData = {};
        statusData.version = NV_GPU_POWER_DEVICE_GET_STATUS_VER;
        MaskE32Clear(statusData.pwrDeviceMask);
        for (NvU32 i = 0; i < NV_GPU_POWER_DEVICE_MAX_DEVICES_V1; ++i)
        {
            if (HasBit(info.pwrDeviceMask, i))
            {
                MaskE32Set(statusData.pwrDeviceMask, i);
            }
        }

        status = NvAPI_GPU_PowerDeviceGetStatus(handle, &statusData);
        if (status != NVAPI_OK)
        {
            PrintNvapiError("  NvAPI_GPU_PowerDeviceGetStatus failed", status);
            return;
        }

        std::printf("  Power device status:\n");
        for (NvU32 i = 0; i < NV_GPU_POWER_DEVICE_MAX_DEVICES_V1; ++i)
        {
            if (!MaskE32Has(statusData.pwrDeviceMask, i))
            {
                continue;
            }
            const auto &device = statusData.devices[i];
            std::printf("    device[%u]: type=%s(%u) providers=%u\n",
                i,
                PowerDeviceTypeName(device.type),
                device.type,
                device.numProviders);
            for (NvU32 p = 0; p < device.numProviders && p < NV_GPU_POWER_DEVICE_MAX_PROVIDERS_V3; ++p)
            {
                const auto &provider = device.providers[p];
                std::printf("      provider[%u]: pwr=%u mW curr=%u mA volt=%u uV thresholds=%u\n",
                    p,
                    provider.tuple.pwrmW,
                    provider.tuple.currmA,
                    provider.tuple.voltuV,
                    provider.numThresholds);
            }
        }
    }

    void PrintPowerCappingInfo(NvPhysicalGpuHandle handle)
    {
        (void)handle;
        NV_GPU_POWER_CAPPING_GET_INFO info = {};
        info.version = NV_GPU_POWER_CAPPING_GET_INFO_VER;
        NvAPI_Status status = NVAPI_NOT_SUPPORTED;
        // Deprecated in release 400: NvAPI_GPU_PowerCappingGetInfo(handle, &info);
        if (status != NVAPI_OK)
        {
            PrintNvapiError("  NvAPI_GPU_PowerCappingGetInfo failed", status);
            return;
        }

        std::printf("  Power capping: alg=%u channelMask=0x%08X\n", info.alg, info.channelMask);
    }

    void PrintPowerCappingSlowdownStatus(NvPhysicalGpuHandle handle)
    {
        (void)handle;
        NV_GPU_POWER_CAPPING_SLOWDOWN_GET_STATUS info = {};
        info.version = NV_GPU_POWER_CAPPING_SLOWDOWN_GET_STATUS_VER;
        NvAPI_Status status = NVAPI_NOT_SUPPORTED;
        // Deprecated in release 400: NvAPI_GPU_PowerCappingSlowdownGetStatus(handle, &info);
        if (status != NVAPI_OK)
        {
            PrintNvapiError("  NvAPI_GPU_PowerCappingSlowdownGetStatus failed", status);
            return;
        }

        std::printf("  Power capping slowdown: enabled=%u capRatio=%u factorIndex=%u factorCount=%u\n",
            info.bEnabled,
            info.capRatio,
            info.factorIndex,
            info.factorCount);
        for (NvU32 i = 0; i < info.factorCount && i < NV_GPU_POWER_CAPPING_SLOWDOWN_MAX_CHANNELS_V1; ++i)
        {
            std::printf("    factor[%u]: %u/%u\n", i, info.factors[i].numerator, info.factors[i].denominator);
        }
    }

    void PrintPowerLeakageInfo(NvPhysicalGpuHandle handle)
    {
        NV_GPU_POWER_LEAKAGE_INFO_PARAMS info = {};
        info.version = NV_GPU_POWER_LEAKAGE_INFO_PARAMS_VER;
        NvAPI_Status status = NvAPI_GPU_PowerLeakageGetInfo(handle, &info);
        if (status != NVAPI_OK)
        {
            PrintNvapiError("  NvAPI_GPU_PowerLeakageGetInfo failed", status);
            return;
        }

        std::printf("  Leakage: iddqVer=%u swVer=%u iddqmA=%u mask=0x%08X\n",
            info.hwIddqVersion,
            info.iddqVersion,
            info.iddqmA,
            info.leakageMask);

        for (NvU32 i = 0; i < NV_GPU_POWER_LEAKAGE_MAX_LEAKAGES_V2; ++i)
        {
            if (!HasBit(info.leakageMask, i))
            {
                continue;
            }
            const auto &leak = info.leakages[i];
            std::printf("    leakage[%u]: type=%s(%u) fsEff=%u pgEff=%u\n",
                i,
                LeakageTypeName(leak.type),
                leak.type,
                leak.fsEff,
                leak.pgEff);
        }
    }

    void PrintPowerLeakageStatus(NvPhysicalGpuHandle handle)
    {
        (void)handle;
        NV_GPU_POWER_LEAKAGE_INFO_PARAMS info = {};
        info.version = NV_GPU_POWER_LEAKAGE_INFO_PARAMS_VER;
        NvAPI_Status status = NvAPI_GPU_PowerLeakageGetInfo(handle, &info);
        if (status != NVAPI_OK)
        {
            PrintNvapiError("  NvAPI_GPU_PowerLeakageGetInfo failed", status);
            return;
        }

        NV_GPU_POWER_LEAKAGE_STATUS_PARAMS statusData = {};
        statusData.version = NV_GPU_POWER_LEAKAGE_STATUS_PARAMS_VER;
        NvU32 leakMask = info.leakageMask;
        if (NV_GPU_POWER_LEAKAGE_MAX_LEAKAGES_V1 < 32)
        {
            leakMask &= ((1u << NV_GPU_POWER_LEAKAGE_MAX_LEAKAGES_V1) - 1u);
        }
        statusData.leakageMask = leakMask;
        status = NVAPI_NOT_SUPPORTED;
        // Deprecated in release 295: NvAPI_GPU_PowerLeakageGetStatus(handle, &statusData);
        if (status != NVAPI_OK)
        {
            PrintNvapiError("  NvAPI_GPU_PowerLeakageGetStatus failed", status);
            return;
        }

        std::printf("  Leakage status: tj=%.2f C\n", static_cast<float>(statusData.tj) / 256.0f);
        for (NvU32 i = 0; i < NV_GPU_POWER_LEAKAGE_MAX_LEAKAGES_V1; ++i)
        {
            if (!HasBit(statusData.leakageMask, i))
            {
                continue;
            }
            const auto &leak = statusData.leakages[i];
            std::printf("    leakage[%u]: entries=%u\n", i, leak.numVoltEntries);
            for (NvU32 v = 0; v < leak.numVoltEntries && v < NV_GPU_POWER_LEAKAGE_MAX_VOLTAGE_ENTRIES_V2; ++v)
            {
                const auto &entry = leak.voltEntries[v];
                std::printf("      %u uV -> %u mW\n", entry.voltageuV, entry.pwrLeakagemW);
            }
        }
    }

    int CmdGpuClientFan(int argc, char **argv)
    {
        if (argc < 1)
        {
            std::printf("Missing client-fan subcommand\n");
            return 1;
        }
        if (std::strcmp(argv[0], "coolers") == 0)
        {
            return CmdGpuClientFanCoolers(argc - 1, argv + 1);
        }
        if (std::strcmp(argv[0], "policies") == 0)
        {
            return CmdGpuClientFanPolicies(argc - 1, argv + 1);
        }
        if (std::strcmp(argv[0], "arbiters") == 0)
        {
            return CmdGpuClientFanArbiters(argc - 1, argv + 1);
        }
        std::printf("Unknown client-fan subcommand: %s\n", argv[0]);
        return 1;
    }

    int CmdGpuClientIllum(int argc, char **argv)
    {
        if (argc < 1)
        {
            std::printf("Missing client-illum subcommand\n");
            return 1;
        }
        if (std::strcmp(argv[0], "devices") == 0)
        {
            return CmdGpuClientIllumDevices(argc - 1, argv + 1);
        }
        if (std::strcmp(argv[0], "zones") == 0)
        {
            return CmdGpuClientIllumZones(argc - 1, argv + 1);
        }
        std::printf("Unknown client-illum subcommand: %s\n", argv[0]);
        return 1;
    }

    int CmdGpuPowerMonitor(int argc, char **argv)
    {
        if (argc < 1)
        {
            std::printf("Missing power monitor subcommand\n");
            return 1;
        }
        NvU32 index = 0;
        bool hasIndex = false;
        if (!ParseGpuIndexArgs(argc - 1, argv + 1, &index, &hasIndex))
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
            if (std::strcmp(argv[0], "info") == 0)
            {
                PrintPowerMonitorInfo(handles[i]);
            }
            else if (std::strcmp(argv[0], "status") == 0)
            {
                PrintPowerMonitorStatus(handles[i]);
            }
            else
            {
                std::printf("Unknown power monitor subcommand: %s\n", argv[0]);
                return 1;
            }
        }
        return 0;
    }

    int CmdGpuPowerDevice(int argc, char **argv)
    {
        if (argc < 1)
        {
            std::printf("Missing power device subcommand\n");
            return 1;
        }
        NvU32 index = 0;
        bool hasIndex = false;
        if (!ParseGpuIndexArgs(argc - 1, argv + 1, &index, &hasIndex))
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
            if (std::strcmp(argv[0], "info") == 0)
            {
                PrintPowerDeviceInfo(handles[i]);
            }
            else if (std::strcmp(argv[0], "status") == 0)
            {
                PrintPowerDeviceStatus(handles[i]);
            }
            else
            {
                std::printf("Unknown power device subcommand: %s\n", argv[0]);
                return 1;
            }
        }
        return 0;
    }

    int CmdGpuPowerCapping(int argc, char **argv)
    {
        if (argc < 1)
        {
            std::printf("Missing power capping subcommand\n");
            return 1;
        }
        NvU32 index = 0;
        bool hasIndex = false;
        if (!ParseGpuIndexArgs(argc - 1, argv + 1, &index, &hasIndex))
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
            if (std::strcmp(argv[0], "info") == 0)
            {
                PrintPowerCappingInfo(handles[i]);
            }
            else if (std::strcmp(argv[0], "slowdown") == 0)
            {
                PrintPowerCappingSlowdownStatus(handles[i]);
            }
            else
            {
                std::printf("Unknown power capping subcommand: %s\n", argv[0]);
                return 1;
            }
        }
        return 0;
    }

    int CmdGpuPowerLeakage(int argc, char **argv)
    {
        if (argc < 1)
        {
            std::printf("Missing power leakage subcommand\n");
            return 1;
        }
        NvU32 index = 0;
        bool hasIndex = false;
        if (!ParseGpuIndexArgs(argc - 1, argv + 1, &index, &hasIndex))
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
            if (std::strcmp(argv[0], "info") == 0)
            {
                PrintPowerLeakageInfo(handles[i]);
            }
            else if (std::strcmp(argv[0], "status") == 0)
            {
                PrintPowerLeakageStatus(handles[i]);
            }
            else
            {
                std::printf("Unknown power leakage subcommand: %s\n", argv[0]);
                return 1;
            }
        }
        return 0;
    }
}

