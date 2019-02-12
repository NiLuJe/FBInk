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

// Because we're pretty much Linux-bound ;).
#ifndef _GNU_SOURCE
#	define _GNU_SOURCE
#endif

#include <stdio.h>
#include <time.h>
// I feel dirty.
#include "../fbink.c"

int fbfd = -1;

static void
    get_fbinfo(void)
{
	// Get variable fb info
	if (ioctl(fbfd, FBIOGET_VSCREENINFO, &vInfo)) {
		perror("ioctl GET_V");
	}
	LOG("Variable fb info: %ux%u, %ubpp @ rotation: %u (%s)",
	    vInfo.xres,
	    vInfo.yres,
	    vInfo.bits_per_pixel,
	    vInfo.rotate,
	    fb_rotate_to_string(vInfo.rotate));
	// Get fixed fb information
	if (ioctl(fbfd, FBIOGET_FSCREENINFO, &fInfo)) {
		perror("ioctl GET_F");
	}
	LOG("Fixed fb info: ID is \"%s\", length of fb mem: %u bytes & line length: %u bytes",
	    fInfo.id,
	    fInfo.smem_len,
	    fInfo.line_length);
}

static void
    set_fbinfo(int rota)
{
	// Set variable fb info
	vInfo.rotate = (uint32_t) rota;
	LOG("Setting rotate to %u (%s)", vInfo.rotate, fb_rotate_to_string(vInfo.rotate));

	if (ioctl(fbfd, FBIOPUT_VSCREENINFO, &vInfo)) {
		perror("ioctl PUT_V");
	}

	LOG("Rotate is now %u (%s)", vInfo.rotate, fb_rotate_to_string(vInfo.rotate));
}

static void
    do_eet(int rota)
{
	const struct timespec zzz = { 0L, 250000000L };

	set_fbinfo(rota);
	nanosleep(&zzz, NULL);
	get_fbinfo();
	nanosleep(&zzz, NULL);
}

int
    main(void)
{
	// For the LOG macro
	g_isVerbose = true;

	// NOTE: We only need this for ioctl, hence O_NONBLOCK (as per open(2)).
	fbfd = open("/dev/fb0", O_RDONLY | O_NONBLOCK | O_CLOEXEC);
	if (!fbfd) {
		perror("open");
		return ERRCODE(EXIT_FAILURE);
	}

	// Print initial status
	get_fbinfo();

	// let's check how quirky it is...
	LOG("\nFB_ROTATE_UR to FB_ROTATE_CCW, +1 increments");
	for (int i = FB_ROTATE_UR; i <= FB_ROTATE_CCW; i++) {
		do_eet(i);
	}

	// Now we'll try to break it...
	LOG("\nFB_ROTATE_UR to FB_ROTATE_CCW, +2 increments");
	for (int i = FB_ROTATE_UR; i <= FB_ROTATE_CCW; i += 2) {
		do_eet(i);
	}
	LOG("\nFB_ROTATE_CW to FB_ROTATE_CCW, +2 increments");
	for (int i = FB_ROTATE_CW; i <= FB_ROTATE_CCW; i += 2) {
		do_eet(i);
	}

	// And let's try to fix it, now...
	// NOTE: That's an H2O & co quirk.
	LOG("\nFB_ROTATE_UR to FB_ROTATE_CCW, +2 increments, intermerdiary rota if ==");
	for (int i = FB_ROTATE_UR; i <= FB_ROTATE_CCW; i += 2) {
		// If current rotate = to be set value, set += 1 (wrapping at 4) first to swap portrait/landscape
		if (vInfo.rotate == (uint32_t) i) {
			LOG("Intermerdiary rotation...");
			do_eet((i + 1) % 4);
			LOG("Requested rotation");
			do_eet(i);
		} else {
			do_eet(i);
		}
	}
	LOG("\nFB_ROTATE_CW to FB_ROTATE_CCW, +2 increments, intermerdiary rota if ==");
	for (int i = FB_ROTATE_CW; i <= FB_ROTATE_CCW; i += 2) {
		// If current rotate = to be set value, set += 1 (wrapping at 4) first to swap portrait/landscape
		if (vInfo.rotate == (uint32_t) i) {
			LOG("Intermerdiary rotation...");
			do_eet((i + 1) % 4);
			LOG("Requested rotation");
			do_eet(i);
		} else {
			do_eet(i);
		}
	}

	close(fbfd);
}
