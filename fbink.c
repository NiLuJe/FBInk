/*
	FBInk: FrameBuffer eInker, a tool to print text & images on eInk devices (Kobo/Kindle)
	Copyright (C) 2018-2020 NiLuJe <ninuje@gmail.com>
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
// Disable a bunch of very verbose but mostly harmless warnings
#	pragma GCC diagnostic   push
#	pragma GCC diagnostic   ignored "-Wunknown-pragmas"
#	pragma clang diagnostic ignored "-Wunknown-warning-option"
#	pragma GCC diagnostic   ignored "-Wcast-qual"
#	pragma GCC diagnostic   ignored "-Wcast-align"
#	pragma GCC diagnostic   ignored "-Wconversion"
#	pragma GCC diagnostic   ignored "-Wsign-conversion"
#	pragma GCC diagnostic   ignored "-Wduplicated-branches"
#	pragma GCC diagnostic   ignored "-Wunused-parameter"
#	pragma GCC diagnostic   ignored "-Wunused-function"
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

// #RGB -> RGB565
static inline uint16_t
    pack_rgb565(uint8_t r, uint8_t g, uint8_t b)
{
	// ((r / 8) * 2048) + ((g / 4) * 32) + (b / 8);
	return (uint16_t)(((r >> 3U) << 11U) | ((g >> 2U) << 5U) | (b >> 3U));
}

// Helper functions to 'plot' a specific pixel in a given color to the framebuffer
static void
    put_pixel_Gray4(const FBInkCoordinates* restrict coords, const FBInkPixel* restrict px)
{
	// calculate the pixel's byte offset inside the buffer
	// note: x / 2 as every byte holds 2 pixels
	size_t pix_offset = (coords->x >> 1U) + (coords->y * fInfo.line_length);

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

static void
    put_pixel_Gray8(const FBInkCoordinates* restrict coords, const FBInkPixel* restrict px)
{
	// calculate the pixel's byte offset inside the buffer
	size_t pix_offset = coords->x + (coords->y * fInfo.line_length);

	// now this is about the same as 'fbp[pix_offset] = value'
	*((unsigned char*) (fbPtr + pix_offset)) = px->gray8;
}

static void
    put_pixel_RGB24(const FBInkCoordinates* restrict coords, const FBInkPixel* restrict px)
{
	// calculate the pixel's byte offset inside the buffer
	// note: x * 3 as every pixel is 3 consecutive bytes
	size_t pix_offset = (coords->x * 3U) + (coords->y * fInfo.line_length);

	// now this is about the same as 'fbp[pix_offset] = value'
	*((unsigned char*) (fbPtr + pix_offset))      = px->bgra.color.b;
	*((unsigned char*) (fbPtr + pix_offset + 1U)) = px->bgra.color.g;
	*((unsigned char*) (fbPtr + pix_offset + 2U)) = px->bgra.color.r;
}

static void
    put_pixel_RGB32(const FBInkCoordinates* restrict coords, const FBInkPixel* restrict px)
{
	// calculate the pixel's byte offset inside the buffer
	// note: x * 4 as every pixel is 4 consecutive bytes
	size_t pix_offset = (uint32_t)(coords->x << 2U) + (coords->y * fInfo.line_length);

	// write the four bytes at once
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wcast-align"
	*((uint32_t*) (fbPtr + pix_offset)) = px->bgra.p;
#pragma GCC diagnostic pop
}

static void
    put_pixel_RGB565(const FBInkCoordinates* restrict coords, const FBInkPixel* restrict px)
{
	// calculate the pixel's byte offset inside the buffer
	// note: x * 2 as every pixel is 2 consecutive bytes
	size_t pix_offset = (uint32_t)(coords->x << 1U) + (coords->y * fInfo.line_length);

	// write the two bytes at once, much to GCC's dismay...
	// NOTE: Input pixel *has* to be properly packed to RGB565 first (via pack_rgb565, c.f., put_pixel)!
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wcast-align"
	*((uint16_t*) (fbPtr + pix_offset)) = px->rgb565;
#pragma GCC diagnostic pop
}

#if defined(FBINK_FOR_KOBO) || defined(FBINK_FOR_CERVANTES)
// Handle rotation quirks...
static void
    rotate_coordinates_pickel(FBInkCoordinates* restrict coords)
{
	// Rotate the coordinates to account for pickel's rotation...
	unsigned short int rx = coords->y;
	unsigned short int ry = (unsigned short int) (screenWidth - coords->x - 1);

// NOTE: This codepath is not production ready, it was just an experiment to wrap my head around framebuffer rotation...
//       In particular, only CW has been actually confirmed to behave properly (to handle the isNTX16bLandscape quirk),
//       and region rotation is NOT handled properly/at all.
//       TL;DR: This is for documentation purposes only, never build w/ MATHS defined ;).
#	ifdef FBINK_WITH_MATHS_ROTA
	uint8_t rotation = FB_ROTATE_CW;
	// i.e., θ (c.f., https://en.wikipedia.org/wiki/Cartesian_coordinate_system#Rotation)
	double rangle = ((rotation * 90) * M_PI / 180.0);
	double fxp    = coords->x * cos(rangle) - coords->y * sin(rangle);
	double fyp    = coords->x * sin(rangle) + coords->y * cos(rangle);
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

static void
    rotate_coordinates_boot(FBInkCoordinates* restrict coords)
{
	// Rotate the coordinates to account for the native boot rotation...
	// NOTE: See the note is fbink_init, this is based on a replicated boot modeset,
	//       which apparently doesn't exactly match the *real* boot modeset... -_-".
	unsigned short int rx = (unsigned short int) (screenHeight - coords->y - 1);
	unsigned short int ry = coords->x;

	coords->x = rx;
	coords->y = ry;
}

#	ifdef FBINK_WITH_BUTTON_SCAN
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
static void
    put_pixel(FBInkCoordinates coords, const FBInkPixel* restrict px, bool rgb565_packed)
{
	// Handle rotation now, so we can properly validate if the pixel is off-screen or not ;).
	// fbink_init() takes care of setting this global pointer to the right function...
	// NOTE: In this case, going through the function pointer is *noticeably* faster than branching...
	(*fxpRotateCoords)(&coords);

	// NOTE: Discard off-screen pixels!
	//       For instance, when we have a halfcell offset in conjunction with a !isPerfectFit pixel offset,
	//       when we're padding and centering, the final whitespace of right-padding will have its last
	//       few pixels (the exact amount being half of the dead zone width) pushed off-screen...
	if (coords.x >= vInfo.xres || coords.y >= vInfo.yres) {
#ifdef DEBUG
		// NOTE: This is only enabled in Debug builds because it can be pretty verbose,
		//       and does not necessarily indicate an actual issue, as we've just explained...
		LOG("Put: discarding off-screen pixel @ (%hu, %hu) (out of %ux%u bounds)",
		    coords.x,
		    coords.y,
		    vInfo.xres,
		    vInfo.yres);
#endif
		return;
	}

	// NOTE: Hmm, here, an if ladder appears to be ever so *slightly* faster than going through the function pointer...
	if (vInfo.bits_per_pixel == 4U) {
		put_pixel_Gray4(&coords, px);
	} else if (vInfo.bits_per_pixel == 8U) {
		put_pixel_Gray8(&coords, px);
	} else if (vInfo.bits_per_pixel == 16U) {
		// Do we need to pack the pixel, first?
		if (rgb565_packed) {
			// Nope :)
			put_pixel_RGB565(&coords, px);
		} else {
			// Yep :(
			FBInkPixel packed_px;
			packed_px.rgb565 = pack_rgb565(px->bgra.color.r, px->bgra.color.g, px->bgra.color.b);
			put_pixel_RGB565(&coords, &packed_px);
		}
	} else if (vInfo.bits_per_pixel == 24U) {
		put_pixel_RGB24(&coords, px);
	} else if (vInfo.bits_per_pixel == 32U) {
		put_pixel_RGB32(&coords, px);
	}
}

// Helper functions to 'get' a specific pixel's color from the framebuffer
// c.f., FBGrab convert* functions
//       (http://trac.ak-team.com/trac/browser/niluje/Configs/trunk/Kindle/Misc/FBGrab/fbgrab.c#L402)
// as well as KOReader's routines
//       (https://github.com/koreader/koreader-base/blob/b3e72affd0e1ba819d92194b229468452c58836f/ffi/blitbuffer.lua#L292)
static void
    get_pixel_Gray4(const FBInkCoordinates* restrict coords, FBInkPixel* restrict px)
{
	// calculate the pixel's byte offset inside the buffer
	// note: x / 2 as every byte holds 2 pixels
	size_t pix_offset = (coords->x >> 1U) + (coords->y * fInfo.line_length);

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
		px->gray8 = (uint8_t)((b & 0x0Fu) * 0x11u);
		// or: pull the low/right nibble, expanded to 8bit
	}
	// NOTE: c.f., FBInkPixel typedef in fbink_types.h for details on the union shenanigans...
	//       In short: gray8 -> gray4.hi -> bgra.color.b
	//                          gray4.lo -> bgra.color.g
}

static void
    get_pixel_Gray8(const FBInkCoordinates* restrict coords, FBInkPixel* restrict px)
{
	// calculate the pixel's byte offset inside the buffer
	size_t pix_offset = coords->x + (coords->y * fInfo.line_length);

	px->gray8 = *((unsigned char*) (fbPtr + pix_offset));
}

static void
    get_pixel_RGB24(const FBInkCoordinates* restrict coords, FBInkPixel* restrict px)
{
	// calculate the pixel's byte offset inside the buffer
	// note: x * 3 as every pixel is 3 consecutive bytes
	size_t pix_offset = (coords->x * 3U) + (coords->y * fInfo.line_length);

	px->bgra.color.b = *((unsigned char*) (fbPtr + pix_offset));
	px->bgra.color.g = *((unsigned char*) (fbPtr + pix_offset + 1U));
	px->bgra.color.r = *((unsigned char*) (fbPtr + pix_offset + 2U));
}

static void
    get_pixel_RGB32(const FBInkCoordinates* restrict coords, FBInkPixel* restrict px)
{
	// calculate the pixel's byte offset inside the buffer
	// note: x * 4 as every pixel is 4 consecutive bytes, which we read in one go
	size_t pix_offset = (uint32_t)(coords->x << 2U) + (coords->y * fInfo.line_length);

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wcast-align"
	px->bgra.p = *((uint32_t*) (fbPtr + pix_offset));
#pragma GCC diagnostic pop
	// NOTE: We generally don't care about alpha, we always assume it's opaque, as that's how it behaves.
	//       We *do* pickup the actual alpha value, here, though.
}

static void
    get_pixel_RGB565(const FBInkCoordinates* restrict coords, FBInkPixel* restrict px)
{
	// calculate the pixel's byte offset inside the buffer
	// note: x * 2 as every pixel is 2 consecutive bytes
	size_t pix_offset = (uint32_t)(coords->x << 1U) + (coords->y * fInfo.line_length);

	// NOTE: We're honoring the fb's bitfield offsets here (B: 0, G: >> 5, R: >> 11)
	// Like put_pixel_RGB565, read those two consecutive bytes at once
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wcast-align"
	const uint16_t v = *((const uint16_t*) (fbPtr + pix_offset));
#pragma GCC diagnostic pop

	// NOTE: Unpack to RGB32, because we have no use for RGB565, it's terrible.
	// NOTE: c.f., https://stackoverflow.com/q/2442576
	//       I feel that this approach tracks better with what we do in put_pixel_RGB565,
	//       and I have an easier time following it than the previous approach ported from KOReader.
	//       Both do exactly the same thing, though ;).
	const uint8_t r = (uint8_t)((v & 0xF800u) >> 11U);
	const uint8_t g = (v & 0x07E0u) >> 5U;
	const uint8_t b = (v & 0x001Fu);

	px->bgra.color.r = (uint8_t)((r << 3U) | (r >> 2U));
	px->bgra.color.g = (uint8_t)((g << 2U) | (g >> 4U));
	px->bgra.color.b = (uint8_t)((b << 3U) | (b >> 2U));
}

// Handle a few sanity checks...
static void
    get_pixel(FBInkCoordinates coords, FBInkPixel* restrict px)
{
	// Handle rotation now, so we can properly validate if the pixel is off-screen or not ;).
	// fbink_init() takes care of setting this global pointer to the right function...
	(*fxpRotateCoords)(&coords);

	// NOTE: Discard off-screen pixels!
	//       For instance, when we have a halfcell offset in conjunction with a !isPerfectFit pixel offset,
	//       when we're padding and centering, the final whitespace of right-padding will have its last
	//       few pixels (the exact amount being half of the dead zone width) pushed off-screen...
	if (coords.x >= vInfo.xres || coords.y >= vInfo.yres) {
#ifdef DEBUG
		// NOTE: This is only enabled in Debug builds because it can be pretty verbose,
		//       and does not necessarily indicate an actual issue, as we've just explained...
		LOG("Put: discarding off-screen pixel @ (%hu, %hu) (out of %ux%u bounds)",
		    coords.x,
		    coords.y,
		    vInfo.xres,
		    vInfo.yres);
#endif
		return;
	}

	// NOTE: Hmm, here, an if ladder appears to be ever so *slightly* faster than going through the function pointer...
	if (vInfo.bits_per_pixel == 4U) {
		get_pixel_Gray4(&coords, px);
	} else if (vInfo.bits_per_pixel == 8U) {
		get_pixel_Gray8(&coords, px);
	} else if (vInfo.bits_per_pixel == 16U) {
		get_pixel_RGB565(&coords, px);
	} else if (vInfo.bits_per_pixel == 24U) {
		get_pixel_RGB24(&coords, px);
	} else if (vInfo.bits_per_pixel == 32U) {
		get_pixel_RGB32(&coords, px);
	}
}

// Helper function to draw a rectangle in given color
static void
    fill_rect(unsigned short int         x,
	      unsigned short int         y,
	      unsigned short int         w,
	      unsigned short int         h,
	      const FBInkPixel* restrict px)
{
	// Bounds-checking, to ensure the memset won't do stupid things...
	// Do signed maths, to account for the fact that x or y might already be OOB!
	if (x + w > screenWidth) {
		w = (unsigned short int) MAX(0, (w - ((x + w) - (int) screenWidth)));
#ifdef DEBUG
		LOG("Chopped rectangle width to %hu", w);
#endif
	}
	if (y + h > screenHeight) {
		h = (unsigned short int) MAX(0, (h - ((y + h) - (int) screenHeight)));
#ifdef DEBUG
		LOG("Chopped rectangle height to %hu", h);
#endif
	}

	// Abort early if that left us with an empty rectangle ;).
	if (w == 0U || h == 0U) {
#ifdef DEBUG
		LOG("Skipped empty %hux%hu rectangle @ (%hu, %hu)", w, h, x, y);
#endif
		return;
	}

	if (vInfo.bits_per_pixel < 8U) {
		// Go with pixel plotting @ 4bpp to keep this simple...
		for (unsigned short int cy = 0U; cy < h; cy++) {
			for (unsigned short int cx = 0U; cx < w; cx++) {
				FBInkCoordinates coords;
				coords.x = (unsigned short int) (x + cx);
				coords.y = (unsigned short int) (y + cy);
				put_pixel_Gray4(&coords, px);
			}
		}
	} else if (vInfo.bits_per_pixel == 16U && px->gray8 != 0x00u && px->gray8 != 0xFFu) {
		// Same thing @ 16bpp if we're not doing black or white, as those are the only colors in our palette
		// that pack into two indentical bytes when packed as RGB565... -_-".
		FBInkPixel packed_px;
		packed_px.rgb565 = pack_rgb565(px->bgra.color.r, px->bgra.color.g, px->bgra.color.b);
		for (unsigned short int cy = 0U; cy < h; cy++) {
			for (unsigned short int cx = 0U; cx < w; cx++) {
				FBInkCoordinates coords;
				coords.x = (unsigned short int) (x + cx);
				coords.y = (unsigned short int) (y + cy);
				(*fxpRotateCoords)(&coords);
				put_pixel_RGB565(&coords, &packed_px);
			}
		}
	} else {
		struct mxcfb_rect region = {
			.top    = y,
			.left   = x,
			.width  = w,
			.height = h,
		};
		uint8_t bpp = (uint8_t)(vInfo.bits_per_pixel >> 3U);

		(*fxpRotateRegion)(&region);
		for (size_t j = region.top; j < region.top + region.height; j++) {
			uint8_t* p = fbPtr + (fInfo.line_length * j) + (bpp * region.left);
			memset(p, px->gray8, bpp * region.width);
		}
	}
#ifdef DEBUG
	LOG("Filled a #%02hhX %hux%hu rectangle @ (%hu, %hu)", px->gray8, w, h, x, y);
#endif
}

// Helper function to clear the screen - fill whole screen with given color
static void
    clear_screen(int fbfd UNUSED_BY_NOTKINDLE, uint8_t v, bool is_flashing UNUSED_BY_NOTKINDLE)
{
#ifdef FBINK_FOR_KINDLE
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
				WARN("FBIO_EINK_CLEAR_SCREEN: %m");
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
#else
	// NOTE: Apparently, some NTX devices do not appreciate a memset of the full smem_len when they're in a 16bpp mode...
	//       In this mode, smem_len is twice as large as it needs to be,
	//       and Cervantes' Qt driver takes this opportunity to use the offscreen memory region to do some... stuff.
	//       c.f., https://github.com/bq/cervantes-qt/blob/eink-imx508/src/plugins/gfxdrivers/einkfb/einkfb.cpp,
	//       in particular size/psize vs. mapsize
	//       Anyway, don't clobber that, as it seems to cause softlocks on BQ/Cervantes,
	//       and be very conservative, using yres instead of yres_virtual, as Qt *may* already rely on that memory region.
	if (vInfo.bits_per_pixel == 16) {
		memset(fbPtr, v, (size_t)(fInfo.line_length * vInfo.yres));
	} else {
		// NOTE: fInfo.smem_len should actually match fInfo.line_length * vInfo.yres_virtual on 32bpp ;).
		//       Which is how things should always be, but, alas, poor Yorick...
		memset(fbPtr, v, fInfo.smem_len);
	}
#endif
}

// Return the font8x8 bitmap for a specific Unicode codepoint
static const unsigned char*
    font8x8_get_bitmap(uint32_t codepoint)
{
	// Get the bitmap for the character mapped to that Unicode codepoint
	if (codepoint <= 0x7Fu) {
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

static const char*
    fontname_to_string(uint8_t fontname)
{
	switch (fontname) {
		case IBM:
			return "IBM";
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
#endif
		default:
			return "IBM (Default)";
	}
}

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
    draw(const char* restrict        text,
	 unsigned short int          row,
	 unsigned short int          col,
	 unsigned short int          multiline_offset,
	 bool                        halfcell_offset,
	 const FBInkConfig* restrict fbink_cfg)
{
	LOG("Printing '%s' @ line offset %hu (meaning row %hu)",
	    text,
	    multiline_offset,
	    (unsigned short int) (row + multiline_offset));

	FBInkPixel fgP = penFGPixel;
	FBInkPixel bgP = penBGPixel;
	if (fbink_cfg->is_inverted) {
		// NOTE: And, of course, RGB565 is terrible. Inverting the lossy packed value would be even lossier...
		if (vInfo.bits_per_pixel == 16U) {
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
	size_t charcount = u8_strlen2(text);
	size_t txtlength = strlen(text);
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
		.top    = (uint32_t) MAX(0 + (viewVertOrigin - viewVertOffset),
                                      (((row - multiline_offset) * FONTH) + voffset + viewVertOrigin)),
		.left   = (uint32_t) MAX(0 + viewHoriOrigin, ((col * FONTW) + hoffset + viewHoriOrigin)),
		.width  = multiline_offset > 0U ? (screenWidth - (uint32_t)(col * FONTW)) : (uint32_t)(charcount * FONTW),
		.height = (uint32_t)((multiline_offset + 1U) * FONTH),
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
					    (uint32_t)((col * FONTW) + hoffset + viewHoriOrigin + pixel_offset),
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
			fill_rect(
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
			fill_rect(
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
	size_t           bi    = 0U;
	size_t           ch_bi = bi;
	size_t           ci    = 0U;
	uint32_t         ch;
	FBInkCoordinates coords = { 0U };
	FBInkPixel*      pxP;
	// NOTE: We don't do much sanity checking on hoffset/voffset,
	//       because we want to allow pushing part of the string off-screen
	//       (we basically only make sure it won't screw up the region rectangle too badly).
	//       put_pixel is checked, and will discard off-screen pixels safely.
	//       Because we store the final position in an unsigned value, this means that, to some extent,
	//       we rely on wraparound on underflow to still point to (large, but positive) off-screen coordinates.
	unsigned short int x_base_offs = (unsigned short int) ((col * FONTW) + pixel_offset + hoffset + viewHoriOrigin);
	unsigned short int y_offs      = (unsigned short int) ((row * FONTH) + voffset + viewVertOrigin);

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
#ifdef FBINK_WITH_FONTS
	if (glyphWidth <= 8) {
#endif
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
			unsigned short int x_offs = (unsigned short int) (x_base_offs + (ci * FONTW));
			// Remember the next char's byte offset for next iteration's logging
			ch_bi = bi;

			// Get the glyph's pixmap (width <= 8 -> uint8_t)
			const unsigned char* restrict bitmap = NULL;
#ifdef FBINK_WITH_FONTS
			bitmap = (*fxpFont8xGetBitmap)(ch);
#else
		bitmap = font8x8_get_bitmap(ch);
#endif

			// Crappy macro to avoid repeating myself in each branch...
#define RENDER_GLYPH()                                                                                                   \
	/* NOTE: We only need to loop on the base glyph's dimensions (i.e., the bitmap resolution), */                   \
	/*       and from there compute the extra pixels for that single input pixel given our scaling factor... */      \
	if (!fbink_cfg->is_overlay && !fbink_cfg->is_bgless && !fbink_cfg->is_fgless) {                                  \
		for (uint8_t y = 0U; y < glyphHeight; y++) {                                                             \
			/* y: input row, j: first output row after scaling */                                            \
			j = (unsigned short int) (y * FONTSIZE_MULT);                                                    \
			for (uint8_t x = 0U; x < glyphWidth; x++) {                                                      \
				/* x: input column, i: first output column after scaling */                              \
				i = (unsigned short int) (x * FONTSIZE_MULT);                                            \
				/* Initial coordinates, before we generate the extra pixels from the scaling factor */   \
				cx = (unsigned short int) (x_offs + i);                                                  \
				cy = (unsigned short int) (y_offs + j);                                                  \
				/* Each element encodes a full row, we access a column's bit in that row by shifting. */ \
				if (bitmap[y] & 1U << x) {                                                               \
					/* bit was set, pixel is fg! */                                                  \
					/* Handle scaling by drawing a FONTSIZE_MULTpx square per pixel ;) */            \
					fill_rect(cx, cy, FONTSIZE_MULT, FONTSIZE_MULT, &fgP);                           \
				} else {                                                                                 \
					/* bit was unset, pixel is bg */                                                 \
					fill_rect(cx, cy, FONTSIZE_MULT, FONTSIZE_MULT, &bgP);                           \
				}                                                                                        \
			}                                                                                                \
		}                                                                                                        \
	} else {                                                                                                         \
		FBInkPixel fbP     = { 0U };                                                                             \
		bool       is_fgpx = false;                                                                              \
		for (uint8_t y = 0U; y < glyphHeight; y++) {                                                             \
			/* y: input row, j: first output row after scaling */                                            \
			j = (unsigned short int) (y * FONTSIZE_MULT);                                                    \
			for (uint8_t x = 0U; x < glyphWidth; x++) {                                                      \
				/* x: input column, i: first output column after scaling */                              \
				i = (unsigned short int) (x * FONTSIZE_MULT);                                            \
				/* Each element encodes a full row, we access a column's bit in that row by shifting. */ \
				if (bitmap[y] & 1U << x) {                                                               \
					/* bit was set, pixel is fg! */                                                  \
					pxP     = &fgP;                                                                  \
					is_fgpx = true;                                                                  \
				} else {                                                                                 \
					/* bit was unset, pixel is bg */                                                 \
					pxP     = &bgP;                                                                  \
					is_fgpx = false;                                                                 \
				}                                                                                        \
				/* Initial coordinates, before we generate the extra pixels from the scaling factor */   \
				cx = (unsigned short int) (x_offs + i);                                                  \
				cy = (unsigned short int) (y_offs + j);                                                  \
				/* NOTE: Apply our scaling factor in both dimensions! */                                 \
				for (uint8_t l = 0U; l < FONTSIZE_MULT; l++) {                                           \
					for (uint8_t k = 0U; k < FONTSIZE_MULT; k++) {                                   \
						coords.x = (unsigned short int) (cx + k);                                \
						coords.y = (unsigned short int) (cy + l);                                \
						/* In overlay mode, we only print foreground pixels, */                  \
						/* and we print in the inverse color of the underlying pixel's */        \
						/* Obviously, the closer we get to GRAY7, the less contrast we get */    \
						if (is_fgpx && !fbink_cfg->is_fgless) {                                  \
							if (fbink_cfg->is_overlay) {                                     \
								get_pixel(coords, &fbP);                                 \
								fbP.bgra.p ^= 0x00FFFFFFu;                               \
								pxP = &fbP;                                              \
								put_pixel(coords, pxP, false);                           \
							} else {                                                         \
								put_pixel(coords, pxP, true);                            \
							}                                                                \
						} else if (!is_fgpx && fbink_cfg->is_fgless) {                           \
							put_pixel(coords, pxP, true);                                    \
						}                                                                        \
					}                                                                                \
				}                                                                                        \
			}                                                                                                \
		}                                                                                                        \
	}

			RENDER_GLYPH();
			// NOTE: If we did not mirror the bitmasks during conversion,
			//       another approach to the fact that Unifont's hex format encodes columns in the reverse order
			//       is simply to access columns in the reverse order ;).
			//       i.e., switch the inner column loop to:
			//       for (int8_t x = (glyphWidth - 1); x >= 0; x--) {
			//           i = (unsigned short int) ((glyphWidth - x) * FONTSIZE_MULT);

			// Next glyph! This serves as the source for the pen position, hence it being used as an index...
			ci++;
		}
