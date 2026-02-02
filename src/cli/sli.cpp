/*
 * Copyright (c) 2026 Noverse (Nohuto). All rights reserved.
 * Proprietary and confidential. Unauthorized copying or redistribution is strictly prohibited.
 */

#include "cli/commands.h"

namespace nvcli {
namespace {
int CmdSliStatusAdapter(int argc, char **argv) {
  (void)argc;
  (void)argv;
  return CmdSliStatus();
}

void PrintSliUsage() { PrintUsageGroup("sli"); }
} // namespace

int CmdSliStatus() {
  NV_CHIPSET_SLI_BOND_INFO bondInfo = {};
  bondInfo.version = NV_CHIPSET_SLI_BOND_INFO_VER;
  NvAPI_Status status = NvAPI_SYS_GetChipSetSliBondInfo(&bondInfo);
  if (status == NVAPI_OK) {
    std::printf("Chipset SLI bond: id=0x%08X name=%s\n", bondInfo.sliBondId, bondInfo.szSliBondName);
  } else {
    PrintNvapiError("NvAPI_SYS_GetChipSetSliBondInfo failed", status);
  }

  NvPhysicalGpuHandle physical[NVAPI_MAX_PHYSICAL_GPUS] = {};
  NvU32 physicalCount = 0;
  status = NvAPI_EnumPhysicalGPUs(physical, &physicalCount);
  if (status != NVAPI_OK) {
    PrintNvapiError("NvAPI_EnumPhysicalGPUs failed", status);
    return 1;
  }

  NvLogicalGpuHandle logical[NVAPI_MAX_LOGICAL_GPUS] = {};
  NvU32 logicalCount = 0;
  status = NvAPI_EnumLogicalGPUs(logical, &logicalCount);
  if (status != NVAPI_OK) {
    PrintNvapiError("NvAPI_EnumLogicalGPUs failed", status);
    return 1;
  }

  std::printf("Logical GPUs: %u\n", logicalCount);
  for (NvU32 i = 0; i < logicalCount; ++i) {
    NvPhysicalGpuHandle group[NVAPI_MAX_PHYSICAL_GPUS] = {};
    NvU32 groupCount = 0;
    status = NvAPI_GetPhysicalGPUsFromLogicalGPU(logical[i], group, &groupCount);
    if (status != NVAPI_OK) {
      PrintNvapiError("  NvAPI_GetPhysicalGPUsFromLogicalGPU failed", status);
      continue;
    }

    std::printf("  Logical[%u] physicalCount=%u\n", i, groupCount);
    for (NvU32 j = 0; j < groupCount; ++j) {
      NvU32 physIndex = NVAPI_MAX_PHYSICAL_GPUS;
      for (NvU32 k = 0; k < physicalCount; ++k) {
        if (physical[k] == group[j]) {
          physIndex = k;
          break;
        }
      }
      NvAPI_ShortString name = {0};
      NvAPI_GPU_GetFullName(group[j], name);
      if (physIndex < physicalCount) {
        std::printf("    GPU[%u] %s\n", physIndex, name);
      } else {
        std::printf("    GPU[?] %s\n", name);
      }
    }
  }

  for (NvU32 i = 0; i < physicalCount; ++i) {
    NvAPI_ShortString name = {0};
    NvAPI_GPU_GetFullName(physical[i], name);
    NvU32 displayIdCount = 0;
    // Deprecated in release 290: status = NvAPI_GPU_GetConnectedSLIOutputs(physical[i], &mask);
    status = NvAPI_GPU_GetConnectedDisplayIds(physical[i], NULL, &displayIdCount, 0);
    if (status != NVAPI_OK) {
      std::printf("GPU[%u] %s\n", i, name);
      PrintNvapiError("  NvAPI_GPU_GetConnectedDisplayIds failed", status);
      continue;
    }
    if (displayIdCount == 0) {
      std::printf("GPU[%u] %s connected display IDs: 0\n", i, name);
      continue;
    }
    std::vector<NV_GPU_DISPLAYIDS> displayIds(displayIdCount);
    for (NvU32 j = 0; j < displayIdCount; ++j) { displayIds[j].version = NV_GPU_DISPLAYIDS_VER; }
    status = NvAPI_GPU_GetConnectedDisplayIds(physical[i], displayIds.data(), &displayIdCount, 0);
    if (status != NVAPI_OK) {
      std::printf("GPU[%u] %s\n", i, name);
      PrintNvapiError("  NvAPI_GPU_GetConnectedDisplayIds failed", status);
      continue;
    }
    std::printf("GPU[%u] %s connected display IDs: %u\n", i, name, displayIdCount);
    for (NvU32 j = 0; j < displayIdCount; ++j) { std::printf("  displayId=0x%08X\n", displayIds[j].displayId); }
  }

  return 0;
}

int CmdSliViews(int argc, char **argv) {
  NvU32 index = 0;
  bool hasIndex = false;
  if (!ParseGpuIndexArgs(argc, argv, &index, &hasIndex)) { return 1; }

  std::vector<NvPhysicalGpuHandle> handles;
  std::vector<NvU32> indices;
  if (!CollectGpus(hasIndex, index, handles, indices)) { return 1; }

  for (size_t i = 0; i < handles.size(); ++i) {
    PrintGpuHeader(indices[i], handles[i]);
    NV_TARGET_VIEW_MODE views[NVAPI_MAX_VIEW_MODES] = {};
    NvU32 viewCount = NVAPI_MAX_VIEW_MODES;
    NvAPI_Status status = NvAPI_GPU_GetSupportedSLIViews(handles[i], views, &viewCount);
    if (status != NVAPI_OK) {
      PrintNvapiError("  NvAPI_GPU_GetSupportedSLIViews failed", status);
      continue;
    }

    std::printf("  Supported SLI views: %u\n", viewCount);
    for (NvU32 v = 0; v < viewCount; ++v) { std::printf("    %u (%s)\n", views[v], TargetViewModeName(views[v])); }
  }

  return 0;
}

int CmdSli(int argc, char **argv) {
  if (argc < 1) {
    PrintUsageGroup("sli");
    return 1;
  }

  static const SubcommandEntry kSubcommands[] = {
      {"status", CmdSliStatusAdapter},
      {"views", CmdSliViews},
  };

  return DispatchSubcommand("sli", argc, argv, kSubcommands, sizeof(kSubcommands) / sizeof(kSubcommands[0]),
                            PrintSliUsage);
}

const char *GsyncConnectorName(NVAPI_GSYNC_GPU_TOPOLOGY_CONNECTOR connector) {
  switch (connector) {
  case NVAPI_GSYNC_GPU_TOPOLOGY_CONNECTOR_PRIMARY: return "PRIMARY";
  case NVAPI_GSYNC_GPU_TOPOLOGY_CONNECTOR_SECONDARY: return "SECONDARY";
  case NVAPI_GSYNC_GPU_TOPOLOGY_CONNECTOR_TERTIARY: return "TERTIARY";
  case NVAPI_GSYNC_GPU_TOPOLOGY_CONNECTOR_QUARTERNARY: return "QUARTERNARY";
  default: return "NONE";
  }
}

bool GetGsyncHandleByIndex(NvU32 index, NvGSyncDeviceHandle *outHandle) {
  if (!outHandle) { return false; }
  NvGSyncDeviceHandle handles[NVAPI_MAX_GSYNC_DEVICES] = {};
  NvU32 count = 0;
  NvAPI_Status status = NvAPI_GSync_EnumSyncDevices(handles, &count);
  if (status != NVAPI_OK) {
    PrintNvapiError("NvAPI_GSync_EnumSyncDevices failed", status);
    return false;
  }
  if (index >= count) {
    std::printf("G-Sync device index %u out of range (0-%u)\n", index, count ? count - 1 : 0);
    return false;
  }
  *outHandle = handles[index];
  return true;
}
} // namespace nvcli
