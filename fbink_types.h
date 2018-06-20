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

#ifndef __FBINK_TYPES_H
#define __FBINK_TYPES_H

#include <stdbool.h>

// default eInk framebuffer palette
// c.f., linux/drivers/video/mxc/cmap_lab126.h
typedef enum
{
	BLACK  = 0,     // 0x00
	GRAY1  = 1,     // 0x11
	GRAY2  = 2,     // 0x22
	GRAY3  = 3,     // 0x33
	GRAY4  = 4,     // 0x44
	GRAY5  = 5,     // 0x55
	GRAY6  = 6,     // 0x66
	GRAY7  = 7,     // 0x77
	GRAY8  = 8,     // 0x88
	GRAY9  = 9,     // 0x99
	GRAY10 = 10,    // 0xAA
	GRAY11 = 11,    // 0xBB
	GRAY12 = 12,    // 0xCC
	GRAY13 = 13,    // 0xDD
	GRAY14 = 14,    // 0xEE
	WHITE  = 15     // 0xFF
} COLOR_INDEX_T;

// List of flags for device or screen-specific quirks...
typedef struct
{
	bool isPerfectFit;
	bool isKoboMk7;
	bool isKindlePearlScreen;
	bool isKindleOasis2;
	bool skipId;
} FBInkDeviceQuirks;

#endif
