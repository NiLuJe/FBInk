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

static const char*
    alt_get_bitmap(uint32_t codepoint)
{
	if (codepoint <= 0x195) {
		return alt_block1[codepoint];
	} else if (codepoint >= 0x197 && codepoint <= 0x19f) {
		return alt_block2[codepoint - 0x197];
	} else if (codepoint >= 0x1cd && codepoint <= 0x1e3) {
		return alt_block3[codepoint - 0x1cd];
	} else if (codepoint >= 0x1e6 && codepoint <= 0x1e8) {
		return alt_block4[codepoint - 0x1e6];
	} else if (codepoint >= 0x1f0 && codepoint <= 0x1f0) {
		return alt_block5[codepoint - 0x1f0];
	} else if (codepoint >= 0x1f4 && codepoint <= 0x1f5) {
		return alt_block6[codepoint - 0x1f4];
	} else if (codepoint >= 0x1f8 && codepoint <= 0x21b) {
		return alt_block7[codepoint - 0x1f8];
	} else if (codepoint >= 0x226 && codepoint <= 0x227) {
		return alt_block8[codepoint - 0x226];
	} else if (codepoint >= 0x22a && codepoint <= 0x233) {
		return alt_block9[codepoint - 0x22a];
	} else if (codepoint >= 0x258 && codepoint <= 0x259) {
		return alt_block10[codepoint - 0x258];
	} else if (codepoint >= 0x263 && codepoint <= 0x263) {
		return alt_block11[codepoint - 0x263];
	} else if (codepoint >= 0x294 && codepoint <= 0x294) {
		return alt_block12[codepoint - 0x294];
	} else if (codepoint >= 0x2c6 && codepoint <= 0x2c6) {
		return alt_block13[codepoint - 0x2c6];
	} else if (codepoint >= 0x2c9 && codepoint <= 0x2c9) {
		return alt_block14[codepoint - 0x2c9];
	} else if (codepoint >= 0x2cd && codepoint <= 0x2cd) {
		return alt_block15[codepoint - 0x2cd];
	} else if (codepoint >= 0x2dc && codepoint <= 0x2dc) {
		return alt_block16[codepoint - 0x2dc];
	} else if (codepoint >= 0x34f && codepoint <= 0x34f) {
		return alt_block17[codepoint - 0x34f];
	} else if (codepoint >= 0x391 && codepoint <= 0x3a1) {
		return alt_block18[codepoint - 0x391];
	} else if (codepoint >= 0x3a3 && codepoint <= 0x3a9) {
		return alt_block19[codepoint - 0x3a3];
	} else if (codepoint >= 0x3b1 && codepoint <= 0x3c9) {
		return alt_block20[codepoint - 0x3b1];
	} else if (codepoint >= 0x400 && codepoint <= 0x401) {
		return alt_block21[codepoint - 0x400];
	} else if (codepoint >= 0x405 && codepoint <= 0x408) {
		return alt_block22[codepoint - 0x405];
	} else if (codepoint >= 0x410 && codepoint <= 0x451) {
		return alt_block23[codepoint - 0x410];
	} else if (codepoint >= 0x455 && codepoint <= 0x458) {
		return alt_block24[codepoint - 0x455];
	} else if (codepoint >= 0x4d0 && codepoint <= 0x4d7) {
		return alt_block25[codepoint - 0x4d0];
	} else if (codepoint >= 0x4e6 && codepoint <= 0x4e7) {
		return alt_block26[codepoint - 0x4e6];
	} else if (codepoint >= 0x4f1 && codepoint <= 0x4f1) {
		return alt_block27[codepoint - 0x4f1];
	} else if (codepoint >= 0x5d0 && codepoint <= 0x5ea) {
		return alt_block28[codepoint - 0x5d0];
	} else if (codepoint >= 0x623 && codepoint <= 0x623) {
		return alt_block29[codepoint - 0x623];
	} else if (codepoint >= 0x627 && codepoint <= 0x62d) {
		return alt_block30[codepoint - 0x627];
	} else if (codepoint >= 0x62f && codepoint <= 0x635) {
		return alt_block31[codepoint - 0x62f];
	} else if (codepoint >= 0x637 && codepoint <= 0x63a) {
		return alt_block32[codepoint - 0x637];
	} else if (codepoint >= 0x640 && codepoint <= 0x64a) {
		return alt_block33[codepoint - 0x640];
	} else if (codepoint >= 0x671 && codepoint <= 0x673) {
		return alt_block34[codepoint - 0x671];
	} else if (codepoint >= 0x1560 && codepoint <= 0x1563) {
		return alt_block35[codepoint - 0x1560];
	} else if (codepoint >= 0x156a && codepoint <= 0x156a) {
		return alt_block36[codepoint - 0x156a];
	} else if (codepoint >= 0x156c && codepoint <= 0x1571) {
		return alt_block37[codepoint - 0x156c];
	} else if (codepoint >= 0x15ec && codepoint <= 0x15ed) {
		return alt_block38[codepoint - 0x15ec];
	} else if (codepoint >= 0x15ef && codepoint <= 0x15ef) {
		return alt_block39[codepoint - 0x15ef];
	} else if (codepoint >= 0x16a0 && codepoint <= 0x16ab) {
		return alt_block40[codepoint - 0x16a0];
	} else if (codepoint >= 0x16b1 && codepoint <= 0x16f8) {
		return alt_block41[codepoint - 0x16b1];
	} else if (codepoint >= 0x1e00 && codepoint <= 0x1e02) {
		return alt_block42[codepoint - 0x1e00];
	} else if (codepoint >= 0x1e04 && codepoint <= 0x1e07) {
		return alt_block43[codepoint - 0x1e04];
	} else if (codepoint >= 0x1e0a && codepoint <= 0x1e0a) {
		return alt_block44[codepoint - 0x1e0a];
	} else if (codepoint >= 0x1e0c && codepoint <= 0x1e0f) {
		return alt_block45[codepoint - 0x1e0c];
	} else if (codepoint >= 0x1e41 && codepoint <= 0x1e4b) {
		return alt_block46[codepoint - 0x1e41];
	} else if (codepoint >= 0x1e58 && codepoint <= 0x1e5b) {
		return alt_block47[codepoint - 0x1e58];
	} else if (codepoint >= 0x1e5e && codepoint <= 0x1e5e) {
		return alt_block48[codepoint - 0x1e5e];
	} else if (codepoint >= 0x2010 && codepoint <= 0x2019) {
		return alt_block49[codepoint - 0x2010];
	} else if (codepoint >= 0x201c && codepoint <= 0x201d) {
		return alt_block50[codepoint - 0x201c];
	} else if (codepoint >= 0x2020 && codepoint <= 0x2022) {
		return alt_block51[codepoint - 0x2020];
	} else if (codepoint >= 0x2024 && codepoint <= 0x2026) {
		return alt_block52[codepoint - 0x2024];
	} else if (codepoint >= 0x2030 && codepoint <= 0x2031) {
		return alt_block53[codepoint - 0x2030];
	} else if (codepoint >= 0x203c && codepoint <= 0x203c) {
		return alt_block54[codepoint - 0x203c];
	} else if (codepoint >= 0x2047 && codepoint <= 0x2049) {
		return alt_block55[codepoint - 0x2047];
	} else if (codepoint >= 0x2070 && codepoint <= 0x2070) {
		return alt_block56[codepoint - 0x2070];
	} else if (codepoint >= 0x2074 && codepoint <= 0x2079) {
		return alt_block57[codepoint - 0x2074];
	} else if (codepoint >= 0x207f && codepoint <= 0x207f) {
		return alt_block58[codepoint - 0x207f];
	} else if (codepoint >= 0x20a7 && codepoint <= 0x20a7) {
		return alt_block59[codepoint - 0x20a7];
	} else if (codepoint >= 0x20ac && codepoint <= 0x20ac) {
		return alt_block60[codepoint - 0x20ac];
	} else if (codepoint >= 0x2117 && codepoint <= 0x2117) {
		return alt_block61[codepoint - 0x2117];
	} else if (codepoint >= 0x2120 && codepoint <= 0x2120) {
		return alt_block62[codepoint - 0x2120];
	} else if (codepoint >= 0x2122 && codepoint <= 0x2122) {
		return alt_block63[codepoint - 0x2122];
	} else if (codepoint >= 0x2190 && codepoint <= 0x2192) {
		return alt_block64[codepoint - 0x2190];
	} else if (codepoint >= 0x2194 && codepoint <= 0x2194) {
		return alt_block65[codepoint - 0x2194];
	} else if (codepoint >= 0x2196 && codepoint <= 0x2199) {
		return alt_block66[codepoint - 0x2196];
	} else if (codepoint >= 0x21a5 && codepoint <= 0x21a5) {
		return alt_block67[codepoint - 0x21a5];
	} else if (codepoint >= 0x21a8 && codepoint <= 0x21a8) {
		return alt_block68[codepoint - 0x21a8];
	} else if (codepoint >= 0x21e6 && codepoint <= 0x21e9) {
		return alt_block69[codepoint - 0x21e6];
	} else if (codepoint >= 0x2212 && codepoint <= 0x2212) {
		return alt_block70[codepoint - 0x2212];
	} else if (codepoint >= 0x2218 && codepoint <= 0x2218) {
		return alt_block71[codepoint - 0x2218];
	} else if (codepoint >= 0x221a && codepoint <= 0x221a) {
		return alt_block72[codepoint - 0x221a];
	} else if (codepoint >= 0x221e && codepoint <= 0x221f) {
		return alt_block73[codepoint - 0x221e];
	} else if (codepoint >= 0x2229 && codepoint <= 0x2229) {
		return alt_block74[codepoint - 0x2229];
	} else if (codepoint >= 0x2248 && codepoint <= 0x2248) {
		return alt_block75[codepoint - 0x2248];
	} else if (codepoint >= 0x2261 && codepoint <= 0x2261) {
		return alt_block76[codepoint - 0x2261];
	} else if (codepoint >= 0x2264 && codepoint <= 0x2265) {
		return alt_block77[codepoint - 0x2264];
	} else if (codepoint >= 0x22c5 && codepoint <= 0x22c5) {
		return alt_block78[codepoint - 0x22c5];
	} else if (codepoint >= 0x2320 && codepoint <= 0x2321) {
		return alt_block79[codepoint - 0x2320];
	} else if (codepoint >= 0x23ea && codepoint <= 0x23ec) {
		return alt_block80[codepoint - 0x23ea];
	} else if (codepoint >= 0x2500 && codepoint <= 0x25ff) {
		return alt_block81[codepoint - 0x2500];
	} else if (codepoint >= 0x2625 && codepoint <= 0x2625) {
		return alt_block82[codepoint - 0x2625];
	} else if (codepoint >= 0x2628 && codepoint <= 0x2628) {
		return alt_block83[codepoint - 0x2628];
	} else if (codepoint >= 0x262f && codepoint <= 0x2637) {
		return alt_block84[codepoint - 0x262f];
	} else if (codepoint >= 0x2639 && codepoint <= 0x263b) {
		return alt_block85[codepoint - 0x2639];
	} else if (codepoint >= 0x2660 && codepoint <= 0x2667) {
		return alt_block86[codepoint - 0x2660];
	} else if (codepoint >= 0x2669 && codepoint <= 0x266c) {
		return alt_block87[codepoint - 0x2669];
	} else if (codepoint >= 0x268a && codepoint <= 0x268f) {
		return alt_block88[codepoint - 0x268a];
	} else if (codepoint >= 0x26aa && codepoint <= 0x26ac) {
		return alt_block89[codepoint - 0x26aa];
	} else if (codepoint >= 0x2708 && codepoint <= 0x2708) {
		return alt_block90[codepoint - 0x2708];
	} else if (codepoint >= 0x2734 && codepoint <= 0x2734) {
		return alt_block91[codepoint - 0x2734];
	} else if (codepoint >= 0x2800 && codepoint <= 0x28ff) {
		return alt_block92[codepoint - 0x2800];
	} else if (codepoint >= 0x2913 && codepoint <= 0x2913) {
		return alt_block93[codepoint - 0x2913];
	} else if (codepoint >= 0x2b1d && codepoint <= 0x2b1d) {
		return alt_block94[codepoint - 0x2b1d];
	} else if (codepoint >= 0x2b24 && codepoint <= 0x2b24) {
		return alt_block95[codepoint - 0x2b24];
	} else if (codepoint >= 0x2b55 && codepoint <= 0x2b55) {
		return alt_block96[codepoint - 0x2b55];
	} else if (codepoint >= 0x2b58 && codepoint <= 0x2b58) {
		return alt_block97[codepoint - 0x2b58];
	} else if (codepoint >= 0x2e2e && codepoint <= 0x2e2e) {
		return alt_block98[codepoint - 0x2e2e];
	} else if (codepoint >= 0xe080 && codepoint <= 0xe19b) {
		return alt_block99[codepoint - 0xe080];
	} else if (codepoint >= 0xe800 && codepoint <= 0xe850) {
		return alt_block100[codepoint - 0xe800];
	} else if (codepoint >= 0xec00 && codepoint <= 0xec26) {
		return alt_block101[codepoint - 0xec00];
	} else if (codepoint >= 0xfe81 && codepoint <= 0xfe82) {
		return alt_block102[codepoint - 0xfe81];
	} else if (codepoint >= 0xfe84 && codepoint <= 0xfe84) {
		return alt_block103[codepoint - 0xfe84];
	} else if (codepoint >= 0xfe87 && codepoint <= 0xfe87) {
		return alt_block104[codepoint - 0xfe87];
	} else if (codepoint >= 0xfe8f && codepoint <= 0xfe9c) {
		return alt_block105[codepoint - 0xfe8f];
	} else if (codepoint >= 0xfe9e && codepoint <= 0xfea4) {
		return alt_block106[codepoint - 0xfe9e];
	} else if (codepoint >= 0xfea9 && codepoint <= 0xfead) {
		return alt_block107[codepoint - 0xfea9];
	} else if (codepoint >= 0xfeaf && codepoint <= 0xfeaf) {
		return alt_block108[codepoint - 0xfeaf];
	} else if (codepoint >= 0xfeb1 && codepoint <= 0xfeb7) {
		return alt_block109[codepoint - 0xfeb1];
	} else if (codepoint >= 0xfeb9 && codepoint <= 0xfebc) {
		return alt_block110[codepoint - 0xfeb9];
	} else if (codepoint >= 0xfec1 && codepoint <= 0xfec6) {
		return alt_block111[codepoint - 0xfec1];
	} else if (codepoint >= 0xfec9 && codepoint <= 0xfedd) {
		return alt_block112[codepoint - 0xfec9];
	} else if (codepoint >= 0xfedf && codepoint <= 0xfee3) {
		return alt_block113[codepoint - 0xfedf];
	} else if (codepoint >= 0xfee5 && codepoint <= 0xfee9) {
		return alt_block114[codepoint - 0xfee5];
	} else if (codepoint >= 0xfeeb && codepoint <= 0xfef4) {
		return alt_block115[codepoint - 0xfeeb];
	} else {
		fprintf(stderr, "[FBInk] Codepoint U+%04X is not covered by our font!\n", codepoint);
		return alt_block1[0];
	}
}

