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

#include "fbink_terminus.h"

static const unsigned char*
    terminus_get_bitmap(uint32_t codepoint)
{
	if (codepoint == 0x00u) {
		return terminus_block1[codepoint];
	} else if (codepoint >= 0x20u && codepoint <= 0x7eu) {
		return terminus_block2[codepoint - 0x20u];
	} else if (codepoint >= 0xa0u && codepoint <= 0x17fu) {
		return terminus_block3[codepoint - 0xa0u];
	} else if (codepoint == 0x186u) {
		return terminus_block4[0];
	} else if (codepoint >= 0x18eu && codepoint <= 0x190u) {
		return terminus_block5[codepoint - 0x18eu];
	} else if (codepoint == 0x192u) {
		return terminus_block6[0];
	} else if (codepoint >= 0x19du && codepoint <= 0x19eu) {
		return terminus_block7[codepoint - 0x19du];
	} else if (codepoint >= 0x1b5u && codepoint <= 0x1b7u) {
		return terminus_block8[codepoint - 0x1b5u];
	} else if (codepoint >= 0x1cdu && codepoint <= 0x1d4u) {
		return terminus_block9[codepoint - 0x1cdu];
	} else if (codepoint >= 0x1e2u && codepoint <= 0x1f0u) {
		return terminus_block10[codepoint - 0x1e2u];
	} else if (codepoint >= 0x1f4u && codepoint <= 0x1f5u) {
		return terminus_block11[codepoint - 0x1f4u];
	} else if (codepoint >= 0x1fcu && codepoint <= 0x1ffu) {
		return terminus_block12[codepoint - 0x1fcu];
	} else if (codepoint >= 0x218u && codepoint <= 0x21bu) {
		return terminus_block13[codepoint - 0x218u];
	} else if (codepoint >= 0x232u && codepoint <= 0x233u) {
		return terminus_block14[codepoint - 0x232u];
	} else if (codepoint == 0x237u) {
		return terminus_block15[0];
	} else if (codepoint == 0x254u) {
		return terminus_block16[0];
	} else if (codepoint >= 0x258u && codepoint <= 0x259u) {
		return terminus_block17[codepoint - 0x258u];
	} else if (codepoint == 0x25bu) {
		return terminus_block18[0];
	} else if (codepoint == 0x272u) {
		return terminus_block19[0];
	} else if (codepoint == 0x292u) {
		return terminus_block20[0];
	} else if (codepoint >= 0x2bbu && codepoint <= 0x2bdu) {
		return terminus_block21[codepoint - 0x2bbu];
	} else if (codepoint >= 0x2c6u && codepoint <= 0x2c7u) {
		return terminus_block22[codepoint - 0x2c6u];
	} else if (codepoint >= 0x2d8u && codepoint <= 0x2d9u) {
		return terminus_block23[codepoint - 0x2d8u];
	} else if (codepoint >= 0x2dbu && codepoint <= 0x2ddu) {
		return terminus_block24[codepoint - 0x2dbu];
	} else if (codepoint >= 0x300u && codepoint <= 0x308u) {
		return terminus_block25[codepoint - 0x300u];
	} else if (codepoint >= 0x30au && codepoint <= 0x30cu) {
		return terminus_block26[codepoint - 0x30au];
	} else if (codepoint == 0x329u) {
		return terminus_block27[0];
	} else if (codepoint >= 0x384u && codepoint <= 0x38au) {
		return terminus_block28[codepoint - 0x384u];
	} else if (codepoint == 0x38cu) {
		return terminus_block29[0];
	} else if (codepoint >= 0x38eu && codepoint <= 0x3a1u) {
		return terminus_block30[codepoint - 0x38eu];
	} else if (codepoint >= 0x3a3u && codepoint <= 0x3ceu) {
		return terminus_block31[codepoint - 0x3a3u];
	} else if (codepoint == 0x3d1u) {
		return terminus_block32[0];
	} else if (codepoint == 0x3d5u) {
		return terminus_block33[0];
	} else if (codepoint >= 0x3f0u && codepoint <= 0x3f6u) {
		return terminus_block34[codepoint - 0x3f0u];
	} else if (codepoint >= 0x400u && codepoint <= 0x45fu) {
		return terminus_block35[codepoint - 0x400u];
	} else if (codepoint >= 0x462u && codepoint <= 0x463u) {
		return terminus_block36[codepoint - 0x462u];
	} else if (codepoint >= 0x46au && codepoint <= 0x46bu) {
		return terminus_block37[codepoint - 0x46au];
	} else if (codepoint >= 0x490u && codepoint <= 0x49du) {
		return terminus_block38[codepoint - 0x490u];
	} else if (codepoint >= 0x4a0u && codepoint <= 0x4a5u) {
		return terminus_block39[codepoint - 0x4a0u];
	} else if (codepoint >= 0x4aau && codepoint <= 0x4abu) {
		return terminus_block40[codepoint - 0x4aau];
	} else if (codepoint >= 0x4aeu && codepoint <= 0x4b3u) {
		return terminus_block41[codepoint - 0x4aeu];
	} else if (codepoint >= 0x4b6u && codepoint <= 0x4bbu) {
		return terminus_block42[codepoint - 0x4b6u];
	} else if (codepoint >= 0x4c0u && codepoint <= 0x4c2u) {
		return terminus_block43[codepoint - 0x4c0u];
	} else if (codepoint >= 0x4cfu && codepoint <= 0x4dfu) {
		return terminus_block44[codepoint - 0x4cfu];
	} else if (codepoint >= 0x4e2u && codepoint <= 0x4f5u) {
		return terminus_block45[codepoint - 0x4e2u];
	} else if (codepoint >= 0x4f8u && codepoint <= 0x4f9u) {
		return terminus_block46[codepoint - 0x4f8u];
	} else if (codepoint >= 0x5d0u && codepoint <= 0x5eau) {
		return terminus_block47[codepoint - 0x5d0u];
	} else if (codepoint >= 0x1e0cu && codepoint <= 0x1e0du) {
		return terminus_block48[codepoint - 0x1e0cu];
	} else if (codepoint >= 0x1e34u && codepoint <= 0x1e37u) {
		return terminus_block49[codepoint - 0x1e34u];
	} else if (codepoint >= 0x1e40u && codepoint <= 0x1e47u) {
		return terminus_block50[codepoint - 0x1e40u];
	} else if (codepoint >= 0x1e6cu && codepoint <= 0x1e6du) {
		return terminus_block51[codepoint - 0x1e6cu];
	} else if (codepoint >= 0x1eb8u && codepoint <= 0x1eb9u) {
		return terminus_block52[codepoint - 0x1eb8u];
	} else if (codepoint >= 0x1ebcu && codepoint <= 0x1ebdu) {
		return terminus_block53[codepoint - 0x1ebcu];
	} else if (codepoint >= 0x1ecau && codepoint <= 0x1ecdu) {
		return terminus_block54[codepoint - 0x1ecau];
	} else if (codepoint >= 0x1ee4u && codepoint <= 0x1ee5u) {
		return terminus_block55[codepoint - 0x1ee4u];
	} else if (codepoint >= 0x1ef8u && codepoint <= 0x1ef9u) {
		return terminus_block56[codepoint - 0x1ef8u];
	} else if (codepoint >= 0x2000u && codepoint <= 0x2022u) {
		return terminus_block57[codepoint - 0x2000u];
	} else if (codepoint == 0x2026u) {
		return terminus_block58[0];
	} else if (codepoint == 0x2030u) {
		return terminus_block59[0];
	} else if (codepoint >= 0x2032u && codepoint <= 0x2033u) {
		return terminus_block60[codepoint - 0x2032u];
	} else if (codepoint >= 0x2039u && codepoint <= 0x203au) {
		return terminus_block61[codepoint - 0x2039u];
	} else if (codepoint == 0x203cu) {
		return terminus_block62[0];
	} else if (codepoint == 0x203eu) {
		return terminus_block63[0];
	} else if (codepoint >= 0x2070u && codepoint <= 0x2071u) {
		return terminus_block64[codepoint - 0x2070u];
	} else if (codepoint >= 0x2074u && codepoint <= 0x208eu) {
		return terminus_block65[codepoint - 0x2074u];
	} else if (codepoint >= 0x2090u && codepoint <= 0x2098u) {
		return terminus_block66[codepoint - 0x2090u];
	} else if (codepoint == 0x209au) {
		return terminus_block67[0];
	} else if (codepoint == 0x20a7u) {
		return terminus_block68[0];
	} else if (codepoint == 0x20aau) {
		return terminus_block69[0];
	} else if (codepoint == 0x20acu) {
		return terminus_block70[0];
	} else if (codepoint == 0x20aeu) {
		return terminus_block71[0];
	} else if (codepoint == 0x2102u) {
		return terminus_block72[0];
	} else if (codepoint >= 0x210eu && codepoint <= 0x210fu) {
		return terminus_block73[codepoint - 0x210eu];
	} else if (codepoint >= 0x2115u && codepoint <= 0x2116u) {
		return terminus_block74[codepoint - 0x2115u];
	} else if (codepoint == 0x211au) {
		return terminus_block75[0];
	} else if (codepoint == 0x211du) {
		return terminus_block76[0];
	} else if (codepoint == 0x2122u) {
		return terminus_block77[0];
	} else if (codepoint == 0x2124u) {
		return terminus_block78[0];
	} else if (codepoint == 0x2126u) {
		return terminus_block79[0];
	} else if (codepoint == 0x2135u) {
		return terminus_block80[0];
	} else if (codepoint >= 0x2190u && codepoint <= 0x2195u) {
		return terminus_block81[codepoint - 0x2190u];
	} else if (codepoint == 0x21a4u) {
		return terminus_block82[0];
	} else if (codepoint == 0x21a6u) {
		return terminus_block83[0];
	} else if (codepoint == 0x21a8u) {
		return terminus_block84[0];
	} else if (codepoint == 0x21b5u) {
		return terminus_block85[0];
	} else if (codepoint == 0x21bbu) {
		return terminus_block86[0];
	} else if (codepoint >= 0x21cbu && codepoint <= 0x21ccu) {
		return terminus_block87[codepoint - 0x21cbu];
	} else if (codepoint >= 0x21d0u && codepoint <= 0x21d5u) {
		return terminus_block88[codepoint - 0x21d0u];
	} else if (codepoint == 0x2200u) {
		return terminus_block89[0];
	} else if (codepoint >= 0x2203u && codepoint <= 0x220du) {
		return terminus_block90[codepoint - 0x2203u];
	} else if (codepoint >= 0x2212u && codepoint <= 0x2216u) {
		return terminus_block91[codepoint - 0x2212u];
	} else if (codepoint >= 0x2219u && codepoint <= 0x221au) {
		return terminus_block92[codepoint - 0x2219u];
	} else if (codepoint >= 0x221eu && codepoint <= 0x221fu) {
		return terminus_block93[codepoint - 0x221eu];
	} else if (codepoint == 0x2225u) {
		return terminus_block94[0];
	} else if (codepoint >= 0x2227u && codepoint <= 0x222au) {
		return terminus_block95[codepoint - 0x2227u];
	} else if (codepoint == 0x2248u) {
		return terminus_block96[0];
	} else if (codepoint >= 0x2260u && codepoint <= 0x2261u) {
		return terminus_block97[codepoint - 0x2260u];
	} else if (codepoint >= 0x2264u && codepoint <= 0x2265u) {
		return terminus_block98[codepoint - 0x2264u];
	} else if (codepoint >= 0x226au && codepoint <= 0x226bu) {
		return terminus_block99[codepoint - 0x226au];
	} else if (codepoint >= 0x2282u && codepoint <= 0x2283u) {
		return terminus_block100[codepoint - 0x2282u];
	} else if (codepoint >= 0x2286u && codepoint <= 0x2287u) {
		return terminus_block101[codepoint - 0x2286u];
	} else if (codepoint == 0x22a5u) {
		return terminus_block102[0];
	} else if (codepoint >= 0x22c2u && codepoint <= 0x22c3u) {
		return terminus_block103[codepoint - 0x22c2u];
	} else if (codepoint == 0x2300u) {
		return terminus_block104[0];
	} else if (codepoint == 0x2302u) {
		return terminus_block105[0];
	} else if (codepoint >= 0x2308u && codepoint <= 0x230bu) {
		return terminus_block106[codepoint - 0x2308u];
	} else if (codepoint == 0x2310u) {
		return terminus_block107[0];
	} else if (codepoint == 0x2319u) {
		return terminus_block108[0];
	} else if (codepoint >= 0x2320u && codepoint <= 0x2321u) {
		return terminus_block109[codepoint - 0x2320u];
	} else if (codepoint >= 0x239bu && codepoint <= 0x23a9u) {
		return terminus_block110[codepoint - 0x239bu];
	} else if (codepoint >= 0x23abu && codepoint <= 0x23afu) {
		return terminus_block111[codepoint - 0x23abu];
	} else if (codepoint >= 0x23bau && codepoint <= 0x23bdu) {
		return terminus_block112[codepoint - 0x23bau];
	} else if (codepoint == 0x23d0u) {
		return terminus_block113[0];
	} else if (codepoint >= 0x2409u && codepoint <= 0x240du) {
		return terminus_block114[codepoint - 0x2409u];
	} else if (codepoint == 0x2424u) {
		return terminus_block115[0];
	} else if (codepoint >= 0x2500u && codepoint <= 0x2503u) {
		return terminus_block116[codepoint - 0x2500u];
	} else if (codepoint >= 0x2508u && codepoint <= 0x254bu) {
		return terminus_block117[codepoint - 0x2508u];
	} else if (codepoint >= 0x2550u && codepoint <= 0x2593u) {
		return terminus_block118[codepoint - 0x2550u];
	} else if (codepoint >= 0x2596u && codepoint <= 0x25a0u) {
		return terminus_block119[codepoint - 0x2596u];
	} else if (codepoint == 0x25acu) {
		return terminus_block120[0];
	} else if (codepoint == 0x25aeu) {
		return terminus_block121[0];
	} else if (codepoint == 0x25b2u) {
		return terminus_block122[0];
	} else if (codepoint == 0x25b6u) {
		return terminus_block123[0];
	} else if (codepoint == 0x25bau) {
		return terminus_block124[0];
	} else if (codepoint == 0x25bcu) {
		return terminus_block125[0];
	} else if (codepoint == 0x25c0u) {
		return terminus_block126[0];
	} else if (codepoint == 0x25c4u) {
		return terminus_block127[0];
	} else if (codepoint == 0x25c6u) {
		return terminus_block128[0];
	} else if (codepoint >= 0x25cau && codepoint <= 0x25cbu) {
		return terminus_block129[codepoint - 0x25cau];
	} else if (codepoint == 0x25cfu) {
		return terminus_block130[0];
	} else if (codepoint >= 0x25d8u && codepoint <= 0x25d9u) {
		return terminus_block131[codepoint - 0x25d8u];
	} else if (codepoint >= 0x263au && codepoint <= 0x263cu) {
		return terminus_block132[codepoint - 0x263au];
	} else if (codepoint == 0x2640u) {
		return terminus_block133[0];
	} else if (codepoint == 0x2642u) {
		return terminus_block134[0];
	} else if (codepoint == 0x2660u) {
		return terminus_block135[0];
	} else if (codepoint == 0x2663u) {
		return terminus_block136[0];
	} else if (codepoint >= 0x2665u && codepoint <= 0x2666u) {
		return terminus_block137[codepoint - 0x2665u];
	} else if (codepoint >= 0x266au && codepoint <= 0x266bu) {
		return terminus_block138[codepoint - 0x266au];
	} else if (codepoint >= 0x2713u && codepoint <= 0x2714u) {
		return terminus_block139[codepoint - 0x2713u];
	} else if (codepoint >= 0x2717u && codepoint <= 0x2718u) {
		return terminus_block140[codepoint - 0x2717u];
	} else if (codepoint >= 0x27e8u && codepoint <= 0x27ebu) {
		return terminus_block141[codepoint - 0x27e8u];
	} else if (codepoint >= 0x2800u && codepoint <= 0x28ffu) {
		return terminus_block142[codepoint - 0x2800u];
	} else if (codepoint == 0x2e2cu) {
		return terminus_block143[0];
	} else if (codepoint >= 0xe0a0u && codepoint <= 0xe0a2u) {
		return terminus_block144[codepoint - 0xe0a0u];
	} else if (codepoint >= 0xe0b0u && codepoint <= 0xe0b3u) {
		return terminus_block145[codepoint - 0xe0b0u];
	} else if (codepoint == 0xf6beu) {
		return terminus_block146[0];
	} else if (codepoint == 0xfffdu) {
		return terminus_block147[0];
	} else {
		WARN("Codepoint U+%04X (%s) is not covered by this font", codepoint, u8_cp_to_utf8(codepoint));
		return terminus_block1[0];
	}
}

