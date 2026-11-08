/* SPDX-License-Identifier: GPL-3.0-only
 * Copyright (C) 2026 Leo Kuroshita (@kurogedelic), Hügelton Instruments
 * X0X / AMB-1: object map changed (see below). SMK-37 map and store owner: M. Geisler. */
/* Persistent storage on the SPI NOR.
 *
 * Every object has an A/B sector pair. A save goes to the copy that is not
 * the current one: erase the sector, program the payload (from offset 256),
 * then the 32-byte header at offset 0 last. The header is the commit record;
 * on load the valid copy with the highest seq wins, so a torn write leaves
 * the previous copy in charge.
 *
 * Flash access goes through three hooks (also used by the host test):
 *   st_read(off, dst, n)   st_erase(off)   st_prog(off, src, n)
 */
#define ST_MAGIC 0x30583058u                   /* "X0X0": never mistaken for a Felucca object */
#define ST_SECTOR 4096u
#define ST_PAYLOAD_OFF 256u
#define ST_PAYLOAD_MAX (ST_SECTOR - ST_PAYLOAD_OFF)

/* flash map, AMB-1 (plat.h OBJ_*): the project at 0xFC000 (Felucca's FL_GLOB), the panel calibration at
 * 0xDC000 (Felucca's user-preset area). Nothing above 0xDFFFF: 0xE0000.. is the update loader's
 * staging area. The magic is X0X's, so a project is told apart by its own magic (project.c). */
typedef struct {
    uint32_t magic;
    uint16_t type, slot;
    uint32_t seq, len, crc, rsv[2];
    uint32_t hcrc;
} st_hdr_t;

static int st_read(uint32_t off, void *dst, uint32_t n);
static int st_erase(uint32_t off);
static int st_prog(uint32_t off, const void *src, uint32_t n);

static uint32_t st_crc32(const void *p, uint32_t n)   /* zlib CRC-32, 4 bits per step */
{
    static const uint32_t T[16] = {
        0x00000000u, 0x1DB71064u, 0x3B6E20C8u, 0x26D930ACu, 0x76DC4190u, 0x6B6B51F4u, 0x4DB26158u, 0x5005713Cu,
        0xEDB88320u, 0xF00F9344u, 0xD6D6A3E8u, 0xCB61B38Cu, 0x9B64C2B0u, 0x86D3D2D4u, 0xA00AE278u, 0xBDBDF21Cu};
    const uint8_t *b = p;
    uint32_t c = 0xFFFFFFFFu;
    while (n--) {
        c ^= *b++;
        c = (c >> 4) ^ T[c & 15u];
        c = (c >> 4) ^ T[c & 15u];
    }
    return ~c;
}

#if BOARD_SMK37
/* PERF-1 SMK (SMK-37 Pro): 26 objects, A/B neighbours, 52 sectors, placed by enum (never by address math) in
 * three windows of the flash map (fm1_flash_map.h FL_DATA, FL_DATA2):
 *   0x9D000..0xAEFFF  the object area every SMK port uses: PROJ PANEL SET VOICES GLOB G1..G4   (18 sectors)
 *   0xAF000           the store's owner tag ("SMK37DATA" + "PERF1"), below
 *   0xB0000..0xB9FFF  G5..G9                                                                    (10 sectors)
 *   0xBA000           left alone: M-VAVE's own update parks its loader here (found in the maker's dump)
 *   0xBB000..0xBFFFF  the update staging (ota.c), not the store
 *   0xC2000..0xD9FFF  G10..G18, KINDS, MMAP, BSTEP: USRFLASH (the stock presets; back with the stock package)
 *   0xDA000..0xE9FFF  free: 16 sectors, 8 objects
 * Never BTIF / USRTRIM 0xC0000..0xC1FFF (pairing, the factory key / pad / wheel calibration), the stock
 * resources 0xF4000..0xFDFFF or key_mac 0xFF000. tests/smk_flash_test.c walks every object against the map. */
#define ST_SMK_BASE 0x9D000u
#define ST_SMK_OBJ_HI 0xAF000u                 /* the object area every SMK port shares (0x9D000..0xAEFFF) */
static const uint8_t ST_SMK_SEC[OBJ_NOBJ] = {  /* copy A's sector number (offset >> 12); copy B is the next */
    [OBJ_PROJ] = 0x9D, [OBJ_PANEL] = 0x9F, [OBJ_SET] = 0xA1, [OBJ_VOICES] = 0xA3, [OBJ_GLOB] = 0xA5,
    [OBJ_G1] = 0xA7, [OBJ_G2] = 0xA9, [OBJ_G3] = 0xAB, [OBJ_G4] = 0xAD,
    [OBJ_G5] = 0xB0, [OBJ_G6] = 0xB2, [OBJ_G7] = 0xB4, [OBJ_G8] = 0xB6, [OBJ_G9] = 0xB8,
    [OBJ_G10] = 0xC2, [OBJ_G11] = 0xC4, [OBJ_G12] = 0xC6, [OBJ_G13] = 0xC8, [OBJ_G14] = 0xCA, [OBJ_G15] = 0xCC,
    [OBJ_G16] = 0xCE, [OBJ_G17] = 0xD0, [OBJ_G18] = 0xD2, [OBJ_KINDS] = 0xD4, [OBJ_MMAP] = 0xD6, [OBJ_BSTEP] = 0xD8};
