/*
	FBInk: FrameBuffer eInker, a tool to print text & images on eInk devices (Kobo/Kindle)
	Copyright (C) 2018 NiLuJe <ninuje@gmail.com>

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

// Build w/ ${CROSS_TC}-gcc -O3 -ffast-math -ftree-vectorize -funroll-loops -march=armv7-a -mtune=cortex-a8 -mfpu=neon -mfloat-abi=hard -mthumb -D_GLIBCXX_USE_CXX11_ABI=0 -pipe -fomit-frame-pointer -frename-registers -fweb -flto=9 -fuse-linker-plugin tools/button_scan.c -LRelease/ -l:libfbink.a -o button_scan

#include "../fbink.h"
// NOTE: Don't do this at home. This is a quick and rough POC to have some fun w/ https://www.mobileread.com/forums/showpost.php?p=3731967&postcount=12
//       No-one should ever, ever, ever include internal headers, I'm just re-using bits of private API to isolate this POC.
#include "../fbink_internal.h"

// FBInk always returns negative values on failure
#define ERRCODE(e) (-(e))

// Help message
static void
    show_helpmsg(void)
{
	printf(
	    "\n"
	    "FBInk Scan (w/ FBInk %s)\n"
	    "\n"
	    "Usage: fbink\n"
	    "\n"
	    "Spits out x, y coordinates for the USB Connect button.\n"
	    "\n",
	    fbink_version());

	return;
}

// Memory map the framebuffer (from fbink.c, part of the private API, but implementation is in the C file, so, copy it).
static int
    memmap_fb(int fbfd)
{
	g_fbink_screensize = finfo.smem_len;
	g_fbink_fbp = (unsigned char*) mmap(NULL, g_fbink_screensize, PROT_READ | PROT_WRITE, MAP_SHARED, fbfd, 0);
	if (g_fbink_fbp == MAP_FAILED) {
		char  buf[256];
		char* errstr = strerror_r(errno, buf, sizeof(buf));
		fprintf(stderr, "[FBInk] mmap: %s\n", errstr);
		return ERRCODE(EXIT_FAILURE);
	} else {
		g_fbink_isFbMapped = true;
	}

	return EXIT_SUCCESS;
}

// Application entry point
int
    main(int argc, char* argv[])
{
	FBInkConfig fbink_config = { 0 };

	// Open framebuffer and keep it around, then setup globals.
	int fbfd = -1;
	if (ERRCODE(EXIT_FAILURE) == (fbfd = fbink_open())) {
		fprintf(stderr, "Failed to open the framebuffer, aborting . . .\n");
		return ERRCODE(EXIT_FAILURE);
	}
	if (fbink_init(fbfd, &fbink_config) == ERRCODE(EXIT_FAILURE)) {
		fprintf(stderr, "Failed to initialize FBInk, aborting . . .\n");
		return ERRCODE(EXIT_FAILURE);
	}

	// mmap the fb if need be...
	if (!g_fbink_isFbMapped) {
		if (memmap_fb(fbfd) != EXIT_SUCCESS) {
			return ERRCODE(EXIT_FAILURE);
		}
	}

	// Wheee!
	FBInkColor button_color  = { 0xD9, 0xD9, 0xD9 };
	FBInkColor color = { 0U };
	unsigned short int x;
	unsigned short int y;
	FBInkCoordinates   coords = { 0U };
	unsigned short int consecutive_matches = 0U;
	unsigned short int match_count = 0U;
	FBInkCoordinates   match_coords = { 0U };

	// Start looping from the bottom half of the screen, to save some time...
	for (y = (viewHeight / 2U); y < viewHeight; y++) {
		for (x = 0U; x < viewWidth; x++) {
			coords.x = x;
			coords.y = y;

			// Handle 16bpp rota (hopefully applies in Nickel, too ;D)
			(*fxpRotateCoords)(&coords);
			(*fxpGetPixel)(&coords, &color);

			if (color.r == button_color.r && color.g == button_color.g && color.b == button_color.b) {
				consecutive_matches++;
				fprintf(stderr, "match %hu @ (%hu, %hu)\n", consecutive_matches, x, y);
			} else {
				// One button is roughly 17% of the screen's width on my H2O (18.5% in Large Print mode)
				// NOTE: May need to be even stricter to avoid skipping the gap between the two buttons on some devices?
				if (consecutive_matches >= (0.165 * viewWidth) && consecutive_matches <= (0.19 * viewWidth)) {
					match_count++;
					fprintf(stderr, "End match %hu @ (%hu, %hu)\n", match_count, x, y);
					match_coords.y = y - ((0.048 * viewHeight) / 2U);	// Try to hit roughly the middle of the button (which takes roughly 4.8% of the screen's height, LP & !LP)
					match_coords.x = x - ((0.17 * viewWidth) / 2U);		// Try to hit roughly the middle of the button
				}
				consecutive_matches=0U;
			}
		}
	}

	// Half of the empty space over the tallest letter of one button (in Large Print) is roughly 0.625% of the screen's height on my H2O (0.76% otherwise).
	// (2 buttons times 2 sides (top + bottom around the text) == 8)
	// LP - 5%
	unsigned short int min_target_count = 8U * (0.00625 * viewHeight) * 0.95;
	// !LP + 10%
	unsigned short int max_target_count = 8U * (0.0076 * viewHeight) * 1.10;
	if (match_count >= min_target_count && match_count <= max_target_count) {
		fprintf(stderr, "Match! :) (count: %hu >= %hu && <= %hu)\n", match_count, min_target_count, max_target_count);
		fprintf(stdout, "x=%hu, y=%hu\n", match_coords.x, match_coords.y);
	} else {
		fprintf(stderr, "No match :( (count: %hu < %hu || > %hu)\n", match_count, min_target_count, max_target_count);
	}

	// Cleanup
	if (g_fbink_isFbMapped) {
		munmap(g_fbink_fbp, g_fbink_screensize);
	}
	close(fbfd);

	return EXIT_SUCCESS;
}
