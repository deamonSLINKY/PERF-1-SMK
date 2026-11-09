/* SPDX-License-Identifier: GPL-3.0-only */
/* Tool (not in the suite): the acid alone through one filter type (FILTERS > Acid: 0 low .. 5 the 303), a piece for some seconds: its level, peak,
 * brightness (the RMS of the first difference over the RMS) and the host time it took; WAV=path writes it (16-bit stereo).
 *   gacid genre piece type [seconds] [resonance 0..100, the piece's when absent]   (piece 1 ..) */
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "amb.h"

void en_midi_out(uint32_t st, uint32_t d1, uint32_t d2) { (void)st; (void)d1; (void)d2; }

static void wr32(FILE *f, uint32_t v) { fputc((int)(v & 255u), f); fputc((int)(v >> 8 & 255u), f); fputc((int)(v >> 16 & 255u), f); fputc((int)(v >> 24), f); }

int main(int argc, char **argv)
{
    const int g = argc > 1 ? atoi(argv[1]) : 2, piece = (argc > 2 ? atoi(argv[2]) : 1) - 1, type = argc > 3 ? atoi(argv[3]) : 0,
              secs = argc > 4 ? atoi(argv[4]) : 20, reso = argc > 5 ? atoi(argv[5]) : -1;
    static int32_t buf[512];
    int16_t par[P_NPARAMS];
    uint16_t pool, pats[EN_NPAT][EN_NLANE];
    int p, l, i;
    long b, nb = (long)secs * 44100 / 256;
    double s2 = 0.0, d2 = 0.0, pk = 0.0, prev = 0.0, cpu;
    clock_t t0;
    FILE *w = getenv("WAV") ? fopen(getenv("WAV"), "wb") : NULL;
    en_init();
    en_genre(g);
    en_piece_g(g, piece, par, &pool);
    en_piece_pats_g(g, piece, pats);
    for (p = 0; p < P_NPARAMS; p++)
        en_set(p, par[p]);
    if (reso >= 0)
        en_set(P_AC_RES, reso);
    if (getenv("CUT"))
        en_set(P_AC_CUT, atoi(getenv("CUT")));
    en_pool(pool);
    for (p = 0; p < EN_NPAT; p++)
        for (l = 0; l < EN_NLANE; l++)
            en_pats[p][l] = pats[p][l];
    en_fxs(EN_FXS_FT_ACID, type);
    if (getenv("SQ")) {                          /* SCREAM all the way */
        en_fxs(EN_FXS_SQ_DRIVE, 100);
        en_fxs(EN_FXS_SQ_AMT, 100);
        en_fxs(EN_FXS_SQ_WOW, 100);
        en_fxs(EN_FXS_SQ_TGT, 0);
    }
    en_trk(0, 1u << EN_TR_ACID);
    en_play(1);
    if (w) {
        fputs("RIFF", w);
        wr32(w, 36u + (uint32_t)nb * 256u * 4u);
        fputs("WAVEfmt ", w);
        wr32(w, 16);
        wr32(w, 1u | 2u << 16);
        wr32(w, 44100);
        wr32(w, 44100 * 4);
        wr32(w, 4u | 16u << 16);
        fputs("data", w);
        wr32(w, (uint32_t)nb * 256u * 4u);
    }
    t0 = clock();
    for (b = 0; b < nb; b++) {
        en_render(buf, 256, 4096);
        for (i = 0; i < 512; i++) {
            const double x = buf[i] / 8388608.0;
            if (i & 1)
                continue;
            s2 += x * x;
            d2 += (x - prev) * (x - prev);
            prev = x;
            pk = fabs(x) > pk ? fabs(x) : pk;
        }
        if (w)
            for (i = 0; i < 512; i++) {
                int v = buf[i] >> 8;
                v = v > 32767 ? 32767 : v < -32768 ? -32768 : v;
                fputc(v & 255, w);
                fputc((v >> 8) & 255, w);
            }
    }
    cpu = (double)(clock() - t0) / CLOCKS_PER_SEC;
    if (w)
        fclose(w);
    printf("%s %02d type %d reso %d: RMS %.1f dBFS, peak %.1f dBFS, brightness %.3f, host %.2f s for %d s\n", en_genre_name(g), piece + 1, type,
           reso >= 0 ? reso : par[P_AC_RES], 10.0 * log10(s2 / (nb * 256.0) + 1e-20), 20.0 * log10(pk + 1e-12), sqrt(d2 / (s2 + 1e-20)), cpu, secs);
    return 0;
}
