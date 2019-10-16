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

// Because we're pretty much Linux-bound ;).
#ifndef _GNU_SOURCE
#	define _GNU_SOURCE
#endif

#include <stdio.h>
// I feel dirty.
#include "../fbink.c"

unsigned char *altPtr = NULL;

// NOTE: We go with the legacy ioctls for simplicty's sake.
static int
    refresh_kobo_alt(int                     fbfd,
		 const struct mxcfb_rect region,
		 const struct mxcfb_rect alt_region,
		 uint32_t                waveform_mode,
		 uint32_t                update_mode,
		 bool                    is_nightmode,
		 uint32_t                marker)
{
	// NOTE: I'm not quite sure which region controls what...
	//       What's for sure is that passing an empty region will timeout!
	struct mxcfb_alt_buffer_data_ntx alt = {
		.virt_addr = NULL,
		.phys_addr = *altPtr,
		.width = vInfo.xres_virtual,
		.height = vInfo.yres,
		.alt_update_region = alt_region,
	};

	struct mxcfb_update_data_v1_ntx update = {
		.update_region = region,
		.waveform_mode = waveform_mode,
		.update_mode   = update_mode,
		.update_marker = marker,
		.temp          = TEMP_USE_AMBIENT,
		.flags         = (waveform_mode == WAVEFORM_MODE_REAGLD)
			     ? EPDC_FLAG_USE_AAD
			     : (waveform_mode == WAVEFORM_MODE_A2) ? EPDC_FLAG_FORCE_MONOCHROME : 0U,
		.alt_buffer_data = alt,
	};

	if (is_nightmode && deviceQuirks.canHWInvert) {
		update.flags |= EPDC_FLAG_ENABLE_INVERSION;
	}

	// And, of course, use the alt buffer ;).
	update.flags |= EPDC_FLAG_USE_ALT_BUFFER;

	int rv;
	rv = ioctl(fbfd, MXCFB_SEND_UPDATE_V1_NTX, &update);

	if (rv < 0) {
		WARN("MXCFB_SEND_UPDATE_V1_NTX w/ alt buffer: %m");
		if (errno == EINVAL) {
			WARN("update_region={top=%u, left=%u, width=%u, height=%u}",
			     region.top,
			     region.left,
			     region.width,
			     region.height);
			WARN("alt_update_region={top=%u, left=%u, width=%u, height=%u}",
			     alt_region.top,
			     alt_region.left,
			     alt_region.width,
			     alt_region.height);
		}
		return ERRCODE(EXIT_FAILURE);
	}

	return EXIT_SUCCESS;
}

// Tweak the region to cover the full screen
static void
    empty_region(struct mxcfb_rect* restrict region)
{
	region->top    = 0U;
	region->left   = 0U;
	region->width  = 0U;
	region->height = 0U;
}


int
    main(void)
{
	// Assume success, until shit happens ;)
	int rv = EXIT_SUCCESS;

	// NOTE: We're going to need a full init
	FBInkConfig fbink_cfg = { 0 };
	fbink_cfg.is_verbose  = true;

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

	// We're of course going to need to mmap the fb
	if (!isFbMapped) {
		if (memmap_fb(fbfd) != EXIT_SUCCESS) {
			rv = ERRCODE(EXIT_FAILURE);
			goto cleanup;
		}
	}

	// NOTE: We mmap for *full* fb (smem_len), so we don't need another mapping for the alt buffer,
	//       since it has to be *inside* the fb.
	// NOTE: That does imply we need scratch space for a shadow buffer, which means 32bpp is out.
	//       Since no-one in their right mind should be using RGB565, that leaves 8bpp ;).
	if (vInfo.bits_per_pixel != 8U) {
		fprintf(stderr, "Not running @ 8bpp, not enough memory for an overlay buffer . . .\n");
		rv = ERRCODE(EXIT_FAILURE);
		goto cleanup;
	}

	// We will need an absolute pointer to the start of said buffer, which we'll compute the expected way,
	// much like panning and double-buffering is supposed to work in LinuxFB (I imagine).
	// NOTE: We know that yoffset is always 0, which is why we never bothered with it for fbPtr.
	//       So our buffer simply starts at yres_virt * line_length!
	//       It also ought to be exactly halfway through smem_len ;).
	altPtr = fbPtr + (vInfo.yres_virtual * fInfo.line_length);
	// Print raw pointer values...
	fprintf(stdout, "fbPtr: %p vs. altPtr: %p (Bounds: %#zx to %#zx)\n", fbPtr, altPtr, (size_t) fInfo.smem_start, (size_t) (fInfo.smem_start + fInfo.smem_len));

	// We start with something simple:
	// Paint the front buffer white
	memset(fbPtr, eInkBGCMap[0], (size_t)(fInfo.line_length * vInfo.yres));

	// Paint the overlay buffer black
	memset(altPtr, eInkFGCMap[0], (size_t)(fInfo.line_length * vInfo.yres));

	// Refresh w/ the front buffer
	struct mxcfb_rect region = { 0U };
	fullscreen_region(&region);
	refresh_kobo(fbfd, region, get_wfm_mode(WFM_GC16), UPDATE_MODE_FULL, false, 42);
	fbink_wait_for_complete(fbfd, 42);

	// refresh w/ the overlay buffer
	struct mxcfb_rect alt_region = { 0U };
	//empty_region(&region);
	fullscreen_region(&alt_region);
	refresh_kobo_alt(fbfd, region, alt_region, get_wfm_mode(WFM_GC16), UPDATE_MODE_FULL, false, 42);
	fbink_wait_for_complete(fbfd, 42);



cleanup:
	if (fbink_close(fbfd) == ERRCODE(EXIT_FAILURE)) {
		fprintf(stderr, "Failed to close the framebuffer, aborting . . .\n");
		rv = ERRCODE(EXIT_FAILURE);
	}

	return rv;
}