static const char*
    thin_get_bitmap(uint32_t codepoint)
{
	if (codepoint <= 0x195) {
		return thin_block1[codepoint];
	} else if (codepoint >= 0x197 && codepoint <= 0x19f) {
		return thin_block2[codepoint - 0x197];
	} else if (codepoint >= 0x1cd && codepoint <= 0x1e3) {
		return thin_block3[codepoint - 0x1cd];
	} else if (codepoint >= 0x1e6 && codepoint <= 0x1e8) {
		return thin_block4[codepoint - 0x1e6];
	} else if (codepoint >= 0x1f0 && codepoint <= 0x1f0) {
		return thin_block5[codepoint - 0x1f0];
	} else if (codepoint >= 0x1f4 && codepoint <= 0x1f5) {
		return thin_block6[codepoint - 0x1f4];
	} else if (codepoint >= 0x1f8 && codepoint <= 0x21b) {
		return thin_block7[codepoint - 0x1f8];
	} else if (codepoint >= 0x226 && codepoint <= 0x227) {
		return thin_block8[codepoint - 0x226];
	} else if (codepoint >= 0x22a && codepoint <= 0x233) {
		return thin_block9[codepoint - 0x22a];
	} else if (codepoint >= 0x258 && codepoint <= 0x259) {
		return thin_block10[codepoint - 0x258];
	} else if (codepoint >= 0x263 && codepoint <= 0x263) {
		return thin_block11[codepoint - 0x263];
	} else if (codepoint >= 0x294 && codepoint <= 0x294) {
		return thin_block12[codepoint - 0x294];
	} else if (codepoint >= 0x2c6 && codepoint <= 0x2c6) {
		return thin_block13[codepoint - 0x2c6];
	} else if (codepoint >= 0x2c9 && codepoint <= 0x2c9) {
		return thin_block14[codepoint - 0x2c9];
	} else if (codepoint >= 0x2cd && codepoint <= 0x2cd) {
		return thin_block15[codepoint - 0x2cd];
	} else if (codepoint >= 0x2dc && codepoint <= 0x2dc) {
		return thin_block16[codepoint - 0x2dc];
	} else if (codepoint >= 0x34f && codepoint <= 0x34f) {
		return thin_block17[codepoint - 0x34f];
	} else if (codepoint >= 0x391 && codepoint <= 0x3a1) {
		return thin_block18[codepoint - 0x391];
	} else if (codepoint >= 0x3a3 && codepoint <= 0x3a9) {
		return thin_block19[codepoint - 0x3a3];
	} else if (codepoint >= 0x3b1 && codepoint <= 0x3c9) {
		return thin_block20[codepoint - 0x3b1];
	} else if (codepoint >= 0x400 && codepoint <= 0x401) {
		return thin_block21[codepoint - 0x400];
	} else if (codepoint >= 0x405 && codepoint <= 0x408) {
		return thin_block22[codepoint - 0x405];
	} else if (codepoint >= 0x410 && codepoint <= 0x451) {
		return thin_block23[codepoint - 0x410];
	} else if (codepoint >= 0x455 && codepoint <= 0x458) {
		return thin_block24[codepoint - 0x455];
	} else if (codepoint >= 0x4d0 && codepoint <= 0x4d7) {
		return thin_block25[codepoint - 0x4d0];
	} else if (codepoint >= 0x4e6 && codepoint <= 0x4e7) {
		return thin_block26[codepoint - 0x4e6];
	} else if (codepoint >= 0x4f1 && codepoint <= 0x4f1) {
		return thin_block27[codepoint - 0x4f1];
	} else if (codepoint >= 0x5d0 && codepoint <= 0x5ea) {
		return thin_block28[codepoint - 0x5d0];
	} else if (codepoint >= 0x623 && codepoint <= 0x623) {
		return thin_block29[codepoint - 0x623];
	} else if (codepoint >= 0x627 && codepoint <= 0x62d) {
		return thin_block30[codepoint - 0x627];
	} else if (codepoint >= 0x62f && codepoint <= 0x635) {
		return thin_block31[codepoint - 0x62f];
	} else if (codepoint >= 0x637 && codepoint <= 0x63a) {
		return thin_block32[codepoint - 0x637];
	} else if (codepoint >= 0x640 && codepoint <= 0x64a) {
		return thin_block33[codepoint - 0x640];
	} else if (codepoint >= 0x671 && codepoint <= 0x673) {
		return thin_block34[codepoint - 0x671];
	} else if (codepoint >= 0x1560 && codepoint <= 0x1563) {
		return thin_block35[codepoint - 0x1560];
	} else if (codepoint >= 0x156a && codepoint <= 0x156a) {
		return thin_block36[codepoint - 0x156a];
	} else if (codepoint >= 0x156c && codepoint <= 0x1571) {
		return thin_block37[codepoint - 0x156c];
	} else if (codepoint >= 0x15ec && codepoint <= 0x15ed) {
		return thin_block38[codepoint - 0x15ec];
	} else if (codepoint >= 0x15ef && codepoint <= 0x15ef) {
		return thin_block39[codepoint - 0x15ef];
	} else if (codepoint >= 0x16a0 && codepoint <= 0x16ab) {
		return thin_block40[codepoint - 0x16a0];
	} else if (codepoint >= 0x16b1 && codepoint <= 0x16f8) {
		return thin_block41[codepoint - 0x16b1];
	} else if (codepoint >= 0x1e00 && codepoint <= 0x1e02) {
		return thin_block42[codepoint - 0x1e00];
	} else if (codepoint >= 0x1e04 && codepoint <= 0x1e07) {
		return thin_block43[codepoint - 0x1e04];
	} else if (codepoint >= 0x1e0a && codepoint <= 0x1e0a) {
		return thin_block44[codepoint - 0x1e0a];
	} else if (codepoint >= 0x1e0c && codepoint <= 0x1e0f) {
		return thin_block45[codepoint - 0x1e0c];
	} else if (codepoint >= 0x1e41 && codepoint <= 0x1e4b) {
		return thin_block46[codepoint - 0x1e41];
	} else if (codepoint >= 0x1e58 && codepoint <= 0x1e5b) {
		return thin_block47[codepoint - 0x1e58];
	} else if (codepoint >= 0x1e5e && codepoint <= 0x1e5e) {
		return thin_block48[codepoint - 0x1e5e];
	} else if (codepoint >= 0x2010 && codepoint <= 0x2019) {
		return thin_block49[codepoint - 0x2010];
	} else if (codepoint >= 0x201c && codepoint <= 0x201d) {
		return thin_block50[codepoint - 0x201c];
	} else if (codepoint >= 0x2020 && codepoint <= 0x2022) {
		return thin_block51[codepoint - 0x2020];
	} else if (codepoint >= 0x2024 && codepoint <= 0x2026) {
		return thin_block52[codepoint - 0x2024];
	} else if (codepoint >= 0x2030 && codepoint <= 0x2031) {
		return thin_block53[codepoint - 0x2030];
	} else if (codepoint >= 0x203c && codepoint <= 0x203c) {
		return thin_block54[codepoint - 0x203c];
	} else if (codepoint >= 0x2047 && codepoint <= 0x2049) {
		return thin_block55[codepoint - 0x2047];
	} else if (codepoint >= 0x2070 && codepoint <= 0x2070) {
		return thin_block56[codepoint - 0x2070];
	} else if (codepoint >= 0x2074 && codepoint <= 0x2079) {
		return thin_block57[codepoint - 0x2074];
	} else if (codepoint >= 0x207f && codepoint <= 0x207f) {
		return thin_block58[codepoint - 0x207f];
	} else if (codepoint >= 0x20a7 && codepoint <= 0x20a7) {
		return thin_block59[codepoint - 0x20a7];
	} else if (codepoint >= 0x20ac && codepoint <= 0x20ac) {
		return thin_block60[codepoint - 0x20ac];
	} else if (codepoint >= 0x2117 && codepoint <= 0x2117) {
		return thin_block61[codepoint - 0x2117];
	} else if (codepoint >= 0x2120 && codepoint <= 0x2120) {
		return thin_block62[codepoint - 0x2120];
	} else if (codepoint >= 0x2122 && codepoint <= 0x2122) {
		return thin_block63[codepoint - 0x2122];
	} else if (codepoint >= 0x2190 && codepoint <= 0x2192) {
		return thin_block64[codepoint - 0x2190];
	} else if (codepoint >= 0x2194 && codepoint <= 0x2194) {
		return thin_block65[codepoint - 0x2194];
	} else if (codepoint >= 0x2196 && codepoint <= 0x2199) {
		return thin_block66[codepoint - 0x2196];
	} else if (codepoint >= 0x21a5 && codepoint <= 0x21a5) {
		return thin_block67[codepoint - 0x21a5];
	} else if (codepoint >= 0x21a8 && codepoint <= 0x21a8) {
		return thin_block68[codepoint - 0x21a8];
	} else if (codepoint >= 0x21e6 && codepoint <= 0x21e9) {
		return thin_block69[codepoint - 0x21e6];
	} else if (codepoint >= 0x2212 && codepoint <= 0x2212) {
		return thin_block70[codepoint - 0x2212];
	} else if (codepoint >= 0x2218 && codepoint <= 0x2218) {
		return thin_block71[codepoint - 0x2218];
	} else if (codepoint >= 0x221a && codepoint <= 0x221a) {
		return thin_block72[codepoint - 0x221a];
	} else if (codepoint >= 0x221e && codepoint <= 0x221f) {
		return thin_block73[codepoint - 0x221e];
	} else if (codepoint >= 0x2229 && codepoint <= 0x2229) {
		return thin_block74[codepoint - 0x2229];
	} else if (codepoint >= 0x2248 && codepoint <= 0x2248) {
		return thin_block75[codepoint - 0x2248];
	} else if (codepoint >= 0x2261 && codepoint <= 0x2261) {
		return thin_block76[codepoint - 0x2261];
	} else if (codepoint >= 0x2264 && codepoint <= 0x2265) {
		return thin_block77[codepoint - 0x2264];
	} else if (codepoint >= 0x22c5 && codepoint <= 0x22c5) {
		return thin_block78[codepoint - 0x22c5];
	} else if (codepoint >= 0x2320 && codepoint <= 0x2321) {
		return thin_block79[codepoint - 0x2320];
	} else if (codepoint >= 0x23ea && codepoint <= 0x23ec) {
		return thin_block80[codepoint - 0x23ea];
	} else if (codepoint >= 0x2500 && codepoint <= 0x25ff) {
		return thin_block81[codepoint - 0x2500];
	} else if (codepoint >= 0x2625 && codepoint <= 0x2625) {
		return thin_block82[codepoint - 0x2625];
	} else if (codepoint >= 0x2628 && codepoint <= 0x2628) {
		return thin_block83[codepoint - 0x2628];
	} else if (codepoint >= 0x262f && codepoint <= 0x2637) {
		return thin_block84[codepoint - 0x262f];
	} else if (codepoint >= 0x2639 && codepoint <= 0x263b) {
		return thin_block85[codepoint - 0x2639];
	} else if (codepoint >= 0x2660 && codepoint <= 0x2667) {
		return thin_block86[codepoint - 0x2660];
	} else if (codepoint >= 0x2669 && codepoint <= 0x266c) {
		return thin_block87[codepoint - 0x2669];
	} else if (codepoint >= 0x268a && codepoint <= 0x268f) {
		return thin_block88[codepoint - 0x268a];
	} else if (codepoint >= 0x26aa && codepoint <= 0x26ac) {
		return thin_block89[codepoint - 0x26aa];
	} else if (codepoint >= 0x2708 && codepoint <= 0x2708) {
		return thin_block90[codepoint - 0x2708];
	} else if (codepoint >= 0x2734 && codepoint <= 0x2734) {
		return thin_block91[codepoint - 0x2734];
	} else if (codepoint >= 0x2800 && codepoint <= 0x28ff) {
		return thin_block92[codepoint - 0x2800];
	} else if (codepoint >= 0x2913 && codepoint <= 0x2913) {
		return thin_block93[codepoint - 0x2913];
	} else if (codepoint >= 0x2b1d && codepoint <= 0x2b1d) {
		return thin_block94[codepoint - 0x2b1d];
	} else if (codepoint >= 0x2b24 && codepoint <= 0x2b24) {
		return thin_block95[codepoint - 0x2b24];
	} else if (codepoint >= 0x2b55 && codepoint <= 0x2b55) {
		return thin_block96[codepoint - 0x2b55];
	} else if (codepoint >= 0x2b58 && codepoint <= 0x2b58) {
		return thin_block97[codepoint - 0x2b58];
	} else if (codepoint >= 0x2e2e && codepoint <= 0x2e2e) {
		return thin_block98[codepoint - 0x2e2e];
	} else if (codepoint >= 0xe080 && codepoint <= 0xe19b) {
		return thin_block99[codepoint - 0xe080];
	} else if (codepoint >= 0xe800 && codepoint <= 0xe850) {
		return thin_block100[codepoint - 0xe800];
	} else if (codepoint >= 0xec00 && codepoint <= 0xec26) {
		return thin_block101[codepoint - 0xec00];
	} else if (codepoint >= 0xfe81 && codepoint <= 0xfe82) {
		return thin_block102[codepoint - 0xfe81];
	} else if (codepoint >= 0xfe84 && codepoint <= 0xfe84) {
		return thin_block103[codepoint - 0xfe84];
	} else if (codepoint >= 0xfe87 && codepoint <= 0xfe87) {
		return thin_block104[codepoint - 0xfe87];
	} else if (codepoint >= 0xfe8f && codepoint <= 0xfe9c) {
		return thin_block105[codepoint - 0xfe8f];
	} else if (codepoint >= 0xfe9e && codepoint <= 0xfea4) {
		return thin_block106[codepoint - 0xfe9e];
	} else if (codepoint >= 0xfea9 && codepoint <= 0xfead) {
		return thin_block107[codepoint - 0xfea9];
	} else if (codepoint >= 0xfeaf && codepoint <= 0xfeaf) {
		return thin_block108[codepoint - 0xfeaf];
	} else if (codepoint >= 0xfeb1 && codepoint <= 0xfeb7) {
		return thin_block109[codepoint - 0xfeb1];
	} else if (codepoint >= 0xfeb9 && codepoint <= 0xfebc) {
		return thin_block110[codepoint - 0xfeb9];
	} else if (codepoint >= 0xfec1 && codepoint <= 0xfec6) {
		return thin_block111[codepoint - 0xfec1];
	} else if (codepoint >= 0xfec9 && codepoint <= 0xfedd) {
		return thin_block112[codepoint - 0xfec9];
	} else if (codepoint >= 0xfedf && codepoint <= 0xfee3) {
		return thin_block113[codepoint - 0xfedf];
	} else if (codepoint >= 0xfee5 && codepoint <= 0xfee9) {
		return thin_block114[codepoint - 0xfee5];
	} else if (codepoint >= 0xfeeb && codepoint <= 0xfef4) {
		return thin_block115[codepoint - 0xfeeb];
	} else {
		fprintf(stderr, "[FBInk] Codepoint U+%04X is not covered by our font!\n", codepoint);
		return thin_block1[0];
	}
}