_Static_assert(OBJ_NOBJ == 26, "a new object needs its place in ST_SMK_SEC (and in tests/smk_flash_test.c)");
static uint32_t st_sector(uint32_t obj, uint32_t copy)  /* flash offset of copy A (0) / B (1) */
{
    return ((uint32_t)ST_SMK_SEC[obj % OBJ_NOBJ] + (copy & 1u)) * ST_SECTOR;
}
/* the windows the store owns: cleared when it takes the store over */
static const uint32_t ST_SMK_WIN[][2] = {{0x9D000u, 0xAF000u}, {0xB0000u, 0xBA000u}, {0xC2000u, 0xDA000u}};

/* The owner of the store. Several FM-1 firmwares are ported to the SMK-37 Pro with the same board layer and
 * this same store area, each with its own object formats: a firmware must never parse another's data. The
 * sector after the object area (0xAF000) holds "SMK37DATA" + the firmware family (FELUCCA_DATA_FAMILY,
 * build.py; AMB-1: "AMB1"). Ours: load as ever. None (erased, data from before the tag) or another
 * family's: clear the whole object area 0x9D000..0xAEFFF and take the store (FELUCCA_DATA_UNTAGGED_OK = 0:
 * AMB-1 never trusts untagged data). Calibration, BT pairing and key_mac are outside FL_DATA: never touched.
 * ST_IDLE() runs between the erases (the device: the watchdog). */
#ifndef FELUCCA_DATA_FAMILY
#define FELUCCA_DATA_FAMILY "PERF1"
#endif
#ifndef FELUCCA_DATA_UNTAGGED_OK
#define FELUCCA_DATA_UNTAGGED_OK 0
#endif
#ifndef ST_IDLE
#define ST_IDLE() ((void)0)
#endif
#define ST_OWNER_OFF ST_SMK_OBJ_HI
_Static_assert(ST_OWNER_OFF + ST_SECTOR <= 0xBB000u, "SMK store owner: past FL_DATA");
static uint8_t st_owner_cleared;                /* (the UI may say so) */
static void st_owner_check(void)
{
    static const char FAM[] = FELUCCA_DATA_FAMILY;
    uint8_t tag[16], want[16];
    uint32_t i, erased = 1, ours = 1, s;
    _Static_assert(sizeof FAM >= 2u && sizeof FAM <= 8u, "FELUCCA_DATA_FAMILY: 1..7 characters");
    for (i = 0; i < 16u; i++)
        want[i] = i < 9u ? (uint8_t)"SMK37DATA"[i] : i - 9u < sizeof FAM - 1u ? (uint8_t)FAM[i - 9u] : 0u;
    if (st_read(ST_OWNER_OFF, tag, 16))
        return;
    for (i = 0; i < 16u; i++) {
        erased &= tag[i] == 0xFFu;
        ours &= tag[i] == want[i];
    }
    if (ours)
        return;
    if (!erased || !FELUCCA_DATA_UNTAGGED_OK) {     /* another firmware's store (or untagged): clear PERF's windows */
        uint32_t w, k;
        for (w = 0; w < sizeof ST_SMK_WIN / sizeof ST_SMK_WIN[0]; w++)
            for (s = ST_SMK_WIN[w][0]; s < ST_SMK_WIN[w][1]; s += ST_SECTOR) {
                uint32_t blank = 1;                  /* (an erased sector stays as it is: the first boot is quicker) */
                for (k = 0; k < ST_SECTOR && blank; k += 64u) {
                    uint32_t q[16], j;
                    if (st_read(s + k, q, sizeof q)) {
                        blank = 0;
                        break;
                    }
                    for (j = 0; j < 16u; j++)
                        blank &= q[j] == 0xFFFFFFFFu;
                }
                ST_IDLE();
                if (!blank)
                    st_erase(s);
            }
        st_owner_cleared = 1;
    }
    ST_IDLE();
    st_erase(ST_OWNER_OFF);
    st_prog(ST_OWNER_OFF, want, 16);
}
#else
static uint32_t st_sector(uint32_t obj, uint32_t copy)  /* flash offset of copy A (0) / B (1) */
{
    const uint32_t base = obj == OBJ_BSTEP ? 0xC3000u : obj == OBJ_MMAP ? 0xC1000u : obj == OBJ_KINDS ? 0xBF000u : obj == OBJ_PANEL ? 0xDC000u : obj == OBJ_SET ? 0xDE000u : obj == OBJ_VOICES ? 0x97000u : obj == OBJ_GLOB ? 0x99000u :
                          obj >= OBJ_G1 ? 0x9B000u + (obj - OBJ_G1) * 2u * ST_SECTOR : 0xFC000u;   /* (the genres' pieces, the player's three: the free flash 0x99000 .. 0xDBFFF) */
    return base + copy * ST_SECTOR;
}
#endif

