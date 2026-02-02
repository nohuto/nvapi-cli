/*
 * Copyright (c) 2026 Noverse (Nohuto). All rights reserved.
 * Proprietary and confidential. Unauthorized copying or redistribution is strictly prohibited.
 */

#include <d3d11.h>
#include <dxgi.h>
#include <windows.h>

#include "cli/commands.h"

#include <vector>
#include <string>
#include <algorithm>

namespace nvcli {
namespace {
struct D3DContext {
  ID3D11Device *device = nullptr;
  ID3D11DeviceContext *context = nullptr;

  void Cleanup() {
    if (context) {
      context->Release();
      context = nullptr;
    }
    if (device) {
      device->Release();
      device = nullptr;
    }
  }
};

bool CreateD3D11Device(D3DContext &ctx);

struct StereoContext {
  D3DContext d3d;
  StereoHandle handle = nullptr;

  bool Init() {
    if (!CreateD3D11Device(d3d)) { return false; }
    NvAPI_Status status = NvAPI_Stereo_CreateHandleFromIUnknown(d3d.device, &handle);
    if (status != NVAPI_OK) {
      PrintNvapiError("NvAPI_Stereo_CreateHandleFromIUnknown failed", status);
      d3d.Cleanup();
      return false;
    }
    return true;
  }

