/*
 * Filename Parser - Extract metadata from archive filenames
 *
 * Parses filenames like:
 * "Dragon Warrior III [Dragon Quest III - Soshite Densetsu he...] (2000-12-08)(Chunsoft)(Tose)(Enix)[GBC].7z"
 *
 * Format: Game Name [Alt Name] (Date)(Developer)(Publisher)(Company)[System].ext
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "filename_parser.h"

/* Trim whitespace from both ends */
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

/* Extract content between delimiters */
static int extract_between(const char *str, char start, char end, char *output, size_t output_size) {
	const char *start_pos = strchr(str, start);
	const char *end_pos;
	size_t len;

	if (!start_pos)
		return 0;

	start_pos++; /* Skip opening delimiter */
	end_pos = strchr(start_pos, end);

	if (!end_pos)
		return 0;

	len = end_pos - start_pos;
	if (len >= output_size)
		len = output_size - 1;

	strncpy(output, start_pos, len);
	output[len] = '\0';

	return 1;
}

/* Parse filename and extract metadata */
int parse_filename(const char *filename, struct filename_metadata *metadata) {
	char buffer[1024];
	char *p;
	const char *ext;
	size_t len;

	if (!filename || !metadata)
		return -1;

	/* Initialize metadata */
	memset(metadata, 0, sizeof(struct filename_metadata));

	/* Get basename (remove path) */
	const char *basename = strrchr(filename, '/');
	if (!basename) basename = strrchr(filename, '\\');
	if (basename) basename++;
	else basename = filename;

	/* Remove extension */
	ext = strrchr(basename, '.');
	if (ext) {
		len = ext - basename;
	} else {
		len = strlen(basename);
	}

	if (len >= sizeof(buffer))
		len = sizeof(buffer) - 1;

	strncpy(buffer, basename, len);
	buffer[len] = '\0';

	/* Extract game name (before first '[' or '(') */
	p = buffer;
	while (*p && *p != '[' && *p != '(') {
		p++;
	}

	if (p > buffer) {
		len = p - buffer;
		if (len >= sizeof(metadata->game_name))
			len = sizeof(metadata->game_name) - 1;
		strncpy(metadata->game_name, buffer, len);
		metadata->game_name[len] = '\0';
		trim_whitespace(metadata->game_name);
	}

	/* Extract alternate name [Alt Name] */
	if (extract_between(buffer, '[', ']', metadata->alt_name, sizeof(metadata->alt_name))) {
		trim_whitespace(metadata->alt_name);

		/* Check if it's a system tag like [GBC], [NES], etc. */
		if (strlen(metadata->alt_name) <= 4 && isupper((unsigned char)metadata->alt_name[0])) {
			/* This is likely a system tag, not an alt name */
			strncpy(metadata->system, metadata->alt_name, sizeof(metadata->system) - 1);
			metadata->alt_name[0] = '\0';
		}
	}

	/* Extract date (YYYY-MM-DD) */
	p = buffer;
	while (*p) {
		if (*p == '(' && isdigit((unsigned char)*(p+1))) {
			char date_buf[32];
			if (extract_between(p, '(', ')', date_buf, sizeof(date_buf))) {
				/* Check if it looks like a date */
				if (strlen(date_buf) == 10 && date_buf[4] == '-' && date_buf[7] == '-') {
					strncpy(metadata->release_date, date_buf, sizeof(metadata->release_date) - 1);
					break;
				}
			}
		}
		p++;
	}

	/* Extract companies (multiple parentheses after date) */
	if (metadata->release_date[0]) {
		p = strstr(buffer, metadata->release_date);
		if (p) {
			p = strchr(p, ')');
			if (p) p++;

			/* Extract up to 3 companies */
			int company_count = 0;
			while (*p && company_count < 3) {
				char company[128];
				if (*p == '(') {
					if (extract_between(p, '(', ')', company, sizeof(company))) {
						trim_whitespace(company);

						if (company_count == 0) {
							strncpy(metadata->developer, company, sizeof(metadata->developer) - 1);
						} else if (company_count == 1) {
							strncpy(metadata->publisher, company, sizeof(metadata->publisher) - 1);
						} else if (company_count == 2) {
							strncpy(metadata->company, company, sizeof(metadata->company) - 1);
						}

						company_count++;
						p = strchr(p, ')');
						if (p) p++;
					} else {
						break;
					}
				} else {
					p++;
				}
			}
		}
	}

	/* Extract system tag [GBC], [NES], etc. */
	if (!metadata->system[0]) {
		p = strrchr(buffer, '[');
		if (p) {
			char system_buf[16];
			if (extract_between(p, '[', ']', system_buf, sizeof(system_buf))) {
				trim_whitespace(system_buf);
				if (strlen(system_buf) <= 4) {
					strncpy(metadata->system, system_buf, sizeof(metadata->system) - 1);
				}
			}
		}
	}

	/* Build composer/author from companies */
	if (metadata->developer[0]) {
		strncpy(metadata->composer, metadata->developer, sizeof(metadata->composer) - 1);
	} else if (metadata->publisher[0]) {
		strncpy(metadata->composer, metadata->publisher, sizeof(metadata->composer) - 1);
	} else if (metadata->company[0]) {
		strncpy(metadata->composer, metadata->company, sizeof(metadata->composer) - 1);
	}

	return 0;
}

/* Print metadata for debugging */
void print_metadata(const struct filename_metadata *metadata) {
	printf("Parsed Metadata:\n");
	printf("  Game Name: %s\n", metadata->game_name[0] ? metadata->game_name : "(none)");
	printf("  Alt Name: %s\n", metadata->alt_name[0] ? metadata->alt_name : "(none)");
	printf("  Release Date: %s\n", metadata->release_date[0] ? metadata->release_date : "(none)");
	printf("  Developer: %s\n", metadata->developer[0] ? metadata->developer : "(none)");
	printf("  Publisher: %s\n", metadata->publisher[0] ? metadata->publisher : "(none)");
	printf("  Company: %s\n", metadata->company[0] ? metadata->company : "(none)");
	printf("  System: %s\n", metadata->system[0] ? metadata->system : "(none)");
	printf("  Composer: %s\n", metadata->composer[0] ? metadata->composer : "(none)");
}
