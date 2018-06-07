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

#include "fbink_internal.h"

#include "fbink.h"

// helper function to 'plot' a pixel in given color
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
	// remember to change main(): vinfo.bits_per_pixel = 24;
	// and: screensize = vinfo.xres * vinfo.yres *
	//                   vinfo.bits_per_pixel / 8;

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
	// remember to change main(): vinfo.bits_per_pixel = 32;
	// and: screensize = vinfo.xres * vinfo.yres *
	//                   vinfo.bits_per_pixel / 8;

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
	// remember to change main(): vinfo.bits_per_pixel = 16;
	// or on RPi just comment out the whole 'Change vinfo'
	// and: screensize = vinfo.xres * vinfo.yres *
	//                   vinfo.bits_per_pixel / 8;

	// calculate the pixel's byte offset inside the buffer
	// note: x * 2 as every pixel is 2 consecutive bytes
	size_t pix_offset = x * 2U + y * finfo.line_length;

	// now this is about the same as 'fbp[pix_offset] = value'
	// but a bit more complicated for RGB565
	unsigned int c = ((r / 8U) << 11) + ((g / 4U) << 5) + (b / 8U);
	// or: c = ((r / 8) * 2048) + ((g / 4) * 32) + (b / 8);
	// write 'two bytes at once'
	*((char*) (fbp + pix_offset)) = (char) c;
}

// handle various bpp...
static void
    put_pixel(unsigned short int x, unsigned short int y, unsigned short int c)
{
	if (vinfo.bits_per_pixel == 8) {
		put_pixel_Gray8(x, y, c);
	} else if (vinfo.bits_per_pixel == 16) {
		put_pixel_RGB565(x, y, def_r[c], def_g[c], def_b[c]);
	} else if (vinfo.bits_per_pixel == 24) {
		put_pixel_RGB24(x, y, def_r[c], def_g[c], def_b[c]);
	} else if (vinfo.bits_per_pixel == 32) {
		put_pixel_RGB32(x, y, def_r[c], def_g[c], def_b[c]);
	}
}

// helper function to draw a rectangle in given color
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

// helper function to clear the screen - fill whole
// screen with given color
static void
    clear_screen(unsigned short int c)
{
	if (vinfo.bits_per_pixel == 8) {
		memset(fbp, c, finfo.smem_len);
	} else {
		// NOTE: Grayscale palette, we could have used def_r or def_g ;).
		memset(fbp, def_b[c], finfo.smem_len);
	}
}

// Return the font8x8 bitmap for a specifric ascii character
static char*
    font8x8_get_bitmap(int ascii)
{
	// Get the bitmap for that ASCII character
	// TODO: Proper validation (w/ the right array depending on the range, to pickup non-basic stuff)
	if (ascii > 127 || ascii < 0) {
		// Default to space when OOR
		ascii = 0;
		printf("ASCII %d is OOR!\n", ascii);
	}

	return font8x8_basic[ascii];
}

// Render a specific font8x8 glyph into a 8x8 pixmap
static void
    font8x8_render(int ascii, char* glyph_pixmap)
{
	char* bitmap = font8x8_get_bitmap(ascii);

	unsigned short int x;
	unsigned short int y;
	bool               set = false;
	for (x = 0U; x < FONTW; x++) {
		// x: input & output row
		for (y = 0U; y < FONTH; y++) {
			// y: input & output column
			set = bitmap[x] & 1 << y;
			// 'Flatten' our pixmap into a 1D array (0=0,0; 1=0,1; 2=0,2; FONTH=1,0)
			unsigned short int idx = (unsigned short int) (y + (x * FONTW));
			glyph_pixmap[idx]      = set ? 1 : 0;
			// Pixmap format is pretty simple, since we're doing monochrome: 1 means fg, and 0 for bg
		}
	}
}

