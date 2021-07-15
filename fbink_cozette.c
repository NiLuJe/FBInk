/*
	FBInk: FrameBuffer eInker, a library to print text & images to an eInk Linux framebuffer
	Copyright (C) 2018-2021 NiLuJe <ninuje@gmail.com>
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

#include "fbink_cozette.h"

static const unsigned char*
    cozette_get_bitmap(uint32_t codepoint)
{
	if (codepoint <= 0x7fu) {
		return cozette_block1[codepoint];
	} else if (codepoint >= 0xa0u && codepoint <= 0x17eu) {
		return cozette_block2[codepoint - 0xa0u];
	} else if (codepoint >= 0x18eu && codepoint <= 0x18fu) {
		return cozette_block3[codepoint - 0x18eu];
	} else if (codepoint >= 0x191u && codepoint <= 0x192u) {
		return cozette_block4[codepoint - 0x191u];
	} else if (codepoint >= 0x19cu && codepoint <= 0x1a5u) {
		return cozette_block5[codepoint - 0x19cu];
	} else if (codepoint >= 0x1b1u && codepoint <= 0x1bau) {
		return cozette_block6[codepoint - 0x1b1u];
	} else if (codepoint == 0x1beu) {
		return cozette_block7[0];
	} else if (codepoint >= 0x1c0u && codepoint <= 0x1c3u) {
		return cozette_block8[codepoint - 0x1c0u];
	} else if (codepoint >= 0x1fcu && codepoint <= 0x1ffu) {
		return cozette_block9[codepoint - 0x1fcu];
	} else if (codepoint >= 0x241u && codepoint <= 0x242u) {
		return cozette_block10[codepoint - 0x241u];
	} else if (codepoint >= 0x246u && codepoint <= 0x247u) {
		return cozette_block11[codepoint - 0x246u];
	} else if (codepoint >= 0x250u && codepoint <= 0x2a2u) {
		return cozette_block12[codepoint - 0x250u];
	} else if (codepoint == 0x2c6u) {
		return cozette_block13[0];
	} else if (codepoint == 0x2dcu) {
		return cozette_block14[0];
	} else if (codepoint == 0x386u) {
		return cozette_block15[0];
	} else if (codepoint >= 0x388u && codepoint <= 0x38au) {
		return cozette_block16[codepoint - 0x388u];
	} else if (codepoint == 0x38cu) {
		return cozette_block17[0];
	} else if (codepoint >= 0x38eu && codepoint <= 0x3a1u) {
		return cozette_block18[codepoint - 0x38eu];
	} else if (codepoint >= 0x3a3u && codepoint <= 0x3ceu) {
		return cozette_block19[codepoint - 0x3a3u];
	} else if (codepoint == 0x3d5u) {
		return cozette_block20[0];
	} else if (codepoint >= 0x3dau && codepoint <= 0x3ddu) {
		return cozette_block21[codepoint - 0x3dau];
	} else if (codepoint == 0x3f4u) {
		return cozette_block22[0];
	} else if (codepoint >= 0x3f7u && codepoint <= 0x3f9u) {
		return cozette_block23[codepoint - 0x3f7u];
	} else if (codepoint >= 0x400u && codepoint <= 0x482u) {
		return cozette_block24[codepoint - 0x400u];
	} else if (codepoint >= 0x48au && codepoint <= 0x52fu) {
		return cozette_block25[codepoint - 0x48au];
	} else if (codepoint == 0x16a0u) {
		return cozette_block26[0];
	} else if (codepoint == 0x1e9eu) {
		return cozette_block27[0];
	} else if (codepoint >= 0x2010u && codepoint <= 0x2027u) {
		return cozette_block28[codepoint - 0x2010u];
	} else if (codepoint == 0x2030u) {
		return cozette_block29[0];
	} else if (codepoint >= 0x2032u && codepoint <= 0x203au) {
		return cozette_block30[codepoint - 0x2032u];
	} else if (codepoint >= 0x2070u && codepoint <= 0x2071u) {
		return cozette_block31[codepoint - 0x2070u];
	} else if (codepoint >= 0x2074u && codepoint <= 0x208eu) {
		return cozette_block32[codepoint - 0x2074u];
	} else if (codepoint == 0x20acu) {
		return cozette_block33[0];
	} else if (codepoint == 0x20bdu) {
		return cozette_block34[0];
	} else if (codepoint == 0x2116u) {
		return cozette_block35[0];
	} else if (codepoint == 0x2122u) {
		return cozette_block36[0];
	} else if (codepoint == 0x2164u) {
		return cozette_block37[0];
	} else if (codepoint >= 0x2190u && codepoint <= 0x219bu) {
		return cozette_block38[codepoint - 0x2190u];
	} else if (codepoint >= 0x21b0u && codepoint <= 0x21b5u) {
		return cozette_block39[codepoint - 0x21b0u];
	} else if (codepoint >= 0x21d0u && codepoint <= 0x21d5u) {
		return cozette_block40[codepoint - 0x21d0u];
	} else if (codepoint >= 0x21e0u && codepoint <= 0x21e3u) {
		return cozette_block41[codepoint - 0x21e0u];
	} else if (codepoint == 0x2200u) {
		return cozette_block42[0];
	} else if (codepoint >= 0x2203u && codepoint <= 0x2213u) {
		return cozette_block43[codepoint - 0x2203u];
	} else if (codepoint >= 0x2217u && codepoint <= 0x221au) {
		return cozette_block44[codepoint - 0x2217u];
	} else if (codepoint == 0x221eu) {
		return cozette_block45[0];
	} else if (codepoint >= 0x2225u && codepoint <= 0x2226u) {
		return cozette_block46[codepoint - 0x2225u];
	} else if (codepoint >= 0x2248u && codepoint <= 0x2249u) {
		return cozette_block47[codepoint - 0x2248u];
	} else if (codepoint >= 0x2260u && codepoint <= 0x2261u) {
		return cozette_block48[codepoint - 0x2260u];
	} else if (codepoint >= 0x2264u && codepoint <= 0x2265u) {
		return cozette_block49[codepoint - 0x2264u];
	} else if (codepoint >= 0x229du && codepoint <= 0x229fu) {
		return cozette_block50[codepoint - 0x229du];
	} else if (codepoint == 0x2300u) {
		return cozette_block51[0];
	} else if (codepoint == 0x2302u) {
		return cozette_block52[0];
	} else if (codepoint == 0x2318u) {
		return cozette_block53[0];
	} else if (codepoint == 0x235fu) {
		return cozette_block54[0];
	} else if (codepoint >= 0x2387u && codepoint <= 0x2388u) {
		return cozette_block55[codepoint - 0x2387u];
	} else if (codepoint >= 0x23ceu && codepoint <= 0x23cfu) {
		return cozette_block56[codepoint - 0x23ceu];
	} else if (codepoint == 0x23e8u) {
		return cozette_block57[0];
	} else if (codepoint >= 0x23f3u && codepoint <= 0x23fcu) {
		return cozette_block58[codepoint - 0x23f3u];
	} else if (codepoint == 0x240au) {
		return cozette_block59[0];
	} else if (codepoint == 0x240du) {
		return cozette_block60[0];
	} else if (codepoint == 0x2424u) {
		return cozette_block61[0];
	} else if (codepoint >= 0x2500u && codepoint <= 0x2593u) {
		return cozette_block62[codepoint - 0x2500u];
	} else if (codepoint >= 0x2596u && codepoint <= 0x25a1u) {
		return cozette_block63[codepoint - 0x2596u];
	} else if (codepoint == 0x25b2u) {
		return cozette_block64[0];
	} else if (codepoint == 0x25b6u) {
		return cozette_block65[0];
	} else if (codepoint == 0x25bcu) {
		return cozette_block66[0];
	} else if (codepoint == 0x25c0u) {
		return cozette_block67[0];
	} else if (codepoint == 0x25cbu) {
		return cozette_block68[0];
	} else if (codepoint == 0x25cfu) {
		return cozette_block69[0];
	} else if (codepoint == 0x2601u) {
		return cozette_block70[0];
	} else if (codepoint == 0x2603u) {
		return cozette_block71[0];
	} else if (codepoint >= 0x2610u && codepoint <= 0x2612u) {
		return cozette_block72[codepoint - 0x2610u];
	} else if (codepoint == 0x2615u) {
		return cozette_block73[0];
	} else if (codepoint >= 0x2630u && codepoint <= 0x2638u) {
		return cozette_block74[codepoint - 0x2630u];
	} else if (codepoint >= 0x263au && codepoint <= 0x263bu) {
		return cozette_block75[codepoint - 0x263au];
	} else if (codepoint == 0x263fu) {
		return cozette_block76[0];
	} else if (codepoint >= 0x2660u && codepoint <= 0x2667u) {
		return cozette_block77[codepoint - 0x2660u];
	} else if (codepoint >= 0x2669u && codepoint <= 0x266fu) {
		return cozette_block78[codepoint - 0x2669u];
	} else if (codepoint >= 0x2690u && codepoint <= 0x2691u) {
		return cozette_block79[codepoint - 0x2690u];
	} else if (codepoint == 0x2699u) {
		return cozette_block80[0];
	} else if (codepoint >= 0x26a0u && codepoint <= 0x26a1u) {
		return cozette_block81[codepoint - 0x26a0u];
	} else if (codepoint == 0x26b8u) {
		return cozette_block82[0];
	} else if (codepoint >= 0x2713u && codepoint <= 0x271cu) {
		return cozette_block83[codepoint - 0x2713u];
	} else if (codepoint == 0x2726u) {
		return cozette_block84[0];
	} else if (codepoint >= 0x272du && codepoint <= 0x272eu) {
		return cozette_block85[codepoint - 0x272du];
	} else if (codepoint == 0x2739u) {
		return cozette_block86[0];
	} else if (codepoint == 0x274eu) {
		return cozette_block87[0];
	} else if (codepoint >= 0x276cu && codepoint <= 0x276fu) {
		return cozette_block88[codepoint - 0x276cu];
	} else if (codepoint == 0x279cu) {
		return cozette_block89[0];
	} else if (codepoint >= 0x27e6u && codepoint <= 0x27ebu) {
		return cozette_block90[codepoint - 0x27e6u];
	} else if (codepoint >= 0x2801u && codepoint <= 0x28ffu) {
		return cozette_block91[codepoint - 0x2801u];
	} else if (codepoint == 0x2b22u) {
		return cozette_block92[0];
	} else if (codepoint == 0x2b50u) {
		return cozette_block93[0];
	} else if (codepoint >= 0x2b60u && codepoint <= 0x2b69u) {
		return cozette_block94[codepoint - 0x2b60u];
	} else if (codepoint >= 0x2b80u && codepoint <= 0x2b83u) {
		return cozette_block95[codepoint - 0x2b80u];
	} else if (codepoint == 0x33d1u) {
		return cozette_block96[0];
	} else if (codepoint == 0xa7a8u) {
		return cozette_block97[0];
	} else if (codepoint >= 0xe000u && codepoint <= 0xe00au) {
		return cozette_block98[codepoint - 0xe000u];
	} else if (codepoint >= 0xe0a0u && codepoint <= 0xe0a3u) {
		return cozette_block99[codepoint - 0xe0a0u];
	} else if (codepoint >= 0xe0b0u && codepoint <= 0xe0bfu) {
		return cozette_block100[codepoint - 0xe0b0u];
	} else if (codepoint == 0xe0d2u) {
		return cozette_block101[0];
	} else if (codepoint == 0xe0d4u) {
		return cozette_block102[0];
	} else if (codepoint == 0xe204u) {
		return cozette_block103[0];
	} else if (codepoint >= 0xe20au && codepoint <= 0xe20cu) {
		return cozette_block104[codepoint - 0xe20au];
	} else if (codepoint == 0xe21eu) {
		return cozette_block105[0];
	} else if (codepoint == 0xe244u) {
		return cozette_block106[0];
	} else if (codepoint == 0xe256u) {
		return cozette_block107[0];
	} else if (codepoint == 0xe271u) {
		return cozette_block108[0];
	} else if (codepoint >= 0xe28au && codepoint <= 0xe28bu) {
		return cozette_block109[codepoint - 0xe28au];
	} else if (codepoint >= 0xe5fbu && codepoint <= 0xe5fcu) {
		return cozette_block110[codepoint - 0xe5fbu];
	} else if (codepoint >= 0xe5feu && codepoint <= 0xe628u) {
		return cozette_block111[codepoint - 0xe5feu];
	} else if (codepoint >= 0xe62au && codepoint <= 0xe62du) {
		return cozette_block112[codepoint - 0xe62au];
	} else if (codepoint == 0xe64eu) {
		return cozette_block113[0];
	} else if (codepoint == 0xe681u) {
		return cozette_block114[0];
	} else if (codepoint == 0xe703u) {
		return cozette_block115[0];
	} else if (codepoint >= 0xe706u && codepoint <= 0xe707u) {
		return cozette_block116[codepoint - 0xe706u];
	} else if (codepoint == 0xe70cu) {
		return cozette_block117[0];
	} else if (codepoint >= 0xe70eu && codepoint <= 0xe70fu) {
		return cozette_block118[codepoint - 0xe70eu];
	} else if (codepoint >= 0xe711u && codepoint <= 0xe712u) {
		return cozette_block119[codepoint - 0xe711u];
	} else if (codepoint == 0xe718u) {
		return cozette_block120[0];
	} else if (codepoint == 0xe71eu) {
		return cozette_block121[0];
	} else if (codepoint >= 0xe725u && codepoint <= 0xe729u) {
		return cozette_block122[codepoint - 0xe725u];
	} else if (codepoint == 0xe72du) {
		return cozette_block123[0];
	} else if (codepoint >= 0xe737u && codepoint <= 0xe73fu) {
		return cozette_block124[codepoint - 0xe737u];
	} else if (codepoint == 0xe743u) {
		return cozette_block125[0];
	} else if (codepoint >= 0xe745u && codepoint <= 0xe746u) {
		return cozette_block126[codepoint - 0xe745u];
	} else if (codepoint >= 0xe749u && codepoint <= 0xe74au) {
		return cozette_block127[codepoint - 0xe749u];
	} else if (codepoint == 0xe74eu) {
		return cozette_block128[0];
	} else if (codepoint >= 0xe755u && codepoint <= 0xe759u) {
		return cozette_block129[codepoint - 0xe755u];
	} else if (codepoint >= 0xe768u && codepoint <= 0xe76au) {
		return cozette_block130[codepoint - 0xe768u];
	} else if (codepoint == 0xe76eu) {
		return cozette_block131[0];
	} else if (codepoint == 0xe777u) {
		return cozette_block132[0];
	} else if (codepoint == 0xe77bu) {
		return cozette_block133[0];
	} else if (codepoint == 0xe77fu) {
		return cozette_block134[0];
	} else if (codepoint == 0xe781u) {
		return cozette_block135[0];
	} else if (codepoint == 0xe791u) {
		return cozette_block136[0];
	} else if (codepoint >= 0xe795u && codepoint <= 0xe796u) {
		return cozette_block137[codepoint - 0xe795u];
	} else if (codepoint == 0xe798u) {
		return cozette_block138[0];
	} else if (codepoint >= 0xe7a2u && codepoint <= 0xe7a3u) {
		return cozette_block139[codepoint - 0xe7a2u];
	} else if (codepoint >= 0xe7a7u && codepoint <= 0xe7a8u) {
		return cozette_block140[codepoint - 0xe7a7u];
	} else if (codepoint == 0xe7aau) {
		return cozette_block141[0];
	} else if (codepoint == 0xe7afu) {
		return cozette_block142[0];
	} else if (codepoint == 0xe7b1u) {
		return cozette_block143[0];
	} else if (codepoint >= 0xe7b4u && codepoint <= 0xe7b5u) {
		return cozette_block144[codepoint - 0xe7b4u];
	} else if (codepoint == 0xe7b8u) {
		return cozette_block145[0];
	} else if (codepoint == 0xe7bau) {
		return cozette_block146[0];
	} else if (codepoint >= 0xe7c4u && codepoint <= 0xe7c5u) {
		return cozette_block147[codepoint - 0xe7c4u];
	} else if (codepoint >= 0xeffau && codepoint <= 0xeffdu) {
		return cozette_block148[codepoint - 0xeffau];
	} else if (codepoint == 0xf001u) {
		return cozette_block149[0];
	} else if (codepoint == 0xf008u) {
		return cozette_block150[0];
	} else if (codepoint >= 0xf00bu && codepoint <= 0xf00du) {
		return cozette_block151[codepoint - 0xf00bu];
	} else if (codepoint == 0xf013u) {
		return cozette_block152[0];
	} else if (codepoint >= 0xf015u && codepoint <= 0xf017u) {
		return cozette_block153[codepoint - 0xf015u];
	} else if (codepoint >= 0xf01au && codepoint <= 0xf01cu) {
		return cozette_block154[codepoint - 0xf01au];
	} else if (codepoint == 0xf023u) {
		return cozette_block155[0];
	} else if (codepoint >= 0xf025u && codepoint <= 0xf028u) {
		return cozette_block156[codepoint - 0xf025u];
	} else if (codepoint == 0xf02bu) {
		return cozette_block157[0];
	} else if (codepoint == 0xf02du) {
		return cozette_block158[0];
	} else if (codepoint >= 0xf031u && codepoint <= 0xf035u) {
		return cozette_block159[codepoint - 0xf031u];
	} else if (codepoint == 0xf03du) {
		return cozette_block160[0];
	} else if (codepoint == 0xf040u) {
		return cozette_block161[0];
	} else if (codepoint >= 0xf048u && codepoint <= 0xf04eu) {
		return cozette_block162[codepoint - 0xf048u];
	} else if (codepoint >= 0xf050u && codepoint <= 0xf05au) {
		return cozette_block163[codepoint - 0xf050u];
	} else if (codepoint >= 0xf067u && codepoint <= 0xf06au) {
		return cozette_block164[codepoint - 0xf067u];
	} else if (codepoint == 0xf073u) {
		return cozette_block165[0];
	} else if (codepoint == 0xf075u) {
		return cozette_block166[0];
	} else if (codepoint >= 0xf07bu && codepoint <= 0xf07cu) {
		return cozette_block167[codepoint - 0xf07bu];
	} else if (codepoint == 0xf080u) {
		return cozette_block168[0];
	} else if (codepoint == 0xf084u) {
		return cozette_block169[0];
	} else if (codepoint == 0xf09cu) {
		return cozette_block170[0];
	} else if (codepoint == 0xf09eu) {
		return cozette_block171[0];
	} else if (codepoint == 0xf0a0u) {
		return cozette_block172[0];
	} else if (codepoint == 0xf0a2u) {
		return cozette_block173[0];
	} else if (codepoint == 0xf0acu) {
		return cozette_block174[0];
	} else if (codepoint == 0xf0aeu) {
		return cozette_block175[0];
	} else if (codepoint == 0xf0b0u) {
		return cozette_block176[0];
	} else if (codepoint == 0xf0c3u) {
		return cozette_block177[0];
	} else if (codepoint == 0xf0c5u) {
		return cozette_block178[0];
	} else if (codepoint == 0xf0e4u) {
		return cozette_block179[0];
	} else if (codepoint == 0xf0f3u) {
		return cozette_block180[0];
	} else if (codepoint == 0xf0fdu) {
		return cozette_block181[0];
	} else if (codepoint == 0xf108u) {
		return cozette_block182[0];
	} else if (codepoint == 0xf111u) {
		return cozette_block183[0];
	} else if (codepoint >= 0xf113u && codepoint <= 0xf115u) {
		return cozette_block184[codepoint - 0xf113u];
	} else if (codepoint >= 0xf120u && codepoint <= 0xf121u) {
		return cozette_block185[codepoint - 0xf120u];
	} else if (codepoint == 0xf126u) {
		return cozette_block186[0];
	} else if (codepoint >= 0xf130u && codepoint <= 0xf131u) {
		return cozette_block187[codepoint - 0xf130u];
	} else if (codepoint == 0xf133u) {
		return cozette_block188[0];
	} else if (codepoint == 0xf13eu) {
		return cozette_block189[0];
	} else if (codepoint == 0xf155u) {
		return cozette_block190[0];
	} else if (codepoint >= 0xf15bu && codepoint <= 0xf15cu) {
		return cozette_block191[codepoint - 0xf15bu];
	} else if (codepoint == 0xf16bu) {
		return cozette_block192[0];
	} else if (codepoint >= 0xf179u && codepoint <= 0xf17cu) {
		return cozette_block193[codepoint - 0xf179u];
	} else if (codepoint == 0xf185u) {
		return cozette_block194[0];
	} else if (codepoint >= 0xf187u && codepoint <= 0xf188u) {
		return cozette_block195[codepoint - 0xf187u];
	} else if (codepoint == 0xf198u) {
		return cozette_block196[0];
	} else if (codepoint >= 0xf1b6u && codepoint <= 0xf1b7u) {
		return cozette_block197[codepoint - 0xf1b6u];
	} else if (codepoint == 0xf1bbu) {
		return cozette_block198[0];
	} else if (codepoint == 0xf1bdu) {
		return cozette_block199[0];
	} else if (codepoint >= 0xf1c0u && codepoint <= 0xf1c6u) {
		return cozette_block200[codepoint - 0xf1c0u];
	} else if (codepoint == 0xf1d3u) {
		return cozette_block201[0];
	} else if (codepoint >= 0xf1eau && codepoint <= 0xf1ebu) {
		return cozette_block202[codepoint - 0xf1eau];
	} else if (codepoint >= 0xf1f6u && codepoint <= 0xf1f7u) {
		return cozette_block203[codepoint - 0xf1f6u];
	} else if (codepoint == 0xf1feu) {
		return cozette_block204[0];
	} else if (codepoint >= 0xf200u && codepoint <= 0xf201u) {
		return cozette_block205[codepoint - 0xf200u];
	} else if (codepoint == 0xf219u) {
		return cozette_block206[0];
	} else if (codepoint >= 0xf240u && codepoint <= 0xf244u) {
		return cozette_block207[codepoint - 0xf240u];
	} else if (codepoint >= 0xf250u && codepoint <= 0xf254u) {
		return cozette_block208[codepoint - 0xf250u];
	} else if (codepoint >= 0xf268u && codepoint <= 0xf26au) {
		return cozette_block209[codepoint - 0xf268u];
	} else if (codepoint == 0xf270u) {
		return cozette_block210[0];
	} else if (codepoint == 0xf296u) {
		return cozette_block211[0];
	} else if (codepoint == 0xf298u) {
		return cozette_block212[0];
	} else if (codepoint >= 0xf2c7u && codepoint <= 0xf2cbu) {
		return cozette_block213[codepoint - 0xf2c7u];
	} else if (codepoint == 0xf2dbu) {
		return cozette_block214[0];
	} else if (codepoint >= 0xf300u && codepoint <= 0xf301u) {
		return cozette_block215[codepoint - 0xf300u];
	} else if (codepoint >= 0xf303u && codepoint <= 0xf30au) {
		return cozette_block216[codepoint - 0xf303u];
	} else if (codepoint >= 0xf30cu && codepoint <= 0xf30eu) {
		return cozette_block217[codepoint - 0xf30cu];
	} else if (codepoint == 0xf310u) {
		return cozette_block218[0];
	} else if (codepoint >= 0xf312u && codepoint <= 0xf314u) {
		return cozette_block219[codepoint - 0xf312u];
	} else if (codepoint >= 0xf317u && codepoint <= 0xf319u) {
		return cozette_block220[codepoint - 0xf317u];
	} else if (codepoint >= 0xf31bu && codepoint <= 0xf31cu) {
		return cozette_block221[codepoint - 0xf31bu];
	} else if (codepoint == 0xf401u) {
		return cozette_block222[0];
	} else if (codepoint == 0xf408u) {
		return cozette_block223[0];
	} else if (codepoint == 0xf410u) {
		return cozette_block224[0];
	} else if (codepoint == 0xf42bu) {
		return cozette_block225[0];
	} else if (codepoint == 0xf440u) {
		return cozette_block226[0];
	} else if (codepoint == 0xf447u) {
		return cozette_block227[0];
	} else if (codepoint >= 0xf461u && codepoint <= 0xf462u) {
		return cozette_block228[codepoint - 0xf461u];
	} else if (codepoint == 0xf464u) {
		return cozette_block229[0];
	} else if (codepoint >= 0xf481u && codepoint <= 0xf482u) {
		return cozette_block230[codepoint - 0xf481u];
	} else if (codepoint >= 0xf489u && codepoint <= 0xf48au) {
		return cozette_block231[codepoint - 0xf489u];
	} else if (codepoint == 0xf498u) {
		return cozette_block232[0];
	} else if (codepoint == 0xf49bu) {
		return cozette_block233[0];
	} else if (codepoint == 0xf4a0u) {
		return cozette_block234[0];
	} else if (codepoint >= 0xf578u && codepoint <= 0xf590u) {
		return cozette_block235[codepoint - 0xf578u];
	} else if (codepoint == 0xf6ffu) {
		return cozette_block236[0];
	} else if (codepoint == 0xf713u) {
		return cozette_block237[0];
	} else if (codepoint == 0xf718u) {
		return cozette_block238[0];
	} else if (codepoint == 0xf7b7u) {
		return cozette_block239[0];
	} else if (codepoint >= 0xf7cau && codepoint <= 0xf7cdu) {
		return cozette_block240[codepoint - 0xf7cau];
	} else if (codepoint == 0xf7cfu) {
		return cozette_block241[0];
	} else if (codepoint == 0xf80au) {
		return cozette_block242[0];
	} else if (codepoint == 0xf81au) {
		return cozette_block243[0];
	} else if (codepoint >= 0xf81fu && codepoint <= 0xf820u) {
		return cozette_block244[codepoint - 0xf81fu];
	} else if (codepoint >= 0xfa7du && codepoint <= 0xfa80u) {
		return cozette_block245[codepoint - 0xfa7du];
	} else if (codepoint >= 0xfaa8u && codepoint <= 0xfaa9u) {
		return cozette_block246[codepoint - 0xfaa8u];
	} else if (codepoint == 0xfabfu) {
		return cozette_block247[0];
	} else if (codepoint == 0xfbf1u) {
		return cozette_block248[0];
	} else if (codepoint >= 0xfc5bu && codepoint <= 0xfc5du) {
		return cozette_block249[codepoint - 0xfc5bu];
	} else if (codepoint == 0xfcccu) {
		return cozette_block250[0];
	} else if (codepoint == 0xfd03u) {
		return cozette_block251[0];
	} else if (codepoint >= 0xfd05u && codepoint <= 0xfd10u) {
		return cozette_block252[codepoint - 0xfd05u];
	} else if (codepoint == 0xfd42u) {
		return cozette_block253[0];
	} else if (codepoint >= 0xfe54u && codepoint <= 0xfe66u) {
		return cozette_block254[codepoint - 0xfe54u];
	} else if (codepoint >= 0xfe68u && codepoint <= 0xfe6bu) {
		return cozette_block255[codepoint - 0xfe68u];
	} else if (codepoint == 0x1f512u) {
		return cozette_block256[0];
	} else if (codepoint == 0x1f333u) {
		return cozette_block257[0];
	} else if (codepoint == 0x1f40fu) {
		return cozette_block258[0];
	} else if (codepoint == 0x1f52eu) {
		return cozette_block259[0];
	} else if (codepoint == 0x1f4e6u) {
		return cozette_block260[0];
	} else if (codepoint == 0x1f418u) {
		return cozette_block261[0];
	} else if (codepoint == 0x1f48eu) {
		return cozette_block262[0];
	} else if (codepoint == 0x1f980u) {
		return cozette_block263[0];
	} else if (codepoint == 0x1f4a0u) {
		return cozette_block264[0];
	} else if (codepoint == 0x1f6e1u) {
		return cozette_block265[0];
	} else if (codepoint == 0x1f608u) {
		return cozette_block266[0];
	} else if (codepoint == 0x1f50bu) {
		return cozette_block267[0];
	} else if (codepoint == 0x1f448u) {
		return cozette_block268[0];
	} else if (codepoint == 0x1f447u) {
		return cozette_block269[0];
	} else if (codepoint == 0x1f331u) {
		return cozette_block270[0];
	} else if (codepoint == 0x1f31eu) {
		return cozette_block271[0];
	} else {
		WARN("Codepoint U+%04X (%s) is not covered by this font", codepoint, u8_cp_to_utf8(codepoint));
		return cozette_block1[0];
	}
}
