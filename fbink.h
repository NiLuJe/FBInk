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

// Magic number for automatic fbfd handling
#define FBFD_AUTO -1

// List of available fonts
typedef enum
{
	IBM            = 0,    // font8x8
	UNSCII         = 1,    // unscii-8
	UNSCII_ALT     = 2,    // unscii-8-alt
	UNSCII_THIN    = 3,    // unscii-8-thin
	UNSCII_FANTASY = 4,    // unscii-8-fantasy
	UNSCII_MCR     = 5,    // unscii-8-mcr
	UNSCII_TALL    = 6     // unscii-16
} FONT_INDEX_T;

// What a FBInk config should look like. Perfectly sane when fully zero-initialized.
typedef struct
{
	short int row;             // y axis (i.e., line), counts down from the bottom of the screen if negative
	short int col;             // x axis (i.e., column), counts down from the right edge of the screen if negative
	uint8_t   fontmult;        // Font scaling multiplier (i.e., 4 -> x4), 0 means automatic.
	uint8_t   fontname;        // Request a specific font (c.f., FONT_INDEX_T enum)
	bool      is_inverted;     // Invert colors
	bool      is_flashing;     // Request a black flash on refresh
	bool      is_cleared;      // Clear the screen beforehand (honors is_inverted)
	bool      is_centered;     // Center the text
	bool      is_padded;       // Pad the text with blanks (on the left, or on both sides if is_centered)
	bool      is_verbose;      // Print verbose diagnostic informations on stdout
	bool      is_quiet;        // Hide fbink_init()'s hardware setup info (sent to stderr)
	bool      ignore_alpha;    // Ignore any potential alpha channel in source image (i.e., flatten the image)
} FBInkConfig;

// NOTE: Unless otherwise specified,
//       stuff returns a negative value (usually -(EXIT_FAILURE)) on failure & EXIT_SUCCESS otherwise ;).

// Returns the version of the currently loaded FBInk library
FBINK_API const char* fbink_version(void) __attribute__((const));

// Open the framebuffer character device,
// Returns the newly opened file descriptor
FBINK_API int fbink_open(void);

// Unmap the framebuffer (if need be) and close its file descriptor
// (c.f., the recap at the bottom if you're concerned about mmap handling).
// fbfd:		open file descriptor to the framebuffer character device, as returned by fbink_open()
FBINK_API int fbink_close(int fbfd);

// Initialize internal variables keeping track of the framebuffer's configuration and state, as well as the device's hardware.
// MUST be called at least *once* before any fbink_print* functions.
// CAN safely be called multiple times, but doing so is only necessary if the framebuffer's state has changed,
//     or if you modified one of the FBInkConfig fields that affects its results (listed below).
// fbfd:		open file descriptor to the framebuffer character device,
//				if set to FBFD_AUTO, the fb is opened & mmap'ed for the duration of this call
// fbink_config:	pointer to an FBInkConfig struct
//				If you wish to customize them,
//				the is_centered, fontmult, fontname, is_verbose & is_quiet fields MUST be set beforehand.
//				This means you MUST call fbink_init() again when you update them, too!
// NOTE: By virtue of, well, setting global variables, do NOT consider this thread-safe.
//       The rest of the API should be, though, so make sure you init in your main thread *before* threading begins...
// NOTE: On devices where the fb state can change (i.e., Kobos switching between 16bpp & 32bpp),
//       this needs to be called as many times as necessary to ensure that every following fbink_* call will be made
//       against a fb state that matches the state it was in during the last fbink_init() call...
//       c.f., KFMon's handling of this via fbink_is_fb_quirky() to detect the initial 16bpp -> 32bpp switch.
FBINK_API int fbink_init(int fbfd, const FBInkConfig* fbink_config);

