/*
	FBInk: FrameBuffer eInker, a library to print text & images to an eInk Linux framebuffer
	Copyright (C) 2018-2020 NiLuJe <ninuje@gmail.com>
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

#ifndef __FBINK_INTERNAL_H
#define __FBINK_INTERNAL_H

// Make sure that KOBO is our default/fallback platform.
// NOTE: The Makefile already takes care of this, this is mainly to make my IDE happy.
#ifndef FBINK_FOR_KINDLE
#	ifndef FBINK_FOR_CERVANTES
#		ifndef FBINK_FOR_LINUX
#			ifndef FBINK_FOR_KOBO
#				ifndef FBINK_FOR_REMARKABLE
#					ifndef FBINK_FOR_POCKETBOOK
#						define FBINK_FOR_KOBO
#					endif
#				endif
#			endif
#		endif
#	endif
#endif

// No extra fonts, no image support, and no OpenType support in minimal builds
#ifndef FBINK_MINIMAL
#	ifndef FBINK_WITH_FONTS
#		define FBINK_WITH_FONTS
#	endif
#	ifndef FBINK_WITH_IMAGE
#		define FBINK_WITH_IMAGE
#	endif
#	ifndef FBINK_WITH_OPENTYPE
#		define FBINK_WITH_OPENTYPE
#	endif
// Connect button scanning is Kobo specific
#	ifdef FBINK_FOR_KOBO
#		ifndef FBINK_WITH_BUTTON_SCAN
#			define FBINK_WITH_BUTTON_SCAN
#		endif
#	endif
#endif
// Bundling Unifont, on the other hand, is *always* optional.

// Try to use GCC's iceilf builtin if possible...
// NOTE: Relies on the fact that:
//       * Clang implements the __has_builtin macro, but currently not the __builtin_iceilf function
//       * GCC implements the __builtin_iceilf function, but probably not the __has_builtin macro (it was introduced in GCC 10.1)
// c.f., https://stackoverflow.com/q/4322352/
// NOTE: The idea here is to rely as much as possible on GCC builtins.
//       GCC should mostly have handled this right on its own, we're just giving it a nudge ;).
//       The original idea was to see if we could actually completely avoid libm, but we really can't since,
//       if need be, GCC builtins *may* emit library calls. Which is the case here ;).
#ifdef FBINK_WITH_OPENTYPE
// Since we can't avoid libm, include the standard header, so everyones gets the right declarations
#	include <math.h>
#	ifdef __clang__
#		if __has_builtin(__builtin_ceilf)
#			define iceilf(x)      ((int) (__builtin_ceilf(x)))
#			define STBTT_iceil(x) ((int) (__builtin_ceilf(x)))
#		endif
#		if __has_builtin(__builtin_lroundf)
#			define iroundf(x) ((int) (__builtin_lroundf(x)))
#		endif
#		if __has_builtin(__builtin_floorf)
#			define ifloorf(x)      ((int) (__builtin_floorf(x)))
#			define STBTT_floorf(x) __builtin_floorf(x)
#			define STBTT_ifloor(x) ((int) (__builtin_floorf(x)))
#		endif
#		if __has_builtin(__builtin_sqrtf)
#			define STBTT_sqrt(x) __builtin_sqrtf(x)
#		endif
#		if __has_builtin(__builtin_powf)
#			define STBTT_pow(x, y) __builtin_powf(x, y)
#		endif
#		if __has_builtin(__builtin_fmodf)
#			define STBTT_fmod(x, y) __builtin_fmodf(x, y)
#		endif
#		if __has_builtin(__builtin_cosf)
#			define STBTT_cos(x) __builtin_cosf(x)
#		endif
#		if __has_builtin(__builtin_acosf)
#			define STBTT_acos(x) __builtin_acosf(x)
#		endif
#		if __has_builtin(__builtin_fabsf)
#			define STBTT_fabs(x) __builtin_fabsf(x)
#		endif
#	else
// Hide all this behind a C99 check, to try to avoid blowing up on really old GCC versions...
#		if __STDC_VERSION__ >= 199901L
#			define iceilf(x)        __builtin_iceilf(x)
#			define iroundf(x)       __builtin_iroundf(x)
#			define ifloorf(x)       __builtin_ifloorf(x)
#			define STBTT_floorf(x)  __builtin_floorf(x)
#			define STBTT_ifloor(x)  __builtin_ifloorf(x)
#			define STBTT_iceil(x)   __builtin_iceilf(x)
#			define STBTT_sqrt(x)    __builtin_sqrtf(x)
#			define STBTT_pow(x, y)  __builtin_powf(x, y)
#			define STBTT_fmod(x, y) __builtin_fmodf(x, y)
// Need to keep double precision for cos, because GCC sneakily replaces a PI approximation in stbtt ;)
#			define STBTT_cos(x)     __builtin_cos(x)
#			define STBTT_acos(x)    __builtin_acosf(x)
#			define STBTT_fabs(x)    __builtin_fabsf(x)
#		endif
#	endif
#endif

// Likely/Unlikely branch tagging
#define likely(x)   __builtin_expect(!!(x), 1)
#define unlikely(x) __builtin_expect(!!(x), 0)

#include <errno.h>
#include <fcntl.h>
#include <limits.h>
#include <linux/fb.h>
#include <linux/kd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <syslog.h>
#include <unistd.h>

// NOTE: This is from https://www.cprogramming.com/tutorial/unicode.html
//         (now https://github.com/JeffBezanson/cutef8)
//         (as well as https://github.com/JuliaLang/julia/blob/master/src/support/utf8.c)
//       This helps us handling *VALID* UTF-8 properly, since our fonts can (and do) support non-ASCII scripts.
//       That said, we have no (easy) way of ensuring all input either is or gets converted to valid UTF-8,
//       because the libc is severely hobbled on Kobo: gconv modules are missing & locales are missing or broken,
//       (at the very least), so we can't use iconv and we can't use locale/multibyte aware libc functions,
//       which in turn renders the wchar_t data type fairly unusable.
//       FWIW, we're okay on the Kindle.
//       BusyBox can bypass some of these issues (for its applets) by basically rolling its own
//       locale/unicode/widechar/multibyte handling, provided you set it up properly ;).
//       We're left with handling UTF-8 ourselves, and taking great pains to try not to horribly blow up on invalid input.
//       The situation has improved with the latest cutef8 builds (FBInk >= 1.9.4), since we now have a validation routine,
//       which we employ to make sure the string passed to fbink_print & fbink_print_ot is a valid UTF-8 sequence.
//
//       TL;DR; for API users: You have to ensure you feed FBInk *valid* UTF-8 input,
//              as this is the encoding it effectively uses internally.
//              Since FBInk 1.9.4, invalid UTF-8 sequences will be *rejected* (there was no kind of validation before).
//
//       NOTE: There's a few comments strewn in the code about u8_strlen/u8_nextchar & using wide-NULLs to avoid issues.
//             The underlying issue *might* have been fixed in our current cutef8 build.
//             That said, I'm not risking anything just to save 3 bytes, so, these quirks stay ;).
//
//       Further reading on the subject, in no particular order:
//           http://bjoern.hoehrmann.de/utf-8/decoder/dfa
//           https://bitbucket.org/knight666/utf8rewind
//           https://github.com/benkasminbullock/unicode-c
//           https://stackoverflow.com/q/7298059
//           https://github.com/JuliaLang/utf8proc
//           https://stackoverflow.com/q/25803627
//           https://www.gnu.org/software/libunistring
//           https://www.tldp.org/HOWTO/Unicode-HOWTO-6.html
//           https://www.cl.cam.ac.uk/~mgk25/unicode.html
//           https://unicodebook.readthedocs.io/
//           https://www.cl.cam.ac.uk/~mgk25/ucs/examples/UTF-8-test.txt
#include "cutef8/utf8.h"
// NOTE: And we're also using a few things based on http://bjoern.hoehrmann.de/utf-8/decoder/dfa
#include "cutef8/dfa.h"

// We're going to need a few more things for OpenType support...
#ifdef FBINK_WITH_OPENTYPE
// We need libunibreak for word breaking
#	include "libunibreak/src/linebreak.h"
#endif

// NOTE: We always neeed one of those, because we rely on mxcfb_rect in a number of places
#if defined(FBINK_FOR_KINDLE)
#	include "eink/mxcfb-kindle.h"
// Legacy einkfb driver
#	include "eink/einkfb.h"
#elif defined(FBINK_FOR_CERVANTES)
#	include "eink/mxcfb-cervantes.h"
#elif defined(FBINK_FOR_KOBO)
#	include "eink/mxcfb-kobo.h"
#elif defined(FBINK_FOR_REMARKABLE)
#	include "eink/mxcfb-remarkable.h"
#elif defined(FBINK_FOR_POCKETBOOK)
#	include "eink/mxcfb-pocketbook.h"
#	include "eink/mxcfb-pocketbook-compat.h"
// We'll try to avoid being tainted by InkView as much as possible...
#	include <dlfcn.h>
#elif defined(FBINK_FOR_LINUX)
// Fallback, because, even on straight Linux, we require a few mxcfb typedefs for some of our own function prototypes...
#	include "eink/mxcfb-kobo.h"
#endif

// NOTE: This is from https://github.com/dhepper/font8x8
//       See also https://github.com/achilikin/bdfe & https://unix.stackexchange.com/q/119236
//       for *potentially* fun ways to add new glyphs or just change the font ;).
//       Right now, there's a bit of a format mismatch issue...
//       But in any case, for a repository of BDF fonts, see https://github.com/Tecate/bitmap-fonts
// NOTE: On the other hand, tools/hextoc.py is a perfectly usable experiment with Unifont's hex format,
//       that can yield you a choice of a few different fonts ;).
//       And with potentially a tiny bit of additional work, can work with Hex files exported from BDF
//       or other bitmap fonts by gbdfed (with maybe an initial FontForge conversion to BDF if need be) ;).
#include "font8x8/font8x8_basic.h"
#include "font8x8/font8x8_block.h"
#include "font8x8/font8x8_box.h"
#include "font8x8/font8x8_control.h"
#include "font8x8/font8x8_ext_latin.h"
#include "font8x8/font8x8_greek.h"
#include "font8x8/font8x8_hiragana.h"
// NOTE: See https://github.com/ansilove/BlockZone for an awesome vector version of the 8x16 VGA variant of this IBM font ;).
// NOTE: As well as https://int10h.org/oldschool-pc-fonts for both vector & bitmap versions of most IBM fonts!
// NOTE: And https://farsil.github.io/ibmfonts for BDF conversions, in case gbdfed wasn't enough ;).'

// Where our (internal) typedefs dwell...
#include "fbink_types.h"

// Speaking of, include the Unscii variants when we're not a minimal build
#ifdef FBINK_WITH_FONTS
#	include "fbink_unscii.h"
#	include "fbink_block.h"
#	include "fbink_leggie.h"
#	include "fbink_orp.h"
#	include "fbink_scientifica.h"
#	include "fbink_terminus.h"
#	include "fbink_fatty.h"
#	include "fbink_spleen.h"
#	include "fbink_tewi.h"
#	include "fbink_misc_fonts.h"
#	include "fbink_topaz.h"
#	include "fbink_microknight.h"
#	include "fbink_vga.h"
#	ifdef FBINK_WITH_UNIFONT
#		include "fbink_unifont.h"
#	endif
#	include "fbink_cozette.h"
#endif

// NOTE: CLOEXEC shenanigans...
//       Story time: it was introduced (for open) in Linux 2.6.23. Kindle FW 2.x runs something older,
//       and I can't be arsed to check if they backported it in there or not.
//       This means we effectively can't use CLOEXEC on LEGACY builds.
//       But, our LEGACY TC actually uses an LTS kernel that *does* support CLOEXEC,
//       so we have to forcefully scrap it to make sure we're not building w/ CLOEXEC support baked in for LEGACY builds...
#ifdef FBINK_FOR_LEGACY
#	ifdef O_CLOEXEC
#		undef O_CLOEXEC
#	endif
#	define O_CLOEXEC 0
#	define STDIO_CLOEXEC
#else
#	define STDIO_CLOEXEC "e"
#endif

// Fallback version tag...
#ifndef FBINK_VERSION
#	define FBINK_FALLBACK_VERSION "v1.23.0-git"
#	ifdef FBINK_FOR_KINDLE
#		define FBINK_VERSION FBINK_FALLBACK_VERSION " for Kindle"
#	else
#		ifdef FBINK_FOR_CERVANTES
#			define FBINK_VERSION FBINK_FALLBACK_VERSION " for Cervantes"
#		else
#			ifdef FBINK_FOR_REMARKABLE
#				define FBINK_VERSION FBINK_FALLBACK_VERSION " for reMarkable"
#			else
#				ifdef FBINK_FOR_POCKETBOOK
#					define FBINK_VERSION FBINK_FALLBACK_VERSION " for PocketBook"
#				else
#					ifdef FBINK_FOR_LINUX
#						define FBINK_VERSION FBINK_FALLBACK_VERSION " for Linux"
#					else
#						ifdef FBINK_FOR_KOBO
#							define FBINK_VERSION FBINK_FALLBACK_VERSION " for Kobo"
#						else
#							define FBINK_VERSION FBINK_FALLBACK_VERSION
#						endif
#					endif
#				endif
#			endif
#		endif
#	endif
#endif

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

// We'll need those on PocketBook...
// c.f., <linux/kernel.h>
#define ALIGN(x, a)                                                                                                      \
	({                                                                                                               \
		__auto_type mask__ = (a) -1U;                                                                            \
		(((x) + (mask__)) & ~(mask__));                                                                          \
	})
#define IS_ALIGNED(x, a)                                                                                                 \
	({                                                                                                               \
		__auto_type mask__ = (a) -1U;                                                                            \
		((x) & (mask__)) == 0 ? true : false;                                                                    \
	})

// NOTE: Some of our ifdef combinations may cause a small number of function arguments to become unused...
//       Handle the warnings in these cases with a bit of trickery,
//       by conditionally marking theses arguments as unused ;).
#ifdef FBINK_MINIMAL
#	define UNUSED_BY_NOTMINIMAL
#	define UNUSED_BY_MINIMAL __attribute__((unused))
#else
#	define UNUSED_BY_NOTMINIMAL __attribute__((unused))
#	define UNUSED_BY_MINIMAL
#endif
#ifndef FBINK_WITH_BUTTON_SCAN
#	define UNUSED_BY_NOBUTTON __attribute__((unused))
#else
#	define UNUSED_BY_NOBUTTON
#endif
#ifdef FBINK_FOR_KINDLE
#	define UNUSED_BY_NOTKINDLE
#	define UNUSED_BY_KINDLE __attribute__((unused))
#else
#	define UNUSED_BY_NOTKINDLE __attribute__((unused))
#	define UNUSED_BY_KINDLE
#endif
#ifdef FBINK_FOR_KOBO
#	define UNUSED_BY_NOTKOBO
#	define UNUSED_BY_KOBO __attribute__((unused))
#else
#	define UNUSED_BY_NOTKOBO __attribute__((unused))
#	define UNUSED_BY_KOBO
#endif
#ifdef FBINK_FOR_CERVANTES
#	define UNUSED_BY_NOTCERVANTES
#	define UNUSED_BY_CERVANTES __attribute__((unused))
#else
#	define UNUSED_BY_NOTCERVANTES __attribute__((unused))
#	define UNUSED_BY_CERVANTES
#endif
#ifdef FBINK_FOR_REMARKABLE
#	define UNUSED_BY_NOTREMARKABLE
#	define UNUSED_BY_REMARKABLE __attribute__((unused))
#else
#	define UNUSED_BY_NOTREMARKABLE __attribute__((unused))
#	define UNUSED_BY_REMARKABLE
#endif
#ifdef FBINK_FOR_POCKETBOOK
#	define UNUSED_BY_NOTPOCKETBOOK
#	define UNUSED_BY_POCKETBOOK __attribute__((unused))
#else
#	define UNUSED_BY_NOTPOCKETBOOK __attribute__((unused))
#	define UNUSED_BY_POCKETBOOK
#endif
#ifdef FBINK_FOR_LINUX
#	define UNUSED_BY_NOTLINUX
#	define UNUSED_BY_LINUX __attribute__((unused))
#else
#	define UNUSED_BY_NOTLINUX __attribute__((unused))
#	define UNUSED_BY_LINUX
#endif

// Handle what we send to stdout (i.e., mostly diagnostic stuff, which tends to be verbose, so no FBInk tag)
#define LOG(fmt, ...)                                                                                                    \
	({                                                                                                               \
		if (g_isVerbose) {                                                                                       \
			if (g_toSysLog) {                                                                                \
				syslog(LOG_INFO, fmt, ##__VA_ARGS__);                                                    \
			} else {                                                                                         \
				fprintf(stdout, fmt "\n", ##__VA_ARGS__);                                                \
			}                                                                                                \
		}                                                                                                        \
	})

// And then what we send to stderr (mostly fbink_init stuff, add an FBInk tag to make it clear where it comes from for API users)
#define ELOG(fmt, ...)                                                                                                   \
	({                                                                                                               \
		if (!g_isQuiet) {                                                                                        \
			if (g_toSysLog) {                                                                                \
				syslog(LOG_NOTICE, "[FBInk] " fmt, ##__VA_ARGS__);                                       \
			} else {                                                                                         \
				fprintf(stderr, "[FBInk] " fmt "\n", ##__VA_ARGS__);                                     \
			}                                                                                                \
		}                                                                                                        \
	})

// And a simple wrapper for actual warnings on error codepaths. Should only be used for warnings before a return/exit.
// Always shown, always tagged, and always ends with a bang.
#define WARN(fmt, ...)                                                                                                   \
	({                                                                                                               \
		if (g_toSysLog) {                                                                                        \
			syslog(LOG_ERR, "[FBInk] " fmt "!", ##__VA_ARGS__);                                              \
		} else {                                                                                                 \
			fprintf(stderr, "[FBInk] " fmt "!\n", ##__VA_ARGS__);                                            \
		}                                                                                                        \
	})

// Same, but with __PRETTY_FUNCTION__ right before fmt
#define PFWARN(fmt, ...) ({ WARN("[%s] " fmt, __PRETTY_FUNCTION__, ##__VA_ARGS__); })

// We want to return negative values on failure, always
#define ERRCODE(e) (-(e))

// eInk color map
// c.f., linux/drivers/video/mxc/cmap_lab126.h
// NOTE: Legacy devices have an inverted color map, which we handle internally!
//       Incidentally, this is partly why we have two LUTs for the Gray ramp ;).
static const uint8_t eInkFGCMap[16] = { 0x00, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77,
					0x88, 0x99, 0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xFF };
static const uint8_t eInkBGCMap[16] = { 0xFF, 0xEE, 0xDD, 0xCC, 0xBB, 0xAA, 0x99, 0x88,
					0x77, 0x66, 0x55, 0x44, 0x33, 0x22, 0x11, 0x00 };

// NOTE: And this one has a crappy Y8 -> RGB565 LUT, which, as I was mostly expecting,
//       is not noticeably faster than pack_rgb565() in our use-cases ;).
//#include "fbink_rgb565.h"

// Global variables to store fb/screen info
unsigned char* restrict  fbPtr      = NULL;
bool                     isFbMapped = false;
struct fb_var_screeninfo vInfo;
struct fb_fix_screeninfo fInfo;
uint32_t                 viewWidth;
uint32_t                 viewHeight;
uint32_t                 screenWidth;
uint32_t                 screenHeight;
uint8_t                  viewHoriOrigin = 0U;
uint8_t                  viewVertOrigin = 0U;
uint8_t                  viewVertOffset = 0U;
uint8_t                  glyphWidth     = 8U;
uint8_t                  glyphHeight    = 8U;
unsigned short int       FONTW          = 8U;
unsigned short int       FONTH          = 8U;
uint8_t                  FONTSIZE_MULT  = 1U;
uint8_t                  penFGColor     = 0x00;
uint8_t                  penBGColor     = 0xFF;
FBInkPixel               penFGPixel;
FBInkPixel               penBGPixel;
uint32_t                 lastMarker = 0U;
// Slightly arbitrary-ish fallback values
unsigned short int MAXROWS = 45U;
unsigned short int MAXCOLS = 32U;
// Verbose is for diagnostic/debug info in general
bool g_isVerbose = false;
// Quiet is for fbink_init's hardware setup info
bool g_isQuiet = false;
// Whether we log to stdout/stderr or the syslog
bool g_toSysLog = false;
// This should be a pretty accurate fallback...
long int USER_HZ = 100;
// Pointers to the appropriate put_pixel/get_pixel functions for the fb's bpp
//void (*fxpPutPixel)(const FBInkCoordinates* restrict, const FBInkPixel* restrict) = NULL;
void (*fxpGetPixel)(const FBInkCoordinates* restrict, FBInkPixel* restrict) = NULL;
void (*fxpFillRect)(unsigned short int,
		    unsigned short int,
		    unsigned short int,
		    unsigned short int,
		    const FBInkPixel* restrict)                             = NULL;
void (*fxpFillRectChecked)(unsigned short int,
			   unsigned short int,
			   unsigned short int,
			   unsigned short int,
			   const FBInkPixel* restrict)                      = NULL;
// As well as the appropriate coordinates rotation functions...
void (*fxpRotateCoords)(FBInkCoordinates* restrict)  = NULL;
void (*fxpRotateRegion)(struct mxcfb_rect* restrict) = NULL;
// And the font bitmap getter...
const unsigned char* (*fxpFont8xGetBitmap)(uint32_t) = NULL;
#ifdef FBINK_WITH_FONTS
const uint16_t* (*fxpFont16xGetBitmap)(uint32_t) = NULL;
const uint32_t* (*fxpFont32xGetBitmap)(uint32_t) = NULL;
//const uint64_t* (*fxpFont64xGetBitmap)(uint32_t) = NULL;
#endif

// Where we track device/screen-specific quirks
FBInkDeviceQuirks deviceQuirks = { 0 };

// Where we track the last drawn rectangle
FBInkRect lastRect = { 0 };

#ifdef FBINK_WITH_OPENTYPE
// Information about the currently loaded OpenType font
bool         otInit  = false;
FBInkOTFonts otFonts = { NULL, NULL, NULL, NULL };
#endif

#if defined(FBINK_FOR_KOBO) || defined(FBINK_FOR_CERVANTES) || defined(FBINK_FOR_POCKETBOOK)
static void rotate_coordinates_pickel(FBInkCoordinates* restrict);
#endif
#if defined(FBINK_FOR_KOBO) || defined(FBINK_FOR_CERVANTES)
static void rotate_coordinates_boot(FBInkCoordinates* restrict);
#	ifdef FBINK_WITH_BUTTON_SCAN
static void rotate_touch_coordinates(FBInkCoordinates* restrict);
#	endif
#endif
static void rotate_coordinates_nop(FBInkCoordinates* restrict __attribute__((unused)));

// NOTE: Making sure all of those are inlined helps fbink_print_ot (c.f., #43).
static inline __attribute__((always_inline, hot)) uint16_t pack_rgb565(uint8_t, uint8_t, uint8_t);

static inline __attribute__((always_inline, hot)) void put_pixel_Gray4(const FBInkCoordinates* restrict,
								       const FBInkPixel*       restrict);
static inline __attribute__((always_inline, hot)) void put_pixel_Gray8(const FBInkCoordinates* restrict,
								       const FBInkPixel*       restrict);
static inline __attribute__((always_inline)) void      put_pixel_RGB24(const FBInkCoordinates* restrict,
								       const FBInkPixel*       restrict);
static inline __attribute__((always_inline, hot)) void put_pixel_RGB32(const FBInkCoordinates* restrict,
								       const FBInkPixel*       restrict);
static inline __attribute__((always_inline, hot)) void put_pixel_RGB565(const FBInkCoordinates* restrict,
									const FBInkPixel*       restrict);
// NOTE: We pass coordinates by value here, because a rotation transformation *may* be applied to them,
//       and that's a rotation that the caller will *never* care about.
static inline __attribute__((always_inline, hot)) void put_pixel(FBInkCoordinates, const FBInkPixel* restrict, bool);
// NOTE: On the other hand, if you happen to be calling function pointers directly,
//       it's left to you to not do anything stupid ;)

static inline __attribute__((always_inline, hot)) void get_pixel_Gray4(const FBInkCoordinates* restrict,
								       FBInkPixel*             restrict);
static inline __attribute__((always_inline, hot)) void get_pixel_Gray8(const FBInkCoordinates* restrict,
								       FBInkPixel*             restrict);
static inline __attribute__((always_inline)) void get_pixel_RGB24(const FBInkCoordinates* restrict, FBInkPixel* restrict);
static inline __attribute__((always_inline, hot)) void get_pixel_RGB32(const FBInkCoordinates* restrict,
								       FBInkPixel*             restrict);
static inline __attribute__((always_inline, hot)) void get_pixel_RGB565(const FBInkCoordinates* restrict,
									FBInkPixel*             restrict);
// NOTE: Same as put_pixel ;)
static inline __attribute__((always_inline, hot)) void get_pixel(FBInkCoordinates, FBInkPixel* restrict);

#if defined(FBINK_WITH_IMAGE) || defined(FBINK_WITH_OPENTYPE)
// This is only needed for alpha blending in the image or OpenType codepath ;).
// c.f., https://github.com/videolan/vlc/blob/6b96ade7dd97acb49303a0a9da9b3d2056b808e0/modules/video_filter/blend.cpp#L49
//     & https://github.com/koreader/koreader-base/blob/b3e72affd0e1ba819d92194b229468452c58836f/blitbuffer.c#L59
//     & https://github.com/python-pillow/Pillow/blob/master/src/libImaging/ImagingUtils.h
// NOTE: May not actually be faster on x86_64, where most compilers will do better with a simple (v / 255U),
//       but on ARM, which is what we mainly care about, this yields better results... ;).
// NOTE: See also SDL2 & pixman for some fairly hardcore blitting trickery ;).
#	define DIV255(V)                                                                                                \
		({                                                                                                       \
			__auto_type _v = (V) + 128;                                                                      \
			(((_v >> 8U) + _v) >> 8U);                                                                       \
		})
// NOTE: On the other hand, for mul, every decent compiler seems to figure that one on its own, regardless of architecture ;).
//       (... most of the time, i.e., when it's on its own).
#	define MUL255(V)                                                                                                \
		({                                                                                                       \
			__auto_type v__ = (V);                                                                           \
			((v__ << 8U) - v__);                                                                             \
		})
#endif

// NOTE: Enforced inlining on fill_rect currently doesn't gain us anything, on the other hand.
//       Which is why we went with a function pointer to bitdepth-specific branchless variants ;).
static __attribute__((hot)) void fill_rect_Gray4(unsigned short int,
						 unsigned short int,
						 unsigned short int,
						 unsigned short int,
						 const FBInkPixel* restrict);
static __attribute__((hot)) void fill_rect_Gray4_checked(unsigned short int,
							 unsigned short int,
							 unsigned short int,
							 unsigned short int,
							 const FBInkPixel* restrict);
static __attribute__((hot)) void fill_rect_Gray8(unsigned short int,
						 unsigned short int,
						 unsigned short int,
						 unsigned short int,
						 const FBInkPixel* restrict);
static __attribute__((hot)) void fill_rect_Gray8_checked(unsigned short int,
							 unsigned short int,
							 unsigned short int,
							 unsigned short int,
							 const FBInkPixel* restrict);
static __attribute__((hot)) void fill_rect_RGB565(unsigned short int,
						  unsigned short int,
						  unsigned short int,
						  unsigned short int,
						  const FBInkPixel* restrict);
static __attribute__((hot)) void fill_rect_RGB565_checked(unsigned short int,
							  unsigned short int,
							  unsigned short int,
							  unsigned short int,
							  const FBInkPixel* restrict);
static void                      fill_rect_RGB24(unsigned short int,
						 unsigned short int,
						 unsigned short int,
						 unsigned short int,
						 const FBInkPixel* restrict);
static void                      fill_rect_RGB24_checked(unsigned short int,
							 unsigned short int,
							 unsigned short int,
							 unsigned short int,
							 const FBInkPixel* restrict);
static __attribute__((hot)) void fill_rect_RGB32(unsigned short int,
						 unsigned short int,
						 unsigned short int,
						 unsigned short int,
						 const FBInkPixel* restrict);
static __attribute__((hot)) void fill_rect_RGB32_checked(unsigned short int,
							 unsigned short int,
							 unsigned short int,
							 unsigned short int,
							 const FBInkPixel* restrict);
static void                      clear_screen(int UNUSED_BY_NOTKINDLE, uint8_t, bool UNUSED_BY_NOTKINDLE);
//static void checkerboard_screen(void);

static const unsigned char* font8x8_get_bitmap(uint32_t);

static __attribute__((cold)) const char* fontname_to_string(uint8_t);

static int zu_print_length(size_t);

static __attribute__((hot)) struct mxcfb_rect draw(const char* restrict,
						   unsigned short int,
						   unsigned short int,
						   unsigned short int,
						   bool,
						   const FBInkConfig* restrict);

#ifndef FBINK_FOR_LINUX
static __attribute__((cold)) long int jiffies_to_ms(long int);
#	if defined(FBINK_FOR_KINDLE)
static int refresh_legacy(int, const struct mxcfb_rect, bool);
static int wait_for_submission_kindle(int, uint32_t);
static int refresh_kindle(int, const struct mxcfb_rect, uint32_t, uint32_t, int, bool, uint32_t);
static int wait_for_complete_kindle_pearl(int, uint32_t);
static int wait_for_complete_kindle(int, uint32_t);
static int refresh_kindle_zelda(int, const struct mxcfb_rect, uint32_t, uint32_t, int, bool, uint32_t);
static int refresh_kindle_rex(int, const struct mxcfb_rect, uint32_t, uint32_t, int, bool, uint32_t);
#	elif defined(FBINK_FOR_CERVANTES)
static int refresh_cervantes(int, const struct mxcfb_rect, uint32_t, uint32_t, int, bool, uint32_t);
static int wait_for_complete_cervantes(int, uint32_t);
#	elif defined(FBINK_FOR_REMARKABLE)
static int refresh_remarkable(int, const struct mxcfb_rect, uint32_t, uint32_t, int, bool, uint32_t);
static int wait_for_complete_remarkable(int, uint32_t);
#	elif defined(FBINK_FOR_POCKETBOOK)
static int refresh_pocketbook(int, const struct mxcfb_rect, uint32_t, uint32_t, int, bool, uint32_t);
static int wait_for_complete_pocketbook(int, uint32_t);
#	elif defined(FBINK_FOR_KOBO)
static int refresh_kobo(int, const struct mxcfb_rect, uint32_t, uint32_t, int, bool, uint32_t);
static int wait_for_complete_kobo(int, uint32_t);
static int refresh_kobo_mk7(int, const struct mxcfb_rect, uint32_t, uint32_t, int, bool, uint32_t);
static int wait_for_complete_kobo_mk7(int, uint32_t);
#	endif    // FBINK_FOR_KINDLE
#endif    // !FBINK_FOR_LINUX
static int refresh(int, const struct mxcfb_rect, uint32_t, int UNUSED_BY_CERVANTES, bool, bool, bool);
#ifndef FBINK_FOR_LINUX
#	if defined(FBINK_FOR_KINDLE)
static int wait_for_submission(int, uint32_t);
#	endif
static int wait_for_complete(int, uint32_t);
#endif

static int open_fb_fd(int* restrict, bool* restrict);
static int open_fb_fd_nonblock(int* restrict, bool* restrict);

static const char* fb_rotate_to_string(uint32_t);
#ifdef FBINK_FOR_KINDLE
static const char* einkfb_orientation_to_string(orientation_t);
#endif
static int  set_pen_color(bool, bool, bool, bool, uint8_t, uint8_t, uint8_t, uint8_t);
static int  update_pen_colors(const FBInkConfig* restrict);
static void update_verbosity(const FBInkConfig* restrict);
#ifdef FBINK_FOR_POCKETBOOK
static void pocketbook_fix_fb_info(void);
#endif
static int initialize_fbink(int, const FBInkConfig* restrict, bool);

static int memmap_fb(int);
static int unmap_fb(void);

#if defined(FBINK_FOR_KOBO) || defined(FBINK_FOR_CERVANTES) || defined(FBINK_FOR_POCKETBOOK)
static void rotate_region_pickel(struct mxcfb_rect* restrict);
#endif
#if defined(FBINK_FOR_KOBO) || defined(FBINK_FOR_CERVANTES)
static void rotate_region_boot(struct mxcfb_rect* restrict);
#endif
static void rotate_region_nop(struct mxcfb_rect* restrict);
static void fullscreen_region(struct mxcfb_rect* restrict);

static int grid_to_region(int, unsigned short int, unsigned short int, bool, const FBInkConfig* restrict);

static void set_last_rect(const struct mxcfb_rect* restrict);

int draw_progress_bars(int, bool, uint8_t, const FBInkConfig* restrict);

#ifdef FBINK_WITH_IMAGE
unsigned char* qSmoothScaleImage(const unsigned char* src, int sw, int sh, int sn, bool ignore_alpha, int dw, int dh);

static unsigned char* img_load_from_file(const char*, int*, int*, int*, int);
static unsigned char* img_convert_px_format(const unsigned char*, int, int, int, int);
static uint8_t        dither_o8x8(unsigned short int, unsigned short int, uint8_t);
static int            draw_image(int,
				 const unsigned char* restrict,
				 const int,
				 const int,
				 const int,
				 const int,
				 short int,
				 short int,
				 const FBInkConfig* restrict);
#endif

#ifdef FBINK_WITH_OPENTYPE
static const char* font_style_to_string(FONT_STYLE_T);
static int         add_ot_font(const char*, FONT_STYLE_T, FBInkOTFonts* restrict);
static int         free_ot_font(stbtt_fontinfo** restrict);
static int         free_ot_fonts(FBInkOTFonts* restrict);
static void        parse_simple_md(const char* restrict, size_t, unsigned char* restrict);
static const char* glyph_style_to_string(CHARACTER_FONT_T);
#endif

static uint32_t    get_wfm_mode(uint8_t);
static const char* wfm_to_string(uint8_t);
#ifndef FBINK_FOR_LINUX
#	ifdef FBINK_FOR_KINDLE
static const char* kindle_wfm_to_string(uint32_t);
static const char* kindle_zelda_wfm_to_string(uint32_t);
#	endif
#	if defined(FBINK_FOR_KOBO) || defined(FBINK_FOR_CERVANTES)
static const char* ntx_wfm_to_string(uint32_t);
#	endif
#	ifdef FBINK_FOR_REMARKABLE
static const char* remarkable_wfm_to_string(uint32_t);
#	endif
#	ifdef FBINK_FOR_POCKETBOOK
static const char* pocketbook_wfm_to_string(uint32_t);
#	endif
#endif
static int         get_hwd_mode(uint8_t);
static const char* hwd_to_string(uint8_t);

// For identify_device, which we need outside of fbink_device_id.c ;)
#ifndef FBINK_FOR_LINUX
#	include "fbink_device_id.h"
#endif

#endif
