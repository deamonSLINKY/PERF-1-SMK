/* SPDX-License-Identifier: GPL-3.0-only */
/* THEORY's spelling (0.16.19, for issue #2 and a player learning to read music): a key's parent major scale, its key signature,
 * and every note named with the letter the key wants (G# in E major and over A minor's dominant, not Ab; Cb in Eb minor). Pure:
 * pitch classes in, letters and accidentals out; the THEORY visualizer (viz.c) draws with it and tests/host/theory_test.c checks
 * it against the mockup's table. Rules (the ones printed circles and sheet music use):
 *  - The parent major is the one sharing the most notes with the scale (a mode has exactly one; a pentatonic sits in three:
 *    the one whose tonic is the root, then the one whose sixth is, i.e. a minor pentatonic's relative major); fewer sharps or
 *    flats on a tie. F# / Gb (six either way): the spelling that gives the tonic fewer accidentals, then the root's usual one.
 *  - The scale's notes take the parent's letters, one letter each; the key signature is the parent's (sharps +, flats -).
 *  - A chord tone outside the scale (minor's raised seventh on V) is the scale note a semitone below it, raised (G# from G), or
 *    the one above it, lowered.
 *  - The other notes of the circle carry the major keys' names (C G D A E B F#/Gb Db Ab Eb Bb F; F# on the sharp side). */
#ifndef THEORY_H
#define THEORY_H
#include <stdint.h>

typedef struct {
    int parent;                  /* the parent major's tonic, a pitch class */
    int sig;                     /* the key signature: sharps > 0, flats < 0 */
    int8_t let[12];              /* each pitch class of the parent major: its letter (0 C .. 6 B), -1 not in it */
    int8_t acc[12];              /* .. and its accidental (-2 .. +2) */
} th_key_t;

static const uint8_t TH_NAT[7] = {0, 2, 4, 5, 7, 9, 11};
static const uint8_t TH_MAJ[7] = {0, 2, 4, 5, 7, 9, 11};

static int th_len(const char *s)                 /* (the firmware has no strlen) */
{
    int n = 0;
    while (s[n])
        n++;
    return n;
}
static int th_bits(uint32_t m)
{
    int n = 0;
    while (m) {
        n += (int)(m & 1u);
        m >>= 1;
    }
    return n;
}
static uint32_t th_major(int p)                  /* a major scale's pitch classes */
{
    uint32_t m = 0;
    int j;
    for (j = 0; j < 7; j++)
        m |= 1u << ((p + TH_MAJ[j]) % 12);
    return m;
}
static int th_sig_of(int p, int flats6)          /* a major key's signature; six either way at F# / Gb */
{
    const int f = p * 7 % 12;                    /* its place on the circle: 0 C, 1 G .. */
    return f == 6 ? (flats6 ? -6 : 6) : f < 6 ? f : f - 12;
}
static void th_fill(th_key_t *k, int p, int tl)  /* the parent's seven notes from its tonic's letter */
{
    int j;
    for (j = 0; j < 12; j++)
        k->let[j] = -1, k->acc[j] = 0;
    for (j = 0; j < 7; j++) {
        const int l = (tl + j) % 7, pc = (p + TH_MAJ[j]) % 12;
        int a = (pc - TH_NAT[l] + 12) % 12;
        if (a > 6)
            a -= 12;
        k->let[pc] = (int8_t)l;
        k->acc[pc] = (int8_t)a;
    }
}

/* the key of a scale (pool: absolute pitch classes) on root; root_flat: the root's usual name has a flat (Eb, Ab, Bb) */
static void th_key(uint32_t pool, int root, int root_flat, th_key_t *k)
{
    /* the major keys' tonic letters: C Db D Eb E F (F#|Gb) G Ab A Bb B */
    static const int8_t PL[12] = {0, 1, 1, 2, 2, 3, -1, 4, 5, 5, 6, 6};
    int p, best = -1, bs = -1000000;
    for (p = 0; p < 12; p++) {
        const int sg = th_sig_of(p, 0), s = th_bits(pool & th_major(p)) * 1000 + (p == root ? 200 : p == (root + 3) % 12 ? 100 : 0) -
                                          (sg < 0 ? -sg : sg);
        if (s > bs)
            bs = s, best = p;
    }
    k->parent = best;
    if (best != 6) {
        k->sig = th_sig_of(best, 0);
        th_fill(k, best, PL[best]);
        return;
    }
    {   /* F# major or Gb major: the tonic with fewer accidentals, then the root's usual spelling */
        th_key_t s, f;
        int as, af;
        th_fill(&s, 6, 3);                       /* F# */
        th_fill(&f, 6, 4);                       /* Gb */
        as = s.let[root] >= 0 ? (s.acc[root] < 0 ? -s.acc[root] : s.acc[root]) : 9;
        af = f.let[root] >= 0 ? (f.acc[root] < 0 ? -f.acc[root] : f.acc[root]) : 9;
        if (af < as || (af == as && root_flat)) {
            *k = f;
            k->sig = -6;
        } else {
            *k = s;
            k->sig = 6;
        }
        k->parent = 6;
    }
}

