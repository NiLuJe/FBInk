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

FBInkInputDevice*
    fbink_input_scan(INPUT_DEVICE_TYPE_T req_types, size_t* dev_count)
{
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

	for (int i = 0; i < ndev; i++) {
		FBInkInputDevice* dev = devices + i;
		dev->type             = INPUT_UNKNOWN;
		dev->fd               = -1;
		strcpy(dev->name, "???");
		snprintf(dev->path, sizeof(dev->path), "%s/%s", DEV_INPUT_EVENT, namelist[i]->d_name);

		// Default to NONBLOCK
		int o_flags = O_RDONLY | O_CLOEXEC;
		if ((req_types & OPEN_BLOCKING) == 0) {
			o_flags |= O_NONBLOCK;
		}
		dev->fd = open(dev->path, o_flags);
		if (dev->fd < 0) {
			PFWARN("open `%s`: %m", dev->path);
			continue;
		}

		ioctl(dev->fd, EVIOCGNAME(sizeof(dev->name)), dev->name);

		// TODO: Also close if !requested types
		if (req_types & SCAN_ONLY) {
			close(dev->fd);
			dev->fd = -1;
		}

		free(namelist[i]);
	}

	return devices;
}
