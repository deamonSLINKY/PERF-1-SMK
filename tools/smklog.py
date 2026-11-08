#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-3.0-only
"""PERF-1 SMK: the inputs log. Listens to the SMK build's inputs frame (F0 7D 50 5C, ten a second, main_fm1.c
smk_tel) and prints what changes: buttons by name, keys with velocity, pad hits with their baseline, the curve's
start, the first readings and the velocity, faders, wheels, pedal, knobs, ADC misses. Also a CSV of every frame.
Sends only PERF's telemetry request (F0 7D 50 46 01 F7, every 3 s): nothing is written to the unit.

  python -P tools/smklog.py [--port SMK] [--secs 120] [--out build/smklog.csv]
"""
import argparse
import csv
import os
import struct
import sys
import time

ARM = [0x7D, 0x50, 0x46, 0x01]
NW = 22
BTN = ["KNOB BANK", "FADER BANK", "<", ">", "OCTAVE-", "OCTAVE+", "ARP", "NOTE REPEAT", "SCALE", "CHORD",
       "GLOBE", "BT", "PATCH", "PARA", "FX", "SEQ", "SEQ PLAY", "SEQ REC"]
NOTE = ["C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B"]


def unpack7(d):
    out, acc, nb = bytearray(), 0, 0
    for x in d:
        acc |= (x & 0x7F) << nb
        nb += 7
        if nb >= 8:
            out.append(acc & 0xFF)
            acc >>= 8
            nb -= 8
    return bytes(out)


