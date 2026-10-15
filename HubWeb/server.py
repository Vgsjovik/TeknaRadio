#!/usr/bin/env python3
"""
TeknaRadio – server for hub-nettsiden.

Leser linjene huben (Hub/Hub.ino) skriver over USB-serial og sender dem
uendret videre til nettsiden (index.html) via Server-Sent Events (SSE).
Formatet står øverst i Hub/Hub.ino. Nettsiden tolker linjene selv.

Bruk:
    python3 server.py                 # auto-finn Arduino-port
    python3 server.py --port /dev/cu.usbmodem1101
    python3 server.py --demo          # ingen hardware – oppdiktet trafikk
    python3 server.py --baud 9600 --http 8765

Avhengighet: pyserial (kun når en ekte port brukes; --demo trenger den ikke).
Alt annet er Pythons standardbibliotek.
"""

import argparse
import glob
import json
import queue
import random
import re
import sys
import threading
import time
from http.server import BaseHTTPRequestHandler, ThreadingHTTPServer
from pathlib import Path

HERE = Path(__file__).resolve().parent
INDEX = HERE / "index.html"

# ── Delt tilstand ────────────────────────────────────────────────
HISTORY = []                 # (ts, linje) for alt unntatt PULS
HISTORY_MAX = 2000
PULS = {}                    # kallenavn -> (ts, siste PULS-linje)
NAVN_LINJE = None            # siste NAVN|…-linje fra huben
SUBSCRIBERS = []             # liste av queue.Queue, én per åpen nettside
LOCK = threading.Lock()
STATUS = {"serial": "starter", "port": None, "demo": False}
SERIAL_CONN = None           # aktiv serial.Serial-tilkobling (settes av serial_loop)
DEMO_STOP = threading.Event()  # sett for å stoppe demo_loop

LAGRES = ("ONLINE", "TENN", "SLUKK", "MSG", "SENDT")
NAVN_OK = re.compile(r"^[A-Z0-9]{1,8}$")


def now_ms():
    return int(time.time() * 1000)


def kringkast(msg: dict):
    with LOCK:
        subs = list(SUBSCRIBERS)
    for q in subs:
        try:
            q.put_nowait(msg)
        except queue.Full:
            pass


def set_serial(value: str):
    """Oppdater serial-status og push til alle åpne nettsider ved endring."""
    if STATUS["serial"] != value:
        STATUS["serial"] = value
        kringkast({"_status": dict(STATUS)})


def linje_inn(line: str):
    """Ta imot én linje fra huben: husk den og send den til nettsidene.
    Returnerer linjetypen, eller None for banner og støy."""
    global NAVN_LINJE
    line = line.strip()
    felt = line.split("|")
    kind = felt[0]
    ts = now_ms()
    with LOCK:
        if kind == "NAVN" and len(felt) > 1:
            NAVN_LINJE = line
        elif kind == "PULS" and len(felt) > 2:
            PULS[felt[1]] = (ts, line)
        elif kind in LAGRES and len(felt) > 2:
            HISTORY.append((ts, line))
            if len(HISTORY) > HISTORY_MAX:
                del HISTORY[0]
        elif kind not in ("READY", "ERR"):
            return None
    if kind != "READY":
        kringkast({"linje": line, "ts": ts})
    return kind


def nullstill():
    """Tøm alt som er hørt, og be nettsidene gjøre det samme."""
    with LOCK:
        del HISTORY[:]
        PULS.clear()
        navn = NAVN_LINJE
    kringkast({"_reset": True})
    if navn:
        kringkast({"linje": navn, "ts": now_ms()})


def write_serial(line: str) -> bool:
    """Skriv en linje til Arduinoen. Tråd-trygt. Returnerer False hvis ikke tilkoblet."""
    with LOCK:
        conn = SERIAL_CONN
    if conn is None:
        return False
    try:
        conn.write((line.rstrip("\n") + "\n").encode("utf-8"))
        return True
    except Exception:
        return False


# ── Serial-leser ─────────────────────────────────────────────────
def find_port():
    patterns = ["/dev/cu.usbmodem*", "/dev/cu.usbserial*",
                "/dev/tty.usbmodem*", "/dev/ttyACM*", "/dev/ttyUSB*"]
    for pat in patterns:
        hits = sorted(glob.glob(pat))
        if hits:
            return hits[0]
    return None


