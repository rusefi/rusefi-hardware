#!/usr/bin/env python3
"""
Extract a pin/net map from a KiCad v6+ schematic (.kicad_sch).

Primary use in this repo:
  - Map labeled nets to STM32 MCU pins (U7) for firmware pin configuration.
  - Emit a JSON report with endpoints on each net for review.

This parser is intentionally dependency-free (stdlib only). It is not a complete
KiCad schematic engine; it supports the constructs used by this repo:
  - lib_symbols (embedded symbol library definitions)
  - symbol instances (placed components with lib_id, at, unit, mirror)
  - wire segments / polylines (wire (pts (xy ...) ...))
  - labels (label/global_label/hierarchical_label)
  - junctions (junction (at ...))
  - hierarchical sheets:
      - parent: (sheet (property "Sheetname" ...) (property "Sheetfile" ...) (pin ... (at ...)))
      - child: (hierarchical_label "PINNAME" (at ...))

Limitations:
  - Only orthogonal wires are fully "split" by intermediate points. Non-orthogonal
    segments are connected by endpoints only.
  - Hierarchical connections are approximated by connecting parent sheet pins to
    child hierarchical labels of the same name. This is sufficient for the
    connectivity patterns used in this project.
"""

from __future__ import annotations

import argparse
import json
import math
import os
import re
import sys
from collections import defaultdict
from bisect import bisect_left, bisect_right
from dataclasses import dataclass
from typing import Any, Dict, Iterable, List, Optional, Sequence, Tuple


Token = str
Sexp = Any  # nested list/atoms


def tokenize(data: str) -> Iterable[Token]:
    i = 0
    n = len(data)
    while i < n:
        c = data[i]
        if c.isspace():
            i += 1
            continue
        if c == "(" or c == ")":
            yield c
            i += 1
            continue
        if c == '"':
            i += 1
            out = []
            while i < n:
                c2 = data[i]
                if c2 == "\\":
                    i += 1
                    if i >= n:
                        break
                    out.append(data[i])
                    i += 1
                    continue
                if c2 == '"':
                    i += 1
                    break
                out.append(c2)
                i += 1
            yield "".join(out)
            continue
        # atom
        j = i
        while j < n and (not data[j].isspace()) and data[j] not in "()":
            j += 1
        yield data[i:j]
        i = j


def parse_sexp(tokens: Iterable[Token]) -> Sexp:
    stack: List[List[Any]] = [[]]
    for tok in tokens:
        if tok == "(":
            new_list: List[Any] = []
            stack[-1].append(new_list)
            stack.append(new_list)
        elif tok == ")":
            if len(stack) == 1:
                raise ValueError("Unbalanced ')'")
            stack.pop()
        else:
            stack[-1].append(tok)
    if len(stack) != 1:
        raise ValueError("Unbalanced '('")
    if not stack[0]:
        raise ValueError("Empty document")
    return stack[0][0]


def is_list(node: Any) -> bool:
    return isinstance(node, list)


def head(node: Sexp) -> Optional[str]:
    if not is_list(node) or not node:
        return None
    h = node[0]
    return h if isinstance(h, str) else None


def find_first(node: Sexp, name: str) -> Optional[Sexp]:
    if not is_list(node):
        return None
    for child in node:
        if is_list(child) and child and child[0] == name:
            return child
    return None


def find_all(node: Sexp, name: str) -> List[Sexp]:
    out: List[Sexp] = []
    if not is_list(node):
        return out
    for child in node:
        if is_list(child) and child and child[0] == name:
            out.append(child)
    return out


def walk(node: Sexp) -> Iterable[Sexp]:
    if not is_list(node):
        return
    yield node
    for child in node:
        if is_list(child):
            yield from walk(child)


def try_float(s: str) -> Optional[float]:
    try:
        return float(s)
    except Exception:
        return None


SCALE = 1000  # coordinate quantization (KiCad schematics commonly use 0.001mm/0.001in steps)


def qcoord(x: float) -> int:
    return int(round(x * SCALE))


Point = Tuple[int, int]  # quantized


def rot_deg_to_rad(deg: float) -> float:
    return deg * (math.pi / 180.0)


