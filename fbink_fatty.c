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

#include "fbink_fatty.h"

static const unsigned char*
    fatty_get_bitmap(uint32_t codepoint)
{
	if (codepoint == 0x00u) {
		return fatty_block1[codepoint];
	} else if (codepoint >= 0x20u && codepoint <= 0x7fu) {
		return fatty_block2[codepoint - 0x20u];
	} else if (codepoint >= 0xa0u && codepoint <= 0x17fu) {
		return fatty_block3[codepoint - 0xa0u];
	} else if (codepoint >= 0x218u && codepoint <= 0x21bu) {
		return fatty_block4[codepoint - 0x218u];
	} else if (codepoint == 0x2c7u) {
		return fatty_block5[0];
	} else if (codepoint >= 0x2d8u && codepoint <= 0x2d9u) {
		return fatty_block6[codepoint - 0x2d8u];
	} else if (codepoint == 0x2dbu) {
		return fatty_block7[0];
	} else if (codepoint == 0x2ddu) {
		return fatty_block8[0];
	} else if (codepoint >= 0x1e02u && codepoint <= 0x1e03u) {
		return fatty_block9[codepoint - 0x1e02u];
	} else if (codepoint >= 0x1e0au && codepoint <= 0x1e0bu) {
		return fatty_block10[codepoint - 0x1e0au];
	} else if (codepoint >= 0x1e1eu && codepoint <= 0x1e1fu) {
		return fatty_block11[codepoint - 0x1e1eu];
	} else if (codepoint >= 0x1e40u && codepoint <= 0x1e41u) {
		return fatty_block12[codepoint - 0x1e40u];
	} else if (codepoint >= 0x1e56u && codepoint <= 0x1e57u) {
		return fatty_block13[codepoint - 0x1e56u];
	} else if (codepoint >= 0x1e60u && codepoint <= 0x1e61u) {
		return fatty_block14[codepoint - 0x1e60u];
	} else if (codepoint >= 0x1e6au && codepoint <= 0x1e6bu) {
		return fatty_block15[codepoint - 0x1e6au];
	} else if (codepoint >= 0x1e80u && codepoint <= 0x1e85u) {
		return fatty_block16[codepoint - 0x1e80u];
	} else if (codepoint >= 0x1ef2u && codepoint <= 0x1ef3u) {
		return fatty_block17[codepoint - 0x1ef2u];
	} else if (codepoint == 0x2010u) {
		return fatty_block18[0];
	} else if (codepoint >= 0x2013u && codepoint <= 0x2015u) {
		return fatty_block19[codepoint - 0x2013u];
	} else if (codepoint >= 0x2018u && codepoint <= 0x2019u) {
		return fatty_block20[codepoint - 0x2018u];
	} else if (codepoint >= 0x201bu && codepoint <= 0x201fu) {
		return fatty_block21[codepoint - 0x201bu];
	} else if (codepoint == 0x2022u) {
		return fatty_block22[0];
	} else if (codepoint == 0x2026u) {
		return fatty_block23[0];
	} else if (codepoint == 0x2030u) {
		return fatty_block24[0];
	} else if (codepoint == 0x2052u) {
		return fatty_block25[0];
	} else if (codepoint == 0x20acu) {
		return fatty_block26[0];
	} else if (codepoint == 0x2122u) {
		return fatty_block27[0];
	} else if (codepoint == 0x2192u) {
		return fatty_block28[0];
	} else if (codepoint == 0x2260u) {
		return fatty_block29[0];
	} else if (codepoint >= 0x2500u && codepoint <= 0x2503u) {
		return fatty_block30[codepoint - 0x2500u];
	} else if (codepoint == 0x250cu) {
		return fatty_block31[0];
	} else if (codepoint == 0x2510u) {
		return fatty_block32[0];
	} else if (codepoint == 0x2514u) {
		return fatty_block33[0];
	} else if (codepoint == 0x2518u) {
		return fatty_block34[0];
	} else if (codepoint == 0x251cu) {
		return fatty_block35[0];
	} else if (codepoint == 0x2524u) {
		return fatty_block36[0];
	} else if (codepoint == 0x252cu) {
		return fatty_block37[0];
	} else if (codepoint == 0x2534u) {
		return fatty_block38[0];
	} else if (codepoint == 0x253cu) {
		return fatty_block39[0];
	} else if (codepoint >= 0x25a0u && codepoint <= 0x25a1u) {
		return fatty_block40[codepoint - 0x25a0u];
	} else if (codepoint == 0x25b2u) {
		return fatty_block41[0];
	} else if (codepoint == 0x25bau) {
		return fatty_block42[0];
	} else if (codepoint == 0x25bcu) {
		return fatty_block43[0];
	} else if (codepoint == 0x25cbu) {
		return fatty_block44[0];
	} else if (codepoint == 0x25cfu) {
		return fatty_block45[0];
	} else if (codepoint == 0x2603u) {
		return fatty_block46[0];
	} else if (codepoint >= 0x2605u && codepoint <= 0x2606u) {
		return fatty_block47[codepoint - 0x2605u];
	} else if (codepoint == 0x263au) {
		return fatty_block48[0];
	} else if (codepoint == 0x2665u) {
		return fatty_block49[0];
	} else if (codepoint == 0x269bu) {
		return fatty_block50[0];
	} else if (codepoint == 0x2705u) {
		return fatty_block51[0];
	} else if (codepoint == 0x2708u) {
		return fatty_block52[0];
	} else if (codepoint == 0x2713u) {
		return fatty_block53[0];
	} else if (codepoint == 0x2744u) {
		return fatty_block54[0];
	} else if (codepoint >= 0x2800u && codepoint <= 0x28ffu) {
		return fatty_block55[codepoint - 0x2800u];
	} else if (codepoint == 0x30fbu) {
		return fatty_block56[0];
	} else if (codepoint == 0xfffdu) {
		return fatty_block57[0];
	} else {
		WARN("Codepoint U+%04X (%s) is not covered by this font", codepoint, u8_cp_to_utf8(codepoint));
		return fatty_block1[0];
	}
}
