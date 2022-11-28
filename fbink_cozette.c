/*
	FBInk: FrameBuffer eInker, a library to print text & images to an eInk Linux framebuffer
	Copyright (C) 2018-2022 NiLuJe <ninuje@gmail.com>
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
	} else if (codepoint >= 0xa0u && codepoint <= 0x1c3u) {
		return cozette_block2[codepoint - 0xa0u];
	} else if (codepoint >= 0x1cdu && codepoint <= 0x1f0u) {
		return cozette_block3[codepoint - 0x1cdu];
	} else if (codepoint >= 0x1f4u && codepoint <= 0x220u) {
		return cozette_block4[codepoint - 0x1f4u];
	} else if (codepoint >= 0x224u && codepoint <= 0x229u) {
		return cozette_block5[codepoint - 0x224u];
	} else if (codepoint == 0x22bu) {
		return cozette_block6[0];
	} else if (codepoint >= 0x22du && codepoint <= 0x22fu) {
		return cozette_block7[codepoint - 0x22du];
	} else if (codepoint >= 0x231u && codepoint <= 0x233u) {
		return cozette_block8[codepoint - 0x231u];
	} else if (codepoint >= 0x241u && codepoint <= 0x242u) {
		return cozette_block9[codepoint - 0x241u];
	} else if (codepoint >= 0x246u && codepoint <= 0x247u) {
		return cozette_block10[codepoint - 0x246u];
	} else if (codepoint >= 0x250u && codepoint <= 0x2a2u) {
		return cozette_block11[codepoint - 0x250u];
	} else if (codepoint >= 0x2b9u && codepoint <= 0x2bdu) {
		return cozette_block12[codepoint - 0x2b9u];
	} else if (codepoint >= 0x2c2u && codepoint <= 0x2ddu) {
		return cozette_block13[codepoint - 0x2c2u];
	} else if (codepoint >= 0x2dfu && codepoint <= 0x2e4u) {
		return cozette_block14[codepoint - 0x2dfu];
	} else if (codepoint >= 0x2ecu && codepoint <= 0x2edu) {
		return cozette_block15[codepoint - 0x2ecu];
	} else if (codepoint >= 0x2efu && codepoint <= 0x2f7u) {
		return cozette_block16[codepoint - 0x2efu];
	} else if (codepoint >= 0x2f9u && codepoint <= 0x2feu) {
		return cozette_block17[codepoint - 0x2f9u];
	} else if (codepoint >= 0x300u && codepoint <= 0x320u) {
		return cozette_block18[codepoint - 0x300u];
	} else if (codepoint >= 0x323u && codepoint <= 0x333u) {
		return cozette_block19[codepoint - 0x323u];
	} else if (codepoint >= 0x33au && codepoint <= 0x343u) {
		return cozette_block20[codepoint - 0x33au];
	} else if (codepoint >= 0x346u && codepoint <= 0x348u) {
		return cozette_block21[codepoint - 0x346u];
	} else if (codepoint == 0x386u) {
		return cozette_block22[0];
	} else if (codepoint >= 0x388u && codepoint <= 0x38au) {
		return cozette_block23[codepoint - 0x388u];
	} else if (codepoint == 0x38cu) {
		return cozette_block24[0];
	} else if (codepoint >= 0x38eu && codepoint <= 0x3a1u) {
		return cozette_block25[codepoint - 0x38eu];
	} else if (codepoint >= 0x3a3u && codepoint <= 0x3ceu) {
		return cozette_block26[codepoint - 0x3a3u];
	} else if (codepoint == 0x3d5u) {
		return cozette_block27[0];
	} else if (codepoint >= 0x3dau && codepoint <= 0x3ddu) {
		return cozette_block28[codepoint - 0x3dau];
	} else if (codepoint == 0x3f4u) {
		return cozette_block29[0];
	} else if (codepoint >= 0x3f7u && codepoint <= 0x3f9u) {
		return cozette_block30[codepoint - 0x3f7u];
	} else if (codepoint >= 0x400u && codepoint <= 0x482u) {
		return cozette_block31[codepoint - 0x400u];
	} else if (codepoint >= 0x48au && codepoint <= 0x52fu) {
		return cozette_block32[codepoint - 0x48au];
	} else if (codepoint == 0xca0u) {
		return cozette_block33[0];
	} else if (codepoint == 0x16a0u) {
		return cozette_block34[0];
	} else if (codepoint >= 0x16a2u && codepoint <= 0x16a6u) {
		return cozette_block35[codepoint - 0x16a2u];
	} else if (codepoint >= 0x16a8u && codepoint <= 0x16acu) {
		return cozette_block36[codepoint - 0x16a8u];
	} else if (codepoint >= 0x1e00u && codepoint <= 0x1ea7u) {
		return cozette_block37[codepoint - 0x1e00u];
	} else if (codepoint >= 0x1eaau && codepoint <= 0x1ec1u) {
		return cozette_block38[codepoint - 0x1eaau];
	} else if (codepoint >= 0x1ec4u && codepoint <= 0x1ed3u) {
		return cozette_block39[codepoint - 0x1ec4u];
	} else if (codepoint >= 0x1ed6u && codepoint <= 0x1ef9u) {
		return cozette_block40[codepoint - 0x1ed6u];
	} else if (codepoint >= 0x1f00u && codepoint <= 0x1f05u) {
		return cozette_block41[codepoint - 0x1f00u];
	} else if (codepoint >= 0x1f08u && codepoint <= 0x1f0du) {
		return cozette_block42[codepoint - 0x1f08u];
	} else if (codepoint >= 0x1f10u && codepoint <= 0x1f15u) {
		return cozette_block43[codepoint - 0x1f10u];
	} else if (codepoint >= 0x1f18u && codepoint <= 0x1f1du) {
		return cozette_block44[codepoint - 0x1f18u];
	} else if (codepoint >= 0x1f20u && codepoint <= 0x1f25u) {
		return cozette_block45[codepoint - 0x1f20u];
	} else if (codepoint >= 0x1f28u && codepoint <= 0x1f2du) {
		return cozette_block46[codepoint - 0x1f28u];
	} else if (codepoint >= 0x1f30u && codepoint <= 0x1f35u) {
		return cozette_block47[codepoint - 0x1f30u];
	} else if (codepoint >= 0x1f38u && codepoint <= 0x1f3du) {
		return cozette_block48[codepoint - 0x1f38u];
	} else if (codepoint >= 0x1f40u && codepoint <= 0x1f45u) {
		return cozette_block49[codepoint - 0x1f40u];
	} else if (codepoint >= 0x1f48u && codepoint <= 0x1f4du) {
		return cozette_block50[codepoint - 0x1f48u];
	} else if (codepoint >= 0x1f50u && codepoint <= 0x1f55u) {
		return cozette_block51[codepoint - 0x1f50u];
	} else if (codepoint == 0x1f59u) {
		return cozette_block52[0];
	} else if (codepoint == 0x1f5bu) {
		return cozette_block53[0];
	} else if (codepoint == 0x1f5du) {
		return cozette_block54[0];
	} else if (codepoint >= 0x1f60u && codepoint <= 0x1f65u) {
		return cozette_block55[codepoint - 0x1f60u];
	} else if (codepoint >= 0x1f68u && codepoint <= 0x1f6du) {
		return cozette_block56[codepoint - 0x1f68u];
	} else if (codepoint >= 0x1f70u && codepoint <= 0x1f7du) {
		return cozette_block57[codepoint - 0x1f70u];
	} else if (codepoint >= 0x1f80u && codepoint <= 0x1f85u) {
		return cozette_block58[codepoint - 0x1f80u];
	} else if (codepoint >= 0x1f88u && codepoint <= 0x1f8du) {
		return cozette_block59[codepoint - 0x1f88u];
	} else if (codepoint >= 0x1f90u && codepoint <= 0x1f95u) {
		return cozette_block60[codepoint - 0x1f90u];
	} else if (codepoint >= 0x1f98u && codepoint <= 0x1f9du) {
		return cozette_block61[codepoint - 0x1f98u];
	} else if (codepoint >= 0x1fa0u && codepoint <= 0x1fa5u) {
		return cozette_block62[codepoint - 0x1fa0u];
	} else if (codepoint >= 0x1fa8u && codepoint <= 0x1fadu) {
		return cozette_block63[codepoint - 0x1fa8u];
	} else if (codepoint >= 0x1fb0u && codepoint <= 0x1fb4u) {
		return cozette_block64[codepoint - 0x1fb0u];
	} else if (codepoint >= 0x1fb6u && codepoint <= 0x1fbcu) {
		return cozette_block65[codepoint - 0x1fb6u];
	} else if (codepoint >= 0x1fc2u && codepoint <= 0x1fc4u) {
		return cozette_block66[codepoint - 0x1fc2u];
	} else if (codepoint >= 0x1fc6u && codepoint <= 0x1fccu) {
		return cozette_block67[codepoint - 0x1fc6u];
	} else if (codepoint >= 0x1fd0u && codepoint <= 0x1fd3u) {
		return cozette_block68[codepoint - 0x1fd0u];
	} else if (codepoint >= 0x1fd6u && codepoint <= 0x1fdbu) {
		return cozette_block69[codepoint - 0x1fd6u];
	} else if (codepoint >= 0x1fe0u && codepoint <= 0x1fe6u) {
		return cozette_block70[codepoint - 0x1fe0u];
	} else if (codepoint >= 0x1fe8u && codepoint <= 0x1fecu) {
		return cozette_block71[codepoint - 0x1fe8u];
	} else if (codepoint >= 0x1ff2u && codepoint <= 0x1ff4u) {
		return cozette_block72[codepoint - 0x1ff2u];
	} else if (codepoint >= 0x1ff6u && codepoint <= 0x1ffcu) {
		return cozette_block73[codepoint - 0x1ff6u];
	} else if (codepoint >= 0x2000u && codepoint <= 0x200au) {
		return cozette_block74[codepoint - 0x2000u];
	} else if (codepoint >= 0x2010u && codepoint <= 0x2027u) {
		return cozette_block75[codepoint - 0x2010u];
	} else if (codepoint >= 0x202fu && codepoint <= 0x2030u) {
		return cozette_block76[codepoint - 0x202fu];
	} else if (codepoint >= 0x2032u && codepoint <= 0x203eu) {
		return cozette_block77[codepoint - 0x2032u];
	} else if (codepoint >= 0x2043u && codepoint <= 0x2046u) {
		return cozette_block78[codepoint - 0x2043u];
	} else if (codepoint == 0x2056u) {
		return cozette_block79[0];
	} else if (codepoint >= 0x2058u && codepoint <= 0x205eu) {
		return cozette_block80[codepoint - 0x2058u];
	} else if (codepoint >= 0x2070u && codepoint <= 0x2071u) {
		return cozette_block81[codepoint - 0x2070u];
	} else if (codepoint >= 0x2074u && codepoint <= 0x208eu) {
		return cozette_block82[codepoint - 0x2074u];
	} else if (codepoint >= 0x2090u && codepoint <= 0x209cu) {
		return cozette_block83[codepoint - 0x2090u];
	} else if (codepoint == 0x20a4u) {
		return cozette_block84[0];
	} else if (codepoint == 0x20aau) {
		return cozette_block85[0];
	} else if (codepoint == 0x20acu) {
		return cozette_block86[0];
	} else if (codepoint == 0x20bdu) {
		return cozette_block87[0];
	} else if (codepoint == 0x20bfu) {
		return cozette_block88[0];
	} else if (codepoint == 0x2116u) {
		return cozette_block89[0];
	} else if (codepoint == 0x2122u) {
		return cozette_block90[0];
	} else if (codepoint >= 0x2160u && codepoint <= 0x2165u) {
		return cozette_block91[codepoint - 0x2160u];
	} else if (codepoint >= 0x2168u && codepoint <= 0x216au) {
		return cozette_block92[codepoint - 0x2168u];
	} else if (codepoint >= 0x2170u && codepoint <= 0x217bu) {
		return cozette_block93[codepoint - 0x2170u];
	} else if (codepoint >= 0x2190u && codepoint <= 0x219bu) {
		return cozette_block94[codepoint - 0x2190u];
	} else if (codepoint >= 0x21a2u && codepoint <= 0x21a7u) {
		return cozette_block95[codepoint - 0x21a2u];
	} else if (codepoint >= 0x21a9u && codepoint <= 0x21acu) {
		return cozette_block96[codepoint - 0x21a9u];
	} else if (codepoint >= 0x21b0u && codepoint <= 0x21b8u) {
		return cozette_block97[codepoint - 0x21b0u];
	} else if (codepoint >= 0x21bau && codepoint <= 0x21c3u) {
		return cozette_block98[codepoint - 0x21bau];
	} else if (codepoint >= 0x21cbu && codepoint <= 0x21ccu) {
		return cozette_block99[codepoint - 0x21cbu];
	} else if (codepoint >= 0x21d0u && codepoint <= 0x21d5u) {
		return cozette_block100[codepoint - 0x21d0u];
	} else if (codepoint >= 0x21e0u && codepoint <= 0x21e3u) {
		return cozette_block101[codepoint - 0x21e0u];
	} else if (codepoint >= 0x21f1u && codepoint <= 0x21f2u) {
		return cozette_block102[codepoint - 0x21f1u];
	} else if (codepoint == 0x2200u) {
		return cozette_block103[0];
	} else if (codepoint >= 0x2203u && codepoint <= 0x2213u) {
		return cozette_block104[codepoint - 0x2203u];
	} else if (codepoint >= 0x2217u && codepoint <= 0x221au) {
		return cozette_block105[codepoint - 0x2217u];
	} else if (codepoint == 0x221eu) {
		return cozette_block106[0];
	} else if (codepoint >= 0x2225u && codepoint <= 0x222cu) {
		return cozette_block107[codepoint - 0x2225u];
	} else if (codepoint >= 0x2248u && codepoint <= 0x2249u) {
		return cozette_block108[codepoint - 0x2248u];
	} else if (codepoint >= 0x2260u && codepoint <= 0x2261u) {
		return cozette_block109[codepoint - 0x2260u];
	} else if (codepoint >= 0x2264u && codepoint <= 0x2265u) {
		return cozette_block110[codepoint - 0x2264u];
	} else if (codepoint >= 0x2282u && codepoint <= 0x228bu) {
		return cozette_block111[codepoint - 0x2282u];
	} else if (codepoint >= 0x229du && codepoint <= 0x229fu) {
		return cozette_block112[codepoint - 0x229du];
	} else if (codepoint == 0x22a1u) {
		return cozette_block113[0];
	} else if (codepoint >= 0x22b2u && codepoint <= 0x22b5u) {
		return cozette_block114[codepoint - 0x22b2u];
	} else if (codepoint >= 0x22c0u && codepoint <= 0x22c6u) {
		return cozette_block115[codepoint - 0x22c0u];
	} else if (codepoint >= 0x22eeu && codepoint <= 0x22f1u) {
		return cozette_block116[codepoint - 0x22eeu];
	} else if (codepoint == 0x2300u) {
		return cozette_block117[0];
	} else if (codepoint == 0x2302u) {
		return cozette_block118[0];
	} else if (codepoint >= 0x2308u && codepoint <= 0x230fu) {
		return cozette_block119[codepoint - 0x2308u];
	} else if (codepoint == 0x2318u) {
		return cozette_block120[0];
	} else if (codepoint >= 0x231cu && codepoint <= 0x2321u) {
		return cozette_block121[codepoint - 0x231cu];
	} else if (codepoint >= 0x2329u && codepoint <= 0x232au) {
		return cozette_block122[codepoint - 0x2329u];
	} else if (codepoint == 0x235fu) {
		return cozette_block123[0];
	} else if (codepoint == 0x237fu) {
		return cozette_block124[0];
	} else if (codepoint >= 0x2387u && codepoint <= 0x2388u) {
		return cozette_block125[codepoint - 0x2387u];
	} else if (codepoint >= 0x23ceu && codepoint <= 0x23cfu) {
		return cozette_block126[codepoint - 0x23ceu];
	} else if (codepoint == 0x23e8u) {
		return cozette_block127[0];
	} else if (codepoint >= 0x23f3u && codepoint <= 0x23fcu) {
		return cozette_block128[codepoint - 0x23f3u];
	} else if (codepoint == 0x2400u) {
		return cozette_block129[0];
	} else if (codepoint == 0x2408u) {
		return cozette_block130[0];
	} else if (codepoint >= 0x240au && codepoint <= 0x240eu) {
		return cozette_block131[codepoint - 0x240au];
	} else if (codepoint == 0x2424u) {
		return cozette_block132[0];
	} else if (codepoint >= 0x2500u && codepoint <= 0x2593u) {
		return cozette_block133[codepoint - 0x2500u];
	} else if (codepoint >= 0x2596u && codepoint <= 0x25a3u) {
		return cozette_block134[codepoint - 0x2596u];
	} else if (codepoint >= 0x25aau && codepoint <= 0x25abu) {
		return cozette_block135[codepoint - 0x25aau];
	} else if (codepoint == 0x25b2u) {
		return cozette_block136[0];
	} else if (codepoint == 0x25b6u) {
		return cozette_block137[0];
	} else if (codepoint == 0x25bcu) {
		return cozette_block138[0];
	} else if (codepoint == 0x25c0u) {
		return cozette_block139[0];
	} else if (codepoint >= 0x25c6u && codepoint <= 0x25c9u) {
		return cozette_block140[codepoint - 0x25c6u];
	} else if (codepoint == 0x25cbu) {
		return cozette_block141[0];
	} else if (codepoint >= 0x25ceu && codepoint <= 0x25d5u) {
		return cozette_block142[codepoint - 0x25ceu];
	} else if (codepoint >= 0x25f0u && codepoint <= 0x25f7u) {
		return cozette_block143[codepoint - 0x25f0u];
	} else if (codepoint == 0x2601u) {
		return cozette_block144[0];
	} else if (codepoint == 0x2603u) {
		return cozette_block145[0];
	} else if (codepoint >= 0x2610u && codepoint <= 0x2612u) {
		return cozette_block146[codepoint - 0x2610u];
	} else if (codepoint == 0x2615u) {
		return cozette_block147[0];
	} else if (codepoint >= 0x2630u && codepoint <= 0x2638u) {
		return cozette_block148[codepoint - 0x2630u];
	} else if (codepoint >= 0x263au && codepoint <= 0x263bu) {
		return cozette_block149[codepoint - 0x263au];
	} else if (codepoint == 0x263fu) {
		return cozette_block150[0];
	} else if (codepoint >= 0x2660u && codepoint <= 0x2667u) {
		return cozette_block151[codepoint - 0x2660u];
	} else if (codepoint >= 0x2669u && codepoint <= 0x266fu) {
		return cozette_block152[codepoint - 0x2669u];
	} else if (codepoint >= 0x2690u && codepoint <= 0x2691u) {
		return cozette_block153[codepoint - 0x2690u];
	} else if (codepoint == 0x2699u) {
		return cozette_block154[0];
	} else if (codepoint >= 0x26a0u && codepoint <= 0x26a1u) {
		return cozette_block155[codepoint - 0x26a0u];
	} else if (codepoint == 0x26b8u) {
		return cozette_block156[0];
	} else if (codepoint >= 0x2713u && codepoint <= 0x271cu) {
		return cozette_block157[codepoint - 0x2713u];
	} else if (codepoint == 0x2726u) {
		return cozette_block158[0];
	} else if (codepoint >= 0x272du && codepoint <= 0x272eu) {
		return cozette_block159[codepoint - 0x272du];
	} else if (codepoint == 0x2739u) {
		return cozette_block160[0];
	} else if (codepoint == 0x2744u) {
		return cozette_block161[0];
	} else if (codepoint == 0x274eu) {
		return cozette_block162[0];
	} else if (codepoint == 0x2753u) {
		return cozette_block163[0];
	} else if (codepoint >= 0x276cu && codepoint <= 0x276fu) {
		return cozette_block164[codepoint - 0x276cu];
	} else if (codepoint == 0x279cu) {
		return cozette_block165[0];
	} else if (codepoint >= 0x27e6u && codepoint <= 0x27ebu) {
		return cozette_block166[codepoint - 0x27e6u];
	} else if (codepoint >= 0x2801u && codepoint <= 0x28ffu) {
		return cozette_block167[codepoint - 0x2801u];
	} else if (codepoint == 0x2b22u) {
		return cozette_block168[0];
	} else if (codepoint == 0x2b50u) {
		return cozette_block169[0];
	} else if (codepoint >= 0x2b60u && codepoint <= 0x2b69u) {
		return cozette_block170[codepoint - 0x2b60u];
	} else if (codepoint >= 0x2b80u && codepoint <= 0x2b83u) {
		return cozette_block171[codepoint - 0x2b80u];
	} else if (codepoint == 0x2e3du) {
		return cozette_block172[0];
	} else if (codepoint == 0x33d1u) {
		return cozette_block173[0];
	} else if (codepoint == 0xa7a8u) {
		return cozette_block174[0];
	} else if (codepoint >= 0xe000u && codepoint <= 0xe00au) {
		return cozette_block175[codepoint - 0xe000u];
	} else if (codepoint >= 0xe0a0u && codepoint <= 0xe0a3u) {
		return cozette_block176[codepoint - 0xe0a0u];
	} else if (codepoint >= 0xe0b0u && codepoint <= 0xe0bfu) {
		return cozette_block177[codepoint - 0xe0b0u];
	} else if (codepoint == 0xe0d2u) {
		return cozette_block178[0];
	} else if (codepoint == 0xe0d4u) {
		return cozette_block179[0];
	} else if (codepoint == 0xe204u) {
		return cozette_block180[0];
	} else if (codepoint >= 0xe20au && codepoint <= 0xe20cu) {
		return cozette_block181[codepoint - 0xe20au];
	} else if (codepoint == 0xe21eu) {
		return cozette_block182[0];
	} else if (codepoint == 0xe244u) {
		return cozette_block183[0];
	} else if (codepoint == 0xe256u) {
		return cozette_block184[0];
	} else if (codepoint == 0xe271u) {
		return cozette_block185[0];
	} else if (codepoint >= 0xe28au && codepoint <= 0xe28bu) {
		return cozette_block186[codepoint - 0xe28au];
	} else if (codepoint >= 0xe5fbu && codepoint <= 0xe5fcu) {
		return cozette_block187[codepoint - 0xe5fbu];
	} else if (codepoint >= 0xe5feu && codepoint <= 0xe628u) {
		return cozette_block188[codepoint - 0xe5feu];
	} else if (codepoint >= 0xe62au && codepoint <= 0xe62du) {
		return cozette_block189[codepoint - 0xe62au];
	} else if (codepoint == 0xe64eu) {
		return cozette_block190[0];
	} else if (codepoint == 0xe681u) {
		return cozette_block191[0];
	} else if (codepoint >= 0xe702u && codepoint <= 0xe703u) {
		return cozette_block192[codepoint - 0xe702u];
	} else if (codepoint >= 0xe706u && codepoint <= 0xe707u) {
		return cozette_block193[codepoint - 0xe706u];
	} else if (codepoint == 0xe70cu) {
		return cozette_block194[0];
	} else if (codepoint >= 0xe70eu && codepoint <= 0xe70fu) {
		return cozette_block195[codepoint - 0xe70eu];
	} else if (codepoint >= 0xe711u && codepoint <= 0xe712u) {
		return cozette_block196[codepoint - 0xe711u];
	} else if (codepoint == 0xe716u) {
		return cozette_block197[0];
	} else if (codepoint == 0xe718u) {
		return cozette_block198[0];
	} else if (codepoint == 0xe71eu) {
		return cozette_block199[0];
	} else if (codepoint >= 0xe725u && codepoint <= 0xe729u) {
		return cozette_block200[codepoint - 0xe725u];
	} else if (codepoint == 0xe72du) {
		return cozette_block201[0];
	} else if (codepoint >= 0xe737u && codepoint <= 0xe73fu) {
		return cozette_block202[codepoint - 0xe737u];
	} else if (codepoint == 0xe743u) {
		return cozette_block203[0];
	} else if (codepoint >= 0xe745u && codepoint <= 0xe746u) {
		return cozette_block204[codepoint - 0xe745u];
	} else if (codepoint >= 0xe749u && codepoint <= 0xe74au) {
		return cozette_block205[codepoint - 0xe749u];
	} else if (codepoint == 0xe74eu) {
		return cozette_block206[0];
	} else if (codepoint >= 0xe755u && codepoint <= 0xe759u) {
		return cozette_block207[codepoint - 0xe755u];
	} else if (codepoint >= 0xe768u && codepoint <= 0xe76au) {
		return cozette_block208[codepoint - 0xe768u];
	} else if (codepoint == 0xe76eu) {
		return cozette_block209[0];
	} else if (codepoint == 0xe777u) {
		return cozette_block210[0];
	} else if (codepoint == 0xe77bu) {
		return cozette_block211[0];
	} else if (codepoint == 0xe77fu) {
		return cozette_block212[0];
	} else if (codepoint == 0xe781u) {
		return cozette_block213[0];
	} else if (codepoint == 0xe791u) {
		return cozette_block214[0];
	} else if (codepoint >= 0xe795u && codepoint <= 0xe796u) {
		return cozette_block215[codepoint - 0xe795u];
	} else if (codepoint == 0xe798u) {
		return cozette_block216[0];
	} else if (codepoint >= 0xe7a2u && codepoint <= 0xe7a3u) {
		return cozette_block217[codepoint - 0xe7a2u];
	} else if (codepoint >= 0xe7a7u && codepoint <= 0xe7a8u) {
		return cozette_block218[codepoint - 0xe7a7u];
	} else if (codepoint == 0xe7aau) {
		return cozette_block219[0];
	} else if (codepoint == 0xe7afu) {
		return cozette_block220[0];
	} else if (codepoint == 0xe7b1u) {
		return cozette_block221[0];
	} else if (codepoint >= 0xe7b4u && codepoint <= 0xe7b5u) {
		return cozette_block222[codepoint - 0xe7b4u];
	} else if (codepoint == 0xe7b8u) {
		return cozette_block223[0];
	} else if (codepoint == 0xe7bau) {
		return cozette_block224[0];
	} else if (codepoint >= 0xe7c4u && codepoint <= 0xe7c5u) {
		return cozette_block225[codepoint - 0xe7c4u];
	} else if (codepoint >= 0xeffau && codepoint <= 0xeffdu) {
		return cozette_block226[codepoint - 0xeffau];
	} else if (codepoint == 0xf001u) {
		return cozette_block227[0];
	} else if (codepoint == 0xf008u) {
		return cozette_block228[0];
	} else if (codepoint >= 0xf00bu && codepoint <= 0xf00du) {
		return cozette_block229[codepoint - 0xf00bu];
	} else if (codepoint >= 0xf013u && codepoint <= 0xf017u) {
		return cozette_block230[codepoint - 0xf013u];
	} else if (codepoint >= 0xf01au && codepoint <= 0xf01cu) {
		return cozette_block231[codepoint - 0xf01au];
	} else if (codepoint == 0xf023u) {
		return cozette_block232[0];
	} else if (codepoint >= 0xf025u && codepoint <= 0xf028u) {
		return cozette_block233[codepoint - 0xf025u];
	} else if (codepoint == 0xf02bu) {
		return cozette_block234[0];
	} else if (codepoint == 0xf02du) {
		return cozette_block235[0];
	} else if (codepoint >= 0xf031u && codepoint <= 0xf035u) {
		return cozette_block236[codepoint - 0xf031u];
	} else if (codepoint == 0xf03du) {
		return cozette_block237[0];
	} else if (codepoint == 0xf040u) {
		return cozette_block238[0];
	} else if (codepoint >= 0xf048u && codepoint <= 0xf04eu) {
		return cozette_block239[codepoint - 0xf048u];
	} else if (codepoint >= 0xf050u && codepoint <= 0xf05au) {
		return cozette_block240[codepoint - 0xf050u];
	} else if (codepoint >= 0xf067u && codepoint <= 0xf06au) {
		return cozette_block241[codepoint - 0xf067u];
	} else if (codepoint == 0xf071u) {
		return cozette_block242[0];
	} else if (codepoint == 0xf073u) {
		return cozette_block243[0];
	} else if (codepoint == 0xf075u) {
		return cozette_block244[0];
	} else if (codepoint >= 0xf07bu && codepoint <= 0xf07cu) {
		return cozette_block245[codepoint - 0xf07bu];
	} else if (codepoint == 0xf080u) {
		return cozette_block246[0];
	} else if (codepoint == 0xf084u) {
		return cozette_block247[0];
	} else if (codepoint == 0xf09cu) {
		return cozette_block248[0];
	} else if (codepoint == 0xf09eu) {
		return cozette_block249[0];
	} else if (codepoint == 0xf0a0u) {
		return cozette_block250[0];
	} else if (codepoint == 0xf0a2u) {
		return cozette_block251[0];
	} else if (codepoint == 0xf0acu) {
		return cozette_block252[0];
	} else if (codepoint == 0xf0aeu) {
		return cozette_block253[0];
	} else if (codepoint == 0xf0b0u) {
		return cozette_block254[0];
	} else if (codepoint >= 0xf0c3u && codepoint <= 0xf0c5u) {
		return cozette_block255[codepoint - 0xf0c3u];
	} else if (codepoint == 0xf0e4u) {
		return cozette_block256[0];
	} else if (codepoint == 0xf0e7u) {
		return cozette_block257[0];
	} else if (codepoint == 0xf0f3u) {
		return cozette_block258[0];
	} else if (codepoint == 0xf0fdu) {
		return cozette_block259[0];
	} else if (codepoint == 0xf108u) {
		return cozette_block260[0];
	} else if (codepoint == 0xf111u) {
		return cozette_block261[0];
	} else if (codepoint >= 0xf113u && codepoint <= 0xf115u) {
		return cozette_block262[codepoint - 0xf113u];
	} else if (codepoint >= 0xf120u && codepoint <= 0xf121u) {
		return cozette_block263[codepoint - 0xf120u];
	} else if (codepoint == 0xf126u) {
		return cozette_block264[0];
	} else if (codepoint >= 0xf130u && codepoint <= 0xf131u) {
		return cozette_block265[codepoint - 0xf130u];
	} else if (codepoint == 0xf133u) {
		return cozette_block266[0];
	} else if (codepoint == 0xf13eu) {
		return cozette_block267[0];
	} else if (codepoint == 0xf155u) {
		return cozette_block268[0];
	} else if (codepoint >= 0xf15bu && codepoint <= 0xf15eu) {
		return cozette_block269[codepoint - 0xf15bu];
	} else if (codepoint == 0xf16bu) {
		return cozette_block270[0];
	} else if (codepoint >= 0xf179u && codepoint <= 0xf17cu) {
		return cozette_block271[codepoint - 0xf179u];
	} else if (codepoint == 0xf185u) {
		return cozette_block272[0];
	} else if (codepoint >= 0xf187u && codepoint <= 0xf188u) {
		return cozette_block273[codepoint - 0xf187u];
	} else if (codepoint == 0xf198u) {
		return cozette_block274[0];
	} else if (codepoint >= 0xf1b6u && codepoint <= 0xf1b7u) {
		return cozette_block275[codepoint - 0xf1b6u];
	} else if (codepoint == 0xf1bbu) {
		return cozette_block276[0];
	} else if (codepoint == 0xf1bdu) {
		return cozette_block277[0];
	} else if (codepoint >= 0xf1c0u && codepoint <= 0xf1c6u) {
		return cozette_block278[codepoint - 0xf1c0u];
	} else if (codepoint == 0xf1d3u) {
		return cozette_block279[0];
	} else if (codepoint >= 0xf1eau && codepoint <= 0xf1ebu) {
		return cozette_block280[codepoint - 0xf1eau];
	} else if (codepoint >= 0xf1f6u && codepoint <= 0xf1f8u) {
		return cozette_block281[codepoint - 0xf1f6u];
	} else if (codepoint == 0xf1feu) {
		return cozette_block282[0];
	} else if (codepoint >= 0xf200u && codepoint <= 0xf201u) {
		return cozette_block283[codepoint - 0xf200u];
	} else if (codepoint == 0xf219u) {
		return cozette_block284[0];
	} else if (codepoint >= 0xf240u && codepoint <= 0xf244u) {
		return cozette_block285[codepoint - 0xf240u];
	} else if (codepoint >= 0xf250u && codepoint <= 0xf254u) {
		return cozette_block286[codepoint - 0xf250u];
	} else if (codepoint == 0xf260u) {
		return cozette_block287[0];
	} else if (codepoint >= 0xf268u && codepoint <= 0xf26au) {
		return cozette_block288[codepoint - 0xf268u];
	} else if (codepoint == 0xf270u) {
		return cozette_block289[0];
	} else if (codepoint >= 0xf293u && codepoint <= 0xf294u) {
		return cozette_block290[codepoint - 0xf293u];
	} else if (codepoint == 0xf296u) {
		return cozette_block291[0];
	} else if (codepoint == 0xf298u) {
		return cozette_block292[0];
	} else if (codepoint >= 0xf2c7u && codepoint <= 0xf2cbu) {
		return cozette_block293[codepoint - 0xf2c7u];
	} else if (codepoint == 0xf2dbu) {
		return cozette_block294[0];
	} else if (codepoint >= 0xf300u && codepoint <= 0xf301u) {
		return cozette_block295[codepoint - 0xf300u];
	} else if (codepoint >= 0xf303u && codepoint <= 0xf30au) {
		return cozette_block296[codepoint - 0xf303u];
	} else if (codepoint >= 0xf30cu && codepoint <= 0xf30eu) {
		return cozette_block297[codepoint - 0xf30cu];
	} else if (codepoint == 0xf310u) {
		return cozette_block298[0];
	} else if (codepoint >= 0xf312u && codepoint <= 0xf314u) {
		return cozette_block299[codepoint - 0xf312u];
	} else if (codepoint >= 0xf317u && codepoint <= 0xf319u) {
		return cozette_block300[codepoint - 0xf317u];
	} else if (codepoint >= 0xf31bu && codepoint <= 0xf31cu) {
		return cozette_block301[codepoint - 0xf31bu];
	} else if (codepoint == 0xf401u) {
		return cozette_block302[0];
	} else if (codepoint == 0xf408u) {
		return cozette_block303[0];
	} else if (codepoint == 0xf410u) {
		return cozette_block304[0];
	} else if (codepoint == 0xf425u) {
		return cozette_block305[0];
	} else if (codepoint == 0xf42bu) {
		return cozette_block306[0];
	} else if (codepoint == 0xf440u) {
		return cozette_block307[0];
	} else if (codepoint == 0xf447u) {
		return cozette_block308[0];
	} else if (codepoint == 0xf449u) {
		return cozette_block309[0];
	} else if (codepoint >= 0xf461u && codepoint <= 0xf462u) {
		return cozette_block310[codepoint - 0xf461u];
	} else if (codepoint == 0xf464u) {
		return cozette_block311[0];
	} else if (codepoint == 0xf475u) {
		return cozette_block312[0];
	} else if (codepoint >= 0xf481u && codepoint <= 0xf482u) {
		return cozette_block313[codepoint - 0xf481u];
	} else if (codepoint >= 0xf489u && codepoint <= 0xf48au) {
		return cozette_block314[codepoint - 0xf489u];
	} else if (codepoint == 0xf48eu) {
		return cozette_block315[0];
	} else if (codepoint == 0xf498u) {
		return cozette_block316[0];
	} else if (codepoint == 0xf49bu) {
		return cozette_block317[0];
	} else if (codepoint == 0xf49eu) {
		return cozette_block318[0];
	} else if (codepoint == 0xf4a0u) {
		return cozette_block319[0];
	} else if (codepoint == 0xf529u) {
		return cozette_block320[0];
	} else if (codepoint >= 0xf578u && codepoint <= 0xf590u) {
		return cozette_block321[codepoint - 0xf578u];
	} else if (codepoint >= 0xf5aeu && codepoint <= 0xf5afu) {
		return cozette_block322[codepoint - 0xf5aeu];
	} else if (codepoint >= 0xf5b1u && codepoint <= 0xf5b2u) {
		return cozette_block323[codepoint - 0xf5b1u];
	} else if (codepoint == 0xf5ebu) {
		return cozette_block324[0];
	} else if (codepoint >= 0xf631u && codepoint <= 0xf632u) {
		return cozette_block325[codepoint - 0xf631u];
	} else if (codepoint >= 0xf658u && codepoint <= 0xf659u) {
		return cozette_block326[codepoint - 0xf658u];
	} else if (codepoint == 0xf668u) {
		return cozette_block327[0];
	} else if (codepoint == 0xf6a6u) {
		return cozette_block328[0];
	} else if (codepoint == 0xf6ffu) {
		return cozette_block329[0];
	} else if (codepoint == 0xf713u) {
		return cozette_block330[0];
	} else if (codepoint == 0xf718u) {
		return cozette_block331[0];
	} else if (codepoint == 0xf71cu) {
		return cozette_block332[0];
	} else if (codepoint == 0xf74au) {
		return cozette_block333[0];
	} else if (codepoint == 0xf783u) {
		return cozette_block334[0];
	} else if (codepoint == 0xf794u) {
		return cozette_block335[0];
	} else if (codepoint == 0xf7b7u) {
		return cozette_block336[0];
	} else if (codepoint >= 0xf7cau && codepoint <= 0xf7cdu) {
		return cozette_block337[codepoint - 0xf7cau];
	} else if (codepoint == 0xf7cfu) {
		return cozette_block338[0];
	} else if (codepoint == 0xf7fbu) {
		return cozette_block339[0];
	} else if (codepoint == 0xf80au) {
		return cozette_block340[0];
	} else if (codepoint == 0xf816u) {
		return cozette_block341[0];
	} else if (codepoint == 0xf81au) {
		return cozette_block342[0];
	} else if (codepoint >= 0xf81fu && codepoint <= 0xf820u) {
		return cozette_block343[codepoint - 0xf81fu];
	} else if (codepoint >= 0xf834u && codepoint <= 0xf835u) {
		return cozette_block344[codepoint - 0xf834u];
	} else if (codepoint == 0xf89fu) {
		return cozette_block345[0];
	} else if (codepoint == 0xf8d7u) {
		return cozette_block346[0];
	} else if (codepoint == 0xf8feu) {
		return cozette_block347[0];
	} else if (codepoint >= 0xfa7du && codepoint <= 0xfa80u) {
		return cozette_block348[codepoint - 0xfa7du];
	} else if (codepoint >= 0xfaa8u && codepoint <= 0xfaa9u) {
		return cozette_block349[codepoint - 0xfaa8u];
	} else if (codepoint == 0xfab6u) {
		return cozette_block350[0];
	} else if (codepoint == 0xfabfu) {
		return cozette_block351[0];
	} else if (codepoint == 0xfbf1u) {
		return cozette_block352[0];
	} else if (codepoint == 0xfc2eu) {
		return cozette_block353[0];
	} else if (codepoint >= 0xfc5bu && codepoint <= 0xfc5du) {
		return cozette_block354[codepoint - 0xfc5bu];
	} else if (codepoint == 0xfcccu) {
		return cozette_block355[0];
	} else if (codepoint == 0xfce4u) {
		return cozette_block356[0];
	} else if (codepoint == 0xfd03u) {
		return cozette_block357[0];
	} else if (codepoint >= 0xfd05u && codepoint <= 0xfd10u) {
		return cozette_block358[codepoint - 0xfd05u];
	} else if (codepoint == 0xfd32u) {
		return cozette_block359[0];
	} else if (codepoint == 0xfd42u) {
		return cozette_block360[0];
	} else if (codepoint >= 0xfe54u && codepoint <= 0xfe66u) {
		return cozette_block361[codepoint - 0xfe54u];
	} else if (codepoint >= 0xfe68u && codepoint <= 0xfe6bu) {
		return cozette_block362[codepoint - 0xfe68u];
	} else if (codepoint == 0x1f512u) {
		return cozette_block363[0];
	} else if (codepoint == 0x1f333u) {
		return cozette_block364[0];
	} else if (codepoint == 0x1f40fu) {
		return cozette_block365[0];
	} else if (codepoint == 0x1f52eu) {
		return cozette_block366[0];
	} else if (codepoint == 0x1f4e6u) {
		return cozette_block367[0];
	} else if (codepoint == 0x1f418u) {
		return cozette_block368[0];
	} else if (codepoint == 0x1f48eu) {
		return cozette_block369[0];
	} else if (codepoint == 0x1f4a0u) {
		return cozette_block370[0];
	} else if (codepoint == 0x1f6e1u) {
		return cozette_block371[0];
	} else if (codepoint == 0x1f608u) {
		return cozette_block372[0];
	} else if (codepoint == 0x1f50bu) {
		return cozette_block373[0];
	} else if (codepoint == 0x1f448u) {
		return cozette_block374[0];
	} else if (codepoint == 0x1f447u) {
		return cozette_block375[0];
	} else if (codepoint == 0x1f331u) {
		return cozette_block376[0];
	} else if (codepoint == 0x1f31eu) {
		return cozette_block377[0];
	} else if (codepoint == 0x1f379u) {
		return cozette_block378[0];
	} else if (codepoint == 0x1f4a1u) {
		return cozette_block379[0];
	} else {
		WARN("Codepoint U+%04X (%s) is not covered by this font", codepoint, u8_cp_to_utf8(codepoint));
		return cozette_block1[0];
	}
}
