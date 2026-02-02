/*
 * Copyright (c) 2026 Noverse (Nohuto). All rights reserved.
 * Proprietary and confidential. Unauthorized copying or redistribution is strictly prohibited.
 */

#include "cli/commands.h"

#include <GL/gl.h>

namespace nvcli {
namespace {
void PrintOglContextInfo() {
  const char *vendor = reinterpret_cast<const char *>(glGetString(GL_VENDOR));
  const char *renderer = reinterpret_cast<const char *>(glGetString(GL_RENDERER));
  const char *version = reinterpret_cast<const char *>(glGetString(GL_VERSION));
  std::printf("OpenGL vendor: %s\n", vendor ? vendor : "<null>");
  std::printf("OpenGL renderer: %s\n", renderer ? renderer : "<null>");
  std::printf("OpenGL version: %s\n", version ? version : "<null>");
}

struct OglContext {
  HWND window = nullptr;
  HDC dc = nullptr;
  HGLRC rc = nullptr;

  void Cleanup() {
    if (rc) {
      wglMakeCurrent(NULL, NULL);
      wglDeleteContext(rc);
      rc = nullptr;
    }
    if (window && dc) {
      ReleaseDC(window, dc);
      dc = nullptr;
    }
    if (window) {
      DestroyWindow(window);
      window = nullptr;
    }
  }
};

HWND CreateHiddenWindow() {
  static bool registered = false;
  static const char kClassName[] = "NvapiCliOglHidden";

  HINSTANCE instance = GetModuleHandle(NULL);
  if (!registered) {
    WNDCLASSEX wc = {};
    wc.cbSize = sizeof(wc);
    wc.lpfnWndProc = DefWindowProc;
    wc.hInstance = instance;
    wc.lpszClassName = kClassName;
    if (!RegisterClassEx(&wc)) { return NULL; }
    registered = true;
  }

  return CreateWindowEx(0, kClassName, "nvapi-cli", WS_OVERLAPPEDWINDOW, 0, 0, 64, 64, NULL, NULL, instance, NULL);
}

bool CreateOglContext(OglContext &ctx) {
  ctx.window = CreateHiddenWindow();
  if (!ctx.window) {
    std::printf("Failed to create hidden window.\n");
    return false;
  }

  ctx.dc = GetDC(ctx.window);
  if (!ctx.dc) {
    std::printf("Failed to get device context.\n");
    return false;
  }

  PIXELFORMATDESCRIPTOR pfd = {};
  pfd.nSize = sizeof(pfd);
  pfd.nVersion = 1;
  pfd.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
  pfd.iPixelType = PFD_TYPE_RGBA;
  pfd.cColorBits = 24;
  pfd.cDepthBits = 24;
  pfd.cStencilBits = 8;

  int pixelFormat = ChoosePixelFormat(ctx.dc, &pfd);
  if (pixelFormat == 0) {
    std::printf("ChoosePixelFormat failed.\n");
    return false;
  }
  if (!SetPixelFormat(ctx.dc, pixelFormat, &pfd)) {
    std::printf("SetPixelFormat failed.\n");
    return false;
  }

  ctx.rc = wglCreateContext(ctx.dc);
  if (!ctx.rc) {
    std::printf("wglCreateContext failed.\n");
    return false;
  }

  if (!wglMakeCurrent(ctx.dc, ctx.rc)) {
    std::printf("wglMakeCurrent failed.\n");
    return false;
  }

  return true;
}

void PrintOglUsage() {
  std::printf("OpenGL toolchain commands:\n");
  std::printf("  %s ogl expert get\n", kToolName);
  std::printf("  %s ogl expert set --detail MASK --report MASK --output MASK\n", kToolName);
  std::printf("  %s ogl expert defaults-get\n", kToolName);
  std::printf("  %s ogl expert defaults-set --detail MASK --report MASK --output MASK\n", kToolName);
}

int CmdOglExpert(int argc, char **argv) {
  if (argc < 1) {
    std::printf("Missing expert command.\n");
    return 1;
  }

  if (std::strcmp(argv[0], "get") == 0) {
    OglContext ctx;
    if (!CreateOglContext(ctx)) {
      ctx.Cleanup();
      return 1;
    }

    NvU32 detail = 0;
    NvU32 report = 0;
    NvU32 output = 0;
    NVAPI_OGLEXPERT_CALLBACK callback = nullptr;
    NvAPI_Status status = NvAPI_OGL_ExpertModeGet(&detail, &report, &output, &callback);
    if (status != NVAPI_OK) {
      PrintOglContextInfo();
      PrintNvapiError("NvAPI_OGL_ExpertModeGet failed", status);
      ctx.Cleanup();
      return 1;
    }

    std::printf("Expert detail mask: 0x%08X\n", detail);
    std::printf("Expert report mask: 0x%08X\n", report);
    std::printf("Expert output mask: 0x%08X\n", output);

    ctx.Cleanup();
    return 0;
  }

  if (std::strcmp(argv[0], "set") == 0) {
    NvU32 detail = 0;
    NvU32 report = 0;
    NvU32 output = 0;
    bool hasDetail = false;
    bool hasReport = false;
    bool hasOutput = false;

    for (int i = 1; i < argc; ++i) {
      if (std::strcmp(argv[i], "--detail") == 0) {
        if (i + 1 >= argc) {
          std::printf("Missing value for --detail\n");
          return 1;
        }
        if (!ParseUint(argv[i + 1], &detail)) {
          std::printf("Invalid detail mask: %s\n", argv[i + 1]);
          return 1;
        }
        hasDetail = true;
        ++i;
        continue;
      }
      if (std::strcmp(argv[i], "--report") == 0) {
        if (i + 1 >= argc) {
          std::printf("Missing value for --report\n");
          return 1;
        }
        if (!ParseUint(argv[i + 1], &report)) {
          std::printf("Invalid report mask: %s\n", argv[i + 1]);
          return 1;
        }
        hasReport = true;
        ++i;
        continue;
      }
      if (std::strcmp(argv[i], "--output") == 0) {
        if (i + 1 >= argc) {
          std::printf("Missing value for --output\n");
          return 1;
        }
        if (!ParseUint(argv[i + 1], &output)) {
          std::printf("Invalid output mask: %s\n", argv[i + 1]);
          return 1;
        }
        hasOutput = true;
        ++i;
        continue;
      }
      std::printf("Unknown option: %s\n", argv[i]);
      return 1;
    }

    if (!hasDetail || !hasReport || !hasOutput) {
      std::printf("Missing required masks (--detail, --report, --output).\n");
      return 1;
    }

    OglContext ctx;
    if (!CreateOglContext(ctx)) {
      ctx.Cleanup();
      return 1;
    }

    NvAPI_Status status = NvAPI_OGL_ExpertModeSet(detail, report, output, nullptr);
    if (status != NVAPI_OK) {
      PrintOglContextInfo();
      PrintNvapiError("NvAPI_OGL_ExpertModeSet failed", status);
      ctx.Cleanup();
      return 1;
    }

    std::printf("Expert mode updated.\n");
    ctx.Cleanup();
    return 0;
  }

  if (std::strcmp(argv[0], "defaults-get") == 0) {
    NvU32 detail = 0;
    NvU32 report = 0;
    NvU32 output = 0;
    NvAPI_Status status = NvAPI_OGL_ExpertModeDefaultsGet(&detail, &report, &output);
    if (status != NVAPI_OK) {
      PrintNvapiError("NvAPI_OGL_ExpertModeDefaultsGet failed", status);
      return 1;
    }
    std::printf("Default detail mask: 0x%08X\n", detail);
    std::printf("Default report mask: 0x%08X\n", report);
    std::printf("Default output mask: 0x%08X\n", output);
    return 0;
  }

  if (std::strcmp(argv[0], "defaults-set") == 0) {
    NvU32 detail = 0;
    NvU32 report = 0;
    NvU32 output = 0;
    bool hasDetail = false;
    bool hasReport = false;
    bool hasOutput = false;

    for (int i = 1; i < argc; ++i) {
      if (std::strcmp(argv[i], "--detail") == 0) {
        if (i + 1 >= argc) {
          std::printf("Missing value for --detail\n");
          return 1;
        }
        if (!ParseUint(argv[i + 1], &detail)) {
          std::printf("Invalid detail mask: %s\n", argv[i + 1]);
          return 1;
        }
        hasDetail = true;
        ++i;
        continue;
      }
      if (std::strcmp(argv[i], "--report") == 0) {
        if (i + 1 >= argc) {
          std::printf("Missing value for --report\n");
          return 1;
        }
        if (!ParseUint(argv[i + 1], &report)) {
          std::printf("Invalid report mask: %s\n", argv[i + 1]);
          return 1;
        }
        hasReport = true;
        ++i;
        continue;
      }
      if (std::strcmp(argv[i], "--output") == 0) {
        if (i + 1 >= argc) {
          std::printf("Missing value for --output\n");
          return 1;
        }
        if (!ParseUint(argv[i + 1], &output)) {
          std::printf("Invalid output mask: %s\n", argv[i + 1]);
          return 1;
        }
        hasOutput = true;
        ++i;
        continue;
      }
      std::printf("Unknown option: %s\n", argv[i]);
      return 1;
    }

    if (!hasDetail || !hasReport || !hasOutput) {
      std::printf("Missing required masks (--detail, --report, --output).\n");
      return 1;
    }

    NvAPI_Status status = NvAPI_OGL_ExpertModeDefaultsSet(detail, report, output);
    if (status != NVAPI_OK) {
      PrintNvapiError("NvAPI_OGL_ExpertModeDefaultsSet failed", status);
      return 1;
    }
    std::printf("Expert defaults updated.\n");
    return 0;
  }

  std::printf("Unknown expert command: %s\n", argv[0]);
  return 1;
}
} // namespace

int CmdOgl(int argc, char **argv) {
  static const SubcommandEntry kSubcommands[] = {
      {"expert", CmdOglExpert},
  };

  return DispatchSubcommand("ogl", argc, argv, kSubcommands, sizeof(kSubcommands) / sizeof(kSubcommands[0]),
                            PrintOglUsage);
}
} // namespace nvcli
