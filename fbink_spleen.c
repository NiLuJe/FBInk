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

#include "fbink_spleen.h"

static const uint16_t*
    spleen_get_bitmap(uint32_t codepoint)
{
	if (codepoint >= 0x20u && codepoint <= 0x7fu) {
		return spleen_block1[codepoint - 0x20u];
	} else if (codepoint >= 0xa0u && codepoint <= 0x17fu) {
		return spleen_block2[codepoint - 0xa0u];
	} else if (codepoint >= 0x2018u && codepoint <= 0x2019u) {
		return spleen_block3[codepoint - 0x2018u];
	} else if (codepoint >= 0x201cu && codepoint <= 0x201du) {
		return spleen_block4[codepoint - 0x201cu];
	} else if (codepoint == 0x2022u) {
		return spleen_block5[0];
	} else if (codepoint == 0x2026u) {
		return spleen_block6[0];
	} else if (codepoint == 0x20acu) {
		return spleen_block7[0];
	} else if (codepoint >= 0x2500u && codepoint <= 0x259fu) {
		return spleen_block8[codepoint - 0x2500u];
	} else if (codepoint == 0x25c6u) {
		return spleen_block9[0];
	} else if (codepoint == 0x25cau) {
		return spleen_block10[0];
	} else if (codepoint >= 0x2630u && codepoint <= 0x2637u) {
		return spleen_block11[codepoint - 0x2630u];
	} else if (codepoint >= 0x2665u && codepoint <= 0x2666u) {
		return spleen_block12[codepoint - 0x2665u];
	} else if (codepoint >= 0x2800u && codepoint <= 0x28ffu) {
		return spleen_block13[codepoint - 0x2800u];
	} else if (codepoint >= 0xe0a0u && codepoint <= 0xe0a2u) {
		return spleen_block14[codepoint - 0xe0a0u];
	} else if (codepoint >= 0xe0b0u && codepoint <= 0xe0b3u) {
		return spleen_block15[codepoint - 0xe0b0u];
	} else {
		WARN("Codepoint U+%04X (%s) is not covered by this font", codepoint, u8_cp_to_utf8(codepoint));
		return spleen_block1[0];
	}
}
