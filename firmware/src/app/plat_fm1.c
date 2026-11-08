/* SPDX-License-Identifier: GPL-3.0-only */
/* AMB-1 platform on the FM-1: plat.h over Felucca's HAL, panel map, USB rings and storage
 * (and on the SMK-37, over hal/smk_input.h).
 * Part of the unity build (amb.c), after the HAL, usb.c, storage.c and panel.c. */

uint32_t plat_ms(void) { return fm1_ms; }

/* the power-on seed: the MASTER pot's noise and the timer between reads, with what the last boot left
 * (RAM that survives a reset; after a power cut, whatever it powered up as) */
static uint32_t seed_carry __attribute__((section(".noinit")));
uint32_t plat_seed(void)
{
    uint32_t h = 2166136261u ^ seed_carry, i;
    for (i = 0; i < 24u; i++) {
        h = (h ^ (uint32_t)fm1_adc_read(FM1_ADC_MASTER)) * 16777619u;
        h = (h ^ fm1_ticks()) * 16777619u;
        fm1_delay_ms(1);
    }
    h ^= h >> 15;
    seed_carry = h * 2654435761u + 1u;
    return h ? h : 1u;
}

uint32_t plat_buttons(void)
{
    uint32_t raw = fm1_in.buttons, out = 0, i;
    for (i = 0; i < NB; i++)
        if (raw & (1u << panel.btn[i]))
            out |= 1u << i;
    return out;
}

#if BOARD_SMK37
uint32_t plat_keys(void) { return fm1_in.knotes & ((1u << NKEYS) - 1u); }   /* the keybed's F3..G5 (not the pads) */
#else
uint32_t plat_keys(void) { return fm1_in.notes & ((1u << NKEYS) - 1u); }
#endif

int32_t plat_enc(int role)
{
    if (role < 0 || role >= NE)
        return 0;
    return fm1_enc_take(panel.enc[role]) * panel.dir[role];
}

static volatile uint32_t master_q12 = 2048;
uint32_t plat_master(void) { return master_q12; }
static volatile uint32_t batt_raw = 0xFFFFFFFFu;
uint32_t plat_battery(void) { return batt_raw; }
int plat_usb_power(void) { return usb.config && !usb.suspended; }

#if BOARD_SMK37
/* ---- the SMK-37 build: hal/smk_input.h. Button LEDs: one flag per button id (no dim level, so no glow);
 * the keys have no LEDs: the pads light in colour (plat_pad_rgb, lcd.c padled_*) */
static void led_pos_init(void) {}
void plat_glow(int on) { (void)on; }
void plat_play_red(int red) { (void)red; }      /* SEQ PLAY has one LED */
void plat_keys_dim(uint32_t keys) { (void)keys; }   /* (no key LEDs on the SMK) */
static uint32_t smk_xled;
void plat_smk_leds(uint32_t lit) { smk_xled = lit; }
void plat_leds(uint32_t buttons, uint32_t keys)
{
    uint8_t nl[SMK_NLED] = {0};
    uint32_t i;
    (void)keys;
    for (i = 0; i < NB; i++)
        if ((buttons >> i) & 1u)
            nl[panel.btn[i]] = 1;
    for (i = 0; i < NSB; i++)                    /* KNOB BANK, FADER BANK, <, >: button ids 0..3 */
        if ((smk_xled >> i) & 1u)
            nl[i] = 1;
    for (i = 0; i < SMK_NLED; i++)
        fm1_led[i] = nl[i];
}

uint32_t plat_smk_buttons(void)
{
    return (fm1_in.buttons >> SMK_BTN_KNOB_BANK & 1u) << SB_KNOB_BANK |
           (fm1_in.buttons >> SMK_BTN_FADER_BANK & 1u) << SB_FADER_BANK |
           (fm1_in.buttons >> SMK_BTN_LEFT & 1u) << SB_LEFT | (fm1_in.buttons >> SMK_BTN_RIGHT & 1u) << SB_RIGHT;
}
uint64_t plat_keybed(void)
{
    uint32_t k = fm1__lock();                    /* (64 bits, written by the scan ISR) */
    uint64_t v = fm1_in.keys;
    fm1__unlock(k);
    return v;
}
uint64_t plat_keybed_hits(void) { return smk_keybed_edges(); }
/* the pads: the HAL reports a pad as its white key (smk_input.h SMK_WHITE_NOTE, SMK_PAD_OF_WHITE); here they are
 * hardware pads again, 0..7 the top row and 8..15 the bottom row, left to right, for hits and LEDs alike */
