/* SPDX-License-Identifier: GPL-3.0-only */
/* AMB-1 platform interface (from X0X's): everything the app needs from the hardware.
 *
 * The device implementation (plat_fm1.c) wraps Felucca's HAL and panel map; the
 * host implementation (host/amb_host.c) is a simulator fed by a script, which is
 * what lets the whole app — UI, sequencer, engines — run and be tested on a Mac.
 * Drawing goes through Felucca's lcd_fill / lcd_blit (lcd.c on the device, a
 * framebuffer on the host) and gfx.c on top. */
#pragma once
#include <stdint.h>

/* big zero-initialised buffers go in the .pool RAM region on the device */
#ifdef EN_HOST
#define EN_POOL
#else
#define EN_POOL __attribute__((section(".pool")))
#endif

/* panel controls, by printed label (Felucca's panel.c map turns these into matrix ids; Felucca
 * calls the second button SCL, the panel prints SEL) */
enum { B_FX, B_SEL, B_ENV, B_LFO, B_EDIT, B_GLO, B_HOME, B_SAVE, B_ARP, B_SEQ, B_PLAY, B_REC,
       B_OCTDN, B_OCTUP, NB };
enum { EN_SELECT, EN_ALGO, EN_PRESET, EN_K1, EN_K2, EN_K3, EN_K4, NE };

#define NKEYS 27                       /* F3 .. G5 */

uint32_t plat_ms(void);
uint32_t plat_buttons(void);           /* held: bit B_* */
uint32_t plat_keys(void);              /* held: bit k = key k (0 = F3) */
int32_t plat_enc(int role);            /* detents since the last call, + = clockwise; EN_* */
int plat_usb_power(void);              /* a USB host has the unit configured (it is powered, and charging, from it) */
uint32_t plat_battery(void);           /* the battery divider's ADC reading, 0..1023 (smoothed, about a second), 0xFFFFFFFF when there is none */
uint32_t plat_master(void);            /* MASTER pot, 0..4096 gain Q12 */
uint32_t plat_seed(void);              /* at power-on: a seed, different each time (0 on the host: no chance) */
void plat_leds(uint32_t buttons, uint32_t keys);   /* lit: bit B_*, bit key */
void plat_glow(int on);                /* the unlit buttons glow dimly, readable in the dark (call before plat_leds) */
void plat_keys_dim(uint32_t keys);     /* these keys lit dim, for the next plat_leds only (0.15.21: the scale dim, the chord playing bright) */
void plat_play_red(int red);           /* PLAY lights green; red (its other LED) while recording (before plat_leds) */

/* USB MIDI, 4-byte USB-MIDI event packets (cable 0) */
int plat_midi_in(uint32_t *pkt);       /* 1 = got one (called from the audio ISR) */
void plat_midi_out(uint32_t pkt);      /* queue one (audio ISR or main loop) */
void plat_usb_host(int on);            /* the USB port as the host of a USB-MIDI device (on), or a device again (off); never kept */
int plat_usb_host_msg(void);           /* what the host has to say since the last call: 0 nothing, PLAT_UH_* */
int plat_usb_host_state(void);         /* 0 off, 1 a host looking for a device, 2 a device connected, 3 a device found with no MIDI (or no answer) */
void plat_usb_host_diag(char *b);      /* what its last search found, in a few words (up to 24 characters): "HUB 4 PORTS, HUB INSIDE" */
enum { PLAT_UH_NONE, PLAT_UH_ON, PLAT_UH_OFF, PLAT_UH_MIDI, PLAT_UH_NOMIDI, PLAT_UH_LOW, PLAT_UH_LOST };   /* (usb_host.c's UH_MSG_*) */

/* persistent storage (Felucca storage.c objects: A/B sectors, CRC, torn-write safe) */
int plat_store_load(uint32_t obj, void *dst, uint32_t max);   /* bytes loaded, < 0 = none / bad */
int plat_store_save(uint32_t obj, const void *src, uint32_t len);   /* 0 = ok */
int plat_store_restore(uint32_t obj, const void *src, uint32_t len);   /* a restored backup's object: written, and no other store is written after it (the unit restarts) */
int plat_store_locked(void);
int plat_store_seamless(void);        /* a flash write does not stop the audio (the erase goes in slices with the interrupts on between): no fade around it */
#define PLAT_STORE_MAX 3840u           /* bytes per object */
enum { OBJ_PROJ, OBJ_PANEL, OBJ_SET, OBJ_VOICES, OBJ_GLOB, OBJ_G1, OBJ_G2, OBJ_G3, OBJ_G4, OBJ_G5, OBJ_G6, OBJ_G7, OBJ_G8, OBJ_G9, OBJ_G10, OBJ_G11, OBJ_G12, OBJ_G13, OBJ_G14, OBJ_G15, OBJ_G16, OBJ_G17, OBJ_G18, OBJ_KINDS, OBJ_MMAP, OBJ_BSTEP, OBJ_NOBJ };   /* (OBJ_BSTEP, 0.16.29: the pieces' bass steps, 0xC3000) */   /* (OBJ_G16: industrial, 0.15.14: 0xB9000; G17 psy, G18 dark forest, 0.16.2: 0xBB000, 0xBD000) */   /* OBJ_GLOB: Tune, MIDI out, the theme (0x99000); OBJ_G1 .. G9: genres 1 to 9's saved pieces (0x9B000 on, 2 sectors each; the original genre's is OBJ_PROJ) */   /* OBJ_SET: the effects' settings and the performing settings (0xDE000, 2 sectors); OBJ_VOICES: the player's DX7 voices (0x97000, 2 sectors) */
/* flash: OBJ_PROJ 0xFC000, OBJ_PANEL 0xDC000 (2 sectors each, inside Felucca's data area); OBJ_KINDS (0.16.6: the progression slots' chord types
 * of every genre's pieces, project.c) 0xBF000, after the last genre's; OBJ_MMAP (0.16.11: the player's MIDI map, ui.c) 0xC1000 */

