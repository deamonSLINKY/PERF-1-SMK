/* SPDX-License-Identifier: GPL-3.0-only */
/* AMB-1 in the browser: the host simulator (host/amb_host.c: the whole app, UI and instrument,
 * against a simulated FM-1) compiled to WebAssembly and run in an AudioWorklet (from X0X's). The worklet asks
 * for audio; producing it advances the device's clock one millisecond at a time, which runs the
 * engine every 256 samples (the I2S half buffer) and the UI every 16 ms, as on the FM-1. The page
 * sends input, and reads the screen, the lights and the saved objects back through these exports.
 *
 *   web/emu/build.sh   ->  build/emu/amb.wasm (+ the page) */
#include <stdint.h>

static void web_audio(const int32_t *blk, uint32_t n);
#include "../../host/amb_host.c"

#undef __attribute__

/* output ring, stereo float, filled by run_ms, drained by web_render */
#define RING 4096u
static float ring[RING * 2];
static uint32_t ring_w, ring_r;
static void web_audio(const int32_t *blk, uint32_t n)
{
    uint32_t k;
    for (k = 0; k < n; k++) {
        uint32_t i = (ring_w++ % RING) * 2u;
        ring[i] = (float)blk[2 * k] / 8388608.0f;
        ring[i + 1] = (float)blk[2 * k + 1] / 8388608.0f;
    }
}

static float out_l[1024], out_r[1024];
__attribute__((used, visibility("default"))) float *web_out_l(void) { return out_l; }
__attribute__((used, visibility("default"))) float *web_out_r(void) { return out_r; }

/* n frames (<= 1024) into out_l / out_r: the device runs until they exist */
__attribute__((used, visibility("default"))) void web_render(uint32_t n)
{
    uint32_t k;
    if (n > 1024u)
        n = 1024u;
    while (ring_w - ring_r < n)
        run_ms(1);
    for (k = 0; k < n; k++) {
        uint32_t i = (ring_r++ % RING) * 2u;
        out_l[k] = ring[i];
        out_r[k] = ring[i + 1];
    }
}

__attribute__((used, visibility("default"))) void web_boot(void)
{
    ui_splash_ms = 3200u;                        /* the boot cards, as on the FM-1 */
    boot();
}
__attribute__((used, visibility("default"))) void web_seed(uint32_t s) { host_seed = s; }   /* before web_boot */
__attribute__((used, visibility("default"))) void web_buttons(uint32_t m) { held_btn = m; }
__attribute__((used, visibility("default"))) void web_keys(uint32_t m) { held_keys = m; }
__attribute__((used, visibility("default"))) void web_enc(int role, int32_t n)
{
    if (role >= 0 && role < NE)
        enc_acc[role] += n;
}
__attribute__((used, visibility("default"))) void web_master(uint32_t v) { master = v > 4096u ? 4096u : v; }
__attribute__((used, visibility("default"))) uint32_t web_lit_buttons(void) { return lit_btn; }
__attribute__((used, visibility("default"))) uint32_t web_lit_keys(void) { return lit_keys; }
__attribute__((used, visibility("default"))) uint16_t *web_fb(void) { return fb; }
__attribute__((used, visibility("default"))) uint32_t web_blits(void) { return blits; }
__attribute__((used, visibility("default"))) uint32_t web_now(void) { return now_ms; }
__attribute__((used, visibility("default"))) uint32_t web_cpu(void) { return cpu_pct; }

/* saved objects: the page keeps them (localStorage) and gives them back before boot */
__attribute__((used, visibility("default"))) uint8_t *web_store(uint32_t obj) { return obj < OBJ_NOBJ ? store[obj] : 0; }
__attribute__((used, visibility("default"))) int web_store_len(uint32_t obj) { return obj < OBJ_NOBJ ? store_len[obj] : -1; }
__attribute__((used, visibility("default"))) void web_store_set_len(uint32_t obj, int n)
{
    if (obj < OBJ_NOBJ)
        store_len[obj] = n < 0 ? -1 : n > (int)PLAT_STORE_MAX ? (int)PLAT_STORE_MAX : n;
}
__attribute__((used, visibility("default"))) uint32_t web_store_writes(void) { return store_writes; }
__attribute__((used, visibility("default"))) uint32_t web_nobj(void) { return OBJ_NOBJ; }
__attribute__((used, visibility("default"))) uint32_t web_store_max(void) { return PLAT_STORE_MAX; }

