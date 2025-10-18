/*
	FBInk: FrameBuffer eInker, a library to print text & images to an eInk Linux framebuffer
	Copyright (C) 2024 NiLuJe <ninuje@gmail.com>
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

#include "fbink_input_scan.h"

// Heavily based on udev's udev-builtin-input_id.c,
// Copyright (C) 2009 Martin Pitt <martin.pitt@ubuntu.com>
// Portions Copyright (C) 2004 David Zeuthen, <david@fubar.dk>
// Copyright (C) 2011 Kay Sievers <kay@vrfy.org>
// Copyright (C) 2014 Carlos Garnacho <carlosg@gnome.org>
// Copyright (C) 2014 David Herrmann <dh.herrmann@gmail.com>
// c.f., https://cgit.freedesktop.org/systemd/systemd/tree/src/udev/udev-builtin-input_id.c
//
// As well as evemu's tools/find_event_devices.c,
// Copyright (C) 2013 Red Hat, Inc
// c.f., https://cgit.freedesktop.org/evemu/tree/tools/find_event_devices.c

#ifdef FBINK_WITH_INPUT
/* pointer devices */
static bool
    test_pointers(FBInkInputDevice* restrict dev,
		  const unsigned long* restrict bitmask_ev,
		  const unsigned long* restrict bitmask_abs,
		  const unsigned long* restrict bitmask_key,
		  const unsigned long* restrict bitmask_rel,
		  const unsigned long* restrict bitmask_props)
{
	// NOTE: bitmask_props may be NULL if EVIOCGPROP is unsupported!
	bool has_keys            = test_bit(EV_KEY, bitmask_ev);
	bool has_abs_coordinates = test_bit(ABS_X, bitmask_abs) && test_bit(ABS_Y, bitmask_abs);
	bool has_3d_coordinates  = has_abs_coordinates && test_bit(ABS_Z, bitmask_abs);
	bool is_accelerometer    = bitmask_props && test_bit(INPUT_PROP_ACCELEROMETER, bitmask_props);

	if (!has_keys && has_3d_coordinates) {
		is_accelerometer = true;
	}

	if (is_accelerometer) {
		dev->type |= INPUT_ACCELEROMETER;
		return true;
	}

	bool is_pointing_stick = bitmask_props && test_bit(INPUT_PROP_POINTING_STICK, bitmask_props);
	bool stylus_or_pen     = test_bit(BTN_STYLUS, bitmask_key) || test_bit(BTN_TOOL_PEN, bitmask_key);
	bool finger_but_no_pen = test_bit(BTN_TOOL_FINGER, bitmask_key) && !test_bit(BTN_TOOL_PEN, bitmask_key);
	bool has_mouse_button  = test_bit(BTN_LEFT, bitmask_key);
	bool has_rel_coordinates =
	    test_bit(EV_REL, bitmask_ev) && test_bit(REL_X, bitmask_rel) && test_bit(REL_Y, bitmask_rel);
	bool has_mt_coordinates = test_bit(ABS_MT_POSITION_X, bitmask_abs) && test_bit(ABS_MT_POSITION_Y, bitmask_abs);

	/* unset has_mt_coordinates if device claims to have all ABS axes */
	if (has_mt_coordinates && test_bit(ABS_MT_SLOT, bitmask_abs) && test_bit(ABS_MT_SLOT - 1, bitmask_abs)) {
		has_mt_coordinates = false;
	}
	bool is_direct = bitmask_props && test_bit(INPUT_PROP_DIRECT, bitmask_props);
	bool has_touch = test_bit(BTN_TOUCH, bitmask_key);
	/* joysticks don't necessarily have buttons;
	 * e. g. rudders/pedals are joystick-like, but buttonless;
	 * they have other fancy axes */
	bool has_joystick_axes_or_buttons =
	    test_bit(BTN_TRIGGER, bitmask_key) || test_bit(BTN_A, bitmask_key) || test_bit(BTN_1, bitmask_key) ||
	    test_bit(ABS_RX, bitmask_abs) || test_bit(ABS_RY, bitmask_abs) || test_bit(ABS_RZ, bitmask_abs) ||
	    test_bit(ABS_THROTTLE, bitmask_abs) || test_bit(ABS_RUDDER, bitmask_abs) ||
	    test_bit(ABS_WHEEL, bitmask_abs) || test_bit(ABS_GAS, bitmask_abs) || test_bit(ABS_BRAKE, bitmask_abs);

	bool is_tablet      = false;
	bool is_touchpad    = false;
	bool is_mouse       = false;
	bool is_touchscreen = false;
	bool is_joystick    = false;
	if (has_abs_coordinates) {
		if (stylus_or_pen) {
			is_tablet = true;
		} else if (finger_but_no_pen && (bitmask_props && !is_direct)) {
			// NOTE: The original udev logic assumes no prop support *also* means !is_direct.
			//       We've tweaked that a bit because we target a fair number of devices without EVIOCGPROP support,
			//       and we'd rather stuff be misidentified as a touchscreen rather than a touchpad.
			//       And, more importantly, that touchscreens that happen to report BTN_TOOL_FINGER,
			//       as Parade and zForce panels do (at least on lab126 kernels),
			//       be properly identified as touchscreens on devices w/o prop support.
			is_touchpad = true;
		} else if (has_mouse_button) {
			/* This path is taken by VMware's USB mouse,
			 * which has absolute axes, but no touch/pressure button. */
			is_mouse = true;
		} else if (has_touch) {
			is_touchscreen = true;
		} else if (has_joystick_axes_or_buttons) {
			is_joystick = true;
		}
	}
	if (has_mt_coordinates) {
		if (is_direct) {
			is_touchscreen = true;
		}
		// If we haven't got a match yet, try relaxed heuristics for old/broken drivers...
		if (!is_touchscreen && !is_tablet && !is_touchpad) {
			// c.f., /usr/src/linux/Documentation/input/event-codes.rst
			// for reference about legacy BTN_TOUCH expectations...
			if (has_touch) {
				is_touchscreen = true;
			} else if (!bitmask_props) {
				is_touchscreen = true;
			}
		}

		// NOTE: Qt's QDeviceDiscovery *unconditionally* assumes is_touchscreen if has_mt_coordinates.
		//       That might not be such a terrible idea...
	}

	if (has_rel_coordinates && has_mouse_button) {
		is_mouse = true;
	}

	if (is_pointing_stick) {
		dev->type |= INPUT_POINTINGSTICK;
	}
	if (is_mouse) {
		dev->type |= INPUT_MOUSE;
	}
	if (is_touchpad) {
		dev->type |= INPUT_TOUCHPAD;
	}
	if (is_touchscreen) {
		dev->type |= INPUT_TOUCHSCREEN;
	}
	if (is_joystick) {
		dev->type |= INPUT_JOYSTICK;
	}
	if (is_tablet) {
		dev->type |= INPUT_TABLET;
	}

	return is_tablet || is_mouse || is_touchpad || is_touchscreen || is_joystick || is_pointing_stick;
}

