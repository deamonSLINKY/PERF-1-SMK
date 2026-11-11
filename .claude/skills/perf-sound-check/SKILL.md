---
name: perf-sound-check
description: Sound check and music-knowledge base for the PERF-1 firmware (FM-1 / SMK-37 generative techno and acid engine). Renders genres or pieces in PERF's host simulator, measures the WAV and MIDI (LUFS, true peak, PSR, spectrum balance, bass mono, stereo, clicks, clipping, DC, tempo, swing, drum grids, key, progressions), judges them against genre targets drawn from mixing, mastering and genre research, compares against the previous dev build, and explains the results in plain words with fixes in the engine. Use it whenever the user asks how PERF sounds, whether a build made the mix better or worse, to check a genre, piece, voice, effect or master change, to tune loudness, low end or drums, after any DSP or sound change in amb.c, or when they ask about music theory, chord progressions, drum patterns, genre conventions, synth patches (303, 808, 909, supersaw, Reese...), mixing or mastering for PERF, even if they don't say "sound check".
---

# PERF sound check

PERF is a generative instrument: nobody mixes its output by hand, so the engine itself has to produce a balanced,
genre-true mix in every piece. This skill gives you two things for that:

1. **A measuring tool**, `scripts/soundcheck.py` (Python 3.9+ and numpy only), that reads a render's WAV and its MIDI
   out and judges them against `references/targets.json`.
2. **A knowledge base**, `references/knowledge.md` (the synthesized research, with a 42-rule table) and
   `references/notes/*.md` (the detailed, cited research notes), for answering design questions about theory,
   drums, genres, patches, mixing and mastering.

Measurements are a guide, not the judge: the maker's ears decide. Say what the numbers suggest, how sure that is
(sourced vs heuristic), and what to listen for.

## Running a sound check

### 1. Find out how this tree renders audio

The host simulator (`host/amb_host.c`, driven by `.amb` scenario scripts in `tests/scenarios/`) renders audio for
the tests; the tree may already have WAV tools (look for `wavstat`, `gpeak`, `--wav`, `render` in `host/`,
`tools/` and `tests/run_tests.sh`). Read how a scenario writes a WAV before inventing anything. You need, per render:

- a **stereo WAV** at the engine's rate (16-bit, 24-bit or float all work), long enough to hear a section change:
  **64 bars** is a good default (about 2 minutes at 128 BPM). Short clips (< 3 s) give no loudness reading.
- the **MIDI out** of the same run as a `.mid` file, if the simulator can write one. It's the ground truth for
  tempo, swing, drum grids, key and chords. If it can't, the audio checks still run; offer to add a MIDI-file
  writer to the host (PERF already sends MIDI out: pad ch1, pluck ch2, bass ch3, acid ch4, drums ch10).
- a **fixed seed**, so two builds can be compared sample for sample.

Render one piece per genre for a quick check, or all ten pieces of a genre when that genre changed. Keep the output
in `build/soundcheck/<dev version>/` so the next build has something to compare against.

### 2. Map PERF's genres to the targets

`targets.json` names genres like `techno`, `acid`, `dub_techno`, `dnb`, `lofi`, `ambient`. PERF's genre names
(HYPNO, LOFI, DUBSTEP, FOREST...) don't all match. The first time, write the mapping to
`tools/soundcheck_genres.json` in the repo (PERF genre → target key; unknown ones → the closest profile, and say so)
and reuse it after that.

### 3. Measure

```
python <skill>/scripts/soundcheck.py analyze build/soundcheck/0.17.18/techno_3.wav \
    --midi build/soundcheck/0.17.18/techno_3.mid --genre techno --block 256 \
    --json build/soundcheck/0.17.18/techno_3.json
python <skill>/scripts/soundcheck.py report build/soundcheck/0.17.18 --out build/soundcheck/0.17.18/report.md
python <skill>/scripts/soundcheck.py compare build/soundcheck/0.17.17/techno_3.json build/soundcheck/0.17.18/techno_3.json
python <skill>/scripts/soundcheck.py compare old.wav new.wav      # adds a null test: bit-identical, or how deep
```

`--block` is the engine's audio block in frames (PERF renders 256-frame halves on the FM-1; check the tree): clicks
that cluster on block edges well above chance point at buffer or core hand-off bugs. `--bpm` overrides the tempo when
there's no MIDI.

