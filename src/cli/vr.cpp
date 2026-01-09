/*
 * Copyright (c) 2026 Noverse (Nohuto). All rights reserved.
 * Proprietary and confidential. Unauthorized copying or redistribution is strictly prohibited.
 */

#include "commands.h"

namespace nvcli
{
    namespace
    {
        bool ParseU64(const char *text, NvU64 *out)
        {
            if (!text || !out)
            {
                return false;
            }
            char *end = NULL;
            unsigned long long value = std::strtoull(text, &end, 0);
            if (end == text || *end != '\0')
            {
                return false;
            }
            *out = static_cast<NvU64>(value);
            return true;
        }

        bool ParseDirectModeFlag(const char *text, NV_ENUM_DIRECTMODE_DISPLAY_FLAG *out)
        {
            if (!text || !out)
            {
                return false;
            }
            std::string value = ToLowerAscii(text);
            if (value == "capable")
            {
                *out = NV_ENUM_DIRECTMODE_DISPLAY_CAPABLE;
                return true;
            }
            if (value == "enabled")
            {
                *out = NV_ENUM_DIRECTMODE_DISPLAY_ENABLED;
                return true;
            }
            return false;
        }

        void PrintVrUsage()
        {
            std::printf("VR toolchain commands:\n");
            std::printf("  %s vr direct-mode enable --vendor-id ID\n", kToolName);
            std::printf("  %s vr direct-mode disable --vendor-id ID\n", kToolName);
            std::printf("  %s vr direct-mode list --vendor-id ID [--flag capable|enabled]\n", kToolName);
            std::printf("  %s vr direct-mode handle-from-id --display-id HEX\n", kToolName);
            std::printf("  %s vr direct-mode id-from-handle --display-id HEX --context HEX\n", kToolName);
        }

