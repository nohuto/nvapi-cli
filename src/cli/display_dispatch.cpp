/*
 * Copyright (c) 2026 Noverse (Nohuto). All rights reserved.
 * Proprietary and confidential. Unauthorized copying or redistribution is strictly prohibited.
 */

#include "cli/commands.h"

namespace nvcli {
namespace {
int CmdDisplayListAdapter(int argc, char **argv) {
  (void)argc;
  (void)argv;
  return CmdDisplayList();
}

int CmdDisplayGdiPrimaryAdapter(int argc, char **argv) {
  (void)argc;
  (void)argv;
  return CmdDisplayGdiPrimary();
}

void PrintDisplayUsage() { PrintUsageGroup("display"); }
} // namespace

int CmdDisplay(int argc, char **argv) {
  if (argc < 1) {
    PrintUsageGroup("display");
    return 1;
  }

  static const SubcommandEntry kSubcommands[] = {
      {"list", CmdDisplayListAdapter},
      {"ids", CmdDisplayIds},
      {"edid", CmdDisplayEdid},
      {"timing", CmdDisplayTiming},
      {"get", CmdDisplayGet},
      {"set", CmdDisplaySet},
      {"custom", CmdDisplayCustom},
      {"monitor-caps", CmdDisplayMonitorCaps},
      {"monitor-color-caps", CmdDisplayMonitorColorCaps},
      {"scaling", CmdDisplayScaling},
      {"scaling-override", CmdDisplayScalingOverride},
      {"viewport", CmdDisplayViewport},
      {"feature", CmdDisplayFeature},
      {"wide-color", CmdDisplayWideColor},
      {"bpc", CmdDisplayBpc},
      {"blanking", CmdDisplayBlanking},
      {"hdr", CmdDisplayHdr},
      {"id-by-name", CmdDisplayIdByName},
      {"gdi-primary", CmdDisplayGdiPrimaryAdapter},
      {"handle-from-id", CmdDisplayHandleFromId},
      {"id-from-handle", CmdDisplayIdFromHandle},
  };

  return DispatchSubcommand("display", argc, argv, kSubcommands, sizeof(kSubcommands) / sizeof(kSubcommands[0]),
                            PrintDisplayUsage);
}
} // namespace nvcli
