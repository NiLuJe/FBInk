/*
	FBInk: FrameBuffer eInker, a library to print text & images to an eInk Linux framebuffer
	Copyright (C) 2018-2023 NiLuJe <ninuje@gmail.com>
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

#include "fbink_misc_fonts.h"

static const unsigned char*
    kates_get_bitmap(uint32_t codepoint)
{
	if (codepoint <= 0x02u) {
		return kates_block1[codepoint];
	} else if (codepoint >= 0x09u && codepoint <= 0x19u) {
		return kates_block2[codepoint - 0x09u];
	} else if (codepoint == 0x1bu) {
		return kates_block3[0];
	} else if (codepoint >= 0x20u && codepoint <= 0x7eu) {
		return kates_block4[codepoint - 0x20u];
	} else if (codepoint >= 0xa1u && codepoint <= 0xacu) {
		return kates_block5[codepoint - 0xa1u];
	} else if (codepoint >= 0xaeu && codepoint <= 0xffu) {
		return kates_block6[codepoint - 0xaeu];
	} else {
		WARN("Codepoint U+%04X (%s) is not covered by this font", codepoint, u8_cp_to_utf8(codepoint));
		return kates_block1[0];
	}
}

static const unsigned char*
    fkp_get_bitmap(uint32_t codepoint)
{
	if (codepoint <= 0xffu) {
		return fkp_block1[codepoint];
	} else {
		WARN("Codepoint U+%04X (%s) is not covered by this font", codepoint, u8_cp_to_utf8(codepoint));
		return fkp_block1[0];
	}
}

static const unsigned char*
    ctrld_get_bitmap(uint32_t codepoint)
{
	if (codepoint >= 0x0au && codepoint <= 0xffu) {
		return ctrld_block1[codepoint - 0x0au];
	} else if (codepoint == 0x3bbu) {
		return ctrld_block2[0];
	} else if (codepoint == 0x3c0u) {
		return ctrld_block3[0];
	} else if (codepoint >= 0x2190u && codepoint <= 0x2193u) {
		return ctrld_block4[codepoint - 0x2190u];
	} else if (codepoint == 0x21b5u) {
		return ctrld_block5[0];
	} else if (codepoint >= 0x21e0u && codepoint <= 0x21e3u) {
		return ctrld_block6[codepoint - 0x21e0u];
	} else if (codepoint == 0x25a0u) {
		return ctrld_block7[0];
	} else if (codepoint == 0x25aau) {
		return ctrld_block8[0];
	} else if (codepoint == 0x25b4u) {
		return ctrld_block9[0];
	} else if (codepoint == 0x25b8u) {
		return ctrld_block10[0];
	} else if (codepoint == 0x25beu) {
		return ctrld_block11[0];
	} else if (codepoint == 0x25c2u) {
		return ctrld_block12[0];
	} else if (codepoint == 0x25c6u) {
		return ctrld_block13[0];
	} else if (codepoint == 0x2713u) {
		return ctrld_block14[0];
	} else if (codepoint == 0x2717u) {
		return ctrld_block15[0];
	} else if (codepoint >= 0x276eu && codepoint <= 0x276fu) {
		return ctrld_block16[codepoint - 0x276eu];
	} else if (codepoint == 0x27f3u) {
		return ctrld_block17[0];
	} else if (codepoint >= 0xe0a0u && codepoint <= 0xe0a3u) {
		return ctrld_block18[codepoint - 0xe0a0u];
	} else if (codepoint >= 0xe0b0u && codepoint <= 0xe0b7u) {
		return ctrld_block19[codepoint - 0xe0b0u];
	} else if (codepoint >= 0xee00u && codepoint <= 0xee03u) {
		return ctrld_block20[codepoint - 0xee00u];
	} else if (codepoint >= 0xee10u && codepoint <= 0xee13u) {
		return ctrld_block21[codepoint - 0xee10u];
	} else if (codepoint >= 0xee20u && codepoint <= 0xee23u) {
		return ctrld_block22[codepoint - 0xee20u];
	} else if (codepoint >= 0xee30u && codepoint <= 0xee37u) {
		return ctrld_block23[codepoint - 0xee30u];
	} else if (codepoint >= 0xee40u && codepoint <= 0xee43u) {
		return ctrld_block24[codepoint - 0xee40u];
	} else if (codepoint >= 0xeef0u && codepoint <= 0xeef9u) {
		return ctrld_block25[codepoint - 0xeef0u];
	} else {
		WARN("Codepoint U+%04X (%s) is not covered by this font", codepoint, u8_cp_to_utf8(codepoint));
		return ctrld_block1[0];
	}
}
