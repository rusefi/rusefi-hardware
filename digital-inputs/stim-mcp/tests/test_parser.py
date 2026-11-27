"""Parser tests built from the exact firmware console format strings.

Run: python3 tests/test_parser.py   (from the stim-mcp dir, or any cwd)
"""
import os
import sys

sys.path.insert(0, os.path.dirname(os.path.dirname(os.path.abspath(__file__))))
from stim_parser import StimParser  # noqa: E402

ESC = "\x1b[32m"  # a green SGR to prove ANSI stripping

GPEC2A_GOOD = [
    " * Board detected: gpec2a rev index 0",
    "       CAN ECU says: total=3 outputs of which low side=3 also 0 DC",
    "       CAN RX BoardStatus: BoardID=61518 numSecs=42",
    ESC + " ************* TPS1_2 analog 0 1 voltage=0.863000 within range 0.798000/0.927000\x1b[0m",
    ESC + "* HAPPY Primary Trigger event counter!\x1b[0m",
    "* HAPPY VVT1 Sensor event counter!",
    " *********************************************** version=5",
    " ************* ALL GOOD ************************ uptime=42",
]

HARLEY_BAD = [
    " * Board detected: hd-min rev index 0",
    "       CAN ECU says: total=6 outputs of which low side=6 also 1 DC",
    "       CAN RX BoardStatus: BoardID=61520 numSecs=55",
    " * BAD analog channel 2 (PPS1): voltage 1.230000 (raw 63) not in range (0.790000..0.920941) mult=1.000000",
    "* ZERO Secondary Trigger event counter!",
    "ERROR! Line 4@2 FAILED! (set 1, received 0 0.012v)",
    " *********************************************** version=5",
    " ************* SOMETHING BAD SEE ABOVE ERRORS=3 ************************ count=7",
]


def check(cond, msg):
    if not cond:
        raise AssertionError(msg)
    print("  ok:", msg)


def main():
    p = StimParser()
    for ln in GPEC2A_GOOD:
        p.feed_line(ln)
    c = p.last_cycle
    s = c.summary()
    print("GPEC2A ALL GOOD:")
    check(s["happy"] is True, "verdict happy")
    check(s["board"]["name"] == "gpec2a", "board gpec2a")
    check(s["board"]["id"] == 61518, "board id 0xF04E=61518")
    check(s["meta"] == {"outputs": 3, "low_side": 3, "dc": 0}, "meta 3/3/0")
    check(s["dc"]["applicable"] is False, "DC n/a on gpec2a (0 DC)")
    check(s["unhappy"]["analog"] == [], "no bad analog")
    check(c.counters["Primary Trigger"]["happy"] and c.counters["VVT1 Sensor"]["happy"],
          "both counters happy")

    for ln in HARLEY_BAD:
        p.feed_line(ln)
    c = p.last_cycle
    s = c.summary()
    print("minimal-Harley SOMETHING BAD:")
    check(s["happy"] is False, "verdict not happy")
    check(s["board"]["id"] == 61520, "board id 0xF050=61520")
    check(s["meta"]["dc"] == 1, "meta dc=1")
    bad_an = {a["name"] for a in s["unhappy"]["analog"]}
    check("PPS1" in bad_an, "PPS1 flagged bad analog")
    check(s["unhappy"]["analog"][0]["voltage"] == 1.23, "bad analog voltage parsed")
    zero = {x["name"] for x in s["unhappy"]["counters"]}
    check("Secondary Trigger" in zero, "Secondary Trigger counter zero")
    fails = {o["line"] for o in s["unhappy"]["outputs"]}
    check(4 in fails, "output line 4 failed")
    check(s["dc"]["happy"] is False and s["dc"]["missing"] == [0],
          "DC unhappy, index 0 missing GOOD DC=")
    print("\nALL PARSER TESTS PASSED")


if __name__ == "__main__":
    main()
