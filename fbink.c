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

#include "fbink.h"

#include "fbink_internal.h"

// Return the library version as devised at library compile-time
const char*
    fbink_version(void)
{
	return FBINK_VERSION;
}

// Helper function to 'plot' a pixel in given color
static void
    put_pixel_Gray4(FBInkCoordinates* coords, unsigned short int c)
{
	// calculate the pixel's byte offset inside the buffer
	size_t pix_offset = coords->x / 2 + coords->y * finfo.line_length;

	// now this is about the same as 'fbp[pix_offset] = value'
	*((unsigned char*) (g_fbink_fbp + pix_offset)) = (unsigned char) c;
}

static void
    put_pixel_Gray8(FBInkCoordinates* coords, unsigned short int c)
{
	// calculate the pixel's byte offset inside the buffer
	size_t pix_offset = coords->x + coords->y * finfo.line_length;

	// now this is about the same as 'fbp[pix_offset] = value'
	*((unsigned char*) (g_fbink_fbp + pix_offset)) = (unsigned char) c;
}

static void
    put_pixel_RGB24(FBInkCoordinates* coords, unsigned short int r, unsigned short int g, unsigned short int b)
{
	// calculate the pixel's byte offset inside the buffer
	// note: x * 3 as every pixel is 3 consecutive bytes
	size_t pix_offset = coords->x * 3U + coords->y * finfo.line_length;

	// now this is about the same as 'fbp[pix_offset] = value'
	*((unsigned char*) (g_fbink_fbp + pix_offset))     = (unsigned char) b;
	*((unsigned char*) (g_fbink_fbp + pix_offset + 1)) = (unsigned char) g;
	*((unsigned char*) (g_fbink_fbp + pix_offset + 2)) = (unsigned char) r;
}

static void
    put_pixel_RGB32(FBInkCoordinates* coords, unsigned short int r, unsigned short int g, unsigned short int b)
{
	// calculate the pixel's byte offset inside the buffer
	// note: x * 4 as every pixel is 4 consecutive bytes
	size_t pix_offset = coords->x * 4U + coords->y * finfo.line_length;

	// now this is about the same as 'fbp[pix_offset] = value'
	*((unsigned char*) (g_fbink_fbp + pix_offset))     = (unsigned char) b;
	*((unsigned char*) (g_fbink_fbp + pix_offset + 1)) = (unsigned char) g;
	*((unsigned char*) (g_fbink_fbp + pix_offset + 2)) = (unsigned char) r;
	*((unsigned char*) (g_fbink_fbp + pix_offset + 3)) = 0xFF;    // Opaque, always.
}

static void
    put_pixel_RGB565(FBInkCoordinates* coords, unsigned short int r, unsigned short int g, unsigned short int b)
{
	// calculate the pixel's byte offset inside the buffer
	// note: x * 2 as every pixel is 2 consecutive bytes
	size_t pix_offset = coords->x * 2U + coords->y * finfo.line_length;

#pragma GCC diagnostic push
	// now this is about the same as 'fbp[pix_offset] = value'
	// but a bit more complicated for RGB565
#pragma GCC diagnostic ignored "-Wconversion"
	unsigned short int c = ((r / 8U) << 11U) + ((g / 4U) << 5U) + (b / 8U);
	// or: c = ((r / 8) * 2048) + ((g / 4) * 32) + (b / 8);
	// write 'two bytes at once', much to GCC's dismay...
#pragma GCC diagnostic ignored "-Wcast-align"
	*((unsigned short int*) (g_fbink_fbp + pix_offset)) = c;
#pragma GCC diagnostic pop
}

// Handle rotation quirks...
static void
    get_physical_coords(FBInkCoordinates* coords)
{
	unsigned short int rx = coords->y;
	unsigned short int ry = viewHeight - coords->x - 1;

	coords->x = rx;
	coords->y = ry;
}

// Handle various bpp...
static void
    put_pixel(unsigned short int x, unsigned short int y, unsigned short int c)
{
	// NOTE: Discard off-screen pixels!
	//       For instance, when we have a halfcell offset in conjunction with a !isPerfectFit pixel offset,
	//       when we're padding and centering, the final whitespace of right-padding will have its last
	//       few pixels (the exact amount being half of the dead zone width) pushed off-screen...
	if (x >= vinfo.xres || y >= vinfo.yres) {
		//LOG("Discarding off-screen pixel @ %u, %u (out of %ux%u bounds)", x, y, vinfo.xres, vinfo.yres);
		return;
	}
#ifdef FBINK_FOR_LEGACY
	// NOTE: Legacy devices all have an inverted palette.
	c = c ^ WHITE;
#endif

	FBInkCoordinates coords = { x, y };
	if (deviceQuirks.isKindleOasis2) {
		get_physical_coords(&coords);
	}

	if (vinfo.bits_per_pixel == 4U) {
		put_pixel_Gray4(&coords, def_b[c]);
	} else if (vinfo.bits_per_pixel == 8U) {
		// NOTE: Grayscale palette, we could have used def_r or def_g ;).
		put_pixel_Gray8(&coords, def_b[c]);
	} else if (vinfo.bits_per_pixel == 16U) {
		put_pixel_RGB565(&coords, def_r[c], def_g[c], def_b[c]);
	} else if (vinfo.bits_per_pixel == 24U) {
		put_pixel_RGB24(&coords, def_r[c], def_g[c], def_b[c]);
	} else if (vinfo.bits_per_pixel == 32U) {
		put_pixel_RGB32(&coords, def_r[c], def_g[c], def_b[c]);
	}
}

