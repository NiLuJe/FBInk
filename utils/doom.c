/*
	FBInk: FrameBuffer eInker, a library to print text & images to an eInk Linux framebuffer
	Copyright (C) 2018-2023 NiLuJe <ninuje@gmail.com>
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

// Because we're pretty much Linux-bound ;).
#ifndef _GNU_SOURCE
#	define _GNU_SOURCE
#endif

// NOTE: We need image support (chiefly for stbi__compute_y)
//       A MINIMAL + IMAGE build is still recommended, because otherwise fbink_init() has to pull all the extra fonts in...
#ifdef FBINK_MINIMAL
#	ifndef FBINK_WITH_IMAGE
#		error Cannot build this tool without Image support!
#	endif
#endif

#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
// I feel dirty.
#include "../fbink.c"
// Really dirty (for u8_cp_to_utf8).
#include "../cutef8/utf8.c"

int fbFd = -1;

static bool
    get_fbinfo(void)
{
	// Get variable fb info
	if (ioctl(fbFd, FBIOGET_VSCREENINFO, &vInfo)) {
		perror("ioctl GET_V");
		return false;
	}
	LOG("Variable fb info: %ux%u (%ux%u), %ubpp @ rotation: %u (%s)",
	    vInfo.xres,
	    vInfo.yres,
	    vInfo.xres_virtual,
	    vInfo.yres_virtual,
	    vInfo.bits_per_pixel,
	    vInfo.rotate,
	    fb_rotate_to_string(vInfo.rotate));
	// Get fixed fb information
	if (ioctl(fbFd, FBIOGET_FSCREENINFO, &fInfo)) {
		perror("ioctl GET_F");
		return false;
	}
	LOG("Fixed fb info: ID is \"%s\", length of fb mem: %u bytes & line length: %u bytes",
	    fInfo.id,
	    fInfo.smem_len,
	    fInfo.line_length);

	return true;
}

#ifndef FBINK_FOR_LINUX
#	ifndef FBINK_FOR_KINDLE
static bool
    set_fbinfo(uint32_t bpp, int8_t rota)
{
	// Set variable fb info
	// Bitdepth
	vInfo.bits_per_pixel = bpp;
	LOG("Setting bitdepth to %ubpp", vInfo.bits_per_pixel);
	// Grayscale flag
	if (bpp == 8U) {
		// NOTE: 1 for Grayscale, 2 for Inverted Grayscale (like on einkfb).
		//       We obviously don't want to inflict an inverted palette on ourselves ;).
		//       c.f., GRAYSCALE_* defines @ mxcfb.h
		vInfo.grayscale = (uint32_t) GRAYSCALE_8BIT;
		LOG("Setting grayscale to %u", vInfo.grayscale);
	} else {
		// NOTE: And of course, 0 for color ;)
		vInfo.grayscale = (uint32_t) 0U;
		LOG("Setting grayscale to %u", vInfo.grayscale);
	}

	// NOTE: We have to counteract the rotation shenanigans the Kernel might be enforcing...
	//       c.f., mxc_epdc_fb_check_var @ drivers/video/mxc/mxc_epdc_fb.c OR drivers/video/fbdev/mxc/mxc_epdc_v2_fb.c
	//       The goal being to end up in the *same* effective rotation as before.
	// First, remember the current rotation as the expected one...
	uint32_t expected_rota = vInfo.rotate;
	// Then, set the requested rotation, if there was one...
	if (rota != -1) {
		vInfo.rotate = (uint32_t) rota;
		LOG("Setting rotate to %u (%s)", vInfo.rotate, fb_rotate_to_string(vInfo.rotate));
		// And flag it as the expected rota for the sanity checks
		expected_rota = (uint32_t) rota;
	}
	if (deviceQuirks.ntxRotaQuirk == NTX_ROTA_ALL_INVERTED) {
		// NOTE: This should cover the H2O and the few other devices suffering from the same quirk...
		vInfo.rotate ^= 2;
		LOG("Setting rotate to %u (%s) to account for kernel rotation quirks",
		    vInfo.rotate,
		    fb_rotate_to_string(vInfo.rotate));
	} else if (deviceQuirks.ntxRotaQuirk == NTX_ROTA_ODD_INVERTED) {
		// NOTE: This is for the Forma, which only inverts CW & CCW (i.e., odd numbers)...
		if ((vInfo.rotate & 0x01) == 1) {
			vInfo.rotate ^= 2;
			LOG("Setting rotate to %u (%s) to account for kernel rotation quirks",
			    vInfo.rotate,
			    fb_rotate_to_string(vInfo.rotate));
		}
	}

	if (ioctl(fbFd, FBIOPUT_VSCREENINFO, &vInfo)) {
		perror("ioctl PUT_V");
		return false;
	}

	// NOTE: Double-check that we weren't bit by rotation quirks...
	if (vInfo.rotate != expected_rota) {
		LOG("\nCurrent rotation (%u) doesn't match the expected rotation (%u), attempting to fix it . . .",
		    vInfo.rotate,
		    expected_rota);

		// Brute-force it until it matches...
		for (uint32_t i = vInfo.rotate, j = FB_ROTATE_UR; j <= FB_ROTATE_CCW; i = (i + 1U) & 3U, j++) {
			// If we finally got the right orientation, break the loop
			if (vInfo.rotate == expected_rota) {
				break;
			}
			// Do the i -> i + 1 -> i dance to be extra sure...
			// (This is useful on devices where the kernel *always* switches to the invert orientation, c.f., rota.c)
			vInfo.rotate = i;
			if (ioctl(fbFd, FBIOPUT_VSCREENINFO, &vInfo)) {
				perror("ioctl PUT_V");
				return false;
			}
			LOG("Kernel rotation quirk recovery: %u -> %u", i, vInfo.rotate);

			// Don't do anything extra if that was enough...
			if (vInfo.rotate == expected_rota) {
				continue;
			}
			// Now for i + 1 w/ wraparound, since the valid rotation range is [0..3] (FB_ROTATE_UR to FB_ROTATE_CCW).
			// (i.e., a Portrait/Landscape swap to counteract potential side-effects of a kernel-side mandatory invert)
			uint32_t n   = (i + 1U) & 3U;
			vInfo.rotate = n;
			if (ioctl(fbFd, FBIOPUT_VSCREENINFO, &vInfo)) {
				perror("ioctl PUT_V");
				return false;
			}
			LOG("Kernel rotation quirk recovery (intermediary @ %u): %u -> %u", i, n, vInfo.rotate);

			// And back to i, if need be...
			if (vInfo.rotate == expected_rota) {
				continue;
			}
			vInfo.rotate = i;
			if (ioctl(fbFd, FBIOPUT_VSCREENINFO, &vInfo)) {
				perror("ioctl PUT_V");
				return false;
			}
			LOG("Kernel rotation quirk recovery: %u -> %u", i, vInfo.rotate);
		}
	}

	// Finally, warn if things *still* look FUBAR...
	if (vInfo.rotate != expected_rota) {
		LOG("\nCurrent rotation (%u) doesn't match the expected rotation (%u), here be dragons!",
		    vInfo.rotate,
		    expected_rota);
	}

	LOG("Bitdepth is now %ubpp (grayscale: %u) @ rotate: %u (%s)\n",
	    vInfo.bits_per_pixel,
	    vInfo.grayscale,
	    vInfo.rotate,
	    fb_rotate_to_string(vInfo.rotate));

	return true;
}
#	endif    // !FBINK_FOR_KINDLE
#endif            // !FBINK_FOR_LINUX

// Shiny DOOM Fire effect
// c.f., http://fabiensanglard.net/doom_fire_psx/index.html
//     & https://github.com/fabiensanglard/DoomFirePSX/blob/master/flames.html
//     & https://github.com/cylgom/ly/blob/master/src/draw.c
static const uint8_t fire_colors[][3] = {
	{0x07, 0x07, 0x07},
        {0x1F, 0x07, 0x07},
        {0x2F, 0x0F, 0x07},
        {0x47, 0x0F, 0x07},
        {0x57, 0x17, 0x07},
	{0x67, 0x1F, 0x07},
        {0x77, 0x1F, 0x07},
        {0x8F, 0x27, 0x07},
        {0x9F, 0x2F, 0x07},
        {0xAF, 0x3F, 0x07},
	{0xBF, 0x47, 0x07},
        {0xC7, 0x47, 0x07},
        {0xDF, 0x4F, 0x07},
        {0xDF, 0x57, 0x07},
        {0xDF, 0x57, 0x07},
	{0xD7, 0x5F, 0x07},
        {0xD7, 0x5F, 0x07},
        {0xD7, 0x67, 0x0F},
        {0xCF, 0x6F, 0x0F},
        {0xCF, 0x77, 0x0F},
	{0xCF, 0x7F, 0x0F},
        {0xCF, 0x87, 0x17},
        {0xC7, 0x87, 0x17},
        {0xC7, 0x8F, 0x17},
        {0xC7, 0x97, 0x1F},
	{0xBF, 0x9F, 0x1F},
        {0xBF, 0x9F, 0x1F},
        {0xBF, 0xA7, 0x27},
        {0xBF, 0xA7, 0x27},
        {0xBF, 0xAF, 0x2F},
	{0xB7, 0xAF, 0x2F},
        {0xB7, 0xB7, 0x2F},
        {0xB7, 0xB7, 0x37},
        {0xCF, 0xCF, 0x6F},
        {0xDF, 0xDF, 0x9F},
	{0xEF, 0xEF, 0xC7},
        {0xFF, 0xFF, 0xFF},
};

#ifndef FBINK_FOR_LINUX
uint8_t fire_palette[sizeof(fire_colors) / sizeof(*fire_colors)];

// If I dumbly quantize that to the eInk palette, that's what this ends up as...
// NOTE: That doesn't work so well in practice, though...
/*
static const uint8_t palette_eink[] = { 0x00, 0x11, 0x11, 0x22, 0x33, 0x33, 0x44, 0x55, 0x55, 0x66, 0x77, 0x77, 0x88,
					0x88, 0x88, 0x88, 0x88, 0x88, 0x88, 0x88, 0x99, 0x99, 0x99, 0x99, 0x99, 0xAA,
					0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xCC, 0xDD, 0xEE, 0xFF };
*/

