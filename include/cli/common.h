/*
 * Copyright (c) 2026 Noverse (Nohuto). All rights reserved.
 * Proprietary and confidential. Unauthorized copying or redistribution is strictly prohibited.
 */

#pragma once

#include <windows.h>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cctype>
#include <algorithm>
#include <cstddef>
#include <string>
#include <vector>

#include <nvapi.h>

namespace nvcli {
extern const char *kToolName;
constexpr NvU32 kMaxDisplayPaths = 16;

std::string NvapiStatusString(NvAPI_Status status);
void PrintNvapiError(const char *prefix, NvAPI_Status status);
bool ParseUint(const char *text, NvU32 *out);
bool ParseSrcDevicePair(const char *text, NvU32 *srcId, NvU32 *device);
bool GetDisplayHandleByIndex(NvU32 index, NvDisplayHandle *outHandle);
double KBToMiB(NvU32 kb);
double BytesToMiB(NvU64 bytes);
const char *BusTypeName(NV_GPU_BUS_TYPE type);
const char *RamTypeName(NV_GPU_RAM_TYPE type);
const char *UtilizationDomainName(NvU32 domain);
const char *ClockDomainName(NvU32 domain);
const char *PstateClockTypeName(NV_GPU_PERF_PSTATE20_CLOCK_TYPE_ID type);
const char *ConnectorTypeName(NV_MONITOR_CONN_TYPE type);
const char *PstateName(NV_GPU_PERF_PSTATE_ID pstate);
const char *CoolerControlName(NV_COOLER_CONTROL control);
const char *VoltageDomainName(NV_GPU_PERF_VOLTAGE_INFO_DOMAIN_ID domain);
const char *PerfVoltageDomainName(NV_GPU_PERF_VOLTAGE_DOMAIN_ID domain);
const char *VoltageEntryTypeName(NV_GPU_PSTATE20_VOLTAGE_ENTRY_TYPE type);
const char *ClockDomainIdName(NV_GPU_CLOCK_DOMAIN_ID domain);
const char *ClockPstateUsageName(NV_GPU_CLOCK_INFO_DOMAIN_PSTATE_USAGE usage);
const char *PerfLimitInputTypeName(NV_GPU_PERF_LIMIT_STATUS_INPUT_TYPE type);
const char *PerfLimitPstatePointName(NV_GPU_PERF_LIMIT_INPUT_DATA_PSTATE_POINT point);
const char *VpstateTypeName(NV_GPU_PERF_VPSTATE_TYPE type);
const char *VfeVarTypeName(NV_GPU_PERF_VFE_VAR_TYPE type);
const char *VfeVarOverrideTypeName(NvU8 type);
const char *VfeEquTypeName(NV_GPU_PERF_VFE_EQU_TYPE type);
const char *VfeEquOutputTypeName(NvU8 type);
const char *VfeEquCompareFuncName(NvU8 func);
bool ParseGpuIndexArgs(int argc, char **argv, NvU32 *indexOut, bool *hasIndex);
bool ParseDisplayIdArg(int argc, char **argv, NvU32 *displayId);
bool ParseDisplayIdsArgs(int argc, char **argv, NvU32 *gpuIndex, bool *hasIndex, bool *all, NvU32 *flags);
bool ParseEdidFlagArg(int argc, char **argv, NV_EDID_FLAG *flag);
bool ParseCoolerPolicy(const char *value, NV_COOLER_POLICY *policy);
bool ParsePstateId(const char *value, NV_GPU_PERF_PSTATE_ID *out);
bool ParsePublicClockId(const char *value, NV_GPU_PUBLIC_CLOCK_ID *out);
bool ParseVoltageDomainId(const char *value, NV_GPU_PERF_VOLTAGE_INFO_DOMAIN_ID *out);
bool ParsePerfVoltageDomainId(const char *value, NV_GPU_PERF_VOLTAGE_DOMAIN_ID *out);
bool ParsePerfLimitInputType(const char *value, NV_GPU_PERF_LIMIT_STATUS_INPUT_TYPE *out);
bool ParsePerfLimitPstatePoint(const char *value, NV_GPU_PERF_LIMIT_INPUT_DATA_PSTATE_POINT *out);
std::string ToLowerAscii(const char *value);
bool ParseBoolValue(const char *value, bool *out);
bool Utf8ToNvUnicode(const char *input, NvAPI_UnicodeString out);
std::string NvUnicodeToUtf8(const NvAPI_UnicodeString value);
std::string GuidToString(const NvGUID &guid);
const char *TargetViewModeName(NV_TARGET_VIEW_MODE mode);
const char *MosaicTopoTypeName(NV_MOSAIC_TOPO_TYPE type);
const char *MosaicTopoName(NV_MOSAIC_TOPO topo);
const char *GsyncDisplaySyncStateName(NVAPI_GSYNC_DISPLAY_SYNC_STATE state);
const char *DpNodeTypeName(NV_DP_NODE_TYPE type);
bool CollectGpus(bool hasIndex, NvU32 index, std::vector<NvPhysicalGpuHandle> &handles, std::vector<NvU32> &indices);
bool MaskE32Has(const NV_GPU_BOARDOBJGRP_MASK_E32 &mask, NvU32 index);
bool MaskE255Has(const NV_GPU_BOARDOBJGRP_MASK_E255 &mask, NvU32 index);
void PrintUsage();
void PrintUsageGroup(const char *group);
const char *DrsSettingTypeName(NVDRS_SETTING_TYPE type);
void InitDrsSetting(NVDRS_SETTING *setting);
void PrintDrsSetting(const NVDRS_SETTING &setting);
bool GetDrsProfileByName(NvDRSSessionHandle session, const char *name, NvDRSProfileHandle *outProfile);
bool GetDrsSettingIdByName(const char *name, NvU32 *outId);

struct SubcommandEntry {
  const char *name;
  int (*handler)(int argc, char **argv);
};

const SubcommandEntry *FindSubcommand(const SubcommandEntry *entries, size_t count, const char *name);
int DispatchSubcommand(const char *group, int argc, char **argv, const SubcommandEntry *entries, size_t count,
                       void (*printUsage)());

class NvApiSession {
public:
  NvApiSession() : m_status(NvAPI_InitializeEx(NV_DISPLAY_DRIVER)) {}

  ~NvApiSession() {
    if (m_status == NVAPI_OK) { NvAPI_UnloadEx(NV_DISPLAY_DRIVER); }
  }

  bool ok() const { return m_status == NVAPI_OK; }

  NvAPI_Status status() const { return m_status; }

private:
  NvAPI_Status m_status;
};

class DrsSession {
public:
  DrsSession() : m_handle(NULL), m_status(NvAPI_DRS_CreateSession(&m_handle)) {
    if (m_status == NVAPI_OK) { m_status = NvAPI_DRS_LoadSettings(m_handle); }
  }

  ~DrsSession() {
    if (m_status == NVAPI_OK && m_handle) { NvAPI_DRS_DestroySession(m_handle); }
  }

  bool ok() const { return m_status == NVAPI_OK; }

  NvAPI_Status status() const { return m_status; }

  NvDRSSessionHandle handle() const { return m_handle; }

private:
  NvDRSSessionHandle m_handle;
  NvAPI_Status m_status;
};
} // namespace nvcli
