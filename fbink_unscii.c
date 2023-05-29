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

#include "fbink_unscii.h"

static const unsigned char*
    unscii_get_bitmap(uint32_t codepoint)
{
	if (codepoint <= 0x195u) {
		return unscii_block1[codepoint];
	} else if (codepoint >= 0x197u && codepoint <= 0x19fu) {
		return unscii_block2[codepoint - 0x197u];
	} else if (codepoint == 0x1b7u) {
		return unscii_block3[0];
	} else if (codepoint >= 0x1cdu && codepoint <= 0x1e3u) {
		return unscii_block4[codepoint - 0x1cdu];
	} else if (codepoint >= 0x1e6u && codepoint <= 0x1e9u) {
		return unscii_block5[codepoint - 0x1e6u];
	} else if (codepoint >= 0x1eeu && codepoint <= 0x1f0u) {
		return unscii_block6[codepoint - 0x1eeu];
	} else if (codepoint >= 0x1f4u && codepoint <= 0x1f5u) {
		return unscii_block7[codepoint - 0x1f4u];
	} else if (codepoint >= 0x1f8u && codepoint <= 0x21bu) {
		return unscii_block8[codepoint - 0x1f8u];
	} else if (codepoint >= 0x226u && codepoint <= 0x227u) {
		return unscii_block9[codepoint - 0x226u];
	} else if (codepoint >= 0x22au && codepoint <= 0x233u) {
		return unscii_block10[codepoint - 0x22au];
	} else if (codepoint >= 0x258u && codepoint <= 0x259u) {
		return unscii_block11[codepoint - 0x258u];
	} else if (codepoint == 0x263u) {
		return unscii_block12[0];
	} else if (codepoint == 0x292u) {
		return unscii_block13[0];
	} else if (codepoint == 0x294u) {
		return unscii_block14[0];
	} else if (codepoint == 0x2c6u) {
		return unscii_block15[0];
	} else if (codepoint == 0x2c9u) {
		return unscii_block16[0];
	} else if (codepoint == 0x2cdu) {
		return unscii_block17[0];
	} else if (codepoint == 0x2dcu) {
		return unscii_block18[0];
	} else if (codepoint >= 0x300u && codepoint <= 0x30du) {
		return unscii_block19[codepoint - 0x300u];
	} else if (codepoint >= 0x30fu && codepoint <= 0x311u) {
		return unscii_block20[codepoint - 0x30fu];
	} else if (codepoint >= 0x313u && codepoint <= 0x318u) {
		return unscii_block21[codepoint - 0x313u];
	} else if (codepoint >= 0x31bu && codepoint <= 0x31cu) {
		return unscii_block22[codepoint - 0x31bu];
	} else if (codepoint >= 0x31fu && codepoint <= 0x329u) {
		return unscii_block23[codepoint - 0x31fu];
	} else if (codepoint >= 0x32bu && codepoint <= 0x32cu) {
		return unscii_block24[codepoint - 0x32bu];
	} else if (codepoint == 0x32eu) {
		return unscii_block25[0];
	} else if (codepoint == 0x330u) {
		return unscii_block26[0];
	} else if (codepoint >= 0x332u && codepoint <= 0x333u) {
		return unscii_block27[codepoint - 0x332u];
	} else if (codepoint >= 0x335u && codepoint <= 0x33eu) {
		return unscii_block28[codepoint - 0x335u];
	} else if (codepoint >= 0x340u && codepoint <= 0x344u) {
		return unscii_block29[codepoint - 0x340u];
	} else if (codepoint == 0x34fu) {
		return unscii_block30[0];
	} else if (codepoint == 0x362u) {
		return unscii_block31[0];
	} else if (codepoint == 0x386u) {
		return unscii_block32[0];
	} else if (codepoint >= 0x388u && codepoint <= 0x38au) {
		return unscii_block33[codepoint - 0x388u];
	} else if (codepoint == 0x38cu) {
		return unscii_block34[0];
	} else if (codepoint >= 0x38eu && codepoint <= 0x38fu) {
		return unscii_block35[codepoint - 0x38eu];
	} else if (codepoint >= 0x391u && codepoint <= 0x3a1u) {
		return unscii_block36[codepoint - 0x391u];
	} else if (codepoint >= 0x3a3u && codepoint <= 0x3afu) {
		return unscii_block37[codepoint - 0x3a3u];
	} else if (codepoint >= 0x3b1u && codepoint <= 0x3ceu) {
		return unscii_block38[codepoint - 0x3b1u];
	} else if (codepoint >= 0x400u && codepoint <= 0x401u) {
		return unscii_block39[codepoint - 0x400u];
	} else if (codepoint >= 0x405u && codepoint <= 0x409u) {
		return unscii_block40[codepoint - 0x405u];
	} else if (codepoint >= 0x410u && codepoint <= 0x451u) {
		return unscii_block41[codepoint - 0x410u];
	} else if (codepoint >= 0x455u && codepoint <= 0x458u) {
		return unscii_block42[codepoint - 0x455u];
	} else if (codepoint >= 0x4d0u && codepoint <= 0x4d7u) {
		return unscii_block43[codepoint - 0x4d0u];
	} else if (codepoint >= 0x4e6u && codepoint <= 0x4e7u) {
		return unscii_block44[codepoint - 0x4e6u];
	} else if (codepoint == 0x4f1u) {
		return unscii_block45[0];
	} else if (codepoint >= 0x5d0u && codepoint <= 0x5eau) {
		return unscii_block46[codepoint - 0x5d0u];
	} else if (codepoint == 0x623u) {
		return unscii_block47[0];
	} else if (codepoint >= 0x627u && codepoint <= 0x62du) {
		return unscii_block48[codepoint - 0x627u];
	} else if (codepoint >= 0x62fu && codepoint <= 0x635u) {
		return unscii_block49[codepoint - 0x62fu];
	} else if (codepoint >= 0x637u && codepoint <= 0x63au) {
		return unscii_block50[codepoint - 0x637u];
	} else if (codepoint >= 0x640u && codepoint <= 0x64au) {
		return unscii_block51[codepoint - 0x640u];
	} else if (codepoint >= 0x671u && codepoint <= 0x673u) {
		return unscii_block52[codepoint - 0x671u];
	} else if (codepoint >= 0x1560u && codepoint <= 0x1563u) {
		return unscii_block53[codepoint - 0x1560u];
	} else if (codepoint == 0x156au) {
		return unscii_block54[0];
	} else if (codepoint >= 0x156cu && codepoint <= 0x1571u) {
		return unscii_block55[codepoint - 0x156cu];
	} else if (codepoint >= 0x15ecu && codepoint <= 0x15edu) {
		return unscii_block56[codepoint - 0x15ecu];
	} else if (codepoint == 0x15efu) {
		return unscii_block57[0];
	} else if (codepoint >= 0x16a0u && codepoint <= 0x16abu) {
		return unscii_block58[codepoint - 0x16a0u];
	} else if (codepoint >= 0x16b1u && codepoint <= 0x16f8u) {
		return unscii_block59[codepoint - 0x16b1u];
	} else if (codepoint >= 0x1e00u && codepoint <= 0x1e02u) {
		return unscii_block60[codepoint - 0x1e00u];
	} else if (codepoint >= 0x1e04u && codepoint <= 0x1e07u) {
		return unscii_block61[codepoint - 0x1e04u];
	} else if (codepoint == 0x1e0au) {
		return unscii_block62[0];
	} else if (codepoint >= 0x1e0cu && codepoint <= 0x1e0fu) {
		return unscii_block63[codepoint - 0x1e0cu];
	} else if (codepoint >= 0x1e41u && codepoint <= 0x1e4bu) {
		return unscii_block64[codepoint - 0x1e41u];
	} else if (codepoint >= 0x1e58u && codepoint <= 0x1e5bu) {
		return unscii_block65[codepoint - 0x1e58u];
	} else if (codepoint == 0x1e5eu) {
		return unscii_block66[0];
	} else if (codepoint >= 0x1f00u && codepoint <= 0x1f7du) {
		return unscii_block67[codepoint - 0x1f00u];
	} else if (codepoint >= 0x1f80u && codepoint <= 0x1fb4u) {
		return unscii_block68[codepoint - 0x1f80u];
	} else if (codepoint >= 0x1fb6u && codepoint <= 0x1fbcu) {
		return unscii_block69[codepoint - 0x1fb6u];
	} else if (codepoint >= 0x1fc2u && codepoint <= 0x1fc4u) {
		return unscii_block70[codepoint - 0x1fc2u];
	} else if (codepoint >= 0x1fc6u && codepoint <= 0x1fc7u) {
		return unscii_block71[codepoint - 0x1fc6u];
	} else if (codepoint >= 0x1fcau && codepoint <= 0x1fccu) {
		return unscii_block72[codepoint - 0x1fcau];
	} else if (codepoint >= 0x1fd0u && codepoint <= 0x1fd4u) {
		return unscii_block73[codepoint - 0x1fd0u];
	} else if (codepoint >= 0x1fd6u && codepoint <= 0x1fdcu) {
		return unscii_block74[codepoint - 0x1fd6u];
	} else if (codepoint >= 0x1fe0u && codepoint <= 0x1fe3u) {
		return unscii_block75[codepoint - 0x1fe0u];
	} else if (codepoint >= 0x1fe6u && codepoint <= 0x1febu) {
		return unscii_block76[codepoint - 0x1fe6u];
	} else if (codepoint >= 0x1ff2u && codepoint <= 0x1ff4u) {
		return unscii_block77[codepoint - 0x1ff2u];
	} else if (codepoint >= 0x1ff6u && codepoint <= 0x1ff7u) {
		return unscii_block78[codepoint - 0x1ff6u];
	} else if (codepoint >= 0x1ffau && codepoint <= 0x1ffcu) {
		return unscii_block79[codepoint - 0x1ffau];
	} else if (codepoint >= 0x2010u && codepoint <= 0x2019u) {
		return unscii_block80[codepoint - 0x2010u];
	} else if (codepoint >= 0x201cu && codepoint <= 0x201du) {
		return unscii_block81[codepoint - 0x201cu];
	} else if (codepoint >= 0x2020u && codepoint <= 0x2022u) {
		return unscii_block82[codepoint - 0x2020u];
	} else if (codepoint >= 0x2024u && codepoint <= 0x2026u) {
		return unscii_block83[codepoint - 0x2024u];
	} else if (codepoint >= 0x2030u && codepoint <= 0x2037u) {
		return unscii_block84[codepoint - 0x2030u];
	} else if (codepoint >= 0x203cu && codepoint <= 0x203du) {
		return unscii_block85[codepoint - 0x203cu];
	} else if (codepoint == 0x2044u) {
		return unscii_block86[0];
	} else if (codepoint >= 0x2047u && codepoint <= 0x2049u) {
		return unscii_block87[codepoint - 0x2047u];
	} else if (codepoint >= 0x2070u && codepoint <= 0x2071u) {
		return unscii_block88[codepoint - 0x2070u];
	} else if (codepoint >= 0x2074u && codepoint <= 0x207cu) {
		return unscii_block89[codepoint - 0x2074u];
	} else if (codepoint == 0x207fu) {
		return unscii_block90[0];
	} else if (codepoint == 0x20a7u) {
		return unscii_block91[0];
	} else if (codepoint == 0x20acu) {
		return unscii_block92[0];
	} else if (codepoint == 0x2117u) {
		return unscii_block93[0];
	} else if (codepoint == 0x2120u) {
		return unscii_block94[0];
	} else if (codepoint == 0x2122u) {
		return unscii_block95[0];
	} else if (codepoint >= 0x2150u && codepoint <= 0x215fu) {
		return unscii_block96[codepoint - 0x2150u];
	} else if (codepoint == 0x2189u) {
		return unscii_block97[0];
	} else if (codepoint >= 0x2190u && codepoint <= 0x2199u) {
		return unscii_block98[codepoint - 0x2190u];
	} else if (codepoint == 0x21a5u) {
		return unscii_block99[0];
	} else if (codepoint == 0x21a8u) {
		return unscii_block100[0];
	} else if (codepoint == 0x21b2u) {
		return unscii_block101[0];
	} else if (codepoint >= 0x21bcu && codepoint <= 0x21c3u) {
		return unscii_block102[codepoint - 0x21bcu];
	} else if (codepoint >= 0x21e6u && codepoint <= 0x21e9u) {
		return unscii_block103[codepoint - 0x21e6u];
	} else if (codepoint == 0x2205u) {
		return unscii_block104[0];
	} else if (codepoint >= 0x2212u && codepoint <= 0x2215u) {
		return unscii_block105[codepoint - 0x2212u];
	} else if (codepoint == 0x2218u) {
		return unscii_block106[0];
	} else if (codepoint >= 0x221au && codepoint <= 0x221cu) {
		return unscii_block107[codepoint - 0x221au];
	} else if (codepoint >= 0x221eu && codepoint <= 0x221fu) {
		return unscii_block108[codepoint - 0x221eu];
	} else if (codepoint == 0x2229u) {
		return unscii_block109[0];
	} else if (codepoint == 0x2248u) {
		return unscii_block110[0];
	} else if (codepoint == 0x2261u) {
		return unscii_block111[0];
	} else if (codepoint >= 0x2264u && codepoint <= 0x2265u) {
		return unscii_block112[codepoint - 0x2264u];
	} else if (codepoint == 0x22c5u) {
		return unscii_block113[0];
	} else if (codepoint == 0x2301u) {
		return unscii_block114[0];
	} else if (codepoint == 0x2318u) {
		return unscii_block115[0];
	} else if (codepoint >= 0x231au && codepoint <= 0x231bu) {
		return unscii_block116[codepoint - 0x231au];
	} else if (codepoint >= 0x2320u && codepoint <= 0x2321u) {
		return unscii_block117[codepoint - 0x2320u];
	} else if (codepoint >= 0x239bu && codepoint <= 0x23afu) {
		return unscii_block118[codepoint - 0x239bu];
	} else if (codepoint >= 0x23b2u && codepoint <= 0x23b3u) {
		return unscii_block119[codepoint - 0x23b2u];
	} else if (codepoint >= 0x23b7u && codepoint <= 0x23bdu) {
		return unscii_block120[codepoint - 0x23b7u];
	} else if (codepoint == 0x23d0u) {
		return unscii_block121[0];
	} else if (codepoint == 0x23dau) {
		return unscii_block122[0];
	} else if (codepoint >= 0x23e9u && codepoint <= 0x23ecu) {
		return unscii_block123[codepoint - 0x23e9u];
	} else if (codepoint >= 0x2500u && codepoint <= 0x25ffu) {
		return unscii_block124[codepoint - 0x2500u];
	} else if (codepoint == 0x2602u) {
		return unscii_block125[0];
	} else if (codepoint >= 0x2605u && codepoint <= 0x2606u) {
		return unscii_block126[codepoint - 0x2605u];
	} else if (codepoint == 0x2609u) {
		return unscii_block127[0];
	} else if (codepoint == 0x260eu) {
		return unscii_block128[0];
	} else if (codepoint >= 0x2610u && codepoint <= 0x2612u) {
		return unscii_block129[codepoint - 0x2610u];
	} else if (codepoint == 0x2615u) {
		return unscii_block130[0];
	} else if (codepoint == 0x2625u) {
		return unscii_block131[0];
	} else if (codepoint == 0x2628u) {
		return unscii_block132[0];
	} else if (codepoint >= 0x262fu && codepoint <= 0x2637u) {
		return unscii_block133[codepoint - 0x262fu];
	} else if (codepoint >= 0x2639u && codepoint <= 0x263bu) {
		return unscii_block134[codepoint - 0x2639u];
	} else if (codepoint == 0x2640u) {
		return unscii_block135[0];
	} else if (codepoint == 0x2642u) {
		return unscii_block136[0];
	} else if (codepoint >= 0x2660u && codepoint <= 0x2667u) {
		return unscii_block137[codepoint - 0x2660u];
	} else if (codepoint >= 0x2669u && codepoint <= 0x266cu) {
		return unscii_block138[codepoint - 0x2669u];
	} else if (codepoint == 0x2683u) {
		return unscii_block139[0];
	} else if (codepoint >= 0x268au && codepoint <= 0x268fu) {
		return unscii_block140[codepoint - 0x268au];
	} else if (codepoint >= 0x269eu && codepoint <= 0x269fu) {
		return unscii_block141[codepoint - 0x269eu];
	} else if (codepoint >= 0x26aau && codepoint <= 0x26acu) {
		return unscii_block142[codepoint - 0x26aau];
	} else if (codepoint == 0x26c8u) {
		return unscii_block143[0];
	} else if (codepoint == 0x26f5u) {
		return unscii_block144[0];
	} else if (codepoint == 0x2708u) {
		return unscii_block145[0];
	} else if (codepoint == 0x2713u) {
		return unscii_block146[0];
	} else if (codepoint >= 0x2734u && codepoint <= 0x2735u) {
		return unscii_block147[codepoint - 0x2734u];
	} else if (codepoint == 0x2764u) {
		return unscii_block148[0];
	} else if (codepoint >= 0x2800u && codepoint <= 0x28ffu) {
		return unscii_block149[codepoint - 0x2800u];
	} else if (codepoint == 0x29c9u) {
		return unscii_block150[0];
	} else if (codepoint >= 0x2b12u && codepoint <= 0x2b19u) {
		return unscii_block151[codepoint - 0x2b12u];
	} else if (codepoint == 0x2b1du) {
		return unscii_block152[0];
	} else if (codepoint >= 0x2b24u && codepoint <= 0x2b2fu) {
		return unscii_block153[codepoint - 0x2b24u];
	} else if (codepoint == 0x2b55u) {
		return unscii_block154[0];
	} else if (codepoint == 0x2b58u) {
		return unscii_block155[0];
	} else if (codepoint == 0x2b73u) {
		return unscii_block156[0];
	} else if (codepoint >= 0x2b9cu && codepoint <= 0x2b9fu) {
		return unscii_block157[codepoint - 0x2b9cu];
	} else if (codepoint == 0x2bbau) {
		return unscii_block158[0];
	} else if (codepoint == 0x2bc0u) {
		return unscii_block159[0];
	} else if (codepoint >= 0x2bc5u && codepoint <= 0x2bc8u) {
		return unscii_block160[codepoint - 0x2bc5u];
	} else if (codepoint >= 0x2bcau && codepoint <= 0x2bcbu) {
		return unscii_block161[codepoint - 0x2bcau];
	} else if (codepoint == 0x2e2eu) {
		return unscii_block162[0];
	} else if (codepoint >= 0x4dc0u && codepoint <= 0x4dffu) {
		return unscii_block163[codepoint - 0x4dc0u];
	} else if (codepoint >= 0xe080u && codepoint <= 0xe1aau) {
		return unscii_block164[codepoint - 0xe080u];
	} else if (codepoint >= 0xe800u && codepoint <= 0xe87du) {
		return unscii_block165[codepoint - 0xe800u];
	} else if (codepoint >= 0xec00u && codepoint <= 0xec07u) {
		return unscii_block166[codepoint - 0xec00u];
	} else if (codepoint >= 0xec0au && codepoint <= 0xec7du) {
		return unscii_block167[codepoint - 0xec0au];
	} else if (codepoint >= 0xec7fu && codepoint <= 0xecdeu) {
		return unscii_block168[codepoint - 0xec7fu];
	} else if (codepoint >= 0xfe81u && codepoint <= 0xfe82u) {
		return unscii_block169[codepoint - 0xfe81u];
	} else if (codepoint == 0xfe84u) {
		return unscii_block170[0];
	} else if (codepoint == 0xfe87u) {
		return unscii_block171[0];
	} else if (codepoint >= 0xfe8fu && codepoint <= 0xfe9cu) {
		return unscii_block172[codepoint - 0xfe8fu];
	} else if (codepoint >= 0xfe9eu && codepoint <= 0xfea4u) {
		return unscii_block173[codepoint - 0xfe9eu];
	} else if (codepoint >= 0xfea9u && codepoint <= 0xfeadu) {
		return unscii_block174[codepoint - 0xfea9u];
	} else if (codepoint == 0xfeafu) {
		return unscii_block175[0];
	} else if (codepoint >= 0xfeb1u && codepoint <= 0xfeb7u) {
		return unscii_block176[codepoint - 0xfeb1u];
	} else if (codepoint >= 0xfeb9u && codepoint <= 0xfebcu) {
		return unscii_block177[codepoint - 0xfeb9u];
	} else if (codepoint >= 0xfec1u && codepoint <= 0xfec6u) {
		return unscii_block178[codepoint - 0xfec1u];
	} else if (codepoint >= 0xfec9u && codepoint <= 0xfeddu) {
		return unscii_block179[codepoint - 0xfec9u];
	} else if (codepoint >= 0xfedfu && codepoint <= 0xfee3u) {
		return unscii_block180[codepoint - 0xfedfu];
	} else if (codepoint >= 0xfee5u && codepoint <= 0xfee9u) {
		return unscii_block181[codepoint - 0xfee5u];
	} else if (codepoint >= 0xfeebu && codepoint <= 0xfef4u) {
		return unscii_block182[codepoint - 0xfeebu];
	} else if (codepoint >= 0xff01u && codepoint <= 0xff36u) {
		return unscii_block183[codepoint - 0xff01u];
	} else if (codepoint >= 0xff38u && codepoint <= 0xff5du) {
		return unscii_block184[codepoint - 0xff38u];
	} else if (codepoint >= 0xff5fu && codepoint <= 0xff9fu) {
		return unscii_block185[codepoint - 0xff5fu];
	} else if (codepoint >= 0x1d300u && codepoint <= 0x1d356u) {
		return unscii_block186[codepoint - 0x1d300u];
	} else if (codepoint >= 0x1f332u && codepoint <= 0x1f333u) {
		return unscii_block187[codepoint - 0x1f332u];
	} else if (codepoint >= 0x1f344u && codepoint <= 0x1f345u) {
		return unscii_block188[codepoint - 0x1f344u];
	} else if (codepoint >= 0x1f34eu && codepoint <= 0x1f34fu) {
		return unscii_block189[codepoint - 0x1f34eu];
	} else if (codepoint == 0x1f37au) {
		return unscii_block190[0];
	} else if (codepoint == 0x1f37eu) {
		return unscii_block191[0];
	} else if (codepoint == 0x1f3ceu) {
		return unscii_block192[0];
	} else if (codepoint == 0x1f3d0u) {
		return unscii_block193[0];
	} else if (codepoint == 0x1f3f0u) {
		return unscii_block194[0];
	} else if (codepoint >= 0x1f407u && codepoint <= 0x1f408u) {
		return unscii_block195[codepoint - 0x1f407u];
	} else if (codepoint == 0x1f40du) {
		return unscii_block196[0];
	} else if (codepoint == 0x1f419u) {
		return unscii_block197[0];
	} else if (codepoint == 0x1f41fu) {
		return unscii_block198[0];
	} else if (codepoint == 0x1f42au) {
		return unscii_block199[0];
	} else if (codepoint == 0x1f47bu) {
		return unscii_block200[0];
	} else if (codepoint == 0x1f47eu) {
		return unscii_block201[0];
	} else if (codepoint == 0x1f480u) {
		return unscii_block202[0];
	} else if (codepoint >= 0x1f48du && codepoint <= 0x1f48eu) {
		return unscii_block203[codepoint - 0x1f48du];
	} else if (codepoint == 0x1f4a7u) {
		return unscii_block204[0];
	} else if (codepoint == 0x1f4adu) {
		return unscii_block205[0];
	} else if (codepoint == 0x1f4b0u) {
		return unscii_block206[0];
	} else if (codepoint == 0x1f4beu) {
		return unscii_block207[0];
	} else if (codepoint == 0x1f4c1u) {
		return unscii_block208[0];
	} else if (codepoint == 0x1f4c4u) {
		return unscii_block209[0];
	} else if (codepoint == 0x1f4dcu) {
		return unscii_block210[0];
	} else if (codepoint == 0x1f4fbu) {
		return unscii_block211[0];
	} else if (codepoint == 0x1f514u) {
		return unscii_block212[0];
	} else if (codepoint == 0x1f52bu) {
		return unscii_block213[0];
	} else if (codepoint == 0x1f52eu) {
		return unscii_block214[0];
	} else if (codepoint == 0x1f574u) {
		return unscii_block215[0];
	} else if (codepoint == 0x1f577u) {
		return unscii_block216[0];
	} else if (codepoint == 0x1f5a8u) {
		return unscii_block217[0];
	} else if (codepoint == 0x1f5d1u) {
		return unscii_block218[0];
	} else if (codepoint == 0x1f68du) {
		return unscii_block219[0];
	} else if (codepoint == 0x1f6d6u) {
		return unscii_block220[0];
	} else if (codepoint == 0x1f6f8u) {
		return unscii_block221[0];
	} else if (codepoint == 0x1f860u) {
		return unscii_block222[0];
	} else if (codepoint == 0x1f986u) {
		return unscii_block223[0];
	} else if (codepoint == 0x1f98bu) {
		return unscii_block224[0];
	} else if (codepoint == 0x1f9f2u) {
		return unscii_block225[0];
	} else if (codepoint == 0x1fa90u) {
		return unscii_block226[0];
	} else if (codepoint == 0x1fa93u) {
		return unscii_block227[0];
	} else if (codepoint == 0x1fa99u) {
		return unscii_block228[0];
	} else if (codepoint == 0x1fa9cu) {
		return unscii_block229[0];
	} else if (codepoint >= 0x1faa6u && codepoint <= 0x1faa8u) {
		return unscii_block230[codepoint - 0x1faa6u];
	} else if (codepoint >= 0x1fb00u && codepoint <= 0x1fbcau) {
		return unscii_block231[codepoint - 0x1fb00u];
	} else if (codepoint >= 0x1fbf0u && codepoint <= 0x1fbf9u) {
		return unscii_block232[codepoint - 0x1fbf0u];
	} else {
		WARN("Codepoint U+%04X (%s) is not covered by this font", codepoint, u8_cp_to_utf8(codepoint));
		return unscii_block1[0];
	}
}

