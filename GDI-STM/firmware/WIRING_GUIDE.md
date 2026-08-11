# GDI-STM Wiring Guide (Produced PCB)

This guide covers wiring for **bench testing** and **ECU/vehicle** use. It is based on the produced PCB netlist (`GDI-STM.kicad_pcb`) and firmware pinmap (`PINMAP.md`).

## Safety notes

- **Do not connect to a vehicle** until you have validated waveforms on a bench supply with a dummy load.
- By default the firmware keeps all high-side driver inputs **disabled** (`HS_VBATx` / `HS_VBOOSTx` LOW). If you enable them, the board can energize outputs.
- Use an inline fuse (e.g. 3–5 A) and current-limited bench supply for bring-up.

---

## Connectors overview

| Connector | Type | Purpose |
|-----------|------|---------|
| `P1` | TE_368255-2 (harness) | ECU inputs, injector/pump outputs, CAN, **power and ground** |
| `J5` | 1×05 header | **Power header** for bench or alternate supply |
| `J6` | Tag-Connect TC2030 | SWD programming/debug |
| `J7` | 1×06 header | Alternate SWD |

---

## J6 (TC2030) → ST-Link v2 / rusEFI SWD mapping

J6 is a Tag-Connect TC2030 pogo-pin header for SWD. Use labels **SWD: R, D, G, C, V** (as on rusEFI boards):

| Label | J6 pin | Net | ST-Link v2 (20-pin IDC) |
|-------|--------|-----|--------------------------|
| V | 1 | +3V3 | Pin 1 — **do not connect if board is powered** |
| D | 2 | SWDIO | Pin 2 |
| R | 3 | nRESET | Pin 7 |
| C | 4 | SWCLK | Pin 4 |
| G | 5 | GND | Pin 3, 5, 6, 8, or 10 |

**To flash** (board powered separately): connect **D, G, C** only.

**J7** (1×06 header): Alternate SWD (2.54 mm). Order: 1=+3V3 (V), 2=C (SWCLK), 3=G (GND), 4=D (SWDIO), 5=R (nRESET), 6=NC.

---

## Power and ground — where to connect

The board has **two power entry points**:

### Option A: J5 header (recommended for bench testing)

| Pin | Net | Connect to |
|-----|-----|------------|
| J5-1 | Vdrive | Driver/boost rail. For VBAT-only: **jumper to J5-2** |
| J5-2 | +BATT | 12 V supply (battery or bench) |
| J5-5 | GND | Supply ground |
| J5-3 | +5V | *(output)* On-board 5 V (do not feed) |
| J5-4 | +3V3 | *(output)* MCU 3.3 V (do not feed) |

**Minimum for bench:** Connect 12 V to J5-2, GND to J5-5, and jumper J5-1 to J5-2 for VBAT-only.

### Option B: P1 harness (vehicle or harness-based bench)

| P1 pad | Net | Purpose |
|--------|-----|---------|
| P1-4 | +12V_C | 12 V supply from harness |
| P1-1, P1-2 | GND | Supply ground |

If using the harness for power, connect P1-4 to 12 V and P1-1/P1-2 to ground. J5 and P1 share the same internal rails; use whichever is convenient.

---

## Bench testing — step-by-step

### Minimum connections

1. **Power and ground**
   - J5-2 ← 12 V (bench supply +)
   - J5-5 ← GND (bench supply −)
   - Jumper J5-1 to J5-2 (Vdrive = battery for VBAT-only)

2. **Load (single injector or dummy)**
   - Between `H1` (P1-118 or P1-121) and `L1` (P1-114)

3. **Command input**
   - Drive P1-44 (`/IN_INJ1`) with 5–12 V logic pulses relative to board GND
   - Start with 5–20 Hz, ~1–2 ms pulse width

### ECU connection (shared ground)

If you drive the command input from an ECU or other device:

- **Connect ECU ground to board ground** (e.g. P1-1 or P1-2, or J5-5).
- All logic signals (`/IN_INJ1` etc.) are referenced to this common ground.
- Use a wire from ECU GND to P1-1 or J5-5 so both share the same reference.

---

## ECU → Board (injector/pump command inputs)

Logic-level inputs from the ECU (rusEFI injector/pump outputs):

