/*
	FBInk: FrameBuffer eInker, a tool to print text & images on eInk devices (Kobo/Kindle)
	Copyright (C) 2018-2019 NiLuJe <ninuje@gmail.com>

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

// NOTE: Fairly useless piece of code basically just there to test the dump/restore functionality ;).
//       We could arguably plug into stb_image_write to basically reimplement fbgrab...

// Because we're pretty much Linux-bound ;).
#ifndef _GNU_SOURCE
#	define _GNU_SOURCE
#endif

#include "../fbink.h"
#include <stdio.h>
#include <stdlib.h>

// We want to return negative values on failure, always
#define ERRCODE(e) (-(e))

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
	if (ERRCODE(EXIT_FAILURE) == (fbfd = fbink_open())) {
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
	if (fbink_dump(fbfd, &dump) != ERRCODE(EXIT_SUCCESS)) {
		fprintf(stderr, "Failed to dump fb, aborting . . .\n");
		rv = ERRCODE(EXIT_FAILURE);
		goto cleanup;
	}

	// Print random crap
	fbink_cfg.is_centered = true;
	fbink_cfg.is_padded   = true;
	fbink_cfg.is_halfway  = true;
	if (fbink_print(fbfd, "Wheeee!", &fbink_cfg) < 0) {
		fprintf(stderr, "Failed to print!\n");
		rv = ERRCODE(EXIT_FAILURE);
		goto cleanup;
	}

	// Restore
	if (fbink_restore(fbfd, &fbink_cfg, &dump) != ERRCODE(EXIT_SUCCESS)) {
		fprintf(stderr, "Failed to restore fb, aborting . . .\n");
		rv = ERRCODE(EXIT_FAILURE);
		goto cleanup;
	}

	// Cleanup
cleanup:
	// Free potential dump data...
	free(dump.data);

	if (fbink_close(fbfd) == ERRCODE(EXIT_FAILURE)) {
		fprintf(stderr, "Failed to close the framebuffer, aborting . . .\n");
		rv = ERRCODE(EXIT_FAILURE);
	}

	return rv;
}
