/*
	FBInk: FrameBuffer eInker, a tool to print strings on eInk devices (Kobo/Kindle)
	Copyright (C) 2018 NiLuJe <ninuje@gmail.com>

	Linux framebuffer routines based on: fbtestfnt.c & fbtest6.c, from
	http://raspberrycompote.blogspot.com/2014/04/low-level-graphics-on-raspberry-pi-text.html &
	https://raspberrycompote.blogspot.com/2013/03/low-level-graphics-on-raspberry-pi-part_8.html
	Original works by J-P Rosti (a.k.a -rst- and 'Raspberry Compote'),
	Licensed under the Creative Commons Attribution 3.0 Unported License
	(http://creativecommons.org/licenses/by/3.0/deed.en_US)

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

#ifndef __FBINK_H
#define __FBINK_H

// Because we're pretty much Linux-bound ;).
#ifndef _GNU_SOURCE
#	define _GNU_SOURCE
#endif

#include <stdarg.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

// Symbol visibility shenanigans...
// c.f., https://gcc.gnu.org/wiki/Visibility
#if __GNUC__ >= 4
#	define DLL_PUBLIC __attribute__((visibility("default")))
#	define DLL_LOCAL __attribute__((visibility("hidden")))
#else
#	define DLL_PUBLIC
#	define DLL_LOCAL
#endif

// Are we actually building the shared lib?
#ifdef FBINK_SHAREDLIB
#	define FBINK_API DLL_PUBLIC
#	define FBINK_LOCAL DLL_LOCAL
#else
#	define FBINK_API
#	define FBINK_LOCAL
#endif

// The few externs we might need...
extern FBINK_API char* fbp;
extern FBINK_API size_t screensize;
extern FBINK_API bool   fb_is_mapped;

// What a FBInk Print config should look like
typedef struct
{
	short int row;
	short int col;
	bool      is_inverted;
	bool      is_flashing;
	bool      is_cleared;
	bool      is_centered;
	bool      is_padded;
} FBInkConfig;

// Return the version of the currently loaded FBInk library
FBINK_API const char* fbink_version(void);

// Open the framebuffer device and returns its fd
FBINK_API int fbink_open(void);

// Initialize the global variables.
// If fd is -1, the fb is opened for the duration of this call
// NOTE: By virtue of, well, setting global variables, do NOT consider this thread-safe.
//       The rest of the API should be, though, so make sure you init in your main thread *before* threading begins...
FBINK_API int fbink_init(int);

// Print a string on screen.
// NOTE: The string is expected to be encoded in valid UTF-8, no validation of any kind is done by the library,
//       and we assume a single multibyte sequence will occupy a maximum of 4 bytes.
//       c.f., my rant about Kobo's broken libc in fbink_internal.h for more details behind this choice.
//       Since any decent system of the last decade should default to UTF-8, that should be pretty much transparent...
// if fd is -1, the fb is opened for the duration of this call
FBINK_API int fbink_print(int, const char*, FBInkConfig*);

// Like fbink_print, but with printf formatting ;).
FBINK_API int fbink_printf(int, FBInkConfig*, const char*, ...) __attribute__((format(printf, 3, 4)));

// And a simple wrapper around the internal refresh, without having to include mxcfb headers
FBINK_API int fbink_refresh(int, uint32_t, uint32_t, uint32_t, uint32_t, const char*, bool);

// When you intend to keep fd open for the lifecycle of your program:
// fd = open() -> init(fd) -> print(fd, ...)
//
// Otherwise:
// init(-1)
// And then whenever you want to print something:
// print(-1, ...)

#endif
