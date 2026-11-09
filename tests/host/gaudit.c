/* SPDX-License-Identifier: GPL-3.0-only */
/* Tool (not in the suite): the song-length audit (2026-11-08). Plays each factory piece for a while (the engine alone: no auto-DJ, the piece's own
 * Form) and measures it from its MIDI out (the GM map: pad 1, pluck 2, bass 3, acid 4, drums 10). One CSV row a piece:
 *   notes per bar and bars silent, per part; the register (10th, 50th, 90th percentile pitch) of bass, acid, pluck;
 *   chord tones: the share of a part's on-beat notes that aren't in the chord, and of all its notes that aren't in the scale;
 *   clashes: a melodic note a semitone from another part's sounding note, one of the two not a chord tone (a minute);
 *   repetition: per part, distinct bars / bars, distinct 4-bar phrases / phrases, the longest run of the same bar;
 *   chords: changes, distinct chords, the chord loop's length (the shortest period the walk repeats with, 0 none);
 *   the band: bars with the drums alone or nothing melodic, bars with every part.
 *   gaudit [seconds, 180] [genre, all] [piece, all]   > audit.csv */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "amb.h"

#define SR 44100
#define BLK 256
#define MAXBAR 512
#define MAXN 60000
enum { PAD, PLK, BAS, ACD, DRM, NPART };
static const char *const PN[NPART] = {"pad", "pluck", "bass", "acid", "drums"};

typedef struct { uint32_t step; uint8_t part, note, vel, inchord, inscale, root; } nt_t;
static nt_t nt[MAXN];
static int nn;
static uint8_t snd[NPART][128];                  /* sounding now (note-on .. note-off) */
static int chord_pc[12], scale_pc[12];
static long clash, clp[NPART][NPART], clash_s, clash_alt, blk_alt;
static uint32_t b0;                              /* en_bars at this piece's start */

static int part_of(uint32_t st)
{
    switch (st & 15u) {
    case 0: return PAD;
    case 1: return PLK;
    case 2: return BAS;
    case 3: return ACD;
    case 9: return DRM;
    default: return -1;
    }
}

void en_midi_out(uint32_t st, uint32_t d1, uint32_t d2)
{
    const int p = part_of(st);
    if (p < 0 || d1 > 127)
        return;
    if ((st & 0xF0u) == 0x80u || ((st & 0xF0u) == 0x90u && !d2)) {
        snd[p][d1] = 0;
        return;
    }
    if ((st & 0xF0u) != 0x90u)
        return;
    if (p != DRM && p != PAD) {                  /* a melodic note against what the others are sounding */
        const int ct = chord_pc[d1 % 12];
        int q, n;
        for (q = PAD; q <= ACD; q++)
            if (q != p)
                for (n = 0; n < 128; n++)
                    if (snd[q][n]) {
                        const int ic = (d1 - n + 120) % 12;
                        if ((ic == 1 || ic == 11) && (!ct || !chord_pc[n % 12]))
                            clash++, clp[p < q ? p : q][p < q ? q : p]++, clash_s += (en_bars - b0) % 2u == 0, clash_alt += en_chord_alt && (en_bars - b0) % 2u == 0;
                    }
    }
    snd[p][d1] = 1;
    if (nn < MAXN) {
        nt[nn].step = en_bars - b0;
        nt[nn].part = (uint8_t)p;
        nt[nn].note = (uint8_t)d1;
        nt[nn].vel = (uint8_t)d2;
        nt[nn].inchord = (uint8_t)chord_pc[d1 % 12];
        nt[nn].inscale = (uint8_t)scale_pc[d1 % 12];
        nt[nn].root = en_chord_root;
        nn++;
    }
}

static int cmpi(const void *a, const void *b) { return *(const int *)a - *(const int *)b; }
static uint32_t mix(uint32_t h, uint32_t v) { return (h ^ v) * 16777619u; }

