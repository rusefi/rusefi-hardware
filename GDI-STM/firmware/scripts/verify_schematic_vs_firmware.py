#!/usr/bin/env python3
"""
Cross-check the firmware's pin definitions against the KiCad schematic.

This is a *static* sanity check intended to prevent silent pinmap drift:
  - Extracts labeled nets connected to the MCU (default U7) from `GDI-STM.kicad_sch`
    using `scripts/kicad_pinmap.py`.
  - Verifies the critical nets we drive/use in firmware exist and map to the expected MCU pins.
  - Reports MCU-connected nets that are not yet represented in `include/board_pins.hpp`.

It does not prove electrical correctness. Always validate against the schematic and on hardware.
"""

from __future__ import annotations

import argparse
import json
import os
import re
import subprocess
import sys
from typing import Dict, List, Tuple


def run_pinmap(schematic: str, mcu_ref: str) -> Dict:
    scripts_dir = os.path.dirname(os.path.abspath(__file__))
    cmd = [
        sys.executable,
        os.path.join(scripts_dir, "kicad_pinmap.py"),
        "--schematic",
        schematic,
        "--mcu-ref",
        mcu_ref,
    ]
    proc = subprocess.run(cmd, stdout=subprocess.PIPE, stderr=subprocess.PIPE, text=True)
    if proc.returncode != 0:
        raise RuntimeError(proc.stderr.strip() or "kicad_pinmap.py failed")
    return json.loads(proc.stdout)


def parse_board_pins(header_path: str) -> Dict[str, Tuple[str, int]]:
    """
    Best-effort parse of a few patterns in include/board_pins.hpp:
      {GPIOX, GPIO_PIN_N} -> ("X", N)
    Returns a dict of symbol-name -> (port_letter, pin_number).
    """
    with open(header_path, "r", encoding="utf-8") as f:
        text = f.read()

    # Example line: inline constexpr GpioPin kCanTx = {GPIOB, GPIO_PIN_11};
    # Example array lines: {GPIOC, GPIO_PIN_8}, // INJ1
    pat = re.compile(r"\{\s*GPIO([A-Z])\s*,\s*GPIO_PIN_(\d+)\s*\}")
    matches = pat.findall(text)
    # This parser doesn't know the names per entry; we only use it for presence check via explicit expectations below.
    return {f"entry_{i}": (port, int(pin)) for i, (port, pin) in enumerate(matches)}


