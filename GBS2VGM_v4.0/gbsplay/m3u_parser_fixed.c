/*
 * gbsplay is a Gameboy sound player
 *
 * M3U playlist parser - FIXED VERSION
 *
 * 2026 (C) Licensed under GNU GPL v1 or, at your option, any later version.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdbool.h>
#include "m3u_parser.h"

#define MAX_LINE_LENGTH 1024

static char *trim_whitespace(char *str) {
	char *end;

	/* Trim leading space */
	while(isspace((unsigned char)*str)) str++;

	if(*str == 0)
		return str;

	/* Trim trailing space */
	end = str + strlen(str) - 1;
	while(end > str && isspace((unsigned char)*end)) end--;

	end[1] = '\0';
	return str;
}

static char *strdup_safe(const char *str) {
	if (!str) return NULL;
	return strdup(str);
}

static void parse_metadata_line(struct m3u_info *info, const char *line) {
	if (strncmp(line, "# @TITLE", 8) == 0) {
		const char *value = line + 8;
		while (*value && isspace((unsigned char)*value)) value++;
		if (*value) {
			info->title = strdup_safe(value);
		}
	} else if (strncmp(line, "# @ARTIST", 9) == 0) {
		const char *value = line + 9;
		while (*value && isspace((unsigned char)*value)) value++;
		if (*value) {
			info->artist = strdup_safe(value);
		}
	} else if (strncmp(line, "# @COMPOSER", 11) == 0) {
		const char *value = line + 11;
		while (*value && isspace((unsigned char)*value)) value++;
		if (*value) {
			info->composer = strdup_safe(value);
		}
	} else if (strncmp(line, "# @DATE", 7) == 0) {
		const char *value = line + 7;
		while (*value && isspace((unsigned char)*value)) value++;
		if (*value) {
			info->date = strdup_safe(value);
		}
	} else if (strncmp(line, "# @RIPPER", 9) == 0) {
		const char *value = line + 9;
		while (*value && isspace((unsigned char)*value)) value++;
		if (*value) {
			info->ripper = strdup_safe(value);
		}
	} else if (strncmp(line, "# @TAGGER", 9) == 0) {
		const char *value = line + 9;
		while (*value && isspace((unsigned char)*value)) value++;
		if (*value) {
			info->tagger = strdup_safe(value);
		}
	}
}

static int parse_time(const char *time_str, int *sec, int *ms) {
	int min = 0;
	*sec = 0;
	*ms = 0;

	/* Format: M:SS or MM:SS or M:SS.mmm */
	if (sscanf(time_str, "%d:%d.%d", &min, sec, ms) == 3) {
		*sec += min * 60;
		return 1;
	} else if (sscanf(time_str, "%d:%d", &min, sec) == 2) {
		*sec += min * 60;
		return 1;
	}
	return 0;
}

/* Extract substring and remove escape characters */
static void extract_and_unescape(char *dest, const char *src, int len, int dest_size) {
	const char *src_end = src + len;
	char *dst = dest;
	int remaining = dest_size - 1;

	while (src < src_end && remaining > 0) {
		if (*src == '\\' && (src + 1) < src_end) {
			src++; /* Skip backslash */
		}
		*dst++ = *src++;
		remaining--;
	}
	*dst = '\0';
}

