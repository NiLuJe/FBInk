/*
	FBInk: FrameBuffer eInker, a tool to print strings on eInk devices (Kobo/Kindle)
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

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>

#define KINDLE_SERIAL_NO_LENGTH 16

// List of flags for device or screen-specific quirks...
typedef struct
{
	bool isPerfectFit;
	bool isKoboMk7;
	bool isKindleTouch;
	bool isKindleOasis2;
} FBInkDeviceQuirks;

void identify_kobo(FBInkDeviceQuirks*);

bool     is_kindle_device(uint32_t);
bool     is_kindle_touch(uint32_t);
bool     is_kindle_device_v2(uint32_t);
bool     is_kindle_oasis2(uint32_t);
uint32_t from_base(char*, unsigned short int);
void     identify_kindle(FBInkDeviceQuirks*);

void identify_device(FBInkDeviceQuirks*);

#endif
