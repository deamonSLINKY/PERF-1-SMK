/* SPDX-License-Identifier: GPL-3.0-only
 * PERF-1 (deamonSLINKY): the SMK-37's own controls (2026-11-08). Included by ui.c in the BOARD_SMK37 build, before input().
 *
 * PADS. On the SMK the 37 keys are only notes. A page that uses the FM-1's keys as buttons gets them from the pads, by
 * itself (smk_bkeys: the FM-1 keys "held" by the pads, bit k = key k, 0 = F3; input() treats them as the FM-1 its keys):
 *   STEPS  the sequencer, the step pages, the gate, SAMPLE: the 16 white keys = steps (top row 1-8, bottom row 9-16)
 *   ROWS   the Set list, the Progression page, SCALE (SEL) held: top row white keys 1-8, bottom row black keys 1-8
 *   KEYS   the FX keys, GLOBE held (the FM-1's mix map): keys 1-16 in order; > keys 17-27, < back
 * Everywhere else the pads are the mode KNOB BANK picks:
 *   PERFORM (the start) top row: breakdown, build, drop, next chord, fill, freeze, auto-DJ, next piece; bottom row: kick,
 *           snare, clap, closed hat, open hat, perc (with velocity), all drums muted or back (pads_in's bank A)
 *   MIX     top row: mute kick, hats, snare, perc, bass, acid, pad, pluck; bottom row: solo the same.
 *           >: scenes 1-4 (tap recalls, hold stores), reset all, undo, tap tempo, all drums; the auto-DJ, its length,
 *           new pieces (the FM-1's mix map keys)
 *   CHORDS  top row: the chord on the scale's degrees I-VII, at once (bank E)
 *   NOTES   the pluck in key, low at the bottom left (bank B); > the bass, the acid; < back
 *   FOLLOW  the FOLLOWERS (amb.h EN_FOL_*): top row F1 root, F2 third, F3 fifth, F4 colour (each on or off; the first turns them on),
 *           FOLLOWERS on / off, Style (Swell, Pulse, Drift), Colour (octave, 7th, 9th, 6th, sus4), Evolve (off, slow, faster).
 *           Bottom row: the Progression page's fills 1-8 (the circle's five, Steven's first three); > fills 9-11, then Move of
 *           F2-F4 (Common, Follow, Hold), F1 (Hold the key's root, Follow), Register, Glide, Rate. Kept with the piece (SAVE).
 *           > again: CALL AND RESPONSE: Call on / off, its channel, Answer (Harmony, Echo, Steer, Swell), Leave room.
 * The pads light in the layout's colour, white while held; MIX shows the mutes (red) and solos (white).
 *
 * FADERS. FADER BANK: A levels (drums, bass, acid, pad), B the four FX knobs (filter, crush, throw, wash), C shape (motion,
 * evolve, acid density, drift). A fader takes over once it moves (after a bank change too). Its LED: lit off A.
 * WHEELS. On the voice the keys play: pitch bends it (two semitones), mod gives it a vibrato (up to half a semitone).
 * KEYS. All 37 play, with velocity: the middle 27 as the FM-1's keys (jam_down_v), the outer ten (C3-E3, G#5-C6) the same
 * way here. Pads and notes: velocity 30 at the softest (the factory pad curve starts high: soft hits came out near 1). */
#define SMK_KEY_OFS 5                            /* keybed key 5 = F3, the FM-1's key 0 */
static int jam_notes(int i, uint8_t *out);       /* (below, with the JAM page) */
enum { SP_KEYS, SP_STEPS, SP_ROWS, SP_PERFORM, SP_MIX, SP_CHORDS, SP_NOTES, SP_FOLLOW };
static const char *const SP_NAME[] = {"KEYS", "STEPS", "SLOTS/FILLS", "PERFORM", "MIX", "CHORDS", "NOTES", "FOLLOW"};
#define SP_LAST SP_FOLLOW
static const uint8_t SP_MUTE_K[8] = {0, 2, 4, 7, 9, 12, 14, 18}, SP_SOLO_K[8] = {1, 3, 5, 8, 10, 13, 15, 25};
static const int8_t SP_MIX2_K[16] = {19, 20, 21, 22, 23, 6, 11, 16, 17, 24, 26, 27, -1, -1, -1, -1};
static struct {
    uint8_t page, lay, mode, voice, led_lay, led_page, led_m, led_s, led_sec;
    uint32_t sb_prev, held_prev, led_held;
} sp = {0, SP_PERFORM, SP_PERFORM, 0, 0xFF, 0xFF, 0, 0, 0xFF, 0, 0, 0xFFFFFFFFu};

static int sp_vel(int v) { return 30 + v * 97 / 127; }   /* the softest pad or key: 30 */

