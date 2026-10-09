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

## KiCad stacked ground pins

- Hidden power-input pins can create implicit global nets from their pin names. When reusing an ADC symbol with a hidden GND exposed pad on a GNDA circuit, check the exported netlist for an unintended GND/GNDA merge. A hidden passive stacked EP plus a visible power-input ground pin preserves physical pad connectivity without adding a global net.

## KiCad / Freerouting round trips

- KiCad DSN export writes existing unlocked copper as `(type route)`. Freerouting 2.4.1 interprets this token as USER_FIXED, not movable copper. For a working DSN intended for rip-up and reroute, use `(type normal)` on the copper that may move; retain `(type fix)` on deliberate fixed connections. Merely clearing KiCad locks does not enable rerouting of imported traces.
- SES import can round component positions. Preserve the original footprint and mechanical drawing nodes, merge only the generated routing, then verify native KiCad DRC and schematic parity. Router completion and router clearance counts do not replace these checks.

- Current KiCad 10 PCB files use name-based `(net "name")` nodes, including pads and tracks; do not assume top-level numeric net definitions. XML netlist export can unescape `{slash}` in a net name. Compare canonical names before treating that spelling difference as a schematic connectivity change.

- When reassigning a standalone stitching via to another net, mark it free (SetIsFree(True)) and clear existing zone fills before saving. Otherwise KiCad 10 can infer its old net from stale filled copper on reload. Refill and verify the saved via net, native DRC and schematic parity afterward; a SetNetCode call alone is not sufficient.

- In the KiCad 10 Python API, the legacy FootprintSave wrapper may select no plugin and fail with a NoneType error. PCB_IO_KICAD_SEXPR().FootprintSave(library_path, placed_footprint) works and normalizes the exported footprint to library coordinates without moving the placed board item. Check the exported copy with native library-mismatch DRC; remove board-only sheet metadata from the library file.

- For color-coded schematic bus fan-outs, keep every entry endpoint on the connection grid and off neighboring buses. A 2.54 mm entry with 2.54 mm parallel-bus spacing can land its wire endpoint on the adjacent bus and produce dangling-entry errors. Opposing fan-outs on the same Y also need a gap between their wire ends to avoid merging differently named nets. Use native ERC plus exact exported netlist comparison; do not suppress these checks.

- ERC can pass while a schematic bus-entry endpoint visually coincides with an unrelated through-wire. Audit the full diagonal against foreign wire segments and other entries, not just netlist connectivity. At dense opposing fan-outs, shorter 45-degree entries and a short colored bus branch can preserve signal separation; trim terminal bus spans to the revised attachment points.

- For the user's Hellen schematic style, prefer the existing GM E38/E67/Hyundai technique: retain 2.54 mm diagonal bus entries and offset a conflicting entry by 1.27 mm with a short orthogonal wire jog. Check the entire jog as well as the diagonal; a bend must not land on another bus, and a vertical wire segment must not overlap a bus. Named brace-only bus aliases such as {LS_CTRL} can join short control-bus sections without adding a net-name prefix; verify the exported netlist after using them.

- KiCad XML netlist exports may reorder the space-separated unit UUIDs in a multi-unit component's tstamps element. Compare that field as a set (or sort the tokens) before treating an otherwise identical component export as a change.

- For partial power-net rerouting, exporting zone-free DSN while retaining every other electrical net can make the router attempt to connect ground islands that are connected only by pours. A temporary DSN containing only the movable nets, with remaining pad/track copper represented as physical obstacles, avoids these false routing jobs. Merge only intended copper back onto the original board, preserve original footprints, correct imported via drill sizes, and validate native DRC after refilling all zones.
- Widening rails can split filled ground polygons and strand old stitching vias. Refill before evaluating connectivity, remove truly unused vias, restore plane connections, and recheck after rerouting obstructing signals. Geometric polygon connectivity and router completion do not replace native KiCad connectivity checks. Use a saved candidate and source hashes to preserve concurrent schematic/project edits.
- Track simplification can cut a narrow foreign-net ground neck even when every track clearance and same-net anchor is preserved. Refill and run native connectivity checks after smoothing; moving the neighboring trace back toward its original corridor can preserve the plane without adding vias. Python effective pad polygons approximate round copper, so use a small clearance margin and verify the exact native DRC afterward.

