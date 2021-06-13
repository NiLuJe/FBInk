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

#ifndef __FBINK_BUTTON_SCAN_H
#define __FBINK_BUTTON_SCAN_H

// Mainly to make IDEs happy
#include "fbink.h"
#include "fbink_internal.h"

#ifdef FBINK_WITH_BUTTON_SCAN
#	include <linux/input.h>
#	include <mntent.h>
#	include <poll.h>
#	include <time.h>

// NOTE: My TC's kernel may be too old for some newer event codes...
//       c.f., /usr/include/linux/input-event-codes.h
#	ifndef SYN_MT_REPORT
#		define SYN_MT_REPORT 2
#	endif
#	ifndef ABS_MT_TOUCH_MAJOR
#		define ABS_MT_TOUCH_MAJOR 0x30
#	endif
#	ifndef ABS_MT_TOUCH_MINOR
#		define ABS_MT_TOUCH_MINOR 0x31
#	endif
#	ifndef ABS_MT_WIDTH_MAJOR
#		define ABS_MT_WIDTH_MAJOR 0x32
#	endif
#	ifndef ABS_MT_ORIENTATION
#		define ABS_MT_ORIENTATION 0x34
#	endif
#	ifndef ABS_MT_POSITION_X
#		define ABS_MT_POSITION_X 0x35
#	endif
#	ifndef ABS_MT_POSITION_Y
#		define ABS_MT_POSITION_Y 0x36
#	endif
#	ifndef ABS_MT_TRACKING_ID
#		define ABS_MT_TRACKING_ID 0x39
#	endif
#	ifndef ABS_MT_PRESSURE
#		define ABS_MT_PRESSURE 0x3a
#	endif
#	ifndef ABS_MT_DISTANCE
#		define ABS_MT_DISTANCE 0x3b
#	endif

// c.f., https://github.com/koreader/koreader-base/pull/468/files
// Also, attempt to deal with 64-bit time keeping on recent 32-bit systems, c.f., #63
#	if (__BITS_PER_LONG != 32 || !defined(__USE_TIME_BITS64))
#		define SEND_INPUT_EVENT(t, c, v)                                                                        \
			({                                                                                               \
				gettimeofday(&ev.time, NULL);                                                            \
				ev.type  = (t);                                                                          \
				ev.code  = (c);                                                                          \
				ev.value = (v);                                                                          \
				write(ifd, &ev, sizeof(ev));                                                             \
			})
#	else
#		define SEND_INPUT_EVENT(t, c, v)                                                                        \
			({                                                                                               \
				struct timeval now;                                                                      \
				gettimeofday(&now, NULL);                                                                \
				ev.input_event_sec  = now.tv_sec;                                                        \
				ev.input_event_usec = now.tv_usec;                                                       \
				ev.type             = (t);                                                               \
				ev.code             = (c);                                                               \
				ev.value            = (v);                                                               \
				write(ifd, &ev, sizeof(ev));                                                             \
			})
#	endif

static bool is_onboard_state(bool);
static bool wait_for_onboard_state(bool);
static bool wait_for_background_color(uint8_t, unsigned short int, unsigned short int);
static bool is_on_connected_screen(void);
static bool is_on_home_screen(void);
static bool is_on_import_screen(void);
static int  generate_button_press(FBInkCoordinates* restrict, bool);
#endif

#endif
