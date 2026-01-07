/*
 * Copyright (c) 2026 Noverse (Nohuto). All rights reserved.
 * Proprietary and confidential. Unauthorized copying or redistribution is strictly prohibited.
 */

#include "commands.h"

namespace nvcli
{
    namespace
    {
        int CmdDrsProfilesAdapter(int argc, char **argv)
        {
            (void)argc;
            (void)argv;
            return CmdDrsProfiles();
        }

        void PrintDrsUsage()
        {
            PrintUsageGroup("drs");
        }

        int CmdDrsSettingDispatch(int argc, char **argv)
        {
            static const SubcommandEntry kSubcommands[] = {
                {"get", CmdDrsSettingGet},
                {"set", CmdDrsSettingSet},
            };

            return DispatchSubcommand("drs setting", argc, argv, kSubcommands, sizeof(kSubcommands) / sizeof(kSubcommands[0]), PrintDrsUsage);
        }

        int CmdDrsProfileDispatch(int argc, char **argv)
        {
            static const SubcommandEntry kSubcommands[] = {
                {"create", CmdDrsProfileCreate},
                {"delete", CmdDrsProfileDelete},
            };

            return DispatchSubcommand("drs profile", argc, argv, kSubcommands, sizeof(kSubcommands) / sizeof(kSubcommands[0]), PrintDrsUsage);
        }
    }
    int CmdDrsProfiles()
    {
        DrsSession session;
        if (!session.ok())
        {
            PrintNvapiError("NvAPI_DRS_CreateSession/LoadSettings failed", session.status());
            return 1;
        }

        NvU32 count = 0;
        NvAPI_Status status = NvAPI_DRS_GetNumProfiles(session.handle(), &count);
        if (status != NVAPI_OK)
        {
            PrintNvapiError("NvAPI_DRS_GetNumProfiles failed", status);
            return 1;
        }

        std::printf("DRS profiles: %u\n", count);
        for (NvU32 i = 0; i < count; ++i)
        {
            NvDRSProfileHandle profile = NULL;
            status = NvAPI_DRS_EnumProfiles(session.handle(), i, &profile);
            if (status != NVAPI_OK)
            {
                PrintNvapiError("NvAPI_DRS_EnumProfiles failed", status);
                return 1;
            }

            NVDRS_PROFILE info = {};
            info.version = NVDRS_PROFILE_VER;
            status = NvAPI_DRS_GetProfileInfo(session.handle(), profile, &info);
            if (status != NVAPI_OK)
            {
                PrintNvapiError("NvAPI_DRS_GetProfileInfo failed", status);
                return 1;
            }

            std::string name = NvUnicodeToUtf8(info.profileName);
            std::printf("  [%u] %s predefined=%u apps=%u settings=%u\n",
                i,
                name.empty() ? "<unnamed>" : name.c_str(),
                info.isPredefined,
                info.numOfApps,
                info.numOfSettings);
        }

        return 0;
    }

    int CmdDrsApps(int argc, char **argv)
    {
        const char *profileName = NULL;
        for (int i = 0; i < argc; ++i)
        {
            if (std::strcmp(argv[i], "--profile") == 0)
            {
                if (i + 1 >= argc)
                {
                    std::printf("Missing value for --profile\n");
                    return 1;
                }
                profileName = argv[i + 1];
                ++i;
                continue;
            }
            std::printf("Unknown option: %s\n", argv[i]);
            return 1;
        }

        if (!profileName)
        {
            std::printf("Missing required --profile\n");
            return 1;
        }

        DrsSession session;
        if (!session.ok())
        {
            PrintNvapiError("NvAPI_DRS_CreateSession/LoadSettings failed", session.status());
            return 1;
        }

        NvDRSProfileHandle profile = NULL;
        if (!GetDrsProfileByName(session.handle(), profileName, &profile))
        {
            return 1;
        }

        std::printf("Applications for profile: %s\n", profileName);
        NvU32 start = 0;
        const NvU32 batchSize = 32;
        while (true)
        {
            NvU32 count = batchSize;
            std::vector<NVDRS_APPLICATION> apps(count);
            for (NvU32 i = 0; i < count; ++i)
            {
                std::memset(&apps[i], 0, sizeof(NVDRS_APPLICATION));
                apps[i].version = NVDRS_APPLICATION_VER;
            }

            NvAPI_Status status = NvAPI_DRS_EnumApplications(session.handle(), profile, start, &count, apps.data());
            if (status == NVAPI_END_ENUMERATION || count == 0)
            {
                break;
            }
            if (status != NVAPI_OK)
            {
                PrintNvapiError("NvAPI_DRS_EnumApplications failed", status);
                return 1;
            }

            for (NvU32 i = 0; i < count; ++i)
            {
                std::string appName = NvUnicodeToUtf8(apps[i].appName);
                std::string friendly = NvUnicodeToUtf8(apps[i].userFriendlyName);
                std::printf("  %s (%s)\n",
                    appName.empty() ? "<unnamed>" : appName.c_str(),
                    friendly.empty() ? "<no description>" : friendly.c_str());
            }

            start += count;
        }

        return 0;
    }

