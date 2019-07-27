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

#include "fbink_scientifica.h"

static const unsigned char*
    scientifica_get_bitmap(uint32_t codepoint)
{
	if (codepoint >= 0x20 && codepoint <= 0x7e) {
		return scientifica_block1[codepoint - 0x20];
	} else if (codepoint >= 0xa0 && codepoint <= 0x1b6) {
		return scientifica_block2[codepoint - 0xa0];
	} else if (codepoint >= 0x1c7 && codepoint <= 0x1d4) {
		return scientifica_block3[codepoint - 0x1c7];
	} else if (codepoint >= 0x20c && codepoint <= 0x21b) {
		return scientifica_block4[codepoint - 0x20c];
	} else if (codepoint == 0x296) {
		return scientifica_block5[0];
	} else if (codepoint == 0x298) {
		return scientifica_block6[0];
	} else if (codepoint == 0x2c7) {
		return scientifica_block7[0];
	} else if (codepoint == 0x2da) {
		return scientifica_block8[0];
	} else if (codepoint == 0x364) {
		return scientifica_block9[0];
	} else if (codepoint >= 0x391 && codepoint <= 0x3a9) {
		return scientifica_block10[codepoint - 0x391];
	} else if (codepoint >= 0x3b1 && codepoint <= 0x3c9) {
		return scientifica_block11[codepoint - 0x3b1];
	} else if (codepoint >= 0xf3c && codepoint <= 0xf3d) {
		return scientifica_block12[codepoint - 0xf3c];
	} else if (codepoint == 0x1e9e) {
		return scientifica_block13[0];
	} else if (codepoint >= 0x2010 && codepoint <= 0x2027) {
		return scientifica_block14[codepoint - 0x2010];
	} else if (codepoint >= 0x2032 && codepoint <= 0x203c) {
		return scientifica_block15[codepoint - 0x2032];
	} else if (codepoint >= 0x203e && codepoint <= 0x2042) {
		return scientifica_block16[codepoint - 0x203e];
	} else if (codepoint >= 0x204f && codepoint <= 0x2051) {
		return scientifica_block17[codepoint - 0x204f];
	} else if (codepoint == 0x205c) {
		return scientifica_block18[0];
	} else if (codepoint == 0x20a8) {
		return scientifica_block19[0];
	} else if (codepoint == 0x20aa) {
		return scientifica_block20[0];
	} else if (codepoint == 0x20ac) {
		return scientifica_block21[0];
	} else if (codepoint == 0x2122) {
		return scientifica_block22[0];
	} else if (codepoint >= 0x2190 && codepoint <= 0x2199) {
		return scientifica_block23[codepoint - 0x2190];
	} else if (codepoint >= 0x21a4 && codepoint <= 0x21a7) {
		return scientifica_block24[codepoint - 0x21a4];
	} else if (codepoint >= 0x21a9 && codepoint <= 0x21aa) {
		return scientifica_block25[codepoint - 0x21a9];
	} else if (codepoint >= 0x21b0 && codepoint <= 0x21b7) {
		return scientifica_block26[codepoint - 0x21b0];
	} else if (codepoint >= 0x21b9 && codepoint <= 0x21c4) {
		return scientifica_block27[codepoint - 0x21b9];
	} else if (codepoint >= 0x21c6 && codepoint <= 0x21cb) {
		return scientifica_block28[codepoint - 0x21c6];
	} else if (codepoint >= 0x21d0 && codepoint <= 0x21d3) {
		return scientifica_block29[codepoint - 0x21d0];
	} else if (codepoint >= 0x2200 && codepoint <= 0x2214) {
		return scientifica_block30[codepoint - 0x2200];
	} else if (codepoint >= 0x2217 && codepoint <= 0x221a) {
		return scientifica_block31[codepoint - 0x2217];
	} else if (codepoint >= 0x221d && codepoint <= 0x221e) {
		return scientifica_block32[codepoint - 0x221d];
	} else if (codepoint >= 0x2227 && codepoint <= 0x222b) {
		return scientifica_block33[codepoint - 0x2227];
	} else if (codepoint >= 0x2234 && codepoint <= 0x2237) {
		return scientifica_block34[codepoint - 0x2234];
	} else if (codepoint == 0x2243) {
		return scientifica_block35[0];
	} else if (codepoint == 0x2248) {
		return scientifica_block36[0];
	} else if (codepoint >= 0x2254 && codepoint <= 0x2255) {
		return scientifica_block37[codepoint - 0x2254];
	} else if (codepoint >= 0x2260 && codepoint <= 0x2262) {
		return scientifica_block38[codepoint - 0x2260];
	} else if (codepoint >= 0x2264 && codepoint <= 0x2265) {
		return scientifica_block39[codepoint - 0x2264];
	} else if (codepoint >= 0x2282 && codepoint <= 0x2287) {
		return scientifica_block40[codepoint - 0x2282];
	} else if (codepoint >= 0x2295 && codepoint <= 0x2296) {
		return scientifica_block41[codepoint - 0x2295];
	} else if (codepoint == 0x2299) {
		return scientifica_block42[0];
	} else if (codepoint >= 0x229e && codepoint <= 0x22a9) {
		return scientifica_block43[codepoint - 0x229e];
	} else if (codepoint == 0x22ab) {
		return scientifica_block44[0];
	} else if (codepoint >= 0x22c4 && codepoint <= 0x22c6) {
		return scientifica_block45[codepoint - 0x22c4];
	} else if (codepoint == 0x2325) {
		return scientifica_block46[0];
	} else if (codepoint >= 0x239b && codepoint <= 0x23ae) {
		return scientifica_block47[codepoint - 0x239b];
	} else if (codepoint >= 0x23ba && codepoint <= 0x23bd) {
		return scientifica_block48[codepoint - 0x23ba];
	} else if (codepoint >= 0x23e9 && codepoint <= 0x23ec) {
		return scientifica_block49[codepoint - 0x23e9];
	} else if (codepoint >= 0x23f4 && codepoint <= 0x23fb) {
		return scientifica_block50[codepoint - 0x23f4];
	} else if (codepoint >= 0x2500 && codepoint <= 0x2503) {
		return scientifica_block51[codepoint - 0x2500];
	} else if (codepoint >= 0x2506 && codepoint <= 0x2507) {
		return scientifica_block52[codepoint - 0x2506];
	} else if (codepoint >= 0x250a && codepoint <= 0x251d) {
		return scientifica_block53[codepoint - 0x250a];
	} else if (codepoint == 0x2520) {
		return scientifica_block54[0];
	} else if (codepoint >= 0x2523 && codepoint <= 0x2525) {
		return scientifica_block55[codepoint - 0x2523];
	} else if (codepoint == 0x2528) {
		return scientifica_block56[0];
	} else if (codepoint >= 0x252b && codepoint <= 0x252c) {
		return scientifica_block57[codepoint - 0x252b];
	} else if (codepoint >= 0x252f && codepoint <= 0x2530) {
		return scientifica_block58[codepoint - 0x252f];
	} else if (codepoint >= 0x2533 && codepoint <= 0x2534) {
		return scientifica_block59[codepoint - 0x2533];
	} else if (codepoint >= 0x2537 && codepoint <= 0x2538) {
		return scientifica_block60[codepoint - 0x2537];
	} else if (codepoint >= 0x253b && codepoint <= 0x253c) {
		return scientifica_block61[codepoint - 0x253b];
	} else if (codepoint == 0x253f) {
		return scientifica_block62[0];
	} else if (codepoint == 0x2542) {
		return scientifica_block63[0];
	} else if (codepoint == 0x254b) {
		return scientifica_block64[0];
	} else if (codepoint >= 0x2550 && codepoint <= 0x2573) {
		return scientifica_block65[codepoint - 0x2550];
	} else if (codepoint == 0x257c) {
		return scientifica_block66[0];
	} else if (codepoint == 0x257e) {
		return scientifica_block67[0];
	} else if (codepoint >= 0x2581 && codepoint <= 0x258a) {
		return scientifica_block68[codepoint - 0x2581];
	} else if (codepoint == 0x258f) {
		return scientifica_block69[0];
	} else if (codepoint >= 0x2591 && codepoint <= 0x2593) {
		return scientifica_block70[codepoint - 0x2591];
	} else if (codepoint >= 0x25a1 && codepoint <= 0x25a2) {
		return scientifica_block71[codepoint - 0x25a1];
	} else if (codepoint >= 0x25aa && codepoint <= 0x25ab) {
		return scientifica_block72[codepoint - 0x25aa];
	} else if (codepoint >= 0x25b2 && codepoint <= 0x25b9) {
		return scientifica_block73[codepoint - 0x25b2];
	} else if (codepoint >= 0x25bc && codepoint <= 0x25be) {
		return scientifica_block74[codepoint - 0x25bc];
	} else if (codepoint >= 0x25c0 && codepoint <= 0x25c4) {
		return scientifica_block75[codepoint - 0x25c0];
	} else if (codepoint == 0x25cb) {
		return scientifica_block76[0];
	} else if (codepoint >= 0x25ce && codepoint <= 0x25d7) {
		return scientifica_block77[codepoint - 0x25ce];
	} else if (codepoint >= 0x25e7 && codepoint <= 0x25eb) {
		return scientifica_block78[codepoint - 0x25e7];
	} else if (codepoint >= 0x25f0 && codepoint <= 0x25f3) {
		return scientifica_block79[codepoint - 0x25f0];
	} else if (codepoint >= 0x2600 && codepoint <= 0x2602) {
		return scientifica_block80[codepoint - 0x2600];
	} else if (codepoint >= 0x2630 && codepoint <= 0x2637) {
		return scientifica_block81[codepoint - 0x2630];
	} else if (codepoint == 0x2661) {
		return scientifica_block82[0];
	} else if (codepoint >= 0x2665 && codepoint <= 0x2666) {
		return scientifica_block83[codepoint - 0x2665];
	} else if (codepoint >= 0x2669 && codepoint <= 0x266c) {
		return scientifica_block84[codepoint - 0x2669];
	} else if (codepoint == 0x267a) {
		return scientifica_block85[0];
	} else if (codepoint >= 0x26aa && codepoint <= 0x26ab) {
		return scientifica_block86[codepoint - 0x26aa];
	} else if (codepoint >= 0x2713 && codepoint <= 0x2718) {
		return scientifica_block87[codepoint - 0x2713];
	} else if (codepoint == 0x272e) {
		return scientifica_block88[0];
	} else if (codepoint == 0x2744) {
		return scientifica_block89[0];
	} else if (codepoint >= 0x276e && codepoint <= 0x2771) {
		return scientifica_block90[codepoint - 0x276e];
	} else if (codepoint >= 0x27c2 && codepoint <= 0x27c4) {
		return scientifica_block91[codepoint - 0x27c2];
	} else if (codepoint >= 0x27d8 && codepoint <= 0x27d9) {
		return scientifica_block92[codepoint - 0x27d8];
	} else if (codepoint >= 0x27dc && codepoint <= 0x27de) {
		return scientifica_block93[codepoint - 0x27dc];
	} else if (codepoint >= 0x2864 && codepoint <= 0x28ff) {
		return scientifica_block94[codepoint - 0x2864];
	} else if (codepoint >= 0x2919 && codepoint <= 0x291c) {
		return scientifica_block95[codepoint - 0x2919];
	} else if (codepoint >= 0x2b5e && codepoint <= 0x2b62) {
		return scientifica_block96[codepoint - 0x2b5e];
	} else if (codepoint == 0x2b64) {
		return scientifica_block97[0];
	} else if (codepoint >= 0x2b80 && codepoint <= 0x2b83) {
		return scientifica_block98[codepoint - 0x2b80];
	} else if (codepoint >= 0x30c3 && codepoint <= 0x30c4) {
		return scientifica_block99[codepoint - 0x30c3];
	} else if (codepoint == 0x5350) {
		return scientifica_block100[0];
	} else if (codepoint >= 0xe09e && codepoint <= 0xe0a2) {
		return scientifica_block101[codepoint - 0xe09e];
	} else if (codepoint >= 0xe0b0 && codepoint <= 0xe0b5) {
		return scientifica_block102[codepoint - 0xe0b0];
	} else if (codepoint >= 0xe0c0 && codepoint <= 0xe0c6) {
		return scientifica_block103[codepoint - 0xe0c0];
	} else if (codepoint == 0xe0d1) {
		return scientifica_block104[0];
	} else if (codepoint == 0xf031) {
		return scientifica_block105[0];
	} else if (codepoint >= 0xf033 && codepoint <= 0xf03d) {
		return scientifica_block106[codepoint - 0xf033];
	} else if (codepoint >= 0xf057 && codepoint <= 0xf059) {
		return scientifica_block107[codepoint - 0xf057];
	} else if (codepoint == 0xf061) {
		return scientifica_block108[0];
	} else if (codepoint == 0xf073) {
		return scientifica_block109[0];
	} else if (codepoint >= 0xf078 && codepoint <= 0xf079) {
		return scientifica_block110[codepoint - 0xf078];
	} else if (codepoint == 0xf07e) {
		return scientifica_block111[0];
	} else if (codepoint >= 0xf0cf && codepoint <= 0xf0d1) {
		return scientifica_block112[codepoint - 0xf0cf];
	} else if (codepoint >= 0xf0d5 && codepoint <= 0xf0dc) {
		return scientifica_block113[codepoint - 0xf0d5];
	} else if (codepoint >= 0xf0de && codepoint <= 0xf0e0) {
		return scientifica_block114[codepoint - 0xf0de];
	} else if (codepoint >= 0xf0ed && codepoint <= 0xf0f0) {
		return scientifica_block115[codepoint - 0xf0ed];
	} else {
		WARN("Codepoint U+%04X is not covered by this font", codepoint);
		return scientifica_block1[0];
	}
}