static int sp_layout(void)
{
    if (plat_buttons() >> B_SEL & 1u)
        return SP_ROWS;
    if (ui.fx_menu)
        return ui.fx_page == 1 || ui.fx_page == FXP_SAMPLE + 1 ? SP_STEPS : SP_KEYS;
    if (plat_buttons() >> B_GLO & 1u)
        return SP_KEYS;
    if (ui.view == V_SEQ || ui.view == V_ACSTEP || ui.view == V_BLSTEP || ui.view == V_CMSTEP || ui.view == V_STEPS)
        return SP_STEPS;
    if (ui.view == V_SETL || ui.view == V_PROG)
        return SP_ROWS;
    return sp.mode;
}

static int sp_key(int lay, uint32_t pad)          /* the FM-1 key a pad is in a key layout, -1 none */
{
    if (lay == SP_STEPS)
        return WHITE_KEY[pad];
    if (lay == SP_ROWS)
        return pad < 8u ? WHITE_KEY[pad] : BLACK_KEY[pad - 8u];
    if (lay == SP_MIX)
        return sp.page ? SP_MIX2_K[pad] : pad < 8u ? SP_MUTE_K[pad] : SP_SOLO_K[pad - 8u];
    if (lay == SP_KEYS) {
        pad += sp.page ? 16u : 0u;
        return pad < (uint32_t)NKEYS ? (int)pad : -1;
    }
    return -1;
}
static int sp_is_keys(int lay) { return lay == SP_KEYS || lay == SP_STEPS || lay == SP_ROWS || lay == SP_MIX; }
/* pads_in's notes: PERFORM bank A (top row its pads 9-16, bottom row 1-8), CHORDS bank E (top row I-VII), NOTES bank B-D
 * (the bottom row the lowest eight tones) */
static int sp_note(int lay, uint32_t pad)
{
    if (lay == SP_PERFORM)
        return PADS_BASE + (int)(pad < 8u ? pad + 8u : pad - 8u);
    if (lay == SP_CHORDS)
        return pad < 7u ? PADS_BASE + 64 + (int)pad : -1;
    if (lay == SP_NOTES)
        return PADS_BASE + 16 * (1 + sp.voice) + (int)(pad < 8u ? pad + 8u : pad - 8u);
    return -1;
}

static void sp_notes_off(void)                    /* NOTES: the pads held let go (a mode or voice change) */
{
    uint32_t p;
    for (p = 0; p < 16u; p++)
        if (sp.held_prev >> p & 1u && sp.held_prev != 0xFFFFFFFFu) {
            const int n = sp_note(SP_NOTES, p);
            if (n >= 0)
                pads_in(0, n, 0);
        }
}

/* ---- CALL AND RESPONSE (Steven's issue #3: his guitar, through a Bento's MIDI out, calls; PERF answers) ------------------
 * Notes on the Call channel are calls: they play no voice. A call starts with its first note and ends a beat after the last
 * note let go. Answer: Harmony, the note (snapped to the scale) is F4's for the rest of the chord (the followers sound it);
 * Echo, the call played back on the acid in key from the next beat, up a fifth, an octave up, mirrored or reversed in turn;
 * Steer, a call moves the chord one step round the circle (a fifth down), a call on the key's root takes it home; Swell,
 * each note swells the followers. Leave room: the pluck and acid 40% down while a call plays. The settings are the
 * player's, kept in the set list's spare byte (glob.sl_x[0]): bits 0-3 the channel (0 = channel 2), 4 on, 5-6 Answer,
 * 7 Leave room. */
