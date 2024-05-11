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
    test_pointers(FBInkInputDevice*    dev,
		  const unsigned long* bitmask_ev,
		  const unsigned long* bitmask_abs,
		  const unsigned long* bitmask_key,
		  const unsigned long* bitmask_rel,
		  const unsigned long* bitmask_props)
{
	bool has_keys            = test_bit(EV_KEY, bitmask_ev);
	bool has_abs_coordinates = test_bit(ABS_X, bitmask_abs) && test_bit(ABS_Y, bitmask_abs);
	bool has_3d_coordinates  = has_abs_coordinates && test_bit(ABS_Z, bitmask_abs);
	bool is_accelerometer    = test_bit(INPUT_PROP_ACCELEROMETER, bitmask_props);

	if (!has_keys && has_3d_coordinates) {
		is_accelerometer = true;
	}

	if (is_accelerometer) {
		dev->type |= INPUT_ACCELEROMETER;
		return true;
	}

	bool is_pointing_stick = test_bit(INPUT_PROP_POINTING_STICK, bitmask_props);
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
	bool is_direct = test_bit(INPUT_PROP_DIRECT, bitmask_props);
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
		} else if (finger_but_no_pen && !is_direct) {
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
	if (has_mt_coordinates && is_direct) {
		is_touchscreen = true;
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
    test_key(FBInkInputDevice* dev, const unsigned long* bitmask_ev, const unsigned long* bitmask_key)
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
	unsigned long mask = 0xFFFFFFFE;
	if ((bitmask_key[0] & mask) == mask) {
		dev->type |= INPUT_KEYBOARD;
		ret        = true;
	}

	return ret;
}

// Look for specific (and often platform specific) keys
static void
    test_platform_keys(FBInkInputDevice* dev, const unsigned long* bitmask_key)
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

	if ((PLATFORM_KEY_PGPREV && test_bit(PLATFORM_KEY_PGPREV, bitmask_key)) &&
	    (PLATFORM_KEY_PGNEXT && test_bit(PLATFORM_KEY_PGNEXT, bitmask_key))) {
		dev->type |= INPUT_PAGINATION_BUTTONS;
	}

	if (PLATFORM_KEY_HOME && test_bit(PLATFORM_KEY_HOME, bitmask_key)) {
		dev->type |= INPUT_HOME_BUTTON;
	}

	if (PLATFORM_KEY_FRONTLIGHT && test_bit(PLATFORM_KEY_FRONTLIGHT, bitmask_key)) {
		dev->type |= INPUT_LIGHT_BUTTON;
	}

	if (PLATFORM_KEY_MENU && test_bit(PLATFORM_KEY_MENU, bitmask_key)) {
		dev->type |= INPUT_MENU_BUTTON;
	}
}

static int
    check_device_cap(FBInkInputDevice* dev)
{
	unsigned long bitmask_ev[NBITS(EV_MAX)]            = { 0U };
	unsigned long bitmask_abs[NBITS(ABS_MAX)]          = { 0U };
	unsigned long bitmask_key[NBITS(KEY_MAX)]          = { 0U };
	unsigned long bitmask_rel[NBITS(REL_MAX)]          = { 0U };
	unsigned long bitmask_props[NBITS(INPUT_PROP_MAX)] = { 0U };

	// These... really shouldn't ever fail.
	ioctl(dev->fd, EVIOCGBIT(0, sizeof(bitmask_ev)), bitmask_ev);
	ioctl(dev->fd, EVIOCGBIT(EV_ABS, sizeof(bitmask_abs)), bitmask_abs);
	ioctl(dev->fd, EVIOCGBIT(EV_REL, sizeof(bitmask_rel)), bitmask_rel);
	ioctl(dev->fd, EVIOCGBIT(EV_KEY, sizeof(bitmask_key)), bitmask_key);
	// But this may not be supported on older kernels, warn about it.
	int rc = ioctl(dev->fd, EVIOCGPROP(sizeof(bitmask_props)), bitmask_props);
	if (rc < 0 && errno == EINVAL) {
		PFLOG(
		    "EVIOCGPROP is unsupported on this device (kernel is too old). Accelerometer & touchpad/touchscreen discrimination may be less accurate.");
	}

	bool is_pointer = test_pointers(dev, bitmask_ev, bitmask_abs, bitmask_key, bitmask_rel, bitmask_props);
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

	return EXIT_SUCCESS;
}

