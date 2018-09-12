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
// Mountpoint monitoring helpers pilfered from KFMon ;).
// Check that onboard is mounted (in the right place)...
static bool
    is_onboard_mounted(void)
{
	// cf. http://program-nix.blogspot.fr/2008/08/c-language-check-filesystem-is-mounted.html
	FILE*          mtab       = NULL;
	struct mntent* part       = NULL;
	bool           is_mounted = false;

	if ((mtab = setmntent("/proc/mounts", "r")) != NULL) {
		while ((part = getmntent(mtab)) != NULL) {
			LOG("Checking fs %s mounted on %s", part->mnt_fsname, part->mnt_dir);
			if ((part->mnt_dir != NULL) && (strcmp(part->mnt_dir, "/mnt/onboard")) == 0) {
				is_mounted = true;
				break;
			}
		}
		endmntent(mtab);
	}

	return is_mounted;
}

// Monitor mountpoint activity...
static bool
    wait_for_onboard(void)
{
	// cf. https://stackoverflow.com/questions/5070801
	int           mfd = open("/proc/mounts", O_RDONLY, 0);
	struct pollfd pfd;

	uint8_t changes = 0;
	pfd.fd          = mfd;
	pfd.events      = POLLERR | POLLPRI;
	pfd.revents     = 0;
	while (poll(&pfd, 1, -1) >= 0) {
		if (pfd.revents & POLLERR) {
			LOG("Mountpoints changed (iteration nr. %hhu)", changes++);

			// Stop polling once we know onboard is available...
			if (is_onboard_mounted()) {
				LOG("Yay, onboard is available!");
				break;
			}
		}
		pfd.revents = 0;

		// If we can't find our mountpoint after that many changes, assume we're screwed...
		if (changes >= 5) {
			LOG("Too many mountpoint changes without finding onboard, aborting!");
			close(mfd);
			return false;
		}
	}

	close(mfd);
	// Onboard is finally available ;).
	return true;
}

static bool
    wait_for_background_color(uint8_t v, unsigned short int timeout)
{
	// Check a single pixel that should always be white in Nickel,
	// and always black when on the "Connected" & "Importing Content" screens.
	// Something on the bottom margin should do the trick,
	// without falling into any "might be behind the bezel" quirk,
	// which would cause it to potentially already be black (or transparent) in Nickel...
	FBInkCoordinates coords = { (unsigned short int) (viewWidth / 2U), (unsigned short int) (viewHeight - 1) };
	(*fxpRotateCoords)(&coords);
	FBInkColor color = { 0U };

	// We loop for timeout seconds at most, waking up every 250ms...
	unsigned short int iterations = (unsigned short int) (timeout * (1 / 0.25f));
	for (uint8_t i = 0U; i < iterations; i++) {
		// Wait 250ms . . .
		nanosleep((const struct timespec[]){ { 0, 250000000L } }, NULL);

		(*fxpGetPixel)(&coords, &color);
		LOG("On iteration nr. %hhu of %hu, pixel (%hu, %hu) was #%02hhX%02hhX%02hhX",
		    i,
		    iterations,
		    coords.x,
		    coords.y,
		    color.r,
		    color.g,
		    color.b);

		// Got it!
		if (color.r == v && color.g == v && color.b == v) {
			return true;
		}
	}

	// If we got this far, we failed :(
	return false;
}

static bool
    is_on_connected_screen(void)
{
	// USB Connected screen has a black background
	LOG("Waiting for the 'USB Connected' screen . . .");
	return wait_for_background_color(eInkBGCMap[BG_BLACK], 5U);
}

static bool
    is_on_home_screen(void)
{
	// Home screen has a white background
	LOG("Waiting for the 'Home' screen . . .");
	return wait_for_background_color(eInkBGCMap[BG_WHITE], 5U);
}

static bool
    is_on_import_screen(void)
{
	// Import screen has a black background
	LOG("Waiting for the 'Content Import' screen . . .");
	return wait_for_background_color(eInkBGCMap[BG_BLACK], 5U);
}

static bool
    is_on_home_screen_again(void)
{
	// Home screen has a white background
	LOG("Waiting for the 'Home' screen . . .");
	// NOTE: Give up after 5 minutes?
	return wait_for_background_color(eInkBGCMap[BG_WHITE], (60U * 5U));
}

