/*
 * Copyright (c) 2026 Noverse (Nohuto). All rights reserved.
 * Proprietary and confidential. Unauthorized copying or redistribution is strictly prohibited.
 */

#include <d3d11.h>
#include <dxgi.h>
#include <dxgi1_2.h>
#include <windows.h>

#include "cli/commands.h"

#include <vector>
#include <string>

namespace nvcli
{
    namespace
    {
        struct DebugLog
        {
            FILE *file = nullptr;
            ~DebugLog()
            {
                if (file)
                {
                    std::fclose(file);
                }
            }
            void Log(const char *msg)
            {
                if (file && msg)
                {
                    std::fprintf(file, "%s\n", msg);
                    std::fflush(file);
                }
            }
        };

        struct D3DContext
        {
            ID3D11Device *device = nullptr;
            ID3D11DeviceContext *context = nullptr;
            IDXGISwapChain *swapChain = nullptr;
            ID3D11Texture2D *backBuffer = nullptr;
            HWND window = nullptr;
            NVDX_ObjectHandle surfaceHandle = 0;

            void Cleanup()
            {
                if (backBuffer)
                {
                    backBuffer->Release();
                    backBuffer = nullptr;
                }
                if (swapChain)
                {
                    swapChain->Release();
                    swapChain = nullptr;
                }
                if (context)
                {
                    context->Release();
                    context = nullptr;
                }
                if (device)
                {
                    device->Release();
                    device = nullptr;
                }
                if (window)
                {
                    DestroyWindow(window);
                    window = nullptr;
                }
                surfaceHandle = 0;
            }
        };

        void PrintHresult(const char *prefix, HRESULT hr)
        {
            std::printf("%s: 0x%08X\n", prefix, static_cast<unsigned int>(hr));
        }

        HWND CreateHiddenWindow()
        {
            static bool registered = false;
            static const char kClassName[] = "NvapiCliHidden";

            HINSTANCE instance = GetModuleHandle(NULL);
            if (!registered)
            {
                WNDCLASSEX wc = {};
                wc.cbSize = sizeof(wc);
                wc.lpfnWndProc = DefWindowProc;
                wc.hInstance = instance;
                wc.lpszClassName = kClassName;
                if (!RegisterClassEx(&wc))
                {
                    return NULL;
                }
                registered = true;
            }

            return CreateWindowEx(0, kClassName, "nvapi-cli", WS_OVERLAPPEDWINDOW,
                0, 0, 64, 64, NULL, NULL, instance, NULL);
        }

        bool CreateD3D11Device(D3DContext &ctx)
        {
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
            if (FAILED(hr))
            {
                PrintHresult("CreateDXGIFactory1 failed", hr);
                return false;
            }

            IDXGIAdapter1 *adapter = nullptr;
            for (UINT i = 0; factory->EnumAdapters1(i, &adapter) == S_OK; ++i)
            {
                DXGI_ADAPTER_DESC1 desc = {};
                if (SUCCEEDED(adapter->GetDesc1(&desc)))
                {
                    if (desc.VendorId == 0x10DE && (desc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE) == 0)
                    {
                        break;
                    }
                }
                adapter->Release();
                adapter = nullptr;
            }
            factory->Release();

            if (!adapter)
            {
                std::printf("No NVIDIA adapter found for D3D device creation.\n");
                return false;
            }

            hr = D3D11CreateDevice(adapter, D3D_DRIVER_TYPE_UNKNOWN, NULL, flags,
                levels, ARRAYSIZE(levels), D3D11_SDK_VERSION,
                &ctx.device, &level, &ctx.context);
            adapter->Release();
            if (FAILED(hr))
            {
                PrintHresult("D3D11CreateDevice (NVIDIA adapter) failed", hr);
                return false;
            }
            return true;
        }

        bool GetFactoryFromDevice(D3DContext &ctx, IDXGIFactory **outFactory)
        {
            if (!outFactory)
            {
                return false;
            }
            *outFactory = nullptr;
            IDXGIDevice *dxgiDevice = nullptr;
            HRESULT hr = ctx.device->QueryInterface(__uuidof(IDXGIDevice), reinterpret_cast<void **>(&dxgiDevice));
            if (FAILED(hr))
            {
                PrintHresult("QueryInterface IDXGIDevice failed", hr);
                return false;
            }

            IDXGIAdapter *adapter = nullptr;
            hr = dxgiDevice->GetAdapter(&adapter);
            dxgiDevice->Release();
            if (FAILED(hr))
            {
                PrintHresult("GetAdapter failed", hr);
                return false;
            }

            hr = adapter->GetParent(__uuidof(IDXGIFactory), reinterpret_cast<void **>(outFactory));
            adapter->Release();
            if (FAILED(hr))
            {
                PrintHresult("GetParent IDXGIFactory failed", hr);
                return false;
            }

            return true;
        }

