#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-3.0-only
"""tools/smklog.py's decoder against a frame packed as the firmware packs it (main_fm1.c smk_tel, ota_pack7)."""
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), "..", "tools"))
import smklog  # noqa: E402


def pack7(data):
    out, acc, nb = [], 0, 0
    for b in data:
        acc |= b << nb
        nb += 8
        while nb >= 7:
            out.append(acc & 0x7F)
            acc >>= 7
            nb -= 7
    if nb:
        out.append(acc & 0x7F)
    return out


w = [7, 12345, 1 << 16 | 1, 1 << 12, 1 << 4, 10 | 20 << 8 | 30 << 16 | 127 << 24, 542 | 651 << 16, 999 | 636 << 16,
     2 | 0xFFFE << 16, 0, 0, 3, 50000, 2, 9 | 8 << 8 | 41 << 16, 12 | 220 << 16, 30 | 80 << 16, 150 | 210 << 16,
     260 | 290 << 16, 300 | 305 << 16, 1 | 12 << 8 | 78 << 16, 1 << 9]
d = smklog.decode([0x7D, 0x50, 0x5C] + pack7(struct.pack("<22I", *w)))
bad = 0
for k, v in (("seq", 7), ("buttons", 1 << 16 | 1), ("keys", 1 << 12 | 1 << 36), ("fader", [10, 20, 30, 127]),
             ("pitch", 542), ("mod", 651), ("pedal", 999), ("batt", 636), ("enc", [2, -2, 0, 0, 0, 0, 3, 0]),
             ("adc_miss", 2), ("hit_pad", 9), ("hit_n", 8), ("hits", 41), ("hit_base", 12), ("pad_t0", 220),
             ("hit_raw", [30, 80, 150, 210, 260, 290, 300, 305]), ("hit_vel", 1), ("key", 12), ("key_vel", 78),
             ("pads", 1 << 9)):
    ok = d and d[k] == v
    bad += not ok
    print("%-9s %-40s %s" % (k, d and d[k], "ok" if ok else "FAIL (want %s)" % (v,)))
p = dict(d, buttons=0, keys=0, hits=40)
ev = smklog.changes(p, d)
print("\n".join(ev))
bad += not any("SEQ PLAY" in e for e in ev) or not any("key 12 C4  on vel 78" in e for e in ev) or not any("pad  9 hit" in e for e in ev)
print("smklog test", "FAILED" if bad else "passed")
sys.exit(1 if bad else 0)
