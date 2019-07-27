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

#include "fbink_fatty.h"

static const unsigned char*
    fatty_get_bitmap(uint32_t codepoint)
{
	if (codepoint <= 0x00) {
		return fatty_block1[codepoint];
	} else if (codepoint >= 0x20 && codepoint <= 0x7f) {
		return fatty_block2[codepoint - 0x20];
	} else if (codepoint >= 0xa0 && codepoint <= 0x17f) {
		return fatty_block3[codepoint - 0xa0];
	} else if (codepoint >= 0x218 && codepoint <= 0x21b) {
		return fatty_block4[codepoint - 0x218];
	} else if (codepoint == 0x2c7) {
		return fatty_block5[0];
	} else if (codepoint >= 0x2d8 && codepoint <= 0x2d9) {
		return fatty_block6[codepoint - 0x2d8];
	} else if (codepoint == 0x2db) {
		return fatty_block7[0];
	} else if (codepoint == 0x2dd) {
		return fatty_block8[0];
	} else if (codepoint >= 0x1e02 && codepoint <= 0x1e03) {
		return fatty_block9[codepoint - 0x1e02];
	} else if (codepoint >= 0x1e0a && codepoint <= 0x1e0b) {
		return fatty_block10[codepoint - 0x1e0a];
	} else if (codepoint >= 0x1e1e && codepoint <= 0x1e1f) {
		return fatty_block11[codepoint - 0x1e1e];
	} else if (codepoint >= 0x1e40 && codepoint <= 0x1e41) {
		return fatty_block12[codepoint - 0x1e40];
	} else if (codepoint >= 0x1e56 && codepoint <= 0x1e57) {
		return fatty_block13[codepoint - 0x1e56];
	} else if (codepoint >= 0x1e60 && codepoint <= 0x1e61) {
		return fatty_block14[codepoint - 0x1e60];
	} else if (codepoint >= 0x1e6a && codepoint <= 0x1e6b) {
		return fatty_block15[codepoint - 0x1e6a];
	} else if (codepoint >= 0x1e80 && codepoint <= 0x1e85) {
		return fatty_block16[codepoint - 0x1e80];
	} else if (codepoint >= 0x1ef2 && codepoint <= 0x1ef3) {
		return fatty_block17[codepoint - 0x1ef2];
	} else if (codepoint == 0x2010) {
		return fatty_block18[0];
	} else if (codepoint >= 0x2013 && codepoint <= 0x2015) {
		return fatty_block19[codepoint - 0x2013];
	} else if (codepoint >= 0x2018 && codepoint <= 0x2019) {
		return fatty_block20[codepoint - 0x2018];
	} else if (codepoint >= 0x201b && codepoint <= 0x201f) {
		return fatty_block21[codepoint - 0x201b];
	} else if (codepoint == 0x2022) {
		return fatty_block22[0];
	} else if (codepoint == 0x2026) {
		return fatty_block23[0];
	} else if (codepoint == 0x2030) {
		return fatty_block24[0];
	} else if (codepoint == 0x2052) {
		return fatty_block25[0];
	} else if (codepoint == 0x20ac) {
		return fatty_block26[0];
	} else if (codepoint == 0x2122) {
		return fatty_block27[0];
	} else if (codepoint == 0x2192) {
		return fatty_block28[0];
	} else if (codepoint == 0x2260) {
		return fatty_block29[0];
	} else if (codepoint >= 0x2500 && codepoint <= 0x2503) {
		return fatty_block30[codepoint - 0x2500];
	} else if (codepoint == 0x250c) {
		return fatty_block31[0];
	} else if (codepoint == 0x2510) {
		return fatty_block32[0];
	} else if (codepoint == 0x2514) {
		return fatty_block33[0];
	} else if (codepoint == 0x2518) {
		return fatty_block34[0];
	} else if (codepoint == 0x251c) {
		return fatty_block35[0];
	} else if (codepoint == 0x2524) {
		return fatty_block36[0];
	} else if (codepoint == 0x252c) {
		return fatty_block37[0];
	} else if (codepoint == 0x2534) {
		return fatty_block38[0];
	} else if (codepoint == 0x253c) {
		return fatty_block39[0];
	} else if (codepoint >= 0x25a0 && codepoint <= 0x25a1) {
		return fatty_block40[codepoint - 0x25a0];
	} else if (codepoint == 0x25b2) {
		return fatty_block41[0];
	} else if (codepoint == 0x25ba) {
		return fatty_block42[0];
	} else if (codepoint == 0x25bc) {
		return fatty_block43[0];
	} else if (codepoint == 0x25cb) {
		return fatty_block44[0];
	} else if (codepoint == 0x25cf) {
		return fatty_block45[0];
	} else if (codepoint == 0x2603) {
		return fatty_block46[0];
	} else if (codepoint >= 0x2605 && codepoint <= 0x2606) {
		return fatty_block47[codepoint - 0x2605];
	} else if (codepoint == 0x263a) {
		return fatty_block48[0];
	} else if (codepoint == 0x2665) {
		return fatty_block49[0];
	} else if (codepoint == 0x269b) {
		return fatty_block50[0];
	} else if (codepoint == 0x2705) {
		return fatty_block51[0];
	} else if (codepoint == 0x2708) {
		return fatty_block52[0];
	} else if (codepoint == 0x2713) {
		return fatty_block53[0];
	} else if (codepoint == 0x2744) {
		return fatty_block54[0];
	} else if (codepoint >= 0x2800 && codepoint <= 0x28ff) {
		return fatty_block55[codepoint - 0x2800];
	} else if (codepoint == 0x30fb) {
		return fatty_block56[0];
	} else if (codepoint == 0xfffd) {
		return fatty_block57[0];
	} else {
		WARN("Codepoint U+%04X is not covered by this font", codepoint);
		return fatty_block1[0];
	}
}