static void
    setup_palette(void)
{
	// Convert the palette to Grayscale
	for (uint8_t i = 0U; i < sizeof(fire_palette); i++) {
		fire_palette[i] = stbi__compute_y(fire_colors[i][0U], fire_colors[i][1U], fire_colors[i][2U]);
	}
}

static unsigned int
    find_palette_id(uint8_t v)
{
	for (uint8_t i = 0U; i < sizeof(fire_palette); i++) {
		if (fire_palette[i] == v) {
			return i;
		}
	}

	// Should hopefully never happen...
	return 0U;
}

static void
    spread_fire_fs(size_t offset)
{
	uint8_t pixel = *((uint8_t*) (fbPtr + offset));
	if (pixel == fire_palette[0U]) {
		*((uint8_t*) (fbPtr + offset - fInfo.line_length)) = fire_palette[0U];
	} else {
		const size_t random                             = (rand() * 3) & 3;
		const size_t dst                                = offset - random + 1U;
		*((uint8_t*) (fbPtr + dst - fInfo.line_length)) = (uint8_t) (pixel - (random & 1U));
		// Huh, if we go the palette way here, the fire doesn't reach as high...
		/*
		const unsigned int pal_idx                      = find_palette_id(pixel);
		*((uint8_t*) (fbPtr + dst - fInfo.line_length)) = fire_palette[(pal_idx - (random & 1U))];
		*/
	}
}

