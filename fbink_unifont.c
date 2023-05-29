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

#include "fbink_unifont.h"

static const unsigned char*
    unifont_get_bitmap(uint32_t codepoint)
{
	if (codepoint >= 0x20u && codepoint <= 0x7eu) {
		return unifont_block1[codepoint - 0x20u];
	} else if (codepoint >= 0xa0u && codepoint <= 0xacu) {
		return unifont_block2[codepoint - 0xa0u];
	} else if (codepoint >= 0xaeu && codepoint <= 0x34eu) {
		return unifont_block3[codepoint - 0xaeu];
	} else if (codepoint >= 0x350u && codepoint <= 0x35bu) {
		return unifont_block4[codepoint - 0x350u];
	} else if (codepoint >= 0x363u && codepoint <= 0x377u) {
		return unifont_block5[codepoint - 0x363u];
	} else if (codepoint >= 0x37au && codepoint <= 0x37fu) {
		return unifont_block6[codepoint - 0x37au];
	} else if (codepoint >= 0x384u && codepoint <= 0x38au) {
		return unifont_block7[codepoint - 0x384u];
	} else if (codepoint == 0x38cu) {
		return unifont_block8[0];
	} else if (codepoint >= 0x38eu && codepoint <= 0x3a1u) {
		return unifont_block9[codepoint - 0x38eu];
	} else if (codepoint >= 0x3a3u && codepoint <= 0x487u) {
		return unifont_block10[codepoint - 0x3a3u];
	} else if (codepoint >= 0x48au && codepoint <= 0x529u) {
		return unifont_block11[codepoint - 0x48au];
	} else if (codepoint >= 0x52cu && codepoint <= 0x52fu) {
		return unifont_block12[codepoint - 0x52cu];
	} else if (codepoint >= 0x531u && codepoint <= 0x556u) {
		return unifont_block13[codepoint - 0x531u];
	} else if (codepoint >= 0x559u && codepoint <= 0x58au) {
		return unifont_block14[codepoint - 0x559u];
	} else if (codepoint == 0x58fu) {
		return unifont_block15[0];
	} else if (codepoint >= 0x591u && codepoint <= 0x5c7u) {
		return unifont_block16[codepoint - 0x591u];
	} else if (codepoint >= 0x5d0u && codepoint <= 0x5eau) {
		return unifont_block17[codepoint - 0x5d0u];
	} else if (codepoint >= 0x5efu && codepoint <= 0x5f4u) {
		return unifont_block18[codepoint - 0x5efu];
	} else if (codepoint >= 0x609u && codepoint <= 0x60au) {
		return unifont_block19[codepoint - 0x609u];
	} else if (codepoint >= 0x60cu && codepoint <= 0x60du) {
		return unifont_block20[codepoint - 0x60cu];
	} else if (codepoint == 0x610u) {
		return unifont_block21[0];
	} else if (codepoint >= 0x618u && codepoint <= 0x61bu) {
		return unifont_block22[codepoint - 0x618u];
	} else if (codepoint >= 0x61fu && codepoint <= 0x655u) {
		return unifont_block23[codepoint - 0x61fu];
	} else if (codepoint >= 0x657u && codepoint <= 0x6a9u) {
		return unifont_block24[codepoint - 0x657u];
	} else if (codepoint >= 0x6abu && codepoint <= 0x6d5u) {
		return unifont_block25[codepoint - 0x6abu];
	} else if (codepoint == 0x6dbu) {
		return unifont_block26[0];
	} else if (codepoint >= 0x6dfu && codepoint <= 0x6e1u) {
		return unifont_block27[codepoint - 0x6dfu];
	} else if (codepoint >= 0x6e4u && codepoint <= 0x6e7u) {
		return unifont_block28[codepoint - 0x6e4u];
	} else if (codepoint >= 0x6eeu && codepoint <= 0x6ffu) {
		return unifont_block29[codepoint - 0x6eeu];
	} else if (codepoint >= 0x750u && codepoint <= 0x78fu) {
		return unifont_block30[codepoint - 0x750u];
	} else if (codepoint >= 0x791u && codepoint <= 0x79cu) {
		return unifont_block31[codepoint - 0x791u];
	} else if (codepoint >= 0x7a0u && codepoint <= 0x7b1u) {
		return unifont_block32[codepoint - 0x7a0u];
	} else if (codepoint >= 0x7c0u && codepoint <= 0x7fau) {
		return unifont_block33[codepoint - 0x7c0u];
	} else if (codepoint >= 0x7fdu && codepoint <= 0x7ffu) {
		return unifont_block34[codepoint - 0x7fdu];
	} else if (codepoint >= 0x8a0u && codepoint <= 0x8b4u) {
		return unifont_block35[codepoint - 0x8a0u];
	} else if (codepoint >= 0x8b6u && codepoint <= 0x8c7u) {
		return unifont_block36[codepoint - 0x8b6u];
	} else if (codepoint >= 0x8d3u && codepoint <= 0x8d9u) {
		return unifont_block37[codepoint - 0x8d3u];
	} else if (codepoint >= 0x8e0u && codepoint <= 0x8ffu) {
		return unifont_block38[codepoint - 0x8e0u];
	} else if (codepoint >= 0xe01u && codepoint <= 0xe3au) {
		return unifont_block39[codepoint - 0xe01u];
	} else if (codepoint >= 0xe3fu && codepoint <= 0xe5au) {
		return unifont_block40[codepoint - 0xe3fu];
	} else if (codepoint >= 0xe81u && codepoint <= 0xe82u) {
		return unifont_block41[codepoint - 0xe81u];
	} else if (codepoint == 0xe84u) {
		return unifont_block42[0];
	} else if (codepoint >= 0xe86u && codepoint <= 0xe8au) {
		return unifont_block43[codepoint - 0xe86u];
	} else if (codepoint >= 0xe8cu && codepoint <= 0xea3u) {
		return unifont_block44[codepoint - 0xe8cu];
	} else if (codepoint == 0xea5u) {
		return unifont_block45[0];
	} else if (codepoint >= 0xea7u && codepoint <= 0xebdu) {
		return unifont_block46[codepoint - 0xea7u];
	} else if (codepoint >= 0xec0u && codepoint <= 0xec4u) {
		return unifont_block47[codepoint - 0xec0u];
	} else if (codepoint == 0xec6u) {
		return unifont_block48[0];
	} else if (codepoint >= 0xec8u && codepoint <= 0xecdu) {
		return unifont_block49[codepoint - 0xec8u];
	} else if (codepoint >= 0xed0u && codepoint <= 0xed9u) {
		return unifont_block50[codepoint - 0xed0u];
	} else if (codepoint >= 0xedcu && codepoint <= 0xedfu) {
		return unifont_block51[codepoint - 0xedcu];
	} else if (codepoint >= 0xf00u && codepoint <= 0xf01u) {
		return unifont_block52[codepoint - 0xf00u];
	} else if (codepoint >= 0xf04u && codepoint <= 0xf15u) {
		return unifont_block53[codepoint - 0xf04u];
	} else if (codepoint >= 0xf19u && codepoint <= 0xf39u) {
		return unifont_block54[codepoint - 0xf19u];
	} else if (codepoint >= 0xf3cu && codepoint <= 0xf47u) {
		return unifont_block55[codepoint - 0xf3cu];
	} else if (codepoint >= 0xf49u && codepoint <= 0xf6cu) {
		return unifont_block56[codepoint - 0xf49u];
	} else if (codepoint >= 0xf71u && codepoint <= 0xf87u) {
		return unifont_block57[codepoint - 0xf71u];
	} else if (codepoint >= 0xf89u && codepoint <= 0xf97u) {
		return unifont_block58[codepoint - 0xf89u];
	} else if (codepoint >= 0xf99u && codepoint <= 0xfbcu) {
		return unifont_block59[codepoint - 0xf99u];
	} else if (codepoint >= 0xfbeu && codepoint <= 0xfc4u) {
		return unifont_block60[codepoint - 0xfbeu];
	} else if (codepoint >= 0xfceu && codepoint <= 0xfcfu) {
		return unifont_block61[codepoint - 0xfceu];
	} else if (codepoint >= 0xfd1u && codepoint <= 0xfd4u) {
		return unifont_block62[codepoint - 0xfd1u];
	} else if (codepoint >= 0x10a0u && codepoint <= 0x10c5u) {
		return unifont_block63[codepoint - 0x10a0u];
	} else if (codepoint == 0x10c7u) {
		return unifont_block64[0];
	} else if (codepoint == 0x10cdu) {
		return unifont_block65[0];
	} else if (codepoint >= 0x10d0u && codepoint <= 0x10ffu) {
		return unifont_block66[codepoint - 0x10d0u];
	} else if (codepoint == 0x1343u) {
		return unifont_block67[0];
	} else if (codepoint == 0x1361u) {
		return unifont_block68[0];
	} else if (codepoint >= 0x1390u && codepoint <= 0x1394u) {
		return unifont_block69[codepoint - 0x1390u];
	} else if (codepoint == 0x1396u) {
		return unifont_block70[0];
	} else if (codepoint == 0x1399u) {
		return unifont_block71[0];
	} else if (codepoint >= 0x13a0u && codepoint <= 0x13f5u) {
		return unifont_block72[codepoint - 0x13a0u];
	} else if (codepoint >= 0x13f8u && codepoint <= 0x13fdu) {
		return unifont_block73[codepoint - 0x13f8u];
	} else if (codepoint == 0x1400u) {
		return unifont_block74[0];
	} else if (codepoint >= 0x141eu && codepoint <= 0x142au) {
		return unifont_block75[codepoint - 0x141eu];
	} else if (codepoint >= 0x1433u && codepoint <= 0x1439u) {
		return unifont_block76[codepoint - 0x1433u];
	} else if (codepoint >= 0x1449u && codepoint <= 0x144bu) {
		return unifont_block77[codepoint - 0x1449u];
	} else if (codepoint == 0x1466u) {
		return unifont_block78[0];
	} else if (codepoint >= 0x146bu && codepoint <= 0x14bfu) {
		return unifont_block79[codepoint - 0x146bu];
	} else if (codepoint >= 0x14d0u && codepoint <= 0x14d2u) {
		return unifont_block80[codepoint - 0x14d0u];
	} else if (codepoint >= 0x14eau && codepoint <= 0x150bu) {
		return unifont_block81[codepoint - 0x14eau];
	} else if (codepoint >= 0x1525u && codepoint <= 0x1541u) {
		return unifont_block82[codepoint - 0x1525u];
	} else if (codepoint >= 0x1548u && codepoint <= 0x1552u) {
		return unifont_block83[codepoint - 0x1548u];
	} else if (codepoint == 0x155du) {
		return unifont_block84[0];
	} else if (codepoint == 0x156au) {
		return unifont_block85[0];
	} else if (codepoint >= 0x157bu && codepoint <= 0x157du) {
		return unifont_block86[codepoint - 0x157bu];
	} else if (codepoint >= 0x1586u && codepoint <= 0x1589u) {
		return unifont_block87[codepoint - 0x1586u];
	} else if (codepoint >= 0x1597u && codepoint <= 0x159au) {
		return unifont_block88[codepoint - 0x1597u];
	} else if (codepoint == 0x159fu) {
		return unifont_block89[0];
	} else if (codepoint >= 0x15a6u && codepoint <= 0x15b7u) {
		return unifont_block90[codepoint - 0x15a6u];
	} else if (codepoint == 0x15eeu) {
		return unifont_block91[0];
	} else if (codepoint >= 0x1601u && codepoint <= 0x1603u) {
		return unifont_block92[codepoint - 0x1601u];
	} else if (codepoint >= 0x1646u && codepoint <= 0x1647u) {
		return unifont_block93[codepoint - 0x1646u];
	} else if (codepoint == 0x165au) {
		return unifont_block94[0];
	} else if (codepoint >= 0x166du && codepoint <= 0x166eu) {
		return unifont_block95[codepoint - 0x166du];
	} else if (codepoint >= 0x1677u && codepoint <= 0x167fu) {
		return unifont_block96[codepoint - 0x1677u];
	} else if (codepoint >= 0x16a0u && codepoint <= 0x16dfu) {
		return unifont_block97[codepoint - 0x16a0u];
	} else if (codepoint == 0x16e1u) {
		return unifont_block98[0];
	} else if (codepoint >= 0x16e3u && codepoint <= 0x16f8u) {
		return unifont_block99[codepoint - 0x16e3u];
	} else if (codepoint == 0x1762u) {
		return unifont_block100[0];
	} else if (codepoint == 0x17bbu) {
		return unifont_block101[0];
	} else if (codepoint == 0x17cbu) {
		return unifont_block102[0];
	} else if (codepoint == 0x17d6u) {
		return unifont_block103[0];
	} else if (codepoint == 0x17dcu) {
		return unifont_block104[0];
	} else if (codepoint >= 0x17f0u && codepoint <= 0x17f9u) {
		return unifont_block105[codepoint - 0x17f0u];
	} else if (codepoint >= 0x18b4u && codepoint <= 0x18b6u) {
		return unifont_block106[codepoint - 0x18b4u];
	} else if (codepoint == 0x18b8u) {
		return unifont_block107[0];
	} else if (codepoint == 0x18bau) {
		return unifont_block108[0];
	} else if (codepoint >= 0x18beu && codepoint <= 0x18bfu) {
		return unifont_block109[codepoint - 0x18beu];
	} else if (codepoint >= 0x18c3u && codepoint <= 0x18c5u) {
		return unifont_block110[codepoint - 0x18c3u];
	} else if (codepoint >= 0x18d4u && codepoint <= 0x18dfu) {
		return unifont_block111[codepoint - 0x18d4u];
	} else if (codepoint == 0x18e9u) {
		return unifont_block112[0];
	} else if (codepoint == 0x18ebu) {
		return unifont_block113[0];
	} else if (codepoint >= 0x18f3u && codepoint <= 0x18f5u) {
		return unifont_block114[codepoint - 0x18f3u];
	} else if (codepoint >= 0x1950u && codepoint <= 0x196du) {
		return unifont_block115[codepoint - 0x1950u];
	} else if (codepoint >= 0x1970u && codepoint <= 0x1974u) {
		return unifont_block116[codepoint - 0x1970u];
	} else if (codepoint == 0x1983u) {
		return unifont_block117[0];
	} else if (codepoint == 0x198fu) {
		return unifont_block118[0];
	} else if (codepoint == 0x1991u) {
		return unifont_block119[0];
	} else if (codepoint == 0x199au) {
		return unifont_block120[0];
	} else if (codepoint == 0x199eu) {
		return unifont_block121[0];
	} else if (codepoint == 0x19a1u) {
		return unifont_block122[0];
	} else if (codepoint >= 0x19a3u && codepoint <= 0x19a4u) {
		return unifont_block123[codepoint - 0x19a3u];
	} else if (codepoint == 0x19a7u) {
		return unifont_block124[0];
	} else if (codepoint >= 0x19b0u && codepoint <= 0x19b3u) {
		return unifont_block125[codepoint - 0x19b0u];
	} else if (codepoint >= 0x19b7u && codepoint <= 0x19bau) {
		return unifont_block126[codepoint - 0x19b7u];
	} else if (codepoint == 0x19c1u) {
		return unifont_block127[0];
	} else if (codepoint == 0x19c6u) {
		return unifont_block128[0];
	} else if (codepoint >= 0x19c8u && codepoint <= 0x19c9u) {
		return unifont_block129[codepoint - 0x19c8u];
	} else if (codepoint >= 0x19d0u && codepoint <= 0x19d4u) {
		return unifont_block130[codepoint - 0x19d0u];
	} else if (codepoint == 0x19d6u) {
		return unifont_block131[0];
	} else if (codepoint >= 0x19d8u && codepoint <= 0x19d9u) {
		return unifont_block132[codepoint - 0x19d8u];
	} else if (codepoint >= 0x1ab0u && codepoint <= 0x1ac0u) {
		return unifont_block133[codepoint - 0x1ab0u];
	} else if (codepoint >= 0x1c50u && codepoint <= 0x1c88u) {
		return unifont_block134[codepoint - 0x1c50u];
	} else if (codepoint >= 0x1c90u && codepoint <= 0x1cbau) {
		return unifont_block135[codepoint - 0x1c90u];
	} else if (codepoint >= 0x1cbdu && codepoint <= 0x1cbfu) {
		return unifont_block136[codepoint - 0x1cbdu];
	} else if (codepoint >= 0x1d00u && codepoint <= 0x1d79u) {
		return unifont_block137[codepoint - 0x1d00u];
	} else if (codepoint >= 0x1d7bu && codepoint <= 0x1dccu) {
		return unifont_block138[codepoint - 0x1d7bu];
	} else if (codepoint >= 0x1dceu && codepoint <= 0x1df9u) {
		return unifont_block139[codepoint - 0x1dceu];
	} else if (codepoint == 0x1dfbu) {
		return unifont_block140[0];
	} else if (codepoint >= 0x1dfdu && codepoint <= 0x1f15u) {
		return unifont_block141[codepoint - 0x1dfdu];
	} else if (codepoint >= 0x1f18u && codepoint <= 0x1f1du) {
		return unifont_block142[codepoint - 0x1f18u];
	} else if (codepoint >= 0x1f20u && codepoint <= 0x1f45u) {
		return unifont_block143[codepoint - 0x1f20u];
	} else if (codepoint >= 0x1f48u && codepoint <= 0x1f4du) {
		return unifont_block144[codepoint - 0x1f48u];
	} else if (codepoint >= 0x1f50u && codepoint <= 0x1f57u) {
		return unifont_block145[codepoint - 0x1f50u];
	} else if (codepoint == 0x1f59u) {
		return unifont_block146[0];
	} else if (codepoint == 0x1f5bu) {
		return unifont_block147[0];
	} else if (codepoint == 0x1f5du) {
		return unifont_block148[0];
	} else if (codepoint >= 0x1f5fu && codepoint <= 0x1f7du) {
		return unifont_block149[codepoint - 0x1f5fu];
	} else if (codepoint >= 0x1f80u && codepoint <= 0x1fb4u) {
		return unifont_block150[codepoint - 0x1f80u];
	} else if (codepoint >= 0x1fb6u && codepoint <= 0x1fc4u) {
		return unifont_block151[codepoint - 0x1fb6u];
	} else if (codepoint >= 0x1fc6u && codepoint <= 0x1fd3u) {
		return unifont_block152[codepoint - 0x1fc6u];
	} else if (codepoint >= 0x1fd6u && codepoint <= 0x1fdbu) {
		return unifont_block153[codepoint - 0x1fd6u];
	} else if (codepoint >= 0x1fddu && codepoint <= 0x1fefu) {
		return unifont_block154[codepoint - 0x1fddu];
	} else if (codepoint >= 0x1ff2u && codepoint <= 0x1ff4u) {
		return unifont_block155[codepoint - 0x1ff2u];
	} else if (codepoint >= 0x1ff6u && codepoint <= 0x1ffeu) {
		return unifont_block156[codepoint - 0x1ff6u];
	} else if (codepoint >= 0x2000u && codepoint <= 0x200au) {
		return unifont_block157[codepoint - 0x2000u];
	} else if (codepoint >= 0x2010u && codepoint <= 0x2027u) {
		return unifont_block158[codepoint - 0x2010u];
	} else if (codepoint >= 0x202fu && codepoint <= 0x2056u) {
		return unifont_block159[codepoint - 0x202fu];
	} else if (codepoint >= 0x2058u && codepoint <= 0x205fu) {
		return unifont_block160[codepoint - 0x2058u];
	} else if (codepoint >= 0x2070u && codepoint <= 0x2071u) {
		return unifont_block161[codepoint - 0x2070u];
	} else if (codepoint >= 0x2074u && codepoint <= 0x208eu) {
		return unifont_block162[codepoint - 0x2074u];
	} else if (codepoint >= 0x2090u && codepoint <= 0x209cu) {
		return unifont_block163[codepoint - 0x2090u];
	} else if (codepoint >= 0x20a0u && codepoint <= 0x20b8u) {
		return unifont_block164[codepoint - 0x20a0u];
	} else if (codepoint >= 0x20bau && codepoint <= 0x20bfu) {
		return unifont_block165[codepoint - 0x20bau];
	} else if (codepoint >= 0x20d0u && codepoint <= 0x20dcu) {
		return unifont_block166[codepoint - 0x20d0u];
	} else if (codepoint == 0x20e1u) {
		return unifont_block167[0];
	} else if (codepoint >= 0x20e5u && codepoint <= 0x20e6u) {
		return unifont_block168[codepoint - 0x20e5u];
	} else if (codepoint >= 0x20e8u && codepoint <= 0x20e9u) {
		return unifont_block169[codepoint - 0x20e8u];
	} else if (codepoint >= 0x20ebu && codepoint <= 0x20f0u) {
		return unifont_block170[codepoint - 0x20ebu];
	} else if (codepoint >= 0x2100u && codepoint <= 0x210du) {
		return unifont_block171[codepoint - 0x2100u];
	} else if (codepoint >= 0x2110u && codepoint <= 0x212du) {
		return unifont_block172[codepoint - 0x2110u];
	} else if (codepoint >= 0x212fu && codepoint <= 0x2139u) {
		return unifont_block173[codepoint - 0x212fu];
	} else if (codepoint == 0x213eu) {
		return unifont_block174[0];
	} else if (codepoint >= 0x2141u && codepoint <= 0x2144u) {
		return unifont_block175[codepoint - 0x2141u];
	} else if (codepoint >= 0x214au && codepoint <= 0x214bu) {
		return unifont_block176[codepoint - 0x214au];
	} else if (codepoint >= 0x214du && codepoint <= 0x214eu) {
		return unifont_block177[codepoint - 0x214du];
	} else if (codepoint >= 0x2150u && codepoint <= 0x2181u) {
		return unifont_block178[codepoint - 0x2150u];
	} else if (codepoint >= 0x2183u && codepoint <= 0x2187u) {
		return unifont_block179[codepoint - 0x2183u];
	} else if (codepoint >= 0x2189u && codepoint <= 0x218bu) {
		return unifont_block180[codepoint - 0x2189u];
	} else if (codepoint >= 0x2190u && codepoint <= 0x219bu) {
		return unifont_block181[codepoint - 0x2190u];
	} else if (codepoint >= 0x219eu && codepoint <= 0x21f3u) {
		return unifont_block182[codepoint - 0x219eu];
	} else if (codepoint >= 0x21f5u && codepoint <= 0x21f8u) {
		return unifont_block183[codepoint - 0x21f5u];
	} else if (codepoint >= 0x21fdu && codepoint <= 0x21feu) {
		return unifont_block184[codepoint - 0x21fdu];
	} else if (codepoint >= 0x2200u && codepoint <= 0x22b5u) {
		return unifont_block185[codepoint - 0x2200u];
	} else if (codepoint >= 0x22b9u && codepoint <= 0x22d7u) {
		return unifont_block186[codepoint - 0x22b9u];
	} else if (codepoint >= 0x22dau && codepoint <= 0x22f1u) {
		return unifont_block187[codepoint - 0x22dau];
	} else if (codepoint == 0x22f4u) {
		return unifont_block188[0];
	} else if (codepoint >= 0x22f7u && codepoint <= 0x22f8u) {
		return unifont_block189[codepoint - 0x22f7u];
	} else if (codepoint == 0x22fcu) {
		return unifont_block190[0];
	} else if (codepoint == 0x22feu) {
		return unifont_block191[0];
	} else if (codepoint >= 0x2301u && codepoint <= 0x2315u) {
		return unifont_block192[codepoint - 0x2301u];
	} else if (codepoint >= 0x2317u && codepoint <= 0x2328u) {
		return unifont_block193[codepoint - 0x2317u];
	} else if (codepoint == 0x232bu) {
		return unifont_block194[0];
	} else if (codepoint >= 0x2336u && codepoint <= 0x237au) {
		return unifont_block195[codepoint - 0x2336u];
	} else if (codepoint >= 0x237fu && codepoint <= 0x2380u) {
		return unifont_block196[codepoint - 0x237fu];
	} else if (codepoint >= 0x2395u && codepoint <= 0x2396u) {
		return unifont_block197[codepoint - 0x2395u];
	} else if (codepoint >= 0x239bu && codepoint <= 0x23b1u) {
		return unifont_block198[codepoint - 0x239bu];
	} else if (codepoint >= 0x23b7u && codepoint <= 0x23bfu) {
		return unifont_block199[codepoint - 0x23b7u];
	} else if (codepoint >= 0x23cbu && codepoint <= 0x23ccu) {
		return unifont_block200[codepoint - 0x23cbu];
	} else if (codepoint >= 0x23cfu && codepoint <= 0x23d3u) {
		return unifont_block201[codepoint - 0x23cfu];
	} else if (codepoint == 0x23dau) {
		return unifont_block202[0];
	} else if (codepoint == 0x23e8u) {
		return unifont_block203[0];
	} else if (codepoint >= 0x2422u && codepoint <= 0x2426u) {
		return unifont_block204[codepoint - 0x2422u];
	} else if (codepoint >= 0x2440u && codepoint <= 0x244au) {
		return unifont_block205[codepoint - 0x2440u];
	} else if (codepoint >= 0x2500u && codepoint <= 0x25eeu) {
		return unifont_block206[codepoint - 0x2500u];
	} else if (codepoint >= 0x25f0u && codepoint <= 0x2602u) {
		return unifont_block207[codepoint - 0x25f0u];
	} else if (codepoint >= 0x2604u && codepoint <= 0x260fu) {
		return unifont_block208[codepoint - 0x2604u];
	} else if (codepoint >= 0x2613u && codepoint <= 0x2614u) {
		return unifont_block209[codepoint - 0x2613u];
	} else if (codepoint >= 0x261au && codepoint <= 0x2621u) {
		return unifont_block210[codepoint - 0x261au];
	} else if (codepoint >= 0x2625u && codepoint <= 0x262au) {
		return unifont_block211[codepoint - 0x2625u];
	} else if (codepoint >= 0x262du && codepoint <= 0x262eu) {
		return unifont_block212[codepoint - 0x262du];
	} else if (codepoint >= 0x2638u && codepoint <= 0x2671u) {
		return unifont_block213[codepoint - 0x2638u];
	} else if (codepoint >= 0x2690u && codepoint <= 0x2691u) {
		return unifont_block214[codepoint - 0x2690u];
	} else if (codepoint == 0x26a1u) {
		return unifont_block215[0];
	} else if (codepoint == 0x26a8u) {
		return unifont_block216[0];
	} else if (codepoint >= 0x26aau && codepoint <= 0x26acu) {
		return unifont_block217[codepoint - 0x26aau];
	} else if (codepoint >= 0x26b2u && codepoint <= 0x26b5u) {
		return unifont_block218[codepoint - 0x26b2u];
	} else if (codepoint >= 0x26b7u && codepoint <= 0x26bcu) {
		return unifont_block219[codepoint - 0x26b7u];
	} else if (codepoint == 0x26e2u) {
		return unifont_block220[0];
	} else if (codepoint >= 0x2768u && codepoint <= 0x2775u) {
		return unifont_block221[codepoint - 0x2768u];
	} else if (codepoint == 0x27b0u) {
		return unifont_block222[0];
	} else if (codepoint == 0x27c0u) {
		return unifont_block223[0];
	} else if (codepoint == 0x27c2u) {
		return unifont_block224[0];
	} else if (codepoint >= 0x27c5u && codepoint <= 0x27c7u) {
		return unifont_block225[codepoint - 0x27c5u];
	} else if (codepoint == 0x27cau) {
		return unifont_block226[0];
	} else if (codepoint == 0x27d1u) {
		return unifont_block227[0];
	} else if (codepoint >= 0x27d3u && codepoint <= 0x27d4u) {
		return unifont_block228[codepoint - 0x27d3u];
	} else if (codepoint >= 0x27dfu && codepoint <= 0x27e0u) {
		return unifont_block229[codepoint - 0x27dfu];
	} else if (codepoint >= 0x27e6u && codepoint <= 0x27efu) {
		return unifont_block230[codepoint - 0x27e6u];
	} else if (codepoint >= 0x2800u && codepoint <= 0x28ffu) {
		return unifont_block231[codepoint - 0x2800u];
	} else if (codepoint >= 0x2908u && codepoint <= 0x2909u) {
		return unifont_block232[codepoint - 0x2908u];
	} else if (codepoint >= 0x2912u && codepoint <= 0x2913u) {
		return unifont_block233[codepoint - 0x2912u];
	} else if (codepoint >= 0x2938u && codepoint <= 0x2939u) {
		return unifont_block234[codepoint - 0x2938u];
	} else if (codepoint == 0x2949u) {
		return unifont_block235[0];
	} else if (codepoint >= 0x294cu && codepoint <= 0x294du) {
		return unifont_block236[codepoint - 0x294cu];
	} else if (codepoint == 0x294fu) {
		return unifont_block237[0];
	} else if (codepoint == 0x2951u) {
		return unifont_block238[0];
	} else if (codepoint >= 0x2954u && codepoint <= 0x2955u) {
		return unifont_block239[codepoint - 0x2954u];
	} else if (codepoint >= 0x2958u && codepoint <= 0x2959u) {
		return unifont_block240[codepoint - 0x2958u];
	} else if (codepoint >= 0x295cu && codepoint <= 0x295du) {
		return unifont_block241[codepoint - 0x295cu];
	} else if (codepoint >= 0x2960u && codepoint <= 0x2961u) {
		return unifont_block242[codepoint - 0x2960u];
	} else if (codepoint >= 0x297cu && codepoint <= 0x297du) {
		return unifont_block243[codepoint - 0x297cu];
	} else if (codepoint >= 0x2980u && codepoint <= 0x2992u) {
		return unifont_block244[codepoint - 0x2980u];
	} else if (codepoint >= 0x2997u && codepoint <= 0x299du) {
		return unifont_block245[codepoint - 0x2997u];
	} else if (codepoint >= 0x299fu && codepoint <= 0x29a7u) {
		return unifont_block246[codepoint - 0x299fu];
	} else if (codepoint >= 0x29d1u && codepoint <= 0x29d9u) {
		return unifont_block247[codepoint - 0x29d1u];
	} else if (codepoint >= 0x29dcu && codepoint <= 0x29deu) {
		return unifont_block248[codepoint - 0x29dcu];
	} else if (codepoint == 0x29e1u) {
		return unifont_block249[0];
	} else if (codepoint == 0x29ebu) {
		return unifont_block250[0];
	} else if (codepoint >= 0x29eeu && codepoint <= 0x29f3u) {
		return unifont_block251[codepoint - 0x29eeu];
	} else if (codepoint >= 0x29f5u && codepoint <= 0x29fdu) {
		return unifont_block252[codepoint - 0x29f5u];
	} else if (codepoint == 0x2a0bu) {
		return unifont_block253[0];
	} else if (codepoint >= 0x2a0du && codepoint <= 0x2a1cu) {
		return unifont_block254[codepoint - 0x2a0du];
	} else if (codepoint >= 0x2a1eu && codepoint <= 0x2a1fu) {
		return unifont_block255[codepoint - 0x2a1eu];
	} else if (codepoint >= 0x2a21u && codepoint <= 0x2a2cu) {
		return unifont_block256[codepoint - 0x2a21u];
	} else if (codepoint >= 0x2a2fu && codepoint <= 0x2a32u) {
		return unifont_block257[codepoint - 0x2a2fu];
	} else if (codepoint >= 0x2a3cu && codepoint <= 0x2a4du) {
		return unifont_block258[codepoint - 0x2a3cu];
	} else if (codepoint >= 0x2a66u && codepoint <= 0x2a67u) {
		return unifont_block259[codepoint - 0x2a66u];
	} else if (codepoint >= 0x2a6au && codepoint <= 0x2a73u) {
		return unifont_block260[codepoint - 0x2a6au];
	} else if (codepoint == 0x2a77u) {
		return unifont_block261[0];
	} else if (codepoint >= 0x2a8bu && codepoint <= 0x2a8cu) {
		return unifont_block262[codepoint - 0x2a8bu];
	} else if (codepoint >= 0x2a8fu && codepoint <= 0x2a94u) {
		return unifont_block263[codepoint - 0x2a8fu];
	} else if (codepoint >= 0x2abfu && codepoint <= 0x2accu) {
		return unifont_block264[codepoint - 0x2abfu];
	} else if (codepoint >= 0x2ad3u && codepoint <= 0x2ad6u) {
		return unifont_block265[codepoint - 0x2ad3u];
	} else if (codepoint == 0x2adeu) {
		return unifont_block266[0];
	} else if (codepoint >= 0x2aeeu && codepoint <= 0x2af2u) {
		return unifont_block267[codepoint - 0x2aeeu];
	} else if (codepoint == 0x2af6u) {
		return unifont_block268[0];
	} else if (codepoint >= 0x2afeu && codepoint <= 0x2affu) {
		return unifont_block269[codepoint - 0x2afeu];
	} else if (codepoint >= 0x2b06u && codepoint <= 0x2b07u) {
		return unifont_block270[codepoint - 0x2b06u];
	} else if (codepoint == 0x2b0du) {
		return unifont_block271[0];
	} else if (codepoint >= 0x2b1du && codepoint <= 0x2b1eu) {
		return unifont_block272[codepoint - 0x2b1du];
	} else if (codepoint >= 0x2b25u && codepoint <= 0x2b2bu) {
		return unifont_block273[codepoint - 0x2b25u];
	} else if (codepoint >= 0x2b2eu && codepoint <= 0x2b2fu) {
		return unifont_block274[codepoint - 0x2b2eu];
	} else if (codepoint == 0x2b31u) {
		return unifont_block275[0];
	} else if (codepoint >= 0x2b4eu && codepoint <= 0x2b4fu) {
		return unifont_block276[codepoint - 0x2b4eu];
	} else if (codepoint == 0x2bc9u) {
		return unifont_block277[0];
	} else if (codepoint >= 0x2bffu && codepoint <= 0x2c0eu) {
		return unifont_block278[codepoint - 0x2bffu];
	} else if (codepoint >= 0x2c10u && codepoint <= 0x2c1eu) {
		return unifont_block279[codepoint - 0x2c10u];
	} else if (codepoint >= 0x2c20u && codepoint <= 0x2c26u) {
		return unifont_block280[codepoint - 0x2c20u];
	} else if (codepoint >= 0x2c2au && codepoint <= 0x2c2eu) {
		return unifont_block281[codepoint - 0x2c2au];
	} else if (codepoint >= 0x2c30u && codepoint <= 0x2c3eu) {
		return unifont_block282[codepoint - 0x2c30u];
	} else if (codepoint >= 0x2c40u && codepoint <= 0x2c4eu) {
		return unifont_block283[codepoint - 0x2c40u];
	} else if (codepoint >= 0x2c50u && codepoint <= 0x2c56u) {
		return unifont_block284[codepoint - 0x2c50u];
	} else if (codepoint >= 0x2c5au && codepoint <= 0x2c5eu) {
		return unifont_block285[codepoint - 0x2c5au];
	} else if (codepoint >= 0x2c60u && codepoint <= 0x2ccdu) {
		return unifont_block286[codepoint - 0x2c60u];
	} else if (codepoint >= 0x2cd0u && codepoint <= 0x2ce6u) {
		return unifont_block287[codepoint - 0x2cd0u];
	} else if (codepoint >= 0x2ce8u && codepoint <= 0x2ce9u) {
		return unifont_block288[codepoint - 0x2ce8u];
	} else if (codepoint >= 0x2cebu && codepoint <= 0x2ceeu) {
		return unifont_block289[codepoint - 0x2cebu];
	} else if (codepoint >= 0x2cf0u && codepoint <= 0x2cf3u) {
		return unifont_block290[codepoint - 0x2cf0u];
	} else if (codepoint >= 0x2cf9u && codepoint <= 0x2d04u) {
		return unifont_block291[codepoint - 0x2cf9u];
	} else if (codepoint == 0x2d06u) {
		return unifont_block292[0];
	} else if (codepoint >= 0x2d08u && codepoint <= 0x2d09u) {
		return unifont_block293[codepoint - 0x2d08u];
	} else if (codepoint >= 0x2d0bu && codepoint <= 0x2d0cu) {
		return unifont_block294[codepoint - 0x2d0bu];
	} else if (codepoint >= 0x2d0eu && codepoint <= 0x2d0fu) {
		return unifont_block295[codepoint - 0x2d0eu];
	} else if (codepoint >= 0x2d11u && codepoint <= 0x2d12u) {
		return unifont_block296[codepoint - 0x2d11u];
	} else if (codepoint >= 0x2d15u && codepoint <= 0x2d1au) {
		return unifont_block297[codepoint - 0x2d15u];
	} else if (codepoint >= 0x2d1du && codepoint <= 0x2d1fu) {
		return unifont_block298[codepoint - 0x2d1du];
	} else if (codepoint >= 0x2d21u && codepoint <= 0x2d24u) {
		return unifont_block299[codepoint - 0x2d21u];
	} else if (codepoint == 0x2d27u) {
		return unifont_block300[0];
	} else if (codepoint == 0x2d2du) {
		return unifont_block301[0];
	} else if (codepoint >= 0x2d30u && codepoint <= 0x2d47u) {
		return unifont_block302[codepoint - 0x2d30u];
	} else if (codepoint >= 0x2d49u && codepoint <= 0x2d67u) {
		return unifont_block303[codepoint - 0x2d49u];
	} else if (codepoint >= 0x2d6fu && codepoint <= 0x2d70u) {
		return unifont_block304[codepoint - 0x2d6fu];
	} else if (codepoint == 0x2d7fu) {
		return unifont_block305[0];
	} else if (codepoint >= 0x2de0u && codepoint <= 0x2e0du) {
		return unifont_block306[codepoint - 0x2de0u];
	} else if (codepoint == 0x2e12u) {
		return unifont_block307[0];
	} else if (codepoint >= 0x2e16u && codepoint <= 0x2e39u) {
		return unifont_block308[codepoint - 0x2e16u];
	} else if (codepoint >= 0x2e3cu && codepoint <= 0x2e42u) {
		return unifont_block309[codepoint - 0x2e3cu];
	} else if (codepoint >= 0x2e44u && codepoint <= 0x2e4fu) {
		return unifont_block310[codepoint - 0x2e44u];
	} else if (codepoint == 0x2e52u) {
		return unifont_block311[0];
	} else if (codepoint == 0x303fu) {
		return unifont_block312[0];
	} else if (codepoint >= 0xa4d0u && codepoint <= 0xa4ffu) {
		return unifont_block313[codepoint - 0xa4d0u];
	} else if (codepoint >= 0xa640u && codepoint <= 0xa64bu) {
		return unifont_block314[codepoint - 0xa640u];
	} else if (codepoint >= 0xa64eu && codepoint <= 0xa64fu) {
		return unifont_block315[codepoint - 0xa64eu];
	} else if (codepoint >= 0xa652u && codepoint <= 0xa65du) {
		return unifont_block316[codepoint - 0xa652u];
	} else if (codepoint >= 0xa65fu && codepoint <= 0xa661u) {
		return unifont_block317[codepoint - 0xa65fu];
	} else if (codepoint >= 0xa668u && codepoint <= 0xa66bu) {
		return unifont_block318[codepoint - 0xa668u];
	} else if (codepoint == 0xa66fu) {
		return unifont_block319[0];
	} else if (codepoint == 0xa671u) {
		return unifont_block320[0];
	} else if (codepoint >= 0xa673u && codepoint <= 0xa683u) {
		return unifont_block321[codepoint - 0xa673u];
	} else if (codepoint >= 0xa686u && codepoint <= 0xa697u) {
		return unifont_block322[codepoint - 0xa686u];
	} else if (codepoint >= 0xa69au && codepoint <= 0xa6f7u) {
		return unifont_block323[codepoint - 0xa69au];
	} else if (codepoint >= 0xa700u && codepoint <= 0xa727u) {
		return unifont_block324[codepoint - 0xa700u];
	} else if (codepoint >= 0xa72au && codepoint <= 0xa731u) {
		return unifont_block325[codepoint - 0xa72au];
	} else if (codepoint >= 0xa73eu && codepoint <= 0xa74du) {
		return unifont_block326[codepoint - 0xa73eu];
	} else if (codepoint >= 0xa750u && codepoint <= 0xa757u) {
		return unifont_block327[codepoint - 0xa750u];
	} else if (codepoint >= 0xa75au && codepoint <= 0xa770u) {
		return unifont_block328[codepoint - 0xa75au];
	} else if (codepoint >= 0xa778u && codepoint <= 0xa7bfu) {
		return unifont_block329[codepoint - 0xa778u];
	} else if (codepoint >= 0xa7c4u && codepoint <= 0xa7cau) {
		return unifont_block330[codepoint - 0xa7c4u];
	} else if (codepoint >= 0xa7f5u && codepoint <= 0xa7feu) {
		return unifont_block331[codepoint - 0xa7f5u];
	} else if (codepoint >= 0xa828u && codepoint <= 0xa829u) {
		return unifont_block332[codepoint - 0xa828u];
	} else if (codepoint >= 0xab30u && codepoint <= 0xab65u) {
		return unifont_block333[codepoint - 0xab30u];
	} else if (codepoint >= 0xab68u && codepoint <= 0xab6bu) {
		return unifont_block334[codepoint - 0xab68u];
	} else if (codepoint >= 0xab70u && codepoint <= 0xabbfu) {
		return unifont_block335[codepoint - 0xab70u];
	} else if (codepoint >= 0xfb00u && codepoint <= 0xfb06u) {
		return unifont_block336[codepoint - 0xfb00u];
	} else if (codepoint >= 0xfb13u && codepoint <= 0xfb17u) {
		return unifont_block337[codepoint - 0xfb13u];
	} else if (codepoint >= 0xfb1du && codepoint <= 0xfb20u) {
		return unifont_block338[codepoint - 0xfb1du];
	} else if (codepoint >= 0xfb29u && codepoint <= 0xfb36u) {
		return unifont_block339[codepoint - 0xfb29u];
	} else if (codepoint >= 0xfb38u && codepoint <= 0xfb3cu) {
		return unifont_block340[codepoint - 0xfb38u];
	} else if (codepoint == 0xfb3eu) {
		return unifont_block341[0];
	} else if (codepoint >= 0xfb40u && codepoint <= 0xfb41u) {
		return unifont_block342[codepoint - 0xfb40u];
	} else if (codepoint >= 0xfb43u && codepoint <= 0xfb44u) {
		return unifont_block343[codepoint - 0xfb43u];
	} else if (codepoint >= 0xfb46u && codepoint <= 0xfbc1u) {
		return unifont_block344[codepoint - 0xfb46u];
	} else if (codepoint >= 0xfbd3u && codepoint <= 0xfc1eu) {
		return unifont_block345[codepoint - 0xfbd3u];
	} else if (codepoint == 0xfc20u) {
		return unifont_block346[0];
	} else if (codepoint >= 0xfc22u && codepoint <= 0xfc24u) {
		return unifont_block347[codepoint - 0xfc22u];
	} else if (codepoint >= 0xfc26u && codepoint <= 0xfc3cu) {
		return unifont_block348[codepoint - 0xfc26u];
	} else if (codepoint >= 0xfc3fu && codepoint <= 0xfcacu) {
		return unifont_block349[codepoint - 0xfc3fu];
	} else if (codepoint == 0xfcb0u) {
		return unifont_block350[0];
	} else if (codepoint >= 0xfcb8u && codepoint <= 0xfce6u) {
		return unifont_block351[codepoint - 0xfcb8u];
	} else if (codepoint >= 0xfcebu && codepoint <= 0xfcfau) {
		return unifont_block352[codepoint - 0xfcebu];
	} else if (codepoint >= 0xfcffu && codepoint <= 0xfd16u) {
		return unifont_block353[codepoint - 0xfcffu];
	} else if (codepoint >= 0xfd21u && codepoint <= 0xfd2cu) {
		return unifont_block354[codepoint - 0xfd21u];
	} else if (codepoint == 0xfd33u) {
		return unifont_block355[0];
	} else if (codepoint >= 0xfd3au && codepoint <= 0xfd3du) {
		return unifont_block356[codepoint - 0xfd3au];
	} else if (codepoint >= 0xfe20u && codepoint <= 0xfe2fu) {
		return unifont_block357[codepoint - 0xfe20u];
	} else if (codepoint >= 0xfe70u && codepoint <= 0xfe74u) {
		return unifont_block358[codepoint - 0xfe70u];
	} else if (codepoint >= 0xfe76u && codepoint <= 0xfefcu) {
		return unifont_block359[codepoint - 0xfe76u];
	} else if (codepoint >= 0xff61u && codepoint <= 0xffbeu) {
		return unifont_block360[codepoint - 0xff61u];
	} else if (codepoint >= 0xffc2u && codepoint <= 0xffc7u) {
		return unifont_block361[codepoint - 0xffc2u];
	} else if (codepoint >= 0xffcau && codepoint <= 0xffcfu) {
		return unifont_block362[codepoint - 0xffcau];
	} else if (codepoint >= 0xffd2u && codepoint <= 0xffd7u) {
		return unifont_block363[codepoint - 0xffd2u];
	} else if (codepoint >= 0xffdau && codepoint <= 0xffdcu) {
		return unifont_block364[codepoint - 0xffdau];
	} else if (codepoint >= 0xffe8u && codepoint <= 0xffeeu) {
		return unifont_block365[codepoint - 0xffe8u];
	} else if (codepoint == 0xfffdu) {
		return unifont_block366[0];
	} else {
		WARN("Codepoint U+%04X (%s) is not covered by this font", codepoint, u8_cp_to_utf8(codepoint));
		return unifont_block1[0];
	}
}

