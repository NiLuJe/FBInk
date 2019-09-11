/*
	FBInk: FrameBuffer eInker, a tool to print text & images on eInk devices (Kobo/Kindle)
	Copyright (C) 2018-2019 NiLuJe <ninuje@gmail.com>
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

#include "../fbink.h"
#include <errno.h>
#include <linux/fb.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/ioctl.h>

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

// "Small" helper for bitdepth switch... (c.f., fbdepth.c)
static bool
    set_bpp(int fbfd, uint32_t bpp, const FBInkState* restrict fbink_state)
{
	struct fb_var_screeninfo fb_vinfo;
	if (ioctl(fbfd, FBIOGET_VSCREENINFO, &fb_vinfo)) {
		perror("ioctl GET_V");
		return false;
	}
	struct fb_fix_screeninfo fb_finfo;
	if (ioctl(fbfd, FBIOGET_FSCREENINFO, &fb_finfo)) {
		perror("ioctl GET_F");
		return false;
	}

	uint32_t expected_rota = fb_vinfo.rotate;

	fb_vinfo.bits_per_pixel = (uint32_t) bpp;
	if (bpp == 8U) {
		fb_vinfo.grayscale = (uint32_t) 1U;
	} else {
		fb_vinfo.grayscale = (uint32_t) 0U;
	}

	if (fbink_state->ntx_rota_quirk == NTX_ROTA_ALL_INVERTED) {
		// NOTE: This should cover the H2O and the few other devices suffering from the same quirk...
		fb_vinfo.rotate ^= 2;
	} else if (fbink_state->ntx_rota_quirk == NTX_ROTA_ODD_INVERTED) {
		// NOTE: This is for the Forma, which only inverts CW & CCW (i.e., odd numbers)...
		if ((fb_vinfo.rotate & 0x01) == 1) {
			fb_vinfo.rotate ^= 2;
		}
	}

	if (ioctl(fbfd, FBIOPUT_VSCREENINFO, &fb_vinfo)) {
		perror("ioctl PUT_V");
		return false;
	}

	if (fb_vinfo.rotate != expected_rota) {
		// Brute-force it until it matches...
		for (uint32_t i = fb_vinfo.rotate, j = FB_ROTATE_UR; j <= FB_ROTATE_CCW; i = (i + 1U) & 3U, j++) {
			// If we finally got the right orientation, break the loop
			if (fb_vinfo.rotate == expected_rota) {
				break;
			}
			// Do the i -> i + 1 -> i dance to be extra sure...
			// (This is useful on devices where the kernel *always* switches to the invert orientation, c.f., rota.c)
			fb_vinfo.rotate = i;
			if (ioctl(fbfd, FBIOPUT_VSCREENINFO, &fb_vinfo)) {
				perror("ioctl PUT_V");
				return false;
			}

			// Don't do anything extra if that was enough...
			if (fb_vinfo.rotate == expected_rota) {
				continue;
			}
			// Now for i + 1 w/ wraparound, since the valid rotation range is [0..3] (FB_ROTATE_UR to FB_ROTATE_CCW).
			// (i.e., a Portrait/Landscape swap to counteract potential side-effects of a kernel-side mandatory invert)
			uint32_t n      = (i + 1U) & 3U;
			fb_vinfo.rotate = n;
			if (ioctl(fbfd, FBIOPUT_VSCREENINFO, &fb_vinfo)) {
				perror("ioctl PUT_V");
				return false;
			}

			// And back to i, if need be...
			if (fb_vinfo.rotate == expected_rota) {
				continue;
			}
			fb_vinfo.rotate = i;
			if (ioctl(fbfd, FBIOPUT_VSCREENINFO, &fb_vinfo)) {
				perror("ioctl PUT_V");
				return false;
			}
		}
	}

	return true;
}

int
    main(void)
{
	// Setup FBInk
	FBInkConfig fbink_cfg = { 0 };
	FBInkDump   dump      = { 0 };
	fbink_cfg.is_verbose  = true;
	// Flash to make stuff more obvious
	fbink_cfg.is_flashing = true;

	// Assume success, until shit happens ;)
	int rv = EXIT_SUCCESS;

	// Init FBInk
	int fbfd = -1;
	// Open framebuffer and keep it around, then setup globals.
	if ((fbfd = fbink_open()) == ERRCODE(EXIT_FAILURE)) {
		fprintf(stderr, "Failed to open the framebuffer, aborting . . .\n");
		rv = ERRCODE(EXIT_FAILURE);
		goto cleanup;
	}
	if (fbink_init(fbfd, &fbink_cfg) == ERRCODE(EXIT_FAILURE)) {
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
	fbink_wait_for_complete(fbfd, LAST_MARKER);

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
	fbink_wait_for_complete(fbfd, LAST_MARKER);

	// We'll be needing some info for the following tests...
	FBInkState fbink_state = { 0 };
	fbink_get_state(&fbink_cfg, &fbink_state);

	// Restore, this time with a positive L + T crop
	fprintf(stdout, "[06b+] RESTORE w/ (+) L+T CROP\n");
	dump.clip      = dump.area;
	dump.clip.left = (unsigned short int) MIN(dump.clip.left + 25U, fbink_state.screen_width);
	//dump.clip.width -= 25;	// Not strictly necessary, will be computed when building the intersection rectangle
	dump.clip.top = (unsigned short int) MIN(dump.clip.top + 30U, fbink_state.screen_height);
	//dump.clip.height -= 30;	// Ditto
	if (fbink_restore(fbfd, &fbink_cfg, &dump) != ERRCODE(EXIT_SUCCESS)) {
		fprintf(stderr, "Failed to restore fb, aborting . . .\n");
		rv = ERRCODE(EXIT_FAILURE);
		goto cleanup;
	}
	fbink_wait_for_complete(fbfd, LAST_MARKER);
	// Forget about the crop for the other tests
	dump.clip = (const FBInkRect){ 0U };

	// Restore, this time with a negative L + T crop
	fprintf(stdout, "[06b-] RESTORE w/ (-) L+T CROP\n");
	dump.clip      = dump.area;
	dump.clip.left = (unsigned short int) MAX(0, dump.clip.left - 25);
	//dump.clip.width -= 25;	// Not strictly necessary, will be computed when building the intersection rectangle
	dump.clip.top = (unsigned short int) MAX(0, dump.clip.top - 30);
	//dump.clip.height -= 30;	// Ditto
	if (fbink_restore(fbfd, &fbink_cfg, &dump) != ERRCODE(EXIT_SUCCESS)) {
		fprintf(stderr, "Failed to restore fb, aborting . . .\n");
		rv = ERRCODE(EXIT_FAILURE);
		goto cleanup;
	}
	fbink_wait_for_complete(fbfd, LAST_MARKER);
	// Forget about the crop for the other tests
	dump.clip = (const FBInkRect){ 0U };

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
	fbink_wait_for_complete(fbfd, LAST_MARKER);
	// Forget about the crop for the other tests
	dump.clip = (const FBInkRect){ 0U };

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
	fbink_wait_for_complete(fbfd, LAST_MARKER);
	// Forget about the crop for the other tests
	dump.clip = (const FBInkRect){ 0U };

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
	fbink_wait_for_complete(fbfd, LAST_MARKER);
	// Forget about the crop for the other tests
	dump.clip = (const FBInkRect){ 0U };

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
	dump.clip = (const FBInkRect){ 0U };

	// And now for some fun stuff, provided we're starting from a 32bpp fb...
	if (fbink_state.bpp == 32U) {
		// Switch to 8bpp (c.f., fbdepth.c)
		fprintf(stdout, "[07] SWITCH TO 8BPP\n");
		if (!set_bpp(fbfd, 8U, &fbink_state)) {
			fprintf(stderr, "Failed to swap bitdepth, aborting . . .\n");
			rv = ERRCODE(EXIT_FAILURE);
			goto cleanup;
		}

		// Re-init fbink so it registers the bitdepth switch
		fprintf(stdout, "[08] REINIT\n");
		fbink_reinit(fbfd, &fbink_cfg);

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
		fbink_cfg.halign = NONE;
		fbink_cfg.valign = NONE;
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
		fbink_wait_for_complete(fbfd, LAST_MARKER);

		// One more time, with a x2 scaling
		fbink_cfg.halign        = CENTER;
		fbink_cfg.valign        = CENTER;
		fbink_cfg.scaled_height = (short int) (dump.area.height * 2U);
		fbink_cfg.scaled_width  = (short int) (dump.area.width * 2U);
		fprintf(stdout, "[13] SCALED PRINT RAW\n");
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
		fbink_wait_for_complete(fbfd, LAST_MARKER);

		// Switch back to 32bpp
		fprintf(stdout, "[14] SWITCH TO 32BPP\n");
		if (!set_bpp(fbfd, 32U, &fbink_state)) {
			fprintf(stderr, "Failed to swap bitdepth, aborting . . .\n");
			rv = ERRCODE(EXIT_FAILURE);
			goto cleanup;
		}
	}

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
