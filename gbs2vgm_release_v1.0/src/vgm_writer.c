/*
 * gbs2vgm - VGM Writer for Game Boy Sound
 *
 * 2026 (C) Licensed under GNU GPL v1 or, at your option, any later version.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <wchar.h>
#include "vgm_writer.h"

#define VGM_IDENT 0x206D6756  /* "Vgm " */
#define VGM_VERSION 0x00000171 /* Version 1.71 */
#define GD3_IDENT 0x20336447  /* "Gd3 " */
#define GD3_VERSION 0x00000100 /* Version 1.00 */

/* VGM command codes */
#define VGM_CMD_GB_WRITE    0xB3  /* Game Boy DMG write: 0xB3 aa dd */
#define VGM_CMD_WAIT_NNNN   0x61  /* Wait n samples: 0x61 nn nn */
#define VGM_CMD_WAIT_735    0x62  /* Wait 735 samples (1/60 sec at 44.1kHz) */
#define VGM_CMD_WAIT_882    0x63  /* Wait 882 samples (1/50 sec at 44.1kHz) */
#define VGM_CMD_END         0x66  /* End of sound data */

/* Helper function to write little-endian values */
static void write_le32(FILE *f, uint32_t val) {
	fputc(val & 0xFF, f);
	fputc((val >> 8) & 0xFF, f);
	fputc((val >> 16) & 0xFF, f);
	fputc((val >> 24) & 0xFF, f);
}

static void write_le16(FILE *f, uint16_t val) {
	fputc(val & 0xFF, f);
	fputc((val >> 8) & 0xFF, f);
}

/* Write UTF-16LE string */
static void write_utf16le_string(FILE *f, const char *str) {
	if (!str) str = "";

	/* Simple ASCII to UTF-16LE conversion */
	while (*str) {
		write_le16(f, (uint16_t)(unsigned char)*str);
		str++;
	}
	/* Null terminator */
	write_le16(f, 0);
}

/* Write GD3 tag */
static void write_gd3_tag(vgm_writer_t *vgm, long gd3_pos) {
	long start_pos, end_pos;
	uint32_t gd3_length;

	if (!vgm || !vgm->file)
		return;

	start_pos = ftell(vgm->file);

	/* Write GD3 header */
	write_le32(vgm->file, GD3_IDENT);
	write_le32(vgm->file, GD3_VERSION);

	/* Placeholder for length */
	long length_pos = ftell(vgm->file);
	write_le32(vgm->file, 0);

	/* Write strings (all in UTF-16LE) */
	write_utf16le_string(vgm->file, vgm->track_name_en);  /* Track name (English) */
	write_utf16le_string(vgm->file, vgm->track_name_jp);  /* Track name (Japanese) */
	write_utf16le_string(vgm->file, vgm->game_name_en);   /* Game name (English) */
	write_utf16le_string(vgm->file, vgm->game_name_jp);   /* Game name (Japanese) */
	write_utf16le_string(vgm->file, vgm->system_name_en); /* System name (English) */
	write_utf16le_string(vgm->file, vgm->system_name_jp); /* System name (Japanese) */
	write_utf16le_string(vgm->file, vgm->author_name_en); /* Author name (English) */
	write_utf16le_string(vgm->file, vgm->author_name_jp); /* Author name (Japanese) */
	write_utf16le_string(vgm->file, vgm->release_date);   /* Release date */
	write_utf16le_string(vgm->file, vgm->vgm_creator);    /* VGM creator */
	write_utf16le_string(vgm->file, vgm->notes);          /* Notes */

	end_pos = ftell(vgm->file);
	gd3_length = end_pos - start_pos - 12; /* Exclude header */

	/* Update length */
	fseek(vgm->file, length_pos, SEEK_SET);
	write_le32(vgm->file, gd3_length);
	fseek(vgm->file, end_pos, SEEK_SET);

	/* Update GD3 offset in header */
	vgm->header.gd3_offset = gd3_pos - 0x14;
}

vgm_writer_t *vgm_writer_init(const char *filename, uint32_t gb_clock) {
	vgm_writer_t *vgm;
	int i;

	vgm = (vgm_writer_t *)calloc(1, sizeof(vgm_writer_t));
	if (!vgm)
		return NULL;

	vgm->file = fopen(filename, "wb");
	if (!vgm->file) {
		free(vgm);
		return NULL;
	}

	/* Initialize header */
	memset(&vgm->header, 0, sizeof(VGM_HEADER));
	vgm->header.ident = VGM_IDENT;
	vgm->header.version = VGM_VERSION;
	vgm->header.dmg_clock = gb_clock;  /* Game Boy clock (typically 4194304 Hz) */
	vgm->header.vgm_data_offset = 0x100 - 0x34;  /* Data starts at 0x100 */

	/* Write placeholder header (will be updated on close) */
	for (i = 0; i < 0x100; i++) {
		fputc(0, vgm->file);
	}

	vgm->data_start_pos = ftell(vgm->file);
	vgm->sample_count = 0;
	vgm->command_count = 0;
	vgm->loop_pos = -1;
	vgm->loop_sample_count = 0;

	/* Initialize GD3 strings */
	vgm->system_name_en = strdup("Game Boy");
	vgm->vgm_creator = strdup("Denjhang");
	vgm->notes = strdup("gbs2vgm by Claude & Denjhang");

	return vgm;
}

