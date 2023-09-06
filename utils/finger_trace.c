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

// Quick'n dirty PoC to test the craziness that is input handling on Kobo.

// Because we're pretty much Linux-bound ;).
#ifndef _GNU_SOURCE
#	define _GNU_SOURCE
#endif

#include "../fbink.h"
#include <errno.h>
#include <fcntl.h>
#include <poll.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <linux/fb.h>

#include <libevdev/libevdev.h>

// Pilfer a few macros from fbink_internal.h...
#define LOG(fmt, ...) ({ fprintf(stdout, fmt "\n", ##__VA_ARGS__); })

#define ELOG(fmt, ...) ({ fprintf(stderr, "[FTrace] " fmt "\n", ##__VA_ARGS__); })

#define WARN(fmt, ...) ({ fprintf(stderr, "[FTrace] " fmt "!\n", ##__VA_ARGS__); })

#define PFWARN(fmt, ...) ({ WARN("[%s] " fmt, __PRETTY_FUNCTION__, ##__VA_ARGS__); })

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

// Having a static input device number all these years couldn't go on forever...
#define NXP_TOUCH_DEV             "/dev/input/event1"
#define SUNXI_ELAN_TOUCHPAD_EVDEV "/dev/input/by-path/platform-0-0010-event"
#define NXP_ELAN_TOUCHPAD_EVDEV   "/dev/input/by-path/platform-1-0010-event"
#define MTK_ELAN_TOUCHPAD_EVDEV   "/dev/input/by-path/platform-2-0010-event"

typedef struct
{
	int32_t x;
	int32_t y;
} FTrace_Coordinates;

// Keep tracks of a single slot's state...
typedef enum
{
	UNKNOWN_TOOL,
	FINGER,
	PEN,
} FTrace_Tool;

typedef enum
{
	UNKNOWN_STATE,
	DOWN,
	UP,
} FTrace_State;

typedef struct
{
	FTrace_Coordinates pos;
	FTrace_Tool        tool;
	FTrace_State       state;
	struct timeval     time;
} FTrace_Slot;

// Shove stuff in a struct to keep the signatures sane-ish
typedef struct
{
	FBInkConfig*     fbink_cfg;
	FBInkState*      fbink_state;
	struct libevdev* dev;
	FTrace_Slot*     touch;
	FTrace_Slot*     prev_touch;
	int              fbfd;
	bool             swap_axes;
	bool             mirror_x;
	bool             mirror_y;
} FTrace_Context;

static const char*
    tool_type_to_str(const FTrace_Tool tool)
{
	switch (tool) {
		case FINGER:
			return " FINGER ";
		case PEN:
			return " STYLUS ";
		case UNKNOWN_TOOL:
		default:
			return "UNKNOWN!";
	}
}

static void
    handle_contact(const FTrace_Context* ctx)
{
	const FBInkConfig* fbink_cfg   = ctx->fbink_cfg;
	const FBInkState*  fbink_state = ctx->fbink_state;
	const FTrace_Slot* touch       = ctx->touch;

	// We'll need a copy of our precomputed rotation + panel coordinates transforms...
	// (If it weren't for the Touch B quirk below, we wouldn't need a copy at all :/).
	bool       swap_axes = ctx->swap_axes;
	bool       mirror_x  = ctx->mirror_x;
	const bool mirror_y  = ctx->mirror_y;

	// The Touch B does something... weird.
	if (fbink_state->device_id == DEVICE_KOBO_TOUCH_B && touch->state == UP) {
		// The frame that reports a contact lift does the panel-specific coordinates transform for us...
		// That means we need to flip it back...
		swap_axes = !swap_axes;
		mirror_x  = !mirror_x;
	}

	// Apply the final transformations
	FTrace_Coordinates translated_pos;
	if (swap_axes) {
		translated_pos.x = touch->pos.y;
		translated_pos.y = touch->pos.x;
	} else {
		translated_pos.x = touch->pos.x;
		translated_pos.y = touch->pos.y;
	}
	if (mirror_x) {
		translated_pos.x = (int32_t) fbink_state->screen_width - 1 - translated_pos.x;
	}
	if (mirror_y) {
		translated_pos.y = (int32_t) fbink_state->screen_height - 1 - translated_pos.y;
	}

	// Recap the craziness...
	LOG("%ld.%.6ld [%s] [%s] @ (%4d, %4d) => (%4d, %4d)",
	    touch->time.tv_sec,
	    touch->time.tv_usec,
	    tool_type_to_str(touch->tool),
	    touch->state == DOWN ? "DOWN" : " UP ",
	    touch->pos.x,
	    touch->pos.y,
	    translated_pos.x,
	    translated_pos.y);

	// Display a font_mul x 2 sized rectangle around the contact point,
	// e.g., a poor man's pointer trail.
	const FBInkRect rect = { .left =
				     (unsigned short int) MAX(0, translated_pos.x - (int32_t) fbink_state->fontsize_mult),
				 .top =
				     (unsigned short int) MAX(0, translated_pos.y - (int32_t) fbink_state->fontsize_mult),
				 .width  = fbink_state->fontsize_mult * 2U,
				 .height = fbink_state->fontsize_mult * 2U };

	// Ignore FBInk warnings (we may hit temporary refresh failures...)
	fbink_cls(ctx->fbfd, fbink_cfg, &rect, false);
}