// Helper function to draw a rectangle in given color
static void
    fill_rect(unsigned short int x,
	      unsigned short int y,
	      unsigned short int w,
	      unsigned short int h,
	      unsigned short int c)
{
	FBInkCoordinates coords = { x, y };
	if (deviceQuirks.isKindleOasis2) {
		get_physical_coords(&coords);
	}

	unsigned short int cx;
	unsigned short int cy;
	for (cy = 0U; cy < h; cy++) {
		for (cx = 0U; cx < w; cx++) {
			put_pixel((unsigned short int) (coords.x + cx), (unsigned short int) (coords.y + cy), c);
		}
	}
	LOG("Filled a %hux%hu rectangle @ %hu, %hu", w, h, coords.x, coords.y);
}

// Helper function to clear the screen - fill whole screen with given color
static void
    clear_screen(unsigned short int c)
{
#ifdef FBINK_FOR_LEGACY
	// NOTE: Legacy devices all have an inverted palette.
	c = c ^ WHITE;
#endif

	// NOTE: Grayscale palette, we could have used def_r or def_g ;).
	memset(g_fbink_fbp, def_b[c], finfo.smem_len);
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
		fprintf(stderr, "[FBInk] Codepoint U+%04X is not covered by this font!\n", codepoint);
		return font8x8_basic[0];
	}
}

// Render a specific font8x8 glyph into a pixmap
// (base size: 8x8, scaled by a factor of FONTSIZE_MULT, which varies depending on screen resolution)
static void
    font8x8_render(uint32_t codepoint, unsigned char* glyph_pixmap, unsigned short int fontname UNUSED_BY_MINIMAL)
{
	const unsigned char* bitmap = NULL;

	// Do we have Unscii fonts compiled in?
#ifdef FBINK_WITH_UNSCII
	switch (fontname) {
		case UNSCII:
			bitmap = unscii_get_bitmap(codepoint);
			break;
		case UNSCII_ALT:
			bitmap = alt_get_bitmap(codepoint);
			break;
		case UNSCII_THIN:
			bitmap = thin_get_bitmap(codepoint);
			break;
		case UNSCII_FANTASY:
			bitmap = fantasy_get_bitmap(codepoint);
			break;
		case UNSCII_MCR:
			bitmap = mcr_get_bitmap(codepoint);
			break;
		case UNSCII_TALL:
			bitmap = tall_get_bitmap(codepoint);
			break;
		case IBM:
		default:
			bitmap = font8x8_get_bitmap(codepoint);
			break;
	}
#else
	bitmap = font8x8_get_bitmap(codepoint);
#endif

	unsigned short int x;
	unsigned short int y;
	unsigned short int i;
	unsigned short int j;
	unsigned short int k;
	bool               set = false;
	for (i = 0U; i < FONTH; i++) {
		// x: input row, i: output row
		x = i / FONTSIZE_MULT;
		for (j = 0U; j < FONTW; j++) {
			// y: input column, j: output column
			y   = j / FONTSIZE_MULT;
			set = bitmap[x] & 1 << y;
			// 'Flatten' our pixmap into a 1D array (0=0,0; 1=0,1; 2=0,2; FONTW=1,0)
			unsigned short int idx = (unsigned short int) (j + (i * FONTW));
			for (k = 0U; k < FONTSIZE_MULT; k++) {
				glyph_pixmap[idx + k] = set ? 1 : 0;
			}
		}
	}
}

