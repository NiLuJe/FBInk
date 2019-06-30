/*
	FBInk: FrameBuffer eInker, a tool to print text & images on eInk devices (Kobo/Kindle)
	Copyright (C) 2018-2019 NiLuJe <ninuje@gmail.com>

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

#include "fbink_vga.h"

static const unsigned char*
    vga_get_bitmap(uint32_t codepoint)
{
	if (codepoint <= 0x7f) {
		return vga_block1[codepoint];
	} else if (codepoint >= 0xa0 && codepoint <= 0x17f) {
		return vga_block2[codepoint - 0xa0];
	} else if (codepoint == 0x192) {
		return vga_block3[0];
	} else if (codepoint == 0x1a1) {
		return vga_block4[0];
	} else if (codepoint == 0x1b7) {
		return vga_block5[0];
	} else if (codepoint >= 0x1fa && codepoint <= 0x1ff) {
		return vga_block6[codepoint - 0x1fa];
	} else if (codepoint >= 0x218 && codepoint <= 0x21b) {
		return vga_block7[codepoint - 0x218];
	} else if (codepoint == 0x251) {
		return vga_block8[0];
	} else if (codepoint == 0x278) {
		return vga_block9[0];
	} else if (codepoint >= 0x2c6 && codepoint <= 0x2c7) {
		return vga_block10[codepoint - 0x2c6];
	} else if (codepoint == 0x2c9) {
		return vga_block11[0];
	} else if (codepoint >= 0x2d8 && codepoint <= 0x2dd) {
		return vga_block12[codepoint - 0x2d8];
	} else if (codepoint == 0x37e) {
		return vga_block13[0];
	} else if (codepoint >= 0x384 && codepoint <= 0x38a) {
		return vga_block14[codepoint - 0x384];
	} else if (codepoint == 0x38c) {
		return vga_block15[0];
	} else if (codepoint >= 0x38e && codepoint <= 0x3a1) {
		return vga_block16[codepoint - 0x38e];
	} else if (codepoint >= 0x3a3 && codepoint <= 0x3ce) {
		return vga_block17[codepoint - 0x3a3];
	} else if (codepoint == 0x3d0) {
		return vga_block18[0];
	} else if (codepoint == 0x3f4) {
		return vga_block19[0];
	} else if (codepoint >= 0x400 && codepoint <= 0x45f) {
		return vga_block20[codepoint - 0x400];
	} else if (codepoint >= 0x490 && codepoint <= 0x491) {
		return vga_block21[codepoint - 0x490];
	} else if (codepoint == 0x5be) {
		return vga_block22[0];
	} else if (codepoint >= 0x5d0 && codepoint <= 0x5ea) {
		return vga_block23[codepoint - 0x5d0];
	} else if (codepoint >= 0x5f0 && codepoint <= 0x5f4) {
		return vga_block24[codepoint - 0x5f0];
	} else if (codepoint == 0x1d1b) {
		return vga_block25[0];
	} else if (codepoint == 0x1d26) {
		return vga_block26[0];
	} else if (codepoint == 0x1d28) {
		return vga_block27[0];
	} else if (codepoint >= 0x1e80 && codepoint <= 0x1e85) {
		return vga_block28[codepoint - 0x1e80];
	} else if (codepoint == 0x1e9f) {
		return vga_block29[0];
	} else if (codepoint >= 0x1ef2 && codepoint <= 0x1ef3) {
		return vga_block30[codepoint - 0x1ef2];
	} else if (codepoint == 0x2010) {
		return vga_block31[0];
	} else if (codepoint >= 0x2012 && codepoint <= 0x2015) {
		return vga_block32[codepoint - 0x2012];
	} else if (codepoint >= 0x2017 && codepoint <= 0x2022) {
		return vga_block33[codepoint - 0x2017];
	} else if (codepoint >= 0x2026 && codepoint <= 0x2027) {
		return vga_block34[codepoint - 0x2026];
	} else if (codepoint == 0x2030) {
		return vga_block35[0];
	} else if (codepoint >= 0x2032 && codepoint <= 0x2033) {
		return vga_block36[codepoint - 0x2032];
	} else if (codepoint == 0x2035) {
		return vga_block37[0];
	} else if (codepoint >= 0x2039 && codepoint <= 0x203a) {
		return vga_block38[codepoint - 0x2039];
	} else if (codepoint == 0x203c) {
		return vga_block39[0];
	} else if (codepoint >= 0x203e && codepoint <= 0x2040) {
		return vga_block40[codepoint - 0x203e];
	} else if (codepoint == 0x2044) {
		return vga_block41[0];
	} else if (codepoint == 0x2054) {
		return vga_block42[0];
	} else if (codepoint >= 0x2074 && codepoint <= 0x207b) {
		return vga_block43[codepoint - 0x2074];
	} else if (codepoint == 0x207f) {
		return vga_block44[0];
	} else if (codepoint >= 0x2081 && codepoint <= 0x208b) {
		return vga_block45[codepoint - 0x2081];
	} else if (codepoint >= 0x20a3 && codepoint <= 0x20a4) {
		return vga_block46[codepoint - 0x20a3];
	} else if (codepoint == 0x20a7) {
		return vga_block47[0];
	} else if (codepoint == 0x20aa) {
		return vga_block48[0];
	} else if (codepoint == 0x20ac) {
		return vga_block49[0];
	} else if (codepoint == 0x2105) {
		return vga_block50[0];
	} else if (codepoint == 0x2113) {
		return vga_block51[0];
	} else if (codepoint == 0x2116) {
		return vga_block52[0];
	} else if (codepoint == 0x2122) {
		return vga_block53[0];
	} else if (codepoint == 0x2126) {
		return vga_block54[0];
	} else if (codepoint == 0x212e) {
		return vga_block55[0];
	} else if (codepoint >= 0x2150 && codepoint <= 0x2151) {
		return vga_block56[codepoint - 0x2150];
	} else if (codepoint >= 0x2153 && codepoint <= 0x215e) {
		return vga_block57[codepoint - 0x2153];
	} else if (codepoint >= 0x2190 && codepoint <= 0x2195) {
		return vga_block58[codepoint - 0x2190];
	} else if (codepoint == 0x21a8) {
		return vga_block59[0];
	} else if (codepoint == 0x2202) {
		return vga_block60[0];
	} else if (codepoint >= 0x2205 && codepoint <= 0x2206) {
		return vga_block61[codepoint - 0x2205];
	} else if (codepoint == 0x2208) {
		return vga_block62[0];
	} else if (codepoint == 0x220f) {
		return vga_block63[0];
	} else if (codepoint >= 0x2211 && codepoint <= 0x2212) {
		return vga_block64[codepoint - 0x2211];
	} else if (codepoint == 0x2215) {
		return vga_block65[0];
	} else if (codepoint >= 0x2219 && codepoint <= 0x221a) {
		return vga_block66[codepoint - 0x2219];
	} else if (codepoint >= 0x221e && codepoint <= 0x221f) {
		return vga_block67[codepoint - 0x221e];
	} else if (codepoint == 0x2229) {
		return vga_block68[0];
	} else if (codepoint == 0x222b) {
		return vga_block69[0];
	} else if (codepoint == 0x2248) {
		return vga_block70[0];
	} else if (codepoint >= 0x2260 && codepoint <= 0x2261) {
		return vga_block71[codepoint - 0x2260];
	} else if (codepoint >= 0x2264 && codepoint <= 0x2265) {
		return vga_block72[codepoint - 0x2264];
	} else if (codepoint == 0x2299) {
		return vga_block73[0];
	} else if (codepoint == 0x2300) {
		return vga_block74[0];
	} else if (codepoint == 0x2302) {
		return vga_block75[0];
	} else if (codepoint == 0x2310) {
		return vga_block76[0];
	} else if (codepoint >= 0x2320 && codepoint <= 0x2321) {
		return vga_block77[codepoint - 0x2320];
	} else if (codepoint == 0x2500) {
		return vga_block78[0];
	} else if (codepoint == 0x2502) {
		return vga_block79[0];
	} else if (codepoint == 0x250c) {
		return vga_block80[0];
	} else if (codepoint == 0x2510) {
		return vga_block81[0];
	} else if (codepoint == 0x2514) {
		return vga_block82[0];
	} else if (codepoint == 0x2518) {
		return vga_block83[0];
	} else if (codepoint == 0x251c) {
		return vga_block84[0];
	} else if (codepoint == 0x2524) {
		return vga_block85[0];
	} else if (codepoint == 0x252c) {
		return vga_block86[0];
	} else if (codepoint == 0x2534) {
		return vga_block87[0];
	} else if (codepoint == 0x253c) {
		return vga_block88[0];
	} else if (codepoint >= 0x2550 && codepoint <= 0x256c) {
		return vga_block89[codepoint - 0x2550];
	} else if (codepoint >= 0x2580 && codepoint <= 0x25a1) {
		return vga_block90[codepoint - 0x2580];
	} else if (codepoint >= 0x25aa && codepoint <= 0x25ac) {
		return vga_block91[codepoint - 0x25aa];
	} else if (codepoint == 0x25b2) {
		return vga_block92[0];
	} else if (codepoint == 0x25ba) {
		return vga_block93[0];
	} else if (codepoint == 0x25bc) {
		return vga_block94[0];
	} else if (codepoint == 0x25c4) {
		return vga_block95[0];
	} else if (codepoint >= 0x25ca && codepoint <= 0x25cb) {
		return vga_block96[codepoint - 0x25ca];
	} else if (codepoint == 0x25cf) {
		return vga_block97[0];
	} else if (codepoint >= 0x25d8 && codepoint <= 0x25d9) {
		return vga_block98[codepoint - 0x25d8];
	} else if (codepoint == 0x25e6) {
		return vga_block99[0];
	} else if (codepoint >= 0x263a && codepoint <= 0x263c) {
		return vga_block100[codepoint - 0x263a];
	} else if (codepoint == 0x2640) {
		return vga_block101[0];
	} else if (codepoint == 0x2642) {
		return vga_block102[0];
	} else if (codepoint == 0x2660) {
		return vga_block103[0];
	} else if (codepoint == 0x2663) {
		return vga_block104[0];
	} else if (codepoint >= 0x2665 && codepoint <= 0x2666) {
		return vga_block105[codepoint - 0x2665];
	} else if (codepoint >= 0x266a && codepoint <= 0x266b) {
		return vga_block106[codepoint - 0x266a];
	} else if (codepoint == 0x2713) {
		return vga_block107[0];
	} else if (codepoint >= 0xfb01 && codepoint <= 0xfb02) {
		return vga_block108[codepoint - 0xfb01];
	} else if (codepoint == 0xfffd) {
		return vga_block109[0];
	} else {
		WARN("Codepoint U+%04X is not covered by this font", codepoint);
		return vga_block1[0];
	}
}