static const char*
    fantasy_get_bitmap(uint32_t codepoint)
{
	if (codepoint <= 0x195) {
		return fantasy_block1[codepoint];
	} else if (codepoint >= 0x197 && codepoint <= 0x19f) {
		return fantasy_block2[codepoint - 0x197];
	} else if (codepoint >= 0x1cd && codepoint <= 0x1e3) {
		return fantasy_block3[codepoint - 0x1cd];
	} else if (codepoint >= 0x1e6 && codepoint <= 0x1e8) {
		return fantasy_block4[codepoint - 0x1e6];
	} else if (codepoint >= 0x1f0 && codepoint <= 0x1f0) {
		return fantasy_block5[codepoint - 0x1f0];
	} else if (codepoint >= 0x1f4 && codepoint <= 0x1f5) {
		return fantasy_block6[codepoint - 0x1f4];
	} else if (codepoint >= 0x1f8 && codepoint <= 0x21b) {
		return fantasy_block7[codepoint - 0x1f8];
	} else if (codepoint >= 0x226 && codepoint <= 0x227) {
		return fantasy_block8[codepoint - 0x226];
	} else if (codepoint >= 0x22a && codepoint <= 0x233) {
		return fantasy_block9[codepoint - 0x22a];
	} else if (codepoint >= 0x258 && codepoint <= 0x259) {
		return fantasy_block10[codepoint - 0x258];
	} else if (codepoint >= 0x263 && codepoint <= 0x263) {
		return fantasy_block11[codepoint - 0x263];
	} else if (codepoint >= 0x294 && codepoint <= 0x294) {
		return fantasy_block12[codepoint - 0x294];
	} else if (codepoint >= 0x2c6 && codepoint <= 0x2c6) {
		return fantasy_block13[codepoint - 0x2c6];
	} else if (codepoint >= 0x2c9 && codepoint <= 0x2c9) {
		return fantasy_block14[codepoint - 0x2c9];
	} else if (codepoint >= 0x2cd && codepoint <= 0x2cd) {
		return fantasy_block15[codepoint - 0x2cd];
	} else if (codepoint >= 0x2dc && codepoint <= 0x2dc) {
		return fantasy_block16[codepoint - 0x2dc];
	} else if (codepoint >= 0x34f && codepoint <= 0x34f) {
		return fantasy_block17[codepoint - 0x34f];
	} else if (codepoint >= 0x391 && codepoint <= 0x3a1) {
		return fantasy_block18[codepoint - 0x391];
	} else if (codepoint >= 0x3a3 && codepoint <= 0x3a9) {
		return fantasy_block19[codepoint - 0x3a3];
	} else if (codepoint >= 0x3b1 && codepoint <= 0x3c9) {
		return fantasy_block20[codepoint - 0x3b1];
	} else if (codepoint >= 0x400 && codepoint <= 0x401) {
		return fantasy_block21[codepoint - 0x400];
	} else if (codepoint >= 0x405 && codepoint <= 0x408) {
		return fantasy_block22[codepoint - 0x405];
	} else if (codepoint >= 0x410 && codepoint <= 0x451) {
		return fantasy_block23[codepoint - 0x410];
	} else if (codepoint >= 0x455 && codepoint <= 0x458) {
		return fantasy_block24[codepoint - 0x455];
	} else if (codepoint >= 0x4d0 && codepoint <= 0x4d7) {
		return fantasy_block25[codepoint - 0x4d0];
	} else if (codepoint >= 0x4e6 && codepoint <= 0x4e7) {
		return fantasy_block26[codepoint - 0x4e6];
	} else if (codepoint >= 0x4f1 && codepoint <= 0x4f1) {
		return fantasy_block27[codepoint - 0x4f1];
	} else if (codepoint >= 0x5d0 && codepoint <= 0x5ea) {
		return fantasy_block28[codepoint - 0x5d0];
	} else if (codepoint >= 0x623 && codepoint <= 0x623) {
		return fantasy_block29[codepoint - 0x623];
	} else if (codepoint >= 0x627 && codepoint <= 0x62d) {
		return fantasy_block30[codepoint - 0x627];
	} else if (codepoint >= 0x62f && codepoint <= 0x635) {
		return fantasy_block31[codepoint - 0x62f];
	} else if (codepoint >= 0x637 && codepoint <= 0x63a) {
		return fantasy_block32[codepoint - 0x637];
	} else if (codepoint >= 0x640 && codepoint <= 0x64a) {
		return fantasy_block33[codepoint - 0x640];
	} else if (codepoint >= 0x671 && codepoint <= 0x673) {
		return fantasy_block34[codepoint - 0x671];
	} else if (codepoint >= 0x1560 && codepoint <= 0x1563) {
		return fantasy_block35[codepoint - 0x1560];
	} else if (codepoint >= 0x156a && codepoint <= 0x156a) {
		return fantasy_block36[codepoint - 0x156a];
	} else if (codepoint >= 0x156c && codepoint <= 0x1571) {
		return fantasy_block37[codepoint - 0x156c];
	} else if (codepoint >= 0x15ec && codepoint <= 0x15ed) {
		return fantasy_block38[codepoint - 0x15ec];
	} else if (codepoint >= 0x15ef && codepoint <= 0x15ef) {
		return fantasy_block39[codepoint - 0x15ef];
	} else if (codepoint >= 0x16a0 && codepoint <= 0x16ab) {
		return fantasy_block40[codepoint - 0x16a0];
	} else if (codepoint >= 0x16b1 && codepoint <= 0x16f8) {
		return fantasy_block41[codepoint - 0x16b1];
	} else if (codepoint >= 0x1e00 && codepoint <= 0x1e02) {
		return fantasy_block42[codepoint - 0x1e00];
	} else if (codepoint >= 0x1e04 && codepoint <= 0x1e07) {
		return fantasy_block43[codepoint - 0x1e04];
	} else if (codepoint >= 0x1e0a && codepoint <= 0x1e0a) {
		return fantasy_block44[codepoint - 0x1e0a];
	} else if (codepoint >= 0x1e0c && codepoint <= 0x1e0f) {
		return fantasy_block45[codepoint - 0x1e0c];
	} else if (codepoint >= 0x1e41 && codepoint <= 0x1e4b) {
		return fantasy_block46[codepoint - 0x1e41];
	} else if (codepoint >= 0x1e58 && codepoint <= 0x1e5b) {
		return fantasy_block47[codepoint - 0x1e58];
	} else if (codepoint >= 0x1e5e && codepoint <= 0x1e5e) {
		return fantasy_block48[codepoint - 0x1e5e];
	} else if (codepoint >= 0x2010 && codepoint <= 0x2019) {
		return fantasy_block49[codepoint - 0x2010];
	} else if (codepoint >= 0x201c && codepoint <= 0x201d) {
		return fantasy_block50[codepoint - 0x201c];
	} else if (codepoint >= 0x2020 && codepoint <= 0x2022) {
		return fantasy_block51[codepoint - 0x2020];
	} else if (codepoint >= 0x2024 && codepoint <= 0x2026) {
		return fantasy_block52[codepoint - 0x2024];
	} else if (codepoint >= 0x2030 && codepoint <= 0x2031) {
		return fantasy_block53[codepoint - 0x2030];
	} else if (codepoint >= 0x203c && codepoint <= 0x203c) {
		return fantasy_block54[codepoint - 0x203c];
	} else if (codepoint >= 0x2047 && codepoint <= 0x2049) {
		return fantasy_block55[codepoint - 0x2047];
	} else if (codepoint >= 0x2070 && codepoint <= 0x2070) {
		return fantasy_block56[codepoint - 0x2070];
	} else if (codepoint >= 0x2074 && codepoint <= 0x2079) {
		return fantasy_block57[codepoint - 0x2074];
	} else if (codepoint >= 0x207f && codepoint <= 0x207f) {
		return fantasy_block58[codepoint - 0x207f];
	} else if (codepoint >= 0x20a7 && codepoint <= 0x20a7) {
		return fantasy_block59[codepoint - 0x20a7];
	} else if (codepoint >= 0x20ac && codepoint <= 0x20ac) {
		return fantasy_block60[codepoint - 0x20ac];
	} else if (codepoint >= 0x2117 && codepoint <= 0x2117) {
		return fantasy_block61[codepoint - 0x2117];
	} else if (codepoint >= 0x2120 && codepoint <= 0x2120) {
		return fantasy_block62[codepoint - 0x2120];
	} else if (codepoint >= 0x2122 && codepoint <= 0x2122) {
		return fantasy_block63[codepoint - 0x2122];
	} else if (codepoint >= 0x2190 && codepoint <= 0x2192) {
		return fantasy_block64[codepoint - 0x2190];
	} else if (codepoint >= 0x2194 && codepoint <= 0x2194) {
		return fantasy_block65[codepoint - 0x2194];
	} else if (codepoint >= 0x2196 && codepoint <= 0x2199) {
		return fantasy_block66[codepoint - 0x2196];
	} else if (codepoint >= 0x21a5 && codepoint <= 0x21a5) {
		return fantasy_block67[codepoint - 0x21a5];
	} else if (codepoint >= 0x21a8 && codepoint <= 0x21a8) {
		return fantasy_block68[codepoint - 0x21a8];
	} else if (codepoint >= 0x21e6 && codepoint <= 0x21e9) {
		return fantasy_block69[codepoint - 0x21e6];
	} else if (codepoint >= 0x2212 && codepoint <= 0x2212) {
		return fantasy_block70[codepoint - 0x2212];
	} else if (codepoint >= 0x2218 && codepoint <= 0x2218) {
		return fantasy_block71[codepoint - 0x2218];
	} else if (codepoint >= 0x221a && codepoint <= 0x221a) {
		return fantasy_block72[codepoint - 0x221a];
	} else if (codepoint >= 0x221e && codepoint <= 0x221f) {
		return fantasy_block73[codepoint - 0x221e];
	} else if (codepoint >= 0x2229 && codepoint <= 0x2229) {
		return fantasy_block74[codepoint - 0x2229];
	} else if (codepoint >= 0x2248 && codepoint <= 0x2248) {
		return fantasy_block75[codepoint - 0x2248];
	} else if (codepoint >= 0x2261 && codepoint <= 0x2261) {
		return fantasy_block76[codepoint - 0x2261];
	} else if (codepoint >= 0x2264 && codepoint <= 0x2265) {
		return fantasy_block77[codepoint - 0x2264];
	} else if (codepoint >= 0x22c5 && codepoint <= 0x22c5) {
		return fantasy_block78[codepoint - 0x22c5];
	} else if (codepoint >= 0x2320 && codepoint <= 0x2321) {
		return fantasy_block79[codepoint - 0x2320];
	} else if (codepoint >= 0x23ea && codepoint <= 0x23ec) {
		return fantasy_block80[codepoint - 0x23ea];
	} else if (codepoint >= 0x2500 && codepoint <= 0x25ff) {
		return fantasy_block81[codepoint - 0x2500];
	} else if (codepoint >= 0x2625 && codepoint <= 0x2625) {
		return fantasy_block82[codepoint - 0x2625];
	} else if (codepoint >= 0x2628 && codepoint <= 0x2628) {
		return fantasy_block83[codepoint - 0x2628];
	} else if (codepoint >= 0x262f && codepoint <= 0x2637) {
		return fantasy_block84[codepoint - 0x262f];
	} else if (codepoint >= 0x2639 && codepoint <= 0x263b) {
		return fantasy_block85[codepoint - 0x2639];
	} else if (codepoint >= 0x2660 && codepoint <= 0x2667) {
		return fantasy_block86[codepoint - 0x2660];
	} else if (codepoint >= 0x2669 && codepoint <= 0x266c) {
		return fantasy_block87[codepoint - 0x2669];
	} else if (codepoint >= 0x268a && codepoint <= 0x268f) {
		return fantasy_block88[codepoint - 0x268a];
	} else if (codepoint >= 0x26aa && codepoint <= 0x26ac) {
		return fantasy_block89[codepoint - 0x26aa];
	} else if (codepoint >= 0x2708 && codepoint <= 0x2708) {
		return fantasy_block90[codepoint - 0x2708];
	} else if (codepoint >= 0x2734 && codepoint <= 0x2734) {
		return fantasy_block91[codepoint - 0x2734];
	} else if (codepoint >= 0x2800 && codepoint <= 0x28ff) {
		return fantasy_block92[codepoint - 0x2800];
	} else if (codepoint >= 0x2913 && codepoint <= 0x2913) {
		return fantasy_block93[codepoint - 0x2913];
	} else if (codepoint >= 0x2b1d && codepoint <= 0x2b1d) {
		return fantasy_block94[codepoint - 0x2b1d];
	} else if (codepoint >= 0x2b24 && codepoint <= 0x2b24) {
		return fantasy_block95[codepoint - 0x2b24];
	} else if (codepoint >= 0x2b55 && codepoint <= 0x2b55) {
		return fantasy_block96[codepoint - 0x2b55];
	} else if (codepoint >= 0x2b58 && codepoint <= 0x2b58) {
		return fantasy_block97[codepoint - 0x2b58];
	} else if (codepoint >= 0x2e2e && codepoint <= 0x2e2e) {
		return fantasy_block98[codepoint - 0x2e2e];
	} else if (codepoint >= 0xe080 && codepoint <= 0xe19b) {
		return fantasy_block99[codepoint - 0xe080];
	} else if (codepoint >= 0xe800 && codepoint <= 0xe850) {
		return fantasy_block100[codepoint - 0xe800];
	} else if (codepoint >= 0xec00 && codepoint <= 0xec26) {
		return fantasy_block101[codepoint - 0xec00];
	} else if (codepoint >= 0xfe81 && codepoint <= 0xfe82) {
		return fantasy_block102[codepoint - 0xfe81];
	} else if (codepoint >= 0xfe84 && codepoint <= 0xfe84) {
		return fantasy_block103[codepoint - 0xfe84];
	} else if (codepoint >= 0xfe87 && codepoint <= 0xfe87) {
		return fantasy_block104[codepoint - 0xfe87];
	} else if (codepoint >= 0xfe8f && codepoint <= 0xfe9c) {
		return fantasy_block105[codepoint - 0xfe8f];
	} else if (codepoint >= 0xfe9e && codepoint <= 0xfea4) {
		return fantasy_block106[codepoint - 0xfe9e];
	} else if (codepoint >= 0xfea9 && codepoint <= 0xfead) {
		return fantasy_block107[codepoint - 0xfea9];
	} else if (codepoint >= 0xfeaf && codepoint <= 0xfeaf) {
		return fantasy_block108[codepoint - 0xfeaf];
	} else if (codepoint >= 0xfeb1 && codepoint <= 0xfeb7) {
		return fantasy_block109[codepoint - 0xfeb1];
	} else if (codepoint >= 0xfeb9 && codepoint <= 0xfebc) {
		return fantasy_block110[codepoint - 0xfeb9];
	} else if (codepoint >= 0xfec1 && codepoint <= 0xfec6) {
		return fantasy_block111[codepoint - 0xfec1];
	} else if (codepoint >= 0xfec9 && codepoint <= 0xfedd) {
		return fantasy_block112[codepoint - 0xfec9];
	} else if (codepoint >= 0xfedf && codepoint <= 0xfee3) {
		return fantasy_block113[codepoint - 0xfedf];
	} else if (codepoint >= 0xfee5 && codepoint <= 0xfee9) {
		return fantasy_block114[codepoint - 0xfee5];
	} else if (codepoint >= 0xfeeb && codepoint <= 0xfef4) {
		return fantasy_block115[codepoint - 0xfeeb];
	} else {
		fprintf(stderr, "[FBInk] Codepoint U+%04X is not covered by our font!\n", codepoint);
		return fantasy_block1[0];
	}
}