// Process an input event
static void
    process_evdev(const struct input_event* ev, const FTrace_Context* ctx)
{
	FBInkConfig*      fbink_cfg   = ctx->fbink_cfg;
	const FBInkState* fbink_state = ctx->fbink_state;
	FTrace_Slot*      touch       = ctx->touch;

	// NOTE: Shitty minimal state machinesque: we don't handle slots, gestures, or whatever ;).
	if (ev->type == EV_SYN && ev->code == SYN_REPORT) {
		FTrace_Slot* prev_touch = ctx->prev_touch;

		touch->time = ev->time;
		// We only do stuff on each REPORT,
		// iff the finger actually moved somewhat significantly...
		// NOTE: Should ideally be clamped to between 0 and the relevant screen dimension ;).
		if ((touch->pos.x > prev_touch->pos.x + (int32_t) fbink_state->fontsize_mult ||
		     touch->pos.x < prev_touch->pos.x - (int32_t) fbink_state->fontsize_mult) ||
		    (touch->pos.y > prev_touch->pos.y + (int32_t) fbink_state->fontsize_mult ||
		     touch->pos.y < prev_touch->pos.y - (int32_t) fbink_state->fontsize_mult)) {
			handle_contact(ctx);
			*prev_touch = *touch;
		} else {
			LOG("%ld.%.6ld [%s] [%s] @ Similar coordinates",
			    touch->time.tv_sec,
			    touch->time.tv_usec,
			    tool_type_to_str(touch->tool),
			    touch->state == DOWN ? "DOWN" : " UP ");
		}

		// We're done with this frame, back to the libevdev read loop!
		// (Which will likely return to poll, unless we're markedly late and the kernel queue has had time to fill up again.
		// But if all goes well, we should be polling only once per input frame).
		return;
	}

	// Detect tool type & all contacts up on Mk. 7 (and possibly earlier "snow" protocol devices).
	if (ev->type == EV_KEY) {
		switch (ev->code) {
			case BTN_TOOL_PEN:
				touch->tool = PEN;
				if (ev->value > 0) {
					touch->state = DOWN;
				} else {
					touch->state = UP;
				}
				break;
			case BTN_TOOL_FINGER:
				touch->tool = FINGER;
				if (ev->value > 0) {
					touch->state = DOWN;
				} else {
					touch->state = UP;
				}
				break;
			case BTN_TOUCH:
				// To detect up/down state on "snow" protocol without weird slot shenanigans...
				// It's out-of-band of MT events, so, it unfortunately means *all* contacts,
				// not a specific slot...
				// (i.e., you won't get an EV_KEY:BTN_TOUCH:0 until *all* contact points have been lifted).
				if (ev->value > 0) {
					touch->state = DOWN;
				} else {
					touch->state = UP;
				}
				break;
		}
	}

	if (ev->type == EV_ABS) {
		switch (ev->code) {
			case ABS_MT_TOOL_TYPE:
				// Detect tool type on Mk. 8
				if (ev->value == 0) {
					touch->tool = FINGER;
				} else if (ev->value == 1) {
					touch->tool = PEN;
				}
				break;
			// NOTE: That should cover everything...
			//       Mk. 6+ reports EV_KEY:BTN_TOUCH events,
			//       which would be easier to deal with,
			//       but redundant here ;).
			// NOTE: When in doubt about what a simple event stream looks like on specific devices,
			//       check generate_button_press @ fbink_button_scan.c ;).
			case ABS_PRESSURE:
			case ABS_MT_WIDTH_MAJOR:
			//case ABS_MT_TOUCH_MAJOR: // Oops, not that one, it's always 0 on early Mk.7 devices :s
			case ABS_MT_PRESSURE:
				if (ev->value > 0) {
					touch->state = DOWN;
				} else {
					// NOTE: Actually getting a 0-pressure event on lift is *not* a guarantee...
					//       (Especially on some Elan panels/drivers...)
					touch->state = UP;
				}
				break;
			case ABS_X:
			case ABS_MT_POSITION_X:
				touch->pos.x = ev->value;
				break;
			case ABS_Y:
			case ABS_MT_POSITION_Y:
				touch->pos.y = ev->value;
				break;
			case ABS_MT_TRACKING_ID:
				if (ev->value == -1) {
					touch->state = UP;

					if (fbink_state->is_sunxi) {
						// NOTE: On sunxi, send a !pen refresh on pen up because otherwise the driver softlocks,
						//       and ultimately trips a reboot watchdog...
						// NOTE: Nickel also toggles pen mode *off* before doing that...
						//       Let's do the same, as we can still somewhat reliably kill the kernel
						//       one way or another otherwise...
						// NOTE: That means that any other competing refresh is potentially dangerous:
						//       make sure only pen refreshes are sent while in pen mode!
						fbink_sunxi_toggle_ntx_pen_mode(ctx->fbfd, false);

						const WFM_MODE_INDEX_T pen_wfm = fbink_cfg->wfm_mode;
						fbink_cfg->wfm_mode            = WFM_GL16;
						fbink_refresh(ctx->fbfd, 0, 0, 0, 0, fbink_cfg);
						fbink_cfg->wfm_mode = pen_wfm;

						fbink_sunxi_toggle_ntx_pen_mode(ctx->fbfd, true);
					}
				}
				break;
			default:
				break;
		}
	}
}

