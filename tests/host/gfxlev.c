/* SPDX-License-Identifier: GPL-3.0-only */
/* Tool (not in the suite): each effect's level against the music without it (0.17.19, the maker: "a good chunk of them reduce the volume"). A piece
 * plays for a while dry, then again from the same start with one effect held, at its settings' defaults; the RMS of each run after its first two
 * seconds, and the difference. gfxlev [genre] [piece] [seconds]   (EN_FX_NAME for the names; FXONLY=n: that effect alone) */
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include "amb.h"

void en_midi_out(uint32_t st, uint32_t d1, uint32_t d2) { (void)st; (void)d1; (void)d2; }
void en_test_reseed(uint32_t s);

static double run(int g, int pc, int secs, int fx)
{
    static int32_t buf[512];
    int16_t par[P_NPARAMS];
    uint16_t pool, pats[EN_NPAT][EN_NLANE];
    int p, l, i, f;
    long b, nb = (long)secs * 44100 / 256, skip = 2L * 44100 / 256;
    double s2 = 0.0;
    for (f = 0; f < EN_NFXS; f++)
        en_fxs(f, EN_FXS_DEF[f]);
    en_init();
    en_test_reseed(7);
    en_genre(g);
    en_piece_g(g, pc, par, &pool);
    en_piece_pats_g(g, pc, pats);
    for (p = 0; p < P_NPARAMS; p++)
        en_set(p, par[p]);
    en_pool(pool);
    for (p = 0; p < EN_NPAT; p++)
        for (l = 0; l < EN_NLANE; l++)
            en_pats[p][l] = pats[p][l];
    en_fx(fx >= 0 ? 1ull << fx : 0ull);
    en_regrow();
    en_play(1);
    for (b = 0; b < nb; b++) {
        en_render(buf, 256, 4096);
        if (b >= skip)
            for (i = 0; i < 512; i++) {
                const double x = buf[i] / 8388608.0;
                s2 += x * x;
            }
    }
    return 10.0 * log10(s2 / ((nb - skip) * 512.0) + 1e-20);
}

int main(int argc, char **argv)
{
    const int g = argc > 1 ? atoi(argv[1]) : 2, pc = (argc > 2 ? atoi(argv[2]) : 1) - 1, secs = argc > 3 ? atoi(argv[3]) : 12,
              only = getenv("FXONLY") ? atoi(getenv("FXONLY")) : -1;
    const double dry = run(g, pc, secs, -1);
    int fx;
    printf("%s %02d dry %.1f dBFS\n", en_genre_name(g), pc + 1, dry);
    for (fx = 0; fx < EN_NFX; fx++) {
        double w;
        if ((only >= 0 && fx != only) || !EN_FX_NAME[fx] || !EN_FX_NAME[fx][0])
            continue;
        w = run(g, pc, secs, fx);
        printf("%2d %-10s %+6.1f dB\n", fx, EN_FX_NAME[fx], w - dry);
    }
    return 0;
}
