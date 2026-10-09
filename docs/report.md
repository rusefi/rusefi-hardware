
## 2026-09-23 - uaBrain GM E38 rev B compatibility

- Updated module L6 from +3.3V to SDA and L8 from +5V to SCL in the shared symbol, footprint labels, carrier schematic and existing PCB M1 instance. Removed obsolete supply connections and added matching digital-interface reserve labels.
- Added the assembled rev B export as an embedded WRZ model in public kicad6-libraries commit 8367b325921799ca5ba5a2d876634c23be0d76da; advanced the carrier submodule to that published commit. No private library added.
- Set an 8 mm board-to-board gap (model Z offset 9.6 mm). Exported geometry extends 4.92 mm below uaBrain; carrier connector geometry within its XY envelope extends 1.49 mm above the carrier, leaving 1.59 mm conservative envelope clearance. Actual standoffs/interconnects remain to be selected. Source model WBO/DNP limitations are documented.
- Verified all 112 module functions against the current source netlist, all 265 carrier contacts, and preservation of 224 pad geometries and unrelated PCB content. Only L6/L8 net assignments changed. KiCad 10 renders the model using the saved project variable without command-line overrides; edited schematic sections visually checked.
- ERC: 23 existing pin_not_driven errors, 100 isolated_pin_label warnings and 106 open-stub warnings. No additional violation types. Full carrier PCB synchronization, routing, mechanical part selection and hardware/firmware validation remain open. Carrier and parent gitlink changes remain uncommitted.

## 2026-09-23 - Remove published model source document

- User explicitly requested deletion of 3d/hw-uaBrain-rev-b.md while retaining the model. Published only that deletion in shared-library commit 1f41db02c31c6e4d39580ae539a3231c7b8ef5c0 and updated the local carrier submodule.
- Verified the WRZ SHA-256 is unchanged and the Markdown file is absent at the current revision. Pin changes remain intact. History was not rewritten; the previous document remains in the earlier commit.

## 2026-09-23 - Align library submodule with rewritten main

- Switched ext/kicad6-libraries to 014741ef206a59785e79f78ecddc776d3b7d753a, verified against origin/main. The submodule working tree is clean and its file tree is identical to the previous approved state.
- Parent gitlink remains a local uncommitted change. No commit, push or support contact was made.


## 2026-09-23 - uaBrain E38 I2C ADC input expansion

- Added i2c-adc.kicad_sch: ADS7128, two MCP6004 buffers and two SRV05-4-P-T7 arrays for seven previously unassigned inputs. Every channel uses premium-quick-test ADC0's 10k/100n input filter and 10k/160p ADC network.
- AIN0..6 map to ATF temperature, AC pressure, fuel-tank pressure, clutch position, primary fuel level, secondary fuel level and oil-level switch. ATF/oil-switch pullups retain E38 BOM values (4.7k/10k); other channels use ADC0's 470k pulldown. DNP alternatives are explicit.
- SDA -> M1.L6, SCL -> M1.L8; existing MCU 2.2k pullups verified. +5VA supplies AVDD/buffers; +3.3V supplies DVDD; returns use GNDA. ADDR open selects 0x10. AIN7 is grounded, ALERT unused, unused op-amp wired as stable follower.
- Added ADS7128 to the existing public chips library and reused public hellen-one passives. Corrected its hidden EP pin to passive stacked pad 17 to prevent an implicit global GND/GNDA short; exported pins 9/17 both connect to GNDA.
- Verified all 265 original contact-to-net assignments and all seven complete signal paths using exported KiCad XML, including component values/footprints against actual ADC0. PCB/project settings are byte-identical to the before snapshot. New ADC sheet ERC is clear; parent has 16 pre-existing errors and 181 pre-existing warnings, with no new violations.
- Schematic rendering checked. Carrier and shared-library changes are local/uncommitted. PCB synchronization/routing, sensor calibration and firmware/hardware validation remain open. Evidence: temp/uabrain-e38-check/i2c-adc under workspace root.


## 2026-09-23 - E38 eighth ADC input and schematic layout

- Connected formerly deferred IN_BK1S1SIG (J1.57) to ADS7128 AIN7 through R29-R32, C20/C21, existing U2D and D2 pin 4. Removed the AIN7 ground tie and U2D input ground; retained its follower feedback. Full ADC0 topology, 470k pulldown and DNP pullup.
- Retained existing component references, non-power symbol UUIDs and sheet identity. Updated the parent sheet pin and removed the BK1S1SIG deferred stub; other deferred outputs are unchanged.
- Reworked all ADC sheet component fields: body-centered resistor/capacitor labels, consistent power labels, aligned eight-channel grid and separate protection/decoupling/ADC blocks. Inspected full render and enlarged channel/lower-block renders.
- Exported netlist verifies eight complete channels, all 265 original harness/module assignments, unchanged channels AIN0-AIN6, separate GND/GNDA and supply/I2C connections. New sheet ERC: zero violations. Parent: 15 pre-existing errors and 178 pre-existing warnings, with no new violations. PCB/project files unchanged. Work remains local/uncommitted.

## 2026-09-28 - uaBrain E38 power symbol label alignment

- Aligned 136 visible power Value fields across the root, I2C ADC, HS5 and LS6 sheets, including custom +12V_RAW symbols. Standardized offsets and horizontal text orientation; used smaller root-sheet power text for connector pitch and placed ADC +3.3VA clear of capacitor fields.
- Preserved hidden labels, symbol positions, electrical properties and all other schematic objects. Compared parsed before/after files: only placed power Value fields changed.
- Exported and inspected SVG views. Before/after netlists match exactly: 222 nets, 632 pin nodes. No commit or push.

## 2026-09-28 - Correct power label formatting per review

- Restored every power label's original font settings from the snapshot before alignment. Removed root-sheet size reduction and vertical offset.
- Used compact 3.175 mm horizontal anchor offsets with text centered on the symbol axis, and 3.81 mm vertical offsets. Removed the special displaced ADC supply label.
- Inspected connector and ADC supply render details. Verified original fonts and parsed schematic equality after excluding power Value fields; electrical objects and other user edits preserved.

## 2026-09-28 - uaBrain footprint I2C legends and microSD access

- Aligned SDA (L6) and SCL (L8) to pad centre lines on both silkscreen layers. Matched the pad-side text edge to neighbouring 5VA legends; retained original font and thickness.
- Added a closed 34 x 22 mm, R2 access window on Edge.Cuts in the library footprint and placed M1. Local centre: X=61.925005, Y=-23.275005 mm. Window is centred on the installed microSD holder; 14.5 x 13.6 mm closed body has 9.75/4.2 mm nominal per-side envelope margin. Extra length accommodates lid swing and card handling. Physical assembly fit has not been tested.
- Mechanical reference: Molex 47219-2001, drawing SD-47219-001. Window located from assembly placement and verified module transforms.
- Preserved concurrent user board placement changes by applying only the footprint edits to the latest saved PCB. Parsed comparison confirms only four text positions and eight Edge.Cuts primitives changed; pad geometry/nets/fonts and other board objects unchanged.
- KiCad accepted the footprint cutout as one closed hole in an isolated outline check. No other placed component intersects the window, and the PCB has no tracks yet. Rendered front legends and cutout inspected. Current user PCB has no external Edge.Cuts outline, so whole-board outline validation remains pending. No commit/push.


## 2026-09-28 - Separate uaBrain mounting footprints

- Added PinHeader and Castellated variants to the shared-library checkout and standalone library, preserving all 112 signal identifiers. Kept the existing hybrid and placed carrier instance unchanged.
- PinHeader: 112 circular 2.00 mm pads, 1.10 mm plated drills, existing microSD window, nominal 14 x 8-pin 2.54 mm header model. Assembly gap 5.94 mm yields 1.00 mm nominal protrusion with 6.00 mm exposed pins, 2.54 mm plastic and 1.60 mm uaBrain; requires an assembly fixture.
- Castellated: 112 original 6 x 2 mm surface pads, no pin holes; 121.4 x 78.4 mm R2 internal window. Minimum nominal copper/window envelope clearance 0.544 mm.
- Validation: both load in KiCad with one closed cutout; all header centres match source within 0.001 mm; current assembly has no geometry below board intersecting retained rim. Inspected pad/cutout comparison. Whole-carrier integration and physical assembly fit remain pending.
- Fresh remotely assembled uaBrain 3D is deliberately pending; documented variant-specific placement heights. No commit/push.


## 2026-09-28 - Place both mounting variants on E38 adapter

- Verified all 232 original footprint text objects (front/back legends, including SDA/SCL) remain unchanged in both variants.
- Set all three M1 schematic units to hw-uaBrain-PinHeader; replaced the placed M1 with that footprint at its original position/orientation. Preserved instance UUID, schematic path, pad UUIDs, all 112 net assignments and user fields.
- Added REF_CASTELLATED to the right of all existing placement as an unconnected, board-only comparison footprint, excluded from BOM and position output. Its Edge.Cuts are real geometry; remove the comparison instance before producing final fabrication files.
- Validated native KiCad loading and semantic comparisons: no unrelated board objects or schematic properties changed. No commit/push. Fresh remote uaBrain PCB model remains pending.


## 2026-09-29 - Fresh assembly model and mounting footprint cleanup

- Attached fresh assembly VRML unchanged in both library checkouts and placed adapter footprints. Verified corrected WBO 24.6 x 14 mm and module mask Z=+0.0125/-1.6125. Model offsets: PinHeader 7.54 mm; Castellated 1.60 mm.
- Moved all 224 PinHeader contact legends to hole centre lines with 0.60 mm pad-to-text bounding-box gap; repositioned eight group headings. Preserved exact text, fonts, layers and justification.
- Removed assembly-note F.Fab text and inherited full-module rule-area keepouts from both library and placed footprints per user request. Routed cutouts retained.
- Removed obsolete hybrid footprint/model, after backup outside the repositories; updated library symbol defaults and embedded symbol footprint default. Updated README/mounting notes.
- Preserved freshly saved user PCB edits, all pads/nets and unrelated objects. Native KiCad load, isolated closed-cutout checks, text/font comparison and model SHA256 identity pass. No commit/push.


## 2026-09-29 - Tighten PinHeader group legends

- Moved DISCR, WBO1, WBO2 and 9924 on both silkscreens closer to their contact legends in both library copies and placed M1. Use only labels overlapping the group heading span when computing clearance, avoiding excessive offsets from longer labels elsewhere in a group. Bounding-box gap is 0.50 mm.
- Verified only eight group text positions changed; fonts, other labels, pads/nets, models and concurrent PCB edits remain unchanged. No commit/push.


## 2026-09-29 - Investigate assembled 3D loading cost

