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

#include "fbink_orp.h"

static const unsigned char*
    orp_get_bitmap(uint32_t codepoint)
{
	if (codepoint == 0x00u) {
		return orp_block1[codepoint];
	} else if (codepoint >= 0x20u && codepoint <= 0x7eu) {
		return orp_block2[codepoint - 0x20u];
	} else if (codepoint >= 0xa0u && codepoint <= 0x377u) {
		return orp_block3[codepoint - 0xa0u];
	} else if (codepoint >= 0x37au && codepoint <= 0x37eu) {
		return orp_block4[codepoint - 0x37au];
	} else if (codepoint >= 0x384u && codepoint <= 0x38au) {
		return orp_block5[codepoint - 0x384u];
	} else if (codepoint == 0x38cu) {
		return orp_block6[0];
	} else if (codepoint >= 0x38eu && codepoint <= 0x3a1u) {
		return orp_block7[codepoint - 0x38eu];
	} else if (codepoint >= 0x3a3u && codepoint <= 0x523u) {
		return orp_block8[codepoint - 0x3a3u];
	} else if (codepoint >= 0x531u && codepoint <= 0x556u) {
		return orp_block9[codepoint - 0x531u];
	} else if (codepoint >= 0x559u && codepoint <= 0x55fu) {
		return orp_block10[codepoint - 0x559u];
	} else if (codepoint >= 0x561u && codepoint <= 0x587u) {
		return orp_block11[codepoint - 0x561u];
	} else if (codepoint >= 0x589u && codepoint <= 0x58au) {
		return orp_block12[codepoint - 0x589u];
	} else if (codepoint >= 0x591u && codepoint <= 0x5c7u) {
		return orp_block13[codepoint - 0x591u];
	} else if (codepoint >= 0x5d0u && codepoint <= 0x5eau) {
		return orp_block14[codepoint - 0x5d0u];
	} else if (codepoint >= 0x5f0u && codepoint <= 0x5f4u) {
		return orp_block15[codepoint - 0x5f0u];
	} else if (codepoint >= 0x1680u && codepoint <= 0x169cu) {
		return orp_block16[codepoint - 0x1680u];
	} else if (codepoint >= 0x16a0u && codepoint <= 0x16f0u) {
		return orp_block17[codepoint - 0x16a0u];
	} else if (codepoint >= 0x1e02u && codepoint <= 0x1e07u) {
		return orp_block18[codepoint - 0x1e02u];
	} else if (codepoint >= 0x1e0au && codepoint <= 0x1e13u) {
		return orp_block19[codepoint - 0x1e0au];
	} else if (codepoint >= 0x1e1eu && codepoint <= 0x1e1fu) {
		return orp_block20[codepoint - 0x1e1eu];
	} else if (codepoint >= 0x1e30u && codepoint <= 0x1e35u) {
		return orp_block21[codepoint - 0x1e30u];
	} else if (codepoint >= 0x1e3eu && codepoint <= 0x1e43u) {
		return orp_block22[codepoint - 0x1e3eu];
	} else if (codepoint >= 0x1e54u && codepoint <= 0x1e57u) {
		return orp_block23[codepoint - 0x1e54u];
	} else if (codepoint >= 0x1e60u && codepoint <= 0x1e71u) {
		return orp_block24[codepoint - 0x1e60u];
	} else if (codepoint >= 0x1e80u && codepoint <= 0x1e8fu) {
		return orp_block25[codepoint - 0x1e80u];
	} else if (codepoint >= 0x1ef2u && codepoint <= 0x1ef9u) {
		return orp_block26[codepoint - 0x1ef2u];
	} else if (codepoint >= 0x1f00u && codepoint <= 0x1f15u) {
		return orp_block27[codepoint - 0x1f00u];
	} else if (codepoint >= 0x1f18u && codepoint <= 0x1f1du) {
		return orp_block28[codepoint - 0x1f18u];
	} else if (codepoint >= 0x1f20u && codepoint <= 0x1f45u) {
		return orp_block29[codepoint - 0x1f20u];
	} else if (codepoint >= 0x1f48u && codepoint <= 0x1f4du) {
		return orp_block30[codepoint - 0x1f48u];
	} else if (codepoint >= 0x1f50u && codepoint <= 0x1f57u) {
		return orp_block31[codepoint - 0x1f50u];
	} else if (codepoint == 0x1f59u) {
		return orp_block32[0];
	} else if (codepoint == 0x1f5bu) {
		return orp_block33[0];
	} else if (codepoint == 0x1f5du) {
		return orp_block34[0];
	} else if (codepoint >= 0x1f5fu && codepoint <= 0x1f7du) {
		return orp_block35[codepoint - 0x1f5fu];
	} else if (codepoint >= 0x1f80u && codepoint <= 0x1fb4u) {
		return orp_block36[codepoint - 0x1f80u];
	} else if (codepoint >= 0x1fb6u && codepoint <= 0x1fc4u) {
		return orp_block37[codepoint - 0x1fb6u];
	} else if (codepoint >= 0x1fc6u && codepoint <= 0x1fd3u) {
		return orp_block38[codepoint - 0x1fc6u];
	} else if (codepoint >= 0x1fd6u && codepoint <= 0x1fdbu) {
		return orp_block39[codepoint - 0x1fd6u];
	} else if (codepoint >= 0x1fddu && codepoint <= 0x1fefu) {
		return orp_block40[codepoint - 0x1fddu];
	} else if (codepoint >= 0x1ff2u && codepoint <= 0x1ff4u) {
		return orp_block41[codepoint - 0x1ff2u];
	} else if (codepoint >= 0x1ff6u && codepoint <= 0x1ffeu) {
		return orp_block42[codepoint - 0x1ff6u];
	} else if (codepoint >= 0x2010u && codepoint <= 0x2027u) {
		return orp_block43[codepoint - 0x2010u];
	} else if (codepoint >= 0x2030u && codepoint <= 0x205eu) {
		return orp_block44[codepoint - 0x2030u];
	} else if (codepoint >= 0x2070u && codepoint <= 0x2071u) {
		return orp_block45[codepoint - 0x2070u];
	} else if (codepoint >= 0x2074u && codepoint <= 0x208eu) {
		return orp_block46[codepoint - 0x2074u];
	} else if (codepoint >= 0x2090u && codepoint <= 0x2094u) {
		return orp_block47[codepoint - 0x2090u];
	} else if (codepoint >= 0x20a0u && codepoint <= 0x20b5u) {
		return orp_block48[codepoint - 0x20a0u];
	} else if (codepoint >= 0x20d0u && codepoint <= 0x20f0u) {
		return orp_block49[codepoint - 0x20d0u];
	} else if (codepoint >= 0x2100u && codepoint <= 0x214fu) {
		return orp_block50[codepoint - 0x2100u];
	} else if (codepoint >= 0x2153u && codepoint <= 0x2188u) {
		return orp_block51[codepoint - 0x2153u];
	} else if (codepoint >= 0x2190u && codepoint <= 0x2328u) {
		return orp_block52[codepoint - 0x2190u];
	} else if (codepoint >= 0x232bu && codepoint <= 0x23e7u) {
		return orp_block53[codepoint - 0x232bu];
	} else if (codepoint >= 0x2400u && codepoint <= 0x2426u) {
		return orp_block54[codepoint - 0x2400u];
	} else if (codepoint >= 0x2440u && codepoint <= 0x244au) {
		return orp_block55[codepoint - 0x2440u];
	} else if (codepoint >= 0x2460u && codepoint <= 0x2613u) {
		return orp_block56[codepoint - 0x2460u];
	} else if (codepoint >= 0x2616u && codepoint <= 0x2617u) {
		return orp_block57[codepoint - 0x2616u];
	} else if (codepoint >= 0x2619u && codepoint <= 0x269cu) {
		return orp_block58[codepoint - 0x2619u];
	} else if (codepoint >= 0x26a0u && codepoint <= 0x26bcu) {
		return orp_block59[codepoint - 0x26a0u];
	} else if (codepoint >= 0x26c0u && codepoint <= 0x26c3u) {
		return orp_block60[codepoint - 0x26c0u];
	} else if (codepoint >= 0x2701u && codepoint <= 0x2704u) {
		return orp_block61[codepoint - 0x2701u];
	} else if (codepoint >= 0x2706u && codepoint <= 0x2709u) {
		return orp_block62[codepoint - 0x2706u];
	} else if (codepoint >= 0x270cu && codepoint <= 0x2727u) {
		return orp_block63[codepoint - 0x270cu];
	} else if (codepoint >= 0x2729u && codepoint <= 0x274bu) {
		return orp_block64[codepoint - 0x2729u];
	} else if (codepoint == 0x274du) {
		return orp_block65[0];
	} else if (codepoint >= 0x274fu && codepoint <= 0x2752u) {
		return orp_block66[codepoint - 0x274fu];
	} else if (codepoint == 0x2756u) {
		return orp_block67[0];
	} else if (codepoint >= 0x2758u && codepoint <= 0x275eu) {
		return orp_block68[codepoint - 0x2758u];
	} else if (codepoint >= 0x2761u && codepoint <= 0x2794u) {
		return orp_block69[codepoint - 0x2761u];
	} else if (codepoint >= 0x2798u && codepoint <= 0x27afu) {
		return orp_block70[codepoint - 0x2798u];
	} else if (codepoint >= 0x27b1u && codepoint <= 0x27beu) {
		return orp_block71[codepoint - 0x27b1u];
	} else if (codepoint >= 0x27c0u && codepoint <= 0x27cau) {
		return orp_block72[codepoint - 0x27c0u];
	} else if (codepoint == 0x27ccu) {
		return orp_block73[0];
	} else if (codepoint >= 0x27d0u && codepoint <= 0x2b4cu) {
		return orp_block74[codepoint - 0x27d0u];
	} else if (codepoint >= 0x2b60u && codepoint <= 0x2b64u) {
		return orp_block75[codepoint - 0x2b60u];
	} else if (codepoint >= 0x2b80u && codepoint <= 0x2b83u) {
		return orp_block76[codepoint - 0x2b80u];
	} else if (codepoint >= 0x2c60u && codepoint <= 0x2c6fu) {
		return orp_block77[codepoint - 0x2c60u];
	} else if (codepoint >= 0x2c71u && codepoint <= 0x2c7du) {
		return orp_block78[codepoint - 0x2c71u];
	} else if (codepoint >= 0x2de0u && codepoint <= 0x2dffu) {
		return orp_block79[codepoint - 0x2de0u];
	} else if (codepoint == 0xe0b2u) {
		return orp_block80[0];
	} else if (codepoint >= 0xfb00u && codepoint <= 0xfb06u) {
		return orp_block81[codepoint - 0xfb00u];
	} else if (codepoint >= 0xfe20u && codepoint <= 0xfe23u) {
		return orp_block82[codepoint - 0xfe20u];
	} else if (codepoint == 0xfffdu) {
		return orp_block83[0];
	} else {
		WARN("Codepoint U+%04X (%s) is not covered by this font", codepoint, u8_cp_to_utf8(codepoint));
		return orp_block1[0];
	}
}