// Helper function for drawing
static struct mxcfb_rect
    draw(const char*        text,
	 unsigned short int row,
	 unsigned short int col,
	 bool               is_inverted,
	 bool               is_centered,
	 unsigned short int multiline_offset,
	 unsigned short int fontname,
	 bool               halfcell_offset)
{
	LOG("Printing '%s' @ line offset %hu (meaning row %d)", text, multiline_offset, row + multiline_offset);
	unsigned short int fgC = is_inverted ? WHITE : BLACK;
	unsigned short int bgC = is_inverted ? BLACK : WHITE;

	unsigned short int x;
	unsigned short int y;
	// Adjust row in case we're a continuation of a multi-line print...
	row = (unsigned short int) (row + multiline_offset);

	// Compute the length of our actual string
	// NOTE: We already took care in fbink_print() of making sure that the string passed in text wouldn't take up
	//       more space (as in columns, not bytes) than (MAXCOLS - col), the maximum printable length.
	//       And as we're printing glyphs, we need to iterate over the number of characters/grapheme clusters,
	//       not bytes.
	unsigned int charcount = u8_strlen(text);
	LOG("Character count: %u (over %zu bytes)", charcount, strlen(text));

	// Compute our actual subcell offset in pixels
	unsigned short int pixel_offset = 0U;
	// Do we have a centering induced halfcell adjustment to correct?
	if (halfcell_offset) {
		pixel_offset = FONTW / 2U;
		LOG("Incrementing pixel_offset by %u pixels to account for a halfcell centering tweak", pixel_offset);
	}
	// Do we have a permanent adjustment to make because of dead space on the right edge?
	if (!deviceQuirks.isPerfectFit) {
		// We correct by half of said dead space, since we want perfect centering ;).
		unsigned short int deadzone_offset =
		    (unsigned short int) (viewWidth - (unsigned short int) (MAXCOLS * FONTW)) / 2U;
		pixel_offset = (unsigned short int) (pixel_offset + deadzone_offset);
		LOG("Incrementing pixel_offset by %u pixels to compensate for dead space on the right edge",
		    deadzone_offset);
	}

	// Compute the dimension of the screen region we'll paint to (taking multi-line into account)
	struct mxcfb_rect region = {
		.top    = (uint32_t)((row - multiline_offset) * FONTH),
		.left   = (uint32_t)(col * FONTW),
		.width  = multiline_offset > 0U ? (viewWidth - (uint32_t)(col * FONTW)) : (uint32_t)(charcount * FONTW),
		.height = (uint32_t)((multiline_offset + 1U) * FONTH),
	};

	LOG("Region: top=%u, left=%u, width=%u, height=%u", region.top, region.left, region.width, region.height);

	// Do we have a pixel offset to honor?
	if (pixel_offset > 0U) {
		LOG("Moving pen %u pixels to the right to honor subcell centering adjustments", pixel_offset);
		// NOTE: We need to update the start of our region rectangle if it doesn't already cover the full line,
		//       i.e., when we're not padding + centering.
		// NOTE: Make sure we don't mess with multiline strings,
		//       because by definition left has to keep matching the value of the first line,
		//       even on subsequent lines.
		if (charcount != MAXCOLS && multiline_offset == 0U) {
			region.left += pixel_offset;
			LOG("Updated region.left to %u", region.left);
		}
	}

	// If we're a full line, we need to fill the space that honoring our offset has left vacant on the left edge...
	// NOTE: Do it also when we're a left-aligned uncentered multiline string, no matter the length of the line,
	//       so the final line matches the previous ones, which fell under the charcount == MAXCOLS case,
	//       while the final one would not if it doesn't fill the line, too ;).
	if ((charcount == MAXCOLS || (col == 0 && !is_centered && multiline_offset > 0U)) && pixel_offset > 0U) {
		LOG("Painting a background rectangle on the left edge on account of pixel_offset");
		fill_rect(0,
			  (unsigned short int) (region.top + (unsigned short int) (multiline_offset * FONTH)),
			  pixel_offset,
			  FONTH,
			  bgC);
		// Correct width, to include that bit of content, too, if needed
		if (region.width < vinfo.xres) {
			region.width += pixel_offset;
			// And make sure it's properly clamped, because we can't necessarily rely on left & width
			// being entirely acurate either because of the multiline print override,
			// or because of a bit of subcell placement overshoot trickery (c.f., comment in put_pixel).
			if (region.width + region.left > vinfo.xres) {
				region.width = vinfo.xres - region.left;
				LOG("Clamped region.width to %u", region.width);
			} else {
				LOG("Updated region.width to %u", region.width);
			}
		}
	}

	// NOTE: In some cases, we also have a matching hole to patch on the right side...
	//       This only applies when pixel_offset *only* accounts for the !isPerfectFit adjustment though,
	//       because in every other case, the halfcell offset handling neatly pushes everything into place ;).
	if (charcount == MAXCOLS && !deviceQuirks.isPerfectFit && !halfcell_offset) {
		// NOTE: !isPerfectFit ensures pixel_offset is non-zero
		LOG("Painting a background rectangle to fill the dead space on the right edge");
		fill_rect((unsigned short int) (viewWidth - pixel_offset),
			  (unsigned short int) (region.top + (unsigned short int) (multiline_offset * FONTH)),
			  pixel_offset,
			  FONTH,
			  bgC);
		// If it's not already the case, update region to the full width,
		// because we've just plugged a hole at the very right edge of a full line.
		if (region.width < vinfo.xres) {
			region.width = vinfo.xres;
			LOG("Updated region.width to %u", region.width);
		}
	}

	// NOTE: In case of a multi-line centered print, we can't really trust the final col,
	//       it might be significantly different than the others, and as such, we'd be computing a cropped region.
	//       Make the region cover the full width of the screen to make sure we won't miss anything.
	if (multiline_offset > 0U && is_centered && (region.left > 0U || region.width < vinfo.xres)) {
		region.left  = 0U;
		region.width = vinfo.xres;
		LOG("Enforced region.left to %u & region.width to %u because of multi-line centering",
		    region.left,
		    region.width);
	}

	// Fill our bounding box with our background color, so that we'll be visible no matter what's already on screen.
	// NOTE: Unneeded, we already plot the background when handling font glyphs ;).
	//fill_rect(region.left, region.top, region.width, region.height, bgC);

	// Alloc our pixmap on the stack, and re-use it.
	// NOTE: We tried using automatic VLAs, but that... didn't go well.
	//       (as in, subtle (or not so) memory and/or stack corruption).
	unsigned char* pixmap = NULL;
	// NOTE: Using alloca may prevent inlining. That said, trust that the compiler will do the right thing.
	//       As for why alloca:
	//       It's a very small allocation, we'll always fully write to it so we don't care about its initialization,
	//       -> it's a perfect fit for the stack.
	//       In any other situation (i.e., constant FONTW & FONTH), it'd have been an automatic.
	// NOTE: Don't forget the extra FONTSIZE_MULT - 1 to avoid a buffer overflow when scaling the last pixel...
	pixmap = alloca(sizeof(*pixmap) * (size_t)((FONTW * FONTH) + FONTSIZE_MULT - 1));

	// Loop through all the *characters* in the text string
	unsigned int       bi = 0U;
	unsigned short int ci = 0U;
	uint32_t           ch = 0U;
	while ((ch = u8_nextchar(text, &bi)) != 0U) {
		LOG("Char %u (@ %u) out of %u is @ byte offset %u and is U+%04X", ci + 1U, ci, charcount, bi, ch);

		// Get the glyph's pixmap
		font8x8_render(ch, pixmap, fontname);

		// loop through pixel rows
		for (y = 0U; y < FONTH; y++) {
			// loop through pixel columns
			for (x = 0U; x < FONTW; x++) {
				// get the pixel value
				unsigned char b = pixmap[(y * FONTW) + x];
				if (b > 0) {
					// plot the pixel (fg, text)
					// NOTE: This is where we used to fudge positioning of hex fonts converted by
					//       tools/hextoc.py before I figured out the root issue ;).
					put_pixel(
					    (unsigned short int) ((col * FONTW) + (ci * FONTW) + x + pixel_offset),
					    (unsigned short int) ((row * FONTH) + y),
					    fgC);
				} else {
					// this is background,
					// fill it so that we'll be visible no matter what was on screen behind us.
					put_pixel(
					    (unsigned short int) ((col * FONTW) + (ci * FONTW) + x + pixel_offset),
					    (unsigned short int) ((row * FONTH) + y),
					    bgC);
				}
			}    // end "for x"
		}            // end "for y"
		// Next glyph! This serves as the source for the pen position, hence it being used as an index...
		ci++;
	}

	return region;
}

