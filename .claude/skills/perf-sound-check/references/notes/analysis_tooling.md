# Open-source audio/music analysis tooling for an automated Python "sound check"

(Saved by the coordinator from the researcher's returned text; the researcher's Write was refused. Lines with a
[Source](URL) were checked by the researcher; "(standard API, not re-verified)" lines are from long-standing docs.)

## 1. Loudness and true peak

### Takeaway
pyloudnorm (MIT, numpy+scipy) for integrated LUFS. It has no documented true-peak meter: compute true peak with 4x
oversampling (`scipy.signal.resample_poly`) or cross-check with ffmpeg `ebur128=peak=true`. ffmpeg `astats` gives DC,
clipping (Flat_factor, Peak_count), noise floor and NaN/Inf/denormal counts in one pass.

### Cited Findings
- pyloudnorm: MIT, NumPy+SciPy, "Implementation of ITU-R BS.1770-4"; `DeMan` filter option is "fully compliant"; LRA is
  only an "attempt" at EBU Tech 3342; usage `pyln.Meter(rate).integrated_loudness(data)` with data (samples, channels);
  the only peak function is `pyln.normalize.peak()`, not true peak — [GitHub](https://github.com/csteinmetz1/pyloudnorm)
- pyloudnorm 0.2.0 released 2026-01-04, Python 3.9+ — [PyPI](https://pypi.org/project/pyloudnorm/). 0.0.1 listed "Add true
  peak measurement" as to-do; no evidence 0.2.0 added it — [PyPI 0.0.1](https://pypi.org/project/pyloudnorm/0.0.1)
- PyPI `loudness` 0.2.0 (2025-12-26, MIT) binds libloudness; only `loudness.integrated_loudness(audio, sr)` documented;
  Windows x64 wheels Py 3.10-3.14; no LRA/true peak mentioned — [PyPI](https://pypi.org/project/loudness/)
- ffmpeg `ebur128`: `peak` = none / sample (SPK) / true (TPK, per-frame FTPK; needs libswresample); `metadata=1` emits
  `lavfi.r128.*` every 100 ms; `framelog` quiet/info/verbose; `dualmono` with `panlaw` (-3.01 dB); outputs integrated,
  range, lra_low, lra_high, sample_peak, true_peak; oversampling factor not stated —
  [docs](https://ayosec.github.io/ffmpeg-filters-docs/7.1/Filters/Multimedia/ebur128.html)
- ffmpeg `astats`: options length (0.05 s), metadata, reset, measure_perchannel, measure_overall; fields DC_offset,
  Crest_factor, Flat_factor (consecutive equal samples at peak), Peak_count, Abs_Peak_count, Noise_floor (minimum local
  peak, dBFS), Number_of_NaNs/Infs/denormals, Peak_level, RMS_level, RMS_peak, RMS_trough, Zero_crossings, Entropy,
  Bit_depth; keys `lavfi.astats.<ch|Overall>.<Key>` — [docs](https://ayosec.github.io/ffmpeg-filters-docs/7.1/Filters/Audio/astats.html)
- BS.1770-4 Annex 2 true peak: 4x oversampling via a 48-tap polyphase FIR (12 taps/phase), then max abs —
  [ebur128-stream](https://docs.rs/crate/ebur128-stream/0.2.0/source/src/peak.rs), [Slate Digital](https://slatedigital.com/how-we-designed-the-bus-clipper/);
  2x is enough at 96 kHz — [cacophony.js](https://cacophony.js.org/classes/TruePeakDetector.html); a BS.1770-3-era paper uses
  40 taps — [KoreaScience](https://koreascience.kr/article/CFKO201636257094978.pdf)
- Essentia `TruePeakDetector` exists but is AGPL — [Essentia](https://essentia.upf.edu/reference/std_TruePeakDetector.html)

### Inferences
- True peak in numpy:
  `up = resample_poly(np.pad(x, ((256,256),(0,0))), 4 if sr < 96000 else 2, 1, axis=0); tp = 20*log10(abs(up).max())`
  (Kaiser FIR, not the Annex 2 filter: expect within ~0.1 dB; validate once against ffmpeg).
- ffmpeg: `ffmpeg -hide_banner -nostats -i f.wav -af ebur128=peak=true:framelog=quiet -f null -` (stderr; regex the
  Summary block for `I:`, `LRA:`, true `Peak:`); `-af astats=metadata=1,ametadata=print:file=stats.txt` for machine output.
  Pin the parser with a fixture.
- Gates: LUFS in a target window; TP <= -1 dBTP; sample peak < 0 dBFS; zero NaN/Inf/denormal; LRA info only.
- Clips < 400 ms give -inf/NaN LUFS: guard, use short-term/momentary.

### Gaps
pyloudnorm 0.2.0 true-peak status; pyebur128 maintenance; official Annex 2 coefficients; ffmpeg's oversampling factor.

## 2. Spectrum and tonal balance

### Takeaway
scipy/numpy cover it all (Welch, 1/3-octave bands, K-weighting biquads, correlation, M/S, DC, clip runs, clicks); librosa
adds centroid/rolloff/flatness; astats as an independent check.

### Cited Findings
- librosa 1.0.0 (2026-08-11), ISC, Python 3.12+ — [PyPI](https://pypi.org/project/librosa/). 1.0 removed the audioread
  backend, set/get_fftlib, `win_length` in yin/pyin, `x_axis` in waveshow, scalar→0-d promotion; 0.11 (Mar 2025) made
  scipy the default FFT and supported NumPy 2 — [changelog](https://librosa.org/doc/latest/changelog.html)

### Inferences
- Pin `librosa>=1.0` on Py 3.12+ (or `<1.0` on older); load audio with soundfile.
- Welch: `welch(mono, fs=sr, nperseg=8192, noverlap=4096)`. 1/3-octave centres `1000*10**(np.arange(-17,14)/10)`, edges
  centre·10^(±1/20); sum P·df per band → dB; compare to a stored reference or fit a dB/octave slope.
- librosa features: spectral_centroid, spectral_rolloff(0.85), spectral_flatness, spectral_bandwidth, spectral_contrast;
  summarise median and IQR (standard API, not re-verified).
- Stereo: `np.corrcoef(L,R)` global and minimum over 400 ms windows; side/mid ratio dB; correlation below 150 Hz.
- DC: mean per channel and per 1 s window; flag > about -60 dBFS.
- Clipping: runs of >= 3 samples at |x| >= 0.999; longest run and timestamps; plus TP > 0 dBTP for inter-sample clipping.
- Clicks: high-pass (~8-10 kHz) or `np.diff(x,2)`; robust z vs local MAD over ~10 ms; flag isolated z of 8-10; check hit
  positions modulo the simulator block size (32/64/128): clustering at block boundaries = buffer bug.
- Noise floor: 5th percentile of 50 ms RMS; a no-note render should be digital zero.
- Speed: each is O(n) or one FFT pass: well under 1 s per minute.
- K-weighting: reuse pyloudnorm's filters; for loudness only, not spectral views.

### Gaps
K-filter coefficients / pyloudnorm internals unverified; no standard click threshold: tune on good and bad renders.

## 3. Rhythm and tempo

### Takeaway
librosa (ISC) for onsets, beats, tempograms. Swing and microtiming from the MIDI as ground truth; audio only confirms
onset placement, latency, jitter. madmom code BSD but models CC BY-NC-SA 4.0; BeatNet CC BY 4.0 (needs madmom, probably
torch); Essentia AGPL-3.0.

### Cited Findings
- madmom: BSD code, models/data CC BY-NC-SA 4.0; needs numpy, scipy, cython, mido, ffmpeg for non-44.1k/16-bit; models in
  a submodule — [GitHub](https://github.com/CPJKU/madmom)
- BeatNet: CC BY 4.0; needs librosa, madmom, PyAudio for streaming; modes stream/realtime/online/offline; madmom 0.16.1
  breaks on Py 3.10+ and NumPy 1.24+ (patches for collections.abc, np.float/np.int); Windows PyAudio needs a manual wheel —
  [GitHub](https://github.com/mjhydri/BeatNet)
- Essentia: PyPI AGPL-3.0, 2.1b6.dev wheels up to Py 3.11 — [ecosyste.ms](https://packages.ecosyste.ms/registries/pypi.org/packages/essentia), [MTG](https://github.com/MTG/essentia)

### Inferences
- librosa: onset_strength, onset_detect(units="time"), beat_track (tempo is an ndarray: `float(tempo[0])`),
  feature.tempogram, feature.rhythm.tempo (standard API, not re-verified). Hop 512 ≈ 11.6 ms: use hop <= 128 or refine ±5 ms.
- Swing from MIDI: quantise to the 8th/16th grid from known BPM/PPQ; swing = off-beat position as % of the beat (50
  straight, 66.7 triplet); microtiming = mean/SD offset per step index.
- Swing from audio: fold onsets on beat phase using the known tempo; histogram.
- Firmware checks: tempo error ppm, jitter SD, audio-vs-MIDI latency (cross-correlate onset envelope with a MIDI impulse
  train), drift (slope of offset over time).
- Licences: AGPL combinable with GPL-3.0 (§13) but the combination takes AGPL terms: keep essentia optional. madmom's NC
  models conflict with GPL's "no further restrictions": never vendor.

### Gaps
madmom's current release / Py 3.12 status; BeatNet torch dependency.

## 4. Pitch and harmony

### Takeaway
Key: librosa `chroma_cqt` vs 24 rotated Krumhansl-Kessler profiles. Tuning: `librosa.pyin` cents error per test note.
Chords: Chordino via `chord-extractor` (vamp + compiled plugin: WSL). For MIDI renders, analyse the MIDI with music21.

### Cited Findings
- chord-extractor wraps Chordino (NNLS Chroma Vamp); install numpy first; bundles a 64-bit Linux Chordino; PyPI 0.1.2,
  Fedora 0.1.3 — [PyPI](https://pypi.org/project/chord-extractor), [docs](https://ohollo.github.io/chord-extractor)
- NNLS Chroma licence text not found — [SlackBuilds](https://www.slackbuilds.org/repository/15.0/audio/nnls-chroma/)
- pyin lost `win_length` in 1.0; accuracy improved in 0.11 — [changelog](https://librosa.org/doc/latest/changelog.html)

### Inferences
- KK profiles (verify before relying): major `[6.35,2.23,3.48,2.33,4.38,4.09,2.52,5.19,2.39,3.66,2.29,2.88]`, minor
  `[6.33,2.68,3.52,5.38,2.60,3.53,2.54,4.75,3.98,2.69,3.34,3.17]`; correlate mean chroma with all 24 rotations; top-two
  margin = confidence. MIDI: `music21 stream.analyze('key')`.
- Pitch: `librosa.pyin(y, fmin=note_to_hz('C2'), fmax=note_to_hz('C7'), sr=sr)`; cents = 1200·log2(f0/expected);
  `estimate_tuning` for a global offset; pyin is slow: short test notes only. CREPE/torchcrepe need TF/Torch.
- Chords: Chordino; madmom CNN/CRF (NC models); autochord (TF); numpy template matching on beat-synced chroma.

### Gaps
Licences of chord-extractor, Chordino, autochord, CREPE, torchcrepe; no accuracy benchmarks.

## 5. Structure

### Takeaway
librosa is enough: RMS / short-term loudness curves, spectral-flux novelty, recurrence matrix, agglomerative segments.
MSAF (MIT) heavier, maintenance unclear. Builds/drops by heuristics.

### Cited Findings
- MSAF: MIT, PyPI `msaf`, 1,375 commits, CI, 19 open issues; no release/Python version in README — [GitHub](https://github.com/urinieto/msaf)

### Inferences
- `feature.rms`, `segment.recurrence_matrix(mode='affinity')` + checkerboard novelty or `segment.agglomerative(feat,k)`,
  `util.peak_pick`.
- Build: rising short-term loudness, high-band energy, onset density and centroid over 4-16 bars. Drop: a dip, then a
  >= 6 dB low-band jump at a bar boundary. Report as candidates with times, not pass/fail.

### Gaps
MSAF on Py 3.12 unverified.

## 6. MIDI analysis

### Takeaway
mido (raw messages, timing), pretty_midi (notes, piano roll, chroma), music21 (key, Roman numerals): MIT, MIT, BSD-3.
muspy has the metrics but is stale since 2022: re-implement in numpy.

### Cited Findings
- music21 10.5.2 (2026-10-28), BSD-3, Py 3.11+; 11.0.0b14 pre-release — [PyPI](https://pypi.org/project/music21/)
- pretty_midi MIT; `estimate_tempo()`, `get_chroma()`; synthesis needs fluidsynth — [GitHub](https://github.com/craffel/pretty-midi)
- muspy 0.5.0 (2022-04-16), MIT — [PyPI](https://pypi.org/project/muspy/). Metrics: pitch_range, n_pitches_used,
  n_pitch_classes_used, polyphony, polyphony_rate(threshold=2), pitch_in_scale_rate(root, mode), scale_consistency (max over
  major/minor), pitch_entropy, pitch_class_entropy, empty_beat_rate, drum_in_pattern_rate(meter),
  drum_pattern_consistency, groove_consistency (1 − mean Hamming distance of neighbouring measures' onset vectors),
  empty_measure_rate; most NaN on empty input — [MusPy metrics](https://hermandong.com/muspy/metrics.html)

### Inferences
- Each metric ≈ 10 lines of numpy over a note list (pitch-class entropy = −Σ p log2 p).
- Chord transitions: pitch-class set per beat → music21 `chord.Chord(...).pitchedCommonName` or
  `roman.romanNumeralFromChord`; transition matrix + entropy.
- Voice leading: `scipy.optimize.linear_sum_assignment` on |p−q|; mean semitones per voice, parallel 5ths/8ves, crossings.
- Groove/health: velocity mean/SD and accents per step, microtiming per step, stuck notes (on without off), duplicate notes
  on a tick, MIDI clock (0xF8) jitter via mido.

### Gaps
miditoolkit, pretty_midi versions; mido licence.

## 7. Reference-free metrics and A/B between builds

### Takeaway
The simulator is deterministic: with a fixed seed and the same input, two builds should be bit-identical, so a null test
is the primary check. When builds legitimately differ: per-band dB deltas, log-mel distance, metric deltas.

### Inferences (no sources retrieved)
- Null test: align by FFT cross-correlation; least-squares gain g = <a,b>/<b,b>; residual r = a − g·b; report lag, gain,
  residual peak/RMS dBFS, null depth. All-zero = bit-exact; > ~90 dB = float rounding; less = a real change.
- Spectral difference: mean |ΔdB| over 1/3-octave bands; median and 95th percentile log-mel frame L2; a spectral-flux
  difference curve shows where builds diverge.
- Per-render checks: NaN/Inf zero; no clip runs; TP <= ceiling; DC below threshold; no clicks; release tail decays below
  the noise floor (stuck voices); correlation > 0; pitch within ±N cents; tempo within N ppm.
- ViSQOL, PEAQ, FAD optional and heavy; not needed for regression.

## 8. Reporting

- matplotlib Agg: waveform, 1/3-octave vs reference, spectrogram, loudness curve, onset-phase histogram, piano roll.
- One self-contained HTML or Markdown with a pass/fail table, plus `report.json` for CI; embed plots as base64.
- Compute the STFT once (`S=`); rhythm/structure at 22.05 kHz; multiprocessing across files; ffmpeg optional cross-check.
- librosa 1.0: `waveshow(x_axis=)` gone; `display.cmap` → `infer_cmap` (old name removed in 1.1).
- Speed: a few seconds per minute without pyin/NN models (estimate, not benchmarked).

## 9. Licences and dependency weight

| Package | Licence | Version (date) | Notes |
|---|---|---|---|
| pyloudnorm | MIT | 0.2.0 (2026-01-04) | numpy+scipy only |
| loudness | MIT | 0.2.0 (2025-12-26) | Windows wheels |
| librosa | ISC | 1.0.0 (2026-08-11), Py 3.12+ | numba |
| madmom | BSD code; models CC BY-NC-SA 4.0 | ? | Cython; NumPy patches |
| BeatNet | CC BY 4.0 | | needs madmom |
| essentia | AGPL-3.0 | 2.1b6.dev | wheels to Py 3.11; WSL |
| music21 | BSD-3 | 10.5.2 (2026-10-28) | pure Python |
| pretty_midi | MIT | | pure Python |
| muspy | MIT | 0.5.0 (2022) | stale |
| MSAF | MIT | | maintenance unclear |
| chord-extractor | unconfirmed | 0.1.2/0.1.3 | vamp; Linux binary |

Core stack (numpy, scipy, soundfile, pyloudnorm, librosa, mido, pretty_midi, music21, matplotlib) is permissive,
GPL-3.0-compatible, wheels on native Windows. Keep essentia (AGPL) and madmom models (NC) out of required deps; WSL for
essentia, vamp/Chordino, madmom. Calling the ffmpeg CLI via subprocess is not a combined work.

Gaps: mido, miditoolkit, autochord, CREPE, Chordino licences; the specific ffmpeg build's licence; numba on Py 3.14.
