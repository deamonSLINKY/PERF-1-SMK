# Mixing and Mastering for Electronic Music: Measurable Targets for an Automated "Sound Check"

Context: rules for a PERF-1 (generative techno/acid hardware synth) analysis script. Research date 2026-11-10. Where a figure comes only from vendor blogs, it says so. Items marked **[background, verify]** come from the researcher's prior knowledge of the standards, not from a source fetched in this session. Check them against the primary spec before hard-coding.

## 1. Loudness: standards, streaming targets, measured genre loudness, crest/PLR/PSR

### Takeaway
BS.1770/R128 give exact measurement definitions: 400 ms momentary, 3 s short-term, gated integrated, LRA as P95-P10, oversampled true peak. Streaming services normalize to about -14 LUFS (Apple about -16). Released techno and house masters are far louder than that. One 563-track corpus has medians of -6.9 to -8.8 LUFS integrated with true peaks above 0 dBTP. A "loudness" rule for PERF should therefore report against several reference targets rather than enforce one number. Dynamics rules should use PSR and PLR, which don't change with playback gain.

### Cited Findings
**Measurement definitions**
- EBU R128: normalize to -23 LUFS integrated, ±0.5 LU tolerance (±1 LU for live or unpredictable programmes); maximum true peak -1 dBTP — [Wikipedia: EBU R 128](https://en.wikipedia.org/wiki/EBU_R_128)
- Momentary = 400 ms sliding window, short-term = 3 s, integrated = whole programme. Short-term meters update at ≥10 Hz, integrated at ≥1 Hz. M and S are ungated — [Wikipedia: EBU R 128](https://en.wikipedia.org/wiki/EBU_R_128)
- Integrated gating: absolute gate -70 LUFS, then a relative gate 10 LU below the ungated-by-relative integrated value — [Wikipedia: EBU R 128](https://en.wikipedia.org/wiki/EBU_R_128)
- True peak is measured by oversampling to reconstruct inter-sample peaks — [Wikipedia: EBU R 128](https://en.wikipedia.org/wiki/EBU_R_128)
- EBU Mode (Tech 3341) exists so meters from different vendors read the same — [Wikipedia: EBU R 128](https://en.wikipedia.org/wiki/EBU_R_128)
- LRA (Tech 3342) = the difference between the 10th and 95th percentiles of the short-term loudness distribution. The low percentile stops fade-outs from dominating, and the high percentile stops one loud event from driving the value. It uses cascaded gating: -70 LUFS absolute, then a relative gate. The current (v4, 2023) document shows -20 LU relative. Window overlap must be ≥2.9 s, i.e. ≥10 Hz — [EBU Tech 3342 PDF](https://tech.ebu.ch/docs/tech/tech3342.pdf); [EBU Tech 3342 page](https://tech.ebu.ch/publications/tech3342)
  - Disagreement: an older revision history mentions changing the gate from -8 to -10 LU. Use -20 LU for LRA (current spec); -10 LU is the integrated-loudness gate. Don't mix them up.
  - The percentile interpolation is not specified. One Rust implementation uses linear interpolation (Hyndman-Fan type 7) — [ebur128-stream lra.rs](https://docs.rs/crate/ebur128-stream/0.2.0/source/src/lra.rs)
- R128 sets no maximum LRA — [Wikipedia: EBU R 128](https://en.wikipedia.org/wiki/EBU_R_128)

**Streaming targets**
- Spotify (official): normalizes to -14 LUFS integrated (ITU 1770) and recommends mastering to -14 LUFS. It asks for ≤ -1 dBTP because of lossy encoding (Ogg/AAC). If the master is louder than -14 LUFS, keep true peak below -2 dBTP. User presets: Loud -11, Normal -14, Quiet -19 LUFS. For quiet tracks, Spotify's gain-up limiter engages at -1 dB (sample values) with 5 ms attack and 100 ms decay. Spotify keeps 1 dB headroom: in its example, a -20 LUFS track with a -5 dBTP peak is raised only to -16 LUFS. The web player and some third-party devices don't normalize — [Spotify for Artists: Loudness normalization](https://support.spotify.com/artists/article/loudness-normalization/); [AU version](https://support.spotify.com/au/artists/article/loudness-normalization/)
- Apple Music Sound Check: about -16 LUFS, -1 dBTP, mostly turns loud tracks down. Disagreement: one review says Apple's own docs don't publish the -16 figure — [Fora Soft 2026](https://www.forasoft.com/learn/audio-for-video/articles-audio/lufs-targets-per-platform-2026); [Interspace](https://interspacemusic.com/blog/?p=4929); contradicted/qualified by [Orphiq](https://orphiq.com/resources/mastering-for-streaming)
- YouTube about -14 LUFS, turn-down only (never boosts) — [RoEx](https://www.roexaudio.com/blog/what-lufs-should-i-master-at); [Fora Soft 2026](https://www.forasoft.com/learn/audio-for-video/articles-audio/lufs-targets-per-platform-2026)
- Tidal: -14 in newer sources, -14.5 in an older Sage Audio article. Album-aware, turns down only — [Sage Audio](https://www.sageaudio.com/articles/master-music-for-streaming); [MasteringBOX](https://masteringbox.com/blog/mastering-for-streaming/)
- Amazon Music: sources disagree (-14 vs. "-9 to -13") — [Travsonic](https://www.travsonic.com/?p=24509)
- SoundCloud: sources disagree. Some say it doesn't normalize (plays at native level), others say -14 LUFS — [RoEx](https://www.roexaudio.com/blog/how-to-master-music-for-streaming-spotify-apple-music-deezer-youtube); [Fora Soft 2026](https://www.forasoft.com/learn/audio-for-video/articles-audio/lufs-targets-per-platform-2026)
- Several sources say these are playback references, not mastering targets — [Rob Kleiner](https://www.robkleiner.com/resources/mastering-music-for-streaming-platforms)

**Measured loudness of commercial electronic masters**
- TrackSensei, 563 released techno and house tracks (published June 2026; release years not stated). BS.1770 stereo integrated loudness, medians: hard techno -6.9 (P25-P75 -8.0 to -6.4), peak-time techno -7.7, groove techno -7.7, house -8.0 (-9.0 to -7.1), tech house -8.8, melodic techno -8.8 (-9.9 to -8.0). Median true peak is **above 0 dBTP** in every style: +0.47 (groove) to +1.22 (tech house). Vendor data. The same vendor's mono-sum scale reads about 3.3 LU lower, so check which number a source quotes — [TrackSensei 563 tracks](https://tracksensei.com/blog/we-analyzed-563-techno-house-tracks); [TrackSensei loudness guide](https://tracksensei.com/blog/how-loud-should-your-master-be)
- LuvLang Studio, 151 commercial masters from 2004-2024 (123 from 2018 or later), BS.1770-4 with 4× oversampled true peak, n=5-10 per genre. The authors call the results "directional". Electronic median -10.2 LUFS / +0.9 dBTP; EDM -11.0 / -1.0; hip-hop -10.7; pop -10.1; metal -14.7 — [LuvLang loudness 2026](https://luvlang.studio/research/loudness-2026)
  - Disagreement: LuvLang's "EDM" (-11) is about 3-4 LU quieter than TrackSensei's techno medians. The samples, selection methods and subgenres differ.
- Drum and bass: one 101-track study found integrated loudness spread over more than 12 LU, with no single target (the article didn't load in full; this comes from the search summary) — [WeFoundDnB](https://wefounddnb.com/dnblog/how-loud-should-drum-bass-be-lufs-from-the-pros-and-101-measured-tracks). Arytmo says DnB and hardstyle run hotter than about -8.5 LUFS typical of other chart genres — [Arytmo](https://arytmo.com/blogs/analysis/mastering-14-lufs)
- Vendor advice (not measurements): club music -11 to -8 LUFS, DnB -10 to -8 — [TrackScore](https://trackscore.ai/blog/ideal-lufs-electronic-music)
- An arXiv dataset study of mixes vs. masters (2412.03373) reports that masters cluster around -14 LUFS (search summary only; the PDF couldn't be parsed in this session) — [arXiv 2412.03373](https://arxiv.org/pdf/2412.03373)

**Crest factor, PSR, PLR**
- Crest factor = peak minus average level in dB. It is not standardized, because "average" can be VU, RMS or loudness. Example: -1 dBFS peak with -14 dBFS RMS gives CF 13 dB — [MeterPlugs blog](https://www.meterplugs.com/blog/2017/05/18/crest-factor-psr-and-plr.html)
- PSR = true peak minus short-term (3 s) loudness. PLR = true peak minus integrated loudness over a whole song or album. Neither changes with playback gain. Exception: Spotify's gain-up limiter can change PSR — [MeterPlugs blog](https://www.meterplugs.com/blog/2017/05/18/crest-factor-psr-and-plr.html)
- Ian Shepherd: don't go below PSR 8 in the loudest sections, in any genre; lower "will often sound crushed" — [MeterPlugs blog](https://www.meterplugs.com/blog/2017/05/18/crest-factor-psr-and-plr.html); [Dynameter guide](https://www.meterplugs.com/files/dynameter-guide.pdf)
- Sound On Sound's Dynameter review gives examples: a dynamic jazz track at PLR about 20, and a modern peak-normalised track at PLR 9 with minimum PSR 6 — [Sound On Sound: Dynameter](https://www.soundonsound.com/reviews/meterplugs-dynameter)
- A secondhand summary describes tiers of "Competitive" = PSR ≥10 and "Limited" = PSR ≥8 (unofficial) — [Lars Lentz blog](https://world.hey.com/larslentz/9-21-23-plr-psr-short-term-lufs-c7079460)
- The AES e-Brief co-authored by Shepherd on standardizing PSR — [AES e-lib 19324](https://aes2.org/publications/elibrary-page/?id=19324)

### Inferences
- Implied PLR of current techno: TrackSensei medians (I ≈ -7.7, TP ≈ +0.9) give PLR ≈ 8.6 dB. Hard techno gives about 7.7. A PERF rule of "PLR < 7 = likely over-limited; 8-10 = club-competitive; 10-14 = streaming-friendly; >14 = very dynamic or ambient" would fit these data and Shepherd's PSR 8 floor. These bands are proposed, not sourced.
- Suggested reported targets: club/DJ reference -9 to -7 LUFS-I; streaming reference -14 (Spotify/YouTube) and -16 (Apple); broadcast -23. Report "dB of turn-down at Spotify" = max(0, I + 14), rather than pass/fail.
- True-peak rule: flag > -1.0 dBTP as "streaming risk (lossy encode clipping)" and > -2.0 dBTP when I > -14 (Spotify guidance). Commercial club masters routinely break this. Make it a warning, not an error, for a "club" profile.
- LRA: no sourced genre ranges were found. In practice, 4-8 LU is typical for loop-based dance music and much lower values suggest a flat, non-evolving arrangement **[background, verify]**. For a generative firmware, LRA below about 3 LU could flag "no dynamic arc".
- **[background, verify]** BS.1770 K-weighting is a two-stage filter: a high-shelf of about +4 dB above about 1.5 kHz ("pre-filter"), then an RLB high-pass at about 38 Hz. The weighted mean square is summed over channels with weights L/R/C = 1.0 and surround = 1.41, and L = -0.691 + 10·log10(Σ G·z). The true-peak meter oversamples 4× at 48 kHz. Sub-bass under about 40 Hz therefore barely counts toward LUFS, which matters for sub-heavy techno.
- **[background, verify]** BS.1770-5 (2023) mainly added advanced-sound-system and object-based channel weightings. Stereo music measurements match -4.

### Gaps
- No peer-reviewed, single-method corpus covering dubstep, ambient, lo-fi, synthwave and industrial loudness was found. These genres have no sourced numbers.
- Beatport has no published loudness normalization or target. No source was found; Beatport is commonly said not to normalize, but that is unverified.
- Official Apple, Tidal, SoundCloud and YouTube documentation was not fetched. All those figures are secondary.
- Full text of arXiv 2412.03373 and the WeFoundDnB article could not be read.

## 2. Spectral balance: target curves, band ratios, measurement method

### Takeaway
Commercial mixes follow a consistent downward-tilting long-term spectrum (Pestana et al. 2013). iZotope's target curves are statistical ranges, not ideal lines, built from thousands of commercial tracks and shown in four broad bands (20-250, 250-2k, 2k-8k [the docs list two different band schemes], 8k-20k). For techno specifically, measured data show that 78-92% of signal energy sits below 250 Hz. Any band-ratio rule must use genre-specific references, not a generic pop curve.

### Cited Findings
- Pestana, Ma, Reiss, Barbosa & Black, "Spectral Characteristics of Popular Commercial Recordings 1950-2010", AES 135th Convention, NY, Oct 2013. Long-term average spectra of a large corpus show a consistent tilt toward a target EQ curve. The curve partly reflects industry practice and partly resembles natural acoustic ensemble spectra. The paper also proposes a new spectral-averaging method — [AES e-lib 17010](https://www.aes.org/e-lib/browse.cfm?elib=17010); [author PDF](https://joshreiss.github.io/documents/2013/Pestana%20Ma%20Reiss%20-%20AES135.pdf); [UCP repository](https://ciencia.ucp.pt/en/publications/spectral-characteristics-of-popular-commercial-recordings-1950-20/)
- iZotope Tonal Balance Control: the curves come from analysis of "thousands of popular songs" and show ranges of typical variation. Broad view uses four bands: Low 20-250 Hz, Low-Mid 250 Hz-2 kHz, High-Mid 2-8 kHz, High 8-20 kHz. Another doc version names them Low/Low-mid/Mid/High-mid with the same edges. TBC2 has 12 curves (9 new genre targets) — [iZotope TBC docs: target curves](https://downloads.izotope.com/docs/tonal-balance-control/meters-and-target-curves/index.html); [iZotope curve page](https://www.izotope.com/en/products/tonal-balance-control-2/features/tonal-balance-curves); [MusicTech](https://musictech.com/news/gear/izotope-release-tonal-balance-control-2/); [iZotope learn](https://www.izotope.com/en/learn/leveling-up-your-mastering-workflow-with-tonal-balance-control)
- Genre curves differ mostly in the width of their allowed range. Example: R&B-Soul widens above about 5 kHz, and Reggae has a wider bass range — [Ozone 8 TBC docs](https://downloads.izotope.com/docs/ozone8/tonal-balance-control/index.html)
- Techno and house band energy shares (TrackSensei, Bark-weighted analysis, medians, 563 tracks). Sub 20-60 Hz / bass 60-250 Hz: house 34.8% / 43.1%; tech house 34.9 / 44.9; groove techno 39.9 / 51.9; melodic techno 46.6 / 42.0; peak-time techno 43.0 / 44.7; hard techno 40.2 / 48.0. That leaves about 8-22% of energy above 250 Hz. Kick fundamental medians: 51-54 Hz (house, melodic, peak-time) and 56.5 Hz (groove); overall cluster 48-62 Hz. Tempo medians: house 126, peak-time 131, groove 140, hard 141 BPM — [TrackSensei 563 tracks](https://tracksensei.com/blog/we-analyzed-563-techno-house-tracks)

### Inferences
- **[background, verify]** The "pink-noise tilt" reference: pink noise is flat per octave (-3 dB/octave on a per-Hz PSD). Commercial masters on a 1/3-octave or per-octave display usually slope down about 3-5 dB/octave above roughly 100-200 Hz, and dance music sits at the bass-heavy end. Use a linear regression of 1/3-octave band levels (dB) against log2(f) from 100 Hz to 10 kHz as the "tilt" metric. Then flag tilt that is shallower than the genre reference by more than about 1.5 dB/oct ("bright/harsh") or steeper ("dull/dark"). The thresholds are proposed, not sourced.
- Measurement recipe (standard DSP, proposed): mono-sum or mid channel; Welch PSD (Hann, 8192-16384 points, 50% overlap) over the whole track or over short-term-loudness-gated frames, to drop silence. Integrate into 1/3-octave bands (ISO 266 centres) or the broad bands listed above. Normalize to total energy, or to the 1 kHz band, so the result doesn't depend on level. Compare against a reference distribution (median ± IQR) built from reference tracks, as TBC does. Use unweighted spectra for tonal balance. K-weighting is for loudness only and would hide the sub band.
- Suggested PERF bands (from the caller's brief, aligned with TBC/TrackSensei): sub <60, low 60-250, low-mid 250-2k, presence 2-6k, high 6-10k, air >10k. Starter techno thresholds (the 563-track medians above are energy shares, so they are dominated by bass):
  - Warn if sub + bass falls outside about 75-95% of energy.
  - Warn if sub/bass ratio is < 0.6 or > 1.2 (measured range about 0.67-1.11).

### Gaps
- The numeric shape of the Pestana average curve (dB/octave) and its genre breakdowns could not be extracted: the PDF text was not parseable in this session.
- iZotope does not publish the electronic/EDM target curve data numerically.
- No sourced per-band dB targets for dubstep, ambient, lo-fi, synthwave or industrial.

## 3. Low end: kick/bass, sidechain, mono bass, DC, sub checks, phase

### Takeaway
In released techno and house, the sub band is essentially mono (median sub-band correlation 0.998-1.0) and mono-sum loss is tiny (0.18-0.46 LU). Kick fundamentals cluster at 48-62 Hz. For PERF, the check is straightforward:
- correlation of the band below about 100-150 Hz should be ≥ about 0.95;
- the mono-sum loudness drop should be below about 1 LU;
- the kick fundamental should be detectable and the sub shouldn't pile up on top of it.

### Cited Findings
- Sub-band stereo correlation median 0.998-1.0 in every techno/house style. Median mono-sum loudness loss 0.18 (groove) to 0.46 LU (house), against a common 1.0 LU rule of thumb. Kick fundamental cluster 48-62 Hz — [TrackSensei 563 tracks](https://tracksensei.com/blog/we-analyzed-563-techno-house-tracks)
- Many electronic producers mono everything below about 100 Hz with a multiband imager on the master. Low bass should be mono because ears localize bass poorly — [MasteringBOX: mixing bass](https://www.masteringbox.com/mixing-bass-3-steps-perfect-low-end/)
- Mono low end is also historically tied to vinyl (out-of-phase low frequencies cause groove and stylus problems). This comes from a forum, so it is anecdotal — [VI-Control thread](https://vi-control.net/community/threads/do-you-mono-the-low-frequencies-of-double-basses.162171/)
- Multiband correlation meters (e.g. MAAT 2BC multiCORR, 31 bands) show phase per band, which is useful for finding out-of-phase low end — [KVR: 2BC multiCORR](https://www.kvraudio.com/product/2bc-multicorr-by-maat-labs)
- Sidechain helps keep kick and bass from colliding. No numeric settings were sourced — [MasteringBOX: mixing bass](https://www.masteringbox.com/mixing-bass-3-steps-perfect-low-end/)
- DC offset = mean displacement from zero. ffmpeg astats reports it per channel. A high-pass removes it — [FFmpeg astats docs](https://ayosec.github.io/ffmpeg-filters-docs/5.0/Filters/Audio/astats.html); [FFmpeg cookbook astats](https://ffmpeg-cookbook.pages.dev/en/articles/astats/)

### Inferences
- Proposed rules:
  - **Sub mono.** Band-pass 20-120 Hz on each channel and compute Pearson correlation in 100-400 ms windows. FAIL if the median is below 0.9, WARN if below 0.97. Also compute S/M energy in that band and flag above -20 dB.
  - **DC offset.** Flag |mean| > 0.001 FS (-60 dBFS) **[proposed threshold]**.
  - **Infrasonic.** Flag a meaningful share of energy below 20-25 Hz. It wastes headroom and is common on analog-modelled acid basses with self-oscillating filters.
  - **Kick/bass overlap and sidechain.** Detect kick onsets (low-band onset envelope), then measure the 40-120 Hz envelope of the non-kick part (or of the whole mix between kicks). Ducking depth of about 3-10 dB, recovering within about 1/8-1/4 beat (e.g. about 115-230 ms at 130 BPM), is typical practice **[background, verify]**. A generative firmware can report "low-band dip at kick (dB)" and "recovery time (ms)".
  - **Sub/kick spectral peak.** Find the kick's fundamental (45-65 Hz) and the bass's fundamentals. If they share a 1/3-octave band, flag a masking risk.

### Gaps
- No authoritative source gave sidechain depth or timing figures, or the exact crossover frequency for mono bass in mastering (100 vs. 150 Hz). Both are industry practice, not standards.
- Elliptical EQ and vinyl cutting specifications were not found.

## 4. Stereo: correlation, side/mid ratio, width by band, mono compatibility

### Takeaway
Released techno and house are fairly narrow (width index 0.16-0.24 on a 0-1 scale) and very mono-compatible (median mono-sum loss of 0.5 LU or less). Correlation meters read +1 for identical channels and about 0 for unrelated ones, and sustained negative values signal phase problems.

### Cited Findings
- Stereo width medians 0.16-0.24 (0-1 scale; vendor-defined metric): groove techno 0.16, melodic techno 0.20, house 0.24. Mono-sum loss 0.18-0.46 LU — [TrackSensei 563 tracks](https://tracksensei.com/blog/we-analyzed-563-techno-house-tracks)
- A correlation meter reads near +1 for alike (mono) channels and near 0 for unrelated channels. Multiband versions locate phase problems by frequency — [KVR: 2BC multiCORR](https://www.kvraudio.com/product/2bc-multicorr-by-maat-labs); [Gearspace correlation thread](https://gearspace.com/threads/correlation-meter-levels.733274/)
- Commercial analyzers (MixDiagnose) report stereo width as a headline metric alongside LUFS and 6-band balance — [DEV: MixDiagnose](https://dev.to/oren_mixdiagnose_1380fb19/i-built-a-free-ai-mix-analyzer-that-scores-your-music-0-100-2jd6)

### Inferences
- Proposed metrics:
  - full-band correlation (median and 5th percentile over 400 ms windows);
  - per-band correlation (sub / low / low-mid / high / air);
  - side-to-mid energy ratio (dB) per band;
  - mono-sum loss = LUFS(L+R)/2 vs. stereo LUFS.

  Starter thresholds:
  - full-band correlation median: WARN below 0.3, FAIL below 0;
  - S/M: WARN above -6 dB full band;
  - mono-sum loss: WARN above 1.0 LU (TrackSensei's rule of thumb), with the techno reference at about 0.3 LU.
- Expect width to grow with frequency: about mono at sub, widest in the 2-10 kHz pads and hats. Flag inverted profiles (bass wider than highs).

### Gaps
- No standard width scale exists. The 0-1 width index is TrackSensei's own and its formula isn't published. Sourced correlation ranges for "good" masters were not found beyond the meter's definition.

## 5. Dynamics: over-limiting, pumping, transients, clipping, ISPs, distortion

### Takeaway
Use PSR (true peak minus short-term loudness) ≥ 8 dB in loud sections as the over-limiting guardrail. Use ffmpeg astats "flat factor" and "peak count" to detect hard clipping, and oversampled true peak to find inter-sample overs. Released techno already runs at a median true peak of +0.5 to +1.2 dBTP, so ISP flags fire on most commercial club masters.

### Cited Findings
- PSR ≥ 8 in the loudest parts, any genre (Shepherd) — [MeterPlugs blog](https://www.meterplugs.com/blog/2017/05/18/crest-factor-psr-and-plr.html)
- ffmpeg astats: crest factor (peak/RMS ratio, not in dB); flat factor (consecutive samples with the same value at the min/max level = clipping signature); peak count; DC offset; per-channel and "Overall" metadata keys (lavfi.astats.N.*) — [FFmpeg astats 5.0 docs](https://ayosec.github.io/ffmpeg-filters-docs/5.0/Filters/Audio/astats.html)
- ffmpeg loudnorm uses libebur128, targets I/LRA/TP, defaults to TP -2.0 dB (range -9 to 0), and upsamples for true-peak limiting — [FFmpeg loudnorm source](https://www.ffmpeg.org/doxygen/trunk/af__loudnorm_8c_source.html); [rentry ebur128 notes](https://rentry.co/ebur128)
- Commercial techno median true peak +0.47 to +1.22 dBTP, i.e. inter-sample overs are normal in released club masters — [TrackSensei 563 tracks](https://tracksensei.com/blog/we-analyzed-563-techno-house-tracks)
- Spotify asks for ≤ -1 dBTP (≤ -2 if louder than -14 LUFS) to avoid lossy-encode distortion — [Spotify for Artists](https://support.spotify.com/artists/article/loudness-normalization/)

### Inferences
Proposed detectors (not sourced as standards):
- **Clipping.** Runs of 3 or more consecutive samples within 0.01 dB of the absolute peak (≥ -0.1 dBFS). Report count per minute.
- **ISP.** TP − sample peak > 0.5 dB means the signal is clipped or limited hard against 0 dBFS.
- **Over-limiting.** The 10th percentile of PSR over loud sections (short-term LUFS within 3 LU of the maximum) below 7 dB.
- **Pumping.** Modulation of the non-kick band envelope (e.g. 200 Hz-8 kHz) in sync with kick onsets, measured as the correlation of band RMS with the kick grid. Intended sidechain pumping is a genre feature, so report it as a value rather than a fault.
- **Transient preservation.** Kick attack crest. Compare the peak of the first 10 ms after onset with the RMS of the following 100 ms. A low value means the transients are squashed.
- **Distortion.** Use spectral flatness, or the ratio of energy above 16 kHz to 4-16 kHz, as a proxy for aliasing and clip harmonics. A generative digital synth can alias: check for inharmonic energy that doesn't track note pitch.

### Gaps
- No sourced threshold for pumping or transient-loss metrics. No validated THD-from-music estimator was found.

## 6. Common mix problems detectable automatically

### Takeaway
Mud (200-500 Hz) and harshness (2-5 kHz) are well-established problem bands, but their thresholds are rules of thumb. Implement them as deviations from a genre reference spectrum (as TBC does), not as absolute dB numbers. Reverb excess, noise floor, clicks and level jumps can be measured with standard DSP, but no sourced thresholds were found.

### Cited Findings
- Low-mid buildup at 200-500 Hz appears in most mixes one analyzer saw. The typical fix is a broad cut of about 2-3 dB near 300 Hz (vendor) — [DEV: MixDiagnose](https://dev.to/oren_mixdiagnose_1380fb19/stop-guessing-if-your-mix-is-ready-for-mastering-19cd); [Gearspace mud thread](https://gearspace.com/threads/mud-cleaning.845652/)
- Hearing is most sensitive at 2-5 kHz, and harshness there causes ear fatigue. Dynamic resonance suppression is the usual fix — [Baby Audio: fix a harsh mix](https://babyaud.io/blog/fix-a-harsh-mix)
- Existing analyzers (MixDiagnose: 6 bands, LUFS, width, masking, genre fit, 0-100 score; built on librosa) don't publish their scoring — [DEV: MixDiagnose](https://dev.to/oren_mixdiagnose_1380fb19/i-built-a-free-ai-mix-analyzer-that-scores-your-music-0-100-2jd6); [Sonic State 2026](https://sonicstate.com/news/2026/07/22/ai-powered-mix-analysis-platform)

### Inferences
Proposed detectors:
- **Mud.** The 200-500 Hz share relative to the 60-250 and 2-6k bands exceeds the reference P75 by more than 2 dB.
- **Harshness.** 2-5 kHz short-term energy relative to 500-2k exceeds the reference by more than 2-3 dB. Also check the peak of the 3 s window, since acid squelch resonances are momentary. K-weighting already boosts this region, so compute it unweighted.
- **Resonances.** On a 1/12- or 1/24-octave smoothed long-term spectrum, find peaks with prominence above about 6 dB that persist in more than 50% of frames. Acid filter resonance moves, so a *static* prominent peak is the problem. A sweeping one is the music.
- **Masking.** Per 1/3-octave band, find time-overlap of kick and bass energy, using onset-aligned frames.
- **Reverb/decay.** Estimate the decay slope after transient offsets (Schroeder-like backward integration on note releases), or the ratio of the side-channel tail to the direct sound. High late energy with low correlation means a washy mix.
- **Noise floor.** Use the 5th-percentile momentary loudness or RMS in "silent" gaps. Flag anything above about -60 dBFS RMS, which suits a hardware synth with a DAC or analog path.
- **Clicks/denormals.** First-difference spikes more than N·σ above local, or zero-crossing glitches at loop or pattern boundaries. A denormal stall shows as dropouts: runs of exact zeros mid-signal.
- **Level jumps between sections.** A short-term LUFS step of more than 3 LU within 1-2 s that isn't on a bar boundary, or a sudden jump in M-loudness when the generative engine switches patterns.

### Gaps
- No peer-reviewed thresholds for mud, harshness, resonance prominence, reverb excess or noise floor in electronic music were found. All of the above need calibrating on reference tracks.

## 7. How pro and open-source tools measure these

### Takeaway
The open-source stack can reproduce the core pro-meter figures. ffmpeg's ebur128 filter and libebur128 give I, S, M, LRA and TP; ffmpeg astats gives DC, crest, flat factor and peak count; pyloudnorm gives BS.1770 integrated loudness in Python. Spectral and stereo metrics need custom numpy/scipy code modelled on TBC-style reference distributions.

### Cited Findings
- ffmpeg loudnorm/ebur128 build on libebur128 (I, LRA, TP; upsampled true peak) — [FFmpeg loudnorm patch](https://ffmpeg.org/pipermail/ffmpeg-devel/2016-April/192668.html); [ffmpeg ebur128.h](https://ffmpeg.org/doxygen/6.0/ebur128_8h.html)
- astats gives peak, RMS, DC, crest, flat factor and peak count, frame-wise or overall — [FFmpeg astats docs](https://ayosec.github.io/ffmpeg-filters-docs/5.0/Filters/Audio/astats.html); [FFmpeg cookbook](https://ffmpeg-cookbook.pages.dev/en/articles/astats/)
- MATLAB Audio Toolbox `integratedLoudness` implements BS.1770 gating as a cross-check reference — [MathWorks](https://www.mathworks.com/help/audio/ref/integratedloudness.html)
- iZotope TBC compares the spectrum against statistical genre ranges in broad and fine views — [iZotope TBC docs](https://downloads.izotope.com/docs/tonal-balance-control/meters-and-target-curves/index.html)
- Dynameter (Ian Shepherd/MeterPlugs) gives real-time PSR (3 s) and PLR — [Dynameter guide](https://www.meterplugs.com/files/dynameter-guide.pdf)
- Mono-sum vs. stereo metering conventions differ by about 3.3 LU (TrackSensei's internal scale). Always state the channel mode — [TrackSensei](https://tracksensei.com/blog/how-loud-should-your-master-be)

### Inferences
- Recommended pipeline:
  1. Run `ffmpeg -i x.wav -af ebur128=peak=true -f null -` for I/LRA/TP and frame-wise M/S (or pyloudnorm plus a 4× resampled peak check).
  2. Run `astats=metadata=1:reset=…` for DC, clipping and crest.
  3. Do spectrum, band shares, tilt and per-band correlation in numpy/scipy (Welch, Butterworth band splits).
  4. Derive PSR from the S series and TP; PLR = TP − I.
- **[background, verify]** pyloudnorm (Steinmetz) implements BS.1770-4 integrated loudness, with gating and K-weighting via biquads. It doesn't compute true peak, LRA or momentary/short-term series natively, so add those yourself or use ffmpeg/libebur128.
- **[background, verify]** Youlean Loudness Meter (free) and iZotope Insight give I/S/M/LRA/TP/PLR. Voxengo SPAN gives an FFT spectrum (default 4096-point, -4.5 dB/oct slope option) plus correlation and an RMS/K-system readout. Sonarworks is room/headphone calibration, not a mix-analysis metric. These are good tools to cross-check a PERF implementation against.

### Gaps
- pyloudnorm, Youlean, Insight and SPAN documentation were not fetched in this session. The tool details above marked [background, verify] need confirming.