// Render a specific font8x8 glyph into a 16x16 pixmap
static void
    font8x8_render_x2(int ascii, char* glyph_pixmap)
{
	char* bitmap = font8x8_get_bitmap(ascii);

	unsigned short int x;
	unsigned short int y;
	unsigned short int i;
	unsigned short int j;
	bool               set = false;
	for (i = 0U; i < FONTW; i++) {
		// x: input row, i: output row
		x = i / FONTSIZE_MULT;
		for (j = 0U; j < FONTH; j++) {
			// y: input column, j: output column
			y   = j / FONTSIZE_MULT;
			set = bitmap[x] & 1 << y;
			// 'Flatten' our pixmap into a 1D array (0=0,0; 1=0,1; 2=0,2; FONTH=1,0)
			unsigned short int idx = (unsigned short int) (j + (i * FONTW));
			glyph_pixmap[idx]      = set ? 1 : 0;
			glyph_pixmap[idx + 1]  = set ? 1 : 0;
		}
	}
}

// Render a specific font8x8 glyph into a 32x32 pixmap
static void
    font8x8_render_x4(int ascii, char* glyph_pixmap)
{
	char* bitmap = font8x8_get_bitmap(ascii);

	unsigned short int x;
	unsigned short int y;
	unsigned short int i;
	unsigned short int j;
	bool               set = false;
	for (i = 0U; i < FONTW; i++) {
		// x: input row, i: output row
		x = i / FONTSIZE_MULT;
		for (j = 0U; j < FONTH; j++) {
			// y: input column, j: output column
			y   = j / FONTSIZE_MULT;
			set = bitmap[x] & 1 << y;
			// 'Flatten' our pixmap into a 1D array (0=0,0; 1=0,1; 2=0,2; FONTH=1,0)
			unsigned short int idx = (unsigned short int) (j + (i * FONTW));
			glyph_pixmap[idx]      = set ? 1 : 0;
			glyph_pixmap[idx + 1]  = set ? 1 : 0;
			glyph_pixmap[idx + 2]  = set ? 1 : 0;
			glyph_pixmap[idx + 3]  = set ? 1 : 0;
		}
	}
}

// helper function for drawing - no more need to go mess with
// the main function when just want to change what to draw...
static struct mxcfb_rect
    draw(char*              text,
	 unsigned short int row,
	 unsigned short int col,
	 bool               is_inverted,
	 unsigned short int multiline_offset)
{
	printf("Printing '%s' @ line offset %hu\n", text, multiline_offset);
	unsigned short int fgC = is_inverted ? WHITE : BLACK;
	unsigned short int bgC = is_inverted ? BLACK : WHITE;

	unsigned short int i;
	unsigned short int x;
	unsigned short int y;
	// Adjust row in case we're a continuation of a multi-line print...
	row = (unsigned short int) (row + multiline_offset);

	// Compute the length of our actual string
	// NOTE: We already took care in fbink_print() of making sure that the string passed in text
	//       wouldn't exceed the maximum printable length, MAXCOLS - col
	size_t len = strnlen(text, MAXCOLS);
	printf("StrLen: %zu\n", len);

	// Compute the dimension of the screen region we'll paint to (taking multi-line into account)
	struct mxcfb_rect region = {
		.top    = (uint32_t)((row - multiline_offset) * FONTH),
		.left   = (uint32_t)(col * FONTW),
		.width  = multiline_offset > 0 ? (vinfo.xres - (uint32_t)(col * FONTW)) : (uint32_t)(len * FONTW),
		.height = (uint32_t)((multiline_offset + 1) * FONTH),
	};

	printf("Region: top=%u, left=%u, width=%u, height=%u\n", region.top, region.left, region.width, region.height);

	// NOTE: eInk framebuffers are weird...,
	//       we should be computing the length of a line (MAXCOLS) based on xres_virtual,
	//       not xres (because it's guaranteed to be a multiple of 16).
	//       Unfortunately, that means this last block of the line is partly offscreen.
	//       Also, it CANNOT be part of the region passed to the eInk controller...
	//       So, since this this last block is basically unusable because partly unreadable,
	//       and partly unrefreshable, don't count it as "available" (i.e., by including it in MAXCOLS),
	//       since that would happen to also wreak havoc in a number of our heuristics,
	//       just fudge printing a blank square 'til the edge of the screen if we're filling a line completely.
	if (len == MAXCOLS) {
		fill_rect((unsigned short int) (region.left + (len * FONTW)),
			  (unsigned short int) (region.top + (unsigned short int) (multiline_offset * FONTH)),
			  (unsigned short int) (vinfo.xres - (len * FONTW)),
			  FONTH,
			  bgC);
	}

	// Fill our bounding box with our background color, so that we'll be visible no matter what's already on screen.
	// NOTE: Unneeded, we already plot the background when handling font glyphs ;).
	//fill_rect(region.left, region.top, region.width, region.height, bgC);

	// Alloc our pixmap on the heap, and re-use it.
	// NOTE: We tried using automatic VLAs, but that... didn't go well.
	//       (as in, subtle (or not so) memory and/or stack corruption).
	char* pixmap;
	pixmap = malloc(sizeof(*pixmap) * (size_t) (FONTW * FONTH));

	// Loop through all characters in the text string
	for (i = 0U; i < len; i++) {
		// get the glyph's pixmap
		switch (FONTSIZE_MULT) {
			case 4:
				font8x8_render_x4(text[i], pixmap);
				break;
			case 2:
				font8x8_render_x2(text[i], pixmap);
				break;
			case 1:
			default:
				font8x8_render(text[i], pixmap);
				break;
		}
		// loop through pixel rows
		for (y = 0U; y < FONTH; y++) {
			// loop through pixel columns
			for (x = 0U; x < FONTW; x++) {
				// get the pixel value
				char b = pixmap[(y * FONTW) + x];
				if (b > 0) {
					// plot the pixel (fg, text)
					put_pixel((unsigned short int) ((col * FONTW) + (i * FONTW) + x),
						  (unsigned short int) ((row * FONTH) + y),
						  fgC);
				} else {
					// this is background,
					// fill it so that we'll be visible no matter what was on screen behind us.
					put_pixel((unsigned short int) ((col * FONTW) + (i * FONTW) + x),
						  (unsigned short int) ((row * FONTH) + y),
						  bgC);
				}
			}    // end "for x"
		}            // end "for y"
	}                    // end "for i"

	// Cleanup
	free(pixmap);

	return region;
}

