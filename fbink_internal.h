/*
	FBInk: FrameBuffer eInker, a tool to print text & images on eInk devices (Kobo/Kindle)
	Copyright (C) 2018-2019 NiLuJe <ninuje@gmail.com>

	Some (8/16/24/32bpp) Linux framebuffer writing routines based on: fbtestfnt.c & fbtest6.c, from
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

#ifndef __FBINK_INTERNAL_H
#define __FBINK_INTERNAL_H

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
#	ifndef FBINK_FOR_KINDLE
#		ifndef FBINK_FOR_CERVANTES
#			ifndef FBINK_WITH_BUTTON_SCAN
#				define FBINK_WITH_BUTTON_SCAN
#			endif
#		endif
#	endif
#endif

// Try to use GCC's iceilf builtin if possible...
// NOTE: Relies on the fact that:
//       * Clang implements the __has_builtin macro, but currently not the __builtin_iceilf function
//       * GCC implements the __builtin_iceilf function, but not the __has_builtin macro
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
#			define iceilf(x) ((int) (__builtin_ceilf(x)))
#			define STBTT_iceil(x) ((int) (__builtin_ceilf(x)))
#		endif
#		if __has_builtin(__builtin_lroundf)
#			define iroundf(x) ((int) (__builtin_lroundf(x)))
#		endif
#		if __has_builtin(__builtin_floorf)
#			define ifloorf(x) ((int) (__builtin_floorf(x)))
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
#			define iceilf(x) __builtin_iceilf(x)
#			define iroundf(x) __builtin_iroundf(x)
#			define ifloorf(x) __builtin_ifloorf(x)
#			define STBTT_floorf(x) __builtin_floorf(x)
#			define STBTT_ifloor(x) __builtin_ifloorf(x)
#			define STBTT_iceil(x) __builtin_iceilf(x)
#			define STBTT_sqrt(x) __builtin_sqrtf(x)
#			define STBTT_pow(x, y) __builtin_powf(x, y)
#			define STBTT_fmod(x, y) __builtin_fmodf(x, y)
// Need to keep double precision for cos, because GCC sneakily replaces a PI approximation in stbtt ;)
#			define STBTT_cos(x) __builtin_cos(x)
#			define STBTT_acos(x) __builtin_acosf(x)
#			define STBTT_fabs(x) __builtin_fabsf(x)
#		endif
#	endif
#endif

#include <errno.h>
#include <fcntl.h>
#include <linux/fb.h>
#include <linux/kd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>
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
#else
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
#	include "fbink_misc_fonts.h"
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
#	define FBINK_VERSION "v1.10.3"
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

// NOTE: Some of our ifdef combinations may cause a small number of function arguments to become unused...
//       Handle the warnings in these cases with a bit of trickery,
//       by conditionally marking theses arguments as unused ;).
#ifdef FBINK_MINIMAL
#	define UNUSED_BY_MINIMAL __attribute__((unused))
#else
#	define UNUSED_BY_MINIMAL
#endif
#ifndef FBINK_WITH_BUTTON_SCAN
#	define UNUSED_BY_NOBUTTON __attribute__((unused))
#else
#	define UNUSED_BY_NOBUTTON
#endif
#ifndef FBINK_FOR_KINDLE
#	define UNUSED_BY_NOTKINDLE __attribute__((unused))
#	define UNUSED_BY_KINDLE
#else
#	define UNUSED_BY_NOTKINDLE
#	define UNUSED_BY_KINDLE __attribute__((unused))
#endif
#ifdef FBINK_FOR_CERVANTES
#	define UNUSED_BY_CERVANTES __attribute__((unused))
#else
#	define UNUSED_BY_CERVANTES
#endif

// Handle what we send to stdout (i.e., mostly diagnostic stuff, which tends to be verbose, so no FBInk tag)
#define LOG(fmt, ...)                                                                                                    \
	({                                                                                                               \
		if (g_isVerbose) {                                                                                       \
			fprintf(stdout, fmt "\n", ##__VA_ARGS__);                                                        \
		}                                                                                                        \
	})

// And then what we send to stderr (mostly fbink_init stuff, add an FBInk tag to make it clear where it comes from for API users)
#define ELOG(fmt, ...)                                                                                                   \
	({                                                                                                               \
		if (!g_isQuiet) {                                                                                        \
			fprintf(stderr, "[FBInk] " fmt "\n", ##__VA_ARGS__);                                             \
		}                                                                                                        \
	})

// And a simple wrapper for actual warnings on error codepaths. Should only be used for warnings before a return/exit.
// Always shown, always tagged, and always ends with a bang.
#define WARN(fmt, ...) ({ fprintf(stderr, "[FBInk] " fmt "!\n", ##__VA_ARGS__); })

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

// Global variables to store fb/screen info
unsigned char*           fbPtr      = NULL;
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
// Slightly arbitrary-ish fallback values
unsigned short int MAXROWS = 45U;
unsigned short int MAXCOLS = 32U;
// Verbose is for diagnostic/debug info in general
bool g_isVerbose = false;
// Quiet is for fbink_init's hardware setup info
bool g_isQuiet = false;
// This should be a pretty accurate fallback...
long int USER_HZ = 100;
// Pointers to the appropriate put_pixel/get_pixel functions for the fb's bpp
void (*fxpPutPixel)(const FBInkCoordinates*, const FBInkColor*) = NULL;
void (*fxpGetPixel)(const FBInkCoordinates*, FBInkColor*)       = NULL;
// As well as the appropriate coordinates rotation functions...
void (*fxpRotateCoords)(FBInkCoordinates*)  = NULL;
void (*fxpRotateRegion)(struct mxcfb_rect*) = NULL;
// And the font bitmap getter...
const unsigned char* (*fxpFont8xGetBitmap)(uint32_t) = NULL;
#ifdef FBINK_WITH_FONTS
const uint16_t* (*fxpFont16xGetBitmap)(uint32_t) = NULL;
const uint32_t* (*fxpFont32xGetBitmap)(uint32_t) = NULL;
//const uint64_t* (*fxpFont64xGetBitmap)(uint32_t) = NULL;
#endif

// Where we track device/screen-specific quirks
FBInkDeviceQuirks deviceQuirks = { 0 };

#ifdef FBINK_WITH_OPENTYPE
// Information about the currently loaded OpenType font
bool         otInit  = false;
FBInkOTFonts otFonts = { NULL, NULL, NULL, NULL };
#endif

#ifndef FBINK_FOR_KINDLE
static void rotate_coordinates_pickel(FBInkCoordinates*);
static void rotate_coordinates_boot(FBInkCoordinates*);
#	ifdef FBINK_WITH_BUTTON_SCAN
static void rotate_touch_coordinates(FBInkCoordinates*);
#	endif
#endif
static void rotate_coordinates_nop(FBInkCoordinates* __attribute__((unused)));

static void put_pixel_Gray4(const FBInkCoordinates*, const FBInkColor*);
static void put_pixel_Gray8(const FBInkCoordinates*, const FBInkColor*);
static void put_pixel_RGB24(const FBInkCoordinates*, const FBInkColor*);
static void put_pixel_RGB32(const FBInkCoordinates*, const FBInkColor*);
static void put_pixel_RGB565(const FBInkCoordinates*, const FBInkColor*);
// NOTE: We pass coordinates by value here, because a rotation transformation *may* be applied to them,
//       and that's a rotation that the caller will *never* care about.
static void put_pixel(FBInkCoordinates, const FBInkColor*);
// NOTE: On the other hand, if you happen to be calling function pointers directly,
//       it's left to you to not do anything stupid ;)

static void get_pixel_Gray4(const FBInkCoordinates*, FBInkColor*);
static void get_pixel_Gray8(const FBInkCoordinates*, FBInkColor*);
static void get_pixel_RGB24(const FBInkCoordinates*, FBInkColor*);
static void get_pixel_RGB32(const FBInkCoordinates*, FBInkColor*);
static void get_pixel_RGB565(const FBInkCoordinates*, FBInkColor*);
// NOTE: Same as put_pixel ;)
static void get_pixel(FBInkCoordinates, FBInkColor*);

#if defined(FBINK_WITH_IMAGE) || defined(FBINK_WITH_OPENTYPE)
// This is only needed for alpha blending in the image or OpenType codepath ;).
// c.f., https://github.com/videolan/vlc/blob/6b96ade7dd97acb49303a0a9da9b3d2056b808e0/modules/video_filter/blend.cpp#L49
//     & https://github.com/koreader/koreader-base/blob/b3e72affd0e1ba819d92194b229468452c58836f/blitbuffer.c#L59
// NOTE: May not actually be faster on x86_64, where most compilers will do better with a simple (v / 255U),
//       but on ARM, which is what we mainly care about, this yields better results... ;).
#	define DIV255(v) (((v >> 8U) + v + 0x01) >> 8U)
// NOTE: On the other hand, for mul, every decent compiler seem to figure that one on its own, regardless of architecture ;).
//       (... most of the time, i.e., when it's on its own).
#	define MUL255(v) ((v << 8U) - v)
#endif

static void fill_rect(unsigned short int, unsigned short int, unsigned short int, unsigned short int, const FBInkColor*);
static void clear_screen(int UNUSED_BY_NOTKINDLE, uint8_t, bool UNUSED_BY_NOTKINDLE);

static const unsigned char* font8x8_get_bitmap(uint32_t);

static const char* fontname_to_string(uint8_t);

static struct mxcfb_rect draw(const char*,
			      unsigned short int,
			      unsigned short int,
			      unsigned short int,
			      bool,
			      const FBInkConfig*);

#ifndef FBINK_FOR_LINUX
static long int jiffies_to_ms(long int);
#	if defined(FBINK_FOR_KINDLE)
static int refresh_legacy(int, const struct mxcfb_rect, bool);
static int refresh_kindle(int, const struct mxcfb_rect, uint32_t, uint32_t, uint32_t);
static int refresh_kindle_koa2(int, const struct mxcfb_rect, uint32_t, uint32_t, int, uint32_t);
static int refresh_kindle_pw4(int, const struct mxcfb_rect, uint32_t, uint32_t, int, uint32_t);
#	elif defined(FBINK_FOR_CERVANTES)
static int refresh_cervantes(int, const struct mxcfb_rect, uint32_t, uint32_t, uint32_t);
#	else
static int refresh_kobo(int, const struct mxcfb_rect, uint32_t, uint32_t, uint32_t);
static int refresh_kobo_mk7(int, const struct mxcfb_rect, uint32_t, uint32_t, int, uint32_t);
#	endif    // FBINK_FOR_KINDLE
#endif            // !FBINK_FOR_LINUX
static int refresh(int, const struct mxcfb_rect, uint32_t, int UNUSED_BY_CERVANTES, bool, bool);

static int open_fb_fd(int*, bool*);

static const char* fb_rotate_to_string(uint32_t);
static int         initialize_fbink(int, const FBInkConfig*, bool);

static int memmap_fb(int);
static int unmap_fb(void);

#ifndef FBINK_FOR_KINDLE
static void rotate_region_pickel(struct mxcfb_rect*);
static void rotate_region_boot(struct mxcfb_rect*);
#endif
static void rotate_region_nop(struct mxcfb_rect*);
static void fullscreen_region(struct mxcfb_rect*);

int draw_progress_bars(int, bool, uint8_t, const FBInkConfig*);

#ifdef FBINK_WITH_IMAGE
static unsigned char* img_load_from_file(const char*, int*, int*, int*, int);
static unsigned char* img_convert_px_format(const unsigned char*, int, int, int, int);
static int            draw_image(int,
				 const unsigned char*,
				 const int,
				 const int,
				 const int,
				 const int,
				 short int,
				 short int,
				 const FBInkConfig*);
#endif

#ifdef FBINK_WITH_OPENTYPE
static void* free_ot_font(stbtt_fontinfo*);
static void  parse_simple_md(const char*, size_t, unsigned char*);
#endif

// For identify_device, which we need outside of fbink_device_id.c ;)
#ifndef FBINK_FOR_LINUX
#	include "fbink_device_id.h"
#endif

#endif
