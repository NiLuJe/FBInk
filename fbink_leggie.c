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

#include "fbink_leggie.h"

static const unsigned char*
    leggie_get_bitmap(uint32_t codepoint)
{
	if (codepoint >= 0x20u && codepoint <= 0x7eu) {
		return leggie_block1[codepoint - 0x20u];
	} else if (codepoint >= 0xa0u && codepoint <= 0x17fu) {
		return leggie_block2[codepoint - 0xa0u];
	} else if (codepoint >= 0x18eu && codepoint <= 0x18fu) {
		return leggie_block3[codepoint - 0x18eu];
	} else if (codepoint == 0x192u) {
		return leggie_block4[0];
	} else if (codepoint >= 0x1a0u && codepoint <= 0x1a1u) {
		return leggie_block5[codepoint - 0x1a0u];
	} else if (codepoint >= 0x1afu && codepoint <= 0x1b0u) {
		return leggie_block6[codepoint - 0x1afu];
	} else if (codepoint >= 0x1b5u && codepoint <= 0x1b7u) {
		return leggie_block7[codepoint - 0x1b5u];
	} else if (codepoint >= 0x1cdu && codepoint <= 0x1ddu) {
		return leggie_block8[codepoint - 0x1cdu];
	} else if (codepoint >= 0x1e4u && codepoint <= 0x1e9u) {
		return leggie_block9[codepoint - 0x1e4u];
	} else if (codepoint >= 0x1eeu && codepoint <= 0x1efu) {
		return leggie_block10[codepoint - 0x1eeu];
	} else if (codepoint >= 0x1fau && codepoint <= 0x1ffu) {
		return leggie_block11[codepoint - 0x1fau];
	} else if (codepoint >= 0x218u && codepoint <= 0x21bu) {
		return leggie_block12[codepoint - 0x218u];
	} else if (codepoint >= 0x250u && codepoint <= 0x2eeu) {
		return leggie_block13[codepoint - 0x250u];
	} else if (codepoint == 0x37au) {
		return leggie_block14[0];
	} else if (codepoint == 0x37eu) {
		return leggie_block15[0];
	} else if (codepoint >= 0x384u && codepoint <= 0x386u) {
		return leggie_block16[codepoint - 0x384u];
	} else if (codepoint >= 0x388u && codepoint <= 0x38au) {
		return leggie_block17[codepoint - 0x388u];
	} else if (codepoint == 0x38cu) {
		return leggie_block18[0];
	} else if (codepoint >= 0x38eu && codepoint <= 0x3a1u) {
		return leggie_block19[codepoint - 0x38eu];
	} else if (codepoint >= 0x3a3u && codepoint <= 0x3ceu) {
		return leggie_block20[codepoint - 0x3a3u];
	} else if (codepoint >= 0x400u && codepoint <= 0x477u) {
		return leggie_block21[codepoint - 0x400u];
	} else if (codepoint >= 0x480u && codepoint <= 0x481u) {
		return leggie_block22[codepoint - 0x480u];
	} else if (codepoint >= 0x48au && codepoint <= 0x493u) {
		return leggie_block23[codepoint - 0x48au];
	} else if (codepoint >= 0x496u && codepoint <= 0x49du) {
		return leggie_block24[codepoint - 0x496u];
	} else if (codepoint >= 0x4a0u && codepoint <= 0x4a3u) {
		return leggie_block25[codepoint - 0x4a0u];
	} else if (codepoint >= 0x4aau && codepoint <= 0x4abu) {
		return leggie_block26[codepoint - 0x4aau];
	} else if (codepoint >= 0x4aeu && codepoint <= 0x4b1u) {
		return leggie_block27[codepoint - 0x4aeu];
	} else if (codepoint >= 0x4bau && codepoint <= 0x4bbu) {
		return leggie_block28[codepoint - 0x4bau];
	} else if (codepoint == 0x4c0u) {
		return leggie_block29[0];
	} else if (codepoint >= 0x4c5u && codepoint <= 0x4cau) {
		return leggie_block30[codepoint - 0x4c5u];
	} else if (codepoint >= 0x4cdu && codepoint <= 0x4d9u) {
		return leggie_block31[codepoint - 0x4cdu];
	} else if (codepoint >= 0x4e2u && codepoint <= 0x4e3u) {
		return leggie_block32[codepoint - 0x4e2u];
	} else if (codepoint >= 0x4e6u && codepoint <= 0x4e9u) {
		return leggie_block33[codepoint - 0x4e6u];
	} else if (codepoint >= 0x4ecu && codepoint <= 0x4f3u) {
		return leggie_block34[codepoint - 0x4ecu];
	} else if (codepoint >= 0x4f8u && codepoint <= 0x4f9u) {
		return leggie_block35[codepoint - 0x4f8u];
	} else if (codepoint >= 0x531u && codepoint <= 0x556u) {
		return leggie_block36[codepoint - 0x531u];
	} else if (codepoint >= 0x559u && codepoint <= 0x55fu) {
		return leggie_block37[codepoint - 0x559u];
	} else if (codepoint >= 0x561u && codepoint <= 0x587u) {
		return leggie_block38[codepoint - 0x561u];
	} else if (codepoint >= 0x589u && codepoint <= 0x58au) {
		return leggie_block39[codepoint - 0x589u];
	} else if (codepoint >= 0x58du && codepoint <= 0x58fu) {
		return leggie_block40[codepoint - 0x58du];
	} else if (codepoint >= 0x5d0u && codepoint <= 0x5eau) {
		return leggie_block41[codepoint - 0x5d0u];
	} else if (codepoint == 0xca0u) {
		return leggie_block42[0];
	} else if (codepoint >= 0x10d0u && codepoint <= 0x10f0u) {
		return leggie_block43[codepoint - 0x10d0u];
	} else if (codepoint >= 0x10f6u && codepoint <= 0x10ffu) {
		return leggie_block44[codepoint - 0x10f6u];
	} else if (codepoint >= 0x1e02u && codepoint <= 0x1e03u) {
		return leggie_block45[codepoint - 0x1e02u];
	} else if (codepoint >= 0x1e0au && codepoint <= 0x1e0bu) {
		return leggie_block46[codepoint - 0x1e0au];
	} else if (codepoint >= 0x1e1eu && codepoint <= 0x1e1fu) {
		return leggie_block47[codepoint - 0x1e1eu];
	} else if (codepoint >= 0x1e24u && codepoint <= 0x1e25u) {
		return leggie_block48[codepoint - 0x1e24u];
	} else if (codepoint >= 0x1e36u && codepoint <= 0x1e37u) {
		return leggie_block49[codepoint - 0x1e36u];
	} else if (codepoint >= 0x1e40u && codepoint <= 0x1e41u) {
		return leggie_block50[codepoint - 0x1e40u];
	} else if (codepoint >= 0x1e56u && codepoint <= 0x1e57u) {
		return leggie_block51[codepoint - 0x1e56u];
	} else if (codepoint >= 0x1e60u && codepoint <= 0x1e61u) {
		return leggie_block52[codepoint - 0x1e60u];
	} else if (codepoint >= 0x1e6au && codepoint <= 0x1e6bu) {
		return leggie_block53[codepoint - 0x1e6au];
	} else if (codepoint >= 0x1e80u && codepoint <= 0x1e85u) {
		return leggie_block54[codepoint - 0x1e80u];
	} else if (codepoint >= 0x1e8au && codepoint <= 0x1e8bu) {
		return leggie_block55[codepoint - 0x1e8au];
	} else if (codepoint >= 0x1ea0u && codepoint <= 0x1ef9u) {
		return leggie_block56[codepoint - 0x1ea0u];
	} else if (codepoint >= 0x2010u && codepoint <= 0x2027u) {
		return leggie_block57[codepoint - 0x2010u];
	} else if (codepoint >= 0x2030u && codepoint <= 0x203au) {
		return leggie_block58[codepoint - 0x2030u];
	} else if (codepoint >= 0x203cu && codepoint <= 0x205eu) {
		return leggie_block59[codepoint - 0x203cu];
	} else if (codepoint >= 0x2061u && codepoint <= 0x2064u) {
		return leggie_block60[codepoint - 0x2061u];
	} else if (codepoint >= 0x2070u && codepoint <= 0x2071u) {
		return leggie_block61[codepoint - 0x2070u];
	} else if (codepoint >= 0x2074u && codepoint <= 0x208eu) {
		return leggie_block62[codepoint - 0x2074u];
	} else if (codepoint >= 0x2090u && codepoint <= 0x209cu) {
		return leggie_block63[codepoint - 0x2090u];
	} else if (codepoint == 0x20a1u) {
		return leggie_block64[0];
	} else if (codepoint >= 0x20a5u && codepoint <= 0x20afu) {
		return leggie_block65[codepoint - 0x20a5u];
	} else if (codepoint >= 0x20b1u && codepoint <= 0x20b2u) {
		return leggie_block66[codepoint - 0x20b1u];
	} else if (codepoint >= 0x20b4u && codepoint <= 0x20b5u) {
		return leggie_block67[codepoint - 0x20b4u];
	} else if (codepoint >= 0x20b8u && codepoint <= 0x20bau) {
		return leggie_block68[codepoint - 0x20b8u];
	} else if (codepoint >= 0x20bcu && codepoint <= 0x20bdu) {
		return leggie_block69[codepoint - 0x20bcu];
	} else if (codepoint == 0x2116u) {
		return leggie_block70[0];
	} else if (codepoint == 0x2122u) {
		return leggie_block71[0];
	} else if (codepoint == 0x212bu) {
		return leggie_block72[0];
	} else if (codepoint >= 0x2190u && codepoint <= 0x2196u) {
		return leggie_block73[codepoint - 0x2190u];
	} else if (codepoint == 0x2198u) {
		return leggie_block74[0];
	} else if (codepoint == 0x21a4u) {
		return leggie_block75[0];
	} else if (codepoint == 0x21a6u) {
		return leggie_block76[0];
	} else if (codepoint >= 0x21a8u && codepoint <= 0x21a9u) {
		return leggie_block77[codepoint - 0x21a8u];
	} else if (codepoint == 0x21b5u) {
		return leggie_block78[0];
	} else if (codepoint >= 0x21b8u && codepoint <= 0x21b9u) {
		return leggie_block79[codepoint - 0x21b8u];
	} else if (codepoint == 0x21c6u) {
		return leggie_block80[0];
	} else if (codepoint >= 0x21d0u && codepoint <= 0x21d5u) {
		return leggie_block81[codepoint - 0x21d0u];
	} else if (codepoint >= 0x21deu && codepoint <= 0x21dfu) {
		return leggie_block82[codepoint - 0x21deu];
	} else if (codepoint >= 0x21e4u && codepoint <= 0x21e5u) {
		return leggie_block83[codepoint - 0x21e4u];
	} else if (codepoint == 0x21e7u) {
		return leggie_block84[0];
	} else if (codepoint == 0x21eau) {
		return leggie_block85[0];
	} else if (codepoint >= 0x21f1u && codepoint <= 0x21f2u) {
		return leggie_block86[codepoint - 0x21f1u];
	} else if (codepoint == 0x2203u) {
		return leggie_block87[0];
	} else if (codepoint == 0x2205u) {
		return leggie_block88[0];
	} else if (codepoint == 0x2208u) {
		return leggie_block89[0];
	} else if (codepoint >= 0x2219u && codepoint <= 0x221au) {
		return leggie_block90[codepoint - 0x2219u];
	} else if (codepoint >= 0x221eu && codepoint <= 0x221fu) {
		return leggie_block91[codepoint - 0x221eu];
	} else if (codepoint >= 0x2227u && codepoint <= 0x222au) {
		return leggie_block92[codepoint - 0x2227u];
	} else if (codepoint == 0x2248u) {
		return leggie_block93[0];
	} else if (codepoint >= 0x2260u && codepoint <= 0x2261u) {
		return leggie_block94[codepoint - 0x2260u];
	} else if (codepoint >= 0x2264u && codepoint <= 0x2265u) {
		return leggie_block95[codepoint - 0x2264u];
	} else if (codepoint >= 0x2296u && codepoint <= 0x2297u) {
		return leggie_block96[codepoint - 0x2296u];
	} else if (codepoint == 0x229du) {
		return leggie_block97[0];
	} else if (codepoint >= 0x2302u && codepoint <= 0x2303u) {
		return leggie_block98[codepoint - 0x2302u];
	} else if (codepoint == 0x2305u) {
		return leggie_block99[0];
	} else if (codepoint == 0x2310u) {
		return leggie_block100[0];
	} else if (codepoint == 0x2318u) {
		return leggie_block101[0];
	} else if (codepoint >= 0x2320u && codepoint <= 0x2321u) {
		return leggie_block102[codepoint - 0x2320u];
	} else if (codepoint >= 0x2324u && codepoint <= 0x2328u) {
		return leggie_block103[codepoint - 0x2324u];
	} else if (codepoint == 0x232bu) {
		return leggie_block104[0];
	} else if (codepoint == 0x233du) {
		return leggie_block105[0];
	} else if (codepoint == 0x2380u) {
		return leggie_block106[0];
	} else if (codepoint == 0x2384u) {
		return leggie_block107[0];
	} else if (codepoint >= 0x2386u && codepoint <= 0x2388u) {
		return leggie_block108[codepoint - 0x2386u];
	} else if (codepoint == 0x238bu) {
		return leggie_block109[0];
	} else if (codepoint >= 0x23bau && codepoint <= 0x23bdu) {
		return leggie_block110[codepoint - 0x23bau];
	} else if (codepoint >= 0x23ceu && codepoint <= 0x23cfu) {
		return leggie_block111[codepoint - 0x23ceu];
	} else if (codepoint >= 0x2400u && codepoint <= 0x2426u) {
		return leggie_block112[codepoint - 0x2400u];
	} else if (codepoint == 0x2500u) {
		return leggie_block113[0];
	} else if (codepoint == 0x2502u) {
		return leggie_block114[0];
	} else if (codepoint == 0x250cu) {
		return leggie_block115[0];
	} else if (codepoint == 0x2510u) {
		return leggie_block116[0];
	} else if (codepoint == 0x2514u) {
		return leggie_block117[0];
	} else if (codepoint == 0x2518u) {
		return leggie_block118[0];
	} else if (codepoint == 0x251cu) {
		return leggie_block119[0];
	} else if (codepoint == 0x2524u) {
		return leggie_block120[0];
	} else if (codepoint == 0x252cu) {
		return leggie_block121[0];
	} else if (codepoint == 0x2534u) {
		return leggie_block122[0];
	} else if (codepoint == 0x253cu) {
		return leggie_block123[0];
	} else if (codepoint >= 0x2550u && codepoint <= 0x256cu) {
		return leggie_block124[codepoint - 0x2550u];
	} else if (codepoint == 0x2580u) {
		return leggie_block125[0];
	} else if (codepoint == 0x2584u) {
		return leggie_block126[0];
	} else if (codepoint == 0x2588u) {
		return leggie_block127[0];
	} else if (codepoint == 0x258cu) {
		return leggie_block128[0];
	} else if (codepoint >= 0x2590u && codepoint <= 0x2593u) {
		return leggie_block129[codepoint - 0x2590u];
	} else if (codepoint >= 0x25a0u && codepoint <= 0x25a1u) {
		return leggie_block130[codepoint - 0x25a0u];
	} else if (codepoint == 0x25a4u) {
		return leggie_block131[0];
	} else if (codepoint >= 0x25aau && codepoint <= 0x25acu) {
		return leggie_block132[codepoint - 0x25aau];
	} else if (codepoint == 0x25b2u) {
		return leggie_block133[0];
	} else if (codepoint >= 0x25b6u && codepoint <= 0x25b8u) {
		return leggie_block134[codepoint - 0x25b6u];
	} else if (codepoint == 0x25bau) {
		return leggie_block135[0];
	} else if (codepoint == 0x25bcu) {
		return leggie_block136[0];
	} else if (codepoint == 0x25c1u) {
		return leggie_block137[0];
	} else if (codepoint == 0x25c4u) {
		return leggie_block138[0];
	} else if (codepoint >= 0x25c6u && codepoint <= 0x25c7u) {
		return leggie_block139[codepoint - 0x25c6u];
	} else if (codepoint >= 0x25cau && codepoint <= 0x25cbu) {
		return leggie_block140[codepoint - 0x25cau];
	} else if (codepoint == 0x25cfu) {
		return leggie_block141[0];
	} else if (codepoint >= 0x25d8u && codepoint <= 0x25d9u) {
		return leggie_block142[codepoint - 0x25d8u];
	} else if (codepoint == 0x25efu) {
		return leggie_block143[0];
	} else if (codepoint >= 0x2610u && codepoint <= 0x2612u) {
		return leggie_block144[codepoint - 0x2610u];
	} else if (codepoint >= 0x263au && codepoint <= 0x263cu) {
		return leggie_block145[codepoint - 0x263au];
	} else if (codepoint == 0x2640u) {
		return leggie_block146[0];
	} else if (codepoint == 0x2642u) {
		return leggie_block147[0];
	} else if (codepoint == 0x2660u) {
		return leggie_block148[0];
	} else if (codepoint == 0x2663u) {
		return leggie_block149[0];
	} else if (codepoint >= 0x2665u && codepoint <= 0x2666u) {
		return leggie_block150[codepoint - 0x2665u];
	} else if (codepoint >= 0x266au && codepoint <= 0x266bu) {
		return leggie_block151[codepoint - 0x266au];
	} else if (codepoint == 0x2713u) {
		return leggie_block152[0];
	} else if (codepoint == 0x2717u) {
		return leggie_block153[0];
	} else if (codepoint >= 0x2726u && codepoint <= 0x2727u) {
		return leggie_block154[codepoint - 0x2726u];
	} else if (codepoint == 0x2732u) {
		return leggie_block155[0];
	} else if (codepoint == 0x2756u) {
		return leggie_block156[0];
	} else if (codepoint >= 0x2800u && codepoint <= 0x28ffu) {
		return leggie_block157[codepoint - 0x2800u];
	} else if (codepoint >= 0xa640u && codepoint <= 0xa643u) {
		return leggie_block158[codepoint - 0xa640u];
	} else if (codepoint >= 0xa64au && codepoint <= 0xa64bu) {
		return leggie_block159[codepoint - 0xa64au];
	} else if (codepoint >= 0xa650u && codepoint <= 0xa651u) {
		return leggie_block160[codepoint - 0xa650u];
	} else if (codepoint >= 0xa656u && codepoint <= 0xa657u) {
		return leggie_block161[codepoint - 0xa656u];
	} else if (codepoint >= 0xa790u && codepoint <= 0xa791u) {
		return leggie_block162[codepoint - 0xa790u];
	} else if (codepoint >= 0xe000u && codepoint <= 0xe005u) {
		return leggie_block163[codepoint - 0xe000u];
	} else if (codepoint >= 0xe010u && codepoint <= 0xe01au) {
		return leggie_block164[codepoint - 0xe010u];
	} else if (codepoint >= 0xe020u && codepoint <= 0xe025u) {
		return leggie_block165[codepoint - 0xe020u];
	} else if (codepoint >= 0xe030u && codepoint <= 0xe039u) {
		return leggie_block166[codepoint - 0xe030u];
	} else if (codepoint >= 0xe0a0u && codepoint <= 0xe0a2u) {
		return leggie_block167[codepoint - 0xe0a0u];
	} else if (codepoint >= 0xe0b0u && codepoint <= 0xe0b3u) {
		return leggie_block168[codepoint - 0xe0b0u];
	} else if (codepoint >= 0xf000u && codepoint <= 0xf002u) {
		return leggie_block169[codepoint - 0xf000u];
	} else if (codepoint >= 0xf800u && codepoint <= 0xf803u) {
		return leggie_block170[codepoint - 0xf800u];
	} else if (codepoint >= 0xf810u && codepoint <= 0xf813u) {
		return leggie_block171[codepoint - 0xf810u];
	} else if (codepoint == 0xf8ffu) {
		return leggie_block172[0];
	} else if (codepoint >= 0xfb00u && codepoint <= 0xfb06u) {
		return leggie_block173[codepoint - 0xfb00u];
	} else if (codepoint >= 0xfb13u && codepoint <= 0xfb17u) {
		return leggie_block174[codepoint - 0xfb13u];
	} else if (codepoint >= 0xfe50u && codepoint <= 0xfe52u) {
		return leggie_block175[codepoint - 0xfe50u];
	} else if (codepoint >= 0xfe54u && codepoint <= 0xfe66u) {
		return leggie_block176[codepoint - 0xfe54u];
	} else if (codepoint >= 0xfe68u && codepoint <= 0xfe6bu) {
		return leggie_block177[codepoint - 0xfe68u];
	} else if (codepoint == 0xfffdu) {
		return leggie_block178[0];
	} else if (codepoint == 0xffffu) {
		return leggie_block179[0];
	} else {
		WARN("Codepoint U+%04X (%s) is not covered by this font", codepoint, u8_cp_to_utf8(codepoint));
		return leggie_block1[0];
	}
}

