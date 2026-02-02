/*
 * Copyright (c) 2026 Noverse (Nohuto). All rights reserved.
 * Proprietary and confidential. Unauthorized copying or redistribution is strictly prohibited.
 */

#include "cli/commands.h"

#include <windows.h>

using namespace nvcli;

int main(int argc, char **argv) {
  if (argc < 2) {
    PrintUsage();
    return 1;
  }

  if (std::strcmp(argv[1], "help") == 0) {
    if (argc >= 3) {
      PrintUsageGroup(argv[2]);
    } else {
      PrintUsage();
    }
    return 0;
  }

  NvApiSession session;
  if (!session.ok()) {
    PrintNvapiError("NvAPI_InitializeEx failed", session.status());
    return 1;
  }

  if (std::strcmp(argv[1], "info") == 0) { return CmdInfo(); }
  if (std::strcmp(argv[1], "gpu") == 0) { return CmdGpu(argc - 2, argv + 2); }
  if (std::strcmp(argv[1], "display") == 0) { return CmdDisplay(argc - 2, argv + 2); }
  if (std::strcmp(argv[1], "mosaic") == 0) { return CmdMosaic(argc - 2, argv + 2); }
  if (std::strcmp(argv[1], "sli") == 0) { return CmdSli(argc - 2, argv + 2); }
  if (std::strcmp(argv[1], "gsync") == 0) { return CmdGsync(argc - 2, argv + 2); }
  if (std::strcmp(argv[1], "drs") == 0) { return CmdDrs(argc - 2, argv + 2); }
  if (std::strcmp(argv[1], "video") == 0) { return CmdVideo(argc - 2, argv + 2); }
  if (std::strcmp(argv[1], "hdmi") == 0) { return CmdHdmi(argc - 2, argv + 2); }
  if (std::strcmp(argv[1], "dp") == 0) { return CmdDp(argc - 2, argv + 2); }
  if (std::strcmp(argv[1], "pcf") == 0) { return CmdPcf(argc - 2, argv + 2); }
  if (std::strcmp(argv[1], "d3d") == 0) { return CmdD3d(argc - 2, argv + 2); }
  if (std::strcmp(argv[1], "ogl") == 0) { return CmdOgl(argc - 2, argv + 2); }
  if (std::strcmp(argv[1], "vr") == 0) { return CmdVr(argc - 2, argv + 2); }
  if (std::strcmp(argv[1], "stereo") == 0) { return CmdStereo(argc - 2, argv + 2); }
  if (std::strcmp(argv[1], "sys") == 0) { return CmdSys(argc - 2, argv + 2); }

  std::printf("Unknown command: %s\n", argv[1]);
  PrintUsage();
  return 1;
}
