"""Parser for the HW-QC stim-rig (digital-inputs firmware) USB-serial console stream.

The tester firmware in ``digital-inputs/firmware`` runs one test cycle at a time and
prints an ANSI-coloured human report to its USB CDC console. This module turns that
stream back into structured state: which analog channels / input counters / outputs /
DC channels the rig itself judged HAPPY vs not, plus board id, meta counts and the
overall ALL GOOD / SOMETHING BAD verdict.

It is a *pure* line parser (no serial, no I/O) so it can be unit-tested against a
captured log and reused by the MCP server and the CLI. Every pattern here is matched
1:1 against a ``chprintf(chp, ...)`` format string in the firmware; keep them in sync
if the firmware console strings change (main.cpp, can.cpp, test_digital_outputs.cpp,
test_logic.cpp).

A "cycle" is everything the firmware prints between two verdict lines. feed_line()
accumulates into an in-progress cycle and, on a verdict line (ALL GOOD / SOMETHING
BAD), commits it to ``last_cycle`` and starts a fresh one.
"""
from __future__ import annotations

import re
from dataclasses import dataclass, field, asdict
from typing import Optional

# ANSI SGR colour codes the firmware emits via terminal_util.cpp
_ANSI = re.compile(r"\x1b\[[0-9;]*m")


def strip_ansi(s: str) -> str:
    return _ANSI.sub("", s).replace("\r", "").rstrip("\n")


# --- line patterns (each maps to one firmware format string) -------------------
_P = {
    # verdict / cycle framing (main.cpp)
    "listen":      re.compile(r"^\s*Listen mode\s*$"),
    "version":     re.compile(r"\*+\s*version=(\d+)"),
    "all_good":    re.compile(r"ALL GOOD\b.*?uptime=(\d+)"),
    "bad":         re.compile(r"SOMETHING BAD SEE ABOVE ERRORS=(\d+)\b.*?count=(\d+)"),
    "error_rate":  re.compile(r"error rate:\s*(\d+)\s+of\s+(\d+)"),
    "low_uptime":  re.compile(r"uptime is too low uptime=(\d+)"),
    # board detection (can.cpp)
    "board":       re.compile(r"Board detected:\s*(.+?)\s+rev index\s+(\d+)"),
    "no_board":    re.compile(r"no board detected, CAN test not happy"),
    "board_none":  re.compile(r"BOARD NOT DETECTED no BOARD_STATUS"),
    "unknown_id":  re.compile(r"UNKNOWN BOARD ID \[(\d+)\]"),
    "detect_err":  re.compile(r"Couldn't detect, unknown board"),
    # meta / status (can.cpp)
    "meta":        re.compile(r"total=(\d+) outputs of which low side=(\d+) also (\d+) DC"),
    "bstatus":     re.compile(r"BoardStatus:\s*BoardID=(\d+)\s+numSecs=(\d+)"),
    # input counters (can.cpp handleCounter)
    "counter":     re.compile(r"\*?\s*(HAPPY|ZERO)\s+(.+?)\s+(event|button|aux digital)\s+counter!"),
    # analog channels (can.cpp)
    "analog_ok":   re.compile(r"\*+\s+(\S+)\s+analog\s+(\d+)\s+(\d+)\s+voltage=([\d.]+)\s+within range\s+([\d.]+)/([\d.]+)"),
    "analog_bad":  re.compile(r"BAD analog channel\s+(\d+)\s+\(([^)]*)\):\s*voltage\s+([\d.]+)\s+\(raw\s+(\d+)\)\s+not in range\s+\(([\d.]+)\.\.([\d.]+)\)\s+mult=([\d.]+)"),
    # digital outputs (test_digital_outputs.cpp + test_logic.cpp)
    "num_outputs": re.compile(r"numOutputs\s+(\d+)"),
    "out_fail":    re.compile(r"ERROR! Line\s+(\d+)@(\d+)\s+FAILED!\s+\(set\s+(\d+),\s+received\s+(\d+)\s+([\d.]+)v\)"),
    "zero_out":    re.compile(r"ZERO OUTPUTS CONFIGURED"),
    "map_err":     re.compile(r"Output test-pad map does not match"),
    # DC outputs (test_digital_outputs.cpp)
    "dc_good":     re.compile(r"GOOD DC=(\d+)"),
    # pad discovery diagnostic (test_digital_outputs.cpp)
    "pad_head":    re.compile(r"PAD-DISCOVERY ECU out\s+(\d+)\s+\(([^)]*)\)"),
    "pad_hit":     re.compile(r"stim pad\s+(\d+)\s+\(addr\s+(\d+)\s+bank\s+(\d+)\):\s*low=([\d.]+)v\s+high=([\d.]+)v\s+delta=([\d.]+)v"),
    "pad_none":    re.compile(r"NONE - ECU out\s+(\d+)\s+reaches no stim pad"),
}