def rotate_point(p: Point, rot_deg: float) -> Point:
    if rot_deg % 360 == 0:
        return p
    x, y = p
    r = rot_deg_to_rad(rot_deg)
    cos_r = math.cos(r)
    sin_r = math.sin(r)
    xr = x * cos_r - y * sin_r
    yr = x * sin_r + y * cos_r
    return (int(round(xr)), int(round(yr)))


def mirror_point(p: Point, mirror_x: bool, mirror_y: bool) -> Point:
    x, y = p
    # KiCad instance `(mirror x|y)` mirrors the symbol along that axis in the schematic plane.
    # Empirically for this schematic (KiCad v9 .kicad_sch):
    # - `mirror y` flips the local X coordinate (left/right mirror)
    # - `mirror x` flips the local Y coordinate (top/bottom mirror)
    if mirror_y:
        x = -x
    if mirror_x:
        y = -y
    return (x, y)


def add_points(a: Point, b: Point) -> Point:
    return (a[0] + b[0], a[1] + b[1])


@dataclass(frozen=True)
class SymbolPin:
    number: str
    name: str
    at: Point  # local position


@dataclass(frozen=True)
class InstancePin:
    ref: str
    lib_id: str
    unit: int
    pin_number: str
    pin_name: str
    pos: Point  # absolute schematic position


@dataclass(frozen=True)
class SheetPinEndpoint:
    sheet_name: str
    pin_name: str
    direction: str
    pos: Point  # absolute schematic position


@dataclass(frozen=True)
class LabelPoint:
    kind: str  # label | global_label | hierarchical_label
    name: str
    pos: Point


@dataclass(frozen=True)
class SheetPin:
    name: str
    direction: str
    pos: Point


@dataclass(frozen=True)
class SheetInstance:
    sheet_name: str
    sheet_file: str
    pins: List[SheetPin]


def parse_pin_from_lib(pin_node: Sexp) -> Optional[SymbolPin]:
    # (pin <type> <shape> (at x y rot) ... (name "PC8" ...) (number "54" ...) ...)
    if head(pin_node) != "pin":
        return None
    at_node = find_first(pin_node, "at")
    name_node = find_first(pin_node, "name")
    num_node = find_first(pin_node, "number")
    if not at_node or not name_node or not num_node:
        return None

    if len(at_node) < 3:
        return None
    x = try_float(at_node[1])
    y = try_float(at_node[2])
    if x is None or y is None:
        return None
    # rotation in (at ...) is pin orientation; the pin location itself is still x,y.
    name = name_node[1] if len(name_node) > 1 and isinstance(name_node[1], str) else ""
    number = num_node[1] if len(num_node) > 1 and isinstance(num_node[1], str) else ""
    return SymbolPin(number=number, name=name, at=(qcoord(x), qcoord(y)))


_SUBSYMBOL_UNIT_RE = re.compile(r".*_([0-9]+)_([0-9]+)$")


def lib_symbol_unit_pins(lib_symbol_node: Sexp, unit: int) -> List[SymbolPin]:
    # lib_symbol_node: (symbol "<lib_id>" ... (symbol "<name>_0_1" ...) (symbol "<name>_1_1" (pin ...) ... ) ...)
    pins: List[SymbolPin] = []
    for child in lib_symbol_node:
        if head(child) != "symbol":
            continue
        if len(child) < 2 or not isinstance(child[1], str):
            continue
        m = _SUBSYMBOL_UNIT_RE.match(child[1])
        if not m:
            continue
        sub_unit = int(m.group(1))
        if sub_unit != unit:
            continue
        for sub_child in child:
            if head(sub_child) == "pin":
                p = parse_pin_from_lib(sub_child)
                if p:
                    pins.append(p)
    return pins


def instance_transform_at(symbol_inst: Sexp) -> Tuple[Point, float, bool, bool]:
    at_node = find_first(symbol_inst, "at")
    if not at_node or len(at_node) < 3:
        return (0, 0), 0.0, False, False
    x = try_float(at_node[1]) or 0.0
    y = try_float(at_node[2]) or 0.0
    rot = try_float(at_node[3]) if len(at_node) > 3 else 0.0
    rot = rot or 0.0

    mirror_node = find_first(symbol_inst, "mirror")
    mirror_x = False
    mirror_y = False
    if mirror_node and len(mirror_node) >= 2:
        axis = mirror_node[1]
        mirror_x = axis == "x"
        mirror_y = axis == "y"

    return (qcoord(x), qcoord(y)), rot, mirror_x, mirror_y


