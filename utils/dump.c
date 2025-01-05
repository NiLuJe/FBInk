/*
	FBInk: FrameBuffer eInker, a library to print text & images to an eInk Linux framebuffer
	Copyright (C) 2018-2024 NiLuJe <ninuje@gmail.com>
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

// NOTE: Fairly useless piece of code basically just there to test the dump/restore functionality ;).
//       We could arguably plug into stb_image_write to basically reimplement fbgrab...

// Because we're pretty much Linux-bound ;).
#ifndef _GNU_SOURCE
#	define _GNU_SOURCE
#endif

// NOTE: We need image support & basic font support.
//       A MINIMAL + BITMAP + IMAGE build is still recommended, because otherwise fbink_init() has to pull all the extra fonts in...
#ifdef FBINK_MINIMAL
#	ifndef FBINK_WITH_BITMAP
#		error Cannot build this tool without fixed-cell font rendering support!
#	endif
#	ifndef FBINK_WITH_IMAGE
#		error Cannot build this tool without Image support!
#	endif
#endif

#include <errno.h>
#include <linux/fb.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/ioctl.h>

#include "../fbink.h"

// We want to return negative values on failure, always
#define ERRCODE(e) (-(e))

// MIN/MAX with no side-effects,
// c.f., https://gcc.gnu.org/onlinedocs/cpp/Duplication-of-Side-Effects.html#Duplication-of-Side-Effects
//     & https://dustri.org/b/min-and-max-macro-considered-harmful.html
#define MIN(X, Y)                                                                                                        \
	({                                                                                                               \
		__auto_type x_ = (X);                                                                                    \
		__auto_type y_ = (Y);                                                                                    \
		(x_ < y_) ? x_ : y_;                                                                                     \
	})

#define MAX(X, Y)                                                                                                        \
	({                                                                                                               \
		__auto_type x__ = (X);                                                                                   \
		__auto_type y__ = (Y);                                                                                   \
		(x__ > y__) ? x__ : y__;                                                                                 \
	})

// Likely/Unlikely branch tagging
#define likely(x)   __builtin_expect(!!(x), 1)
#define unlikely(x) __builtin_expect(!!(x), 0)

int
    main(int argc, char* argv[] __attribute__((unused)))
{
	// Setup FBInk
	FBInkConfig fbink_cfg = { 0 };
	FBInkDump   dump      = { 0 };
	fbink_cfg.is_verbose  = true;
	// Flash to make stuff more obvious
	fbink_cfg.is_flashing = true;

	// Quick'n dirty way to disable wait_for_complete, because this makes a fun testcase of how the EPDC handles merging...
	bool wait_for = true;
	if (argc > 1) {
		wait_for = false;
		fprintf(stdout, "Enabled non-blocking refreshes, expect massive optimization (merging) from the EPDC!\n");
	}

	// Assume success, until shit happens ;)
	int rv = EXIT_SUCCESS;

	// Init FBInk
	int fbfd = fbink_open();
	// Open framebuffer and keep it around, then setup globals.
	if (fbfd == ERRCODE(EXIT_FAILURE)) {
		fprintf(stderr, "Failed to open the framebuffer, aborting . . .\n");
		rv = ERRCODE(EXIT_FAILURE);
		goto cleanup;
	}
	if (fbink_init(fbfd, &fbink_cfg) != EXIT_SUCCESS) {
		fprintf(stderr, "Failed to initialize FBInk, aborting . . .\n");
		rv = ERRCODE(EXIT_FAILURE);
		goto cleanup;
	}

	// Dump
	fprintf(stdout, "[01] DUMP FULL\n");
	if (fbink_dump(fbfd, &dump) != ERRCODE(EXIT_SUCCESS)) {
		fprintf(stderr, "Failed to dump fb, aborting . . .\n");
		rv = ERRCODE(EXIT_FAILURE);
		goto cleanup;
	}

	// Print random crap
	fprintf(stdout, "[02] PRINT\n");
	fbink_cfg.is_centered = true;
	fbink_cfg.is_padded   = true;
	fbink_cfg.is_halfway  = true;
	// Invert the text to make the subsequent region restore easier to spot
	fbink_cfg.is_inverted = true;
	if (fbink_print(fbfd, "Wheeee!", &fbink_cfg) < 0) {
		fprintf(stderr, "Failed to print!\n");
		rv = ERRCODE(EXIT_FAILURE);
		goto cleanup;
	}

	// Restore
	fprintf(stdout, "[03] RESTORE\n");
	if (fbink_restore(fbfd, &fbink_cfg, &dump) != ERRCODE(EXIT_SUCCESS)) {
		fprintf(stderr, "Failed to restore fb, aborting . . .\n");
		rv = ERRCODE(EXIT_FAILURE);
		goto cleanup;
	}
	if (wait_for) {
		fbink_wait_for_complete(fbfd, LAST_MARKER);
	}

	// Dump a region at the center of the screen, with a few funky offsets to test that
	fprintf(stdout, "[04] DUMP REGION\n");
	fbink_cfg.halign = CENTER;
	fbink_cfg.valign = CENTER;
	if (fbink_region_dump(fbfd, -650, -50, 501, 250, &fbink_cfg, &dump) != ERRCODE(EXIT_SUCCESS)) {
		fprintf(stderr, "Failed to dump fb region, aborting . . .\n");
		rv = ERRCODE(EXIT_FAILURE);
		goto cleanup;
	}

	// Print random crap, again
	fprintf(stdout, "[05] PRINT\n");
	if (fbink_print(fbfd, "Wheeee!", &fbink_cfg) < 0) {
		fprintf(stderr, "Failed to print!\n");
		rv = ERRCODE(EXIT_FAILURE);
		goto cleanup;
	}

	// Restore, again
	fprintf(stdout, "[06] RESTORE\n");
	if (fbink_restore(fbfd, &fbink_cfg, &dump) != ERRCODE(EXIT_SUCCESS)) {
		fprintf(stderr, "Failed to restore fb, aborting . . .\n");
		rv = ERRCODE(EXIT_FAILURE);
		goto cleanup;
	}
	if (wait_for) {
		fbink_wait_for_complete(fbfd, LAST_MARKER);
	}

	// We'll be needing some info for the following tests...
	FBInkState fbink_state = { 0 };
	fbink_get_state(&fbink_cfg, &fbink_state);

	// Restore, this time with a positive L + T crop
	fprintf(stdout, "[06b+] RESTORE w/ (+) L+T CROP\n");
	dump.clip      = dump.area;
	dump.clip.left = (unsigned short int) MIN(dump.clip.left + 25U, fbink_state.screen_width);
	//dump.clip.width -= 25;	// Not strictly necessary, will be computed when building the intersection rectangle
	dump.clip.top  = (unsigned short int) MIN(dump.clip.top + 30U, fbink_state.screen_height);
	//dump.clip.height -= 30;	// Ditto
	if (fbink_restore(fbfd, &fbink_cfg, &dump) != ERRCODE(EXIT_SUCCESS)) {
		fprintf(stderr, "Failed to restore fb, aborting . . .\n");
		rv = ERRCODE(EXIT_FAILURE);
		goto cleanup;
	}
	if (wait_for) {
		fbink_wait_for_complete(fbfd, LAST_MARKER);
	}
	// Forget about the crop for the other tests
	dump.clip = (const FBInkRect) { 0U };

	// Restore, this time with a negative L + T crop
	fprintf(stdout, "[06b-] RESTORE w/ (-) L+T CROP\n");
	dump.clip      = dump.area;
	dump.clip.left = (unsigned short int) MAX(0, dump.clip.left - 25);
	//dump.clip.width -= 25;	// Not strictly necessary, will be computed when building the intersection rectangle
	dump.clip.top  = (unsigned short int) MAX(0, dump.clip.top - 30);
	//dump.clip.height -= 30;	// Ditto
	if (fbink_restore(fbfd, &fbink_cfg, &dump) != ERRCODE(EXIT_SUCCESS)) {
		fprintf(stderr, "Failed to restore fb, aborting . . .\n");
		rv = ERRCODE(EXIT_FAILURE);
		goto cleanup;
	}
	if (wait_for) {
		fbink_wait_for_complete(fbfd, LAST_MARKER);
	}
	// Forget about the crop for the other tests
	dump.clip = (const FBInkRect) { 0U };

	// Restore, this time with a positive R + B crop
	dump.clip        = dump.area;
	dump.clip.width  = (unsigned short int) MAX(0, dump.clip.width - 25);
	dump.clip.height = (unsigned short int) MAX(0, dump.clip.height - 30);
	fprintf(stdout, "[06c+] RESTORE w/ (+) R+B CROP\n");
	if (fbink_restore(fbfd, &fbink_cfg, &dump) != ERRCODE(EXIT_SUCCESS)) {
		fprintf(stderr, "Failed to restore fb, aborting . . .\n");
		rv = ERRCODE(EXIT_FAILURE);
		goto cleanup;
	}
	if (wait_for) {
		fbink_wait_for_complete(fbfd, LAST_MARKER);
	}
	// Forget about the crop for the other tests
	dump.clip = (const FBInkRect) { 0U };

	// Restore, this time with a negative R + B crop (i.e., the overlap will match the full dump area)
	dump.clip        = dump.area;
	dump.clip.width  = (unsigned short int) MIN(dump.clip.width + 25U, fbink_state.screen_width);
	dump.clip.height = (unsigned short int) MIN(dump.clip.height + 30U, fbink_state.screen_height);
	fprintf(stdout, "[06c-] RESTORE w/ (-) R+B CROP\n");
	if (fbink_restore(fbfd, &fbink_cfg, &dump) != ERRCODE(EXIT_SUCCESS)) {
		fprintf(stderr, "Failed to restore fb, aborting . . .\n");
		rv = ERRCODE(EXIT_FAILURE);
		goto cleanup;
	}
	if (wait_for) {
		fbink_wait_for_complete(fbfd, LAST_MARKER);
	}
	// Forget about the crop for the other tests
	dump.clip = (const FBInkRect) { 0U };

	// Restore, this time with a crop on all sides
	dump.clip        = dump.area;
	dump.clip.left   = (unsigned short int) MIN(dump.clip.left + 15U, fbink_state.screen_width);
	dump.clip.width  = (unsigned short int) MAX(0, dump.clip.width - 15);
	dump.clip.top    = (unsigned short int) MIN(dump.clip.top + 30U, fbink_state.screen_height);
	dump.clip.height = (unsigned short int) MAX(0, dump.clip.height - 30);
	dump.clip.width  = (unsigned short int) MAX(0, dump.clip.width - 20);
	dump.clip.height = (unsigned short int) MAX(0, dump.clip.height - 25);
	fprintf(stdout, "[06d] RESTORE w/ T+B+L+R CROP\n");
	if (fbink_restore(fbfd, &fbink_cfg, &dump) != ERRCODE(EXIT_SUCCESS)) {
		fprintf(stderr, "Failed to restore fb, aborting . . .\n");
		rv = ERRCODE(EXIT_FAILURE);
		goto cleanup;
	}
	if (wait_for) {
		fbink_wait_for_complete(fbfd, LAST_MARKER);
	}
	// Forget about the crop for the other tests
	dump.clip = (const FBInkRect) { 0U };

	// This should fail to restore (no overlap)
	dump.clip = dump.area;
	dump.clip.left =
	    (unsigned short int) MIN((unsigned short int) (dump.clip.left + dump.area.width), fbink_state.screen_width);
	fprintf(stdout, "[06e] RESTORE w/ broken CROP\n");
	if (fbink_restore(fbfd, &fbink_cfg, &dump) != ERRCODE(EXIT_SUCCESS)) {
		fprintf(stderr, "Failed to restore fb, as expected :)\n");
	} else {
		fprintf(stderr, "Err, dump was restored *despite* a non-overlapping crop ?!\n");
		rv = ERRCODE(EXIT_FAILURE);
		goto cleanup;
	}
	// Forget about the crop for the other tests
	dump.clip = (const FBInkRect) { 0U };

	// Now, for something slightly crazy, which I don't necessarily recommend relying on,
	// restore, but at a different position.
	// This is slightly crazy because we'll be playing with the dump's area *directly*,
	// which is obviously potentially extremely unsafe ^^.
	// NOTE: On that note, as far as this specific trick is concerned, absolutely no safety checks are done by FBInk
	//       (i.e., it utterly *trusts* the content of the FBInkDump struct).
	//       Making sure you don't go off-screen, or blow past a scanline or screen boundary is *your* responsibility.
	// First, keep a copy of the original, sane dump area around.
	const FBInkRect orig_area = dump.area;
	dump.area.left            = (unsigned short int) (dump.area.left + 250U);
	dump.area.top             = (unsigned short int) (dump.area.top - 125U);
	fprintf(stdout, "[06f] CRAZY RESTORE w/ MOVE\n");
	if (fbink_restore(fbfd, &fbink_cfg, &dump) != ERRCODE(EXIT_SUCCESS)) {
		fprintf(stderr, "Failed to restore fb, aborting . . .\n");
		rv = ERRCODE(EXIT_FAILURE);
		goto cleanup;
	}
	if (wait_for) {
		fbink_wait_for_complete(fbfd, LAST_MARKER);
	}
	// Restore the sane dump area
	dump.area = orig_area;

	// And while we're on the crazy train, let's do a manual unchecked crop on top of that move...
	// Obviously, unlike via clip, you can only crop from the right & bottom edges ;).
	dump.area.left   = (unsigned short int) (dump.area.left + 500U);
	dump.area.top    = (unsigned short int) (dump.area.top - 250U);
	dump.area.width  = (unsigned short int) (dump.area.width - 50U);
	dump.area.height = (unsigned short int) (dump.area.height - 125U);
	// NOTE: In practice, to avoid issues, you'd have to make sure:
	//       0 <= left < screen_width
	//       0 <= top < screen_height
	//       (left + width) <= screen_width
	//       (top + height) <= screen_height
	// So, let's do it...
	dump.area.left   = (unsigned short int) MIN(dump.area.left, fbink_state.screen_width - 1U);
	dump.area.top    = (unsigned short int) MIN(dump.area.top, fbink_state.screen_height - 1U);
	if (dump.area.left + dump.area.width > fbink_state.screen_width) {
		dump.area.width = (unsigned short int) (fbink_state.screen_width - dump.area.left);
	}
	if (dump.area.top + dump.area.height > fbink_state.screen_height) {
		dump.area.height = (unsigned short int) (fbink_state.screen_height - dump.area.top);
	}
	fprintf(stdout, "[06g] CRAZY RESTORE w/ MOVE + RAW CROP\n");
	if (fbink_restore(fbfd, &fbink_cfg, &dump) != ERRCODE(EXIT_SUCCESS)) {
		fprintf(stderr, "Failed to restore fb, aborting . . .\n");
		rv = ERRCODE(EXIT_FAILURE);
		goto cleanup;
	}
	if (wait_for) {
		fbink_wait_for_complete(fbfd, LAST_MARKER);
	}
	// Restore the sane dump area
	dump.area = orig_area;

#ifndef FBINK_FOR_LINUX
	// And now for some fun stuff, provided we're starting from a 32bpp fb...
	if (fbink_state.bpp == 32U) {
		// Switch to 8bpp (c.f., fbdepth.c)
		fprintf(stdout, "[07] SWITCH TO 8BPP\n");
		if (fbink_set_fb_info(fbfd, KEEP_CURRENT_ROTATE, 8U, KEEP_CURRENT_GRAYSCALE, &fbink_cfg) < 0) {
			fprintf(stderr, "Failed to swap bitdepth, aborting . . .\n");
			rv = ERRCODE(EXIT_FAILURE);
			goto cleanup;
		}

		// Re-init fbink so it registers the bitdepth switch
		fprintf(stdout, "[08] REINIT\n");
		if (unlikely(fbink_reinit(fbfd, &fbink_cfg) < 0)) {
			// We don't track state, so we only need to handle plain failures.
			fprintf(stderr, "fbink_reinit failed!\n");
			rv = ERRCODE(EXIT_FAILURE);
			goto cleanup;
		}

		// Print random crap, once more
		fprintf(stdout, "[09] PRINT\n");
		if (fbink_print(fbfd, "Wheeee!", &fbink_cfg) < 0) {
			fprintf(stderr, "Failed to print!\n");
			rv = ERRCODE(EXIT_FAILURE);
			goto cleanup;
		}

		// Watch the restore fail because of a bitdepth mismatch
		fprintf(stdout, "[10] RESTORE MISMATCH\n");
		if (fbink_restore(fbfd, &fbink_cfg, &dump) != ERRCODE(EXIT_SUCCESS)) {
			fprintf(stderr, "Failed to restore fb, as expected :)\n");
		} else {
			fprintf(stderr, "Err, dump was restored *despite* a bitdepth mismatch ?!\n");
			rv = ERRCODE(EXIT_FAILURE);
			goto cleanup;
		}

		// Disable H & V centering, so that fbink_print_raw_data honors our dump coordinates as-is,
		// without re-centering them ;).
		fbink_cfg.halign      = NONE;
		fbink_cfg.valign      = NONE;
		// We need to disable viewport shenanigans, too...
		fbink_cfg.no_viewport = true;
		// no_viewport requires a *full* reinit (i.e., init) to be taken into account...
		fprintf(stdout, "[11] FULL REINIT\n");
		fbink_init(fbfd, &fbink_cfg);
		// We leave invert on, to make stuff more obvious, though ;).

		// Then ninja restore it via fbink_print_raw_data, which will handle the 32bpp-to-8bpp conversion for us...
		fprintf(stdout, "[12] PRINT RAW\n");
		if (fbink_print_raw_data(fbfd,
					 dump.data,
					 dump.area.width,
					 dump.area.height,
					 dump.size,
					 (short int) dump.area.left,
					 (short int) dump.area.top,
					 &fbink_cfg) != ERRCODE(EXIT_SUCCESS)) {
			fprintf(stderr, "Failed to print raw data!\n");
			rv = ERRCODE(EXIT_FAILURE);
			goto cleanup;
		}
		if (wait_for) {
			fbink_wait_for_complete(fbfd, LAST_MARKER);
		}

		// One more time, with a x2 scaling
		// We tweak the offsets to potentially push part of it offscreen, to test the bounds checking.
		fbink_cfg.halign        = CENTER;
		fbink_cfg.valign        = CENTER;
		fbink_cfg.scaled_height = (short int) (dump.area.height * 2U);
		fbink_cfg.scaled_width  = (short int) (dump.area.width * 2U);
		fprintf(stdout, "[13] SCALED PRINT RAW\n");
		if (fbink_print_raw_data(
			fbfd, dump.data, dump.area.width, dump.area.height, dump.size, 125, 550, &fbink_cfg) !=
		    ERRCODE(EXIT_SUCCESS)) {
			fprintf(stderr, "Failed to print raw data!\n");
			rv = ERRCODE(EXIT_FAILURE);
			goto cleanup;
		}
		if (wait_for) {
			fbink_wait_for_complete(fbfd, LAST_MARKER);
		}

		// Switch back to 32bpp
		fprintf(stdout, "[14] SWITCH TO 32BPP\n");
		if (fbink_set_fb_info(fbfd, KEEP_CURRENT_ROTATE, 32U, KEEP_CURRENT_GRAYSCALE, &fbink_cfg) < 0) {
			fprintf(stderr, "Failed to swap bitdepth, aborting . . .\n");
			rv = ERRCODE(EXIT_FAILURE);
			goto cleanup;
		}
	}
#endif    // !FBINK_FOR_LINUX

	// Cleanup
cleanup:
	// Free potential dump data...
	if (fbink_free_dump_data(&dump) == ERRCODE(EINVAL)) {
		fprintf(stderr, "There was no dump data to release!\n");
		rv = ERRCODE(EXIT_FAILURE);
	}

	if (fbink_close(fbfd) == ERRCODE(EXIT_FAILURE)) {
		fprintf(stderr, "Failed to close the framebuffer, aborting . . .\n");
		rv = ERRCODE(EXIT_FAILURE);
	}

	return rv;
}
