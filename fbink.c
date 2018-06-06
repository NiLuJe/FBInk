/*
 * fbtestfnt.c
 *
 * http://raspberrycompote.blogspot.com/2014/04/low-level-graphics-on-raspberry-pi-text.html
 *
 * Original work by J-P Rosti (a.k.a -rst- and 'Raspberry Compote')
 *
 * Licensed under the Creative Commons Attribution 3.0 Unported License
 * (http://creativecommons.org/licenses/by/3.0/deed.en_US)
 *
 * Distributed in the hope that this will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 *
 */

#include <fcntl.h>
#include <getopt.h>
#include <linux/fb.h>
#include <linux/kd.h>
#include <math.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <unistd.h>

#include "eink/mxcfb-kobo.h"
#include "fbink.h"
#include "font8x8/font8x8_latin.h"

// default eInk framebuffer palette
// c.f. linux/drivers/video/mxc/cmap_lab126.h
typedef enum
{
	BLACK  = 0,     // 0x00
	GRAY1  = 1,     // 0x11
	GRAY2  = 2,     // 0x22
	GRAY3  = 3,     // 0x33
	GRAY4  = 4,     // 0x44
	GRAY5  = 5,     // 0x55
	GRAY6  = 6,     // 0x66
	GRAY7  = 7,     // 0x77
	GRAY8  = 8,     // 0x88
	GRAY9  = 9,     // 0x99
	GRAY10 = 10,    // 0xAA
	GRAY11 = 11,    // 0xBB
	GRAY12 = 12,    // 0xCC
	GRAY13 = 13,    // 0xDD
	GRAY14 = 14,    // 0xEE
	WHITE  = 15     // 0xFF
} COLOR_INDEX_T;

static unsigned short def_r[] = { 0x00, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77,
				  0x88, 0x99, 0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xFF };
static unsigned short def_g[] = { 0x00, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77,
				  0x88, 0x99, 0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xFF };
static unsigned short def_b[] = { 0x00, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77,
				  0x88, 0x99, 0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xFF };

// 'global' variables to store screen info
char*                    fbp = 0;
struct fb_var_screeninfo vinfo;
struct fb_fix_screeninfo finfo;
unsigned short int       FONTW         = 8;
unsigned short int       FONTH         = 8;
unsigned short int       FONTSIZE_MULT = 1;
// Slightly arbitrary-ish fallback values
unsigned short int MAXROWS = 45;
unsigned short int MAXCOLS = 32;

// helper function to 'plot' a pixel in given color
void
    put_pixel_Gray8(int x, int y, int c)
{
	// calculate the pixel's byte offset inside the buffer
	unsigned int pix_offset = x + y * finfo.line_length;

	// now this is about the same as 'fbp[pix_offset] = value'
	*((char*) (fbp + pix_offset)) = c;
}

void
    put_pixel_RGB24(int x, int y, int r, int g, int b)
{
	// remember to change main(): vinfo.bits_per_pixel = 24;
	// and: screensize = vinfo.xres * vinfo.yres *
	//                   vinfo.bits_per_pixel / 8;

	// calculate the pixel's byte offset inside the buffer
	// note: x * 3 as every pixel is 3 consecutive bytes
	unsigned int pix_offset = x * 3 + y * finfo.line_length;

	// now this is about the same as 'fbp[pix_offset] = value'
	*((char*) (fbp + pix_offset))     = b;
	*((char*) (fbp + pix_offset + 1)) = g;
	*((char*) (fbp + pix_offset + 2)) = r;
}

void
    put_pixel_RGB32(int x, int y, int r, int g, int b)
{
	// remember to change main(): vinfo.bits_per_pixel = 32;
	// and: screensize = vinfo.xres * vinfo.yres *
	//                   vinfo.bits_per_pixel / 8;

	// calculate the pixel's byte offset inside the buffer
	// note: x * 4 as every pixel is 4 consecutive bytes
	unsigned int pix_offset = x * 4 + y * finfo.line_length;

	// now this is about the same as 'fbp[pix_offset] = value'
	*((char*) (fbp + pix_offset))     = b;
	*((char*) (fbp + pix_offset + 1)) = g;
	*((char*) (fbp + pix_offset + 2)) = r;
	*((char*) (fbp + pix_offset + 3)) = 0xFF;    // Opaque, always.
}

