# Drum patterns, tempos and arrangement conventions for electronic and nearby genres (for PERF-1 generative firmware)

Grid notation used throughout: 16 steps = one 4/4 bar of 16th notes, step 1 = downbeat, steps 1/5/9/13 = beats 1-4, steps 3/7/11/15 = the 8th-note offbeats ("and"s).
`x` = hit, `X` = accented hit, `g` = ghost (low velocity, roughly 20-45 of 127), `o` = open hat, `.` = rest.
**Provenance tags:** [S] = stated in a cited source; [C] = a common practitioner convention written from general domain knowledge and **not verified against a source in this research pass**. The firmware author should treat [C] grids as reasonable defaults to audition, not as authoritative.

## Q1. Defining rhythm patterns per genre, and the microtiming/swing values producers use

### Takeaway
Most of these genres can be built from a few templates: four-on-the-floor (house/techno/trance/EBM/Afro/amapiano/dub techno), broken 2-step (UKG, DnB, breaks, electro), half-time (dubstep, DnB halftime, trip-hop) and backbeat-with-swing (boom bap, lo-fi, synthwave). What sets each genre apart is mostly tempo, swing amount, hat/percussion layer and one or two signature hits. Swing is best encoded the Roger Linn/MPC way: delay every second 16th, where 50% = straight, 54% = "loose but not swung" and 66% = triplet swing.

### Cited findings