def serial_loop(port, baud):
    try:
        import serial  # pyserial – importeres kun her
    except ImportError:
        STATUS["serial"] = "mangler pyserial"
        print("FEIL: pyserial er ikke installert. Kjør: pip install pyserial",
              file=sys.stderr)
        return

    while True:
        p = port or find_port()
        if not p:
            STATUS["port"] = None
            set_serial("venter på hub")
            time.sleep(1.5)
            continue
        try:
            STATUS["port"] = p
            set_serial("kobler til")
            with serial.Serial(p, baud, timeout=1) as ser:
                with LOCK:
                    global SERIAL_CONN
                    SERIAL_CONN = ser
                set_serial("port_funnet")   # port åpen, Arduino ikke bekreftet ennå
                print(f"Serial tilkoblet: {p} @ {baud}")
                # Huben kan ha startet før vi koblet til. PING får den til å
                # svare med READY og navnelista, så siden ikke står og venter.
                ping_etter = time.time() + 2
                while True:
                    if ping_etter and time.time() >= ping_etter:
                        ser.write(b"PING\n")
                        ping_etter = None
                    raw = ser.readline()
                    if not raw:
                        continue
                    kind = linje_inn(raw.decode("utf-8", "replace"))
                    # Faktisk trafikk = Arduino bekreftet (uansett tidligere tilstand)
                    if kind and not STATUS["demo"]:
                        set_serial("tilkoblet")
        except Exception as e:  # noqa: BLE001 – port forsvant, prøv igjen
            with LOCK:
                SERIAL_CONN = None
            set_serial("frakoblet")
            print(f"Serial-feil ({p}): {e} – prøver igjen…", file=sys.stderr)
            time.sleep(2)


# ── Demo-modus (ingen hardware) ──────────────────────────────────
DEMO_NAVN = ["HAWK", "JET", "ICE", "WOLF", "ACE", "PUMA",
             "BOLT", "ORCA", "ION", "FOX", "LYNX", "NOVA"]
DEMO_PARTNER = {n: DEMO_NAVN[i - 1 if i % 2 else i + 1] for i, n in enumerate(DEMO_NAVN)}
DEMO_FRASER = ["Hei! Ser du denne?", "HEI", "Funker det hos deg?", "TEST 1 2 3",
               "Kom dette fram?", "SVAR HVIS DU SER", "Vi er i gang", "OK", "JA"]


def demo_loop():
    """Oppdiktet trafikk i samme format som huben skriver. ORCA kommer sent,
    LYNX aldri, og WOLF mister kontakten en stund, så alle tilstander vises."""
    DEMO_STOP.clear()
    STATUS["demo"] = True
    nullstill()
    set_serial("demo")
    linje_inn("NAVN|" + ",".join(DEMO_NAVN))
    pa = {n: n not in ("ORCA", "LYNX") for n in DEMO_NAVN}
    tent = set()
    start = time.time()
    neste_puls = {n: start + random.random() * 5 for n in DEMO_NAVN}
    neste_handling = start + 3
    for n in DEMO_NAVN:
        if pa[n]:
            linje_inn(f"ONLINE|{n}|{DEMO_PARTNER[n]}")

    while not DEMO_STOP.wait(timeout=0.5):
        naa = time.time()
        t = naa - start
        if t > 9 and not pa["ORCA"]:
            pa["ORCA"] = True
            linje_inn("ONLINE|ORCA|BOLT")
        pa["WOLF"] = not (28 < t % 110 < 62)
        for n in DEMO_NAVN:
            if pa[n] and naa >= neste_puls[n]:
                neste_puls[n] = naa + 5
                linje_inn(f"PULS|{n}|{DEMO_PARTNER[n]}")
        if naa < neste_handling:
            continue
        neste_handling = naa + 2.6 + random.random() * 2.6
        aktive = [n for n in DEMO_NAVN if pa[n]]
        n = random.choice([x for x in aktive if pa[DEMO_PARTNER[x]]])
        p = DEMO_PARTNER[n]
        r = random.random()
        if r < 0.35:
            linje_inn(f"{'SLUKK' if p in tent else 'TENN'}|{n}|{p}")
            tent ^= {p}
        elif r < 0.8:
            linje_inn(f"MSG|{n}|{p}|{random.choice(DEMO_FRASER)}")
        elif r < 0.9:
            andre = random.choice([x for x in aktive if x not in (n, p)])
            linje_inn(f"MSG|{n}|{andre}|Hvem er du sammen med?")
        else:
            linje_inn(f"MSG|{n}|ALLE|Hei alle sammen")

    STATUS["demo"] = False
    nullstill()
    # Gjenopprett ekte tilstand: Arduino fortsatt tilkoblet → tilkoblet
    with LOCK:
        conn = SERIAL_CONN
    set_serial("tilkoblet" if conn else "venter på hub")
    if conn:
        write_serial("PING")


