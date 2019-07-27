/*
	FBInk: FrameBuffer eInker, a tool to print text & images on eInk devices (Kobo/Kindle)
	Copyright (C) 2018-2019 NiLuJe <ninuje@gmail.com>

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

#ifndef __FBINK_SCIENTIFICA_H
#define __FBINK_SCIENTIFICA_H

// Mainly to make IDEs happy
#include "fbink.h"
#include "fbink_internal.h"

#include "fonts/scientifica.h"
#include "fonts/scientificab.h"
#include "fonts/scientificai.h"

// NOTE: Should technically be pure, but we can get away with const, according to https://lwn.net/Articles/285332/
static const unsigned char* scientifica_get_bitmap(uint32_t codepoint) __attribute__((const));
static const unsigned char* scientificab_get_bitmap(uint32_t codepoint) __attribute__((const));
static const unsigned char* scientificai_get_bitmap(uint32_t codepoint) __attribute__((const));

#endif