- When editing a temporary PCB with the KiCad Python API, keep the matching .kicad_pro (same basename) beside it before LoadBoard. Loading a renamed standalone PCB and saving it over a candidate can propagate default project rules into that candidate. Copy the source PCB into the prepared project directory before loading, then verify the project settings and native DRC.

- A zero-error native DRC result covers only enabled categories. For order-readiness reviews, inspect rule_severities and run an isolated audit with ignored categories enabled; keep those audit settings out of the production project. In particular, ignored starved_thermal and missing_courtyard checks can conceal incomplete plane spokes and limit mechanical collision coverage even when electrical connectivity passes.

- KiCad footprint reference fields can draw at a different angle from their stored text angle because of KeepUpright. When normalizing orientation, verify GetDrawRotation and the mirrored back-side rendering. A stored 270-degree back-side field with KeepUpright enabled can still draw at 90 degrees; disable that policy for the affected vertical fields and preserve their rendered center and font settings.

- Manufacturing export: the legacy Hellen BOM helper checks MyComment=DNP but does not by itself honor all native KiCad DNP/exclude-from-BOM flags. Filter against the saved board flags and verify excluded references. Hellen 0603 footprints can have SMD pads without the footprint-level SMD attribute, while QFN thermal-via footprints mix SMD and PTH pads; neither --smd-only nor --exclude-fp-th alone is a complete assembly selection. Export native positions first, then select fitted references explicitly.

- Native KiCad CSV placement headers (Ref, PosX, PosY, Rot, Side) are not the JLCPCB CPL schema. For a JLCPCB order handoff, export Designator, Mid X, Mid Y, Rotation, Layer, use mm and Top/Bottom, and validate references against the fitted BOM. Local CSV parsing alone does not prove acceptance by the assembly uploader.

- In the KiCad 10 Python API, attach newly loaded footprints to their board before calling Flip; flipping a parentless loaded footprint can crash the native process. For padless library logos, preserve the original footprint type when adding board-only/BOM/position exclusion flags. Replacing the type with only exclusion flags causes an avoidable library-mismatch warning.

- KiCad 10 Python footprint child removal: FOOTPRINT.Remove also changes SWIG ownership and can invalidate wrappers when removing multiple loaded children. RemoveNative avoids that ownership hand-off for batch removal. New pads reassigned to another physical footprint should get fresh UUIDs while the originals are still registered on the board, or duplicate-UUID assertions can block the native process. Verify saved pad geometry, thermal settings and fabrication output afterward.
- MoveAnchorPosition shifts footprint children and model offsets relative to the anchor but does not translate the footprint position. To centre an assembly anchor without moving real pads, apply the negative local centroid offset, then set the footprint position to the desired absolute centroid. Check native position export and 3D model placement after save/reload.

- Assembly-only parts can use a pinless Mechanical:Mechanical_Shape schematic instance with BOM/POS enabled and a padless placed footprint. KiCad 10 native BOM and placement exports retain these references; their electrical pads can remain owned by a separate, normally linked module footprint. Exclude that later-installed module only from BOM/POS, not from board transfer. Verify exact drill/pad geometry and use a disposable wrong-net negative control to prove native schematic parity audits the physical pad owner; a zero-error result with that owner excluded from board transfer does not demonstrate direct synchronization.

- KiCad F8 association compares a footprint's complete path with the exported component sheetpath plus a symbol-unit UUID. On the root sheet, this is /symbol-UUID, not /root-schematic-UUID/symbol-UUID (schematic instance paths use a different convention). Native schematic-parity DRC can pass despite a wrong prefix; audit exact full paths and uniqueness against a fresh netlist. Wrong prefixes can make F8 add normal library footprints and report the existing locked instances as unused. Preserve the intended instance and fix its association rather than unlocking/deleting it or suppressing missing-pin warnings.
