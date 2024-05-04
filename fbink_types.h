/*
	FBInk: FrameBuffer eInker, a library to print text & images to an eInk Linux framebuffer
	Copyright (C) 2018-2024 NiLuJe <ninuje@gmail.com>
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

#ifndef __FBINK_TYPES_H
#define __FBINK_TYPES_H

#include <stdbool.h>
#include <stdint.h>

#ifdef FBINK_WITH_OPENTYPE
// NOTE: We need to import stbtt early because we depend on stbtt_fontinfo here
//       We'll want it as static/private, so do that here, because we're importing it earlier than fbink.c
#	define STBTT_STATIC
#	include "stb/stb_truetype.h"
#endif

// List of flags for device or screen-specific quirks...
typedef struct
{
	unsigned short int  screenDPI;
	DEVICE_ID_T         deviceId;
	char                deviceName[32];
	char                deviceCodename[32];
	char                devicePlatform[32];
	bool                isPerfectFit;
	bool                isMTK;
	bool                isSunxi;
	bool                isKindleLegacy;
	bool                isKindlePearlScreen;
	bool                isKindleZelda;
	bool                isKindleRex;
	bool                isKoboNonMT;
	bool                isNTX16bLandscape;
	bool                isKoboMk7;
	int8_t              koboVertOffset;
	uint8_t             ntxBootRota;
	NTX_ROTA_INDEX_T    ntxRotaQuirk;
	uint8_t             rotationMap[4];    // Index is native, value is canonical
	bool                touchSwapAxes;
	bool                touchMirrorX;
	bool                touchMirrorY;
	bool                unreliableWaitFor;
	bool                canRotate;
	bool                canHWInvert;
	bool                hasEclipseWfm;
	bool                canWaitForSubmission;
	bool                hasColorPanel;
	FBINK_PXFMT_INDEX_T pixelFormat;
	bool isRGB;    // i.e., RGB565, RGB24, RGBA or RGB32. Allows us to simplify some internal checks for RGB order.
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
typedef struct
{
	uint32_t u24 : 24;
} __attribute__((packed)) uint24_t;

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

// And a super type that we can use for every target bitdepth we support
// (We need a single data type because of our function pointers shenanigans...)
typedef union
{
	uint32_t       p;
	FBInkPixelBGRA bgra;
	FBInkPixelRGBA rgba;
	uint16_t       rgb565;
	uint8_t        gray8;    // Will point to bgra.color.b
	struct
	{
		uint8_t hi;    // Will point to bgra.color.b
		uint8_t lo;    // Will point to bgra.color.g
	} gray4;
} FBInkPixel;

#ifdef FBINK_WITH_OPENTYPE
// Stores the information necessary to render a line of text
// using OpenType/TrueType fonts
typedef struct FBInkOTLine
{
	size_t startCharIndex;
	size_t endCharIndex;
	int    line_gap;
	bool   line_used;
	bool   has_a_break;
} FBInkOTLine;

typedef struct FBInkOTFonts
{
	stbtt_fontinfo* otRegular;
	stbtt_fontinfo* otItalic;
	stbtt_fontinfo* otBold;
	stbtt_fontinfo* otBoldItalic;
} FBInkOTFonts;

typedef enum
{
	CH_IGNORE = 0U,
	CH_REGULAR,
	CH_ITALIC,
	CH_BOLD,
	CH_BOLD_ITALIC,
	CH_MAX = 0xFFu,    // uint8_t
} __attribute__((packed)) CHARACTER_FONT_E;
typedef uint8_t           CHARACTER_FONT_T;
#endif    // FBINK_WITH_OPENTYPE

#ifdef FBINK_FOR_KOBO
typedef struct
{
	uint16_t bus;
	uint16_t address;
} FBInkI2CDev;

// Chuck all the sunxi mess in a single place...
typedef struct
{
	int                       disp_fd;
	int                       i2c_fd;
	FBInkI2CDev               i2c_dev;
	int                       ion_fd;
	size_t                    alloc_size;
	struct ion_fd_data        ion;
	struct disp_layer_config2 layer;
	uint32_t                  rota;
	SUNXI_FORCE_ROTA_INDEX_T  force_rota;
	bool                      has_fbdamage;
} FBInkKoboSunxi;
#endif    // FBINK_FOR_KOBO

#endif