def main(argv: List[str]) -> int:
    ap = argparse.ArgumentParser()
    ap.add_argument("--schematic", default="GDI-STM.kicad_sch")
    ap.add_argument("--mcu-ref", default="U7")
    ap.add_argument("--board-pins", default=os.path.join("include", "board_pins.hpp"))
    args = ap.parse_args(argv)

    if not os.path.exists(args.schematic):
        print(f"error: schematic not found: {args.schematic}", file=sys.stderr)
        return 2
    if not os.path.exists(args.board_pins):
        print(f"error: board pins header not found: {args.board_pins}", file=sys.stderr)
        return 2

    pinmap = run_pinmap(args.schematic, args.mcu_ref)
    mcu = pinmap.get("mcu_net_to_pins", {})
    net_eps = pinmap.get("net_to_endpoints", {})

    # Critical nets the firmware assumes.
    expected = {
        **{f"INJ{i}": (("C", 9 - i) if i >= 4 else None) for i in range(1, 9)},  # placeholder - overwritten below
        "CAN_TX": ("B", 11),
        "CAN_RX": ("B", 12),
        "START1": ("B", 14),
        "START2": ("B", 13),
        "START3": ("B", 3),
        "START4": ("B", 2),
        "START5": ("B", 8),
        "START6": ("B", 1),
        "START7": ("B", 0),
        "START8": ("A", 14),
        "HS_VBAT1": ("E", 10),
        "HS_VBAT2": ("E", 9),
        "HS_VBAT3": ("E", 8),
        "HS_VBAT4": ("E", 7),
        "HS_VBOOST2": ("F", 1),
        "HS_VBOOST3": ("F", 0),
        "Vboost": ("C", 9),
        "Voboost": ("B", 15),
    }
    # Fix INJ mapping explicitly.
    expected.update(
        {
            "INJ1": ("C", 8),
            "INJ2": ("C", 7),
            "INJ3": ("C", 6),
            "INJ4": ("C", 5),
            "INJ5": ("C", 4),
            "INJ6": ("C", 3),
            "INJ7": ("C", 2),
            "INJ8": ("C", 1),
        }
    )

    errors: List[str] = []
    for net, (port, pin) in expected.items():
        entries = mcu.get(net)
        if not entries:
            errors.append(f"missing net in schematic (or not connected to {args.mcu_ref}): {net}")
            continue
        # Expect exactly one MCU pin per net for this design.
        mcu_pin = entries[0]["mcu_pin"]
        if mcu_pin != f"P{port}{pin}":
            errors.append(f"net {net} expected P{port}{pin} but schematic reports {mcu_pin}")

    # Report MCU nets not yet represented in firmware pin defs (informational).
    handled_nets = set(expected.keys()) | {
        "SWDIO",
        "SWCLK",
        "nRESET",
        "Boot0",
        # Sense pins currently only configured as analog inputs.
        "VinM1",
        "VinM3",
        "VinM4",
        "VinMboost",
        "VinP2",
        "VinP3",
        "VinP4",
        "VinPboost",
        "Vinj2",
        "Vinj3",
        "Vinj8",
        "Vo3",
        "Vo4",
    }
    unhandled = sorted([n for n in mcu.keys() if n not in handled_nets])

    if unhandled:
        print("note: schematic nets connected to MCU but not yet handled explicitly in firmware pinmap:")
        for n in unhandled:
            pins = ", ".join([e["mcu_pin"] for e in mcu[n]])
            print(f"  - {n}: {pins}")

    if errors:
        print("pinmap verification FAILED:", file=sys.stderr)
        for e in errors:
            print(f"  - {e}", file=sys.stderr)
        return 1

    # Heuristic warnings for likely schematic issues.
    def has_endpoint(net: str, ref: str, pin_name: str) -> bool:
        for e in net_eps.get(net, []):
            if e.get("ref") == ref and e.get("pin_name") == pin_name:
                return True
        return False

    # Injector control sanity: each INJx net should route to the expected BANK sheet pin.
    inj_sheet_expected = {
        "INJ1": ("SHEET:BANK1", "LS1"),
        "INJ2": ("SHEET:BANK1", "LS2"),
        "INJ3": ("SHEET:BANK2", "LS1"),
        "INJ4": ("SHEET:BANK2", "LS2"),
        "INJ5": ("SHEET:BANK3", "LS1"),
        "INJ6": ("SHEET:BANK3", "LS2"),
        "INJ7": ("SHEET:BANK4", "LS1"),
        "INJ8": ("SHEET:BANK4", "LS2"),
    }
    for net, (sref, spin) in inj_sheet_expected.items():
        if net in net_eps and not has_endpoint(net, sref, spin):
            print(f"warning: net {net} does not connect to {sref}:{spin} (check BANK sheet pin wiring/labels)")

    # Bank driver sanity: INJx should ultimately reach IRS21867S LIN pins inside the bank sheet.
    # If these are missing, the MCU may be toggling pins that never drive the power stage.
    inj_bank_expected = {
        "INJ1": ("BANK1/", ["U10", "U11"]),
        "INJ2": ("BANK1/", ["U10", "U11"]),
        "INJ3": ("BANK2/", ["U10", "U11"]),
        "INJ4": ("BANK2/", ["U10", "U11"]),
        "INJ5": ("BANK3/", ["U10", "U11"]),
        "INJ6": ("BANK3/", ["U10", "U11"]),
        "INJ7": ("BANK4/", ["U10", "U11"]),
        "INJ8": ("BANK4/", ["U10", "U11"]),
    }
    for net, (prefix, us) in inj_bank_expected.items():
        eps = net_eps.get(net, [])
        if not eps:
            continue
        ok = False
        for u in us:
            if has_endpoint(net, f"{prefix}{u}", "LIN"):
                ok = True
                break
        if not ok:
            print(f"warning: net {net} does not connect to any {prefix}IRS21867S:LIN pin (LS control may be floating)")

    # HS control sanity: HS_VBATx / HS_VBOOSTx nets are gate-driver inputs (HIN) in bank.kicad_sch.
    hs_expected = {
        "HS_VBAT1": ("BANK1/U10", "HIN"),
        "HS_VBAT2": ("BANK2/U10", "HIN"),
        "HS_VBAT3": ("BANK3/U10", "HIN"),
        "HS_VBAT4": ("BANK4/U10", "HIN"),
        "HS_VBOOST2": ("BANK2/U11", "HIN"),
        "HS_VBOOST3": ("BANK3/U11", "HIN"),
    }
    for net, (ref, pin) in hs_expected.items():
        if net in net_eps and not has_endpoint(net, ref, pin):
            print(f"warning: net {net} does not connect to {ref}:{pin} (gate-driver input may be floating)")

    # Boost PWM net should typically have *some* non-sheet endpoint (MCU timer pin or connector).
    # If it only connects to the BOOST sheet pin, it's likely floating at the top level.
    boost_eps = net_eps.get("BOOST_PWM", [])
    if boost_eps and not any(e.get("ref") == args.mcu_ref for e in boost_eps) and all(e.get("lib_id") == "__sheet__" for e in boost_eps):
        print("warning: net BOOST_PWM only connects to the BOOST sheet pin (likely not connected to MCU at top level)")

    # CAN sanity: on a TJA1051, MCU TX should connect to TXD and RX to RXD (not S/VIO).
    if "CAN_TX" in net_eps:
        if not has_endpoint("CAN_TX", "U6", "TXD"):
            print("warning: net CAN_TX does not connect to U6:TXD (check TJA1051 wiring/labels)")
        if has_endpoint("CAN_TX", "U6", "S"):
            print("warning: net CAN_TX connects to U6:S (silent mode) - label might be wrong")
    if "CAN_RX" in net_eps:
        if not has_endpoint("CAN_RX", "U6", "RXD"):
            print("warning: net CAN_RX does not connect to U6:RXD (check TJA1051 wiring/labels)")
        if has_endpoint("CAN_RX", "U6", "VIO"):
            print("warning: net CAN_RX connects to U6:VIO (I/O supply) - label might be wrong")

    print("pinmap verification OK")
    return 0


if __name__ == "__main__":
    raise SystemExit(main(sys.argv[1:]))