enum { CA_HARMONY, CA_ECHO, CA_STEER, CA_SWELL, CA_N };
#define CR_MAX 16
static struct {
    uint8_t n, held, active, ewait, ekind, eplay;
    uint8_t note[CR_MAX], vel[CR_MAX], eout[CR_MAX];
    uint16_t at[CR_MAX], len[CR_MAX];
    uint16_t eon, edone;                          /* the echo's notes sounding, and played */
    uint32_t t0, last, et0, ebar, echoed;         /* (echoed: notes echoed since power-on, for the tests) */
} cr;
static int call_ch(void) { return (int)((glob.sl_x[0] + 1u) & 15u); }   /* the channel, 0-based */
static int call_on(void) { return glob.sl_x[0] >> 4 & 1u; }
static int call_ans(void) { return glob.sl_x[0] >> 5 & 3u; }
static void call_load(void) { en_follow(EN_FOL_CALLPC, 0); en_follow(EN_FOL_DUCK, 0); }
static uint32_t call_beat_ms(void) { const int t = proj.par[P_TEMPO]; return 60000u / (uint32_t)(t > 20 ? t : 120); }
static int call_snap(int n)                       /* the nearest note in the scale */
{
    static const int8_t D[7] = {0, -1, 1, -2, 2, -3, 3};
    const uint32_t pool = proj.pool | 1u;
    int k;
    for (k = 0; k < 7; k++) {
        const int m = n + D[k], rel = ((m - proj.par[P_ROOT]) % 12 + 12) % 12;
        if (pool >> rel & 1u)
            return m;
    }
    return n;
}
static void call_echo_stop(void)
{
    int k;
    for (k = 0; k < CR_MAX; k++)
        if (cr.eon >> k & 1u)
            en_note(EN_V_ACID, cr.eout[k], 0);
    cr.eon = 0;
    cr.eplay = cr.ewait = 0;
}
static void call_steer(int first)                 /* a step round the circle, or home */
{
    int st[12], m = en_scale(proj.pool, st), k, d = 0;
    const int root = proj.par[P_ROOT];
    if (m > 7)
        m = 7;
    for (k = 0; k < m; k++)
        if ((root + st[k]) % 12 == en_chord_root)
            d = k;
    d = first % 12 == root % 12 ? 0 : m == 7 ? (d + 3) % 7 : (d + 1) % m;
    en_chord_deg(d);
}
static int call_in(uint32_t st, uint32_t n, uint32_t v)
{
    const uint32_t ty = st & 0xF0u, now = plat_ms();
    int k;
    if (!call_on() || (int)(st & 15u) != call_ch() || (ty != 0x90u && ty != 0x80u))
        return 0;
    if (ty == 0x90u && v) {
        const int a = call_ans();
        if (!cr.active) {                         /* a new call: the echo gives way */
            call_echo_stop();
            cr.active = 1;
            cr.n = 0;
            cr.t0 = now;
            if (glob.sl_x[0] >> 7 & 1u)
                en_follow(EN_FOL_DUCK, 40);
            if (a == CA_STEER)
                call_steer((int)n);
        }
        if (cr.n < CR_MAX) {
            cr.note[cr.n] = (uint8_t)n;
            cr.vel[cr.n] = (uint8_t)(v < 40u ? 40u : v);
            cr.at[cr.n] = (uint16_t)(now - cr.t0 > 60000u ? 60000u : now - cr.t0);
            cr.len[cr.n] = 0xFFFFu;              /* (held) */
            cr.n++;
        }
        cr.held++;
        if (a == CA_HARMONY)
            en_follow(EN_FOL_CALLPC, call_snap((int)n) % 12 + 1);
        else if (a == CA_SWELL)
            en_follow(EN_FOL_SWELL, 1);
    } else {
        for (k = cr.n - 1; k >= 0; k--)
            if (cr.note[k] == n && cr.len[k] == 0xFFFFu) {
                cr.len[k] = (uint16_t)(now - cr.t0 - cr.at[k] > 60000u ? 60000u : now - cr.t0 - cr.at[k]);
                break;
            }
        if (cr.held)
            cr.held--;
    }
    cr.last = now;
    return 1;
}
static void call_tick(void)
{
    const uint32_t now = plat_ms(), beat = call_beat_ms();
    int k;
    if (cr.active && !cr.held && now - cr.last > beat) {   /* the call is over: a beat with nothing held */
        cr.active = 0;
        en_follow(EN_FOL_DUCK, 0);
        if (call_ans() == CA_ECHO && cr.n) {
            for (k = 0; k < cr.n; k++)
                if (cr.len[k] == 0xFFFFu)
                    cr.len[k] = (uint16_t)beat;
            cr.ewait = 1;
            cr.ebar = en_bars;
        }
    }
    if (cr.ewait && (!en_playing || (en_bars != cr.ebar && en_bars % 4u == 0u))) {   /* the echo starts on the next beat */
        const int first = cr.note[0];
        uint32_t total = 0;
        cr.ewait = 0;
        cr.eplay = 1;
        cr.et0 = now;
        cr.eon = cr.edone = 0;
        for (k = 0; k < cr.n; k++) {
            int m = cr.note[k];
            if (cr.at[k] + cr.len[k] > total)
                total = cr.at[k] + cr.len[k];
            m = cr.ekind == 0 ? call_snap(m + 7) : cr.ekind == 1 ? m + 12 : cr.ekind == 2 ? call_snap(2 * first - m) : m;
            while (m < 36)
                m += 12;
            while (m > 84)
                m -= 12;
            cr.eout[k] = (uint8_t)m;
        }
        if (cr.ekind == 3)                        /* reversed: the last note first */
            for (k = 0; k < cr.n; k++) {
                const uint32_t e = cr.at[k] + cr.len[k];
                cr.at[k] = (uint16_t)(total - e);
            }
        cr.ekind = (uint8_t)((cr.ekind + 1) % 4u);
    }
    if (cr.eplay) {
        const uint32_t t = now - cr.et0;
        for (k = 0; k < cr.n; k++) {
            const uint32_t on = cr.at[k], off = on + (cr.len[k] < 60u ? 60u : cr.len[k] > beat ? beat : cr.len[k]);
            if (!(cr.edone >> k & 1u) && t >= on) {
                en_note_v(EN_V_ACID, cr.eout[k], cr.vel[k]);
                cr.eon |= (uint16_t)(1u << k);
                cr.edone |= (uint16_t)(1u << k);
                cr.echoed++;
            }
            if (cr.eon >> k & 1u && t >= off) {
                en_note(EN_V_ACID, cr.eout[k], 0);
                cr.eon &= (uint16_t)~(1u << k);
            }
        }
        if (cr.edone == (uint16_t)((1u << cr.n) - 1u) && !cr.eon)
            cr.eplay = 0;
    }
}
static void call_pad(uint32_t p)                  /* FOLLOW's third page, the bottom row */
{
    static const char *const AN[CA_N] = {"HARMONY", "ECHO", "STEER", "SWELL"};
    static char b[8];
    uint8_t *x = &glob.sl_x[0];
    switch (p) {
    case 8:
        *x ^= 0x10;
        if (!call_on()) {
            call_echo_stop();
            cr.active = 0;
            en_follow(EN_FOL_CALLPC, 0);
            en_follow(EN_FOL_DUCK, 0);
        }
        b[0] = 'C', b[1] = 'H', b[2] = ' ', b[3] = (char)(call_ch() + 1 >= 10 ? '1' : '0' + call_ch() + 1);
        b[4] = call_ch() + 1 >= 10 ? (char)('0' + call_ch() + 1 - 10) : 0, b[5] = 0;
        say(call_on() ? "CALL: ON" : "CALL: OFF", call_on() ? b : "");
        break;
    case 9:
        *x = (uint8_t)((*x & 0xF0u) | ((*x & 15u) + 1u) % 16u);
        b[0] = 'C', b[1] = 'H', b[2] = ' ', b[3] = (char)(call_ch() + 1 >= 10 ? '1' : '0' + call_ch() + 1);
        b[4] = call_ch() + 1 >= 10 ? (char)('0' + call_ch() + 1 - 10) : 0, b[5] = 0;
        say("CALL CHANNEL", b);
        break;
    case 10:
        *x = (uint8_t)((*x & ~0x60u) | (uint32_t)((call_ans() + 1) % CA_N) << 5);
        en_follow(EN_FOL_CALLPC, 0);
        say("ANSWER", AN[call_ans()]);
        break;
    case 11:
        *x ^= 0x80;
        if (!(*x & 0x80u))
            en_follow(EN_FOL_DUCK, 0);
        say("LEAVE ROOM", *x & 0x80u ? "ON" : "OFF");
        break;
    default:
        return;
    }
    project_globals();
}
static void call_led(uint32_t p, uint8_t *r, uint8_t *g, uint8_t *b)
{
    static const uint8_t AC[CA_N][3] = {{0, 70, 110}, {110, 60, 0}, {110, 0, 50}, {60, 110, 0}};
    *r = *g = *b = 0;
    if (p == 8u) {
        if (cr.active)
            *r = *g = *b = 200;
        else
            *g = call_on() ? 140 : 10, *r = call_on() ? 0 : 30;
    } else if (p == 9u) {
        *r = 40, *g = 40, *b = 40;
    } else if (p == 10u) {
        *r = AC[call_ans()][0], *g = AC[call_ans()][1], *b = AC[call_ans()][2];
    } else if (p == 11u) {
        *r = glob.sl_x[0] & 0x80u ? 100 : 15, *g = glob.sl_x[0] & 0x80u ? 60 : 10;
    }
}