static __attribute__((cold)) void
    input_type_to_str(INPUT_DEVICE_TYPE_E type, char* string)
{
	bool first = true;
	// Udev
	if (type == INPUT_UNKNOWN) {
		strcat(string, " = UNKNOWN");
	}
	if (type & INPUT_POINTINGSTICK) {
		if (first) {
			strcat(string, " = POINTING_STICK");
			first = false;
		} else {
			strcat(string, " | POINTING_STICK");
		}
	}
	if (type & INPUT_MOUSE) {
		if (first) {
			strcat(string, " = MOUSE");
			first = false;
		} else {
			strcat(string, " | MOUSE");
		}
	}
	if (type & INPUT_TOUCHPAD) {
		if (first) {
			strcat(string, " = TOUCHPAD");
			first = false;
		} else {
			strcat(string, " | TOUCHPAD");
		}
	}
	if (type & INPUT_TOUCHSCREEN) {
		if (first) {
			strcat(string, " = TOUCHSCREEN");
			first = false;
		} else {
			strcat(string, " | TOUCHSCREEN");
		}
	}
	if (type & INPUT_JOYSTICK) {
		if (first) {
			strcat(string, " = JOYSTICK");
			first = false;
		} else {
			strcat(string, " | JOYSTICK");
		}
	}
	if (type & INPUT_TABLET) {
		if (first) {
			strcat(string, " = TABLET");
			first = false;
		} else {
			strcat(string, " | TABLET");
		}
	}
	if (type & INPUT_KEY) {
		if (first) {
			strcat(string, " = KEY");
			first = false;
		} else {
			strcat(string, " | KEY");
		}
	}
	if (type & INPUT_KEYBOARD) {
		if (first) {
			strcat(string, " = KEYBOARD");
			first = false;
		} else {
			strcat(string, " | KEYBOARD");
		}
	}
	if (type & INPUT_ACCELEROMETER) {
		if (first) {
			strcat(string, " = ACCELEROMETER");
			first = false;
		} else {
			strcat(string, " | ACCELEROMETER");
		}
	}
	// Platform-specific
	if (type & INPUT_POWER_BUTTON) {
		if (first) {
			strcat(string, " = POWER_BUTTON");
			first = false;
		} else {
			strcat(string, " | POWER_BUTTON");
		}
	}
	if (type & INPUT_SLEEP_COVER) {
		if (first) {
			strcat(string, " = SLEEP_COVER");
			first = false;
		} else {
			strcat(string, " | SLEEP_COVER");
		}
	}
	if (type & INPUT_PAGINATION_BUTTONS) {
		if (first) {
			strcat(string, " = PAGINATION_BUTTONS");
			first = false;
		} else {
			strcat(string, " | PAGINATION_BUTTONS");
		}
	}
	if (type & INPUT_HOME_BUTTON) {
		if (first) {
			strcat(string, " = HOME_BUTTON");
			first = false;
		} else {
			strcat(string, " | HOME_BUTTON");
		}
	}
	if (type & INPUT_LIGHT_BUTTON) {
		if (first) {
			strcat(string, " = LIGHT_BUTTON");
			first = false;
		} else {
			strcat(string, " | LIGHT_BUTTON");
		}
	}
	if (type & INPUT_MENU_BUTTON) {
		if (first) {
			strcat(string, " = MENU_BUTTON");
			first = false;
		} else {
			strcat(string, " | MENU_BUTTON");
		}
	}
}
#endif    // FBINK_WITH_INPUT

FBInkInputDevice*
    fbink_input_scan(INPUT_DEVICE_TYPE_T req_types UNUSED_BY_NOINPUT, size_t* dev_count UNUSED_BY_NOINPUT)
{
#ifdef FBINK_WITH_INPUT
	struct dirent** namelist;
	int             ndev = scandir(DEV_INPUT_EVENT, &namelist, is_event_device, sort_fn);
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
		return NULL;
	}

	// Default to NONBLOCK
	int o_flags = O_RDONLY | O_CLOEXEC;
	if ((req_types & OPEN_BLOCKING) == 0) {
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

		ioctl(dev->fd, EVIOCGNAME(sizeof(dev->name)), dev->name);

		// Let udev's builtin input_id logic do its thing!
		check_device_cap(dev);

		// Recap the device's capabilities
		char recap[4096] = { 0 };
		input_type_to_str(dev->type, recap);
		ELOG("%s: `%s`%s", dev->path, dev->name, recap);

		// If the classification matches our request, flag it as such
		dev->matched = !!(dev->type & req_types);

		// If this was a dry-run, or if the device wasn't a match, close the fd
		if (req_types & SCAN_ONLY || !dev->matched) {
			close(dev->fd);
			dev->fd = -1;
		}

		free(namelist[i]);
	}

	return devices;
#else
	WARN("Input utilities are disabled in this FBInk build");
	return NULL;
#endif    // FBINK_WITH_INPUT
}
