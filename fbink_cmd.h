/*
	FBInk: FrameBuffer eInker, a tool to print text & images on eInk devices (Kobo/Kindle)
	Copyright (C) 2018 NiLuJe <ninuje@gmail.com>

	----

	This program is free software: you can redistribute it and/or modify
	it under the terms of the GNU Affero General Public License as
	published by the Free Software Foundation, either version 3 of the
	License, or (at your option) any later version.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU Affero General Public License for more details.

	You should have received a copy of the GNU Affero General Public License
	along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef __FBINK_CMD_H
#define __FBINK_CMD_H

#include "fbink.h"

#include <errno.h>
#include <getopt.h>
#include <limits.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <time.h>
#include <unistd.h>

// FBInk always returns negative values on failure
#define ERRCODE(e) (-(e))

static void show_helpmsg(void);

static int do_infinite_progress_bar(int, const FBInkConfig*);

// Sprinkle a bit of C11 in there...
// c.f., http://www.robertgamble.net/2012/01/c11-generic-selections.html
#define TYPENAME(x)                                                                                                      \
	_Generic((x), _Bool                                                                                              \
		 : "_Bool", char                                                                                         \
		 : "char", signed char                                                                                   \
		 : "signed char", unsigned char                                                                          \
		 : "unsigned char", short int                                                                            \
		 : "short int", unsigned short int                                                                       \
		 : "unsigned short int", int                                                                             \
		 : "int", unsigned int                                                                                   \
		 : "unsigned int", long int                                                                              \
		 : "long int", unsigned long int                                                                         \
		 : "unsigned long int", long long int                                                                    \
		 : "long long int", unsigned long long int                                                               \
		 : "unsigned long long int", default                                                                     \
		 : "other")

#define TYPEMIN(x)                                                                                                       \
	_Generic((x), _Bool : 0, char                                                                                    \
		 : CHAR_MIN, signed char                                                                                 \
		 : SCHAR_MIN, unsigned char : 0U, short int                                                              \
		 : SHRT_MIN, unsigned short int : 0U, int                                                                \
		 : INT_MIN, unsigned int : 0U, long int                                                                  \
		 : LONG_MIN, unsigned long int : 0U, long long int                                                       \
		 : LLONG_MIN, unsigned long long int : 0U, default                                                       \
		 : -42)

#define TYPEMAX(x)                                                                                                       \
	_Generic((x), _Bool : 1, char                                                                                    \
		 : CHAR_MAX, signed char                                                                                 \
		 : SCHAR_MAX, unsigned char                                                                              \
		 : UCHAR_MAX, short int                                                                                  \
		 : SHRT_MAX, unsigned short int                                                                          \
		 : USHRT_MAX, int                                                                                        \
		 : INT_MAX, unsigned int                                                                                 \
		 : UINT_MAX, long int                                                                                    \
		 : LONG_MAX, unsigned long int                                                                           \
		 : ULONG_MAX, long long int                                                                              \
		 : LLONG_MAX, unsigned long long int                                                                     \
		 : ULLONG_MAX, default : 42)

// And now we can implement generic, checked strtoul/strtol macros!
#define strtoul_chk(opt, subopt, str, result)                                                                                                       \
	({                                                                                                                                          \
		/* NOTE: We want to *reject* negative values (which strtoul does not)! */                                                           \
		if (strchr(str, '-')) {                                                                                                             \
			fprintf(stderr,                                                                                                             \
				"Assigned a negative value (%s) to an option (%c%s%s) expecting a %s.\n",                                           \
				str,                                                                                                                \
				opt,                                                                                                                \
				subopt ? ":" : "",                                                                                                  \
				subopt ? subopt : "",                                                                                               \
				TYPENAME(*result));                                                                                                 \
			return ERRCODE(EINVAL);                                                                                                     \
		}                                                                                                                                   \
                                                                                                                                                    \
		/* Now that we know it's positive, we can go on with strtoul... */                                                                  \
		char*             endptr;                                                                                                           \
		unsigned long int val;                                                                                                              \
                                                                                                                                                    \
		errno = 0; /* To distinguish success/failure after call */                                                                          \
		val   = strtoul(str, &endptr, 10);                                                                                                  \
                                                                                                                                                    \
		if ((errno == ERANGE && val == ULONG_MAX) || (errno != 0 && val == 0)) {                                                            \
			perror("[FBInk] strtoul");                                                                                                  \
			return ERRCODE(EINVAL);                                                                                                     \
		}                                                                                                                                   \
                                                                                                                                                    \
		if (endptr == str) {                                                                                                                \
			fprintf(stderr,                                                                                                             \
				"No digits were found in value '%s' assigned to an option (%c%s%s) expecting a %s.\n",                              \
				str,                                                                                                                \
				opt,                                                                                                                \
				subopt ? ":" : "",                                                                                                  \
				subopt ? subopt : "",                                                                                               \
				TYPENAME(*result));                                                                                                 \
			return ERRCODE(EINVAL);                                                                                                     \
		}                                                                                                                                   \
                                                                                                                                                    \
		/* If we got here, strtoul() successfully parsed at least part of a number. */                                                      \
		/* But we do want to enforce the fact that the input really was *only* an integer value. */                                         \
		if (*endptr != '\0') {                                                                                                              \
			fprintf(                                                                                                                    \
			    stderr,                                                                                                                 \
			    "Found trailing characters (%s) behind value '%lu' assigned from string '%s' to an option (%c%s%s) expecting an %s.\n", \
			    endptr,                                                                                                                 \
			    val,                                                                                                                    \
			    str,                                                                                                                    \
			    opt,                                                                                                                    \
			    subopt ? ":" : "",                                                                                                      \
			    subopt ? subopt : "",                                                                                                   \
			    TYPENAME(*result));                                                                                                     \
			return ERRCODE(EINVAL);                                                                                                     \
		}                                                                                                                                   \
                                                                                                                                                    \
		/* Make sure there isn't a loss of precision on this arch when casting explictly */                                                 \
		if ((__typeof__(*result)) val != val) {                                                                                             \
			fprintf(                                                                                                                    \
			    stderr,                                                                                                                 \
			    "Loss of precision when casting value '%lu' to a %s for option '%c%s%s' (valid range: %u to %llu).\n",                  \
			    val,                                                                                                                    \
			    TYPENAME(*result),                                                                                                      \
			    opt,                                                                                                                    \
			    subopt ? ":" : "",                                                                                                      \
			    subopt ? subopt : "",                                                                                                   \
			    (unsigned int) TYPEMIN(*result),                                                                                        \
			    (unsigned long long int) TYPEMAX(*result));                                                                             \
			return ERRCODE(EINVAL);                                                                                                     \
		}                                                                                                                                   \
                                                                                                                                                    \
		*result = (__typeof__(*result)) val;                                                                                                \
		return EXIT_SUCCESS;                                                                                                                \
	})