uint32_t plat_pad_hits(void)
{
    uint32_t n = smk_pad_edges(), out = 0, w;
    for (w = 0; w < SMK_NPAD; w++)
        if (n >> SMK_WHITE_NOTE[w] & 1u)
            out |= 1u << SMK_PAD_OF_WHITE(w);
    return out;
}
int plat_pad_vel(int pad)
{
    uint32_t v = (uint32_t)pad < SMK_NPAD ? fm1_in.key_vel[SMK_KEY_F3 + SMK_WHITE_NOTE[SMK_WHITE_OF_PAD((uint32_t)pad)]] : 0u;
    return v ? (int)v : 100;
}
int plat_fader(int i) { return fm1_in.frames < 200u || (uint32_t)i > 3u ? -1 : (int)fm1_in.fader[i]; }
void plat_pad_rgb(int pad, uint8_t r, uint8_t g, uint8_t b)
{
    if ((uint32_t)pad < SMK_NPAD)
        padled_set((uint32_t)pad, r, g, b);
}

/* the wheels (mux C address 4 = pitch, 5 = mod), self-calibrating as in the reference port: the pitch wheel's
 * rest is its centre (taken once the readings settle, followed slowly while it rests), each wheel's range grows
 * to what it has reached (the travel is short: raw ~445..640 pitch, ~474..629 mod on a unit). Pitch: -127..127
 * with a dead band around the centre; mod: 0..127, up (lower raw) = more. */
#define SMK_PITCH_MAX 127
int plat_wheels(int32_t *pitch, int32_t *modw)
{
    enum { DEAD = 12, SPAN0 = 80, MOD_LO = 485, MOD_HI = 615 };
    static int32_t pc, plo, phi, mlo, mhi, rest, last_b = -1, last_m = -1;
    int32_t p = fm1_in.wheel[0], m = fm1_in.wheel[1], d, b, mw;
    if (fm1_in.frames < 300u)                    /* (the readings settle) */
        return 0;
    if (!pc) {
        pc = p;
        plo = p - SPAN0;
        phi = p + SPAN0;
        mlo = m < MOD_LO ? m : MOD_LO;
        mhi = m > MOD_HI ? m : MOD_HI;
    }
    if (p < plo)
        plo = p;
    if (p > phi)
        phi = p;
    if (m < mlo)
        mlo = m;
    if (m > mhi)
        mhi = m;
    d = p - pc;
    if (d > -DEAD && d < DEAD) {
        b = 0;
        if (++rest > 200) {                      /* resting: the centre follows a slow drift */
            pc += d > 0 ? 1 : d < 0 ? -1 : 0;
            rest = 0;
        }
    } else {
        rest = 0;
        b = d > 0 ? (d - DEAD) * SMK_PITCH_MAX / (phi - pc - DEAD > 1 ? phi - pc - DEAD : 1)
                  : (d + DEAD) * SMK_PITCH_MAX / (pc - plo - DEAD > 1 ? pc - plo - DEAD : 1);
        b = clamp(b, -SMK_PITCH_MAX, SMK_PITCH_MAX);
    }
    mw = mhi - m <= DEAD ? 0 : (mhi - m - DEAD) * 127 / (mhi - mlo - DEAD > 1 ? mhi - mlo - DEAD : 1);
    mw = clamp(mw, 0, 127);
    if (b == last_b && mw == last_m)
        return 0;
    last_b = b;
    last_m = mw;
    *pitch = b;
    *modw = mw;
    return 1;
}

/* the factory velocity curves of the keys and pads: USRTRIM (flash 0xC1000, "trimdat" at +0x224), read only,
 * over the SPI read (no XIP window needed); smk_trim_load ignores a record that does not check out */
static uint8_t smk_trim_buf[0x22Cu] __attribute__((aligned(4)));
static uint8_t smk_trim_ok;                      /* the factory record was taken (the HW TEST screen says so) */
static void smk_trim_init(void)
{
    if (flash_ok && st_read(SMK_TRIM_FLASH, smk_trim_buf, sizeof smk_trim_buf) == 0)
        smk_trim_ok = (uint8_t)smk_trim_load(smk_trim_buf);
}

/* the HW TEST screen: button LEDs by hardware id (bit i = button i's LED, smk_input.h), not through the FM-1 map */
void plat_smk_btn_leds(uint32_t ids)
{
    uint32_t i;
    for (i = 0; i < SMK_NLED; i++)
        fm1_led[i] = (uint8_t)(i < FM1_NBTN && (ids >> i & 1u));
}

