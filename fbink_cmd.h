/*
	FBInk: FrameBuffer eInker, a library to print text & images to an eInk Linux framebuffer
	Copyright (C) 2018-2022 NiLuJe <ninuje@gmail.com>
	SPDX-License-Identifier: GPL-3.0-or-later

	----

	This program is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/

#ifndef __FBINK_CMD_H
#define __FBINK_CMD_H

#include "fbink.h"

#include <alloca.h>
#include <errno.h>
#include <fcntl.h>
#include <getopt.h>
#include <limits.h>
#include <locale.h>
#include <math.h>
#include <poll.h>
#include <signal.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <syslog.h>
#include <time.h>
#include <unistd.h>

// FBInk always returns negative values on failure
#define ERRCODE(e) (-(e))

// MIN/MAX with no side-effects,
// c.f., https://gcc.gnu.org/onlinedocs/cpp/Duplication-of-Side-Effects.html#Duplication-of-Side-Effects
//     & https://dustri.org/b/min-and-max-macro-considered-harmful.html
#define MIN(X, Y)                                                                                                        \
	({                                                                                                               \
		__auto_type x_ = (X);                                                                                    \
		__auto_type y_ = (Y);                                                                                    \
		(x_ < y_) ? x_ : y_;                                                                                     \
	})

#define MAX(X, Y)                                                                                                        \
	({                                                                                                               \
		__auto_type x__ = (X);                                                                                   \
		__auto_type y__ = (Y);                                                                                   \
		(x__ > y__) ? x__ : y__;                                                                                 \
	})

// Where our named pipe lives (/tmp should be a safe bet on every supported platform)
#define FBINK_PIPE "/tmp/fbink-fifo"

// We'll need a global instead of relying on the FBInkConfig field, because we're using these macros in various places:
// where we have a *pointer* to an FBInkConfig struct, where we have an *instance* of it, or where we have nothing...
bool toSysLog = false;

// Handle what we send to stdout (i.e., mostly recaps, no FBInk tag)
#define LOG(fmt, ...)                                                                                                    \
	({                                                                                                               \
		if (toSysLog) {                                                                                          \
			syslog(LOG_INFO, fmt, ##__VA_ARGS__);                                                            \
		} else {                                                                                                 \
			fprintf(stdout, fmt "\n", ##__VA_ARGS__);                                                        \
		}                                                                                                        \
	})

// And then what we send to stderr (add an FBInk tag to make it clear where it comes from)
#define ELOG(fmt, ...)                                                                                                   \
	({                                                                                                               \
		if (toSysLog) {                                                                                          \
			syslog(LOG_NOTICE, "[CLI] " fmt, ##__VA_ARGS__);                                                 \
		} else {                                                                                                 \
			fprintf(stderr, "[CLI] " fmt "\n", ##__VA_ARGS__);                                               \
		}                                                                                                        \
	})

// And a simple wrapper for actual warnings on error codepaths. Should only be used for warnings before a return/exit.
// Always shown, always tagged, and always ends with a bang.
#define WARN(fmt, ...)                                                                                                   \
	({                                                                                                               \
		if (toSysLog) {                                                                                          \
			syslog(LOG_ERR, "[CLI] " fmt "!", ##__VA_ARGS__);                                                \
		} else {                                                                                                 \
			fprintf(stderr, "[CLI] " fmt "!\n", ##__VA_ARGS__);                                              \
		}                                                                                                        \
	})

// Same, but with __PRETTY_FUNCTION__ right before fmt
#define PFWARN(fmt, ...) ({ WARN("[%s] " fmt, __PRETTY_FUNCTION__, ##__VA_ARGS__); })

// Likely/Unlikely branch tagging
#define likely(x)   __builtin_expect(!!(x), 1)
#define unlikely(x) __builtin_expect(!!(x), 0)

static void show_helpmsg(void);

// For our cleanup signal handler....
volatile sig_atomic_t g_timeToDie = 0;
// NOTE: We're less concerned with the atomicity of the siginfo stuff...
typedef struct
{
	int   signo;
	pid_t pid;
	uid_t uid;
} FBInkSigInfo;
FBInkSigInfo g_sigCaught = { 0 };
static void  cleanup_handler(int __attribute__((unused)), siginfo_t*, void* __attribute__((unused)));
static int   daemonize(void);

static int do_infinite_progress_bar(int, const FBInkConfig*);

static void load_ot_fonts(const char*, const char*, const char*, const char*, const FBInkConfig*, FBInkOTConfig*);

FBInkRect   totalRect = { 0U };
static void compute_lastrect(void);
static void recap_lastrect(void);
static void print_lastrect(void);

// Sprinkle a bit of C11 in there...
// c.f., http://www.robertgamble.net/2012/01/c11-generic-selections.html
#define TYPENAME(x)                                                                                                      \
	_Generic((x), _Bool                                                                                              \
		 : "bool", char                                                                                          \
		 : "char", signed char                                                                                   \
		 : "signed char", unsigned char                                                                          \
		 : "unsigned char", short int                                                                            \
		 : "short int", unsigned short int                                                                       \
		 : "unsigned short int", int                                                                             \
		 : "int", unsigned int                                                                                   \
		 : "unsigned int", long int                                                                              \
		 : "long int", unsigned long int                                                                         \
		 : "unsigned long int", float                                                                            \
		 : "float", default                                                                                      \
		 : "other")

#define TYPEMIN(x)                                                                                                       \
	_Generic((x), _Bool                                                                                              \
		 : false, char                                                                                           \
		 : CHAR_MIN, signed char                                                                                 \
		 : SCHAR_MIN, unsigned char     : 0U, short int                                                          \
		 : SHRT_MIN, unsigned short int : 0U, int                                                                \
		 : INT_MIN, unsigned int        : 0U, long int                                                           \
		 : LONG_MIN, unsigned long int  : 0U, float                                                              \
		 : -HUGE_VALF, default                                                                                   \
		 : -42)

#define TYPEMAX(x)                                                                                                       \
	_Generic((x), _Bool                                                                                              \
		 : true, char                                                                                            \
		 : CHAR_MAX, signed char                                                                                 \
		 : SCHAR_MAX, unsigned char                                                                              \
		 : UCHAR_MAX, short int                                                                                  \
		 : SHRT_MAX, unsigned short int                                                                          \
		 : USHRT_MAX, int                                                                                        \
		 : INT_MAX, unsigned int                                                                                 \
		 : UINT_MAX, long int                                                                                    \
		 : LONG_MAX, unsigned long int                                                                           \
		 : ULONG_MAX, float                                                                                      \
		 : HUGE_VALF, default : 42)

// And now we can implement generic, checked strtoul/strtol macros!
#define strtoul_chk(opt, subopt, str, result)                                                                            \
	({                                                                                                               \
		/* NOTE: We want to *reject* negative values (which strtoul does not)! */                                \
		if (strchr(str, '-')) {                                                                                  \
			ELOG("Assigned a negative value (%s) to an option (%c%s%s) expecting an %s.",                    \
			     str,                                                                                        \
			     opt,                                                                                        \
			     subopt ? ":" : "",                                                                          \
			     subopt ? subopt : "",                                                                       \
			     TYPENAME(*result));                                                                         \
			return ERRCODE(EINVAL);                                                                          \
		}                                                                                                        \
                                                                                                                         \
		/* Now that we know it's positive, we can go on with strtoul... */                                       \
		char* endptr;                                                                                            \
		errno                 = 0; /* To distinguish success/failure after call */                               \
		unsigned long int val = strtoul(str, &endptr, 10);                                                       \
                                                                                                                         \
		if ((errno == ERANGE && val == ULONG_MAX) || (errno != 0 && val == 0)) {                                 \
			PFWARN("strtoul: %m");                                                                           \
			return ERRCODE(EINVAL);                                                                          \
		}                                                                                                        \
                                                                                                                         \
		if (endptr == str) {                                                                                     \
			ELOG("No digits were found in value '%s' assigned to an option (%c%s%s) expecting an %s.",       \
			     str,                                                                                        \
			     opt,                                                                                        \
			     subopt ? ":" : "",                                                                          \
			     subopt ? subopt : "",                                                                       \
			     TYPENAME(*result));                                                                         \
			return ERRCODE(EINVAL);                                                                          \
		}                                                                                                        \
                                                                                                                         \
		/* If we got here, strtoul() successfully parsed at least part of a number. */                           \
		/* But we do want to enforce the fact that the input really was *only* an integer value. */              \
		if (*endptr != '\0') {                                                                                   \
			ELOG(                                                                                            \
			    "Found trailing characters (%s) behind value '%lu' assigned from string '%s' "               \
			    "to an option (%c%s%s) expecting an %s.",                                                    \
			    endptr,                                                                                      \
			    val,                                                                                         \
			    str,                                                                                         \
			    opt,                                                                                         \
			    subopt ? ":" : "",                                                                           \
			    subopt ? subopt : "",                                                                        \
			    TYPENAME(*result));                                                                          \
			return ERRCODE(EINVAL);                                                                          \
		}                                                                                                        \
                                                                                                                         \
		/* Make sure there isn't a loss of precision on this arch when casting explicitly */                     \
		if ((__typeof__(*result)) val != val) {                                                                  \
			ELOG(                                                                                            \
			    "Loss of precision when casting value '%lu' to an %s for option '%c%s%s' "                   \
			    "(valid range: %u to %lu).",                                                                 \
			    val,                                                                                         \
			    TYPENAME(*result),                                                                           \
			    opt,                                                                                         \
			    subopt ? ":" : "",                                                                           \
			    subopt ? subopt : "",                                                                        \
			    (unsigned int) TYPEMIN(*result),                                                             \
			    (unsigned long int) TYPEMAX(*result));                                                       \
			return ERRCODE(EINVAL);                                                                          \
		}                                                                                                        \
                                                                                                                         \
		*result = (__typeof__(*result)) val;                                                                     \
		return EXIT_SUCCESS;                                                                                     \
	})