static uint8_t st_buf[ST_PAYLOAD_MAX] __attribute__((aligned(4)));

static int st_head(uint32_t obj, uint32_t copy, st_hdr_t *h)   /* commit record valid: 0 */
{
    if (st_read(st_sector(obj, copy), h, sizeof *h))
        return -1;
    if (h->magic != ST_MAGIC || h->type != obj || h->len > ST_PAYLOAD_MAX ||
        h->hcrc != st_crc32(h, sizeof *h - 4u))
        return -1;
    return 0;
}

static int st_body(uint32_t obj, uint32_t copy, const st_hdr_t *h)   /* payload -> st_buf, CRC ok: 0 */
{
    if (st_read(st_sector(obj, copy) + ST_PAYLOAD_OFF, st_buf, h->len) || st_crc32(st_buf, h->len) != h->crc)
        return -1;
    return 0;
}

/* the current copy: the valid one with the highest seq (A on a tie), -1 when
 * neither is valid. Headers first, so only the winner's payload is read (it
 * is left in st_buf); *h gets its header. */
static int st_current(uint32_t obj, st_hdr_t *h)
{
    st_hdr_t a, b;
    int va = st_head(obj, 0, &a) == 0, vb = st_head(obj, 1, &b) == 0;
    if (vb && (!va || b.seq > a.seq)) {
        if (st_body(obj, 1, &b) == 0) {
            *h = b;
            return 1;
        }
        vb = 0;
    }
    if (va && st_body(obj, 0, &a) == 0) {
        *h = a;
        return 0;
    }
    if (vb && st_body(obj, 1, &b) == 0) {
        *h = b;
        return 1;
    }
    return -1;
}

/* load object into dst (up to max bytes); returns the length, or -1 */
static int st_load(uint32_t obj, void *dst, uint32_t max)
{
    uint32_t i;
    st_hdr_t h;
    if (st_current(obj, &h) < 0)
        return -1;
    if (h.len > max)
        h.len = max;
    for (i = 0; i < h.len; i++)
        ((uint8_t *)dst)[i] = st_buf[i];
    return (int)h.len;
}

static int st_save(uint32_t obj, const void *src, uint32_t len)
{
    uint32_t seq, base, off;
    int cur, rc;
    st_hdr_t h;
    if (len > ST_PAYLOAD_MAX)
        return -1;
    cur = st_current(obj, &h);
    seq = cur < 0 ? 0u : h.seq;
    base = st_sector(obj, cur == 0 ? 1u : 0u);       /* write the other copy */
    for (off = 0; off < len; off++)
        st_buf[off] = ((const uint8_t *)src)[off];    /* the driver wants RAM sources */
    if ((rc = st_erase(base)) != 0)
        return rc;
    for (off = 0; off < len; off += 256u) {
        uint32_t n = len - off > 256u ? 256u : len - off;
        if ((rc = st_prog(base + ST_PAYLOAD_OFF + off, st_buf + off, n)) != 0)
            return rc;
    }
    h.magic = ST_MAGIC;
    h.type = (uint16_t)obj;
    h.slot = (uint16_t)(cur == 0 ? 1 : 0);
    h.seq = seq + 1u;
    h.len = len;
    h.crc = st_crc32(st_buf, len);
    h.rsv[0] = h.rsv[1] = 0xFFFFFFFFu;
    h.hcrc = st_crc32(&h, sizeof h - 4u);
    if ((rc = st_prog(base, &h, sizeof h)) != 0)       /* the commit record, last */
        return rc;
    {   /* read back: a write-protected or failing part must not report SAVED */
        st_hdr_t chk;
        uint32_t c = cur == 0 ? 1u : 0u;
        if (st_head(obj, c, &chk) || chk.seq != h.seq || st_body(obj, c, &chk))
            return -7;
    }
    return 0;
}