@dataclass
class Cycle:
    """One parsed stim test cycle."""
    verdict: Optional[str] = None            # "ALL_GOOD" | "BAD" | None (in progress)
    uptime_s: Optional[int] = None
    errors: Optional[int] = None             # from SOMETHING BAD ERRORS=
    version: Optional[int] = None
    listen_mode: bool = False
    low_uptime: bool = False
    board_detected: bool = False
    board_name: Optional[str] = None
    board_rev: Optional[int] = None
    board_id: Optional[int] = None           # decimal board id from BoardStatus
    num_secs: Optional[int] = None
    meta_outputs: Optional[int] = None
    meta_low_side: Optional[int] = None
    meta_dc: Optional[int] = None
    board_errors: list = field(default_factory=list)  # textual board-level problems
    counters: dict = field(default_factory=dict)      # name -> {kind, happy}
    analog: dict = field(default_factory=dict)        # name -> {ok, voltage, min, max, raw, channel}
    output_fails: dict = field(default_factory=dict)  # line -> {addr, set, received, voltage}
    dc_good: list = field(default_factory=list)       # DC indices the rig passed
    pad_discovery: list = field(default_factory=list) # [{out, name, pads:[...]}]

    # ---- derived summaries ----------------------------------------------------
    def unhappy_analog(self) -> list:
        return [dict(name=n, **v) for n, v in self.analog.items() if not v["ok"]]

    def unhappy_counters(self) -> list:
        return [dict(name=n, **v) for n, v in self.counters.items() if not v["happy"]]

    def unhappy_outputs(self) -> list:
        return [dict(line=int(l), **v) for l, v in self.output_fails.items()]

    def dc_status(self) -> dict:
        """Answer 'is DC happy?' from the rig's own GOOD DC= lines."""
        dc = self.meta_dc
        if dc is None:
            return {"applicable": None, "happy": None, "note": "no meta yet"}
        if dc == 0:
            return {"applicable": False, "happy": None,
                    "note": "board advertises 0 DC channels (n/a)"}
        want = set(range(dc))
        got = set(self.dc_good)
        missing = sorted(want - got)
        return {"applicable": True, "happy": not missing,
                "dc_count": dc, "passed": sorted(got), "missing": missing,
                "note": "all DC channels passed" if not missing
                        else f"DC index {missing} did not report GOOD DC="}

    def is_happy(self) -> Optional[bool]:
        if self.verdict == "ALL_GOOD":
            return True
        if self.verdict == "BAD":
            return False
        return None  # in progress

    def summary(self) -> dict:
        return {
            "verdict": self.verdict,
            "happy": self.is_happy(),
            "uptime_s": self.uptime_s if self.uptime_s is not None else self.num_secs,
            "listen_mode": self.listen_mode,
            "board": {"name": self.board_name, "rev": self.board_rev,
                      "id": self.board_id, "detected": self.board_detected},
            "meta": {"outputs": self.meta_outputs, "low_side": self.meta_low_side,
                     "dc": self.meta_dc},
            "errors": self.errors,
            "board_errors": self.board_errors,
            "unhappy": {
                "analog": self.unhappy_analog(),
                "counters": self.unhappy_counters(),
                "outputs": self.unhappy_outputs(),
            },
            "dc": self.dc_status(),
            "counts": {
                "analog_checked": len(self.analog),
                "analog_bad": len(self.unhappy_analog()),
                "counters_checked": len(self.counters),
                "counters_zero": len(self.unhappy_counters()),
                "output_fails": len(self.output_fails),
            },
        }

    def as_dict(self) -> dict:
        return asdict(self)