static char *th_put(char *o, int l, int a)       /* a letter and its accidental: b, bb, #, ## */
{
    *o++ = "CDEFGAB"[l];
    if (a < 0)
        while (a++ < 0)
            *o++ = 'b';
    else
        while (a-- > 0)
            *o++ = '#';
    *o = 0;
    return o;
}

/* a pitch class named in the key: its letter in the parent major; else (borrowed) the parent's note a semitone below it raised,
 * or the one above lowered; returns 1 when borrowed */
static int th_spell(const th_key_t *k, int pc, char *o)
{
    const int lo = (pc + 11) % 12, hi = (pc + 1) % 12;
    pc %= 12;
    if (k->let[pc] >= 0) {
        th_put(o, k->let[pc], k->acc[pc]);
        return 0;
    }
    if (k->let[lo] >= 0 && k->acc[lo] < 2)
        th_put(o, k->let[lo], k->acc[lo] + 1);
    else
        th_put(o, k->let[hi], k->acc[hi] - 1);
    return 1;
}

/* a chord's root named in the key: in it, the key's letter; outside it (bVII, bVI, bIII borrowed), lowered from the parent's note
 * above (Bb in C, not A#), else the semitone below raised. l, a: its letter and accidental. */
static void th_root(const th_key_t *k, int root, int *l, int *a)
{
    const int lo = (root + 11) % 12, hi = (root + 1) % 12;
    root %= 12;
    if (k->let[root] >= 0)
        *l = k->let[root], *a = k->acc[root];
    else if (k->let[hi] >= 0 && k->acc[hi] > -2)
        *l = k->let[hi], *a = k->acc[hi] - 1;
    else
        *l = k->let[lo], *a = k->acc[lo] + 1;
}

/* a chord tone named by its interval from the root, the way chords are spelled in thirds (F7 = F A C Eb, E7 = E G# B D):
 * the root's letter plus the interval's (b9/9 one up, 3rds two, 4th three, b5/5/#5 four, 6th five, 7ths six); a spelling that
 * would need three accidentals falls back to th_spell. */
static void th_tone(const th_key_t *k, int root, int pc, char *o)
{
    static const int8_t OFF[12] = {0, 1, 1, 2, 2, 3, 4, 4, 4, 5, 6, 6};
    int rl, ra, l, a;
    root %= 12, pc %= 12;
    th_root(k, root, &rl, &ra);
    if (pc == root) {
        th_put(o, rl, ra);
        return;
    }
    l = (rl + OFF[(pc - root + 12) % 12]) % 7;
    a = ((pc - TH_NAT[l]) % 12 + 12) % 12;
    if (a > 6)
        a -= 12;
    if (a < -2 || a > 2)
        th_spell(k, pc, o);
    else
        th_put(o, l, a);
}

/* the circle's label for a pitch class: the key's spelling for the parent's notes, the major keys' names for the rest */
static void th_label(const th_key_t *k, int pc, char *o)
{
    static const char *const KN[12] = {"C", "Db", "D", "Eb", "E", "F", "F#", "G", "Ab", "A", "Bb", "B"};
    const char *s = KN[pc % 12];
    if (k->let[pc % 12] >= 0) {
        th_put(o, k->let[pc % 12], k->acc[pc % 12]);
        return;
    }
    if (pc % 12 == 6 && k->sig < 0)
        s = "Gb";
    while (*s)
        *o++ = *s++;
    *o = 0;
}

/* a key signature as text: "0", "4#", "3b" */
static char *th_sig_text(int sig, char *o)
{
    const int n = sig < 0 ? -sig : sig;
    if (n >= 10)
        *o++ = '1';
    *o++ = (char)('0' + n % 10);
    if (n)
        *o++ = sig > 0 ? '#' : 'b';
    *o = 0;
    return o;
}

/* the letter a chord root takes in the key (borrowed roots lowered, th_root): for the Roman numeral's degree */
static int th_letter(const th_key_t *k, int pc)
{
    int l, a;
    th_root(k, pc, &l, &a);
    return l;
}
#endif