def instance_ref(symbol_inst: Sexp) -> str:
    for child in symbol_inst:
        if head(child) == "property" and len(child) >= 3 and child[1] == "Reference":
            return child[2]
    return "?"


def instance_lib_id(symbol_inst: Sexp) -> str:
    lib_id = find_first(symbol_inst, "lib_id")
    if lib_id and len(lib_id) >= 2:
        return lib_id[1]
    return ""


def instance_unit(symbol_inst: Sexp) -> int:
    unit_node = find_first(symbol_inst, "unit")
    if unit_node and len(unit_node) >= 2:
        try:
            return int(unit_node[1])
        except Exception:
            return 1
    return 1


def extract_instances(root: Sexp) -> List[Sexp]:
    return [n for n in walk(root) if head(n) == "symbol" and find_first(n, "lib_id") is not None]


def extract_lib_symbols(root: Sexp) -> Dict[str, Sexp]:
    lib_symbols = find_first(root, "lib_symbols")
    if not lib_symbols:
        return {}
    out: Dict[str, Sexp] = {}
    for child in lib_symbols:
        if head(child) != "symbol":
            continue
        if len(child) >= 2 and isinstance(child[1], str):
            out[child[1]] = child
    return out


def extract_labels(root: Sexp) -> List[Tuple[str, Point]]:
    # Back-compat shim; prefer `extract_label_points`.
    labels: List[Tuple[str, Point]] = []
    for n in walk(root):
        if head(n) not in ("label", "global_label", "hierarchical_label"):
            continue
        if len(n) < 2 or not isinstance(n[1], str):
            continue
        at_node = find_first(n, "at")
        if not at_node or len(at_node) < 3:
            continue
        x = try_float(at_node[1])
        y = try_float(at_node[2])
        if x is None or y is None:
            continue
        labels.append((n[1], (qcoord(x), qcoord(y))))
    return labels


def extract_label_points(root: Sexp) -> List[LabelPoint]:
    labels: List[LabelPoint] = []
    for n in walk(root):
        k = head(n)
        if k not in ("label", "global_label", "hierarchical_label"):
            continue
        if len(n) < 2 or not isinstance(n[1], str):
            continue
        at_node = find_first(n, "at")
        if not at_node or len(at_node) < 3:
            continue
        x = try_float(at_node[1])
        y = try_float(at_node[2])
        if x is None or y is None:
            continue
        labels.append(LabelPoint(kind=k, name=n[1], pos=(qcoord(x), qcoord(y))))
    return labels


def extract_junctions(root: Sexp) -> List[Point]:
    pts: List[Point] = []
    for n in walk(root):
        if head(n) != "junction":
            continue
        at_node = find_first(n, "at")
        if not at_node or len(at_node) < 3:
            continue
        x = try_float(at_node[1])
        y = try_float(at_node[2])
        if x is None or y is None:
            continue
        pts.append((qcoord(x), qcoord(y)))
    return pts


def extract_wires(root: Sexp) -> List[List[Point]]:
    wires: List[List[Point]] = []
    for n in walk(root):
        if head(n) != "wire":
            continue
        pts_node = find_first(n, "pts")
        if not pts_node:
            continue
        pts: List[Point] = []
        for xy in pts_node:
            if head(xy) != "xy" or len(xy) < 3:
                continue
            x = try_float(xy[1])
            y = try_float(xy[2])
            if x is None or y is None:
                continue
            pts.append((qcoord(x), qcoord(y)))
        if len(pts) >= 2:
            wires.append(pts)
    return wires


def sheet_instance_name(sheet_node: Sexp) -> str:
    # (sheet ... (property "Sheetname" "BANK1" ...) ...)
    if not is_list(sheet_node):
        return "?"
    for child in sheet_node:
        if head(child) == "property" and len(child) >= 3 and child[1] == "Sheetname":
            return child[2]
    return "?"