static const unsigned char*
    scientificab_get_bitmap(uint32_t codepoint)
{
	if (codepoint >= 0x20 && codepoint <= 0x7e) {
		return scientificab_block1[codepoint - 0x20];
	} else if (codepoint >= 0xa0 && codepoint <= 0x1b6) {
		return scientificab_block2[codepoint - 0xa0];
	} else if (codepoint >= 0x1c7 && codepoint <= 0x1d4) {
		return scientificab_block3[codepoint - 0x1c7];
	} else if (codepoint >= 0x20c && codepoint <= 0x211) {
		return scientificab_block4[codepoint - 0x20c];
	} else if (codepoint >= 0x213 && codepoint <= 0x21b) {
		return scientificab_block5[codepoint - 0x213];
	} else if (codepoint == 0x296) {
		return scientificab_block6[0];
	} else if (codepoint == 0x2c7) {
		return scientificab_block7[0];
	} else if (codepoint == 0x2da) {
		return scientificab_block8[0];
	} else if (codepoint == 0x364) {
		return scientificab_block9[0];
	} else if (codepoint >= 0x391 && codepoint <= 0x3a9) {
		return scientificab_block10[codepoint - 0x391];
	} else if (codepoint >= 0x3b1 && codepoint <= 0x3c9) {
		return scientificab_block11[codepoint - 0x3b1];
	} else if (codepoint >= 0xf3c && codepoint <= 0xf3d) {
		return scientificab_block12[codepoint - 0xf3c];
	} else if (codepoint == 0x1e9e) {
		return scientificab_block13[0];
	} else if (codepoint >= 0x2010 && codepoint <= 0x2027) {
		return scientificab_block14[codepoint - 0x2010];
	} else if (codepoint >= 0x2032 && codepoint <= 0x203a) {
		return scientificab_block15[codepoint - 0x2032];
	} else if (codepoint == 0x203c) {
		return scientificab_block16[0];
	} else if (codepoint >= 0x203e && codepoint <= 0x2042) {
		return scientificab_block17[codepoint - 0x203e];
	} else if (codepoint >= 0x204f && codepoint <= 0x2051) {
		return scientificab_block18[codepoint - 0x204f];
	} else if (codepoint == 0x20ac) {
		return scientificab_block19[0];
	} else if (codepoint == 0x2122) {
		return scientificab_block20[0];
	} else if (codepoint >= 0x2190 && codepoint <= 0x2199) {
		return scientificab_block21[codepoint - 0x2190];
	} else if (codepoint >= 0x21a4 && codepoint <= 0x21a7) {
		return scientificab_block22[codepoint - 0x21a4];
	} else if (codepoint >= 0x21a9 && codepoint <= 0x21aa) {
		return scientificab_block23[codepoint - 0x21a9];
	} else if (codepoint >= 0x21b1 && codepoint <= 0x21b4) {
		return scientificab_block24[codepoint - 0x21b1];
	} else if (codepoint >= 0x21b9 && codepoint <= 0x21c4) {
		return scientificab_block25[codepoint - 0x21b9];
	} else if (codepoint >= 0x21c6 && codepoint <= 0x21c7) {
		return scientificab_block26[codepoint - 0x21c6];
	} else if (codepoint == 0x21c9) {
		return scientificab_block27[0];
	} else if (codepoint >= 0x21d0 && codepoint <= 0x21d3) {
		return scientificab_block28[codepoint - 0x21d0];
	} else if (codepoint >= 0x2200 && codepoint <= 0x2214) {
		return scientificab_block29[codepoint - 0x2200];
	} else if (codepoint >= 0x2218 && codepoint <= 0x221a) {
		return scientificab_block30[codepoint - 0x2218];
	} else if (codepoint >= 0x221d && codepoint <= 0x221e) {
		return scientificab_block31[codepoint - 0x221d];
	} else if (codepoint >= 0x2229 && codepoint <= 0x222b) {
		return scientificab_block32[codepoint - 0x2229];
	} else if (codepoint >= 0x2234 && codepoint <= 0x2237) {
		return scientificab_block33[codepoint - 0x2234];
	} else if (codepoint == 0x2248) {
		return scientificab_block34[0];
	} else if (codepoint >= 0x2260 && codepoint <= 0x2261) {
		return scientificab_block35[codepoint - 0x2260];
	} else if (codepoint >= 0x2264 && codepoint <= 0x2265) {
		return scientificab_block36[codepoint - 0x2264];
	} else if (codepoint == 0x227a) {
		return scientificab_block37[0];
	} else if (codepoint >= 0x2282 && codepoint <= 0x2287) {
		return scientificab_block38[codepoint - 0x2282];
	} else if (codepoint >= 0x2295 && codepoint <= 0x2296) {
		return scientificab_block39[codepoint - 0x2295];
	} else if (codepoint == 0x2299) {
		return scientificab_block40[0];
	} else if (codepoint >= 0x229e && codepoint <= 0x22a9) {
		return scientificab_block41[codepoint - 0x229e];
	} else if (codepoint == 0x22ab) {
		return scientificab_block42[0];
	} else if (codepoint >= 0x22c4 && codepoint <= 0x22c6) {
		return scientificab_block43[codepoint - 0x22c4];
	} else if (codepoint >= 0x23ba && codepoint <= 0x23bd) {
		return scientificab_block44[codepoint - 0x23ba];
	} else if (codepoint >= 0x23e9 && codepoint <= 0x23ec) {
		return scientificab_block45[codepoint - 0x23e9];
	} else if (codepoint >= 0x23f4 && codepoint <= 0x23fb) {
		return scientificab_block46[codepoint - 0x23f4];
	} else if (codepoint >= 0x2500 && codepoint <= 0x2503) {
		return scientificab_block47[codepoint - 0x2500];
	} else if (codepoint >= 0x250a && codepoint <= 0x251d) {
		return scientificab_block48[codepoint - 0x250a];
	} else if (codepoint == 0x2520) {
		return scientificab_block49[0];
	} else if (codepoint >= 0x2523 && codepoint <= 0x2525) {
		return scientificab_block50[codepoint - 0x2523];
	} else if (codepoint == 0x2528) {
		return scientificab_block51[0];
	} else if (codepoint >= 0x252b && codepoint <= 0x252c) {
		return scientificab_block52[codepoint - 0x252b];
	} else if (codepoint >= 0x252f && codepoint <= 0x2530) {
		return scientificab_block53[codepoint - 0x252f];
	} else if (codepoint >= 0x2533 && codepoint <= 0x2534) {
		return scientificab_block54[codepoint - 0x2533];
	} else if (codepoint >= 0x2537 && codepoint <= 0x2538) {
		return scientificab_block55[codepoint - 0x2537];
	} else if (codepoint >= 0x253b && codepoint <= 0x253c) {
		return scientificab_block56[codepoint - 0x253b];
	} else if (codepoint == 0x253f) {
		return scientificab_block57[0];
	} else if (codepoint == 0x2542) {
		return scientificab_block58[0];
	} else if (codepoint == 0x254b) {
		return scientificab_block59[0];
	} else if (codepoint >= 0x2550 && codepoint <= 0x2570) {
		return scientificab_block60[codepoint - 0x2550];
	} else if (codepoint == 0x257c) {
		return scientificab_block61[0];
	} else if (codepoint == 0x257e) {
		return scientificab_block62[0];
	} else if (codepoint >= 0x2581 && codepoint <= 0x258a) {
		return scientificab_block63[codepoint - 0x2581];
	} else if (codepoint == 0x258f) {
		return scientificab_block64[0];
	} else if (codepoint >= 0x2591 && codepoint <= 0x2593) {
		return scientificab_block65[codepoint - 0x2591];
	} else if (codepoint >= 0x25a1 && codepoint <= 0x25a2) {
		return scientificab_block66[codepoint - 0x25a1];
	} else if (codepoint == 0x25aa) {
		return scientificab_block67[0];
	} else if (codepoint >= 0x25b2 && codepoint <= 0x25b4) {
		return scientificab_block68[codepoint - 0x25b2];
	} else if (codepoint >= 0x25b6 && codepoint <= 0x25b9) {
		return scientificab_block69[codepoint - 0x25b6];
	} else if (codepoint >= 0x25bc && codepoint <= 0x25be) {
		return scientificab_block70[codepoint - 0x25bc];
	} else if (codepoint >= 0x25c0 && codepoint <= 0x25c4) {
		return scientificab_block71[codepoint - 0x25c0];
	} else if (codepoint >= 0x25d2 && codepoint <= 0x25d3) {
		return scientificab_block72[codepoint - 0x25d2];
	} else if (codepoint >= 0x25d5 && codepoint <= 0x25d7) {
		return scientificab_block73[codepoint - 0x25d5];
	} else if (codepoint >= 0x25e7 && codepoint <= 0x25ea) {
		return scientificab_block74[codepoint - 0x25e7];
	} else if (codepoint >= 0x2600 && codepoint <= 0x2602) {
		return scientificab_block75[codepoint - 0x2600];
	} else if (codepoint == 0x2661) {
		return scientificab_block76[0];
	} else if (codepoint == 0x2665) {
		return scientificab_block77[0];
	} else if (codepoint >= 0x2669 && codepoint <= 0x266c) {
		return scientificab_block78[codepoint - 0x2669];
	} else if (codepoint >= 0x2713 && codepoint <= 0x2718) {
		return scientificab_block79[codepoint - 0x2713];
	} else if (codepoint == 0x272e) {
		return scientificab_block80[0];
	} else if (codepoint == 0x2744) {
		return scientificab_block81[0];
	} else if (codepoint >= 0x276e && codepoint <= 0x2771) {
		return scientificab_block82[codepoint - 0x276e];
	} else if (codepoint >= 0x27c2 && codepoint <= 0x27c4) {
		return scientificab_block83[codepoint - 0x27c2];
	} else if (codepoint >= 0x27d8 && codepoint <= 0x27d9) {
		return scientificab_block84[codepoint - 0x27d8];
	} else if (codepoint >= 0x27dc && codepoint <= 0x27de) {
		return scientificab_block85[codepoint - 0x27dc];
	} else if (codepoint >= 0x287d && codepoint <= 0x28ff) {
		return scientificab_block86[codepoint - 0x287d];
	} else if (codepoint >= 0x2919 && codepoint <= 0x291c) {
		return scientificab_block87[codepoint - 0x2919];
	} else if (codepoint >= 0x2b5e && codepoint <= 0x2b62) {
		return scientificab_block88[codepoint - 0x2b5e];
	} else if (codepoint == 0x2b64) {
		return scientificab_block89[0];
	} else if (codepoint >= 0x2b80 && codepoint <= 0x2b83) {
		return scientificab_block90[codepoint - 0x2b80];
	} else if (codepoint >= 0x30c3 && codepoint <= 0x30c4) {
		return scientificab_block91[codepoint - 0x30c3];
	} else if (codepoint == 0x5350) {
		return scientificab_block92[0];
	} else if (codepoint >= 0xe09e && codepoint <= 0xe0a2) {
		return scientificab_block93[codepoint - 0xe09e];
	} else if (codepoint >= 0xe0b0 && codepoint <= 0xe0b3) {
		return scientificab_block94[codepoint - 0xe0b0];
	} else if (codepoint >= 0xe0c0 && codepoint <= 0xe0c6) {
		return scientificab_block95[codepoint - 0xe0c0];
	} else if (codepoint == 0xe0d1) {
		return scientificab_block96[0];
	} else if (codepoint == 0xf031) {
		return scientificab_block97[0];
	} else if (codepoint >= 0xf033 && codepoint <= 0xf03d) {
		return scientificab_block98[codepoint - 0xf033];
	} else if (codepoint >= 0xf057 && codepoint <= 0xf059) {
		return scientificab_block99[codepoint - 0xf057];
	} else if (codepoint == 0xf061) {
		return scientificab_block100[0];
	} else if (codepoint == 0xf073) {
		return scientificab_block101[0];
	} else if (codepoint >= 0xf078 && codepoint <= 0xf079) {
		return scientificab_block102[codepoint - 0xf078];
	} else if (codepoint == 0xf07e) {
		return scientificab_block103[0];
	} else if (codepoint >= 0xf0cf && codepoint <= 0xf0d1) {
		return scientificab_block104[codepoint - 0xf0cf];
	} else if (codepoint >= 0xf0d5 && codepoint <= 0xf0dc) {
		return scientificab_block105[codepoint - 0xf0d5];
	} else if (codepoint >= 0xf0de && codepoint <= 0xf0e0) {
		return scientificab_block106[codepoint - 0xf0de];
	} else if (codepoint >= 0xf0ed && codepoint <= 0xf0f0) {
		return scientificab_block107[codepoint - 0xf0ed];
	} else {
		WARN("Codepoint U+%04X is not covered by this font", codepoint);
		return scientificab_block1[0];
	}
}

