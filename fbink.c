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
    put_pixel_Gray8(unsigned short int x, unsigned short int y, unsigned short int c)
{
	// calculate the pixel's byte offset inside the buffer
	size_t pix_offset = x + y * finfo.line_length;

	// now this is about the same as 'fbp[pix_offset] = value'
	*((char*) (fbp + pix_offset)) = (char) c;
}

static void
    put_pixel_RGB24(unsigned short int x,
		    unsigned short int y,
		    unsigned short int r,
		    unsigned short int g,
		    unsigned short int b)
{
	// calculate the pixel's byte offset inside the buffer
	// note: x * 3 as every pixel is 3 consecutive bytes
	size_t pix_offset = x * 3U + y * finfo.line_length;

	// now this is about the same as 'fbp[pix_offset] = value'
	*((char*) (fbp + pix_offset))     = (char) b;
	*((char*) (fbp + pix_offset + 1)) = (char) g;
	*((char*) (fbp + pix_offset + 2)) = (char) r;
}

static void
    put_pixel_RGB32(unsigned short int x,
		    unsigned short int y,
		    unsigned short int r,
		    unsigned short int g,
		    unsigned short int b)
{
	// calculate the pixel's byte offset inside the buffer
	// note: x * 4 as every pixel is 4 consecutive bytes
	size_t pix_offset = x * 4U + y * finfo.line_length;

	// now this is about the same as 'fbp[pix_offset] = value'
	*((char*) (fbp + pix_offset))     = (char) b;
	*((char*) (fbp + pix_offset + 1)) = (char) g;
	*((char*) (fbp + pix_offset + 2)) = (char) r;
	*((char*) (fbp + pix_offset + 3)) = 0xFF;    // Opaque, always.
}

static void
    put_pixel_RGB565(unsigned short int x,
		     unsigned short int y,
		     unsigned short int r,
		     unsigned short int g,
		     unsigned short int b)
{
	// calculate the pixel's byte offset inside the buffer
	// note: x * 2 as every pixel is 2 consecutive bytes
	size_t pix_offset = x * 2U + y * finfo.line_length;

	// now this is about the same as 'fbp[pix_offset] = value'
	// but a bit more complicated for RGB565
	unsigned int c = ((r / 8U) << 11U) + ((g / 4U) << 5U) + (b / 8U);
	// or: c = ((r / 8) * 2048) + ((g / 4) * 32) + (b / 8);
	// write 'two bytes at once'
	*((char*) (fbp + pix_offset)) = (char) c;
}

// Handle various bpp...
static void
    put_pixel(unsigned short int x, unsigned short int y, unsigned short int c)
{
	if (vinfo.bits_per_pixel == 8U) {
		// NOTE: Grayscale palette, we could have used def_r or def_g ;).
		put_pixel_Gray8(x, y, def_b[c]);
	} else if (vinfo.bits_per_pixel == 16U) {
		// FIXME: Colors *may* actually be inverted on 16bpp Kobos...
		//        This should fix it:
		//c = c ^ WHITE;
		put_pixel_RGB565(x, y, def_r[c], def_g[c], def_b[c]);
	} else if (vinfo.bits_per_pixel == 24U) {
		put_pixel_RGB24(x, y, def_r[c], def_g[c], def_b[c]);
	} else if (vinfo.bits_per_pixel == 32U) {
		put_pixel_RGB32(x, y, def_r[c], def_g[c], def_b[c]);
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
	unsigned short int cx;
	unsigned short int cy;
	for (cy = 0U; cy < h; cy++) {
		for (cx = 0U; cx < w; cx++) {
			put_pixel((unsigned short int) (x + cx), (unsigned short int) (y + cy), c);
		}
	}
	printf("filled %hux%hu rectangle @ %hu, %hu\n", w, h, x, y);
}

// Helper function to clear the screen - fill whole screen with given color
static void
    clear_screen(unsigned short int c)
{
	// NOTE: Grayscale palette, we could have used def_r or def_g ;).
	if (vinfo.bits_per_pixel == 8U) {
		memset(fbp, def_b[c], finfo.smem_len);
	} else {
		memset(fbp, def_b[c], finfo.smem_len);
	}
}

// Return the font8x8 bitmap for a specifric ascii character
static char*
    font8x8_get_bitmap(uint32_t codepoint)
{
	// Get the bitmap for that ASCII character
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
		fprintf(stderr, "[FBInk] Codepoint U+%04X is not covered by our font!\n", codepoint);
		return font8x8_basic[0];
	}
}