static const unsigned char*
    alt_get_bitmap(uint32_t codepoint)
{
	if (codepoint <= 0x195u) {
		return alt_block1[codepoint];
	} else if (codepoint >= 0x197u && codepoint <= 0x19fu) {
		return alt_block2[codepoint - 0x197u];
	} else if (codepoint == 0x1b7u) {
		return alt_block3[0];
	} else if (codepoint >= 0x1cdu && codepoint <= 0x1e3u) {
		return alt_block4[codepoint - 0x1cdu];
	} else if (codepoint >= 0x1e6u && codepoint <= 0x1e9u) {
		return alt_block5[codepoint - 0x1e6u];
	} else if (codepoint >= 0x1eeu && codepoint <= 0x1f0u) {
		return alt_block6[codepoint - 0x1eeu];
	} else if (codepoint >= 0x1f4u && codepoint <= 0x1f5u) {
		return alt_block7[codepoint - 0x1f4u];
	} else if (codepoint >= 0x1f8u && codepoint <= 0x21bu) {
		return alt_block8[codepoint - 0x1f8u];
	} else if (codepoint >= 0x226u && codepoint <= 0x227u) {
		return alt_block9[codepoint - 0x226u];
	} else if (codepoint >= 0x22au && codepoint <= 0x233u) {
		return alt_block10[codepoint - 0x22au];
	} else if (codepoint >= 0x258u && codepoint <= 0x259u) {
		return alt_block11[codepoint - 0x258u];
	} else if (codepoint == 0x263u) {
		return alt_block12[0];
	} else if (codepoint == 0x292u) {
		return alt_block13[0];
	} else if (codepoint == 0x294u) {
		return alt_block14[0];
	} else if (codepoint == 0x2c6u) {
		return alt_block15[0];
	} else if (codepoint == 0x2c9u) {
		return alt_block16[0];
	} else if (codepoint == 0x2cdu) {
		return alt_block17[0];
	} else if (codepoint == 0x2dcu) {
		return alt_block18[0];
	} else if (codepoint >= 0x300u && codepoint <= 0x30du) {
		return alt_block19[codepoint - 0x300u];
	} else if (codepoint >= 0x30fu && codepoint <= 0x311u) {
		return alt_block20[codepoint - 0x30fu];
	} else if (codepoint >= 0x313u && codepoint <= 0x318u) {
		return alt_block21[codepoint - 0x313u];
	} else if (codepoint >= 0x31bu && codepoint <= 0x31cu) {
		return alt_block22[codepoint - 0x31bu];
	} else if (codepoint >= 0x31fu && codepoint <= 0x329u) {
		return alt_block23[codepoint - 0x31fu];
	} else if (codepoint >= 0x32bu && codepoint <= 0x32cu) {
		return alt_block24[codepoint - 0x32bu];
	} else if (codepoint == 0x32eu) {
		return alt_block25[0];
	} else if (codepoint == 0x330u) {
		return alt_block26[0];
	} else if (codepoint >= 0x332u && codepoint <= 0x333u) {
		return alt_block27[codepoint - 0x332u];
	} else if (codepoint >= 0x335u && codepoint <= 0x33eu) {
		return alt_block28[codepoint - 0x335u];
	} else if (codepoint >= 0x340u && codepoint <= 0x344u) {
		return alt_block29[codepoint - 0x340u];
	} else if (codepoint == 0x34fu) {
		return alt_block30[0];
	} else if (codepoint == 0x362u) {
		return alt_block31[0];
	} else if (codepoint == 0x386u) {
		return alt_block32[0];
	} else if (codepoint >= 0x388u && codepoint <= 0x38au) {
		return alt_block33[codepoint - 0x388u];
	} else if (codepoint == 0x38cu) {
		return alt_block34[0];
	} else if (codepoint >= 0x38eu && codepoint <= 0x38fu) {
		return alt_block35[codepoint - 0x38eu];
	} else if (codepoint >= 0x391u && codepoint <= 0x3a1u) {
		return alt_block36[codepoint - 0x391u];
	} else if (codepoint >= 0x3a3u && codepoint <= 0x3afu) {
		return alt_block37[codepoint - 0x3a3u];
	} else if (codepoint >= 0x3b1u && codepoint <= 0x3ceu) {
		return alt_block38[codepoint - 0x3b1u];
	} else if (codepoint >= 0x400u && codepoint <= 0x401u) {
		return alt_block39[codepoint - 0x400u];
	} else if (codepoint >= 0x405u && codepoint <= 0x409u) {
		return alt_block40[codepoint - 0x405u];
	} else if (codepoint >= 0x410u && codepoint <= 0x451u) {
		return alt_block41[codepoint - 0x410u];
	} else if (codepoint >= 0x455u && codepoint <= 0x458u) {
		return alt_block42[codepoint - 0x455u];
	} else if (codepoint >= 0x4d0u && codepoint <= 0x4d7u) {
		return alt_block43[codepoint - 0x4d0u];
	} else if (codepoint >= 0x4e6u && codepoint <= 0x4e7u) {
		return alt_block44[codepoint - 0x4e6u];
	} else if (codepoint == 0x4f1u) {
		return alt_block45[0];
	} else if (codepoint >= 0x5d0u && codepoint <= 0x5eau) {
		return alt_block46[codepoint - 0x5d0u];
	} else if (codepoint == 0x623u) {
		return alt_block47[0];
	} else if (codepoint >= 0x627u && codepoint <= 0x62du) {
		return alt_block48[codepoint - 0x627u];
	} else if (codepoint >= 0x62fu && codepoint <= 0x635u) {
		return alt_block49[codepoint - 0x62fu];
	} else if (codepoint >= 0x637u && codepoint <= 0x63au) {
		return alt_block50[codepoint - 0x637u];
	} else if (codepoint >= 0x640u && codepoint <= 0x64au) {
		return alt_block51[codepoint - 0x640u];
	} else if (codepoint >= 0x671u && codepoint <= 0x673u) {
		return alt_block52[codepoint - 0x671u];
	} else if (codepoint >= 0x1560u && codepoint <= 0x1563u) {
		return alt_block53[codepoint - 0x1560u];
	} else if (codepoint == 0x156au) {
		return alt_block54[0];
	} else if (codepoint >= 0x156cu && codepoint <= 0x1571u) {
		return alt_block55[codepoint - 0x156cu];
	} else if (codepoint >= 0x15ecu && codepoint <= 0x15edu) {
		return alt_block56[codepoint - 0x15ecu];
	} else if (codepoint == 0x15efu) {
		return alt_block57[0];
	} else if (codepoint >= 0x16a0u && codepoint <= 0x16abu) {
		return alt_block58[codepoint - 0x16a0u];
	} else if (codepoint >= 0x16b1u && codepoint <= 0x16f8u) {
		return alt_block59[codepoint - 0x16b1u];
	} else if (codepoint >= 0x1e00u && codepoint <= 0x1e02u) {
		return alt_block60[codepoint - 0x1e00u];
	} else if (codepoint >= 0x1e04u && codepoint <= 0x1e07u) {
		return alt_block61[codepoint - 0x1e04u];
	} else if (codepoint == 0x1e0au) {
		return alt_block62[0];
	} else if (codepoint >= 0x1e0cu && codepoint <= 0x1e0fu) {
		return alt_block63[codepoint - 0x1e0cu];
	} else if (codepoint >= 0x1e41u && codepoint <= 0x1e4bu) {
		return alt_block64[codepoint - 0x1e41u];
	} else if (codepoint >= 0x1e58u && codepoint <= 0x1e5bu) {
		return alt_block65[codepoint - 0x1e58u];
	} else if (codepoint == 0x1e5eu) {
		return alt_block66[0];
	} else if (codepoint >= 0x1f00u && codepoint <= 0x1f7du) {
		return alt_block67[codepoint - 0x1f00u];
	} else if (codepoint >= 0x1f80u && codepoint <= 0x1fb4u) {
		return alt_block68[codepoint - 0x1f80u];
	} else if (codepoint >= 0x1fb6u && codepoint <= 0x1fbcu) {
		return alt_block69[codepoint - 0x1fb6u];
	} else if (codepoint >= 0x1fc2u && codepoint <= 0x1fc4u) {
		return alt_block70[codepoint - 0x1fc2u];
	} else if (codepoint >= 0x1fc6u && codepoint <= 0x1fc7u) {
		return alt_block71[codepoint - 0x1fc6u];
	} else if (codepoint >= 0x1fcau && codepoint <= 0x1fccu) {
		return alt_block72[codepoint - 0x1fcau];
	} else if (codepoint >= 0x1fd0u && codepoint <= 0x1fd4u) {
		return alt_block73[codepoint - 0x1fd0u];
	} else if (codepoint >= 0x1fd6u && codepoint <= 0x1fdcu) {
		return alt_block74[codepoint - 0x1fd6u];
	} else if (codepoint >= 0x1fe0u && codepoint <= 0x1fe3u) {
		return alt_block75[codepoint - 0x1fe0u];
	} else if (codepoint >= 0x1fe6u && codepoint <= 0x1febu) {
		return alt_block76[codepoint - 0x1fe6u];
	} else if (codepoint >= 0x1ff2u && codepoint <= 0x1ff4u) {
		return alt_block77[codepoint - 0x1ff2u];
	} else if (codepoint >= 0x1ff6u && codepoint <= 0x1ff7u) {
		return alt_block78[codepoint - 0x1ff6u];
	} else if (codepoint >= 0x1ffau && codepoint <= 0x1ffcu) {
		return alt_block79[codepoint - 0x1ffau];
	} else if (codepoint >= 0x2010u && codepoint <= 0x2019u) {
		return alt_block80[codepoint - 0x2010u];
	} else if (codepoint >= 0x201cu && codepoint <= 0x201du) {
		return alt_block81[codepoint - 0x201cu];
	} else if (codepoint >= 0x2020u && codepoint <= 0x2022u) {
		return alt_block82[codepoint - 0x2020u];
	} else if (codepoint >= 0x2024u && codepoint <= 0x2026u) {
		return alt_block83[codepoint - 0x2024u];
	} else if (codepoint >= 0x2030u && codepoint <= 0x2037u) {
		return alt_block84[codepoint - 0x2030u];
	} else if (codepoint >= 0x203cu && codepoint <= 0x203du) {
		return alt_block85[codepoint - 0x203cu];
	} else if (codepoint == 0x2044u) {
		return alt_block86[0];
	} else if (codepoint >= 0x2047u && codepoint <= 0x2049u) {
		return alt_block87[codepoint - 0x2047u];
	} else if (codepoint >= 0x2070u && codepoint <= 0x2071u) {
		return alt_block88[codepoint - 0x2070u];
	} else if (codepoint >= 0x2074u && codepoint <= 0x207cu) {
		return alt_block89[codepoint - 0x2074u];
	} else if (codepoint == 0x207fu) {
		return alt_block90[0];
	} else if (codepoint == 0x20a7u) {
		return alt_block91[0];
	} else if (codepoint == 0x20acu) {
		return alt_block92[0];
	} else if (codepoint == 0x2117u) {
		return alt_block93[0];
	} else if (codepoint == 0x2120u) {
		return alt_block94[0];
	} else if (codepoint == 0x2122u) {
		return alt_block95[0];
	} else if (codepoint >= 0x2150u && codepoint <= 0x215fu) {
		return alt_block96[codepoint - 0x2150u];
	} else if (codepoint == 0x2189u) {
		return alt_block97[0];
	} else if (codepoint >= 0x2190u && codepoint <= 0x2199u) {
		return alt_block98[codepoint - 0x2190u];
	} else if (codepoint == 0x21a5u) {
		return alt_block99[0];
	} else if (codepoint == 0x21a8u) {
		return alt_block100[0];
	} else if (codepoint == 0x21b2u) {
		return alt_block101[0];
	} else if (codepoint >= 0x21bcu && codepoint <= 0x21c3u) {
		return alt_block102[codepoint - 0x21bcu];
	} else if (codepoint >= 0x21e6u && codepoint <= 0x21e9u) {
		return alt_block103[codepoint - 0x21e6u];
	} else if (codepoint == 0x2205u) {
		return alt_block104[0];
	} else if (codepoint >= 0x2212u && codepoint <= 0x2215u) {
		return alt_block105[codepoint - 0x2212u];
	} else if (codepoint == 0x2218u) {
		return alt_block106[0];
	} else if (codepoint >= 0x221au && codepoint <= 0x221cu) {
		return alt_block107[codepoint - 0x221au];
	} else if (codepoint >= 0x221eu && codepoint <= 0x221fu) {
		return alt_block108[codepoint - 0x221eu];
	} else if (codepoint == 0x2229u) {
		return alt_block109[0];
	} else if (codepoint == 0x2248u) {
		return alt_block110[0];
	} else if (codepoint == 0x2261u) {
		return alt_block111[0];
	} else if (codepoint >= 0x2264u && codepoint <= 0x2265u) {
		return alt_block112[codepoint - 0x2264u];
	} else if (codepoint == 0x22c5u) {
		return alt_block113[0];
	} else if (codepoint == 0x2301u) {
		return alt_block114[0];
	} else if (codepoint == 0x2318u) {
		return alt_block115[0];
	} else if (codepoint >= 0x231au && codepoint <= 0x231bu) {
		return alt_block116[codepoint - 0x231au];
	} else if (codepoint >= 0x2320u && codepoint <= 0x2321u) {
		return alt_block117[codepoint - 0x2320u];
	} else if (codepoint >= 0x239bu && codepoint <= 0x23afu) {
		return alt_block118[codepoint - 0x239bu];
	} else if (codepoint >= 0x23b2u && codepoint <= 0x23b3u) {
		return alt_block119[codepoint - 0x23b2u];
	} else if (codepoint >= 0x23b7u && codepoint <= 0x23bdu) {
		return alt_block120[codepoint - 0x23b7u];
	} else if (codepoint == 0x23d0u) {
		return alt_block121[0];
	} else if (codepoint == 0x23dau) {
		return alt_block122[0];
	} else if (codepoint >= 0x23e9u && codepoint <= 0x23ecu) {
		return alt_block123[codepoint - 0x23e9u];
	} else if (codepoint >= 0x2500u && codepoint <= 0x25ffu) {
		return alt_block124[codepoint - 0x2500u];
	} else if (codepoint == 0x2602u) {
		return alt_block125[0];
	} else if (codepoint >= 0x2605u && codepoint <= 0x2606u) {
		return alt_block126[codepoint - 0x2605u];
	} else if (codepoint == 0x2609u) {
		return alt_block127[0];
	} else if (codepoint == 0x260eu) {
		return alt_block128[0];
	} else if (codepoint >= 0x2610u && codepoint <= 0x2612u) {
		return alt_block129[codepoint - 0x2610u];
	} else if (codepoint == 0x2615u) {
		return alt_block130[0];
	} else if (codepoint == 0x2625u) {
		return alt_block131[0];
	} else if (codepoint == 0x2628u) {
		return alt_block132[0];
	} else if (codepoint >= 0x262fu && codepoint <= 0x2637u) {
		return alt_block133[codepoint - 0x262fu];
	} else if (codepoint >= 0x2639u && codepoint <= 0x263bu) {
		return alt_block134[codepoint - 0x2639u];
	} else if (codepoint == 0x2640u) {
		return alt_block135[0];
	} else if (codepoint == 0x2642u) {
		return alt_block136[0];
	} else if (codepoint >= 0x2660u && codepoint <= 0x2667u) {
		return alt_block137[codepoint - 0x2660u];
	} else if (codepoint >= 0x2669u && codepoint <= 0x266cu) {
		return alt_block138[codepoint - 0x2669u];
	} else if (codepoint == 0x2683u) {
		return alt_block139[0];
	} else if (codepoint >= 0x268au && codepoint <= 0x268fu) {
		return alt_block140[codepoint - 0x268au];
	} else if (codepoint >= 0x269eu && codepoint <= 0x269fu) {
		return alt_block141[codepoint - 0x269eu];
	} else if (codepoint >= 0x26aau && codepoint <= 0x26acu) {
		return alt_block142[codepoint - 0x26aau];
	} else if (codepoint == 0x26c8u) {
		return alt_block143[0];
	} else if (codepoint == 0x26f5u) {
		return alt_block144[0];
	} else if (codepoint == 0x2708u) {
		return alt_block145[0];
	} else if (codepoint == 0x2713u) {
		return alt_block146[0];
	} else if (codepoint >= 0x2734u && codepoint <= 0x2735u) {
		return alt_block147[codepoint - 0x2734u];
	} else if (codepoint == 0x2764u) {
		return alt_block148[0];
	} else if (codepoint >= 0x2800u && codepoint <= 0x28ffu) {
		return alt_block149[codepoint - 0x2800u];
	} else if (codepoint == 0x29c9u) {
		return alt_block150[0];
	} else if (codepoint >= 0x2b12u && codepoint <= 0x2b19u) {
		return alt_block151[codepoint - 0x2b12u];
	} else if (codepoint == 0x2b1du) {
		return alt_block152[0];
	} else if (codepoint >= 0x2b24u && codepoint <= 0x2b2fu) {
		return alt_block153[codepoint - 0x2b24u];
	} else if (codepoint == 0x2b55u) {
		return alt_block154[0];
	} else if (codepoint == 0x2b58u) {
		return alt_block155[0];
	} else if (codepoint == 0x2b73u) {
		return alt_block156[0];
	} else if (codepoint >= 0x2b9cu && codepoint <= 0x2b9fu) {
		return alt_block157[codepoint - 0x2b9cu];
	} else if (codepoint == 0x2bbau) {
		return alt_block158[0];
	} else if (codepoint == 0x2bc0u) {
		return alt_block159[0];
	} else if (codepoint >= 0x2bc5u && codepoint <= 0x2bc8u) {
		return alt_block160[codepoint - 0x2bc5u];
	} else if (codepoint >= 0x2bcau && codepoint <= 0x2bcbu) {
		return alt_block161[codepoint - 0x2bcau];
	} else if (codepoint == 0x2e2eu) {
		return alt_block162[0];
	} else if (codepoint >= 0x4dc0u && codepoint <= 0x4dffu) {
		return alt_block163[codepoint - 0x4dc0u];
	} else if (codepoint >= 0xe080u && codepoint <= 0xe1aau) {
		return alt_block164[codepoint - 0xe080u];
	} else if (codepoint >= 0xe800u && codepoint <= 0xe87du) {
		return alt_block165[codepoint - 0xe800u];
	} else if (codepoint >= 0xec00u && codepoint <= 0xec07u) {
		return alt_block166[codepoint - 0xec00u];
	} else if (codepoint >= 0xec0au && codepoint <= 0xec7du) {
		return alt_block167[codepoint - 0xec0au];
	} else if (codepoint >= 0xec7fu && codepoint <= 0xecdeu) {
		return alt_block168[codepoint - 0xec7fu];
	} else if (codepoint >= 0xfe81u && codepoint <= 0xfe82u) {
		return alt_block169[codepoint - 0xfe81u];
	} else if (codepoint == 0xfe84u) {
		return alt_block170[0];
	} else if (codepoint == 0xfe87u) {
		return alt_block171[0];
	} else if (codepoint >= 0xfe8fu && codepoint <= 0xfe9cu) {
		return alt_block172[codepoint - 0xfe8fu];
	} else if (codepoint >= 0xfe9eu && codepoint <= 0xfea4u) {
		return alt_block173[codepoint - 0xfe9eu];
	} else if (codepoint >= 0xfea9u && codepoint <= 0xfeadu) {
		return alt_block174[codepoint - 0xfea9u];
	} else if (codepoint == 0xfeafu) {
		return alt_block175[0];
	} else if (codepoint >= 0xfeb1u && codepoint <= 0xfeb7u) {
		return alt_block176[codepoint - 0xfeb1u];
	} else if (codepoint >= 0xfeb9u && codepoint <= 0xfebcu) {
		return alt_block177[codepoint - 0xfeb9u];
	} else if (codepoint >= 0xfec1u && codepoint <= 0xfec6u) {
		return alt_block178[codepoint - 0xfec1u];
	} else if (codepoint >= 0xfec9u && codepoint <= 0xfeddu) {
		return alt_block179[codepoint - 0xfec9u];
	} else if (codepoint >= 0xfedfu && codepoint <= 0xfee3u) {
		return alt_block180[codepoint - 0xfedfu];
	} else if (codepoint >= 0xfee5u && codepoint <= 0xfee9u) {
		return alt_block181[codepoint - 0xfee5u];
	} else if (codepoint >= 0xfeebu && codepoint <= 0xfef4u) {
		return alt_block182[codepoint - 0xfeebu];
	} else if (codepoint >= 0xff01u && codepoint <= 0xff36u) {
		return alt_block183[codepoint - 0xff01u];
	} else if (codepoint >= 0xff38u && codepoint <= 0xff5du) {
		return alt_block184[codepoint - 0xff38u];
	} else if (codepoint >= 0xff5fu && codepoint <= 0xff9fu) {
		return alt_block185[codepoint - 0xff5fu];
	} else if (codepoint >= 0x1d300u && codepoint <= 0x1d356u) {
		return alt_block186[codepoint - 0x1d300u];
	} else if (codepoint >= 0x1f332u && codepoint <= 0x1f333u) {
		return alt_block187[codepoint - 0x1f332u];
	} else if (codepoint >= 0x1f344u && codepoint <= 0x1f345u) {
		return alt_block188[codepoint - 0x1f344u];
	} else if (codepoint >= 0x1f34eu && codepoint <= 0x1f34fu) {
		return alt_block189[codepoint - 0x1f34eu];
	} else if (codepoint == 0x1f37au) {
		return alt_block190[0];
	} else if (codepoint == 0x1f37eu) {
		return alt_block191[0];
	} else if (codepoint == 0x1f3ceu) {
		return alt_block192[0];
	} else if (codepoint == 0x1f3d0u) {
		return alt_block193[0];
	} else if (codepoint == 0x1f3f0u) {
		return alt_block194[0];
	} else if (codepoint >= 0x1f407u && codepoint <= 0x1f408u) {
		return alt_block195[codepoint - 0x1f407u];
	} else if (codepoint == 0x1f40du) {
		return alt_block196[0];
	} else if (codepoint == 0x1f419u) {
		return alt_block197[0];
	} else if (codepoint == 0x1f41fu) {
		return alt_block198[0];
	} else if (codepoint == 0x1f42au) {
		return alt_block199[0];
	} else if (codepoint == 0x1f47bu) {
		return alt_block200[0];
	} else if (codepoint == 0x1f47eu) {
		return alt_block201[0];
	} else if (codepoint == 0x1f480u) {
		return alt_block202[0];
	} else if (codepoint >= 0x1f48du && codepoint <= 0x1f48eu) {
		return alt_block203[codepoint - 0x1f48du];
	} else if (codepoint == 0x1f4a7u) {
		return alt_block204[0];
	} else if (codepoint == 0x1f4adu) {
		return alt_block205[0];
	} else if (codepoint == 0x1f4b0u) {
		return alt_block206[0];
	} else if (codepoint == 0x1f4beu) {
		return alt_block207[0];
	} else if (codepoint == 0x1f4c1u) {
		return alt_block208[0];
	} else if (codepoint == 0x1f4c4u) {
		return alt_block209[0];
	} else if (codepoint == 0x1f4dcu) {
		return alt_block210[0];
	} else if (codepoint == 0x1f4fbu) {
		return alt_block211[0];
	} else if (codepoint == 0x1f514u) {
		return alt_block212[0];
	} else if (codepoint == 0x1f52bu) {
		return alt_block213[0];
	} else if (codepoint == 0x1f52eu) {
		return alt_block214[0];
	} else if (codepoint == 0x1f574u) {
		return alt_block215[0];
	} else if (codepoint == 0x1f577u) {
		return alt_block216[0];
	} else if (codepoint == 0x1f5a8u) {
		return alt_block217[0];
	} else if (codepoint == 0x1f5d1u) {
		return alt_block218[0];
	} else if (codepoint == 0x1f68du) {
		return alt_block219[0];
	} else if (codepoint == 0x1f6d6u) {
		return alt_block220[0];
	} else if (codepoint == 0x1f6f8u) {
		return alt_block221[0];
	} else if (codepoint == 0x1f860u) {
		return alt_block222[0];
	} else if (codepoint == 0x1f986u) {
		return alt_block223[0];
	} else if (codepoint == 0x1f98bu) {
		return alt_block224[0];
	} else if (codepoint == 0x1f9f2u) {
		return alt_block225[0];
	} else if (codepoint == 0x1fa90u) {
		return alt_block226[0];
	} else if (codepoint == 0x1fa93u) {
		return alt_block227[0];
	} else if (codepoint == 0x1fa99u) {
		return alt_block228[0];
	} else if (codepoint == 0x1fa9cu) {
		return alt_block229[0];
	} else if (codepoint >= 0x1faa6u && codepoint <= 0x1faa8u) {
		return alt_block230[codepoint - 0x1faa6u];
	} else if (codepoint >= 0x1fb00u && codepoint <= 0x1fbcau) {
		return alt_block231[codepoint - 0x1fb00u];
	} else if (codepoint >= 0x1fbf0u && codepoint <= 0x1fbf9u) {
		return alt_block232[codepoint - 0x1fbf0u];
	} else {
		WARN("Codepoint U+%04X (%s) is not covered by this font", codepoint, u8_cp_to_utf8(codepoint));
		return alt_block1[0];
	}
}