static void pct(int part, int *p10, int *p50, int *p90)
{
    static int v[MAXN];
    int i, k = 0;
    for (i = 0; i < nn; i++)
        if (nt[i].part == part)
            v[k++] = nt[i].note;
    if (!k) {
        *p10 = *p50 = *p90 = 0;
        return;
    }
    qsort(v, (size_t)k, sizeof v[0], cmpi);
    *p10 = v[k / 10];
    *p50 = v[k / 2];
    *p90 = v[k * 9 / 10];
}

/* distinct values among n hashes, the longest run of equal neighbours */
static int distinct(const uint32_t *h, int n, int *run)
{
    int i, j, d = 0, r = 1;
    *run = n ? 1 : 0;
    for (i = 0; i < n; i++) {
        for (j = 0; j < i && h[j] != h[i]; j++)
            ;
        d += j == i;
        r = i && h[i] == h[i - 1] ? r + 1 : 1;
        if (r > *run)
            *run = r;
    }
    return d;
}

static void one(int g, int pc, int secs)
{
    static int32_t buf[BLK * 2];
    static uint32_t bh[NPART][MAXBAR], ph[NPART][MAXBAR / 4], chs[4096];
    static int bn[NPART][MAXBAR];
    int16_t par[P_NPARAMS];
    uint16_t pool, pats[EN_NPAT][EN_NLANE];
    int p, l, i, b, nb, st[12], m, nch = 0, period = 0;
    uint32_t lastc = 0xFFFFFFFFu;
    long blk;
    nn = 0;
    clash = 0;
    clash_s = 0;
    clash_alt = blk_alt = 0;
    memset(clp, 0, sizeof clp);
    memset(snd, 0, sizeof snd);
    memset(bn, 0, sizeof bn);
    en_init();
    en_song = (uint8_t)(getenv("LOOP") ? 0 : 1);   /* FORM: Song, as the unit starts (ui.form 0) */
    en_genre(g);
    en_piece_g(g, pc, par, &pool);
    en_piece_pats_g(g, pc, pats);
    for (p = 0; p < P_NPARAMS; p++)
        en_set(p, par[p]);
    en_set(P_MIDI, 1);
    en_pool(pool);
    for (p = 0; p < EN_NPAT; p++)
        for (l = 0; l < EN_NLANE; l++)
            en_pats[p][l] = pats[p][l];
    memset(scale_pc, 0, sizeof scale_pc);
    m = en_scale(pool, st);
    for (i = 0; i < m; i++)
        scale_pc[(st[i] + par[P_ROOT]) % 12] = 1;
    en_play(1);
    b0 = en_bars;
    for (blk = 0; blk < (long)secs * SR / BLK; blk++) {
        uint32_t c = 0;
        memset(chord_pc, 0, sizeof chord_pc);
        for (i = 0; i < EN_NPAD; i++)
            if (en_chord[i])
                chord_pc[en_chord[i] % 12] = 1;
        for (i = 0; i < 12; i++)
            c = c * 2u + (uint32_t)chord_pc[i];
        c = c * 16u + en_chord_root;
        if (c != lastc && nch < 4096 && en_chord[0])
            chs[nch++] = c, lastc = c;
        blk_alt += en_chord_alt != 0;
        en_render(buf, BLK, 4096);
    }
    if (getenv("DEBUG")) {
        int h[NPART][12] = {{0}}, q;
        fprintf(stderr, "root %d mode %d pool %03x en_chord_root %d chord:", par[P_ROOT], par[P_MODE], (unsigned)pool, en_chord_root);
        for (i = 0; i < 12; i++)
            if (chord_pc[i])
                fprintf(stderr, " %d", i);
        fprintf(stderr, "\nscale:");
        for (i = 0; i < 12; i++)
            if (scale_pc[i])
                fprintf(stderr, " %d", i);
        fprintf(stderr, "\n");
        for (i = 0; i < nn; i++)
            h[nt[i].part][nt[i].note % 12]++;
        for (q = 0; q < NPART; q++) {
            fprintf(stderr, "%s:", PN[q]);
            for (i = 0; i < 12; i++)
                fprintf(stderr, " %d", h[q][i]);
            fprintf(stderr, "\n");
        }
    }
    if (getenv("NOTES")) {
        const int q = atoi(getenv("NOTES"));
        for (i = 0; i < nn && i < 4000; i++)
            if (nt[i].part == q && nt[i].step < 16u * 24u)
                fprintf(stderr, "%u.%02u %d%s%s  ", nt[i].step / 16u, nt[i].step % 16u, nt[i].note, nt[i].inchord ? "" : "*", nt[i].inscale ? "" : "!");
        fprintf(stderr, "\n");
    }
    nb = (int)((en_bars - b0) / 16u);
    if (nb > MAXBAR)
        nb = MAXBAR;
    for (p = 0; p < NPART; p++)
        for (b = 0; b < nb; b++)
            bh[p][b] = 2166136261u;
    for (i = 0; i < nn; i++) {
        b = (int)(nt[i].step / 16u);
        if (b >= nb)
            continue;
        bh[nt[i].part][b] = mix(mix(mix(bh[nt[i].part][b], nt[i].step % 16u), nt[i].note), nt[i].vel > 100);
        bn[nt[i].part][b]++;
    }
    for (p = 0; p < NPART; p++)
        for (b = 0; b + 3 < nb; b += 4)
            ph[p][b / 4] = mix(mix(mix(mix(2166136261u, bh[p][b]), bh[p][b + 1]), bh[p][b + 2]), bh[p][b + 3]);
    for (l = 1; l <= nch / 3 && !period; l++) {  /* the chord loop: the walk's second half repeats with period l */
        int ok = 1;
        for (i = nch / 2; i + l < nch && ok; i++)
            ok = chs[i] == chs[i + l];
        if (ok && nch - nch / 2 > 2 * l)
            period = l;
    }
    printf("%s,%d,%d,%d,%d", en_genre_name(g), g, pc + 1, par[P_TEMPO], nb);
    for (p = 0; p < NPART; p++) {               /* notes a bar, bars silent */
        int tot = 0, sil = 0;
        for (b = 0; b < nb; b++)
            tot += bn[p][b], sil += !bn[p][b];
        printf(",%.2f,%d", nb ? (double)tot / nb : 0.0, sil);
    }
    for (p = PLK; p <= ACD; p++) {               /* register */
        int a, c, d;
        pct(p, &a, &c, &d);
        printf(",%d,%d,%d", a, c, d);
    }
    for (p = PLK; p <= ACD; p++) {               /* on-beat non-chord tones, out of scale */
        int beat = 0, nct = 0, all = 0, oos = 0;
        for (i = 0; i < nn; i++)
            if (nt[i].part == p) {
                all++;
                oos += !nt[i].inscale;
                if (nt[i].step % 4u == 0) {
                    beat++;
                    nct += !nt[i].inchord;
                }
            }
        printf(",%.3f,%.3f", beat ? (double)nct / beat : 0.0, all ? (double)oos / all : 0.0);
    }
    printf(",%.1f", clash * 60.0 / secs);
    for (p = PLK; p <= DRM; p++) {               /* repetition */
        int run, prun, db = distinct(bh[p], nb, &run), dp = distinct(ph[p], nb / 4, &prun);
        printf(",%.3f,%.3f,%d", nb ? (double)db / nb : 0.0, nb / 4 ? (double)dp / (nb / 4) : 0.0, run);
    }
    for (int kind = 0; kind < 3; kind++)
    for (p = PLK; p <= ACD; p++) {              /* the echo: each bar against the 8 before, the best overlap (kind 0 notes, 1 to the chord root, 2 rhythm) */
        static uint16_t sets[MAXBAR][24];
        static int ns[MAXBAR];
        double sum = 0.0;
        int cnt = 0, k, j, a, c;
        memset(ns, 0, sizeof ns);
        for (i = 0; i < nn; i++) {
            b = (int)(nt[i].step / 16u);
            if (nt[i].part == p && b < nb && ns[b] < 24)
                sets[b][ns[b]++] = (uint16_t)(nt[i].step % 16u * 256u + (kind == 0 ? nt[i].note : kind == 1 ? (nt[i].note - nt[i].root + 132) % 256 : 0));
        }
        for (b = 1; b < nb; b++) {
            double best = 0.0;
            if (!ns[b])
                continue;
            for (k = b - 1; k >= 0 && k >= b - 8; k--) {
                int both = 0;
                for (j = 0; j < ns[b]; j++)
                    for (a = 0; a < ns[k]; a++)
                        if (sets[b][j] == sets[k][a]) {
                            both++;
                            break;
                        }
                c = ns[b] + ns[k] - both;
                if (c && (double)both / c > best)
                    best = (double)both / c;
            }
            sum += best;
            cnt++;
        }
        printf(",%.3f", cnt ? sum / cnt : 0.0);
    }
    printf(",%.1f", clash_s * 60.0 / secs);
    printf(",%.1f,%.1f,%.1f,%.1f,%.1f,%.1f", clp[PAD][PLK] * 60.0 / secs, clp[PAD][BAS] * 60.0 / secs, clp[PAD][ACD] * 60.0 / secs,
           clp[PLK][BAS] * 60.0 / secs, clp[PLK][ACD] * 60.0 / secs, clp[BAS][ACD] * 60.0 / secs);
    {
        int dc, r, alone = 0, full = 0;
        dc = distinct(chs, nch, &r);
        for (b = 0; b < nb; b++) {
            const int mel = (bn[PLK][b] > 0) + (bn[BAS][b] > 0) + (bn[ACD][b] > 0);
            alone += mel == 0;
            full += mel == 3 && bn[DRM][b] > 0;
        }
        int po = 0, pa = 0;
        for (i = 0; i < nn; i++)
            if (nt[i].part == PAD)
                pa++, po += !nt[i].inscale;
        printf(",%d,%d,%d,%d,%d,%d,%.3f,%.3f,%ld", nch, dc, period, alone, full, par[P_CHORD], pa ? (double)po / pa : 0.0,
               (double)blk_alt / ((double)secs * SR / BLK), clash_alt);
        printf("\n");
    }
    fflush(stdout);
}

