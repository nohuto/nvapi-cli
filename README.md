# nvapi-cli

CLI wrapper around NVIDIA's NVAPI for querying and controlling GPU, display, and driver features on Windows. NVAPI is NVIDIA's proprietary driver API that exposes GPU and display capabilities beyond the standard OS interfaces. It's hardware and driver dependent, many functions are supported only on specific GPUs, drivers, or product lines. Expect `NVAPI_NOT_SUPPORTED` for unsupported features.

Note that the documentation is partly parsed from official documentation partly rewritten by myself.

> [!CAUTION]
> Use the tool with caution when applying control APIs, I'm not responsible for any damage/issues. This tool is in BETA state, bugs may exist. I didn't test each option on my own yet.

[assets/supported_nvapi.txt](https://github.com/nohuto/nvapi-cli/blob/main/assets/supported_nvapi.txt) includes all NVAPI functions referenced by the current source code. [assets/unsupported_nvapi.txt](https://github.com/nohuto/nvapi-cli/blob/main/assets/unsupported_nvapi.txt) includes NVAPI functions present in `nvapi.h` but not used by the current version.

## Usage

Since showing all options by default would make it very confusing, it's splitted into groups. Use `nvapi-cli info` to print the NVAPI interface version and driver branch details.

```powershell
Usage:
  nvapi-cli help [group]
  nvapi-cli info
  nvapi-cli <group> <command> [options]
    groups: gpu display mosaic sli gsync drs video hdmi dp pcf sys d3d ogl vr stereo

Use "nvapi-cli help <group>" or "nvapi-cli <group> help" for details.
Use "nvapi-cli help all" for the full list.
```

## Documentation

Each group has a dedicated reference that lists commands, flags, and the underlying NVAPI calls.

- [docs/gpu.md](https://github.com/nohuto/nvapi-cli/blob/main/docs/gpu.md) - Clocks, Pstates, power, voltage, fans
- [docs/display.md](https://github.com/nohuto/nvapi-cli/blob/main/docs/display.md) - Display enumeration, EDID, timing, HDR, display path configuration
- [docs/mosaic.md](https://github.com/nohuto/nvapi-cli/blob/main/docs/mosaic.md) - Mosaic capabilities and topology control
- [docs/sli.md](https://github.com/nohuto/nvapi-cli/blob/main/docs/sli.md) - SLI status and supported views
- [docs/gsync.md](https://github.com/nohuto/nvapi-cli/blob/main/docs/gsync.md) - GSync devices, capabilities, and topology
- [docs/drs.md](https://github.com/nohuto/nvapi-cli/blob/main/docs/drs.md) - Driver profile management (DRS)
- [docs/video.md](https://github.com/nohuto/nvapi-cli/blob/main/docs/video.md) - Video color controls
- [docs/hdmi.md](https://github.com/nohuto/nvapi-cli/blob/main/docs/hdmi.md) - HDMI support, HDCP diagnostics, stereo, audio mute
- [docs/dp.md](https://github.com/nohuto/nvapi-cli/blob/main/docs/dp.md) - DisplayPort (DP) info, configuration, dongle, topology
- [docs/pcf.md](https://github.com/nohuto/nvapi-cli/blob/main/docs/pcf.md) - PCF master controls
- [docs/sys.md](https://github.com/nohuto/nvapi-cli/blob/main/docs/sys.md) - System/ACPI/feature/license APIs
- [docs/d3d.md](https://github.com/nohuto/nvapi-cli/blob/main/docs/d3d.md) - Direct3D toolchain helpers (VRR, Ansel...)
- [docs/ogl.md](https://github.com/nohuto/nvapi-cli/blob/main/docs/ogl.md) - OpenGL expert mode settings
- [docs/vr.md](https://github.com/nohuto/nvapi-cli/blob/main/docs/vr.md) - Direct mode display controls
- [docs/stereo.md](https://github.com/nohuto/nvapi-cli/blob/main/docs/stereo.md) - Stereo 3D and driver registry controls