| ECU signal | Board net | P1 pad |
|------------|-----------|--------|
| INJ1 | `/IN_INJ1` | 44 |
| INJ2 | `/IN_INJ2` | 48 |
| INJ3 | `/IN_INJ3` | 46 |
| INJ4 | `/IN_INJ4` | 49 |
| INJ5 | `/IN_INJ5` | 45 |
| INJ6 | `/IN_INJ6` | 47 |
| HPFP/Pump cmd A | `/IN_PUMP2` | 40 |
| HPFP/Pump cmd B | `/IN_PUMP1` | 41 |

**Shared ground required:** ECU ground must be tied to board ground (P1-1, P1-2, or J5-5) for these signals to work.

---

## Board → Injector outputs (power stage)

Each load connects **between one `H*` and one `L*`** on P1.

### 4-cylinder injectors (INJ1..INJ4)

| Cylinder | ECU→Board input | Injector between |
|----------|------------------|-------------------|
| 1 | P1-44 `/IN_INJ1` | H1 and L1 |
| 2 | P1-48 `/IN_INJ2` | H1 and L2 |
| 3 | P1-46 `/IN_INJ3` | H3 and L3 |
| 4 | P1-49 `/IN_INJ4` | H3 and L4 |

P1 pads:
- H1: P1-118 or P1-121 | L1: P1-114 | L2: P1-119
- H3: P1-94/95/96/97 | L3: P1-90 or P1-91 | L4: P1-92 or P1-93

### Optional channels (INJ5..INJ8 / HPFP)

- Bank 3: H5 (P1-116/120), L5 (P1-117), L6 (P1-115)
- Bank 4: H7 (P1-82/83/86/87), L7 (P1-84/85), L8 (P1-88/89)

HPFP example: `/IN_PUMP2` (P1-40) → load between H7 and L7.

---

## CAN wiring (ECU ↔ board)

| P1 pad | Net |
|--------|-----|
| P1-52 | CAN_H |
| P1-53 | CAN_L |
| P1-1 or P1-2 | GND (required for CAN bus) |

Use a twisted pair for CAN_H/CAN_L. Connect CAN ground (P1-1 or P1-2) to ECU/harness ground.

---

## ECU/vehicle connection — checklist

When wiring to an ECU or vehicle harness:

| Connection | Where |
|------------|-------|
| **12 V supply** | P1-4 or J5-2 |
| **Ground** | P1-1, P1-2, or J5-5 — **must be common with ECU** |
| **Injector commands** | P1-40..49 (see table above) |
| **Injector outputs** | H1/L1..H7/L8 on P1 |
| **CAN** | P1-52 (CAN_H), P1-53 (CAN_L), P1-1/2 (GND) |
| **Vdrive** | Jumper J5-1 to J5-2 for VBAT-only, or use boost converter when available |

---

## Enabling high-side drivers

By default the firmware holds `HS_VBATx` and `HS_VBOOSTx` LOW (outputs disabled).

- **HS_VBOOSTx** switches **Vdrive → Hx** (injector supply).
- **HS_VBATx** is a separate VBAT path; it does not drive Hx directly.
- For VBAT-only bring-up, tie Vdrive to battery (jumper J5-1↔J5-2) and enable **HS_VBOOSTx**.

### Recommended profile: VBAT-only, 4 injectors + HPFP on bank 4

```bat
set GDI_CMAKE_EXTRA_ARGS=-DGDI_PROFILE_VBAT_ONLY_4CYL_HPFP_BANK4=ON
scripts\build_firmware.bat
```

### Other profiles

Enable VBOOST banks 1+2 after 500 ms:

```bat
set GDI_CMAKE_EXTRA_ARGS=-DGDI_ENABLE_HS_VBOOST=ON -DGDI_HS_VBOOST_MASK=0x3
scripts\build_firmware.bat
```

Enable VBAT banks 1+2:

```bat
set GDI_CMAKE_EXTRA_ARGS=-DGDI_ENABLE_HS_VBAT=ON -DGDI_HS_VBAT_MASK=0x3
scripts\build_firmware.bat
```

Bit 0 = bank 1, bit 1 = bank 2, bit 2 = bank 3, bit 3 = bank 4. If SafetyMonitor reports unhealthy, all enables are forced LOW.

---

## Bench test — single injector (INJ1)

1. **Power:** J5-2 (+) and J5-5 (−) from 12 V supply. Jumper J5-1 to J5-2.
2. **Load:** Injector or dummy load between H1 (P1-118/121) and L1 (P1-114).
3. **Command:** Drive P1-44 with logic pulses (5–20 Hz, ~1–2 ms) relative to board GND.
4. **Expect:** With HS_VBOOST1 enabled and Vdrive tied to battery, H1 ≈ 12 V after delay; L1 pulses low during on-time.