// Print a string on screen.
// NOTE: The string is expected to be encoded in valid UTF-8, no validation of any kind is done by the library,
//       and we assume a single multibyte sequence will occupy a maximum of 4 bytes.
//       c.f., my rant about Kobo's broken libc in fbink_internal.h for more details behind this choice.
//       Since any decent system built in the last decade should default to UTF-8, that should be pretty much transparent...
// Returns the amount of lines printed on success (helpful when you keep track of which row you're printing to).
// fbfd:		open file descriptor to the framebuffer character device,
//				if set to FBFD_AUTO, the fb is opened & mmap'ed for the duration of this call
// string:		UTF-8 encoded string to print
// fbink_config:	pointer to an FBInkConfig struct
FBINK_API int fbink_print(int fbfd, const char* string, const FBInkConfig* fbink_config);

// Like fbink_print, but with printf formatting ;).
// fbfd:		open file descriptor to the framebuffer character device,
//				if set to FBFD_AUTO, the fb is opened & mmap'ed for the duration of this call
// fbink_config:	pointer to an FBInkConfig struct
FBINK_API int fbink_printf(int fbfd, const FBInkConfig* fbink_config, const char* fmt, ...)
    __attribute__((format(printf, 3, 4)));

// A simple wrapper around the internal screen refresh handling, without requiring you to include einkfb/mxcfb headers
// fbfd:		open file descriptor to the framebuffer character device,
//				if set to FBFD_AUTO, the fb is opened & mmap'ed for the duration of this call
// region_top:		top field of an mxcfb rectangle
// region_left:		left field of an mxcfb rectangle
// region_width:	width field of an mxcfb rectangle
// region_height:	height field of an mxcfb rectangle
// waveform_mode:	waveform mode (i.e, "GC16")
// is_flashing:		will ask for a black flash if true
FBINK_API int fbink_refresh(int         fbfd,
			    uint32_t    region_top,
			    uint32_t    region_left,
			    uint32_t    region_width,
			    uint32_t    region_height,
			    const char* waveform_mode,
			    bool        is_flashing);

// Returns true if the device appears to be in a quirky framebuffer state
// NOTE: Right now, this only checks for the isKobo16Landscape Device Quirk,
//       because that's the only one that is not permanent (i.e., hardware specific),
//       but instead software specific (here, because of pickel).
//       In practical terms, this means the Kobo's fb is in 16bpp mode, with its origin in the top-right corner (i.e., Landscape).
FBINK_API bool fbink_is_fb_quirky(void);

// Print an image on screen
// Returns -(ENOSYS) when image support is disabled (MINIMAL build)
// fdfd:		open file descriptor to the framebuffer character device,
//				if set to FBFD_AUTO, the fb is opened & mmap'ed for the duration of this call
// filename:		path to the image file (Supported formats: JPEG, PNG, TGA, BMP, GIF & PNM)
// x_off:		target coordinates, x (honors negative offsets)
// y_off:		target coordinates, y (honors negative offsets)
// fbink_config:	pointer to an FBInkConfig struct (honors row/col, in *addition* to x_off/y_off)
FBINK_API int fbink_print_image(int                fbfd,
				const char*        filename,
				short int          x_off,
				short int          y_off,
				const FBInkConfig* fbink_config);

// Scan the screen for Kobo's "Connect" button in the "USB plugged in" popup,
// and optionally generate an input event to press that button.
// KOBO Only! Returns -(ENOSYS) when disabled (!KOBO, as well as MINIMAL builds).
// fdfd:		open file descriptor to the framebuffer character device,
//				if set to FBFD_AUTO, the fb is opened & mmap'ed for the duration of this call
// press_button:	generate an input event to press the button if true
FBINK_API int fbink_button_scan(int fbfd, bool press_button);

//
// When you intend to keep the framebuffer fd open for the lifecycle of your program:
// fd = open() -> init(fd, ...) -> print*(fd, ...) -> ... -> close(fd)
// NOTE: This implies keeping the framebuffer's mmap around, too.
//       The initial mmap will only happen on the first function call that actually needs to write to the fb, i.e., print*.
//       On the upside, that's going to be the only mmap to ever happen, as subsequent print* calls will re-use it.
//
// Otherwise, you can simply forget about open() & close(), and just do:
// init(FBFD_AUTO, ...)
// And then whenever you want to print something:
// print*(FBFD_AUTO, ...)
//
// See fbink_cmd.c for an example of the former, and KFMon for an example of the latter.

#endif
