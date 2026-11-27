#!/usr/bin/env python3
"""CLI front-end for the stim parser (no MCP needed).

  stim_cli.py parse <logfile>     parse a captured console log, print JSON summary
  stim_cli.py status [port]       open serial, wait one cycle, print summary
  stim_cli.py tail [port]         live: print structured summary after every cycle
"""
import json
import sys

from stim_parser import StimParser


def parse_file(path):
    p = StimParser()
    with open(path, "r", errors="replace") as f:
        for line in f:
            p.feed_line(line)
    print(json.dumps(p.current().summary(), indent=2))


def _serial(port, follow):
    from stim_reader import StimReader
    r = StimReader(port=port)
    r.start()
    print(f"# connecting: {r.status()}", file=sys.stderr)
    while True:
        c = r.wait_for_cycle(60)
        if c is None:
            print(f"# no cycle in 60s: {r.status()}", file=sys.stderr)
            if not follow:
                return
            continue
        print(json.dumps(c.summary(), indent=2))
        if not follow:
            return


def main(argv):
    if len(argv) < 2:
        print(__doc__)
        return 2
    cmd = argv[1]
    if cmd == "parse" and len(argv) >= 3:
        parse_file(argv[2])
    elif cmd == "status":
        _serial(argv[2] if len(argv) >= 3 else None, follow=False)
    elif cmd == "tail":
        _serial(argv[2] if len(argv) >= 3 else None, follow=True)
    else:
        print(__doc__)
        return 2
    return 0


if __name__ == "__main__":
    sys.exit(main(sys.argv))