/* the HW TEST screen's snapshot: every input as the HAL sees it (raw where it helps to find a wrong row or index) */
void plat_smk_hw(smk_hw_t *h)
{
    uint32_t k = fm1__lock(), i;
    h->keys = fm1_in.keys;
    for (i = 0; i < 37u; i++)
        h->key_vel[i] = fm1_in.key_vel[i];
    for (i = 0; i < 16u; i++) {
        h->pad_press[i] = (fm1_in.pad_st[i] & 0x80u) ? (uint8_t)(fm1_in.pad_press[i] ? fm1_in.pad_press[i] : 1u) : 0u;
        h->pad_raw[i] = fm1_in.pad_raw[i];
    }
    for (i = 0; i < 4u; i++)
        h->fader[i] = fm1_in.fader[i];
    for (i = 0; i < 8u; i++)
        h->enc[i] = smk_enc_total[i];
    h->wheel[0] = fm1_in.wheel[0];
    h->wheel[1] = fm1_in.wheel[1];
    h->pedal = fm1_in.pedal;
    h->batt = fm1_in.batt;
    h->buttons = fm1_in.buttons;
    h->frames = fm1_in.frames;
    h->adc_ok = fm1_in.adc_ok;
    h->adc_miss = fm1_in.adc_miss;
    for (i = 0; i < 8u; i++)
        h->hit_raw[i] = smk_hit_raw[i];
    h->hit_base = smk_hit_base;
    h->hit_pad = smk_hit_pad;
    h->hit_n = smk_hit_n;
    h->pad_t0 = smk_trim_pad[smk_hit_pad < 16u ? smk_hit_pad : 0u][0];
    fm1__unlock(k);
    h->trim_ok = smk_trim_ok;
    h->flash_chip = flash_chip;
}
#else
/* LEDs: the picture is built off-line and copied one byte per column (Felucca: clearing
 * and relighting would let the 10 kHz scan catch the dark gap and flicker) */
static uint8_t led_pos[FM1_NKEY];
static void led_pos_init(void)
{
    uint32_t id, p, r;
    for (id = 0; id < FM1_NKEY; id++) {
        led_pos[id] = 0xFF;
        for (p = 0; p < FM1_NCOL; p++)
            for (r = 1; r < 5u; r++)
                if (FM1_KEYMAP[r][p] == (int8_t)id)
                    led_pos[id] = (uint8_t)((p << 3) | r);
    }
}
/* Glow: every button not lit is lit dim, on one scan frame in GLOW_DIV (the HAL's fm1_led_dim) */
#define GLOW_DIV 4u
#define KEYS_DIV 6u                             /* the keys' dim (the scale while a chord plays): one frame in six, ~150 Hz; fainter than the glow (0.15.24) */
static uint8_t glow_on = 1;
void plat_glow(int on) { glow_on = (uint8_t)(on != 0); }
static uint32_t keys_dim;               /* (plat_keys_dim: the next plat_leds only) */
void plat_keys_dim(uint32_t keys) { keys_dim = keys; }
/* PLAY has two LEDs: its red at the button's own position, a green at the keyless matrix position
 * column 8, row PA5 (found on the device). Green, unless recording. */
static uint8_t play_red;
void plat_play_red(int red) { play_red = (uint8_t)(red != 0); }
void plat_leds(uint32_t buttons, uint32_t keys)
{
    uint8_t nl[FM1_NCOL] = {0}, dl[FM1_NCOL] = {0}, dk[FM1_NCOL] = {0};
    uint32_t i, c;
    for (i = 0; i < NB + NKEYS; i++) {
        uint32_t id = i < NB ? panel.btn[i] : 14u + (i - NB);
        int on = i < NB ? (buttons >> i) & 1u : (keys >> (i - NB)) & 1u;
        uint8_t q = led_pos[id];
        if (i == B_PLAY && !play_red)            /* PLAY's green LED instead (keyless position col 8, row PA5) */
            q = (uint8_t)((8u << 3) | 1u);
        if (q == 0xFF)
            continue;
        if (on)
            nl[q >> 3] |= (uint8_t)(1u << (q & 7u));
        else if (i < NB && glow_on)
            dl[q >> 3] |= (uint8_t)(1u << (q & 7u));
        else if (i >= NB && (keys_dim >> (i - NB) & 1u))
            dk[q >> 3] |= (uint8_t)(1u << (q & 7u));
    }
    for (c = 0; c < FM1_NCOL; c++) {
        fm1_led[c] = nl[c];
        fm1_led_dim[c] = dl[c];
        fm1_led_dim2[c] = dk[c];
    }
    fm1_led_dim_div = glow_on ? GLOW_DIV : 0u;
    fm1_led_dim2_div = keys_dim ? KEYS_DIV : 0u;
    keys_dim = 0;
}

#endif

/* MIDI: Felucca's rings carry 4-byte USB-MIDI event packets, byte 0 = cable / CIN */
int plat_midi_in(uint32_t *pkt)
{
    if (mi_r == mi_w)
        return 0;
    *pkt = midi_in_q[mi_r % MQ];
    mi_r++;
    return 1;
}
void plat_midi_out(uint32_t pkt)                    /* the ring has one writer at a time: IRQs off */
{
    uint32_t f = irq_save();
    midi_out_event(pkt);
    irq_restore(f);
}
/* the engine's MIDI out (from the render): a USB-MIDI packet, cable 0, CIN = the status's high nibble */
void en_midi_out(uint32_t st, uint32_t d1, uint32_t d2) { plat_midi_out(st >> 4 | st << 8 | d1 << 16 | d2 << 24); }