/* ---- FOLLOW: the followers' settings (a piece's: project.c keeps them) ------------------------------------------------ */
/* [0] bits 0-3 the voices on, 4-5 Style, 6 the followers on; [1] Move (2 bits a voice); [2] Colour (bits 0-2), Evolve (3-4),
 * Register (5-6); [3] Glide 0..100; [4] Rate (bits 0-1), bit 7: set */
static uint8_t sfol[PX_NFOL];
static const uint8_t SFOL_DEF[PX_NFOL] = {0x07, EN_FM_HOLD | EN_FM_COMMON << 2 | EN_FM_COMMON << 4 | EN_FM_COMMON << 6,
                                          1 | 1 << 3 | 1 << 5, 40, 1 | 0x80};
static void sfol_send(void)
{
    en_follow(EN_FOL_MOVE, sfol[1]);
    en_follow(EN_FOL_COLOUR, sfol[2] & 7);
    en_follow(EN_FOL_EVOLVE, sfol[2] >> 3 & 3);
    en_follow(EN_FOL_REG, sfol[2] >> 5 & 3);
    en_follow(EN_FOL_GLIDE, sfol[3]);
    en_follow(EN_FOL_STYLE, sfol[0] >> 4 & 3);
    en_follow(EN_FOL_RATE, sfol[4] & 3);
    en_follow(EN_FOL_ON, (sfol[0] & 0x0F) | (sfol[0] & 0x40 ? 0x80 : 0));
}
void ui_fol_snapshot(uint8_t *v)
{
    int k;
    for (k = 0; k < PX_NFOL; k++)
        v[k] = sfol[k];
    v[PX_NFOL - 1] |= 0x80;
}
void ui_fol_apply(void)
{
    uint8_t v[PX_NFOL];
    int k;
    if (!project_fol_get(v))
        for (k = 0; k < PX_NFOL; k++)
            v[k] = SFOL_DEF[k];
    for (k = 0; k < PX_NFOL; k++)
        sfol[k] = v[k];
    sfol_send();
}
static int sfol_cycle(int v, int n) { return (v + 1) % n; }
static void sfol_pad(uint32_t p)                  /* a FOLLOW pad pressed */
{
    static const char *const VN[4] = {"F1 ROOT", "F2 THIRD", "F3 FIFTH", "F4 COLOUR"}, *const ST[3] = {"SWELL", "PULSE", "DRIFT"},
                             *const CO[EN_FOL_NCOL] = {"OCTAVE", "7TH", "9TH", "6TH", "SUS4"}, *const EV[3] = {"OFF", "SLOW", "FASTER"},
                             *const MV[EN_NFM] = {"FOLLOW", "COMMON", "HOLD"}, *const RG[3] = {"LOW", "MID", "HIGH"},
                             *const RT[EN_FOL_NRATE] = {"1/4", "1/8", "1/8 DOTTED", "1/16"};
    static const uint8_t GL[5] = {0, 20, 40, 70, 100};
    static char b[16];
    int v, k;
    if (p < 4u) {                                 /* a voice: on or off; while the followers are off, on and the followers with it */
        if (!(sfol[0] & 0x40))
            sfol[0] |= (uint8_t)(0x40 | 1u << p);
        else
            sfol[0] ^= (uint8_t)(1u << p);
        en_follow(EN_FOL_ON, (sfol[0] & 0x0F) | (sfol[0] & 0x40 ? 0x80 : 0));
        say(VN[p], sfol[0] >> p & 1u ? "ON" : "OFF");
        return;
    }
    switch (p) {
    case 4:
        sfol[0] ^= 0x40;
        en_follow(EN_FOL_ON, (sfol[0] & 0x0F) | (sfol[0] & 0x40 ? 0x80 : 0));
        say("FOLLOWERS", sfol[0] & 0x40 ? "ON" : "OFF: THE PAD");
        return;
    case 5:
        v = sfol_cycle(sfol[0] >> 4 & 3, 3);
        sfol[0] = (uint8_t)((sfol[0] & ~0x30) | v << 4);
        en_follow(EN_FOL_STYLE, v);
        say("STYLE", ST[v]);
        return;
    case 6:
        v = sfol_cycle(sfol[2] & 7, EN_FOL_NCOL);
        sfol[2] = (uint8_t)((sfol[2] & ~7) | v);
        en_follow(EN_FOL_COLOUR, v);
        say("COLOUR", CO[v]);
        return;
    case 7:
        v = sfol_cycle(sfol[2] >> 3 & 3, 3);
        sfol[2] = (uint8_t)((sfol[2] & ~0x18) | v << 3);
        en_follow(EN_FOL_EVOLVE, v);
        say("EVOLVE", EV[v]);
        return;
    }
    if (sp.page == 2) {                           /* the bottom row, third page: call and response */
        call_pad(p);
        return;
    }
    if (!sp.page) {                               /* the bottom row: fills 1-8 */
        pg_fill((int)p - 8);
        return;
    }
    if (p < 11u) {                                /* fills 9-11 */
        pg_fill((int)p - 8 + 8);
        return;
    }
    switch (p) {
    case 11:                                      /* F2-F4's Move, together */
        v = sfol_cycle(sfol[1] >> 2 & 3, EN_NFM);
        sfol[1] = (uint8_t)((sfol[1] & 3) | v << 2 | v << 4 | v << 6);
        en_follow(EN_FOL_MOVE, sfol[1]);
        say("F2-F4 MOVE", MV[v]);
        break;
    case 12:                                      /* F1: hold the key's root, or follow the chords' */
        v = (sfol[1] & 3) == EN_FM_HOLD ? EN_FM_FOLLOW : EN_FM_HOLD;
        sfol[1] = (uint8_t)((sfol[1] & ~3) | v);
        en_follow(EN_FOL_MOVE, sfol[1]);
        say("F1 ROOT", v == EN_FM_HOLD ? "HOLDS THE KEY" : "FOLLOWS");
        break;
    case 13:
        v = sfol_cycle(sfol[2] >> 5 & 3, 3);
        sfol[2] = (uint8_t)((sfol[2] & ~0x60) | v << 5);
        en_follow(EN_FOL_REG, v);
        say("REGISTER", RG[v]);
        break;
    case 14:
        for (k = 0, v = 0; k < 5; k++)
            if (GL[k] <= sfol[3])
                v = k;
        v = GL[(v + 1) % 5];
        sfol[3] = (uint8_t)v;
        en_follow(EN_FOL_GLIDE, v);
        b[0] = (char)('0' + v / 50), b[1] = '.', b[2] = (char)('0' + v / 5 % 10), b[3] = ' ', b[4] = 'S', b[5] = 0;
        say("GLIDE", b);
        break;
    case 15:
        v = sfol_cycle(sfol[4] & 3, EN_FOL_NRATE);
        sfol[4] = (uint8_t)((sfol[4] & ~3) | v | 0x80);
        en_follow(EN_FOL_RATE, v);
        say("PULSE RATE", RT[v]);
        break;
    }
}
static void sfol_led(uint32_t p, uint8_t *r, uint8_t *g, uint8_t *b)
{
    static const uint8_t VC[4][3] = {{110, 40, 0}, {0, 80, 60}, {0, 50, 110}, {90, 0, 100}};   /* F1-F4 */
    static const uint8_t VAL[5][3] = {{30, 30, 30}, {0, 80, 40}, {0, 40, 110}, {100, 60, 0}, {100, 0, 60}};   /* a setting's value 0-4 */
    const int act = sfol[0] & 0x40;
    int v = 0;
    if (p < 4u) {
        const int on = sfol[0] >> p & 1u;
        *r = (uint8_t)(on ? VC[p][0] * (act ? 2 : 1) / 2 + (act ? 30 : 0) : VC[p][0] / 8);
        *g = (uint8_t)(on ? VC[p][1] * (act ? 2 : 1) / 2 : VC[p][1] / 8);
        *b = (uint8_t)(on ? VC[p][2] * (act ? 2 : 1) / 2 : VC[p][2] / 8);
        return;
    }
    if (p == 4u) {
        *r = act ? 0 : 40, *g = act ? 140 : 0, *b = 0;
        return;
    }
    if (p >= 8u && sp.page == 2) {
        call_led(p, r, g, b);
        return;
    }
    if (p >= 8u && (!sp.page || p < 11u)) {       /* the fills */
        *r = 70, *g = 30, *b = 0;
        return;
    }
    v = p == 5u ? sfol[0] >> 4 & 3 : p == 6u ? (sfol[2] & 7) : p == 7u ? sfol[2] >> 3 & 3 : p == 11u ? sfol[1] >> 2 & 3 :
        p == 12u ? ((sfol[1] & 3) == EN_FM_HOLD) : p == 13u ? sfol[2] >> 5 & 3 : p == 14u ? sfol[3] / 25 : sfol[4] & 3;
    v = v > 4 ? 4 : v;
    *r = VAL[v][0], *g = VAL[v][1], *b = VAL[v][2];
}

