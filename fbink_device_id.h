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

#ifndef __FBINK_DEVICE_ID_H
#define __FBINK_DEVICE_ID_H

// Mainly to make IDEs happy
#include "fbink.h"
#include "fbink_internal.h"

#if defined(FBINK_FOR_KINDLE) || defined(FBINK_FOR_LEGACY)
#	define KINDLE_SERIAL_NO_LENGTH 16

static bool     is_kindle_device(uint32_t, FBInkDeviceQuirks*);
static bool     is_kindle_device_new(uint32_t, FBInkDeviceQuirks*);
static uint32_t from_base(char*, uint8_t);
static void     identify_kindle(FBInkDeviceQuirks*);
#else
static void identify_kobo(FBInkDeviceQuirks*);
#endif

static void identify_device(FBInkDeviceQuirks*);

#endif
