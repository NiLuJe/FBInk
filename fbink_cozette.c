/*
	FBInk: FrameBuffer eInker, a library to print text & images to an eInk Linux framebuffer
	Copyright (C) 2018-2024 NiLuJe <ninuje@gmail.com>
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
	} else if (codepoint >= 0x1f4u && codepoint <= 0x221u) {
		return cozette_block4[codepoint - 0x1f4u];
	} else if (codepoint >= 0x224u && codepoint <= 0x229u) {
		return cozette_block5[codepoint - 0x224u];
	} else if (codepoint == 0x22bu) {
		return cozette_block6[0];
	} else if (codepoint >= 0x22du && codepoint <= 0x22fu) {
		return cozette_block7[codepoint - 0x22du];
	} else if (codepoint >= 0x231u && codepoint <= 0x236u) {
		return cozette_block8[codepoint - 0x231u];
	} else if (codepoint >= 0x238u && codepoint <= 0x239u) {
		return cozette_block9[codepoint - 0x238u];
	} else if (codepoint >= 0x241u && codepoint <= 0x242u) {
		return cozette_block10[codepoint - 0x241u];
	} else if (codepoint >= 0x246u && codepoint <= 0x247u) {
		return cozette_block11[codepoint - 0x246u];
	} else if (codepoint >= 0x250u && codepoint <= 0x2a8u) {
		return cozette_block12[codepoint - 0x250u];
	} else if (codepoint >= 0x2aeu && codepoint <= 0x2b7u) {
		return cozette_block13[codepoint - 0x2aeu];
	} else if (codepoint >= 0x2b9u && codepoint <= 0x2e9u) {
		return cozette_block14[codepoint - 0x2b9u];
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
	} else if (codepoint >= 0x339u && codepoint <= 0x343u) {
		return cozette_block20[codepoint - 0x339u];
	} else if (codepoint >= 0x346u && codepoint <= 0x348u) {
		return cozette_block21[codepoint - 0x346u];
	} else if (codepoint == 0x351u) {
		return cozette_block22[0];
	} else if (codepoint == 0x357u) {
		return cozette_block23[0];
	} else if (codepoint == 0x386u) {
		return cozette_block24[0];
	} else if (codepoint >= 0x388u && codepoint <= 0x38au) {
		return cozette_block25[codepoint - 0x388u];
	} else if (codepoint == 0x38cu) {
		return cozette_block26[0];
	} else if (codepoint >= 0x38eu && codepoint <= 0x3a1u) {
		return cozette_block27[codepoint - 0x38eu];
	} else if (codepoint >= 0x3a3u && codepoint <= 0x3ceu) {
		return cozette_block28[codepoint - 0x3a3u];
	} else if (codepoint == 0x3d5u) {
		return cozette_block29[0];
	} else if (codepoint >= 0x3dau && codepoint <= 0x3ddu) {
		return cozette_block30[codepoint - 0x3dau];
	} else if (codepoint == 0x3f4u) {
		return cozette_block31[0];
	} else if (codepoint >= 0x3f7u && codepoint <= 0x3f9u) {
		return cozette_block32[codepoint - 0x3f7u];
	} else if (codepoint >= 0x400u && codepoint <= 0x482u) {
		return cozette_block33[codepoint - 0x400u];
	} else if (codepoint >= 0x48au && codepoint <= 0x52fu) {
		return cozette_block34[codepoint - 0x48au];
	} else if (codepoint == 0xca0u) {
		return cozette_block35[0];
	} else if (codepoint == 0x16a0u) {
		return cozette_block36[0];
	} else if (codepoint >= 0x16a2u && codepoint <= 0x16a6u) {
		return cozette_block37[codepoint - 0x16a2u];
	} else if (codepoint >= 0x16a8u && codepoint <= 0x16acu) {
		return cozette_block38[codepoint - 0x16a8u];
	} else if (codepoint == 0x1d00u) {
		return cozette_block39[0];
	} else if (codepoint == 0x1d07u) {
		return cozette_block40[0];
	} else if (codepoint == 0x1d25u) {
		return cozette_block41[0];
	} else if (codepoint == 0x1d4au) {
		return cozette_block42[0];
	} else if (codepoint == 0x1d91u) {
		return cozette_block43[0];
	} else if (codepoint >= 0x1e00u && codepoint <= 0x1ef9u) {
		return cozette_block44[codepoint - 0x1e00u];
	} else if (codepoint >= 0x1f00u && codepoint <= 0x1f05u) {
		return cozette_block45[codepoint - 0x1f00u];
	} else if (codepoint >= 0x1f08u && codepoint <= 0x1f0du) {
		return cozette_block46[codepoint - 0x1f08u];
	} else if (codepoint >= 0x1f10u && codepoint <= 0x1f15u) {
		return cozette_block47[codepoint - 0x1f10u];
	} else if (codepoint >= 0x1f18u && codepoint <= 0x1f1du) {
		return cozette_block48[codepoint - 0x1f18u];
	} else if (codepoint >= 0x1f20u && codepoint <= 0x1f25u) {
		return cozette_block49[codepoint - 0x1f20u];
	} else if (codepoint >= 0x1f28u && codepoint <= 0x1f2du) {
		return cozette_block50[codepoint - 0x1f28u];
	} else if (codepoint >= 0x1f30u && codepoint <= 0x1f35u) {
		return cozette_block51[codepoint - 0x1f30u];
	} else if (codepoint >= 0x1f38u && codepoint <= 0x1f3du) {
		return cozette_block52[codepoint - 0x1f38u];
	} else if (codepoint >= 0x1f40u && codepoint <= 0x1f45u) {
		return cozette_block53[codepoint - 0x1f40u];
	} else if (codepoint >= 0x1f48u && codepoint <= 0x1f4du) {
		return cozette_block54[codepoint - 0x1f48u];
	} else if (codepoint >= 0x1f50u && codepoint <= 0x1f55u) {
		return cozette_block55[codepoint - 0x1f50u];
	} else if (codepoint == 0x1f59u) {
		return cozette_block56[0];
	} else if (codepoint == 0x1f5bu) {
		return cozette_block57[0];
	} else if (codepoint == 0x1f5du) {
		return cozette_block58[0];
	} else if (codepoint >= 0x1f60u && codepoint <= 0x1f65u) {
		return cozette_block59[codepoint - 0x1f60u];
	} else if (codepoint >= 0x1f68u && codepoint <= 0x1f6du) {
		return cozette_block60[codepoint - 0x1f68u];
	} else if (codepoint >= 0x1f70u && codepoint <= 0x1f7du) {
		return cozette_block61[codepoint - 0x1f70u];
	} else if (codepoint >= 0x1f80u && codepoint <= 0x1f85u) {
		return cozette_block62[codepoint - 0x1f80u];
	} else if (codepoint >= 0x1f88u && codepoint <= 0x1f8du) {
		return cozette_block63[codepoint - 0x1f88u];
	} else if (codepoint >= 0x1f90u && codepoint <= 0x1f95u) {
		return cozette_block64[codepoint - 0x1f90u];
	} else if (codepoint >= 0x1f98u && codepoint <= 0x1f9du) {
		return cozette_block65[codepoint - 0x1f98u];
	} else if (codepoint >= 0x1fa0u && codepoint <= 0x1fa5u) {
		return cozette_block66[codepoint - 0x1fa0u];
	} else if (codepoint >= 0x1fa8u && codepoint <= 0x1fadu) {
		return cozette_block67[codepoint - 0x1fa8u];
	} else if (codepoint >= 0x1fb0u && codepoint <= 0x1fb4u) {
		return cozette_block68[codepoint - 0x1fb0u];
	} else if (codepoint >= 0x1fb6u && codepoint <= 0x1fbcu) {
		return cozette_block69[codepoint - 0x1fb6u];
	} else if (codepoint >= 0x1fc2u && codepoint <= 0x1fc4u) {
		return cozette_block70[codepoint - 0x1fc2u];
	} else if (codepoint >= 0x1fc6u && codepoint <= 0x1fccu) {
		return cozette_block71[codepoint - 0x1fc6u];
	} else if (codepoint >= 0x1fd0u && codepoint <= 0x1fd3u) {
		return cozette_block72[codepoint - 0x1fd0u];
	} else if (codepoint >= 0x1fd6u && codepoint <= 0x1fdbu) {
		return cozette_block73[codepoint - 0x1fd6u];
	} else if (codepoint >= 0x1fe0u && codepoint <= 0x1fe6u) {
		return cozette_block74[codepoint - 0x1fe0u];
	} else if (codepoint >= 0x1fe8u && codepoint <= 0x1fecu) {
		return cozette_block75[codepoint - 0x1fe8u];
	} else if (codepoint >= 0x1ff2u && codepoint <= 0x1ff4u) {
		return cozette_block76[codepoint - 0x1ff2u];
	} else if (codepoint >= 0x1ff6u && codepoint <= 0x1ffcu) {
		return cozette_block77[codepoint - 0x1ff6u];
	} else if (codepoint >= 0x2000u && codepoint <= 0x200au) {
		return cozette_block78[codepoint - 0x2000u];
	} else if (codepoint >= 0x2010u && codepoint <= 0x2027u) {
		return cozette_block79[codepoint - 0x2010u];
	} else if (codepoint >= 0x202fu && codepoint <= 0x2030u) {
		return cozette_block80[codepoint - 0x202fu];
	} else if (codepoint >= 0x2032u && codepoint <= 0x203fu) {
		return cozette_block81[codepoint - 0x2032u];
	} else if (codepoint >= 0x2043u && codepoint <= 0x2046u) {
		return cozette_block82[codepoint - 0x2043u];
	} else if (codepoint == 0x2056u) {
		return cozette_block83[0];
	} else if (codepoint >= 0x2058u && codepoint <= 0x205eu) {
		return cozette_block84[codepoint - 0x2058u];
	} else if (codepoint >= 0x2070u && codepoint <= 0x2071u) {
		return cozette_block85[codepoint - 0x2070u];
	} else if (codepoint >= 0x2074u && codepoint <= 0x208eu) {
		return cozette_block86[codepoint - 0x2074u];
	} else if (codepoint >= 0x2090u && codepoint <= 0x209cu) {
		return cozette_block87[codepoint - 0x2090u];
	} else if (codepoint == 0x20a4u) {
		return cozette_block88[0];
	} else if (codepoint == 0x20aau) {
		return cozette_block89[0];
	} else if (codepoint == 0x20acu) {
		return cozette_block90[0];
	} else if (codepoint == 0x20bdu) {
		return cozette_block91[0];
	} else if (codepoint == 0x20bfu) {
		return cozette_block92[0];
	} else if (codepoint == 0x2116u) {
		return cozette_block93[0];
	} else if (codepoint == 0x2122u) {
		return cozette_block94[0];
	} else if (codepoint == 0x2130u) {
		return cozette_block95[0];
	} else if (codepoint >= 0x2160u && codepoint <= 0x2165u) {
		return cozette_block96[codepoint - 0x2160u];
	} else if (codepoint >= 0x2168u && codepoint <= 0x216au) {
		return cozette_block97[codepoint - 0x2168u];
	} else if (codepoint >= 0x2170u && codepoint <= 0x217bu) {
		return cozette_block98[codepoint - 0x2170u];
	} else if (codepoint >= 0x2190u && codepoint <= 0x219bu) {
		return cozette_block99[codepoint - 0x2190u];
	} else if (codepoint >= 0x21a2u && codepoint <= 0x21a7u) {
		return cozette_block100[codepoint - 0x21a2u];
	} else if (codepoint >= 0x21a9u && codepoint <= 0x21acu) {
		return cozette_block101[codepoint - 0x21a9u];
	} else if (codepoint >= 0x21afu && codepoint <= 0x21c3u) {
		return cozette_block102[codepoint - 0x21afu];
	} else if (codepoint >= 0x21cbu && codepoint <= 0x21ccu) {
		return cozette_block103[codepoint - 0x21cbu];
	} else if (codepoint >= 0x21d0u && codepoint <= 0x21d5u) {
		return cozette_block104[codepoint - 0x21d0u];
	} else if (codepoint >= 0x21e0u && codepoint <= 0x21e3u) {
		return cozette_block105[codepoint - 0x21e0u];
	} else if (codepoint >= 0x21f1u && codepoint <= 0x21f2u) {
		return cozette_block106[codepoint - 0x21f1u];
	} else if (codepoint >= 0x2200u && codepoint <= 0x222cu) {
		return cozette_block107[codepoint - 0x2200u];
	} else if (codepoint >= 0x2234u && codepoint <= 0x2237u) {
		return cozette_block108[codepoint - 0x2234u];
	} else if (codepoint == 0x223au) {
		return cozette_block109[0];
	} else if (codepoint == 0x223eu) {
		return cozette_block110[0];
	} else if (codepoint == 0x2245u) {
		return cozette_block111[0];
	} else if (codepoint >= 0x2248u && codepoint <= 0x2249u) {
		return cozette_block112[codepoint - 0x2248u];
	} else if (codepoint == 0x224du) {
		return cozette_block113[0];
	} else if (codepoint >= 0x2260u && codepoint <= 0x2262u) {
		return cozette_block114[codepoint - 0x2260u];
	} else if (codepoint >= 0x2264u && codepoint <= 0x2265u) {
		return cozette_block115[codepoint - 0x2264u];
	} else if (codepoint >= 0x2282u && codepoint <= 0x228bu) {
		return cozette_block116[codepoint - 0x2282u];
	} else if (codepoint >= 0x228fu && codepoint <= 0x2298u) {
		return cozette_block117[codepoint - 0x228fu];
	} else if (codepoint >= 0x229bu && codepoint <= 0x22a5u) {
		return cozette_block118[codepoint - 0x229bu];
	} else if (codepoint == 0x22a8u) {
		return cozette_block119[0];
	} else if (codepoint >= 0x22b2u && codepoint <= 0x22b8u) {
		return cozette_block120[codepoint - 0x22b2u];
	} else if (codepoint >= 0x22c0u && codepoint <= 0x22c6u) {
		return cozette_block121[codepoint - 0x22c0u];
	} else if (codepoint == 0x22c8u) {
		return cozette_block122[0];
	} else if (codepoint >= 0x22eeu && codepoint <= 0x22f1u) {
		return cozette_block123[codepoint - 0x22eeu];
	} else if (codepoint == 0x2300u) {
		return cozette_block124[0];
	} else if (codepoint == 0x2302u) {
		return cozette_block125[0];
	} else if (codepoint >= 0x2308u && codepoint <= 0x230fu) {
		return cozette_block126[codepoint - 0x2308u];
	} else if (codepoint == 0x2315u) {
		return cozette_block127[0];
	} else if (codepoint == 0x2318u) {
		return cozette_block128[0];
	} else if (codepoint >= 0x231cu && codepoint <= 0x2321u) {
		return cozette_block129[codepoint - 0x231cu];
	} else if (codepoint >= 0x2329u && codepoint <= 0x232au) {
		return cozette_block130[codepoint - 0x2329u];
	} else if (codepoint >= 0x2335u && codepoint <= 0x233au) {
		return cozette_block131[codepoint - 0x2335u];
	} else if (codepoint >= 0x233du && codepoint <= 0x2342u) {
		return cozette_block132[codepoint - 0x233du];
	} else if (codepoint == 0x2349u) {
		return cozette_block133[0];
	} else if (codepoint == 0x234bu) {
		return cozette_block134[0];
	} else if (codepoint >= 0x234du && codepoint <= 0x234eu) {
		return cozette_block135[codepoint - 0x234du];
	} else if (codepoint == 0x2352u) {
		return cozette_block136[0];
	} else if (codepoint == 0x2355u) {
		return cozette_block137[0];
	} else if (codepoint >= 0x2358u && codepoint <= 0x2365u) {
		return cozette_block138[codepoint - 0x2358u];
	} else if (codepoint == 0x2368u) {
		return cozette_block139[0];
	} else if (codepoint >= 0x236au && codepoint <= 0x236fu) {
		return cozette_block140[codepoint - 0x236au];
	} else if (codepoint >= 0x2371u && codepoint <= 0x237au) {
		return cozette_block141[codepoint - 0x2371u];
	} else if (codepoint == 0x237fu) {
		return cozette_block142[0];
	} else if (codepoint >= 0x2387u && codepoint <= 0x238bu) {
		return cozette_block143[codepoint - 0x2387u];
	} else if (codepoint == 0x2395u) {
		return cozette_block144[0];
	} else if (codepoint >= 0x23ceu && codepoint <= 0x23cfu) {
		return cozette_block145[codepoint - 0x23ceu];
	} else if (codepoint == 0x23e8u) {
		return cozette_block146[0];
	} else if (codepoint >= 0x23f3u && codepoint <= 0x23fcu) {
		return cozette_block147[codepoint - 0x23f3u];
	} else if (codepoint == 0x2400u) {
		return cozette_block148[0];
	} else if (codepoint >= 0x2408u && codepoint <= 0x240fu) {
		return cozette_block149[codepoint - 0x2408u];
	} else if (codepoint >= 0x241cu && codepoint <= 0x2420u) {
		return cozette_block150[codepoint - 0x241cu];
	} else if (codepoint == 0x2424u) {
		return cozette_block151[0];
	} else if (codepoint >= 0x2500u && codepoint <= 0x2594u) {
		return cozette_block152[codepoint - 0x2500u];
	} else if (codepoint >= 0x2596u && codepoint <= 0x25a3u) {
		return cozette_block153[codepoint - 0x2596u];
	} else if (codepoint >= 0x25aau && codepoint <= 0x25abu) {
		return cozette_block154[codepoint - 0x25aau];
	} else if (codepoint >= 0x25b2u && codepoint <= 0x25b3u) {
		return cozette_block155[codepoint - 0x25b2u];
	} else if (codepoint == 0x25b6u) {
		return cozette_block156[0];
	} else if (codepoint >= 0x25bcu && codepoint <= 0x25bdu) {
		return cozette_block157[codepoint - 0x25bcu];
	} else if (codepoint == 0x25c0u) {
		return cozette_block158[0];
	} else if (codepoint >= 0x25c6u && codepoint <= 0x25c9u) {
		return cozette_block159[codepoint - 0x25c6u];
	} else if (codepoint == 0x25cbu) {
		return cozette_block160[0];
	} else if (codepoint >= 0x25ceu && codepoint <= 0x25d5u) {
		return cozette_block161[codepoint - 0x25ceu];
	} else if (codepoint == 0x25ebu) {
		return cozette_block162[0];
	} else if (codepoint >= 0x25f0u && codepoint <= 0x25f7u) {
		return cozette_block163[codepoint - 0x25f0u];
	} else if (codepoint == 0x25ffu) {
		return cozette_block164[0];
	} else if (codepoint == 0x2601u) {
		return cozette_block165[0];
	} else if (codepoint == 0x2603u) {
		return cozette_block166[0];
	} else if (codepoint >= 0x2610u && codepoint <= 0x2612u) {
		return cozette_block167[codepoint - 0x2610u];
	} else if (codepoint == 0x2615u) {
		return cozette_block168[0];
	} else if (codepoint >= 0x2630u && codepoint <= 0x263bu) {
		return cozette_block169[codepoint - 0x2630u];
	} else if (codepoint >= 0x263fu && codepoint <= 0x2642u) {
		return cozette_block170[codepoint - 0x263fu];
	} else if (codepoint >= 0x2660u && codepoint <= 0x2667u) {
		return cozette_block171[codepoint - 0x2660u];
	} else if (codepoint >= 0x2669u && codepoint <= 0x266fu) {
		return cozette_block172[codepoint - 0x2669u];
	} else if (codepoint >= 0x2680u && codepoint <= 0x2685u) {
		return cozette_block173[codepoint - 0x2680u];
	} else if (codepoint == 0x2687u) {
		return cozette_block174[0];
	} else if (codepoint >= 0x2690u && codepoint <= 0x2691u) {
		return cozette_block175[codepoint - 0x2690u];
	} else if (codepoint == 0x2699u) {
		return cozette_block176[0];
	} else if (codepoint >= 0x26a0u && codepoint <= 0x26a3u) {
		return cozette_block177[codepoint - 0x26a0u];
	} else if (codepoint >= 0x26a5u && codepoint <= 0x26a6u) {
		return cozette_block178[codepoint - 0x26a5u];
	} else if (codepoint == 0x26a8u) {
		return cozette_block179[0];
	} else if (codepoint >= 0x26b2u && codepoint <= 0x26b5u) {
		return cozette_block180[codepoint - 0x26b2u];
	} else if (codepoint == 0x26b8u) {
		return cozette_block181[0];
	} else if (codepoint >= 0x2713u && codepoint <= 0x271cu) {
		return cozette_block182[codepoint - 0x2713u];
	} else if (codepoint == 0x2726u) {
		return cozette_block183[0];
	} else if (codepoint >= 0x272du && codepoint <= 0x272eu) {
		return cozette_block184[codepoint - 0x272du];
	} else if (codepoint == 0x2739u) {
		return cozette_block185[0];
	} else if (codepoint == 0x2744u) {
		return cozette_block186[0];
	} else if (codepoint == 0x274cu) {
		return cozette_block187[0];
	} else if (codepoint == 0x2753u) {
		return cozette_block188[0];
	} else if (codepoint >= 0x276cu && codepoint <= 0x276fu) {
		return cozette_block189[codepoint - 0x276cu];
	} else if (codepoint == 0x279cu) {
		return cozette_block190[0];
	} else if (codepoint == 0x27dcu) {
		return cozette_block191[0];
	} else if (codepoint >= 0x27e6u && codepoint <= 0x27ebu) {
		return cozette_block192[codepoint - 0x27e6u];
	} else if (codepoint >= 0x2800u && codepoint <= 0x28ffu) {
		return cozette_block193[codepoint - 0x2800u];
	} else if (codepoint == 0x294au) {
		return cozette_block194[0];
	} else if (codepoint == 0x29bfu) {
		return cozette_block195[0];
	} else if (codepoint == 0x29fbu) {
		return cozette_block196[0];
	} else if (codepoint == 0x2b22u) {
		return cozette_block197[0];
	} else if (codepoint == 0x2b50u) {
		return cozette_block198[0];
	} else if (codepoint >= 0x2b60u && codepoint <= 0x2b69u) {
		return cozette_block199[codepoint - 0x2b60u];
	} else if (codepoint >= 0x2b80u && codepoint <= 0x2b83u) {
		return cozette_block200[codepoint - 0x2b80u];
	} else if (codepoint == 0x2c71u) {
		return cozette_block201[0];
	} else if (codepoint == 0x2e3du) {
		return cozette_block202[0];
	} else if (codepoint >= 0x3000u && codepoint <= 0x3003u) {
		return cozette_block203[codepoint - 0x3000u];
	} else if (codepoint == 0x3005u) {
		return cozette_block204[0];
	} else if (codepoint >= 0x3008u && codepoint <= 0x3011u) {
		return cozette_block205[codepoint - 0x3008u];
	} else if (codepoint >= 0x3014u && codepoint <= 0x301bu) {
		return cozette_block206[codepoint - 0x3014u];
	} else if (codepoint == 0x3041u) {
		return cozette_block207[0];
	} else if (codepoint >= 0x3043u && codepoint <= 0x3049u) {
		return cozette_block208[codepoint - 0x3043u];
	} else if (codepoint == 0x304du) {
		return cozette_block209[0];
	} else if (codepoint >= 0x304fu && codepoint <= 0x3051u) {
		return cozette_block210[codepoint - 0x304fu];
	} else if (codepoint == 0x3053u) {
		return cozette_block211[0];
	} else if (codepoint == 0x3055u) {
		return cozette_block212[0];
	} else if (codepoint >= 0x3057u && codepoint <= 0x3059u) {
		return cozette_block213[codepoint - 0x3057u];
	} else if (codepoint == 0x305bu) {
		return cozette_block214[0];
	} else if (codepoint == 0x305du) {
		return cozette_block215[0];
	} else if (codepoint == 0x3061u) {
		return cozette_block216[0];
	} else if (codepoint == 0x3063u) {
		return cozette_block217[0];
	} else if (codepoint == 0x3066u) {
		return cozette_block218[0];
	} else if (codepoint >= 0x3068u && codepoint <= 0x3069u) {
		return cozette_block219[codepoint - 0x3068u];
	} else if (codepoint == 0x306fu) {
		return cozette_block220[0];
	} else if (codepoint == 0x307bu) {
		return cozette_block221[0];
	} else if (codepoint == 0x307eu) {
		return cozette_block222[0];
	} else if (codepoint >= 0x3082u && codepoint <= 0x3085u) {
		return cozette_block223[codepoint - 0x3082u];
	} else if (codepoint >= 0x3087u && codepoint <= 0x308bu) {
		return cozette_block224[codepoint - 0x3087u];
	} else if (codepoint >= 0x308du && codepoint <= 0x308eu) {
		return cozette_block225[codepoint - 0x308du];
	} else if (codepoint >= 0x3092u && codepoint <= 0x3096u) {
		return cozette_block226[codepoint - 0x3092u];
	} else if (codepoint >= 0x309bu && codepoint <= 0x30a1u) {
		return cozette_block227[codepoint - 0x309bu];
	} else if (codepoint == 0x30a3u) {
		return cozette_block228[0];
	} else if (codepoint >= 0x30a5u && codepoint <= 0x30a7u) {
		return cozette_block229[codepoint - 0x30a5u];
	} else if (codepoint >= 0x30a9u && codepoint <= 0x30abu) {
		return cozette_block230[codepoint - 0x30a9u];
	} else if (codepoint == 0x30adu) {
		return cozette_block231[0];
	} else if (codepoint == 0x30afu) {
		return cozette_block232[0];
	} else if (codepoint == 0x30b1u) {
		return cozette_block233[0];
	} else if (codepoint == 0x30b3u) {
		return cozette_block234[0];
	} else if (codepoint == 0x30b7u) {
		return cozette_block235[0];
	} else if (codepoint == 0x30bdu) {
		return cozette_block236[0];
	} else if (codepoint == 0x30bfu) {
		return cozette_block237[0];
	} else if (codepoint == 0x30c1u) {
		return cozette_block238[0];
	} else if (codepoint >= 0x30c3u && codepoint <= 0x30c4u) {
		return cozette_block239[codepoint - 0x30c3u];
	} else if (codepoint == 0x30c6u) {
		return cozette_block240[0];
	} else if (codepoint >= 0x30c8u && codepoint <= 0x30cau) {
		return cozette_block241[codepoint - 0x30c8u];
	} else if (codepoint == 0x30ccu) {
		return cozette_block242[0];
	} else if (codepoint >= 0x30ceu && codepoint <= 0x30cfu) {
		return cozette_block243[codepoint - 0x30ceu];
	} else if (codepoint >= 0x30d2u && codepoint <= 0x30d3u) {
		return cozette_block244[codepoint - 0x30d2u];
	} else if (codepoint == 0x30d5u) {
		return cozette_block245[0];
	} else if (codepoint >= 0x30deu && codepoint <= 0x30dfu) {
		return cozette_block246[codepoint - 0x30deu];
	} else if (codepoint >= 0x30e1u && codepoint <= 0x30e3u) {
		return cozette_block247[codepoint - 0x30e1u];
	} else if (codepoint == 0x30e5u) {
		return cozette_block248[0];
	} else if (codepoint >= 0x30e7u && codepoint <= 0x30eau) {
		return cozette_block249[codepoint - 0x30e7u];
	} else if (codepoint == 0x30ecu) {
		return cozette_block250[0];
	} else if (codepoint >= 0x30eeu && codepoint <= 0x30f0u) {
		return cozette_block251[codepoint - 0x30eeu];
	} else if (codepoint >= 0x30f2u && codepoint <= 0x30f3u) {
		return cozette_block252[codepoint - 0x30f2u];
	} else if (codepoint >= 0x30f5u && codepoint <= 0x30f6u) {
		return cozette_block253[codepoint - 0x30f5u];
	} else if (codepoint == 0x30fbu) {
		return cozette_block254[0];
	} else if (codepoint >= 0x30fdu && codepoint <= 0x30feu) {
		return cozette_block255[codepoint - 0x30fdu];
	} else if (codepoint == 0x33d1u) {
		return cozette_block256[0];
	} else if (codepoint == 0xa7a8u) {
		return cozette_block257[0];
	} else if (codepoint == 0xab65u) {
		return cozette_block258[0];
	} else if (codepoint >= 0xe000u && codepoint <= 0xe00au) {
		return cozette_block259[codepoint - 0xe000u];
	} else if (codepoint >= 0xe0a0u && codepoint <= 0xe0a3u) {
		return cozette_block260[codepoint - 0xe0a0u];
	} else if (codepoint >= 0xe0b0u && codepoint <= 0xe0bfu) {
		return cozette_block261[codepoint - 0xe0b0u];
	} else if (codepoint == 0xe0d2u) {
		return cozette_block262[0];
	} else if (codepoint == 0xe0d4u) {
		return cozette_block263[0];
	} else if (codepoint == 0xe204u) {
		return cozette_block264[0];
	} else if (codepoint >= 0xe20au && codepoint <= 0xe20cu) {
		return cozette_block265[codepoint - 0xe20au];
	} else if (codepoint == 0xe20fu) {
		return cozette_block266[0];
	} else if (codepoint == 0xe21eu) {
		return cozette_block267[0];
	} else if (codepoint == 0xe22cu) {
		return cozette_block268[0];
	} else if (codepoint == 0xe235u) {
		return cozette_block269[0];
	} else if (codepoint == 0xe244u) {
		return cozette_block270[0];
	} else if (codepoint == 0xe256u) {
		return cozette_block271[0];
	} else if (codepoint == 0xe271u) {
		return cozette_block272[0];
	} else if (codepoint >= 0xe28au && codepoint <= 0xe28bu) {
		return cozette_block273[codepoint - 0xe28au];
	} else if (codepoint >= 0xe5fau && codepoint <= 0xe628u) {
		return cozette_block274[codepoint - 0xe5fau];
	} else if (codepoint >= 0xe62au && codepoint <= 0xe62du) {
		return cozette_block275[codepoint - 0xe62au];
	} else if (codepoint == 0xe634u) {
		return cozette_block276[0];
	} else if (codepoint == 0xe63au) {
		return cozette_block277[0];
	} else if (codepoint == 0xe64eu) {
		return cozette_block278[0];
	} else if (codepoint == 0xe681u) {
		return cozette_block279[0];
	} else if (codepoint == 0xe697u) {
		return cozette_block280[0];
	} else if (codepoint == 0xe6a9u) {
		return cozette_block281[0];
	} else if (codepoint >= 0xe702u && codepoint <= 0xe703u) {
		return cozette_block282[codepoint - 0xe702u];
	} else if (codepoint >= 0xe706u && codepoint <= 0xe707u) {
		return cozette_block283[codepoint - 0xe706u];
	} else if (codepoint == 0xe70cu) {
		return cozette_block284[0];
	} else if (codepoint >= 0xe70eu && codepoint <= 0xe70fu) {
		return cozette_block285[codepoint - 0xe70eu];
	} else if (codepoint >= 0xe711u && codepoint <= 0xe712u) {
		return cozette_block286[codepoint - 0xe711u];
	} else if (codepoint == 0xe716u) {
		return cozette_block287[0];
	} else if (codepoint == 0xe718u) {
		return cozette_block288[0];
	} else if (codepoint == 0xe71eu) {
		return cozette_block289[0];
	} else if (codepoint >= 0xe725u && codepoint <= 0xe729u) {
		return cozette_block290[codepoint - 0xe725u];
	} else if (codepoint == 0xe72du) {
		return cozette_block291[0];
	} else if (codepoint >= 0xe736u && codepoint <= 0xe73fu) {
		return cozette_block292[codepoint - 0xe736u];
	} else if (codepoint == 0xe743u) {
		return cozette_block293[0];
	} else if (codepoint >= 0xe745u && codepoint <= 0xe746u) {
		return cozette_block294[codepoint - 0xe745u];
	} else if (codepoint >= 0xe749u && codepoint <= 0xe74au) {
		return cozette_block295[codepoint - 0xe749u];
	} else if (codepoint == 0xe74eu) {
		return cozette_block296[0];
	} else if (codepoint >= 0xe755u && codepoint <= 0xe759u) {
		return cozette_block297[codepoint - 0xe755u];
	} else if (codepoint == 0xe764u) {
		return cozette_block298[0];
	} else if (codepoint >= 0xe768u && codepoint <= 0xe76au) {
		return cozette_block299[codepoint - 0xe768u];
	} else if (codepoint >= 0xe76du && codepoint <= 0xe76eu) {
		return cozette_block300[codepoint - 0xe76du];
	} else if (codepoint == 0xe777u) {
		return cozette_block301[0];
	} else if (codepoint == 0xe779u) {
		return cozette_block302[0];
	} else if (codepoint == 0xe77bu) {
		return cozette_block303[0];
	} else if (codepoint == 0xe77fu) {
		return cozette_block304[0];
	} else if (codepoint == 0xe781u) {
		return cozette_block305[0];
	} else if (codepoint == 0xe786u) {
		return cozette_block306[0];
	} else if (codepoint == 0xe791u) {
		return cozette_block307[0];
	} else if (codepoint >= 0xe795u && codepoint <= 0xe796u) {
		return cozette_block308[codepoint - 0xe795u];
	} else if (codepoint == 0xe798u) {
		return cozette_block309[0];
	} else if (codepoint == 0xe79bu) {
		return cozette_block310[0];
	} else if (codepoint >= 0xe7a2u && codepoint <= 0xe7a3u) {
		return cozette_block311[codepoint - 0xe7a2u];
	} else if (codepoint >= 0xe7a7u && codepoint <= 0xe7a8u) {
		return cozette_block312[codepoint - 0xe7a7u];
	} else if (codepoint == 0xe7aau) {
		return cozette_block313[0];
	} else if (codepoint >= 0xe7afu && codepoint <= 0xe7b1u) {
		return cozette_block314[codepoint - 0xe7afu];
	} else if (codepoint >= 0xe7b4u && codepoint <= 0xe7b5u) {
		return cozette_block315[codepoint - 0xe7b4u];
	} else if (codepoint == 0xe7b8u) {
		return cozette_block316[0];
	} else if (codepoint == 0xe7bau) {
		return cozette_block317[0];
	} else if (codepoint >= 0xe7c4u && codepoint <= 0xe7c5u) {
		return cozette_block318[codepoint - 0xe7c4u];
	} else if (codepoint >= 0xeffau && codepoint <= 0xeffdu) {
		return cozette_block319[codepoint - 0xeffau];
	} else if (codepoint == 0xf001u) {
		return cozette_block320[0];
	} else if (codepoint == 0xf005u) {
		return cozette_block321[0];
	} else if (codepoint == 0xf008u) {
		return cozette_block322[0];
	} else if (codepoint >= 0xf00bu && codepoint <= 0xf00du) {
		return cozette_block323[codepoint - 0xf00bu];
	} else if (codepoint >= 0xf013u && codepoint <= 0xf017u) {
		return cozette_block324[codepoint - 0xf013u];
	} else if (codepoint >= 0xf01au && codepoint <= 0xf01cu) {
		return cozette_block325[codepoint - 0xf01au];
	} else if (codepoint == 0xf023u) {
		return cozette_block326[0];
	} else if (codepoint >= 0xf025u && codepoint <= 0xf028u) {
		return cozette_block327[codepoint - 0xf025u];
	} else if (codepoint == 0xf02bu) {
		return cozette_block328[0];
	} else if (codepoint == 0xf02du) {
		return cozette_block329[0];
	} else if (codepoint >= 0xf031u && codepoint <= 0xf035u) {
		return cozette_block330[codepoint - 0xf031u];
	} else if (codepoint == 0xf03au) {
		return cozette_block331[0];
	} else if (codepoint >= 0xf03du && codepoint <= 0xf03eu) {
		return cozette_block332[codepoint - 0xf03du];
	} else if (codepoint == 0xf040u) {
		return cozette_block333[0];
	} else if (codepoint >= 0xf048u && codepoint <= 0xf04eu) {
		return cozette_block334[codepoint - 0xf048u];
	} else if (codepoint >= 0xf050u && codepoint <= 0xf05au) {
		return cozette_block335[codepoint - 0xf050u];
	} else if (codepoint == 0xf064u) {
		return cozette_block336[0];
	} else if (codepoint >= 0xf067u && codepoint <= 0xf06au) {
		return cozette_block337[codepoint - 0xf067u];
	} else if (codepoint == 0xf071u) {
		return cozette_block338[0];
	} else if (codepoint == 0xf073u) {
		return cozette_block339[0];
	} else if (codepoint >= 0xf075u && codepoint <= 0xf076u) {
		return cozette_block340[codepoint - 0xf075u];
	} else if (codepoint >= 0xf07bu && codepoint <= 0xf07cu) {
		return cozette_block341[codepoint - 0xf07bu];
	} else if (codepoint == 0xf080u) {
		return cozette_block342[0];
	} else if (codepoint >= 0xf084u && codepoint <= 0xf085u) {
		return cozette_block343[codepoint - 0xf084u];
	} else if (codepoint == 0xf09au) {
		return cozette_block344[0];
	} else if (codepoint == 0xf09cu) {
		return cozette_block345[0];
	} else if (codepoint == 0xf09eu) {
		return cozette_block346[0];
	} else if (codepoint == 0xf0a0u) {
		return cozette_block347[0];
	} else if (codepoint == 0xf0a2u) {
		return cozette_block348[0];
	} else if (codepoint == 0xf0acu) {
		return cozette_block349[0];
	} else if (codepoint == 0xf0aeu) {
		return cozette_block350[0];
	} else if (codepoint == 0xf0b0u) {
		return cozette_block351[0];
	} else if (codepoint >= 0xf0c3u && codepoint <= 0xf0c5u) {
		return cozette_block352[codepoint - 0xf0c3u];
	} else if (codepoint == 0xf0e4u) {
		return cozette_block353[0];
	} else if (codepoint == 0xf0e7u) {
		return cozette_block354[0];
	} else if (codepoint >= 0xf0f3u && codepoint <= 0xf0f4u) {
		return cozette_block355[codepoint - 0xf0f3u];
	} else if (codepoint == 0xf0f6u) {
		return cozette_block356[0];
	} else if (codepoint == 0xf0fdu) {
		return cozette_block357[0];
	} else if (codepoint == 0xf108u) {
		return cozette_block358[0];
	} else if (codepoint == 0xf111u) {
		return cozette_block359[0];
	} else if (codepoint >= 0xf113u && codepoint <= 0xf115u) {
		return cozette_block360[codepoint - 0xf113u];
	} else if (codepoint == 0xf11cu) {
		return cozette_block361[0];
	} else if (codepoint >= 0xf120u && codepoint <= 0xf121u) {
		return cozette_block362[codepoint - 0xf120u];
	} else if (codepoint == 0xf126u) {
		return cozette_block363[0];
	} else if (codepoint >= 0xf130u && codepoint <= 0xf131u) {
		return cozette_block364[codepoint - 0xf130u];
	} else if (codepoint == 0xf133u) {
		return cozette_block365[0];
	} else if (codepoint == 0xf13bu) {
		return cozette_block366[0];
	} else if (codepoint == 0xf13eu) {
		return cozette_block367[0];
	} else if (codepoint == 0xf144u) {
		return cozette_block368[0];
	} else if (codepoint == 0xf155u) {
		return cozette_block369[0];
	} else if (codepoint >= 0xf15bu && codepoint <= 0xf15eu) {
		return cozette_block370[codepoint - 0xf15bu];
	} else if (codepoint == 0xf16bu) {
		return cozette_block371[0];
	} else if (codepoint >= 0xf179u && codepoint <= 0xf17cu) {
		return cozette_block372[codepoint - 0xf179u];
	} else if (codepoint == 0xf185u) {
		return cozette_block373[0];
	} else if (codepoint >= 0xf187u && codepoint <= 0xf188u) {
		return cozette_block374[codepoint - 0xf187u];
	} else if (codepoint == 0xf18du) {
		return cozette_block375[0];
	} else if (codepoint == 0xf198u) {
		return cozette_block376[0];
	} else if (codepoint >= 0xf1b6u && codepoint <= 0xf1b7u) {
		return cozette_block377[codepoint - 0xf1b6u];
	} else if (codepoint == 0xf1bbu) {
		return cozette_block378[0];
	} else if (codepoint == 0xf1bdu) {
		return cozette_block379[0];
	} else if (codepoint >= 0xf1c0u && codepoint <= 0xf1c6u) {
		return cozette_block380[codepoint - 0xf1c0u];
	} else if (codepoint == 0xf1d3u) {
		return cozette_block381[0];
	} else if (codepoint >= 0xf1eau && codepoint <= 0xf1ebu) {
		return cozette_block382[codepoint - 0xf1eau];
	} else if (codepoint >= 0xf1f6u && codepoint <= 0xf1f8u) {
		return cozette_block383[codepoint - 0xf1f6u];
	} else if (codepoint == 0xf1fau) {
		return cozette_block384[0];
	} else if (codepoint == 0xf1feu) {
		return cozette_block385[0];
	} else if (codepoint >= 0xf200u && codepoint <= 0xf201u) {
		return cozette_block386[codepoint - 0xf200u];
	} else if (codepoint == 0xf219u) {
		return cozette_block387[0];
	} else if (codepoint == 0xf233u) {
		return cozette_block388[0];
	} else if (codepoint >= 0xf240u && codepoint <= 0xf244u) {
		return cozette_block389[codepoint - 0xf240u];
	} else if (codepoint >= 0xf250u && codepoint <= 0xf254u) {
		return cozette_block390[codepoint - 0xf250u];
	} else if (codepoint == 0xf260u) {
		return cozette_block391[0];
	} else if (codepoint >= 0xf268u && codepoint <= 0xf26au) {
		return cozette_block392[codepoint - 0xf268u];
	} else if (codepoint == 0xf270u) {
		return cozette_block393[0];
	} else if (codepoint >= 0xf292u && codepoint <= 0xf294u) {
		return cozette_block394[codepoint - 0xf292u];
	} else if (codepoint == 0xf296u) {
		return cozette_block395[0];
	} else if (codepoint == 0xf298u) {
		return cozette_block396[0];
	} else if (codepoint >= 0xf2c7u && codepoint <= 0xf2cbu) {
		return cozette_block397[codepoint - 0xf2c7u];
	} else if (codepoint >= 0xf2dbu && codepoint <= 0xf2dcu) {
		return cozette_block398[codepoint - 0xf2dbu];
	} else if (codepoint >= 0xf300u && codepoint <= 0xf30au) {
		return cozette_block399[codepoint - 0xf300u];
	} else if (codepoint >= 0xf30cu && codepoint <= 0xf30eu) {
		return cozette_block400[codepoint - 0xf30cu];
	} else if (codepoint == 0xf310u) {
		return cozette_block401[0];
	} else if (codepoint >= 0xf312u && codepoint <= 0xf314u) {
		return cozette_block402[codepoint - 0xf312u];
	} else if (codepoint >= 0xf317u && codepoint <= 0xf319u) {
		return cozette_block403[codepoint - 0xf317u];
	} else if (codepoint >= 0xf31bu && codepoint <= 0xf31cu) {
		return cozette_block404[codepoint - 0xf31bu];
	} else if (codepoint == 0xf401u) {
		return cozette_block405[0];
	} else if (codepoint == 0xf408u) {
		return cozette_block406[0];
	} else if (codepoint >= 0xf40eu && codepoint <= 0xf411u) {
		return cozette_block407[codepoint - 0xf40eu];
	} else if (codepoint == 0xf413u) {
		return cozette_block408[0];
	} else if (codepoint == 0xf415u) {
		return cozette_block409[0];
	} else if (codepoint == 0xf417u) {
		return cozette_block410[0];
	} else if (codepoint == 0xf423u) {
		return cozette_block411[0];
	} else if (codepoint == 0xf425u) {
		return cozette_block412[0];
	} else if (codepoint == 0xf42bu) {
		return cozette_block413[0];
	} else if (codepoint >= 0xf431u && codepoint <= 0xf434u) {
		return cozette_block414[codepoint - 0xf431u];
	} else if (codepoint == 0xf440u) {
		return cozette_block415[0];
	} else if (codepoint == 0xf447u) {
		return cozette_block416[0];
	} else if (codepoint >= 0xf449u && codepoint <= 0xf44bu) {
		return cozette_block417[codepoint - 0xf449u];
	} else if (codepoint >= 0xf460u && codepoint <= 0xf462u) {
		return cozette_block418[codepoint - 0xf460u];
	} else if (codepoint == 0xf464u) {
		return cozette_block419[0];
	} else if (codepoint == 0xf471u) {
		return cozette_block420[0];
	} else if (codepoint == 0xf475u) {
		return cozette_block421[0];
	} else if (codepoint == 0xf47cu) {
		return cozette_block422[0];
	} else if (codepoint >= 0xf481u && codepoint <= 0xf482u) {
		return cozette_block423[codepoint - 0xf481u];
	} else if (codepoint >= 0xf489u && codepoint <= 0xf48au) {
		return cozette_block424[codepoint - 0xf489u];
	} else if (codepoint == 0xf48eu) {
		return cozette_block425[0];
	} else if (codepoint >= 0xf498u && codepoint <= 0xf499u) {
		return cozette_block426[codepoint - 0xf498u];
	} else if (codepoint == 0xf49bu) {
		return cozette_block427[0];
	} else if (codepoint == 0xf49eu) {
		return cozette_block428[0];
	} else if (codepoint == 0xf4a0u) {
		return cozette_block429[0];
	} else if (codepoint == 0xf4a5u) {
		return cozette_block430[0];
	} else if (codepoint == 0xf529u) {
		return cozette_block431[0];
	} else if (codepoint == 0xf53bu) {
		return cozette_block432[0];
	} else if (codepoint >= 0xf541u && codepoint <= 0xf544u) {
		return cozette_block433[codepoint - 0xf541u];
	} else if (codepoint >= 0xf54bu && codepoint <= 0xf54cu) {
		return cozette_block434[codepoint - 0xf54bu];
	} else if (codepoint == 0xf553u) {
		return cozette_block435[0];
	} else if (codepoint >= 0xf55au && codepoint <= 0xf55cu) {
		return cozette_block436[codepoint - 0xf55au];
	} else if (codepoint >= 0xf578u && codepoint <= 0xf590u) {
		return cozette_block437[codepoint - 0xf578u];
	} else if (codepoint >= 0xf5aeu && codepoint <= 0xf5afu) {
		return cozette_block438[codepoint - 0xf5aeu];
	} else if (codepoint >= 0xf5b1u && codepoint <= 0xf5b2u) {
		return cozette_block439[codepoint - 0xf5b1u];
	} else if (codepoint >= 0xf5bcu && codepoint <= 0xf5bdu) {
		return cozette_block440[codepoint - 0xf5bcu];
	} else if (codepoint == 0xf5ebu) {
		return cozette_block441[0];
	} else if (codepoint >= 0xf631u && codepoint <= 0xf632u) {
		return cozette_block442[codepoint - 0xf631u];
	} else if (codepoint >= 0xf658u && codepoint <= 0xf659u) {
		return cozette_block443[codepoint - 0xf658u];
	} else if (codepoint >= 0xf668u && codepoint <= 0xf669u) {
		return cozette_block444[codepoint - 0xf668u];
	} else if (codepoint == 0xf673u) {
		return cozette_block445[0];
	} else if (codepoint == 0xf68cu) {
		return cozette_block446[0];
	} else if (codepoint >= 0xf6a6u && codepoint <= 0xf6a7u) {
		return cozette_block447[codepoint - 0xf6a6u];
	} else if (codepoint >= 0xf6b7u && codepoint <= 0xf6b9u) {
		return cozette_block448[codepoint - 0xf6b7u];
	} else if (codepoint == 0xf6ffu) {
		return cozette_block449[0];
	} else if (codepoint == 0xf713u) {
		return cozette_block450[0];
	} else if (codepoint == 0xf718u) {
		return cozette_block451[0];
	} else if (codepoint == 0xf71au) {
		return cozette_block452[0];
	} else if (codepoint == 0xf71cu) {
		return cozette_block453[0];
	} else if (codepoint == 0xf71eu) {
		return cozette_block454[0];
	} else if (codepoint == 0xf722u) {
		return cozette_block455[0];
	} else if (codepoint == 0xf724u) {
		return cozette_block456[0];
	} else if (codepoint >= 0xf72au && codepoint <= 0xf72bu) {
		return cozette_block457[codepoint - 0xf72au];
	} else if (codepoint == 0xf72du) {
		return cozette_block458[0];
	} else if (codepoint == 0xf74au) {
		return cozette_block459[0];
	} else if (codepoint == 0xf783u) {
		return cozette_block460[0];
	} else if (codepoint == 0xf794u) {
		return cozette_block461[0];
	} else if (codepoint == 0xf7b7u) {
		return cozette_block462[0];
	} else if (codepoint >= 0xf7cau && codepoint <= 0xf7cdu) {
		return cozette_block463[codepoint - 0xf7cau];
	} else if (codepoint == 0xf7cfu) {
		return cozette_block464[0];
	} else if (codepoint == 0xf7d9u) {
		return cozette_block465[0];
	} else if (codepoint == 0xf7fbu) {
		return cozette_block466[0];
	} else if (codepoint == 0xf80au) {
		return cozette_block467[0];
	} else if (codepoint == 0xf816u) {
		return cozette_block468[0];
	} else if (codepoint == 0xf81au) {
		return cozette_block469[0];
	} else if (codepoint >= 0xf81fu && codepoint <= 0xf820u) {
		return cozette_block470[codepoint - 0xf81fu];
	} else if (codepoint >= 0xf831u && codepoint <= 0xf837u) {
		return cozette_block471[codepoint - 0xf831u];
	} else if (codepoint == 0xf83cu) {
		return cozette_block472[0];
	} else if (codepoint == 0xf886u) {
		return cozette_block473[0];
	} else if (codepoint == 0xf89fu) {
		return cozette_block474[0];
	} else if (codepoint >= 0xf8d6u && codepoint <= 0xf8d7u) {
		return cozette_block475[codepoint - 0xf8d6u];
	} else if (codepoint == 0xf8feu) {
		return cozette_block476[0];
	} else if (codepoint >= 0xfa7du && codepoint <= 0xfa80u) {
		return cozette_block477[codepoint - 0xfa7du];
	} else if (codepoint >= 0xfaa8u && codepoint <= 0xfaa9u) {
		return cozette_block478[codepoint - 0xfaa8u];
	} else if (codepoint == 0xfab6u) {
		return cozette_block479[0];
	} else if (codepoint == 0xfabfu) {
		return cozette_block480[0];
	} else if (codepoint == 0xfb01u) {
		return cozette_block481[0];
	} else if (codepoint == 0xfb68u) {
		return cozette_block482[0];
	} else if (codepoint == 0xfbf1u) {
		return cozette_block483[0];
	} else if (codepoint == 0xfc2eu) {
		return cozette_block484[0];
	} else if (codepoint >= 0xfc5bu && codepoint <= 0xfc5du) {
		return cozette_block485[codepoint - 0xfc5bu];
	} else if (codepoint == 0xfcccu) {
		return cozette_block486[0];
	} else if (codepoint == 0xfce4u) {
		return cozette_block487[0];
	} else if (codepoint == 0xfd03u) {
		return cozette_block488[0];
	} else if (codepoint >= 0xfd05u && codepoint <= 0xfd10u) {
		return cozette_block489[codepoint - 0xfd05u];
	} else if (codepoint == 0xfd32u) {
		return cozette_block490[0];
	} else if (codepoint == 0xfd42u) {
		return cozette_block491[0];
	} else if (codepoint >= 0xfe54u && codepoint <= 0xfe66u) {
		return cozette_block492[codepoint - 0xfe54u];
	} else if (codepoint >= 0xfe68u && codepoint <= 0xfe6bu) {
		return cozette_block493[codepoint - 0xfe68u];
	} else if (codepoint >= 0xff01u && codepoint <= 0xff0eu) {
		return cozette_block494[codepoint - 0xff01u];
	} else if (codepoint >= 0xff10u && codepoint <= 0xff3bu) {
		return cozette_block495[codepoint - 0xff10u];
	} else if (codepoint >= 0xff3du && codepoint <= 0xff3eu) {
		return cozette_block496[codepoint - 0xff3du];
	} else if (codepoint >= 0xff40u && codepoint <= 0xff5du) {
		return cozette_block497[codepoint - 0xff40u];
	} else if (codepoint >= 0xff5fu && codepoint <= 0xff60u) {
		return cozette_block498[codepoint - 0xff5fu];
	} else if (codepoint == 0x1016fu) {
		return cozette_block499[0];
	} else if (codepoint == 0x102a6u) {
		return cozette_block500[0];
	} else if (codepoint == 0x102c9u) {
		return cozette_block501[0];
	} else if (codepoint == 0x102cfu) {
		return cozette_block502[0];
	} else if (codepoint == 0x10315u) {
		return cozette_block503[0];
	} else if (codepoint == 0x10343u) {
		return cozette_block504[0];
	} else if (codepoint == 0x1d4d0u) {
		return cozette_block505[0];
	} else if (codepoint == 0x1d4d2u) {
		return cozette_block506[0];
	} else if (codepoint == 0x1d4e2u) {
		return cozette_block507[0];
	} else if (codepoint >= 0x1d53du && codepoint <= 0x1d53eu) {
		return cozette_block508[codepoint - 0x1d53du];
	} else if (codepoint == 0x1d54au) {
		return cozette_block509[0];
	} else if (codepoint >= 0x1d54eu && codepoint <= 0x1d54fu) {
		return cozette_block510[codepoint - 0x1d54eu];
	} else if (codepoint >= 0x1d557u && codepoint <= 0x1d558u) {
		return cozette_block511[codepoint - 0x1d557u];
	} else if (codepoint >= 0x1d563u && codepoint <= 0x1d564u) {
		return cozette_block512[codepoint - 0x1d563u];
	} else if (codepoint >= 0x1d568u && codepoint <= 0x1d569u) {
		return cozette_block513[codepoint - 0x1d568u];
	} else if (codepoint == 0x1d64fu) {
		return cozette_block514[0];
	} else if (codepoint == 0x1f31eu) {
		return cozette_block515[0];
	} else if (codepoint == 0x1f331u) {
		return cozette_block516[0];
	} else if (codepoint == 0x1f333u) {
		return cozette_block517[0];
	} else if (codepoint == 0x1f379u) {
		return cozette_block518[0];
	} else if (codepoint == 0x1f40fu) {
		return cozette_block519[0];
	} else if (codepoint == 0x1f418u) {
		return cozette_block520[0];
	} else if (codepoint >= 0x1f447u && codepoint <= 0x1f448u) {
		return cozette_block521[codepoint - 0x1f447u];
	} else if (codepoint == 0x1f48eu) {
		return cozette_block522[0];
	} else if (codepoint >= 0x1f4a0u && codepoint <= 0x1f4a1u) {
		return cozette_block523[codepoint - 0x1f4a0u];
	} else if (codepoint == 0x1f4c4u) {
		return cozette_block524[0];
	} else if (codepoint == 0x1f4e6u) {
		return cozette_block525[0];
	} else if (codepoint == 0x1f50bu) {
		return cozette_block526[0];
	} else if (codepoint == 0x1f512u) {
		return cozette_block527[0];
	} else if (codepoint == 0x1f52eu) {
		return cozette_block528[0];
	} else if (codepoint == 0x1f5f2u) {
		return cozette_block529[0];
	} else if (codepoint == 0x1f608u) {
		return cozette_block530[0];
	} else if (codepoint == 0x1f6e1u) {
		return cozette_block531[0];
	} else if (codepoint == 0xf0002u) {
		return cozette_block532[0];
	} else if (codepoint == 0xf006fu) {
		return cozette_block533[0];
	} else if (codepoint == 0xf0172u) {
		return cozette_block534[0];
	} else if (codepoint == 0xf01a8u) {
		return cozette_block535[0];
	} else if (codepoint == 0xf01f0u) {
		return cozette_block536[0];
	} else if (codepoint == 0xf0232u) {
		return cozette_block537[0];
	} else if (codepoint == 0xf02d1u) {
		return cozette_block538[0];
	} else if (codepoint == 0xf02d4u) {
		return cozette_block539[0];
	} else if (codepoint == 0xf0306u) {
		return cozette_block540[0];
	} else if (codepoint == 0xf031bu) {
		return cozette_block541[0];
	} else if (codepoint == 0xf0320u) {
		return cozette_block542[0];
	} else if (codepoint == 0xf0411u) {
		return cozette_block543[0];
	} else if (codepoint == 0xf048du) {
		return cozette_block544[0];
	} else if (codepoint == 0xf04b2u) {
		return cozette_block545[0];
	} else if (codepoint == 0xf05c6u) {
		return cozette_block546[0];
	} else if (codepoint == 0xf0645u) {
		return cozette_block547[0];
	} else if (codepoint == 0xf06a9u) {
		return cozette_block548[0];
	} else if (codepoint == 0xf072bu) {
		return cozette_block549[0];
	} else if (codepoint == 0xf07d4u) {
		return cozette_block550[0];
	} else if (codepoint == 0xf0844u) {
		return cozette_block551[0];
	} else if (codepoint == 0xf0a0au) {
		return cozette_block552[0];
	} else if (codepoint == 0xf1417u) {
		return cozette_block553[0];
	} else {
		WARN("Codepoint U+%04X (%s) is not covered by this font", codepoint, u8_cp_to_utf8(codepoint));
		return cozette_block1[0];
	}
}

