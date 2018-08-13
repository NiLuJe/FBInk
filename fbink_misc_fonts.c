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

#include "fbink_misc_fonts.h"

static const unsigned char*
    kates_get_bitmap(uint32_t codepoint)
{
	if (codepoint <= 0x02) {
		return kates_block1[codepoint];
	} else if (codepoint >= 0x09 && codepoint <= 0x19) {
		return kates_block2[codepoint - 0x09];
	} else if (codepoint >= 0x1b && codepoint <= 0x1b) {
		return kates_block3[codepoint - 0x1b];
	} else if (codepoint >= 0x20 && codepoint <= 0x7e) {
		return kates_block4[codepoint - 0x20];
	} else if (codepoint >= 0xa1 && codepoint <= 0xac) {
		return kates_block5[codepoint - 0xa1];
	} else if (codepoint >= 0xae && codepoint <= 0xff) {
		return kates_block6[codepoint - 0xae];
	} else {
		fprintf(stderr, "[FBInk] Codepoint U+%04X is not covered by this font!\n", codepoint);
		return kates_block1[0];
	}
}

static const unsigned char*
    fkp_get_bitmap(uint32_t codepoint)
{
	if (codepoint <= 0xff) {
		return fkp_block1[codepoint];
	} else {
		fprintf(stderr, "[FBInk] Codepoint U+%04X is not covered by this font!\n", codepoint);
		return fkp_block1[0];
	}
}
