/*
 * Copyright (c) 2026 Noverse (Nohuto). All rights reserved.
 * Proprietary and confidential. Unauthorized copying or redistribution is strictly prohibited.
 */

#include "commands.h"

namespace nvcli
{
    namespace
    {
        class PcfSession
        {
        public:
            PcfSession()
                : m_status(NvAPI_InitializeEx(NV_PLATFORM_DRIVER))
            {
            }

            ~PcfSession()
            {
                if (m_status == NVAPI_OK)
                {
                    NvAPI_UnloadEx(NV_PLATFORM_DRIVER);
                }
            }

            bool ok() const
            {
                return m_status == NVAPI_OK;
            }

            NvAPI_Status status() const
            {
                return m_status;
            }

        private:
            NvAPI_Status m_status;
        };

        void PrintPcfUsage()
        {
            PrintUsageGroup("pcf");
        }

        bool PcfMaskHas(const NV_PCF_BOARDOBJGRP_MASK_E32 &mask, NvU32 index)
        {
            if (index >= NV_PCF_BOARDOBJGRP_E32_MAX_OBJECTS)
            {
                return false;
            }
            return (mask.super.pData[0] & (1u << index)) != 0;
        }

        const char *PcfMasterTypeName(NvU8 type)
        {
            switch (type)
            {
            case NV_PCF_MASTER_CPU_FREQUENCY_CONTROL_TYPE_ACPI_ECIO:
                return "CPU_FREQUENCY_CONTROL";
            case NV_PCF_MASTER_CPU_DATA_COLLECTOR:
                return "CPU_DATA_COLLECTOR";
            case NV_PCF_MASTER_WM2_PLATFORM_PARAMS:
                return "WM2_PLATFORM_PARAMS";
            case NV_PCF_MASTER_CPU_TDP_LIMIT_CONTROL:
                return "CPU_TDP_LIMIT_CONTROL";
            default:
                return "UNKNOWN";
            }
        }

        bool FetchPcfMasterInfo(NV_PCF_MASTER_INFO_PARAMS *info)
        {
            if (!info)
            {
                return false;
            }
            std::memset(info, 0, sizeof(*info));
            info->ver = NV_PCF_MASTER_INFO_PARAMS_VER;
            NvAPI_Status status = NvAPI_PCF_MasterGetInfo(info);
            if (status != NVAPI_OK)
            {
                PrintNvapiError("NvAPI_PCF_MasterGetInfo failed", status);
                return false;
            }
            return true;
        }

        int CmdPcfMasterInfo()
        {
            PcfSession session;
            if (!session.ok())
            {
                PrintNvapiError("NvAPI_InitializeEx(NV_PLATFORM_DRIVER) failed", session.status());
                return 1;
            }

            NV_PCF_MASTER_INFO_PARAMS info = {};
            if (!FetchPcfMasterInfo(&info))
            {
                return 1;
            }

            std::printf("PCF master info:\n");
            for (NvU32 i = 0; i < NV_PCF_BOARDOBJGRP_E32_MAX_OBJECTS; ++i)
            {
                if (!PcfMaskHas(info.super.objMask, i))
                {
                    continue;
                }

                const auto &entry = info.infoParams[i];
                const NvU8 type = entry.super.type;
                std::printf("  master[%u] type=%s (%u)\n", i, PcfMasterTypeName(type), type);

                if (type == NV_PCF_MASTER_CPU_FREQUENCY_CONTROL_TYPE_ACPI_ECIO)
                {
                    std::printf("    busRatioHigh=%u busRatioNominal=%u offset=0x%X\n",
                        entry.info.infoData.busRatioHigh,
                        entry.info.infoData.busRatioNominal,
                        entry.info.infoData.offSet);
                }
                else if (type == NV_PCF_MASTER_CPU_DATA_COLLECTOR)
                {
                    std::printf("    activeLogicalCpus=%llu version=%u\n",
                        static_cast<unsigned long long>(entry.info.collectorData.activeLogicalCpus),
                        entry.info.collectorData.version);
                }
                else if (type == NV_PCF_MASTER_WM2_PLATFORM_PARAMS)
                {
                    std::printf("    supportedSlsMask=0x%02X tableTbandMask=0x%02X fieldsMask=0x%08X\n",
                        entry.info.infoWm2.supportedSlsMask,
                        entry.info.infoWm2.tableTbandMask,
                        entry.info.infoWm2.supportedFieldsMask);
                }
                else if (type == NV_PCF_MASTER_CPU_TDP_LIMIT_CONTROL)
                {
                    std::printf("    cpuTdpLimitControl=0x%08X\n",
                        entry.info.infoCpuTdpLimitControl.rsvd);
                }
            }

            return 0;
        }