def decode(data):
    """mido sysex data (without F0/F7) -> dict, or None"""
    if list(data[:3]) != [0x7D, 0x50, 0x5C]:
        return None
    b = unpack7(bytes(data[3:]))
    if len(b) < NW * 4:
        return None
    w = struct.unpack_from("<%dI" % NW, b)
    enc = [struct.unpack("<h", struct.pack("<H", (w[8 + i // 2] >> (16 * (i % 2))) & 0xFFFF))[0] for i in range(8)]
    raw = [(w[16 + i // 2] >> (16 * (i % 2))) & 0xFFFF for i in range(8)]
    return {"seq": w[0], "ms": w[1], "buttons": w[2], "keys": w[3] | w[4] << 32,
            "fader": [(w[5] >> (8 * i)) & 0xFF for i in range(4)], "pitch": w[6] & 0xFFFF, "mod": w[6] >> 16,
            "pedal": w[7] & 0xFFFF, "batt": w[7] >> 16, "enc": enc, "adc_ok": w[12], "adc_miss": w[13],
            "hit_pad": w[14] & 0xFF, "hit_n": (w[14] >> 8) & 0xFF, "hits": w[14] >> 16, "hit_base": w[15] & 0xFFFF,
            "pad_t0": w[15] >> 16, "hit_raw": raw, "hit_vel": w[20] & 0xFF, "key": (w[20] >> 8) & 0xFF,
            "key_vel": (w[20] >> 16) & 0xFF, "pads": w[21]}


def key_name(k):
    return "%s%d" % (NOTE[k % 12], 3 + k // 12)


def changes(p, d):
    ev = []
    for i in range(18):
        a, b = p["buttons"] >> i & 1, d["buttons"] >> i & 1
        if a != b:
            ev.append("button %-11s %s" % (BTN[i], "down" if b else "up"))
    new = d["keys"] & ~p["keys"]
    for k in range(37):
        if new >> k & 1:
            ev.append("key %2d %-3s on%s" % (k, key_name(k), (" vel %d" % d["key_vel"]) if d["key"] == k else ""))
    if d["hits"] != p["hits"]:
        n = (d["hits"] - p["hits"]) & 0xFFFF
        ev.append("pad %2d hit  vel %3d  base %4d  curve from %4d  rise %s%s" % (
            d["hit_pad"], d["hit_vel"], d["hit_base"], d["pad_t0"], " ".join(str(x) for x in d["hit_raw"][:d["hit_n"]]),
            ("  (+%d hits between frames)" % (n - 1)) if n > 1 else ""))
    for i in range(4):
        if abs(d["fader"][i] - p["fader"][i]) >= 2:
            ev.append("fader %d %3d" % (i + 1, d["fader"][i]))
    for i in range(8):
        if d["enc"][i] != p["enc"][i]:
            ev.append("knob K%d %+d (total %d)" % (i + 1, d["enc"][i] - p["enc"][i], d["enc"][i]))
    for k in ("pitch", "mod", "pedal"):
        if abs(d[k] - p[k]) >= 6:
            ev.append("%s %d" % (k, d[k]))
    if d["adc_miss"] != p["adc_miss"]:
        ev.append("ADC missed +%d" % (d["adc_miss"] - p["adc_miss"]))
    return ev


def main():
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("--port", default="SMK")
    ap.add_argument("--secs", type=float, default=120)
    ap.add_argument("--out", default=os.path.join("build", time.strftime("smklog_%Y%m%d_%H%M%S.csv")))
    a = ap.parse_args()
    import mido
    pin = next((n for n in mido.get_input_names() if a.port.lower() in n.lower()), None)
    pout = next((n for n in mido.get_output_names() if a.port.lower() in n.lower()), None)
    if not pin or not pout:
        sys.exit("no MIDI port with %r: in %s out %s" % (a.port, mido.get_input_names(), mido.get_output_names()))
    os.makedirs(os.path.dirname(a.out) or ".", exist_ok=True)
    print("in:", pin, "| out:", pout, "| csv:", a.out, flush=True)
    fin, fout = mido.open_input(pin), mido.open_output(pout)
    prev, frames, t0, last_arm = None, 0, time.time(), 0.0
    with open(a.out, "w", newline="") as f:
        cw = csv.writer(f)
        cw.writerow(["t", "seq", "buttons", "keys", "f1", "f2", "f3", "f4", "pitch", "mod", "pedal", "batt"] +
                    ["k%d" % (i + 1) for i in range(8)] + ["adc_ok", "adc_miss", "hits", "hit_pad", "hit_vel", "hit_base",
                                                          "pad_t0"] + ["r%d" % i for i in range(8)] + ["pads"])
        try:
            while time.time() - t0 < a.secs:
                now = time.time()
                if now - last_arm > 3.0:
                    fout.send(mido.Message("sysex", data=ARM))
                    last_arm = now
                got = False
                for m in fin.iter_pending():
                    if m.type != "sysex":
                        continue
                    d = decode(m.data)
                    if not d:
                        continue
                    got = True
                    frames += 1
                    if prev is None:
                        print("%7.2f first frame: faders %s pitch %d mod %d pedal %d batt %d knobs %s ADC %d/%d missed" % (
                            now - t0, d["fader"], d["pitch"], d["mod"], d["pedal"], d["batt"], d["enc"], d["adc_ok"],
                            d["adc_miss"]), flush=True)
                    else:
                        if d["seq"] != prev["seq"] + 1:
                            print("%7.2f (lost %d frames)" % (now - t0, d["seq"] - prev["seq"] - 1), flush=True)
                        for e in changes(prev, d):
                            print("%7.2f %s" % (now - t0, e), flush=True)
                    cw.writerow(["%.2f" % (now - t0), d["seq"], d["buttons"], d["keys"]] + d["fader"] +
                                [d["pitch"], d["mod"], d["pedal"], d["batt"]] + d["enc"] +
                                [d["adc_ok"], d["adc_miss"], d["hits"], d["hit_pad"], d["hit_vel"], d["hit_base"],
                                 d["pad_t0"]] + d["hit_raw"] + [d["pads"]])
                    prev = d
                if not got:
                    time.sleep(0.01)
        except KeyboardInterrupt:
            pass
    fin.close()
    fout.close()
    print("%d frames in %.0f s" % (frames, time.time() - t0))


if __name__ == "__main__":
    main()