/* key like devices */
static bool
    test_key(FBInkInputDevice* restrict dev,
	     const unsigned long* restrict bitmask_ev,
	     const unsigned long* restrict bitmask_key)
{
	bool ret = false;

	/* do we have any KEY_* capability? */
	if (!test_bit(EV_KEY, bitmask_ev)) {
		PFLOG("[%s] no EV_KEY capability", dev->name);
		return false;
	}

	/* only consider KEY_* here, not BTN_* */
	unsigned long found = 0;
	for (unsigned i = 0; i < BTN_MISC / BITS_PER_LONG; ++i) {
		found |= bitmask_key[i];
		PFLOG("[%s] checking bit block %lu for any keys; found=%i",
		      dev->name,
		      (unsigned long) i * BITS_PER_LONG,
		      found > 0);
	}
	/* If there are no keys in the lower block, check the higher block */
	if (!found) {
		for (unsigned i = KEY_OK; i < BTN_TRIGGER_HAPPY; ++i) {
			if (test_bit(i, bitmask_key)) {
				PFLOG("[%s] Found key %x in high block", dev->name, i);
				found = 1;
				break;
			}
		}
	}

	if (found > 0) {
		dev->type |= INPUT_KEY;
		ret        = true;
	}

	/* the first 32 bits are ESC, numbers, and Q to D;
	 * if we have all of those, consider it a full keyboard;
	 * do not test KEY_RESERVED, though */
	// NOTE: This will *not* match on any of the "keyboard" Kindles!
	unsigned long mask = 0xFFFFFFFE;
	if ((bitmask_key[0] & mask) == mask) {
		dev->type |= INPUT_KEYBOARD;
		ret        = true;
	}

	return ret;
}

