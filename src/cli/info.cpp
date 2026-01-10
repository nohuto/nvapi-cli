/*
 * Copyright (c) 2026 Noverse (Nohuto). All rights reserved.
 * Proprietary and confidential. Unauthorized copying or redistribution is strictly prohibited.
 */

#include "cli/commands.h"

namespace nvcli
{
    int CmdInfo()
    {
        NvAPI_ShortString iface = {0};
        NvAPI_Status status = NvAPI_GetInterfaceVersionString(iface);
        if (status != NVAPI_OK)
        {
            PrintNvapiError("NvAPI_GetInterfaceVersionString failed", status);
            return 1;
        }

        NvU32 driverVersion = 0;
        NvAPI_ShortString branch = {0};
        status = NvAPI_SYS_GetDriverAndBranchVersion(&driverVersion, branch);
        if (status != NVAPI_OK)
        {
            PrintNvapiError("NvAPI_SYS_GetDriverAndBranchVersion failed", status);
            return 1;
        }

        std::printf("NVAPI Interface: %s\n", iface);
        std::printf("Driver Version: %u\n", driverVersion);
        std::printf("Driver Branch: %s\n", branch);
        return 0;
    }

    void PrintGpuHeader(NvU32 index, NvPhysicalGpuHandle handle)
    {
        NvAPI_ShortString name = {0};
        NvAPI_Status status = NvAPI_GPU_GetFullName(handle, name);
        if (status != NVAPI_OK)
        {
            strncpy_s(name, sizeof(name), "<name unavailable>", _TRUNCATE);
        }
        std::printf("GPU[%u] %s\n", index, name);
    }

    void PrintBusInfo(NvPhysicalGpuHandle handle)
    {
        NvU32 deviceId = 0;
        NvU32 subSystemId = 0;
        NvU32 revisionId = 0;
        NvU32 extDeviceId = 0;
        NvAPI_Status status = NvAPI_GPU_GetPCIIdentifiers(handle, &deviceId, &subSystemId, &revisionId, &extDeviceId);
        if (status == NVAPI_OK)
        {
            std::printf("  PCI: device=0x%04X subsystem=0x%08X revision=0x%02X ext=0x%08X\n",
                deviceId, subSystemId, revisionId, extDeviceId);
        }
        else
        {
            PrintNvapiError("  NvAPI_GPU_GetPCIIdentifiers failed", status);
        }

        NV_GPU_BUS_TYPE busType = NVAPI_GPU_BUS_TYPE_UNDEFINED;
        status = NvAPI_GPU_GetBusType(handle, &busType);
        if (status == NVAPI_OK)
        {
            std::printf("  Bus: %s\n", BusTypeName(busType));
        }
        else
        {
            PrintNvapiError("  NvAPI_GPU_GetBusType failed", status);
        }

        NvU32 busId = 0;
        status = NvAPI_GPU_GetBusId(handle, &busId);
        if (status == NVAPI_OK)
        {
            std::printf("  Bus ID: %u\n", busId);
        }
        else
        {
            PrintNvapiError("  NvAPI_GPU_GetBusId failed", status);
        }

        NvU32 busSlotId = 0;
        status = NvAPI_GPU_GetBusSlotId(handle, &busSlotId);
        if (status == NVAPI_OK)
        {
            std::printf("  Bus Slot: %u\n", busSlotId);
        }
        else
        {
            PrintNvapiError("  NvAPI_GPU_GetBusSlotId failed", status);
        }

        NvU32 irq = 0;
        status = NvAPI_GPU_GetIRQ(handle, &irq);
        if (status == NVAPI_OK)
        {
            std::printf("  IRQ: %u\n", irq);
        }
        else
        {
            PrintNvapiError("  NvAPI_GPU_GetIRQ failed", status);
        }
    }

    void PrintVbiosInfo(NvPhysicalGpuHandle handle)
    {
        NvAPI_ShortString vbios = {0};
        NvAPI_Status status = NvAPI_GPU_GetVbiosVersionString(handle, vbios);
        if (status == NVAPI_OK)
        {
            std::printf("  VBIOS: %s\n", vbios);
        }
        else
        {
            PrintNvapiError("  NvAPI_GPU_GetVbiosVersionString failed", status);
        }

        NvU32 revision = 0;
        status = NvAPI_GPU_GetVbiosRevision(handle, &revision);
        if (status == NVAPI_OK)
        {
            std::printf("  VBIOS Revision: 0x%08X\n", revision);
        }
        else
        {
            PrintNvapiError("  NvAPI_GPU_GetVbiosRevision failed", status);
        }

        NvU32 oemRevision = 0;
        status = NvAPI_GPU_GetVbiosOEMRevision(handle, &oemRevision);
        if (status == NVAPI_OK)
        {
            std::printf("  VBIOS OEM Revision: 0x%08X\n", oemRevision);
        }
        else
        {
            PrintNvapiError("  NvAPI_GPU_GetVbiosOEMRevision failed", status);
        }
    }

