/*
 * gbs2wav - Convert GBS files to WAV using M3U track information
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
#include "plugout.h"
#include "m3u_parser.h"
#include "util.h"

/* Global variables */
static long rate = 44100;
static long fadeout = 3;
static long silence_timeout = 5;
static char *filter_type = "dmg";
static char *output_dir = NULL;

/* Required by gbhw.c */
int seek_needed = 0;

/* Output buffer */
static struct gbs_output_buffer buf = {
	.data = NULL,
	.bytes = 8192,
	.pos = 0,
};

/* WAV file writing */
static FILE *wav_file = NULL;
static long sample_rate = 44100;

static const uint8_t blank_hdr[44];

static int wav_write_header() {
	const uint32_t fmt_subchunk_length = 16;
	const uint16_t audio_format_uncompressed_pcm = 1;
	const uint16_t num_channels = 2;
	const uint16_t bits_per_sample = 16;
	const uint32_t byte_rate = sample_rate * num_channels * bits_per_sample / 8;
	const uint16_t block_align = num_channels * bits_per_sample / 8;

	long filesize = ftell(wav_file);
	if (filesize < 0 || filesize > 0xffffffff)
		return -1;

	fpackat(wav_file, 0, "<{RIFF}d{WAVE}<{fmt }dwwddww{data}d",
	        (uint32_t)filesize - 8,
	        fmt_subchunk_length,
	        audio_format_uncompressed_pcm,
	        num_channels,
	        sample_rate,
	        byte_rate,
	        block_align,
	        bits_per_sample,
	        (uint32_t)filesize - 44);
	return 0;
}

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

static int open_wav_file(const char *filename) {
	wav_file = fopen(filename, "wb");
	if (!wav_file) {
		fprintf(stderr, "Failed to create WAV file: %s\n", filename);
		return -1;
	}

	/* Write blank header (will be updated later) */
	fwrite(blank_hdr, sizeof(blank_hdr), 1, wav_file);
	return 0;
}

static int close_wav_file() {
	if (!wav_file)
		return -1;

	if (wav_write_header()) {
		fclose(wav_file);
		wav_file = NULL;
		return -1;
	}

	fclose(wav_file);
	wav_file = NULL;
	return 0;
}

static void audio_callback(struct gbs *gbs, struct gbs_output_buffer *buf, void *priv) {
	(void)gbs;
	(void)priv;

	if (wav_file) {
		fwrite(buf->data, buf->pos * 2 * sizeof(int16_t), 1, wav_file);
	}
	buf->pos = 0;
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
			sample_rate = rate;
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

static enum gbs_filter_type parse_filter(const char *filter_name) {
	if (strcasecmp(filter_name, "off") == 0)
		return FILTER_OFF;
	if (strcasecmp(filter_name, "dmg") == 0)
		return FILTER_DMG;
	if (strcasecmp(filter_name, "cgb") == 0)
		return FILTER_CGB;
	return FILTER_DMG;
}

static int convert_track(const char *gbs_filename, struct m3u_entry *entry, int track_num) {
	struct gbs *gbs;
	char output_filename[512];
	char track_title[256];
	long total_samples = 0;
	long target_samples;

	/* Create output filename */
	snprintf(track_title, sizeof(track_title), "%02d %s", track_num, entry->title);
	sanitize_filename(track_title);

	if (output_dir) {
		snprintf(output_filename, sizeof(output_filename), "%s/%s.wav", output_dir, track_title);
	} else {
		snprintf(output_filename, sizeof(output_filename), "%s.wav", track_title);
	}

	printf("Converting: %s (subsong %d) -> %s\n", gbs_filename, entry->subsong, output_filename);

	/* Open GBS file */
	gbs = gbs_open(gbs_filename);
	if (!gbs) {
		fprintf(stderr, "Failed to open GBS file: %s\n", gbs_filename);
		return -1;
	}

	/* Configure GBS */
	buf.data = malloc(buf.bytes);
	gbs_set_sound_callback(gbs, audio_callback, NULL);
	gbs_configure_output(gbs, &buf, rate);
	gbs_set_filter(gbs, parse_filter(filter_type));

	/* Calculate target duration */
	target_samples = (entry->duration_sec * rate) + ((entry->duration_ms * rate) / 1000);
	target_samples += fadeout * rate; /* Add fadeout time */

	/* Configure subsong */
	long subsong_timeout = entry->duration_sec + fadeout + 5;
	gbs_configure(gbs, entry->subsong, subsong_timeout, silence_timeout, 0, fadeout);
	gbs_init(gbs, entry->subsong);

	/* Open output WAV file */
	if (open_wav_file(output_filename) < 0) {
		free(buf.data);
		gbs_close(gbs);
		return -1;
	}

	/* Render audio */
	while (total_samples < target_samples) {
		if (!gbs_step(gbs, 33)) {
			/* Playback ended */
			break;
		}
		total_samples += buf.bytes / (2 * sizeof(int16_t));
	}

	/* Close files */
	close_wav_file();
	free(buf.data);
	gbs_close(gbs);

	printf("  Done: %ld samples written\n", total_samples);
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

		if (convert_track(gbs_path, entry, i + 1) < 0) {
			fprintf(stderr, "Failed to convert track %d\n", i + 1);
		}
	}

	free(m3u_dir);
	m3u_free(m3u);

	printf("\nConversion complete!\n");
	return 0;
}
