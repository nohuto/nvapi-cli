/*
 * Copyright (c) 2026 Noverse (Nohuto). All rights reserved.
 * Proprietary and confidential. Unauthorized copying or redistribution is strictly prohibited.
 */

#include "cli/commands.h"

#include <cstring>

namespace nvcli
{
    namespace
    {
        static bool ParseSysSmpArgs(int argc, char **argv, bool *useDefault)
        {
            if (!useDefault)
            {
                return false;
            }
            *useDefault = false;
            for (int i = 0; i < argc; ++i)
            {
                if (std::strcmp(argv[i], "--default") == 0)
                {
                    *useDefault = true;
                    continue;
                }
                std::printf("Unknown sys smp option: %s\n", argv[i]);
                return false;
            }
            return true;
        }

        static int CmdSysGpuCount()
        {
            NV_GPU_COUNT_DATA data = {};
            data.version = NV_GPU_COUNT_DATA_VER;
            NvAPI_Status status = NvAPI_SYS_GetGpuCount(&data);
            if (status != NVAPI_OK)
            {
                PrintNvapiError("NvAPI_SYS_GetGpuCount failed", status);
                return 1;
            }
            std::printf("GPU count:\n");
            std::printf("  fromBus=%u\n", data.gpuCountFromBus);
            std::printf("  active=%u\n", data.activeGpuCount);
            return 0;
        }

        static int CmdSysSmpInfo(int argc, char **argv)
        {
            bool useDefault = false;
            if (!ParseSysSmpArgs(argc, argv, &useDefault))
            {
                std::printf("Usage: %s sys smp [--default]\n", kToolName);
                return 1;
            }
            NV_SMP_INFO info = {};
            info.version = NV_SMP_INFO_VER;
            info.bDefault = useDefault ? 1u : 0u;
            NvAPI_Status status = NvAPI_SYS_GetSMPInfo(&info);
            if (status != NVAPI_OK)
            {
                PrintNvapiError("NvAPI_SYS_GetSMPInfo failed", status);
                return 1;
            }
            std::printf("SMP info:\n");
            std::printf("  tilt=%.3f\n", info.tilt);
            std::printf("  verticalFOV=%.3f\n", info.verticalFOV);
            std::printf("  cameraMode=%u\n", info.cameraMode);
            std::printf("  default=%u\n", info.bDefault);
            return 0;
        }

        static int CmdSysChipsetSliBondInfo()
        {
            NV_CHIPSET_SLI_BOND_INFO info = {};
            info.version = NV_CHIPSET_SLI_BOND_INFO_VER;
            NvAPI_Status status = NvAPI_SYS_GetChipSetSliBondInfo(&info);
            if (status != NVAPI_OK)
            {
                PrintNvapiError("NvAPI_SYS_GetChipSetSliBondInfo failed", status);
                return 1;
            }
            size_t len = 0;
            for (; len < sizeof(info.szSliBondName); ++len)
            {
                if (info.szSliBondName[len] == '\0')
                {
                    break;
                }
            }
            std::printf("Chipset SLI bond:\n");
            std::printf("  bondId=%u\n", info.sliBondId);
            if (len == 0)
            {
                std::printf("  name=<empty>\n");
            }
            else
            {
                std::printf("  name=%.*s\n", static_cast<int>(len), info.szSliBondName);
            }
            return 0;
        }

        static void PrintSysUsage()
        {
            PrintUsageGroup("sys");
        }
    }

    int CmdSys(int argc, char **argv)
    {
        if (argc < 1)
        {
            PrintSysUsage();
            return 1;
        }
        if (std::strcmp(argv[0], "help") == 0)
        {
            PrintSysUsage();
            return 0;
        }
        if (std::strcmp(argv[0], "gpu-count") == 0)
        {
            return CmdSysGpuCount();
        }
        if (std::strcmp(argv[0], "smp") == 0)
        {
            return CmdSysSmpInfo(argc - 1, argv + 1);
        }
        if (std::strcmp(argv[0], "chipset-sli") == 0)
        {
            return CmdSysChipsetSliBondInfo();
        }
        std::printf("Unknown sys command: %s\n", argv[0]);
        PrintSysUsage();
        return 1;
    }
}