    void PrintMemoryInfo(NvPhysicalGpuHandle handle)
    {
        NV_DISPLAY_DRIVER_MEMORY_INFO memory = {};
        memory.version = NV_DISPLAY_DRIVER_MEMORY_INFO_VER;
        NvAPI_Status status = NvAPI_GPU_GetMemoryInfo(handle, &memory);
        if (status == NVAPI_INCOMPATIBLE_STRUCT_VERSION)
        {
            memory.version = NV_DISPLAY_DRIVER_MEMORY_INFO_VER_2;
            status = NvAPI_GPU_GetMemoryInfo(handle, &memory);
        }
        if (status == NVAPI_INCOMPATIBLE_STRUCT_VERSION)
        {
            memory.version = NV_DISPLAY_DRIVER_MEMORY_INFO_VER_1;
            status = NvAPI_GPU_GetMemoryInfo(handle, &memory);
        }

        if (status == NVAPI_OK)
        {
            std::printf("  Memory (driver):\n");
            std::printf("    dedicated: %u KB (%.1f MiB)\n", memory.dedicatedVideoMemory, KBToMiB(memory.dedicatedVideoMemory));
            std::printf("    available: %u KB (%.1f MiB)\n", memory.availableDedicatedVideoMemory, KBToMiB(memory.availableDedicatedVideoMemory));
            std::printf("    current available: %u KB (%.1f MiB)\n", memory.curAvailableDedicatedVideoMemory, KBToMiB(memory.curAvailableDedicatedVideoMemory));
            std::printf("    system: %u KB (%.1f MiB)\n", memory.systemVideoMemory, KBToMiB(memory.systemVideoMemory));
            std::printf("    shared: %u KB (%.1f MiB)\n", memory.sharedSystemMemory, KBToMiB(memory.sharedSystemMemory));
            std::printf("    eviction size: %u KB\n", memory.dedicatedVideoMemoryEvictionsSize);
            std::printf("    eviction count: %u\n", memory.dedicatedVideoMemoryEvictionCount);
        }
        else
        {
            PrintNvapiError("  NvAPI_GPU_GetMemoryInfo failed", status);
        }

        NvU32 physicalFb = 0;
        status = NvAPI_GPU_GetPhysicalFrameBufferSize(handle, &physicalFb);
        if (status == NVAPI_OK)
        {
            std::printf("  Framebuffer (physical): %u KB (%.1f MiB)\n", physicalFb, KBToMiB(physicalFb));
        }
        else
        {
            PrintNvapiError("  NvAPI_GPU_GetPhysicalFrameBufferSize failed", status);
        }

        NvU32 virtualFb = 0;
        status = NvAPI_GPU_GetVirtualFrameBufferSize(handle, &virtualFb);
        if (status == NVAPI_OK)
        {
            std::printf("  Framebuffer (virtual): %u KB (%.1f MiB)\n", virtualFb, KBToMiB(virtualFb));
        }
        else
        {
            PrintNvapiError("  NvAPI_GPU_GetVirtualFrameBufferSize failed", status);
        }

        NV_GPU_RAM_TYPE ramType = NV_GPU_RAM_TYPE_UNKNOWN;
        status = NvAPI_GPU_GetRamType(handle, &ramType);
        if (status == NVAPI_OK)
        {
            std::printf("  RAM Type: %s\n", RamTypeName(ramType));
        }
        else
        {
            PrintNvapiError("  NvAPI_GPU_GetRamType failed", status);
        }

        NvU32 ramBusWidth = 0;
        status = NvAPI_GPU_GetRamBusWidth(handle, &ramBusWidth);
        if (status == NVAPI_OK)
        {
            std::printf("  RAM Bus Width: %u-bit\n", ramBusWidth);
        }
        else
        {
            PrintNvapiError("  NvAPI_GPU_GetRamBusWidth failed", status);
        }

        NvU32 ramBankCount = 0;
        status = NvAPI_GPU_GetRamBankCount(handle, &ramBankCount);
        if (status == NVAPI_OK)
        {
            std::printf("  RAM Bank Count: %u\n", ramBankCount);
        }
        else
        {
            PrintNvapiError("  NvAPI_GPU_GetRamBankCount failed", status);
        }
    }

