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

#include "fbink_button_scan.h"

#ifdef FBINK_WITH_BUTTON_SCAN
static int
    generate_button_press(int fbfd, FBInkCoordinates* match_coords)
{
	LOG("Pressing the Connect button . . .");
	struct input_event ev;
	int                ifd = -1;
	ifd                    = open("/dev/input/event1", O_WRONLY | O_NONBLOCK);
	if (ifd == -1) {
		fprintf(stderr, "[FBInk] Failed to open input device!\n");
		return ERRCODE(ENODEV);
	}

	// NOTE: May not be completely right for every model... (OK on H2O)
	//       Double-check on your device w/ hexdump -x /dev/input/event1 (or -d if you prefer decimal).
	SEND_INPUT_EVENT(EV_ABS, ABS_MT_TRACKING_ID, 1);
	SEND_INPUT_EVENT(EV_ABS, ABS_MT_TOUCH_MAJOR, 1);
	SEND_INPUT_EVENT(EV_ABS, ABS_MT_WIDTH_MAJOR, 1);
	SEND_INPUT_EVENT(EV_ABS, ABS_MT_POSITION_X, match_coords->x);
	SEND_INPUT_EVENT(EV_ABS, ABS_MT_POSITION_Y, match_coords->y);
	SEND_INPUT_EVENT(EV_SYN, SYN_MT_REPORT, 0);
	SEND_INPUT_EVENT(EV_SYN, SYN_REPORT, 0);

	SEND_INPUT_EVENT(EV_ABS, ABS_MT_TRACKING_ID, 1);
	SEND_INPUT_EVENT(EV_ABS, ABS_MT_TOUCH_MAJOR, 0);
	SEND_INPUT_EVENT(EV_ABS, ABS_MT_WIDTH_MAJOR, 0);
	SEND_INPUT_EVENT(EV_ABS, ABS_MT_POSITION_X, match_coords->x);
	SEND_INPUT_EVENT(EV_ABS, ABS_MT_POSITION_Y, match_coords->y);
	SEND_INPUT_EVENT(EV_SYN, SYN_MT_REPORT, 0);
	SEND_INPUT_EVENT(EV_SYN, SYN_REPORT, 0);

	close(ifd);

	// Assume success until shit happens :)
	int rv = EXIT_SUCCESS;

	// Check if screen content changed 100ms later, as a poor man's way of checking if the tap was successful...
	nanosleep((const struct timespec[]){ { 0, 100000000L } }, NULL);

	// Neuter logging for this pass...
	bool orig_verbose = g_isVerbose;
	bool orig_quiet   = g_isQuiet;
	g_isVerbose       = false;
	g_isQuiet         = true;

	// If we still *find* a button, we've failed to press it ;).
	if (fbink_button_scan(fbfd, false, true) == EXIT_SUCCESS) {
		rv = ERRCODE(ENOTSUP);
	}

	// Restore logging
	g_isVerbose = orig_verbose;
	g_isQuiet   = orig_quiet;

	return rv;
}
#endif

