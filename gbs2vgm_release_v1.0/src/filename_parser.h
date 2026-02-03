/*
 * Filename Parser - Extract metadata from archive filenames
 */

#ifndef _FILENAME_PARSER_H_
#define _FILENAME_PARSER_H_

#include <stddef.h>

/* Metadata extracted from filename */
struct filename_metadata {
	char game_name[256];      /* Main game name */
	char alt_name[256];       /* Alternate name (Japanese, etc.) */
	char release_date[32];    /* Release date (YYYY-MM-DD) */
	char developer[128];      /* Developer company */
	char publisher[128];      /* Publisher company */
	char company[128];        /* Additional company */
	char system[16];          /* System tag (GBC, NES, etc.) */
	char composer[256];       /* Composer (derived from companies) */
};

/* Parse filename and extract metadata
 * Returns: 0 on success, -1 on error
 */
int parse_filename(const char *filename, struct filename_metadata *metadata);

/* Print metadata for debugging */
void print_metadata(const struct filename_metadata *metadata);

#endif /* _FILENAME_PARSER_H_ */
