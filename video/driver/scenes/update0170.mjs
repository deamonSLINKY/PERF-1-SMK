// The 0.17.0 update video in one take: DISCO 03 (G Dorian, 122) from the power-on (it comes back to the piece played last) to the credits;
// everything new since the 0.15.0 video over that one piece, then the three new genres, then TRIP under the web pages and the credits.
// Set beforehand, out of sight (prepare0170): DISCO 03 played and stopped (the place to come back to), THEORY as the visualizer, a set list.
import { text, ring, readout, at, tapB, tapK, turnK, holdB } from "./lib.mjs";
import { prepare0170 } from "./probe0170.mjs";

const T = (s, sec) => at(s, sec * 1000);
async function settingsWalk(s, pos = 0) {                    // hold HOME: the unit's settings, FILTERS first; SELECT walks them
  await holdB(s, "HOME", 900, "HOME"); await s.wait(300);
  if (pos) { ring(s, "knob:SELECT", 1.4, "SELECT"); await s.turn("SELECT", pos, 160); }
  await s.wait(500);
}
function headerBox(s) {                                      // how much of the header's corner is the piece's box (cream): low while it names an effect
  const fb = new Uint16Array(s.mem.buffer, s.ex.web_fb(), 57600);
  let n = 0, tot = 0;
  for (let y = 5; y < 17; y++) for (let x = 5; x < 31; x++) { const v = fb[y * 240 + x], p = ((v >> 8) | (v << 8)) & 0xffff; tot++; if (((p >> 11) & 31) > 24 && ((p >> 5) & 63) > 48) n++; }
  return n / tot;
}
async function viz(s) { ring(s, "btn:HOME", 0.9, "HOME"); await s.tap("HOME", 60); await s.wait(120); await s.tap("HOME", 60); await s.wait(700); }
async function step(s, w, kind, note, oct) {                   // a bass step: its white key, then Step, Note, Octave
  ring(s, "key:" + [0, 2, 4, 6, 7, 9, 11, 12, 14, 16, 18, 19, 21, 23, 24, 26][w], 1.0, "KEY");
  await s.tapKey("w" + w); await s.wait(300);
  await s.turn("KNOB1", -5, 120); await turnK(s, "KNOB1", kind, 200, "KNOB1"); await s.wait(250);
  if (note !== undefined) { await s.turn("KNOB2", -8, 120); await turnK(s, "KNOB2", note, 200, "KNOB2"); await s.wait(250); }
  if (oct !== undefined) { await s.turn("KNOB3", -3, 120); await turnK(s, "KNOB3", oct, 200, "KNOB3"); await s.wait(250); }
}

