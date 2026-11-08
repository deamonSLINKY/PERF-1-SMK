/* SPDX-License-Identifier: GPL-3.0-only
 * PERF-1 (deamonSLINKY): the SMK-37's pads as the FM-1's keys in their button duties (2026-11-08). Included by ui.c in
 * the BOARD_SMK37 build, before input(). On the SMK the 37 keys are only notes; whatever the FM-1 does with a key as a
 * button (the effects, the mix map, steps, slots, fills, SEL's shifts, the game) comes from a pad. smk_bkeys() returns
 * the FM-1 keys "held" by the pads (bit k = key k, 0 = F3 .. 26 = G5), and input() treats them as the FM-1 treats its
 * keys there. The page decides the layout (the pads switch by themselves):
 *
 *   STEPS  the sequencer, the step pages, the gate, the SAMPLE page: the 16 white keys = steps 1..16
 *          (top row 1..8, bottom row 9..16)
 *   ROWS   the Set list, the Progression page, and SEL held: top row the white keys 1..8 (slots, patterns),
 *          bottom row the black keys 1..8 (parts, fills, copies)
 *   KEYS   the FX keys, the mix map (GLO held, the Perform screen), everywhere else: the keys 1..16 in order;
 *          > shows keys 17..27, < keys 1..16 again
 *
 * The pads light dim in the layout's colour (KEYS page 2 another), white while held. The PERFORM / FX / MIX pad modes of
 * the port plan (sections, track buttons) come on top of this later. */
enum { SP_KEYS, SP_STEPS, SP_ROWS };
static struct {
    uint8_t page, lay, led_lay, led_page;
    uint32_t sb_prev, held_prev;
} sp = {0, SP_KEYS, 0xFF, 0xFF, 0, 0xFFFFFFFFu};

static int sp_layout(void)
{
    if (plat_buttons() >> B_SEL & 1u)
        return SP_ROWS;
    if (ui.fx_menu)
        return ui.fx_page == 1 || ui.fx_page == FXP_SAMPLE + 1 ? SP_STEPS : SP_KEYS;
    if (ui.view == V_SEQ || ui.view == V_ACSTEP || ui.view == V_BLSTEP || ui.view == V_CMSTEP || ui.view == V_STEPS)
        return SP_STEPS;
    if (ui.view == V_SETL || ui.view == V_PROG)
        return SP_ROWS;
    return SP_KEYS;
}

static int sp_key(int lay, uint32_t pad)          /* the FM-1 key a pad is in this layout, -1 none */
{
    if (lay == SP_STEPS)
        return WHITE_KEY[pad];
    if (lay == SP_ROWS)
        return pad < 8u ? WHITE_KEY[pad] : BLACK_KEY[pad - 8u];
    pad += sp.page ? 16u : 0u;
    return pad < (uint32_t)NKEYS ? (int)pad : -1;
}

static uint32_t smk_bkeys(void)
{
    const uint32_t sb = plat_smk_buttons(), held = plat_smk_pads_held();
    uint32_t out = 0, p;
    const int lay = sp_layout();
    if ((sb & ~sp.sb_prev) >> SB_RIGHT & 1u && lay == SP_KEYS && !sp.page) {
        sp.page = 1;
        say("PADS", "KEYS 17-27");
    }
    if ((sb & ~sp.sb_prev) >> SB_LEFT & 1u && lay == SP_KEYS && sp.page) {
        sp.page = 0;
        say("PADS", "KEYS 1-16");
    }
    sp.sb_prev = sb;
    sp.lay = (uint8_t)lay;
    for (p = 0; p < 16u; p++)
        if (held >> p & 1u) {
            const int k = sp_key(lay, p);
            if (k >= 0)
                out |= 1u << k;
        }
    if (held != sp.held_prev || lay != sp.led_lay || sp.page != sp.led_page) {   /* the pads' lights: only on a change */
        sp.held_prev = held;
        sp.led_lay = (uint8_t)lay;
        sp.led_page = sp.page;
        for (p = 0; p < 16u; p++) {
            if (held >> p & 1u)
                plat_pad_rgb((int)p, 255, 255, 255);
            else if (sp_key(lay, p) < 0)
                plat_pad_rgb((int)p, 0, 0, 0);
            else if (lay == SP_STEPS)
                plat_pad_rgb((int)p, 60, 24, 0);
            else if (lay == SP_ROWS)
                plat_pad_rgb((int)p, p < 8u ? 60 : 30, p < 8u ? 24 : 0, p < 8u ? 0 : 60);
            else
                plat_pad_rgb((int)p, 0, sp.page ? 50 : 14, sp.page ? 14 : 60);
        }
    }
    return out;
}
