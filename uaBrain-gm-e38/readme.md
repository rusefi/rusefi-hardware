# uaBrain-gm-e38

GM E38 carrier for the hw-uaBrain module, mounted above the connector board.
**Status (2026-10-09): routed carrier with rev B module and seven BOM-listed support headers. ERC, PCB connectivity and schematic parity pass under the project rules. Hardware and firmware have not been validated on a built unit. Earlier dated notes below describe previous project states.**

Only grey 80-pin J1 and black 73-pin J2 are fitted. The middle blue connector is omitted. Micro-Fit assignments are deferred. Names below follow the actual hellen-gm-e38 schematic; firmware pinout files call the black connector J1 and the grey connector J2.

Prototype decision (2026-10-09): the existing VNLD5160 control levels are accepted by the project owner for this build. No driver or control-interface change is requested. Manufacturing files match the saved CAD; review of the current JLC draft, its stock and placement preview is still pending.

## Sources and scope

- J1/J2 retain the singleside symbols from the source E38 schematic, including their pin types, numbering, positions and graphics. Both connectors now use the existing public `ext/kicad6-libraries` submodule; the added private-library submodule registration has been removed. No additional project-local libraries are used. The 73-pin singleside symbol and both connector footprints already existed in the public library. Only the missing 80-pin singleside symbol was added to its existing library file, copied from the schematic with its footprint reference changed to `kicad6-libraries`. Connector symbol footprint references now match the public library names already used on the PCB.
- Connections traced from the current local `../../hellen-gm-e38/hellen-gm-e38.kicad_sch` and its hierarchical sheets, including series fuses and ETB jumpers.
- Every one of the 112 module symbol pads checked against `../../hw-uaBrain/hw-uaBrain.kicad_sch` and its 14 eight-pin connectors.
- VSS polarity and permanent / relay power roles additionally checked in `../../fw-e38/connectors/e38-j1-black.yaml`.
- Eight injector channels are connected: INJ1-INJ6 use the dedicated module outputs; INJ7/INJ8 use LS9/LS10. Four logic ignition outputs drive all eight harness coil inputs in wasted-spark pairs: 1/6, 8/5, 7/4 and 2/3. Module IGN5/IGN6 are reserved. Firmware configuration and carrier design are still incomplete.
- All ten low-side outputs are assigned. LS7 (G6) now drives skip-shift, LS9 (M5) drives INJ7, and LS10 (M6) drives INJ8 after deferring EVAP PURGE and the two oxygen-sensor heaters. DOD and EVAP VENT were already unassigned. LS9/LS10 have no external flyback diode to +12V_RAW on the module, matching its dedicated injector stages; LS1-LS8 have these diodes. No high-side outputs are substituted by low-side outputs.
- This step adds module-to-harness connections. Original hellen-gm-e38 ignition-output fuses and external tachometer conditioning are not duplicated; carrier protection and tachometer pull-up requirements remain to be designed.
- Unmatched named contacts have wires and actual KiCad net labels (power symbols for supply rails). Matching copies are collected on short connection stubs in three groups: 8 other unassigned harness nets and the module auxiliary/reserve nets (power and I2C also serve the ADC sheet), and 8 deliberately deferred harness nets (DOD, EVAP and O2SHTR). These are electrically connected labels. Within each group, stubs are sorted under signal-type headings: power, analog inputs, digital inputs, logic outputs, low/high-side or other power outputs, analog/digital interfaces, and spare contacts, as applicable. WBO analog interfaces remain distinct from general analog inputs; their heater outputs are grouped with power outputs. Only the 46 unnamed unused harness contacts retain no-connect crosses. In the mapping tables, NC means there is no assigned counterpart on uaBrain; the named source net is still brought to its group stub.

A fourth group, `HARNESS: CONNECTED TO uaBrain`, sits to the left of the harness reserve group. It contains matching electrical labels and power symbols for all 49 assigned nets, sorted into power/ground, analog inputs, knock/VR inputs, digital inputs, ignition, injectors, relay/auxiliary outputs, ETB and CAN. Contact annotations identify J1/J2 pins; `J1/J2 [n]` gives the total harness contact count for a shared supply/ground rail. These stubs duplicate existing net names without changing contact assignments.

## Library setup

The project symbol table uses public `../ext/kicad6-libraries` for uaBrain, the two Molex connector symbols and ADS7128, public `../ext/hellen-one/kicad` for the existing resistor/capacitor symbols and footprints, and standard KiCad power, op-amp and protection libraries. No private-library dependency is added. Both connector footprints and STEP models already existed in that shared library; no footprint geometry or model files were copied or changed in the published update. This carrier PCB already uses `${KIPRJMOD}/../ext/kicad6-libraries/0313874018.stp` and `0313872014.stp`. The shared footprint files retain their original model paths; those paths need adjustment when importing footprints into a project with a different directory layout.