static const unsigned char*
    veggie_get_bitmap(uint32_t codepoint)
{
	if (codepoint >= 0x20u && codepoint <= 0x7eu) {
		return veggie_block1[codepoint - 0x20u];
	} else if (codepoint >= 0xa0u && codepoint <= 0x113u) {
		return veggie_block2[codepoint - 0xa0u];
	} else if (codepoint >= 0x116u && codepoint <= 0x12bu) {
		return veggie_block3[codepoint - 0x116u];
	} else if (codepoint >= 0x12eu && codepoint <= 0x131u) {
		return veggie_block4[codepoint - 0x12eu];
	} else if (codepoint >= 0x134u && codepoint <= 0x13eu) {
		return veggie_block5[codepoint - 0x134u];
	} else if (codepoint >= 0x141u && codepoint <= 0x148u) {
		return veggie_block6[codepoint - 0x141u];
	} else if (codepoint >= 0x14au && codepoint <= 0x14du) {
		return veggie_block7[codepoint - 0x14au];
	} else if (codepoint >= 0x150u && codepoint <= 0x17eu) {
		return veggie_block8[codepoint - 0x150u];
	} else if (codepoint == 0x192u) {
		return veggie_block9[0];
	} else if (codepoint >= 0x218u && codepoint <= 0x21bu) {
		return veggie_block10[codepoint - 0x218u];
	} else if (codepoint == 0x2c7u) {
		return veggie_block11[0];
	} else if (codepoint >= 0x2d8u && codepoint <= 0x2d9u) {
		return veggie_block12[codepoint - 0x2d8u];
	} else if (codepoint == 0x2dbu) {
		return veggie_block13[0];
	} else if (codepoint == 0x2ddu) {
		return veggie_block14[0];
	} else if (codepoint == 0x37au) {
		return veggie_block15[0];
	} else if (codepoint == 0x37eu) {
		return veggie_block16[0];
	} else if (codepoint >= 0x384u && codepoint <= 0x386u) {
		return veggie_block17[codepoint - 0x384u];
	} else if (codepoint >= 0x388u && codepoint <= 0x38au) {
		return veggie_block18[codepoint - 0x388u];
	} else if (codepoint == 0x38cu) {
		return veggie_block19[0];
	} else if (codepoint >= 0x38eu && codepoint <= 0x3a1u) {
		return veggie_block20[codepoint - 0x38eu];
	} else if (codepoint >= 0x3a3u && codepoint <= 0x3ceu) {
		return veggie_block21[codepoint - 0x3a3u];
	} else if (codepoint >= 0x401u && codepoint <= 0x44fu) {
		return veggie_block22[codepoint - 0x401u];
	} else if (codepoint >= 0x451u && codepoint <= 0x45fu) {
		return veggie_block23[codepoint - 0x451u];
	} else if (codepoint >= 0x490u && codepoint <= 0x491u) {
		return veggie_block24[codepoint - 0x490u];
	} else if (codepoint >= 0x1e02u && codepoint <= 0x1e03u) {
		return veggie_block25[codepoint - 0x1e02u];
	} else if (codepoint >= 0x1e0au && codepoint <= 0x1e0bu) {
		return veggie_block26[codepoint - 0x1e0au];
	} else if (codepoint >= 0x1e1eu && codepoint <= 0x1e1fu) {
		return veggie_block27[codepoint - 0x1e1eu];
	} else if (codepoint >= 0x1e40u && codepoint <= 0x1e41u) {
		return veggie_block28[codepoint - 0x1e40u];
	} else if (codepoint >= 0x1e56u && codepoint <= 0x1e57u) {
		return veggie_block29[codepoint - 0x1e56u];
	} else if (codepoint >= 0x1e60u && codepoint <= 0x1e61u) {
		return veggie_block30[codepoint - 0x1e60u];
	} else if (codepoint >= 0x1e6au && codepoint <= 0x1e6bu) {
		return veggie_block31[codepoint - 0x1e6au];
	} else if (codepoint >= 0x1e80u && codepoint <= 0x1e85u) {
		return veggie_block32[codepoint - 0x1e80u];
	} else if (codepoint >= 0x1ef2u && codepoint <= 0x1ef3u) {
		return veggie_block33[codepoint - 0x1ef2u];
	} else if (codepoint == 0x2015u) {
		return veggie_block34[0];
	} else if (codepoint >= 0x2018u && codepoint <= 0x2019u) {
		return veggie_block35[codepoint - 0x2018u];
	} else if (codepoint >= 0x201bu && codepoint <= 0x2022u) {
		return veggie_block36[codepoint - 0x201bu];
	} else if (codepoint == 0x2026u) {
		return veggie_block37[0];
	} else if (codepoint == 0x2030u) {
		return veggie_block38[0];
	} else if (codepoint >= 0x203cu && codepoint <= 0x203du) {
		return veggie_block39[codepoint - 0x203cu];
	} else if (codepoint == 0x207fu) {
		return veggie_block40[0];
	} else if (codepoint == 0x20a7u) {
		return veggie_block41[0];
	} else if (codepoint == 0x20acu) {
		return veggie_block42[0];
	} else if (codepoint == 0x20afu) {
		return veggie_block43[0];
	} else if (codepoint == 0x2116u) {
		return veggie_block44[0];
	} else if (codepoint == 0x2122u) {
		return veggie_block45[0];
	} else if (codepoint >= 0x2190u && codepoint <= 0x2195u) {
		return veggie_block46[codepoint - 0x2190u];
	} else if (codepoint == 0x21a8u) {
		return veggie_block47[0];
	} else if (codepoint == 0x21b5u) {
		return veggie_block48[0];
	} else if (codepoint >= 0x2219u && codepoint <= 0x221au) {
		return veggie_block49[codepoint - 0x2219u];
	} else if (codepoint >= 0x221eu && codepoint <= 0x221fu) {
		return veggie_block50[codepoint - 0x221eu];
	} else if (codepoint == 0x2229u) {
		return veggie_block51[0];
	} else if (codepoint == 0x2248u) {
		return veggie_block52[0];
	} else if (codepoint >= 0x2260u && codepoint <= 0x2261u) {
		return veggie_block53[codepoint - 0x2260u];
	} else if (codepoint >= 0x2264u && codepoint <= 0x2265u) {
		return veggie_block54[codepoint - 0x2264u];
	} else if (codepoint == 0x2302u) {
		return veggie_block55[0];
	} else if (codepoint == 0x2310u) {
		return veggie_block56[0];
	} else if (codepoint >= 0x2320u && codepoint <= 0x2321u) {
		return veggie_block57[codepoint - 0x2320u];
	} else if (codepoint == 0x2500u) {
		return veggie_block58[0];
	} else if (codepoint == 0x2502u) {
		return veggie_block59[0];
	} else if (codepoint == 0x250cu) {
		return veggie_block60[0];
	} else if (codepoint == 0x2510u) {
		return veggie_block61[0];
	} else if (codepoint == 0x2514u) {
		return veggie_block62[0];
	} else if (codepoint == 0x2518u) {
		return veggie_block63[0];
	} else if (codepoint == 0x251cu) {
		return veggie_block64[0];
	} else if (codepoint == 0x2524u) {
		return veggie_block65[0];
	} else if (codepoint == 0x252cu) {
		return veggie_block66[0];
	} else if (codepoint == 0x2534u) {
		return veggie_block67[0];
	} else if (codepoint == 0x253cu) {
		return veggie_block68[0];
	} else if (codepoint >= 0x2550u && codepoint <= 0x256cu) {
		return veggie_block69[codepoint - 0x2550u];
	} else if (codepoint == 0x2580u) {
		return veggie_block70[0];
	} else if (codepoint == 0x2584u) {
		return veggie_block71[0];
	} else if (codepoint == 0x2588u) {
		return veggie_block72[0];
	} else if (codepoint == 0x258cu) {
		return veggie_block73[0];
	} else if (codepoint >= 0x2590u && codepoint <= 0x2593u) {
		return veggie_block74[codepoint - 0x2590u];
	} else if (codepoint == 0x25a0u) {
		return veggie_block75[0];
	} else if (codepoint == 0x25acu) {
		return veggie_block76[0];
	} else if (codepoint == 0x25b2u) {
		return veggie_block77[0];
	} else if (codepoint == 0x25b6u) {
		return veggie_block78[0];
	} else if (codepoint == 0x25bcu) {
		return veggie_block79[0];
	} else if (codepoint == 0x25c0u) {
		return veggie_block80[0];
	} else if (codepoint == 0x25cbu) {
		return veggie_block81[0];
	} else if (codepoint >= 0x25d8u && codepoint <= 0x25d9u) {
		return veggie_block82[codepoint - 0x25d8u];
	} else if (codepoint >= 0x263au && codepoint <= 0x263cu) {
		return veggie_block83[codepoint - 0x263au];
	} else if (codepoint == 0x2640u) {
		return veggie_block84[0];
	} else if (codepoint == 0x2642u) {
		return veggie_block85[0];
	} else if (codepoint == 0x2660u) {
		return veggie_block86[0];
	} else if (codepoint == 0x2663u) {
		return veggie_block87[0];
	} else if (codepoint >= 0x2665u && codepoint <= 0x2666u) {
		return veggie_block88[codepoint - 0x2665u];
	} else if (codepoint >= 0x266au && codepoint <= 0x266bu) {
		return veggie_block89[codepoint - 0x266au];
	} else if (codepoint >= 0xe0a0u && codepoint <= 0xe0a2u) {
		return veggie_block90[codepoint - 0xe0a0u];
	} else if (codepoint >= 0xe0b0u && codepoint <= 0xe0b3u) {
		return veggie_block91[codepoint - 0xe0b0u];
	} else if (codepoint == 0xfffdu) {
		return veggie_block92[0];
	} else if (codepoint == 0xffffu) {
		return veggie_block93[0];
	} else {
		WARN("Codepoint U+%04X (%s) is not covered by this font", codepoint, u8_cp_to_utf8(codepoint));
		return veggie_block1[0];
	}
}