    void PrintBarInfo(NvPhysicalGpuHandle handle)
    {
        NV_GPU_BAR_INFO barInfo = {};
        barInfo.version = NV_GPU_BAR_INFO_VER;
        NvAPI_Status status = NvAPI_GPU_GetBarInfo(handle, &barInfo);
        if (status == NVAPI_INCOMPATIBLE_STRUCT_VERSION)
        {
            barInfo.version = NV_GPU_BAR_INFO_VER_1;
            status = NvAPI_GPU_GetBarInfo(handle, &barInfo);
        }

        if (status != NVAPI_OK)
        {
            PrintNvapiError("  NvAPI_GPU_GetBarInfo failed", status);
            return;
        }

        std::printf("  BARs: %u\n", barInfo.count);
        for (NvU32 i = 0; i < barInfo.count && i < NV_GPU_MAX_BAR_COUNT; ++i)
        {
            std::printf("    BAR[%u]: size=%.1f MiB offset=0x%llX\n",
                i,
                BytesToMiB(barInfo.barInfo[i].barSizeBytes),
                static_cast<unsigned long long>(barInfo.barInfo[i].barOffset));
        }
    }

    void PrintPowerInfo(NvPhysicalGpuHandle handle)
    {
        NvU32 connectorCount = 0;
        NvU32 connectionAtBoot = 0;
        NvU32 currentConnection = 0;
        NvAPI_Status status = NvAPI_GPU_GetPowerConnectorStatus(handle, &connectorCount, &connectionAtBoot, &currentConnection);
        if (status == NVAPI_OK)
        {
            std::printf("  Power connectors: %u\n", connectorCount);
            std::printf("  Power connected (boot): 0x%08X\n", connectionAtBoot);
            std::printf("  Power connected (current): 0x%08X\n", currentConnection);
        }
        else
        {
            PrintNvapiError("  NvAPI_GPU_GetPowerConnectorStatus failed", status);
        }
    }

    void PrintPstateInfo(NvPhysicalGpuHandle handle)
    {
        NV_GPU_PERF_PSTATE_ID pstate = NVAPI_GPU_PERF_PSTATE_UNDEFINED;
        NvAPI_Status status = NvAPI_GPU_GetCurrentPstate(handle, &pstate);
        if (status == NVAPI_OK)
        {
            std::printf("  Current Pstate: %s\n", PstateName(pstate));
        }
        else
        {
            PrintNvapiError("  NvAPI_GPU_GetCurrentPstate failed", status);
        }
    }

    void PrintUtilizationInfo(NvPhysicalGpuHandle handle)
    {
        NV_GPU_DYNAMIC_PSTATES_INFO_EX pstates = {};
        pstates.version = NV_GPU_DYNAMIC_PSTATES_INFO_EX_VER;
        NvAPI_Status status = NvAPI_GPU_GetDynamicPstatesInfoEx(handle, &pstates);
        if (status != NVAPI_OK)
        {
            PrintNvapiError("  NvAPI_GPU_GetDynamicPstatesInfoEx failed", status);
            return;
        }

        std::printf("  Utilization (dynamic Pstate): %s\n", (pstates.flags & 0x1) ? "enabled" : "disabled");
        for (NvU32 i = 0; i < NVAPI_MAX_GPU_UTILIZATIONS; ++i)
        {
            if (!pstates.utilization[i].bIsPresent)
            {
                continue;
            }
            std::printf("    %s: %u%%\n", UtilizationDomainName(i), pstates.utilization[i].percentage);
        }
    }

