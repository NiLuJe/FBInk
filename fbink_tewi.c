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

#include "fbink_tewi.h"

static const unsigned char*
    tewi_get_bitmap(uint32_t codepoint)
{
	if (codepoint >= 0x20u && codepoint <= 0x7eu) {
		return tewi_block1[codepoint - 0x20u];
	} else if (codepoint >= 0xa1u && codepoint <= 0xacu) {
		return tewi_block2[codepoint - 0xa1u];
	} else if (codepoint >= 0xaeu && codepoint <= 0x2bdu) {
		return tewi_block3[codepoint - 0xaeu];
	} else if (codepoint >= 0x2c2u && codepoint <= 0x2cbu) {
		return tewi_block4[codepoint - 0x2c2u];
	} else if (codepoint >= 0x2d0u && codepoint <= 0x2d1u) {
		return tewi_block5[codepoint - 0x2d0u];
	} else if (codepoint >= 0x2d8u && codepoint <= 0x2deu) {
		return tewi_block6[codepoint - 0x2d8u];
	} else if (codepoint == 0x2e0u) {
		return tewi_block7[0];
	} else if (codepoint == 0x2e4u) {
		return tewi_block8[0];
	} else if (codepoint == 0x2eeu) {
		return tewi_block9[0];
	} else if (codepoint >= 0x300u && codepoint <= 0x30eu) {
		return tewi_block10[codepoint - 0x300u];
	} else if (codepoint >= 0x310u && codepoint <= 0x315u) {
		return tewi_block11[codepoint - 0x310u];
	} else if (codepoint == 0x31au) {
		return tewi_block12[0];
	} else if (codepoint == 0x31cu) {
		return tewi_block13[0];
	} else if (codepoint >= 0x31fu && codepoint <= 0x320u) {
		return tewi_block14[codepoint - 0x31fu];
	} else if (codepoint >= 0x324u && codepoint <= 0x325u) {
		return tewi_block15[codepoint - 0x324u];
	} else if (codepoint == 0x32au) {
		return tewi_block16[0];
	} else if (codepoint == 0x32cu) {
		return tewi_block17[0];
	} else if (codepoint == 0x330u) {
		return tewi_block18[0];
	} else if (codepoint >= 0x339u && codepoint <= 0x33cu) {
		return tewi_block19[codepoint - 0x339u];
	} else if (codepoint >= 0x343u && codepoint <= 0x344u) {
		return tewi_block20[codepoint - 0x343u];
	} else if (codepoint == 0x35cu) {
		return tewi_block21[0];
	} else if (codepoint == 0x361u) {
		return tewi_block22[0];
	} else if (codepoint >= 0x370u && codepoint <= 0x377u) {
		return tewi_block23[codepoint - 0x370u];
	} else if (codepoint >= 0x37au && codepoint <= 0x37fu) {
		return tewi_block24[codepoint - 0x37au];
	} else if (codepoint >= 0x384u && codepoint <= 0x38au) {
		return tewi_block25[codepoint - 0x384u];
	} else if (codepoint == 0x38cu) {
		return tewi_block26[0];
	} else if (codepoint >= 0x38eu && codepoint <= 0x3a1u) {
		return tewi_block27[codepoint - 0x38eu];
	} else if (codepoint >= 0x3a3u && codepoint <= 0x486u) {
		return tewi_block28[codepoint - 0x3a3u];
	} else if (codepoint >= 0x48au && codepoint <= 0x4ffu) {
		return tewi_block29[codepoint - 0x48au];
	} else if (codepoint == 0x5beu) {
		return tewi_block30[0];
	} else if (codepoint == 0x5c0u) {
		return tewi_block31[0];
	} else if (codepoint == 0x5c3u) {
		return tewi_block32[0];
	} else if (codepoint == 0x5c6u) {
		return tewi_block33[0];
	} else if (codepoint >= 0x5d0u && codepoint <= 0x5efu) {
		return tewi_block34[codepoint - 0x5d0u];
	} else if (codepoint == 0xca0u) {
		return tewi_block35[0];
	} else if (codepoint == 0xe3fu) {
		return tewi_block36[0];
	} else if (codepoint == 0x16a0u) {
		return tewi_block37[0];
	} else if (codepoint >= 0x16a2u && codepoint <= 0x16a3u) {
		return tewi_block38[codepoint - 0x16a2u];
	} else if (codepoint >= 0x16a5u && codepoint <= 0x16a6u) {
		return tewi_block39[codepoint - 0x16a5u];
	} else if (codepoint >= 0x16a8u && codepoint <= 0x16acu) {
		return tewi_block40[codepoint - 0x16a8u];
	} else if (codepoint >= 0x1e00u && codepoint <= 0x1ea9u) {
		return tewi_block41[codepoint - 0x1e00u];
	} else if (codepoint >= 0x1eabu && codepoint <= 0x1eb3u) {
		return tewi_block42[codepoint - 0x1eabu];
	} else if (codepoint >= 0x1eb5u && codepoint <= 0x1ec3u) {
		return tewi_block43[codepoint - 0x1eb5u];
	} else if (codepoint >= 0x1ec5u && codepoint <= 0x1f15u) {
		return tewi_block44[codepoint - 0x1ec5u];
	} else if (codepoint >= 0x1f18u && codepoint <= 0x1f1du) {
		return tewi_block45[codepoint - 0x1f18u];
	} else if (codepoint >= 0x1f20u && codepoint <= 0x1f45u) {
		return tewi_block46[codepoint - 0x1f20u];
	} else if (codepoint >= 0x1f48u && codepoint <= 0x1f4du) {
		return tewi_block47[codepoint - 0x1f48u];
	} else if (codepoint >= 0x1f50u && codepoint <= 0x1f57u) {
		return tewi_block48[codepoint - 0x1f50u];
	} else if (codepoint == 0x1f59u) {
		return tewi_block49[0];
	} else if (codepoint == 0x1f5bu) {
		return tewi_block50[0];
	} else if (codepoint == 0x1f5du) {
		return tewi_block51[0];
	} else if (codepoint >= 0x1f5fu && codepoint <= 0x1f7du) {
		return tewi_block52[codepoint - 0x1f5fu];
	} else if (codepoint >= 0x1f80u && codepoint <= 0x1fb4u) {
		return tewi_block53[codepoint - 0x1f80u];
	} else if (codepoint >= 0x1fb6u && codepoint <= 0x1fbfu) {
		return tewi_block54[codepoint - 0x1fb6u];
	} else if (codepoint >= 0x1fc2u && codepoint <= 0x1fc4u) {
		return tewi_block55[codepoint - 0x1fc2u];
	} else if (codepoint >= 0x1fc6u && codepoint <= 0x1fd3u) {
		return tewi_block56[codepoint - 0x1fc6u];
	} else if (codepoint >= 0x1fd6u && codepoint <= 0x1fdbu) {
		return tewi_block57[codepoint - 0x1fd6u];
	} else if (codepoint >= 0x1fddu && codepoint <= 0x1fefu) {
		return tewi_block58[codepoint - 0x1fddu];
	} else if (codepoint >= 0x1ff2u && codepoint <= 0x1ff4u) {
		return tewi_block59[codepoint - 0x1ff2u];
	} else if (codepoint >= 0x1ff6u && codepoint <= 0x1ffeu) {
		return tewi_block60[codepoint - 0x1ff6u];
	} else if (codepoint == 0x2010u) {
		return tewi_block61[0];
	} else if (codepoint >= 0x2012u && codepoint <= 0x2027u) {
		return tewi_block62[codepoint - 0x2012u];
	} else if (codepoint == 0x2030u) {
		return tewi_block63[0];
	} else if (codepoint >= 0x2032u && codepoint <= 0x203au) {
		return tewi_block64[codepoint - 0x2032u];
	} else if (codepoint >= 0x203cu && codepoint <= 0x2056u) {
		return tewi_block65[codepoint - 0x203cu];
	} else if (codepoint >= 0x2058u && codepoint <= 0x205eu) {
		return tewi_block66[codepoint - 0x2058u];
	} else if (codepoint >= 0x2070u && codepoint <= 0x2071u) {
		return tewi_block67[codepoint - 0x2070u];
	} else if (codepoint >= 0x2074u && codepoint <= 0x208eu) {
		return tewi_block68[codepoint - 0x2074u];
	} else if (codepoint >= 0x2090u && codepoint <= 0x209cu) {
		return tewi_block69[codepoint - 0x2090u];
	} else if (codepoint >= 0x20a0u && codepoint <= 0x20a6u) {
		return tewi_block70[codepoint - 0x20a0u];
	} else if (codepoint >= 0x20a8u && codepoint <= 0x20afu) {
		return tewi_block71[codepoint - 0x20a8u];
	} else if (codepoint >= 0x20b1u && codepoint <= 0x20b3u) {
		return tewi_block72[codepoint - 0x20b1u];
	} else if (codepoint >= 0x20b5u && codepoint <= 0x20b6u) {
		return tewi_block73[codepoint - 0x20b5u];
	} else if (codepoint >= 0x20b8u && codepoint <= 0x20bau) {
		return tewi_block74[codepoint - 0x20b8u];
	} else if (codepoint >= 0x20bcu && codepoint <= 0x20bdu) {
		return tewi_block75[codepoint - 0x20bcu];
	} else if (codepoint >= 0x2100u && codepoint <= 0x210bu) {
		return tewi_block76[codepoint - 0x2100u];
	} else if (codepoint >= 0x2116u && codepoint <= 0x2117u) {
		return tewi_block77[codepoint - 0x2116u];
	} else if (codepoint == 0x2122u) {
		return tewi_block78[0];
	} else if (codepoint >= 0x2125u && codepoint <= 0x2127u) {
		return tewi_block79[codepoint - 0x2125u];
	} else if (codepoint >= 0x212au && codepoint <= 0x212bu) {
		return tewi_block80[codepoint - 0x212au];
	} else if (codepoint == 0x212fu) {
		return tewi_block81[0];
	} else if (codepoint >= 0x2133u && codepoint <= 0x2134u) {
		return tewi_block82[codepoint - 0x2133u];
	} else if (codepoint >= 0x2160u && codepoint <= 0x2166u) {
		return tewi_block83[codepoint - 0x2160u];
	} else if (codepoint >= 0x2168u && codepoint <= 0x2176u) {
		return tewi_block84[codepoint - 0x2168u];
	} else if (codepoint >= 0x2178u && codepoint <= 0x2180u) {
		return tewi_block85[codepoint - 0x2178u];
	} else if (codepoint == 0x2183u) {
		return tewi_block86[0];
	} else if (codepoint >= 0x2190u && codepoint <= 0x2193u) {
		return tewi_block87[codepoint - 0x2190u];
	} else if (codepoint >= 0x2195u && codepoint <= 0x21acu) {
		return tewi_block88[codepoint - 0x2195u];
	} else if (codepoint >= 0x21afu && codepoint <= 0x21cdu) {
		return tewi_block89[codepoint - 0x21afu];
	} else if (codepoint >= 0x21cfu && codepoint <= 0x21d3u) {
		return tewi_block90[codepoint - 0x21cfu];
	} else if (codepoint >= 0x21d5u && codepoint <= 0x21efu) {
		return tewi_block91[codepoint - 0x21d5u];
	} else if (codepoint >= 0x21f1u && codepoint <= 0x21f8u) {
		return tewi_block92[codepoint - 0x21f1u];
	} else if (codepoint == 0x21fau) {
		return tewi_block93[0];
	} else if (codepoint >= 0x21fdu && codepoint <= 0x21feu) {
		return tewi_block94[codepoint - 0x21fdu];
	} else if (codepoint >= 0x2200u && codepoint <= 0x222cu) {
		return tewi_block95[codepoint - 0x2200u];
	} else if (codepoint >= 0x222eu && codepoint <= 0x222fu) {
		return tewi_block96[codepoint - 0x222eu];
	} else if (codepoint >= 0x2231u && codepoint <= 0x225cu) {
		return tewi_block97[codepoint - 0x2231u];
	} else if (codepoint >= 0x225eu && codepoint <= 0x2299u) {
		return tewi_block98[codepoint - 0x225eu];
	} else if (codepoint == 0x229bu) {
		return tewi_block99[0];
	} else if (codepoint >= 0x229du && codepoint <= 0x2320u) {
		return tewi_block100[codepoint - 0x229du];
	} else if (codepoint >= 0x2322u && codepoint <= 0x2328u) {
		return tewi_block101[codepoint - 0x2322u];
	} else if (codepoint == 0x232bu) {
		return tewi_block102[0];
	} else if (codepoint == 0x232du) {
		return tewi_block103[0];
	} else if (codepoint >= 0x232fu && codepoint <= 0x237eu) {
		return tewi_block104[codepoint - 0x232fu];
	} else if (codepoint == 0x2380u) {
		return tewi_block105[0];
	} else if (codepoint == 0x2388u) {
		return tewi_block106[0];
	} else if (codepoint == 0x238bu) {
		return tewi_block107[0];
	} else if (codepoint >= 0x23bau && codepoint <= 0x23bdu) {
		return tewi_block108[codepoint - 0x23bau];
	} else if (codepoint >= 0x23e9u && codepoint <= 0x23efu) {
		return tewi_block109[codepoint - 0x23e9u];
	} else if (codepoint >= 0x2408u && codepoint <= 0x240fu) {
		return tewi_block110[codepoint - 0x2408u];
	} else if (codepoint >= 0x2423u && codepoint <= 0x2426u) {
		return tewi_block111[codepoint - 0x2423u];
	} else if (codepoint >= 0x2440u && codepoint <= 0x244au) {
		return tewi_block112[codepoint - 0x2440u];
	} else if (codepoint >= 0x2460u && codepoint <= 0x24b5u) {
		return tewi_block113[codepoint - 0x2460u];
	} else if (codepoint >= 0x2500u && codepoint <= 0x2609u) {
		return tewi_block114[codepoint - 0x2500u];
	} else if (codepoint >= 0x2610u && codepoint <= 0x2611u) {
		return tewi_block115[codepoint - 0x2610u];
	} else if (codepoint == 0x2614u) {
		return tewi_block116[0];
	} else if (codepoint >= 0x2616u && codepoint <= 0x2617u) {
		return tewi_block117[codepoint - 0x2616u];
	} else if (codepoint == 0x261eu) {
		return tewi_block118[0];
	} else if (codepoint >= 0x2630u && codepoint <= 0x2637u) {
		return tewi_block119[codepoint - 0x2630u];
	} else if (codepoint >= 0x263au && codepoint <= 0x263cu) {
		return tewi_block120[codepoint - 0x263au];
	} else if (codepoint >= 0x263fu && codepoint <= 0x2642u) {
		return tewi_block121[codepoint - 0x263fu];
	} else if (codepoint >= 0x2660u && codepoint <= 0x266fu) {
		return tewi_block122[codepoint - 0x2660u];
	} else if (codepoint >= 0x26a2u && codepoint <= 0x26a9u) {
		return tewi_block123[codepoint - 0x26a2u];
	} else if (codepoint == 0x2708u) {
		return tewi_block124[0];
	} else if (codepoint >= 0x2713u && codepoint <= 0x2718u) {
		return tewi_block125[codepoint - 0x2713u];
	} else if (codepoint >= 0x271au && codepoint <= 0x2721u) {
		return tewi_block126[codepoint - 0x271au];
	} else if (codepoint >= 0x2724u && codepoint <= 0x2727u) {
		return tewi_block127[codepoint - 0x2724u];
	} else if (codepoint >= 0x2729u && codepoint <= 0x272bu) {
		return tewi_block128[codepoint - 0x2729u];
	} else if (codepoint == 0x2733u) {
		return tewi_block129[0];
	} else if (codepoint == 0x2736u) {
		return tewi_block130[0];
	} else if (codepoint == 0x273fu) {
		return tewi_block131[0];
	} else if (codepoint == 0x2741u) {
		return tewi_block132[0];
	} else if (codepoint >= 0x274fu && codepoint <= 0x2752u) {
		return tewi_block133[codepoint - 0x274fu];
	} else if (codepoint >= 0x275bu && codepoint <= 0x2760u) {
		return tewi_block134[codepoint - 0x275bu];
	} else if (codepoint >= 0x2764u && codepoint <= 0x2766u) {
		return tewi_block135[codepoint - 0x2764u];
	} else if (codepoint >= 0x276cu && codepoint <= 0x2771u) {
		return tewi_block136[codepoint - 0x276cu];
	} else if (codepoint >= 0x278au && codepoint <= 0x279bu) {
		return tewi_block137[codepoint - 0x278au];
	} else if (codepoint >= 0x27e8u && codepoint <= 0x27e9u) {
		return tewi_block138[codepoint - 0x27e8u];
	} else if (codepoint >= 0x27f2u && codepoint <= 0x27f3u) {
		return tewi_block139[codepoint - 0x27f2u];
	} else if (codepoint >= 0x27f5u && codepoint <= 0x27f6u) {
		return tewi_block140[codepoint - 0x27f5u];
	} else if (codepoint >= 0x27f8u && codepoint <= 0x27f9u) {
		return tewi_block141[codepoint - 0x27f8u];
	} else if (codepoint >= 0x27fbu && codepoint <= 0x28ffu) {
		return tewi_block142[codepoint - 0x27fbu];
	} else if (codepoint >= 0x2902u && codepoint <= 0x2903u) {
		return tewi_block143[codepoint - 0x2902u];
	} else if (codepoint >= 0x2906u && codepoint <= 0x2909u) {
		return tewi_block144[codepoint - 0x2906u];
	} else if (codepoint >= 0x290cu && codepoint <= 0x290fu) {
		return tewi_block145[codepoint - 0x290cu];
	} else if (codepoint >= 0x2912u && codepoint <= 0x2913u) {
		return tewi_block146[codepoint - 0x2912u];
	} else if (codepoint >= 0x2919u && codepoint <= 0x291eu) {
		return tewi_block147[codepoint - 0x2919u];
	} else if (codepoint >= 0x2921u && codepoint <= 0x2926u) {
		return tewi_block148[codepoint - 0x2921u];
	} else if (codepoint >= 0x2933u && codepoint <= 0x2946u) {
		return tewi_block149[codepoint - 0x2933u];
	} else if (codepoint >= 0x2949u && codepoint <= 0x294du) {
		return tewi_block150[codepoint - 0x2949u];
	} else if (codepoint == 0x294fu) {
		return tewi_block151[0];
	} else if (codepoint >= 0x2951u && codepoint <= 0x2976u) {
		return tewi_block152[codepoint - 0x2951u];
	} else if (codepoint >= 0x2978u && codepoint <= 0x297fu) {
		return tewi_block153[codepoint - 0x2978u];
	} else if (codepoint >= 0x2b00u && codepoint <= 0x2b03u) {
		return tewi_block154[codepoint - 0x2b00u];
	} else if (codepoint >= 0x2b05u && codepoint <= 0x2b0bu) {
		return tewi_block155[codepoint - 0x2b05u];
	} else if (codepoint >= 0x2b0du && codepoint <= 0x2b32u) {
		return tewi_block156[codepoint - 0x2b0du];
	} else if (codepoint == 0x2b38u) {
		return tewi_block157[0];
	} else if (codepoint >= 0x2b3fu && codepoint <= 0x2b55u) {
		return tewi_block158[codepoint - 0x2b3fu];
	} else if (codepoint == 0x2b58u) {
		return tewi_block159[0];
	} else if (codepoint >= 0x2b5au && codepoint <= 0x2b63u) {
		return tewi_block160[codepoint - 0x2b5au];
	} else if (codepoint >= 0x2b65u && codepoint <= 0x2b73u) {
		return tewi_block161[codepoint - 0x2b65u];
	} else if (codepoint >= 0x2b76u && codepoint <= 0x2b79u) {
		return tewi_block162[codepoint - 0x2b76u];
	} else if (codepoint == 0x2b7eu) {
		return tewi_block163[0];
	} else if (codepoint == 0x2b80u) {
		return tewi_block164[0];
	} else if (codepoint == 0x2b82u) {
		return tewi_block165[0];
	} else if (codepoint >= 0x2b88u && codepoint <= 0x2b93u) {
		return tewi_block166[codepoint - 0x2b88u];
	} else if (codepoint == 0x2b95u) {
		return tewi_block167[0];
	} else if (codepoint >= 0x2ba0u && codepoint <= 0x2bafu) {
		return tewi_block168[codepoint - 0x2ba0u];
	} else if (codepoint == 0x2bb8u) {
		return tewi_block169[0];
	} else if (codepoint >= 0x2bc0u && codepoint <= 0x2bc2u) {
		return tewi_block170[codepoint - 0x2bc0u];
	} else if (codepoint >= 0x2bc5u && codepoint <= 0x2bc8u) {
		return tewi_block171[codepoint - 0x2bc5u];
	} else if (codepoint >= 0x2bcau && codepoint <= 0x2bcfu) {
		return tewi_block172[codepoint - 0x2bcau];
	} else if (codepoint >= 0x2becu && codepoint <= 0x2befu) {
		return tewi_block173[codepoint - 0x2becu];
	} else if (codepoint >= 0x2c60u && codepoint <= 0x2c7fu) {
		return tewi_block174[codepoint - 0x2c60u];
	} else if (codepoint >= 0x2e00u && codepoint <= 0x2e0du) {
		return tewi_block175[codepoint - 0x2e00u];
	} else if (codepoint >= 0x2e0fu && codepoint <= 0x2e18u) {
		return tewi_block176[codepoint - 0x2e0fu];
	} else if (codepoint >= 0x2e1au && codepoint <= 0x2e40u) {
		return tewi_block177[codepoint - 0x2e1au];
	} else if (codepoint >= 0xe0a0u && codepoint <= 0xe0a2u) {
		return tewi_block178[codepoint - 0xe0a0u];
	} else if (codepoint >= 0xe0a5u && codepoint <= 0xe0acu) {
		return tewi_block179[codepoint - 0xe0a5u];
	} else if (codepoint >= 0xe0b0u && codepoint <= 0xe0b3u) {
		return tewi_block180[codepoint - 0xe0b0u];
	} else if (codepoint >= 0xe0b5u && codepoint <= 0xe0b8u) {
		return tewi_block181[codepoint - 0xe0b5u];
	} else if (codepoint >= 0xe0c0u && codepoint <= 0xe0c7u) {
		return tewi_block182[codepoint - 0xe0c0u];
	} else if (codepoint >= 0xfb01u && codepoint <= 0xfb02u) {
		return tewi_block183[codepoint - 0xfb01u];
	} else if (codepoint == 0xfffdu) {
		return tewi_block184[0];
	} else {
		WARN("Codepoint U+%04X (%s) is not covered by this font", codepoint, u8_cp_to_utf8(codepoint));
		return tewi_block1[0];
	}
}

static const unsigned char*
    tewib_get_bitmap(uint32_t codepoint)
{
	if (codepoint >= 0x20u && codepoint <= 0x7eu) {
		return tewib_block1[codepoint - 0x20u];
	} else if (codepoint >= 0xa0u && codepoint <= 0x131u) {
		return tewib_block2[codepoint - 0xa0u];
	} else if (codepoint >= 0x134u && codepoint <= 0x1b2u) {
		return tewib_block3[codepoint - 0x134u];
	} else {
		WARN("Codepoint U+%04X (%s) is not covered by this font", codepoint, u8_cp_to_utf8(codepoint));
		return tewib_block1[0];
	}
}