static void sp_leds(int lay, uint32_t held)
{
    uint32_t p;
    const uint8_t sec = (uint8_t)(en_sec | en_frozen << 2 | ui.dj_on << 3) ^
                        (uint8_t)(lay == SP_FOLLOW ? sfol[0] * 3u + sfol[1] * 5u + sfol[2] * 7u + sfol[3] + sfol[4] * 11u +
                                                     glob.sl_x[0] * 13u + cr.active * 17u : 0u);
    if (held == sp.led_held && lay == sp.led_lay && sp.page == sp.led_page && ui.trk_m == sp.led_m && ui.trk_s == sp.led_s &&
        sec == sp.led_sec)
        return;                                  /* (only on a change: padled_flush sends nothing then either) */
    sp.led_held = held;
    sp.led_lay = (uint8_t)lay;
    sp.led_page = sp.page;
    sp.led_m = ui.trk_m;
    sp.led_s = ui.trk_s;
    sp.led_sec = sec;
    for (p = 0; p < 16u; p++) {
        uint8_t r = 0, g = 0, b = 0;
        if (held >> p & 1u) {
            r = g = b = 255;
        } else if (lay == SP_STEPS) {
            r = 60, g = 24;
        } else if (lay == SP_ROWS) {
            if (p < 8u)
                r = 60, g = 24;
            else
                r = 30, b = 60;
        } else if (lay == SP_KEYS) {
            if (sp_key(lay, p) >= 0)
                g = sp.page ? 50 : 14, b = sp.page ? 14 : 60;
        } else if (lay == SP_MIX) {
            if (sp.page)
                g = sp_key(lay, p) >= 0 ? 40 : 0, b = sp_key(lay, p) >= 0 ? 20 : 0;
            else if (p < 8u)
                r = ui.trk_m >> p & 1u ? 140 : 20, g = ui.trk_m >> p & 1u ? 0 : 30, b = ui.trk_m >> p & 1u ? 0 : 20;
            else
                r = g = b = ui.trk_s >> (p - 8u) & 1u ? 160 : 12;
        } else if (lay == SP_PERFORM) {
            if (p < 8u) {                        /* the sections: the one playing brighter */
                static const uint8_t C[8][3] = {{0, 40, 90}, {90, 50, 0}, {120, 0, 30}, {60, 60, 0}, {0, 60, 30},
                                                {0, 70, 90}, {70, 0, 90}, {40, 40, 40}};
                int lit = (p == 0 && en_sec == EN_SEC_BREAK) || (p == 1 && en_sec == EN_SEC_BUILD) ||
                          (p == 5 && en_frozen) || (p == 6 && ui.dj_on);
                r = (uint8_t)(C[p][0] * (lit ? 2 : 1)), g = (uint8_t)(C[p][1] * (lit ? 2 : 1)), b = (uint8_t)(C[p][2] * (lit ? 2 : 1));
            } else if (p < 14u) {
                r = 90, g = 10;                  /* the drums */
            } else if (p == 14u) {
                r = 40, g = 10, b = 10;
            }
        } else if (lay == SP_CHORDS) {
            if (p < 7u)
                r = 20, g = 60, b = 40;
        } else if (lay == SP_NOTES) {
            static const uint8_t VC[3][3] = {{0, 60, 70}, {70, 30, 0}, {40, 70, 0}};   /* pluck, bass, acid */
            r = VC[sp.voice][0], g = VC[sp.voice][1], b = VC[sp.voice][2];
        } else if (lay == SP_FOLLOW) {
            sfol_led(p, &r, &g, &b);
        }
        plat_pad_rgb((int)p, r, g, b);
    }
}