// Look for specific (and often platform specific) keys
static void
    test_platform_keys(FBInkInputDevice* restrict dev, const unsigned long* restrict bitmask_key)
{
	if (PLATFORM_KEY_POWER && test_bit(PLATFORM_KEY_POWER, bitmask_key)) {
		dev->type |= INPUT_POWER_BUTTON;
	}

	// Kobo has used two different keycodes for the SleepCover over the years...
	// Devices can support one or both of these.
	if (PLATFORM_KEY_SLEEP && test_bit(PLATFORM_KEY_SLEEP, bitmask_key)) {
		dev->type |= INPUT_SLEEP_COVER;
	}
	if (PLATFORM_KEY_WAKEUP && test_bit(PLATFORM_KEY_WAKEUP, bitmask_key)) {
		dev->type |= INPUT_SLEEP_COVER;
	}

#	ifdef FBINK_FOR_KINDLE
	// There are so many possible combinations, so just match on any *single* relevant key, àla DPAD...
	if (test_bit(PLATFORM_KEY_PGPREV, bitmask_key) || test_bit(PLATFORM_KEY_PGPREV_ALT, bitmask_key) ||
	    test_bit(PLATFORM_KEY_PGNEXT, bitmask_key) || test_bit(PLATFORM_KEY_PGNEXT_ALT, bitmask_key)) {
		dev->type |= INPUT_PAGINATION_BUTTONS;
	}
#	else
	if ((PLATFORM_KEY_PGPREV && test_bit(PLATFORM_KEY_PGPREV, bitmask_key)) &&
	    (PLATFORM_KEY_PGNEXT && test_bit(PLATFORM_KEY_PGNEXT, bitmask_key))) {
		dev->type |= INPUT_PAGINATION_BUTTONS;
	}
#	endif

	if (PLATFORM_KEY_HOME && test_bit(PLATFORM_KEY_HOME, bitmask_key)) {
		dev->type |= INPUT_HOME_BUTTON;
	}

	if (PLATFORM_KEY_FRONTLIGHT && test_bit(PLATFORM_KEY_FRONTLIGHT, bitmask_key)) {
		dev->type |= INPUT_LIGHT_BUTTON;
	}

	if (PLATFORM_KEY_MENU && test_bit(PLATFORM_KEY_MENU, bitmask_key)) {
		dev->type |= INPUT_MENU_BUTTON;
	}

	if (PLATFORM_KEY_DPAD && test_bit(PLATFORM_KEY_DPAD, bitmask_key)) {
		dev->type |= INPUT_DPAD;
	}
	if (test_bit(KEY_UP, bitmask_key)) {
		dev->type |= INPUT_DPAD;
	}

	if (test_bit(KEY_VOLUMEUP, bitmask_key) && test_bit(KEY_VOLUMEDOWN, bitmask_key)) {
		dev->type |= INPUT_VOLUME_BUTTONS;
	}

#	ifdef FBINK_FOR_KINDLE
	// Check for the fancy Kindle "gesture_tap" thingy, which only reports a couple of keys (F6, F7 & F8).
	// Since those keycodes are horrendously generic, we'll just resort to checking the name...
	// c.f., https://github.com/koreader/koreader/issues/14461#issuecomment-3408244649)
	if (dev->type == INPUT_KEY && test_bit(KEY_F7, bitmask_key)) {
		if (strcmp(dev->name, "gesture_tap") == 0) {
			dev->type |= INPUT_KINDLE_FRAME_TAP;
		}
	}
#	endif
}