static void parse_entry_line(struct m3u_info *info, const char *line) {
	char filename[256];
	char title[512];
	char time_str[32] = "";
	char intro_loop_str[64] = "";
	char fade_str[32] = "";
	int subsong;
	int loop_count = 1;
	char line_copy[1024];

	/* Format: filename::GBS,subsong,title,time,intro-loop,fade,loop_count */
	/* Example: CGB-BD3J-JPN.gbs::GBS,0,Overture ~ Roto's Theme,1:39,0:10-,3,10 */
	/* Or old format: filename::GBS,subsong,title,time,,loop_count */

	strncpy(line_copy, line, sizeof(line_copy) - 1);
	line_copy[sizeof(line_copy) - 1] = '\0';

	/* Find all unescaped commas */
	char *comma_positions[20];
	int comma_count = 0;
	char *p = line_copy;
	while (*p && comma_count < 20) {
		if (*p == '\\' && *(p + 1)) {
			p += 2; /* Skip escaped character */
			continue;
		}
		if (*p == ',') {
			comma_positions[comma_count++] = p;
		}
		p++;
	}

	if (comma_count < 5) return; /* Not enough commas */

	/* Old format: filename::GBS,subsong,title,time,,loop_count (5 commas)
	 * New format: filename::GBS,subsong,title,time,intro-loop,fade,loop_count (6 commas)
	 */
	bool old_format = (comma_count == 5);

	/* Parse from the end */
	char *last_comma = comma_positions[comma_count - 1];
	char *second_last_comma = comma_positions[comma_count - 2];
	char *third_last_comma = comma_positions[comma_count - 3];
	char *fourth_last_comma = (comma_count >= 4) ? comma_positions[comma_count - 4] : NULL;
	char *fifth_last_comma = (comma_count >= 5) ? comma_positions[comma_count - 5] : NULL;

	/* Parse loop count */
	if (sscanf(last_comma + 1, "%d", &loop_count) != 1) {
		loop_count = 1;
	}

	if (old_format) {
		/* Old format: ...,title,time,,loop_count */
		/* second_last_comma points to empty field before loop_count */
		/* third_last_comma points to time field */
		int time_len = second_last_comma - third_last_comma - 1;
		if (time_len > 0 && time_len < sizeof(time_str)) {
			strncpy(time_str, third_last_comma + 1, time_len);
			time_str[time_len] = '\0';
		}

		/* fourth_last_comma points to title field */
		if (fourth_last_comma) {
			int title_len = third_last_comma - fourth_last_comma - 1;
			if (title_len > 0 && title_len < sizeof(title)) {
				extract_and_unescape(title, fourth_last_comma + 1, title_len, sizeof(title));
			}
		}
	} else {
		/* New format: ...,title,time,intro-loop,fade,loop_count */
		/* Parse fade */
		int fade_len = last_comma - second_last_comma - 1;
		if (fade_len > 0 && fade_len < sizeof(fade_str)) {
			strncpy(fade_str, second_last_comma + 1, fade_len);
			fade_str[fade_len] = '\0';
		}

		/* Parse intro-loop */
		int intro_loop_len = second_last_comma - third_last_comma - 1;
		if (intro_loop_len > 0 && intro_loop_len < sizeof(intro_loop_str)) {
			strncpy(intro_loop_str, third_last_comma + 1, intro_loop_len);
			intro_loop_str[intro_loop_len] = '\0';
		}

		/* Parse time */
		if (fourth_last_comma) {
			int time_len = third_last_comma - fourth_last_comma - 1;
			if (time_len > 0 && time_len < sizeof(time_str)) {
				strncpy(time_str, fourth_last_comma + 1, time_len);
				time_str[time_len] = '\0';
			}
		}

		/* Parse title */
		if (fifth_last_comma) {
			int title_len = fourth_last_comma - fifth_last_comma - 1;
			if (title_len > 0 && title_len < sizeof(title)) {
				extract_and_unescape(title, fifth_last_comma + 1, title_len, sizeof(title));
			}
		}
	}

	/* Now parse the beginning part (filename, subsong) */
	/* Find the first comma (after subsong) */
	char *first_comma = (comma_count > 0) ? comma_positions[0] : NULL;
	if (!first_comma) return;

	/* Parse filename and subsong */
	if (sscanf(line_copy, "%255[^:]::GBS,%d", filename, &subsong) >= 2) {

		/* Reallocate entries array */
		info->entries = realloc(info->entries,
		                        sizeof(struct m3u_entry) * (info->entry_count + 1));

		struct m3u_entry *entry = &info->entries[info->entry_count];
		memset(entry, 0, sizeof(struct m3u_entry));

		entry->filename = strdup_safe(filename);
		entry->subsong = subsong;
		entry->title = strdup_safe(title);
		entry->loop_count = loop_count;

		/* Parse duration */
		parse_time(time_str, &entry->duration_sec, &entry->duration_ms);

		/* Initialize intro/loop to -1 (not specified) */
		entry->intro_sec = -1;
		entry->intro_ms = 0;
		entry->loop_sec = -1;
		entry->loop_ms = 0;
		entry->fade_sec = -1;

		/* Parse intro-loop field (GME format) */
		char *trimmed_intro_loop = trim_whitespace(intro_loop_str);
		if (strlen(trimmed_intro_loop) > 0 && strcmp(trimmed_intro_loop, "") != 0) {
			if (strcmp(trimmed_intro_loop, "-") == 0) {
				/* "-" means entire song is loop (intro=0, loop=duration) */
				entry->intro_sec = 0;
				entry->intro_ms = 0;
				entry->loop_sec = entry->duration_sec;
				entry->loop_ms = entry->duration_ms;
			} else if (strchr(trimmed_intro_loop, '-')) {
				/* Format: "intro-" or "intro-loop" */
				char intro_str[32], loop_str[32];
				if (sscanf(trimmed_intro_loop, "%31[^-]-%31s", intro_str, loop_str) >= 1) {
					/* Parse intro */
					parse_time(intro_str, &entry->intro_sec, &entry->intro_ms);

					/* Check if loop is specified */
					if (strlen(loop_str) > 0) {
						/* "intro-loop" format */
						parse_time(loop_str, &entry->loop_sec, &entry->loop_ms);
					} else {
						/* "intro-" format: loop = duration - intro */
						int intro_total_ms = entry->intro_sec * 1000 + entry->intro_ms;
						int duration_total_ms = entry->duration_sec * 1000 + entry->duration_ms;
						int loop_total_ms = duration_total_ms - intro_total_ms;
						entry->loop_sec = loop_total_ms / 1000;
						entry->loop_ms = loop_total_ms % 1000;
					}
				}
			} else {
				/* Just a time value: treat as loop length */
				parse_time(trimmed_intro_loop, &entry->loop_sec, &entry->loop_ms);
				entry->intro_sec = 0;
				entry->intro_ms = 0;
			}
		}

		/* Parse fade */
		char *trimmed_fade = trim_whitespace(fade_str);
		if (strlen(trimmed_fade) > 0) {
			int fade_ms;
			if (parse_time(trimmed_fade, &entry->fade_sec, &fade_ms)) {
				/* fade_ms is ignored for now, we only use seconds */
			}
		}

		info->entry_count++;
	}
}

struct m3u_info *m3u_parse(const char *filename) {
	FILE *fp;
	char line[MAX_LINE_LENGTH];
	struct m3u_info *info;

	fp = fopen(filename, "r");
	if (!fp) {
		fprintf(stderr, "Failed to open M3U file: %s\n", filename);
		return NULL;
	}

	info = calloc(1, sizeof(struct m3u_info));
	if (!info) {
		fclose(fp);
		return NULL;
	}

	while (fgets(line, sizeof(line), fp)) {
		char *trimmed = trim_whitespace(line);

		/* Skip empty lines */
		if (strlen(trimmed) == 0)
			continue;

		/* Parse metadata comments */
		if (trimmed[0] == '#') {
			parse_metadata_line(info, trimmed);
		}
		/* Parse entry lines */
		else {
			parse_entry_line(info, trimmed);
		}
	}

	fclose(fp);
	return info;
}

void m3u_free(struct m3u_info *info) {
	if (!info)
		return;

	free(info->title);
	free(info->artist);
	free(info->composer);
	free(info->date);
	free(info->ripper);
	free(info->tagger);

	for (int i = 0; i < info->entry_count; i++) {
		free(info->entries[i].filename);
		free(info->entries[i].title);
	}

	free(info->entries);
	free(info);
}
