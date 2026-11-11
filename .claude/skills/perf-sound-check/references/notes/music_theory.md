# Music Theory for Generative Electronic Music Engines (rules a generator can follow, a checker can test)

Scope note: this pass got hard data from two corpus sources: de Clercq & Temperley's rock corpus, read as full text, and Faraldo's 2017 PhD thesis on EDM tonality, about 2,000 hand-annotated EDM excerpts, also read as full text. It also used Hooktheory API data, published key-profile tables, and practitioner guides (Attack Magazine, MusicTech, Producer School, forums). Several requested items did not get a primary source in this pass and are listed under Gaps rather than invented: drop-2, avoid-note tables, lo-fi ii-V statistics, synthwave, Huron's interval statistics and McGill Billboard chord statistics. Where something below is "standard textbook theory" without a fetched source, it is marked under Inferences.

---

## 1. Chord progressions per genre + statistical findings on chord transitions

### Takeaway
EDM is overwhelmingly minor, about 85% in two large datasets (vs. about 85% major in pop/rock). It is Aeolian-first: the subtonic bVII is far more common than the leading tone. It is built from 2-, 4- or 8-bar loops with roughly one chord per bar and no cadences. The best transition statistics come from rock/pop corpora (de Clercq & Temperley, Hooktheory). They show "non-functional" harmony: IV is as likely before I as V is, root motion by 4th is symmetric up/down, and frequency falls off with circle-of-fifths distance. These are usable as priors, but they come from major-dominated pop and need re-weighting for minor EDM.

