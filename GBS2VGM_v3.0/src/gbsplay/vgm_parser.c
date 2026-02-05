/*
 * VGM Parser - Analyze and compare VGM files
 *
 * 2026 (C) Licensed under GNU GPL v1 or, at your option, any later version.
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

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
	uint8_t  sn76489_shift_width;
	uint8_t  sn76489_flags;
	uint32_t ym2612_clock;
	uint32_t ym2151_clock;
	uint32_t vgm_data_offset;
	uint32_t dmg_clock;
} VGM_HEADER_INFO;

static uint32_t read_le32(FILE *f) {
	uint32_t val = 0;
	val |= fgetc(f);
	val |= fgetc(f) << 8;
	val |= fgetc(f) << 16;
	val |= fgetc(f) << 24;
	return val;
}

static uint16_t read_le16(FILE *f) {
	uint16_t val = 0;
	val |= fgetc(f);
	val |= fgetc(f) << 8;
	return val;
}

static void parse_vgm_header(FILE *f, VGM_HEADER_INFO *info) {
	fseek(f, 0, SEEK_SET);

	info->ident = read_le32(f);
	info->eof_offset = read_le32(f);
	info->version = read_le32(f);
	info->sn76489_clock = read_le32(f);
	info->ym2413_clock = read_le32(f);
	info->gd3_offset = read_le32(f);
	info->total_samples = read_le32(f);
	info->loop_offset = read_le32(f);
	info->loop_samples = read_le32(f);
	info->rate = read_le32(f);
	info->sn76489_feedback = read_le16(f);
	info->sn76489_shift_width = fgetc(f);
	info->sn76489_flags = fgetc(f);
	info->ym2612_clock = read_le32(f);
	info->ym2151_clock = read_le32(f);
	info->vgm_data_offset = read_le32(f);

	/* Skip to DMG clock at 0x80 */
	fseek(f, 0x80, SEEK_SET);
	info->dmg_clock = read_le32(f);
}

static void print_header_info(const char *filename, VGM_HEADER_INFO *info) {
	printf("\n=== %s ===\n", filename);
	printf("VGM Version: 0x%08X\n", info->version);
	printf("EOF Offset: 0x%08X\n", info->eof_offset);
	printf("GD3 Offset: 0x%08X\n", info->gd3_offset);
	printf("Total Samples: %u\n", info->total_samples);
	printf("Loop Offset: 0x%08X\n", info->loop_offset);
	printf("Loop Samples: %u\n", info->loop_samples);
	printf("Rate: %u\n", info->rate);
	printf("DMG Clock: %u Hz\n", info->dmg_clock);
	printf("VGM Data Offset: 0x%08X (absolute: 0x%08X)\n",
	       info->vgm_data_offset, info->vgm_data_offset + 0x34);
}

static void analyze_commands(FILE *f, VGM_HEADER_INFO *info) {
	long data_start = 0x34 + info->vgm_data_offset;
	int cmd_count = 0;
	int gb_write_count = 0;
	int wait_count = 0;
	int first_gb_cmd = -1;

	fseek(f, data_start, SEEK_SET);

	printf("\nCommand Analysis:\n");
	printf("Data starts at: 0x%08lX\n", data_start);

	/* Analyze all commands */
	while (1) {
		int cmd = fgetc(f);
		if (cmd == EOF) break;

		if (cmd == 0x66) {
			printf("  [%d] 0x66 - End of sound data\n", cmd_count);
			break;
		} else if (cmd == 0xB3) {
			uint8_t reg = fgetc(f);
			uint8_t data = fgetc(f);
			if (first_gb_cmd < 0) first_gb_cmd = cmd_count;
			if (cmd_count < 20) {
				printf("  [%d] 0xB3 - GB write: reg=0x%02X data=0x%02X\n",
				       cmd_count, reg, data);
			}
			gb_write_count++;
		} else if (cmd == 0x61) {
			uint16_t samples = read_le16(f);
			if (cmd_count < 20) {
				printf("  [%d] 0x61 - Wait %u samples\n", cmd_count, samples);
			}
			wait_count++;
		} else if (cmd == 0x62) {
			if (cmd_count < 20) {
				printf("  [%d] 0x62 - Wait 735 samples\n", cmd_count);
			}
			wait_count++;
		} else if (cmd == 0x63) {
			if (cmd_count < 20) {
				printf("  [%d] 0x63 - Wait 882 samples\n", cmd_count);
			}
			wait_count++;
		} else if (cmd >= 0x70 && cmd <= 0x7F) {
			if (cmd_count < 20) {
				printf("  [%d] 0x%02X - Wait %d samples\n", cmd_count, cmd, (cmd - 0x70) + 1);
			}
			wait_count++;
		} else {
			if (cmd_count < 20) {
				printf("  [%d] 0x%02X - Unknown command\n", cmd_count, cmd);
			}
		}

		cmd_count++;
	}

	printf("\nSummary:\n");
	printf("  Total commands analyzed: %d\n", cmd_count);
	printf("  GB write commands: %d\n", gb_write_count);
	printf("  Wait commands: %d\n", wait_count);
	printf("  First GB command at: %d\n", first_gb_cmd);
}

int main(int argc, char **argv) {
	if (argc < 2) {
		fprintf(stderr, "Usage: %s <vgm-file1> [vgm-file2]\n", argv[0]);
		return 1;
	}

	for (int i = 1; i < argc; i++) {
		FILE *f = fopen(argv[i], "rb");
		if (!f) {
			fprintf(stderr, "Failed to open: %s\n", argv[i]);
			continue;
		}

		VGM_HEADER_INFO info;
		parse_vgm_header(f, &info);
		print_header_info(argv[i], &info);
		analyze_commands(f, &info);

		fclose(f);
	}

	return 0;
}
