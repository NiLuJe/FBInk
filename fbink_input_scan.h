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

#ifndef __FBINK_INPUT_SCAN_H
#define __FBINK_INPUT_SCAN_H

// Mainly to make IDEs happy
#include "fbink.h"
#include "fbink_internal.h"

#include <dirent.h>
#include <fcntl.h>
#include <limits.h>
#include <linux/input.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#ifdef __GLIBC__
// versionsort sorts the device names in proper numerical order
// (e.g. so that event10 is listed after event9, not event1),
// but is only available in glibc.
static int (*sort_fn)(const struct dirent**, const struct dirent**) = versionsort;
#else
// Fall back to basic alphabetical sorting for other libc implementations.
static int (*sort_fn)(const struct dirent**, const struct dirent**) = alphasort;
#endif

#define DEV_INPUT_EVENT "/dev/input"
#define EVENT_DEV_NAME  "event"

static int
    is_event_device(const struct dirent* dir)
{
	return strncmp(EVENT_DEV_NAME, dir->d_name, 5) == 0;
}

// This needs to be kernel-compatible
#define BITS_PER_LONG        (sizeof(unsigned long) * 8)
#define NBITS(x)             ((((x) - 1) / BITS_PER_LONG) + 1)
#define OFF(x)               ((x) % BITS_PER_LONG)
#define BIT(x)               (1UL << OFF(x))
#define LONG(x)              ((x) / BITS_PER_LONG)
#define test_bit(bit, array) ((array[LONG(bit)] >> OFF(bit)) & 1)

static bool test_pointers(FBInkInputDevice*    dev,
			  const unsigned long* bitmask_ev,
			  const unsigned long* bitmask_abs,
			  const unsigned long* bitmask_key,
			  const unsigned long* bitmask_rel,
			  const unsigned long* bitmask_props);
static bool test_key(FBInkInputDevice* dev, const unsigned long* bitmask_ev, const unsigned long* bitmask_key);
static void test_platform_keys(FBInkInputDevice* dev, const unsigned long* bitmask_key);
static int  check_device_cap(FBInkInputDevice* dev);

// Old kernels need a hand...
#ifndef EVIOCGPROP
#	define EVIOCGPROP(len) _IOC(_IOC_READ, 'E', 0x09, len)
#endif
#ifndef INPUT_PROP_DIRECT
#	define INPUT_PROP_DIRECT 0x01
#endif
#ifndef INPUT_PROP_POINTING_STICK
#	define INPUT_PROP_POINTING_STICK 0x05
#endif
#ifndef INPUT_PROP_ACCELEROMETER
#	define INPUT_PROP_ACCELEROMETER 0x06
#endif
#ifndef INPUT_PROP_MAX
#	define INPUT_PROP_MAX 0x1f
#endif
#ifndef ABS_MT_SLOT
#	define ABS_MT_SLOT 0x2f
#endif
#ifndef BTN_TRIGGER_HAPPY
#	define BTN_TRIGGER_HAPPY 0x2c0
#endif

// Each of our target platforms tend to settle on some specific,
// sometimes barely related, keycodes for common functions...
#if defined(FBINK_FOR_KOBO)
#	define PLATFORM_KEY_POWER      KEY_POWER
#	define PLATFORM_KEY_SLEEP      KEY_H     // PowerCover, since the Elipsa
#	define PLATFORM_KEY_WAKEUP     KEY_F1    // PowerCover, before the Elipsa
#	define PLATFORM_KEY_PGPREV     KEY_F23
#	define PLATFORM_KEY_PGNEXT     KEY_F24
#	define PLATFORM_KEY_HOME       KEY_HOME
#	define PLATFORM_KEY_FRONTLIGHT KEY_KATAKANA
#	define PLATFORM_KEY_MENU       0
#elif defined(FBINK_FOR_KINDLE)
#	define PLATFORM_KEY_POWER      KEY_POWER    // Generally handled by powerd
#	define PLATFORM_KEY_SLEEP      0            // :?
#	define PLATFORM_KEY_WAKEUP     0            // :?
#	define PLATFORM_KEY_PGPREV     KEY_PAGEUP
#	define PLATFORM_KEY_PGNEXT     KEY_PAGEDOWN
#	define PLATFORM_KEY_HOME       KEY_HOME
#	define PLATFORM_KEY_FRONTLIGHT 0
#	define PLATFORM_KEY_MENU       KEY_MENU
#else
#	define PLATFORM_KEY_POWER      KEY_POWER
#	define PLATFORM_KEY_SLEEP      KEY_SLEEP
#	define PLATFORM_KEY_WAKEUP     KEY_WAKEUP
#	define PLATFORM_KEY_PGPREV     KEY_BACK
#	define PLATFORM_KEY_PGNEXT     KEY_FORWARD
#	define PLATFORM_KEY_HOME       KEY_HOMEPAGE
#	define PLATFORM_KEY_FRONTLIGHT KEY_BRIGHTNESS_CYCLE
#	define PLATFORM_KEY_MENU       KEY_MENU
#endif

#endif