    int CmdDrsSettings(int argc, char **argv)
    {
        const char *profileName = NULL;
        NvU32 start = 0;
        NvU32 limit = 50;

        for (int i = 0; i < argc; ++i)
        {
            if (std::strcmp(argv[i], "--profile") == 0)
            {
                if (i + 1 >= argc)
                {
                    std::printf("Missing value for --profile\n");
                    return 1;
                }
                profileName = argv[i + 1];
                ++i;
                continue;
            }
            if (std::strcmp(argv[i], "--start") == 0)
            {
                if (i + 1 >= argc)
                {
                    std::printf("Missing value for --start\n");
                    return 1;
                }
                if (!ParseUint(argv[i + 1], &start))
                {
                    std::printf("Invalid start: %s\n", argv[i + 1]);
                    return 1;
                }
                ++i;
                continue;
            }
            if (std::strcmp(argv[i], "--limit") == 0)
            {
                if (i + 1 >= argc)
                {
                    std::printf("Missing value for --limit\n");
                    return 1;
                }
                if (!ParseUint(argv[i + 1], &limit))
                {
                    std::printf("Invalid limit: %s\n", argv[i + 1]);
                    return 1;
                }
                ++i;
                continue;
            }
            std::printf("Unknown option: %s\n", argv[i]);
            return 1;
        }

        if (!profileName)
        {
            std::printf("Missing required --profile\n");
            return 1;
        }

        DrsSession session;
        if (!session.ok())
        {
            PrintNvapiError("NvAPI_DRS_CreateSession/LoadSettings failed", session.status());
            return 1;
        }

        NvDRSProfileHandle profile = NULL;
        if (!GetDrsProfileByName(session.handle(), profileName, &profile))
        {
            return 1;
        }

        std::printf("Settings for profile: %s\n", profileName);

        NvU32 printed = 0;
        const NvU32 batchSize = 32;
        NvU32 index = start;
        while (printed < limit)
        {
            NvU32 count = batchSize;
            std::vector<NVDRS_SETTING> settings(count);
            for (NvU32 i = 0; i < count; ++i)
            {
                InitDrsSetting(&settings[i]);
            }

            NvAPI_Status status = NvAPI_DRS_EnumSettings(session.handle(), profile, index, &count, settings.data());
            if (status == NVAPI_END_ENUMERATION || count == 0)
            {
                break;
            }
            if (status != NVAPI_OK)
            {
                PrintNvapiError("NvAPI_DRS_EnumSettings failed", status);
                return 1;
            }

            for (NvU32 i = 0; i < count && printed < limit; ++i)
            {
                PrintDrsSetting(settings[i]);
                ++printed;
            }
            index += count;
        }

        if (printed == 0)
        {
            std::printf("  <no settings>\n");
        }
        return 0;
    }

