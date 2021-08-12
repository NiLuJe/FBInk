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

#ifndef __FBINK_ROTA_QUIRKS_H
#define __FBINK_ROTA_QUIRKS_H

// Mainly to make IDEs happy
#include "fbink.h"
#include "fbink_internal.h"

#ifdef FBINK_FOR_KOBO
#	include <dirent.h>
#	include <sys/types.h>

#	include <linux/i2c-dev.h>
#	include <linux/i2c.h>

#	include <sys/ioctl.h>

// We need i2c-dev ;)
#	include <i2c/smbus.h>
// We need the KX122 register constants :).
#	include "eink/kx122-kobo.h"

static int                               find_accelerometer(const char* driver);
static int                               close_accelerometer_i2c(void);
static int                               populate_accelerometer_i2c_info(void);
static int                               open_accelerometer_i2c(void);
static __attribute__((cold)) const char* gyro_state_to_string(int state);
static int                               translate_kx122(uint16_t val);
static int                               query_kx122(void);
static int                               query_accelerometer(void);
static int                               query_fbdamage(void);

// Custom constants for accelerometer translations
#	define GYRO_STATE_UNKNOWN             -1
#	define GYRO_STATE_FACE_UP             -2
#	define GYRO_STATE_FACE_DOWN           -3
#	define GYRO_STATE_OUTSIDE_CONSTRAINTS -4

#endif    // FBINK_FOR_KOBO

#endif