// handle eink updates
static void
    refresh(int fbfd, struct mxcfb_rect region, bool is_flashing)
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
		.waveform_mode = is_flashing ? WAVEFORM_MODE_GC16 : WAVEFORM_MODE_AUTO,
		.flags         = 0,
	};

	if (ioctl(fbfd, MXCFB_SEND_UPDATE, &update) < 0) {
		perror("MXCFB_SEND_UPDATE");
		exit(EXIT_FAILURE);
	}
}

// Magic happens here!
void
    fbink_print(char*     string,
		short int row,
		short int col,
		bool      is_inverted,
		bool      is_flashing,
		bool      is_cleared,
		bool      is_centered,
		bool      is_padded)
{
	int    fbfd       = 0;
	size_t screensize = 0;

	// Open the framebuffer file for reading and writing
	fbfd = open("/dev/fb0", O_RDWR);
	if (!fbfd) {
		printf("Error: cannot open framebuffer device.\n");
		return;
	}
	printf("The framebuffer device was opened successfully.\n");

	// Get variable screen information
	if (ioctl(fbfd, FBIOGET_VSCREENINFO, &vinfo)) {
		printf("Error reading variable information.\n");
	}
	printf("Variable info: %dx%d, %dbpp\n", vinfo.xres, vinfo.yres, vinfo.bits_per_pixel);

	// Set font-size based on screen resolution (roughly matches: Pearl, Carta, Carta HD)
	// FIXME: Even on 600x800 screens, 8x8 might be too small...
	if (vinfo.yres <= 800) {
		FONTSIZE_MULT = 1;    // 8x8
	} else if (vinfo.yres <= 1024) {
		FONTSIZE_MULT = 2;    // 16x16
	} else {
		FONTSIZE_MULT = 4;    // 32x32
	}
	// Go!
	FONTW = (unsigned short int) (FONTW * FONTSIZE_MULT);
	FONTH = (unsigned short int) (FONTH * FONTSIZE_MULT);
	printf("Fontsize set to %dx%d.\n", FONTW, FONTH);

	// Compute MAX* values now that we know the screen & font resolution
	MAXCOLS = (unsigned short int) (vinfo.xres / FONTW);
	MAXROWS = (unsigned short int) (vinfo.yres / FONTH);
	printf("Line length: %hu, Column length: %hu.\n", MAXCOLS, MAXROWS);

	// Get fixed screen information
	if (ioctl(fbfd, FBIOGET_FSCREENINFO, &finfo)) {
		printf("Error reading fixed information.\n");
	}
	printf("Fixed info: smem_len %d, line_length %d\n", finfo.smem_len, finfo.line_length);

	// map fb to user mem
	// NOTE: Beware of smem_len on Kobos?
	//       c.f., https://github.com/koreader/koreader-base/blob/master/ffi/framebuffer_linux.lua#L36
	screensize = finfo.smem_len;
	fbp        = (char*) mmap(NULL, screensize, PROT_READ | PROT_WRITE, MAP_SHARED, fbfd, 0);

	struct mxcfb_rect region;

	if (fbp == MAP_FAILED) {
		perror("mmap");
	} else {
		// Clear screen?
		if (is_cleared) {
			clear_screen(is_inverted ? BLACK : WHITE);
		}

		// See if want to position our text relative to the edge of the screen, and not the beginning
		if (col < 0) {
			col = (short int) MAX(MAXCOLS + col, 0);
		}
		if (row < 0) {
			row = (short int) MAX(MAXROWS + row, 0);
		}
		printf("Adjusted position: column %hu, row %hu\n", col, row);

		// See if we need to break our string down into multiple lines...
		size_t len = strlen(string);
		// Compute the amount of characters (i.e., rows) needed to print that string...
		unsigned short int rows             = (unsigned short int) ((unsigned short int) col + len);
		unsigned short int lines            = 1;
		unsigned short int multiline_offset = 0;
		// NOTE: The maximum length of a single row is the total amount of columns in a row (i.e., line)!
		if (rows > MAXCOLS) {
			lines = (unsigned short int) ceilf((float) rows / (float) MAXCOLS);
		}

		// Truncate to a single screen...
		if (lines > MAXROWS) {
			printf("Can only print %hu out of %hu lines, truncating!\n", MAXROWS, lines);
			lines = MAXROWS;
		}

		// Move our initial row up if we add so much line that some of it goes off-screen...
		if (row + lines > MAXROWS) {
			row = (short int) MIN(row - ((row + lines) - MAXROWS), MAXROWS);
		}
		printf("Final position: column %hu, row %hu\n", col, row);

		// We'll copy our text in chunks of formatted line...
		// NOTE: Store that on the heap, we've had some wonky adventures with automatic VLAs...
		char* line;
		line = malloc(sizeof(*line) * (size_t) (MAXCOLS + 1));

		printf("Need %hu lines to print %zu characters over %hu rows\n", lines, len, rows);
		// If we have multiple lines to print, draw 'em line per line
		for (multiline_offset = 0; multiline_offset < lines; multiline_offset++) {
			// Compute the amount of characters left to print...
			size_t left = len - (size_t)((multiline_offset) * (MAXCOLS - col));
			// And use it to compute the amount of characters to print on *this* line
			size_t line_len = MIN(left, (size_t)(MAXCOLS - col));
			printf("Size to print: %zu out of %zu (left: %zu)\n",
			       line_len,
			       (size_t)(MAXCOLS - col) * sizeof(char),
			       left);

			// Just fudge the column for centering...
			if (is_centered) {
				// When also padding, begin at the edge, since we'll want full padding anyway.
				col = is_padded ? 0 : (short int) ((MAXCOLS / 2) - (line_len / 2));
				printf("Adjusted column to %hu for centering\n", col);
			}
			// Just fudge the (formatted) line length for free padding :).
			if (is_padded) {
				// Don't fudge if also centered, we'll need the original value to split padding in two.
				line_len = is_centered ? line_len : (size_t)(MAXCOLS - col);
				printf("Adjusted line_len to %zu for centering\n", line_len);
			}

			// FIXME: Fix the whole MAXCOLS as field length to snprintf... (plus fix 'scrolling' when col > 0).
			// When centered & padded, we need to split the padding in two, left & right.
			if (is_centered && is_padded) {
				size_t pad_len = (MAXCOLS - line_len) / 2;
				// If we're not at the edge of the screen because of rounding errors,
				// add extra padding on the right
				size_t extra_pad = MAXCOLS - line_len - (pad_len * 2);
				printf("Total size: %zu + %zu + %zu + %zu = %zu\n",
				       pad_len,
				       line_len,
				       pad_len,
				       extra_pad,
				       (pad_len * 2) + line_len + extra_pad);
				snprintf(line,
					 (size_t) (MAXCOLS + 1),
					 "%*s%*s%*s",
					 (int) pad_len,
					 " ",
					 (int) line_len,
					 string + (multiline_offset * (MAXCOLS - col)),
					 (int) (pad_len + extra_pad),
					 " ");
			} else {
				snprintf(line,
					 (size_t) line_len + 1,
					 "%*s",
					 (int) line_len,
					 string + (multiline_offset * (MAXCOLS - col)));
			}

			region = draw(
			    line, (unsigned short int) row, (unsigned short int) col, is_inverted, multiline_offset);
		}

		// Cleanup
		free(line);
	}

	// Fudge the region if we asked for a screen clear, so that we actually refresh the full screen...
	if (is_cleared) {
		region.top    = 0;
		region.left   = 0;
		region.width  = vinfo.xres;
		region.height = vinfo.yres;
	}

	// Refresh screen
	refresh(fbfd, region, is_flashing);

	// cleanup
	munmap(fbp, screensize);
	close(fbfd);
}