#define strtol_chk(opt, subopt, str, result)                                                                                                        \
	({                                                                                                                                          \
		/* Go on with strtol... */                                                                                                          \
		char*    endptr;                                                                                                                    \
		long int val;                                                                                                                       \
                                                                                                                                                    \
		errno = 0; /* To distinguish success/failure after call */                                                                          \
		val   = strtol(str, &endptr, 10);                                                                                                   \
                                                                                                                                                    \
		if ((errno == ERANGE && (val == LONG_MAX || val == LONG_MIN)) || (errno != 0 && val == 0)) {                                        \
			perror("[FBInk] strtol");                                                                                                   \
			return ERRCODE(EINVAL);                                                                                                     \
		}                                                                                                                                   \
                                                                                                                                                    \
		if (endptr == str) {                                                                                                                \
			fprintf(stderr,                                                                                                             \
				"No digits were found in value '%s' assigned to an option (%c%s%s) expecting a %s.\n",                              \
				str,                                                                                                                \
				opt,                                                                                                                \
				subopt ? ":" : "",                                                                                                  \
				subopt ? subopt : "",                                                                                               \
				TYPENAME(*result));                                                                                                 \
			return ERRCODE(EINVAL);                                                                                                     \
		}                                                                                                                                   \
                                                                                                                                                    \
		/* If we got here, strtol() successfully parsed at least part of a number. */                                                       \
		/* But we do want to enforce the fact that the input really was *only* an integer value. */                                         \
		if (*endptr != '\0') {                                                                                                              \
			fprintf(                                                                                                                    \
			    stderr,                                                                                                                 \
			    "Found trailing characters (%s) behind value '%ld' assigned from string '%s' to an option (%c%s%s) expecting an %s.\n", \
			    endptr,                                                                                                                 \
			    val,                                                                                                                    \
			    str,                                                                                                                    \
			    opt,                                                                                                                    \
			    subopt ? ":" : "",                                                                                                      \
			    subopt ? subopt : "",                                                                                                   \
			    TYPENAME(*result));                                                                                                     \
			return ERRCODE(EINVAL);                                                                                                     \
		}                                                                                                                                   \
                                                                                                                                                    \
		/* Make sure there isn't a loss of precision on this arch when casting explictly */                                                 \
		if ((__typeof__(*result)) val != val) {                                                                                             \
			fprintf(                                                                                                                    \
			    stderr,                                                                                                                 \
			    "Loss of precision when casting value '%ld' to a %s for option '%c%s%s' (valid range: %d to %lld).\n",                  \
			    val,                                                                                                                    \
			    TYPENAME(*result),                                                                                                      \
			    opt,                                                                                                                    \
			    subopt ? ":" : "",                                                                                                      \
			    subopt ? subopt : "",                                                                                                   \
			    (int) TYPEMIN(*result),                                                                                                 \
			    (long long int) TYPEMAX(*result));                                                                                      \
			return ERRCODE(EINVAL);                                                                                                     \
		}                                                                                                                                   \
                                                                                                                                                    \
		*result = (__typeof__(*result)) val;                                                                                                \
		return EXIT_SUCCESS;                                                                                                                \
	})

// And we'll use those through these...
static int strtoul_u(int, const char*, const char*, uint32_t*);
static int strtoul_hu(int, const char*, const char*, unsigned short int*);
static int strtoul_hhu(int, const char*, const char*, uint8_t*);
static int strtol_hi(int, const char*, const char*, short int*);
static int strtol_hhi(int, const char*, const char*, int8_t*);

#endif