    void PrintClockInfo(NvPhysicalGpuHandle handle)
    {
        NV_GPU_CLOCK_FREQUENCIES clocks = {};
        clocks.version = NV_GPU_CLOCK_FREQUENCIES_VER;
        clocks.ClockType = NV_GPU_CLOCK_FREQUENCIES_CURRENT_FREQ;
        NvAPI_Status status = NvAPI_GPU_GetAllClockFrequencies(handle, &clocks);
        if (status == NVAPI_INCOMPATIBLE_STRUCT_VERSION)
        {
            clocks.version = NV_GPU_CLOCK_FREQUENCIES_VER_2;
            status = NvAPI_GPU_GetAllClockFrequencies(handle, &clocks);
        }
        if (status == NVAPI_INCOMPATIBLE_STRUCT_VERSION)
        {
            clocks.version = NV_GPU_CLOCK_FREQUENCIES_VER_1;
            status = NvAPI_GPU_GetAllClockFrequencies(handle, &clocks);
        }

        if (status != NVAPI_OK)
        {
            PrintNvapiError("  NvAPI_GPU_GetAllClockFrequencies failed", status);
            return;
        }

        std::printf("  Clocks (kHz):\n");
        for (NvU32 i = 0; i < NVAPI_MAX_GPU_PUBLIC_CLOCKS; ++i)
        {
            if (!clocks.domain[i].bIsPresent)
            {
                continue;
            }
            std::printf("    %s[%u]: %u kHz\n", ClockDomainName(i), i, clocks.domain[i].frequency);
        }
    }

    void PrintCoolerInfo(NvPhysicalGpuHandle handle)
    {
        NV_GPU_GETCOOLER_SETTINGS coolers = {};
        coolers.version = NV_GPU_GETCOOLER_SETTINGS_VER;
        NvAPI_Status status = NvAPI_GPU_GetCoolerSettings(handle, NVAPI_COOLER_TARGET_ALL, &coolers);
        if (status == NVAPI_INCOMPATIBLE_STRUCT_VERSION)
        {
            coolers.version = NV_GPU_GETCOOLER_SETTINGS_VER3;
            status = NvAPI_GPU_GetCoolerSettings(handle, NVAPI_COOLER_TARGET_ALL, &coolers);
        }
        if (status == NVAPI_INCOMPATIBLE_STRUCT_VERSION)
        {
            coolers.version = NV_GPU_GETCOOLER_SETTINGS_VER2;
            status = NvAPI_GPU_GetCoolerSettings(handle, NVAPI_COOLER_TARGET_ALL, &coolers);
        }
        if (status == NVAPI_INCOMPATIBLE_STRUCT_VERSION)
        {
            coolers.version = NV_GPU_GETCOOLER_SETTINGS_VER1;
            status = NvAPI_GPU_GetCoolerSettings(handle, NVAPI_COOLER_TARGET_ALL, &coolers);
        }

        if (status != NVAPI_OK)
        {
            PrintNvapiError("  NvAPI_GPU_GetCoolerSettings failed", status);
            return;
        }

        std::printf("  Coolers: %u\n", coolers.count);
        for (NvU32 i = 0; i < coolers.count; ++i)
        {
            const auto &cooler = coolers.cooler[i];
            const bool manualSupported = (cooler.supportedPolicies & NVAPI_COOLER_POLICY_MANUAL) != 0;
            std::printf("    cooler[%u]: level=%u%% min=%u%% max=%u%% policy=%u target=%u active=%u\n",
                i,
                cooler.currentLevel,
                cooler.currentMinLevel,
                cooler.currentMaxLevel,
                cooler.currentPolicy,
                cooler.target,
                cooler.active);
            std::printf("      control=%s supportedPolicies=0x%08X manual=%s\n",
                CoolerControlName(cooler.controlType),
                cooler.supportedPolicies,
                manualSupported ? "yes" : "no");
            if (cooler.tachometer.bSupported)
            {
                std::printf("      tach: %u RPM (min=%u max=%u)\n",
                    cooler.tachometer.speedRPM,
                    cooler.tachometer.minSpeedRPM,
                    cooler.tachometer.maxSpeedRPM);
            }
        }

        NvU32 tachReading = 0;
        status = NvAPI_GPU_GetTachReading(handle, &tachReading);
        if (status == NVAPI_OK)
        {
            std::printf("  Tachometer: %u RPM\n", tachReading);
        }
    }

    void PrintHexBytes(const NvU8 *data, NvU32 size)
    {
        for (NvU32 i = 0; i < size; ++i)
        {
            if (i % 16 == 0)
            {
                std::printf("    ");
            }
            std::printf("%02X ", data[i]);
            if (i % 16 == 15 || i + 1 == size)
            {
                std::printf("\n");
            }
        }
    }

    double TimingRefreshHz(const NV_TIMING &timing)
    {
        if (timing.HTotal == 0 || timing.VTotal == 0)
        {
            return 0.0;
        }
        double pixelClockHz = static_cast<double>(timing.pclk) * 10000.0;
        return pixelClockHz / (static_cast<double>(timing.HTotal) * static_cast<double>(timing.VTotal));
}
}
