/*
	FBInk: FrameBuffer eInker, a tool to print text & images on eInk devices (Kobo/Kindle)
	Copyright (C) 2018-2019 NiLuJe <ninuje@gmail.com>

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

#ifdef FBINK_WITH_OPENTYPE
// NOTE: We need to import stbtt early because we depend on stbtt_fontinfo here
//       We'll want it as static/private, so do that here, because we're importing it earlier than fbink.c
#	define STBTT_STATIC
#	include "stb/stb_truetype.h"
#endif

// List of flags for device or screen-specific quirks...
typedef struct
{
	unsigned short int screenDPI;
	unsigned short int deviceId;
	char               deviceName[16];
	char               deviceCodename[16];
	char               devicePlatform[16];
	bool               isPerfectFit;
	bool               isKindleLegacy;
	bool               isKindlePearlScreen;
	bool               isKindleOasis2;
	bool               isKindleRex;
	bool               isKoboNonMT;
	bool               isNTX16bLandscape;
	bool               isKoboMk7;
	int8_t             koboVertOffset;
	uint8_t            ntxBootRota;
	uint8_t            ntxRotaQuirk;
	bool               canRotate;
	bool               canHWInvert;
	bool               skipId;
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

#ifdef FBINK_WITH_OPENTYPE
// Stores the information necessary to render a line of text
// using OpenType/TrueType fonts
typedef struct FBInkOTLine
{
	size_t startCharIndex;
	size_t endCharIndex;
	int    line_gap;
	bool   line_used;
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
	CH_BOLD_ITALIC
} CHARACTER_FONT_T;
#endif    // FBINK_WITH_OPENTYPE

#endif