class StimParser:
    def __init__(self):
        self.cur = Cycle()
        self.last_cycle: Optional[Cycle] = None
        self.cycles_seen = 0
        self._pad_cur: Optional[dict] = None

    def feed_line(self, raw: str) -> Optional[Cycle]:
        """Parse one console line. Returns the committed Cycle on a verdict line, else None."""
        line = strip_ansi(raw)
        if not line.strip():
            return None
        c = self.cur

        m = _P["listen"].search(line)
        if m:
            c.listen_mode = True
            return None

        m = _P["board"].search(line)
        if m:
            c.board_detected = True
            c.board_name = m.group(1)
            c.board_rev = int(m.group(2))
            return None

        m = _P["bstatus"].search(line)
        if m:
            c.board_id = int(m.group(1))
            c.num_secs = int(m.group(2))
            return None

        m = _P["meta"].search(line)
        if m:
            c.meta_outputs = int(m.group(1))
            c.meta_low_side = int(m.group(2))
            c.meta_dc = int(m.group(3))
            return None

        m = _P["counter"].search(line)
        if m:
            happy = m.group(1) == "HAPPY"
            c.counters[m.group(2).strip()] = {"kind": m.group(3), "happy": happy}
            return None

        m = _P["analog_ok"].search(line)
        if m:
            name = m.group(1)
            c.analog[name] = {"ok": True, "channel": int(m.group(3)),
                              "voltage": float(m.group(4)),
                              "min": float(m.group(5)), "max": float(m.group(6)),
                              "raw": None}
            return None

        m = _P["analog_bad"].search(line)
        if m:
            name = m.group(2)
            c.analog[name] = {"ok": False, "channel": int(m.group(1)),
                              "voltage": float(m.group(3)), "raw": int(m.group(4)),
                              "min": float(m.group(5)), "max": float(m.group(6)),
                              "mult": float(m.group(7))}
            return None

        m = _P["out_fail"].search(line)
        if m:
            c.output_fails[int(m.group(1))] = {
                "addr": int(m.group(2)), "set": int(m.group(3)),
                "received": int(m.group(4)), "voltage": float(m.group(5))}
            return None

        m = _P["dc_good"].search(line)
        if m:
            idx = int(m.group(1))
            if idx not in c.dc_good:
                c.dc_good.append(idx)
            return None

        m = _P["pad_head"].search(line)
        if m:
            self._pad_cur = {"out": int(m.group(1)), "name": m.group(2), "pads": []}
            c.pad_discovery.append(self._pad_cur)
            return None
        m = _P["pad_hit"].search(line)
        if m and self._pad_cur is not None:
            self._pad_cur["pads"].append({
                "pad": int(m.group(1)), "addr": int(m.group(2)), "bank": int(m.group(3)),
                "low": float(m.group(4)), "high": float(m.group(5)), "delta": float(m.group(6))})
            return None
        m = _P["pad_none"].search(line)
        if m:
            c.pad_discovery.append({"out": int(m.group(1)), "name": None, "pads": []})
            return None

        for key, msg in (("no_board", "no board detected"),
                         ("board_none", "no BOARD_STATUS, cannot test outputs"),
                         ("detect_err", "unknown board id (not in tester table)"),
                         ("zero_out", "ZERO OUTPUTS CONFIGURED"),
                         ("map_err", "output test-pad map mismatch")):
            if _P[key].search(line):
                c.board_errors.append(msg)
                return None
        m = _P["unknown_id"].search(line)
        if m:
            c.board_errors.append(f"unknown board id [{m.group(1)}]")
            return None

        m = _P["low_uptime"].search(line)
        if m:
            c.low_uptime = True
            c.uptime_s = int(m.group(1))
            return None

        m = _P["error_rate"].search(line)
        if m:
            c.error_rate = {"bad": int(m.group(1)), "good": int(m.group(2))}
            return None

        m = _P["version"].search(line)
        if m:
            c.version = int(m.group(1))
            return None

        # verdict lines commit the cycle
        m = _P["all_good"].search(line)
        if m:
            c.verdict = "ALL_GOOD"
            c.uptime_s = int(m.group(1))
            return self._commit()
        m = _P["bad"].search(line)
        if m:
            c.verdict = "BAD"
            c.errors = int(m.group(1))
            return self._commit()
        return None

    def _commit(self) -> Cycle:
        self.last_cycle = self.cur
        self.cycles_seen += 1
        committed = self.cur
        self.cur = Cycle()
        self._pad_cur = None
        return committed

    def current(self) -> Cycle:
        """Best available view: last completed cycle, else the in-progress one."""
        return self.last_cycle if self.last_cycle is not None else self.cur