export default {
  seed: 7, fps: 30,
  prepare: prepare0170,

  async run(s) {
    s.start(30);
    // ---- 1. power on: back where it was; Song form
    text(s, { label: "0.17.0", title: "Power on where you left off", body: "It comes back to the piece you played last, exactly as stored. Here: disco 3.", chips: [] });
    await T(s, 4.5);
    readout(s, "DISCO 03", "G DORIAN · 122", 3.5);
    await T(s, 8.5);
    text(s, { label: "PIECES THAT SOUND FINISHED", title: "Song or Loop", body: "A piece moves through its own drum patterns now, a fill in the bar before each change, the way a track does.", chips: [] });
    await T(s, 15.6); readout(s, "FILL", "THE BAR BEFORE", 2);
    await T(s, 17.8); readout(s, "PATTERN 2", "EVERY 8 BARS", 3);
    // ---- 2. THEORY and the circle moves
    await T(s, 21);
    text(s, { label: "THEORY", title: "The circle of fifths", body: "Drawn like a theory book: the key's signature, the chord playing, the next one ringed, the arrow between.", chips: ["tap", "*HOME", "twice"] });
    await s.wait(500); await viz(s);
    await T(s, 30);
    text(s, { label: "THE CIRCLE MOVES", title: "Secondary dominants", body: "A chord a fifth above the next can become its dominant: here G minor turns G7 to lead into C. Disco, lofi, garage, amapiano.", chips: [] });
    await T(s, 33.1); readout(s, "V OF THE NEXT", "G7 → C7", 4);
    await T(s, 37.5);
    text(s, { label: "THE CIRCLE MOVES", title: "Phrases end at home", body: "Each 8-bar phrase lands the pluck on the chord's root. Borrowed chords in synthwave, and no diminished chord by accident.", chips: [] });
    // ---- 3. your own progression, fills, Pedal
    await T(s, 42.5);
    text(s, { label: "YOUR OWN PROGRESSION", title: "Eight chord slots", body: "Hold EDIT: the Progression page, the chords you just heard. Eight slots from the scale, each its own bars.", chips: ["hold", "*EDIT"] });
    await holdB(s, "EDIT", 700, "EDIT"); await s.wait(2500);
    text(s, { label: "YOUR OWN PROGRESSION", title: "Fills on the black keys", body: "Eleven ready progressions: five round the circle of fifths, six from the songbooks. A bar a chord here.", chips: ["*KNOB3", "then", "*BLACK KEY"] });
    await turnK(s, "KNOB3", -1, 200, "KNOB3"); await s.wait(600);
    readout(s, "ii V I I", "THE JAZZ TURNAROUND", 4);
    await tapK(s, 1, 120, "BLACK KEY"); await s.wait(9000);
    text(s, { label: "YOUR OWN PROGRESSION", title: "A chord type a slot", body: "Each slot can be its own type: triads, sevenths, ninths, sus, elevenths.", chips: ["*KNOB2"] });
    readout(s, "11TH", "SLOT 1: THE NEW TYPE", 3.5);
    await turnK(s, "KNOB2", 7, 220, "KNOB2"); await s.wait(7000);
    text(s, { label: "YOUR OWN PROGRESSION", title: "The whole circle", body: "I IV vii iii vi ii V I, round the circle of fifths, in the piece's scale. The keys light the chord.", chips: ["*BLACK KEY"] });
    readout(s, "THE WHOLE CIRCLE", "8 CHORDS", 4);
    await tapK(s, 10, 120, "BLACK KEY"); await s.wait(8000);
    text(s, { label: "YOUR OWN PROGRESSION", title: "Pedal", body: "The bass, or the pad, holds the key while the chords move above it. White keys 9 and 10.", chips: ["*KEY 9"] });
    readout(s, "PEDAL: BASS", "G UNDER EVERY CHORD", 5);
    await tapK(s, 14, 120, "KEY 9"); await s.wait(9500);
    readout(s, "BASS FOLLOWS", "", 2.5);
    await tapK(s, 14, 120, "KEY 9"); await s.wait(1500);
    text(s, { label: "YOUR OWN PROGRESSION", title: "Back to the walk", body: "Play on Walk gives the chords back to the genre. Saved with the piece when you store it.", chips: ["*KNOB4"] });
    await s.wait(400);
    await turnK(s, "KNOB4", -1, 200, "KNOB4"); readout(s, "WALK", "", 2.5); await s.wait(2500);
    // ---- 4. Bass steps
    text(s, { label: "BASS STEPS", title: "Sixteen steps of bass", body: "LFO five times. It opens with the bar the line just played. A white key picks a step.", chips: ["tap", "*LFO", "x5"] });
    for (let i = 0; i < 5; i++) { await tapB(s, "LFO", "LFO"); await s.wait(220); }
    await s.wait(2500);
    text(s, { label: "BASS STEPS", title: "Accent, glide, octave", body: "Each step a rest, a note, an accent or a glide; its note counted in the scale above the chord, so the line follows the chords.", chips: ["*KEY", "*KNOB1", "*KNOB2", "*KNOB3"] });
    readout(s, "STEP 3: ACCENT", "AN OCTAVE UP", 3.5); await step(s, 2, 2, undefined, 2); await s.wait(2500);
    readout(s, "STEP 7: GLIDE", "TO THE 5TH", 3.5); await step(s, 6, 3, 4); await s.wait(2500);
    readout(s, "STEP 11: ACCENT + GLIDE", "THE OCTAVE", 3.5); await step(s, 10, 4, 7); await s.wait(2500);
    readout(s, "STEP 15: ACCENT", "THE 3RD", 3.5); await step(s, 14, 2, 2); await s.wait(2500);
    text(s, { label: "BASS STEPS", title: "Saved with the piece", body: "In backups and in shared pieces too. GitHub issue #6.", chips: [] });
    await s.wait(7000);
    // ---- 5. FILTERS and SCREAM
    text(s, { label: "FILTERS AND SCREAM", title: "Filter types", body: "Hold HOME for the unit's settings. FILTERS: low, high, band, notch or peak for the pad, the bass and the acid.", chips: ["hold", "*HOME"] });
    await settingsWalk(s, 0); await s.wait(800);
    readout(s, "PAD: BAND", "", 4); await turnK(s, "KNOB1", 2, 250, "KNOB1"); await s.wait(6000);
    readout(s, "PAD: LOW", "", 2); await turnK(s, "KNOB1", -2, 120, "KNOB1"); await s.wait(1500);
    text(s, { label: "FILTERS AND SCREAM", title: "SCREAM", body: "The bass and acid filters pushed into self-oscillation. Both saved with each piece now.", chips: ["*SELECT", "*KNOB1", "*KNOB2", "*KNOB4"] });
    ring(s, "knob:SELECT", 1.2, "SELECT"); await s.turn("SELECT", 1, 200); await s.wait(800);
    await turnK(s, "KNOB4", 1, 200, "KNOB4"); await s.wait(400);
        readout(s, "DRIVE 60 · SCREAM 80", "THE BASS", 5);
    await turnK(s, "KNOB1", 60, 60, "KNOB1"); await turnK(s, "KNOB2", 80, 60, "KNOB2"); await s.wait(7000);
    await s.turn("KNOB2", -100, 15); await s.turn("KNOB1", -100, 15); await s.wait(600);
    await s.tap("FX"); await s.wait(900);                                    // out of the settings
    // ---- 6. punch-ins
    text(s, { label: "PUNCH-INS", title: "Effects on chosen tracks", body: "FILTER, DELAY and PHASE on just the tracks you pick: the drums stay punchy.", chips: ["tap", "*FX", "hold", "*KEY"] });
    await tapB(s, "FX", "FX"); await s.wait(800);
    for (const [k, name] of [[14, "FILTER"], [16, "DELAY"], [18, "PHASE"]]) {
      ring(s, "key:" + k, 4.2, "KEY"); readout(s, name, "HELD", 4);
      s.key(k, true); await s.wait(4000); s.key(k, false); await s.wait(700);
    }
    await s.tap("FX"); await s.wait(900);
    // ---- 7. Auto FX
    text(s, { label: "AUTO FX", title: "Effects played for you", body: "Timed to the phrase, picked for the genre and the section. Gentle, medium, wild.", chips: ["hold", "*HOME", "turn", "*SELECT"] });
    await settingsWalk(s, 4); await s.wait(600);
    readout(s, "AUTO FX: MEDIUM", "", 3); await turnK(s, "KNOB1", 2, 250, "KNOB1"); await s.wait(900);
    readout(s, "EVERY 4 BARS", "", 2.5); await turnK(s, "KNOB2", -1, 250, "KNOB2"); await s.wait(1500);
    await s.tap("FX"); await s.wait(800); await s.tap("HOME"); await s.wait(800);
    text(s, { label: "AUTO FX", title: "On the phrase lines", body: "Each effect ends as the next phrase starts. Its name shows in the header as it plays.", chips: [] });
    for (let ms = 0; ms < 25000 && headerBox(s) > 0.45; ms += 100) await s.wait(100);   // until the header names an Auto FX effect
    readout(s, "AUTO FX: PHASER", "", 3);
    await s.wait(4500);
    // ---- 8. the set list
    text(s, { label: "THE SET LIST", title: "Sixteen pieces, any genre", body: "GLO twice. Your order; PRESETS and the auto-DJ follow it, and Mix glides the tempo between them.", chips: ["tap", "*GLO", "twice"] });
    await tapB(s, "GLO", "GLO"); await s.wait(150); await tapB(s, "GLO", "GLO"); await s.wait(5500);
    await s.tap("HOME"); await s.wait(800);
    // ---- 9. three new genres
    text(s, { label: "THREE NEW GENRES", title: "Industrial", body: "Detuned, harsh drums, fills on the off-beats, huge tempo swings.", chips: ["hold", "*GLO", "turn", "*PRESETS"] });
    ring(s, "btn:GLO", 3.4, "GLO"); ring(s, "knob:PRESETS", 3.0, "PRESETS");
    s.press("GLO"); await s.wait(400); await s.turn("PRESETS", 6, 250); await s.wait(400); s.release("GLO");
    readout(s, "INDUSTRIAL", "", 3); await s.wait(9000);
    text(s, { label: "THREE NEW GENRES", title: "Dark forest", body: "Fast, dark psytrance: the rolling bass, the squelch, the night.", chips: ["hold", "*GLO", "turn", "*PRESETS"] });
    ring(s, "btn:GLO", 1.8, "GLO"); ring(s, "knob:PRESETS", 1.4, "PRESETS");
    s.press("GLO"); await s.wait(400); await s.turn("PRESETS", 2, 250); await s.wait(400); s.release("GLO");
    readout(s, "FOREST", "", 3); await s.wait(9000);
    text(s, { label: "THREE NEW GENRES", title: "Psytrance", body: "From cosmic psy-ambient into full-on trance and back. Nineteen genres in all.", chips: ["hold", "*GLO", "turn", "*PRESETS"] });
    ring(s, "btn:GLO", 1.6, "GLO"); ring(s, "knob:PRESETS", 1.2, "PRESETS");
    s.press("GLO"); await s.wait(400); await s.turn("PRESETS", -1, 250); await s.wait(400); s.release("GLO"); await s.wait(600);
    ring(s, "knob:PRESETS", 1.6, "PRESETS"); await s.turn("PRESETS", -2, 300);
    readout(s, "PSY", "Y01 · COSMIC PSY-AMBIENT", 3.5); await s.wait(8000);
    // ---- 10. TRIP, the thank-you, beyond the box (the web pages over the music), the credits
    text(s, { label: "AND", title: "TRIP", body: "A new visualizer: a kaleidoscope you fall through, thrown forward by the kick.", chips: ["tap", "*HOME", "twice"] });
    await viz(s); ring(s, "knob:SELECT", 1.2, "SELECT"); await s.turn("SELECT", -1, 200); await s.wait(6000);
    s.mark("web", { badge: "NEW", img: "web/library.png", label: "THE LIBRARY", title: "Share pieces, play other people's", body: "The Pieces page: hear one in the browser, add it to your FM-1, send yours in. deamonslinky.github.io/PERF-1/pieces", dur: 11 });
    await s.wait(11000);
    s.mark("web", { img: "web/midilearn.png", label: "A KNOB FOR ANY SETTING", title: "MIDI learn, any controller", body: "Click a setting, move a knob: it goes to the FM-1 with your map. Ready maps for some controllers, the Launch Control XL 3 first.", dur: 12 });
    await s.wait(12000);
    s.mark("web", { img: "web/install.png", label: "THE INSTALL PAGE", title: "Full speed, newer FM-1s", body: "Undoes M-VAVE V16's 12% slowdown. Works on the newer flash chip. Back to V15 or V16 any time.", dur: 10 });
    await s.wait(10000);
    text(s, { label: "YOUR IDEAS", title: "Most of this came from you", body: "THEORY (#2), the fills and Pedal (#3), the newer FM-1s (#5), bass steps (#6), SCREAM, the punch-ins, the new genres: your issues and comments.", chips: [] });
    await s.wait(7000);
    s.mark("thanks", { dur: 12, title: "Thank you.",
      body: ["For the awesome feedback, the comments and the ideas.", "It makes me want to keep pushing harder.", "Keep the suggestions coming."],
      foot: "GitHub issues · the Firmware Hub · the comments below · deamonslinky.github.io/PERF-1", sign: "deamonSLINKY" });
    await s.wait(12000);
    s.mark("credits", {});
    await s.wait(22000);
  },
};
