# GDI STM Firmware

Firmware for the GDI-STM board (STM32G474, FreeRTOS, CAN/FDCAN, Lua).

## Features

- Injector control (current schematic: **8 channels**)
- CAN bus (FDCAN1)
- Lua scripting hooks
- FreeRTOS tasking
- Watchdog + basic health monitoring

## Quick Start (Windows)

1. Install **CMake** and **ARM GNU Toolchain (arm-none-eabi)**.
2. Build:

```bat
scripts\build_firmware.bat
```

Outputs are written to `build\` (notably `GDI_STM_Firmware.hex` / `.bin`).

## Releases And Firmware Assets

Release binaries are distributed as **GitLab Release assets** attached to tags (not committed in the repo).

### Automated (CI/CD)

When you push a tag, GitLab CI automatically:

1. Builds the firmware on Linux (CMake + arm-none-eabi-gcc)
2. Runs pinmap verification
3. Creates a GitLab Release and attaches `.hex`, `.bin`, `.map`, and `SHA256SUMS.txt` as assets

To create a release:

```bash
git tag v0.1.1
git push origin v0.1.1
```

The release appears under **Deployments → Releases** once the pipeline finishes.

### Manual (Windows)

For manual builds with a custom profile:

1. Build with the intended profile (example VBAT-only profile):

```bat
set GDI_CMAKE_EXTRA_ARGS=-DGDI_PROFILE_VBAT_ONLY_4CYL_HPFP_BANK4=ON
scripts\build_firmware.bat
```

2. Package upload files for a tag:

```bat
scripts\package_release_assets.bat v0.1.1
```

This creates `release\v0.1.1\` with:
- `GDI_STM_Firmware.hex`
- `GDI_STM_Firmware.bin`
- `GDI_STM_Firmware.map`
- `SHA256SUMS.txt`

3. Create/publish the GitLab Release for that tag and upload those files as assets.

`v0.1.0` remains available in git history as a legacy in-repo artifact snapshot.

## Pin Map

See `PINMAP.md` for PCB-derived net-to-MCU mapping.
See `WIRING_GUIDE.md` for harness wiring (ECU inputs, injector/HPFP outputs, CAN, power, SWD).

The firmware build runs a static cross-check against the produced PCB (`GDI-STM.kicad_pcb`):

```bat
python scripts\verify_pcb_vs_firmware.py
```

## Notes

- Host-side unit test coverage is not wired end-to-end in this repo/environment (no host C/C++ compiler detected). The current automated check is PCB-vs-firmware pinmap verification.
- Always validate pin behavior and timings on real hardware before enabling injector outputs in a vehicle.