static int
    check_device_cap(FBInkInputDevice* restrict dev)
{
	unsigned long  bitmask_ev[NBITS(EV_MAX)]            = { 0U };
	unsigned long  bitmask_abs[NBITS(ABS_MAX)]          = { 0U };
	unsigned long  bitmask_key[NBITS(KEY_MAX)]          = { 0U };
	unsigned long  bitmask_rel[NBITS(REL_MAX)]          = { 0U };
	unsigned long  bitmask_props[NBITS(INPUT_PROP_MAX)] = { 0U };
	unsigned long* bitmask_props_p                      = bitmask_props;
	unsigned long  bitmask_msc[NBITS(MSC_MAX)]          = { 0U };

	// These... really shouldn't ever fail.
	ioctl(dev->fd, EVIOCGBIT(0, sizeof(bitmask_ev)), bitmask_ev);
	if (test_bit(EV_ABS, bitmask_ev)) {
		ioctl(dev->fd, EVIOCGBIT(EV_ABS, sizeof(bitmask_abs)), bitmask_abs);
	}
	if (test_bit(EV_REL, bitmask_ev)) {
		ioctl(dev->fd, EVIOCGBIT(EV_REL, sizeof(bitmask_rel)), bitmask_rel);
	}
	if (test_bit(EV_KEY, bitmask_ev)) {
		ioctl(dev->fd, EVIOCGBIT(EV_KEY, sizeof(bitmask_key)), bitmask_key);
	}
	if (test_bit(EV_MSC, bitmask_ev)) {
		ioctl(dev->fd, EVIOCGBIT(EV_MSC, sizeof(bitmask_msc)), bitmask_msc);
	}
	// But this may not be supported on older kernels, warn about it.
	int rc = ioctl(dev->fd, EVIOCGPROP(sizeof(bitmask_props)), bitmask_props);
	if (rc < 0 && errno == EINVAL) {
		PFLOG(
		    "EVIOCGPROP is unsupported on this device (kernel is too old). Accelerometer & touchpad/touchscreen discrimination may be less accurate.");
		// Let test_pointers know that this is unsupported, so it can relax some of its heuristics...
		bitmask_props_p = NULL;
	}

	bool is_pointer = test_pointers(dev, bitmask_ev, bitmask_abs, bitmask_key, bitmask_rel, bitmask_props_p);
	bool is_key     = test_key(dev, bitmask_ev, bitmask_key);
	/* Some devices only have a scrollwheel */
	if (!is_pointer && !is_key && test_bit(EV_REL, bitmask_ev) &&
	    (test_bit(REL_WHEEL, bitmask_rel) || test_bit(REL_HWHEEL, bitmask_rel))) {
		dev->type |= INPUT_KEY;
	}

	// And then check for more fine-grained stuff on real is_key devices for our own use-cases
	if (is_key) {
		test_platform_keys(dev, bitmask_key);
	}
	// We also want to check where the "device was rotated" events end up...
	// ... potentially. There's a rather large potential for false-positives:
	// the Kobo variant is fairly generic (EV_MSC:MSC_RAW),
	// and the Kindle variant is *extremely* generic *and* nonsensical (EV_ABS:ABS_PRESSURE ?!)...
	switch (PLATFORM_ROTATION_EV_TYPE) {
		case EV_ABS:
			if (test_bit(PLATFORM_ROTATION_EV_CODE, bitmask_abs)) {
				dev->type |= INPUT_ROTATION_EVENT;
			}
			break;
		case EV_MSC:
			if (test_bit(PLATFORM_ROTATION_EV_CODE, bitmask_msc)) {
				dev->type |= INPUT_ROTATION_EVENT;
			}
			break;
		default:
			// NOP
			break;
	}
	// In order to identify properly scaled INPUT_TABLET devices,
	// we'll need to compare the max ABS_X/ABS_Y values with our framebuffer's resolution...
	if (dev->type & INPUT_TABLET) {
		// This is easy enough if we're running as part of FBInk proper, and we run post-init,
		// but we'll have to do some more work otherwise (e.g., in standalone input lib builds)...
		if (vInfo.xres == 0U && vInfo.yres == 0U) {
			// Open the framebuffer...
			int fbfd = open("/dev/fb0", O_RDONLY | O_NONBLOCK | O_CLOEXEC);
			if (fbfd == -1) {
				PFWARN("Cannot open framebuffer character device (%m)");
			} else {
				// Request the info we need
				if (ioctl(fbfd, FBIOGET_VSCREENINFO, &vInfo)) {
					PFWARN("Error reading variable fb information: %m");
					// Too bad, we'll go on anyway
				}
				// And we're done with that
				close(fbfd);
			}
		}

		// Okay, *now* we can check our stuff...
		struct input_absinfo abs_info = { 0U };
		ioctl(dev->fd, EVIOCGABS(ABS_X), &abs_info);
		uint32_t max_x = (uint32_t) abs_info.maximum;
		ioctl(dev->fd, EVIOCGABS(ABS_Y), &abs_info);
		uint32_t max_y = (uint32_t) abs_info.maximum;

		// Check both dimensions to account for rotation shenanigans...
		if ((max_x == vInfo.xres || max_x == vInfo.yres) && (max_y == vInfo.xres || max_y == vInfo.yres)) {
			dev->type |= INPUT_SCALED_TABLET;
		}
	}

	return EXIT_SUCCESS;
}