// Parse an evdev event
static bool
    handle_evdev(const FTrace_Context* ctx)
{
	struct libevdev* dev = ctx->dev;

	int rc = 1;
	do {
		struct input_event ev;
		rc = libevdev_next_event(dev, LIBEVDEV_READ_FLAG_NORMAL, &ev);
		if (rc == LIBEVDEV_READ_STATUS_SYNC) {
			LOG(">>> DROPPED <<<");
			while (rc == LIBEVDEV_READ_STATUS_SYNC) {
				// NOTE: Since we don't actually handle slots & stuff,
				//       we can probably get away with just doing this...
				//       c.f., https://www.freedesktop.org/software/libevdev/doc/latest/syn_dropped.html
				process_evdev(&ev, ctx);
				rc = libevdev_next_event(dev, LIBEVDEV_READ_FLAG_SYNC, &ev);
			}
			LOG("<<< RE-SYNCED >>>");
		} else if (rc == LIBEVDEV_READ_STATUS_SUCCESS) {
			process_evdev(&ev, ctx);
		}
	} while (rc == LIBEVDEV_READ_STATUS_SYNC || rc == LIBEVDEV_READ_STATUS_SUCCESS);
	if (rc != LIBEVDEV_READ_STATUS_SUCCESS && rc != -EAGAIN) {
		PFWARN("Failed to handle input events: %s", strerror(-rc));
		return false;
	}

	// EAGAIN: we've drained the kernel queue, back to poll :)
	return true;
}

// Fun helper to make sure we disable pen mode on exit...
// For our cleanup signal handler....
volatile sig_atomic_t g_timeToDie = 0;
static void
    cleanup_handler(int        signum __attribute__((unused)),
		    siginfo_t* siginfo __attribute__((unused)),
		    void*      context __attribute__((unused)))
{
	// Our main loop handles EINTR, and will abort cleanly once it sees that flag
	g_timeToDie = 1;
}