static const char*
    mcr_get_bitmap(uint32_t codepoint)
{
	if (codepoint <= 0x195) {
		return mcr_block1[codepoint];
	} else if (codepoint >= 0x197 && codepoint <= 0x19f) {
		return mcr_block2[codepoint - 0x197];
	} else if (codepoint >= 0x1cd && codepoint <= 0x1e3) {
		return mcr_block3[codepoint - 0x1cd];
	} else if (codepoint >= 0x1e6 && codepoint <= 0x1e8) {
		return mcr_block4[codepoint - 0x1e6];
	} else if (codepoint >= 0x1f0 && codepoint <= 0x1f0) {
		return mcr_block5[codepoint - 0x1f0];
	} else if (codepoint >= 0x1f4 && codepoint <= 0x1f5) {
		return mcr_block6[codepoint - 0x1f4];
	} else if (codepoint >= 0x1f8 && codepoint <= 0x21b) {
		return mcr_block7[codepoint - 0x1f8];
	} else if (codepoint >= 0x226 && codepoint <= 0x227) {
		return mcr_block8[codepoint - 0x226];
	} else if (codepoint >= 0x22a && codepoint <= 0x233) {
		return mcr_block9[codepoint - 0x22a];
	} else if (codepoint >= 0x258 && codepoint <= 0x259) {
		return mcr_block10[codepoint - 0x258];
	} else if (codepoint >= 0x263 && codepoint <= 0x263) {
		return mcr_block11[codepoint - 0x263];
	} else if (codepoint >= 0x294 && codepoint <= 0x294) {
		return mcr_block12[codepoint - 0x294];
	} else if (codepoint >= 0x2c6 && codepoint <= 0x2c6) {
		return mcr_block13[codepoint - 0x2c6];
	} else if (codepoint >= 0x2c9 && codepoint <= 0x2c9) {
		return mcr_block14[codepoint - 0x2c9];
	} else if (codepoint >= 0x2cd && codepoint <= 0x2cd) {
		return mcr_block15[codepoint - 0x2cd];
	} else if (codepoint >= 0x2dc && codepoint <= 0x2dc) {
		return mcr_block16[codepoint - 0x2dc];
	} else if (codepoint >= 0x34f && codepoint <= 0x34f) {
		return mcr_block17[codepoint - 0x34f];
	} else if (codepoint >= 0x391 && codepoint <= 0x3a1) {
		return mcr_block18[codepoint - 0x391];
	} else if (codepoint >= 0x3a3 && codepoint <= 0x3a9) {
		return mcr_block19[codepoint - 0x3a3];
	} else if (codepoint >= 0x3b1 && codepoint <= 0x3c9) {
		return mcr_block20[codepoint - 0x3b1];
	} else if (codepoint >= 0x400 && codepoint <= 0x401) {
		return mcr_block21[codepoint - 0x400];
	} else if (codepoint >= 0x405 && codepoint <= 0x408) {
		return mcr_block22[codepoint - 0x405];
	} else if (codepoint >= 0x410 && codepoint <= 0x451) {
		return mcr_block23[codepoint - 0x410];
	} else if (codepoint >= 0x455 && codepoint <= 0x458) {
		return mcr_block24[codepoint - 0x455];
	} else if (codepoint >= 0x4d0 && codepoint <= 0x4d7) {
		return mcr_block25[codepoint - 0x4d0];
	} else if (codepoint >= 0x4e6 && codepoint <= 0x4e7) {
		return mcr_block26[codepoint - 0x4e6];
	} else if (codepoint >= 0x4f1 && codepoint <= 0x4f1) {
		return mcr_block27[codepoint - 0x4f1];
	} else if (codepoint >= 0x5d0 && codepoint <= 0x5ea) {
		return mcr_block28[codepoint - 0x5d0];
	} else if (codepoint >= 0x623 && codepoint <= 0x623) {
		return mcr_block29[codepoint - 0x623];
	} else if (codepoint >= 0x627 && codepoint <= 0x62d) {
		return mcr_block30[codepoint - 0x627];
	} else if (codepoint >= 0x62f && codepoint <= 0x635) {
		return mcr_block31[codepoint - 0x62f];
	} else if (codepoint >= 0x637 && codepoint <= 0x63a) {
		return mcr_block32[codepoint - 0x637];
	} else if (codepoint >= 0x640 && codepoint <= 0x64a) {
		return mcr_block33[codepoint - 0x640];
	} else if (codepoint >= 0x671 && codepoint <= 0x673) {
		return mcr_block34[codepoint - 0x671];
	} else if (codepoint >= 0x1560 && codepoint <= 0x1563) {
		return mcr_block35[codepoint - 0x1560];
	} else if (codepoint >= 0x156a && codepoint <= 0x156a) {
		return mcr_block36[codepoint - 0x156a];
	} else if (codepoint >= 0x156c && codepoint <= 0x1571) {
		return mcr_block37[codepoint - 0x156c];
	} else if (codepoint >= 0x15ec && codepoint <= 0x15ed) {
		return mcr_block38[codepoint - 0x15ec];
	} else if (codepoint >= 0x15ef && codepoint <= 0x15ef) {
		return mcr_block39[codepoint - 0x15ef];
	} else if (codepoint >= 0x16a0 && codepoint <= 0x16ab) {
		return mcr_block40[codepoint - 0x16a0];
	} else if (codepoint >= 0x16b1 && codepoint <= 0x16f8) {
		return mcr_block41[codepoint - 0x16b1];
	} else if (codepoint >= 0x1e00 && codepoint <= 0x1e02) {
		return mcr_block42[codepoint - 0x1e00];
	} else if (codepoint >= 0x1e04 && codepoint <= 0x1e07) {
		return mcr_block43[codepoint - 0x1e04];
	} else if (codepoint >= 0x1e0a && codepoint <= 0x1e0a) {
		return mcr_block44[codepoint - 0x1e0a];
	} else if (codepoint >= 0x1e0c && codepoint <= 0x1e0f) {
		return mcr_block45[codepoint - 0x1e0c];
	} else if (codepoint >= 0x1e41 && codepoint <= 0x1e4b) {
		return mcr_block46[codepoint - 0x1e41];
	} else if (codepoint >= 0x1e58 && codepoint <= 0x1e5b) {
		return mcr_block47[codepoint - 0x1e58];
	} else if (codepoint >= 0x1e5e && codepoint <= 0x1e5e) {
		return mcr_block48[codepoint - 0x1e5e];
	} else if (codepoint >= 0x2010 && codepoint <= 0x2019) {
		return mcr_block49[codepoint - 0x2010];
	} else if (codepoint >= 0x201c && codepoint <= 0x201d) {
		return mcr_block50[codepoint - 0x201c];
	} else if (codepoint >= 0x2020 && codepoint <= 0x2022) {
		return mcr_block51[codepoint - 0x2020];
	} else if (codepoint >= 0x2024 && codepoint <= 0x2026) {
		return mcr_block52[codepoint - 0x2024];
	} else if (codepoint >= 0x2030 && codepoint <= 0x2031) {
		return mcr_block53[codepoint - 0x2030];
	} else if (codepoint >= 0x203c && codepoint <= 0x203c) {
		return mcr_block54[codepoint - 0x203c];
	} else if (codepoint >= 0x2047 && codepoint <= 0x2049) {
		return mcr_block55[codepoint - 0x2047];
	} else if (codepoint >= 0x2070 && codepoint <= 0x2070) {
		return mcr_block56[codepoint - 0x2070];
	} else if (codepoint >= 0x2074 && codepoint <= 0x2079) {
		return mcr_block57[codepoint - 0x2074];
	} else if (codepoint >= 0x207f && codepoint <= 0x207f) {
		return mcr_block58[codepoint - 0x207f];
	} else if (codepoint >= 0x20a7 && codepoint <= 0x20a7) {
		return mcr_block59[codepoint - 0x20a7];
	} else if (codepoint >= 0x20ac && codepoint <= 0x20ac) {
		return mcr_block60[codepoint - 0x20ac];
	} else if (codepoint >= 0x2117 && codepoint <= 0x2117) {
		return mcr_block61[codepoint - 0x2117];
	} else if (codepoint >= 0x2120 && codepoint <= 0x2120) {
		return mcr_block62[codepoint - 0x2120];
	} else if (codepoint >= 0x2122 && codepoint <= 0x2122) {
		return mcr_block63[codepoint - 0x2122];
	} else if (codepoint >= 0x2190 && codepoint <= 0x2192) {
		return mcr_block64[codepoint - 0x2190];
	} else if (codepoint >= 0x2194 && codepoint <= 0x2194) {
		return mcr_block65[codepoint - 0x2194];
	} else if (codepoint >= 0x2196 && codepoint <= 0x2199) {
		return mcr_block66[codepoint - 0x2196];
	} else if (codepoint >= 0x21a5 && codepoint <= 0x21a5) {
		return mcr_block67[codepoint - 0x21a5];
	} else if (codepoint >= 0x21a8 && codepoint <= 0x21a8) {
		return mcr_block68[codepoint - 0x21a8];
	} else if (codepoint >= 0x21e6 && codepoint <= 0x21e9) {
		return mcr_block69[codepoint - 0x21e6];
	} else if (codepoint >= 0x2212 && codepoint <= 0x2212) {
		return mcr_block70[codepoint - 0x2212];
	} else if (codepoint >= 0x2218 && codepoint <= 0x2218) {
		return mcr_block71[codepoint - 0x2218];
	} else if (codepoint >= 0x221a && codepoint <= 0x221a) {
		return mcr_block72[codepoint - 0x221a];
	} else if (codepoint >= 0x221e && codepoint <= 0x221f) {
		return mcr_block73[codepoint - 0x221e];
	} else if (codepoint >= 0x2229 && codepoint <= 0x2229) {
		return mcr_block74[codepoint - 0x2229];
	} else if (codepoint >= 0x2248 && codepoint <= 0x2248) {
		return mcr_block75[codepoint - 0x2248];
	} else if (codepoint >= 0x2261 && codepoint <= 0x2261) {
		return mcr_block76[codepoint - 0x2261];
	} else if (codepoint >= 0x2264 && codepoint <= 0x2265) {
		return mcr_block77[codepoint - 0x2264];
	} else if (codepoint >= 0x22c5 && codepoint <= 0x22c5) {
		return mcr_block78[codepoint - 0x22c5];
	} else if (codepoint >= 0x2320 && codepoint <= 0x2321) {
		return mcr_block79[codepoint - 0x2320];
	} else if (codepoint >= 0x23ea && codepoint <= 0x23ec) {
		return mcr_block80[codepoint - 0x23ea];
	} else if (codepoint >= 0x2500 && codepoint <= 0x25ff) {
		return mcr_block81[codepoint - 0x2500];
	} else if (codepoint >= 0x2625 && codepoint <= 0x2625) {
		return mcr_block82[codepoint - 0x2625];
	} else if (codepoint >= 0x2628 && codepoint <= 0x2628) {
		return mcr_block83[codepoint - 0x2628];
	} else if (codepoint >= 0x262f && codepoint <= 0x2637) {
		return mcr_block84[codepoint - 0x262f];
	} else if (codepoint >= 0x2639 && codepoint <= 0x263b) {
		return mcr_block85[codepoint - 0x2639];
	} else if (codepoint >= 0x2660 && codepoint <= 0x2667) {
		return mcr_block86[codepoint - 0x2660];
	} else if (codepoint >= 0x2669 && codepoint <= 0x266c) {
		return mcr_block87[codepoint - 0x2669];
	} else if (codepoint >= 0x268a && codepoint <= 0x268f) {
		return mcr_block88[codepoint - 0x268a];
	} else if (codepoint >= 0x26aa && codepoint <= 0x26ac) {
		return mcr_block89[codepoint - 0x26aa];
	} else if (codepoint >= 0x2708 && codepoint <= 0x2708) {
		return mcr_block90[codepoint - 0x2708];
	} else if (codepoint >= 0x2734 && codepoint <= 0x2734) {
		return mcr_block91[codepoint - 0x2734];
	} else if (codepoint >= 0x2800 && codepoint <= 0x28ff) {
		return mcr_block92[codepoint - 0x2800];
	} else if (codepoint >= 0x2913 && codepoint <= 0x2913) {
		return mcr_block93[codepoint - 0x2913];
	} else if (codepoint >= 0x2b1d && codepoint <= 0x2b1d) {
		return mcr_block94[codepoint - 0x2b1d];
	} else if (codepoint >= 0x2b24 && codepoint <= 0x2b24) {
		return mcr_block95[codepoint - 0x2b24];
	} else if (codepoint >= 0x2b55 && codepoint <= 0x2b55) {
		return mcr_block96[codepoint - 0x2b55];
	} else if (codepoint >= 0x2b58 && codepoint <= 0x2b58) {
		return mcr_block97[codepoint - 0x2b58];
	} else if (codepoint >= 0x2e2e && codepoint <= 0x2e2e) {
		return mcr_block98[codepoint - 0x2e2e];
	} else if (codepoint >= 0xe080 && codepoint <= 0xe19b) {
		return mcr_block99[codepoint - 0xe080];
	} else if (codepoint >= 0xe800 && codepoint <= 0xe850) {
		return mcr_block100[codepoint - 0xe800];
	} else if (codepoint >= 0xec00 && codepoint <= 0xec26) {
		return mcr_block101[codepoint - 0xec00];
	} else if (codepoint >= 0xfe81 && codepoint <= 0xfe82) {
		return mcr_block102[codepoint - 0xfe81];
	} else if (codepoint >= 0xfe84 && codepoint <= 0xfe84) {
		return mcr_block103[codepoint - 0xfe84];
	} else if (codepoint >= 0xfe87 && codepoint <= 0xfe87) {
		return mcr_block104[codepoint - 0xfe87];
	} else if (codepoint >= 0xfe8f && codepoint <= 0xfe9c) {
		return mcr_block105[codepoint - 0xfe8f];
	} else if (codepoint >= 0xfe9e && codepoint <= 0xfea4) {
		return mcr_block106[codepoint - 0xfe9e];
	} else if (codepoint >= 0xfea9 && codepoint <= 0xfead) {
		return mcr_block107[codepoint - 0xfea9];
	} else if (codepoint >= 0xfeaf && codepoint <= 0xfeaf) {
		return mcr_block108[codepoint - 0xfeaf];
	} else if (codepoint >= 0xfeb1 && codepoint <= 0xfeb7) {
		return mcr_block109[codepoint - 0xfeb1];
	} else if (codepoint >= 0xfeb9 && codepoint <= 0xfebc) {
		return mcr_block110[codepoint - 0xfeb9];
	} else if (codepoint >= 0xfec1 && codepoint <= 0xfec6) {
		return mcr_block111[codepoint - 0xfec1];
	} else if (codepoint >= 0xfec9 && codepoint <= 0xfedd) {
		return mcr_block112[codepoint - 0xfec9];
	} else if (codepoint >= 0xfedf && codepoint <= 0xfee3) {
		return mcr_block113[codepoint - 0xfedf];
	} else if (codepoint >= 0xfee5 && codepoint <= 0xfee9) {
		return mcr_block114[codepoint - 0xfee5];
	} else if (codepoint >= 0xfeeb && codepoint <= 0xfef4) {
		return mcr_block115[codepoint - 0xfeeb];
	} else {
		fprintf(stderr, "[FBInk] Codepoint U+%04X is not covered by our font!\n", codepoint);
		return mcr_block1[0];
	}
}

