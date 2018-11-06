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
#include <math.h>
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
		 : "unsigned long long int", float                                                                       \
		 : "float", double                                                                                       \
		 : "double", long double                                                                                 \
		 : "long double", default                                                                                \
		 : "other")

#define TYPEMIN(x)                                                                                                       \
	_Generic((x), _Bool : 0, char                                                                                    \
		 : CHAR_MIN, signed char                                                                                 \
		 : SCHAR_MIN, unsigned char : 0U, short int                                                              \
		 : SHRT_MIN, unsigned short int : 0U, int                                                                \
		 : INT_MIN, unsigned int : 0U, long int                                                                  \
		 : LONG_MIN, unsigned long int : 0U, long long int                                                       \
		 : LLONG_MIN, unsigned long long int : 0U, float                                                         \
		 : -HUGE_VALF, double                                                                                    \
		 : -HUGE_VAL, long double                                                                                \
		 : -HUGE_VALL, default                                                                                   \
		 : -42)

#define TYPEMAX(x)                                                                                                       \
	_Generic((x), _Bool : 0, char                                                                                    \
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
		 : ULLONG_MAX, float                                                                                     \
		 : HUGE_VALF, double                                                                                     \
		 : HUGE_VAL, long double                                                                                 \
		 : HUGE_VALL, default : 42)

static int strtoul_u(int, const char*, const char*, uint32_t*);
static int strtoul_hu(int, const char*, const char*, unsigned short int*);
static int strtoul_hhu(int, const char*, const char*, uint8_t*);
static int strtol_hi(int, const char*, const char*, short int*);
static int strtol_hhi(int, const char*, const char*, int8_t*);

#endif