        int CmdVrDirectMode(int argc, char **argv)
        {
            if (argc < 1)
            {
                std::printf("Missing direct-mode command.\n");
                return 1;
            }

            if (std::strcmp(argv[0], "enable") == 0 || std::strcmp(argv[0], "disable") == 0)
            {
                NvU32 vendorId = 0;
                bool hasVendor = false;
                for (int i = 1; i < argc; ++i)
                {
                    if (std::strcmp(argv[i], "--vendor-id") == 0)
                    {
                        if (i + 1 >= argc)
                        {
                            std::printf("Missing value for --vendor-id\n");
                            return 1;
                        }
                        if (!ParseUint(argv[i + 1], &vendorId))
                        {
                            std::printf("Invalid vendor id: %s\n", argv[i + 1]);
                            return 1;
                        }
                        hasVendor = true;
                        ++i;
                        continue;
                    }
                    std::printf("Unknown option: %s\n", argv[i]);
                    return 1;
                }

                if (!hasVendor)
                {
                    std::printf("Missing required --vendor-id\n");
                    return 1;
                }

                NvAPI_Status status = NVAPI_OK;
                if (std::strcmp(argv[0], "enable") == 0)
                {
                    status = NvAPI_DISP_EnableDirectMode(vendorId, 0);
                }
                else
                {
                    status = NvAPI_DISP_DisableDirectMode(vendorId, 0);
                }

                if (status != NVAPI_OK)
                {
                    PrintNvapiError("Direct mode call failed", status);
                    return 1;
                }

                std::printf("Direct mode %s request sent.\n", argv[0]);
                return 0;
            }

            if (std::strcmp(argv[0], "list") == 0)
            {
                NvU32 vendorId = 0;
                bool hasVendor = false;
                NV_ENUM_DIRECTMODE_DISPLAY_FLAG flag = NV_ENUM_DIRECTMODE_DISPLAY_CAPABLE;
                for (int i = 1; i < argc; ++i)
                {
                    if (std::strcmp(argv[i], "--vendor-id") == 0)
                    {
                        if (i + 1 >= argc)
                        {
                            std::printf("Missing value for --vendor-id\n");
                            return 1;
                        }
                        if (!ParseUint(argv[i + 1], &vendorId))
                        {
                            std::printf("Invalid vendor id: %s\n", argv[i + 1]);
                            return 1;
                        }
                        hasVendor = true;
                        ++i;
                        continue;
                    }
                    if (std::strcmp(argv[i], "--flag") == 0)
                    {
                        if (i + 1 >= argc)
                        {
                            std::printf("Missing value for --flag\n");
                            return 1;
                        }
                        if (!ParseDirectModeFlag(argv[i + 1], &flag))
                        {
                            std::printf("Invalid flag: %s\n", argv[i + 1]);
                            return 1;
                        }
                        ++i;
                        continue;
                    }
                    std::printf("Unknown option: %s\n", argv[i]);
                    return 1;
                }

                if (!hasVendor)
                {
                    std::printf("Missing required --vendor-id\n");
                    return 1;
                }

                NvU32 count = 0;
                NvAPI_Status status = NvAPI_DISP_EnumerateDirectModeDisplays(vendorId, &count, nullptr, flag);
                if (status != NVAPI_OK && status != NVAPI_INSUFFICIENT_BUFFER)
                {
                    PrintNvapiError("NvAPI_DISP_EnumerateDirectModeDisplays failed", status);
                    return 1;
                }

                if (count == 0)
                {
                    std::printf("No direct mode displays found.\n");
                    return 0;
                }

                std::vector<NV_DIRECT_MODE_DISPLAY_HANDLE> handles(count);
                NvU32 actual = count;
                status = NvAPI_DISP_EnumerateDirectModeDisplays(vendorId, &actual, handles.data(), flag);
                if (status != NVAPI_OK)
                {
                    PrintNvapiError("NvAPI_DISP_EnumerateDirectModeDisplays failed", status);
                    return 1;
                }

                std::printf("Direct mode displays: %u\n", actual);
                for (NvU32 i = 0; i < actual; ++i)
                {
                    std::printf("  [%u] displayId=0x%08X context=0x%p\n", i, handles[i].displayId, handles[i].pDisplayContext);
                }
                return 0;
            }

            if (std::strcmp(argv[0], "handle-from-id") == 0)
            {
                NvU32 displayId = 0;
                bool hasDisplayId = false;
                for (int i = 1; i < argc; ++i)
                {
                    if (std::strcmp(argv[i], "--display-id") == 0)
                    {
                        if (i + 1 >= argc)
                        {
                            std::printf("Missing value for --display-id\n");
                            return 1;
                        }
                        if (!ParseUint(argv[i + 1], &displayId))
                        {
                            std::printf("Invalid display id: %s\n", argv[i + 1]);
                            return 1;
                        }
                        hasDisplayId = true;
                        ++i;
                        continue;
                    }
                    std::printf("Unknown option: %s\n", argv[i]);
                    return 1;
                }

                if (!hasDisplayId)
                {
                    std::printf("Missing required --display-id\n");
                    return 1;
                }

                NV_DIRECT_MODE_DISPLAY_HANDLE handle = {};
                NvAPI_Status status = NvAPI_DISP_GetDirectModeDisplayHandleFromDisplayId(displayId, &handle);
                if (status != NVAPI_OK)
                {
                    PrintNvapiError("NvAPI_DISP_GetDirectModeDisplayHandleFromDisplayId failed", status);
                    return 1;
                }

                std::printf("Display handle: displayId=0x%08X context=0x%p\n", handle.displayId, handle.pDisplayContext);
                return 0;
            }

            if (std::strcmp(argv[0], "id-from-handle") == 0)
            {
                NvU32 displayId = 0;
                NvU64 contextValue = 0;
                bool hasDisplayId = false;
                bool hasContext = false;

                for (int i = 1; i < argc; ++i)
                {
                    if (std::strcmp(argv[i], "--display-id") == 0)
                    {
                        if (i + 1 >= argc)
                        {
                            std::printf("Missing value for --display-id\n");
                            return 1;
                        }
                        if (!ParseUint(argv[i + 1], &displayId))
                        {
                            std::printf("Invalid display id: %s\n", argv[i + 1]);
                            return 1;
                        }
                        hasDisplayId = true;
                        ++i;
                        continue;
                    }
                    if (std::strcmp(argv[i], "--context") == 0)
                    {
                        if (i + 1 >= argc)
                        {
                            std::printf("Missing value for --context\n");
                            return 1;
                        }
                        if (!ParseU64(argv[i + 1], &contextValue))
                        {
                            std::printf("Invalid context value: %s\n", argv[i + 1]);
                            return 1;
                        }
                        hasContext = true;
                        ++i;
                        continue;
                    }
                    std::printf("Unknown option: %s\n", argv[i]);
                    return 1;
                }

                if (!hasDisplayId || !hasContext)
                {
                    std::printf("Missing required --display-id or --context\n");
                    return 1;
                }

                NV_DIRECT_MODE_DISPLAY_HANDLE handle = {};
                handle.displayId = displayId;
                handle.pDisplayContext = reinterpret_cast<void *>(static_cast<uintptr_t>(contextValue));

                NvU32 outDisplayId = 0;
                NvAPI_Status status = NvAPI_DISP_GetDisplayIdFromDirectModeDisplayHandle(&handle, &outDisplayId);
                if (status != NVAPI_OK)
                {
                    PrintNvapiError("NvAPI_DISP_GetDisplayIdFromDirectModeDisplayHandle failed", status);
                    return 1;
                }

                std::printf("Display id: 0x%08X\n", outDisplayId);
                return 0;
            }

            std::printf("Unknown direct-mode command: %s\n", argv[0]);
            return 1;
        }
    }

    int CmdVr(int argc, char **argv)
    {
        static const SubcommandEntry kSubcommands[] = {
            {"direct-mode", CmdVrDirectMode},
        };

        return DispatchSubcommand("vr", argc, argv, kSubcommands, sizeof(kSubcommands) / sizeof(kSubcommands[0]), PrintVrUsage);
    }
}