void
    put_pixel_RGB565(int x, int y, int r, int g, int b)
{
	// remember to change main(): vinfo.bits_per_pixel = 16;
	// or on RPi just comment out the whole 'Change vinfo'
	// and: screensize = vinfo.xres * vinfo.yres *
	//                   vinfo.bits_per_pixel / 8;

	// calculate the pixel's byte offset inside the buffer
	// note: x * 2 as every pixel is 2 consecutive bytes
	unsigned int pix_offset = x * 2 + y * finfo.line_length;

	// now this is about the same as 'fbp[pix_offset] = value'
	// but a bit more complicated for RGB565
	unsigned short c = ((r / 8) << 11) + ((g / 4) << 5) + (b / 8);
	// or: c = ((r / 8) * 2048) + ((g / 4) * 32) + (b / 8);
	// write 'two bytes at once'
	*((unsigned short*) (fbp + pix_offset)) = c;
}

// handle various bpp...
void
    put_pixel(int x, int y, int c)
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
void
    fill_rect(int x, int y, int w, int h, int c)
{
	int cx, cy;
	for (cy = 0; cy < h; cy++) {
		for (cx = 0; cx < w; cx++) {
			put_pixel(x + cx, y + cy, c);
		}
	}
	//printf("filled %dx%d rectangle @ %d, %d\n", w, h, x, y);
}

// helper function to clear the screen - fill whole
// screen with given color
void
    clear_screen(int c)
{
	if (vinfo.bits_per_pixel == 8) {
		memset(fbp, c, finfo.smem_len);
	} else {
		// NOTE: Grayscale palette, we could have used def_r or def_g ;).
		memset(fbp, def_b[c], finfo.smem_len);
	}
}

// Return the font8x8 bitmap for a specifric ascii character
char*
    font8x8_get_bitmap(int ascii)
{
	// Get the bitmap for that ASCII character
	// TODO: Proper validation (w/ the right array depending on the range, to pickup non-basic stuff)
	if (ascii > 127 || ascii < 0) {
		// Default to space when OOR
		ascii = 0;
	}

	return font8x8_basic[ascii];
}

// Render a specific font8x8 glyph into a 8x8 pixmap
void
    font8x8_render(int ascii, char* glyph_pixmap)
{
	char* bitmap = font8x8_get_bitmap(ascii);

	int  x, y = 0;
	bool set = false;
	for (x = 0; x < FONTW; x++) {
		// x: input & output row
		for (y = 0; y < FONTH; y++) {
			// y: input & output column
			set = bitmap[x] & 1 << y;
			// 'Flatten' our pixmap into a 1D array (0=0,0; 1=0,1; 2=0,2; FONTH=1,0)
			//int idx = x + (y * FONTH);	// 90° Left rotattion ;).
			int idx           = y + (x * FONTW);
			glyph_pixmap[idx] = set ? 1 : 0;
			// Pixmap format is pretty simple, since we're doing monochrome: 1 means fg, and 0 for bg
		}
	}
}

// Render a specific font8x8 glyph into a 16x16 pixmap
void
    font8x8_render_x2(int ascii, char* glyph_pixmap)
{
	char* bitmap = font8x8_get_bitmap(ascii);

	int  x, y, i, j = 0;
	bool set = false;
	for (i = 0; i < FONTW; i++) {
		// x: input row, i: output row
		x = i / FONTSIZE_MULT;
		for (j = 0; j < FONTH; j++) {
			// y: input column, j: output column
			y   = j / FONTSIZE_MULT;
			set = bitmap[x] & 1 << y;
			// 'Flatten' our pixmap into a 1D array (0=0,0; 1=0,1; 2=0,2; FONTH=1,0)
			int idx               = j + (i * FONTW);
			glyph_pixmap[idx]     = set ? 1 : 0;
			glyph_pixmap[idx + 1] = set ? 1 : 0;
		}
	}
}

// Render a specific font8x8 glyph into a 32x32 pixmap
void
    font8x8_render_x4(int ascii, char* glyph_pixmap)
{
	char* bitmap = font8x8_get_bitmap(ascii);

	int  x, y, i, j = 0;
	bool set = false;
	for (i = 0; i < FONTW; i++) {
		// x: input row, i: output row
		x = i / FONTSIZE_MULT;
		for (j = 0; j < FONTH; j++) {
			// y: input column, j: output column
			y   = j / FONTSIZE_MULT;
			set = bitmap[x] & 1 << y;
			// 'Flatten' our pixmap into a 1D array (0=0,0; 1=0,1; 2=0,2; FONTH=1,0)
			int idx               = j + (i * FONTW);
			glyph_pixmap[idx]     = set ? 1 : 0;
			glyph_pixmap[idx + 1] = set ? 1 : 0;
			glyph_pixmap[idx + 2] = set ? 1 : 0;
			glyph_pixmap[idx + 3] = set ? 1 : 0;
		}
	}
}