static const unsigned char*
    thin_get_bitmap(uint32_t codepoint)
{
	if (codepoint <= 0x195u) {
		return thin_block1[codepoint];
	} else if (codepoint >= 0x197u && codepoint <= 0x19fu) {
		return thin_block2[codepoint - 0x197u];
	} else if (codepoint == 0x1b7u) {
		return thin_block3[0];
	} else if (codepoint >= 0x1cdu && codepoint <= 0x1e3u) {
		return thin_block4[codepoint - 0x1cdu];
	} else if (codepoint >= 0x1e6u && codepoint <= 0x1e9u) {
		return thin_block5[codepoint - 0x1e6u];
	} else if (codepoint >= 0x1eeu && codepoint <= 0x1f0u) {
		return thin_block6[codepoint - 0x1eeu];
	} else if (codepoint >= 0x1f4u && codepoint <= 0x1f5u) {
		return thin_block7[codepoint - 0x1f4u];
	} else if (codepoint >= 0x1f8u && codepoint <= 0x21bu) {
		return thin_block8[codepoint - 0x1f8u];
	} else if (codepoint >= 0x226u && codepoint <= 0x227u) {
		return thin_block9[codepoint - 0x226u];
	} else if (codepoint >= 0x22au && codepoint <= 0x233u) {
		return thin_block10[codepoint - 0x22au];
	} else if (codepoint >= 0x258u && codepoint <= 0x259u) {
		return thin_block11[codepoint - 0x258u];
	} else if (codepoint == 0x263u) {
		return thin_block12[0];
	} else if (codepoint == 0x292u) {
		return thin_block13[0];
	} else if (codepoint == 0x294u) {
		return thin_block14[0];
	} else if (codepoint == 0x2c6u) {
		return thin_block15[0];
	} else if (codepoint == 0x2c9u) {
		return thin_block16[0];
	} else if (codepoint == 0x2cdu) {
		return thin_block17[0];
	} else if (codepoint == 0x2dcu) {
		return thin_block18[0];
	} else if (codepoint >= 0x300u && codepoint <= 0x30du) {
		return thin_block19[codepoint - 0x300u];
	} else if (codepoint >= 0x30fu && codepoint <= 0x311u) {
		return thin_block20[codepoint - 0x30fu];
	} else if (codepoint >= 0x313u && codepoint <= 0x318u) {
		return thin_block21[codepoint - 0x313u];
	} else if (codepoint >= 0x31bu && codepoint <= 0x31cu) {
		return thin_block22[codepoint - 0x31bu];
	} else if (codepoint >= 0x31fu && codepoint <= 0x329u) {
		return thin_block23[codepoint - 0x31fu];
	} else if (codepoint >= 0x32bu && codepoint <= 0x32cu) {
		return thin_block24[codepoint - 0x32bu];
	} else if (codepoint == 0x32eu) {
		return thin_block25[0];
	} else if (codepoint == 0x330u) {
		return thin_block26[0];
	} else if (codepoint >= 0x332u && codepoint <= 0x333u) {
		return thin_block27[codepoint - 0x332u];
	} else if (codepoint >= 0x335u && codepoint <= 0x33eu) {
		return thin_block28[codepoint - 0x335u];
	} else if (codepoint >= 0x340u && codepoint <= 0x344u) {
		return thin_block29[codepoint - 0x340u];
	} else if (codepoint == 0x34fu) {
		return thin_block30[0];
	} else if (codepoint == 0x362u) {
		return thin_block31[0];
	} else if (codepoint == 0x386u) {
		return thin_block32[0];
	} else if (codepoint >= 0x388u && codepoint <= 0x38au) {
		return thin_block33[codepoint - 0x388u];
	} else if (codepoint == 0x38cu) {
		return thin_block34[0];
	} else if (codepoint >= 0x38eu && codepoint <= 0x38fu) {
		return thin_block35[codepoint - 0x38eu];
	} else if (codepoint >= 0x391u && codepoint <= 0x3a1u) {
		return thin_block36[codepoint - 0x391u];
	} else if (codepoint >= 0x3a3u && codepoint <= 0x3afu) {
		return thin_block37[codepoint - 0x3a3u];
	} else if (codepoint >= 0x3b1u && codepoint <= 0x3ceu) {
		return thin_block38[codepoint - 0x3b1u];
	} else if (codepoint >= 0x400u && codepoint <= 0x401u) {
		return thin_block39[codepoint - 0x400u];
	} else if (codepoint >= 0x405u && codepoint <= 0x409u) {
		return thin_block40[codepoint - 0x405u];
	} else if (codepoint >= 0x410u && codepoint <= 0x451u) {
		return thin_block41[codepoint - 0x410u];
	} else if (codepoint >= 0x455u && codepoint <= 0x458u) {
		return thin_block42[codepoint - 0x455u];
	} else if (codepoint >= 0x4d0u && codepoint <= 0x4d7u) {
		return thin_block43[codepoint - 0x4d0u];
	} else if (codepoint >= 0x4e6u && codepoint <= 0x4e7u) {
		return thin_block44[codepoint - 0x4e6u];
	} else if (codepoint == 0x4f1u) {
		return thin_block45[0];
	} else if (codepoint >= 0x5d0u && codepoint <= 0x5eau) {
		return thin_block46[codepoint - 0x5d0u];
	} else if (codepoint == 0x623u) {
		return thin_block47[0];
	} else if (codepoint >= 0x627u && codepoint <= 0x62du) {
		return thin_block48[codepoint - 0x627u];
	} else if (codepoint >= 0x62fu && codepoint <= 0x635u) {
		return thin_block49[codepoint - 0x62fu];
	} else if (codepoint >= 0x637u && codepoint <= 0x63au) {
		return thin_block50[codepoint - 0x637u];
	} else if (codepoint >= 0x640u && codepoint <= 0x64au) {
		return thin_block51[codepoint - 0x640u];
	} else if (codepoint >= 0x671u && codepoint <= 0x673u) {
		return thin_block52[codepoint - 0x671u];
	} else if (codepoint >= 0x1560u && codepoint <= 0x1563u) {
		return thin_block53[codepoint - 0x1560u];
	} else if (codepoint == 0x156au) {
		return thin_block54[0];
	} else if (codepoint >= 0x156cu && codepoint <= 0x1571u) {
		return thin_block55[codepoint - 0x156cu];
	} else if (codepoint >= 0x15ecu && codepoint <= 0x15edu) {
		return thin_block56[codepoint - 0x15ecu];
	} else if (codepoint == 0x15efu) {
		return thin_block57[0];
	} else if (codepoint >= 0x16a0u && codepoint <= 0x16abu) {
		return thin_block58[codepoint - 0x16a0u];
	} else if (codepoint >= 0x16b1u && codepoint <= 0x16f8u) {
		return thin_block59[codepoint - 0x16b1u];
	} else if (codepoint >= 0x1e00u && codepoint <= 0x1e02u) {
		return thin_block60[codepoint - 0x1e00u];
	} else if (codepoint >= 0x1e04u && codepoint <= 0x1e07u) {
		return thin_block61[codepoint - 0x1e04u];
	} else if (codepoint == 0x1e0au) {
		return thin_block62[0];
	} else if (codepoint >= 0x1e0cu && codepoint <= 0x1e0fu) {
		return thin_block63[codepoint - 0x1e0cu];
	} else if (codepoint >= 0x1e41u && codepoint <= 0x1e4bu) {
		return thin_block64[codepoint - 0x1e41u];
	} else if (codepoint >= 0x1e58u && codepoint <= 0x1e5bu) {
		return thin_block65[codepoint - 0x1e58u];
	} else if (codepoint == 0x1e5eu) {
		return thin_block66[0];
	} else if (codepoint >= 0x1f00u && codepoint <= 0x1f7du) {
		return thin_block67[codepoint - 0x1f00u];
	} else if (codepoint >= 0x1f80u && codepoint <= 0x1fb4u) {
		return thin_block68[codepoint - 0x1f80u];
	} else if (codepoint >= 0x1fb6u && codepoint <= 0x1fbcu) {
		return thin_block69[codepoint - 0x1fb6u];
	} else if (codepoint >= 0x1fc2u && codepoint <= 0x1fc4u) {
		return thin_block70[codepoint - 0x1fc2u];
	} else if (codepoint >= 0x1fc6u && codepoint <= 0x1fc7u) {
		return thin_block71[codepoint - 0x1fc6u];
	} else if (codepoint >= 0x1fcau && codepoint <= 0x1fccu) {
		return thin_block72[codepoint - 0x1fcau];
	} else if (codepoint >= 0x1fd0u && codepoint <= 0x1fd4u) {
		return thin_block73[codepoint - 0x1fd0u];
	} else if (codepoint >= 0x1fd6u && codepoint <= 0x1fdcu) {
		return thin_block74[codepoint - 0x1fd6u];
	} else if (codepoint >= 0x1fe0u && codepoint <= 0x1fe3u) {
		return thin_block75[codepoint - 0x1fe0u];
	} else if (codepoint >= 0x1fe6u && codepoint <= 0x1febu) {
		return thin_block76[codepoint - 0x1fe6u];
	} else if (codepoint >= 0x1ff2u && codepoint <= 0x1ff4u) {
		return thin_block77[codepoint - 0x1ff2u];
	} else if (codepoint >= 0x1ff6u && codepoint <= 0x1ff7u) {
		return thin_block78[codepoint - 0x1ff6u];
	} else if (codepoint >= 0x1ffau && codepoint <= 0x1ffcu) {
		return thin_block79[codepoint - 0x1ffau];
	} else if (codepoint >= 0x2010u && codepoint <= 0x2019u) {
		return thin_block80[codepoint - 0x2010u];
	} else if (codepoint >= 0x201cu && codepoint <= 0x201du) {
		return thin_block81[codepoint - 0x201cu];
	} else if (codepoint >= 0x2020u && codepoint <= 0x2022u) {
		return thin_block82[codepoint - 0x2020u];
	} else if (codepoint >= 0x2024u && codepoint <= 0x2026u) {
		return thin_block83[codepoint - 0x2024u];
	} else if (codepoint >= 0x2030u && codepoint <= 0x2037u) {
		return thin_block84[codepoint - 0x2030u];
	} else if (codepoint >= 0x203cu && codepoint <= 0x203du) {
		return thin_block85[codepoint - 0x203cu];
	} else if (codepoint == 0x2044u) {
		return thin_block86[0];
	} else if (codepoint >= 0x2047u && codepoint <= 0x2049u) {
		return thin_block87[codepoint - 0x2047u];
	} else if (codepoint >= 0x2070u && codepoint <= 0x2071u) {
		return thin_block88[codepoint - 0x2070u];
	} else if (codepoint >= 0x2074u && codepoint <= 0x207cu) {
		return thin_block89[codepoint - 0x2074u];
	} else if (codepoint == 0x207fu) {
		return thin_block90[0];
	} else if (codepoint == 0x20a7u) {
		return thin_block91[0];
	} else if (codepoint == 0x20acu) {
		return thin_block92[0];
	} else if (codepoint == 0x2117u) {
		return thin_block93[0];
	} else if (codepoint == 0x2120u) {
		return thin_block94[0];
	} else if (codepoint == 0x2122u) {
		return thin_block95[0];
	} else if (codepoint >= 0x2150u && codepoint <= 0x215fu) {
		return thin_block96[codepoint - 0x2150u];
	} else if (codepoint == 0x2189u) {
		return thin_block97[0];
	} else if (codepoint >= 0x2190u && codepoint <= 0x2199u) {
		return thin_block98[codepoint - 0x2190u];
	} else if (codepoint == 0x21a5u) {
		return thin_block99[0];
	} else if (codepoint == 0x21a8u) {
		return thin_block100[0];
	} else if (codepoint == 0x21b2u) {
		return thin_block101[0];
	} else if (codepoint >= 0x21bcu && codepoint <= 0x21c3u) {
		return thin_block102[codepoint - 0x21bcu];
	} else if (codepoint >= 0x21e6u && codepoint <= 0x21e9u) {
		return thin_block103[codepoint - 0x21e6u];
	} else if (codepoint == 0x2205u) {
		return thin_block104[0];
	} else if (codepoint >= 0x2212u && codepoint <= 0x2215u) {
		return thin_block105[codepoint - 0x2212u];
	} else if (codepoint == 0x2218u) {
		return thin_block106[0];
	} else if (codepoint >= 0x221au && codepoint <= 0x221cu) {
		return thin_block107[codepoint - 0x221au];
	} else if (codepoint >= 0x221eu && codepoint <= 0x221fu) {
		return thin_block108[codepoint - 0x221eu];
	} else if (codepoint == 0x2229u) {
		return thin_block109[0];
	} else if (codepoint == 0x2248u) {
		return thin_block110[0];
	} else if (codepoint == 0x2261u) {
		return thin_block111[0];
	} else if (codepoint >= 0x2264u && codepoint <= 0x2265u) {
		return thin_block112[codepoint - 0x2264u];
	} else if (codepoint == 0x22c5u) {
		return thin_block113[0];
	} else if (codepoint == 0x2301u) {
		return thin_block114[0];
	} else if (codepoint == 0x2318u) {
		return thin_block115[0];
	} else if (codepoint >= 0x231au && codepoint <= 0x231bu) {
		return thin_block116[codepoint - 0x231au];
	} else if (codepoint >= 0x2320u && codepoint <= 0x2321u) {
		return thin_block117[codepoint - 0x2320u];
	} else if (codepoint >= 0x239bu && codepoint <= 0x23afu) {
		return thin_block118[codepoint - 0x239bu];
	} else if (codepoint >= 0x23b2u && codepoint <= 0x23b3u) {
		return thin_block119[codepoint - 0x23b2u];
	} else if (codepoint >= 0x23b7u && codepoint <= 0x23bdu) {
		return thin_block120[codepoint - 0x23b7u];
	} else if (codepoint == 0x23d0u) {
		return thin_block121[0];
	} else if (codepoint == 0x23dau) {
		return thin_block122[0];
	} else if (codepoint >= 0x23e9u && codepoint <= 0x23ecu) {
		return thin_block123[codepoint - 0x23e9u];
	} else if (codepoint >= 0x2500u && codepoint <= 0x25ffu) {
		return thin_block124[codepoint - 0x2500u];
	} else if (codepoint == 0x2602u) {
		return thin_block125[0];
	} else if (codepoint >= 0x2605u && codepoint <= 0x2606u) {
		return thin_block126[codepoint - 0x2605u];
	} else if (codepoint == 0x2609u) {
		return thin_block127[0];
	} else if (codepoint == 0x260eu) {
		return thin_block128[0];
	} else if (codepoint >= 0x2610u && codepoint <= 0x2612u) {
		return thin_block129[codepoint - 0x2610u];
	} else if (codepoint == 0x2615u) {
		return thin_block130[0];
	} else if (codepoint == 0x2625u) {
		return thin_block131[0];
	} else if (codepoint == 0x2628u) {
		return thin_block132[0];
	} else if (codepoint >= 0x262fu && codepoint <= 0x2637u) {
		return thin_block133[codepoint - 0x262fu];
	} else if (codepoint >= 0x2639u && codepoint <= 0x263bu) {
		return thin_block134[codepoint - 0x2639u];
	} else if (codepoint == 0x2640u) {
		return thin_block135[0];
	} else if (codepoint == 0x2642u) {
		return thin_block136[0];
	} else if (codepoint >= 0x2660u && codepoint <= 0x2667u) {
		return thin_block137[codepoint - 0x2660u];
	} else if (codepoint >= 0x2669u && codepoint <= 0x266cu) {
		return thin_block138[codepoint - 0x2669u];
	} else if (codepoint == 0x2683u) {
		return thin_block139[0];
	} else if (codepoint >= 0x268au && codepoint <= 0x268fu) {
		return thin_block140[codepoint - 0x268au];
	} else if (codepoint >= 0x269eu && codepoint <= 0x269fu) {
		return thin_block141[codepoint - 0x269eu];
	} else if (codepoint >= 0x26aau && codepoint <= 0x26acu) {
		return thin_block142[codepoint - 0x26aau];
	} else if (codepoint == 0x26c8u) {
		return thin_block143[0];
	} else if (codepoint == 0x26f5u) {
		return thin_block144[0];
	} else if (codepoint == 0x2708u) {
		return thin_block145[0];
	} else if (codepoint == 0x2713u) {
		return thin_block146[0];
	} else if (codepoint >= 0x2734u && codepoint <= 0x2735u) {
		return thin_block147[codepoint - 0x2734u];
	} else if (codepoint == 0x2764u) {
		return thin_block148[0];
	} else if (codepoint >= 0x2800u && codepoint <= 0x28ffu) {
		return thin_block149[codepoint - 0x2800u];
	} else if (codepoint == 0x29c9u) {
		return thin_block150[0];
	} else if (codepoint >= 0x2b12u && codepoint <= 0x2b19u) {
		return thin_block151[codepoint - 0x2b12u];
	} else if (codepoint == 0x2b1du) {
		return thin_block152[0];
	} else if (codepoint >= 0x2b24u && codepoint <= 0x2b2fu) {
		return thin_block153[codepoint - 0x2b24u];
	} else if (codepoint == 0x2b55u) {
		return thin_block154[0];
	} else if (codepoint == 0x2b58u) {
		return thin_block155[0];
	} else if (codepoint == 0x2b73u) {
		return thin_block156[0];
	} else if (codepoint >= 0x2b9cu && codepoint <= 0x2b9fu) {
		return thin_block157[codepoint - 0x2b9cu];
	} else if (codepoint == 0x2bbau) {
		return thin_block158[0];
	} else if (codepoint == 0x2bc0u) {
		return thin_block159[0];
	} else if (codepoint >= 0x2bc5u && codepoint <= 0x2bc8u) {
		return thin_block160[codepoint - 0x2bc5u];
	} else if (codepoint >= 0x2bcau && codepoint <= 0x2bcbu) {
		return thin_block161[codepoint - 0x2bcau];
	} else if (codepoint == 0x2e2eu) {
		return thin_block162[0];
	} else if (codepoint >= 0x4dc0u && codepoint <= 0x4dffu) {
		return thin_block163[codepoint - 0x4dc0u];
	} else if (codepoint >= 0xe080u && codepoint <= 0xe1aau) {
		return thin_block164[codepoint - 0xe080u];
	} else if (codepoint >= 0xe800u && codepoint <= 0xe87du) {
		return thin_block165[codepoint - 0xe800u];
	} else if (codepoint >= 0xec00u && codepoint <= 0xec07u) {
		return thin_block166[codepoint - 0xec00u];
	} else if (codepoint >= 0xec0au && codepoint <= 0xec7du) {
		return thin_block167[codepoint - 0xec0au];
	} else if (codepoint >= 0xec7fu && codepoint <= 0xecdeu) {
		return thin_block168[codepoint - 0xec7fu];
	} else if (codepoint >= 0xfe81u && codepoint <= 0xfe82u) {
		return thin_block169[codepoint - 0xfe81u];
	} else if (codepoint == 0xfe84u) {
		return thin_block170[0];
	} else if (codepoint == 0xfe87u) {
		return thin_block171[0];
	} else if (codepoint >= 0xfe8fu && codepoint <= 0xfe9cu) {
		return thin_block172[codepoint - 0xfe8fu];
	} else if (codepoint >= 0xfe9eu && codepoint <= 0xfea4u) {
		return thin_block173[codepoint - 0xfe9eu];
	} else if (codepoint >= 0xfea9u && codepoint <= 0xfeadu) {
		return thin_block174[codepoint - 0xfea9u];
	} else if (codepoint == 0xfeafu) {
		return thin_block175[0];
	} else if (codepoint >= 0xfeb1u && codepoint <= 0xfeb7u) {
		return thin_block176[codepoint - 0xfeb1u];
	} else if (codepoint >= 0xfeb9u && codepoint <= 0xfebcu) {
		return thin_block177[codepoint - 0xfeb9u];
	} else if (codepoint >= 0xfec1u && codepoint <= 0xfec6u) {
		return thin_block178[codepoint - 0xfec1u];
	} else if (codepoint >= 0xfec9u && codepoint <= 0xfeddu) {
		return thin_block179[codepoint - 0xfec9u];
	} else if (codepoint >= 0xfedfu && codepoint <= 0xfee3u) {
		return thin_block180[codepoint - 0xfedfu];
	} else if (codepoint >= 0xfee5u && codepoint <= 0xfee9u) {
		return thin_block181[codepoint - 0xfee5u];
	} else if (codepoint >= 0xfeebu && codepoint <= 0xfef4u) {
		return thin_block182[codepoint - 0xfeebu];
	} else if (codepoint >= 0xff01u && codepoint <= 0xff36u) {
		return thin_block183[codepoint - 0xff01u];
	} else if (codepoint >= 0xff38u && codepoint <= 0xff5du) {
		return thin_block184[codepoint - 0xff38u];
	} else if (codepoint >= 0xff5fu && codepoint <= 0xff9fu) {
		return thin_block185[codepoint - 0xff5fu];
	} else if (codepoint >= 0x1d300u && codepoint <= 0x1d356u) {
		return thin_block186[codepoint - 0x1d300u];
	} else if (codepoint >= 0x1f332u && codepoint <= 0x1f333u) {
		return thin_block187[codepoint - 0x1f332u];
	} else if (codepoint >= 0x1f344u && codepoint <= 0x1f345u) {
		return thin_block188[codepoint - 0x1f344u];
	} else if (codepoint >= 0x1f34eu && codepoint <= 0x1f34fu) {
		return thin_block189[codepoint - 0x1f34eu];
	} else if (codepoint == 0x1f37au) {
		return thin_block190[0];
	} else if (codepoint == 0x1f37eu) {
		return thin_block191[0];
	} else if (codepoint == 0x1f3ceu) {
		return thin_block192[0];
	} else if (codepoint == 0x1f3d0u) {
		return thin_block193[0];
	} else if (codepoint == 0x1f3f0u) {
		return thin_block194[0];
	} else if (codepoint >= 0x1f407u && codepoint <= 0x1f408u) {
		return thin_block195[codepoint - 0x1f407u];
	} else if (codepoint == 0x1f40du) {
		return thin_block196[0];
	} else if (codepoint == 0x1f419u) {
		return thin_block197[0];
	} else if (codepoint == 0x1f41fu) {
		return thin_block198[0];
	} else if (codepoint == 0x1f42au) {
		return thin_block199[0];
	} else if (codepoint == 0x1f47bu) {
		return thin_block200[0];
	} else if (codepoint == 0x1f47eu) {
		return thin_block201[0];
	} else if (codepoint == 0x1f480u) {
		return thin_block202[0];
	} else if (codepoint >= 0x1f48du && codepoint <= 0x1f48eu) {
		return thin_block203[codepoint - 0x1f48du];
	} else if (codepoint == 0x1f4a7u) {
		return thin_block204[0];
	} else if (codepoint == 0x1f4adu) {
		return thin_block205[0];
	} else if (codepoint == 0x1f4b0u) {
		return thin_block206[0];
	} else if (codepoint == 0x1f4beu) {
		return thin_block207[0];
	} else if (codepoint == 0x1f4c1u) {
		return thin_block208[0];
	} else if (codepoint == 0x1f4c4u) {
		return thin_block209[0];
	} else if (codepoint == 0x1f4dcu) {
		return thin_block210[0];
	} else if (codepoint == 0x1f4fbu) {
		return thin_block211[0];
	} else if (codepoint == 0x1f514u) {
		return thin_block212[0];
	} else if (codepoint == 0x1f52bu) {
		return thin_block213[0];
	} else if (codepoint == 0x1f52eu) {
		return thin_block214[0];
	} else if (codepoint == 0x1f574u) {
		return thin_block215[0];
	} else if (codepoint == 0x1f577u) {
		return thin_block216[0];
	} else if (codepoint == 0x1f5a8u) {
		return thin_block217[0];
	} else if (codepoint == 0x1f5d1u) {
		return thin_block218[0];
	} else if (codepoint == 0x1f68du) {
		return thin_block219[0];
	} else if (codepoint == 0x1f6d6u) {
		return thin_block220[0];
	} else if (codepoint == 0x1f6f8u) {
		return thin_block221[0];
	} else if (codepoint == 0x1f860u) {
		return thin_block222[0];
	} else if (codepoint == 0x1f986u) {
		return thin_block223[0];
	} else if (codepoint == 0x1f98bu) {
		return thin_block224[0];
	} else if (codepoint == 0x1f9f2u) {
		return thin_block225[0];
	} else if (codepoint == 0x1fa90u) {
		return thin_block226[0];
	} else if (codepoint == 0x1fa93u) {
		return thin_block227[0];
	} else if (codepoint == 0x1fa99u) {
		return thin_block228[0];
	} else if (codepoint == 0x1fa9cu) {
		return thin_block229[0];
	} else if (codepoint >= 0x1faa6u && codepoint <= 0x1faa8u) {
		return thin_block230[codepoint - 0x1faa6u];
	} else if (codepoint >= 0x1fb00u && codepoint <= 0x1fbcau) {
		return thin_block231[codepoint - 0x1fb00u];
	} else if (codepoint >= 0x1fbf0u && codepoint <= 0x1fbf9u) {
		return thin_block232[codepoint - 0x1fbf0u];
	} else {
		WARN("Codepoint U+%04X (%s) is not covered by this font", codepoint, u8_cp_to_utf8(codepoint));
		return thin_block1[0];
	}
}