static const char*
    tall_get_bitmap(uint32_t codepoint)
{
	if (codepoint <= 0x19f) {
		return tall_block1[codepoint];
	} else if (codepoint >= 0x1cd && codepoint <= 0x1e3) {
		return tall_block2[codepoint - 0x1cd];
	} else if (codepoint >= 0x1e6 && codepoint <= 0x1e8) {
		return tall_block3[codepoint - 0x1e6];
	} else if (codepoint >= 0x1f0 && codepoint <= 0x1f0) {
		return tall_block4[codepoint - 0x1f0];
	} else if (codepoint >= 0x1f4 && codepoint <= 0x1f5) {
		return tall_block5[codepoint - 0x1f4];
	} else if (codepoint >= 0x1f8 && codepoint <= 0x21b) {
		return tall_block6[codepoint - 0x1f8];
	} else if (codepoint >= 0x226 && codepoint <= 0x227) {
		return tall_block7[codepoint - 0x226];
	} else if (codepoint >= 0x22a && codepoint <= 0x233) {
		return tall_block8[codepoint - 0x22a];
	} else if (codepoint >= 0x258 && codepoint <= 0x259) {
		return tall_block9[codepoint - 0x258];
	} else if (codepoint >= 0x25b && codepoint <= 0x25c) {
		return tall_block10[codepoint - 0x25b];
	} else if (codepoint >= 0x263 && codepoint <= 0x263) {
		return tall_block11[codepoint - 0x263];
	} else if (codepoint >= 0x26a && codepoint <= 0x26a) {
		return tall_block12[codepoint - 0x26a];
	} else if (codepoint >= 0x28a && codepoint <= 0x296) {
		return tall_block13[codepoint - 0x28a];
	} else if (codepoint >= 0x2c6 && codepoint <= 0x2c6) {
		return tall_block14[codepoint - 0x2c6];
	} else if (codepoint >= 0x2c9 && codepoint <= 0x2c9) {
		return tall_block15[codepoint - 0x2c9];
	} else if (codepoint >= 0x2cd && codepoint <= 0x2cd) {
		return tall_block16[codepoint - 0x2cd];
	} else if (codepoint >= 0x2dc && codepoint <= 0x2dc) {
		return tall_block17[codepoint - 0x2dc];
	} else if (codepoint >= 0x34f && codepoint <= 0x34f) {
		return tall_block18[codepoint - 0x34f];
	} else if (codepoint >= 0x391 && codepoint <= 0x3a1) {
		return tall_block19[codepoint - 0x391];
	} else if (codepoint >= 0x3a3 && codepoint <= 0x3a9) {
		return tall_block20[codepoint - 0x3a3];
	} else if (codepoint >= 0x3b1 && codepoint <= 0x3c9) {
		return tall_block21[codepoint - 0x3b1];
	} else if (codepoint >= 0x400 && codepoint <= 0x401) {
		return tall_block22[codepoint - 0x400];
	} else if (codepoint >= 0x405 && codepoint <= 0x409) {
		return tall_block23[codepoint - 0x405];
	} else if (codepoint >= 0x410 && codepoint <= 0x451) {
		return tall_block24[codepoint - 0x410];
	} else if (codepoint >= 0x455 && codepoint <= 0x458) {
		return tall_block25[codepoint - 0x455];
	} else if (codepoint >= 0x4d0 && codepoint <= 0x4d7) {
		return tall_block26[codepoint - 0x4d0];
	} else if (codepoint >= 0x4e6 && codepoint <= 0x4e7) {
		return tall_block27[codepoint - 0x4e6];
	} else if (codepoint >= 0x4f1 && codepoint <= 0x4f1) {
		return tall_block28[codepoint - 0x4f1];
	} else if (codepoint >= 0x5d0 && codepoint <= 0x5ea) {
		return tall_block29[codepoint - 0x5d0];
	} else if (codepoint >= 0x623 && codepoint <= 0x623) {
		return tall_block30[codepoint - 0x623];
	} else if (codepoint >= 0x627 && codepoint <= 0x62d) {
		return tall_block31[codepoint - 0x627];
	} else if (codepoint >= 0x62f && codepoint <= 0x635) {
		return tall_block32[codepoint - 0x62f];
	} else if (codepoint >= 0x637 && codepoint <= 0x63a) {
		return tall_block33[codepoint - 0x637];
	} else if (codepoint >= 0x640 && codepoint <= 0x64a) {
		return tall_block34[codepoint - 0x640];
	} else if (codepoint >= 0x671 && codepoint <= 0x673) {
		return tall_block35[codepoint - 0x671];
	} else if (codepoint >= 0x1560 && codepoint <= 0x1563) {
		return tall_block36[codepoint - 0x1560];
	} else if (codepoint >= 0x156a && codepoint <= 0x156a) {
		return tall_block37[codepoint - 0x156a];
	} else if (codepoint >= 0x156c && codepoint <= 0x1571) {
		return tall_block38[codepoint - 0x156c];
	} else if (codepoint >= 0x15ec && codepoint <= 0x15ed) {
		return tall_block39[codepoint - 0x15ec];
	} else if (codepoint >= 0x15ef && codepoint <= 0x15ef) {
		return tall_block40[codepoint - 0x15ef];
	} else if (codepoint >= 0x16a0 && codepoint <= 0x16ab) {
		return tall_block41[codepoint - 0x16a0];
	} else if (codepoint >= 0x16b1 && codepoint <= 0x16f8) {
		return tall_block42[codepoint - 0x16b1];
	} else if (codepoint >= 0x1e00 && codepoint <= 0x1e02) {
		return tall_block43[codepoint - 0x1e00];
	} else if (codepoint >= 0x1e04 && codepoint <= 0x1e07) {
		return tall_block44[codepoint - 0x1e04];
	} else if (codepoint >= 0x1e0a && codepoint <= 0x1e0a) {
		return tall_block45[codepoint - 0x1e0a];
	} else if (codepoint >= 0x1e0c && codepoint <= 0x1e0f) {
		return tall_block46[codepoint - 0x1e0c];
	} else if (codepoint >= 0x1e41 && codepoint <= 0x1e4b) {
		return tall_block47[codepoint - 0x1e41];
	} else if (codepoint >= 0x1e58 && codepoint <= 0x1e5b) {
		return tall_block48[codepoint - 0x1e58];
	} else if (codepoint >= 0x1e5e && codepoint <= 0x1e5e) {
		return tall_block49[codepoint - 0x1e5e];
	} else if (codepoint >= 0x2010 && codepoint <= 0x2019) {
		return tall_block50[codepoint - 0x2010];
	} else if (codepoint >= 0x201c && codepoint <= 0x201d) {
		return tall_block51[codepoint - 0x201c];
	} else if (codepoint >= 0x2020 && codepoint <= 0x2022) {
		return tall_block52[codepoint - 0x2020];
	} else if (codepoint >= 0x2024 && codepoint <= 0x2026) {
		return tall_block53[codepoint - 0x2024];
	} else if (codepoint >= 0x2030 && codepoint <= 0x2031) {
		return tall_block54[codepoint - 0x2030];
	} else if (codepoint >= 0x203c && codepoint <= 0x203c) {
		return tall_block55[codepoint - 0x203c];
	} else if (codepoint >= 0x2047 && codepoint <= 0x2049) {
		return tall_block56[codepoint - 0x2047];
	} else if (codepoint >= 0x2070 && codepoint <= 0x2070) {
		return tall_block57[codepoint - 0x2070];
	} else if (codepoint >= 0x2074 && codepoint <= 0x2079) {
		return tall_block58[codepoint - 0x2074];
	} else if (codepoint >= 0x207f && codepoint <= 0x207f) {
		return tall_block59[codepoint - 0x207f];
	} else if (codepoint >= 0x20a7 && codepoint <= 0x20a7) {
		return tall_block60[codepoint - 0x20a7];
	} else if (codepoint >= 0x20ac && codepoint <= 0x20ac) {
		return tall_block61[codepoint - 0x20ac];
	} else if (codepoint >= 0x2117 && codepoint <= 0x2117) {
		return tall_block62[codepoint - 0x2117];
	} else if (codepoint >= 0x2120 && codepoint <= 0x2120) {
		return tall_block63[codepoint - 0x2120];
	} else if (codepoint >= 0x2122 && codepoint <= 0x2122) {
		return tall_block64[codepoint - 0x2122];
	} else if (codepoint >= 0x2190 && codepoint <= 0x2192) {
		return tall_block65[codepoint - 0x2190];
	} else if (codepoint >= 0x2194 && codepoint <= 0x2194) {
		return tall_block66[codepoint - 0x2194];
	} else if (codepoint >= 0x2196 && codepoint <= 0x2199) {
		return tall_block67[codepoint - 0x2196];
	} else if (codepoint >= 0x21a5 && codepoint <= 0x21a5) {
		return tall_block68[codepoint - 0x21a5];
	} else if (codepoint >= 0x21a8 && codepoint <= 0x21a8) {
		return tall_block69[codepoint - 0x21a8];
	} else if (codepoint >= 0x21e6 && codepoint <= 0x21e9) {
		return tall_block70[codepoint - 0x21e6];
	} else if (codepoint >= 0x2212 && codepoint <= 0x2212) {
		return tall_block71[codepoint - 0x2212];
	} else if (codepoint >= 0x2218 && codepoint <= 0x2218) {
		return tall_block72[codepoint - 0x2218];
	} else if (codepoint >= 0x221a && codepoint <= 0x221a) {
		return tall_block73[codepoint - 0x221a];
	} else if (codepoint >= 0x221e && codepoint <= 0x221f) {
		return tall_block74[codepoint - 0x221e];
	} else if (codepoint >= 0x2229 && codepoint <= 0x2229) {
		return tall_block75[codepoint - 0x2229];
	} else if (codepoint >= 0x2248 && codepoint <= 0x2248) {
		return tall_block76[codepoint - 0x2248];
	} else if (codepoint >= 0x2261 && codepoint <= 0x2261) {
		return tall_block77[codepoint - 0x2261];
	} else if (codepoint >= 0x2264 && codepoint <= 0x2265) {
		return tall_block78[codepoint - 0x2264];
	} else if (codepoint >= 0x22c5 && codepoint <= 0x22c5) {
		return tall_block79[codepoint - 0x22c5];
	} else if (codepoint >= 0x2320 && codepoint <= 0x2321) {
		return tall_block80[codepoint - 0x2320];
	} else if (codepoint >= 0x23ea && codepoint <= 0x23ec) {
		return tall_block81[codepoint - 0x23ea];
	} else if (codepoint >= 0x2500 && codepoint <= 0x25ff) {
		return tall_block82[codepoint - 0x2500];
	} else if (codepoint >= 0x2625 && codepoint <= 0x2625) {
		return tall_block83[codepoint - 0x2625];
	} else if (codepoint >= 0x2628 && codepoint <= 0x2628) {
		return tall_block84[codepoint - 0x2628];
	} else if (codepoint >= 0x262f && codepoint <= 0x2637) {
		return tall_block85[codepoint - 0x262f];
	} else if (codepoint >= 0x2639 && codepoint <= 0x263b) {
		return tall_block86[codepoint - 0x2639];
	} else if (codepoint >= 0x2660 && codepoint <= 0x2667) {
		return tall_block87[codepoint - 0x2660];
	} else if (codepoint >= 0x2669 && codepoint <= 0x266c) {
		return tall_block88[codepoint - 0x2669];
	} else if (codepoint >= 0x268a && codepoint <= 0x268f) {
		return tall_block89[codepoint - 0x268a];
	} else if (codepoint >= 0x26aa && codepoint <= 0x26ac) {
		return tall_block90[codepoint - 0x26aa];
	} else if (codepoint >= 0x2708 && codepoint <= 0x2708) {
		return tall_block91[codepoint - 0x2708];
	} else if (codepoint >= 0x2734 && codepoint <= 0x2734) {
		return tall_block92[codepoint - 0x2734];
	} else if (codepoint >= 0x2800 && codepoint <= 0x28ff) {
		return tall_block93[codepoint - 0x2800];
	} else if (codepoint >= 0x2913 && codepoint <= 0x2913) {
		return tall_block94[codepoint - 0x2913];
	} else if (codepoint >= 0x2b1d && codepoint <= 0x2b1d) {
		return tall_block95[codepoint - 0x2b1d];
	} else if (codepoint >= 0x2b24 && codepoint <= 0x2b24) {
		return tall_block96[codepoint - 0x2b24];
	} else if (codepoint >= 0x2b55 && codepoint <= 0x2b55) {
		return tall_block97[codepoint - 0x2b55];
	} else if (codepoint >= 0x2b58 && codepoint <= 0x2b58) {
		return tall_block98[codepoint - 0x2b58];
	} else if (codepoint >= 0x2e2e && codepoint <= 0x2e2e) {
		return tall_block99[codepoint - 0x2e2e];
	} else if (codepoint >= 0xe080 && codepoint <= 0xe19b) {
		return tall_block100[codepoint - 0xe080];
	} else if (codepoint >= 0xe800 && codepoint <= 0xe850) {
		return tall_block101[codepoint - 0xe800];
	} else if (codepoint >= 0xec00 && codepoint <= 0xec26) {
		return tall_block102[codepoint - 0xec00];
	} else if (codepoint >= 0xfe81 && codepoint <= 0xfe82) {
		return tall_block103[codepoint - 0xfe81];
	} else if (codepoint >= 0xfe84 && codepoint <= 0xfe84) {
		return tall_block104[codepoint - 0xfe84];
	} else if (codepoint >= 0xfe87 && codepoint <= 0xfe88) {
		return tall_block105[codepoint - 0xfe87];
	} else if (codepoint >= 0xfe8e && codepoint <= 0xfe9c) {
		return tall_block106[codepoint - 0xfe8e];
	} else if (codepoint >= 0xfe9e && codepoint <= 0xfea4) {
		return tall_block107[codepoint - 0xfe9e];
	} else if (codepoint >= 0xfea9 && codepoint <= 0xfeb7) {
		return tall_block108[codepoint - 0xfea9];
	} else if (codepoint >= 0xfeb9 && codepoint <= 0xfebc) {
		return tall_block109[codepoint - 0xfeb9];
	} else if (codepoint >= 0xfec1 && codepoint <= 0xfec6) {
		return tall_block110[codepoint - 0xfec1];
	} else if (codepoint >= 0xfec9 && codepoint <= 0xfee3) {
		return tall_block111[codepoint - 0xfec9];
	} else if (codepoint >= 0xfee5 && codepoint <= 0xfef4) {
		return tall_block112[codepoint - 0xfee5];
	} else if (codepoint >= 0xff01 && codepoint <= 0xff3a) {
		return tall_block113[codepoint - 0xff01];
	} else if (codepoint >= 0xff3c && codepoint <= 0xff3c) {
		return tall_block114[codepoint - 0xff3c];
	} else if (codepoint >= 0xff3e && codepoint <= 0xff5a) {
		return tall_block115[codepoint - 0xff3e];
	} else if (codepoint >= 0xff5c && codepoint <= 0xff5e) {
		return tall_block116[codepoint - 0xff5c];
	} else {
		fprintf(stderr, "[FBInk] Codepoint U+%04X is not covered by our font!\n", codepoint);
		return tall_block1[0];
	}
}