static void
    do_fire_fs(void)
{
	const uint32_t vertViewport = (uint32_t) (viewVertOrigin - viewVertOffset);
	// Burn baby, burn!
	// NOTE: Switching the outer loop to the y one leads to different interactions w/ the PXP & the EPDC...
	//       Also, slightly uglier flames ;p.
	for (uint32_t x = 0U; x < fInfo.line_length; x++) {
		for (uint32_t y = 1U + vertViewport; y < viewHeight + vertViewport; y++) {
			spread_fire_fs(y * fInfo.line_length + x);
		}
	}
}

static void
    setup_fire_fs(void)
{
	// Convert the palette to Grayscale
	setup_palette();

	// Fill the whole screen w/ color 0
	memset(fbPtr, fire_palette[0U], fInfo.smem_len);

	// Set the bottom line to the final color
	const FBInkPixel px           = { .gray8 = fire_palette[sizeof(fire_palette) - 1U] };
	const uint32_t   vertViewport = (uint32_t) (viewVertOrigin - viewVertOffset);
	fill_rect_Gray8(
	    0U, (unsigned short int) (viewHeight + vertViewport - 1U), (unsigned short int) fInfo.line_length, 1U, &px);
}

// And now for a -- hopefully -- slightly less taxing version, in a smaller window...
#	define FIRE_WIDTH  320U
#	define FIRE_HEIGHT 168U
unsigned short int fire_y_origin;
unsigned short int fire_x_origin;

static void
    setup_fire(void)
{
	// Convert the palette to Grayscale
	setup_palette();

	// Compute window position (centered)
	const uint32_t vertViewport = (uint32_t) (viewVertOrigin - viewVertOffset);
	fire_y_origin               = (unsigned short int) (viewHeight / 2U - FIRE_HEIGHT / 2U + vertViewport);
	fire_x_origin               = (unsigned short int) (viewWidth / 2U - FIRE_WIDTH / 2U);

	// Fill the window w/ color 0
	const FBInkPixel bg = { .gray8 = fire_palette[0U] };
	fill_rect_Gray8(fire_x_origin, fire_y_origin, FIRE_WIDTH, FIRE_HEIGHT, &bg);

	// Set the bottom line to the final color
	const FBInkPixel fire = { .gray8 = fire_palette[sizeof(fire_palette) - 1U] };
	fill_rect_Gray8(fire_x_origin,
			(unsigned short int) (fire_y_origin + FIRE_HEIGHT - 1U),
			(unsigned short int) FIRE_WIDTH,
			1U,
			&fire);
}

static void
    spread_fire(size_t offset, uint32_t x, uint32_t y)
{
	uint8_t pixel = *((uint8_t*) (fbPtr + offset));
	if (pixel == fire_palette[0U]) {
		*((uint8_t*) (fbPtr + offset - fInfo.line_length)) = fire_palette[0U];
	} else {
		const size_t       random  = (rand() * 3) & 3;
		// Make sure we stay within our window...
		const size_t       shift   = ((y - fire_y_origin) * FIRE_WIDTH + (x - fire_x_origin)) - random + 1U;
		const size_t       dst_y   = shift / FIRE_WIDTH + fire_y_origin;
		const size_t       dst_x   = shift % FIRE_WIDTH + fire_x_origin;
		const size_t       dst     = dst_y * fInfo.line_length + dst_x;
		// We'll need the palette id of the current pixel so we can swap it to another *palette* color!
		const unsigned int pal_idx = find_palette_id(pixel);
		*((uint8_t*) (fbPtr + dst - fInfo.line_length)) = fire_palette[(pal_idx - (random & 1U))];
	}
}

static void
    do_fire(void)
{
	// Burn baby, burn!
	// NOTE: Switching the outer loop to the y one leads to different interactions w/ the PXP & the EPDC...
	//       Also, slightly uglier flames ;p.
	for (uint32_t x = fire_x_origin; x < FIRE_WIDTH + fire_x_origin; x++) {
		for (uint32_t y = 1U + fire_y_origin; y < FIRE_HEIGHT + fire_y_origin; y++) {
			spread_fire(y * fInfo.line_length + x, x, y);
		}
	}
}

// And a slight variation with scaling...
uint32_t scaled_Width  = FIRE_WIDTH;
uint32_t scaled_Height = FIRE_HEIGHT;
uint8_t  fire_canvas[FIRE_WIDTH * FIRE_HEIGHT];