        int CmdPcfMasterControl()
        {
            PcfSession session;
            if (!session.ok())
            {
                PrintNvapiError("NvAPI_InitializeEx(NV_PLATFORM_DRIVER) failed", session.status());
                return 1;
            }

            NV_PCF_MASTER_INFO_PARAMS info = {};
            if (!FetchPcfMasterInfo(&info))
            {
                return 1;
            }

            NV_PCF_MASTER_CONTROL_PARAMS control = {};
            control.ver = NV_PCF_MASTER_CONTROL_PARAMS_VER;
            control.super.objMask = info.super.objMask;

            NvAPI_Status status = NvAPI_PCF_MasterGetControl(&control);
            if (status != NVAPI_OK)
            {
                PrintNvapiError("NvAPI_PCF_MasterGetControl failed", status);
                return 1;
            }

            std::printf("PCF master control:\n");
            for (NvU32 i = 0; i < NV_PCF_BOARDOBJGRP_E32_MAX_OBJECTS; ++i)
            {
                if (!PcfMaskHas(control.super.objMask, i))
                {
                    continue;
                }

                const auto &entry = control.ctrlParams[i];
                const NvU8 type = entry.super.type;
                std::printf("  master[%u] type=%s (%u)\n", i, PcfMasterTypeName(type), type);

                if (type == NV_PCF_MASTER_CPU_FREQUENCY_CONTROL_TYPE_ACPI_ECIO)
                {
                    std::printf("    busRatioHigh=%u busRatioNominal=%u\n",
                        entry.control.controlData.busRatioHigh,
                        entry.control.controlData.busRatioNominal);
                }
                else if (type == NV_PCF_MASTER_WM2_PLATFORM_PARAMS)
                {
                    std::printf("    slModeCurr=%u cpuTjLimitCurr=0x%08X\n",
                        entry.control.controlWm2.slModeCurr,
                        entry.control.controlWm2.cpuTjLimitCurr);
                }
                else if (type == NV_PCF_MASTER_CPU_DATA_COLLECTOR)
                {
                    std::printf("    dataCollectorSubFunction=%u\n",
                        entry.control.collectorData.subFunction);
                }
                else if (type == NV_PCF_MASTER_CPU_TDP_LIMIT_CONTROL)
                {
                    std::printf("    cpuTdpLimitControl=0x%08X\n",
                        entry.control.controlCpuTdpLimitControl.rsvd);
                }
            }

            return 0;
        }

        int CmdPcfMasterStatus()
        {
            PcfSession session;
            if (!session.ok())
            {
                PrintNvapiError("NvAPI_InitializeEx(NV_PLATFORM_DRIVER) failed", session.status());
                return 1;
            }

            NV_PCF_MASTER_INFO_PARAMS info = {};
            if (!FetchPcfMasterInfo(&info))
            {
                return 1;
            }

            NV_PCF_MASTER_STATUS_PARAMS statusParams = {};
            statusParams.ver = NV_PCF_MASTER_STATUS_PARAMS_VER;
            statusParams.super.objMask = info.super.objMask;

            NvAPI_Status status = NvAPI_PCF_MasterGetStatus(&statusParams);
            if (status != NVAPI_OK)
            {
                PrintNvapiError("NvAPI_PCF_MasterGetStatus failed", status);
                return 1;
            }

            std::printf("PCF master status:\n");
            for (NvU32 i = 0; i < NV_PCF_BOARDOBJGRP_E32_MAX_OBJECTS; ++i)
            {
                if (!PcfMaskHas(statusParams.super.objMask, i))
                {
                    continue;
                }

                const auto &entry = statusParams.statusParams[i];
                const NvU8 type = entry.super.type;
                std::printf("  master[%u] type=%s (%u)\n", i, PcfMasterTypeName(type), type);

                if (type == NV_PCF_MASTER_WM2_PLATFORM_PARAMS)
                {
                    const auto &wm2 = entry.status.statusWm2;
                    std::printf("    platformEnabled=%u updateEventCount=%u cpuTj=%u gpuTj=%u sysPower=%uW\n",
                        wm2.bPlatformEnabled,
                        wm2.updateEventCount,
                        wm2.cpuTj,
                        wm2.gpuTj,
                        wm2.sysTspW);
                }
            }

            return 0;
        }

