# PERF-1 SMK: PERF-1 on the M-VAVE SMK-37 Pro

PERF-1 on the SMK-37 Pro: the same firmware, the same pages and the same manual, with the SMK's keys, pads, faders and
wheels added. SMK-37 board layer by genkigenki (fm1-to-smk37, fm1-amb's smk37 branch).

## The controls

| SMK-37 | PERF-1 |
|---|---|
| K1 | MASTER |
| K2 / K3 / K4 | SELECT / ALGORITHM / PRESETS |
| K5 .. K8 | KNOB 1 .. 4 |
| PATCH, CHORD, ARP, NOTE REPEAT, SEQ, FX, GLOBE, PARA, BT | HOME, ENV, ARP, LFO, SEQ, FX, GLO, EDIT, SAVE |
| SEQ PLAY / SEQ REC | PLAY / REC |
| SCALE | SEL (shift) |
| OCTAVE- / OCTAVE+ | OCT- / OCT+ |
| All 37 keys | notes, with velocity, on every page: Jam, a voice by hand, the page's voice; on Global, the scale |
| KNOB BANK | the pads' mode: PERFORM, MIX, CHORDS, NOTES, FOLLOW (lit when not PERFORM) |
| FADER BANK | the faders' bank: A, B, C (lit when not A) |
| < / > | the pads' page (keys 17-27, MIX's scenes) or, in NOTES, the voice |
| Faders 1-4 | the bank's four (a fader takes over once it moves) |
| Pitch wheel | bend, two semitones, on the voice the keys play (not the pad) |
| Mod wheel | vibrato on the voice the keys play, up to half a semitone |

## The pads

A page that uses the FM-1's keys as buttons takes the pads over by itself. Count the FM-1 manual's keys from F3:

| Where | Pads |
|---|---|
| Sequencer (SEQ held), Acid / Bass / Pluck steps, Steps, the gate, SAMPLE | the 16 steps: top row 1-8, bottom row 9-16 |
| Set list, Progression, and SCALE (SEL) held | top row the white keys 1-8 (slots, patterns), bottom row the black keys 1-8 (parts, fills, copies) |
| FX keys, GLOBE held (the mix map) | keys 1-16 in order; `>` keys 17-27, `<` back |

Everywhere else, the mode KNOB BANK picks:

| Mode | Top row | Bottom row |
|---|---|---|
| PERFORM (the start) | breakdown, build, drop, next chord, fill, freeze, auto-DJ, next piece | kick, snare, clap, closed hat, open hat, perc (velocity), all drums muted / back |
| MIX | mute kick, hats, snare, perc, bass, acid, pad, pluck | solo the same |
| MIX, `>` | scenes 1-4 (tap recalls, hold stores), reset all, undo, tap tempo, all drums | all drums solo, auto-DJ, its length, new pieces |
| CHORDS | the chord on degrees I-VII | |
| NOTES | the pluck in key (`>` the bass, the acid), the eight higher tones | the eight lowest tones |
| FOLLOW | F1 root, F2 third, F3 fifth, F4 colour (each on / off), FOLLOWERS on / off, Style, Colour, Evolve | the fills: ii V I, vi ii V I, iii vi ii V I, I vi ii V, the whole circle, I IV V V, I I IV vi, I IV I V |
| FOLLOW, `>` | (the same) | fills I iii IV vi, I vi IV, IV V IV I; Move of F2-F4, F1 hold / follow, Register, Glide, Pulse rate |

The pads light in the mode's colour, white while held; MIX shows the mutes red and the solos white. Velocity 30 at the
softest (the SMK's factory pad curve starts high).

## FOLLOWERS (FOLLOW pads)

Drones on the chord, for playing over (Steven's idea, issue #3): the pad's four voices become four followers, low and
sustained. The pad stops playing its own chords while they are on; FOLLOWERS off gives it back.

| | |
|---|---|
| F1 root | C2 .. B2. **Hold** (the default): the key's root the whole time, the chords moving above it; or Follow each chord's root |
| F2 third, F3 fifth | C3 .. B4, the chord's third and fifth. **Common** (the default): a note the next chord also has stays, otherwise the nearest of its own; Follow: always its own; Hold: the key's own chord |
| F4 colour | the chord's 7th (the default), 9th, 6th, sus4 or octave |
| Evolve | off, slow (every 4 chords), faster (every 2): the colour changes by itself, in the scale, mostly 7ths and 9ths |
| Style | Swell (sustained, breathing slowly), Pulse (the notes again at the rate, 1/4, 1/8, dotted 1/8 or 1/16, a slow attack), Drift (8 bars of each) |
| Register | low, mid, high (an octave each) |
| Glide | 0, 0.4, 0.8, 1.4, 2 s between notes |

The pad's level (fader A4), its sound (PADTONE), attack and release (Pad env) and every effect act on them. The bottom row's
fills write the progression and it plays at once. Kept with the piece: SAVE stores them, another piece has its own.

## The faders

| Bank | 1 | 2 | 3 | 4 |
|---|---|---|---|---|
| A levels | drums | bass | acid | pad |
| B FX | filter | crush | throw | wash |
| C shape | motion | evolve | acid density | drift |

## Not on the SMK

TRS MIDI IN (the SMK's jack is a MIDI out, not used yet): clock in comes over USB. The boot stub (OCT- at power-on):
for update mode, hold OCTAVE- + OCTAVE+ for 5 s.

## Install and the way back

See `SMK37-HWTEST.md`. KNOB BANK held at power-on opens the hardware test (KNOB BANK + FADER BANK for 2 s leaves it).