- Fresh model: 10,777,663 compressed bytes / 78,318,020 decoded bytes, 34,671 Shape declarations, 43,099 instantiated meshes and 1,123,773 triangles via fragment-wise parsing.
- Tried material batching into 76 mesh groups plus one line group without removing geometry. Test outputs kept in temp/uabrain-3d-optimized only; production footprint/model references unchanged.
- Native KiCad CLI 900 x 600 basic top renders: original uncached WRL 8.63 s total (7.64 s update), batched uncached WRL 9.67 s total (8.69 s update). This attempt did not improve loading. Initial cached original render was 3.57 s total. These are CLI measurements, not timings of the interactive 3D window.
- Compared original/batched renders visually. Compressed candidate loading emitted a temporary WRL sharing error; plain WRL benchmark avoided that error. Need actual geometry reduction (cosmetic copper/silkscreen/detail) or a native simplified STEP assembly for a materially lighter model. No footprint changes or commit/push.


## 2026-09-29 - Enlarge microSD finger-access opening

- Increased PinHeader microSD window from 34 x 22 R2 to 45 x 30 R3 in both library checkouts and placed M1. New local bounds X=37.925..82.925, Y=-41.275..-11.275. Added 7 mm on hinge/swing side (-X), 4 mm opposite, 7/1 mm along Y.
- Used Molex SD-47219-001 drawing and bottom-view assembly to identify hinge direction. Initial 56 x 36 proposal collided with current J1 body and J4/J5/J6 pads, so retained their placement and constrained the opening; nominal bbox gaps are 1.665 mm to J1 and 2.528 mm to J4/J5/J6. Physical hand-access validation remains pending.
- Verified closed cutout in KiCad and unchanged non-Edge.Cuts PCB objects. Castellated large opening, models, labels, pads and nets untouched. No commit/push.


## 2026-09-29 - Replicate U5 high-side support placement

- Matched U5 C22/D3/R36/R41/R46 to all four HS channels by footprint, value and per-pad nets; confirmed board nets against freshly exported schematic XML.
- Placed C23-C26, D4-D7, R37-R40, R42-R45 and R47-R50 using U5-relative geometry, rotated for unchanged U6-U9 driver orientations. Copied reference/value label placement, preserving pad numbers, nets and component identity.
- Native reload verifies transformed positions of every support-component pad within 0.000002 mm; parsed comparison verifies only placements/angles/field positions changed. All drivers and unrelated PCB objects unchanged. Routing remains absent; full-board DRC was not run. No commit/push.

## 2026-09-29 - Match LS support placement to hellen-gm-e38

- Used the regular U5 VNLD5160 two-channel cell in hellen-gm-e38 as the placement reference for adapter U13-U15. Moved R63-R74 to B.Cu with the same driver-relative spacing and reference/value label locations; driver positions and orientations remain unchanged.
- Matched input series and pull-down resistors against freshly exported source and target schematic netlists. Source series driver-side pin 1 corresponds to target pin 2, so series resistors receive the additional 180-degree rotation. Retained target 4.7k values rather than source 10k values.
- Native board reload confirms exact transformed pad locations (zero error), unchanged pad nets/UUIDs/sizes and label typography, and no same-side component bounding-box overlaps. Parsed comparison confirms all objects outside the 12 support footprints are unchanged. Board is not routed; full-board DRC was not run. No commit/push.

## 2026-09-29 - Two-layer routing of the prepared uaBrain E38 adapter

- Routed the user's saved placement on F.Cu/B.Cu and changed the board stack from four copper layers to two (1.6 mm total). Added GND pours on both sides. Preserved every original footprint, pad assignment, label and mechanical drawing, including the microSD opening.
- Derived routing netclasses from actual hellen-gm-e38 copper: default signals 0.15 mm, analog/WBO signals and GNDA 0.2 mm, LS/injector outputs 0.65 mm, HS/ETB outputs 1.0 mm, WBO heaters and bulk +12V_RAW 2.0 mm. Permanent/key supply routes use 0.5 mm. Through vias use 0.6/0.3, 0.8/0.4 and 1.2/0.6 mm diameter/drill.
- Completed routing with local VSS HS power connection and a 0.3 mm, 1.57 mm-long feedback branch to R39. Corrected undersized status-trace necks to 0.15 mm and moved the status bend by 0.03 mm to restore clearance. The router's temporary cutout obstacle was expanded by 0.15 mm to respect KiCad's 0.25 mm copper-to-edge clearance without changing the actual opening.
- Retained short pad neck-downs: +12V_RAW 0.601 mm over 2.511 mm total; OUT_VSS_HS 0.601 mm over 1.673 mm; OUT_MAIN_RELAY 0.601 mm over 1.571 mm; GND 0.225 mm over 4.703 mm. Bulk widths remain as listed above. This is a routing draft for review, not a manufacturing release.
- Final board has 2268 track segments and 275 through vias. Native KiCad 10 DRC with zone refill and schematic parity: zero unconnected items, zero schematic parity issues, and zero new DRC findings. All 103 original findings match their baseline type/item UUIDs: 50 silk-over-copper, 39 silk overlaps, 8 silk-edge clearances, 4 existing 0.2 mm U4 thermal-hole drill violations, and 2 library-footprint mismatches.
- Parsed comparison confirms all original footprints and drawing objects unchanged; native pad/placement checks pass. Project diff whitespace check passes. Backups, source-width mapping, router sessions, preview and DRC evidence are in C:/Work/rusefi/temp/e38-routing. No commit/push.

## 2026-09-29 - Ground stitching and routing cleanup

- Added 205 GND through vias (0.6 mm copper diameter, 0.3 mm drill): 72 around the perimeter, 31 near driver ground connections, 7 beside the microSD opening and 95 across available ground areas. Each via joins filled GND copper on both layers; avoided component pads and preserved all original vias. Used approximately 6 mm perimeter and 8 mm interior spacing, adjusted around existing copper and holes.
- Stitched previously removed ground islands to already connected copper. Retained the original isolated-island removal policy. Final filled GND area increased from 6084 to 7204 square mm on F.Cu and from 1996 to 6790 square mm on B.Cu.
- Simplified 36 track chains at unchanged widths, removing 66 segments and shortening routes by 16.10 mm. Final board has 2202 track segments and 480 vias. Footprints, labels, pad nets, mechanical drawings, stackup and project rules remain unchanged.
- Final native KiCad DRC after refill: zero unconnected items, zero schematic parity issues and zero new findings. The 103 pre-existing findings match the baseline by type and item UUID. Verified that every added stitch touches ground fill on both layers. Backups and checks are in C:/Work/rusefi/temp/e38-cleanup. No commit/push.

## 2026-09-29 - Connect remaining accessible ground islands

- Analyzed individual potential ground-fill components and their connections across layers, rather than using only a regular stitching mesh. Added another 147 through GND vias (0.6/0.3 mm) connecting reachable fill islands into the main ground network. Discarded trial vias that would only join floating islands to one another. Kept the original isolated-island removal setting.
- Removed 258 redundant GND track segments, totaling 670.7466 mm. Retained one original 0.3 mm-wide, 1.4508 mm-long link near C6/C8/C10 and D2 because removing it disconnects that local ground region. Preserved all other-net routing, original vias, footprints, pad nets, labels and mechanical geometry.
- Final refill and schematic-parity DRC: zero unconnected items, zero schematic parity issues, and the same 103 pre-existing findings by type/item UUID. Verified that every new via contacts filled ground on both layers. This does not claim all geometrically isolated slivers can be connected without changing signal routing. Backups and analysis are in C:/Work/rusefi/temp/e38-ground-complete. No commit/push.


## 2026-10-01 - uaBrain E38 routing after J7 placement (partial)

- Preserved the user's saved two-layer placement, board outline, cutout, footprint data and net assignments exactly. The primary PCB remains unchanged.
- Saved a separate review candidate: `uaBrain-gm-e38/uaBrain-gm-e38-rerouted.kicad_pcb` with matching routing-rule project files. J7 is included. Trace widths follow the hellen-gm-e38 reference mapping.
- Rebuilt routing, simplified trace chains, added GND pours and stitching, and removed redundant GND tracks.
- Final native KiCad refill, DRC and schematic parity: 124 pre-existing findings unchanged by type and item identity; zero new DRC findings; zero schematic parity issues. Two connections remain incomplete: `/EVAP_VENT` and `+12V_RAW`. This is not a completed routing result.
- The parity check used an identical temporary root schematic beside the candidate; that temporary copy was removed after checking. Use the original schematic as the source of truth.
- No commit or push. Current artifacts, source snapshots, routing attempts and detailed DRC results are in `C:/Work/rusefi/temp/e38-oct01`.


## 2026-10-02 - Complete routing in the original uaBrain E38 project

- Continued in `uaBrain-gm-e38/uaBrain-gm-e38.kicad_pcb`, as requested. Updated J7 pin 11 to `/OUT_EVAP_VENT` from the corrected schematic. Verified that this net connects J7.11, J1.61 and U15.8; `/EVAP_VENT` stays on the control side. The schematic was not modified.
- Completed both-layer routing while preserving the exact component placement, mechanical geometry, footprint properties and models. Reworked neighboring routes to connect J7 and the remaining raw-power branch. Main power routes retain the reference widths; the short U8 supply connection uses 0.5/0.65 mm where required by the available clearance (0.5 mm also exists on the reference +12V_RAW net).
- Restored and checked ground pours/stitching. Final board: 2563 trace segments, 740 vias including 404 GND vias. Removed 338 redundant GND segments; retained one necessary 0.3 mm long GND link.
- Final native KiCad refill + DRC + schematic parity on the PRIMARY file: 0 unconnected items, 0 schematic parity issues, and exactly the same 124 baseline DRC findings by type and item UUID. Also compared every mapped pad against the exported current schematic netlist and checked exact footprint/outline preservation except the requested J7.11 net change.
- Removed the previous separate rerouted project/board from the project directory, preserving those files under `C:/Work/rusefi/temp/e38-oct02/superseded-preview`. Original input snapshots and detailed results are under `C:/Work/rusefi/temp/e38-oct02`.
- No commit or push.


## 2026-10-05 - Via cleanup and schematic GNDA update

- Cleaned the original uaBrain-gm-e38 PCB. Removed redundant GND vias beside same-net through-hole pads, including the four around M1.P5. Reconnected J1.66 (OUT_SKIP_SHIFT_SOLENOID), J2.58 (IN_MAP), and J6.4 (OUT_DOD_CYL4) directly through their plated pads. Removed unnecessary via pairs on OUT_INJ8 and HS5/IN5 and simplified trace bends. This cleanup removed 29 vias and 35 segments before the subsequent schematic update.
- Applied the user's saved GND -> GNDA changes to all 40 analog-section pads. Added priority-1 GNDA pours on both copper layers and reassigned local stitching. Restored separate GND and GNDA connectivity after the split; removed six now-isolated stitches. New vias avoid SMD pad copper by at least 0.1 mm and remain more than 3.2 mm from same-net through-hole pad centers. Synchronized the five hidden Description fields on C22-C26 with the current schematic.
- Final board has 2539 segments and 715 vias (344 GND and 45 GNDA), a net reduction of 24 segments and 25 vias from the session input. All cleaned signal/power traces and widths are preserved through the GNDA update. Added GNDA links use 0.2 mm and local control-ground links use 0.3 mm; the remaining ground-plane joins use stitching vias.
- Final native KiCad refill, DRC and schematic parity on the PRIMARY board: 0 unconnected items, 0 schematic parity issues, and exactly the same 151 baseline findings by type/item UUID (124 existing board findings plus 27 library-resolution findings in this CLI environment). Fresh XML netlist comparison checks all 655 mapped pads. Parsed validation proves exact preservation of placement, footprint geometry, visible labels, stackup, outline, microSD opening and models, except the 40 requested pad-net changes and five hidden description fields.
- Before/after previews, snapshots, validation scripts and final DRC evidence are in C:/Work/rusefi/temp/e38-oct05. Existing DRC issues remain outside this routing cleanup. No commit or push.


