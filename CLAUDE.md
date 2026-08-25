# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## What this repo is

rusEFI hardware designs. Each top-level directory is an independent board project — a KiCad project (`.kicad_pro`/`.kicad_sch`/`.kicad_pcb`) plus supporting files: `connectors/*.yaml` pinout definitions, `gerber/` or `export/` or `production/` manufacturing outputs, and for a few boards a `firmware/` directory. There is no single top-level build; you build the specific firmware project you are working on. Main rusEFI firmware lives in the separate rusefi/rusefi repo.

## Session reporting & knowledge capture

After each completed unit of work (a landed feature, a fixed bug, or a finished investigation), and at minimum once per working session:

1. **Append** a dated entry to `docs/report.md` — never rewrite or reorder earlier entries. Cover: what was done, key decisions and why, validation performed (tests run, hardware checks), and open follow-ups. Match the file's existing style: plain ASCII, `-`/`->` instead of dashes/arrows, tables for change inventories.
2. **Fold durable, non-obvious knowledge into this CLAUDE.md**: build/tooling quirks, hardware protocols, architecture invariants, recurring debugging root-causes. Skip anything derivable from the code or git history — CLAUDE.md records what the code cannot say.

## Submodules

Clone/update with `git submodule update --init --recursive`. Firmware builds fail without them:
- `ChibiOS` — rusEFI fork, branch `stable_20.3.x.rusefi`; RTOS for the Makefile-based firmware projects
- `ext/libfirmware` — shared rusEFI firmware library (referenced as `RUSEFI_LIB` in Makefiles)
- `GDI-STM/firmware/lib/FreeRTOS` and `GDI-STM/firmware/lib/STM32CubeG4` — GDI-STM only; these have their own nested submodules (`FreeRTOS/Source`, HAL/CMSIS drivers) that also need init (see `.github/workflows/build-gdi-stm.yaml` for the exact minimal set)
- `kicad-libraries`, `ext/kicad6-libraries` — shared KiCad symbol/footprint libraries
- `ext/hellen-one` — board export/manufacturing scripts; `ext/googletest`, `ext/wideband`, `ext/openblt` — used by specific projects

## Firmware builds

Toolchain: `arm-none-eabi-gcc` (CI pins 12.3.Rel1) and GNU make.

ChibiOS Makefile projects (STM32F103, output in `build/`):
```
make -C GDI-4ch/firmware -j4        # builds build/gdi4.hex etc.
make -C GDI-6ch/firmware -j4
make -C digital-inputs/firmware     # HW quality-control board
```

GDI-STM (STM32G4, CMake + FreeRTOS + STM32CubeG4):
```
cd GDI-STM/firmware
cmake -B build -DCMAKE_TOOLCHAIN_FILE=arm-gcc-toolchain.cmake
cmake --build build -j
```

Flashing: each Makefile firmware dir has `flash.bat` / `erase.bat` wrapping `st-link_cli` (SWD).

### Windows: build through pixi

Bare `make` in PowerShell/cmd fails ("'sed' is not recognized") because the Makefiles shell out to a Unix userland. `pixi.toml` at the repo root provides make, arm-none-eabi-gcc, and the msys2 userland:
```
pixi install                              # one-time
pixi shell                                # everything on PATH, then: make -C GDI-4ch/firmware -j12
pixi run make -C GDI-4ch/firmware -j12    # one-off
```
Note: the `pixi run build-fw` / `pixi run test` tasks in `pixi.toml` reference `firmware/` and `unit_tests/` directories from the main rusefi repo that do not exist here — use `make -C <project>/firmware` instead.

## digital-inputs QC tester (Nucleo-144 F429ZI)

- LED semantics: blue (LD2) is a dedicated alive-blinker thread and must always
  blink at 10Hz; green/red latch the last test verdict. RED with blue *frozen*
  means the firmware itself crashed (hard fault / halt) - there is no watchdog
  and the ChibiOS halt hook is empty, so it stays frozen until power cycle.
- All CH_DBG_* checks are disabled in `cfg/chconf.h`; stack overflows corrupt
  silently. A 512-byte THREAD_STACK once caused exactly the frozen-blue symptom
  on error-heavy boards (chvprintf with %f is stack-hungry) - now 2048.
- `currentBoard` is nullptr until a known board ID arrives over CAN and is
  re-nulled by `startNewCanTest()` every cycle. Dereferencing it while null
  does NOT fault on STM32 (address 0 aliases flash) - it reads garbage, which
  once produced a wild ADC index and a hard fault. Guard every use.
- Console output (`chp`) goes through a non-blocking wrapper
  (`getNonBlockingConsole()` in `source/usbconsole.cpp`) that drops output when
  no USB host is draining SDU1. Never point `chp` back at SDU1 directly:
  chprintf blocks forever once the queue fills, and the tester must run
  standalone on +12v with no USB.

## Connector pinouts

`*/connectors/*.yaml` files are consumed by the interactive-pinout CI job (`.github/workflows/gen-pinouts.yaml`, see https://github.com/rusefi/rusefi/wiki/Connector-Mapping). Each entry has `pin`, `function`, and optionally `ts_name`, `class`, `type`, `color`. The workflow runs with `warnings: error` — duplicate pins or pins missing from the diagram fail the build. `type` values must come from the color map in that workflow (`12v`, `5v`, `gnd`, `ls`, `hs`, `ign`, `inj`, `din`, `av`, `at`, `can`, `vr`, `hall`, ...).

## CI notes

- `build-firmware.yaml` (every push/PR): builds GDI-4ch, GDI-6ch, and digital-inputs firmware.
- `build-gdi-stm.yaml`: builds GDI-STM when its files change.
- `build-unit-tests.yaml` is stale — it references `SENT-box/unit_tests/`, which no longer exists in this repo (manual dispatch only).
- `create-board.yaml`: regenerates gerbers via `ext/hellen-one/kicad/bin/export.sh` under KiCad 7 (manual dispatch).