static uint32_t smk_bkeys(void)
{
    const uint32_t sb = plat_smk_buttons(), held = plat_smk_pads_held(), hits = plat_pad_hits();
    const uint32_t sbn = (sb & 3u) == 3u ? 0u : sb & ~sp.sb_prev;   /* (KNOB BANK + FADER BANK: the test's way out, nothing here) */
    const uint32_t prev = sp.held_prev == 0xFFFFFFFFu ? 0u : sp.held_prev, up = prev & ~held;
    uint32_t out = 0, p;
    int lay;
    sp.sb_prev = sb;
    if (sbn >> SB_KNOB_BANK & 1u) {               /* KNOB BANK: the next pad mode */
        if (sp.mode == SP_NOTES)
            sp_notes_off();
        sp.mode = (uint8_t)(sp.mode == SP_LAST ? SP_PERFORM : sp.mode + 1);
        sp.page = 0;
        say("PADS", SP_NAME[sp.mode]);
    }
    lay = sp_layout();
    if (lay != sp.lay) {
        if (sp.lay == SP_NOTES)
            sp_notes_off();
        if (sp_is_keys(lay) != sp_is_keys(sp.lay) || lay == SP_STEPS || lay == SP_ROWS)
            sp.page = 0;
        sp.lay = (uint8_t)lay;
    }
    if (sbn >> SB_RIGHT & 1u || sbn >> SB_LEFT & 1u) {   /* < >: the page (KEYS, MIX) or the voice (NOTES) */
        const int right = (int)(sbn >> SB_RIGHT & 1u);
        if (lay == SP_KEYS || lay == SP_MIX) {
            sp.page = (uint8_t)right;
            say("PADS", lay == SP_MIX ? (right ? "SCENES, DJ" : "MUTES, SOLOS") : right ? "KEYS 17-27" : "KEYS 1-16");
        } else if (lay == SP_FOLLOW) {
            static const char *const PG[3] = {"FILLS 1-8", "FILLS 9-11, MOVE ..", "CALL AND RESPONSE"};
            sp.page = (uint8_t)(right ? (sp.page < 2u ? sp.page + 1u : 2u) : sp.page ? sp.page - 1u : 0u);
            say("PADS: FOLLOW", PG[sp.page]);
        } else if (lay == SP_NOTES) {
            static const char *const VN[3] = {"PLUCK", "BASS", "ACID"};
            sp_notes_off();
            sp.voice = (uint8_t)(right ? (sp.voice + 1) % 3u : (sp.voice + 2) % 3u);
            say("PADS: NOTES", VN[sp.voice]);
        }
    }
    if (lay == SP_FOLLOW) {
        for (p = 0; p < 16u; p++)
            if (hits >> p & 1u)
                sfol_pad(p);
    } else if (sp_is_keys(lay)) {
        for (p = 0; p < 16u; p++)
            if (held >> p & 1u) {
                const int k = sp_key(lay, p);
                if (k >= 0)
                    out |= 1u << k;
            }
    } else {
        for (p = 0; p < 16u; p++) {
            const int n = sp_note(lay, p);
            if (n < 0)
                continue;
            if (hits >> p & 1u)
                pads_in(1, n, sp_vel(plat_pad_vel((int)p)));
            if (up >> p & 1u && lay == SP_NOTES)
                pads_in(0, n, 0);
        }
    }
    sp.held_prev = held;
    sp_leds(lay, held);
    return out;
}
static int smk_mix_mode(void) { return sp.lay == SP_MIX; }   /* input(): MIX on any page plays the mix map */

