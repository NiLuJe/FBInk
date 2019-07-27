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

#include "fbink_terminus.h"

static const unsigned char*
    terminus_get_bitmap(uint32_t codepoint)
{
	if (codepoint <= 0x00) {
		return terminus_block1[codepoint];
	} else if (codepoint >= 0x20 && codepoint <= 0x7e) {
		return terminus_block2[codepoint - 0x20];
	} else if (codepoint >= 0xa0 && codepoint <= 0x17f) {
		return terminus_block3[codepoint - 0xa0];
	} else if (codepoint == 0x186) {
		return terminus_block4[0];
	} else if (codepoint >= 0x18e && codepoint <= 0x190) {
		return terminus_block5[codepoint - 0x18e];
	} else if (codepoint == 0x192) {
		return terminus_block6[0];
	} else if (codepoint >= 0x19d && codepoint <= 0x19e) {
		return terminus_block7[codepoint - 0x19d];
	} else if (codepoint >= 0x1b5 && codepoint <= 0x1b7) {
		return terminus_block8[codepoint - 0x1b5];
	} else if (codepoint >= 0x1cd && codepoint <= 0x1d4) {
		return terminus_block9[codepoint - 0x1cd];
	} else if (codepoint >= 0x1e2 && codepoint <= 0x1f0) {
		return terminus_block10[codepoint - 0x1e2];
	} else if (codepoint >= 0x1f4 && codepoint <= 0x1f5) {
		return terminus_block11[codepoint - 0x1f4];
	} else if (codepoint >= 0x1fc && codepoint <= 0x1ff) {
		return terminus_block12[codepoint - 0x1fc];
	} else if (codepoint >= 0x218 && codepoint <= 0x21b) {
		return terminus_block13[codepoint - 0x218];
	} else if (codepoint >= 0x232 && codepoint <= 0x233) {
		return terminus_block14[codepoint - 0x232];
	} else if (codepoint == 0x237) {
		return terminus_block15[0];
	} else if (codepoint == 0x254) {
		return terminus_block16[0];
	} else if (codepoint >= 0x258 && codepoint <= 0x259) {
		return terminus_block17[codepoint - 0x258];
	} else if (codepoint == 0x25b) {
		return terminus_block18[0];
	} else if (codepoint == 0x272) {
		return terminus_block19[0];
	} else if (codepoint == 0x292) {
		return terminus_block20[0];
	} else if (codepoint >= 0x2bb && codepoint <= 0x2bd) {
		return terminus_block21[codepoint - 0x2bb];
	} else if (codepoint >= 0x2c6 && codepoint <= 0x2c7) {
		return terminus_block22[codepoint - 0x2c6];
	} else if (codepoint >= 0x2d8 && codepoint <= 0x2d9) {
		return terminus_block23[codepoint - 0x2d8];
	} else if (codepoint >= 0x2db && codepoint <= 0x2dd) {
		return terminus_block24[codepoint - 0x2db];
	} else if (codepoint >= 0x300 && codepoint <= 0x308) {
		return terminus_block25[codepoint - 0x300];
	} else if (codepoint >= 0x30a && codepoint <= 0x30c) {
		return terminus_block26[codepoint - 0x30a];
	} else if (codepoint == 0x329) {
		return terminus_block27[0];
	} else if (codepoint >= 0x384 && codepoint <= 0x38a) {
		return terminus_block28[codepoint - 0x384];
	} else if (codepoint == 0x38c) {
		return terminus_block29[0];
	} else if (codepoint >= 0x38e && codepoint <= 0x3a1) {
		return terminus_block30[codepoint - 0x38e];
	} else if (codepoint >= 0x3a3 && codepoint <= 0x3ce) {
		return terminus_block31[codepoint - 0x3a3];
	} else if (codepoint == 0x3d1) {
		return terminus_block32[0];
	} else if (codepoint == 0x3d5) {
		return terminus_block33[0];
	} else if (codepoint >= 0x3f0 && codepoint <= 0x3f6) {
		return terminus_block34[codepoint - 0x3f0];
	} else if (codepoint >= 0x400 && codepoint <= 0x45f) {
		return terminus_block35[codepoint - 0x400];
	} else if (codepoint >= 0x462 && codepoint <= 0x463) {
		return terminus_block36[codepoint - 0x462];
	} else if (codepoint >= 0x46a && codepoint <= 0x46b) {
		return terminus_block37[codepoint - 0x46a];
	} else if (codepoint >= 0x490 && codepoint <= 0x49d) {
		return terminus_block38[codepoint - 0x490];
	} else if (codepoint >= 0x4a0 && codepoint <= 0x4a5) {
		return terminus_block39[codepoint - 0x4a0];
	} else if (codepoint >= 0x4aa && codepoint <= 0x4ab) {
		return terminus_block40[codepoint - 0x4aa];
	} else if (codepoint >= 0x4ae && codepoint <= 0x4b3) {
		return terminus_block41[codepoint - 0x4ae];
	} else if (codepoint >= 0x4b6 && codepoint <= 0x4bb) {
		return terminus_block42[codepoint - 0x4b6];
	} else if (codepoint >= 0x4c0 && codepoint <= 0x4c2) {
		return terminus_block43[codepoint - 0x4c0];
	} else if (codepoint >= 0x4cf && codepoint <= 0x4df) {
		return terminus_block44[codepoint - 0x4cf];
	} else if (codepoint >= 0x4e2 && codepoint <= 0x4f5) {
		return terminus_block45[codepoint - 0x4e2];
	} else if (codepoint >= 0x4f8 && codepoint <= 0x4f9) {
		return terminus_block46[codepoint - 0x4f8];
	} else if (codepoint >= 0x1e0c && codepoint <= 0x1e0d) {
		return terminus_block47[codepoint - 0x1e0c];
	} else if (codepoint >= 0x1e34 && codepoint <= 0x1e37) {
		return terminus_block48[codepoint - 0x1e34];
	} else if (codepoint >= 0x1e40 && codepoint <= 0x1e47) {
		return terminus_block49[codepoint - 0x1e40];
	} else if (codepoint >= 0x1e6c && codepoint <= 0x1e6d) {
		return terminus_block50[codepoint - 0x1e6c];
	} else if (codepoint >= 0x1eb8 && codepoint <= 0x1eb9) {
		return terminus_block51[codepoint - 0x1eb8];
	} else if (codepoint >= 0x1ebc && codepoint <= 0x1ebd) {
		return terminus_block52[codepoint - 0x1ebc];
	} else if (codepoint >= 0x1eca && codepoint <= 0x1ecd) {
		return terminus_block53[codepoint - 0x1eca];
	} else if (codepoint >= 0x1ee4 && codepoint <= 0x1ee5) {
		return terminus_block54[codepoint - 0x1ee4];
	} else if (codepoint >= 0x1ef8 && codepoint <= 0x1ef9) {
		return terminus_block55[codepoint - 0x1ef8];
	} else if (codepoint >= 0x2000 && codepoint <= 0x2022) {
		return terminus_block56[codepoint - 0x2000];
	} else if (codepoint == 0x2026) {
		return terminus_block57[0];
	} else if (codepoint == 0x2030) {
		return terminus_block58[0];
	} else if (codepoint >= 0x2032 && codepoint <= 0x2033) {
		return terminus_block59[codepoint - 0x2032];
	} else if (codepoint >= 0x2039 && codepoint <= 0x203a) {
		return terminus_block60[codepoint - 0x2039];
	} else if (codepoint == 0x203c) {
		return terminus_block61[0];
	} else if (codepoint == 0x203e) {
		return terminus_block62[0];
	} else if (codepoint >= 0x2070 && codepoint <= 0x2071) {
		return terminus_block63[codepoint - 0x2070];
	} else if (codepoint >= 0x2074 && codepoint <= 0x208e) {
		return terminus_block64[codepoint - 0x2074];
	} else if (codepoint >= 0x2090 && codepoint <= 0x2098) {
		return terminus_block65[codepoint - 0x2090];
	} else if (codepoint == 0x209a) {
		return terminus_block66[0];
	} else if (codepoint == 0x20a7) {
		return terminus_block67[0];
	} else if (codepoint == 0x20ac) {
		return terminus_block68[0];
	} else if (codepoint == 0x20ae) {
		return terminus_block69[0];
	} else if (codepoint == 0x2102) {
		return terminus_block70[0];
	} else if (codepoint >= 0x210e && codepoint <= 0x210f) {
		return terminus_block71[codepoint - 0x210e];
	} else if (codepoint >= 0x2115 && codepoint <= 0x2116) {
		return terminus_block72[codepoint - 0x2115];
	} else if (codepoint == 0x211a) {
		return terminus_block73[0];
	} else if (codepoint == 0x211d) {
		return terminus_block74[0];
	} else if (codepoint == 0x2122) {
		return terminus_block75[0];
	} else if (codepoint == 0x2124) {
		return terminus_block76[0];
	} else if (codepoint == 0x2126) {
		return terminus_block77[0];
	} else if (codepoint == 0x2135) {
		return terminus_block78[0];
	} else if (codepoint >= 0x2190 && codepoint <= 0x2195) {
		return terminus_block79[codepoint - 0x2190];
	} else if (codepoint == 0x21a4) {
		return terminus_block80[0];
	} else if (codepoint == 0x21a6) {
		return terminus_block81[0];
	} else if (codepoint == 0x21a8) {
		return terminus_block82[0];
	} else if (codepoint == 0x21b5) {
		return terminus_block83[0];
	} else if (codepoint == 0x21bb) {
		return terminus_block84[0];
	} else if (codepoint >= 0x21cb && codepoint <= 0x21cc) {
		return terminus_block85[codepoint - 0x21cb];
	} else if (codepoint >= 0x21d0 && codepoint <= 0x21d5) {
		return terminus_block86[codepoint - 0x21d0];
	} else if (codepoint == 0x2200) {
		return terminus_block87[0];
	} else if (codepoint >= 0x2203 && codepoint <= 0x220d) {
		return terminus_block88[codepoint - 0x2203];
	} else if (codepoint >= 0x2212 && codepoint <= 0x2216) {
		return terminus_block89[codepoint - 0x2212];
	} else if (codepoint >= 0x2219 && codepoint <= 0x221a) {
		return terminus_block90[codepoint - 0x2219];
	} else if (codepoint >= 0x221e && codepoint <= 0x221f) {
		return terminus_block91[codepoint - 0x221e];
	} else if (codepoint == 0x2225) {
		return terminus_block92[0];
	} else if (codepoint >= 0x2227 && codepoint <= 0x222a) {
		return terminus_block93[codepoint - 0x2227];
	} else if (codepoint == 0x2248) {
		return terminus_block94[0];
	} else if (codepoint >= 0x2260 && codepoint <= 0x2261) {
		return terminus_block95[codepoint - 0x2260];
	} else if (codepoint >= 0x2264 && codepoint <= 0x2265) {
		return terminus_block96[codepoint - 0x2264];
	} else if (codepoint >= 0x226a && codepoint <= 0x226b) {
		return terminus_block97[codepoint - 0x226a];
	} else if (codepoint >= 0x2282 && codepoint <= 0x2283) {
		return terminus_block98[codepoint - 0x2282];
	} else if (codepoint >= 0x2286 && codepoint <= 0x2287) {
		return terminus_block99[codepoint - 0x2286];
	} else if (codepoint == 0x22a5) {
		return terminus_block100[0];
	} else if (codepoint >= 0x22c2 && codepoint <= 0x22c3) {
		return terminus_block101[codepoint - 0x22c2];
	} else if (codepoint == 0x2300) {
		return terminus_block102[0];
	} else if (codepoint == 0x2302) {
		return terminus_block103[0];
	} else if (codepoint >= 0x2308 && codepoint <= 0x230b) {
		return terminus_block104[codepoint - 0x2308];
	} else if (codepoint == 0x2310) {
		return terminus_block105[0];
	} else if (codepoint == 0x2319) {
		return terminus_block106[0];
	} else if (codepoint >= 0x2320 && codepoint <= 0x2321) {
		return terminus_block107[codepoint - 0x2320];
	} else if (codepoint >= 0x239b && codepoint <= 0x23a9) {
		return terminus_block108[codepoint - 0x239b];
	} else if (codepoint >= 0x23ab && codepoint <= 0x23af) {
		return terminus_block109[codepoint - 0x23ab];
	} else if (codepoint >= 0x23ba && codepoint <= 0x23bd) {
		return terminus_block110[codepoint - 0x23ba];
	} else if (codepoint == 0x23d0) {
		return terminus_block111[0];
	} else if (codepoint >= 0x2409 && codepoint <= 0x240d) {
		return terminus_block112[codepoint - 0x2409];
	} else if (codepoint == 0x2424) {
		return terminus_block113[0];
	} else if (codepoint >= 0x2500 && codepoint <= 0x2503) {
		return terminus_block114[codepoint - 0x2500];
	} else if (codepoint >= 0x2508 && codepoint <= 0x254b) {
		return terminus_block115[codepoint - 0x2508];
	} else if (codepoint >= 0x2550 && codepoint <= 0x2593) {
		return terminus_block116[codepoint - 0x2550];
	} else if (codepoint >= 0x2596 && codepoint <= 0x25a0) {
		return terminus_block117[codepoint - 0x2596];
	} else if (codepoint == 0x25ac) {
		return terminus_block118[0];
	} else if (codepoint == 0x25ae) {
		return terminus_block119[0];
	} else if (codepoint == 0x25b2) {
		return terminus_block120[0];
	} else if (codepoint == 0x25b6) {
		return terminus_block121[0];
	} else if (codepoint == 0x25ba) {
		return terminus_block122[0];
	} else if (codepoint == 0x25bc) {
		return terminus_block123[0];
	} else if (codepoint == 0x25c0) {
		return terminus_block124[0];
	} else if (codepoint == 0x25c4) {
		return terminus_block125[0];
	} else if (codepoint == 0x25c6) {
		return terminus_block126[0];
	} else if (codepoint >= 0x25ca && codepoint <= 0x25cb) {
		return terminus_block127[codepoint - 0x25ca];
	} else if (codepoint == 0x25cf) {
		return terminus_block128[0];
	} else if (codepoint >= 0x25d8 && codepoint <= 0x25d9) {
		return terminus_block129[codepoint - 0x25d8];
	} else if (codepoint >= 0x263a && codepoint <= 0x263c) {
		return terminus_block130[codepoint - 0x263a];
	} else if (codepoint == 0x2640) {
		return terminus_block131[0];
	} else if (codepoint == 0x2642) {
		return terminus_block132[0];
	} else if (codepoint == 0x2660) {
		return terminus_block133[0];
	} else if (codepoint == 0x2663) {
		return terminus_block134[0];
	} else if (codepoint >= 0x2665 && codepoint <= 0x2666) {
		return terminus_block135[codepoint - 0x2665];
	} else if (codepoint >= 0x266a && codepoint <= 0x266b) {
		return terminus_block136[codepoint - 0x266a];
	} else if (codepoint >= 0x2713 && codepoint <= 0x2714) {
		return terminus_block137[codepoint - 0x2713];
	} else if (codepoint >= 0x2717 && codepoint <= 0x2718) {
		return terminus_block138[codepoint - 0x2717];
	} else if (codepoint >= 0x27e8 && codepoint <= 0x27eb) {
		return terminus_block139[codepoint - 0x27e8];
	} else if (codepoint >= 0x2800 && codepoint <= 0x28ff) {
		return terminus_block140[codepoint - 0x2800];
	} else if (codepoint == 0x2e2c) {
		return terminus_block141[0];
	} else if (codepoint >= 0xe0a0 && codepoint <= 0xe0a2) {
		return terminus_block142[codepoint - 0xe0a0];
	} else if (codepoint >= 0xe0b0 && codepoint <= 0xe0b3) {
		return terminus_block143[codepoint - 0xe0b0];
	} else if (codepoint == 0xf6be) {
		return terminus_block144[0];
	} else if (codepoint == 0xfffd) {
		return terminus_block145[0];
	} else {
		WARN("Codepoint U+%04X is not covered by this font", codepoint);
		return terminus_block1[0];
	}
}

