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

#include "fbink_button_scan.h"

#ifdef FBINK_WITH_BUTTON_SCAN
// Mountpoint monitoring helpers pilfered from KFMon ;).
// Check if onboard (the mountpoint, not the fs) is mounted or not...
static bool
    is_onboard_state(bool mounted)
{
	// c.f., http://program-nix.blogspot.com/2008/08/c-language-check-filesystem-is-mounted.html
	FILE* restrict mtab          = NULL;
	struct mntent* restrict part = NULL;
	bool is_mounted              = false;

	if ((mtab = setmntent("/proc/mounts", "r")) != NULL) {
		while ((part = getmntent(mtab)) != NULL) {
			if ((part->mnt_dir != NULL) && (strcmp(part->mnt_dir, "/mnt/onboard")) == 0) {
				LOG("fs %s is mounted on %s", part->mnt_fsname, part->mnt_dir);
				is_mounted = true;
				break;
			}
		}
		endmntent(mtab);
	}

	if (!is_mounted) {
		LOG("/mnt/onboard is unmounted");
	}

	// Return the right thing depending on which state we want onboard to be...
	if (mounted) {
		return is_mounted;
	} else {
		return !is_mounted;
	}
}

// Monitor mountpoint activity...
static bool
    wait_for_onboard_state(bool mounted)
{
	// c.f., https://stackoverflow.com/questions/5070801
	int           mfd   = open("/proc/mounts", O_RDONLY);
	struct pollfd pfd   = { 0 };
	pfd.fd              = mfd;
	pfd.events          = POLLERR | POLLPRI;
	pfd.revents         = 0;
	uint8_t changes     = 0U;
	uint8_t max_changes = 6U;

	// Assume success unless proven otherwise ;).
	bool rb = true;

	// NOTE: Abort early if the mountpoint is already in the requested state,
	//       in an effort to keep the race window as short as possible...
	if (is_onboard_state(mounted)) {
		goto cleanup;
	}

	// NOTE: We're going with no timeout, which works out great when everything behaves as expected,
	//       but *might* be problematic in case something goes awfully wrong,
	//       in which case we might block for a while...
	while (poll(&pfd, 1, -1) >= 0) {
		if (pfd.revents & POLLERR) {
			LOG("Mountpoints changed (iteration nr. %d of %hhu)", ++changes, max_changes);

			// Stop polling once we know onboard is in the requested state...
			if (is_onboard_state(mounted)) {
				LOG("Good, onboard is finally %s!", mounted ? "mounted" : "unmounted");
				break;
			}
		}
		pfd.revents = 0;

		// If we can't find our mountpoint after that many changes, assume we're screwed...
		if (changes >= max_changes) {
			LOG("Too many mountpoint changes without finding onboard, aborting!");
			rb = false;
			break;
		}
	}

cleanup:
	close(mfd);
	return rb;
}