# ── HTTP / SSE ───────────────────────────────────────────────────
class Handler(BaseHTTPRequestHandler):
    def log_message(self, *a):  # stillhet i konsollen
        pass

    def _send(self, code, ctype, body):
        self.send_response(code)
        self.send_header("Content-Type", ctype)
        self.send_header("Content-Length", str(len(body)))
        self.end_headers()
        self.wfile.write(body)

    def do_POST(self):
        # Bare siden selv sender JSON. Kravet stopper skjemaer på andre nettsteder
        # fra å sende radiomeldinger gjennom denne serveren.
        if self.headers.get("Content-Type", "").split(";")[0] != "application/json":
            self._send(415, "application/json", b'{"error":"krever application/json"}')
            return

        if self.path == "/demo":
            if STATUS["demo"]:
                DEMO_STOP.set()          # stopp pågående demo
            else:
                threading.Thread(target=demo_loop, daemon=True).start()
            time.sleep(0.15)             # la loop rekke å oppdatere STATUS
            self._send(200, "application/json",
                       json.dumps({"demo": STATUS["demo"]}).encode())
            return

        if self.path == "/send":
            try:
                length = int(self.headers.get("Content-Length", 0))
                data = json.loads(self.rfile.read(length))
                # Klipp på BYTES, ikke tegn: æøå er 2 bytes i UTF-8, og
                # radiopakken har plass til 28 bytes tekst.
                text = " ".join(str(data.get("text", "")).split())
                text = text.encode("utf-8")[:28].decode("utf-8", "ignore")
                til = str(data.get("til", "ALLE")).upper()
                if not text or not NAVN_OK.match(til):
                    self._send(400, "application/json", b'{"error":"tom melding eller ugyldig mottaker"}')
                    return
                if STATUS["demo"]:
                    linje_inn(f"SENDT|{til}|{text}")
                    ok = True
                else:
                    ok = write_serial(f"SEND|{til}|{text}")
                self._send(200, "application/json",
                           json.dumps({"ok": ok}).encode())
            except Exception as e:
                self._send(500, "application/json",
                           json.dumps({"error": str(e)}).encode())
            return
        self._send(404, "text/plain; charset=utf-8", b"ikke funnet")

    def do_GET(self):
        if self.path == "/favicon.ico":
            self._send(204, "image/x-icon", b"")
            return

        if self.path == "/logo":
            try:
                logo = HERE / "Logo" / "Tekna_logo_Kompakt_Mørk_blå_RGB.svg"
                self._send(200, "image/svg+xml; charset=utf-8", logo.read_bytes())
            except FileNotFoundError:
                self._send(404, "text/plain; charset=utf-8", b"logo mangler")
            return

        if self.path in ("/", "/index.html"):
            try:
                self._send(200, "text/html; charset=utf-8",
                           INDEX.read_bytes())
            except FileNotFoundError:
                self._send(404, "text/plain; charset=utf-8",
                           b"index.html mangler")
            return

        if self.path == "/status":
            self._send(200, "application/json",
                       json.dumps(STATUS).encode("utf-8"))
            return

        if self.path == "/stream":
            self.stream_sse()
            return

        self._send(404, "text/plain; charset=utf-8", b"ikke funnet")

    def stream_sse(self):
        self.send_response(200)
        self.send_header("Content-Type", "text/event-stream")
        self.send_header("Cache-Control", "no-cache")
        self.send_header("Connection", "keep-alive")
        self.end_headers()

        q = queue.Queue(maxsize=1000)
        with LOCK:
            # Hvis status sier "tilkoblet" men ingen serial-tilkobling er aktiv,
            # betyr det at serveren er gammel stale-data fra forrige sesjon.
            if STATUS["serial"] in ("tilkoblet", "port_funnet") and SERIAL_CONN is None and not STATUS["demo"]:
                STATUS["serial"] = "frakoblet"
            hist = list(HISTORY) + list(PULS.values())
            navn = NAVN_LINJE
            SUBSCRIBERS.append(q)
        try:
            # En ny (eller gjenoppkoblet) side begynner med blanke ark og får
            # deretter alt som er hørt, med de opprinnelige tidspunktene.
            self._sse({"_status": STATUS})
            self._sse({"_reset": True})
            if navn:
                self._sse({"linje": navn, "ts": now_ms()})
            for ts, line in hist:
                self._sse({"linje": line, "ts": ts})
            # Live
            while True:
                try:
                    self._sse(q.get(timeout=15))
                except queue.Empty:
                    self.wfile.write(b": ping\n\n")  # heartbeat
                    self.wfile.flush()
        except (BrokenPipeError, ConnectionResetError):
            pass
        finally:
            with LOCK:
                if q in SUBSCRIBERS:
                    SUBSCRIBERS.remove(q)

    def _sse(self, msg: dict):
        self.wfile.write(b"data: " + json.dumps(msg).encode("utf-8") + b"\n\n")
        self.wfile.flush()


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--port", help="serial-port (auto hvis utelatt)")
    ap.add_argument("--baud", type=int, default=9600)
    ap.add_argument("--http", type=int, default=8765)
    ap.add_argument("--demo", action="store_true",
                    help="kjør med oppdiktet trafikk, uten hardware")
    args = ap.parse_args()

    if args.demo:
        threading.Thread(target=demo_loop, daemon=True).start()
    else:
        threading.Thread(target=serial_loop,
                         args=(args.port, args.baud), daemon=True).start()

    pidfile = HERE / ".hub.pid"
    try:
        pidfile.write_text(str(__import__("os").getpid()))
    except Exception:
        pass

    srv = ThreadingHTTPServer(("127.0.0.1", args.http), Handler)
    url = f"http://localhost:{args.http}"
    print(f"TeknaRadio hub-nettside: {url}")
    print("Avslutt med Ctrl-C.")
    try:
        srv.serve_forever()
    except KeyboardInterrupt:
        print("\nAvslutter.")
    finally:
        try:
            pidfile.unlink()
        except Exception:
            pass


if __name__ == "__main__":
    main()
