# PERF-1 SMK, build 1: the hardware test (bench plan)

For the maker's SMK-37 Pro (stock 016, backed up 2026-11-08: `G:\fm1\stock-smk37\smk37-dump-1.bin`). Build from
branch `smk37`: `BOARD=smk37 sh build.sh` -> `build/perf1-smk.fwsc` (identity `SMK-37 Pro_700`).

## What it is

PERF-1 on the SMK, booting into a **HW TEST** screen instead of the music. Every input live, as the HAL reads it:

| Screen | Shows | Look for |
|---|---|---|
| Top: keybed | 37 keys, lit while held; `KEY k Vv` top right: the last key (0 = C3 .. 36 = C6) and its velocity | each key lights the right one; soft = low V, hard = high V (1..127) |
| Top: pads | 16 boxes, 0..7 the top row, 8..15 the bottom row; filled while pressed; `PAD p Vv` the last pad and velocity | the box under your finger fills; velocity follows how hard |
| Top: info | `FLASH P25Q80` (the chip), `TRIM OK` (the factory curves read), `ADC n MISS m` | TRIM OK (green). MISS stays 0 or near it |
| Bottom: F1..F4 | the faders 0..127 | full travel 0 to 127, the right bar moves |
| Bottom: K1..K8 | encoder transitions since boot (+ clockwise) | each knob moves its own number, + one way, - the other |
| Bottom: PITCH, MOD, PEDAL, BATT | raw 0..1023 | wheels move their numbers; pitch springs back |
| Bottom: 18 buttons | lit while held; the button's own LED lights too | each name lights for its button; its LED lights |
| Pads' LEDs | 1 s red, 1 s green, 1 s blue, then one pad white at a time 0..15; a pressed pad goes white | colours right (no R/G swap), the walk goes 0..15 in screen order (no shifted neighbour) |

**Hold KNOB BANK + FADER BANK for 2 s:** PERF-1 itself starts (the music, the FM-1's UI; the middle 27 keys are the
FM-1's keys, the pads its white keys). That's a first look, not the SMK controls (those come next).

## Install (writes the app area only; calibration, pairing, boot sectors are never written)

1. Close M-UPGRADE and anything using MIDI.
2. ```
   python -P G:\fm1\smk37\tools\fm1_install.py --port SMK G:\fm1\smk37\build\perf1-smk.fwsc
   ```
   Answer `y`. It checks, switches to update mode (black screen), writes, restarts. Don't unplug while it writes.
3. The SMK boots to HW TEST.

## What to send back

- Photos of the HW TEST screen: one at rest, one with a key, a pad and a fader held.
- A line per row of the table above: right / wrong (which key or pad lit instead).
- Anything odd: the screen offset or colours, a freeze, a restart.
- After the exit combo: does PERF play, does the audio sound right (USB audio / the SMK's own output)?

## Back to stock (always possible)

- From PERF-1 SMK: M-UPGRADE 3.11, Select firmware `G:\fm1\stock-smk37\SMK-37_Pro_016.fwsc`, Start upgrade.
  Or: `python -P G:\fm1\smk37\tools\fm1_install.py --force --port SMK G:\fm1\stock-smk37\SMK-37_Pro_016.fwsc`
- If it won't start: hold OCTAVE- + OCTAVE+ for 5 s (update mode), then the same.
- If it shows a `WL82 UBOOT1.00` disk: stop and ask (the dumps can restore anything, by hand).
- The first boot of PERF clears its windows (0x9D000..0xAEFFF, 0xB0000..0xB9FFF, 0xC2000..0xD9FFF: the stock's
  settings and part of USRFLASH, the stock presets). Whether stock rebuilds its presets when reinstalled isn't known
  yet (the kit doesn't say): if they're missing, the dump has them. The calibration (0xC1000) and pairing (0xC0000)
  are never written.
