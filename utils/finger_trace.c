/*
	FBInk: FrameBuffer eInker, a library to print text & images to an eInk Linux framebuffer
	Copyright (C) 2018-2021 NiLuJe <ninuje@gmail.com>
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

// Quick'n dirty PoC to test the craziness that is input handling on Kobo.

// Because we're pretty much Linux-bound ;).
#ifndef _GNU_SOURCE
#	define _GNU_SOURCE
#endif

#include "../fbink.h"
#include <errno.h>
#include <fcntl.h>
#include <poll.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <libevdev/libevdev.h>

// Pilfer logging macros from fbink_internal.h...
#define LOG(fmt, ...) ({ fprintf(stdout, fmt "\n", ##__VA_ARGS__); })

#define ELOG(fmt, ...) ({ fprintf(stderr, "[FTrace] " fmt "\n", ##__VA_ARGS__); })

#define WARN(fmt, ...) ({ fprintf(stderr, "[FTrace] " fmt "!\n", ##__VA_ARGS__); })

#define PFWARN(fmt, ...) ({ WARN("[%s] " fmt, __PRETTY_FUNCTION__, ##__VA_ARGS__); })

// We want to return negative values on failure, always
#define ERRCODE(e) (-(e))

// Having a static input device number all these years couldn't go on forever...
#define NXP_TOUCH_DEV   "/dev/input/event1"
#define SUNXI_TOUCH_DEV "/dev/input/event2"

// Keep track of a single slot's state...
// FIXME: We don't actually discriminate between finger & pen ;p.
typedef enum
{
	UNKNOWN,
	FINGER_DOWN,
	FINGER_UP,
	PEN_DOWN,
	PEN_UP,
} FTrace_State;

typedef struct
{
	int32_t        x;
	int32_t        y;
	FTrace_State   state;
	struct timeval time;
} FTrace_Slot;

// Parse an evdev event
static bool
    handle_evdev(struct libevdev* dev, FTrace_Slot* touch)
{
	int rc = 1;
	do {
		struct input_event ev;
		rc = libevdev_next_event(dev, LIBEVDEV_READ_FLAG_NORMAL, &ev);
		if (rc == LIBEVDEV_READ_STATUS_SYNC) {
			while (rc == LIBEVDEV_READ_STATUS_SYNC) {
				// NOTE: We're ignoring the sync delta events here.
				rc = libevdev_next_event(dev, LIBEVDEV_READ_FLAG_SYNC, &ev);
			}
		} else if (rc == LIBEVDEV_READ_STATUS_SUCCESS) {
			// NOTE: Shitty minimal state machinesque: we don't handle slots, gestures, or whatever ;).
			if (ev.type == EV_SYN && ev.code == SYN_REPORT) {
				touch->time = ev.time;
				return true;
			}

			if (ev.type == EV_ABS) {
				switch (ev.code) {
					// NOTE: That should cover everything...
					//       Mk. 6+ reports EV_KEY:BTN_TOUCH events,
					//       which would be easier to deal with,
					//       but redundant here ;).
					case ABS_MT_TOUCH_MAJOR:
					case ABS_MT_WIDTH_MAJOR:
					case ABS_MT_PRESSURE:
					case ABS_PRESSURE:
						if (ev.value > 0) {
							touch->state = FINGER_DOWN;
						} else {
							touch->state = FINGER_UP;
						}
						break;
					case ABS_MT_POSITION_X:
					case ABS_X:
						touch->x = ev.value;
						break;
					case ABS_MT_POSITION_Y:
					case ABS_Y:
						touch->y = ev.value;
						break;
					default:
						break;
				}
			}
		}
	} while (rc == LIBEVDEV_READ_STATUS_SYNC || rc == LIBEVDEV_READ_STATUS_SUCCESS);
	if (rc != LIBEVDEV_READ_STATUS_SUCCESS && rc != -EAGAIN) {
		PFWARN("Failed to handle input events: %s", strerror(-rc));
	}

	return false;
}

int
    main(void)
{
	// Assume success, until shit happens ;)
	int rv = EXIT_SUCCESS;

	// Early declarations for error handling purposes
	int              fbfd = -1;
	struct libevdev* dev  = NULL;
	int              evfd = -1;

	// Setup FBInk
	FBInkConfig fbink_cfg = { 0 };
	fbink_cfg.wfm_mode    = WFM_DU;

	// Init FBInk
	// Open framebuffer and keep it around, then setup globals.
	if ((fbfd = fbink_open()) == ERRCODE(EXIT_FAILURE)) {
		WARN("Failed to open the framebuffer, aborting");
		rv = ERRCODE(EXIT_FAILURE);
		goto cleanup;
	}
	if (fbink_init(fbfd, &fbink_cfg) != EXIT_SUCCESS) {
		WARN("Failed to initialize FBInk, aborting");
		rv = ERRCODE(EXIT_FAILURE);
		goto cleanup;
	}
	LOG("Initialized FBInk %s", fbink_version());

	// We'll need the state to pick the right input device...
	FBInkState fbink_state = { 0 };
	fbink_get_state(&fbink_cfg, &fbink_state);

	// Setup libevdev
	evfd = open(fbink_state.is_sunxi ? SUNXI_TOUCH_DEV : NXP_TOUCH_DEV, O_RDONLY | O_CLOEXEC | O_NONBLOCK);
	if (evfd == -1) {
		PFWARN("open: %m");
		rv = ERRCODE(EXIT_FAILURE);
		goto cleanup;
	}

	dev    = libevdev_new();
	int rc = libevdev_set_fd(dev, evfd);
	if (rc < 0) {
		WARN("Failed to initialize libevdev (%s)", strerror(-rc));
		rv = ERRCODE(EXIT_FAILURE);
		goto cleanup;
	}
	// Check that nothing else has grabbed the input device, because that would prevent us from using it...
	if (libevdev_grab(dev, LIBEVDEV_GRAB) != 0) {
		WARN("Cannot read input events because the input device is currently grabbed by something else!");
		rv = ERRCODE(EXIT_FAILURE);
		goto cleanup;
	}
	// And we ourselves don't need to grab it, so, don't ;).
	libevdev_grab(dev, LIBEVDEV_UNGRAB);
	LOG("Initialized libevdev for device `%s`", libevdev_get_name(dev));

	// Main loop
	struct pollfd pfd = { 0 };
	pfd.fd            = evfd;
	pfd.events        = POLLIN;

	FTrace_Slot touch = { 0 };
	while (true) {
		int poll_num = poll(&pfd, 1, -1);

		if (poll_num == -1) {
			if (errno == EINTR) {
				continue;
			}
			PFWARN("poll: %m");
			rv = ERRCODE(EXIT_FAILURE);
			goto cleanup;
		}

		if (poll_num > 0) {
			if (pfd.revents & POLLIN) {
				if (handle_evdev(dev, &touch)) {
					LOG("%ld.%.9ld @ (%d, %d)",
					    touch.time.tv_sec,
					    touch.time.tv_usec,
					    touch.x,
					    touch.y);

					// TODO: Translate & display font_mul rect
				}
			}
		}
	}

	// Cleanup
cleanup:
	if (fbink_close(fbfd) == ERRCODE(EXIT_FAILURE)) {
		WARN("Failed to close the framebuffer, aborting");
		rv = ERRCODE(EXIT_FAILURE);
	}

	libevdev_free(dev);
	if (evfd != -1) {
		close(evfd);
	}

	return rv;
}