static const unsigned char*
    terminusb_get_bitmap(uint32_t codepoint)
{
	if (codepoint <= 0x00) {
		return terminusb_block1[codepoint];
	} else if (codepoint >= 0x20 && codepoint <= 0x7e) {
		return terminusb_block2[codepoint - 0x20];
	} else if (codepoint >= 0xa0 && codepoint <= 0x17f) {
		return terminusb_block3[codepoint - 0xa0];
	} else if (codepoint == 0x186) {
		return terminusb_block4[0];
	} else if (codepoint >= 0x18e && codepoint <= 0x190) {
		return terminusb_block5[codepoint - 0x18e];
	} else if (codepoint == 0x192) {
		return terminusb_block6[0];
	} else if (codepoint >= 0x19d && codepoint <= 0x19e) {
		return terminusb_block7[codepoint - 0x19d];
	} else if (codepoint >= 0x1b5 && codepoint <= 0x1b7) {
		return terminusb_block8[codepoint - 0x1b5];
	} else if (codepoint >= 0x1cd && codepoint <= 0x1d4) {
		return terminusb_block9[codepoint - 0x1cd];
	} else if (codepoint >= 0x1e2 && codepoint <= 0x1f0) {
		return terminusb_block10[codepoint - 0x1e2];
	} else if (codepoint >= 0x1f4 && codepoint <= 0x1f5) {
		return terminusb_block11[codepoint - 0x1f4];
	} else if (codepoint >= 0x1fc && codepoint <= 0x1ff) {
		return terminusb_block12[codepoint - 0x1fc];
	} else if (codepoint >= 0x218 && codepoint <= 0x21b) {
		return terminusb_block13[codepoint - 0x218];
	} else if (codepoint >= 0x232 && codepoint <= 0x233) {
		return terminusb_block14[codepoint - 0x232];
	} else if (codepoint == 0x237) {
		return terminusb_block15[0];
	} else if (codepoint == 0x254) {
		return terminusb_block16[0];
	} else if (codepoint >= 0x258 && codepoint <= 0x259) {
		return terminusb_block17[codepoint - 0x258];
	} else if (codepoint == 0x25b) {
		return terminusb_block18[0];
	} else if (codepoint == 0x272) {
		return terminusb_block19[0];
	} else if (codepoint == 0x292) {
		return terminusb_block20[0];
	} else if (codepoint >= 0x2bb && codepoint <= 0x2bd) {
		return terminusb_block21[codepoint - 0x2bb];
	} else if (codepoint >= 0x2c6 && codepoint <= 0x2c7) {
		return terminusb_block22[codepoint - 0x2c6];
	} else if (codepoint >= 0x2d8 && codepoint <= 0x2d9) {
		return terminusb_block23[codepoint - 0x2d8];
	} else if (codepoint >= 0x2db && codepoint <= 0x2dd) {
		return terminusb_block24[codepoint - 0x2db];
	} else if (codepoint >= 0x300 && codepoint <= 0x308) {
		return terminusb_block25[codepoint - 0x300];
	} else if (codepoint >= 0x30a && codepoint <= 0x30c) {
		return terminusb_block26[codepoint - 0x30a];
	} else if (codepoint == 0x329) {
		return terminusb_block27[0];
	} else if (codepoint >= 0x384 && codepoint <= 0x38a) {
		return terminusb_block28[codepoint - 0x384];
	} else if (codepoint == 0x38c) {
		return terminusb_block29[0];
	} else if (codepoint >= 0x38e && codepoint <= 0x3a1) {
		return terminusb_block30[codepoint - 0x38e];
	} else if (codepoint >= 0x3a3 && codepoint <= 0x3ce) {
		return terminusb_block31[codepoint - 0x3a3];
	} else if (codepoint == 0x3d1) {
		return terminusb_block32[0];
	} else if (codepoint == 0x3d5) {
		return terminusb_block33[0];
	} else if (codepoint >= 0x3f0 && codepoint <= 0x3f6) {
		return terminusb_block34[codepoint - 0x3f0];
	} else if (codepoint >= 0x400 && codepoint <= 0x45f) {
		return terminusb_block35[codepoint - 0x400];
	} else if (codepoint >= 0x462 && codepoint <= 0x463) {
		return terminusb_block36[codepoint - 0x462];
	} else if (codepoint >= 0x46a && codepoint <= 0x46b) {
		return terminusb_block37[codepoint - 0x46a];
	} else if (codepoint >= 0x490 && codepoint <= 0x49d) {
		return terminusb_block38[codepoint - 0x490];
	} else if (codepoint >= 0x4a0 && codepoint <= 0x4a5) {
		return terminusb_block39[codepoint - 0x4a0];
	} else if (codepoint >= 0x4aa && codepoint <= 0x4ab) {
		return terminusb_block40[codepoint - 0x4aa];
	} else if (codepoint >= 0x4ae && codepoint <= 0x4b3) {
		return terminusb_block41[codepoint - 0x4ae];
	} else if (codepoint >= 0x4b6 && codepoint <= 0x4bb) {
		return terminusb_block42[codepoint - 0x4b6];
	} else if (codepoint >= 0x4c0 && codepoint <= 0x4c2) {
		return terminusb_block43[codepoint - 0x4c0];
	} else if (codepoint >= 0x4cf && codepoint <= 0x4df) {
		return terminusb_block44[codepoint - 0x4cf];
	} else if (codepoint >= 0x4e2 && codepoint <= 0x4f5) {
		return terminusb_block45[codepoint - 0x4e2];
	} else if (codepoint >= 0x4f8 && codepoint <= 0x4f9) {
		return terminusb_block46[codepoint - 0x4f8];
	} else if (codepoint >= 0x1e0c && codepoint <= 0x1e0d) {
		return terminusb_block47[codepoint - 0x1e0c];
	} else if (codepoint >= 0x1e34 && codepoint <= 0x1e37) {
		return terminusb_block48[codepoint - 0x1e34];
	} else if (codepoint >= 0x1e40 && codepoint <= 0x1e47) {
		return terminusb_block49[codepoint - 0x1e40];
	} else if (codepoint >= 0x1e6c && codepoint <= 0x1e6d) {
		return terminusb_block50[codepoint - 0x1e6c];
	} else if (codepoint >= 0x1eb8 && codepoint <= 0x1eb9) {
		return terminusb_block51[codepoint - 0x1eb8];
	} else if (codepoint >= 0x1ebc && codepoint <= 0x1ebd) {
		return terminusb_block52[codepoint - 0x1ebc];
	} else if (codepoint >= 0x1eca && codepoint <= 0x1ecd) {
		return terminusb_block53[codepoint - 0x1eca];
	} else if (codepoint >= 0x1ee4 && codepoint <= 0x1ee5) {
		return terminusb_block54[codepoint - 0x1ee4];
	} else if (codepoint >= 0x1ef8 && codepoint <= 0x1ef9) {
		return terminusb_block55[codepoint - 0x1ef8];
	} else if (codepoint >= 0x2000 && codepoint <= 0x2022) {
		return terminusb_block56[codepoint - 0x2000];
	} else if (codepoint == 0x2026) {
		return terminusb_block57[0];
	} else if (codepoint == 0x2030) {
		return terminusb_block58[0];
	} else if (codepoint >= 0x2032 && codepoint <= 0x2033) {
		return terminusb_block59[codepoint - 0x2032];
	} else if (codepoint >= 0x2039 && codepoint <= 0x203a) {
		return terminusb_block60[codepoint - 0x2039];
	} else if (codepoint == 0x203c) {
		return terminusb_block61[0];
	} else if (codepoint == 0x203e) {
		return terminusb_block62[0];
	} else if (codepoint >= 0x2070 && codepoint <= 0x2071) {
		return terminusb_block63[codepoint - 0x2070];
	} else if (codepoint >= 0x2074 && codepoint <= 0x208e) {
		return terminusb_block64[codepoint - 0x2074];
	} else if (codepoint >= 0x2090 && codepoint <= 0x2098) {
		return terminusb_block65[codepoint - 0x2090];
	} else if (codepoint == 0x209a) {
		return terminusb_block66[0];
	} else if (codepoint == 0x20a7) {
		return terminusb_block67[0];
	} else if (codepoint == 0x20ac) {
		return terminusb_block68[0];
	} else if (codepoint == 0x20ae) {
		return terminusb_block69[0];
	} else if (codepoint == 0x2102) {
		return terminusb_block70[0];
	} else if (codepoint >= 0x210e && codepoint <= 0x210f) {
		return terminusb_block71[codepoint - 0x210e];
	} else if (codepoint >= 0x2115 && codepoint <= 0x2116) {
		return terminusb_block72[codepoint - 0x2115];
	} else if (codepoint == 0x211a) {
		return terminusb_block73[0];
	} else if (codepoint == 0x211d) {
		return terminusb_block74[0];
	} else if (codepoint == 0x2122) {
		return terminusb_block75[0];
	} else if (codepoint == 0x2124) {
		return terminusb_block76[0];
	} else if (codepoint == 0x2126) {
		return terminusb_block77[0];
	} else if (codepoint == 0x2135) {
		return terminusb_block78[0];
	} else if (codepoint >= 0x2190 && codepoint <= 0x2195) {
		return terminusb_block79[codepoint - 0x2190];
	} else if (codepoint == 0x21a4) {
		return terminusb_block80[0];
	} else if (codepoint == 0x21a6) {
		return terminusb_block81[0];
	} else if (codepoint == 0x21a8) {
		return terminusb_block82[0];
	} else if (codepoint == 0x21b5) {
		return terminusb_block83[0];
	} else if (codepoint == 0x21bb) {
		return terminusb_block84[0];
	} else if (codepoint >= 0x21cb && codepoint <= 0x21cc) {
		return terminusb_block85[codepoint - 0x21cb];
	} else if (codepoint >= 0x21d0 && codepoint <= 0x21d5) {
		return terminusb_block86[codepoint - 0x21d0];
	} else if (codepoint == 0x2200) {
		return terminusb_block87[0];
	} else if (codepoint >= 0x2203 && codepoint <= 0x220d) {
		return terminusb_block88[codepoint - 0x2203];
	} else if (codepoint >= 0x2212 && codepoint <= 0x2216) {
		return terminusb_block89[codepoint - 0x2212];
	} else if (codepoint >= 0x2219 && codepoint <= 0x221a) {
		return terminusb_block90[codepoint - 0x2219];
	} else if (codepoint >= 0x221e && codepoint <= 0x221f) {
		return terminusb_block91[codepoint - 0x221e];
	} else if (codepoint == 0x2225) {
		return terminusb_block92[0];
	} else if (codepoint >= 0x2227 && codepoint <= 0x222a) {
		return terminusb_block93[codepoint - 0x2227];
	} else if (codepoint == 0x2248) {
		return terminusb_block94[0];
	} else if (codepoint >= 0x2260 && codepoint <= 0x2261) {
		return terminusb_block95[codepoint - 0x2260];
	} else if (codepoint >= 0x2264 && codepoint <= 0x2265) {
		return terminusb_block96[codepoint - 0x2264];
	} else if (codepoint >= 0x226a && codepoint <= 0x226b) {
		return terminusb_block97[codepoint - 0x226a];
	} else if (codepoint >= 0x2282 && codepoint <= 0x2283) {
		return terminusb_block98[codepoint - 0x2282];
	} else if (codepoint >= 0x2286 && codepoint <= 0x2287) {
		return terminusb_block99[codepoint - 0x2286];
	} else if (codepoint == 0x22a5) {
		return terminusb_block100[0];
	} else if (codepoint >= 0x22c2 && codepoint <= 0x22c3) {
		return terminusb_block101[codepoint - 0x22c2];
	} else if (codepoint == 0x2300) {
		return terminusb_block102[0];
	} else if (codepoint == 0x2302) {
		return terminusb_block103[0];
	} else if (codepoint >= 0x2308 && codepoint <= 0x230b) {
		return terminusb_block104[codepoint - 0x2308];
	} else if (codepoint == 0x2310) {
		return terminusb_block105[0];
	} else if (codepoint == 0x2319) {
		return terminusb_block106[0];
	} else if (codepoint >= 0x2320 && codepoint <= 0x2321) {
		return terminusb_block107[codepoint - 0x2320];
	} else if (codepoint >= 0x239b && codepoint <= 0x23a9) {
		return terminusb_block108[codepoint - 0x239b];
	} else if (codepoint >= 0x23ab && codepoint <= 0x23af) {
		return terminusb_block109[codepoint - 0x23ab];
	} else if (codepoint >= 0x23ba && codepoint <= 0x23bd) {
		return terminusb_block110[codepoint - 0x23ba];
	} else if (codepoint == 0x23d0) {
		return terminusb_block111[0];
	} else if (codepoint >= 0x2409 && codepoint <= 0x240d) {
		return terminusb_block112[codepoint - 0x2409];
	} else if (codepoint == 0x2424) {
		return terminusb_block113[0];
	} else if (codepoint >= 0x2500 && codepoint <= 0x2503) {
		return terminusb_block114[codepoint - 0x2500];
	} else if (codepoint >= 0x2508 && codepoint <= 0x254b) {
		return terminusb_block115[codepoint - 0x2508];
	} else if (codepoint >= 0x2550 && codepoint <= 0x2593) {
		return terminusb_block116[codepoint - 0x2550];
	} else if (codepoint >= 0x2596 && codepoint <= 0x25a0) {
		return terminusb_block117[codepoint - 0x2596];
	} else if (codepoint == 0x25ac) {
		return terminusb_block118[0];
	} else if (codepoint == 0x25ae) {
		return terminusb_block119[0];
	} else if (codepoint == 0x25b2) {
		return terminusb_block120[0];
	} else if (codepoint == 0x25b6) {
		return terminusb_block121[0];
	} else if (codepoint == 0x25ba) {
		return terminusb_block122[0];
	} else if (codepoint == 0x25bc) {
		return terminusb_block123[0];
	} else if (codepoint == 0x25c0) {
		return terminusb_block124[0];
	} else if (codepoint == 0x25c4) {
		return terminusb_block125[0];
	} else if (codepoint == 0x25c6) {
		return terminusb_block126[0];
	} else if (codepoint >= 0x25ca && codepoint <= 0x25cb) {
		return terminusb_block127[codepoint - 0x25ca];
	} else if (codepoint == 0x25cf) {
		return terminusb_block128[0];
	} else if (codepoint >= 0x25d8 && codepoint <= 0x25d9) {
		return terminusb_block129[codepoint - 0x25d8];
	} else if (codepoint >= 0x263a && codepoint <= 0x263c) {
		return terminusb_block130[codepoint - 0x263a];
	} else if (codepoint == 0x2640) {
		return terminusb_block131[0];
	} else if (codepoint == 0x2642) {
		return terminusb_block132[0];
	} else if (codepoint == 0x2660) {
		return terminusb_block133[0];
	} else if (codepoint == 0x2663) {
		return terminusb_block134[0];
	} else if (codepoint >= 0x2665 && codepoint <= 0x2666) {
		return terminusb_block135[codepoint - 0x2665];
	} else if (codepoint >= 0x266a && codepoint <= 0x266b) {
		return terminusb_block136[codepoint - 0x266a];
	} else if (codepoint >= 0x2713 && codepoint <= 0x2714) {
		return terminusb_block137[codepoint - 0x2713];
	} else if (codepoint >= 0x2717 && codepoint <= 0x2718) {
		return terminusb_block138[codepoint - 0x2717];
	} else if (codepoint >= 0x27e8 && codepoint <= 0x27eb) {
		return terminusb_block139[codepoint - 0x27e8];
	} else if (codepoint >= 0x2800 && codepoint <= 0x28ff) {
		return terminusb_block140[codepoint - 0x2800];
	} else if (codepoint == 0x2e2c) {
		return terminusb_block141[0];
	} else if (codepoint >= 0xe0a0 && codepoint <= 0xe0a2) {
		return terminusb_block142[codepoint - 0xe0a0];
	} else if (codepoint >= 0xe0b0 && codepoint <= 0xe0b3) {
		return terminusb_block143[codepoint - 0xe0b0];
	} else if (codepoint == 0xf6be) {
		return terminusb_block144[0];
	} else if (codepoint == 0xfffd) {
		return terminusb_block145[0];
	} else {
		WARN("Codepoint U+%04X is not covered by this font", codepoint);
		return terminusb_block1[0];
	}
}