/* ---- faders, wheels, the outer keys ---------------------------------------------------------------------------- */
static const int16_t SF_K[3][4] = {{P_PULSE, P_BASS, P_ACID, P_PAD},
                                   {K_FXF + EN_FXK_FILTER, K_FXF + EN_FXK_CRUSH, K_FXF + EN_FXK_THROW, K_FXF + EN_FXK_WASH},
                                   {P_MOTION, P_EVOLVE, P_AC_DENS, P_DRIFT}};
static const char *const SF_NAME[3][4] = {{"DRUMS", "BASS", "ACID", "PAD"}, {"FILTER", "CRUSH", "THROW", "WASH"},
                                          {"MOTION", "EVOLVE", "ACID DENSITY", "DRIFT"}};
static struct {
    uint8_t bank, live[4];
    int16_t ref[4];
    uint8_t ext_n[10], ext_vc[10], ext_note[10][3];
    int16_t pb;                                  /* the wheels as last read: pitch -127..127, mod 0..127, and the voice they act on */
    uint8_t mw, wvc;
} sf = {0, {0, 0, 0, 0}, {-1, -1, -1, -1}, {0}, {0}, {{0}}, 0, 0, 0};

static int sf_ext_ix(int k) { return k < 5 ? k : k - 27; }   /* the outer keys 0-4, 32-36 -> 0..9 */