static const unsigned char*
    fantasy_get_bitmap(uint32_t codepoint)
{
	if (codepoint <= 0x195u) {
		return fantasy_block1[codepoint];
	} else if (codepoint >= 0x197u && codepoint <= 0x19fu) {
		return fantasy_block2[codepoint - 0x197u];
	} else if (codepoint == 0x1b7u) {
		return fantasy_block3[0];
	} else if (codepoint >= 0x1cdu && codepoint <= 0x1e3u) {
		return fantasy_block4[codepoint - 0x1cdu];
	} else if (codepoint >= 0x1e6u && codepoint <= 0x1e9u) {
		return fantasy_block5[codepoint - 0x1e6u];
	} else if (codepoint >= 0x1eeu && codepoint <= 0x1f0u) {
		return fantasy_block6[codepoint - 0x1eeu];
	} else if (codepoint >= 0x1f4u && codepoint <= 0x1f5u) {
		return fantasy_block7[codepoint - 0x1f4u];
	} else if (codepoint >= 0x1f8u && codepoint <= 0x21bu) {
		return fantasy_block8[codepoint - 0x1f8u];
	} else if (codepoint >= 0x226u && codepoint <= 0x227u) {
		return fantasy_block9[codepoint - 0x226u];
	} else if (codepoint >= 0x22au && codepoint <= 0x233u) {
		return fantasy_block10[codepoint - 0x22au];
	} else if (codepoint >= 0x258u && codepoint <= 0x259u) {
		return fantasy_block11[codepoint - 0x258u];
	} else if (codepoint == 0x263u) {
		return fantasy_block12[0];
	} else if (codepoint == 0x292u) {
		return fantasy_block13[0];
	} else if (codepoint == 0x294u) {
		return fantasy_block14[0];
	} else if (codepoint == 0x2c6u) {
		return fantasy_block15[0];
	} else if (codepoint == 0x2c9u) {
		return fantasy_block16[0];
	} else if (codepoint == 0x2cdu) {
		return fantasy_block17[0];
	} else if (codepoint == 0x2dcu) {
		return fantasy_block18[0];
	} else if (codepoint >= 0x300u && codepoint <= 0x30du) {
		return fantasy_block19[codepoint - 0x300u];
	} else if (codepoint >= 0x30fu && codepoint <= 0x311u) {
		return fantasy_block20[codepoint - 0x30fu];
	} else if (codepoint >= 0x313u && codepoint <= 0x318u) {
		return fantasy_block21[codepoint - 0x313u];
	} else if (codepoint >= 0x31bu && codepoint <= 0x31cu) {
		return fantasy_block22[codepoint - 0x31bu];
	} else if (codepoint >= 0x31fu && codepoint <= 0x329u) {
		return fantasy_block23[codepoint - 0x31fu];
	} else if (codepoint >= 0x32bu && codepoint <= 0x32cu) {
		return fantasy_block24[codepoint - 0x32bu];
	} else if (codepoint == 0x32eu) {
		return fantasy_block25[0];
	} else if (codepoint == 0x330u) {
		return fantasy_block26[0];
	} else if (codepoint >= 0x332u && codepoint <= 0x333u) {
		return fantasy_block27[codepoint - 0x332u];
	} else if (codepoint >= 0x335u && codepoint <= 0x33eu) {
		return fantasy_block28[codepoint - 0x335u];
	} else if (codepoint >= 0x340u && codepoint <= 0x344u) {
		return fantasy_block29[codepoint - 0x340u];
	} else if (codepoint == 0x34fu) {
		return fantasy_block30[0];
	} else if (codepoint == 0x362u) {
		return fantasy_block31[0];
	} else if (codepoint == 0x386u) {
		return fantasy_block32[0];
	} else if (codepoint >= 0x388u && codepoint <= 0x38au) {
		return fantasy_block33[codepoint - 0x388u];
	} else if (codepoint == 0x38cu) {
		return fantasy_block34[0];
	} else if (codepoint >= 0x38eu && codepoint <= 0x38fu) {
		return fantasy_block35[codepoint - 0x38eu];
	} else if (codepoint >= 0x391u && codepoint <= 0x3a1u) {
		return fantasy_block36[codepoint - 0x391u];
	} else if (codepoint >= 0x3a3u && codepoint <= 0x3afu) {
		return fantasy_block37[codepoint - 0x3a3u];
	} else if (codepoint >= 0x3b1u && codepoint <= 0x3ceu) {
		return fantasy_block38[codepoint - 0x3b1u];
	} else if (codepoint >= 0x400u && codepoint <= 0x401u) {
		return fantasy_block39[codepoint - 0x400u];
	} else if (codepoint >= 0x405u && codepoint <= 0x409u) {
		return fantasy_block40[codepoint - 0x405u];
	} else if (codepoint >= 0x410u && codepoint <= 0x451u) {
		return fantasy_block41[codepoint - 0x410u];
	} else if (codepoint >= 0x455u && codepoint <= 0x458u) {
		return fantasy_block42[codepoint - 0x455u];
	} else if (codepoint >= 0x4d0u && codepoint <= 0x4d7u) {
		return fantasy_block43[codepoint - 0x4d0u];
	} else if (codepoint >= 0x4e6u && codepoint <= 0x4e7u) {
		return fantasy_block44[codepoint - 0x4e6u];
	} else if (codepoint == 0x4f1u) {
		return fantasy_block45[0];
	} else if (codepoint >= 0x5d0u && codepoint <= 0x5eau) {
		return fantasy_block46[codepoint - 0x5d0u];
	} else if (codepoint == 0x623u) {
		return fantasy_block47[0];
	} else if (codepoint >= 0x627u && codepoint <= 0x62du) {
		return fantasy_block48[codepoint - 0x627u];
	} else if (codepoint >= 0x62fu && codepoint <= 0x635u) {
		return fantasy_block49[codepoint - 0x62fu];
	} else if (codepoint >= 0x637u && codepoint <= 0x63au) {
		return fantasy_block50[codepoint - 0x637u];
	} else if (codepoint >= 0x640u && codepoint <= 0x64au) {
		return fantasy_block51[codepoint - 0x640u];
	} else if (codepoint >= 0x671u && codepoint <= 0x673u) {
		return fantasy_block52[codepoint - 0x671u];
	} else if (codepoint >= 0x1560u && codepoint <= 0x1563u) {
		return fantasy_block53[codepoint - 0x1560u];
	} else if (codepoint == 0x156au) {
		return fantasy_block54[0];
	} else if (codepoint >= 0x156cu && codepoint <= 0x1571u) {
		return fantasy_block55[codepoint - 0x156cu];
	} else if (codepoint >= 0x15ecu && codepoint <= 0x15edu) {
		return fantasy_block56[codepoint - 0x15ecu];
	} else if (codepoint == 0x15efu) {
		return fantasy_block57[0];
	} else if (codepoint >= 0x16a0u && codepoint <= 0x16abu) {
		return fantasy_block58[codepoint - 0x16a0u];
	} else if (codepoint >= 0x16b1u && codepoint <= 0x16f8u) {
		return fantasy_block59[codepoint - 0x16b1u];
	} else if (codepoint >= 0x1e00u && codepoint <= 0x1e02u) {
		return fantasy_block60[codepoint - 0x1e00u];
	} else if (codepoint >= 0x1e04u && codepoint <= 0x1e07u) {
		return fantasy_block61[codepoint - 0x1e04u];
	} else if (codepoint == 0x1e0au) {
		return fantasy_block62[0];
	} else if (codepoint >= 0x1e0cu && codepoint <= 0x1e0fu) {
		return fantasy_block63[codepoint - 0x1e0cu];
	} else if (codepoint >= 0x1e41u && codepoint <= 0x1e4bu) {
		return fantasy_block64[codepoint - 0x1e41u];
	} else if (codepoint >= 0x1e58u && codepoint <= 0x1e5bu) {
		return fantasy_block65[codepoint - 0x1e58u];
	} else if (codepoint == 0x1e5eu) {
		return fantasy_block66[0];
	} else if (codepoint >= 0x1f00u && codepoint <= 0x1f7du) {
		return fantasy_block67[codepoint - 0x1f00u];
	} else if (codepoint >= 0x1f80u && codepoint <= 0x1fb4u) {
		return fantasy_block68[codepoint - 0x1f80u];
	} else if (codepoint >= 0x1fb6u && codepoint <= 0x1fbcu) {
		return fantasy_block69[codepoint - 0x1fb6u];
	} else if (codepoint >= 0x1fc2u && codepoint <= 0x1fc4u) {
		return fantasy_block70[codepoint - 0x1fc2u];
	} else if (codepoint >= 0x1fc6u && codepoint <= 0x1fc7u) {
		return fantasy_block71[codepoint - 0x1fc6u];
	} else if (codepoint >= 0x1fcau && codepoint <= 0x1fccu) {
		return fantasy_block72[codepoint - 0x1fcau];
	} else if (codepoint >= 0x1fd0u && codepoint <= 0x1fd4u) {
		return fantasy_block73[codepoint - 0x1fd0u];
	} else if (codepoint >= 0x1fd6u && codepoint <= 0x1fdcu) {
		return fantasy_block74[codepoint - 0x1fd6u];
	} else if (codepoint >= 0x1fe0u && codepoint <= 0x1fe3u) {
		return fantasy_block75[codepoint - 0x1fe0u];
	} else if (codepoint >= 0x1fe6u && codepoint <= 0x1febu) {
		return fantasy_block76[codepoint - 0x1fe6u];
	} else if (codepoint >= 0x1ff2u && codepoint <= 0x1ff4u) {
		return fantasy_block77[codepoint - 0x1ff2u];
	} else if (codepoint >= 0x1ff6u && codepoint <= 0x1ff7u) {
		return fantasy_block78[codepoint - 0x1ff6u];
	} else if (codepoint >= 0x1ffau && codepoint <= 0x1ffcu) {
		return fantasy_block79[codepoint - 0x1ffau];
	} else if (codepoint >= 0x2010u && codepoint <= 0x2019u) {
		return fantasy_block80[codepoint - 0x2010u];
	} else if (codepoint >= 0x201cu && codepoint <= 0x201du) {
		return fantasy_block81[codepoint - 0x201cu];
	} else if (codepoint >= 0x2020u && codepoint <= 0x2022u) {
		return fantasy_block82[codepoint - 0x2020u];
	} else if (codepoint >= 0x2024u && codepoint <= 0x2026u) {
		return fantasy_block83[codepoint - 0x2024u];
	} else if (codepoint >= 0x2030u && codepoint <= 0x2037u) {
		return fantasy_block84[codepoint - 0x2030u];
	} else if (codepoint >= 0x203cu && codepoint <= 0x203du) {
		return fantasy_block85[codepoint - 0x203cu];
	} else if (codepoint == 0x2044u) {
		return fantasy_block86[0];
	} else if (codepoint >= 0x2047u && codepoint <= 0x2049u) {
		return fantasy_block87[codepoint - 0x2047u];
	} else if (codepoint >= 0x2070u && codepoint <= 0x2071u) {
		return fantasy_block88[codepoint - 0x2070u];
	} else if (codepoint >= 0x2074u && codepoint <= 0x207cu) {
		return fantasy_block89[codepoint - 0x2074u];
	} else if (codepoint == 0x207fu) {
		return fantasy_block90[0];
	} else if (codepoint == 0x20a7u) {
		return fantasy_block91[0];
	} else if (codepoint == 0x20acu) {
		return fantasy_block92[0];
	} else if (codepoint == 0x2117u) {
		return fantasy_block93[0];
	} else if (codepoint == 0x2120u) {
		return fantasy_block94[0];
	} else if (codepoint == 0x2122u) {
		return fantasy_block95[0];
	} else if (codepoint >= 0x2150u && codepoint <= 0x215fu) {
		return fantasy_block96[codepoint - 0x2150u];
	} else if (codepoint == 0x2189u) {
		return fantasy_block97[0];
	} else if (codepoint >= 0x2190u && codepoint <= 0x2199u) {
		return fantasy_block98[codepoint - 0x2190u];
	} else if (codepoint == 0x21a5u) {
		return fantasy_block99[0];
	} else if (codepoint == 0x21a8u) {
		return fantasy_block100[0];
	} else if (codepoint == 0x21b2u) {
		return fantasy_block101[0];
	} else if (codepoint >= 0x21bcu && codepoint <= 0x21c3u) {
		return fantasy_block102[codepoint - 0x21bcu];
	} else if (codepoint >= 0x21e6u && codepoint <= 0x21e9u) {
		return fantasy_block103[codepoint - 0x21e6u];
	} else if (codepoint == 0x2205u) {
		return fantasy_block104[0];
	} else if (codepoint >= 0x2212u && codepoint <= 0x2215u) {
		return fantasy_block105[codepoint - 0x2212u];
	} else if (codepoint == 0x2218u) {
		return fantasy_block106[0];
	} else if (codepoint >= 0x221au && codepoint <= 0x221cu) {
		return fantasy_block107[codepoint - 0x221au];
	} else if (codepoint >= 0x221eu && codepoint <= 0x221fu) {
		return fantasy_block108[codepoint - 0x221eu];
	} else if (codepoint == 0x2229u) {
		return fantasy_block109[0];
	} else if (codepoint == 0x2248u) {
		return fantasy_block110[0];
	} else if (codepoint == 0x2261u) {
		return fantasy_block111[0];
	} else if (codepoint >= 0x2264u && codepoint <= 0x2265u) {
		return fantasy_block112[codepoint - 0x2264u];
	} else if (codepoint == 0x22c5u) {
		return fantasy_block113[0];
	} else if (codepoint == 0x2301u) {
		return fantasy_block114[0];
	} else if (codepoint == 0x2318u) {
		return fantasy_block115[0];
	} else if (codepoint >= 0x231au && codepoint <= 0x231bu) {
		return fantasy_block116[codepoint - 0x231au];
	} else if (codepoint >= 0x2320u && codepoint <= 0x2321u) {
		return fantasy_block117[codepoint - 0x2320u];
	} else if (codepoint >= 0x239bu && codepoint <= 0x23afu) {
		return fantasy_block118[codepoint - 0x239bu];
	} else if (codepoint >= 0x23b2u && codepoint <= 0x23b3u) {
		return fantasy_block119[codepoint - 0x23b2u];
	} else if (codepoint >= 0x23b7u && codepoint <= 0x23bdu) {
		return fantasy_block120[codepoint - 0x23b7u];
	} else if (codepoint == 0x23d0u) {
		return fantasy_block121[0];
	} else if (codepoint == 0x23dau) {
		return fantasy_block122[0];
	} else if (codepoint >= 0x23e9u && codepoint <= 0x23ecu) {
		return fantasy_block123[codepoint - 0x23e9u];
	} else if (codepoint >= 0x2500u && codepoint <= 0x25ffu) {
		return fantasy_block124[codepoint - 0x2500u];
	} else if (codepoint == 0x2602u) {
		return fantasy_block125[0];
	} else if (codepoint >= 0x2605u && codepoint <= 0x2606u) {
		return fantasy_block126[codepoint - 0x2605u];
	} else if (codepoint == 0x2609u) {
		return fantasy_block127[0];
	} else if (codepoint == 0x260eu) {
		return fantasy_block128[0];
	} else if (codepoint >= 0x2610u && codepoint <= 0x2612u) {
		return fantasy_block129[codepoint - 0x2610u];
	} else if (codepoint == 0x2615u) {
		return fantasy_block130[0];
	} else if (codepoint == 0x2625u) {
		return fantasy_block131[0];
	} else if (codepoint == 0x2628u) {
		return fantasy_block132[0];
	} else if (codepoint >= 0x262fu && codepoint <= 0x2637u) {
		return fantasy_block133[codepoint - 0x262fu];
	} else if (codepoint >= 0x2639u && codepoint <= 0x263bu) {
		return fantasy_block134[codepoint - 0x2639u];
	} else if (codepoint == 0x2640u) {
		return fantasy_block135[0];
	} else if (codepoint == 0x2642u) {
		return fantasy_block136[0];
	} else if (codepoint >= 0x2660u && codepoint <= 0x2667u) {
		return fantasy_block137[codepoint - 0x2660u];
	} else if (codepoint >= 0x2669u && codepoint <= 0x266cu) {
		return fantasy_block138[codepoint - 0x2669u];
	} else if (codepoint == 0x2683u) {
		return fantasy_block139[0];
	} else if (codepoint >= 0x268au && codepoint <= 0x268fu) {
		return fantasy_block140[codepoint - 0x268au];
	} else if (codepoint >= 0x269eu && codepoint <= 0x269fu) {
		return fantasy_block141[codepoint - 0x269eu];
	} else if (codepoint >= 0x26aau && codepoint <= 0x26acu) {
		return fantasy_block142[codepoint - 0x26aau];
	} else if (codepoint == 0x26c8u) {
		return fantasy_block143[0];
	} else if (codepoint == 0x26f5u) {
		return fantasy_block144[0];
	} else if (codepoint == 0x2708u) {
		return fantasy_block145[0];
	} else if (codepoint == 0x2713u) {
		return fantasy_block146[0];
	} else if (codepoint >= 0x2734u && codepoint <= 0x2735u) {
		return fantasy_block147[codepoint - 0x2734u];
	} else if (codepoint == 0x2764u) {
		return fantasy_block148[0];
	} else if (codepoint >= 0x2800u && codepoint <= 0x28ffu) {
		return fantasy_block149[codepoint - 0x2800u];
	} else if (codepoint == 0x29c9u) {
		return fantasy_block150[0];
	} else if (codepoint >= 0x2b12u && codepoint <= 0x2b19u) {
		return fantasy_block151[codepoint - 0x2b12u];
	} else if (codepoint == 0x2b1du) {
		return fantasy_block152[0];
	} else if (codepoint >= 0x2b24u && codepoint <= 0x2b2fu) {
		return fantasy_block153[codepoint - 0x2b24u];
	} else if (codepoint == 0x2b55u) {
		return fantasy_block154[0];
	} else if (codepoint == 0x2b58u) {
		return fantasy_block155[0];
	} else if (codepoint == 0x2b73u) {
		return fantasy_block156[0];
	} else if (codepoint >= 0x2b9cu && codepoint <= 0x2b9fu) {
		return fantasy_block157[codepoint - 0x2b9cu];
	} else if (codepoint == 0x2bbau) {
		return fantasy_block158[0];
	} else if (codepoint == 0x2bc0u) {
		return fantasy_block159[0];
	} else if (codepoint >= 0x2bc5u && codepoint <= 0x2bc8u) {
		return fantasy_block160[codepoint - 0x2bc5u];
	} else if (codepoint >= 0x2bcau && codepoint <= 0x2bcbu) {
		return fantasy_block161[codepoint - 0x2bcau];
	} else if (codepoint == 0x2e2eu) {
		return fantasy_block162[0];
	} else if (codepoint >= 0x4dc0u && codepoint <= 0x4dffu) {
		return fantasy_block163[codepoint - 0x4dc0u];
	} else if (codepoint >= 0xe080u && codepoint <= 0xe1aau) {
		return fantasy_block164[codepoint - 0xe080u];
	} else if (codepoint >= 0xe800u && codepoint <= 0xe87du) {
		return fantasy_block165[codepoint - 0xe800u];
	} else if (codepoint >= 0xec00u && codepoint <= 0xec07u) {
		return fantasy_block166[codepoint - 0xec00u];
	} else if (codepoint >= 0xec0au && codepoint <= 0xec7du) {
		return fantasy_block167[codepoint - 0xec0au];
	} else if (codepoint >= 0xec7fu && codepoint <= 0xecdeu) {
		return fantasy_block168[codepoint - 0xec7fu];
	} else if (codepoint >= 0xfe81u && codepoint <= 0xfe82u) {
		return fantasy_block169[codepoint - 0xfe81u];
	} else if (codepoint == 0xfe84u) {
		return fantasy_block170[0];
	} else if (codepoint == 0xfe87u) {
		return fantasy_block171[0];
	} else if (codepoint >= 0xfe8fu && codepoint <= 0xfe9cu) {
		return fantasy_block172[codepoint - 0xfe8fu];
	} else if (codepoint >= 0xfe9eu && codepoint <= 0xfea4u) {
		return fantasy_block173[codepoint - 0xfe9eu];
	} else if (codepoint >= 0xfea9u && codepoint <= 0xfeadu) {
		return fantasy_block174[codepoint - 0xfea9u];
	} else if (codepoint == 0xfeafu) {
		return fantasy_block175[0];
	} else if (codepoint >= 0xfeb1u && codepoint <= 0xfeb7u) {
		return fantasy_block176[codepoint - 0xfeb1u];
	} else if (codepoint >= 0xfeb9u && codepoint <= 0xfebcu) {
		return fantasy_block177[codepoint - 0xfeb9u];
	} else if (codepoint >= 0xfec1u && codepoint <= 0xfec6u) {
		return fantasy_block178[codepoint - 0xfec1u];
	} else if (codepoint >= 0xfec9u && codepoint <= 0xfeddu) {
		return fantasy_block179[codepoint - 0xfec9u];
	} else if (codepoint >= 0xfedfu && codepoint <= 0xfee3u) {
		return fantasy_block180[codepoint - 0xfedfu];
	} else if (codepoint >= 0xfee5u && codepoint <= 0xfee9u) {
		return fantasy_block181[codepoint - 0xfee5u];
	} else if (codepoint >= 0xfeebu && codepoint <= 0xfef4u) {
		return fantasy_block182[codepoint - 0xfeebu];
	} else if (codepoint >= 0xff01u && codepoint <= 0xff36u) {
		return fantasy_block183[codepoint - 0xff01u];
	} else if (codepoint >= 0xff38u && codepoint <= 0xff5du) {
		return fantasy_block184[codepoint - 0xff38u];
	} else if (codepoint >= 0xff5fu && codepoint <= 0xff9fu) {
		return fantasy_block185[codepoint - 0xff5fu];
	} else if (codepoint >= 0x1d300u && codepoint <= 0x1d356u) {
		return fantasy_block186[codepoint - 0x1d300u];
	} else if (codepoint >= 0x1f332u && codepoint <= 0x1f333u) {
		return fantasy_block187[codepoint - 0x1f332u];
	} else if (codepoint >= 0x1f344u && codepoint <= 0x1f345u) {
		return fantasy_block188[codepoint - 0x1f344u];
	} else if (codepoint >= 0x1f34eu && codepoint <= 0x1f34fu) {
		return fantasy_block189[codepoint - 0x1f34eu];
	} else if (codepoint == 0x1f37au) {
		return fantasy_block190[0];
	} else if (codepoint == 0x1f37eu) {
		return fantasy_block191[0];
	} else if (codepoint == 0x1f3ceu) {
		return fantasy_block192[0];
	} else if (codepoint == 0x1f3d0u) {
		return fantasy_block193[0];
	} else if (codepoint == 0x1f3f0u) {
		return fantasy_block194[0];
	} else if (codepoint >= 0x1f407u && codepoint <= 0x1f408u) {
		return fantasy_block195[codepoint - 0x1f407u];
	} else if (codepoint == 0x1f40du) {
		return fantasy_block196[0];
	} else if (codepoint == 0x1f419u) {
		return fantasy_block197[0];
	} else if (codepoint == 0x1f41fu) {
		return fantasy_block198[0];
	} else if (codepoint == 0x1f42au) {
		return fantasy_block199[0];
	} else if (codepoint == 0x1f47bu) {
		return fantasy_block200[0];
	} else if (codepoint == 0x1f47eu) {
		return fantasy_block201[0];
	} else if (codepoint == 0x1f480u) {
		return fantasy_block202[0];
	} else if (codepoint >= 0x1f48du && codepoint <= 0x1f48eu) {
		return fantasy_block203[codepoint - 0x1f48du];
	} else if (codepoint == 0x1f4a7u) {
		return fantasy_block204[0];
	} else if (codepoint == 0x1f4adu) {
		return fantasy_block205[0];
	} else if (codepoint == 0x1f4b0u) {
		return fantasy_block206[0];
	} else if (codepoint == 0x1f4beu) {
		return fantasy_block207[0];
	} else if (codepoint == 0x1f4c1u) {
		return fantasy_block208[0];
	} else if (codepoint == 0x1f4c4u) {
		return fantasy_block209[0];
	} else if (codepoint == 0x1f4dcu) {
		return fantasy_block210[0];
	} else if (codepoint == 0x1f4fbu) {
		return fantasy_block211[0];
	} else if (codepoint == 0x1f514u) {
		return fantasy_block212[0];
	} else if (codepoint == 0x1f52bu) {
		return fantasy_block213[0];
	} else if (codepoint == 0x1f52eu) {
		return fantasy_block214[0];
	} else if (codepoint == 0x1f574u) {
		return fantasy_block215[0];
	} else if (codepoint == 0x1f577u) {
		return fantasy_block216[0];
	} else if (codepoint == 0x1f5a8u) {
		return fantasy_block217[0];
	} else if (codepoint == 0x1f5d1u) {
		return fantasy_block218[0];
	} else if (codepoint == 0x1f68du) {
		return fantasy_block219[0];
	} else if (codepoint == 0x1f6d6u) {
		return fantasy_block220[0];
	} else if (codepoint == 0x1f6f8u) {
		return fantasy_block221[0];
	} else if (codepoint == 0x1f860u) {
		return fantasy_block222[0];
	} else if (codepoint == 0x1f986u) {
		return fantasy_block223[0];
	} else if (codepoint == 0x1f98bu) {
		return fantasy_block224[0];
	} else if (codepoint == 0x1f9f2u) {
		return fantasy_block225[0];
	} else if (codepoint == 0x1fa90u) {
		return fantasy_block226[0];
	} else if (codepoint == 0x1fa93u) {
		return fantasy_block227[0];
	} else if (codepoint == 0x1fa99u) {
		return fantasy_block228[0];
	} else if (codepoint == 0x1fa9cu) {
		return fantasy_block229[0];
	} else if (codepoint >= 0x1faa6u && codepoint <= 0x1faa8u) {
		return fantasy_block230[codepoint - 0x1faa6u];
	} else if (codepoint >= 0x1fb00u && codepoint <= 0x1fbcau) {
		return fantasy_block231[codepoint - 0x1fb00u];
	} else if (codepoint >= 0x1fbf0u && codepoint <= 0x1fbf9u) {
		return fantasy_block232[codepoint - 0x1fbf0u];
	} else {
		WARN("Codepoint U+%04X (%s) is not covered by this font", codepoint, u8_cp_to_utf8(codepoint));
		return fantasy_block1[0];
	}
}