#ifdef FBINK_WITH_FONTS
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
			unsigned short int x_offs = (unsigned short int) (x_base_offs + (ci * FONTW));
			// Remember the next char's byte offset for next iteration's logging
			ch_bi = bi;

			// Get the glyph's pixmap (width <= 16 -> uint16_t)
			const uint16_t* restrict bitmap = NULL;
			bitmap                          = (*fxpFont16xGetBitmap)(ch);

			// Render, scale & plot!
			RENDER_GLYPH();

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
			unsigned short int x_offs = (unsigned short int) (x_base_offs + (ci * FONTW));
			// Remember the next char's byte offset for next iteration's logging
			ch_bi = bi;

			// Get the glyph's pixmap (width <= 32 -> uint32_t)
			const uint32_t* restrict bitmap = NULL;
			bitmap                          = (*fxpFont32xGetBitmap)(ch);

			// Render, scale & plot!
			RENDER_GLYPH();

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
			unsigned short int x_offs = (unsigned short int) (x_base_offs + (ci * FONTW));
			// Remember the next char's byte offset for next iteration's logging
			ch_bi = bi;

			// Get the glyph's pixmap (width <= 64 -> uint64_t)
			const uint64_t* restrict bitmap = NULL;
			bitmap = (*fxpFont64xGetBitmap)(ch);

			// Render, scale & plot!
			RENDER_GLYPH();

			// Next glyph! This serves as the source for the pen position, hence it being used as an index...
			ci++;
		}
	*/
	}
#endif

	return region;
}

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
static long int
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
			WARN("FBIO_EINK_UPDATE_DISPLAY: %m");
		} else {
			WARN("FBIO_EINK_UPDATE_DISPLAY_AREA: %m");
		}
		return ERRCODE(EXIT_FAILURE);
	}

	return EXIT_SUCCESS;
}