Published on 2026-09-23 to `rusefi/kicad6-libraries` main: [bd0d908 — Add singleside GM E38 80-pin connector symbol](https://github.com/rusefi/kicad6-libraries/commit/bd0d9084e067fbdc85fe0fac28c1c17d1be0ac70). The update adds only the missing 80-pin singleside symbol and changes the existing 73-pin singleside symbol's footprint reference to the public library. It retains the library's KiCad 6 file format. The carrier submodule was checked out at this connector-only commit and is now at the rev B update below; the parent repository's gitlink update remains part of its uncommitted project changes. The earlier local footprint model-path edits were removed when switching to the published library. Do not restore the removed private submodule from old snapshots or rerun the earlier one-off library-switching scripts.

## uaBrain rev B update (2026-09-23)

The public library and the carrier's module instance now match source revision B
at `hw-uaBrain` commit `51655b7b9c2ee789dcc988f333b0093db6252914`:

| Pad | Previous function | Rev B function | Carrier handling |
|---|---|---|---|
| L6 | +3.3V | SDA | Disconnected from +3.3V; labelled at M1 and in the digital-interface reserve |
| L8 | +5V | SCL | Disconnected from +5V; labelled at M1 and in the digital-interface reserve |

D6 remains +3.3V and D8 remains +5V. All other module-to-harness assignments
are unchanged. The existing PCB M1 instance has matching L6/L8 net names,
pin functions and front/back labels; other PCB nets and placement were preserved.
The PCB still requires a complete synchronization and routing pass.

## uaBrain support headers (2026-10-09)

The carrier now has seven physical 1x16 headers, J8-J14, on the top side.
They are XUNPU PH2.54-01-16PZD, JLC/LCSC C7501590, 2.54 mm pitch,
0.64 mm square pins, 6.0 mm mating length and 3.0 mm solder tails.
The [manufacturer drawing](https://www.lcsc.com/datasheet/C7501590.pdf)
specifies a 2.50 mm insulator and a nominal 40.44 mm body length.

| Header | uaBrain pads, in header pin 1 to 16 order | Position |
|---|---|---|
| J8 | A1-A8, B1-B8 | Right, lower half |
| J9 | C1-C8, D1-D8 | Right, upper half |
| J10 | E1-E8, F1-F8 | Top, right third |
| J11 | G1-G8, H1-H8 | Top, middle third |
| J12 | J1-J8, K1-K8 | Top, left third |
| J13 | L1-L8, M1-M8 | Left, upper half |
| J14 | N1-N8, P1-P8 | Left, lower half |

M1 owns all 112 physical plated holes and their electrical connections.
Its symbol is included in PCB updates, with the original module pad numbers
and schematic association restored. Only M1's BOM and position-export
exclusions are enabled, because the module is installed after JLC assembly.
All original hole positions, 1.1 mm drills, 2.0 mm copper pads and thermal
settings are unchanged. M1 retains the pin legends, microSD cutout and
assembled module model.

J8-J14 are assembly-only components: standard Mechanical:Mechanical_Shape
symbols without electrical pins and project-instance header footprints
without copper pads or drilled holes. Their physical contacts use M1's
existing holes. Each retains its part number, 3D model and native placement
centre, and appears in the BOM and CPL. The header symbols do not duplicate
any net connections. This keeps electrical synchronization directly on M1.

The module and seven headers belong to one locked PCB group. Enter the
group to select a member, or unlock and move the whole group together.
Normal Update PCB from Schematic retains the placed footprint geometry.
Do not reset J8-J14 from their standard pin-header library footprints:
these intentionally padless board instances would regain duplicate pads.
Use the project-instance footprint editor for changes to these components.

Assembly: solder all seven header insulators flush against the carrier's
top face, with the 6 mm ends pointing up. Keep the three rows aligned and
the pins perpendicular. Adjacent strips have 0.20 mm nominal end clearance;
the drawing's body-length tolerance can consume that clearance, so check
the moulded ends during assembly. No strip needs cutting to pin count.
Install and solder uaBrain later at a nominal 5.90 mm face-to-face PCB gap:
2.50 + 6.00 - 1.60 - 1.00 = 5.90 mm, leaving about 1 mm exposed above a
1.6 mm module. Use spacers/alignment tooling while soldering the module.

The module model remains 3d/hw-uaBrain-assembled.wrz, at scale 1/2.54
and Z=7.50 mm. The project variable
KICAD6_LIBRARIES=${KIPRJMOD}/../ext/kicad6-libraries resolves it.
Each header has the standard KiCad STEP model; its 2.54 mm insulator is a
representative model, 0.04 mm taller than this part's nominal 2.50 mm.
The old combined decorative header model is removed from M1.
The alternate hw-uaBrain-Castellated library footprint is not populated on
the production carrier.

Manufacturing data in production/ contains 114 assembly references in 23
BOM groups, including seven C7501590 headers and excluding M1. Native BOM
and placement exports match the existing production CSV content exactly.
ERC, DRC connectivity and schematic parity pass; the 16 library-mismatch
warnings reflect the retained local footprint overrides. Independent
comparison confirms identical geometry in all 11 fabrication files,
including both drill files, copper, masks, paste, silkscreen and outline.
A disposable negative control with the wrong net on M1.N5 produced a
native schematic-parity error, confirming M1 is checked again.

Published shared-library update: [8367b32 - Update uaBrain for rev B and add assembled 3D model](https://github.com/rusefi/kicad6-libraries/commit/8367b325921799ca5ba5a2d876634c23be0d76da).
The carrier's public-library submodule is at this commit, superseding the
connector-only revision described above. Carrier project changes and the
parent gitlink update remain uncommitted.

## I2C ADC expansion (bias audited 2026-10-07)

The hierarchical [i2c-adc.kicad_sch](i2c-adc.kicad_sch) sheet connects eight
harness inputs through ADS7128 U4 to module SDA (M1.L6) and SCL (M1.L8).
U2 buffers AIN0-AIN3 and U3 buffers AIN4-AIN7; both are MCP6004.
D1/D2 are SRV05-4-P-T7 protection arrays in SOT-23-6.

Input bias follows the effective fitted configuration of hellen-gm-e38
revision C, including its module BOM overrides and baseboard resistors.
References and connector names below are those of the current carrier:
J1 is the 73-pin connector and J2 is the 80-pin connector.

| ADC | Harness contact | Input | Pullup to +5VA | Pulldown to GNDA |
|---|---|---|---|---|
| AIN0 | J2.23 | IN_ATF_TEMP | R4: 4.7k | R5: DNP |
| AIN1 | J1.12 | IN_AC_PRESSURE | R8: DNP | R9: 680k |
| AIN2 | J1.24 | IN_FUEL_TANK_PRESSURE | R12: DNP | R13: 680k |
| AIN3 | J1.26 | IN_CLUTCH_POSITION | R16: DNP | R17: 680k |
| AIN4 | J1.16 | IN_PRIM_FUEL_LEVEL | R1: 100 ohm, 2512; R6: DNP | R7: 680k |
| AIN5 | J1.70 | IN_SEC_FUEL_LEVEL | R2: 100 ohm, 2512; R10: DNP | R11: 680k |
| AIN6 | J2.33 | IN_OIL_LEVEL_SWITCH | R14: 10k | R15: DNP |
| AIN7 | J2.57 | IN_BK1S1SIG | R18: 4.7k | R19: DNP |

R1/R2 already provide fuel-level excitation on the main sheet; do not
populate the additional R6/R10 pullup options. R1/R2 use C24920 (100 ohm,
1 W, 2512), matching the original board. The five 680k pulldowns use
[C25822](https://www.lcsc.com/product-detail/C25822.html), a 0603 part;
the original module's 0402 ordering code must not be copied to these pads.
All eight unpopulated bias resistors have KiCad's DNP attribute set in
both schematic and PCB, and their LCSC fields are empty.

The buffer and filter topology remains the requested premium-quick-test
ADC0 circuit: 10k series, 100n and clamp at the MCP6004 input, unity-gain
follower, then 10k series and 160p at the ADS7128 input. The bias correction
does not copy the original module's different filters or digital receiver.
In particular, BK1S1 was a digital AUX input in the original; here it is
read through AIN7 with the original 4.7k bias. Firmware thresholds and
sensor calibration must reflect the fitted bias and the ADC0 filter.

AVDD, buffer supplies and clamp rails use +5VA; DVDD uses +3.3VA.
Analog returns and ADC pads 9/17 connect to GNDA. The hidden stacked pad
17 is passive, preventing an implicit connection between GND and GNDA.
The module provides the I2C pullups. ADS7128 has 1u decoupling on AVDD,
DVDD and DECAP; each MCP6004 has 100n. ADDR remains open for address 0x10,
ALERT is unused, and all eight AIN pins are intended as ADC inputs.

Audit sources: the original schematic, mega-mcu144 module 0.7 schematic,
and [revision C bias overrides](../../hellen-gm-e38/bom_pullups_hellen-gm-e38-c.csv).
Fresh XML netlists verify every resistor pin and all eight signal paths.
The 2026-10-07 correction changes only resistor values, LCSC codes and DNP
attributes; wiring, filtering, PCB routing, placement and text geometry
are preserved. Native ERC reports zero violations under project rules;
DRC reports zero errors, zero unconnected items, zero schematic parity
issues and the same nine existing footprint-library warnings.

## Paired ignition outputs

The [requested wiring reference](https://github.com/rusefi/private-hardware/issues/287#issuecomment-5603998714) gives companion-cylinder pairs for a Gen IV GM LS with firing order 1-8-7-2-6-5-4-3. Four module outputs are assigned in that pair order:

| Module output | M1 pad | MCU pin | Cylinders | J1 contacts | Carrier net |
|---|---|---|---|---|---|
| OUT_IGN1 | B1 | PC13 | 1 and 6 | 70 and 74 | OUT_IGN1_6 |
| OUT_IGN2 | B2 | PE5 | 8 and 5 | 71 and 75 | OUT_IGN8_5 |
| OUT_IGN3 | B3 | PE4 | 7 and 4 | 72 and 76 | OUT_IGN7_4 |
| OUT_IGN4 | J1 | PE3 | 2 and 3 | 73 and 77 | OUT_IGN2_3 |

Each pair is one net with two harness contacts and one module output, with identical labels at all three contacts. Module outputs are not tied to one another. OUT_IGN5 (M1.J2) and OUT_IGN6 (M1.J3) remain separately labelled at the module and in its `LOGIC OUTPUTS / IGNITION` reserve category.

Configure firmware for wasted-spark operation with this output order and the actual coil dwell/polarity. Individual sequential control of the two coils within a wired pair is not available. This step changes schematic connections only; simultaneous loading by two coil signal inputs and operation on the engine have not been tested.

## Deferred functions and reassigned module outputs

DOD, EVAP and oxygen-sensor heaters are not used on uaBrain at this stage. Their original labels remain attached to the harness contacts, with matching labels in the separate `DEFERRED: NOT USED` group. IN_BK1S1SIG was formerly deferred because the reference E38 used it as a digital AUX input; it is now assigned to ADS7128 AIN7 as an analog input.

| Function | Harness contact(s) | Freed uaBrain output |
|---|---|---|
| EVAP PURGE | J1.8 | G6 / OUT_LS7 |
| DOD CYL3, CYL1, CYL2, CYL4 | J1.9, J1.10, J1.11, J1.14 | Already unassigned |
| O2SHTR: BK1S1HTR | J1.12 | M5 / OUT_LS9 |
| O2SHTR: BK2S1HTR | J1.13 | M6 / OUT_LS10 |
| EVAP VENT | J2.61 | Already unassigned |
| BK1S1SIG, now ADS7128 AIN7 | J1.57 | K4 / IN_HALL3 remains assigned to reverse switch |

The three released outputs are now assigned as follows. Matching function labels are attached at the harness and module contacts; their former reserve stubs have been removed.

| Carrier net | Harness contact | uaBrain pad / signal | Firmware signal |
|---|---|---|---|
| OUT_INJ7 | J1.40 | M5 / OUT_LS9 | PD13 |
| OUT_INJ8 | J1.17 | M6 / OUT_LS10 | PC6 |
| OUT_SKIP_SHIFT_SOLENOID | J2.66 | G6 / OUT_LS7 | IO2, MCU module pad N19b |

LS9/LS10 use U6 (VNLD5160TR-E), the same driver as the dedicated injector stages U9-U11. Both have the same 4.7 kOhm input series/pulldown network and no external output diode to the supply. The original E38 INJ7/INJ8 stages also use VNLD5160. The driver's internal clamp provides fast demagnetization at turn-off; see the [ST datasheet](https://www.st.com/resource/en/datasheet/vnld5160-e.pdf). This assignment follows the existing injector-driver topology; injector load and thermal validation are still required. Firmware must configure PD13 and PC6 as INJ7 and INJ8; this schematic change does not configure firmware.

LS7 uses U18 with an external flyback diode D11 to the module supply, matching the original E38 skip-shift stage U9/D19. OUT_ALT_LIGHT remains reserved: its original stage also has a 100 Ohm pull-up to +5VA, which is not present in a plain uaBrain low-side output.

## Power and sensor returns

Power rails use the standard KiCad `power` library. The `power:+12V` instances have distinct Value fields for +12V_PERM, 12V_KEY, +12V_RAW and +12V_RAW_FUSED; KiCad 10 uses those values as the global power net names. No project-local power library is required.

All 79 power symbols at J1/J2 and M1 are aligned in consistent columns, with their names aligned to the adjacent signal labels. Their contact assignments and separate rail names are preserved.

- **J2.47 main-relay supply (+12V_RAW) -> M1.M1 (+12V input)**. The uaBrain +12V contact is the RAW supply input; there is no separate PERM input.
- **J2.20 permanent battery (+12V_PERM) has no uaBrain counterpart** and is brought to the harness reserve group, with matching power symbols at J2.20 and the reserve stub.
- **J2.19 ignition key -> M1.M2 (12V_KEY)**, through the module F1 to IN_VIGN.
- **A6/H6 remain available in the uaBrain reserve group as +12V_RAW_FUSED**. Inside uaBrain, F2 connects the +12V input to the internal +12V_RAW rail on A6/H6. The carrier uses the distinct name +12V_RAW_FUSED for these contacts so that carrier power symbols cannot bypass F2. The module symbol pin names remain the original +12V_RAW. Motor drivers have their own F3/F4 branches.
- +5VP sensor supply pads share a net. GNDA sensor-return pads share a separate net from carrier GND. Their internal module connections and final PCB return routing still govern noise performance.
- J2.72 is the VSS negative lead. It goes to C1 (VR_DISCRETE-), not to carrier GND. J2.73 remains power ground. The old board grounded the VSS negative lead because its NCV1124 input was single ended.

## Input and firmware assumptions

- J1.68 crank -> C4/HALL1 (MCU PE12); J1.64 cam -> G3/HALL2 (PE13).
- J2.71 / J2.72 VSS -> C2 / C1, the built-in discrete VR conditioner. Its digital output goes to PE0; the old draft HALL3 assignment was unsuitable for the referenced VR sensor.
- J1.57 / IN_BK1S1SIG now goes to ADS7128 AIN7. Its former K4/HALL3 input remains connected to **J2.8 / IN_REVERSE_SWITCH (MCU PE14)**. The actual module net connects J18.4 to MCU-module M6.S2 / IN_D3; the MCU-module schematic shows input filtering, clamps and a Schmitt buffer. `../../hw-uaBrain/bom_pullups_hw-uaBrain-a.csv` specifies a 4.7 kOhm pull-up R132 to V5A, no R138 pulldown and 1 nF C145. This supports a switch closing to ground; configure active-low polarity and debounce in firmware and confirm that contact behavior on the actual harness.
- J2.41 MAF -> F3/AUX2 preserves the analog-input assignment of the reference board. A frequency-output MAF requires reassignment to a suitable timer input and verification of pulls / filtering.
- J1.50 oil pressure -> D2/AUX1 requires the normal uaBrain AUX1 configuration (CAN wake-up option disabled).
- P/N, brake and clutch switches use the module BUTTON inputs. Check switch polarity and the fitted configurable pull resistors for the actual harness.
- Firmware must be configured for the new module pin assignment, including relay defaults, ETB polarity, VSS channel and ignition/injection order. Schematic/ERC checks do not establish operation on a vehicle.

## J1: 80-pin grey connector (Molex 31387-4018)

| Pin | E38 function | M1 pad(s) | Module signal | Note |
|---|---|---|---|---|
| 1 | -  | NC | - | Unused in reference schematic |
| 2 | +5VP | C5, D3, E5, F4, F7, K5, L3, N3, P7 | +5VP |  |
| 3 | +5VP | C5, D3, E5, F4, F7, K5, L3, N3, P7 | +5VP |  |
| 4 | -  | NC | - | Unused in reference schematic |
| 5 | OUT_ETB+ | E2 | OUT_DC1+ |  |
| 6 | OUT_ETB- | E1 | OUT_DC1- |  |
| 7 | -  | NC | - | Unused in reference schematic |
| 8 | OUT_EVAP_PURGE | NC | - | Deferred; G6 / OUT_LS7 reassigned to skip-shift |
| 9 | OUT_DOD_CYL3 | NC | - | Deferred DOD function |
| 10 | OUT_DOD_CYL1 | NC | - | Deferred DOD function |
| 11 | OUT_DOD_CYL2 | NC | - | Deferred DOD function |
| 12 | OUT_BK1S1HTR | NC | - | Deferred O2SHTR; M5 / OUT_LS9 reassigned to INJ7 |
| 13 | OUT_BK2S1HTR | NC | - | Deferred O2SHTR; M6 / OUT_LS10 reassigned to INJ8 |
| 14 | OUT_DOD_CYL4 | NC | - | Deferred DOD function |
| 15 | -  | NC | - | Unused in reference schematic |
| 16 | OUT_VVT_HS | NC | - | Reserved: requires a high-side driver; no matching exposed uaBrain output |
| 17 | OUT_INJ8 | M6 | OUT_LS10 | VNLD5160 injector stage; MCU PC6 |
| 18 | OUT_INJ5 | J5 | OUT_INJ5 |  |
| 19 | OUT_INJ6 | J6 | OUT_INJ6 |  |
| 20 | OUT_INJ1 | B4 | OUT_INJ1 |  |
| 21 | IN_CLT | E7 | IN_CLT |  |
| 22 | GNDA | B8, C6, D4, E6, E8, F5, F8, G2, J8, K6, L4, N4, P8 | GNDA |  |
| 23 | IN_ATF_TEMP | via I2C | ADS7128 AIN0 | ADC0-type filter/protection/buffer; SDA L6, SCL L8 |
| 24 | GNDA | B8, C6, D4, E6, E8, F5, F8, G2, J8, K6, L4, N4, P8 | GNDA |  |
| 25 | -  | NC | - | Unused in reference schematic |
| 26 | IN_KNOCK1 | B7 | IN_KNOCK1_RAW |  |
| 27 | GNDA | B8, C6, D4, E6, E8, F5, F8, G2, J8, K6, L4, N4, P8 | GNDA |  |
| 28 | -  | NC | - | Unused in reference schematic |
| 29 | IN_KNOCK2 | J7 | IN_KNOCK2_RAW |  |
| 30 | GNDA | B8, C6, D4, E6, E8, F5, F8, G2, J8, K6, L4, N4, P8 | GNDA |  |
| 31 | -  | NC | - | Unused in reference schematic |
| 32 | P1_SPARE | NC | - | Reserved spare pad in reference schematic |
| 33 | IN_OIL_LEVEL_SWITCH | via I2C | ADS7128 AIN6 | ADC0-type filter/protection/buffer; SDA L6, SCL L8 |
| 34 | GNDA | B8, C6, D4, E6, E8, F5, F8, G2, J8, K6, L4, N4, P8 | GNDA |  |
| 35 | GNDA | B8, C6, D4, E6, E8, F5, F8, G2, J8, K6, L4, N4, P8 | GNDA |  |
| 36 | -  | NC | - | Unused in reference schematic |
| 37 | OUT_INJ2 | B5 | OUT_INJ2 |  |
| 38 | OUT_INJ3 | B6 | OUT_INJ3 |  |
| 39 | OUT_INJ4 | J4 | OUT_INJ4 |  |
| 40 | OUT_INJ7 | M5 | OUT_LS9 | VNLD5160 injector stage; MCU PD13 |
| 41 | +5VP | C5, D3, E5, F4, F7, K5, L3, N3, P7 | +5VP |  |
| 42 | -  | NC | - | Unused in reference schematic |
| 43 | +5VP | C5, D3, E5, F4, F7, K5, L3, N3, P7 | +5VP |  |
| 44 | +5VP | C5, D3, E5, F4, F7, K5, L3, N3, P7 | +5VP |  |
| 45 | -  | NC | - | Unused in reference schematic |
| 46 | -  | NC | - | Unused in reference schematic |
| 47 | -  | NC | - | Unused in reference schematic |
| 48 | -  | NC | - | Unused in reference schematic |
| 49 | -  | NC | - | Unused in reference schematic |
| 50 | IN_OIL_PRESSURE | D2 | IN_AUX1 |  |
| 51 | -  | NC | - | Unused in reference schematic |
| 52 | GND | C3, C8, G8, K3, K8, M3, M4, P5 | GND |  |
| 53 | GNDA | B8, C6, D4, E6, E8, F5, F8, G2, J8, K6, L4, N4, P8 | GNDA |  |
| 54 | -  | NC | - | Unused in reference schematic |
| 55 | -  | NC | - | Unused in reference schematic |
| 56 | GND | C3, C8, G8, K3, K8, M3, M4, P5 | GND |  |
| 57 | IN_BK1S1SIG | via I2C | ADS7128 AIN7 | ADC0-type filter/protection/buffer; SDA L6, SCL L8 |
| 58 | IN_MAP | F6 | IN_MAP |  |
| 59 | -  | NC | - | Unused in reference schematic |
| 60 | GND | C3, C8, G8, K3, K8, M3, M4, P5 | GND |  |
| 61 | OUT_ALT_LIGHT | NC | - | Reserved: no free low-side output; original stage also needs a 100 Ohm pull-up to +5VA |
| 62 | -  | NC | - | Unused in reference schematic |
| 63 | IN_TPS2 | E4 | IN_TPS2 |  |
| 64 | IN_CAM | G3 | IN_HALL2 |  |
| 65 | IN_TPS1 | E3 | IN_TPS1 |  |
| 66 | GNDA | B8, C6, D4, E6, E8, F5, F8, G2, J8, K6, L4, N4, P8 | GNDA |  |
| 67 | -  | NC | - | Unused in reference schematic |
| 68 | IN_CRANK | C4 | IN_HALL1 |  |
| 69 | GND | C3, C8, G8, K3, K8, M3, M4, P5 | GND |  |
| 70 | OUT_IGN1 | B1 | OUT_IGN1 | Cyl 1; paired with J1.74 / cyl 6 on OUT_IGN1_6 |
| 71 | OUT_IGN8 | B2 | OUT_IGN2 | Cyl 8; paired with J1.75 / cyl 5 on OUT_IGN8_5 |
| 72 | OUT_IGN7 | B3 | OUT_IGN3 | Cyl 7; paired with J1.76 / cyl 4 on OUT_IGN7_4 |
| 73 | OUT_IGN2 | J1 | OUT_IGN4 | Cyl 2; paired with J1.77 / cyl 3 on OUT_IGN2_3 |
| 74 | OUT_IGN6 | B1 | OUT_IGN1 | Cyl 6; paired with J1.70 / cyl 1 on OUT_IGN1_6 |
| 75 | OUT_IGN5 | B2 | OUT_IGN2 | Cyl 5; paired with J1.71 / cyl 8 on OUT_IGN8_5 |
| 76 | OUT_IGN4 | B3 | OUT_IGN3 | Cyl 4; paired with J1.72 / cyl 7 on OUT_IGN7_4 |
| 77 | OUT_IGN3 | J1 | OUT_IGN4 | Cyl 3; paired with J1.73 / cyl 2 on OUT_IGN2_3 |
| 78 | GND | C3, C8, G8, K3, K8, M3, M4, P5 | GND |  |
| 79 | GND | C3, C8, G8, K3, K8, M3, M4, P5 | GND |  |
| 80 | -  | NC | - | Unused in reference schematic |

## J2: 73-pin black connector (Molex 31387-2014)

| Pin | E38 function | M1 pad(s) | Module signal | Note |
|---|---|---|---|---|
| 1 | IN_P/N_SWITCH | L2 | IN_BUTTON3 |  |
| 2 | -  | NC | - | Unused in reference schematic |
| 3 | -  | NC | - | Unused in reference schematic |
| 4 | -  | NC | - | Unused in reference schematic |
| 5 | -  | NC | - | Unused in reference schematic |
| 6 | IN_CLUTCH_SWITCH | N8 | IN_BUTTON2 |  |
| 7 | -  | NC | - | Unused in reference schematic |
| 8 | IN_REVERSE_SWITCH | K4 | IN_HALL3 | MCU PE14; existing 4.7 kOhm pull-up, switch-to-ground input |
| 9 | IN_STOP_SWITCH | N7 | IN_BUTTON1 |  |
| 10 | -  | NC | - | Unused in reference schematic |
| 11 | -  | NC | - | Unused in reference schematic |
| 12 | IN_AC_PRESSURE | via I2C | ADS7128 AIN1 | ADC0-type filter/protection/buffer; SDA L6, SCL L8 |
| 13 | GNDA | B8, C6, D4, E6, E8, F5, F8, G2, J8, K6, L4, N4, P8 | GNDA |  |
| 14 | -  | NC | - | Unused in reference schematic |
| 15 | -  | NC | - | Unused in reference schematic |
| 16 | IN_PRIM_FUEL_LEVEL | via I2C | ADS7128 AIN4 | ADC0-type filter/protection/buffer; SDA L6, SCL L8 |
| 17 | OUT_FAN1_RELAY | M8 | OUT_LS2 |  |
| 18 | -  | NC | - | Unused in reference schematic |
| 19 | 12V_KEY | M2 | 12V_KEY | Ignition key to module 12V_KEY, with module F1 in series |
| 20 | +12V_PERM | NC | - | Reserved: uaBrain has no separate PERM input |
| 21 | -  | NC | - | Unused in reference schematic |
| 22 | GNDA | B8, C6, D4, E6, E8, F5, F8, G2, J8, K6, L4, N4, P8 | GNDA |  |
| 23 | GNDA | B8, C6, D4, E6, E8, F5, F8, G2, J8, K6, L4, N4, P8 | GNDA |  |
| 24 | IN_FTPS | via I2C | ADS7128 AIN2 | ADC0-type filter/protection/buffer; SDA L6, SCL L8 |
| 25 | -  | NC | - | Unused in reference schematic |
| 26 | IN_CLUTCH_POSITION | via I2C | ADS7128 AIN3 | ADC0-type filter/protection/buffer; SDA L6, SCL L8 |
| 27 | CAN_L | N6 | CAN1- |  |
| 28 | CAN_H | N5 | CAN1+ |  |
| 29 | IN_PPS1 | N1 | IN_PPS1 |  |
| 30 | GNDA | B8, C6, D4, E6, E8, F5, F8, G2, J8, K6, L4, N4, P8 | GNDA |  |
| 31 | GNDA | B8, C6, D4, E6, E8, F5, F8, G2, J8, K6, L4, N4, P8 | GNDA |  |
| 32 | IN_PPS2 | N2 | IN_PPS2 |  |
| 33 | +5VP | C5, D3, E5, F4, F7, K5, L3, N3, P7 | +5VP |  |
| 34 | +5VP | C5, D3, E5, F4, F7, K5, L3, N3, P7 | +5VP |  |
| 35 | -  | NC | - | Unused in reference schematic |
| 36 | +5VP | C5, D3, E5, F4, F7, K5, L3, N3, P7 | +5VP |  |
| 37 | IN_IAT | G1 | IN_IAT |  |
| 38 | GNDA | B8, C6, D4, E6, E8, F5, F8, G2, J8, K6, L4, N4, P8 | GNDA |  |
| 39 | -  | NC | - | Unused in reference schematic |
| 40 | IN_DIN1 | P6 | IN_FLEX |  |
| 41 | IN_MAF | F3 | IN_AUX2 | Analog MAF, as on hellen-gm-e38; frequency MAF needs different input assignment |
| 42 | GNDA | B8, C6, D4, E6, E8, F5, F8, G2, J8, K6, L4, N4, P8 | GNDA |  |
| 43 | -  | NC | - | Unused in reference schematic |
| 44 | -  | NC | - | Unused in reference schematic |
| 45 | -  | NC | - | Unused in reference schematic |
| 46 | -  | NC | - | Unused in reference schematic |
| 47 | +12V_RAW | M1 | +12V | Main-relay supply to module RAW input; F2 feeds the separate A6/H6 rail |
| 48 | OUT_TACH | L1 | OUT_LS6 |  |
| 49 | -  | NC | - | Unused in reference schematic |
| 50 | OUT_FUEL_PUMP1_RELAY_HS | NC | - | Reserved: requires a high-side driver; no matching exposed uaBrain output |
| 51 | OUT_FUEL_PUMP2_RELAY_HS | NC | - | Reserved: requires a high-side driver; no matching exposed uaBrain output |
| 52 | OUT_STARTER_RELAY_HS | NC | - | Reserved: requires a high-side driver; no matching exposed uaBrain output |
| 53 | +5VP | C5, D3, E5, F4, F7, K5, L3, N3, P7 | +5VP |  |
| 54 | +5VP | C5, D3, E5, F4, F7, K5, L3, N3, P7 | +5VP |  |
| 55 | -  | NC | - | Unused in reference schematic |
| 56 | +5VP | C5, D3, E5, F4, F7, K5, L3, N3, P7 | +5VP |  |
| 57 | OUT_VSS_HS | NC | - | Reserved: requires a high-side driver; no matching exposed uaBrain output |
| 58 | OUT_FAN2_RELAY | C7 | OUT_LS3 |  |
| 59 | OUT_MAIN_RELAY | M7 | OUT_LS1 |  |
| 60 | -  | NC | - | Unused in reference schematic |
| 61 | OUT_EVAP_VENT | NC | - | Deferred EVAP function |
| 62 | -  | NC | - | Unused in reference schematic |
| 63 | OUT_AC_RELAY | D1 | OUT_LS4 |  |
| 64 | -  | NC | - | Unused in reference schematic |
| 65 | -  | NC | - | Unused in reference schematic |
| 66 | OUT_SKIP_SHIFT_SOLENOID | G6 | OUT_LS7 | VNLD5160 with external flyback diode, as on original E38 |
| 67 | OUT_STARTER_RELAY | G7 | OUT_LS8 |  |
| 68 | OUT_CHECK_ENGINE | K7 | OUT_LS5 |  |
| 69 | -  | NC | - | Unused in reference schematic |
| 70 | IN_SEC_FUEL_LEVEL | via I2C | ADS7128 AIN5 | ADC0-type filter/protection/buffer; SDA L6, SCL L8 |
| 71 | IN_VSS+ | C2 | VR_DISCRETE+ | VR sensor positive to discrete VR input; MCU PE0 |
| 72 | IN_VSS- | C1 | VR_DISCRETE- | VR sensor negative; separate from carrier GND |
| 73 | GND | C3, C8, G8, K3, K8, M3, M4, P5 | GND |  |

## Module pads reserved for later connections

A1, A2, A3, A4, A5, A6, A7, A8, D5, D6, D7, D8, F1, F2, G4, G5, H1, H2, H3, H4, H5, H6, H7, H8, J2, J3, K1, K2, L5, L6, L7, L8, P1, P2, P3, P4

## Verification

Checked on 2026-09-23 for rev B: all 112 module pin names match the fresh source
schematic netlist. Of 265 carrier contacts, only M1.L6 and M1.L8 changed nets;
all 224 footprint pad geometries, connector footprints, module placement and
other PCB nets are unchanged. SDA/SCL each have matching module and reserve
labels; reserve annotations now list only D6 on +3.3V and D8 on +5V. The module
reserve now has 33 nets on the same 36 pads, making 57 total reserve/deferred
nets on 60 contacts/pads. The 49 assigned nets are unchanged.

KiCad 10 exported the schematic and rendered the assembled module. Current
ERC: 23 `pin_not_driven` errors, 100 `isolated_pin_label` warnings and 106
`unconnected_wire_endpoint` warnings. The error count is unchanged; six extra
warnings come from the two new intentionally reserved interface nets and their
stubs. No other violation types are present. Snapshots, netlists, renders and
the comparison report are in `C:\Work\rusefi\temp\uabrain-e38-check\rev-b`.

Checked on 2026-09-23 after publishing the shared-library update: GitHub main and the carrier submodule both resolve to `bd0d9084e067fbdc85fe0fac28c1c17d1be0ac70`. Both singleside symbols export successfully with KiCad 10; their graphics and all pin definitions match the carrier schematic. The existing library symbols are preserved except for the intended 73-pin footprint reference. Carrier schematic, PCB and project settings are byte-identical to the pre-publication snapshot. No additional copies of existing footprints or STEP models were added. Publication snapshots and exports are in `C:\Work\rusefi\temp\uabrain-e38-check\upstream-library-2026-09-23`.

Checked on 2026-09-16 after removing the private-library dependency: KiCad 10 successfully loaded/exported both singleside symbols from the public library. Their pin types, positions, numbering and graphics match the schematic cache; the existing 73-pin definition was reused. All 265 physical contacts and all net connections are identical before and after the change. The schematic changed only in four footprint library references. PCB and project settings are byte-identical to the pre-change snapshot. Both original footprint geometries are preserved and their public STEP paths resolve. ERC results remain unchanged. Latest exports and snapshots are in `C:\Work\rusefi\temp\uabrain-e38-check\public-library-2026-09-16`; verification is `C:\Work\rusefi\temp\uabrain-e38-check\verify_public_library.py`.

Checked on 2026-09-09 with KiCad 10 after adding the connected-net group and aligning power symbols: the complete netlist, including all 265 physical contacts, is identical before and after this formatting change. There are 49 assigned nets linking 83 harness contacts to 76 module pads, plus 55 reserve/deferred nets covering 60 contacts/pads. All 49 new group stubs have matching electrical labels/power symbols and verified contact annotations, ordered by signal type. The three existing reserve/deferred groups are unchanged. All 79 source power symbols are aligned. Original E38 symbols, footprint references, power assignments, PCB and project settings are preserved, including the four ignition pairs and separately reserved IGN5/IGN6.

Historical ERC for the 2026-09-09 layout reported 23 `pin_not_driven` errors (13 on J1, 10 on J2), all on deliberately unassigned or deferred single-contact harness nets with the original `input` pin types. That layout had 200 warnings: 96 isolated-pin labels and 104 open group-stub endpoints (55 reserve/deferred and 49 connected-net stubs). No other ERC violations are present; rules and exclusions were not changed.
