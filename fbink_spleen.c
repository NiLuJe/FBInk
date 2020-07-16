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
	} else if (codepoint >= 0x1cdu && codepoint <= 0x1d4u) {
		return spleen_block3[codepoint - 0x1cdu];
	} else if (codepoint >= 0x1e6u && codepoint <= 0x1e9u) {
		return spleen_block4[codepoint - 0x1e6u];
	} else if (codepoint == 0x1f0u) {
		return spleen_block5[0];
	} else if (codepoint >= 0x1f4u && codepoint <= 0x1f5u) {
		return spleen_block6[codepoint - 0x1f4u];
	} else if (codepoint >= 0x1feu && codepoint <= 0x201u) {
		return spleen_block7[codepoint - 0x1feu];
	} else if (codepoint >= 0x204u && codepoint <= 0x205u) {
		return spleen_block8[codepoint - 0x204u];
	} else if (codepoint >= 0x208u && codepoint <= 0x209u) {
		return spleen_block9[codepoint - 0x208u];
	} else if (codepoint >= 0x20cu && codepoint <= 0x20du) {
		return spleen_block10[codepoint - 0x20cu];
	} else if (codepoint >= 0x210u && codepoint <= 0x211u) {
		return spleen_block11[codepoint - 0x210u];
	} else if (codepoint >= 0x214u && codepoint <= 0x215u) {
		return spleen_block12[codepoint - 0x214u];
	} else if (codepoint >= 0x21eu && codepoint <= 0x21fu) {
		return spleen_block13[codepoint - 0x21eu];
	} else if (codepoint >= 0x228u && codepoint <= 0x229u) {
		return spleen_block14[codepoint - 0x228u];
	} else if (codepoint >= 0x2d8u && codepoint <= 0x2d9u) {
		return spleen_block15[codepoint - 0x2d8u];
	} else if (codepoint >= 0x2dbu && codepoint <= 0x2ddu) {
		return spleen_block16[codepoint - 0x2dbu];
	} else if (codepoint == 0x2016u) {
		return spleen_block17[0];
	} else if (codepoint >= 0x2018u && codepoint <= 0x2019u) {
		return spleen_block18[codepoint - 0x2018u];
	} else if (codepoint >= 0x201cu && codepoint <= 0x201du) {
		return spleen_block19[codepoint - 0x201cu];
	} else if (codepoint == 0x2022u) {
		return spleen_block20[0];
	} else if (codepoint == 0x2026u) {
		return spleen_block21[0];
	} else if (codepoint >= 0x2039u && codepoint <= 0x203au) {
		return spleen_block22[codepoint - 0x2039u];
	} else if (codepoint == 0x203cu) {
		return spleen_block23[0];
	} else if (codepoint == 0x20acu) {
		return spleen_block24[0];
	} else if (codepoint == 0x2248u) {
		return spleen_block25[0];
	} else if (codepoint >= 0x2500u && codepoint <= 0x259fu) {
		return spleen_block26[codepoint - 0x2500u];
	} else if (codepoint == 0x25c6u) {
		return spleen_block27[0];
	} else if (codepoint >= 0x25cau && codepoint <= 0x25cbu) {
		return spleen_block28[codepoint - 0x25cau];
	} else if (codepoint == 0x25cfu) {
		return spleen_block29[0];
	} else if (codepoint >= 0x25d8u && codepoint <= 0x25d9u) {
		return spleen_block30[codepoint - 0x25d8u];
	} else if (codepoint >= 0x25e2u && codepoint <= 0x25e5u) {
		return spleen_block31[codepoint - 0x25e2u];
	} else if (codepoint >= 0x2630u && codepoint <= 0x2637u) {
		return spleen_block32[codepoint - 0x2630u];
	} else if (codepoint >= 0x2665u && codepoint <= 0x2666u) {
		return spleen_block33[codepoint - 0x2665u];
	} else if (codepoint >= 0x2800u && codepoint <= 0x28ffu) {
		return spleen_block34[codepoint - 0x2800u];
	} else if (codepoint >= 0xe0a0u && codepoint <= 0xe0a2u) {
		return spleen_block35[codepoint - 0xe0a0u];
	} else if (codepoint >= 0xe0b0u && codepoint <= 0xe0b3u) {
		return spleen_block36[codepoint - 0xe0b0u];
	} else {
		WARN("Codepoint U+%04X (%s) is not covered by this font", codepoint, u8_cp_to_utf8(codepoint));
		return spleen_block1[0];
	}
}