def extract_sheet_pins(root: Sexp) -> List[SheetPinEndpoint]:
    """
    Extract hierarchical sheet pins from `(sheet ...)` blocks.

    These appear in the parent schematic at absolute coordinates and participate
    in net connectivity like symbol pins.
    """
    out: List[SheetPinEndpoint] = []
    for n in walk(root):
        if head(n) != "sheet":
            continue
        sname = sheet_instance_name(n)
        for child in n:
            if head(child) != "pin":
                continue
            # (pin "LS1" input (at x y rot) ...)
            if len(child) < 3:
                continue
            pin_name = child[1] if isinstance(child[1], str) else ""
            direction = child[2] if isinstance(child[2], str) else ""
            at_node = find_first(child, "at")
            if not at_node or len(at_node) < 3:
                continue
            x = try_float(at_node[1])
            y = try_float(at_node[2])
            if x is None or y is None:
                continue
            out.append(
                SheetPinEndpoint(
                    sheet_name=sname,
                    pin_name=pin_name,
                    direction=direction,
                    pos=(qcoord(x), qcoord(y)),
                )
            )
    return out


def sheet_instance_file(sheet_node: Sexp) -> str:
    if not is_list(sheet_node):
        return ""
    for child in sheet_node:
        if head(child) == "property" and len(child) >= 3 and child[1] == "Sheetfile":
            return child[2]
    return ""


def extract_sheet_instances(root: Sexp) -> List[SheetInstance]:
    out: List[SheetInstance] = []
    for n in walk(root):
        if head(n) != "sheet":
            continue
        sname = sheet_instance_name(n)
        sfile = sheet_instance_file(n)
        pins: List[SheetPin] = []
        for child in n:
            if head(child) != "pin" or len(child) < 3:
                continue
            pin_name = child[1] if isinstance(child[1], str) else ""
            direction = child[2] if isinstance(child[2], str) else ""
            at_node = find_first(child, "at")
            if not at_node or len(at_node) < 3:
                continue
            x = try_float(at_node[1])
            y = try_float(at_node[2])
            if x is None or y is None:
                continue
            pins.append(SheetPin(name=pin_name, direction=direction, pos=(qcoord(x), qcoord(y))))
        out.append(SheetInstance(sheet_name=sname, sheet_file=sfile, pins=pins))
    return out


@dataclass
class ParsedDoc:
    doc_path: str
    instance_prefix: str
    endpoints: List[Tuple[int, Dict[str, Any]]]  # (comp_root_id, endpoint_dict)
    labels: List[Tuple[int, LabelPoint]]         # (comp_root_id, label)
    sheet_pins: List[Tuple[int, SheetPinEndpoint]]  # (comp_root_id, sheet_pin)
    sheets: List[SheetInstance]
    mcu_ref: str