static __attribute__((cold)) const char*
    input_type_to_string(INPUT_DEVICE_TYPE_E type)
{
	switch (type) {
		case INPUT_UNKNOWN:
			return "UNKNOWN";
		case INPUT_POINTINGSTICK:
			return "POINTING_STICK";
		case INPUT_MOUSE:
			return "MOUSE";
		case INPUT_TOUCHPAD:
			return "TOUCHPAD";
		case INPUT_TOUCHSCREEN:
			return "TOUCHSCREEN";
		case INPUT_JOYSTICK:
			return "JOYSTICK";
		case INPUT_TABLET:
			return "TABLET";
		case INPUT_KEY:
			return "KEY";
		case INPUT_KEYBOARD:
			return "KEYBOARD";
		case INPUT_ACCELEROMETER:
			return "ACCELEROMETER";
		case INPUT_POWER_BUTTON:
			return "POWER_BUTTON";
		case INPUT_SLEEP_COVER:
			return "SLEEP_COVER";
		case INPUT_PAGINATION_BUTTONS:
			return "PAGINATION_BUTTONS";
		case INPUT_HOME_BUTTON:
			return "HOME_BUTTON";
		case INPUT_LIGHT_BUTTON:
			return "LIGHT_BUTTON";
		case INPUT_MENU_BUTTON:
			return "MENU_BUTTON";
		case INPUT_DPAD:
			return "DPAD";
		case INPUT_ROTATION_EVENT:
			return "ROTATION_EVENT";
		case INPUT_SCALED_TABLET:
			return "SCALED_TABLET";
		case INPUT_VOLUME_BUTTONS:
			return "VOLUME_BUTTONS";
		case INPUT_KINDLE_FRAME_TAP:
			return "KINDLE_FRAME_TAP";
		default:
			return NULL;
	}
}

static __attribute__((cold)) void
    concat_type_recap(INPUT_DEVICE_TYPE_T type, char* string, size_t dsize)
{
	char* p   = string;
	char* end = string + dsize;

	if (type == INPUT_UNKNOWN) {
		p = stpecpy(p, end, " = UNKNOWN");
		return;
	}

	bool first = true;
	// Since we can't actually iterate over an enum's values in C,
	// fake it by computing them at run-time, since we know it's a simple bitmask.
	// Basically, we just loop for the amount of bits in an INPUT_DEVICE_TYPE_T, i.e., 32 times,
	// computing the value of that specific bit on each iteration.
	for (INPUT_DEVICE_TYPE_T i = 0U, v = (1U << i); i < (sizeof(v) * 8U); i++, v = (1U << i)) {
		const char* type_name = input_type_to_string(v);
		if (!type_name) {
			// That bit doesn't actually map to a valid value in our enum, next!
			continue;
		}

		if (type & v) {
			if (first) {
				p     = stpecpy(p, end, " = ");
				first = false;
			} else {
				p = stpecpy(p, end, " | ");
			}
			p = stpecpy(p, end, type_name);
		}
	}
}

// We've got an open fd, check the device's caps and apply the match filters
static void
    check_device(FBInkInputDevice* restrict dev,
		 INPUT_DEVICE_TYPE_T match_types,
		 INPUT_DEVICE_TYPE_T exclude_types,
		 INPUT_DEVICE_TYPE_T settings)
{
	// Path was already set by the caller, and fd is open; now get the device's name
	ioctl(dev->fd, EVIOCGNAME(sizeof(dev->name)), dev->name);

	// Let udev's builtin input_id logic do its thing!
	check_device_cap(dev);

	// Recap the device's capabilities, unless requested otherwise.
	if (!(settings & NO_RECAP)) {
		char recap[4096] = { 0 };
		concat_type_recap(dev->type, recap, sizeof(recap));
		ELOG("%s: `%s`%s", dev->path, dev->name, recap);
	}

	// Do we want to match on *all* or *any* of the match bits?
	if (settings & MATCH_ALL) {
		dev->matched = !!((dev->type & match_types) == match_types);
	} else {
		dev->matched = !!(dev->type & match_types);
	}
	// Do we want to exclude on *all* or *any* of the exclude bits?
	if (dev->matched && exclude_types) {
		if (settings & EXCLUDE_ALL) {
			dev->matched = !((dev->type & exclude_types) == exclude_types);
		} else {
			dev->matched = !(dev->type & exclude_types);
		}
	}

	// If this was a dry-run, or if the device wasn't a match, close the fd
	if (settings & SCAN_ONLY || !dev->matched) {
		if (strcmp(dev->name, "zForce-ir-touch") == 0) {
			// NOTE: Some zForce panels have a weird race condition around Active/Deactivate commands,
			//       which can potentially lead to no reports being generated if the Active command fumbles.
			//       Since open/close do an active/deactivate, sleep 50ms on each side to try to tame the beast...
			//       A better bet is to avoid using SCAN_ONLY and keep the fd around if you actually need it.
			const struct timespec zzz = { 0L, 50 * 1e+6 };
			nanosleep(&zzz, NULL);
			close(dev->fd);
			nanosleep(&zzz, NULL);
		} else {
			close(dev->fd);
		}
		dev->fd = -1;
	}
}
#endif    // FBINK_WITH_INPUT