#define strtol_chk(opt, subopt, str, result)                                                                             \
	({                                                                                                               \
		/* Go on with strtol... */                                                                               \
		char* endptr;                                                                                            \
		errno        = 0; /* To distinguish success/failure after call */                                        \
		long int val = strtol(str, &endptr, 10);                                                                 \
                                                                                                                         \
		if ((errno == ERANGE && (val == LONG_MAX || val == LONG_MIN)) || (errno != 0 && val == 0)) {             \
			PFWARN("strtol: %m");                                                                            \
			return ERRCODE(EINVAL);                                                                          \
		}                                                                                                        \
                                                                                                                         \
		if (endptr == str) {                                                                                     \
			ELOG("No digits were found in value '%s' assigned to an option (%c%s%s) expecting a %s.",        \
			     str,                                                                                        \
			     opt,                                                                                        \
			     subopt ? ":" : "",                                                                          \
			     subopt ? subopt : "",                                                                       \
			     TYPENAME(*result));                                                                         \
			return ERRCODE(EINVAL);                                                                          \
		}                                                                                                        \
                                                                                                                         \
		/* If we got here, strtol() successfully parsed at least part of a number. */                            \
		/* But we do want to enforce the fact that the input really was *only* an integer value. */              \
		if (*endptr != '\0') {                                                                                   \
			ELOG(                                                                                            \
			    "Found trailing characters (%s) behind value '%ld' assigned from string '%s' "               \
			    "to an option (%c%s%s) expecting a %s.",                                                     \
			    endptr,                                                                                      \
			    val,                                                                                         \
			    str,                                                                                         \
			    opt,                                                                                         \
			    subopt ? ":" : "",                                                                           \
			    subopt ? subopt : "",                                                                        \
			    TYPENAME(*result));                                                                          \
			return ERRCODE(EINVAL);                                                                          \
		}                                                                                                        \
                                                                                                                         \
		/* Make sure there isn't a loss of precision on this arch when casting explicitly */                     \
		if ((__typeof__(*result)) val != val) {                                                                  \
			ELOG(                                                                                            \
			    "Loss of precision when casting value '%ld' to a %s for option '%c%s%s' "                    \
			    "(valid range: %ld to %ld).",                                                                \
			    val,                                                                                         \
			    TYPENAME(*result),                                                                           \
			    opt,                                                                                         \
			    subopt ? ":" : "",                                                                           \
			    subopt ? subopt : "",                                                                        \
			    (long int) TYPEMIN(*result),                                                                 \
			    (long int) TYPEMAX(*result));                                                                \
			return ERRCODE(EINVAL);                                                                          \
		}                                                                                                        \
                                                                                                                         \
		*result = (__typeof__(*result)) val;                                                                     \
		return EXIT_SUCCESS;                                                                                     \
	})

