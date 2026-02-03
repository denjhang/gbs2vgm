/*
 * gbsplay is a Gameboy sound player
 *
 * M3U playlist parser
 *
 * 2026 (C) Licensed under GNU GPL v1 or, at your option, any later version.
 */

#ifndef _M3U_PARSER_H_
#define _M3U_PARSER_H_

#include <stdint.h>

struct m3u_entry {
	char *filename;      /* GBS filename */
	int subsong;         /* Subsong number */
	char *title;         /* Track title */
	int duration_sec;    /* Duration in seconds */
	int duration_ms;     /* Duration milliseconds part */
	int loop_count;      /* Loop count */
};

struct m3u_info {
	char *title;         /* Album/Game title */
	char *artist;        /* Artist */
	char *composer;      /* Composer */
	char *date;          /* Release date */
	char *ripper;        /* Ripper */
	char *tagger;        /* Tagger */

	struct m3u_entry *entries;
	int entry_count;
};

/* Parse M3U file and return m3u_info structure */
struct m3u_info *m3u_parse(const char *filename);

/* Free m3u_info structure */
void m3u_free(struct m3u_info *info);

#endif
