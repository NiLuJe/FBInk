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

#ifndef __FBINK_TYPES_H
#define __FBINK_TYPES_H

#include <stdbool.h>
#include <stdint.h>

// default eInk framebuffer palette
// c.f., linux/drivers/video/mxc/cmap_lab126.h
// NOTE: Legacy devices have an inverted color map!
typedef enum
{
	BLACK  = 0x00,
	GRAY1  = 0x11,
	GRAY2  = 0x22,
	GRAY3  = 0x33,
	GRAY4  = 0x44,
	GRAY5  = 0x55,
	GRAY6  = 0x66,
	GRAY7  = 0x77,
	GRAY8  = 0x88,
	GRAY9  = 0x99,
	GRAY10 = 0xAA,
	GRAY11 = 0xBB,
	GRAY12 = 0xCC,
	GRAY13 = 0xDD,
	GRAY14 = 0xEE,
	WHITE  = 0xFF
} FBInkGrayRamp;

// List of flags for device or screen-specific quirks...
typedef struct
{
	bool isPerfectFit;
	bool isKindleLegacy;
	bool isKindlePearlScreen;
	bool isKindleOasis2;
	bool isKoboNonMT;
	bool isKobo16Landscape;
	bool isKoboMk7;
	bool skipId;
} FBInkDeviceQuirks;

// An (x, y) coordinates tuple
typedef struct
{
	unsigned short int x;
	unsigned short int y;
} FBInkCoordinates;

// A color, as an (r, g, b) triplet for an 8-bit per component, 3 channel color
// NOTE: For grayscale, r = g = b (= v), so we assume v is r for simplicity's sake.
typedef struct
{
	uint8_t r;
	uint8_t g;
	uint8_t b;
} FBInkColor;

// Magical uni(c)on(rn)s to speed up working with true color, packed 32bpp pixels,
// so we can do a single get/set per pixel, instead of one per component...
typedef union
{
	uint32_t p;
	struct
	{
		uint8_t r;
		uint8_t g;
		uint8_t b;
		uint8_t a;
	} color;
} FBInkPixelRGBA;

// Because of course, images are stored RGB, but the framebuffer is BGR...
typedef union
{
	uint32_t p;
	struct
	{
		uint8_t b;
		uint8_t g;
		uint8_t r;
		uint8_t a;
	} color;
} FBInkPixelBGRA;

// Used to speedup alpha-blending of Grayscale images
typedef union
{
	uint16_t p;
	struct
	{
		uint8_t v;
		uint8_t a;
	} color;
} FBInkPixelG8A;

// And here be fucking dragons... Crazy-ass trickery for 24bits handling...
// NOTE: The packed attribute ensures that sizeof == 3 for this :)
struct _uint24_t
{
	uint32_t u24 : 24;
} __attribute__((packed));

// ... But we can't pack a typedef, so do it like this...
typedef struct _uint24_t uint24_t;

// So we can have this, with 3 bytes on both sides of the union :).
typedef union
{
	uint24_t p;
	struct
	{
		uint8_t r;
		uint8_t g;
		uint8_t b;
	} color;
} FBInkPixelRGB;

// And its BGR brother for 24bpp fbs...
typedef union
{
	uint24_t p;
	struct
	{
		uint8_t b;
		uint8_t g;
		uint8_t r;
	} color;
} FBInkPixelBGR;

#endif
