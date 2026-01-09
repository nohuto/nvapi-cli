/*
 * Copyright (c) 2026 Noverse (Nohuto). All rights reserved.
 * Proprietary and confidential. Unauthorized copying or redistribution is strictly prohibited.
 */

#include "commands.h"

#include <cctype>
#include <string>
#include <vector>

namespace nvcli
{
    namespace
    {
        const char *EccConfigName(NV_ECC_CONFIGURATION config)
        {
            switch (config)
            {
            case NV_ECC_CONFIGURATION_NOT_SUPPORTED:
                return "NOT_SUPPORTED";
            case NV_ECC_CONFIGURATION_DEFERRED:
                return "DEFERRED";
            case NV_ECC_CONFIGURATION_IMMEDIATE:
                return "IMMEDIATE";
            default:
                return "UNKNOWN";
            }
        }

        const char *PcieAspmStateName(NV_PCIE_ASPM_STATE state)
        {
            switch (state)
            {
            case NV_PCIE_ASPM_STATE_DISABLED:
                return "DISABLED";
            case NV_PCIE_ASPM_STATE_L0S:
                return "L0S";
            case NV_PCIE_ASPM_STATE_L1:
                return "L1";
            case NV_PCIE_ASPM_STATE_L0S_L1:
                return "L0S_L1";
            default:
                return "UNKNOWN";
            }
        }

        const char *PcieGenerationName(NV_PCIE_GENERATION gen)
        {
            switch (gen)
            {
            case NV_PCIE_GENERATION_GEN_1:
                return "GEN1";
            case NV_PCIE_GENERATION_GEN_2:
                return "GEN2";
            case NV_PCIE_GENERATION_GEN_3:
                return "GEN3";
            case NV_PCIE_GENERATION_GEN_4:
                return "GEN4";
            case NV_PCIE_GENERATION_GEN_NONE:
                return "NONE";
            default:
                return "UNKNOWN";
            }
        }

        const char *SurpriseRemovalBusName(NV_KNOWN_SURPRISE_REMOVAL_BUS_TYPE type)
        {
            switch (type)
            {
            case NV_KNOWN_SURPRISE_REMOVAL_BUS_TYPE_UNKNOWN:
                return "UNKNOWN";
            case NV_KNOWN_SURPRISE_REMOVAL_BUS_TYPE_TB2:
                return "TB2";
            case NV_KNOWN_SURPRISE_REMOVAL_BUS_TYPE_TB3:
                return "TB3";
            case NV_KNOWN_SURPRISE_REMOVAL_BUS_TYPE_OEM_SPECIFIC:
                return "OEM";
            default:
                return "UNKNOWN";
            }
        }

        std::string BuildFlagList(NvU32 mask, const std::vector<std::pair<NvU32, const char *>> &flags)
        {
            std::string out;
            for (const auto &item : flags)
            {
                if ((mask & item.first) == 0)
                {
                    continue;
                }
                if (!out.empty())
                {
                    out += ", ";
                }
                out += item.second;
            }
            if (out.empty())
            {
                out = "none";
            }
            return out;
        }

        std::string BufferToAscii(const NvU8 *data, size_t size, bool *hasNonPrintable)
        {
            if (hasNonPrintable)
            {
                *hasNonPrintable = false;
            }
            if (!data || size == 0)
            {
                return std::string();
            }

            size_t length = 0;
            while (length < size && data[length] != 0)
            {
                ++length;
            }

            std::string out;
            out.reserve(length);
            for (size_t i = 0; i < length; ++i)
            {
                unsigned char ch = data[i];
                if (std::isprint(ch))
                {
                    out.push_back(static_cast<char>(ch));
                }
                else
                {
                    if (hasNonPrintable)
                    {
                        *hasNonPrintable = true;
                    }
                    out.push_back('.');
                }
            }
            return out;
        }