static void smk_controls(void)
{
    const uint32_t sb = plat_smk_buttons();
    static uint32_t sb_prev;
    int i;
    int32_t pb, mw, ev;
    call_tick();                                  /* a call ending, its echo */
    if ((sb & 3u) != 3u && (sb & ~sb_prev) >> SB_FADER_BANK & 1u) {  /* FADER BANK: the next bank; each fader waits to be moved */
        static const char *const BN[3] = {"A: LEVELS", "B: FX", "C: SHAPE"};
        sf.bank = (uint8_t)((sf.bank + 1) % 3u);
        for (i = 0; i < 4; i++)
            sf.live[i] = 0, sf.ref[i] = -1;
        say("FADERS", BN[sf.bank]);
    }
    sb_prev = sb;
    plat_smk_leds((sf.bank ? 1u << SB_FADER_BANK : 0u) | (sp.mode != SP_PERFORM ? 1u << SB_KNOB_BANK : 0u));
    for (i = 0; i < 4; i++) {
        const int v = plat_fader(i);
        if (v < 0)
            continue;
        if (sf.ref[i] < 0)
            sf.ref[i] = (int16_t)v;              /* where it stood when the bank came: no jump */
        if (!sf.live[i] && (v - sf.ref[i] > 2 || sf.ref[i] - v > 2)) {
            sf.live[i] = 1;
            say("FADER", SF_NAME[sf.bank][i]);
        }
        if (sf.live[i] && v != sf.ref[i]) {
            const int k = SF_K[sf.bank][i];
            int lo, hi;
            knob_range(k, &lo, &hi);
            knob_set(k, lo + v * (hi - lo) / 127);
            sf.ref[i] = (int16_t)v;
        }
    }
    {   /* the wheels on the voice the keys play (the engine bends one voice: a new one takes both, the old one straightens) */
        static const uint8_t WV[4] = {EN_V_PLUCK, EN_V_PAD, EN_V_ACID, EN_V_DRONE};
        const uint8_t vc = WV[key_voice() & 3];
        const int nw = plat_wheels(&pb, &mw);
        if (nw) {
            sf.pb = (int16_t)pb;
            sf.mw = (uint8_t)mw;
        }
        if (nw || vc != sf.wvc) {
            sf.wvc = vc;
            en_bend(vc, sf.pb * 64);             /* -127..127 -> two semitones each way */
            en_modw(vc, sf.mw);                  /* the mod wheel: a vibrato, up to half a semitone each way */
        }
    }
    while ((ev = plat_smk_extkey()) >= 0) {      /* the outer keys: C3-E3 and G#5-C6, the voice the page plays */
        static const uint8_t VC[4] = {EN_V_PLUCK, EN_V_PAD, EN_V_ACID, EN_V_DRONE};
        static const int8_t OFS[4] = {0, 0, 12, 24};
        const int k = ev >> 8, vel = ev & 0x7F, x = sf_ext_ix(k), idx = k - (int)SMK_KEY_OFS;
        if ((unsigned)x >= 10u)
            continue;
        if (vel) {
            if (ui.view == V_GLOBAL && !ui.fx_menu) {
                toggle_note(((5 + idx) % 12 + 12) % 12);
            } else {
                const int vc = (ui.view == V_JAM ? ui.jam_voice : key_voice()) & 3;
                uint8_t nn[4];
                int n = jam_notes(idx, nn), j;
                sf.ext_n[x] = (uint8_t)n;
                sf.ext_vc[x] = VC[vc];
                for (j = 0; j < n; j++) {
                    const int m = nn[j] + OFS[vc];
                    sf.ext_note[x][j] = (uint8_t)(m > 127 ? 127 : m);
                    en_note_v(VC[vc], sf.ext_note[x][j], vel);
                }
            }
        } else {
            int j;
            for (j = 0; j < sf.ext_n[x]; j++)
                en_note(sf.ext_vc[x], sf.ext_note[x][j], 0);
            sf.ext_n[x] = 0;
        }
    }
}