// Scan /dev/input/event* for input devices, and classify/filter them
FBInkInputDevice*
    fbink_input_scan(INPUT_DEVICE_TYPE_T match_types   UNUSED_BY_NOINPUT,
		     INPUT_DEVICE_TYPE_T exclude_types UNUSED_BY_NOINPUT,
		     INPUT_DEVICE_TYPE_T settings      UNUSED_BY_NOINPUT,
		     size_t* dev_count                 UNUSED_BY_NOINPUT)
{
#ifdef FBINK_WITH_INPUT
	struct dirent** namelist = NULL;
	int             ndev     = scandir(DEV_INPUT_EVENT, &namelist, is_event_device, sort_fn);
	if (ndev <= 0) {
		PFWARN("scandir: %m");
		*dev_count = 0U;
		return NULL;
	}

	*dev_count                = (size_t) ndev;
	FBInkInputDevice* devices = calloc((size_t) ndev, sizeof(*devices));
	if (!devices) {
		PFWARN("calloc: %m");
		*dev_count = 0U;
		for (int i = 0; i < ndev; i++) {
			free(namelist[i]);
		}
		free(namelist);
		return NULL;
	}

	// Default to NONBLOCK
	int o_flags = O_RDONLY | O_CLOEXEC;
	if ((settings & OPEN_BLOCKING) == 0) {
		o_flags |= O_NONBLOCK;
	}

	for (int i = 0; i < ndev; i++) {
		FBInkInputDevice* dev = devices + i;
		dev->fd               = -1;
		snprintf(dev->path, sizeof(dev->path), "%s/%s", DEV_INPUT_EVENT, namelist[i]->d_name);

		dev->fd = open(dev->path, o_flags);
		if (dev->fd < 0) {
			PFWARN("open `%s`: %m", dev->path);
			free(namelist[i]);
			continue;
		}
#	ifdef FBINK_FOR_LEGACY
		// Enforce CLOEXEC the hard way on legacy devices with kernels too old to support it via open flags.
		int fdflags = fcntl(dev->fd, F_GETFD);
		fcntl(dev->fd, F_SETFD, fdflags | FD_CLOEXEC);
#	endif

		check_device(dev, match_types, exclude_types, settings);

		free(namelist[i]);
	}
	free(namelist);

	return devices;
#else
	WARN("Input utilities are disabled in this FBInk build");
	return NULL;
#endif    // FBINK_WITH_INPUT
}

// Variant that takes a path instead of scanning /dev/input/event*
FBInkInputDevice*
    fbink_input_check(const char* filepath              UNUSED_BY_NOINPUT,
		      INPUT_DEVICE_TYPE_T match_types   UNUSED_BY_NOINPUT,
		      INPUT_DEVICE_TYPE_T exclude_types UNUSED_BY_NOINPUT,
		      INPUT_DEVICE_TYPE_T settings      UNUSED_BY_NOINPUT)
{
#ifdef FBINK_WITH_INPUT
	// Default to NONBLOCK
	int o_flags = O_RDONLY | O_CLOEXEC;
	if ((settings & OPEN_BLOCKING) == 0) {
		o_flags |= O_NONBLOCK;
	}

	int fd = open(filepath, o_flags);
	if (fd < 0) {
		PFWARN("open `%s`: %m", filepath);
		return NULL;
	}

	FBInkInputDevice* dev = calloc(1, sizeof(*dev));
	if (!dev) {
		PFWARN("calloc: %m");
		close(fd);
		return NULL;
	}

	dev->fd = fd;
	strtcpy(dev->path, filepath, sizeof(dev->path));
	check_device(dev, match_types, exclude_types, settings);

	return dev;
#else
	WARN("Input utilities are disabled in this FBInk build");
	return NULL;
#endif    // FBINK_WITH_INPUT
}