static int
    generate_button_press(FBInkCoordinates* match_coords, bool nosleep)
{
	LOG("Pressing the Connect button . . .");
	struct input_event ev;
	int                ifd = -1;
	ifd                    = open("/dev/input/event1", O_WRONLY | O_NONBLOCK | O_CLOEXEC);
	if (ifd == -1) {
		fprintf(stderr, "[FBInk] Failed to open input device!\n");
		return ERRCODE(ENODEV);
	}

	// NOTE: Kobo devices use a wide range of weird & quirky variations on the touch input protocol,
	//       depending on the exact device, so do our best to handle that properly...
	// NOTE: Double-check on your device w/ hexdump -x /dev/input/event1 (or -d if you prefer decimal).
	if (deviceQuirks.isKoboNonMT) {
		// NOTE: Should match what Kobo does on devices who don't hanve a Multi-Touch aware driver...
		//       Should cover the Touch A/B/C, Mini, Glo, Aura HD
		// NOTE: The original Touch is known to come in multiple variants,
		//       some of which might handle things slightly differently.
		//       Trying to guess which is which seems to be a lost cause, so don't try too hard...
		SEND_INPUT_EVENT(EV_ABS, ABS_Y, match_coords->y);
		SEND_INPUT_EVENT(EV_ABS, ABS_X, match_coords->x);
		SEND_INPUT_EVENT(EV_ABS, ABS_PRESSURE, 100);
		SEND_INPUT_EVENT(EV_KEY, BTN_TOUCH, 1);
		SEND_INPUT_EVENT(EV_SYN, SYN_REPORT, 0);

		SEND_INPUT_EVENT(EV_ABS, ABS_Y, match_coords->y);
		SEND_INPUT_EVENT(EV_ABS, ABS_X, match_coords->x);
		SEND_INPUT_EVENT(EV_ABS, ABS_PRESSURE, 0);
		SEND_INPUT_EVENT(EV_KEY, BTN_TOUCH, 0);
		SEND_INPUT_EVENT(EV_SYN, SYN_REPORT, 0);
	} else if (deviceQuirks.isKoboMk7) {
		// NOTE: Roughly corresponds to what we call the "Snow" protocol in KOReader.
		//       Should handle the H2O²r2, (Aura SEr2?), Clara HD
		//       And possibly the H2O²r1, although this one will need a dedicated quirk to compute the proper x/y coords.
		SEND_INPUT_EVENT(EV_KEY, BTN_TOOL_FINGER, 1);
		SEND_INPUT_EVENT(EV_KEY, BTN_TOUCH, 1);
		SEND_INPUT_EVENT(EV_ABS, ABS_MT_TRACKING_ID, 0);
		SEND_INPUT_EVENT(EV_ABS, ABS_MT_DISTANCE, 0);
		SEND_INPUT_EVENT(EV_ABS, ABS_MT_POSITION_X, match_coords->x);
		SEND_INPUT_EVENT(EV_ABS, ABS_MT_POSITION_Y, match_coords->y);
		SEND_INPUT_EVENT(EV_ABS, ABS_MT_PRESSURE, 20);
		SEND_INPUT_EVENT(EV_ABS, ABS_MT_TOUCH_MAJOR, 0);
		SEND_INPUT_EVENT(EV_ABS, ABS_MT_TOUCH_MINOR, 0);
		SEND_INPUT_EVENT(EV_ABS, ABS_MT_ORIENTATION, 0);
		SEND_INPUT_EVENT(EV_SYN, SYN_MT_REPORT, 0);
		SEND_INPUT_EVENT(EV_SYN, SYN_REPORT, 0);

		SEND_INPUT_EVENT(EV_ABS, ABS_MT_TRACKING_ID, 0);
		SEND_INPUT_EVENT(EV_ABS, ABS_MT_DISTANCE, 0);
		SEND_INPUT_EVENT(EV_ABS, ABS_MT_POSITION_X, match_coords->x);
		SEND_INPUT_EVENT(EV_ABS, ABS_MT_POSITION_Y, match_coords->y);
		SEND_INPUT_EVENT(EV_ABS, ABS_MT_PRESSURE, 0);
		SEND_INPUT_EVENT(EV_ABS, ABS_MT_TOUCH_MAJOR, 0);
		SEND_INPUT_EVENT(EV_ABS, ABS_MT_TOUCH_MINOR, 0);
		SEND_INPUT_EVENT(EV_ABS, ABS_MT_ORIENTATION, 0);
		SEND_INPUT_EVENT(EV_SYN, SYN_MT_REPORT, 0);
		SEND_INPUT_EVENT(EV_SYN, SYN_REPORT, 0);
		SEND_INPUT_EVENT(EV_KEY, BTN_TOUCH, 0);
		SEND_INPUT_EVENT(EV_KEY, BTN_TOOL_FINGER, 0);
		SEND_INPUT_EVENT(EV_SYN, SYN_REPORT, 0);
	} else {
		// NOTE: Corresponds to what we call the "Phoenix" protocol in KOReader
		//       (with or without the Alyssum tweaks, which appear irrelevant),
		//       (and with or without the ev_epoch_time tweaks, which shouldn't matter for us).
		//       Which means we should cover the: KA1, H2O, Aura, Aura SEr1, (Aura SEr2?), Glo HD, Touch 2.0
		SEND_INPUT_EVENT(EV_ABS, ABS_MT_TRACKING_ID, 1);
		SEND_INPUT_EVENT(EV_ABS, ABS_MT_TOUCH_MAJOR, 1);
		SEND_INPUT_EVENT(EV_ABS, ABS_MT_WIDTH_MAJOR, 1);
		SEND_INPUT_EVENT(EV_ABS, ABS_MT_POSITION_X, match_coords->x);
		SEND_INPUT_EVENT(EV_ABS, ABS_MT_POSITION_Y, match_coords->y);
		// At this point, the Glo HD adds another pair of events:
		// NOTE: Not adding them appears to work on Mk6, and adding them also works on Mk5,
		//       so, let's add them unconditionally until something blows up ;).
		SEND_INPUT_EVENT(EV_ABS, ABS_PRESSURE, 1024);
		SEND_INPUT_EVENT(EV_KEY, BTN_TOUCH, 1);

		SEND_INPUT_EVENT(EV_SYN, SYN_MT_REPORT, 0);
		SEND_INPUT_EVENT(EV_SYN, SYN_REPORT, 0);

		SEND_INPUT_EVENT(EV_ABS, ABS_MT_TRACKING_ID, 1);
		SEND_INPUT_EVENT(EV_ABS, ABS_MT_TOUCH_MAJOR, 0);
		SEND_INPUT_EVENT(EV_ABS, ABS_MT_WIDTH_MAJOR, 0);
		SEND_INPUT_EVENT(EV_ABS, ABS_MT_POSITION_X, match_coords->x);
		SEND_INPUT_EVENT(EV_ABS, ABS_MT_POSITION_Y, match_coords->y);
		// Don't forget the extra Mk6 events...
		SEND_INPUT_EVENT(EV_ABS, ABS_PRESSURE, 0);
		SEND_INPUT_EVENT(EV_KEY, BTN_TOUCH, 0);

		SEND_INPUT_EVENT(EV_SYN, SYN_MT_REPORT, 0);
		SEND_INPUT_EVENT(EV_SYN, SYN_REPORT, 0);
	}
	// NOTE: FWIW, on a PW2 (where we don't have to jump through crazy hoops to calculate coordinates, ahem...):
	//       BTN_TOUCH:1;ABS_MT_TRACKING_ID:0;ABS_MT_POSITION_X:x;ABS_MT_POSITION_Y:y;SYN_REPORT;BTN_TOOL_FINGER:1;SYN_REPORT
	//       ABS_MT_POSITION_X:x;ABS_MT_POSITION_Y:y;SYN_REPORT;ABS_MT_TRACKING_ID:-1;SYN_REPORT;BTN_TOUCH:0;BTN_TOOL_FINGER:0;SYN_REPORT

	close(ifd);

	// Assume success until shit happens :)
	int rv = EXIT_SUCCESS;

	// NOTE: Checking if the input was successful is optional, since it can sleep for up to 5s,
	//       which may not be desirable, especially when you have other ways of checking the success of the operation,
	//       f.g., checking the state of the /mnt/onboard mountpoint...
	if (!nosleep) {
		// If we fail to detect the "Connected" screen, we've failed to press the button...
		if (!is_on_connected_screen()) {
			rv = ERRCODE(ENOTSUP);
		}
	}

	return rv;
}
#endif

