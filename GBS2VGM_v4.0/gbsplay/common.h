/*
 * gbsplay is a Gameboy sound player
 *
 * 2003-2021 (C) by Tobias Diedrich <ranma+gbsplay@tdiedrich.de>
 *
 * Licensed under GNU GPL v1 or, at your option, any later version.
 */

#ifndef _COMMON_H_
#define _COMMON_H_

#include <inttypes.h>
#include <stddef.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <math.h>

#include "config.h"

typedef uint64_t cycles_t;

/* mingw64 also defines __MINGW32__ in addition to __MINGW64__ */
#ifdef __MINGW32__
#define HAVE_MINGW 1
#endif

#ifndef true
#define true (0==0)
#define false (!true)
#endif

#define UNUSED(x) (void)(x)

#define WARN_N(n, ...) { \
	static long ctr = n; \
	if (ctr) { \
		ctr--; \
		fprintf(stderr, __VA_ARGS__); \
	} \
}
#define WARN_ONCE(...) WARN_N(1, __VA_ARGS__)

#define CONTAINER_OF(ptr, type, member) \
	((type *)((void *)(ptr) - offsetof(type, member)))

#define ARRAY_SIZE(x) (sizeof(x) / sizeof(*(x)))

#define TEXTDOMAIN "gbsplay"
#define N_(x) x

#define C0HZ 16.35
#define C0MIDI 12
#define C6GBRAW 1923 /* First note in gameboy startup sound, 1048.576Hz (3.4 cents too high) */
#define C6GB (2048 - C6GBRAW)
#define C6MIDI 84 /* 1046.50Hz in equal temperament */
/* wave channel produces half the frequency */
#define FREQ(x,ch) (131072. / ((x)<<((ch)>1)))
#define FREQTONOTE(f) (lround(log2((f)/C0HZ)*12)+C0MIDI)
#define NOTE(x,ch) FREQTONOTE(FREQ(x,ch))


#if USE_I18N == 1

#  include <locale.h>
#  include <libintl.h>

#  if GBS_PRESERVE_TEXTDOMAIN == 1

static inline char* _(const char* const msgid)
{
	char *olddomain = textdomain(NULL);
	char *olddir = bindtextdomain(olddomain, NULL);
	char *res;

	bindtextdomain(TEXTDOMAIN, LOCALE_PREFIX);
	res = dgettext(TEXTDOMAIN, msgid);
	bindtextdomain(olddomain, olddir);

	return res;
}

#  else

static inline char* _(const char* const msgid)
{
	return gettext(msgid);
}

static inline void i18n_init(void)
{
	if (setlocale(LC_ALL, "") == NULL) {
		fprintf(stderr, "setlocale() failed\n");
	}
	if (bindtextdomain(TEXTDOMAIN, LOCALE_PREFIX) == NULL) {
		fprintf(stderr, "bindtextdomain() failed: %s\n", strerror(errno));
	}
	if (textdomain(TEXTDOMAIN) == NULL) {
		fprintf(stderr, "textdomain() failed: %s\n", strerror(errno));
	}
}

#  endif

#else

#  define _(x) (x)
static inline void i18n_init(void) {}

#endif

#if !defined(__BYTE_ORDER__) || !defined(__ORDER_LITTLE_ENDIAN__) || !defined(__ORDER_BIG_ENDIAN__)
#error Unsupported compiler
#endif
#if __BYTE_ORDER__ != 1234 && __BYTE_ORDER__ != 4321
#error Unexpected endian value __BYTE_ORDER__
#endif
#if __ORDER_LITTLE_ENDIAN__ != 1234
#error Unexpected endian value
#endif
#if __ORDER_BIG_ENDIAN__ != 4321
#error Unexpected endian value
#endif

#define GBS_BYTE_ORDER __BYTE_ORDER__
#define GBS_ORDER_LITTLE_ENDIAN __ORDER_LITTLE_ENDIAN__
#define GBS_ORDER_BIG_ENDIAN __ORDER_BIG_ENDIAN__

static inline long is_le_machine() {
	return __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__;
}

static inline long is_be_machine() {
	return __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__;
}

// VGM tools compatibility functions
#ifdef _WIN32
#include <conio.h>
#include <windows.h>
#else
#include <limits.h>
#ifndef MAX_PATH
#define MAX_PATH PATH_MAX
#endif
#endif

#ifdef _MSC_VER
#define stricmp		_stricmp
#define strnicmp	_strnicmp
#else
#ifndef stricmp
#define stricmp		strcasecmp
#endif
#ifndef strnicmp
#define strnicmp	strncasecmp
#endif
#endif

#define ZLIB_SEEKBUG_CHECK(vgmhdr) \
	if (vgmhdr.fccVGM != FCC_VGM) \
	{ \
		printf("VGM signature matched on the first read, but not on the second one!\n"); \
		printf("This is a known zlib bug where gzseek fails. Please install a fixed zlib.\n"); \
		goto OpenErr; \
	}

static inline void RemoveNewLines(char* String)
{
	char* strPtr;
	strPtr = String + strlen(String) - 1;
	while(strPtr >= String && (uint8_t)*strPtr < 0x20) {
		*strPtr = '\0';
		strPtr --;
	}
}

static inline void ReadFilename(char* buffer, size_t bufsize)
{
	char* retStr;
	retStr = fgets(buffer, (int)bufsize, stdin);
	if (retStr == NULL)
		buffer[0] = '\0';
#ifdef _WIN32
	if (GetConsoleCP() == GetOEMCP())
		OemToChar(buffer, buffer);
#endif
	RemoveNewLines(buffer);
}

static inline void DblClickWait(const char* argv0)
{
#ifdef _WIN32
	char* msystem;
	if (argv0[1] != ':')
		return;
	msystem = getenv("MSYSTEM");
	if (msystem != NULL) {
		if (! strncmp(msystem, "MINGW", 5) ||
			! strncmp(msystem, "MSYS", 4))
			return;
	}
	if (_kbhit())
		_getch();
	_getch();
#else
	(void)argv0;
#endif
}

#endif
