/* SPDX-License-Identifier: GPL-3.0-only */
/* Tool (not in the suite): the engine's filter cores, measured directly (amb.c included, so its static filters are in reach): each SVF type's gain
 * (dB) at frequencies around a 1 kHz cutoff for a few resonances, the two-stage 24 dB types as the voices run them, and the acid's 303. gfilt */
#include "amb.c"
#include <stdio.h>

void en_midi_out(uint32_t st, uint32_t d1, uint32_t d2) { (void)st; (void)d1; (void)d2; }

static const float FR[] = {50, 200, 500, 800, 1000, 1250, 2000, 5000, 12000};
#define NF (int)(sizeof FR / sizeof FR[0])

static double gain_svf(float f, float fc, float k, int t, int two, float fc2, float k2)
{
    float c1[4], c2[4], s[4] = {0, 0, 0, 0};
    double pk = 0.0;
    long n;
    svf_coefs(c1, fc, k);
    svf_coefs(c2, fc2, k2);
    for (n = 0; n < 44100; n++) {
        float x = 0.1f * sinf(2.0f * FM_PI * f * (float)n / EN_SR), y = svf_mm(x, c1, &s[0], t);
        if (two)
            y = svf_mm(y, c2, &s[2], t);
        if (n > 22050 && fabs(y) > pk)
            pk = fabs(y);
    }
    return 20.0 * log10(pk / 0.1 + 1e-12);
}

int main(void)
{
    static const char *const TN[5] = {"low", "high", "band", "notch", "peak"};
    static const float K[3] = {1.41f, 0.6f, 0.15f};
    int t, ki, i;
    printf("one SVF stage, cutoff 1 kHz (k 1.41 flat, 0.6, 0.15 resonant): gain dB at");
    for (i = 0; i < NF; i++)
        printf(" %6.0f", FR[i]);
    printf("\n");
    for (t = 0; t < 5; t++)
        for (ki = 0; ki < 3; ki++) {
            printf("  %-5s k %.2f                                              ", TN[t], K[ki]);
            for (i = 0; i < NF; i++)
                printf(" %6.1f", gain_svf(FR[i], 1000.0f, K[ki], t, 0, 0, 0));
            printf("\n");
        }
    printf("two stages as the acid runs them (the second at 1.15x, k 1.2):\n");
    for (t = 0; t < 2; t++)
        for (ki = 0; ki < 3; ki++) {
            printf("  %-5s k %.2f                                              ", TN[t], K[ki]);
            for (i = 0; i < NF; i++)
                printf(" %6.1f", gain_svf(FR[i], 1000.0f, K[ki], t, 1, 1150.0f, 1.2f));
            printf("\n");
        }
    printf("two stages as the bass runs them (the second at 1.3x, k2 as for k):\n");
    for (t = 0; t < 2; t++)
        for (ki = 0; ki < 3; ki++) {
            printf("  %-5s k %.2f                                              ", TN[t], K[ki]);
            for (i = 0; i < NF; i++)
                printf(" %6.1f", gain_svf(FR[i], 1000.0f, K[ki], t, 1, 1300.0f, 1.2f));
            printf("\n");
        }
    return 0;
}