// helper function for drawing - no more need to go mess with
// the main function when just want to change what to draw...
struct mxcfb_rect
    draw(char* text, unsigned short int row, unsigned short int col, bool is_inverted, unsigned short int line_offset)
{

	printf("Printing '%s' @ line offset %hu\n", text, line_offset);
	int fgC = is_inverted ? WHITE : BLACK;
	int bgC = is_inverted ? BLACK : WHITE;

	unsigned short int i, x, y;
	// Adjust row in case we're a continuation of a multi-line print...
	row += line_offset;

	// Compute the length of our actual string
	// NOTE: We already took care in fbink_print() of making sure that the string passed in text
	//       wouldn't exceed the maximum printable length, MAXCOLS - col
	size_t len = strnlen(text, MAXCOLS);
	printf("StrLen: %zu\n", len);

	// Compute the dimension of the screen region we'll paint to (taking multi-line into account)
	struct mxcfb_rect region = {
		.top    = (row - line_offset) * FONTH,
		.left   = col * FONTW,
		.width  = line_offset > 0 ? (MAXCOLS - col) * FONTW : len * FONTW,
		.height = (line_offset + 1) * FONTH,
	};

	printf("Region: top=%u, left=%u, width=%u, height=%u\n", region.top, region.left, region.width, region.height);

	// Fill our bounding box with our background color, so that we'll be visible no matter what's already on screen.
	// NOTE: Unneeded, we already plot the background when handling font glyphs ;).
	//fill_rect(region.left, region.top, region.width, region.height, bgC);

	// Loop through all characters in the text string
	for (i = 0; i < len; i++) {
		// get the 'image' index for this character
		//int ix = font_index(text[i]);
		// get the font 'image'
		//char* img = fontImg[ix];
		char img[FONTW * FONTH];
		// Make sure the array is zero-initialized...
		// NOTE: That sizeof may feel weird, but in C99, it does get evaluated at runtime :).
		//       Otherwise, we'd need to do memset(img, 0, (FONTW * FONTH) * sizeof(*img));
		//       Which, granted, should be equal to simply memset(img, 0, FONTW * FONTH);
		memset(img, 0, sizeof(img));
		switch (FONTSIZE_MULT) {
			case 4:
				font8x8_render_x4(text[i], img);
				break;
			case 2:
				font8x8_render_x2(text[i], img);
				break;
			case 1:
			default:
				font8x8_render(text[i], img);
				break;
		}
		// loop through pixel rows
		for (y = 0; y < FONTH; y++) {
			// loop through pixel columns
			for (x = 0; x < FONTW; x++) {
				// get the pixel value
				char b = img[y * FONTW + x];
				if (b > 0) {
					// plot the pixel (fg, text)
					put_pixel((col * FONTW) + (i * FONTW) + x, (row * FONTH) + y, fgC);
				} else {
					// this is background, fill it so that we'll be visible no matter what was on screen behind us.
					put_pixel((col * FONTW) + (i * FONTW) + x, (row * FONTH) + y, bgC);
				}
			}    // end "for x"
		}            // end "for y"
	}                    // end "for i"

	return region;
}

