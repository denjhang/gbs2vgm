/*
 * GBS to VGM Batch Converter with 7z/ZIP support
 *
 * Supports:
 * - M3U playlists
 * - 7z archives (auto-extract)
 * - ZIP archives (auto-extract)
 * - Automatic metadata extraction from filename
 * - ZIP packaging of output
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <time.h>
#include "common.h"
#include "libgbs.h"
#include "m3u_parser.h"
#include "vgm_writer.h"
#include "filename_parser.h"
#include "archive_utils.h"

#ifdef _WIN32
#include <direct.h>
#include <windows.h>
#define mkdir(path, mode) _mkdir(path)
#define popen _popen
#define pclose _pclose
#else
#include <unistd.h>
#endif

/* Global variables */
static long rate = 44100;
static long fadeout = 3;
static long silence_timeout = 5;

/* Required by gbhw.c */
int seek_needed = 0;

/* VGM writer */
static vgm_writer_t *vgm = NULL;
static uint32_t samples_since_last_write = 0;

/* Game Boy hardware clock */
#define GB_CLOCK 4194304

/* Sanitize filename */
static void sanitize_filename(char *filename) {
	char *p = filename;
	while (*p) {
		if (*p == '/' || *p == '\\' || *p == ':' || *p == '*' ||
		    *p == '?' || *p == '"' || *p == '<' || *p == '>' || *p == '|' ||
		    (unsigned char)*p < 32 || (unsigned char)*p >= 127) {
			*p = '_';
		}
		p++;
	}
}

/* IO callback */
static void io_callback(struct gbs *gbs, cycles_t cycles, uint32_t addr, uint8_t value, void *priv) {
	(void)gbs;
	(void)priv;
	(void)cycles;

	if (!vgm)
		return;

	if (addr >= 0xFF10 && addr <= 0xFF3F) {
		uint8_t reg = (uint8_t)(addr - 0xFF10);

		if (samples_since_last_write > 0) {
			vgm_write_wait(vgm, samples_since_last_write);
			samples_since_last_write = 0;
		}

		vgm_write_gb_reg(vgm, reg, value);
	}
}

/* Find file with extension in directory */
static int find_file_with_ext(const char *dir, const char *ext, char *output, size_t output_size) {
#ifdef _WIN32
	WIN32_FIND_DATAA find_data;
	HANDLE hFind;
	char search_path[1024];
	char pattern[32];
	char first_match[1024] = {0};
	char gbs_basename[512] = {0};
	int found_gbs_match = 0;

	/* First, find GBS file to get its basename */
	snprintf(pattern, sizeof(pattern), "*.gbs");
	snprintf(search_path, sizeof(search_path), "%s\\%s", dir, pattern);
	hFind = FindFirstFileA(search_path, &find_data);
	if (hFind != INVALID_HANDLE_VALUE) {
		/* Extract basename without extension */
		const char *dot = strrchr(find_data.cFileName, '.');
		if (dot) {
			size_t len = dot - find_data.cFileName;
			if (len < sizeof(gbs_basename)) {
				strncpy(gbs_basename, find_data.cFileName, len);
				gbs_basename[len] = '\0';
			}
		}
		FindClose(hFind);
	}

	/* Now search for M3U files, prioritizing one with same basename as GBS */
	snprintf(pattern, sizeof(pattern), "*.%s", ext);
	snprintf(search_path, sizeof(search_path), "%s\\%s", dir, pattern);

	hFind = FindFirstFileA(search_path, &find_data);
	if (hFind != INVALID_HANDLE_VALUE) {
		do {
			/* Check if this M3U has the same basename as GBS */
			if (gbs_basename[0]) {
				const char *dot = strrchr(find_data.cFileName, '.');
				if (dot) {
					size_t len = dot - find_data.cFileName;
					if (len == strlen(gbs_basename) &&
					    strncmp(find_data.cFileName, gbs_basename, len) == 0) {
						/* Found matching M3U! */
						snprintf(output, output_size, "%s\\%s", dir, find_data.cFileName);
						FindClose(hFind);
						return 0;
					}
				}
			}

			/* Save first match as fallback */
			if (!first_match[0]) {
				snprintf(first_match, sizeof(first_match), "%s\\%s", dir, find_data.cFileName);
			}
		} while (FindNextFileA(hFind, &find_data));

		FindClose(hFind);

		/* Use first match if no GBS-matching M3U found */
		if (first_match[0]) {
			strncpy(output, first_match, output_size - 1);
			output[output_size - 1] = '\0';
			return 0;
		}
	}
#endif
	return -1;
}

