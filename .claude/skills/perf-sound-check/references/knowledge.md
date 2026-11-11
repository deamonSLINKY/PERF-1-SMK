# Measure PERF-1 against club masters, not textbooks

A sound check for PERF-1 needs genre-specific reference distributions, not one universal mix target. Released techno and house are mastered at a median of about **-7 to -9 LUFS integrated, with true peaks above 0 dBTP**. **78-92% of their energy sits below 250 Hz**, and the sub band is close to perfectly mono (correlation 0.998-1.0). Their harmony is about **85% minor** and Aeolian-first, built from 2-4 bar loops without cadences. Every one of these figures contradicts a generic "pop mix" rule set, so the tool would wrongly fail good techno if it used one. The firmware-specific checks with real source backing are: BS.1770/R128 loudness, PSR ≥ 8 dB in loud sections, Roger Linn's swing scale (50% straight, 54% loose, 66% triplet), Ableton's genre tempo bands, the TB-303's cumulative accent sweep and constant-time slide, and measured 808 kick behaviour (49.5 Hz median, about a 1-semitone settle over about 0.4 s). Most other thresholds a checker needs are not published: chord-tone ratios, swing per genre, mud and harshness margins, sidechain depth, section bar counts and everything spectral for dubstep, ambient, lo-fi, synthwave and industrial. They are practitioner conventions and must be calibrated on reference renders. This report tags them **[S] sourced** or **[H] heuristic, calibrate**. The tooling is simple and licence-clean: numpy, scipy, soundfile, pyloudnorm, librosa, mido, pretty_midi, music21 and matplotlib are all permissive and compatible with GPL-3.0-only. Essentia (AGPL) and madmom's models (CC BY-NC-SA) must stay optional, and ffmpeg is run as a subprocess for cross-checks.

## Loudness: club masters sit 5-7 LU above streaming targets

