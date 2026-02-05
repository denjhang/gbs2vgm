/*
 * gbs2vgm - VGM Writer for Game Boy Sound
 *
 * 2026 (C) Licensed under GNU GPL v1 or, at your option, any later version.
 */

#ifndef _VGM_WRITER_H_
#define _VGM_WRITER_H_

#include <stdint.h>
#include <stdio.h>

/* VGM Header structure (version 1.71) */
typedef struct {
	uint32_t ident;              /* 0x00: "Vgm " identifier */
	uint32_t eof_offset;         /* 0x04: EOF offset */
	uint32_t version;            /* 0x08: Version number */
	uint32_t sn76489_clock;      /* 0x0C: SN76489 clock */
	uint32_t ym2413_clock;       /* 0x10: YM2413 clock */
	uint32_t gd3_offset;         /* 0x14: GD3 offset */
	uint32_t total_samples;      /* 0x18: Total # samples */
	uint32_t loop_offset;        /* 0x1C: Loop offset */
	uint32_t loop_samples;       /* 0x20: Loop # samples */
	uint32_t rate;               /* 0x24: Rate */
	uint16_t sn76489_feedback;   /* 0x28: SN76489 feedback */
	uint8_t  sn76489_shift_width;/* 0x2A: SN76489 shift register width */
	uint8_t  sn76489_flags;      /* 0x2B: SN76489 flags */
	uint32_t ym2612_clock;       /* 0x2C: YM2612 clock */
	uint32_t ym2151_clock;       /* 0x30: YM2151 clock */
	uint32_t vgm_data_offset;    /* 0x34: VGM data offset */
	uint32_t sega_pcm_clock;     /* 0x38: Sega PCM clock */
	uint32_t spcm_interface;     /* 0x3C: SPCM Interface */
	uint32_t rf5c68_clock;       /* 0x40: RF5C68 clock */
	uint32_t ym2203_clock;       /* 0x44: YM2203 clock */
	uint32_t ym2608_clock;       /* 0x48: YM2608 clock */
	uint32_t ym2610_clock;       /* 0x4C: YM2610/B clock */
	uint32_t ym3812_clock;       /* 0x50: YM3812 clock */
	uint32_t ym3526_clock;       /* 0x54: YM3526 clock */
	uint32_t y8950_clock;        /* 0x58: Y8950 clock */
	uint32_t ymf262_clock;       /* 0x5C: YMF262 clock */
	uint32_t ymf278b_clock;      /* 0x60: YMF278B clock */
	uint32_t ymf271_clock;       /* 0x64: YMF271 clock */
	uint32_t ymz280b_clock;      /* 0x68: YMZ280B clock */
	uint32_t rf5c164_clock;      /* 0x6C: RF5C164 clock */
	uint32_t pwm_clock;          /* 0x70: PWM clock */
	uint32_t ay8910_clock;       /* 0x74: AY8910 clock */
	uint8_t  ay8910_chip_type;   /* 0x78: AY8910 chip type */
	uint8_t  ay8910_flags;       /* 0x79: AY8910 flags */
	uint8_t  ym2203_flags;       /* 0x7A: YM2203/YM2608/YM2610 flags */
	uint8_t  ym2151_flags;       /* 0x7B: YM2151 flags */
	uint32_t reserved_7c;        /* 0x7C: Reserved */
	uint32_t dmg_clock;          /* 0x80: Game Boy DMG clock */
	uint32_t nes_apu_clock;      /* 0x84: NES APU clock */
	uint32_t multipcm_clock;     /* 0x88: MultiPCM clock */
	uint32_t upd7759_clock;      /* 0x8C: uPD7759 clock */
	uint32_t okim6258_clock;     /* 0x90: OKIM6258 clock */
	uint8_t  okim6258_flags;     /* 0x94: OKIM6258 flags */
	uint8_t  k054539_flags;      /* 0x95: K054539 flags */
	uint8_t  c140_chip_type;     /* 0x96: C140 chip type */
	uint8_t  reserved1;          /* 0x97: Reserved */
	uint32_t okim6295_clock;     /* 0x98: OKIM6295 clock */
	uint32_t k051649_clock;      /* 0x98: K051649 clock */
	uint32_t k054539_clock;      /* 0x9C: K054539 clock */
	uint32_t huc6280_clock;      /* 0xA0: HuC6280 clock */
	uint32_t c140_clock;         /* 0xA4: C140 clock */
	uint32_t k053260_clock;      /* 0xA8: K053260 clock */
	uint32_t pokey_clock;        /* 0xAC: Pokey clock */
	uint32_t qsound_clock;       /* 0xB0: QSound clock */
	uint32_t scsp_clock;         /* 0xB4: SCSP clock */
	uint32_t extra_hdr_offset;   /* 0xB8: Extra header offset */
	uint32_t wonderswan_clock;   /* 0xBC: WonderSwan clock */
	uint32_t vsu_clock;          /* 0xC0: VSU clock */
	uint32_t saa1099_clock;      /* 0xC4: SAA1099 clock */
	uint32_t es5503_clock;       /* 0xC8: ES5503 clock */
	uint32_t es5505_clock;       /* 0xCC: ES5505/ES5506 clock */
	uint8_t  es5503_chans;       /* 0xD0: ES5503 output channels */
	uint8_t  es5505_chans;       /* 0xD1: ES5505/ES5506 output channels */
	uint8_t  c352_clock_divider; /* 0xD2: C352 clock divider */
	uint8_t  reserved2;          /* 0xD3: Reserved */
	uint32_t x1_010_clock;       /* 0xD4: X1-010 clock */
	uint32_t c352_clock;         /* 0xD8: C352 clock */
	uint32_t ga20_clock;         /* 0xDC: GA20 clock */
	uint8_t  reserved3[0x100-0xE0]; /* 0xE0-0xFF: Reserved */
} VGM_HEADER;

/* VGM Writer context */
typedef struct {
	FILE *file;
	VGM_HEADER header;
	uint32_t sample_count;
	uint32_t command_count;
	long data_start_pos;
	long loop_pos;              /* Position where loop starts */
	uint32_t loop_sample_count; /* Sample count at loop point */

	/* GD3 tag data */
	char *track_name_en;
	char *track_name_jp;
	char *game_name_en;
	char *game_name_jp;
	char *system_name_en;
	char *system_name_jp;
	char *author_name_en;
	char *author_name_jp;
	char *release_date;
	char *vgm_creator;
	char *notes;
} vgm_writer_t;

/* Initialize VGM writer */
vgm_writer_t *vgm_writer_init(const char *filename, uint32_t gb_clock);

/* Set GD3 tag information */
void vgm_set_gd3_info(vgm_writer_t *vgm,
                      const char *track_name,
                      const char *game_name,
                      const char *author_name,
                      const char *release_date,
                      const char *ripper,
                      const char *notes);

/* Mark loop point */
void vgm_mark_loop_point(vgm_writer_t *vgm);

/* Write Game Boy register */
void vgm_write_gb_reg(vgm_writer_t *vgm, uint8_t reg, uint8_t data);

/* Write wait command */
void vgm_write_wait(vgm_writer_t *vgm, uint32_t samples);

/* Finalize and close VGM file */
void vgm_writer_close(vgm_writer_t *vgm);

#endif /* _VGM_WRITER_H_ */