## 2026-10-05 - Resolve remaining uaBrain E38 DRC findings

- Corrected all 124 physical/library-mismatch findings and made the five used standard footprint libraries explicit in the project table, eliminating another 27 configuration-only findings. Final native KiCad DRC on the primary PCB, with zone refill, schematic parity and an empty user configuration: 0 violations, 0 unconnected items, 0 schematic parity issues. The project rules, severity settings and existing ignored-check list remain unchanged.
- Aligned all 12 LS resistor references R63-R74 beside their own components, preserving font size, thickness and orientation. Moved the two 9924 group labels inside the board and placed KNOCK2 on the inner side of its pad row on both sides. Trimmed the M1 bottom silkscreen outline over the lower connector row, retaining the two end portions on each side. Visually checked exact KiCad text/shape renders before and after.
- Changed four U4 exposed-pad thermal holes from 0.2 to 0.3 mm and copper diameter from 0.5 to 0.6 mm. Moved their local centers from +/-0.59 to +/-0.54 mm on each axis so their copper stays inside the 1.68 mm exposed-pad outline and clears the nearby ADC3 trace. Annular ring is 0.15 mm; thermal-hole pitch is 1.08 mm. External pin geometry, solder mask and the four existing paste windows are unchanged. The opposite-side exposed-pad copper remains mask-covered. TI SLUA271C section 3.4 gives 0.3 mm as a manufacturing starting point and discusses solder loss through vias; ADS7128's example drawing uses 0.2 mm. Reference: https://www.ti.com/lit/an/slua271b/slua271b.pdf and https://www.ti.com/lit/ds/symlink/ads7128.pdf .
- Saved four project-specific footprints in uaBrain-gm-e38/e38-local.pretty: M1 with this carrier's silkscreen, U4 with the revised thermal holes, and exact existing J1/J2 board variants. Updated their schematic footprint assignments and PCB library IDs. Common library/submodule footprints are unchanged. This removes library mismatch warnings without replacing the user's existing connector geometry.
- Parsed preservation check proves exact unchanged routing, net assignments, component placement, board outline/cutout, zone parameters, 3D models and other footprint data outside the listed edits. Current PCB remains 2539 segments and 715 vias. Original snapshots, before/after renders, checks and the final DRC report are in C:/Work/rusefi/temp/e38-drc-oct05. No commit or push.


## 2026-10-06 - Keep E38 footprint edits in board instances

- Restored original footprint library IDs for M1, J1, J2 and U4 in the primary PCB and corresponding schematic fields. M1 and both Molex connectors reference kicad6-libraries again; U4 references the standard Package_DFN_QFN thermal-via footprint. Added that standard library to the project footprint table.
- Removed e38-local from the footprint table and moved its four footprint files out of the project into C:/Work/rusefi/temp/e38-footprint-links-oct06/removed-e38-local.pretty. Existing customizations remain embedded in the PCB, including labels and U4 thermal-hole geometry. Shared library footprints were not changed.
- Exact byte comparison after the requested ID substitutions confirms unchanged footprint geometry, placement, labels, copper, net assignments, board outline/cutout and 3D models. Project rules and ignored-check settings are unchanged.
- Native KiCad 10 DRC with zone refill and schematic parity on the primary board reports only four expected lib_footprint_mismatch warnings for M1/J1/J2/U4, zero unconnected items and zero schematic parity issues. These intentional board-instance differences remain visible; no new exclusions were added.
- Backups, mapping, preservation validation and DRC evidence are in C:/Work/rusefi/temp/e38-footprint-links-oct06. No commit or push.


## 2026-10-07 - Connector signal labels and uaBrain text edge margins

- Added 126 signal labels to the placed J1/J2 automotive connector footprints: J1 has 24 front and 17 back labels; J2 has 43 front and 42 back labels. Names come from current pad nets, with readable abbreviations for longer switch/relay functions. Unconnected pins and locations lacking clear space are left unlabelled. Both-side labels are aligned to their pad rows; backside text is mirrored for normal reading from the back.
- New text uses 1.0 mm KiCad stroke lettering and 0.15 mm strokes, matching the existing connector pin-number size. Placement avoids pads, existing labels, silkscreen outlines and board edges. Original pin numbering remains intact.
- Moved the uaBrain upper label row 0.6 mm inward and the left-side label row 0.25 mm inward on both sides, preserving every font, text string and rotation. The minimum rendered-stroke distance to the board outline increased from 0.163 mm to 0.763 mm. The other module labels already have enough edge clearance.
- Parsed before/after comparison confirms that only 162 module text positions and 126 new connector text items changed. Copper, pad geometry/net assignments, component placement, outline/cutout, 3D models, schematic, project rules and libraries are unchanged. Changes remain embedded in the original PCB footprint instances.
- Native KiCad DRC with zone refill and schematic parity on the primary board: the same four expected library-footprint mismatch warnings, zero unconnected items, zero schematic parity issues, no new violations or exclusions. Visually checked both-side previews and detailed connector views.
- Backups, net-to-label mapping, previews, preservation validation and final DRC are in C:/Work/rusefi/temp/e38-labels-oct07. No commit or push.


## 2026-10-07 - Hellen-style schematic buses

- Added functional color buses on the primary uaBrain-gm-e38 schematic using the exact hellen-gm-e38 palette and default stroke widths: blue ignition, red injection, orange outputs, magenta analog sensors, yellow-green discrete inputs, teal CAN and the default general-purpose bus for WBO/I2C/control signals. Added a matching dashed Bus types legend.
- Connected labelled stubs at J1-J7, all three uaBrain units, ADC/HS/LS sheets and local control/pull-up circuits using 205 diagonal bus entries. Retained the reference design's 2.54 mm diagonal entries and used 3.81 mm spacing between parallel buses so wire endpoints do not land on neighboring buses. Completed ten previously open WBO entries. ADC-connected oil-level and BK1S1 signals follow the analog group on this board.
- Kept all symbols, component positions/properties, power symbols, hierarchy and existing fonts unchanged. Aligned the 68 uaBrain signal labels at their pin ends to clear the bus fan-out. Existing signal names are unchanged; no named bus aliases or net-prefix changes were introduced.
- Native XML netlist comparison on the installed primary schematic confirms all 218 nets and 654 pin nodes exactly match the input. Component data also matches. ERC dropped from 29 to 19 findings: ten unconnected-wire-endpoint findings resolved, no new findings; the remaining 15 symbol-library and four footprint-link findings are unchanged. Rules and exclusions are unchanged.
- Visually checked the full native SVG export and enlarged uaBrain, driver/ADC, connector and legend views. The PCB was saved independently during this task and was never written by this task. Installed only the root schematic; child sheets and library tables are unchanged.
- Backups, previews, change inventory, preservation validation, netlists and ERC evidence are in C:/Work/rusefi/temp/e38-buses-oct07. No commit or push.


## 2026-10-07 - Remove ambiguous schematic bus-entry contacts

- Audited all 205 bus entries after the user's screenshots exposed visually ambiguous joins that ERC did not flag. Found 24 entry-to-bus endpoints coinciding with unrelated signal wires.
- Adjusted 30 entries and the associated 26 wire-stub endpoints. Retained 45-degree entries, using half-pitch entries or reversing their direction where needed. Added two short colored bus branches for the crowded opposing LS fan-outs. Trimmed/rebuilt vertical bus spans to their actual connection points, removing bus ends that visually landed on unrelated wires.
- Geometric verification checks every entry against all unrelated wires and every other entry: zero foreign-wire contacts and zero crossing diagonals remain. Inspected native before/after schematic renders, including both screenshot locations and ADC input fan-outs.
- Parsed validation confirms unchanged symbols, labels, typography, power symbols, hierarchy and existing non-bus drawings. Native exports from the installed primary schematic retain all 218 original nets exactly; ERC retains the same 19 pre-existing library findings with no new findings. No rule or exclusion changes. PCB and child sheets were not edited.
- Backups, geometry audit, before/after previews, netlists and ERC evidence are in C:/Work/rusefi/temp/e38-bus-cleanup-oct07. No commit or push.

## 2026-10-07 - Separate functional buses and match Hellen grid-offset fan-outs

- Replaced the mixed general-purpose bus with a WBO-only trunk for WBO1/WBO2, a short local I2C bus connecting uaBrain/ADC/PCA9685, and two named LS_CTRL sections for STARTER_RELAY, CHECK_ENGINE, EVAP_VENT and FAN1_RELAY. CAN retains its own teal trunk. Added matching legend rows and functional headings for the direct uaBrain and HS PWM control wires.
- Inspected the saved GM E38, E67, Hyundai and other Hellen schematics. The reference GM E38 CAN_H branch uses a 1.27 mm vertical wire jog with a normal 2.54 mm diagonal entry. Applied this technique to 24 conflicting fan-outs; all 205 entries now use 2.54 mm diagonals. Removed the previous two short colored bus branches and avoided wire bends landing on or overlapping another bus.
- Full geometric audit reports zero foreign-wire contacts and zero crossing bus entries. Visually checked native KiCad SVG renders of the whole page and the ignition, opposing LS/analog, ADC, I2C, control and legend regions.
- Native exports from the installed primary schematic confirm unchanged 218 nets, 654 component-pin nodes, component properties and all 207 original signal labels. Existing symbols, hierarchy and typography are preserved. ERC retains exactly the same 19 pre-existing library findings, with no new violations or rule/exclusion changes. Only the root schematic was installed; PCB and child sheets were not edited.
- Backups, reference examples, previews, geometry checks, preservation validation, netlists and ERC evidence are in C:/Work/rusefi/temp/e38-functional-buses-oct07. No commit or push.

## 2026-10-07 - BMW N52 pinout terminology and reference audit

- Standardized both BMW-N52-146 vehicle YAML files against the pinout naming conventions, keeping connector images and coordinates unchanged. Corrected the 1-20 key, the reference anti-theft terminal (2-21), sensor grounds, oxygen-sensor heater functions and injector types.
- Kept the established pedal channel numbering and identified stock ignition primary controls as requiring power ignition drivers. Left interface types unspecified where the reference does not settle the electrical interface.
- Validated YAML parsing, unique pins, image coordinates, paired descriptions/types, 119 adapter connections and rendered interactive pages. No PCB or schematic changes. Only the two BMW YAML files are included in this task's hardware commit; other working-tree changes are preserved.


## 2026-10-07 - Continuous schematic buses without free tails