static const unsigned char*
    terminusb_get_bitmap(uint32_t codepoint)
{
	if (codepoint == 0x00u) {
		return terminusb_block1[codepoint];
	} else if (codepoint >= 0x20u && codepoint <= 0x7eu) {
		return terminusb_block2[codepoint - 0x20u];
	} else if (codepoint >= 0xa0u && codepoint <= 0x17fu) {
		return terminusb_block3[codepoint - 0xa0u];
	} else if (codepoint == 0x186u) {
		return terminusb_block4[0];
	} else if (codepoint >= 0x18eu && codepoint <= 0x190u) {
		return terminusb_block5[codepoint - 0x18eu];
	} else if (codepoint == 0x192u) {
		return terminusb_block6[0];
	} else if (codepoint >= 0x19du && codepoint <= 0x19eu) {
		return terminusb_block7[codepoint - 0x19du];
	} else if (codepoint >= 0x1b5u && codepoint <= 0x1b7u) {
		return terminusb_block8[codepoint - 0x1b5u];
	} else if (codepoint >= 0x1cdu && codepoint <= 0x1d4u) {
		return terminusb_block9[codepoint - 0x1cdu];
	} else if (codepoint >= 0x1e2u && codepoint <= 0x1f0u) {
		return terminusb_block10[codepoint - 0x1e2u];
	} else if (codepoint >= 0x1f4u && codepoint <= 0x1f5u) {
		return terminusb_block11[codepoint - 0x1f4u];
	} else if (codepoint >= 0x1fcu && codepoint <= 0x1ffu) {
		return terminusb_block12[codepoint - 0x1fcu];
	} else if (codepoint >= 0x218u && codepoint <= 0x21bu) {
		return terminusb_block13[codepoint - 0x218u];
	} else if (codepoint >= 0x232u && codepoint <= 0x233u) {
		return terminusb_block14[codepoint - 0x232u];
	} else if (codepoint == 0x237u) {
		return terminusb_block15[0];
	} else if (codepoint == 0x254u) {
		return terminusb_block16[0];
	} else if (codepoint >= 0x258u && codepoint <= 0x259u) {
		return terminusb_block17[codepoint - 0x258u];
	} else if (codepoint == 0x25bu) {
		return terminusb_block18[0];
	} else if (codepoint == 0x272u) {
		return terminusb_block19[0];
	} else if (codepoint == 0x292u) {
		return terminusb_block20[0];
	} else if (codepoint >= 0x2bbu && codepoint <= 0x2bdu) {
		return terminusb_block21[codepoint - 0x2bbu];
	} else if (codepoint >= 0x2c6u && codepoint <= 0x2c7u) {
		return terminusb_block22[codepoint - 0x2c6u];
	} else if (codepoint >= 0x2d8u && codepoint <= 0x2d9u) {
		return terminusb_block23[codepoint - 0x2d8u];
	} else if (codepoint >= 0x2dbu && codepoint <= 0x2ddu) {
		return terminusb_block24[codepoint - 0x2dbu];
	} else if (codepoint >= 0x300u && codepoint <= 0x308u) {
		return terminusb_block25[codepoint - 0x300u];
	} else if (codepoint >= 0x30au && codepoint <= 0x30cu) {
		return terminusb_block26[codepoint - 0x30au];
	} else if (codepoint == 0x329u) {
		return terminusb_block27[0];
	} else if (codepoint >= 0x384u && codepoint <= 0x38au) {
		return terminusb_block28[codepoint - 0x384u];
	} else if (codepoint == 0x38cu) {
		return terminusb_block29[0];
	} else if (codepoint >= 0x38eu && codepoint <= 0x3a1u) {
		return terminusb_block30[codepoint - 0x38eu];
	} else if (codepoint >= 0x3a3u && codepoint <= 0x3ceu) {
		return terminusb_block31[codepoint - 0x3a3u];
	} else if (codepoint == 0x3d1u) {
		return terminusb_block32[0];
	} else if (codepoint == 0x3d5u) {
		return terminusb_block33[0];
	} else if (codepoint >= 0x3f0u && codepoint <= 0x3f6u) {
		return terminusb_block34[codepoint - 0x3f0u];
	} else if (codepoint >= 0x400u && codepoint <= 0x45fu) {
		return terminusb_block35[codepoint - 0x400u];
	} else if (codepoint >= 0x462u && codepoint <= 0x463u) {
		return terminusb_block36[codepoint - 0x462u];
	} else if (codepoint >= 0x46au && codepoint <= 0x46bu) {
		return terminusb_block37[codepoint - 0x46au];
	} else if (codepoint >= 0x490u && codepoint <= 0x49du) {
		return terminusb_block38[codepoint - 0x490u];
	} else if (codepoint >= 0x4a0u && codepoint <= 0x4a5u) {
		return terminusb_block39[codepoint - 0x4a0u];
	} else if (codepoint >= 0x4aau && codepoint <= 0x4abu) {
		return terminusb_block40[codepoint - 0x4aau];
	} else if (codepoint >= 0x4aeu && codepoint <= 0x4b3u) {
		return terminusb_block41[codepoint - 0x4aeu];
	} else if (codepoint >= 0x4b6u && codepoint <= 0x4bbu) {
		return terminusb_block42[codepoint - 0x4b6u];
	} else if (codepoint >= 0x4c0u && codepoint <= 0x4c2u) {
		return terminusb_block43[codepoint - 0x4c0u];
	} else if (codepoint >= 0x4cfu && codepoint <= 0x4dfu) {
		return terminusb_block44[codepoint - 0x4cfu];
	} else if (codepoint >= 0x4e2u && codepoint <= 0x4f5u) {
		return terminusb_block45[codepoint - 0x4e2u];
	} else if (codepoint >= 0x4f8u && codepoint <= 0x4f9u) {
		return terminusb_block46[codepoint - 0x4f8u];
	} else if (codepoint >= 0x5d0u && codepoint <= 0x5eau) {
		return terminusb_block47[codepoint - 0x5d0u];
	} else if (codepoint >= 0x1e0cu && codepoint <= 0x1e0du) {
		return terminusb_block48[codepoint - 0x1e0cu];
	} else if (codepoint >= 0x1e34u && codepoint <= 0x1e37u) {
		return terminusb_block49[codepoint - 0x1e34u];
	} else if (codepoint >= 0x1e40u && codepoint <= 0x1e47u) {
		return terminusb_block50[codepoint - 0x1e40u];
	} else if (codepoint >= 0x1e6cu && codepoint <= 0x1e6du) {
		return terminusb_block51[codepoint - 0x1e6cu];
	} else if (codepoint >= 0x1eb8u && codepoint <= 0x1eb9u) {
		return terminusb_block52[codepoint - 0x1eb8u];
	} else if (codepoint >= 0x1ebcu && codepoint <= 0x1ebdu) {
		return terminusb_block53[codepoint - 0x1ebcu];
	} else if (codepoint >= 0x1ecau && codepoint <= 0x1ecdu) {
		return terminusb_block54[codepoint - 0x1ecau];
	} else if (codepoint >= 0x1ee4u && codepoint <= 0x1ee5u) {
		return terminusb_block55[codepoint - 0x1ee4u];
	} else if (codepoint >= 0x1ef8u && codepoint <= 0x1ef9u) {
		return terminusb_block56[codepoint - 0x1ef8u];
	} else if (codepoint >= 0x2000u && codepoint <= 0x2022u) {
		return terminusb_block57[codepoint - 0x2000u];
	} else if (codepoint == 0x2026u) {
		return terminusb_block58[0];
	} else if (codepoint == 0x2030u) {
		return terminusb_block59[0];
	} else if (codepoint >= 0x2032u && codepoint <= 0x2033u) {
		return terminusb_block60[codepoint - 0x2032u];
	} else if (codepoint >= 0x2039u && codepoint <= 0x203au) {
		return terminusb_block61[codepoint - 0x2039u];
	} else if (codepoint == 0x203cu) {
		return terminusb_block62[0];
	} else if (codepoint == 0x203eu) {
		return terminusb_block63[0];
	} else if (codepoint >= 0x2070u && codepoint <= 0x2071u) {
		return terminusb_block64[codepoint - 0x2070u];
	} else if (codepoint >= 0x2074u && codepoint <= 0x208eu) {
		return terminusb_block65[codepoint - 0x2074u];
	} else if (codepoint >= 0x2090u && codepoint <= 0x2098u) {
		return terminusb_block66[codepoint - 0x2090u];
	} else if (codepoint == 0x209au) {
		return terminusb_block67[0];
	} else if (codepoint == 0x20a7u) {
		return terminusb_block68[0];
	} else if (codepoint == 0x20aau) {
		return terminusb_block69[0];
	} else if (codepoint == 0x20acu) {
		return terminusb_block70[0];
	} else if (codepoint == 0x20aeu) {
		return terminusb_block71[0];
	} else if (codepoint == 0x2102u) {
		return terminusb_block72[0];
	} else if (codepoint >= 0x210eu && codepoint <= 0x210fu) {
		return terminusb_block73[codepoint - 0x210eu];
	} else if (codepoint >= 0x2115u && codepoint <= 0x2116u) {
		return terminusb_block74[codepoint - 0x2115u];
	} else if (codepoint == 0x211au) {
		return terminusb_block75[0];
	} else if (codepoint == 0x211du) {
		return terminusb_block76[0];
	} else if (codepoint == 0x2122u) {
		return terminusb_block77[0];
	} else if (codepoint == 0x2124u) {
		return terminusb_block78[0];
	} else if (codepoint == 0x2126u) {
		return terminusb_block79[0];
	} else if (codepoint == 0x2135u) {
		return terminusb_block80[0];
	} else if (codepoint >= 0x2190u && codepoint <= 0x2195u) {
		return terminusb_block81[codepoint - 0x2190u];
	} else if (codepoint == 0x21a4u) {
		return terminusb_block82[0];
	} else if (codepoint == 0x21a6u) {
		return terminusb_block83[0];
	} else if (codepoint == 0x21a8u) {
		return terminusb_block84[0];
	} else if (codepoint == 0x21b5u) {
		return terminusb_block85[0];
	} else if (codepoint == 0x21bbu) {
		return terminusb_block86[0];
	} else if (codepoint >= 0x21cbu && codepoint <= 0x21ccu) {
		return terminusb_block87[codepoint - 0x21cbu];
	} else if (codepoint >= 0x21d0u && codepoint <= 0x21d5u) {
		return terminusb_block88[codepoint - 0x21d0u];
	} else if (codepoint == 0x2200u) {
		return terminusb_block89[0];
	} else if (codepoint >= 0x2203u && codepoint <= 0x220du) {
		return terminusb_block90[codepoint - 0x2203u];
	} else if (codepoint >= 0x2212u && codepoint <= 0x2216u) {
		return terminusb_block91[codepoint - 0x2212u];
	} else if (codepoint >= 0x2219u && codepoint <= 0x221au) {
		return terminusb_block92[codepoint - 0x2219u];
	} else if (codepoint >= 0x221eu && codepoint <= 0x221fu) {
		return terminusb_block93[codepoint - 0x221eu];
	} else if (codepoint == 0x2225u) {
		return terminusb_block94[0];
	} else if (codepoint >= 0x2227u && codepoint <= 0x222au) {
		return terminusb_block95[codepoint - 0x2227u];
	} else if (codepoint == 0x2248u) {
		return terminusb_block96[0];
	} else if (codepoint >= 0x2260u && codepoint <= 0x2261u) {
		return terminusb_block97[codepoint - 0x2260u];
	} else if (codepoint >= 0x2264u && codepoint <= 0x2265u) {
		return terminusb_block98[codepoint - 0x2264u];
	} else if (codepoint >= 0x226au && codepoint <= 0x226bu) {
		return terminusb_block99[codepoint - 0x226au];
	} else if (codepoint >= 0x2282u && codepoint <= 0x2283u) {
		return terminusb_block100[codepoint - 0x2282u];
	} else if (codepoint >= 0x2286u && codepoint <= 0x2287u) {
		return terminusb_block101[codepoint - 0x2286u];
	} else if (codepoint == 0x22a5u) {
		return terminusb_block102[0];
	} else if (codepoint >= 0x22c2u && codepoint <= 0x22c3u) {
		return terminusb_block103[codepoint - 0x22c2u];
	} else if (codepoint == 0x2300u) {
		return terminusb_block104[0];
	} else if (codepoint == 0x2302u) {
		return terminusb_block105[0];
	} else if (codepoint >= 0x2308u && codepoint <= 0x230bu) {
		return terminusb_block106[codepoint - 0x2308u];
	} else if (codepoint == 0x2310u) {
		return terminusb_block107[0];
	} else if (codepoint == 0x2319u) {
		return terminusb_block108[0];
	} else if (codepoint >= 0x2320u && codepoint <= 0x2321u) {
		return terminusb_block109[codepoint - 0x2320u];
	} else if (codepoint >= 0x239bu && codepoint <= 0x23a9u) {
		return terminusb_block110[codepoint - 0x239bu];
	} else if (codepoint >= 0x23abu && codepoint <= 0x23afu) {
		return terminusb_block111[codepoint - 0x23abu];
	} else if (codepoint >= 0x23bau && codepoint <= 0x23bdu) {
		return terminusb_block112[codepoint - 0x23bau];
	} else if (codepoint == 0x23d0u) {
		return terminusb_block113[0];
	} else if (codepoint >= 0x2409u && codepoint <= 0x240du) {
		return terminusb_block114[codepoint - 0x2409u];
	} else if (codepoint == 0x2424u) {
		return terminusb_block115[0];
	} else if (codepoint >= 0x2500u && codepoint <= 0x2503u) {
		return terminusb_block116[codepoint - 0x2500u];
	} else if (codepoint >= 0x2508u && codepoint <= 0x254bu) {
		return terminusb_block117[codepoint - 0x2508u];
	} else if (codepoint >= 0x2550u && codepoint <= 0x2593u) {
		return terminusb_block118[codepoint - 0x2550u];
	} else if (codepoint >= 0x2596u && codepoint <= 0x25a0u) {
		return terminusb_block119[codepoint - 0x2596u];
	} else if (codepoint == 0x25acu) {
		return terminusb_block120[0];
	} else if (codepoint == 0x25aeu) {
		return terminusb_block121[0];
	} else if (codepoint == 0x25b2u) {
		return terminusb_block122[0];
	} else if (codepoint == 0x25b6u) {
		return terminusb_block123[0];
	} else if (codepoint == 0x25bau) {
		return terminusb_block124[0];
	} else if (codepoint == 0x25bcu) {
		return terminusb_block125[0];
	} else if (codepoint == 0x25c0u) {
		return terminusb_block126[0];
	} else if (codepoint == 0x25c4u) {
		return terminusb_block127[0];
	} else if (codepoint == 0x25c6u) {
		return terminusb_block128[0];
	} else if (codepoint >= 0x25cau && codepoint <= 0x25cbu) {
		return terminusb_block129[codepoint - 0x25cau];
	} else if (codepoint == 0x25cfu) {
		return terminusb_block130[0];
	} else if (codepoint >= 0x25d8u && codepoint <= 0x25d9u) {
		return terminusb_block131[codepoint - 0x25d8u];
	} else if (codepoint >= 0x263au && codepoint <= 0x263cu) {
		return terminusb_block132[codepoint - 0x263au];
	} else if (codepoint == 0x2640u) {
		return terminusb_block133[0];
	} else if (codepoint == 0x2642u) {
		return terminusb_block134[0];
	} else if (codepoint == 0x2660u) {
		return terminusb_block135[0];
	} else if (codepoint == 0x2663u) {
		return terminusb_block136[0];
	} else if (codepoint >= 0x2665u && codepoint <= 0x2666u) {
		return terminusb_block137[codepoint - 0x2665u];
	} else if (codepoint >= 0x266au && codepoint <= 0x266bu) {
		return terminusb_block138[codepoint - 0x266au];
	} else if (codepoint >= 0x2713u && codepoint <= 0x2714u) {
		return terminusb_block139[codepoint - 0x2713u];
	} else if (codepoint >= 0x2717u && codepoint <= 0x2718u) {
		return terminusb_block140[codepoint - 0x2717u];
	} else if (codepoint >= 0x27e8u && codepoint <= 0x27ebu) {
		return terminusb_block141[codepoint - 0x27e8u];
	} else if (codepoint >= 0x2800u && codepoint <= 0x28ffu) {
		return terminusb_block142[codepoint - 0x2800u];
	} else if (codepoint == 0x2e2cu) {
		return terminusb_block143[0];
	} else if (codepoint >= 0xe0a0u && codepoint <= 0xe0a2u) {
		return terminusb_block144[codepoint - 0xe0a0u];
	} else if (codepoint >= 0xe0b0u && codepoint <= 0xe0b3u) {
		return terminusb_block145[codepoint - 0xe0b0u];
	} else if (codepoint == 0xf6beu) {
		return terminusb_block146[0];
	} else if (codepoint == 0xfffdu) {
		return terminusb_block147[0];
	} else {
		WARN("Codepoint U+%04X (%s) is not covered by this font", codepoint, u8_cp_to_utf8(codepoint));
		return terminusb_block1[0];
	}
}
