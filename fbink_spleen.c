/*
	FBInk: FrameBuffer eInker, a tool to print text & images on eInk devices (Kobo/Kindle)
	Copyright (C) 2018-2019 NiLuJe <ninuje@gmail.com>
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

#include "fbink_spleen.h"

static const uint16_t*
    spleen_get_bitmap(uint32_t codepoint)
{
	if (codepoint >= 0x20 && codepoint <= 0x7f) {
		return spleen_block1[codepoint - 0x20];
	} else if (codepoint >= 0xa0 && codepoint <= 0x17f) {
		return spleen_block2[codepoint - 0xa0];
	} else if (codepoint == 0x20ac) {
		return spleen_block3[0];
	} else if (codepoint >= 0x2500 && codepoint <= 0x257f) {
		return spleen_block4[codepoint - 0x2500];
	} else if (codepoint >= 0x2800 && codepoint <= 0x28ff) {
		return spleen_block5[codepoint - 0x2800];
	} else if (codepoint >= 0xe0b0 && codepoint <= 0xe0b3) {
		return spleen_block6[codepoint - 0xe0b0];
	} else {
		WARN("Codepoint U+%04X is not covered by this font", codepoint);
		return spleen_block1[0];
	}
}