// Handle the various eInk update API quirks for the full range of HW we support...

#ifdef FBINK_FOR_LEGACY
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

	int rv;
	rv = ioctl(fbfd, FBIO_EINK_UPDATE_DISPLAY_AREA, &area);

	if (rv < 0) {
		// NOTE: perror() is not thread-safe...
		char  buf[256];
		char* errstr = strerror_r(errno, buf, sizeof(buf));
		fprintf(stderr, "[FBInk] FBIO_EINK_UPDATE_DISPLAY_AREA: %s\n", errstr);
		return EXIT_FAILURE;
	}

	return rv;
}

#else
// NOTE: Small helper function to aid with logging the exact amount of time MXCFB_WAIT_FOR_UPDATE_COMPLETE blocked...
//       The driver is using the Kernel's wait-for-completion handler,
//       which returns the amount of jiffies left until the timeout set by the caller.
//       As we don't give a rat's ass about jiffies, we need to convert 'em to milliseconds.
// NOTE: The ioctl often actually blocks slightly longer than the perceived speed of the eInk refresh. Nevertheless,
//       there is a direct correlation between the two, as can be shown by switching between waveform modes...
static long int
    jiffies_to_ms(long int jiffies)
{
	// We need the Kernel's HZ value for this, which we stored in USER_HZ during fbink_init ;).
	return (jiffies * 1000 / USER_HZ);
}

#	ifdef FBINK_FOR_KINDLE
// Touch Kindle devices ([K5<->]KOA2)
static int
    refresh_kindle(int fbfd,
		   const struct mxcfb_rect region,
		   uint32_t waveform_mode,
		   uint32_t update_mode,
		   uint32_t marker)
{
	struct mxcfb_update_data update = {
		.update_region = region,
		.waveform_mode = waveform_mode,
		.update_mode = update_mode,
		.update_marker = marker,
		.hist_bw_waveform_mode = WAVEFORM_MODE_DU,
		.hist_gray_waveform_mode = WAVEFORM_MODE_GC16_FAST,
		.temp = TEMP_USE_AUTO,
		.flags = 0U,
		.alt_buffer_data = { 0U },
	};

	int rv;
	rv = ioctl(fbfd, MXCFB_SEND_UPDATE, &update);

	if (rv < 0) {
		char buf[256];
		char* errstr = strerror_r(errno, buf, sizeof(buf));
		fprintf(stderr, "[FBInk] MXCFB_SEND_UPDATE: %s\n", errstr);
		return EXIT_FAILURE;
	}

	if (update_mode == UPDATE_MODE_FULL) {
		if (deviceQuirks.isKindlePearlScreen) {
			rv = ioctl(fbfd, MXCFB_WAIT_FOR_UPDATE_COMPLETE_PEARL, &marker);
		} else {
			struct mxcfb_update_marker_data update_marker = {
				.update_marker = marker,
				.collision_test = 0U,
			};

			rv = ioctl(fbfd, MXCFB_WAIT_FOR_UPDATE_COMPLETE, &update_marker);
		}

		if (rv < 0) {
			char buf[256];
			char* errstr = strerror_r(errno, buf, sizeof(buf));
			if (deviceQuirks.isKindlePearlScreen) {
				fprintf(stderr, "[FBInk] MXCFB_WAIT_FOR_UPDATE_COMPLETE_PEARL: %s\n", errstr);
			} else {
				fprintf(stderr, "[FBInk] MXCFB_WAIT_FOR_UPDATE_COMPLETE: %s\n", errstr);
			}
			return EXIT_FAILURE;
		} else {
			// NOTE: Timeout is set to 5000ms
			LOG("Waited %ldms for completion of flashing update %u", (5000 - jiffies_to_ms(rv)), marker);
		}
	}

	return EXIT_SUCCESS;
}

