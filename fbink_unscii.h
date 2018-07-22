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

#ifndef __FBINK_UNSCII_H
#define __FBINK_UNSCII_H

#include <stdint.h>
#include <stdio.h>

#include "fonts/alt.h"
#include "fonts/fantasy.h"
#include "fonts/mcr.h"
#include "fonts/tall.h"
#include "fonts/thin.h"
#include "fonts/unscii.h"

// NOTE: Should technically be pure, but we can get away with const, according to https://lwn.net/Articles/285332/
const unsigned char* unscii_get_bitmap(uint32_t) __attribute__((const));
const unsigned char* alt_get_bitmap(uint32_t) __attribute__((const));
const unsigned char* thin_get_bitmap(uint32_t) __attribute__((const));
const unsigned char* fantasy_get_bitmap(uint32_t) __attribute__((const));
const unsigned char* mcr_get_bitmap(uint32_t) __attribute__((const));
const unsigned char* tall_get_bitmap(uint32_t) __attribute__((const));

#endif