    int CmdDrsSettingGet(int argc, char **argv)
    {
        const char *profileName = NULL;
        const char *settingName = NULL;
        NvU32 settingId = 0;
        bool hasId = false;

        for (int i = 0; i < argc; ++i)
        {
            if (std::strcmp(argv[i], "--profile") == 0)
            {
                if (i + 1 >= argc)
                {
                    std::printf("Missing value for --profile\n");
                    return 1;
                }
                profileName = argv[i + 1];
                ++i;
                continue;
            }
            if (std::strcmp(argv[i], "--id") == 0)
            {
                if (i + 1 >= argc)
                {
                    std::printf("Missing value for --id\n");
                    return 1;
                }
                if (!ParseUint(argv[i + 1], &settingId))
                {
                    std::printf("Invalid id: %s\n", argv[i + 1]);
                    return 1;
                }
                hasId = true;
                ++i;
                continue;
            }
            if (std::strcmp(argv[i], "--name") == 0)
            {
                if (i + 1 >= argc)
                {
                    std::printf("Missing value for --name\n");
                    return 1;
                }
                settingName = argv[i + 1];
                ++i;
                continue;
            }
            std::printf("Unknown option: %s\n", argv[i]);
            return 1;
        }

        if (!profileName)
        {
            std::printf("Missing required --profile\n");
            return 1;
        }
        if (!hasId && !settingName)
        {
            std::printf("Missing required --id or --name\n");
            return 1;
        }

        if (!hasId && settingName)
        {
            if (!GetDrsSettingIdByName(settingName, &settingId))
            {
                return 1;
            }
        }

        DrsSession session;
        if (!session.ok())
        {
            PrintNvapiError("NvAPI_DRS_CreateSession/LoadSettings failed", session.status());
            return 1;
        }

        NvDRSProfileHandle profile = NULL;
        if (!GetDrsProfileByName(session.handle(), profileName, &profile))
        {
            return 1;
        }

        NVDRS_SETTING setting;
        InitDrsSetting(&setting);
        NvAPI_Status status = NvAPI_DRS_GetSetting(session.handle(), profile, settingId, &setting);
        if (status != NVAPI_OK)
        {
            PrintNvapiError("NvAPI_DRS_GetSetting failed", status);
            return 1;
        }

        PrintDrsSetting(setting);
        return 0;
    }

    int CmdDrsSettingSet(int argc, char **argv)
    {
        const char *profileName = NULL;
        const char *settingName = NULL;
        NvU32 settingId = 0;
        NvU32 value = 0;
        bool hasId = false;
        bool hasValue = false;

        for (int i = 0; i < argc; ++i)
        {
            if (std::strcmp(argv[i], "--profile") == 0)
            {
                if (i + 1 >= argc)
                {
                    std::printf("Missing value for --profile\n");
                    return 1;
                }
                profileName = argv[i + 1];
                ++i;
                continue;
            }
            if (std::strcmp(argv[i], "--id") == 0)
            {
                if (i + 1 >= argc)
                {
                    std::printf("Missing value for --id\n");
                    return 1;
                }
                if (!ParseUint(argv[i + 1], &settingId))
                {
                    std::printf("Invalid id: %s\n", argv[i + 1]);
                    return 1;
                }
                hasId = true;
                ++i;
                continue;
            }
            if (std::strcmp(argv[i], "--name") == 0)
            {
                if (i + 1 >= argc)
                {
                    std::printf("Missing value for --name\n");
                    return 1;
                }
                settingName = argv[i + 1];
                ++i;
                continue;
            }
            if (std::strcmp(argv[i], "--dword") == 0)
            {
                if (i + 1 >= argc)
                {
                    std::printf("Missing value for --dword\n");
                    return 1;
                }
                if (!ParseUint(argv[i + 1], &value))
                {
                    std::printf("Invalid dword value: %s\n", argv[i + 1]);
                    return 1;
                }
                hasValue = true;
                ++i;
                continue;
            }
            std::printf("Unknown option: %s\n", argv[i]);
            return 1;
        }

        if (!profileName)
        {
            std::printf("Missing required --profile\n");
            return 1;
        }
        if (!hasId && !settingName)
        {
            std::printf("Missing required --id or --name\n");
            return 1;
        }
        if (!hasValue)
        {
            std::printf("Missing required --dword\n");
            return 1;
        }

        if (!hasId && settingName)
        {
            if (!GetDrsSettingIdByName(settingName, &settingId))
            {
                return 1;
            }
        }

        std::printf("DRS set: profile=%s settingId=0x%08X value=0x%08X (%u)\n",
            profileName,
            settingId,
            value,
            value);

        DrsSession session;
        if (!session.ok())
        {
            PrintNvapiError("NvAPI_DRS_CreateSession/LoadSettings failed", session.status());
            return 1;
        }

        NvDRSProfileHandle profile = NULL;
        if (!GetDrsProfileByName(session.handle(), profileName, &profile))
        {
            return 1;
        }

        NVDRS_SETTING setting;
        InitDrsSetting(&setting);
        setting.settingId = settingId;
        setting.settingType = NVDRS_DWORD_TYPE;
        setting.u32CurrentValue = value;
        setting.settingLocation = NVDRS_CURRENT_PROFILE_LOCATION;
        setting.isCurrentPredefined = 0;
        setting.isPredefinedValid = 0;

        NvAPI_Status status = NvAPI_DRS_SetSetting(session.handle(), profile, &setting);
        if (status != NVAPI_OK)
        {
            PrintNvapiError("NvAPI_DRS_SetSetting failed", status);
            return 1;
        }

        status = NvAPI_DRS_SaveSettings(session.handle());
        if (status != NVAPI_OK)
        {
            PrintNvapiError("NvAPI_DRS_SaveSettings failed", status);
            return 1;
        }

        std::printf("DRS setting updated.\n");
        return 0;
    }

