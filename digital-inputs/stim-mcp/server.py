#!/usr/bin/env python3
"""Stim-rig MCP server: exposes the parsed HW-QC tester console as MCP tools.

Zero third-party MCP dependency: speaks the MCP stdio transport (newline-delimited
JSON-RPC 2.0) directly, so it runs with only python3 + pyserial. Reads the rig's USB
serial, parses it with stim_parser, and answers questions like "which channels are
not happy?" / "is DC happy?" from the rig's own verdicts.

Config via env:
  STIM_SERIAL_PORT   explicit device (default: autodetect /dev/ttyACM* then ttyUSB*)
  STIM_SERIAL_BAUD   nominal baud for the CDC port (default 115200)

Register in Claude Code (.mcp.json), see README.md.
"""
from __future__ import annotations

import json
import os
import sys

from stim_reader import StimReader

PROTOCOL_VERSION = "2024-11-05"
reader: StimReader


def _fresh(wait_s: float = 0.0):
    """Optionally wait for a fresh cycle, then return the best cycle view."""
    if wait_s > 0:
        c = reader.wait_for_cycle(wait_s)
        if c is not None:
            return c
    return reader.current_cycle()


def _text(obj) -> dict:
    return {"content": [{"type": "text", "text": json.dumps(obj, indent=2)}]}


# -- tool implementations -------------------------------------------------------
def tool_status(args) -> dict:
    wait = float(args.get("wait_s", 0))
    c = _fresh(wait)
    return _text({"connection": reader.status(), "cycle": c.summary()})


def tool_unhappy(args) -> dict:
    wait = float(args.get("wait_s", 0))
    c = _fresh(wait)
    s = c.summary()
    return _text({
        "connection": reader.status(),
        "verdict": s["verdict"], "happy": s["happy"],
        "board": s["board"], "meta": s["meta"],
        "not_happy": {
            "analog_channels": s["unhappy"]["analog"],
            "input_counters": s["unhappy"]["counters"],
            "digital_outputs": s["unhappy"]["outputs"],
            "board_errors": s["board_errors"],
        },
        "dc": s["dc"],
    })


def tool_dc(args) -> dict:
    c = _fresh(float(args.get("wait_s", 0)))
    return _text({"connection": reader.status(), "dc": c.dc_status(),
                  "meta": {"outputs": c.meta_outputs, "low_side": c.meta_low_side, "dc": c.meta_dc}})


def tool_wait_cycle(args) -> dict:
    timeout = float(args.get("timeout_s", 30))
    c = reader.wait_for_cycle(timeout)
    if c is None:
        return _text({"connection": reader.status(),
                      "error": f"no cycle committed within {timeout}s",
                      "hint": "rig may be idle/listen-mode, serial unplugged, or ECU not in QC mode"})
    return _text({"connection": reader.status(), "cycle": c.summary()})


def tool_raw_tail(args) -> dict:
    n = int(args.get("lines", 40))
    return _text({"connection": reader.status(), "lines": reader.tail(n)})


def tool_cycle_full(args) -> dict:
    c = _fresh(float(args.get("wait_s", 0)))
    return _text({"connection": reader.status(), "cycle": c.as_dict()})


TOOLS = [
    ("stim_status", "Latest parsed stim cycle: verdict, board, meta counts and a summary "
     "of unhappy analog/counter/output channels plus DC status. Pass wait_s>0 to block "
     "for a fresh cycle first.",
     {"type": "object", "properties": {"wait_s": {"type": "number",
      "description": "seconds to wait for a fresh cycle (0 = use latest)"}}}, tool_status),
    ("stim_unhappy", "Just the NOT-happy items the rig reported (bad analog channels, zero "
     "input counters, failed digital outputs, board errors) plus the DC verdict. Direct "
     "answer to 'which channels are not happy / is DC happy'.",
     {"type": "object", "properties": {"wait_s": {"type": "number"}}}, tool_unhappy),
    ("stim_dc", "Is DC happy? Derived from the rig's GOOD DC= lines vs the advertised DC count.",
     {"type": "object", "properties": {"wait_s": {"type": "number"}}}, tool_dc),
    ("stim_wait_cycle", "Block until the rig finishes its next test cycle (next ALL GOOD / "
     "SOMETHING BAD) and return that cycle's parse.",
     {"type": "object", "properties": {"timeout_s": {"type": "number", "default": 30}}}, tool_wait_cycle),
    ("stim_raw_tail", "Last N de-ANSI'd raw console lines from the rig (debugging).",
     {"type": "object", "properties": {"lines": {"type": "integer", "default": 40}}}, tool_raw_tail),
    ("stim_cycle_full", "Full structured dump of the current/last cycle (all fields).",
     {"type": "object", "properties": {"wait_s": {"type": "number"}}}, tool_cycle_full),
]
_DISPATCH = {name: fn for name, _d, _s, fn in TOOLS}


# -- MCP stdio plumbing ---------------------------------------------------------
def _send(obj):
    sys.stdout.write(json.dumps(obj) + "\n")
    sys.stdout.flush()


def _result(rid, result):
    _send({"jsonrpc": "2.0", "id": rid, "result": result})


def _error(rid, code, message):
    _send({"jsonrpc": "2.0", "id": rid, "error": {"code": code, "message": message}})


def handle(msg):
    method = msg.get("method")
    rid = msg.get("id")
    if method == "initialize":
        _result(rid, {"protocolVersion": PROTOCOL_VERSION,
                      "capabilities": {"tools": {}},
                      "serverInfo": {"name": "stim-rig", "version": "1.0.0"}})
    elif method in ("notifications/initialized", "initialized"):
        pass  # notification, no reply
    elif method == "ping":
        _result(rid, {})
    elif method == "tools/list":
        _result(rid, {"tools": [{"name": n, "description": d, "inputSchema": s}
                                for n, d, s, _fn in TOOLS]})
    elif method == "tools/call":
        params = msg.get("params", {})
        name = params.get("name")
        args = params.get("arguments", {}) or {}
        fn = _DISPATCH.get(name)
        if fn is None:
            _error(rid, -32601, f"unknown tool: {name}")
            return
        try:
            _result(rid, fn(args))
        except Exception as e:
            _result(rid, {"content": [{"type": "text",
                     "text": f"tool error: {type(e).__name__}: {e}"}], "isError": True})
    elif rid is not None:
        _error(rid, -32601, f"method not found: {method}")


def main():
    global reader
    port = os.environ.get("STIM_SERIAL_PORT") or None
    baud = int(os.environ.get("STIM_SERIAL_BAUD", "115200"))
    reader = StimReader(port=port, baud=baud)
    reader.start()
    for raw in sys.stdin:
        raw = raw.strip()
        if not raw:
            continue
        try:
            msg = json.loads(raw)
        except json.JSONDecodeError:
            continue
        handle(msg)


if __name__ == "__main__":
    main()