static const unsigned char*
    mcr_get_bitmap(uint32_t codepoint)
{
	if (codepoint <= 0x195u) {
		return mcr_block1[codepoint];
	} else if (codepoint >= 0x197u && codepoint <= 0x19fu) {
		return mcr_block2[codepoint - 0x197u];
	} else if (codepoint == 0x1b7u) {
		return mcr_block3[0];
	} else if (codepoint >= 0x1cdu && codepoint <= 0x1e3u) {
		return mcr_block4[codepoint - 0x1cdu];
	} else if (codepoint >= 0x1e6u && codepoint <= 0x1e9u) {
		return mcr_block5[codepoint - 0x1e6u];
	} else if (codepoint >= 0x1eeu && codepoint <= 0x1f0u) {
		return mcr_block6[codepoint - 0x1eeu];
	} else if (codepoint >= 0x1f4u && codepoint <= 0x1f5u) {
		return mcr_block7[codepoint - 0x1f4u];
	} else if (codepoint >= 0x1f8u && codepoint <= 0x21bu) {
		return mcr_block8[codepoint - 0x1f8u];
	} else if (codepoint >= 0x226u && codepoint <= 0x227u) {
		return mcr_block9[codepoint - 0x226u];
	} else if (codepoint >= 0x22au && codepoint <= 0x233u) {
		return mcr_block10[codepoint - 0x22au];
	} else if (codepoint >= 0x258u && codepoint <= 0x259u) {
		return mcr_block11[codepoint - 0x258u];
	} else if (codepoint == 0x263u) {
		return mcr_block12[0];
	} else if (codepoint == 0x292u) {
		return mcr_block13[0];
	} else if (codepoint == 0x294u) {
		return mcr_block14[0];
	} else if (codepoint == 0x2c6u) {
		return mcr_block15[0];
	} else if (codepoint == 0x2c9u) {
		return mcr_block16[0];
	} else if (codepoint == 0x2cdu) {
		return mcr_block17[0];
	} else if (codepoint == 0x2dcu) {
		return mcr_block18[0];
	} else if (codepoint >= 0x300u && codepoint <= 0x30du) {
		return mcr_block19[codepoint - 0x300u];
	} else if (codepoint >= 0x30fu && codepoint <= 0x311u) {
		return mcr_block20[codepoint - 0x30fu];
	} else if (codepoint >= 0x313u && codepoint <= 0x318u) {
		return mcr_block21[codepoint - 0x313u];
	} else if (codepoint >= 0x31bu && codepoint <= 0x31cu) {
		return mcr_block22[codepoint - 0x31bu];
	} else if (codepoint >= 0x31fu && codepoint <= 0x329u) {
		return mcr_block23[codepoint - 0x31fu];
	} else if (codepoint >= 0x32bu && codepoint <= 0x32cu) {
		return mcr_block24[codepoint - 0x32bu];
	} else if (codepoint == 0x32eu) {
		return mcr_block25[0];
	} else if (codepoint == 0x330u) {
		return mcr_block26[0];
	} else if (codepoint >= 0x332u && codepoint <= 0x333u) {
		return mcr_block27[codepoint - 0x332u];
	} else if (codepoint >= 0x335u && codepoint <= 0x33eu) {
		return mcr_block28[codepoint - 0x335u];
	} else if (codepoint >= 0x340u && codepoint <= 0x344u) {
		return mcr_block29[codepoint - 0x340u];
	} else if (codepoint == 0x34fu) {
		return mcr_block30[0];
	} else if (codepoint == 0x362u) {
		return mcr_block31[0];
	} else if (codepoint == 0x386u) {
		return mcr_block32[0];
	} else if (codepoint >= 0x388u && codepoint <= 0x38au) {
		return mcr_block33[codepoint - 0x388u];
	} else if (codepoint == 0x38cu) {
		return mcr_block34[0];
	} else if (codepoint >= 0x38eu && codepoint <= 0x38fu) {
		return mcr_block35[codepoint - 0x38eu];
	} else if (codepoint >= 0x391u && codepoint <= 0x3a1u) {
		return mcr_block36[codepoint - 0x391u];
	} else if (codepoint >= 0x3a3u && codepoint <= 0x3afu) {
		return mcr_block37[codepoint - 0x3a3u];
	} else if (codepoint >= 0x3b1u && codepoint <= 0x3ceu) {
		return mcr_block38[codepoint - 0x3b1u];
	} else if (codepoint >= 0x400u && codepoint <= 0x401u) {
		return mcr_block39[codepoint - 0x400u];
	} else if (codepoint >= 0x405u && codepoint <= 0x409u) {
		return mcr_block40[codepoint - 0x405u];
	} else if (codepoint >= 0x410u && codepoint <= 0x451u) {
		return mcr_block41[codepoint - 0x410u];
	} else if (codepoint >= 0x455u && codepoint <= 0x458u) {
		return mcr_block42[codepoint - 0x455u];
	} else if (codepoint >= 0x4d0u && codepoint <= 0x4d7u) {
		return mcr_block43[codepoint - 0x4d0u];
	} else if (codepoint >= 0x4e6u && codepoint <= 0x4e7u) {
		return mcr_block44[codepoint - 0x4e6u];
	} else if (codepoint == 0x4f1u) {
		return mcr_block45[0];
	} else if (codepoint >= 0x5d0u && codepoint <= 0x5eau) {
		return mcr_block46[codepoint - 0x5d0u];
	} else if (codepoint == 0x623u) {
		return mcr_block47[0];
	} else if (codepoint >= 0x627u && codepoint <= 0x62du) {
		return mcr_block48[codepoint - 0x627u];
	} else if (codepoint >= 0x62fu && codepoint <= 0x635u) {
		return mcr_block49[codepoint - 0x62fu];
	} else if (codepoint >= 0x637u && codepoint <= 0x63au) {
		return mcr_block50[codepoint - 0x637u];
	} else if (codepoint >= 0x640u && codepoint <= 0x64au) {
		return mcr_block51[codepoint - 0x640u];
	} else if (codepoint >= 0x671u && codepoint <= 0x673u) {
		return mcr_block52[codepoint - 0x671u];
	} else if (codepoint >= 0x1560u && codepoint <= 0x1563u) {
		return mcr_block53[codepoint - 0x1560u];
	} else if (codepoint == 0x156au) {
		return mcr_block54[0];
	} else if (codepoint >= 0x156cu && codepoint <= 0x1571u) {
		return mcr_block55[codepoint - 0x156cu];
	} else if (codepoint >= 0x15ecu && codepoint <= 0x15edu) {
		return mcr_block56[codepoint - 0x15ecu];
	} else if (codepoint == 0x15efu) {
		return mcr_block57[0];
	} else if (codepoint >= 0x16a0u && codepoint <= 0x16abu) {
		return mcr_block58[codepoint - 0x16a0u];
	} else if (codepoint >= 0x16b1u && codepoint <= 0x16f8u) {
		return mcr_block59[codepoint - 0x16b1u];
	} else if (codepoint >= 0x1e00u && codepoint <= 0x1e02u) {
		return mcr_block60[codepoint - 0x1e00u];
	} else if (codepoint >= 0x1e04u && codepoint <= 0x1e07u) {
		return mcr_block61[codepoint - 0x1e04u];
	} else if (codepoint == 0x1e0au) {
		return mcr_block62[0];
	} else if (codepoint >= 0x1e0cu && codepoint <= 0x1e0fu) {
		return mcr_block63[codepoint - 0x1e0cu];
	} else if (codepoint >= 0x1e41u && codepoint <= 0x1e4bu) {
		return mcr_block64[codepoint - 0x1e41u];
	} else if (codepoint >= 0x1e58u && codepoint <= 0x1e5bu) {
		return mcr_block65[codepoint - 0x1e58u];
	} else if (codepoint == 0x1e5eu) {
		return mcr_block66[0];
	} else if (codepoint >= 0x1f00u && codepoint <= 0x1f7du) {
		return mcr_block67[codepoint - 0x1f00u];
	} else if (codepoint >= 0x1f80u && codepoint <= 0x1fb4u) {
		return mcr_block68[codepoint - 0x1f80u];
	} else if (codepoint >= 0x1fb6u && codepoint <= 0x1fbcu) {
		return mcr_block69[codepoint - 0x1fb6u];
	} else if (codepoint >= 0x1fc2u && codepoint <= 0x1fc4u) {
		return mcr_block70[codepoint - 0x1fc2u];
	} else if (codepoint >= 0x1fc6u && codepoint <= 0x1fc7u) {
		return mcr_block71[codepoint - 0x1fc6u];
	} else if (codepoint >= 0x1fcau && codepoint <= 0x1fccu) {
		return mcr_block72[codepoint - 0x1fcau];
	} else if (codepoint >= 0x1fd0u && codepoint <= 0x1fd4u) {
		return mcr_block73[codepoint - 0x1fd0u];
	} else if (codepoint >= 0x1fd6u && codepoint <= 0x1fdcu) {
		return mcr_block74[codepoint - 0x1fd6u];
	} else if (codepoint >= 0x1fe0u && codepoint <= 0x1fe3u) {
		return mcr_block75[codepoint - 0x1fe0u];
	} else if (codepoint >= 0x1fe6u && codepoint <= 0x1febu) {
		return mcr_block76[codepoint - 0x1fe6u];
	} else if (codepoint >= 0x1ff2u && codepoint <= 0x1ff4u) {
		return mcr_block77[codepoint - 0x1ff2u];
	} else if (codepoint >= 0x1ff6u && codepoint <= 0x1ff7u) {
		return mcr_block78[codepoint - 0x1ff6u];
	} else if (codepoint >= 0x1ffau && codepoint <= 0x1ffcu) {
		return mcr_block79[codepoint - 0x1ffau];
	} else if (codepoint >= 0x2010u && codepoint <= 0x2019u) {
		return mcr_block80[codepoint - 0x2010u];
	} else if (codepoint >= 0x201cu && codepoint <= 0x201du) {
		return mcr_block81[codepoint - 0x201cu];
	} else if (codepoint >= 0x2020u && codepoint <= 0x2022u) {
		return mcr_block82[codepoint - 0x2020u];
	} else if (codepoint >= 0x2024u && codepoint <= 0x2026u) {
		return mcr_block83[codepoint - 0x2024u];
	} else if (codepoint >= 0x2030u && codepoint <= 0x2037u) {
		return mcr_block84[codepoint - 0x2030u];
	} else if (codepoint >= 0x203cu && codepoint <= 0x203du) {
		return mcr_block85[codepoint - 0x203cu];
	} else if (codepoint == 0x2044u) {
		return mcr_block86[0];
	} else if (codepoint >= 0x2047u && codepoint <= 0x2049u) {
		return mcr_block87[codepoint - 0x2047u];
	} else if (codepoint >= 0x2070u && codepoint <= 0x2071u) {
		return mcr_block88[codepoint - 0x2070u];
	} else if (codepoint >= 0x2074u && codepoint <= 0x207cu) {
		return mcr_block89[codepoint - 0x2074u];
	} else if (codepoint == 0x207fu) {
		return mcr_block90[0];
	} else if (codepoint == 0x20a7u) {
		return mcr_block91[0];
	} else if (codepoint == 0x20acu) {
		return mcr_block92[0];
	} else if (codepoint == 0x2117u) {
		return mcr_block93[0];
	} else if (codepoint == 0x2120u) {
		return mcr_block94[0];
	} else if (codepoint == 0x2122u) {
		return mcr_block95[0];
	} else if (codepoint >= 0x2150u && codepoint <= 0x215fu) {
		return mcr_block96[codepoint - 0x2150u];
	} else if (codepoint == 0x2189u) {
		return mcr_block97[0];
	} else if (codepoint >= 0x2190u && codepoint <= 0x2199u) {
		return mcr_block98[codepoint - 0x2190u];
	} else if (codepoint == 0x21a5u) {
		return mcr_block99[0];
	} else if (codepoint == 0x21a8u) {
		return mcr_block100[0];
	} else if (codepoint == 0x21b2u) {
		return mcr_block101[0];
	} else if (codepoint >= 0x21bcu && codepoint <= 0x21c3u) {
		return mcr_block102[codepoint - 0x21bcu];
	} else if (codepoint >= 0x21e6u && codepoint <= 0x21e9u) {
		return mcr_block103[codepoint - 0x21e6u];
	} else if (codepoint == 0x2205u) {
		return mcr_block104[0];
	} else if (codepoint >= 0x2212u && codepoint <= 0x2215u) {
		return mcr_block105[codepoint - 0x2212u];
	} else if (codepoint == 0x2218u) {
		return mcr_block106[0];
	} else if (codepoint >= 0x221au && codepoint <= 0x221cu) {
		return mcr_block107[codepoint - 0x221au];
	} else if (codepoint >= 0x221eu && codepoint <= 0x221fu) {
		return mcr_block108[codepoint - 0x221eu];
	} else if (codepoint == 0x2229u) {
		return mcr_block109[0];
	} else if (codepoint == 0x2248u) {
		return mcr_block110[0];
	} else if (codepoint == 0x2261u) {
		return mcr_block111[0];
	} else if (codepoint >= 0x2264u && codepoint <= 0x2265u) {
		return mcr_block112[codepoint - 0x2264u];
	} else if (codepoint == 0x22c5u) {
		return mcr_block113[0];
	} else if (codepoint == 0x2301u) {
		return mcr_block114[0];
	} else if (codepoint == 0x2318u) {
		return mcr_block115[0];
	} else if (codepoint >= 0x231au && codepoint <= 0x231bu) {
		return mcr_block116[codepoint - 0x231au];
	} else if (codepoint >= 0x2320u && codepoint <= 0x2321u) {
		return mcr_block117[codepoint - 0x2320u];
	} else if (codepoint >= 0x239bu && codepoint <= 0x23afu) {
		return mcr_block118[codepoint - 0x239bu];
	} else if (codepoint >= 0x23b2u && codepoint <= 0x23b3u) {
		return mcr_block119[codepoint - 0x23b2u];
	} else if (codepoint >= 0x23b7u && codepoint <= 0x23bdu) {
		return mcr_block120[codepoint - 0x23b7u];
	} else if (codepoint == 0x23d0u) {
		return mcr_block121[0];
	} else if (codepoint == 0x23dau) {
		return mcr_block122[0];
	} else if (codepoint >= 0x23e9u && codepoint <= 0x23ecu) {
		return mcr_block123[codepoint - 0x23e9u];
	} else if (codepoint >= 0x2500u && codepoint <= 0x25ffu) {
		return mcr_block124[codepoint - 0x2500u];
	} else if (codepoint == 0x2602u) {
		return mcr_block125[0];
	} else if (codepoint >= 0x2605u && codepoint <= 0x2606u) {
		return mcr_block126[codepoint - 0x2605u];
	} else if (codepoint == 0x2609u) {
		return mcr_block127[0];
	} else if (codepoint == 0x260eu) {
		return mcr_block128[0];
	} else if (codepoint >= 0x2610u && codepoint <= 0x2612u) {
		return mcr_block129[codepoint - 0x2610u];
	} else if (codepoint == 0x2615u) {
		return mcr_block130[0];
	} else if (codepoint == 0x2625u) {
		return mcr_block131[0];
	} else if (codepoint == 0x2628u) {
		return mcr_block132[0];
	} else if (codepoint >= 0x262fu && codepoint <= 0x2637u) {
		return mcr_block133[codepoint - 0x262fu];
	} else if (codepoint >= 0x2639u && codepoint <= 0x263bu) {
		return mcr_block134[codepoint - 0x2639u];
	} else if (codepoint == 0x2640u) {
		return mcr_block135[0];
	} else if (codepoint == 0x2642u) {
		return mcr_block136[0];
	} else if (codepoint >= 0x2660u && codepoint <= 0x2667u) {
		return mcr_block137[codepoint - 0x2660u];
	} else if (codepoint >= 0x2669u && codepoint <= 0x266cu) {
		return mcr_block138[codepoint - 0x2669u];
	} else if (codepoint == 0x2683u) {
		return mcr_block139[0];
	} else if (codepoint >= 0x268au && codepoint <= 0x268fu) {
		return mcr_block140[codepoint - 0x268au];
	} else if (codepoint >= 0x269eu && codepoint <= 0x269fu) {
		return mcr_block141[codepoint - 0x269eu];
	} else if (codepoint >= 0x26aau && codepoint <= 0x26acu) {
		return mcr_block142[codepoint - 0x26aau];
	} else if (codepoint == 0x26c8u) {
		return mcr_block143[0];
	} else if (codepoint == 0x26f5u) {
		return mcr_block144[0];
	} else if (codepoint == 0x2708u) {
		return mcr_block145[0];
	} else if (codepoint == 0x2713u) {
		return mcr_block146[0];
	} else if (codepoint >= 0x2734u && codepoint <= 0x2735u) {
		return mcr_block147[codepoint - 0x2734u];
	} else if (codepoint == 0x2764u) {
		return mcr_block148[0];
	} else if (codepoint >= 0x2800u && codepoint <= 0x28ffu) {
		return mcr_block149[codepoint - 0x2800u];
	} else if (codepoint == 0x29c9u) {
		return mcr_block150[0];
	} else if (codepoint >= 0x2b12u && codepoint <= 0x2b19u) {
		return mcr_block151[codepoint - 0x2b12u];
	} else if (codepoint == 0x2b1du) {
		return mcr_block152[0];
	} else if (codepoint >= 0x2b24u && codepoint <= 0x2b2fu) {
		return mcr_block153[codepoint - 0x2b24u];
	} else if (codepoint == 0x2b55u) {
		return mcr_block154[0];
	} else if (codepoint == 0x2b58u) {
		return mcr_block155[0];
	} else if (codepoint == 0x2b73u) {
		return mcr_block156[0];
	} else if (codepoint >= 0x2b9cu && codepoint <= 0x2b9fu) {
		return mcr_block157[codepoint - 0x2b9cu];
	} else if (codepoint == 0x2bbau) {
		return mcr_block158[0];
	} else if (codepoint == 0x2bc0u) {
		return mcr_block159[0];
	} else if (codepoint >= 0x2bc5u && codepoint <= 0x2bc8u) {
		return mcr_block160[codepoint - 0x2bc5u];
	} else if (codepoint >= 0x2bcau && codepoint <= 0x2bcbu) {
		return mcr_block161[codepoint - 0x2bcau];
	} else if (codepoint == 0x2e2eu) {
		return mcr_block162[0];
	} else if (codepoint >= 0x4dc0u && codepoint <= 0x4dffu) {
		return mcr_block163[codepoint - 0x4dc0u];
	} else if (codepoint >= 0xe080u && codepoint <= 0xe1aau) {
		return mcr_block164[codepoint - 0xe080u];
	} else if (codepoint >= 0xe800u && codepoint <= 0xe87du) {
		return mcr_block165[codepoint - 0xe800u];
	} else if (codepoint >= 0xec00u && codepoint <= 0xec07u) {
		return mcr_block166[codepoint - 0xec00u];
	} else if (codepoint >= 0xec0au && codepoint <= 0xec7du) {
		return mcr_block167[codepoint - 0xec0au];
	} else if (codepoint >= 0xec7fu && codepoint <= 0xecdeu) {
		return mcr_block168[codepoint - 0xec7fu];
	} else if (codepoint >= 0xfe81u && codepoint <= 0xfe82u) {
		return mcr_block169[codepoint - 0xfe81u];
	} else if (codepoint == 0xfe84u) {
		return mcr_block170[0];
	} else if (codepoint == 0xfe87u) {
		return mcr_block171[0];
	} else if (codepoint >= 0xfe8fu && codepoint <= 0xfe9cu) {
		return mcr_block172[codepoint - 0xfe8fu];
	} else if (codepoint >= 0xfe9eu && codepoint <= 0xfea4u) {
		return mcr_block173[codepoint - 0xfe9eu];
	} else if (codepoint >= 0xfea9u && codepoint <= 0xfeadu) {
		return mcr_block174[codepoint - 0xfea9u];
	} else if (codepoint == 0xfeafu) {
		return mcr_block175[0];
	} else if (codepoint >= 0xfeb1u && codepoint <= 0xfeb7u) {
		return mcr_block176[codepoint - 0xfeb1u];
	} else if (codepoint >= 0xfeb9u && codepoint <= 0xfebcu) {
		return mcr_block177[codepoint - 0xfeb9u];
	} else if (codepoint >= 0xfec1u && codepoint <= 0xfec6u) {
		return mcr_block178[codepoint - 0xfec1u];
	} else if (codepoint >= 0xfec9u && codepoint <= 0xfeddu) {
		return mcr_block179[codepoint - 0xfec9u];
	} else if (codepoint >= 0xfedfu && codepoint <= 0xfee3u) {
		return mcr_block180[codepoint - 0xfedfu];
	} else if (codepoint >= 0xfee5u && codepoint <= 0xfee9u) {
		return mcr_block181[codepoint - 0xfee5u];
	} else if (codepoint >= 0xfeebu && codepoint <= 0xfef4u) {
		return mcr_block182[codepoint - 0xfeebu];
	} else if (codepoint >= 0xff01u && codepoint <= 0xff36u) {
		return mcr_block183[codepoint - 0xff01u];
	} else if (codepoint >= 0xff38u && codepoint <= 0xff5du) {
		return mcr_block184[codepoint - 0xff38u];
	} else if (codepoint >= 0xff5fu && codepoint <= 0xff9fu) {
		return mcr_block185[codepoint - 0xff5fu];
	} else if (codepoint >= 0x1d300u && codepoint <= 0x1d356u) {
		return mcr_block186[codepoint - 0x1d300u];
	} else if (codepoint >= 0x1f332u && codepoint <= 0x1f333u) {
		return mcr_block187[codepoint - 0x1f332u];
	} else if (codepoint >= 0x1f344u && codepoint <= 0x1f345u) {
		return mcr_block188[codepoint - 0x1f344u];
	} else if (codepoint >= 0x1f34eu && codepoint <= 0x1f34fu) {
		return mcr_block189[codepoint - 0x1f34eu];
	} else if (codepoint == 0x1f37au) {
		return mcr_block190[0];
	} else if (codepoint == 0x1f37eu) {
		return mcr_block191[0];
	} else if (codepoint == 0x1f3ceu) {
		return mcr_block192[0];
	} else if (codepoint == 0x1f3d0u) {
		return mcr_block193[0];
	} else if (codepoint == 0x1f3f0u) {
		return mcr_block194[0];
	} else if (codepoint >= 0x1f407u && codepoint <= 0x1f408u) {
		return mcr_block195[codepoint - 0x1f407u];
	} else if (codepoint == 0x1f40du) {
		return mcr_block196[0];
	} else if (codepoint == 0x1f419u) {
		return mcr_block197[0];
	} else if (codepoint == 0x1f41fu) {
		return mcr_block198[0];
	} else if (codepoint == 0x1f42au) {
		return mcr_block199[0];
	} else if (codepoint == 0x1f47bu) {
		return mcr_block200[0];
	} else if (codepoint == 0x1f47eu) {
		return mcr_block201[0];
	} else if (codepoint == 0x1f480u) {
		return mcr_block202[0];
	} else if (codepoint >= 0x1f48du && codepoint <= 0x1f48eu) {
		return mcr_block203[codepoint - 0x1f48du];
	} else if (codepoint == 0x1f4a7u) {
		return mcr_block204[0];
	} else if (codepoint == 0x1f4adu) {
		return mcr_block205[0];
	} else if (codepoint == 0x1f4b0u) {
		return mcr_block206[0];
	} else if (codepoint == 0x1f4beu) {
		return mcr_block207[0];
	} else if (codepoint == 0x1f4c1u) {
		return mcr_block208[0];
	} else if (codepoint == 0x1f4c4u) {
		return mcr_block209[0];
	} else if (codepoint == 0x1f4dcu) {
		return mcr_block210[0];
	} else if (codepoint == 0x1f4fbu) {
		return mcr_block211[0];
	} else if (codepoint == 0x1f514u) {
		return mcr_block212[0];
	} else if (codepoint == 0x1f52bu) {
		return mcr_block213[0];
	} else if (codepoint == 0x1f52eu) {
		return mcr_block214[0];
	} else if (codepoint == 0x1f574u) {
		return mcr_block215[0];
	} else if (codepoint == 0x1f577u) {
		return mcr_block216[0];
	} else if (codepoint == 0x1f5a8u) {
		return mcr_block217[0];
	} else if (codepoint == 0x1f5d1u) {
		return mcr_block218[0];
	} else if (codepoint == 0x1f68du) {
		return mcr_block219[0];
	} else if (codepoint == 0x1f6d6u) {
		return mcr_block220[0];
	} else if (codepoint == 0x1f6f8u) {
		return mcr_block221[0];
	} else if (codepoint == 0x1f860u) {
		return mcr_block222[0];
	} else if (codepoint == 0x1f986u) {
		return mcr_block223[0];
	} else if (codepoint == 0x1f98bu) {
		return mcr_block224[0];
	} else if (codepoint == 0x1f9f2u) {
		return mcr_block225[0];
	} else if (codepoint == 0x1fa90u) {
		return mcr_block226[0];
	} else if (codepoint == 0x1fa93u) {
		return mcr_block227[0];
	} else if (codepoint == 0x1fa99u) {
		return mcr_block228[0];
	} else if (codepoint == 0x1fa9cu) {
		return mcr_block229[0];
	} else if (codepoint >= 0x1faa6u && codepoint <= 0x1faa8u) {
		return mcr_block230[codepoint - 0x1faa6u];
	} else if (codepoint >= 0x1fb00u && codepoint <= 0x1fbcau) {
		return mcr_block231[codepoint - 0x1fb00u];
	} else if (codepoint >= 0x1fbf0u && codepoint <= 0x1fbf9u) {
		return mcr_block232[codepoint - 0x1fbf0u];
	} else {
		WARN("Codepoint U+%04X (%s) is not covered by this font", codepoint, u8_cp_to_utf8(codepoint));
		return mcr_block1[0];
	}
}

