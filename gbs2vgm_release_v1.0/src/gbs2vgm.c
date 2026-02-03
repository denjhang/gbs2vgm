/*
 * gbs2vgm - Convert GBS files to VGM using M3U track information
 *
 * 2026 (C) Licensed under GNU GPL v1 or, at your option, any later version.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>

#include "common.h"
#include "libgbs.h"
#include "m3u_parser.h"
#include "vgm_writer.h"

/* Global variables */
static long rate = 44100;
static long fadeout = 3;
static long silence_timeout = 5;
static char *output_dir = NULL;

/* Required by gbhw.c */
int seek_needed = 0;

/* VGM writer */
static vgm_writer_t *vgm = NULL;
static uint32_t samples_since_last_write = 0;

/* Game Boy hardware clock */
#define GB_CLOCK 4194304

/* Sanitize filename - replace invalid characters */
static void sanitize_filename(char *filename) {
	char *p = filename;
	while (*p) {
		/* Replace invalid filename characters and non-ASCII characters */
		if (*p == '/' || *p == '\\' || *p == ':' || *p == '*' ||
		    *p == '?' || *p == '"' || *p == '<' || *p == '>' || *p == '|' ||
		    (unsigned char)*p < 32 || (unsigned char)*p >= 127) {
			*p = '_';
		}
		p++;
	}
}

/* IO callback - captures Game Boy register writes */
static void io_callback(struct gbs *gbs, cycles_t cycles, uint32_t addr, uint8_t value, void *priv) {
	(void)gbs;
	(void)priv;
	(void)cycles;

	if (!vgm)
		return;

	/* Convert address to Game Boy register offset */
	/* Game Boy sound registers are at 0xFF10-0xFF3F */
	if (addr >= 0xFF10 && addr <= 0xFF3F) {
		uint8_t reg = (uint8_t)(addr - 0xFF10);

		/* Write any pending wait */
		if (samples_since_last_write > 0) {
			vgm_write_wait(vgm, samples_since_last_write);
			samples_since_last_write = 0;
		}

		/* Write register command */
		vgm_write_gb_reg(vgm, reg, value);
	}
}

static void usage(const char *progname) {
	fprintf(stderr,
	        "Usage: %s [options] <m3u-file>\n"
	        "\n"
	        "Options:\n"
	        "  -r <rate>     Sample rate (default: 44100)\n"
	        "  -f <seconds>  Fadeout duration (default: 3)\n"
	        "  -o <dir>      Output directory (default: current directory)\n"
	        "  -h            Show this help\n"
	        "\n",
	        progname);
	exit(1);
}

static void parse_args(int argc, char **argv) {
	int opt;

	while ((opt = getopt(argc, argv, "r:f:o:h")) != -1) {
		switch (opt) {
		case 'r':
			rate = atol(optarg);
			break;
		case 'f':
			fadeout = atol(optarg);
			break;
		case 'o':
			output_dir = optarg;
			break;
		case 'h':
		default:
			usage(argv[0]);
		}
	}
}

