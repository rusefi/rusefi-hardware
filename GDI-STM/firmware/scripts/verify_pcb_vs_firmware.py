#!/usr/bin/env python3
"""
Cross-check the firmware's pin definitions against the *produced PCB*.

Source of truth:
  - `GDI-STM.kicad_pcb` (footprint U7 pad pinfunctions + net names)

Firmware source:
  - `include/board_pins.hpp`

This is a static sanity check intended to prevent silent pinmap drift after PCB production.
"""

from __future__ import annotations

import argparse
import json
import os
import re
import subprocess
import sys
from typing import Dict, List, Tuple


def run_pinmap(pcb: str, mcu_ref: str) -> Dict:
    scripts_dir = os.path.dirname(os.path.abspath(__file__))
    cmd = [
        sys.executable,
        os.path.join(scripts_dir, "kicad_pcb_pinmap.py"),
        "--pcb",
        pcb,
        "--mcu-ref",
        mcu_ref,
    ]
    proc = subprocess.run(cmd, stdout=subprocess.PIPE, stderr=subprocess.PIPE, text=True)
    if proc.returncode != 0:
        raise RuntimeError(proc.stderr.strip() or "kicad_pcb_pinmap.py failed")
    return json.loads(proc.stdout)


def parse_board_pins(header_path: str) -> Dict[str, Tuple[str, int]]:
    """
    Parse `include/board_pins.hpp` into a net-name -> (port_letter, pin_number) dict.

    Net names are inferred from trailing comments:
      - arrays: `{GPIOX, GPIO_PIN_N}, // INJ1`
      - singles: `... = {GPIOX, GPIO_PIN_N}; // CAN_TX ...`

    This keeps the verifier decoupled from C++ symbol names while still being explicit.
    """
    text = open(header_path, "r", encoding="utf-8").read()
    out: Dict[str, Tuple[str, int]] = {}

    # Array entries with a comment name.
    for port, pin, name in re.findall(
        r"\{\s*GPIO([A-Z])\s*,\s*GPIO_PIN_(\d+)\s*\}\s*,\s*//\s*([A-Za-z0-9_]+)",
        text,
    ):
        out[name] = (port, int(pin))

    # Single GpioPin definitions with trailing comment.
    for port, pin, comment in re.findall(
        r"inline\s+constexpr\s+GpioPin\s+\w+\s*=\s*\{\s*GPIO([A-Z])\s*,\s*GPIO_PIN_(\d+)\s*\}\s*;\s*//\s*([^\n\r]+)",
        text,
    ):
        name = comment.strip().split()[0]
        out[name] = (port, int(pin))

    return out


def pin_str(port: str, pin: int) -> str:
    return f"P{port}{pin}"


def main(argv: List[str]) -> int:
    ap = argparse.ArgumentParser()
    ap.add_argument("--pcb", default="GDI-STM.kicad_pcb")
    ap.add_argument("--mcu-ref", default="U7")
    ap.add_argument("--board-pins", default=os.path.join("include", "board_pins.hpp"))
    args = ap.parse_args(argv)

    if not os.path.exists(args.pcb):
        print(f"error: pcb not found: {args.pcb}", file=sys.stderr)
        return 2
    if not os.path.exists(args.board_pins):
        print(f"error: board pins header not found: {args.board_pins}", file=sys.stderr)
        return 2

    pinmap = run_pinmap(args.pcb, args.mcu_ref)
    mcu = pinmap.get("mcu_net_to_pins", {})
    fw = parse_board_pins(args.board_pins)

    required = (
        [f"INJ{i}" for i in range(1, 9)]
        + [f"START{i}" for i in range(1, 9)]
        + ["CAN_TX", "CAN_RX"]
        + [f"HS_VBAT{i}" for i in range(1, 5)]
        + [f"HS_VBOOST{i}" for i in range(1, 5)]
        + ["BOOST_PWM"]
    )

    errors: List[str] = []

    for net in required:
        if net not in fw:
            errors.append(f"missing net in include/board_pins.hpp comments: {net}")
            continue
        if net not in mcu:
            errors.append(f"missing net on PCB (or not connected to {args.mcu_ref}): {net}")
            continue

        want = pin_str(*fw[net])
        got = mcu[net][0].get("mcu_pin")
        if got != want:
            errors.append(f"net {net} expected {want} but PCB reports {got}")

    # Informational: list PCB nets connected to the MCU but not represented in firmware pinmap.
    ignore = {
        "SWDIO",
        "SWCLK",
        "nRESET",
        "Boot0",
        "+3V3",
        "GND",
    }
    unhandled = []
    for net in sorted(mcu.keys()):
        if net in fw:
            continue
        if net in ignore:
            continue
        if net.startswith("Net-("):
            continue
        if net.startswith("unconnected-("):
            continue
        unhandled.append(net)

    if unhandled:
        print("note: PCB nets connected to MCU but not yet named in firmware pinmap comments:")
        for n in unhandled:
            pins = ", ".join([e.get("mcu_pin", "?") for e in mcu.get(n, [])])
            print(f"  - {n}: {pins}")

    if errors:
        print("pinmap verification FAILED:", file=sys.stderr)
        for e in errors:
            print(f"  - {e}", file=sys.stderr)
        return 1

    print("pinmap verification OK (PCB vs firmware)")
    return 0


if __name__ == "__main__":
    raise SystemExit(main(sys.argv[1:]))