static const unsigned char*
    orpb_get_bitmap(uint32_t codepoint)
{
	if (codepoint == 0x00u) {
		return orpb_block1[codepoint];
	} else if (codepoint >= 0x20u && codepoint <= 0x7eu) {
		return orpb_block2[codepoint - 0x20u];
	} else if (codepoint >= 0xa0u && codepoint <= 0x377u) {
		return orpb_block3[codepoint - 0xa0u];
	} else if (codepoint >= 0x37au && codepoint <= 0x37eu) {
		return orpb_block4[codepoint - 0x37au];
	} else if (codepoint >= 0x384u && codepoint <= 0x38au) {
		return orpb_block5[codepoint - 0x384u];
	} else if (codepoint == 0x38cu) {
		return orpb_block6[0];
	} else if (codepoint >= 0x38eu && codepoint <= 0x3a1u) {
		return orpb_block7[codepoint - 0x38eu];
	} else if (codepoint >= 0x3a3u && codepoint <= 0x523u) {
		return orpb_block8[codepoint - 0x3a3u];
	} else if (codepoint >= 0x531u && codepoint <= 0x556u) {
		return orpb_block9[codepoint - 0x531u];
	} else if (codepoint >= 0x559u && codepoint <= 0x55fu) {
		return orpb_block10[codepoint - 0x559u];
	} else if (codepoint >= 0x561u && codepoint <= 0x587u) {
		return orpb_block11[codepoint - 0x561u];
	} else if (codepoint >= 0x589u && codepoint <= 0x58au) {
		return orpb_block12[codepoint - 0x589u];
	} else if (codepoint >= 0x591u && codepoint <= 0x5c7u) {
		return orpb_block13[codepoint - 0x591u];
	} else if (codepoint >= 0x5d0u && codepoint <= 0x5eau) {
		return orpb_block14[codepoint - 0x5d0u];
	} else if (codepoint >= 0x5f0u && codepoint <= 0x5f4u) {
		return orpb_block15[codepoint - 0x5f0u];
	} else if (codepoint >= 0x1680u && codepoint <= 0x169cu) {
		return orpb_block16[codepoint - 0x1680u];
	} else if (codepoint >= 0x16a0u && codepoint <= 0x16f0u) {
		return orpb_block17[codepoint - 0x16a0u];
	} else if (codepoint >= 0x1e02u && codepoint <= 0x1e07u) {
		return orpb_block18[codepoint - 0x1e02u];
	} else if (codepoint >= 0x1e0au && codepoint <= 0x1e13u) {
		return orpb_block19[codepoint - 0x1e0au];
	} else if (codepoint >= 0x1e1eu && codepoint <= 0x1e1fu) {
		return orpb_block20[codepoint - 0x1e1eu];
	} else if (codepoint >= 0x1e30u && codepoint <= 0x1e35u) {
		return orpb_block21[codepoint - 0x1e30u];
	} else if (codepoint >= 0x1e3eu && codepoint <= 0x1e43u) {
		return orpb_block22[codepoint - 0x1e3eu];
	} else if (codepoint >= 0x1e54u && codepoint <= 0x1e57u) {
		return orpb_block23[codepoint - 0x1e54u];
	} else if (codepoint >= 0x1e60u && codepoint <= 0x1e71u) {
		return orpb_block24[codepoint - 0x1e60u];
	} else if (codepoint >= 0x1e80u && codepoint <= 0x1e8fu) {
		return orpb_block25[codepoint - 0x1e80u];
	} else if (codepoint >= 0x1ef2u && codepoint <= 0x1ef9u) {
		return orpb_block26[codepoint - 0x1ef2u];
	} else if (codepoint >= 0x1f00u && codepoint <= 0x1f15u) {
		return orpb_block27[codepoint - 0x1f00u];
	} else if (codepoint >= 0x1f18u && codepoint <= 0x1f1du) {
		return orpb_block28[codepoint - 0x1f18u];
	} else if (codepoint >= 0x1f20u && codepoint <= 0x1f45u) {
		return orpb_block29[codepoint - 0x1f20u];
	} else if (codepoint >= 0x1f48u && codepoint <= 0x1f4du) {
		return orpb_block30[codepoint - 0x1f48u];
	} else if (codepoint >= 0x1f50u && codepoint <= 0x1f57u) {
		return orpb_block31[codepoint - 0x1f50u];
	} else if (codepoint == 0x1f59u) {
		return orpb_block32[0];
	} else if (codepoint == 0x1f5bu) {
		return orpb_block33[0];
	} else if (codepoint == 0x1f5du) {
		return orpb_block34[0];
	} else if (codepoint >= 0x1f5fu && codepoint <= 0x1f7du) {
		return orpb_block35[codepoint - 0x1f5fu];
	} else if (codepoint >= 0x1f80u && codepoint <= 0x1fb4u) {
		return orpb_block36[codepoint - 0x1f80u];
	} else if (codepoint >= 0x1fb6u && codepoint <= 0x1fc4u) {
		return orpb_block37[codepoint - 0x1fb6u];
	} else if (codepoint >= 0x1fc6u && codepoint <= 0x1fd3u) {
		return orpb_block38[codepoint - 0x1fc6u];
	} else if (codepoint >= 0x1fd6u && codepoint <= 0x1fdbu) {
		return orpb_block39[codepoint - 0x1fd6u];
	} else if (codepoint >= 0x1fddu && codepoint <= 0x1fefu) {
		return orpb_block40[codepoint - 0x1fddu];
	} else if (codepoint >= 0x1ff2u && codepoint <= 0x1ff4u) {
		return orpb_block41[codepoint - 0x1ff2u];
	} else if (codepoint >= 0x1ff6u && codepoint <= 0x1ffeu) {
		return orpb_block42[codepoint - 0x1ff6u];
	} else if (codepoint >= 0x2010u && codepoint <= 0x2027u) {
		return orpb_block43[codepoint - 0x2010u];
	} else if (codepoint >= 0x2030u && codepoint <= 0x205eu) {
		return orpb_block44[codepoint - 0x2030u];
	} else if (codepoint >= 0x2070u && codepoint <= 0x2071u) {
		return orpb_block45[codepoint - 0x2070u];
	} else if (codepoint >= 0x2074u && codepoint <= 0x208eu) {
		return orpb_block46[codepoint - 0x2074u];
	} else if (codepoint >= 0x2090u && codepoint <= 0x2094u) {
		return orpb_block47[codepoint - 0x2090u];
	} else if (codepoint >= 0x20a0u && codepoint <= 0x20b5u) {
		return orpb_block48[codepoint - 0x20a0u];
	} else if (codepoint >= 0x20d0u && codepoint <= 0x20f0u) {
		return orpb_block49[codepoint - 0x20d0u];
	} else if (codepoint >= 0x2100u && codepoint <= 0x214fu) {
		return orpb_block50[codepoint - 0x2100u];
	} else if (codepoint >= 0x2153u && codepoint <= 0x2188u) {
		return orpb_block51[codepoint - 0x2153u];
	} else if (codepoint >= 0x2190u && codepoint <= 0x2328u) {
		return orpb_block52[codepoint - 0x2190u];
	} else if (codepoint >= 0x232bu && codepoint <= 0x23e7u) {
		return orpb_block53[codepoint - 0x232bu];
	} else if (codepoint >= 0x2400u && codepoint <= 0x2426u) {
		return orpb_block54[codepoint - 0x2400u];
	} else if (codepoint >= 0x2440u && codepoint <= 0x244au) {
		return orpb_block55[codepoint - 0x2440u];
	} else if (codepoint >= 0x2460u && codepoint <= 0x2613u) {
		return orpb_block56[codepoint - 0x2460u];
	} else if (codepoint >= 0x2616u && codepoint <= 0x2617u) {
		return orpb_block57[codepoint - 0x2616u];
	} else if (codepoint >= 0x2619u && codepoint <= 0x269cu) {
		return orpb_block58[codepoint - 0x2619u];
	} else if (codepoint >= 0x26a0u && codepoint <= 0x26bcu) {
		return orpb_block59[codepoint - 0x26a0u];
	} else if (codepoint >= 0x26c0u && codepoint <= 0x26c3u) {
		return orpb_block60[codepoint - 0x26c0u];
	} else if (codepoint >= 0x2701u && codepoint <= 0x2704u) {
		return orpb_block61[codepoint - 0x2701u];
	} else if (codepoint >= 0x2706u && codepoint <= 0x2709u) {
		return orpb_block62[codepoint - 0x2706u];
	} else if (codepoint >= 0x270cu && codepoint <= 0x2727u) {
		return orpb_block63[codepoint - 0x270cu];
	} else if (codepoint >= 0x2729u && codepoint <= 0x274bu) {
		return orpb_block64[codepoint - 0x2729u];
	} else if (codepoint == 0x274du) {
		return orpb_block65[0];
	} else if (codepoint >= 0x274fu && codepoint <= 0x2752u) {
		return orpb_block66[codepoint - 0x274fu];
	} else if (codepoint == 0x2756u) {
		return orpb_block67[0];
	} else if (codepoint >= 0x2758u && codepoint <= 0x275eu) {
		return orpb_block68[codepoint - 0x2758u];
	} else if (codepoint >= 0x2761u && codepoint <= 0x2794u) {
		return orpb_block69[codepoint - 0x2761u];
	} else if (codepoint >= 0x2798u && codepoint <= 0x27afu) {
		return orpb_block70[codepoint - 0x2798u];
	} else if (codepoint >= 0x27b1u && codepoint <= 0x27beu) {
		return orpb_block71[codepoint - 0x27b1u];
	} else if (codepoint >= 0x27c0u && codepoint <= 0x27cau) {
		return orpb_block72[codepoint - 0x27c0u];
	} else if (codepoint == 0x27ccu) {
		return orpb_block73[0];
	} else if (codepoint >= 0x27d0u && codepoint <= 0x2b4cu) {
		return orpb_block74[codepoint - 0x27d0u];
	} else if (codepoint >= 0x2c60u && codepoint <= 0x2c6fu) {
		return orpb_block75[codepoint - 0x2c60u];
	} else if (codepoint >= 0x2c71u && codepoint <= 0x2c7du) {
		return orpb_block76[codepoint - 0x2c71u];
	} else if (codepoint >= 0x2de0u && codepoint <= 0x2dffu) {
		return orpb_block77[codepoint - 0x2de0u];
	} else if (codepoint >= 0xfb00u && codepoint <= 0xfb06u) {
		return orpb_block78[codepoint - 0xfb00u];
	} else if (codepoint >= 0xfe20u && codepoint <= 0xfe23u) {
		return orpb_block79[codepoint - 0xfe20u];
	} else if (codepoint == 0xfffdu) {
		return orpb_block80[0];
	} else {
		WARN("Codepoint U+%04X (%s) is not covered by this font", codepoint, u8_cp_to_utf8(codepoint));
		return orpb_block1[0];
	}
}