static void
    setup_fire_scaled(uint8_t scale)
{
	// Convert the palette to Grayscale
	setup_palette();

	// Compute window size
	scaled_Width  = scale * FIRE_WIDTH;
	scaled_Height = scale * FIRE_HEIGHT;

	// Compute window position (centered)
	const uint32_t vertViewport = (uint32_t) (viewVertOrigin - viewVertOffset);
	fire_y_origin               = (unsigned short int) (viewHeight / 2U - scaled_Height / 2U + vertViewport);
	fire_x_origin               = (unsigned short int) (viewWidth / 2U - scaled_Width / 2U);

	// Fill the window w/ color 0
	const FBInkPixel bg = { .gray8 = fire_palette[0U] };
	fill_rect_Gray8(
	    fire_x_origin, fire_y_origin, (unsigned short int) scaled_Width, (unsigned short int) scaled_Height, &bg);
	// And the source canvas
	memset(fire_canvas, fire_palette[0U], sizeof(fire_canvas));

	// Set the bottom line to the final color
	const FBInkPixel fire = { .gray8 = fire_palette[sizeof(fire_palette) - 1U] };
	fill_rect_Gray8(fire_x_origin,
			(unsigned short int) (fire_y_origin + scaled_Height - 1U),
			(unsigned short int) scaled_Width,
			scale,
			&fire);
	// Again, the source canvas
	memset(fire_canvas + ((FIRE_HEIGHT - 1U) * FIRE_WIDTH), fire_palette[sizeof(fire_palette) - 1U], FIRE_WIDTH);
}

static void
    spread_fire_scaled(size_t offset, uint32_t x, uint32_t y, uint8_t scale)
{
	uint8_t pixel = *((uint8_t*) (fire_canvas + offset));
	if (pixel == fire_palette[0U]) {
		// Update the source canvas
		*((uint8_t*) (fire_canvas + offset - FIRE_WIDTH)) = fire_palette[0U];
		// Update the fb
		const FBInkPixel px                               = { .gray8 = fire_palette[0U] };
		fill_rect_Gray8((unsigned short int) (fire_x_origin + (x * scale)),
				(unsigned short int) (fire_y_origin + ((y * scale) - 1U)),
				scale,
				scale,
				&px);
	} else {
		const size_t       random                      = (rand() * 3) & 3;
		// Update the source canvas
		const size_t       dst                         = offset - random + 1U;
		// We'll need the palette id of the current pixel so we can swap it to another *palette* color!
		const unsigned int pal_idx                     = find_palette_id(pixel);
		*((uint8_t*) (fire_canvas + dst - FIRE_WIDTH)) = fire_palette[(pal_idx - (random & 1U))];
		// Update the fb
		const size_t     dst_y                         = dst / FIRE_WIDTH * scale + fire_y_origin;
		const size_t     dst_x                         = dst % FIRE_WIDTH * scale + fire_x_origin;
		const FBInkPixel px                            = { .gray8 = fire_palette[(pal_idx - (random & 1U))] };
		fill_rect_Gray8((unsigned short int) dst_x, (unsigned short int) (dst_y - 1U), scale, scale, &px);
	}
}

static void
    do_fire_scaled(uint8_t scale)
{
	// Burn baby, burn!
	// NOTE: Switching the outer loop to the y one leads to different interactions w/ the PXP & the EPDC...
	//       Also, slightly uglier flames ;p.
	for (uint32_t x = 0U; x < FIRE_WIDTH; x++) {
		for (uint32_t y = 1U; y < FIRE_HEIGHT; y++) {
			spread_fire_scaled(y * FIRE_WIDTH + x, x, y, scale);
		}
	}
}

static void
    drown_fire_scaled(size_t offset, uint32_t x, uint32_t y, uint8_t scale)
{
	uint8_t pixel = *((uint8_t*) (fire_canvas + offset));
	if (pixel > fire_palette[0U]) {
		const size_t       random            = rand() & 3;
		// We'll need the palette id of the current pixel so we can swap it to another *palette* color!
		const unsigned int pal_idx           = find_palette_id(pixel);
		// Update the source canvas
		*((uint8_t*) (fire_canvas + offset)) = fire_palette[(pal_idx - random)];
		// Update the fb
		const FBInkPixel px                  = { .gray8 = fire_palette[(pal_idx - random)] };
		fill_rect_Gray8((unsigned short int) (fire_x_origin + (x * scale)),
				(unsigned short int) (fire_y_origin + (y * scale)),
				scale,
				scale,
				&px);
	}
}

static void
    stop_fire_scaled(uint8_t scale)
{
	// Psssshhhhht!
	for (uint32_t y = FIRE_HEIGHT - 1U; y > FIRE_HEIGHT - (FIRE_HEIGHT / 28U) - 1U; y--) {
		for (uint32_t x = 0U; x < FIRE_WIDTH; x++) {
			drown_fire_scaled(y * FIRE_WIDTH + x, x, y, scale);
		}
	}
}

#else

// FIXME: Slow & crashy ;).
static unsigned int
    find_palette_id_32(uint8_t r, uint8_t g, uint8_t b)
{
	for (uint8_t i = 0U; i < sizeof(fire_colors) / sizeof(*fire_colors); i++) {
		if (fire_colors[i][0U] == r && fire_colors[i][1U] == g && fire_colors[i][2U] == b) {
			return i;
		}
	}

	// Should hopefully never happen...
	return 0U;
}

