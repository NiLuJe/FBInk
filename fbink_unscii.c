/*
	FBInk: FrameBuffer eInker, a tool to print strings on eInk devices (Kobo/Kindle)
	Copyright (C) 2018 NiLuJe <ninuje@gmail.com>

	Linux framebuffer routines based on: fbtestfnt.c & fbtest6.c, from
	http://raspberrycompote.blogspot.com/2014/04/low-level-graphics-on-raspberry-pi-text.html &
	https://raspberrycompote.blogspot.com/2013/03/low-level-graphics-on-raspberry-pi-part_8.html
	Original works by J-P Rosti (a.k.a -rst- and 'Raspberry Compote'),
	Licensed under the Creative Commons Attribution 3.0 Unported License
	(http://creativecommons.org/licenses/by/3.0/deed.en_US)

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

#include "fbink_unscii.h"

static const char*
    unscii_get_bitmap(uint32_t codepoint)
{
	if (codepoint <= 0x195) {
		return unscii_block1[codepoint];
	} else if (codepoint >= 0x197 && codepoint <= 0x19f) {
		return unscii_block2[codepoint - 0x197];
	} else if (codepoint >= 0x1cd && codepoint <= 0x1e3) {
		return unscii_block3[codepoint - 0x1cd];
	} else if (codepoint >= 0x1e6 && codepoint <= 0x1e8) {
		return unscii_block4[codepoint - 0x1e6];
	} else if (codepoint >= 0x1f0 && codepoint <= 0x1f0) {
		return unscii_block5[codepoint - 0x1f0];
	} else if (codepoint >= 0x1f4 && codepoint <= 0x1f5) {
		return unscii_block6[codepoint - 0x1f4];
	} else if (codepoint >= 0x1f8 && codepoint <= 0x21b) {
		return unscii_block7[codepoint - 0x1f8];
	} else if (codepoint >= 0x226 && codepoint <= 0x227) {
		return unscii_block8[codepoint - 0x226];
	} else if (codepoint >= 0x22a && codepoint <= 0x233) {
		return unscii_block9[codepoint - 0x22a];
	} else if (codepoint >= 0x258 && codepoint <= 0x259) {
		return unscii_block10[codepoint - 0x258];
	} else if (codepoint >= 0x263 && codepoint <= 0x263) {
		return unscii_block11[codepoint - 0x263];
	} else if (codepoint >= 0x294 && codepoint <= 0x294) {
		return unscii_block12[codepoint - 0x294];
	} else if (codepoint >= 0x2c6 && codepoint <= 0x2c6) {
		return unscii_block13[codepoint - 0x2c6];
	} else if (codepoint >= 0x2c9 && codepoint <= 0x2c9) {
		return unscii_block14[codepoint - 0x2c9];
	} else if (codepoint >= 0x2cd && codepoint <= 0x2cd) {
		return unscii_block15[codepoint - 0x2cd];
	} else if (codepoint >= 0x2dc && codepoint <= 0x2dc) {
		return unscii_block16[codepoint - 0x2dc];
	} else if (codepoint >= 0x34f && codepoint <= 0x34f) {
		return unscii_block17[codepoint - 0x34f];
	} else if (codepoint >= 0x391 && codepoint <= 0x3a1) {
		return unscii_block18[codepoint - 0x391];
	} else if (codepoint >= 0x3a3 && codepoint <= 0x3a9) {
		return unscii_block19[codepoint - 0x3a3];
	} else if (codepoint >= 0x3b1 && codepoint <= 0x3c9) {
		return unscii_block20[codepoint - 0x3b1];
	} else if (codepoint >= 0x400 && codepoint <= 0x401) {
		return unscii_block21[codepoint - 0x400];
	} else if (codepoint >= 0x405 && codepoint <= 0x408) {
		return unscii_block22[codepoint - 0x405];
	} else if (codepoint >= 0x410 && codepoint <= 0x451) {
		return unscii_block23[codepoint - 0x410];
	} else if (codepoint >= 0x455 && codepoint <= 0x458) {
		return unscii_block24[codepoint - 0x455];
	} else if (codepoint >= 0x4d0 && codepoint <= 0x4d7) {
		return unscii_block25[codepoint - 0x4d0];
	} else if (codepoint >= 0x4e6 && codepoint <= 0x4e7) {
		return unscii_block26[codepoint - 0x4e6];
	} else if (codepoint >= 0x4f1 && codepoint <= 0x4f1) {
		return unscii_block27[codepoint - 0x4f1];
	} else if (codepoint >= 0x5d0 && codepoint <= 0x5ea) {
		return unscii_block28[codepoint - 0x5d0];
	} else if (codepoint >= 0x623 && codepoint <= 0x623) {
		return unscii_block29[codepoint - 0x623];
	} else if (codepoint >= 0x627 && codepoint <= 0x62d) {
		return unscii_block30[codepoint - 0x627];
	} else if (codepoint >= 0x62f && codepoint <= 0x635) {
		return unscii_block31[codepoint - 0x62f];
	} else if (codepoint >= 0x637 && codepoint <= 0x63a) {
		return unscii_block32[codepoint - 0x637];
	} else if (codepoint >= 0x640 && codepoint <= 0x64a) {
		return unscii_block33[codepoint - 0x640];
	} else if (codepoint >= 0x671 && codepoint <= 0x673) {
		return unscii_block34[codepoint - 0x671];
	} else if (codepoint >= 0x1560 && codepoint <= 0x1563) {
		return unscii_block35[codepoint - 0x1560];
	} else if (codepoint >= 0x156a && codepoint <= 0x156a) {
		return unscii_block36[codepoint - 0x156a];
	} else if (codepoint >= 0x156c && codepoint <= 0x1571) {
		return unscii_block37[codepoint - 0x156c];
	} else if (codepoint >= 0x15ec && codepoint <= 0x15ed) {
		return unscii_block38[codepoint - 0x15ec];
	} else if (codepoint >= 0x15ef && codepoint <= 0x15ef) {
		return unscii_block39[codepoint - 0x15ef];
	} else if (codepoint >= 0x16a0 && codepoint <= 0x16ab) {
		return unscii_block40[codepoint - 0x16a0];
	} else if (codepoint >= 0x16b1 && codepoint <= 0x16f8) {
		return unscii_block41[codepoint - 0x16b1];
	} else if (codepoint >= 0x1e00 && codepoint <= 0x1e02) {
		return unscii_block42[codepoint - 0x1e00];
	} else if (codepoint >= 0x1e04 && codepoint <= 0x1e07) {
		return unscii_block43[codepoint - 0x1e04];
	} else if (codepoint >= 0x1e0a && codepoint <= 0x1e0a) {
		return unscii_block44[codepoint - 0x1e0a];
	} else if (codepoint >= 0x1e0c && codepoint <= 0x1e0f) {
		return unscii_block45[codepoint - 0x1e0c];
	} else if (codepoint >= 0x1e41 && codepoint <= 0x1e4b) {
		return unscii_block46[codepoint - 0x1e41];
	} else if (codepoint >= 0x1e58 && codepoint <= 0x1e5b) {
		return unscii_block47[codepoint - 0x1e58];
	} else if (codepoint >= 0x1e5e && codepoint <= 0x1e5e) {
		return unscii_block48[codepoint - 0x1e5e];
	} else if (codepoint >= 0x2010 && codepoint <= 0x2019) {
		return unscii_block49[codepoint - 0x2010];
	} else if (codepoint >= 0x201c && codepoint <= 0x201d) {
		return unscii_block50[codepoint - 0x201c];
	} else if (codepoint >= 0x2020 && codepoint <= 0x2022) {
		return unscii_block51[codepoint - 0x2020];
	} else if (codepoint >= 0x2024 && codepoint <= 0x2026) {
		return unscii_block52[codepoint - 0x2024];
	} else if (codepoint >= 0x2030 && codepoint <= 0x2031) {
		return unscii_block53[codepoint - 0x2030];
	} else if (codepoint >= 0x203c && codepoint <= 0x203c) {
		return unscii_block54[codepoint - 0x203c];
	} else if (codepoint >= 0x2047 && codepoint <= 0x2049) {
		return unscii_block55[codepoint - 0x2047];
	} else if (codepoint >= 0x2070 && codepoint <= 0x2070) {
		return unscii_block56[codepoint - 0x2070];
	} else if (codepoint >= 0x2074 && codepoint <= 0x2079) {
		return unscii_block57[codepoint - 0x2074];
	} else if (codepoint >= 0x207f && codepoint <= 0x207f) {
		return unscii_block58[codepoint - 0x207f];
	} else if (codepoint >= 0x20a7 && codepoint <= 0x20a7) {
		return unscii_block59[codepoint - 0x20a7];
	} else if (codepoint >= 0x20ac && codepoint <= 0x20ac) {
		return unscii_block60[codepoint - 0x20ac];
	} else if (codepoint >= 0x2117 && codepoint <= 0x2117) {
		return unscii_block61[codepoint - 0x2117];
	} else if (codepoint >= 0x2120 && codepoint <= 0x2120) {
		return unscii_block62[codepoint - 0x2120];
	} else if (codepoint >= 0x2122 && codepoint <= 0x2122) {
		return unscii_block63[codepoint - 0x2122];
	} else if (codepoint >= 0x2190 && codepoint <= 0x2192) {
		return unscii_block64[codepoint - 0x2190];
	} else if (codepoint >= 0x2194 && codepoint <= 0x2194) {
		return unscii_block65[codepoint - 0x2194];
	} else if (codepoint >= 0x2196 && codepoint <= 0x2199) {
		return unscii_block66[codepoint - 0x2196];
	} else if (codepoint >= 0x21a5 && codepoint <= 0x21a5) {
		return unscii_block67[codepoint - 0x21a5];
	} else if (codepoint >= 0x21a8 && codepoint <= 0x21a8) {
		return unscii_block68[codepoint - 0x21a8];
	} else if (codepoint >= 0x21e6 && codepoint <= 0x21e9) {
		return unscii_block69[codepoint - 0x21e6];
	} else if (codepoint >= 0x2212 && codepoint <= 0x2212) {
		return unscii_block70[codepoint - 0x2212];
	} else if (codepoint >= 0x2218 && codepoint <= 0x2218) {
		return unscii_block71[codepoint - 0x2218];
	} else if (codepoint >= 0x221a && codepoint <= 0x221a) {
		return unscii_block72[codepoint - 0x221a];
	} else if (codepoint >= 0x221e && codepoint <= 0x221f) {
		return unscii_block73[codepoint - 0x221e];
	} else if (codepoint >= 0x2229 && codepoint <= 0x2229) {
		return unscii_block74[codepoint - 0x2229];
	} else if (codepoint >= 0x2248 && codepoint <= 0x2248) {
		return unscii_block75[codepoint - 0x2248];
	} else if (codepoint >= 0x2261 && codepoint <= 0x2261) {
		return unscii_block76[codepoint - 0x2261];
	} else if (codepoint >= 0x2264 && codepoint <= 0x2265) {
		return unscii_block77[codepoint - 0x2264];
	} else if (codepoint >= 0x22c5 && codepoint <= 0x22c5) {
		return unscii_block78[codepoint - 0x22c5];
	} else if (codepoint >= 0x2320 && codepoint <= 0x2321) {
		return unscii_block79[codepoint - 0x2320];
	} else if (codepoint >= 0x23ea && codepoint <= 0x23ec) {
		return unscii_block80[codepoint - 0x23ea];
	} else if (codepoint >= 0x2500 && codepoint <= 0x25ff) {
		return unscii_block81[codepoint - 0x2500];
	} else if (codepoint >= 0x2625 && codepoint <= 0x2625) {
		return unscii_block82[codepoint - 0x2625];
	} else if (codepoint >= 0x2628 && codepoint <= 0x2628) {
		return unscii_block83[codepoint - 0x2628];
	} else if (codepoint >= 0x262f && codepoint <= 0x2637) {
		return unscii_block84[codepoint - 0x262f];
	} else if (codepoint >= 0x2639 && codepoint <= 0x263b) {
		return unscii_block85[codepoint - 0x2639];
	} else if (codepoint >= 0x2660 && codepoint <= 0x2667) {
		return unscii_block86[codepoint - 0x2660];
	} else if (codepoint >= 0x2669 && codepoint <= 0x266c) {
		return unscii_block87[codepoint - 0x2669];
	} else if (codepoint >= 0x268a && codepoint <= 0x268f) {
		return unscii_block88[codepoint - 0x268a];
	} else if (codepoint >= 0x26aa && codepoint <= 0x26ac) {
		return unscii_block89[codepoint - 0x26aa];
	} else if (codepoint >= 0x2708 && codepoint <= 0x2708) {
		return unscii_block90[codepoint - 0x2708];
	} else if (codepoint >= 0x2734 && codepoint <= 0x2734) {
		return unscii_block91[codepoint - 0x2734];
	} else if (codepoint >= 0x2800 && codepoint <= 0x28ff) {
		return unscii_block92[codepoint - 0x2800];
	} else if (codepoint >= 0x2913 && codepoint <= 0x2913) {
		return unscii_block93[codepoint - 0x2913];
	} else if (codepoint >= 0x2b1d && codepoint <= 0x2b1d) {
		return unscii_block94[codepoint - 0x2b1d];
	} else if (codepoint >= 0x2b24 && codepoint <= 0x2b24) {
		return unscii_block95[codepoint - 0x2b24];
	} else if (codepoint >= 0x2b55 && codepoint <= 0x2b55) {
		return unscii_block96[codepoint - 0x2b55];
	} else if (codepoint >= 0x2b58 && codepoint <= 0x2b58) {
		return unscii_block97[codepoint - 0x2b58];
	} else if (codepoint >= 0x2e2e && codepoint <= 0x2e2e) {
		return unscii_block98[codepoint - 0x2e2e];
	} else if (codepoint >= 0xe080 && codepoint <= 0xe19b) {
		return unscii_block99[codepoint - 0xe080];
	} else if (codepoint >= 0xe800 && codepoint <= 0xe850) {
		return unscii_block100[codepoint - 0xe800];
	} else if (codepoint >= 0xec00 && codepoint <= 0xec26) {
		return unscii_block101[codepoint - 0xec00];
	} else if (codepoint >= 0xfe81 && codepoint <= 0xfe82) {
		return unscii_block102[codepoint - 0xfe81];
	} else if (codepoint >= 0xfe84 && codepoint <= 0xfe84) {
		return unscii_block103[codepoint - 0xfe84];
	} else if (codepoint >= 0xfe87 && codepoint <= 0xfe87) {
		return unscii_block104[codepoint - 0xfe87];
	} else if (codepoint >= 0xfe8f && codepoint <= 0xfe9c) {
		return unscii_block105[codepoint - 0xfe8f];
	} else if (codepoint >= 0xfe9e && codepoint <= 0xfea4) {
		return unscii_block106[codepoint - 0xfe9e];
	} else if (codepoint >= 0xfea9 && codepoint <= 0xfead) {
		return unscii_block107[codepoint - 0xfea9];
	} else if (codepoint >= 0xfeaf && codepoint <= 0xfeaf) {
		return unscii_block108[codepoint - 0xfeaf];
	} else if (codepoint >= 0xfeb1 && codepoint <= 0xfeb7) {
		return unscii_block109[codepoint - 0xfeb1];
	} else if (codepoint >= 0xfeb9 && codepoint <= 0xfebc) {
		return unscii_block110[codepoint - 0xfeb9];
	} else if (codepoint >= 0xfec1 && codepoint <= 0xfec6) {
		return unscii_block111[codepoint - 0xfec1];
	} else if (codepoint >= 0xfec9 && codepoint <= 0xfedd) {
		return unscii_block112[codepoint - 0xfec9];
	} else if (codepoint >= 0xfedf && codepoint <= 0xfee3) {
		return unscii_block113[codepoint - 0xfedf];
	} else if (codepoint >= 0xfee5 && codepoint <= 0xfee9) {
		return unscii_block114[codepoint - 0xfee5];
	} else if (codepoint >= 0xfeeb && codepoint <= 0xfef4) {
		return unscii_block115[codepoint - 0xfeeb];
	} else {
		fprintf(stderr, "[FBInk] Codepoint U+%04X is not covered by our font!\n", codepoint);
		return unscii_block1[0];
	}
}

