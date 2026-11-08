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
 * The pads light in the layout's colour, white while held; MIX shows the mutes (red) and solos (white).
 *
 * FADERS. FADER BANK: A levels (drums, bass, acid, pad), B the four FX knobs (filter, crush, throw, wash), C shape (motion,
 * evolve, acid density, drift). A fader takes over once it moves (after a bank change too). Its LED: lit off A.
 * WHEELS. Pitch: bend (two semitones) on the voices played by hand. Mod: the acid's cutoff opens (the piece's own at rest).
 * KEYS. All 37 play, with velocity: the middle 27 as the FM-1's keys (jam_down_v), the outer ten (C3-E3, G#5-C6) the same
 * way here. Pads and notes: velocity 30 at the softest (the factory pad curve starts high: soft hits came out near 1). */
#define SMK_KEY_OFS 5                            /* keybed key 5 = F3, the FM-1's key 0 */
static int jam_notes(int i, uint8_t *out);       /* (below, with the JAM page) */
enum { SP_KEYS, SP_STEPS, SP_ROWS, SP_PERFORM, SP_MIX, SP_CHORDS, SP_NOTES };
static const char *const SP_NAME[] = {"KEYS", "STEPS", "SLOTS/FILLS", "PERFORM", "MIX", "CHORDS", "NOTES"};
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

static void sp_leds(int lay, uint32_t held)
{
    uint32_t p;
    const uint8_t sec = (uint8_t)(en_sec | en_frozen << 2 | ui.dj_on << 3);
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
        sp.mode = (uint8_t)(sp.mode == SP_NOTES ? SP_PERFORM : sp.mode + 1);
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
        } else if (lay == SP_NOTES) {
            static const char *const VN[3] = {"PLUCK", "BASS", "ACID"};
            sp_notes_off();
            sp.voice = (uint8_t)(right ? (sp.voice + 1) % 3u : (sp.voice + 2) % 3u);
            say("PADS: NOTES", VN[sp.voice]);
        }
    }
    if (sp_is_keys(lay)) {
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
} sf = {0, {0, 0, 0, 0}, {-1, -1, -1, -1}, {0}, {0}, {{0}}};

static int sf_ext_ix(int k) { return k < 5 ? k : k - 27; }   /* the outer keys 0-4, 32-36 -> 0..9 */

static void smk_controls(void)
{
    const uint32_t sb = plat_smk_buttons();
    static uint32_t sb_prev;
    int i;
    int32_t pb, mw, ev;
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
    if (plat_wheels(&pb, &mw)) {
        const en_param_t *pi = en_param_info(P_AC_CUT);
        const int base = mood_val(P_AC_CUT, proj.par[P_AC_CUT]);
        en_bend(EN_V_PLUCK, pb * 64);            /* -127..127 -> two semitones each way */
        en_bend(EN_V_DRONE, pb * 64);
        en_bend(EN_V_ACID, pb * 64);
        en_set(P_AC_CUT, base + (pi->hi - base) * mw / 127);   /* the mod wheel opens the acid; at rest, the piece's own */
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