static bool
    wait_for_background_color(uint8_t v, unsigned short int timeout, unsigned short int granularity)
{
	// Check a single pixel that should always be white in Nickel,
	// and always black when on the "Connected" & "Importing Content" screens.
	// Something on the bottom margin should do the trick,
	// without falling into any "might be behind the bezel" quirk,
	// which would cause it to potentially already be black (or transparent) in Nickel...
	FBInkCoordinates coords = { (unsigned short int) (viewWidth / 2U), (unsigned short int) (viewHeight - 1) };
	(*fxpRotateCoords)(&coords);
	FBInkPixel pixel = { 0U };

	// We loop for <timeout> seconds at most, waking up every <granularity> ms...
	unsigned short int iterations;
	// In case we asked for no timeout (i.e., 0), take that to mean a single iteration...
	if (timeout > 0U) {
		iterations = (unsigned short int) (timeout * (1000 / (float) granularity));
	} else {
		iterations = 1U;
	}
	const struct timespec zzz = { 0L, (long int) (granularity * 1000000L) };
	for (unsigned short int i = 1U; i <= iterations; i++) {
		// Wait <granularity> ms . . .
		nanosleep(&zzz, NULL);

		(*fxpGetPixel)(&coords, &pixel);
		// NOTE: get_pixel_* will only set gray8 (leaving at least bgra.color.r untouched) @ 4 & 8bpp!
		//       (It will unpack RGB565 to RGB32, though ;)).
		if (vInfo.bits_per_pixel > 8U) {
			LOG("On iteration nr. %hu of %hu, pixel (%hu, %hu) was #%02hhX%02hhX%02hhX",
			    i,
			    iterations,
			    coords.x,
			    coords.y,
			    pixel.bgra.color.r,
			    pixel.bgra.color.g,
			    pixel.bgra.color.b);

			// Got it!
			if (pixel.bgra.color.r == v && pixel.bgra.color.g == v && pixel.bgra.color.b == v) {
				return true;
			}
		} else {
			LOG("On iteration nr. %hu of %hu, pixel (%hu, %hu) was #%02hhX",
			    i,
			    iterations,
			    coords.x,
			    coords.y,
			    pixel.gray8);

			// Got it!
			if (pixel.gray8 == v) {
				return true;
			}
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
	return wait_for_background_color(eInkBGCMap[BG_BLACK], 10U, 250U);
}

static bool
    is_on_home_screen(void)
{
	// Home screen has a white background
	LOG("Waiting for the 'Home' screen . . .");
	// NOTE: This one *might* need a slightly larger timeout, just to be safe...
	return wait_for_background_color(eInkBGCMap[BG_WHITE], 12U, 250U);
}

static bool
    is_on_import_screen(void)
{
	// Import screen has a black background
	LOG("Waiting for the 'Content Import' screen . . .");
	return wait_for_background_color(eInkBGCMap[BG_BLACK], 10U, 250U);
}

static bool
    is_on_home_screen_again(void)
{
	// Home screen has a white background
	LOG("Waiting for the 'Home' screen again . . .");
	// NOTE: Give up after 5 minutes...
	return wait_for_background_color(eInkBGCMap[BG_WHITE], (60U * 5U), 750U);
}

static int
    generate_button_press(FBInkCoordinates* restrict match_coords, bool nosleep)
{
	LOG("Pressing the Connect button . . .");
	struct input_event ev;
	int                ifd = -1;
	ifd                    = open("/dev/input/event1", O_WRONLY | O_NONBLOCK | O_CLOEXEC);
	if (ifd == -1) {
		PFWARN("Failed to open input device: %m");
		return ERRCODE(ENODEV);
	}

	// NOTE: Kobo devices use a wide range of weird & quirky variations on the touch input protocol,
	//       depending on the exact device, so do our best to handle that properly...
	// NOTE: Double-check on your device w/ hexdump -x /dev/input/event1 (or -d if you prefer decimal).
	//       Or evemu-record or evtest if you've got KoboStuff installed ;).
	if (deviceQuirks.isKoboNonMT) {
		// NOTE: Should match what Kobo does on devices who don't have a Multi-Touch aware driver...
		//       Should cover the Touch A/B/C, Mini, Glo & Aura HD
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
		//       Should handle the H2O²r2, (Aura SEr2?), Clara HD, Forma.
		//       And possibly the H2O²r1, although this one will need a dedicated quirk to compute the proper x/y coords.
		SEND_INPUT_EVENT(EV_KEY, BTN_TOOL_FINGER, 1);
		SEND_INPUT_EVENT(EV_KEY, BTN_TOUCH, 1);
		SEND_INPUT_EVENT(EV_ABS, ABS_MT_TRACKING_ID, 0);
		SEND_INPUT_EVENT(EV_ABS, ABS_MT_DISTANCE, 0);
		SEND_INPUT_EVENT(EV_ABS, ABS_MT_POSITION_X, match_coords->x);
		SEND_INPUT_EVENT(EV_ABS, ABS_MT_POSITION_Y, match_coords->y);
		// NOTE: Of the four following codes,
		//       which ones are actually set to non-zero values on real events depends on the hardware...
		//       c.f., https://www.mobileread.com/forums/showpost.php?p=4152920&postcount=718 for an example on a Forma.
		//       The lowest common denominator appears to be ABS_MT_PRESSURE,
		//       which is why it's the only event we're setting to a meaningful value here.
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
		// NOTE: Much like above, you can't rely on any of those actually being set to 0 to detect a contact lift...
		//       c.f., process_evdev @ utils/finger_trace.c
		SEND_INPUT_EVENT(EV_ABS, ABS_MT_PRESSURE, 0);
		SEND_INPUT_EVENT(EV_ABS, ABS_MT_TOUCH_MAJOR, 0);
		SEND_INPUT_EVENT(EV_ABS, ABS_MT_TOUCH_MINOR, 0);
		SEND_INPUT_EVENT(EV_ABS, ABS_MT_ORIENTATION, 0);
		SEND_INPUT_EVENT(EV_SYN, SYN_MT_REPORT, 0);
		SEND_INPUT_EVENT(EV_SYN, SYN_REPORT, 0);
		SEND_INPUT_EVENT(EV_KEY, BTN_TOUCH, 0);
		SEND_INPUT_EVENT(EV_KEY, BTN_TOOL_FINGER, 0);
		SEND_INPUT_EVENT(EV_SYN, SYN_REPORT, 0);
	} else if (deviceQuirks.isSunxi) {
		// NOTE: Mostly for documentation's sake, as this feature is unsupported on sunxi SoCs...
		// If multiple contact points are detected (e.g., multi-touch),
		// each tracking ID is preceded by its slot assignment (e.g., EV_ABS:ABS_MT_SLOT:0 for the first finger).
		// It's only elided when there's only a single contact point present/left.
		// NOTE: The fact that it needlessly *repeats* the ABS_MT_TRACKING_ID (i.e., not changed/released),
		//       is upsetting libevdev (trips the "double tracking ID" check)...
		SEND_INPUT_EVENT(EV_ABS, ABS_MT_TRACKING_ID, 0);    // Increases with each subsequent contact point.
		SEND_INPUT_EVENT(EV_ABS, ABS_MT_TOOL_TYPE, 0);      // 0 for finger, 1 for pen
		SEND_INPUT_EVENT(EV_KEY, BTN_TOUCH, 1);
		// Plus an EV_KEY:BTN_STYLUS toggle when the *eraser* button is toggled.
		// And an EV_KEY:BTN_STYLUS2 for the *highlighter* button.
		SEND_INPUT_EVENT(EV_ABS, ABS_MT_TOUCH_MAJOR, 1632);    // Pen can go higher
		// ABS_MT_PRESSURE always matches ABS_MT_TOUCH_MAJOR,
		// unless the pen is not touching the screen (e.g., hovering),
		// in which case ABS_MT_TOUCH_MAJOR is elided, and ABS_MT_PRESSURE reports 0.
		SEND_INPUT_EVENT(EV_ABS, ABS_MT_PRESSURE, 1632);
		// Plus an EV_ABS:ABS_MT_DISTANCE for the pen (15 when PRESSURE is 0 (i.e., pen is hovering), 0 otherwise).
		SEND_INPUT_EVENT(EV_ABS, ABS_MT_POSITION_X, match_coords->x);
		SEND_INPUT_EVENT(EV_ABS, ABS_MT_POSITION_Y, match_coords->y);
		SEND_INPUT_EVENT(EV_SYN, SYN_REPORT, 0);

		SEND_INPUT_EVENT(EV_ABS, ABS_MT_TRACKING_ID, 0);
		SEND_INPUT_EVENT(EV_ABS, ABS_MT_TOOL_TYPE, 0);
		SEND_INPUT_EVENT(EV_ABS, ABS_MT_TOUCH_MAJOR, 0);
		SEND_INPUT_EVENT(EV_ABS, ABS_MT_PRESSURE, 0);
		SEND_INPUT_EVENT(EV_ABS, ABS_MT_POSITION_X, match_coords->x);
		SEND_INPUT_EVENT(EV_ABS, ABS_MT_POSITION_Y, match_coords->y);
		SEND_INPUT_EVENT(EV_ABS, ABS_MT_TRACKING_ID, -1);
		SEND_INPUT_EVENT(EV_KEY, BTN_TOUCH, 0);
		SEND_INPUT_EVENT(EV_SYN, SYN_REPORT, 0);
	} else {
		// NOTE: Corresponds to what we call the "Phoenix" protocol in KOReader.
		//       Which means we should cover the: KA1, H2O, Aura, Aura SEr1, (Aura SEr2?), Glo HD, Touch 2.0 & Nia.
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
	//       e.g., checking the state of the /mnt/onboard mountpoint...
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
    fbink_button_scan(int fbfd UNUSED_BY_NOBUTTON, bool press_button UNUSED_BY_NOBUTTON, bool nosleep UNUSED_BY_NOBUTTON)
{
#ifdef FBINK_WITH_BUTTON_SCAN
	if (deviceQuirks.isSunxi) {
		PFWARN("This feature is not supported on your device");
		return ERRCODE(ENOSYS);
	}

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
	FBInkColor button_color = { 0xD9u, 0xD9u, 0xD9u };

	// And handle yet another bit of 16bpp weirdness...
	// NOTE: RGB565 conversions are complex and potentially slightly lossy,
	//       with slight rounding/truncation errors that can be different depending on how exactly the conversions were done.
	//       This matches with what *we* do, hopefully that'll be close enough to what Nickel actually does...
	if (deviceQuirks.isNTX16bLandscape) {
		button_color.r = 0xDEu;
		button_color.g = 0xDBu;
		button_color.b = 0xDEu;
	}

	FBInkPixel         pixel         = { 0U };
	FBInkCoordinates   coords        = { 0U };
	unsigned short int button_width  = 0U;
	unsigned short int match_count   = 0U;
	FBInkCoordinates   match_coords  = { 0U };
	bool               gotcha        = false;
	unsigned short int button_height = 0U;

	// Centralize the various thresholds we use...
	// NOTE: Depending on the device's DPI & resolution, a button takes between 17% and 20% of the screen's width.
	//       Possibly less on larger resolutions, and more on smaller resolutions, so try to handle everyone in one fell swoop.
	// NOTE: Truncating floats via explicit casts is good enough for us.
	//       With poor man's rounding, no need to pull in libm ;).
	unsigned short int min_target_pixels = (unsigned short int) ((0.125f * (float) viewWidth) + 0.5f);
	unsigned short int max_target_pixels = (unsigned short int) ((0.25f * (float) viewWidth) + 0.5f);

	// Recap the various settings as computed for this screen...
	LOG("Button color is expected to be #%02hhX%02hhX%02hhX", button_color.r, button_color.g, button_color.b);
	LOG("We need to match two buttons each between %hu and %hu pixels wide!", min_target_pixels, max_target_pixels);

	// Only look in the area of the screen where we're likely to find the buttons, both to save some time,
	// and to lower the risk of false positives, as unlikely as that might be.
	unsigned short int min_height = (unsigned short int) ((0.55f * (float) viewHeight) + 0.5f);
	unsigned short int max_height = (unsigned short int) ((0.85f * (float) viewHeight) + 0.5f);
	unsigned short int min_width  = (unsigned short int) ((0.05f * (float) viewWidth) + 0.5f);
	unsigned short int max_width  = (unsigned short int) ((0.80f * (float) viewWidth) + 0.5f);

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
			(*fxpGetPixel)(&coords, &pixel);

			// NOTE: Again, get_pixel_* will only set gray8 @ 4 & 8bpp
			if ((vInfo.bits_per_pixel > 8U && pixel.bgra.color.r == button_color.r &&
			     pixel.bgra.color.g == button_color.g && pixel.bgra.color.b == button_color.b) ||
			    (vInfo.bits_per_pixel <= 8U && pixel.gray8 == button_color.b)) {
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
			(*fxpGetPixel)(&coords, &pixel);

			// NOTE: Again, with the gray/rgb dance...
			if ((vInfo.bits_per_pixel > 8U && pixel.bgra.color.r == button_color.r &&
			     pixel.bgra.color.g == button_color.g && pixel.bgra.color.b == button_color.b) ||
			    (vInfo.bits_per_pixel <= 8U && pixel.gray8 == button_color.b)) {
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
				gotcha         = true;
				break;
			}
		}
	}

	if (gotcha) {
		LOG("Matched on a %hux%hu button centered @ (%hu, %hu)! :)",
		    button_width,
		    button_height,
		    match_coords.x,
		    match_coords.y);

		// The touch panel has a fixed origin that differs from the framebuffer's... >_<".
		// NOTE: On the Forma, take the current rotation into account,
		//       because the Home screen *can* be shown in Inverted Portrait...
		//       Note that while rotate_touch_coordinates handles Landscape orientations just fine,
		//       the various optimizations that limit the area to scan and the size of the button to match
		//       are completely tailored for Portrait orientations, and would probably need tweaking for Landscape...
		if (deviceQuirks.canRotate) {
			rotate_touch_coordinates(&match_coords);
		} else {
			rotate_coordinates_pickel(&match_coords);
		}
		ELOG("Transformed for input -> x=%hu, y=%hu", match_coords.x, match_coords.y);

		// NOTE: The H2O²r1 is a special snowflake, input is rotated 90° in the *other* direction
		//       (i.e., origin at the bottom-left instead of top-right).
		//       Hopefully that doesn't apply to the fb itself, too...
		// NOTE: Hide that on the Forma, since it's definitely useless there ;).
		if (!deviceQuirks.canRotate) {
			ELOG("Transformed for H2O²r1 input -> x=%hu, y=%hu",
			     (unsigned short int) (screenHeight - match_coords.x - 1),
			     (unsigned short int) (screenWidth - match_coords.y - 1));
		}

		// Press it if requested...
		if (press_button) {
			if ((rv = generate_button_press(&match_coords, nosleep)) != EXIT_SUCCESS) {
				WARN("Failed to press the Connect button");
				goto cleanup;
			} else {
				LOG(". . . appears to have been a success!");
			}
		}
	} else {
		LOG("No match :(");
		WARN("Failed to find a Connect button on screen");
		rv = ERRCODE(EXIT_FAILURE);
		goto cleanup;
	}

	// Cleanup
cleanup:
	if (isFbMapped && !keep_fd) {
		unmap_fb();
	}
	if (!keep_fd) {
		close_fb(fbfd);
	}

	return rv;
#else
	WARN("Kobo Connect button scanning is disabled in this FBInk build");
	return ERRCODE(ENOSYS);
#endif    // FBINK_WITH_BUTTON_SCAN
}

// Wait for the end of an USBMS session, trying to detect a successful content import in the process.
int
    fbink_wait_for_usbms_processing(int fbfd UNUSED_BY_NOBUTTON, bool force_unplug UNUSED_BY_NOBUTTON)
{
#ifdef FBINK_WITH_BUTTON_SCAN
	if (deviceQuirks.isSunxi) {
		PFWARN("This feature is not supported on your device");
		return ERRCODE(ENOSYS);
	}

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

	// Double-check that we're really on something that looks like the Connected screen,
	// in case someone slipped on the wrong CLI flag or the wrong function ;).
	if (!wait_for_background_color(eInkBGCMap[BG_BLACK], 0U, 0U)) {
		// That won't do... abort!
		WARN("We don't appear to be on the Connected screen, abort");
		rv = ERRCODE(EXIT_FAILURE);
		goto cleanup;
	}

	// NOTE: Since USB is terrible, it may take a bit for onboard to *actually* get unmounted,
	//       so we wait for that to happen (wait_for_onboard_state() will return early if that's already the case)...
	bool mounted = true;
	LOG("Waiting for the start of the USBMS session . . .");
	if (!wait_for_onboard_state(!mounted)) {
		// That won't do... abort!
		WARN("Failed to detect start of USBMS session, can't detect content import");
		rv = ERRCODE(EXIT_FAILURE);
		goto cleanup;
	}

	// Right, now that we've made sure that we're properly in USBMS, wait for onboard to come back up :)
	LOG("Waiting for onboard to come back up . . .");

	// If we were asked to force an unplug event, now is the time ;).
	if (force_unplug) {
		LOG("Sending an USB unplug event . . .");
		int nfd = -1;
		nfd     = open("/tmp/nickel-hardware-status", O_WRONLY | O_NONBLOCK | O_CLOEXEC);
		if (nfd == -1) {
			PFWARN("Failed to open Nickel pipe: %m");
			rv = ERRCODE(EXIT_FAILURE);
			goto cleanup;
		}

		const unsigned char cmd[] = "usb plug remove";
		if (write(nfd, cmd, sizeof(cmd)) < 0) {
			PFWARN("Failed to write to Nickel pipe: %m");
			rv = ERRCODE(EXIT_FAILURE);
			close(nfd);
			goto cleanup;
		}

		close(nfd);
	}

	if (!wait_for_onboard_state(mounted)) {
		// That won't do... abort!
		WARN("Failed to detect end of USBMS session, can't detect content import");
		rv = ERRCODE(EXIT_FAILURE);
		goto cleanup;
	}

	// Now check that we're back on the Home screen...
	if (!is_on_home_screen()) {
		// That won't do... abort!
		WARN("Failed to detect Home screen, can't detect content import");
		rv = ERRCODE(EXIT_FAILURE);
		goto cleanup;
	}

	// Then wait a while to see if the Import screen pops up...
	if (!is_on_import_screen()) {
		// Maybe there was nothing to import?
		WARN("Couldn't detect the Import screen, maybe there was nothing to import?");
		rv = ERRCODE(ENODATA);
		goto cleanup;
	}

	// Then wait a potentially long while (~5min) for the end of the Import process...
	if (!is_on_home_screen_again()) {
		// NOTE: LF better method than a stupid hard timeout... ;'(
		WARN(
		    "Failed to detect the end of the Import process, maybe it's hung or running suspiciously long (it's been > 5min)?");
		rv = ERRCODE(ETIME);
		goto cleanup;
	}

	// Cleanup
cleanup:
	if (isFbMapped && !keep_fd) {
		unmap_fb();
	}
	if (!keep_fd) {
		close_fb(fbfd);
	}

	return rv;
#else
	WARN("Kobo USBMS monitoring is disabled in this FBInk build");
	return ERRCODE(ENOSYS);
#endif    // FBINK_WITH_BUTTON_SCAN
}