// Kindle Oasis 2 ([KOA2<->??)
static int
    refresh_kindle_koa2(int fbfd,
			const struct mxcfb_rect region,
			uint32_t waveform_mode,
			uint32_t update_mode,
			uint32_t marker)
{
	struct mxcfb_update_data_koa2 update = {
		.update_region = region,
		.waveform_mode = waveform_mode,
		.update_mode = update_mode,
		.update_marker = marker,
		.temp = TEMP_USE_AMBIENT,
		.flags = (waveform_mode == WAVEFORM_MODE_DU) ? EPDC_FLAG_FORCE_MONOCHROME : 0U,
		.dither_mode = EPDC_FLAG_USE_DITHERING_PASSTHROUGH,
		.quant_bit = 0,
		.alt_buffer_data = { 0U },
		.hist_bw_waveform_mode = WAVEFORM_MODE_DU,
		.hist_gray_waveform_mode = WAVEFORM_MODE_GC16,
		.ts_pxp = 0U,
		.ts_epdc = 0U,
	};

	int rv;
	rv = ioctl(fbfd, MXCFB_SEND_UPDATE_KOA2, &update);

	if (rv < 0) {
		char buf[256];
		char* errstr = strerror_r(errno, buf, sizeof(buf));
		fprintf(stderr, "[FBInk] MXCFB_SEND_UPDATE_KOA2: %s\n", errstr);
		return EXIT_FAILURE;
	}

	if (update_mode == UPDATE_MODE_FULL) {
		struct mxcfb_update_marker_data update_marker = {
			.update_marker = marker,
			.collision_test = 0U,
		};

		rv = ioctl(fbfd, MXCFB_WAIT_FOR_UPDATE_COMPLETE, &update_marker);

		if (rv < 0) {
			char buf[256];
			char* errstr = strerror_r(errno, buf, sizeof(buf));
			fprintf(stderr, "[FBInk] MXCFB_WAIT_FOR_UPDATE_COMPLETE: %s\n", errstr);
			return EXIT_FAILURE;
		} else {
			// NOTE: Timeout is set to 5000ms
			LOG("Waited %ldms for completion of flashing update %u", (5000 - jiffies_to_ms(rv)), marker);
		}
	}

	return EXIT_SUCCESS;
}

#	else
// Kobo devices ([Mk3<->Mk6])
static int
    refresh_kobo(int                     fbfd,
		 const struct mxcfb_rect region,
		 uint32_t                waveform_mode,
		 uint32_t                update_mode,
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

	int rv;
	rv = ioctl(fbfd, MXCFB_SEND_UPDATE_V1_NTX, &update);

	if (rv < 0) {
		char  buf[256];
		char* errstr = strerror_r(errno, buf, sizeof(buf));
		fprintf(stderr, "[FBInk] MXCFB_SEND_UPDATE_V1_NTX: %s\n", errstr);
		return EXIT_FAILURE;
	}

	if (update_mode == UPDATE_MODE_FULL) {
		rv = ioctl(fbfd, MXCFB_WAIT_FOR_UPDATE_COMPLETE_V1, &marker);

		if (rv < 0) {
			char  buf[256];
			char* errstr = strerror_r(errno, buf, sizeof(buf));
			fprintf(stderr, "[FBInk] MXCFB_WAIT_FOR_UPDATE_COMPLETE_V1: %s\n", errstr);
			return EXIT_FAILURE;
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
		// FIXME: Here be dragons! (REGAL flag, dither_mode & quant_bit)
		.flags = (waveform_mode == WAVEFORM_MODE_REAGLD)
			     ? EPDC_FLAG_USE_REGAL
			     : (waveform_mode == WAVEFORM_MODE_A2) ? EPDC_FLAG_FORCE_MONOCHROME : 0U,
		.dither_mode     = EPDC_FLAG_USE_DITHERING_PASSTHROUGH,
		.quant_bit       = 7,
		.alt_buffer_data = { 0U },
	};

	int rv;
	rv = ioctl(fbfd, MXCFB_SEND_UPDATE_V2, &update);

	if (rv < 0) {
		char  buf[256];
		char* errstr = strerror_r(errno, buf, sizeof(buf));
		fprintf(stderr, "[FBInk] MXCFB_SEND_UPDATE_V2: %s\n", errstr);
		return EXIT_FAILURE;
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
			return EXIT_FAILURE;
		} else {
			// NOTE: Timeout is set to 5000ms
			LOG("Waited %ldms for completion of flashing update %u", (5000 - jiffies_to_ms(rv)), marker);
		}
	}

	return EXIT_SUCCESS;
}
#	endif    // FBINK_FOR_KINDLE
#endif            // FBINK_FOR_LEGACY

// And finally, dispatch the right refresh request for our HW...
static int
    refresh(int fbfd, const struct mxcfb_rect region, uint32_t waveform_mode UNUSED_BY_LEGACY, bool is_flashing)
{
	// NOTE: Discard bogus regions, they can cause a softlock on some devices.
	//       A 0x0 region is a no go on most devices, while a 1x1 region may only upset some Kindle models.
	if (region.width <= 1 && region.height <= 1) {
		fprintf(stderr,
			"[FBInk] Discarding bogus empty region (%ux%u) to avoid a softlock.\n",
			region.width,
			region.height);
		return EXIT_FAILURE;
	}

#ifdef FBINK_FOR_LEGACY
	return refresh_legacy(fbfd, region, is_flashing);
#else
	// NOTE: While we'd be perfect candidates for using A2 waveform mode, it's all kinds of fucked up on Kobos,
	//       and may lead to disappearing text or weird blending depending on the surrounding fb content...
	//       It only shows up properly when FULL, which isn't great...
	// NOTE: And while we're on the fun quirks train: FULL never flashes w/ AUTO on (some?) Kobos,
	//       so request GC16 if we want a flash...
	// NOTE: FWIW, DU behaves properly when PARTIAL, but doesn't flash when FULL.
	//       Which somewhat tracks given AUTO's behavior on Kobos, as well as on Kindles.
	//       (i.e., DU or GC16 is most likely often what AUTO will land on).

	// So, handle this common switcheroo here...
	uint32_t wfm = (is_flashing && waveform_mode == WAVEFORM_MODE_AUTO) ? WAVEFORM_MODE_GC16 : waveform_mode;
	uint32_t upm = is_flashing ? UPDATE_MODE_FULL : UPDATE_MODE_PARTIAL;
	uint32_t marker = (uint32_t) getpid();

	// NOTE: Make sure update_marker is valid, an invalid marker *may* hang the kernel instead of failing gracefully,
	//       depending on the device/FW...
	if (marker == 0U) {
		marker = (70U + 66U + 73U + 78U + 75U);
	}

#	ifdef FBINK_FOR_KINDLE
	if (deviceQuirks.isKindleOasis2) {
		return refresh_kindle_koa2(fbfd, region, wfm, upm, marker);
	} else {
		return refresh_kindle(fbfd, region, wfm, upm, marker);
	}
#	else
	if (deviceQuirks.isKoboMk7) {
		return refresh_kobo_mk7(fbfd, region, wfm, upm, marker);
	} else {
		return refresh_kobo(fbfd, region, wfm, upm, marker);
	}
#	endif    // FBINK_FOR_KINDLE
#endif            // FBINK_FOR_LEGACY
}