- Joined the two LS control bus sections with one continuous purple route around the ADC block. Removed the LS_CTRL alias and duplicate endpoint labels, replacing them with a single functional heading. Removed both former LS label tails and the unused I2C heading tail.
- Graph validation confirms all nine functional buses each form one connected component and have no free endpoints beyond their outermost entries. Legend samples are excluded from the connectivity check. Visually checked the ADC/PWM and LS control regions.
- Installed only the root uaBrain-gm-e38 schematic. All 400 signal-wire segments, 205 bus entries, symbols, original signal labels, typography and child sheets are preserved. Fresh native exports retain all 218 nets and 654 pin nodes; ERC retains the same 19 pre-existing library findings. No PCB, project settings or library files were edited.
- Backups, previews and validation are in C:/Work/rusefi/temp/e38-continuous-buses-oct07. No commit or push.


## 2026-10-07 - Verify revised E38 outline and widen analog/logic power rails

- Checked the user's saved 149 x 104 mm outline, rounded corners, four mounting holes and internal microSD cutout. The contour was valid; copper-to-edge checks passed. Initial refill exposed one disconnected GND region, ten back-silkscreen connector body lines crossing the new bottom edge, and four missing MountingHole library entries.
- Routed GNDA, +5VA, +5VP, +3.3V, +3.3VA and +3V3 at 0.5 mm where possible, with 0.4 mm passages and short narrower pad neckdowns explicitly allowed by the user. Every track below 0.4 mm is confined to its own pad neighborhood (within 1 mm of pad copper). Set all six nets' existing project netclass assignments to Route_0p5; retained all clearance rules and exclusions.
- Relocated obstructing signal sections, restored GND/GNDA continuity after refills, removed stranded stitching vias and unused trace branches. Clipped only the offending J3-J7 silkscreen body lines to the revised lower edge and added the standard MountingHole library reference. No separate footprint library was created.
- Final geometric comparison confirms the user's exact Edge.Cuts, footprint positions, rotations, pad geometry and pad net mapping are preserved. Preserved 2510 original non-target copper items; the remaining local signal/ground changes are audited in the temporary workspace.
- Native DRC on the installed main project, with zone refill, all-track checking and schematic parity: zero errors, zero unconnected items and zero parity issues. The only nine warnings are intentional embedded footprint differences: M1, J1, J2, U4, and J3-J7 after clipping their body silk. No dangling tracks or vias remain.
- Retained the schematic and project edits made concurrently during this work; merged only the six netclass assignments into the latest project file. Inspected the final two-sided copper preview and validated saved-file widths and preservation again after installation.

| Net | 0.5 mm length, mm | 0.4 to <0.5 mm length, mm | Short pad neckdowns, mm |
| --- | ---: | ---: | ---: |
| +3.3V | 58.17 | 1.30 | 0.00 |
| +3.3VA | 142.37 | 70.15 | 0.10 |
| +3V3 | 72.47 | 23.22 | 0.00 |
| +5VA | 237.87 | 20.58 | 1.20 |
| +5VP | 382.63 | 20.17 | 0.00 |
| GNDA | 704.43 | 47.96 | 1.82 |

- Backups, routing/cleanup audit, width validation, final preview and installed DRC are in C:/Work/rusefi/temp/e38-power-widths-oct07. Installed only the main PCB, project netclass settings and fp-lib-table. No commit or push.

## 2026-10-07 - Smooth E38 routing throughout the board

- Simplified 165 track chains on both copper layers, replacing 2076 segments with 751 and shortening total routing by 29.963 mm. Removed the GNDA loop at M1 N4 and the neighboring signal stair steps shown by the user; applied the same cleanup to redundant jogs and collinear pieces throughout the board.
- Kept 0.5 mm power routing where the selected paths permit, with 0.4 mm constrained passages and the previously allowed short pad neckdowns. Verified that every power segment below 0.4 mm remains within 1 mm of same-net pad copper. Retained a narrow GND connection near D7/R40/C26 by moving the adjacent +3V3 run left and reducing it to five clean segments.
- Exact parsed comparison confirms unchanged footprints, pad geometry and net mapping, placement, Edge.Cuts, text, graphics, zone boundaries and board settings. All 678 vias are unchanged. Every modified track is included in the saved cleanup audit. Installed only the PCB after checking the original file hash and editor state; retained schematic, project and library files.
- Native DRC on the installed project with zone refill, all-track checking and schematic parity reports zero errors, zero unconnected items and zero parity issues. The same nine intentional embedded-footprint warnings remain. Inspected the before/after detail and both copper layers.
- Backups, track audit, width validation, previews and installed DRC are in C:/Work/rusefi/temp/e38-track-cleanup-oct07. No commit or push.

## 2026-10-07 - Review the purpose of the E38 GNDA pours

- Inspected the saved adapter and fresh adapter/uaBrain schematic netlists. GNDA has two local pours around the ADC and op-amp region: 448.86 mm2 on F.Cu and 280.14 mm2 on B.Cu; it is not a whole-board plane. The adapter keeps GNDA and GND as separate nets.
- Removed only the two GNDA zones in a temporary copy and ran native DRC with refill and schematic parity. This produced 12 missing connections, including actual op-amp/filter ground connections as well as a stitching via, plus four dangling tracks and five dangling vias. Therefore deleting the pours requires routing repair; the count alone does not establish that a pour is the only viable grounding topology.
- Checked ADS7128 datasheet section 11.1: TI recommends a short, low-impedance connection to a ground plane and close decoupling. Inspected sheet 12 of the installed mega-mcu100-f7/0.3 module schematic: it shows the GND/GNDA link inside the MCU module. Recommendation is to retain a compact local analog pour, with its extent determined by analog return paths; do not add an arbitrary second GND/GNDA link on the adapter.
- The production schematic, PCB, project settings and libraries were not changed. Audit, netlists and the temporary no-GNDA experiment are in C:/Work/rusefi/temp/e38-gnda-review-oct07.

## 2026-10-07 - Route E38 GNDA without pours and reinforce 12 V transitions

- Removed both GNDA copper zones and connected all remaining analog-ground pads with traces. Retained 0.5 mm preferred / 0.4 mm minimum GNDA routing, with the existing short pad neckdowns. Removed the now-unused pour-only via/track branch and smoothed the new routes. GNDA remains separate from GND.
- Checked the reference hellen-gm-e38 PCB and fresh uaBrain netlist. The carrier +12V connection J1.47 -> M1.M1 feeds the module power fuses and motor drivers, rather than being only a voltage-sense input. Expanded its main sections to 0.9-1.0 mm, retaining short 0.5-0.8 mm constrained sections. A full 2 mm trial required excessive signal rip-up and was discarded; the installed result uses local adjustments only. Set +12V to the existing Route_1p0 netclass without changing clearance rules or exclusions.
- Retained the 2 mm +12V_RAW main bus; widened accessible portions of the U8 supply branch to 0.8-1.0 mm, with 0.5/0.65 mm local necks. 12V_KEY is 0.5 mm and has no layer transitions.
- Increased power vias from 5 to 13: the +12V_RAW main transition now has four parallel vias, and each end of its U8 bridge has three. The +12V connector-side transition has two 0.4 mm drills; the tight uaBrain-side transition uses a 0.9 mm pad / 0.6 mm drill in place of 0.8/0.4 mm. Locally moved CAN2+, clutch input, tach, ignition-key and GND stitching geometry to preserve clearance. Kept all other signal routing unchanged.
- Exact parsed comparison confirms unchanged footprints, pad geometry/net mapping, component positions, outline, silkscreen and remaining zone boundaries. Preserved 3359 original copper items. All six previously widened analog/logic rails retain >=0.4 mm routing outside their own pad neckdowns.
- Native KiCad DRC on the installed main project with refill, all-track checking and schematic parity: zero errors, zero unconnected items, zero parity issues, and the same nine intentional embedded-footprint mismatch warnings. No dangling tracks or vias remain. Inspected both copper layers and detailed power-transition previews.
- Installed only the PCB and +12V netclass assignment after checking all saved source hashes and editor state. Schematic and library files were preserved. This is a geometry/connectivity review against the reference, not a certified current rating: actual load, temperature rise and manufactured via plating still determine the permissible current.
- Backups, full copper-change audit, width/via inventory, previews and installed DRC are in C:/Work/rusefi/temp/e38-gnda-12v-oct07. No commit or push.

## 2026-10-07 - Recover GND pours with stitching and review E38 order readiness

- Added 121 GND-only through vias, 0.6 mm diameter / 0.3 mm drill, to join recoverable copper islands to the existing connected ground network. Chose overlaps between native potential fills on both layers, avoiding foreign copper, holes and redundant vias next to same-net PTH pads. Retained the original island-removal settings; no floating copper was retained to improve the visual fill.
- Actual connected GND fill increased from 5396.54 to 6831.13 mm2 on F.Cu (+1434.59), and from 4595.52 to 6233.08 mm2 on B.Cu (+1637.56). No previously filled area was lost. Pockets without a valid two-layer via position remain unfilled rather than violating clearance or adding disconnected copper.
- Exact comparison confirms every original track and via is unchanged; the only copper additions are the 121 audited GND vias. Footprints, placement, pad mapping, outline, text, GNDA routing and zone definitions/settings are unchanged. GNDA still has no copper zones. Installed only the PCB after confirming all source hashes and editor state.
- Native DRC on the installed primary board, with refill, all-track checks and schematic parity: zero errors, zero unconnected items and zero parity issues; the same nine embedded-footprint mismatch warnings remain. Native ERC in an isolated audit copy with the installed standard symbol libraries linked reports zero violations; the unmodified minimal headless library configuration alone produced 15 missing-library warnings.
- Exported all nine manufacturing Gerber layers plus separate PTH/NPTH Excellon files for review. Drill report contains 1157 plated and 9 unplated holes. These are temporary inspection outputs, not a released order package. Inspected the two-sided filled-copper preview.
- Order readiness is not unconditionally approved. An isolated audit enabled the project's ignored DRC categories without changing the production project's checks. It found 15 starved-thermal warnings (16 before stitching), 87 missing courtyards, 35 off-center track/via endpoints and the same nine embedded-footprint differences; the three extra schematic-parity warnings are symbol footprint-filter mismatches at D1/D2/U4, not missing electrical connections. The thermal warnings require review of the real return paths at M1 C3/C8/G8/K3/K8/P5, R38.1, R44.1, U14.7, U15.7, J2.60 and J6.1. Courtyard omissions limit automated mechanical collision coverage. Existing disabled categories remain untouched.
- Supply current/temperature limits, final assembled module/connector/microSD clearances, and the actual fabricator's rules still need release sign-off. The file specifies two copper layers, 1.6 mm board thickness and 35 um copper. Do not assume that selecting 70 um copper at order time preserves manufacturability: JLCPCB currently specifies 0.16/0.16 mm minimum track/space for 2-layer 2 oz, while this design uses 0.15 mm signals/clearance. Source: https://jlcpcb.com/capabilities/Capabilities . The ADC grounding/decoupling recommendation was checked in https://www.ti.com/lit/ds/symlink/ads7128.pdf ; trace-only GNDA performance still needs bench validation with switching loads.
- Evidence, preservation/coverage audit, extended-before/after DRC, ERC, temporary Gerber/drill outputs and preview: C:/Work/rusefi/temp/e38-gnd-preorder-oct07. No commit or push.