// All mxcfb Kindle devices ([K5<->??)
static int
    wait_for_submission_kindle(int fbfd, uint32_t marker)
{
	int rv;
	rv = ioctl(fbfd, MXCFB_WAIT_FOR_UPDATE_SUBMISSION, &marker);

	if (rv < 0) {
		WARN("MXCFB_WAIT_FOR_UPDATE_SUBMISSION: %m");
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
    refresh_kindle(int                     fbfd,
		   const struct mxcfb_rect region,
		   uint32_t                waveform_mode,
		   uint32_t                update_mode,
		   bool                    is_nightmode,
		   uint32_t                marker)
{
	struct mxcfb_update_data update = {
		.update_region         = region,
		.waveform_mode         = waveform_mode,
		.update_mode           = update_mode,
		.update_marker         = marker,
		.hist_bw_waveform_mode = (waveform_mode == WAVEFORM_MODE_REAGL) ? WAVEFORM_MODE_REAGL : WAVEFORM_MODE_DU,
		.hist_gray_waveform_mode =
		    (waveform_mode == WAVEFORM_MODE_REAGL) ? WAVEFORM_MODE_REAGL : WAVEFORM_MODE_GC16_FAST,
		.temp            = TEMP_USE_AUTO,
		.flags           = (waveform_mode == WAVEFORM_MODE_A2) ? EPDC_FLAG_FORCE_MONOCHROME : 0U,
		.alt_buffer_data = { 0U },
	};

	if (is_nightmode && deviceQuirks.canHWInvert) {
		update.flags |= EPDC_FLAG_ENABLE_INVERSION;
	}

	int rv;
	rv = ioctl(fbfd, MXCFB_SEND_UPDATE, &update);

	if (rv < 0) {
		WARN("MXCFB_SEND_UPDATE: %m");
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

// Touch Kindle devices with a Pearl screen ([K5<->PW1])
static int
    wait_for_complete_kindle_pearl(int fbfd, uint32_t marker)
{
	int rv;
	rv = ioctl(fbfd, MXCFB_WAIT_FOR_UPDATE_COMPLETE_PEARL, &marker);

	if (rv < 0) {
		WARN("MXCFB_WAIT_FOR_UPDATE_COMPLETE_PEARL: %m");
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
	int rv;
	rv = ioctl(fbfd, MXCFB_WAIT_FOR_UPDATE_COMPLETE, &update_marker);

	if (rv < 0) {
		WARN("MXCFB_WAIT_FOR_UPDATE_COMPLETE: %m");
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

// Kindle Oasis 2 & Oasis 3 ([KOA2<->??)
static int
    refresh_kindle_zelda(int                     fbfd,
			 const struct mxcfb_rect region,
			 uint32_t                waveform_mode,
			 uint32_t                update_mode,
			 int                     dithering_mode,
			 bool                    is_nightmode,
			 uint32_t                marker)
{
	struct mxcfb_update_data_zelda update = {
		.update_region = region,
		.waveform_mode = waveform_mode,
		.update_mode   = update_mode,
		.update_marker = marker,
		.temp          = TEMP_USE_AMBIENT,
		.flags         = (waveform_mode == WAVEFORM_MODE_ZELDA_GLD16)
			     ? EPDC_FLAG_USE_ZELDA_REGAL
			     : (waveform_mode == WAVEFORM_MODE_ZELDA_A2) ? EPDC_FLAG_FORCE_MONOCHROME : 0U,
		.dither_mode = dithering_mode,
		.quant_bit   = (dithering_mode == EPDC_FLAG_USE_DITHERING_PASSTHROUGH)
				 ? 0
				 : (waveform_mode == WAVEFORM_MODE_ZELDA_A2 || waveform_mode == WAVEFORM_MODE_DU)
				       ? 1
				       : (waveform_mode == WAVEFORM_MODE_GC4 ||
					  waveform_mode == WAVEFORM_MODE_ZELDA_GL4 || waveform_mode == WAVEFORM_MODE_DU4)
					     ? 3
					     : 7,
		.alt_buffer_data = { 0U },
		.hist_bw_waveform_mode =
		    (waveform_mode == WAVEFORM_MODE_ZELDA_REAGL) ? WAVEFORM_MODE_ZELDA_REAGL : WAVEFORM_MODE_DU,
		.hist_gray_waveform_mode =
		    (waveform_mode == WAVEFORM_MODE_ZELDA_REAGL) ? WAVEFORM_MODE_ZELDA_REAGL : WAVEFORM_MODE_GC16,
		.ts_pxp  = 0U,
		.ts_epdc = 0U,
	};

	if (is_nightmode && deviceQuirks.canHWInvert) {
		update.flags |= EPDC_FLAG_ENABLE_INVERSION;
	}

	int rv;
	rv = ioctl(fbfd, MXCFB_SEND_UPDATE_ZELDA, &update);

	if (rv < 0) {
		WARN("MXCFB_SEND_UPDATE_ZELDA: %m");
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

// Kindle PaperWhite 4 & Basic 3 ([PW4<->??)
static int
    refresh_kindle_rex(int                     fbfd,
		       const struct mxcfb_rect region,
		       uint32_t                waveform_mode,
		       uint32_t                update_mode,
		       int                     dithering_mode,
		       bool                    is_nightmode,
		       uint32_t                marker)
{
	// NOTE: Different mcfb_update_data struct (no ts_* debug fields), but otherwise, identical to the zelda one!
	struct mxcfb_update_data_rex update = {
		.update_region = region,
		.waveform_mode = waveform_mode,
		.update_mode   = update_mode,
		.update_marker = marker,
		.temp          = TEMP_USE_AMBIENT,
		.flags         = (waveform_mode == WAVEFORM_MODE_ZELDA_GLD16)
			     ? EPDC_FLAG_USE_ZELDA_REGAL
			     : (waveform_mode == WAVEFORM_MODE_ZELDA_A2) ? EPDC_FLAG_FORCE_MONOCHROME : 0U,
		.dither_mode = dithering_mode,
		.quant_bit   = (dithering_mode == EPDC_FLAG_USE_DITHERING_PASSTHROUGH)
				 ? 0
				 : (waveform_mode == WAVEFORM_MODE_ZELDA_A2 || waveform_mode == WAVEFORM_MODE_DU)
				       ? 1
				       : (waveform_mode == WAVEFORM_MODE_GC4 ||
					  waveform_mode == WAVEFORM_MODE_ZELDA_GL4 || waveform_mode == WAVEFORM_MODE_DU4)
					     ? 3
					     : 7,
		.alt_buffer_data = { 0U },
		.hist_bw_waveform_mode =
		    (waveform_mode == WAVEFORM_MODE_ZELDA_REAGL) ? WAVEFORM_MODE_ZELDA_REAGL : WAVEFORM_MODE_DU,
		.hist_gray_waveform_mode =
		    (waveform_mode == WAVEFORM_MODE_ZELDA_REAGL) ? WAVEFORM_MODE_ZELDA_REAGL : WAVEFORM_MODE_GC16,
	};

	if (is_nightmode && deviceQuirks.canHWInvert) {
		update.flags |= EPDC_FLAG_ENABLE_INVERSION;
	}

	int rv;
	rv = ioctl(fbfd, MXCFB_SEND_UPDATE_REX, &update);

	if (rv < 0) {
		WARN("MXCFB_SEND_UPDATE_REX: %m");
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
    refresh_cervantes(int fbfd,
		      const struct mxcfb_rect region,
		      uint32_t waveform_mode,
		      uint32_t update_mode,
		      bool is_nightmode,
		      uint32_t marker)
{
	struct mxcfb_update_data update = {
		.update_region = region,
		.waveform_mode = waveform_mode,
		.update_mode = update_mode,
		.update_marker = marker,
		.temp = TEMP_USE_AMBIENT,
		.flags = (waveform_mode == WAVEFORM_MODE_REAGLD)
			     ? EPDC_FLAG_USE_AAD
			     : (waveform_mode == WAVEFORM_MODE_A2) ? EPDC_FLAG_FORCE_MONOCHROME : 0U,
		.alt_buffer_data = { 0U },
	};

	if (is_nightmode && deviceQuirks.canHWInvert) {
		update.flags |= EPDC_FLAG_ENABLE_INVERSION;
	}

	int rv;
	rv = ioctl(fbfd, MXCFB_SEND_UPDATE, &update);

	if (rv < 0) {
		WARN("MXCFB_SEND_UPDATE: %m");
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

// NOTE: We *could* theoretically use MXCFB_WAIT_FOR_UPDATE_COMPLETE2 on 2013+ stuff (C2+)
//       But, like on Kobo, don't bother, we'd gain nothing by switching anyway ;).
static int
    wait_for_complete_cervantes(int fbfd, uint32_t marker)
{
	int rv;
	rv = ioctl(fbfd, MXCFB_WAIT_FOR_UPDATE_COMPLETE, &marker);

	if (rv < 0) {
		WARN("MXCFB_WAIT_FOR_UPDATE_COMPLETE: %m");
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
    refresh_remarkable(int                     fbfd,
		       const struct mxcfb_rect region,
		       uint32_t                waveform_mode,
		       uint32_t                update_mode,
		       bool                    is_nightmode,
		       uint32_t                marker)
{
	// NOTE: Actually uses the V1 epdc driver, hence dither & quant_bit being unused.
	// NOTE: The USE_DITHERING flags (based on Atkison's algo) *ought* to be supported, though,
	//       but the only available choices are Y1 (monochrome) and Y4, so it's not as useful in practice,
	//       especially with no clear identification of a Y4-friendly waveform mode,
	//       (i.e., no GC4, and no conclusive tests with DU4 & GL4)...
	//       Y1 might work/be mildly useful for A2 & DU, though.
	struct mxcfb_update_data update = { .update_region = region,
					    .waveform_mode = waveform_mode,
					    .update_mode   = update_mode,
					    .update_marker = marker,
					    .temp          = TEMP_USE_REMARKABLE,
					    .flags         = (waveform_mode == WAVEFORM_MODE_REAGLD)
							 ? EPDC_FLAG_USE_REGAL
							 : (waveform_mode == WAVEFORM_MODE_A2)
							       ? EPDC_FLAG_FORCE_MONOCHROME
							       : 0U,
					    .dither_mode     = 0,
					    .quant_bit       = 0,
					    .alt_buffer_data = { 0U } };

	if (is_nightmode && deviceQuirks.canHWInvert) {
		update.flags |= EPDC_FLAG_ENABLE_INVERSION;
	}

	int rv;
	rv = ioctl(fbfd, MXCFB_SEND_UPDATE, &update);

	if (rv < 0) {
		WARN("MXCFB_SEND_UPDATE: %m");
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
    wait_for_complete_remarkable(int fbfd, uint32_t marker)
{
	struct mxcfb_update_marker_data update_data = { .update_marker = marker, .collision_test = 0U };
	int                             rv;
	rv = ioctl(fbfd, MXCFB_WAIT_FOR_UPDATE_COMPLETE, &update_data);

	if (rv < 0) {
		WARN("MXCFB_WAIT_FOR_UPDATE_COMPLETE: %m");
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
    refresh_kobo(int                     fbfd,
		 const struct mxcfb_rect region,
		 uint32_t                waveform_mode,
		 uint32_t                update_mode,
		 bool                    is_nightmode,
		 uint32_t                marker)
{
	struct mxcfb_update_data_v1_ntx update = {
		.update_region = region,
		.waveform_mode = waveform_mode,
		.update_mode   = update_mode,
		.update_marker = marker,
		.temp          = TEMP_USE_AMBIENT,
		.flags         = (waveform_mode == WAVEFORM_MODE_REAGLD)
			     ? EPDC_FLAG_USE_AAD
			     : (waveform_mode == WAVEFORM_MODE_A2) ? EPDC_FLAG_FORCE_MONOCHROME : 0U,
		.alt_buffer_data = { 0U },
	};

	if (is_nightmode && deviceQuirks.canHWInvert) {
		update.flags |= EPDC_FLAG_ENABLE_INVERSION;
	}

	int rv;
	rv = ioctl(fbfd, MXCFB_SEND_UPDATE_V1_NTX, &update);

	if (rv < 0) {
		WARN("MXCFB_SEND_UPDATE_V1_NTX: %m");
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
    wait_for_complete_kobo(int fbfd, uint32_t marker)
{
	int rv;
	rv = ioctl(fbfd, MXCFB_WAIT_FOR_UPDATE_COMPLETE_V1, &marker);

	if (rv < 0) {
		WARN("MXCFB_WAIT_FOR_UPDATE_COMPLETE_V1: %m");
		return ERRCODE(EXIT_FAILURE);
	} else {
		if (rv == 0) {
			LOG("Update %u has already fully been completed", marker);
		} else {
			// NOTE: Timeout is set to 10000ms
			LOG("Waited %ldms for completion of update %u", (10000 - jiffies_to_ms(rv)), marker);
		}
	}

	return EXIT_SUCCESS;
}

// Kobo Mark 7 devices ([Mk7<->??)
static int
    refresh_kobo_mk7(int                     fbfd,
		     const struct mxcfb_rect region,
		     uint32_t                waveform_mode,
		     uint32_t                update_mode,
		     int                     dithering_mode,
		     bool                    is_nightmode,
		     uint32_t                marker)
{
	struct mxcfb_update_data_v2 update = {
		.update_region = region,
		.waveform_mode = waveform_mode,
		.update_mode   = update_mode,
		.update_marker = marker,
		.temp          = TEMP_USE_AMBIENT,
		.flags         = (waveform_mode == WAVEFORM_MODE_GLD16)
			     ? EPDC_FLAG_USE_REGAL
			     : (waveform_mode == WAVEFORM_MODE_A2) ? EPDC_FLAG_FORCE_MONOCHROME : 0U,
		.dither_mode = dithering_mode,
		.quant_bit   = (dithering_mode == EPDC_FLAG_USE_DITHERING_PASSTHROUGH)
				 ? 0
				 : (waveform_mode == WAVEFORM_MODE_A2 || waveform_mode == WAVEFORM_MODE_DU)
				       ? 1
				       : (waveform_mode == WAVEFORM_MODE_GC4) ? 3 : 7,
		.alt_buffer_data = { 0U },
	};

	if (is_nightmode && deviceQuirks.canHWInvert) {
		update.flags |= EPDC_FLAG_ENABLE_INVERSION;
	}

	int rv;
	rv = ioctl(fbfd, MXCFB_SEND_UPDATE_V2, &update);

	if (rv < 0) {
		WARN("MXCFB_SEND_UPDATE_V2: %m");
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
    wait_for_complete_kobo_mk7(int fbfd, uint32_t marker)
{
	struct mxcfb_update_marker_data update_marker = {
		.update_marker  = marker,
		.collision_test = 0U,
	};
	int rv;
	rv = ioctl(fbfd, MXCFB_WAIT_FOR_UPDATE_COMPLETE_V3, &update_marker);

	if (rv < 0) {
		WARN("MXCFB_WAIT_FOR_UPDATE_COMPLETE_V3: %m");
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

// And finally, dispatch the right refresh request for our HW...
#ifdef FBINK_FOR_LINUX
// NOP when we don't have an eInk screen ;).
static int
    refresh(int                     fbfd __attribute__((unused)),
	    const struct mxcfb_rect region __attribute__((unused)),
	    uint32_t                waveform_mode __attribute__((unused)),
	    int                     dithering_mode __attribute__((unused)),
	    bool                    is_nightmode __attribute__((unused)),
	    bool                    is_flashing __attribute__((unused)),
	    bool                    no_refresh __attribute__((unused)))
{
	return EXIT_SUCCESS;
}
#else
static int
    refresh(int fbfd,
	    const struct mxcfb_rect region,
	    uint32_t waveform_mode,
	    int dithering_mode UNUSED_BY_CERVANTES UNUSED_BY_REMARKABLE,
	    bool is_nightmode,
	    bool is_flashing,
	    bool no_refresh)
{
	// Were we asked to skip refreshes?
	if (no_refresh) {
		LOG("Skipping eInk refresh, as requested.");
		return EXIT_SUCCESS;
	}

	// NOTE: Discard bogus regions, they can cause a softlock on some devices.
	//       A 0x0 region is a no go on most devices, while a 1x1 region may only upset some Kindle models.
	//       Some devices even balk at 1xN or Nx1, so, catch that, too.
	if (region.width <= 1 || region.height <= 1) {
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
		return refresh_legacy(fbfd, region, is_flashing);
	}
#	endif
	// NOTE: While we'd be perfect candidates for using A2 waveform mode, it's all kinds of fucked up on Kobos,
	//       and may lead to disappearing text or weird blending depending on the surrounding fb content...
	//       It only shows up properly when FULL, which isn't great...
	// NOTE: On the Forma, the (apparently randomly) broken A2 behavior is exacerbated if the FB is UR @ 8bpp...
	//       Which is intriguing, because that should make the driver's job easier (except maybe not on latest epdc v2 revs),
	//       c.f., epdc_submit_work_func @ drivers/video/fbdev/mxc/mxc_epdc_v2_fb.c
	// NOTE: And while we're on the fun quirks train: FULL never flashes w/ AUTO on (some?) Kobos,
	//       so request GC16 if we want a flash...
	// NOTE: FWIW, DU behaves properly when PARTIAL, but doesn't flash when FULL.
	//       Which somewhat tracks given AUTO's behavior on Kobos, as well as on Kindles.
	//       (i.e., DU or GC16 is most likely often what AUTO will land on).

	// So, handle this common switcheroo here...
	uint32_t wfm = (is_flashing && waveform_mode == WAVEFORM_MODE_AUTO) ? WAVEFORM_MODE_GC16 : waveform_mode;
	uint32_t upm = is_flashing ? UPDATE_MODE_FULL : UPDATE_MODE_PARTIAL;
	// We'll want to increment the marker on each subsequent calls (for API users)...
	if (lastMarker == 0U) {
		// Seed it with our PID
		lastMarker = (uint32_t) getpid();
	} else {
		lastMarker++;
	}

	// NOTE: Make sure update_marker is valid, an invalid marker *may* hang the kernel instead of failing gracefully,
	//       depending on the device/FW...
	if (lastMarker == 0U) {
		lastMarker = ('F' + 'B' + 'I' + 'n' + 'k');
		// i.e.,  70  + 66  + 73  + 110 + 107
	}

#	if defined(FBINK_FOR_KINDLE)
	if (deviceQuirks.isKindleRex) {
		return refresh_kindle_rex(fbfd, region, wfm, upm, dithering_mode, is_nightmode, lastMarker);
	} else if (deviceQuirks.isKindleZelda) {
		return refresh_kindle_zelda(fbfd, region, wfm, upm, dithering_mode, is_nightmode, lastMarker);
	} else {
		return refresh_kindle(fbfd, region, wfm, upm, is_nightmode, lastMarker);
	}
#	elif defined(FBINK_FOR_CERVANTES)
	return refresh_cervantes(fbfd, region, wfm, upm, is_nightmode, lastMarker);
#	elif defined(FBINK_FOR_REMARKABLE)
	return refresh_remarkable(fbfd, region, wfm, upm, is_nightmode, lastMarker);
#	elif defined(FBINK_FOR_KOBO)
	if (deviceQuirks.isKoboMk7) {
		return refresh_kobo_mk7(fbfd, region, wfm, upm, dithering_mode, is_nightmode, lastMarker);
	} else {
		return refresh_kobo(fbfd, region, wfm, upm, is_nightmode, lastMarker);
	}
#	endif    // FBINK_FOR_KINDLE
}
#endif            // FBINK_FOR_LINUX

// Same thing for WAIT_FOR_UPDATE_SUBMISSION requests...
#ifndef FBINK_FOR_LINUX
#	if defined(FBINK_FOR_KINDLE)
// This is only implemented on Kindle kernels...
static int
    wait_for_submission(int fbfd, uint32_t marker)
{
	// Only implemented for mxcfb Kindles...
	if (deviceQuirks.isKindleLegacy) {
		return EXIT_SUCCESS;
	} else {
		return wait_for_submission_kindle(fbfd, marker);
	}
}
#	endif    // FBINK_FOR_KINDLE

// Same thing for WAIT_FOR_UPDATE_COMPLETE requests...
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
	if (deviceQuirks.isKoboMk7) {
		return wait_for_complete_kobo_mk7(fbfd, marker);
	} else {
		return wait_for_complete_kobo(fbfd, marker);
	}
#	elif defined(FBINK_FOR_REMARKABLE)
	return wait_for_complete_remarkable(fbfd, marker);
#	endif    // FBINK_FOR_KINDLE
}
#endif    // !FBINK_FOR_LINUX

// Open the framebuffer file & return the opened fd
int
    fbink_open(void)
{
	int fbfd = -1;

	// Open the framebuffer file for reading and writing
	fbfd = open("/dev/fb0", O_RDWR | O_CLOEXEC);
	if (fbfd == -1) {
		WARN("Cannot open framebuffer character device: %m");
		return ERRCODE(EXIT_FAILURE);
	}

	return fbfd;
}

// Internal version of this which keeps track of whether we were fed an already opened fd or not...
static int
    open_fb_fd(int* restrict fbfd, bool* restrict keep_fd)
{
	if (*fbfd == FBFD_AUTO) {
		// If we're opening a fd now, don't keep it around.
		*keep_fd = false;
		if (ERRCODE(EXIT_FAILURE) == (*fbfd = fbink_open())) {
			WARN("Failed to open the framebuffer character device, aborting");
			return ERRCODE(EXIT_FAILURE);
		}
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
		*fbfd = open("/dev/fb0", O_RDONLY | O_NONBLOCK | O_CLOEXEC);
		if (!*fbfd) {
			WARN("Cannot open framebuffer character device (%m), aborting");
			return ERRCODE(EXIT_FAILURE);
		}
	}

	return EXIT_SUCCESS;
}

static const char*
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

// Get the various fb info & setup global variables
static int
    initialize_fbink(int fbfd, const FBInkConfig* restrict fbink_cfg, bool skip_vinfo)
{
	// Open the framebuffer if need be (nonblock, we'll only do ioctls)...
	bool keep_fd = true;
	if (open_fb_fd_nonblock(&fbfd, &keep_fd) != EXIT_SUCCESS) {
		return ERRCODE(EXIT_FAILURE);
	}

	// Assume success, until shit happens ;)
	int rv = EXIT_SUCCESS;

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

	// Start with some more generic stuff, not directly related to the framebuffer.
	// As all this stuff is pretty much set in stone, we'll only query it once.
	if (!deviceQuirks.skipId) {
#ifndef FBINK_FOR_LINUX
		// Identify the device's specific model...
		identify_device();
#	if defined(FBINK_FOR_KINDLE)
		if (deviceQuirks.isKindleLegacy) {
			ELOG("Enabled Legacy einkfb Kindle quirks");
		} else if (deviceQuirks.isKindlePearlScreen) {
			ELOG("Enabled Kindle with Pearl screen quirks");
		} else if (deviceQuirks.isKindleZelda) {
			ELOG("Enabled Kindle Zelda platform quirks");
		} else if (deviceQuirks.isKindleRex) {
			ELOG("Enabled Kindle Rex platform quirks");
		}
#	elif defined(FBINK_FOR_KOBO)
		if (deviceQuirks.isKoboNonMT) {
			ELOG("Enabled Kobo w/o Multi-Touch quirks");
		} else if (deviceQuirks.isKoboMk7) {
			ELOG("Enabled Kobo Mark 7 quirks");
		}
#	endif
#endif

		// Ask the system for its clock tick frequency so we can translate jiffies into human-readable units.
		// NOTE: This will most likely be 100, even if CONFIG_HZ is > 100
		//       c.f., sysconf(3)
		long int rc = sysconf(_SC_CLK_TCK);
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
			WARN("Error reading variable fb information: %m");
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
	if (vInfo.xres > vInfo.yres) {
		// NOTE: PW2:
		//         vInfo.rotate == 2 in Landscape (vs. 3 in Portrait mode), w/ the xres/yres switch in Landscape,
		//         and (0, 0) is always at the top-left of the viewport, so we're always correct.
		//       Kindle Touch:
		//         Doesn't propose a Landscape mode, and defaults to vInfo.rotate == 1
		//       K4:
		//         It supports the four possible rotations, and while it is always using vInfo.rotate == 0,
		//         xres & yres switch accordingly when in Landscape modes,
		//         and (0, 0) is always at the top-left of the viewport, so we're always correct.
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
		if (vInfo.bits_per_pixel == 16U) {
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
		} else if (vInfo.bits_per_pixel == 8U) {
			// We also need to account for the fact that KOReader *might* have switched to 8bpp,
			// and done so from this quirky pickel state (as that's what KSM emulates).
			// We explicitly skip this check on the Forma, despite it being unreachable under normal circumstances:
			// because someone, some day, might want to take advantage of an 8bpp fb *AND* do hardware rotations,
			// and we don't want to misbehave there ;).
			if (!deviceQuirks.canRotate && vInfo.rotate == (deviceQuirks.ntxBootRota ^ 2)) {
				// NOTE: On affected devices, pickel is helpfully using the invert of the boot rotation ;).
				// NOTE: As mentioned earlier, in normal conditions, the Forma is blissfully immune,
				//       since pickel defaults to a Portrait orientation (i.e., xres < yres, despite it being CCW),
				//       so we don't even enter this xres > yres branch:
				//       given that, the !canRotate check is simply to future-proof this for custom use-cases ;).
				// NOTE: Because of course, everything is terrible,
				//       current unofficial KSM 9 builds for the Forma are doing it wrong:
				//       they're setting the rota to UR (i.e., like pickel on most !6.8" panels) instead of CCW.
				// Correct screenWidth & screenHeight, so we do all our row/column arithmetics on the right values...
				screenWidth                    = vInfo.yres;
				screenHeight                   = vInfo.xres;
				deviceQuirks.isNTX16bLandscape = true;
				// We only care about the *pickel* state (mainly to avoid false-positives)
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

	// NOTE: Well, granted, this next part is (hopefully) Kobo-specific ;).
	// Handle the Kobo viewport trickery for the few devices with hidden rows of pixels...
	// Things should generally not be broken-by-design on the horizontal axis...
	viewWidth      = screenWidth;
	viewHoriOrigin = 0U;
	// But on the vertical axis, oh my...
	if (!fbink_cfg->no_viewport && deviceQuirks.koboVertOffset != 0) {
		viewHeight = screenHeight - (uint32_t) abs(deviceQuirks.koboVertOffset);
		if (deviceQuirks.koboVertOffset > 0) {
			// Rows of pixels are hidden at the top
			viewVertOrigin = (uint8_t) deviceQuirks.koboVertOffset;
		} else {
			// Rows of pixels are hidden at the bottom
			viewVertOrigin = 0U;
		}
		ELOG("Enabled Kobo viewport insanity (%ux%u -> %ux%u), top-left corner is @ (%hhu, %hhu)",
		     screenWidth,
		     screenHeight,
		     viewWidth,
		     viewHeight,
		     viewHoriOrigin,
		     viewVertOrigin);
	} else {
		// Device is not utterly mad, the top-left corner is at (0, 0)!
		viewHeight     = screenHeight;
		viewVertOrigin = 0U;
	}
#else
	// Kindle devices are generally never broken-by-design (at least not on that front ;))
	viewWidth = screenWidth;
	viewHoriOrigin = 0U;
	viewHeight = screenHeight;
	viewVertOrigin = 0U;
#endif

	// NOTE: Set (& reset) original font resolution, in case we're re-init'ing,
	//       since we're relying on the default value to calculate the scaled value,
	//       and we're using this value to set MAXCOLS & MAXROWS, which we *need* to be sane.
#ifdef FBINK_WITH_FONTS
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
			glyphWidth  = 32U;
			glyphHeight = 32U;
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
		case IBM:
		default:
			glyphWidth         = 8U;
			glyphHeight        = 8U;
			fxpFont8xGetBitmap = &font8x8_get_bitmap;
			break;
	}
#else
	// Default font is IBM
	glyphWidth = 8U;
	glyphHeight = 8U;
	fxpFont8xGetBitmap = &font8x8_get_bitmap;

	if (fbink_cfg->fontname != IBM) {
		ELOG("Custom fonts are not supported in this FBInk build, using IBM instead.");
	}
#endif

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
		uint8_t max_fontmult_width = (uint8_t)(viewWidth / min_maxcols / glyphWidth);
		// We want at least 1 row, so, viewHeight / glyphHeight gives us the maximum multiplier.
		uint8_t max_fontmult_height = (uint8_t)(viewHeight / glyphHeight);
		max_fontmult                = (uint8_t) MIN(max_fontmult_width, max_fontmult_height);
		if (FONTSIZE_MULT > max_fontmult) {
			FONTSIZE_MULT = max_fontmult;
			ELOG("Clamped font size multiplier from %hhu to %hhu", fbink_cfg->fontmult, max_fontmult);
		}
#else
		// The default font's glyphs are 8x8, do the least amount of work possible ;).
		max_fontmult = (uint8_t)(viewWidth / min_maxcols / 8U);
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
			uint32_t actual_height = MAX(vInfo.xres, vInfo.yres);
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
		} else if (fbink_cfg->fontname == SPLEEN) {
			// Spleen is roughly twice as wide as other fonts, compensate for that...
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
	if ((uint32_t)(FONTW * MAXCOLS) == viewWidth) {
		deviceQuirks.isPerfectFit = true;
		ELOG("Horizontal fit is perfect!");
	} else {
		deviceQuirks.isPerfectFit = false;
	}

	// In a similar fashion, add a vertical offset to make sure rows are vertically "centered",
	// in case we can't perfectly fit the final row.
	if ((uint32_t)(FONTH * MAXROWS) == viewHeight) {
		viewVertOffset = 0U;
	} else {
		// NOTE: That should also fall under no_viewport's purview
		if (!fbink_cfg->no_viewport) {
			viewVertOffset = (uint8_t)(((float) (viewHeight - (uint32_t)(FONTH * MAXROWS)) / 2.0f) + 0.5f);
			ELOG("Vertical fit isn't perfect, shifting rows down by %hhu pixels", viewVertOffset);
		} else {
			viewVertOffset = 0U;
			ELOG("Vertical fit isn't perfect, but viewport fiddling was explicitly disabled");
		}
	}
	// Bake that into the viewport computations,
	// we'll special-case the image codepath to ignore it when row is unspecified (i.e., 0) ;).
	viewVertOrigin = (uint8_t)(viewVertOrigin + viewVertOffset);

	// Get fixed screen information
	if (ioctl(fbfd, FBIOGET_FSCREENINFO, &fInfo)) {
		WARN("Error reading fixed fb information: %m");
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

	// NOTE: Now that we know which device we're running on, setup pen colors,
	//       taking into account the inverted cmap on legacy Kindles...
#ifdef FBINK_FOR_KINDLE
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
#endif
		penFGColor = eInkFGCMap[fbink_cfg->fg_color];
		penBGColor = eInkBGCMap[fbink_cfg->bg_color];

		ELOG("Pen colors set to #%02X%02X%02X for the foreground and #%02X%02X%02X for the background",
		     penFGColor,
		     penFGColor,
		     penFGColor,
		     penBGColor,
		     penBGColor,
		     penBGColor);
#ifdef FBINK_FOR_KINDLE
	}
#endif

	// Use the appropriate get/put pixel functions, and pack the pen colors into the appropriate pixel format...
	switch (vInfo.bits_per_pixel) {
		case 4U:
			//fxpPutPixel      = &put_pixel_Gray4;
			fxpGetPixel      = &get_pixel_Gray4;
			penFGPixel.gray8 = penFGColor;
			penBGPixel.gray8 = penBGColor;
			break;
		case 8U:
			//fxpPutPixel      = &put_pixel_Gray8;
			fxpGetPixel      = &get_pixel_Gray8;
			penFGPixel.gray8 = penFGColor;
			penBGPixel.gray8 = penBGColor;
			break;
		case 16U:
			//fxpPutPixel       = &put_pixel_RGB565;
			fxpGetPixel       = &get_pixel_RGB565;
			penFGPixel.rgb565 = pack_rgb565(penFGColor, penFGColor, penFGColor);
			penBGPixel.rgb565 = pack_rgb565(penBGColor, penBGColor, penBGColor);
			break;
		case 24U:
			//fxpPutPixel             = &put_pixel_RGB24;
			fxpGetPixel             = &get_pixel_RGB24;
			penFGPixel.bgra.color.r = penFGPixel.bgra.color.g = penFGPixel.bgra.color.b = penFGColor;
			penBGPixel.bgra.color.r = penBGPixel.bgra.color.g = penBGPixel.bgra.color.b = penBGColor;
			break;
		case 32U:
			//fxpPutPixel             = &put_pixel_RGB32;
			fxpGetPixel             = &get_pixel_RGB32;
			penFGPixel.bgra.color.a = 0xFFu;
			penFGPixel.bgra.color.r = penFGPixel.bgra.color.g = penFGPixel.bgra.color.b = penFGColor;
			penBGPixel.bgra.color.a                                                     = 0xFFu;
			penBGPixel.bgra.color.r = penBGPixel.bgra.color.g = penBGPixel.bgra.color.b = penBGColor;
			break;
		default:
			// Huh oh... Should never happen!
			WARN("Unsupported framebuffer bpp");
			rv = ERRCODE(EXIT_FAILURE);
			goto cleanup;
			break;
	}

	// NOTE: Do we want to keep the fb0 fd open, or simply close it for now?
	//       Useful because we probably want to close it to keep open fds to a minimum when used as a library,
	//       while wanting to avoid a useless open/close/open/close cycle when used as a standalone tool.
cleanup:
	if (!keep_fd) {
		close(fbfd);
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
static const char*
    font_style_to_string(uint8_t style)
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
#endif    // FBINK_WITH_OPENTYPE

// Load font from given file path. Up to four font styles may be used by FBInk at any given time.
int
    fbink_add_ot_font(const char* filename UNUSED_BY_MINIMAL, FONT_STYLE_T style UNUSED_BY_MINIMAL)
{
#ifdef FBINK_WITH_OPENTYPE
#	ifdef FBINK_FOR_KOBO
	// NOTE: Bail if we were passed a Kobo system font, as they're obfuscated,
	//       and some of them risk crashing stbtt because of bogus data...
	const char blacklist[] = "/usr/local/Trolltech/QtEmbedded-4.6.2-arm/lib/fonts/";
	if (!strncmp(filename, blacklist, sizeof(blacklist) - 1)) {
		WARN("Cannot use font '%s': it's an obfuscated Kobo system font", filename + sizeof(blacklist) - 1);
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
	FILE*                   f    = fopen(filename, "r" STDIO_CLOEXEC);
	unsigned char* restrict data = NULL;
	if (!f) {
		WARN("fopen: %m");
		otInit = false;
		return ERRCODE(EXIT_FAILURE);
	} else {
		int         fd = fileno(f);
		struct stat st;
		if (fstat(fd, &st) == -1) {
			WARN("fstat: %m");
			fclose(f);
			otInit = false;
			return ERRCODE(EXIT_FAILURE);
		}
		data = calloc((size_t) st.st_size, sizeof(*data));
		if (!data) {
			WARN("Error allocating font data buffer: %m");
			fclose(f);
			otInit = false;
			return ERRCODE(EXIT_FAILURE);
		}
		if (fread(data, 1U, (size_t) st.st_size, f) < (size_t) st.st_size || ferror(f) != 0) {
			free(data);
			fclose(f);
			otInit = false;
			WARN("Error reading font file");
			return ERRCODE(EXIT_FAILURE);
		}
		fclose(f);
	}
	stbtt_fontinfo* font_info = calloc(1U, sizeof(stbtt_fontinfo));
	if (!font_info) {
		WARN("Error allocating stbtt_fontinfo struct: %m");
		free(data);
		return ERRCODE(EXIT_FAILURE);
	}
	// First, check if we can actually find a recognizable font format in the data...
	int fontcount = stbtt_GetNumberOfFonts(data);
	if (fontcount == 0) {
		free(data);
		free(font_info);
		WARN("File '%s' doesn't appear to be a valid or supported font", filename);
		return ERRCODE(EXIT_FAILURE);
	} else if (fontcount > 1) {
		LOG("Font file '%s' appears to be a font collection containing %d fonts, but we'll only use the first one!",
		    filename,
		    fontcount);
	}
	// Then, get the offset to the first font
	int fontoffset = stbtt_GetFontOffsetForIndex(data, 0);
	if (fontoffset == -1) {
		free(data);
		free(font_info);
		WARN("File '%s' doesn't appear to contain valid font data at offset %d", filename, fontoffset);
		return ERRCODE(EXIT_FAILURE);
	}
	// And finally, initialize that font
	// NOTE: We took the long way 'round to try to avoid crashes on invalid data...
	if (!stbtt_InitFont(font_info, data, fontoffset)) {
		free(font_info->data);
		free(font_info);
		WARN("Error initialising font '%s'", filename);
		return ERRCODE(EXIT_FAILURE);
	}
	// Assign the current font to its appropriate otFonts struct member, depending on the style specified by the caller.
	// NOTE: We make sure we free any previous allocation first!
	switch (style) {
		case FNT_REGULAR:
			if (free_ot_font(&otFonts.otRegular) == EXIT_SUCCESS) {
				LOG("Replacing an existing Regular font style!");
			}
			otFonts.otRegular = font_info;
			break;
		case FNT_ITALIC:
			if (free_ot_font(&otFonts.otItalic) == EXIT_SUCCESS) {
				LOG("Replacing an existing Italic font style!");
			}
			otFonts.otItalic = font_info;
			break;
		case FNT_BOLD:
			if (free_ot_font(&otFonts.otBold) == EXIT_SUCCESS) {
				LOG("Replacing an existing Bold font style!");
			}
			otFonts.otBold = font_info;
			break;
		case FNT_BOLD_ITALIC:
			if (free_ot_font(&otFonts.otBoldItalic) == EXIT_SUCCESS) {
				LOG("Replacing an existing Bold Italic font style!");
			}
			otFonts.otBoldItalic = font_info;
			break;
	}

	ELOG("Font '%s' loaded for style '%s'", filename, font_style_to_string((uint8_t) style));
	return EXIT_SUCCESS;
#else
	WARN("OpenType support is disabled in this FBInk build");
	return ERRCODE(ENOSYS);
#endif    // FBINK_WITH_OPENTYPE
}

#ifdef FBINK_WITH_OPENTYPE
// Free an individual OpenType font structure
static int
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
#endif    // FBINK_WITH_OPENTYPE

// Free all OpenType fonts
int
    fbink_free_ot_fonts(void)
{
#ifdef FBINK_WITH_OPENTYPE
	if (free_ot_font(&otFonts.otRegular) == EXIT_SUCCESS) {
		LOG("Released Regular font data");
	}
	if (free_ot_font(&otFonts.otItalic) == EXIT_SUCCESS) {
		LOG("Released Italic font data");
	}
	if (free_ot_font(&otFonts.otBold) == EXIT_SUCCESS) {
		LOG("Released Bold font data");
	}
	if (free_ot_font(&otFonts.otBoldItalic) == EXIT_SUCCESS) {
		LOG("Released Bold Italic font data");
	}

	return EXIT_SUCCESS;
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
	    "viewWidth=%u;viewHeight=%u;screenWidth=%u;screenHeight=%u;viewHoriOrigin=%hhu;viewVertOrigin=%hhu;viewVertOffset=%hhu;DPI=%hu;BPP=%u;lineLength=%u;FONTW=%hu;FONTH=%hu;FONTSIZE_MULT=%hhu;FONTNAME='%s';glyphWidth=%hhu;glyphHeight=%hhu;MAXCOLS=%hu;MAXROWS=%hu;isPerfectFit=%d;FBID='%s';USER_HZ=%ld;penFGColor=%hhu;penBGColor=%hhu;deviceName='%s';deviceId=%hu;deviceCodename='%s';devicePlatform='%s';isKoboNonMT=%d;ntxBootRota=%hhu;ntxRotaQuirk=%hhu;isNTX16bLandscape=%d;currentRota=%u;canRotate=%d;",
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
	    deviceQuirks.isKoboNonMT,
	    deviceQuirks.ntxBootRota,
	    deviceQuirks.ntxRotaQuirk,
	    deviceQuirks.isNTX16bLandscape,
	    vInfo.rotate,
	    deviceQuirks.canRotate);
}

// Dump a few of our internal state variables to the FBInkState struct pointed to by fbink_state
void
    fbink_get_state(const FBInkConfig* restrict fbink_cfg, FBInkState* restrict fbink_state)
{
	if (fbink_state) {
		fbink_state->user_hz       = USER_HZ;
		fbink_state->font_name     = fontname_to_string(fbink_cfg->fontname);
		fbink_state->view_width    = viewWidth;
		fbink_state->view_height   = viewHeight;
		fbink_state->screen_width  = screenWidth;
		fbink_state->screen_height = screenHeight;
		fbink_state->bpp           = vInfo.bits_per_pixel;
		strncpy(fbink_state->device_name, deviceQuirks.deviceName, sizeof(fbink_state->device_name) - 1U);
		strncpy(
		    fbink_state->device_codename, deviceQuirks.deviceCodename, sizeof(fbink_state->device_codename) - 1U);
		strncpy(
		    fbink_state->device_platform, deviceQuirks.devicePlatform, sizeof(fbink_state->device_platform) - 1U);
		fbink_state->device_id               = deviceQuirks.deviceId;
		fbink_state->pen_fg_color            = penFGColor;
		fbink_state->pen_bg_color            = penBGColor;
		fbink_state->screen_dpi              = deviceQuirks.screenDPI;
		fbink_state->font_w                  = FONTW;
		fbink_state->font_h                  = FONTH;
		fbink_state->max_cols                = MAXCOLS;
		fbink_state->max_rows                = MAXROWS;
		fbink_state->view_hori_origin        = viewHoriOrigin;
		fbink_state->view_vert_origin        = viewVertOrigin;
		fbink_state->view_vert_offset        = viewVertOffset;
		fbink_state->fontsize_mult           = FONTSIZE_MULT;
		fbink_state->glyph_width             = glyphWidth;
		fbink_state->glyph_height            = glyphHeight;
		fbink_state->is_perfect_fit          = deviceQuirks.isPerfectFit;
		fbink_state->is_kobo_non_mt          = deviceQuirks.isKoboNonMT;
		fbink_state->ntx_boot_rota           = deviceQuirks.ntxBootRota;
		fbink_state->ntx_rota_quirk          = deviceQuirks.ntxRotaQuirk;
		fbink_state->is_ntx_quirky_landscape = deviceQuirks.isNTX16bLandscape;
		fbink_state->current_rota            = (uint8_t) vInfo.rotate;
		fbink_state->can_rotate              = deviceQuirks.canRotate;
	} else {
		WARN("Err, it appears we were passed a NULL fbink_state pointer?");
	}
}

// Memory map the framebuffer
static int
    memmap_fb(int fbfd)
{
	// NOTE: Beware of smem_len on Kobos?
	//       c.f., https://github.com/koreader/koreader-base/blob/master/ffi/framebuffer_linux.lua#L36
	//       TL;DR: On 16bpp fbs, it *might* be a bit larger than strictly necessary,
	//              but I've yet to see that be an issue with what I'm doing,
	//              and trusting it is much simpler than trying to outsmart broken fb setup info...
	//       See also the Cervantes quirk documented in clear_screen...
	fbPtr = (unsigned char*) mmap(NULL, fInfo.smem_len, PROT_READ | PROT_WRITE, MAP_SHARED, fbfd, 0);
	if (fbPtr == MAP_FAILED) {
		WARN("mmap: %m");
		fbPtr = NULL;
		return ERRCODE(EXIT_FAILURE);
	} else {
		isFbMapped = true;
	}

	return EXIT_SUCCESS;
}

// And unmap it
static int
    unmap_fb(void)
{
	if (munmap(fbPtr, fInfo.smem_len) < 0) {
		WARN("munmap: %m");
		return ERRCODE(EXIT_FAILURE);
	} else {
		// NOTE: Don't forget to reset those state flags,
		//       so we won't skip mmap'ing on the next call without a fb fd passed...
		isFbMapped = false;
		fbPtr      = NULL;
	}

	return EXIT_SUCCESS;
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
			WARN("close: %m");
			return ERRCODE(EXIT_FAILURE);
		}
	}

	return EXIT_SUCCESS;
}

// Much like rotate_coordinates, but for a mxcfb rectangle
// c.f., adjust_coordinates @ drivers/video/fbdev/mxc/mxc_epdc_v2_fb.c
#if defined(FBINK_FOR_KOBO) || defined(FBINK_FOR_CERVANTES)
static void
    rotate_region_pickel(struct mxcfb_rect* restrict region)
{
	// Rotate the region to account for pickel's rotation...
	struct mxcfb_rect oregion = *region;
	// NOTE: left = x, top = y
	region->top    = screenWidth - oregion.left - oregion.width;
	region->left   = oregion.top;
	region->width  = oregion.height;
	region->height = oregion.width;
}

static void
    rotate_region_boot(struct mxcfb_rect* restrict region)
{
	// Rotate the region to account for the native boot rotation...
	struct mxcfb_rect oregion = *region;
	// NOTE: left = x, top = y
	region->top    = oregion.left;
	region->left   = screenHeight - oregion.top - oregion.height;
	region->width  = oregion.height;
	region->height = oregion.width;
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
    fbink_cls(int fbfd, const FBInkConfig* restrict fbink_cfg, const FBInkRect* restrict rect)
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

	// We'll need a matching region for the refresh...
	struct mxcfb_rect region = { 0U };

	// Did we request a regional clear?
	if (!rect || (rect->width == 0U || rect->height == 0U)) {
		// Nope -> full-screen
		clear_screen(fbfd, fbink_cfg->is_inverted ? penBGColor ^ 0xFFu : penBGColor, fbink_cfg->is_flashing);
		fullscreen_region(&region);
	} else {
		// Yes -> simply fill a rectangle w/ the bg color
		FBInkPixel bgP = penBGPixel;
		if (fbink_cfg->is_inverted) {
			// NOTE: And, of course, RGB565 is terrible. Inverting the lossy packed value would be even lossier...
			if (vInfo.bits_per_pixel == 16U) {
				const uint8_t bgcolor = penBGColor ^ 0xFFu;
				bgP.rgb565            = pack_rgb565(bgcolor, bgcolor, bgcolor);
			} else {
				bgP.bgra.p ^= 0x00FFFFFFu;
			}
		}
		fill_rect(rect->left, rect->top, rect->width, rect->height, &bgP);
		// And update the region...
		region.top    = rect->top;
		region.left   = rect->left;
		region.width  = rect->width;
		region.height = rect->height;
	}

	// Refresh screen
	if (refresh(fbfd,
		    region,
		    get_wfm_mode(fbink_cfg->wfm_mode),
		    fbink_cfg->is_dithered ? EPDC_FLAG_USE_DITHERING_ORDERED : EPDC_FLAG_USE_DITHERING_PASSTHROUGH,
		    fbink_cfg->is_nightmode,
		    fbink_cfg->is_flashing,
		    fbink_cfg->no_refresh) != EXIT_SUCCESS) {
		WARN("Failed to refresh the screen");
		rv = ERRCODE(EXIT_FAILURE);
		goto cleanup;
	}

	// Cleanup
cleanup:
	if (isFbMapped && !keep_fd) {
		unmap_fb();
	}
	if (!keep_fd) {
		close(fbfd);
	}

	return rv;
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
    fbink_print(int fbfd, const char* restrict string, const FBInkConfig* fbink_cfg)
{
	// Abort if we were passed an empty string
	if (!*string) {
		// Unless we just want a clear, in which case, bypass everything and just do that.
		if (fbink_cfg->is_cleared) {
			return fbink_cls(fbfd, fbink_cfg, NULL);
		} else {
			WARN("Cannot print an empty string");
			return ERRCODE(EINVAL);
		}
	}

	// Abort if we were passed an invalid UTF-8 sequence
	size_t len       = strlen(string);    // Flawfinder: ignore
	size_t charcount = u8_strlen2(string);
	if (charcount == 0) {
		WARN("Cannot print an invalid UTF-8 sequence");
		return ERRCODE(EILSEQ);
	}

	// If we open a fd now, we'll only keep it open for this single print call!
	// NOTE: We *expect* to be initialized at this point, though, but that's on the caller's hands!
	bool keep_fd = true;
	if (open_fb_fd(&fbfd, &keep_fd) != EXIT_SUCCESS) {
		return ERRCODE(EXIT_FAILURE);
	}

	// Assume success, until shit happens ;)
	int rv = EXIT_SUCCESS;
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

	struct mxcfb_rect region = { 0U };
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
		WARN("line calloc: %m");
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

		// When centered & padded, we need to split the padding in two, left & right.
		if (fbink_cfg->is_centered && fbink_cfg->is_padded) {
			// We always want full padding
			col = 0;

			// Compute our padding length
			unsigned short int left_pad = (unsigned short int) (MAXCOLS - line_len) / 2U;
			// As for the right padding, we basically just have to print 'til the edge of the screen
			unsigned short int right_pad = (unsigned short int) (MAXCOLS - line_len - left_pad);

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
		} else if (fbink_cfg->is_padded) {
			// NOTE: Rely on the field width for padding ;).
			// Padding character is a space, which is 1 byte, so that's good enough ;).
			size_t padded_bytes = line_bytes + (size_t)(available_cols - line_len);
			// NOTE: Don't touch line_len, because we're *adding* new blank characters,
			//       we're still printing the exact same amount of characters *from our string*.
			LOG("Left padded %zu bytes to %zu to cover %hu columns",
			    line_bytes,
			    padded_bytes,
			    available_cols);
			bytes_printed = snprintf(
			    line, padded_bytes + 1U, "%*.*s", (int) padded_bytes, (int) line_bytes, string + line_offset);
		} else if (fbink_cfg->is_rpadded) {
			// NOTE: Rely on the field width for padding ;).
			// Padding character is a space, which is 1 byte, so that's good enough ;).
			size_t padded_bytes = line_bytes + (size_t)(available_cols - line_len);
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
		if (wrapped_line && line_len < available_cols) {
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

	// Rotate the region if need be...
	(*fxpRotateRegion)(&region);

	// Handle the last rect stuff...
	set_last_rect(&region);

	// Fudge the region if we asked for a screen clear, so that we actually refresh the full screen...
	if (fbink_cfg->is_cleared) {
		fullscreen_region(&region);
	}

	// Refresh screen
	if (refresh(fbfd,
		    region,
		    get_wfm_mode(fbink_cfg->wfm_mode),
		    fbink_cfg->is_dithered ? EPDC_FLAG_USE_DITHERING_ORDERED : EPDC_FLAG_USE_DITHERING_PASSTHROUGH,
		    fbink_cfg->is_nightmode,
		    fbink_cfg->is_flashing,
		    fbink_cfg->no_refresh) != EXIT_SUCCESS) {
		WARN("Failed to refresh the screen");
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
		close(fbfd);
	}

	return rv;
}

#ifdef FBINK_WITH_OPENTYPE
// An extremely rudimentry "markdown" parser. It would probably be wise to cook up something better at some point...
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
				if (ch == '_' && ci > 0 && string[ci - 1] != ' ' && string[ci + 1] != ' ') {
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
	int            ret    = -1;
	size_t         size   = 0;
	char* restrict buffer = NULL;
	va_list        args;

	// Initial vsnprintf run on a zero length NULL pointer, just to determine the required buffer size
	// NOTE: see vsnprintf(3), this is a C99 behavior made canon in POSIX.1-2001, honored since glibc 2.1
	va_start(args, fmt);
	ret = vsnprintf(buffer, size, fmt, args);
	va_end(args);

	// See if vsnprintf made a boo-boo
	if (ret < 0) {
		WARN("initial vsnprintf: %m");
		rv = ERRCODE(EXIT_FAILURE);
		goto cleanup;
	}

	// We need enough space for NULL-termination (which we make 'wide' for u8 reasons) :).
	size = (size_t)(ret + 4);
	// NOTE: We use calloc to make sure it'll always be zero-initialized,
	//       and the OS is smart enough to make it fast if we don't use the full space anyway (CoW zeroing).
	buffer = calloc(size, sizeof(*buffer));
	if (buffer == NULL) {
		WARN("calloc: %m");
		rv = ERRCODE(EXIT_FAILURE);
		goto cleanup;
	}

	// And now we can actually let vsnprintf do its job, for real ;).
	va_start(args, fmt);
	ret = vsnprintf(buffer, size, fmt, args);
	va_end(args);

	// See if vsnprintf made a boo-boo, one final time
	if (ret < 0) {
		WARN("vsnprintf: %m");
		rv = ERRCODE(EXIT_FAILURE);
		goto cleanup;
	}

	// Okay, now that we've got a formatted buffer...
	// Did we get a valid FBInkOTConfig pointer?
	if (cfg) {
#ifdef FBINK_WITH_OPENTYPE
		// Then feed our formatted string to fbink_print_ot
		rv = fbink_print_ot(fbfd, buffer, cfg, fbink_cfg, NULL);
#else
		WARN("OpenType support is disabled in this FBInk build");
		rv = ERRCODE(ENOSYS);
#endif
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
			return fbink_cls(fbfd, fbink_cfg, NULL);
		} else {
			WARN("Cannot print an empty string");
			return ERRCODE(EINVAL);
		}
	}

	// Abort if we were passed an invalid UTF-8 sequence
	size_t str_len_bytes = strlen(string);    // Flawfinder: ignore
	if (!u8_isvalid2(string)) {
		WARN("Cannot print an invalid UTF-8 sequence");
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
	FBInkOTLine* restrict   lines      = NULL;
	char* restrict          brk_buff   = NULL;
	unsigned char* restrict fmt_buff   = NULL;
	unsigned char*          line_buff  = NULL;
	unsigned char*          glyph_buff = NULL;
	// This also needs to be declared early, as we refresh on cleanup.
	struct mxcfb_rect region       = { 0U };
	bool              is_flashing  = false;
	bool              is_cleared   = false;
	uint8_t           wfm_mode     = WFM_AUTO;
	bool              is_dithered  = false;
	bool              is_nightmode = false;
	bool              no_refresh   = false;

	// map fb to user mem
	// NOTE: If we're keeping the fb's fd open, keep this mmap around, too.
	if (!isFbMapped) {
		if (memmap_fb(fbfd) != EXIT_SUCCESS) {
			rv = ERRCODE(EXIT_FAILURE);
			goto cleanup;
		}
	}

	// Make sure we return accurate data in case the fit struct is being recycled...
	if (fit) {
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
	area.tl.x = left_margin;
	area.tl.y = (unsigned short int) (top_margin + (viewVertOrigin - viewVertOffset));
	area.br.x = (unsigned short int) (viewWidth - right_margin);
	area.br.y = (unsigned short int) (viewHeight - bottom_margin);
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
		unsigned short int ppi = deviceQuirks.screenDPI;
		// Given the ppi, convert point height to pixels. Note, 1pt is 1/72th of an inch
		font_size_px = (unsigned short int) iroundf(ppi / 72.0f * size_pt);
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
	if (otFonts.otRegular) {
		rgSF = stbtt_ScaleForPixelHeight(otFonts.otRegular, (float) font_size_px);
		stbtt_GetFontVMetrics(otFonts.otRegular, &asc, &desc, &lg);
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
		// Set default font, for when markdown parsing is disabled
		if (!curr_font) {
			curr_font = otFonts.otRegular;
			sf        = rgSF;
			LOG("Unformatted text defaulting to Regular font style");
		}
	}
	if (otFonts.otItalic) {
		itSF = stbtt_ScaleForPixelHeight(otFonts.otItalic, (float) font_size_px);
		stbtt_GetFontVMetrics(otFonts.otItalic, &asc, &desc, &lg);
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
		// Set default font, for when markdown parsing is disabled
		if (!curr_font) {
			curr_font = otFonts.otItalic;
			sf        = itSF;
			LOG("Unformatted text defaulting to Italic font style");
		}
	}
	if (otFonts.otBold) {
		bdSF = stbtt_ScaleForPixelHeight(otFonts.otBold, (float) font_size_px);
		stbtt_GetFontVMetrics(otFonts.otBold, &asc, &desc, &lg);
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
		// Set default font, for when markdown parsing is disabled
		if (!curr_font) {
			curr_font = otFonts.otBold;
			sf        = bdSF;
			LOG("Unformatted text defaulting to Bold font style");
		}
	}
	if (otFonts.otBoldItalic) {
		bditSF = stbtt_ScaleForPixelHeight(otFonts.otBoldItalic, (float) font_size_px);
		stbtt_GetFontVMetrics(otFonts.otBoldItalic, &asc, &desc, &lg);
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
		// Set default font, for when markdown parsing is disabled
		if (!curr_font) {
			curr_font = otFonts.otBoldItalic;
			sf        = bditSF;
			LOG("Unformatted text defaulting to Bold Italic font style");
		}
	}
	// If no font was loaded, exit early. We checked earlier, but just in case...
	if (!curr_font) {
		WARN("No font appears to be loaded");
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
	unsigned int print_height, num_lines;
	print_height = (unsigned int) (area.br.y - area.tl.y + (viewVertOrigin - viewVertOffset));
	num_lines    = print_height / (unsigned int) max_row_height;

	// And allocate the memory for it...
	lines = calloc(num_lines, sizeof(FBInkOTLine));
	if (!lines) {
		WARN("Lines metadata buffer could not be allocated: %m");
		rv = ERRCODE(EXIT_FAILURE);
		goto cleanup;
	}

	// Now, lets use libunibreak to find the possible break opportunities in our string.

	// Note: we only care about the byte length here
	brk_buff = calloc(str_len_bytes + 1U, sizeof(*brk_buff));
	if (!brk_buff) {
		WARN("Linebreak buffer could not be allocated: %m");
		rv = ERRCODE(EXIT_FAILURE);
		goto cleanup;
	}

	set_linebreaks_utf8((const utf8_t*) string, str_len_bytes + 1U, "en", brk_buff);
	LOG("Finished looking for linebreaks");

	// Parse our string for formatting, if requested
	if (cfg->is_formatted) {
		fmt_buff = calloc(str_len_bytes + 1U, sizeof(*fmt_buff));
		if (!fmt_buff) {
			WARN("Formatted text buffer could not be allocated: %m");
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
	int          adv, lsb, curr_x;
	bool         complete_str = false;
	int          x0, y0, x1, y1, gw, gh, cx, cy;
	unsigned int lw = 0U;
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
							curr_font = otFonts.otItalic;
							sf        = itSF;
							break;
						case CH_BOLD:
							curr_font = otFonts.otBold;
							sf        = bdSF;
							break;
						case CH_BOLD_ITALIC:
							curr_font = otFonts.otBoldItalic;
							sf        = bditSF;
							break;
						case CH_REGULAR:
						default:
							curr_font = otFonts.otRegular;
							sf        = rgSF;
							break;
					}
				}
			}
			if (!curr_font) {
				WARN("The specified font style was not loaded");
				rv = ERRCODE(ENOENT);
				goto cleanup;
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
			c = u8_nextchar2(string, &c_index);
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
			// Adjust our x position for kerning, because we can :)
			if (string[c_index + 1]) {
				tmp_c_index   = c_index;
				uint32_t c2   = u8_nextchar2(string, &tmp_c_index);
				int      g2i  = stbtt_FindGlyphIndex(curr_font, (int) c2);
				int      xadv = stbtt_GetGlyphKernAdvance(curr_font, gi, g2i);
				curr_x += iroundf(sf * (float) xadv);
			}
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
	unsigned int computed_lines_amount = line + complete_str;
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
	if (fit) {
		fit->computed_lines = (unsigned short int) computed_lines_amount;
		fit->truncated      = !complete_str;
	}

	// Abort early if we detected a truncation and the user flagged that as a failure.
	if (cfg->no_truncation && !complete_str) {
		LOG("Requested early abort on truncation!");
		rv = ERRCODE(ENOSPC);
		goto cleanup;
	}

	// If we only asked for a computation pass, abort now (successfully).
	if (cfg->compute_only) {
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
		valign       = fbink_cfg->valign;
		halign       = fbink_cfg->halign;
		is_inverted  = fbink_cfg->is_inverted;
		is_overlay   = fbink_cfg->is_overlay;
		is_bgless    = fbink_cfg->is_bgless;
		is_fgless    = fbink_cfg->is_fgless;
		is_flashing  = fbink_cfg->is_flashing;
		is_cleared   = fbink_cfg->is_cleared;
		is_centered  = fbink_cfg->is_centered;
		is_halfway   = fbink_cfg->is_halfway;
		wfm_mode     = fbink_cfg->wfm_mode;
		is_dithered  = fbink_cfg->is_dithered;
		is_nightmode = fbink_cfg->is_nightmode;
		no_refresh   = fbink_cfg->no_refresh;
	} else {
		is_centered = cfg->is_centered;
	}

	// Hopefully, we have some lines to render!

	// Create a bitmap buffer to render a single line.
	// We don't render the glyphs directly to the fb here, as we need to do some simple blending,
	// and it makes it easier to calculate our centering if required.
	line_buff = calloc(max_lw * (size_t) max_line_height, sizeof(*line_buff));
	// We also don't want to be creating a new buffer for every glyph, so make it roomy, just in case...
	size_t glyph_buffer_dims = font_size_px * (size_t) max_line_height * 2U;
	glyph_buff               = calloc(glyph_buffer_dims, sizeof(*glyph_buff));
	if (!line_buff || !glyph_buff) {
		WARN("Line or glyph buffers could not be allocated: %m");
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
		region.left = area.tl.x + ((uint32_t)(area.br.x - area.tl.x) / 2U);
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
			fill_rect((unsigned short int) region.left,
				  (unsigned short int) region.top,
				  max_lw,
				  (unsigned short int) print_height,
				  &bgP);
		}
	}

	uint32_t                tmp_c;
	int                     tmp_gi;
	unsigned char* restrict lnPtr   = NULL;
	unsigned char* restrict glPtr   = NULL;
	unsigned short int      start_x = area.tl.x;

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
							curr_font = otFonts.otRegular;
							sf        = rgSF;
							break;
						case CH_ITALIC:
							curr_font = otFonts.otItalic;
							sf        = itSF;
							break;
						case CH_BOLD:
							curr_font = otFonts.otBold;
							sf        = bdSF;
							break;
						case CH_BOLD_ITALIC:
							curr_font = otFonts.otBoldItalic;
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
				unsigned char* tmp_g_buff = NULL;
				tmp_g_buff                = realloc(glyph_buff, new_buff_size);
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
				//       f.g., with my trusty Bookerly, at whatever size it needs to be to break (here, 36),
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
						//         already set pixel, (f.g., with 'fl' in serif fonts,
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
			if (ci < lines[line].endCharIndex) {
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
				// Left padding (left edge of the drawing area to initial pen position)
				fill_rect((unsigned short int) region.left,
					  paint_point.y,
					  (unsigned short int) (paint_point.x - region.left),
					  (unsigned short int) curr_print_height,
					  &bgP);
				// Right padding (final pen position to the right edge of the drawing area)
				fill_rect((unsigned short int) (paint_point.x + lw),
					  paint_point.y,
					  (unsigned short int) (viewWidth - (paint_point.x + lw)),
					  (unsigned short int) curr_print_height,
					  &bgP);
			}
		} else if (cfg->padding == VERT_PADDING) {
			region.top    = area.tl.y;
			region.height = print_height;
			if (!is_overlay && !is_bgless) {
				// First line? Top padding (top edge of the drawing area to initial pen position)
				if (line == 0U) {
					fill_rect(paint_point.x,
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
				uint16_t pmul_bg = (uint16_t)(bgcolor * 0xFFu);
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
			FBInkPixel fb_px   = { 0U };
			uint16_t   pmul_bg = (uint16_t)(bgcolor * 0xFFu);
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
			fill_rect(start_x,
				  paint_point.y,
				  (unsigned short int) lw,
				  (unsigned short int) ((viewHeight + (uint32_t)(viewVertOrigin - viewVertOffset)) -
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
	if (fit) {
		fit->rendered_lines = (unsigned short int) line;
	}

	// Warn if there was an unforeseen truncation (potentially because of broken metrics)...
	if (line < computed_lines_amount) {
		LOG("Rendering took more space than expected, string was truncated to %u lines instead of %u!",
		    line,
		    computed_lines_amount);

		// Remember that in fit if it's a valid pointer...
		if (fit) {
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
		(*fxpRotateRegion)(&region);
		// Handle the last rect stuff...
		set_last_rect(&region);
		// NOTE: If we asked for a clear screen, fudge the region at the last moment,
		// so we don't get mangled by previous adjustments...
		if (is_cleared) {
			fullscreen_region(&region);
		}
		refresh(fbfd,
			region,
			get_wfm_mode(wfm_mode),
			is_dithered ? EPDC_FLAG_USE_DITHERING_ORDERED : EPDC_FLAG_USE_DITHERING_PASSTHROUGH,
			is_nightmode,
			is_flashing,
			no_refresh);
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
		close(fbfd);
	}
	return rv;
#else
	WARN("OpenType support is disabled in this FBInk build");
	return ERRCODE(ENOSYS);
#endif    // FBINK_WITH_OPENTYPE
}

// Convert our public WFM_MODE_INDEX_T values to an appropriate mxcfb waveform mode constant for the current device
static uint32_t
    get_wfm_mode(uint8_t wfm_mode_index)
{
	uint32_t waveform_mode = WAVEFORM_MODE_AUTO;

	// Parse waveform mode...
#if defined(FBINK_FOR_KINDLE)
	// Is this a Zelda or a Rex with new waveforms?
	bool has_new_wfm = false;
	if (deviceQuirks.isKindleZelda || deviceQuirks.isKindleRex) {
		has_new_wfm = true;
	}

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
			waveform_mode = WAVEFORM_MODE_ZELDA_GCK16;
			break;
		case WFM_GLKW16:
			waveform_mode = WAVEFORM_MODE_ZELDA_GLKW16;
			break;
		default:
			LOG("Unknown (or unsupported) waveform mode '%s' @ index %hhu, defaulting to AUTO",
			    wfm_to_string(wfm_mode_index),
			    wfm_mode_index);
			waveform_mode = WAVEFORM_MODE_AUTO;
			break;
	}
#elif defined(FBINK_FOR_REMARKABLE)
	// NOTE: Let's go with a dedicated switch for the reMarkable,
	//       because we don't actually have sane constant names in the upstream kernel,
	//       so most of these are guesswork based on libremarkable's findings.
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
		case WFM_GC16_FAST:
			waveform_mode = WAVEFORM_MODE_GC16_FAST;
			break;
		case WFM_A2:
			waveform_mode = WAVEFORM_MODE_A2;
			break;
		case WFM_GL16:
			waveform_mode = WAVEFORM_MODE_GL16;
			break;
		case WFM_GL16_FAST:
			waveform_mode = WAVEFORM_MODE_GL16_FAST;
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
		case WFM_GL4:
			waveform_mode = WAVEFORM_MODE_GL4;
			break;
		case WFM_GL16_INV:
			waveform_mode = WAVEFORM_MODE_GL16_INV;
			break;
		default:
			LOG("Unknown (or unsupported) waveform mode '%s' @ index %hhu, defaulting to AUTO",
			    wfm_to_string(wfm_mode_index),
			    wfm_mode_index);
			waveform_mode = WAVEFORM_MODE_AUTO;
			break;
	}
#else
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
#endif    // FBINK_FOR_KINDLE

	return waveform_mode;
}

// Convert a WFM_MODE_INDEX_T value to a human readable string
static const char*
    wfm_to_string(uint8_t wfm_mode_index)
{
	switch (wfm_mode_index) {
		case WFM_GC16:
			return "GC16";
		case WFM_DU:
			return "DU";
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
		case WFM_AUTO:
			return "AUTO";
		case WFM_INIT:
			return "INIT";
		default:
			return "Unknown";
	}
}

#ifndef FBINK_FOR_LINUX
// Convert our public HW_DITHER_INDEX_T values to an appropriate mxcfb dithering mode constant
static int
    get_hwd_mode(uint8_t hw_dither_index)
{
	// NOTE: This hardware dithering (handled by the PxP) is only supported since EPDC v2!
	//       AFAICT, most of our eligible target devices only support PASSTHROUGH & ORDERED...
	//       (c.f., drivers/dma/pxp/pxp_dma_v3.c)
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
static const char*
    hwd_to_string(uint8_t hw_dither_index)
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
		default:
			return "Unknown";
	}
}
#endif    // !FBINK_FOR_LINUX

// Small public wrapper around refresh(), without the caller having to depend on mxcfb headers
int
    fbink_refresh(int fbfd                              UNUSED_BY_LINUX,
		  uint32_t region_top                   UNUSED_BY_LINUX,
		  uint32_t region_left                  UNUSED_BY_LINUX,
		  uint32_t region_width                 UNUSED_BY_LINUX,
		  uint32_t region_height                UNUSED_BY_LINUX,
		  uint8_t dithering_mode                UNUSED_BY_LINUX,
		  const FBInkConfig* restrict fbink_cfg UNUSED_BY_LINUX)
{
#ifndef FBINK_FOR_LINUX
	// Open the framebuffer if need be (nonblock, we'll only do ioctls)...
	bool keep_fd = true;
	if (open_fb_fd_nonblock(&fbfd, &keep_fd) != EXIT_SUCCESS) {
		return ERRCODE(EXIT_FAILURE);
	}

	// Same for the dithering mode, if we actually requested dithering...
	int region_dither = EPDC_FLAG_USE_DITHERING_PASSTHROUGH;
	if (dithering_mode > 0U) {
		region_dither = get_hwd_mode(dithering_mode);
	} else {
		LOG("No hardware dithering requested");
	}
	// NOTE: Right now, we enforce quant_bit to what appears to be sane values depending on the waveform mode.

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

	int ret;
	if ((ret = refresh(fbfd,
			   region,
			   get_wfm_mode(fbink_cfg->wfm_mode),
			   region_dither,
			   fbink_cfg->is_nightmode,
			   fbink_cfg->is_flashing,
			   false)) != EXIT_SUCCESS) {
		WARN("Failed to refresh the screen");
	}

	if (!keep_fd) {
		close(fbfd);
	}

	return ret;
#else
	WARN("e-Ink screen refreshes require an e-Ink device");
	return ERRCODE(ENOSYS);
#endif    // !FBINK_FOR_LINUX
}

// Small public wrapper around wait_for_submission(), without the caller having to depend on mxcfb headers
int
    fbink_wait_for_submission(int fbfd UNUSED_BY_NOTKINDLE, uint32_t marker UNUSED_BY_NOTKINDLE)
{
#ifdef FBINK_FOR_KINDLE
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
			WARN("No previous update found, cannot wait for an invalid update marker");
			rv = ERRCODE(EINVAL);
			goto cleanup;
		}
	}

	if ((rv = wait_for_submission(fbfd, marker)) != EXIT_SUCCESS) {
		WARN("Failed to wait for submission of update %u", marker);
	}

cleanup:
	if (!keep_fd) {
		close(fbfd);
	}

	return rv;
#else
	WARN("Waiting for update submission is only supported on Kindle");
	return ERRCODE(ENOSYS);
#endif    // FBINK_FOR_KINDLE
}

// Small public wrapper around wait_for_complete(), without the caller having to depend on mxcfb headers
int
    fbink_wait_for_complete(int fbfd UNUSED_BY_LINUX, uint32_t marker UNUSED_BY_LINUX)
{
#ifndef FBINK_FOR_LINUX
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
			WARN("No previous update found, cannot wait for an invalid update marker");
			rv = ERRCODE(EINVAL);
			goto cleanup;
		}
	}

	if ((rv = wait_for_complete(fbfd, marker)) != EXIT_SUCCESS) {
		WARN("Failed to wait for completion of update %u", marker);
	}

cleanup:
	if (!keep_fd) {
		close(fbfd);
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

// Reinitialize FBInk in case the framebuffer state has changed
// NOTE: We initially (< 1.10.4) tried to limit this to specific scenarios, specifically:
//         * the bitdepth switch during boot between pickel & nickel
//         * eventual rotations during the boot between boot & pickel and/or nickel
//           (c.f., https://www.mobileread.com/forums/showpost.php?p=3764776&postcount=229 for those two points).
//         * rotation in general on devices with an accelerometer
//       However, while that accounts for *stock* behavior, that left some custom setups in the lurch, in particular:
//         * rotation in Nickel may be patched in
//         * custom software might affect bitdepth or rotation on purpose
//           (f.g., Plato relies on HW rotation, and KOReader may change the bitdepth).
//       TL;DR: We now monitor *any* change in bitdepth and/or rotation.
int
    fbink_reinit(int fbfd UNUSED_BY_KINDLE, const FBInkConfig* restrict fbink_cfg UNUSED_BY_KINDLE)
{
#ifndef FBINK_FOR_KINDLE
	// So, we're concerned with stuff that affects the logical & physical layout, namely, bitdepth & rotation.
	uint32_t old_bpp  = vInfo.bits_per_pixel;
	uint32_t old_rota = vInfo.rotate;

	// Open the framebuffer if need be (nonblock, we'll only do ioctls)...
	bool keep_fd = true;
	if (open_fb_fd_nonblock(&fbfd, &keep_fd) != EXIT_SUCCESS) {
		return ERRCODE(EXIT_FAILURE);
	}

	// Assume success, until shit happens ;)
	int rv = EXIT_SUCCESS;

	// Now that we've stored the relevant bits of the previous state, query the current one...
	if (ioctl(fbfd, FBIOGET_VSCREENINFO, &vInfo)) {
		WARN("Error reading variable fb information: %m");
		rv = ERRCODE(EXIT_FAILURE);
		goto cleanup;
	}

	// Start with the more drastic change: bitdepth, before checking rotation
	if (old_bpp != vInfo.bits_per_pixel) {
		// It's a reinit, so ask to skip the vinfo ioctl we just did
		ELOG("Detected a change in framebuffer bitdepth, reinitializing...");
		rv = initialize_fbink(fbfd, fbink_cfg, true);
	} else if (old_rota != vInfo.rotate) {
		// It's a reinit, so ask to skip the vinfo ioctl we just did
		ELOG("Detected a change in framebuffer rotation, reinitializing...");
		rv = initialize_fbink(fbfd, fbink_cfg, true);
	}

	// And we're done!
	// We keep silent when we do nothing, to avoid flooding logs (mainly, KFMon's ;)).

cleanup:
	if (!keep_fd) {
		close(fbfd);
	}

	return rv;
#else
	// NOTE: I currently haven't found the need for any shenanigans like that on Kindle, so, make this a NOP there ;).
	// NOTE: That said, we technically could make use of the canRotate quirk if we had an API user on Kindle that cared...
	WARN("Reinitilization is not needed on Kindle devices :)");
	return ERRCODE(ENOSYS);
#endif    // !FBINK_FOR_KINDLE
}

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
		if (vInfo.bits_per_pixel == 16U) {
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
	unsigned short int top_pos =
	    (unsigned short int) MAX(0 + (viewVertOrigin - viewVertOffset), ((row * FONTH) + voffset + viewVertOrigin));
	unsigned short int left_pos = 0U + viewHoriOrigin;

	// ... unless we were asked to skip background pixels... ;).
	if (!fbink_cfg->is_bgless) {
		fill_rect(left_pos, top_pos, (unsigned short int) screenWidth, FONTH, &bgP);
	}

	// NOTE: We always use the same BG_ constant in order to get a rough inverse by just swapping to the inverted LUT ;).
	uint8_t emptyC;
	uint8_t borderC;
	// Handle devices with an inverted palette properly...
	if (deviceQuirks.isKindleLegacy) {
		emptyC  = fbink_cfg->is_inverted ? eInkBGCMap[BG_GRAYB] : eInkFGCMap[BG_GRAYB];
		borderC = fbink_cfg->is_inverted ? eInkBGCMap[BG_GRAY4] : eInkFGCMap[BG_GRAY4];
	} else {
		if (fbink_cfg->wfm_mode == WFM_A2) {
			// NOTE: If we're using A2 refresh mode, we'll be enforcing monochrome anyway...
			//       Making sure we do that on our end (... at least with default bg/fg colors anyway ;),
			//       avoids weird behavior on devices where A2 can otherwise be quirky, like Kobo Mk. 7
			emptyC  = bgcolor;
			borderC = fgcolor;
		} else {
			emptyC  = fbink_cfg->is_inverted ? eInkFGCMap[BG_GRAYB] : eInkBGCMap[BG_GRAYB];
			borderC = fbink_cfg->is_inverted ? eInkFGCMap[BG_GRAY4] : eInkBGCMap[BG_GRAY4];
		}
	}
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
		unsigned short int bar_width = (unsigned short int) (((0.90f * (float) viewWidth)) + 0.5f);
		unsigned short int fill_width =
		    (unsigned short int) (((value / 100.0f) * (0.90f * (float) viewWidth)) + 0.5f);
		unsigned short int fill_left   = (unsigned short int) (left_pos + (0.05f * (float) viewWidth) + 0.5f);
		unsigned short int empty_width = (unsigned short int) (bar_width - fill_width);
		unsigned short int empty_left  = (unsigned short int) (fill_left + fill_width);

		// Draw the border...
		fill_rect(fill_left, top_pos, bar_width, FONTH, &borderP);
		// Draw the fill bar, which we want to override the border with!
		fill_rect(fill_left, top_pos, fill_width, FONTH, &fgP);
		// And the empty bar...
		// NOTE: With a minor tweak to keep a double-width border on the bottom & right sides ;).
		if (value == 0U) {
			// Keep the left border alone!
			fill_rect((unsigned short int) (empty_left + 1U),
				  (unsigned short int) (top_pos + 1U),
				  (unsigned short int) MAX(0, empty_width - 3),
				  (unsigned short int) (FONTH - 3U),
				  &emptyP);
		} else {
			fill_rect(empty_left,
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
		size_t line_len = strlen(percentage_text);    // Flawfinder: ignore

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
		// This is especially important w/ A2 wfm mode, as it *will* quantize the existing pixels down to B&W!
		if (fbink_cfg->is_bgless) {
			region.left  = fill_left;
			region.width = bar_width;
		}
	} else {
		// This is an infinite progress bar (a.k.a., activity bar)!

		// We'll want 5% of padding on each side,
		// with rounding to make sure the bar's size is constant across all percentage values...
		unsigned short int bar_width = (unsigned short int) ((0.90f * (float) viewWidth) + 0.5f);
		unsigned short int bar_left  = (unsigned short int) (left_pos + (0.05f * (float) viewWidth) + 0.5f);

		// Draw the border...
		fill_rect(bar_left, top_pos, (unsigned short int) (bar_width), FONTH, &borderP);
		// Draw the empty bar...
		fill_rect((unsigned short int) (bar_left + 1U),
			  (unsigned short int) (top_pos + 1U),
			  (unsigned short int) MAX(0, bar_width - 3),
			  (unsigned short int) (FONTH - 3U),
			  &emptyP);

		// We want our thumb to take 20% of the bar's width
		unsigned short int thumb_width = (unsigned short int) ((0.20f * bar_width) + 0.5f);
		// We move the thumb in increment of 5% of the bar's width (i.e., half its width),
		// with rounding to avoid accumulating drift...
		unsigned short int thumb_left = (unsigned short int) (bar_left + ((0.05f * bar_width) * value) + 0.5f);

		// And finally, draw the thumb, which we want to override the border with!
		fill_rect(thumb_left, top_pos, thumb_width, FONTH, &fgP);

		// Draw an ellipsis in the middle of the thumb...
		uint8_t ellipsis_size = (uint8_t)(FONTH / 3U);
		// Three dots = two spaces, 3 + 2 = 5 ;).
		unsigned short int ellipsis_left = (unsigned short int) ((thumb_width - (5U * ellipsis_size)) / 2U);
		for (uint8_t i = 0U; i < 3U; i++) {
			fill_rect((unsigned short int) (thumb_left + ellipsis_left +
							(unsigned short int) (i * 2U * ellipsis_size)),
				  (unsigned short int) (top_pos + ellipsis_size),
				  ellipsis_size,
				  ellipsis_size,
				  &bgP);
		}

		// Don't refresh beyond the borders of the bar if we're backgroundless...
		// This is especially important w/ A2 wfm mode, as it *will* quantize the existing pixels down to B&W!
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

	// Rotate the region if need be...
	(*fxpRotateRegion)(&region);

	// Handle the last rect stuff...
	set_last_rect(&region);

	// Fudge the region if we asked for a screen clear, so that we actually refresh the full screen...
	if (fbink_cfg->is_cleared) {
		fullscreen_region(&region);
	}

	// And finally, refresh the screen
	// NOTE: FWIW, using A2 basically ends up drawing the border black, and the empty white, which kinda works...
	//       It has the added benefit of increasing the framerate limit after which the eInk controller risks getting
	//       confused (unless is_flashing is enabled, since that'll block,
	//       essentially throttling the bar to the screen's refresh rate).
	if (refresh(fbfd,
		    region,
		    get_wfm_mode(fbink_cfg->wfm_mode),
		    fbink_cfg->is_dithered ? EPDC_FLAG_USE_DITHERING_ORDERED : EPDC_FLAG_USE_DITHERING_PASSTHROUGH,
		    fbink_cfg->is_nightmode,
		    fbink_cfg->is_flashing,
		    fbink_cfg->no_refresh) != EXIT_SUCCESS) {
		WARN("Failed to refresh the screen");
		return ERRCODE(EXIT_FAILURE);
	}

	return EXIT_SUCCESS;
}

// Draw a full-width progress bar
int
    fbink_print_progress_bar(int fbfd, uint8_t percentage, const FBInkConfig* restrict caller_fbink_cfg)
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

	// Begin by sanitizing the input...
	if (percentage > 100U) {
		LOG("The specified percentage (%hhu) is larger than 100, clamping it.", percentage);
		percentage = 100U;
	}

	// We need a local copy of the config struct, because we have a few things to enforce on our end,
	// and we don't want to mess with the caller's setup (plus, it's const for that very reason anyway).
	FBInkConfig fbink_cfg = *caller_fbink_cfg;
	// Namely, we need overlay mode to properly print the percentage text,
	fbink_cfg.is_overlay = true;
	// and no hoffset, because it makes no sense for a full-width bar,
	// and we don't want the text to be affected by a stray value...
	fbink_cfg.hoffset = 0;
	// And we enforce centered text internally, so we'll set col ourselves later...

	// And do the work ;).
	rv = draw_progress_bars(fbfd, false, percentage, &fbink_cfg);

	// Cleanup
cleanup:
	if (isFbMapped && !keep_fd) {
		unmap_fb();
	}
	if (!keep_fd) {
		close(fbfd);
	}

	return rv;
}

// Draw a full-width activity bar
int
    fbink_print_activity_bar(int fbfd, uint8_t progress, const FBInkConfig* restrict caller_fbink_cfg)
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
		close(fbfd);
	}

	return rv;
}

#ifdef FBINK_WITH_IMAGE
// Load & decode image data from a file or stdin, via stbi
static unsigned char*
    img_load_from_file(const char* filename, int* w, int* h, int* n, int req_n)
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
					WARN("realloc: %m");
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
			WARN("realloc: %m");
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
		WARN("Failed to open or decode image '%s'", filename);
		return NULL;
	}

	LOG("Requested %d color channels, image had %d", req_n, *n);

	return data;
}

// Convert raw image data between various pixel formats
// NOTE: This is a direct copy of stbi's stbi__convert_format, except that it doesn't free the input buffer.
static unsigned char*
    img_convert_px_format(const unsigned char* data, int img_n, int req_comp, int x, int y)
{
	unsigned char* good = NULL;

	// NOTE: We're already doing that in fbink_print_raw_data ;)
	//if (req_comp == img_n) return data;
	STBI_ASSERT(req_comp >= 1 && req_comp <= 4);

	good = (unsigned char*) stbi__malloc_mad3(req_comp, x, y, 0);
	if (good == NULL) {
		//STBI_FREE(data);
		return NULL;
	}

	// NOTE: Using restricted pointers is enough to make vectorizers happy, no need for ivdep pragmas ;).
	for (int j = 0; j < y; ++j) {
		const unsigned char* restrict src  = data + (j * x * img_n);
		unsigned char* restrict       dest = good + (j * x * req_comp);

#	define STBI__COMBO(a, b) ((a) *8 + (b))
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
static uint8_t
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
	uint32_t t = DIV255(v * ((15U << 6U) + 1U));
	// level = t / (D-1);
	uint32_t l = (t >> 6U);
	// t -= l * (D-1);
	t = (t - (l << 6U));

	// map width & height = 8
	// c = ClampToQuantum((l+(t >= map[(x % mw) + mw * (y % mh)])) * QuantumRange / (L-1));
	uint32_t q = ((l + (t >= threshold_map_o8x8[(x & 7U) + 8U * (y & 7U)])) * 17U);
	// NOTE: We're doing unsigned maths, so, clamping is basically MIN(q, UINT8_MAX) ;).
	//       The only overflow we should ever catch should be for a few white (v = 0xFF) input pixels
	//       that get shifted to the next step (i.e., q = 272 (0xFF + 17)).
	return (q > UINT8_MAX ? UINT8_MAX : (uint8_t) q);
}

// Draw image data on screen (we inherit a few of the variable types/names from stbi ;))
static int
    draw_image(int                           fbfd,
	       const unsigned char* restrict data,
	       const int                     w,
	       const int                     h,
	       const int                     n,
	       const int                     req_n,
	       short int                     x_off,
	       short int                     y_off,
	       const FBInkConfig* restrict   fbink_cfg)
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
	unsigned short int img_x_off = 0;
	unsigned short int img_y_off = 0;
	if (x_off < 0) {
		// We'll start plotting from the beginning of the *visible* part of the image ;)
		img_x_off = (unsigned short int) (abs(x_off) + viewHoriOrigin);
		max_width = (unsigned short int) (max_width + img_x_off);
		// Make sure we're not trying to loop past the actual width of the image!
		max_width = (unsigned short int) MIN(w, max_width);
		// Only if the visible section of the image's width is smaller than our screen's width...
		if ((uint32_t)(w - img_x_off) < viewWidth) {
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
		if ((uint32_t)(h - img_y_off) < viewHeight) {
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
	if (fb_is_grayscale) {
		// 4bpp & 8bpp
		if (!fbink_cfg->ignore_alpha && img_has_alpha) {
			if (!fb_is_legacy) {
				// 8bpp
				// There's an alpha channel in the image, we'll have to do alpha blending...
				// c.f., https://en.wikipedia.org/wiki/Alpha_compositing
				//       https://blogs.msdn.microsoft.com/shawnhar/2009/11/06/premultiplied-alpha/
				FBInkCoordinates coords;
				FBInkPixelG8A    img_px;
				for (unsigned short int j = img_y_off; j < max_height; j++) {
					for (unsigned short int i = img_x_off; i < max_width; i++) {
						// NOTE: In this branch, req_n == 2, so we can do << 1 instead of * 2 ;).
						size_t pix_offset = (size_t)(((j << 1U) * w) + (i << 1U));
#	pragma GCC diagnostic push
#	pragma GCC diagnostic ignored "-Wcast-align"
						// First, we gobble the full image pixel (all 2 bytes)
						// cppcheck-suppress unreadVariable ; false-positive (union)
						img_px.p = *((const uint16_t*) &data[pix_offset]);
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
							FBInkPixel bg_px;
							get_pixel_Gray8(&coords, &bg_px);

							uint8_t ainv = img_px.color.a ^ 0xFFu;
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
						size_t        pix_offset = (size_t)(((j << 1U) * w) + (i << 1U));
						FBInkPixelG8A img_px;
#	pragma GCC diagnostic push
#	pragma GCC diagnostic ignored "-Wcast-align"
						// We gobble the full image pixel (all 2 bytes)
						img_px.p = *((const uint16_t*) &data[pix_offset]);
#	pragma GCC diagnostic pop

						uint8_t ainv = img_px.color.a ^ 0xFFu;
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
					size_t pix_offset = (size_t)((j * w) + img_x_off);
					size_t fb_offset  = ((uint32_t)(j + y_off) * fInfo.line_length) +
							   (unsigned int) (img_x_off + x_off);
					memcpy(fbPtr + fb_offset, data + pix_offset, max_width);
				}
			} else {
				for (unsigned short int j = img_y_off; j < max_height; j++) {
					for (unsigned short int i = img_x_off; i < max_width; i++) {
						// NOTE: Here, req_n is either 2, or 1 if ignore_alpha, so, no shift trickery ;)
						size_t pix_offset = (size_t)((j * req_n * w) + (i * req_n));
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
							put_pixel_Gray8(&coords, &pixel);
						} else {
							put_pixel_Gray4(&coords, &pixel);
						}
					}
				}
			}
		}
	} else if (fb_is_true_bgr) {
		// 24bpp & 32bpp
		if (!fbink_cfg->ignore_alpha && img_has_alpha) {
			FBInkPixelRGBA img_px;
			size_t         pix_offset;
			if (!fb_is_24bpp) {
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
						pix_offset = (size_t)(((j << 2U) * w) + (i << 2U));
#	pragma GCC diagnostic push
#	pragma GCC diagnostic ignored "-Wcast-align"
						// First, we gobble the full image pixel (all 4 bytes)
						// cppcheck-suppress unreadVariable ; false-positive (union)
						img_px.p = *((const uint32_t*) &data[pix_offset]);
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

							pix_offset =
							    (uint32_t)((unsigned short int) (i + x_off) << 2U) +
							    ((unsigned short int) (j + y_off) * fInfo.line_length);
#	pragma GCC diagnostic push
#	pragma GCC diagnostic ignored "-Wcast-align"
							// And we write the full pixel to the fb (all 4 bytes)
							*((uint32_t*) (fbPtr + pix_offset)) = fb_px.p;
#	pragma GCC diagnostic pop
						} else if (img_px.color.a == 0) {
							// Transparent! Keep fb as-is.
						} else {
							// Alpha blending...
							uint8_t ainv = img_px.color.a ^ 0xFFu;

							pix_offset =
							    (uint32_t)((unsigned short int) (i + x_off) << 2U) +
							    ((unsigned short int) (j + y_off) * fInfo.line_length);
							FBInkPixelBGRA bg_px;
#	pragma GCC diagnostic push
#	pragma GCC diagnostic ignored "-Wcast-align"
							// Again, read the full pixel from the framebuffer (all 4 bytes)
							bg_px.p = *((uint32_t*) (fbPtr + pix_offset));
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
							*((uint32_t*) (fbPtr + pix_offset)) = fb_px.p;
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
						pix_offset = (size_t)(((j << 2U) * w) + (i << 2U));
#	pragma GCC diagnostic push
#	pragma GCC diagnostic ignored "-Wcast-align"
						// First, we gobble the full image pixel (all 4 bytes)
						// cppcheck-suppress unreadVariable ; false-positive (union)
						img_px.p = *((const uint32_t*) &data[pix_offset]);
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

							pix_offset =
							    (uint32_t)((unsigned short int) (i + x_off) << 2U) +
							    ((unsigned short int) (j + y_off) * fInfo.line_length);
							// And we write the full pixel to the fb (all 3 bytes)
							*((uint24_t*) (fbPtr + pix_offset)) = fb_px.p;
						} else if (img_px.color.a == 0) {
							// Transparent! Keep fb as-is.
						} else {
							// Alpha blending...
							uint8_t ainv = img_px.color.a ^ 0xFFu;

							pix_offset =
							    (uint32_t)((unsigned short int) (i + x_off) << 2U) +
							    ((unsigned short int) (j + y_off) * fInfo.line_length);
							// Again, read the full pixel from the framebuffer (all 3 bytes)
							FBInkPixelBGR bg_px;
							bg_px.p = *((uint24_t*) (fbPtr + pix_offset));

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
							*((uint24_t*) (fbPtr + pix_offset)) = fb_px.p;
						}
					}
				}
			}
		} else {
			// No alpha in image, or ignored
			// We don't care about image alpha in this branch, so we don't even store it.
			if (!fb_is_24bpp) {
				// 32bpp
				FBInkPixelBGRA fb_px;
				// This is essentially a constant in our case...
				// cppcheck-suppress unreadVariable ; false-positive (union)
				fb_px.color.a = 0xFFu;
				for (unsigned short int j = img_y_off; j < max_height; j++) {
					for (unsigned short int i = img_x_off; i < max_width; i++) {
						// NOTE: Here, req_n is either 4, or 3 if ignore_alpha, so, no shift trickery ;)
						size_t pix_offset = (size_t)((j * req_n * w) + (i * req_n));
						// Gobble the full image pixel (3 bytes, we don't care about alpha if it's there)
						FBInkPixelRGB img_px;
						img_px.p = *((const uint24_t*) &data[pix_offset]);
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
						pix_offset = (uint32_t)((unsigned short int) (i + x_off) << 2U) +
							     ((unsigned short int) (j + y_off) * fInfo.line_length);
#	pragma GCC diagnostic push
#	pragma GCC diagnostic ignored "-Wcast-align"
						// Write the full pixel to the fb (all 4 bytes)
						*((uint32_t*) (fbPtr + pix_offset)) = fb_px.p;
#	pragma GCC diagnostic pop
					}
				}
			} else {
				// 24bpp
				for (unsigned short int j = img_y_off; j < max_height; j++) {
					for (unsigned short int i = img_x_off; i < max_width; i++) {
						// NOTE: Here, req_n is either 4, or 3 if ignore_alpha, so, no shift trickery ;)
						size_t pix_offset = (size_t)((j * req_n * w) + (i * req_n));
						// Gobble the full image pixel (3 bytes, we don't care about alpha if it's there)
						FBInkPixelRGB img_px;
						img_px.p = *((const uint24_t*) &data[pix_offset]);
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
						pix_offset = (uint32_t)((unsigned short int) (i + x_off) << 2U) +
							     ((unsigned short int) (j + y_off) * fInfo.line_length);
						// Write the full pixel to the fb (all 3 bytes)
						*((uint24_t*) (fbPtr + pix_offset)) = fb_px.p;
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
					size_t         pix_offset = (size_t)(((j << 2U) * w) + (i << 2U));
					FBInkPixelRGBA img_px;
#	pragma GCC diagnostic push
#	pragma GCC diagnostic ignored "-Wcast-align"
					// Gobble the full image pixel (all 4 bytes)
					img_px.p = *((const uint32_t*) &data[pix_offset]);
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
						uint8_t ainv = img_px.color.a ^ 0xFFu;

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
					size_t pix_offset = (size_t)((j * req_n * w) + (i * req_n));
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

	// Rotate the region if need be...
	(*fxpRotateRegion)(&region);

	// Handle the last rect stuff...
	set_last_rect(&region);

	// Fudge the region if we asked for a screen clear, so that we actually refresh the full screen...
	if (fbink_cfg->is_cleared) {
		fullscreen_region(&region);
	}

	// Refresh screen
	if (refresh(fbfd,
		    region,
		    get_wfm_mode(fbink_cfg->wfm_mode),
		    fbink_cfg->is_dithered ? EPDC_FLAG_USE_DITHERING_ORDERED : EPDC_FLAG_USE_DITHERING_PASSTHROUGH,
		    fbink_cfg->is_nightmode,
		    fbink_cfg->is_flashing,
		    fbink_cfg->no_refresh) != EXIT_SUCCESS) {
		WARN("Failed to refresh the screen");
	}

	// Cleanup
cleanup:
	if (isFbMapped && !keep_fd) {
		unmap_fb();
	}
	if (!keep_fd) {
		close(fbfd);
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
	int                     w;
	int                     h;
	int                     n;
	data = img_load_from_file(filename, &w, &h, &n, req_n);
	if (data == NULL) {
		WARN("Failed to decode image data from '%s'", filename);
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
			float aspect = (float) w / (float) h;
			// We want to fit the image *inside* the viewport, so, enforce our starting scaled dimensions...
			scaled_width  = (unsigned short int) viewWidth;
			scaled_height = (unsigned short int) viewHeight;
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
			WARN("Failed to resize image");
			return ERRCODE(EXIT_FAILURE);
		}

		// We're drawing the scaled data, at the requested scaled resolution
		if (draw_image(fbfd, sdata, scaled_width, scaled_height, n, req_n, x_off, y_off, fbink_cfg) !=
		    EXIT_SUCCESS) {
			WARN("Failed to display image data on screen");
			rv = ERRCODE(EXIT_FAILURE);
			goto cleanup;
		}
	} else {
		// We're drawing the original unscaled data at its native resolution
		if (draw_image(fbfd, data, w, h, n, req_n, x_off, y_off, fbink_cfg) != EXIT_SUCCESS) {
			WARN("Failed to display image data on screen");
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
			 unsigned char* data                   UNUSED_BY_MINIMAL,
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
	unsigned char* restrict sdata        = NULL;
	bool                    want_scaling = false;
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
	unsigned char* imgdata = NULL;
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
			float aspect = (float) w / (float) h;
			// We want to fit the image *inside* the viewport, so, enforce our starting scaled dimensions...
			scaled_width  = (unsigned short int) viewWidth;
			scaled_height = (unsigned short int) viewHeight;
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
			WARN("Failed to resize image");
			return ERRCODE(EXIT_FAILURE);
		}

		// We're drawing the scaled data, at the requested scaled resolution
		if (draw_image(fbfd, sdata, scaled_width, scaled_height, n, req_n, x_off, y_off, fbink_cfg) !=
		    EXIT_SUCCESS) {
			WARN("Failed to display image data on screen");
			rv = ERRCODE(EXIT_FAILURE);
			goto cleanup;
		}
	} else {
		// We should now be able to draw that on screen, knowing that it probably won't horribly implode ;p
		if (draw_image(fbfd, imgdata, w, h, n, req_n, x_off, y_off, fbink_cfg) != EXIT_SUCCESS) {
			WARN("Failed to display image data on screen");
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
	dump->data = calloc((size_t)(fInfo.line_length * vInfo.yres), sizeof(*dump->data));
	if (dump->data == NULL) {
		WARN("dump->data calloc: %m");
		rv = ERRCODE(EXIT_FAILURE);
		goto cleanup;
	}
	// Store the current fb state for that dump
	dump->size        = (size_t)(fInfo.line_length * vInfo.yres);
	dump->area.left   = 0U;
	dump->area.top    = 0U;
	dump->area.width  = (unsigned short int) vInfo.xres_virtual;
	dump->area.height = (unsigned short int) vInfo.yres;
	dump->rota        = (uint8_t) vInfo.rotate;
	dump->bpp         = (uint8_t) vInfo.bits_per_pixel;
	dump->is_full     = true;
	// And finally, the fb data itself
	memcpy(dump->data, fbPtr, (size_t)(fInfo.line_length * vInfo.yres));

	// Cleanup
cleanup:
	if (isFbMapped && !keep_fd) {
		unmap_fb();
	}
	if (!keep_fd) {
		close(fbfd);
	}

	return rv;
#else
	WARN("Image support is disabled in this FBInk build");
	return ERRCODE(ENOSYS);
#endif    // FBINK_WITH_IMAGE
}

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
		if ((uint32_t)(w - img_x_off) < viewWidth) {
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
		if ((uint32_t)(h - img_y_off) < viewHeight) {
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

	// Free current data in case the dump struct is being reused
	if (dump->data) {
		LOG("Recycling FBinkDump!");
		free(dump->data);
		dump->data = NULL;
		// Reset the crop settings
		dump->clip = (const FBInkRect){ 0U };
	}
	// Start by allocating enough memory for a full dump of the computed region...
	// We're going to need the amount of bytes taken per pixel...
	uint8_t bpp = (uint8_t)(vInfo.bits_per_pixel / 8U);
	// And then to handle 4bpp on its own, because 4/8 == 0 ;).
	if (vInfo.bits_per_pixel == 4U) {
		// Align to the nearest byte boundary to make our life easier...
		if (region.left & 0x01u) {
			// x is odd, round *down* to the nearest multiple of two (i.e., align to the start of the current byte)
			region.left &= ~0x01u;
			LOG("Updated region.left to %u because of alignment constraints", region.left);
		}
		if (region.width & 0x01u) {
			// w is odd, round *up* to the nearest multiple of two (i.e., align to the end of the current byte)
			region.width = (region.width + 1) & ~0x01u;
			LOG("Updated region.width to %u because of alignment constraints", region.width);
		}
		// Two pixels per byte, and we've just ensured to never end up with a decimal when dividing by two ;).
		dump->data = calloc((size_t)((region.width >> 1) * region.height), sizeof(*dump->data));
	} else {
		dump->data = calloc((size_t)((region.width * bpp) * region.height), sizeof(*dump->data));
	}
	if (dump->data == NULL) {
		WARN("dump->data calloc: %m");
		rv = ERRCODE(EXIT_FAILURE);
		goto cleanup;
	}
	// Store the current fb state for that dump
	dump->area.left   = (unsigned short int) region.left;
	dump->area.top    = (unsigned short int) region.top;
	dump->area.width  = (unsigned short int) region.width;
	dump->area.height = (unsigned short int) region.height;
	dump->rota        = (uint8_t) vInfo.rotate;
	dump->bpp         = (uint8_t) vInfo.bits_per_pixel;
	dump->is_full     = false;
	// And finally, the fb data itself, scanline per scanline
	if (dump->bpp == 4U) {
		dump->size = (size_t)((dump->area.width >> 1) * dump->area.height);
		for (unsigned short int j = dump->area.top, l = 0U; l < dump->area.height; j++, l++) {
			size_t dump_offset = (size_t)(l * (dump->area.width >> 1));
			size_t fb_offset   = (size_t)(dump->area.left >> 1) + (j * fInfo.line_length);
			memcpy(dump->data + dump_offset, fbPtr + fb_offset, (size_t) dump->area.width >> 1);
		}
	} else {
		dump->size = (size_t)((dump->area.width * bpp) * dump->area.height);
		for (unsigned short int j = dump->area.top, l = 0U; l < dump->area.height; j++, l++) {
			size_t dump_offset = (size_t)(l * (dump->area.width * bpp));
			size_t fb_offset   = (size_t)(dump->area.left * bpp) + (j * fInfo.line_length);
			memcpy(dump->data + dump_offset, fbPtr + fb_offset, (size_t) dump->area.width * bpp);
		}
	}

	// Cleanup
cleanup:
	if (isFbMapped && !keep_fd) {
		unmap_fb();
	}
	if (!keep_fd) {
		close(fbfd);
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
		WARN("No dump data to restore!");
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
		memcpy(fbPtr, dump->data, (size_t)(fInfo.line_length * vInfo.yres));
		fullscreen_region(&region);
	} else {
		// NOTE: The crop codepath is perfectly safe with no cropping, it's just a little bit hairier to follow...
		if (dump->clip.width == 0U && dump->clip.height == 0U) {
			// Region dump, restore line by line
			if (dump->bpp == 4U) {
				for (unsigned short int j = dump->area.top, l = 0U; l < dump->area.height; j++, l++) {
					size_t fb_offset   = (size_t)(dump->area.left >> 1U) + (j * fInfo.line_length);
					size_t dump_offset = (size_t)(l * (dump->area.width >> 1U));
					memcpy(
					    fbPtr + fb_offset, dump->data + dump_offset, (size_t) dump->area.width >> 1U);
				}
			} else {
				// We're going to need the amount of bytes taken per pixel...
				const uint8_t bpp = dump->bpp >> 3U;
				for (unsigned short int j = dump->area.top, l = 0U; l < dump->area.height; j++, l++) {
					size_t fb_offset   = (size_t)(dump->area.left * bpp) + (j * fInfo.line_length);
					size_t dump_offset = (size_t)(l * (dump->area.width * bpp));
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
			const unsigned short int x = (unsigned short int) (dump->area.left + x_skip);
			const unsigned short int y_skip =
			    dump->clip.top > dump->area.top ? (unsigned short int) (dump->clip.top - dump->area.top) : 0U;
			const unsigned short int y = (unsigned short int) (dump->area.top + y_skip);
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
					size_t fb_offset = (size_t)(x >> 1U) + (j * fInfo.line_length);
					size_t dump_offset =
					    (size_t)((x_skip >> 1U) + ((y_skip + l) * (dump->area.width >> 1U)));
					memcpy(fbPtr + fb_offset, dump->data + dump_offset, (size_t) w >> 1U);
				}
			} else {
				// We're going to need the amount of bytes taken per pixel...
				const uint8_t bpp = dump->bpp >> 3U;
				for (unsigned short int j = y, l = 0U; l < h; j++, l++) {
					size_t fb_offset = (size_t)(x * bpp) + (j * fInfo.line_length);
					size_t dump_offset =
					    (size_t)((x_skip * bpp) + ((y_skip + l) * (dump->area.width * bpp)));
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
	if (refresh(fbfd,
		    region,
		    get_wfm_mode(fbink_cfg->wfm_mode),
		    fbink_cfg->is_dithered ? EPDC_FLAG_USE_DITHERING_ORDERED : EPDC_FLAG_USE_DITHERING_PASSTHROUGH,
		    fbink_cfg->is_nightmode,
		    fbink_cfg->is_flashing,
		    fbink_cfg->no_refresh) != EXIT_SUCCESS) {
		WARN("Failed to refresh the screen");
	}

	// Cleanup
cleanup:
	if (isFbMapped && !keep_fd) {
		unmap_fb();
	}
	if (!keep_fd) {
		close(fbfd);
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
		// Don't leave a dangling pointer, ensuring a subsequent dump() won't try to recycle this struct again.
		dump->data = NULL;
		// Invalidate the metadata while we're here
		dump->size    = 0U;
		dump->area    = (const FBInkRect){ 0U };
		dump->clip    = (const FBInkRect){ 0U };
		dump->rota    = 0U;
		dump->bpp     = 0U;
		dump->is_full = false;

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
    fbink_get_last_rect(void)
{
	return lastRect;
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
#endif
// Contains fbink_button_scan's implementation, Kobo only, and has a bit of Linux MT input thrown in ;).
#include "fbink_button_scan.c"