static float bg_out[12];                         /* for the stream view's background: what the screen shows */
static void bg_rgb(int at, uint16_t c)
{
    bg_out[at] = (float)(c >> 11) / 31.0f;
    bg_out[at + 1] = (float)((c >> 5) & 63u) / 63.0f;
    bg_out[at + 2] = (float)(c & 31u) / 31.0f;
}
/* the stream view's background: the screen theme's primary, accent and ground */
__attribute__((used, visibility("default"))) float *web_bg(void)
{
    bg_rgb(0, th->pad);
    bg_rgb(3, th->acc);
    bg_rgb(6, th->bg);
    bg_rgb(9, th->acid);
    return bg_out;
}

/* ---- the studio (web/studio, 0.17): a piece of any genre taken up as the panel takes it (GLO / PRESETS), read out in its stored form, an
 * edited one put back into play as you change it, and what the engine plays now (the bar, the chord, the lines it makes up as it goes).
 * Nothing here writes the flash: the bank in RAM is put back as it was. st_io: the record (IMG_REC) | the progression (EN_PRG_LEN) | its chord
 * types (EN_PRK_LEN) | the bass steps (16) | 1 when the FILTERS and SCREAM follow | those (PX_NFX). */
#define ST_REC 0
#define ST_PRG (ST_REC + IMG_REC)
#define ST_PRK (ST_PRG + EN_PRG_LEN)
#define ST_BS (ST_PRK + EN_PRK_LEN)
#define ST_FXON (ST_BS + 16)
#define ST_FX (ST_FXON + 1)
#define ST_LEN (ST_FX + PX_NFX)
static uint8_t st_io[ST_LEN];
__attribute__((used, visibility("default"))) uint8_t *web_st_io(void) { return st_io; }
__attribute__((used, visibility("default"))) int web_st_io_len(void) { return ST_LEN; }
/* the playing piece into st_io, as SAVE would store it */
__attribute__((used, visibility("default"))) int web_st_read(void)
{
    const int i = proj.piece;
    slot_t *s = &proj.slot[i];
    uint8_t *rec = image + IMG_HDR + (uint32_t)i * IMG_REC, *tail = image + IMG_TAIL + (uint32_t)i * EN_PRG_LEN;
    uint8_t k_rec[IMG_REC], k_tail[EN_PRG_LEN], k_px[PX_LEN];
    const uint8_t k_pend = pending_px;
    int k;
    keep();
    for (k = 0; k < (int)IMG_REC; k++) k_rec[k] = rec[k];
    for (k = 0; k < EN_PRG_LEN; k++) k_tail[k] = tail[k];
    for (k = 0; k < (int)PX_LEN; k++) k_px[k] = px[i][k];
    pack_slot(s, i);
    for (k = 0; k < (int)IMG_REC; k++) st_io[ST_REC + k] = rec[k];
    for (k = 0; k < EN_PRG_LEN; k++) st_io[ST_PRG + k] = tail[k];
    for (k = 0; k < (int)IMG_REC; k++) rec[k] = k_rec[k];
    for (k = 0; k < EN_PRG_LEN; k++) tail[k] = k_tail[k];
    for (k = 0; k < (int)PX_LEN; k++) px[i][k] = k_px[k];
    pending_px = k_pend;
    for (k = 0; k < EN_PRK_LEN; k++) st_io[ST_PRK + k] = s->prk[k];
    for (k = 0; k < 16; k++) st_io[ST_BS + k] = s->bs[k];
    st_io[ST_FXON] = 1;
    project_fx_get(st_io + ST_FX);
    return 1;
}
/* piece i of genre g into play, as the panel takes it up (as left this time, else as saved, else its recipe); then read out */
__attribute__((used, visibility("default"))) int web_st_open(int g, int i)
{
    if ((unsigned)g >= EN_NGENRE || (unsigned)i >= NPIECES || ui.store_wait)
        return 0;
    if (g != proj.genre) {
        project_set_genre(g);
        en_genre(g);
        genre_fx(g);
    }
    set_piece(i);
    return web_st_read();
}
/* st_io into the playing piece, as if its settings, steps and lines had been changed on the panel (the track goes on) */
__attribute__((used, visibility("default"))) int web_st_put(void)
{
    const int i = proj.piece;
    slot_t *s = &proj.slot[i];
    uint8_t *rec = image + IMG_HDR + (uint32_t)i * IMG_REC, *tail = image + IMG_TAIL + (uint32_t)i * EN_PRG_LEN;
    uint8_t k_rec[IMG_REC], k_tail[EN_PRG_LEN], k_px[PX_LEN];
    int k, ok;
    keep();
    for (k = 0; k < (int)IMG_REC; k++) { k_rec[k] = rec[k]; rec[k] = st_io[ST_REC + k]; }
    for (k = 0; k < EN_PRG_LEN; k++) { k_tail[k] = tail[k]; tail[k] = st_io[ST_PRG + k]; }
    for (k = 0; k < (int)PX_LEN; k++) k_px[k] = px[i][k];
    for (k = 0; k < EN_PRK_LEN; k++) px[i][k] = st_io[ST_PRK + k];
    ok = unpack_slot(i, P_NPARAMS, 1, s);
    for (k = 0; k < (int)IMG_REC; k++) rec[k] = k_rec[k];
    for (k = 0; k < EN_PRG_LEN; k++) tail[k] = k_tail[k];
    for (k = 0; k < (int)PX_LEN; k++) px[i][k] = k_px[k];
    for (k = 0; k < 16; k++) s->bs[k] = st_io[ST_BS + k];
    if (st_io[ST_FXON]) {
        pxl[i][0] = 1;
        for (k = 0; k < PX_NFX; k++) pxl[i][1 + k] = st_io[ST_FX + k];
    }
    take(i);
    ui.key_on = 0;
    project_apply();
    proj.edited |= (uint16_t)(1u << i);
    return ok;
}
/* what plays now: the sixteenth since start (4), the step, the pattern, the progression's slot (-1 the walk), the chord's root (pitch class),
 * its notes (4), the pad's (4), the bass's last bar (16) and step, the acid's line (length, step, gate, accents, slides: 2 each; offsets, octaves:
 * 16 each), the pluck's loop (rungs, levels: 16 each; its slot), the pattern each lane plays (6), playing, the genre, the piece, the chord's tones,
 * each lane's step (6), the pool (2), the root (P_ROOT), the bass mode */