void vgm_set_gd3_info(vgm_writer_t *vgm,
                      const char *track_name,
                      const char *game_name,
                      const char *author_name,
                      const char *release_date,
                      const char *ripper,
                      const char *notes) {
	if (!vgm)
		return;

	if (track_name) {
		free(vgm->track_name_en);
		vgm->track_name_en = strdup(track_name);
	}
	if (game_name) {
		free(vgm->game_name_en);
		vgm->game_name_en = strdup(game_name);
	}
	if (author_name) {
		free(vgm->author_name_en);
		vgm->author_name_en = strdup(author_name);
	}
	if (release_date) {
		free(vgm->release_date);
		vgm->release_date = strdup(release_date);
	}
	if (ripper) {
		free(vgm->vgm_creator);
		vgm->vgm_creator = strdup(ripper);
	}
	if (notes) {
		free(vgm->notes);
		vgm->notes = strdup(notes);
	}
}

void vgm_mark_loop_point(vgm_writer_t *vgm) {
	if (!vgm || !vgm->file)
		return;

	vgm->loop_pos = ftell(vgm->file);
	vgm->loop_sample_count = vgm->sample_count;
}

void vgm_write_gb_reg(vgm_writer_t *vgm, uint8_t reg, uint8_t data) {
	if (!vgm || !vgm->file)
		return;

	/* Write Game Boy register command: 0xB3 aa dd */
	fputc(VGM_CMD_GB_WRITE, vgm->file);
	fputc(reg, vgm->file);
	fputc(data, vgm->file);

	vgm->command_count++;
}

void vgm_write_wait(vgm_writer_t *vgm, uint32_t samples) {
	if (!vgm || !vgm->file || samples == 0)
		return;

	/* Use optimized wait commands when possible */
	while (samples > 0) {
		uint32_t wait_samples;

		if (samples == 735) {
			fputc(VGM_CMD_WAIT_735, vgm->file);
			wait_samples = 735;
			samples = 0;
		} else if (samples == 882) {
			fputc(VGM_CMD_WAIT_882, vgm->file);
			wait_samples = 882;
			samples = 0;
		} else if (samples <= 16) {
			/* Short wait: 0x7n = wait n+1 samples (0x70-0x7F) */
			fputc(0x70 + (samples - 1), vgm->file);
			wait_samples = samples;
			samples = 0;
		} else if (samples <= 65535) {
			/* Wait n samples: 0x61 nn nn */
			fputc(VGM_CMD_WAIT_NNNN, vgm->file);
			write_le16(vgm->file, (uint16_t)samples);
			wait_samples = samples;
			samples = 0;
		} else {
			/* Split large waits */
			fputc(VGM_CMD_WAIT_NNNN, vgm->file);
			write_le16(vgm->file, 0xFFFF);
			wait_samples = 0xFFFF;
			samples -= 0xFFFF;
		}

		vgm->sample_count += wait_samples;
	}
}

void vgm_writer_close(vgm_writer_t *vgm) {
	long end_pos, gd3_pos;

	if (!vgm)
		return;

	if (vgm->file) {
		/* Write end of sound data command */
		fputc(VGM_CMD_END, vgm->file);

		/* Write GD3 tag if we have metadata */
		gd3_pos = ftell(vgm->file);
		if (vgm->track_name_en || vgm->game_name_en || vgm->author_name_en) {
			write_gd3_tag(vgm, gd3_pos);
		}

		/* Get file size */
		end_pos = ftell(vgm->file);

		/* Update header */
		vgm->header.eof_offset = end_pos - 4;
		vgm->header.total_samples = vgm->sample_count;

		/* Set loop info if loop point was marked */
		if (vgm->loop_pos >= 0) {
			vgm->header.loop_offset = vgm->loop_pos - 0x1C;
			vgm->header.loop_samples = vgm->sample_count - vgm->loop_sample_count;
		}

		/* Write updated header */
		fseek(vgm->file, 0, SEEK_SET);

		write_le32(vgm->file, vgm->header.ident);
		write_le32(vgm->file, vgm->header.eof_offset);
		write_le32(vgm->file, vgm->header.version);
		write_le32(vgm->file, vgm->header.sn76489_clock);
		write_le32(vgm->file, vgm->header.ym2413_clock);
		write_le32(vgm->file, vgm->header.gd3_offset);
		write_le32(vgm->file, vgm->header.total_samples);
		write_le32(vgm->file, vgm->header.loop_offset);
		write_le32(vgm->file, vgm->header.loop_samples);
		write_le32(vgm->file, vgm->header.rate);
		write_le16(vgm->file, vgm->header.sn76489_feedback);
		fputc(vgm->header.sn76489_shift_width, vgm->file);
		fputc(vgm->header.sn76489_flags, vgm->file);
		write_le32(vgm->file, vgm->header.ym2612_clock);
		write_le32(vgm->file, vgm->header.ym2151_clock);
		write_le32(vgm->file, vgm->header.vgm_data_offset);

		/* Skip to DMG clock offset (0x80) */
		fseek(vgm->file, 0x80, SEEK_SET);
		write_le32(vgm->file, vgm->header.dmg_clock);

		fclose(vgm->file);
	}

	/* Free GD3 strings */
	free(vgm->track_name_en);
	free(vgm->track_name_jp);
	free(vgm->game_name_en);
	free(vgm->game_name_jp);
	free(vgm->system_name_en);
	free(vgm->system_name_jp);
	free(vgm->author_name_en);
	free(vgm->author_name_jp);
	free(vgm->release_date);
	free(vgm->vgm_creator);
	free(vgm->notes);

	free(vgm);
}