        void PrintPcieLinkInfo(const char *label, const NV_PCIE_LINK_INFO_V2 &info)
        {
            std::printf("  %s:\n", label);
            std::printf("    maxSpeed=%u Mbps maxWidth=%u gen=%s aspm=%s\n",
                info.maxSpeed,
                info.maxWidth,
                PcieGenerationName(info.maxGeneration),
                PcieAspmStateName(info.supportedStates));
            std::printf("    currentSpeed=%u Mbps currentWidth=%u gen=%s aspm=%s\n",
                info.currentSpeed,
                info.currentWidth,
                PcieGenerationName(info.currentGeneration),
                PcieAspmStateName(info.currentState));
        }

        void PrintPcieErrorMask(const char *label, NvU32 mask)
        {
            std::vector<std::pair<NvU32, const char *>> flags = {
                {NV_PCIE_LINK_ERROR_CORR_ERROR, "CORR"},
                {NV_PCIE_LINK_ERROR_NON_FATAL_ERROR, "NON_FATAL"},
                {NV_PCIE_LINK_ERROR_FATAL_ERROR, "FATAL"},
                {NV_PCIE_LINK_ERROR_UNSUPP_REQUEST, "UNSUPPORTED_REQ"},
            };
            std::printf("  %s: 0x%08X (%s)\n", label, mask, BuildFlagList(mask, flags).c_str());
        }

        void PrintPcieAERMask(NvU32 mask)
        {
            std::vector<std::pair<NvU32, const char *>> flags = {
                {NV_PCIE_LINK_AER_UNCORR_TRAINING_ERR, "UNCORR_TRAINING"},
                {NV_PCIE_LINK_AER_UNCORR_DLINK_PROTO_ERR, "UNCORR_DLINK"},
                {NV_PCIE_LINK_AER_UNCORR_POISONED_TLP, "UNCORR_POISONED_TLP"},
                {NV_PCIE_LINK_AER_UNCORR_FC_PROTO_ERR, "UNCORR_FC_PROTO"},
                {NV_PCIE_LINK_AER_UNCORR_CPL_TIMEOUT, "UNCORR_CPL_TIMEOUT"},
                {NV_PCIE_LINK_AER_UNCORR_CPL_ABORT, "UNCORR_CPL_ABORT"},
                {NV_PCIE_LINK_AER_UNCORR_UNEXP_CPL, "UNCORR_UNEXP_CPL"},
                {NV_PCIE_LINK_AER_UNCORR_RCVR_OVERFLOW, "UNCORR_RCVR_OVERFLOW"},
                {NV_PCIE_LINK_AER_UNCORR_MALFORMED_TLP, "UNCORR_MALFORMED_TLP"},
                {NV_PCIE_LINK_AER_UNCORR_ECRC_ERROR, "UNCORR_ECRC"},
                {NV_PCIE_LINK_AER_UNCORR_UNSUPPORTED_REQ, "UNCORR_UNSUPPORTED_REQ"},
                {NV_PCIE_LINK_AER_CORR_RCV_ERR, "CORR_RCV"},
                {NV_PCIE_LINK_AER_CORR_BAD_TLP, "CORR_BAD_TLP"},
                {NV_PCIE_LINK_AER_CORR_BAD_DLLP, "CORR_BAD_DLLP"},
                {NV_PCIE_LINK_AER_CORR_RPLY_ROLLOVER, "CORR_RPLY_ROLLOVER"},
                {NV_PCIE_LINK_AER_CORR_RPLY_TIMEOUT, "CORR_RPLY_TIMEOUT"},
                {NV_PCIE_LINK_AER_CORR_ADVISORY_NONFATAL, "CORR_ADVISORY_NONFATAL"},
            };
            std::printf("  AER: 0x%08X (%s)\n", mask, BuildFlagList(mask, flags).c_str());
        }
    }