def parse_doc(
    schematic_path: str,
    *,
    instance_prefix: str,
    mcu_ref: str,
    include_sheets: bool,
) -> ParsedDoc:
    with open(schematic_path, "r", encoding="utf-8") as f:
        data = f.read()

    root = parse_sexp(tokenize(data))
    lib_symbols = extract_lib_symbols(root)
    instances = extract_instances(root)
    label_points = extract_label_points(root)
    junctions = extract_junctions(root)
    wires = extract_wires(root)

    sheets = extract_sheet_instances(root) if include_sheets else []
    sheet_pins = extract_sheet_pins(root) if include_sheets else []

    # Build instance pins (all symbols) so we can list endpoints per net.
    instance_pins: List[InstancePin] = []
    for inst in instances:
        lib_id = instance_lib_id(inst)
        if not lib_id:
            continue
        ref = instance_ref(inst)
        if instance_prefix:
            ref = f"{instance_prefix}{ref}"
        unit = instance_unit(inst)
        lib_sym = lib_symbols.get(lib_id)
        if not lib_sym:
            continue
        pos0, rot, mirror_x, mirror_y = instance_transform_at(inst)
        for p in lib_symbol_unit_pins(lib_sym, unit):
            local = mirror_point(p.at, mirror_x=mirror_x, mirror_y=mirror_y)
            abs_pos = add_points(pos0, rotate_point(local, rot))
            instance_pins.append(
                InstancePin(
                    ref=ref,
                    lib_id=lib_id,
                    unit=unit,
                    pin_number=p.number,
                    pin_name=p.name,
                    pos=abs_pos,
                )
            )

    # Snap labels to nearby wires/pins.
    label_snap_tol = qcoord(2.0)
    pin_points_all: List[Point] = [p.pos for p in instance_pins] + [p.pos for p in sheet_pins]
    snapped_labels: List[LabelPoint] = [
        LabelPoint(kind=lp.kind, name=lp.name, pos=snap_point(lp.pos, wires, pin_points_all, label_snap_tol))
        for lp in label_points
    ]

    # Points participating in connectivity graph.
    wire_points: set[Point] = set(junctions)
    for poly in wires:
        for p in poly:
            wire_points.add(p)
    pin_points: set[Point] = set(p.pos for p in instance_pins)
    sheet_pin_points: set[Point] = set(p.pos for p in sheet_pins)
    label_points_set: set[Point] = set(p.pos for p in snapped_labels)

    points_set: set[Point] = set()
    points_set |= wire_points
    points_set |= pin_points
    points_set |= sheet_pin_points
    points_set |= label_points_set

    points = sorted(points_set)
    dsu = build_connectivity(wires, points)
    idx: Dict[Point, int] = {p: i for i, p in enumerate(points)}

    # Component -> labels present in this doc (by DSU root id).
    comp_labels: Dict[int, List[LabelPoint]] = defaultdict(list)
    for lp in snapped_labels:
        cid = dsu.find(idx[lp.pos])
        comp_labels[cid].append(lp)

    # Union by label name to emulate KiCad label behavior within *this* schematic file.
    # Only union labels that are actually attached to something (wire/junction/pin/sheetpin) after snapping.
    comp_has_other: Dict[int, bool] = {}
    for pt in (wire_points | pin_points | sheet_pin_points):
        cid = dsu.find(idx[pt])
        comp_has_other[cid] = True

    label_first_component: Dict[str, int] = {}
    for cid, lps in list(comp_labels.items()):
        if not comp_has_other.get(cid, False):
            continue
        for lp in lps:
            # In KiCad, label kinds still represent the same named net within a sheet
            # (e.g. a `label` and a `hierarchical_label` of the same name).
            if lp.name in label_first_component:
                dsu.union(cid, label_first_component[lp.name])
            else:
                label_first_component[lp.name] = cid

    # Rebuild label list after unions.
    labels_after: List[Tuple[int, LabelPoint]] = []
    seen = set()
    for lp in snapped_labels:
        cid = dsu.find(idx[lp.pos])
        k = (cid, lp.kind, lp.name)
        if k in seen:
            continue
        seen.add(k)
        labels_after.append((cid, lp))

    endpoints: List[Tuple[int, Dict[str, Any]]] = []
    for pin in instance_pins:
        cid = dsu.find(idx[pin.pos])
        endpoints.append(
            (
                cid,
                {
                    "ref": pin.ref,
                    "pin_number": pin.pin_number,
                    "pin_name": pin.pin_name,
                    "lib_id": pin.lib_id,
                },
            )
        )

    sheet_endpoints: List[Tuple[int, SheetPinEndpoint]] = []
    for sp in sheet_pins:
        cid = dsu.find(idx[sp.pos])
        sheet_endpoints.append((cid, sp))

    return ParsedDoc(
        doc_path=os.path.abspath(schematic_path),
        instance_prefix=instance_prefix,
        endpoints=endpoints,
        labels=labels_after,
        sheet_pins=sheet_endpoints,
        sheets=sheets,
        mcu_ref=mcu_ref,
    )


class DSU:
    def __init__(self, n: int):
        self.parent = list(range(n))
        self.rank = [0] * n

    def find(self, a: int) -> int:
        while self.parent[a] != a:
            self.parent[a] = self.parent[self.parent[a]]
            a = self.parent[a]
        return a

    def union(self, a: int, b: int) -> None:
        ra = self.find(a)
        rb = self.find(b)
        if ra == rb:
            return
        if self.rank[ra] < self.rank[rb]:
            self.parent[ra] = rb
        elif self.rank[ra] > self.rank[rb]:
            self.parent[rb] = ra
        else:
            self.parent[rb] = ra
            self.rank[ra] += 1