int plat_store_load(uint32_t obj, void *dst, uint32_t max) { return flash_ok ? st_load(obj, dst, max) : -1; }
static volatile uint32_t tel_stores, tel_store_us;       /* telemetry: flash writes so far, and how long the last one stopped the chip */
static uint8_t store_lock;                               /* a backup was restored: what is in RAM is out of date, so nothing more is written until the restart */
int plat_store_locked(void) { return store_lock; }
int plat_store_seamless(void) { return flash_ok && !susp_bad; }
#if PERF_USB_HOST
void plat_usb_host(int on) { uh.want = (uint8_t)(on != 0); }   /* uh_task (the main loop) switches the port */
int plat_usb_host_state(void) { return !uh.on ? 0 : uh.run ? 2 : uh.tried ? 3 : 1; }
int plat_usb_host_msg(void)
{
    const int m = uh.msg;
    uh.msg = UH_MSG_NONE;
    return m;
}
static char *diag_put(char *b, const char *s)
{
    while (*s)
        *b++ = *s++;
    *b = 0;
    return b;
}
static char *diag_num(char *b, uint32_t v)
{
    char t[8];
    int n = 0;
    do
        t[n++] = (char)('0' + v % 10u);
    while ((v /= 10u) && n < 7);
    while (n)
        *b++ = t[--n];
    *b = 0;
    return b;
}
void plat_usb_host_diag(char *b)
{
    static const char *const STEP[7] = {"OK", "NO ANSWER", "NO ADDR", "NO CONFIG", "NO MIDI", "CFG REFUSED", "IF REFUSED"};
    static const char HX[16] = {'0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E', 'F'};
    char *p = b;
    *p = 0;
    if (!uh_d.root) {                            /* nothing answered the first request: the error, the line and DEVCTL (0.15.11) */
        p = diag_put(p, "NO ANSWER E");
        p = diag_num(p, uh_d.err);
        p = diag_put(p, " L");
        p = diag_num(p, uh_d.line);
        p = diag_put(p, " D");
        *p++ = HX[uh_d.devctl >> 4];
        *p++ = HX[uh_d.devctl & 15u];
        *p = 0;
        return;
    }
    if (uh_d.root == 1) {
        p = diag_put(p, "HUB ");
        p = diag_num(p, uh_d.ports);
        p = diag_put(p, uh_d.nested ? "P, HUB INSIDE" : uh_d.devs ? "P " : "P, NOTHING ON IT");   /* (23 characters at most: say) */
        if (uh_d.nested || !uh_d.devs)
            return;
    }
    if (uh_d.devs || uh_d.root == 2) {
        p = diag_put(p, "C");
        p = diag_num(p, uh_d.cls);
        p = diag_put(p, " ");
        p = diag_num(p, uh_d.cfg);
        p = diag_put(p, "B ");
        diag_put(p, STEP[uh_d.step % 7u]);
    }
}
#else
void plat_usb_host(int on) { (void)on; }
int plat_usb_host_msg(void) { return 0; }
void plat_usb_host_diag(char *b) { b[0] = 0; }
int plat_usb_host_state(void) { return 0; }
#endif
int plat_store_save(uint32_t obj, const void *src, uint32_t len)
{
    uint32_t t0, r;
    if (!flash_ok)
        return -9;
    if (store_lock)
        return -8;
    t0 = fm1_ticks();
    r = (uint32_t)st_save(obj, src, len);
    tel_store_us = (fm1_ticks() - t0) / FM1_TICKS_PER_US;
    tel_stores++;
    return (int)r;
}
int plat_store_restore(uint32_t obj, const void *src, uint32_t len)
{
    int r;
    if (!flash_ok)
        return -9;
    store_lock = 1;
    r = st_save(obj, src, len);
    tel_stores++;
    return r;
}

static volatile uint32_t audio_cpu_pct, audio_xruns, audio_late;
uint32_t plat_cpu_pct(void) { return audio_cpu_pct; }
uint32_t plat_xruns(void) { return audio_xruns; }
uint32_t plat_late(void) { return audio_late; }
static volatile uint32_t pk_cur_pct, pk_prev_pct, pk_n;   /* the longest render, in percent of its time, in this second and the last */
uint32_t plat_peak_pct(void) { return pk_cur_pct > pk_prev_pct ? pk_cur_pct : pk_prev_pct; }
uint32_t plat_cpu1_pct(void) { return fm1_core1_load(); }
uint32_t plat_core2(void) { return EN_CORE2 ? (fm1_core1_online() ? 1u : 2u) : 0u; }