static const uint16_t*
    unifontdw_get_bitmap(uint32_t codepoint)
{
	if (codepoint <= 0x1fu) {
		return unifontdw_block1[codepoint];
	} else if (codepoint >= 0x7fu && codepoint <= 0x9fu) {
		return unifontdw_block2[codepoint - 0x7fu];
	} else if (codepoint == 0xadu) {
		return unifontdw_block3[0];
	} else if (codepoint == 0x34fu) {
		return unifontdw_block4[0];
	} else if (codepoint >= 0x35cu && codepoint <= 0x362u) {
		return unifontdw_block5[codepoint - 0x35cu];
	} else if (codepoint >= 0x378u && codepoint <= 0x379u) {
		return unifontdw_block6[codepoint - 0x378u];
	} else if (codepoint >= 0x380u && codepoint <= 0x383u) {
		return unifontdw_block7[codepoint - 0x380u];
	} else if (codepoint == 0x38bu) {
		return unifontdw_block8[0];
	} else if (codepoint == 0x38du) {
		return unifontdw_block9[0];
	} else if (codepoint == 0x3a2u) {
		return unifontdw_block10[0];
	} else if (codepoint >= 0x488u && codepoint <= 0x489u) {
		return unifontdw_block11[codepoint - 0x488u];
	} else if (codepoint >= 0x52au && codepoint <= 0x52bu) {
		return unifontdw_block12[codepoint - 0x52au];
	} else if (codepoint == 0x530u) {
		return unifontdw_block13[0];
	} else if (codepoint >= 0x557u && codepoint <= 0x558u) {
		return unifontdw_block14[codepoint - 0x557u];
	} else if (codepoint >= 0x58bu && codepoint <= 0x58eu) {
		return unifontdw_block15[codepoint - 0x58bu];
	} else if (codepoint == 0x590u) {
		return unifontdw_block16[0];
	} else if (codepoint >= 0x5c8u && codepoint <= 0x5cfu) {
		return unifontdw_block17[codepoint - 0x5c8u];
	} else if (codepoint >= 0x5ebu && codepoint <= 0x5eeu) {
		return unifontdw_block18[codepoint - 0x5ebu];
	} else if (codepoint >= 0x5f5u && codepoint <= 0x608u) {
		return unifontdw_block19[codepoint - 0x5f5u];
	} else if (codepoint == 0x60bu) {
		return unifontdw_block20[0];
	} else if (codepoint >= 0x60eu && codepoint <= 0x60fu) {
		return unifontdw_block21[codepoint - 0x60eu];
	} else if (codepoint >= 0x611u && codepoint <= 0x617u) {
		return unifontdw_block22[codepoint - 0x611u];
	} else if (codepoint >= 0x61cu && codepoint <= 0x61eu) {
		return unifontdw_block23[codepoint - 0x61cu];
	} else if (codepoint == 0x656u) {
		return unifontdw_block24[0];
	} else if (codepoint == 0x6aau) {
		return unifontdw_block25[0];
	} else if (codepoint >= 0x6d6u && codepoint <= 0x6dau) {
		return unifontdw_block26[codepoint - 0x6d6u];
	} else if (codepoint >= 0x6dcu && codepoint <= 0x6deu) {
		return unifontdw_block27[codepoint - 0x6dcu];
	} else if (codepoint >= 0x6e2u && codepoint <= 0x6e3u) {
		return unifontdw_block28[codepoint - 0x6e2u];
	} else if (codepoint >= 0x6e8u && codepoint <= 0x6edu) {
		return unifontdw_block29[codepoint - 0x6e8u];
	} else if (codepoint >= 0x700u && codepoint <= 0x74fu) {
		return unifontdw_block30[codepoint - 0x700u];
	} else if (codepoint == 0x790u) {
		return unifontdw_block31[0];
	} else if (codepoint >= 0x79du && codepoint <= 0x79fu) {
		return unifontdw_block32[codepoint - 0x79du];
	} else if (codepoint >= 0x7b2u && codepoint <= 0x7bfu) {
		return unifontdw_block33[codepoint - 0x7b2u];
	} else if (codepoint >= 0x7fbu && codepoint <= 0x7fcu) {
		return unifontdw_block34[codepoint - 0x7fbu];
	} else if (codepoint >= 0x800u && codepoint <= 0x89fu) {
		return unifontdw_block35[codepoint - 0x800u];
	} else if (codepoint == 0x8b5u) {
		return unifontdw_block36[0];
	} else if (codepoint >= 0x8c8u && codepoint <= 0x8d2u) {
		return unifontdw_block37[codepoint - 0x8c8u];
	} else if (codepoint >= 0x8dau && codepoint <= 0x8dfu) {
		return unifontdw_block38[codepoint - 0x8dau];
	} else if (codepoint >= 0x900u && codepoint <= 0xe00u) {
		return unifontdw_block39[codepoint - 0x900u];
	} else if (codepoint >= 0xe3bu && codepoint <= 0xe3eu) {
		return unifontdw_block40[codepoint - 0xe3bu];
	} else if (codepoint >= 0xe5bu && codepoint <= 0xe80u) {
		return unifontdw_block41[codepoint - 0xe5bu];
	} else if (codepoint == 0xe83u) {
		return unifontdw_block42[0];
	} else if (codepoint == 0xe85u) {
		return unifontdw_block43[0];
	} else if (codepoint == 0xe8bu) {
		return unifontdw_block44[0];
	} else if (codepoint == 0xea4u) {
		return unifontdw_block45[0];
	} else if (codepoint == 0xea6u) {
		return unifontdw_block46[0];
	} else if (codepoint >= 0xebeu && codepoint <= 0xebfu) {
		return unifontdw_block47[codepoint - 0xebeu];
	} else if (codepoint == 0xec5u) {
		return unifontdw_block48[0];
	} else if (codepoint == 0xec7u) {
		return unifontdw_block49[0];
	} else if (codepoint >= 0xeceu && codepoint <= 0xecfu) {
		return unifontdw_block50[codepoint - 0xeceu];
	} else if (codepoint >= 0xedau && codepoint <= 0xedbu) {
		return unifontdw_block51[codepoint - 0xedau];
	} else if (codepoint >= 0xee0u && codepoint <= 0xeffu) {
		return unifontdw_block52[codepoint - 0xee0u];
	} else if (codepoint >= 0xf02u && codepoint <= 0xf03u) {
		return unifontdw_block53[codepoint - 0xf02u];
	} else if (codepoint >= 0xf16u && codepoint <= 0xf18u) {
		return unifontdw_block54[codepoint - 0xf16u];
	} else if (codepoint >= 0xf3au && codepoint <= 0xf3bu) {
		return unifontdw_block55[codepoint - 0xf3au];
	} else if (codepoint == 0xf48u) {
		return unifontdw_block56[0];
	} else if (codepoint >= 0xf6du && codepoint <= 0xf70u) {
		return unifontdw_block57[codepoint - 0xf6du];
	} else if (codepoint == 0xf88u) {
		return unifontdw_block58[0];
	} else if (codepoint == 0xf98u) {
		return unifontdw_block59[0];
	} else if (codepoint == 0xfbdu) {
		return unifontdw_block60[0];
	} else if (codepoint >= 0xfc5u && codepoint <= 0xfcdu) {
		return unifontdw_block61[codepoint - 0xfc5u];
	} else if (codepoint == 0xfd0u) {
		return unifontdw_block62[0];
	} else if (codepoint >= 0xfd5u && codepoint <= 0x109fu) {
		return unifontdw_block63[codepoint - 0xfd5u];
	} else if (codepoint == 0x10c6u) {
		return unifontdw_block64[0];
	} else if (codepoint >= 0x10c8u && codepoint <= 0x10ccu) {
		return unifontdw_block65[codepoint - 0x10c8u];
	} else if (codepoint >= 0x10ceu && codepoint <= 0x10cfu) {
		return unifontdw_block66[codepoint - 0x10ceu];
	} else if (codepoint >= 0x1100u && codepoint <= 0x1342u) {
		return unifontdw_block67[codepoint - 0x1100u];
	} else if (codepoint >= 0x1344u && codepoint <= 0x1360u) {
		return unifontdw_block68[codepoint - 0x1344u];
	} else if (codepoint >= 0x1362u && codepoint <= 0x138fu) {
		return unifontdw_block69[codepoint - 0x1362u];
	} else if (codepoint == 0x1395u) {
		return unifontdw_block70[0];
	} else if (codepoint >= 0x1397u && codepoint <= 0x1398u) {
		return unifontdw_block71[codepoint - 0x1397u];
	} else if (codepoint >= 0x139au && codepoint <= 0x139fu) {
		return unifontdw_block72[codepoint - 0x139au];
	} else if (codepoint >= 0x13f6u && codepoint <= 0x13f7u) {
		return unifontdw_block73[codepoint - 0x13f6u];
	} else if (codepoint >= 0x13feu && codepoint <= 0x13ffu) {
		return unifontdw_block74[codepoint - 0x13feu];
	} else if (codepoint >= 0x1401u && codepoint <= 0x141du) {
		return unifontdw_block75[codepoint - 0x1401u];
	} else if (codepoint >= 0x142bu && codepoint <= 0x1432u) {
		return unifontdw_block76[codepoint - 0x142bu];
	} else if (codepoint >= 0x143au && codepoint <= 0x1448u) {
		return unifontdw_block77[codepoint - 0x143au];
	} else if (codepoint >= 0x144cu && codepoint <= 0x1465u) {
		return unifontdw_block78[codepoint - 0x144cu];
	} else if (codepoint >= 0x1467u && codepoint <= 0x146au) {
		return unifontdw_block79[codepoint - 0x1467u];
	} else if (codepoint >= 0x14c0u && codepoint <= 0x14cfu) {
		return unifontdw_block80[codepoint - 0x14c0u];
	} else if (codepoint >= 0x14d3u && codepoint <= 0x14e9u) {
		return unifontdw_block81[codepoint - 0x14d3u];
	} else if (codepoint >= 0x150cu && codepoint <= 0x1524u) {
		return unifontdw_block82[codepoint - 0x150cu];
	} else if (codepoint >= 0x1542u && codepoint <= 0x1547u) {
		return unifontdw_block83[codepoint - 0x1542u];
	} else if (codepoint >= 0x1553u && codepoint <= 0x155cu) {
		return unifontdw_block84[codepoint - 0x1553u];
	} else if (codepoint >= 0x155eu && codepoint <= 0x1569u) {
		return unifontdw_block85[codepoint - 0x155eu];
	} else if (codepoint >= 0x156bu && codepoint <= 0x157au) {
		return unifontdw_block86[codepoint - 0x156bu];
	} else if (codepoint >= 0x157eu && codepoint <= 0x1585u) {
		return unifontdw_block87[codepoint - 0x157eu];
	} else if (codepoint >= 0x158au && codepoint <= 0x1596u) {
		return unifontdw_block88[codepoint - 0x158au];
	} else if (codepoint >= 0x159bu && codepoint <= 0x159eu) {
		return unifontdw_block89[codepoint - 0x159bu];
	} else if (codepoint >= 0x15a0u && codepoint <= 0x15a5u) {
		return unifontdw_block90[codepoint - 0x15a0u];
	} else if (codepoint >= 0x15b8u && codepoint <= 0x15edu) {
		return unifontdw_block91[codepoint - 0x15b8u];
	} else if (codepoint >= 0x15efu && codepoint <= 0x1600u) {
		return unifontdw_block92[codepoint - 0x15efu];
	} else if (codepoint >= 0x1604u && codepoint <= 0x1645u) {
		return unifontdw_block93[codepoint - 0x1604u];
	} else if (codepoint >= 0x1648u && codepoint <= 0x1659u) {
		return unifontdw_block94[codepoint - 0x1648u];
	} else if (codepoint >= 0x165bu && codepoint <= 0x166cu) {
		return unifontdw_block95[codepoint - 0x165bu];
	} else if (codepoint >= 0x166fu && codepoint <= 0x1676u) {
		return unifontdw_block96[codepoint - 0x166fu];
	} else if (codepoint >= 0x1680u && codepoint <= 0x169fu) {
		return unifontdw_block97[codepoint - 0x1680u];
	} else if (codepoint == 0x16e0u) {
		return unifontdw_block98[0];
	} else if (codepoint == 0x16e2u) {
		return unifontdw_block99[0];
	} else if (codepoint >= 0x16f9u && codepoint <= 0x1761u) {
		return unifontdw_block100[codepoint - 0x16f9u];
	} else if (codepoint >= 0x1763u && codepoint <= 0x17bau) {
		return unifontdw_block101[codepoint - 0x1763u];
	} else if (codepoint >= 0x17bcu && codepoint <= 0x17cau) {
		return unifontdw_block102[codepoint - 0x17bcu];
	} else if (codepoint >= 0x17ccu && codepoint <= 0x17d5u) {
		return unifontdw_block103[codepoint - 0x17ccu];
	} else if (codepoint >= 0x17d7u && codepoint <= 0x17dbu) {
		return unifontdw_block104[codepoint - 0x17d7u];
	} else if (codepoint >= 0x17ddu && codepoint <= 0x17efu) {
		return unifontdw_block105[codepoint - 0x17ddu];
	} else if (codepoint >= 0x17fau && codepoint <= 0x18b3u) {
		return unifontdw_block106[codepoint - 0x17fau];
	} else if (codepoint == 0x18b7u) {
		return unifontdw_block107[0];
	} else if (codepoint == 0x18b9u) {
		return unifontdw_block108[0];
	} else if (codepoint >= 0x18bbu && codepoint <= 0x18bdu) {
		return unifontdw_block109[codepoint - 0x18bbu];
	} else if (codepoint >= 0x18c0u && codepoint <= 0x18c2u) {
		return unifontdw_block110[codepoint - 0x18c0u];
	} else if (codepoint >= 0x18c6u && codepoint <= 0x18d3u) {
		return unifontdw_block111[codepoint - 0x18c6u];
	} else if (codepoint >= 0x18e0u && codepoint <= 0x18e8u) {
		return unifontdw_block112[codepoint - 0x18e0u];
	} else if (codepoint == 0x18eau) {
		return unifontdw_block113[0];
	} else if (codepoint >= 0x18ecu && codepoint <= 0x18f2u) {
		return unifontdw_block114[codepoint - 0x18ecu];
	} else if (codepoint >= 0x18f6u && codepoint <= 0x194fu) {
		return unifontdw_block115[codepoint - 0x18f6u];
	} else if (codepoint >= 0x196eu && codepoint <= 0x196fu) {
		return unifontdw_block116[codepoint - 0x196eu];
	} else if (codepoint >= 0x1975u && codepoint <= 0x1982u) {
		return unifontdw_block117[codepoint - 0x1975u];
	} else if (codepoint >= 0x1984u && codepoint <= 0x198eu) {
		return unifontdw_block118[codepoint - 0x1984u];
	} else if (codepoint == 0x1990u) {
		return unifontdw_block119[0];
	} else if (codepoint >= 0x1992u && codepoint <= 0x1999u) {
		return unifontdw_block120[codepoint - 0x1992u];
	} else if (codepoint >= 0x199bu && codepoint <= 0x199du) {
		return unifontdw_block121[codepoint - 0x199bu];
	} else if (codepoint >= 0x199fu && codepoint <= 0x19a0u) {
		return unifontdw_block122[codepoint - 0x199fu];
	} else if (codepoint == 0x19a2u) {
		return unifontdw_block123[0];
	} else if (codepoint >= 0x19a5u && codepoint <= 0x19a6u) {
		return unifontdw_block124[codepoint - 0x19a5u];
	} else if (codepoint >= 0x19a8u && codepoint <= 0x19afu) {
		return unifontdw_block125[codepoint - 0x19a8u];
	} else if (codepoint >= 0x19b4u && codepoint <= 0x19b6u) {
		return unifontdw_block126[codepoint - 0x19b4u];
	} else if (codepoint >= 0x19bbu && codepoint <= 0x19c0u) {
		return unifontdw_block127[codepoint - 0x19bbu];
	} else if (codepoint >= 0x19c2u && codepoint <= 0x19c5u) {
		return unifontdw_block128[codepoint - 0x19c2u];
	} else if (codepoint == 0x19c7u) {
		return unifontdw_block129[0];
	} else if (codepoint >= 0x19cau && codepoint <= 0x19cfu) {
		return unifontdw_block130[codepoint - 0x19cau];
	} else if (codepoint == 0x19d5u) {
		return unifontdw_block131[0];
	} else if (codepoint == 0x19d7u) {
		return unifontdw_block132[0];
	} else if (codepoint >= 0x19dau && codepoint <= 0x1aafu) {
		return unifontdw_block133[codepoint - 0x19dau];
	} else if (codepoint >= 0x1ac1u && codepoint <= 0x1c4fu) {
		return unifontdw_block134[codepoint - 0x1ac1u];
	} else if (codepoint >= 0x1c89u && codepoint <= 0x1c8fu) {
		return unifontdw_block135[codepoint - 0x1c89u];
	} else if (codepoint >= 0x1cbbu && codepoint <= 0x1cbcu) {
		return unifontdw_block136[codepoint - 0x1cbbu];
	} else if (codepoint >= 0x1cc0u && codepoint <= 0x1cffu) {
		return unifontdw_block137[codepoint - 0x1cc0u];
	} else if (codepoint == 0x1d7au) {
		return unifontdw_block138[0];
	} else if (codepoint == 0x1dcdu) {
		return unifontdw_block139[0];
	} else if (codepoint == 0x1dfau) {
		return unifontdw_block140[0];
	} else if (codepoint == 0x1dfcu) {
		return unifontdw_block141[0];
	} else if (codepoint >= 0x1f16u && codepoint <= 0x1f17u) {
		return unifontdw_block142[codepoint - 0x1f16u];
	} else if (codepoint >= 0x1f1eu && codepoint <= 0x1f1fu) {
		return unifontdw_block143[codepoint - 0x1f1eu];
	} else if (codepoint >= 0x1f46u && codepoint <= 0x1f47u) {
		return unifontdw_block144[codepoint - 0x1f46u];
	} else if (codepoint >= 0x1f4eu && codepoint <= 0x1f4fu) {
		return unifontdw_block145[codepoint - 0x1f4eu];
	} else if (codepoint == 0x1f58u) {
		return unifontdw_block146[0];
	} else if (codepoint == 0x1f5au) {
		return unifontdw_block147[0];
	} else if (codepoint == 0x1f5cu) {
		return unifontdw_block148[0];
	} else if (codepoint == 0x1f5eu) {
		return unifontdw_block149[0];
	} else if (codepoint >= 0x1f7eu && codepoint <= 0x1f7fu) {
		return unifontdw_block150[codepoint - 0x1f7eu];
	} else if (codepoint == 0x1fb5u) {
		return unifontdw_block151[0];
	} else if (codepoint == 0x1fc5u) {
		return unifontdw_block152[0];
	} else if (codepoint >= 0x1fd4u && codepoint <= 0x1fd5u) {
		return unifontdw_block153[codepoint - 0x1fd4u];
	} else if (codepoint == 0x1fdcu) {
		return unifontdw_block154[0];
	} else if (codepoint >= 0x1ff0u && codepoint <= 0x1ff1u) {
		return unifontdw_block155[codepoint - 0x1ff0u];
	} else if (codepoint == 0x1ff5u) {
		return unifontdw_block156[0];
	} else if (codepoint == 0x1fffu) {
		return unifontdw_block157[0];
	} else if (codepoint >= 0x200bu && codepoint <= 0x200fu) {
		return unifontdw_block158[codepoint - 0x200bu];
	} else if (codepoint >= 0x2028u && codepoint <= 0x202eu) {
		return unifontdw_block159[codepoint - 0x2028u];
	} else if (codepoint == 0x2057u) {
		return unifontdw_block160[0];
	} else if (codepoint >= 0x2060u && codepoint <= 0x206fu) {
		return unifontdw_block161[codepoint - 0x2060u];
	} else if (codepoint >= 0x2072u && codepoint <= 0x2073u) {
		return unifontdw_block162[codepoint - 0x2072u];
	} else if (codepoint == 0x208fu) {
		return unifontdw_block163[0];
	} else if (codepoint >= 0x209du && codepoint <= 0x209fu) {
		return unifontdw_block164[codepoint - 0x209du];
	} else if (codepoint == 0x20b9u) {
		return unifontdw_block165[0];
	} else if (codepoint >= 0x20c0u && codepoint <= 0x20cfu) {
		return unifontdw_block166[codepoint - 0x20c0u];
	} else if (codepoint >= 0x20ddu && codepoint <= 0x20e0u) {
		return unifontdw_block167[codepoint - 0x20ddu];
	} else if (codepoint >= 0x20e2u && codepoint <= 0x20e4u) {
		return unifontdw_block168[codepoint - 0x20e2u];
	} else if (codepoint == 0x20e7u) {
		return unifontdw_block169[0];
	} else if (codepoint == 0x20eau) {
		return unifontdw_block170[0];
	} else if (codepoint >= 0x20f1u && codepoint <= 0x20ffu) {
		return unifontdw_block171[codepoint - 0x20f1u];
	} else if (codepoint >= 0x210eu && codepoint <= 0x210fu) {
		return unifontdw_block172[codepoint - 0x210eu];
	} else if (codepoint == 0x212eu) {
		return unifontdw_block173[0];
	} else if (codepoint >= 0x213au && codepoint <= 0x213du) {
		return unifontdw_block174[codepoint - 0x213au];
	} else if (codepoint >= 0x213fu && codepoint <= 0x2140u) {
		return unifontdw_block175[codepoint - 0x213fu];
	} else if (codepoint >= 0x2145u && codepoint <= 0x2149u) {
		return unifontdw_block176[codepoint - 0x2145u];
	} else if (codepoint == 0x214cu) {
		return unifontdw_block177[0];
	} else if (codepoint == 0x214fu) {
		return unifontdw_block178[0];
	} else if (codepoint == 0x2182u) {
		return unifontdw_block179[0];
	} else if (codepoint == 0x2188u) {
		return unifontdw_block180[0];
	} else if (codepoint >= 0x218cu && codepoint <= 0x218fu) {
		return unifontdw_block181[codepoint - 0x218cu];
	} else if (codepoint >= 0x219cu && codepoint <= 0x219du) {
		return unifontdw_block182[codepoint - 0x219cu];
	} else if (codepoint == 0x21f4u) {
		return unifontdw_block183[0];
	} else if (codepoint >= 0x21f9u && codepoint <= 0x21fcu) {
		return unifontdw_block184[codepoint - 0x21f9u];
	} else if (codepoint == 0x21ffu) {
		return unifontdw_block185[0];
	} else if (codepoint >= 0x22b6u && codepoint <= 0x22b8u) {
		return unifontdw_block186[codepoint - 0x22b6u];
	} else if (codepoint >= 0x22d8u && codepoint <= 0x22d9u) {
		return unifontdw_block187[codepoint - 0x22d8u];
	} else if (codepoint >= 0x22f2u && codepoint <= 0x22f3u) {
		return unifontdw_block188[codepoint - 0x22f2u];
	} else if (codepoint >= 0x22f5u && codepoint <= 0x22f6u) {
		return unifontdw_block189[codepoint - 0x22f5u];
	} else if (codepoint >= 0x22f9u && codepoint <= 0x22fbu) {
		return unifontdw_block190[codepoint - 0x22f9u];
	} else if (codepoint == 0x22fdu) {
		return unifontdw_block191[0];
	} else if (codepoint >= 0x22ffu && codepoint <= 0x2300u) {
		return unifontdw_block192[codepoint - 0x22ffu];
	} else if (codepoint == 0x2316u) {
		return unifontdw_block193[0];
	} else if (codepoint >= 0x2329u && codepoint <= 0x232au) {
		return unifontdw_block194[codepoint - 0x2329u];
	} else if (codepoint >= 0x232cu && codepoint <= 0x2335u) {
		return unifontdw_block195[codepoint - 0x232cu];
	} else if (codepoint >= 0x237bu && codepoint <= 0x237eu) {
		return unifontdw_block196[codepoint - 0x237bu];
	} else if (codepoint >= 0x2381u && codepoint <= 0x2394u) {
		return unifontdw_block197[codepoint - 0x2381u];
	} else if (codepoint >= 0x2397u && codepoint <= 0x239au) {
		return unifontdw_block198[codepoint - 0x2397u];
	} else if (codepoint >= 0x23b2u && codepoint <= 0x23b6u) {
		return unifontdw_block199[codepoint - 0x23b2u];
	} else if (codepoint >= 0x23c0u && codepoint <= 0x23cau) {
		return unifontdw_block200[codepoint - 0x23c0u];
	} else if (codepoint >= 0x23cdu && codepoint <= 0x23ceu) {
		return unifontdw_block201[codepoint - 0x23cdu];
	} else if (codepoint >= 0x23d4u && codepoint <= 0x23d9u) {
		return unifontdw_block202[codepoint - 0x23d4u];
	} else if (codepoint >= 0x23dbu && codepoint <= 0x23e7u) {
		return unifontdw_block203[codepoint - 0x23dbu];
	} else if (codepoint >= 0x23e9u && codepoint <= 0x2421u) {
		return unifontdw_block204[codepoint - 0x23e9u];
	} else if (codepoint >= 0x2427u && codepoint <= 0x243fu) {
		return unifontdw_block205[codepoint - 0x2427u];
	} else if (codepoint >= 0x244bu && codepoint <= 0x24ffu) {
		return unifontdw_block206[codepoint - 0x244bu];
	} else if (codepoint == 0x25efu) {
		return unifontdw_block207[0];
	} else if (codepoint == 0x2603u) {
		return unifontdw_block208[0];
	} else if (codepoint >= 0x2610u && codepoint <= 0x2612u) {
		return unifontdw_block209[codepoint - 0x2610u];
	} else if (codepoint >= 0x2615u && codepoint <= 0x2619u) {
		return unifontdw_block210[codepoint - 0x2615u];
	} else if (codepoint >= 0x2622u && codepoint <= 0x2624u) {
		return unifontdw_block211[codepoint - 0x2622u];
	} else if (codepoint >= 0x262bu && codepoint <= 0x262cu) {
		return unifontdw_block212[codepoint - 0x262bu];
	} else if (codepoint >= 0x262fu && codepoint <= 0x2637u) {
		return unifontdw_block213[codepoint - 0x262fu];
	} else if (codepoint >= 0x2672u && codepoint <= 0x268fu) {
		return unifontdw_block214[codepoint - 0x2672u];
	} else if (codepoint >= 0x2692u && codepoint <= 0x26a0u) {
		return unifontdw_block215[codepoint - 0x2692u];
	} else if (codepoint >= 0x26a2u && codepoint <= 0x26a7u) {
		return unifontdw_block216[codepoint - 0x26a2u];
	} else if (codepoint == 0x26a9u) {
		return unifontdw_block217[0];
	} else if (codepoint >= 0x26adu && codepoint <= 0x26b1u) {
		return unifontdw_block218[codepoint - 0x26adu];
	} else if (codepoint == 0x26b6u) {
		return unifontdw_block219[0];
	} else if (codepoint >= 0x26bdu && codepoint <= 0x26e1u) {
		return unifontdw_block220[codepoint - 0x26bdu];
	} else if (codepoint >= 0x26e3u && codepoint <= 0x2767u) {
		return unifontdw_block221[codepoint - 0x26e3u];
	} else if (codepoint >= 0x2776u && codepoint <= 0x27afu) {
		return unifontdw_block222[codepoint - 0x2776u];
	} else if (codepoint >= 0x27b1u && codepoint <= 0x27bfu) {
		return unifontdw_block223[codepoint - 0x27b1u];
	} else if (codepoint == 0x27c1u) {
		return unifontdw_block224[0];
	} else if (codepoint >= 0x27c3u && codepoint <= 0x27c4u) {
		return unifontdw_block225[codepoint - 0x27c3u];
	} else if (codepoint >= 0x27c8u && codepoint <= 0x27c9u) {
		return unifontdw_block226[codepoint - 0x27c8u];
	} else if (codepoint >= 0x27cbu && codepoint <= 0x27d0u) {
		return unifontdw_block227[codepoint - 0x27cbu];
	} else if (codepoint == 0x27d2u) {
		return unifontdw_block228[0];
	} else if (codepoint >= 0x27d5u && codepoint <= 0x27deu) {
		return unifontdw_block229[codepoint - 0x27d5u];
	} else if (codepoint >= 0x27e1u && codepoint <= 0x27e5u) {
		return unifontdw_block230[codepoint - 0x27e1u];
	} else if (codepoint >= 0x27f0u && codepoint <= 0x27ffu) {
		return unifontdw_block231[codepoint - 0x27f0u];
	} else if (codepoint >= 0x2900u && codepoint <= 0x2907u) {
		return unifontdw_block232[codepoint - 0x2900u];
	} else if (codepoint >= 0x290au && codepoint <= 0x2911u) {
		return unifontdw_block233[codepoint - 0x290au];
	} else if (codepoint >= 0x2914u && codepoint <= 0x2937u) {
		return unifontdw_block234[codepoint - 0x2914u];
	} else if (codepoint >= 0x293au && codepoint <= 0x2948u) {
		return unifontdw_block235[codepoint - 0x293au];
	} else if (codepoint >= 0x294au && codepoint <= 0x294bu) {
		return unifontdw_block236[codepoint - 0x294au];
	} else if (codepoint == 0x294eu) {
		return unifontdw_block237[0];
	} else if (codepoint == 0x2950u) {
		return unifontdw_block238[0];
	} else if (codepoint >= 0x2952u && codepoint <= 0x2953u) {
		return unifontdw_block239[codepoint - 0x2952u];
	} else if (codepoint >= 0x2956u && codepoint <= 0x2957u) {
		return unifontdw_block240[codepoint - 0x2956u];
	} else if (codepoint >= 0x295au && codepoint <= 0x295bu) {
		return unifontdw_block241[codepoint - 0x295au];
	} else if (codepoint >= 0x295eu && codepoint <= 0x295fu) {
		return unifontdw_block242[codepoint - 0x295eu];
	} else if (codepoint >= 0x2962u && codepoint <= 0x297bu) {
		return unifontdw_block243[codepoint - 0x2962u];
	} else if (codepoint >= 0x297eu && codepoint <= 0x297fu) {
		return unifontdw_block244[codepoint - 0x297eu];
	} else if (codepoint >= 0x2993u && codepoint <= 0x2996u) {
		return unifontdw_block245[codepoint - 0x2993u];
	} else if (codepoint == 0x299eu) {
		return unifontdw_block246[0];
	} else if (codepoint >= 0x29a8u && codepoint <= 0x29d0u) {
		return unifontdw_block247[codepoint - 0x29a8u];
	} else if (codepoint >= 0x29dau && codepoint <= 0x29dbu) {
		return unifontdw_block248[codepoint - 0x29dau];
	} else if (codepoint >= 0x29dfu && codepoint <= 0x29e0u) {
		return unifontdw_block249[codepoint - 0x29dfu];
	} else if (codepoint >= 0x29e2u && codepoint <= 0x29eau) {
		return unifontdw_block250[codepoint - 0x29e2u];
	} else if (codepoint >= 0x29ecu && codepoint <= 0x29edu) {
		return unifontdw_block251[codepoint - 0x29ecu];
	} else if (codepoint == 0x29f4u) {
		return unifontdw_block252[0];
	} else if (codepoint >= 0x29feu && codepoint <= 0x2a0au) {
		return unifontdw_block253[codepoint - 0x29feu];
	} else if (codepoint == 0x2a0cu) {
		return unifontdw_block254[0];
	} else if (codepoint == 0x2a1du) {
		return unifontdw_block255[0];
	} else if (codepoint == 0x2a20u) {
		return unifontdw_block256[0];
	} else if (codepoint >= 0x2a2du && codepoint <= 0x2a2eu) {
		return unifontdw_block257[codepoint - 0x2a2du];
	} else if (codepoint >= 0x2a33u && codepoint <= 0x2a3bu) {
		return unifontdw_block258[codepoint - 0x2a33u];
	} else if (codepoint >= 0x2a4eu && codepoint <= 0x2a65u) {
		return unifontdw_block259[codepoint - 0x2a4eu];
	} else if (codepoint >= 0x2a68u && codepoint <= 0x2a69u) {
		return unifontdw_block260[codepoint - 0x2a68u];
	} else if (codepoint >= 0x2a74u && codepoint <= 0x2a76u) {
		return unifontdw_block261[codepoint - 0x2a74u];
	} else if (codepoint >= 0x2a78u && codepoint <= 0x2a8au) {
		return unifontdw_block262[codepoint - 0x2a78u];
	} else if (codepoint >= 0x2a8du && codepoint <= 0x2a8eu) {
		return unifontdw_block263[codepoint - 0x2a8du];
	} else if (codepoint >= 0x2a95u && codepoint <= 0x2abeu) {
		return unifontdw_block264[codepoint - 0x2a95u];
	} else if (codepoint >= 0x2acdu && codepoint <= 0x2ad2u) {
		return unifontdw_block265[codepoint - 0x2acdu];
	} else if (codepoint >= 0x2ad7u && codepoint <= 0x2addu) {
		return unifontdw_block266[codepoint - 0x2ad7u];
	} else if (codepoint >= 0x2adfu && codepoint <= 0x2aedu) {
		return unifontdw_block267[codepoint - 0x2adfu];
	} else if (codepoint >= 0x2af3u && codepoint <= 0x2af5u) {
		return unifontdw_block268[codepoint - 0x2af3u];
	} else if (codepoint >= 0x2af7u && codepoint <= 0x2afdu) {
		return unifontdw_block269[codepoint - 0x2af7u];
	} else if (codepoint >= 0x2b00u && codepoint <= 0x2b05u) {
		return unifontdw_block270[codepoint - 0x2b00u];
	} else if (codepoint >= 0x2b08u && codepoint <= 0x2b0cu) {
		return unifontdw_block271[codepoint - 0x2b08u];
	} else if (codepoint >= 0x2b0eu && codepoint <= 0x2b1cu) {
		return unifontdw_block272[codepoint - 0x2b0eu];
	} else if (codepoint >= 0x2b1fu && codepoint <= 0x2b24u) {
		return unifontdw_block273[codepoint - 0x2b1fu];
	} else if (codepoint >= 0x2b2cu && codepoint <= 0x2b2du) {
		return unifontdw_block274[codepoint - 0x2b2cu];
	} else if (codepoint == 0x2b30u) {
		return unifontdw_block275[0];
	} else if (codepoint >= 0x2b32u && codepoint <= 0x2b4du) {
		return unifontdw_block276[codepoint - 0x2b32u];
	} else if (codepoint >= 0x2b50u && codepoint <= 0x2bc8u) {
		return unifontdw_block277[codepoint - 0x2b50u];
	} else if (codepoint >= 0x2bcau && codepoint <= 0x2bfeu) {
		return unifontdw_block278[codepoint - 0x2bcau];
	} else if (codepoint == 0x2c0fu) {
		return unifontdw_block279[0];
	} else if (codepoint == 0x2c1fu) {
		return unifontdw_block280[0];
	} else if (codepoint >= 0x2c27u && codepoint <= 0x2c29u) {
		return unifontdw_block281[codepoint - 0x2c27u];
	} else if (codepoint == 0x2c2fu) {
		return unifontdw_block282[0];
	} else if (codepoint == 0x2c3fu) {
		return unifontdw_block283[0];
	} else if (codepoint == 0x2c4fu) {
		return unifontdw_block284[0];
	} else if (codepoint >= 0x2c57u && codepoint <= 0x2c59u) {
		return unifontdw_block285[codepoint - 0x2c57u];
	} else if (codepoint == 0x2c5fu) {
		return unifontdw_block286[0];
	} else if (codepoint >= 0x2cceu && codepoint <= 0x2ccfu) {
		return unifontdw_block287[codepoint - 0x2cceu];
	} else if (codepoint == 0x2ce7u) {
		return unifontdw_block288[0];
	} else if (codepoint == 0x2ceau) {
		return unifontdw_block289[0];
	} else if (codepoint == 0x2cefu) {
		return unifontdw_block290[0];
	} else if (codepoint >= 0x2cf4u && codepoint <= 0x2cf8u) {
		return unifontdw_block291[codepoint - 0x2cf4u];
	} else if (codepoint == 0x2d05u) {
		return unifontdw_block292[0];
	} else if (codepoint == 0x2d07u) {
		return unifontdw_block293[0];
	} else if (codepoint == 0x2d0au) {
		return unifontdw_block294[0];
	} else if (codepoint == 0x2d0du) {
		return unifontdw_block295[0];
	} else if (codepoint == 0x2d10u) {
		return unifontdw_block296[0];
	} else if (codepoint >= 0x2d13u && codepoint <= 0x2d14u) {
		return unifontdw_block297[codepoint - 0x2d13u];
	} else if (codepoint >= 0x2d1bu && codepoint <= 0x2d1cu) {
		return unifontdw_block298[codepoint - 0x2d1bu];
	} else if (codepoint == 0x2d20u) {
		return unifontdw_block299[0];
	} else if (codepoint >= 0x2d25u && codepoint <= 0x2d26u) {
		return unifontdw_block300[codepoint - 0x2d25u];
	} else if (codepoint >= 0x2d28u && codepoint <= 0x2d2cu) {
		return unifontdw_block301[codepoint - 0x2d28u];
	} else if (codepoint >= 0x2d2eu && codepoint <= 0x2d2fu) {
		return unifontdw_block302[codepoint - 0x2d2eu];
	} else if (codepoint == 0x2d48u) {
		return unifontdw_block303[0];
	} else if (codepoint >= 0x2d68u && codepoint <= 0x2d6eu) {
		return unifontdw_block304[codepoint - 0x2d68u];
	} else if (codepoint >= 0x2d71u && codepoint <= 0x2d7eu) {
		return unifontdw_block305[codepoint - 0x2d71u];
	} else if (codepoint >= 0x2d80u && codepoint <= 0x2ddfu) {
		return unifontdw_block306[codepoint - 0x2d80u];
	} else if (codepoint >= 0x2e0eu && codepoint <= 0x2e11u) {
		return unifontdw_block307[codepoint - 0x2e0eu];
	} else if (codepoint >= 0x2e13u && codepoint <= 0x2e15u) {
		return unifontdw_block308[codepoint - 0x2e13u];
	} else if (codepoint >= 0x2e3au && codepoint <= 0x2e3bu) {
		return unifontdw_block309[codepoint - 0x2e3au];
	} else if (codepoint == 0x2e43u) {
		return unifontdw_block310[0];
	} else if (codepoint >= 0x2e50u && codepoint <= 0x2e51u) {
		return unifontdw_block311[codepoint - 0x2e50u];
	} else if (codepoint >= 0x2e53u && codepoint <= 0x303eu) {
		return unifontdw_block312[codepoint - 0x2e53u];
	} else if (codepoint >= 0x3040u && codepoint <= 0xa4cfu) {
		return unifontdw_block313[codepoint - 0x3040u];
	} else if (codepoint >= 0xa500u && codepoint <= 0xa63fu) {
		return unifontdw_block314[codepoint - 0xa500u];
	} else if (codepoint >= 0xa64cu && codepoint <= 0xa64du) {
		return unifontdw_block315[codepoint - 0xa64cu];
	} else if (codepoint >= 0xa650u && codepoint <= 0xa651u) {
		return unifontdw_block316[codepoint - 0xa650u];
	} else if (codepoint == 0xa65eu) {
		return unifontdw_block317[0];
	} else if (codepoint >= 0xa662u && codepoint <= 0xa667u) {
		return unifontdw_block318[codepoint - 0xa662u];
	} else if (codepoint >= 0xa66cu && codepoint <= 0xa66eu) {
		return unifontdw_block319[codepoint - 0xa66cu];
	} else if (codepoint == 0xa670u) {
		return unifontdw_block320[0];
	} else if (codepoint == 0xa672u) {
		return unifontdw_block321[0];
	} else if (codepoint >= 0xa684u && codepoint <= 0xa685u) {
		return unifontdw_block322[codepoint - 0xa684u];
	} else if (codepoint >= 0xa698u && codepoint <= 0xa699u) {
		return unifontdw_block323[codepoint - 0xa698u];
	} else if (codepoint >= 0xa6f8u && codepoint <= 0xa6ffu) {
		return unifontdw_block324[codepoint - 0xa6f8u];
	} else if (codepoint >= 0xa728u && codepoint <= 0xa729u) {
		return unifontdw_block325[codepoint - 0xa728u];
	} else if (codepoint >= 0xa732u && codepoint <= 0xa73du) {
		return unifontdw_block326[codepoint - 0xa732u];
	} else if (codepoint >= 0xa74eu && codepoint <= 0xa74fu) {
		return unifontdw_block327[codepoint - 0xa74eu];
	} else if (codepoint >= 0xa758u && codepoint <= 0xa759u) {
		return unifontdw_block328[codepoint - 0xa758u];
	} else if (codepoint >= 0xa771u && codepoint <= 0xa777u) {
		return unifontdw_block329[codepoint - 0xa771u];
	} else if (codepoint >= 0xa7c0u && codepoint <= 0xa7c3u) {
		return unifontdw_block330[codepoint - 0xa7c0u];
	} else if (codepoint >= 0xa7cbu && codepoint <= 0xa7f4u) {
		return unifontdw_block331[codepoint - 0xa7cbu];
	} else if (codepoint >= 0xa7ffu && codepoint <= 0xa827u) {
		return unifontdw_block332[codepoint - 0xa7ffu];
	} else if (codepoint >= 0xa82au && codepoint <= 0xab2fu) {
		return unifontdw_block333[codepoint - 0xa82au];
	} else if (codepoint >= 0xab66u && codepoint <= 0xab67u) {
		return unifontdw_block334[codepoint - 0xab66u];
	} else if (codepoint >= 0xab6cu && codepoint <= 0xab6fu) {
		return unifontdw_block335[codepoint - 0xab6cu];
	} else if (codepoint >= 0xabc0u && codepoint <= 0xd7ffu) {
		return unifontdw_block336[codepoint - 0xabc0u];
	} else if (codepoint >= 0xf900u && codepoint <= 0xfaffu) {
		return unifontdw_block337[codepoint - 0xf900u];
	} else if (codepoint >= 0xfb07u && codepoint <= 0xfb12u) {
		return unifontdw_block338[codepoint - 0xfb07u];
	} else if (codepoint >= 0xfb18u && codepoint <= 0xfb1cu) {
		return unifontdw_block339[codepoint - 0xfb18u];
	} else if (codepoint >= 0xfb21u && codepoint <= 0xfb28u) {
		return unifontdw_block340[codepoint - 0xfb21u];
	} else if (codepoint == 0xfb37u) {
		return unifontdw_block341[0];
	} else if (codepoint == 0xfb3du) {
		return unifontdw_block342[0];
	} else if (codepoint == 0xfb3fu) {
		return unifontdw_block343[0];
	} else if (codepoint == 0xfb42u) {
		return unifontdw_block344[0];
	} else if (codepoint == 0xfb45u) {
		return unifontdw_block345[0];
	} else if (codepoint >= 0xfbc2u && codepoint <= 0xfbd2u) {
		return unifontdw_block346[codepoint - 0xfbc2u];
	} else if (codepoint == 0xfc1fu) {
		return unifontdw_block347[0];
	} else if (codepoint == 0xfc21u) {
		return unifontdw_block348[0];
	} else if (codepoint == 0xfc25u) {
		return unifontdw_block349[0];
	} else if (codepoint >= 0xfc3du && codepoint <= 0xfc3eu) {
		return unifontdw_block350[codepoint - 0xfc3du];
	} else if (codepoint >= 0xfcadu && codepoint <= 0xfcafu) {
		return unifontdw_block351[codepoint - 0xfcadu];
	} else if (codepoint >= 0xfcb1u && codepoint <= 0xfcb7u) {
		return unifontdw_block352[codepoint - 0xfcb1u];
	} else if (codepoint >= 0xfce7u && codepoint <= 0xfceau) {
		return unifontdw_block353[codepoint - 0xfce7u];
	} else if (codepoint >= 0xfcfbu && codepoint <= 0xfcfeu) {
		return unifontdw_block354[codepoint - 0xfcfbu];
	} else if (codepoint >= 0xfd17u && codepoint <= 0xfd20u) {
		return unifontdw_block355[codepoint - 0xfd17u];
	} else if (codepoint >= 0xfd2du && codepoint <= 0xfd32u) {
		return unifontdw_block356[codepoint - 0xfd2du];
	} else if (codepoint >= 0xfd34u && codepoint <= 0xfd39u) {
		return unifontdw_block357[codepoint - 0xfd34u];
	} else if (codepoint >= 0xfd3eu && codepoint <= 0xfe1fu) {
		return unifontdw_block358[codepoint - 0xfd3eu];
	} else if (codepoint >= 0xfe30u && codepoint <= 0xfe6fu) {
		return unifontdw_block359[codepoint - 0xfe30u];
	} else if (codepoint == 0xfe75u) {
		return unifontdw_block360[0];
	} else if (codepoint >= 0xfefdu && codepoint <= 0xff60u) {
		return unifontdw_block361[codepoint - 0xfefdu];
	} else if (codepoint >= 0xffbfu && codepoint <= 0xffc1u) {
		return unifontdw_block362[codepoint - 0xffbfu];
	} else if (codepoint >= 0xffc8u && codepoint <= 0xffc9u) {
		return unifontdw_block363[codepoint - 0xffc8u];
	} else if (codepoint >= 0xffd0u && codepoint <= 0xffd1u) {
		return unifontdw_block364[codepoint - 0xffd0u];
	} else if (codepoint >= 0xffd8u && codepoint <= 0xffd9u) {
		return unifontdw_block365[codepoint - 0xffd8u];
	} else if (codepoint >= 0xffddu && codepoint <= 0xffe7u) {
		return unifontdw_block366[codepoint - 0xffddu];
	} else if (codepoint >= 0xffefu && codepoint <= 0xfffcu) {
		return unifontdw_block367[codepoint - 0xffefu];
	} else {
		WARN("Codepoint U+%04X (%s) is not covered by this font", codepoint, u8_cp_to_utf8(codepoint));
		return unifontdw_block1[0];
	}
}