static const unsigned char*
    orpi_get_bitmap(uint32_t codepoint)
{
	if (codepoint == 0x00u) {
		return orpi_block1[codepoint];
	} else if (codepoint >= 0x20u && codepoint <= 0x7eu) {
		return orpi_block2[codepoint - 0x20u];
	} else if (codepoint >= 0xa0u && codepoint <= 0x377u) {
		return orpi_block3[codepoint - 0xa0u];
	} else if (codepoint >= 0x37au && codepoint <= 0x37eu) {
		return orpi_block4[codepoint - 0x37au];
	} else if (codepoint >= 0x384u && codepoint <= 0x38au) {
		return orpi_block5[codepoint - 0x384u];
	} else if (codepoint == 0x38cu) {
		return orpi_block6[0];
	} else if (codepoint >= 0x38eu && codepoint <= 0x3a1u) {
		return orpi_block7[codepoint - 0x38eu];
	} else if (codepoint >= 0x3a3u && codepoint <= 0x523u) {
		return orpi_block8[codepoint - 0x3a3u];
	} else if (codepoint >= 0x531u && codepoint <= 0x556u) {
		return orpi_block9[codepoint - 0x531u];
	} else if (codepoint >= 0x559u && codepoint <= 0x55fu) {
		return orpi_block10[codepoint - 0x559u];
	} else if (codepoint >= 0x561u && codepoint <= 0x587u) {
		return orpi_block11[codepoint - 0x561u];
	} else if (codepoint >= 0x589u && codepoint <= 0x58au) {
		return orpi_block12[codepoint - 0x589u];
	} else if (codepoint >= 0x591u && codepoint <= 0x5c7u) {
		return orpi_block13[codepoint - 0x591u];
	} else if (codepoint >= 0x5d0u && codepoint <= 0x5eau) {
		return orpi_block14[codepoint - 0x5d0u];
	} else if (codepoint >= 0x5f0u && codepoint <= 0x5f4u) {
		return orpi_block15[codepoint - 0x5f0u];
	} else if (codepoint >= 0x1680u && codepoint <= 0x169cu) {
		return orpi_block16[codepoint - 0x1680u];
	} else if (codepoint >= 0x16a0u && codepoint <= 0x16f0u) {
		return orpi_block17[codepoint - 0x16a0u];
	} else if (codepoint >= 0x1e02u && codepoint <= 0x1e07u) {
		return orpi_block18[codepoint - 0x1e02u];
	} else if (codepoint >= 0x1e0au && codepoint <= 0x1e13u) {
		return orpi_block19[codepoint - 0x1e0au];
	} else if (codepoint >= 0x1e1eu && codepoint <= 0x1e1fu) {
		return orpi_block20[codepoint - 0x1e1eu];
	} else if (codepoint >= 0x1e30u && codepoint <= 0x1e35u) {
		return orpi_block21[codepoint - 0x1e30u];
	} else if (codepoint >= 0x1e3eu && codepoint <= 0x1e43u) {
		return orpi_block22[codepoint - 0x1e3eu];
	} else if (codepoint >= 0x1e54u && codepoint <= 0x1e57u) {
		return orpi_block23[codepoint - 0x1e54u];
	} else if (codepoint >= 0x1e60u && codepoint <= 0x1e71u) {
		return orpi_block24[codepoint - 0x1e60u];
	} else if (codepoint >= 0x1e80u && codepoint <= 0x1e8fu) {
		return orpi_block25[codepoint - 0x1e80u];
	} else if (codepoint >= 0x1ef2u && codepoint <= 0x1ef9u) {
		return orpi_block26[codepoint - 0x1ef2u];
	} else if (codepoint >= 0x1f00u && codepoint <= 0x1f15u) {
		return orpi_block27[codepoint - 0x1f00u];
	} else if (codepoint >= 0x1f18u && codepoint <= 0x1f1du) {
		return orpi_block28[codepoint - 0x1f18u];
	} else if (codepoint >= 0x1f20u && codepoint <= 0x1f45u) {
		return orpi_block29[codepoint - 0x1f20u];
	} else if (codepoint >= 0x1f48u && codepoint <= 0x1f4du) {
		return orpi_block30[codepoint - 0x1f48u];
	} else if (codepoint >= 0x1f50u && codepoint <= 0x1f57u) {
		return orpi_block31[codepoint - 0x1f50u];
	} else if (codepoint == 0x1f59u) {
		return orpi_block32[0];
	} else if (codepoint == 0x1f5bu) {
		return orpi_block33[0];
	} else if (codepoint == 0x1f5du) {
		return orpi_block34[0];
	} else if (codepoint >= 0x1f5fu && codepoint <= 0x1f7du) {
		return orpi_block35[codepoint - 0x1f5fu];
	} else if (codepoint >= 0x1f80u && codepoint <= 0x1fb4u) {
		return orpi_block36[codepoint - 0x1f80u];
	} else if (codepoint >= 0x1fb6u && codepoint <= 0x1fc4u) {
		return orpi_block37[codepoint - 0x1fb6u];
	} else if (codepoint >= 0x1fc6u && codepoint <= 0x1fd3u) {
		return orpi_block38[codepoint - 0x1fc6u];
	} else if (codepoint >= 0x1fd6u && codepoint <= 0x1fdbu) {
		return orpi_block39[codepoint - 0x1fd6u];
	} else if (codepoint >= 0x1fddu && codepoint <= 0x1fefu) {
		return orpi_block40[codepoint - 0x1fddu];
	} else if (codepoint >= 0x1ff2u && codepoint <= 0x1ff4u) {
		return orpi_block41[codepoint - 0x1ff2u];
	} else if (codepoint >= 0x1ff6u && codepoint <= 0x1ffeu) {
		return orpi_block42[codepoint - 0x1ff6u];
	} else if (codepoint >= 0x2010u && codepoint <= 0x2027u) {
		return orpi_block43[codepoint - 0x2010u];
	} else if (codepoint >= 0x2030u && codepoint <= 0x205eu) {
		return orpi_block44[codepoint - 0x2030u];
	} else if (codepoint >= 0x2070u && codepoint <= 0x2071u) {
		return orpi_block45[codepoint - 0x2070u];
	} else if (codepoint >= 0x2074u && codepoint <= 0x208eu) {
		return orpi_block46[codepoint - 0x2074u];
	} else if (codepoint >= 0x2090u && codepoint <= 0x2094u) {
		return orpi_block47[codepoint - 0x2090u];
	} else if (codepoint >= 0x20a0u && codepoint <= 0x20b5u) {
		return orpi_block48[codepoint - 0x20a0u];
	} else if (codepoint >= 0x20d0u && codepoint <= 0x20f0u) {
		return orpi_block49[codepoint - 0x20d0u];
	} else if (codepoint >= 0x2100u && codepoint <= 0x214fu) {
		return orpi_block50[codepoint - 0x2100u];
	} else if (codepoint >= 0x2153u && codepoint <= 0x2188u) {
		return orpi_block51[codepoint - 0x2153u];
	} else if (codepoint >= 0x2190u && codepoint <= 0x2328u) {
		return orpi_block52[codepoint - 0x2190u];
	} else if (codepoint >= 0x232bu && codepoint <= 0x23e7u) {
		return orpi_block53[codepoint - 0x232bu];
	} else if (codepoint >= 0x2400u && codepoint <= 0x2426u) {
		return orpi_block54[codepoint - 0x2400u];
	} else if (codepoint >= 0x2440u && codepoint <= 0x244au) {
		return orpi_block55[codepoint - 0x2440u];
	} else if (codepoint >= 0x2460u && codepoint <= 0x2613u) {
		return orpi_block56[codepoint - 0x2460u];
	} else if (codepoint >= 0x2616u && codepoint <= 0x2617u) {
		return orpi_block57[codepoint - 0x2616u];
	} else if (codepoint >= 0x2619u && codepoint <= 0x269cu) {
		return orpi_block58[codepoint - 0x2619u];
	} else if (codepoint >= 0x26a0u && codepoint <= 0x26bcu) {
		return orpi_block59[codepoint - 0x26a0u];
	} else if (codepoint >= 0x26c0u && codepoint <= 0x26c3u) {
		return orpi_block60[codepoint - 0x26c0u];
	} else if (codepoint >= 0x2701u && codepoint <= 0x2704u) {
		return orpi_block61[codepoint - 0x2701u];
	} else if (codepoint >= 0x2706u && codepoint <= 0x2709u) {
		return orpi_block62[codepoint - 0x2706u];
	} else if (codepoint >= 0x270cu && codepoint <= 0x2727u) {
		return orpi_block63[codepoint - 0x270cu];
	} else if (codepoint >= 0x2729u && codepoint <= 0x274bu) {
		return orpi_block64[codepoint - 0x2729u];
	} else if (codepoint == 0x274du) {
		return orpi_block65[0];
	} else if (codepoint >= 0x274fu && codepoint <= 0x2752u) {
		return orpi_block66[codepoint - 0x274fu];
	} else if (codepoint == 0x2756u) {
		return orpi_block67[0];
	} else if (codepoint >= 0x2758u && codepoint <= 0x275eu) {
		return orpi_block68[codepoint - 0x2758u];
	} else if (codepoint >= 0x2761u && codepoint <= 0x2794u) {
		return orpi_block69[codepoint - 0x2761u];
	} else if (codepoint >= 0x2798u && codepoint <= 0x27afu) {
		return orpi_block70[codepoint - 0x2798u];
	} else if (codepoint >= 0x27b1u && codepoint <= 0x27beu) {
		return orpi_block71[codepoint - 0x27b1u];
	} else if (codepoint >= 0x27c0u && codepoint <= 0x27cau) {
		return orpi_block72[codepoint - 0x27c0u];
	} else if (codepoint == 0x27ccu) {
		return orpi_block73[0];
	} else if (codepoint >= 0x27d0u && codepoint <= 0x2b4cu) {
		return orpi_block74[codepoint - 0x27d0u];
	} else if (codepoint >= 0x2c60u && codepoint <= 0x2c6fu) {
		return orpi_block75[codepoint - 0x2c60u];
	} else if (codepoint >= 0x2c71u && codepoint <= 0x2c7du) {
		return orpi_block76[codepoint - 0x2c71u];
	} else if (codepoint >= 0x2de0u && codepoint <= 0x2dffu) {
		return orpi_block77[codepoint - 0x2de0u];
	} else if (codepoint >= 0xfb00u && codepoint <= 0xfb06u) {
		return orpi_block78[codepoint - 0xfb00u];
	} else if (codepoint >= 0xfe20u && codepoint <= 0xfe23u) {
		return orpi_block79[codepoint - 0xfe20u];
	} else if (codepoint == 0xfffdu) {
		return orpi_block80[0];
	} else {
		WARN("Codepoint U+%04X (%s) is not covered by this font", codepoint, u8_cp_to_utf8(codepoint));
		return orpi_block1[0];
	}
}