        int CmdPcfMasterSet(int argc, char **argv)
        {
            NvU32 index = 0;
            bool hasIndex = false;
            NvU32 busHigh = 0;
            bool hasHigh = false;
            NvU32 busNominal = 0;
            bool hasNominal = false;

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
                        std::printf("Invalid index: %s\n", argv[i + 1]);
                        return 1;
                    }
                    hasIndex = true;
                    ++i;
                    continue;
                }
                if (std::strcmp(argv[i], "--bus-high") == 0)
                {
                    if (i + 1 >= argc)
                    {
                        std::printf("Missing value for --bus-high\n");
                        return 1;
                    }
                    if (!ParseUint(argv[i + 1], &busHigh))
                    {
                        std::printf("Invalid bus-high: %s\n", argv[i + 1]);
                        return 1;
                    }
                    hasHigh = true;
                    ++i;
                    continue;
                }
                if (std::strcmp(argv[i], "--bus-nominal") == 0)
                {
                    if (i + 1 >= argc)
                    {
                        std::printf("Missing value for --bus-nominal\n");
                        return 1;
                    }
                    if (!ParseUint(argv[i + 1], &busNominal))
                    {
                        std::printf("Invalid bus-nominal: %s\n", argv[i + 1]);
                        return 1;
                    }
                    hasNominal = true;
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
            if (!hasHigh && !hasNominal)
            {
                std::printf("Missing bus ratio update (--bus-high and/or --bus-nominal).\n");
                return 1;
            }
            if (busHigh > 0xFF || busNominal > 0xFF)
            {
                std::printf("Bus ratio values must be 0-255.\n");
                return 1;
            }

            PcfSession session;
            if (!session.ok())
            {
                PrintNvapiError("NvAPI_InitializeEx(NV_PLATFORM_DRIVER) failed", session.status());
                return 1;
            }

            NV_PCF_MASTER_INFO_PARAMS info = {};
            if (!FetchPcfMasterInfo(&info))
            {
                return 1;
            }

            if (index >= NV_PCF_BOARDOBJGRP_E32_MAX_OBJECTS || !PcfMaskHas(info.super.objMask, index))
            {
                std::printf("Master index not present: %u\n", index);
                return 1;
            }

            NV_PCF_MASTER_CONTROL_PARAMS control = {};
            control.ver = NV_PCF_MASTER_CONTROL_PARAMS_VER;
            control.super.objMask = info.super.objMask;

            NvAPI_Status status = NvAPI_PCF_MasterGetControl(&control);
            if (status != NVAPI_OK)
            {
                PrintNvapiError("NvAPI_PCF_MasterGetControl failed", status);
                return 1;
            }

            auto &entry = control.ctrlParams[index];
            if (entry.super.type != NV_PCF_MASTER_CPU_FREQUENCY_CONTROL_TYPE_ACPI_ECIO)
            {
                std::printf("Master[%u] type %u does not support CPU frequency control.\n", index, entry.super.type);
                return 1;
            }

            if (hasHigh)
            {
                entry.control.controlData.busRatioHigh = static_cast<NvU8>(busHigh);
            }
            if (hasNominal)
            {
                entry.control.controlData.busRatioNominal = static_cast<NvU8>(busNominal);
            }

            std::printf("PCF master set request: index=%u busHigh=%u busNominal=%u\n",
                index,
                entry.control.controlData.busRatioHigh,
                entry.control.controlData.busRatioNominal);

            control.super.objMask.super.pData[0] = (1u << index);
            status = NvAPI_PCF_MasterSetControl(&control);
            if (status != NVAPI_OK)
            {
                PrintNvapiError("NvAPI_PCF_MasterSetControl failed", status);
                return 1;
            }

            std::printf("PCF master updated.\n");
            return 0;
        }

        int CmdPcfMasterInfoAdapter(int argc, char **argv)
        {
            (void)argc;
            (void)argv;
            return CmdPcfMasterInfo();
        }

        int CmdPcfMasterControlAdapter(int argc, char **argv)
        {
            (void)argc;
            (void)argv;
            return CmdPcfMasterControl();
        }

        int CmdPcfMasterStatusAdapter(int argc, char **argv)
        {
            (void)argc;
            (void)argv;
            return CmdPcfMasterStatus();
        }

        int CmdPcfMaster(int argc, char **argv)
        {
            if (argc < 1)
            {
                PrintUsageGroup("pcf");
                return 1;
            }

            static const SubcommandEntry kSubcommands[] = {
                {"info", CmdPcfMasterInfoAdapter},
                {"control", CmdPcfMasterControlAdapter},
                {"status", CmdPcfMasterStatusAdapter},
                {"set", CmdPcfMasterSet},
            };

            return DispatchSubcommand("pcf master", argc, argv, kSubcommands, sizeof(kSubcommands) / sizeof(kSubcommands[0]), PrintPcfUsage);
        }
    }

    int CmdPcf(int argc, char **argv)
    {
        if (argc < 1)
        {
            PrintUsageGroup("pcf");
            return 1;
        }

        static const SubcommandEntry kSubcommands[] = {
            {"master", CmdPcfMaster},
        };

        return DispatchSubcommand("pcf", argc, argv, kSubcommands, sizeof(kSubcommands) / sizeof(kSubcommands[0]), PrintPcfUsage);
    }
}