static const unsigned char*
    scientificai_get_bitmap(uint32_t codepoint)
{
	if (codepoint >= 0x20 && codepoint <= 0x7e) {
		return scientificai_block1[codepoint - 0x20];
	} else if (codepoint >= 0xa0 && codepoint <= 0x1b6) {
		return scientificai_block2[codepoint - 0xa0];
	} else if (codepoint >= 0x1c7 && codepoint <= 0x1d4) {
		return scientificai_block3[codepoint - 0x1c7];
	} else if (codepoint >= 0x20c && codepoint <= 0x21b) {
		return scientificai_block4[codepoint - 0x20c];
	} else if (codepoint == 0x296) {
		return scientificai_block5[0];
	} else if (codepoint == 0x298) {
		return scientificai_block6[0];
	} else if (codepoint == 0x2c7) {
		return scientificai_block7[0];
	} else if (codepoint == 0x2da) {
		return scientificai_block8[0];
	} else if (codepoint == 0x364) {
		return scientificai_block9[0];
	} else if (codepoint >= 0x391 && codepoint <= 0x3a9) {
		return scientificai_block10[codepoint - 0x391];
	} else if (codepoint >= 0x3b1 && codepoint <= 0x3c9) {
		return scientificai_block11[codepoint - 0x3b1];
	} else if (codepoint >= 0xf3c && codepoint <= 0xf3d) {
		return scientificai_block12[codepoint - 0xf3c];
	} else if (codepoint == 0x1e9e) {
		return scientificai_block13[0];
	} else if (codepoint >= 0x2010 && codepoint <= 0x2027) {
		return scientificai_block14[codepoint - 0x2010];
	} else if (codepoint >= 0x2032 && codepoint <= 0x203c) {
		return scientificai_block15[codepoint - 0x2032];
	} else if (codepoint >= 0x203e && codepoint <= 0x2042) {
		return scientificai_block16[codepoint - 0x203e];
	} else if (codepoint >= 0x204f && codepoint <= 0x2051) {
		return scientificai_block17[codepoint - 0x204f];
	} else if (codepoint == 0x205c) {
		return scientificai_block18[0];
	} else if (codepoint == 0x20a8) {
		return scientificai_block19[0];
	} else if (codepoint == 0x20aa) {
		return scientificai_block20[0];
	} else if (codepoint == 0x20ac) {
		return scientificai_block21[0];
	} else if (codepoint == 0x2122) {
		return scientificai_block22[0];
	} else if (codepoint >= 0x2190 && codepoint <= 0x2199) {
		return scientificai_block23[codepoint - 0x2190];
	} else if (codepoint >= 0x21a4 && codepoint <= 0x21a7) {
		return scientificai_block24[codepoint - 0x21a4];
	} else if (codepoint >= 0x21a9 && codepoint <= 0x21aa) {
		return scientificai_block25[codepoint - 0x21a9];
	} else if (codepoint >= 0x21b0 && codepoint <= 0x21b7) {
		return scientificai_block26[codepoint - 0x21b0];
	} else if (codepoint >= 0x21b9 && codepoint <= 0x21c4) {
		return scientificai_block27[codepoint - 0x21b9];
	} else if (codepoint >= 0x21c6 && codepoint <= 0x21cb) {
		return scientificai_block28[codepoint - 0x21c6];
	} else if (codepoint >= 0x21d0 && codepoint <= 0x21d3) {
		return scientificai_block29[codepoint - 0x21d0];
	} else if (codepoint >= 0x2200 && codepoint <= 0x2214) {
		return scientificai_block30[codepoint - 0x2200];
	} else if (codepoint >= 0x2217 && codepoint <= 0x221a) {
		return scientificai_block31[codepoint - 0x2217];
	} else if (codepoint >= 0x221d && codepoint <= 0x221e) {
		return scientificai_block32[codepoint - 0x221d];
	} else if (codepoint >= 0x2227 && codepoint <= 0x222b) {
		return scientificai_block33[codepoint - 0x2227];
	} else if (codepoint >= 0x2234 && codepoint <= 0x2237) {
		return scientificai_block34[codepoint - 0x2234];
	} else if (codepoint == 0x2243) {
		return scientificai_block35[0];
	} else if (codepoint == 0x2248) {
		return scientificai_block36[0];
	} else if (codepoint >= 0x2254 && codepoint <= 0x2255) {
		return scientificai_block37[codepoint - 0x2254];
	} else if (codepoint >= 0x2260 && codepoint <= 0x2262) {
		return scientificai_block38[codepoint - 0x2260];
	} else if (codepoint >= 0x2264 && codepoint <= 0x2265) {
		return scientificai_block39[codepoint - 0x2264];
	} else if (codepoint >= 0x2282 && codepoint <= 0x2287) {
		return scientificai_block40[codepoint - 0x2282];
	} else if (codepoint >= 0x2295 && codepoint <= 0x2296) {
		return scientificai_block41[codepoint - 0x2295];
	} else if (codepoint == 0x2299) {
		return scientificai_block42[0];
	} else if (codepoint >= 0x229e && codepoint <= 0x22a9) {
		return scientificai_block43[codepoint - 0x229e];
	} else if (codepoint == 0x22ab) {
		return scientificai_block44[0];
	} else if (codepoint >= 0x22c4 && codepoint <= 0x22c6) {
		return scientificai_block45[codepoint - 0x22c4];
	} else if (codepoint == 0x2325) {
		return scientificai_block46[0];
	} else if (codepoint >= 0x239b && codepoint <= 0x23ae) {
		return scientificai_block47[codepoint - 0x239b];
	} else if (codepoint >= 0x23ba && codepoint <= 0x23bd) {
		return scientificai_block48[codepoint - 0x23ba];
	} else if (codepoint >= 0x23e9 && codepoint <= 0x23ec) {
		return scientificai_block49[codepoint - 0x23e9];
	} else if (codepoint >= 0x23f4 && codepoint <= 0x23fb) {
		return scientificai_block50[codepoint - 0x23f4];
	} else if (codepoint >= 0x2500 && codepoint <= 0x2503) {
		return scientificai_block51[codepoint - 0x2500];
	} else if (codepoint >= 0x2506 && codepoint <= 0x2507) {
		return scientificai_block52[codepoint - 0x2506];
	} else if (codepoint >= 0x250a && codepoint <= 0x251d) {
		return scientificai_block53[codepoint - 0x250a];
	} else if (codepoint == 0x2520) {
		return scientificai_block54[0];
	} else if (codepoint >= 0x2523 && codepoint <= 0x2525) {
		return scientificai_block55[codepoint - 0x2523];
	} else if (codepoint == 0x2528) {
		return scientificai_block56[0];
	} else if (codepoint >= 0x252b && codepoint <= 0x252c) {
		return scientificai_block57[codepoint - 0x252b];
	} else if (codepoint >= 0x252f && codepoint <= 0x2530) {
		return scientificai_block58[codepoint - 0x252f];
	} else if (codepoint >= 0x2533 && codepoint <= 0x2534) {
		return scientificai_block59[codepoint - 0x2533];
	} else if (codepoint >= 0x2537 && codepoint <= 0x2538) {
		return scientificai_block60[codepoint - 0x2537];
	} else if (codepoint >= 0x253b && codepoint <= 0x253c) {
		return scientificai_block61[codepoint - 0x253b];
	} else if (codepoint == 0x253f) {
		return scientificai_block62[0];
	} else if (codepoint == 0x2542) {
		return scientificai_block63[0];
	} else if (codepoint == 0x254b) {
		return scientificai_block64[0];
	} else if (codepoint >= 0x2550 && codepoint <= 0x2573) {
		return scientificai_block65[codepoint - 0x2550];
	} else if (codepoint == 0x257c) {
		return scientificai_block66[0];
	} else if (codepoint == 0x257e) {
		return scientificai_block67[0];
	} else if (codepoint >= 0x2581 && codepoint <= 0x258a) {
		return scientificai_block68[codepoint - 0x2581];
	} else if (codepoint == 0x258f) {
		return scientificai_block69[0];
	} else if (codepoint >= 0x2591 && codepoint <= 0x2593) {
		return scientificai_block70[codepoint - 0x2591];
	} else if (codepoint >= 0x25a1 && codepoint <= 0x25a2) {
		return scientificai_block71[codepoint - 0x25a1];
	} else if (codepoint >= 0x25aa && codepoint <= 0x25ab) {
		return scientificai_block72[codepoint - 0x25aa];
	} else if (codepoint >= 0x25b2 && codepoint <= 0x25b9) {
		return scientificai_block73[codepoint - 0x25b2];
	} else if (codepoint >= 0x25bc && codepoint <= 0x25be) {
		return scientificai_block74[codepoint - 0x25bc];
	} else if (codepoint >= 0x25c0 && codepoint <= 0x25c4) {
		return scientificai_block75[codepoint - 0x25c0];
	} else if (codepoint == 0x25cb) {
		return scientificai_block76[0];
	} else if (codepoint >= 0x25ce && codepoint <= 0x25d7) {
		return scientificai_block77[codepoint - 0x25ce];
	} else if (codepoint >= 0x25e7 && codepoint <= 0x25eb) {
		return scientificai_block78[codepoint - 0x25e7];
	} else if (codepoint >= 0x25f0 && codepoint <= 0x25f3) {
		return scientificai_block79[codepoint - 0x25f0];
	} else if (codepoint >= 0x2600 && codepoint <= 0x2602) {
		return scientificai_block80[codepoint - 0x2600];
	} else if (codepoint >= 0x2630 && codepoint <= 0x2637) {
		return scientificai_block81[codepoint - 0x2630];
	} else if (codepoint == 0x2661) {
		return scientificai_block82[0];
	} else if (codepoint >= 0x2665 && codepoint <= 0x2666) {
		return scientificai_block83[codepoint - 0x2665];
	} else if (codepoint >= 0x2669 && codepoint <= 0x266c) {
		return scientificai_block84[codepoint - 0x2669];
	} else if (codepoint == 0x267a) {
		return scientificai_block85[0];
	} else if (codepoint >= 0x26aa && codepoint <= 0x26ab) {
		return scientificai_block86[codepoint - 0x26aa];
	} else if (codepoint >= 0x2713 && codepoint <= 0x2718) {
		return scientificai_block87[codepoint - 0x2713];
	} else if (codepoint == 0x272e) {
		return scientificai_block88[0];
	} else if (codepoint == 0x2744) {
		return scientificai_block89[0];
	} else if (codepoint >= 0x276e && codepoint <= 0x2771) {
		return scientificai_block90[codepoint - 0x276e];
	} else if (codepoint >= 0x27c2 && codepoint <= 0x27c4) {
		return scientificai_block91[codepoint - 0x27c2];
	} else if (codepoint >= 0x27d8 && codepoint <= 0x27d9) {
		return scientificai_block92[codepoint - 0x27d8];
	} else if (codepoint >= 0x27dc && codepoint <= 0x27de) {
		return scientificai_block93[codepoint - 0x27dc];
	} else if (codepoint >= 0x2864 && codepoint <= 0x28ff) {
		return scientificai_block94[codepoint - 0x2864];
	} else if (codepoint >= 0x2919 && codepoint <= 0x291c) {
		return scientificai_block95[codepoint - 0x2919];
	} else if (codepoint >= 0x2b5e && codepoint <= 0x2b62) {
		return scientificai_block96[codepoint - 0x2b5e];
	} else if (codepoint == 0x2b64) {
		return scientificai_block97[0];
	} else if (codepoint >= 0x2b80 && codepoint <= 0x2b83) {
		return scientificai_block98[codepoint - 0x2b80];
	} else if (codepoint >= 0x30c3 && codepoint <= 0x30c4) {
		return scientificai_block99[codepoint - 0x30c3];
	} else if (codepoint == 0x5350) {
		return scientificai_block100[0];
	} else if (codepoint >= 0xe09e && codepoint <= 0xe0a2) {
		return scientificai_block101[codepoint - 0xe09e];
	} else if (codepoint >= 0xe0b0 && codepoint <= 0xe0b5) {
		return scientificai_block102[codepoint - 0xe0b0];
	} else if (codepoint >= 0xe0c0 && codepoint <= 0xe0c6) {
		return scientificai_block103[codepoint - 0xe0c0];
	} else if (codepoint == 0xe0d1) {
		return scientificai_block104[0];
	} else if (codepoint == 0xf031) {
		return scientificai_block105[0];
	} else if (codepoint >= 0xf033 && codepoint <= 0xf03d) {
		return scientificai_block106[codepoint - 0xf033];
	} else if (codepoint >= 0xf057 && codepoint <= 0xf059) {
		return scientificai_block107[codepoint - 0xf057];
	} else if (codepoint == 0xf061) {
		return scientificai_block108[0];
	} else if (codepoint == 0xf073) {
		return scientificai_block109[0];
	} else if (codepoint >= 0xf078 && codepoint <= 0xf079) {
		return scientificai_block110[codepoint - 0xf078];
	} else if (codepoint == 0xf07e) {
		return scientificai_block111[0];
	} else if (codepoint >= 0xf0cf && codepoint <= 0xf0d1) {
		return scientificai_block112[codepoint - 0xf0cf];
	} else if (codepoint >= 0xf0d5 && codepoint <= 0xf0dc) {
		return scientificai_block113[codepoint - 0xf0d5];
	} else if (codepoint >= 0xf0de && codepoint <= 0xf0e0) {
		return scientificai_block114[codepoint - 0xf0de];
	} else if (codepoint >= 0xf0ed && codepoint <= 0xf0f0) {
		return scientificai_block115[codepoint - 0xf0ed];
	} else {
		WARN("Codepoint U+%04X is not covered by this font", codepoint);
		return scientificai_block1[0];
	}
}