/* Convert track */
static int convert_track(const char *gbs_filename, struct m3u_entry *entry, int track_num,
                         const char *game_name, const char *release_date,
                         const char *ripper, const char *notes, const char *output_dir) {
	struct gbs *gbs;
	const struct gbs_metadata *metadata;
	char output_filename[512];
	char track_title[256];
	const char *author_name = "Unknown";
	long total_cycles = 0;
	long target_cycles, loop_cycles;
	long refresh_delay = 33;
	struct gbs_output_buffer buf;
	int has_loop = (entry->loop_count > 1);

	buf.data = NULL;
	buf.bytes = 8192;
	buf.pos = 0;

	/* Create output filename */
	snprintf(track_title, sizeof(track_title), "%02d %s", track_num, entry->title);
	sanitize_filename(track_title);
	snprintf(output_filename, sizeof(output_filename), "%s/%s.vgm", output_dir, track_title);

	printf("Converting: %s (subsong %d) -> %s\n", gbs_filename, entry->subsong, track_title);

	/* Open GBS file */
	gbs = gbs_open(gbs_filename);
	if (!gbs) {
		fprintf(stderr, "Failed to open GBS file: %s\n", gbs_filename);
		return -1;
	}

	/* Get metadata from GBS file */
	metadata = gbs_get_metadata(gbs);
	if (metadata && metadata->author && metadata->author[0]) {
		author_name = metadata->author;
	}

	/* Initialize VGM writer */
	vgm = vgm_writer_init(output_filename, GB_CLOCK);
	if (!vgm) {
		fprintf(stderr, "Failed to create VGM file: %s\n", output_filename);
		gbs_close(gbs);
		return -1;
	}

	/* Set GD3 tag information - use author from GBS file */
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

	/* Set up callbacks */
	gbs_set_io_callback(gbs, io_callback, NULL);

	/* Calculate target duration */
	if (has_loop) {
		loop_cycles = entry->duration_sec * GB_CLOCK;
		target_cycles = loop_cycles * 2;
	} else {
		loop_cycles = 0;
		target_cycles = (entry->duration_sec + fadeout) * GB_CLOCK;
	}

	/* Configure subsong */
	long subsong_timeout = (target_cycles / GB_CLOCK) + 5;
	gbs_configure(gbs, entry->subsong, subsong_timeout, silence_timeout, 0, fadeout);
	gbs_init(gbs, entry->subsong);

	/* Render */
	while (total_cycles < target_cycles) {
		if (has_loop && total_cycles >= loop_cycles && vgm->loop_pos < 0) {
			if (samples_since_last_write > 0) {
				vgm_write_wait(vgm, samples_since_last_write);
				samples_since_last_write = 0;
			}
			vgm_mark_loop_point(vgm);
		}

		if (!gbs_step(gbs, refresh_delay)) {
			break;
		}

		uint32_t step_samples = (refresh_delay * rate) / 1000;
		samples_since_last_write += step_samples;
		total_cycles += (refresh_delay * GB_CLOCK) / 1000;
	}

	/* Write final wait */
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

static void print_usage(const char *progname) {
	fprintf(stderr,
	        "GBS to VGM Batch Converter (7z/ZIP support)\n"
	        "Usage: %s <input> [output_dir]\n"
	        "\n"
	        "Input formats:\n"
	        "  .m3u         M3U playlist file\n"
	        "  .7z          7z archive containing GBS and M3U\n"
	        "  .zip         ZIP archive containing GBS and M3U\n"
	        "\n"
	        "Options:\n"
	        "  output_dir   Output directory (default: auto-generated)\n"
	        "\n"
	        "Examples:\n"
	        "  %s playlist.m3u\n"
	        "  %s \"Dragon Warrior III.7z\"\n"
	        "\n"
	        "Note: Archives will be extracted, converted, and output as ZIP.\n"
	        "\n",
	        progname, progname, progname);
	exit(1);
}

int main(int argc, char **argv) {
	const char *input_file;
	char output_dir[1024];
	char temp_extract_dir[1024];
	char m3u_file[1024];
	char gbs_path[1024];
	char zip_output[1024];
	struct m3u_info *m3u;
	struct filename_metadata file_metadata;
	int is_archive = 0;
	int i;

	if (argc < 2) {
		print_usage(argv[0]);
	}

	input_file = argv[1];

	/* Detect if input is an archive */
	const char *ext = strrchr(input_file, '.');
	if (ext && (strcmp(ext, ".7z") == 0 || strcmp(ext, ".zip") == 0)) {
		is_archive = 1;
	}

	/* Parse filename for metadata */
	if (parse_filename(input_file, &file_metadata) == 0) {
		printf("\n=== Metadata from filename ===\n");
		print_metadata(&file_metadata);
		printf("\n");
	}

	if (is_archive) {
		/* Extract archive */
		snprintf(temp_extract_dir, sizeof(temp_extract_dir), "temp_extract_%d", (int)time(NULL));

		if (archive_extract(input_file, temp_extract_dir) != 0) {
			fprintf(stderr, "Failed to extract archive\n");
			return 1;
		}

		/* Find M3U file */
		if (find_file_with_ext(temp_extract_dir, "m3u", m3u_file, sizeof(m3u_file)) != 0) {
			fprintf(stderr, "No M3U file found in archive\n");
			return 1;
		}

		printf("Found M3U: %s\n", m3u_file);
	} else {
		/* Input is M3U file directly */
		strncpy(m3u_file, input_file, sizeof(m3u_file) - 1);
		m3u_file[sizeof(m3u_file) - 1] = '\0';
	}

	/* Parse M3U file */
	m3u = m3u_parse(m3u_file);
	if (!m3u) {
		fprintf(stderr, "Failed to parse M3U file: %s\n", m3u_file);
		return 1;
	}

	printf("\nM3U Info:\n");
	printf("  Title: %s\n", m3u->title ? m3u->title : "N/A");
	printf("  Composer: %s\n", m3u->composer ? m3u->composer : "N/A");
	printf("  Tracks: %d\n\n", m3u->entry_count);

	/* Determine output directory */
	if (argc > 2) {
		strncpy(output_dir, argv[2], sizeof(output_dir) - 1);
	} else {
		/* Use game name from filename or M3U */
		const char *game_name = file_metadata.game_name[0] ? file_metadata.game_name :
		                        (m3u->title ? m3u->title : "output");
		snprintf(output_dir, sizeof(output_dir), "%s_vgm", game_name);
		sanitize_filename(output_dir);
	}

	/* Create output directory */
	mkdir(output_dir, 0755);
	printf("Output directory: %s\n\n", output_dir);

	/* Get M3U directory for GBS path */
	char m3u_dir[1024];
	strncpy(m3u_dir, m3u_file, sizeof(m3u_dir) - 1);
	char *last_slash = strrchr(m3u_dir, '/');
	if (!last_slash) last_slash = strrchr(m3u_dir, '\\');
	if (last_slash) {
		*(last_slash + 1) = '\0';
	} else {
		m3u_dir[0] = '\0';
	}

	/* Convert each track */
	for (i = 0; i < m3u->entry_count; i++) {
		struct m3u_entry *entry = &m3u->entries[i];

		/* Build full GBS path */
		snprintf(gbs_path, sizeof(gbs_path), "%s%s", m3u_dir, entry->filename);

		/* Use metadata from filename or M3U */
		const char *game_name = file_metadata.game_name[0] ? file_metadata.game_name :
		                        (m3u->title ? m3u->title : "Unknown Game");
		const char *release_date = file_metadata.release_date[0] ? file_metadata.release_date :
		                           (m3u->date ? m3u->date : "");
		const char *ripper = m3u->ripper ? m3u->ripper : "Denjhang";
		const char *notes = "gbs2vgm by Claude & Denjhang";

		/* Author name will be read from GBS file in convert_track */
		if (convert_track(gbs_path, entry, i + 1, game_name, release_date,
		                  ripper, notes, output_dir) < 0) {
			fprintf(stderr, "Failed to convert track %d\n", i + 1);
		}
	}

	m3u_free(m3u);

	/* If input was archive, create ZIP output */
	if (is_archive) {
		snprintf(zip_output, sizeof(zip_output), "%s.zip", output_dir);
		printf("\nCreating output archive...\n");
		archive_create_zip(output_dir, zip_output);

		/* Clean up temp directory */
		printf("Cleaning up temporary files...\n");
		// TODO: Remove temp_extract_dir recursively
	}

	printf("\nConversion complete!\n");
	return 0;
}