int
    main(void)
{
	// Assume success, until shit happens ;)
	int rv = EXIT_SUCCESS;

	// Early declarations for error handling purposes
	FTrace_Context   ctx  = { 0 };
	struct libevdev* dev  = NULL;
	int              evfd = -1;

	// Setup FBInk
	FBInkConfig fbink_cfg  = { 0 };
	fbink_cfg.wfm_mode     = WFM_DU;
	fbink_cfg.bg_color     = BG_BLACK;
	FBInkState fbink_state = { 0 };

	// Init FBInk
	// Open framebuffer and keep it around, then setup globals.
	if ((ctx.fbfd = fbink_open()) == ERRCODE(EXIT_FAILURE)) {
		WARN("Failed to open the framebuffer, aborting");
		rv = ERRCODE(EXIT_FAILURE);
		goto cleanup;
	}
	if (fbink_init(ctx.fbfd, &fbink_cfg) != EXIT_SUCCESS) {
		WARN("Failed to initialize FBInk, aborting");
		rv = ERRCODE(EXIT_FAILURE);
		goto cleanup;
	}
	LOG("Initialized FBInk %s", fbink_version());
	ctx.fbink_cfg = &fbink_cfg;

	// We'll need the state for device identification...
	fbink_get_state(&fbink_cfg, &fbink_state);
	ctx.fbink_state = &fbink_state;

	// Dump the rotation map
	LOG("Rotation map: {%hhu, %hhu, %hhu, %hhu}",
	    fbink_state.rotation_map[FB_ROTATE_UR],
	    fbink_state.rotation_map[FB_ROTATE_CW],
	    fbink_state.rotation_map[FB_ROTATE_UD],
	    fbink_state.rotation_map[FB_ROTATE_CCW]);

	// Attempt not to murder the crappy sunxi driver, because as suspected,
	// it doesn't really deal well with refresh storms...
	// NOTE: We don't bracket the actual refresh themselves,
	//       because apparently this made things worse...
	// NOTE: This has a tendancy to kill the kernel regardless
	//       (e.g., the eink image proc kernel threads spinning in the wind hogging a core @ 100%,
	//       until the next non-pen update (i.e., not A2 or DU, even if the mode is still enabled)...
	//       Hence the GL16 update we send on pen up...
	// NOTE: Speaking of, having two of the four cores online help (because the eink threads will spin like mad),
	//       but more than that is probably overkill, unless you need them yourself.
	//       (Nickel tends to stick to three cores in the Notebooks).
	// NOTE: Fun fact: high disp debug levels appear to make everything worse, for some reason...
	if (fbink_state.is_sunxi) {
		fbink_sunxi_toggle_ntx_pen_mode(ctx.fbfd, true);
	}

	// This means we need a signal handler to make sure this gets reset on quit...
	struct sigaction new_action = { 0 };
	new_action.sa_sigaction     = &cleanup_handler;
	sigemptyset(&new_action.sa_mask);
	new_action.sa_flags = SA_SIGINFO;
	if ((rv = sigaction(SIGTERM, &new_action, NULL)) != 0) {
		PFWARN("sigaction (TERM): %m");
		rv = ERRCODE(EXIT_FAILURE);
		goto cleanup;
	}
	if ((rv = sigaction(SIGINT, &new_action, NULL)) != 0) {
		PFWARN("sigaction (INT): %m");
		rv = ERRCODE(EXIT_FAILURE);
		goto cleanup;
	}
	if ((rv = sigaction(SIGQUIT, &new_action, NULL)) != 0) {
		PFWARN("sigaction (QUIT): %m");
		rv = ERRCODE(EXIT_FAILURE);
		goto cleanup;
	}

	// On sunxi, clear the buffer to *white* first,
	// because we'll periodically trigger full-screen refreshes to placate the kernel,
	// and dmabuffs are zero-initialized, so our bg is currently *black*...
	if (fbink_state.is_sunxi) {
		const BG_COLOR_INDEX_T pen_color = fbink_cfg.bg_color;
		const WFM_MODE_INDEX_T pen_wfm   = fbink_cfg.wfm_mode;
		fbink_cfg.bg_color               = BG_WHITE;
		fbink_update_pen_colors(&fbink_cfg);
		fbink_cfg.wfm_mode = WFM_GL16;
		fbink_cls(ctx.fbfd, &fbink_cfg, NULL, false);
		fbink_cfg.wfm_mode = pen_wfm;
		fbink_cfg.bg_color = pen_color;
		fbink_update_pen_colors(&fbink_cfg);
	}

	// Setup libevdev, poking at the right input device...
	if (fbink_state.is_mtk) {
		evfd = open(MTK_ELAN_TOUCHPAD_EVDEV, O_RDONLY | O_CLOEXEC | O_NONBLOCK);
	} else if (fbink_state.is_sunxi) {
		evfd = open(SUNXI_ELAN_TOUCHPAD_EVDEV, O_RDONLY | O_CLOEXEC | O_NONBLOCK);
	} else {
		if (access(NXP_ELAN_TOUCHPAD_EVDEV, F_OK) == 0) {
			// e.g., the Libra 2
			evfd = open(NXP_ELAN_TOUCHPAD_EVDEV, O_RDONLY | O_CLOEXEC | O_NONBLOCK);
		} else {
			evfd = open(NXP_TOUCH_DEV, O_RDONLY | O_CLOEXEC | O_NONBLOCK);
		}
	}
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
	// We'l keep it grabbed, to prevent interaction with whatever may currently be running.
	//libevdev_grab(dev, LIBEVDEV_UNGRAB);
	LOG("Initialized libevdev for device `%s`", libevdev_get_name(dev));
	ctx.dev = dev;

	// We need to know the canonical rotation to deal with coordinates translation...
	// NOTE: We only check this and compute the matching swap/mirror flags on startup here,
	//       this would need to be updated on relevant fbink_reinit returns if we cared about runtime rotation handling ;).
	const uint8_t canonical_rota = fbink_rota_native_to_canonical(fbink_state.current_rota);
	LOG("Rotation: %hhu -> %hhu", fbink_state.current_rota, canonical_rota);

	// Grab the device-specific panel translation quirks: we end up with flags similar to what is used in KOReader,
	// c.f., https://github.com/koreader/koreader/blob/master/frontend/device/kobo/device.lua
	ctx.swap_axes = fbink_state.touch_swap_axes;
	ctx.mirror_x  = fbink_state.touch_mirror_x;
	ctx.mirror_y  = fbink_state.touch_mirror_y;

	// Then, we can handle standard coordinates translation given the current rotation.
	// We'll deal with this by flipping the swap/mirror flags,
	// which will allow us to handle everything at once when processing an input frame.
	// c.f., set_rotation @ https://github.com/rmkit-dev/rmkit/blob/master/src/rmkit/input/events.cpy
	switch (canonical_rota) {
		case FB_ROTATE_CW:
			ctx.swap_axes = !ctx.swap_axes;
			ctx.mirror_y  = !ctx.mirror_y;
			break;
		case FB_ROTATE_UD:
			ctx.mirror_x = !ctx.mirror_x;
			ctx.mirror_y = !ctx.mirror_y;
			break;
		case FB_ROTATE_CCW:
			ctx.swap_axes = !ctx.swap_axes;
			ctx.mirror_x  = !ctx.mirror_x;
			break;
		default:
			// NOP
			break;
	}

	// Main loop
	struct pollfd pfd = { 0 };
	pfd.fd            = evfd;
	pfd.events        = POLLIN;

	FTrace_Slot touch      = { 0 };
	ctx.touch              = &touch;
	FTrace_Slot prev_touch = { 0 };
	ctx.prev_touch         = &prev_touch;

	// Check if the device supports tool types...
	if (!libevdev_has_event_code(dev, EV_ABS, ABS_MT_TOOL_TYPE)) {
		// Assume finger ;).
		touch.tool = FINGER;
	}

	while (true) {
		// If we caught one of the signals we setup earlier, it's time to die ;).
		if (g_timeToDie != 0) {
			ELOG("Caught a cleanup signal, winding down . . .");
			goto cleanup;
		}

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
				if (!handle_evdev(&ctx)) {
					// Uh ho, something went wrong when reading input events...
					break;
				}
			}
		}
	}

	// Cleanup
cleanup:
	if (ctx.fbfd != -1) {
		if (fbink_state.is_sunxi) {
			fbink_sunxi_toggle_ntx_pen_mode(ctx.fbfd, false);
		}
	}

	if (fbink_close(ctx.fbfd) == ERRCODE(EXIT_FAILURE)) {
		WARN("Failed to close the framebuffer, aborting");
		rv = ERRCODE(EXIT_FAILURE);
	}

	libevdev_free(dev);
	if (evfd != -1) {
		close(evfd);
	}

	return rv;
}