**Swing/microtiming (cross-genre)**
- Linn defines swing as the ratio of the first to the second 16th within each 8th. 50% = no swing, and 66% = "perfect triplet swing" (first 16th gets 2/3 of the 8th, second gets 1/3) — [Attack Magazine / Roger Linn, Swing in DAWs](https://attackmagazine.com/technique/passing-notes/daw-drum-machine-swing/2)
- Linn: on straight-16th beats, about 54% "will loosen up the feel without it sounding like swing" — [Attack Magazine (via search excerpt)](https://attackmagazine.com/technique/passing-notes/daw-drum-machine-swing/2)
- Implementation: swing only delays the playback of alternate 16th notes. At 48 PPQN this allows 50, 54, 58, 60, 62, 66, 70 and 75%, and Linn says he "rarely need[s] more swing increments than this". He called it "shuffle" on the LM-1 (1979) — [Attack Magazine, Roger Linn interview p.2](https://attackmagazine.com/features/interview/roger-linn-swing-groove-magic-mpc-timing/2)
- Caveat: on higher-resolution systems the exact timing for a given percentage is not standardised across devices — [synth-diy Yahoo Groups archive](https://synth-diy.org/yg-archives2/group/xl7/message/5869/)
- Encoding formula (my arithmetic): delay of every even 16th = (swing% − 50%) × 2 × (16th duration). At 54% that is 8% of a 16th, at 66% it is 32% of a 16th (= triplet position). This agrees with the Renoise 48-PPQN tick split quoted in [Renoise forum](https://forum.renoise.com/t/swing-variations-of-50-54-58-60-62-66-70-and-75-as-commands/48546).

**Tempo ranges (authoritative baseline + disagreements)**
- Ableton Learning Music: Dub 60-90, Hip-hop 60-100, House 115-130, Techno/trance 120-140, Dubstep 135-145, Drum and bass 160-180 BPM — [Ableton Learning Music](https://learningmusic.ableton.com/make-beats/tempo-and-genre.html)
- Techno is typically 125-140, and hard techno can reach 180+ — [Digital DJ Pool BPM guide](https://digitaldjpool.com/blog/dj-bpm-guide/). This disagrees with Ableton's 140 ceiling, because Ableton lumps techno and trance together and does not cover hard techno.
- Trance is 135-145, and classic late-90s trance sits around 135-138 — [Digital DJ Pool](https://digitaldjpool.com/blog/dj-bpm-guide/)
- Psytrance: one source gives 138-145 and another gives Goa 135-150 — [psytranceguide.com](https://psytranceguide.com); [Digital DJ Pool](https://digitaldjpool.com/blog/dj-bpm-guide/). The sources disagree.
- Dub techno is about 118-128 in catalog data, which is distinct from dub (60-90) — [mixgraph EDM genre chart](https://www.mixgraph.io/tools/edm-genre-chart); [Ableton](https://learningmusic.ableton.com/make-beats/tempo-and-genre.html)

**Drum and bass / jungle**
- Two-step at about 174 BPM: kick on beat 1, snares on 2 and 4, and the second kick pushed late toward the 8th before the second snare (= step 11) — [MusicRadar, 6 jungle & DnB grooves](https://www.musicradar.com/how-to/how-to-program-6-different-jungle-and-drum-n-bass-grooves); [musicproductionwiki](https://musicproductionwiki.com/articles/how-to-make-drum-and-bass). The MusicRadar article body could not be fetched, so this comes from a search excerpt.
- DnB half-time keeps the tempo (172-174) but puts the snare on beat 3 only, so it feels like 86-88 BPM — [violetrecording](https://violetrecording.com/how-to-make-drum-and-bass/) (a production blog of lower authority)
- Jungle: Amen break chopped into slices (kick, snare, ghost snare, open hat, crash) and retriggered. The break's identity comes from its displaced second/late bar, where the kick lands late. Tempos run 165-174 — [MusicRadar, Amen-style break](https://musicradar.com/tuition/tech/how-to-program-an-amen-style-break-637374); [EDMProd jungle guide](https://www.edmprod.com/how-to-make-jungle-music/); [eMastered](https://emastered.com/it/blog/how-to-make-jungle-music)
- Heavy chopping truncates the kick's decay, and a clear snare anchor is needed or "the ear loses the main accent" — [Drumsplit genre guide](https://drumsplit.pages.dev/guides/drum-separation-for-breakbeat-and-jungle) (low-authority site)

**UK garage / 2-step**
- Kick on beat 1 and an 8th after beat 3 (step 11), claps on 2 and 4. A variant uses rimshots on 2 and 4 plus an extra rimshot a 16th before beat 3 (step 8) for syncopation — [MusicRadar UK garage tutorial](https://www.musicradar.com/how-to/uk-garage-tutorial)
- Swing of about 8-12% (in the tutorial's DAW units, not MPC %) for the garage shuffle. Shakers and closed hats can sit slightly off-grid against each other — [MusicRadar](https://www.musicradar.com/how-to/uk-garage-tutorial)
- Open hats halfway through beats 1, 2 and 4 (steps 3, 7, 15), and late 16ths create the swing — [Native Instruments blog, UK garage](https://blog.native-instruments.com/uk-garage-music/)
- To turn four-on-the-floor into 2-step, move kicks off the beats — [The Producer School](https://theproducerschool.com/blogs/featured-blogs/how-to-program-uk-garage-drums-complete-guide-for-producers)

**Reggaeton / dembow**
- Dembow is a 3+3+2 (tresillo) cross-rhythm with syncopation "on every other half-beat", described as "boom-ch-boom-chick". It descends from Steely & Clevie's "Fish Market"/"Pocoman Jam" riddim (1989), Shabba Ranks' "Dem Bow" (1990) and Dennis the Menace's "Pounder (Dub Mix II)" (1990) — [Wikipedia: Dembow](https://en.wikipedia.org/wiki/Dembow)
- Sources disagree on exact step positions. One grid gives the signature snare hits on steps 7 and 15 and notes that without them "the pattern sounds like house or techno". Others give kick on 1 and 3 with snare on the "and"s of 2 and 4 — [writeseen](https://writeseen.com/blog/reggaeton-beat); [UJAM dembow tutorial](https://www.ujam.com/tutorials/how-to-make-a-dembow-groove/); [MusicRadar reggaeton beat building](https://www.musicradar.com/news/beat-building-how-to-produce-a-reggaeton-beat)
- Reggaeton is 90-100 BPM (one source says 85-105). Dominican dembow is faster, at 115-130 — [orphiq: what is dembow](https://orphiq.com/resources/what-is-dembow); [orphiq: reggaeton](https://orphiq.com/resources/what-is-reggaeton)

**Amapiano**
- Roland's walkthrough sets 112 BPM and programs a 4-bar drum pattern. The genre typically runs 110-115 (other sources say 110-120 or 112-118) — [Roland Articles: Creating Amapiano Tracks](https://articles.roland.com/?p=67237); [beatstorapon](https://beatstorapon.com/blog/how-to-make-amapiano-beat/)
- The shaker is the timekeeping backbone: constant 16ths, often 2-3 layers panned apart with different accents, plus an open hat on offbeats — [beatstorapon](https://beatstorapon.com/blog/how-to-make-amapiano-beat/); [Native Instruments amapiano](https://www.native-instruments.com/blogs/blog/amapiano-music)
- The log drum is the bass voice ("the 808 of amapiano"), tuned to the key (root, then fifths). Start with a hit on beat 1 and vary bar 2. Apply the same swing to the log drum as to the drums/percussion — [beatstorapon](https://beatstorapon.com/blog/how-to-make-amapiano-beat/); [RouteNote](https://create.routenote.com/blog/beatmakers-guide-how-to-make-an-amapiano-beat/)

**Footwork / juke**
- About 160 BPM — [DJ TechTools footwork](https://djtechtools.com/2014/04/24/footwork-what-is-it-and-how-do-you-make-it/); [orphiq footwork](https://orphiq.com/resources/what-is-footwork-music)
- Kicks come in dense bursts of consecutive 32nds ("machine-gun"). Sources disagree on triplets: one says never put triplets in the main groove, another says the feel comes from grouping the bar in threes — [beatkey footwork](https://beatkey.pages.dev/how-to-make-footwork-music) vs [Jack Righteous guide](https://jackrighteous.com/es-gb/blogs/guides-using-suno-ai-music-creation/footwork-juke-ai-music-genre-control-guide) (both low authority)
- Pitched 808 kick patterns at 160 BPM (juke technique) — [MusicRadar / Computer Music](https://www.musicradar.com/tuition/tech/how-to-make-a-juke-style-pitched-808-kick-pattern-554083)

### Inferences (grid library; [C] unless marked)

Velocity scale used below: X ≈ 110-127, x ≈ 85-105, g ≈ 25-45.

**TECHNO family**
| Genre | BPM | Swing | Kick | Snare/clap | Hats | Perc / signature |
|---|---|---|---|---|---|---|
| Detroit techno | 125-135 | 50-56% | `x...x...x...x...` | clap `....x.......x...` | CH `x.x.x.x.x.x.x.x.` or 16ths, OH `..o...o...o...o.` | 909/808 toms, rides, syncopated strings/stabs; more swing and melody than Berlin [C] |
| Berlin/minimal | 124-132 | 50-54% | `x...x...x...x...` | sparse clap/rim, e.g. `....x.......x...` or rim `......x.....x..x` | OH offbeat `..o...o...o...o.` | dub chords, small click percussion, microvariation over long loops [C] |
| Hypnotic techno | 128-136 | 50% | `x...x...x...x...` | often no clap; rim `.....x.......x..` | 16th hats with velocity shaping (accent offbeats) | polyrhythmic perc loops (3- or 5-step loops against 16), rumble kick [C] |
| Industrial techno | 130-145 | 50% | `x...x...x...x...` with distorted rumble tails | metallic hits `....x..x....x...` | noisy hats | distortion, reverb-rumble ("kick → reverb → distortion → LPF"), clang perc [C] |
| Hard techno | 145-160+ (to 180+ [S, Digital DJ Pool]) | 50% | `x...x...x...x...` sometimes with 16th kick pickups `x...x...x...x.xx` | clap 2 & 4 | 16th hats, OH offbeat | pounding distorted kick, rave stabs, 8-bar kick rolls [C] |
| Acid (house/techno) | 120-135 | 50-58% (house side swings more) | `x...x...x...x...` | clap 2 & 4 | OH offbeat | TB-303 16th line with accent/slide and per-step filter automation; 909 kit [C] |

**HOUSE family**
| Genre | BPM | Swing | Kick | Clap/snare | Hats | Perc / signature |
|---|---|---|---|---|---|---|
| Chicago house | 118-128 | 54-58% | `x...x...x...x...` | clap `....x.......x...` | OH `..o...o...o...o.`, CH 16ths swung | 808/909, jacking, tambourine 16ths [C] |
| Deep house | 118-124 | 54-62% | `x...x...x...x...` | soft clap/rim 2 & 4 | swung 16th shaker, OH offbeat | rim/conga ghosts `..g..g....g..g..` [C] |
| Tech house | 124-128 | 52-56% | `x...x...x...x...` (bass often on offbeats) | clap 2 & 4, plus extra clap `...........x..x.` as a groove | 16th CH with accent on offbeat, OH offbeat | shaker/perc loops, bassline-led grooves [C] |
| Afro house | 118-124 | 54-58% | `x...x...x...x...` | rim/clap 2 & 4 | shaker 16ths | conga/djembe tresillo `x..x..x.` repeated (steps 1,4,7,9,12,15), toms, call-response perc [C] |
| Amapiano | 110-115 [S] | swung (apply same swing to log drum [S]) | sparse or four-on-floor `x...x...x...x...` | clap/rim on 2 & 4 or off-grid | shaker 16ths [S], OH offbeat [S] | log-drum pitched syncopation e.g. `x..x..x...x..x..` [C]; "piano" chords; often sparse kick [C] |

**TRANCE / PSY**
| Genre | BPM | Kick | Clap | Hats | Bass/signature |
|---|---|---|---|---|---|
| Trance | 135-145 [S] | `x...x...x...x...` | `....x.......x...` | OH `..o...o...o...o.`, CH 16ths | offbeat bass `..x...x...x...x.`, long breakdowns with supersaw pads, snare rolls [C] |
| Psytrance | 138-145 [S] | `x...x...x...x...` (short, tight) | sparse clap 2 & 4 | 16th hats | "KBBB" rolling bass: `.xxx.xxx.xxx.xxx` (bass on the three 16ths after each kick) [C]; full-on/night variants |

**DUB**
| Genre | BPM | Kick | Snare | Hats | Signature |
|---|---|---|---|---|---|
| Dub techno | 118-128 [S] | `x...x...x...x...` (soft) | rim sparse | OH offbeat | offbeat chord stab `..x...x...x...x.` into long feedback delay + reverb, filter drift [C] |
| Dub (reggae) | 60-90 [S] | one drop: `........x.......` (kick + snare/rim together on beat 3 in half-time); steppers: `x...x...x...x...`; rockers: `x.......x.......` [C] | rim with snare on 3 | 8th hats swung ~58-62%, skank on offbeats `..x...x...x...x.` | echo/spring reverb throws, dropouts (mutes) [C] |

**BREAK-BASED / UK**
| Genre | BPM | Kick | Snare | Hats | Signature |
|---|---|---|---|---|---|
| DnB two-step | 170-176 (174 default [S]) | `x.........x.....` [S] | `....x.......x...` [S] | 8th or 16th hats, ghost snares `.g.....g.g....g.` [C] | Reese/sub bass, rolling ghost notes |
| Liquid DnB | 170-174 | as two-step, softer | plus ghost snares | ride/shaker 16ths | jazzy/soul pads, chopped breaks layered low [C] |
| Neurofunk | 172-176 | `x.........x.....` tight | hard snare 2 & 4 | sparse | heavily processed bass "edits" filling gaps between kick and snare [C] |
| Jungle (Amen) | 160-170 (to 174 [S]) | Amen bars 1-2 [C]: `x.x.......xx....` | `....x..x.x..x..x` (with ghosts) | ride 8ths | bar 3/4 displacement [S, MusicRadar]; ragga vocals, sub bass, chopped re-sequenced breaks |
| Breakbeat/breaks | 125-140 | `x.....x...x.....` (funky-drummer-ish) [C] | `....x.......x...` + ghost `.......g.g....g.` | 16th hats swung 54-58% | sampled breaks, nu-skool basslines [C] |
| Dubstep (140 half-time) | 138-142 (135-145 [S]) | `x.........x.....` or `x.....x.........` | `........x.......` (beat 3 only) [C, consistent with half-time definition [S]] | 16th or triplet hats | wobble/LFO bass on 1/8 or 1/8T, sub drops [C] |
| Riddim dubstep | 140-150 | sparse `x...............` | `........x.......` | minimal | repetitive, metallic, gated/stuttered bass in 1/16 or triplets, very minimal melody [C] |
| UK garage / 2-step | 130-138 [C] | `x.........x.....` [S] | clap `....x.......x...` [S], rim `.......x........` [S] | OH `..o...o.......o.` [S], 16th shaker swung (8-12% DAW swing [S], about 58-62% MPC [C]) | chopped/pitched vocals, organ bass; skipping kick [C] |
| Electro | 120-135 (Planet Rock-era about 125-130) [C] | `x......x..x.....` or `x.....x...x..x..` [C] | `....x.......x...` | 16th CH | 808 cowbell/claps, robotic vocoder, syncopated kick (Kraftwerk/"Planet Rock" lineage) [C] |
| Footwork | 158-162 [S] | `x..x..x...x..x..` triplet/3-grouped feel plus 32nd bursts [S] [C grid] | clap/snare on `....x.......x...` (or half-time `........x.......`) [C] | sparse 16ths/rolls | chopped vocal stabs repeated, 808 pitched kicks [S] |

**DOWNTEMPO / HIP-HOP**
| Genre | BPM | Swing | Kick | Snare | Hats | Signature |
|---|---|---|---|---|---|---|
| Boom bap | 85-95 [C] | 54-62% MPC [C] | `x......x..x.....` or `x.....x...x..x..` | `....X.......X...` with ghost `......g......g..` | 8th or 16th hats with velocity alternation (accent downbeats) | dusty sampled breaks, sidechain-free; SP-1200/MPC grit [C] |
| Lo-fi hip-hop | 70-90 | 58-66% or unquantised "drunk" timing [C] | `x......x..x.....` | soft snare/rim `....x.......x...`, often slightly late (5-20 ms) [C] | swung 8th hats, low velocity | vinyl crackle, tape wow, Rhodes chords [C] |
| Trip-hop | 70-95 | 52-58% | `x.......x.x.....` (heavy, slow) | `....x.......x...` or half-time `........x.......` | sparse | slow breaks, cinematic strings, scratches, dub FX [C] |
| Reggaeton (dembow) | 90-100 [S] | 50% | `x...x...x...x...` [C] | snare/rim `...x..x....x..x.` (steps 4, 7, 12, 15) [C; cf. 7/15 signature [S]] | 16th hats | tresillo 3+3+2 [S]; Dominican dembow 115-130 [S] |
| Synthwave/outrun | 80-118 (often 100-118) [C] | 50% | `x.......x.......` (or four-on-floor in outrun) | big gated-reverb snare `....x.......x...` | 8th or 16th CH | LinnDrum/DMX samples, gated reverb, arpeggiated 16th bass [C] |
| Ambient/drone | free or 60-90 | — | often no drums | — | — | long pads, slow evolution; if drums exist, sparse kick `x...............` and textures [C] |
| EBM/industrial | 110-130 [C] | 50% | `x...x...x...x...` (or `x.......x.......`) | snare `....x.......x...` | minimal | sequenced 16th or 8th bass `xxxxxxxxxxxxxxxx` (DAF/Front 242 style), shouted vocals, metallic perc [C] |

**Velocity / ghost habits [C]**
- Hats: alternate accents (e.g. offbeat 8th = 100, 16th "e/a" = 60-75) for drive. Techno and tech house often accent the offbeat open hat strongest.
- Ghost snares in DnB, breaks, boom bap and jungle sit 1-2 16ths before or after backbeats (steps 4, 7, 10, 15 etc.) at velocity 25-45.
- Humanisation: ±2-8 ms random offset plus ±5-10 velocity on hats. Kick stays on grid in 4/4 club genres. Snare is sometimes late in lo-fi and boom bap.

**Fills [C]**
- 4/4 club: at the end of 8/16/32-bar phrases, drop the kick for the last beat or bar, add a snare/clap roll, a reverse cymbal, or a tom run `........x.x.xxxx`.
- DnB/jungle: last 1-2 beats of a 8-bar phrase get chopped-break edits and snare flams.
- Hip-hop: drop the kick or hats for 1 bar ("drop-out") before the hook.

### Gaps
- I could not fetch the article bodies of MusicRadar's "6 jungle & DnB grooves" or Attack Magazine's "Beat Dissected" pages, so most grids marked [C] were not checked against those primary tutorials.
- No reliable source was found for exact swing % per genre (house, garage, boom bap). The ranges given are conventions.
- Footwork grid and triplet usage: sources conflict and are low-authority.
- No source was fetched for EBM, electro, synthwave, trip-hop, riddim, hypnotic or industrial techno specifics. Those rows are entirely [C].
- Exact Amen break transcription was not verified against a source in this pass.

## Q2. Arrangement: track lengths, section bar counts, builds/risers/drops, DJ-friendly intros/outros, mixing tempo ranges

### Takeaway
Club genres are built in 8-bar units grouped into 16/32-bar phrases. DJ tools need 16-32 bars of sparse intro and outro. Builds layer a doubling snare roll, an upward filter/noise sweep and pitch risers over 4-8 bars (sometimes 16), and the drop lands on the downbeat of a new 16/32-bar phrase. Exact per-genre section bar counts are conventions rather than published statistics.

### Cited findings
- Most dance music is phrased in 32-beat (8-bar) units, with 8-, 16- and 32-bar phrases as building blocks. DJs start the incoming intro aligned to the outgoing phrase end — [DJ.Studio phrasing](https://dj.studio/blog/phrasing-dj-mixing); [Cursa DJ mixing foundations](https://cursa.app/en/page/dj-mixing-foundations-bars-phrasing-and-starting-on-the-one)
- "Most DJs need 16 or 32 bars of simple elements at the beginning", and typical intros/outros run 16-32 bars — [Lost Stories Academy, DJ-friendly track](https://loststoriesacademy.com/blogs-and-tutorials/what-makes-a-track-dj-friendly)
- Disagreement: one forum rule of thumb gives 32 counts for hip-hop and 64 for house, while another says house/techno phrases are 8 bars "in the vast majority of cases". Some tracks add an extra bar or a quarter bar at the end of a breakdown — [DJ TechTools forum](https://forum.djtechtools.com/showthread.php?p=714701); [DJ TechTools forum, intros/outros](https://forum.djtechtools.com/t/intros-and-outros/111)
- Energy management across a set — [Mixed In Key, Control the Energy Level](https://mixedinkey.com/Book/Control-The-Energy-Level-Of-Your-DJ-Sets)
- Build-ups: start with a basic snare roll and "double-time it until the end", raising velocity at each step. Automate a filter, pitch and reverb on the roll. Beware clipping — [WA Production, building EDM build-ups](https://blog.waproduction.com/crafting-the-perfect-edm-build-up-techniques-and-tips); [Soundbridge layered build-ups](https://soundbridge.io/make-massive-layered-build-ups)
- Riser: a white-noise oscillator into a low-pass filter, with the cutoff automated to open. Or a resonant high-pass sweep over 4-8 measures immediately before the drop — [MusicRadar synth sweep FX](https://www.musicradar.com/how-to/synth-sweep-fx); [Soundbridge 8-bar EDM buildup](https://soundbridge.io/8-bar-edm-buildup)
- A kick-free breakdown before the build is a tension tool, and a mediocre drop after an excellent build hits harder than an excellent drop with no build — [FaderPro, 3 tips for better drops](https://blog.faderpro.com/techniques/3-tips-for-better-edm-drops/)
- Academic: the drop is detectable from content features plus temporal structure (ISMIR 2014, using SoundCloud timed comments as weak labels), and drops vary widely in how they are realised — [Yadati et al., ISMIR 2014](https://archives.ismir.net/ismir2014/paper/000297.pdf)
- EDMFormer (arXiv 2603.08759, 2026) introduces EDM-98 (98 professionally annotated EDM tracks), with labels intro, build-up, drop, breakdown, outro, end. It reports that pop-trained segmenters handle EDM poorly — [arXiv 2603.08759](https://arxiv.org/abs/2603.08759)
- Twente thesis "Finding 'The Drop'": a hand-labelled EDM drop dataset plus ML models — [University of Twente](https://purl.utwente.nl/essays/82333)

### Inferences (arrangement templates, [C])
Bar counts are for 4/4 at genre tempo, and every section is a multiple of 8.

| Genre group | Typical length | Template (bars) | Energy curve |
|---|---|---|---|
| Techno (all) | 6-8 min | Intro 32 (kick+hats) → Groove A 32 → Groove B 32 (add perc/lead) → Breakdown 16-32 (kick out) → Build 8 → Drop/Main 32-64 → Outro 32 | plateau with one dip; changes every 8/16 bars by adding/removing one layer |
| House/tech/deep | 5-7 min | Intro 16-32 → Groove 32 → Break 16 → Build 8 → Drop 32 → Groove 32 → Outro 16-32 | gentle waves |
| Trance | 6-9 min (club) | Intro 32 → Groove 32 → Breakdown 32-64 (melody/pads) → Build 16 (snare roll 1/4→1/8→1/16→1/32) → Drop 32 → Outro 32 | one big valley + peak |
| Psytrance | 7-9 min | Intro 32 → Groove 64 → short Break 8-16 → Drop 64 → Outro 32 | relentless, high plateau |
| DnB/jungle | 5-6 min | Intro 32 (DJ) → Build 8-16 → Drop 1 32 (often 64 bars at 174 counted as 32 "half-time" bars) → Breakdown 16-32 → Build 8 → Drop 2 32 → Outro 32 | two drops |
| Dubstep/riddim | 4-5 min | Intro 16-32 → Build 8 → Drop 16-32 → Break 16 → Build 8 → Drop 2 16-32 → Outro 16 | two peaks |
| UKG/2-step | 4-6 min | Intro 16 → Verse 16 → Hook 16 → Break 8 → Hook 16 → Outro 16 | song-like |
| Breaks/electro | 5-6 min | Intro 16-32 → Groove 32 → Break 16 → Build 8 → Main 32 → Outro 16-32 | moderate waves |
| Amapiano/Afro house | 6-8 min | Intro 32 → Groove 32 → log-drum/piano entry 32 → Break 16 → Main 32-64 → Outro 32 | long slow builds; amapiano "piano" drops of the log drum |
| Hip-hop/lo-fi/trip-hop/reggaeton | 2-4 min | Intro 4-8 → Verse 16 → Hook 8 → Verse 16 → Hook 8 → Bridge 8 → Outro 4-8 | song form; lo-fi = loop 2-4 bars repeated with slight variations |
| Synthwave/EBM | 4-6 min | Intro 8-16 → Verse 16 → Chorus 16 → ... → Outro 8-16 | song form |
| Ambient/drone | 5-20 min | no grid; slow 32-64-bar layer fades | flat/slow-rising |
| Dub | 3-6 min | riddim versions; mutes/drop-ins every 4-8 bars | rhythmic subtraction rather than build |

- Build construction recipe (encodable): over N = 8 bars, snare roll density 1/4 for bars 1-2 → 1/8 bars 3-4 → 1/16 bars 5-6 → 1/32 bars 7-8 (doubling per 2 bars), velocity ramp 60→127, HPF on drums 20 Hz→300-500 Hz, white-noise riser LPF opening plus volume ramp, pitch riser +12 semitones. Last beat = silence or reverse cymbal, then crash + kick on bar 1 of the drop. This is consistent with the "double-time until the end" description [S], but the exact schedule is a convention.
- DJ mixing tempo ranges: house/techno sets usually stay within about ±6-8% of the track's original BPM (pitch fader limits) [C]. DnB stays 170-176 [C]. Genre bridges commonly exploit half/double time: 87↔174 (hip-hop↔DnB), 70↔140 (trip-hop/dub↔dubstep/grime) [C].

### Gaps
- No published statistics were found on section bar counts per genre (e.g. from Beatport-scale analysis). EDM-98 annotations might provide this but were not inspected.
- Butler's "Unlocking the Groove" (2006) was not accessed in this pass. It is the authoritative musicological source on EDM layering and phrase structure and should be checked.
- No source found for typical track lengths per genre. The table values are conventions.

## Q3. What makes each genre recognisable versus generic (signature elements)

### Takeaway
A generator stops sounding generic when it encodes one or two "must-have" cues per genre: the specific displaced hit (UKG skip kick, dembow steps 7/15, DnB step-11 kick, dubstep beat-3 snare), the right percussion bed (amapiano shakers + log drum, Afro conga tresillo, psy KBBB bass), the right swing, and the right tempo.

### Cited findings
- Dembow: without the step-7/step-15 hits "the pattern sounds like house or techno" — [writeseen](https://writeseen.com/blog/reggaeton-beat)
- Amapiano: the log drum as the bass voice, and the shaker as the timing backbone — [beatstorapon](https://beatstorapon.com/blog/how-to-make-amapiano-beat/); [Native Instruments](https://www.native-instruments.com/blogs/blog/amapiano-music)
- UKG: kick off the beat plus swing on 16ths/shakers — [MusicRadar UKG](https://www.musicradar.com/how-to/uk-garage-tutorial); [Native Instruments UKG](https://blog.native-instruments.com/uk-garage-music/)
- Jungle: the displaced late kick bar of the Amen — [MusicRadar Amen](https://musicradar.com/tuition/tech/how-to-program-an-amen-style-break-637374)
- Footwork: 160 BPM plus machine-gun kick bursts — [beatkey](https://beatkey.pages.dev/how-to-make-footwork-music); [DJ TechTools](https://djtechtools.com/2014/04/24/footwork-what-is-it-and-how-do-you-make-it/)
- DnB half-time: snare on 3 at 174 — [violetrecording](https://violetrecording.com/how-to-make-drum-and-bass/)

### Inferences (signature checklist, [C])
- Detroit techno: swing + melodic chords; Berlin minimal: sparse microdetail, dub chords; hypnotic: polymetric loop lengths (3/5/7 steps); industrial: rumble + distortion; hard techno: >145 BPM, distorted kick.
- Acid: 303 accent/slide line + filter movement (the drums are generic).
- Chicago: jacking 909/808, swung hats; deep: soft, swung, rim ghosts; tech house: groove bass + extra claps + shakers.
- Trance: offbeat bass + long breakdown + snare-roll build; psytrance: KBBB bass at 140-145.
- Dub techno: offbeat chord stab into delay; dub: one-drop + mutes/echo throws.
- DnB: step-11 kick, 174; liquid: soft ghosts + ride; neuro: bass edits; jungle: Amen chops + 160-170.
- Breaks: syncopated non-4/4 kick around 130; electro: 808 syncopated kick + cowbell; footwork: 160, triplet-ish kicks, vocal chops.
- Dubstep: beat-3 snare at 140 + wobble; riddim: ultra-sparse + repetitive stutter.
- UKG: skip kick (step 11) + swing; boom bap: MPC swing + loud backbeat; lo-fi: heavy swing/late snare + low velocities; trip-hop: slow heavy breaks.
- Synthwave: gated reverb snare 2 & 4; EBM: 16th sequenced bass + 4/4; Afro house: conga tresillo; amapiano: log drum + shaker; reggaeton: dembow 4/7/12/15.

### Gaps
- No authoritative comparative source was found that lists signature elements across genres. This section is largely convention.

## Q4. Openly usable data sources

### Takeaway
The Groove MIDI Dataset (CC BY 4.0) is the main open drum-performance source. It contains real microtiming and velocity from human drummers in mostly acoustic styles (useful for humanisation models, less so for EDM grids). EDM-structure datasets (EDM-98, the ISMIR 2014 drop work) exist but are small.

### Cited findings
- Groove MIDI Dataset (Magenta): about 13.6 h, 1,150 MIDI files, 22,000+ measures, 10 drummers (80%+ of the duration from professionals), recorded on a Roland TD-11 to a click. Each file carries a per-file "style" (primary/secondary) label. CC BY 4.0. A different Magenta index page gives 13.4 h / 1,134 files — [Magenta Groove dataset](https://magenta.tensorflow.org/datasets/groove); [Magenta datasets index](https://magenta.tensorflow.org/datasets); [Gillick et al., arXiv 1905.06118](https://arxiv.org/pdf/1905.06118v2.pdf)
- E-GMD: 444 h of audio across 43 drum kits, based on GMD — [Magenta](https://magenta.tensorflow.org/datasets/groove)
- Loaders exist in mirdata — [mirdata groove_midi](https://mirdata.readthedocs.io/en/latest/_modules/mirdata/datasets/groove_midi.html); Zenodo record — [Zenodo 3957999](https://zenodo.org/record/3957999)
- EDM-98 annotated structure dataset (intro, build-up, drop, breakdown, outro) — [arXiv 2603.08759](https://arxiv.org/abs/2603.08759)
- Drop detection with SoundCloud timed comments — [ISMIR 2014](https://archives.ismir.net/ismir2014/paper/000297.pdf)

### Inferences
- GMD is best used to learn per-instrument timing offsets and velocity distributions (humanisation tables), not genre grids for EDM. Its styles are drummer genres such as rock, funk, jazz, latin, hip-hop (from memory of the paper; label list not verified this pass).
- For firmware, a compact encoding per genre: {bpm_min, bpm_max, bpm_default, swing_pct, 16-step bitmasks per voice (kick/snare/clap/CH/OH/perc1/perc2), per-step velocity arrays, ghost mask, fill templates, arrangement template (section list with bar counts, multiples of 8), build recipe parameters}.

### Gaps
- I could not extract the GMD style-label list or counts. The PDF was not text-readable in this environment, so `info.csv` in the dataset download should be checked.
- No open dataset of electronic-genre drum MIDI (with genre labels for techno/house/DnB etc.) was found in this pass.
- No Beatport-scale structural analysis paper was located.
