# DRS Group

Covers the `nvapi-cli drs` command group (`src/cli/drs.cpp`). It opens a DRS session (`NvAPI_DRS_CreateSession`) and loads settings (`NvAPI_DRS_LoadSettings`) for each command. `--profile NAME` is matched by name via `NvAPI_DRS_FindProfileByName` and uses UTF-8 conversion. `--id ID` refers to a DRS setting ID (32-bit). `--name NAME` resolves to an ID with `NvAPI_DRS_GetSettingIdFromName`.

```powershell
nvapi-cli drs profiles
nvapi-cli drs apps --profile NAME
nvapi-cli drs settings --profile NAME [--start N] [--limit N]
nvapi-cli drs setting get --profile NAME (--id ID|--name NAME)
nvapi-cli drs setting set --profile NAME (--id ID|--name NAME) --dword VALUE
nvapi-cli drs profile create --name NAME
nvapi-cli drs profile delete --name NAME
```

# Command Reference

## drs profiles
Uses `NvAPI_DRS_GetNumProfiles`, `NvAPI_DRS_EnumProfiles`, and `NvAPI_DRS_GetProfileInfo` to list profiles and their metadata (predefined flag, app count, setting count).

## drs apps
Uses `NvAPI_DRS_EnumApplications` to list applications attached to a profile, enumerated in batches.

```powershell
--profile NAME # profile name to inspect
```

## drs settings
Uses `NvAPI_DRS_EnumSettings` to list settings for a profile. The CLI paginates with `--start` and limits output with `--limit`.

```powershell
--profile NAME # profile name to inspect
--start N # starting index for enumeration
--limit N # maximum settings to print (default 50)
```

## drs setting get
Uses `NvAPI_DRS_GetSetting` to read a single setting by ID. If `--name` is provided, the CLI resolves it to an ID first.

```powershell
--profile NAME # profile name
--id ID # setting ID
--name NAME # setting name (resolved to ID)
# exactly one of --id or --name is required
```

## drs setting set
Uses `NvAPI_DRS_SetSetting` and `NvAPI_DRS_SaveSettings` to update a setting in the current profile. The CLI writes only DWORD settings.

```powershell
--profile NAME # profile name
--id ID # setting ID
--name NAME # setting name (resolved to ID)
--dword VALUE # new DWORD value
# only DWORD settings are supported by this command
```

## drs profile create
Uses `NvAPI_DRS_CreateProfile` and `NvAPI_DRS_SaveSettings` to create a new profile with the supplied name.

```powershell
--name NAME # new profile name
```

## drs profile delete
Uses `NvAPI_DRS_DeleteProfile` and `NvAPI_DRS_SaveSettings` to delete an existing profile by name.

```powershell
--name NAME # profile name to delete
```
