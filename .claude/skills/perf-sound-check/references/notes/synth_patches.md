# Synthesizer Sound Design and Classic Patches for Electronic Genres (recipes + measurable signatures)

Research note scope: TB-303 acid, classic patches, drum synthesis (808/909), register map, FM tips, measurable checks.
Budget-limited pass (15 tool calls). Several primary PDFs (Werner/Abel/Smith DAFx-14 808 BD paper, Devil Fish manual, Deruty arXiv PDF) downloaded as compressed binaries the fetch tool could not read; the arXiv HTML version did work. Many commonly quoted patch numbers could NOT be verified against a source in this pass; they are listed in Gaps as "values to verify", clearly separated from cited findings.

## 1. TB-303 acid sound: what makes it authentic

### Takeaway
Authenticity comes from the 303's accent system more than its oscillator: on accented notes the filter envelope (MEG) is forced to its shortest decay, is added to the VCA (louder), and is fed to the filter through a capacitor-smoothed "accent sweep" circuit whose charge accumulates over consecutive accents, so repeated accents climb higher, especially at high resonance. Slide is a fixed-time exponential (RC) glide, not a fixed-rate one. Emulations that treat accent as "velocity" or glide as constant-rate miss this.

### Cited Findings
- Two envelopes: VEG (volume) has a sharp attack and fixed, fairly long exponential decay and drives only the VCA; MEG (main env) has sharp attack, exponential decay, and mainly drives filter cutoff — [Robin Whittle, "What makes the TB-303 unique"](https://www.firstpr.com.au/rwi/dfish/303-unique.html)
- Normal notes: MEG decay set by the Decay pot. Accented notes: the Decay pot is shorted out, so MEG decay is always the minimum (pot fully anticlockwise) value regardless of the knob — [Whittle](https://www.firstpr.com.au/rwi/dfish/303-unique.html); consistent with [Patreon "Patch Ideas I Learned While Studying the TB-303"](https://www.patreon.com/posts/patch-ideas-i-tb-41917616) citing Robin.
- Env Mod on the original cannot reach zero even fully anticlockwise (always some filter sweep); Devil Fish allows zero and extends maximum — [Whittle](https://www.firstpr.com.au/rwi/dfish/303-unique.html)
- On accent, the MEG goes via a switch and the Accent pot to (1) the VCA through a 47k resistor + 0.033 uF capacitor (softened attack; main reason accented notes are louder) and (2) the filter via the Accent Sweep Circuit — [Whittle](https://www.firstpr.com.au/rwi/dfish/303-unique.html)
- Accent Sweep Circuit: diode + 47k in series to a 100k pot (the second gang of the Resonance pot), 1 uF capacitor to ground, wiper into a 100k summing resistor at the filter. With resonance low, filter follows the MEG closely; with resonance high, the MEG pulse is smoothed by the 1 uF cap (charged through diode + 147k) giving a smooth "wow" rise-and-fall. On repeated accents the cap has not discharged, so each successive sweep peaks higher; stronger with more resonance — [Whittle](https://www.firstpr.com.au/rwi/dfish/303-unique.html)
- Whittle argues this cannot be replicated on standard modular synths without special additional circuitry — [Whittle](https://www.firstpr.com.au/rwi/dfish/303-unique.html)
- Slide is a simple RC integrator: fixed time, exponential curve; sliding C-1 to C-3 takes the same time as C-1 to C#1 (constant-time glide) — [synth-diy list, 1998](https://synth-diy.org/pipermail/synth-diy/1998-April/010281.html)
- A modern clone (nt_303) defaults slide to 60 ms (range 1-200 ms) and cutoff to 20 Hz-10 kHz; these are clone parameters, not measured original-hardware values — [nt_303 GitHub](https://github.com/thorinside/nt_303)
- Filter slope DISAGREEMENT: some describe it as an 18 dB/oct ladder low-pass; [Polynominal](https://www.polynominal.com/m/roland-tb303.htm) calls it a 4-pole diode ladder VCF. A measurement project found the diode ladder behaves almost exactly per theoretical model but there is extra high-pass filtering after the filter that complicates fitting — [a1k0n/303 GitHub](https://github.com/a1k0n/303)
- Claim (single forum source) that the 303 filter does not self-oscillate at any resonance setting — [synth-diy, 1998](https://synth-diy.org/pipermail/synth-diy/1998-April/010281.html)
- Roland describes accent as boosting volume and filter cutoff — [Roland articles](https://articles.roland.com/?p=92325)

### Inferences
- Engine rules for an authentic 303 voice: (a) accent = forced-short MEG decay + VCA boost + a leaky-integrator (one-pole, diode-charged) added to cutoff whose state persists across notes; (b) the accent-sweep amount should scale with the resonance control (shared pot); (c) Env Mod minimum > 0; (d) slide = one-pole lag on pitch CV with fixed time constant (independent of interval), active when the previous step has slide and the gate is held (legato, no envelope retrigger); (e) filter non-self-oscillating, plus a post-filter high-pass.
- Common emulation mistakes implied by the above: accent implemented as simple velocity; constant-rate portamento; envelopes retriggered on slid notes; resonance that self-oscillates/whistles; Env Mod able to reach zero; no cumulative accent behaviour.
- Testable signatures: on a pattern of N consecutive accented notes, the per-note peak spectral centroid should increase monotonically over the run (cumulative sweep); glide time measured by pitch tracking should be roughly the same for a semitone and for two octaves; accented notes should show a shorter filter-decay (centroid decay time) than unaccented notes at the same Decay setting.

### Gaps
- Numeric MEG decay range (commonly quoted ~200 ms to ~2 s normal, ~200 ms accented) and VEG decay time not verified — the Devil Fish manual PDF that likely contains them could not be parsed.
- Original slide time in ms (often quoted ~60 ms) not verified for original hardware; only clone default found.
- Original cutoff range in Hz and exact pole count unresolved (18 vs 24 dB/oct disagreement above).
- Square vs saw: the 303 offers both waveforms (saw/square switch); no source fetched documented the square's derivation (commonly said to be waveshaped from the saw, with a non-50% duty / tapered top) — verify.
- Post-303 distortion practice (e.g., overdrive/fuzz pedals) not sourced in this pass.

## 2. Classic patches with recipes

### Takeaway
Only the supersaw (JP-8000) had a technical primary-source trail found in this pass (Szabo's KTH thesis): 7 saws, a non-linear (roughly exponential) detune curve, a mix control that rebalances centre vs side saws, aliasing as part of the character, and a high-pass filter. Other patch recipes (Reese, hoover, Juno pad, dub stab, M1 organ, rumble kick, wobble, amapiano log drum, etc.) were not verified with citable numbers in this pass and are listed as values to verify.

### Cited Findings
- Supersaw: thesis "How to Emulate the Super Saw" (Adam Szabo, KTH, nada.kth.se report 10131) targets the JP-8000/JP-8080 SuperSaw — [NI Reaktor user library entry](https://native-instruments.com/en/reaktor-community/reaktor-user-library/entry/show/9323)
- Detune curve is non-linear, giving finer control at low values and spreading rapidly near maximum; forum summary calls it "exponential-ish" — [NI Reaktor entry](https://native-instruments.com/en/reaktor-community/reaktor-user-library/entry/show/9323); [KVR forum](https://www.kvraudio.com/forum/viewtopic.php?p=5429065)
- Mix: at maximum mix the centre saw is smaller in amplitude than the side saws; at lower mix the centre equals or exceeds the sides — [KVR forum](https://www.kvraudio.com/forum/viewtopic.php?p=5448947)
- Character attributed to aliasing (bright/"airy"), with a high-pass filter removing content below the fundamental — [KVR forum / Synthtopia discussion](https://www.synthtopia.com/content/2011/03/28/super-saw-synth-for-windows/)
- A reverse-engineered fitting equation for the JP-8000 detune curve exists (schollz, SuperCollider, Nov 2022) — [schollz TIL](https://schollz.com/til/221103)
- 808-style boom bass: the 808 kick is a sine-like tone with pitch, so with long release it has distinct pitch/tuning; saturation is used to add harmonics so it is audible on small speakers — [Attack Magazine Synth Secrets: 808 bass with saturation](https://www.attackmagazine.com/technique/synth-secrets/808-bass-with-saturation/); [Attack: 808-style booms](https://www.attackmagazine.com/technique/synth-secrets/808-style-booms/)
- Drive/saturation keeps lasting harmonics in "driven" 808s, while clean "long" 808s decay to a lone fundamental — [Deruty, arXiv 2502.07524](https://arxiv.org/html/2502.07524v2)

### Inferences
- Supersaw check: render a single held note and measure 7 spectral peaks clustered around each harmonic; spread (cents) should grow non-linearly with the detune parameter; a high-pass should be visible below the fundamental.

### Gaps (commonly cited practitioner values, NOT verified in this pass — verify before encoding as rules)
- Szabo's exact 7 detune offsets and the mix polynomials (often reproduced as offsets approx. +/-0.110, +/-0.063, +/-0.019 of max and quadratic/linear mix laws) — the thesis PDF was not fetched.
- Reese bass: two (or more) saws detuned a few cents to ~10-20 cents, low-pass ~300 Hz-1 kHz, often with a separate clean sub sine below ~100 Hz; slow beating/phasing; optional chorus/distortion. Unverified.
- Sub bass: sine or triangle, fundamental roughly 30-80 Hz (E1 ~41 Hz to ~C3 range), mono, minimal harmonics. Unverified.
- Hoover (Alpha Juno "What the"): PWM saw with chorus, pitch envelope dip/rise at note-on, portamento. Unverified.
- Juno pad/strings: one DCO saw + sub + PWM, BBD chorus (Juno-60 chorus I/II LFO rates often cited ~0.5 Hz / ~0.8 Hz), slow attack. Unverified.
- Dub techno chord stab (Basic Channel style): minor 7/9 chord, short envelope, LP filter moved over time, tempo-synced dotted-eighth or quarter delay with high feedback and filtered repeats, plate/spring reverb. Unverified numbers.
- House organ (Korg M1 "Organ 2") and M1 piano stab, techno rumble kick (kick -> long reverb -> low-pass ~100-200 Hz, sidechained), FM bells/plucks, wobble bass (LFO on LP cutoff synced to 1/4, 1/8, 1/8T, 1/16), short plucks (decay 50-300 ms, sustain 0), ambient drones/pads (attack seconds, slow LFOs <0.2 Hz), synthwave lead/bass (8th/16th arps, gated), EBM sequenced bass (16th-note mono bass, short decay, Pro-One/SH-101 style), amapiano log drum (pitched, short decaying tone with pitch drop, often from a kick/tom-like synth with glide), industrial noise textures — no citable recipes collected this pass.

## 3. Drum synthesis (808 / 909) and judging a kick

### Takeaway
The 808 kick is a self-decaying bridged-T resonator excited by a trigger pulse (no envelope generators, no noise), drifting slightly flat on long decays; measured "long" 808 samples have a median fundamental of 49.5 Hz (G1) with a ~1-semitone initial downward settle over ~0.4 s. The 909 kick is an oscillator (saw waveshaped to near-sine) with a separate pitch envelope plus a noise+pulse click path. The 909 cymbals/hats are 6-bit samples; the 808 metal sounds come from six square oscillators split into band-passed, separately-enveloped bands.

### Cited Findings
- 808 BD: bridged-T oscillator that decays to silence on its own; no contour generators or noise; the trigger excites the oscillator; trigger+accent pulse added as the "beater" impulse; a low-pass filter tames the click; pitch drifts slightly flat at long decays — [SOS Synth Secrets, Practical Bass Drum Synthesis](https://www.soundonsound.com/techniques/practical-bass-drum-synthesis)
- Roland: the 808 kick is effectively a resonating circuit whose time constant sets both frequency and decay — [MusicRadar](https://www.musicradar.com/tutorials/the-tr-808s-bass-drum-is-undoubtedly-the-most-recognisable-electronic-kick-sound-of-all-time-how-to-get-the-perfect-808s)
- Measured 808 kicks (Trisample library): median fundamental ~49.5 Hz (G1); non-driven presets peak at 49.48 Hz; "short" presets show a secondary peak at 51.05 Hz; driven presets higher. Median initial sweep close to one semitone; initial attack portion ~0.4 s, after which long samples are a single low sine; harmonics appear at the start and fade — [Deruty, arXiv 2502.07524](https://arxiv.org/html/2502.07524v2)
- Transposing an 808 down a fourth (G1 49.5 Hz -> D1 ~37 Hz) loses ~6.3 dB from near-field monitors plus ~5.5 dB of ear sensitivity at 60 phon = ~11.8 dB on a lone fundamental; for a 5-partial complex the loss is ~4.5 dB. Advice: transpose the song up rather than the 808 down; "low E" cited as lowest comfortable note — [Deruty](https://arxiv.org/html/2502.07524v2)
- 909 BD: upper path = sawtooth oscillator with pitch from EG3 (instant attack, slow decay), waveshaped to near-sine, amplitude by EG1 (AR); accent alters EG1 attack/decay. Lower path = noise through LPF mixed with a short pulse (the click), shaped by EG2 — [SOS Practical Bass Drum Synthesis](https://www.soundonsound.com/techniques/practical-bass-drum-synthesis)
- Generic analogue-synth kick recipe (SH-101): resonance max, cutoff min, VCF env ~60%, decay = release (~"6"), all oscillators off (self-oscillating filter ping); sustain 0 so pitch always sweeps down; attack 0 gives the click; decay matched to release so held/unheld sounds the same — [SOS](https://www.soundonsound.com/techniques/practical-bass-drum-synthesis)
- Tutorial value: pitch/filter envelope decay ~200-500 ms for an 808-style kick — [MusicRadar](https://musicradar.com/how-to/how-to-recreate-classic-analogue-drum-sounds-in-your-daw-and-with-hardware)
- Vendor claim (not a measurement): ~33% pitch-sweep depth labeled "authentic 909"; model uses a triangle oscillator with diode clipping, LFSR noise click, RC envelopes — [Rave Generation Knock product page](https://www.muziker.de/rave-generation-knock)
- 808 cymbal: six square-wave oscillators tuned enharmonically, low harmonics removed; split by two band-pass filters into bands with their own VCAs/AR envelopes (low band decay set by Decay knob; upper band split into a shortest-decay path and a mid-decay path); all bands high-passed then mixed by tone control — [SOS Practical Cymbal Synthesis](https://www.soundonsound.com/techniques/practical-cymbal-synthesis)
- 808 cowbell, cymbal and both hi-hats share six rectangular oscillators from one HD14584 hex Schmitt-trigger chip — [Cherry Audio Transistor 808 docs / search summary](https://docs.cherryaudio.com/misfit-audio/transistor-808-hats); oscillator values are in the service manual per [Scrapbox TR-808 note](https://scrapbox.io/0b5vr/TR-808)
- 909 cymbals: 6-bit sample playback, ~30 kHz address clock; tuning changes clock speed; decay tied to the address counter; post-VCA low-pass removes quantisation noise/aliasing — [SOS Practical Cymbal Synthesis](https://www.soundonsound.com/techniques/practical-cymbal-synthesis)

### Inferences
- Kick judging rules a tool can apply: fundamental (pitch-tracked over 50-400 ms) in ~40-60 Hz for 808/techno styles (808 median 49.5 Hz); initial pitch settle for 808-like kicks ~1 semitone over ~0.4 s, while 909-like kicks need a larger fast drop (depth to verify); presence of a broadband click transient in the first few ms; tail length consistent with the style (808 "long" multi-second vs. short techno kicks).
- Penalise kicks pitched below ~E1 (~41 Hz) unless saturated (harmonics carry them), per Deruty's loudness-loss analysis.

### Gaps
- Exact 808 hi-hat/cymbal oscillator frequencies (widely circulated as ~205.3, 304.4, 369.6, 522.7, 540, 800 Hz) NOT confirmed — would need the 808 service manual or Werner et al. ICMC-SMC 2014 cymbal paper ([PDF](https://speech.di.uoa.gr/ICMC-SMC-2014/images/VOL_2/1453.pdf)).
- 808 BD knob tuning range, decay range, and trigger pulse width (in Werner/Abel/Smith [DAFx-14](https://dafx14.fau.de/papers/dafx14_kurt_james_werner_a_physically_informed,_ci.pdf)) not extracted — PDF unreadable to the fetch tool.
- 909 kick measured start/end frequencies and decay (ms), 808/909 snare (two resonant tones + filtered noise) frequencies, clap (multiple noise bursts ~10 ms apart + reverb tail) not sourced this pass.
- "Click 2-5 kHz" for kick attack: not verified with a source.
- Modern techno kick recipes (layered sine + transient + saturation + rumble) not sourced.

## 4. Register / frequency ranges per role

### Takeaway
Only bass-region evidence was gathered: kick/808 fundamentals cluster near 49.5 Hz, and going below ~E1 costs large perceived loudness unless harmonics are present. A full role-by-role register table was not sourced.

### Cited Findings
- 808 kick median fundamental 49.5 Hz (G1); dropping to ~37 Hz loses ~11.8 dB perceived for a pure tone, ~4.5 dB with five partials; "low E" as practical floor — [Deruty](https://arxiv.org/html/2502.07524v2)
- Saturation on 808 bass to make it audible via harmonics — [Attack Magazine](https://www.attackmagazine.com/technique/synth-secrets/808-bass-with-saturation/)

### Inferences
- If an 808/sub bass carries the bassline, the kick and the bass fundamentals will collide in 40-60 Hz; a tool should flag overlapping fundamentals at the same time (e.g. both energetic below 80 Hz on the same beat) and suggest sidechain or tuning the kick to the key.

### Gaps
- Typical roles (sub ~20-80 Hz; bass ~40-250 Hz; mids/chords ~200 Hz-2 kHz; leads ~500 Hz-5 kHz; pads broad 200 Hz-8 kHz; hats/cymbals ~6-16 kHz) are common mixing-guide values, not verified with a source here.

## 5. FM synthesis tips (ratios, index)

### Takeaway
No citable FM sources were gathered in this pass.

### Cited Findings
- None collected.

### Inferences
- None that can be grounded in this pass's sources.

### Gaps (values to verify; commonly cited)
- Integer C:M ratios (1:1, 1:2) give harmonic spectra (basses, e-pianos); non-integer ratios (e.g. 1:1.4, 1:3.5) give inharmonic bell/metal spectra; modulation index governs brightness, and a decaying index envelope gives the classic "bright attack, mellowing decay" of plucks/bells/E.PIANO. DX7 E.PIANO 1 and TUB BELLS operator ratios/levels should be taken from the actual factory patch data (sysex) — not fetched. Chowning's 1973 FM paper and SOS Synth Secrets FM parts are the right primary sources.

## 6. Measurable checks a tool can run on a rendered sound

### Takeaway
The sources support a handful of checks directly (pitch tracking of kick fundamental and sweep, harmonic decay vs. fundamental, accent-driven centroid behaviour, constant-time glide, supersaw peak spread). Others are standard DSP descriptors that need thresholds calibrated from reference renders.

### Cited Findings
- Deruty's methodology: partial tracking with thresholds at 0.7 and 0.5 of peak-bin energy; fundamental estimated on 0.2 s windows — a usable template for kick pitch checks — [Deruty](https://arxiv.org/html/2502.07524v2)
- Behaviours that are measurable per sources: 303 cumulative accent sweep and fixed accent decay ([Whittle](https://www.firstpr.com.au/rwi/dfish/303-unique.html)); constant-time exponential slide ([synth-diy](https://synth-diy.org/pipermail/synth-diy/1998-April/010281.html)); 808 slight flat drift on long decay ([SOS](https://www.soundonsound.com/techniques/practical-bass-drum-synthesis)); ~1 semitone settle over ~0.4 s ([Deruty](https://arxiv.org/html/2502.07524v2)); supersaw non-linear detune and HPF ([KVR](https://www.kvraudio.com/forum/viewtopic.php?p=5429065)).

### Inferences
- Proposed check set: attack time (10%-90% of envelope peak); decay time (T60 or time to -20/-40 dB); spectral centroid trajectory (filter-env behaviour, accent detection); f0 track (pitch accuracy in cents vs. intended note; glide time; kick sweep depth/duration); detune spread (peak-splitting around each harmonic or amplitude-beat rate between two saws: beat Hz = f*(2^(cents/1200)-1)); resonance peak (prominence of a spectral peak near the cutoff vs. neighbouring harmonics, tracked over time); low-band energy ratios per role to detect masking; transient click energy (high-band energy in first ~5 ms).

### Gaps
- No published thresholds found for "authentic" ranges of these descriptors; thresholds must be derived by rendering reference patches/samples.
