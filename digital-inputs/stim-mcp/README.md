# stim-mcp — HW-QC stim-rig console parser + MCP server

Parses the **USB-serial console stream** of the HW-QC tester firmware
(`digital-inputs/firmware`, the STM32 Nucleo-F429ZI "stim" board) and exposes it as
structured data — over an **MCP** server for an agent, and a plain CLI for a shell.

The rig already decides, per cycle, which analog channels / input counters / outputs /
DC channels are **HAPPY** vs not and prints an ANSI-coloured report ending in
`ALL GOOD` or `SOMETHING BAD`. This tool reads that stream and answers questions like
*"which channels are not happy?"* / *"is DC happy?"* from the rig's own verdicts —
no CAN reimplementation, no guessing.

> It reports what the **tester** concluded. The per-output pass/fail and the
> analog-range checks are the physical fixture's (its analog mux + op-amp reference
> pads); this parser surfaces them, it does not re-measure anything.

## Files

| File | Role |
|------|------|
| `stim_parser.py` | pure line parser → `Cycle` state (no I/O; unit-tested) |
| `stim_reader.py` | pyserial reader thread, rolling cycle state + raw ring buffer |
| `server.py` | zero-dependency **stdio MCP** server (python3 + pyserial only) |
| `stim_cli.py` | CLI: `parse <log>`, `status [port]`, `tail [port]` |
| `tests/` | parser tests + a sample captured log |

Each regex in `stim_parser.py` maps 1:1 to a `chprintf` format string in the firmware
(`main.cpp`, `can.cpp`, `test_digital_outputs.cpp`). If the firmware console strings
change, update the patterns here.

## Serial connection (WSL bench)

The console is the **STM32's own native USB CDC** (VID:PID `0483:5740`, "USB Serial
Device", COM15 on Windows) — **not** the ST-Link VCP (`0483:374b`), which the firmware
does *not* print to. Reading the ST-Link port shows only silence; that was a real
time-sink. Attach the right one to WSL:

```bash
usbipd.exe list                              # find the 0483:5740 busid (e.g. 4-3)
usbipd.exe attach --wsl --busid 4-3          # now appears as the NEXT /dev/ttyACM*
ls -l /dev/ttyACM*                           # the new node (often ttyACM1) is the rig
```

CDC baud is nominal. The node is `dialout`-group (be in `dialout`). Then set
`STIM_SERIAL_PORT=/dev/ttyACM1`.

- Autodetect: leave `STIM_SERIAL_PORT` unset → first `/dev/ttyACM*` then `/dev/ttyUSB*`.
- Explicit: `STIM_SERIAL_PORT=/dev/ttyACM0`.

When the port is absent the tools still answer, with `connection.connected=false` and a
`last_error` — they never crash.

## CLI

```bash
python3 stim_cli.py parse tests/sample_cycle.log   # parse a captured log
python3 stim_cli.py status /dev/ttyACM0            # one cycle, print JSON summary
python3 stim_cli.py tail                            # live, autodetect port
```

## Register as an MCP server in Claude Code

Add to `.mcp.json` (project) or `~/.claude.json`:

```json
{
  "mcpServers": {
    "stim-rig": {
      "command": "python3",
      "args": ["/home/rusefi/rusefi-hardware/digital-inputs/stim-mcp/server.py"],
      "env": { "STIM_SERIAL_PORT": "/dev/ttyACM0" }
    }
  }
}
```

### Tools

| Tool | Answers |
|------|---------|
| `stim_status` | full latest-cycle summary (verdict, board, meta, unhappy lists, DC) |
| `stim_unhappy` | **just the not-happy items** + DC verdict |
| `stim_dc` | is DC happy? (GOOD DC= vs advertised DC count) |
| `stim_wait_cycle` | block for the rig's next full cycle, return its parse |
| `stim_raw_tail` | last N raw console lines (debugging) |
| `stim_cycle_full` | every parsed field of the current/last cycle |

`stim_status` / `stim_unhappy` / `stim_dc` take optional `wait_s` to block for a fresh
cycle first (otherwise they return the last completed one, instantly).

## What "happy" means (from the firmware)

- **analog channel**: raw voltage within the board's accept range in
  `test_logic_<board>.cpp`. Bad → ` * BAD analog channel N (NAME): voltage V ...`.
- **input counter** (triggers/VVT/VSS, buttons, aux digital): the rig saw the counter
  take **two different non-zero values** → `* HAPPY NAME KIND counter!`, else
  `* ZERO ...`. Only counters the board marks *expected* are printed.
- **digital output**: driven and read back through the fixture mux; mismatch →
  `ERROR! Line L@A FAILED! (set S, received R Vv)`.
- **DC output**: each H-bridge index prints `GOOD DC=i` on pass. DC is "happy" when
  every index `0..dc_count-1` reported GOOD DC.
- **cycle**: `ALL GOOD` only if outputs + CAN analog + uptime>10s + DC + input counters
  all pass; otherwise `SOMETHING BAD ... ERRORS=n`.

Board tables (GPEC2A `0xF04E`, minimal-Harley `0xF050`, …) and their expected channels
live in the firmware `test_logic_*.cpp`; this parser does not duplicate them — it
reports whatever the running rig printed.