  void Cleanup() {
    if (handle) {
      NvAPI_Stereo_DestroyHandle(handle);
      handle = nullptr;
    }
    d3d.Cleanup();
  }
};

void PrintHresult(const char *prefix, HRESULT hr) {
  std::printf("%s: 0x%08X\n", prefix, static_cast<unsigned int>(hr));
}

bool CreateD3D11Device(D3DContext &ctx) {
  D3D_FEATURE_LEVEL levels[] = {
      D3D_FEATURE_LEVEL_11_1,
      D3D_FEATURE_LEVEL_11_0,
      D3D_FEATURE_LEVEL_10_1,
      D3D_FEATURE_LEVEL_10_0,
  };
  D3D_FEATURE_LEVEL level = D3D_FEATURE_LEVEL_11_0;
  UINT flags = D3D11_CREATE_DEVICE_BGRA_SUPPORT;

  IDXGIFactory1 *factory = nullptr;
  HRESULT hr = CreateDXGIFactory1(__uuidof(IDXGIFactory1), reinterpret_cast<void **>(&factory));
  if (FAILED(hr)) {
    PrintHresult("CreateDXGIFactory1 failed", hr);
    return false;
  }

  IDXGIAdapter1 *adapter = nullptr;
  for (UINT i = 0; factory->EnumAdapters1(i, &adapter) == S_OK; ++i) {
    DXGI_ADAPTER_DESC1 desc = {};
    if (SUCCEEDED(adapter->GetDesc1(&desc))) {
      if (desc.VendorId == 0x10DE && (desc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE) == 0) { break; }
    }
    adapter->Release();
    adapter = nullptr;
  }
  factory->Release();

  if (!adapter) {
    std::printf("No NVIDIA adapter found for D3D device creation.\n");
    return false;
  }

  hr = D3D11CreateDevice(adapter, D3D_DRIVER_TYPE_UNKNOWN, NULL, flags, levels, ARRAYSIZE(levels), D3D11_SDK_VERSION,
                         &ctx.device, &level, &ctx.context);
  adapter->Release();
  if (FAILED(hr)) {
    PrintHresult("D3D11CreateDevice (NVIDIA adapter) failed", hr);
    return false;
  }
  return true;
}

bool ParseU64(const char *text, NvU64 *out) {
  if (!text || !out) { return false; }
  char *end = NULL;
  unsigned long long value = std::strtoull(text, &end, 0);
  if (end == text || *end != '\0') { return false; }
  *out = static_cast<NvU64>(value);
  return true;
}

bool ParseFloatValue(const char *text, float *out) {
  if (!text || !out) { return false; }
  char *end = NULL;
  float value = std::strtof(text, &end);
  if (end == text || *end != '\0') { return false; }
  *out = value;
  return true;
}

bool ReadBinaryFile(const char *path, void *data, size_t size) {
  if (!path || !data || size == 0) { return false; }
  FILE *file = nullptr;
  if (fopen_s(&file, path, "rb") != 0 || !file) {
    std::printf("Failed to open input file: %s\n", path);
    return false;
  }
  if (fseek(file, 0, SEEK_END) != 0) {
    std::printf("Failed to seek input file: %s\n", path);
    std::fclose(file);
    return false;
  }
  long length = ftell(file);
  if (length < 0) {
    std::printf("Failed to read input file size: %s\n", path);
    std::fclose(file);
    return false;
  }
  if (static_cast<size_t>(length) != size) {
    std::printf("Input file size mismatch: %s (expected %zu bytes, got %ld)\n", path, size, length);
    std::fclose(file);
    return false;
  }
  std::rewind(file);
  size_t read = std::fread(data, 1, size, file);
  std::fclose(file);
  if (read != size) {
    std::printf("Failed to read input file: %s\n", path);
    return false;
  }
  return true;
}

bool WriteBinaryFile(const char *path, const void *data, size_t size) {
  if (!path || !data || size == 0) { return false; }
  FILE *file = nullptr;
  if (fopen_s(&file, path, "wb") != 0 || !file) {
    std::printf("Failed to open output file: %s\n", path);
    return false;
  }
  size_t written = std::fwrite(data, 1, size, file);
  std::fclose(file);
  if (written != size) {
    std::printf("Failed to write output file: %s\n", path);
    return false;
  }
  return true;
}

bool ParseGuidString(const char *text, NvGUID *out) {
  if (!text || !out) { return false; }
  unsigned int d1 = 0;
  unsigned int d2 = 0;
  unsigned int d3 = 0;
  unsigned int b0 = 0;
  unsigned int b1 = 0;
  unsigned int b2 = 0;
  unsigned int b3 = 0;
  unsigned int b4 = 0;
  unsigned int b5 = 0;
  unsigned int b6 = 0;
  unsigned int b7 = 0;
  int count =
      ::sscanf_s(text, "%8x-%4x-%4x-%2x%2x-%2x%2x%2x%2x%2x%2x", &d1, &d2, &d3, &b0, &b1, &b2, &b3, &b4, &b5, &b6, &b7);
  if (count != 11) { return false; }
  out->data1 = d1;
  out->data2 = static_cast<NvU16>(d2);
  out->data3 = static_cast<NvU16>(d3);
  out->data4[0] = static_cast<NvU8>(b0);
  out->data4[1] = static_cast<NvU8>(b1);
  out->data4[2] = static_cast<NvU8>(b2);
  out->data4[3] = static_cast<NvU8>(b3);
  out->data4[4] = static_cast<NvU8>(b4);
  out->data4[5] = static_cast<NvU8>(b5);
  out->data4[6] = static_cast<NvU8>(b6);
  out->data4[7] = static_cast<NvU8>(b7);
  return true;
}

bool ParseHexBytes(const char *text, std::vector<NvU8> &out) {
  out.clear();
  if (!text) { return false; }
  int hi = -1;
  for (const char *p = text; *p; ++p) {
    unsigned char c = static_cast<unsigned char>(*p);
    if (std::isspace(c) || c == ':' || c == ',' || c == '-') { continue; }
    int value = -1;
    if (c >= '0' && c <= '9') {
      value = c - '0';
    } else if (c >= 'a' && c <= 'f') {
      value = 10 + (c - 'a');
    } else if (c >= 'A' && c <= 'F') {
      value = 10 + (c - 'A');
    } else {
      return false;
    }
    if (hi < 0) {
      hi = value;
    } else {
      out.push_back(static_cast<NvU8>((hi << 4) | value));
      hi = -1;
    }
  }
  if (hi >= 0) { return false; }
  return !out.empty();
}

struct MonitorEnumState {
  NvU32 target = 0;
  NvU32 current = 0;
  HMONITOR found = nullptr;
};

BOOL CALLBACK MonitorEnumProc(HMONITOR hMonitor, HDC, LPRECT, LPARAM lParam) {
  MonitorEnumState *state = reinterpret_cast<MonitorEnumState *>(lParam);
  if (!state) { return FALSE; }
  if (state->current == state->target) {
    state->found = hMonitor;
    return FALSE;
  }
  ++state->current;
  return TRUE;
}

bool GetMonitorHandleByIndex(NvU32 index, NvMonitorHandle *out) {
  if (!out) { return false; }
  MonitorEnumState state;
  state.target = index;
  EnumDisplayMonitors(NULL, NULL, MonitorEnumProc, reinterpret_cast<LPARAM>(&state));
  if (!state.found) { return false; }
  *out = reinterpret_cast<NvMonitorHandle>(state.found);
  return true;
}

bool GetMonitorHandleByDisplayId(NvU32 displayId, NvMonitorHandle *out) {
  if (!out) { return false; }
  NvDisplayHandle displayHandle = nullptr;
  NvAPI_Status status = NvAPI_DISP_GetDisplayHandleFromDisplayId(displayId, &displayHandle);
  if (status != NVAPI_OK) {
    PrintNvapiError("NvAPI_DISP_GetDisplayHandleFromDisplayId failed", status);
    return false;
  }
  NvAPI_ShortString displayName = {0};
  status = NvAPI_GetAssociatedNvidiaDisplayName(displayHandle, displayName);
  if (status != NVAPI_OK) {
    PrintNvapiError("NvAPI_GetAssociatedNvidiaDisplayName failed", status);
    return false;
  }

  struct MatchState {
    const char *target;
    HMONITOR found;
  } match = {displayName, nullptr};

  auto EnumProc = [](HMONITOR hMonitor, HDC, LPRECT, LPARAM lParam) -> BOOL {
    MatchState *state = reinterpret_cast<MatchState *>(lParam);
    MONITORINFOEX info = {};
    info.cbSize = sizeof(info);
    if (GetMonitorInfo(hMonitor, &info)) {
      if (_stricmp(info.szDevice, state->target) == 0) {
        state->found = hMonitor;
        return FALSE;
      }
    }
    return TRUE;
  };

  EnumDisplayMonitors(NULL, NULL, EnumProc, reinterpret_cast<LPARAM>(&match));
  if (!match.found) { return false; }
  *out = reinterpret_cast<NvMonitorHandle>(match.found);
  return true;
}
const char *WindowedModeName(NVAPI_STEREO_WINDOWED_MODE mode) {
  switch (mode) {
  case WINDOWED_MODE_STEREO_OFF: return "off";
  case WINDOWED_MODE_STEREO_AUTOMATIC: return "automatic";
  case WINDOWED_MODE_STEREO_PERSISTENT: return "persistent";
  default: return "unknown";
  }
}

const char *DriverModeName(NV_STEREO_DRIVER_MODE mode) {
  switch (mode) {
  case NVAPI_STEREO_DRIVER_MODE_AUTOMATIC: return "automatic";
  case NVAPI_STEREO_DRIVER_MODE_DIRECT: return "direct";
  default: return "unknown";
  }
}

const char *ActiveEyeName(NV_STEREO_ACTIVE_EYE eye) {
  switch (eye) {
  case NVAPI_STEREO_EYE_RIGHT: return "right";
  case NVAPI_STEREO_EYE_LEFT: return "left";
  case NVAPI_STEREO_EYE_MONO: return "mono";
  default: return "unknown";
  }
}

const char *FrustumAdjustModeName(NV_FRUSTUM_ADJUST_MODE mode) {
  switch (mode) {
  case NVAPI_NO_FRUSTUM_ADJUST: return "none";
  case NVAPI_FRUSTUM_STRETCH: return "stretch";
  case NVAPI_FRUSTUM_CLEAR_EDGES: return "clear-edges";
  default: return "unknown";
  }
}

const char *SurfaceCreationModeName(NVAPI_STEREO_SURFACECREATEMODE mode) {
  switch (mode) {
  case NVAPI_STEREO_SURFACECREATEMODE_AUTO: return "auto";
  case NVAPI_STEREO_SURFACECREATEMODE_FORCESTEREO: return "force-stereo";
  case NVAPI_STEREO_SURFACECREATEMODE_FORCEMONO: return "force-mono";
  default: return "unknown";
  }
}

const char *StereoSrcLayoutName(STEREO_SRC_LAYOUT layout) {
  switch (layout) {
  case STEREO_SRC_LAYOUT_LEFT_RIGHT: return "left-right";
  case STEREO_SRC_LAYOUT_RIGHT_LEFT: return "right-left";
  case STEREO_SRC_LAYOUT_TOP_BOTTOM: return "top-bottom";
  case STEREO_SRC_LAYOUT_BOTTOM_TOP: return "bottom-top";
  case STEREO_SRC_LAYOUT_MONO_MODE: return "mono";
  default: return "unknown";
  }
}

const char *ModeEnumCommandName(STEREO_MODE_ENUM_CONTROL command) {
  switch (command) {
  case STEREO_MODE_ENUM_DISABLE: return "disable";
  case STEREO_MODE_ENUM_ENBALE: return "enable";
  case STEREO_MODE_ENUM_STATUS: return "status";
  default: return "unknown";
  }
}

const char *DongleCommandName(STEREO_DONGLE_CONTROL_COMMAND command) {
  switch (command) {
  case STEREO_DONGLE_COMMAND_POLL: return "poll";
  case STEREO_DONGLE_COMMAND_SUPPORTED: return "supported";
  case STEREO_DONGLE_COMMAND_SET_TIMING: return "set-timing";
  case STEREO_DONGLE_COMMAND_ACTIVATE: return "activate";
  case STEREO_DONGLE_COMMAND_DEACTIVATE: return "deactivate";
  case STEREO_DONGLE_COMMAND_AEGIS_SURROUND: return "aegis-surround";
  case STEREO_DONGLE_COMMAND_GET_ACTIVE_COUNT: return "active-count";
  case STEREO_DONGLE_COMMAND_SUPPORTED_DEVINDEPENDENT: return "supported-device-independent";
  default: return "unknown";
  }
}

bool ParseWindowedMode(const char *text, NVAPI_STEREO_WINDOWED_MODE *out) {
  if (!text || !out) { return false; }
  std::string value = ToLowerAscii(text);
  if (value == "off") {
    *out = WINDOWED_MODE_STEREO_OFF;
    return true;
  }
  if (value == "auto") {
    *out = WINDOWED_MODE_STEREO_AUTOMATIC;
    return true;
  }
  if (value == "persistent") {
    *out = WINDOWED_MODE_STEREO_PERSISTENT;
    return true;
  }
  return false;
}

bool ParseDriverMode(const char *text, NV_STEREO_DRIVER_MODE *out) {
  if (!text || !out) { return false; }
  std::string value = ToLowerAscii(text);
  if (value == "automatic") {
    *out = NVAPI_STEREO_DRIVER_MODE_AUTOMATIC;
    return true;
  }
  if (value == "direct") {
    *out = NVAPI_STEREO_DRIVER_MODE_DIRECT;
    return true;
  }
  return false;
}

bool ParseActiveEye(const char *text, NV_STEREO_ACTIVE_EYE *out) {
  if (!text || !out) { return false; }
  std::string value = ToLowerAscii(text);
  if (value == "left") {
    *out = NVAPI_STEREO_EYE_LEFT;
    return true;
  }
  if (value == "right") {
    *out = NVAPI_STEREO_EYE_RIGHT;
    return true;
  }
  if (value == "mono") {
    *out = NVAPI_STEREO_EYE_MONO;
    return true;
  }
  return false;
}

bool ParseFrustumAdjustMode(const char *text, NV_FRUSTUM_ADJUST_MODE *out) {
  if (!text || !out) { return false; }
  std::string value = ToLowerAscii(text);
  if (value == "none") {
    *out = NVAPI_NO_FRUSTUM_ADJUST;
    return true;
  }
  if (value == "stretch") {
    *out = NVAPI_FRUSTUM_STRETCH;
    return true;
  }
  if (value == "clear-edges") {
    *out = NVAPI_FRUSTUM_CLEAR_EDGES;
    return true;
  }
  return false;
}

bool ParseSurfaceCreationMode(const char *text, NVAPI_STEREO_SURFACECREATEMODE *out) {
  if (!text || !out) { return false; }
  std::string value = ToLowerAscii(text);
  if (value == "auto") {
    *out = NVAPI_STEREO_SURFACECREATEMODE_AUTO;
    return true;
  }
  if (value == "force-stereo") {
    *out = NVAPI_STEREO_SURFACECREATEMODE_FORCESTEREO;
    return true;
  }
  if (value == "force-mono") {
    *out = NVAPI_STEREO_SURFACECREATEMODE_FORCEMONO;
    return true;
  }
  return false;
}

bool ParseStereoSrcLayout(const char *text, STEREO_SRC_LAYOUT *out) {
  if (!text || !out) { return false; }
  std::string value = ToLowerAscii(text);
  if (value == "left-right") {
    *out = STEREO_SRC_LAYOUT_LEFT_RIGHT;
    return true;
  }
  if (value == "right-left") {
    *out = STEREO_SRC_LAYOUT_RIGHT_LEFT;
    return true;
  }
  if (value == "top-bottom") {
    *out = STEREO_SRC_LAYOUT_TOP_BOTTOM;
    return true;
  }
  if (value == "bottom-top") {
    *out = STEREO_SRC_LAYOUT_BOTTOM_TOP;
    return true;
  }
  if (value == "mono") {
    *out = STEREO_SRC_LAYOUT_MONO_MODE;
    return true;
  }
  return false;
}

bool ParseModeEnumCommand(const char *text, STEREO_MODE_ENUM_CONTROL *out) {
  if (!text || !out) { return false; }
  std::string value = ToLowerAscii(text);
  if (value == "disable") {
    *out = STEREO_MODE_ENUM_DISABLE;
    return true;
  }
  if (value == "enable") {
    *out = STEREO_MODE_ENUM_ENBALE;
    return true;
  }
  if (value == "status") {
    *out = STEREO_MODE_ENUM_STATUS;
    return true;
  }
  return false;
}

bool ParseDongleCommand(const char *text, STEREO_DONGLE_CONTROL_COMMAND *out) {
  if (!text || !out) { return false; }
  std::string value = ToLowerAscii(text);
  if (value == "poll") {
    *out = STEREO_DONGLE_COMMAND_POLL;
    return true;
  }
  if (value == "supported") {
    *out = STEREO_DONGLE_COMMAND_SUPPORTED;
    return true;
  }
  if (value == "set-timing") {
    *out = STEREO_DONGLE_COMMAND_SET_TIMING;
    return true;
  }
  if (value == "activate") {
    *out = STEREO_DONGLE_COMMAND_ACTIVATE;
    return true;
  }
  if (value == "deactivate") {
    *out = STEREO_DONGLE_COMMAND_DEACTIVATE;
    return true;
  }
  if (value == "aegis-surround") {
    *out = STEREO_DONGLE_COMMAND_AEGIS_SURROUND;
    return true;
  }
  if (value == "active-count") {
    *out = STEREO_DONGLE_COMMAND_GET_ACTIVE_COUNT;
    return true;
  }
  if (value == "supported-device-independent") {
    *out = STEREO_DONGLE_COMMAND_SUPPORTED_DEVINDEPENDENT;
    return true;
  }
  return false;
}

bool ParseRegistryProfileType(const char *text, NV_STEREO_REGISTRY_PROFILE_TYPE *out) {
  if (!text || !out) { return false; }
  std::string value = ToLowerAscii(text);
  if (value == "default") {
    *out = NVAPI_STEREO_DEFAULT_REGISTRY_PROFILE;
    return true;
  }
  if (value == "dx9") {
    *out = NVAPI_STEREO_DX9_REGISTRY_PROFILE;
    return true;
  }
  if (value == "dx10") {
    *out = NVAPI_STEREO_DX10_REGISTRY_PROFILE;
    return true;
  }
  return false;
}

bool ParseRegistryId(const char *text, NV_STEREO_REGISTRY_ID *out) {
  if (!text || !out) { return false; }
  std::string value = ToLowerAscii(text);
  if (value == "convergence") {
    *out = NVAPI_CONVERGENCE_ID;
    return true;
  }
  if (value == "frustum") {
    *out = NVAPI_FRUSTUM_ADJUST_MODE_ID;
    return true;
  }
  return false;
}

bool ParseInitActivationFlag(const char *text, NVAPI_STEREO_INIT_ACTIVATION_FLAGS *out) {
  if (!text || !out) { return false; }
  std::string value = ToLowerAscii(text);
  if (value == "immediate") {
    *out = NVAPI_STEREO_INIT_ACTIVATION_IMMEDIATE;
    return true;
  }
  if (value == "delayed") {
    *out = NVAPI_STEREO_INIT_ACTIVATION_DELAYED;
    return true;
  }
  return false;
}

bool ParseHandshakeCommand(const char *text, NVAPI_STEREO_HANDSHAKE_COMMANDS *out) {
  if (!text || !out) { return false; }
  std::string value = ToLowerAscii(text);
  if (value == "challenge") {
    *out = NVAPI_STEREO_HANDSHAKE_CMD_CHALLENGE;
    return true;
  }
  if (value == "response") {
    *out = NVAPI_STEREO_HANDSHAKE_CMD_RESPONSE;
    return true;
  }
  return false;
}

void DumpHex(const void *data, size_t size) {
  const unsigned char *bytes = static_cast<const unsigned char *>(data);
  for (size_t i = 0; i < size; ++i) {
    if (i % 16 == 0) { std::printf("    "); }
    std::printf("%02X ", bytes[i]);
    if (i % 16 == 15 || i + 1 == size) { std::printf("\n"); }
  }
}

void PrintStereoCaps(const NVAPI_STEREO_CAPS &caps) {
  std::printf("  windowedOff=%u windowedAuto=%u windowedPersistent=%u\n", caps.supportsWindowedModeOff,
              caps.supportsWindowedModeAutomatic, caps.supportsWindowedModePersistent);
}
void PrintStereoUsage() {
  std::printf("Stereo commands:\n");
  std::printf("  %s stereo enable\n", kToolName);
  std::printf("  %s stereo disable\n", kToolName);
  std::printf("  %s stereo is-enabled\n", kToolName);
  std::printf("  %s stereo windowed get\n", kToolName);
  std::printf("  %s stereo windowed set --mode off|auto|persistent [--flags N]\n", kToolName);
  std::printf("  %s stereo windowed supported\n", kToolName);
  std::printf("  %s stereo caps get|internal\n", kToolName);
  std::printf("  %s stereo caps monitor --monitor-index N | --id HEX\n", kToolName);
  std::printf("  %s stereo info [--handle-index N]\n", kToolName);
  std::printf("  %s stereo app-info [--handle-index N]\n", kToolName);
  std::printf("  %s stereo mode-enum get\n", kToolName);
  std::printf("  %s stereo mode-enum set --command enable|disable [--data N]\n", kToolName);
  std::printf("  %s stereo accessory\n", kToolName);
  std::printf("  %s stereo dongle control --command NAME [--data N] [--handle-index N]\n", kToolName);
  std::printf("  %s stereo dongle status --id HEX --param N\n", kToolName);
  std::printf("  %s stereo aegis --panel-id N\n", kToolName);
  std::printf("  %s stereo default-profile get\n", kToolName);
  std::printf("  %s stereo default-profile set --name NAME\n", kToolName);
  std::printf("  %s stereo profile create|delete --type default|dx9|dx10\n", kToolName);
  std::printf("  %s stereo profile set --type TYPE --id convergence|frustum (--dword N|--float F)\n", kToolName);
  std::printf("  %s stereo profile delete-value --type TYPE --id convergence|frustum\n", kToolName);
  std::printf("  %s stereo driver-mode set --mode automatic|direct\n", kToolName);
  std::printf("  %s stereo activate|deactivate|is-activated\n", kToolName);
  std::printf("  %s stereo separation get|set|inc|dec [--value PCT]\n", kToolName);
  std::printf("  %s stereo convergence get|set|inc|dec [--value F]\n", kToolName);
  std::printf("  %s stereo frustum get|set --mode none|stretch|clear-edges\n", kToolName);
  std::printf("  %s stereo capture jpeg --quality 0-100\n", kToolName);
  std::printf("  %s stereo capture png\n", kToolName);
  std::printf("  %s stereo init-activation --flag immediate|delayed\n", kToolName);
  std::printf("  %s stereo trigger-activation\n", kToolName);
  std::printf("  %s stereo reverse-blit --enable 0|1\n", kToolName);
  std::printf("  %s stereo notify --hwnd HEX --message-id N\n", kToolName);
  std::printf("  %s stereo active-eye set --eye left|right|mono\n", kToolName);
  std::printf("  %s stereo eye-separation get\n", kToolName);
  std::printf("  %s stereo cursor supported|get|set [--value PCT]\n", kToolName);
  std::printf("  %s stereo surface get|set --mode auto|force-stereo|force-mono\n", kToolName);
  std::printf("  %s stereo debug last-draw\n", kToolName);
  std::printf("  %s stereo force-to-screen --enable 0|1\n", kToolName);
  std::printf("  %s stereo video-control --layout NAME --client-id N --enable 0|1\n", kToolName);
  std::printf("  %s stereo video-metadata --width W --height H --src HEX --dst HEX\n", kToolName);
  std::printf("  %s stereo handshake challenge\n", kToolName);
  std::printf("  %s stereo handshake response --guid GUID --response-hex HEX\n", kToolName);
  std::printf("  %s stereo handshake-trigger\n", kToolName);
  std::printf("  %s stereo handshake-message --enable 0|1\n", kToolName);
  std::printf("  %s stereo profile-name set --name NAME [--flags N]\n", kToolName);
  std::printf("  %s stereo diag\n", kToolName);
  std::printf(
      "  %s stereo shader set --stage vs|ps --type f|i|b --start N --count N --mono PATH --left PATH --right PATH\n",
      kToolName);
  std::printf(
      "  %s stereo shader get --stage vs|ps --type f|i|b --start N --count N --mono PATH --left PATH --right PATH\n",
      kToolName);
}

int CmdStereoEnable(bool enable) {
  NvAPI_Status status = enable ? NvAPI_Stereo_Enable() : NvAPI_Stereo_Disable();
  if (status != NVAPI_OK) {
    PrintNvapiError(enable ? "NvAPI_Stereo_Enable failed" : "NvAPI_Stereo_Disable failed", status);
    return 1;
  }
  std::printf("Stereo %s.\n", enable ? "enabled" : "disabled");
  return 0;
}

int CmdStereoIsEnabled() {
  NvU8 enabled = 0;
  NvAPI_Status status = NvAPI_Stereo_IsEnabled(&enabled);
  if (status != NVAPI_OK) {
    PrintNvapiError("NvAPI_Stereo_IsEnabled failed", status);
    return 1;
  }
  std::printf("Stereo enabled: %s\n", enabled ? "yes" : "no");
  return 0;
}

int CmdStereoWindowed(int argc, char **argv) {
  if (argc < 1) {
    std::printf("Missing windowed command.\n");
    return 1;
  }
  if (std::strcmp(argv[0], "get") == 0) {
    NVAPI_STEREO_WINDOWED_MODE mode = WINDOWED_MODE_STEREO_OFF;
    NvAPI_Status status = NvAPI_Stereo_GetWindowedMode(&mode);
    if (status != NVAPI_OK) {
      PrintNvapiError("NvAPI_Stereo_GetWindowedMode failed", status);
      return 1;
    }
    std::printf("Windowed mode: %s\n", WindowedModeName(mode));
    return 0;
  }
  if (std::strcmp(argv[0], "supported") == 0) {
    NvU8 supported = 0;
    NvAPI_Status status = NvAPI_Stereo_IsWindowedModeSupported(&supported);
    if (status != NVAPI_OK) {
      PrintNvapiError("NvAPI_Stereo_IsWindowedModeSupported failed", status);
      return 1;
    }
    std::printf("Windowed mode supported: %s\n", supported ? "yes" : "no");
    return 0;
  }
  if (std::strcmp(argv[0], "set") == 0) {
    NVAPI_STEREO_WINDOWED_MODE mode = WINDOWED_MODE_STEREO_OFF;
    bool hasMode = false;
    NvU32 flags = 0;

    for (int i = 1; i < argc; ++i) {
      if (std::strcmp(argv[i], "--mode") == 0) {
        if (i + 1 >= argc) {
          std::printf("Missing value for --mode\n");
          return 1;
        }
        if (!ParseWindowedMode(argv[i + 1], &mode)) {
          std::printf("Invalid mode: %s\n", argv[i + 1]);
          return 1;
        }
        hasMode = true;
        ++i;
        continue;
      }
      if (std::strcmp(argv[i], "--flags") == 0) {
        if (i + 1 >= argc) {
          std::printf("Missing value for --flags\n");
          return 1;
        }
        if (!ParseUint(argv[i + 1], &flags)) {
          std::printf("Invalid flags: %s\n", argv[i + 1]);
          return 1;
        }
        ++i;
        continue;
      }
      std::printf("Unknown option: %s\n", argv[i]);
      return 1;
    }

    if (!hasMode) {
      std::printf("Missing required --mode.\n");
      return 1;
    }

    NvAPI_Status status = NvAPI_Stereo_SetWindowedMode(mode, flags);
    if (status != NVAPI_OK) {
      PrintNvapiError("NvAPI_Stereo_SetWindowedMode failed", status);
      return 1;
    }
    std::printf("Windowed mode updated to %s.\n", WindowedModeName(mode));
    return 0;
  }

  std::printf("Unknown windowed command: %s\n", argv[0]);
  return 1;
}

int CmdStereoCaps(int argc, char **argv) {
  if (argc < 1) {
    std::printf("Missing caps command.\n");
    return 1;
  }
  if (std::strcmp(argv[0], "get") == 0) {
    NVAPI_STEREO_CAPS caps = {};
    caps.version = NVAPI_STEREO_CAPS_VER;
    NvAPI_Status status = NvAPI_Stereo_GetStereoCaps(&caps);
    if (status != NVAPI_OK) {
      PrintNvapiError("NvAPI_Stereo_GetStereoCaps failed", status);
      return 1;
    }
    PrintStereoCaps(caps);
    return 0;
  }
  if (std::strcmp(argv[0], "internal") == 0) {
    NVAPI_STEREO_CAPS caps = {};
    caps.version = NVAPI_STEREO_CAPS_VER;
    NvAPI_Status status = NvAPI_Stereo_GetStereoCapsInternal(&caps);
    if (status != NVAPI_OK) {
      PrintNvapiError("NvAPI_Stereo_GetStereoCapsInternal failed", status);
      return 1;
    }
    PrintStereoCaps(caps);
    return 0;
  }
  if (std::strcmp(argv[0], "monitor") == 0) {
    NvMonitorHandle monitor = nullptr;
    bool hasMonitorIndex = false;
    bool hasDisplayId = false;
    NvU32 monitorIndex = 0;
    NvU32 displayId = 0;

    for (int i = 1; i < argc; ++i) {
      if (std::strcmp(argv[i], "--monitor-index") == 0) {
        if (i + 1 >= argc) {
          std::printf("Missing value for --monitor-index\n");
          return 1;
        }
        if (!ParseUint(argv[i + 1], &monitorIndex)) {
          std::printf("Invalid monitor index: %s\n", argv[i + 1]);
          return 1;
        }
        hasMonitorIndex = true;
        ++i;
        continue;
      }
      if (std::strcmp(argv[i], "--id") == 0) {
        if (i + 1 >= argc) {
          std::printf("Missing value for --id\n");
          return 1;
        }
        if (!ParseUint(argv[i + 1], &displayId)) {
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

    if (hasMonitorIndex) {
      if (!GetMonitorHandleByIndex(monitorIndex, &monitor)) {
        std::printf("Failed to resolve monitor index %u.\n", monitorIndex);
        return 1;
      }
    } else if (hasDisplayId) {
      if (!GetMonitorHandleByDisplayId(displayId, &monitor)) {
        std::printf("Failed to resolve monitor for display id 0x%08X.\n", displayId);
        return 1;
      }
    } else {
      std::printf("Missing --monitor-index or --id for monitor caps.\n");
      return 1;
    }

    NVAPI_STEREO_CAPS caps = {};
    caps.version = NVAPI_STEREO_CAPS_VER;
    NvAPI_Status status = NvAPI_Stereo_GetStereoSupport(monitor, &caps);
    if (status != NVAPI_OK) {
      PrintNvapiError("NvAPI_Stereo_GetStereoSupport failed", status);
      return 1;
    }
    PrintStereoCaps(caps);
    return 0;
  }

  std::printf("Unknown caps command: %s\n", argv[0]);
  return 1;
}
int CmdStereoInfo(int argc, char **argv) {
  NvU32 handleIndex = 0;
  for (int i = 0; i < argc; ++i) {
    if (std::strcmp(argv[i], "--handle-index") == 0) {
      if (i + 1 >= argc) {
        std::printf("Missing value for --handle-index\n");
        return 1;
      }
      if (!ParseUint(argv[i + 1], &handleIndex)) {
        std::printf("Invalid handle index: %s\n", argv[i + 1]);
        return 1;
      }
      ++i;
      continue;
    }
    std::printf("Unknown option: %s\n", argv[i]);
    return 1;
  }

  NvDisplayHandle displayHandle = nullptr;
  if (!GetDisplayHandleByIndex(handleIndex, &displayHandle)) {
    std::printf("Failed to resolve display handle %u.\n", handleIndex);
    return 1;
  }

  NV_STEREO_DIAG_INFO info = {};
  info.version = NV_STEREO_DIAG_INFO_VER;
  info.queryId = NV_STEREO_DIAG_INFO_QUERY_ID_COMMON;
  NvAPI_Status status = NvAPI_Stereo_GetInfo(displayHandle, &info);
  if (status != NVAPI_OK) {
    PrintNvapiError("NvAPI_Stereo_GetInfo failed", status);
    return 1;
  }

  const NV_STEREO_DIAG_COMMON_INFO &common = info.commonInfo;
  std::printf("Stereo info result=%u\n", info.result);
  std::printf("  enabled=%u active=%u clients=%u devices=%d\n", common.status.isStereoEnabled,
              common.status.isStereoActive, common.status.numStereoClients, common.status.stereoDeviceCount);
  for (NvU32 i = 0; i < NV_STEREO_MAX_HEADS; ++i) {
    std::printf("  head[%u] config=0x%08X pending=0x%08X persistent=0x%08X\n", i, common.status.stereoConfig[i],
                common.status.pendingStereoConfig[i], common.status.persistentStereoConfig[i]);
  }
  std::printf("  emitter init=%u connected=%u activated=%u activeHead=%u activeCount=%u\n", common.emitter.isInit,
              common.emitter.isConnected, common.emitter.isActivated, common.emitter.activatedHead,
              common.emitter.activeCount);
  std::printf("  aero appCount=%u appPid=%u originalRR=%u\n", common.aero.appCount, common.aero.appPid,
              common.aero.orgRR);
  return 0;
}

int CmdStereoAppInfo(int argc, char **argv) {
  NvU32 handleIndex = 0;
  for (int i = 0; i < argc; ++i) {
    if (std::strcmp(argv[i], "--handle-index") == 0) {
      if (i + 1 >= argc) {
        std::printf("Missing value for --handle-index\n");
        return 1;
      }
      if (!ParseUint(argv[i + 1], &handleIndex)) {
        std::printf("Invalid handle index: %s\n", argv[i + 1]);
        return 1;
      }
      ++i;
      continue;
    }
    std::printf("Unknown option: %s\n", argv[i]);
    return 1;
  }

  NvDisplayHandle displayHandle = nullptr;
  if (!GetDisplayHandleByIndex(handleIndex, &displayHandle)) {
    std::printf("Failed to resolve display handle %u.\n", handleIndex);
    return 1;
  }

  NV_STEREO_DIAG_APP_INFO info = {};
  info.version = NV_STEREO_DIAG_APP_INFO_VER;
  info.appCount = NV_STEREO_INFO_MAX_APPS;
  info.appDataSize = NV_STEREO_APP_INFO_SIZE;
  NvAPI_Status status = NvAPI_Stereo_GetAppInfo(displayHandle, &info);
  if (status != NVAPI_OK) {
    PrintNvapiError("NvAPI_Stereo_GetAppInfo failed", status);
    return 1;
  }

  std::printf("App info count=%u dataSize=%u\n", info.appCount, info.appDataSize);
  for (NvU32 i = 0; i < info.appCount && i < NV_STEREO_INFO_MAX_APPS; ++i) {
    const NV_STEREO_APP_INFO &app = info.info[i];
    const char *dxName = "unknown";
    if (app.dxVersion == NV_STEREO_INFO_DX9_APP) {
      dxName = "dx9";
    } else if (app.dxVersion == NV_STEREO_INFO_DX10_APP) {
      dxName = "dx10";
    }
    std::printf("  app[%u] dx=%s pid=%u\n", i, dxName, app.processId);
  }
  return 0;
}

int CmdStereoModeEnum(int argc, char **argv) {
  if (argc < 1) {
    std::printf("Missing mode-enum command.\n");
    return 1;
  }
  if (std::strcmp(argv[0], "get") == 0) {
    NVAPI_STEREO_MODE_ENUM_CONTROL control = {};
    control.version = NVAPI_STEREO_MODE_ENUM_CONTROL_VER;
    control.command = STEREO_MODE_ENUM_STATUS;
    control.data = 0;
    NvAPI_Status status = NvAPI_Stereo_ModeEnumControl(&control);
    if (status != NVAPI_OK) {
      PrintNvapiError("NvAPI_Stereo_ModeEnumControl failed", status);
      return 1;
    }
    std::printf("Mode enum status: command=%s data=%u\n", ModeEnumCommandName(control.command), control.data);
    return 0;
  }
  if (std::strcmp(argv[0], "set") == 0) {
    STEREO_MODE_ENUM_CONTROL command = STEREO_MODE_ENUM_DISABLE;
    bool hasCommand = false;
    NvU32 data = 0;

    for (int i = 1; i < argc; ++i) {
      if (std::strcmp(argv[i], "--command") == 0) {
        if (i + 1 >= argc) {
          std::printf("Missing value for --command\n");
          return 1;
        }
        if (!ParseModeEnumCommand(argv[i + 1], &command)) {
          std::printf("Invalid command: %s\n", argv[i + 1]);
          return 1;
        }
        hasCommand = true;
        ++i;
        continue;
      }
      if (std::strcmp(argv[i], "--data") == 0) {
        if (i + 1 >= argc) {
          std::printf("Missing value for --data\n");
          return 1;
        }
        if (!ParseUint(argv[i + 1], &data)) {
          std::printf("Invalid data: %s\n", argv[i + 1]);
          return 1;
        }
        ++i;
        continue;
      }
      std::printf("Unknown option: %s\n", argv[i]);
      return 1;
    }

    if (!hasCommand) {
      std::printf("Missing required --command.\n");
      return 1;
    }

    NVAPI_STEREO_MODE_ENUM_CONTROL control = {};
    control.version = NVAPI_STEREO_MODE_ENUM_CONTROL_VER;
    control.command = command;
    control.data = data;
    NvAPI_Status status = NvAPI_Stereo_ModeEnumControl(&control);
    if (status != NVAPI_OK) {
      PrintNvapiError("NvAPI_Stereo_ModeEnumControl failed", status);
      return 1;
    }
    std::printf("Mode enum updated: command=%s data=%u\n", ModeEnumCommandName(control.command), control.data);
    return 0;
  }

  std::printf("Unknown mode-enum command: %s\n", argv[0]);
  return 1;
}

int CmdStereoAccessory() {
  NvU32 enabled = 0;
  NvAPI_Status status = NvAPI_Stereo_IsAccessoryDisplayEnabled(&enabled);
  if (status != NVAPI_OK) {
    PrintNvapiError("NvAPI_Stereo_IsAccessoryDisplayEnabled failed", status);
    return 1;
  }
  std::printf("Accessory display enabled: %s\n", enabled ? "yes" : "no");
  return 0;
}

int CmdStereoDongle(int argc, char **argv) {
  if (argc < 1) {
    std::printf("Missing dongle command.\n");
    return 1;
  }
  if (std::strcmp(argv[0], "status") == 0) {
    NvU32 displayId = 0;
    NvU32 param = 0;
    bool hasDisplayId = false;
    bool hasParam = false;

    for (int i = 1; i < argc; ++i) {
      if (std::strcmp(argv[i], "--id") == 0) {
        if (i + 1 >= argc) {
          std::printf("Missing value for --id\n");
          return 1;
        }
        if (!ParseUint(argv[i + 1], &displayId)) {
          std::printf("Invalid display id: %s\n", argv[i + 1]);
          return 1;
        }
        hasDisplayId = true;
        ++i;
        continue;
      }
      if (std::strcmp(argv[i], "--param") == 0) {
        if (i + 1 >= argc) {
          std::printf("Missing value for --param\n");
          return 1;
        }
        if (!ParseUint(argv[i + 1], &param)) {
          std::printf("Invalid param: %s\n", argv[i + 1]);
          return 1;
        }
        hasParam = true;
        ++i;
        continue;
      }
      std::printf("Unknown option: %s\n", argv[i]);
      return 1;
    }

    if (!hasDisplayId || !hasParam) {
      std::printf("Missing --id or --param for dongle status.\n");
      return 1;
    }

    NvU32 result = 0;
    NvAPI_Status status = NvAPI_Stereo_Dongle_Status(displayId, static_cast<NvU8>(param), &result);
    if (status != NVAPI_OK) {
      PrintNvapiError("NvAPI_Stereo_Dongle_Status failed", status);
      return 1;
    }
    std::printf("Dongle status result=%u\n", result);
    return 0;
  }
  if (std::strcmp(argv[0], "control") == 0) {
    NvU32 handleIndex = 0;
    STEREO_DONGLE_CONTROL_COMMAND command = STEREO_DONGLE_COMMAND_POLL;
    NvU32 data = 0;
    bool hasCommand = false;

    for (int i = 1; i < argc; ++i) {
      if (std::strcmp(argv[i], "--handle-index") == 0) {
        if (i + 1 >= argc) {
          std::printf("Missing value for --handle-index\n");
          return 1;
        }
        if (!ParseUint(argv[i + 1], &handleIndex)) {
          std::printf("Invalid handle index: %s\n", argv[i + 1]);
          return 1;
        }
        ++i;
        continue;
      }
      if (std::strcmp(argv[i], "--command") == 0) {
        if (i + 1 >= argc) {
          std::printf("Missing value for --command\n");
          return 1;
        }
        if (!ParseDongleCommand(argv[i + 1], &command)) {
          std::printf("Invalid dongle command: %s\n", argv[i + 1]);
          return 1;
        }
        hasCommand = true;
        ++i;
        continue;
      }
      if (std::strcmp(argv[i], "--data") == 0) {
        if (i + 1 >= argc) {
          std::printf("Missing value for --data\n");
          return 1;
        }
        if (!ParseUint(argv[i + 1], &data)) {
          std::printf("Invalid data: %s\n", argv[i + 1]);
          return 1;
        }
        ++i;
        continue;
      }
      std::printf("Unknown option: %s\n", argv[i]);
      return 1;
    }

    if (!hasCommand) {
      std::printf("Missing required --command.\n");
      return 1;
    }

    NvDisplayHandle displayHandle = nullptr;
    if (!GetDisplayHandleByIndex(handleIndex, &displayHandle)) {
      std::printf("Failed to resolve display handle %u.\n", handleIndex);
      return 1;
    }

    NVAPI_STEREO_DONGLE_CONTROL control = {};
    control.version = NVAPI_STEREO_DONGLE_CONTROL_VER;
    control.command = command;
    control.data = data;
    NvAPI_Status status = NvAPI_Stereo_DongleControl(displayHandle, &control);
    if (status != NVAPI_OK) {
      PrintNvapiError("NvAPI_Stereo_DongleControl failed", status);
      return 1;
    }
    std::printf("Dongle control ok: command=%s data=%u result=%u\n", DongleCommandName(control.command), control.data,
                control.result);
    return 0;
  }

  std::printf("Unknown dongle command: %s\n", argv[0]);
  return 1;
}

int CmdStereoAegis(int argc, char **argv) {
  NvU32 panelId = 0;
  bool hasPanel = false;
  for (int i = 0; i < argc; ++i) {
    if (std::strcmp(argv[i], "--panel-id") == 0) {
      if (i + 1 >= argc) {
        std::printf("Missing value for --panel-id\n");
        return 1;
      }
      if (!ParseUint(argv[i + 1], &panelId)) {
        std::printf("Invalid panel id: %s\n", argv[i + 1]);
        return 1;
      }
      hasPanel = true;
      ++i;
      continue;
    }
    std::printf("Unknown option: %s\n", argv[i]);
    return 1;
  }
  if (!hasPanel) {
    std::printf("Missing required --panel-id.\n");
    return 1;
  }
  NvU8 detected = 0;
  NvAPI_Status status = NvAPI_Stereo_IsDisplayAegisDTType(panelId, &detected);
  if (status != NVAPI_OK) {
    PrintNvapiError("NvAPI_Stereo_IsDisplayAegisDTType failed", status);
    return 1;
  }
  std::printf("Aegis DT detected: %s\n", detected ? "yes" : "no");
  return 0;
}

int CmdStereoDefaultProfile(int argc, char **argv) {
  if (argc < 1) {
    std::printf("Missing default-profile command.\n");
    return 1;
  }
  if (std::strcmp(argv[0], "get") == 0) {
    NvAPI_String name = {};
    NvU32 sizeOut = 0;
    NvAPI_Status status = NvAPI_Stereo_GetDefaultProfile(sizeof(name), name, &sizeOut);
    if (status != NVAPI_OK) {
      PrintNvapiError("NvAPI_Stereo_GetDefaultProfile failed", status);
      return 1;
    }
    std::printf("Default profile: %s\n", name);
    return 0;
  }
  if (std::strcmp(argv[0], "set") == 0) {
    const char *name = nullptr;
    for (int i = 1; i < argc; ++i) {
      if (std::strcmp(argv[i], "--name") == 0) {
        if (i + 1 >= argc) {
          std::printf("Missing value for --name\n");
          return 1;
        }
        name = argv[i + 1];
        ++i;
        continue;
      }
      std::printf("Unknown option: %s\n", argv[i]);
      return 1;
    }
    if (!name) {
      std::printf("Missing required --name.\n");
      return 1;
    }
    NvAPI_Status status = NvAPI_Stereo_SetDefaultProfile(name);
    if (status != NVAPI_OK) {
      PrintNvapiError("NvAPI_Stereo_SetDefaultProfile failed", status);
      return 1;
    }
    std::printf("Default profile updated.\n");
    return 0;
  }
  std::printf("Unknown default-profile command: %s\n", argv[0]);
  return 1;
}

int CmdStereoProfile(int argc, char **argv) {
  if (argc < 1) {
    std::printf("Missing profile command.\n");
    return 1;
  }
  NV_STEREO_REGISTRY_PROFILE_TYPE type = NVAPI_STEREO_DEFAULT_REGISTRY_PROFILE;
  bool hasType = false;
  NV_STEREO_REGISTRY_ID id = NVAPI_CONVERGENCE_ID;
  bool hasId = false;
  NvU32 dwordValue = 0;
  float floatValue = 0.0f;
  bool hasDword = false;
  bool hasFloat = false;

  auto ParseCommon = [&](int start) -> bool {
    for (int i = start; i < argc; ++i) {
      if (std::strcmp(argv[i], "--type") == 0) {
        if (i + 1 >= argc) {
          std::printf("Missing value for --type\n");
          return false;
        }
        if (!ParseRegistryProfileType(argv[i + 1], &type)) {
          std::printf("Invalid profile type: %s\n", argv[i + 1]);
          return false;
        }
        hasType = true;
        ++i;
        continue;
      }
      if (std::strcmp(argv[i], "--id") == 0) {
        if (i + 1 >= argc) {
          std::printf("Missing value for --id\n");
          return false;
        }
        if (!ParseRegistryId(argv[i + 1], &id)) {
          std::printf("Invalid registry id: %s\n", argv[i + 1]);
          return false;
        }
        hasId = true;
        ++i;
        continue;
      }
      if (std::strcmp(argv[i], "--dword") == 0) {
        if (i + 1 >= argc) {
          std::printf("Missing value for --dword\n");
          return false;
        }
        if (!ParseUint(argv[i + 1], &dwordValue)) {
          std::printf("Invalid dword: %s\n", argv[i + 1]);
          return false;
        }
        hasDword = true;
        ++i;
        continue;
      }
      if (std::strcmp(argv[i], "--float") == 0) {
        if (i + 1 >= argc) {
          std::printf("Missing value for --float\n");
          return false;
        }
        if (!ParseFloatValue(argv[i + 1], &floatValue)) {
          std::printf("Invalid float: %s\n", argv[i + 1]);
          return false;
        }
        hasFloat = true;
        ++i;
        continue;
      }
      std::printf("Unknown option: %s\n", argv[i]);
      return false;
    }
    return true;
  };

  if (std::strcmp(argv[0], "create") == 0) {
    if (!ParseCommon(1)) { return 1; }
    if (!hasType) {
      std::printf("Missing required --type.\n");
      return 1;
    }
    NvAPI_Status status = NvAPI_Stereo_CreateConfigurationProfileRegistryKey(type);
    if (status != NVAPI_OK) {
      PrintNvapiError("NvAPI_Stereo_CreateConfigurationProfileRegistryKey failed", status);
      return 1;
    }
    std::printf("Profile key created.\n");
    return 0;
  }
  if (std::strcmp(argv[0], "delete") == 0) {
    if (!ParseCommon(1)) { return 1; }
    if (!hasType) {
      std::printf("Missing required --type.\n");
      return 1;
    }
    NvAPI_Status status = NvAPI_Stereo_DeleteConfigurationProfileRegistryKey(type);
    if (status != NVAPI_OK) {
      PrintNvapiError("NvAPI_Stereo_DeleteConfigurationProfileRegistryKey failed", status);
      return 1;
    }
    std::printf("Profile key deleted.\n");
    return 0;
  }
  if (std::strcmp(argv[0], "set") == 0) {
    if (!ParseCommon(1)) { return 1; }
    if (!hasType || !hasId) {
      std::printf("Missing required --type or --id.\n");
      return 1;
    }
    if (!hasDword && !hasFloat) {
      std::printf("Missing --dword or --float value.\n");
      return 1;
    }

    NvAPI_Status status = NVAPI_OK;
    if (hasDword) {
      status = NvAPI_Stereo_SetConfigurationProfileValue(type, id, &dwordValue);
    } else {
      status = NvAPI_Stereo_SetConfigurationProfileValue(type, id, &floatValue);
    }
    if (status != NVAPI_OK) {
      PrintNvapiError("NvAPI_Stereo_SetConfigurationProfileValue failed", status);
      return 1;
    }
    std::printf("Profile value updated.\n");
    return 0;
  }
  if (std::strcmp(argv[0], "delete-value") == 0) {
    if (!ParseCommon(1)) { return 1; }
    if (!hasType || !hasId) {
      std::printf("Missing required --type or --id.\n");
      return 1;
    }
    NvAPI_Status status = NvAPI_Stereo_DeleteConfigurationProfileValue(type, id);
    if (status != NVAPI_OK) {
      PrintNvapiError("NvAPI_Stereo_DeleteConfigurationProfileValue failed", status);
      return 1;
    }
    std::printf("Profile value deleted.\n");
    return 0;
  }

  std::printf("Unknown profile command: %s\n", argv[0]);
  return 1;
}

int CmdStereoDriverMode(int argc, char **argv) {
  if (argc < 1 || std::strcmp(argv[0], "set") != 0) {
    std::printf("Driver-mode command requires: set --mode automatic|direct\n");
    return 1;
  }
  NV_STEREO_DRIVER_MODE mode = NVAPI_STEREO_DRIVER_MODE_AUTOMATIC;
  bool hasMode = false;
  for (int i = 1; i < argc; ++i) {
    if (std::strcmp(argv[i], "--mode") == 0) {
      if (i + 1 >= argc) {
        std::printf("Missing value for --mode\n");
        return 1;
      }
      if (!ParseDriverMode(argv[i + 1], &mode)) {
        std::printf("Invalid mode: %s\n", argv[i + 1]);
        return 1;
      }
      hasMode = true;
      ++i;
      continue;
    }
    std::printf("Unknown option: %s\n", argv[i]);
    return 1;
  }
  if (!hasMode) {
    std::printf("Missing required --mode.\n");
    return 1;
  }
  NvAPI_Status status = NvAPI_Stereo_SetDriverMode(mode);
  if (status != NVAPI_OK) {
    PrintNvapiError("NvAPI_Stereo_SetDriverMode failed", status);
    return 1;
  }
  std::printf("Driver mode set to %s.\n", DriverModeName(mode));
  return 0;
}
int CmdStereoActivation(const char *command) {
  StereoContext ctx;
  if (!ctx.Init()) { return 1; }
  NvAPI_Status status = NVAPI_OK;
  if (std::strcmp(command, "activate") == 0) {
    status = NvAPI_Stereo_Activate(ctx.handle);
  } else if (std::strcmp(command, "deactivate") == 0) {
    status = NvAPI_Stereo_Deactivate(ctx.handle);
  } else if (std::strcmp(command, "is-activated") == 0) {
    NvU8 active = 0;
    status = NvAPI_Stereo_IsActivated(ctx.handle, &active);
    if (status == NVAPI_OK) { std::printf("Stereo activated: %s\n", active ? "yes" : "no"); }
  } else {
    std::printf("Unknown activation command: %s\n", command);
    ctx.Cleanup();
    return 1;
  }
  if (status != NVAPI_OK) { PrintNvapiError("Stereo activation command failed", status); }
  ctx.Cleanup();
  return status == NVAPI_OK ? 0 : 1;
}

int CmdStereoSeparation(int argc, char **argv) {
  if (argc < 1) {
    std::printf("Missing separation command.\n");
    return 1;
  }
  float value = 0.0f;
  bool hasValue = false;
  for (int i = 1; i < argc; ++i) {
    if (std::strcmp(argv[i], "--value") == 0) {
      if (i + 1 >= argc) {
        std::printf("Missing value for --value\n");
        return 1;
      }
      if (!ParseFloatValue(argv[i + 1], &value)) {
        std::printf("Invalid value: %s\n", argv[i + 1]);
        return 1;
      }
      hasValue = true;
      ++i;
      continue;
    }
    std::printf("Unknown option: %s\n", argv[i]);
    return 1;
  }

  StereoContext ctx;
  if (!ctx.Init()) { return 1; }
  NvAPI_Status status = NVAPI_OK;
  if (std::strcmp(argv[0], "get") == 0) {
    status = NvAPI_Stereo_GetSeparation(ctx.handle, &value);
    if (status == NVAPI_OK) { std::printf("Separation: %.3f\n", value); }
  } else if (std::strcmp(argv[0], "set") == 0) {
    if (!hasValue) {
      std::printf("Missing required --value.\n");
      ctx.Cleanup();
      return 1;
    }
    status = NvAPI_Stereo_SetSeparation(ctx.handle, value);
  } else if (std::strcmp(argv[0], "inc") == 0) {
    status = NvAPI_Stereo_IncreaseSeparation(ctx.handle);
  } else if (std::strcmp(argv[0], "dec") == 0) {
    status = NvAPI_Stereo_DecreaseSeparation(ctx.handle);
  } else {
    std::printf("Unknown separation command: %s\n", argv[0]);
    ctx.Cleanup();
    return 1;
  }

  if (status != NVAPI_OK) { PrintNvapiError("NvAPI_Stereo separation failed", status); }
  ctx.Cleanup();
  return status == NVAPI_OK ? 0 : 1;
}

int CmdStereoConvergence(int argc, char **argv) {
  if (argc < 1) {
    std::printf("Missing convergence command.\n");
    return 1;
  }
  float value = 0.0f;
  bool hasValue = false;
  for (int i = 1; i < argc; ++i) {
    if (std::strcmp(argv[i], "--value") == 0) {
      if (i + 1 >= argc) {
        std::printf("Missing value for --value\n");
        return 1;
      }
      if (!ParseFloatValue(argv[i + 1], &value)) {
        std::printf("Invalid value: %s\n", argv[i + 1]);
        return 1;
      }
      hasValue = true;
      ++i;
      continue;
    }
    std::printf("Unknown option: %s\n", argv[i]);
    return 1;
  }

  StereoContext ctx;
  if (!ctx.Init()) { return 1; }
  NvAPI_Status status = NVAPI_OK;
  if (std::strcmp(argv[0], "get") == 0) {
    status = NvAPI_Stereo_GetConvergence(ctx.handle, &value);
    if (status == NVAPI_OK) { std::printf("Convergence: %.3f\n", value); }
  } else if (std::strcmp(argv[0], "set") == 0) {
    if (!hasValue) {
      std::printf("Missing required --value.\n");
      ctx.Cleanup();
      return 1;
    }
    status = NvAPI_Stereo_SetConvergence(ctx.handle, value);
  } else if (std::strcmp(argv[0], "inc") == 0) {
    status = NvAPI_Stereo_IncreaseConvergence(ctx.handle);
  } else if (std::strcmp(argv[0], "dec") == 0) {
    status = NvAPI_Stereo_DecreaseConvergence(ctx.handle);
  } else {
    std::printf("Unknown convergence command: %s\n", argv[0]);
    ctx.Cleanup();
    return 1;
  }

  if (status != NVAPI_OK) { PrintNvapiError("NvAPI_Stereo convergence failed", status); }
  ctx.Cleanup();
  return status == NVAPI_OK ? 0 : 1;
}

int CmdStereoFrustum(int argc, char **argv) {
  if (argc < 1) {
    std::printf("Missing frustum command.\n");
    return 1;
  }
  NV_FRUSTUM_ADJUST_MODE mode = NVAPI_NO_FRUSTUM_ADJUST;
  bool hasMode = false;
  for (int i = 1; i < argc; ++i) {
    if (std::strcmp(argv[i], "--mode") == 0) {
      if (i + 1 >= argc) {
        std::printf("Missing value for --mode\n");
        return 1;
      }
      if (!ParseFrustumAdjustMode(argv[i + 1], &mode)) {
        std::printf("Invalid mode: %s\n", argv[i + 1]);
        return 1;
      }
      hasMode = true;
      ++i;
      continue;
    }
    std::printf("Unknown option: %s\n", argv[i]);
    return 1;
  }

  StereoContext ctx;
  if (!ctx.Init()) { return 1; }
  NvAPI_Status status = NVAPI_OK;
  if (std::strcmp(argv[0], "get") == 0) {
    status = NvAPI_Stereo_GetFrustumAdjustMode(ctx.handle, &mode);
    if (status == NVAPI_OK) { std::printf("Frustum adjust mode: %s\n", FrustumAdjustModeName(mode)); }
  } else if (std::strcmp(argv[0], "set") == 0) {
    if (!hasMode) {
      std::printf("Missing required --mode.\n");
      ctx.Cleanup();
      return 1;
    }
    status = NvAPI_Stereo_SetFrustumAdjustMode(ctx.handle, mode);
  } else {
    std::printf("Unknown frustum command: %s\n", argv[0]);
    ctx.Cleanup();
    return 1;
  }

  if (status != NVAPI_OK) { PrintNvapiError("NvAPI_Stereo frustum failed", status); }
  ctx.Cleanup();
  return status == NVAPI_OK ? 0 : 1;
}

int CmdStereoCapture(int argc, char **argv) {
  if (argc < 1) {
    std::printf("Missing capture command.\n");
    return 1;
  }
  StereoContext ctx;
  if (!ctx.Init()) { return 1; }
  NvAPI_Status status = NVAPI_OK;
  if (std::strcmp(argv[0], "jpeg") == 0) {
    NvU32 quality = 90;
    for (int i = 1; i < argc; ++i) {
      if (std::strcmp(argv[i], "--quality") == 0) {
        if (i + 1 >= argc) {
          std::printf("Missing value for --quality\n");
          ctx.Cleanup();
          return 1;
        }
        if (!ParseUint(argv[i + 1], &quality)) {
          std::printf("Invalid quality: %s\n", argv[i + 1]);
          ctx.Cleanup();
          return 1;
        }
        ++i;
        continue;
      }
      std::printf("Unknown option: %s\n", argv[i]);
      ctx.Cleanup();
      return 1;
    }
    status = NvAPI_Stereo_CaptureJpegImage(ctx.handle, quality);
  } else if (std::strcmp(argv[0], "png") == 0) {
    status = NvAPI_Stereo_CapturePngImage(ctx.handle);
  } else {
    std::printf("Unknown capture command: %s\n", argv[0]);
    ctx.Cleanup();
    return 1;
  }
  if (status != NVAPI_OK) { PrintNvapiError("NvAPI_Stereo_Capture failed", status); }
  ctx.Cleanup();
  return status == NVAPI_OK ? 0 : 1;
}

int CmdStereoInitActivation(int argc, char **argv) {
  NVAPI_STEREO_INIT_ACTIVATION_FLAGS flag = NVAPI_STEREO_INIT_ACTIVATION_IMMEDIATE;
  bool hasFlag = false;
  for (int i = 0; i < argc; ++i) {
    if (std::strcmp(argv[i], "--flag") == 0) {
      if (i + 1 >= argc) {
        std::printf("Missing value for --flag\n");
        return 1;
      }
      if (!ParseInitActivationFlag(argv[i + 1], &flag)) {
        std::printf("Invalid flag: %s\n", argv[i + 1]);
        return 1;
      }
      hasFlag = true;
      ++i;
      continue;
    }
    std::printf("Unknown option: %s\n", argv[i]);
    return 1;
  }
  if (!hasFlag) {
    std::printf("Missing required --flag.\n");
    return 1;
  }
  StereoContext ctx;
  if (!ctx.Init()) { return 1; }
  NvAPI_Status status = NvAPI_Stereo_InitActivation(ctx.handle, flag);
  if (status != NVAPI_OK) { PrintNvapiError("NvAPI_Stereo_InitActivation failed", status); }
  ctx.Cleanup();
  return status == NVAPI_OK ? 0 : 1;
}

int CmdStereoTriggerActivation() {
  StereoContext ctx;
  if (!ctx.Init()) { return 1; }
  NvAPI_Status status = NvAPI_Stereo_Trigger_Activation(ctx.handle);
  if (status != NVAPI_OK) { PrintNvapiError("NvAPI_Stereo_Trigger_Activation failed", status); }
  ctx.Cleanup();
  return status == NVAPI_OK ? 0 : 1;
}

int CmdStereoReverseBlit(int argc, char **argv) {
  bool enable = false;
  bool hasEnable = false;
  for (int i = 0; i < argc; ++i) {
    if (std::strcmp(argv[i], "--enable") == 0) {
      if (i + 1 >= argc) {
        std::printf("Missing value for --enable\n");
        return 1;
      }
      if (!ParseBoolValue(argv[i + 1], &enable)) {
        std::printf("Invalid enable value: %s\n", argv[i + 1]);
        return 1;
      }
      hasEnable = true;
      ++i;
      continue;
    }
    std::printf("Unknown option: %s\n", argv[i]);
    return 1;
  }
  if (!hasEnable) {
    std::printf("Missing required --enable.\n");
    return 1;
  }
  StereoContext ctx;
  if (!ctx.Init()) { return 1; }
  NvAPI_Status status = NvAPI_Stereo_ReverseStereoBlitControl(ctx.handle, enable ? 1 : 0);
  if (status != NVAPI_OK) { PrintNvapiError("NvAPI_Stereo_ReverseStereoBlitControl failed", status); }
  ctx.Cleanup();
  return status == NVAPI_OK ? 0 : 1;
}

int CmdStereoNotify(int argc, char **argv) {
  NvU64 hwndValue = 0;
  NvU64 msgId = 0;
  bool hasHwnd = false;
  bool hasMsg = false;
  for (int i = 0; i < argc; ++i) {
    if (std::strcmp(argv[i], "--hwnd") == 0) {
      if (i + 1 >= argc) {
        std::printf("Missing value for --hwnd\n");
        return 1;
      }
      if (!ParseU64(argv[i + 1], &hwndValue)) {
        std::printf("Invalid hwnd: %s\n", argv[i + 1]);
        return 1;
      }
      hasHwnd = true;
      ++i;
      continue;
    }
    if (std::strcmp(argv[i], "--message-id") == 0) {
      if (i + 1 >= argc) {
        std::printf("Missing value for --message-id\n");
        return 1;
      }
      if (!ParseU64(argv[i + 1], &msgId)) {
        std::printf("Invalid message id: %s\n", argv[i + 1]);
        return 1;
      }
      hasMsg = true;
      ++i;
      continue;
    }
    std::printf("Unknown option: %s\n", argv[i]);
    return 1;
  }
  if (!hasHwnd || !hasMsg) {
    std::printf("Missing required --hwnd and --message-id.\n");
    return 1;
  }
  StereoContext ctx;
  if (!ctx.Init()) { return 1; }
  NvAPI_Status status = NvAPI_Stereo_SetNotificationMessage(ctx.handle, hwndValue, msgId);
  if (status != NVAPI_OK) { PrintNvapiError("NvAPI_Stereo_SetNotificationMessage failed", status); }
  ctx.Cleanup();
  return status == NVAPI_OK ? 0 : 1;
}

int CmdStereoActiveEye(int argc, char **argv) {
  if (argc < 1 || std::strcmp(argv[0], "set") != 0) {
    std::printf("Active-eye command requires: set --eye left|right|mono\n");
    return 1;
  }
  NV_STEREO_ACTIVE_EYE eye = NVAPI_STEREO_EYE_LEFT;
  bool hasEye = false;
  for (int i = 1; i < argc; ++i) {
    if (std::strcmp(argv[i], "--eye") == 0) {
      if (i + 1 >= argc) {
        std::printf("Missing value for --eye\n");
        return 1;
      }
      if (!ParseActiveEye(argv[i + 1], &eye)) {
        std::printf("Invalid eye: %s\n", argv[i + 1]);
        return 1;
      }
      hasEye = true;
      ++i;
      continue;
    }
    std::printf("Unknown option: %s\n", argv[i]);
    return 1;
  }
  if (!hasEye) {
    std::printf("Missing required --eye.\n");
    return 1;
  }
  StereoContext ctx;
  if (!ctx.Init()) { return 1; }
  NvAPI_Status status = NvAPI_Stereo_SetActiveEye(ctx.handle, eye);
  if (status != NVAPI_OK) { PrintNvapiError("NvAPI_Stereo_SetActiveEye failed", status); }
  ctx.Cleanup();
  return status == NVAPI_OK ? 0 : 1;
}

int CmdStereoEyeSeparation() {
  StereoContext ctx;
  if (!ctx.Init()) { return 1; }
  float separation = 0.0f;
  NvAPI_Status status = NvAPI_Stereo_GetEyeSeparation(ctx.handle, &separation);
  if (status != NVAPI_OK) {
    PrintNvapiError("NvAPI_Stereo_GetEyeSeparation failed", status);
  } else {
    std::printf("Eye separation: %.6f\n", separation);
  }
  ctx.Cleanup();
  return status == NVAPI_OK ? 0 : 1;
}

int CmdStereoCursor(int argc, char **argv) {
  if (argc < 1) {
    std::printf("Missing cursor command.\n");
    return 1;
  }
  if (std::strcmp(argv[0], "supported") == 0) {
    StereoContext ctx;
    if (!ctx.Init()) { return 1; }
    NvU8 supported = 0;
    NvAPI_Status status = NvAPI_Stereo_Is3DCursorSupported(ctx.handle, &supported);
    if (status != NVAPI_OK) {
      PrintNvapiError("NvAPI_Stereo_Is3DCursorSupported failed", status);
    } else {
      std::printf("3D cursor supported: %s\n", supported ? "yes" : "no");
    }
    ctx.Cleanup();
    return status == NVAPI_OK ? 0 : 1;
  }
  if (std::strcmp(argv[0], "get") == 0) {
    StereoContext ctx;
    if (!ctx.Init()) { return 1; }
    float value = 0.0f;
    NvAPI_Status status = NvAPI_Stereo_GetCursorSeparation(ctx.handle, &value);
    if (status != NVAPI_OK) {
      PrintNvapiError("NvAPI_Stereo_GetCursorSeparation failed", status);
    } else {
      std::printf("Cursor separation: %.3f\n", value);
    }
    ctx.Cleanup();
    return status == NVAPI_OK ? 0 : 1;
  }
  if (std::strcmp(argv[0], "set") == 0) {
    float value = 0.0f;
    bool hasValue = false;
    for (int i = 1; i < argc; ++i) {
      if (std::strcmp(argv[i], "--value") == 0) {
        if (i + 1 >= argc) {
          std::printf("Missing value for --value\n");
          return 1;
        }
        if (!ParseFloatValue(argv[i + 1], &value)) {
          std::printf("Invalid value: %s\n", argv[i + 1]);
          return 1;
        }
        hasValue = true;
        ++i;
        continue;
      }
      std::printf("Unknown option: %s\n", argv[i]);
      return 1;
    }
    if (!hasValue) {
      std::printf("Missing required --value.\n");
      return 1;
    }
    StereoContext ctx;
    if (!ctx.Init()) { return 1; }
    NvAPI_Status status = NvAPI_Stereo_SetCursorSeparation(ctx.handle, value);
    if (status != NVAPI_OK) { PrintNvapiError("NvAPI_Stereo_SetCursorSeparation failed", status); }
    ctx.Cleanup();
    return status == NVAPI_OK ? 0 : 1;
  }
  std::printf("Unknown cursor command: %s\n", argv[0]);
  return 1;
}

int CmdStereoSurface(int argc, char **argv) {
  if (argc < 1) {
    std::printf("Missing surface command.\n");
    return 1;
  }
  if (std::strcmp(argv[0], "get") == 0) {
    StereoContext ctx;
    if (!ctx.Init()) { return 1; }
    NVAPI_STEREO_SURFACECREATEMODE mode = NVAPI_STEREO_SURFACECREATEMODE_AUTO;
    NvAPI_Status status = NvAPI_Stereo_GetSurfaceCreationMode(ctx.handle, &mode);
    if (status != NVAPI_OK) {
      PrintNvapiError("NvAPI_Stereo_GetSurfaceCreationMode failed", status);
    } else {
      std::printf("Surface creation mode: %s\n", SurfaceCreationModeName(mode));
    }
    ctx.Cleanup();
    return status == NVAPI_OK ? 0 : 1;
  }
  if (std::strcmp(argv[0], "set") == 0) {
    NVAPI_STEREO_SURFACECREATEMODE mode = NVAPI_STEREO_SURFACECREATEMODE_AUTO;
    bool hasMode = false;
    for (int i = 1; i < argc; ++i) {
      if (std::strcmp(argv[i], "--mode") == 0) {
        if (i + 1 >= argc) {
          std::printf("Missing value for --mode\n");
          return 1;
        }
        if (!ParseSurfaceCreationMode(argv[i + 1], &mode)) {
          std::printf("Invalid mode: %s\n", argv[i + 1]);
          return 1;
        }
        hasMode = true;
        ++i;
        continue;
      }
      std::printf("Unknown option: %s\n", argv[i]);
      return 1;
    }
    if (!hasMode) {
      std::printf("Missing required --mode.\n");
      return 1;
    }
    StereoContext ctx;
    if (!ctx.Init()) { return 1; }
    NvAPI_Status status = NvAPI_Stereo_SetSurfaceCreationMode(ctx.handle, mode);
    if (status != NVAPI_OK) { PrintNvapiError("NvAPI_Stereo_SetSurfaceCreationMode failed", status); }
    ctx.Cleanup();
    return status == NVAPI_OK ? 0 : 1;
  }
  std::printf("Unknown surface command: %s\n", argv[0]);
  return 1;
}

int CmdStereoDebugLastDraw() {
  StereoContext ctx;
  if (!ctx.Init()) { return 1; }
  NvU8 stereoized = 0;
  NvAPI_Status status = NvAPI_Stereo_Debug_WasLastDrawStereoized(ctx.handle, &stereoized);
  if (status != NVAPI_OK) {
    PrintNvapiError("NvAPI_Stereo_Debug_WasLastDrawStereoized failed", status);
  } else {
    std::printf("Last draw stereoized: %s\n", stereoized ? "yes" : "no");
  }
  ctx.Cleanup();
  return status == NVAPI_OK ? 0 : 1;
}

int CmdStereoForceToScreen(int argc, char **argv) {
  bool enable = false;
  bool hasEnable = false;
  for (int i = 0; i < argc; ++i) {
    if (std::strcmp(argv[i], "--enable") == 0) {
      if (i + 1 >= argc) {
        std::printf("Missing value for --enable\n");
        return 1;
      }
      if (!ParseBoolValue(argv[i + 1], &enable)) {
        std::printf("Invalid enable value: %s\n", argv[i + 1]);
        return 1;
      }
      hasEnable = true;
      ++i;
      continue;
    }
    std::printf("Unknown option: %s\n", argv[i]);
    return 1;
  }
  if (!hasEnable) {
    std::printf("Missing required --enable.\n");
    return 1;
  }
  StereoContext ctx;
  if (!ctx.Init()) { return 1; }
  NvAPI_Status status = NvAPI_Stereo_ForceToScreenDepth(ctx.handle, enable ? 1 : 0);
  if (status != NVAPI_OK) { PrintNvapiError("NvAPI_Stereo_ForceToScreenDepth failed", status); }
  ctx.Cleanup();
  return status == NVAPI_OK ? 0 : 1;
}
int CmdStereoVideoControl(int argc, char **argv) {
  STEREO_SRC_LAYOUT layout = STEREO_SRC_LAYOUT_LEFT_RIGHT;
  bool hasLayout = false;
  NvU32 clientId = 0;
  bool hasClient = false;
  bool enable = false;
  bool hasEnable = false;

  for (int i = 0; i < argc; ++i) {
    if (std::strcmp(argv[i], "--layout") == 0) {
      if (i + 1 >= argc) {
        std::printf("Missing value for --layout\n");
        return 1;
      }
      if (!ParseStereoSrcLayout(argv[i + 1], &layout)) {
        std::printf("Invalid layout: %s\n", argv[i + 1]);
        return 1;
      }
      hasLayout = true;
      ++i;
      continue;
    }
    if (std::strcmp(argv[i], "--client-id") == 0) {
      if (i + 1 >= argc) {
        std::printf("Missing value for --client-id\n");
        return 1;
      }
      if (!ParseUint(argv[i + 1], &clientId)) {
        std::printf("Invalid client id: %s\n", argv[i + 1]);
        return 1;
      }
      hasClient = true;
      ++i;
      continue;
    }
    if (std::strcmp(argv[i], "--enable") == 0) {
      if (i + 1 >= argc) {
        std::printf("Missing value for --enable\n");
        return 1;
      }
      if (!ParseBoolValue(argv[i + 1], &enable)) {
        std::printf("Invalid enable value: %s\n", argv[i + 1]);
        return 1;
      }
      hasEnable = true;
      ++i;
      continue;
    }
    std::printf("Unknown option: %s\n", argv[i]);
    return 1;
  }

  if (!hasLayout || !hasClient || !hasEnable) {
    std::printf("Missing required --layout, --client-id, or --enable.\n");
    return 1;
  }

  StereoContext ctx;
  if (!ctx.Init()) { return 1; }
  NVAPI_STEREO_VIDEO_CONTROL control = {};
  control.dwVersion = NVAPI_STEREO_VIDEO_CONTROL_VER;
  control.eStereoSrcLayout = layout;
  control.dwClientIdentifier = clientId;
  control.dwStereoEnable = enable ? 1 : 0;
  NvAPI_Status status = NvAPI_Stereo_SetVideoControl(ctx.handle, &control);
  if (status != NVAPI_OK) { PrintNvapiError("NvAPI_Stereo_SetVideoControl failed", status); }
  ctx.Cleanup();
  return status == NVAPI_OK ? 0 : 1;
}

int CmdStereoVideoMetadata(int argc, char **argv) {
  NvU32 width = 0;
  NvU32 height = 0;
  NvU64 src = 0;
  NvU64 dst = 0;
  bool hasWidth = false;
  bool hasHeight = false;
  bool hasSrc = false;
  bool hasDst = false;

  for (int i = 0; i < argc; ++i) {
    if (std::strcmp(argv[i], "--width") == 0) {
      if (i + 1 >= argc) {
        std::printf("Missing value for --width\n");
        return 1;
      }
      if (!ParseUint(argv[i + 1], &width)) {
        std::printf("Invalid width: %s\n", argv[i + 1]);
        return 1;
      }
      hasWidth = true;
      ++i;
      continue;
    }
    if (std::strcmp(argv[i], "--height") == 0) {
      if (i + 1 >= argc) {
        std::printf("Missing value for --height\n");
        return 1;
      }
      if (!ParseUint(argv[i + 1], &height)) {
        std::printf("Invalid height: %s\n", argv[i + 1]);
        return 1;
      }
      hasHeight = true;
      ++i;
      continue;
    }
    if (std::strcmp(argv[i], "--src") == 0) {
      if (i + 1 >= argc) {
        std::printf("Missing value for --src\n");
        return 1;
      }
      if (!ParseU64(argv[i + 1], &src)) {
        std::printf("Invalid src: %s\n", argv[i + 1]);
        return 1;
      }
      hasSrc = true;
      ++i;
      continue;
    }
    if (std::strcmp(argv[i], "--dst") == 0) {
      if (i + 1 >= argc) {
        std::printf("Missing value for --dst\n");
        return 1;
      }
      if (!ParseU64(argv[i + 1], &dst)) {
        std::printf("Invalid dst: %s\n", argv[i + 1]);
        return 1;
      }
      hasDst = true;
      ++i;
      continue;
    }
    std::printf("Unknown option: %s\n", argv[i]);
    return 1;
  }

  if (!hasWidth || !hasHeight || !hasSrc || !hasDst) {
    std::printf("Missing required --width, --height, --src, or --dst.\n");
    return 1;
  }

  StereoContext ctx;
  if (!ctx.Init()) { return 1; }
  NVAPI_STEREO_VIDEO_METADATA meta = {};
  meta.dwVersion = NVAPI_STEREO_VIDEO_METADATA_VER;
  meta.dwVideoWidth = width;
  meta.dwVideoHeight = height;
  meta.hSrcLuma = src;
  meta.hDst = dst;
  NvAPI_Status status = NvAPI_Stereo_SetVideoMetadata(ctx.handle, &meta);
  if (status != NVAPI_OK) { PrintNvapiError("NvAPI_Stereo_SetVideoMetadata failed", status); }
  ctx.Cleanup();
  return status == NVAPI_OK ? 0 : 1;
}
int CmdStereoHandshake(int argc, char **argv) {
  if (argc < 1) {
    std::printf("Missing handshake command.\n");
    return 1;
  }
  StereoContext ctx;
  if (!ctx.Init()) { return 1; }
  NVAPI_STEREO_HANDSHAKE_PARAMS params = {};
  params.version = NVAPI_STEREO_HANDSHAKE_PARAMS_VER;

  if (std::strcmp(argv[0], "challenge") == 0) {
    params.dwCommand = NVAPI_STEREO_HANDSHAKE_CMD_CHALLENGE;
    NvAPI_Status status = NvAPI_Stereo_AppHandShake(ctx.handle, &params);
    if (status != NVAPI_OK) {
      PrintNvapiError("NvAPI_Stereo_AppHandShake failed", status);
      ctx.Cleanup();
      return 1;
    }
    std::printf("Handshake challenge: ");
    for (NvU32 i = 0; i < NVAPI_STEREO_HANDSHAKE_CHALLENGE_SIZE; ++i) {
      std::printf("%02X", params.ChallengeParams.challenge[i]);
    }
    std::printf("\n");
    ctx.Cleanup();
    return 0;
  }
  if (std::strcmp(argv[0], "response") == 0) {
    const char *guidText = nullptr;
    const char *respText = nullptr;
    NvU32 flags = 0;
    bool hasFlags = false;
    for (int i = 1; i < argc; ++i) {
      if (std::strcmp(argv[i], "--guid") == 0) {
        if (i + 1 >= argc) {
          std::printf("Missing value for --guid\n");
          ctx.Cleanup();
          return 1;
        }
        guidText = argv[i + 1];
        ++i;
        continue;
      }
      if (std::strcmp(argv[i], "--response-hex") == 0) {
        if (i + 1 >= argc) {
          std::printf("Missing value for --response-hex\n");
          ctx.Cleanup();
          return 1;
        }
        respText = argv[i + 1];
        ++i;
        continue;
      }
      if (std::strcmp(argv[i], "--flags") == 0) {
        if (i + 1 >= argc) {
          std::printf("Missing value for --flags\n");
          ctx.Cleanup();
          return 1;
        }
        if (!ParseUint(argv[i + 1], &flags)) {
          std::printf("Invalid flags: %s\n", argv[i + 1]);
          ctx.Cleanup();
          return 1;
        }
        hasFlags = true;
        ++i;
        continue;
      }
      std::printf("Unknown option: %s\n", argv[i]);
      ctx.Cleanup();
      return 1;
    }
    if (!guidText || !respText) {
      std::printf("Missing required --guid or --response-hex.\n");
      ctx.Cleanup();
      return 1;
    }
    NvGUID guid = {};
    if (!ParseGuidString(guidText, &guid)) {
      std::printf("Invalid GUID: %s\n", guidText);
      ctx.Cleanup();
      return 1;
    }
    std::vector<NvU8> bytes;
    if (!ParseHexBytes(respText, bytes) || bytes.size() != NVAPI_STEREO_HANDSHAKE_RESPONSE_SIZE) {
      std::printf("Invalid response hex; expected %u bytes.\n", NVAPI_STEREO_HANDSHAKE_RESPONSE_SIZE);
      ctx.Cleanup();
      return 1;
    }
    params.dwCommand = NVAPI_STEREO_HANDSHAKE_CMD_RESPONSE;
    params.ResponseParams.vendorGUID = guid;
    std::memset(params.ResponseParams.response, 0, sizeof(params.ResponseParams.response));
    std::memcpy(params.ResponseParams.response, bytes.data(), bytes.size());
    params.ResponseParams.flags = hasFlags ? flags : 0;

    NvAPI_Status status = NvAPI_Stereo_AppHandShake(ctx.handle, &params);
    if (status != NVAPI_OK) {
      PrintNvapiError("NvAPI_Stereo_AppHandShake failed", status);
      ctx.Cleanup();
      return 1;
    }
    std::printf("Handshake response submitted.\n");
    ctx.Cleanup();
    return 0;
  }

  std::printf("Unknown handshake command: %s\n", argv[0]);
  ctx.Cleanup();
  return 1;
}

int CmdStereoHandshakeTrigger() {
  StereoContext ctx;
  if (!ctx.Init()) { return 1; }
  NvAPI_Status status = NvAPI_Stereo_HandShake_Trigger_Activation(ctx.handle);
  if (status != NVAPI_OK) { PrintNvapiError("NvAPI_Stereo_HandShake_Trigger_Activation failed", status); }
  ctx.Cleanup();
  return status == NVAPI_OK ? 0 : 1;
}

int CmdStereoHandshakeMessage(int argc, char **argv) {
  bool enable = false;
  bool hasEnable = false;
  for (int i = 0; i < argc; ++i) {
    if (std::strcmp(argv[i], "--enable") == 0) {
      if (i + 1 >= argc) {
        std::printf("Missing value for --enable\n");
        return 1;
      }
      if (!ParseBoolValue(argv[i + 1], &enable)) {
        std::printf("Invalid enable value: %s\n", argv[i + 1]);
        return 1;
      }
      hasEnable = true;
      ++i;
      continue;
    }
    std::printf("Unknown option: %s\n", argv[i]);
    return 1;
  }
  if (!hasEnable) {
    std::printf("Missing required --enable.\n");
    return 1;
  }
  StereoContext ctx;
  if (!ctx.Init()) { return 1; }
  NvAPI_Status status = NvAPI_Stereo_HandShake_Message_Control(ctx.handle, enable ? 1 : 0);
  if (status != NVAPI_OK) { PrintNvapiError("NvAPI_Stereo_HandShake_Message_Control failed", status); }
  ctx.Cleanup();
  return status == NVAPI_OK ? 0 : 1;
}
int CmdStereoProfileName(int argc, char **argv) {
  if (argc < 1 || std::strcmp(argv[0], "set") != 0) {
    std::printf("Profile-name command requires: set --name NAME\n");
    return 1;
  }
  const char *name = nullptr;
  NvU32 flags = 0;
  for (int i = 1; i < argc; ++i) {
    if (std::strcmp(argv[i], "--name") == 0) {
      if (i + 1 >= argc) {
        std::printf("Missing value for --name\n");
        return 1;
      }
      name = argv[i + 1];
      ++i;
      continue;
    }
    if (std::strcmp(argv[i], "--flags") == 0) {
      if (i + 1 >= argc) {
        std::printf("Missing value for --flags\n");
        return 1;
      }
      if (!ParseUint(argv[i + 1], &flags)) {
        std::printf("Invalid flags: %s\n", argv[i + 1]);
        return 1;
      }
      ++i;
      continue;
    }
    std::printf("Unknown option: %s\n", argv[i]);
    return 1;
  }
  if (!name) {
    std::printf("Missing required --name.\n");
    return 1;
  }
  StereoContext ctx;
  if (!ctx.Init()) { return 1; }
  NvAPI_String profile = {};
  strncpy_s(profile, sizeof(profile), name, _TRUNCATE);
  NvAPI_Status status = NvAPI_Stereo_SetProfileName(ctx.handle, profile, ctx.d3d.device, flags);
  if (status != NVAPI_OK) { PrintNvapiError("NvAPI_Stereo_SetProfileName failed", status); }
  ctx.Cleanup();
  return status == NVAPI_OK ? 0 : 1;
}

int CmdStereoDiag() {
  StereoContext ctx;
  if (!ctx.Init()) { return 1; }
  NVAPI_STEREO_DIAG_KMD kmd = {};
  kmd.version = NVAPI_STEREO_DIAG_KMD_VER;
  NVAPI_STEREO_DIAG diag = {};
  diag.version = NVAPI_STEREO_DIAG_VER;
  diag.pKMDData = &kmd;
  diag.pUMDData = nullptr;

  NvAPI_Status status = NvAPI_Stereo_GetStereoDiag(ctx.handle, &diag);
  if (status != NVAPI_OK) {
    PrintNvapiError("NvAPI_Stereo_GetStereoDiag failed", status);
    ctx.Cleanup();
    return 1;
  }

  std::printf("Stereo diag: dongleActiveCount=%u\n", kmd.ulDongleActiveCount);
  for (NvU32 i = 0; i < NV_MAX_HEADS; ++i) {
    std::printf("  head[%u] stereoConfig=0x%08X\n", i, kmd.ulStereoConfig[i]);
  }
  ctx.Cleanup();
  return 0;
}
enum class ShaderStage { Vertex, Pixel };

enum class ShaderType { Float, Int, Bool };

bool ParseShaderStage(const char *text, ShaderStage *out) {
  if (!text || !out) { return false; }
  std::string value = ToLowerAscii(text);
  if (value == "vs") {
    *out = ShaderStage::Vertex;
    return true;
  }
  if (value == "ps") {
    *out = ShaderStage::Pixel;
    return true;
  }
  return false;
}

bool ParseShaderType(const char *text, ShaderType *out) {
  if (!text || !out) { return false; }
  std::string value = ToLowerAscii(text);
  if (value == "f") {
    *out = ShaderType::Float;
    return true;
  }
  if (value == "i") {
    *out = ShaderType::Int;
    return true;
  }
  if (value == "b") {
    *out = ShaderType::Bool;
    return true;
  }
  return false;
}
int CmdStereoShader(int argc, char **argv) {
  if (argc < 1) {
    std::printf("Missing shader command.\n");
    return 1;
  }
  bool isSet = false;
  if (std::strcmp(argv[0], "set") == 0) {
    isSet = true;
  } else if (std::strcmp(argv[0], "get") == 0) {
    isSet = false;
  } else {
    std::printf("Unknown shader command: %s\n", argv[0]);
    return 1;
  }

  ShaderStage stage = ShaderStage::Vertex;
  ShaderType type = ShaderType::Float;
  bool hasStage = false;
  bool hasType = false;
  NvU32 startReg = 0;
  NvU32 count = 0;
  bool hasStart = false;
  bool hasCount = false;
  const char *monoPath = nullptr;
  const char *leftPath = nullptr;
  const char *rightPath = nullptr;

  for (int i = 1; i < argc; ++i) {
    if (std::strcmp(argv[i], "--stage") == 0) {
      if (i + 1 >= argc) {
        std::printf("Missing value for --stage\n");
        return 1;
      }
      if (!ParseShaderStage(argv[i + 1], &stage)) {
        std::printf("Invalid stage: %s\n", argv[i + 1]);
        return 1;
      }
      hasStage = true;
      ++i;
      continue;
    }
    if (std::strcmp(argv[i], "--type") == 0) {
      if (i + 1 >= argc) {
        std::printf("Missing value for --type\n");
        return 1;
      }
      if (!ParseShaderType(argv[i + 1], &type)) {
        std::printf("Invalid type: %s\n", argv[i + 1]);
        return 1;
      }
      hasType = true;
      ++i;
      continue;
    }
    if (std::strcmp(argv[i], "--start") == 0) {
      if (i + 1 >= argc) {
        std::printf("Missing value for --start\n");
        return 1;
      }
      if (!ParseUint(argv[i + 1], &startReg)) {
        std::printf("Invalid start: %s\n", argv[i + 1]);
        return 1;
      }
      hasStart = true;
      ++i;
      continue;
    }
    if (std::strcmp(argv[i], "--count") == 0) {
      if (i + 1 >= argc) {
        std::printf("Missing value for --count\n");
        return 1;
      }
      if (!ParseUint(argv[i + 1], &count)) {
        std::printf("Invalid count: %s\n", argv[i + 1]);
        return 1;
      }
      hasCount = true;
      ++i;
      continue;
    }
    if (std::strcmp(argv[i], "--mono") == 0) {
      if (i + 1 >= argc) {
        std::printf("Missing value for --mono\n");
        return 1;
      }
      monoPath = argv[i + 1];
      ++i;
      continue;
    }
    if (std::strcmp(argv[i], "--left") == 0) {
      if (i + 1 >= argc) {
        std::printf("Missing value for --left\n");
        return 1;
      }
      leftPath = argv[i + 1];
      ++i;
      continue;
    }
    if (std::strcmp(argv[i], "--right") == 0) {
      if (i + 1 >= argc) {
        std::printf("Missing value for --right\n");
        return 1;
      }
      rightPath = argv[i + 1];
      ++i;
      continue;
    }
    std::printf("Unknown option: %s\n", argv[i]);
    return 1;
  }

  if (!hasStage || !hasType || !hasStart || !hasCount) {
    std::printf("Missing required --stage, --type, --start, or --count.\n");
    return 1;
  }
  if (isSet && (!monoPath || !leftPath || !rightPath)) {
    std::printf("Missing required --mono/--left/--right files for set.\n");
    return 1;
  }
  if (!isSet && (!monoPath || !leftPath || !rightPath)) {
    std::printf("Missing required --mono/--left/--right output files for get.\n");
    return 1;
  }
  size_t elementCount = (type == ShaderType::Bool) ? count : static_cast<size_t>(count) * 4;
  size_t elementSize =
      (type == ShaderType::Bool) ? sizeof(BOOL) : (type == ShaderType::Int ? sizeof(int) : sizeof(float));
  size_t byteSize = elementCount * elementSize;

  StereoContext ctx;
  if (!ctx.Init()) { return 1; }

  NvAPI_Status status = NVAPI_OK;
  if (type == ShaderType::Float) {
    std::vector<float> mono(elementCount);
    std::vector<float> left(elementCount);
    std::vector<float> right(elementCount);
    if (isSet) {
      if (!ReadBinaryFile(monoPath, mono.data(), byteSize) || !ReadBinaryFile(leftPath, left.data(), byteSize) ||
          !ReadBinaryFile(rightPath, right.data(), byteSize)) {
        ctx.Cleanup();
        return 1;
      }
    }
    if (stage == ShaderStage::Vertex) {
      if (isSet) {
        status =
            NvAPI_Stereo_SetVertexShaderConstantF(ctx.handle, startReg, mono.data(), left.data(), right.data(), count);
      } else {
        status =
            NvAPI_Stereo_GetVertexShaderConstantF(ctx.handle, startReg, mono.data(), left.data(), right.data(), count);
      }
    } else {
      if (isSet) {
        status =
            NvAPI_Stereo_SetPixelShaderConstantF(ctx.handle, startReg, mono.data(), left.data(), right.data(), count);
      } else {
        status =
            NvAPI_Stereo_GetPixelShaderConstantF(ctx.handle, startReg, mono.data(), left.data(), right.data(), count);
      }
    }
    if (!isSet && status == NVAPI_OK) {
      if (!WriteBinaryFile(monoPath, mono.data(), byteSize) || !WriteBinaryFile(leftPath, left.data(), byteSize) ||
          !WriteBinaryFile(rightPath, right.data(), byteSize)) {
        ctx.Cleanup();
        return 1;
      }
    }
  } else if (type == ShaderType::Int) {
    std::vector<int> mono(elementCount);
    std::vector<int> left(elementCount);
    std::vector<int> right(elementCount);
    if (isSet) {
      if (!ReadBinaryFile(monoPath, mono.data(), byteSize) || !ReadBinaryFile(leftPath, left.data(), byteSize) ||
          !ReadBinaryFile(rightPath, right.data(), byteSize)) {
        ctx.Cleanup();
        return 1;
      }
    }
    if (stage == ShaderStage::Vertex) {
      if (isSet) {
        status =
            NvAPI_Stereo_SetVertexShaderConstantI(ctx.handle, startReg, mono.data(), left.data(), right.data(), count);
      } else {
        status =
            NvAPI_Stereo_GetVertexShaderConstantI(ctx.handle, startReg, mono.data(), left.data(), right.data(), count);
      }
    } else {
      if (isSet) {
        status =
            NvAPI_Stereo_SetPixelShaderConstantI(ctx.handle, startReg, mono.data(), left.data(), right.data(), count);
      } else {
        status =
            NvAPI_Stereo_GetPixelShaderConstantI(ctx.handle, startReg, mono.data(), left.data(), right.data(), count);
      }
    }
    if (!isSet && status == NVAPI_OK) {
      if (!WriteBinaryFile(monoPath, mono.data(), byteSize) || !WriteBinaryFile(leftPath, left.data(), byteSize) ||
          !WriteBinaryFile(rightPath, right.data(), byteSize)) {
        ctx.Cleanup();
        return 1;
      }
    }
  } else {
    std::vector<BOOL> mono(elementCount);
    std::vector<BOOL> left(elementCount);
    std::vector<BOOL> right(elementCount);
    if (isSet) {
      if (!ReadBinaryFile(monoPath, mono.data(), byteSize) || !ReadBinaryFile(leftPath, left.data(), byteSize) ||
          !ReadBinaryFile(rightPath, right.data(), byteSize)) {
        ctx.Cleanup();
        return 1;
      }
    }
    if (stage == ShaderStage::Vertex) {
      if (isSet) {
        status =
            NvAPI_Stereo_SetVertexShaderConstantB(ctx.handle, startReg, mono.data(), left.data(), right.data(), count);
      } else {
        status =
            NvAPI_Stereo_GetVertexShaderConstantB(ctx.handle, startReg, mono.data(), left.data(), right.data(), count);
      }
    } else {
      if (isSet) {
        status =
            NvAPI_Stereo_SetPixelShaderConstantB(ctx.handle, startReg, mono.data(), left.data(), right.data(), count);
      } else {
        status =
            NvAPI_Stereo_GetPixelShaderConstantB(ctx.handle, startReg, mono.data(), left.data(), right.data(), count);
      }
    }
    if (!isSet && status == NVAPI_OK) {
      if (!WriteBinaryFile(monoPath, mono.data(), byteSize) || !WriteBinaryFile(leftPath, left.data(), byteSize) ||
          !WriteBinaryFile(rightPath, right.data(), byteSize)) {
        ctx.Cleanup();
        return 1;
      }
    }
  }

  if (status != NVAPI_OK) { PrintNvapiError("Stereo shader constants failed", status); }
  ctx.Cleanup();
  return status == NVAPI_OK ? 0 : 1;
}
} // namespace

int CmdStereo(int argc, char **argv) {
  if (argc < 1) {
    PrintStereoUsage();
    return 1;
  }
  if (std::strcmp(argv[0], "help") == 0) {
    PrintStereoUsage();
    return 0;
  }

  if (std::strcmp(argv[0], "enable") == 0) { return CmdStereoEnable(true); }
  if (std::strcmp(argv[0], "disable") == 0) { return CmdStereoEnable(false); }
  if (std::strcmp(argv[0], "is-enabled") == 0) { return CmdStereoIsEnabled(); }
  if (std::strcmp(argv[0], "windowed") == 0) { return CmdStereoWindowed(argc - 1, argv + 1); }
  if (std::strcmp(argv[0], "caps") == 0) { return CmdStereoCaps(argc - 1, argv + 1); }
  if (std::strcmp(argv[0], "info") == 0) { return CmdStereoInfo(argc - 1, argv + 1); }
  if (std::strcmp(argv[0], "app-info") == 0) { return CmdStereoAppInfo(argc - 1, argv + 1); }
  if (std::strcmp(argv[0], "mode-enum") == 0) { return CmdStereoModeEnum(argc - 1, argv + 1); }
  if (std::strcmp(argv[0], "accessory") == 0) { return CmdStereoAccessory(); }
  if (std::strcmp(argv[0], "dongle") == 0) { return CmdStereoDongle(argc - 1, argv + 1); }
  if (std::strcmp(argv[0], "aegis") == 0) { return CmdStereoAegis(argc - 1, argv + 1); }
  if (std::strcmp(argv[0], "default-profile") == 0) { return CmdStereoDefaultProfile(argc - 1, argv + 1); }
  if (std::strcmp(argv[0], "profile") == 0) { return CmdStereoProfile(argc - 1, argv + 1); }
  if (std::strcmp(argv[0], "driver-mode") == 0) { return CmdStereoDriverMode(argc - 1, argv + 1); }
  if (std::strcmp(argv[0], "activate") == 0 || std::strcmp(argv[0], "deactivate") == 0 ||
      std::strcmp(argv[0], "is-activated") == 0) {
    return CmdStereoActivation(argv[0]);
  }
  if (std::strcmp(argv[0], "separation") == 0) { return CmdStereoSeparation(argc - 1, argv + 1); }
  if (std::strcmp(argv[0], "convergence") == 0) { return CmdStereoConvergence(argc - 1, argv + 1); }
  if (std::strcmp(argv[0], "frustum") == 0) { return CmdStereoFrustum(argc - 1, argv + 1); }
  if (std::strcmp(argv[0], "capture") == 0) { return CmdStereoCapture(argc - 1, argv + 1); }
  if (std::strcmp(argv[0], "init-activation") == 0) { return CmdStereoInitActivation(argc - 1, argv + 1); }
  if (std::strcmp(argv[0], "trigger-activation") == 0) { return CmdStereoTriggerActivation(); }
  if (std::strcmp(argv[0], "reverse-blit") == 0) { return CmdStereoReverseBlit(argc - 1, argv + 1); }
  if (std::strcmp(argv[0], "notify") == 0) { return CmdStereoNotify(argc - 1, argv + 1); }
  if (std::strcmp(argv[0], "active-eye") == 0) { return CmdStereoActiveEye(argc - 1, argv + 1); }
  if (std::strcmp(argv[0], "eye-separation") == 0) { return CmdStereoEyeSeparation(); }
  if (std::strcmp(argv[0], "cursor") == 0) { return CmdStereoCursor(argc - 1, argv + 1); }
  if (std::strcmp(argv[0], "surface") == 0) { return CmdStereoSurface(argc - 1, argv + 1); }
  if (std::strcmp(argv[0], "debug") == 0) {
    if (argc > 1 && std::strcmp(argv[1], "last-draw") == 0) { return CmdStereoDebugLastDraw(); }
    std::printf("Unknown debug command.\n");
    return 1;
  }
  if (std::strcmp(argv[0], "force-to-screen") == 0) { return CmdStereoForceToScreen(argc - 1, argv + 1); }
  if (std::strcmp(argv[0], "video-control") == 0) { return CmdStereoVideoControl(argc - 1, argv + 1); }
  if (std::strcmp(argv[0], "video-metadata") == 0) { return CmdStereoVideoMetadata(argc - 1, argv + 1); }
  if (std::strcmp(argv[0], "handshake") == 0) { return CmdStereoHandshake(argc - 1, argv + 1); }
  if (std::strcmp(argv[0], "handshake-trigger") == 0) { return CmdStereoHandshakeTrigger(); }
  if (std::strcmp(argv[0], "handshake-message") == 0) { return CmdStereoHandshakeMessage(argc - 1, argv + 1); }
  if (std::strcmp(argv[0], "profile-name") == 0) { return CmdStereoProfileName(argc - 1, argv + 1); }
  if (std::strcmp(argv[0], "diag") == 0) { return CmdStereoDiag(); }
  if (std::strcmp(argv[0], "shader") == 0) { return CmdStereoShader(argc - 1, argv + 1); }

  std::printf("Unknown stereo command: %s\n", argv[0]);
  PrintStereoUsage();
  return 1;
}
} // namespace nvcli