/* audio load in % of the render budget, and xruns since boot (device: from the ISR) */
uint32_t plat_cpu_pct(void);
uint32_t plat_xruns(void);                  /* halves that were never rendered (their buffer played again): the real dropouts */
uint32_t plat_late(void);                   /* renders that ran past their half (the next one started at once: nothing lost, but the margin is thin) */
uint32_t plat_peak_pct(void);               /* the longest render of the last second or two, in percent of the time it has: 100 is a late render */
uint32_t plat_cpu1_pct(void);                /* the other core's load, in percent of a chunk (0 when it is not there) */
uint32_t plat_core2(void);                  /* the second core: 0 not in this build, 1 reverbing, 2 not (lost, or barred after a crash) */
uint32_t plat_xruns(void);

#if defined(BOARD_SMK37) && BOARD_SMK37
/* The SMK-37 build: the M-Vave SMK-37's controls beyond the FM-1's (plat_fm1.c over hal/smk_input.h). plat_keys()
 * above is the keybed's F3..G5, the FM-1's 27 keys; the pads are not keys here. Pads are numbered as the hardware:
 * 0..7 the top row, 8..15 the bottom row, left to right. */
#define SMK_NKEYBED 37                 /* C3 .. C6 */
#define SMK_NPADS 16
enum { SB_KNOB_BANK, SB_FADER_BANK, SB_LEFT, SB_RIGHT, NSB };   /* the four buttons the FM-1 has not */
uint32_t plat_smk_buttons(void);       /* held: bit SB_* */
void plat_smk_leds(uint32_t lit);      /* their LEDs, bit SB_* (before plat_leds) */
uint64_t plat_keybed(void);            /* keybed keys held: bit k = key k (0 = C3) */
uint64_t plat_keybed_hits(void);       /* keybed presses since the last call */
uint32_t plat_pad_hits(void);          /* pad presses since the last call: bit p = pad p */
int plat_pad_vel(int pad);             /* velocity 1..127 of pad p's last press */
int plat_fader(int i);                 /* fader 0..3: 0..127, -1 while the readings settle */
int plat_wheels(int32_t *pitch, int32_t *modw);   /* 1 = new values: pitch -127..127 (0 at rest), mod 0..127 */
void plat_pad_rgb(int pad, uint8_t r, uint8_t g, uint8_t b);   /* pad p's LED */
/* the HW TEST screen (app/smk_hwtest.c): every input as the HAL sees it */
typedef struct {
    uint64_t keys;                     /* keybed keys held, bit k = key k (0 = C3) */
    uint8_t key_vel[37];               /* each key's last velocity 1..127 */
    uint8_t pad_press[16];             /* 0 = off, else the pressure 1..127 (pads as the hardware: 0..7 top row) */
    uint16_t pad_raw[16];              /* the pad's ADC reading 0..1023 */
    uint8_t fader[4];                  /* 0..127 */
    int16_t enc[8];                    /* encoder transitions since boot, + = clockwise (B leads) */
    uint16_t wheel[2], pedal, batt;    /* raw 0..1023: pitch, mod, pedal, battery */
    uint32_t buttons;                  /* bit i = button id i (smk_input.h: 0 KNOB BANK .. 17 SEQ REC) */
    uint32_t frames, adc_ok, adc_miss;
    uint8_t trim_ok, flash_chip;       /* the factory curves taken; fm1_flash_id.h FL_CHIP */
    uint16_t hit_raw[8], hit_base;     /* the last pad hit: its first 8 readings (2.4 ms apart) and the baseline */
    uint8_t hit_pad, hit_n;            /* which pad (0xFF none yet), readings taken */
    uint16_t pad_t0;                   /* the factory pad curve's first point (velocity 1 at or below it) */
} smk_hw_t;
void plat_smk_hw(smk_hw_t *h);
void plat_smk_btn_leds(uint32_t ids);  /* button LEDs by hardware id (bit i = button id i), for the test screen */
uint32_t plat_smk_pads_held(void);     /* pads held now: bit p = pad p (0..7 the top row) */
int plat_smk_key_vel(int k);           /* keybed key k's last velocity 1..127 (0 = C3) */
int32_t plat_smk_extkey(void);         /* the next outer-key event (keys 0-4, 32-36): key << 8 | velocity (0 up), -1 none */
#endif
