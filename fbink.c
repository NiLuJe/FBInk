/*
	FBInk: FrameBuffer eInker, a library to print text & images to an eInk Linux framebuffer
	Copyright (C) 2018-2023 NiLuJe <ninuje@gmail.com>
	SPDX-License-Identifier: GPL-3.0-or-later

	----

	Some (the 8/16/24/32bpp put_pixel_* variants) Linux framebuffer writing routines based on: fbtestfnt.c & fbtest6.c, from
	https://raspberrycompote.blogspot.com/2014/04/low-level-graphics-on-raspberry-pi-text.html &
	https://raspberrycompote.blogspot.com/2013/03/low-level-graphics-on-raspberry-pi-part_8.html
	Original works by J-P Rosti (a.k.a -rst- and 'Raspberry Compote'),
	Licensed under the Creative Commons Attribution 3.0 Unported License
	(http://creativecommons.org/licenses/by/3.0/deed.en_US)

	Ordered dithering routine (dither_o8x8) completely based on ImageMagick's implementation,
	(OrderedDitherImage @ https://github.com/ImageMagick/ImageMagick/blob/master/MagickCore/threshold.c),
	Copyright 1999-2019 ImageMagick Studio LLC,
	Licensed under the ImageMagick License,
	(https://imagemagick.org/script/license.php)

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

#include "fbink.h"

#include "fbink_internal.h"

#ifdef FBINK_WITH_IMAGE
#	define STB_IMAGE_IMPLEMENTATION
// Make it private, we don't need it anywhere else
#	define STB_IMAGE_STATIC
// Disable HDR, as well as the linear light API, to avoid pulling in libm
#	define STBI_NO_HDR
#	define STBI_NO_LINEAR
// We want SIMD for JPEG decoding (... if we can actually use it)!
// It's not the end of the world if we can't, the speed gains are minimal (~5%).
#	ifdef __ARM_NEON
#		define STBI_NEON
#	endif
// We don't care about those formats (PhotoShop, AutoDesk)
#	define STBI_NO_PSD
#	define STBI_NO_PIC
// We can't use stbi_failure_reason as it's not thread-safe, so ditch the strings
#	define STBI_NO_FAILURE_STRINGS
// Prevent attempting to decode ginormous images
#	define STBI_MAX_DIMENSIONS (1 << 13)
// Disable a bunch of very verbose but mostly harmless warnings
#	pragma GCC diagnostic   push
#	pragma GCC diagnostic   ignored "-Wunknown-pragmas"
#	pragma clang diagnostic ignored "-Wunknown-warning-option"
#	pragma GCC diagnostic   ignored "-Wcast-qual"
#	pragma GCC diagnostic   ignored "-Wcast-align"
#	pragma GCC diagnostic   ignored "-Wconversion"
#	pragma GCC diagnostic   ignored "-Wsign-conversion"
#	pragma GCC diagnostic   ignored "-Wduplicated-branches"
#	pragma GCC diagnostic   ignored "-Wunused-function"
#	pragma GCC diagnostic   ignored "-Wsign-compare"
#	pragma GCC diagnostic   ignored "-Wunused-but-set-variable"
#	pragma GCC diagnostic   ignored "-Wsuggest-attribute=pure"
#	include "stb/stb_image.h"
#	pragma GCC diagnostic pop
#endif

#ifdef FBINK_WITH_OPENTYPE
#	define STB_TRUETYPE_IMPLEMENTATION
// Make it private, we don't need it anywhere else
#	define STBTT_STATIC
// stb_truetype is.... noisy
#	pragma GCC diagnostic   push
#	pragma GCC diagnostic   ignored "-Wunknown-pragmas"
#	pragma clang diagnostic ignored "-Wunknown-warning-option"
#	pragma GCC diagnostic   ignored "-Wcast-qual"
#	pragma GCC diagnostic   ignored "-Wconversion"
#	pragma GCC diagnostic   ignored "-Wsign-conversion"
#	pragma GCC diagnostic   ignored "-Wunused-function"
#	pragma GCC diagnostic   ignored "-Wsuggest-attribute=pure"
#	include "stb/stb_truetype.h"
#	pragma GCC diagnostic pop
#endif

// Return the library version as devised at library compile-time
const char*
    fbink_version(void)
{
	return FBINK_VERSION;
}

// Return the target platform of the current library build
FBINK_TARGET_T
fbink_target(void)
{
#if defined(FBINK_FOR_LINUX)
	return FBINK_TARGET_LINUX;
#elif defined(FBINK_FOR_KOBO)
	return FBINK_TARGET_KOBO;
#elif defined(FBINK_FOR_KINDLE)
	return FBINK_TARGET_KINDLE;
#elif defined(FBINK_FOR_LEGACY)
	return FBINK_TARGET_KINDLE_LEGACY;
#elif defined(FBINK_FOR_CERVANTES)
	return FBINK_TARGET_CERVANTES;
#elif defined(FBINK_FOR_REMARKABLE)
	return FBINK_TARGET_REMARKABLE;
#elif defined(FBINK_FOR_POCKETBOOK)
	return FBINK_TARGET_POCKETBOOK;
#else
	// Unreachable
	return FBINK_TARGET_MAX;
#endif
}

// Return the feature set of the current library build
uint32_t
    fbink_features(void)
{
#ifndef FBINK_MINIMAL
	uint32_t features = FBINK_FEATURE_FULL;
#else
	uint32_t features = FBINK_FEATURE_MINIMAL;
#	ifdef FBINK_WITH_DRAW
	features |= FBINK_FEATURE_DRAW;
#	endif
#	ifdef FBINK_WITH_BITMAP
	features |= FBINK_FEATURE_BITMAP;
#	endif
#	ifdef FBINK_WITH_FONTS
	features |= FBINK_FEATURE_FONTS;
#	endif
#	ifdef FBINK_WITH_UNIFONT
	features |= FBINK_FEATURE_UNIFONT;
#	endif
#	ifdef FBINK_WITH_OPENTYPE
	features |= FBINK_FEATURE_OPENTYPE;
#	endif
#	ifdef FBINK_WITH_IMAGE
	features |= FBINK_FEATURE_IMAGE;
#	endif
#	ifdef FBINK_WITH_BUTTON_SCAN
	features |= FBINK_FEATURE_BUTTON_SCAN;
#	endif
#endif    // !FBINK_MINIMAL
	return features;
}

#ifdef FBINK_WITH_DRAW
// #RGB -> RGB565
static inline __attribute__((always_inline, hot)) uint16_t
    pack_rgb565(uint8_t r, uint8_t g, uint8_t b)
{
	// ((r / 8) * 2048) + ((g / 4) * 32) + (b / 8);
	return (uint16_t) (((r >> 3U) << 11U) | ((g >> 2U) << 5U) | (b >> 3U));
}

// Helper functions to 'plot' a specific pixel in a given color to the framebuffer
static inline __attribute__((always_inline, hot)) void
    put_pixel_Gray4(const FBInkCoordinates* restrict coords, const FBInkPixel* restrict px)
{
	// calculate the pixel's byte offset inside the buffer
	// note: x / 2 as every byte holds 2 pixels
	const size_t pix_offset = (coords->x >> 1U) + (coords->y * fInfo.line_length);

	// NOTE: Squash 8bpp to 4bpp:
	// (v >> 4)
	// or: v * 16 / 256

	// First, we'll need the current full byte to make sure we never clobber a nibble...
	const uint8_t b = *((unsigned char*) (fbPtr + pix_offset));

	// We can't address nibbles directly, so this takes some shenanigans...
	if ((coords->x & 0x01u) == 0U) {
		// Even pixel: high nibble
		// ORed to avoid clobbering our odd pixel
		*((unsigned char*) (fbPtr + pix_offset)) = (unsigned char) ((b & 0x0Fu) | (px->gray8 & 0xF0u));
		// Squash to 4bpp, and write to the top/left nibble
		// or: ((v >> 4) << 4)
	} else {
		// Odd pixel: low nibble
		// ORed to avoid clobbering our even pixel
		*((unsigned char*) (fbPtr + pix_offset)) = (unsigned char) ((b & 0xF0u) | (px->gray8 >> 4U));
	}
}

static inline __attribute__((always_inline, hot)) void
    put_pixel_Gray8(const FBInkCoordinates* restrict coords, const FBInkPixel* restrict px)
{
	// calculate the pixel's byte offset inside the buffer
	const size_t pix_offset = coords->x + (coords->y * fInfo.line_length);

	// now this is about the same as 'fbp[pix_offset] = value'
	*((unsigned char*) (fbPtr + pix_offset)) = px->gray8;
}

static inline __attribute__((always_inline)) void
    put_pixel_RGB24(const FBInkCoordinates* restrict coords, const FBInkPixel* restrict px)
{
	// calculate the pixel's byte offset inside the buffer
	// note: x * 3 as every pixel is 3 consecutive bytes
	const size_t pix_offset = (coords->x * 3U) + (coords->y * fInfo.line_length);

	// now this is about the same as 'fbp[pix_offset] = value'
	// NOTE: Technically legitimate warning. In practice, we always pass RGB32 pixels in 24bpp codepaths.
#	pragma GCC diagnostic   push
#	pragma GCC diagnostic   ignored "-Wunknown-pragmas"
#	pragma clang diagnostic ignored "-Wunknown-warning-option"
#	pragma GCC diagnostic   ignored "-Wmaybe-uninitialized"
	*((unsigned char*) (fbPtr + pix_offset))      = px->bgra.color.b;
	*((unsigned char*) (fbPtr + pix_offset + 1U)) = px->bgra.color.g;
	*((unsigned char*) (fbPtr + pix_offset + 2U)) = px->bgra.color.r;
#	pragma GCC diagnostic pop
}

static inline __attribute__((always_inline, hot)) void
    put_pixel_RGB32(const FBInkCoordinates* restrict coords, const FBInkPixel* restrict px)
{
	// calculate the scanline's byte offset inside the buffer
	const size_t scanline_offset = (size_t) coords->y * fInfo.line_length;

	// write the four bytes at once
	// NOTE: We rely on pointer arithmetic rules to handle the pixel offset inside the scanline,
	//       i.e., if we add x *after* the cast, that's an addition of x uint32_t elements, meaning x times 4 bytes,
	//       which is exactly what we want ;).
#	pragma GCC diagnostic push
#	pragma GCC diagnostic ignored "-Wcast-align"
	*((uint32_t*) (fbPtr + scanline_offset) + coords->x) = px->bgra.p;
#	pragma GCC diagnostic pop
}

static inline __attribute__((always_inline, hot)) void
    put_pixel_RGB565(const FBInkCoordinates* restrict coords, const FBInkPixel* restrict px)
{
	// calculate the scanline's byte offset inside the buffer
	const size_t scanline_offset = (size_t) coords->y * fInfo.line_length;

	// write the two bytes at once, much to GCC's dismay...
	// NOTE: Input pixel *has* to be properly packed to RGB565 first (via pack_rgb565, c.f., put_pixel)!
#	pragma GCC diagnostic push
#	pragma GCC diagnostic ignored "-Wcast-align"
	*((uint16_t*) (fbPtr + scanline_offset) + coords->x) = px->rgb565;
#	pragma GCC diagnostic pop
}
#endif    // FBINK_WITH_DRAW

#if defined(FBINK_FOR_KOBO) || defined(FBINK_FOR_CERVANTES) || defined(FBINK_FOR_POCKETBOOK)
// Handle rotation quirks...
static void
    rotate_coordinates_pickel(FBInkCoordinates* restrict coords)
{
	// Rotate the coordinates to account for pickel's rotation...
	const unsigned short int rx = coords->y;
	const unsigned short int ry = (unsigned short int) (screenWidth - coords->x - 1);

// NOTE: This codepath is not production ready, it was just an experiment to wrap my head around framebuffer rotation...
//       In particular, only CW has been actually confirmed to behave properly (to handle the isNTX16bLandscape quirk),
//       and region rotation is NOT handled properly/at all.
//       TL;DR: This is for documentation purposes only, never build w/ MATHS defined ;).
#	ifdef FBINK_WITH_MATHS_ROTA
	uint8_t rotation = FB_ROTATE_CW;
	// i.e., θ (c.f., https://en.wikipedia.org/wiki/Cartesian_coordinate_system#Rotation)
	double  rangle   = ((rotation * 90) * M_PI / 180.0);
	double  fxp      = coords->x * cos(rangle) - coords->y * sin(rangle);
	double  fyp      = coords->x * sin(rangle) + coords->y * cos(rangle);
	LOG("(fxp, fyp) -> (%f, %f)", fxp, fyp);
	unsigned short int xp;
	unsigned short int yp;
	switch (rotation) {
		case FB_ROTATE_CW:
			xp = (unsigned short int) lround(-fxp);
			yp = (unsigned short int) lround(vInfo.yres - 1 - fyp);
			break;
		case FB_ROTATE_UD:
			// NOTE: IIRC, this pretty much ends up with (x', y') being equal to (y, x).
			xp = (unsigned short int) lround(-fyp);
			yp = (unsigned short int) lround(-fxp);
			break;
		case FB_ROTATE_CCW:
			xp = (unsigned short int) lround(vInfo.xres - 1 - fxp);
			yp = (unsigned short int) lround(-fyp);
			break;
		default:
			xp = (unsigned short int) lround(fxp);
			yp = (unsigned short int) lround(fyp);
			break;
	}

	LOG("(x, y) -> (%hu, %hu) vs. (rx, ry) -> (%hu, %hu) vs. (x', y') -> (%hu, %hu)",
	    coords->x,
	    coords->y,
	    rx,
	    ry,
	    xp,
	    yp);

	coords->x = xp;
	coords->y = yp;
#	else
	coords->x = rx;
	coords->y = ry;
#	endif
}
#endif    // FBINK_FOR_KOBO || FBINK_FOR_CERVANTES || FBINK_FOR_POCKETBOOK

#if defined(FBINK_FOR_KOBO) || defined(FBINK_FOR_CERVANTES)
static void
    rotate_coordinates_boot(FBInkCoordinates* restrict coords)
{
	// Rotate the coordinates to account for the native boot rotation...
	// NOTE: See the note is fbink_init, this is based on a replicated boot modeset,
	//       which apparently doesn't exactly match the *real* boot modeset... -_-".
	const unsigned short int rx = (unsigned short int) (screenHeight - coords->y - 1);
	const unsigned short int ry = coords->x;

	coords->x = rx;
	coords->y = ry;
}

#	ifdef FBINK_WITH_BUTTON_SCAN
// NOTE: Do *not* trust this to do the right thing, see utils/finger_trace.c instead!
//       This is basically left as-is for archeological purposes only,
//       the only caller is in button_scan, which was kind of a crazy experiment to begin with ;).
static void
    rotate_touch_coordinates(FBInkCoordinates* restrict coords)
{
	unsigned short int rx = coords->x;
	unsigned short int ry = coords->y;

	uint32_t rotation = vInfo.rotate;
	// NOTE: Try to take into account the various rotation quirks, depending on the device...
	//       c.f., mxc_epdc_fb_check_var @ drivers/video/mxc/mxc_epdc_fb.c OR drivers/video/fbdev/mxc/mxc_epdc_v2_fb.c
	if (deviceQuirks.ntxRotaQuirk == NTX_ROTA_ODD_INVERTED) {
		// On the Forma, only Portrait orientations are inverted...
		// When I say Portrait/Landscape, that's how the device *looks*, which doesn't match the FB_ROTATE_* constants...
		// i.e., in Nickel, *visually*, UR is 3, CW is 2, UD is 1, CCW is 0,
		// and when sending ioctls, UR returns 0 (match), CW returns 3 (^= 2), UD returns 2 (match), CCW returns 1 (^= 2).
		if (rotation & 0x01u) {
			// Rotation constant is odd (i.e., CW or CCW), invert it.
			rotation ^= 2U;
		}
		// NOTE: Plato goes with a simple rotation = (4 - rotation) % 4; which does the exact same thing,
		//       I just have a harder time wrapping my head around it ;).
		//       Plus, I'm inclined to believe a simple branch would be faster than a modulo.
	} else if (deviceQuirks.ntxRotaQuirk == NTX_ROTA_ALL_INVERTED) {
		// On *some* devices with a 6.8" panel, *every* orientation is inverted...
		rotation ^= 2U;
	} else if (deviceQuirks.ntxRotaQuirk == NTX_ROTA_SANE) {
		// TODO: This is for the Libra, double-check that it holds up...
		// The reasoning being to try to match the Forma's behavior:
		// UR -> 3 ^ 2 -> CW / CW -> 2 -> UD / UD -> 1 ^ 2 -> CCW / CCW -> 0 -> UR
		// The format being: *effective* orientation (i.e., user-facing) -> actual fb rotate value -> input transform
		// Wich means we essentially want:
		// UR -> CW / CW -> UD / UD -> CCW / CCW -> UR
		// Given the fact that the Libra's panel is finally Portrait, and kept @ UR (boot, pickel & nickel),
		// effective orientation & fb rotate value should always match,
		// that means we just need to shift by +90°, one CW rotation.
		// I suspect this bit of insanity was actually mangled back in for backwards compatibility w/ NTX shenanigans...
		rotation = (rotation + 1U) & 3U;
	}

	// NOTE: Should match *most* Kobo devices...
	// c.f., https://patchwork.openembedded.org/patch/149258
	// NOTE: See also create_and_get_mt_pdata @ drivers/input/touchscreen/cyttsp5_devtree.c,
	//       there may be method to this madness...
	switch (rotation) {
		case FB_ROTATE_UR:
			// NOP!
			break;
		case FB_ROTATE_CW:
			rx = coords->y;
			ry = (unsigned short int) (screenWidth - coords->x - 1);
			break;
		case FB_ROTATE_UD:
			rx = (unsigned short int) (screenWidth - coords->x - 1);
			ry = (unsigned short int) (screenHeight - coords->y - 1);
			break;
		case FB_ROTATE_CCW:
			rx = (unsigned short int) (screenHeight - coords->y - 1);
			ry = coords->x;
			break;
	}

	// NOTE: The H2O²r1 (possibly r2 as well), on the other hand, is a special snowflake...
	//       (It'll need a dedicated deviceQuirks).
	// c.f., https://www.mobileread.com/forums/showpost.php?p=3766627&postcount=236
	//     & https://github.com/baskerville/plato/commit/5181eaf0b48a9e1201b6ea5751c2af108512f74f
	//     & https://github.com/baskerville/plato/commit/bf7af35eef9c29250d206687738b4888f40ecab1
	/*
	switch(rotation) {
		case FB_ROTATE_UR:
			rx = coords->x;
			ry = (unsigned short int) (screenHeight - coords->y - 1);
			break;
		case FB_ROTATE_CW:
			rx = (unsigned short int) (screenHeight - coords->y - 1);
			ry = (unsigned short int) (screenWidth - coords->x - 1);
			break;
		case FB_ROTATE_UD:
			rx = (unsigned short int) (screenWidth - coords->x - 1);
			ry = coords->y;
			break;
		case FB_ROTATE_CCW:
			rx = coords->y;
			ry = coords->x;
			break;
	}
	*/

	coords->x = rx;
	coords->y = ry;
}
#	endif    // FBINK_WITH_BUTTON_SCAN
#endif            // FBINK_FOR_KOBO || FBINK_FOR_CERVANTES

static void
    rotate_coordinates_nop(FBInkCoordinates* restrict coords __attribute__((unused)))
{
	// NOP!
	// May be smarter than one might think on armv7-a,
	// (quoting http://infocenter.arm.com/help/index.jsp?topic=/com.arm.doc.100069_0610_01_en/pge1425898111637.html
	//  "NOP is not necessarily a time-consuming NOP.
	//  The processor might remove it from the pipeline before it reaches the execution stage."),
	// which might explain why this isn't worse than branching (i.e., what we did before
	// https://github.com/NiLuJe/FBInk/commit/75407d4a44d7bfc7705665ad4ec9ecad0d03a368).
}

#ifdef FBINK_WITH_DRAW
// Handle a few sanity checks...
// NOTE: If you can, prefer using the right put_pixel_* function directly.
//       While the bounds checking is generally rather cheap,
//       (i.e., (*fxpPutPixel) is only marginally faster than put_pixel()),
//       the overhead of going through the function pointers is rather large
//       (i.e., put_pixel() can be twice as slow as put_pixel_*()).
//       On the oldest of our target HW, it's often *slightly* faster than branching or switching, though ;).
//       But on modern processors, even on our target HW, branching should eventually take the lead, though,
//       and in this case (ha!) appears to behave *noticeably* better than switching...
//       Which is why we now branch via an if ladder, as it should offer marginally better performance on newer devices.
static inline __attribute__((always_inline, hot)) void
    put_pixel(FBInkCoordinates coords, const FBInkPixel* restrict px, bool is_rgb565)
{
	// Handle rotation now, so we can properly validate if the pixel is off-screen or not ;).
	// fbink_init() takes care of setting this global pointer to the right function...
	// NOTE: In this case, going through the function pointer is *noticeably* faster than branching...
	(*fxpRotateCoords)(&coords);

	// NOTE: Discard off-screen pixels!
	//       For instance, when we have a halfcell offset in conjunction with a !isPerfectFit pixel offset,
	//       when we're padding and centering, the final whitespace of right-padding will have its last
	//       few pixels (the exact amount being half of the dead zone width) pushed off-screen...
	//       And, of course, anything using hoffset or voffset can happily push stuff OOB ;).
	if (unlikely(coords.x >= vInfo.xres || coords.y >= vInfo.yres)) {
#	ifdef DEBUG
		// NOTE: This is only enabled in Debug builds because it can be pretty verbose,
		//       and does not necessarily indicate an actual issue, as we've just explained...
		LOG("Put: discarding off-screen pixel @ (%hu, %hu) (out of %ux%u bounds)",
		    coords.x,
		    coords.y,
		    vInfo.xres,
		    vInfo.yres);
#	endif
		return;
	}

	// NOTE: Hmm, here, an if ladder appears to be ever so *slightly* faster than going through the function pointer...
	if (vInfo.bits_per_pixel == 4U) {
		put_pixel_Gray4(&coords, px);
	} else if (likely(vInfo.bits_per_pixel == 8U)) {
		put_pixel_Gray8(&coords, px);
	} else if (vInfo.bits_per_pixel == 16U) {
		// Do we need to pack the pixel, first?
		if (is_rgb565) {
			// Nope :)
			put_pixel_RGB565(&coords, px);
		} else {
			// Yep :(
			FBInkPixel packed_px;
			// NOTE: Technically legitimate warning. In practice, we always pass RGB32 pixels in 16bpp codepaths.
#	pragma GCC diagnostic   push
#	pragma GCC diagnostic   ignored "-Wunknown-pragmas"
#	pragma clang diagnostic ignored "-Wunknown-warning-option"
#	pragma GCC diagnostic   ignored "-Wmaybe-uninitialized"
                        packed_px.rgb565 = pack_rgb565(px->bgra.color.r, px->bgra.color.g, px->bgra.color.b);
#	pragma GCC diagnostic pop
			put_pixel_RGB565(&coords, &packed_px);
		}
	} else if (unlikely(vInfo.bits_per_pixel == 24U)) {
		put_pixel_RGB24(&coords, px);
	} else if (likely(vInfo.bits_per_pixel == 32U)) {
		put_pixel_RGB32(&coords, px);
	}
}

// Helper functions to 'get' a specific pixel's color from the framebuffer
// c.f., FBGrab convert* functions
//       (http://trac.ak-team.com/trac/browser/niluje/Configs/trunk/Kindle/Misc/FBGrab/fbgrab.c#L402)
// as well as KOReader's routines
//       (https://github.com/koreader/koreader-base/blob/b3e72affd0e1ba819d92194b229468452c58836f/ffi/blitbuffer.lua#L292)
static inline __attribute__((always_inline, hot)) void
    get_pixel_Gray4(const FBInkCoordinates* restrict coords, FBInkPixel* restrict px)
{
	// calculate the pixel's byte offset inside the buffer
	// note: x / 2 as every byte holds 2 pixels
	const size_t pix_offset = (coords->x >> 1U) + (coords->y * fInfo.line_length);

	// NOTE: Expand 4bpp to 8bpp:
	// (v * 0x11)
	// Byte to nibble (c.f., https://en.wikipedia.org/wiki/Nibble)
	// Hi:
	// (((b) >> 4) & 0x0F)
	// Lo:
	// ((b) & 0x0F)

	// We'll need the full byte first...
	const uint8_t b = *((const unsigned char*) (fbPtr + pix_offset));

	if ((coords->x & 0x01u) == 0U) {
		// Even pixel: high nibble
		const uint8_t v = (b & 0xF0u);
		px->gray8       = (v | (v >> 4U));
		// pull the top/left nibble, expanded to 8bit
		// or: (uint8_t)((((b) >> 4) & 0x0F) * 0x11);
	} else {
		// Odd pixel: low nibble
		px->gray8 = (uint8_t) ((b & 0x0Fu) * 0x11u);
		// or: pull the low/right nibble, expanded to 8bit
	}
	// NOTE: c.f., FBInkPixel typedef in fbink_types.h for details on the union shenanigans...
	//       In short: gray8 -> gray4.hi -> bgra.color.b
	//                          gray4.lo -> bgra.color.g
}

static inline __attribute__((always_inline, hot)) void
    get_pixel_Gray8(const FBInkCoordinates* restrict coords, FBInkPixel* restrict px)
{
	// calculate the pixel's byte offset inside the buffer
	const size_t pix_offset = coords->x + (coords->y * fInfo.line_length);

	px->gray8 = *((unsigned char*) (fbPtr + pix_offset));
}

static inline __attribute__((always_inline)) void
    get_pixel_RGB24(const FBInkCoordinates* restrict coords, FBInkPixel* restrict px)
{
	// calculate the pixel's byte offset inside the buffer
	// note: x * 3 as every pixel is 3 consecutive bytes
	const size_t pix_offset = (coords->x * 3U) + (coords->y * fInfo.line_length);

	px->bgra.color.b = *((unsigned char*) (fbPtr + pix_offset));
	px->bgra.color.g = *((unsigned char*) (fbPtr + pix_offset + 1U));
	px->bgra.color.r = *((unsigned char*) (fbPtr + pix_offset + 2U));
}

static inline __attribute__((always_inline, hot)) void
    get_pixel_RGB32(const FBInkCoordinates* restrict coords, FBInkPixel* restrict px)
{
	// calculate the pixel's byte offset inside the buffer
	const size_t scanline_offset = (size_t) coords->y * fInfo.line_length;

#	pragma GCC diagnostic push
#	pragma GCC diagnostic ignored "-Wcast-align"
	px->bgra.p = *((uint32_t*) (fbPtr + scanline_offset) + coords->x);
#	pragma GCC diagnostic pop
	// NOTE: We generally don't care about alpha, we always assume it's opaque, as that's how it behaves.
	//       We *do* pickup the actual alpha value, here, though.
}

static inline __attribute__((always_inline, hot)) void
    get_pixel_RGB565(const FBInkCoordinates* restrict coords, FBInkPixel* restrict px)
{
	// calculate the pixel's byte offset inside the buffer
	const size_t scanline_offset = (size_t) coords->y * fInfo.line_length;

	// NOTE: We're honoring the fb's bitfield offsets here (B: 0, G: >> 5, R: >> 11)
	// Like put_pixel_RGB565, read those two consecutive bytes at once
#	pragma GCC diagnostic push
#	pragma GCC diagnostic ignored "-Wcast-align"
	const uint16_t         v = *((const uint16_t*) (fbPtr + scanline_offset) + coords->x);
#	pragma GCC diagnostic pop

	// NOTE: Unpack to RGB32, because we have no use for RGB565, it's terrible.
	// NOTE: c.f., https://stackoverflow.com/q/2442576
	//       I feel that this approach tracks better with what we do in put_pixel_RGB565,
	//       and I have an easier time following it than the previous approach ported from KOReader.
	//       Both do exactly the same thing, though ;).
	const uint8_t r = (uint8_t) ((v & 0xF800u) >> 11U);    // 11111000 00000000 = 0xF800
	const uint8_t g = (v & 0x07E0u) >> 5U;                 // 00000111 11100000 = 0x07E0
	const uint8_t b = (v & 0x001Fu);                       // 00000000 00011111 = 0x001F

	px->bgra.color.r = (uint8_t) ((r << 3U) | (r >> 2U));
	px->bgra.color.g = (uint8_t) ((g << 2U) | (g >> 4U));
	px->bgra.color.b = (uint8_t) ((b << 3U) | (b >> 2U));
}

// Handle a few sanity checks...
static inline __attribute__((always_inline, hot)) void
    get_pixel(FBInkCoordinates coords, FBInkPixel* restrict px)
{
	// Handle rotation now, so we can properly validate if the pixel is off-screen or not ;).
	// fbink_init() takes care of setting this global pointer to the right function...
	(*fxpRotateCoords)(&coords);

	// NOTE: Discard off-screen pixels!
	//       For instance, when we have a halfcell offset in conjunction with a !isPerfectFit pixel offset,
	//       when we're padding and centering, the final whitespace of right-padding will have its last
	//       few pixels (the exact amount being half of the dead zone width) pushed off-screen...
	//       And, of course, anything using hoffset or voffset can happily push stuff OOB ;).
	if (unlikely(coords.x >= vInfo.xres || coords.y >= vInfo.yres)) {
#	ifdef DEBUG
		// NOTE: This is only enabled in Debug builds because it can be pretty verbose,
		//       and does not necessarily indicate an actual issue, as we've just explained...
		LOG("Put: discarding off-screen pixel @ (%hu, %hu) (out of %ux%u bounds)",
		    coords.x,
		    coords.y,
		    vInfo.xres,
		    vInfo.yres);
#	endif
		return;
	}

	// NOTE: Hmm, here, an if ladder appears to be ever so *slightly* faster than going through the function pointer...
	if (vInfo.bits_per_pixel == 4U) {
		get_pixel_Gray4(&coords, px);
	} else if (likely(vInfo.bits_per_pixel == 8U)) {
		get_pixel_Gray8(&coords, px);
	} else if (vInfo.bits_per_pixel == 16U) {
		get_pixel_RGB565(&coords, px);
	} else if (unlikely(vInfo.bits_per_pixel == 24U)) {
		get_pixel_RGB24(&coords, px);
	} else if (likely(vInfo.bits_per_pixel == 32U)) {
		get_pixel_RGB32(&coords, px);
	}
}

// Helper functions to draw a rectangle in a given color
static __attribute__((hot)) void
    fill_rect_Gray4(unsigned short int x,
		    unsigned short int y,
		    unsigned short int w,
		    unsigned short int h,
		    const FBInkPixel* restrict px)
{
	// Go with pixel plotting @ 4bpp to keep this simple...
	for (unsigned short int cy = 0U; cy < h; cy++) {
		for (unsigned short int cx = 0U; cx < w; cx++) {
			const FBInkCoordinates coords = {
				.x = (unsigned short int) (x + cx),
				.y = (unsigned short int) (y + cy),
			};
			put_pixel_Gray4(&coords, px);
		}
	}

#	ifdef DEBUG
	LOG("Filled a #%02hhX %hux%hu rectangle @ (%hu, %hu)", px->gray8, w, h, x, y);
#	endif
}

static __attribute__((hot)) void
    fill_rect_Gray4_checked(unsigned short int x,
			    unsigned short int y,
			    unsigned short int w,
			    unsigned short int h,
			    const FBInkPixel* restrict px)
{
	// Bounds-checking, to ensure the memset won't do stupid things...
	// Do signed maths, to account for the fact that x or y might already be OOB!
	if (unlikely(x + w > screenWidth)) {
		w = (unsigned short int) MAX(0, (w - ((x + w) - (int) screenWidth)));
#	ifdef DEBUG
		LOG("Chopped rectangle width to %hu", w);
#	endif
	}
	if (unlikely(y + h > screenHeight)) {
		h = (unsigned short int) MAX(0, (h - ((y + h) - (int) screenHeight)));
#	ifdef DEBUG
		LOG("Chopped rectangle height to %hu", h);
#	endif
	}

	// Abort early if that left us with an empty rectangle ;).
	if (unlikely(w == 0U || h == 0U)) {
#	ifdef DEBUG
		LOG("Skipped empty %hux%hu rectangle @ (%hu, %hu)", w, h, x, y);
#	endif
		return;
	}

	return fill_rect_Gray4(x, y, w, h, px);
}

#	ifdef FBINK_FOR_POCKETBOOK
static __attribute__((hot)) void
    fill_rect_Gray8(unsigned short int x,
		    unsigned short int y,
		    unsigned short int w,
		    unsigned short int h,
		    const FBInkPixel* restrict px)
{
	// NOTE: We may require fxpRotateRegion on PB :(.
	struct mxcfb_rect region = {
		.top    = y,
		.left   = x,
		.width  = w,
		.height = h,
	};
	(*fxpRotateRegion)(&region);

	for (size_t j = region.top; j < region.top + region.height; j++) {
		uint8_t* p = fbPtr + (fInfo.line_length * j) + (region.left);
		memset(p, px->gray8, region.width);
	}

#		ifdef DEBUG
	LOG("Filled a #%02hhX %hux%hu rectangle @ (%hu, %hu)", px->gray8, w, h, x, y);
#		endif
}
#	else
static __attribute__((hot)) void
    fill_rect_Gray8(unsigned short int x,
		    unsigned short int y,
		    unsigned short int w,
		    unsigned short int h,
		    const FBInkPixel* restrict px)
{
	// NOTE: fxpRotateRegion is never set at 8bpp :).
	for (size_t j = y; j < y + h; j++) {
		uint8_t* p = fbPtr + (fInfo.line_length * j) + (x);
		memset(p, px->gray8, w);
	}

#		ifdef DEBUG
	LOG("Filled a #%02hhX %hux%hu rectangle @ (%hu, %hu)", px->gray8, w, h, x, y);
#		endif
}
#	endif

static __attribute__((hot)) void
    fill_rect_Gray8_checked(unsigned short int x,
			    unsigned short int y,
			    unsigned short int w,
			    unsigned short int h,
			    const FBInkPixel* restrict px)
{
	// Bounds-checking, to ensure the memset won't do stupid things...
	// Do signed maths, to account for the fact that x or y might already be OOB!
	if (unlikely(x + w > screenWidth)) {
		w = (unsigned short int) MAX(0, (w - ((x + w) - (int) screenWidth)));
#	ifdef DEBUG
		LOG("Chopped rectangle width to %hu", w);
#	endif
	}
	if (unlikely(y + h > screenHeight)) {
		h = (unsigned short int) MAX(0, (h - ((y + h) - (int) screenHeight)));
#	ifdef DEBUG
		LOG("Chopped rectangle height to %hu", h);
#	endif
	}

	// Abort early if that left us with an empty rectangle ;).
	if (unlikely(w == 0U || h == 0U)) {
#	ifdef DEBUG
		LOG("Skipped empty %hux%hu rectangle @ (%hu, %hu)", w, h, x, y);
#	endif
		return;
	}

	return fill_rect_Gray8(x, y, w, h, px);
}

static __attribute__((hot)) void
    fill_rect_RGB565(unsigned short int x,
		     unsigned short int y,
		     unsigned short int w,
		     unsigned short int h,
		     const FBInkPixel* restrict px)
{
	// Things are a bit trickier @ 16bpp, because except for black or white, we're not sure the requested color
	// will be composed of two indentical bytes when packed as RGB565... -_-".
	// NOTE: Silver lining: as fill_rect was originally designed to only ever be fed eInk palette colors,
	//       we have a guarantee that the input pixel is already packed, so we can use px->rgb565 ;).

	struct mxcfb_rect region = {
		.top    = y,
		.left   = x,
		.width  = w,
		.height = h,
	};
	(*fxpRotateRegion)(&region);

	// And that's a cheap-ass manual memset16, let's hope the compiler can do something fun with that...
	// That's the exact pattern used by the Linux kernel (c.f., memset16 @ lib/string.c), so, here's hoping ;).
	for (size_t j = region.top; j < region.top + region.height; j++) {
		const size_t scanline_offset = fInfo.line_length * j;
#	pragma GCC diagnostic push
#	pragma GCC diagnostic ignored "-Wcast-align"
		uint16_t* restrict p = (uint16_t*) (fbPtr + scanline_offset) + region.left;
#	pragma GCC diagnostic pop
		size_t px_count = region.width;

		while (px_count--) {
			*p++ = px->rgb565;
		}
	}

#	ifdef DEBUG
	LOG("Filled a #%02hhX %hux%hu rectangle @ (%hu, %hu)", px->gray8, w, h, x, y);
#	endif
}

static __attribute__((hot)) void
    fill_rect_RGB565_checked(unsigned short int x,
			     unsigned short int y,
			     unsigned short int w,
			     unsigned short int h,
			     const FBInkPixel* restrict px)
{
	// Bounds-checking, to ensure the memset won't do stupid things...
	// Do signed maths, to account for the fact that x or y might already be OOB!
	// NOTE: Unlike put_pixel, we check against screenWidth/screenHeight instead of xres/yres because we're doing this
	//       *before* fxpRotateRegion!
	if (unlikely(x + w > screenWidth)) {
		w = (unsigned short int) MAX(0, (w - ((x + w) - (int) screenWidth)));
#	ifdef DEBUG
		LOG("Chopped rectangle width to %hu", w);
#	endif
	}
	if (unlikely(y + h > screenHeight)) {
		h = (unsigned short int) MAX(0, (h - ((y + h) - (int) screenHeight)));
#	ifdef DEBUG
		LOG("Chopped rectangle height to %hu", h);
#	endif
	}

	// Abort early if that left us with an empty rectangle ;).
	if (unlikely(w == 0U || h == 0U)) {
#	ifdef DEBUG
		LOG("Skipped empty %hux%hu rectangle @ (%hu, %hu)", w, h, x, y);
#	endif
		return;
	}

	return fill_rect_RGB565(x, y, w, h, px);
}

static void
    fill_rect_RGB24(unsigned short int x,
		    unsigned short int y,
		    unsigned short int w,
		    unsigned short int h,
		    const FBInkPixel* restrict px)
{
	// NOTE: fxpRotateRegion is never set at 24bpp :).
	for (size_t j = y; j < y + h; j++) {
		uint8_t* p = fbPtr + (fInfo.line_length * j) + (x * 3U);
		memset(p, px->gray8, w * 3U);
	}

#	ifdef DEBUG
	LOG("Filled a #%02hhX %hux%hu rectangle @ (%hu, %hu)", px->gray8, w, h, x, y);
#	endif
}

static void
    fill_rect_RGB24_checked(unsigned short int x,
			    unsigned short int y,
			    unsigned short int w,
			    unsigned short int h,
			    const FBInkPixel* restrict px)
{
	// Bounds-checking, to ensure the memset won't do stupid things...
	// Do signed maths, to account for the fact that x or y might already be OOB!
	if (unlikely(x + w > screenWidth)) {
		w = (unsigned short int) MAX(0, (w - ((x + w) - (int) screenWidth)));
#	ifdef DEBUG
		LOG("Chopped rectangle width to %hu", w);
#	endif
	}
	if (unlikely(y + h > screenHeight)) {
		h = (unsigned short int) MAX(0, (h - ((y + h) - (int) screenHeight)));
#	ifdef DEBUG
		LOG("Chopped rectangle height to %hu", h);
#	endif
	}

	// Abort early if that left us with an empty rectangle ;).
	if (unlikely(w == 0U || h == 0U)) {
#	ifdef DEBUG
		LOG("Skipped empty %hux%hu rectangle @ (%hu, %hu)", w, h, x, y);
#	endif
		return;
	}

	return fill_rect_RGB24(x, y, w, h, px);
}

static __attribute__((hot)) void
    fill_rect_RGB32(unsigned short int x,
		    unsigned short int y,
		    unsigned short int w,
		    unsigned short int h,
		    const FBInkPixel* restrict px)
{
	// NOTE: fxpRotateRegion is never set at 32bpp :).
	for (size_t j = y; j < y + h; j++) {
		// NOTE: Go with a cheap memset32 in order to preserve the alpha value of our input pixel...
		//       The compiler should be able to turn that into something as fast as a plain memset ;).
		const size_t scanline_offset = fInfo.line_length * j;
#	pragma GCC diagnostic push
#	pragma GCC diagnostic ignored "-Wcast-align"
		uint32_t* p = (uint32_t*) (fbPtr + scanline_offset) + x;
#	pragma GCC diagnostic pop
		size_t px_count = w;

		while (px_count--) {
			*p++ = px->bgra.p;
		}
	}

#	ifdef DEBUG
	LOG("Filled a #%02hhX %hux%hu rectangle @ (%hu, %hu)", px->gray8, w, h, x, y);
#	endif
}

static __attribute__((hot)) void
    fill_rect_RGB32_checked(unsigned short int x,
			    unsigned short int y,
			    unsigned short int w,
			    unsigned short int h,
			    const FBInkPixel* restrict px)
{
	// Bounds-checking, to ensure the memset won't do stupid things...
	// Do signed maths, to account for the fact that x or y might already be OOB!
	if (unlikely(x + w > screenWidth)) {
		w = (unsigned short int) MAX(0, (w - ((x + w) - (int) screenWidth)));
#	ifdef DEBUG
		LOG("Chopped rectangle width to %hu", w);
#	endif
	}
	if (unlikely(y + h > screenHeight)) {
		h = (unsigned short int) MAX(0, (h - ((y + h) - (int) screenHeight)));
#	ifdef DEBUG
		LOG("Chopped rectangle height to %hu", h);
#	endif
	}

	// Abort early if that left us with an empty rectangle ;).
	if (unlikely(w == 0U || h == 0U)) {
#	ifdef DEBUG
		LOG("Skipped empty %hux%hu rectangle @ (%hu, %hu)", w, h, x, y);
#	endif
		return;
	}

	return fill_rect_RGB32(x, y, w, h, px);
}

// Helper function to clear the screen - fill whole screen with given color
static void
    clear_screen(int fbfd UNUSED_BY_NOTKINDLE, uint8_t v, bool is_flashing UNUSED_BY_NOTKINDLE)
{
#	ifdef FBINK_FOR_KINDLE
	// NOTE: einkfb has a dedicated ioctl, so, use that, when it's not doing more harm than good...
	if (deviceQuirks.isKindleLegacy) {
		// NOTE: The ioctl only does white, though, and it has a tendency to enforce a flash,
		//       which would cause a double refresh if we were to print a rectangle in another color right after...
		//       So, basically, only use the ioctl when we request a FLASHING clear to WHITE...
		//       This still enforces an explicit flash that's lengthier than what we usually get,
		//       not that that's a great comparison, since we hardly ever manage to coax a *real* full flash
		//       out of einkfb... This appears to do it (i.e., ~800ms)!
		// NOTE: We're on inverted palette devices, hence the use of the "wrong" LUT...
		// NOTE: We're calling clear_screen to handle is_cleared,
		//       which means we're probably refreshing twice in these cases...
		//       Oh, well...
		if (is_flashing && v == eInkFGCMap[BG_WHITE]) {
			if (ioctl(fbfd, FBIO_EINK_CLEAR_SCREEN, EINK_CLEAR_SCREEN) < 0) {
				PFWARN("FBIO_EINK_CLEAR_SCREEN: %m");
				// Just warn, this is non-fatal ;).
			}

			LOG("Requested a flashing WHITE clear, only doing an FBIO_EINK_CLEAR_SCREEN to save some time!");
			return;
		}
		// NOTE: And because we can't have nice things, the einkfb driver has a stupid "optimization",
		//       where it discards redundant FBIO_EINK_UPDATE_DISPLAY* calls if the buffer content hasn't changed...
		//       If we memset the full smem_len, that trips this check, because we probably overwrite both buffers...
		//       Do a slightly more targeted memset instead (line_length * yres_virtual),
		//       which should cover the active & visible buffer only...
		memset(fbPtr, v, fInfo.line_length * vInfo.yres_virtual);
	} else {
		memset(fbPtr, v, fInfo.smem_len);
	}
#	else
	// NOTE: Apparently, some NTX devices do not appreciate a memset of the full smem_len when they're in a 16bpp mode...
	//       In this mode, smem_len is twice as large as it needs to be,
	//       and Cervantes' Qt driver takes this opportunity to use the offscreen memory region to do some... stuff.
	//       c.f., https://github.com/bq/cervantes-qt/blob/eink-imx508/src/plugins/gfxdrivers/einkfb/einkfb.cpp,
	//       in particular size/psize vs. mapsize
	//       Anyway, don't clobber that, as it seems to cause softlocks on BQ/Cervantes,
	//       and be very conservative, using yres instead of yres_virtual, as Qt *may* already rely on that memory region.
	if (unlikely(vInfo.bits_per_pixel == 16U)) {
		// NOTE: Besides, we can't use a straight memset, since we need pixels to be properly packed for RGB565...
		//       Se we whip up a quick memset16, like fill_rect() does.
		const uint16_t px       = pack_rgb565(v, v, v);
#		pragma GCC diagnostic push
#		pragma GCC diagnostic ignored "-Wcast-align"
		uint16_t*      p        = (uint16_t*) fbPtr;
#		pragma GCC diagnostic pop
		size_t         px_count = (size_t) vInfo.xres_virtual * vInfo.yres;
		while (px_count--) {
			*p++ = px;
		}
	} else if (vInfo.bits_per_pixel == 32U) {
		// Much like in fill_rect_RGB32, whip up something that'll preserve the alpha byte...
		const FBInkPixelBGRA px       = { .color.b = v, .color.g = v, .color.r = v, .color.a = 0xFF };
#		pragma GCC diagnostic push
#		pragma GCC diagnostic ignored "-Wcast-align"
		uint32_t*            p        = (uint32_t*) fbPtr;
#		pragma GCC diagnostic pop
		size_t               px_count = (size_t) vInfo.xres_virtual * vInfo.yres;
		while (px_count--) {
			*p++ = px.p;
		}
	} else {
		// NOTE: fInfo.smem_len should actually match fInfo.line_length * vInfo.yres_virtual on 32bpp ;).
		//       Which is how things should always be, but, alas, poor Yorick...
		memset(fbPtr, v, fInfo.smem_len);
	}
#	endif
}

/*
// Same thing, but with a FG/BG checkerboard pattern instead of a uniform BG
static void
    checkerboard_screen(void)
{
	const uint8_t bpp = (uint8_t)(vInfo.bits_per_pixel >> 3U);
	// NOTE: EPDC alignment constraints ensure that xres_virtual will be evenly divisble by 16,
	//       (as it's aligned to the next multiple of 32).
	const uint32_t px_stride = vInfo.xres_virtual >> 4U;

	bool checker = false;
	for (size_t y = 0U; y < vInfo.yres; y++) {
		// Alternate initial color every px_stride lines,
		// (depending on whether the amount of vertical squares we've already painted (i.e., y/stride) is even or odd).
		checker = !!((y / px_stride) & 1);
		for (size_t x = 0U; x < vInfo.xres_virtual; x += px_stride) {
			// NOTE: That's a bit of a shortcut @ RGB565 since we're not using properly packed pixels...
			memset(fbPtr + ((y * fInfo.line_length) + (x * bpp)),
			       checker ? penBGColor : penFGColor,
			       (size_t)(px_stride * bpp));
			checker = !checker;
		}
	}
}
*/
#endif    // FBINK_WITH_DRAW

#ifdef FBINK_WITH_BITMAP
// Return the font8x8 bitmap for a specific Unicode codepoint
static const unsigned char*
    font8x8_get_bitmap(uint32_t codepoint)
{
	// Get the bitmap for the character mapped to that Unicode codepoint
	if (likely(codepoint <= 0x7Fu)) {
		return font8x8_basic[codepoint];
	} else if (codepoint >= 0x80u && codepoint <= 0x9Fu) {    // lgtm [cpp/constant-comparison]
		return font8x8_control[codepoint - 0x80u];
	} else if (codepoint >= 0xA0u && codepoint <= 0xFFu) {    // lgtm [cpp/constant-comparison]
		return font8x8_ext_latin[codepoint - 0xA0u];
	} else if (codepoint >= 0x390u && codepoint <= 0x3C9u) {
		return font8x8_greek[codepoint - 0x390u];
	} else if (codepoint >= 0x2500u && codepoint <= 0x257Fu) {
		return font8x8_box[codepoint - 0x2500u];
	} else if (codepoint >= 0x2580u && codepoint <= 0x259Fu) {    // lgtm [cpp/constant-comparison]
		return font8x8_block[codepoint - 0x2580u];
	} else if (codepoint >= 0x3040u && codepoint <= 0x309Fu) {
		return font8x8_hiragana[codepoint - 0x3040u];
	} else {
		// NOTE: Print a blank space for unknown codepoints
		WARN("Codepoint U+%04X (%s) is not covered by this font", codepoint, u8_cp_to_utf8(codepoint));
		return font8x8_basic[0];
	}
}
#endif    // FBINK_WITH_BITMAP

static __attribute__((cold)) const char*
    fontname_to_string(uint8_t fontname)
{
	switch (fontname) {
#ifdef FBINK_WITH_BITMAP
		case IBM:
			return "IBM";
#endif
#ifdef FBINK_WITH_FONTS
		case UNSCII:
			return "Unscii";
		case UNSCII_ALT:
			return "Unscii Alt";
		case UNSCII_THIN:
			return "Unscii Thin";
		case UNSCII_FANTASY:
			return "Unscii Fantasy";
		case UNSCII_MCR:
			return "Unscii MCR";
		case UNSCII_TALL:
			return "Unscii 16";
		case BLOCK:
			return "BLOCK";
		case LEGGIE:
			return "Leggie Regular";
		case VEGGIE:
			return "Leggie VGA/EGA/FB";
		case KATES:
			return "Kates";
		case FKP:
			return "FKP";
		case CTRLD:
			return "CtrlD";
		case ORP:
			return "Orp Regular";
		case ORPB:
			return "Orp Bold";
		case ORPI:
			return "Orp Italic";
		case SCIENTIFICA:
			return "Scientifica Regular";
		case SCIENTIFICAB:
			return "Scientifica Bold";
		case SCIENTIFICAI:
			return "Scientifica Italic";
		case TERMINUS:
			return "Terminus";
		case TERMINUSB:
			return "Terminus Bold";
		case FATTY:
			return "Fatty";
		case SPLEEN:
			return "Spleen";
		case TEWI:
			return "Tewi";
		case TEWIB:
			return "Tewi Bold";
		case TOPAZ:
			return "Topaz+ A1200";
		case MICROKNIGHT:
			return "MicroKnight+";
		case VGA:
			return "IBM VGA";
#	ifdef FBINK_WITH_UNIFONT
		case UNIFONT:
			return "Unifont";
		case UNIFONTDW:
			return "Unifont (double-wide)";
#	endif
		case COZETTE:
			return "Cozette";
#endif
		default:
			return "IBM (Default)";
	}
}

#ifdef FBINK_WITH_BITMAP
// KISS helper function to count the amount of digits in an integer (for dynamic padding purposes in printf calls)
// c.f., https://stackoverflow.com/a/3069580
static int
    zu_print_length(size_t x)
{
	// NOTE: Cut that short, we'll rarely pass stuff larger than that, and if we ever do, padding be damned.
	/*
	if (x >= 1000000000u) {
		return 10;
	}
	if (x >= 100000000u) {
		return 9;
	}
	if (x >= 10000000u) {
		return 8;
	}
	if (x >= 1000000u) {
		return 7;
	}
	if (x >= 100000u) {
		return 6;
	}
	*/
	if (x >= 10000u) {
		return 5;
	}
	if (x >= 1000u) {
		return 4;
	}
	if (x >= 100u) {
		return 3;
	}
	if (x >= 10u) {
		return 2;
	}
	return 1;
}

// Helper function for drawing
static struct mxcfb_rect
    draw(const char* restrict text,
	 unsigned short int row,
	 unsigned short int col,
	 unsigned short int multiline_offset,
	 bool               halfcell_offset,
	 const FBInkConfig* restrict fbink_cfg)
{
	LOG("Printing `%s` @ line offset %hu (meaning row %hu)",
	    text,
	    multiline_offset,
	    (unsigned short int) (row + multiline_offset));

	FBInkPixel fgP = penFGPixel;
	FBInkPixel bgP = penBGPixel;
	if (fbink_cfg->is_inverted) {
		// NOTE: And, of course, RGB565 is terrible. Inverting the lossy packed value would be even lossier...
		if (unlikely(vInfo.bits_per_pixel == 16U)) {
			const uint8_t fgcolor = penFGColor ^ 0xFFu;
			const uint8_t bgcolor = penBGColor ^ 0xFFu;
			fgP.rgb565            = pack_rgb565(fgcolor, fgcolor, fgcolor);
			bgP.rgb565            = pack_rgb565(bgcolor, bgcolor, bgcolor);
		} else {
			fgP.bgra.p ^= 0x00FFFFFFu;
			bgP.bgra.p ^= 0x00FFFFFFu;
		}
	}

	// Adjust row in case we're a continuation of a multi-line print...
	row = (unsigned short int) (row + multiline_offset);

	// Compute the length of our actual string
	// NOTE: We already took care in fbink_print() of making sure that the string passed in text wouldn't take up
	//       more space (as in columns, not bytes) than (MAXCOLS - col), the maximum printable length.
	//       And as we're printing glyphs, we need to iterate over the number of characters/grapheme clusters,
	//       not bytes.
	const size_t charcount = u8_strlen2(text);
	const size_t txtlength = strlen(text);
	// Flawfinder: ignore
	LOG("Character count: %zu (over %zu bytes)", charcount, txtlength);

	// Compute our actual subcell offset in pixels
	unsigned short int pixel_offset = 0U;
	// Do we have a centering induced halfcell adjustment to correct?
	if (halfcell_offset) {
		pixel_offset = FONTW / 2U;
		LOG("Incrementing pixel_offset by %hu pixels to account for a halfcell centering tweak", pixel_offset);
	}
	// Do we have a permanent adjustment to make because of dead space on the right edge?
	if (!deviceQuirks.isPerfectFit) {
		// We correct by half of said dead space, since we want perfect centering ;).
		unsigned short int deadzone_offset =
		    (unsigned short int) (viewWidth - (unsigned short int) (MAXCOLS * FONTW)) / 2U;
		pixel_offset = (unsigned short int) (pixel_offset + deadzone_offset);
		LOG("Incrementing pixel_offset by %hu pixels to compensate for dead space on the right edge",
		    deadzone_offset);
	}

	// Clamp h/v offset to safe values
	short int voffset = fbink_cfg->voffset;
	short int hoffset = fbink_cfg->hoffset;
	// NOTE: This test isn't perfect, but then, if you play with this, you do it knowing the risks...
	//       It's mainly there so that stupidly large values don't wrap back on screen because of overflow wraparound.
	if ((uint32_t) abs(voffset) >= viewHeight) {
		LOG("The specified vertical offset (%hd) necessarily pushes *all* content out of bounds, discarding it",
		    voffset);
		voffset = 0;
	}
	if ((uint32_t) abs(hoffset) >= viewWidth) {
		LOG("The specified horizontal offset (%hd) necessarily pushes *all* content out of bounds, discarding it",
		    hoffset);
		hoffset = 0;
	}

	// Compute the dimension of the screen region we'll paint to (taking multi-line into account)
	struct mxcfb_rect region = {
		.top  = (uint32_t) MAX(0 + (viewVertOrigin - viewVertOffset),
                                      (((row - multiline_offset) * FONTH) + voffset + viewVertOrigin)),
		.left = (uint32_t) MAX(0 + viewHoriOrigin, ((col * FONTW) + hoffset + viewHoriOrigin)),
		.width =
		    multiline_offset > 0U ? (screenWidth - (uint32_t) (col * FONTW)) : (uint32_t) (charcount * FONTW),
		.height = (uint32_t) ((multiline_offset + 1U) * FONTH),
	};

	// Recap final offset values
	if (hoffset != 0 || viewHoriOrigin != 0) {
		LOG("Adjusting horizontal pen position by %hd pixels, as requested, plus %hhu pixels, as mandated by the native viewport",
		    hoffset,
		    viewHoriOrigin);
		// Clamp region to sane values if h/v offset is pushing stuff off-screen
		if ((region.width + region.left + pixel_offset) > screenWidth) {
			region.width = (uint32_t) MAX(0, (short int) (screenWidth - region.left - pixel_offset));
			LOG("Adjusted region width to account for horizontal offset pushing part of the content off-screen");
		}
		if ((region.left + pixel_offset) >= screenWidth) {
			region.left = screenWidth - pixel_offset - 1;
			LOG("Adjusted region left to account for horizontal offset pushing part of the content off-screen");
		}
	}
	if (voffset != 0 || viewVertOrigin != 0) {
		LOG("Adjusting vertical pen position by %hd pixels, as requested, plus %hhu pixels, as mandated by the native viewport",
		    voffset,
		    viewVertOrigin);
		// Clamp region to sane values if h/v offset is pushing stuff off-screen
		if ((region.top + region.height) > screenHeight) {
			region.height = (uint32_t) MAX(0, (short int) (screenHeight - region.top));
			LOG("Adjusted region height to account for vertical offset pushing part of the content off-screen");
		}
		if (region.top >= screenHeight) {
			region.top = screenHeight - 1;
			LOG("Adjusted region top to account for vertical offset pushing part of the content off-screen");
		}
	}

	LOG("Region: top=%u, left=%u, width=%u, height=%u", region.top, region.left, region.width, region.height);

	// Do we have a pixel offset to honor?
	if (pixel_offset > 0U) {
		LOG("Moving pen %hu pixels to the right to honor subcell centering adjustments", pixel_offset);
		// NOTE: We need to update the start of our region rectangle if it doesn't already cover the full line,
		//       i.e., when we're not padding + centering.
		// NOTE: Make sure we don't mess with multiline strings,
		//       because by definition left has to keep matching the value of the first line,
		//       even on subsequent lines.
		if (charcount != MAXCOLS && multiline_offset == 0U) {
			if ((hoffset + viewHoriOrigin) == 0) {
				region.left += pixel_offset;
				LOG("Updated region.left to %u", region.left);
			} else {
				// Things are a bit more complex when hoffset is involved...
				// We basically have to re-do the maths from scratch,
				// do it signed to catch corner-cases interactions between col/hoffset/pixel_offset,
				// and clamp it to safe values!
				if ((hoffset + viewHoriOrigin) < 0) {
					region.left =
					    (uint32_t) MAX(0 + viewHoriOrigin,
							   ((col * FONTW) + hoffset + viewHoriOrigin + pixel_offset));
				} else {
					region.left = (uint32_t) MIN(
					    (uint32_t) ((col * FONTW) + hoffset + viewHoriOrigin + pixel_offset),
					    (screenWidth - 1U));
				}
				LOG("Updated region.left to %u", region.left);
			}
		}
	}

	// If we're a full line, we need to fill the space that honoring our offset has left vacant on the left edge...
	// NOTE: Do it also when we're a left-aligned uncentered multiline string, no matter the length of the line,
	//       so the final line matches the previous ones, which fell under the charcount == MAXCOLS case,
	//       while the final one would not if it doesn't fill the line, too ;).
	// NOTE: In overlay or bgless mode, we don't paint background pixels. This is pure background, so skip it ;).
	if (!fbink_cfg->is_overlay && !fbink_cfg->is_bgless) {
		if ((charcount == MAXCOLS || (col == 0 && !fbink_cfg->is_centered && multiline_offset > 0U)) &&
		    pixel_offset > 0U) {
			LOG("Painting a background rectangle on the left edge on account of pixel_offset");
			// Make sure we don't leave a hoffset sized gap when we have a positive hoffset...
			(*fxpFillRectChecked)(
			    hoffset > 0 ? (unsigned short int) (hoffset + viewHoriOrigin)
					: (unsigned short int) (0U + viewHoriOrigin),
			    (unsigned short int) (region.top + (unsigned short int) (multiline_offset * FONTH)),
			    pixel_offset,    // Don't append hoffset here, to make it clear stuff moved to the right.
			    FONTH,
			    &bgP);
			// Correct width, to include that bit of content, too, if needed
			if (region.width < screenWidth) {
				region.width += pixel_offset;
				// And make sure it's properly clamped, because we can't necessarily rely on left & width
				// being entirely acurate either because of the multiline print override,
				// or because of a bit of subcell placement overshoot trickery (c.f., comment in put_pixel).
				if (region.width + region.left > screenWidth) {
					region.width = screenWidth - region.left;
					LOG("Clamped region.width to %u", region.width);
				} else {
					LOG("Updated region.width to %u", region.width);
				}
			}
		}

		// NOTE: In some cases, we also have a matching hole to patch on the right side...
		//       This only applies when pixel_offset *only* accounts for the !isPerfectFit adjustment though,
		//       because in every other case, the halfcell offset handling neatly pushes everything into place ;).
		// NOTE: Again, skip this in overlay/bgless mode ;).
		if (charcount == MAXCOLS && !deviceQuirks.isPerfectFit && !halfcell_offset) {
			// NOTE: !isPerfectFit ensures pixel_offset is non-zero
			LOG("Painting a background rectangle to fill the dead space on the right edge");
			// Make sure we don't leave a hoffset sized gap when we have a negative hoffset...
			(*fxpFillRectChecked)(
			    hoffset < 0 ? (unsigned short int) (screenWidth - pixel_offset -
								(unsigned short int) abs(hoffset) - viewHoriOrigin)
					: (unsigned short int) (screenWidth - pixel_offset - viewHoriOrigin),
			    (unsigned short int) (region.top + (unsigned short int) (multiline_offset * FONTH)),
			    pixel_offset,    // Don't append abs(hoffset) here, to make it clear stuff moved to the left.
			    FONTH,
			    &bgP);
			// If it's not already the case, update region to the full width,
			// because we've just plugged a hole at the very right edge of a full line.
			if (region.width < screenWidth) {
				region.width = screenWidth;
				// Keep making sure it's properly clamped, interaction w/ hoffset can push us over the edge.
				if (region.width + region.left > screenWidth) {
					region.width = screenWidth - region.left;
					LOG("Clamped region.width to %u", region.width);
				} else {
					LOG("Updated region.width to %u", region.width);
				}
			}
		}
	}

	// NOTE: In case of a multi-line centered print, we can't really trust the final col,
	//       it might be significantly different than the others, and as such, we'd be computing a cropped region.
	//       Make the region cover the full width of the screen to make sure we won't miss anything.
	if (multiline_offset > 0U && fbink_cfg->is_centered &&
	    (region.left > (0U + viewHoriOrigin) || region.width < screenWidth)) {
		region.left  = 0U + viewHoriOrigin;
		region.width = screenWidth;
		LOG("Enforced region.left to %u & region.width to %u because of multi-line centering",
		    region.left,
		    region.width);
	}

	// Loop through all the *characters* in the text string
	size_t                   bi    = 0U;
	size_t                   ch_bi = bi;
	size_t                   ci    = 0U;
	uint32_t                 ch;
	FBInkCoordinates         coords = { 0U };
	FBInkPixel*              pxP;
	// NOTE: We don't do much sanity checking on hoffset/voffset,
	//       because we want to allow pushing part of the string off-screen
	//       (we basically only make sure it won't screw up the region rectangle too badly).
	//       put_pixel is checked, and will discard off-screen pixels safely.
	//       Because we store the final position in an unsigned value, this means that, to some extent,
	//       we rely on wraparound on underflow to still point to (large, but positive) off-screen coordinates.
	const unsigned short int x_base_offs =
	    (unsigned short int) ((col * FONTW) + pixel_offset + hoffset + viewHoriOrigin);
	const unsigned short int y_offs = (unsigned short int) ((row * FONTH) + voffset + viewVertOrigin);

	unsigned short int i;
	unsigned short int j;
	unsigned short int cx;
	unsigned short int cy;

	// We'll also need to compute the amount of zero padding we'll want for logging...
	// i.e., we'll use the amount of digits in the text's length in bytes as the printf field width.
	// We cap at 5 because that should cover most sane use-cases.
	int pad_len = zu_print_length(txtlength);

	// NOTE: Extra code duplication because the glyph's bitmap data type depends on the glyph's width,
	//       so, one way or another, we have to duplicate the inner loops,
	//       but we want to inline this *and* branch outside the loops,
	//       and I don't feel like moving that to inline functions,
	//       because it depends on seven billion different variables I'd have to pass around...
#	ifdef FBINK_WITH_FONTS
	if (glyphWidth <= 8) {
#	endif
		while ((ch = u8_nextchar2(text, &bi)) != 0U) {
			LOG("Char %.*zu out of %.*zu is @ byte offset %.*zu and is U+%04X (%s)",
			    pad_len,
			    (ci + 1U),
			    pad_len,
			    charcount,
			    pad_len,
			    ch_bi,
			    ch,
			    u8_cp_to_utf8(ch));

			// Update the x coordinates for this character
			const unsigned short int x_offs = (unsigned short int) (x_base_offs + (ci * FONTW));
			// Remember the next char's byte offset for next iteration's logging
			ch_bi                           = bi;

			// Crappy macro to avoid repeating myself in each branch...
			// NOTE: When no special processing is needed, we attempt to speed things up by using fill_rect
			//       instead of plotting pixels one by one.
			//       The initial implementation (ca. 2caac37)
			//       was simply mapping one input pixel to one output scaled square,
			//       this one (ca. 08a1c29) attempts to merge rectangles in the *current* row.
			//       My heavily math-challenged attempt at detecting rectangles in both directions
			//       turned out to be too expensive to compute to be really useful (the gain is roughly null).
			//       (c.f., https://gist.github.com/NiLuJe/306f33510846bf30e35271f0eee2d263, avert your eyes).
			//       There's probably much faster math-y ways to do that, but that's not my field ;).
			//       Even then, it'd probably make more sense to make this a pre-processing step,
			//       that would generate a different font format to use at runtime,
			//       one that's basically just a list of rectangles (tl coordinates + wh) to draw.
			//       Think SVG redux, with a single shape: filled rectangles ;).
			// NOTE: Suprisingly enough, a variant of the current approach (ca. ae82336),
			//       in which we start by filling the bg canvas, then only draw fg rectangles,
			//       did not yield performance improvements across the board:
			//       only degenerate use-cases (that is, *extremely* long strings) were a tiny bit faster.
#	define RENDER_GLYPH()                                                                                                         \
		/* NOTE: We only need to loop on the base glyph's dimensions (i.e., the bitmap resolution), */                         \
		/*       and from there compute the extra pixels for that single input pixel given our scaling factor... */            \
		if (!fbink_cfg->is_overlay && !fbink_cfg->is_bgless && !fbink_cfg->is_fgless) {                                        \
			for (uint8_t y = 0U; y < glyphHeight; y++) {                                                                   \
				/* y: input row, j: first output row after scaling */                                                  \
				j                         = (unsigned short int) (y * FONTSIZE_MULT);                                  \
				cy                        = (unsigned short int) (y_offs + j);                                         \
				/* First column might be fg or bg, but we're precomputing it anyway */                                 \
				uint8_t px_count          = 1U;                                                                        \
				/* We'll need to remember whether the previous pixel was already using the same color... */            \
				/* 1 is fg, 0 is bg, first pixel could be either, so, -1 */                                            \
				int8_t  last_px_type      = -1;                                                                        \
				/* We're already pre-computing the first column below, so start with this false */                     \
				bool    initial_stripe_px = false;                                                                     \
				/* Precompute the initial coordinates for the first column of that glyph row */                        \
				i                         = 0U;                                                                        \
				cx                        = x_offs;                                                                    \
				for (uint8_t x = 0U; x < glyphWidth; x++) {                                                            \
					/* Each element encodes a full row, we access a column's bit in that row by shifting. */       \
					if (bitmap[y] & 1U << x) {                                                                     \
						/* bit was set, pixel is fg! */                                                        \
						if (last_px_type == 1) {                                                               \
							/* Continuation of a fg color stripe */                                        \
							px_count++;                                                                    \
							initial_stripe_px = false;                                                     \
						} else if (last_px_type == 0) {                                                        \
							/* Handle scaling by drawing a FONTSIZE_MULT pixels high rectangle, batched */ \
							/* in a FONTSIZE_MULT * px_count wide stripe per same-color streak ;) */       \
							/* Note that we're printing the *previous* color's stripe, so, bg! */          \
							(*fxpFillRectChecked)(                                                         \
							    cx,                                                                        \
							    cy,                                                                        \
							    (unsigned short int) (FONTSIZE_MULT * px_count),                           \
							    FONTSIZE_MULT,                                                             \
							    &bgP);                                                                     \
							/* Which means we're already one pixel deep into a new stripe */               \
							px_count          = 1U;                                                        \
							initial_stripe_px = true;                                                      \
						}                                                                                      \
						last_px_type = 1;                                                                      \
					} else {                                                                                       \
						/* bit was unset, pixel is bg */                                                       \
						if (last_px_type == 0) {                                                               \
							/* Continuation of a bg color stripe */                                        \
							px_count++;                                                                    \
							initial_stripe_px = false;                                                     \
						} else if (last_px_type == 1) {                                                        \
							/* Note that we're printing the *previous* color's stripe, so, fg! */          \
							(*fxpFillRectChecked)(                                                         \
							    cx,                                                                        \
							    cy,                                                                        \
							    (unsigned short int) (FONTSIZE_MULT * px_count),                           \
							    FONTSIZE_MULT,                                                             \
							    &fgP);                                                                     \
							px_count          = 1U;                                                        \
							initial_stripe_px = true;                                                      \
						}                                                                                      \
						last_px_type = 0;                                                                      \
					}                                                                                              \
					/* If we're the first pixel of a new stripe, compute the coordinates of the stripe's start */  \
					if (initial_stripe_px) {                                                                       \
						/* x: input column, i: first output column after scaling */                            \
						i  = (unsigned short int) (x * FONTSIZE_MULT);                                         \
						/* Initial coordinates, before we generate the extra pixels from the scaling factor */ \
						cx = (unsigned short int) (x_offs + i);                                                \
					}                                                                                              \
				}                                                                                                      \
				/* Draw the final fg stripe of the glyph row no matter what */                                         \
				/* If last_px_type != -1, we're sure px_count > 0U ;) */                                               \
				if (last_px_type == 1) {                                                                               \
					(*fxpFillRectChecked)(cx,                                                                      \
							      cy,                                                                      \
							      (unsigned short int) (FONTSIZE_MULT * px_count),                         \
							      FONTSIZE_MULT,                                                           \
							      &fgP);                                                                   \
				} else if (last_px_type == 0) {                                                                        \
					(*fxpFillRectChecked)(cx,                                                                      \
							      cy,                                                                      \
							      (unsigned short int) (FONTSIZE_MULT * px_count),                         \
							      FONTSIZE_MULT,                                                           \
							      &bgP);                                                                   \
				}                                                                                                      \
			}                                                                                                              \
		} else {                                                                                                               \
			FBInkPixel fbP     = { 0U };                                                                                   \
			bool       is_fgpx = false;                                                                                    \
			for (uint8_t y = 0U; y < glyphHeight; y++) {                                                                   \
				/* y: input row, j: first output row after scaling */                                                  \
				j  = (unsigned short int) (y * FONTSIZE_MULT);                                                         \
				cy = (unsigned short int) (y_offs + j);                                                                \
				for (uint8_t x = 0U; x < glyphWidth; x++) {                                                            \
					/* x: input column, i: first output column after scaling */                                    \
					i = (unsigned short int) (x * FONTSIZE_MULT);                                                  \
					/* Each element encodes a full row, we access a column's bit in that row by shifting. */       \
					if (bitmap[y] & 1U << x) {                                                                     \
						/* bit was set, pixel is fg! */                                                        \
						pxP     = &fgP;                                                                        \
						is_fgpx = true;                                                                        \
					} else {                                                                                       \
						/* bit was unset, pixel is bg */                                                       \
						pxP     = &bgP;                                                                        \
						is_fgpx = false;                                                                       \
					}                                                                                              \
					/* Initial coordinates, before we generate the extra pixels from the scaling factor */         \
					cx = (unsigned short int) (x_offs + i);                                                        \
					/* NOTE: Apply our scaling factor in both dimensions! */                                       \
					for (uint8_t l = 0U; l < FONTSIZE_MULT; l++) {                                                 \
						coords.y = (unsigned short int) (cy + l);                                              \
						for (uint8_t k = 0U; k < FONTSIZE_MULT; k++) {                                         \
							coords.x = (unsigned short int) (cx + k);                                      \
							/* In overlay mode, we only print foreground pixels, */                        \
							/* and we print in the inverse color of the underlying pixel's */              \
							/* Obviously, the closer we get to GRAY7, the less contrast we get */          \
							if (is_fgpx && !fbink_cfg->is_fgless) {                                        \
								if (fbink_cfg->is_overlay) {                                           \
									get_pixel(coords, &fbP);                                       \
									fbP.bgra.p ^= 0x00FFFFFFu;                                     \
									pxP = &fbP;                                                    \
									put_pixel(coords, pxP, false);                                 \
								} else {                                                               \
									put_pixel(coords, pxP, true);                                  \
								}                                                                      \
							} else if (!is_fgpx && fbink_cfg->is_fgless) {                                 \
								put_pixel(coords, pxP, true);                                          \
							}                                                                              \
						}                                                                                      \
					}                                                                                              \
				}                                                                                                      \
			}                                                                                                              \
		}

			// Fast-path through spaces, which are always going to be a FONTWxFONTH bg rectangle.
			if (ch == 0x20u) {
				// Unless we're not printing bg pixels, of course ;).
				if (!fbink_cfg->is_overlay && !fbink_cfg->is_bgless) {
					(*fxpFillRectChecked)(x_offs, y_offs, FONTW, FONTH, &bgP);
				}
			} else {
				// Get the glyph's pixmap (width <= 8 -> uint8_t)
				const unsigned char* restrict bitmap = NULL;
#	ifdef FBINK_WITH_FONTS
				bitmap = (*fxpFont8xGetBitmap)(ch);
#	else
			bitmap = font8x8_get_bitmap(ch);
#	endif
				RENDER_GLYPH();
			}
			// NOTE: If we did not mirror the bitmasks during conversion,
			//       another approach to the fact that Unifont's hex format encodes columns in the reverse order
			//       is simply to access columns in the reverse order ;).
			//       i.e., switch the inner column loop to:
			//       for (int8_t x = (glyphWidth - 1); x >= 0; x--) {
			//           i = (unsigned short int) ((glyphWidth - x) * FONTSIZE_MULT);

			// Next glyph! This serves as the source for the pen position, hence it being used as an index...
			ci++;
		}
#	ifdef FBINK_WITH_FONTS
	} else if (glyphWidth <= 16) {
		while ((ch = u8_nextchar2(text, &bi)) != 0U) {
			LOG("Char %.*zu out of %.*zu is @ byte offset %.*zu and is U+%04X (%s)",
			    pad_len,
			    (ci + 1U),
			    pad_len,
			    charcount,
			    pad_len,
			    ch_bi,
			    ch,
			    u8_cp_to_utf8(ch));

			// Update the x coordinates for this character
			const unsigned short int x_offs = (unsigned short int) (x_base_offs + (ci * FONTW));
			// Remember the next char's byte offset for next iteration's logging
			ch_bi                           = bi;

			// Fast-path through spaces, which are always going to be a FONTWxFONTH bg rectangle.
			if (ch == 0x20u) {
				// Unless we're not printing bg pixels, of course ;).
				if (!fbink_cfg->is_overlay && !fbink_cfg->is_bgless) {
					(*fxpFillRectChecked)(x_offs, y_offs, FONTW, FONTH, &bgP);
				}
			} else {
				// Get the glyph's pixmap (width <= 16 -> uint16_t)
				const uint16_t* restrict bitmap = NULL;
				bitmap                          = (*fxpFont16xGetBitmap)(ch);

				// Render, scale & plot!
				RENDER_GLYPH();
			}

			// Next glyph! This serves as the source for the pen position, hence it being used as an index...
			ci++;
		}
	} else if (glyphWidth <= 32) {
		while ((ch = u8_nextchar2(text, &bi)) != 0U) {
			LOG("Char %.*zu out of %.*zu is @ byte offset %.*zu and is U+%04X (%s)",
			    pad_len,
			    (ci + 1U),
			    pad_len,
			    charcount,
			    pad_len,
			    ch_bi,
			    ch,
			    u8_cp_to_utf8(ch));

			// Update the x coordinates for this character
			const unsigned short int x_offs = (unsigned short int) (x_base_offs + (ci * FONTW));
			// Remember the next char's byte offset for next iteration's logging
			ch_bi                           = bi;

			// Fast-path through spaces, which are always going to be a FONTWxFONTH bg rectangle.
			if (ch == 0x20u) {
				// Unless we're not printing bg pixels, of course ;).
				if (!fbink_cfg->is_overlay && !fbink_cfg->is_bgless) {
					(*fxpFillRectChecked)(x_offs, y_offs, FONTW, FONTH, &bgP);
				}
			} else {
				// Get the glyph's pixmap (width <= 32 -> uint32_t)
				const uint32_t* restrict bitmap = NULL;
				bitmap                          = (*fxpFont32xGetBitmap)(ch);

				// Render, scale & plot!
				RENDER_GLYPH();
			}

			// Next glyph! This serves as the source for the pen position, hence it being used as an index...
			ci++;
		}
		/*
	} else if (glyphWidth <= 64) {
		while ((ch = u8_nextchar2(text, &bi)) != 0U) {
			LOG("Char %.*zu out of %.*zu is @ byte offset %.*zu and is U+%04X (%s)",
			    pad_len,
			    (ci + 1U),
			    pad_len,
			    charcount,
			    pad_len,
			    ch_bi,
			    ch,
			    u8_cp_to_utf8(ch));

			// Update the x coordinates for this character
			const unsigned short int x_offs = (unsigned short int) (x_base_offs + (ci * FONTW));
			// Remember the next char's byte offset for next iteration's logging
			ch_bi = bi;

			// Fast-path through spaces, which are always going to be a FONTWxFONTH bg rectangle.
			if (ch == 0x20u) {
				// Unless we're not printing bg pixels, of course ;).
				if (!fbink_cfg->is_overlay && !fbink_cfg->is_bgless) {
					(*fxpFillRectChecked)(x_offs, y_offs, FONTW, FONTH, &bgP);
				}
			} else {
				// Get the glyph's pixmap (width <= 64 -> uint64_t)
				const uint64_t* restrict bitmap = NULL;
				bitmap = (*fxpFont64xGetBitmap)(ch);

				// Render, scale & plot!
				RENDER_GLYPH();
			}

			// Next glyph! This serves as the source for the pen position, hence it being used as an index...
			ci++;
		}
	*/
	}
#	endif    // FBINK_WITH_FONTS

	return region;
}
#endif    // FBINK_WITH_BITMAP

#ifndef FBINK_FOR_LINUX
// NOTE: Small helper function to aid with logging the exact amount of time MXCFB_WAIT_FOR_UPDATE_COMPLETE blocked...
//       The driver is using the Kernel's wait-for-completion handler,
//       which returns the amount of jiffies left until the timeout set by the caller.
//       As we don't give a rat's ass about jiffies, we need to convert 'em to milliseconds.
// NOTE: The ioctl often actually blocks slightly longer than the perceived speed of the eInk refresh. Nevertheless,
//       there is a direct correlation between the two, as can be shown by switching between waveform modes...
// NOTE: This should be fairly accurate, given USER_HZ (i.e., 100Hz -> to the dozen ms, +/- rounding).
//       This can be confirmed w/ manual timing via clock_gettime(CLOCK_MONOTONIC) ;).
// NOTE: Fun fact, waiting for a FULL update is hardly any longer than waiting for a PARTIAL one.
//       Apparently, the gist of the differences lies in the waveform mode, not the update mode or the region size.
static __attribute__((cold)) long int
    jiffies_to_ms(long int jiffies)
{
	// We need the Kernel's clock tick frequency for this, which we stored in USER_HZ during fbink_init ;).
	return (jiffies * 1000 / USER_HZ);
}

// Handle the various eInk update API quirks for the full range of HW we support...
#	if defined(FBINK_FOR_KINDLE)
// Legacy Kindle devices ([K2<->K4])
static int
    refresh_legacy(int fbfd, const struct mxcfb_rect region, bool is_flashing)
{
	struct update_area_t area = {
		.x1       = (int) region.left,
		.y1       = (int) region.top,
		.x2       = (int) (region.left + region.width),
		.y2       = (int) (region.top + region.height),
		.which_fx = is_flashing ? fx_update_full : fx_update_partial,
		.buffer   = NULL,
	};
	LOG("Area is: x1: %d, y1: %d, x2: %d, y2: %d with fx: %d", area.x1, area.y1, area.x2, area.y2, area.which_fx);

	// NOTE: Getting UPDATE_DISPLAY_AREA to actually flash seems to be less straightforward than it appears...
	//       That said, fx_update_full does behave differently than fx_update_partial, despite the lack of flash,
	//       and it really is what the framework itself uses...
	int  rv;
	bool is_fs = false;
	if (region.width == vInfo.xres && region.height == vInfo.yres) {
		// NOTE: In the hopes that UPDATE_DISPLAY is less finicky,
		//       we use it instead when area covers the full screen.
		LOG("Detected a full-screen area, upgrading to FBIO_EINK_UPDATE_DISPLAY");
		is_fs = true;
		rv    = ioctl(fbfd, FBIO_EINK_UPDATE_DISPLAY, area.which_fx);
	} else {
		rv = ioctl(fbfd, FBIO_EINK_UPDATE_DISPLAY_AREA, &area);
	}

	if (rv < 0) {
		if (is_fs) {
			PFWARN("FBIO_EINK_UPDATE_DISPLAY: %m");
		} else {
			PFWARN("FBIO_EINK_UPDATE_DISPLAY_AREA: %m");
		}
		return ERRCODE(EXIT_FAILURE);
	}

	return EXIT_SUCCESS;
}

// All mxcfb Kindle devices ([K5<->??)
static int
    wait_for_submission_kindle(int fbfd, uint32_t marker)
{
	int rv = ioctl(fbfd, MXCFB_WAIT_FOR_UPDATE_SUBMISSION, &marker);

	if (rv < 0) {
		PFWARN("MXCFB_WAIT_FOR_UPDATE_SUBMISSION: %m");
		return ERRCODE(EXIT_FAILURE);
	} else {
		if (rv == 0) {
			LOG("Update %u has already fully been submitted", marker);
		} else {
			// NOTE: Timeout is set to 5000ms
			LOG("Waited %ldms for submission of update %u", (5000 - jiffies_to_ms(rv)), marker);
		}
	}

	return EXIT_SUCCESS;
}

// Touch Kindle devices ([K5<->]KOA2)
static int
    refresh_kindle(int fbfd, const struct mxcfb_rect region, const FBInkConfig* fbink_cfg)
{
	// Handle the common waveform_mode/update_mode switcheroo...
	const uint32_t waveform_mode = (fbink_cfg->is_flashing && fbink_cfg->wfm_mode == WFM_AUTO)
					   ? get_wfm_mode(WFM_GC16)
					   : get_wfm_mode(fbink_cfg->wfm_mode);
	const uint32_t update_mode   = fbink_cfg->is_flashing ? UPDATE_MODE_FULL : UPDATE_MODE_PARTIAL;

	// NOTE: The hist_* fields are probably used to color the final decision AUTO will take,
	//       hence our conservative choices...
	struct mxcfb_update_data update = {
		.update_region         = region,
		.waveform_mode         = waveform_mode,
		.update_mode           = update_mode,
		.update_marker         = lastMarker,
		.hist_bw_waveform_mode = (waveform_mode == WAVEFORM_MODE_REAGL) ? WAVEFORM_MODE_REAGL : WAVEFORM_MODE_DU,
		.hist_gray_waveform_mode =
		    (waveform_mode == WAVEFORM_MODE_REAGL) ? WAVEFORM_MODE_REAGL : WAVEFORM_MODE_GC16_FAST,
		.temp            = TEMP_USE_AUTO,
		.flags           = (waveform_mode == WAVEFORM_MODE_REAGLD) ? EPDC_FLAG_USE_REAGLD
				   : (waveform_mode == WAVEFORM_MODE_A2)   ? EPDC_FLAG_FORCE_MONOCHROME
									   : 0U,
		.alt_buffer_data = { 0U },
	};

	if (fbink_cfg->is_nightmode && deviceQuirks.canHWInvert) {
		update.flags |= EPDC_FLAG_ENABLE_INVERSION;
	}

	// NOTE: When dithering is enabled, you generally want to get rid of FORCE_MONOCHROME, because it gets applied *first*...
	if (fbink_cfg->dithering_mode == HWD_LEGACY) {
		update.flags &= (unsigned int) ~EPDC_FLAG_FORCE_MONOCHROME;

		// And now we can deal with the algo selection :).
		if (waveform_mode == WAVEFORM_MODE_A2 || waveform_mode == WAVEFORM_MODE_DU) {
			// NOTE: Unlike other platforms, might be a Sierra Lite matrix instead of Atkinson's
			update.flags |= EPDC_FLAG_USE_DITHERING_Y1;
		} else if (waveform_mode == WAVEFORM_MODE_DU4 || waveform_mode == WAVEFORM_MODE_GL4) {
			// NOTE: That is also Kindle-specific...
			//       There's also a EPDC_FLAG_FORCE_Y2 flag.
			update.flags |= EPDC_FLAG_USE_DITHERING_Y2;
		} else {
			// NOTE: Generally much less pleasing/useful than Y1 or Y2. Actually barely visible on my PW2.
			update.flags |= EPDC_FLAG_USE_DITHERING_Y4;
		}
	}

	int rv = ioctl(fbfd, MXCFB_SEND_UPDATE, &update);

	if (rv < 0) {
		PFWARN("MXCFB_SEND_UPDATE: %m");
		if (errno == EINVAL) {
			WARN("update_region={top=%u, left=%u, width=%u, height=%u}",
			     region.top,
			     region.left,
			     region.width,
			     region.height);
		}
		return ERRCODE(EXIT_FAILURE);
	}

	LOG("hist_bw_waveform_mode is now %#03x (%s)",
	    update.hist_bw_waveform_mode,
	    kindle_wfm_to_string(update.hist_bw_waveform_mode));
	LOG("hist_gray_waveform_mode is now %#03x (%s)",
	    update.hist_gray_waveform_mode,
	    kindle_wfm_to_string(update.hist_gray_waveform_mode));
	LOG("waveform_mode is now %#03x (%s)", update.waveform_mode, kindle_wfm_to_string(update.waveform_mode));

	return EXIT_SUCCESS;
}

// Touch Kindle devices with a Pearl screen ([K5<->PW1])
static int
    wait_for_complete_kindle_pearl(int fbfd, uint32_t marker)
{
	int rv = ioctl(fbfd, MXCFB_WAIT_FOR_UPDATE_COMPLETE_PEARL, &marker);

	if (rv < 0) {
		PFWARN("MXCFB_WAIT_FOR_UPDATE_COMPLETE_PEARL: %m");
		return ERRCODE(EXIT_FAILURE);
	} else {
		if (rv == 0) {
			LOG("Update %u has already fully been completed", marker);
		} else {
			// NOTE: Timeout is set to 5000ms
			LOG("Waited %ldms for completion of update %u", (5000 - jiffies_to_ms(rv)), marker);
		}
	}

	return EXIT_SUCCESS;
}

// Touch Kindle devices with a Carta screen ([PW2<->??)
static int
    wait_for_complete_kindle(int fbfd, uint32_t marker)
{
	struct mxcfb_update_marker_data update_marker = {
		.update_marker  = marker,
		.collision_test = 0U,
	};
	int rv = ioctl(fbfd, MXCFB_WAIT_FOR_UPDATE_COMPLETE, &update_marker);

	if (rv < 0) {
		PFWARN("MXCFB_WAIT_FOR_UPDATE_COMPLETE: %m");
		return ERRCODE(EXIT_FAILURE);
	} else {
		if (rv == 0) {
			LOG("Update %u has already fully been completed", marker);
		} else {
			// NOTE: Timeout is set to 5000ms
			LOG("Waited %ldms for completion of update %u", (5000 - jiffies_to_ms(rv)), marker);
		}
	}

	return EXIT_SUCCESS;
}

// Kindle Oasis 2 & Oasis 3 ([KOA2<->KOA3])
static int
    refresh_kindle_zelda(int fbfd, const struct mxcfb_rect region, const FBInkConfig* fbink_cfg)
{
	// Handle the common waveform_mode/update_mode switcheroo...
	const uint32_t waveform_mode = (fbink_cfg->is_flashing && fbink_cfg->wfm_mode == WFM_AUTO)
					   ? get_wfm_mode(WFM_GC16)
					   : get_wfm_mode(fbink_cfg->wfm_mode);
	const uint32_t update_mode   = fbink_cfg->is_flashing ? UPDATE_MODE_FULL : UPDATE_MODE_PARTIAL;

	// Did we request legacy dithering?
	int  dithering_mode       = get_hwd_mode(fbink_cfg->dithering_mode);
	bool use_legacy_dithering = false;
	if (dithering_mode == HWD_LEGACY) {
		// Make sure we won't setup EPDC v2 dithering
		dithering_mode       = EPDC_FLAG_USE_DITHERING_PASSTHROUGH;
		// And make sure we'll setup EPDC v1 flags later
		use_legacy_dithering = true;
	}

	struct mxcfb_update_data_zelda update = {
		.update_region   = region,
		.waveform_mode   = waveform_mode,
		.update_mode     = update_mode,
		.update_marker   = lastMarker,
		.temp            = TEMP_USE_AMBIENT,
		.flags           = (waveform_mode == WAVEFORM_MODE_ZELDA_GLD16) ? EPDC_FLAG_USE_ZELDA_REGAL
				   : (waveform_mode == WAVEFORM_MODE_ZELDA_A2)  ? EPDC_FLAG_FORCE_MONOCHROME
										: 0U,
		.dither_mode     = dithering_mode,
		.quant_bit       = (dithering_mode == EPDC_FLAG_USE_DITHERING_PASSTHROUGH)                            ? 0
				   : (waveform_mode == WAVEFORM_MODE_ZELDA_A2 || waveform_mode == WAVEFORM_MODE_DU)   ? 1
				   : (waveform_mode == WAVEFORM_MODE_ZELDA_GL4 || waveform_mode == WAVEFORM_MODE_DU4) ? 3
														      : 7,
		.alt_buffer_data = { 0U },
		.hist_bw_waveform_mode =
		    (waveform_mode == WAVEFORM_MODE_ZELDA_REAGL) ? WAVEFORM_MODE_ZELDA_REAGL : WAVEFORM_MODE_DU,
		.hist_gray_waveform_mode =
		    (waveform_mode == WAVEFORM_MODE_ZELDA_REAGL) ? WAVEFORM_MODE_ZELDA_REAGL : WAVEFORM_MODE_GC16,
		.ts_pxp  = 0U,
		.ts_epdc = 0U,
	};

	if (fbink_cfg->is_nightmode && deviceQuirks.canHWInvert) {
		update.flags |= EPDC_FLAG_ENABLE_INVERSION;
	}

	// NOTE: When dithering is enabled, you generally want to get rid of FORCE_MONOCHROME, because it gets applied *first*...
	//       That'd render EPDC v1 dithering useless, and as for EPDC v2, this only yields B&W with severe patterning.
	//       It does help hide the vectorization? artefacts (i.e., the 4 visible horizontal "bands" of processing), though.
	if (dithering_mode != EPDC_FLAG_USE_DITHERING_PASSTHROUGH) {
		// EPDC v2 here, where we prefer the newer PxP alternatives, so no need to mess with the old dithering flags.
		update.flags &= (unsigned int) ~EPDC_FLAG_FORCE_MONOCHROME;
	}

	// And setup EPDC v1 dithering
	if (use_legacy_dithering) {
		if (waveform_mode == WAVEFORM_MODE_ZELDA_A2 || waveform_mode == WAVEFORM_MODE_DU) {
			update.flags |= EPDC_FLAG_USE_DITHERING_Y1;
		} else {
			// NOTE: Generally much less useful/pleasing than Y1.
			//       Then again, it's not any better with EPDC v2 dithering @ q3, either ;).
			update.flags |= EPDC_FLAG_USE_ZELDA_DITHERING_Y4;
		}
		// NOTE: EPDC_FLAG_USE_DITHERING_Y2 is gone on Zelda/Rex.
	}

	int rv = ioctl(fbfd, MXCFB_SEND_UPDATE_ZELDA, &update);

	if (rv < 0) {
		PFWARN("MXCFB_SEND_UPDATE_ZELDA: %m");
		if (errno == EINVAL) {
			WARN("update_region={top=%u, left=%u, width=%u, height=%u}",
			     region.top,
			     region.left,
			     region.width,
			     region.height);
		}
		return ERRCODE(EXIT_FAILURE);
	}

	LOG("hist_bw_waveform_mode is now %#03x (%s)",
	    update.hist_bw_waveform_mode,
	    kindle_zelda_wfm_to_string(update.hist_bw_waveform_mode));
	LOG("hist_gray_waveform_mode is now %#03x (%s)",
	    update.hist_gray_waveform_mode,
	    kindle_zelda_wfm_to_string(update.hist_gray_waveform_mode));
	LOG("waveform_mode is now %#03x (%s)", update.waveform_mode, kindle_zelda_wfm_to_string(update.waveform_mode));

	return EXIT_SUCCESS;
}

// Kindle PaperWhite 4 & Basic 3 ([PW4<->KT4])
static int
    refresh_kindle_rex(int fbfd, const struct mxcfb_rect region, const FBInkConfig* fbink_cfg)
{
	// Handle the common waveform_mode/update_mode switcheroo...
	const uint32_t waveform_mode = (fbink_cfg->is_flashing && fbink_cfg->wfm_mode == WFM_AUTO)
					   ? get_wfm_mode(WFM_GC16)
					   : get_wfm_mode(fbink_cfg->wfm_mode);
	const uint32_t update_mode   = fbink_cfg->is_flashing ? UPDATE_MODE_FULL : UPDATE_MODE_PARTIAL;

	// Did we request legacy dithering?
	int  dithering_mode       = get_hwd_mode(fbink_cfg->dithering_mode);
	bool use_legacy_dithering = false;
	if (dithering_mode == HWD_LEGACY) {
		// Make sure we won't setup EPDC v2 dithering
		dithering_mode       = EPDC_FLAG_USE_DITHERING_PASSTHROUGH;
		// And make sure we'll setup EPDC v1 flags later
		use_legacy_dithering = true;
	}

	// NOTE: Different mcfb_update_data struct (no ts_* debug fields), but otherwise, identical to the zelda one!
	struct mxcfb_update_data_rex update = {
		.update_region   = region,
		.waveform_mode   = waveform_mode,
		.update_mode     = update_mode,
		.update_marker   = lastMarker,
		.temp            = TEMP_USE_AMBIENT,
		.flags           = (waveform_mode == WAVEFORM_MODE_ZELDA_GLD16) ? EPDC_FLAG_USE_ZELDA_REGAL
				   : (waveform_mode == WAVEFORM_MODE_ZELDA_A2)  ? EPDC_FLAG_FORCE_MONOCHROME
										: 0U,
		.dither_mode     = dithering_mode,
		.quant_bit       = (dithering_mode == EPDC_FLAG_USE_DITHERING_PASSTHROUGH)                            ? 0
				   : (waveform_mode == WAVEFORM_MODE_ZELDA_A2 || waveform_mode == WAVEFORM_MODE_DU)   ? 1
				   : (waveform_mode == WAVEFORM_MODE_ZELDA_GL4 || waveform_mode == WAVEFORM_MODE_DU4) ? 3
														      : 7,
		.alt_buffer_data = { 0U },
		.hist_bw_waveform_mode =
		    (waveform_mode == WAVEFORM_MODE_ZELDA_REAGL) ? WAVEFORM_MODE_ZELDA_REAGL : WAVEFORM_MODE_DU,
		.hist_gray_waveform_mode =
		    (waveform_mode == WAVEFORM_MODE_ZELDA_REAGL) ? WAVEFORM_MODE_ZELDA_REAGL : WAVEFORM_MODE_GC16,
	};

	if (fbink_cfg->is_nightmode && deviceQuirks.canHWInvert) {
		update.flags |= EPDC_FLAG_ENABLE_INVERSION;
	}

	// NOTE: When dithering is enabled, you generally want to get rid of FORCE_MONOCHROME, because it gets applied *first*...
	//       That'd render EPDC v1 dithering useless, and as for EPDC v2, this only yields B&W with severe patterning.
	//       It does help hide the vectorization? artefacts (i.e., the 4 visible horizontal "bands" of processing), though.
	if (dithering_mode != EPDC_FLAG_USE_DITHERING_PASSTHROUGH) {
		update.flags &= (unsigned int) ~EPDC_FLAG_FORCE_MONOCHROME;
	}

	// And setup EPDC v1 dithering
	if (use_legacy_dithering) {
		if (waveform_mode == WAVEFORM_MODE_ZELDA_A2 || waveform_mode == WAVEFORM_MODE_DU) {
			update.flags |= EPDC_FLAG_USE_DITHERING_Y1;
		} else {
			// NOTE: Generally much less useful/pleasing than Y1.
			//       Then again, it's not any better with EPDC v2 dithering @ q3, either ;).
			update.flags |= EPDC_FLAG_USE_ZELDA_DITHERING_Y4;
		}
		// NOTE: EPDC_FLAG_USE_DITHERING_Y2 is gone on Zelda/Rex.
	}

	int rv = ioctl(fbfd, MXCFB_SEND_UPDATE_REX, &update);

	if (rv < 0) {
		PFWARN("MXCFB_SEND_UPDATE_REX: %m");
		if (errno == EINVAL) {
			WARN("update_region={top=%u, left=%u, width=%u, height=%u}",
			     region.top,
			     region.left,
			     region.width,
			     region.height);
		}
		return ERRCODE(EXIT_FAILURE);
	}

	LOG("hist_bw_waveform_mode is now %#03x (%s)",
	    update.hist_bw_waveform_mode,
	    kindle_zelda_wfm_to_string(update.hist_bw_waveform_mode));
	LOG("hist_gray_waveform_mode is now %#03x (%s)",
	    update.hist_gray_waveform_mode,
	    kindle_zelda_wfm_to_string(update.hist_gray_waveform_mode));
	LOG("waveform_mode is now %#03x (%s)", update.waveform_mode, kindle_zelda_wfm_to_string(update.waveform_mode));

	return EXIT_SUCCESS;
}

// Kindle PaperWhite 5, Basic 4 & Scribe ([PW5<->??)
static int
    refresh_kindle_mtk(int fbfd, const struct mxcfb_rect region, const FBInkConfig* fbink_cfg)
{
	// Handle the common waveform_mode/update_mode switcheroo...
	const uint32_t waveform_mode = (fbink_cfg->is_flashing && fbink_cfg->wfm_mode == WFM_AUTO)
					   ? get_wfm_mode(WFM_GC16)
					   : get_wfm_mode(fbink_cfg->wfm_mode);
	const uint32_t update_mode   = fbink_cfg->is_flashing ? UPDATE_MODE_FULL : UPDATE_MODE_PARTIAL;

	// NOTE: Despite the struct layout, the EPDC v2 style of dithering setup is unused,
	//       so we leave it alone here.
	// NOTE: EPDC_FLAG_USE_REGAL is also unused.
	// NOTE: The hist_ fields are also unused, we're just setting them for old time's sake ;).
	struct mxcfb_update_data_mtk update = {
		.update_region   = region,
		.waveform_mode   = waveform_mode,
		.update_mode     = update_mode,
		.update_marker   = lastMarker,
		.temp            = TEMP_USE_AMBIENT,
		.flags           = (waveform_mode == MTK_WAVEFORM_MODE_A2) ? EPDC_FLAG_FORCE_MONOCHROME : 0U,
		.dither_mode     = EPDC_FLAG_USE_DITHERING_PASSTHROUGH,
		.quant_bit       = 0,
		.alt_buffer_data = { 0U },
		.swipe_data      = { 0U },
		.hist_bw_waveform_mode =
		    (waveform_mode == MTK_WAVEFORM_MODE_REAGL) ? MTK_WAVEFORM_MODE_REAGL : MTK_WAVEFORM_MODE_DU,
		.hist_gray_waveform_mode =
		    (waveform_mode == MTK_WAVEFORM_MODE_REAGL) ? MTK_WAVEFORM_MODE_REAGL : MTK_WAVEFORM_MODE_GC16,
		.ts_pxp  = 0U,
		.ts_epdc = 0U,
	};

	// NOTE: If the intent is a true night mode (rather than a one off),
	//       the kernel prefers the fb state being set to GRAYSCALE_8BIT_INVERTED,
	//       as it's used to handle a few nightmode shenanigans...
	if (fbink_cfg->is_nightmode) {
		update.flags |= EPDC_FLAG_ENABLE_INVERSION;
	}

	// If requested, enable the refresh animation, as setup by fbink_mtk_set_swipe_data.
	if (fbink_cfg->is_animated) {
		// NOTE: The MTK Driver will crash if given an area smaller than the number of steps:
		//       i.e., if direction is L/R and w is smaller or if it is U/D and h is smaller.
		bool sane_stride = false;
		switch (mtkSwipeData.direction) {
			case MTK_SWIPE_DOWN:
			case MTK_SWIPE_UP:
				if (region.height >= mtkSwipeData.steps) {
					sane_stride = true;
				}
				break;
			case MTK_SWIPE_LEFT:
			case MTK_SWIPE_RIGHT:
				if (region.width >= mtkSwipeData.steps) {
					sane_stride = true;
				}
				break;
			default:
				break;
		}

		if (sane_stride) {
			update.swipe_data    = mtkSwipeData;
			// Leave waveform_mode on AUTO, the kernel will internally switch to REAGL & P2SW as-needed.
			update.waveform_mode = WAVEFORM_MODE_AUTO;
			update.update_mode   = UPDATE_MODE_PARTIAL;

			update.flags |= MTK_EPDC_FLAG_ENABLE_SWIPE;
		}
	}

	// NOTE: Despite what hwtcon_mdp_convert @ drivers/misc/mediatek/hwtcon_v2/hwtcon_mdp.c does,
	//       dithering doesn't seem to actually *do* anything...
	// NOTE: This *might* be fixed on Bellatrix3, with the caveat that the MDP is bypassed in some cases in pen mode
	//       (mostly DU/DUNM updates of < MIN_MDP_SIZE (6000) pixels regions at 90 or 270° rotations).
	if (fbink_cfg->dithering_mode != HWD_PASSTHROUGH) {
		// NOTE: If dither actually worked, MONOCHROME + DITHER might actually have been viable...
		update.flags &= (unsigned int) ~EPDC_FLAG_FORCE_MONOCHROME;

		// NOTE: Not much variety left, this is the only flag currently honored.
		//       Dithering is handled as part of the image processing pass by the MDP.
		// NOTE: As mentioned above, it doesn't currently appear to do anything, though...
		update.flags |= MTK_EPDC_FLAG_USE_DITHERING_Y4;
	}

	int rv = ioctl(fbfd, MXCFB_SEND_UPDATE_MTK, &update);

	if (rv < 0) {
		PFWARN("MXCFB_SEND_UPDATE_MTK: %m");
		if (errno == EINVAL) {
			WARN("update_region={top=%u, left=%u, width=%u, height=%u}",
			     region.top,
			     region.left,
			     region.width,
			     region.height);
		}
		return ERRCODE(EXIT_FAILURE);
	}

	return EXIT_SUCCESS;
}
#	elif defined(FBINK_FOR_CERVANTES)
// Cervantes devices
// All of them support MX50 "compat" ioctls, much like Kobos.
static int
    refresh_cervantes(int fbfd, const struct mxcfb_rect region, const FBInkConfig* fbink_cfg)
{
	// Handle the common waveform_mode/update_mode switcheroo...
	const uint32_t waveform_mode = (fbink_cfg->is_flashing && fbink_cfg->wfm_mode == WFM_AUTO)
					   ? get_wfm_mode(WFM_GC16)
					   : get_wfm_mode(fbink_cfg->wfm_mode);
	const uint32_t update_mode   = fbink_cfg->is_flashing ? UPDATE_MODE_FULL : UPDATE_MODE_PARTIAL;

	struct mxcfb_update_data update = {
		.update_region   = region,
		.waveform_mode   = waveform_mode,
		.update_mode     = update_mode,
		.update_marker   = lastMarker,
		.temp            = TEMP_USE_AMBIENT,
		.flags           = (waveform_mode == WAVEFORM_MODE_REAGLD) ? EPDC_FLAG_USE_AAD
				   : (waveform_mode == WAVEFORM_MODE_A2)   ? EPDC_FLAG_FORCE_MONOCHROME
									   : 0U,
		.alt_buffer_data = { 0U },
	};

	if (fbink_cfg->is_nightmode && deviceQuirks.canHWInvert) {
		update.flags |= EPDC_FLAG_ENABLE_INVERSION;
	}

	// NOTE: When dithering is enabled, you generally want to get rid of FORCE_MONOCHROME, because it gets applied *first*...
	if (fbink_cfg->dithering_mode == HWD_LEGACY) {
		update.flags &= (unsigned int) ~EPDC_FLAG_FORCE_MONOCHROME;

		// And now we can deal with the algo selection :).
		if (waveform_mode == WAVEFORM_MODE_A2 || waveform_mode == WAVEFORM_MODE_DU) {
			update.flags |= EPDC_FLAG_USE_DITHERING_Y1;
		} else if (waveform_mode == WAVEFORM_MODE_GC4) {
			// NOTE: Generally much less useful/pleasing than Y1. Then again, GC4 is an odd duck to begin with.
			update.flags |= EPDC_FLAG_USE_DITHERING_Y4;
		} else {
			// NOTE: I have no idea how this looks ;). But given the results on Kobo, I'm going to go with "subpar".
			update.flags |= EPDC_FLAG_USE_DITHERING_NTX_D8;
		}
	}

	int rv = ioctl(fbfd, MXCFB_SEND_UPDATE, &update);

	if (rv < 0) {
		PFWARN("MXCFB_SEND_UPDATE: %m");
		if (errno == EINVAL) {
			WARN("update_region={top=%u, left=%u, width=%u, height=%u}",
			     region.top,
			     region.left,
			     region.width,
			     region.height);
		}
		return ERRCODE(EXIT_FAILURE);
	}

	LOG("waveform_mode is now %#03x (%s)", update.waveform_mode, ntx_wfm_to_string(update.waveform_mode));

	return EXIT_SUCCESS;
}

// NOTE: We *could* theoretically use MXCFB_WAIT_FOR_UPDATE_COMPLETE2 on 2013+ stuff (C2+)
//       But, like on Kobo, don't bother, we'd gain nothing by switching anyway ;).
static int
    wait_for_complete_cervantes(int fbfd, uint32_t marker)
{
	int rv = ioctl(fbfd, MXCFB_WAIT_FOR_UPDATE_COMPLETE, &marker);

	if (rv < 0) {
		PFWARN("MXCFB_WAIT_FOR_UPDATE_COMPLETE: %m");
		return ERRCODE(EXIT_FAILURE);
	} else {
		if (rv == 0) {
			LOG("Update %u has already fully been completed", marker);
		} else {
			// NOTE: Timeout is set to 5000ms
			LOG("Waited %ldms for completion of update %u", (5000 - jiffies_to_ms(rv)), marker);
		}
	}

	return EXIT_SUCCESS;
}
#	elif defined(FBINK_FOR_REMARKABLE)
static int
    refresh_remarkable(int fbfd, const struct mxcfb_rect region, const FBInkConfig* fbink_cfg)
{
	// Handle the common waveform_mode/update_mode switcheroo...
	const uint32_t waveform_mode = (fbink_cfg->is_flashing && fbink_cfg->wfm_mode == WFM_AUTO)
					   ? get_wfm_mode(WFM_GC16)
					   : get_wfm_mode(fbink_cfg->wfm_mode);
	const uint32_t update_mode   = fbink_cfg->is_flashing ? UPDATE_MODE_FULL : UPDATE_MODE_PARTIAL;

	// NOTE: Actually uses the V1 epdc driver, hence dither_mode & quant_bit being unused.
	struct mxcfb_update_data update = {
		.update_region   = region,
		.waveform_mode   = waveform_mode,
		.update_mode     = update_mode,
		.update_marker   = lastMarker,
		.temp            = (waveform_mode == WAVEFORM_MODE_DU) ? TEMP_USE_REMARKABLE : TEMP_USE_AMBIENT,
		.flags           = (waveform_mode == WAVEFORM_MODE_A2) ? EPDC_FLAG_FORCE_MONOCHROME : 0U,
		.dither_mode     = 0,
		.quant_bit       = 0,
		.alt_buffer_data = { 0U }
	};

	if (fbink_cfg->is_nightmode && deviceQuirks.canHWInvert) {
		update.flags |= EPDC_FLAG_ENABLE_INVERSION;
	}

	// NOTE: When dithering is enabled, you generally want to get rid of FORCE_MONOCHROME, because it gets applied *first*...
	if (fbink_cfg->dithering_mode == HWD_LEGACY) {
		update.flags &= (unsigned int) ~EPDC_FLAG_FORCE_MONOCHROME;

		// And now we can deal with the algo selection :).
		if (waveform_mode == WAVEFORM_MODE_A2 || waveform_mode == WAVEFORM_MODE_DU) {
			update.flags |= EPDC_FLAG_USE_DITHERING_Y1;
		} else {
			// NOTE: Nothing better than Y4 is available.
			update.flags |= EPDC_FLAG_USE_DITHERING_Y4;
		}
	}

	int rv = ioctl(fbfd, MXCFB_SEND_UPDATE, &update);

	if (rv < 0) {
		PFWARN("MXCFB_SEND_UPDATE: %m");
		if (errno == EINVAL) {
			WARN("update_region={top=%u, left=%u, width=%u, height=%u}",
			     region.top,
			     region.left,
			     region.width,
			     region.height);
		}
		return ERRCODE(EXIT_FAILURE);
	}

	LOG("waveform_mode is now %#03x (%s)", update.waveform_mode, remarkable_wfm_to_string(update.waveform_mode));

	return EXIT_SUCCESS;
}

static int
    wait_for_complete_remarkable(int fbfd, uint32_t marker)
{
	struct mxcfb_update_marker_data update_data = { .update_marker = marker, .collision_test = 0U };
	int                             rv          = ioctl(fbfd, MXCFB_WAIT_FOR_UPDATE_COMPLETE, &update_data);

	if (rv < 0) {
		PFWARN("MXCFB_WAIT_FOR_UPDATE_COMPLETE: %m");
		return ERRCODE(EXIT_FAILURE);
	} else {
		if (rv == 0) {
			LOG("Update %u has already fully been completed", marker);
		} else {
			// NOTE: Timeout is set to 5000ms
			LOG("Waited %ldms for completion of update %u", (5000 - jiffies_to_ms(rv)), marker);
		}
	}

	return EXIT_SUCCESS;
}
#	elif defined(FBINK_FOR_POCKETBOOK)
static int
    refresh_pocketbook(int fbfd, const struct mxcfb_rect region, const FBInkConfig* fbink_cfg)
{
	// Handle the common waveform_mode/update_mode switcheroo...
	const uint32_t waveform_mode = (fbink_cfg->is_flashing && fbink_cfg->wfm_mode == WFM_AUTO)
					   ? get_wfm_mode(WFM_GC16)
					   : get_wfm_mode(fbink_cfg->wfm_mode);
	const uint32_t update_mode   = fbink_cfg->is_flashing ? UPDATE_MODE_FULL : UPDATE_MODE_PARTIAL;

	// NOTE: Apparently benefits from the same trick as on rM of enforcing the 24°C table for DU
	struct mxcfb_update_data update = { .update_region = region,
					    .waveform_mode = waveform_mode,
					    .update_mode   = update_mode,
					    .update_marker = lastMarker,
					    .temp          = (waveform_mode == WAVEFORM_MODE_DU) ? 24 : TEMP_USE_AMBIENT,
					    .flags         = (waveform_mode == WAVEFORM_MODE_REAGLD) ? EPDC_FLAG_USE_AAD
							     : (waveform_mode == WAVEFORM_MODE_A2) ? EPDC_FLAG_FORCE_MONOCHROME
												   : 0U,
					    .alt_buffer_data = { 0U } };

	if (fbink_cfg->is_nightmode && deviceQuirks.canHWInvert) {
		update.flags |= EPDC_FLAG_ENABLE_INVERSION;
	}

	// NOTE: When dithering is enabled, you generally want to get rid of FORCE_MONOCHROME, because it gets applied *first*...
	if (fbink_cfg->dithering_mode == HWD_LEGACY) {
		update.flags &= (unsigned int) ~EPDC_FLAG_FORCE_MONOCHROME;

		// And now we can deal with the algo selection :).
		if (waveform_mode == WAVEFORM_MODE_A2 || waveform_mode == WAVEFORM_MODE_DU) {
			update.flags |= EPDC_FLAG_USE_DITHERING_Y1;
		} else if (waveform_mode == WAVEFORM_MODE_GC4 || waveform_mode == WAVEFORM_MODE_DU4) {
			// NOTE: Generally much less useful/pleasing than Y1. Then again, GC4 & DU4 are odd ducks to begin with.
			update.flags |= EPDC_FLAG_USE_DITHERING_Y4;
		} else {
			// NOTE: Technically only available on newer kernels (PB631)...
			update.flags |= EPDC_FLAG_USE_DITHERING_NTX_D8;
		}
	}

	// NOTE: The MXCFB shim on devices with a B288 SoC does *NOT* support AUTO (and *will* throw an EINVAL).
	//       Use GC16 instead to be conservative (this appears to be what InkView itself does).
	if (deviceQuirks.isSunxi && waveform_mode == WAVEFORM_MODE_AUTO) {
		update.waveform_mode = WAVEFORM_MODE_GC16;
	}

	int rv = ioctl(fbfd, MXCFB_SEND_UPDATE, &update);

	if (rv < 0) {
		PFWARN("MXCFB_SEND_UPDATE: %m");
		if (errno == EINVAL) {
			WARN("update_region={top=%u, left=%u, width=%u, height=%u}",
			     region.top,
			     region.left,
			     region.width,
			     region.height);
		}
		return ERRCODE(EXIT_FAILURE);
	}

	LOG("waveform_mode is now %#03x (%s)", update.waveform_mode, pocketbook_wfm_to_string(update.waveform_mode));

	return EXIT_SUCCESS;
}

static int
    wait_for_complete_pocketbook(int fbfd, uint32_t marker)
{
	// NOTE: Yes, some kernels will attempt to write back to the struct,
	//       despite using an ioctl that should only read an uint32_t...
	//       c.f., https://github.com/koreader/koreader/pull/6669
	struct mxcfb_update_marker_data update_data = { .update_marker = marker, .collision_test = 0U };
	int                             rv          = ioctl(fbfd, MXCFB_WAIT_FOR_UPDATE_COMPLETE_PB, &update_data);

	if (rv < 0) {
		PFWARN("MXCFB_WAIT_FOR_UPDATE_COMPLETE_PB: %m");
		return ERRCODE(EXIT_FAILURE);
	} else {
		if (rv == 0) {
			LOG("Update %u has already fully been completed", marker);
		} else {
			// NOTE: Timeout is set to 5000ms
			LOG("Waited %ldms for completion of update %u", (5000 - jiffies_to_ms(rv)), marker);
		}
	}

	return EXIT_SUCCESS;
}
#	elif defined(FBINK_FOR_KOBO)
// Kobo devices ([Mk3<->Mk6])
static int
    refresh_kobo(int fbfd, const struct mxcfb_rect region, const FBInkConfig* fbink_cfg)
{
	// Handle the common waveform_mode/update_mode switcheroo...
	const uint32_t waveform_mode = (fbink_cfg->is_flashing && fbink_cfg->wfm_mode == WFM_AUTO)
					   ? get_wfm_mode(WFM_GC16)
					   : get_wfm_mode(fbink_cfg->wfm_mode);
	const uint32_t update_mode   = fbink_cfg->is_flashing ? UPDATE_MODE_FULL : UPDATE_MODE_PARTIAL;

	struct mxcfb_update_data_v1_ntx update = {
		.update_region   = region,
		.waveform_mode   = waveform_mode,
		.update_mode     = update_mode,
		.update_marker   = lastMarker,
		.temp            = TEMP_USE_AMBIENT,
		.flags           = (waveform_mode == WAVEFORM_MODE_REAGLD) ? EPDC_FLAG_USE_AAD
				   : (waveform_mode == WAVEFORM_MODE_A2)   ? EPDC_FLAG_FORCE_MONOCHROME
									   : 0U,
		.alt_buffer_data = { 0U },
	};

	if (fbink_cfg->is_nightmode && deviceQuirks.canHWInvert) {
		update.flags |= EPDC_FLAG_ENABLE_INVERSION;
	}

	// NOTE: This requires a Mk. 6 device, the flags are silently ignored on older devices.
	// NOTE: When dithering is enabled, you generally want to get rid of FORCE_MONOCHROME, because it gets applied *first*...
	if (fbink_cfg->dithering_mode == HWD_LEGACY) {
		update.flags &= (unsigned int) ~EPDC_FLAG_FORCE_MONOCHROME;

		// And now we can deal with the algo selection :).
		if (waveform_mode == WAVEFORM_MODE_A2 || waveform_mode == WAVEFORM_MODE_DU) {
			update.flags |= EPDC_FLAG_USE_DITHERING_Y1;
		} else if (waveform_mode == WAVEFORM_MODE_GC4) {
			// NOTE: Generally much less useful/pleasing than Y1. Then again, GC4 is an odd duck to begin with.
			update.flags |= EPDC_FLAG_USE_DITHERING_Y4;
		} else {
			// NOTE: I have no idea how this looks, as I don't have a Mk. 6 device ;).
			//       But apparently, the answer is "bad":
			//       https://github.com/baskerville/plato/issues/150#issuecomment-757547202
			update.flags |= EPDC_FLAG_USE_DITHERING_NTX_D8;
		}
	}

	int rv = ioctl(fbfd, MXCFB_SEND_UPDATE_V1_NTX, &update);

	if (rv < 0) {
		PFWARN("MXCFB_SEND_UPDATE_V1_NTX: %m");
		if (errno == EINVAL) {
			WARN("update_region={top=%u, left=%u, width=%u, height=%u}",
			     region.top,
			     region.left,
			     region.width,
			     region.height);
		}
		return ERRCODE(EXIT_FAILURE);
	}

	// NOTE: Kernel doesn't send the updated data to userland, so, so need to print it.

	return EXIT_SUCCESS;
}

static int
    wait_for_complete_kobo(int fbfd, uint32_t marker)
{
	int rv = ioctl(fbfd, MXCFB_WAIT_FOR_UPDATE_COMPLETE_V1, &marker);

	if (rv < 0) {
		PFWARN("MXCFB_WAIT_FOR_UPDATE_COMPLETE_V1: %m");
		return ERRCODE(EXIT_FAILURE);
	} else {
		if (rv == 0) {
			LOG("Update %u has already fully been completed", marker);
		} else {
			if (strcmp(deviceQuirks.devicePlatform, "Mark 4") <= 0) {
				// NOTE: Timeout is set to 5000ms on older devices
				LOG("Waited %ldms for completion of update %u", (5000 - jiffies_to_ms(rv)), marker);
			} else {
				// NOTE: Timeout is set to 10000ms
				LOG("Waited %ldms for completion of update %u", (10000 - jiffies_to_ms(rv)), marker);
			}
		}
	}

	return EXIT_SUCCESS;
}

// Kobo Mark 7+ devices on i.MX SoCs (Mk7, Mk9, Mk10)
static int
    refresh_kobo_mk7(int fbfd, const struct mxcfb_rect region, const FBInkConfig* fbink_cfg)
{
	// NOTE: On Mk. 7 devices, for reasons that are unclear (and under circumstances which are equally unclear),
	//       the EPDC may repeatedly ignore the requested flags and/or dither_mode...
	//       A fb rotation *usually* jog things up, but other events may also clear things up...
	//       Being UR @ 8bpp seems to exacerbate the issue, FWIW...
	//           e.g., @ 8bpp, a clear_screen right after the UR rotation will fairly reproducibly trigger this behavior...
	// NOTE: Setting both EPDC_FLAG_FORCE_MONOCHROME and EPDC_FLAG_ENABLE_INVERSION on non-A2/DU waveform modes also
	//       seems like a great way to trigger this...
	//       c.f., https://github.com/baskerville/plato/issues/79
	//       But even with DU, if you pile on a dither_mode or a USE_DITHERING flag on top of that, it may still go haywire.
	//       That, or simply queueing a few updates in a short time...
	//       The main culprit appears to be FORCE_MONOCHROME here, as I can't break DU without it in this case ;).
	// NOTE: This may not be limited to NTX boards, other EPDCv2 devices running on the same (or similar) SoCs
	//       and the same kernel series appear to suffer from the same quirk (e.g., Kindle Rex, possibly Zelda, too).
	// NOTE: Speaking of FORCE_MONOCHROME, see the notes below in refresh(). Like on other devices, we enforce it for A2,
	//       because it *usually* ensures the from/to B&W A2 constraints are met.
	//       Operative word being "usually", because of the flags bug... So, given its constraints,
	//       you'll want to use it on *already* completely B&W content, otherwise things risk going a bit wonky
	//       if the flags bug gets tripped.
	//       See the notes about WFM_A2 in <fbink.h> for more details.
	//       That pretty much leaves button highlights in UIs (which is precisely what nickel uses it for).
	//       If you need a more reliable alternative, prefer DU (which'll barely be any slower anyway).
	// NOTE: Another Mk. 7 quirk, that one easier to handle, appears to be some kind of alignment constraint snafu
	//       when hardware dithering is enabled: the driver ought to be taking care of it, but something goes wrong somewhere,
	//       leading to non-aligned partial refreshes possibly visibly "moving" the content by a few pixels.
	//       For more details and a viable workaround, c.f.,
	//       https://github.com/koreader/koreader-base/blob/0792aeb2a0d4c9e48cee728579fd4a9924971df0/ffi/framebuffer_mxcfb.lua#L256-L276
	// NOTE: Specifically on the Kobo Libra, we've also seen multiple reports of the WAIT_FOR_UPDATE_COMPLETE ioctl
	//       failing with a timeout in bog-standard circumstances where it doesn't really appear to have any reason to fail.
	//       c.f., https://github.com/koreader/koreader/issues/7340
	//       The issue *may* be compounded by being UR @ 8bpp, but was also reproduced in Nickel (e.g., UR @ 32bpp)...
	//       Unlike other Kobo devices, UR happens to be the native rotation of the Libra.
	//       This also happens to be a configuration that was shown to fatally upset the Clara's kernel, FWIW...
	//       c.f., https://github.com/baskerville/plato/issues/129 (for the Clara bit).

	// Handle the common waveform_mode/update_mode switcheroo...
	const uint32_t waveform_mode = (fbink_cfg->is_flashing && fbink_cfg->wfm_mode == WFM_AUTO)
					   ? get_wfm_mode(WFM_GC16)
					   : get_wfm_mode(fbink_cfg->wfm_mode);
	const uint32_t update_mode   = fbink_cfg->is_flashing ? UPDATE_MODE_FULL : UPDATE_MODE_PARTIAL;

	// Did we request legacy dithering?
	int  dithering_mode       = get_hwd_mode(fbink_cfg->dithering_mode);
	bool use_legacy_dithering = false;
	if (dithering_mode == HWD_LEGACY) {
		// Make sure we won't setup EPDC v2 dithering
		dithering_mode       = EPDC_FLAG_USE_DITHERING_PASSTHROUGH;
		// And make sure we'll setup EPDC v1 flags later
		use_legacy_dithering = true;
	}

	struct mxcfb_update_data_v2 update = {
		.update_region   = region,
		.waveform_mode   = waveform_mode,
		.update_mode     = update_mode,
		.update_marker   = lastMarker,
		.temp            = TEMP_USE_AMBIENT,
		.flags           = (waveform_mode == WAVEFORM_MODE_GLD16) ? EPDC_FLAG_USE_REGAL
				   : (waveform_mode == WAVEFORM_MODE_A2)  ? EPDC_FLAG_FORCE_MONOCHROME
									  : 0U,
		.dither_mode     = dithering_mode,
		.quant_bit       = (dithering_mode == EPDC_FLAG_USE_DITHERING_PASSTHROUGH)                      ? 0
				   : (waveform_mode == WAVEFORM_MODE_A2 || waveform_mode == WAVEFORM_MODE_DU)   ? 1
				   : (waveform_mode == WAVEFORM_MODE_GC4 || waveform_mode == WAVEFORM_MODE_DU4) ? 3
														: 7,
		.alt_buffer_data = { 0U },
	};

	if (fbink_cfg->is_nightmode && deviceQuirks.canHWInvert) {
		update.flags |= EPDC_FLAG_ENABLE_INVERSION;
	}

	// NOTE: When dithering is enabled, you generally want to get rid of FORCE_MONOCHROME, because it gets applied *first*...
	//       That'd render EPDC v1 dithering useless, and as for EPDC v2, this only yields B&W with severe patterning.
	//       It does help hide the vectorization? artefacts (i.e., the 4 visible horizontal "bands" of processing), though.
	//       Fun fact: I don't see those bands @ UR (provided I manage to get the kernel to honor the flags/dither_mode...).
	//       Unfortunately, that's not the native Portrait orientation on the Forma... (it is on the Libra, though).
	if (dithering_mode != EPDC_FLAG_USE_DITHERING_PASSTHROUGH) {
		update.flags &= (unsigned int) ~EPDC_FLAG_FORCE_MONOCHROME;
	}

	// And setup EPDC v1 dithering
	if (use_legacy_dithering) {
		if (waveform_mode == WAVEFORM_MODE_A2 || waveform_mode == WAVEFORM_MODE_DU) {
			update.flags |= EPDC_FLAG_USE_DITHERING_Y1;
		} else {
			// NOTE: Generally much less useful/pleasing than Y1.
			//       Then again, it's not any better with EPDC v2 dithering @ q3, either ;).
			update.flags |= EPDC_FLAG_USE_DITHERING_Y4;
		}
		// NOTE: EPDC_FLAG_USE_DITHERING_NTX_D8 is gone on Mk. 7.
		//       Which makes sense: ORDERED @ q7 looks pretty neat, and it's handled by the PxP.
		//       On the other hand, Y1 will generally yield more pleasing results than ORDERED @ q1,
		//       (even @ rota UR, where the q1 dithering pattern appears to be less obvious and much less glitchy).
	}

	int rv = ioctl(fbfd, MXCFB_SEND_UPDATE_V2, &update);

	if (rv < 0) {
		PFWARN("MXCFB_SEND_UPDATE_V2: %m");
		if (errno == EINVAL) {
			WARN("update_region={top=%u, left=%u, width=%u, height=%u}",
			     region.top,
			     region.left,
			     region.width,
			     region.height);
		}
		return ERRCODE(EXIT_FAILURE);
	}

	LOG("waveform_mode is now %#03x (%s)", update.waveform_mode, ntx_wfm_to_string(update.waveform_mode));

	return EXIT_SUCCESS;
}

static int
    wait_for_complete_kobo_mk7(int fbfd, uint32_t marker)
{
	struct mxcfb_update_marker_data update_marker = {
		.update_marker  = marker,
		.collision_test = 0U,
	};
	int rv = ioctl(fbfd, MXCFB_WAIT_FOR_UPDATE_COMPLETE_V3, &update_marker);

	if (rv < 0) {
		PFWARN("MXCFB_WAIT_FOR_UPDATE_COMPLETE_V3: %m");
		return ERRCODE(EXIT_FAILURE);
	} else {
		if (rv == 0) {
			LOG("Update %u has already fully been completed", marker);
		} else {
			// NOTE: Timeout is set to 5000ms
			LOG("Waited %ldms for completion of update %u", (5000 - jiffies_to_ms(rv)), marker);
		}
	}

	LOG("collision_test is now %u", update_marker.collision_test);

	return EXIT_SUCCESS;
}

// Kobo Mark 8 devices ([Mk8<->??)
static int
    refresh_kobo_sunxi(const struct mxcfb_rect region, const FBInkConfig* fbink_cfg)
{
	// NOTE: In case of issues, enable full verbosity in the DISP driver:
	//       echo 8 >| /proc/sys/kernel/printk
	//       echo 9 >| /sys/kernel/debug/dispdbg/dgblvl
	//       And running klogd so you get everything timestamped in the syslog always helps ;).
	//       "Small" caveat: it appears to make the driver *that* much buggy and crashy...
	// NOTE: Speaking of debugfs, Nickel periodically (haven't looked at the circumstances in detail)
	//       wakes up the EPDC via debugfs (e.g., name=lcd0, command=enable, start=1).
	//       Possibly related to the aggressive standby on idle? (Which I haven't looked into there, either).

	// Convert our mxcfb_rect into a sunxi area_info
	struct area_info area = { .x_top    = region.left,
				  .y_top    = region.top,
				  .x_bottom = region.left + region.width - 1,
				  .y_bottom = region.top + region.height - 1 };

	// Handle the common waveform_mode/update_mode switcheroo...
	const uint32_t waveform_mode = (fbink_cfg->is_flashing && fbink_cfg->wfm_mode == WFM_AUTO)
					   ? get_wfm_mode(WFM_GC16)
					   : get_wfm_mode(fbink_cfg->wfm_mode);

	sunxi_disp_eink_update2 update = { .area        = &area,
					   .layer_num   = 1U,
					   .update_mode = waveform_mode,
					   .lyr_cfg2    = &sunxiCtx.layer,
					   .frame_id    = &lastMarker,
					   .rotate      = &sunxiCtx.rota,
					   .cfa_use     = 0U };

	// Update mode shenanigans...
	if (!fbink_cfg->is_flashing && waveform_mode != EINK_AUTO_MODE) {
		// For some reason, AUTO shouldn't specify PARTIAL...
		// (it trips the unknown mode warning, which falls back to... plain AUTO ;)).
		update.update_mode |= EINK_PARTIAL_MODE;
	}
	if (waveform_mode == EINK_GLR16_MODE || waveform_mode == EINK_GLD16_MODE) {
		// NOTE: Fun fact: nothing currently checks this flag in the Elipsa kernel... -_-".
		update.update_mode |= EINK_REGAL_MODE;
	}
	if (waveform_mode == EINK_A2_MODE) {
		// NOTE: Unlike on mxcfb, this isn't HW assisted, this just uses the "simple" Y8->Y1 dither algorithm...
		update.update_mode |= EINK_MONOCHROME;
	}
	if (fbink_cfg->no_merge) {
		// Bypass dubious driver optimizations to ensure we will actually flash, even when submitting identical content...
		// NOTE: New in the Sage kernel on FW 4.29,
		//       c.f., the memcmp check in eink_pixel_process_thread in a !IS_NO_MERGE branch...
		update.update_mode |= EINK_NO_MERGE;
	}

	// And now for the dithering flags. They're all SW only, so, stuff 'em behind LEGACY...
	if (fbink_cfg->dithering_mode == HWD_LEGACY) {
		if (waveform_mode == EINK_DU_MODE) {
			// NOTE: A2 is flagged EINK_MONOCHROME, which actually just flags it EINK_DITHERING_SIMPLE...
			update.update_mode |= EINK_DITHERING_NTX_Y1;
		} else {
			// NOTE: Results are eerily similar to the Y1 algos... (i.e., we do *much* better).
			//       That's probably because the bitmask handling in eink_image_process_thread is fishy as hell...
			//       (The constants are slightly broken in that *every* EINK_DITHERING_?? flag
			//       includes the same bit as EINK_DITHERING_Y1,
			//       and the check starts by checking for... EINK_DITHERING_Y1 >_<").
			//       TL;DR: No matter what you request, the kernel always uses EINK_DITHERING_Y1 :(.
			update.update_mode |= EINK_DITHERING_Y4;
		}
	}

	int rv = ioctl(sunxiCtx.disp_fd, DISP_EINK_UPDATE2, &update);

	if (rv < 0) {
		// NOTE: The code flow is so funky that you can end up with a wide array of not necessarily
		//       semantically meaningful errno values...
		PFWARN("DISP_EINK_UPDATE2: %m");
		WARN("screen_win={x=%d, y=%d, width=%u, height=%u}; update_region={top=%u, left=%u, width=%u, height=%u}",
		     sunxiCtx.layer.info.screen_win.x,
		     sunxiCtx.layer.info.screen_win.y,
		     sunxiCtx.layer.info.screen_win.width,
		     sunxiCtx.layer.info.screen_win.height,
		     region.top,
		     region.left,
		     region.width,
		     region.height);
		return ERRCODE(EXIT_FAILURE);
	}

	return EXIT_SUCCESS;
}

static int
    wait_for_complete_kobo_sunxi(uint32_t marker)
{
	// Use the union to avoid passing garbage to the ioctl handler...
	sunxi_disp_eink_ioctl cmd = { .wait_for.frame_id = marker };

	int rv = ioctl(sunxiCtx.disp_fd, DISP_EINK_WAIT_FRAME_SYNC_COMPLETE, &cmd);

	if (rv < 0) {
		PFWARN("DISP_EINK_WAIT_FRAME_SYNC_COMPLETE: %m");
		return ERRCODE(EXIT_FAILURE);
	} else {
		if (rv == 0) {
			LOG("Update %u has already fully been completed", marker);
		} else {
			// NOTE: Timeout is set to 3000ms
			LOG("Waited %ldms for completion of update %u", (3000 - jiffies_to_ms(rv)), marker);
		}
	}

	return EXIT_SUCCESS;
}

// Kobo Mark 11 devices on MTK SoCs
static int
    refresh_kobo_mtk(int fbfd, const struct mxcfb_rect region, const FBInkConfig* fbink_cfg)
{
	// Handle the common waveform_mode/update_mode switcheroo...
	const uint32_t waveform_mode = (fbink_cfg->is_flashing && fbink_cfg->wfm_mode == WFM_AUTO)
					   ? get_wfm_mode(WFM_GC16)
					   : get_wfm_mode(fbink_cfg->wfm_mode);
	const uint32_t update_mode   = fbink_cfg->is_flashing ? UPDATE_MODE_FULL : UPDATE_MODE_PARTIAL;

	// NOTE: hwtcon_rect and mxcfb_rect are perfectly interhcnageable
	struct hwtcon_update_data update = {
		.update_region = {.top    = region.top,
                                  .left   = region.left,
                                  .width  = region.width,
                                  .height = region.height},
		.waveform_mode = waveform_mode,
		.update_mode   = update_mode,
		.update_marker = lastMarker,
		.flags       = 0U, // FIXME: Check if HWTCON_FLAG_FORCE_A2_OUTPUT makes sense outside of pen strokes...
		.dither_mode = 0
	};

	// NOTE: There isn't a per-update concept, fb inversion is global.
	//       The kernel makes a distinction between `enable_night_mode`/`night_mode`, which affects low-level timing and power shenanigans,
	//       and is (by default, unless you use the broken HWTCON_SET_NIGHTMODE ioctl) automatically enabled for updates
	//       using an Eclipse waveform mode (i.e., GCK16 & GLKW16) and `invert_fb`.
	//       You can use fbink_set_fb_info to poke at that internal `invert_fb` flag on its own.
	/*
	if (fbink_cfg->is_nightmode && deviceQuirks.canHWInvert) {
		// FIXME: See how well toggling invert_fb here to fake a per-update flag would work?
		// FIXME: In the same vein, handling pen mode might require clunky APIs,
		//        as it involves A2 + HWTCON_FLAG_FORCE_A2_OUTPUT +/- HWTCON_FLAG_FORCE_A2_OUTPUT_(WHITE|BLACK)...
	}
	*/

	// FIXME: Test if any of these actually work...
	// Dithering is handled as part of the image processing pass by the MDP.
	if (fbink_cfg->dithering_mode != HWD_PASSTHROUGH) {
		// FIXME: That combination might actually be viable here...
		update.flags &= (unsigned int) ~HWTCON_FLAG_FORCE_A2_OUTPUT;
		update.flags |= HWTCON_FLAG_USE_DITHERING;

		switch (fbink_cfg->dithering_mode) {
			case HWD_QUANT_ONLY:
				update.dither_mode = HWTCON_FLAG_USE_DITHERING_Y8_Y4_Q;
				break;
			case HWD_ORDERED:
				update.dither_mode = HWTCON_FLAG_USE_DITHERING_Y8_Y4_B;
				break;
			case HWD_FLOYD_STEINBERG:
			default:
				update.dither_mode = HWTCON_FLAG_USE_DITHERING_Y8_Y4_S;
				break;
		}

		// It's easy enough to swap from Y4 targets to Y1 targets: just add 0x200 ;).
		if (waveform_mode == HWTCON_WAVEFORM_MODE_A2 || waveform_mode == HWTCON_WAVEFORM_MODE_DU) {
			update.dither_mode += 0x200;
		}
	}

	int rv = ioctl(fbfd, HWTCON_SEND_UPDATE, &update);

	if (rv < 0) {
		PFWARN("HWTCON_SEND_UPDATE: %m");
		if (errno == EINVAL) {
			WARN("update_region={top=%u, left=%u, width=%u, height=%u}",
			     region.top,
			     region.left,
			     region.width,
			     region.height);
		}
		return ERRCODE(EXIT_FAILURE);
	}

	return EXIT_SUCCESS;
}

static int
    wait_for_submission_kobo_mtk(int fbfd, uint32_t marker)
{
	int rv = ioctl(fbfd, HWTCON_WAIT_FOR_UPDATE_SUBMISSION, &marker);

	if (rv < 0) {
		PFWARN("HWTCON_WAIT_FOR_UPDATE_SUBMISSION: %m");
		return ERRCODE(EXIT_FAILURE);
	} else {
		if (rv == 0) {
			LOG("Update %u has already fully been submitted", marker);
		} else {
			// NOTE: Timeout is set to 5000ms
			LOG("Waited %ldms for submission of update %u", (5000 - jiffies_to_ms(rv)), marker);
		}
	}

	return EXIT_SUCCESS;
}

static int
    wait_for_complete_kobo_mtk(int fbfd, uint32_t marker)
{
	int rv = ioctl(fbfd, HWTCON_WAIT_FOR_UPDATE_COMPLETE, &marker);

	if (rv < 0) {
		PFWARN("HWTCON_WAIT_FOR_UPDATE_COMPLETE: %m");
		return ERRCODE(EXIT_FAILURE);
	} else {
		if (rv == 0) {
			LOG("Update %u has already fully been completed", marker);
		} else {
			// NOTE: Timeout is set to 5000ms
			LOG("Waited %ldms for completion of update %u", (5000 - jiffies_to_ms(rv)), marker);
		}
	}

	return EXIT_SUCCESS;
}
#	endif    // FBINK_FOR_KINDLE
#endif            // !FBINK_FOR_LINUX

int
    fbink_sunxi_toggle_ntx_pen_mode(int fbfd UNUSED_BY_NOTKOBO, bool toggle UNUSED_BY_NOTKOBO)
{
#ifndef FBINK_FOR_KOBO
	PFWARN("This feature is not supported on your device");
	return ERRCODE(ENOSYS);
#else
	if (!deviceQuirks.isSunxi) {
		PFWARN("This feature is not supported on your device");
		return ERRCODE(ENOSYS);
	}

	bool keep_fd = true;
	if (open_fb_fd(&fbfd, &keep_fd) != EXIT_SUCCESS) {
		return ERRCODE(EXIT_FAILURE);
	}

	// Assume success, until shit happens ;)
	int rv = EXIT_SUCCESS;

	// We need a disp fd...
	if (!isFbMapped) {
		if (memmap_fb(fbfd) != EXIT_SUCCESS) {
			rv = ERRCODE(EXIT_FAILURE);
			goto cleanup;
		}
	}

	// Use the union to avoid passing garbage to the ioctl handler...
	sunxi_disp_eink_ioctl cmd = { .toggle_handw.enable = toggle };

	rv = ioctl(sunxiCtx.disp_fd, DISP_EINK_SET_NTX_HANDWRITE_ONOFF, &cmd);

	if (rv < 0) {
		PFWARN("DISP_EINK_SET_NTX_HANDWRITE_ONOFF: %m");
		rv = ERRCODE(EXIT_FAILURE);
		goto cleanup;
	}

	// Cleanup
cleanup:
	if (isFbMapped && !keep_fd) {
		unmap_fb();
	}
	if (!keep_fd) {
		close_fb(fbfd);
	}

	return rv;
#endif    // !FBINK_FOR_KOBO
}

int
    fbink_sunxi_ntx_enforce_rota(int fbfd                              UNUSED_BY_NOTKOBO,
				 SUNXI_FORCE_ROTA_INDEX_T mode         UNUSED_BY_NOTKOBO,
				 const FBInkConfig* restrict fbink_cfg UNUSED_BY_NOTKOBO)
{
#ifndef FBINK_FOR_KOBO
	PFWARN("This feature is not supported on your device");
	return ERRCODE(ENOSYS);
#else
	if (!deviceQuirks.isSunxi) {
		PFWARN("This feature is not supported on your device");
		return ERRCODE(ENOSYS);
	}

	// Assume success, until shit happens ;)
	int rv = EXIT_SUCCESS;

	// Check whether we can actually use the fbdamage modes...
	if (mode == FORCE_ROTA_CURRENT_ROTA || mode == FORCE_ROTA_CURRENT_LAYOUT || mode == FORCE_ROTA_WORKBUF) {
		if (!sunxiCtx.has_fbdamage) {
			WARN(
			    "Unsupported fbdamage mode `%hhd` passed to fbink_sunxi_ntx_enforce_rota, keeping the current value: %hhd (%s)",
			    mode,
			    sunxiCtx.force_rota,
			    sunxi_force_rota_to_string(sunxiCtx.force_rota));
			rv = ERRCODE(ENOTSUP);
			goto cleanup;
		}
	}

	switch (mode) {
		case FORCE_ROTA_CURRENT_ROTA:
		case FORCE_ROTA_CURRENT_LAYOUT:
		case FORCE_ROTA_PORTRAIT:
		case FORCE_ROTA_LANDSCAPE:
		case FORCE_ROTA_GYRO:
		case FORCE_ROTA_UR:
		case FORCE_ROTA_CW:
		case FORCE_ROTA_UD:
		case FORCE_ROTA_CCW:
		case FORCE_ROTA_WORKBUF:
			sunxiCtx.force_rota = mode;
			LOG("Set custom rotation handling mode to: %hhd (%s)",
			    sunxiCtx.force_rota,
			    sunxi_force_rota_to_string(sunxiCtx.force_rota));
			break;
		default:
			WARN(
			    "Invalid mode `%hhd` passed to fbink_sunxi_ntx_enforce_rota, keeping the current value: %hhd (%s)",
			    mode,
			    sunxiCtx.force_rota,
			    sunxi_force_rota_to_string(sunxiCtx.force_rota));
			rv = ERRCODE(EINVAL);
			goto cleanup;
	}

	// Chain an fbink_reinit to make sure the new mode takes *immediately*.
	return kobo_sunxi_reinit_check(fbfd, fbink_cfg);

cleanup:
	return rv;
#endif    // !FBINK_FOR_KOBO
}

int
    fbink_mtk_set_swipe_data(MTK_SWIPE_DIRECTION_INDEX_T direction UNUSED_BY_NOTKINDLE, uint8_t steps UNUSED_BY_NOTKINDLE)
{
#ifndef FBINK_FOR_KINDLE
	PFWARN("This feature is not supported on your device");
	return ERRCODE(ENOSYS);
#else
	if (!deviceQuirks.isMTK) {
		PFWARN("This feature is not supported on your device");
		return ERRCODE(ENOSYS);
	}

	mtkSwipeData.direction = (uint32_t) direction;
	mtkSwipeData.steps     = (uint32_t) steps;

	return EXIT_SUCCESS;
#endif    // !FBINK_FOR_KINDLE
}

int
    fbink_wait_for_any_complete(int fbfd UNUSED_BY_NOTKINDLE)
{
#ifndef FBINK_FOR_KINDLE
	PFWARN("This feature is not supported on your device");
	return ERRCODE(ENOSYS);
#else
	if (!deviceQuirks.isMTK) {
		PFWARN("This feature is not supported on your device");
		return ERRCODE(ENOSYS);
	}

	bool keep_fd = true;
	// Open the framebuffer if need be (nonblock, we'll only do ioctls)...
	if (open_fb_fd_nonblock(&fbfd, &keep_fd) != EXIT_SUCCESS) {
		return ERRCODE(EXIT_FAILURE);
	}

	// We don't care about the feature check, so just leave everything empty.
	mxcfb_markers_data markers_data = { 0U };
	int                rv           = ioctl(fbfd, MXCFB_WAIT_FOR_ANY_UPDATE_COMPLETE_MTK, &markers_data);

	if (rv < 0) {
		PFWARN("MXCFB_WAIT_FOR_ANY_UPDATE_COMPLETE_MTK: %m");
		return ERRCODE(EXIT_FAILURE);
	} else {
		if (rv == 0) {
			// NOTE: Timeout is set to 2000ms
			LOG("There weren't any pending updates");
		} else {
			// Return value is the amount of *bytes* written (a marker is an uint32_t)
			const size_t marker_count = (size_t) rv / sizeof(*markers_data.markers);
			for (size_t i = 0U; i < marker_count; i++) {
				LOG("Waited for completion of update %u (%zu of %zu)",
				    markers_data.markers[i],
				    i + 1U,
				    marker_count);
			}
		}
	}

	if (!keep_fd) {
		close_fb(fbfd);
	}

	return rv;
#endif    // !FBINK_FOR_KINDLE
}

int
    fbink_mtk_set_halftone(int fbfd                       UNUSED_BY_NOTKINDLE,
			   const FBInkRect                exclude_regions[2] UNUSED_BY_NOTKINDLE,
			   MTK_HALFTONE_MODE_INDEX_T size UNUSED_BY_NOTKINDLE)
{
#ifndef FBINK_FOR_KINDLE
	PFWARN("This feature is not supported on your device");
	return ERRCODE(ENOSYS);
#else
	if (!deviceQuirks.isMTK) {
		PFWARN("This feature is not supported on your device");
		return ERRCODE(ENOSYS);
	}

	bool keep_fd = true;
	// Open the framebuffer if need be (nonblock, we'll only do ioctls)...
	if (open_fb_fd_nonblock(&fbfd, &keep_fd) != EXIT_SUCCESS) {
		return ERRCODE(EXIT_FAILURE);
	}

	// Massage things into the proper data layout...
	struct mxcfb_halftone_data halftone_data = {
		.region[0] = {
			.top = (uint32_t) exclude_regions[0].top,
			.left = (uint32_t) exclude_regions[0].left,
			.width = (uint32_t) exclude_regions[0].width,
			.height = (uint32_t) exclude_regions[0].height,
		},
		.region[1] = {
			.top = (uint32_t) exclude_regions[1].top,
			.left = (uint32_t) exclude_regions[1].left,
			.width = (uint32_t) exclude_regions[1].width,
			.height = (uint32_t) exclude_regions[1].height,
		},
		.halftone_mode = size,
	};
	int rv = ioctl(fbfd, MXCFB_SET_HALFTONE_MTK, &halftone_data);

	if (rv < 0) {
		PFWARN("MXCFB_SET_HALFTONE_MTK: %m");
		return ERRCODE(EXIT_FAILURE);
	}

	if (!keep_fd) {
		close_fb(fbfd);
	}

	return rv;
#endif    // !FBINK_FOR_KINDLE
}

int
    fbink_mtk_toggle_auto_reagl(int fbfd UNUSED_BY_NOTKINDLE, bool toggle UNUSED_BY_NOTKINDLE)
{
#ifndef FBINK_FOR_KINDLE
	PFWARN("This feature is not supported on your device");
	return ERRCODE(ENOSYS);
#else
	if (!deviceQuirks.isMTK) {
		PFWARN("This feature is not supported on your device");
		return ERRCODE(ENOSYS);
	}

	bool keep_fd = true;
	// Open the framebuffer if need be (nonblock, we'll only do ioctls)...
	if (open_fb_fd_nonblock(&fbfd, &keep_fd) != EXIT_SUCCESS) {
		return ERRCODE(EXIT_FAILURE);
	}

	// c.f., auto_waveform_replacement @ drivers/misc/mediatek/hwtcon_v2/hwtcon_extra_feature.c,
	// this must *always* be set, and represents the default, allowing REAGL upgrades.
	uint32_t flags = (uint32_t) UPDATE_FLAGS_FAST_MODE;
	if (!toggle) {
		// NOTE: Leave the other low bits alone, they're meaningless for the kernel,
		//       just interesting to set the userland "source" of the request.
		flags |= UPDATE_FLAGS_MODE_FAST_FLAG;
	}
	int rv = ioctl(fbfd, MXCFB_SET_UPDATE_FLAGS_MTK, &flags);

	if (rv < 0) {
		PFWARN("MXCFB_SET_UPDATE_FLAGS_MTK: %m");
		return ERRCODE(EXIT_FAILURE);
	}

	if (!keep_fd) {
		close_fb(fbfd);
	}

	return rv;
#endif    // !FBINK_FOR_KINDLE
}

int
    fbink_mtk_toggle_pen_mode(int fbfd UNUSED_BY_NOTKINDLE, bool toggle UNUSED_BY_NOTKINDLE)
{
#ifndef FBINK_FOR_KINDLE
	PFWARN("This feature is not supported on your device");
	return ERRCODE(ENOSYS);
#else
	// NOTE: Technically requires a Bellatrix3, will return HWTCON_STATUS_INVALID_IOCTL_CMD (-4) otherwise.
	if (!deviceQuirks.isMTK) {
		PFWARN("This feature is not supported on your device");
		return ERRCODE(ENOSYS);
	}

	bool keep_fd = true;
	// Open the framebuffer if need be (nonblock, we'll only do ioctls)...
	if (open_fb_fd_nonblock(&fbfd, &keep_fd) != EXIT_SUCCESS) {
		return ERRCODE(EXIT_FAILURE);
	}

	uint32_t mode = toggle ? (uint32_t) EPDC_STYLUS_MODE_WITH_NO_TPS : (uint32_t) EPDC_STYLUS_MODE_DISABLED;
	int      rv   = ioctl(fbfd, MXCFB_SET_STYLUS_MODE, &mode);

	if (rv < 0) {
		PFWARN("MXCFB_SET_STYLUS_MODE: %m");
		return ERRCODE(EXIT_FAILURE);
	}

	if (!keep_fd) {
		close_fb(fbfd);
	}

	return rv;
#endif    // !FBINK_FOR_KINDLE
}

// And finally, dispatch the right refresh request for our HW...
#ifdef FBINK_FOR_LINUX
// NOP when we don't have an eInk screen ;).
static int
    refresh(int                     fbfd __attribute__((unused)),
	    const struct mxcfb_rect region __attribute__((unused)),
	    const FBInkConfig*      fbink_cfg __attribute__((unused)))
{
	return EXIT_SUCCESS;
}

static int
    refresh_compat(int                     fbfd __attribute__((unused)),
		   const struct mxcfb_rect region __attribute__((unused)),
		   bool                    no_refresh __attribute__((unused)),
		   const FBInkConfig*      fbink_cfg __attribute__((unused)))
{
	return EXIT_SUCCESS;
}
#else

static inline void
    compute_update_marker(void)
{
	// We'll want to increment the marker on each subsequent calls (for API users)...
	if (lastMarker == 0U) {
		// Seed it with our PID
		lastMarker = (uint32_t) getpid();
	} else {
		lastMarker++;
	}

	// NOTE: Make sure update_marker is valid, an invalid marker *may* hang the kernel instead of failing gracefully,
	//       depending on the device/FW...
	if (unlikely(lastMarker == 0U)) {
		lastMarker = ('F' + 'B' + 'I' + 'n' + 'k');
		// i.e.,      70  + 66  + 73  + 110 + 107
	}
}

static int
    refresh(int fbfd, const struct mxcfb_rect region, const FBInkConfig* fbink_cfg)
{
	// Were we asked to skip refreshes?
	if (fbink_cfg->no_refresh) {
		LOG("Skipping eInk refresh, as requested.");
		return EXIT_SUCCESS;
	}

	// NOTE: Discard bogus regions, they can cause a softlock on some devices.
	//       A 0x0 region is a no go on most devices, while a 1x1 region may only upset some Kindle models.
	//       Some devices even balk at 1xN or Nx1, so, catch that, too.
	if (unlikely(region.width <= 1 || region.height <= 1)) {
		WARN("Discarding bogus empty region (%ux%u) to avoid a softlock", region.width, region.height);
		return ERRCODE(EXIT_FAILURE);
	}

	// NOTE: There are also a number of hardware quirks (which got better on newer devices) related to region alignment,
	//       that the driver should already be taking care of...
	//       c.f., epdc_process_update @ mxc_epdc_fb.c or mxc_epdc_v2_fb.c
	//       If you see strays "unaligned" ... "Copying update before processing" entries in your dmesg, that's it.
	//       I'm hoping everything handles this sanely, because I really don't want to duplicate the driver's job...

#	ifdef FBINK_FOR_KINDLE
	if (deviceQuirks.isKindleLegacy) {
		return refresh_legacy(fbfd, region, fbink_cfg->is_flashing);
	}
#	endif
	// NOTE: While the fixed-cell codepath, when rendering in B&W, would be the perfect candidate for using A2 waveform mode,
	//       it requires the *on-screen* content to *already* be B&W... And, FORCE_MONOCHROME,
	//       which ought to make sure these constraints are met, is unfortunately unreliable on Kobo Mk. 7 kernels,
	//       (see the notes about the flags stacking bug in refresh_kobo_mk7)...
	//       Otherwise, offending pixels are not updated, leading to disappearing text,
	//       or weird blending/overlay effects depending on the previous screen content...
	// NOTE: And while we're on the fun quirks train: FULL never flashes w/ AUTO on (some?) Kobos,
	//       so request GC16 if we want a flash...
	// NOTE: FWIW, DU behaves properly when PARTIAL, but doesn't flash when FULL (c.f., WFM_DU in <fbink.h>).
	//       Which somewhat tracks given AUTO's behavior on Kobos, as well as on Kindles.
	//       (i.e., DU or GL16/GC16 is most likely often what AUTO will land on).

	// Update our own update marker
#	ifdef FBINK_FOR_KOBO
	if (!deviceQuirks.isSunxi) {
		// NOTE: On Sunxi, it's the *kernel* that updates the marker, not us.
		compute_update_marker();
	}
#	else
	compute_update_marker();
#	endif

#	if defined(FBINK_FOR_KINDLE)
	if (deviceQuirks.isMTK) {
		return refresh_kindle_mtk(fbfd, region, fbink_cfg);
	} else if (deviceQuirks.isKindleRex) {
		return refresh_kindle_rex(fbfd, region, fbink_cfg);
	} else if (deviceQuirks.isKindleZelda) {
		return refresh_kindle_zelda(fbfd, region, fbink_cfg);
	} else {
		return refresh_kindle(fbfd, region, fbink_cfg);
	}
#	elif defined(FBINK_FOR_CERVANTES)
	return refresh_cervantes(fbfd, region, fbink_cfg);
#	elif defined(FBINK_FOR_REMARKABLE)
	return refresh_remarkable(fbfd, region, fbink_cfg);
#	elif defined(FBINK_FOR_POCKETBOOK)
	return refresh_pocketbook(fbfd, region, fbink_cfg);
#	elif defined(FBINK_FOR_KOBO)
	if (deviceQuirks.isMTK) {
		return refresh_kobo_mtk(fbfd, region, fbink_cfg);
	} else if (deviceQuirks.isSunxi) {
		return refresh_kobo_sunxi(region, fbink_cfg);
	} else if (deviceQuirks.isKoboMk7) {
		return refresh_kobo_mk7(fbfd, region, fbink_cfg);
	} else {
		return refresh_kobo(fbfd, region, fbink_cfg);
	}
#	endif    // FBINK_FOR_KINDLE
}

// Compat variant for functions that support not using an FBInkConfig, or want to tweak the settings internally,
// on a per call basis...
static int
    refresh_compat(int fbfd, const struct mxcfb_rect region, bool no_refresh, const FBInkConfig* fbink_cfg)
{
	if (no_refresh) {
		LOG("Skipping eInk refresh, as requested.");
		return EXIT_SUCCESS;
	}

	FBInkConfig cfg = { 0 };

	// If we have an FBInkConfig, use it as a seed
	if (fbink_cfg) {
		cfg = *fbink_cfg;
	}

	// And then enforce the per-call overrides
	cfg.no_refresh = no_refresh;

	int ret = refresh(fbfd, region, &cfg);
	return ret;
}
#endif            // FBINK_FOR_LINUX

// Same thing for WAIT_FOR_UPDATE_SUBMISSION requests...
#if defined(FBINK_FOR_KINDLE) || defined(FBINK_FOR_KOBO)
static int
    wait_for_submission(int fbfd, uint32_t marker)
{
#	if defined(FBINK_FOR_KINDLE)
	// Only implemented for mxcfb Kindles...
	if (deviceQuirks.isKindleLegacy) {
		return ERRCODE(ENOSYS);
	} else {
		return wait_for_submission_kindle(fbfd, marker);
	}
#	elif defined(FBINK_FOR_KOBO)
	// Only implemented on MTK
	if (deviceQuirks.isMTK) {
		return wait_for_submission_kobo_mtk(fbfd, marker);
	} else {
		return ERRCODE(ENOSYS);
	}
#	endif
}
#endif    // FBINK_FOR_KINDLE || FBINK_FOR_KOBO

// Same thing for WAIT_FOR_UPDATE_COMPLETE requests...
#ifndef FBINK_FOR_LINUX
static int
    wait_for_complete(int fbfd, uint32_t marker)
{
#	if defined(FBINK_FOR_KINDLE)
	if (deviceQuirks.isKindleLegacy) {
		// MXCFB only ;).
		return EXIT_SUCCESS;
	} else if (deviceQuirks.isKindlePearlScreen) {
		return wait_for_complete_kindle_pearl(fbfd, marker);
	} else {
		return wait_for_complete_kindle(fbfd, marker);
	}
#	elif defined(FBINK_FOR_CERVANTES)
	return wait_for_complete_cervantes(fbfd, marker);
#	elif defined(FBINK_FOR_KOBO)
	if (deviceQuirks.isMTK) {
		return wait_for_complete_kobo_mtk(fbfd, marker);
	} else if (deviceQuirks.isSunxi) {
		return wait_for_complete_kobo_sunxi(marker);
	} else if (deviceQuirks.isKoboMk7) {
		return wait_for_complete_kobo_mk7(fbfd, marker);
	} else {
		return wait_for_complete_kobo(fbfd, marker);
	}
#	elif defined(FBINK_FOR_REMARKABLE)
	return wait_for_complete_remarkable(fbfd, marker);
#	elif defined(FBINK_FOR_POCKETBOOK)
	return wait_for_complete_pocketbook(fbfd, marker);
#	endif    // FBINK_FOR_KINDLE
}
#endif    // !FBINK_FOR_LINUX

#ifdef FBINK_FOR_KOBO
// Implementations of the various ways one can poke at the EPDC
// on NTX kernels in order to wake it up immediately instead of relying on the driver's own PM...
// TODO: Depending on how https://github.com/koreader/koreader/pull/10771 goes, see if we need deeper integration of this, e.g.:
//       * A FBInkConfig flag to run this before every refresh (possibly enforced by default (w/ log message) on unreliableWaitFor Mk7+ boards)
//       * Another one to run this before waits?
//       * Sunxi pen mode will *probably* need to ignore this for performance reasons, though...
//       * Document the quirks and the reasoning in the API doc
//       * Also document Nickel's own usage of this (e.g., the 1.5s debounced on *touch* input thing), c.f., PRs
static int
    wakeup_epdc_nop(void)
{
	return ERRCODE(ENOSYS);
}

// We know we'll only ever pass string literals to this,
// and we want to be able to use sizeof to compute the length of the string,
// so we have to go through a macro instead of an inline function...
#	define WRITE_TO_SYSFS(value, path)                                                                              \
		({                                                                                                       \
			int fd = open(path, O_WRONLY | O_CLOEXEC);                                                       \
			if (fd == -1) {                                                                                  \
				PFWARN("open(%s): %m", path);                                                            \
				return ERRCODE(EXIT_FAILURE);                                                            \
			}                                                                                                \
			size_t  bytes = sizeof(value);                                                                   \
			ssize_t nw    = write(fd, value, bytes);                                                         \
			if (nw == -1) {                                                                                  \
				PFWARN("write: %m");                                                                     \
				return ERRCODE(EXIT_FAILURE);                                                            \
			}                                                                                                \
			close(fd);                                                                                       \
                                                                                                                         \
			if ((size_t) nw == bytes) {                                                                      \
				return EXIT_SUCCESS;                                                                     \
			} else {                                                                                         \
				return ERRCODE(EXIT_FAILURE);                                                            \
			}                                                                                                \
		})

static int
    wakeup_epdc_kobo_nxp(void)
{
	// c.f., power_state_write @ drivers/video/fbdev/mxc/mxc_epdc_fake_s1d13522.c
	WRITE_TO_SYSFS("1,0", NTX_NXP_EPDC_POWER);
}

static int
    wakeup_epdc_kobo_sunxi(void)
{
	// c.f., dispdbg_process @ drivers/video/fbdev/sunxi/disp2/disp/dev_disp_debugfs.c
	WRITE_TO_SYSFS("lcd0", "/sys/kernel/debug/dispdbg/name");
	WRITE_TO_SYSFS("enable", "/sys/kernel/debug/dispdbg/command");
	WRITE_TO_SYSFS("1", "/sys/kernel/debug/dispdbg/start");
}

static int
    wakeup_epdc_kobo_mtk(void)
{
	// c.f., debug_fiti_power_control @ drivers/misc/mediatek/hwtcon/hwtcon_debug.c
	WRITE_TO_SYSFS("fiti_power 1", "/proc/hwtcon/cmd");
}
#endif    // FBINK_FOR_KOBO

// And the public API call for that mess
int
    fbink_wakeup_epdc(void)
{
#ifndef FBINK_FOR_KOBO
	// Abort silently, this thing is niche enough already...
	return ERRCODE(ENOSYS);
#else
	// We go through a function pointer instead of an if ladder because the NXP implementation is only available on the latest devices,
	// so to keep things simple, we want to handle that check during fbink_init...
	return (*fxpWakeupEpdc)();
#endif
}

static inline __attribute__((always_inline)) const char*
    get_fbdev_path(void)
{
#ifdef FBINK_FOR_LINUX
	// NOTE: FBGrab & DirectFB use "FRAMEBUFER", follow suit
	const char* fbdev = getenv("FRAMEBUFFER");
	if (fbdev && *fbdev != '\0') {
		return fbdev;
	}
#endif

	return "/dev/fb0";
}

// Open the framebuffer file & return the opened fd
int
    fbink_open(void)
{
	// Open the framebuffer file for reading and writing
	int fbfd = open(get_fbdev_path(), O_RDWR | O_CLOEXEC);
	if (fbfd == -1) {
		PFWARN("Cannot open framebuffer character device: %m");
		return ERRCODE(EXIT_FAILURE);
	}

	// NOTE: On the first fbink_open, it's too early to know whether we need to access the gyro over I²C,
	//       as we're called *before* device identification,
	//       which is why it is handled inside initialize_fbink instead, for the first fbink_init call only ;).
	//       But, because we can chain multiple fbink_open & fbink_close during the lifetime of a process,
	//       we still need to handle it here, because fbink_close would have closed it.
#if defined(FBINK_FOR_KOBO)
	if (deviceQuirks.isSunxi && sunxiCtx.force_rota < FORCE_ROTA_UR) {
		if (open_accelerometer_i2c() != EXIT_SUCCESS) {
			PFWARN("Cannot open accelerometer I²C handle, aborting");
			return ERRCODE(EXIT_FAILURE);
		}
	}
#endif

	return fbfd;
}

// Internal version of this which keeps track of whether we were fed an already opened fd or not...
static int
    open_fb_fd(int* restrict fbfd, bool* restrict keep_fd)
{
	if (*fbfd == FBFD_AUTO) {
		// If we're opening a fd now, don't keep it around.
		*keep_fd = false;
		if ((*fbfd = fbink_open()) == ERRCODE(EXIT_FAILURE)) {
			WARN("Failed to open the framebuffer character device, aborting");
			return ERRCODE(EXIT_FAILURE);
		}

#if defined(FBINK_FOR_KOBO)
		// NOTE: In case we're *not* in an FBFD_AUTO workflow,
		//       the I²C handle opened during the first fbink_init has been kept, too.
		if (deviceQuirks.isSunxi && sunxiCtx.force_rota < FORCE_ROTA_UR) {
			if (open_accelerometer_i2c() != EXIT_SUCCESS) {
				PFWARN("Cannot open accelerometer I²C handle, aborting");
				return ERRCODE(EXIT_FAILURE);
			}
		}
#endif
	}

	return EXIT_SUCCESS;
}

// And another variant that'll try to get a nonblocking fd when using FBFD_AUTO
// NOTE: Only use this for functions that don't actually need to write to the fb, and only need an fd for ioctls!
//       Generally, those won't try to mmap the fb either ;).
static int
    open_fb_fd_nonblock(int* restrict fbfd, bool* restrict keep_fd)
{
	if (*fbfd == FBFD_AUTO) {
		// If we're opening a fd now, don't keep it around.
		*keep_fd = false;
		// We only need an fd for ioctl, hence O_NONBLOCK (as per open(2)).
		*fbfd    = open(get_fbdev_path(), O_RDONLY | O_NONBLOCK | O_CLOEXEC);
		if (*fbfd == -1) {
			PFWARN("Cannot open framebuffer character device (%m), aborting");
			return ERRCODE(EXIT_FAILURE);
		}

#if defined(FBINK_FOR_KOBO)
		if (deviceQuirks.isSunxi && sunxiCtx.force_rota < FORCE_ROTA_UR) {
			if (open_accelerometer_i2c() != EXIT_SUCCESS) {
				PFWARN("Cannot open accelerometer I²C handle, aborting");
				return ERRCODE(EXIT_FAILURE);
			}
		}
#endif
	}

	return EXIT_SUCCESS;
}

static __attribute__((cold)) const char*
    fb_rotate_to_string(uint32_t rotate)
{
	switch (rotate) {
		case FB_ROTATE_UR:
			return "Upright, 0°";
		case FB_ROTATE_CW:
			return "Clockwise, 90°";
		case FB_ROTATE_UD:
			return "Upside Down, 180°";
		case FB_ROTATE_CCW:
			return "Counter Clockwise, 270°";
		default:
			return "Unknown?!";
	}
}

#ifdef FBINK_FOR_KINDLE
// einkfb handles rotation via a custom set of ioctls, with a different mapping than the Linux standard...
static __attribute__((cold)) const char*
    einkfb_orientation_to_string(orientation_t orientation)
{
	switch (orientation) {
		case orientation_portrait:
			return "Portrait, 0°";
		case orientation_portrait_upside_down:
			return "Inverted Portrait (Upside Down), 180°";
		case orientation_landscape:
			return "Landscape, 90°";
		case orientation_landscape_upside_down:
			return "Inverted Landscape (Upside Down), 270°";
		default:
			return "Unknown?!";
	}
}
#endif

#ifdef FBINK_WITH_DRAW
// Used to manually set the pen colors
static __attribute__((cold)) int
    set_pen_color(bool is_fg, bool is_y8, bool quantize, bool update, uint8_t r, uint8_t g, uint8_t b, uint8_t a)
{
	int rv = EXIT_SUCCESS;

	// Do we need to grayscale it?
	uint8_t y;
	if (is_y8) {
		// If we passed a Grayscale pixel, r = g = b ;).
		y = r;
	} else {
		// NOTE: We inline stbi__compute_y to avoid needing to depend on FBINK_WITH_IMAGE
		y = (uint8_t) (((r * 77U) + (g * 150U) + (29U * b)) >> 8U);
	}

	// Do we need to match that to the nearest palette color?
	// NOTE: Given that the eInk palette is effectively composed of every multiple of 0x11,
	//       it becomes as simple as rounding to the nearest multiple of 0x11 ;).
	//       c.f., https://stackoverflow.com/a/29557629
	//           & https://stackoverflow.com/a/38117380
	//       as well as https://stackoverflow.com/questions/3407012
	uint8_t v;
	if (quantize) {
		v = (uint8_t) (((y + 0x11u / 2U) / 0x11u) * 0x11u);
	} else {
		v = y;
	}

	// If we asked for quantization on an RGB value, we're now effectively using that as grayscale ;).
	if (quantize && !is_y8) {
		is_y8 = true;
	}

	// If it's already the current color, bail early-ish.
	// NOTE: Always make the comparison in grayscale to KISS.
	//       RGB565 would be tricky to handle as the packing is lossy,
	//       but we could arguably check RGB & RGBA properly at >= 24bpp...
	if (update) {
		if (is_fg) {
			if (v == penFGColor) {
				return OK_ALREADY_SAME;
			}
		} else {
			if (v == penBGColor) {
				return OK_ALREADY_SAME;
			}
		}
	}

	// NOTE: We're using ELOG here to be consistent w/ fbink_init, and because this affects an internal global state.
	// NOTE: We need to take into account the inverted cmap on Legacy Kindles...
#	ifdef FBINK_FOR_KINDLE
	if (deviceQuirks.isKindleLegacy) {
		if (is_fg) {
			penFGColor = v ^ 0xFFu;
			ELOG("Foreground pen color set to #%02X -> #%02X", v, penFGColor);
		} else {
			penBGColor = v ^ 0xFFu;
			ELOG("Background pen color set to #%02X -> #%02X", v, penBGColor);
		}
	} else {
#	endif
		// NOTE: penFGColor/penBGColor are designed to be grayscale only,
		//       but if we passed an RGBA value, we'll actually honor it for penFGPixel & penBGPixel!
		if (is_fg) {
			penFGColor = v;
			if (is_y8) {
				ELOG("Foreground pen color set to #%02X", penFGColor);
			} else {
				ELOG("Foreground pen color set to #%02X%02X%02X%02X (grayscaled: #%02X)",
				     r,
				     g,
				     b,
				     a,
				     penFGColor);
			}
		} else {
			penBGColor = v;
			if (is_y8) {
				ELOG("Background pen color set to #%02X", penBGColor);
			} else {
				ELOG("Background pen color set to #%02X%02X%02X%02X (grayscaled: #%02X)",
				     r,
				     g,
				     b,
				     a,
				     penBGColor);
			}
		}
#	ifdef FBINK_FOR_KINDLE
	}
#	endif

	// Pack the pen colors into the appropriate pixel format...
	switch (vInfo.bits_per_pixel) {
		case 4U:
			if (is_fg) {
				penFGPixel.gray8 = penFGColor;
			} else {
				penBGPixel.gray8 = penBGColor;
			}
			break;
		case 8U:
			if (is_fg) {
				penFGPixel.gray8 = penFGColor;
			} else {
				penBGPixel.gray8 = penBGColor;
			}
			break;
		case 16U:
			if (is_fg) {
				if (is_y8) {
					penFGPixel.rgb565 = pack_rgb565(penFGColor, penFGColor, penFGColor);
				} else {
					penFGPixel.rgb565 = pack_rgb565(r, g, b);
				}
			} else {
				if (is_y8) {
					penBGPixel.rgb565 = pack_rgb565(penBGColor, penBGColor, penBGColor);
				} else {
					penBGPixel.rgb565 = pack_rgb565(r, g, b);
				}
			}
			break;
		case 24U:
			if (is_fg) {
				if (is_y8) {
					penFGPixel.bgra.color.r = penFGPixel.bgra.color.g = penFGPixel.bgra.color.b =
					    penFGColor;
				} else {
					penFGPixel.bgra.color.r = r;
					penFGPixel.bgra.color.g = g;
					penFGPixel.bgra.color.b = b;
				}
			} else {
				if (is_y8) {
					penBGPixel.bgra.color.r = penBGPixel.bgra.color.g = penBGPixel.bgra.color.b =
					    penBGColor;
				} else {
					penBGPixel.bgra.color.r = r;
					penBGPixel.bgra.color.g = g;
					penBGPixel.bgra.color.b = b;
				}
			}
			break;
		case 32U:
			if (is_fg) {
				if (is_y8) {
					penFGPixel.bgra.color.a = 0xFFu;
					penFGPixel.bgra.color.r = penFGPixel.bgra.color.g = penFGPixel.bgra.color.b =
					    penFGColor;
				} else {
					penFGPixel.bgra.color.r = r;
					penFGPixel.bgra.color.g = g;
					penFGPixel.bgra.color.b = b;
					penFGPixel.bgra.color.a = a;
				}
			} else {
				if (is_y8) {
					penBGPixel.bgra.color.a = 0xFFu;
					penBGPixel.bgra.color.r = penBGPixel.bgra.color.g = penBGPixel.bgra.color.b =
					    penBGColor;
				} else {
					penBGPixel.bgra.color.r = r;
					penBGPixel.bgra.color.g = g;
					penBGPixel.bgra.color.b = b;
					penBGPixel.bgra.color.a = a;
				}
			}
			break;
		default:
			// Huh oh... Should never happen!
			WARN("Unsupported framebuffer bpp");
			rv = ERRCODE(EXIT_FAILURE);
			break;
	}

	return rv;
}
#endif    // FBINK_WITH_DRAW

// Public wrappers around set_pen_color
int
    fbink_set_fg_pen_gray(uint8_t y UNUSED_BY_NODRAW, bool quantize UNUSED_BY_NODRAW, bool update UNUSED_BY_NODRAW)
{
#ifdef FBINK_WITH_DRAW
	return set_pen_color(true, true, quantize, update, y, y, y, 0xFFu);
#else
	WARN("Drawing primitives are disabled in this FBInk build");
	return ERRCODE(ENOSYS);
#endif
}

int
    fbink_set_fg_pen_rgba(uint8_t r     UNUSED_BY_NODRAW,
			  uint8_t g     UNUSED_BY_NODRAW,
			  uint8_t b     UNUSED_BY_NODRAW,
			  uint8_t a     UNUSED_BY_NODRAW,
			  bool quantize UNUSED_BY_NODRAW,
			  bool update   UNUSED_BY_NODRAW)
{
#ifdef FBINK_WITH_DRAW
	return set_pen_color(true, false, quantize, update, r, g, b, a);
#else
	WARN("Drawing primitives are disabled in this FBInk build");
	return ERRCODE(ENOSYS);
#endif
}

int
    fbink_set_bg_pen_gray(uint8_t y UNUSED_BY_NODRAW, bool quantize UNUSED_BY_NODRAW, bool update UNUSED_BY_NODRAW)
{
#ifdef FBINK_WITH_DRAW
	return set_pen_color(false, true, quantize, update, y, y, y, 0xFFu);
#else
	WARN("Drawing primitives are disabled in this FBInk build");
	return ERRCODE(ENOSYS);
#endif
}

int
    fbink_set_bg_pen_rgba(uint8_t r     UNUSED_BY_NODRAW,
			  uint8_t g     UNUSED_BY_NODRAW,
			  uint8_t b     UNUSED_BY_NODRAW,
			  uint8_t a     UNUSED_BY_NODRAW,
			  bool quantize UNUSED_BY_NODRAW,
			  bool update   UNUSED_BY_NODRAW)
{
#ifdef FBINK_WITH_DRAW
	return set_pen_color(false, false, quantize, update, r, g, b, a);
#else
	WARN("Drawing primitives are disabled in this FBInk build");
	return ERRCODE(ENOSYS);
#endif
}

#ifdef FBINK_WITH_DRAW
// Update our internal representation of pen colors (i.e., packed into the right pixel format).
static __attribute__((cold)) int
    update_pen_colors(const FBInkConfig* restrict fbink_cfg)
{
	int rv = EXIT_SUCCESS;

	// NOTE: Now that we know which device we're running on, setup pen colors,
	//       taking into account the inverted cmap on legacy Kindles...
#	ifdef FBINK_FOR_KINDLE
	if (deviceQuirks.isKindleLegacy) {
		penFGColor = eInkBGCMap[fbink_cfg->fg_color];
		penBGColor = eInkFGCMap[fbink_cfg->bg_color];

		ELOG(
		    "Pen colors set to #%02X%02X%02X -> #%02X%02X%02X for the foreground and #%02X%02X%02X -> #%02X%02X%02X for the background",
		    eInkFGCMap[fbink_cfg->fg_color],
		    eInkFGCMap[fbink_cfg->fg_color],
		    eInkFGCMap[fbink_cfg->fg_color],
		    penFGColor,
		    penFGColor,
		    penFGColor,
		    eInkBGCMap[fbink_cfg->bg_color],
		    eInkBGCMap[fbink_cfg->bg_color],
		    eInkBGCMap[fbink_cfg->bg_color],
		    penBGColor,
		    penBGColor,
		    penBGColor);
	} else {
#	endif
		penFGColor = eInkFGCMap[fbink_cfg->fg_color];
		penBGColor = eInkBGCMap[fbink_cfg->bg_color];

		ELOG("Pen colors set to #%02X%02X%02X for the foreground and #%02X%02X%02X for the background",
		     penFGColor,
		     penFGColor,
		     penFGColor,
		     penBGColor,
		     penBGColor,
		     penBGColor);
#	ifdef FBINK_FOR_KINDLE
	}
#	endif

	// Pack the pen colors into the appropriate pixel format...
	switch (vInfo.bits_per_pixel) {
		case 4U:
			penFGPixel.gray8 = penFGColor;
			penBGPixel.gray8 = penBGColor;
			break;
		case 8U:
			penFGPixel.gray8 = penFGColor;
			penBGPixel.gray8 = penBGColor;
			break;
		case 16U:
			penFGPixel.rgb565 = pack_rgb565(penFGColor, penFGColor, penFGColor);
			penBGPixel.rgb565 = pack_rgb565(penBGColor, penBGColor, penBGColor);
			break;
		case 24U:
			penFGPixel.bgra.color.r = penFGPixel.bgra.color.g = penFGPixel.bgra.color.b = penFGColor;
			penBGPixel.bgra.color.r = penBGPixel.bgra.color.g = penBGPixel.bgra.color.b = penBGColor;
			break;
		case 32U:
			penFGPixel.bgra.color.a = 0xFFu;
			penFGPixel.bgra.color.r = penFGPixel.bgra.color.g = penFGPixel.bgra.color.b = penFGColor;
			penBGPixel.bgra.color.a                                                     = 0xFFu;
			penBGPixel.bgra.color.r = penBGPixel.bgra.color.g = penBGPixel.bgra.color.b = penBGColor;
			break;
		default:
			// Huh oh... Should never happen!
			WARN("Unsupported framebuffer bpp");
			rv = ERRCODE(EXIT_FAILURE);
			break;
	}

	return rv;
}
#endif    // FBINK_WITH_DRAW

// Update the global logging verbosity flags
static __attribute__((cold)) void
    update_verbosity(const FBInkConfig* restrict fbink_cfg)
{
	// Update verbosity flag
	if (fbink_cfg->is_verbose) {
		g_isVerbose = true;
	} else {
		g_isVerbose = false;
	}
	// Update quiet flag
	if (fbink_cfg->is_quiet) {
		g_isQuiet = true;
	} else {
		g_isQuiet = false;
	}
	// Update syslog flag
	if (fbink_cfg->to_syslog) {
		g_toSysLog = true;
	} else {
		g_toSysLog = false;
	}
}

// On some PocketBook devices, the fb setup as returned by the ioctls is... questionable at best.
// We'll fudge it back into order here.
// c.f., the similar logic in https://github.com/koreader/koreader-base/blob/50a965c28fd5ea2100257aa9ce2e62c9c301155c/ffi/framebuffer_linux.lua#L119-L189
#ifdef FBINK_FOR_POCKETBOOK
static __attribute__((cold)) void
    pocketbook_fix_fb_info(void)
{
	ELOG("Virtual resolution: %ux%u", vInfo.xres_virtual, vInfo.yres_virtual);
	// Not duplicating all the explanations here, c.f., the KOReader snippet linked earlier ;).
	if (fInfo.id[0] == '\0') {
		const uint32_t xres_virtual = vInfo.xres_virtual;
		if (!IS_ALIGNED(vInfo.xres_virtual, 32)) {
			vInfo.xres_virtual = ALIGN(vInfo.xres, 32);
			ELOG("xres_virtual -> %u", vInfo.xres_virtual);
		}
		const uint32_t yres_virtual = vInfo.yres_virtual;
		if (!IS_ALIGNED(vInfo.yres_virtual, 128)) {
			vInfo.yres_virtual = ALIGN(vInfo.yres, 128);
			ELOG("yres_virtual -> %u", vInfo.yres_virtual);
		}
		const uint32_t line_length = fInfo.line_length;
		fInfo.line_length          = (vInfo.xres_virtual * vInfo.bits_per_pixel) >> 3U;
		ELOG("line_length -> %u", fInfo.line_length);

		size_t fb_size = fInfo.line_length * vInfo.yres_virtual;
		if (fb_size > fInfo.smem_len) {
			if (!IS_ALIGNED(yres_virtual, 32)) {
				vInfo.yres_virtual = ALIGN(vInfo.yres, 32);
				ELOG("yres_virtual => %u", vInfo.yres_virtual);
			} else {
				vInfo.yres_virtual = yres_virtual;
				ELOG("yres_virtual <- %u", vInfo.yres_virtual);
			}
			fb_size = fInfo.line_length * vInfo.yres_virtual;

			if (fb_size > fInfo.smem_len) {
				fb_size           = fInfo.smem_len;
				fInfo.line_length = line_length;
				ELOG("line_length <- %u", fInfo.line_length);
				vInfo.xres_virtual = xres_virtual;
				ELOG("xres_virtual <- %u", vInfo.xres_virtual);
				vInfo.yres_virtual = yres_virtual;
				ELOG("yres_virtual <- %u", vInfo.yres_virtual);

				// Trust line_length to compute the amount of *pixels* in a scanline, visible or not,
				// because that's what we use xres_virtual for throughout the code...
				vInfo.xres_virtual = (fInfo.line_length << 3U) / vInfo.bits_per_pixel;
				ELOG("xres_virtual => %u", vInfo.xres_virtual);
			}
		}
	}

	if (deviceQuirks.deviceId == DEVICE_POCKETBOOK_COLOR_LUX) {
		vInfo.bits_per_pixel = 24U;
		vInfo.xres           = vInfo.xres / 3U;
	}
}
#endif    // FBINK_FOR_POCKETBOOK

#ifdef FBINK_FOR_KOBO
static __attribute__((cold)) void
    kobo_sunxi_fb_fixup(bool is_reinit)
{
	// If necessary, query the accelerometer to check the current rotation...
	if (!is_reinit) {
		// fbink_reinit already took care of this, so this only affects explicit fbink_init calls.
		// NOTE: Ideally, we should only affect the *first* fbink_init call, period...
		if (sunxiCtx.force_rota >= FORCE_ROTA_UR) {
			if (sunxiCtx.force_rota == FORCE_ROTA_WORKBUF) {
				// Attempt to match the working buffer...
				int rotate = query_fbdamage();
				if (rotate < 0) {
					ELOG("FBDamage is inconclusive, assuming Upright");
					rotate = FB_ROTATE_UR;
				}
				vInfo.rotate = (uint32_t) rotate;
			} else {
				vInfo.rotate = (uint32_t) sunxiCtx.force_rota;
			}
		} else {
			int rotate = query_accelerometer();
			if (rotate < 0) {
				ELOG("Accelerometer is inconclusive, assuming Upright");
				rotate = FB_ROTATE_UR;
			}
			vInfo.rotate = (uint32_t) rotate;
		}
	}
	ELOG("Canonical rotation: %u (%s)", vInfo.rotate, fb_rotate_to_string(vInfo.rotate));
	// NOTE: And because, of course, we can't have nice things, if the current working buffer
	//       (e.g., Nickel's) is laid out in a different rotation,
	//       the layer overlap detection and subsequent blending royally screws Nickel's own layer... :(.
	//       A shitty workaround might be to enable NTX_HANDWRITE and switch to DU,
	//       because it appears to disable the offending checks, but that, in turn,
	//       will leave the eink image proc kernel thread spinning at 100% CPU
	//       until the next non pen mode update...

	// Devise the required G2D rotation angle (for UPDATE ioctls), given the current "fb" rotate flag.
	// This is unfortunately not as nice and easy as usual...
	sunxiCtx.rota = ((deviceQuirks.ntxBootRota - vInfo.rotate) & 3) * 90U;

	// Handle Portrait/Landscape swaps
	const uint32_t xres = vInfo.xres;
	const uint32_t yres = vInfo.yres;
	if ((vInfo.rotate & 0x01) == 1) {
		// Odd, Landscape
		vInfo.xres = MAX(xres, yres);
		vInfo.yres = MIN(xres, yres);
	} else {
		// Even, Portrait
		vInfo.xres = MIN(xres, yres);
		vInfo.yres = MAX(xres, yres);
	}
	ELOG("Screen layout fixup (%ux%u -> %ux%u)", xres, yres, vInfo.xres, vInfo.yres);

	// Make the pitch NEON-friendly...
	// NOTE: We don't do it because it can introduce layout change glitches on rotation,
	//       or even just when layer overlap blending is involved...
	//       (e.g., it breaks the pen up update in utils/finger_trace).
	/*
	vInfo.xres_virtual = ALIGN(vInfo.xres, 32);
	ELOG("xres_virtual -> %u", vInfo.xres_virtual);
	vInfo.yres_virtual = ALIGN(vInfo.yres, 32);
	ELOG("yres_virtual -> %u", vInfo.yres_virtual);
	*/
	vInfo.xres_virtual = vInfo.xres;
	ELOG("xres_virtual -> %u", vInfo.xres_virtual);
	vInfo.yres_virtual = vInfo.yres;
	ELOG("yres_virtual -> %u", vInfo.yres_virtual);

	// Make it grayscale...
	vInfo.bits_per_pixel = 8U;
	vInfo.grayscale      = 1U;
	ELOG("bits_per_pixel -> %u", vInfo.bits_per_pixel);
	fInfo.line_length = (vInfo.xres_virtual * vInfo.bits_per_pixel) >> 3U;
	ELOG("line_length -> %u", fInfo.line_length);
	// Used by clear_screen & memmap_ion
	fInfo.smem_len = fInfo.line_length * vInfo.yres_virtual;
	ELOG("smem_len -> %u", fInfo.smem_len);
}

static __attribute__((cold)) const char*
    sunxi_force_rota_to_string(SUNXI_FORCE_ROTA_INDEX_T mode)
{
	switch (mode) {
		case FORCE_ROTA_CURRENT_ROTA:
			return "Honor gyro if it matches the current rotation";
		case FORCE_ROTA_CURRENT_LAYOUT:
			return "Honor gyro if it matches the current layout";
		case FORCE_ROTA_PORTRAIT:
			return "Honor gyro if it matches a Portrait layout";
		case FORCE_ROTA_LANDSCAPE:
			return "Honor gyro if it matches a Landscape layout";
		case FORCE_ROTA_GYRO:
			return "Honor gyro";
		case FORCE_ROTA_UR:
			return "Enforce Upright, 0°";
		case FORCE_ROTA_CW:
			return "Enforce Clockwise, 90°";
		case FORCE_ROTA_UD:
			return "Enforce Upside Down, 180°";
		case FORCE_ROTA_CCW:
			return "Enforce Counter Clockwise, 270°";
		case FORCE_ROTA_WORKBUF:
			return "Match working buffer";
		default:
			return "Unknown?!";
	}
}
#endif    // FBINK_FOR_KOBO

// Get the various fb info & setup global variables
static __attribute__((cold)) int
    initialize_fbink(int fbfd, const FBInkConfig* restrict fbink_cfg, bool skip_vinfo)
{
	// Open the framebuffer if need be (nonblock, we'll only do ioctls)...
	bool keep_fd = true;
	if (open_fb_fd_nonblock(&fbfd, &keep_fd) != EXIT_SUCCESS) {
		return ERRCODE(EXIT_FAILURE);
	}

	// Assume success, until shit happens ;)
	int rv = EXIT_SUCCESS;

	update_verbosity(fbink_cfg);

	// Start with some more generic stuff, not directly related to the framebuffer.
	// As all this stuff is pretty much set in stone, we'll only query it once.
	if (!deviceQuirks.skipId) {
#ifndef FBINK_FOR_LINUX
		// Identify the device's specific model...
		identify_device();
#	if defined(FBINK_FOR_KINDLE)
		// Most kindle support the WAIT_FOR_UPDATE_SUBMISSION ioctl...
		deviceQuirks.canWaitForSubmission = true;
		if (deviceQuirks.isKindleLegacy) {
			ELOG("Enabled Legacy einkfb Kindle quirks");
			// ... as long as they're not really old devices ;).
			deviceQuirks.canWaitForSubmission = false;
		} else if (deviceQuirks.isKindlePearlScreen) {
			ELOG("Enabled Kindle with Pearl screen quirks");
		} else if (deviceQuirks.isKindleZelda) {
			ELOG("Enabled Kindle Zelda platform quirks");
		} else if (deviceQuirks.isKindleRex) {
			ELOG("Enabled Kindle Rex platform quirks");
		}
#	elif defined(FBINK_FOR_KOBO)
		// Being able to poke at the EPDC PM is roughly a Mk.8+ thing...
		fxpWakeupEpdc = &wakeup_epdc_nop;

		if (deviceQuirks.isKoboNonMT) {
			ELOG("Enabled Kobo w/o Multi-Touch quirks");
		} else if (deviceQuirks.isKoboMk7) {
			ELOG("Enabled Kobo Mark 7 quirks");

			// This is only available on the latest boards (ca. Mk.8 and up)
			if (access(NTX_NXP_EPDC_POWER, F_OK) == 0) {
				fxpWakeupEpdc = &wakeup_epdc_kobo_nxp;
			}
		} else if (deviceQuirks.isSunxi) {
			ELOG("Enabled sunxi quirks");

			// NOTE: Check if the fbdamage kernel module is loaded,
			//       as it'll allow us to actually suss out the current rotation
			//       according to the working buffer, and not the gyro...
			// NOTE: Only checking this on startup ought to be good enough,
			//       as the module is only really truly useful when loaded very early during the boot process...
			if (access(FBDAMAGE_ROTATE_SYSFS, F_OK) == 0) {
				sunxiCtx.has_fbdamage = true;
				ELOG("Working buffer rotation sniffing available, thanks to fbdamage");
			} else {
				sunxiCtx.has_fbdamage = false;
			}

			// NOTE: Allow selectively or completely overriding the accelerometer.
			const char* force_rota = getenv("FBINK_FORCE_ROTA");
			if (force_rota) {
				// We can forgo the usual fun & games of strtol error checking,
				// as a 0 on parsing errors suits us just fine here ;).
				long int val = strtol(force_rota, NULL, 10);

				// If we're attempting to use a mode that requires fbdamage *without* fbdamage,
				// check and validate the fallback value instead...
				if (val == FORCE_ROTA_CURRENT_ROTA || val == FORCE_ROTA_CURRENT_LAYOUT ||
				    val == FORCE_ROTA_WORKBUF) {
					if (!sunxiCtx.has_fbdamage) {
						const char* force_rota_fallback = getenv("FBINK_FORCE_ROTA_FALLBACK");
						if (force_rota_fallback) {
							val = strtol(force_rota_fallback, NULL, 10);
							if (val == FORCE_ROTA_CURRENT_ROTA ||
							    val == FORCE_ROTA_CURRENT_LAYOUT ||
							    val == FORCE_ROTA_WORKBUF) {
								WARN(
								    "Attempted to use a FBINK_FORCE_ROTA_FALLBACK mode that requires fbdamage");
								val = FORCE_ROTA_NOTSUP;
							}
						} else {
							WARN(
							    "Attempted to use a FBINK_FORCE_ROTA mode that requires fbdamage without fbdamage being loaded and without a FBINK_FORCE_ROTA_FALLBACK mode set");
							val = FORCE_ROTA_NOTSUP;
						}
					}
				}

				switch (val) {
					case FORCE_ROTA_CURRENT_ROTA:
					case FORCE_ROTA_CURRENT_LAYOUT:
					case FORCE_ROTA_PORTRAIT:
					case FORCE_ROTA_LANDSCAPE:
					case FORCE_ROTA_GYRO:
					case FORCE_ROTA_UR:
					case FORCE_ROTA_CW:
					case FORCE_ROTA_UD:
					case FORCE_ROTA_CCW:
					case FORCE_ROTA_WORKBUF:
						sunxiCtx.force_rota = (SUNXI_FORCE_ROTA_INDEX_T) val;
						break;
					default:
						WARN("Invalid value `%s` for env var FBINK_FORCE_ROTA", force_rota);
						sunxiCtx.force_rota = FORCE_ROTA_GYRO;
						break;
				}
				ELOG("Requested custom rotation handling: %hhd (%s)",
				     sunxiCtx.force_rota,
				     sunxi_force_rota_to_string(sunxiCtx.force_rota));
			}

			// As the force_rota state may be updated at runtime,
			// make sure we *always* lookup the accelerometer bus/address...
			if (populate_accelerometer_i2c_info() != EXIT_SUCCESS) {
				WARN("Unable to handle rotation detection: assuming UR");
				// Make sure we won't try again
				sunxiCtx.force_rota = FORCE_ROTA_UR;
			}

			// Only open an I²C handle if we're actually going to query the gyro
			if (sunxiCtx.force_rota < FORCE_ROTA_UR) {
				// The fb fixup may require being able to poke at the accelerometer...
				if (open_accelerometer_i2c() != EXIT_SUCCESS) {
					WARN("Unable to handle rotation detection: assuming UR");
					sunxiCtx.force_rota = FORCE_ROTA_UR;
				}
			}

			fxpWakeupEpdc = &wakeup_epdc_kobo_sunxi;
		} else if (deviceQuirks.isMTK) {
			ELOG("Enabled MediaTek quirks");

			deviceQuirks.canWaitForSubmission = true;
			fxpWakeupEpdc                     = &wakeup_epdc_kobo_mtk;
		}
#	elif defined(FBINK_FOR_POCKETBOOK)
		// Check if the device is running on an AllWinner SoC instead of an NXP one...
		uint32_t aw_busy = 0U;
		// Should fail w/ ENOTTY (or EINVAL?) on NXP
		if (ioctl(fbfd, EPDC_GET_UPDATE_STATE, &aw_busy) != -1) {
			ELOG("Device appears to be running on an AW B288 SoC!");
			deviceQuirks.isSunxi = true;
		}
#	elif defined(FBINK_FOR_REMARKABLE)
		// NOTE: Check if we're running on an rM 2, in which case abort with extreme prejudice,
		//       because its kernel doesn't ship with an EPDC driver, despite running on an i.MX 7D...
		if (deviceQuirks.deviceId == DEVICE_REMARKABLE_2) {
			// ... unless we're running under the https://github.com/ddvk/remarkable2-framebuffer shim
			const char* rm2fb = getenv("RM2FB_SHIM");
			if (rm2fb) {
				ELOG(
				    "Running under the rm2fb compatibility shim (version %s), functionality may be limited",
				    rm2fb);
			} else {
				ELOG("The rM 2 kernel doesn't ship with an EPDC driver, aborting!");
				ELOG(
				    "For basic functionality, a compatibility shim is available at https://github.com/ddvk/remarkable2-framebuffer");
				// NOTE: Depending on how the fb actually behaves, it could possibly make sense to simply
				//       downgrade to FBINK_FOR_LINUX behavior instead of nothing?
				return ERRCODE(ENOSYS);
			}
		}
#	endif
#endif

		// Ask the system for its clock tick frequency so we can translate jiffies into human-readable units.
		// NOTE: This will most likely be 100, even if CONFIG_HZ is > 100
		//       c.f., sysconf(3)
		const long int rc = sysconf(_SC_CLK_TCK);
		if (rc > 0) {
			USER_HZ = rc;
			ELOG("Clock tick frequency appears to be %ld Hz", USER_HZ);
		} else {
			ELOG("Unable to query clock tick frequency, assuming %ld Hz", USER_HZ);
		}

		// Much like KOReader, assume a baseline DPI for devices where we don't specify a value in device_id
		if (deviceQuirks.screenDPI == 0U) {
#ifdef FBINK_FOR_LINUX
			// Assume non-HiDPI screens on pure Linux
			deviceQuirks.screenDPI = 96U;
#else
			// Should roughly apply to a vast majority of early Pearl screens
			deviceQuirks.screenDPI = 167U;
#endif
		}
		ELOG("Screen density set to %hu dpi", deviceQuirks.screenDPI);

		// And make sure we won't do that again ;).
		deviceQuirks.skipId = true;
	}

	// Get variable screen information (unless we were asked to skip it, because we've already populated it elsewhere)
	if (!skip_vinfo) {
		if (ioctl(fbfd, FBIOGET_VSCREENINFO, &vInfo)) {
			PFWARN("Error reading variable fb information: %m");
			rv = ERRCODE(EXIT_FAILURE);
			goto cleanup;
		}
	}
	ELOG("Variable fb info: %ux%u, %ubpp @ rotation: %u (%s)",
	     vInfo.xres,
	     vInfo.yres,
	     vInfo.bits_per_pixel,
	     vInfo.rotate,
	     fb_rotate_to_string(vInfo.rotate));
#ifdef FBINK_FOR_KINDLE
	// On einkfb, log the effective orientation, too...
	if (deviceQuirks.isKindleLegacy) {
		orientation_t orientation = orientation_portrait;
		if (ioctl(fbfd, FBIO_EINK_GET_DISPLAY_ORIENTATION, &orientation)) {
			PFWARN("FBIO_EINK_GET_DISPLAY_ORIENTATION: %m");
			rv = ERRCODE(EXIT_FAILURE);
			goto cleanup;
		}

		ELOG("Actual einkfb orientation: %u (%s)", orientation, einkfb_orientation_to_string(orientation));
	}
#endif

	// Get fixed screen information
	if (ioctl(fbfd, FBIOGET_FSCREENINFO, &fInfo)) {
		PFWARN("Error reading fixed fb information: %m");
		rv = ERRCODE(EXIT_FAILURE);
		goto cleanup;
	}
	ELOG("Fixed fb info: ID is \"%s\", length of fb mem: %u bytes & line length: %u bytes",
	     fInfo.id,
	     fInfo.smem_len,
	     fInfo.line_length);
	// NOTE: On a reinit, we're trusting that smem_len will *NOT* have changed,
	//       which thankfully appears to hold true on our target devices.
	//       Otherwise, we'd probably have to compare the previous smem_len to the new, and to
	//       mremap fbPtr if isFbMapped in case they differ (and the old smem_len != 0, which would indicate a first init).

#ifdef FBINK_FOR_POCKETBOOK
	// On PocketBook, fix the broken mess that the ioctls returns...
	pocketbook_fix_fb_info();
#endif
#ifdef FBINK_FOR_KOBO
	// Ditto on Sunxi...
	if (deviceQuirks.isSunxi) {
		kobo_sunxi_fb_fixup(skip_vinfo);
	}
#endif

	// NOTE: In most every cases, we assume (0, 0) is at the top left of the screen,
	//       and (xres, yres) at the bottom right, as we should.
	screenWidth  = vInfo.xres;
	screenHeight = vInfo.yres;

	// NOTE: This needs to be NOP by default, no matter the target device ;).
	fxpRotateCoords = &rotate_coordinates_nop;
	fxpRotateRegion = &rotate_region_nop;
#if defined(FBINK_FOR_KOBO) || defined(FBINK_FOR_CERVANTES)
	// NOTE: This applies both to Kobo & Cervantes!
	// Make sure we default to no rotation shenanigans, to avoid issues on reinit...
	deviceQuirks.isNTX16bLandscape = false;
	// NOTE: But in some very specific circumstances, that doesn't hold true...
	//       In particular, Kobos boot with a framebuffer in Landscape orientation (i.e., xres > yres),
	//       but a viewport in Portrait (the boot progress, as well as Nickel itself are presented in Portrait mode),
	//       which leads to a broken origin: (0, 0) is at the top-right of the screen
	//       (i.e., as if it were intended to be used in the same Landscape viewport as the framebuffer orientation).
	//       So we have to handle the rotation ourselves. We limit this to Kobos and a simple xres > yres check,
	//       because as we'll show, vInfo.rotate doesn't necessarily provide us with actionable info...
	// NOTE: Nickel itself will put things back into order, so this should NOT affect behavior under Nickel...
	//       Be aware that pickel, on the other hand, will forcibly drop back to this modeset!
	//       In fact, if you manage to run *before* pickel (i.e., before on-animator),
	//       you'll notice that it's in yet another rotation at very early boot (CCW?)...
	// NOTE: The Libra finally appears to have put a stop to this madness (it boots UR, with an UR panel).
	if (!deviceQuirks.isSunxi && vInfo.xres > vInfo.yres) {
		// NOTE: PW2:
		//         vInfo.rotate == 2 in Landscape (vs. 3 in Portrait mode), w/ the xres/yres switch in Landscape,
		//         and (0, 0) is always at the top-left of the viewport, so we're always correct.
		//       Kindle Touch:
		//         Doesn't propose a Landscape mode, and defaults to vInfo.rotate == 1
		//       K4:
		//         It supports the four possible rotations, and while it is always using vInfo.rotate == 0,
		//         xres & yres switch accordingly when in Landscape modes,
		//         and (0, 0) is always at the top-left of the viewport, so we're always correct.
		//         NOTE: That's because it relies on einkfb's custom orientation ioctls.
		//       TL;DR: We can't really rely on rotate to tell us anything reliably actionable, but, thankfully,
		//              we don't have to do anything extra on Kindles anyway :).
		// NOTE: The Kobos, on the other hand, at boot, are in 16bpp mode, and appear to be natively rotated CCW
		//       (or CW, depending on how you look at it...).
		//       Because we have legitimate uses in that state,
		//       (be it during the boot process, i.e., on-animator; or out-of-Nickel use cases),
		//       we attempt to handle this rotation properly, much like KOReader does.
		//       c.f., https://github.com/koreader/koreader/blob/master/frontend/device/kobo/device.lua#L32-L33
		//           & https://github.com/koreader/koreader-base/blob/master/ffi/framebuffer.lua#L74-L84
		// NOTE: See the discussion around p16 of the Plato thread for even more gory details!
		//       https://www.mobileread.com/forums/showthread.php?t=292914&page=16
		if (unlikely(vInfo.bits_per_pixel == 16U)) {
			// Correct screenWidth & screenHeight, so we do all our row/column arithmetics on the right values...
			screenWidth                    = vInfo.yres;
			screenHeight                   = vInfo.xres;
			deviceQuirks.isNTX16bLandscape = true;
			// NOTE: Here be dragons!
			//       I'm assuming that most devices follow the same pattern as far as rotation is concerned,
			//       with a few exceptions hardcoded (c.f., identify_kobo in fbink_device_id.c).
			//       Right now, the only values I'm confident about are:
			//       - the ones on the H2O, because I've got one ;).
			//       - the nickel values on other devices.
			//       - possibly the pickel values on other devices...
			//       Yay?
			// NOTE: The *name* of the rotation quirk *might* be wrong on newer devices (c.f., my notes on a Forma,
			//       in set_kobo_quirks in fbink_device_id.c),
			//       but we nonetheless currently appear to do the right thing, so just let sleeping dogs lie...
			if (vInfo.rotate == deviceQuirks.ntxBootRota) {
				// NOTE: Fun fact: on my H2O, the initial boot rotation appears to be even weirder...
				//       This rotation shenanigan was tested by running pickel, then rotating the fb via sysfs,
				//       until I got something that matched what I get during KFMon's boot process.
				//       And yet, while this rotates stuff properly in my tests, during boot, y is wrong:
				//       with row set to -5,
				//       the message is printed near the top of the screen instead of near the bottom...
				//       On the plus side, it's no longer upside down and RTL, so, progress!
				//       >_<".
				fxpRotateCoords = &rotate_coordinates_boot;
				fxpRotateRegion = &rotate_region_boot;
				ELOG("Enabled NTX @ 16bpp boot rotation quirks (%ux%u -> %ux%u)",
				     vInfo.xres,
				     vInfo.yres,
				     screenWidth,
				     screenHeight);
			} else {
				fxpRotateCoords = &rotate_coordinates_pickel;
				fxpRotateRegion = &rotate_region_pickel;
				ELOG("Enabled NTX @ 16bpp pickel rotation quirks (%ux%u -> %ux%u)",
				     vInfo.xres,
				     vInfo.yres,
				     screenWidth,
				     screenHeight);
			}
		}
	}

	// NOTE: If *any* quirky state has been detected, assume the canonical rotation is Portrait,
	//       as a hardware rotation on top of that is *highly* unlikely (also, a terrible idea).
	uint8_t canonical_rota = FB_ROTATE_UR;
#	if defined(FBINK_FOR_KOBO)
	// NOTE: fbink_rota_native_to_canonical is only implemented/tested on Kobo, so, don't do it on Cervantes.
	//       They're all in a quirky state anyway ;).
	if (!deviceQuirks.isSunxi && !deviceQuirks.isNTX16bLandscape) {
		// Otherwise, attempt to untangle it ourselves...
		canonical_rota = fbink_rota_native_to_canonical(vInfo.rotate);
		ELOG("Canonical rotation: %hhu (%s)", canonical_rota, fb_rotate_to_string(canonical_rota));
	}

	// Setup the disp layer insanity on sunxi...
	if (deviceQuirks.isSunxi) {
		// disp_layer_info2
		sunxiCtx.layer.info.mode        = LAYER_MODE_BUFFER;
		sunxiCtx.layer.info.zorder      = 0U;
		// NOTE: Ignore pixel alpha.
		//       We actually *do* handle alpha sanely, so,
		//       if we were actually using an RGB32 fb, we might want to tweak that & pre_multiply...
		sunxiCtx.layer.info.alpha_mode  = 1U;
		sunxiCtx.layer.info.alpha_value = 0xFFu;

		// disp_rect
		sunxiCtx.layer.info.screen_win.x      = 0;
		sunxiCtx.layer.info.screen_win.y      = 0;
		sunxiCtx.layer.info.screen_win.width  = vInfo.xres;
		sunxiCtx.layer.info.screen_win.height = vInfo.yres;

		sunxiCtx.layer.info.b_trd_out    = false;
		sunxiCtx.layer.info.out_trd_mode = 0;

		// disp_fb_info2
		// NOTE: fd & y8_fd are handled in mmap_ion.
		//       And they are *explicitly* set to 0 and not -1 when unused,
		//       because that's what the disp code (mostly) expects (*sigh*).

		// disp_rectsz
		// NOTE: Used in conjunction with align below.
		//       We obviously only have a single buffer, because we're not a 3D display...
		sunxiCtx.layer.info.fb.size[0].width  = vInfo.xres_virtual;
		sunxiCtx.layer.info.fb.size[0].height = vInfo.yres_virtual;
		sunxiCtx.layer.info.fb.size[1].width  = 0U;
		sunxiCtx.layer.info.fb.size[1].height = 0U;
		sunxiCtx.layer.info.fb.size[2].width  = 0U;
		sunxiCtx.layer.info.fb.size[2].height = 0U;

		// NOTE: Used to compute the scanline pitch in bytes (e.g., pitch = ALIGN(scanline_pixels * components, align).
		//       This is set to 2 by Nickel, but we appear to go by without it just fine with a Y8 fb fd...
		sunxiCtx.layer.info.fb.align[0]      = 0U;
		sunxiCtx.layer.info.fb.align[1]      = 0U;
		sunxiCtx.layer.info.fb.align[2]      = 0U;
		sunxiCtx.layer.info.fb.format        = DISP_FORMAT_8BIT_GRAY;
		sunxiCtx.layer.info.fb.color_space   = DISP_GBR_F;    // Full-range RGB
		sunxiCtx.layer.info.fb.trd_right_fd  = 0;
		sunxiCtx.layer.info.fb.pre_multiply  = true;    // Because we're using global alpha, I guess?
		sunxiCtx.layer.info.fb.crop.x        = 0;
		sunxiCtx.layer.info.fb.crop.y        = 0;
		// Don't ask me why this needs to be shifted 32 bits to the left... ¯\_(ツ)_/¯
		// NOTE: I managed to bork it during the KOReader port and it appeared to behave fine *without* the shift...
		sunxiCtx.layer.info.fb.crop.width    = (long long int) vInfo.xres << 32;
		sunxiCtx.layer.info.fb.crop.height   = (long long int) vInfo.yres << 32;
		sunxiCtx.layer.info.fb.flags         = DISP_BF_NORMAL;
		sunxiCtx.layer.info.fb.scan          = DISP_SCAN_PROGRESSIVE;
		sunxiCtx.layer.info.fb.eotf          = DISP_EOTF_GAMMA22;    // SDR
		sunxiCtx.layer.info.fb.depth         = 0;
		sunxiCtx.layer.info.fb.fbd_en        = 0U;
		sunxiCtx.layer.info.fb.metadata_fd   = 0;
		sunxiCtx.layer.info.fb.metadata_size = 0U;
		sunxiCtx.layer.info.fb.metadata_flag = 0U;

		sunxiCtx.layer.info.id = 0U;

		// disp_atw_info
		sunxiCtx.layer.info.atw.used   = false;
		sunxiCtx.layer.info.atw.mode   = 0;
		sunxiCtx.layer.info.atw.b_row  = 0;
		sunxiCtx.layer.info.atw.b_col  = 0;
		sunxiCtx.layer.info.atw.cof_fd = 0;

		sunxiCtx.layer.enable   = true;
		sunxiCtx.layer.channel  = 0U;
		// NOTE: Nickel uses layer 0, pickel layer 1.
		sunxiCtx.layer.layer_id = 1U;
	}
#	endif

	// NOTE: Well, granted, this next part is (hopefully) Kobo-specific ;).
	// Handle the Kobo viewport trickery for the few devices with hidden rows of pixels...
	if (fbink_cfg->no_viewport || deviceQuirks.koboVertOffset == 0) {
		// Device is not utterly mad, the top-left corner is at (0, 0)!
		viewWidth      = screenWidth;
		viewHoriOrigin = 0U;
		viewHeight     = screenHeight;
		viewVertOrigin = 0U;
	} else {
		// Device has a few rows of pixels hidden behind the bezel, what fun...
		switch (canonical_rota) {
			case FB_ROTATE_UR:
				viewWidth      = screenWidth;
				viewHoriOrigin = 0U;
				viewHeight     = screenHeight - (uint32_t) abs(deviceQuirks.koboVertOffset);
				if (deviceQuirks.koboVertOffset > 0) {
					// Rows of pixels are hidden at the (physical) top
					viewVertOrigin = (uint8_t) deviceQuirks.koboVertOffset;
				} else {
					// Rows of pixels are hidden at the (physical) bottom
					viewVertOrigin = 0U;
				}
				break;
			case FB_ROTATE_CW:
				viewWidth = screenWidth - (uint32_t) abs(deviceQuirks.koboVertOffset);
				if (deviceQuirks.koboVertOffset > 0) {
					// Rows of pixels are hidden at the (physical) top
					viewHoriOrigin = 0U;
				} else {
					// Rows of pixels are hidden at the (physical) bottom
					viewHoriOrigin = (uint8_t) deviceQuirks.koboVertOffset;
				}
				viewHeight     = screenHeight;
				viewVertOrigin = 0U;
				break;
			case FB_ROTATE_UD:
				viewWidth      = screenWidth;
				viewHoriOrigin = 0U;
				viewHeight     = screenHeight - (uint32_t) abs(deviceQuirks.koboVertOffset);
				if (deviceQuirks.koboVertOffset > 0) {
					// Rows of pixels are hidden at the (physical) top
					viewVertOrigin = 0U;
				} else {
					// Rows of pixels are hidden at the (physical) bottom
					viewVertOrigin = (uint8_t) deviceQuirks.koboVertOffset;
				}
				break;
			case FB_ROTATE_CCW:
				viewWidth = screenWidth - (uint32_t) abs(deviceQuirks.koboVertOffset);
				if (deviceQuirks.koboVertOffset > 0) {
					// Rows of pixels are hidden at the (physical) top
					viewHoriOrigin = (uint8_t) deviceQuirks.koboVertOffset;
				} else {
					// Rows of pixels are hidden at the (physical) bottom
					viewHoriOrigin = 0U;
				}
				viewHeight     = screenHeight;
				viewVertOrigin = 0U;
				break;
		}

		ELOG("Enabled Kobo viewport insanity (%ux%u -> %ux%u), top-left corner is @ (%hhu, %hhu)",
		     screenWidth,
		     screenHeight,
		     viewWidth,
		     viewHeight,
		     viewHoriOrigin,
		     viewVertOrigin);
	}
#elif defined(FBINK_FOR_POCKETBOOK)
	// NOTE: Some PocketBook devices have their panel mounted sideways, like the NTX boards we handled above...
	//       I'm *hoping* this is enough to do the trick, without having to resort to a deviceQuirks flag,
	//       which is essentially how this is handled in KOReader (via isAlwaysPortrait).
	//       Obviously, the broadness of this check severely limits the possibility of actually handling hardware rotations
	//       sanely, but for now, we only want to deal with the default rotation properly...
	if (!getenv("FBINK_NO_SW_ROTA")) {
		if (vInfo.xres > vInfo.yres) {
			screenWidth     = vInfo.yres;
			screenHeight    = vInfo.xres;
			fxpRotateCoords = &rotate_coordinates_pickel;
			fxpRotateRegion = &rotate_region_pickel;
			ELOG("Enabled PocketBook rotation quirks (%ux%u -> %ux%u)",
			     vInfo.xres,
			     vInfo.yres,
			     screenWidth,
			     screenHeight);
		}
	}

	viewWidth      = screenWidth;
	viewHoriOrigin = 0U;
	viewHeight     = screenHeight;
	viewVertOrigin = 0U;
#else
	// Other devices are generally never broken-by-design (at least not on that front ;))
	viewWidth      = screenWidth;
	viewHoriOrigin = 0U;
	viewHeight     = screenHeight;
	viewVertOrigin = 0U;
#endif

#ifdef FBINK_WITH_BITMAP
	// NOTE: Set (& reset) original font resolution, in case we're re-init'ing,
	//       since we're relying on the default value to calculate the scaled value,
	//       and we're using this value to set MAXCOLS & MAXROWS, which we *need* to be sane.
#	ifdef FBINK_WITH_FONTS
	// Setup custom fonts (glyph size, render fx, bitmap fx)
	switch (fbink_cfg->fontname) {
		case VGA:
			glyphWidth         = 8U;
			glyphHeight        = 16U;
			fxpFont8xGetBitmap = &vga_get_bitmap;
			break;
		case MICROKNIGHT:
			glyphWidth         = 8U;
			glyphHeight        = 16U;
			fxpFont8xGetBitmap = &microknight_get_bitmap;
			break;
		case TOPAZ:
			glyphWidth         = 8U;
			glyphHeight        = 16U;
			fxpFont8xGetBitmap = &topaz_get_bitmap;
			break;
		case TEWIB:
			glyphWidth         = 6U;
			glyphHeight        = 13U;
			fxpFont8xGetBitmap = &tewib_get_bitmap;
			break;
		case TEWI:
			glyphWidth         = 6U;
			glyphHeight        = 13U;
			fxpFont8xGetBitmap = &tewi_get_bitmap;
			break;
		case SPLEEN:
			glyphWidth          = 16U;
			glyphHeight         = 32U;
			fxpFont16xGetBitmap = &spleen_get_bitmap;
			break;
		case FATTY:
			glyphWidth         = 7U;
			glyphHeight        = 16U;
			fxpFont8xGetBitmap = &fatty_get_bitmap;
			break;
		case TERMINUSB:
			glyphWidth         = 8U;
			glyphHeight        = 16U;
			fxpFont8xGetBitmap = &terminusb_get_bitmap;
			break;
		case TERMINUS:
			glyphWidth         = 8U;
			glyphHeight        = 16U;
			fxpFont8xGetBitmap = &terminus_get_bitmap;
			break;
		case SCIENTIFICAI:
			glyphWidth         = 7U;
			glyphHeight        = 12U;
			fxpFont8xGetBitmap = &scientificai_get_bitmap;
			break;
		case SCIENTIFICAB:
			glyphWidth         = 5U;
			glyphHeight        = 12U;
			fxpFont8xGetBitmap = &scientificab_get_bitmap;
			break;
		case SCIENTIFICA:
			glyphWidth         = 5U;
			glyphHeight        = 12U;
			fxpFont8xGetBitmap = &scientifica_get_bitmap;
			break;
		case ORPI:
			glyphWidth         = 6U;
			glyphHeight        = 12U;
			fxpFont8xGetBitmap = &orpi_get_bitmap;
			break;
		case ORPB:
			glyphWidth         = 6U;
			glyphHeight        = 12U;
			fxpFont8xGetBitmap = &orpb_get_bitmap;
			break;
		case ORP:
			glyphWidth         = 6U;
			glyphHeight        = 12U;
			fxpFont8xGetBitmap = &orp_get_bitmap;
			break;
		case CTRLD:
			glyphWidth         = 8U;
			glyphHeight        = 16U;
			fxpFont8xGetBitmap = &ctrld_get_bitmap;
			break;
		case FKP:
			glyphWidth         = 8U;
			glyphHeight        = 16U;
			fxpFont8xGetBitmap = &fkp_get_bitmap;
			break;
		case KATES:
			glyphWidth         = 7U;
			glyphHeight        = 15U;
			fxpFont8xGetBitmap = &kates_get_bitmap;
			break;
		case VEGGIE:
			glyphWidth         = 8U;
			glyphHeight        = 16U;
			fxpFont8xGetBitmap = &veggie_get_bitmap;
			break;
		case LEGGIE:
			glyphWidth         = 8U;
			glyphHeight        = 18U;
			fxpFont8xGetBitmap = &leggie_get_bitmap;
			break;
		case BLOCK:
			glyphWidth          = 32U;
			glyphHeight         = 32U;
			// An horizontal resolution > 8 means a different data type...
			fxpFont32xGetBitmap = &block_get_bitmap;
			break;
		case UNSCII_TALL:
			glyphWidth         = 8U;
			glyphHeight        = 16U;
			fxpFont8xGetBitmap = &tall_get_bitmap;
			break;
		case UNSCII_MCR:
			glyphWidth         = 8U;
			glyphHeight        = 8U;
			fxpFont8xGetBitmap = &mcr_get_bitmap;
			break;
		case UNSCII_FANTASY:
			glyphWidth         = 8U;
			glyphHeight        = 8U;
			fxpFont8xGetBitmap = &fantasy_get_bitmap;
			break;
		case UNSCII_THIN:
			glyphWidth         = 8U;
			glyphHeight        = 8U;
			fxpFont8xGetBitmap = &thin_get_bitmap;
			break;
		case UNSCII_ALT:
			glyphWidth         = 8U;
			glyphHeight        = 8U;
			fxpFont8xGetBitmap = &alt_get_bitmap;
			break;
		case UNSCII:
			glyphWidth         = 8U;
			glyphHeight        = 8U;
			fxpFont8xGetBitmap = &unscii_get_bitmap;
			break;
#		ifdef FBINK_WITH_UNIFONT
		case UNIFONT:
			glyphWidth         = 8U;
			glyphHeight        = 16U;
			fxpFont8xGetBitmap = &unifont_get_bitmap;
			break;
		case UNIFONTDW:
			glyphWidth          = 16U;
			glyphHeight         = 16U;
			fxpFont16xGetBitmap = &unifontdw_get_bitmap;
			break;
#		endif
		case COZETTE:
			glyphWidth         = 8U;
			glyphHeight        = 13U;
			fxpFont8xGetBitmap = &cozette_get_bitmap;
			break;
		case IBM:
		default:
			glyphWidth         = 8U;
			glyphHeight        = 8U;
			fxpFont8xGetBitmap = &font8x8_get_bitmap;
			break;
	}
#	else
	// Default font is IBM
	glyphWidth         = 8U;
	glyphHeight        = 8U;
	fxpFont8xGetBitmap = &font8x8_get_bitmap;

	if (fbink_cfg->fontname != IBM) {
		ELOG("Custom fonts are not supported in this FBInk build, using IBM instead.");
	}
#	endif    // FBINK_WITH_FONTS
#endif            // FBINK_WITH_BITMAP

	// Obey user-specified font scaling multiplier
	if (fbink_cfg->fontmult > 0) {
		FONTSIZE_MULT = fbink_cfg->fontmult;
		uint8_t max_fontmult;

		// NOTE: Clamp to safe values to avoid a division by zero later if a glyph becomes so big
		//       that it causes MAXCOLS or MAXROWS to become too small...
		//       We want to ensure available_cols in fbink_print will be > 0, so,
		//       follow the same heuristics to devise the minimum MAXCOLS we want to enforce...
		unsigned short int min_maxcols = 1U;
		if (fbink_cfg->is_centered) {
			min_maxcols++;
			// NOTE: If that weren't a circular dependency, we'd take care of the isPerfectFit case here,
			//       but we can't, so instead that corner-case is handled in fbink_print...
		}
#ifdef FBINK_WITH_FONTS
		// NOTE: Handle custom fonts, no matter their base glyph size...
		// We want at least N columns, so, viewWidth / N / glyphWidth gives us the maximum multiplier.
		const uint8_t max_fontmult_width  = (uint8_t) (viewWidth / min_maxcols / glyphWidth);
		// We want at least 1 row, so, viewHeight / glyphHeight gives us the maximum multiplier.
		const uint8_t max_fontmult_height = (uint8_t) (viewHeight / glyphHeight);
		max_fontmult                      = (uint8_t) MIN(max_fontmult_width, max_fontmult_height);
		if (FONTSIZE_MULT > max_fontmult) {
			FONTSIZE_MULT = max_fontmult;
			ELOG("Clamped font size multiplier from %hhu to %hhu", fbink_cfg->fontmult, max_fontmult);
		}
#else
		// The default font's glyphs are 8x8, do the least amount of work possible ;).
		max_fontmult = (uint8_t) (viewWidth / min_maxcols / 8U);
		if (FONTSIZE_MULT > max_fontmult) {
			FONTSIZE_MULT = max_fontmult;
			ELOG("Clamped font size multiplier from %hhu to %hhu", fbink_cfg->fontmult, max_fontmult);
		}
#endif
	} else {
		// Set font-size based on screen DPI
		// (roughly matches: Linux (96), Pearl (167), Carta (212), 6.8" Carta (265), 6/7/8" Carta HD (300))
		if (deviceQuirks.screenDPI <= 212U) {
			// Most of these should be 6", with the notable exception of the DX/DXg ;).
			FONTSIZE_MULT = 2U;    // 16x16
		} else if (deviceQuirks.screenDPI <= 265U) {
			// All of these 265dpi devices are from NTX/Kobo, and feature a 6.8" screen,
			// so we have horizontal space to spare to justify the higher scaling factor ;).
			FONTSIZE_MULT = 3U;    // 24x24
		} else {
			// NOTE: Try to differentiate between 6" devices and larger screens,
			//       because we'll want a smaller scaling factor on 6" devices,
			//       because column space is sparse there ;).
			const uint32_t actual_height = MAX(vInfo.xres, vInfo.yres);
			if (actual_height <= 1448U) {
				// That should cover everyone (Voyage/Oasis 1/PaperWhite 3 & 4/Glo HD/Clara HD)
				FONTSIZE_MULT = 3U;    // 24x24
			} else {
				// We have more headroom on larger screens ;) (Oasis 2/Forma)
				FONTSIZE_MULT = 4U;    // 32x32
			}
		}
#ifdef FBINK_WITH_FONTS
		if (fbink_cfg->fontname == BLOCK) {
			// Block is roughly 4 times wider than other fonts, compensate for that...
			FONTSIZE_MULT = (uint8_t) MAX(1U, FONTSIZE_MULT / 4U);
#	ifdef FBINK_WITH_UNIFONT
		} else if (fbink_cfg->fontname == SPLEEN || fbink_cfg->fontname == UNIFONTDW) {
#	else
		} else if (fbink_cfg->fontname == SPLEEN) {
#	endif
			// Spleen & Unifont DW are roughly twice as wide as other fonts, compensate for that...
			FONTSIZE_MULT = (uint8_t) MAX(1U, FONTSIZE_MULT / 2U);
		}
#endif
	}
	// Go!
	FONTW = (unsigned short int) (glyphWidth * FONTSIZE_MULT);
	FONTH = (unsigned short int) (glyphHeight * FONTSIZE_MULT);
	ELOG("Fontsize set to %hux%hu (%s base glyph size: %hhux%hhu)",
	     FONTW,
	     FONTH,
	     fontname_to_string(fbink_cfg->fontname),
	     glyphWidth,
	     glyphHeight);

	// Compute MAX* values now that we know the screen & font resolution
	MAXCOLS = (unsigned short int) (viewWidth / FONTW);
	MAXROWS = (unsigned short int) (viewHeight / FONTH);
	ELOG("Line length: %hu cols, Page size: %hu rows", MAXCOLS, MAXROWS);

	// Mention & remember if we can perfectly fit the final column on screen
	if ((uint32_t) (FONTW * MAXCOLS) == viewWidth) {
		deviceQuirks.isPerfectFit = true;
		ELOG("Horizontal fit is perfect!");
	} else {
		deviceQuirks.isPerfectFit = false;
	}

	// In a similar fashion, add a vertical offset to make sure rows are vertically "centered",
	// in case we can't perfectly fit the final row.
	if ((uint32_t) (FONTH * MAXROWS) == viewHeight) {
		viewVertOffset = 0U;
	} else {
		// NOTE: That should also fall under no_viewport's purview
		if (!fbink_cfg->no_viewport) {
			viewVertOffset = (uint8_t) (((float) (viewHeight - (uint32_t) (FONTH * MAXROWS)) / 2.0f) + 0.5f);
			ELOG("Vertical fit isn't perfect, shifting rows down by %hhu pixels", viewVertOffset);
		} else {
			viewVertOffset = 0U;
			ELOG("Vertical fit isn't perfect, but viewport fiddling was explicitly disabled");
		}
	}
	// Bake that into the viewport computations,
	// we'll special-case the image codepath to ignore it when row is unspecified (i.e., 0) ;).
	viewVertOrigin = (uint8_t) (viewVertOrigin + viewVertOffset);

#ifdef FBINK_WITH_DRAW
	// Pack the pen colors into the right pixel format...
	if (update_pen_colors(fbink_cfg) != EXIT_SUCCESS) {
		goto cleanup;
	}

	// Use the appropriate get/put pixel functions...
	switch (vInfo.bits_per_pixel) {
		case 4U:
			//fxpPutPixel = &put_pixel_Gray4;
			fxpGetPixel        = &get_pixel_Gray4;
			fxpFillRect        = &fill_rect_Gray4;
			fxpFillRectChecked = &fill_rect_Gray4_checked;
			break;
		case 8U:
			//fxpPutPixel = &put_pixel_Gray8;
			fxpGetPixel        = &get_pixel_Gray8;
			fxpFillRect        = &fill_rect_Gray8;
			fxpFillRectChecked = &fill_rect_Gray8_checked;
			break;
		case 16U:
			//fxpPutPixel = &put_pixel_RGB565;
			fxpGetPixel        = &get_pixel_RGB565;
			fxpFillRect        = &fill_rect_RGB565;
			fxpFillRectChecked = &fill_rect_RGB565_checked;
			break;
		case 24U:
			//fxpPutPixel = &put_pixel_RGB24;
			fxpGetPixel        = &get_pixel_RGB24;
			fxpFillRect        = &fill_rect_RGB24;
			fxpFillRectChecked = &fill_rect_RGB24_checked;
			break;
		case 32U:
			//fxpPutPixel = &put_pixel_RGB32;
			fxpGetPixel        = &get_pixel_RGB32;
			fxpFillRect        = &fill_rect_RGB32;
			fxpFillRectChecked = &fill_rect_RGB32_checked;
			break;
		default:
			// Huh oh... Should never happen!
			WARN("Unsupported framebuffer bpp");
			rv = ERRCODE(EXIT_FAILURE);
			goto cleanup;
			break;
	}
#endif    // FBINK_WITH_DRAW

	// NOTE: Do we want to keep the fb0 fd open, or simply close it for now?
	//       Useful because we probably want to close it to keep open fds to a minimum when used as a library,
	//       while wanting to avoid a useless open/close/open/close cycle when used as a standalone tool.
cleanup:
	if (!keep_fd) {
		close_fb(fbfd);
	}

	return rv;
}

// And that's how we expose it to the API ;)
int
    fbink_init(int fbfd, const FBInkConfig* restrict fbink_cfg)
{
	// Don't skip any ioctls on a first init ;)
	return initialize_fbink(fbfd, fbink_cfg, false);
}

#ifdef FBINK_WITH_OPENTYPE
static __attribute__((cold)) const char*
    font_style_to_string(FONT_STYLE_T style)
{
	switch (style) {
		case FNT_REGULAR:
			return "Regular";
		case FNT_ITALIC:
			return "Italic";
		case FNT_BOLD:
			return "Bold";
		case FNT_BOLD_ITALIC:
			return "Bold Italic";
		default:
			return "Unknown?!";
	}
}

// Load OT fonts for fbink_add_ot_font & fbink_add_ot_font_v2
static __attribute__((cold)) int
    add_ot_font(const char* filename, FONT_STYLE_T style, FBInkOTFonts* restrict ot_fonts)
{
#	ifdef FBINK_FOR_KOBO
	// NOTE: Bail if we were passed a Kobo system font, as they're obfuscated,
	//       and some of them risk crashing stbtt because of bogus data...
	const char blacklist[] = "/usr/local/Trolltech/QtEmbedded-4.6.2-arm/lib/fonts/";
	if (!strncmp(filename, blacklist, sizeof(blacklist) - 1)) {
		WARN("Cannot use font `%s`: it's an obfuscated Kobo system font", filename + sizeof(blacklist) - 1);
		return ERRCODE(EXIT_FAILURE);
	}
#	endif

	// Init libunibreak the first time we're called
	if (!otInit) {
		init_linebreak();
		LOG("Initialized libunibreak");
	}
	otInit = true;

	// Open font from given path, and load into buffer
	FILE* f                      = fopen(filename, "r" STDIO_CLOEXEC);
	unsigned char* restrict data = NULL;
	if (!f) {
		PFWARN("fopen: %m");
		otInit = false;
		return ERRCODE(EXIT_FAILURE);
	} else {
		const int   fd = fileno(f);
		struct stat st;
		if (fstat(fd, &st) == -1) {
			PFWARN("fstat: %m");
			fclose(f);
			otInit = false;
			return ERRCODE(EXIT_FAILURE);
		}
		data = calloc((size_t) st.st_size, sizeof(*data));
		if (!data) {
			PFWARN("Error allocating font data buffer: %m");
			fclose(f);
			otInit = false;
			return ERRCODE(EXIT_FAILURE);
		}
		if (fread(data, 1U, (size_t) st.st_size, f) < (size_t) st.st_size || ferror(f) != 0) {
			free(data);
			fclose(f);
			otInit = false;
			WARN("Error reading font file `%s`", filename);
			return ERRCODE(EXIT_FAILURE);
		}
		fclose(f);
	}
	stbtt_fontinfo* font_info = calloc(1U, sizeof(stbtt_fontinfo));
	if (!font_info) {
		PFWARN("Error allocating stbtt_fontinfo struct: %m");
		free(data);
		return ERRCODE(EXIT_FAILURE);
	}
	// First, check if we can actually find a recognizable font format in the data...
	const int fontcount = stbtt_GetNumberOfFonts(data);
	if (fontcount == 0) {
		free(data);
		free(font_info);
		WARN("File `%s` doesn't appear to be a valid or supported font", filename);
		return ERRCODE(EXIT_FAILURE);
	} else if (fontcount > 1) {
		LOG("Font file `%s` appears to be a font collection containing %d fonts, but we'll only use the first one!",
		    filename,
		    fontcount);
	}
	// Then, get the offset to the first font
	const int fontoffset = stbtt_GetFontOffsetForIndex(data, 0);
	if (fontoffset == -1) {
		free(data);
		free(font_info);
		WARN("File `%s` doesn't appear to contain valid font data at offset %d", filename, fontoffset);
		return ERRCODE(EXIT_FAILURE);
	}
	// And finally, initialize that font
	// NOTE: We took the long way 'round to try to avoid crashes on invalid data...
	if (!stbtt_InitFont(font_info, data, fontoffset)) {
		free(font_info->data);
		free(font_info);
		WARN("Error initialising font `%s`", filename);
		return ERRCODE(EXIT_FAILURE);
	}
	// Assign the current font to its appropriate FBInkOTFonts struct member, depending on the style specified by the caller.
	// NOTE: We make sure we free any previous allocation first!
	switch (style) {
		case FNT_REGULAR:
			if (free_ot_font(&(ot_fonts->otRegular)) == EXIT_SUCCESS) {
				LOG("Replacing an existing Regular font style!");
			}
			ot_fonts->otRegular = font_info;
			break;
		case FNT_ITALIC:
			if (free_ot_font(&(ot_fonts->otItalic)) == EXIT_SUCCESS) {
				LOG("Replacing an existing Italic font style!");
			}
			ot_fonts->otItalic = font_info;
			break;
		case FNT_BOLD:
			if (free_ot_font(&(ot_fonts->otBold)) == EXIT_SUCCESS) {
				LOG("Replacing an existing Bold font style!");
			}
			ot_fonts->otBold = font_info;
			break;
		case FNT_BOLD_ITALIC:
			if (free_ot_font(&(ot_fonts->otBoldItalic)) == EXIT_SUCCESS) {
				LOG("Replacing an existing Bold Italic font style!");
			}
			ot_fonts->otBoldItalic = font_info;
			break;
		default:
			free(font_info->data);
			free(font_info);
			WARN("Cannot load font `%s`: requested style (%d) is invalid!", filename, style);
			return ERRCODE(EXIT_FAILURE);
	}

	ELOG("Font `%s` loaded for style '%s'", filename, font_style_to_string(style));
	return EXIT_SUCCESS;
}
#endif    // FBINK_WITH_OPENTYPE

// Load font from given file path. Up to four font styles may be used by FBInk at any given time.
int
    fbink_add_ot_font(const char* filename UNUSED_BY_MINIMAL, FONT_STYLE_T style UNUSED_BY_MINIMAL)
{
#ifdef FBINK_WITH_OPENTYPE
	// Legacy variant, using the global otFonts
	LOG("Loading font data in the global font pool . . .");
	return add_ot_font(filename, style, &otFonts);
#else
	WARN("OpenType support is disabled in this FBInk build");
	return ERRCODE(ENOSYS);
#endif    // FBINK_WITH_OPENTYPE
}

// Load font from given file path. Up to four font styles may be used per FBInkOTConfig instance.
int
    fbink_add_ot_font_v2(const char* filename        UNUSED_BY_MINIMAL,
			 FONT_STYLE_T style          UNUSED_BY_MINIMAL,
			 FBInkOTConfig* restrict cfg UNUSED_BY_MINIMAL)
{
#ifdef FBINK_WITH_OPENTYPE
	// Start by allocating an FBInkOTFonts struct, if need be...
	if (!cfg->font) {
		cfg->font = calloc(1U, sizeof(FBInkOTFonts));
		if (!cfg->font) {
			PFWARN("Error allocating FBInkOTFonts struct: %m");
			return ERRCODE(EXIT_FAILURE);
		}
	}
	// New variant, using a per-FBInkOTConfig instance
	LOG("Loading font data in a local FBInkOTFonts instance (%p) . . .", cfg->font);
	return add_ot_font(filename, style, (FBInkOTFonts*) cfg->font);
#else
	WARN("OpenType support is disabled in this FBInk build");
	return ERRCODE(ENOSYS);
#endif    // FBINK_WITH_OPENTYPE
}

#ifdef FBINK_WITH_OPENTYPE
// Free an individual OpenType font structure
static __attribute__((cold)) int
    free_ot_font(stbtt_fontinfo** restrict font_info)
{
	if (*font_info) {
		free((*font_info)->data);    // This is the font data we loaded
		free(*font_info);
		// Don't leave a dangling pointer
		*font_info = NULL;

		return EXIT_SUCCESS;
	} else {
		return ERRCODE(EINVAL);
	}
}

// Free OT fonts for fbink_free_ot_fonts & fbink_free_ot_fonts_v2
static __attribute__((cold)) int
    free_ot_fonts(FBInkOTFonts* restrict ot_fonts)
{
	if (free_ot_font(&(ot_fonts->otRegular)) == EXIT_SUCCESS) {
		LOG("Released Regular font data");
	}
	if (free_ot_font(&(ot_fonts->otItalic)) == EXIT_SUCCESS) {
		LOG("Released Italic font data");
	}
	if (free_ot_font(&(ot_fonts->otBold)) == EXIT_SUCCESS) {
		LOG("Released Bold font data");
	}
	if (free_ot_font(&(ot_fonts->otBoldItalic)) == EXIT_SUCCESS) {
		LOG("Released Bold Italic font data");
	}

	return EXIT_SUCCESS;
}
#endif    // FBINK_WITH_OPENTYPE

// Free all OpenType fonts (as loaded by fbink_add_ot_font)
int
    fbink_free_ot_fonts(void)
{
#ifdef FBINK_WITH_OPENTYPE
	// Legacy variant, using the global otFonts
	LOG("Releasing font data from the global font pool . . .");
	return free_ot_fonts(&otFonts);
#else
	WARN("OpenType support is disabled in this FBInk build");
	return ERRCODE(ENOSYS);
#endif    // FBINK_WITH_OPENTYPE
}

// Same, but as loaded by fbink_add_ot_font_v2 for this specific FBInkOTConfig instance.
int
    fbink_free_ot_fonts_v2(FBInkOTConfig* cfg UNUSED_BY_MINIMAL)
{
#ifdef FBINK_WITH_OPENTYPE
	if (cfg->font) {
		// New variant, using a per-FBInkOTConfig instance
		LOG("Releasing font data from a local FBInkOTFonts instance (%p) . . .", cfg->font);
		int rv = free_ot_fonts((FBInkOTFonts*) cfg->font);
		// Free the FBInkOTFonts struct itself
		free(cfg->font);
		// Don't leave a dangling pointer
		cfg->font = NULL;

		return rv;
	} else {
		// There were no fonts allocated for this FBInkOTConfig!
		return ERRCODE(EINVAL);
	}
#else
	WARN("OpenType support is disabled in this FBInk build");
	return ERRCODE(ENOSYS);
#endif    // FBINK_WITH_OPENTYPE
}

// Dump a few of our internal state variables to stdout, for shell script consumption
void
    fbink_state_dump(const FBInkConfig* fbink_cfg)
{
	fprintf(
	    stdout,
	    "FBINK_VERSION='%s';FBINK_TARGET=%hhu;FBINK_FEATURES=%#x;viewWidth=%u;viewHeight=%u;screenWidth=%u;screenHeight=%u;viewHoriOrigin=%hhu;viewVertOrigin=%hhu;viewVertOffset=%hhu;DPI=%hu;BPP=%u;lineLength=%u;invertedGrayscale=%d;FONTW=%hu;FONTH=%hu;FONTSIZE_MULT=%hhu;FONTNAME='%s';glyphWidth=%hhu;glyphHeight=%hhu;MAXCOLS=%hu;MAXROWS=%hu;isPerfectFit=%d;FBID='%s';USER_HZ=%ld;penFGColor=%hhu;penBGColor=%hhu;deviceName='%s';deviceId=%hu;deviceCodename='%s';devicePlatform='%s';isMTK=%d;isSunxi=%d;SunxiHasFBDamage=%d;SunxiForceRota=%d;isKindleLegacy=%d;isKoboNonMT=%d;unreliableWaitFor=%d;ntxBootRota=%hhu;ntxRotaQuirk=%hhu;rotationMap='{ %hhu, %hhu, %hhu, %hhu }';touchSwapAxes=%d;touchMirrorX=%d;touchMirrorY=%d;isNTX16bLandscape=%d;currentRota=%u;canRotate=%d;canHWInvert=%d;hasEclipseWfm=%d;canWaitForSubmission=%d;",
	    fbink_version(),
	    fbink_target(),
	    fbink_features(),
	    viewWidth,
	    viewHeight,
	    screenWidth,
	    screenHeight,
	    viewHoriOrigin,
	    viewVertOrigin,
	    viewVertOffset,
	    deviceQuirks.screenDPI,
	    vInfo.bits_per_pixel,
	    fInfo.line_length,
	    !!(vInfo.bits_per_pixel == 8U && vInfo.grayscale == GRAYSCALE_8BIT_INVERTED),
	    FONTW,
	    FONTH,
	    FONTSIZE_MULT,
	    fontname_to_string(fbink_cfg->fontname),
	    glyphWidth,
	    glyphHeight,
	    MAXCOLS,
	    MAXROWS,
	    deviceQuirks.isPerfectFit,
	    fInfo.id,
	    USER_HZ,
	    penFGColor,
	    penBGColor,
	    deviceQuirks.deviceName,
	    deviceQuirks.deviceId,
	    deviceQuirks.deviceCodename,
	    deviceQuirks.devicePlatform,
	    deviceQuirks.isMTK,
	    deviceQuirks.isSunxi,
#if defined(FBINK_FOR_KOBO)
	    sunxiCtx.has_fbdamage,
	    sunxiCtx.force_rota,
#else
	    false,
	    FORCE_ROTA_NOTSUP,
#endif
	    deviceQuirks.isKindleLegacy,
	    deviceQuirks.isKoboNonMT,
	    deviceQuirks.unreliableWaitFor,
	    deviceQuirks.ntxBootRota,
	    deviceQuirks.ntxRotaQuirk,
	    deviceQuirks.rotationMap[FB_ROTATE_UR],
	    deviceQuirks.rotationMap[FB_ROTATE_CW],
	    deviceQuirks.rotationMap[FB_ROTATE_UD],
	    deviceQuirks.rotationMap[FB_ROTATE_CCW],
	    deviceQuirks.touchSwapAxes,
	    deviceQuirks.touchMirrorX,
	    deviceQuirks.touchMirrorY,
	    deviceQuirks.isNTX16bLandscape,
	    vInfo.rotate,
	    deviceQuirks.canRotate,
	    deviceQuirks.canHWInvert,
	    deviceQuirks.hasEclipseWfm,
	    deviceQuirks.canWaitForSubmission);
}

// Dump a few of our internal state variables to the FBInkState struct pointed to by fbink_state
void
    fbink_get_state(const FBInkConfig* restrict fbink_cfg, FBInkState* restrict fbink_state)
{
	if (fbink_state) {
		fbink_state->user_hz         = USER_HZ;
		fbink_state->font_name       = fontname_to_string(fbink_cfg->fontname);
		fbink_state->view_width      = viewWidth;
		fbink_state->view_height     = viewHeight;
		fbink_state->screen_width    = screenWidth;
		fbink_state->screen_height   = screenHeight;
		fbink_state->scanline_stride = fInfo.line_length;
		fbink_state->bpp             = vInfo.bits_per_pixel;
		fbink_state->inverted_grayscale =
		    !!(vInfo.bits_per_pixel == 8U && vInfo.grayscale == GRAYSCALE_8BIT_INVERTED);
		strncpy(fbink_state->device_name, deviceQuirks.deviceName, sizeof(fbink_state->device_name) - 1U);
		strncpy(
		    fbink_state->device_codename, deviceQuirks.deviceCodename, sizeof(fbink_state->device_codename) - 1U);
		strncpy(
		    fbink_state->device_platform, deviceQuirks.devicePlatform, sizeof(fbink_state->device_platform) - 1U);
		fbink_state->device_id        = deviceQuirks.deviceId;
		fbink_state->pen_fg_color     = penFGColor;
		fbink_state->pen_bg_color     = penBGColor;
		fbink_state->screen_dpi       = deviceQuirks.screenDPI;
		fbink_state->font_w           = FONTW;
		fbink_state->font_h           = FONTH;
		fbink_state->max_cols         = MAXCOLS;
		fbink_state->max_rows         = MAXROWS;
		fbink_state->view_hori_origin = viewHoriOrigin;
		fbink_state->view_vert_origin = viewVertOrigin;
		fbink_state->view_vert_offset = viewVertOffset;
		fbink_state->fontsize_mult    = FONTSIZE_MULT;
		fbink_state->glyph_width      = glyphWidth;
		fbink_state->glyph_height     = glyphHeight;
		fbink_state->is_perfect_fit   = deviceQuirks.isPerfectFit;
		fbink_state->is_mtk           = deviceQuirks.isMTK;
		fbink_state->is_sunxi         = deviceQuirks.isSunxi;
#if defined(FBINK_FOR_KOBO)
		fbink_state->sunxi_has_fbdamage = sunxiCtx.has_fbdamage;
		fbink_state->sunxi_force_rota   = sunxiCtx.force_rota;
#else
		fbink_state->sunxi_has_fbdamage = false;
		fbink_state->sunxi_force_rota   = FORCE_ROTA_NOTSUP;
#endif
		fbink_state->is_kindle_legacy    = deviceQuirks.isKindleLegacy;
		fbink_state->is_kobo_non_mt      = deviceQuirks.isKoboNonMT;
		fbink_state->unreliable_wait_for = deviceQuirks.unreliableWaitFor;
		fbink_state->ntx_boot_rota       = deviceQuirks.ntxBootRota;
		fbink_state->ntx_rota_quirk      = deviceQuirks.ntxRotaQuirk;
		memcpy(fbink_state->rotation_map, deviceQuirks.rotationMap, sizeof(deviceQuirks.rotationMap));
		fbink_state->touch_swap_axes         = deviceQuirks.touchSwapAxes;
		fbink_state->touch_mirror_x          = deviceQuirks.touchMirrorX;
		fbink_state->touch_mirror_y          = deviceQuirks.touchMirrorY;
		fbink_state->is_ntx_quirky_landscape = deviceQuirks.isNTX16bLandscape;
		fbink_state->current_rota            = (uint8_t) vInfo.rotate;
		fbink_state->can_rotate              = deviceQuirks.canRotate;
		fbink_state->can_hw_invert           = deviceQuirks.canHWInvert;
		fbink_state->has_eclipse_wfm         = deviceQuirks.hasEclipseWfm;
		fbink_state->can_wait_for_submission = deviceQuirks.canWaitForSubmission;
	} else {
		WARN("Err, it appears we were passed a NULL fbink_state pointer?");
	}
}

// Memory map the framebuffer
static int
    memmap_fb(int fbfd)
{
#ifdef FBINK_FOR_KOBO
	if (deviceQuirks.isSunxi) {
		return memmap_ion();
	}
#endif

	// NOTE: Beware of smem_len on Kobos?
	//       c.f., https://github.com/koreader/koreader-base/blob/master/ffi/framebuffer_linux.lua#L36
	//       TL;DR: On 16bpp fbs, it *might* be a bit larger than strictly necessary,
	//              but I've yet to see that be an issue with what I'm doing,
	//              and trusting it is much simpler than trying to outsmart broken fb setup info...
	//       See also the Cervantes quirk documented in clear_screen...
	fbPtr = (unsigned char*) mmap(NULL, fInfo.smem_len, PROT_READ | PROT_WRITE, MAP_SHARED, fbfd, 0);
	if (fbPtr == MAP_FAILED) {
		PFWARN("mmap: %m");
		fbPtr = NULL;
		return ERRCODE(EXIT_FAILURE);
	} else {
		isFbMapped = true;
	}

	return EXIT_SUCCESS;
}

#ifdef FBINK_FOR_KOBO

// I'm wary of inlined nanosleep calls, c.f., do_infinite_progress_bar @ fbink_cmd.c
// NOTE: Mark me __attribute__((noinline, cold)) if necessary...
static void
    yield_to_sunxi_disp(void)
{
	const struct timespec zzz = { 0L, 50000000L };    // 50ms
	nanosleep(&zzz, NULL);
}

// Do the same, but via ION on sunxi...
static int
    memmap_ion(void)
{
	int rv = EXIT_SUCCESS;

	// Start by registering as an ION client
	sunxiCtx.ion_fd = open("/dev/ion", O_RDONLY | O_NONBLOCK | O_CLOEXEC);
	if (sunxiCtx.ion_fd == -1) {
		PFWARN("ion open: %m");
		return ERRCODE(EXIT_FAILURE);
	}

	// Then request a page-aligned carveout mapping large enough to fit our screen.
	sunxiCtx.alloc_size              = ALIGN(fInfo.smem_len, 4096);
	struct ion_allocation_data alloc = { .len          = sunxiCtx.alloc_size,
					     .align        = 4096,
					     .heap_id_mask = ION_HEAP_MASK_CARVEOUT };
	int                        ret   = ioctl(sunxiCtx.ion_fd, ION_IOC_ALLOC, &alloc);
	if (ret < 0) {
		PFWARN("ION_IOC_ALLOC: %m");
		rv = ERRCODE(EXIT_FAILURE);
		goto cleanup;
	}

	// Request a dmabuff handle that we can share & mmap for that alloc
	sunxiCtx.ion.handle = alloc.handle;
	ret                 = ioctl(sunxiCtx.ion_fd, ION_IOC_MAP, &sunxiCtx.ion);
	if (ret < 0) {
		PFWARN("ION_IOC_MAP: %m");
		rv = ERRCODE(EXIT_FAILURE);
		goto cleanup;
	}

	// Finally, mmap it...
	fbPtr = (unsigned char*) mmap(NULL, sunxiCtx.alloc_size, PROT_READ | PROT_WRITE, MAP_SHARED, sunxiCtx.ion.fd, 0);
	if (fbPtr == MAP_FAILED) {
		PFWARN("mmap: %m");
		fbPtr = NULL;
		rv    = ERRCODE(EXIT_FAILURE);
		goto cleanup;
	} else {
		isFbMapped = true;
	}

	// And finally, register as a DISP client, too
	// NOTE: Since FW 4.31.19086, this appears to be race-y, most notably during early boot,
	//       at which point it can reliably throw an ENODEV for some mysterious reason...
	//       O_NONBLOCK appears to only have a minimal effect on this new quirk...
	size_t disp_retry = 0U;
	while ((sunxiCtx.disp_fd = open("/dev/disp", O_RDONLY | O_NONBLOCK | O_CLOEXEC)) == -1) {
		// Retry a few times, waiting for a few ticks between each attempt...
		yield_to_sunxi_disp();
		disp_retry++;
		PFWARN("disp open (attempt %zu): %m", disp_retry);
		// Give up after a second.
		if (disp_retry >= 20) {
			PFWARN("Giving up on disp open");
			rv = ERRCODE(EXIT_FAILURE);
			goto cleanup;
		}
	}

	// And update our layer config to use that dmabuff fd, as a grayscale buffer.
	sunxiCtx.layer.info.fb.fd    = 0;
	sunxiCtx.layer.info.fb.y8_fd = sunxiCtx.ion.fd;

	return rv;

cleanup:
	if (isFbMapped) {
		if (munmap(fbPtr, sunxiCtx.alloc_size) < 0) {
			PFWARN("munmap: %m");
			rv = ERRCODE(EXIT_FAILURE);
		} else {
			isFbMapped          = false;
			fbPtr               = NULL;
			sunxiCtx.alloc_size = 0U;

			if (close(sunxiCtx.ion.fd) != 0) {
				PFWARN("close: %m");
				rv = ERRCODE(EXIT_FAILURE);
			} else {
				sunxiCtx.ion.fd = -1;
			}
		}
	}

	if (sunxiCtx.ion.handle != 0) {
		struct ion_handle_data handle = { .handle = sunxiCtx.ion.handle };
		ret                           = ioctl(sunxiCtx.ion_fd, ION_IOC_FREE, &handle);
		if (ret < 0) {
			PFWARN("ION_IOC_FREE: %m");
			rv = ERRCODE(EXIT_FAILURE);
		} else {
			sunxiCtx.ion.handle = 0;
		}
	}

	if (sunxiCtx.ion_fd != -1) {
		if (close(sunxiCtx.ion_fd) != 0) {
			PFWARN("ion close: %m");
			rv = ERRCODE(EXIT_FAILURE);
		} else {
			sunxiCtx.ion_fd = -1;
		}
	}

	if (sunxiCtx.disp_fd != -1) {
		if (close(sunxiCtx.disp_fd) != 0) {
			PFWARN("disp close: %m");
			rv = ERRCODE(EXIT_FAILURE);
		} else {
			sunxiCtx.disp_fd = -1;
		}
	}

	return rv;
}
#endif    // FBINK_FOR_KOBO

// And unmap it
static int
    unmap_fb(void)
{
#ifdef FBINK_FOR_KOBO
	if (deviceQuirks.isSunxi) {
		return unmap_ion();
	}
#endif

	if (munmap(fbPtr, fInfo.smem_len) < 0) {
		PFWARN("munmap: %m");
		return ERRCODE(EXIT_FAILURE);
	} else {
		// NOTE: Don't forget to reset those state flags,
		//       so we won't skip mmap'ing on the next call without a fb fd passed...
		isFbMapped = false;
		fbPtr      = NULL;
	}

	return EXIT_SUCCESS;
}

#ifdef FBINK_FOR_KOBO
// And the same for ION again...
static int
    unmap_ion(void)
{
	int rv = EXIT_SUCCESS;

	if (munmap(fbPtr, sunxiCtx.alloc_size) < 0) {
		PFWARN("munmap: %m");
		rv = ERRCODE(EXIT_FAILURE);
	} else {
		isFbMapped          = false;
		fbPtr               = NULL;
		sunxiCtx.alloc_size = 0U;

		if (close(sunxiCtx.ion.fd) != 0) {
			PFWARN("close: %m");
			rv = ERRCODE(EXIT_FAILURE);
		} else {
			sunxiCtx.ion.fd = -1;
		}
	}

	struct ion_handle_data handle = { .handle = sunxiCtx.ion.handle };
	int                    ret    = ioctl(sunxiCtx.ion_fd, ION_IOC_FREE, &handle);
	if (ret < 0) {
		PFWARN("ION_IOC_FREE: %m");
		rv = ERRCODE(EXIT_FAILURE);
	} else {
		sunxiCtx.ion.handle          = 0;
		sunxiCtx.layer.info.fb.fd    = 0;
		sunxiCtx.layer.info.fb.y8_fd = 0;
	}

	if (close(sunxiCtx.ion_fd) != 0) {
		PFWARN("close: %m");
		rv = ERRCODE(EXIT_FAILURE);
	} else {
		sunxiCtx.ion_fd = -1;
	}

	if (close(sunxiCtx.disp_fd) != 0) {
		PFWARN("close: %m");
		rv = ERRCODE(EXIT_FAILURE);
	} else {
		sunxiCtx.disp_fd = -1;
	}

	return rv;
}
#endif    // FBINK_FOR_KOBO

// Handle closing the fb's fd for FBFD_AUTO workflows
static inline void
    close_fb(int fbfd)
{
	close(fbfd);

#ifdef FBINK_FOR_KOBO
	if (deviceQuirks.isSunxi) {
		// Also close the accelerometer I²C handle...
		close_accelerometer_i2c();
	}
#endif
}

// Public helper function that handles unmapping the fb & closing its fd, for applications that want to keep both around,
// (i.e., those that use fbink_open in the first place).
int
    fbink_close(int fbfd)
{
	// With a few sprinkles of sanity checks, in case something *really* unexpected happen,
	// or simply to cover a wide range of API usage.
	if (isFbMapped) {
		if (unmap_fb() != EXIT_SUCCESS) {
			return ERRCODE(EXIT_FAILURE);
		}
	}

	if (fbfd != FBFD_AUTO) {
		if (close(fbfd) < 0) {
			PFWARN("close: %m");
			return ERRCODE(EXIT_FAILURE);
		}
	}

#ifdef FBINK_FOR_KOBO
	if (deviceQuirks.isSunxi) {
		if (close_accelerometer_i2c() != EXIT_SUCCESS) {
			WARN("Failed to close accelerometer I²C handle");
			return ERRCODE(EXIT_FAILURE);
		}
	}
#endif

	return EXIT_SUCCESS;
}

// Much like rotate_coordinates, but for a mxcfb rectangle
// c.f., adjust_coordinates @ drivers/video/fbdev/mxc/mxc_epdc_v2_fb.c
#if defined(FBINK_FOR_KOBO) || defined(FBINK_FOR_CERVANTES) || defined(FBINK_FOR_POCKETBOOK)
static void
    rotate_region_pickel(struct mxcfb_rect* restrict region)
{
	// Rotate the region to account for pickel's rotation...
	struct mxcfb_rect oregion = *region;
	// NOTE: left = x, top = y
	region->top               = screenWidth - oregion.left - oregion.width;
	region->left              = oregion.top;
	region->width             = oregion.height;
	region->height            = oregion.width;
}
#endif    // FBINK_FOR_KOBO || FBINK_FOR_CERVANTES || FBINK_FOR_POCKETBOOK

#if defined(FBINK_FOR_KOBO) || defined(FBINK_FOR_CERVANTES)
static void
    rotate_region_boot(struct mxcfb_rect* restrict region)
{
	// Rotate the region to account for the native boot rotation...
	struct mxcfb_rect oregion = *region;
	// NOTE: left = x, top = y
	region->top               = oregion.left;
	region->left              = screenHeight - oregion.top - oregion.height;
	region->width             = oregion.height;
	region->height            = oregion.width;
}
#endif    // FBINK_FOR_KOBO || FBINK_FOR_CERVANTES

static void
    rotate_region_nop(struct mxcfb_rect* restrict region __attribute__((unused)))
{
	// NOP! See rotate_coordinates_nop for the rationale ;)
}

// Tweak the region to cover the full screen
static void
    fullscreen_region(struct mxcfb_rect* restrict region)
{
	region->top    = 0U;
	region->left   = 0U;
	region->width  = vInfo.xres;
	region->height = vInfo.yres;
}

// Do a full-screen clear, eInk refresh included
int
    fbink_cls(int fbfd                              UNUSED_BY_NODRAW,
	      const FBInkConfig* restrict fbink_cfg UNUSED_BY_NODRAW,
	      const FBInkRect* restrict rect        UNUSED_BY_NODRAW,
	      bool no_rota                          UNUSED_BY_NODRAW)
{
#ifdef FBINK_WITH_DRAW
	// If we open a fd now, we'll only keep it open for this single call!
	// NOTE: We *expect* to be initialized at this point, though, but that's on the caller's hands!
	bool keep_fd = true;
	if (open_fb_fd(&fbfd, &keep_fd) != EXIT_SUCCESS) {
		return ERRCODE(EXIT_FAILURE);
	}

	// Assume success, until shit happens ;)
	int rv = EXIT_SUCCESS;

	// mmap fb to user mem
	if (!isFbMapped) {
		if (memmap_fb(fbfd) != EXIT_SUCCESS) {
			rv = ERRCODE(EXIT_FAILURE);
			goto cleanup;
		}
	}

	// If we requested to disable rotation tricks, just fudge fxpRotateRegion for the call's duration...
	void (*actual_region_rotate_fxp)(struct mxcfb_rect* restrict) = fxpRotateRegion;
	if (no_rota) {
		fxpRotateRegion = &rotate_region_nop;
	}

	// We'll need a matching region for the refresh...
	struct mxcfb_rect region = { 0U };

	// Did we request a regional clear?
	bool full_clear = false;
	if (!rect || (rect->width == 0U || rect->height == 0U)) {
		// Nope -> full-screen
		clear_screen(fbfd, fbink_cfg->is_inverted ? penBGColor ^ 0xFFu : penBGColor, fbink_cfg->is_flashing);
		full_clear    = true;
		// Set a region for set_last_rect...
		region.top    = 0U;
		region.left   = 0U;
		region.width  = screenWidth;
		region.height = screenHeight;
	} else {
		// Yes -> simply fill a rectangle w/ the bg color
		FBInkPixel bgP = penBGPixel;
		if (fbink_cfg->is_inverted) {
			// NOTE: And, of course, RGB565 is terrible. Inverting the lossy packed value would be even lossier...
			if (unlikely(vInfo.bits_per_pixel == 16U)) {
				const uint8_t bgcolor = penBGColor ^ 0xFFu;
				bgP.rgb565            = pack_rgb565(bgcolor, bgcolor, bgcolor);
			} else {
				bgP.bgra.p ^= 0x00FFFFFFu;
			}
		}
		(*fxpFillRectChecked)(rect->left, rect->top, rect->width, rect->height, &bgP);
		// And update the region...
		region.top    = rect->top;
		region.left   = rect->left;
		region.width  = rect->width;
		region.height = rect->height;
	}

	// Remember the rect...
	set_last_rect(&region);
	// Rotate the region if need be...
	(*fxpRotateRegion)(&region);
	// Fudge the region if we asked for a screen clear, so that we actually refresh the full screen...
	if (full_clear) {
		fullscreen_region(&region);
	}

	if (no_rota) {
		fxpRotateRegion = actual_region_rotate_fxp;
	}

	// Refresh screen
	if (refresh(fbfd, region, fbink_cfg) != EXIT_SUCCESS) {
		PFWARN("Failed to refresh the screen");
		rv = ERRCODE(EXIT_FAILURE);
		goto cleanup;
	}

	// Cleanup
cleanup:
	if (isFbMapped && !keep_fd) {
		unmap_fb();
	}
	if (!keep_fd) {
		close_fb(fbfd);
	}

	return rv;
#else
	WARN("Drawing primitives are disabled in this FBInk build");
	return ERRCODE(ENOSYS);
#endif
}

// Do a full-screen invert, eInk refresh included
int
    fbink_invert_screen(int fbfd, const FBInkConfig* restrict fbink_cfg)
{
	// If we open a fd now, we'll only keep it open for this single call!
	// NOTE: We *expect* to be initialized at this point, though, but that's on the caller's hands!
	bool keep_fd = true;
	if (open_fb_fd(&fbfd, &keep_fd) != EXIT_SUCCESS) {
		return ERRCODE(EXIT_FAILURE);
	}

	// Assume success, until shit happens ;)
	int rv = EXIT_SUCCESS;

	// mmap fb to user mem
	if (!isFbMapped) {
		if (memmap_fb(fbfd) != EXIT_SUCCESS) {
			rv = ERRCODE(EXIT_FAILURE);
			goto cleanup;
		}
	}

	// Similar in spirit to clear_screen, but closer to KOReader's BB_invert_rect ;).
	if (unlikely(vInfo.bits_per_pixel == 16U)) {
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wcast-align"
		uint16_t* p = (uint16_t*) fbPtr;
#pragma GCC diagnostic pop
		size_t px_count = (size_t) vInfo.xres_virtual * vInfo.yres;
		while (px_count--) {
			// NOTE: Not actually accurate, but I don't care about RGB565 ;).
			*p++ ^= 0xFFFFu;
		}
	} else if (vInfo.bits_per_pixel == 32U) {
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wcast-align"
		uint32_t* p = (uint32_t*) fbPtr;
#pragma GCC diagnostic pop
		size_t px_count = (size_t) vInfo.xres_virtual * vInfo.yres;
		while (px_count--) {
			*p++ ^= 0x00FFFFFFu;
		}
	} else {
		// Byte per byte should do the trick for the other bitdepths
		uint8_t* p          = fbPtr;
		size_t   byte_count = (size_t) fInfo.line_length * vInfo.yres;
		while (byte_count--) {
			*p++ ^= 0xFFu;
		}
	}

	// We'll need a matching region for the refresh...
	struct mxcfb_rect region = { 0U };
	fullscreen_region(&region);

	// Remember the rect...
	set_last_rect(&region);
	// Rotate the region if need be...
	(*fxpRotateRegion)(&region);

	// Refresh screen
	if (refresh(fbfd, region, fbink_cfg) != EXIT_SUCCESS) {
		PFWARN("Failed to refresh the screen");
		rv = ERRCODE(EXIT_FAILURE);
		goto cleanup;
	}

	// Cleanup
cleanup:
	if (isFbMapped && !keep_fd) {
		unmap_fb();
	}
	if (!keep_fd) {
		close_fb(fbfd);
	}

	return rv;
}

// Inverts a region of the framebuffer. Can be used to help implement nightmode on devices without HW inversion support.
// NOTE: Unlike fbink_invert_screen, this does *not* trigger a refresh.
int
    fbink_invert_rect(int fbfd                       UNUSED_BY_NODRAW,
		      const FBInkRect* restrict rect UNUSED_BY_NODRAW,
		      bool no_rota                   UNUSED_BY_NODRAW)
{
#ifdef FBINK_WITH_DRAW
	// If we open a fd now, we'll only keep it open for this single call!
	// NOTE: We *expect* to be initialized at this point, though, but that's on the caller's hands!
	bool keep_fd = true;
	if (open_fb_fd(&fbfd, &keep_fd) != EXIT_SUCCESS) {
		return ERRCODE(EXIT_FAILURE);
	}

	// Assume success, until shit happens ;)
	int rv = EXIT_SUCCESS;

	// mmap fb to user mem
	if (!isFbMapped) {
		if (memmap_fb(fbfd) != EXIT_SUCCESS) {
			rv = ERRCODE(EXIT_FAILURE);
			goto cleanup;
		}
	}

	// We'll need a matching region...
	struct mxcfb_rect region = { 0U };

	// Did we actually request a rect?
	bool full_clear = false;
	if (!rect || (rect->width == 0U || rect->height == 0U)) {
		full_clear = true;
		fullscreen_region(&region);
	} else {
		region.top    = rect->top;
		region.left   = rect->left;
		region.width  = rect->width;
		region.height = rect->height;
	}

	// Remember the rect...
	set_last_rect(&region);
	// Rotate the region if need be...
	if (!no_rota) {
		(*fxpRotateRegion)(&region);
	}

	// Do the thing...
	if (full_clear) {
		// Basically, fbink_invert_screen but without a refresh
		if (unlikely(vInfo.bits_per_pixel == 16U)) {
#	pragma GCC diagnostic push
#	pragma GCC diagnostic ignored "-Wcast-align"
			uint16_t* p = (uint16_t*) fbPtr;
#	pragma GCC diagnostic pop
			size_t px_count = (size_t) vInfo.xres_virtual * vInfo.yres;
			while (px_count--) {
				// NOTE: Not actually accurate, but I don't care about RGB565 ;).
				*p++ ^= 0xFFFFu;
			}
		} else if (vInfo.bits_per_pixel == 32U) {
#	pragma GCC diagnostic push
#	pragma GCC diagnostic ignored "-Wcast-align"
			uint32_t* p = (uint32_t*) fbPtr;
#	pragma GCC diagnostic pop
			size_t px_count = (size_t) vInfo.xres_virtual * vInfo.yres;
			while (px_count--) {
				*p++ ^= 0x00FFFFFFu;
			}
		} else {
			// Byte per byte should do the trick for the other bitdepths
			uint8_t* p          = fbPtr;
			size_t   byte_count = (size_t) fInfo.line_length * vInfo.yres;
			while (byte_count--) {
				*p++ ^= 0xFFu;
			}
		}
	} else {
		// Based on their fill_rect counterparts
		if (unlikely(vInfo.bits_per_pixel == 16U)) {
			for (size_t j = region.top; j < region.top + region.height; j++) {
				const size_t scanline_offset = fInfo.line_length * j;
#	pragma GCC diagnostic push
#	pragma GCC diagnostic ignored "-Wcast-align"
				uint16_t* restrict p = (uint16_t*) (fbPtr + scanline_offset) + region.left;
#	pragma GCC diagnostic pop
				size_t px_count = region.width;

				while (px_count--) {
					// NOTE: Not actually accurate, but I don't care about RGB565 ;).
					*p++ ^= 0xFFFFu;
				}
			}
		} else if (unlikely(vInfo.bits_per_pixel == 4U)) {
			// I don't particularly care about doing this in a smarter way either...
			for (unsigned short int cy = 0U; cy < region.height; cy++) {
				for (unsigned short int cx = 0U; cx < region.width; cx++) {
					const FBInkCoordinates coords = {
						.x = (unsigned short int) (region.left + cx),
						.y = (unsigned short int) (region.top + cy),
					};
					FBInkPixel px = { 0 };
					get_pixel_Gray4(&coords, &px);
					px.gray8 ^= 0xFFu;
					put_pixel_Gray4(&coords, &px);
				}
			}
		} else if (vInfo.bits_per_pixel == 32U) {
			for (size_t j = region.top; j < region.top + region.height; j++) {
				const size_t scanline_offset = fInfo.line_length * j;
#	pragma GCC diagnostic push
#	pragma GCC diagnostic ignored "-Wcast-align"
				uint32_t* p = (uint32_t*) (fbPtr + scanline_offset) + region.left;
#	pragma GCC diagnostic pop
				size_t px_count = region.width;

				while (px_count--) {
					*p++ ^= 0x00FFFFFFu;
				}
			}
		} else {
			// Byte per byte should do the trick for the other bitdepths
			for (size_t j = region.top; j < region.top + region.height; j++) {
				// NOTE: Go with a cheap memset32 in order to preserve the alpha value of our input pixel...
				//       The compiler should be able to turn that into something as fast as a plain memset ;).
				const size_t scanline_offset = fInfo.line_length * j;
#	pragma GCC diagnostic push
#	pragma GCC diagnostic ignored "-Wcast-align"
				uint8_t* p = (uint8_t*) (fbPtr + scanline_offset) + region.left;
#	pragma GCC diagnostic pop
				size_t px_count = region.width;

				while (px_count--) {
					*p++ ^= 0xFFu;
				}
			}
		}
	}

	// Cleanup
cleanup:
	if (isFbMapped && !keep_fd) {
		unmap_fb();
	}
	if (!keep_fd) {
		close_fb(fbfd);
	}

	return rv;
#else
	WARN("Drawing primitives are disabled in this FBInk build");
	return ERRCODE(ENOSYS);
#endif
}

// Handle cls & refresh, but for grid-based coordinates (i.e., like fbink_print()'s draw())
static int
    grid_to_region(int                fbfd,
		   unsigned short int cols,
		   unsigned short int rows,
		   bool               do_clear,
		   const FBInkConfig* restrict fbink_cfg)
{
	// If we open a fd now, we'll only keep it open for this single call!
	// NOTE: We *expect* to be initialized at this point, though, but that's on the caller's hands!
	bool keep_fd = true;
	if (open_fb_fd(&fbfd, &keep_fd) != EXIT_SUCCESS) {
		return ERRCODE(EXIT_FAILURE);
	}

	// Assume success, until shit happens ;)
	int rv = EXIT_SUCCESS;

	// mmap fb to user mem if need be
	if (do_clear) {
		if (!isFbMapped) {
			if (memmap_fb(fbfd) != EXIT_SUCCESS) {
				rv = ERRCODE(EXIT_FAILURE);
				goto cleanup;
			}
		}
	}

	// NOTE: Since the idea is to play nice with fbink_print()/draw(),
	//       that implies duplicating their whole set of insane positioning tweaks...

	// NOTE: Starting with the initial row/col handling from fbink_print...
	// NOTE: Make copies of these so we don't wreck our original struct, since we passed it by reference,
	//       and we *will* heavily mangle these two...
	short int col = fbink_cfg->col;
	short int row = fbink_cfg->row;

	// NOTE: If we asked to print in the middle of the screen, make the specified row an offset from the middle of the screen,
	//       instead of the top.
	if (fbink_cfg->is_halfway) {
		row = (short int) (row + (short int) (MAXROWS / 2U));
		// NOTE: Clamp to positive values to avoid wrap-around,
		//       (i.e., -MAX should always be 0, and +MAX always the last row (i.e., MAXROWS - 1)).
		//       Our default behavior of starting negative rows from the bottom essentially amounts to valign == bottom,
		//       which is not desirable when we explicitly asked for center ;).
		row = (short int) MIN(MAX(0, row), (MAXROWS - 1));
		LOG("Adjusted row to %hd for vertical centering", row);
	}

	// See if want to position our text relative to the edge of the screen, and not the beginning
	if (col < 0) {
		col = (short int) MAX(MAXCOLS + col, 0);
	}
	if (row < 0) {
		row = (short int) MAX(MAXROWS + row, 0);
	}
	LOG("Adjusted position: column %hd, row %hd", col, row);

	// Clamp coordinates to the screen, to avoid blowing up ;).
	if (col >= MAXCOLS) {
		col = (short int) (MAXCOLS - 1U);
		LOG("Clamped column to %hd", col);
	}
	// NOTE: For lines, we attempt to do something potentially a bit less destructive, namely,
	//       wrapping back to the top of the screen.
	//       This seems like a better way to handle large dumps of data over stdin, for instance,
	//       as well as a slightly less cryptic behavior when in interactive mode...
	//       In any case, we're always going to overwrite something, but that way,
	//       stuff shown on screen should be more or less consistent,
	//       instead of overwriting over and over again the final line at the bottom of the screen...
	while (row >= MAXROWS) {
		row = (short int) (row - MAXROWS);
		LOG("Wrapped row back to %hd", row);
	}

	// Compute the amount of characters we can actually print on *one* line given the column we start on...
	// NOTE: When centered, we enforce one padding character on the left,
	//       as well as one padding character on the right when we have a perfect fit.
	//       This is to avoid potentially printing stuff too close to the bezel and/or behind the bezel.
	unsigned short int available_cols = MAXCOLS;
	if (fbink_cfg->is_centered) {
		// One for the left padding
		available_cols = (unsigned short int) (available_cols - 1U);
		if (deviceQuirks.isPerfectFit) {
			// And one for the right padding
			available_cols = (unsigned short int) (available_cols - 1U);
			// NOTE: If that makes us fall to 0 (because of high font scaling),
			//       set it to 1 to avoid a division by zero, and warn that centering will be slightly off.
			//       This takes a *really* unlucky perfect fit at high font sizes to actually happen...
			if (available_cols == 0U) {
				available_cols = 1U;
				LOG("Enforced a minimum of 1 available column to compensate for an unlucky perfect fit at high font scaling! Centering may be slightly off.");
			}
		}
	} else {
		// Otherwise, col will be fixed, so, trust it.
		available_cols = (unsigned short int) (available_cols - col);
	}

	// Move our initial row up if we add so much lines that some of it goes off-screen...
	if (row + rows > MAXROWS) {
		row = (short int) MIN(row - ((row + rows) - MAXROWS), MAXROWS);
	}
	LOG("Final position: column %hd, row %hd", col, row);

	// Just fudge the column for centering...
	bool halfcell_offset = false;
	if (fbink_cfg->is_centered) {
		col = (short int) ((unsigned short int) (MAXCOLS - cols) / 2U);

		// NOTE: If the line itself is not a perfect fit, ask draw to start drawing half a cell
		//       to the right to compensate, in order to achieve perfect centering...
		//       This piggybacks a bit on the !isPerfectFit compensation done in draw,
		//       which already does subcell placement ;).
		if (((unsigned short int) col * 2U) + cols != MAXCOLS) {
			LOG("Line is not a perfect fit, fudging centering by one half of a cell to the right");
			// NOTE: Flag it for correction in draw
			halfcell_offset = true;
		}
		LOG("Adjusted column to %hd for centering", col);
	}

	// Handle centering & padding
	if (fbink_cfg->is_centered && (fbink_cfg->is_padded || fbink_cfg->is_rpadded)) {
		// We always want full padding
		col = 0;

		// We'll essentially pad to the edge of the screen
		cols = MAXCOLS;
	} else if (fbink_cfg->is_padded || fbink_cfg->is_rpadded) {
		// We'll essentially pad to the edge of the available area
		cols = available_cols;
	}

	// NOTE: And then, the truly insane draw() bits...
	// Compute our actual subcell offset in pixels
	unsigned short int pixel_offset = 0U;
	// Do we have a centering induced halfcell adjustment to correct?
	if (halfcell_offset) {
		pixel_offset = FONTW / 2U;
		LOG("Incrementing pixel_offset by %hu pixels to account for a halfcell centering tweak", pixel_offset);
	}
	// Do we have a permanent adjustment to make because of dead space on the right edge?
	if (!deviceQuirks.isPerfectFit) {
		// We correct by half of said dead space, since we want perfect centering ;).
		unsigned short int deadzone_offset =
		    (unsigned short int) (viewWidth - (unsigned short int) (MAXCOLS * FONTW)) / 2U;
		pixel_offset = (unsigned short int) (pixel_offset + deadzone_offset);
		LOG("Incrementing pixel_offset by %hu pixels to compensate for dead space on the right edge",
		    deadzone_offset);
	}

	// Clamp h/v offset to safe values
	short int voffset = fbink_cfg->voffset;
	short int hoffset = fbink_cfg->hoffset;
	// NOTE: This test isn't perfect, but then, if you play with this, you do it knowing the risks...
	//       It's mainly there so that stupidly large values don't wrap back on screen because of overflow wraparound.
	if ((uint32_t) abs(voffset) >= viewHeight) {
		LOG("The specified vertical offset (%hd) necessarily pushes *all* content out of bounds, discarding it",
		    voffset);
		voffset = 0;
	}
	if ((uint32_t) abs(hoffset) >= viewWidth) {
		LOG("The specified horizontal offset (%hd) necessarily pushes *all* content out of bounds, discarding it",
		    hoffset);
		hoffset = 0;
	}

	// Compute the dimension of the screen region we'll paint to
	struct mxcfb_rect region = {
		.top  = (uint32_t) MAX(0 + (viewVertOrigin - viewVertOffset), ((row * FONTH) + voffset + viewVertOrigin)),
		.left = (uint32_t) MAX(0 + viewHoriOrigin, ((col * FONTW) + hoffset + viewHoriOrigin)),
		.width  = (uint32_t) (cols * FONTW),
		.height = (uint32_t) (rows * FONTH),
	};

	// Recap final offset values
	if (hoffset != 0 || viewHoriOrigin != 0) {
		LOG("Adjusting horizontal pen position by %hd pixels, as requested, plus %hhu pixels, as mandated by the native viewport",
		    hoffset,
		    viewHoriOrigin);
		// Clamp region to sane values if h/v offset is pushing stuff off-screen
		if ((region.width + region.left + pixel_offset) > screenWidth) {
			region.width = (uint32_t) MAX(0, (short int) (screenWidth - region.left - pixel_offset));
			LOG("Adjusted region width to account for horizontal offset pushing part of the content off-screen");
		}
		if ((region.left + pixel_offset) >= screenWidth) {
			region.left = screenWidth - pixel_offset - 1;
			LOG("Adjusted region left to account for horizontal offset pushing part of the content off-screen");
		}
	}
	if (voffset != 0 || viewVertOrigin != 0) {
		LOG("Adjusting vertical pen position by %hd pixels, as requested, plus %hhu pixels, as mandated by the native viewport",
		    voffset,
		    viewVertOrigin);
		// Clamp region to sane values if h/v offset is pushing stuff off-screen
		if ((region.top + region.height) > screenHeight) {
			region.height = (uint32_t) MAX(0, (short int) (screenHeight - region.top));
			LOG("Adjusted region height to account for vertical offset pushing part of the content off-screen");
		}
		if (region.top >= screenHeight) {
			region.top = screenHeight - 1;
			LOG("Adjusted region top to account for vertical offset pushing part of the content off-screen");
		}
	}

	LOG("Region: top=%u, left=%u, width=%u, height=%u", region.top, region.left, region.width, region.height);

	// Do we have a pixel offset to honor?
	if (pixel_offset > 0U) {
		LOG("Moving pen %hu pixels to the right to honor subcell centering adjustments", pixel_offset);
		// NOTE: We need to update the start of our region rectangle if it doesn't already cover the full line,
		//       i.e., when we're not padding + centering.
		if (cols != MAXCOLS) {
			if ((hoffset + viewHoriOrigin) == 0) {
				region.left += pixel_offset;
				LOG("Updated region.left to %u", region.left);
			} else {
				// Things are a bit more complex when hoffset is involved...
				// We basically have to re-do the maths from scratch,
				// do it signed to catch corner-cases interactions between col/hoffset/pixel_offset,
				// and clamp it to safe values!
				if ((hoffset + viewHoriOrigin) < 0) {
					region.left =
					    (uint32_t) MAX(0 + viewHoriOrigin,
							   ((col * FONTW) + hoffset + viewHoriOrigin + pixel_offset));
				} else {
					region.left = (uint32_t) MIN(
					    (uint32_t) ((col * FONTW) + hoffset + viewHoriOrigin + pixel_offset),
					    (screenWidth - 1U));
				}
				LOG("Updated region.left to %u", region.left);
			}
		}
	}

	// If we're a full line, we need to fill the space that honoring our offset has left vacant on the left edge...
	// NOTE: In overlay or bgless mode, we don't paint background pixels. This is pure background, so skip it ;).
	if (!fbink_cfg->is_overlay && !fbink_cfg->is_bgless) {
		if (cols == MAXCOLS && pixel_offset > 0U) {
			LOG("Extra background rectangle on the left edge on account of pixel_offset");
			// Correct width, to include that bit of content, too, if needed
			if (region.width < screenWidth) {
				region.width += pixel_offset;
				// And make sure it's properly clamped, because we can't necessarily rely on left & width
				// being entirely acurate either because of the multiline print override,
				// or because of a bit of subcell placement overshoot trickery (c.f., comment in put_pixel).
				if (region.width + region.left > screenWidth) {
					region.width = screenWidth - region.left;
					LOG("Clamped region.width to %u", region.width);
				} else {
					LOG("Updated region.width to %u", region.width);
				}
			}
		}

		// NOTE: In some cases, we also have a matching hole to patch on the right side...
		//       This only applies when pixel_offset *only* accounts for the !isPerfectFit adjustment though,
		//       because in every other case, the halfcell offset handling neatly pushes everything into place ;).
		// NOTE: Again, skip this in overlay/bgless mode ;).
		if (cols == MAXCOLS && !deviceQuirks.isPerfectFit && !halfcell_offset) {
			// NOTE: !isPerfectFit ensures pixel_offset is non-zero
			LOG("Extra background rectangle to fill the dead space on the right edge");
			// If it's not already the case, update region to the full width,
			// because we've just plugged a hole at the very right edge of a full line.
			if (region.width < screenWidth) {
				region.width = screenWidth;
				// Keep making sure it's properly clamped, interaction w/ hoffset can push us over the edge.
				if (region.width + region.left > screenWidth) {
					region.width = screenWidth - region.left;
					LOG("Clamped region.width to %u", region.width);
				} else {
					LOG("Updated region.width to %u", region.width);
				}
			}
		}
	}

	// Did we want to paint a background rectangle (i.e., to mimic fbink_cls)?
	if (do_clear) {
		(*fxpFillRectChecked)((unsigned short int) region.left,
				      (unsigned short int) region.top,
				      (unsigned short int) region.width,
				      (unsigned short int) region.height,
				      &penBGPixel);
	}

	// Remember the rect, and rotate the region if need be...
	set_last_rect(&region);
	(*fxpRotateRegion)(&region);

	// Refresh screen
	if (refresh_compat(fbfd, region, do_clear ? fbink_cfg->no_refresh : false, fbink_cfg) != EXIT_SUCCESS) {
		PFWARN("Failed to refresh the screen");
		rv = ERRCODE(EXIT_FAILURE);
		goto cleanup;
	}

	// Cleanup
cleanup:
	if (do_clear) {
		if (isFbMapped && !keep_fd) {
			unmap_fb();
		}
	}
	if (!keep_fd) {
		close_fb(fbfd);
	}

	return rv;
}

// Public wrappers around grid_to_region
int
    fbink_grid_clear(int fbfd, unsigned short int cols, unsigned short int rows, const FBInkConfig* restrict fbink_cfg)
{
	return grid_to_region(fbfd, cols, rows, true, fbink_cfg);
}

int
    fbink_grid_refresh(int fbfd, unsigned short int cols, unsigned short int rows, const FBInkConfig* restrict fbink_cfg)
{
	return grid_to_region(fbfd, cols, rows, false, fbink_cfg);
}

// Utility function to handle get_last_rect tracking
static void
    set_last_rect(const struct mxcfb_rect* restrict region)
{
	// Remember the region we've just drawn to
	lastRect.left   = (unsigned short int) region->left;
	lastRect.top    = (unsigned short int) region->top;
	lastRect.width  = (unsigned short int) region->width;
	lastRect.height = (unsigned short int) region->height;
}

// Magic happens here!
int
    fbink_print(int fbfd                     UNUSED_BY_NOBITMAP,
		const char* restrict string  UNUSED_BY_NOBITMAP,
		const FBInkConfig* fbink_cfg UNUSED_BY_NOBITMAP)
{
#ifdef FBINK_WITH_BITMAP
	// Abort if we were passed an empty string
	if (!*string) {
		// Unless we just want a clear, in which case, bypass everything and just do that.
		if (fbink_cfg->is_cleared) {
			return fbink_cls(fbfd, fbink_cfg, NULL, false);
		} else {
			PFWARN("Cannot print an empty string");
			return ERRCODE(EINVAL);
		}
	}

	// Abort if we were passed an invalid UTF-8 sequence
	const size_t len       = strlen(string);    // Flawfinder: ignore
	const size_t charcount = u8_strlen2(string);
	if (charcount == 0) {
		PFWARN("Cannot print an invalid UTF-8 sequence");
		return ERRCODE(EILSEQ);
	}

	// If we open a fd now, we'll only keep it open for this single print call!
	// NOTE: We *expect* to be initialized at this point, though, but that's on the caller's hands!
	bool keep_fd = true;
	if (open_fb_fd(&fbfd, &keep_fd) != EXIT_SUCCESS) {
		return ERRCODE(EXIT_FAILURE);
	}

	// Assume success, until shit happens ;)
	int rv              = EXIT_SUCCESS;
	// We need to declare this early (& sentinel it to NULL) to make our cleanup jumps safe
	char* restrict line = NULL;

	// map fb to user mem
	// NOTE: If we're keeping the fb's fd open, keep this mmap around, too.
	if (!isFbMapped) {
		if (memmap_fb(fbfd) != EXIT_SUCCESS) {
			rv = ERRCODE(EXIT_FAILURE);
			goto cleanup;
		}
	}

	// NOTE: Make copies of these so we don't wreck our original struct, since we passed it by reference,
	//       and we *will* heavily mangle these two...
	short int col = fbink_cfg->col;
	short int row = fbink_cfg->row;

	// NOTE: If we asked to print in the middle of the screen, make the specified row an offset from the middle of the screen,
	//       instead of the top.
	if (fbink_cfg->is_halfway) {
		row = (short int) (row + (short int) (MAXROWS / 2U));
		// NOTE: Clamp to positive values to avoid wrap-around,
		//       (i.e., -MAX should always be 0, and +MAX always the last row (i.e., MAXROWS - 1)).
		//       Our default behavior of starting negative rows from the bottom essentially amounts to valign == bottom,
		//       which is not desirable when we explicitly asked for center ;).
		row = (short int) MIN(MAX(0, row), (MAXROWS - 1));
		LOG("Adjusted row to %hd for vertical centering", row);
	}

	struct mxcfb_rect  region           = { 0U };
	// We declare that a bit early, because that'll hold our return value on success.
	unsigned short int multiline_offset = 0U;

	// Clear screen?
	if (fbink_cfg->is_cleared) {
		clear_screen(fbfd, fbink_cfg->is_inverted ? penBGColor ^ 0xFFu : penBGColor, fbink_cfg->is_flashing);
	}

	// See if want to position our text relative to the edge of the screen, and not the beginning
	if (col < 0) {
		col = (short int) MAX(MAXCOLS + col, 0);
	}
	if (row < 0) {
		row = (short int) MAX(MAXROWS + row, 0);
	}
	LOG("Adjusted position: column %hd, row %hd", col, row);

	// Clamp coordinates to the screen, to avoid blowing up ;).
	if (col >= MAXCOLS) {
		col = (short int) (MAXCOLS - 1U);
		LOG("Clamped column to %hd", col);
	}
	// NOTE: For lines, we attempt to do something potentially a bit less destructive, namely,
	//       wrapping back to the top of the screen.
	//       This seems like a better way to handle large dumps of data over stdin, for instance,
	//       as well as a slightly less cryptic behavior when in interactive mode...
	//       In any case, we're always going to overwrite something, but that way,
	//       stuff shown on screen should be more or less consistent,
	//       instead of overwriting over and over again the final line at the bottom of the screen...
	bool wrapped_line = false;
	while (row >= MAXROWS) {
		row = (short int) (row - MAXROWS);
		LOG("Wrapped row back to %hd", row);
		// Remember that, so we'll append something to our line to make the wraparound clearer...
		wrapped_line = true;
	}

	// See if we need to break our string down into multiple lines...
	// Check how much extra storage is used up by multibyte sequences.
	if (len > charcount) {
		LOG("Extra storage used up by multibyte sequences: %zu bytes (for a total of %zu characters over %zu bytes)",
		    (len - charcount),
		    charcount,
		    len);
	}

	// Compute the amount of characters we can actually print on *one* line given the column we start on...
	// NOTE: When centered, we enforce one padding character on the left,
	//       as well as one padding character on the right when we have a perfect fit.
	//       This is to avoid potentially printing stuff too close to the bezel and/or behind the bezel.
	unsigned short int available_cols = MAXCOLS;
	if (fbink_cfg->is_centered) {
		// One for the left padding
		available_cols = (unsigned short int) (available_cols - 1U);
		if (deviceQuirks.isPerfectFit) {
			// And one for the right padding
			available_cols = (unsigned short int) (available_cols - 1U);
			// NOTE: If that makes us fall to 0 (because of high font scaling),
			//       set it to 1 to avoid a division by zero, and warn that centering will be slightly off.
			//       This takes a *really* unlucky perfect fit at high font sizes to actually happen...
			if (available_cols == 0U) {
				available_cols = 1U;
				LOG("Enforced a minimum of 1 available column to compensate for an unlucky perfect fit at high font scaling! Centering may be slightly off.");
			}
		}
	} else {
		// Otherwise, col will be fixed, so, trust it.
		available_cols = (unsigned short int) (available_cols - col);
	}
	// Given that, compute how many lines it'll take to print all that in these constraints...
	unsigned short int lines = 1U;
	if (charcount > available_cols) {
		lines = (unsigned short int) (charcount / available_cols);
		// If there's a remainder, we'll need an extra line ;).
		if (charcount % available_cols) {
			lines++;
		}
	}

	// Truncate to a single screen...
	if (lines > MAXROWS) {
		LOG("Can only print %hu out of %hu lines, truncating!", MAXROWS, lines);
		lines = MAXROWS;
	}

	// Move our initial row up if we add so much lines that some of it goes off-screen...
	if (row + lines > MAXROWS) {
		row = (short int) MIN(row - ((row + lines) - MAXROWS), MAXROWS);
	}
	LOG("Final position: column %hd, row %hd", col, row);

	// We'll copy our text in chunks of formatted line...
	// NOTE: Store that on the heap, we've had some wacky adventures with automatic VLAs...
	// NOTE: UTF-8 is at most 4 bytes per sequence, make sure we can fit a full line of UTF-8,
	//       (+ 1 'wide' NULL, wide to make sure u8_strlen won't skip over it).
	line = calloc((MAXCOLS + 1U) * 4U, sizeof(*line));
	if (line == NULL) {
		PFWARN("line calloc: %m");
		rv = ERRCODE(EXIT_FAILURE);
		goto cleanup;
	}
	// NOTE: This makes sure it's always full of NULLs, to avoid weird shit happening later with u8_strlen()
	//       and uninitialized or re-used memory...
	//       Namely, a single NULL immediately followed by something that *might* be interpreted as an UTF-8
	//       sequence would trip it into counting bogus characters.
	//       And as snprintf() only NULL-terminates what it expects to be a non-wide string,
	//       it's not filling the end of the buffer with NULLs, it just outputs a single one!
	//       That's why we're also using calloc here.
	//       Plus, the OS will ensure that'll always be smarter than malloc + memset ;).
	// NOTE: Since we re-use line on each iteration of the loop,
	//       we do also need to clear it at the end of the loop, in preparation of the next iteration.

	LOG("Need %hu lines to print %zu characters over %hu available columns", lines, charcount, available_cols);

	// Do the initial computation outside the loop,
	// so we'll be able to re-use line_len to accurately compute chars_left when looping.
	// NOTE: This is where it gets tricky. With multibyte sequences, 1 byte doesn't necessarily mean 1 char.
	//       And we need to work both in amount of characters for column/width arithmetic,
	//       and in bytes for snprintf...
	size_t chars_left  = charcount;
	size_t line_len    = 0U;
	size_t line_bytes  = 0U;
	size_t line_offset = 0U;
	// If we have multiple lines worth of stuff to print, draw it line per line
	while (chars_left > line_len) {
		LOG("Line %hu (of ~%hu), previous line was %zu characters long and there were %zu characters left to print",
		    (unsigned short int) (multiline_offset + 1U),
		    lines,
		    line_len,
		    chars_left);
		// Make sure we don't try to draw off-screen...
		if (row + multiline_offset >= MAXROWS) {
			LOG("Can only print %hu lines, discarding the %zu characters left!",
			    MAXROWS,
			    chars_left - line_len);
			// And that's it, we're done.
			break;
		}

		// Compute the amount of characters left to print...
		chars_left -= line_len;
		// And use it to compute the amount of characters to print on *this* line
		line_len = (size_t) MIN(chars_left, available_cols);
		LOG("Characters to print: %zu out of the %zu remaining ones", line_len, chars_left);

		// NOTE: Now we just have to switch from characters to bytes, both for line_len & chars_left...
		// First, get the byte offset of this section of our string (i.e., this line)...
		line_offset += line_bytes;
		// ... then compute how many bytes we'll need to store it.
		line_bytes         = 0U;
		size_t   cn        = 0U;
		uint32_t ch        = 0U;
		bool     caught_lf = false;
		while ((ch = u8_nextchar2(string + line_offset, &line_bytes)) != 0U) {
			cn++;
			// NOTE: Honor linefeeds...
			//       The main use-case for this is throwing tail'ed logfiles at us and having them
			//       be readable instead of a jumbled glued together mess ;).
			if (ch == 0x0Au) {
				// Remember that, we'll fudge it to a blank later
				caught_lf = true;
				LOG("Caught a linefeed!");
				// NOTE: We're essentially forcing a reflow by cutting the line mid-stream,
				//       so we have to update our counters...
				//       But we can only correct *one* of chars_left or line_len,
				//       to avoid screwing the count on the next iteration if we correct both,
				//       since the one depend on the other.
				//       And as, for the rest of this iteration/line, we only rely on
				//       line_len being accurate (for padding & centering), the choice is easy.
				// Increment lines, because of course we're adding a line,
				// even if the reflowing changes that'll cause mean we might not end up using it.
				lines++;
				// Don't decrement the byte index, we want to print the LF,
				// (which we'll replace with a blank space, to account for fonts with a visible LF glyph),
				// mostly to make padding look nicer,
				// but also so that line_bytes matches line_len ;).
				// And finally, as we've explained earlier, trim line_len to where we stopped.
				LOG("Line length was %zu characters, but LF is character number %zu", line_len, cn);
				line_len = cn;
				// Don't touch line_offset, the beginning of our line has not changed,
				// only its length was cut short.
				LOG("Adjusted lines to %hu & line_len to %zu", lines, line_len);
				// And of course we break, because that was the whole point of this shenanigan!
				break;
			}
			// We've walked our full line, stop!
			if (cn >= line_len) {
				break;
			}
		}
		LOG("Line takes up %zu bytes", line_bytes);
		int bytes_printed = 0;

		// Just fudge the column for centering...
		bool halfcell_offset = false;
		if (fbink_cfg->is_centered) {
			col = (short int) ((unsigned short int) (MAXCOLS - line_len) / 2U);

			// NOTE: If the line itself is not a perfect fit, ask draw to start drawing half a cell
			//       to the right to compensate, in order to achieve perfect centering...
			//       This piggybacks a bit on the !isPerfectFit compensation done in draw,
			//       which already does subcell placement ;).
			if (((unsigned short int) col * 2U) + line_len != MAXCOLS) {
				LOG("Line is not a perfect fit, fudging centering by one half of a cell to the right");
				// NOTE: Flag it for correction in draw
				halfcell_offset = true;
			}
			LOG("Adjusted column to %hd for centering", col);
		}

		// We don't need any padding if the line is already full...
		// (except when centering is involved, because that implies extra trickery that ensures *some* padding).
		bool can_be_padded = !!(line_len < available_cols);
		// When centered & padded, we need to split the padding in two, left & right.
		if (fbink_cfg->is_centered && (fbink_cfg->is_padded || fbink_cfg->is_rpadded)) {
			// We always want full padding
			col = 0;

			// Compute our padding length
			unsigned short int left_pad  = (unsigned short int) (MAXCOLS - line_len) / 2U;
			// As for the right padding, we basically just have to print 'til the edge of the screen
			unsigned short int right_pad = (unsigned short int) (MAXCOLS - line_len - left_pad);
			// Leave a space for the wraparound marker
			if (wrapped_line) {
				// Prefer clipping from the *right* edge
				if (right_pad > 0U) {
					right_pad = (unsigned short int) (right_pad - 1U);
				} else if (left_pad > 0U) {
					left_pad = (unsigned short int) (left_pad - 1U);
				}
				// NOTE: We enforce a MAXCOLS of at least 2 in fbink_init when centering,
				//       so the earlier available_cols tweaks when centering ensure we'll *always* have at least
				//       1 byte to clip from one of the padding edge.
			}

			// Compute the effective right padding value for science!
			LOG("Total size: %hu + %zu + %hu = %hu",
			    left_pad,
			    line_len,
			    right_pad,
			    (unsigned short int) (left_pad + line_len + right_pad));

			// NOTE: To recap:
			//       Copy at most (MAXCOLS * 4) + 1 bytes into line
			//       (thus ensuring both that its NULL-terminated, and fits a full UTF-8 string)
			//       Left-pad a blank with spaces for left_pad characters
			//       Print line_len characters of our string at the correct position for this line
			//       Right pad a blank with spaces for right_pad characters
			//           Given that we split this in three sections,
			//           left-padding would have had a similar effect.
			bytes_printed = snprintf(line,
						 (MAXCOLS * 4U) + 1U,
						 "%*s%.*s%-*s",
						 (int) left_pad,
						 "",
						 (int) line_bytes,
						 string + line_offset,
						 (int) right_pad,
						 "");
		} else if (fbink_cfg->is_padded && can_be_padded) {
			// NOTE: Rely on the field width for padding ;).
			// Padding character is a space, which is 1 byte, so that's good enough ;).
			size_t padded_bytes = line_bytes + (size_t) (available_cols - line_len);
			// Leave a space for the wraparound marker (can_be_padded ensures we *can* clip a byte here)
			if (wrapped_line) {
				padded_bytes -= 1U;
			}
			// NOTE: Don't touch line_len, because we're *adding* new blank characters,
			//       we're still printing the exact same amount of characters *from our string*.
			LOG("Left padded %zu bytes to %zu to cover %hu columns",
			    line_bytes,
			    padded_bytes,
			    available_cols);
			bytes_printed = snprintf(
			    line, padded_bytes + 1U, "%*.*s", (int) padded_bytes, (int) line_bytes, string + line_offset);
		} else if (fbink_cfg->is_rpadded && can_be_padded) {
			// NOTE: Rely on the field width for padding ;).
			// Padding character is a space, which is 1 byte, so that's good enough ;).
			size_t padded_bytes = line_bytes + (size_t) (available_cols - line_len);
			// Leave a space for the wraparound marker (can_be_padded ensures we *can* clip a byte here)
			if (wrapped_line) {
				padded_bytes -= 1U;
			}
			// NOTE: Don't touch line_len, because we're *adding* new blank characters,
			//       we're still printing the exact same amount of characters *from our string*.
			LOG("Right padded %zu bytes to %zu to cover %hu columns",
			    line_bytes,
			    padded_bytes,
			    available_cols);
			bytes_printed = snprintf(line,
						 padded_bytes + 1U,
						 "%-*.*s",
						 (int) padded_bytes,
						 (int) line_bytes,
						 string + line_offset);
		} else {
			// NOTE: Enforce precision for safety.
			bytes_printed = snprintf(
			    line, line_bytes + 1U, "%*.*s", (int) line_bytes, (int) line_bytes, string + line_offset);
		}
		// NOTE: We don't check for snprintf failure or truncation,
		//       because I'm fairly confident that truncation is not a risk here...
		LOG("snprintf wrote %d bytes", bytes_printed);

		// NOTE: If we caught a LF, replace it with a space to make it behave with fonts that have a visible LF glyph...
		if (caught_lf) {
			// We can't rely on line_bytes as the exact position of the LF, since padding may have moved it...
			char* lf = strrchr(line, 0x0A);
			if (lf) {
				// LF -> space
				*lf = 0x20u;
			}
		}

		// NOTE: And don't forget our wraparound marker (U+2588, a solid black block).
		//       We don't need nor even *want* to add it if the line is already full,
		//       (since the idea is to make it clearer when we're potentially mixing up content from two different lines).
		//       Plus, that'd bork the region in the following draw call, and potentially risk a buffer overflow anyway.
		if (wrapped_line && can_be_padded) {
			LOG("Capping the line with a solid block to make it clearer it has wrapped around...");
			strcat(line, "\u2588");
			// NOTE: U+2588 (█) is a multibyte sequence, namely, it takes 3 bytes
			bytes_printed += 3;
		}

		region = draw(line,
			      (unsigned short int) row,
			      (unsigned short int) col,
			      multiline_offset,
			      halfcell_offset,
			      fbink_cfg);

		// Next line!
		multiline_offset++;

		// NOTE: If we've actually written something, and we're not the final line,
		//       clear what we've just written, to get back a pristine NULL-filled buffer,
		//       so u8_nextchar() has zero chance to skip a NULL on the next iteration.
		//       See the comments around the initial calloc() call for more details.
		if (bytes_printed > 0 && line_len < chars_left) {
			LOG("We have more stuff to print, clearing the line buffer for re-use!");
			memset(line, 0, (size_t) bytes_printed);
		}
		// The nuclear option is simply to unconditonally zero the *full* buffer ;).
		//memset(line, 0, ((MAXCOLS + 1U) * 4U) * sizeof(*line));
	}

	// Handle the last rect stuff...
	set_last_rect(&region);

	// Rotate the region if need be...
	(*fxpRotateRegion)(&region);

	// Fudge the region if we asked for a screen clear, so that we actually refresh the full screen...
	if (fbink_cfg->is_cleared) {
		fullscreen_region(&region);
	}

	// Refresh screen
	if (refresh(fbfd, region, fbink_cfg) != EXIT_SUCCESS) {
		PFWARN("Failed to refresh the screen");
		rv = ERRCODE(EXIT_FAILURE);
		goto cleanup;
	}

	// On success, we return the total amount of lines we occupied on screen
	rv = (int) multiline_offset;

	// Cleanup
cleanup:
	free(line);
	if (isFbMapped && !keep_fd) {
		unmap_fb();
	}
	if (!keep_fd) {
		close_fb(fbfd);
	}

	return rv;
#else
	WARN("Fixed cell font support is disabled in this FBInk build");
	return ERRCODE(ENOSYS);
#endif    // FBINK_WITH_BITMAP
}

#ifdef FBINK_WITH_OPENTYPE
// An extremely rudimentry "markdown" parser. It would probably be wise to cook up something better at some point...
// (c.f., https://github.com/commonmark/commonmark-spec/wiki/List-of-CommonMark-Implementations for inspiration ^^)
// This is *italic* text.
// This is **bold** text.
// This is ***bold italic*** text.
// As well as their underscore equivalents
static void
    parse_simple_md(const char* restrict string, size_t size, unsigned char* restrict result)
{
	size_t ci        = 0;
	bool   is_italic = false;
	bool   is_bold   = false;
	while (ci < size) {
		char ch;
		switch (ch = string[ci]) {
			case '*':
			case '_':
				if (ci + 1 < size && string[ci + 1] == ch) {
					if (ci + 2 < size && string[ci + 2] == ch) {
						is_bold        = !is_bold;
						is_italic      = !is_italic;
						result[ci]     = CH_IGNORE;
						result[ci + 1] = CH_IGNORE;
						result[ci + 2] = CH_IGNORE;
						ci += 3;
						break;
					}
					is_bold        = !is_bold;
					result[ci]     = CH_IGNORE;
					result[ci + 1] = CH_IGNORE;
					ci += 2;
					break;
				}
				// Try to avoid flagging a single underscore in the middle of a word.
				if (ch == '_' && ci > 0 && string[ci - 1] != ' ' && ci + 1 < size &&
				    string[ci + 1] != ' ') {
					result[ci] = CH_REGULAR;
					ci++;
					break;
				}
				is_italic  = !is_italic;
				result[ci] = CH_IGNORE;
				ci++;
				break;
			default:
				if (is_bold && is_italic) {
					result[ci] = CH_BOLD_ITALIC;
				} else if (is_bold) {
					result[ci] = CH_BOLD;
				} else if (is_italic) {
					result[ci] = CH_ITALIC;
				} else {
					result[ci] = CH_REGULAR;
				}
				ci++;
				break;
		}
	}
}

// Small helper for verbose log messages
static __attribute__((cold)) const char*
    glyph_style_to_string(CHARACTER_FONT_T glyph_style)
{
	switch (glyph_style) {
		case CH_IGNORE:
			return "Ignore";
		case CH_REGULAR:
			return "Regular";
		case CH_ITALIC:
			return "Italic";
		case CH_BOLD:
			return "Bold";
		case CH_BOLD_ITALIC:
			return "Bold Italic";
		default:
			return "Unknown?!";
	}
}
#endif    // FBINK_WITH_OPENTYPE

// printf-like wrapper around fbink_print & fbink_print_ot ;).
int
    fbink_printf(int fbfd, const FBInkOTConfig* restrict cfg, const FBInkConfig* restrict fbink_cfg, const char* fmt, ...)
{
	// Assume success, until shit happens ;)
	int rv = EXIT_SUCCESS;

	// We'll need to store our formatted string somewhere...
	// Rely on vsnprintf itself to tell us exactly how many bytes it needs ;).
	// c.f., vsnprintf(3) && stdarg(3) && https://stackoverflow.com/q/10069597
	// (especially as far as the va_start/va_end bracketing is concerned)
	int    ret            = -1;
	size_t size           = 0;
	char* restrict buffer = NULL;
	va_list args;

	// Initial vsnprintf run on a zero length NULL pointer, just to determine the required buffer size
	// NOTE: see vsnprintf(3), this is a C99 behavior made canon in POSIX.1-2001, honored since glibc 2.1
	va_start(args, fmt);
	ret = vsnprintf(buffer, size, fmt, args);
	va_end(args);

	// See if vsnprintf made a boo-boo
	if (ret < 0) {
		PFWARN("initial vsnprintf: %m");
		rv = ERRCODE(EXIT_FAILURE);
		goto cleanup;
	}

	// We need enough space for NULL-termination (which we make 'wide' for u8 reasons) :).
	size   = (size_t) (ret + 4);
	// NOTE: We use calloc to make sure it'll always be zero-initialized,
	//       and the OS is smart enough to make it fast if we don't use the full space anyway (CoW zeroing).
	buffer = calloc(size, sizeof(*buffer));
	if (buffer == NULL) {
		PFWARN("calloc: %m");
		rv = ERRCODE(EXIT_FAILURE);
		goto cleanup;
	}

	// And now we can actually let vsnprintf do its job, for real ;).
	va_start(args, fmt);
	ret = vsnprintf(buffer, size, fmt, args);
	va_end(args);

	// See if vsnprintf made a boo-boo, one final time
	if (ret < 0) {
		PFWARN("vsnprintf: %m");
		rv = ERRCODE(EXIT_FAILURE);
		goto cleanup;
	}

	// Okay, now that we've got a formatted buffer...
	// Did we get a valid FBInkOTConfig pointer?
	if (cfg) {
		// Then feed our formatted string to fbink_print_ot
		rv = fbink_print_ot(fbfd, buffer, cfg, fbink_cfg, NULL);
	} else {
		// Otherwise, feed it to fbink_print instead
		rv = fbink_print(fbfd, buffer, fbink_cfg);
	}

	// Cleanup
cleanup:
	free(buffer);
	return rv;
}

int
    fbink_print_ot(int fbfd                              UNUSED_BY_MINIMAL,
		   const char* restrict string           UNUSED_BY_MINIMAL,
		   const FBInkOTConfig* restrict cfg     UNUSED_BY_MINIMAL,
		   const FBInkConfig* restrict fbink_cfg UNUSED_BY_MINIMAL,
		   FBInkOTFit* restrict fit              UNUSED_BY_MINIMAL)
{
#ifdef FBINK_WITH_OPENTYPE
	// Abort if we were passed an empty string
	if (!*string) {
		// Unless we just want a clear, in which case, bypass everything and just do that.
		if (fbink_cfg->is_cleared) {
			return fbink_cls(fbfd, fbink_cfg, NULL, false);
		} else {
			PFWARN("Cannot print an empty string");
			return ERRCODE(EINVAL);
		}
	}

	// Abort if we were passed an invalid UTF-8 sequence
	const size_t str_len_bytes = strlen(string);    // Flawfinder: ignore
	if (!u8_isvalid2(string)) {
		PFWARN("Cannot print an invalid UTF-8 sequence");
		return ERRCODE(EILSEQ);
	}

	// Has fbink_add_ot_font() been successfully called yet?
	if (!otInit) {
		WARN("No fonts have been loaded");
		return ERRCODE(ENODATA);
	}

	// Just in case we receive a NULL pointer to the cfg struct
	if (!cfg) {
		WARN("FBInkOTConfig expected. Got NULL pointer instead");
		return ERRCODE(EXIT_FAILURE);
	}

	// If we open a fd now, we'll only keep it open for this single print call!
	// NOTE: We *expect* to be initialized at this point, though, but that's on the caller's hands!
	bool keep_fd = true;
	if (open_fb_fd(&fbfd, &keep_fd) != EXIT_SUCCESS) {
		return ERRCODE(EXIT_FAILURE);
	}

	// Assume success, until shit happens ;)
	int rv = EXIT_SUCCESS;

	// Declare buffers early to make cleanup easier
	FBInkOTLine* restrict lines        = NULL;
	char* restrict brk_buff            = NULL;
	unsigned char* restrict fmt_buff   = NULL;
	unsigned char* restrict line_buff  = NULL;
	unsigned char* restrict glyph_buff = NULL;
	// This also needs to be declared early, as we refresh on cleanup.
	struct mxcfb_rect region           = { 0U };
	bool              is_flashing      = false;
	bool              is_cleared       = false;

	// map fb to user mem
	// NOTE: If we're keeping the fb's fd open, keep this mmap around, too.
	if (!isFbMapped) {
		if (memmap_fb(fbfd) != EXIT_SUCCESS) {
			rv = ERRCODE(EXIT_FAILURE);
			goto cleanup;
		}
	}

	// Make sure we return accurate data in case the fit struct is being recycled...
	if (unlikely(fit)) {
		fit->computed_lines = 0U;
		fit->rendered_lines = 0U;
		fit->truncated      = false;
	}

	LOG("Printing OpenType text.");

	// Handle negative margins (meaning count backwards from the opposite edge)
	// NOTE: Obviously makes more sense for top & left than for bottom & right.
	unsigned short int top_margin = 0U;
	if (cfg->margins.top >= 0) {
		top_margin = (unsigned short int) cfg->margins.top;
	} else {
		top_margin = (unsigned short int) MAX(0, (int) viewHeight - abs(cfg->margins.top));
		LOG("Adjusted top margin to %hupx", top_margin);
	}
	unsigned short int bottom_margin = 0U;
	if (cfg->margins.bottom >= 0) {
		bottom_margin = (unsigned short int) cfg->margins.bottom;
	} else {
		bottom_margin = (unsigned short int) MAX(0, (int) viewHeight - abs(cfg->margins.bottom));
		LOG("Adjusted bottom margin to %hupx", bottom_margin);
	}
	unsigned short int left_margin = 0U;
	if (cfg->margins.left >= 0) {
		left_margin = (unsigned short int) cfg->margins.left;
	} else {
		left_margin = (unsigned short int) MAX(0, (int) viewWidth - abs(cfg->margins.left));
		LOG("Adjusted left margin to %hupx", left_margin);
	}
	unsigned short int right_margin = 0U;
	if (cfg->margins.right >= 0) {
		right_margin = (unsigned short int) cfg->margins.right;
	} else {
		right_margin = (unsigned short int) MAX(0, (int) viewWidth - abs(cfg->margins.right));
		LOG("Adjusted right margin to %hupx", right_margin);
	}

	// Sanity check the provided margins and calculate the printable area.
	// We'll cap the margin at 100% for each side, and margins for opposing edges should sum to less than 100%
	if (top_margin >= viewHeight || bottom_margin >= viewHeight || left_margin >= viewWidth ||
	    right_margin >= viewWidth) {
		WARN("A margin was out of range (allowed ranges :: Vert < %u  Horiz < %u)", viewHeight, viewWidth);
		rv = ERRCODE(ERANGE);
		goto cleanup;
	}
	if ((top_margin + bottom_margin) >= viewHeight || (left_margin + right_margin) >= viewWidth) {
		WARN("Opposing margins sum to greater than the viewport height or width");
		rv = ERRCODE(ERANGE);
		goto cleanup;
	}
#	pragma GCC diagnostic push
#	pragma GCC diagnostic ignored "-Wmissing-braces"
	struct
	{
		FBInkCoordinates tl;
		FBInkCoordinates br;
	} area = { 0U };
#	pragma GCC diagnostic pop
	area.tl.x                       = left_margin;
	area.tl.y                       = (unsigned short int) (top_margin + (viewVertOrigin - viewVertOffset));
	area.br.x                       = (unsigned short int) (viewWidth - right_margin);
	area.br.y                       = (unsigned short int) (viewHeight - bottom_margin);
	// Font size can be specified in pixels or in points. Pixels take precedence.
	unsigned short int font_size_px = cfg->size_px;
	// If it wasn't specified in pixels, then it was specified in points (which is also how the default is handled).
	if (font_size_px == 0U) {
		// Set default font size if required
		float size_pt = cfg->size_pt;
		// NOTE: Technically, using the iszero() macro ought to be enough (at least for values coming from our CLI tool),
		//       but of course, it wasn't yet available in the glibc versions we target (as it's from TS 18661-1:2014)...
		//       c.f., https://www.gnu.org/software/libc/manual/html_node/Floating-Point-Classes.html#Floating-Point-Classes
		if (!isnormal(size_pt)) {
			size_pt = 12.0f;
		}
		// We should have a fairly accurate idea of what the screen DPI is...
		const unsigned short int ppi = deviceQuirks.screenDPI;
		// Given the ppi, convert point height to pixels. Note, 1pt is 1/72th of an inch
		font_size_px                 = (unsigned short int) iroundf(ppi / 72.0f * size_pt);
	}

	// This is a pointer to whichever font is currently active. It gets updated for every character in the loop, as needed.
	stbtt_fontinfo* restrict curr_font = NULL;

	int max_row_height = 0;
	// Calculate some metrics for every font we have loaded.
	// Please forgive the repetition here.
	// NOTE: See also stbtt_GetFontBoundingBox?

	// Declaring these three sets of variables early, so a default can be set
	float sf           = 0.0f;
	int   max_baseline = 0;
	int   max_lg       = 0;
	int   max_desc     = 0;
	float rgSF         = 0.0f;
	float itSF         = 0.0f;
	float bdSF         = 0.0f;
	float bditSF       = 0.0f;
	int   asc, desc, lg;
	int   scaled_bl, scaled_desc, scaled_lg;

	// Are we using a local or global FBInkOTFonts?
	const FBInkOTFonts* ot_fonts = NULL;
	if (cfg->font) {
		// If we have local fonts attached to the FBInkOTConfig instance, use 'em
		ot_fonts = (const FBInkOTFonts*) cfg->font;
		LOG("Using fonts from a local FBInkOTFonts instance (%p)", ot_fonts);
	} else {
		// Otherwise, default to the legacy behavior (i.e., use the global).
		ot_fonts = &otFonts;
		LOG("Using fonts from the global font pool");
	}

	if (ot_fonts->otRegular) {
		rgSF = stbtt_ScaleForPixelHeight(ot_fonts->otRegular, (float) font_size_px);
		stbtt_GetFontVMetrics(ot_fonts->otRegular, &asc, &desc, &lg);
		scaled_bl   = iceilf(rgSF * (float) asc);
		scaled_desc = iceilf(rgSF * (float) desc);
		scaled_lg   = iceilf(rgSF * (float) lg);
		if (scaled_bl > max_baseline) {
			max_baseline = scaled_bl;
		}
		if (scaled_desc < max_desc) {
			max_desc = scaled_desc;
		}
		if (scaled_lg > max_lg) {
			max_lg = scaled_lg;
		}
	}
	if (ot_fonts->otItalic) {
		itSF = stbtt_ScaleForPixelHeight(ot_fonts->otItalic, (float) font_size_px);
		stbtt_GetFontVMetrics(ot_fonts->otItalic, &asc, &desc, &lg);
		scaled_bl   = iceilf(itSF * (float) asc);
		scaled_desc = iceilf(itSF * (float) desc);
		scaled_lg   = iceilf(itSF * (float) lg);
		if (scaled_bl > max_baseline) {
			max_baseline = scaled_bl;
		}
		if (scaled_desc < max_desc) {
			max_desc = scaled_desc;
		}
		if (scaled_lg > max_lg) {
			max_lg = scaled_lg;
		}
	}
	if (ot_fonts->otBold) {
		bdSF = stbtt_ScaleForPixelHeight(ot_fonts->otBold, (float) font_size_px);
		stbtt_GetFontVMetrics(ot_fonts->otBold, &asc, &desc, &lg);
		scaled_bl   = iceilf(bdSF * (float) asc);
		scaled_desc = iceilf(bdSF * (float) desc);
		scaled_lg   = iceilf(bdSF * (float) lg);
		if (scaled_bl > max_baseline) {
			max_baseline = scaled_bl;
		}
		if (scaled_desc < max_desc) {
			max_desc = scaled_desc;
		}
		if (scaled_lg > max_lg) {
			max_lg = scaled_lg;
		}
	}
	if (ot_fonts->otBoldItalic) {
		bditSF = stbtt_ScaleForPixelHeight(ot_fonts->otBoldItalic, (float) font_size_px);
		stbtt_GetFontVMetrics(ot_fonts->otBoldItalic, &asc, &desc, &lg);
		scaled_bl   = iceilf(bditSF * (float) asc);
		scaled_desc = iceilf(bditSF * (float) desc);
		scaled_lg   = iceilf(bditSF * (float) lg);
		if (scaled_bl > max_baseline) {
			max_baseline = scaled_bl;
		}
		if (scaled_desc < max_desc) {
			max_desc = scaled_desc;
		}
		if (scaled_lg > max_lg) {
			max_lg = scaled_lg;
		}
	}

	// Set the default font style, for when Markdown parsing is disabled.
	switch (cfg->style) {
		case FNT_ITALIC:
			curr_font = ot_fonts->otItalic;
			sf        = itSF;
			LOG("Unformatted text defaulting to Italic font style");
			break;
		case FNT_BOLD:
			curr_font = ot_fonts->otBold;
			sf        = bdSF;
			LOG("Unformatted text defaulting to Bold font style");
			break;
		case FNT_BOLD_ITALIC:
			curr_font = ot_fonts->otBoldItalic;
			sf        = bditSF;
			LOG("Unformatted text defaulting to Bold Italic font style");
			break;
		case FNT_REGULAR:
		default:
			curr_font = ot_fonts->otRegular;
			sf        = rgSF;
			LOG("Unformatted text defaulting to Regular font style");
			break;
	}
	// If no font was loaded, exit early.
	if (!curr_font) {
		WARN("No font appears to be loaded for the default font style (%s)", font_style_to_string(cfg->style));
		rv = ERRCODE(ENOENT);
		goto cleanup;
	}

	max_row_height = max_baseline + abs(max_desc) + max_lg;
	LOG("Max BL: %d  Max Desc: %d  Max LG: %d  =>  Max LH (according to metrics): %d",
	    max_baseline,
	    max_desc,
	    max_lg,
	    max_row_height);
	// And if max_row_height was not changed from zero, this is also an unrecoverable error.
	// Also guards against a potential divide-by-zero in the following calculation
	if (max_row_height <= 0) {
		WARN("Max line height not set");
		rv = ERRCODE(EXIT_FAILURE);
		goto cleanup;
	}

	// Calculate the maximum number of lines we may have to deal with
	const unsigned int print_height = (unsigned int) (area.br.y - area.tl.y + (viewVertOrigin - viewVertOffset));
	const unsigned int num_lines    = print_height / (unsigned int) max_row_height;

	// And allocate the memory for it...
	lines = calloc(num_lines, sizeof(FBInkOTLine));
	if (!lines) {
		PFWARN("Lines metadata buffer could not be allocated: %m");
		rv = ERRCODE(EXIT_FAILURE);
		goto cleanup;
	}

	// Now, lets use libunibreak to find the possible break opportunities in our string.

	// Note: we only care about the byte length here
	brk_buff = calloc(str_len_bytes + 1U, sizeof(*brk_buff));
	if (!brk_buff) {
		PFWARN("Linebreak buffer could not be allocated: %m");
		rv = ERRCODE(EXIT_FAILURE);
		goto cleanup;
	}

	set_linebreaks_utf8((const utf8_t*) string, str_len_bytes + 1U, "en", brk_buff);
	LOG("Finished looking for linebreaks");

	// Parse our string for formatting, if requested
	if (cfg->is_formatted) {
		fmt_buff = calloc(str_len_bytes + 1U, sizeof(*fmt_buff));
		if (!fmt_buff) {
			PFWARN("Formatted text buffer could not be allocated: %m");
			rv = ERRCODE(EXIT_FAILURE);
			goto cleanup;
		}
		parse_simple_md(string, str_len_bytes, fmt_buff);
		LOG("Finished parsing formatting markup");
	}
	// Lets find our lines! Nothing fancy, just a simple first fit algorithm, but we do our best not to break inside a word.

	size_t             c_index     = 0U;
	size_t             tmp_c_index = c_index;
	uint32_t           c;
	int                gi;
	unsigned short int max_lw = (unsigned short int) (area.br.x - area.tl.x);
	unsigned int       line;
	int                max_line_height = max_row_height - max_lg;
	// adv = advance: the horizontal distance along the baseline to the origin of the next glyph
	// lsb = left side bearing: The horizontal distance from the origin point to left edge of the glyph
	// NOTE: We're not doing anything with lsb, we're honoring stbtt_GetGlyphBitmapBox's x0 instead.
	//       Rounding method aside, they should roughly match.
	int                adv, lsb, curr_x;
	bool               complete_str = false;
	int                x0, y0, x1, y1, gw, gh, cx, cy;
	unsigned int       lw = 0U;
	for (line = 0U; line < num_lines; line++) {
		// Every line has a start character index and an end char index.
		curr_x                     = 0;
		lines[line].startCharIndex = c_index;
		lines[line].line_gap       = max_lg;
		lines[line].line_used      = true;
		while (c_index < str_len_bytes) {
			if (cfg->is_formatted) {
				// Check if we need to skip formatting characters
				if (fmt_buff[c_index] == CH_IGNORE) {
					u8_inc(string, &c_index);
					continue;
				} else {
					switch (fmt_buff[c_index]) {
						case CH_ITALIC:
							curr_font = ot_fonts->otItalic;
							sf        = itSF;
							break;
						case CH_BOLD:
							curr_font = ot_fonts->otBold;
							sf        = bdSF;
							break;
						case CH_BOLD_ITALIC:
							curr_font = ot_fonts->otBoldItalic;
							sf        = bditSF;
							break;
						case CH_REGULAR:
						default:
							curr_font = ot_fonts->otRegular;
							sf        = rgSF;
							break;
					}
					if (!curr_font) {
						WARN("The specified font style (%s) was not loaded",
						     glyph_style_to_string(fmt_buff[c_index]));
						rv = ERRCODE(ENOENT);
						goto cleanup;
					}
				}
			}
			// We check for a mandatory break
			if (brk_buff[c_index] == LINEBREAK_MUSTBREAK) {
				LOG("Found a hard break @ #%zu", c_index);
				size_t last_index = c_index;
				// We don't want to print the break character,
				// but if it's the first character in the buffer, we shouldn't try to look backwards,
				// as u8_dec would blow past the start of the buffer...
				if (c_index > 0) {
					u8_dec(string, &last_index);
				}
				lines[line].endCharIndex = last_index;
				lines[line].has_a_break  = true;
				// We want our next line to start after this breakpoint
				if (c_index > 0) {
					u8_inc(string, &c_index);
				} else {
					// But if the first character in the buffer was a hard break,
					// make sure we won't try to render it by ensuring startCharIndex > endCharIndex,
					// since we've just set endCharIndex to 0 above...
					u8_inc(string, &c_index);
					lines[line].startCharIndex = c_index;
				}
				// And we're done processing this line
				break;
			}
			c  = u8_nextchar2(string, &c_index);
			// Get the glyph index now, instead of having to look it up each time
			gi = stbtt_FindGlyphIndex(curr_font, (int) c);
			// Note, these metrics are unscaled,
			// we need to use our previously obtained scale factor (sf) to get the metrics as pixels
			stbtt_GetGlyphHMetrics(curr_font, gi, &adv, &lsb);
			// But these are already scaled
			stbtt_GetGlyphBitmapBox(curr_font, gi, sf, sf, &x0, &y0, &x1, &y1);
			gw = x1 - x0;
			// Ensure that curr_x never goes negative
			cx = curr_x;
			if (cx + x0 < 0) {
				curr_x += abs(cx + x0);
			}
			// Handle the situation where the metrics may lie, and the glyph descends below what the metrics say.
			if (max_baseline + y1 > max_line_height) {
				int height_diff = (max_baseline + y1) - max_line_height;
				LOG("Descend Height Diff: %d, available LG: %d", height_diff, lines[line].line_gap);
				if (height_diff > lines[line].line_gap) {
					lines[line].line_gap = 0;
				} else {
					lines[line].line_gap -= height_diff;
				}
				max_line_height = max_baseline + y1;
			}
			// Same thing if it tries to ascend too high...
			if (max_baseline + y0 < 0) {
				int height_diff = abs(max_baseline + y0);
				LOG("Ascend Height Diff: %d, available LG: %d", height_diff, lines[line].line_gap);
				if (height_diff > lines[line].line_gap) {
					lines[line].line_gap = 0;
				} else {
					lines[line].line_gap -= height_diff;
				}
				max_line_height += height_diff;
				// We have to bump the baseline, too, to make sure this glyph's baseline is aligned with others...
				max_baseline += height_diff;
			}
			// stb_truetype does not appear to create a bounding box for space characters,
			// so we need to handle this situation.
			// (i.e., stbtt_GetGlyphBitmapBox returns (0, 0, 0, 0) for spaces, so gw is 0)
			if (gw != 0) {
				lw = (unsigned int) (curr_x + x0 + gw);
			} else {
				lw = (unsigned int) curr_x;
			}
			LOG("Current Measured LW: %u  Line# %u", lw, line);
			// Oops, we appear to have advanced too far :)
			// Better backtrack to see if we can find a suitable break opportunity
			if (lw > max_lw) {
				LOG("Looking for a linebreak opportunity . . .");
				// Is the glyph itself too wide for our printable area? If so, we abort
				if ((unsigned int) gw >= max_lw) {
					WARN(
					    "Font size too big for current printable area. Try to reduce margins or font size");
					rv = ERRCODE(EXIT_FAILURE);
					goto cleanup;
				}
				// Reset the index to our current character (c_index is ahead by one grapheme at this point).
				// We're behind u8_nextchar2, so this u8_dec is safe.
				u8_dec(string, &c_index);
				// But beyond this point, we have to make sure c_index is > 0,
				// because u8_dec would happily blow past our buffer, and leave us with an underflowed c_index...
				if (c_index > 0) {
					// If the current glyph is a space, handle that now.
					if (brk_buff[c_index] == LINEBREAK_ALLOWBREAK) {
						tmp_c_index = c_index;
						u8_dec(string, &tmp_c_index);
						lines[line].endCharIndex = tmp_c_index;
						lines[line].has_a_break  = true;
						LOG("Can break on current whitespace @ #%zu", c_index);
						u8_inc(string, &c_index);
						break;
					} else {
						// Note, we need to do this a second time, to get the previous character,
						// as u8_nextchar2() 'consumes' a character.
						u8_dec(string, &c_index);
						// Ensure we'll have a hard-break here if we can't find a better opportunity
						lines[line].endCharIndex = c_index;
						lines[line].has_a_break  = true;
						LOG("Flagging a last-resort break @ #%zu", c_index);
						// That u8_dec is safe because startCharIndex will always be >= 0
						for (tmp_c_index = c_index; tmp_c_index > lines[line].startCharIndex;
						     u8_dec(string, &tmp_c_index)) {
							if (brk_buff[tmp_c_index] == LINEBREAK_ALLOWBREAK) {
								c_index                  = tmp_c_index;
								lines[line].endCharIndex = c_index;
								LOG("Found a break @ #%zu", c_index);
								break;
							}
						}
						u8_inc(string, &c_index);
						break;
					}
				} else {
					// Let the EOS check handle it...
					break;
				}
			}
			curr_x += iroundf(sf * (float) adv);
			// Adjust our x position for kerning, because we can (assuming there's a next char, of course) :)
			if (c_index < str_len_bytes) {
				tmp_c_index   = c_index;
				uint32_t c2   = u8_nextchar2(string, &tmp_c_index);
				int      g2i  = stbtt_FindGlyphIndex(curr_font, (int) c2);
				int      xadv = stbtt_GetGlyphKernAdvance(curr_font, gi, g2i);
				curr_x += iroundf(sf * (float) xadv);
			}
		}
		// Remember the widest line
		if (unlikely(fit)) {
			fit->bbox.width = (unsigned short int) MAX(fit->bbox.width, lw);
		}
		// We've run out of string! This is our last line.
		if (c_index >= str_len_bytes) {
			// Don't clobber an existing legitimate break...
			if (!lines[line].has_a_break) {
				// That u8_dec is safe because str_len_bytes will always be > 0 as we abort on an empty string.
				u8_dec(string, &c_index);
				lines[line].endCharIndex = c_index;
				lines[line].has_a_break  = true;
				LOG("Flagging a mandatory break at EOS @ #%zu", c_index);
			}
			complete_str = true;
			break;
		}
	}
	const unsigned int computed_lines_amount = line + complete_str;
	LOG("%u lines to be printed", computed_lines_amount);
	if (!complete_str) {
		LOG("String too long. Truncated to ~%zu characters", c_index);
	}
	// Let's determine our exact height, so we can determine vertical alignment later if required.
	LOG("Maximum printable height is %u", print_height);
	unsigned int curr_print_height = 0U;
	for (line = 0U; line < num_lines; line++) {
		if (!lines[line].line_used) {
			break;
		}
		if (curr_print_height + (unsigned int) max_line_height > print_height) {
			// This line can't be printed, so set it to unused
			lines[line].line_used = false;
			break;
		}
		curr_print_height += (unsigned int) max_line_height;
		if (line <= num_lines - 1) {
			if (line == num_lines - 1 || !lines[line + 1].line_used) {
				// Last line, we don't want to add a line gap
				lines[line].line_gap = 0;
				break;
			}
		}
		// We only add a line gap if there's room for one
		if (!(curr_print_height + (unsigned int) lines[line].line_gap > print_height)) {
			curr_print_height += (unsigned int) lines[line].line_gap;
		}
	}
	LOG("Actual print height is %u", curr_print_height);

	// Remember that in fit if it's a valid pointer...
	if (unlikely(fit)) {
		fit->computed_lines = (unsigned short int) computed_lines_amount;
		fit->bbox.height    = (unsigned short int) curr_print_height;
		fit->truncated      = !complete_str;
	}

	// Abort early if we detected a truncation and the user flagged that as a failure.
	if (cfg->no_truncation && !complete_str) {
		LOG("Requested early abort on truncation!");
		rv = ERRCODE(ENOSPC);
		goto cleanup;
	}

	// If we only asked for a computation pass, abort now (successfully).
	if (unlikely(cfg->compute_only)) {
		LOG("Requested early abort after computation pass");
		rv = EXIT_SUCCESS;
		goto cleanup;
	}

	// Let's get some rendering options from FBInkConfig
	uint8_t valign      = NONE;
	uint8_t halign      = NONE;
	bool    is_inverted = false;
	bool    is_overlay  = false;
	bool    is_bgless   = false;
	bool    is_fgless   = false;
	bool    is_centered = false;
	bool    is_halfway  = false;
	if (fbink_cfg) {
		valign      = fbink_cfg->valign;
		halign      = fbink_cfg->halign;
		is_inverted = fbink_cfg->is_inverted;
		is_overlay  = fbink_cfg->is_overlay;
		is_bgless   = fbink_cfg->is_bgless;
		is_fgless   = fbink_cfg->is_fgless;
		is_flashing = fbink_cfg->is_flashing;
		is_cleared  = fbink_cfg->is_cleared;
		is_centered = fbink_cfg->is_centered;
		is_halfway  = fbink_cfg->is_halfway;
	} else {
		is_centered = cfg->is_centered;
	}

	// Hopefully, we have some lines to render!

	// Create a bitmap buffer to render a single line.
	// We don't render the glyphs directly to the fb here, as we need to do some simple blending,
	// and it makes it easier to calculate our centering if required.
	line_buff                = calloc(max_lw * (size_t) max_line_height, sizeof(*line_buff));
	// We also don't want to be creating a new buffer for every glyph, so make it roomy, just in case...
	size_t glyph_buffer_dims = font_size_px * (size_t) max_line_height * 2U;
	glyph_buff               = calloc(glyph_buffer_dims, sizeof(*glyph_buff));
	if (!line_buff || !glyph_buff) {
		PFWARN("Line or glyph buffers could not be allocated: %m");
		rv = ERRCODE(EXIT_FAILURE);
		goto cleanup;
	}
	LOG("Max LW: %hu  Max LH: %d  Max BL: %d  FntSize: %hu", max_lw, max_line_height, max_baseline, font_size_px);

	// Setup the variables needed to render
	FBInkCoordinates curr_point  = { 0U, 0U };
	FBInkCoordinates ins_point   = { 0U, 0U };
	FBInkCoordinates paint_point = { area.tl.x, area.tl.y };
	// Set the vertical positioning now
	if (is_halfway || valign == CENTER) {
		paint_point.y = (unsigned short int) (paint_point.y + ((print_height - curr_print_height) / 2U));
	} else if (valign == EDGE) {
		paint_point.y = (unsigned short int) (paint_point.y + print_height - curr_print_height);
	}
	// Setup our eink refresh region now. We will call refresh during cleanup.
	if (is_centered || halign == CENTER) {
		region.left = area.tl.x + ((uint32_t) (area.br.x - area.tl.x) / 2U);
	} else if (halign == EDGE) {
		region.left = area.br.x;
	} else {
		region.left = paint_point.x;
	}
	region.top = paint_point.y;

	const uint8_t   invert     = is_inverted ? 0xFFu : 0U;
	const uint8_t   fgcolor    = penFGColor ^ invert;
	const uint8_t   bgcolor    = penBGColor ^ invert;
	const short int layer_diff = (short int) (fgcolor - bgcolor);
	FBInkPixel      fgP        = penFGPixel;
	FBInkPixel      bgP        = penBGPixel;
	if (is_inverted) {
		// NOTE: And, of course, RGB565 is terrible. Inverting the lossy packed value would be even lossier...
		if (vInfo.bits_per_pixel == 16U) {
			fgP.rgb565 = pack_rgb565(fgcolor, fgcolor, fgcolor);
			bgP.rgb565 = pack_rgb565(bgcolor, bgcolor, bgcolor);
		} else {
			fgP.bgra.p ^= 0x00FFFFFFu;
			bgP.bgra.p ^= 0x00FFFFFFu;
		}
	}

	// Do we need to clear the screen?
	if (is_cleared) {
		clear_screen(fbfd, bgcolor, is_flashing);
	}

	// Handle padding related region tweaks
	if (cfg->padding == HORI_PADDING) {
		region.left = area.tl.x;
	} else if (cfg->padding == VERT_PADDING) {
		region.top = area.tl.y;
	} else if (cfg->padding == FULL_PADDING) {
		region.left = area.tl.x;
		region.top  = area.tl.y;
		// That's easy enough, simply fill the drawing area with the bg color before rendering anything
		if (!is_overlay && !is_bgless) {
			(*fxpFillRect)((unsigned short int) region.left,
				       (unsigned short int) region.top,
				       max_lw,
				       (unsigned short int) print_height,
				       &bgP);
		}
	}

	uint32_t tmp_c;
	int      tmp_gi;
	unsigned char* restrict lnPtr = NULL;
	unsigned char* restrict glPtr = NULL;
	unsigned short int start_x    = area.tl.x;

	bool abort_line = false;
	// Render!
	for (line = 0U; line < num_lines; line++) {
		if (!lines[line].line_used) {
			break;
		}
		// We have run out of (vertical) printable area, most likely due to incorrect font metrics in the font.
		if (abort_line) {
			break;
		}
		lw        = 0U;
		size_t ci = lines[line].startCharIndex;
		while (ci <= lines[line].endCharIndex) {
			if (cfg->is_formatted) {
				if (fmt_buff[ci] == CH_IGNORE) {
					u8_inc(string, &ci);
					continue;
				} else {
					switch (fmt_buff[ci]) {
						case CH_REGULAR:
							curr_font = ot_fonts->otRegular;
							sf        = rgSF;
							break;
						case CH_ITALIC:
							curr_font = ot_fonts->otItalic;
							sf        = itSF;
							break;
						case CH_BOLD:
							curr_font = ot_fonts->otBold;
							sf        = bdSF;
							break;
						case CH_BOLD_ITALIC:
							curr_font = ot_fonts->otBoldItalic;
							sf        = bditSF;
							break;
					}
				}
			}
			curr_point.y = ins_point.y = (unsigned short int) max_baseline;
			c                          = u8_nextchar2(string, &ci);
			gi                         = stbtt_FindGlyphIndex(curr_font, (int) c);
			stbtt_GetGlyphHMetrics(curr_font, gi, &adv, &lsb);
			stbtt_GetGlyphBitmapBox(curr_font, gi, sf, sf, &x0, &y0, &x1, &y1);
			gw = x1 - x0;
			gh = y1 - y0;
			// Ensure that our glyph size does not exceed the buffer size. Resize the buffer if it does
			if ((gw * gh) > (int) glyph_buffer_dims) {
				size_t new_buff_size = (size_t) gw * (size_t) gh * 2U * sizeof(*glyph_buff);
				LOG("Growing glyph buffer size from %zu to %zu", glyph_buffer_dims, new_buff_size);
				unsigned char* restrict tmp_g_buff = NULL;
				tmp_g_buff                         = realloc(glyph_buff, new_buff_size);
				if (!tmp_g_buff) {
					ELOG("Failure resizing glyph buffer");
					rv = ERRCODE(EXIT_FAILURE);
					goto cleanup;
				}
				glyph_buff        = tmp_g_buff;
				glyph_buffer_dims = new_buff_size;
			}
			// Make sure we don't have an underflow/wrap around
			cx = (int) curr_point.x;
			if (cx + x0 < 0) {
				curr_point.x = (unsigned short int) (curr_point.x + abs(cx + x0));
			}
			// Same on the vertical axis, except we'll prefer clipping the top of the glpyh off,
			// instead of an unsightly vertical shift towards the bottom if we were to tweak the insertion point.
			cy = (int) curr_point.y;
			if (cy + y0 < 0) {
				unsigned short int vclip = (unsigned short int) abs(cy + y0);
				LOG("Clipping %hupx off the top of this glpyh", vclip);
				// Clip it
				gh -= vclip;
				// Fudge positioning so we don't underflow
				y0 += vclip;
			}
			ins_point.x = (unsigned short int) (curr_point.x + x0);
			ins_point.y = (unsigned short int) (curr_point.y + y0);
			// We only increase the lw if the glyph is not a space.
			// This hopefully prevent trailing spaces from being printed on a line.
			if (gw != 0) {
				lw = ins_point.x + (unsigned int) gw;
			} else {
				// NOTE: Slightly hackish attempt at preventing clipping on the final character of a line...
				if (ci < lines[line].endCharIndex) {
					// We're not the final character, business as usual.
					lw = ins_point.x;
				} else {
					// We're the final character, and we're a space...
					// If the last visible glyph was one with protruding serifs (LSB 0, RSB > 0),
					// that could potentially make LW go backwards,
					// clipping a few pixels off the right edge of the glyph.
					// (Most likely because the glyph's advance positions us inside the glyph box, by design).
					// NOTE: Check the 'Current Measured LW' logs for a LW going backwards,
					//       to pinpoint where this might kick in!
					if (lw < ins_point.x) {
						// Growing LW, honor it.
						lw = ins_point.x;
					} else {
						// Decreasing LW, keep the previous one -> NOP!
						if (lw > max_lw) {
							// That should never happen, but better be safe than sorry...
							// Plus, we don't want to abort in the actual LW bounding check later.
							// We know we won't risk segfaulting,
							// because we're honoring a previous gw ;).
							WARN(
							    "Clamping LW (%u) to Max (%hu) after edge clipping workaround",
							    lw,
							    max_lw);
							lw = max_lw;
						}
					}
				}
				// NOTE: We only do this dance in the second render pass, because in the first compute one,
				//       we don't know yet if a space will actually be the last character of a line ;).
				// NOTE: As should be obvious since we're in the "I'm a space!" branch,
				//       This only happens when the last character of a line is a space,
				//       which is common with linebreaking.
				//       e.g., with my trusty Bookerly, at whatever size it needs to be to break (here, 36),
				//       "I'm waiting for" will never cause any issue, but "I'm waiting for ",
				//       (note the final space) *will* ;).
			}

			// Just in case our arithmetic was off by a pixel or two...
			// Note that we are deliberately using a slightly shorter line width during the measurement phase,
			// so this should not happen.
			// If it does occur, we will now exit instead of clipping the glyph bounding box,
			// to avoid the possibility of stb_truetype segfaulting.
			if (lw > max_lw) {
				WARN("Max allowed line width exceeded");
				WARN("Curr LW: %u   Max Allowed: %hu", lw, max_lw);
				rv = ERRCODE(EXIT_FAILURE);
				goto cleanup;
			}
			if (gw != 0 && fgcolor != bgcolor) {
				// Because the stbtt_MakeGlyphBitmap documentation is a bit vague on this point,
				// the parameter 'out_stride' should be the width of the surface in our buffer.
				// It's designed so that the glyph can be rendered directly to a screen buffer.
				// For example, if we were rendering directly to a 1080x1440 screen,
				// out_stride should be set to 1080.
				// In this case however, we want to render to a 'box' of the dimensions of the glyph,
				// so we set 'out_stride' to the glyph width.
				stbtt_MakeGlyphBitmap(curr_font, glyph_buff, gw, gh, gw, sf, sf, gi);
				// paint our glyph into the line buffer
				lnPtr = line_buff + ins_point.x + (max_lw * ins_point.y);
				glPtr = glyph_buff;
				// NOTE: We keep storing it as an alpha coverage mask, we'll blend it in the final rendering stage
				for (int j = 0; j < gh; j++) {
					for (int k = 0; k < gw; k++) {
						// NOTE: We skip:
						//       * 0 value pixels, because they're transparent (no coverage),
						//         and our line buffer is already filled with zeroes ;).
						//       * 1-254 value pixels (i.e., AA), when they're on top of an
						//         already set pixel, (e.g., with 'fl' in serif fonts,
						//         bits of the l's LSB may be positioned over the f's RSB),
						//         as alpha-blending wouldn't net us much in these very few cases.
						if (glPtr[k] != 0U) {
							if (glPtr[k] != 0xFFu && lnPtr[k] != 0U) {
								// Glyph AA would be clobbering the line buffer -> NOP!
							} else {
								lnPtr[k] = glPtr[k];
							}
						}
					}
					// And advance one scanline. Quick! Hide! Pointer arithmetic
					glPtr += gw;
					lnPtr += max_lw;
				}
			}
			curr_point.x = (unsigned short int) (curr_point.x + iroundf(sf * (float) adv));
			if (ci < str_len_bytes) {
				size_t tmp_i = ci;
				tmp_c        = u8_nextchar2(string, &tmp_i);
				tmp_gi       = stbtt_FindGlyphIndex(curr_font, (int) tmp_c);
				int xadv     = stbtt_GetGlyphKernAdvance(curr_font, gi, tmp_gi);
				curr_point.x = (unsigned short int) (curr_point.x + iroundf(sf * (float) xadv));
			}
		}
		curr_point.x = 0U;
		// Right, we've rendered a line to a bitmap, time to display it.
		if (is_centered || halign == CENTER) {
			paint_point.x = (unsigned short int) (paint_point.x + ((max_lw - lw) / 2U));
			if (paint_point.x < region.left) {
				region.left  = paint_point.x;
				region.width = lw;
			}
		} else if (halign == EDGE) {
			paint_point.x = (unsigned short int) (paint_point.x + (max_lw - lw));
			if (paint_point.x < region.left) {
				region.left  = paint_point.x;
				region.width = lw;
			}
		} else if (lw > region.width) {
			region.width = lw;
		}
		// Handle padding...
		if (cfg->padding == HORI_PADDING) {
			region.left  = area.tl.x;
			region.width = max_lw;
			// Unless we're in a backgroundless drawing mode, draw the padding rectangles...
			if (!is_overlay && !is_bgless) {
				// NOTE: Line gaps (if any...) are left alone! Use full padding if that's an issue.
				// Left padding (left edge of the drawing area to initial pen position)
				(*fxpFillRect)((unsigned short int) region.left,
					       paint_point.y,
					       (unsigned short int) (paint_point.x - region.left),
					       (unsigned short int) max_line_height,
					       &bgP);
				// Right padding (final pen position to the right edge of the drawing area)
				(*fxpFillRect)((unsigned short int) (paint_point.x + lw),
					       paint_point.y,
					       (unsigned short int) (viewWidth - (paint_point.x + lw)),
					       (unsigned short int) max_line_height,
					       &bgP);
			}
		} else if (cfg->padding == VERT_PADDING) {
			region.top    = area.tl.y;
			region.height = print_height;
			if (!is_overlay && !is_bgless) {
				// First line? Top padding (top edge of the drawing area to initial pen position)
				if (line == 0U) {
					(*fxpFillRect)(paint_point.x,
						       (unsigned short int) region.top,
						       (unsigned short int) lw,
						       (unsigned short int) (paint_point.y - region.top),
						       &bgP);
				}
			}
		} else if (cfg->padding == FULL_PADDING) {
			region.left   = area.tl.x;
			region.top    = area.tl.y;
			region.width  = max_lw;
			region.height = print_height;
		}

		FBInkPixel pixel;
		pixel.bgra.color.a = 0xFFu;
		start_x            = paint_point.x;
		lnPtr              = line_buff;
		// Normal painting to framebuffer. Please forgive the code repetition. Performance...
		// What we get from stbtt is an alpha coverage mask, hence the need for alpha-blending for anti-aliasing.
		// As it's obviously expensive, we try to avoid it if possible (on fully opaque & fully transparent pixels).
		if (!is_overlay && !is_fgless && !is_bgless) {
			if (abs(layer_diff) == 0xFFu) {
				// If we're painting in B&W, use the mask as-is, it's already B&W ;).
				// We just need to invert it ;).
				uint8_t ainv = 0xFFu;
#	ifdef FBINK_FOR_KINDLE
				if ((deviceQuirks.isKindleLegacy && !is_inverted) ||
				    (!deviceQuirks.isKindleLegacy && is_inverted)) {
#	else
				if (is_inverted) {
#	endif
					ainv = 0U;
				}
				for (int j = 0; j < max_line_height; j++) {
					for (unsigned int k = 0U; k < lw; k++) {
						pixel.bgra.color.r = pixel.bgra.color.g = pixel.bgra.color.b =
						    lnPtr[k] ^ ainv;
						put_pixel(paint_point, &pixel, false);
						paint_point.x++;
					}
					lnPtr += max_lw;
					paint_point.x = start_x;
					paint_point.y++;
				}
			} else {
				const uint16_t pmul_bg = (uint16_t) (bgcolor * 0xFFu);
				for (int j = 0; j < max_line_height; j++) {
					for (unsigned int k = 0U; k < lw; k++) {
						if (lnPtr[k] == 0U) {
							// No coverage (transparent) -> background
							put_pixel(paint_point, &bgP, true);
						} else if (lnPtr[k] == 0xFFu) {
							// Full coverage (opaque) -> foreground
							put_pixel(paint_point, &fgP, true);
						} else {
							// AA, blend it using the coverage mask as alpha
							pixel.bgra.color.r = pixel.bgra.color.g = pixel.bgra.color.b =
							    (uint8_t) DIV255((pmul_bg + (layer_diff * lnPtr[k])));
							put_pixel(paint_point, &pixel, false);
						}
						paint_point.x++;
					}
					lnPtr += max_lw;
					paint_point.x = start_x;
					paint_point.y++;
				}
			}
		} else if (is_fgless) {
			FBInkPixel     fb_px   = { 0U };
			const uint16_t pmul_bg = (uint16_t) (bgcolor * 0xFFu);
			// NOTE: One more branch needed because 4bpp fbs are terrible...
			if (vInfo.bits_per_pixel > 4U) {
				// 8, 16, 24 & 32bpp
				for (int j = 0; j < max_line_height; j++) {
					for (unsigned int k = 0U; k < lw; k++) {
						if (lnPtr[k] == 0U) {
							// No coverage (transparent) -> background
							put_pixel(paint_point, &bgP, true);
						} else if (lnPtr[k] != 0xFFu) {
							// AA, blend it using the coverage mask as alpha,
							// and the underlying pixel as fg
							get_pixel(paint_point, &fb_px);
							pixel.bgra.color.r = (uint8_t) DIV255(
							    (pmul_bg + ((fb_px.bgra.color.r - bgcolor) * lnPtr[k])));
							pixel.bgra.color.g = (uint8_t) DIV255(
							    (pmul_bg + ((fb_px.bgra.color.g - bgcolor) * lnPtr[k])));
							pixel.bgra.color.b = (uint8_t) DIV255(
							    (pmul_bg + ((fb_px.bgra.color.b - bgcolor) * lnPtr[k])));
							put_pixel(paint_point, &pixel, false);
						}
						paint_point.x++;
					}
					lnPtr += max_lw;
					paint_point.x = start_x;
					paint_point.y++;
				}
			} else {
				// 4bpp... We'll have to alpha-blend *everything* to avoid clobbering pixels...
				for (int j = 0; j < max_line_height; j++) {
					for (unsigned int k = 0U; k < lw; k++) {
						// AA, blend it using the coverage mask as alpha, and the underlying pixel as fg
						get_pixel(paint_point, &fb_px);
						pixel.gray8 =
						    (uint8_t) DIV255((pmul_bg + ((fb_px.gray8 - bgcolor) * lnPtr[k])));
						put_pixel(paint_point, &pixel, false);
						paint_point.x++;
					}
					lnPtr += max_lw;
					paint_point.x = start_x;
					paint_point.y++;
				}
			}
		} else if (is_overlay) {
			FBInkPixel fb_px = { 0U };
			if (vInfo.bits_per_pixel > 4U) {
				// 8, 16, 24 & 32bpp
				for (int j = 0; j < max_line_height; j++) {
					for (unsigned int k = 0U; k < lw; k++) {
						if (lnPtr[k] == 0xFFu) {
							// Full coverage (opaque) -> foreground
							get_pixel(paint_point, &fb_px);
							// We want our foreground to be the inverse of the underlying pixel...
							pixel.bgra.p = fb_px.bgra.p ^ 0x00FFFFFFu;
							put_pixel(paint_point, &pixel, false);
						} else if (lnPtr[k] != 0U) {
							// AA, blend it using the coverage mask as alpha,
							// and the underlying pixel as bg
							// Without forgetting our foreground color trickery...
							get_pixel(paint_point, &fb_px);
							pixel.bgra.color.r = (uint8_t) DIV255(
							    (MUL255(fb_px.bgra.color.r) +
							     (((fb_px.bgra.color.r ^ 0xFF) - fb_px.bgra.color.r) *
							      lnPtr[k])));
							pixel.bgra.color.g = (uint8_t) DIV255(
							    (MUL255(fb_px.bgra.color.g) +
							     (((fb_px.bgra.color.g ^ 0xFF) - fb_px.bgra.color.g) *
							      lnPtr[k])));
							pixel.bgra.color.b = (uint8_t) DIV255(
							    (MUL255(fb_px.bgra.color.b) +
							     (((fb_px.bgra.color.b ^ 0xFF) - fb_px.bgra.color.b) *
							      lnPtr[k])));
							put_pixel(paint_point, &pixel, false);
						}
						paint_point.x++;
					}
					lnPtr += max_lw;
					paint_point.x = start_x;
					paint_point.y++;
				}
			} else {
				// 4bpp...
				for (int j = 0; j < max_line_height; j++) {
					for (unsigned int k = 0U; k < lw; k++) {
						// AA, blend it using the coverage mask as alpha, and the underlying pixel as bg
						// Without forgetting our foreground color trickery...
						get_pixel(paint_point, &fb_px);
						pixel.gray8 =
						    (uint8_t) DIV255((MUL255(fb_px.gray8) +
								      (((fb_px.gray8 ^ 0xFF) - fb_px.gray8) * lnPtr[k])));
						put_pixel(paint_point, &pixel, false);
						paint_point.x++;
					}
					lnPtr += max_lw;
					paint_point.x = start_x;
					paint_point.y++;
				}
			}
		} else if (is_bgless) {
			FBInkPixel fb_px = { 0U };
			if (vInfo.bits_per_pixel > 4U) {
				// 8, 16, 24 & 32bpp
				for (int j = 0; j < max_line_height; j++) {
					for (unsigned int k = 0U; k < lw; k++) {
						if (lnPtr[k] == 0xFFu) {
							// Full coverage (opaque) -> foreground
							put_pixel(paint_point, &fgP, true);
						} else if (lnPtr[k] != 0U) {
							// AA, blend it using the coverage mask as alpha,
							// and the underlying pixel as bg
							get_pixel(paint_point, &fb_px);
							pixel.bgra.color.r = (uint8_t) DIV255(
							    (MUL255(fb_px.bgra.color.r) +
							     ((fgcolor - fb_px.bgra.color.r) * lnPtr[k])));
							pixel.bgra.color.g = (uint8_t) DIV255(
							    (MUL255(fb_px.bgra.color.g) +
							     ((fgcolor - fb_px.bgra.color.g) * lnPtr[k])));
							pixel.bgra.color.b = (uint8_t) DIV255(
							    (MUL255(fb_px.bgra.color.b) +
							     ((fgcolor - fb_px.bgra.color.b) * lnPtr[k])));
							put_pixel(paint_point, &pixel, false);
						}
						paint_point.x++;
					}
					lnPtr += max_lw;
					paint_point.x = start_x;
					paint_point.y++;
				}
			} else {
				// 4bpp...
				for (int j = 0; j < max_line_height; j++) {
					for (unsigned int k = 0U; k < lw; k++) {
						// AA, blend it using the coverage mask as alpha, and the underlying pixel as bg
						get_pixel(paint_point, &fb_px);
						pixel.gray8 = (uint8_t) DIV255(
						    (MUL255(fb_px.gray8) + ((fgcolor - fb_px.gray8) * lnPtr[k])));
						put_pixel(paint_point, &pixel, false);
						paint_point.x++;
					}
					lnPtr += max_lw;
					paint_point.x = start_x;
					paint_point.y++;
				}
			}
		}

		paint_point.y = (unsigned short int) (paint_point.y + lines[line].line_gap);
		paint_point.x = area.tl.x;
		if (paint_point.y + max_line_height > area.br.y) {
			abort_line = true;
			LOG("Ran out of drawing area at the end of line# %u after ~%zu characters!", line, ci);
		}
		// Don't fudge region again if we're padding vertically
		if (cfg->padding != VERT_PADDING && cfg->padding != FULL_PADDING) {
			region.height += (unsigned int) max_line_height;
			if (region.top + region.height > screenHeight) {
				region.height = (screenHeight - region.top);
			}
		}
		LOG("Finished printing line# %u", line);
		// And clear our line buffer for next use. The glyph buffer shouldn't need clearing,
		// as stbtt_MakeGlyphBitmap() should overwrite it.
		// NOTE: Fill it with 0 (no coverage -> background)
		memset(line_buff, 0, (max_lw * (size_t) max_line_height * sizeof(*line_buff)));
	}
	// Now that we're sure we've got nothing left to print, handle bottom padding...
	if (cfg->padding == VERT_PADDING) {
		if (!is_overlay && !is_bgless) {
			// Final line? Bottom padding (final pen position to bottom edge of the drawing area)
			// NOTE: Top padding is based on the first line's width, and bottom padding on the last line's width.
			//       Ideally, both would be based on the widest line's width instead, but,
			//       since we're rendering/computing line width line by line, we can't do that ;).
			//       This makes vertical padding a bit gimmicky in practice.
			//       Thankfully, there's a full padding mode available ;).
			(*fxpFillRect)(start_x,
				       paint_point.y,
				       (unsigned short int) lw,
				       (unsigned short int) ((viewHeight + (uint32_t) (viewVertOrigin - viewVertOffset)) -
							     paint_point.y),
				       &bgP);
		}
	}
	if (paint_point.y + max_line_height > area.br.y) {
		rv = 0;    // Inform the caller there is no room left to print another row.
	} else {
		rv = paint_point.y;    // inform the caller what their next top margin should be to follow on
		// NOTE: Don't forget to subtract the hardware viewport shenanigans, to avoid applying the viewport twice,
		//       since the next print_ot call will add this to top_margin again...
		rv -= (viewVertOrigin - viewVertOffset);
		// NOTE: The same idea applies to vertical centering, we don't want to apply it twice...
		if (is_halfway || valign == CENTER) {
			rv = rv - (int) ((print_height - curr_print_height) / 2U);
			rv = rv + (int) curr_print_height;
		}
	}
	// Recap the actual amount of printed lines, as broken metrics may affect what we initially computed ;).
	LOG("Printed %u visible lines", line);

	// Remember that in fit if it's a valid pointer...
	if (unlikely(fit)) {
		fit->rendered_lines = (unsigned short int) line;
	}

	// Warn if there was an unforeseen truncation (potentially because of broken metrics)...
	if (line < computed_lines_amount) {
		LOG("Rendering took more space than expected, string was truncated to %u lines instead of %u!",
		    line,
		    computed_lines_amount);

		// Remember that in fit if it's a valid pointer...
		if (unlikely(fit)) {
			fit->truncated = true;
		}

		// Abort if the user flagged that as a failure.
		if (cfg->no_truncation) {
			LOG("Requested late abort on truncation!");
			rv = ERRCODE(ENOSPC);
			// NOTE: Do *NOT* inhibit the pending refresh, to avoid screwing with set_last_rect ;).
			goto cleanup;
		}
	}
cleanup:
	// Rotate our eink refresh region before refreshing
	LOG("Refreshing region from LEFT: %u, TOP: %u, WIDTH: %u, HEIGHT: %u",
	    region.left,
	    region.top,
	    region.width,
	    region.height);
	if (region.width > 0U && region.height > 0U) {
		// Handle the last rect stuff...
		set_last_rect(&region);
		(*fxpRotateRegion)(&region);
		// NOTE: If we asked for a clear screen, fudge the region at the last moment,
		// so we don't get mangled by previous adjustments...
		if (is_cleared) {
			fullscreen_region(&region);
		}
		refresh_compat(fbfd, region, fbink_cfg ? fbink_cfg->no_refresh : false, fbink_cfg);
	}
	free(lines);
	free(brk_buff);
	free(fmt_buff);
	free(line_buff);
	free(glyph_buff);
	if (isFbMapped && !keep_fd) {
		unmap_fb();
	}
	if (!keep_fd) {
		close_fb(fbfd);
	}
	return rv;
#else
	WARN("OpenType support is disabled in this FBInk build");
	return ERRCODE(ENOSYS);
#endif    // FBINK_WITH_OPENTYPE
}

#ifndef FBINK_FOR_LINUX
// Convert our public WFM_MODE_INDEX_T values to an appropriate mxcfb waveform mode constant for the current device
static uint32_t
    get_wfm_mode(WFM_MODE_INDEX_T wfm_mode_index)
{
	uint32_t waveform_mode = WAVEFORM_MODE_AUTO;

	// Parse waveform mode...
#	if defined(FBINK_FOR_KINDLE)
	// Is this a Zelda or a Rex with new waveforms?
	// (MTK also supports those with matching constants).
	const bool has_new_wfm = (deviceQuirks.isKindleZelda || deviceQuirks.isKindleRex || deviceQuirks.isMTK);
	// Is this device running on a MTK SoC?
	const bool has_mtk_wfm = deviceQuirks.isMTK;

	switch (wfm_mode_index) {
		case WFM_INIT:
			waveform_mode = WAVEFORM_MODE_INIT;
			break;
		case WFM_AUTO:
			waveform_mode = WAVEFORM_MODE_AUTO;
			break;
		case WFM_DU:
			waveform_mode = WAVEFORM_MODE_DU;
			break;
		case WFM_GC16:
			waveform_mode = WAVEFORM_MODE_GC16;
			break;
		case WFM_GC4:
			waveform_mode = WAVEFORM_MODE_GC4;
			break;
		case WFM_A2:
			if (has_new_wfm) {
				waveform_mode = WAVEFORM_MODE_ZELDA_A2;
			} else {
				waveform_mode = WAVEFORM_MODE_A2;
			}
			break;
		case WFM_GL16:
			if (has_new_wfm) {
				waveform_mode = WAVEFORM_MODE_ZELDA_GL16;
			} else {
				waveform_mode = WAVEFORM_MODE_GL16;
			}
			break;
		case WFM_REAGL:
			if (has_new_wfm) {
				waveform_mode = WAVEFORM_MODE_ZELDA_REAGL;
			} else {
				waveform_mode = WAVEFORM_MODE_REAGL;
			}
			break;
		case WFM_REAGLD:
			if (has_new_wfm) {
				waveform_mode = WAVEFORM_MODE_ZELDA_REAGLD;
			} else {
				waveform_mode = WAVEFORM_MODE_REAGLD;
			}
			break;
		case WFM_GC16_FAST:
			if (has_new_wfm) {
				waveform_mode = WAVEFORM_MODE_ZELDA_GC16_FAST;
			} else {
				waveform_mode = WAVEFORM_MODE_GC16_FAST;
			}
			break;
		case WFM_GL16_FAST:
			if (has_new_wfm) {
				waveform_mode = WAVEFORM_MODE_ZELDA_GL16_FAST;
			} else {
				waveform_mode = WAVEFORM_MODE_GL16_FAST;
			}
			break;
		case WFM_DU4:
			waveform_mode = WAVEFORM_MODE_DU4;
			break;
		case WFM_GL4:
			if (has_new_wfm) {
				waveform_mode = WAVEFORM_MODE_ZELDA_GL4;
			} else {
				waveform_mode = WAVEFORM_MODE_GL4;
			}
			break;
		case WFM_GL16_INV:
			if (has_new_wfm) {
				waveform_mode = WAVEFORM_MODE_ZELDA_GL16_INV;
			} else {
				waveform_mode = WAVEFORM_MODE_GL16_INV;
			}
			break;
		case WFM_GCK16:
			if (deviceQuirks.hasEclipseWfm) {
				waveform_mode = WAVEFORM_MODE_ZELDA_GCK16;
			} else {
				waveform_mode = WAVEFORM_MODE_GC16;
			}
			break;
		case WFM_GLKW16:
			if (deviceQuirks.hasEclipseWfm) {
				waveform_mode = WAVEFORM_MODE_ZELDA_GLKW16;
			} else {
				if (has_new_wfm) {
					waveform_mode = WAVEFORM_MODE_ZELDA_GL16;
				} else {
					waveform_mode = WAVEFORM_MODE_GL16;
				}
			}
			break;
		case WFM_GC16_PARTIAL:
			if (has_mtk_wfm) {
				waveform_mode = MTK_WAVEFORM_MODE_GC16_PARTIAL;
			} else {
				waveform_mode = WAVEFORM_MODE_GC16;
			}
			break;
		case WFM_GCK16_PARTIAL:
			if (has_mtk_wfm) {
				waveform_mode = MTK_WAVEFORM_MODE_GCK16_PARTIAL;
			} else {
				if (deviceQuirks.hasEclipseWfm) {
					waveform_mode = WAVEFORM_MODE_ZELDA_GCK16;
				} else {
					waveform_mode = WAVEFORM_MODE_GC16;
				}
			}
			break;
		case WFM_DUNM:
			if (has_mtk_wfm) {
				waveform_mode = MTK_WAVEFORM_MODE_DUNM;
			} else {
				waveform_mode = WAVEFORM_MODE_AUTO;
			}
			break;
		case WFM_P2SW:
			if (has_mtk_wfm) {
				waveform_mode = MTK_WAVEFORM_MODE_P2SW;
			} else {
				waveform_mode = WAVEFORM_MODE_AUTO;
			}
			break;
		default:
			LOG("Unknown (or unsupported) waveform mode '%s' @ index %hhu, defaulting to AUTO",
			    wfm_to_string(wfm_mode_index),
			    wfm_mode_index);
			waveform_mode = WAVEFORM_MODE_AUTO;
			break;
	}
#	elif defined(FBINK_FOR_REMARKABLE)
	// NOTE: Let's go with a dedicated switch for the reMarkable,
	//       because we don't actually have sane constant names in the upstream kernel,
	//       and some of what's detailed in the SDK's <epframebuffer.h> looks slightly weird...
	// NOTE: See https://github.com/NiLuJe/FBInk/pull/41#issuecomment-579012002 if you want to check for yourself ;).
	switch (wfm_mode_index) {
		case WFM_INIT:
			waveform_mode = WAVEFORM_MODE_INIT;
			break;
		case WFM_AUTO:
			waveform_mode = WAVEFORM_MODE_AUTO;
			break;
		case WFM_DU:
			waveform_mode = WAVEFORM_MODE_DU;
			break;
		case WFM_GC16:
			waveform_mode = WAVEFORM_MODE_GC16;
			break;
		case WFM_GL16:
			waveform_mode = WAVEFORM_MODE_GL16;
			break;
		/*
		case WFM_REAGL:
			waveform_mode = WAVEFORM_MODE_GLR16;
			break;
		case WFM_REAGLD:
			waveform_mode = WAVEFORM_MODE_GLD16;
			break;
		*/
		case WFM_A2:
			waveform_mode = WAVEFORM_MODE_A2;
			break;
		/*
		case WFM_DU4:
			waveform_mode = WAVEFORM_MODE_DU4;
			break;
		case WFM_UNKNOWN:
			waveform_mode = WAVEFORM_MODE_UNKNOWN;
			break;
		case WFM_INIT2:
			waveform_mode = WAVEFORM_MODE_INIT2;
			break;
		*/
		default:
			LOG("Unknown (or unsupported) waveform mode '%s' @ index %hhu, defaulting to AUTO",
			    wfm_to_string(wfm_mode_index),
			    wfm_mode_index);
			waveform_mode = WAVEFORM_MODE_AUTO;
			break;
	}
#	elif defined(FBINK_FOR_POCKETBOOK)
	// NOTE: PB has a few extra weird waveform modes, so, go with a dedicated branch...
	switch (wfm_mode_index) {
		case WFM_INIT:
			waveform_mode = WAVEFORM_MODE_INIT;
			break;
		case WFM_AUTO:
			waveform_mode = WAVEFORM_MODE_AUTO;
			break;
		case WFM_DU:
			waveform_mode = WAVEFORM_MODE_DU;
			break;
		case WFM_GC16:
			waveform_mode = WAVEFORM_MODE_GC16;
			break;
		case WFM_GC4:
			waveform_mode = WAVEFORM_MODE_GC4;
			break;
		case WFM_A2:
			waveform_mode = WAVEFORM_MODE_A2;
			break;
		case WFM_GL16:
			waveform_mode = WAVEFORM_MODE_GL16;
			break;
		case WFM_A2IN:
			waveform_mode = WAVEFORM_MODE_A2IN;
			break;
		case WFM_A2OUT:
			waveform_mode = WAVEFORM_MODE_A2OUT;
			break;
		case WFM_DU4:
			waveform_mode = WAVEFORM_MODE_DU4;
			break;
		case WFM_REAGL:
			waveform_mode = WAVEFORM_MODE_REAGL;
			break;
		case WFM_REAGLD:
			waveform_mode = WAVEFORM_MODE_REAGLD;
			break;
		case WFM_GC16HQ:
			waveform_mode = WAVEFORM_MODE_GC16HQ;
			break;
		case WFM_GS16:
			waveform_mode = WAVEFORM_MODE_GS16;
			break;
		default:
			LOG("Unknown (or unsupported) waveform mode '%s' @ index %hhu, defaulting to AUTO",
			    wfm_to_string(wfm_mode_index),
			    wfm_mode_index);
			waveform_mode = WAVEFORM_MODE_AUTO;
			break;
	}
#	elif defined(FBINK_FOR_KOBO)
	if (deviceQuirks.isMTK) {
		switch (wfm_mode_index) {
			case WFM_INIT:
				waveform_mode = HWTCON_WAVEFORM_MODE_INIT;
				break;
			case WFM_DU:
				waveform_mode = HWTCON_WAVEFORM_MODE_DU;
				break;
			case WFM_GC16:
				waveform_mode = HWTCON_WAVEFORM_MODE_GC16;
				break;
			case WFM_GL16:
				waveform_mode = HWTCON_WAVEFORM_MODE_GL16;
				break;
			case WFM_REAGL:
				waveform_mode = HWTCON_WAVEFORM_MODE_GLR16;
				break;
			case WFM_A2:
				waveform_mode = HWTCON_WAVEFORM_MODE_A2;
				break;
			case WFM_GCK16:
				waveform_mode = HWTCON_WAVEFORM_MODE_GCK16;
				break;
			case WFM_GLKW16:
				waveform_mode = HWTCON_WAVEFORM_MODE_GLKW16;
				break;
			case WFM_AUTO:
				waveform_mode = HWTCON_WAVEFORM_MODE_AUTO;
				break;
			default:
				LOG("Unknown (or unsupported) waveform mode '%s' @ index %hhu, defaulting to GL16",
				    wfm_to_string(wfm_mode_index),
				    wfm_mode_index);
				waveform_mode = HWTCON_WAVEFORM_MODE_GL16;
				break;
		}
	} else if (deviceQuirks.isSunxi) {
		switch (wfm_mode_index) {
			case WFM_INIT:
				waveform_mode = EINK_INIT_MODE;
				break;
			case WFM_DU:
				waveform_mode = EINK_DU_MODE;
				break;
			case WFM_GC16:
				waveform_mode = EINK_GC16_MODE;
				break;
			case WFM_GC4:
				waveform_mode = EINK_GC4_MODE;
				break;
			case WFM_A2:
				waveform_mode = EINK_A2_MODE;
				break;
			case WFM_GL16:
				waveform_mode = EINK_GL16_MODE;
				break;
			case WFM_REAGL:
				waveform_mode = EINK_GLR16_MODE;
				break;
			case WFM_REAGLD:
				waveform_mode = EINK_GLD16_MODE;
				break;
			case WFM_GU16:
				waveform_mode = EINK_GU16_MODE;
				break;
			case WFM_GCK16:
				waveform_mode = EINK_GCK16_MODE;
				break;
			case WFM_GLK16:
				waveform_mode = EINK_GLK16_MODE;
				break;
			case WFM_CLEAR:
				waveform_mode = EINK_CLEAR_MODE;
				break;
			case WFM_GC4L:
				waveform_mode = EINK_GC4L_MODE;
				break;
			case WFM_GCC16:
				waveform_mode = EINK_GCC16_MODE;
				break;
			case WFM_AUTO:
				waveform_mode = EINK_AUTO_MODE;
				break;
			default:
				LOG("Unknown (or unsupported) waveform mode '%s' @ index %hhu, defaulting to GL16",
				    wfm_to_string(wfm_mode_index),
				    wfm_mode_index);
				waveform_mode = EINK_GL16_MODE;
				break;
		}
	} else {
		switch (wfm_mode_index) {
			case WFM_INIT:
				waveform_mode = WAVEFORM_MODE_INIT;
				break;
			case WFM_AUTO:
				waveform_mode = WAVEFORM_MODE_AUTO;
				break;
			case WFM_DU:
				waveform_mode = WAVEFORM_MODE_DU;
				break;
			case WFM_GC16:
				waveform_mode = WAVEFORM_MODE_GC16;
				break;
			case WFM_GC4:
				waveform_mode = WAVEFORM_MODE_GC4;
				break;
			case WFM_A2:
				waveform_mode = WAVEFORM_MODE_A2;
				break;
			case WFM_GL16:
				waveform_mode = WAVEFORM_MODE_GL16;
				break;
			case WFM_REAGL:
				waveform_mode = WAVEFORM_MODE_REAGL;
				break;
			case WFM_REAGLD:
				waveform_mode = WAVEFORM_MODE_REAGLD;
				break;
			case WFM_DU4:
				if (deviceQuirks.hasEclipseWfm) {
					// NOTE: Not *technically* related to eclipse waveform modes,
					//       but actually introduced on Kobo at the same time (i.e., Mk. 9).
					waveform_mode = WAVEFORM_MODE_DU4;
				} else {
					waveform_mode = WAVEFORM_MODE_GC4;
				}
				break;
			case WFM_GCK16:
				if (deviceQuirks.hasEclipseWfm) {
					waveform_mode = WAVEFORM_MODE_GCK16;
				} else {
					waveform_mode = WAVEFORM_MODE_GC16;
				}
				break;
			case WFM_GLKW16:
				if (deviceQuirks.hasEclipseWfm) {
					waveform_mode = WAVEFORM_MODE_GLKW16;
				} else {
					waveform_mode = WAVEFORM_MODE_GL16;
				}
				break;
			default:
				LOG("Unknown (or unsupported) waveform mode '%s' @ index %hhu, defaulting to AUTO",
				    wfm_to_string(wfm_mode_index),
				    wfm_mode_index);
				waveform_mode = WAVEFORM_MODE_AUTO;
				break;
		}
	}
#	else
	switch (wfm_mode_index) {
		case WFM_INIT:
			waveform_mode = WAVEFORM_MODE_INIT;
			break;
		case WFM_AUTO:
			waveform_mode = WAVEFORM_MODE_AUTO;
			break;
		case WFM_DU:
			waveform_mode = WAVEFORM_MODE_DU;
			break;
		case WFM_GC16:
			waveform_mode = WAVEFORM_MODE_GC16;
			break;
		case WFM_GC4:
			waveform_mode = WAVEFORM_MODE_GC4;
			break;
		case WFM_A2:
			waveform_mode = WAVEFORM_MODE_A2;
			break;
		case WFM_GL16:
			waveform_mode = WAVEFORM_MODE_GL16;
			break;
		case WFM_REAGL:
			waveform_mode = WAVEFORM_MODE_REAGL;
			break;
		case WFM_REAGLD:
			waveform_mode = WAVEFORM_MODE_REAGLD;
			break;
		default:
			LOG("Unknown (or unsupported) waveform mode '%s' @ index %hhu, defaulting to AUTO",
			    wfm_to_string(wfm_mode_index),
			    wfm_mode_index);
			waveform_mode = WAVEFORM_MODE_AUTO;
			break;
	}
#	endif    // FBINK_FOR_KINDLE

	return waveform_mode;
}

// Convert a WFM_MODE_INDEX_T value to a human readable string
static __attribute__((cold)) const char*
    wfm_to_string(WFM_MODE_INDEX_T wfm_mode_index)
{
	switch (wfm_mode_index) {
		case WFM_AUTO:
			return "AUTO";
		case WFM_DU:
			return "DU";
		case WFM_GC16:
			return "GC16";
		case WFM_GC4:
			return "GC4";
		case WFM_A2:
			return "A2";
		case WFM_GL16:
			return "GL16";
		case WFM_REAGL:
			return "REAGL";
		case WFM_REAGLD:
			return "REAGLD";
		case WFM_GC16_FAST:
			return "GC16 FAST";
		case WFM_GL16_FAST:
			return "GL16 FAST";
		case WFM_DU4:
			return "DU4";
		case WFM_GL4:
			return "GL4";
		case WFM_GL16_INV:
			return "GL16 INVERTED";
		case WFM_GCK16:
			return "GCK16";
		case WFM_GLKW16:
			return "GLKW16";
		case WFM_INIT:
			return "INIT";
		case WFM_UNKNOWN:
			return "UNKNOWN (Highlight?)";
		case WFM_INIT2:
			return "INIT2?";
		case WFM_A2IN:
			return "A2 IN";
		case WFM_A2OUT:
			return "A2 OUT";
		case WFM_GC16HQ:
			return "GC16 HQ";
		case WFM_GS16:
			return "GS16";
		case WFM_GU16:
			return "GU16";
		case WFM_GLK16:
			return "GLK16";
		case WFM_CLEAR:
			return "CLEAR";
		case WFM_GC4L:
			return "GC4L";
		case WFM_GCC16:
			return "GCC16";
		default:
			return "Unknown";
	}
}

#	ifdef FBINK_FOR_KINDLE
// Convert a platform-specifc mxcfb WAVEFORM_MODE value to a human readable string
static __attribute__((cold)) const char*
    kindle_wfm_to_string(uint32_t wfm_mode)
{
	switch (wfm_mode) {
		case WAVEFORM_MODE_INIT:
			return "INIT";
		case WAVEFORM_MODE_DU:
			return "DU";
		case WAVEFORM_MODE_GC16:
			return "GC16";
		case WAVEFORM_MODE_GC16_FAST:
			return "GC16 FAST";
		case WAVEFORM_MODE_A2:
			return "A2";
		case WAVEFORM_MODE_GL16:
			return "GL16";
		case WAVEFORM_MODE_GL16_FAST:
			return "GL16 FAST";
		case WAVEFORM_MODE_DU4:
			return "DU4";
		case WAVEFORM_MODE_REAGL:
			return "REAGL";
		case WAVEFORM_MODE_REAGLD:
			return "REAGLD";
		case WAVEFORM_MODE_GL4:
			return "GL4";
		case WAVEFORM_MODE_GL16_INV:
			return "GL16 INVERTED";
		case WAVEFORM_MODE_AUTO:
			return "AUTO";
		default:
			return "Unknown";
	}
}

// Same, but for Zelda/Rex constants
static __attribute__((cold)) const char*
    kindle_zelda_wfm_to_string(uint32_t wfm_mode)
{
	switch (wfm_mode) {
		case WAVEFORM_MODE_INIT:
			return "INIT";
		case WAVEFORM_MODE_DU:
			return "DU";
		case WAVEFORM_MODE_GC16:
			return "GC16";
		case WAVEFORM_MODE_ZELDA_GL16:
			return "GL16";
		case WAVEFORM_MODE_ZELDA_GLR16:
			return "REAGL";
		case WAVEFORM_MODE_ZELDA_GLD16:
			return "REAGLD";
		case WAVEFORM_MODE_ZELDA_A2:
			return "A2";
		case WAVEFORM_MODE_DU4:
			return "DU4";
		case WAVEFORM_MODE_ZELDA_GCK16:
			return "GCK16";
		case WAVEFORM_MODE_ZELDA_GLKW16:
			return "GLKW16";
		case WAVEFORM_MODE_AUTO:
			return "AUTO";
		default:
			return "Unknown";
	}
}
#	endif    // FBINK_FOR_KINDLE

#	if defined(FBINK_FOR_KOBO)
static __attribute__((cold)) const char*
    ntx_wfm_to_string(uint32_t wfm_mode)
{
	switch (wfm_mode) {
		case WAVEFORM_MODE_INIT:
			return "INIT";
		case WAVEFORM_MODE_DU:
			return "DU";
		case WAVEFORM_MODE_GC16:
			return "GC16";
		case WAVEFORM_MODE_GC4:
			return "GC4";
		case WAVEFORM_MODE_A2:
			return "A2";
		case WAVEFORM_MODE_GL16:
			return "GL16";
		case WAVEFORM_MODE_REAGL:
			return "REAGL";
		case WAVEFORM_MODE_REAGLD:
			return "REAGLD";
		case WAVEFORM_MODE_DU4:
			return "DU4";
		case WAVEFORM_MODE_GCK16:
			return "GCK16";
		case WAVEFORM_MODE_GLKW16:
			return "GLKW16";
		case WAVEFORM_MODE_AUTO:
			return "AUTO";
		default:
			return "Unknown";
	}
}

// NOTE: Currently unnecessary, the ioctl doesn't update the userland data
/*
static __attribute__((cold)) const char*
    mtk_wfm_to_string(uint32_t wfm_mode)
{
	switch (wfm_mode) {
		case HWTCON_WAVEFORM_MODE_INIT:
			return "INIT";
		case HWTCON_WAVEFORM_MODE_DU:
			return "DU";
		case HWTCON_WAVEFORM_MODE_GC16:
			return "GC16";
		case HWTCON_WAVEFORM_MODE_GL16:
			return "GL16";
		case HWTCON_WAVEFORM_MODE_REAGL:
			return "REAGL";
		case HWTCON_WAVEFORM_MODE_A2:
			return "A2";
		case HWTCON_WAVEFORM_MODE_GCK16:
			return "GCK16";
		case HWTCON_WAVEFORM_MODE_GLKW16:
			return "GLKW16";
		case HWTCON_WAVEFORM_MODE_AUTO:
			return "AUTO";
		default:
			return "Unknown";
	}
}
*/
#	endif    // FBINK_FOR_KOBO

#	if defined(FBINK_FOR_CERVANTES)
static __attribute__((cold)) const char*
    ntx_wfm_to_string(uint32_t wfm_mode)
{
	switch (wfm_mode) {
		case WAVEFORM_MODE_INIT:
			return "INIT";
		case WAVEFORM_MODE_DU:
			return "DU";
		case WAVEFORM_MODE_GC16:
			return "GC16";
		case WAVEFORM_MODE_GC4:
			return "GC4";
		case WAVEFORM_MODE_A2:
			return "A2";
		case WAVEFORM_MODE_GL16:
			return "GL16";
		case WAVEFORM_MODE_REAGL:
			return "REAGL";
		case WAVEFORM_MODE_REAGLD:
			return "REAGLD";
		case WAVEFORM_MODE_AUTO:
			return "AUTO";
		default:
			return "Unknown";
	}
}
#	endif    // FBINK_FOR_CERVANTES

#	ifdef FBINK_FOR_REMARKABLE
static __attribute__((cold)) const char*
    remarkable_wfm_to_string(uint32_t wfm_mode)
{
	switch (wfm_mode) {
		case WAVEFORM_MODE_INIT:
			return "INIT";
		case WAVEFORM_MODE_DU:
			return "DU";
		case WAVEFORM_MODE_GC16:
			return "GC16";
		case WAVEFORM_MODE_GL16:
			return "GL16";
		case WAVEFORM_MODE_A2:
			return "A2";
		case WAVEFORM_MODE_AUTO:
			return "AUTO";
		default:
			return "Unknown";
	}
}
#	endif    // FBINK_FOR_REMARKABLE

#	ifdef FBINK_FOR_POCKETBOOK
static __attribute__((cold)) const char*
    pocketbook_wfm_to_string(uint32_t wfm_mode)
{
	switch (wfm_mode) {
		case WAVEFORM_MODE_INIT:
			return "INIT";
		case WAVEFORM_MODE_DU:
			return "DU";
		case WAVEFORM_MODE_GC16:
			return "GC16";
		case WAVEFORM_MODE_GC4:
			return "GC4";
		case WAVEFORM_MODE_A2:
			return "A2";
		case WAVEFORM_MODE_GL16:
			return "GL16";
		case WAVEFORM_MODE_A2IN:
			return "A2IN";
		case WAVEFORM_MODE_A2OUT:
			return "A2OUT";
		case WAVEFORM_MODE_DU4:
			return "DU4";
		case WAVEFORM_MODE_REAGL:
			return "REAGL";
		case WAVEFORM_MODE_REAGLD:
			return "REAGLD";
		case WAVEFORM_MODE_GC16HQ:
			return "GC16HQ";
		case WAVEFORM_MODE_GS16:
			return "GS16";
		case WAVEFORM_MODE_AUTO:
			return "AUTO";
		default:
			return "Unknown";
	}
}
#	endif    // FBINK_FOR_POCKETBOOK

#	if defined(FBINK_FOR_KOBO) || defined(FBINK_FOR_KINDLE)
// Convert our public HW_DITHER_INDEX_T values to an appropriate mxcfb dithering mode constant
static int
    get_hwd_mode(HW_DITHER_INDEX_T hw_dither_index)
{
	// NOTE: This hardware dithering (handled by the PxP on i.MX) is only supported since EPDC v2!
	//       AFAICT, most of our eligible target devices only support PASSTHROUGH & ORDERED...
	//       (c.f., drivers/dma/pxp/pxp_dma_v3.c)
	// NOTE: On newer platforms (sunxi or mtk), if dithering support there is,
	//       it's either implemented in C, or shuffled off to the media processing unit.
	//       Generally, those platforms prefer using "legacy" flags.
	int dither_algo = EPDC_FLAG_USE_DITHERING_PASSTHROUGH;

	// Parse dithering algo...
	switch (hw_dither_index) {
		case HWD_PASSTHROUGH:
			dither_algo = EPDC_FLAG_USE_DITHERING_PASSTHROUGH;
			break;
		case HWD_FLOYD_STEINBERG:
			dither_algo = EPDC_FLAG_USE_DITHERING_FLOYD_STEINBERG;
			break;
		case HWD_ATKINSON:
			dither_algo = EPDC_FLAG_USE_DITHERING_ATKINSON;
			break;
		case HWD_ORDERED:
			dither_algo = EPDC_FLAG_USE_DITHERING_ORDERED;
			break;
		case HWD_QUANT_ONLY:
			dither_algo = EPDC_FLAG_USE_DITHERING_QUANT_ONLY;
			break;
		case HWD_LEGACY:
			// NOTE: We'll fudge that back to PASSTHROUGH inside the platform-specific refresh calls...
			dither_algo = HWD_LEGACY;
			break;
		default:
			LOG("Unknown (or unsupported) dithering mode '%s' @ index %hhu, defaulting to PASSTHROUGH",
			    hwd_to_string(hw_dither_index),
			    hw_dither_index);
			dither_algo = EPDC_FLAG_USE_DITHERING_PASSTHROUGH;
			break;
	}

	return dither_algo;
}

// Convert a HW_DITHER_INDEX_T value to a human readable string
static __attribute__((cold)) const char*
    hwd_to_string(HW_DITHER_INDEX_T hw_dither_index)
{
	switch (hw_dither_index) {
		case HWD_PASSTHROUGH:
			return "PASSTHROUGH";
		case HWD_FLOYD_STEINBERG:
			return "FLOYD STEINBERG";
		case HWD_ATKINSON:
			return "ATKINSON";
		case HWD_ORDERED:
			return "ORDERED";
		case HWD_QUANT_ONLY:
			return "QUANTIZE ONLY";
		case HWD_LEGACY:
			return "LEGACY";
		default:
			return "Unknown";
	}
}
#	endif    // FBINK_FOR_KOBO || FBINK_FOR_KINDLE
#endif            // !FBINK_FOR_LINUX

// Small public wrapper around refresh(), without the caller having to depend on mxcfb headers
int
    fbink_refresh(int fbfd                              UNUSED_BY_LINUX,
		  uint32_t region_top                   UNUSED_BY_LINUX,
		  uint32_t region_left                  UNUSED_BY_LINUX,
		  uint32_t region_width                 UNUSED_BY_LINUX,
		  uint32_t region_height                UNUSED_BY_LINUX,
		  const FBInkConfig* restrict fbink_cfg UNUSED_BY_LINUX)
{
#ifndef FBINK_FOR_LINUX
	// Assume success, until shit happens ;)
	int  rv      = EXIT_SUCCESS;
	bool keep_fd = true;
#	ifdef FBINK_FOR_KOBO
	if (deviceQuirks.isSunxi) {
		// We need the full monty on sunxi...
		if (open_fb_fd(&fbfd, &keep_fd) != EXIT_SUCCESS) {
			return ERRCODE(EXIT_FAILURE);
		}

		if (!isFbMapped) {
			if (memmap_fb(fbfd) != EXIT_SUCCESS) {
				rv = ERRCODE(EXIT_FAILURE);
				goto cleanup;
			}
		}
	} else {
		if (open_fb_fd_nonblock(&fbfd, &keep_fd) != EXIT_SUCCESS) {
			return ERRCODE(EXIT_FAILURE);
		}
	}
#	else
	// Open the framebuffer if need be (nonblock, we'll only do ioctls)...
	if (open_fb_fd_nonblock(&fbfd, &keep_fd) != EXIT_SUCCESS) {
		return ERRCODE(EXIT_FAILURE);
	}
#	endif    // FBINK_FOR_KOBO

	// Same for the dithering mode, if we actually requested dithering...
	if (fbink_cfg->dithering_mode == HWD_ORDERED) {
		LOG("Hardware dithering requested");
	} else if (fbink_cfg->dithering_mode == HWD_LEGACY) {
		LOG("Legacy in-kernel software dithering requested");
	}
	// NOTE: Right now, we enforce quant_bit to what appears to be sane values depending on the waveform mode.
	//       We do the same for actual algorithm selection w/ LEGACY.

	struct mxcfb_rect region = {
		.top    = region_top,
		.left   = region_left,
		.width  = region_width,
		.height = region_height,
	};

	// If region is empty, do a full-screen refresh!
	if (region.top == 0U && region.left == 0U && region.width == 0U && region.height == 0U) {
		fullscreen_region(&region);
	}

	if ((rv = refresh_compat(fbfd, region, false, fbink_cfg)) != EXIT_SUCCESS) {
		PFWARN("Failed to refresh the screen");
	}

#	ifdef FBINK_FOR_KOBO
cleanup:
	if (deviceQuirks.isSunxi) {
		if (isFbMapped && !keep_fd) {
			unmap_fb();
		}
	}
#	endif
	if (!keep_fd) {
		close_fb(fbfd);
	}

	return rv;
#else
	WARN("e-Ink screen refreshes require an e-Ink device");
	return ERRCODE(ENOSYS);
#endif    // !FBINK_FOR_LINUX
}

// Because I just couldn't be arsed to deal with broken out rectangles anymore ;p.
int
    fbink_refresh_rect(int fbfd                              UNUSED_BY_LINUX,
		       const FBInkRect* restrict rect        UNUSED_BY_LINUX,
		       const FBInkConfig* restrict fbink_cfg UNUSED_BY_LINUX)
{
#ifndef FBINK_FOR_LINUX
	return fbink_refresh(fbfd, rect->top, rect->left, rect->width, rect->height, fbink_cfg);
#else
	WARN("e-Ink screen refreshes require an e-Ink device");
	return ERRCODE(ENOSYS);
#endif    // !FBINK_FOR_LINUX
}

// Small public wrapper around wait_for_submission(), without the caller having to depend on mxcfb headers
int
    fbink_wait_for_submission(int fbfd UNUSED_BY_NOTKINDLE, uint32_t marker UNUSED_BY_NOTKINDLE)
{
#if defined(FBINK_FOR_KINDLE) || defined(FBINK_FOR_KOBO)
	// Abort early on unsupported devices
	if (!deviceQuirks.canWaitForSubmission) {
		WARN("Waiting for update submission is not supported on your device");
		return ERRCODE(ENOSYS);
	}

	// Open the framebuffer if need be (nonblock, we'll only do ioctls)...
	bool keep_fd = true;
	if (open_fb_fd_nonblock(&fbfd, &keep_fd) != EXIT_SUCCESS) {
		return ERRCODE(EXIT_FAILURE);
	}

	// Assume success, until shit happens ;)
	int rv = EXIT_SUCCESS;

	// Try to retrieve the last sent marker, if any, if we passed marker 0...
	if (marker == LAST_MARKER) {
		if (lastMarker != LAST_MARKER) {
			marker = lastMarker;
		} else {
			// Otherwise, don't even try to wait for an invalid marker!
			WARN("No previous update found, cannot wait for submission of an invalid update marker");
			rv = ERRCODE(EINVAL);
			goto cleanup;
		}
	}

	if ((rv = wait_for_submission(fbfd, marker)) != EXIT_SUCCESS) {
		WARN("Failed to wait for submission of update %u", marker);
	}

cleanup:
	if (!keep_fd) {
		close_fb(fbfd);
	}

	return rv;
#else
	WARN("Waiting for update submission is not supported on your device");
	return ERRCODE(ENOSYS);
#endif    // FBINK_FOR_KINDLE || FBINK_FOR_KOBO
}

// Small public wrapper around wait_for_complete(), without the caller having to depend on mxcfb headers
int
    fbink_wait_for_complete(int fbfd UNUSED_BY_LINUX, uint32_t marker UNUSED_BY_LINUX)
{
#ifndef FBINK_FOR_LINUX
	// Assume success, until shit happens ;)
	int  rv      = EXIT_SUCCESS;
	bool keep_fd = true;
#	ifdef FBINK_FOR_KOBO
	if (deviceQuirks.isSunxi) {
		// We need the full monty on sunxi...
		if (open_fb_fd(&fbfd, &keep_fd) != EXIT_SUCCESS) {
			return ERRCODE(EXIT_FAILURE);
		}

		if (!isFbMapped) {
			if (memmap_fb(fbfd) != EXIT_SUCCESS) {
				rv = ERRCODE(EXIT_FAILURE);
				goto cleanup;
			}
		}
	} else {
		if (open_fb_fd_nonblock(&fbfd, &keep_fd) != EXIT_SUCCESS) {
			return ERRCODE(EXIT_FAILURE);
		}
	}
#	else
	// Open the framebuffer if need be (nonblock, we'll only do ioctls)...
	if (open_fb_fd_nonblock(&fbfd, &keep_fd) != EXIT_SUCCESS) {
		return ERRCODE(EXIT_FAILURE);
	}
#	endif    // FBINK_FOR_KOBO

	// Try to retrieve the last sent marker, if any, if we passed marker 0...
	if (marker == LAST_MARKER) {
		if (lastMarker != LAST_MARKER) {
			marker = lastMarker;
		} else {
			// Otherwise, don't even try to wait for an invalid marker!
			WARN("No previous update found, cannot wait for completion of an invalid update marker");
			rv = ERRCODE(EINVAL);
			goto cleanup;
		}
	}

	if ((rv = wait_for_complete(fbfd, marker)) != EXIT_SUCCESS) {
		WARN("Failed to wait for completion of update %u", marker);
	}

cleanup:
#	ifdef FBINK_FOR_KOBO
	if (deviceQuirks.isSunxi) {
		if (isFbMapped && !keep_fd) {
			unmap_fb();
		}
	}
#	endif
	if (!keep_fd) {
		close_fb(fbfd);
	}

	return rv;
#else
	WARN("Waiting for update completion requires an e-Ink device");
	return ERRCODE(ENOSYS);
#endif    // !FBINK_FOR_LINUX
}

// Simple public getter for lastMarker
uint32_t
    fbink_get_last_marker(void)
{
#ifndef FBINK_FOR_LINUX
	return lastMarker;
#else
	WARN("Waiting for updates requires an e-Ink device");
	return LAST_MARKER;
#endif    // !FBINK_FOR_LINUX
}

// Simple public getter for temporary Device Quirks
// NOTE: Deprecated, see fbink_reinit instead!
bool
    fbink_is_fb_quirky(void)
{
	// NOTE: For now, that's easy enough, we only have one ;).
	return deviceQuirks.isNTX16bLandscape;
}

#ifdef FBINK_FOR_KOBO
static int
    kobo_sunxi_reinit_check(int fbfd, const FBInkConfig* restrict fbink_cfg)
{
	// Assume success, until shit happens ;)
	int rv = EXIT_SUCCESS;
	// We'll track what triggered the reinit in a bitmask
	int rf = 0;

	// We may need an I²C handle...
	if (fbfd == FBFD_AUTO && sunxiCtx.force_rota < FORCE_ROTA_UR) {
		if (open_accelerometer_i2c() != EXIT_SUCCESS) {
			PFWARN("Cannot open accelerometer I²C handle, aborting");
			return ERRCODE(EXIT_FAILURE);
		}
	}

	const uint32_t old_rota = vInfo.rotate;
	int            rotate;
	if (sunxiCtx.force_rota >= FORCE_ROTA_UR) {
		if (sunxiCtx.force_rota == FORCE_ROTA_WORKBUF) {
			// Attempt to match the working buffer...
			rotate = query_fbdamage();
			if (rotate < 0) {
				ELOG("FBDamage is inconclusive, keeping current rotation");
				rotate = (int) old_rota;
			}
		} else {
			rotate = sunxiCtx.force_rota;
		}
	} else {
		rotate = query_accelerometer();
		if (rotate < 0) {
			ELOG("Accelerometer is inconclusive, keeping current rotation");
			rotate = (int) old_rota;
		}
	}

	// If we're in an FBFD_AUTO workflow, close the I²C handle
	if (fbfd == FBFD_AUTO) {
		if (close_accelerometer_i2c() != EXIT_SUCCESS) {
			WARN("Failed to close accelerometer I²C handle");
			return ERRCODE(EXIT_FAILURE);
		}
	}

	if (old_rota != (uint32_t) rotate) {
		ELOG("Detected a change in framebuffer rotation (%u -> %d)", old_rota, rotate);
		rf |= OK_ROTA_CHANGE;

		// A layout change can only happen after a rotation change ;).
		if ((rotate & 0x01) != (old_rota & 0x01)) {
			// Technically an orientation change, but layout is less likely to be confused w/ rotation ;).
			ELOG("Detected a change in framebuffer layout (%s -> %s)",
			     ((old_rota & 0x01) == 1) ? "Landscape" : "Portrait",
			     ((rotate & 0x01) == 1) ? "Landscape" : "Portrait");
			rf |= OK_LAYOUT_CHANGE;
		}

		// Update the vInfo flag now, so we don't have to poke at the gyro again in initialize_fbink
		vInfo.rotate = (uint32_t) rotate;
	}

	// If our bitmask is not empty, it means we have a reinit to do.
	if (rf > 0) {
		ELOG("Reinitializing...");
		rv = initialize_fbink(fbfd, fbink_cfg, true);

		// If it went fine, make the caller aware of why we did it by returning the bitmask
		if (rv == EXIT_SUCCESS) {
			rv = rf;
		}
	}

	return rv;
}
#endif    // FBINK_FOR_KOBO

// Reinitialize FBInk in case the framebuffer state has changed
// NOTE: We initially (< 1.10.4) tried to limit this to specific scenarios, specifically:
//         * the bitdepth switch during boot between pickel & nickel
//         * eventual rotations during the boot between boot & pickel and/or nickel
//           (c.f., https://www.mobileread.com/forums/showpost.php?p=3764776&postcount=229 for those two points).
//         * rotation in general on devices with an accelerometer
//       However, while that accounts for *stock* behavior, that left some custom setups in the lurch, in particular:
//         * rotation in Nickel may be patched in
//         * custom software might affect bitdepth or rotation on purpose
//           (e.g., Plato relies on HW rotation, and KOReader may change the bitdepth).
//       TL;DR: We now monitor *any* change in bitdepth and/or rotation.
int
    fbink_reinit(int fbfd, const FBInkConfig* restrict fbink_cfg)
{
#ifdef FBINK_FOR_KOBO
	// On sunxi, the framebuffer state is meaningless, so, just do our own thing...
	if (deviceQuirks.isSunxi) {
		return kobo_sunxi_reinit_check(fbfd, fbink_cfg);
	}
#endif

	// So, we're concerned with stuff that affects the logical & physical layout, namely, bitdepth & rotation.
	const uint32_t old_bpp       = vInfo.bits_per_pixel;
	const uint32_t old_rota      = vInfo.rotate;
	// As well as xres & yres to catch layout changes
	const uint32_t old_xres      = vInfo.xres;
	const uint32_t old_yres      = vInfo.yres;
	// We also want to keep track of the grayscale flag to detect global nightmode swaps
	const uint32_t old_grayscale = vInfo.grayscale;

	// Open the framebuffer if need be (nonblock, we'll only do ioctls)...
	bool keep_fd = true;
	if (open_fb_fd_nonblock(&fbfd, &keep_fd) != EXIT_SUCCESS) {
		return ERRCODE(EXIT_FAILURE);
	}

	// Assume success, until shit happens ;)
	int rv = EXIT_SUCCESS;
	// We'll track what triggered the reinit in a bitmask
	int rf = 0;

	// Now that we've stored the relevant bits of the previous state, query the current one...
	if (ioctl(fbfd, FBIOGET_VSCREENINFO, &vInfo)) {
		PFWARN("Error reading variable fb information: %m");
		rv = ERRCODE(EXIT_FAILURE);
		goto cleanup;
	}

#ifdef FBINK_FOR_POCKETBOOK
	// On PocketBook, fix the broken mess that the ioctls returns...
	pocketbook_fix_fb_info();
#endif

	// We want to flag each trigger independently
	if (old_bpp != vInfo.bits_per_pixel) {
		ELOG("Detected a change in framebuffer bitdepth (%u -> %u)", old_bpp, vInfo.bits_per_pixel);
		rf |= OK_BPP_CHANGE;
	}
	if (old_rota != vInfo.rotate) {
		ELOG("Detected a change in framebuffer rotation (%u -> %u)", old_rota, vInfo.rotate);
		rf |= OK_ROTA_CHANGE;

		// A layout change can only happen after a rotation change ;).
		if (old_xres != vInfo.xres || old_yres != vInfo.yres) {
			// Technically an orientation change, but layout is less likely to be confused w/ rotation ;).
			ELOG("Detected a change in framebuffer layout (%ux%u -> %ux%u)",
			     old_xres,
			     old_yres,
			     vInfo.xres,
			     vInfo.yres);
			rf |= OK_LAYOUT_CHANGE;
		}
	}
	// The grayscale flip is only meaningful at 8bpp
	if (old_bpp == 8U && vInfo.bits_per_pixel == 8U && old_grayscale != vInfo.grayscale) {
		ELOG("Detected a change in grayscale state (%u -> %u)", old_grayscale, vInfo.grayscale);
		rf |= OK_GRAYSCALE_CHANGE;
	}

	// If our bitmask is not empty, it means we have a reinit to do, one where we'll skip the vinfo ioctl we just did.
	// NOTE: OK_GRAYSCALE_CHANGE doesn't technically require a reinit, but we do it anyway for simplicity's sake.
	if (rf > 0) {
		ELOG("Reinitializing...");
		rv = initialize_fbink(fbfd, fbink_cfg, true);

		// If it went fine, make the caller aware of why we did it by returning the bitmask
		if (rv == EXIT_SUCCESS) {
			rv = rf;
		}
	}

	// And we're done!
	// We keep silent when we do nothing, to avoid flooding logs (mainly, KFMon's ;)).

cleanup:
	if (!keep_fd) {
		close_fb(fbfd);
	}

	return rv;
}

// Public wrapper around update_pen_colors
int
    fbink_update_pen_colors(const FBInkConfig* restrict fbink_cfg UNUSED_BY_NODRAW)
{
#ifdef FBINK_WITH_DRAW
	return update_pen_colors(fbink_cfg);
#else
	WARN("Drawing primitives are disabled in this FBInk build");
	return ERRCODE(ENOSYS);
#endif
}

// Public wrapper around update_verbosity
void
    fbink_update_verbosity(const FBInkConfig* restrict fbink_cfg)
{
	return update_verbosity(fbink_cfg);
}

#ifdef FBINK_WITH_BITMAP
// Handle drawing both types of progress bars
int
    draw_progress_bars(int fbfd, bool is_infinite, uint8_t value, const FBInkConfig* restrict fbink_cfg)
{
	const uint8_t invert  = fbink_cfg->is_inverted ? 0xFFu : 0U;
	const uint8_t fgcolor = penFGColor ^ invert;
	const uint8_t bgcolor = penBGColor ^ invert;

	// Clear screen?
	if (fbink_cfg->is_cleared) {
		clear_screen(fbfd, bgcolor, fbink_cfg->is_flashing);
	}

	// Let's go! Start by pilfering some computations from draw...
	FBInkPixel fgP = penFGPixel;
	FBInkPixel bgP = penBGPixel;
	if (fbink_cfg->is_inverted) {
		// NOTE: And, of course, RGB565 is terrible. Inverting the lossy packed value would be even lossier...
		if (unlikely(vInfo.bits_per_pixel == 16U)) {
			fgP.rgb565 = pack_rgb565(fgcolor, fgcolor, fgcolor);
			bgP.rgb565 = pack_rgb565(bgcolor, bgcolor, bgcolor);
		} else {
			fgP.bgra.p ^= 0x00FFFFFFu;
			bgP.bgra.p ^= 0x00FFFFFFu;
		}
	}

	// Clamp v offset to safe values
	// NOTE: This test isn't perfect, but then, if you play with this, you do it knowing the risks...
	//       It's mainly there so that stupidly large values don't wrap back on screen because of overflow wraparound.
	short int voffset = fbink_cfg->voffset;
	if ((uint32_t) abs(voffset) >= viewHeight) {
		LOG("The specified vertical offset (%hd) necessarily pushes *all* content out of bounds, discarding it",
		    voffset);
		voffset = 0;
	}

	// And then some from fbink_print...
	short int row = fbink_cfg->row;
	if (fbink_cfg->is_halfway) {
		row = (short int) (row + (short int) (MAXROWS / 2U));
		// NOTE: Clamp to positive values to avoid wrap-around,
		//       (i.e., -MAX should always be 0, and +MAX always the last row (i.e., MAXROWS - 1)).
		//       Our default behavior of starting negative rows from the bottom essentially amounts to valign == bottom,
		//       which is not desirable when we explicitly asked for center ;).
		row = (short int) MIN(MAX(0, row), (MAXROWS - 1));
		LOG("Adjusted row to %hd for vertical centering", row);
	}

	if (row < 0) {
		row = (short int) MAX(MAXROWS + row, 0);
		LOG("Adjusted row to %hd", row);
	}
	while (row >= MAXROWS) {
		row = (short int) (row - MAXROWS);
		LOG("Wrapped row back to %hd", row);
	}

	// We'll begin by painting a blank canvas, just to make sure everything's clean behind us...
	const unsigned short int top_pos =
	    (unsigned short int) MAX(0 + (viewVertOrigin - viewVertOffset), ((row * FONTH) + voffset + viewVertOrigin));
	const unsigned short int left_pos = 0U + viewHoriOrigin;

	// ... unless we were asked to skip background pixels... ;).
	if (!fbink_cfg->is_bgless) {
		(*fxpFillRectChecked)(left_pos, top_pos, (unsigned short int) screenWidth, FONTH, &bgP);
	}

	// NOTE: We always use the same BG_ constant in order to get a rough inverse by just swapping to the inverted LUT ;).
	uint8_t emptyC;
	uint8_t borderC;
	// Handle devices with an inverted palette properly...
#	ifdef FBINK_FOR_KINDLE
	if (deviceQuirks.isKindleLegacy) {
		emptyC  = fbink_cfg->is_inverted ? eInkBGCMap[BG_GRAYB] : eInkFGCMap[BG_GRAYB];
		borderC = fbink_cfg->is_inverted ? eInkBGCMap[BG_GRAY4] : eInkFGCMap[BG_GRAY4];
	} else {
#	endif
		if (fbink_cfg->wfm_mode == WFM_A2) {
			// NOTE: If we're using A2 refresh mode, we'll be enforcing monochrome anyway...
			//       Making sure we do that on our end (... at least with default bg/fg colors anyway ;),
			//       avoids weird behavior on devices where A2 + FORCE_MONOCHROME can otherwise be quirky,
			//       like Kobo Mk. 7
			emptyC  = bgcolor;
			borderC = fgcolor;
		} else {
			emptyC  = fbink_cfg->is_inverted ? eInkFGCMap[BG_GRAYB] : eInkBGCMap[BG_GRAYB];
			borderC = fbink_cfg->is_inverted ? eInkFGCMap[BG_GRAY4] : eInkBGCMap[BG_GRAY4];
		}
#	ifdef FBINK_FOR_KINDLE
	}
#	endif
	// Pack that into the right pixel format...
	FBInkPixel emptyP;
	FBInkPixel borderP;
	switch (vInfo.bits_per_pixel) {
		case 4U:
		case 8U:
			emptyP.gray8  = emptyC;
			borderP.gray8 = borderC;
			break;
		case 16U:
			emptyP.rgb565  = pack_rgb565(emptyC, emptyC, emptyC);
			borderP.rgb565 = pack_rgb565(borderC, borderC, borderC);
			break;
		case 24U:
		case 32U:
		default:
			emptyP.bgra.color.a = 0xFFu;
			emptyP.bgra.color.r = emptyP.bgra.color.g = emptyP.bgra.color.b = emptyC;
			borderP.bgra.color.a                                            = 0xFFu;
			borderP.bgra.color.r = borderP.bgra.color.g = borderP.bgra.color.b = borderC;
			break;
	}

	// Start setting up the screen refresh...
	struct mxcfb_rect region = {
		.top    = top_pos,
		.left   = left_pos,
		.width  = screenWidth,
		.height = FONTH,
	};

	// Which kind of bar did we request?
	if (!is_infinite) {
		// This is a real progress bar ;).

		// We'll want 5% of padding on each side,
		// with rounding to make sure the bar's size is constant across all percentage values...
		const unsigned short int bar_width = (unsigned short int) (((0.90f * (float) viewWidth)) + 0.5f);
		const unsigned short int fill_width =
		    (unsigned short int) (((value / 100.0f) * (0.90f * (float) viewWidth)) + 0.5f);
		const unsigned short int fill_left = (unsigned short int) (left_pos + (0.05f * (float) viewWidth) + 0.5f);
		const unsigned short int empty_width = (unsigned short int) (bar_width - fill_width);
		const unsigned short int empty_left  = (unsigned short int) (fill_left + fill_width);

		// Draw the border...
		(*fxpFillRectChecked)(fill_left, top_pos, bar_width, FONTH, &borderP);
		// Draw the fill bar, which we want to override the border with!
		(*fxpFillRectChecked)(fill_left, top_pos, fill_width, FONTH, &fgP);
		// And the empty bar...
		// NOTE: With a minor tweak to keep a double-width border on the bottom & right sides ;).
		if (value == 0U) {
			// Keep the left border alone!
			(*fxpFillRectChecked)((unsigned short int) (empty_left + 1U),
					      (unsigned short int) (top_pos + 1U),
					      (unsigned short int) MAX(0, empty_width - 3),
					      (unsigned short int) (FONTH - 3U),
					      &emptyP);
		} else {
			(*fxpFillRectChecked)(empty_left,
					      (unsigned short int) (top_pos + 1U),
					      (unsigned short int) MAX(0, empty_width - 2),
					      (unsigned short int) (FONTH - 3U),
					      &emptyP);
		}

		// We enforce centering for the percentage text...
		// NOTE: Zero init, and leave enough space for a *wide* NULL,
		//       to avoid spurious behavior with u8_strlen later on...
		char percentage_text[8] = { 0 };
		snprintf(percentage_text, sizeof(percentage_text), "%hhu%%", value);
		const size_t line_len = strlen(percentage_text);    // Flawfinder: ignore

		bool      halfcell_offset = false;
		short int col             = (short int) ((unsigned short int) (MAXCOLS - line_len) / 2U);

		// NOTE: If the line itself is not a perfect fit, ask draw to start drawing half a cell
		//       to the right to compensate, in order to achieve perfect centering...
		//       This piggybacks a bit on the !isPerfectFit compensation done in draw,
		//       which already does subcell placement ;).
		if (((unsigned short int) col * 2U) + line_len != MAXCOLS) {
			LOG("Line is not a perfect fit, fudging centering by one half of a cell to the right");
			// NOTE: Flag it for correction in draw
			halfcell_offset = true;
		}

		// Draw percentage in the middle of the bar...
		draw(percentage_text, (unsigned short int) row, (unsigned short int) col, 0U, halfcell_offset, fbink_cfg);

		// Don't refresh beyond the borders of the bar if we're backgroundless...
		// This is especially important w/ A2 wfm mode,
		// as FORCE_MONOCHROME *will* quantize the region's existing pixels down to B&W!
		if (fbink_cfg->is_bgless) {
			region.left  = fill_left;
			region.width = bar_width;
		}
	} else {
		// This is an infinite progress bar (a.k.a., activity bar)!

		// We'll want 5% of padding on each side,
		// with rounding to make sure the bar's size is constant across all percentage values...
		const unsigned short int bar_width = (unsigned short int) ((0.90f * (float) viewWidth) + 0.5f);
		const unsigned short int bar_left  = (unsigned short int) (left_pos + (0.05f * (float) viewWidth) + 0.5f);

		// Draw the border...
		(*fxpFillRectChecked)(bar_left, top_pos, (unsigned short int) (bar_width), FONTH, &borderP);
		// Draw the empty bar...
		(*fxpFillRectChecked)((unsigned short int) (bar_left + 1U),
				      (unsigned short int) (top_pos + 1U),
				      (unsigned short int) MAX(0, bar_width - 3),
				      (unsigned short int) (FONTH - 3U),
				      &emptyP);

		// We want our thumb to take 20% of the bar's width
		const unsigned short int thumb_width = (unsigned short int) ((0.20f * bar_width) + 0.5f);
		// We move the thumb in increment of 5% of the bar's width (i.e., half its width),
		// with rounding to avoid accumulating drift...
		const unsigned short int thumb_left =
		    (unsigned short int) (bar_left + ((0.05f * bar_width) * value) + 0.5f);

		// And finally, draw the thumb, which we want to override the border with!
		(*fxpFillRectChecked)(thumb_left, top_pos, thumb_width, FONTH, &fgP);

		// Draw an ellipsis in the middle of the thumb...
		const uint8_t            ellipsis_size = (uint8_t) (FONTH / 3U);
		// Three dots = two spaces, 3 + 2 = 5 ;).
		const unsigned short int ellipsis_left = (unsigned short int) ((thumb_width - (5U * ellipsis_size)) / 2U);
		for (uint8_t i = 0U; i < 3U; i++) {
			(*fxpFillRectChecked)((unsigned short int) (thumb_left + ellipsis_left +
								    (unsigned short int) (i * 2U * ellipsis_size)),
					      (unsigned short int) (top_pos + ellipsis_size),
					      ellipsis_size,
					      ellipsis_size,
					      &bgP);
		}

		// Don't refresh beyond the borders of the bar if we're backgroundless...
		// This is especially important w/ A2 wfm mode,
		// as FORCE_MONOCHROME *will* quantize the region's existing pixels down to B&W!
		if (fbink_cfg->is_bgless) {
			region.left  = bar_left;
			region.width = bar_width;
		}
	}

	// V offset handling is the pits.
	if (voffset != 0 || viewVertOrigin != 0) {
		LOG("Adjusting vertical pen position by %hd pixels, as requested, plus %hhu pixels, as mandated by the native viewport",
		    voffset,
		    viewVertOrigin);
		// Clamp region to sane values if h/v offset is pushing stuff off-screen
		if ((region.top + region.height) > screenHeight) {
			region.height = (uint32_t) MAX(0, (short int) (screenHeight - region.top));
			LOG("Adjusted region height to account for vertical offset pushing part of the content off-screen");
		}
		if (region.top >= screenHeight) {
			region.top = screenHeight - 1;
			LOG("Adjusted region top to account for vertical offset pushing part of the content off-screen");
		}
	}

	// Handle the last rect stuff...
	set_last_rect(&region);

	// Rotate the region if need be...
	(*fxpRotateRegion)(&region);

	// Fudge the region if we asked for a screen clear, so that we actually refresh the full screen...
	if (fbink_cfg->is_cleared) {
		fullscreen_region(&region);
	}

	// And finally, refresh the screen
	// NOTE: FWIW, using A2 basically ends up drawing the border black, and the empty white, which kinda works...
	//       It has the added benefit of increasing the framerate limit after which the eInk controller risks getting
	//       confused (unless is_flashing is enabled, since that'll block,
	//       essentially throttling the bar to the screen's refresh rate).
	if (refresh(fbfd, region, fbink_cfg) != EXIT_SUCCESS) {
		PFWARN("Failed to refresh the screen");
		return ERRCODE(EXIT_FAILURE);
	}

	return EXIT_SUCCESS;
}
#endif    // FBINK_WITH_BITMAP

// Draw a full-width progress bar
int
    fbink_print_progress_bar(int fbfd                                     UNUSED_BY_NOBITMAP,
			     uint8_t percentage                           UNUSED_BY_NOBITMAP,
			     const FBInkConfig* restrict caller_fbink_cfg UNUSED_BY_NOBITMAP)
{
#ifdef FBINK_WITH_BITMAP
	// Open the framebuffer if need be...
	// NOTE: As usual, we *expect* to be initialized at this point!
	bool keep_fd = true;
	if (open_fb_fd(&fbfd, &keep_fd) != EXIT_SUCCESS) {
		return ERRCODE(EXIT_FAILURE);
	}

	// Assume success, until shit happens ;)
	int rv = EXIT_SUCCESS;

	// mmap the fb if need be...
	if (!isFbMapped) {
		if (memmap_fb(fbfd) != EXIT_SUCCESS) {
			rv = ERRCODE(EXIT_FAILURE);
			goto cleanup;
		}
	}

	// Begin by sanitizing the input...
	if (percentage > 100U) {
		LOG("The specified percentage (%hhu) is larger than 100, clamping it.", percentage);
		percentage = 100U;
	}

	// We need a local copy of the config struct, because we have a few things to enforce on our end,
	// and we don't want to mess with the caller's setup (plus, it's const for that very reason anyway).
	FBInkConfig fbink_cfg = *caller_fbink_cfg;
	// Namely, we need overlay mode to properly print the percentage text,
	fbink_cfg.is_overlay  = true;
	// and no hoffset, because it makes no sense for a full-width bar,
	// and we don't want the text to be affected by a stray value...
	fbink_cfg.hoffset     = 0;
	// And we enforce centered text internally, so we'll set col ourselves later...

	// And do the work ;).
	rv = draw_progress_bars(fbfd, false, percentage, &fbink_cfg);

	// Cleanup
cleanup:
	if (isFbMapped && !keep_fd) {
		unmap_fb();
	}
	if (!keep_fd) {
		close_fb(fbfd);
	}

	return rv;
#else
	WARN("Fixed cell font support is disabled in this FBInk build");
	return ERRCODE(ENOSYS);
#endif    // FBINK_WITH_BITMAP
}

// Draw a full-width activity bar
int
    fbink_print_activity_bar(int fbfd                                     UNUSED_BY_NOBITMAP,
			     uint8_t progress                             UNUSED_BY_NOBITMAP,
			     const FBInkConfig* restrict caller_fbink_cfg UNUSED_BY_NOBITMAP)
{
#ifdef FBINK_WITH_BITMAP
	// Open the framebuffer if need be...
	// NOTE: As usual, we *expect* to be initialized at this point!
	bool keep_fd = true;
	if (open_fb_fd(&fbfd, &keep_fd) != EXIT_SUCCESS) {
		return ERRCODE(EXIT_FAILURE);
	}

	// Assume success, until shit happens ;)
	int rv = EXIT_SUCCESS;

	// mmap the fb if need be...
	if (!isFbMapped) {
		if (memmap_fb(fbfd) != EXIT_SUCCESS) {
			rv = ERRCODE(EXIT_FAILURE);
			goto cleanup;
		}
	}

	// We don't need to fudge with fbink_cfg, no text to show ;).

	// Begin by sanitizing the input...
	if (progress > 16U) {
		LOG("The specified progress step (%hhu) is larger than 16, clamping it.", progress);
		progress = 16U;
	}

	// And do the work ;).
	rv = draw_progress_bars(fbfd, true, progress, caller_fbink_cfg);

	// Cleanup
cleanup:
	if (isFbMapped && !keep_fd) {
		unmap_fb();
	}
	if (!keep_fd) {
		close_fb(fbfd);
	}

	return rv;
#else
	WARN("Fixed cell font support is disabled in this FBInk build");
	return ERRCODE(ENOSYS);
#endif    // FBINK_WITH_BITMAP
}

#ifdef FBINK_WITH_IMAGE
// Load & decode image data from a file or stdin, via stbi
static unsigned char*
    img_load_from_file(const char* filename, int* restrict w, int* restrict h, int* restrict n, int req_n)
{
	unsigned char* restrict data = NULL;

	// Read image either from stdin (provided we're not running from a terminal), or a file
	if (strcmp(filename, "-") == 0 && !isatty(fileno(stdin))) {
		// NOTE: Ideally, we'd simply feed stdin to stbi_load_from_file, but that doesn't work because it relies on fseek,
		//       so read stdin ourselves...
		//       c.f., https://stackoverflow.com/a/44894946
		unsigned char* imgdata = NULL;
		unsigned char* temp    = NULL;
		size_t         size    = 0U;
		size_t         used    = 0U;

		if (ferror(stdin)) {
			WARN("Failed to read image data from stdin");
			return NULL;
		}

#	define CHUNK (256 * 1024)
		while (1) {
			if (used + CHUNK + 1U > size) {
				size = used + CHUNK + 1U;

				// Overflow check
				if (size <= used) {
					free(imgdata);
					WARN("Too much input data");
					return NULL;
				}

				// OOM check
				temp = realloc(imgdata, size);
				if (temp == NULL) {
					PFWARN("realloc: %m");
					free(imgdata);
					return NULL;
				}
				imgdata = temp;
				temp    = NULL;
			}

			// cppcheck-suppress nullPointerArithmetic ; imgdata can't be NULL
			size_t nread = fread(imgdata + used, 1U, CHUNK, stdin);
			if (nread == 0U) {
				break;
			}
			used += nread;
		}

		if (ferror(stdin)) {
			free(imgdata);
			WARN("Failed to read image data from stdin");
			return NULL;
		}

		// Shrink & NULL terminate
		// NOTE: We're not buffering C strings, and we're discarding the buffer very very soon, so skip that ;).
		/*
		temp = realloc(imgdata, used + 1U);
		if (temp == NULL) {
			PFWARN("realloc: %m");
			free(imgdata);
			return NULL;
		}
		imgdata       = temp;
		temp          = NULL;
		imgdata[used] = '\0';
		*/

		// Finally, load the image from that buffer, and discard it once we're done.
		data = stbi_load_from_memory(imgdata, (int) used, w, h, n, req_n);
		free(imgdata);
	} else {
		// With a filepath, we can just let stbi handle it ;).
		data = stbi_load(filename, w, h, n, req_n);
	}
	if (data == NULL) {
		WARN("Failed to open or decode image `%s`", filename);
		return NULL;
	}

	LOG("Requested %d color channels, image had %d", req_n, *n);

	return data;
}

// Convert raw image data between various pixel formats
// NOTE: This is a direct copy of stbi's stbi__convert_format, except that it doesn't free the input buffer.
static unsigned char*
    img_convert_px_format(const unsigned char* restrict data, int img_n, int req_comp, int x, int y)
{
	unsigned char* restrict good = NULL;

	// NOTE: We're already doing that in fbink_print_raw_data ;)
	//if (req_comp == img_n) return data;
	STBI_ASSERT(req_comp >= 1 && req_comp <= 4);

	good = (unsigned char* restrict) stbi__malloc_mad3(req_comp, x, y, 0);
	if (good == NULL) {
		//STBI_FREE(data);
		WARN("Failed to allocate pixel format conversion buffer: %m");
		return NULL;
	}

	// NOTE: Using restricted pointers is enough to make vectorizers happy, no need for ivdep pragmas ;).
	for (int j = 0; j < y; ++j) {
		const unsigned char* restrict src = data + (j * x * img_n);
		unsigned char* restrict dest      = good + (j * x * req_comp);

#	define STBI__COMBO(a, b) ((a) * 8 + (b))
#	define STBI__CASE(a, b)                                                                                         \
		case STBI__COMBO(a, b):                                                                                  \
			for (int i = x - 1; i >= 0; --i, src += a, dest += b)
		// convert source image with img_n components to one with req_comp components;
		// avoid switch per pixel, so use switch per scanline and massive macros
		switch (STBI__COMBO(img_n, req_comp)) {
			STBI__CASE(1, 2)
			{
				dest[0] = src[0];
				dest[1] = 255;
			}
			break;
			STBI__CASE(1, 3)
			{
				dest[0] = dest[1] = dest[2] = src[0];
			}
			break;
			STBI__CASE(1, 4)
			{
				dest[0] = dest[1] = dest[2] = src[0];
				dest[3]                     = 255;
			}
			break;
			STBI__CASE(2, 1)
			{
				dest[0] = src[0];
			}
			break;
			STBI__CASE(2, 3)
			{
				dest[0] = dest[1] = dest[2] = src[0];
			}
			break;
			STBI__CASE(2, 4)
			{
				dest[0] = dest[1] = dest[2] = src[0];
				dest[3]                     = src[1];
			}
			break;
			STBI__CASE(3, 4)
			{
				dest[0] = src[0];
				dest[1] = src[1];
				dest[2] = src[2];
				dest[3] = 255;
			}
			break;
			STBI__CASE(3, 1)
			{
				dest[0] = stbi__compute_y(src[0], src[1], src[2]);
			}
			break;
			STBI__CASE(3, 2)
			{
				dest[0] = stbi__compute_y(src[0], src[1], src[2]);
				dest[1] = 255;
			}
			break;
			STBI__CASE(4, 1)
			{
				dest[0] = stbi__compute_y(src[0], src[1], src[2]);
			}
			break;
			STBI__CASE(4, 2)
			{
				dest[0] = stbi__compute_y(src[0], src[1], src[2]);
				dest[1] = src[3];
			}
			break;
			STBI__CASE(4, 3)
			{
				dest[0] = src[0];
				dest[1] = src[1];
				dest[2] = src[2];
			}
			break;
			default:
				STBI_ASSERT(0);
				//STBI_FREE(data);
				STBI_FREE(good);
				WARN("Unsupported pixel format conversion");
				return NULL;
		}
#	undef STBI__CASE
#	undef STBI__COMBO
	}

	//STBI_FREE(data);
	return good;
}

// Quantize an 8-bit color value down to a palette of 16 evenly spaced colors, using an ordered 8x8 dithering pattern.
// With a grayscale input, this happens to match the eInk palette perfectly ;).
// If the input is not grayscale, and the output fb is not grayscale either,
// this usually still happens to match the eInk palette after the EPDC's own quantization pass.
// c.f., https://en.wikipedia.org/wiki/Ordered_dithering
// & https://github.com/ImageMagick/ImageMagick/blob/ecfeac404e75f304004f0566557848c53030bad6/MagickCore/threshold.c#L1627
// NOTE: As the references imply, this is straight from ImageMagick,
//       with only minor simplifications to enforce Q8 & avoid fp maths.
static __attribute__((hot)) uint8_t
    dither_o8x8(unsigned short int x, unsigned short int y, uint8_t v)
{
	// c.f., https://github.com/ImageMagick/ImageMagick/blob/ecfeac404e75f304004f0566557848c53030bad6/config/thresholds.xml#L107
	static const uint8_t threshold_map_o8x8[] = { 1,  49, 13, 61, 4,  52, 16, 64, 33, 17, 45, 29, 36, 20, 48, 32,
						      9,  57, 5,  53, 12, 60, 8,  56, 41, 25, 37, 21, 44, 28, 40, 24,
						      3,  51, 15, 63, 2,  50, 14, 62, 35, 19, 47, 31, 34, 18, 46, 30,
						      11, 59, 7,  55, 10, 58, 6,  54, 43, 27, 39, 23, 42, 26, 38, 22 };

	// Constants:
	// Quantum = 8; Levels = 16; map Divisor = 65
	// QuantumRange = 0xFF
	// QuantumScale = 1.0 / QuantumRange
	//
	// threshold = QuantumScale * v * ((L-1) * (D-1) + 1)
	// NOTE: The initial computation of t (specifically, what we pass to DIV255) would overflow an uint8_t.
	//       With a Q8 input value, we're at no risk of ever underflowing, so, keep to unsigned maths.
	//       Technically, an uint16_t would be wide enough, but it gains us nothing,
	//       and requires a few explicit casts to make GCC happy ;).
	uint32_t       t = DIV255(v * ((15U << 6U) + 1U));
	// level = t / (D-1);
	const uint32_t l = (t >> 6U);
	// t -= l * (D-1);
	t                = (t - (l << 6U));

	// map width & height = 8
	// c = ClampToQuantum((l+(t >= map[(x % mw) + mw * (y % mh)])) * QuantumRange / (L-1));
	const uint32_t q = ((l + (t >= threshold_map_o8x8[(x & 7U) + 8U * (y & 7U)])) * 17U);
	// NOTE: We're doing unsigned maths, so, clamping is basically MIN(q, UINT8_MAX) ;).
	//       The only overflow we should ever catch should be for a few white (v = 0xFF) input pixels
	//       that get shifted to the next step (i.e., q = 272 (0xFF + 17)).
	return (q > UINT8_MAX ? UINT8_MAX : (uint8_t) q);
}

// Draw image data on screen (we inherit a few of the variable types/names from stbi ;))
static int
    draw_image(int fbfd,
	       const unsigned char* restrict data,
	       const int w,
	       const int h,
	       const int n,
	       const int req_n,
	       short int x_off,
	       short int y_off,
	       const FBInkConfig* restrict fbink_cfg)
{
	// Open the framebuffer if need be...
	// NOTE: As usual, we *expect* to be initialized at this point!
	bool keep_fd = true;
	if (open_fb_fd(&fbfd, &keep_fd) != EXIT_SUCCESS) {
		return ERRCODE(EXIT_FAILURE);
	}

	// Assume success, until shit happens ;)
	int rv = EXIT_SUCCESS;

	// mmap the fb if need be...
	if (!isFbMapped) {
		if (memmap_fb(fbfd) != EXIT_SUCCESS) {
			rv = ERRCODE(EXIT_FAILURE);
			goto cleanup;
		}
	}

	// Clear screen?
	if (fbink_cfg->is_cleared) {
		clear_screen(fbfd, fbink_cfg->is_inverted ? penBGColor ^ 0xFFu : penBGColor, fbink_cfg->is_flashing);
	}

	// NOTE: We compute initial offsets from row/col, to help aligning images with text.
	if (fbink_cfg->col < 0) {
		x_off = (short int) (viewHoriOrigin + x_off + (MAX(MAXCOLS + fbink_cfg->col, 0) * FONTW));
	} else {
		x_off = (short int) (viewHoriOrigin + x_off + (fbink_cfg->col * FONTW));
	}
	// NOTE: Unless we *actually* specified a row, ignore viewVertOffset
	//       The rationale being we want to keep being aligned to text rows when we do specify a row,
	//       but we don't want the extra offset when we don't (in particular, when printing full-screen images).
	// NOTE: This means that row 0 and row -MAXROWS *will* behave differently, but so be it...
	if (fbink_cfg->row < 0) {
		y_off = (short int) (viewVertOrigin + y_off + (MAX(MAXROWS + fbink_cfg->row, 0) * FONTH));
	} else if (fbink_cfg->row == 0) {
		y_off = (short int) (viewVertOrigin - viewVertOffset + y_off + (fbink_cfg->row * FONTH));
		// This of course means that row 0 effectively breaks that "align with text" contract if viewVertOffset != 0,
		// on the off-chance we do explicitly really want to align something to row 0, so, warn about it...
		// The "print full-screen images" use-case is greatly more prevalent than "actually rely on row 0 alignment" ;).
		// And in case that's *really* needed, using -MAXROWS instead of 0 will honor alignment anyway.
		if (viewVertOffset != 0U) {
			LOG("Ignoring the %hhupx row offset because row is 0!", viewVertOffset);
		}
	} else {
		y_off = (short int) (viewVertOrigin + y_off + (fbink_cfg->row * FONTH));
	}
	LOG("Adjusted image display coordinates to (%hd, %hd), after column %hd & row %hd",
	    x_off,
	    y_off,
	    fbink_cfg->col,
	    fbink_cfg->row);

	bool       fb_is_grayscale = false;
	bool       fb_is_legacy    = false;
	bool       fb_is_24bpp     = false;
	bool       fb_is_true_bgr  = false;
	bool       img_has_alpha   = false;
	FBInkPixel pixel           = { 0U };
	// Use boolean flags to make the mess of branching slightly more human-readable later...
	switch (vInfo.bits_per_pixel) {
		case 4U:
			fb_is_grayscale = true;
			fb_is_legacy    = true;
			break;
		case 8U:
			fb_is_grayscale = true;
			break;
		case 16U:
			break;
		case 24U:
			fb_is_24bpp    = true;
			fb_is_true_bgr = true;
			break;
		case 32U:
		default:
			fb_is_true_bgr = true;
			break;
	}

	// Handle horizontal alignment...
	switch (fbink_cfg->halign) {
		case CENTER:
			x_off = (short int) (x_off + (int) (viewWidth / 2U));
			x_off = (short int) (x_off - (w / 2));
			break;
		case EDGE:
			x_off = (short int) (x_off + (int) (viewWidth - (uint32_t) w));
			break;
		case NONE:
		default:
			break;
	}
	if (fbink_cfg->halign != NONE) {
		LOG("Adjusted image display coordinates to (%hd, %hd) after horizontal alignment", x_off, y_off);
	}

	// Handle vertical alignment...
	switch (fbink_cfg->valign) {
		case CENTER:
			y_off = (short int) (y_off + (int) (viewHeight / 2U));
			y_off = (short int) (y_off - (h / 2));
			break;
		case EDGE:
			y_off = (short int) (y_off + (int) (viewHeight - (uint32_t) h));
			break;
		case NONE:
		default:
			break;
	}
	if (fbink_cfg->valign != NONE) {
		LOG("Adjusted image display coordinates to (%hd, %hd) after vertical alignment", x_off, y_off);
	}

	// Clamp everything to a safe range, because we can't have *anything* going off-screen here.
	struct mxcfb_rect region;
	// NOTE: Assign each field individually to avoid a false-positive with Clang's SA...
	if (fbink_cfg->row == 0) {
		region.top = MIN(screenHeight, (uint32_t) MAX((viewVertOrigin - viewVertOffset), y_off));
	} else {
		region.top = MIN(screenHeight, (uint32_t) MAX(viewVertOrigin, y_off));
	}
	region.left   = MIN(screenWidth, (uint32_t) MAX(viewHoriOrigin, x_off));
	region.width  = MIN(screenWidth - region.left, (uint32_t) w);
	region.height = MIN(screenHeight - region.top, (uint32_t) h);

	// NOTE: If we ended up with negative display offsets, we should shave those off region.width & region.height,
	//       when it makes sense to do so,
	//       but we need to remember the unshaven value for the pixel loop condition,
	//       to avoid looping on only part of the image.
	unsigned short int max_width  = (unsigned short int) region.width;
	unsigned short int max_height = (unsigned short int) region.height;
	// NOTE: We also need to decide if we start looping at the top left of the image, or if we start later, to
	//       avoid plotting off-screen pixels when using negative display offsets...
	unsigned short int img_x_off  = 0;
	unsigned short int img_y_off  = 0;
	if (x_off < 0) {
		// We'll start plotting from the beginning of the *visible* part of the image ;)
		img_x_off = (unsigned short int) (abs(x_off) + viewHoriOrigin);
		max_width = (unsigned short int) (max_width + img_x_off);
		// Make sure we're not trying to loop past the actual width of the image!
		max_width = (unsigned short int) MIN(w, max_width);
		// Only if the visible section of the image's width is smaller than our screen's width...
		if ((uint32_t) (w - img_x_off) < viewWidth) {
			region.width -= img_x_off;
		}
	}
	if (y_off < 0) {
		// We'll start plotting from the beginning of the *visible* part of the image ;)
		if (fbink_cfg->row == 0) {
			img_y_off = (unsigned short int) (abs(y_off) + viewVertOrigin - viewVertOffset);
		} else {
			img_y_off = (unsigned short int) (abs(y_off) + viewVertOrigin);
		}
		max_height = (unsigned short int) (max_height + img_y_off);
		// Make sure we're not trying to loop past the actual height of the image!
		max_height = (unsigned short int) MIN(h, max_height);
		// Only if the visible section of the image's height is smaller than our screen's height...
		if ((uint32_t) (h - img_y_off) < viewHeight) {
			region.height -= img_y_off;
		}
	}
	LOG("Region: top=%u, left=%u, width=%u, height=%u", region.top, region.left, region.width, region.height);
	LOG("Image becomes visible @ (%hu, %hu), looping 'til (%hu, %hu) out of %dx%d pixels",
	    img_x_off,
	    img_y_off,
	    max_width,
	    max_height,
	    w,
	    h);
	// Warn if there's an alpha channel, because it's usually a bit more expensive to handle...
	if (n == 2 || n == 4) {
		img_has_alpha = true;
		if (fbink_cfg->ignore_alpha) {
			LOG("Ignoring the image's alpha channel.");
		} else {
			LOG("Image has an alpha channel, we'll have to do alpha blending.");
		}
	}

	// Handle inversion if requested, in a way that avoids branching in the loop ;).
	// And, as an added bonus, plays well with the fact that legacy devices have an inverted color map...
	uint8_t  inv     = 0U;
	uint32_t inv_rgb = 0U;
#	ifdef FBINK_FOR_KINDLE
	if ((deviceQuirks.isKindleLegacy && !fbink_cfg->is_inverted) ||
	    (!deviceQuirks.isKindleLegacy && fbink_cfg->is_inverted)) {
#	else
	if (fbink_cfg->is_inverted) {
#	endif
		inv     = 0xFFu;
		inv_rgb = 0x00FFFFFFu;
	}
	// And we'll make 'em constants to eke out a tiny bit of performance...
	const uint8_t  invert     = inv;
	const uint32_t invert_rgb = inv_rgb;
	// NOTE: The *slight* duplication is on purpose, to move the branching outside the loop,
	//       and make use of a few different blitting tweaks depending on the situation...
	//       And since we can easily do so from here,
	//       we also entirely avoid trying to plot off-screen pixels (on any sides).
	if (likely(fb_is_grayscale)) {
		// 4bpp & 8bpp
		if (!fbink_cfg->ignore_alpha && img_has_alpha) {
			if (likely(!fb_is_legacy)) {
				// 8bpp
				// There's an alpha channel in the image, we'll have to do alpha blending...
				// c.f., https://en.wikipedia.org/wiki/Alpha_compositing
				//       https://blogs.msdn.microsoft.com/shawnhar/2009/11/06/premultiplied-alpha/
				FBInkCoordinates coords;
				FBInkPixelG8A    img_px;
				for (unsigned short int j = img_y_off; j < max_height; j++) {
					for (unsigned short int i = img_x_off; i < max_width; i++) {
						// NOTE: In this branch, req_n == 2, so we can do << 1 instead of * 2 ;).
						const size_t img_scanline_offset = (size_t) ((j << 1U) * w);
#	pragma GCC diagnostic push
#	pragma GCC diagnostic ignored "-Wcast-align"
						// First, we gobble the full image pixel (all 2 bytes)
						// cppcheck-suppress unreadVariable ; false-positive (union)
						img_px.p = *((const uint16_t*) (data + img_scanline_offset) + i);
#	pragma GCC diagnostic pop

						// Take a shortcut for the most common alpha values (none & full)
						if (img_px.color.a == 0xFFu) {
							// Fully opaque, we can blit the image (almost) directly.
							// We do need to honor inversion ;).
							// And SW dithering
							if (fbink_cfg->sw_dithering) {
								pixel.gray8 = dither_o8x8(i, j, img_px.color.v ^ invert);
							} else {
								pixel.gray8 = img_px.color.v ^ invert;
							}

							coords.x = (unsigned short int) (i + x_off);
							coords.y = (unsigned short int) (j + y_off);

#	ifdef FBINK_FOR_POCKETBOOK
							(*fxpRotateCoords)(&coords);
#	endif
							put_pixel_Gray8(&coords, &pixel);
						} else if (img_px.color.a == 0) {
							// Transparent! Keep fb as-is.
						} else {
							// Alpha blending...

							// We need to know what this pixel currently looks like in the framebuffer...
							coords.x = (unsigned short int) (i + x_off);
							coords.y = (unsigned short int) (j + y_off);
							// NOTE: We use the the pixel functions directly, to avoid the OOB checks,
							//       because we know we're only processing on-screen pixels,
							//       and we don't care about the rotation checks at this bpp :).
#	ifdef FBINK_FOR_POCKETBOOK
							// ... except on PB, where we *may* require rotation...
							(*fxpRotateCoords)(&coords);
#	endif
							FBInkPixel bg_px;
							get_pixel_Gray8(&coords, &bg_px);

							const uint8_t ainv = img_px.color.a ^ 0xFFu;
							// Don't forget to honor inversion
							img_px.color.v ^= invert;
							// Blend it!
							pixel.gray8 = (uint8_t) DIV255(
							    ((img_px.color.v * img_px.color.a) + (bg_px.gray8 * ainv)));
							// SW dithering
							if (fbink_cfg->sw_dithering) {
								pixel.gray8 = dither_o8x8(i, j, pixel.gray8);
							}

							put_pixel_Gray8(&coords, &pixel);
						}
					}
				}
			} else {
				// 4bpp
				// NOTE: The fact that the fb stores two pixels per byte means we can't take any shortcut,
				//       because they may only apply to one of those two pixels...
				FBInkPixel bg_px = { 0U };
				for (unsigned short int j = img_y_off; j < max_height; j++) {
					for (unsigned short int i = img_x_off; i < max_width; i++) {
						// We need to know what this pixel currently looks like in the framebuffer...
						FBInkCoordinates coords;
						coords.x = (unsigned short int) (i + x_off);
						coords.y = (unsigned short int) (j + y_off);
						// NOTE: We use the the pixel function directly, to avoid the OOB checks,
						//       because we know we're only processing on-screen pixels,
						//       and we don't care about the rotation checks at this bpp :).
						get_pixel_Gray4(&coords, &bg_px);

						// NOTE: In this branch, req_n == 2, so we can do << 1 instead of * 2 ;).
						const size_t  img_scanline_offset = (size_t) ((j << 1U) * w);
						FBInkPixelG8A img_px;
#	pragma GCC diagnostic push
#	pragma GCC diagnostic ignored "-Wcast-align"
						// We gobble the full image pixel (all 2 bytes)
						img_px.p = *((const uint16_t*) (data + img_scanline_offset) + i);
#	pragma GCC diagnostic pop

						const uint8_t ainv = img_px.color.a ^ 0xFFu;
						// Don't forget to honor inversion
						img_px.color.v ^= invert;
						// Blend it!
						pixel.gray8 = (uint8_t) DIV255(
						    ((img_px.color.v * img_px.color.a) + (bg_px.gray8 * ainv)));
						// SW dithering
						if (fbink_cfg->sw_dithering) {
							pixel.gray8 = dither_o8x8(i, j, pixel.gray8);
						}

						put_pixel_Gray4(&coords, &pixel);
					}
				}
			}
		} else {
			// No alpha in image, or ignored
			// We can do a simple copy if the target is 8bpp, the source is 8bpp (no alpha), we don't invert,
			// and we don't dither.
			if (!fb_is_legacy && req_n == 1 && invert == 0U && !fbink_cfg->sw_dithering) {
				// Scanline by scanline, as we usually have input/output x offsets to honor
				for (unsigned short int j = img_y_off; j < max_height; j++) {
					// NOTE: Again, assume the fb origin is @ (0, 0), which should hold true at that bitdepth.
					const size_t pix_offset = (size_t) ((j * w) + img_x_off);
					const size_t fb_offset  = ((uint32_t) (j + y_off) * fInfo.line_length) +
								 (unsigned int) (img_x_off + x_off);
					memcpy(fbPtr + fb_offset, data + pix_offset, max_width);
				}
			} else {
				for (unsigned short int j = img_y_off; j < max_height; j++) {
					for (unsigned short int i = img_x_off; i < max_width; i++) {
						// NOTE: Here, req_n is either 2, or 1 if ignore_alpha, so, no shift trickery ;)
						const size_t pix_offset = (size_t) ((j * req_n * w) + (i * req_n));
						// SW dithering
						if (fbink_cfg->sw_dithering) {
							pixel.gray8 = dither_o8x8(i, j, data[pix_offset] ^ invert);
						} else {
							pixel.gray8 = data[pix_offset] ^ invert;
						}

						FBInkCoordinates coords;
						coords.x = (unsigned short int) (i + x_off);
						coords.y = (unsigned short int) (j + y_off);

						// NOTE: Again, use the pixel functions directly, to skip redundant OOB checks,
						//       as well as unneeded rotation checks (can't happen at this bpp).
						// NOTE: GCC appears to be smart enough to hoist that branch out of the loop ;).
						if (!fb_is_legacy) {
#	ifdef FBINK_FOR_POCKETBOOK
							(*fxpRotateCoords)(&coords);
#	endif
							put_pixel_Gray8(&coords, &pixel);
						} else {
							put_pixel_Gray4(&coords, &pixel);
						}
					}
				}
			}
		}
	} else if (likely(fb_is_true_bgr)) {
		// 24bpp & 32bpp
		if (!fbink_cfg->ignore_alpha && img_has_alpha) {
			FBInkPixelRGBA img_px;
			if (likely(!fb_is_24bpp)) {
				// 32bpp
				FBInkPixelBGRA fb_px;
				// This is essentially a constant in our case... (c.f., put_pixel_RGB32)
				// cppcheck-suppress unreadVariable ; false-positive (union)
				fb_px.color.a = 0xFFu;
				for (unsigned short int j = img_y_off; j < max_height; j++) {
					for (unsigned short int i = img_x_off; i < max_width; i++) {
						// NOTE: We should be able to skip rotation hacks at this bpp...

						// Yeah, I know, GCC...
						// NOTE: In this branch, req_n == 4, so we can do << 2 instead of * 4 ;).
						const size_t img_scanline_offset = (size_t) ((j << 2U) * w);
#	pragma GCC diagnostic push
#	pragma GCC diagnostic ignored "-Wcast-align"
						// First, we gobble the full image pixel (all 4 bytes)
						// cppcheck-suppress unreadVariable ; false-positive (union)
						img_px.p = *((const uint32_t*) (data + img_scanline_offset) + i);
#	pragma GCC diagnostic pop

						// Take a shortcut for the most common alpha values (none & full)
						if (img_px.color.a == 0xFFu) {
							// Fully opaque, we can blit the image (almost) directly.
							// We do need to handle BGR and honor inversion ;).
							// cppcheck-suppress unreadVariable ; false-positive (union)
							img_px.p ^= invert_rgb;
							// And software dithering... Not a fan of the extra branching,
							// but that's probably the best we can do.
							if (fbink_cfg->sw_dithering) {
								fb_px.color.r = dither_o8x8(i, j, img_px.color.r);
								fb_px.color.g = dither_o8x8(i, j, img_px.color.g);
								fb_px.color.b = dither_o8x8(i, j, img_px.color.b);
							} else {
								fb_px.color.r = img_px.color.r;
								fb_px.color.g = img_px.color.g;
								fb_px.color.b = img_px.color.b;
							}

							const size_t fb_scanline_offset =
							    (uint32_t) ((unsigned short int) (j + y_off) *
									fInfo.line_length);
#	pragma GCC diagnostic push
#	pragma GCC diagnostic ignored "-Wcast-align"
							// And we write the full pixel to the fb (all 4 bytes)
							*((uint32_t*) (fbPtr + fb_scanline_offset) + (i + x_off)) =
							    fb_px.p;
#	pragma GCC diagnostic pop
						} else if (img_px.color.a == 0) {
							// Transparent! Keep fb as-is.
						} else {
							// Alpha blending...
							const uint8_t ainv = img_px.color.a ^ 0xFFu;

							const size_t fb_scanline_offset =
							    (uint32_t) ((unsigned short int) (j + y_off) *
									fInfo.line_length);
							FBInkPixelBGRA bg_px;
#	pragma GCC diagnostic push
#	pragma GCC diagnostic ignored "-Wcast-align"
							// Again, read the full pixel from the framebuffer (all 4 bytes)
							bg_px.p =
							    *((uint32_t*) (fbPtr + fb_scanline_offset) + (i + x_off));
#	pragma GCC diagnostic pop

							// Don't forget to honor inversion
							// cppcheck-suppress unreadVariable ; false-positive (union)
							img_px.p ^= invert_rgb;
							// Blend it, we get our BGR swap in the process ;).
							fb_px.color.r = (uint8_t) DIV255(
							    ((img_px.color.r * img_px.color.a) + (bg_px.color.r * ainv)));
							fb_px.color.g = (uint8_t) DIV255(
							    ((img_px.color.g * img_px.color.a) + (bg_px.color.g * ainv)));
							fb_px.color.b = (uint8_t) DIV255(
							    ((img_px.color.b * img_px.color.a) + (bg_px.color.b * ainv)));
							// SW dithering
							if (fbink_cfg->sw_dithering) {
								fb_px.color.r = dither_o8x8(i, j, fb_px.color.r);
								fb_px.color.g = dither_o8x8(i, j, fb_px.color.g);
								fb_px.color.b = dither_o8x8(i, j, fb_px.color.b);
							}

#	pragma GCC diagnostic push
#	pragma GCC diagnostic ignored "-Wcast-align"
							// And we write the full blended pixel to the fb (all 4 bytes)
							*((uint32_t*) (fbPtr + fb_scanline_offset) + (i + x_off)) =
							    fb_px.p;
#	pragma GCC diagnostic pop
						}
					}
				}
			} else {
				// 24bpp
				FBInkPixelBGR fb_px;
				for (unsigned short int j = img_y_off; j < max_height; j++) {
					for (unsigned short int i = img_x_off; i < max_width; i++) {
						// NOTE: We should be able to skip rotation hacks at this bpp...

						// Yeah, I know, GCC...
						// NOTE: In this branch, req_n == 4, so we can do << 2 instead of * 4 ;).
						const size_t img_scanline_offset = (size_t) ((j << 2U) * w);
#	pragma GCC diagnostic push
#	pragma GCC diagnostic ignored "-Wcast-align"
						// First, we gobble the full image pixel (all 4 bytes)
						// cppcheck-suppress unreadVariable ; false-positive (union)
						img_px.p = *((const uint32_t*) (data + img_scanline_offset) + i);
#	pragma GCC diagnostic pop

						// Take a shortcut for the most common alpha values (none & full)
						if (img_px.color.a == 0xFFu) {
							// Fully opaque, we can blit the image (almost) directly.
							// We do need to handle BGR and honor inversion ;).
							// cppcheck-suppress unreadVariable ; false-positive (union)
							img_px.p ^= invert_rgb;
							// SW dithering
							if (fbink_cfg->sw_dithering) {
								fb_px.color.r = dither_o8x8(i, j, img_px.color.r);
								fb_px.color.g = dither_o8x8(i, j, img_px.color.g);
								fb_px.color.b = dither_o8x8(i, j, img_px.color.b);
							} else {
								fb_px.color.r = img_px.color.r;
								fb_px.color.g = img_px.color.g;
								fb_px.color.b = img_px.color.b;
							}

							const size_t fb_pix_offset =
							    (uint32_t) ((unsigned short int) (i + x_off) << 2U) +
							    ((unsigned short int) (j + y_off) * fInfo.line_length);
							// And we write the full pixel to the fb (all 3 bytes)
							*((uint24_t*) (fbPtr + fb_pix_offset)) = fb_px.p;
						} else if (img_px.color.a == 0) {
							// Transparent! Keep fb as-is.
						} else {
							// Alpha blending...
							const uint8_t ainv = img_px.color.a ^ 0xFFu;

							const size_t fb_pix_offset =
							    (uint32_t) ((unsigned short int) (i + x_off) << 2U) +
							    ((unsigned short int) (j + y_off) * fInfo.line_length);
							// Again, read the full pixel from the framebuffer (all 3 bytes)
							FBInkPixelBGR bg_px;
							bg_px.p = *((uint24_t*) (fbPtr + fb_pix_offset));

							// Don't forget to honor inversion
							// cppcheck-suppress unreadVariable ; false-positive (union)
							img_px.p ^= invert_rgb;
							// Blend it, we get our BGR swap in the process ;).
							fb_px.color.r = (uint8_t) DIV255(
							    ((img_px.color.r * img_px.color.a) + (bg_px.color.r * ainv)));
							fb_px.color.g = (uint8_t) DIV255(
							    ((img_px.color.g * img_px.color.a) + (bg_px.color.g * ainv)));
							fb_px.color.b = (uint8_t) DIV255(
							    ((img_px.color.b * img_px.color.a) + (bg_px.color.b * ainv)));
							// SW dithering
							if (fbink_cfg->sw_dithering) {
								fb_px.color.r = dither_o8x8(i, j, fb_px.color.r);
								fb_px.color.g = dither_o8x8(i, j, fb_px.color.g);
								fb_px.color.b = dither_o8x8(i, j, fb_px.color.b);
							}

							// And we write the full blended pixel to the fb (all 3 bytes)
							*((uint24_t*) (fbPtr + fb_pix_offset)) = fb_px.p;
						}
					}
				}
			}
		} else {
			// No alpha in image, or ignored
			// We don't care about image alpha in this branch, so we don't even store it.
			if (likely(!fb_is_24bpp)) {
				// 32bpp
				FBInkPixelBGRA fb_px;
				// This is essentially a constant in our case...
				// cppcheck-suppress unreadVariable ; false-positive (union)
				fb_px.color.a = 0xFFu;
				for (unsigned short int j = img_y_off; j < max_height; j++) {
					for (unsigned short int i = img_x_off; i < max_width; i++) {
						// NOTE: Here, req_n is either 4, or 3 if ignore_alpha, so, no shift trickery ;)
						const size_t  img_pix_offset = (size_t) ((j * req_n * w) + (i * req_n));
						// Gobble the full image pixel (3 bytes, we don't care about alpha if it's there)
						FBInkPixelRGB img_px;
						img_px.p = *((const uint24_t*) &data[img_pix_offset]);
						// NOTE: Given our typedef trickery, this exactly boils down to a 3 bytes memcpy:
						//memcpy(&img_px.p, &data[pix_offset], 3 * sizeof(uint8_t));

						// Handle BGR, inversion & SW dithering
						if (fbink_cfg->sw_dithering) {
							// cppcheck-suppress unreadVariable ; false-positive (union)
							fb_px.color.r = dither_o8x8(i, j, img_px.color.r);
							// cppcheck-suppress unreadVariable ; false-positive (union)
							fb_px.color.g = dither_o8x8(i, j, img_px.color.g);
							// cppcheck-suppress unreadVariable ; false-positive (union)
							fb_px.color.b = dither_o8x8(i, j, img_px.color.b);
						} else {
							// cppcheck-suppress unreadVariable ; false-positive (union)
							fb_px.color.r = img_px.color.r;
							// cppcheck-suppress unreadVariable ; false-positive (union)
							fb_px.color.g = img_px.color.g;
							// cppcheck-suppress unreadVariable ; false-positive (union)
							fb_px.color.b = img_px.color.b;
						}
						// NOTE: The RGB -> BGR dance precludes us from simply doing a 3 bytes memcpy,
						//       and our union trickery appears to be faster than packing the pixel
						//       ourselves with something like:
						//       fb_px.p = 0xFF<<24U | img_px.color.r<<16U | img_px.color.g<<8U | img_px.color.b;
						// We *can* do the inversion on the full pixel, though ;).
						fb_px.p ^= invert_rgb;

						// NOTE: Again, assume we can safely skip rotation tweaks
						const size_t fb_scanline_offset =
						    (uint32_t) ((unsigned short int) (j + y_off) * fInfo.line_length);
#	pragma GCC diagnostic push
#	pragma GCC diagnostic ignored "-Wcast-align"
						// Write the full pixel to the fb (all 4 bytes)
						*((uint32_t*) (fbPtr + fb_scanline_offset) + (i + x_off)) = fb_px.p;
#	pragma GCC diagnostic pop
					}
				}
			} else {
				// 24bpp
				for (unsigned short int j = img_y_off; j < max_height; j++) {
					for (unsigned short int i = img_x_off; i < max_width; i++) {
						// NOTE: Here, req_n is either 4, or 3 if ignore_alpha, so, no shift trickery ;)
						const size_t  img_pix_offset = (size_t) ((j * req_n * w) + (i * req_n));
						// Gobble the full image pixel (3 bytes, we don't care about alpha if it's there)
						FBInkPixelRGB img_px;
						img_px.p = *((const uint24_t*) &data[img_pix_offset]);
						// NOTE: Given our typedef trickery, this exactly boils down to a 3 bytes memcpy:
						//memcpy(&img_px.p, &data[pix_offset], 3 * sizeof(uint8_t));

						// Handle BGR, inversion & SW dithering
						FBInkPixelBGR fb_px;
						if (fbink_cfg->sw_dithering) {
							fb_px.color.r = dither_o8x8(i, j, img_px.color.r ^ invert);
							fb_px.color.g = dither_o8x8(i, j, img_px.color.g ^ invert);
							fb_px.color.b = dither_o8x8(i, j, img_px.color.b ^ invert);
						} else {
							fb_px.color.r = img_px.color.r ^ invert;
							fb_px.color.g = img_px.color.g ^ invert;
							fb_px.color.b = img_px.color.b ^ invert;
						}

						// NOTE: Again, assume we can safely skip rotation tweaks
						const size_t fb_pix_offset =
						    (uint32_t) ((unsigned short int) (i + x_off) << 2U) +
						    ((unsigned short int) (j + y_off) * fInfo.line_length);
						// Write the full pixel to the fb (all 3 bytes)
						*((uint24_t*) (fbPtr + fb_pix_offset)) = fb_px.p;
						// NOTE: Again, this should roughly amount to a 3 bytes memcpy,
						//       although in this instance, GCC generates slightly different code.
						//memcpy(fbPtr + pix_offset, &fb_px.p, 3 * sizeof(uint8_t));
					}
				}
			}
		}
	} else {
		// 16bpp
		if (!fbink_cfg->ignore_alpha && img_has_alpha) {
			FBInkCoordinates coords;
			for (unsigned short int j = img_y_off; j < max_height; j++) {
				for (unsigned short int i = img_x_off; i < max_width; i++) {
					// NOTE: Same general idea as the fb_is_grayscale case,
					//       except at this bpp we then have to handle rotation ourselves...
					// NOTE: In this branch, req_n == 4, so we can do << 2 instead of * 4 ;).
					const size_t   img_scanline_offset = (size_t) ((j << 2U) * w);
					FBInkPixelRGBA img_px;
#	pragma GCC diagnostic push
#	pragma GCC diagnostic ignored "-Wcast-align"
					// Gobble the full image pixel (all 4 bytes)
					img_px.p = *((const uint32_t*) (data + img_scanline_offset) + i);
#	pragma GCC diagnostic pop

					// Take a shortcut for the most common alpha values (none & full)
					if (img_px.color.a == 0xFFu) {
						// Fully opaque, we can blit the image (almost) directly.
						// We do need to handle BGR and honor inversion ;).
						img_px.p ^= invert_rgb;
						// SW dithering
						if (fbink_cfg->sw_dithering) {
							pixel.bgra.color.r = dither_o8x8(i, j, img_px.color.r);
							pixel.bgra.color.g = dither_o8x8(i, j, img_px.color.g);
							pixel.bgra.color.b = dither_o8x8(i, j, img_px.color.b);
						} else {
							pixel.bgra.color.r = img_px.color.r;
							pixel.bgra.color.g = img_px.color.g;
							pixel.bgra.color.b = img_px.color.b;
						}
						// Pack it
						pixel.rgb565 = pack_rgb565(
						    pixel.bgra.color.r, pixel.bgra.color.g, pixel.bgra.color.b);

						coords.x = (unsigned short int) (i + x_off);
						coords.y = (unsigned short int) (j + y_off);
						(*fxpRotateCoords)(&coords);
						put_pixel_RGB565(&coords, &pixel);
					} else if (img_px.color.a == 0) {
						// Transparent! Keep fb as-is.
					} else {
						// Alpha blending...
						const uint8_t ainv = img_px.color.a ^ 0xFFu;

						coords.x = (unsigned short int) (i + x_off);
						coords.y = (unsigned short int) (j + y_off);
						(*fxpRotateCoords)(&coords);
						FBInkPixel bg_px;
						get_pixel_RGB565(&coords, &bg_px);

						// Don't forget to honor inversion
						img_px.p ^= invert_rgb;
						// Blend it, we get our BGR swap in the process ;).
						pixel.bgra.color.r = (uint8_t) DIV255(
						    ((img_px.color.r * img_px.color.a) + (bg_px.bgra.color.r * ainv)));
						pixel.bgra.color.g = (uint8_t) DIV255(
						    ((img_px.color.g * img_px.color.a) + (bg_px.bgra.color.g * ainv)));
						pixel.bgra.color.b = (uint8_t) DIV255(
						    ((img_px.color.b * img_px.color.a) + (bg_px.bgra.color.b * ainv)));
						// SW dithering
						if (fbink_cfg->sw_dithering) {
							pixel.bgra.color.r = dither_o8x8(i, j, pixel.bgra.color.r);
							pixel.bgra.color.g = dither_o8x8(i, j, pixel.bgra.color.g);
							pixel.bgra.color.b = dither_o8x8(i, j, pixel.bgra.color.b);
						}
						// Pack it
						pixel.rgb565 = pack_rgb565(
						    pixel.bgra.color.r, pixel.bgra.color.g, pixel.bgra.color.b);

						put_pixel_RGB565(&coords, &pixel);
					}
				}
			}
		} else {
			// No alpha in image, or ignored
			// NOTE: For some reason, reading the image 3 or 4 bytes at once doesn't win us anything, here...
			for (unsigned short int j = img_y_off; j < max_height; j++) {
				for (unsigned short int i = img_x_off; i < max_width; i++) {
					// NOTE: Here, req_n is either 4, or 3 if ignore_alpha, so, no shift trickery ;)
					const size_t pix_offset = (size_t) ((j * req_n * w) + (i * req_n));
					// SW dithering
					if (fbink_cfg->sw_dithering) {
						pixel.bgra.color.r = dither_o8x8(i, j, data[pix_offset + 0U] ^ invert);
						pixel.bgra.color.g = dither_o8x8(i, j, data[pix_offset + 1U] ^ invert);
						pixel.bgra.color.b = dither_o8x8(i, j, data[pix_offset + 2U] ^ invert);
					} else {
						pixel.bgra.color.r = data[pix_offset + 0U] ^ invert;
						pixel.bgra.color.g = data[pix_offset + 1U] ^ invert;
						pixel.bgra.color.b = data[pix_offset + 2U] ^ invert;
					}
					// Pack it
					pixel.rgb565 =
					    pack_rgb565(pixel.bgra.color.r, pixel.bgra.color.g, pixel.bgra.color.b);

					FBInkCoordinates coords;
					coords.x = (unsigned short int) (i + x_off);
					coords.y = (unsigned short int) (j + y_off);
					// NOTE: Again, we can only skip the OOB checks at this bpp.
					(*fxpRotateCoords)(&coords);
					put_pixel_RGB565(&coords, &pixel);
				}
			}
		}
	}

	// Handle the last rect stuff...
	set_last_rect(&region);

	// Rotate the region if need be...
	(*fxpRotateRegion)(&region);

	// Fudge the region if we asked for a screen clear, so that we actually refresh the full screen...
	if (fbink_cfg->is_cleared) {
		fullscreen_region(&region);
	}

	// Refresh screen
	if (refresh(fbfd, region, fbink_cfg) != EXIT_SUCCESS) {
		PFWARN("Failed to refresh the screen");
	}

	// Cleanup
cleanup:
	if (isFbMapped && !keep_fd) {
		unmap_fb();
	}
	if (!keep_fd) {
		close_fb(fbfd);
	}

	return rv;
}
#endif    // FBINK_WITH_IMAGE

// Draw an image on screen
int
    fbink_print_image(int fbfd                              UNUSED_BY_MINIMAL,
		      const char* filename                  UNUSED_BY_MINIMAL,
		      short int x_off                       UNUSED_BY_MINIMAL,
		      short int y_off                       UNUSED_BY_MINIMAL,
		      const FBInkConfig* restrict fbink_cfg UNUSED_BY_MINIMAL)
{
#ifdef FBINK_WITH_IMAGE
	// Assume success, until shit happens ;)
	int rv = EXIT_SUCCESS;

	// Let stbi handle grayscaling for us
	// NOTE: It does so via an approximation of the Rec601Luma formula, in IM-speak
	//       (c.f., https://www.imagemagick.org/script/command-line-options.php#intensity)
	//       (c.f., stbi__compute_y @ stb/stb_image.h)
	int req_n;
	switch (vInfo.bits_per_pixel) {
		case 4U:
			req_n = 1 + !fbink_cfg->ignore_alpha;
			break;
		case 8U:
			req_n = 1 + !fbink_cfg->ignore_alpha;
			break;
		case 16U:
			req_n = 3 + !fbink_cfg->ignore_alpha;
			break;
		case 24U:
			req_n = 3 + !fbink_cfg->ignore_alpha;
			break;
		case 32U:
		default:
			req_n = 3 + !fbink_cfg->ignore_alpha;
			break;
	}

	// Was scaling requested?
	bool want_scaling = false;
	if (fbink_cfg->scaled_width != 0 || fbink_cfg->scaled_height != 0) {
		LOG("Image scaling requested!");
		want_scaling = true;

		// NOTE: QImageScale only accepts Y8, Y8A and RGBA input (i.e., RGBA, or RGB stored @ 32bpp, with 8 unused bits).
		//       We, on the other hand, store RGB in 24bits, so, that won't do...
		//       TL;DR: When outputting RGB, request RGBA from stbi instead to ensure a 32bpp buffer,
		//              stbi will set the alpha bytes to 0xFF if the input doesn't have any alpha,
		//              but that's not relevant, as we'll ask QImageScale to ignore alpha *processing* w/ ignore_alpha
		// NOTE: That said, if input *has* an alpha channel, QImageScale expects premultiplied alpha,
		//       while stbi leaves it untouched, meaning straight alpha in the vast majority of cases...
		if (req_n == 3) {
			LOG("Enforcing 32bpp buffer for scaling!");
			req_n = 4;
		}
	}

	// Decode image via stbi
	unsigned char* restrict data = NULL;
	int w;
	int h;
	int n;
	data = img_load_from_file(filename, &w, &h, &n, req_n);
	if (data == NULL) {
		WARN("Failed to decode image data from `%s`", filename);
		return ERRCODE(EXIT_FAILURE);
	}

	unsigned char* restrict sdata = NULL;
	// Scale it w/ QImageScale, if requested
	if (want_scaling) {
		// Make sure the scaled dimensions start sane...
		unsigned short int scaled_width;
		if (fbink_cfg->scaled_width > 0) {
			// Honor the specified dimension
			scaled_width = (unsigned short int) fbink_cfg->scaled_width;
		} else if (fbink_cfg->scaled_width < 0) {
			// -1 or less -> use the viewport's dimension
			scaled_width = (unsigned short int) viewWidth;
		} else {
			// 0 -> No scaling requested
			scaled_width = (unsigned short int) w;
		}
		unsigned short int scaled_height;
		if (fbink_cfg->scaled_height > 0) {
			// Honor the specified dimension
			scaled_height = (unsigned short int) fbink_cfg->scaled_height;
		} else if (fbink_cfg->scaled_height < 0) {
			// -1 or less -> use the viewport's dimension
			scaled_height = (unsigned short int) viewHeight;
		} else {
			// 0 -> No scaling requested
			scaled_height = (unsigned short int) h;
		}

		// NOTE: Handle AR if best fit was requested, or if scaling was requested on one side only...
		if (fbink_cfg->scaled_width < -1 || fbink_cfg->scaled_height < -1) {
			float aspect                      = (float) w / (float) h;
			// We want to fit the image *inside* the viewport, so, enforce our starting scaled dimensions...
			scaled_width                      = (unsigned short int) viewWidth;
			scaled_height                     = (unsigned short int) viewHeight;
			// NOTE: Loosely based on Qt's QSize boundedTo implementation
			//       c.f., QSize::scaled @ https://github.com/qt/qtbase/blob/dev/src/corelib/tools/qsize.cpp
			unsigned short int rescaled_width = (unsigned short int) (scaled_height * aspect + 0.5f);
			// NOTE: One would simply have to check for >= instead of <= to implement
			//       Qt::KeepAspectRatioByExpanding instead of Qt::KeepAspectRatio
			if (rescaled_width <= scaled_width) {
				scaled_width = rescaled_width;
			} else {
				scaled_height = (unsigned short int) (scaled_width / aspect + 0.5f);
			}
		} else if (fbink_cfg->scaled_width == 0 && fbink_cfg->scaled_height != 0) {
			// ?xH, compute width, honoring AR
			float aspect = (float) w / (float) h;
			scaled_width = (unsigned short int) (scaled_height * aspect + 0.5f);
		} else if (fbink_cfg->scaled_width != 0 && fbink_cfg->scaled_height == 0) {
			// Wx?, compute height, honoring AR
			float aspect  = (float) w / (float) h;
			scaled_height = (unsigned short int) (scaled_width / aspect + 0.5f);
		}

		LOG("Scaling image from %dx%d to %hux%hu . . .", w, h, scaled_width, scaled_height);

		sdata = qSmoothScaleImage(data, w, h, req_n, fbink_cfg->ignore_alpha, scaled_width, scaled_height);
		if (sdata == NULL) {
			PFWARN("Failed to resize image");
			return ERRCODE(EXIT_FAILURE);
		}

		// We're drawing the scaled data, at the requested scaled resolution
		if (draw_image(fbfd, sdata, scaled_width, scaled_height, n, req_n, x_off, y_off, fbink_cfg) !=
		    EXIT_SUCCESS) {
			PFWARN("Failed to display image data on screen");
			rv = ERRCODE(EXIT_FAILURE);
			goto cleanup;
		}
	} else {
		// We're drawing the original unscaled data at its native resolution
		if (draw_image(fbfd, data, w, h, n, req_n, x_off, y_off, fbink_cfg) != EXIT_SUCCESS) {
			PFWARN("Failed to display image data on screen");
			rv = ERRCODE(EXIT_FAILURE);
			goto cleanup;
		}
	}

	// Cleanup
cleanup:
	// Free the buffer holding our decoded image data
	stbi_image_free(data);
	free(sdata);

	return rv;
#else
	WARN("Image support is disabled in this FBInk build");
	return ERRCODE(ENOSYS);
#endif    // FBINK_WITH_IMAGE
}

// Draw raw (supposedly image) data on screen
int
    fbink_print_raw_data(int fbfd                              UNUSED_BY_MINIMAL,
			 unsigned char* restrict data          UNUSED_BY_MINIMAL,
			 const int w                           UNUSED_BY_MINIMAL,
			 const int h                           UNUSED_BY_MINIMAL,
			 const size_t len                      UNUSED_BY_MINIMAL,
			 short int x_off                       UNUSED_BY_MINIMAL,
			 short int y_off                       UNUSED_BY_MINIMAL,
			 const FBInkConfig* restrict fbink_cfg UNUSED_BY_MINIMAL)
{
#ifdef FBINK_WITH_IMAGE
	// Assume success, until shit happens ;)
	int rv = EXIT_SUCCESS;

	// Since draw_image doesn't really handle every possible case,
	// we'll have to fiddle with an intermediary buffer ourselves to make it happy,
	// while still accepting various different kinds of inputs so the user doesn't have to worry about any of this...
	int req_n;
	switch (vInfo.bits_per_pixel) {
		case 4U:
			req_n = 1 + !fbink_cfg->ignore_alpha;
			break;
		case 8U:
			req_n = 1 + !fbink_cfg->ignore_alpha;
			break;
		case 16U:
			req_n = 3 + !fbink_cfg->ignore_alpha;
			break;
		case 24U:
			req_n = 3 + !fbink_cfg->ignore_alpha;
			break;
		case 32U:
		default:
			req_n = 3 + !fbink_cfg->ignore_alpha;
			break;
	}

	// Devising the actual amount of components in the input should be as easy as that...
	int n = (int) len / h / w;

	LOG("Requested %d color channels, supplied data had %d", req_n, n);

	// Was scaling requested?
	unsigned char* restrict sdata = NULL;
	bool want_scaling             = false;
	if (fbink_cfg->scaled_width != 0 || fbink_cfg->scaled_height != 0) {
		LOG("Image scaling requested!");
		want_scaling = true;

		// NOTE: QImageScale only accepts Y8, Y8A and RGBA input (i.e., RGBA, or RGB stored @ 32bpp, with 8 unused bits).
		//       We, on the other hand, store RGB in 24bits, so, that won't do...
		//       TL;DR: When outputting RGB, request RGBA from stbi instead to ensure a 32bpp buffer,
		//              stbi will set the alpha bytes to 0xFF if the input doesn't have any alpha,
		//              but that's not relevant, as we'll ask QImageScale to ignore alpha *processing* w/ ignore_alpha
		// NOTE: That said, if input *has* an alpha channel, QImageScale expects premultiplied alpha,
		//       while stbi leaves it untouched, meaning straight alpha in the vast majority of cases...
		if (req_n == 3) {
			LOG("Enforcing 32bpp buffer for scaling!");
			req_n = 4;
		}
	}

	// If there's a mismatch between the components in the input data vs. what the fb expects,
	// re-interleave the data w/ stbi's help...
	unsigned char* restrict imgdata = NULL;
	if (req_n != n) {
		LOG("Converting from %d components to the requested %d", n, req_n);
		// NOTE: stbi__convert_format will *always* free the input buffer, which we do NOT want here...
		//       Which is why we're using a tweaked internal copy, which does not free ;).
		imgdata = img_convert_px_format(data, n, req_n, w, h);
		if (imgdata == NULL) {
			WARN("Failed to re-interleave input data in a suitable format");
			rv = ERRCODE(EXIT_FAILURE);
			goto cleanup;
		}
	} else {
		// We can use the input buffer as-is :)
		LOG("No conversion needed, using the input buffer directly");
		imgdata = data;
	}

	// Scale it w/ QImageScale, if requested
	if (want_scaling) {
		// Make sure the scaled dimensions start sane...
		unsigned short int scaled_width;
		if (fbink_cfg->scaled_width > 0) {
			// Honor the specified dimension
			scaled_width = (unsigned short int) fbink_cfg->scaled_width;
		} else if (fbink_cfg->scaled_width < 0) {
			// -1 or less -> use the viewport's dimension
			scaled_width = (unsigned short int) viewWidth;
		} else {
			// 0 -> No scaling requested
			scaled_width = (unsigned short int) w;
		}
		unsigned short int scaled_height;
		if (fbink_cfg->scaled_height > 0) {
			// Honor the specified dimension
			scaled_height = (unsigned short int) fbink_cfg->scaled_height;
		} else if (fbink_cfg->scaled_height < 0) {
			// -1 or less -> use the viewport's dimension
			scaled_height = (unsigned short int) viewHeight;
		} else {
			// 0 -> No scaling requested
			scaled_height = (unsigned short int) h;
		}

		// NOTE: Handle AR if best fit was requested, or if scaling was requested on one side only...
		if (fbink_cfg->scaled_width < -1 || fbink_cfg->scaled_height < -1) {
			float aspect                      = (float) w / (float) h;
			// We want to fit the image *inside* the viewport, so, enforce our starting scaled dimensions...
			scaled_width                      = (unsigned short int) viewWidth;
			scaled_height                     = (unsigned short int) viewHeight;
			// NOTE: Loosely based on Qt's QSize boundedTo implementation
			//       c.f., QSize::scaled @ https://github.com/qt/qtbase/blob/dev/src/corelib/tools/qsize.cpp
			unsigned short int rescaled_width = (unsigned short int) (scaled_height * aspect + 0.5f);
			// NOTE: One would simply have to check for >= instead of <= to implement
			//       Qt::KeepAspectRatioByExpanding instead of Qt::KeepAspectRatio
			if (rescaled_width <= scaled_width) {
				scaled_width = rescaled_width;
			} else {
				scaled_height = (unsigned short int) (scaled_width / aspect + 0.5f);
			}
		} else if (fbink_cfg->scaled_width == 0 && fbink_cfg->scaled_height != 0) {
			// ?xH, compute width, honoring AR
			float aspect = (float) w / (float) h;
			scaled_width = (unsigned short int) (scaled_height * aspect + 0.5f);
		} else if (fbink_cfg->scaled_width != 0 && fbink_cfg->scaled_height == 0) {
			// Wx?, compute height, honoring AR
			float aspect  = (float) w / (float) h;
			scaled_height = (unsigned short int) (scaled_width / aspect + 0.5f);
		}

		LOG("Scaling image data from %dx%d to %hux%hu . . .", w, h, scaled_width, scaled_height);

		sdata = qSmoothScaleImage(imgdata, w, h, req_n, fbink_cfg->ignore_alpha, scaled_width, scaled_height);
		if (sdata == NULL) {
			PFWARN("Failed to resize image");
			return ERRCODE(EXIT_FAILURE);
		}

		// We're drawing the scaled data, at the requested scaled resolution
		if (draw_image(fbfd, sdata, scaled_width, scaled_height, n, req_n, x_off, y_off, fbink_cfg) !=
		    EXIT_SUCCESS) {
			PFWARN("Failed to display image data on screen");
			rv = ERRCODE(EXIT_FAILURE);
			goto cleanup;
		}
	} else {
		// We should now be able to draw that on screen, knowing that it probably won't horribly implode ;p
		if (draw_image(fbfd, imgdata, w, h, n, req_n, x_off, y_off, fbink_cfg) != EXIT_SUCCESS) {
			PFWARN("Failed to display image data on screen");
			rv = ERRCODE(EXIT_FAILURE);
			goto cleanup;
		}
	}

	// Cleanup
cleanup:
	// If we created an intermediary buffer ourselves, free it.
	if (req_n != n) {
		stbi_image_free(imgdata);
	}
	// And the scaled buffer
	free(sdata);

	return rv;
#else
	WARN("Image support is disabled in this FBInk build");
	return ERRCODE(ENOSYS);
#endif    // FBINK_WITH_IMAGE
}

// Dump the full fb (first visible screen)
int
    fbink_dump(int fbfd UNUSED_BY_MINIMAL, FBInkDump* restrict dump UNUSED_BY_MINIMAL)
{
#ifdef FBINK_WITH_IMAGE
	// Open the framebuffer if need be...
	// NOTE: As usual, we *expect* to be initialized at this point!
	bool keep_fd = true;
	if (open_fb_fd(&fbfd, &keep_fd) != EXIT_SUCCESS) {
		return ERRCODE(EXIT_FAILURE);
	}

	// Assume success, until shit happens ;)
	int rv = EXIT_SUCCESS;

	// mmap the fb if need be...
	if (!isFbMapped) {
		if (memmap_fb(fbfd) != EXIT_SUCCESS) {
			rv = ERRCODE(EXIT_FAILURE);
			goto cleanup;
		}
	}

	// Dump the *full* fb
	// Free current data in case the dump struct is being reused
	if (dump->data) {
		LOG("Recycling FBinkDump!");
		free(dump->data);
		dump->data = NULL;
		// Reset the crop settings
		dump->clip = (const FBInkRect){ 0U };
	}
	// Start by allocating enough memory for a full dump of the visible screen...
	dump->stride = fInfo.line_length;
	dump->size   = (size_t) (dump->stride * vInfo.yres);
	dump->data   = calloc(dump->size, sizeof(*dump->data));
	if (dump->data == NULL) {
		PFWARN("dump->data %zu bytes calloc: %m", dump->size);
		dump->stride = 0U;
		dump->size   = 0U;
		rv           = ERRCODE(EXIT_FAILURE);
		goto cleanup;
	}
	// Store the current fb state for that dump
	dump->area.left   = 0U;
	dump->area.top    = 0U;
	dump->area.width  = (unsigned short int) vInfo.xres_virtual;
	dump->area.height = (unsigned short int) vInfo.yres;
	dump->rota        = (uint8_t) vInfo.rotate;
	dump->bpp         = (uint8_t) vInfo.bits_per_pixel;
	dump->is_full     = true;
	// And finally, the fb data itself
	memcpy(dump->data, fbPtr, dump->size);

	// Cleanup
cleanup:
	if (isFbMapped && !keep_fd) {
		unmap_fb();
	}
	if (!keep_fd) {
		close_fb(fbfd);
	}

	return rv;
#else
	WARN("Image support is disabled in this FBInk build");
	return ERRCODE(ENOSYS);
#endif    // FBINK_WITH_IMAGE
}

#ifdef FBINK_WITH_IMAGE
// Helper for fbink_region_dump & fbink_rect_dump
static int
    dump_region(struct mxcfb_rect* region, FBInkDump* restrict dump)
{
	// Bail if we were fed an empty region
	if (region->width == 0U || region->height == 0U) {
		WARN("Tried to dump an empty region");
		return ERRCODE(EINVAL);
	}

	// Free current data in case the dump struct is being reused
	if (dump->data) {
		LOG("Recycling FBinkDump!");
		free(dump->data);
		memset(dump, 0, sizeof(*dump));
	}
	// Start by allocating enough memory for a full dump of the computed region...
	// We're going to need the amount of bytes taken per pixel...
	const uint8_t bpp = (uint8_t) (vInfo.bits_per_pixel >> 3U);
	// And then to handle 4bpp on its own, because 4/8 == 0 ;).
	if (vInfo.bits_per_pixel == 4U) {
		// Align to the nearest byte boundary to make our life easier...
		if (region->left & 0x01u) {
			// x is odd, round *down* to the nearest multiple of two (i.e., align to the start of the current byte)
			region->left &= ~0x01u;
			LOG("Updated region.left to %u because of alignment constraints", region->left);
		}
		if (region->width & 0x01u) {
			// w is odd, round *up* to the nearest multiple of two (i.e., align to the end of the current byte)
			region->width = (region->width + 1U) & ~0x01u;
			LOG("Updated region.width to %u because of alignment constraints", region->width);
		}
		// Two pixels per byte, and we've just ensured to never end up with a decimal when dividing by two ;).
		dump->stride = (size_t) (region->width >> 1U);
		dump->size   = (size_t) (dump->stride * region->height);
		dump->data   = calloc(dump->size, sizeof(*dump->data));
	} else {
		dump->stride = (size_t) (region->width * bpp);
		dump->size   = (size_t) (dump->stride * region->height);
		dump->data   = calloc(dump->size, sizeof(*dump->data));
	}
	if (dump->data == NULL) {
		PFWARN("dump->data %zu bytes calloc: %m", dump->size);
		dump->stride = 0U;
		dump->size   = 0U;
		return ERRCODE(EXIT_FAILURE);
	}
	// Store the current fb state for that dump
	dump->area.left   = (unsigned short int) region->left;
	dump->area.top    = (unsigned short int) region->top;
	dump->area.width  = (unsigned short int) region->width;
	dump->area.height = (unsigned short int) region->height;
	dump->rota        = (uint8_t) vInfo.rotate;
	dump->bpp         = (uint8_t) vInfo.bits_per_pixel;
	dump->is_full     = false;
	// And finally, the fb data itself, scanline per scanline
	if (dump->bpp == 4U) {
		for (unsigned short int j = dump->area.top, l = 0U; l < dump->area.height; j++, l++) {
			size_t dump_offset = (size_t) (l * dump->stride);
			size_t fb_offset   = (size_t) (dump->area.left >> 1U) + (j * fInfo.line_length);
			memcpy(dump->data + dump_offset, fbPtr + fb_offset, dump->stride);
		}
	} else {
		for (unsigned short int j = dump->area.top, l = 0U; l < dump->area.height; j++, l++) {
			size_t dump_offset = (size_t) (l * dump->stride);
			size_t fb_offset   = (size_t) (dump->area.left * bpp) + (j * fInfo.line_length);
			memcpy(dump->data + dump_offset, fbPtr + fb_offset, dump->stride);
		}
	}

	return EXIT_SUCCESS;
}
#endif    // FBINK_WITH_IMAGE

// Dump a specific region of the fb
int
    fbink_region_dump(int fbfd                              UNUSED_BY_MINIMAL,
		      short int x_off                       UNUSED_BY_MINIMAL,
		      short int y_off                       UNUSED_BY_MINIMAL,
		      unsigned short int w                  UNUSED_BY_MINIMAL,
		      unsigned short int h                  UNUSED_BY_MINIMAL,
		      const FBInkConfig* restrict fbink_cfg UNUSED_BY_MINIMAL,
		      FBInkDump* restrict dump              UNUSED_BY_MINIMAL)
{
#ifdef FBINK_WITH_IMAGE
	// Open the framebuffer if need be...
	// NOTE: As usual, we *expect* to be initialized at this point!
	bool keep_fd = true;
	if (open_fb_fd(&fbfd, &keep_fd) != EXIT_SUCCESS) {
		return ERRCODE(EXIT_FAILURE);
	}

	// Assume success, until shit happens ;)
	int rv = EXIT_SUCCESS;

	// mmap the fb if need be...
	if (!isFbMapped) {
		if (memmap_fb(fbfd) != EXIT_SUCCESS) {
			rv = ERRCODE(EXIT_FAILURE);
			goto cleanup;
		}
	}

	// Dump a region of the fb
	// Pilfer the coordinates computations from draw_image ;).
	// NOTE: We compute initial offsets from row/col, to help aligning the region with text.
	if (fbink_cfg->col < 0) {
		x_off = (short int) (viewHoriOrigin + x_off + (MAX(MAXCOLS + fbink_cfg->col, 0) * FONTW));
	} else {
		x_off = (short int) (viewHoriOrigin + x_off + (fbink_cfg->col * FONTW));
	}
	// NOTE: Unless we *actually* specified a row, ignore viewVertOffset
	//       The rationale being we want to keep being aligned to text rows when we do specify a row,
	//       but we don't want the extra offset when we don't (in particular, when dumping what amounts to the full screen).
	// NOTE: This means that row 0 and row -MAXROWS *will* behave differently, but so be it...
	if (fbink_cfg->row < 0) {
		y_off = (short int) (viewVertOrigin + y_off + (MAX(MAXROWS + fbink_cfg->row, 0) * FONTH));
	} else if (fbink_cfg->row == 0) {
		y_off = (short int) (viewVertOrigin - viewVertOffset + y_off + (fbink_cfg->row * FONTH));
		// This of course means that row 0 effectively breaks that "align with text" contract if viewVertOffset != 0,
		// on the off-chance we do explicitly really want to align something to row 0, so, warn about it...
		// The "print full-screen data" use-case is greatly more prevalent than "actually rely on row 0 alignment" ;).
		// And in case that's *really* needed, using -MAXROWS instead of 0 will honor alignment anyway.
		if (viewVertOffset != 0U) {
			LOG("Ignoring the %hhupx row offset because row is 0!", viewVertOffset);
		}
	} else {
		y_off = (short int) (viewVertOrigin + y_off + (fbink_cfg->row * FONTH));
	}
	LOG("Adjusted dump coordinates to (%hd, %hd), after column %hd & row %hd",
	    x_off,
	    y_off,
	    fbink_cfg->col,
	    fbink_cfg->row);

	// Handle horizontal alignment...
	switch (fbink_cfg->halign) {
		case CENTER:
			x_off = (short int) (x_off + (int) (viewWidth / 2U));
			x_off = (short int) (x_off - (w / 2));
			break;
		case EDGE:
			x_off = (short int) (x_off + (int) (viewWidth - (uint32_t) w));
			break;
		case NONE:
		default:
			break;
	}
	if (fbink_cfg->halign != NONE) {
		LOG("Adjusted dump coordinates to (%hd, %hd) after horizontal alignment", x_off, y_off);
	}

	// Handle vertical alignment...
	switch (fbink_cfg->valign) {
		case CENTER:
			y_off = (short int) (y_off + (int) (viewHeight / 2U));
			y_off = (short int) (y_off - (h / 2));
			break;
		case EDGE:
			y_off = (short int) (y_off + (int) (viewHeight - (uint32_t) h));
			break;
		case NONE:
		default:
			break;
	}
	if (fbink_cfg->valign != NONE) {
		LOG("Adjusted dump coordinates to (%hd, %hd) after vertical alignment", x_off, y_off);
	}

	// Clamp everything to a safe range, because we can't have *anything* going off-screen here.
	struct mxcfb_rect region;
	// NOTE: Assign each field individually to avoid a false-positive with Clang's SA...
	if (fbink_cfg->row == 0) {
		region.top = MIN(screenHeight, (uint32_t) MAX((viewVertOrigin - viewVertOffset), y_off));
	} else {
		region.top = MIN(screenHeight, (uint32_t) MAX(viewVertOrigin, y_off));
	}
	region.left   = MIN(screenWidth, (uint32_t) MAX(viewHoriOrigin, x_off));
	region.width  = MIN(screenWidth - region.left, (uint32_t) w);
	region.height = MIN(screenHeight - region.top, (uint32_t) h);

	// NOTE: If we ended up with negative display offsets, we should shave those off region.width & region.height,
	//       when it makes sense to do so.
	unsigned short int img_x_off = 0;
	unsigned short int img_y_off = 0;
	if (x_off < 0) {
		// We'll start dumping from the beginning of the *visible* part of the region ;)
		img_x_off = (unsigned short int) (abs(x_off) + viewHoriOrigin);
		// Only if the visible section of the region's width is smaller than our screen's width...
		if ((uint32_t) (w - img_x_off) < viewWidth) {
			region.width -= img_x_off;
		}
	}
	if (y_off < 0) {
		// We'll start dumping from the beginning of the *visible* part of the region ;)
		if (fbink_cfg->row == 0) {
			img_y_off = (unsigned short int) (abs(y_off) + viewVertOrigin - viewVertOffset);
		} else {
			img_y_off = (unsigned short int) (abs(y_off) + viewVertOrigin);
		}
		// Only if the visible section of the region's height is smaller than our screen's height...
		if ((uint32_t) (h - img_y_off) < viewHeight) {
			region.height -= img_y_off;
		}
	}
	LOG("Region: top=%u, left=%u, width=%u, height=%u", region.top, region.left, region.width, region.height);
	LOG("Dump becomes visible @ (%hu, %hu) for %ux%u out of the requested %dx%d pixels",
	    img_x_off,
	    img_y_off,
	    region.width,
	    region.height,
	    w,
	    h);

	// Rotate the region if need be...
	(*fxpRotateRegion)(&region);

	rv = dump_region(&region, dump);

	// Cleanup
cleanup:
	if (isFbMapped && !keep_fd) {
		unmap_fb();
	}
	if (!keep_fd) {
		close_fb(fbfd);
	}

	return rv;
#else
	WARN("Image support is disabled in this FBInk build");
	return ERRCODE(ENOSYS);
#endif    // FBINK_WITH_IMAGE
}

// Dump a specific rect of the fb (as-is)
int
    fbink_rect_dump(int fbfd                       UNUSED_BY_MINIMAL,
		    const FBInkRect* restrict rect UNUSED_BY_MINIMAL,
		    FBInkDump* restrict dump       UNUSED_BY_MINIMAL)
{
#ifdef FBINK_WITH_IMAGE
	// If no rect, or an empty rect was passed, fallback to a *full* dump.
	if (!rect || (rect->width == 0U || rect->height == 0U)) {
		WARN("Requested to dump an empty rect (or none at all), falling back to a full dump");
		return fbink_dump(fbfd, dump);
	}

	// Open the framebuffer if need be...
	// NOTE: As usual, we *expect* to be initialized at this point!
	bool keep_fd = true;
	if (open_fb_fd(&fbfd, &keep_fd) != EXIT_SUCCESS) {
		return ERRCODE(EXIT_FAILURE);
	}

	// Assume success, until shit happens ;)
	int rv = EXIT_SUCCESS;

	// mmap the fb if need be...
	if (!isFbMapped) {
		if (memmap_fb(fbfd) != EXIT_SUCCESS) {
			rv = ERRCODE(EXIT_FAILURE);
			goto cleanup;
		}
	}

	// Sanity check...
	if ((rect->top + rect->height > vInfo.yres) || (rect->left + rect->width > vInfo.xres)) {
		WARN("Requested to dump an OOB rect");
		rv = ERRCODE(EINVAL);
		goto cleanup;
	}

	// Plain rect -> region mapping, we don't want any further rotation trickery here.
	struct mxcfb_rect region = { .top = rect->top, .left = rect->left, .width = rect->width, .height = rect->height };

	rv = dump_region(&region, dump);

	// Cleanup
cleanup:
	if (isFbMapped && !keep_fd) {
		unmap_fb();
	}
	if (!keep_fd) {
		close_fb(fbfd);
	}

	return rv;
#else
	WARN("Image support is disabled in this FBInk build");
	return ERRCODE(ENOSYS);
#endif    // FBINK_WITH_IMAGE
}

// Restore a fb dump
int
    fbink_restore(int fbfd                              UNUSED_BY_MINIMAL,
		  const FBInkConfig* restrict fbink_cfg UNUSED_BY_MINIMAL,
		  const FBInkDump* restrict dump        UNUSED_BY_MINIMAL)
{
#ifdef FBINK_WITH_IMAGE
	// Open the framebuffer if need be...
	// NOTE: As usual, we *expect* to be initialized at this point!
	bool keep_fd = true;
	if (open_fb_fd(&fbfd, &keep_fd) != EXIT_SUCCESS) {
		return ERRCODE(EXIT_FAILURE);
	}

	// Assume success, until shit happens ;)
	int rv = EXIT_SUCCESS;

	// mmap the fb if need be...
	if (!isFbMapped) {
		if (memmap_fb(fbfd) != EXIT_SUCCESS) {
			rv = ERRCODE(EXIT_FAILURE);
			goto cleanup;
		}
	}

	// Restore (if possible)
	if (!dump->data) {
		WARN("No dump data to restore");
		rv = ERRCODE(EINVAL);
		goto cleanup;
	}
	if (dump->rota != vInfo.rotate) {
		WARN("Can't restore the dump because of a rotation mismatch! dump: %hhu (%s) vs. fb: %u (%s)",
		     dump->rota,
		     fb_rotate_to_string(dump->rota),
		     vInfo.rotate,
		     fb_rotate_to_string(vInfo.rotate));
		rv = ERRCODE(ENOTSUP);
		goto cleanup;
	}
	if (dump->bpp != vInfo.bits_per_pixel) {
		WARN("Can't restore the dump because of a bitdepth mismatch! dump: %hhu vs. fb: %u",
		     dump->bpp,
		     vInfo.bits_per_pixel);
		rv = ERRCODE(ENOTSUP);
		goto cleanup;
	}
	if (dump->area.width > vInfo.xres_virtual) {
		WARN("Can't restore the dump because it's wider than the screen! dump: %hu vs. fb: %u",
		     dump->area.width,
		     vInfo.xres_virtual);
		rv = ERRCODE(ENOTSUP);
		goto cleanup;
	}
	if (dump->area.height > vInfo.yres) {
		WARN("Can't restore the dump because it's taller than the screen! dump: %hu vs. fb: %u",
		     dump->area.height,
		     vInfo.yres);
		rv = ERRCODE(ENOTSUP);
		goto cleanup;
	}
	if (dump->size > fInfo.smem_len) {
		WARN("Can't restore the dump because it's larger than the framebuffer! dump: %zu vs. fb: %u",
		     dump->size,
		     fInfo.smem_len);
		rv = ERRCODE(ENOTSUP);
		goto cleanup;
	}
	// Cropping related sanity checks...
	if (dump->clip.width != 0U || dump->clip.height != 0U) {
		if (dump->is_full) {
			WARN("Can't crop a full-screen dump");
			rv = ERRCODE(ENOTSUP);
			goto cleanup;
		}
		if (dump->clip.width == 0U) {
			WARN("Cropped width can't be zero");
			rv = ERRCODE(ENOTSUP);
			goto cleanup;
		}
		if (dump->clip.height == 0U) {
			WARN("Cropped height can't be zero");
			rv = ERRCODE(ENOTSUP);
			goto cleanup;
		}
		if (dump->clip.width > vInfo.xres_virtual) {
			WARN("Clip rectangle is wider than the screen! clip: %hu vs. fb: %u",
			     dump->clip.width,
			     vInfo.xres_virtual);
			rv = ERRCODE(ENOTSUP);
			goto cleanup;
		}
		if (dump->clip.height > vInfo.yres) {
			WARN("Clip rectangle is taller than the screen! clip: %hu vs. fb: %u",
			     dump->clip.height,
			     vInfo.yres);
			rv = ERRCODE(ENOTSUP);
			goto cleanup;
		}
		if (dump->clip.left >= vInfo.xres) {
			WARN("Clip rectangle's left edge is OOB! clip: %hu vs. fb: %u", dump->clip.left, vInfo.xres);
			rv = ERRCODE(ENOTSUP);
			goto cleanup;
		}
		if (dump->clip.top >= vInfo.yres) {
			WARN("Clip rectangle's top edge is OOB! clip: %hu vs. fb: %u", dump->clip.top, vInfo.yres);
			rv = ERRCODE(ENOTSUP);
			goto cleanup;
		}
		// Overlap check (c.f., https://stackoverflow.com/q/306316)
		if (!(dump->area.left < dump->clip.left + dump->clip.width)) {
			WARN("Clip rectangle is outside the dumped area (on the left)");
			rv = ERRCODE(ENOTSUP);
			goto cleanup;
		}
		if (!(dump->area.left + dump->area.width > dump->clip.left)) {
			WARN("Clip rectangle is outside the dumped area (on the right)");
			rv = ERRCODE(ENOTSUP);
			goto cleanup;
		}
		if (!(dump->area.top < dump->clip.top + dump->clip.height)) {
			WARN("Clip rectangle is outside the dumped area (at the top)");
			rv = ERRCODE(ENOTSUP);
			goto cleanup;
		}
		if (!(dump->area.top + dump->area.height > dump->clip.top)) {
			WARN("Clip rectangle is outside the dumped area (at the bottom)");
			rv = ERRCODE(ENOTSUP);
			goto cleanup;
		}
	}

	// We'll need a region...
	struct mxcfb_rect region;

	if (dump->is_full) {
		// Full dump, easy enough
		memcpy(fbPtr, dump->data, dump->size);
		fullscreen_region(&region);
	} else {
		// NOTE: The crop codepath is perfectly safe with no cropping, it's just a little bit hairier to follow...
		if (dump->clip.width == 0U && dump->clip.height == 0U) {
			// Region dump, restore line by line
			if (dump->bpp == 4U) {
				for (unsigned short int j = dump->area.top, l = 0U; l < dump->area.height; j++, l++) {
					size_t fb_offset   = (size_t) (dump->area.left >> 1U) + (j * fInfo.line_length);
					size_t dump_offset = (size_t) (l * dump->stride);
					memcpy(
					    fbPtr + fb_offset, dump->data + dump_offset, (size_t) dump->area.width >> 1U);
				}
			} else {
				// We're going to need the amount of bytes taken per pixel...
				const uint8_t bpp = dump->bpp >> 3U;
				for (unsigned short int j = dump->area.top, l = 0U; l < dump->area.height; j++, l++) {
					size_t fb_offset   = (size_t) (dump->area.left * bpp) + (j * fInfo.line_length);
					size_t dump_offset = (size_t) (l * dump->stride);
					memcpy(
					    fbPtr + fb_offset, dump->data + dump_offset, (size_t) dump->area.width * bpp);
				}
			}
			region.left   = dump->area.left;
			region.top    = dump->area.top;
			region.width  = dump->area.width;
			region.height = dump->area.height;
		} else {
			// Handle cropping shenanigans...
			const unsigned short int x_skip = dump->clip.left > dump->area.left
							      ? (unsigned short int) (dump->clip.left - dump->area.left)
							      : 0U;
			const unsigned short int x      = (unsigned short int) (dump->area.left + x_skip);
			const unsigned short int y_skip =
			    dump->clip.top > dump->area.top ? (unsigned short int) (dump->clip.top - dump->area.top) : 0U;
			const unsigned short int y  = (unsigned short int) (dump->area.top + y_skip);
			// NOTE: We only want to display the intersection between the full dump area and the clip rectangle...
			//       The earlier overlap check should ensure the sanity of the resulting rectangle here.
			//       c.f., https://stackoverflow.com/q/19753134
			const unsigned short int x1 = (unsigned short int) MAX(dump->area.left, dump->clip.left);
			const unsigned short int y1 = (unsigned short int) MAX(dump->area.top, dump->clip.top);
			const unsigned short int x2 = (unsigned short int) MIN(dump->area.left + dump->area.width,
									       dump->clip.left + dump->clip.width);
			const unsigned short int y2 = (unsigned short int) MIN(dump->area.top + dump->area.height,
									       dump->clip.top + dump->clip.height);
			const unsigned short int w  = (unsigned short int) (x2 - x1);
			const unsigned short int h  = (unsigned short int) (y2 - y1);
			LOG("The dump area (%hu, %hu) %hux%hu",
			    dump->area.left,
			    dump->area.top,
			    dump->area.width,
			    dump->area.height);
			LOG("and clip rectangle (%hu, %hu) %hux%hu",
			    dump->clip.left,
			    dump->clip.top,
			    dump->clip.width,
			    dump->clip.height);
			LOG("intersect as (%hu, %hu, %hu, %hu) %hux%hu", x1, y1, x2, y2, w, h);
			// Region dump, restore line by line
			if (dump->bpp == 4U) {
				for (unsigned short int j = y, l = 0U; l < h; j++, l++) {
					size_t fb_offset   = (size_t) (x >> 1U) + (j * fInfo.line_length);
					size_t dump_offset = (x_skip >> 1U) + ((size_t) (y_skip + l) * dump->stride);
					memcpy(fbPtr + fb_offset, dump->data + dump_offset, (size_t) w >> 1U);
				}
			} else {
				// We're going to need the amount of bytes taken per pixel...
				const uint8_t bpp = dump->bpp >> 3U;
				for (unsigned short int j = y, l = 0U; l < h; j++, l++) {
					size_t fb_offset = (size_t) (x * bpp) + (j * fInfo.line_length);
					size_t dump_offset =
					    (size_t) (x_skip * bpp) + ((size_t) (y_skip + l) * dump->stride);
					memcpy(fbPtr + fb_offset, dump->data + dump_offset, (size_t) w * bpp);
				}
			}
			region.left   = x;
			region.top    = y;
			region.width  = w;
			region.height = h;
		}
	}

	// And now, we can refresh the screen
	if (refresh(fbfd, region, fbink_cfg) != EXIT_SUCCESS) {
		PFWARN("Failed to refresh the screen");
	}

	// Cleanup
cleanup:
	if (isFbMapped && !keep_fd) {
		unmap_fb();
	}
	if (!keep_fd) {
		close_fb(fbfd);
	}

	return rv;
#else
	WARN("Image support is disabled in this FBInk build");
	return ERRCODE(ENOSYS);
#endif    // FBINK_WITH_IMAGE
}

// Explicitly frees the FBInkDump *data* allocated by fbink_dump & fbink_region_dump
int
    fbink_free_dump_data(FBInkDump* restrict dump UNUSED_BY_MINIMAL)
{
#ifdef FBINK_WITH_IMAGE
	if (dump->data) {
		free(dump->data);
		// Clear the struct, both to clear the metadata, and to make sure not to leave a dangling pointer,
		// ensuring a subsequent dump() won't try to recycle this struct again.
		memset(dump, 0, sizeof(*dump));

		return EXIT_SUCCESS;
	} else {
		return ERRCODE(EINVAL);
	}
#else
	WARN("Image support is disabled in this FBInk build");
	return ERRCODE(ENOSYS);
#endif    // FBINK_WITH_IMAGE
}

// Return a copy of the last drawn rectangle coordinates/dimensions
FBInkRect
    fbink_get_last_rect(bool rotated)
{
	if (rotated) {
		// I'm really glad I went with a different layout in FBInkRect there... >_<".
		struct mxcfb_rect region = {
			.top = lastRect.top, .left = lastRect.left, .width = lastRect.width, .height = lastRect.height
		};
		(*fxpRotateRegion)(&region);
		FBInkRect rect = { .left   = (unsigned short int) region.left,
				   .top    = (unsigned short int) region.top,
				   .width  = (unsigned short int) region.width,
				   .height = (unsigned short int) region.height };
		return rect;
	}
	return lastRect;
}

// Grants direct access to the backing buffer's pointer & size
unsigned char*
    fbink_get_fb_pointer(int fbfd, size_t* buffer_size)
{
	if (!deviceQuirks.skipId) {
		PFWARN("FBInk hasn't been initialized yet");
		goto failure;
	}

	if (fbfd == FBFD_AUTO) {
		PFWARN("Requires a persistent backing buffer");
		goto failure;
	}

	if (!isFbMapped) {
		if (memmap_fb(fbfd) != EXIT_SUCCESS) {
			goto failure;
		}
	}

#ifdef FBINK_FOR_KOBO
	*buffer_size = deviceQuirks.isSunxi ? sunxiCtx.alloc_size : fInfo.smem_len;
#else
	*buffer_size = fInfo.smem_len;
#endif
	return fbPtr;

failure:
	*buffer_size = 0U;
	return NULL;
}

// Returns a *full* copy of our internal framebuffer structs
void
    fbink_get_fb_info(struct fb_var_screeninfo* var_info, struct fb_fix_screeninfo* fix_info)
{
	*var_info = vInfo;
	*fix_info = fInfo;
}

// And now, we just bundle auxiliary parts of the public or private API,
// that are implemented in separate source files because they deal with a specific concept,
// but that still rely heavily on either the public or the private API.
// We want them in a single compilation unit because it makes dealing with all-static functions slightly less cumbersome.
//
// Deals with device identification
#ifndef FBINK_FOR_LINUX
#	include "fbink_device_id.c"
#endif
// Extra fonts
#ifdef FBINK_WITH_FONTS
// Viznut's Unscii (http://pelulamu.net/unscii)
#	include "fbink_unscii.c"
// PoP's Block font, c.f., https://www.mobileread.com/forums/showpost.php?p=3736203&postcount=26 and earlier ;).
#	include "fbink_block.c"
// Wiktor Kerr's Leggie (https://memleek.org/leggie)
#	include "fbink_leggie.c"
// Micah Elliott's Orp (https://github.com/MicahElliott/Orp-Font)
#	include "fbink_orp.c"
// Nerdy Pepper's Scientifica (https://github.com/NerdyPepper/scientifica)
#	include "fbink_scientifica.c"
// Dimitar Toshkov Zhekov's Terminus (http://terminus-font.sourceforge.net)
#	include "fbink_terminus.c"
// Tomi Ollila's Fatty (https://github.com/domo141/fatty-bitmap-font)
#	include "fbink_fatty.c"
// Frederic Cambus's Spleen (https://github.com/fcambus/spleen)
#	include "fbink_spleen.c"
// Lucy Luz's Tewi (https://github.com/lucy/tewi-font)
#	include "fbink_tewi.c"
// Various other small fonts (c.f., CREDITS for details)
#	include "fbink_misc_fonts.c"
// Amiga fonts (https://www.trueschool.se/html/fonts.html)
#	include "fbink_topaz.c"
#	include "fbink_microknight.c"
// VGA variant of the IBM font (https://farsil.github.io/ibmfonts & https://int10h.org/oldschool-pc-fonts)
#	include "fbink_vga.c"
// GNU Unifont glyphs (http://unifoundry.com/unifont/index.html)
#	ifdef FBINK_WITH_UNIFONT
#		include "fbink_unifont.c"
#	endif
// Slavfox's Cozette (https://github.com/slavfox/Cozette)
#	include "fbink_cozette.c"
#endif
// Contains fbink_button_scan's implementation, Kobo only, and has a bit of Linux MT input thrown in ;).
#include "fbink_button_scan.c"
// Contains the Kobo only native/canonical rotation conversion helpers
#include "fbink_rota_quirks.c"