int main(int argc, char **argv)
{
    const int secs = argc > 1 ? atoi(argv[1]) : 180, g1 = argc > 2 ? atoi(argv[2]) : -1, p1 = argc > 3 ? atoi(argv[3]) - 1 : -1;
    int g, pc, p;
    printf("genre,g,piece,bpm,bars");
    for (p = 0; p < NPART; p++)
        printf(",%s_npb,%s_sil", PN[p], PN[p]);
    for (p = PLK; p <= ACD; p++)
        printf(",%s_p10,%s_p50,%s_p90", PN[p], PN[p], PN[p]);
    for (p = PLK; p <= ACD; p++)
        printf(",%s_nct,%s_oos", PN[p], PN[p]);
    printf(",clash_pm");
    for (p = PLK; p <= DRM; p++)
        printf(",%s_dbar,%s_dphr,%s_run", PN[p], PN[p], PN[p]);
    printf(",pluck_echo,bass_echo,acid_echo,pluck_echor,bass_echor,acid_echor,pluck_echot,bass_echot,acid_echot,clash_strong,cl_pad_plk,cl_pad_bas,cl_pad_acd,cl_plk_bas,cl_plk_acd,cl_bas_acd");
    printf(",chord_changes,chord_distinct,chord_loop,bars_no_melody,bars_full,chord_kind,pad_oos,alt_time,clash_alt_n\n");
    for (g = 0; g < EN_NGENRE; g++)
        if (g1 < 0 || g == g1)
            for (pc = 0; pc < 10; pc++)
                if (p1 < 0 || pc == p1)
                    one(g, pc, secs);
    return 0;
}