    int CmdGpuEccStatus(int argc, char **argv)
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
            NV_GPU_ECC_STATUS_INFO info = {};
            info.version = NV_GPU_ECC_STATUS_INFO_VER;
            NvAPI_Status status = NvAPI_GPU_GetECCStatusInfo(handles[i], &info);
            if (status != NVAPI_OK)
            {
                PrintNvapiError("  NvAPI_GPU_GetECCStatusInfo failed", status);
                continue;
            }

            std::printf("  ECC supported: %s\n", info.isSupported ? "yes" : "no");
            std::printf("  ECC enabled: %s\n", info.isEnabled ? "yes" : "no");
            std::printf("  ECC configuration: %s (%u)\n", EccConfigName(info.configurationOptions), info.configurationOptions);
        }

        return 0;
    }

    int CmdGpuEccErrors(int argc, char **argv)
    {
        NvU32 index = 0;
        bool hasIndex = false;
        bool raw = false;
        for (int i = 0; i < argc; ++i)
        {
            if (std::strcmp(argv[i], "--raw") == 0)
            {
                raw = true;
                continue;
            }
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
            std::printf("Unknown option: %s\n", argv[i]);
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
            NV_GPU_ECC_ERROR_INFO info = {};
            info.version = NV_GPU_ECC_ERROR_INFO_VER;
            NvAPI_Status status = NVAPI_OK;
            if (raw)
            {
                status = NvAPI_GPU_GetECCErrorInfoEx(handles[i], NVAPI_GPU_ECC_STATUS_FLAGS_TYPE_RAW, &info);
            }
            else
            {
                status = NvAPI_GPU_GetECCErrorInfo(handles[i], &info);
            }

            if (status != NVAPI_OK)
            {
                PrintNvapiError("  NvAPI_GPU_GetECCErrorInfo failed", status);
                continue;
            }

            std::printf("  ECC errors (current): single=%llu double=%llu\n",
                static_cast<unsigned long long>(info.current.singleBitErrors),
                static_cast<unsigned long long>(info.current.doubleBitErrors));
            std::printf("  ECC errors (aggregate): single=%llu double=%llu\n",
                static_cast<unsigned long long>(info.aggregate.singleBitErrors),
                static_cast<unsigned long long>(info.aggregate.doubleBitErrors));
        }

        return 0;
    }

    int CmdGpuEccConfig(int argc, char **argv)
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
            NV_GPU_ECC_CONFIGURATION_INFO info = {};
            info.version = NV_GPU_ECC_CONFIGURATION_INFO_VER;
            NvAPI_Status status = NvAPI_GPU_GetECCConfigurationInfo(handles[i], &info);
            if (status != NVAPI_OK)
            {
                PrintNvapiError("  NvAPI_GPU_GetECCConfigurationInfo failed", status);
                continue;
            }
            std::printf("  ECC config stored: %s\n", info.isEnabled ? "enabled" : "disabled");
            std::printf("  ECC default: %s\n", info.isEnabledByDefault ? "enabled" : "disabled");
        }
        return 0;
    }

    int CmdGpuEccSet(int argc, char **argv)
    {
        NvU32 index = 0;
        bool hasIndex = false;
        bool hasEnable = false;
        bool enable = false;
        bool immediate = false;
        bool hasClear = false;
        bool clear = false;

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
            if (std::strcmp(argv[i], "--immediate") == 0)
            {
                if (i + 1 >= argc)
                {
                    std::printf("Missing value for --immediate\n");
                    return 1;
                }
                if (!ParseBoolValue(argv[i + 1], &immediate))
                {
                    std::printf("Invalid value for --immediate: %s\n", argv[i + 1]);
                    return 1;
                }
                ++i;
                continue;
            }
            if (std::strcmp(argv[i], "--clear") == 0)
            {
                if (i + 1 >= argc)
                {
                    std::printf("Missing value for --clear\n");
                    return 1;
                }
                if (!ParseBoolValue(argv[i + 1], &clear))
                {
                    std::printf("Invalid value for --clear: %s\n", argv[i + 1]);
                    return 1;
                }
                hasClear = true;
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
        if (!hasClear)
        {
            clear = !enable;
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
            NV_GPU_ECC_STATUS_INFO statusInfo = {};
            statusInfo.version = NV_GPU_ECC_STATUS_INFO_VER;
            NvAPI_Status status = NvAPI_GPU_GetECCStatusInfo(handles[i], &statusInfo);
            if (status == NVAPI_OK && !statusInfo.isSupported)
            {
                std::printf("  ECC not supported on this GPU.\n");
                continue;
            }
            if (status != NVAPI_OK)
            {
                PrintNvapiError("  NvAPI_GPU_GetECCStatusInfo failed", status);
                continue;
            }

            status = NvAPI_GPU_SetECCConfiguration(handles[i], enable ? 1 : 0, immediate ? 1 : 0);
            if (status != NVAPI_OK)
            {
                PrintNvapiError("  NvAPI_GPU_SetECCConfiguration failed", status);
                if (status == NVAPI_INVALID_USER_PRIVILEGE)
                {
                    std::printf("  Try running the terminal as Administrator.\n");
                }
                continue;
            }

            std::printf("  ECC %s requested (immediate=%u).\n", enable ? "enable" : "disable", immediate ? 1 : 0);

            if (clear)
            {
                status = NvAPI_GPU_ResetECCErrorInfo(handles[i], 1, 1);
                if (status != NVAPI_OK)
                {
                    PrintNvapiError("  NvAPI_GPU_ResetECCErrorInfo failed", status);
                    continue;
                }
                std::printf("  ECC counters reset.\n");
            }
        }

        return 0;
    }

    int CmdGpuEccReset(int argc, char **argv)
    {
        NvU32 index = 0;
        bool hasIndex = false;
        bool resetCurrent = false;
        bool resetAggregate = false;

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
            if (std::strcmp(argv[i], "--current") == 0)
            {
                if (i + 1 >= argc)
                {
                    std::printf("Missing value for --current\n");
                    return 1;
                }
                if (!ParseBoolValue(argv[i + 1], &resetCurrent))
                {
                    std::printf("Invalid value for --current: %s\n", argv[i + 1]);
                    return 1;
                }
                ++i;
                continue;
            }
            if (std::strcmp(argv[i], "--aggregate") == 0)
            {
                if (i + 1 >= argc)
                {
                    std::printf("Missing value for --aggregate\n");
                    return 1;
                }
                if (!ParseBoolValue(argv[i + 1], &resetAggregate))
                {
                    std::printf("Invalid value for --aggregate: %s\n", argv[i + 1]);
                    return 1;
                }
                ++i;
                continue;
            }
            std::printf("Unknown option: %s\n", argv[i]);
            return 1;
        }

        if (!resetCurrent && !resetAggregate)
        {
            std::printf("Missing --current and/or --aggregate.\n");
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
            NvAPI_Status status = NvAPI_GPU_ResetECCErrorInfo(handles[i], resetCurrent ? 1 : 0, resetAggregate ? 1 : 0);
            if (status != NVAPI_OK)
            {
                PrintNvapiError("  NvAPI_GPU_ResetECCErrorInfo failed", status);
                continue;
            }
            std::printf("  ECC counters reset.\n");
        }
        return 0;
    }

    int CmdGpuEcc(int argc, char **argv)
    {
        if (argc < 1)
        {
            std::printf("Missing ecc subcommand.\n");
            return 1;
        }
        if (std::strcmp(argv[0], "status") == 0)
        {
            return CmdGpuEccStatus(argc - 1, argv + 1);
        }
        if (std::strcmp(argv[0], "errors") == 0)
        {
            return CmdGpuEccErrors(argc - 1, argv + 1);
        }
        if (std::strcmp(argv[0], "config") == 0)
        {
            return CmdGpuEccConfig(argc - 1, argv + 1);
        }
        if (std::strcmp(argv[0], "set") == 0)
        {
            return CmdGpuEccSet(argc - 1, argv + 1);
        }
        if (std::strcmp(argv[0], "reset") == 0)
        {
            return CmdGpuEccReset(argc - 1, argv + 1);
        }
        std::printf("Unknown ecc subcommand: %s\n", argv[0]);
        return 1;
    }

    int CmdGpuBoardInfo(int argc, char **argv)
    {
        NvU32 index = 0;
        bool hasIndex = false;
        bool raw = false;
        for (int i = 0; i < argc; ++i)
        {
            if (std::strcmp(argv[i], "--raw") == 0)
            {
                raw = true;
                continue;
            }
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
            std::printf("Unknown option: %s\n", argv[i]);
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
            NV_BOARD_INFO_V3 info = {};
            NvU32 version = NV_BOARD_INFO_VER;
            info.version = version;
            NvAPI_Status status = NvAPI_GPU_GetBoardInfo(handles[i], reinterpret_cast<NV_BOARD_INFO *>(&info));
            if (status == NVAPI_INCOMPATIBLE_STRUCT_VERSION)
            {
                version = NV_BOARD_INFO_VER2;
                info.version = version;
                status = NvAPI_GPU_GetBoardInfo(handles[i], reinterpret_cast<NV_BOARD_INFO *>(&info));
            }
            if (status == NVAPI_INCOMPATIBLE_STRUCT_VERSION)
            {
                version = NV_BOARD_INFO_VER1;
                info.version = version;
                status = NvAPI_GPU_GetBoardInfo(handles[i], reinterpret_cast<NV_BOARD_INFO *>(&info));
            }
            if (status != NVAPI_OK)
            {
                PrintNvapiError("  NvAPI_GPU_GetBoardInfo failed", status);
                continue;
            }

            bool nonPrintable = false;
            std::string boardNum = BufferToAscii(info.BoardNum, sizeof(info.BoardNum), &nonPrintable);
            std::printf("  Board serial: %s\n", boardNum.empty() ? "<empty>" : boardNum.c_str());
            if (nonPrintable)
            {
                std::printf("  Board serial (hex):\n");
                PrintHexBytes(info.BoardNum, static_cast<NvU32>(sizeof(info.BoardNum)));
            }

            NvU64 serial = 0;
            status = NvAPI_GPU_GetSerialNumber(handles[i], &serial);
            if (status == NVAPI_OK)
            {
                std::printf("  GPU serial: 0x%016llX\n", static_cast<unsigned long long>(serial));
            }

            if (version == NV_BOARD_INFO_VER2 || version == NV_BOARD_INFO_VER3)
            {
                bool partNonPrintable = false;
                std::string part = BufferToAscii(info.productPartNumber, sizeof(info.productPartNumber), &partNonPrintable);
                std::printf("  Product part: %s\n", part.empty() ? "<empty>" : part.c_str());
                std::printf("  Board revision: %u.%u.%u\n", info.boardRevision[0], info.boardRevision[1], info.boardRevision[2]);
                std::printf("  Board type: %u\n", info.boardType);
                if (raw || partNonPrintable)
                {
                    std::printf("  Product part (hex):\n");
                    PrintHexBytes(info.productPartNumber, static_cast<NvU32>(sizeof(info.productPartNumber)));
                }
            }

            if (version == NV_BOARD_INFO_VER3 && raw)
            {
                std::printf("  OEM info (hex):\n");
                PrintHexBytes(info.oemInfo, static_cast<NvU32>(sizeof(info.oemInfo)));
            }
        }
        return 0;
    }

    int CmdGpuBoardManufacturing(int argc, char **argv)
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
            NV_MANUFACTURING_INFO info = {};
            info.version = NV_MANUFACTURING_INFO_VER;
            NvAPI_Status status = NvAPI_GPU_ManufacturingInfo(handles[i], &info);
            if (status != NVAPI_OK)
            {
                PrintNvapiError("  NvAPI_GPU_ManufacturingInfo failed", status);
                continue;
            }
            std::printf("  ECID: 0x%016llX%016llX\n",
                static_cast<unsigned long long>(info.ecid[1]),
                static_cast<unsigned long long>(info.ecid[0]));
        }
        return 0;
    }

    int CmdGpuBoard(int argc, char **argv)
    {
        if (argc < 1)
        {
            std::printf("Missing board subcommand.\n");
            return 1;
        }
        if (std::strcmp(argv[0], "mfg") == 0)
        {
            return CmdGpuBoardManufacturing(argc - 1, argv + 1);
        }
        std::printf("Unknown board subcommand: %s\n", argv[0]);
        return 1;
    }

    int CmdGpuPcieInfo(int argc, char **argv)
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
            NV_PCIE_INFO info = {};
            info.version = NV_PCIE_INFO_VER;
            NvAPI_Status status = NvAPI_GPU_GetPCIEInfo(handles[i], &info);
            if (status != NVAPI_OK)
            {
                PrintNvapiError("  NvAPI_GPU_GetPCIEInfo failed", status);
                continue;
            }

            PrintPcieLinkInfo("Root", info.root);
            PrintPcieLinkInfo("GPU", info.gpu);
            PrintPcieLinkInfo("Upstream", info.upstream);
            PrintPcieLinkInfo("Downstream", info.downstream);
            PrintPcieLinkInfo("Board", info.board);

            std::printf("  Surprise removal: vendor=0x%04X device=0x%04X supported=%s bus=%s\n",
                info.rootPortSurpriseRemovalCaps.vendorId,
                info.rootPortSurpriseRemovalCaps.deviceId,
                info.rootPortSurpriseRemovalCaps.isSurpriseRemovalSupported ? "yes" : "no",
                SurpriseRemovalBusName(info.rootPortSurpriseRemovalCaps.removalBusType));
        }
        return 0;
    }

    int CmdGpuPcieErrors(int argc, char **argv)
    {
        NvU32 index = 0;
        bool hasIndex = false;
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
            std::printf("Unknown option: %s\n", argv[i]);
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
            NV_PCIE_LINK_ERROR_INFO info = {};
            info.version = NV_PCIE_LINK_ERROR_INFO_VER;
            NvAPI_Status status = NvAPI_GPU_ClearPCIELinkErrorInfo(handles[i], &info);
            if (status != NVAPI_OK)
            {
                PrintNvapiError("  NvAPI_GPU_ClearPCIELinkErrorInfo failed", status);
                continue;
            }

            PrintPcieErrorMask("Root", info.root);
            PrintPcieErrorMask("GPU", info.gpu);
            std::printf("  Flags: 0x%08X\n", info.flags);
        }
        return 0;
    }

    int CmdGpuPcieAer(int argc, char **argv)
    {
        NvU32 index = 0;
        bool hasIndex = false;
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
            std::printf("Unknown option: %s\n", argv[i]);
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
            NvU32 mask = 0;
            NvAPI_Status status = NvAPI_GPU_ClearPCIELinkAERInfo(handles[i], &mask);
            if (status != NVAPI_OK)
            {
                PrintNvapiError("  NvAPI_GPU_ClearPCIELinkAERInfo failed", status);
                continue;
            }
            PrintPcieAERMask(mask);
        }
        return 0;
    }

    int CmdGpuPcieSwitchErrors(int argc, char **argv)
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
            NV_PCIELINK_SWITCH_ERROR_INFO info = {};
            info.version = NV_PCIELINK_SWITCH_ERROR_INFO_VER;
            NvAPI_Status status = NvAPI_GPU_GetPCIELinkSwitchErrorInfo(handles[i], &info);
            if (status != NVAPI_OK)
            {
                PrintNvapiError("  NvAPI_GPU_GetPCIELinkSwitchErrorInfo failed", status);
                continue;
            }
            std::printf("  Speed switch errors: %u\n", info.speedSwitchErrorCount);
            std::printf("  Width switch errors: %u\n", info.widthSwitchErrorCount);
        }
        return 0;
    }



    int CmdGpuPcieSetWidth(int argc, char **argv)
    {
        NvU32 index = 0;
        bool hasIndex = false;
        NvU32 width = 0;
        bool hasWidth = false;

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
            if (std::strcmp(argv[i], "--width") == 0)
            {
                if (i + 1 >= argc)
                {
                    std::printf("Missing value for --width\n");
                    return 1;
                }
                if (!ParseUint(argv[i + 1], &width))
                {
                    std::printf("Invalid width: %s\n", argv[i + 1]);
                    return 1;
                }
                hasWidth = true;
                ++i;
                continue;
            }
            std::printf("Unknown option: %s\n", argv[i]);
            return 1;
        }

        if (!hasWidth)
        {
            std::printf("Missing required --width\n");
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
            NvAPI_Status status = NvAPI_GPU_SetCurrentPCIEWidth(handles[i], width);
            if (status != NVAPI_OK)
            {
                PrintNvapiError("  NvAPI_GPU_SetCurrentPCIEWidth failed", status);
                if (status == NVAPI_INVALID_USER_PRIVILEGE)
                {
                    std::printf("  Try running the terminal as Administrator.\n");
                }
                continue;
            }
            std::printf("  PCIe width set: x%u\n", width);
        }

        return 0;
    }

    int CmdGpuPcieSetSpeed(int argc, char **argv)
    {
        NvU32 index = 0;
        bool hasIndex = false;
        NvU32 speed = 0;
        bool hasSpeed = false;

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
            if (std::strcmp(argv[i], "--speed") == 0)
            {
                if (i + 1 >= argc)
                {
                    std::printf("Missing value for --speed\n");
                    return 1;
                }
                if (!ParseUint(argv[i + 1], &speed))
                {
                    std::printf("Invalid speed: %s\n", argv[i + 1]);
                    return 1;
                }
                hasSpeed = true;
                ++i;
                continue;
            }
            std::printf("Unknown option: %s\n", argv[i]);
            return 1;
        }

        if (!hasSpeed)
        {
            std::printf("Missing required --speed\n");
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
            NvAPI_Status status = NvAPI_GPU_SetCurrentPCIESpeed(handles[i], speed);
            if (status != NVAPI_OK)
            {
                PrintNvapiError("  NvAPI_GPU_SetCurrentPCIESpeed failed", status);
                if (status == NVAPI_INVALID_USER_PRIVILEGE)
                {
                    std::printf("  Try running the terminal as Administrator.\n");
                }
                continue;
            }
            std::printf("  PCIe speed set: %u Mbps\n", speed);
        }

        return 0;
    }
    int CmdGpuPcie(int argc, char **argv)
    {
        if (argc < 1)
        {
            std::printf("Missing pcie subcommand.\n");
            return 1;
        }
        if (std::strcmp(argv[0], "info") == 0)
        {
            return CmdGpuPcieInfo(argc - 1, argv + 1);
        }
        if (std::strcmp(argv[0], "switch-errors") == 0)
        {
            return CmdGpuPcieSwitchErrors(argc - 1, argv + 1);
        }
        if (std::strcmp(argv[0], "errors") == 0)
        {
            return CmdGpuPcieErrors(argc - 1, argv + 1);
        }
        if (std::strcmp(argv[0], "aer") == 0)
        {
            return CmdGpuPcieAer(argc - 1, argv + 1);
        }
        if (std::strcmp(argv[0], "aspm-set") == 0)
        {
            return CmdGpuPcieAspmSet(argc - 1, argv + 1);
        }
        if (std::strcmp(argv[0], "width-set") == 0)
        {
            return CmdGpuPcieSetWidth(argc - 1, argv + 1);
        }
        if (std::strcmp(argv[0], "speed-set") == 0)
        {
            return CmdGpuPcieSetSpeed(argc - 1, argv + 1);
        }
        std::printf("Unknown pcie subcommand: %s\n", argv[0]);
        return 1;
    }
}