// application entry point
int
    main(int argc, char* argv[])
{
	int                        opt;
	int                        opt_index;
	static const struct option opts[] = {
		{ "row", required_argument, NULL, 'y' }, { "col", required_argument, NULL, 'x' },
		{ "invert", no_argument, NULL, 'h' },    { "flash", no_argument, NULL, 'f' },
		{ "clear", no_argument, NULL, 'c' },     { "centered", no_argument, NULL, 'm' },
		{ "padded", no_argument, NULL, 'p' },    { NULL, 0, NULL, 0 }
	};

	short int row         = 0;
	short int col         = 0;
	bool      is_inverted = false;
	bool      is_flashing = false;
	bool      is_cleared  = false;
	bool      is_centered = false;
	bool      is_padded   = false;

	while ((opt = getopt_long(argc, argv, "y:x:hfcmp", opts, &opt_index)) != -1) {
		switch (opt) {
			case 'y':
				row = (short int) atoi(optarg);
				break;
			case 'x':
				col = (short int) atoi(optarg);
				break;
			case 'h':
				is_inverted = true;
				break;
			case 'f':
				is_flashing = true;
				break;
			case 'c':
				is_cleared = true;
				break;
			case 'm':
				is_centered = true;
				break;
			case 'p':
				is_padded = true;
				break;
			default:
				fprintf(stderr, "?? Unknown option code 0%o ??\n", opt);
				return EXIT_FAILURE;
				break;
		}
	}

	char* string;
	if (optind < argc) {
		while (optind < argc) {
			string = argv[optind++];
			printf("Printing%sstring '%s' @ column %hu, row %hu\n",
			       is_inverted ? " inverted " : " ",
			       string,
			       col,
			       row);
			fbink_print(string, row, col, is_inverted, is_flashing, is_cleared, is_centered, is_padded);
			// NOTE: Don't clobber previous entries if multiple strings were passed...
			//row++;
			// FIXME: Actually handle this sanely... :D
		}
	}

	return EXIT_SUCCESS;
}

/*
 * TODO: DOC
 * TODO: Library (thread safety: fbinlk_init to setup globals? /!\ fb fd?)
 * TODO: waveform mode user-selection? -w
 * TODO: ioctl only (i.e., refresh current fb data, don't paint)
 *       -s w=758,h=1024 -f
 *       NOTE: Don't bother w/ getsubopt() and always make it full-screen?
 * TODO: Move all option flags in a struct to keep the sigs in check... (or not? library...)
 */
