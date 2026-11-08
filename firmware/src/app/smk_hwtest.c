/* SPDX-License-Identifier: GPL-3.0-only
 * PERF-1 (deamonSLINKY): the SMK-37 hardware test screen (build 1 of PERF-1 SMK, 2026-11-08). Included by ui.c in the
 * BOARD_SMK37 build. The unit boots into it: every input live, as the HAL sees it, so one look (or a photo) finds a
 * wrong row, index or curve. Hold KNOB BANK + FADER BANK for 1.5 s: PERF-1 itself.
 *
 *   top band:    the keybed (37 keys, held lit; the last key and its velocity), the 16 pads (pressure as the fill,
 *                0..7 the top row as the hardware numbers them), flash part, factory curves, ADC health
 *   bottom band: the 4 faders, the 8 encoders (transitions since boot, + = clockwise), the wheels, pedal and
 *                battery (raw 0..1023), and all 18 buttons (lit while held; the button's own LED lights too)
 *   pads' LEDs:  a test that loops: all red, all green, all blue (1 s each), then one pad white at a time 0..15;
 *                a pressed pad shows white over it
 */
#define HWT_EXIT_MS 1500u
#ifdef EN_HOST
#define HWT_BOOT 0                               /* the simulator boots into PERF-1 (script: hwtest) */
#else
#define HWT_BOOT 1
#endif

static struct {
    uint8_t on, last_key, last_vel, last_pad, last_pvel;
    uint32_t t0, both_t;
    uint64_t prev_keys;
} hwt = {HWT_BOOT, 0xFF, 0, 0xFF, 0, 0, 0, 0};

static const char *const HWT_BTN[18] = {"KNB", "FDR", "<", ">", "OC-", "OC+", "ARP", "NRP", "SCL",
                                        "CHD", "GLB", "BT", "PAT", "PAR", "FX", "SEQ", "PLY", "REC"};

static void hwt_num(char *b, int32_t v)          /* signed, into b */
{
    if (v < 0) {
        *b++ = '-';
        v = -v;
    }
    itoa_u((uint32_t)v, b);
}

static uint32_t hwt_len(const char *s)
{
    uint32_t n = 0;
    while (s[n])
        n++;
    return n;
}

static void hwt_cat(char *d, const char *s)
{
    while (*d)
        d++;
    while ((*d++ = *s++) != 0) {
    }
}

static void hwt_pads_led(const smk_hw_t *h)
{
    const uint32_t t = plat_ms() - hwt.t0, ph = t % 5400u;
    uint32_t p;
    for (p = 0; p < 16u; p++) {
        uint8_t r = 0, g = 0, b = 0;
        if (h->pad_press[p])
            r = g = b = 255;
        else if (ph < 1000u)
            r = 160;
        else if (ph < 2000u)
            g = 160;
        else if (ph < 3000u)
            b = 160;
        else if ((ph - 3000u) / 150u == p)
            r = g = b = 200;
        plat_pad_rgb((int)p, r, g, b);
    }
}

static void hwt_draw_top(const smk_hw_t *h)
{
    static const uint8_t BLACK_OF[12] = {0, 1, 0, 1, 0, 0, 1, 0, 1, 0, 1, 0};
    char b[40];
    int32_t k, x, wi = 0;
    cv_begin(240, 120, RGB(12, 12, 16));
    cv_text(4, 0, &FONT_XS, "PERF-1 SMK  HW TEST", RGB(255, 180, 70));
    if (hwt.last_key != 0xFF) {
        b[0] = 0;
        hwt_cat(b, "KEY ");
        itoa_u(hwt.last_key, b + 4);
        hwt_cat(b, " V");
        itoa_u(hwt.last_vel, b + hwt_len(b));
        cv_text(236 - text_w(&FONT_XS, b), 0, &FONT_XS, b, C_WHITE);
    }
    /* the keybed: 22 white keys of 10 px from x 10, the black ones over them */
    for (k = 0; k < 37; k++) {
        if (BLACK_OF[k % 12])
            continue;
        x = 10 + wi * 10;
        cv_rect(x, 16, 9, 28, (h->keys >> k & 1u) ? RGB(255, 180, 70) : RGB(200, 200, 205));
        wi++;
    }
    wi = 0;
    for (k = 0; k < 37; k++) {
        if (!BLACK_OF[k % 12]) {
            wi++;
            continue;
        }
        x = 10 + wi * 10 - 3;
        cv_rect(x, 16, 6, 17, (h->keys >> k & 1u) ? RGB(255, 120, 40) : RGB(30, 30, 36));
    }
    /* the pads: 0..7 the top row, 8..15 the bottom row */
    for (k = 0; k < 16; k++) {
        const int32_t px = 2 + (k % 8) * 30, py = k < 8 ? 47 : 67;
        const uint32_t pr = h->pad_press[k];
        cv_frame(px, py, 28, 18, pr ? C_WHITE : RGB(70, 70, 80));
        if (pr)
            cv_rect(px + 2, py + 2, 24, 14, RGB(40 + pr, 20 + pr, 160));
        itoa_u((uint32_t)k, b);
        cv_text(px + 4, py + 2, &FONT_XS, b, pr ? C_WHITE : RGB(120, 120, 130));
    }
    b[0] = 0;
    hwt_cat(b, "FLASH ");
    hwt_cat(b, h->flash_chip == 1u ? "P25Q80" : h->flash_chip == 2u ? "UC25HQ80" : h->flash_chip == 3u ? "OTHER" : "NONE");
    hwt_cat(b, h->trim_ok ? "  TRIM OK" : "  TRIM DEFAULT");
    if (hwt.last_pad != 0xFF) {
        hwt_cat(b, "  PAD ");
        itoa_u(hwt.last_pad, b + hwt_len(b));
        hwt_cat(b, " V");
        itoa_u(hwt.last_pvel, b + hwt_len(b));
    }
    cv_text(4, 88, &FONT_XS, b, h->trim_ok ? RGB(140, 220, 140) : RGB(255, 120, 90));
    b[0] = 0;
    hwt_cat(b, "ADC ");
    itoa_u(h->adc_ok % 100000u, b + 4);
    hwt_cat(b, " MISS ");
    itoa_u(h->adc_miss, b + hwt_len(b));
    hwt_cat(b, "  HOLD KNB+FDR: PERF");
    cv_text(4, 103, &FONT_XS, b, RGB(150, 150, 160));
    cv_blit(0, 0);
}

