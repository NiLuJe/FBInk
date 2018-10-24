/*
	FBInk: FrameBuffer eInker, a tool to print text & images on eInk devices (Kobo/Kindle)
	Copyright (C) 2018 NiLuJe <ninuje@gmail.com>

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
#	pragma GCC diagnostic push
#	pragma GCC diagnostic ignored "-Wunknown-pragmas"
#	pragma clang diagnostic ignored "-Wunknown-warning-option"
#	pragma GCC diagnostic ignored "-Wcast-qual"
#	pragma GCC diagnostic ignored "-Wcast-align"
#	pragma GCC diagnostic ignored "-Wconversion"
#	pragma GCC diagnostic ignored "-Wsign-conversion"
#	pragma GCC diagnostic ignored "-Wduplicated-branches"
#	pragma GCC diagnostic ignored "-Wunused-parameter"
#	pragma GCC diagnostic ignored "-Wunused-function"
#	include "stb/stb_image.h"
#	pragma GCC diagnostic pop
#endif

// Return the library version as devised at library compile-time
const char*
    fbink_version(void)
{
	return FBINK_VERSION;
}

// Helper functions to 'plot' a specific pixel in a given color to the framebuffer
static void
    put_pixel_Gray4(FBInkCoordinates* coords, FBInkColor* color)
{
	// calculate the pixel's byte offset inside the buffer
	// note: x / 2 as every byte holds 2 pixels
	size_t pix_offset = (coords->x >> 1U) + (coords->y * fInfo.line_length);

	// NOTE: Squash 8bpp to 4bpp:
	// (v >> 4)
	// or: v * 16 / 256

	// We can't address nibbles directly, so this takes some shenanigans...
	if ((coords->x & 0x01) == 0) {
		// Even pixel: high nibble
		*((unsigned char*) (fbPtr + pix_offset)) = (color->r & 0xF0);
		// Squash to 4bpp, and write to the top/left nibble
		// or: ((v >> 4) << 4)
	} else {
		// Odd pixel: low nibble
		// ORed to avoid clobbering our even pixel
		*((unsigned char*) (fbPtr + pix_offset)) |= (color->r >> 4U);
	}
}

static void
    put_pixel_Gray8(FBInkCoordinates* coords, FBInkColor* color)
{
	// calculate the pixel's byte offset inside the buffer
	size_t pix_offset = coords->x + coords->y * fInfo.line_length;

	// now this is about the same as 'fbp[pix_offset] = value'
	*((unsigned char*) (fbPtr + pix_offset)) = color->r;
}

static void
    put_pixel_RGB24(FBInkCoordinates* coords, FBInkColor* color)
{
	// calculate the pixel's byte offset inside the buffer
	// note: x * 3 as every pixel is 3 consecutive bytes
	size_t pix_offset = coords->x * 3U + coords->y * fInfo.line_length;

	// now this is about the same as 'fbp[pix_offset] = value'
	*((unsigned char*) (fbPtr + pix_offset))     = color->b;
	*((unsigned char*) (fbPtr + pix_offset + 1)) = color->g;
	*((unsigned char*) (fbPtr + pix_offset + 2)) = color->r;
}

static void
    put_pixel_RGB32(FBInkCoordinates* coords, FBInkColor* color)
{
	// calculate the pixel's byte offset inside the buffer
	// note: x * 4 as every pixel is 4 consecutive bytes
	size_t pix_offset = (uint32_t)(coords->x << 2U) + (coords->y * fInfo.line_length);

	// now this is about the same as 'fbp[pix_offset] = value'
	*((unsigned char*) (fbPtr + pix_offset))     = color->b;
	*((unsigned char*) (fbPtr + pix_offset + 1)) = color->g;
	*((unsigned char*) (fbPtr + pix_offset + 2)) = color->r;
	// Opaque, always. Note that everything is rendered as opaque, no matter what.
	// But at least this way we ensure fb grabs are consistent with what's seen on screen.
	*((unsigned char*) (fbPtr + pix_offset + 3)) = 0xFF;

	// NOTE: Trying to retrofit FBInkPixelBGRA into this doesn't appear to net us anything noticeable...
}

static void
    put_pixel_RGB565(FBInkCoordinates* coords, FBInkColor* color)
{
	// calculate the pixel's byte offset inside the buffer
	// note: x * 2 as every pixel is 2 consecutive bytes
	size_t pix_offset = (uint32_t)(coords->x << 1U) + (coords->y * fInfo.line_length);

	// now this is about the same as 'fbp[pix_offset] = value'
	// but a bit more complicated for RGB565
	uint16_t c = (uint16_t)(((color->r >> 3U) << 11U) | ((color->g >> 2U) << 5U) | (color->b >> 3U));
	// or: c = ((r / 8) * 2048) + ((g / 4) * 32) + (b / 8);
	// write 'two bytes at once', much to GCC's dismay...
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wcast-align"
	*((uint16_t*) (fbPtr + pix_offset)) = c;
#pragma GCC diagnostic pop
}

#ifndef FBINK_FOR_KINDLE
// Handle rotation quirks...
static void
    rotate_coordinates(FBInkCoordinates* coords)
{
	unsigned short int rx = coords->y;
	unsigned short int ry = (unsigned short int) (screenWidth - coords->x - 1);

// NOTE: This codepath is not production ready, it was just an experiment to wrap my head around framebuffer rotation...
//       In particular, only CW has been actually confirmed to behave properly (to handle the isKobo16Landscape quirk),
//       and region rotation is NOT handled properly/at all.
//       TL;DR: This is for documentation purposes only, never build w/ MATHS defined ;).
#	ifdef FBINK_WITH_MATHS
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
#endif    // !FBINK_FOR_KINDLE

static void
    rotate_nop(FBInkCoordinates* coords __attribute__((unused)))
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
static void
    put_pixel(FBInkCoordinates* coords, FBInkColor* color)
{
	// Handle rotation now, so we can properly validate if the pixel is off-screen or not ;).
	(*fxpRotateCoords)(coords);

	// NOTE: Discard off-screen pixels!
	//       For instance, when we have a halfcell offset in conjunction with a !isPerfectFit pixel offset,
	//       when we're padding and centering, the final whitespace of right-padding will have its last
	//       few pixels (the exact amount being half of the dead zone width) pushed off-screen...
	if (coords->x >= vInfo.xres || coords->y >= vInfo.yres) {
#ifdef DEBUG
		// NOTE: This is only enabled in Debug builds because it can be pretty verbose,
		//       and does not necessarily indicate an actual issue, as we've just explained...
		LOG("Put: discarding off-screen pixel @ (%hu, %hu) (out of %ux%u bounds)",
		    coords->x,
		    coords->y,
		    vInfo.xres,
		    vInfo.yres);
#endif
		return;
	}

	// fbink_init() takes care of setting this global pointer to the right function for the fb's bpp
	(*fxpPutPixel)(coords, color);
}

// Helper functions to 'get' a specific pixel's color from the framebuffer
// c.f., FBGrab convert* functions
//       (http://trac.ak-team.com/trac/browser/niluje/Configs/trunk/Kindle/Misc/FBGrab/fbgrab.c#L402)
// as well as KOReader's routines
//       (https://github.com/koreader/koreader-base/blob/b3e72affd0e1ba819d92194b229468452c58836f/ffi/blitbuffer.lua#L292)
static void
    get_pixel_Gray4(FBInkCoordinates* coords, FBInkColor* color)
{
	// NOTE: Expand 4bpp to 8bpp:
	// (v * 0x11)
	// Byte to nibble (c.f., https://en.wikipedia.org/wiki/Nibble)
	// Hi:
	// (((b) >> 4) & 0x0F)
	// Lo:
	// ((b) & 0x0F)

	if ((coords->x & 0x01) == 0) {
		// calculate the pixel's byte offset inside the buffer
		// note: x / 2 as every byte holds 2 pixels
		size_t  pix_offset = (coords->x >> 1U) + (coords->y * fInfo.line_length);
		uint8_t b          = *((unsigned char*) (fbPtr + pix_offset));

		// Even pixel: high nibble
		uint8_t v = (b & 0xF0);
		color->r  = (v | (v >> 4U));
		// pull the top/left nibble, expanded to 8bit
		// or: (uint8_t)((((b) >> 4) & 0x0F) * 0x11);

		// We need to get the low nibble *now*, before it gets clobbered by our alpha-blending put...
		// Thankfully, we have two empty channels in our color struct that we can use ;).
		color->g = (uint8_t)((b & 0x0F) * 0x11);
		// or: pull the low/right nibble, expanded to 8bit
	} else {
		// Odd pixel: low nibble
		// We just have to point to what we got during the even pixel pass ;).
		color->r = color->g;
	}
}

static void
    get_pixel_Gray8(FBInkCoordinates* coords, FBInkColor* color)
{
	// calculate the pixel's byte offset inside the buffer
	size_t pix_offset = coords->x + coords->y * fInfo.line_length;

	color->r = *((unsigned char*) (fbPtr + pix_offset));
}

static void
    get_pixel_RGB24(FBInkCoordinates* coords, FBInkColor* color)
{
	// calculate the pixel's byte offset inside the buffer
	// note: x * 3 as every pixel is 3 consecutive bytes
	size_t pix_offset = coords->x * 3U + coords->y * fInfo.line_length;

	color->b = *((unsigned char*) (fbPtr + pix_offset));
	color->g = *((unsigned char*) (fbPtr + pix_offset + 1));
	color->r = *((unsigned char*) (fbPtr + pix_offset + 2));
}

static void
    get_pixel_RGB32(FBInkCoordinates* coords, FBInkColor* color)
{
	// NOTE: We retrofitted a bit of union magic implemented for fbink_print_image for a noticeable performance bump :)
	FBInkPixelBGRA px;

	// calculate the pixel's byte offset inside the buffer
	// note: x * 4 as every pixel is 4 consecutive bytes, which we read in one go
	size_t pix_offset = (uint32_t)(coords->x << 2U) + (coords->y * fInfo.line_length);

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wcast-align"
	px.p = *((uint32_t*) (fbPtr + pix_offset));
#pragma GCC diagnostic pop
	color->b = px.color.b;
	color->g = px.color.g;
	color->r = px.color.r;
	// NOTE: We don't care about alpha, we always assume it's opaque,
	//       as that's how it behaves.
}

static void
    get_pixel_RGB565(FBInkCoordinates* coords, FBInkColor* color)
{
	// calculate the pixel's byte offset inside the buffer
	// note: x * 2 as every pixel is 2 consecutive bytes
	size_t pix_offset = (uint32_t)(coords->x << 1U) + (coords->y * fInfo.line_length);

	// NOTE: We're assuming RGB565 and not BGR565 here (as well as in put_pixel)...
	uint16_t v;
	uint8_t  r;
	uint8_t  g;
	uint8_t  b;
	// Like put_pixel_RGB565, read those two consecutive bytes at once
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wcast-align"
	v = *((uint16_t*) (fbPtr + pix_offset));
#pragma GCC diagnostic pop

	// NOTE: c.f., https://stackoverflow.com/q/2442576
	//       I feel that this approach tracks better with what we do in put_pixel_RGB565,
	//       and I have an easier time following it than the previous approach ported from KOReader.
	//       Both do exactly the same thing, though ;).
	r = (uint8_t)((v & 0xF800) >> 11U);
	g = (v & 0x07E0) >> 5U;
	b = (v & 0x001F);

	color->r = (uint8_t)((r << 3U) | (r >> 2U));
	color->g = (uint8_t)((g << 2U) | (g >> 4U));
	color->b = (uint8_t)((b << 3U) | (b >> 2U));
}

// Handle a few sanity checks...
static void
    get_pixel(FBInkCoordinates* coords, FBInkColor* color)
{
	// Handle rotation now, so we can properly validate if the pixel is off-screen or not ;).
	(*fxpRotateCoords)(coords);

	// NOTE: Discard off-screen pixels!
	//       For instance, when we have a halfcell offset in conjunction with a !isPerfectFit pixel offset,
	//       when we're padding and centering, the final whitespace of right-padding will have its last
	//       few pixels (the exact amount being half of the dead zone width) pushed off-screen...
	if (coords->x >= vInfo.xres || coords->y >= vInfo.yres) {
#ifdef DEBUG
		// NOTE: This is only enabled in Debug builds because it can be pretty verbose,
		//       and does not necessarily indicate an actual issue, as we've just explained...
		LOG("Put: discarding off-screen pixel @ (%hu, %hu) (out of %ux%u bounds)",
		    coords->x,
		    coords->y,
		    vInfo.xres,
		    vInfo.yres);
#endif
		return;
	}

	// fbink_init() takes care of setting this global pointer to the right function for the fb's bpp
	(*fxpGetPixel)(coords, color);
}

// Helper function to draw a rectangle in given color
static void
    fill_rect(unsigned short int x, unsigned short int y, unsigned short int w, unsigned short int h, FBInkColor* color)
{
	FBInkCoordinates coords = { 0U };
	for (unsigned short int cy = 0U; cy < h; cy++) {
		for (unsigned short int cx = 0U; cx < w; cx++) {
			coords.x = (unsigned short int) (x + cx);
			coords.y = (unsigned short int) (y + cy);
			put_pixel(&coords, color);
		}
	}
	LOG("Filled a %hux%hu rectangle @ (%hu, %hu)", w, h, x, y);
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
		if (is_flashing && v == eInkFGCMap[BG_WHITE]) {
			if (ioctl(fbfd, FBIO_EINK_CLEAR_SCREEN, EINK_CLEAR_SCREEN) < 0) {
				// NOTE: perror() is not thread-safe...
				char  buf[256];
				char* errstr = strerror_r(errno, buf, sizeof(buf));
				fprintf(stderr, "[FBInk] FBIO_EINK_CLEAR_SCREEN: %s\n", errstr);
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
		// And get out now, we don't want to pile another full memset on top of that ;).
		return;
	}
#endif
	memset(fbPtr, v, fInfo.smem_len);
}

// Return the font8x8 bitmap for a specific Unicode codepoint
static const unsigned char*
    font8x8_get_bitmap(uint32_t codepoint)
{
	// Get the bitmap for the character mapped to that Unicode codepoint
	if (codepoint <= 0x7F) {
		return font8x8_basic[codepoint];
	} else if (codepoint >= 0x80 && codepoint <= 0x9F) {
		return font8x8_control[codepoint - 0x80];
	} else if (codepoint >= 0xA0 && codepoint <= 0xFF) {
		return font8x8_ext_latin[codepoint - 0xA0];
	} else if (codepoint >= 0x390 && codepoint <= 0x3C9) {
		return font8x8_greek[codepoint - 0x390];
	} else if (codepoint >= 0x2500 && codepoint <= 0x257F) {
		return font8x8_box[codepoint - 0x2500];
	} else if (codepoint >= 0x2580 && codepoint <= 0x259F) {
		return font8x8_block[codepoint - 0x2580];
	} else if (codepoint >= 0x3040 && codepoint <= 0x309F) {
		return font8x8_hiragana[codepoint - 0x3040];
	} else {
		// NOTE: Print a blank space for unknown codepoints
		fprintf(stderr, "[FBInk] Codepoint U+%04X is not covered by this font!\n", codepoint);
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
#endif
		default:
			return "IBM (Default)";
	}
}

// Helper function for drawing
static struct mxcfb_rect
    draw(const char*        text,
	 unsigned short int row,
	 unsigned short int col,
	 unsigned short int multiline_offset,
	 bool               halfcell_offset,
	 const FBInkConfig* fbink_config)
{
	LOG("Printing '%s' @ line offset %hu (meaning row %hu)",
	    text,
	    multiline_offset,
	    (unsigned short int) (row + multiline_offset));

	// NOTE: It's a grayscale ramp, so r = g = b (= v).
	FBInkColor fgC = { fbink_config->is_inverted ? penBGColor : penFGColor, fgC.r, fgC.r };
	FBInkColor bgC = { fbink_config->is_inverted ? penFGColor : penBGColor, bgC.r, bgC.r };

	// Adjust row in case we're a continuation of a multi-line print...
	row = (unsigned short int) (row + multiline_offset);

	// Compute the length of our actual string
	// NOTE: We already took care in fbink_print() of making sure that the string passed in text wouldn't take up
	//       more space (as in columns, not bytes) than (MAXCOLS - col), the maximum printable length.
	//       And as we're printing glyphs, we need to iterate over the number of characters/grapheme clusters,
	//       not bytes.
	unsigned short int charcount = (unsigned short int) u8_strlen(text);
	LOG("Character count: %hu (over %zu bytes)", charcount, strlen(text));

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
	short int voffset = fbink_config->voffset;
	short int hoffset = fbink_config->hoffset;
	// NOTE: This test isn't perfect, but then, if you play with this, you do it knowing the risks...
	//       It's mainly there so that stupidly large values don't wrap back on screen because of overflow wraparound.
	if (abs(voffset) >= viewHeight) {
		LOG("The specified vertical offset (%hd) necessarily pushes *all* content out of bounds, discarding it",
		    voffset);
		voffset = 0;
	}
	if (abs(hoffset) >= viewWidth) {
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
	if (!fbink_config->is_overlay && !fbink_config->is_bgless) {
		if ((charcount == MAXCOLS || (col == 0 && !fbink_config->is_centered && multiline_offset > 0U)) &&
		    pixel_offset > 0U) {
			LOG("Painting a background rectangle on the left edge on account of pixel_offset");
			// Make sure we don't leave a hoffset sized gap when we have a positive hoffset...
			fill_rect(
			    hoffset > 0 ? (unsigned short int) (hoffset + viewHoriOrigin)
					: (unsigned short int) (0U + viewHoriOrigin),
			    (unsigned short int) (region.top + (unsigned short int) (multiline_offset * FONTH)),
			    pixel_offset,    // Don't append hoffset here, to make it clear stuff moved to the right.
			    FONTH,
			    &bgC);
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
	}

	// NOTE: In some cases, we also have a matching hole to patch on the right side...
	//       This only applies when pixel_offset *only* accounts for the !isPerfectFit adjustment though,
	//       because in every other case, the halfcell offset handling neatly pushes everything into place ;).
	// NOTE: Again, skip this in overlay/bgless mode ;).
	if (!fbink_config->is_overlay && !fbink_config->is_bgless) {
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
			    &bgC);
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
	if (multiline_offset > 0U && fbink_config->is_centered &&
	    (region.left > (0U + viewHoriOrigin) || region.width < screenWidth)) {
		region.left  = 0U + viewHoriOrigin;
		region.width = screenWidth;
		LOG("Enforced region.left to %u & region.width to %u because of multi-line centering",
		    region.left,
		    region.width);
	}

	// Loop through all the *characters* in the text string
	unsigned int       bi     = 0U;
	unsigned short int ci     = 0U;
	uint32_t           ch     = 0U;
	FBInkCoordinates   coords = { 0U };
	FBInkColor*        pxC;
	// NOTE: We don't do much sanity checking on hoffset/voffset,
	//       because we want to allow pushing part of the string off-screen
	//       (we basically only make sure it won't screw up the region rectangle too badly).
	//       put_pixel is checked, and will discard off-screen pixels safely.
	//       Because we store the final position in an unsigned value, this means that, to some extent,
	//       we rely on wraparound on underflow to still point to (large, but positive) off-screen coordinates.
	unsigned short int x_base_offs = (unsigned short int) ((col * FONTW) + pixel_offset + hoffset + viewHoriOrigin);
	unsigned short int y_offs      = (unsigned short int) ((row * FONTH) + voffset + viewVertOrigin);
	unsigned short int x_offs      = 0U;

	unsigned short int i;
	unsigned short int j;
	unsigned short int cx;
	unsigned short int cy;

	// NOTE: Extra code duplication because the glyph's bitmap data type depends on the glyph's width,
	//       so, one way or another, we have to duplicate the inner loops,
	//       but we want to inline this *and* branch outside the loops,
	//       and I don't feel like moving that to inline functions,
	//       because it depends on seven billion different variables I'd have to pass around...
#ifdef FBINK_WITH_FONTS
	if (glyphWidth <= 8) {
#endif
		while ((ch = u8_nextchar(text, &bi)) != 0U) {
			LOG("Char %hu (@ %hu) out of %hu is @ byte offset %u and is U+%04X",
			    (unsigned short int) (ci + 1U),
			    ci,
			    charcount,
			    bi,
			    ch);

			// Update the x coordinates for this character
			x_offs = (unsigned short int) (x_base_offs + (ci * FONTW));

			// Get the glyph's pixmap (width <= 8 -> uint8_t)
			const unsigned char* bitmap = NULL;
			bitmap                      = (*fxpFont8xGetBitmap)(ch);

			// Crappy macro to avoid repeating myself in each branch...
#define RENDER_GLYPH()                                                                                                   \
	/* NOTE: We only need to loop on the base glyph's dimensions (i.e., the bitmap resolution), */                   \
	/*       and from there compute the extra pixels for that single input pixel given our scaling factor... */      \
	if (!fbink_config->is_overlay && !fbink_config->is_bgless && !fbink_config->is_fgless) {                         \
		for (uint8_t y = 0U; y < glyphHeight; y++) {                                                             \
			/* y: input row, j: first output row after scaling */                                            \
			j = (unsigned short int) (y * FONTSIZE_MULT);                                                    \
			for (uint8_t x = 0U; x < glyphWidth; x++) {                                                      \
				/* x: input column, i: first output column after scaling */                              \
				i = (unsigned short int) (x * FONTSIZE_MULT);                                            \
				/* Each element encodes a full row, we access a column's bit in that row by shifting. */ \
				if (bitmap[y] & 1U << x) {                                                               \
					/* bit was set, pixel is fg! */                                                  \
					pxC = &fgC;                                                                      \
				} else {                                                                                 \
					/* bit was unset, pixel is bg */                                                 \
					pxC = &bgC;                                                                      \
				}                                                                                        \
				/* Initial coordinates, before we generate the extra pixels from the scaling factor */   \
				cx = (unsigned short int) (x_offs + i);                                                  \
				cy = (unsigned short int) (y_offs + j);                                                  \
				/* NOTE: Apply our scaling factor in both dimensions! */                                 \
				for (uint8_t l = 0U; l < FONTSIZE_MULT; l++) {                                           \
					for (uint8_t k = 0U; k < FONTSIZE_MULT; k++) {                                   \
						coords.x = (unsigned short int) (cx + k);                                \
						coords.y = (unsigned short int) (cy + l);                                \
						put_pixel(&coords, pxC);                                                 \
					}                                                                                \
				}                                                                                        \
			}                                                                                                \
		}                                                                                                        \
	} else {                                                                                                         \
		FBInkColor fbC     = { 0U };                                                                             \
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
					pxC     = &fgC;                                                                  \
					is_fgpx = true;                                                                  \
				} else {                                                                                 \
					/* bit was unset, pixel is bg */                                                 \
					pxC     = &bgC;                                                                  \
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
						if (is_fgpx && !fbink_config->is_fgless) {                               \
							if (fbink_config->is_overlay) {                                  \
								get_pixel(&coords, &fbC);                                \
								fbC.r ^= 0xFF;                                           \
								/* NOTE: Don't touch g & b if it's not needed! */        \
								/*       It's especially important on 4bpp, */           \
								/*       to avoid clobbering the low nibble, */          \
								/*       which we store in b... */                       \
								if (vInfo.bits_per_pixel > 8U) {                         \
									fbC.g ^= 0xFF;                                   \
									fbC.b ^= 0xFF;                                   \
								}                                                        \
								pxC = &fbC;                                              \
							}                                                                \
							put_pixel(&coords, pxC);                                         \
						} else if (!is_fgpx && fbink_config->is_fgless) {                        \
							put_pixel(&coords, pxC);                                         \
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
		/*
	} else if (glyphWidth <= 16) {
		while ((ch = u8_nextchar(text, &bi)) != 0U) {
			LOG("Char %hu (@ %hu) out of %hu is @ byte offset %u and is U+%04X",
			(unsigned short int) (ci + 1U),
			ci,
			charcount,
			bi,
			ch);

			// Update the x coordinates for this character
			x_offs = (unsigned short int) (x_base_offs + (ci * FONTW));

			// Get the glyph's pixmap (width <= 16 -> uint16_t)
			const uint16_t* bitmap = NULL;
			bitmap = (*fxpFont16xGetBitmap)(ch);

			// Render, scale & plot!
			RENDER_GLYPH();

			// Next glyph! This serves as the source for the pen position, hence it being used as an index...
			ci++;
		}
	*/
	} else if (glyphWidth <= 32) {
		while ((ch = u8_nextchar(text, &bi)) != 0U) {
			LOG("Char %hu (@ %hu) out of %hu is @ byte offset %u and is U+%04X",
			    (unsigned short int) (ci + 1U),
			    ci,
			    charcount,
			    bi,
			    ch);

			// Update the x coordinates for this character
			x_offs = (unsigned short int) (x_base_offs + (ci * FONTW));

			// Get the glyph's pixmap (width <= 32 -> uint32_t)
			const uint32_t* bitmap = NULL;
			bitmap                 = (*fxpFont32xGetBitmap)(ch);

			// Render, scale & plot!
			RENDER_GLYPH();

			// Next glyph! This serves as the source for the pen position, hence it being used as an index...
			ci++;
		}
		/*
	} else if (glyphWidth <= 64) {
		while ((ch = u8_nextchar(text, &bi)) != 0U) {
			LOG("Char %hu (@ %hu) out of %hu is @ byte offset %u and is U+%04X",
			(unsigned short int) (ci + 1U),
			ci,
			charcount,
			bi,
			ch);

			// Update the x coordinates for this character
			x_offs = (unsigned short int) (x_base_offs + (ci * FONTW));

			// Get the glyph's pixmap (width <= 64 -> uint64_t)
			const uint64_t* bitmap = NULL;
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

// NOTE: Small helper function to aid with logging the exact amount of time MXCFB_WAIT_FOR_UPDATE_COMPLETE blocked...
//       The driver is using the Kernel's wait-for-completion handler,
//       which returns the amount of jiffies left until the timeout set by the caller.
//       As we don't give a rat's ass about jiffies, we need to convert 'em to milliseconds.
// NOTE: The ioctl often actually blocks slightly longer than the perceived speed of the eInk refresh. Nevertheless,
//       there is a direct correlation between the two, as can be shown by switching between waveform modes...
static long int
    jiffies_to_ms(long int jiffies)
{
	// We need the Kernel's clock tick frequency for this, which we stored in USER_HZ during fbink_init ;).
	return (jiffies * 1000 / USER_HZ);
}

// Handle the various eInk update API quirks for the full range of HW we support...
#if defined(FBINK_FOR_KINDLE)
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
		// NOTE: perror() is not thread-safe...
		char  buf[256];
		char* errstr = strerror_r(errno, buf, sizeof(buf));
		if (is_fs) {
			fprintf(stderr, "[FBInk] FBIO_EINK_UPDATE_DISPLAY: %s\n", errstr);
		} else {
			fprintf(stderr, "[FBInk] FBIO_EINK_UPDATE_DISPLAY_AREA: %s\n", errstr);
		}
		return ERRCODE(EXIT_FAILURE);
	}

	return rv;
}

// Touch Kindle devices ([K5<->]KOA2)
static int
    refresh_kindle(int                     fbfd,
		   const struct mxcfb_rect region,
		   uint32_t                waveform_mode,
		   uint32_t                update_mode,
		   uint32_t                marker)
{
	struct mxcfb_update_data update = {
		.update_region           = region,
		.waveform_mode           = waveform_mode,
		.update_mode             = update_mode,
		.update_marker           = marker,
		.hist_bw_waveform_mode   = WAVEFORM_MODE_DU,
		.hist_gray_waveform_mode = WAVEFORM_MODE_GC16_FAST,
		.temp                    = TEMP_USE_AUTO,
		.flags                   = 0U,
		.alt_buffer_data         = { 0U },
	};

	int rv;
	rv = ioctl(fbfd, MXCFB_SEND_UPDATE, &update);

	if (rv < 0) {
		char  buf[256];
		char* errstr = strerror_r(errno, buf, sizeof(buf));
		fprintf(stderr, "[FBInk] MXCFB_SEND_UPDATE: %s\n", errstr);
		if (errno == EINVAL) {
			fprintf(stderr,
				"[FBInk] update_region={top=%u, left=%u, width=%u, height=%u}\n",
				region.top,
				region.left,
				region.width,
				region.height);
		}
		return ERRCODE(EXIT_FAILURE);
	}

	if (update_mode == UPDATE_MODE_FULL) {
		if (deviceQuirks.isKindlePearlScreen) {
			rv = ioctl(fbfd, MXCFB_WAIT_FOR_UPDATE_COMPLETE_PEARL, &marker);
		} else {
			struct mxcfb_update_marker_data update_marker = {
				.update_marker  = marker,
				.collision_test = 0U,
			};

			rv = ioctl(fbfd, MXCFB_WAIT_FOR_UPDATE_COMPLETE, &update_marker);
		}

		if (rv < 0) {
			char  buf[256];
			char* errstr = strerror_r(errno, buf, sizeof(buf));
			if (deviceQuirks.isKindlePearlScreen) {
				fprintf(stderr, "[FBInk] MXCFB_WAIT_FOR_UPDATE_COMPLETE_PEARL: %s\n", errstr);
			} else {
				fprintf(stderr, "[FBInk] MXCFB_WAIT_FOR_UPDATE_COMPLETE: %s\n", errstr);
			}
			return ERRCODE(EXIT_FAILURE);
		} else {
			// NOTE: Timeout is set to 5000ms
			LOG("Waited %ldms for completion of flashing update %u", (5000 - jiffies_to_ms(rv)), marker);
		}
	}

	return EXIT_SUCCESS;
}

// Kindle Oasis 2 ([KOA2<->??)
static int
    refresh_kindle_koa2(int                     fbfd,
			const struct mxcfb_rect region,
			uint32_t                waveform_mode,
			uint32_t                update_mode,
			uint32_t                marker)
{
	struct mxcfb_update_data_koa2 update = {
		.update_region = region,
		.waveform_mode = waveform_mode,
		.update_mode   = update_mode,
		.update_marker = marker,
		.temp          = TEMP_USE_AMBIENT,
		.flags         = (waveform_mode == WAVEFORM_MODE_KOA2_GLD16)
			     ? EPDC_FLAG_USE_KOA2_REGAL
			     : (waveform_mode == WAVEFORM_MODE_KOA2_A2 || waveform_mode == WAVEFORM_MODE_DU)
				   ? EPDC_FLAG_FORCE_MONOCHROME
				   : 0U,
		.dither_mode             = EPDC_FLAG_USE_DITHERING_PASSTHROUGH,
		.quant_bit               = 0,
		.alt_buffer_data         = { 0U },
		.hist_bw_waveform_mode   = WAVEFORM_MODE_DU,
		.hist_gray_waveform_mode = WAVEFORM_MODE_GC16,
		.ts_pxp                  = 0U,
		.ts_epdc                 = 0U,
	};

	int rv;
	rv = ioctl(fbfd, MXCFB_SEND_UPDATE_KOA2, &update);

	if (rv < 0) {
		char  buf[256];
		char* errstr = strerror_r(errno, buf, sizeof(buf));
		fprintf(stderr, "[FBInk] MXCFB_SEND_UPDATE_KOA2: %s\n", errstr);
		if (errno == EINVAL) {
			fprintf(stderr,
				"[FBInk] update_region={top=%u, left=%u, width=%u, height=%u}\n",
				region.top,
				region.left,
				region.width,
				region.height);
		}
		return ERRCODE(EXIT_FAILURE);
	}

	if (update_mode == UPDATE_MODE_FULL) {
		struct mxcfb_update_marker_data update_marker = {
			.update_marker  = marker,
			.collision_test = 0U,
		};

		rv = ioctl(fbfd, MXCFB_WAIT_FOR_UPDATE_COMPLETE, &update_marker);

		if (rv < 0) {
			char  buf[256];
			char* errstr = strerror_r(errno, buf, sizeof(buf));
			fprintf(stderr, "[FBInk] MXCFB_WAIT_FOR_UPDATE_COMPLETE: %s\n", errstr);
			return ERRCODE(EXIT_FAILURE);
		} else {
			// NOTE: Timeout is set to 5000ms
			LOG("Waited %ldms for completion of flashing update %u", (5000 - jiffies_to_ms(rv)), marker);
		}
	}

	return EXIT_SUCCESS;
}
#elif defined(FBINK_FOR_CERVANTES)
// Legacy Cervantes devices (2013)
static int
    refresh_cervantes(int                     fbfd,
		      const struct mxcfb_rect region,
		      uint32_t                waveform_mode,
		      uint32_t                update_mode,
		      uint32_t                marker)
{
	struct mxcfb_update_data update = {
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

	int rv;
	rv = ioctl(fbfd, MXCFB_SEND_UPDATE, &update);

	if (rv < 0) {
		char  buf[256];
		char* errstr = strerror_r(errno, buf, sizeof(buf));
		fprintf(stderr, "[FBInk] MXCFB_SEND_UPDATE: %s\n", errstr);
		if (errno == EINVAL) {
			fprintf(stderr,
				"[FBInk] update_region={top=%u, left=%u, width=%u, height=%u}\n",
				region.top,
				region.left,
				region.width,
				region.height);
		}
		return ERRCODE(EXIT_FAILURE);
	}

	if (update_mode == UPDATE_MODE_FULL) {
		rv = ioctl(fbfd, MXCFB_WAIT_FOR_UPDATE_COMPLETE, &marker);

		if (rv < 0) {
			char  buf[256];
			char* errstr = strerror_r(errno, buf, sizeof(buf));
			fprintf(stderr, "[FBInk] MXCFB_WAIT_FOR_UPDATE_COMPLETE: %s\n", errstr);
			return ERRCODE(EXIT_FAILURE);
		} else {
			// NOTE: Timeout is set to 10000ms
			LOG("Waited %ldms for completion of flashing update %u", (10000 - jiffies_to_ms(rv)), marker);
		}
	}

	return EXIT_SUCCESS;
}

// New cervantes devices (2013+)
static int
    refresh_cervantes_new(int                     fbfd,
			  const struct mxcfb_rect region,
			  uint32_t                waveform_mode,
			  uint32_t                update_mode,
			  uint32_t                marker)
{
	struct mxcfb_update_data_org update = {
		.update_region = region,
		.waveform_mode = waveform_mode,
		.update_mode   = update_mode,
		.update_marker = marker,
		.temp          = TEMP_USE_AMBIENT,
		.flags         = (waveform_mode == WAVEFORM_MODE_GLD16)
			     ? EPDC_FLAG_USE_REGAL
			     : (waveform_mode == WAVEFORM_MODE_A2) ? EPDC_FLAG_FORCE_MONOCHROME : 0U,
		.dither_mode     = EPDC_FLAG_USE_DITHERING_PASSTHROUGH,
		.quant_bit       = 0,
		.alt_buffer_data = { 0U },
	};

	int rv;
	rv = ioctl(fbfd, MXCFB_SEND_UPDATE, &update);

	if (rv < 0) {
		char  buf[256];
		char* errstr = strerror_r(errno, buf, sizeof(buf));
		fprintf(stderr, "[FBInk] MXCFB_SEND_UPDATE: %s\n", errstr);
		if (errno == EINVAL) {
			fprintf(stderr,
				"[FBInk] update_region={top=%u, left=%u, width=%u, height=%u}\n",
				region.top,
				region.left,
				region.width,
				region.height);
		}
		return ERRCODE(EXIT_FAILURE);
	}

	if (update_mode == UPDATE_MODE_FULL) {
		struct mxcfb_update_marker_data update_marker = {
			.update_marker  = marker,
			.collision_test = 0U,
		};

		rv = ioctl(fbfd, MXCFB_WAIT_FOR_UPDATE_COMPLETE2, &update_marker);

		if (rv < 0) {
			char  buf[256];
			char* errstr = strerror_r(errno, buf, sizeof(buf));
			fprintf(stderr, "[FBInk] MXCFB_WAIT_FOR_UPDATE_COMPLETE_V3: %s\n", errstr);
			return ERRCODE(EXIT_FAILURE);
		} else {
			// NOTE: Timeout is set to 5000ms
			LOG("Waited %ldms for completion of flashing update %u", (5000 - jiffies_to_ms(rv)), marker);
		}
	}
	return EXIT_SUCCESS;
}
#else
// Kobo devices ([Mk3<->Mk6])
static int
    refresh_kobo(int fbfd, const struct mxcfb_rect region, uint32_t waveform_mode, uint32_t update_mode, uint32_t marker)
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

	int rv;
	rv = ioctl(fbfd, MXCFB_SEND_UPDATE_V1_NTX, &update);

	if (rv < 0) {
		char  buf[256];
		char* errstr = strerror_r(errno, buf, sizeof(buf));
		fprintf(stderr, "[FBInk] MXCFB_SEND_UPDATE_V1_NTX: %s\n", errstr);
		if (errno == EINVAL) {
			fprintf(stderr,
				"[FBInk] update_region={top=%u, left=%u, width=%u, height=%u}\n",
				region.top,
				region.left,
				region.width,
				region.height);
		}
		return ERRCODE(EXIT_FAILURE);
	}

	if (update_mode == UPDATE_MODE_FULL) {
		rv = ioctl(fbfd, MXCFB_WAIT_FOR_UPDATE_COMPLETE_V1, &marker);

		if (rv < 0) {
			char  buf[256];
			char* errstr = strerror_r(errno, buf, sizeof(buf));
			fprintf(stderr, "[FBInk] MXCFB_WAIT_FOR_UPDATE_COMPLETE_V1: %s\n", errstr);
			return ERRCODE(EXIT_FAILURE);
		} else {
			// NOTE: Timeout is set to 10000ms
			LOG("Waited %ldms for completion of flashing update %u", (10000 - jiffies_to_ms(rv)), marker);
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
		.dither_mode     = EPDC_FLAG_USE_DITHERING_PASSTHROUGH,
		.quant_bit       = 0,
		.alt_buffer_data = { 0U },
	};

	int rv;
	rv = ioctl(fbfd, MXCFB_SEND_UPDATE_V2, &update);

	if (rv < 0) {
		char  buf[256];
		char* errstr = strerror_r(errno, buf, sizeof(buf));
		fprintf(stderr, "[FBInk] MXCFB_SEND_UPDATE_V2: %s\n", errstr);
		if (errno == EINVAL) {
			fprintf(stderr,
				"[FBInk] update_region={top=%u, left=%u, width=%u, height=%u}\n",
				region.top,
				region.left,
				region.width,
				region.height);
		}
		return ERRCODE(EXIT_FAILURE);
	}

	if (update_mode == UPDATE_MODE_FULL) {
		struct mxcfb_update_marker_data update_marker = {
			.update_marker  = marker,
			.collision_test = 0U,
		};

		rv = ioctl(fbfd, MXCFB_WAIT_FOR_UPDATE_COMPLETE_V3, &update_marker);

		if (rv < 0) {
			char  buf[256];
			char* errstr = strerror_r(errno, buf, sizeof(buf));
			fprintf(stderr, "[FBInk] MXCFB_WAIT_FOR_UPDATE_COMPLETE_V3: %s\n", errstr);
			return ERRCODE(EXIT_FAILURE);
		} else {
			// NOTE: Timeout is set to 5000ms
			LOG("Waited %ldms for completion of flashing update %u", (5000 - jiffies_to_ms(rv)), marker);
		}
	}

	return EXIT_SUCCESS;
}
#endif    // FBINK_FOR_KINDLE

// And finally, dispatch the right refresh request for our HW...
static int
    refresh(int fbfd, const struct mxcfb_rect region, uint32_t waveform_mode, bool is_flashing)
{
	// NOP when we don't have an eInk screen ;).
#ifdef FBINK_FOR_LINUX
	return EXIT_SUCCESS;
#endif

	// NOTE: Discard bogus regions, they can cause a softlock on some devices.
	//       A 0x0 region is a no go on most devices, while a 1x1 region may only upset some Kindle models.
	//       Some devices even balk at 1xN or Nx1, so, catch that, too.
	if (region.width <= 1 || region.height <= 1) {
		fprintf(stderr,
			"[FBInk] Discarding bogus empty region (%ux%u) to avoid a softlock.\n",
			region.width,
			region.height);
		return ERRCODE(EXIT_FAILURE);
	}

#ifdef FBINK_FOR_KINDLE
	if (deviceQuirks.isKindleLegacy) {
		return refresh_legacy(fbfd, region, is_flashing);
	}
#endif
	// NOTE: While we'd be perfect candidates for using A2 waveform mode, it's all kinds of fucked up on Kobos,
	//       and may lead to disappearing text or weird blending depending on the surrounding fb content...
	//       It only shows up properly when FULL, which isn't great...
	// NOTE: And while we're on the fun quirks train: FULL never flashes w/ AUTO on (some?) Kobos,
	//       so request GC16 if we want a flash...
	// NOTE: FWIW, DU behaves properly when PARTIAL, but doesn't flash when FULL.
	//       Which somewhat tracks given AUTO's behavior on Kobos, as well as on Kindles.
	//       (i.e., DU or GC16 is most likely often what AUTO will land on).

	// So, handle this common switcheroo here...
	uint32_t wfm    = (is_flashing && waveform_mode == WAVEFORM_MODE_AUTO) ? WAVEFORM_MODE_GC16 : waveform_mode;
	uint32_t upm    = is_flashing ? UPDATE_MODE_FULL : UPDATE_MODE_PARTIAL;
	uint32_t marker = (uint32_t) getpid();

	// NOTE: Make sure update_marker is valid, an invalid marker *may* hang the kernel instead of failing gracefully,
	//       depending on the device/FW...
	if (marker == 0U) {
		marker = (70U + 66U + 73U + 78U + 75U);
	}

#if defined(FBINK_FOR_KINDLE)
	if (deviceQuirks.isKindleOasis2) {
		return refresh_kindle_koa2(fbfd, region, wfm, upm, marker);
	} else {
		return refresh_kindle(fbfd, region, wfm, upm, marker);
	}
#elif defined(FBINK_FOR_CERVANTES)
	if (deviceQuirks.isCervantesNew) {
		return refresh_cervantes_new(fbfd, region, wfm, upm, marker);
	} else {
		return refresh_cervantes(fbfd, region, wfm, upm, marker);
	}
#else
	if (deviceQuirks.isKoboMk7) {
		return refresh_kobo_mk7(fbfd, region, wfm, upm, marker);
	} else {
		return refresh_kobo(fbfd, region, wfm, upm, marker);
	}
#endif    // FBINK_FOR_KINDLE
}

// Open the framebuffer file & return the opened fd
int
    fbink_open(void)
{
	int fbfd = -1;

	// Open the framebuffer file for reading and writing
	fbfd = open("/dev/fb0", O_RDWR | O_CLOEXEC);
	if (!fbfd) {
		fprintf(stderr, "[FBInk] Error: cannot open framebuffer character device.\n");
		return ERRCODE(EXIT_FAILURE);
	}

	return fbfd;
}

// Internal version of this which keeps track of whether we were fed an already opened fd or not...
static int
    open_fb_fd(int* fbfd, bool* keep_fd)
{
	if (*fbfd == FBFD_AUTO) {
		// If we're opening a fd now, don't keep it around.
		*keep_fd = false;
		if (ERRCODE(EXIT_FAILURE) == (*fbfd = fbink_open())) {
			fprintf(stderr, "[FBInk] Failed to open the framebuffer character device, aborting . . .\n");
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
int
    fbink_init(int fbfd, const FBInkConfig* fbink_config)
{
	// Open the framebuffer if need be...
	bool keep_fd = true;
	if (open_fb_fd(&fbfd, &keep_fd) != EXIT_SUCCESS) {
		return ERRCODE(EXIT_FAILURE);
	}

	// Assume success, until shit happens ;)
	int rv = EXIT_SUCCESS;

	// Update verbosity flag
	if (fbink_config->is_verbose) {
		g_isVerbose = true;
	} else {
		g_isVerbose = false;
	}
	// Update quiet flag
	if (fbink_config->is_quiet) {
		g_isQuiet = true;
	} else {
		g_isQuiet = false;
	}

	// Start with some more generic stuff, not directly related to the framebuffer.
	// As all this stuff is pretty much set in stone, we'll only query it once.
	if (!deviceQuirks.skipId) {
#ifndef FBINK_FOR_LINUX
		// Identify the device's specific model...
		identify_device(&deviceQuirks);
#	ifdef FBINK_FOR_KINDLE
		if (deviceQuirks.isKindleLegacy) {
			ELOG("[FBInk] Enabled Legacy einkfb Kindle quirks");
		} else if (deviceQuirks.isKindlePearlScreen) {
			ELOG("[FBInk] Enabled Kindle with Pearl screen quirks");
		} else if (deviceQuirks.isKindleOasis2) {
			ELOG("[FBInk] Enabled Kindle Oasis 2 quirks");
		}
#	else
		if (deviceQuirks.isKoboNonMT) {
			ELOG("[FBInk] Enabled Kobo w/o Multi-Touch quirks");
		} else if (deviceQuirks.isKoboMk7) {
			ELOG("[FBInk] Enabled Kobo Mark 7 quirks");
		}
#	endif
#endif

		// Ask the system for its clock tick frequency so we can translate jiffies into human-readable units.
		// NOTE: This will most likely be 100, even if CONFIG_HZ is > 100
		//       c.f., sysconf(3)
		long int rc = sysconf(_SC_CLK_TCK);
		if (rc > 0) {
			USER_HZ = rc;
			ELOG("[FBInk] Clock tick frequency appears to be %ld Hz", USER_HZ);
		} else {
			ELOG("[FBInk] Unable to query clock tick frequency, assuming %ld Hz", USER_HZ);
		}

		// And make sure we won't do that again ;).
		deviceQuirks.skipId = true;
	}

	// Get variable screen information
	if (ioctl(fbfd, FBIOGET_VSCREENINFO, &vInfo)) {
		fprintf(stderr, "[FBInk] Error reading variable information.\n");
		rv = ERRCODE(EXIT_FAILURE);
		goto cleanup;
	}
	ELOG("[FBInk] Variable fb info: %ux%u, %ubpp @ rotation: %u (%s)",
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
	fxpRotateCoords = &rotate_nop;
#ifndef FBINK_FOR_KINDLE
	// Make sure we default to no rotation shenanigans, to avoid issues on reinit...
	deviceQuirks.isKobo16Landscape = false;
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
			deviceQuirks.isKobo16Landscape = true;
			fxpRotateCoords                = &rotate_coordinates;
			ELOG("[FBInk] Enabled Kobo @ 16bpp fb rotation quirks (%ux%u -> %ux%u)",
			     vInfo.xres,
			     vInfo.yres,
			     screenWidth,
			     screenHeight);
		}
	}

	// Handle the Kobo viewport trickery for the few devices with hidden rows of pixels...
	// Things should generally not be broken-by-design on the horizontal axis...
	viewWidth      = screenWidth;
	viewHoriOrigin = 0U;
	// But on the vertical axis, oh my...
	if (!fbink_config->no_viewport && deviceQuirks.koboVertOffset != 0) {
		viewHeight = screenHeight - (uint32_t) abs(deviceQuirks.koboVertOffset);
		if (deviceQuirks.koboVertOffset > 0) {
			// Rows of pixels are hidden at the top
			viewVertOrigin = (uint8_t) deviceQuirks.koboVertOffset;
		} else {
			// Rows of pixels are hidden at the bottom
			viewVertOrigin = 0U;
		}
		ELOG("[FBInk] Enabled Kobo viewport insanity (%ux%u -> %ux%u), top-left corner is @ (%hhu, %hhu)",
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
	viewWidth      = screenWidth;
	viewHoriOrigin = 0U;
	viewHeight     = screenHeight;
	viewVertOrigin = 0U;
#endif

	// NOTE: Set (& reset) original font resolution, in case we're re-init'ing,
	//       since we're relying on the default value to calculate the scaled value,
	//       and we're using this value to set MAXCOLS & MAXROWS, which we *need* to be sane.
#ifdef FBINK_WITH_FONTS
	// Setup custom fonts (glyph size, render fx, bitmap fx)
	switch (fbink_config->fontname) {
		case SCIENTIFICA:
			glyphWidth         = 5U;
			glyphHeight        = 12U;
			fxpFont8xGetBitmap = &scientifica_get_bitmap;
			break;
		case SCIENTIFICAB:
			glyphWidth         = 5U;
			glyphHeight        = 12U;
			fxpFont8xGetBitmap = &scientificab_get_bitmap;
			break;
		case SCIENTIFICAI:
			glyphWidth         = 7U;
			glyphHeight        = 12U;
			fxpFont8xGetBitmap = &scientificai_get_bitmap;
			break;
		case ORP:
			glyphWidth         = 6U;
			glyphHeight        = 12U;
			fxpFont8xGetBitmap = &orp_get_bitmap;
			break;
		case ORPB:
			glyphWidth         = 6U;
			glyphHeight        = 12U;
			fxpFont8xGetBitmap = &orpb_get_bitmap;
			break;
		case ORPI:
			glyphWidth         = 6U;
			glyphHeight        = 12U;
			fxpFont8xGetBitmap = &orpi_get_bitmap;
			break;
		case KATES:
			glyphWidth         = 7U;
			glyphHeight        = 15U;
			fxpFont8xGetBitmap = &kates_get_bitmap;
			break;
		case UNSCII_TALL:
			glyphWidth         = 8U;
			glyphHeight        = 16U;
			fxpFont8xGetBitmap = &tall_get_bitmap;
			break;
		case VEGGIE:
			glyphWidth         = 8U;
			glyphHeight        = 16U;
			fxpFont8xGetBitmap = &veggie_get_bitmap;
			break;
		case FKP:
			glyphWidth         = 8U;
			glyphHeight        = 16U;
			fxpFont8xGetBitmap = &fkp_get_bitmap;
			break;
		case CTRLD:
			glyphWidth         = 8U;
			glyphHeight        = 16U;
			fxpFont8xGetBitmap = &ctrld_get_bitmap;
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
	glyphWidth         = 8U;
	glyphHeight        = 8U;
	fxpFont8xGetBitmap = &font8x8_get_bitmap;

	if (fbink_config->fontname != IBM) {
		ELOG("[FBInk] Custom fonts are not supported in this FBInk build, using IBM instead.");
	}
#endif

	// Obey user-specified font scaling multiplier
	if (fbink_config->fontmult > 0) {
		FONTSIZE_MULT = fbink_config->fontmult;
		uint8_t max_fontmult;

		// NOTE: Clamp to safe values to avoid a division by zero later if a glyph becomes so big
		//       that it causes MAXCOLS or MAXROWS to become too small...
		//       We want to ensure available_cols in fbink_print will be > 0, so,
		//       follow the same heuristics to devise the minimum MAXCOLS we want to enforce...
		unsigned short int min_maxcols = 1U;
		if (fbink_config->is_centered) {
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
			ELOG("[FBInk] Clamped font size multiplier from %hhu to %hhu",
			     fbink_config->fontmult,
			     max_fontmult);
		}
#else
		// The default font's glyphs are 8x8, do the least amount of work possible ;).
		max_fontmult = (uint8_t)(viewWidth / min_maxcols / 8U);
		if (FONTSIZE_MULT > max_fontmult) {
			FONTSIZE_MULT = max_fontmult;
			ELOG("[FBInk] Clamped font size multiplier from %hhu to %hhu",
			     fbink_config->fontmult,
			     max_fontmult);
		}
#endif
	} else {
		// Set font-size based on screen resolution (roughly matches: Pearl, Carta, Carta HD & 7" Carta, 7" Carta HD)
		// NOTE: We still want to compare against the screen's "height", even in Landscape mode,
		//       so we simply use the longest edge to do just that...
		uint32_t actual_height = MAX(vInfo.xres, vInfo.yres);
		if (actual_height <= 600U) {
			FONTSIZE_MULT = 1U;    // 8x8
		} else if (actual_height <= 1024U) {
			FONTSIZE_MULT = 2U;    // 16x16
		} else if (actual_height <= 1448U) {
			FONTSIZE_MULT = 3U;    // 24x24
		} else {
			FONTSIZE_MULT = 4U;    // 32x32
		}
#ifdef FBINK_WITH_FONTS
		if (fbink_config->fontname == BLOCK) {
			// Block is roughly 4 times wider than other fonts, compensate for that...
			FONTSIZE_MULT = (uint8_t) MAX(1U, FONTSIZE_MULT / 4U);
		}
#endif
	}
	// Go!
	FONTW = (unsigned short int) (glyphWidth * FONTSIZE_MULT);
	FONTH = (unsigned short int) (glyphHeight * FONTSIZE_MULT);
	ELOG("[FBInk] Fontsize set to %hux%hu (%s base glyph size: %hhux%hhu)",
	     FONTW,
	     FONTH,
	     fontname_to_string(fbink_config->fontname),
	     glyphWidth,
	     glyphHeight);

	// Compute MAX* values now that we know the screen & font resolution
	MAXCOLS = (unsigned short int) (viewWidth / FONTW);
	MAXROWS = (unsigned short int) (viewHeight / FONTH);
	ELOG("[FBInk] Line length: %hu cols, Page size: %hu rows", MAXCOLS, MAXROWS);

	// Mention & remember if we can perfectly fit the final column on screen
	if ((FONTW * MAXCOLS) == viewWidth) {
		deviceQuirks.isPerfectFit = true;
		ELOG("[FBInk] Horizontal fit is perfect!");
	} else {
		deviceQuirks.isPerfectFit = false;
	}

	// In a similar fashion, add a vertical offset to make sure rows are vertically "centered",
	// in case we can't perfectly fit the final row.
	if ((FONTH * MAXROWS) == viewHeight) {
		viewVertOffset = 0U;
	} else {
		// NOTE: That should also fall under no_viewport's purview
		if (!fbink_config->no_viewport) {
			viewVertOffset = (uint8_t)(((float) (viewHeight - (uint32_t)(FONTH * MAXROWS)) / 2.0f) + 0.5f);
			ELOG("[FBInk] Vertical fit isn't perfect, shifting rows down by %hhu pixels", viewVertOffset);
		} else {
			viewVertOffset = 0U;
			ELOG("[FBInk] Vertical fit isn't perfect, but viewport fiddling was explicitly disabled");
		}
	}
	// Bake that into the viewport computations,
	// we'll special-case the image codepath to ignore it when row is unspecified (i.e., 0) ;).
	viewVertOrigin = (uint8_t)(viewVertOrigin + viewVertOffset);

	// Get fixed screen information
	if (ioctl(fbfd, FBIOGET_FSCREENINFO, &fInfo)) {
		fprintf(stderr, "[FBInk] Error reading fixed information.\n");
		rv = ERRCODE(EXIT_FAILURE);
		goto cleanup;
	}
	ELOG("[FBInk] Fixed fb info: ID is \"%s\", length of fb mem: %u bytes & line length: %u bytes",
	     fInfo.id,
	     fInfo.smem_len,
	     fInfo.line_length);
	// NOTE: On a reinit, we're trusting that smem_len will *NOT* have changed,
	//       which thankfully appears to hold true on our target devices.
	//       Otherwise, we'd probably have to compare the previous smem_len to the new, and to
	//       mremap fbPtr if isFbMapped in case they differ (and the old smem_len != 0, which would indicate a first init).

	// Use the appropriate get/put pixel functions...
	switch (vInfo.bits_per_pixel) {
		case 4U:
			fxpPutPixel = &put_pixel_Gray4;
			fxpGetPixel = &get_pixel_Gray4;
			break;
		case 8U:
			fxpPutPixel = &put_pixel_Gray8;
			fxpGetPixel = &get_pixel_Gray8;
			break;
		case 16U:
			fxpPutPixel = &put_pixel_RGB565;
			fxpGetPixel = &get_pixel_RGB565;
			break;
		case 24U:
			fxpPutPixel = &put_pixel_RGB24;
			fxpGetPixel = &get_pixel_RGB24;
			break;
		case 32U:
			fxpPutPixel = &put_pixel_RGB32;
			fxpGetPixel = &get_pixel_RGB32;
			break;
		default:
			// Huh oh... Should never happen!
			fprintf(stderr, "[FBInk] Unsupported framebuffer bpp!\n");
			rv = ERRCODE(EXIT_FAILURE);
			goto cleanup;
			break;
	}

		// NOTE: Now that we know which device we're running on, setup pen colors,
		//       taking into account the inverted cmap on legacy Kindles...
#ifdef FBINK_FOR_KINDLE
	if (deviceQuirks.isKindleLegacy) {
		penFGColor = eInkBGCMap[fbink_config->fg_color];
		penBGColor = eInkFGCMap[fbink_config->bg_color];

		ELOG(
		    "[FBInk] Pen colors set to #%02X%02X%02X -> #%02X%02X%02X for the foreground and #%02X%02X%02X -> #%02X%02X%02X for the background",
		    eInkFGCMap[fbink_config->fg_color],
		    eInkFGCMap[fbink_config->fg_color],
		    eInkFGCMap[fbink_config->fg_color],
		    penFGColor,
		    penFGColor,
		    penFGColor,
		    eInkBGCMap[fbink_config->bg_color],
		    eInkBGCMap[fbink_config->bg_color],
		    eInkBGCMap[fbink_config->bg_color],
		    penBGColor,
		    penBGColor,
		    penBGColor);
	} else {
#endif
		penFGColor = eInkFGCMap[fbink_config->fg_color];
		penBGColor = eInkBGCMap[fbink_config->bg_color];

		ELOG("[FBInk] Pen colors set to #%02X%02X%02X for the foreground and #%02X%02X%02X for the background",
		     penFGColor,
		     penFGColor,
		     penFGColor,
		     penBGColor,
		     penBGColor,
		     penBGColor);
#ifdef FBINK_FOR_KINDLE
	}
#endif

	// NOTE: Do we want to keep the fb0 fd open, or simply close it for now?
	//       Useful because we probably want to close it to keep open fds to a minimum when used as a library,
	//       while wanting to avoid a useless open/close/open/close cycle when used as a standalone tool.
cleanup:
	if (!keep_fd) {
		close(fbfd);
	}

	return rv;
}

// Dump a few of our internal state variables to stdout, for shell script consumption
void
    fbink_state_dump(const FBInkConfig* fbink_config)
{
	fprintf(
	    stdout,
	    "viewWidth=%u;viewHeight=%u;screenWidth=%u;screenHeight=%u;viewHoriOrigin=%hhu;viewVertOrigin=%hhu;viewVertOffset=%hhu;BPP=%u;FONTW=%hu;FONTH=%hu;FONTSIZE_MULT=%hhu;FONTNAME='%s';glyphWidth=%hhu;glyphHeight=%hhu;MAXCOLS=%hu;MAXROWS=%hu;isPerfectFit=%d;FBID=%s;USER_HZ=%ld;penFGColor=%hhu;penBGColor=%hhu",
	    viewWidth,
	    viewHeight,
	    screenWidth,
	    screenHeight,
	    viewHoriOrigin,
	    viewVertOrigin,
	    viewVertOffset,
	    vInfo.bits_per_pixel,
	    FONTW,
	    FONTH,
	    FONTSIZE_MULT,
	    fontname_to_string(fbink_config->fontname),
	    glyphWidth,
	    glyphHeight,
	    MAXCOLS,
	    MAXROWS,
	    deviceQuirks.isPerfectFit,
	    fInfo.id,
	    USER_HZ,
	    penFGColor,
	    penBGColor);
}

// Dump a few of our internal state variables to the FBInkState struct pointed to by fbink_state
void
    fbink_get_state(const FBInkConfig* fbink_config, FBInkState* fbink_state)
{
	if (fbink_state) {
		fbink_state->view_width       = viewWidth;
		fbink_state->view_height      = viewHeight;
		fbink_state->screen_width     = screenWidth;
		fbink_state->screen_height    = screenHeight;
		fbink_state->view_hori_origin = viewHoriOrigin;
		fbink_state->view_vert_origin = viewVertOrigin;
		fbink_state->view_vert_offset = viewVertOffset;
		fbink_state->bpp              = vInfo.bits_per_pixel;
		fbink_state->font_w           = FONTW;
		fbink_state->font_h           = FONTH;
		fbink_state->fontsize_mult    = FONTSIZE_MULT;
		fbink_state->font_name        = fontname_to_string(fbink_config->fontname);
		fbink_state->glyph_width      = glyphWidth;
		fbink_state->glyph_height     = glyphHeight;
		fbink_state->max_cols         = MAXCOLS;
		fbink_state->max_rows         = MAXROWS;
		fbink_state->is_perfect_fit   = deviceQuirks.isPerfectFit;
		fbink_state->user_hz          = USER_HZ;
		fbink_state->pen_fg_color     = penFGColor;
		fbink_state->pen_bg_color     = penBGColor;
	} else {
		fprintf(stderr, "[FBInk] Err, it appears we were passed a NULL fbink_state pointer?\n");
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
	fbPtr = (unsigned char*) mmap(NULL, fInfo.smem_len, PROT_READ | PROT_WRITE, MAP_SHARED, fbfd, 0);
	if (fbPtr == MAP_FAILED) {
		char  buf[256];
		char* errstr = strerror_r(errno, buf, sizeof(buf));
		fprintf(stderr, "[FBInk] mmap: %s\n", errstr);
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
		char  buf[256];
		char* errstr = strerror_r(errno, buf, sizeof(buf));
		fprintf(stderr, "[FBInk] munmap: %s\n", errstr);
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
			char  buf[256];
			char* errstr = strerror_r(errno, buf, sizeof(buf));
			fprintf(stderr, "[FBInk] close: %s\n", errstr);
			return ERRCODE(EXIT_FAILURE);
		}
	}

	return EXIT_SUCCESS;
}

// Much like rotate_coordinates, but for a mxcfb rectangle
static void
    rotate_region(struct mxcfb_rect* region)
{
	// Rotate the region if need be...
	struct mxcfb_rect oregion = *region;
	// NOTE: left = x, top = y
	region->top    = screenWidth - oregion.left - oregion.width;
	region->left   = oregion.top;
	region->width  = oregion.height;
	region->height = oregion.width;
}

// Tweak the region to cover the full screen
static void
    fullscreen_region(struct mxcfb_rect* region)
{
	region->top    = 0U;
	region->left   = 0U;
	region->width  = vInfo.xres;
	region->height = vInfo.yres;
}

// Magic happens here!
int
    fbink_print(int fbfd, const char* string, const FBInkConfig* fbink_config)
{
	// If we open a fd now, we'll only keep it open for this single print call!
	// NOTE: We *expect* to be initialized at this point, though, but that's on the caller's hands!
	bool keep_fd = true;
	if (open_fb_fd(&fbfd, &keep_fd) != EXIT_SUCCESS) {
		return ERRCODE(EXIT_FAILURE);
	}

	// Assume success, until shit happens ;)
	int rv = EXIT_SUCCESS;
	// We need to declare this early (& sentinel it to NULL) to make our cleanup jumps safe
	char* line = NULL;

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
	short int col = fbink_config->col;
	short int row = fbink_config->row;

	// NOTE: If we asked to print in the middle of the screen, make the specified row an offset from the middle of the screen,
	//       instead of the top.
	if (fbink_config->is_halfway) {
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
	if (fbink_config->is_cleared) {
		clear_screen(fbfd, fbink_config->is_inverted ? penFGColor : penBGColor, fbink_config->is_flashing);
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
	size_t       len       = strlen(string);
	unsigned int charcount = u8_strlen(string);
	// Check how much extra storage is used up by multibyte sequences.
	if (len > charcount) {
		LOG("Extra storage used up by multibyte sequences: %zu bytes (for a total of %u characters over %zu bytes)",
		    (len - charcount),
		    charcount,
		    len);
	}

	// Compute the amount of characters we can actually print on *one* line given the column we start on...
	// NOTE: When centered, we enforce one padding character on the left,
	//       as well as one padding character on the right when we have a perfect fit.
	//       This is to avoid potentially printing stuff too close to the bezel and/or behind the bezel.
	unsigned short int available_cols = MAXCOLS;
	if (fbink_config->is_centered) {
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
		char  buf[256];
		char* errstr = strerror_r(errno, buf, sizeof(buf));
		fprintf(stderr, "[FBInk] calloc (line): %s\n", errstr);
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

	LOG("Need %hu lines to print %u characters over %hu available columns", lines, charcount, available_cols);

	// Do the initial computation outside the loop,
	// so we'll be able to re-use line_len to accurately compute chars_left when looping.
	// NOTE: This is where it gets tricky. With multibyte sequences, 1 byte doesn't necessarily mean 1 char.
	//       And we need to work both in amount of characters for column/width arithmetic,
	//       and in bytes for snprintf...
	unsigned int       chars_left = charcount;
	unsigned short int line_len   = 0U;
	// If we have multiple lines worth of stuff to print, draw it line per line
	while (chars_left > line_len) {
		LOG("Line %hu (of ~%hu), previous line was %hu characters long and there were %u characters left to print",
		    (unsigned short int) (multiline_offset + 1U),
		    lines,
		    line_len,
		    chars_left);
		// Make sure we don't try to draw off-screen...
		if (row + multiline_offset >= MAXROWS) {
			LOG("Can only print %hu lines, discarding the %u characters left!",
			    MAXROWS,
			    chars_left - line_len);
			// And that's it, we're done.
			break;
		}

		// Compute the amount of characters left to print...
		chars_left -= line_len;
		// And use it to compute the amount of characters to print on *this* line
		line_len = (unsigned short int) MIN(chars_left, available_cols);
		LOG("Characters to print: %hu out of the %u remaining ones", line_len, chars_left);

		// NOTE: Now we just have to switch from characters to bytes, both for line_len & chars_left...
		// First, get the byte offset of this section of our string (i.e., this line)...
		unsigned int line_offset = u8_offset(string, charcount - chars_left);
		// ... then compute how many bytes we'll need to store it.
		unsigned int       line_bytes = 0U;
		unsigned short int cn         = 0U;
		uint32_t           ch         = 0U;
		while ((ch = u8_nextchar(string + line_offset, &line_bytes)) != 0U) {
			cn++;
			// NOTE: Honor linefeeds...
			//       The main use-case for this is throwing tail'ed logfiles at us and having them
			//       be readable instead of a jumbled glued together mess ;).
			if (ch == 0x0A) {
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
				// (it'll render as a blank), mostly to make padding look nicer,
				// but also so that line_bytes matches line_len ;).
				// And finally, as we've explained earlier, trim line_len to where we stopped.
				LOG("Line length was %hu characters, but LF is character number %u", line_len, cn);
				line_len = cn;
				// Don't touch line_offset, the beginning of our line has not changed,
				// only its length was cut short.
				LOG("Adjusted lines to %hu & line_len to %hu", lines, line_len);
				// And of course we break, because that was the whole point of this shenanigan!
				break;
			}
			// We've walked our full line, stop!
			if (cn >= line_len) {
				break;
			}
		}
		LOG("Line takes up %u bytes", line_bytes);
		int bytes_printed = 0;

		// Just fudge the column for centering...
		bool halfcell_offset = false;
		if (fbink_config->is_centered) {
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
		if (fbink_config->is_centered && fbink_config->is_padded) {
			// We always want full padding
			col = 0;

			// Compute our padding length
			unsigned short int left_pad = (unsigned short int) (MAXCOLS - line_len) / 2U;
			// As for the right padding, we basically just have to print 'til the edge of the screen
			unsigned short int right_pad = (unsigned short int) (MAXCOLS - line_len - left_pad);

			// Compute the effective right padding value for science!
			LOG("Total size: %hu + %hu + %hu = %hu",
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
		} else if (fbink_config->is_padded) {
			// NOTE: Rely on the field width for padding ;).
			// Padding character is a space, which is 1 byte, so that's good enough ;).
			unsigned int padded_bytes = line_bytes + (unsigned int) (available_cols - line_len);
			// NOTE: Don't touch line_len, because we're *adding* new blank characters,
			//       we're still printing the exact same amount of characters *from our string*.
			LOG("Padded %u bytes to %u to cover %hu columns", line_bytes, padded_bytes, available_cols);
			bytes_printed = snprintf(
			    line, padded_bytes + 1U, "%*.*s", (int) padded_bytes, (int) line_bytes, string + line_offset);
		} else {
			// NOTE: Enforce precision for safety.
			bytes_printed = snprintf(
			    line, line_bytes + 1U, "%*.*s", (int) line_bytes, (int) line_bytes, string + line_offset);
		}
		LOG("snprintf wrote %d bytes", bytes_printed);

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
			      fbink_config);

		// Next line!
		multiline_offset++;

		// NOTE: If we've actually written something, and we're not the final line,
		//       clear what we've just written, to get back a pristine NULL-filled buffer,
		//       so u8_nextchar() has zero chance to skip a NULL on the next iteration.
		//       See the comments around the initial calloc() call for more details.
		if (bytes_printed > 0 && line_len < chars_left) {
			LOG("We have more stuff to print, clearing the line buffer for re-use!\n");
			memset(line, 0, (size_t) bytes_printed);
		}
		// The nuclear option is simply to unconditonally zero the *full* buffer ;).
		//memset(line, 0, ((MAXCOLS + 1U) * 4U) * sizeof(*line));
	}

	// Rotate the region if need be...
	if (deviceQuirks.isKobo16Landscape) {
		rotate_region(&region);
	}

	// Fudge the region if we asked for a screen clear, so that we actually refresh the full screen...
	if (fbink_config->is_cleared) {
		fullscreen_region(&region);
	}

	// Refresh screen
	if (refresh(fbfd, region, WAVEFORM_MODE_AUTO, fbink_config->is_flashing) != EXIT_SUCCESS) {
		fprintf(stderr, "[FBInk] Failed to refresh the screen!\n");
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

// printf-like wrapper around fbink_print ;).
int
    fbink_printf(int fbfd, const FBInkConfig* fbink_config, const char* fmt, ...)
{
	// We'll need to store our formatted string somewhere...
	// NOTE: Fit a single page's worth of characters in it, as that's the best we can do anyway.
	// NOTE: UTF-8 is at most 4 bytes per sequence, make sure we can fit a full page of UTF-8 (+1 'wide' NULL) :).
	char* buffer = NULL;
	// NOTE: We use calloc to make sure it'll always be zero-initialized,
	//       and the OS is smart enough to make it fast if we don't use the full space anyway (CoW zeroing).
	buffer = calloc(((size_t)(MAXCOLS * MAXROWS) + 1U) * 4U, sizeof(*buffer));
	if (buffer == NULL) {
		char  buf[256];
		char* errstr = strerror_r(errno, buf, sizeof(buf));
		fprintf(stderr, "[FBInk] calloc (page): %s\n", errstr);
		return ERRCODE(EXIT_FAILURE);
	}

	va_list args;
	va_start(args, fmt);
	// vsnprintf will ensure we'll always be NULL-terminated (but not NULL-backfilled, hence calloc!) ;).
	vsnprintf(buffer, ((size_t)(MAXCOLS * MAXROWS) * 4U) + 1U, fmt, args);
	va_end(args);

	int rv = fbink_print(fbfd, buffer, fbink_config);

	// Cleanup
	free(buffer);
	return rv;
}

// Small public wrapper around refresh(), without the caller having to depend on mxcfb headers
int
    fbink_refresh(int         fbfd,
		  uint32_t    region_top,
		  uint32_t    region_left,
		  uint32_t    region_width,
		  uint32_t    region_height,
		  const char* waveform_mode,
		  bool        is_flashing)
{
	// Open the framebuffer if need be...
	bool keep_fd = true;
	if (open_fb_fd(&fbfd, &keep_fd) != EXIT_SUCCESS) {
		return ERRCODE(EXIT_FAILURE);
	}

	uint32_t region_wfm = WAVEFORM_MODE_AUTO;
	// Parse waveform mode...
#ifdef FBINK_FOR_KINDLE
	if (strcasecmp("DU", waveform_mode) == 0) {
		region_wfm = WAVEFORM_MODE_DU;
	} else if (strcasecmp("GC16", waveform_mode) == 0) {
		region_wfm = WAVEFORM_MODE_GC16;
	} else if (strcasecmp("GC4", waveform_mode) == 0) {
		region_wfm = WAVEFORM_MODE_GC4;
	} else if (strcasecmp("A2", waveform_mode) == 0) {
		if (deviceQuirks.isKindleOasis2) {
			region_wfm = WAVEFORM_MODE_KOA2_A2;
		} else {
			region_wfm = WAVEFORM_MODE_A2;
		}
	} else if (strcasecmp("GL16", waveform_mode) == 0) {
		if (deviceQuirks.isKindleOasis2) {
			region_wfm = WAVEFORM_MODE_KOA2_GL16;
		} else {
			region_wfm = WAVEFORM_MODE_GL16;
		}
	} else if (strcasecmp("REAGL", waveform_mode) == 0) {
		if (deviceQuirks.isKindleOasis2) {
			region_wfm = WAVEFORM_MODE_KOA2_REAGL;
		} else {
			region_wfm = WAVEFORM_MODE_REAGL;
		}
	} else if (strcasecmp("REAGLD", waveform_mode) == 0) {
		if (deviceQuirks.isKindleOasis2) {
			region_wfm = WAVEFORM_MODE_KOA2_REAGLD;
		} else {
			region_wfm = WAVEFORM_MODE_REAGLD;
		}
	} else if (strcasecmp("GC16_FAST", waveform_mode) == 0) {
		if (deviceQuirks.isKindleOasis2) {
			region_wfm = WAVEFORM_MODE_KOA2_GC16_FAST;
		} else {
			region_wfm = WAVEFORM_MODE_GC16_FAST;
		}
	} else if (strcasecmp("GL16_FAST", waveform_mode) == 0) {
		if (deviceQuirks.isKindleOasis2) {
			region_wfm = WAVEFORM_MODE_KOA2_GL16_FAST;
		} else {
			region_wfm = WAVEFORM_MODE_GL16_FAST;
		}
	} else if (strcasecmp("DU4", waveform_mode) == 0) {
		region_wfm = WAVEFORM_MODE_DU4;
	} else if (strcasecmp("GL4", waveform_mode) == 0) {
		if (deviceQuirks.isKindleOasis2) {
			region_wfm = WAVEFORM_MODE_KOA2_GL4;
		} else {
			region_wfm = WAVEFORM_MODE_GL4;
		}
	} else if (strcasecmp("GL16_INV", waveform_mode) == 0) {
		if (deviceQuirks.isKindleOasis2) {
			region_wfm = WAVEFORM_MODE_KOA2_GL16_INV;
		} else {
			region_wfm = WAVEFORM_MODE_GL16_INV;
		}
	} else if (deviceQuirks.isKindleOasis2 && strcasecmp("GCK16", waveform_mode) == 0) {
		region_wfm = WAVEFORM_MODE_KOA2_GCK16;
	} else if (deviceQuirks.isKindleOasis2 && strcasecmp("GLKW16", waveform_mode) == 0) {
		region_wfm = WAVEFORM_MODE_KOA2_GLKW16;
#else
	if (strcasecmp("DU", waveform_mode) == 0) {
		region_wfm = WAVEFORM_MODE_DU;
	} else if (strcasecmp("GC16", waveform_mode) == 0) {
		region_wfm = WAVEFORM_MODE_GC16;
	} else if (strcasecmp("GC4", waveform_mode) == 0) {
		region_wfm = WAVEFORM_MODE_GC4;
	} else if (strcasecmp("A2", waveform_mode) == 0) {
		region_wfm = WAVEFORM_MODE_A2;
	} else if (strcasecmp("GL16", waveform_mode) == 0) {
		region_wfm = WAVEFORM_MODE_GL16;
	} else if (strcasecmp("REAGL", waveform_mode) == 0) {
		region_wfm = WAVEFORM_MODE_REAGL;
	} else if (strcasecmp("REAGLD", waveform_mode) == 0) {
		region_wfm = WAVEFORM_MODE_REAGLD;
#endif
	} else if (strcasecmp("AUTO", waveform_mode) == 0) {
		region_wfm = WAVEFORM_MODE_AUTO;
	} else {
		LOG("Unknown (or unsupported) waveform mode '%s', defaulting to AUTO", waveform_mode);
		region_wfm = WAVEFORM_MODE_AUTO;
	}

	struct mxcfb_rect region = {
		.top    = region_top,
		.left   = region_left,
		.width  = region_width,
		.height = region_height,
	};

	int ret;
	if (EXIT_SUCCESS != (ret = refresh(fbfd, region, region_wfm, is_flashing))) {
		fprintf(stderr, "[FBInk] Failed to refresh the screen!\n");
	}

	if (!keep_fd) {
		close(fbfd);
	}

	return ret;
}

// Simple public getter for temporary Device Quirks
bool
    fbink_is_fb_quirky(void)
{
	// NOTE: For now, that's easy enough, we only have one ;).
	return deviceQuirks.isKobo16Landscape;
}

// Handle drawing both types of progress bars
int
    draw_progress_bars(int fbfd, bool is_infinite, uint8_t value, const FBInkConfig* fbink_config)
{
	// Clear screen?
	if (fbink_config->is_cleared) {
		clear_screen(fbfd, fbink_config->is_inverted ? penFGColor : penBGColor, fbink_config->is_flashing);
	}

	// Let's go! Start by pilfering some computations from draw...
	// NOTE: It's a grayscale ramp, so r = g = b (= v).
	FBInkColor fgC = { fbink_config->is_inverted ? penBGColor : penFGColor, fgC.r, fgC.r };
	FBInkColor bgC = { fbink_config->is_inverted ? penFGColor : penBGColor, bgC.r, bgC.r };

	// Clamp v offset to safe values
	// NOTE: This test isn't perfect, but then, if you play with this, you do it knowing the risks...
	//       It's mainly there so that stupidly large values don't wrap back on screen because of overflow wraparound.
	short int voffset = fbink_config->voffset;
	if (abs(voffset) >= viewHeight) {
		LOG("The specified vertical offset (%hd) necessarily pushes *all* content out of bounds, discarding it",
		    voffset);
		voffset = 0;
	}

	// And then some from fbink_print...
	short int row = fbink_config->row;
	if (fbink_config->is_halfway) {
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
	if (!fbink_config->is_bgless) {
		fill_rect(left_pos, top_pos, (unsigned short int) screenWidth, FONTH, &bgC);
	}

	// NOTE: We always use the same BG_ constant in order to get a rough inverse by just swapping to the inverted LUT ;).
	FBInkColor emptyC;
	FBInkColor borderC;
	// Handle devices with an inverted palette properly...
	if (deviceQuirks.isKindleLegacy) {
		emptyC.r  = fbink_config->is_inverted ? eInkBGCMap[BG_GRAYB] : eInkFGCMap[BG_GRAYB];
		borderC.r = fbink_config->is_inverted ? eInkBGCMap[BG_GRAY4] : eInkFGCMap[BG_GRAY4];
	} else {
		emptyC.r  = fbink_config->is_inverted ? eInkFGCMap[BG_GRAYB] : eInkBGCMap[BG_GRAYB];
		borderC.r = fbink_config->is_inverted ? eInkFGCMap[BG_GRAY4] : eInkBGCMap[BG_GRAY4];
	}
	emptyC.g  = emptyC.r;
	emptyC.b  = emptyC.r;
	borderC.g = borderC.r;
	borderC.b = borderC.r;

	// Which kind of bar did we request?
	if (!is_infinite) {
		// This is a real progress bar ;).

		// We'll want 5% of padding on each side,
		// with rounding to make sure the bar's size is constant across all percentage values...
		unsigned short int fill_width =
		    (unsigned short int) (((value / 100.0f) * (0.90f * (float) viewWidth)) + 0.5f);
		unsigned short int fill_left = (unsigned short int) (left_pos + (0.05f * (float) viewWidth) + 0.5f);
		unsigned short int empty_width =
		    (unsigned short int) ((((float) (100U - value) / 100.0f) * (0.90f * (float) viewWidth)) + 0.5f);
		unsigned short int empty_left = (unsigned short int) (fill_left + fill_width);

		// Draw the border...
		fill_rect(fill_left, top_pos, (unsigned short int) (fill_width + empty_width), FONTH, &borderC);
		// Draw the fill bar, which we want to override the border with!
		fill_rect(fill_left, top_pos, fill_width, FONTH, &fgC);
		// And the empty bar...
		// NOTE: With a minor tweak to keep a double-width border on the bottom & right sides ;).
		if (value == 0U) {
			// Keep the left border alone!
			fill_rect((unsigned short int) (empty_left + 1U),
				  (unsigned short int) (top_pos + 1U),
				  (unsigned short int) MAX(0, empty_width - 3),
				  (unsigned short int) (FONTH - 3U),
				  &emptyC);
		} else {
			fill_rect(empty_left,
				  (unsigned short int) (top_pos + 1U),
				  (unsigned short int) MAX(0, empty_width - 2),
				  (unsigned short int) (FONTH - 3U),
				  &emptyC);
		}

		// We enforce centering for the percentage text...
		// NOTE: Zero init, and leave enough space for a *wide* NULL,
		//       to avoid spurious behavior with u8_strlen later on...
		char percentage_text[8] = { 0 };
		snprintf(percentage_text, sizeof(percentage_text), "%hhu%%", value);
		size_t line_len = strlen(percentage_text);

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
		draw(percentage_text,
		     (unsigned short int) row,
		     (unsigned short int) col,
		     0U,
		     halfcell_offset,
		     fbink_config);
	} else {
		// This is an infinite progress bar (a.k.a., activity bar)!

		// We'll want 5% of padding on each side,
		// with rounding to make sure the bar's size is constant across all percentage values...
		unsigned short int empty_width = (unsigned short int) ((0.90f * (float) viewWidth) + 0.5f);
		unsigned short int empty_left  = (unsigned short int) (left_pos + (0.05f * (float) viewWidth) + 0.5f);

		// Draw the border...
		fill_rect(empty_left, top_pos, (unsigned short int) (empty_width), FONTH, &borderC);
		// Draw the empty bar...
		fill_rect((unsigned short int) (empty_left + 1U),
			  (unsigned short int) (top_pos + 1U),
			  (unsigned short int) MAX(0, empty_width - 3),
			  (unsigned short int) (FONTH - 3U),
			  &emptyC);

		// We want our thumb to take 20% of the bar's width
		unsigned short int thumb_width = (unsigned short int) ((0.20f * empty_width) + 0.5f);
		// We move the thumb in increment of 5% of the bar's width (i.e., half its width),
		// with rounding to avoid accumulating drift...
		unsigned short int thumb_left =
		    (unsigned short int) (empty_left + ((0.05f * empty_width) * value) + 0.5f);

		// And finally, draw the thumb, which we want to override the border with!
		fill_rect(thumb_left, top_pos, thumb_width, FONTH, &fgC);

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
				  &bgC);
		}
	}

	// Start setting up the screen refresh...
	struct mxcfb_rect region = {
		.top    = top_pos,
		.left   = left_pos,
		.width  = screenWidth,
		.height = FONTH,
	};

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
	if (deviceQuirks.isKobo16Landscape) {
		rotate_region(&region);
	}

	// Fudge the region if we asked for a screen clear, so that we actually refresh the full screen...
	if (fbink_config->is_cleared) {
		fullscreen_region(&region);
	}

	// And finally, refresh the screen
	// NOTE: FWIW, using A2 basically ends up drawing the border black, and the empty white, which kinda works...
	//       It has the added benefit of increasing the framerate limit after which the eInk controller risks getting
	//       confused (unless is_flashing is enabled, since that'll block,
	//       essentially throttling the bar to the screen's refresh rate).
	if (refresh(fbfd, region, WAVEFORM_MODE_AUTO, fbink_config->is_flashing) != EXIT_SUCCESS) {
		fprintf(stderr, "[FBInk] Failed to refresh the screen!\n");
		return ERRCODE(EXIT_FAILURE);
	}

	return EXIT_SUCCESS;
}

// Draw a full-width progress bar
int
    fbink_print_progress_bar(int fbfd, uint8_t percentage, const FBInkConfig* caller_fbink_config)
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
	FBInkConfig fbink_config = *caller_fbink_config;
	// Namely, we need overlay mode to properly print the percentage text,
	fbink_config.is_overlay = true;
	// and no hoffset, because it makes no sense for a full-width bar,
	// and we don't want the text to be affected by a stray value...
	fbink_config.hoffset = 0;
	// And we enforce centered text internally, so we'll set col ourselves later...

	// And do the work ;).
	rv = draw_progress_bars(fbfd, false, percentage, &fbink_config);

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
    fbink_print_activity_bar(int fbfd, uint8_t progress, const FBInkConfig* caller_fbink_config)
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

	// We don't need to fudge with fbink_config, no text to show ;).

	// Begin by sanitizing the input...
	if (progress > 16U) {
		LOG("The specified progress step (%hhu) is larger than 16, clamping it.", progress);
		progress = 16U;
	}

	// And do the work ;).
	rv = draw_progress_bars(fbfd, true, progress, caller_fbink_config);

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
	unsigned char* data = NULL;

	// Read image either from stdin (provided we're not running from a terminal), or a file
	if (strcmp(filename, "-") == 0 && !isatty(fileno(stdin))) {
		// NOTE: Ideally, we'd simply feed stdin to stbi_load_from_file, but that doesn't work because it relies on fseek,
		//       so read stdin ourselves...
		//       c.f., https://stackoverflow.com/a/44894946
		unsigned char* imgdata = NULL;
		unsigned char* temp    = NULL;
		size_t         size    = 0;
		size_t         used    = 0;
		size_t         nread;

		if (ferror(stdin)) {
			fprintf(stderr, "[FBInk] Failed to read image data from stdin!\n");
			return NULL;
		}

#	define CHUNK (256 * 1024)
		while (1) {
			if (used + CHUNK + 1U > size) {
				size = used + CHUNK + 1U;

				// Overflow check
				if (size <= used) {
					free(imgdata);
					fprintf(stderr, "[FBInk] Too much input data!\n");
					return NULL;
				}

				// OOM check
				temp = realloc(imgdata, size);
				if (temp == NULL) {
					free(imgdata);
					fprintf(stderr, "[FBInk] realloc: out of memory!\n");
					return NULL;
				}
				imgdata = temp;
				temp    = NULL;
			}

			nread = fread(imgdata + used, 1U, CHUNK, stdin);
			if (nread == 0) {
				break;
			}
			used += nread;
		}

		if (ferror(stdin)) {
			free(imgdata);
			fprintf(stderr, "[FBInk] Failed to read image data from stdin!\n");
			return NULL;
		}

		// Shrink & NULL terminate
		// NOTE: We're not buffering C strings, and we're discarding the buffer very very soon, so skip that ;).
		/*
		temp = realloc(imgdata, used + 1U);
		if (temp == NULL) {
			free(imgdata);
			fprintf(stderr, "[FBInk] realloc: out of memory!\n");
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
		fprintf(stderr, "[FBInk] Failed to open or decode image '%s'!\n", filename);
		return NULL;
	}

	LOG("Requested %d color channels, image had %d", req_n, *n);

	return data;
}

// Convert raw image data between various pixel formats
// NOTE: This is a direct copy of stbi's stbi__convert_format, except that it doesn't free the input buffer.
static unsigned char*
    img_convert_px_format(unsigned char* data, int img_n, int req_comp, int x, int y)
{
	unsigned char* good;

	// NOTE: We're already doing that in fbink_print_raw_data ;)
	//if (req_comp == img_n) return data;
	STBI_ASSERT(req_comp >= 1 && req_comp <= 4);

	good = (unsigned char*) stbi__malloc_mad3(req_comp, x, y, 0);
	if (good == NULL) {
		//STBI_FREE(data);
		return NULL;
	}

	for (int j = 0; j < (int) y; ++j) {
		unsigned char* src  = data + j * x * img_n;
		unsigned char* dest = good + j * x * req_comp;

		// NOTE: stbi undef's STBI__CASE, but not STBI__COMBO...
#	ifdef STBI__COMBO
#		undef STBI__COMBO
#	endif
#	define STBI__COMBO(a, b) ((a) *8 + (b))
#	define STBI__CASE(a, b)                                                                                         \
		case STBI__COMBO(a, b):                                                                                  \
			for (int i = x - 1; i >= 0; --i, src += a, dest += b)
		// convert source image with img_n components to one with req_comp components;
		// avoid switch per pixel, so use switch per scanline and massive macros
		switch (STBI__COMBO(img_n, req_comp)) {
			STBI__CASE(1, 2)
			{
				dest[0] = src[0], dest[1] = 255;
			}
			break;
			STBI__CASE(1, 3)
			{
				dest[0] = dest[1] = dest[2] = src[0];
			}
			break;
			STBI__CASE(1, 4)
			{
				dest[0] = dest[1] = dest[2] = src[0], dest[3] = 255;
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
				dest[0] = dest[1] = dest[2] = src[0], dest[3] = src[1];
			}
			break;
			STBI__CASE(3, 4)
			{
				dest[0] = src[0], dest[1] = src[1], dest[2] = src[2], dest[3] = 255;
			}
			break;
			STBI__CASE(3, 1)
			{
				dest[0] = stbi__compute_y(src[0], src[1], src[2]);
			}
			break;
			STBI__CASE(3, 2)
			{
				dest[0] = stbi__compute_y(src[0], src[1], src[2]), dest[1] = 255;
			}
			break;
			STBI__CASE(4, 1)
			{
				dest[0] = stbi__compute_y(src[0], src[1], src[2]);
			}
			break;
			STBI__CASE(4, 2)
			{
				dest[0] = stbi__compute_y(src[0], src[1], src[2]), dest[1] = src[3];
			}
			break;
			STBI__CASE(4, 3)
			{
				dest[0] = src[0], dest[1] = src[1], dest[2] = src[2];
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

// Draw image data on screen (we inherit a few of the variable types/names from stbi ;))
static int
    draw_image(int                fbfd,
	       unsigned char*     data,
	       const int          w,
	       const int          h,
	       const int          n,
	       const int          req_n,
	       short int          x_off,
	       short int          y_off,
	       const FBInkConfig* fbink_config)
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
	if (fbink_config->is_cleared) {
		clear_screen(fbfd, fbink_config->is_inverted ? penFGColor : penBGColor, fbink_config->is_flashing);
	}

	// NOTE: We compute initial offsets from row/col, to help aligning images with text.
	if (fbink_config->col < 0) {
		x_off = (short int) (viewHoriOrigin + x_off + (MAX(MAXCOLS + fbink_config->col, 0) * FONTW));
	} else {
		x_off = (short int) (viewHoriOrigin + x_off + (fbink_config->col * FONTW));
	}
	// NOTE: Unless we *actually* specified a row, ignore viewVertOffset
	//       The rationale being we want to keep being aligned to text rows when we do specify a row,
	//       but we don't want the extra offset when we don't (in particular, when printing full-screen images).
	// NOTE: This means that row 0 and row -MAXROWS *will* behave differently, but so be it...
	if (fbink_config->row < 0) {
		y_off = (short int) (viewVertOrigin + y_off + (MAX(MAXROWS + fbink_config->row, 0) * FONTH));
	} else if (fbink_config->row == 0) {
		y_off = (short int) (viewVertOrigin - viewVertOffset + y_off + (fbink_config->row * FONTH));
		// This of course means that row 0 effectively breaks that "align with text" contract if viewVertOffset != 0,
		// on the off-chance we do explicitly really want to align something to row 0, so, warn about it...
		// The "print full-screen images" use-case is greatly more prevalent than "actually rely on row 0 alignment" ;).
		// And in case that's *really* needed, using -MAXROWS instead of 0 will honor alignment anyway.
		if (viewVertOffset != 0U) {
			LOG("Ignoring the %hhupx row offset because row is 0!", viewVertOffset);
		}
	} else {
		y_off = (short int) (viewVertOrigin + y_off + (fbink_config->row * FONTH));
	}
	LOG("Adjusted image display coordinates to (%hd, %hd), after column %hd & row %hd",
	    x_off,
	    y_off,
	    fbink_config->col,
	    fbink_config->row);

	bool       fb_is_grayscale = false;
	bool       fb_is_legacy    = false;
	bool       fb_is_24bpp     = false;
	bool       fb_is_true_bgr  = false;
	bool       img_has_alpha   = false;
	FBInkColor color           = { 0U };
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
	switch (fbink_config->halign) {
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
	if (fbink_config->halign != NONE) {
		LOG("Adjusted image display coordinates to (%hd, %hd) after horizontal alignment", x_off, y_off);
	}

	// Handle vertical alignment...
	switch (fbink_config->valign) {
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
	if (fbink_config->valign != NONE) {
		LOG("Adjusted image display coordinates to (%hd, %hd) after vertical alignment", x_off, y_off);
	}

	// Clamp everything to a safe range, because we can't have *anything* going off-screen here.
	struct mxcfb_rect region;
	// NOTE: Assign each field individually to avoid a false-positive with Clang's SA...
	if (fbink_config->row == 0) {
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
		if (fbink_config->row == 0) {
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
		if (fbink_config->ignore_alpha) {
			LOG("Ignoring the image's alpha channel.");
		} else {
			LOG("Image has an alpha channel, we'll have to do alpha blending.");
		}
	}

	// Handle inversion if requested, in a way that avoids branching in the loop ;).
	// And, as an added bonus, plays well with the fact that legacy devices have an inverted color map...
	uint8_t  invert     = 0U;
	uint32_t invert_rgb = 0U;
#	ifdef FBINK_FOR_KINDLE
	if ((deviceQuirks.isKindleLegacy && !fbink_config->is_inverted) ||
	    (!deviceQuirks.isKindleLegacy && fbink_config->is_inverted)) {
#	else
	if (fbink_config->is_inverted) {
#	endif
		invert     = 0xFF;
		invert_rgb = 0x00FFFFFF;
	}
	unsigned short int i;
	unsigned short int j;
	// NOTE: The *slight* duplication is on purpose, to move the branching outside the loop,
	//       and make use of a few different blitting tweaks depending on the situation...
	//       And since we can easily do so from here,
	//       we also entirely avoid trying to plot off-screen pixels (on any sides).
	if (fb_is_grayscale) {
		// 4bpp & 8bpp
		if (!fbink_config->ignore_alpha && img_has_alpha) {
			if (!fb_is_legacy) {
				// 8bpp
				// There's an alpha channel in the image, we'll have to do alpha blending...
				// c.f., https://en.wikipedia.org/wiki/Alpha_compositing
				//       https://blogs.msdn.microsoft.com/shawnhar/2009/11/06/premultiplied-alpha/
				FBInkCoordinates coords   = { 0U };
				FBInkColor       bg_color = { 0U };
				size_t           pix_offset;
				FBInkPixelG8A    img_px;
				uint8_t          ainv = 0U;
				for (j = img_y_off; j < max_height; j++) {
					for (i = img_x_off; i < max_width; i++) {
						// NOTE: In this branch, req_n == 2, so we can do << 1 instead of * 2 ;).
						pix_offset = (size_t)(((j << 1U) * w) + (i << 1U));
#	pragma GCC diagnostic push
#	pragma GCC diagnostic ignored "-Wcast-align"
						// First, we gobble the full image pixel (all 2 bytes)
						img_px.p = *((uint16_t*) &data[pix_offset]);
#	pragma GCC diagnostic pop

						// Take a shortcut for the most common alpha values (none & full)
						if (img_px.color.a == 0xFF) {
							// Fully opaque, we can blit the image (almost) directly.
							// We do need to honor inversion ;).
							color.r = img_px.color.v ^ invert;

							coords.x = (unsigned short int) (i + x_off);
							coords.y = (unsigned short int) (j + y_off);

							(*fxpPutPixel)(&coords, &color);
						} else if (img_px.color.a == 0) {
							// Transparent! Keep fb as-is.
						} else {
							// Alpha blending...

							// We need to know what this pixel currently looks like in the framebuffer...
							coords.x = (unsigned short int) (i + x_off);
							coords.y = (unsigned short int) (j + y_off);
							// NOTE: We use the the function pointers directly, to avoid the OOB checks,
							//       because we know we're only processing on-screen pixels,
							//       and we don't care about the rotation checks at this bpp :).
							(*fxpGetPixel)(&coords, &bg_color);

							ainv = img_px.color.a ^ 0xFF;
							// Don't forget to honor inversion
							img_px.color.v ^= invert;
							// Blend it!
							color.r = (uint8_t) DIV255(
							    ((img_px.color.v * img_px.color.a) + (bg_color.r * ainv)));

							(*fxpPutPixel)(&coords, &color);
						}
					}
				}
			} else {
				// 4bpp
				// NOTE: The fact that the fb stores two pixels per byte means we can't take any shortcut,
				//       because they may only apply to one of those two pixels...
				FBInkCoordinates coords   = { 0U };
				FBInkColor       bg_color = { 0U };
				size_t           pix_offset;
				FBInkPixelG8A    img_px;
				uint8_t          ainv = 0U;
				for (j = img_y_off; j < max_height; j++) {
					for (i = img_x_off; i < max_width; i++) {
						// We need to know what this pixel currently looks like in the framebuffer...
						coords.x = (unsigned short int) (i + x_off);
						coords.y = (unsigned short int) (j + y_off);
						// NOTE: We use the the function pointers directly, to avoid the OOB checks,
						//       because we know we're only processing on-screen pixels,
						//       and we don't care about the rotation checks at this bpp :).
						(*fxpGetPixel)(&coords, &bg_color);

						// NOTE: In this branch, req_n == 2, so we can do << 1 instead of * 2 ;).
						pix_offset = (size_t)(((j << 1U) * w) + (i << 1U));
#	pragma GCC diagnostic push
#	pragma GCC diagnostic ignored "-Wcast-align"
						// We gobble the full image pixel (all 2 bytes)
						img_px.p = *((uint16_t*) &data[pix_offset]);
#	pragma GCC diagnostic pop

						ainv = img_px.color.a ^ 0xFF;
						// Don't forget to honor inversion
						img_px.color.v ^= invert;
						// Blend it!
						color.r = (uint8_t) DIV255(
						    ((img_px.color.v * img_px.color.a) + (bg_color.r * ainv)));

						(*fxpPutPixel)(&coords, &color);
					}
				}
			}
		} else {
			// No alpha in image, or ignored
			size_t           pix_offset;
			FBInkCoordinates coords = { 0U };
			for (j = img_y_off; j < max_height; j++) {
				for (i = img_x_off; i < max_width; i++) {
					// NOTE: Here, req_n is either 2, or 1 if ignore_alpha, so, no shift trickery ;)
					pix_offset = (size_t)((j * req_n * w) + (i * req_n));
					color.r    = data[pix_offset] ^ invert;

					coords.x = (unsigned short int) (i + x_off);
					coords.y = (unsigned short int) (j + y_off);

					// NOTE: Again, use the function pointer directly, to skip redundant OOB checks,
					//       as well as unneeded rotation checks (can't happen at this bpp).
					(*fxpPutPixel)(&coords, &color);
				}
			}
		}
	} else if (fb_is_true_bgr) {
		// 24bpp & 32bpp
		if (!fbink_config->ignore_alpha && img_has_alpha) {
			FBInkPixelRGBA img_px;
			uint8_t        ainv = 0U;
			size_t         pix_offset;
			if (!fb_is_24bpp) {
				// 32bpp
				FBInkPixelBGRA fb_px;
				FBInkPixelBGRA bg_px;
				// This is essentially a constant in our case... (c.f., put_pixel_RGB32)
				fb_px.color.a = 0xFF;
				for (j = img_y_off; j < max_height; j++) {
					for (i = img_x_off; i < max_width; i++) {
						// NOTE: We should be able to skip rotation hacks at this bpp...

						// Yeah, I know, GCC...
						// NOTE: In this branch, req_n == 4, so we can do << 2 instead of * 4 ;).
						pix_offset = (size_t)(((j << 2U) * w) + (i << 2U));
#	pragma GCC diagnostic push
#	pragma GCC diagnostic ignored "-Wcast-align"
						// First, we gobble the full image pixel (all 4 bytes)
						img_px.p = *((uint32_t*) &data[pix_offset]);
#	pragma GCC diagnostic pop

						// Take a shortcut for the most common alpha values (none & full)
						if (img_px.color.a == 0xFF) {
							// Fully opaque, we can blit the image (almost) directly.
							// We do need to handle BGR and honor inversion ;).
							img_px.p ^= invert_rgb;
							fb_px.color.r = img_px.color.r;
							fb_px.color.g = img_px.color.g;
							fb_px.color.b = img_px.color.b;

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
							ainv = img_px.color.a ^ 0xFF;

							pix_offset =
							    (uint32_t)((unsigned short int) (i + x_off) << 2U) +
							    ((unsigned short int) (j + y_off) * fInfo.line_length);
#	pragma GCC diagnostic push
#	pragma GCC diagnostic ignored "-Wcast-align"
							// Again, read the full pixel from the framebuffer (all 4 bytes)
							bg_px.p = *((uint32_t*) (fbPtr + pix_offset));
#	pragma GCC diagnostic pop

							// Don't forget to honor inversion
							img_px.p ^= invert_rgb;
							// Blend it, we get our BGR swap in the process ;).
							fb_px.color.r = (uint8_t) DIV255(
							    ((img_px.color.r * img_px.color.a) + (bg_px.color.r * ainv)));
							fb_px.color.g = (uint8_t) DIV255(
							    ((img_px.color.g * img_px.color.a) + (bg_px.color.r * ainv)));
							fb_px.color.b = (uint8_t) DIV255(
							    ((img_px.color.b * img_px.color.a) + (bg_px.color.b * ainv)));

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
				FBInkPixelBGR bg_px;
				for (j = img_y_off; j < max_height; j++) {
					for (i = img_x_off; i < max_width; i++) {
						// NOTE: We should be able to skip rotation hacks at this bpp...

						// Yeah, I know, GCC...
						// NOTE: In this branch, req_n == 4, so we can do << 2 instead of * 4 ;).
						pix_offset = (size_t)(((j << 2U) * w) + (i << 2U));
#	pragma GCC diagnostic push
#	pragma GCC diagnostic ignored "-Wcast-align"
						// First, we gobble the full image pixel (all 4 bytes)
						img_px.p = *((uint32_t*) &data[pix_offset]);
#	pragma GCC diagnostic pop

						// Take a shortcut for the most common alpha values (none & full)
						if (img_px.color.a == 0xFF) {
							// Fully opaque, we can blit the image (almost) directly.
							// We do need to handle BGR and honor inversion ;).
							img_px.p ^= invert_rgb;
							fb_px.color.r = img_px.color.r;
							fb_px.color.g = img_px.color.g;
							fb_px.color.b = img_px.color.b;

							pix_offset =
							    (uint32_t)((unsigned short int) (i + x_off) << 2U) +
							    ((unsigned short int) (j + y_off) * fInfo.line_length);
							// And we write the full pixel to the fb (all 3 bytes)
							*((uint24_t*) (fbPtr + pix_offset)) = fb_px.p;
						} else if (img_px.color.a == 0) {
							// Transparent! Keep fb as-is.
						} else {
							// Alpha blending...
							ainv = img_px.color.a ^ 0xFF;

							pix_offset =
							    (uint32_t)((unsigned short int) (i + x_off) << 2U) +
							    ((unsigned short int) (j + y_off) * fInfo.line_length);
							// Again, read the full pixel from the framebuffer (all 3 bytes)
							bg_px.p = *((uint24_t*) (fbPtr + pix_offset));

							// Don't forget to honor inversion
							img_px.p ^= invert_rgb;
							// Blend it, we get our BGR swap in the process ;).
							fb_px.color.r = (uint8_t) DIV255(
							    ((img_px.color.r * img_px.color.a) + (bg_px.color.r * ainv)));
							fb_px.color.g = (uint8_t) DIV255(
							    ((img_px.color.g * img_px.color.a) + (bg_px.color.r * ainv)));
							fb_px.color.b = (uint8_t) DIV255(
							    ((img_px.color.b * img_px.color.a) + (bg_px.color.b * ainv)));

							// And we write the full blended pixel to the fb (all 3 bytes)
							*((uint24_t*) (fbPtr + pix_offset)) = fb_px.p;
						}
					}
				}
			}
		} else {
			// No alpha in image, or ignored
			size_t pix_offset;
			// We don't care about image alpha in this branch, so we don't even store it.
			FBInkPixelRGB img_px;
			if (!fb_is_24bpp) {
				// 32bpp
				FBInkPixelBGRA fb_px;
				// This is essentially a constant in our case...
				fb_px.color.a = 0xFF;
				for (j = img_y_off; j < max_height; j++) {
					for (i = img_x_off; i < max_width; i++) {
						// NOTE: Here, req_n is either 4, or 3 if ignore_alpha, so, no shift trickery ;)
						pix_offset = (size_t)((j * req_n * w) + (i * req_n));
						// Gobble the full image pixel (3 bytes, we don't care about alpha if it's there)
						img_px.p = *((uint24_t*) &data[pix_offset]);
						// NOTE: Given our typedef trickery, this exactly boils down to a 3 bytes memcpy:
						//memcpy(&img_px.p, &data[pix_offset], 3 * sizeof(uint8_t));

						// Handle BGR & inversion
						fb_px.color.r = img_px.color.r ^ invert;
						fb_px.color.g = img_px.color.g ^ invert;
						fb_px.color.b = img_px.color.b ^ invert;

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
				FBInkPixelBGR fb_px;
				for (j = img_y_off; j < max_height; j++) {
					for (i = img_x_off; i < max_width; i++) {
						// NOTE: Here, req_n is either 4, or 3 if ignore_alpha, so, no shift trickery ;)
						pix_offset = (size_t)((j * req_n * w) + (i * req_n));
						// Gobble the full image pixel (3 bytes, we don't care about alpha if it's there)
						img_px.p = *((uint24_t*) &data[pix_offset]);
						// NOTE: Given our typedef trickery, this exactly boils down to a 3 bytes memcpy:
						//memcpy(&img_px.p, &data[pix_offset], 3 * sizeof(uint8_t));

						// Handle BGR & inversion
						fb_px.color.r = img_px.color.r ^ invert;
						fb_px.color.g = img_px.color.g ^ invert;
						fb_px.color.b = img_px.color.b ^ invert;

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
		if (!fbink_config->ignore_alpha && img_has_alpha) {
			FBInkCoordinates coords   = { 0U };
			FBInkColor       bg_color = { 0U };
			size_t           pix_offset;
			FBInkPixelRGBA   img_px;
			uint8_t          ainv = 0U;
			for (j = img_y_off; j < max_height; j++) {
				for (i = img_x_off; i < max_width; i++) {
					// NOTE: Same general idea as the fb_is_grayscale case,
					//       except at this bpp we then have to handle rotation ourselves...
					// NOTE: In this branch, req_n == 4, so we can do << 2 instead of * 4 ;).
					pix_offset = (size_t)(((j << 2U) * w) + (i << 2U));
#	pragma GCC diagnostic push
#	pragma GCC diagnostic ignored "-Wcast-align"
					// Gobble the full image pixel (all 4 bytes)
					img_px.p = *((uint32_t*) &data[pix_offset]);
#	pragma GCC diagnostic pop

					// Take a shortcut for the most common alpha values (none & full)
					if (img_px.color.a == 0xFF) {
						// Fully opaque, we can blit the image (almost) directly.
						// We do need to handle BGR and honor inversion ;).
						img_px.p ^= invert_rgb;
						color.r = img_px.color.r;
						color.g = img_px.color.g;
						color.b = img_px.color.b;

						coords.x = (unsigned short int) (i + x_off);
						coords.y = (unsigned short int) (j + y_off);
						(*fxpRotateCoords)(&coords);
						(*fxpPutPixel)(&coords, &color);
					} else if (img_px.color.a == 0) {
						// Transparent! Keep fb as-is.
					} else {
						// Alpha blending...
						ainv = img_px.color.a ^ 0xFF;

						coords.x = (unsigned short int) (i + x_off);
						coords.y = (unsigned short int) (j + y_off);
						(*fxpRotateCoords)(&coords);
						(*fxpGetPixel)(&coords, &bg_color);

						// Don't forget to honor inversion
						img_px.p ^= invert_rgb;
						// Blend it, we get our BGR swap in the process ;).
						color.r = (uint8_t) DIV255(
						    ((img_px.color.r * img_px.color.a) + (bg_color.r * ainv)));
						color.g = (uint8_t) DIV255(
						    ((img_px.color.g * img_px.color.a) + (bg_color.r * ainv)));
						color.b = (uint8_t) DIV255(
						    ((img_px.color.b * img_px.color.a) + (bg_color.b * ainv)));

						(*fxpPutPixel)(&coords, &color);
					}
				}
			}
		} else {
			// No alpha in image, or ignored
			size_t           pix_offset;
			FBInkCoordinates coords = { 0U };
			// NOTE: For some reason, reading the image 3 or 4 bytes at once doesn't win us anything, here...
			for (j = img_y_off; j < max_height; j++) {
				for (i = img_x_off; i < max_width; i++) {
					// NOTE: Here, req_n is either 4, or 3 if ignore_alpha, so, no shift trickery ;)
					pix_offset = (size_t)((j * req_n * w) + (i * req_n));
					color.r    = data[pix_offset + 0] ^ invert;
					color.g    = data[pix_offset + 1] ^ invert;
					color.b    = data[pix_offset + 2] ^ invert;

					coords.x = (unsigned short int) (i + x_off);
					coords.y = (unsigned short int) (j + y_off);
					// NOTE: Again, we can only skip the OOB checks at this bpp.
					(*fxpRotateCoords)(&coords);
					(*fxpPutPixel)(&coords, &color);
				}
			}
		}
	}

	// Rotate the region if need be...
	if (deviceQuirks.isKobo16Landscape) {
		rotate_region(&region);
	}

	// Fudge the region if we asked for a screen clear, so that we actually refresh the full screen...
	if (fbink_config->is_cleared) {
		fullscreen_region(&region);
	}

	// Refresh screen
	if (refresh(fbfd, region, WAVEFORM_MODE_GC16, fbink_config->is_flashing) != EXIT_SUCCESS) {
		fprintf(stderr, "[FBInk] Failed to refresh the screen!\n");
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
    fbink_print_image(int fbfd    UNUSED_BY_MINIMAL,
		      const char* filename UNUSED_BY_MINIMAL,
		      short int x_off UNUSED_BY_MINIMAL,
		      short int y_off    UNUSED_BY_MINIMAL,
		      const FBInkConfig* fbink_config UNUSED_BY_MINIMAL)
{
#ifdef FBINK_WITH_IMAGE
	// Assume success, until shit happens ;)
	int rv = EXIT_SUCCESS;

	// Let stbi handle grayscaling for us
	int req_n;
	switch (vInfo.bits_per_pixel) {
		case 4U:
			req_n = 1 + !fbink_config->ignore_alpha;
			break;
		case 8U:
			req_n = 1 + !fbink_config->ignore_alpha;
			break;
		case 16U:
			req_n = 3 + !fbink_config->ignore_alpha;
			break;
		case 24U:
			req_n = 3 + !fbink_config->ignore_alpha;
			break;
		case 32U:
		default:
			req_n = 3 + !fbink_config->ignore_alpha;
			break;
	}

	// Decode image via stbi
	unsigned char* data = NULL;
	int            w;
	int            h;
	int            n;
	data = img_load_from_file(filename, &w, &h, &n, req_n);
	if (data == NULL) {
		fprintf(stderr, "[FBInk] Failed to decode image data from '%s'!\n", filename);
		return ERRCODE(EXIT_FAILURE);
	}

	// Finally, draw it on screen
	if (draw_image(fbfd, data, w, h, n, req_n, x_off, y_off, fbink_config) != EXIT_SUCCESS) {
		fprintf(stderr, "[FBInk] Failed to display image data on screen!\n");
		rv = ERRCODE(EXIT_FAILURE);
		goto cleanup;
	}

	// Cleanup
cleanup:
	// Free the buffer holding our decoded image data
	stbi_image_free(data);

	return rv;
#else
	fprintf(stderr, "[FBInk] Image support is disabled in this FBInk build!\n");
	return ERRCODE(ENOSYS);
#endif    // FBINK_WITH_IMAGE
}

// Draw raw (supposedly image) data on screen
int
    fbink_print_raw_data(int fbfd       UNUSED_BY_MINIMAL,
			 unsigned char* data UNUSED_BY_MINIMAL,
			 const int w UNUSED_BY_MINIMAL,
			 const int h  UNUSED_BY_MINIMAL,
			 const size_t len UNUSED_BY_MINIMAL,
			 short int x_off UNUSED_BY_MINIMAL,
			 short int y_off    UNUSED_BY_MINIMAL,
			 const FBInkConfig* fbink_config UNUSED_BY_MINIMAL)
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
			req_n = 1 + !fbink_config->ignore_alpha;
			break;
		case 8U:
			req_n = 1 + !fbink_config->ignore_alpha;
			break;
		case 16U:
			req_n = 3 + !fbink_config->ignore_alpha;
			break;
		case 24U:
			req_n = 3 + !fbink_config->ignore_alpha;
			break;
		case 32U:
		default:
			req_n = 3 + !fbink_config->ignore_alpha;
			break;
	}

	// Devising the actual amount of components in the input should be as easy as that...
	int n = (int) len / h / w;

	LOG("Requested %d color channels, supplied data had %d", req_n, n);

	// If there's a mismatch between the components in the input data vs. what the fb expects,
	// re-interleave the data w/ stbi's help...
	unsigned char* imgdata = NULL;
	if (req_n != n) {
		LOG("Converting from %d components to the requested %d", n, req_n);
		// NOTE: stbi__convert_format will *always* free the input buffer, which we do NOT want here...
		//       Which is why we're using a tweaked internal copy, which does not free ;).
		imgdata = img_convert_px_format(data, n, req_n, w, h);
		if (imgdata == NULL) {
			fprintf(stderr, "[FBInk] Failed to re-interleave input data in a suitable format!\n");
			rv = ERRCODE(EXIT_FAILURE);
			goto cleanup;
		}
	} else {
		// We can use the input buffer as-is :)
		LOG("No conversion needed, using the input buffer directly");
		imgdata = data;
	}

	// We should now be able to draw that on screen, knowing that it probably won't horribly implode ;p
	if (draw_image(fbfd, imgdata, w, h, n, req_n, x_off, y_off, fbink_config) != EXIT_SUCCESS) {
		fprintf(stderr, "[FBInk] Failed to display image data on screen!\n");
		rv = ERRCODE(EXIT_FAILURE);
		goto cleanup;
	}

	// Cleanup
cleanup:
	// If we created an intermediary buffer ourselves, free it.
	if (req_n != n) {
		stbi_image_free(imgdata);
	}

	return rv;
#else
	fprintf(stderr, "[FBInk] Image support is disabled in this FBInk build!\n");
	return ERRCODE(ENOSYS);
#endif    // FBINK_WITH_IMAGE
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
// Various other small fonts (c.f., CREDITS for details)
#	include "fbink_misc_fonts.c"
#endif
// Contains fbink_button_scan's implementation, Kobo only, and has a bit of Linux MT input thrown in ;).
#include "fbink_button_scan.c"