## 2026-10-07 - Label every angled Molex pin and normalize reference orientation

- Added 96 pin-function labels: all 48 contacts of J3/J4/J5/J6/J7 on both silkscreen layers. Names follow the current saved schematic and pad nets, including OUT_EVAP_VENT, WBO1/WBO2, CAN2, EGT, DOD, clutch, fuel-level and auxiliary signals. Short names fit in aligned horizontal rows immediately above the corresponding pads, using 0.82 mm text and 0.12 mm strokes. Added connector references on the front and WBO1/WBO2/CAN2 identifiers on both sides.
- Adjusted 44 existing reference fields, including 38 stored-angle corrections. Horizontal references read left to right; all vertical references read bottom to top when viewed from their respective board side. Preserved the existing fonts, text sizes and stroke widths. Restored U8's off-board reference beside its package, moved the five Molex references clear of pin labels, and shifted R20-R27 references 0.12 mm away from pad openings.
- Removed 48 decorative Molex pin-leg silkscreen segments that crossed the new names. Retained package outlines and pin-1 markers. These edits remain embedded in the project PCB; shared library footprints were not changed.
- Checked actual rendered text strokes and mirrored back-side previews. Added/edited silkscreen has at least 0.168 mm clearance to pad copper expanded by 0.05 mm, 0.160 mm to neighboring silkscreen, and 0.597 mm to the board edge. Verified all 96 labels against a fresh schematic XML netlist, and all 113 visible reference fields for consistent effective drawing angles and mirroring.
- Exact parsed comparison confirms unchanged copper, zone fills, pad mapping/geometry, component placement, board outline and all unrelated content. Original reference font settings are preserved. Installed only the PCB after checking source hashes and editor state. Native installed-board DRC with refill, all-track checking and schematic parity reports zero errors, zero unconnected items, zero parity issues and the same nine intentional embedded-footprint warnings.
- Backups, label-to-net inventory, preservation/clearance audit, two-sided previews and installed DRC: C:/Work/rusefi/temp/e38-molex-labels-oct07. No commit or push.

## 2026-10-07 - Enlarge new Molex pin labels for JLCPCB silkscreen

