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

#endif
