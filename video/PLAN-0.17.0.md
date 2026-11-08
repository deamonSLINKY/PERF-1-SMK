# PERF-1 0.17.0 update video: as built (2026-11-07)

Agreed with the maker 2026-11-07: DISCO 03, the new-genres coda in, no MPC/MPK part (their own video covers it), the Library and the Install
page as screenshots (the maker may record them instead), the MIDI learn page with a mock custom map, the custom maps for devices mentioned.

Covers everything since the 0.15.0 video (0.15.1 .. 0.17.0). One take of one device, no cuts, no zoom, made from the 0.17.0 firmware
(`build/emu/amb.wasm`): `driver/scenes/update0170.mjs` (set out of sight by `prepare0170` in `driver/scenes/probe0170.mjs`: DISCO 03 played
and stopped, THEORY as the visualizer, a set list of DISCO 03 / FUNK 07 / SYNTHWV 03). The web pages are full-frame stills over the take's
music (cue `web`), so the music never stops. **Length 5:19** (title 4.5 s, the take 5:14).

## Rebuild

```
node driver/capture.mjs driver/scenes/update0170.mjs public/update0170      # ~2 min, deterministic (audio sha1 7dc05ef62367)
node driver/webshot.mjs "https://deamonslinky.github.io/PERF-1/pieces/#library" public/web/library.png "" 8000
node driver/webshot.mjs "https://deamonslinky.github.io/PERF-1/install/" public/web/install.png driver/websetup/install.js 6000
node driver/webshot.mjs "https://deamonslinky.github.io/PERF-1/editor/" public/web/midilearn.png driver/websetup/editor.js 6000 1600 1000
node driver/render.mjs out/perf1_update0170.mp4 update0170 web
```

Why render.mjs: the first plain `npx remotion render` died at the audio mix ("remotion-audio-mixing\0.wav: No such file or directory")
after Remotion had copied all of `public/` (5+ GB of old traces) into C:'s Temp with C: at 95%. render.mjs hard-links only the named
public folders into tmp/public and puts Temp on G: (tmp/temp). webshot.mjs keeps its Edge profile in tmp/ and closes Edge fully.

`webshot.mjs` drives headless Edge; the setup scripts (scroll to Full speed; put the mock map in localStorage, reload, Learn on, the list
open) are `driver/websetup/*.js`; the mock map as a file is `maps/mock-8-knob-controller.perfmap` (CC 70-77 knobs, CC 78 an encoder).
To swap in your own recording of a page: replace `public/web/<name>.png` with a still, or tell me and I'll make the cue play a clip.

## The storyboard, cut 2 (video time = take time + 0:04.5; 5:19)

Cut 2, the maker's notes on cut 1: PSY Y01 (cosmic psy-ambient) not Y06 (its FM bass too harsh); no return to THEORY after the Progression
page, THEORY itself shorter; Auto FX every 4 bars and the take moves on once one has played (it waits for the header to name it); a
full-frame thank-you in the maker's words; a pulsing NEW flag on the Library still.

| video | on the device | card | readout |
|---|---|---|---|
| 0:00 | title: PERF-1, THE 0.17.0 UPDATE | | |
| 0:04 | the boot logo, then DISCO 03 (S03, G Dorian, 122), not HYPNO | **Power on where you left off** | DISCO 03 |
| 0:13 | the home; the header shows DRUM DRIFT PATTERN 2 at 0:22 | **Song or Loop** | FILL, PATTERN 2 |
| 0:25 | HOME twice: THEORY | **The circle of fifths** | |
| 0:34 | G7, red V OF THE NEXT, into C7 | **Secondary dominants** | V OF THE NEXT · G7 → C7 |
| 0:42 | IV C7 on THEORY | **Phrases end at home** | |
| 0:47 | hold EDIT: the Progression page; KNOB3 to 1 bar; black key 1: ii V I I | **Eight chord slots**, **Fills on the black keys** | ii V I I |
| 1:00 | KNOB2 to 11TH on slot 1 | **A chord type a slot** | 11TH |
| 1:08 | black key 5: the whole circle | **The whole circle** | THE WHOLE CIRCLE |
| 1:16 | white key 9: W9 BASS: PEDAL, then off | **Pedal** | PEDAL: BASS |
| 1:28 | KNOB4 to WALK | **Back to the walk** | WALK |
| 1:31 | LFO x5: Bass steps; steps 3, 7, 11, 15 | **Sixteen steps of bass**, **Accent, glide, octave**, **Saved with the piece** | each step |
| 2:07 | hold HOME: FILTERS, Pad to BAND and back | **Filter types** | PAD: BAND |
| 2:18 | SCREAM on the bass, Drive 60, Scream 80 | **SCREAM** | DRIVE 60 · SCREAM 80 |
| 2:40 | FX: hold FILTER, DELAY, PHASE | **Effects on chosen tracks** | each |
| 2:56 | AUTO FX: Medium, every 4 bars; it plays a Phaser at 3:14 | **Effects played for you**, **On the phrase lines** | AUTO FX: PHASER |
| 3:18 | GLO twice: the set list | **Sixteen pieces, any genre** | |
| 3:25 | INDUSTRIAL (I03), FOREST (O03), PSY Y01 FADED ROOM (92) | **Industrial**, **Dark forest**, **Psytrance** | each |
| 3:57 | TRIP | **TRIP** | |
| 4:05 | the Library still, a pulsing NEW flag | **Share pieces, play other people's** | |
| 4:16 | the MIDI learn still (the mock map) | **MIDI learn, any controller** | |
| 4:28 | the Install page's Full speed still | **Full speed, newer FM-1s** | |
| 4:38 | TRIP | **Most of this came from you** (#2, #3, #5, #6, the commenters) | |
| 4:45 | full frame over TRIP: "Thank you." For the awesome feedback, the comments and the ideas. It makes me want to keep pushing harder. Keep the suggestions coming. | | |
| 4:57 | credits, two pages, over TRIP | | |

## Cut 3: the cold open and the Shorts (2026-11-07)

- **Cold open** (src/Full.jsx COLD_FROM, COLD_FRAMES): the take's 0:31.5 to 0:37.5 (G7 turns red, V OF THE NEXT, into C7) before the
  title, its music carried on under the title and faded out before the boot. Everything after moves 6 s later: the video is 5:25, the
  storyboard's times above + 0:06. COLD_FRAMES = 0 takes it out.
- **Shorts** (src/Short.jsx SHORTS, compositions ShortTheory, ShortFills, ShortBass; 1080 x 1920): a slice of the same take each, the
  screen big, the card and the readout under it, the panel with its cue rings at the foot, a one-line hook. Take 21-46 s, 45.5-84 s,
  86.9-121.9 s. Render: `COMP=ShortTheory node driver/render.mjs out/short0170_ShortTheory.mp4 update0170` (render.mjs takes COMP).
- Left out for this video, the maker's call: real-unit footage (no recording gear at hand), naming commenters (no waiting on permission).

## Found while building

- THEORY spells F7's seventh as **D#** in G Dorian (it should be Eb): theory.h's borrowed note raises the scale note below instead of
  lowering the one above. Seen at take 57 s in the probe; the video leaves THEORY before then and comes back on the hand-set circle, whose
  D7 (F# red) is right. Fixed on vis in 8f015ad (after this render): a later render, with the emulator rebuilt, may show that screen.
- The Song form's pattern changes show in the header as DRUM DRIFT PATTERN n.
- Knob turns: 120 ms a detent to go down to an end, 200 ms to count up exactly (faster ones accelerate).
- Black keys by index: b0 = key 1, b4 = key 10 (`emu.mjs` BLACK_K), not 8.