        bool CreateSwapChain(D3DContext &ctx, DebugLog *debugLog)
        {
            auto Log = [&](const char *msg)
            {
                if (debugLog)
                {
                    debugLog->Log(msg);
                }
            };

            ctx.window = CreateHiddenWindow();
            if (!ctx.window)
            {
                std::printf("Failed to create hidden window.\n");
                Log("Failed to create hidden window.");
                return false;
            }

            IDXGIFactory *factory = nullptr;
            if (!GetFactoryFromDevice(ctx, &factory))
            {
                return false;
            }

            IDXGIFactory2 *factory2 = nullptr;
            HRESULT hr = factory->QueryInterface(__uuidof(IDXGIFactory2), reinterpret_cast<void **>(&factory2));
            if (SUCCEEDED(hr) && factory2)
            {
                DXGI_SWAP_CHAIN_DESC1 desc1 = {};
                desc1.Width = 64;
                desc1.Height = 64;
                desc1.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
                desc1.SampleDesc.Count = 1;
                desc1.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
                desc1.BufferCount = 2;
                desc1.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
                desc1.AlphaMode = DXGI_ALPHA_MODE_IGNORE;
                desc1.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING;

                IDXGISwapChain1 *swapChain1 = nullptr;
                __try
                {
                    hr = factory2->CreateSwapChainForHwnd(ctx.device, ctx.window, &desc1, nullptr, nullptr, &swapChain1);
                }
                __except (EXCEPTION_EXECUTE_HANDLER)
                {
                    std::printf("CreateSwapChainForHwnd raised exception 0x%08X\n", GetExceptionCode());
                    char msg[96] = {};
                    std::snprintf(msg, sizeof(msg), "CreateSwapChainForHwnd exception 0x%08X", GetExceptionCode());
                    Log(msg);
                    factory2->Release();
                    factory->Release();
                    return false;
                }
                if (SUCCEEDED(hr) && swapChain1)
                {
                    ctx.swapChain = swapChain1;
                    swapChain1 = nullptr;
                }
                if (swapChain1)
                {
                    swapChain1->Release();
                }
                factory2->Release();
            }

            if (!ctx.swapChain)
            {
                DXGI_SWAP_CHAIN_DESC desc = {};
                desc.BufferCount = 2;
                desc.BufferDesc.Width = 64;
                desc.BufferDesc.Height = 64;
                desc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
                desc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
                desc.OutputWindow = ctx.window;
                desc.SampleDesc.Count = 1;
                desc.Windowed = TRUE;
                desc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;

                __try
                {
                    hr = factory->CreateSwapChain(ctx.device, &desc, &ctx.swapChain);
                }
                __except (EXCEPTION_EXECUTE_HANDLER)
                {
                    std::printf("CreateSwapChain raised exception 0x%08X\n", GetExceptionCode());
                    char msg[80] = {};
                    std::snprintf(msg, sizeof(msg), "CreateSwapChain exception 0x%08X", GetExceptionCode());
                    Log(msg);
                    factory->Release();
                    return false;
                }
            }
            factory->Release();
            if (FAILED(hr))
            {
                PrintHresult("CreateSwapChain failed", hr);
                Log("CreateSwapChain failed.");
                return false;
            }

            __try
            {
                hr = ctx.swapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), reinterpret_cast<void **>(&ctx.backBuffer));
            }
            __except (EXCEPTION_EXECUTE_HANDLER)
            {
                std::printf("GetBuffer raised exception 0x%08X\n", GetExceptionCode());
                char msg[72] = {};
                std::snprintf(msg, sizeof(msg), "GetBuffer exception 0x%08X", GetExceptionCode());
                Log(msg);
                return false;
            }
            if (FAILED(hr))
            {
                PrintHresult("GetBuffer failed", hr);
                Log("GetBuffer failed.");
                return false;
            }

