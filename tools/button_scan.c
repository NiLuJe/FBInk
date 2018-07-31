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
	FBInkColor         button_color = { 0xD9, 0xD9, 0xD9 };
	FBInkColor         color        = { 0U };
	unsigned short int x;
	unsigned short int y;
	FBInkCoordinates   coords              = { 0U };
	unsigned short int consecutive_matches = 0U;
	unsigned short int match_count         = 0U;
	unsigned short int matched_lines       = 0U;
	bool               gotcha              = false;
	FBInkCoordinates   match_coords        = { 0U };

	// DEBUG: Fake a Glo ;).
	/*
	viewWidth  = 758U;
	viewHeight = 1024U;
	*/

	// Start looping from the bottom half of the screen, to save some time...
	for (y = (viewHeight / 2U); y < viewHeight; y++) {
		if (match_count == 2) {
			// It looks like we found the buttons on the previous line, keep looking...
			matched_lines++;
			fprintf(stderr, "Now at %hu consecutive lines matched\n", matched_lines);
			// If we matched over 0.5% of the screen's height in consecutive lines, we got it!
			if (matched_lines >= (0.005 * viewHeight)) {
				gotcha = true;
				fprintf(stderr, "Gotcha! (After %hu consecutive lines matched)\n", matched_lines);
				break;
			}
		} else {
			// No match on the previous line, break if we were attempting to track a pair of buttons
			if (matched_lines > 0) {
				fprintf(
				    stderr, "Booh :(. Failed to match after %hu consecutive lines\n", matched_lines);
				break;
			} else {
				// Reset the counter otherwise.
				matched_lines = 0U;
			}
		}
		// New line, reset counters
		consecutive_matches = 0U;
		match_count         = 0U;

		for (x = 0U; x < viewWidth; x++) {
			coords.x = x;
			coords.y = y;

			// Handle 16bpp rota (hopefully applies in Nickel, too ;D)
			(*fxpRotateCoords)(&coords);
			(*fxpGetPixel)(&coords, &color);

			if (color.r == button_color.r && color.g == button_color.g && color.b == button_color.b) {
				consecutive_matches++;
			} else {
				// One button is roughly 17% of the screen's width on my H2O (18.5% in Large Print mode)
				// The larger window should hopefully cover the various range of resolutions & DPI...
				if (consecutive_matches >= (0.125 * viewWidth) &&
				    consecutive_matches <= (0.25 * viewWidth)) {
					match_count++;
					fprintf(stderr,
						"End of match %hu after %hu consecutive matche @ (%hu, %hu)\n",
						match_count,
						consecutive_matches,
						x,
						y);
					// NOTE: We store un-rotated coords. That may not be what we ultimately need on those 16bpp FW?
					if (match_count == 2) {
						// We only care about the second button, Connect :).
						// Try to hit roughly the middle of the button (which takes roughly 4.8% of the screen's height, LP & !LP)
						match_coords.y = y + (0.02 * viewHeight);
						// Try to hit roughly the middle of the button
						match_coords.x = x - (0.08 * viewWidth);
					}
				} else {
					if (consecutive_matches > 0U) {
						fprintf(
						    stderr,
						    "Failed end of match after %hu consecutive matches @ (%hu, %hu)\n",
						    consecutive_matches,
						    x,
						    y);
					}
				}
				consecutive_matches = 0U;
			}
		}
	}

	if (gotcha) {
		fprintf(stderr, "Match! :) (over %hu lines)\n", matched_lines);
		fprintf(stdout, "x=%hu, y=%hu\n", match_coords.x, match_coords.y);
		// NOTE: Rotate coords, because the Touch origin may not match the fb origin... >_<"
		// FIXME: May output garbage if fb is already rotated -_-"
		//        Set match_coords to coords.x/y and only do this rota if !fbink_is_fb_quirky?
		// NOTE: H2O²r1 appears to be a special snowflake, too, so, that one might be fun... (i.e., ry = x there?)
		unsigned short int rx = match_coords.y;
		unsigned short int ry = (unsigned short int) (viewWidth - match_coords.x - 1);
		fprintf(stdout, "Rotated: x=%hu, y=%hu\n", rx, ry);
	} else {
		fprintf(stderr, "No match :(\n");
	}

	// Cleanup
	if (g_fbink_isFbMapped) {
		munmap(g_fbink_fbp, g_fbink_screensize);
	}
	close(fbfd);

	return EXIT_SUCCESS;
}