static const unsigned char*
    tall_get_bitmap(uint32_t codepoint)
{
	if (codepoint <= 0x19fu) {
		return tall_block1[codepoint];
	} else if (codepoint == 0x1b7u) {
		return tall_block2[0];
	} else if (codepoint >= 0x1cdu && codepoint <= 0x1e3u) {
		return tall_block3[codepoint - 0x1cdu];
	} else if (codepoint >= 0x1e6u && codepoint <= 0x1e9u) {
		return tall_block4[codepoint - 0x1e6u];
	} else if (codepoint >= 0x1eeu && codepoint <= 0x1f0u) {
		return tall_block5[codepoint - 0x1eeu];
	} else if (codepoint >= 0x1f4u && codepoint <= 0x1f5u) {
		return tall_block6[codepoint - 0x1f4u];
	} else if (codepoint >= 0x1f8u && codepoint <= 0x21bu) {
		return tall_block7[codepoint - 0x1f8u];
	} else if (codepoint >= 0x226u && codepoint <= 0x227u) {
		return tall_block8[codepoint - 0x226u];
	} else if (codepoint >= 0x22au && codepoint <= 0x233u) {
		return tall_block9[codepoint - 0x22au];
	} else if (codepoint >= 0x258u && codepoint <= 0x259u) {
		return tall_block10[codepoint - 0x258u];
	} else if (codepoint >= 0x25bu && codepoint <= 0x25cu) {
		return tall_block11[codepoint - 0x25bu];
	} else if (codepoint == 0x263u) {
		return tall_block12[0];
	} else if (codepoint == 0x26au) {
		return tall_block13[0];
	} else if (codepoint >= 0x28au && codepoint <= 0x296u) {
		return tall_block14[codepoint - 0x28au];
	} else if (codepoint == 0x2c6u) {
		return tall_block15[0];
	} else if (codepoint == 0x2c9u) {
		return tall_block16[0];
	} else if (codepoint == 0x2cdu) {
		return tall_block17[0];
	} else if (codepoint == 0x2dcu) {
		return tall_block18[0];
	} else if (codepoint >= 0x300u && codepoint <= 0x30du) {
		return tall_block19[codepoint - 0x300u];
	} else if (codepoint >= 0x30fu && codepoint <= 0x311u) {
		return tall_block20[codepoint - 0x30fu];
	} else if (codepoint >= 0x313u && codepoint <= 0x318u) {
		return tall_block21[codepoint - 0x313u];
	} else if (codepoint >= 0x31bu && codepoint <= 0x31cu) {
		return tall_block22[codepoint - 0x31bu];
	} else if (codepoint >= 0x31fu && codepoint <= 0x329u) {
		return tall_block23[codepoint - 0x31fu];
	} else if (codepoint >= 0x32bu && codepoint <= 0x32cu) {
		return tall_block24[codepoint - 0x32bu];
	} else if (codepoint == 0x32eu) {
		return tall_block25[0];
	} else if (codepoint == 0x330u) {
		return tall_block26[0];
	} else if (codepoint >= 0x332u && codepoint <= 0x333u) {
		return tall_block27[codepoint - 0x332u];
	} else if (codepoint >= 0x335u && codepoint <= 0x33eu) {
		return tall_block28[codepoint - 0x335u];
	} else if (codepoint >= 0x340u && codepoint <= 0x344u) {
		return tall_block29[codepoint - 0x340u];
	} else if (codepoint == 0x34fu) {
		return tall_block30[0];
	} else if (codepoint == 0x362u) {
		return tall_block31[0];
	} else if (codepoint == 0x386u) {
		return tall_block32[0];
	} else if (codepoint >= 0x388u && codepoint <= 0x38au) {
		return tall_block33[codepoint - 0x388u];
	} else if (codepoint == 0x38cu) {
		return tall_block34[0];
	} else if (codepoint >= 0x38eu && codepoint <= 0x38fu) {
		return tall_block35[codepoint - 0x38eu];
	} else if (codepoint >= 0x391u && codepoint <= 0x3a1u) {
		return tall_block36[codepoint - 0x391u];
	} else if (codepoint >= 0x3a3u && codepoint <= 0x3afu) {
		return tall_block37[codepoint - 0x3a3u];
	} else if (codepoint >= 0x3b1u && codepoint <= 0x3ceu) {
		return tall_block38[codepoint - 0x3b1u];
	} else if (codepoint >= 0x400u && codepoint <= 0x401u) {
		return tall_block39[codepoint - 0x400u];
	} else if (codepoint >= 0x405u && codepoint <= 0x409u) {
		return tall_block40[codepoint - 0x405u];
	} else if (codepoint >= 0x410u && codepoint <= 0x451u) {
		return tall_block41[codepoint - 0x410u];
	} else if (codepoint >= 0x455u && codepoint <= 0x458u) {
		return tall_block42[codepoint - 0x455u];
	} else if (codepoint >= 0x4d0u && codepoint <= 0x4d7u) {
		return tall_block43[codepoint - 0x4d0u];
	} else if (codepoint >= 0x4e6u && codepoint <= 0x4e7u) {
		return tall_block44[codepoint - 0x4e6u];
	} else if (codepoint == 0x4f1u) {
		return tall_block45[0];
	} else if (codepoint >= 0x5d0u && codepoint <= 0x5eau) {
		return tall_block46[codepoint - 0x5d0u];
	} else if (codepoint == 0x623u) {
		return tall_block47[0];
	} else if (codepoint >= 0x627u && codepoint <= 0x62du) {
		return tall_block48[codepoint - 0x627u];
	} else if (codepoint >= 0x62fu && codepoint <= 0x635u) {
		return tall_block49[codepoint - 0x62fu];
	} else if (codepoint >= 0x637u && codepoint <= 0x63au) {
		return tall_block50[codepoint - 0x637u];
	} else if (codepoint >= 0x640u && codepoint <= 0x64au) {
		return tall_block51[codepoint - 0x640u];
	} else if (codepoint >= 0x671u && codepoint <= 0x673u) {
		return tall_block52[codepoint - 0x671u];
	} else if (codepoint >= 0x1560u && codepoint <= 0x1563u) {
		return tall_block53[codepoint - 0x1560u];
	} else if (codepoint == 0x156au) {
		return tall_block54[0];
	} else if (codepoint >= 0x156cu && codepoint <= 0x1571u) {
		return tall_block55[codepoint - 0x156cu];
	} else if (codepoint >= 0x15ecu && codepoint <= 0x15edu) {
		return tall_block56[codepoint - 0x15ecu];
	} else if (codepoint == 0x15efu) {
		return tall_block57[0];
	} else if (codepoint >= 0x16a0u && codepoint <= 0x16abu) {
		return tall_block58[codepoint - 0x16a0u];
	} else if (codepoint >= 0x16b1u && codepoint <= 0x16f8u) {
		return tall_block59[codepoint - 0x16b1u];
	} else if (codepoint >= 0x1e00u && codepoint <= 0x1e02u) {
		return tall_block60[codepoint - 0x1e00u];
	} else if (codepoint >= 0x1e04u && codepoint <= 0x1e07u) {
		return tall_block61[codepoint - 0x1e04u];
	} else if (codepoint == 0x1e0au) {
		return tall_block62[0];
	} else if (codepoint >= 0x1e0cu && codepoint <= 0x1e0fu) {
		return tall_block63[codepoint - 0x1e0cu];
	} else if (codepoint >= 0x1e41u && codepoint <= 0x1e4bu) {
		return tall_block64[codepoint - 0x1e41u];
	} else if (codepoint >= 0x1e58u && codepoint <= 0x1e5bu) {
		return tall_block65[codepoint - 0x1e58u];
	} else if (codepoint == 0x1e5eu) {
		return tall_block66[0];
	} else if (codepoint >= 0x1f00u && codepoint <= 0x1f7du) {
		return tall_block67[codepoint - 0x1f00u];
	} else if (codepoint >= 0x1f80u && codepoint <= 0x1fb4u) {
		return tall_block68[codepoint - 0x1f80u];
	} else if (codepoint >= 0x1fb6u && codepoint <= 0x1fbcu) {
		return tall_block69[codepoint - 0x1fb6u];
	} else if (codepoint >= 0x1fc2u && codepoint <= 0x1fc4u) {
		return tall_block70[codepoint - 0x1fc2u];
	} else if (codepoint >= 0x1fc6u && codepoint <= 0x1fc7u) {
		return tall_block71[codepoint - 0x1fc6u];
	} else if (codepoint >= 0x1fcau && codepoint <= 0x1fccu) {
		return tall_block72[codepoint - 0x1fcau];
	} else if (codepoint >= 0x1fd0u && codepoint <= 0x1fd4u) {
		return tall_block73[codepoint - 0x1fd0u];
	} else if (codepoint >= 0x1fd6u && codepoint <= 0x1fdcu) {
		return tall_block74[codepoint - 0x1fd6u];
	} else if (codepoint >= 0x1fe0u && codepoint <= 0x1fe3u) {
		return tall_block75[codepoint - 0x1fe0u];
	} else if (codepoint >= 0x1fe6u && codepoint <= 0x1febu) {
		return tall_block76[codepoint - 0x1fe6u];
	} else if (codepoint >= 0x1ff2u && codepoint <= 0x1ff4u) {
		return tall_block77[codepoint - 0x1ff2u];
	} else if (codepoint >= 0x1ff6u && codepoint <= 0x1ff7u) {
		return tall_block78[codepoint - 0x1ff6u];
	} else if (codepoint >= 0x1ffau && codepoint <= 0x1ffcu) {
		return tall_block79[codepoint - 0x1ffau];
	} else if (codepoint >= 0x2010u && codepoint <= 0x2019u) {
		return tall_block80[codepoint - 0x2010u];
	} else if (codepoint >= 0x201cu && codepoint <= 0x201du) {
		return tall_block81[codepoint - 0x201cu];
	} else if (codepoint >= 0x2020u && codepoint <= 0x2022u) {
		return tall_block82[codepoint - 0x2020u];
	} else if (codepoint >= 0x2024u && codepoint <= 0x2026u) {
		return tall_block83[codepoint - 0x2024u];
	} else if (codepoint >= 0x2030u && codepoint <= 0x2037u) {
		return tall_block84[codepoint - 0x2030u];
	} else if (codepoint >= 0x203cu && codepoint <= 0x203du) {
		return tall_block85[codepoint - 0x203cu];
	} else if (codepoint == 0x2044u) {
		return tall_block86[0];
	} else if (codepoint >= 0x2047u && codepoint <= 0x2049u) {
		return tall_block87[codepoint - 0x2047u];
	} else if (codepoint >= 0x2070u && codepoint <= 0x2071u) {
		return tall_block88[codepoint - 0x2070u];
	} else if (codepoint >= 0x2074u && codepoint <= 0x207cu) {
		return tall_block89[codepoint - 0x2074u];
	} else if (codepoint == 0x207fu) {
		return tall_block90[0];
	} else if (codepoint == 0x20a7u) {
		return tall_block91[0];
	} else if (codepoint == 0x20acu) {
		return tall_block92[0];
	} else if (codepoint == 0x2117u) {
		return tall_block93[0];
	} else if (codepoint == 0x2120u) {
		return tall_block94[0];
	} else if (codepoint == 0x2122u) {
		return tall_block95[0];
	} else if (codepoint >= 0x2150u && codepoint <= 0x215fu) {
		return tall_block96[codepoint - 0x2150u];
	} else if (codepoint == 0x2189u) {
		return tall_block97[0];
	} else if (codepoint >= 0x2190u && codepoint <= 0x2199u) {
		return tall_block98[codepoint - 0x2190u];
	} else if (codepoint == 0x21a5u) {
		return tall_block99[0];
	} else if (codepoint == 0x21a8u) {
		return tall_block100[0];
	} else if (codepoint == 0x21b2u) {
		return tall_block101[0];
	} else if (codepoint >= 0x21bcu && codepoint <= 0x21c3u) {
		return tall_block102[codepoint - 0x21bcu];
	} else if (codepoint >= 0x21e6u && codepoint <= 0x21e9u) {
		return tall_block103[codepoint - 0x21e6u];
	} else if (codepoint == 0x2205u) {
		return tall_block104[0];
	} else if (codepoint >= 0x2212u && codepoint <= 0x2215u) {
		return tall_block105[codepoint - 0x2212u];
	} else if (codepoint == 0x2218u) {
		return tall_block106[0];
	} else if (codepoint >= 0x221au && codepoint <= 0x221cu) {
		return tall_block107[codepoint - 0x221au];
	} else if (codepoint >= 0x221eu && codepoint <= 0x221fu) {
		return tall_block108[codepoint - 0x221eu];
	} else if (codepoint == 0x2229u) {
		return tall_block109[0];
	} else if (codepoint == 0x2248u) {
		return tall_block110[0];
	} else if (codepoint == 0x2261u) {
		return tall_block111[0];
	} else if (codepoint >= 0x2264u && codepoint <= 0x2265u) {
		return tall_block112[codepoint - 0x2264u];
	} else if (codepoint == 0x22c5u) {
		return tall_block113[0];
	} else if (codepoint == 0x2301u) {
		return tall_block114[0];
	} else if (codepoint == 0x2318u) {
		return tall_block115[0];
	} else if (codepoint >= 0x2320u && codepoint <= 0x2321u) {
		return tall_block116[codepoint - 0x2320u];
	} else if (codepoint >= 0x239bu && codepoint <= 0x23afu) {
		return tall_block117[codepoint - 0x239bu];
	} else if (codepoint >= 0x23b2u && codepoint <= 0x23b3u) {
		return tall_block118[codepoint - 0x23b2u];
	} else if (codepoint >= 0x23b7u && codepoint <= 0x23bdu) {
		return tall_block119[codepoint - 0x23b7u];
	} else if (codepoint == 0x23d0u) {
		return tall_block120[0];
	} else if (codepoint == 0x23dau) {
		return tall_block121[0];
	} else if (codepoint >= 0x2500u && codepoint <= 0x25fcu) {
		return tall_block122[codepoint - 0x2500u];
	} else if (codepoint == 0x25ffu) {
		return tall_block123[0];
	} else if (codepoint == 0x2602u) {
		return tall_block124[0];
	} else if (codepoint >= 0x2605u && codepoint <= 0x2606u) {
		return tall_block125[codepoint - 0x2605u];
	} else if (codepoint == 0x2609u) {
		return tall_block126[0];
	} else if (codepoint == 0x260eu) {
		return tall_block127[0];
	} else if (codepoint >= 0x2610u && codepoint <= 0x2612u) {
		return tall_block128[codepoint - 0x2610u];
	} else if (codepoint == 0x2625u) {
		return tall_block129[0];
	} else if (codepoint == 0x2628u) {
		return tall_block130[0];
	} else if (codepoint >= 0x262fu && codepoint <= 0x2637u) {
		return tall_block131[codepoint - 0x262fu];
	} else if (codepoint >= 0x2639u && codepoint <= 0x263bu) {
		return tall_block132[codepoint - 0x2639u];
	} else if (codepoint == 0x2640u) {
		return tall_block133[0];
	} else if (codepoint == 0x2642u) {
		return tall_block134[0];
	} else if (codepoint >= 0x2660u && codepoint <= 0x2667u) {
		return tall_block135[codepoint - 0x2660u];
	} else if (codepoint >= 0x2669u && codepoint <= 0x266cu) {
		return tall_block136[codepoint - 0x2669u];
	} else if (codepoint == 0x2683u) {
		return tall_block137[0];
	} else if (codepoint >= 0x268au && codepoint <= 0x268fu) {
		return tall_block138[codepoint - 0x268au];
	} else if (codepoint >= 0x269eu && codepoint <= 0x269fu) {
		return tall_block139[codepoint - 0x269eu];
	} else if (codepoint == 0x26acu) {
		return tall_block140[0];
	} else if (codepoint == 0x26c8u) {
		return tall_block141[0];
	} else if (codepoint == 0x2708u) {
		return tall_block142[0];
	} else if (codepoint == 0x2713u) {
		return tall_block143[0];
	} else if (codepoint >= 0x2734u && codepoint <= 0x2735u) {
		return tall_block144[codepoint - 0x2734u];
	} else if (codepoint == 0x2764u) {
		return tall_block145[0];
	} else if (codepoint >= 0x2800u && codepoint <= 0x28ffu) {
		return tall_block146[codepoint - 0x2800u];
	} else if (codepoint == 0x29c9u) {
		return tall_block147[0];
	} else if (codepoint >= 0x2b12u && codepoint <= 0x2b19u) {
		return tall_block148[codepoint - 0x2b12u];
	} else if (codepoint == 0x2b1du) {
		return tall_block149[0];
	} else if (codepoint >= 0x2b24u && codepoint <= 0x2b2fu) {
		return tall_block150[codepoint - 0x2b24u];
	} else if (codepoint == 0x2b58u) {
		return tall_block151[0];
	} else if (codepoint == 0x2b73u) {
		return tall_block152[0];
	} else if (codepoint >= 0x2b9cu && codepoint <= 0x2b9fu) {
		return tall_block153[codepoint - 0x2b9cu];
	} else if (codepoint == 0x2bbau) {
		return tall_block154[0];
	} else if (codepoint == 0x2bc0u) {
		return tall_block155[0];
	} else if (codepoint >= 0x2bc5u && codepoint <= 0x2bc8u) {
		return tall_block156[codepoint - 0x2bc5u];
	} else if (codepoint >= 0x2bcau && codepoint <= 0x2bcbu) {
		return tall_block157[codepoint - 0x2bcau];
	} else if (codepoint == 0x2e2eu) {
		return tall_block158[0];
	} else if (codepoint >= 0xe080u && codepoint <= 0xe1aau) {
		return tall_block159[codepoint - 0xe080u];
	} else if (codepoint >= 0xe800u && codepoint <= 0xe87du) {
		return tall_block160[codepoint - 0xe800u];
	} else if (codepoint >= 0xec00u && codepoint <= 0xec07u) {
		return tall_block161[codepoint - 0xec00u];
	} else if (codepoint >= 0xec0au && codepoint <= 0xec7du) {
		return tall_block162[codepoint - 0xec0au];
	} else if (codepoint >= 0xec7fu && codepoint <= 0xecdeu) {
		return tall_block163[codepoint - 0xec7fu];
	} else if (codepoint >= 0xfe81u && codepoint <= 0xfe82u) {
		return tall_block164[codepoint - 0xfe81u];
	} else if (codepoint == 0xfe84u) {
		return tall_block165[0];
	} else if (codepoint >= 0xfe87u && codepoint <= 0xfe88u) {
		return tall_block166[codepoint - 0xfe87u];
	} else if (codepoint >= 0xfe8eu && codepoint <= 0xfe9cu) {
		return tall_block167[codepoint - 0xfe8eu];
	} else if (codepoint >= 0xfe9eu && codepoint <= 0xfea4u) {
		return tall_block168[codepoint - 0xfe9eu];
	} else if (codepoint >= 0xfea9u && codepoint <= 0xfeb7u) {
		return tall_block169[codepoint - 0xfea9u];
	} else if (codepoint >= 0xfeb9u && codepoint <= 0xfebcu) {
		return tall_block170[codepoint - 0xfeb9u];
	} else if (codepoint >= 0xfec1u && codepoint <= 0xfec6u) {
		return tall_block171[codepoint - 0xfec1u];
	} else if (codepoint >= 0xfec9u && codepoint <= 0xfee3u) {
		return tall_block172[codepoint - 0xfec9u];
	} else if (codepoint >= 0xfee5u && codepoint <= 0xfef4u) {
		return tall_block173[codepoint - 0xfee5u];
	} else if (codepoint >= 0xff61u && codepoint <= 0xff9fu) {
		return tall_block174[codepoint - 0xff61u];
	} else if (codepoint >= 0x1d300u && codepoint <= 0x1d356u) {
		return tall_block175[codepoint - 0x1d300u];
	} else if (codepoint == 0x1f3ceu) {
		return tall_block176[0];
	} else if (codepoint == 0x1f574u) {
		return tall_block177[0];
	} else if (codepoint == 0x1f577u) {
		return tall_block178[0];
	} else if (codepoint == 0x1f5a8u) {
		return tall_block179[0];
	} else if (codepoint == 0x1f5d1u) {
		return tall_block180[0];
	} else if (codepoint == 0x1f6d6u) {
		return tall_block181[0];
	} else if (codepoint == 0x1f860u) {
		return tall_block182[0];
	} else if (codepoint >= 0x1fb00u && codepoint <= 0x1fbcau) {
		return tall_block183[codepoint - 0x1fb00u];
	} else if (codepoint >= 0x1fbf0u && codepoint <= 0x1fbf9u) {
		return tall_block184[codepoint - 0x1fbf0u];
	} else {
		WARN("Codepoint U+%04X (%s) is not covered by this font", codepoint, u8_cp_to_utf8(codepoint));
		return tall_block1[0];
	}
}
