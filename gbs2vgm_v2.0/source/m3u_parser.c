/*
 * gbsplay is a Gameboy sound player
 *
 * M3U playlist parser
 *
 * 2026 (C) Licensed under GNU GPL v1 or, at your option, any later version.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
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

static void parse_entry_line(struct m3u_info *info, const char *line) {
	char filename[256];
	char title[512];
	char time_str[32];
	int subsong;
	int loop_count = 1;
	char *last_comma, *second_last_comma, *third_last_comma;
	char line_copy[1024];

	/* Format: filename::GBS,subsong,title,time,,loop_count */
	/* Example: CGB-BD3J-JPN.gbs::GBS,0,Overture ~ Roto's Theme,1:39,,10 */

	strncpy(line_copy, line, sizeof(line_copy) - 1);
	line_copy[sizeof(line_copy) - 1] = '\0';

	/* Find the last comma (before loop_count) */
	last_comma = strrchr(line_copy, ',');
	if (!last_comma) return;

	/* Parse loop count */
	if (sscanf(last_comma + 1, "%d", &loop_count) != 1) {
		loop_count = 1;
	}
	*last_comma = '\0';

	/* Find second last comma (empty field) */
	second_last_comma = strrchr(line_copy, ',');
	if (!second_last_comma) return;
	*second_last_comma = '\0';

	/* Find third last comma (before time) */
	third_last_comma = strrchr(line_copy, ',');
	if (!third_last_comma) return;

	/* Parse time */
	strncpy(time_str, third_last_comma + 1, sizeof(time_str) - 1);
	time_str[sizeof(time_str) - 1] = '\0';
	*third_last_comma = '\0';

	/* Now parse the beginning part (filename, subsong, title) */
	if (sscanf(line_copy, "%255[^:]::GBS,%d,%511[^\n]",
	           filename, &subsong, title) >= 3) {

		/* Remove escape characters from title */
		char *src = title, *dst = title;
		while (*src) {
			if (*src == '\\' && *(src + 1)) {
				src++; /* Skip backslash */
			}
			*dst++ = *src++;
		}
		*dst = '\0';

		/* Reallocate entries array */
		info->entries = realloc(info->entries,
		                        sizeof(struct m3u_entry) * (info->entry_count + 1));

		struct m3u_entry *entry = &info->entries[info->entry_count];
		memset(entry, 0, sizeof(struct m3u_entry));

		entry->filename = strdup_safe(filename);
		entry->subsong = subsong;
		entry->title = strdup_safe(title);
		entry->loop_count = loop_count;

		parse_time(time_str, &entry->duration_sec, &entry->duration_ms);

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
