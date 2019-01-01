/*
	FBInk: FrameBuffer eInker, a tool to print text & images on eInk devices (Kobo/Kindle)
	Copyright (C) 2018-2019 NiLuJe <ninuje@gmail.com>

	----

	This program is free software: you can redistribute it and/or modify
	it under the terms of the GNU Affero General Public License as
	published by the Free Software Foundation, either version 3 of the
	License, or (at your option) any later version.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU Affero General Public License for more details.

	You should have received a copy of the GNU Affero General Public License
	along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "fbink_orp.h"

static const unsigned char*
    orp_get_bitmap(uint32_t codepoint)
{
	if (codepoint <= 0x00) {
		return orp_block1[codepoint];
	} else if (codepoint >= 0x20 && codepoint <= 0x7e) {
		return orp_block2[codepoint - 0x20];
	} else if (codepoint >= 0xa0 && codepoint <= 0x377) {
		return orp_block3[codepoint - 0xa0];
	} else if (codepoint >= 0x37a && codepoint <= 0x37e) {
		return orp_block4[codepoint - 0x37a];
	} else if (codepoint >= 0x384 && codepoint <= 0x38a) {
		return orp_block5[codepoint - 0x384];
	} else if (codepoint == 0x38c) {
		return orp_block6[0];
	} else if (codepoint >= 0x38e && codepoint <= 0x3a1) {
		return orp_block7[codepoint - 0x38e];
	} else if (codepoint >= 0x3a3 && codepoint <= 0x523) {
		return orp_block8[codepoint - 0x3a3];
	} else if (codepoint >= 0x531 && codepoint <= 0x556) {
		return orp_block9[codepoint - 0x531];
	} else if (codepoint >= 0x559 && codepoint <= 0x55f) {
		return orp_block10[codepoint - 0x559];
	} else if (codepoint >= 0x561 && codepoint <= 0x587) {
		return orp_block11[codepoint - 0x561];
	} else if (codepoint >= 0x589 && codepoint <= 0x58a) {
		return orp_block12[codepoint - 0x589];
	} else if (codepoint >= 0x591 && codepoint <= 0x5c7) {
		return orp_block13[codepoint - 0x591];
	} else if (codepoint >= 0x5d0 && codepoint <= 0x5ea) {
		return orp_block14[codepoint - 0x5d0];
	} else if (codepoint >= 0x5f0 && codepoint <= 0x5f4) {
		return orp_block15[codepoint - 0x5f0];
	} else if (codepoint >= 0x1680 && codepoint <= 0x169c) {
		return orp_block16[codepoint - 0x1680];
	} else if (codepoint >= 0x16a0 && codepoint <= 0x16f0) {
		return orp_block17[codepoint - 0x16a0];
	} else if (codepoint >= 0x1e02 && codepoint <= 0x1e07) {
		return orp_block18[codepoint - 0x1e02];
	} else if (codepoint >= 0x1e0a && codepoint <= 0x1e13) {
		return orp_block19[codepoint - 0x1e0a];
	} else if (codepoint >= 0x1e1e && codepoint <= 0x1e1f) {
		return orp_block20[codepoint - 0x1e1e];
	} else if (codepoint >= 0x1e30 && codepoint <= 0x1e35) {
		return orp_block21[codepoint - 0x1e30];
	} else if (codepoint >= 0x1e3e && codepoint <= 0x1e43) {
		return orp_block22[codepoint - 0x1e3e];
	} else if (codepoint >= 0x1e54 && codepoint <= 0x1e57) {
		return orp_block23[codepoint - 0x1e54];
	} else if (codepoint >= 0x1e60 && codepoint <= 0x1e71) {
		return orp_block24[codepoint - 0x1e60];
	} else if (codepoint >= 0x1e80 && codepoint <= 0x1e8f) {
		return orp_block25[codepoint - 0x1e80];
	} else if (codepoint >= 0x1ef2 && codepoint <= 0x1ef9) {
		return orp_block26[codepoint - 0x1ef2];
	} else if (codepoint >= 0x1f00 && codepoint <= 0x1f15) {
		return orp_block27[codepoint - 0x1f00];
	} else if (codepoint >= 0x1f18 && codepoint <= 0x1f1d) {
		return orp_block28[codepoint - 0x1f18];
	} else if (codepoint >= 0x1f20 && codepoint <= 0x1f45) {
		return orp_block29[codepoint - 0x1f20];
	} else if (codepoint >= 0x1f48 && codepoint <= 0x1f4d) {
		return orp_block30[codepoint - 0x1f48];
	} else if (codepoint >= 0x1f50 && codepoint <= 0x1f57) {
		return orp_block31[codepoint - 0x1f50];
	} else if (codepoint == 0x1f59) {
		return orp_block32[0];
	} else if (codepoint == 0x1f5b) {
		return orp_block33[0];
	} else if (codepoint == 0x1f5d) {
		return orp_block34[0];
	} else if (codepoint >= 0x1f5f && codepoint <= 0x1f7d) {
		return orp_block35[codepoint - 0x1f5f];
	} else if (codepoint >= 0x1f80 && codepoint <= 0x1fb4) {
		return orp_block36[codepoint - 0x1f80];
	} else if (codepoint >= 0x1fb6 && codepoint <= 0x1fc4) {
		return orp_block37[codepoint - 0x1fb6];
	} else if (codepoint >= 0x1fc6 && codepoint <= 0x1fd3) {
		return orp_block38[codepoint - 0x1fc6];
	} else if (codepoint >= 0x1fd6 && codepoint <= 0x1fdb) {
		return orp_block39[codepoint - 0x1fd6];
	} else if (codepoint >= 0x1fdd && codepoint <= 0x1fef) {
		return orp_block40[codepoint - 0x1fdd];
	} else if (codepoint >= 0x1ff2 && codepoint <= 0x1ff4) {
		return orp_block41[codepoint - 0x1ff2];
	} else if (codepoint >= 0x1ff6 && codepoint <= 0x1ffe) {
		return orp_block42[codepoint - 0x1ff6];
	} else if (codepoint >= 0x2010 && codepoint <= 0x2027) {
		return orp_block43[codepoint - 0x2010];
	} else if (codepoint >= 0x2030 && codepoint <= 0x205e) {
		return orp_block44[codepoint - 0x2030];
	} else if (codepoint >= 0x2070 && codepoint <= 0x2071) {
		return orp_block45[codepoint - 0x2070];
	} else if (codepoint >= 0x2074 && codepoint <= 0x208e) {
		return orp_block46[codepoint - 0x2074];
	} else if (codepoint >= 0x2090 && codepoint <= 0x2094) {
		return orp_block47[codepoint - 0x2090];
	} else if (codepoint >= 0x20a0 && codepoint <= 0x20b5) {
		return orp_block48[codepoint - 0x20a0];
	} else if (codepoint >= 0x20d0 && codepoint <= 0x20f0) {
		return orp_block49[codepoint - 0x20d0];
	} else if (codepoint >= 0x2100 && codepoint <= 0x214f) {
		return orp_block50[codepoint - 0x2100];
	} else if (codepoint >= 0x2153 && codepoint <= 0x2188) {
		return orp_block51[codepoint - 0x2153];
	} else if (codepoint >= 0x2190 && codepoint <= 0x2328) {
		return orp_block52[codepoint - 0x2190];
	} else if (codepoint >= 0x232b && codepoint <= 0x23e7) {
		return orp_block53[codepoint - 0x232b];
	} else if (codepoint >= 0x2400 && codepoint <= 0x2426) {
		return orp_block54[codepoint - 0x2400];
	} else if (codepoint >= 0x2440 && codepoint <= 0x244a) {
		return orp_block55[codepoint - 0x2440];
	} else if (codepoint >= 0x2460 && codepoint <= 0x2613) {
		return orp_block56[codepoint - 0x2460];
	} else if (codepoint >= 0x2616 && codepoint <= 0x2617) {
		return orp_block57[codepoint - 0x2616];
	} else if (codepoint >= 0x2619 && codepoint <= 0x269c) {
		return orp_block58[codepoint - 0x2619];
	} else if (codepoint >= 0x26a0 && codepoint <= 0x26bc) {
		return orp_block59[codepoint - 0x26a0];
	} else if (codepoint >= 0x26c0 && codepoint <= 0x26c3) {
		return orp_block60[codepoint - 0x26c0];
	} else if (codepoint >= 0x2701 && codepoint <= 0x2704) {
		return orp_block61[codepoint - 0x2701];
	} else if (codepoint >= 0x2706 && codepoint <= 0x2709) {
		return orp_block62[codepoint - 0x2706];
	} else if (codepoint >= 0x270c && codepoint <= 0x2727) {
		return orp_block63[codepoint - 0x270c];
	} else if (codepoint >= 0x2729 && codepoint <= 0x274b) {
		return orp_block64[codepoint - 0x2729];
	} else if (codepoint == 0x274d) {
		return orp_block65[0];
	} else if (codepoint >= 0x274f && codepoint <= 0x2752) {
		return orp_block66[codepoint - 0x274f];
	} else if (codepoint == 0x2756) {
		return orp_block67[0];
	} else if (codepoint >= 0x2758 && codepoint <= 0x275e) {
		return orp_block68[codepoint - 0x2758];
	} else if (codepoint >= 0x2761 && codepoint <= 0x2794) {
		return orp_block69[codepoint - 0x2761];
	} else if (codepoint >= 0x2798 && codepoint <= 0x27af) {
		return orp_block70[codepoint - 0x2798];
	} else if (codepoint >= 0x27b1 && codepoint <= 0x27be) {
		return orp_block71[codepoint - 0x27b1];
	} else if (codepoint >= 0x27c0 && codepoint <= 0x27ca) {
		return orp_block72[codepoint - 0x27c0];
	} else if (codepoint == 0x27cc) {
		return orp_block73[0];
	} else if (codepoint >= 0x27d0 && codepoint <= 0x2b4c) {
		return orp_block74[codepoint - 0x27d0];
	} else if (codepoint >= 0x2b60 && codepoint <= 0x2b64) {
		return orp_block75[codepoint - 0x2b60];
	} else if (codepoint >= 0x2b80 && codepoint <= 0x2b83) {
		return orp_block76[codepoint - 0x2b80];
	} else if (codepoint >= 0x2c60 && codepoint <= 0x2c6f) {
		return orp_block77[codepoint - 0x2c60];
	} else if (codepoint >= 0x2c71 && codepoint <= 0x2c7d) {
		return orp_block78[codepoint - 0x2c71];
	} else if (codepoint >= 0x2de0 && codepoint <= 0x2dff) {
		return orp_block79[codepoint - 0x2de0];
	} else if (codepoint == 0xe0b2) {
		return orp_block80[0];
	} else if (codepoint >= 0xfb00 && codepoint <= 0xfb06) {
		return orp_block81[codepoint - 0xfb00];
	} else if (codepoint >= 0xfe20 && codepoint <= 0xfe23) {
		return orp_block82[codepoint - 0xfe20];
	} else if (codepoint == 0xfffd) {
		return orp_block83[0];
	} else {
		WARN("Codepoint U+%04X is not covered by this font", codepoint);
		return orp_block1[0];
	}
}

static const unsigned char*
    orpb_get_bitmap(uint32_t codepoint)
{
	if (codepoint <= 0x00) {
		return orpb_block1[codepoint];
	} else if (codepoint >= 0x20 && codepoint <= 0x7e) {
		return orpb_block2[codepoint - 0x20];
	} else if (codepoint >= 0xa0 && codepoint <= 0x377) {
		return orpb_block3[codepoint - 0xa0];
	} else if (codepoint >= 0x37a && codepoint <= 0x37e) {
		return orpb_block4[codepoint - 0x37a];
	} else if (codepoint >= 0x384 && codepoint <= 0x38a) {
		return orpb_block5[codepoint - 0x384];
	} else if (codepoint == 0x38c) {
		return orpb_block6[0];
	} else if (codepoint >= 0x38e && codepoint <= 0x3a1) {
		return orpb_block7[codepoint - 0x38e];
	} else if (codepoint >= 0x3a3 && codepoint <= 0x523) {
		return orpb_block8[codepoint - 0x3a3];
	} else if (codepoint >= 0x531 && codepoint <= 0x556) {
		return orpb_block9[codepoint - 0x531];
	} else if (codepoint >= 0x559 && codepoint <= 0x55f) {
		return orpb_block10[codepoint - 0x559];
	} else if (codepoint >= 0x561 && codepoint <= 0x587) {
		return orpb_block11[codepoint - 0x561];
	} else if (codepoint >= 0x589 && codepoint <= 0x58a) {
		return orpb_block12[codepoint - 0x589];
	} else if (codepoint >= 0x591 && codepoint <= 0x5c7) {
		return orpb_block13[codepoint - 0x591];
	} else if (codepoint >= 0x5d0 && codepoint <= 0x5ea) {
		return orpb_block14[codepoint - 0x5d0];
	} else if (codepoint >= 0x5f0 && codepoint <= 0x5f4) {
		return orpb_block15[codepoint - 0x5f0];
	} else if (codepoint >= 0x1680 && codepoint <= 0x169c) {
		return orpb_block16[codepoint - 0x1680];
	} else if (codepoint >= 0x16a0 && codepoint <= 0x16f0) {
		return orpb_block17[codepoint - 0x16a0];
	} else if (codepoint >= 0x1e02 && codepoint <= 0x1e07) {
		return orpb_block18[codepoint - 0x1e02];
	} else if (codepoint >= 0x1e0a && codepoint <= 0x1e13) {
		return orpb_block19[codepoint - 0x1e0a];
	} else if (codepoint >= 0x1e1e && codepoint <= 0x1e1f) {
		return orpb_block20[codepoint - 0x1e1e];
	} else if (codepoint >= 0x1e30 && codepoint <= 0x1e35) {
		return orpb_block21[codepoint - 0x1e30];
	} else if (codepoint >= 0x1e3e && codepoint <= 0x1e43) {
		return orpb_block22[codepoint - 0x1e3e];
	} else if (codepoint >= 0x1e54 && codepoint <= 0x1e57) {
		return orpb_block23[codepoint - 0x1e54];
	} else if (codepoint >= 0x1e60 && codepoint <= 0x1e71) {
		return orpb_block24[codepoint - 0x1e60];
	} else if (codepoint >= 0x1e80 && codepoint <= 0x1e8f) {
		return orpb_block25[codepoint - 0x1e80];
	} else if (codepoint >= 0x1ef2 && codepoint <= 0x1ef9) {
		return orpb_block26[codepoint - 0x1ef2];
	} else if (codepoint >= 0x1f00 && codepoint <= 0x1f15) {
		return orpb_block27[codepoint - 0x1f00];
	} else if (codepoint >= 0x1f18 && codepoint <= 0x1f1d) {
		return orpb_block28[codepoint - 0x1f18];
	} else if (codepoint >= 0x1f20 && codepoint <= 0x1f45) {
		return orpb_block29[codepoint - 0x1f20];
	} else if (codepoint >= 0x1f48 && codepoint <= 0x1f4d) {
		return orpb_block30[codepoint - 0x1f48];
	} else if (codepoint >= 0x1f50 && codepoint <= 0x1f57) {
		return orpb_block31[codepoint - 0x1f50];
	} else if (codepoint == 0x1f59) {
		return orpb_block32[0];
	} else if (codepoint == 0x1f5b) {
		return orpb_block33[0];
	} else if (codepoint == 0x1f5d) {
		return orpb_block34[0];
	} else if (codepoint >= 0x1f5f && codepoint <= 0x1f7d) {
		return orpb_block35[codepoint - 0x1f5f];
	} else if (codepoint >= 0x1f80 && codepoint <= 0x1fb4) {
		return orpb_block36[codepoint - 0x1f80];
	} else if (codepoint >= 0x1fb6 && codepoint <= 0x1fc4) {
		return orpb_block37[codepoint - 0x1fb6];
	} else if (codepoint >= 0x1fc6 && codepoint <= 0x1fd3) {
		return orpb_block38[codepoint - 0x1fc6];
	} else if (codepoint >= 0x1fd6 && codepoint <= 0x1fdb) {
		return orpb_block39[codepoint - 0x1fd6];
	} else if (codepoint >= 0x1fdd && codepoint <= 0x1fef) {
		return orpb_block40[codepoint - 0x1fdd];
	} else if (codepoint >= 0x1ff2 && codepoint <= 0x1ff4) {
		return orpb_block41[codepoint - 0x1ff2];
	} else if (codepoint >= 0x1ff6 && codepoint <= 0x1ffe) {
		return orpb_block42[codepoint - 0x1ff6];
	} else if (codepoint >= 0x2010 && codepoint <= 0x2027) {
		return orpb_block43[codepoint - 0x2010];
	} else if (codepoint >= 0x2030 && codepoint <= 0x205e) {
		return orpb_block44[codepoint - 0x2030];
	} else if (codepoint >= 0x2070 && codepoint <= 0x2071) {
		return orpb_block45[codepoint - 0x2070];
	} else if (codepoint >= 0x2074 && codepoint <= 0x208e) {
		return orpb_block46[codepoint - 0x2074];
	} else if (codepoint >= 0x2090 && codepoint <= 0x2094) {
		return orpb_block47[codepoint - 0x2090];
	} else if (codepoint >= 0x20a0 && codepoint <= 0x20b5) {
		return orpb_block48[codepoint - 0x20a0];
	} else if (codepoint >= 0x20d0 && codepoint <= 0x20f0) {
		return orpb_block49[codepoint - 0x20d0];
	} else if (codepoint >= 0x2100 && codepoint <= 0x214f) {
		return orpb_block50[codepoint - 0x2100];
	} else if (codepoint >= 0x2153 && codepoint <= 0x2188) {
		return orpb_block51[codepoint - 0x2153];
	} else if (codepoint >= 0x2190 && codepoint <= 0x2328) {
		return orpb_block52[codepoint - 0x2190];
	} else if (codepoint >= 0x232b && codepoint <= 0x23e7) {
		return orpb_block53[codepoint - 0x232b];
	} else if (codepoint >= 0x2400 && codepoint <= 0x2426) {
		return orpb_block54[codepoint - 0x2400];
	} else if (codepoint >= 0x2440 && codepoint <= 0x244a) {
		return orpb_block55[codepoint - 0x2440];
	} else if (codepoint >= 0x2460 && codepoint <= 0x2613) {
		return orpb_block56[codepoint - 0x2460];
	} else if (codepoint >= 0x2616 && codepoint <= 0x2617) {
		return orpb_block57[codepoint - 0x2616];
	} else if (codepoint >= 0x2619 && codepoint <= 0x269c) {
		return orpb_block58[codepoint - 0x2619];
	} else if (codepoint >= 0x26a0 && codepoint <= 0x26bc) {
		return orpb_block59[codepoint - 0x26a0];
	} else if (codepoint >= 0x26c0 && codepoint <= 0x26c3) {
		return orpb_block60[codepoint - 0x26c0];
	} else if (codepoint >= 0x2701 && codepoint <= 0x2704) {
		return orpb_block61[codepoint - 0x2701];
	} else if (codepoint >= 0x2706 && codepoint <= 0x2709) {
		return orpb_block62[codepoint - 0x2706];
	} else if (codepoint >= 0x270c && codepoint <= 0x2727) {
		return orpb_block63[codepoint - 0x270c];
	} else if (codepoint >= 0x2729 && codepoint <= 0x274b) {
		return orpb_block64[codepoint - 0x2729];
	} else if (codepoint == 0x274d) {
		return orpb_block65[0];
	} else if (codepoint >= 0x274f && codepoint <= 0x2752) {
		return orpb_block66[codepoint - 0x274f];
	} else if (codepoint == 0x2756) {
		return orpb_block67[0];
	} else if (codepoint >= 0x2758 && codepoint <= 0x275e) {
		return orpb_block68[codepoint - 0x2758];
	} else if (codepoint >= 0x2761 && codepoint <= 0x2794) {
		return orpb_block69[codepoint - 0x2761];
	} else if (codepoint >= 0x2798 && codepoint <= 0x27af) {
		return orpb_block70[codepoint - 0x2798];
	} else if (codepoint >= 0x27b1 && codepoint <= 0x27be) {
		return orpb_block71[codepoint - 0x27b1];
	} else if (codepoint >= 0x27c0 && codepoint <= 0x27ca) {
		return orpb_block72[codepoint - 0x27c0];
	} else if (codepoint == 0x27cc) {
		return orpb_block73[0];
	} else if (codepoint >= 0x27d0 && codepoint <= 0x2b4c) {
		return orpb_block74[codepoint - 0x27d0];
	} else if (codepoint >= 0x2c60 && codepoint <= 0x2c6f) {
		return orpb_block75[codepoint - 0x2c60];
	} else if (codepoint >= 0x2c71 && codepoint <= 0x2c7d) {
		return orpb_block76[codepoint - 0x2c71];
	} else if (codepoint >= 0x2de0 && codepoint <= 0x2dff) {
		return orpb_block77[codepoint - 0x2de0];
	} else if (codepoint >= 0xfb00 && codepoint <= 0xfb06) {
		return orpb_block78[codepoint - 0xfb00];
	} else if (codepoint >= 0xfe20 && codepoint <= 0xfe23) {
		return orpb_block79[codepoint - 0xfe20];
	} else if (codepoint == 0xfffd) {
		return orpb_block80[0];
	} else {
		WARN("Codepoint U+%04X is not covered by this font", codepoint);
		return orpb_block1[0];
	}
}

static const unsigned char*
    orpi_get_bitmap(uint32_t codepoint)
{
	if (codepoint <= 0x00) {
		return orpi_block1[codepoint];
	} else if (codepoint >= 0x20 && codepoint <= 0x7e) {
		return orpi_block2[codepoint - 0x20];
	} else if (codepoint >= 0xa0 && codepoint <= 0x377) {
		return orpi_block3[codepoint - 0xa0];
	} else if (codepoint >= 0x37a && codepoint <= 0x37e) {
		return orpi_block4[codepoint - 0x37a];
	} else if (codepoint >= 0x384 && codepoint <= 0x38a) {
		return orpi_block5[codepoint - 0x384];
	} else if (codepoint == 0x38c) {
		return orpi_block6[0];
	} else if (codepoint >= 0x38e && codepoint <= 0x3a1) {
		return orpi_block7[codepoint - 0x38e];
	} else if (codepoint >= 0x3a3 && codepoint <= 0x523) {
		return orpi_block8[codepoint - 0x3a3];
	} else if (codepoint >= 0x531 && codepoint <= 0x556) {
		return orpi_block9[codepoint - 0x531];
	} else if (codepoint >= 0x559 && codepoint <= 0x55f) {
		return orpi_block10[codepoint - 0x559];
	} else if (codepoint >= 0x561 && codepoint <= 0x587) {
		return orpi_block11[codepoint - 0x561];
	} else if (codepoint >= 0x589 && codepoint <= 0x58a) {
		return orpi_block12[codepoint - 0x589];
	} else if (codepoint >= 0x591 && codepoint <= 0x5c7) {
		return orpi_block13[codepoint - 0x591];
	} else if (codepoint >= 0x5d0 && codepoint <= 0x5ea) {
		return orpi_block14[codepoint - 0x5d0];
	} else if (codepoint >= 0x5f0 && codepoint <= 0x5f4) {
		return orpi_block15[codepoint - 0x5f0];
	} else if (codepoint >= 0x1680 && codepoint <= 0x169c) {
		return orpi_block16[codepoint - 0x1680];
	} else if (codepoint >= 0x16a0 && codepoint <= 0x16f0) {
		return orpi_block17[codepoint - 0x16a0];
	} else if (codepoint >= 0x1e02 && codepoint <= 0x1e07) {
		return orpi_block18[codepoint - 0x1e02];
	} else if (codepoint >= 0x1e0a && codepoint <= 0x1e13) {
		return orpi_block19[codepoint - 0x1e0a];
	} else if (codepoint >= 0x1e1e && codepoint <= 0x1e1f) {
		return orpi_block20[codepoint - 0x1e1e];
	} else if (codepoint >= 0x1e30 && codepoint <= 0x1e35) {
		return orpi_block21[codepoint - 0x1e30];
	} else if (codepoint >= 0x1e3e && codepoint <= 0x1e43) {
		return orpi_block22[codepoint - 0x1e3e];
	} else if (codepoint >= 0x1e54 && codepoint <= 0x1e57) {
		return orpi_block23[codepoint - 0x1e54];
	} else if (codepoint >= 0x1e60 && codepoint <= 0x1e71) {
		return orpi_block24[codepoint - 0x1e60];
	} else if (codepoint >= 0x1e80 && codepoint <= 0x1e8f) {
		return orpi_block25[codepoint - 0x1e80];
	} else if (codepoint >= 0x1ef2 && codepoint <= 0x1ef9) {
		return orpi_block26[codepoint - 0x1ef2];
	} else if (codepoint >= 0x1f00 && codepoint <= 0x1f15) {
		return orpi_block27[codepoint - 0x1f00];
	} else if (codepoint >= 0x1f18 && codepoint <= 0x1f1d) {
		return orpi_block28[codepoint - 0x1f18];
	} else if (codepoint >= 0x1f20 && codepoint <= 0x1f45) {
		return orpi_block29[codepoint - 0x1f20];
	} else if (codepoint >= 0x1f48 && codepoint <= 0x1f4d) {
		return orpi_block30[codepoint - 0x1f48];
	} else if (codepoint >= 0x1f50 && codepoint <= 0x1f57) {
		return orpi_block31[codepoint - 0x1f50];
	} else if (codepoint == 0x1f59) {
		return orpi_block32[0];
	} else if (codepoint == 0x1f5b) {
		return orpi_block33[0];
	} else if (codepoint == 0x1f5d) {
		return orpi_block34[0];
	} else if (codepoint >= 0x1f5f && codepoint <= 0x1f7d) {
		return orpi_block35[codepoint - 0x1f5f];
	} else if (codepoint >= 0x1f80 && codepoint <= 0x1fb4) {
		return orpi_block36[codepoint - 0x1f80];
	} else if (codepoint >= 0x1fb6 && codepoint <= 0x1fc4) {
		return orpi_block37[codepoint - 0x1fb6];
	} else if (codepoint >= 0x1fc6 && codepoint <= 0x1fd3) {
		return orpi_block38[codepoint - 0x1fc6];
	} else if (codepoint >= 0x1fd6 && codepoint <= 0x1fdb) {
		return orpi_block39[codepoint - 0x1fd6];
	} else if (codepoint >= 0x1fdd && codepoint <= 0x1fef) {
		return orpi_block40[codepoint - 0x1fdd];
	} else if (codepoint >= 0x1ff2 && codepoint <= 0x1ff4) {
		return orpi_block41[codepoint - 0x1ff2];
	} else if (codepoint >= 0x1ff6 && codepoint <= 0x1ffe) {
		return orpi_block42[codepoint - 0x1ff6];
	} else if (codepoint >= 0x2010 && codepoint <= 0x2027) {
		return orpi_block43[codepoint - 0x2010];
	} else if (codepoint >= 0x2030 && codepoint <= 0x205e) {
		return orpi_block44[codepoint - 0x2030];
	} else if (codepoint >= 0x2070 && codepoint <= 0x2071) {
		return orpi_block45[codepoint - 0x2070];
	} else if (codepoint >= 0x2074 && codepoint <= 0x208e) {
		return orpi_block46[codepoint - 0x2074];
	} else if (codepoint >= 0x2090 && codepoint <= 0x2094) {
		return orpi_block47[codepoint - 0x2090];
	} else if (codepoint >= 0x20a0 && codepoint <= 0x20b5) {
		return orpi_block48[codepoint - 0x20a0];
	} else if (codepoint >= 0x20d0 && codepoint <= 0x20f0) {
		return orpi_block49[codepoint - 0x20d0];
	} else if (codepoint >= 0x2100 && codepoint <= 0x214f) {
		return orpi_block50[codepoint - 0x2100];
	} else if (codepoint >= 0x2153 && codepoint <= 0x2188) {
		return orpi_block51[codepoint - 0x2153];
	} else if (codepoint >= 0x2190 && codepoint <= 0x2328) {
		return orpi_block52[codepoint - 0x2190];
	} else if (codepoint >= 0x232b && codepoint <= 0x23e7) {
		return orpi_block53[codepoint - 0x232b];
	} else if (codepoint >= 0x2400 && codepoint <= 0x2426) {
		return orpi_block54[codepoint - 0x2400];
	} else if (codepoint >= 0x2440 && codepoint <= 0x244a) {
		return orpi_block55[codepoint - 0x2440];
	} else if (codepoint >= 0x2460 && codepoint <= 0x2613) {
		return orpi_block56[codepoint - 0x2460];
	} else if (codepoint >= 0x2616 && codepoint <= 0x2617) {
		return orpi_block57[codepoint - 0x2616];
	} else if (codepoint >= 0x2619 && codepoint <= 0x269c) {
		return orpi_block58[codepoint - 0x2619];
	} else if (codepoint >= 0x26a0 && codepoint <= 0x26bc) {
		return orpi_block59[codepoint - 0x26a0];
	} else if (codepoint >= 0x26c0 && codepoint <= 0x26c3) {
		return orpi_block60[codepoint - 0x26c0];
	} else if (codepoint >= 0x2701 && codepoint <= 0x2704) {
		return orpi_block61[codepoint - 0x2701];
	} else if (codepoint >= 0x2706 && codepoint <= 0x2709) {
		return orpi_block62[codepoint - 0x2706];
	} else if (codepoint >= 0x270c && codepoint <= 0x2727) {
		return orpi_block63[codepoint - 0x270c];
	} else if (codepoint >= 0x2729 && codepoint <= 0x274b) {
		return orpi_block64[codepoint - 0x2729];
	} else if (codepoint == 0x274d) {
		return orpi_block65[0];
	} else if (codepoint >= 0x274f && codepoint <= 0x2752) {
		return orpi_block66[codepoint - 0x274f];
	} else if (codepoint == 0x2756) {
		return orpi_block67[0];
	} else if (codepoint >= 0x2758 && codepoint <= 0x275e) {
		return orpi_block68[codepoint - 0x2758];
	} else if (codepoint >= 0x2761 && codepoint <= 0x2794) {
		return orpi_block69[codepoint - 0x2761];
	} else if (codepoint >= 0x2798 && codepoint <= 0x27af) {
		return orpi_block70[codepoint - 0x2798];
	} else if (codepoint >= 0x27b1 && codepoint <= 0x27be) {
		return orpi_block71[codepoint - 0x27b1];
	} else if (codepoint >= 0x27c0 && codepoint <= 0x27ca) {
		return orpi_block72[codepoint - 0x27c0];
	} else if (codepoint == 0x27cc) {
		return orpi_block73[0];
	} else if (codepoint >= 0x27d0 && codepoint <= 0x2b4c) {
		return orpi_block74[codepoint - 0x27d0];
	} else if (codepoint >= 0x2c60 && codepoint <= 0x2c6f) {
		return orpi_block75[codepoint - 0x2c60];
	} else if (codepoint >= 0x2c71 && codepoint <= 0x2c7d) {
		return orpi_block76[codepoint - 0x2c71];
	} else if (codepoint >= 0x2de0 && codepoint <= 0x2dff) {
		return orpi_block77[codepoint - 0x2de0];
	} else if (codepoint >= 0xfb00 && codepoint <= 0xfb06) {
		return orpi_block78[codepoint - 0xfb00];
	} else if (codepoint >= 0xfe20 && codepoint <= 0xfe23) {
		return orpi_block79[codepoint - 0xfe20];
	} else if (codepoint == 0xfffd) {
		return orpi_block80[0];
	} else {
		WARN("Codepoint U+%04X is not covered by this font", codepoint);
		return orpi_block1[0];
	}
}