The measurement definitions are exact and should be implemented to the letter. In EBU R128 / BS.1770, momentary loudness uses a 400 ms window and short-term a 3 s window, both ungated. Integrated loudness is gated, first at -70 LUFS absolute and then **10 LU below** the ungated integrated value. True peak needs oversampling to catch inter-sample peaks ([Wikipedia: EBU R 128](https://en.wikipedia.org/wiki/EBU_R_128)). Loudness range (LRA) is the 10th-to-95th percentile spread of short-term loudness. It uses a **-20 LU** relative gate, not the integrated gate's -10 LU, and windows must step at ≥10 Hz ([EBU Tech 3342](https://tech.ebu.ch/docs/tech/tech3342.pdf)). Confusing the two gates is the most likely implementation bug. True peak per BS.1770-4 Annex 2 means 4× oversampling through a 48-tap polyphase FIR ([ebur128-stream](https://docs.rs/crate/ebur128-stream/0.2.0/source/src/peak.rs)). R128's broadcast target is -23 LUFS ±0.5 LU, with a -1 dBTP ceiling ([Wikipedia: EBU R 128](https://en.wikipedia.org/wiki/EBU_R_128)).

Streaming services play tracks back at fixed levels, but those are playback references, not mastering targets. Spotify normalizes to **-14 LUFS** and asks for **≤ -1 dBTP**, or **≤ -2 dBTP if the master is louder than -14 LUFS**, because lossy encoding causes overs ([Spotify for Artists](https://support.spotify.com/artists/article/loudness-normalization/)). Apple Music is usually quoted at about -16 LUFS, but one review notes that Apple does not publish the figure ([Fora Soft](https://www.forasoft.com/learn/audio-for-video/articles-audio/lufs-targets-per-platform-2026); [Orphiq](https://orphiq.com/resources/mastering-for-streaming)). YouTube turns loud tracks down to about -14 but never boosts quiet ones ([RoEx](https://www.roexaudio.com/blog/what-lufs-should-i-master-at)).

Real club masters ignore those numbers. A 563-track vendor corpus of techno and house measured these medians ([TrackSensei](https://tracksensei.com/blog/we-analyzed-563-techno-house-tracks)):

| Style | Median integrated loudness | P25-P75 range |
|---|---|---|
| Hard techno | **-6.9 LUFS** | -8.0 to -6.4 |
| Peak-time techno | -7.7 | not given |
| Groove techno | -7.7 | not given |
| House | **-8.0** | -9.0 to -7.1 |
| Tech house | -8.8 | not given |
| Melodic techno | -8.8 | -9.9 to -8.0 |

Median true peak runs from **+0.47 to +1.22 dBTP** across these styles, so the Spotify rule fails almost every commercial club master. A smaller 151-master study puts "EDM" at -11.0 LUFS and "electronic" at -10.2. Its authors call the results only directional, and the 3-4 LU gap from TrackSensei reflects different samples and subgenres ([LuvLang](https://luvlang.studio/research/loudness-2026)). Drum and bass has no single target: 101 measured tracks spread across more than 12 LU ([WeFoundDnB](https://wefounddnb.com/dnblog/how-loud-should-drum-bass-be-lufs-from-the-pros-and-101-measured-tracks)). One trap: TrackSensei's mono-sum scale reads about 3.3 LU lower than stereo BS.1770, so every reported figure must state its channel mode ([TrackSensei](https://tracksensei.com/blog/how-loud-should-your-master-be)).

Because absolute level depends on playback gain, dynamics rules should use ratios that don't. **PSR** is true peak minus short-term loudness, and **PLR** is true peak minus integrated loudness ([MeterPlugs](https://www.meterplugs.com/blog/2017/05/18/crest-factor-psr-and-plr.html)). Ian Shepherd's floor is **PSR ≥ 8 dB in the loudest sections, in any genre**, below which masters "will often sound crushed" ([MeterPlugs](https://www.meterplugs.com/blog/2017/05/18/crest-factor-psr-and-plr.html)). For reference points, Sound On Sound cites a dynamic jazz track at PLR about 20, and a modern loud track at PLR 9 with minimum PSR 6 ([SOS](https://www.soundonsound.com/reviews/meterplugs-dynameter)). The TrackSensei medians imply a PLR of about **8.6 dB** for current techno and about 7.7 for hard techno. That supports bands of PLR < 7 = over-limited, 8-10 = club-competitive, 10-14 = streaming-friendly and > 14 = ambient or very dynamic **[H]**. The tool should therefore report loudness against three profiles rather than fail one number: club (-9 to -7 LUFS), streaming (-14 / -16) and broadcast (-23). It should also print "dB of Spotify turn-down" = max(0, I + 14). No sourced loudness corpus exists for dubstep, ambient, lo-fi, synthwave or industrial. Their targets have to come from PERF-1's own calibrated reference set.

## Low end and stereo: almost all the energy is mono bass

Commercial mixes share a downward-tilting long-term spectrum ([Pestana et al., AES 2013](https://joshreiss.github.io/documents/2013/Pestana%20Ma%20Reiss%20-%20AES135.pdf)). iZotope's Tonal Balance Control turns that idea into statistical ranges over four broad bands (20-250 Hz, 250 Hz-2 kHz, 2-8 kHz, 8-20 kHz), built from thousands of tracks. Its genre curves differ mostly in how wide each allowed range is ([iZotope TBC docs](https://downloads.izotope.com/docs/tonal-balance-control/meters-and-target-curves/index.html)). The only measured techno and house band shares come from the same 563-track corpus, as Bark-weighted median energy shares ([TrackSensei](https://tracksensei.com/blog/we-analyzed-563-techno-house-tracks)):

| Style | Sub 20-60 Hz | Bass 60-250 Hz | Above 250 Hz (remainder) | Median BPM |
|---|---|---|---|---|
| House | 34.8% | 43.1% | about 22% | 126 |
| Tech house | 34.9% | 44.9% | about 20% | not given |
| Groove techno | 39.9% | 51.9% | about 8% | 140 |
| Melodic techno | 46.6% | 42.0% | about 11% | not given |
| Peak-time techno | 43.0% | 44.7% | about 12% | 131 |
| Hard techno | 40.2% | 48.0% | about 12% | 141 |

Kick fundamentals cluster at **48-62 Hz**, with medians of 51-54 Hz in house, melodic and peak-time techno and 56.5 Hz in groove techno. Median sub-band stereo correlation is **0.998-1.0** in every style, and mono-sum loudness loss is only **0.18-0.46 LU**. That is well inside the common 1 LU rule of thumb. Released techno is also narrow, with a width index of 0.16-0.24 on TrackSensei's own (unpublished) 0-1 scale ([TrackSensei](https://tracksensei.com/blog/we-analyzed-563-techno-house-tracks)).

These figures translate into firm checks for the 4/4 club presets. Sub + bass should be about 75-95% of energy, and the sub-to-bass ratio about 0.6-1.2 (observed range about 0.67-1.11). Correlation below about 120 Hz should be ≥ 0.97 to pass and ≥ 0.9 to avoid failing, and mono-sum loss should stay ≤ 1 LU. These thresholds are proposed from the data **[H]**. Bass should be mono because ears localise it poorly, and many producers mono everything below about 100 Hz ([MasteringBOX](https://www.masteringbox.com/mixing-bass-3-steps-perfect-low-end/)). The exact crossover (100 vs. 150 Hz) and sidechain depth or timing have no authoritative source. A starting point is ducking of 3-10 dB that recovers within 1/8-1/4 beat, about 115-230 ms at 130 BPM **[H]**, and the tool should report it as a value rather than a fault.

The spectral analysis must be unweighted. Loudness uses K-weighting, whose roughly 38 Hz high-pass makes sub-bass barely count toward LUFS, so a K-weighted spectrum would hide exactly the band techno is built on **[H, verify against BS.1770]**. Mud (200-500 Hz) and harshness (2-5 kHz, where hearing is most sensitive) are well-known problem bands ([MixDiagnose](https://dev.to/oren_mixdiagnose_1380fb19/stop-guessing-if-your-mix-is-ready-for-mastering-19cd); [Baby Audio](https://babyaud.io/blog/fix-a-harsh-mix)). They should be detected as deviations of more than 2-3 dB from the genre reference's P75, not as absolute levels **[H]**. Acid has one specific trap. A *static* resonant spectral peak that persists in more than 50% of frames is a fault, but a *sweeping* resonance is the music. A naive resonance detector will flag every good 303 line **[H]**.

## Harmony: Aeolian loops, the b2, and permission to be ambiguous

The biggest corpus finding is that EDM is the mirror image of pop. In Faraldo's hand-annotated datasets, **85.1% (KeyFinder) and 84.6% (GiantSteps) of EDM tracks are minor**, while Billboard/Beatles/rock corpora are about 84% major ([Faraldo 2017](https://zenodo.org/records/1154586/files/Faraldo_PhDThesis.pdf)). Among heptatonic GiantSteps+ tracks, the modes break down as **Aeolian 122, Dorian 22, Ionian 18, Phrygian 9, Mixolydian 5**. The leading tone barely appears in minor, where Aeolian bVII and Phrygian bII are preferred ([Faraldo et al., ECIR 2016](https://www.researchgate.net/publication/309018542_Key_Estimation_in_Electronic_Dance_Music)). The **b2-to-1 relation appears in 86 tracks**, "especially in genres such as tech house and techno," and tritone relations in 53, mostly drum and bass and tech house ([Faraldo 2017](https://zenodo.org/records/1154586/files/Faraldo_PhDThesis.pdf)).

Texture statistics define what "normal" means:

| Feature in GiantSteps+ | Prevalence | Implication for the checker |
|---|---|---|
| Chord sequences, riffs, pedal tones, arpeggios | Each under 40% of tracks | None of them is mandatory |
| Lead melody | 60% (about half of those vocal) | A missing lead is not a failure |
| Drumless (pitch-only) section | Over 40% | Breakdowns are normal |
| Key change | About 5% | One modulation is plenty |

Of 1,986 excerpts, **3.5% had structural key changes**. They happened suddenly at a hypermeter boundary or after an unpitched transition, usually to a relative, fifth-related or parallel key ([Faraldo 2017](https://zenodo.org/records/1154586/files/Faraldo_PhDThesis.pdf)). House loops are "normally" 2 or 4 bars with about one chord per bar and "no cadential points". Two-chord shuttles leave the tonic ambiguous, so the bass and beat layers fix it instead ([Faraldo et al., TENOR 2017](https://www.tenor-conference.org/proceedings/2017/17_Faraldo_tenor2017.pdf)). Deep house draws extended chords from soul and jazz, such as min9 cores and Fm7-Abmaj7-Cm7 strings. The bass note decides whether Cm7 is heard as Cm7 or Eb6 ([Attack Magazine](https://www.attackmagazine.com/technique/passing-notes/passing-notes-deep-house-chords/); [MusicRadar](https://musicradar.com/how-to/how-to-create-deep-house-minor-ninth-chords)). EDM tonics are also spread more evenly across all 12 pitch classes than guitar pop's, so the generator must handle keys like D#m and Bbm and spell them correctly ([Faraldo 2017](https://zenodo.org/records/1154586/files/Faraldo_PhDThesis.pdf)).

The only published transition statistics come from rock and pop, which is major-mode material, so they need rotating to the relative minor before use. In de Clercq & Temperley's corpus the roots are I .328, IV .226, V .163, bVII .081 and VI .072, and those five make up 87% of chords. Root motion by fourth is almost symmetric (+P4 2,266 vs. -P4 2,220), and 94.1% of chords are in root position ([de Clercq & Temperley 2011](https://rockcorpus.midside.com/2011_paper/declercq_temperley_2011.pdf)). Hooktheory's data gives P(I | IV) = .332 and P(V | IV) = .266. Its chord encoding (`b7`, `5/6`, with mode as a separate parameter) makes a good internal symbol format ([Hooktheory API](https://hooktheory.com/api/trends/docs)). No EDM chord-transition matrix exists. Per-genre presets are theory-consistent but not corpus-verified **[H]**:

| Preset | Harmony |
|---|---|
| Techno | i drone or i-bII / i-bVII shuttle |
| Dub techno | A single m7/m9 stab |
| Deep house | m7/m9/maj7 two-chord shuttles in Dorian/Aeolian |
| Trance | i-bVI-bIII-bVII, with optional harmonic-minor V |
| Lo-fi | ii7-V7-Imaj7 with 9ths |
| Ambient | Lydian/Dorian drones |

Key detection should correlate a duration-weighted pitch-class histogram against 24 rotated profiles. The Krumhansl-Kessler values are major 6.35, 2.23, 3.48, 2.33, 4.38, 4.09, 2.52, 5.19, 2.39, 3.66, 2.29, 2.88 and minor 6.33, 2.68, 3.52, 5.38, 2.60, 3.53, 2.54, 4.75, 3.98, 2.69, 3.34, 3.17 ([partitura](https://partitura.readthedocs.io/en/latest/_modules/partitura/musicanalysis/key_identification.html)). For EDM presets, use Faraldo's EDM-derived profiles instead. The minor profile is 1.00, 0.31, 0.44, 0.58, 0.33, 0.49, 0.29, 0.78, 0.43, 0.29, 0.53, 0.32, with b7 (0.53) well above the natural 7 (0.32). These EDM profiles mislabel major pop, so choose the profile by genre ([Faraldo 2017](https://zenodo.org/records/1154586/files/Faraldo_PhDThesis.pdf)). Accuracy is capped: the best commercial tool scored only about **70%** on the KeyFinder EDM set ([Faraldo 2017](https://zenodo.org/records/1154586/files/Faraldo_PhDThesis.pdf)). The checker should accept a relative, parallel or fifth-neighbour result, and should return "ambiguous" when the top two correlations are within about 0.05 **[H]**. Because PERF-1 emits MIDI, key, scale and chord checks should run on the MIDI as ground truth, with audio chroma only as confirmation.

The melodic and voice-leading rules backed by research are few. Large leaps tend to be followed by reversals, but von Hippel & Huron explain this as regression toward the middle of the range, not as a rule about leaps ([abstract](https://scilit.com/publications/d61da2e28fb69e0f7a57e22fd7fd1969)). EDM form is accumulative layering of short motifs rather than a climactic theme ([Faraldo 2017](https://zenodo.org/records/1154586/files/Faraldo_PhDThesis.pdf)). Parallel fifths and octaves are only errors between independent lines. Chord-memory planing and power stabs are genre norms, so whitelist them in stab mode ([Intermediate Theory](https://intmus.github.io/inttheory20-21/10-intro-part-writing/b1-part-writing-errors.html)). The numeric melody targets are all heuristics to calibrate **[H]**:

| Target | Starting value |
|---|---|
| Steps (0-2 semitones) | ≥ 60-70% of intervals |
| Leaps over a fifth | ≤ 10% of intervals |
| Strong-beat chord tones | ≥ 0.7 |
| Bass = chord root on downbeats | ≥ 0.8 |
| Scale conformance (diatonic / acid and lo-fi) | ≥ 0.95 / ≥ 0.85 |
| Bar repetition ratio | 0.5-0.9 |

## Rhythm and arrangement: tempo, one displaced hit, and 8-bar phrases

Swing should be encoded the Roger Linn / MPC way. Swing is the ratio of the first to the second 16th within each 8th: **50% = straight, about 54% loosens the feel "without it sounding like swing," and 66% = perfect triplet swing**. Linn's 48-PPQN steps are 50, 54, 58, 60, 62, 66, 70 and 75%, and he "rarely need[s]" more ([Attack Magazine / Linn](https://attackmagazine.com/features/interview/roger-linn-swing-groove-magic-mpc-timing/2)). Only the even 16ths are delayed, by (s − 0.5) × 2 × T₁₆. That is 8% of a 16th at 54% and 32% at 66%, consistent with the Renoise tick table ([Renoise forum](https://forum.renoise.com/t/swing-variations-of-50-54-58-60-62-66-70-and-75-as-commands/48546)). Percentages are not standardised across devices at high resolution ([synth-diy](https://synth-diy.org/yg-archives2/group/xl7/message/5869/)). The checker should therefore measure swing from MIDI tick positions, not trust a parameter label.

Tempo bands come from Ableton ([Learning Music](https://learningmusic.ableton.com/make-beats/tempo-and-genre.html)):

| Genre | Ableton band (BPM) |
|---|---|
| Dub | 60-90 |
| Hip-hop | 60-100 |
| House | 115-130 |
| Techno / trance | 120-140 |
| Dubstep | 135-145 |
| Drum and bass | 160-180 |

Other sources widen these. Techno runs 125-140, hard techno 180+, and trance 135-145 ([Digital DJ Pool](https://digitaldjpool.com/blog/dj-bpm-guide/)). Dub techno sits at **118-128**, unlike dub's 60-90 ([mixgraph](https://www.mixgraph.io/tools/edm-genre-chart)). The cited structural hits are:

| Genre | Signature hit |
|---|---|
| Drum and bass two-step (about 174 BPM) | Kick on step 1 and step 11, snares on 2 and 4 ([MusicRadar](https://www.musicradar.com/how-to/how-to-program-6-different-jungle-and-drum-n-bass-grooves)) |
| Half-time DnB | Snare on beat 3 at 172-174, feels like 86-88 ([violetrecording](https://violetrecording.com/how-to-make-drum-and-bass/)) |
| UK garage | Kick on steps 1 and 11 with 8-12% DAW swing ([MusicRadar](https://www.musicradar.com/how-to/uk-garage-tutorial)) |
| Jungle | Amen break identified by its displaced late-kick bar ([MusicRadar](https://musicradar.com/tuition/tech/how-to-program-an-amen-style-break-637374)) |
| Amapiano (110-115 BPM) | Constant 16th shaker backbone, key-tuned log drum carrying the same swing ([Roland](https://articles.roland.com/?p=67237); [beatstorapon](https://beatstorapon.com/blog/how-to-make-amapiano-beat/)) |

The grids for PERF-1's main genres are practitioner conventions (16 steps per bar, step 1 = downbeat), meant as defaults to audition, not authority **[C/H]**. Where a row has a source, it is linked:

| Genre | BPM | Swing | Kick | Snare/clap | Signature cue |
|---|---|---|---|---|---|
| Techno (Berlin/peak) | 124-140 | 50-54% | `x...x...x...x...` | sparse clap/rim 2 & 4 | offbeat OH `..o...o...o...o.`, one layer change per 8-16 bars |
| Hard / industrial techno | 130-160+ | 50% | four-on-floor, pickups `x...x...x...x.xx` | metallic hits | rumble (kick→reverb→distortion→LPF), distortion |
| Acid | 120-135 | 50-58% | four-on-floor | clap 2 & 4 | 303 16th line with accent/slide and filter movement |
| House / deep | 115-128 / 118-124 | 54-62% | four-on-floor | clap or rim 2 & 4 | swung 16th shaker, rim/conga ghosts |
| Dub techno | 118-128 ([mixgraph](https://www.mixgraph.io/tools/edm-genre-chart)) | 50-54% | soft four-on-floor | sparse rim | offbeat m7 stab `..x...x...x...x.` into high-feedback delay |
| Dub | 60-90 ([Ableton](https://learningmusic.ableton.com/make-beats/tempo-and-genre.html)) | 58-62% | one drop: kick+snare on beat 3 | rim on 3 | skank offbeats, echo throws, mutes every 4-8 bars |
| Drum and bass | 170-176 | 50% | `x.........x.....` ([MusicRadar](https://www.musicradar.com/how-to/how-to-program-6-different-jungle-and-drum-n-bass-grooves)) | `....x.......x...` | ghost snares at velocity 25-45, Reese/sub |
| Breaks | 125-140 | 54-58% | `x.....x...x.....` | 2 & 4 plus ghosts | sampled-break feel, syncopated kick |
| Dubstep | 138-142 | 50% | `x.........x.....` | beat 3 only `........x.......` | wobble LFO on 1/8 or 1/8T, sub drops |
| Lo-fi hip-hop | 70-90 | 58-66% | `x......x..x.....` | soft, 5-20 ms late | low velocities, crackle, Rhodes 7th/9th chords |
| Trip-hop | 70-95 | 52-58% | `x.......x.x.....` | 2 & 4 or half-time | slow heavy breaks, dub FX |
| Synthwave | 80-118 | 50% | `x.......x.......` or four-on-floor | gated-reverb snare 2 & 4 | 16th arpeggiated bass |
| EBM | 110-130 | 50% | four-on-floor | 2 & 4 | 16th sequenced mono bass `xxxxxxxxxxxxxxxx` |
| Ambient | free / 60-90 | n/a | none or `x...............` | none | slow 32-64 bar layer fades |
| Trance / psytrance | 135-145 / 138-145 ([Digital DJ Pool](https://digitaldjpool.com/blog/dj-bpm-guide/); [psytranceguide](https://psytranceguide.com)) | 50% | four-on-floor | 2 & 4 | offbeat bass / "KBBB" bass `.xxx.xxx.xxx.xxx` |

Bass rules follow the drums. Offbeat bass must not land on the kick and must be cut before the next kick transient. 1/16 rolling bass makes sidechain "mandatory" ([Producer School](https://theproducerschool.com/blogs/featured-blogs/5-essential-bass-patterns-that-define-hard-house-and-trance-music)). Psytrance bass is "usually a single note most of the way through" ([Gearspace](https://gearspace.com/threads/303-psytrance-style-basslines.980708/)).

Arrangement runs on the 8-bar unit. Most dance music is phrased in 8-bar blocks grouped into 16- and 32-bar phrases ([DJ.Studio](https://dj.studio/blog/phrasing-dj-mixing)), and "most DJs need 16 or 32 bars of simple elements" at the start and end ([Lost Stories Academy](https://loststoriesacademy.com/blogs-and-tutorials/what-makes-a-track-dj-friendly)). Builds double the snare roll "until the end" while velocity, filter and pitch rise ([WA Production](https://blog.waproduction.com/crafting-the-perfect-edm-build-up-techniques-and-tips)). Risers are filtered white noise or a resonant high-pass sweep over 4-8 measures ([MusicRadar](https://www.musicradar.com/how-to/synth-sweep-fx)). An encodable 8-bar build runs 1/4 → 1/8 → 1/16 → 1/32 every 2 bars, with velocity 60 → 127, a drum HPF from 20 Hz to 300-500 Hz and a +12-semitone riser **[H]**. Pop-trained segmenters handle EDM poorly ([EDMFormer, arXiv 2603.08759](https://arxiv.org/abs/2603.08759)). Since PERF-1 knows its own arrangement plan, the checker should verify the plan against the audio rather than infer structure blind.

## Sound design: the 303's accent memory and the 808's half-second settle

The 303's character lives in its accent circuit. On an accented note the Decay pot is shorted out, so the filter envelope always takes its *minimum* decay, whatever the knob says. The accent signal also feeds the VCA through a 47k/0.033 µF network, which is why accents are louder. It reaches the filter through an "accent sweep" circuit: a diode plus 147k charging a **1 µF capacitor**, scaled by a second gang on the Resonance pot. On consecutive accents the cap hasn't discharged, so **each sweep peaks higher**, and more so at high resonance ([Robin Whittle](https://www.firstpr.com.au/rwi/dfish/303-unique.html)). Env Mod cannot reach zero on the original ([Whittle](https://www.firstpr.com.au/rwi/dfish/303-unique.html)). Slide is an RC lag with **fixed time, not fixed rate**: C1 to C3 takes as long as C1 to C#1 ([synth-diy 1998](https://synth-diy.org/pipermail/synth-diy/1998-April/010281.html)). Roland itself describes accent as boosting both volume and filter cutoff ([Roland](https://articles.roland.com/beyond-acid-pushing-the-tb-303-into-new-sonic-territory/)).

Some figures are less settled. The 60 ms slide time often quoted is a clone default (nt_303, range 1-200 ms), not a measurement of the hardware ([nt_303](https://github.com/thorinside/nt_303)). Sources also disagree on the filter: some say 18 dB/oct, others a 4-pole diode ladder with extra post-filter high-passing ([Polynominal](https://www.polynominal.com/m/roland-tb303.htm); [a1k0n/303](https://github.com/a1k0n/303)). A single forum source claims the filter does not self-oscillate. Programming advice is to fill all 16 steps, scatter slides and accents and then prune them. Shortening the pattern to 13 or 15 steps gives the "woozy" phasing against the bar ([MusicTech](https://musictech.com/guides/essential-guide/how-to-create-a-chicago-style-acid-house-bassline)). Emulators may read overlapping notes as slides, so PERF-1 must control note overlap explicitly in its MIDI ([KVR](https://www.kvraudio.com/forum/viewtopic.php?p=8644925)).

Four 303 behaviours are directly testable:

| Test | What a correct engine shows |
|---|---|
| Run of consecutive accents | Peak spectral centroid rises monotonically across the run |
| Glide timing | Pitch-tracked glide takes the same time for a semitone and for two octaves |
| Accent decay | Accented notes show a shorter centroid decay than unaccented ones at the same Decay setting |
| Slid notes | No envelope retrigger |

The densities are not sourced and need calibrating **[H]**: octave-up notes on about 20-35% of steps, accents about 25%, slides 15-25%.

The 808 and 909 kicks differ in kind. The 808 bass drum is a bridged-T resonator that a trigger pulse "pings". It has no envelope generators or noise, and it drifts slightly flat on long decays ([SOS Synth Secrets](https://www.soundonsound.com/techniques/practical-bass-drum-synthesis)). Measured 808 samples have a **median fundamental of 49.5 Hz (G1)** and an initial downward settle of **about one semitone over about 0.4 s**, after which long samples are a near-pure sine. Driven presets keep their harmonics. Transposing an 808 from G1 down to D1 costs about **11.8 dB** of perceived loudness as a pure tone but only about 4.5 dB with five partials, and "low E" is the practical floor ([Deruty, arXiv 2502.07524](https://arxiv.org/html/2502.07524v2)). The 909 kick is a sawtooth waveshaped to near-sine, with its own pitch envelope. Its click is a separate noise-plus-pulse path, and accent changes the amplitude envelope ([SOS](https://www.soundonsound.com/techniques/practical-bass-drum-synthesis)). The 909's cymbals are **6-bit samples clocked at about 30 kHz**, with a post-VCA low-pass removing quantisation noise. The 808's metal voices come from six square oscillators on one chip, split into band-passed, separately enveloped bands ([SOS Practical Cymbal Synthesis](https://www.soundonsound.com/techniques/practical-cymbal-synthesis)).

Most other patch numbers did not survive verification. Only the JP-8000 supersaw has a technical trail: 7 saws, a non-linear detune curve, a mix control that shrinks the centre saw relative to the side saws at maximum, aliasing as part of the character, and a high-pass below the fundamental ([KVR](https://www.kvraudio.com/forum/viewtopic.php?p=5429065); [schollz](https://schollz.com/til/221103)). Reese, hoover, Juno pad, dub stab, wobble and FM ratio recipes all remain unverified conventions. The checker should judge those patches against reference renders, not against fixed descriptor values.

## The sound-check rule table

The table encodes each rule. "Club" means the 4/4 techno and house profile, and other genres inherit thresholds only after calibration. Where there is no published threshold, **heuristic, calibrate** means: render 20-50 reference passes per genre, take median ± IQR, and set warn at P10/P90 and fail at P2/P98.

| # | Metric | How to measure | Pass | Warn | Source |
|---|---|---|---|---|---|
| 1 | Render health | NaN, Inf and denormal counts (ffmpeg astats or numpy) | 0 | any = fail | Heuristic (zero tolerance) |
| 2 | Sample clipping | Runs of ≥ 3 samples at abs(x) ≥ 0.999 | 0 runs | ≥ 1 run | Heuristic, calibrate |
| 3 | True peak | 4× `resample_poly` max, cross-checked with ffmpeg `ebur128=peak=true` | ≤ -1.0 dBTP (streaming); ≤ +1.2 (club) | > -1 streaming; > -2 if I > -14 | [Spotify](https://support.spotify.com/artists/article/loudness-normalization/); club range [TrackSensei](https://tracksensei.com/blog/we-analyzed-563-techno-house-tracks) |
| 4 | Integrated loudness | pyloudnorm BS.1770-4, stereo, state channel mode | Club -9.9 to -6.4 LUFS; streaming -14 ±1 | outside profile; report Spotify turn-down | [TrackSensei](https://tracksensei.com/blog/we-analyzed-563-techno-house-tracks); [Spotify](https://support.spotify.com/artists/article/loudness-normalization/) |
| 5 | PSR (loud sections) | TP − short-term LUFS, P10 over frames within 3 LU of max S | ≥ 8 dB | 7-8 warn; < 7 fail | [Shepherd via MeterPlugs](https://www.meterplugs.com/blog/2017/05/18/crest-factor-psr-and-plr.html) |
| 6 | PLR | TP − integrated | 8-10 club; 10-14 streaming | < 7 | Heuristic from TrackSensei medians, calibrate |
| 7 | LRA | EBU Tech 3342 (-20 LU relative gate, P10-P95 of S) | report | < 3 LU = "no dynamic arc" | Definition [EBU](https://tech.ebu.ch/docs/tech/tech3342.pdf); threshold heuristic |
| 8 | Sub + bass share | Welch PSD, unweighted, energy share < 250 Hz | 75-95% (club) | outside | Heuristic from [TrackSensei](https://tracksensei.com/blog/we-analyzed-563-techno-house-tracks) |
| 9 | Sub/bass ratio | Energy 20-60 Hz ÷ 60-250 Hz | 0.6-1.2 | outside | Heuristic from TrackSensei (0.67-1.11 observed) |
| 10 | Kick fundamental | f0 track of kick hits, 50-400 ms window | 45-65 Hz (club) | < 41 Hz (E1) unless harmonics present | [TrackSensei](https://tracksensei.com/blog/we-analyzed-563-techno-house-tracks); [Deruty](https://arxiv.org/html/2502.07524v2) |
| 11 | 808 pitch settle | Pitch track over first 0.5 s | about 1 semitone over about 0.4 s | > 3 semitones or none | [Deruty](https://arxiv.org/html/2502.07524v2); tolerance heuristic |
| 12 | Sub mono | Pearson L/R correlation, band-pass 20-120 Hz, 100-400 ms windows, median | ≥ 0.97 | 0.9-0.97; < 0.9 fail | Heuristic; reference 0.998 [TrackSensei](https://tracksensei.com/blog/we-analyzed-563-techno-house-tracks) |
| 13 | Mono-sum loss | LUFS(mono sum) vs. stereo LUFS | ≤ 0.5 LU | > 1.0 LU | [TrackSensei](https://tracksensei.com/blog/we-analyzed-563-techno-house-tracks) |
| 14 | Full-band correlation | Median and P5 over 400 ms windows | median ≥ 0.3 | < 0.3; < 0 fail | Heuristic, calibrate |
| 15 | Width by band | S/M energy ratio per band | rises with frequency | bass wider than highs | Heuristic |
| 16 | DC offset | Mean per channel, per 1 s window | < -60 dBFS | ≥ -60 dBFS | Heuristic |
| 17 | Infrasonic energy | Energy share < 20-25 Hz | negligible | above calibrated P90 | Heuristic, calibrate |
| 18 | Spectral tilt | Regression of 1/3-octave dB vs. log2 f, 100 Hz-10 kHz | within ±1.5 dB/oct of genre reference | outside | Heuristic, calibrate |
| 19 | Mud / harshness | 200-500 Hz and 2-5 kHz relative to neighbours vs. reference P75 | ≤ +2 dB | > +2-3 dB | Bands [MixDiagnose](https://dev.to/oren_mixdiagnose_1380fb19/stop-guessing-if-your-mix-is-ready-for-mastering-19cd), [Baby Audio](https://babyaud.io/blog/fix-a-harsh-mix); margin heuristic |
| 20 | Static resonance | Peaks > 6 dB prominence, 1/12-octave, in > 50% of frames | none static | static peak | Heuristic (sweeping peaks allowed in acid) |
| 21 | Sidechain dip | 40-120 Hz envelope of non-kick part around kick onsets | 3-10 dB, recovery 1/8-1/4 beat (report) | none in rolling-bass mode | Heuristic, verify |
| 22 | Clicks / buffer bugs | Robust z of 2nd difference vs. local MAD; clustering mod block size | none | z ≥ 8-10, or clustering at 32/64/128 | Heuristic, calibrate |
| 23 | Noise floor / stuck voices | P5 of 50 ms RMS; tail after last note-off | digital zero on silent render | > -60 dBFS | Heuristic |
| 24 | Tempo | MIDI clock and onset grid vs. preset BPM | in genre band; error < 100 ppm | outside band | Bands [Ableton](https://learningmusic.ableton.com/make-beats/tempo-and-genre.html); ppm heuristic |
| 25 | Swing | Even-16th tick offset → Linn % | within preset ±2% | outside | Scale [Linn/Attack](https://attackmagazine.com/features/interview/roger-linn-swing-groove-magic-mpc-timing/2); per-genre % heuristic |
| 26 | Audio-vs-MIDI timing | Cross-correlate onset envelope with MIDI impulse train | latency stable, jitter SD < 1 ms | drift > 0 | Heuristic |
| 27 | Signature hit present | 16-step mask match per genre (e.g. DnB step 11, dubstep beat-3 snare) | present in ≥ 75% of groove bars | absent | Hits [MusicRadar](https://www.musicradar.com/how-to/how-to-program-6-different-jungle-and-drum-n-bass-grooves), [violetrecording](https://violetrecording.com/how-to-make-drum-and-bass/); ratio heuristic |
| 28 | Bass/kick collision | Bass onsets coinciding with kick onsets | 0 in offbeat mode | > 0 | [Producer School](https://theproducerschool.com/blogs/featured-blogs/5-essential-bass-patterns-that-define-hard-house-and-trance-music) |
| 29 | Key/mode | Duration-weighted pc histogram vs. KK (pop) or EDMA (EDM) profiles | declared key or relative/parallel/5th neighbour | top-2 margin < 0.05 = "ambiguous" (allowed) | Profiles [partitura](https://partitura.readthedocs.io/en/latest/_modules/partitura/musicanalysis/key_identification.html), [Faraldo](https://zenodo.org/records/1154586/files/Faraldo_PhDThesis.pdf); margin heuristic |
| 30 | Mode prior | Share of minor/Aeolian across a genre batch | about 85% minor for EDM presets | < 60% minor | [Faraldo](https://zenodo.org/records/1154586/files/Faraldo_PhDThesis.pdf) |
| 31 | Scale conformance | Duration share on declared scale | ≥ 0.95 (≥ 0.85 acid/lo-fi) | below | Heuristic, calibrate |
| 32 | Chord-tone ratio | Strong-beat melody notes in chord; bass = root on downbeats | ≥ 0.7 / ≥ 0.8 | below | Heuristic, calibrate |
| 33 | Melodic intervals | Step/skip/leap histogram | ≥ 60% steps, ≤ 10% leaps > 5th | outside | Heuristic, calibrate |
| 34 | Parallel 5ths/8ves | Between independent layers only | 0 | > 0 (whitelisted in stab mode) | [Intermediate Theory](https://intmus.github.io/inttheory20-21/10-intro-part-writing/b1-part-writing-errors.html) |
| 35 | Key changes | Count per track | ≤ 1, at section boundary, to related key | > 1 or mid-phrase | [Faraldo](https://zenodo.org/records/1154586/files/Faraldo_PhDThesis.pdf) (3.5-5% of tracks) |
| 36 | Repetition ratio | Bars identical to an earlier bar | 0.5-0.9 | > 0.9 for > 32 bars with no variation; < 0.3 | Heuristic, calibrate |
| 37 | Phrase grid | Section lengths | multiples of 8 bars; intro/outro 16-32 (club) | other | [DJ.Studio](https://dj.studio/blog/phrasing-dj-mixing); [Lost Stories](https://loststoriesacademy.com/blogs-and-tutorials/what-makes-a-track-dj-friendly) |
| 38 | Build/drop | S-LUFS, high-band energy, onset density rising over 4-16 bars; ≥ 6 dB low-band jump at bar line | matches arrangement plan | mismatch | Heuristic |
| 39 | Level jumps | S-LUFS step > 3 LU within 1-2 s off a bar boundary | none | any | Heuristic |
| 40 | 303 accent sweep | Peak centroid across runs of consecutive accents | monotonic rise | flat | [Whittle](https://www.firstpr.com.au/rwi/dfish/303-unique.html) |
| 41 | 303 slide | Glide duration, semitone vs. 2 octaves | equal within about 20% | rate-constant | [synth-diy](https://synth-diy.org/pipermail/synth-diy/1998-April/010281.html); tolerance heuristic |
| 42 | Build regression | Null test vs. previous build (same seed) | bit-exact or residual < -90 dBFS | real change: report band deltas | Heuristic |

## Tooling: a permissive core with AGPL and NC parts quarantined

Every package in the required stack has a permissive licence and installs from wheels on native Windows. Versions and licences in the table below come from each project's PyPI or GitHub page. Two licence hazards drive the design. Essentia is AGPL-3.0. Combining it with GPL-3.0-only code is allowed (§13), but the combined work then takes AGPL terms, and its wheels stop at Python 3.11 ([ecosyste.ms](https://packages.ecosyste.ms/registries/pypi.org/packages/essentia)). madmom's code is BSD, but its models and data are **CC BY-NC-SA 4.0**. That non-commercial clause conflicts with the GPL's "no further restrictions", so the models must never be vendored ([madmom](https://github.com/CPJKU/madmom)). BeatNet (CC BY 4.0) depends on madmom and breaks on Python 3.10+ / NumPy 1.24+ without patches ([BeatNet](https://github.com/mjhydri/BeatNet)). Calling the ffmpeg CLI through a subprocess does not create a combined work.

| Role | Package | Licence | Version (date) | Use and caveats |
|---|---|---|---|---|
| Arrays, DSP, PSD, filters, correlation, true peak | numpy, scipy | BSD | n/a | Welch, `resample_poly` 4× true peak (within about 0.1 dB of the Annex 2 FIR; validate once), `linear_sum_assignment` for voice leading |
| WAV I/O | soundfile | BSD | n/a | Load audio; librosa 1.0 dropped audioread ([changelog](https://librosa.org/doc/latest/changelog.html)) |
| Integrated loudness | pyloudnorm | MIT | 0.2.0 (2026-01-04) | BS.1770-4; LRA only an "attempt" at Tech 3342; **no true peak** ([GitHub](https://github.com/csteinmetz1/pyloudnorm), [PyPI](https://pypi.org/project/pyloudnorm/)). Clips < 400 ms return -inf |
| Features, onsets, beats, chroma, pyin, segmentation | librosa | ISC | 1.0.0 (2026-08-11), Py 3.12+ | `beat_track` tempo is an ndarray; hop ≤ 128 for timing; pyin is slow ([PyPI](https://pypi.org/project/librosa/)) |
| Raw MIDI, clock jitter | mido | MIT (verify) | n/a | Ticks, 0xF8 clock, stuck notes |
| Notes, piano roll, chroma | pretty_midi | MIT | n/a | `get_chroma`, `estimate_tempo` ([GitHub](https://github.com/craffel/pretty-midi)) |
| Key, Roman numerals, spelling | music21 | BSD-3 | 10.5.2 (2026-10-28), Py 3.11+ | `analyze('key')`, `romanNumeralFromChord` ([PyPI](https://pypi.org/project/music21/)) |
| Plots and report | matplotlib (Agg) | PSF-style | n/a | Base64 plots in HTML/Markdown plus `report.json` for CI |
| Cross-check (subprocess) | ffmpeg `ebur128`, `astats` | LGPL/GPL build | n/a | I/LRA/TP and per-100 ms `lavfi.r128.*`; DC, Flat_factor, Peak_count, NaN/denormal counts ([ebur128](https://ayosec.github.io/ffmpeg-filters-docs/7.1/Filters/Multimedia/ebur128.html), [astats](https://ayosec.github.io/ffmpeg-filters-docs/7.1/Filters/Audio/astats.html)) |
| Metric reference only | muspy | MIT | 0.5.0 (2022) | Stale; re-implement `pitch_in_scale_rate`, `groove_consistency` etc. in numpy ([metrics](https://hermandong.com/muspy/metrics.html)) |
| Optional structure | MSAF | MIT | unclear | Maintenance and Python 3.12 status unverified ([GitHub](https://github.com/urinieto/msaf)) |
| Optional, WSL only | essentia; madmom; chord-extractor (Chordino) | AGPL; BSD + NC models; unconfirmed | 2.1b6.dev; n/a; 0.1.2 | Never a required dependency ([chord-extractor](https://pypi.org/project/chord-extractor)) |
| Humanisation data | Groove MIDI Dataset | CC BY 4.0 | 13.6 h, 1,150 files | Per-step timing and velocity priors from human drummers. Acoustic styles, not EDM grids ([Magenta](https://magenta.tensorflow.org/datasets/groove)) |

Because the simulator is deterministic, the cheapest and strongest regression check is a null test between builds. Align the two renders by cross-correlation, fit a least-squares gain and subtract. An all-zero residual is bit-exact, a residual under -90 dBFS is float rounding, and anything larger is a real change to inspect band by band **[H]**. Without pyin or neural models, the whole suite should run in a few seconds per rendered minute (estimate, not benchmarked).

## Conclusion

Most of what this knowledge base can enforce comes from the hardware and the standards, not from published targets for "good" genre sound. The firm rules are the BS.1770 maths, Linn's swing scale, Whittle's account of the 303 circuit, Deruty's 808 measurements and one large techno/house corpus. Everything else in the rule table should start as a measurement the tool reports and only become a pass/fail gate after PERF-1 has rendered its own reference distributions. The important design choice follows from that: build the checker as a calibration harness first. That means per-genre median/IQR baselines generated from curated renders, plus a small set of commercial reference tracks per genre to anchor them. The judge should be a set of percentile tests against those baselines, not a list of hard-coded numbers.

The coverage gaps matter for PERF-1's genre list. Dub, dubstep, lo-fi, trip-hop, synthwave, ambient, industrial/EBM and breaks have no sourced loudness or spectral data at all, and their drum grids are convention. Missing pieces worth closing first are:

- the TB-303's original decay and slide times (the Devil Fish manual);
- the 808/909 timing and frequency constants (the Werner/Abel/Smith DAFx-14 paper and the service manuals);
- Butler's *Unlocking the Groove* for phrase structure;
- a genre-labelled EDM drum MIDI corpus, which does not appear to exist openly and that PERF-1's own curated output could eventually become.

Finally, PERF-1 writes its own MIDI, so its harmony and rhythm checks run on exact ground truth. Audio analysis of key, tempo and structure is the error-prone part and should only confirm the MIDI, never override it.