// Open the framebuffer file & return the opened fd
int
    fbink_open(void)
{
	int fbfd = -1;

	// Open the framebuffer file for reading and writing
	fbfd = open("/dev/fb0", O_RDWR);
	if (!fbfd) {
		fprintf(stderr, "[FBInk] Error: cannot open framebuffer device.\n");
		return EXIT_FAILURE;
	}

	return fbfd;
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
	if (fbfd == -1) {
		// If we're opening a fd now, don't keep it around.
		keep_fd = false;
		if (-1 == (fbfd = fbink_open())) {
			fprintf(stderr, "[FBInk] Failed to open the framebuffer, aborting . . .\n");
			return EXIT_FAILURE;
		}
	}

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

	// Get variable screen information
	if (ioctl(fbfd, FBIOGET_VSCREENINFO, &vinfo)) {
		fprintf(stderr, "[FBInk] Error reading variable information.\n");
		return EXIT_FAILURE;
	}
	ELOG("[FBInk] Variable fb info: %ux%u, %ubpp @ rotation: %u (%s)",
	     vinfo.xres,
	     vinfo.yres,
	     vinfo.bits_per_pixel,
	     vinfo.rotate,
	     fb_rotate_to_string(vinfo.rotate));

	// NOTE: In most every cases, assume 0,0 is at the top-left of the screen, and xyes,yres at the bottom right.
	viewWidth  = vinfo.xres;
	viewHeight = vinfo.yres;

	// NOTE: Reset original font resolution, in case we're re-init'ing,
	//       since we're relying on the default value to calculate the scaled value,
	//       and we're using this value to set MAXCOLS & MAXROWS, which we *need* to be sane.
	FONTW = 8U;
	FONTH = 8U;

#ifdef FBINK_WITH_UNSCII
	// NOTE: Unscii-16 is 8x16, handle it ;).
	if (fbink_config->fontname == UNSCII_TALL) {
		FONTH = 16U;
	}
#endif

	// Obey user-specified font scaling multiplier
	if (fbink_config->fontmult > 0) {
		FONTSIZE_MULT = fbink_config->fontmult;

		// NOTE: Clamp to safe values to avoid blowing up the stack with alloca later,
		//       in case we get fed a stupidly large value.
		//       Incidentally, this is also necessary to avoid overflowing an unsigned short with the result of
		//       FONTW * FONTH, because that's basically what happens in one form or another during scaling.
#ifdef FBINK_WITH_UNSCII
		// NOTE: Unscii-16 is 8x16, handle it ;).
		if (fbink_config->fontname == UNSCII_TALL) {
			FONTSIZE_MULT = MIN(22, FONTSIZE_MULT);    // $(( (8 * 23) * ( 16 * 23) )) -> 67712. MEEP!
		} else {
			FONTSIZE_MULT = MIN(31, FONTSIZE_MULT);    // $(( (8 * 32) ** 2 )) -> 65536. MEEP!
		}
#else
		FONTSIZE_MULT = MIN(31, FONTSIZE_MULT);
#endif
	} else {
		// Set font-size based on screen resolution (roughly matches: Pearl, Carta, Carta HD & 7" Carta, 7" Carta HD)
		// NOTE: We still want to compare against the screen's "height", even in Landscape mode...
		uint32_t actual_height = MAX(vinfo.xres, vinfo.yres);
		//deviceQuirks.isKobo16Landscape = true;
		if (vinfo.xres > vinfo.yres) {
			// NOTE: vinfo.rotate == 2 (vs. 3 in Portrait mode) on my PW2
			//       My Touch, which doesn't propose Landscape mode, defaults to vinfo.rotate == 1
			//       My K4, which supports the four possible rotations,
			//          is always using vinfo.rotate == 0 (but xres & yres do switch).
			// NOTE: Here be dragons!
#if !defined(FBINK_FOR_KINDLE) && !defined(FBINK_FOR_LEGACY)
			if (vinfo.bits_per_pixel == 16) {
				viewWidth                      = vinfo.yres;
				viewHeight                     = vinfo.xres;
				deviceQuirks.isKobo16Landscape = true;
				ELOG("[FBInk] Enabled Kobo 16bpp fb rotation device quirks");
			}
#endif
		}
		if (actual_height <= 600U) {
			FONTSIZE_MULT = 1U;    // 8x8
		} else if (actual_height <= 1024U) {
			FONTSIZE_MULT = 2U;    // 16x16
		} else if (actual_height <= 1440U) {
			FONTSIZE_MULT = 3U;    // 24x24
		} else {
			FONTSIZE_MULT = 4U;    // 32x32
		}
	}
	// Go!
	FONTW = (unsigned short int) (FONTW * FONTSIZE_MULT);
	FONTH = (unsigned short int) (FONTH * FONTSIZE_MULT);
	ELOG("[FBInk] Fontsize set to %dx%d.", FONTW, FONTH);

	// Compute MAX* values now that we know the screen & font resolution
	MAXCOLS = (unsigned short int) (viewWidth / FONTW);
	MAXROWS = (unsigned short int) (viewHeight / FONTH);
	ELOG("[FBInk] Line length: %hu cols, Page size: %hu rows.", MAXCOLS, MAXROWS);

	// Mention & remember if we can perfectly fit the final column on screen
	if ((unsigned short int) (FONTW * MAXCOLS) == viewWidth) {
		deviceQuirks.isPerfectFit = true;
		ELOG("[FBInk] It's a perfect fit!");
	}

	// Get fixed screen information
	if (ioctl(fbfd, FBIOGET_FSCREENINFO, &finfo)) {
		fprintf(stderr, "[FBInk] Error reading fixed information.\n");
	}
	ELOG("[FBInk] Fixed fb info: ID is \"%s\", length of fb mem: %u bytes & line length: %u bytes",
	     finfo.id,
	     finfo.smem_len,
	     finfo.line_length);

	// Finish with some more generic stuff, not directly related to the framebuffer.
	// As all this stuff is pretty much set in stone, we'll only query it once.
	if (!deviceQuirks.skipId) {
		// Identify the device's specific model...
		identify_device(&deviceQuirks);
		if (deviceQuirks.isKindlePearlScreen) {
			ELOG("[FBInk] Enabled Kindle with Pearl screen device quirks");
		} else if (deviceQuirks.isKindleOasis2) {
			ELOG("[FBInk] Enabled Kindle Oasis 2 device quirks");
		} else if (deviceQuirks.isKoboMk7) {
			ELOG("[FBInk] Enabled Kobo Mark 7 device quirks");
		}

		// Ask the Kernel for its HZ value so we can translate jiffies into human-readable units.
		long int rv = sysconf(_SC_CLK_TCK);
		if (rv > 0) {
			USER_HZ = rv;
			ELOG("[FBInk] Kernel's HZ value appears to be %ld", USER_HZ);
		} else {
			ELOG("[FBInk] Unable to query Kernel's HZ value, assuming %ld", USER_HZ);
		}

		// And make sure we won't do that again ;).
		deviceQuirks.skipId = true;
	}

	// NOTE: Do we want to keep the fb0 fd open and pass it to our caller, or simply close it for now?
	//       Useful because we probably want to close it to keep open fds to a minimum when used as a library,
	//       while wanting to avoid a useless open/close/open/close cycle when used as a standalone tool.
	if (keep_fd) {
		return fbfd;
	} else {
		close(fbfd);
		return EXIT_SUCCESS;
	}
}

