/*
	FBInk: FrameBuffer eInker, a library to print text & images to an eInk Linux framebuffer
	Copyright (C) 2018-2020 NiLuJe <ninuje@gmail.com>
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

#ifndef __FBINK_ORP_H
#define __FBINK_ORP_H

// Mainly to make IDEs happy
#include "fbink.h"
#include "fbink_internal.h"

#include "fonts/orp.h"
#include "fonts/orpb.h"
#include "fonts/orpi.h"

// NOTE: Should technically be pure, but we can get away with const, according to https://lwn.net/Articles/285332/
static const unsigned char* orp_get_bitmap(uint32_t codepoint) __attribute__((const));
static const unsigned char* orpb_get_bitmap(uint32_t codepoint) __attribute__((const));
static const unsigned char* orpi_get_bitmap(uint32_t codepoint) __attribute__((const));

#endif