static void
    spread_fire_32(size_t offset)
{
	FBInkPixel px;
	px.bgra.p = *((uint32_t*) (fbPtr + offset));
	if (px.bgra.color.r == fire_colors[0U][0U] && px.bgra.color.g == fire_colors[0U][1U] &&
	    px.bgra.color.b == fire_colors[0U][2U]) {
		*((uint32_t*) (fbPtr + offset - fInfo.line_length)) = px.bgra.p;
	} else {
		const size_t       random  = (rand() * 3) & 3;
		const size_t       dst     = offset - random + 1U;
		const unsigned int pal_idx = find_palette_id_32(px.bgra.color.r, px.bgra.color.g, px.bgra.color.b);
		const size_t       idx     = pal_idx - (random & 1U);
		// cppcheck-suppress unreadVariable ; false-positive (union)
		px.bgra.color.r            = fire_colors[idx][0U];
		// cppcheck-suppress unreadVariable ; false-positive (union)
		px.bgra.color.g            = fire_colors[idx][1U];
		// cppcheck-suppress unreadVariable ; false-positive (union)
		px.bgra.color.b            = fire_colors[idx][2U];
		*((uint32_t*) (fbPtr + dst - fInfo.line_length)) = px.bgra.p;
	}
}

static void
    do_fire_32(void)
{
	// Burn baby, burn!
	for (uint32_t x = 0U; x < viewWidth; x++) {
		for (uint32_t y = 1U; y < viewHeight; y++) {
			spread_fire_32((y * fInfo.line_length) + (x << 2U));
		}
	}
}

static void
    setup_fire_32(void)
{
	// Fill the whole screen w/ color 0
	const FBInkPixel bg = { .bgra.color.r = fire_colors[0U][0U],
				.bgra.color.g = fire_colors[0U][1U],
				.bgra.color.b = fire_colors[0U][2U] };
	fill_rect_RGB32(0U, 0U, (unsigned short int) viewWidth, (unsigned short int) viewHeight, &bg);

	// Set the bottom line to the final color
	const size_t     idx = sizeof(fire_colors) / sizeof(*fire_colors) - 1U;
	const FBInkPixel px  = { .bgra.color.r = fire_colors[idx][0U],
				 .bgra.color.g = fire_colors[idx][1U],
				 .bgra.color.b = fire_colors[idx][2U] };
	fill_rect_RGB32(0U, (unsigned short int) (viewHeight - 1U), (unsigned short int) viewWidth, 1U, &px);
}
#endif    // !FBINK_FOR_LINUX

#define BILLION  1000000000L
#define MILLION  1000000.f
#define THOUSAND 1000

// Help message
static void
    show_helpmsg(void)
{
	printf(
	    "\n"
	    "Doom Fire (via FBInk %s)\n"
	    "\n"
	    "Usage: doom [-f | -S] -FtWDlc\n"
	    "\n"
	    "Shiny!\n"
	    "\n"
	    "OPTIONS:\n"
	    "\t-h, --help\t\t\tShow this help message.\n"
	    "\t-v, --verbose\t\t\tToggle printing diagnostic messages.\n"
	    "\t-q, --quiet\t\t\tToggle hiding diagnostic messages.\n"
	    "\t-f, --fs\t\t\tBurn all the things!\n"
	    "\t-F, --flash\t\t\tUse flashing updates.\n"
	    "\t-S, --scale\t\t\tScale factor.\n"
	    "\t-t, --time\t\t\tPrint frame timings.\n"
	    "\t-W, --wfm\t\t\tSet waveform mode.\n"
	    "\t-D, --dither\t\t\tSet dithering mode.\n"
	    "\t-l, --limit\t\t\tFramerate cap (in FPS).\n"
	    "\t-c, --cap\t\t\tOverride the rendering iteration cap.\n"
	    "\t-r, --rota <-1|0|1|2|3>\t\tSwitch the framebuffer to the supplied rotation. -1 is a magic value matching the device-specific Portrait orientation.\n"
	    "\n",
	    fbink_version());
	return;
}