static void hwt_draw_bottom(const smk_hw_t *h)
{
    char b[24];
    int32_t i;
    cv_begin(240, 120, RGB(12, 12, 16));
    for (i = 0; i < 4; i++) {                    /* faders */
        const int32_t y = i * 14;
        b[0] = 'F', b[1] = (char)('1' + i), b[2] = 0;
        cv_text(2, y, &FONT_XS, b, RGB(150, 150, 160));
        cv_rect(20, y + 4, 64, 6, RGB(40, 40, 48));
        cv_rect(20, y + 4, h->fader[i] / 2, 6, RGB(255, 180, 70));
        itoa_u(h->fader[i], b);
        cv_text(88, y, &FONT_XS, b, C_WHITE);
    }
    for (i = 0; i < 8; i++) {                    /* encoders K1..K8 */
        const int32_t x = 120 + (i / 4) * 60, y = (i % 4) * 14;
        b[0] = 'K', b[1] = (char)('1' + i), b[2] = ' ', b[3] = 0;
        hwt_num(b + 3, h->enc[i]);
        cv_text(x, y, &FONT_XS, b, h->enc[i] ? C_WHITE : RGB(150, 150, 160));
    }
    {
        static const char *const L[4] = {"PITCH ", "MOD ", "PEDAL ", "BATT "};
        const uint32_t v[4] = {h->wheel[0], h->wheel[1], h->pedal, h->batt};
        for (i = 0; i < 4; i++) {
            b[0] = 0;
            hwt_cat(b, L[i]);
            itoa_u(v[i], b + hwt_len(b));
            cv_text(2 + (i % 2) * 120, 58 + (i / 2) * 14, &FONT_XS, b, C_WHITE);
        }
    }
    for (i = 0; i < 18; i++) {                   /* buttons: 9 a row */
        const int32_t x = 1 + (i % 9) * 26, y = 88 + (i / 9) * 16;
        const int held = (int)(h->buttons >> i & 1u);
        if (held)
            cv_rect(x, y, 25, 15, RGB(255, 180, 70));
        else
            cv_frame(x, y, 25, 15, RGB(70, 70, 80));
        cv_text(x + (25 - text_w(&FONT_XS, HWT_BTN[i])) / 2, y + 1, &FONT_XS, HWT_BTN[i], held ? RGB(12, 12, 16) : C_WHITE);
    }
    cv_blit(0, 120);
}

/* one frame of the test; 0 = it ended (hold KNOB BANK + FADER BANK), and PERF-1 takes over */
static int hwt_frame(void)
{
    smk_hw_t h;
    uint64_t newk;
    uint32_t hits, p;
    int k;
    if (!hwt.t0)
        hwt.t0 = plat_ms() | 1u;
    plat_smk_hw(&h);
    newk = h.keys & ~hwt.prev_keys;
    hwt.prev_keys = h.keys;
    for (k = 0; k < 37; k++)
        if (newk >> k & 1u) {
            hwt.last_key = (uint8_t)k;
            hwt.last_vel = h.key_vel[k];
        }
    (void)plat_keybed_hits();                    /* (drained: PERF-1 starts with none pending) */
    hits = plat_pad_hits();
    for (p = 0; p < 16u; p++)
        if (hits >> p & 1u) {
            hwt.last_pad = (uint8_t)p;
            hwt.last_pvel = (uint8_t)plat_pad_vel((int)p);
        }
    {
        uint32_t i;
        for (i = 0; i < (uint32_t)NE; i++)
            (void)plat_enc((int)i);              /* (the UI's encoder steps: drained, the screen shows the transitions) */
    }
    if ((h.buttons & 3u) == 3u) {                /* KNOB BANK + FADER BANK held */
        if (!hwt.both_t)
            hwt.both_t = plat_ms() | 1u;
        else if (plat_ms() - hwt.both_t > HWT_EXIT_MS) {
            for (p = 0; p < 16u; p++)
                plat_pad_rgb((int)p, 0, 0, 0);
            plat_smk_leds(0);
            plat_smk_btn_leds(0);
            hwt.on = 0;
            return 0;
        }
    } else {
        hwt.both_t = 0;
    }
    hwt_pads_led(&h);
    plat_smk_btn_leds(h.buttons);                /* the button LEDs: each lights while its button is held */
    if ((ui.frame++ & 1u) == 0u)
        hwt_draw_top(&h);
    else
        hwt_draw_bottom(&h);
    return 1;
}
