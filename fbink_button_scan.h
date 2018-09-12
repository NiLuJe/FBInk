/*
	FBInk: FrameBuffer eInker, a tool to print text & images on eInk devices (Kobo/Kindle)
	Copyright (C) 2018 NiLuJe <ninuje@gmail.com>

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

#ifndef __FBINK_BUTTON_SCAN_H
#define __FBINK_BUTTON_SCAN_H

// Mainly to make IDEs happy
#include "fbink.h"
#include "fbink_internal.h"

#ifdef FBINK_WITH_BUTTON_SCAN
#	include <linux/input.h>
#	include <time.h>
#	include <mntent.h>
#	include <poll.h>

// NOTE: My TC's kernel is too old for some newer event codes...
#	ifndef ABS_MT_DISTANCE
#		define ABS_MT_DISTANCE 0x3b
#	endif

// c.f., https://github.com/koreader/koreader-base/pull/468/files
#	define SEND_INPUT_EVENT(t, c, v)                                                                                \
		({                                                                                                       \
			gettimeofday(&ev.time, NULL);                                                                    \
			ev.type  = (t);                                                                                  \
			ev.code  = (c);                                                                                  \
			ev.value = (v);                                                                                  \
			write(ifd, &ev, sizeof(ev));                                                                     \
		})

static bool is_onboard_mounted(void);
static bool wait_for_onboard(void);
static bool wait_for_background_color(uint8_t, unsigned short int);
static bool is_on_connected_screen(void);
static bool is_on_home_screen(void);
static bool is_on_import_screen(void);
static int  generate_button_press(FBInkCoordinates*, bool);
#endif

#endif
