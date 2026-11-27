"""Background serial reader for the stim-rig console, feeding StimParser.

The rig is an STM32 Nucleo-F429ZI exposing a USB CDC ACM port (ANSI-coloured text,
CDC so baud is nominal). On this WSL bench it shows up as /dev/ttyACM* once the USB
device is attached (usbip). When it is absent the reader stays in a retry loop and
the MCP tools report "serial not connected" instead of failing.
"""
from __future__ import annotations

import glob
import threading
import time
from collections import deque
from typing import Optional

import serial  # pyserial

from stim_parser import StimParser, Cycle, strip_ansi


def autodetect_port() -> Optional[str]:
    for pat in ("/dev/ttyACM*", "/dev/ttyUSB*"):
        hits = sorted(glob.glob(pat))
        if hits:
            return hits[0]
    return None


class StimReader:
    def __init__(self, port: Optional[str] = None, baud: int = 115200, raw_keep: int = 4000):
        self.port = port
        self.baud = baud
        self.parser = StimParser()
        self.raw = deque(maxlen=raw_keep)       # de-ANSI'd console lines
        self.lock = threading.Lock()
        self.connected = False
        self.last_line_ts: Optional[float] = None
        self.last_error: Optional[str] = None
        self._cycle_event = threading.Event()   # set each time a cycle commits
        self._stop = threading.Event()
        self._thread: Optional[threading.Thread] = None

    # -- lifecycle --------------------------------------------------------------
    def start(self):
        if self._thread and self._thread.is_alive():
            return
        self._stop.clear()
        self._thread = threading.Thread(target=self._run, daemon=True)
        self._thread.start()

    def stop(self):
        self._stop.set()

    def _run(self):
        buf = b""
        while not self._stop.is_set():
            port = self.port or autodetect_port()
            if not port:
                self.connected = False
                self.last_error = "no /dev/ttyACM* or /dev/ttyUSB* present"
                time.sleep(1.0)
                continue
            try:
                with serial.Serial(port, self.baud, timeout=0.5) as ser:
                    self.connected = True
                    self.last_error = None
                    buf = b""
                    while not self._stop.is_set():
                        chunk = ser.read(256)
                        if not chunk:
                            continue
                        buf += chunk
                        while b"\n" in buf:
                            line, buf = buf.split(b"\n", 1)
                            self._handle(line.decode("utf-8", "replace"))
            except Exception as e:  # port vanished / permission / unplug
                self.connected = False
                self.last_error = f"{type(e).__name__}: {e}"
                time.sleep(1.0)

    def _handle(self, raw: str):
        clean = strip_ansi(raw)
        with self.lock:
            self.last_line_ts = time.time()
            if clean.strip():
                self.raw.append(clean)
            committed = self.parser.feed_line(raw)
        if committed is not None:
            self._cycle_event.set()

    # -- accessors (thread-safe) ------------------------------------------------
    def current_cycle(self) -> Cycle:
        with self.lock:
            return self.parser.current()

    def last_cycle(self) -> Optional[Cycle]:
        with self.lock:
            return self.parser.last_cycle

    def tail(self, n: int = 40) -> list:
        with self.lock:
            return list(self.raw)[-n:]

    def status(self) -> dict:
        with self.lock:
            age = None if self.last_line_ts is None else round(time.time() - self.last_line_ts, 1)
            return {
                "connected": self.connected,
                "port": self.port or autodetect_port(),
                "baud": self.baud,
                "last_line_age_s": age,
                "cycles_seen": self.parser.cycles_seen,
                "last_error": self.last_error,
            }

    def wait_for_cycle(self, timeout_s: float = 30.0) -> Optional[Cycle]:
        """Block until the next cycle commits (next ALL GOOD / SOMETHING BAD)."""
        self._cycle_event.clear()
        if self._cycle_event.wait(timeout_s):
            return self.last_cycle()
        return None