// Scan the screen's content for Kobo's "Connect" button in the "USB plugged in" popup.
int
    fbink_button_scan(int fbfd UNUSED_BY_NOBUTTON,
		      bool press_button UNUSED_BY_NOBUTTON,
		      bool nosleep UNUSED_BY_NOBUTTON,
		      bool detect_import UNUSED_BY_NOBUTTON)
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
	// NOTE: RGB565 conversions are complex and potentially slightly lossy,
	//       with slight rounding/truncation errors that can be different depending on how exactly the conversions were done.
	//       This matches with what *we* do, hopefully that'll be close enough to what Nickel actually does...
	if (deviceQuirks.isKobo16Landscape) {
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
	// NOTE: Truncating floats via explicit casts is good enough for us, no need to pull in libm ;).
	unsigned short int min_target_pixels = (unsigned short int) (0.125f * (float) viewWidth);
	unsigned short int max_target_pixels = (unsigned short int) (0.25f * (float) viewWidth);

	// Recap the various settings as computed for this screen...
	LOG("Button color is expected to be #%02hhX%02hhX%02hhX", button_color.r, button_color.g, button_color.b);
	LOG("We need to match two buttons each between %hu and %hu pixels wide!", min_target_pixels, max_target_pixels);

	// Only look in the area of the screen where we're likely to find the buttons, both to save some time,
	// and to lower the risk of false positives, as unlikely as that might be.
	unsigned short int min_height = (unsigned short int) (0.55f * (float) viewHeight);
	unsigned short int max_height = (unsigned short int) (0.85f * (float) viewHeight);
	unsigned short int min_width  = (unsigned short int) (0.05f * (float) viewWidth);
	unsigned short int max_width  = (unsigned short int) (0.80f * (float) viewWidth);

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
						match_coords.x = (unsigned short int) (x - 1);
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
		gotcha = false;
		// We're just going too scan down that final column of the button until we hit the end of it :).
		for (unsigned short int j = match_coords.y; j < max_height; j++) {
			// NOTE: Keep those two together to avoid desyncing them if we have a rotation quirk...
			coords.x = match_coords.x;
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
			if ((rv = generate_button_press(&match_coords, nosleep)) != EXIT_SUCCESS) {
				fprintf(stderr, "[FBInk] Failed to press the Connect button!\n");
				goto cleanup;
			} else {
				LOG(". . . appears to have been a success!");

				// Do we want to do the extra mile and wait for the end of a content import?
				if (!nosleep && detect_import) {
					// Right now, we're on the "USB Connected" screen, onboard *should* be unmounted...
					// NOTE: Except that USB is terrible, so it takes quite a bit of time for things to settle down...
					//       So, wait 10s and hope shit will have settled down by then...
					LOG("Waiting 10s for USB to settle down . . .");
					// NOTE: time_t is int64_t on Linux, so, long too ;).
					nanosleep((const struct timespec[]){ { 10L, 0L } }, NULL);
					if (is_onboard_mounted()) {
						LOG("Err, we're supposed to be in USBMS mode, but onboard appears to still be mounted ?!");
						// That won't do... abort!
						fprintf(
						    stderr,
						    "[FBInk] Unexpected onboard mount status, can't detect content import!\n");
						rv = ERRCODE(EXIT_FAILURE);
						goto cleanup;
					}

					// Right, now that we've made sure of that, wait for onboard to come back up :)
					if (!wait_for_onboard()) {
						// That won't do... abort!
						fprintf(
						    stderr,
						    "[FBInk] Failed to detect end of USBMS session, can't detect content import!\n");
						rv = ERRCODE(EXIT_FAILURE);
						goto cleanup;
					}

					// Now check that we're back on the Home screen...
					if (!is_on_home_screen()) {
						// That won't do... abort!
						fprintf(
						    stderr,
						    "[FBInk] Failed to detect Home screen, can't detect content import!\n");
						rv = ERRCODE(EXIT_FAILURE);
						goto cleanup;
					}

					// Then wait a while to see if the Import screen pops up...
					if (!is_on_import_screen()) {
						// Maybe there was nothing to import?
						fprintf(
						    stderr,
						    "[FBInk] Couldn't detect the Import screen, maybe there was nothing to import?\n");
						rv = ERRCODE(ENODATA);
						goto cleanup;
					}

					// The wait a potentially long while (~5min) for the end of the Import process...
					if (!is_on_home_screen_again()) {
						// That won't do... abort!
						fprintf(
						    stderr,
						    "[FBInk] Failed to detect the end of the Import process, maybe it's hung or running suspiciously long (> 5min)?\n");
						rv = ERRCODE(ETIME);
						goto cleanup;
					}
				}
			}
		}
	} else {
		LOG("No match :(");
		fprintf(stderr, "[FBInk] Failed to find a Connect button on screen!\n");
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
