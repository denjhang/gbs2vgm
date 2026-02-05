/*
 * VGM Parser - Analyze and compare VGM files
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#define VGM_IDENT 0x206D6756  /* "Vgm " */

typedef struct {
	uint32_t ident;
	uint32_t eof_offset;
	uint32_t version;
	uint32_t sn76489_clock;
	uint32_t ym2413_clock;
	uint32_t gd3_offset;
	uint32_t total_samples;
	uint32_t loop_offset;
	uint32_t loop_samples;
	uint32_t rate;
	uint16_t sn76489_feedback;
	uint8_t sn76489_shift_width;
	uint8_t sn76489_flags;
	uint32_t ym2612_clock;
	uint32_t ym2151_clock;
	uint32_t vgm_data_offset;
	/* ... more fields ... */
} vgm_header_t;

static uint32_t read_le32(const uint8_t *data) {
	return data[0] | (data[1] << 8) | (data[2] << 16) | (data[3] << 24);
}

static uint16_t read_le16(const uint8_t *data) {
	return data[0] | (data[1] << 8);
}

static void parse_vgm_header(const char *filename) {
	FILE *f;
	uint8_t header[256];
	size_t read_size;
	uint32_t ident, version, gb_clock, data_offset;
	uint32_t total_samples, loop_samples, loop_offset;
	uint32_t gd3_offset;

	f = fopen(filename, "rb");
	if (!f) {
		fprintf(stderr, "Failed to open: %s\n", filename);
		return;
	}

	read_size = fread(header, 1, 256, f);
	if (read_size < 256) {
		fprintf(stderr, "File too small: %s\n", filename);
		fclose(f);
		return;
	}

	/* Parse header */
	ident = read_le32(&header[0x00]);
	if (ident != VGM_IDENT) {
		fprintf(stderr, "Not a VGM file: %s (ident: 0x%08X)\n", filename, ident);
		fclose(f);
		return;
	}

	printf("\n=== VGM File: %s ===\n", filename);

	/* Basic header info */
	version = read_le32(&header[0x08]);
	printf("Version: %d.%02d\n", (version >> 8) & 0xFF, version & 0xFF);

	total_samples = read_le32(&header[0x18]);
	printf("Total samples: %u (%.2f seconds at 44100Hz)\n",
	       total_samples, total_samples / 44100.0);

	loop_offset = read_le32(&header[0x1C]);
	if (loop_offset) {
		loop_offset += 0x1C;  /* Relative to 0x1C */
		printf("Loop offset: 0x%08X\n", loop_offset);
	} else {
		printf("Loop offset: None (non-looping)\n");
	}

	loop_samples = read_le32(&header[0x20]);
	printf("Loop samples: %u\n", loop_samples);

	/* VGM data offset */
	if (version >= 0x150) {
		data_offset = read_le32(&header[0x34]);
		if (data_offset) {
			data_offset += 0x34;  /* Relative to 0x34 */
		} else {
			data_offset = 0x40;  /* Default for v1.50+ */
		}
	} else {
		data_offset = 0x40;  /* Default for older versions */
	}
	printf("VGM data offset: 0x%08X\n", data_offset);

	/* GD3 offset */
	gd3_offset = read_le32(&header[0x14]);
	if (gd3_offset) {
		gd3_offset += 0x14;  /* Relative to 0x14 */
		printf("GD3 offset: 0x%08X\n", gd3_offset);
	}

	/* Game Boy clock - offset 0x80 (VGM 1.61+) */
	if (version >= 0x161) {
		gb_clock = read_le32(&header[0x80]);
		printf("\nGame Boy DMG Clock: 0x%08X (%u Hz)\n", gb_clock, gb_clock);

		if (gb_clock == 0) {
			printf("  WARNING: GB clock is 0 (chip not used or error)\n");
		} else if (gb_clock == 4194304) {
			printf("  OK: Standard GB clock (4.194304 MHz)\n");
		} else {
			printf("  WARNING: Non-standard GB clock!\n");
		}
	} else {
		printf("\nGame Boy DMG Clock: Not supported in version %d.%02d\n",
		       (version >> 8) & 0xFF, version & 0xFF);
	}

	/* Parse first few commands */
	printf("\n=== First Commands ===\n");
	fseek(f, data_offset, SEEK_SET);

	uint8_t cmd_buf[16];
	int cmd_count = 0;
	while (cmd_count < 20 && fread(cmd_buf, 1, 1, f) == 1) {
		uint8_t cmd = cmd_buf[0];

		if (cmd == 0x66) {
			printf("  [%04d] 0x66: End of sound data\n", cmd_count);
			break;
		} else if (cmd == 0x61) {
			fread(&cmd_buf[1], 1, 2, f);
			uint16_t wait = read_le16(&cmd_buf[1]);
			printf("  [%04d] 0x61: Wait %u samples\n", cmd_count, wait);
		} else if (cmd == 0x62) {
			printf("  [%04d] 0x62: Wait 735 samples (1/60s)\n", cmd_count);
		} else if (cmd == 0x63) {
			printf("  [%04d] 0x63: Wait 882 samples (1/50s)\n", cmd_count);
		} else if (cmd >= 0x70 && cmd <= 0x7F) {
			printf("  [%04d] 0x%02X: Wait %u samples\n", cmd_count, cmd, (cmd & 0x0F) + 1);
		} else if (cmd == 0xB3) {
			/* Game Boy register write */
			fread(&cmd_buf[1], 1, 2, f);
			printf("  [%04d] 0xB3: GB write reg 0x%02X = 0x%02X\n",
			       cmd_count, cmd_buf[1], cmd_buf[2]);
		} else {
			/* Unknown/other command - try to skip it */
			printf("  [%04d] 0x%02X: (other command)\n", cmd_count, cmd);

			/* Try to determine command length */
			if (cmd >= 0x30 && cmd <= 0x3F) {
				fread(&cmd_buf[1], 1, 1, f);  /* 1 data byte */
			} else if (cmd >= 0x40 && cmd <= 0x4E) {
				fread(&cmd_buf[1], 1, 2, f);  /* 2 data bytes */
			} else if (cmd >= 0x50 && cmd <= 0x5F) {
				fread(&cmd_buf[1], 1, 1, f);  /* 1 data byte */
			} else if (cmd >= 0xA0 && cmd <= 0xBF) {
				fread(&cmd_buf[1], 1, 2, f);  /* 2 data bytes */
			} else if (cmd >= 0xC0 && cmd <= 0xDF) {
				fread(&cmd_buf[1], 1, 3, f);  /* 3 data bytes */
			} else if (cmd >= 0xE0 && cmd <= 0xFF) {
				fread(&cmd_buf[1], 1, 4, f);  /* 4 data bytes */
			}
		}

		cmd_count++;
	}

	fclose(f);
}

int main(int argc, char **argv) {
	if (argc < 2) {
		fprintf(stderr, "Usage: %s <vgm-file> [vgm-file2 ...]\n", argv[0]);
		return 1;
	}

	for (int i = 1; i < argc; i++) {
		parse_vgm_header(argv[i]);
	}

	return 0;
}