- Corrected all 96 recently added pin-function labels on J3-J7 from 0.82 mm text / 0.12 mm stroke to 1.0 x 1.0 mm text / 0.15 mm stroke. This addresses the published JLCPCB standard limits (https://jlcpcb.com/capabilities/pcb-capabilities/). The previous native DRC pass did not establish compliance with these manufacturing text limits.
- Retained all 48 pin functions on both sides, and spread long names along their respective rows with at least 0.5 mm between labels. Every text center remains nearest its own pad column (maximum horizontal offset 1.665 mm, below half of the 4.2 mm pitch). Changed P/N to P-N to keep the slash's oversized vertical extent out of the pad clearance. Names and pin nets were checked against a fresh schematic XML export.
- Cut small gaps in J1's decorative back-side package outline behind five enlarged J4/J5 labels. Retained the remaining straight strokes and native circular arcs. Actual stroked-label geometry has minimum 0.224 mm clearance to pad copper expanded by 0.05 mm, 0.242 mm to other silk, and 1.633 mm to the board edge. Inspected both mirrored back-side and front-side previews.
- Exact parsed comparison confirms unchanged tracks, vias, copper fills, pads, nets, footprint placement, board outline, reference designators and all other texts. Shared libraries, schematic and project rules were preserved. This change updates the new Molex labels only; it is not a blanket certification of all pre-existing silkscreen on the board.
- Installed only the main PCB after source-hash and editor-state checks. Native DRC on the installed board with refill, all-track checking and schematic parity reports zero errors, zero unconnected items, zero parity issues and the same nine intentional embedded-footprint warnings. The installed file matches the validated candidate exactly.
- Backups, label inventory, geometry/preservation validation, previews and DRC: C:/Work/rusefi/temp/e38-molex-jlc-oct07. No commit or push.

## 2026-10-07 - Restore original E38 ADC input bias

- Compared all eight ADS7128 inputs with hellen-gm-e38 revision C, tracing the original mega-mcu144 module 0.7 schematic and effective bom_pullups_hellen-gm-e38-c.csv overrides. Included the main-sheet excitation resistors: carrier R1/R2 already match original R3/R71 (100 ohm, C24920, 2512), so their footprints and routing required no change.

| Input | Carrier pullup to +5VA | Carrier pulldown to GNDA | Original module pullup/pulldown refs |
| --- | --- | --- | --- |
| ATF_TEMP | R4: 4.7k | R5: DNP | R212 / R216 |
| AC_PRESSURE | R8: DNP | R9: 680k | R210 / R214 |
| FUEL_TANK_PRESSURE | R12: DNP | R13: 680k | R230 / R234 |
| CLUTCH_POSITION | R16: DNP | R17: 680k | R239 / R243 |
| PRIM_FUEL_LEVEL | R1: 100 ohm; R6: DNP | R7: 680k | R222 / R226; external R3 |
| SEC_FUEL_LEVEL | R2: 100 ohm; R10: DNP | R11: 680k | R220 / R224; external R71 |
| OIL_LEVEL_SWITCH | R14: 10k | R15: DNP | R237 / R241 |
| BK1S1SIG | R18: 4.7k | R19: DNP | R132 / R138 |

- Updated 11 resistor values and matching LCSC fields in schematic and PCB. Used existing 0603 C23162 for 4.7k, C15401 for 10k, and verified 0603 C25822 for 680k (https://www.lcsc.com/product-detail/C25822.html). Marked all eight unpopulated bias options as DNP in both files and cleared their ordering codes; value text alone previously left these components marked as fitted.
- Retained the explicitly requested premium-quick-test ADC0 buffer/filter topology and all filter values. The original module uses different input filters and a Schmitt receiver for BK1S1; this task restores its input bias, not those alternative signal paths. Updated the ADC README section to match current U2/U3/U4 references, connector numbering, +3.3VA supply and actual fitted bias.
- Exact text-preservation checks allow only Value/LCSC/DNP edits in the 16 bias component instances. Fresh before/after XML exports prove unchanged net nodes and all other component properties; native PCB loading verifies fitted/DNP states, values, ordering codes and pin nets. Copper, fills, pad geometry, placement, outline, text geometry/fonts and project checks are unchanged. Visually inspected the exported ADC sheet.
- Candidate ERC: zero violations under existing project rules. DRC with refill, all-track checking and schematic parity: zero errors, zero unconnected items, zero parity issues; the same nine pre-existing embedded-footprint warnings. Source hashes and editor state were checked before installation, and installed files were verified against the validated candidate.
- Evidence and backups: C:/Work/rusefi/temp/e38-adc-pullups-oct07. No commit or push.

## 2026-10-07 - Prepare missing Molex models for the standard KiCad directory

- J3-J7 already reference the correct standard KiCad 10 filenames for Molex Mini-Fit Jr 5569-04A2, 5569-06A2 and 5569-16A2. All three files are absent from the local library and the inspected official Packages3D Connector_Molex catalog.
- Prepared STEP solids from a published manufacturer model for 039300040 and existing uaefi manufacturer models 039301060/039301160. The latter are the 94V-2 material variants of the same 5569-A2 housing; this is a mechanical visualization, not a change to ordering codes. Source paths, URLs, hashes and exact rigid transforms are recorded in C:/Work/rusefi/temp/e38-molex-3d-oct07/README.txt and model-validation.json.
- Transformed the models themselves for zero-offset, zero-rotation, unit-scale use in the existing footprints. Re-imported all three STEP outputs and checked solid validity. Matched all 48 actual J3-J7 terminal centers and all plastic-lock axes to their PCB holes. Inspected a native KiCad 10 render of all five placed connectors.
- Installation into C:/Program Files/KiCad/10.0/share/kicad/3dmodels/Connector_Molex.3dshapes remains PENDING: ordinary copy was denied by Windows ACLs; the elevated copy request ended with Windows reporting that the user cancelled the operation. No models were installed, no Windows permissions/security settings were changed, and all project PCB/schematic/footprint/settings hashes remain unchanged. Prepared files remain in temp/e38-molex-3d-oct07/models/Connector_Molex.3dshapes. Await user confirmation before retrying administrator installation.
- No commit or push.

## 2026-10-07 - Complete Molex model installation after UAC confirmation

- After explicit user confirmation, copied the three prepared 5569-A2 STEP models into C:/Program Files/KiCad/10.0/share/kicad/3dmodels/Connector_Molex.3dshapes using the standard Windows administrator prompt. All installed SHA-256 hashes match the validated files. This completes the pending installation recorded above.
- Ran native KiCad 10 rendering without a KICAD10_3DMODEL_DIR override and inspected installed-models.png: all five Molex J3-J7 automatically resolve their existing model references and render correctly. Existing component model offsets, rotations and scales remain unchanged. Project PCB/schematic/library-table/settings hashes are unchanged across installation. No commit or push.
- Evidence: C:/Work/rusefi/temp/e38-molex-3d-oct07/installation-status.json, installed-render.log and installed-models.png.

## 2026-10-08 - Prototype PCB order preflight and manufacturing export

- Reviewed the current saved uaBrain-gm-e38, including disabled DRC/ERC categories in isolated audit copies. Preserved schematic, project settings, all existing copper, placement, pad and hole geometry, and board outline. GNDA remains trace-only. Source hashes and closed editor state were checked before installing the validated PCB.
- Improved 11 pad connections: rotated thermals on M1 C3/C8/G8/K3/K8/P5, U15.7 and J6.1; adjusted R44.1 relief gap/spokes; made U14.7 a solid GND return; routed J2.60 to J2.79 with 0.65 mm copper instead of isolated thermal islands. Normal DRC after refill: 0 errors, 0 opens, 0 parity issues, 9 existing local-footprint warnings. ERC: 0 under existing settings.
- Extended DRC reduced starved thermals from 15 to 2. Remaining R38.1 is a 4.7k logic pulldown with one 0.25 mm spoke; M1.C8 connects through two top spokes while its bottom island is redundant. Other audit warnings: 87 missing courtyards, 35 off-center via endpoints, 9 local footprint differences and 3 stale symbol footprint filters. Verified ADS7128 and SRV05-4 supply/pin mapping. Extended ERC: 89 graphical bus membership, 13 four-way junction, 11 missing power flags, 3 footprint-filter warnings; no pin-to-pin conflicts. Reports retained, production severity settings unchanged.
- Exported 9 Gerber layers and separate PTH/NPTH Excellon. Independently parsed the outputs: 149 x 104 mm board; closed 45 x 30 mm microSD opening with 3 mm corner radii; 1157 plated and 9 nonplated holes, all matching PCB coordinates/diameters within 1 um output rounding. Inspected copper/silk previews and both assembly sides in native 3D with resolved models.
- Release: uaBrain-gm-e38/production/2026-10-08. PCB-only ZIP is the fabrication upload; full ZIP is production/uaBrain-gm-e38-2026-10-08-full.zip. Includes schematic PDF, assembly PDFs, fitted/all-option BOMs, native positions, SHA-256 manifest, checks and ordering instructions. BOM has 108 fitted components and excludes 8 native DNP options; 112 native position rows include 4 mechanical holes. Assembler part availability and library rotation conventions remain for online assembly preview.
- Recommended release use: first prototype batch, FR-4, 2 layers, 1.6 mm, 1 oz copper. Bench load/thermal tests and physical enclosure/harness fit are not replaced by CAD checks. No order, payment, commit or push. Backups and verification scripts: C:/Work/rusefi/temp/e38-release-oct07.

## 2026-10-08 - Simplify the order handoff

- Per user request, flattened uaBrain-gm-e38/production to four files: uaBrain-gm-e38-PCB.zip, BOM.csv, positions.csv, README.txt. Removed dated nesting, duplicate raw Gerbers, full archive, previews and audit reports from the order directory; retained them in C:/Work/rusefi/temp/e38-release-oct07/removed-release-clutter.
- Fabrication ZIP and fitted BOM remain byte-identical. Filtered the four mechanical mounting-hole rows from the native positions export; all 108 fitted BOM references remain with unchanged coordinates, angles and sides. Short Russian README contains order settings and assembly preview requirements. PCB, schematic and project settings untouched. No commit or push.

## 2026-10-08 - Fix JLCPCB placement-file column headers

- User reported JLCPCB file-processing failure for positions.csv. The previous handoff incorrectly retained KiCad-native Ref/PosX/PosY/Rot/Side headers. Converted the existing file in place to the documented JLCPCB Designator/Mid X/Mid Y/Rotation/Layer format, with Top/Bottom values, ASCII encoding and CRLF line endings. Removed unused value/package columns.
- Verified all 108 references still exactly match BOM.csv and every coordinate, angle and side is preserved. Design files and fabrication ZIP are unchanged. User needs to re-upload the corrected positions.csv; server acceptance has not been observed. Backup and validation: C:/Work/rusefi/temp/e38-release-oct07/cpl-format-fix. No extra production files, commit or push.
- Format reference: https://jlcpcb.com/help/article/how-to-generate-the-bom-and-centroid-file-from-kicad

## 2026-10-08 - Group JLCPCB BOM and start live upload verification

- Grouped the existing production/BOM.csv by exact Comment, Footprint and LCSC Part #. Added Quantity and comma-separated explicit designators: 23 groups, 108 fitted components. Kept the two 10k resistor part numbers separate. README now states the group count. Four-file production directory retained.
- Independently re-imported the CSV and checked 108 unique references, quantity total 108, exact metadata preservation, and no differences against positions.csv. Positions and fabrication ZIP hashes unchanged. Backup and validation: C:/Work/rusefi/temp/e38-release-oct07/bom-grouping.
- Signed-in JLCPCB browser session has an empty cart. Uploaded the fabrication ZIP into a draft quotation; JLCPCB accepted it and detected 2 layers, 149 x 104 mm. Selected lead-free HASL, Standard PCBA, both sides and 5 units for inspection; site automatically added two 5 mm edge rails. Paused before NEXT because this accepts assembly service terms; explicit user confirmation requested. BOM/CPL server acceptance and part matching are not yet verified. No order, payment, commit or push.

## 2026-10-08 - JLCPCB live BOM/CPL processing verified

- User explicitly approved acceptance of assembly-service terms. Continued the 5-unit, both-side Standard PCBA draft and uploaded the grouped BOM.csv and corrected positions.csv from production. JLCPCB processed both successfully: 23 groups detected, 17 confirmed, 3 inventory shortages and 3 unmatched groups. The earlier positions.csv processing failure is resolved in the actual website.
- Shortages at inspection time: D1/D2 SRV05-4-P-T7 C85364 require 10, stock 2 (short 8); J6/J7 39301160 C485577 require 10, stock 9 (short 1); U13/U14/U15 VNLD5160TR-E C377942 require 15, stock 12 (short 3). J1, J2 and M1 have no selected part, consistent with their blank LCSC fields. No substitute or omission was authorized/applied.
- Discovered a source metadata mismatch: U2/U3 are labelled MCP6004 / MCP6004T-I/ST but carry C248577, which JLCPCB correctly matches to TP6004-TR. This code also occurs in premium-quick-test and the saved carrier PCB. Microchip MCP6004T-I/ST is C50282 per LCSC. Reported the mismatch without silently selecting another amplifier or modifying CAD. 10k groups differ legitimately: C15401 is 5 percent and C25804 is 1 percent.
- Next produces an unselected-parts confirmation. Returned to part selection without accepting omission. Placement geometry and rotations are not yet approved. Browser draft left open, no order/payment, commit or push. Evidence in bom-grouping/jlc-matched-parts.txt and jlc-bom-accepted.jpg. Production still contains exactly the same four filenames.

## 2026-10-08 - Select available ADC protection, amplifier and Mini-Fit parts

- Read mega-mcu144/0.7/mega-mcu144-BOM.csv in the canonical hellen-one checkout: U170/U175/U180/U185/U190/U195 use Microchip MCP6004T-I/ST, C50282. Corrected U2/U3 from TP6004-TR code C248577 to C50282 in every schematic unit, the PCB and grouped production BOM. The existing Part #/manufacturer fields already specified Microchip. Verified standard quad pinout, TSSOP-14, +5VA/GNDA supplies, unity-gain feedback and isolated capacitive loads. Microchip's I grade specifies -40 to +85 C; operation to +125 C has reduced performance and is not the E grade, despite the JLC catalog's broad temperature label.
- Searched replacement CSVs across all locally available github boards. MSKSEMI SRV05-4-P-T7 C6454456 is used in HD81, Hellen154Hyundai, Polaris112, Superuaefi and uaefi121 replacements. Selected it for D1/D2. Manufacturer PDF confirms pin 2 GND, pin 5 VCC, IO pins 1/3/4/6, 5 V working voltage and SOT-23-6. The 60 W version is not pulse-rating-identical to every SRV05 vendor, but fits this circuit behind the existing 10k series resistors. Added supplier datasheet URLs.
- Replaced unavailable quantity of Molex J6/J7 C485577 with CJT C4201WR-F-2x8P C5355564. Verified saved PCB terminals at 4.2 mm pitch, 5.5 mm row spacing, 1.8 mm terminal drills, 3 mm plastic-lock holes spaced 29.4 mm at 7.3 mm offset, body width 34.8 mm, numbering and housing key pattern against CJT catalog pages C-135/C-154 and Molex drawing 55690002-SD. Retained the standard Molex footprint and representative 3D model. CJT catalog specifies 9 A maximum and -40 to +105 C; do not infer 13 A qualification from JLC's parametric summary. BOOMELE C69283 and HCTL C2845793 lack the original mounting locks; XKB C2884381 has a narrower -25 to +85 C temperature range. J3-J5 original Molex parts already match with adequate stock and were preserved.
- Also searched JLC's 31387 family for the large automotive connectors: exact J1 313872014 C563753 has 2, exact J2 0313874018 C17284829 has 0. Candidate 0313872032 C17401674 has 5 and 0313874017 C17327235 has 24, but they have different housing colors (gray/blue) and the former has silver instead of gold mating plating. Mating-key equivalence has not been established, so no automatic substitution was made for J1/J2. M1 remains a separately supplied uaBrain module.
- Live draft inventory at selection: C50282 10555, C6454456 3789, C5355564 127. Re-uploaded the actual updated production/BOM.csv and processed it with the unchanged CPL successfully. JLC now detects 23 groups: 19 confirmed, 1 shortage (VNLD5160TR-E C377942: need 15, stock 12), 3 unselected (J1/J2/M1). JLC's SRV order quantity is 15 including attrition; the BOM correctly remains 2 per board.
- Validation: only selected component property values changed. All 218 exported nets and 120 schematic components retain their connectivity; changed metadata is restricted to U2/U3/D1/D2/J6/J7. Native DRC: 0 errors, 0 opens, 0 schematic parity issues, 9 existing local-footprint warnings. ERC with the standard symbol table in an isolated config: 0 violations. Initial CLI ERC had 15 missing-library warnings from the incomplete scratch config, resolved by supplying the installed standard symbol table. No project severity changes.
- Production BOM remains 23 groups and 108 unique fitted references matching CPL. Fabrication ZIP and positions.csv SHA-256 hashes are unchanged; production retains exactly four files. Evidence, before copies and manufacturer PDFs: C:/Work/rusefi/temp/e38-parts-oct08. No commit, push, order or payment.
- Sources: C:/Work/rusefi/github/hellen-one/modules/mega-mcu144/0.7/mega-mcu144-BOM.csv; C:/Work/rusefi/github/hellen154hyundai/bom_replace_hellen154hyundai-e.csv; https://ww1.microchip.com/downloads/aemDocuments/documents/MSLD/ProductDocuments/DataSheets/MCP6001-1R-1U-2-4-1-MHz-Low-Power-Op-Amp-DS20001733L.pdf; https://jlcpcb.com/partdetail/Msksemi-SRV05_4_PT7/C6454456; https://jlcpcb.com/partdetail/6156263-C4201WR_F2x8P/C5355564; https://www.molex.com/pdm_docs/sd/039300020_sd.pdf

## 2026-10-08 - VNLD5160 replacement availability and drive-level review

- Read-only JLC catalog and draft inventory check: BM2LC105FJ-CE2 C3235499, BM2LC120FJ-CE2 C3235502 and NSD12409-Q1SPR C42387987 each have zero public stock. Broader BM2LC and NSD124 family searches did not reveal another stocked variant. VNLD5160TR-E C377942 has 12 for the 15-part build; VNLD5090TR-E C222209 has 7 public and 7 idle parts, still reported as a shortage. VNLD5300TR-E C123317 has 2000 public stock (1968 available order quantity on its detail page) and is selectable for the full group. Availability is a point-in-time observation.
- VNLD5300 pin mapping verified against the ST PDF configuration diagram and the saved netlist: 1 IN1, 2 STATUS1, 3 IN2, 4 STATUS2, 5 SOURCE2, 6 DRAIN2, 7 SOURCE1, 8 DRAIN1. Existing SOIC-8 footprint and disconnected status pins agree. It is a candidate, not an approved unconditional substitution: RON is 300 mOhm instead of 160 mOhm, and DC current limit is 2/2.8/3.8 A min/typ/max instead of 3.5/5/7.5 A. Check actual EVAP current and dissipation before selection. VNLD5090 has a much higher 13/18/25 A limit and is not equivalent short-circuit protection.
- Found an existing logic-margin issue independent of substitution. U1 PCA9685 VDD is +3.3V; U14/U15 inputs are driven from its outputs through R65-R68 4.7k. ST specifies VNLD5160/5300 electrical characteristics for VIN 4.5-5.5 V and threshold up to 3.5 V. The present 3.3 V drive is not guaranteed across specified conditions. U13 comes from module pins J2/J3 and requires separate source-level verification. Follow-up: establish proper drive levels and actual load currents before approving a driver change or manufacturing release.
- No schematic, PCB, production BOM or selected JLC component changed. Downloaded ST VNLD5300 data sheet from its JLC listing and inspected its pin diagram. Sources: https://jlcpcb.com/partdetail/STMicroelectronics-VNLD5300TRE/C123317 ; https://www.st.com/resource/en/datasheet/vnld5300-e.pdf ; https://www.st.com/resource/en/datasheet/vnld5160-e.pdf ; https://www.st.com/resource/en/datasheet/vnld5090-e.pdf

## 2026-10-08 - Refresh production export with VNLD5160 retained

- User requested retaining VNLD5160 and uploading the remaining approved substitutions. Fresh netlist/PCB verification confirms U2/U3 C50282, D1/D2 C6454456 and J6/J7 C5355564 already exist in both saved CAD files; U13/U14/U15 remain VNLD5160TR-E C377942. No further CAD mutation was necessary. All 218 nets are unchanged.
- Regenerated Gerber, separate PTH/NPTH drills, native placements, grouped BOM and JLC CPL. Compared all eleven fabrication files against the preceding production ZIP: every line is identical after removing creation-date comments only. Refreshed the local fabrication ZIP; the current JLC draft's existing fabrication geometry therefore remains valid and was retained instead of starting a replacement PCB draft. BOM has 23 groups / 108 fitted references, CPL has the same 108 references and byte-identical coordinates. Production still contains only BOM.csv, positions.csv, uaBrain-gm-e38-PCB.zip and README.txt.
- Fresh native validation: ERC zero violations; DRC zero errors, zero unconnected items, zero schematic parity issues and the same nine footprint warnings. Saved schematic and PCB source hashes remained unchanged throughout export. The previously identified 3.3 V driver-control margin remains open; this export is not approval of that electrical issue.
- Re-uploaded actual production BOM.csv and positions.csv to existing JLC draft 139a381fe804460a95239e3b09b848c6 and processed both successfully. Result remains 23 groups: 19 confirmed, one shortage (VNLD5160 needs 15, public stock 12), three unselected (J1/J2/M1). No omissions, order, payment, commit or push. Evidence and before files: C:/Work/rusefi/temp/e38-refresh-oct08; browser screenshot jlc-uploaded.png.


## 2026-10-08 - Add uaBrain E38 silkscreen identification

- Added uaBrain-gm-e38 rev. a, the original kicad6-libraries rusEFI logo and rusefi.com/s/uabrain-e38 on the automotive connector side. Added the existing Hellen wording Always looking for C/C++/PHP/Java developers plus https://rusefi.com/s/mission on the clear front lower strip. User explicitly selected the uabrain-e38 short-link suffix.
- Added an editable native KiCad QR barcode for https://rusefi.com/s/uabrain-e38 between the ADC and resistor bank. Its 29 x 29 modules have 0.22 mm pitch, 6.38 mm data square, at least 1.0 mm clear surround and ECC M. A larger initial placement conflicted with vias; the final location avoids holes and pads without changing copper or moving vias. Independently decoded the final mask-subtracted plot at 600 and 300 dpi and the actual exported B.Silkscreen Gerber. Bottom text and QR mirror correctly for viewing from the component side.
- Installed exactly six new graphic objects; retained every byte of the original board nodes and all other project files. The logo is board-only and excluded from BOM and placements while retaining the library footprint type. Native DRC and schematic parity: 0 errors, 0 opens, 0 parity issues, same 9 pre-existing local-footprint warnings. Inspected both final silkscreen plots.
- Refreshed production/uaBrain-gm-e38-PCB.zip. Only the front and back silkscreen Gerbers changed; the other nine fabrication files are identical after stripping creation-date comments. BOM.csv, positions.csv and README.txt are byte-identical, and production still contains exactly four files. Current JLC draft has the preceding Gerber revision; this updated fabrication ZIP has not been uploaded. No commit, push, order or payment.
- Open follow-up: https://rusefi.com/s/uabrain-e38 currently returns HTTP 404; the redirect needs to be configured. This silkscreen-only change does not resolve the previously recorded 3.3 V driver-input margin. Audit files and before snapshot: C:/Work/rusefi/temp/e38-branding-oct08.


## 2026-10-08 - Investigate JLC assembly of uaBrain support headers

- Read-only inspection confirms M1 uses hw-uaBrain-PinHeader with 112 holes of 1.1 mm diameter: two 32-pin columns and one 48-pin row, all with 2.54 mm pitch. Seven complete 1x16 headers cover the existing pin positions (2 + 2 + 3), avoiding a request to cut strips. Current BOM/CPL represent the module as M1 and do not identify separate headers.
- JLC documentation currently supports mixed SMT/THT assembly under Economic and Standard; CPL is required for THT components too. Proposed next step is separate physical header references with a selected JLC part number and seven placement records, plus a mounting-side/orientation drawing. Module installation remains a subsequent operation. No component has been selected, ordered, or changed in CAD/BOM/CPL by this investigation.
- Existing nominal geometry uses 6 mm mating pins above a 2.54 mm insulator, a 1.6 mm module and 1 mm pin protrusion, requiring 5.94 mm face-to-face board spacing. The final selected header drawing, stock, adjacent housing fit and assembly alignment still need checking. Catalog example PZ2.54-1*16 C5360903 matches the nominal 6/2.54/3 mm height convention and is listed for wave soldering; stock was not verified. Sources: https://jlcpcb.com/capabilities/pcb-assembly-capabilities ; https://jlcpcb.com/help/article/pick-place-file-for-pcb-assembly ; https://jlcpcb.com/partdetail/ZHOURI-PZ2_54_116/C5360903

## 2026-10-08 - Add physical uaBrain support headers to the E38 carrier

- Added real schematic and PCB connectors J8-J14, seven XUNPU PH2.54-01-16PZD 1x16 headers with LCSC/JLC C7501590. Fresh public JLC catalog data showed 168 pieces during selection. Checked the manufacturer drawing: 2.54 mm pitch, 0.64 mm square pins, 6 mm mating ends, 3 mm solder tails, 2.50 mm insulator, nominal 40.44 mm body and 3 A rating. The body length tolerance can consume the nominal 0.20 mm end clearance; assembly notes require checking adjacent moulded ends and row alignment.
- Retained M1 as a logical module excluded from BOM, PCB update and position export. Its locked board-only mechanical instance retains pin legends, microSD cutout and assembled module model, but no pads or obsolete combined decorative header model. J8-J14 own all 112 original pads with exact original coordinates, drill/copper dimensions and thermal overrides. Standard KiCad symbols and header footprints have project-instance overrides; no new local library and no shared-library changes.
- Set each header footprint anchor to its 16-pin centroid, making native CPL export use the real placement centre. Added standard STEP header models and seven front silkscreen references. Verified a native 3D rendering with only the overlying module hidden in a disposable copy. The actual project retains the assembled module at Z=7.50 mm for a nominal 5.90 mm PCB gap and 1 mm pin protrusion above a 1.6 mm module; the standard header STEP's insulator is representative, 0.04 mm taller than the selected part.
- Saved and rechecked canonical project: ERC 0 violations; DRC 0 errors, 0 opens, 0 schematic-parity issues, 16 library-mismatch warnings from local overrides (9 previous plus 7 headers). Verified every original component-pin assignment on all 218 original nets. All child schematics, project design rules and unrelated source files are unchanged. Independent Gerber/Excellon primitive comparison shows identical copper, masks, pastes, outline and both drill files; only 163 front-silkscreen primitives were added for J8-J14.
- Regenerated grouped BOM and JLC CPL from CAD: 114 fitted references in 23 groups; headers grouped as quantity 7; M1 omitted. All other 107 component selections and placement rows are unchanged. Updated the manufacturing ZIP and installation instructions. Production still contains exactly four files: BOM.csv, positions.csv, README.txt and uaBrain-gm-e38-PCB.zip. Included top-side THT seating and later module-installation instructions.
- Browser control failed to initialize because of its Windows sandbox helper; no upload or JLC component selection was performed in this step. Existing draft still needs the current Gerber, BOM and CPL. Existing VNLD5160 availability, J1/J2 sourcing, short-URL redirect and 3.3 V driver-input margin follow-ups are unchanged. No commit, push, order or payment. Before snapshot, manufacturer drawing, native renders and validation evidence: C:/Work/rusefi/temp/e38-headers-oct08.

## 2026-10-09 - Review module/header synchronization ownership

- Confirmed the current M1 is excluded from PCB transfer; electrical pad synchronization is through J8-J14. This passes parity but removes direct M1 pin-to-pad synchronization and duplicates the intended wiring at the header symbols.
- Recommended restoring all 112 pads and the schematic association to M1, retaining only its BOM/POS exclusions. J8-J14 can instead be padless assembly footprints with pinless assembly symbols, selected part metadata, 3D models and placement centres. Group the placed instances with M1 for mechanical movement. This alternative has not been installed in the project.
- Verified in a disposable PCB that KiCad 10 native position export still includes all seven padless header instances at their existing coordinates/angles. No full F8 synchronization test or alternate ERC/DRC result is claimed. Canonical CAD and production files were not changed. Temporary evidence: C:/Work/rusefi/temp/e38-header-options-oct09.
- KiCad 10 documentation confirms Exclude from board omits a symbol from PCB transfer, and coincident drilled holes generate a co-located-hole DRC warning. Thus overlaying two full drilled-pad footprints is unnecessary; keep one physical set of copper and drill definitions. Sources: https://docs.kicad.org/10.0/en/eeschema/eeschema.html ; https://docs.kicad.org/10.0/en/pcbnew/pcbnew.html

## 2026-10-09 - Restore M1 electrical footprint and use assembly-only support headers

- User approved the pad-ownership change and saved/closed both KiCad editors. Snapshotted the saved project, checked source hashes before replacement, and retained all unrelated changes. M1 now owns all 112 PTH pads with original module pin numbers, positions, 2.0 mm copper, 1.1 mm drills, net connections and thermal settings. Restored its original schematic path, sheet metadata and PCB-transfer flag; M1 remains excluded only from BOM and position export.
- J8-J14 are pinless Mechanical:Mechanical_Shape assembly symbols with padless project-instance header footprints. They retain C7501590, quantity 7, reference text, native placement centres and STEP models. Removed duplicated header wires/net labels and compacted the assembly list in the existing root sheet. All 218 original nets and original component-pin assignments match exactly. Added one locked PCB group containing M1 and J8-J14; all footprint locations and every 3D model block are unchanged. No new custom library or shared-library change.
- Canonical installed project validation: native ERC 0 violations, native DRC 0 errors, 0 opens and 0 schematic-parity differences; the same 16 library-override warnings remain. In a disposable negative control, changing M1.N5 from CAN_H to GND produced a net_conflict schematic-parity warning identifying M1.N5, confirming direct checking of M1 is active. The GUI F8 dialog was not driven; verified saved association/flags, exported netlist and native parity instead.
- Refilled both ground zones. Independently compared all 11 exported Gerber/Excellon files against the previous manufacturing ZIP as geometric primitives: identical copper, paste, mask, silkscreen, outline, PTH and NPTH drilling. Native KiCad BOM matches all 23 prior groups / 114 fitted references, including the seven padless headers, and native positions match every production CPL row. Existing BOM.csv, positions.csv and production README.txt were therefore preserved byte-for-byte. Refreshed only the fabrication ZIP; production still contains exactly four files.
- Updated project mounting/synchronization documentation. Do not reset J8-J14 from their original library footprints, which would restore duplicate pads; their assembly-only overrides reside inside this PCB. Existing hardware/order follow-ups remain unchanged. No commit, push, JLC upload, order or payment. Before snapshot, native exports, negative control, rendered schematic and checks: C:/Work/rusefi/temp/e38-m1-pads-oct09.


## 2026-10-09 - Repair support-header schematic-to-PCB identity paths

- User supplied the real F8 warning log screenshot: J9/J12 pad numbers absent from their symbols and locked J8-J14 reported as unused. Found an earlier generator error: these seven root-sheet footprints had /root-schematic-UUID/symbol-UUID paths, while the exported component paths require /symbol-UUID. KiCad's F8 updater compares the complete path, so it attempted to add library headers and remove the unmatched assembly instances. The previous native DRC/parity pass did not detect this association error.
- User saved and closed the PCB editor. Fresh source hashes remained unchanged; the saved board contained no duplicate references. Corrected only the seven path fields, keeping every other PCB byte unchanged. All 127 schematic components now have an exact matching full PCB path, with no unmatched electrical/assembly instances. Native board reload confirms the seven paths, M1's 112 pads, seven padless headers and the eight-member module group.
- Validation: fresh native ERC 0 violations; DRC 0 errors, 0 opens, 0 schematic-parity issues and the same 16 library-override warnings. All other CAD files and all four production files remain byte-identical. Copper, drills, silkscreen, models and assembly coordinates were not changed; manufacturing regeneration is unnecessary for this identity-only correction.
- Windows GUI automation failed to initialize (sandbox helper apply deny-read ACLs), so the real F8 dialog could not be driven. Checked all full paths against a fresh XML netlist using the matching rule in KiCad 10 board_netlist_updater.cpp; a GUI repeat remains the final interactive confirmation. No commit, push or upload. Snapshot and checks: C:/Work/rusefi/temp/e38-header-links-oct09.
- Reference: https://gitlab.com/kicad/code/kicad/-/raw/10.0/pcbnew/netlist_reader/board_netlist_updater.cpp


## 2026-10-09 - Reassess readiness before placing the carrier order

- Did not approve manufacturing release. Fresh schematic XML confirms U1 PCA9685 VDD=+3.3V, U14/U15 inputs driven through R65-R68 4.7k, and their source signals pulled down by R71-R74 4.7k. Re-read the official ST VNLD5160-E datasheet, page 8: input threshold maximum 3.5 V; characterized input/supply range 4.5-5.5 V. The current 3.3 V drive is therefore not guaranteed. Resolve the drive interface while retaining the selected VNLD5160 before ordering; U13's module-derived controls still need separate source-level confirmation.
- All saved CAD hashes match the just-validated header-link repair; all four production files are unchanged. An isolated DRC with every ignored category enabled reports zero errors and zero opens, with 16 library differences, 88 missing courtyards, 35 off-centre via endpoints, 112 module PTH pads inside their assembly-header courtyards, and the same two known thermal warnings (M1.C8 bottom island and R38.1 one-spoke logic pulldown). Ten additional parity warnings are footprint filters only, including the seven assembly-symbol/header pairs; no electrical net conflict is present. Production rules were not changed.
- JLC component availability, J1/J2 supply, current THT placement preview and upload of the latest header-inclusive package still require confirmation; no live stock claim is made. The real F8 GUI repeat remains unconfirmed. No CAD/production edits, order, payment, commit or push. Evidence: C:/Work/rusefi/temp/e38-readiness-oct09.
- Sources: https://www.st.com/resource/en/datasheet/vnld5160-e.pdf ; https://www.nxp.com/docs/en/data-sheet/PCA9685.pdf


## 2026-10-09 - Clean schematic annotations and prepare JLC review

- User explicitly accepted the existing VNLD5160 control levels for this build; that item is no longer a release blocker under this agreed assumption. Recorded the decision in the project README without changing the circuit or implying new bench validation.
- Removed 18 added free-text annotations: module pad-ownership/installation explanations, bus-side captions and the bus-colour legend labels. Also removed the legend's nine disconnected colour samples and four dashed frame lines, and shortened the mounting-header heading. Retained actual net labels, bus aliases, functional wiring and header-to-module pin mappings. Other sheets and the PCB are byte-identical. Inspected a fresh native schematic rendering.
- Fresh native exports confirm all 218 nets and 127 component definitions are unchanged, every full schematic/PCB association matches, M1 owns 112 pads and J8-J14 remain padless assembly instances. ERC: 0 violations. DRC: 0 errors, 0 opens, 0 parity issues and the same 16 library-override warnings. Native BOM/CPL match production: 23 groups, 114 fitted references, seven C7501590 headers and M1 excluded. All four production files remain byte-identical; no duplicate exports or folders were added there.
- Attempted to connect to existing JLC draft 139a381fe804460a95239e3b09b848c6 through Browser Use, reset/retried, then tried the supported Computer Use initialization/reset/retry. Both runtimes failed before any page observation or input: Windows helper apply deny-read ACLs / trusted Node process exited unexpectedly. Live order contents, stock, placements and current uploads could not be checked. The app accepted a queued request to show the existing draft, which is not proof that its contents were read. No website mutation, upload, order, payment, commit or push.
- Remaining work: restore browser access, upload/check the current header-inclusive manufacturing package in the same JLC draft, verify all selected parts and THT placement, and confirm the real KiCad F8 update if still needed. Evidence and before snapshot: C:/Work/rusefi/temp/e38-final-cleanup-oct09.

## 2026-10-09 - Recover JLC browser access and refresh assembly files

- Browser access recovered after the user restarted Codex; authenticated draft 139a381fe804460a95239e3b09b848c6 opened successfully.
- Uploaded current production/BOM.csv and production/positions.csv. JLC processed both successfully: 23 groups, 20 confirmed, one shortage and two unmatched. J8-J14 are selected as C7501590 on the top side (35 pieces for five boards); obsolete M1 assembly row is absent.
- Live stock: VNLD5160TR-E C377942 requires 15 pieces, only 10 available, five short. J1/J2 remain unmatched. NEXT presents Project has unselected parts with Do not place / Select parts. Did not authorize omissions; awaiting the user's assembly-scope decision.
- Gerber freshness and component placement preview remain unverified. Change PCB specifications opened a blank quote rather than restored settings; returned without uploading or changing those settings. No order, payment, substitute selection, CAD changes, commit or push.
- Evidence: C:/Work/rusefi/temp/e38-jlc-review-oct09/unselected-parts.jpg. Left the draft open for continuation.

## 2026-10-09 - Confirm manual J1/J2 assembly and correct JLC header rotations

- User confirmed J1/J2 will be hand assembled and VNLD5160 should remain unchanged pending incoming stock. Accepted JLC's Do not place dialog for the two unmatched connectors.
- Reached Component Placements. All seven C7501590 headers initially lay perpendicular to their hole rows. Selected only J8-J14 and applied Rotate Left by 90 degrees. Visual check confirms all seven strips now align with their hole rows; no XY changes. JLC auto-saved the placement correction at 19:58 when continuing to Quote & Order.
- VNLD5160 remains the matched BOM part with five-piece shortage; its instances are not present in the assembly preview while unavailable. Recheck selection and placements after replenishment before ordering. Do not interpret the quote step as confirmation that these parts will be assembled.
- Current Gerber freshness and complete bottom-side pin-1 validation remain open. No order or payment. Header correction is currently in this JLC draft only; re-uploading the original CPL may require reapplying the 90-degree correction. Evidence: C:/Work/rusefi/temp/e38-jlc-review-oct09/headers-aligned.jpg.

## 2026-10-09 - Verify live Gerber freshness

- Read the PCB Bottom preview in the current JLC draft. It lacks the revision/name, EFI logo and QR code present in the current production output. The uploaded Gerber set is stale; manufacturing readiness of this draft is not confirmed.
- Current board SHA-256 remains b6dd669f078abb2872df07e3ad5057c45ae046797ba952ed65faf750070455c1 and production archive remains 7fb26bbf1b92a119f4af2a636df6feb13ee08124b50ad974299008bbf85404f8, matching the validated release. Parsed current archive B_Silkscreen with Gerbonara: all 11139 geometric primitives exactly match the previously rendered and QR-verified branded bottom layer. Thus the missing branding is in the uploaded draft, not the local production ZIP.
- Remaining actions: replace JLC Gerber with current production ZIP, recheck assembly registration and rotations after replacement, and verify VNLD inclusion when stock arrives. J1/J2 are intentionally hand assembled. No CAD edits, upload, order, payment, commit or push during this verification. Evidence: C:/Work/rusefi/temp/e38-jlc-review-oct09/stale-gerber-bottom.jpg.

## 2026-10-09 - Upload current Gerber and rebuild the JLC draft

- Uploaded production/uaBrain-gm-e38-PCB.zip through Change PCB specifications. JLC allocated new draft ce1f49cf50084600b467a6eaf0360063 instead of replacing the old draft. Use https://cart.jlcpcb.com/smt-order/?pcbFileNo=ce1f49cf50084600b467a6eaf0360063 from now on; old 139a381fe804460a95239e3b09b848c6 is stale and was not deleted.
- Detected 2 layers, 149 x 104 mm. Set FR-4, 1.6 mm, 1 oz, LeadFree HASL, Standard PCBA, Both Sides, five boards. JLC adds two 5 mm process rails. Re-uploaded current BOM.csv and positions.csv; 23 groups, 20 available, VNLD5160 shortage unchanged and J1/J2 manually assembled as authorized.
- After server processing completed, visually confirmed current name/revision, EFI logo, short URL, QR and Always looking inscription in the new Gerber previews. Reapplied Rotate Left 90 degrees to only J8-J14 and verified all seven strips align with holes. Inspected both sides for gross registration. Full final pin-1/DFM confirmation remains the assembly review gate; no claim of bench validation.
- Continued to Quote & Order to save placement at 20:06. Current quote is USD 248.02 excluding shipping and missing VNLD; 20 available component groups only. Do not place this draft as-is expecting VNLD to be installed: reselect/check them after replenishment. No order, payment, commit or push.
- Evidence: C:/Work/rusefi/temp/e38-jlc-review-oct09/current-gerber-bottom.jpg and current-headers-aligned.jpg. Local CAD and production files unchanged; header rotation adjustment resides in the JLC draft.

## 2026-10-09 - Remove production README at user request

- Removed uaBrain-gm-e38/production/README.txt. Production now contains only BOM.csv, positions.csv and uaBrain-gm-e38-PCB.zip. Manufacturing files were not edited. No commit or push.