#define strtof_chk(opt, subopt, str, result)                                                                             \
	({                                                                                                               \
		/* NOTE: We want to *reject* negative values */                                                          \
		if (strchr(str, '-')) {                                                                                  \
			ELOG("Assigned a negative value (%s) to an option (%c%s%s) expecting a positive %s.",            \
			     str,                                                                                        \
			     opt,                                                                                        \
			     subopt ? ":" : "",                                                                          \
			     subopt ? subopt : "",                                                                       \
			     TYPENAME(*result));                                                                         \
			return ERRCODE(EINVAL);                                                                          \
		}                                                                                                        \
                                                                                                                         \
		/* NOTE: Use the POSIX variant to enforce a specific locale, */                                          \
		/*       ensuring the radix point character will always be a dot, and not a comma, */                    \
		/*       because that wouldn't play nice with getsubopt... */                                            \
		locale_t c_loc;                                                                                          \
		c_loc = newlocale(LC_NUMERIC_MASK, "C", (locale_t) 0);                                                   \
		if (c_loc == (locale_t) 0) {                                                                             \
			PFWARN("newlocale: %m");                                                                         \
			return ERRCODE(EINVAL);                                                                          \
		}                                                                                                        \
                                                                                                                         \
		/* Now that we know it's positive, we can go on with strtof... */                                        \
		char* endptr;                                                                                            \
		errno     = 0; /* To distinguish success/failure after call */                                           \
		float val = strtof_l(str, &endptr, c_loc);                                                               \
                                                                                                                         \
		freelocale(c_loc);                                                                                       \
                                                                                                                         \
		if ((errno == ERANGE && (val == HUGE_VALF || val == -HUGE_VALF)) || (errno != 0 && val == 0)) {          \
			PFWARN("strtof: %m");                                                                            \
			return ERRCODE(EINVAL);                                                                          \
		}                                                                                                        \
                                                                                                                         \
		if (endptr == str) {                                                                                     \
			ELOG("No digits were found in value '%s' assigned to an option (%c%s%s) expecting a %s.",        \
			     str,                                                                                        \
			     opt,                                                                                        \
			     subopt ? ":" : "",                                                                          \
			     subopt ? subopt : "",                                                                       \
			     TYPENAME(*result));                                                                         \
			return ERRCODE(EINVAL);                                                                          \
		}                                                                                                        \
                                                                                                                         \
		/* If we got here, strtof() successfully parsed at least part of a number. */                            \
		/* But we do want to enforce the fact that the input really was *only* a decimal value. */               \
		if (*endptr != '\0') {                                                                                   \
			ELOG(                                                                                            \
			    "Found trailing characters (%s) behind value '%f' assigned from string '%s' "                \
			    "to an option (%c%s%s) expecting a %s.",                                                     \
			    endptr,                                                                                      \
			    val,                                                                                         \
			    str,                                                                                         \
			    opt,                                                                                         \
			    subopt ? ":" : "",                                                                           \
			    subopt ? subopt : "",                                                                        \
			    TYPENAME(*result));                                                                          \
			return ERRCODE(EINVAL);                                                                          \
		}                                                                                                        \
                                                                                                                         \
		/* Reject infinity & NaN */                                                                              \
		if (!isfinite(val)) {                                                                                    \
			ELOG("Assigned a non-finite value (%f from '%s') to an option (%c%s%s) expecting a %s.",         \
			     val,                                                                                        \
			     str,                                                                                        \
			     opt,                                                                                        \
			     subopt ? ":" : "",                                                                          \
			     subopt ? subopt : "",                                                                       \
			     TYPENAME(*result));                                                                         \
			return ERRCODE(EINVAL);                                                                          \
		}                                                                                                        \
                                                                                                                         \
		/* Make sure there isn't a loss of precision on this arch when casting explicitly */                     \
		if ((__typeof__(*result)) val != val) {                                                                  \
			ELOG(                                                                                            \
			    "Loss of precision when casting value '%f' to a %s for option '%c%s%s' "                     \
			    "(valid range: %f to %f).",                                                                  \
			    val,                                                                                         \
			    TYPENAME(*result),                                                                           \
			    opt,                                                                                         \
			    subopt ? ":" : "",                                                                           \
			    subopt ? subopt : "",                                                                        \
			    (float) TYPEMIN(*result),                                                                    \
			    (float) TYPEMAX(*result));                                                                   \
			return ERRCODE(EINVAL);                                                                          \
		}                                                                                                        \
                                                                                                                         \
		*result = (__typeof__(*result)) val;                                                                     \
		return EXIT_SUCCESS;                                                                                     \
	})

// And we'll use those through these...
static int strtoul_u(int, const char*, const char*, uint32_t*);
static int strtoul_hu(int, const char*, const char*, uint16_t*);
static int strtoul_hhu(int, const char*, const char*, uint8_t*);
static int strtol_hi(int, const char*, const char*, short int*);
static int strtol_hhi(int, const char*, const char*, int8_t*);
static int strtof_pos(int, const char*, const char*, float*);

#endif