**Reading clicks.** When clicks repeat at a steady period the tool reports `clicks_period_s`, `clicks_per_beat` and
`clicks_phase_in_beat_s`. A click once a beat at a fixed time after the beat is a voice ending without a fade (the
phase is the voice's length: a kick cut at 350 ms) or a voice being stolen; a click right on the beat is an attack
starting mid-waveform. Check it on the waveform around one click before naming the cause, and compare the phase with
the MIDI note-ons.

**Check the render length** against what was asked for (bars × 4 × 60 / BPM): a render shorter than intended changes
the loudness range and arrangement findings, so say so first.

Run `soundcheck.py selftest` once in a new environment: it checks the meters against signals with known answers
(EBU Tech 3341 tone, inter-sample overs, pink noise, clicks, swing, key).

### 4. Explain it

Write the result for the maker, who knows the instrument well but isn't a mastering engineer:

- **Lead with what matters**: fails first, then warnings, then what got better. One line each, in plain words:
  "The bass isn't mono below 120 Hz (correlation 0.82): on a club system it will thin out."
- **Point at the engine**, not just the number: which voice, effect or stage probably causes it (the pad's chorus
  width, the echo's feedback on the bass, the master pump, the acid's resonance, voice stealing...), and what change
  would test that idea. Read the relevant code before naming a cause.
- **Say how sure it is.** Every rule has a `source`. "sourced" rules (true peak, PSR, sub mono, LUFS of club masters,
  BPM bands) are solid. "heuristic: calibrate" ones are starting points: present them as "worth a listen", never as
  a verdict, until they've been calibrated (step 6).
- **Remember PERF is an instrument, not a master.** Its absolute loudness depends on the MASTER knob, so loudness is
  a gain-staging hint. Balance (band shares, tilt), dynamics (PSR), mono bass, clicks, clipping and timing are what
  the engine controls.
- **Compare builds** when there's a previous run: "0.17.18's acid is 1.8 dB brighter in 2-8 kHz and PSR fell from
  9.1 to 7.6: the new ladder drive is squashing the peaks." A bit-identical null test means the change didn't touch
  that piece's sound.
- **End with what to listen for**, and a next step (a change to try, a render to make).

Put the full report next to the renders, and give the summary in chat.

### 5. Checking one sound

For a patch rather than a mix (a new kick, the 303 voice, a pad), render the sound alone (a scenario that solos the
voice or plays one note) and check it against `references/knowledge.md` "Sound design" and `notes/synth_patches.md`:
the kick's low peak (45-65 Hz for club kicks; the 808's measured median is 49.5 Hz, settling about a semitone over
0.4 s), the 303's accent behaviour (accented notes get the shortest filter decay and each accent in a run sweeps
higher; slides take a fixed time whatever the interval), the supersaw's detune and mix. Measure with the tool's
spectrum output, or a few lines of numpy on the WAV.

### 6. Calibrating the heuristics

Most thresholds are heuristics because no source gives them. Once the maker picks renders they like (by ear), run:

```
python <skill>/scripts/soundcheck.py calibrate build/soundcheck/liked/ --out build/soundcheck/calibration.json
```

It gives each metric's P10, median and P90 per genre. Propose updating `targets.json` from those (warn outside
P10-P90 of the liked set), mark each updated rule `"source": "calibrated: <date>, n renders"`, and show the maker
the diff before saving. Calibrated targets are worth more than any generic rule: they describe what PERF sounds
like when it sounds right.

## Answering music questions for PERF

When the maker asks about theory, a genre, drums, a patch, mixing or mastering (for a new genre, a factory piece,
the THEORY screen, Steven's followers, the MIDI study feels...), read `references/knowledge.md` first; go to the
matching note for detail:

| Question about | Read |
|---|---|
| loudness, peaks, PSR/PLR, spectrum, stereo, low end | `notes/mixing_mastering.md` |
| tempo, swing, step grids per genre, fills, builds, arrangement | `notes/drums_genres.md` |
| 303, 808/909, supersaw, Reese and other patches, registers | `notes/synth_patches.md` |
| keys, modes, progressions, voice leading, bass and acid lines | `notes/music_theory.md` |
| analysis libraries, licences, how to measure something new | `notes/analysis_tooling.md` |

The notes tag what's sourced and what's convention. Keep that distinction when you answer: "Aeolian is the default
in EDM (Faraldo's corpus: 85 % minor)" is a fact; "a trance offbeat bass on every 16th 3" is a convention to try by
ear. Turn answers into PERF terms: which genre table, step pattern, chord habit or voice parameter it would change.

## Things to keep in mind

- **Licences.** The tool is numpy-only on purpose. If you add a library, keep to permissive ones (MIT, BSD, ISC);
  Essentia is AGPL and madmom's models are non-commercial, so they stay optional and out of the repo. No copyrighted
  songs as reference material for anything that ships (the MIDI study spec's rule).
- **Known limits of the tool:** true peak uses a windowed-sinc 4x interpolator (within about 0.1 dB of BS.1770
  Annex 2); PSR uses window sample peaks plus the file's true-peak overshoot; the click detector misses a click that
  lands inside a hard drum attack, and a deliberately abrupt sound (a hard-cut sample, a truncated tail) counts as a
  click, which is usually right; key detection on EDM is only ~70 % accurate in the literature, so "ambiguous" (a
  small margin) is a valid answer; chords are one per bar from all tonal channels.
- **Optional plugins** the maker may have installed: AudioPod (stems, audio to MIDI) can turn the maker's own
  recordings into MIDI for the MIDI study; Audiorective helps with the browser emulator and web editor. Neither is
  needed for a sound check, and AudioPod sends audio to its servers, so only use it on the maker's own recordings.