            NvAPI_Status status = NVAPI_OK;
            __try
            {
                status = NvAPI_D3D_GetObjectHandleForResource(ctx.device, ctx.backBuffer, &ctx.surfaceHandle);
            }
            __except (EXCEPTION_EXECUTE_HANDLER)
            {
                std::printf("NvAPI_D3D_GetObjectHandleForResource raised exception 0x%08X\n", GetExceptionCode());
                return false;
            }
            if (status != NVAPI_OK)
            {
                PrintNvapiError("NvAPI_D3D_GetObjectHandleForResource failed", status);
                Log("NvAPI_D3D_GetObjectHandleForResource failed.");
                return false;
            }

            return true;
        }

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

        bool ParseBoolWord(const char *text, bool *out)
        {
            if (!text || !out)
            {
                return false;
            }
            if (std::strcmp(text, "on") == 0)
            {
                *out = true;
                return true;
            }
            if (std::strcmp(text, "off") == 0)
            {
                *out = false;
                return true;
            }
            return false;
        }

        bool ParseLatencyMarkerType(const char *text, NV_LATENCY_MARKER_TYPE *out)
        {
            if (!text || !out)
            {
                return false;
            }
            std::string value = ToLowerAscii(text);
            if (value == "simulation-start")
            {
                *out = SIMULATION_START;
                return true;
            }
            if (value == "simulation-end")
            {
                *out = SIMULATION_END;
                return true;
            }
            if (value == "rendersubmit-start")
            {
                *out = RENDERSUBMIT_START;
                return true;
            }
            if (value == "rendersubmit-end")
            {
                *out = RENDERSUBMIT_END;
                return true;
            }
            if (value == "present-start")
            {
                *out = PRESENT_START;
                return true;
            }
            if (value == "present-end")
            {
                *out = PRESENT_END;
                return true;
            }
            if (value == "input-sample")
            {
                *out = INPUT_SAMPLE;
                return true;
            }
            if (value == "trigger-flash")
            {
                *out = TRIGGER_FLASH;
                return true;
            }
            if (value == "pc-latency-ping")
            {
                *out = PC_LATENCY_PING;
                return true;
            }
            return false;
        }

        bool ParseAnselModifier(const char *text, NVAPI_ANSEL_HOTKEY_MODIFIER *out)
        {
            if (!text || !out)
            {
                return false;
            }
            std::string value = ToLowerAscii(text);
            if (value == "none")
            {
                *out = NVAPI_ANSEL_HOTKEY_MODIFIER_UNKNOWN;
                return true;
            }
            if (value == "ctrl")
            {
                *out = NVAPI_ANSEL_HOTKEY_MODIFIER_CTRL;
                return true;
            }
            if (value == "shift")
            {
                *out = NVAPI_ANSEL_HOTKEY_MODIFIER_SHIFT;
                return true;
            }
            if (value == "alt")
            {
                *out = NVAPI_ANSEL_HOTKEY_MODIFIER_ALT;
                return true;
            }
            return false;
        }

        bool ParseAnselFeatureSpec(const char *text, NVAPI_ANSEL_FEATURE_CONFIGURATION_STRUCT *out)
        {
            if (!text || !out)
            {
                return false;
            }

            std::string spec(text);
            size_t first = spec.find(':');
            if (first == std::string::npos)
            {
                return false;
            }
            size_t second = spec.find(':', first + 1);

            std::string feature = spec.substr(0, first);
            std::string state = second == std::string::npos ? spec.substr(first + 1) : spec.substr(first + 1, second - first - 1);
            std::string key = second == std::string::npos ? std::string() : spec.substr(second + 1);

            std::string featureLower = ToLowerAscii(feature.c_str());
            if (featureLower == "black-and-white")
            {
                out->featureId = NVAPI_ANSEL_FEATURE_BLACK_AND_WHITE;
            }
            else if (featureLower == "hudless")
            {
                out->featureId = NVAPI_ANSEL_FEATURE_HUDLESS;
            }
            else
            {
                return false;
            }

            std::string stateLower = ToLowerAscii(state.c_str());
            if (stateLower == "enable")
            {
                out->featureState = NVAPI_ANSEL_FEATURE_STATE_ENABLE;
            }
            else if (stateLower == "disable")
            {
                out->featureState = NVAPI_ANSEL_FEATURE_STATE_DISABLE;
            }
            else
            {
                return false;
            }

            out->hotkey = 0;
            if (!key.empty())
            {
                NvU32 hotkey = 0;
                if (!ParseUint(key.c_str(), &hotkey))
                {
                    return false;
                }
                out->hotkey = static_cast<UINT>(hotkey);
            }
            return true;
        }

        void DumpHex(const void *data, size_t size)
        {
            const unsigned char *bytes = static_cast<const unsigned char *>(data);
            for (size_t i = 0; i < size; ++i)
            {
                if (i % 16 == 0)
                {
                    std::printf("    ");
                }
                std::printf("%02X ", bytes[i]);
                if (i % 16 == 15 || i + 1 == size)
                {
                    std::printf("\n");
                }
            }
        }

        bool WriteBinaryFile(const char *path, const void *data, size_t size)
        {
            if (!path || !data || size == 0)
            {
                return false;
            }
            FILE *file = nullptr;
            if (fopen_s(&file, path, "wb") != 0 || !file)
            {
                std::printf("Failed to open output file: %s\n", path);
                return false;
            }
            size_t written = std::fwrite(data, 1, size, file);
            std::fclose(file);
            if (written != size)
            {
                std::printf("Failed to write output file: %s\n", path);
                return false;
            }
            return true;
        }

        bool SafeD3DSetVRRState(IUnknown *deviceOrContext, NVDX_ObjectHandle surfaceHandle, BOOL enable, NvAPI_Status *statusOut)
        {
            if (!statusOut)
            {
                return false;
            }
            NvAPI_Status status = NVAPI_ERROR;
            __try
            {
                status = NvAPI_D3D_SetVRRState(deviceOrContext, surfaceHandle, enable);
            }
            __except (EXCEPTION_EXECUTE_HANDLER)
            {
                std::printf("NvAPI_D3D_SetVRRState raised exception 0x%08X\n", GetExceptionCode());
                *statusOut = NVAPI_ERROR;
                return false;
            }
            *statusOut = status;
            return true;
        }

        bool SafeD3DGetVRRState(IUnknown *deviceOrContext, NVDX_ObjectHandle surfaceHandle, BOOL *isEnabled, BOOL *isRequested, NvAPI_Status *statusOut)
        {
            if (!statusOut)
            {
                return false;
            }
            NvAPI_Status status = NVAPI_ERROR;
            __try
            {
                status = NvAPI_D3D_GetVRRState(deviceOrContext, surfaceHandle, isEnabled, isRequested);
            }
            __except (EXCEPTION_EXECUTE_HANDLER)
            {
                std::printf("NvAPI_D3D_GetVRRState raised exception 0x%08X\n", GetExceptionCode());
                *statusOut = NVAPI_ERROR;
                return false;
            }
            *statusOut = status;
            return true;
        }

        void PrintD3dUsage()
        {
            std::printf("D3D toolchain commands:\n");
            std::printf("  %s d3d vrr get [--swapchain] [--surface HANDLE] [--present] [--debug]\n", kToolName);
            std::printf("  %s d3d vrr set --state on|off [--swapchain] [--surface HANDLE] [--present] [--debug]\n", kToolName);
            std::printf("  %s d3d latency get [--out PATH] [--raw]\n", kToolName);
            std::printf("  %s d3d latency mark --frame ID --type TYPE\n", kToolName);
            std::printf("  %s d3d ansel set [--modifier none|ctrl|shift|alt] [--enable-key VK] [--feature NAME:STATE[:VK]]\n", kToolName);
        }

        int CmdD3dVrr(int argc, char **argv)
        {
            if (argc < 1)
            {
                std::printf("Missing vrr command.\n");
                return 1;
            }

            bool doSet = false;
            bool createSwapchain = false;
            bool doPresent = false;
            bool hasSurface = false;
            bool enable = false;
            bool hasState = false;
            bool debug = false;
            NvU64 surfaceValue = 0;
            DebugLog debugLog;
            auto DebugPrint = [&](const char *msg)
            {
                if (debug)
                {
                    std::printf("%s\n", msg);
                    debugLog.Log(msg);
                }
            };

            if (std::strcmp(argv[0], "get") == 0)
            {
                doSet = false;
            }
            else if (std::strcmp(argv[0], "set") == 0)
            {
                doSet = true;
            }
            else
            {
                std::printf("Unknown vrr command: %s\n", argv[0]);
                return 1;
            }

            for (int i = 1; i < argc; ++i)
            {
                if (std::strcmp(argv[i], "--swapchain") == 0)
                {
                    createSwapchain = true;
                    continue;
                }
                if (std::strcmp(argv[i], "--present") == 0)
                {
                    doPresent = true;
                    continue;
                }
                if (std::strcmp(argv[i], "--debug") == 0)
                {
                    debug = true;
                    continue;
                }
                if (std::strcmp(argv[i], "--surface") == 0)
                {
                    if (i + 1 >= argc)
                    {
                        std::printf("Missing value for --surface\n");
                        return 1;
                    }
                    if (!ParseU64(argv[i + 1], &surfaceValue))
                    {
                        std::printf("Invalid surface handle: %s\n", argv[i + 1]);
                        return 1;
                    }
                    hasSurface = true;
                    ++i;
                    continue;
                }
                if (std::strcmp(argv[i], "--state") == 0)
                {
                    if (i + 1 >= argc)
                    {
                        std::printf("Missing value for --state\n");
                        return 1;
                    }
                    if (!ParseBoolWord(argv[i + 1], &enable))
                    {
                        std::printf("Invalid state: %s\n", argv[i + 1]);
                        return 1;
                    }
                    hasState = true;
                    ++i;
                    continue;
                }
                std::printf("Unknown option: %s\n", argv[i]);
                return 1;
            }

            if (debug)
            {
                if (fopen_s(&debugLog.file, "nvapi-cli_d3d_vrr.log", "a") == 0 && debugLog.file)
                {
                    debugLog.Log("----");
                }
            }

            if (doSet && !hasState)
            {
                std::printf("Missing required --state on|off for vrr set.\n");
                return 1;
            }

            if (!createSwapchain && !hasSurface)
            {
                std::printf("Missing --swapchain or --surface HANDLE.\n");
                return 1;
            }

            D3DContext ctx;
            DebugPrint("D3D VRR: creating D3D11 device");
            if (!CreateD3D11Device(ctx))
            {
                ctx.Cleanup();
                return 1;
            }
            DebugPrint("D3D VRR: device created");

            NVDX_ObjectHandle surfaceHandle = 0;
            if (createSwapchain)
            {
                DebugPrint("D3D VRR: creating swapchain");
                if (!CreateSwapChain(ctx, debug ? &debugLog : nullptr))
                {
                    ctx.Cleanup();
                    return 1;
                }
                DebugPrint("D3D VRR: swapchain created");
                surfaceHandle = ctx.surfaceHandle;
                if (!surfaceHandle)
                {
                    std::printf("Failed to obtain a valid surface handle.\n");
                    ctx.Cleanup();
                    return 1;
                }
                if (doPresent && ctx.swapChain)
                {
                    ctx.swapChain->Present(0, 0);
                    DebugPrint("D3D VRR: present issued");
                }
            }
            else if (hasSurface)
            {
                surfaceHandle = reinterpret_cast<NVDX_ObjectHandle>(static_cast<uintptr_t>(surfaceValue));
            }

            NvAPI_Status status = NVAPI_OK;
            IUnknown *deviceOrContext = ctx.context
                ? static_cast<IUnknown *>(ctx.context)
                : static_cast<IUnknown *>(ctx.device);
            if (doSet)
            {
                DebugPrint("D3D VRR: calling NvAPI_D3D_SetVRRState");
                if (!SafeD3DSetVRRState(deviceOrContext, surfaceHandle, enable ? TRUE : FALSE, &status))
                {
                    ctx.Cleanup();
                    return 1;
                }
                if (status == NVAPI_OK)
                {
                    std::printf("VRR state updated.\n");
                }
                else
                {
                    PrintNvapiError("NvAPI_D3D_SetVRRState failed", status);
                }
            }
            else
            {
                BOOL isEnabled = FALSE;
                BOOL isRequested = FALSE;
                DebugPrint("D3D VRR: calling NvAPI_D3D_GetVRRState");
                if (!SafeD3DGetVRRState(deviceOrContext, surfaceHandle, &isEnabled, &isRequested, &status))
                {
                    ctx.Cleanup();
                    return 1;
                }
                if (status == NVAPI_OK)
                {
                    std::printf("VRR enabled: %s\n", isEnabled ? "yes" : "no");
                    std::printf("VRR requested: %s\n", isRequested ? "yes" : "no");
                }
                else
                {
                    PrintNvapiError("NvAPI_D3D_GetVRRState failed", status);
                }
            }

            ctx.Cleanup();
            return status == NVAPI_OK ? 0 : 1;
        }

        int CmdD3dLatency(int argc, char **argv)
        {
            if (argc < 1)
            {
                std::printf("Missing latency command.\n");
                return 1;
            }

            bool raw = false;
            const char *outPath = nullptr;

            if (std::strcmp(argv[0], "get") == 0)
            {
                for (int i = 1; i < argc; ++i)
                {
                    if (std::strcmp(argv[i], "--out") == 0)
                    {
                        if (i + 1 >= argc)
                        {
                            std::printf("Missing value for --out\n");
                            return 1;
                        }
                        outPath = argv[i + 1];
                        ++i;
                        continue;
                    }
                    if (std::strcmp(argv[i], "--raw") == 0)
                    {
                        raw = true;
                        continue;
                    }
                    std::printf("Unknown option: %s\n", argv[i]);
                    return 1;
                }

                D3DContext ctx;
                if (!CreateD3D11Device(ctx))
                {
                    ctx.Cleanup();
                    return 1;
                }

                NV_LATENCY_RESULT_PARAMS params = {};
                params.version = NV_LATENCY_RESULT_PARAMS_VER;
                NvAPI_Status status = NvAPI_D3D_GetLatency(ctx.device, &params);
                if (status != NVAPI_OK)
                {
                    PrintNvapiError("NvAPI_D3D_GetLatency failed", status);
                    ctx.Cleanup();
                    return 1;
                }

                int latestIndex = -1;
                for (int i = 63; i >= 0; --i)
                {
                    if (params.frameReport[i].frameID != 0)
                    {
                        latestIndex = i;
                        break;
                    }
                }

                if (latestIndex >= 0)
                {
                    const auto &report = params.frameReport[latestIndex];
                    std::printf("Latency frame: %llu\n", static_cast<unsigned long long>(report.frameID));
                    std::printf("  gpuActiveRenderTimeUs=%u\n", report.gpuActiveRenderTimeUs);
                    std::printf("  gpuFrameTimeUs=%u\n", report.gpuFrameTimeUs);
                }
                else
                {
                    std::printf("No latency frames recorded.\n");
                }

                if (outPath)
                {
                    if (!WriteBinaryFile(outPath, &params, sizeof(params)))
                    {
                        ctx.Cleanup();
                        return 1;
                    }
                    std::printf("Wrote output: %s\n", outPath);
                }

                if (raw)
                {
                    DumpHex(&params, sizeof(params));
                }

                ctx.Cleanup();
                return 0;
            }

            if (std::strcmp(argv[0], "mark") == 0)
            {
                NvU64 frameId = 0;
                bool hasFrame = false;
                NV_LATENCY_MARKER_TYPE marker = SIMULATION_START;
                bool hasType = false;

                for (int i = 1; i < argc; ++i)
                {
                    if (std::strcmp(argv[i], "--frame") == 0)
                    {
                        if (i + 1 >= argc)
                        {
                            std::printf("Missing value for --frame\n");
                            return 1;
                        }
                        if (!ParseU64(argv[i + 1], &frameId))
                        {
                            std::printf("Invalid frame id: %s\n", argv[i + 1]);
                            return 1;
                        }
                        hasFrame = true;
                        ++i;
                        continue;
                    }
                    if (std::strcmp(argv[i], "--type") == 0)
                    {
                        if (i + 1 >= argc)
                        {
                            std::printf("Missing value for --type\n");
                            return 1;
                        }
                        if (!ParseLatencyMarkerType(argv[i + 1], &marker))
                        {
                            std::printf("Invalid marker type: %s\n", argv[i + 1]);
                            return 1;
                        }
                        hasType = true;
                        ++i;
                        continue;
                    }
                    std::printf("Unknown option: %s\n", argv[i]);
                    return 1;
                }

                if (!hasFrame || !hasType)
                {
                    std::printf("Missing required --frame and/or --type\n");
                    return 1;
                }

                D3DContext ctx;
                if (!CreateD3D11Device(ctx))
                {
                    ctx.Cleanup();
                    return 1;
                }

                NV_LATENCY_MARKER_PARAMS params = {};
                params.version = NV_LATENCY_MARKER_PARAMS_VER;
                params.frameID = frameId;
                params.markerType = marker;

                NvAPI_Status status = NvAPI_D3D_SetLatencyMarker(ctx.device, &params);
                if (status != NVAPI_OK)
                {
                    PrintNvapiError("NvAPI_D3D_SetLatencyMarker failed", status);
                    ctx.Cleanup();
                    return 1;
                }

                std::printf("Latency marker set.\n");
                ctx.Cleanup();
                return 0;
            }

            std::printf("Unknown latency command: %s\n", argv[0]);
            return 1;
        }

        int CmdD3dAnsel(int argc, char **argv)
        {
            if (argc < 1)
            {
                std::printf("Missing ansel command.\n");
                return 1;
            }

            if (std::strcmp(argv[0], "set") != 0)
            {
                std::printf("Unknown ansel command: %s\n", argv[0]);
                return 1;
            }

            NVAPI_ANSEL_HOTKEY_MODIFIER modifier = NVAPI_ANSEL_HOTKEY_MODIFIER_UNKNOWN;
            NvU32 enableKey = 0;
            bool hasEnableKey = false;
            std::vector<NVAPI_ANSEL_FEATURE_CONFIGURATION_STRUCT> features;

            for (int i = 1; i < argc; ++i)
            {
                if (std::strcmp(argv[i], "--modifier") == 0)
                {
                    if (i + 1 >= argc)
                    {
                        std::printf("Missing value for --modifier\n");
                        return 1;
                    }
                    if (!ParseAnselModifier(argv[i + 1], &modifier))
                    {
                        std::printf("Invalid modifier: %s\n", argv[i + 1]);
                        return 1;
                    }
                    ++i;
                    continue;
                }
                if (std::strcmp(argv[i], "--enable-key") == 0)
                {
                    if (i + 1 >= argc)
                    {
                        std::printf("Missing value for --enable-key\n");
                        return 1;
                    }
                    if (!ParseUint(argv[i + 1], &enableKey))
                    {
                        std::printf("Invalid enable key: %s\n", argv[i + 1]);
                        return 1;
                    }
                    hasEnableKey = true;
                    ++i;
                    continue;
                }
                if (std::strcmp(argv[i], "--feature") == 0)
                {
                    if (i + 1 >= argc)
                    {
                        std::printf("Missing value for --feature\n");
                        return 1;
                    }
                    NVAPI_ANSEL_FEATURE_CONFIGURATION_STRUCT feature = {};
                    if (!ParseAnselFeatureSpec(argv[i + 1], &feature))
                    {
                        std::printf("Invalid feature spec: %s\n", argv[i + 1]);
                        return 1;
                    }
                    features.push_back(feature);
                    ++i;
                    continue;
                }
                std::printf("Unknown option: %s\n", argv[i]);
                return 1;
            }

            D3DContext ctx;
            if (!CreateD3D11Device(ctx))
            {
                ctx.Cleanup();
                return 1;
            }

            NVAPI_ANSEL_CONFIGURATION_STRUCT config = {};
            config.version = NVAPI_ANSEL_CONFIGURATION_STRUCT_VER;
            config.hotkeyModifier = modifier;
            config.keyEnable = hasEnableKey ? static_cast<UINT>(enableKey) : 0;
            config.numAnselFeatures = static_cast<UINT>(features.size());
            config.pAnselFeatures = features.empty() ? nullptr : features.data();

            NvAPI_Status status = NvAPI_D3D_ConfigureAnsel(ctx.device, &config);
            if (status != NVAPI_OK)
            {
                PrintNvapiError("NvAPI_D3D_ConfigureAnsel failed", status);
                ctx.Cleanup();
                return 1;
            }

            std::printf("Ansel configuration updated.\n");
            ctx.Cleanup();
            return 0;
        }
    }

    int CmdD3d(int argc, char **argv)
    {
        static const SubcommandEntry kSubcommands[] = {
            {"vrr", CmdD3dVrr},
            {"latency", CmdD3dLatency},
            {"ansel", CmdD3dAnsel},
        };

        return DispatchSubcommand("d3d", argc, argv, kSubcommands, sizeof(kSubcommands) / sizeof(kSubcommands[0]), PrintD3dUsage);
    }
}

