/* SPDX-License-Identifier: GPL-3.0-only
 * SMK-37 port: Copyright (C) 2026 M. Geisler (genkigenki/makkuro) */
/* SMK-37 Pro flash map (build with -DBOARD_SMK37=1):
 * every 4 KiB sector of the 1 MiB part through the app's allow-list (FL_RANGE_OK / FL_GUARD, fm1_flash_map.h)
 * and the update loader's (loader.c): none of them may ever reach the head 0x0-0x3FFF, BTIF / USRTRIM
 * 0xC0000-0xC1FFF, USR 0xF4000-0xFDFFF or key_mac 0xFF000-; the store's objects (storage.c) and the OTA
 * staging (ota.c) lie inside the allowed windows; ranges that wrap or straddle a boundary are refused.
 * Then the store owner tag (storage.c st_owner_check) on a simulated flash: ours kept, another family's
 * or untagged data cleared (only the object area), the tag written. */
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include "fm1_cc.h"
#if !BOARD_SMK37
#error "build with -DBOARD_SMK37=1"
#endif

#include "fm1_flash_map.h"
#include "fm1_flash_id.h"                  /* FL_JEDEC_OK (PERF-1 keeps the parts there) */
#include "../firmware/src/app/plat.h"      /* OBJ_* */

static uint8_t nor[0x100000];
static uint32_t bad_write;
static int st_read(uint32_t off, void *dst, uint32_t n) { memcpy(dst, nor + off, n); return 0; }
static int st_erase(uint32_t off)
{
    if (!FL_GUARD(off, 0x1000u) || (off & 0xFFFu)) {
        bad_write++;
        return -8;
    }
    memset(nor + off, 0xFF, 0x1000u);
    return 0;
}
static int st_prog(uint32_t off, const void *src, uint32_t n)
{
    const uint8_t *s = src;
    uint32_t i;
    if (!FL_GUARD(off, n) || ((off & 0xFFu) + n) > 256u) {
        bad_write++;
        return -8;
    }
    for (i = 0; i < n; i++)
        nor[off + i] &= s[i];
    return 0;
}
#include "../firmware/src/storage.c"
#define LDR_RANGE_OK(off, n) FL_LDR_RANGE_OK(off, n)       /* what loader.c uses on the SMK */

static int bad;
static void check(const char *what, int ok) { printf("smk flash: %-78s %s\n", what, ok ? "ok" : "FAIL"); bad += !ok; }

static int protected_sector(uint32_t s)
{
    return s < 0x4000u || (s >= 0xC0000u && s < 0xC2000u) || (s >= 0xF4000u && s < 0xFE000u) || s >= 0xFF000u;
}

static int all(uint32_t lo, uint32_t hi, uint8_t v)
{
    uint32_t i;
    for (i = lo; i < hi; i++)
        if (nor[i] != v)
            return 0;
    return 1;
}