// Magic happens here!
int
    fbink_print(int fbfd, const char* string, const FBInkConfig* fbink_config)
{
	// Open the framebuffer if need be...
	bool keep_fd = true;
	if (fbfd == -1) {
		// If we open a fd now, we'll only keep it open for this single print call!
		// NOTE: We *expect* to be initialized at this point, though, but that's on the caller's hands!
		keep_fd = false;
		if (-1 == (fbfd = fbink_open())) {
			fprintf(stderr, "[FBInk] Failed to open the framebuffer, aborting . . .\n");
			return -1;
		}
	}

	// map fb to user mem
	// NOTE: Beware of smem_len on Kobos?
	//       c.f., https://github.com/koreader/koreader-base/blob/master/ffi/framebuffer_linux.lua#L36
	// NOTE: If we're keeping the fb's fd open, keep this mmap around, too.
	if (!g_fbink_isFbMapped) {
		g_fbink_screensize = finfo.smem_len;
		g_fbink_fbp =
		    (unsigned char*) mmap(NULL, g_fbink_screensize, PROT_READ | PROT_WRITE, MAP_SHARED, fbfd, 0);
		if (g_fbink_fbp == MAP_FAILED) {
			char  buf[256];
			char* errstr = strerror_r(errno, buf, sizeof(buf));
			fprintf(stderr, "[FBInk] mmap: %s\n", errstr);
			return -1;
		} else {
			g_fbink_isFbMapped = true;
		}
	}

	// NOTE: Make copies of these so we don't wreck our original struct, since we passed it by reference,
	//       and we *will* heavily mangle these two...
	short int col = fbink_config->col;
	short int row = fbink_config->row;

	struct mxcfb_rect region = { 0U };
	// We declare that a bit early, because that'll hold our return value on success.
	unsigned short int multiline_offset = 0U;

	if (g_fbink_isFbMapped) {
		// Clear screen?
		if (fbink_config->is_cleared) {
			clear_screen(fbink_config->is_inverted ? BLACK : WHITE);
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
			col = (short int) (MAXCOLS - 1);
			LOG("Clamped column to %hd", col);
		}
		if (row >= MAXROWS) {
			row = (short int) (MAXROWS - 1);
			LOG("Clamped row to %hd", row);
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
		// NOTE: UTF-8 is at most 4 bytes per sequence, make sure we can fit a full line of UTF-8 (+ 1 'wide' NULL).
		char* line = NULL;
		line       = calloc((MAXCOLS + 1U) * 4U, sizeof(*line));
		if (line == NULL) {
			char  buf[256];
			char* errstr = strerror_r(errno, buf, sizeof(buf));
			fprintf(stderr, "[FBInk] calloc (line): %s\n", errstr);
			return -1;
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

		LOG("Need %hu lines to print %u characters over %hu available columns",
		    lines,
		    charcount,
		    available_cols);

		// Do the initial computation outside the loop,
		// so we'll be able to re-use line_len to accurately compute chars_left when looping.
		// NOTE: This is where it gets tricky. With multibyte sequences, 1 byte doesn't necessarily mean 1 char.
		//       And we need to work both in amount of characters for column/width arithmetic,
		//       and in bytes for snprintf...
		unsigned int chars_left = charcount;
		unsigned int line_len   = 0U;
		// If we have multiple lines worth of stuff to print, draw it line per line
		while (chars_left > line_len) {
			LOG("Line %u (of ~%u), previous line was %u characters long and there were %u characters left to print",
			    multiline_offset + 1U,
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
			line_len = MIN(chars_left, available_cols);
			LOG("Characters to print: %u out of the %u remaining ones", line_len, chars_left);

			// NOTE: Now we just have to switch from characters to bytes, both for line_len & chars_left...
			// First, get the byte offset of this section of our string (i.e., this line)...
			unsigned int line_offset = u8_offset(string, charcount - chars_left);
			// ... then compute how many bytes we'll need to store it.
			unsigned int line_bytes = 0U;
			unsigned int cn         = 0U;
			uint32_t     ch         = 0U;
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
					LOG("Line length was %u characters, but LF is character number %u",
					    line_len,
					    cn);
					line_len = cn;
					// Don't touch line_offset, the beginning of our line has not changed,
					// only its length was cut short.
					LOG("Adjusted lines to %u & line_len to %u", lines, line_len);
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
				col = (short int) ((MAXCOLS - line_len) / 2U);

				// NOTE: If the line itself is not a perfect fit, ask draw to start drawing half a cell
				//       to the right to compensate, in order to achieve perfect centering...
				//       This piggybacks a bit on the !isPerfectFit compensation done in draw,
				//       which already does subcell placement ;).
				if ((unsigned int) (col * 2) + line_len != MAXCOLS) {
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
				unsigned int left_pad = (MAXCOLS - line_len) / 2U;
				// As for the right padding, we basically just have to print 'til the edge of the screen
				unsigned int right_pad = MAXCOLS - line_len - left_pad;

				// Compute the effective right padding value for science!
				LOG("Total size: %u + %u + %u = %u",
				    left_pad,
				    line_len,
				    right_pad,
				    left_pad + line_len + right_pad);

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
				unsigned int padded_bytes = line_bytes + (available_cols - line_len);
				// NOTE: Don't touch line_len, because we're *adding* new blank characters,
				//       we're still printing the exact same amount of characters *from our string*.
				LOG("Padded %u bytes to %u to cover %u columns",
				    line_bytes,
				    padded_bytes,
				    available_cols);
				bytes_printed = snprintf(line,
							 padded_bytes + 1U,
							 "%*.*s",
							 (int) padded_bytes,
							 (int) line_bytes,
							 string + line_offset);
			} else {
				// NOTE: Enforce precision for safety.
				bytes_printed = snprintf(line,
							 line_bytes + 1U,
							 "%*.*s",
							 (int) line_bytes,
							 (int) line_bytes,
							 string + line_offset);
			}
			LOG("snprintf wrote %d bytes", bytes_printed);

			region = draw(line,
				      (unsigned short int) row,
				      (unsigned short int) col,
				      fbink_config->is_inverted,
				      fbink_config->is_centered,
				      multiline_offset,
				      fbink_config->fontname,
				      halfcell_offset);

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

		// Cleanup
		free(line);
	}

	// Fudge the region if we asked for a screen clear, so that we actually refresh the full screen...
	if (fbink_config->is_cleared) {
		region.top    = 0U;
		region.left   = 0U;
		region.width  = vinfo.xres;
		region.height = vinfo.yres;
	}

	// Refresh screen
	if (refresh(fbfd, region, WAVEFORM_MODE_AUTO, fbink_config->is_flashing) != EXIT_SUCCESS) {
		fprintf(stderr, "[FBInk] Failed to refresh the screen!\n");
	}

	// cleanup
	if (g_fbink_isFbMapped && !keep_fd) {
		munmap(g_fbink_fbp, g_fbink_screensize);
		// NOTE: Don't forget to reset those state flags,
		//       so we won't skip mmap'ing on the next call without an fb fd passed...
		g_fbink_isFbMapped = false;
		g_fbink_fbp        = 0U;
	}
	if (!keep_fd) {
		close(fbfd);
	}

	// We return the total amount of lines we occupied on screen
	return (int) multiline_offset;
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
		return -1;
	}

	va_list args;
	va_start(args, fmt);
	// vsnprintf will ensure we'll always be NULL-terminated (but not NULL-backfilled, hence calloc!) ;).
	vsnprintf(buffer, ((size_t)(MAXCOLS * MAXROWS) * 4U) + 1U, fmt, args);
	va_end(args);

	int rc = fbink_print(fbfd, buffer, fbink_config);

	// Cleanup
	free(buffer);
	return rc;
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
	if (fbfd == -1) {
		// If we open a fd now, we'll only keep it open for this single print call!
		keep_fd = false;
		if (-1 == (fbfd = fbink_open())) {
			fprintf(stderr, "[FBInk] Failed to open the framebuffer, aborting . . .\n");
			return EXIT_FAILURE;
		}
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