// Render a specific font8x8 glyph into a pixmap
// (base size: 8x8, scaled by a factor of FONTSIZE_MULT, which varies depending on screen resolution)
static void
    font8x8_render(uint32_t codepoint, char* glyph_pixmap)
{
	char* bitmap = font8x8_get_bitmap(codepoint);

	unsigned short int x;
	unsigned short int y;
	unsigned short int i;
	unsigned short int j;
	unsigned short int k;
	bool               set = false;
	for (i = 0U; i < FONTW; i++) {
		// x: input row, i: output row
		x = i / FONTSIZE_MULT;
		for (j = 0U; j < FONTH; j++) {
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
	 unsigned short int multiline_offset)
{
	printf("Printing '%s' @ line offset %hu\n", text, multiline_offset);
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
	printf("Character count: %u (over %zu bytes)\n", charcount, strlen(text));

	// Compute the dimension of the screen region we'll paint to (taking multi-line into account)
	struct mxcfb_rect region = {
		.top   = (uint32_t)((row - multiline_offset) * FONTH),
		.left  = (uint32_t)(col * FONTW),
		.width = multiline_offset > 0U ? (vinfo.xres - (uint32_t)(col * FONTW)) : (uint32_t)(charcount * FONTW),
		.height = (uint32_t)((multiline_offset + 1U) * FONTH),
	};

	printf("Region: top=%u, left=%u, width=%u, height=%u\n", region.top, region.left, region.width, region.height);

	// NOTE: eInk framebuffers are weird...
	//       We should be computing the length of a line (MAXCOLS) based on xres_virtual,
	//       not xres (because it's guaranteed to be a multiple of 16).
	//       Unfortunately, that means this last block of the line would then be partly offscreen.
	//       Also, it CANNOT be part of the region passed to the eInk controller for a screen update,
	//       since it's expecting the effective screen size...
	//       So, since this this last block may basically be unusable because partly unreadable,
	//       and partly unrefreshable, don't count it as "available" (i.e., by including it in MAXCOLS),
	//       since that would happen to also wreak havoc in a number of our heuristics,
	//       just fudge printing a blank square 'til the edge of the screen if we're filling a line *completely*.
	//       TL;DR: We compute stuff with xres and not xres_virtual, unlike eips.
	// NOTE: Given that, if we can perfectly fit our final character on the line
	//       (c.f., how is_perfect_fit is computed, basically, when MAXCOLS is not a fraction),
	//       this effectively works around the issue, in which case, we don't need to do anything :).
	// NOTE: Use charcount + col == MAXCOLS if we want to do that everytime we simply *hit* the edge...
	if (charcount == MAXCOLS && !is_perfect_fit) {
		fill_rect((unsigned short int) (region.left + (charcount * FONTW)),
			  (unsigned short int) (region.top + (unsigned short int) (multiline_offset * FONTH)),
			  (unsigned short int) (vinfo.xres - (charcount * FONTW)),
			  FONTH,
			  bgC);
		// Update region to the full width, no matter the circumstances
		region.width += (vinfo.xres - (charcount * FONTW));
		// And make sure it's properly clamped, in case it's already been tweaked because of a multiline print
		if (region.width + region.left > vinfo.xres) {
			region.width = vinfo.xres - region.left;
		}
		printf("Updated region.width to %u\n", region.width);
	}

	// NOTE: In case of a multi-line centered print, we can't really trust the final col,
	//       it might be significantly different than the others, and as such, we'd be computing a cropped region.
	//       Make the region cover the full width of the screen to make sure we won't miss anything.
	if (multiline_offset > 0U && is_centered) {
		region.left  = 0U;
		region.width = vinfo.xres;
		printf("Updated region.left to %u & region.width to %u because of multi-line centering\n",
		       region.left,
		       region.width);
	}

	// Fill our bounding box with our background color, so that we'll be visible no matter what's already on screen.
	// NOTE: Unneeded, we already plot the background when handling font glyphs ;).
	//fill_rect(region.left, region.top, region.width, region.height, bgC);

	// Alloc our pixmap on the stack, and re-use it.
	// NOTE: We tried using automatic VLAs, but that... didn't go well.
	//       (as in, subtle (or not so) memory and/or stack corruption).
	char* pixmap = NULL;
	// NOTE: Using alloca may prevent inlining. That said, trust that the compiler will do the right thing.
	//       As for why alloca:
	//       It's a very small allocation, we'll always fully write to it so we don't care about its initialization,
	//       -> it's a perfect fit for the stack.
	//       In any other situation (i.e., constant FONTW & FONTH), it'd have been an automatic.
	pixmap = alloca(sizeof(*pixmap) * (size_t)(FONTW * FONTH));

	// Loop through all the *characters* in the text string
	unsigned int       bi = 0U;
	unsigned short int ci = 0U;
	uint32_t           ch = 0U;
	while ((ch = u8_nextchar(text, &bi)) != 0U) {
		printf("Char %u (@ %u) out of %u is @ byte offset %d and is U+%04X\n", ci + 1, ci, charcount, bi, ch);

		// Get the glyph's pixmap
		font8x8_render(ch, pixmap);

		// loop through pixel rows
		for (y = 0U; y < FONTH; y++) {
			// loop through pixel columns
			for (x = 0U; x < FONTW; x++) {
				// get the pixel value
				char b = pixmap[(y * FONTW) + x];
				if (b > 0) {
					// plot the pixel (fg, text)
					put_pixel((unsigned short int) ((col * FONTW) + (ci * FONTW) + x),
						  (unsigned short int) ((row * FONTH) + y),
						  fgC);
				} else {
					// this is background,
					// fill it so that we'll be visible no matter what was on screen behind us.
					put_pixel((unsigned short int) ((col * FONTW) + (ci * FONTW) + x),
						  (unsigned short int) ((row * FONTH) + y),
						  bgC);
				}
			}    // end "for x"
		}            // end "for y"
		// Next glyph! This serves as the source for the pen position, hence it being used as an index...
		ci++;
	}
	printf("\n");

	return region;
}

// Handle eInk updates
static int
    refresh(int fbfd, struct mxcfb_rect region, uint32_t waveform_mode, bool is_flashing)
{
	// NOTE: While we'd be perfect candidates for using A2 waveform mode, it's all kinds of fucked up on Kobos,
	//       and may lead to disappearing text or weird blending depending on the surrounding fb content...
	//       It only shows up properly when FULL, which isn't great...
	//       Also, we need to set the EPDC_FLAG_FORCE_MONOCHROME flag to do it right.
	// NOTE: And while we're on the fun quirks train: FULL never flashes w/ AUTO on (some?) Kobos,
	//       so request GC16 if we want a flash...
	struct mxcfb_update_data update = {
		.temp          = TEMP_USE_AMBIENT,
		.update_marker = (uint32_t) getpid(),
		.update_mode   = is_flashing ? UPDATE_MODE_FULL : UPDATE_MODE_PARTIAL,
		.update_region = region,
		.waveform_mode =
		    (is_flashing && waveform_mode == WAVEFORM_MODE_AUTO) ? WAVEFORM_MODE_GC16 : waveform_mode,
#ifndef FBINK_FOR_KINDLE
		.flags = (waveform_mode == WAVEFORM_MODE_REAGLD)
			     ? EPDC_FLAG_USE_AAD
			     : (waveform_mode == WAVEFORM_MODE_A2) ? EPDC_FLAG_FORCE_MONOCHROME : 0U,
#else
		.flags = 0U,
#endif
#ifdef FBINK_FOR_KINDLE
		.hist_bw_waveform_mode   = WAVEFORM_MODE_DU,
		.hist_gray_waveform_mode = WAVEFORM_MODE_GC16_FAST,
#endif
	};

	// NOTE: Make sure update_marker is valid, an invalid marker *may* hang the kernel instead of failing gracefully,
	//       depending on the device/FW...
	if (update.update_marker == 0U) {
		update.update_marker = (70U + 66U + 73U + 78U + 75U);
	}

	if (ioctl(fbfd, MXCFB_SEND_UPDATE, &update) < 0) {
		// NOTE: perror() is not thread-safe...
		char  buf[256];
		char* errstr = strerror_r(errno, buf, sizeof(buf));
		fprintf(stderr, "[FBInk] MXCFB_SEND_UPDATE: %s\n", errstr);
		return EXIT_FAILURE;
	}

	// NOTE: Let's be extremely thorough, and wait for completion on flashing updates ;)
	if (is_flashing) {
#ifdef FBINK_FOR_KINDLE
		// FIXME: Handle the Carta/Pearl switch in a saner way...
		struct mxcfb_update_marker_data update_marker = {
			.update_marker  = update.update_marker,
			.collision_test = 0U,
		};
		bool failed = false;
		if (ioctl(fbfd, MXCFB_WAIT_FOR_UPDATE_COMPLETE, &update_marker) < 0) {
			char  buf[256];
			char* errstr = strerror_r(errno, buf, sizeof(buf));
			fprintf(stderr, "[FBInk] MXCFB_WAIT_FOR_UPDATE_COMPLETE: %s\n", errstr);
			failed = true;
			// NOTE: See FIXME. Because right now that's very, very cheap.
			//       And trusts that the Kernel won't barf. Which seems to hold true on my Touch.
			if (ioctl(fbfd, MXCFB_WAIT_FOR_UPDATE_COMPLETE_PEARL, &update.update_marker) < 0) {
				errstr = strerror_r(errno, buf, sizeof(buf));
				fprintf(stderr, "[FBInk] MXCFB_WAIT_FOR_UPDATE_COMPLETE_PEARL: %s\n", errstr);
				failed = true;
			} else {
				failed = false;
			}
		}
		if (failed) {
			return EXIT_FAILURE;
		}
#else
		if (ioctl(fbfd, MXCFB_WAIT_FOR_UPDATE_COMPLETE, &update.update_marker) < 0) {
			{
				char buf[256];
				char* errstr = strerror_r(errno, buf, sizeof(buf));
				fprintf(stderr, "[FBInk] MXCFB_WAIT_FOR_UPDATE_COMPLETE: %s\n", errstr);
				return EXIT_FAILURE;
			}
		}
#endif
	}

	return EXIT_SUCCESS;
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

// Get the various fb info & setup global variables
int
    fbink_init(int fbfd)
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

	// Get variable screen information
	if (ioctl(fbfd, FBIOGET_VSCREENINFO, &vinfo)) {
		fprintf(stderr, "[FBInk] Error reading variable information.\n");
		return EXIT_FAILURE;
	}
	fprintf(stderr,
		"[FBInk] Variable fb info: %ux%u, %ubpp @ rotation: %u\n",
		vinfo.xres,
		vinfo.yres,
		vinfo.bits_per_pixel,
		vinfo.rotate);

	// NOTE: Reset original font resolution, in case we're re-init'ing,
	//       since we're relying on the default value to calculate the scaled value,
	//       and we're using this value to set MAXCOLS & MAXROWS, which we *need* to be sane.
	FONTW = 8U;
	FONTH = 8U;

	// Set font-size based on screen resolution (roughly matches: Pearl, Carta, Carta HD & 7" Carta, 7" Carta HD)
	// NOTE: We still want to compare against the screen's "height", even in Landscape mode...
	uint32_t screen_height = vinfo.yres;
	if (vinfo.xres > vinfo.yres) {
		// NOTE: vinfo.rotate == 2 (vs. 3 in Portrait mode) on my PW2
		//       My Touch, which doesn't propose Landscape mode, defaults to vinfo.rotate == 1
		//       My K4, which supports the four possible rotations,
		//          is always using vinfo.rotate == 0 (but xres & yres do switch).
		//          It's also using the old eink_fb driver, which we do not support anyway :D.
		screen_height = vinfo.xres;
	}
	if (screen_height <= 600U) {
		FONTSIZE_MULT = 1U;    // 8x8
	} else if (screen_height <= 1024U) {
		FONTSIZE_MULT = 2U;    // 16x16
	} else if (screen_height <= 1440U) {
		FONTSIZE_MULT = 3U;    // 24x24
	} else {
		FONTSIZE_MULT = 4U;    // 32x32
	}
	// Go!
	FONTW = (unsigned short int) (FONTW * FONTSIZE_MULT);
	FONTH = (unsigned short int) (FONTH * FONTSIZE_MULT);
	fprintf(stderr, "[FBInk] Fontsize set to %dx%d.\n", FONTW, FONTH);

	// Compute MAX* values now that we know the screen & font resolution
	MAXCOLS = (unsigned short int) (vinfo.xres / FONTW);
	MAXROWS = (unsigned short int) (vinfo.yres / FONTH);
	fprintf(stderr, "[FBInk] Line length: %hu cols, Page size: %hu rows.\n", MAXCOLS, MAXROWS);

	// Mention & remember if we can perfectly fit the final column on screen
	if (FONTW * MAXCOLS == vinfo.xres) {
		is_perfect_fit = true;
		fprintf(stderr, "[FBInk] It's a perfect fit!\n");
	}

	// Get fixed screen information
	if (ioctl(fbfd, FBIOGET_FSCREENINFO, &finfo)) {
		fprintf(stderr, "[FBInk] Error reading fixed information.\n");
	}
	fprintf(stderr, "[FBInk] Fixed fb info: smem_len %d, line_length %d\n", finfo.smem_len, finfo.line_length);

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
    fbink_print(int fbfd, const char* string, FBInkConfig* fbink_config)
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
	if (!fb_is_mapped) {
		screensize = finfo.smem_len;
		fbp        = (char*) mmap(NULL, screensize, PROT_READ | PROT_WRITE, MAP_SHARED, fbfd, 0);
		if (fbp == MAP_FAILED) {
			char  buf[256];
			char* errstr = strerror_r(errno, buf, sizeof(buf));
			fprintf(stderr, "[FBInk] mmap: %s\n", errstr);
			return -1;
		} else {
			fb_is_mapped = true;
		}
	}

	// NOTE: Make copies of these so we don't wreck our original struct, since we passed it by reference,
	//       and we *will* heavily mangle these two...
	short int col = fbink_config->col;
	short int row = fbink_config->row;

	struct mxcfb_rect region;
	// We declare that a bit early, because that'll hold our return value on success.
	unsigned short int multiline_offset = 0U;

	if (fb_is_mapped) {
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
		printf("Adjusted position: column %hd, row %hd\n", col, row);

		// Clamp coordinates to the screen, to avoid blowing up ;).
		if (col >= MAXCOLS) {
			col = (short int) (MAXCOLS - 1);
			printf("Clamped column to %hd\n", col);
		}
		if (row >= MAXROWS) {
			row = (short int) (MAXROWS - 1);
			printf("Clamped row to %hd\n", row);
		}

		// See if we need to break our string down into multiple lines...
		size_t       len       = strlen(string);
		unsigned int charcount = u8_strlen(string);
		// Check how much extra storage is used up by multibyte sequences.
		if (len > charcount) {
			printf(
			    "Extra storage used up by multibyte sequences: %zu bytes (for a total of %u characters over %zu bytes)\n",
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
			if (is_perfect_fit) {
				// And one for the right padding
				available_cols = (unsigned short int) (available_cols - 1U);
			}
		} else {
			// Otherwise, col will be fixed, so, trust it.
			available_cols = (unsigned short int) (available_cols - col);
		}
		// Given that, compute how many lines it'll take to print all that in these constraints...
		unsigned short int lines            = 1U;
		if (charcount > available_cols) {
			lines = (unsigned short int) (charcount / available_cols);
			// If there's a remainder, we'll need an extra line ;).
			if (charcount % available_cols) {
				lines++;
			}
		}

		// Truncate to a single screen...
		if (lines > MAXROWS) {
			printf("Can only print %hu out of %hu lines, truncating!\n", MAXROWS, lines);
			lines = MAXROWS;
		}

		// Move our initial row up if we add so much lines that some of it goes off-screen...
		if (row + lines > MAXROWS) {
			row = (short int) MIN(row - ((row + lines) - MAXROWS), MAXROWS);
		}
		printf("Final position: column %hd, row %hd\n", col, row);

		// We'll copy our text in chunks of formatted line...
		// NOTE: Store that on the heap, we've had some wacky adventures with automatic VLAs...
		// NOTE: UTF-8 is at most 4 bytes per sequence, make sure we can fit a full line of UTF-8 (+ 1 'wide' NULL).
		char* line = NULL;
		line       = calloc((MAXCOLS + 1U) * 4U, sizeof(*line));
		// NOTE: This makes sure it's always full of NULLs, to avoid weird shit happening later with u8_strlen()
		//       and uninitialized or re-used memory...
		//       Namely, a single NULL immediately followed by something that *might* be interpreted as an UTF-8
		//       sequence would trip it into counting bogus characters.
		//       And as snprintf() only NULL-terminates what it expects to be a non-wide string,
		//       it's not filling the end of the buffer with NULLs, it just outputs a single one!
		//       That's why we're also using calloc here.
		//       Plus, the OS will ensure that'll always be smarter than malloc + memset ;).

		printf("Need %hu lines to print %u characters over %hu available columns\n",
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
			printf("Line %u (of ~%u), previous line was %u characters long and there were %u characters left to print\n", multiline_offset + 1, lines, line_len, chars_left);
			// Make sure we don't try to draw off-screen...
			if (row + multiline_offset >= MAXROWS) {
				printf("Can only print %hu lines, discarding the %u characters left!\n", MAXROWS, chars_left - line_len);
				// And that's it, we're done.
				break;
			}

			// Compute the amount of characters left to print...
			chars_left -= line_len;
			// And use it to compute the amount of characters to print on *this* line
			line_len = MIN(chars_left, available_cols);
			printf("Characters to print: %u out of %zu (left: %u)\n",
			       line_len,
			       available_cols * sizeof(char),
			       chars_left);

			// NOTE: Now we just have to switch from characters to bytes, both for line_len & chars_left...
			// First, get the byte offset of this section of our string (i.e., this line)...
			unsigned int line_offset = u8_offset(string, charcount - chars_left);
			// ... then compute how many bytes we'll need to store it.
			unsigned int line_bytes = 0U;
			unsigned int cn         = 0U;
			uint32_t             ch = 0U;
			while ((ch = u8_nextchar(string + line_offset, &line_bytes)) != 0U) {
				cn++;
				// NOTE: This is fairly hackish: in every case, we get extra blank lines
				//       (w/ left=0; or with chars_left underflowing when padding is enabled),
				//       and padding only actually applies to the last line.
				//       Granted, that last one may be by design...
				//       Or just kill padding if we catch a LF?
				// We've hit a linefeed, stop!
				if (ch == 0x0A) {
					printf("Caught a linefeed!\n");
					// NOTE: We're essentially forcing a reflow by cutting the line mid-stream,
					//       so we have to update our counters...
					//       But we can only correct one of chars_left or line_len,
					//       to avoid screwing the count on the next iteration if we correct both,
					//       since the one depend on the other.
					//       And as, for the rest of this iteration of the loop, we only rely on
					//       line_len being accurate (for padding & centering), the choice is easy.
					// Increment lines, because of course we're adding a line,
					// even if the reflowing changes that'll cause mean we might not end up using it.
					lines++;
					// Don't decrement the byte index, we want to print the LF,
					// (it'll render as a blank), mostly to make padding look nicer,
					// but also so that line_bytes matches line_len ;).
					// And finally, as we've explained earlier, trim line_len to where we stopped.
					printf("Line length was %u characters, but LF is character number %u\n", line_len, cn);
					line_len = cn;
					// Don't touch line_offset, the beginning of our line has not changed,
					// only its length was cut short.
					printf("Updated lines to %u & line_len to %u\n", lines, line_len);
					break;
				}
				// We've walked our full line, stop!
				if (cn >= line_len) {
					break;
				}
			}
			printf("Line takes up %u bytes\n", line_bytes);
			int bytes_printed = 0;

			// Just fudge the column for centering...
			if (fbink_config->is_centered) {
				col = (short int) ((MAXCOLS / 2U) - (line_len / 2U));
				// When we're not padding, we have a few more things to take care of...
				if (!fbink_config->is_padded) {
					// We don't do padding via snprintf in this case,
					// so just fudge col to avoid the first column.
					if (col == 0) {
						col = 1;
					}
				}
				printf("Adjusted column to %hd for centering\n", col);
			}

			// When centered & padded, we need to split the padding in two, left & right.
			if (fbink_config->is_centered && fbink_config->is_padded) {
				// We always want full padding
				col = 0;

				// Compute our padding length
				unsigned int left_pad = (MAXCOLS - line_len) / 2U;
				// We want to enforce at least a single character of padding on the left.
				if (left_pad < 1U) {
					left_pad = 1U;
				}
				// As for the right padding, we basically just have to print 'til the edge of the screen
				unsigned int right_pad = MAXCOLS - line_len - left_pad;

				// Compute the effective right padding value for science!
				printf("Total size: %u + %u + %u = %u\n",
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
				printf("Padded %u bytes to %u to cover %u columns\n", line_bytes, padded_bytes, available_cols);
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
			printf("snprintf wrote %d bytes\n", bytes_printed);

			region = draw(line,
				      (unsigned short int) row,
				      (unsigned short int) col,
				      fbink_config->is_inverted,
				      fbink_config->is_centered,
				      multiline_offset);

			// Next line!
			multiline_offset++;
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
	if (fb_is_mapped && !keep_fd) {
		munmap(fbp, screensize);
		// NOTE: Don't forget to reset those state flags,
		//       so we won't skip mmap'ing on the next call without an fb fd passed...
		fb_is_mapped = false;
		fbp          = 0;
	}
	if (!keep_fd) {
		close(fbfd);
	}

	// We return the total amount of lines we occupied on screen
	return (int) multiline_offset;
}

// printf-like wrapper around fbink_print ;).
int
    fbink_printf(int fbfd, FBInkConfig* fbink_config, const char* fmt, ...)
{
	// We'll need to store our formatted string somewhere...
	// NOTE: Fit a single page's worth of characters in it, as that's the best we can do anyway.
	// NOTE: UTF-8 is at most 4 bytes per sequence, make sure we can fit a full page of UTF-8 (+1 'wide' NULL) :).
	char* buffer = NULL;
	// NOTE: We use calloc to make sure it'll always be zero-initialized,
	//       and the OS is smart enough to make it fast if we don't use the full space anyway (CoW zeroing).
	buffer = calloc(((size_t)(MAXCOLS * MAXROWS) + 1U) * 4U, sizeof(*buffer));

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
#ifdef FBINK_FOR_KINDLE
	} else if (strcasecmp("GC16_FAST", waveform_mode) == 0) {
		region_wfm = WAVEFORM_MODE_GC16_FAST;
	} else if (strcasecmp("GL16_FAST", waveform_mode) == 0) {
		region_wfm = WAVEFORM_MODE_GL16_FAST;
	} else if (strcasecmp("DU4", waveform_mode) == 0) {
		region_wfm = WAVEFORM_MODE_DU4;
	} else if (strcasecmp("GL4", waveform_mode) == 0) {
		region_wfm = WAVEFORM_MODE_GL4;
	} else if (strcasecmp("GL16_INV", waveform_mode) == 0) {
		region_wfm = WAVEFORM_MODE_GL16_INV;
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