// Main entry point
int
    main(int argc, char* argv[])
{
	// For the LOG & ELOG macros
	g_isQuiet   = false;
	g_isVerbose = true;

	int                        opt;
	int                        opt_index;
	static const struct option opts[] = {
		{   "help",       no_argument, NULL, 'h'},
		{"verbose",       no_argument, NULL, 'v'},
		{  "quiet",       no_argument, NULL, 'q'},
		{     "fs",       no_argument, NULL, 'f'},
		{  "flash",       no_argument, NULL, 'F'},
		{  "scale", required_argument, NULL, 'S'},
		{   "time",       no_argument, NULL, 't'},
		{    "wfm", required_argument, NULL, 'W'},
		{ "dither", required_argument, NULL, 'D'},
		{  "limit", required_argument, NULL, 'l'},
		{    "cap", required_argument, NULL, 'c'},
		{   "rota", required_argument, NULL, 'r'},
		{     NULL,                 0, NULL,   0}
	};

#ifndef FBINK_FOR_LINUX
	// We need to be @ 8bpp
	uint32_t req_bpp = 8U;
#endif    // !FBINK_FOR_LINUX
	int8_t req_rota = -1;

	FBInkConfig fbink_cfg = { 0U };

	bool     is_fs          = false;
	bool     is_flashing    = false;
	bool     is_timed       = false;
	uint8_t  scaling_factor = 1U;
	uint8_t  frame_cap      = 24U;
	uint16_t iter_cap       = 250U;
	bool     is_capped      = false;

	bool errfnd = false;

	while ((opt = getopt_long(argc, argv, "hvqfFS:tW:D:l:c:r:", opts, &opt_index)) != -1) {
		switch (opt) {
			case 'v':
				g_isQuiet   = false;
				g_isVerbose = true;
				break;
			case 'q':
				g_isQuiet   = true;
				g_isVerbose = false;
				break;
			case 'h':
				show_helpmsg();
				return EXIT_SUCCESS;
				break;
			case 'f':
				is_fs = true;
				break;
			case 'F':
				is_flashing = true;
				break;
			case 'S':
				scaling_factor = (uint8_t) strtoul(optarg, NULL, 10);
				break;
			case 't':
				is_timed = true;
				break;
			case 'W':
				if (strcasecmp(optarg, "AUTO") == 0) {
					fbink_cfg.wfm_mode = WFM_AUTO;
				} else if (strcasecmp(optarg, "DU") == 0) {
					fbink_cfg.wfm_mode = WFM_DU;
				} else if (strcasecmp(optarg, "GC16") == 0) {
					fbink_cfg.wfm_mode = WFM_GC16;
				} else if (strcasecmp(optarg, "GC4") == 0) {
					fbink_cfg.wfm_mode = WFM_GC4;
				} else if (strcasecmp(optarg, "A2") == 0) {
					fbink_cfg.wfm_mode = WFM_A2;
				} else if (strcasecmp(optarg, "GL16") == 0) {
					fbink_cfg.wfm_mode = WFM_GL16;
				} else if (strcasecmp(optarg, "REAGL") == 0) {
					fbink_cfg.wfm_mode = WFM_REAGL;
				} else if (strcasecmp(optarg, "REAGLD") == 0) {
					fbink_cfg.wfm_mode = WFM_REAGLD;
				} else if (strcasecmp(optarg, "GC16_FAST") == 0) {
					fbink_cfg.wfm_mode = WFM_GC16_FAST;
				} else if (strcasecmp(optarg, "GL16_FAST") == 0) {
					fbink_cfg.wfm_mode = WFM_GL16_FAST;
				} else if (strcasecmp(optarg, "DU4") == 0) {
					fbink_cfg.wfm_mode = WFM_DU4;
				} else if (strcasecmp(optarg, "GL4") == 0) {
					fbink_cfg.wfm_mode = WFM_GL4;
				} else if (strcasecmp(optarg, "GL16_INV") == 0) {
					fbink_cfg.wfm_mode = WFM_GL16_INV;
				} else if (strcasecmp(optarg, "GCK16") == 0) {
					fbink_cfg.wfm_mode = WFM_GCK16;
				} else if (strcasecmp(optarg, "GLKW16") == 0) {
					fbink_cfg.wfm_mode = WFM_GLKW16;
				} else if (strcasecmp(optarg, "INIT") == 0) {
					fbink_cfg.wfm_mode = WFM_INIT;
				} else if (strcasecmp(optarg, "UNKNOWN") == 0) {
					fbink_cfg.wfm_mode = WFM_UNKNOWN;
				} else if (strcasecmp(optarg, "INIT2") == 0) {
					fbink_cfg.wfm_mode = WFM_INIT2;
				} else {
					ELOG("Unknown waveform update mode '%s'.", optarg);
					errfnd = true;
				}
				break;
			case 'D':
				if (strcasecmp(optarg, "PASSTHROUGH") == 0) {
					fbink_cfg.dithering_mode = HWD_PASSTHROUGH;
				} else if (strcasecmp(optarg, "FLOYD_STEINBERG") == 0) {
					fbink_cfg.dithering_mode = HWD_FLOYD_STEINBERG;
				} else if (strcasecmp(optarg, "ATKINSON") == 0) {
					fbink_cfg.dithering_mode = HWD_ATKINSON;
				} else if (strcasecmp(optarg, "ORDERED") == 0) {
					fbink_cfg.dithering_mode = HWD_ORDERED;
				} else if (strcasecmp(optarg, "QUANT_ONLY") == 0) {
					fbink_cfg.dithering_mode = HWD_QUANT_ONLY;
				} else if (strcasecmp(optarg, "LEGACY") == 0) {
					fbink_cfg.dithering_mode = HWD_LEGACY;
				} else {
					ELOG("Unknown hardware dithering algorithm '%s'.", optarg);
					errfnd = true;
				}
				break;
			case 'l':
				frame_cap = (uint8_t) strtoul(optarg, NULL, 10);
				is_capped = true;
				// Requires frame timing!
				is_timed  = true;
				break;
			case 'c':
				iter_cap = (uint16_t) strtoul(optarg, NULL, 10);
				break;
			case 'r':
				req_rota = (int8_t) strtol(optarg, NULL, 10);
				// Cheap-ass sanity check
				switch (req_rota) {
					case FB_ROTATE_UR:
					case FB_ROTATE_CW:
					case FB_ROTATE_UD:
					case FB_ROTATE_CCW:
						break;
					case -1:
						// NOTE: We'll compute it later, as we need the results from identify_device() ;).
						break;
					default:
						fprintf(stderr, "Invalid rotation '%s'!\n", optarg);
						errfnd = true;
						break;
				}
				break;
			default:
				fprintf(stderr, "?? Unknown option code 0%o ??\n", (unsigned int) opt);
				errfnd = true;
				break;
		}
	}

	if (errfnd) {
		show_helpmsg();
		return ERRCODE(EXIT_FAILURE);
	}

#ifdef FBINK_FOR_LINUX
	fprintf(stderr, "Requires an e-Ink device!\n");
	return ERRCODE(ENOSYS);
#endif

	// Assume success, until shit happens ;)
	int rv = EXIT_SUCCESS;

#ifndef FBINK_FOR_LINUX
	// NOTE: We're going to need to identify the device, to handle rotation quirks...
	identify_device();
#endif

	// NOTE: We'll need to write to the fb, so do a full open
	if ((fbFd = fbink_open()) == ERRCODE(EXIT_FAILURE)) {
		fprintf(stderr, "Failed to open the framebuffer, aborting . . .\n");
		return ERRCODE(EXIT_FAILURE);
	}

	// Print initial status, and store current vInfo
	if (!get_fbinfo()) {
		rv = ERRCODE(EXIT_FAILURE);
		goto cleanup;
	}

#ifndef FBINK_FOR_LINUX
#	ifndef FBINK_FOR_KINDLE
	// If the automagic Portrait rotation was requested, compute it
	// cppcheck-suppress knownConditionTrueFalse ; true, kept to mirror fbdepth
	if (req_rota == -1) {
		// NOTE: For *most* devices, Nickel's Portrait orientation should *always* match BootRota + 1
		//       Thankfully, the Libra appears to be ushering in a new era filled with puppies and rainbows,
		//       and, hopefully, less insane rotation quirks ;).
		if (deviceQuirks.ntxRotaQuirk != NTX_ROTA_SANE) {
			req_rota = (deviceQuirks.ntxBootRota + 1) & 3;
		} else {
			req_rota = (int8_t) deviceQuirks.ntxBootRota;
		}
		LOG("Device's expected Portrait orientation should be: %hhd (%s)!",
		    req_rota,
		    fb_rotate_to_string((uint32_t) req_rota));
	}

	// If no rotation was requested, reset req_rota to our expected sentinel value
	if (req_rota == 42) {
		req_rota = -1;
	}

	// Ensure the requested rotation is sane (if all is well, this should never be tripped)
	if (req_rota < -1 || req_rota > FB_ROTATE_CCW) {
		LOG("Requested rotation (%hhd) is bogus, discarding it!\n", req_rota);
		req_rota = -1;
	}

	// If a change was requested, do it, but check if it's necessary first
	bool is_change_needed = false;
	if (vInfo.bits_per_pixel == req_bpp) {
		// Also check that the grayscale flag is flipped properly
		if ((vInfo.bits_per_pixel == 8U && vInfo.grayscale != GRAYSCALE_8BIT) ||
		    (vInfo.bits_per_pixel > 8U && vInfo.grayscale != 0U)) {
			LOG("\nCurrent bitdepth is already %ubpp, but the grayscale flag is bogus!", req_bpp);
			// Continue, we'll need to flip the grayscale flag properly
			is_change_needed = true;
		} else {
			LOG("\nCurrent bitdepth is already %ubpp!", req_bpp);
			// No change needed as far as bitdepth is concerned...
		}
	} else {
		is_change_needed = true;
	}

	// Same for rotation, if we requested one...
	if (req_rota != -1) {
		if (vInfo.rotate == (uint32_t) req_rota) {
			LOG("\nCurrent rotation is already %hhd!", req_rota);
			// No change needed as far as rotation is concerned...
		} else {
			is_change_needed = true;
		}
	}

	// If we're here, we really want to change the bitdepth and/or rota ;)
	if (is_change_needed) {
		if (req_rota != -1) {
			LOG("\nSwitching fb to %ubpp%s @ rotation %hhd . . .",
			    req_bpp,
			    (req_bpp == vInfo.bits_per_pixel) ? " (current bitdepth)" : "",
			    req_rota);
		} else {
			LOG("\nSwitching fb to %ubpp%s . . .",
			    req_bpp,
			    (req_bpp == vInfo.bits_per_pixel) ? " (current bitdepth)" : "");
		}
		if (!set_fbinfo(req_bpp, req_rota)) {
			rv = ERRCODE(EXIT_FAILURE);
			goto cleanup;
		}
		// Recap
		if (!get_fbinfo()) {
			rv = ERRCODE(EXIT_FAILURE);
			goto cleanup;
		}
	}
#	else
	if (vInfo.bits_per_pixel != req_bpp) {
		fprintf(stderr, "Requires a K4+!\n");
		return ERRCODE(ENOSYS);
	}
#	endif    // !FBINK_FOR_KINDLE
#endif            // !FBINK_FOR_LINUX

	// Setup FBInk
	// NOTE: We pretty much need flashing updates, otherwise the ghosting heavily mangles the effect ;).
	// The downside is that it's murder to look at full-screen... :D.
	// A good middle-ground would perhaps be to only pepper a flashing update periodically?
	if (is_flashing) {
		fbink_cfg.is_flashing = true;
	}

	// Setup framecap
	// NOTE: Interesting to check how far stuff can go (especially A2)...
	//       FWIW, on my H2O, going over ~8fps starts triggering jittery behavior (i.e., intermittent stalls).
	//       On the other hand, my PW2 can take >24fps without flinching (it does feature a few fancier _FAST wfm modes).
	//       The Forma fares much better than its older brother, but still can't keep up with the PW2's versatility,
	//       (i.e., no in-between "fast" waveform modes, so AUTO falls back to something higher quality and judders).
	//       On the other hand, it does feature hardware dithering, and this yields pretty neat results w/ DU @ 24 fps...
	const long sleep_cap = BILLION / frame_cap;

	if (fbink_init(fbFd, &fbink_cfg) != EXIT_SUCCESS) {
		rv = ERRCODE(EXIT_FAILURE);
		goto cleanup;
	}
	// We also need to mmap the fb
	if (!isFbMapped) {
		if (memmap_fb(fbFd) != EXIT_SUCCESS) {
			rv = ERRCODE(EXIT_FAILURE);
			goto cleanup;
		}
	}

	// Fire!
#ifndef FBINK_FOR_LINUX
	if (is_fs) {
		setup_fire_fs();

		size_t i = 0U;
		while (true) {
			if (i > (iter_cap + iter_cap / 2U)) {
				break;
			}
			i++;

			struct timespec t0;
			if (is_timed) {
				clock_gettime(CLOCK_MONOTONIC, &t0);
			}

			do_fire_fs();

			fbink_refresh(fbFd, 0U, 0U, 0U, 0U, &fbink_cfg);

			if (is_timed) {
				struct timespec t1;
				clock_gettime(CLOCK_MONOTONIC, &t1);
				const long frame_time_ns =
				    ((t1.tv_sec * BILLION) + t1.tv_nsec) - ((t0.tv_sec * BILLION) + t0.tv_nsec);
				const float frame_time = ((float) (frame_time_ns) / MILLION);
				printf("%.1f FPS (%.3f ms)\n", THOUSAND / frame_time, frame_time);

				// Slow down?
				if (is_capped) {
					if (frame_time_ns < sleep_cap) {
						const struct timespec zzz = { 0L, sleep_cap - frame_time_ns };
						nanosleep(&zzz, NULL);
					}
				}
			}
		}
	} else if (scaling_factor > 1U) {
		// Start by clamping the scaling factor to safe values...
		scaling_factor = (uint8_t) MIN(scaling_factor, viewWidth / FIRE_HEIGHT);

		setup_fire_scaled(scaling_factor);

		size_t i = 0U;
		while (true) {
			if (i > (iter_cap + iter_cap / 2U)) {
				break;
			}
			i++;

			struct timespec t0;
			if (is_timed) {
				clock_gettime(CLOCK_MONOTONIC, &t0);
			}

			if (i > iter_cap) {
				stop_fire_scaled(scaling_factor);
			}
			do_fire_scaled(scaling_factor);

			fbink_refresh(fbFd, fire_y_origin, fire_x_origin, scaled_Width, scaled_Height, &fbink_cfg);

			if (is_timed) {
				struct timespec t1;
				clock_gettime(CLOCK_MONOTONIC, &t1);
				const long frame_time_ns =
				    ((t1.tv_sec * BILLION) + t1.tv_nsec) - ((t0.tv_sec * BILLION) + t0.tv_nsec);
				const float frame_time = ((float) (frame_time_ns) / MILLION);
				printf("%.1f FPS (%.3f ms)\n", THOUSAND / frame_time, frame_time);

				// Slow down?
				if (is_capped) {
					if (frame_time_ns < sleep_cap) {
						const struct timespec zzz = { 0L, sleep_cap - frame_time_ns };
						nanosleep(&zzz, NULL);
					}
				}
			}
		}
	} else {
		setup_fire();

		size_t i = 0U;
		while (true) {
			if (i > (iter_cap + iter_cap / 2U)) {
				break;
			}
			i++;

			struct timespec t0;
			if (is_timed) {
				clock_gettime(CLOCK_MONOTONIC, &t0);
			}

			do_fire();

			fbink_refresh(fbFd, fire_y_origin, fire_x_origin, FIRE_WIDTH, FIRE_HEIGHT, &fbink_cfg);

			if (is_timed) {
				struct timespec t1;
				clock_gettime(CLOCK_MONOTONIC, &t1);
				const long frame_time_ns =
				    ((t1.tv_sec * BILLION) + t1.tv_nsec) - ((t0.tv_sec * BILLION) + t0.tv_nsec);
				const float frame_time = ((float) (frame_time_ns) / MILLION);
				printf("%.1f FPS (%.3f ms)\n", THOUSAND / frame_time, frame_time);

				// Slow down?
				if (is_capped) {
					if (frame_time_ns < sleep_cap) {
						const struct timespec zzz = { 0L, sleep_cap - frame_time_ns };
						nanosleep(&zzz, NULL);
					}
				}
			}
		}
	}
#else
	if (vInfo.bits_per_pixel != 32U) {
		fprintf(stderr, "Requires a 32bpp fb!\n");
		return ERRCODE(ENOSYS);
	}

	setup_fire_32();

	size_t i = 0U;
	while (true) {
		if (i > (iter_cap + iter_cap / 2U)) {
			break;
		}
		i++;

		struct timespec t0;
		if (is_timed) {
			clock_gettime(CLOCK_MONOTONIC, &t0);
		}

		/*
		if (i > iter_cap) {
			stop_fire_32();
		}
		*/
		do_fire_32();

		if (is_timed) {
			struct timespec t1;
			clock_gettime(CLOCK_MONOTONIC, &t1);
			const long frame_time_ns =
			    ((t1.tv_sec * BILLION) + t1.tv_nsec) - ((t0.tv_sec * BILLION) + t0.tv_nsec);
			const float frame_time = ((float) (frame_time_ns) / MILLION);
			printf("%.1f FPS (%.3f ms)\n", THOUSAND / frame_time, frame_time);

			// Slow down?
			if (is_capped) {
				if (frame_time_ns < sleep_cap) {
					const struct timespec zzz = { 0L, sleep_cap - frame_time_ns };
					nanosleep(&zzz, NULL);
				}
			}
		}
	}
#endif    // !FBINK_FOR_LINUX

cleanup:
	if (fbink_close(fbFd) == ERRCODE(EXIT_FAILURE)) {
		fprintf(stderr, "Failed to close the framebuffer, aborting . . .\n");
		rv = ERRCODE(EXIT_FAILURE);
	}

	return rv;
}