static uint8_t st_lv[132];
__attribute__((used, visibility("default"))) uint8_t *web_st_live(void)
{
    uint8_t *b = st_lv;
    const uint32_t t = en_bars;
    int k;
    b[0] = (uint8_t)t; b[1] = (uint8_t)(t >> 8); b[2] = (uint8_t)(t >> 16); b[3] = (uint8_t)(t >> 24);
    b[4] = en_drum_pos; b[5] = en_pat_now; b[6] = (uint8_t)en_prog_pos; b[7] = en_chord_root;
    for (k = 0; k < 4; k++) { b[8 + k] = en_chord[k]; b[12 + k] = en_pad_note[k]; }
    for (k = 0; k < 16; k++) b[16 + k] = en_bs_hist[k];
    b[32] = (uint8_t)en_bs_pos; b[33] = en_acid_len; b[34] = en_acid_pos;
    b[35] = (uint8_t)en_acid_gate; b[36] = (uint8_t)(en_acid_gate >> 8);
    b[37] = (uint8_t)en_acid_acc; b[38] = (uint8_t)(en_acid_acc >> 8);
    b[39] = (uint8_t)en_acid_slide; b[40] = (uint8_t)(en_acid_slide >> 8);
    for (k = 0; k < 16; k++) { b[41 + k] = en_acid_off[k]; b[57 + k] = en_acid_oct[k]; b[73 + k] = en_cm_rung[k]; b[89 + k] = en_cm_lvl[k]; }
    b[105] = en_cm_pos;
    for (k = 0; k < EN_NLANE; k++) { b[106 + k] = en_lane_src[k]; b[116 + k] = en_lane_pos[k]; }
    b[112] = en_playing; b[113] = proj.genre; b[114] = proj.piece; b[115] = en_chord_n;
    b[122] = (uint8_t)proj.pool; b[123] = (uint8_t)(proj.pool >> 8);
    b[124] = (uint8_t)proj.par[P_ROOT]; b[125] = (uint8_t)proj.par[P_BL_MODE];
    b[126] = (uint8_t)proj.par[P_AC_LOCK]; b[127] = (uint8_t)proj.par[P_CM_LOCK];
    return b;
}
__attribute__((used, visibility("default"))) void web_st_boot(void)
{
    ui_splash_ms = 600u;                         /* the studio: the boot card short */
    boot();
}
/* track codes (0.17.11, ui.c): a code's text in st_txt started as the panel's Code page starts it (its genre, its piece's recipe, key, mode,
 * tempo, every generator from its seed), then read out into st_io (-1: not a code); and the code of the track playing into st_txt (1: it gives
 * the piece exactly as it is) */
static char st_txt[16];
__attribute__((used, visibility("default"))) char *web_st_txt(void) { return st_txt; }
__attribute__((used, visibility("default"))) int web_st_code_play(void)
{
    st_txt[15] = 0;
    if (ui_code_play(st_txt) < 0)
        return -1;
    return web_st_read();
}
__attribute__((used, visibility("default"))) int web_st_code_now(void) { return ui_code_now(st_txt); }
/* a new take: the track from its start with a new seed, the piece as it is (the panel's roll); the engine's own parts come out anew */
__attribute__((used, visibility("default"))) void web_st_roll(void)
{
    track_fresh(1, 1);
}