    int CmdDrsProfileCreate(int argc, char **argv)
    {
        const char *profileName = NULL;

        for (int i = 0; i < argc; ++i)
        {
            if (std::strcmp(argv[i], "--name") == 0)
            {
                if (i + 1 >= argc)
                {
                    std::printf("Missing value for --name\n");
                    return 1;
                }
                profileName = argv[i + 1];
                ++i;
                continue;
            }
            std::printf("Unknown option: %s\n", argv[i]);
            return 1;
        }

        if (!profileName)
        {
            std::printf("Missing required --name\n");
            return 1;
        }

        std::printf("DRS create profile: %s\n", profileName);

        DrsSession session;
        if (!session.ok())
        {
            PrintNvapiError("NvAPI_DRS_CreateSession/LoadSettings failed", session.status());
            return 1;
        }

        NVDRS_PROFILE profile = {};
        profile.version = NVDRS_PROFILE_VER;
        if (!Utf8ToNvUnicode(profileName, profile.profileName))
        {
            std::printf("Invalid profile name encoding.\n");
            return 1;
        }

        NvDRSProfileHandle handle = NULL;
        NvAPI_Status status = NvAPI_DRS_CreateProfile(session.handle(), &profile, &handle);
        if (status != NVAPI_OK)
        {
            PrintNvapiError("NvAPI_DRS_CreateProfile failed", status);
            return 1;
        }

        status = NvAPI_DRS_SaveSettings(session.handle());
        if (status != NVAPI_OK)
        {
            PrintNvapiError("NvAPI_DRS_SaveSettings failed", status);
            return 1;
        }

        std::printf("DRS profile created.\n");
        return 0;
    }

    int CmdDrsProfileDelete(int argc, char **argv)
    {
        const char *profileName = NULL;

        for (int i = 0; i < argc; ++i)
        {
            if (std::strcmp(argv[i], "--name") == 0)
            {
                if (i + 1 >= argc)
                {
                    std::printf("Missing value for --name\n");
                    return 1;
                }
                profileName = argv[i + 1];
                ++i;
                continue;
            }
            std::printf("Unknown option: %s\n", argv[i]);
            return 1;
        }

        if (!profileName)
        {
            std::printf("Missing required --name\n");
            return 1;
        }

        std::printf("DRS delete profile: %s\n", profileName);

        DrsSession session;
        if (!session.ok())
        {
            PrintNvapiError("NvAPI_DRS_CreateSession/LoadSettings failed", session.status());
            return 1;
        }

        NvDRSProfileHandle profile = NULL;
        if (!GetDrsProfileByName(session.handle(), profileName, &profile))
        {
            return 1;
        }

        NvAPI_Status status = NvAPI_DRS_DeleteProfile(session.handle(), profile);
        if (status != NVAPI_OK)
        {
            PrintNvapiError("NvAPI_DRS_DeleteProfile failed", status);
            return 1;
        }

        status = NvAPI_DRS_SaveSettings(session.handle());
        if (status != NVAPI_OK)
        {
            PrintNvapiError("NvAPI_DRS_SaveSettings failed", status);
            return 1;
        }

        std::printf("DRS profile deleted.\n");
        return 0;
    }

    int CmdDrs(int argc, char **argv)
    {
        if (argc < 1)
        {
            PrintUsageGroup("drs");
            return 1;
        }

        static const SubcommandEntry kSubcommands[] = {
            {"profiles", CmdDrsProfilesAdapter},
            {"apps", CmdDrsApps},
            {"settings", CmdDrsSettings},
            {"setting", CmdDrsSettingDispatch},
            {"profile", CmdDrsProfileDispatch},
        };

        return DispatchSubcommand("drs", argc, argv, kSubcommands, sizeof(kSubcommands) / sizeof(kSubcommands[0]), PrintDrsUsage);
    }
}