// Scan the screen's content for Kobo's "Connect" button in the "USB plugged in" popup.
int
    fbink_button_scan(int fbfd UNUSED_BY_NOBUTTON, bool press_button UNUSED_BY_NOBUTTON, bool silent UNUSED_BY_NOBUTTON)
{
#ifdef FBINK_WITH_BUTTON_SCAN
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

	// Wheee! (Default to the proper value on 32bpp FW)
	FBInkColor button_color = { 0xD9, 0xD9, 0xD9 };

	// And handle yet another bit of 16bpp weirdness...
	// NOTE: There *may* be a rounding/conversion error somewhere...
	//       I can vouch for get_pixel_RGB565's accuracy,
	//       and set_pixel_RGB565 looks straightforward enough, so, err, I blame Kobo? :D.
	if (fbink_is_fb_quirky()) {
		button_color.r = 0xDE;
		button_color.g = 0xDB;
		button_color.b = 0xDE;
	}

	FBInkColor         color         = { 0U };
	FBInkCoordinates   coords        = { 0U };
	unsigned short int button_width  = 0U;
	unsigned short int match_count   = 0U;
	FBInkCoordinates   match_coords  = { 0U };
	bool               gotcha        = false;
	unsigned short int button_height = 0U;

	// Centralize the various thresholds we use...
	// NOTE: Depending on the device's DPI & resolution, a button takes between 17% and 20% of the screen's width.
	//       Possibly less on larger resolutions, and more on smaller resolutions, so try to handle everyone in one fell swoop.
	// NOTE: Yes, GCC, we're truncating floats, I know. We don't care ;).
#	pragma GCC diagnostic push
#	pragma GCC diagnostic ignored "-Wconversion"
#	pragma GCC diagnostic ignored "-Wfloat-conversion"
	unsigned short int     min_target_pixels = (0.125f * viewWidth);
	unsigned short int     max_target_pixels = (0.25f * viewWidth);
#	pragma GCC diagnostic pop

	// Recap the various settings as computed for this screen...
	LOG("Button color is expected to be #%hhx%hhx%hhx", button_color.r, button_color.g, button_color.b);
	LOG("We need to match two buttons each between %hu and %hu pixels wide!", min_target_pixels, max_target_pixels);

	// Only look in the area of the screen where we're likely to find the buttons, both to save some time,
	// and to lower the risk of false positives, as unlikely as that might be.
#	pragma GCC diagnostic push
#	pragma GCC diagnostic ignored "-Wconversion"
#	pragma GCC diagnostic ignored "-Wfloat-conversion"
	unsigned short int     min_height = (0.55f * viewHeight);
	unsigned short int     max_height = (0.85f * viewHeight);
	unsigned short int     min_width  = (0.05f * viewWidth);
	unsigned short int     max_width  = (0.80f * viewWidth);
#	pragma GCC diagnostic pop
	LOG("Looking for buttons in a %hux%hu rectangle, from (%hu, %hu) to (%hu, %hu)",
	    (unsigned short int) (max_width - min_width),
	    (unsigned short int) (max_height - min_height),
	    min_width,
	    min_height,
	    max_width,
	    max_height);

	for (unsigned short int y = min_height; y < max_height; y++) {
		if (match_count == 2) {
			// It looks like we found the top of the buttons on the previous line, we can stop looping.
			gotcha = true;
			break;
		}

		// New line, reset counters
		button_width = 0U;
		match_count  = 0U;

		for (unsigned short int x = min_width; x < max_width; x++) {
			coords.x = x;
			coords.y = y;

			// Handle 16bpp rotation (hopefully applies in Nickel, too ;D)
			(*fxpRotateCoords)(&coords);
			(*fxpGetPixel)(&coords, &color);

			if (color.r == button_color.r && color.g == button_color.g && color.b == button_color.b) {
				// Found a pixel of the right color for a button...
				button_width++;
			} else {
				// Pixel is no longer part of a button...
				if (button_width >= min_target_pixels && button_width <= max_target_pixels) {
					// But we've just finished matching enough pixels in a row to assume we found a button!
					match_count++;
					LOG("End of match %hu after %hu consecutive matches @ (%hu, %hu)",
					    match_count,
					    button_width,
					    x,
					    y);
					// We only care about the second button, Connect :).
					if (match_count == 2) {
						match_coords.y = y;
						// Last good pixel was the previous one, store that one ;).
						match_coords.x = (short unsigned int) (x - 1);
						// We've got the top-right corner of the Connect button, stop looping.
						break;
					}
				} else {
					if (button_width > 0U) {
						// And we only matched a few stray pixels of the right color before, not a button.
						LOG("Failed end of match after %hu consecutive matches @ (%hu, %hu)",
						    button_width,
						    x,
						    y);
					}
				}
				// In any case, wrong color, reset the counter.
				button_width = 0U;
			}
		}
	}

	// If we've got a button corner in the previous pass, we're not quite done yet...
	if (gotcha) {
		gotcha   = false;
		coords.x = match_coords.x;
		// We're just going too scan down that final column of the button until we hit the end of it :).
		for (unsigned short int j = match_coords.y; j < max_height; j++) {
			coords.y = j;

			(*fxpRotateCoords)(&coords);
			(*fxpGetPixel)(&coords, &color);

			if (color.r == button_color.r && color.g == button_color.g && color.b == button_color.b) {
				// Found a pixel of the right color for a button...
				button_height++;
			} else {
				// Pixel is no longer part of a button,
				// which likely means we've now hit the bottom-right of the Connect button.
				// NOTE: No more guesses, assume we *really* got the corner of the button earlier.
				// Backtrack from half the height & half the width to get the center of the button.
				match_coords.y = (unsigned short int) (j - (button_height / 2U));
				match_coords.x = (unsigned short int) (match_coords.x - (button_width / 2U));
				// And we're done!
				gotcha = true;
				break;
			}
		}
	}

	if (gotcha) {
		LOG("Matched on a %hux%hu button! :)", button_width, button_height);

		// The touch panel has a fixed origin that differs from the framebuffer's... >_<".
		rotate_coordinates(&match_coords);
		ELOG("x=%hu, y=%hu", match_coords.x, match_coords.y);

		// NOTE: The H2O²r1 is a special snowflake, input is rotated 90° in the *other* direction
		//       (i.e., origin at the bottom-left instead of top-right).
		//       Hopefully that doesn't apply to the fb itself, too...
		ELOG("H2O²r1: x=%hu, y=%hu",
		     (unsigned short int) (viewHeight - match_coords.x - 1),
		     (unsigned short int) (viewWidth - match_coords.y - 1));

		// Press it if requested...
		if (press_button) {
			if ((rv = generate_button_press(fbfd, &match_coords)) != EXIT_SUCCESS) {
				fprintf(stderr, "[FBInk] Failed to press the Connect button!\n");
				goto cleanup;
			} else {
				LOG(". . . appeared to have been a success!");
			}
		}
	} else {
		LOG("No match :(");
		// We don't want to see that message during the second checking pass done by generate_button_press...
		if (!silent) {
			fprintf(stderr, "[FBInk] Failed to find a Connect button on screen!\n");
		}
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
#else
	fprintf(stderr, "[FBInk] Kobo Connect button scanning is disabled in this FBInk build!\n");
	return ERRCODE(ENOSYS);
#endif    // FBINK_WITH_BUTTON_SCAN
}