// handle eink updates
void
    refresh(int fbfd, struct mxcfb_rect region, bool is_flashing)
{
	// NOTE: While we'd be perfect candidates for using A2 waveform mode, it's all kinds of fucked up on Kobos,
	//       and may lead to disappearing text or weird blending depending on the surrounding fb content...
	//       It only shows up properly when FULL, which isn't great...
	//       Also, we need to set the EPDC_FLAG_FORCE_MONOCHROME flag to do it right.
	// NOTE: And while we're on the fun quirks train: FULL doesn't flash w/ AUTO on some Kobos...
	struct mxcfb_update_data update = {
		.temp          = TEMP_USE_AMBIENT,
		.update_marker = getpid(),
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
    fbink_print(char*              string,
		unsigned short int row,
		unsigned short int col,
		bool               is_inverted,
		bool               is_flashing,
		bool               is_cleared)
{
	int      fbfd       = 0;
	long int screensize = 0;

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
	FONTW = FONTW * FONTSIZE_MULT;
	FONTH = FONTH * FONTSIZE_MULT;
	printf("Fontsize set to %dx%d.\n", FONTW, FONTH);

	// Compute MAX* values now that we know the screen & font resolution
	MAXCOLS = vinfo.xres / FONTW;
	MAXROWS = vinfo.yres / FONTH;
	printf("Line length: %hu, Column length: %hu.\n", MAXCOLS, MAXROWS);

	// Get fixed screen information
	if (ioctl(fbfd, FBIOGET_FSCREENINFO, &finfo)) {
		printf("Error reading fixed information.\n");
	}
	printf("Fixed info: smem_len %d, line_length %d\n", finfo.smem_len, finfo.line_length);

	// map fb to user mem
	// NOTE: Beware of smem_len on Kobos? c.f., https://github.com/koreader/koreader-base/blob/master/ffi/framebuffer_linux.lua#L36
	screensize = finfo.smem_len;
	fbp        = (char*) mmap(0, screensize, PROT_READ | PROT_WRITE, MAP_SHARED, fbfd, 0);

	struct mxcfb_rect region;

	if ((int) fbp == -1) {
		printf("Failed to mmap.\n");
	} else {
		// Clear screen?
		if (is_cleared) {
			clear_screen(is_inverted ? BLACK : WHITE);
		}

		// See if we need to break our string down into multiple lines...
		char   line[MAXCOLS];
		size_t len = strlen(string);
		// Compute the amount of characters (i.e., rows) needed to print that string...
		size_t             rows             = col + len;
		unsigned short int lines            = 1;
		unsigned short int multiline_offset = 0;
		// NOTE: The maximum length of a single row is the total amount of columns in a row (i.e., line)!
		if (rows > MAXCOLS) {
			lines = ceilf((float) rows / (float) MAXCOLS);
		}
		printf("Need %hu lines to print %zu characters in %zu rows\n", lines, len, rows);
		// If we have multiple lines to print, draw 'em line per line
		for (multiline_offset = 0; multiline_offset < lines; multiline_offset++) {
			printf("Size to print: %zu\n", (MAXCOLS - col) * sizeof(char));
			strncpy(line, string + (multiline_offset * (MAXCOLS - col)), (MAXCOLS - col) * sizeof(char));
			// Ensure line is NULL terminated so that stuff stays sane later :).
			line[MAXCOLS - col] = '\0';
			region              = draw(line, row, col, is_inverted, multiline_offset);
		}
		// draw...
		// FIXME: Fuck it , and chunk the draw calls from here...
		// MACOL/MAXROW
		// multiline_offset
		// count line_number, for multiline_offset < line_number
		// Support negative col/row, meaning MAXCOL-/MAXROW- (start from the end)
		// Adjust row if row + line_number > MAXROW
		// If adjusted row + line_number > MAXROW -> truncate
		// Make sure adjusted row/col >= 0
		//region = draw(string, row, col, is_inverted, 0);
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
		{ "clear", no_argument, NULL, 'c' },     { NULL, 0, NULL, 0 }
	};

	unsigned short int row         = 0;
	unsigned short int col         = 0;
	bool               is_inverted = false;
	// NOTE: Not terribly useful for text-only, it's often optimized out by the driver for small regions (i.e., us).
	bool is_flashing = false;
	// TODO: Unimplemented (because fairly useless for text only).
	bool is_cleared = false;

	while ((opt = getopt_long(argc, argv, "y:x:hfc", opts, &opt_index)) != -1) {
		switch (opt) {
			case 'y':
				row = atoi(optarg);
				break;
			case 'x':
				col = atoi(optarg);
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
			fbink_print(string, row, col, is_inverted, is_flashing, is_cleared);
		}
	}

	return EXIT_SUCCESS;
}

/*
 * TODO: Truncate/Multi-Line
 * TODO: Header
 * TODO: Makefile
 * TODO: License
 * TODO: DOC
 * TODO: Library (thread safety: fbinlk_init to setup globals? /!\ fb fd?)
 * TODO: waveform mode user-selection? -w
 * TODO: ioctl only (i.e., refresh current fb data, don't paint)
 *       -s w=758,h=1024 -f
 *       NOTE: Don't bother w/ getsubopt() and always make it full-screen?
 * TODO: Centered text, padded/non-padded
 */
