/*
	FBInk: FrameBuffer eInker, a library to print text & images to an eInk Linux framebuffer
	Copyright (C) 2018-2020 NiLuJe <ninuje@gmail.com>
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
	} else if (codepoint >= 0x1cdu && codepoint <= 0x1e3u) {
		return unscii_block3[codepoint - 0x1cdu];
	} else if (codepoint >= 0x1e6u && codepoint <= 0x1e8u) {
		return unscii_block4[codepoint - 0x1e6u];
	} else if (codepoint == 0x1f0u) {
		return unscii_block5[0];
	} else if (codepoint >= 0x1f4u && codepoint <= 0x1f5u) {
		return unscii_block6[codepoint - 0x1f4u];
	} else if (codepoint >= 0x1f8u && codepoint <= 0x21bu) {
		return unscii_block7[codepoint - 0x1f8u];
	} else if (codepoint >= 0x226u && codepoint <= 0x227u) {
		return unscii_block8[codepoint - 0x226u];
	} else if (codepoint >= 0x22au && codepoint <= 0x233u) {
		return unscii_block9[codepoint - 0x22au];
	} else if (codepoint >= 0x258u && codepoint <= 0x259u) {
		return unscii_block10[codepoint - 0x258u];
	} else if (codepoint == 0x263u) {
		return unscii_block11[0];
	} else if (codepoint == 0x294u) {
		return unscii_block12[0];
	} else if (codepoint == 0x2c6u) {
		return unscii_block13[0];
	} else if (codepoint == 0x2c9u) {
		return unscii_block14[0];
	} else if (codepoint == 0x2cdu) {
		return unscii_block15[0];
	} else if (codepoint == 0x2dcu) {
		return unscii_block16[0];
	} else if (codepoint == 0x34fu) {
		return unscii_block17[0];
	} else if (codepoint >= 0x391u && codepoint <= 0x3a1u) {
		return unscii_block18[codepoint - 0x391u];
	} else if (codepoint >= 0x3a3u && codepoint <= 0x3a9u) {
		return unscii_block19[codepoint - 0x3a3u];
	} else if (codepoint >= 0x3b1u && codepoint <= 0x3c9u) {
		return unscii_block20[codepoint - 0x3b1u];
	} else if (codepoint >= 0x400u && codepoint <= 0x401u) {
		return unscii_block21[codepoint - 0x400u];
	} else if (codepoint >= 0x405u && codepoint <= 0x408u) {
		return unscii_block22[codepoint - 0x405u];
	} else if (codepoint >= 0x410u && codepoint <= 0x451u) {
		return unscii_block23[codepoint - 0x410u];
	} else if (codepoint >= 0x455u && codepoint <= 0x458u) {
		return unscii_block24[codepoint - 0x455u];
	} else if (codepoint >= 0x4d0u && codepoint <= 0x4d7u) {
		return unscii_block25[codepoint - 0x4d0u];
	} else if (codepoint >= 0x4e6u && codepoint <= 0x4e7u) {
		return unscii_block26[codepoint - 0x4e6u];
	} else if (codepoint == 0x4f1u) {
		return unscii_block27[0];
	} else if (codepoint >= 0x5d0u && codepoint <= 0x5eau) {
		return unscii_block28[codepoint - 0x5d0u];
	} else if (codepoint == 0x623u) {
		return unscii_block29[0];
	} else if (codepoint >= 0x627u && codepoint <= 0x62du) {
		return unscii_block30[codepoint - 0x627u];
	} else if (codepoint >= 0x62fu && codepoint <= 0x635u) {
		return unscii_block31[codepoint - 0x62fu];
	} else if (codepoint >= 0x637u && codepoint <= 0x63au) {
		return unscii_block32[codepoint - 0x637u];
	} else if (codepoint >= 0x640u && codepoint <= 0x64au) {
		return unscii_block33[codepoint - 0x640u];
	} else if (codepoint >= 0x671u && codepoint <= 0x673u) {
		return unscii_block34[codepoint - 0x671u];
	} else if (codepoint >= 0x1560u && codepoint <= 0x1563u) {
		return unscii_block35[codepoint - 0x1560u];
	} else if (codepoint == 0x156au) {
		return unscii_block36[0];
	} else if (codepoint >= 0x156cu && codepoint <= 0x1571u) {
		return unscii_block37[codepoint - 0x156cu];
	} else if (codepoint >= 0x15ecu && codepoint <= 0x15edu) {
		return unscii_block38[codepoint - 0x15ecu];
	} else if (codepoint == 0x15efu) {
		return unscii_block39[0];
	} else if (codepoint >= 0x16a0u && codepoint <= 0x16abu) {
		return unscii_block40[codepoint - 0x16a0u];
	} else if (codepoint >= 0x16b1u && codepoint <= 0x16f8u) {
		return unscii_block41[codepoint - 0x16b1u];
	} else if (codepoint >= 0x1e00u && codepoint <= 0x1e02u) {
		return unscii_block42[codepoint - 0x1e00u];
	} else if (codepoint >= 0x1e04u && codepoint <= 0x1e07u) {
		return unscii_block43[codepoint - 0x1e04u];
	} else if (codepoint == 0x1e0au) {
		return unscii_block44[0];
	} else if (codepoint >= 0x1e0cu && codepoint <= 0x1e0fu) {
		return unscii_block45[codepoint - 0x1e0cu];
	} else if (codepoint >= 0x1e41u && codepoint <= 0x1e4bu) {
		return unscii_block46[codepoint - 0x1e41u];
	} else if (codepoint >= 0x1e58u && codepoint <= 0x1e5bu) {
		return unscii_block47[codepoint - 0x1e58u];
	} else if (codepoint == 0x1e5eu) {
		return unscii_block48[0];
	} else if (codepoint >= 0x2010u && codepoint <= 0x2019u) {
		return unscii_block49[codepoint - 0x2010u];
	} else if (codepoint >= 0x201cu && codepoint <= 0x201du) {
		return unscii_block50[codepoint - 0x201cu];
	} else if (codepoint >= 0x2020u && codepoint <= 0x2022u) {
		return unscii_block51[codepoint - 0x2020u];
	} else if (codepoint >= 0x2024u && codepoint <= 0x2026u) {
		return unscii_block52[codepoint - 0x2024u];
	} else if (codepoint >= 0x2030u && codepoint <= 0x2031u) {
		return unscii_block53[codepoint - 0x2030u];
	} else if (codepoint == 0x203cu) {
		return unscii_block54[0];
	} else if (codepoint >= 0x2047u && codepoint <= 0x2049u) {
		return unscii_block55[codepoint - 0x2047u];
	} else if (codepoint == 0x2070u) {
		return unscii_block56[0];
	} else if (codepoint >= 0x2074u && codepoint <= 0x2079u) {
		return unscii_block57[codepoint - 0x2074u];
	} else if (codepoint == 0x207fu) {
		return unscii_block58[0];
	} else if (codepoint == 0x20a7u) {
		return unscii_block59[0];
	} else if (codepoint == 0x20acu) {
		return unscii_block60[0];
	} else if (codepoint == 0x2117u) {
		return unscii_block61[0];
	} else if (codepoint == 0x2120u) {
		return unscii_block62[0];
	} else if (codepoint == 0x2122u) {
		return unscii_block63[0];
	} else if (codepoint >= 0x2190u && codepoint <= 0x2192u) {
		return unscii_block64[codepoint - 0x2190u];
	} else if (codepoint == 0x2194u) {
		return unscii_block65[0];
	} else if (codepoint >= 0x2196u && codepoint <= 0x2199u) {
		return unscii_block66[codepoint - 0x2196u];
	} else if (codepoint == 0x21a5u) {
		return unscii_block67[0];
	} else if (codepoint == 0x21a8u) {
		return unscii_block68[0];
	} else if (codepoint >= 0x21e6u && codepoint <= 0x21e9u) {
		return unscii_block69[codepoint - 0x21e6u];
	} else if (codepoint == 0x2212u) {
		return unscii_block70[0];
	} else if (codepoint == 0x2218u) {
		return unscii_block71[0];
	} else if (codepoint == 0x221au) {
		return unscii_block72[0];
	} else if (codepoint >= 0x221eu && codepoint <= 0x221fu) {
		return unscii_block73[codepoint - 0x221eu];
	} else if (codepoint == 0x2229u) {
		return unscii_block74[0];
	} else if (codepoint == 0x2248u) {
		return unscii_block75[0];
	} else if (codepoint == 0x2261u) {
		return unscii_block76[0];
	} else if (codepoint >= 0x2264u && codepoint <= 0x2265u) {
		return unscii_block77[codepoint - 0x2264u];
	} else if (codepoint == 0x22c5u) {
		return unscii_block78[0];
	} else if (codepoint >= 0x2320u && codepoint <= 0x2321u) {
		return unscii_block79[codepoint - 0x2320u];
	} else if (codepoint >= 0x23eau && codepoint <= 0x23ecu) {
		return unscii_block80[codepoint - 0x23eau];
	} else if (codepoint >= 0x2500u && codepoint <= 0x25ffu) {
		return unscii_block81[codepoint - 0x2500u];
	} else if (codepoint == 0x2625u) {
		return unscii_block82[0];
	} else if (codepoint == 0x2628u) {
		return unscii_block83[0];
	} else if (codepoint >= 0x262fu && codepoint <= 0x2637u) {
		return unscii_block84[codepoint - 0x262fu];
	} else if (codepoint >= 0x2639u && codepoint <= 0x263bu) {
		return unscii_block85[codepoint - 0x2639u];
	} else if (codepoint >= 0x2660u && codepoint <= 0x2667u) {
		return unscii_block86[codepoint - 0x2660u];
	} else if (codepoint >= 0x2669u && codepoint <= 0x266cu) {
		return unscii_block87[codepoint - 0x2669u];
	} else if (codepoint >= 0x268au && codepoint <= 0x268fu) {
		return unscii_block88[codepoint - 0x268au];
	} else if (codepoint >= 0x26aau && codepoint <= 0x26acu) {
		return unscii_block89[codepoint - 0x26aau];
	} else if (codepoint == 0x2708u) {
		return unscii_block90[0];
	} else if (codepoint == 0x2734u) {
		return unscii_block91[0];
	} else if (codepoint >= 0x2800u && codepoint <= 0x28ffu) {
		return unscii_block92[codepoint - 0x2800u];
	} else if (codepoint == 0x2913u) {
		return unscii_block93[0];
	} else if (codepoint == 0x2b1du) {
		return unscii_block94[0];
	} else if (codepoint == 0x2b24u) {
		return unscii_block95[0];
	} else if (codepoint == 0x2b55u) {
		return unscii_block96[0];
	} else if (codepoint == 0x2b58u) {
		return unscii_block97[0];
	} else if (codepoint == 0x2e2eu) {
		return unscii_block98[0];
	} else if (codepoint >= 0xe080u && codepoint <= 0xe19bu) {
		return unscii_block99[codepoint - 0xe080u];
	} else if (codepoint >= 0xe800u && codepoint <= 0xe850u) {
		return unscii_block100[codepoint - 0xe800u];
	} else if (codepoint >= 0xec00u && codepoint <= 0xec26u) {
		return unscii_block101[codepoint - 0xec00u];
	} else if (codepoint >= 0xfe81u && codepoint <= 0xfe82u) {
		return unscii_block102[codepoint - 0xfe81u];
	} else if (codepoint == 0xfe84u) {
		return unscii_block103[0];
	} else if (codepoint == 0xfe87u) {
		return unscii_block104[0];
	} else if (codepoint >= 0xfe8fu && codepoint <= 0xfe9cu) {
		return unscii_block105[codepoint - 0xfe8fu];
	} else if (codepoint >= 0xfe9eu && codepoint <= 0xfea4u) {
		return unscii_block106[codepoint - 0xfe9eu];
	} else if (codepoint >= 0xfea9u && codepoint <= 0xfeadu) {
		return unscii_block107[codepoint - 0xfea9u];
	} else if (codepoint == 0xfeafu) {
		return unscii_block108[0];
	} else if (codepoint >= 0xfeb1u && codepoint <= 0xfeb7u) {
		return unscii_block109[codepoint - 0xfeb1u];
	} else if (codepoint >= 0xfeb9u && codepoint <= 0xfebcu) {
		return unscii_block110[codepoint - 0xfeb9u];
	} else if (codepoint >= 0xfec1u && codepoint <= 0xfec6u) {
		return unscii_block111[codepoint - 0xfec1u];
	} else if (codepoint >= 0xfec9u && codepoint <= 0xfeddu) {
		return unscii_block112[codepoint - 0xfec9u];
	} else if (codepoint >= 0xfedfu && codepoint <= 0xfee3u) {
		return unscii_block113[codepoint - 0xfedfu];
	} else if (codepoint >= 0xfee5u && codepoint <= 0xfee9u) {
		return unscii_block114[codepoint - 0xfee5u];
	} else if (codepoint >= 0xfeebu && codepoint <= 0xfef4u) {
		return unscii_block115[codepoint - 0xfeebu];
	} else if (codepoint >= 0xff61u && codepoint <= 0xff9fu) {
		return unscii_block116[codepoint - 0xff61u];
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
	} else if (codepoint >= 0x1cdu && codepoint <= 0x1e3u) {
		return alt_block3[codepoint - 0x1cdu];
	} else if (codepoint >= 0x1e6u && codepoint <= 0x1e8u) {
		return alt_block4[codepoint - 0x1e6u];
	} else if (codepoint == 0x1f0u) {
		return alt_block5[0];
	} else if (codepoint >= 0x1f4u && codepoint <= 0x1f5u) {
		return alt_block6[codepoint - 0x1f4u];
	} else if (codepoint >= 0x1f8u && codepoint <= 0x21bu) {
		return alt_block7[codepoint - 0x1f8u];
	} else if (codepoint >= 0x226u && codepoint <= 0x227u) {
		return alt_block8[codepoint - 0x226u];
	} else if (codepoint >= 0x22au && codepoint <= 0x233u) {
		return alt_block9[codepoint - 0x22au];
	} else if (codepoint >= 0x258u && codepoint <= 0x259u) {
		return alt_block10[codepoint - 0x258u];
	} else if (codepoint == 0x263u) {
		return alt_block11[0];
	} else if (codepoint == 0x294u) {
		return alt_block12[0];
	} else if (codepoint == 0x2c6u) {
		return alt_block13[0];
	} else if (codepoint == 0x2c9u) {
		return alt_block14[0];
	} else if (codepoint == 0x2cdu) {
		return alt_block15[0];
	} else if (codepoint == 0x2dcu) {
		return alt_block16[0];
	} else if (codepoint == 0x34fu) {
		return alt_block17[0];
	} else if (codepoint >= 0x391u && codepoint <= 0x3a1u) {
		return alt_block18[codepoint - 0x391u];
	} else if (codepoint >= 0x3a3u && codepoint <= 0x3a9u) {
		return alt_block19[codepoint - 0x3a3u];
	} else if (codepoint >= 0x3b1u && codepoint <= 0x3c9u) {
		return alt_block20[codepoint - 0x3b1u];
	} else if (codepoint >= 0x400u && codepoint <= 0x401u) {
		return alt_block21[codepoint - 0x400u];
	} else if (codepoint >= 0x405u && codepoint <= 0x408u) {
		return alt_block22[codepoint - 0x405u];
	} else if (codepoint >= 0x410u && codepoint <= 0x451u) {
		return alt_block23[codepoint - 0x410u];
	} else if (codepoint >= 0x455u && codepoint <= 0x458u) {
		return alt_block24[codepoint - 0x455u];
	} else if (codepoint >= 0x4d0u && codepoint <= 0x4d7u) {
		return alt_block25[codepoint - 0x4d0u];
	} else if (codepoint >= 0x4e6u && codepoint <= 0x4e7u) {
		return alt_block26[codepoint - 0x4e6u];
	} else if (codepoint == 0x4f1u) {
		return alt_block27[0];
	} else if (codepoint >= 0x5d0u && codepoint <= 0x5eau) {
		return alt_block28[codepoint - 0x5d0u];
	} else if (codepoint == 0x623u) {
		return alt_block29[0];
	} else if (codepoint >= 0x627u && codepoint <= 0x62du) {
		return alt_block30[codepoint - 0x627u];
	} else if (codepoint >= 0x62fu && codepoint <= 0x635u) {
		return alt_block31[codepoint - 0x62fu];
	} else if (codepoint >= 0x637u && codepoint <= 0x63au) {
		return alt_block32[codepoint - 0x637u];
	} else if (codepoint >= 0x640u && codepoint <= 0x64au) {
		return alt_block33[codepoint - 0x640u];
	} else if (codepoint >= 0x671u && codepoint <= 0x673u) {
		return alt_block34[codepoint - 0x671u];
	} else if (codepoint >= 0x1560u && codepoint <= 0x1563u) {
		return alt_block35[codepoint - 0x1560u];
	} else if (codepoint == 0x156au) {
		return alt_block36[0];
	} else if (codepoint >= 0x156cu && codepoint <= 0x1571u) {
		return alt_block37[codepoint - 0x156cu];
	} else if (codepoint >= 0x15ecu && codepoint <= 0x15edu) {
		return alt_block38[codepoint - 0x15ecu];
	} else if (codepoint == 0x15efu) {
		return alt_block39[0];
	} else if (codepoint >= 0x16a0u && codepoint <= 0x16abu) {
		return alt_block40[codepoint - 0x16a0u];
	} else if (codepoint >= 0x16b1u && codepoint <= 0x16f8u) {
		return alt_block41[codepoint - 0x16b1u];
	} else if (codepoint >= 0x1e00u && codepoint <= 0x1e02u) {
		return alt_block42[codepoint - 0x1e00u];
	} else if (codepoint >= 0x1e04u && codepoint <= 0x1e07u) {
		return alt_block43[codepoint - 0x1e04u];
	} else if (codepoint == 0x1e0au) {
		return alt_block44[0];
	} else if (codepoint >= 0x1e0cu && codepoint <= 0x1e0fu) {
		return alt_block45[codepoint - 0x1e0cu];
	} else if (codepoint >= 0x1e41u && codepoint <= 0x1e4bu) {
		return alt_block46[codepoint - 0x1e41u];
	} else if (codepoint >= 0x1e58u && codepoint <= 0x1e5bu) {
		return alt_block47[codepoint - 0x1e58u];
	} else if (codepoint == 0x1e5eu) {
		return alt_block48[0];
	} else if (codepoint >= 0x2010u && codepoint <= 0x2019u) {
		return alt_block49[codepoint - 0x2010u];
	} else if (codepoint >= 0x201cu && codepoint <= 0x201du) {
		return alt_block50[codepoint - 0x201cu];
	} else if (codepoint >= 0x2020u && codepoint <= 0x2022u) {
		return alt_block51[codepoint - 0x2020u];
	} else if (codepoint >= 0x2024u && codepoint <= 0x2026u) {
		return alt_block52[codepoint - 0x2024u];
	} else if (codepoint >= 0x2030u && codepoint <= 0x2031u) {
		return alt_block53[codepoint - 0x2030u];
	} else if (codepoint == 0x203cu) {
		return alt_block54[0];
	} else if (codepoint >= 0x2047u && codepoint <= 0x2049u) {
		return alt_block55[codepoint - 0x2047u];
	} else if (codepoint == 0x2070u) {
		return alt_block56[0];
	} else if (codepoint >= 0x2074u && codepoint <= 0x2079u) {
		return alt_block57[codepoint - 0x2074u];
	} else if (codepoint == 0x207fu) {
		return alt_block58[0];
	} else if (codepoint == 0x20a7u) {
		return alt_block59[0];
	} else if (codepoint == 0x20acu) {
		return alt_block60[0];
	} else if (codepoint == 0x2117u) {
		return alt_block61[0];
	} else if (codepoint == 0x2120u) {
		return alt_block62[0];
	} else if (codepoint == 0x2122u) {
		return alt_block63[0];
	} else if (codepoint >= 0x2190u && codepoint <= 0x2192u) {
		return alt_block64[codepoint - 0x2190u];
	} else if (codepoint == 0x2194u) {
		return alt_block65[0];
	} else if (codepoint >= 0x2196u && codepoint <= 0x2199u) {
		return alt_block66[codepoint - 0x2196u];
	} else if (codepoint == 0x21a5u) {
		return alt_block67[0];
	} else if (codepoint == 0x21a8u) {
		return alt_block68[0];
	} else if (codepoint >= 0x21e6u && codepoint <= 0x21e9u) {
		return alt_block69[codepoint - 0x21e6u];
	} else if (codepoint == 0x2212u) {
		return alt_block70[0];
	} else if (codepoint == 0x2218u) {
		return alt_block71[0];
	} else if (codepoint == 0x221au) {
		return alt_block72[0];
	} else if (codepoint >= 0x221eu && codepoint <= 0x221fu) {
		return alt_block73[codepoint - 0x221eu];
	} else if (codepoint == 0x2229u) {
		return alt_block74[0];
	} else if (codepoint == 0x2248u) {
		return alt_block75[0];
	} else if (codepoint == 0x2261u) {
		return alt_block76[0];
	} else if (codepoint >= 0x2264u && codepoint <= 0x2265u) {
		return alt_block77[codepoint - 0x2264u];
	} else if (codepoint == 0x22c5u) {
		return alt_block78[0];
	} else if (codepoint >= 0x2320u && codepoint <= 0x2321u) {
		return alt_block79[codepoint - 0x2320u];
	} else if (codepoint >= 0x23eau && codepoint <= 0x23ecu) {
		return alt_block80[codepoint - 0x23eau];
	} else if (codepoint >= 0x2500u && codepoint <= 0x25ffu) {
		return alt_block81[codepoint - 0x2500u];
	} else if (codepoint == 0x2625u) {
		return alt_block82[0];
	} else if (codepoint == 0x2628u) {
		return alt_block83[0];
	} else if (codepoint >= 0x262fu && codepoint <= 0x2637u) {
		return alt_block84[codepoint - 0x262fu];
	} else if (codepoint >= 0x2639u && codepoint <= 0x263bu) {
		return alt_block85[codepoint - 0x2639u];
	} else if (codepoint >= 0x2660u && codepoint <= 0x2667u) {
		return alt_block86[codepoint - 0x2660u];
	} else if (codepoint >= 0x2669u && codepoint <= 0x266cu) {
		return alt_block87[codepoint - 0x2669u];
	} else if (codepoint >= 0x268au && codepoint <= 0x268fu) {
		return alt_block88[codepoint - 0x268au];
	} else if (codepoint >= 0x26aau && codepoint <= 0x26acu) {
		return alt_block89[codepoint - 0x26aau];
	} else if (codepoint == 0x2708u) {
		return alt_block90[0];
	} else if (codepoint == 0x2734u) {
		return alt_block91[0];
	} else if (codepoint >= 0x2800u && codepoint <= 0x28ffu) {
		return alt_block92[codepoint - 0x2800u];
	} else if (codepoint == 0x2913u) {
		return alt_block93[0];
	} else if (codepoint == 0x2b1du) {
		return alt_block94[0];
	} else if (codepoint == 0x2b24u) {
		return alt_block95[0];
	} else if (codepoint == 0x2b55u) {
		return alt_block96[0];
	} else if (codepoint == 0x2b58u) {
		return alt_block97[0];
	} else if (codepoint == 0x2e2eu) {
		return alt_block98[0];
	} else if (codepoint >= 0xe080u && codepoint <= 0xe19bu) {
		return alt_block99[codepoint - 0xe080u];
	} else if (codepoint >= 0xe800u && codepoint <= 0xe850u) {
		return alt_block100[codepoint - 0xe800u];
	} else if (codepoint >= 0xec00u && codepoint <= 0xec26u) {
		return alt_block101[codepoint - 0xec00u];
	} else if (codepoint >= 0xfe81u && codepoint <= 0xfe82u) {
		return alt_block102[codepoint - 0xfe81u];
	} else if (codepoint == 0xfe84u) {
		return alt_block103[0];
	} else if (codepoint == 0xfe87u) {
		return alt_block104[0];
	} else if (codepoint >= 0xfe8fu && codepoint <= 0xfe9cu) {
		return alt_block105[codepoint - 0xfe8fu];
	} else if (codepoint >= 0xfe9eu && codepoint <= 0xfea4u) {
		return alt_block106[codepoint - 0xfe9eu];
	} else if (codepoint >= 0xfea9u && codepoint <= 0xfeadu) {
		return alt_block107[codepoint - 0xfea9u];
	} else if (codepoint == 0xfeafu) {
		return alt_block108[0];
	} else if (codepoint >= 0xfeb1u && codepoint <= 0xfeb7u) {
		return alt_block109[codepoint - 0xfeb1u];
	} else if (codepoint >= 0xfeb9u && codepoint <= 0xfebcu) {
		return alt_block110[codepoint - 0xfeb9u];
	} else if (codepoint >= 0xfec1u && codepoint <= 0xfec6u) {
		return alt_block111[codepoint - 0xfec1u];
	} else if (codepoint >= 0xfec9u && codepoint <= 0xfeddu) {
		return alt_block112[codepoint - 0xfec9u];
	} else if (codepoint >= 0xfedfu && codepoint <= 0xfee3u) {
		return alt_block113[codepoint - 0xfedfu];
	} else if (codepoint >= 0xfee5u && codepoint <= 0xfee9u) {
		return alt_block114[codepoint - 0xfee5u];
	} else if (codepoint >= 0xfeebu && codepoint <= 0xfef4u) {
		return alt_block115[codepoint - 0xfeebu];
	} else if (codepoint >= 0xff61u && codepoint <= 0xff9fu) {
		return alt_block116[codepoint - 0xff61u];
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
	} else if (codepoint >= 0x1cdu && codepoint <= 0x1e3u) {
		return thin_block3[codepoint - 0x1cdu];
	} else if (codepoint >= 0x1e6u && codepoint <= 0x1e8u) {
		return thin_block4[codepoint - 0x1e6u];
	} else if (codepoint == 0x1f0u) {
		return thin_block5[0];
	} else if (codepoint >= 0x1f4u && codepoint <= 0x1f5u) {
		return thin_block6[codepoint - 0x1f4u];
	} else if (codepoint >= 0x1f8u && codepoint <= 0x21bu) {
		return thin_block7[codepoint - 0x1f8u];
	} else if (codepoint >= 0x226u && codepoint <= 0x227u) {
		return thin_block8[codepoint - 0x226u];
	} else if (codepoint >= 0x22au && codepoint <= 0x233u) {
		return thin_block9[codepoint - 0x22au];
	} else if (codepoint >= 0x258u && codepoint <= 0x259u) {
		return thin_block10[codepoint - 0x258u];
	} else if (codepoint == 0x263u) {
		return thin_block11[0];
	} else if (codepoint == 0x294u) {
		return thin_block12[0];
	} else if (codepoint == 0x2c6u) {
		return thin_block13[0];
	} else if (codepoint == 0x2c9u) {
		return thin_block14[0];
	} else if (codepoint == 0x2cdu) {
		return thin_block15[0];
	} else if (codepoint == 0x2dcu) {
		return thin_block16[0];
	} else if (codepoint == 0x34fu) {
		return thin_block17[0];
	} else if (codepoint >= 0x391u && codepoint <= 0x3a1u) {
		return thin_block18[codepoint - 0x391u];
	} else if (codepoint >= 0x3a3u && codepoint <= 0x3a9u) {
		return thin_block19[codepoint - 0x3a3u];
	} else if (codepoint >= 0x3b1u && codepoint <= 0x3c9u) {
		return thin_block20[codepoint - 0x3b1u];
	} else if (codepoint >= 0x400u && codepoint <= 0x401u) {
		return thin_block21[codepoint - 0x400u];
	} else if (codepoint >= 0x405u && codepoint <= 0x408u) {
		return thin_block22[codepoint - 0x405u];
	} else if (codepoint >= 0x410u && codepoint <= 0x451u) {
		return thin_block23[codepoint - 0x410u];
	} else if (codepoint >= 0x455u && codepoint <= 0x458u) {
		return thin_block24[codepoint - 0x455u];
	} else if (codepoint >= 0x4d0u && codepoint <= 0x4d7u) {
		return thin_block25[codepoint - 0x4d0u];
	} else if (codepoint >= 0x4e6u && codepoint <= 0x4e7u) {
		return thin_block26[codepoint - 0x4e6u];
	} else if (codepoint == 0x4f1u) {
		return thin_block27[0];
	} else if (codepoint >= 0x5d0u && codepoint <= 0x5eau) {
		return thin_block28[codepoint - 0x5d0u];
	} else if (codepoint == 0x623u) {
		return thin_block29[0];
	} else if (codepoint >= 0x627u && codepoint <= 0x62du) {
		return thin_block30[codepoint - 0x627u];
	} else if (codepoint >= 0x62fu && codepoint <= 0x635u) {
		return thin_block31[codepoint - 0x62fu];
	} else if (codepoint >= 0x637u && codepoint <= 0x63au) {
		return thin_block32[codepoint - 0x637u];
	} else if (codepoint >= 0x640u && codepoint <= 0x64au) {
		return thin_block33[codepoint - 0x640u];
	} else if (codepoint >= 0x671u && codepoint <= 0x673u) {
		return thin_block34[codepoint - 0x671u];
	} else if (codepoint >= 0x1560u && codepoint <= 0x1563u) {
		return thin_block35[codepoint - 0x1560u];
	} else if (codepoint == 0x156au) {
		return thin_block36[0];
	} else if (codepoint >= 0x156cu && codepoint <= 0x1571u) {
		return thin_block37[codepoint - 0x156cu];
	} else if (codepoint >= 0x15ecu && codepoint <= 0x15edu) {
		return thin_block38[codepoint - 0x15ecu];
	} else if (codepoint == 0x15efu) {
		return thin_block39[0];
	} else if (codepoint >= 0x16a0u && codepoint <= 0x16abu) {
		return thin_block40[codepoint - 0x16a0u];
	} else if (codepoint >= 0x16b1u && codepoint <= 0x16f8u) {
		return thin_block41[codepoint - 0x16b1u];
	} else if (codepoint >= 0x1e00u && codepoint <= 0x1e02u) {
		return thin_block42[codepoint - 0x1e00u];
	} else if (codepoint >= 0x1e04u && codepoint <= 0x1e07u) {
		return thin_block43[codepoint - 0x1e04u];
	} else if (codepoint == 0x1e0au) {
		return thin_block44[0];
	} else if (codepoint >= 0x1e0cu && codepoint <= 0x1e0fu) {
		return thin_block45[codepoint - 0x1e0cu];
	} else if (codepoint >= 0x1e41u && codepoint <= 0x1e4bu) {
		return thin_block46[codepoint - 0x1e41u];
	} else if (codepoint >= 0x1e58u && codepoint <= 0x1e5bu) {
		return thin_block47[codepoint - 0x1e58u];
	} else if (codepoint == 0x1e5eu) {
		return thin_block48[0];
	} else if (codepoint >= 0x2010u && codepoint <= 0x2019u) {
		return thin_block49[codepoint - 0x2010u];
	} else if (codepoint >= 0x201cu && codepoint <= 0x201du) {
		return thin_block50[codepoint - 0x201cu];
	} else if (codepoint >= 0x2020u && codepoint <= 0x2022u) {
		return thin_block51[codepoint - 0x2020u];
	} else if (codepoint >= 0x2024u && codepoint <= 0x2026u) {
		return thin_block52[codepoint - 0x2024u];
	} else if (codepoint >= 0x2030u && codepoint <= 0x2031u) {
		return thin_block53[codepoint - 0x2030u];
	} else if (codepoint == 0x203cu) {
		return thin_block54[0];
	} else if (codepoint >= 0x2047u && codepoint <= 0x2049u) {
		return thin_block55[codepoint - 0x2047u];
	} else if (codepoint == 0x2070u) {
		return thin_block56[0];
	} else if (codepoint >= 0x2074u && codepoint <= 0x2079u) {
		return thin_block57[codepoint - 0x2074u];
	} else if (codepoint == 0x207fu) {
		return thin_block58[0];
	} else if (codepoint == 0x20a7u) {
		return thin_block59[0];
	} else if (codepoint == 0x20acu) {
		return thin_block60[0];
	} else if (codepoint == 0x2117u) {
		return thin_block61[0];
	} else if (codepoint == 0x2120u) {
		return thin_block62[0];
	} else if (codepoint == 0x2122u) {
		return thin_block63[0];
	} else if (codepoint >= 0x2190u && codepoint <= 0x2192u) {
		return thin_block64[codepoint - 0x2190u];
	} else if (codepoint == 0x2194u) {
		return thin_block65[0];
	} else if (codepoint >= 0x2196u && codepoint <= 0x2199u) {
		return thin_block66[codepoint - 0x2196u];
	} else if (codepoint == 0x21a5u) {
		return thin_block67[0];
	} else if (codepoint == 0x21a8u) {
		return thin_block68[0];
	} else if (codepoint >= 0x21e6u && codepoint <= 0x21e9u) {
		return thin_block69[codepoint - 0x21e6u];
	} else if (codepoint == 0x2212u) {
		return thin_block70[0];
	} else if (codepoint == 0x2218u) {
		return thin_block71[0];
	} else if (codepoint == 0x221au) {
		return thin_block72[0];
	} else if (codepoint >= 0x221eu && codepoint <= 0x221fu) {
		return thin_block73[codepoint - 0x221eu];
	} else if (codepoint == 0x2229u) {
		return thin_block74[0];
	} else if (codepoint == 0x2248u) {
		return thin_block75[0];
	} else if (codepoint == 0x2261u) {
		return thin_block76[0];
	} else if (codepoint >= 0x2264u && codepoint <= 0x2265u) {
		return thin_block77[codepoint - 0x2264u];
	} else if (codepoint == 0x22c5u) {
		return thin_block78[0];
	} else if (codepoint >= 0x2320u && codepoint <= 0x2321u) {
		return thin_block79[codepoint - 0x2320u];
	} else if (codepoint >= 0x23eau && codepoint <= 0x23ecu) {
		return thin_block80[codepoint - 0x23eau];
	} else if (codepoint >= 0x2500u && codepoint <= 0x25ffu) {
		return thin_block81[codepoint - 0x2500u];
	} else if (codepoint == 0x2625u) {
		return thin_block82[0];
	} else if (codepoint == 0x2628u) {
		return thin_block83[0];
	} else if (codepoint >= 0x262fu && codepoint <= 0x2637u) {
		return thin_block84[codepoint - 0x262fu];
	} else if (codepoint >= 0x2639u && codepoint <= 0x263bu) {
		return thin_block85[codepoint - 0x2639u];
	} else if (codepoint >= 0x2660u && codepoint <= 0x2667u) {
		return thin_block86[codepoint - 0x2660u];
	} else if (codepoint >= 0x2669u && codepoint <= 0x266cu) {
		return thin_block87[codepoint - 0x2669u];
	} else if (codepoint >= 0x268au && codepoint <= 0x268fu) {
		return thin_block88[codepoint - 0x268au];
	} else if (codepoint >= 0x26aau && codepoint <= 0x26acu) {
		return thin_block89[codepoint - 0x26aau];
	} else if (codepoint == 0x2708u) {
		return thin_block90[0];
	} else if (codepoint == 0x2734u) {
		return thin_block91[0];
	} else if (codepoint >= 0x2800u && codepoint <= 0x28ffu) {
		return thin_block92[codepoint - 0x2800u];
	} else if (codepoint == 0x2913u) {
		return thin_block93[0];
	} else if (codepoint == 0x2b1du) {
		return thin_block94[0];
	} else if (codepoint == 0x2b24u) {
		return thin_block95[0];
	} else if (codepoint == 0x2b55u) {
		return thin_block96[0];
	} else if (codepoint == 0x2b58u) {
		return thin_block97[0];
	} else if (codepoint == 0x2e2eu) {
		return thin_block98[0];
	} else if (codepoint >= 0xe080u && codepoint <= 0xe19bu) {
		return thin_block99[codepoint - 0xe080u];
	} else if (codepoint >= 0xe800u && codepoint <= 0xe850u) {
		return thin_block100[codepoint - 0xe800u];
	} else if (codepoint >= 0xec00u && codepoint <= 0xec26u) {
		return thin_block101[codepoint - 0xec00u];
	} else if (codepoint >= 0xfe81u && codepoint <= 0xfe82u) {
		return thin_block102[codepoint - 0xfe81u];
	} else if (codepoint == 0xfe84u) {
		return thin_block103[0];
	} else if (codepoint == 0xfe87u) {
		return thin_block104[0];
	} else if (codepoint >= 0xfe8fu && codepoint <= 0xfe9cu) {
		return thin_block105[codepoint - 0xfe8fu];
	} else if (codepoint >= 0xfe9eu && codepoint <= 0xfea4u) {
		return thin_block106[codepoint - 0xfe9eu];
	} else if (codepoint >= 0xfea9u && codepoint <= 0xfeadu) {
		return thin_block107[codepoint - 0xfea9u];
	} else if (codepoint == 0xfeafu) {
		return thin_block108[0];
	} else if (codepoint >= 0xfeb1u && codepoint <= 0xfeb7u) {
		return thin_block109[codepoint - 0xfeb1u];
	} else if (codepoint >= 0xfeb9u && codepoint <= 0xfebcu) {
		return thin_block110[codepoint - 0xfeb9u];
	} else if (codepoint >= 0xfec1u && codepoint <= 0xfec6u) {
		return thin_block111[codepoint - 0xfec1u];
	} else if (codepoint >= 0xfec9u && codepoint <= 0xfeddu) {
		return thin_block112[codepoint - 0xfec9u];
	} else if (codepoint >= 0xfedfu && codepoint <= 0xfee3u) {
		return thin_block113[codepoint - 0xfedfu];
	} else if (codepoint >= 0xfee5u && codepoint <= 0xfee9u) {
		return thin_block114[codepoint - 0xfee5u];
	} else if (codepoint >= 0xfeebu && codepoint <= 0xfef4u) {
		return thin_block115[codepoint - 0xfeebu];
	} else if (codepoint >= 0xff61u && codepoint <= 0xff9fu) {
		return thin_block116[codepoint - 0xff61u];
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
	} else if (codepoint >= 0x1cdu && codepoint <= 0x1e3u) {
		return fantasy_block3[codepoint - 0x1cdu];
	} else if (codepoint >= 0x1e6u && codepoint <= 0x1e8u) {
		return fantasy_block4[codepoint - 0x1e6u];
	} else if (codepoint == 0x1f0u) {
		return fantasy_block5[0];
	} else if (codepoint >= 0x1f4u && codepoint <= 0x1f5u) {
		return fantasy_block6[codepoint - 0x1f4u];
	} else if (codepoint >= 0x1f8u && codepoint <= 0x21bu) {
		return fantasy_block7[codepoint - 0x1f8u];
	} else if (codepoint >= 0x226u && codepoint <= 0x227u) {
		return fantasy_block8[codepoint - 0x226u];
	} else if (codepoint >= 0x22au && codepoint <= 0x233u) {
		return fantasy_block9[codepoint - 0x22au];
	} else if (codepoint >= 0x258u && codepoint <= 0x259u) {
		return fantasy_block10[codepoint - 0x258u];
	} else if (codepoint == 0x263u) {
		return fantasy_block11[0];
	} else if (codepoint == 0x294u) {
		return fantasy_block12[0];
	} else if (codepoint == 0x2c6u) {
		return fantasy_block13[0];
	} else if (codepoint == 0x2c9u) {
		return fantasy_block14[0];
	} else if (codepoint == 0x2cdu) {
		return fantasy_block15[0];
	} else if (codepoint == 0x2dcu) {
		return fantasy_block16[0];
	} else if (codepoint == 0x34fu) {
		return fantasy_block17[0];
	} else if (codepoint >= 0x391u && codepoint <= 0x3a1u) {
		return fantasy_block18[codepoint - 0x391u];
	} else if (codepoint >= 0x3a3u && codepoint <= 0x3a9u) {
		return fantasy_block19[codepoint - 0x3a3u];
	} else if (codepoint >= 0x3b1u && codepoint <= 0x3c9u) {
		return fantasy_block20[codepoint - 0x3b1u];
	} else if (codepoint >= 0x400u && codepoint <= 0x401u) {
		return fantasy_block21[codepoint - 0x400u];
	} else if (codepoint >= 0x405u && codepoint <= 0x408u) {
		return fantasy_block22[codepoint - 0x405u];
	} else if (codepoint >= 0x410u && codepoint <= 0x451u) {
		return fantasy_block23[codepoint - 0x410u];
	} else if (codepoint >= 0x455u && codepoint <= 0x458u) {
		return fantasy_block24[codepoint - 0x455u];
	} else if (codepoint >= 0x4d0u && codepoint <= 0x4d7u) {
		return fantasy_block25[codepoint - 0x4d0u];
	} else if (codepoint >= 0x4e6u && codepoint <= 0x4e7u) {
		return fantasy_block26[codepoint - 0x4e6u];
	} else if (codepoint == 0x4f1u) {
		return fantasy_block27[0];
	} else if (codepoint >= 0x5d0u && codepoint <= 0x5eau) {
		return fantasy_block28[codepoint - 0x5d0u];
	} else if (codepoint == 0x623u) {
		return fantasy_block29[0];
	} else if (codepoint >= 0x627u && codepoint <= 0x62du) {
		return fantasy_block30[codepoint - 0x627u];
	} else if (codepoint >= 0x62fu && codepoint <= 0x635u) {
		return fantasy_block31[codepoint - 0x62fu];
	} else if (codepoint >= 0x637u && codepoint <= 0x63au) {
		return fantasy_block32[codepoint - 0x637u];
	} else if (codepoint >= 0x640u && codepoint <= 0x64au) {
		return fantasy_block33[codepoint - 0x640u];
	} else if (codepoint >= 0x671u && codepoint <= 0x673u) {
		return fantasy_block34[codepoint - 0x671u];
	} else if (codepoint >= 0x1560u && codepoint <= 0x1563u) {
		return fantasy_block35[codepoint - 0x1560u];
	} else if (codepoint == 0x156au) {
		return fantasy_block36[0];
	} else if (codepoint >= 0x156cu && codepoint <= 0x1571u) {
		return fantasy_block37[codepoint - 0x156cu];
	} else if (codepoint >= 0x15ecu && codepoint <= 0x15edu) {
		return fantasy_block38[codepoint - 0x15ecu];
	} else if (codepoint == 0x15efu) {
		return fantasy_block39[0];
	} else if (codepoint >= 0x16a0u && codepoint <= 0x16abu) {
		return fantasy_block40[codepoint - 0x16a0u];
	} else if (codepoint >= 0x16b1u && codepoint <= 0x16f8u) {
		return fantasy_block41[codepoint - 0x16b1u];
	} else if (codepoint >= 0x1e00u && codepoint <= 0x1e02u) {
		return fantasy_block42[codepoint - 0x1e00u];
	} else if (codepoint >= 0x1e04u && codepoint <= 0x1e07u) {
		return fantasy_block43[codepoint - 0x1e04u];
	} else if (codepoint == 0x1e0au) {
		return fantasy_block44[0];
	} else if (codepoint >= 0x1e0cu && codepoint <= 0x1e0fu) {
		return fantasy_block45[codepoint - 0x1e0cu];
	} else if (codepoint >= 0x1e41u && codepoint <= 0x1e4bu) {
		return fantasy_block46[codepoint - 0x1e41u];
	} else if (codepoint >= 0x1e58u && codepoint <= 0x1e5bu) {
		return fantasy_block47[codepoint - 0x1e58u];
	} else if (codepoint == 0x1e5eu) {
		return fantasy_block48[0];
	} else if (codepoint >= 0x2010u && codepoint <= 0x2019u) {
		return fantasy_block49[codepoint - 0x2010u];
	} else if (codepoint >= 0x201cu && codepoint <= 0x201du) {
		return fantasy_block50[codepoint - 0x201cu];
	} else if (codepoint >= 0x2020u && codepoint <= 0x2022u) {
		return fantasy_block51[codepoint - 0x2020u];
	} else if (codepoint >= 0x2024u && codepoint <= 0x2026u) {
		return fantasy_block52[codepoint - 0x2024u];
	} else if (codepoint >= 0x2030u && codepoint <= 0x2031u) {
		return fantasy_block53[codepoint - 0x2030u];
	} else if (codepoint == 0x203cu) {
		return fantasy_block54[0];
	} else if (codepoint >= 0x2047u && codepoint <= 0x2049u) {
		return fantasy_block55[codepoint - 0x2047u];
	} else if (codepoint == 0x2070u) {
		return fantasy_block56[0];
	} else if (codepoint >= 0x2074u && codepoint <= 0x2079u) {
		return fantasy_block57[codepoint - 0x2074u];
	} else if (codepoint == 0x207fu) {
		return fantasy_block58[0];
	} else if (codepoint == 0x20a7u) {
		return fantasy_block59[0];
	} else if (codepoint == 0x20acu) {
		return fantasy_block60[0];
	} else if (codepoint == 0x2117u) {
		return fantasy_block61[0];
	} else if (codepoint == 0x2120u) {
		return fantasy_block62[0];
	} else if (codepoint == 0x2122u) {
		return fantasy_block63[0];
	} else if (codepoint >= 0x2190u && codepoint <= 0x2192u) {
		return fantasy_block64[codepoint - 0x2190u];
	} else if (codepoint == 0x2194u) {
		return fantasy_block65[0];
	} else if (codepoint >= 0x2196u && codepoint <= 0x2199u) {
		return fantasy_block66[codepoint - 0x2196u];
	} else if (codepoint == 0x21a5u) {
		return fantasy_block67[0];
	} else if (codepoint == 0x21a8u) {
		return fantasy_block68[0];
	} else if (codepoint >= 0x21e6u && codepoint <= 0x21e9u) {
		return fantasy_block69[codepoint - 0x21e6u];
	} else if (codepoint == 0x2212u) {
		return fantasy_block70[0];
	} else if (codepoint == 0x2218u) {
		return fantasy_block71[0];
	} else if (codepoint == 0x221au) {
		return fantasy_block72[0];
	} else if (codepoint >= 0x221eu && codepoint <= 0x221fu) {
		return fantasy_block73[codepoint - 0x221eu];
	} else if (codepoint == 0x2229u) {
		return fantasy_block74[0];
	} else if (codepoint == 0x2248u) {
		return fantasy_block75[0];
	} else if (codepoint == 0x2261u) {
		return fantasy_block76[0];
	} else if (codepoint >= 0x2264u && codepoint <= 0x2265u) {
		return fantasy_block77[codepoint - 0x2264u];
	} else if (codepoint == 0x22c5u) {
		return fantasy_block78[0];
	} else if (codepoint >= 0x2320u && codepoint <= 0x2321u) {
		return fantasy_block79[codepoint - 0x2320u];
	} else if (codepoint >= 0x23eau && codepoint <= 0x23ecu) {
		return fantasy_block80[codepoint - 0x23eau];
	} else if (codepoint >= 0x2500u && codepoint <= 0x25ffu) {
		return fantasy_block81[codepoint - 0x2500u];
	} else if (codepoint == 0x2625u) {
		return fantasy_block82[0];
	} else if (codepoint == 0x2628u) {
		return fantasy_block83[0];
	} else if (codepoint >= 0x262fu && codepoint <= 0x2637u) {
		return fantasy_block84[codepoint - 0x262fu];
	} else if (codepoint >= 0x2639u && codepoint <= 0x263bu) {
		return fantasy_block85[codepoint - 0x2639u];
	} else if (codepoint >= 0x2660u && codepoint <= 0x2667u) {
		return fantasy_block86[codepoint - 0x2660u];
	} else if (codepoint >= 0x2669u && codepoint <= 0x266cu) {
		return fantasy_block87[codepoint - 0x2669u];
	} else if (codepoint >= 0x268au && codepoint <= 0x268fu) {
		return fantasy_block88[codepoint - 0x268au];
	} else if (codepoint >= 0x26aau && codepoint <= 0x26acu) {
		return fantasy_block89[codepoint - 0x26aau];
	} else if (codepoint == 0x2708u) {
		return fantasy_block90[0];
	} else if (codepoint == 0x2734u) {
		return fantasy_block91[0];
	} else if (codepoint >= 0x2800u && codepoint <= 0x28ffu) {
		return fantasy_block92[codepoint - 0x2800u];
	} else if (codepoint == 0x2913u) {
		return fantasy_block93[0];
	} else if (codepoint == 0x2b1du) {
		return fantasy_block94[0];
	} else if (codepoint == 0x2b24u) {
		return fantasy_block95[0];
	} else if (codepoint == 0x2b55u) {
		return fantasy_block96[0];
	} else if (codepoint == 0x2b58u) {
		return fantasy_block97[0];
	} else if (codepoint == 0x2e2eu) {
		return fantasy_block98[0];
	} else if (codepoint >= 0xe080u && codepoint <= 0xe19bu) {
		return fantasy_block99[codepoint - 0xe080u];
	} else if (codepoint >= 0xe800u && codepoint <= 0xe850u) {
		return fantasy_block100[codepoint - 0xe800u];
	} else if (codepoint >= 0xec00u && codepoint <= 0xec26u) {
		return fantasy_block101[codepoint - 0xec00u];
	} else if (codepoint >= 0xfe81u && codepoint <= 0xfe82u) {
		return fantasy_block102[codepoint - 0xfe81u];
	} else if (codepoint == 0xfe84u) {
		return fantasy_block103[0];
	} else if (codepoint == 0xfe87u) {
		return fantasy_block104[0];
	} else if (codepoint >= 0xfe8fu && codepoint <= 0xfe9cu) {
		return fantasy_block105[codepoint - 0xfe8fu];
	} else if (codepoint >= 0xfe9eu && codepoint <= 0xfea4u) {
		return fantasy_block106[codepoint - 0xfe9eu];
	} else if (codepoint >= 0xfea9u && codepoint <= 0xfeadu) {
		return fantasy_block107[codepoint - 0xfea9u];
	} else if (codepoint == 0xfeafu) {
		return fantasy_block108[0];
	} else if (codepoint >= 0xfeb1u && codepoint <= 0xfeb7u) {
		return fantasy_block109[codepoint - 0xfeb1u];
	} else if (codepoint >= 0xfeb9u && codepoint <= 0xfebcu) {
		return fantasy_block110[codepoint - 0xfeb9u];
	} else if (codepoint >= 0xfec1u && codepoint <= 0xfec6u) {
		return fantasy_block111[codepoint - 0xfec1u];
	} else if (codepoint >= 0xfec9u && codepoint <= 0xfeddu) {
		return fantasy_block112[codepoint - 0xfec9u];
	} else if (codepoint >= 0xfedfu && codepoint <= 0xfee3u) {
		return fantasy_block113[codepoint - 0xfedfu];
	} else if (codepoint >= 0xfee5u && codepoint <= 0xfee9u) {
		return fantasy_block114[codepoint - 0xfee5u];
	} else if (codepoint >= 0xfeebu && codepoint <= 0xfef4u) {
		return fantasy_block115[codepoint - 0xfeebu];
	} else if (codepoint >= 0xff61u && codepoint <= 0xff9fu) {
		return fantasy_block116[codepoint - 0xff61u];
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
	} else if (codepoint >= 0x1cdu && codepoint <= 0x1e3u) {
		return mcr_block3[codepoint - 0x1cdu];
	} else if (codepoint >= 0x1e6u && codepoint <= 0x1e8u) {
		return mcr_block4[codepoint - 0x1e6u];
	} else if (codepoint == 0x1f0u) {
		return mcr_block5[0];
	} else if (codepoint >= 0x1f4u && codepoint <= 0x1f5u) {
		return mcr_block6[codepoint - 0x1f4u];
	} else if (codepoint >= 0x1f8u && codepoint <= 0x21bu) {
		return mcr_block7[codepoint - 0x1f8u];
	} else if (codepoint >= 0x226u && codepoint <= 0x227u) {
		return mcr_block8[codepoint - 0x226u];
	} else if (codepoint >= 0x22au && codepoint <= 0x233u) {
		return mcr_block9[codepoint - 0x22au];
	} else if (codepoint >= 0x258u && codepoint <= 0x259u) {
		return mcr_block10[codepoint - 0x258u];
	} else if (codepoint == 0x263u) {
		return mcr_block11[0];
	} else if (codepoint == 0x294u) {
		return mcr_block12[0];
	} else if (codepoint == 0x2c6u) {
		return mcr_block13[0];
	} else if (codepoint == 0x2c9u) {
		return mcr_block14[0];
	} else if (codepoint == 0x2cdu) {
		return mcr_block15[0];
	} else if (codepoint == 0x2dcu) {
		return mcr_block16[0];
	} else if (codepoint == 0x34fu) {
		return mcr_block17[0];
	} else if (codepoint >= 0x391u && codepoint <= 0x3a1u) {
		return mcr_block18[codepoint - 0x391u];
	} else if (codepoint >= 0x3a3u && codepoint <= 0x3a9u) {
		return mcr_block19[codepoint - 0x3a3u];
	} else if (codepoint >= 0x3b1u && codepoint <= 0x3c9u) {
		return mcr_block20[codepoint - 0x3b1u];
	} else if (codepoint >= 0x400u && codepoint <= 0x401u) {
		return mcr_block21[codepoint - 0x400u];
	} else if (codepoint >= 0x405u && codepoint <= 0x408u) {
		return mcr_block22[codepoint - 0x405u];
	} else if (codepoint >= 0x410u && codepoint <= 0x451u) {
		return mcr_block23[codepoint - 0x410u];
	} else if (codepoint >= 0x455u && codepoint <= 0x458u) {
		return mcr_block24[codepoint - 0x455u];
	} else if (codepoint >= 0x4d0u && codepoint <= 0x4d7u) {
		return mcr_block25[codepoint - 0x4d0u];
	} else if (codepoint >= 0x4e6u && codepoint <= 0x4e7u) {
		return mcr_block26[codepoint - 0x4e6u];
	} else if (codepoint == 0x4f1u) {
		return mcr_block27[0];
	} else if (codepoint >= 0x5d0u && codepoint <= 0x5eau) {
		return mcr_block28[codepoint - 0x5d0u];
	} else if (codepoint == 0x623u) {
		return mcr_block29[0];
	} else if (codepoint >= 0x627u && codepoint <= 0x62du) {
		return mcr_block30[codepoint - 0x627u];
	} else if (codepoint >= 0x62fu && codepoint <= 0x635u) {
		return mcr_block31[codepoint - 0x62fu];
	} else if (codepoint >= 0x637u && codepoint <= 0x63au) {
		return mcr_block32[codepoint - 0x637u];
	} else if (codepoint >= 0x640u && codepoint <= 0x64au) {
		return mcr_block33[codepoint - 0x640u];
	} else if (codepoint >= 0x671u && codepoint <= 0x673u) {
		return mcr_block34[codepoint - 0x671u];
	} else if (codepoint >= 0x1560u && codepoint <= 0x1563u) {
		return mcr_block35[codepoint - 0x1560u];
	} else if (codepoint == 0x156au) {
		return mcr_block36[0];
	} else if (codepoint >= 0x156cu && codepoint <= 0x1571u) {
		return mcr_block37[codepoint - 0x156cu];
	} else if (codepoint >= 0x15ecu && codepoint <= 0x15edu) {
		return mcr_block38[codepoint - 0x15ecu];
	} else if (codepoint == 0x15efu) {
		return mcr_block39[0];
	} else if (codepoint >= 0x16a0u && codepoint <= 0x16abu) {
		return mcr_block40[codepoint - 0x16a0u];
	} else if (codepoint >= 0x16b1u && codepoint <= 0x16f8u) {
		return mcr_block41[codepoint - 0x16b1u];
	} else if (codepoint >= 0x1e00u && codepoint <= 0x1e02u) {
		return mcr_block42[codepoint - 0x1e00u];
	} else if (codepoint >= 0x1e04u && codepoint <= 0x1e07u) {
		return mcr_block43[codepoint - 0x1e04u];
	} else if (codepoint == 0x1e0au) {
		return mcr_block44[0];
	} else if (codepoint >= 0x1e0cu && codepoint <= 0x1e0fu) {
		return mcr_block45[codepoint - 0x1e0cu];
	} else if (codepoint >= 0x1e41u && codepoint <= 0x1e4bu) {
		return mcr_block46[codepoint - 0x1e41u];
	} else if (codepoint >= 0x1e58u && codepoint <= 0x1e5bu) {
		return mcr_block47[codepoint - 0x1e58u];
	} else if (codepoint == 0x1e5eu) {
		return mcr_block48[0];
	} else if (codepoint >= 0x2010u && codepoint <= 0x2019u) {
		return mcr_block49[codepoint - 0x2010u];
	} else if (codepoint >= 0x201cu && codepoint <= 0x201du) {
		return mcr_block50[codepoint - 0x201cu];
	} else if (codepoint >= 0x2020u && codepoint <= 0x2022u) {
		return mcr_block51[codepoint - 0x2020u];
	} else if (codepoint >= 0x2024u && codepoint <= 0x2026u) {
		return mcr_block52[codepoint - 0x2024u];
	} else if (codepoint >= 0x2030u && codepoint <= 0x2031u) {
		return mcr_block53[codepoint - 0x2030u];
	} else if (codepoint == 0x203cu) {
		return mcr_block54[0];
	} else if (codepoint >= 0x2047u && codepoint <= 0x2049u) {
		return mcr_block55[codepoint - 0x2047u];
	} else if (codepoint == 0x2070u) {
		return mcr_block56[0];
	} else if (codepoint >= 0x2074u && codepoint <= 0x2079u) {
		return mcr_block57[codepoint - 0x2074u];
	} else if (codepoint == 0x207fu) {
		return mcr_block58[0];
	} else if (codepoint == 0x20a7u) {
		return mcr_block59[0];
	} else if (codepoint == 0x20acu) {
		return mcr_block60[0];
	} else if (codepoint == 0x2117u) {
		return mcr_block61[0];
	} else if (codepoint == 0x2120u) {
		return mcr_block62[0];
	} else if (codepoint == 0x2122u) {
		return mcr_block63[0];
	} else if (codepoint >= 0x2190u && codepoint <= 0x2192u) {
		return mcr_block64[codepoint - 0x2190u];
	} else if (codepoint == 0x2194u) {
		return mcr_block65[0];
	} else if (codepoint >= 0x2196u && codepoint <= 0x2199u) {
		return mcr_block66[codepoint - 0x2196u];
	} else if (codepoint == 0x21a5u) {
		return mcr_block67[0];
	} else if (codepoint == 0x21a8u) {
		return mcr_block68[0];
	} else if (codepoint >= 0x21e6u && codepoint <= 0x21e9u) {
		return mcr_block69[codepoint - 0x21e6u];
	} else if (codepoint == 0x2212u) {
		return mcr_block70[0];
	} else if (codepoint == 0x2218u) {
		return mcr_block71[0];
	} else if (codepoint == 0x221au) {
		return mcr_block72[0];
	} else if (codepoint >= 0x221eu && codepoint <= 0x221fu) {
		return mcr_block73[codepoint - 0x221eu];
	} else if (codepoint == 0x2229u) {
		return mcr_block74[0];
	} else if (codepoint == 0x2248u) {
		return mcr_block75[0];
	} else if (codepoint == 0x2261u) {
		return mcr_block76[0];
	} else if (codepoint >= 0x2264u && codepoint <= 0x2265u) {
		return mcr_block77[codepoint - 0x2264u];
	} else if (codepoint == 0x22c5u) {
		return mcr_block78[0];
	} else if (codepoint >= 0x2320u && codepoint <= 0x2321u) {
		return mcr_block79[codepoint - 0x2320u];
	} else if (codepoint >= 0x23eau && codepoint <= 0x23ecu) {
		return mcr_block80[codepoint - 0x23eau];
	} else if (codepoint >= 0x2500u && codepoint <= 0x25ffu) {
		return mcr_block81[codepoint - 0x2500u];
	} else if (codepoint == 0x2625u) {
		return mcr_block82[0];
	} else if (codepoint == 0x2628u) {
		return mcr_block83[0];
	} else if (codepoint >= 0x262fu && codepoint <= 0x2637u) {
		return mcr_block84[codepoint - 0x262fu];
	} else if (codepoint >= 0x2639u && codepoint <= 0x263bu) {
		return mcr_block85[codepoint - 0x2639u];
	} else if (codepoint >= 0x2660u && codepoint <= 0x2667u) {
		return mcr_block86[codepoint - 0x2660u];
	} else if (codepoint >= 0x2669u && codepoint <= 0x266cu) {
		return mcr_block87[codepoint - 0x2669u];
	} else if (codepoint >= 0x268au && codepoint <= 0x268fu) {
		return mcr_block88[codepoint - 0x268au];
	} else if (codepoint >= 0x26aau && codepoint <= 0x26acu) {
		return mcr_block89[codepoint - 0x26aau];
	} else if (codepoint == 0x2708u) {
		return mcr_block90[0];
	} else if (codepoint == 0x2734u) {
		return mcr_block91[0];
	} else if (codepoint >= 0x2800u && codepoint <= 0x28ffu) {
		return mcr_block92[codepoint - 0x2800u];
	} else if (codepoint == 0x2913u) {
		return mcr_block93[0];
	} else if (codepoint == 0x2b1du) {
		return mcr_block94[0];
	} else if (codepoint == 0x2b24u) {
		return mcr_block95[0];
	} else if (codepoint == 0x2b55u) {
		return mcr_block96[0];
	} else if (codepoint == 0x2b58u) {
		return mcr_block97[0];
	} else if (codepoint == 0x2e2eu) {
		return mcr_block98[0];
	} else if (codepoint >= 0xe080u && codepoint <= 0xe19bu) {
		return mcr_block99[codepoint - 0xe080u];
	} else if (codepoint >= 0xe800u && codepoint <= 0xe850u) {
		return mcr_block100[codepoint - 0xe800u];
	} else if (codepoint >= 0xec00u && codepoint <= 0xec26u) {
		return mcr_block101[codepoint - 0xec00u];
	} else if (codepoint >= 0xfe81u && codepoint <= 0xfe82u) {
		return mcr_block102[codepoint - 0xfe81u];
	} else if (codepoint == 0xfe84u) {
		return mcr_block103[0];
	} else if (codepoint == 0xfe87u) {
		return mcr_block104[0];
	} else if (codepoint >= 0xfe8fu && codepoint <= 0xfe9cu) {
		return mcr_block105[codepoint - 0xfe8fu];
	} else if (codepoint >= 0xfe9eu && codepoint <= 0xfea4u) {
		return mcr_block106[codepoint - 0xfe9eu];
	} else if (codepoint >= 0xfea9u && codepoint <= 0xfeadu) {
		return mcr_block107[codepoint - 0xfea9u];
	} else if (codepoint == 0xfeafu) {
		return mcr_block108[0];
	} else if (codepoint >= 0xfeb1u && codepoint <= 0xfeb7u) {
		return mcr_block109[codepoint - 0xfeb1u];
	} else if (codepoint >= 0xfeb9u && codepoint <= 0xfebcu) {
		return mcr_block110[codepoint - 0xfeb9u];
	} else if (codepoint >= 0xfec1u && codepoint <= 0xfec6u) {
		return mcr_block111[codepoint - 0xfec1u];
	} else if (codepoint >= 0xfec9u && codepoint <= 0xfeddu) {
		return mcr_block112[codepoint - 0xfec9u];
	} else if (codepoint >= 0xfedfu && codepoint <= 0xfee3u) {
		return mcr_block113[codepoint - 0xfedfu];
	} else if (codepoint >= 0xfee5u && codepoint <= 0xfee9u) {
		return mcr_block114[codepoint - 0xfee5u];
	} else if (codepoint >= 0xfeebu && codepoint <= 0xfef4u) {
		return mcr_block115[codepoint - 0xfeebu];
	} else if (codepoint >= 0xff61u && codepoint <= 0xff9fu) {
		return mcr_block116[codepoint - 0xff61u];
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
	} else if (codepoint >= 0x1cdu && codepoint <= 0x1e3u) {
		return tall_block2[codepoint - 0x1cdu];
	} else if (codepoint >= 0x1e6u && codepoint <= 0x1e8u) {
		return tall_block3[codepoint - 0x1e6u];
	} else if (codepoint == 0x1f0u) {
		return tall_block4[0];
	} else if (codepoint >= 0x1f4u && codepoint <= 0x1f5u) {
		return tall_block5[codepoint - 0x1f4u];
	} else if (codepoint >= 0x1f8u && codepoint <= 0x21bu) {
		return tall_block6[codepoint - 0x1f8u];
	} else if (codepoint >= 0x226u && codepoint <= 0x227u) {
		return tall_block7[codepoint - 0x226u];
	} else if (codepoint >= 0x22au && codepoint <= 0x233u) {
		return tall_block8[codepoint - 0x22au];
	} else if (codepoint >= 0x258u && codepoint <= 0x259u) {
		return tall_block9[codepoint - 0x258u];
	} else if (codepoint >= 0x25bu && codepoint <= 0x25cu) {
		return tall_block10[codepoint - 0x25bu];
	} else if (codepoint == 0x263u) {
		return tall_block11[0];
	} else if (codepoint == 0x26au) {
		return tall_block12[0];
	} else if (codepoint >= 0x28au && codepoint <= 0x296u) {
		return tall_block13[codepoint - 0x28au];
	} else if (codepoint == 0x2c6u) {
		return tall_block14[0];
	} else if (codepoint == 0x2c9u) {
		return tall_block15[0];
	} else if (codepoint == 0x2cdu) {
		return tall_block16[0];
	} else if (codepoint == 0x2dcu) {
		return tall_block17[0];
	} else if (codepoint == 0x34fu) {
		return tall_block18[0];
	} else if (codepoint >= 0x391u && codepoint <= 0x3a1u) {
		return tall_block19[codepoint - 0x391u];
	} else if (codepoint >= 0x3a3u && codepoint <= 0x3a9u) {
		return tall_block20[codepoint - 0x3a3u];
	} else if (codepoint >= 0x3b1u && codepoint <= 0x3c9u) {
		return tall_block21[codepoint - 0x3b1u];
	} else if (codepoint >= 0x400u && codepoint <= 0x401u) {
		return tall_block22[codepoint - 0x400u];
	} else if (codepoint >= 0x405u && codepoint <= 0x409u) {
		return tall_block23[codepoint - 0x405u];
	} else if (codepoint >= 0x410u && codepoint <= 0x451u) {
		return tall_block24[codepoint - 0x410u];
	} else if (codepoint >= 0x455u && codepoint <= 0x458u) {
		return tall_block25[codepoint - 0x455u];
	} else if (codepoint >= 0x4d0u && codepoint <= 0x4d7u) {
		return tall_block26[codepoint - 0x4d0u];
	} else if (codepoint >= 0x4e6u && codepoint <= 0x4e7u) {
		return tall_block27[codepoint - 0x4e6u];
	} else if (codepoint == 0x4f1u) {
		return tall_block28[0];
	} else if (codepoint >= 0x5d0u && codepoint <= 0x5eau) {
		return tall_block29[codepoint - 0x5d0u];
	} else if (codepoint == 0x623u) {
		return tall_block30[0];
	} else if (codepoint >= 0x627u && codepoint <= 0x62du) {
		return tall_block31[codepoint - 0x627u];
	} else if (codepoint >= 0x62fu && codepoint <= 0x635u) {
		return tall_block32[codepoint - 0x62fu];
	} else if (codepoint >= 0x637u && codepoint <= 0x63au) {
		return tall_block33[codepoint - 0x637u];
	} else if (codepoint >= 0x640u && codepoint <= 0x64au) {
		return tall_block34[codepoint - 0x640u];
	} else if (codepoint >= 0x671u && codepoint <= 0x673u) {
		return tall_block35[codepoint - 0x671u];
	} else if (codepoint >= 0x1560u && codepoint <= 0x1563u) {
		return tall_block36[codepoint - 0x1560u];
	} else if (codepoint == 0x156au) {
		return tall_block37[0];
	} else if (codepoint >= 0x156cu && codepoint <= 0x1571u) {
		return tall_block38[codepoint - 0x156cu];
	} else if (codepoint >= 0x15ecu && codepoint <= 0x15edu) {
		return tall_block39[codepoint - 0x15ecu];
	} else if (codepoint == 0x15efu) {
		return tall_block40[0];
	} else if (codepoint >= 0x16a0u && codepoint <= 0x16abu) {
		return tall_block41[codepoint - 0x16a0u];
	} else if (codepoint >= 0x16b1u && codepoint <= 0x16f8u) {
		return tall_block42[codepoint - 0x16b1u];
	} else if (codepoint >= 0x1e00u && codepoint <= 0x1e02u) {
		return tall_block43[codepoint - 0x1e00u];
	} else if (codepoint >= 0x1e04u && codepoint <= 0x1e07u) {
		return tall_block44[codepoint - 0x1e04u];
	} else if (codepoint == 0x1e0au) {
		return tall_block45[0];
	} else if (codepoint >= 0x1e0cu && codepoint <= 0x1e0fu) {
		return tall_block46[codepoint - 0x1e0cu];
	} else if (codepoint >= 0x1e41u && codepoint <= 0x1e4bu) {
		return tall_block47[codepoint - 0x1e41u];
	} else if (codepoint >= 0x1e58u && codepoint <= 0x1e5bu) {
		return tall_block48[codepoint - 0x1e58u];
	} else if (codepoint == 0x1e5eu) {
		return tall_block49[0];
	} else if (codepoint >= 0x2010u && codepoint <= 0x2019u) {
		return tall_block50[codepoint - 0x2010u];
	} else if (codepoint >= 0x201cu && codepoint <= 0x201du) {
		return tall_block51[codepoint - 0x201cu];
	} else if (codepoint >= 0x2020u && codepoint <= 0x2022u) {
		return tall_block52[codepoint - 0x2020u];
	} else if (codepoint >= 0x2024u && codepoint <= 0x2026u) {
		return tall_block53[codepoint - 0x2024u];
	} else if (codepoint >= 0x2030u && codepoint <= 0x2031u) {
		return tall_block54[codepoint - 0x2030u];
	} else if (codepoint == 0x203cu) {
		return tall_block55[0];
	} else if (codepoint >= 0x2047u && codepoint <= 0x2049u) {
		return tall_block56[codepoint - 0x2047u];
	} else if (codepoint == 0x2070u) {
		return tall_block57[0];
	} else if (codepoint >= 0x2074u && codepoint <= 0x2079u) {
		return tall_block58[codepoint - 0x2074u];
	} else if (codepoint == 0x207fu) {
		return tall_block59[0];
	} else if (codepoint == 0x20a7u) {
		return tall_block60[0];
	} else if (codepoint == 0x20acu) {
		return tall_block61[0];
	} else if (codepoint == 0x2117u) {
		return tall_block62[0];
	} else if (codepoint == 0x2120u) {
		return tall_block63[0];
	} else if (codepoint == 0x2122u) {
		return tall_block64[0];
	} else if (codepoint >= 0x2190u && codepoint <= 0x2192u) {
		return tall_block65[codepoint - 0x2190u];
	} else if (codepoint == 0x2194u) {
		return tall_block66[0];
	} else if (codepoint >= 0x2196u && codepoint <= 0x2199u) {
		return tall_block67[codepoint - 0x2196u];
	} else if (codepoint == 0x21a5u) {
		return tall_block68[0];
	} else if (codepoint == 0x21a8u) {
		return tall_block69[0];
	} else if (codepoint >= 0x21e6u && codepoint <= 0x21e9u) {
		return tall_block70[codepoint - 0x21e6u];
	} else if (codepoint == 0x2212u) {
		return tall_block71[0];
	} else if (codepoint == 0x2218u) {
		return tall_block72[0];
	} else if (codepoint == 0x221au) {
		return tall_block73[0];
	} else if (codepoint >= 0x221eu && codepoint <= 0x221fu) {
		return tall_block74[codepoint - 0x221eu];
	} else if (codepoint == 0x2229u) {
		return tall_block75[0];
	} else if (codepoint == 0x2248u) {
		return tall_block76[0];
	} else if (codepoint == 0x2261u) {
		return tall_block77[0];
	} else if (codepoint >= 0x2264u && codepoint <= 0x2265u) {
		return tall_block78[codepoint - 0x2264u];
	} else if (codepoint == 0x22c5u) {
		return tall_block79[0];
	} else if (codepoint >= 0x2320u && codepoint <= 0x2321u) {
		return tall_block80[codepoint - 0x2320u];
	} else if (codepoint >= 0x23eau && codepoint <= 0x23ecu) {
		return tall_block81[codepoint - 0x23eau];
	} else if (codepoint >= 0x2500u && codepoint <= 0x25ffu) {
		return tall_block82[codepoint - 0x2500u];
	} else if (codepoint == 0x2625u) {
		return tall_block83[0];
	} else if (codepoint == 0x2628u) {
		return tall_block84[0];
	} else if (codepoint >= 0x262fu && codepoint <= 0x2637u) {
		return tall_block85[codepoint - 0x262fu];
	} else if (codepoint >= 0x2639u && codepoint <= 0x263bu) {
		return tall_block86[codepoint - 0x2639u];
	} else if (codepoint >= 0x2660u && codepoint <= 0x2667u) {
		return tall_block87[codepoint - 0x2660u];
	} else if (codepoint >= 0x2669u && codepoint <= 0x266cu) {
		return tall_block88[codepoint - 0x2669u];
	} else if (codepoint >= 0x268au && codepoint <= 0x268fu) {
		return tall_block89[codepoint - 0x268au];
	} else if (codepoint >= 0x26aau && codepoint <= 0x26acu) {
		return tall_block90[codepoint - 0x26aau];
	} else if (codepoint == 0x2708u) {
		return tall_block91[0];
	} else if (codepoint == 0x2734u) {
		return tall_block92[0];
	} else if (codepoint >= 0x2800u && codepoint <= 0x28ffu) {
		return tall_block93[codepoint - 0x2800u];
	} else if (codepoint == 0x2913u) {
		return tall_block94[0];
	} else if (codepoint == 0x2b1du) {
		return tall_block95[0];
	} else if (codepoint == 0x2b24u) {
		return tall_block96[0];
	} else if (codepoint == 0x2b55u) {
		return tall_block97[0];
	} else if (codepoint == 0x2b58u) {
		return tall_block98[0];
	} else if (codepoint == 0x2e2eu) {
		return tall_block99[0];
	} else if (codepoint >= 0xe080u && codepoint <= 0xe19bu) {
		return tall_block100[codepoint - 0xe080u];
	} else if (codepoint >= 0xe800u && codepoint <= 0xe850u) {
		return tall_block101[codepoint - 0xe800u];
	} else if (codepoint >= 0xec00u && codepoint <= 0xec26u) {
		return tall_block102[codepoint - 0xec00u];
	} else if (codepoint >= 0xfe81u && codepoint <= 0xfe82u) {
		return tall_block103[codepoint - 0xfe81u];
	} else if (codepoint == 0xfe84u) {
		return tall_block104[0];
	} else if (codepoint >= 0xfe87u && codepoint <= 0xfe88u) {
		return tall_block105[codepoint - 0xfe87u];
	} else if (codepoint >= 0xfe8eu && codepoint <= 0xfe9cu) {
		return tall_block106[codepoint - 0xfe8eu];
	} else if (codepoint >= 0xfe9eu && codepoint <= 0xfea4u) {
		return tall_block107[codepoint - 0xfe9eu];
	} else if (codepoint >= 0xfea9u && codepoint <= 0xfeb7u) {
		return tall_block108[codepoint - 0xfea9u];
	} else if (codepoint >= 0xfeb9u && codepoint <= 0xfebcu) {
		return tall_block109[codepoint - 0xfeb9u];
	} else if (codepoint >= 0xfec1u && codepoint <= 0xfec6u) {
		return tall_block110[codepoint - 0xfec1u];
	} else if (codepoint >= 0xfec9u && codepoint <= 0xfee3u) {
		return tall_block111[codepoint - 0xfec9u];
	} else if (codepoint >= 0xfee5u && codepoint <= 0xfef4u) {
		return tall_block112[codepoint - 0xfee5u];
	} else if (codepoint >= 0xff01u && codepoint <= 0xff3au) {
		return tall_block113[codepoint - 0xff01u];
	} else if (codepoint == 0xff3cu) {
		return tall_block114[0];
	} else if (codepoint >= 0xff3eu && codepoint <= 0xff5au) {
		return tall_block115[codepoint - 0xff3eu];
	} else if (codepoint >= 0xff5cu && codepoint <= 0xff5eu) {
		return tall_block116[codepoint - 0xff5cu];
	} else if (codepoint >= 0xff61u && codepoint <= 0xff9fu) {
		return tall_block117[codepoint - 0xff61u];
	} else {
		WARN("Codepoint U+%04X (%s) is not covered by this font", codepoint, u8_cp_to_utf8(codepoint));
		return tall_block1[0];
	}
}