static int convert_track(const char *gbs_filename, struct m3u_entry *entry, int track_num,
                         const char *game_name, const char *author_name, const char *release_date,
                         const char *ripper, const char *notes) {
	struct gbs *gbs;
	char output_filename[512];
	char track_title[256];
	long total_cycles = 0;
	long target_cycles, loop_cycles;
	long refresh_delay = 33; /* milliseconds */
	struct gbs_output_buffer buf;
	int has_loop = (entry->loop_count > 1);

	/* Initialize output buffer */
	buf.data = NULL;
	buf.bytes = 8192;
	buf.pos = 0;

	/* Create output filename */
	snprintf(track_title, sizeof(track_title), "%02d %s", track_num, entry->title);
	sanitize_filename(track_title);

	if (output_dir) {
		snprintf(output_filename, sizeof(output_filename), "%s/%s.vgm", output_dir, track_title);
	} else {
		snprintf(output_filename, sizeof(output_filename), "%s.vgm", track_title);
	}

	printf("Converting: %s (subsong %d) -> %s\n", gbs_filename, entry->subsong, output_filename);

	/* Open GBS file */
	gbs = gbs_open(gbs_filename);
	if (!gbs) {
		fprintf(stderr, "Failed to open GBS file: %s\n", gbs_filename);
		return -1;
	}

	/* Initialize VGM writer */
	vgm = vgm_writer_init(output_filename, GB_CLOCK);
	if (!vgm) {
		fprintf(stderr, "Failed to create VGM file: %s\n", output_filename);
		gbs_close(gbs);
		return -1;
	}

	/* Set GD3 tag information */
	vgm_set_gd3_info(vgm, entry->title, game_name, author_name, release_date, ripper, notes);

	samples_since_last_write = 0;

	/* Configure output buffer */
	buf.data = malloc(buf.bytes);
	if (!buf.data) {
		fprintf(stderr, "Failed to allocate output buffer\n");
		vgm_writer_close(vgm);
		vgm = NULL;
		gbs_close(gbs);
		return -1;
	}
	gbs_configure_output(gbs, &buf, rate);

	/* Set up callbacks to capture IO writes */
	gbs_set_io_callback(gbs, io_callback, NULL);

	/* Calculate target duration in cycles */
	/* For looping tracks, play intro + one loop */
	if (has_loop) {
		loop_cycles = entry->duration_sec * GB_CLOCK;
		target_cycles = loop_cycles * 2; /* Intro + one loop */
	} else {
		loop_cycles = 0;
		target_cycles = (entry->duration_sec + fadeout) * GB_CLOCK;
	}

	/* Configure subsong */
	long subsong_timeout = (target_cycles / GB_CLOCK) + 5;
	gbs_configure(gbs, entry->subsong, subsong_timeout, silence_timeout, 0, fadeout);
	gbs_init(gbs, entry->subsong);

	/* Render - step through emulation */
	while (total_cycles < target_cycles) {
		/* Mark loop point after first iteration for looping tracks */
		if (has_loop && total_cycles >= loop_cycles && vgm->loop_pos < 0) {
			if (samples_since_last_write > 0) {
				vgm_write_wait(vgm, samples_since_last_write);
				samples_since_last_write = 0;
			}
			vgm_mark_loop_point(vgm);
		}

		if (!gbs_step(gbs, refresh_delay)) {
			/* Playback ended */
			break;
		}

		/* Calculate samples for this step */
		/* refresh_delay is in milliseconds, convert to samples */
		uint32_t step_samples = (refresh_delay * rate) / 1000;
		samples_since_last_write += step_samples;

		total_cycles += (refresh_delay * GB_CLOCK) / 1000;
	}

	/* Write final wait if needed */
	if (samples_since_last_write > 0) {
		vgm_write_wait(vgm, samples_since_last_write);
	}

	/* Close files */
	free(buf.data);
	vgm_writer_close(vgm);
	vgm = NULL;
	gbs_close(gbs);

	printf("  Done: %ld cycles processed\n", total_cycles);
	return 0;
}

int main(int argc, char **argv) {
	struct m3u_info *m3u;
	char gbs_path[512];
	char *m3u_dir;
	char *m3u_filename;
	int i;

	parse_args(argc, argv);

	if (optind >= argc) {
		usage(argv[0]);
	}

	m3u_filename = argv[optind];

	/* Parse M3U file */
	m3u = m3u_parse(m3u_filename);
	if (!m3u) {
		fprintf(stderr, "Failed to parse M3U file: %s\n", m3u_filename);
		return 1;
	}

	printf("M3U Info:\n");
	printf("  Title: %s\n", m3u->title ? m3u->title : "N/A");
	printf("  Artist: %s\n", m3u->artist ? m3u->artist : "N/A");
	printf("  Composer: %s\n", m3u->composer ? m3u->composer : "N/A");
	printf("  Tracks: %d\n\n", m3u->entry_count);

	/* Get M3U directory for resolving GBS path */
	m3u_dir = strdup(m3u_filename);
	char *last_slash = strrchr(m3u_dir, '/');
	if (!last_slash)
		last_slash = strrchr(m3u_dir, '\\');
	if (last_slash) {
		*last_slash = '\0';
	} else {
		strcpy(m3u_dir, ".");
	}

	/* Create output directory if specified */
	if (output_dir) {
#ifdef _WIN32
		mkdir(output_dir);
#else
		mkdir(output_dir, 0755);
#endif
	}

	/* Convert each track */
	for (i = 0; i < m3u->entry_count; i++) {
		struct m3u_entry *entry = &m3u->entries[i];

		/* Build full GBS path */
		snprintf(gbs_path, sizeof(gbs_path), "%s/%s", m3u_dir, entry->filename);

		/* Pass metadata to converter */
		const char *game_name = m3u->title ? m3u->title : "Unknown Game";
		const char *author_name = m3u->composer ? m3u->composer :
		                          (m3u->artist ? m3u->artist : "Unknown");
		const char *release_date = m3u->date ? m3u->date : "";
		const char *ripper = m3u->ripper ? m3u->ripper : "Denjhang";
		const char *notes = "gbs2vgm by Claude & Denjhang";

		if (convert_track(gbs_path, entry, i + 1, game_name, author_name, release_date, ripper, notes) < 0) {
			fprintf(stderr, "Failed to convert track %d\n", i + 1);
		}
	}

	free(m3u_dir);
	m3u_free(m3u);

	printf("\nConversion complete!\n");
	return 0;
}