int main(void)
{
    uint32_t s, o, c, leak_app = 0, leak_ldr = 0, leak_guard = 0, allowed = 0, in_store = 1;
    for (s = 0; s < 0x100000u; s += 0x1000u) {
        if (!protected_sector(s))
            continue;
        leak_app += FL_RANGE_OK(s, 0x1000u) || FL_STORE_OK(s, 0x1000u) || FL_RANGE_OK(s, 256u) || FL_RANGE_OK(s + 0xF00u, 256u);
        leak_ldr += LDR_RANGE_OK(s, 0x1000u) || LDR_RANGE_OK(s + 0xF00u, 256u);
        leak_guard += !FL_PROTECTED(s, 0x1000u) || !FL_PROTECTED(s + 0xF00u, 256u);
    }
    for (s = 0; s < 0x100000u; s += 0x1000u)
        allowed += FL_GUARD(s, 0x1000u);
    check("no protected sector passes the app's allow-list (erase, page)", leak_app == 0);
    check("no protected sector passes the loader's allow-list", leak_ldr == 0);
    check("the RAM driver's own net (FL_PROTECTED) covers every protected sector", leak_guard == 0);
    check("straddling ranges refused: 0xBF000+0x2000, 0x3000+0x2000",
          !FL_GUARD(0xBF000u, 0x2000u) && !LDR_RANGE_OK(0xBF000u, 0x2000u) &&
          !LDR_RANGE_OK(0x3000u, 0x2000u) && FL_PROTECTED(0xBF000u, 0x2000u) && FL_PROTECTED(0x3F00u, 0x200u));
    check("wrapping ranges refused (off + n past 4 GiB)",
          !FL_GUARD(0xBB000u, 0xFFFFF000u) && !LDR_RANGE_OK(0x9D000u, 0xFFFFFF00u) && FL_PROTECTED(0xBB000u, 0xFFFFF000u));
    check("app windows: FL_DATA 0x9D000-0xBAFFF, FL_OTA 0xBB000-0xBFFFF, FL_DATA2 0xC2000-0xE9FFF, nothing else",
          FL_GUARD(0x9D000u, 0x1000u) && FL_GUARD(0xBA000u, 0x1000u) && FL_GUARD(0xBF000u, 0x1000u) &&
          FL_GUARD(0xC2000u, 0x1000u) && FL_GUARD(0xE9000u, 0x1000u) && !FL_GUARD(0xEA000u, 0x1000u) &&
          !FL_GUARD(0x9C000u, 0x1000u) && !FL_GUARD(0xC1000u, 0x1000u) &&
          allowed == (0xC0000u - 0x9D000u) / 0x1000u + (0xEA000u - 0xC2000u) / 0x1000u);
    check("loader: app area 0x4000-0x9CFFF, records 0x9D000-0xBFFFF only",
          LDR_RANGE_OK(0x4000u, 0x1000u) && LDR_RANGE_OK(0x9C000u, 0x1000u) && LDR_RANGE_OK(0xBF000u, 0x1000u) &&
          !LDR_RANGE_OK(0xC0000u, 0x1000u) && FL_REC_HI == 0xC0000u);
    for (o = 0; o < OBJ_NOBJ; o++)          /* PERF-1: in one of the store's windows (ST_SMK_WIN), never the owner tag,
                                             * the stock loader's 0xBA000 or the update staging */
        for (c = 0; c < 2u; c++) {
            const uint32_t a = st_sector(o, c);
            uint32_t w, inw = 0;
            for (w = 0; w < sizeof ST_SMK_WIN / sizeof ST_SMK_WIN[0]; w++)
                inw |= a >= ST_SMK_WIN[w][0] && a < ST_SMK_WIN[w][1];
            in_store &= FL_STORE_OK(a, 0x1000u) && inw && a != ST_OWNER_OFF && a != 0xBA000u &&
                        !(a >= FL_OTA_LO && a < FL_OTA_HI);
        }
    {   /* every object's two copies distinct */
        uint32_t o2, c2, distinct = 1;
        for (o = 0; o < OBJ_NOBJ; o++)
            for (c = 0; c < 2u; c++)
                for (o2 = 0; o2 < OBJ_NOBJ; o2++)
                    for (c2 = 0; c2 < 2u; c2++)
                        if ((o != o2 || c != c2) && st_sector(o, c) == st_sector(o2, c2))
                            distinct = 0;
        check("storage.c: every object (A/B) in the store's windows, none on the tag, 0xBA000 or the staging, no two share a sector",
              in_store && distinct);
    }
    check("storage.c: project 0x9D000/0x9E000, panel 0x9F000/0xA0000, owner tag 0xAF000",
          st_sector(OBJ_PROJ, 0) == 0x9D000u && st_sector(OBJ_PROJ, 1) == 0x9E000u &&
          st_sector(OBJ_PANEL, 0) == 0x9F000u && st_sector(OBJ_PANEL, 1) == 0xA0000u && ST_OWNER_OFF == 0xAF000u);
    check("storage.c: KINDS and MMAP (FM-1 0xBF000, 0xC1000) moved off the staging and the calibration",
          st_sector(OBJ_KINDS, 0) == 0xD4000u && st_sector(OBJ_MMAP, 0) == 0xD6000u && st_sector(OBJ_BSTEP, 1) == 0xD9000u);
    check("FL_PLAIN_LO = the VM start 0x9D000", FL_PLAIN_LO == 0x9D000u);
    check("FL_JEDEC_OK: 0x856014 (the maker's SMK), 0xB36014, 0xEB6014 (the kit's) only",
          FL_JEDEC_OK(0xEB6014u) && FL_JEDEC_OK(0x856014u) && FL_JEDEC_OK(0xB36014u) &&
          !FL_JEDEC_OK(0u) && !FL_JEDEC_OK(0xFFFFFFu) && !FL_JEDEC_OK(0xEB6015u));

    /* the store owner (FELUCCA_DATA_FAMILY "PERF1", FELUCCA_DATA_UNTAGGED_OK 0) */
    {
        static const uint8_t TAG[16] = "SMK37DATAPERF1";
        char a[300], got[300];
        int n;
        memset(nor, 0xA5, sizeof nor);           /* everything outside the store: must stay */
        memset(nor + 0x9D000u, 0xFF, 0xBB000u - 0x9D000u);
        memset(nor + 0xBA000u, 0x5A, 0x1000u);   /* M-VAVE's own loader, left from a stock update: stays */
        memset(nor + 0xC2000u, 0x33, 0xEA000u - 0xC2000u);   /* the stock presets in USRFLASH: PERF's part cleared */
        memset(a, 'A', sizeof a);
        check("FELUCCA_DATA_UNTAGGED_OK defaults to 0, the family to PERF1",
              FELUCCA_DATA_UNTAGGED_OK == 0 && !strcmp(FELUCCA_DATA_FAMILY, "PERF1"));
        /* 1. untagged data (another firmware from before the tag, or stock VM data): cleared, tag written */
        memset(nor + 0x9D000u, 0x11, 0x1000u);
        memset(nor + 0xAE000u, 0x22, 0x1000u);
        st_owner_check();
        check("untagged store: PERF's windows cleared (0x9D000-0xAEFFF, 0xB0000-0xB9FFF, 0xC2000-0xD9FFF), tag PERF1 written",
              st_owner_cleared && all(0x9D000u, 0xAF000u, 0xFF) && all(0xB0000u, 0xBA000u, 0xFF) &&
              all(0xC2000u, 0xDA000u, 0xFF) && !memcmp(nor + 0xAF000u, TAG, 16));
        check("... and the stock loader at 0xBA000 and USRFLASH past 0xDA000 left as they were",
              all(0xBA000u, 0xBB000u, 0x5A) && all(0xDA000u, 0xEA000u, 0x33));
        /* 2. ours: kept */
        st_owner_cleared = 0;
        check("save a project", st_save(OBJ_PROJ, a, sizeof a) == 0);
        st_owner_check();
        n = st_load(OBJ_PROJ, got, sizeof got);
        check("our tag: nothing cleared, the project loads",
              !st_owner_cleared && n == (int)sizeof a && !memcmp(got, a, sizeof a));
        /* 3. another family's tag: cleared, taken over */
        memset(nor + 0xAF000u, 0xFF, 0x1000u);
        memcpy(nor + 0xAF000u, "SMK37DATASLOOP", 15);
        st_owner_check();
        check("another family (SLOOP): object area cleared, tag PERF1 written",
              st_owner_cleared && st_load(OBJ_PROJ, got, sizeof got) < 0 && all(0x9D000u, 0xAF000u, 0xFF) &&
              !memcmp(nor + 0xAF000u, TAG, 16));
        {   /* every object saved and loaded back on the simulated part */
            uint32_t ok = 1;
            for (o = 0; o < OBJ_NOBJ; o++) {
                memset(a, (int)('a' + o), sizeof a);
                ok &= st_save(o, a, sizeof a) == 0 && st_load(o, got, sizeof got) == (int)sizeof a && !memcmp(got, a, sizeof a);
            }
            for (o = 0; o < OBJ_NOBJ; o++) {     /* (and none overwrote another) */
                memset(a, (int)('a' + o), sizeof a);
                ok &= st_load(o, got, sizeof got) == (int)sizeof a && !memcmp(got, a, sizeof a);
            }
            check("all 26 objects save and load back, none over another", ok);
        }
        check("nothing outside the store's windows touched (stock loader 0xBA000, staging, BTIF, USRTRIM, USR), no write refused",
              all(0, 0x9D000u, 0xA5) && all(0xBA000u, 0xBB000u, 0x5A) && all(0xBB000u, 0xC2000u, 0xA5) &&
              all(0xDA000u, 0xEA000u, 0x33) && all(0xEA000u, 0x100000u, 0xA5) && bad_write == 0);
    }
    printf("%s\n", bad ? "SMK FLASH TEST FAILED" : "smk flash test passed");
    return bad != 0;
}
