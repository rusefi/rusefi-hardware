#!/usr/bin/env python3
"""
Extract a net -> MCU pin mapping from a KiCad `.kicad_pcb` file.

This is intentionally lightweight (no third-party dependencies) and tailored for:
  - locating a footprint by reference (default: U7)
  - reading each `pad`'s `(pinfunction "...")` and `(net ... "...")`

Output JSON shape mirrors `scripts/kicad_pinmap.py` enough for verification scripts:
  {
    "mcu_ref": "U7",
    "mcu_net_to_pins": {
      "INJ1": [{"mcu_pin": "PE8"}],
      ...
    }
  }
"""

from __future__ import annotations

import argparse
import json
import os
import re
import sys
from typing import Dict, List, Optional


def extract_block(text: str, start: int) -> Optional[str]:
    depth = 0
    i = start
    while i < len(text):
        c = text[i]
        if c == "(":
            depth += 1
        elif c == ")":
            depth -= 1
            if depth == 0:
                return text[start : i + 1]
        i += 1
    return None


def find_footprint(text: str, ref: str) -> Optional[str]:
    for m in re.finditer(r"\(footprint\s", text):
        blk = extract_block(text, m.start())
        if not blk:
            continue
        if f'"Reference" "{ref}"' in blk:
            return blk
    return None


def normalize_net(net: str) -> str:
    if net.startswith("/"):
        return net[1:]
    return net


def main(argv: List[str]) -> int:
    ap = argparse.ArgumentParser()
    ap.add_argument("--pcb", default="GDI-STM.kicad_pcb")
    ap.add_argument("--mcu-ref", default="U7")
    args = ap.parse_args(argv)

    if not os.path.exists(args.pcb):
        print(f"error: pcb not found: {args.pcb}", file=sys.stderr)
        return 2

    text = open(args.pcb, "r", encoding="utf-8").read()
    fp = find_footprint(text, args.mcu_ref)
    if not fp:
        print(f"error: footprint not found for reference {args.mcu_ref}", file=sys.stderr)
        return 2

    net_to_pins: Dict[str, List[Dict[str, str]]] = {}
    seen = set()

    for m in re.finditer(r"\(pad\s", fp):
        pad_blk = extract_block(fp, m.start())
        if not pad_blk:
            continue

        pinfm = re.search(r'\(pinfunction\s+"([^"]+)"\)', pad_blk)
        netm = re.search(r'\(net\s+\d+\s+"([^"]+)"\)', pad_blk)
        if not pinfm or not netm:
            continue

        pinfunction = pinfm.group(1).strip()
        net = normalize_net(netm.group(1).strip())

        # Only keep GPIO-like pins (PA0, PB12, PC5, PE7, ...).
        if not re.match(r"^P[A-G]\d+$", pinfunction):
            continue

        key = (net, pinfunction)
        if key in seen:
            continue
        seen.add(key)
        net_to_pins.setdefault(net, []).append({"mcu_pin": pinfunction})

    out = {"mcu_ref": args.mcu_ref, "mcu_net_to_pins": net_to_pins}
    print(json.dumps(out, indent=2, sort_keys=True))
    return 0


if __name__ == "__main__":
    raise SystemExit(main(sys.argv[1:]))