### Cited Findings
**EDM-specific (Faraldo thesis, audio + expert annotation)**
- KeyFinder EDM dataset: 85.1% of tracks in minor keys. GiantSteps dataset: 84.6% minor. The three pop datasets combined (Billboard, Beatles, Rolling Stone corpus) are about 84% major, so pop vs. EDM modality is "almost inverse." — [Faraldo 2017, Tonality Estimation in EDM (PhD thesis, UPF)](https://zenodo.org/records/1154586/files/Faraldo_PhDThesis.pdf)
- Even when the Beatport sample was deliberately drawn from tracks Beatport tagged as major, manual annotation still gave 63% minor, 28% major and the rest ambiguous or multi-tag. — [Faraldo 2017](https://zenodo.org/records/1154586/files/Faraldo_PhDThesis.pdf)
- In the revised GiantSteps+ set, purely major tracks were about 10%, "comparable to the share of modally ambiguous entries." Ambiguous tracks appear across all subgenres, suggesting modal ambiguity is characteristic of EDM as a meta-genre. — [Faraldo 2017](https://zenodo.org/records/1154586/files/Faraldo_PhDThesis.pdf)
- "In minor mode, there is hardly any appearance of the leading tone (♮VII)… favouring other minor scales, especially aeolian (♭VII) and phrygian (♭II)." — [Faraldo et al., Key Estimation in EDM (ECIR 2016)](https://www.researchgate.net/publication/309018542_Key_Estimation_in_Electronic_Dance_Music)
- Textural census of GiantSteps+ (expert checklist):
  - Chord sequences, riffs, pedal tones and arpeggios are each present in under 40% of tracks.
  - 60% have a lead melody, and about half of those (about 30% of all tracks) are vocal.
  - Over 40% have a pitch-only (drumless) section, i.e. a break or build-up.
  - Key changes occur in about 5% of tracks.
  - Source: [Faraldo 2017](https://zenodo.org/records/1154586/files/Faraldo_PhDThesis.pdf)
- House loops "normally consist of sequences of 2 or 4 bars, with a tendency to have a single chord per measure." 8-bar loops are rarer and usually a 4-bar pattern with a variation at the end. — [Faraldo et al., House Harmonic Filler (TENOR 2017)](https://www.tenor-conference.org/proceedings/2017/17_Faraldo_tenor2017.pdf)
- Loop harmony creates "clear harmonic units without cadential points."
  - A two-chord "shuttle" (Tagg) oscillates between two chords of equal weight, so the tonic is often ambiguous.
  - The tonic is fixed mostly by non-tonal cues: the beat layer, the bass layer and hypermetric arrangement changes.
  - The authors take the first chord's root as tonic, citing Tagg.
  - Source: [House Harmonic Filler](https://www.tenor-conference.org/proceedings/2017/17_Faraldo_tenor2017.pdf)
- Deep house "is usually composed with chord loops borrowed from… soul, rhythm-and-blues or even jazz, using extended chords other than simple triads." In the 48-loop deep/classic house MIDI corpus, mixed modality is common. One example has Aeolian first and last chords, but the second chord is borrowed from the parallel major. — [House Harmonic Filler](https://www.tenor-conference.org/proceedings/2017/17_Faraldo_tenor2017.pdf)
- Techno and two-chord vamps:
  - Spicer (quoted by Faraldo) says tritone and semitone relationships seem characteristic of many techno tracks.
  - Examples are Prodigy's "Full Throttle", an i-bII vamp with Phrygian mixture, and "Break and Enter", Gm7 to a tritone-related D-flat major triad ("locrian mixture").
  - Faraldo cautions that Spicer's generalisation is not backed by stated sources (disagreement noted).
  - Source: [Faraldo 2017](https://zenodo.org/records/1154586/files/Faraldo_PhDThesis.pdf)
- Deep house chord example (Attack Magazine):
  - MAW "To Be In Love" strings = Fm7 - Abmaj7 - Cm7.
  - Am7 - Em7 - Dm7 loop: dropping Am7's 7th an octave makes a descending E-D-C top line.
  - Advice is to keep the 7th in every chord.
  - Cm7 = Eb6 by pitch content, and the bass note decides which one is heard.
  - Source: [Attack Magazine, Passing Notes: Deep House Chords](https://www.attackmagazine.com/technique/passing-notes/passing-notes-deep-house-chords/)
- The deep house "core chord" is the min9, a full five-note voicing including the b7. — [MusicRadar, deep house minor ninth chords](https://musicradar.com/how-to/how-to-create-deep-house-minor-ninth-chords)
  - The Herbie Hancock m11 voicing is LH 5th, RH b3-11-b7-9. — [PianoGroove](https://pianogroove.com/jazz-piano-lessons/herbie-hancock-voicing/)
  - Rootless chord voicings are recommended so the bass owns the root. This comes from a sample-pack page, low reliability. — [jamburgr deep house](https://jamburgr-web.pages.dev/library/deep-house)
- Trance (practitioner sources, low authority):
  - The i, VI and VII chords are "some of the most common" in trance.
  - Euphoric trance is often minor-based with major-mode borrowed chords.
  - Others say "there is no specific set of chord progressions for trance."
  - Source: [KVR forum](https://www.kvraudio.com/forum/viewtopic.php?p=5515583); [TranceAddict](https://tranceaddict.com/forums/showthread/t-298275.html)
- Psytrance: harmony is "sparse and modal (often minor, Phrygian, or Phrygian dominant)," chord changes are kept minimal, and the music relies on short motifs. — [Melodigging, psytrance](https://www.melodigging.com/genre/psytrance)

**Pop/rock transition statistics (priors)**
- de Clercq & Temperley root distribution (100 songs, 1950s-90s). Proportions of all chords:
  - I .328, IV .226, V .163, bVII .081, VI .072, bVI .040, II .036, bIII .026, III .019, bII .005, VII .004, #IV .003.
  - The top five (I, IV, V, bVII, VI) make up 87% of harmonies.
  - Source: [de Clercq & Temperley 2011, A corpus analysis of rock harmony, Popular Music 30(1)](https://rockcorpus.midside.com/2011_paper/declercq_temperley_2011.pdf)
- Pre-tonic distribution (chords immediately before I, excluding tonic): IV .396, V .269, bVII .132, bVI .071, VI .050, II .041. Post-tonic distribution: IV .356, V .240, bVII .159, VI .102. VI is avoided pre-tonic. — [de Clercq & Temperley 2011](https://rockcorpus.midside.com/2011_paper/declercq_temperley_2011.pdf)
- Root motion counts (chromatic):
  - +P4 2,266 and -P4 2,220
  - +M2 1,384 and -M2 1,386
  - +m3 410, -m3 454, +M3 326, -M3 412
  - +m2 113, -m2 162, tritone 20
  - Ascending and descending forms are roughly equal, unlike common practice where ascending 4ths dominate.
  - Frequency falls off regularly with circle-of-fifths distance.
  - Source: [de Clercq & Temperley 2011](https://rockcorpus.midside.com/2011_paper/declercq_temperley_2011.pdf)
- Most common trigrams ending on I: IV-V-I, V-IV-I, bVII-IV-I, VI-IV-I, bVII-bVI-I, bIII-bVI-I, II-V-I, bVI-bVII-I, V-VI-I, IV-bVII-I. Pre-tonic V is almost always preceded by IV. — [de Clercq & Temperley 2011](https://rockcorpus.midside.com/2011_paper/declercq_temperley_2011.pdf)
- Co-occurrence clusters:
  - {bVII, bIII, bVI} co-occur, a natural-minor/"flat-side" palette.
  - {II, VI, III} co-occur, a major palette.
  - Source: [de Clercq & Temperley 2011](https://rockcorpus.midside.com/2011_paper/declercq_temperley_2011.pdf)
- 75.8% of chords are major-quality, 23.4% minor, 0.7% diminished and 0.1% augmented. 94.1% are in root position, against about 60% in common practice. — [de Clercq & Temperley 2011](https://rockcorpus.midside.com/2011_paper/declercq_temperley_2011.pdf)
- Hooktheory (TheoryTab DB, "over 40,000" analyses):
  - Root distribution: I .176, IV .12, V .116, vi .075, ii .033.
  - After IV: I .332, V .266, vi .066.
  - After IV-I: V .407, IV .199, vi .101.
  - The API docs note the figures have drifted since publication.
  - Source: [Hooktheory API docs](https://hooktheory.com/api/trends/docs); [Hooktheory Trends](https://www.hooktheory.com/blog/trends-tool/)
- Hooktheory API chord encoding, useful as an internal symbol format:
  - Scale degree digits: `1` = I, `6` = vi.
  - Inversion suffix: `16` = I6. Seventh suffix: `27` = ii7.
  - Borrowed chords: `b7` = bVII. Secondary chords: `5/6` = V/vi.
  - Mode is a separate `scale` parameter (major, minor, dorian, harmonicMinor…).
  - Rate limit 10 requests per 10 s. Source: [Hooktheory API docs](https://hooktheory.com/api/trends/docs)
- McGill Billboard: 742 unique songs, 1958-1991, with chord, meter and structure annotations. A 625-song single-key subset exists (Korzeniowski & Widmer 2017). — [Faraldo 2017 (describing Burgoyne et al. 2011)](https://zenodo.org/records/1154586/files/Faraldo_PhDThesis.pdf)

### Inferences
- Default progression rules for EDM:
  - Generate in Aeolian. Use 4-bar loops, one chord per bar, starting on i.
  - Draw the vocabulary from {i, bVI, bIII, bVII, iv, v}, with bII optional for Phrygian/techno colour.
  - Weight root motions by circle-of-fifths distance, in the spirit of the de Clercq-Temperley table.
  - Keep the V (major, with leading tone) rare unless the "harmonic minor / epic trance" style is selected.
  - i-bVI-bIII-bVII (= vi-IV-I-V in the relative major) is consistent with this palette. No EDM corpus count for it was found.
- Genre presets (theory-consistent, not corpus-verified):

  | Preset | Chords and rhythm | Scale |
  |---|---|---|
  | Techno | 1-2 chords (i drone, or i-bII / i-bVII shuttle), stabs | Minor or Phrygian |
  | Dub techno | A single m7 (or m9) chord, repeated and echoed | Not specified |
  | Deep house | m7/m9/m11 and maj7 loops, often a 2-chord shuttle (e.g. Fm9-Ebmaj7 or ii7-i7) | Dorian / Aeolian |
  | Trance | 4- or 8-bar i-bVI-bIII-bVII family, with an optional major V (harmonic minor) for "epic" lift | Aeolian |
  | Psytrance | Near-static harmony, single-root bass | Phrygian / Phrygian dominant leads |
  | Lo-fi | ii7-V7-Imaj7 / Imaj7-vi7 with 9ths, root-position bass | Not specified |
  | Ambient | Modal drone with slow planing chords | Lydian / Dorian / Aeolian |

- Treat the rock/pop statistics as major-mode priors. Before use for minor-mode EDM, rotate them to the relative minor (I->bIII, vi->i) or replace them with Hooktheory `scale=minor` queries.

### Gaps
- No EDM chord-transition matrix was found. Faraldo's corpora annotate key and pitch-class sets, not Roman numerals. A per-genre transition table would need to be built (e.g. from Hooktheory genre filters or a MIDI loop corpus).
- Synthwave, lo-fi and dub techno: no authoritative source was fetched. The dub techno "single m7 chord" norm and the lo-fi ii-V norm are common knowledge but unsourced here.
- The McGill Billboard chord-transition statistics themselves were not retrieved, only the corpus description.
- Hooktheory genre-filtered (EDM) probabilities need an authenticated API call, which was not done.

---

## 2. Modes and scales by genre, and their character

### Takeaway
EDM's tonal centre is usually defined by a minor tonic triad plus characteristic degrees. Aeolian is by far the most common heptatonic collection, then Dorian, Ionian, Phrygian and Mixolydian. Phrygian bII-to-i (the "descending leading tone") is a distinctive techno/tech-house marker. Phrygian dominant appears in psytrance and some techno and dubstep. Key detection should allow modal labels rather than only major/minor.

### Cited Findings
- In GiantSteps+, heptatonic sets are by far the most common pitch-class cardinality (209 items), then hexatonic and octatonic.
  - Of heptatonic tracks: Aeolian 122, Dorian 22, Ionian 18, Phrygian 9, Mixolydian 5 (non-Aeolian diatonic modes together about 35% of the dataset).
  - Modal quality is often conveyed by 3-6 note subsets: the tonic triad plus characteristic degrees.
  - Source: [Faraldo 2017](https://zenodo.org/records/1154586/files/Faraldo_PhDThesis.pdf)
- Phrygian, chromatic and tritone sets in GiantSteps+:
  - 45 tracks contain the "elementary phrygian set" {0,1,3,7}.
  - 86 contain the b2-to-1 relation, "especially in genres such as tech house and techno."
  - 41 contain the cluster {0,1,2,3}, read as Aeolian with Phrygian cadential resolutions.
  - 12 contain the chromatic set {0-5}.
  - Tritone relations appear in 53 tracks, mostly drum'n'bass and tech house.
  - Source: [Faraldo 2017](https://zenodo.org/records/1154586/files/Faraldo_PhDThesis.pdf)
- Phrygian-major (Phrygian dominant) examples: Mark Broom "M28" (techno), Dubfire "I Feel Speed" (progressive house), Excision "Headbanga" (dubstep). — [Faraldo 2017](https://zenodo.org/records/1154586/files/Faraldo_PhDThesis.pdf)
- The EDM-derived profiles show a "higher presence of the subtonic (b7) in both modalities," indicating Mixolydian/Aeolian over the classical Ionian/harmonic minor. — [Faraldo 2017](https://zenodo.org/records/1154586/files/Faraldo_PhDThesis.pdf)
- Modal continuum used to place loops, ordered by fewest changed notes: Phrygian - Aeolian - Dorian - Mixolydian - Ionian - Lydian. — [House Harmonic Filler](https://www.tenor-conference.org/proceedings/2017/17_Faraldo_tenor2017.pdf)
- Rock modality groups as major (Ionian, Lydian, Mixolydian) and minor (Aeolian, Dorian, Phrygian, harmonic), mostly by tonic-chord quality. Moore says Ionian, Mixolydian, Dorian and Aeolian are the most common rock modes. — [Faraldo 2017](https://zenodo.org/records/1154586/files/Faraldo_PhDThesis.pdf); [de Clercq & Temperley 2011](https://rockcorpus.midside.com/2011_paper/declercq_temperley_2011.pdf)
- Stephenson's rock "palettes" are natural-minor, major/Mixolydian, and "chromatic-minor" (major triads on natural-minor degrees). — [de Clercq & Temperley 2011](https://rockcorpus.midside.com/2011_paper/declercq_temperley_2011.pdf)
- Psytrance modes are minor, Phrygian or Phrygian dominant. — [Melodigging](https://www.melodigging.com/genre/psytrance)
- The minor pentatonic is a reduced natural minor and fits natural-minor progressions. It cannot build full triads except i and bIII. — [Attack Magazine (author reply in comments)](https://www.attackmagazine.com/technique/passing-notes/passing-notes-deep-house-chords/)

### Inferences
- Mode table for an engine, as semitone sets from the tonic, with the characteristic degree in brackets:

  | Mode | Semitone set | Character / main use |
  |---|---|---|
  | Aeolian | {0,2,3,5,7,8,10} [b6] | Default dark minor (techno, trance) |
  | Dorian | {0,2,3,5,7,9,10} [nat 6] | Deep house, soulful, less dark |
  | Phrygian | {0,1,3,5,7,8,10} [b2] | Techno menace |
  | Phrygian dominant | {0,1,4,5,7,8,10} [b2 + major 3] | Psytrance "eastern" leads |
  | Harmonic minor | {0,2,3,5,7,8,11} [nat 7] | Epic trance V chord |
  | Mixolydian | {0,2,4,5,7,9,10} [b7] | Major but hip |
  | Lydian | {0,2,4,6,7,9,11} [#4] | Ambient/cinematic float |
  | Minor pentatonic | {0,3,5,7,10} | Riffs, acid lines |

  Character words are conventional and not sourced.
- Checker rule: the "characteristic degree" of the declared mode must appear at least once in the harmony or melody. Otherwise the mode is ambiguous, and that is acceptable in EDM per Faraldo.

### Gaps
- No quantitative per-subgenre mode distribution was found (e.g. % Phrygian in psytrance). Faraldo's Figure 5.4 breaks mode down by Beatport genre, but the numbers were not extractable from the text.

---

## 3. Voice leading rules, voicing ranges, avoid-notes

### Takeaway
Classical SATB rules apply only selectively to electronic music. The ones worth encoding are:
- Keep common tones.
- Move other voices by the smallest motion.
- Keep upper voices within an octave of each other, and space wide only at the bottom.
- Let the bass own the root, with rootless or upper-structure voicings in the pad/keys.

Parallel 5ths/octaves matter for exposed independent lines. They are idiomatic in power-chord, stab and "chord-memory" planing, which is a genre norm in house/techno.

### Cited Findings
- Core SATB prohibitions:
  - Parallel perfect 5ths/octaves.
  - Hidden (direct) 5ths/octaves between the outer voices, unless the soprano moves by step.
  - Contrary 5ths/octaves.
  - Source: [Intermediate Theory, part-writing errors](https://intmus.github.io/inttheory20-21/10-intro-part-writing/b1-part-writing-errors.html); [SFCM four-part writing](https://sfcm.edu/sites/default/files/sfcm-theory/harmony_supplements/chap_05/chap_05.pdf)
- Spacing: adjacent upper voices within an octave, tenor-bass usually no more than a 12th. Closed spacing = the upper three voices within one octave. Open spacing = soprano-to-tenor span over an octave. — [Intermediate Theory](https://intmus.github.io/inttheory20-21/10-intro-part-writing/b1-part-writing-errors.html); [ChoraleGuide spacing](https://www.choraleguide.com/vl-spacing.php)
- Avoid voice crossing and overlap. Avoid doubling the leading tone. Keep common tones, and move mostly by step. — [Intermediate Theory](https://intmus.github.io/inttheory20-21/10-intro-part-writing/b1-part-writing-errors.html)
- The sources call some of these rules archaic and style-specific. Strictness on hidden 5ths varies by textbook (disagreement). — [Intermediate Theory](https://intmus.github.io/inttheory20-21/10-intro-part-writing/b1-part-writing-errors.html)
- Producer-side voicing:
  - Re-voicing by octave changes the voicing, not the chord.
  - The highest and lowest notes most shape the perceived chord and melody.
  - The bass note decides whether Cm7 is heard as Cm7 or Eb6.
  - Source: [Attack Magazine](https://www.attackmagazine.com/technique/passing-notes/passing-notes-deep-house-chords/)
- Rock harmony is 94.1% root position, so inversions are rare in pop-derived styles. — [de Clercq & Temperley 2011](https://rockcorpus.midside.com/2011_paper/declercq_temperley_2011.pdf)
- The House Harmonic Filler exposes "spread" (close vs. open over octaves), "register/inversion" (folding chords up or down) and legato as the main loop-voicing variation parameters. — [House Harmonic Filler](https://www.tenor-conference.org/proceedings/2017/17_Faraldo_tenor2017.pdf)

### Inferences (encodable, mostly standard theory; verify)
- Voice-leading cost between successive chord voicings = sum of absolute semitone displacements, with an optimal assignment between voices. The generator picks the inversion that minimises it. The checker flags average motion above about 4 semitones per voice in pads.
- Spacing and register rules:
  - Keep intervals below about C3 (MIDI 48) to a 5th or wider. Keep 3rds out of the low register to avoid "low-interval mud". This is the common "low interval limit" rule, unsourced here.
  - Pad/chord layer roughly C3-C5. Bass root in C1-C2. The pad does not double the bass root in the bass register.
- Parallel motion: flag parallel 5ths/8ves only between independent melodic layers (lead vs. bass, counter-melody vs. lead). Whitelist them in "stab/chord-memory/power-chord" mode.
- Drop-2: take a close-position 4-note chord and drop the second voice from the top an octave. This is a standard jazz definition, unsourced in this pass.
- Avoid-notes (jazz convention, unsourced): the natural 11 over major/dominant chords (b9 against the 3rd), and the b6/b13 over minor chords on sustained strong-beat notes. Dorian's nat-6 over m7 is fine.

### Gaps
- No primary source was fetched for drop-2, avoid-note tables or "low interval limits". Open Music Theory pages returned 403. These need a follow-up fetch (e.g. Open Music Theory "Voice leading basics", Levine's Jazz Theory Book).
- No source quantifies how often electronic producers use parallel planing. This is a genre norm, not measured.

---

## 4. Bass line construction in electronic music

### Takeaway
EDM bass is mostly rhythmic, not melodic:
- Root-locked patterns (offbeat, double offbeat, rolling 1/16, octave jump, syncopated one-bar groove) are placed relative to a four-on-the-floor kick, and sidechain/ducking is integral.
- The psytrance bass is usually a single repeated root with velocity variation.
- Acid (303) lines are root/scale notes plus octave jumps. Slides and accents are distributed somewhat randomly and then curated, and odd-length patterns create phasing.

### Cited Findings
- Offbeat bass, double offbeat and 1/16th rolling bass:
  - Offbeat bass is one note on each offbeat, answering the kick. The kick must be short and the bass cut before the next kick transient.
  - Double offbeat plays pairs on 16th steps 3+4 and 7+8.
  - 1/16th rolling bass is a note on every 16th, and sidechain is "mandatory".
  - Source: [Producer School, 5 essential bass patterns](https://theproducerschool.com/blogs/featured-blogs/5-essential-bass-patterns-that-define-hard-house-and-trance-music)
- Octave jump and syncopated groove:
  - Octave jump is 1/8 notes alternating a note and its octave, with slight glide, for an "uplifting, euphoric drop." Low-only is darker.
  - Syncopated groove is one bar of off-grid accented bass, repeated exactly rather than varied.
  - Source: [Producer School](https://theproducerschool.com/blogs/featured-blogs/5-essential-bass-patterns-that-define-hard-house-and-trance-music)
- Psytrance bass:
  - "Rolling 16-32nd notes and is usually a single note most of the way through." — [Gearspace, 303 psytrance-style basslines](https://gearspace.com/threads/303-psytrance-style-basslines.980708/)
  - Patterns often have "no note changes", just rhythmic root patterns with velocity variation. — [KVR, Goa melodies](https://www.kvraudio.com/forum/viewtopic.php?p=7177239)
  - Sidechain to the kick, and tune note starts and lengths precisely. — [Melodigging](https://www.melodigging.com/genre/psytrance)
  - Disagreement/caution: these are forum and practitioner sources. The common "kick + 3 bass 16ths per beat" (K-B-B-B) and triplet "galloping" variants were not confirmed by a fetched source.
- Acid line programming:
  - Fill all 16 steps quickly, then refine; "doesn't have to sound like something a human bassist would play."
  - Use one octave up (as on the original hardware).
  - Scatter slides and accents randomly, then delete the ones you don't like.
  - Shorten to an odd length (15 or 13 steps) for a "woozy" bassline that cycles against the bar, and rotate the pattern step by step.
  - Source: [MusicTech, Chicago-style 303 acid bassline](https://musictech.com/guides/essential-guide/how-to-create-a-chicago-style-acid-house-bassline)
- 303 accent and slide behaviour:
  - Accent is binary (full or none) and also raises filter cutoff. — [Roland, Beyond Acid](https://articles.roland.com/beyond-acid-pushing-the-tb-303-into-new-sonic-territory/); [KVR](https://www.kvraudio.com/forum/viewtopic.php?p=8644925)
  - Emulators may treat overlapping adjacent notes as slides, so a generator must control note overlap explicitly. — [KVR](https://www.kvraudio.com/forum/viewtopic.php?p=8644925)
- Bass and chords should not compete for the root, hence rootless chord voicings over a root bass (low-reliability source). — [jamburgr](https://jamburgr-web.pages.dev/library/deep-house)
- Moore's textural layers are explicit beat, functional bass, melodic, and harmonic filler. In loop music the tonic is defined largely by the bass and beat layers. — [House Harmonic Filler](https://www.tenor-conference.org/proceedings/2017/17_Faraldo_tenor2017.pdf)

### Inferences
- Bass generator rules:
  - Strong-beat bass notes are the chord root in at least about 80% of chord spans. The octave and 5th are allowed, and passing tones only on weak 16ths.
  - Never place a bass note on the kick in offbeat mode.
  - Acid mode:
    - Pitch pool = minor pentatonic of the key, plus b2 or a chromatic neighbour as passing tones.
    - Octave-up notes on about 20-35% of steps, accents on about 25%, slides on about 15-25%.
    - Pattern length 16, with an optional 13/15 polymeter.
    - The ratios are engineering guesses, not sourced.
- Checker:
  - Proportion of bass onsets coinciding with kick onsets: should be 0 in offbeat mode, and the rolling mode requires a sidechain flag.
  - Bass-root agreement with the chord on downbeats.
  - Range: bass fundamental 30-120 Hz, with sub notes about E0-E2. This is unsourced convention.
- Walking bass (lo-fi/jazz): quarter notes, chord tone on beat 1, and a chromatic or scalar approach note on beat 4 into the next root. This is standard jazz pedagogy, unsourced here.

### Gaps
- No authoritative source for the psytrance triplet/"galloping" bass or exact note-per-beat counts.
- No source quantifies the slide, accent and octave densities in classic acid lines.
- No walking-bass source was fetched.

---

## 5. Melody/hook writing rules

### Takeaway
The empirical melodic regularities with research backing are:
- Small intervals dominate.
- Large leaps tend to be followed by a change of direction, but this is better explained as "regression to the mean" (tessitura) than as a rule about leaps.
- EDM leads are short, repeated motifs, often minimal in pitch content.

The remaining hook rules (call-and-response, repetition ratio, note counts) are craft conventions that this pass could not source quantitatively.

### Cited Findings
- von Hippel & Huron (2000, Music Perception 18(1)):
  - Across cultures, a large interval tends to be followed by a direction change.
  - They argue this "post-skip reversal" is a side effect of tessitura constraints.
  - Later work frames it as regression to the mean: after a leap toward the median, melodies tend to continue in the same direction.
  - Source: [Why Do Skips Precede Reversals? (abstract)](https://scilit.com/publications/d61da2e28fb69e0f7a57e22fd7fd1969); [Regression to the mean in music (course notes)](https://shanahdt.github.io/mus-345-445/emp_methods_workshop/regression_in_music.html)
- Skips are more frequent at the extremes of a melody's or instrument's range. — [Jazzomat tutorial, low-skip bias](https://jazzomat.hfm-weimar.de/tutorials/klaus/tutorial_low_skip.html)
- Psytrance/Goa: rely on short motifs, with lead "melodies" often rhythmic patterns on one note. — [Melodigging](https://www.melodigging.com/genre/psytrance); [KVR](https://www.kvraudio.com/forum/viewtopic.php?p=7177239)
- EDM form is "accumulative": loops and riffs pile up into a texturally thick groove rather than presenting a climactic main theme (Spicer, Butler, Garcia). 60% of tracks have a lead melody. — [Faraldo 2017](https://zenodo.org/records/1154586/files/Faraldo_PhDThesis.pdf)
- Semitone motion carries "much narrative power". The descending b2-to-1 is an underused counterpart to the leading tone, prominent in techno and tech house. — [Faraldo 2017](https://zenodo.org/records/1154586/files/Faraldo_PhDThesis.pdf)
- Attack example: a descending top line (E-D-C) can be produced purely by voicing choices in a chord loop, so "melody" can emerge from the top voice of the chords. — [Attack Magazine](https://www.attackmagazine.com/technique/passing-notes/passing-notes-deep-house-chords/)

### Inferences (encodable heuristics; unsourced numbers flagged)
- Motif and repetition:
  - A motif is 1-2 bars with 3-8 notes. A hook phrase is 2-4 bars.
  - Form AA'AB, or call (bars 1-2) / response (bars 3-4) where the response keeps the rhythm and changes the last 1-3 pitches, or answers on a different chord tone.
- Interval and contour rules:
  - Interval distribution target: at least 60-70% steps (0-2 semitones) and at most about 10% leaps over a 5th. After a leap over 4 semitones, prefer the opposite direction unless the leap moves toward the centre of the range.
  - Range: about an octave to a 10th for leads.
  - Contour: an arch or descending shape per phrase, with the phrase's highest note not repeated too often.
  - These thresholds are heuristics, to be calibrated against a corpus.
- Strong-beat tension: chord tones on beats 1 and 3 (or each downbeat) at about 70% or more. Extensions (9, 11, 13) are allowed as "colour tones" on strong beats in deep house/lo-fi. Avoid-notes are allowed only as short passing tones.

### Gaps
- No sourced figures were found for interval distributions in EDM leads, the ideal note counts per hook, or "repetition ratio" targets.
- Huron's *Sweet Anticipation* step/leap statistics were not retrieved.
- An MIR feature-extraction pass over a lead-melody MIDI corpus would be needed to set thresholds.

---

## 6. Tension and release over an arrangement

### Takeaway
In EDM, tension comes mostly from texture, density and timbre rather than harmonic cadence:
- Builds and breaks are often pitch-only (no drums).
- Cadences are largely absent in looped harmony.
- Key changes are rare (about 3.5-5%). When they happen, they are abrupt, at a hypermeter boundary or after an unpitched transition, usually to a closely related key.
- Harmonic tension devices are pedal tones, modal mixture (borrowed major chords in minor), and semitone/tritone colour.

### Cited Findings
- Key changes and loop harmony:
  - Across 1,986 analysed EDM excerpts, 70 (about 3.5%) had structural key changes, mostly to relative, neighbour (5th) or parallel keys.
  - These changes "tend to occur suddenly at the start of a new hypermeter or after an atonal (unpitched) transition," with drastic instrumentation and texture changes.
  - Pivot-chord modulations occur mainly in disco-influenced tracks.
  - Loop harmony has no cadential points.
  - Source: [Faraldo 2017](https://zenodo.org/records/1154586/files/Faraldo_PhDThesis.pdf); [House Harmonic Filler](https://www.tenor-conference.org/proceedings/2017/17_Faraldo_tenor2017.pdf)
- Over 40% of GiantSteps+ tracks have a pitch-only section (break or build-up). Pedal tones are among the listed tonal indicators, under 40% prevalence. — [Faraldo 2017](https://zenodo.org/records/1154586/files/Faraldo_PhDThesis.pdf)
- Rock/EDM directionality is mostly subdominant (I-IV-I) rather than dominant (I-V-I). Faraldo counts both up and down 5th errors as equally "neighbour" for this reason. — [Faraldo 2017](https://zenodo.org/records/1154586/files/Faraldo_PhDThesis.pdf)
- Euphoric trance: a minor key with borrowed major chords. — [KVR](https://www.kvraudio.com/forum/viewtopic.php?p=5515583)
- In deep house loops, the parallel-major borrowed chord is a regular colour. — [House Harmonic Filler](https://www.tenor-conference.org/proceedings/2017/17_Faraldo_tenor2017.pdf)
- Tonal-tension proxy used for loops: the number of distinct pitch classes plus the number of semitone adjacencies in the pc-set. It runs from diatonic (low) to chromatic/jazzy (high). — [House Harmonic Filler](https://www.tenor-conference.org/proceedings/2017/17_Faraldo_tenor2017.pdf)

### Inferences
- Arrangement rules:
  - Hold harmony constant within 8- or 16-bar sections. Change the progression or add extensions at section boundaries.
  - In breakdowns, reduce to a pad plus a pedal root.
  - In builds, keep a pedal bass and raise tension with a rising lead or filter, optionally adding bVI-bVII-(i) or a borrowed major V/IV at the last bar.
  - The drop returns to the main loop.
  - Allow at most one key change per track, only at a section boundary, and only to a relative, 5th-neighbour or parallel key (or up a whole step for a "lift" chorus; that is a pop convention, unsourced).
- Checker: the tension proxy (|pc-set| + semitone count) should be higher in builds than in intros and outros, and lower or equal in breakdowns than in drops. This is a heuristic.

### Gaps
- No quantitative study of tension curves in EDM harmony was found (e.g. Lerdahl tension per section). Studies of tension in EDM tend to focus on timbre, loudness and rhythm (not fetched).
- Note: the "key change up at the drop" folk claim has no corpus support here. Faraldo's data suggests key changes are rare overall.

---

## 7. Note spelling, key signatures and Roman numeral labelling

### Takeaway
Use one letter per scale degree, so a heptatonic scale uses each letter A-G exactly once. Spell chords in stacked thirds (letters skip by one), and choose the key signature that gives the fewest accidentals.

For Roman numerals there are two conventions:
- Diatonic Roman numerals relative to the mode: minor = Aeolian, so VII = bVII.
- Chromatic numerals relative to the tonic regardless of mode (I, bIII, bVI, bVII) with a separate quality flag.

Pop/EDM corpora and APIs prefer the chromatic/flat-prefix style.

### Cited Findings
- Notation conventions:
  - de Clercq & Temperley use chromatic relative roots (I, bII, II, bIII, …, bVII, VII) with quality as separate sub-information. They collapse repeated same-root chords.
  - They treat minor as Aeolian (VII = bVII).
  - Source: [de Clercq & Temperley 2011](https://rockcorpus.midside.com/2011_paper/declercq_temperley_2011.pdf); [Vassar thesis on pop harmony](https://digitallibrary.vassar.edu/sites/default/files/2022-01/Harmony_and_Syntax_in_Contemporary_Pop_Music_0.pdf)
- The House Harmonic Filler uses chromatic notation:
  - A flat before a numeral = a minor or diminished interval above the tonic, and a lowercase "m" after = minor chord quality.
  - This avoids the major/minor binary of diatonic numerals.
  - Source: [House Harmonic Filler](https://www.tenor-conference.org/proceedings/2017/17_Faraldo_tenor2017.pdf)
- Hooktheory notation:
  - `b` prefix for borrowed chords, `/` for secondary chords (`5/6` = V/vi), digits for inversion and seventh.
  - Mode is a separate scale parameter, so "bVII in major" and "VII in Mixolydian" are distinct encodings of the same chord.
  - Source: [Hooktheory API docs](https://hooktheory.com/api/trends/docs)
- Spelling examples:
  - D# minor's 9th and 11th are E# and G# (Attack comment spells the 9th "F (E#)", correctly preferring E#).
  - C minor 7 = C-Eb-G-Bb, and lowering the 3rd and 7th of a maj7 gives the m7.
  - Source: [Attack Magazine](https://www.attackmagazine.com/technique/passing-notes/passing-notes-deep-house-chords/)
- EDM tonics are distributed more evenly across the 12 pitch classes than in guitar-centric pop. Pop clusters on open-chord keys (C, D, E, G, A). This means a generator will often need awkward keys (D#m, G#m, Bbm). — [Faraldo 2017](https://zenodo.org/records/1154586/files/Faraldo_PhDThesis.pdf)

### Inferences (encodable algorithm; standard theory)
- Spelling algorithm:
  - For a mode with tonic letter L, degree k gets the letter (L + k-1) mod 7. Its accidental = target pitch-class minus the natural pitch-class of that letter, in the range -2..+2.
  - Reject if double accidentals appear and an enharmonic tonic gives fewer accidentals. Prefer Eb minor (6 flats) vs. D# minor (6 sharps) by context or style, since they tie.
  - Key signature for mode M on tonic T: find the parent Ionian (Dorian = 2nd degree of parent, Phrygian = 3rd, Lydian = 4th, Mixolydian = 5th, Aeolian = 6th, Locrian = 7th). Use the parent's signature.
- Roman numeral labelling:
  - Uppercase = major, lowercase = minor, ° = diminished, + = augmented. Figures 7 / maj7 / ø7 / 9 / 11 / add9 / sus2 / sus4. Inversion figures 6, 64 (triads) and 65, 43, 42 (sevenths), or slash bass.
  - A chord is "borrowed" when its root or quality is not in the declared mode but is in the parallel mode. Label it with an accidental relative to the major-scale degree (bVI, bVII, iv in major).
  - A secondary dominant X/Y is a major triad or dominant 7th whose root is a perfect 5th above a diatonic chord Y (not the tonic), resolving to Y. A checker labels V/x only if the resolution follows, otherwise "chromatic chord."
- A useful tool: music21's `roman.romanNumeralFromChord` and `Key` objects do spelling-aware RN labelling (doc not fetched in this pass; see Gaps).

### Gaps
- music21 Roman numeral docs were not fetched. The spelling algorithm above is standard theory, not a cited source.

---

## 8. Checkable metrics

### Takeaway
A small metric set covers most of what a checker needs:
- Profile-correlation key detection (Krumhansl-Schmuckler with K-K, Temperley, Aarden-Essen or EDM-specific profiles).
- Scale conformance.
- Chord-tone ratio on strong beats.
- Melodic interval histogram.
- Pc-set tension.
- Repetition/novelty (self-similarity).

EDM-specific profiles exist (Faraldo's EDMA). They beat classical profiles on EDM, but they mislabel major pop, so the checker should choose the profile by genre.

### Cited Findings
- Krumhansl-Kessler profiles (indices 0-11 = semitones above the tonic):
  - Major: 6.35, 2.23, 3.48, 2.33, 4.38, 4.09, 2.52, 5.19, 2.39, 3.66, 2.29, 2.88.
  - Minor: 6.33, 2.68, 3.52, 5.38, 2.60, 3.53, 2.54, 4.75, 3.98, 2.69, 3.34, 3.17.
  - Source: [partitura key_identification source](https://partitura.readthedocs.io/en/latest/_modules/partitura/musicanalysis/key_identification.html); [MuseScore keyfinder.cpp](https://sources.debian.org/src/musescore3/3.2.3+dfsg2-11/libmscore/keyfinder.cpp)
- Temperley / "CBMS" profiles as reproduced in MuseScore (verify against Temperley 1999):
  - Major: 5.0, 2.0, 3.5, 2.0, 4.5, 4.0, 2.0, 4.5, 2.0, 3.5, 1.5, 4.0.
  - Minor: 5.0, 2.0, 3.5, 4.5, 2.0, 4.0, 2.0, 4.5, 3.5, 2.0, 1.5, 4.0.
  - Source: [MuseScore keyfinder.cpp](https://sources.debian.org/src/musescore3/3.2.3+dfsg2-11/libmscore/keyfinder.cpp); [partitura](https://partitura.readthedocs.io/en/latest/_modules/partitura/musicanalysis/key_identification.html)
- Krumhansl-Schmuckler method: correlate the piece's pitch-class duration vector with all 24 rotated profiles, and the maximum is the key. — [partitura](https://partitura.readthedocs.io/en/latest/_modules/partitura/musicanalysis/key_identification.html); [Temperley, What's Key for Key?](https://music.informatics.indiana.edu/courses/I546/pdf/temperley.pdf)
- K-K tends to pick the dominant as tonic.
  - music21 offers SimpleWeights, AardenEssen, BellmanBudge, KrumhanslSchmuckler and TemperleyKostkaPayne weightings, and returns alternates in `Key.alternateInterpretations`.
  - The music21 docs warn that some minor weights are "of uncertain origin."
  - Source: [music21 analysis.discrete docs](https://music21.org/music21docs/moduleReference/moduleAnalysisDiscrete.html); [astroneko K-S in Python](https://astroneko404.github.io/posts/Key-Determination)
- EDM-derived EDMA profile (Faraldo; the median of averaged chromagrams from 925 EDM tracks, normalised; read from Figure 6.5):
  - Major: 1.00, 0.29, 0.50, 0.40, 0.60, 0.56, 0.32, 0.80, 0.32, 0.45, 0.42, 0.39. Note b7 (0.42) is greater than or equal to natural 7 (0.39).
  - Minor: 1.00, 0.31, 0.44, 0.58, 0.33, 0.49, 0.29, 0.78, 0.43, 0.29, 0.53, 0.32. Note b7 (0.53) is much greater than natural 7 (0.32).
  - A further EDMM variant uses a "flat major" profile. It improves EDM results but performs poorly on pop because of parallel-key errors.
  - Source: [Faraldo 2017](https://zenodo.org/records/1154586/files/Faraldo_PhDThesis.pdf)
- Key-detection accuracy on EDM is bounded:
  - On the KeyFinder EDM set, the best tool (Mixed In Key) got about 70% correct. Faraldo's methods reached at most 62% (BGATE2).
  - On GiantSteps, MIREX weighted scores reached about 0.886 (MIK) and 0.877 (BGATE+).
  - Faraldo's four-profile method (major, minor, ambiguous "majmin", …) got 78-80% on the rock/pop sets.
  - Implication: allow "ambiguous" as a legitimate checker output.
  - Source: [Faraldo 2017](https://zenodo.org/records/1154586/files/Faraldo_PhDThesis.pdf); [Faraldo, Jordà, Herrera, A Multi-Profile Method for Key Estimation in EDM (AES 2017)](https://www.aes.org/e-lib/browse.cfm?elib=18775); code: [github.com/angelfaraldo/edmkey](https://www.github.com/angelfaraldo/edmkey)
- Evaluation weighting (MIREX): count correct keys plus partial credit for fifth, relative and parallel errors. Faraldo counts both up and down 5ths as "neighbour" errors for pop and EDM. — [Faraldo 2017](https://zenodo.org/records/1154586/files/Faraldo_PhDThesis.pdf)
- Modal templates: Faraldo et al. place a loop on the Phrygian-to-Lydian axis by the Euclidean distance between its pitch-class profile and stored modal templates (nearest = mode). Chromaticity = distinct pc count plus semitone count. — [House Harmonic Filler](https://www.tenor-conference.org/proceedings/2017/17_Faraldo_tenor2017.pdf)
- Consonance models: Parncutt and Terhardt (psychoacoustic chord consonance), and the Tonal Interval Space of Bernardes et al., which gives a 12-D DFT-based measure of the consonance and perceptual proximity of pitch aggregates. — [House Harmonic Filler (literature review)](https://www.tenor-conference.org/proceedings/2017/17_Faraldo_tenor2017.pdf)

### Inferences (proposed checker suite)
1. **Key and mode check.** Compute a duration-weighted (optionally beat-weighted) pc histogram, correlate it with K-K or Temperley (pop) or EDMA (EDM) profiles, and compare the result to the declared key. Pass if it matches, or if it is the relative, parallel or 5th-neighbour key and the margin between the top two correlations is under 0.05 (report as "ambiguous"). Add a modal-template nearest-neighbour pass for Dorian, Phrygian and Mixolydian.
2. **Scale conformance.** Fraction of note-duration on declared-scale pitch classes. Target at least 0.95 for diatonic presets, and 0.85 or more where chromatic passing tones are allowed (acid, lo-fi).
3. **Chord-tone ratio on strong beats.** For melody notes with onsets on beats 1 and 3 (or every quarter at a 16-step resolution), the share that are chord tones (root, 3rd, 5th, 7th, plus declared extensions). Target at least 0.7. Bass on downbeats should be the chord root at 0.8 or more.
4. **Melodic interval histogram.** Proportion of steps (2 semitones or fewer), skips (3-4) and leaps (5 or more). Also check that the share of leaps followed by a direction change or a return toward the mean is high.
5. **Voice-leading cost.** Total semitone movement between consecutive pad voicings with optimal voice assignment. Flag parallel 5ths/8ves only between independent layers.
6. **Tension and chromaticity.** |pc-set| plus semitone adjacencies per section. Check its monotonicity against the arrangement plan.
7. **Repetition and novelty.** Bar-level self-similarity matrix on (pitch, onset) n-grams. Repetition ratio = fraction of bars identical to an earlier bar. EDM sections are expected to be high, for example 0.5-0.9 (heuristic). Novelty = new 4-gram types per 8 bars. Flag 100% identical over more than 32 bars with no variation layer, and under 30% repetition as "unhooky."
8. **Spelling check.** Every heptatonic-mode note uses its degree's letter, with no letter repeated within a scale or chord stack.
9. **Range and register.** Bass, pad and lead ranges, with no 3rds below about C3 in pads.

### Gaps
- No published thresholds exist for metrics 2-7 specific to EDM. The numbers above are heuristics and should be calibrated on a reference MIDI set (e.g. the house loop corpus or Hooktheory exports).
- The Temperley 1999 profile values were taken from MuseScore's code rather than the paper. Verify them.
- music21 docs were seen only via search snippets.
- The Aarden-Essen and Bellman-Budge profile values were not retrieved.
- Consonance metric formulas (Parncutt, Plomp-Levelt, Tonal Interval Space) were not fetched.