def build_connectivity(
    wires: Sequence[Sequence[Point]],
    points: Sequence[Point],
) -> DSU:
    idx: Dict[Point, int] = {p: i for i, p in enumerate(points)}
    dsu = DSU(len(points))

    # Build point indexes per axis for fast slicing.
    points_by_y: Dict[int, List[int]] = {}
    points_by_x: Dict[int, List[int]] = {}
    for x, y in points:
        points_by_y.setdefault(y, []).append(x)
        points_by_x.setdefault(x, []).append(y)
    for y, xs in points_by_y.items():
        xs.sort()
    for x, ys in points_by_x.items():
        ys.sort()

    def union_chain(chain: List[Point]) -> None:
        for a, b in zip(chain, chain[1:]):
            dsu.union(idx[a], idx[b])

    for poly in wires:
        for p1, p2 in zip(poly, poly[1:]):
            x1, y1 = p1
            x2, y2 = p2
            if x1 == x2:
                # vertical
                x = x1
                y_lo, y_hi = (y1, y2) if y1 <= y2 else (y2, y1)
                ys = points_by_x.get(x, [])
                lo = bisect_left(ys, y_lo)
                hi = bisect_right(ys, y_hi)
                chain = [(x, y) for y in ys[lo:hi]]
                if len(chain) < 2:
                    chain = [p1, p2] if p1 != p2 else [p1]
                union_chain(chain)
            elif y1 == y2:
                # horizontal
                y = y1
                x_lo, x_hi = (x1, x2) if x1 <= x2 else (x2, x1)
                xs = points_by_y.get(y, [])
                lo = bisect_left(xs, x_lo)
                hi = bisect_right(xs, x_hi)
                chain = [(x, y) for x in xs[lo:hi]]
                if len(chain) < 2:
                    chain = [p1, p2] if p1 != p2 else [p1]
                union_chain(chain)
            else:
                # diagonal/odd: connect endpoints only
                if p1 in idx and p2 in idx:
                    dsu.union(idx[p1], idx[p2])
    return dsu


def snap_point(pt: Point, wires: Sequence[Sequence[Point]], other_points: Sequence[Point], tol: int) -> Point:
    """
    Snap a point to the nearest orthogonal wire segment or existing point within `tol` (quantized units).
    Returns the snapped point, or the original point if no nearby segment is found.
    """
    x0, y0 = pt
    best: Optional[Point] = None
    best_d2 = (tol + 1) * (tol + 1)

    def consider(candidate: Point) -> None:
        nonlocal best, best_d2
        dx = candidate[0] - x0
        dy = candidate[1] - y0
        d2 = dx * dx + dy * dy
        if d2 < best_d2:
            best_d2 = d2
            best = candidate

    for poly in wires:
        for p1, p2 in zip(poly, poly[1:]):
            x1, y1 = p1
            x2, y2 = p2
            if x1 == x2:
                # vertical segment
                x = x1
                y_lo, y_hi = (y1, y2) if y1 <= y2 else (y2, y1)
                if abs(x0 - x) > tol:
                    continue
                if y0 < y_lo - tol or y0 > y_hi + tol:
                    continue
                y = min(max(y0, y_lo), y_hi)
                consider((x, y))
            elif y1 == y2:
                # horizontal segment
                y = y1
                x_lo, x_hi = (x1, x2) if x1 <= x2 else (x2, x1)
                if abs(y0 - y) > tol:
                    continue
                if x0 < x_lo - tol or x0 > x_hi + tol:
                    continue
                x = min(max(x0, x_lo), x_hi)
                consider((x, y))
            else:
                # ignore non-orthogonal for snapping
                continue

    # Also consider snapping directly onto a known point (symbol pin / junction), since labels are
    # often placed on pin stubs without an explicit wire segment.
    for p in other_points:
        dx = p[0] - x0
        dy = p[1] - y0
        if abs(dx) > tol or abs(dy) > tol:
            continue
        d2 = dx * dx + dy * dy
        if d2 < best_d2:
            best_d2 = d2
            best = p

    return best if best is not None else pt


def main(argv: Sequence[str]) -> int:
    ap = argparse.ArgumentParser()
    ap.add_argument("--schematic", default="GDI-STM.kicad_sch")
    ap.add_argument("--mcu-ref", default="U7")
    ap.add_argument("--json-out", default="")
    ap.add_argument("--only-labeled", action="store_true", help="Only report nets with explicit labels")
    args = ap.parse_args(argv)

    if not os.path.exists(args.schematic):
        print(f"error: schematic not found: {args.schematic}", file=sys.stderr)
        return 2

    root_dir = os.path.dirname(os.path.abspath(args.schematic))

    # Parse top-level.
    docs: List[ParsedDoc] = [
        parse_doc(args.schematic, instance_prefix="", mcu_ref=args.mcu_ref, include_sheets=True)
    ]

    # Parse hierarchical sheets as separate *instances* (scope isolation), then connect via sheet pins.
    for sheet in docs[0].sheets:
        if not sheet.sheet_file:
            continue
        sheet_path = os.path.join(root_dir, sheet.sheet_file)
        if not os.path.exists(sheet_path):
            continue
        prefix = f"{sheet.sheet_name}/"
        docs.append(parse_doc(sheet_path, instance_prefix=prefix, mcu_ref=args.mcu_ref, include_sheets=False))

    # Build a global DSU over "doc-local connectivity components" (represented by the DSU root ids from each doc).
    # We assign a unique node per (doc_index, comp_root_id).
    node_for_comp: Dict[Tuple[int, int], int] = {}
    comp_for_node: List[Tuple[int, int]] = []

    def get_node(doc_i: int, comp_root: int) -> int:
        key = (doc_i, comp_root)
        if key in node_for_comp:
            return node_for_comp[key]
        nid = len(comp_for_node)
        node_for_comp[key] = nid
        comp_for_node.append(key)
        return nid

    # Gather label membership per component node.
    labels_by_node: Dict[int, List[LabelPoint]] = defaultdict(list)
    global_label_nodes_by_name: Dict[str, List[int]] = defaultdict(list)
    hier_label_nodes_by_doc_and_name: Dict[Tuple[int, str], List[int]] = defaultdict(list)
    any_label_nodes_by_doc_and_name: Dict[Tuple[int, str], List[int]] = defaultdict(list)

    for di, doc in enumerate(docs):
        for comp_root, lp in doc.labels:
            nid = get_node(di, comp_root)
            labels_by_node[nid].append(lp)
            any_label_nodes_by_doc_and_name[(di, lp.name)].append(nid)
            if lp.kind == "global_label":
                global_label_nodes_by_name[lp.name].append(nid)
            if lp.kind == "hierarchical_label":
                hier_label_nodes_by_doc_and_name[(di, lp.name)].append(nid)

    # Ensure we have nodes for all endpoint-bearing components too (not only label-bearing ones).
    for di, doc in enumerate(docs):
        for comp_root, _ep in doc.endpoints:
            get_node(di, comp_root)
        for comp_root, _sp in doc.sheet_pins:
            get_node(di, comp_root)

    global_dsu = DSU(len(comp_for_node))

    # Union global labels across all docs/instances (project-wide behavior).
    for name, nodes in global_label_nodes_by_name.items():
        if len(nodes) < 2:
            continue
        base = nodes[0]
        for other in nodes[1:]:
            global_dsu.union(base, other)

    # Connect sheet pins in top-level to hierarchical labels in the corresponding sheet instance.
    # We assume one-level hierarchy: docs[0] is top-level, docs[1..] are direct children.
    # Multiple instances of the same file are parsed separately and thus isolated.
    top = docs[0]
    child_by_sheetname: Dict[str, int] = {}
    for i in range(1, len(docs)):
        # instance_prefix is "BANK1/" etc.
        prefix = docs[i].instance_prefix
        if prefix.endswith("/"):
            prefix = prefix[:-1]
        if prefix:
            child_by_sheetname[prefix] = i

    for comp_root, sp in top.sheet_pins:
        child_i = child_by_sheetname.get(sp.sheet_name)
        if child_i is None:
            continue
        parent_node = get_node(0, comp_root)

        # Prefer hierarchical_label in child, otherwise fall back to any label of same name.
        child_nodes = hier_label_nodes_by_doc_and_name.get((child_i, sp.pin_name), [])
        if not child_nodes:
            child_nodes = any_label_nodes_by_doc_and_name.get((child_i, sp.pin_name), [])
        for cn in child_nodes:
            global_dsu.union(parent_node, cn)

    # Helper to pick a stable net name for a global DSU component.
    def net_name_for_group(nodes: List[int]) -> str:
        """
        Pick a stable name for a merged net.

        In this repo, top-level labels are the authoritative net names, while child-sheet
        hierarchical labels often use generic pin names like H1/L1 that would collide
        across instances. Therefore, we prefer labels originating from the top-level
        schematic (doc index 0) when present.
        """
        top_globals: List[str] = []
        top_locals: List[str] = []
        globals_: List[str] = []
        locals_: List[str] = []
        hiers: List[str] = []

        for nid in nodes:
            doc_i, _comp_root = comp_for_node[nid]
            for lp in labels_by_node.get(nid, []):
                if lp.kind == "global_label":
                    if doc_i == 0:
                        top_globals.append(lp.name)
                    globals_.append(lp.name)
                elif lp.kind == "label":
                    if doc_i == 0:
                        top_locals.append(lp.name)
                    locals_.append(lp.name)
                elif lp.kind == "hierarchical_label":
                    hiers.append(lp.name)
                else:
                    if doc_i == 0:
                        top_locals.append(lp.name)
                    locals_.append(lp.name)

        for pool in (top_globals, top_locals, globals_, locals_, hiers):
            if pool:
                return sorted(set(pool))[0]
        return ""

    # Build final group membership and names.
    groups: Dict[int, List[int]] = defaultdict(list)
    for nid in range(len(comp_for_node)):
        groups[global_dsu.find(nid)].append(nid)

    group_name: Dict[int, str] = {}
    for gid, members in groups.items():
        name = net_name_for_group(members)
        group_name[gid] = name if name else f"<unlabeled:{gid}>"

    # Emit endpoints per net name.
    net_to_endpoints: Dict[str, List[Dict[str, Any]]] = {}
    mcu_net_to_pins: Dict[str, List[Dict[str, Any]]] = {}

    def add_endpoint(net: str, ep: Dict[str, Any]) -> None:
        if args.only_labeled and net.startswith("<unlabeled:"):
            return
        net_to_endpoints.setdefault(net, []).append(ep)

    # Symbol endpoints.
    for di, doc in enumerate(docs):
        for comp_root, ep in doc.endpoints:
            nid = get_node(di, comp_root)
            net = group_name[global_dsu.find(nid)]
            add_endpoint(net, ep)
            if ep["ref"] == args.mcu_ref and not net.startswith("<unlabeled:"):
                mcu_net_to_pins.setdefault(net, []).append({"mcu_pin": ep["pin_name"], "package_pin": ep["pin_number"]})

    # Parent sheet pins as endpoints.
    for comp_root, sp in top.sheet_pins:
        nid = get_node(0, comp_root)
        net = group_name[global_dsu.find(nid)]
        add_endpoint(
            net,
            {
                "ref": f"SHEET:{sp.sheet_name}",
                "pin_number": "",
                "pin_name": sp.pin_name,
                "lib_id": "__sheet__",
            },
        )

    out = {
        "schematic": os.path.basename(args.schematic),
        "mcu_ref": args.mcu_ref,
        "mcu_net_to_pins": {k: sorted(v, key=lambda e: (e["mcu_pin"], e["package_pin"])) for k, v in sorted(mcu_net_to_pins.items())},
        "net_to_endpoints": {k: v for k, v in sorted(net_to_endpoints.items())},
    }

    if args.json_out:
        with open(args.json_out, "w", encoding="utf-8") as f:
            json.dump(out, f, indent=2, sort_keys=True)
        print(args.json_out)
    else:
        json.dump(out, sys.stdout, indent=2, sort_keys=True)
        print()
    return 0


if __name__ == "__main__":
    raise SystemExit(main(sys.argv[1:]))
