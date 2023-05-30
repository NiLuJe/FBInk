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

#include "fbink_spleen.h"

static const uint16_t*
    spleen_get_bitmap(uint32_t codepoint)
{
	if (codepoint <= 0x7fu) {
		return spleen_block1[codepoint];
	} else if (codepoint >= 0xa0u && codepoint <= 0x17fu) {
		return spleen_block2[codepoint - 0xa0u];
	} else if (codepoint == 0x192u) {
		return spleen_block3[0];
	} else if (codepoint >= 0x1cdu && codepoint <= 0x1d4u) {
		return spleen_block4[codepoint - 0x1cdu];
	} else if (codepoint >= 0x1e2u && codepoint <= 0x1e3u) {
		return spleen_block5[codepoint - 0x1e2u];
	} else if (codepoint >= 0x1e6u && codepoint <= 0x1edu) {
		return spleen_block6[codepoint - 0x1e6u];
	} else if (codepoint == 0x1f0u) {
		return spleen_block7[0];
	} else if (codepoint >= 0x1f4u && codepoint <= 0x1f5u) {
		return spleen_block8[codepoint - 0x1f4u];
	} else if (codepoint >= 0x1fcu && codepoint <= 0x217u) {
		return spleen_block9[codepoint - 0x1fcu];
	} else if (codepoint >= 0x21eu && codepoint <= 0x21fu) {
		return spleen_block10[codepoint - 0x21eu];
	} else if (codepoint >= 0x226u && codepoint <= 0x229u) {
		return spleen_block11[codepoint - 0x226u];
	} else if (codepoint >= 0x22eu && codepoint <= 0x22fu) {
		return spleen_block12[codepoint - 0x22eu];
	} else if (codepoint >= 0x2d8u && codepoint <= 0x2d9u) {
		return spleen_block13[codepoint - 0x2d8u];
	} else if (codepoint >= 0x2dbu && codepoint <= 0x2ddu) {
		return spleen_block14[codepoint - 0x2dbu];
	} else if (codepoint == 0x393u) {
		return spleen_block15[0];
	} else if (codepoint == 0x398u) {
		return spleen_block16[0];
	} else if (codepoint == 0x3a3u) {
		return spleen_block17[0];
	} else if (codepoint == 0x3a6u) {
		return spleen_block18[0];
	} else if (codepoint == 0x3a9u) {
		return spleen_block19[0];
	} else if (codepoint == 0x3b1u) {
		return spleen_block20[0];
	} else if (codepoint >= 0x3b4u && codepoint <= 0x3b5u) {
		return spleen_block21[codepoint - 0x3b4u];
	} else if (codepoint == 0x3c0u) {
		return spleen_block22[0];
	} else if (codepoint >= 0x3c3u && codepoint <= 0x3c4u) {
		return spleen_block23[codepoint - 0x3c3u];
	} else if (codepoint == 0x3c6u) {
		return spleen_block24[0];
	} else if (codepoint == 0x2016u) {
		return spleen_block25[0];
	} else if (codepoint >= 0x2018u && codepoint <= 0x2019u) {
		return spleen_block26[codepoint - 0x2018u];
	} else if (codepoint >= 0x201cu && codepoint <= 0x201du) {
		return spleen_block27[codepoint - 0x201cu];
	} else if (codepoint == 0x2022u) {
		return spleen_block28[0];
	} else if (codepoint == 0x2026u) {
		return spleen_block29[0];
	} else if (codepoint >= 0x2039u && codepoint <= 0x203au) {
		return spleen_block30[codepoint - 0x2039u];
	} else if (codepoint == 0x203cu) {
		return spleen_block31[0];
	} else if (codepoint == 0x207fu) {
		return spleen_block32[0];
	} else if (codepoint == 0x20a7u) {
		return spleen_block33[0];
	} else if (codepoint == 0x20acu) {
		return spleen_block34[0];
	} else if (codepoint >= 0x2190u && codepoint <= 0x2195u) {
		return spleen_block35[codepoint - 0x2190u];
	} else if (codepoint == 0x21a8u) {
		return spleen_block36[0];
	} else if (codepoint >= 0x2219u && codepoint <= 0x221au) {
		return spleen_block37[codepoint - 0x2219u];
	} else if (codepoint == 0x221eu) {
		return spleen_block38[0];
	} else if (codepoint >= 0x2229u && codepoint <= 0x222au) {
		return spleen_block39[codepoint - 0x2229u];
	} else if (codepoint == 0x2248u) {
		return spleen_block40[0];
	} else if (codepoint == 0x2261u) {
		return spleen_block41[0];
	} else if (codepoint >= 0x2264u && codepoint <= 0x2265u) {
		return spleen_block42[codepoint - 0x2264u];
	} else if (codepoint == 0x2302u) {
		return spleen_block43[0];
	} else if (codepoint == 0x2310u) {
		return spleen_block44[0];
	} else if (codepoint == 0x2319u) {
		return spleen_block45[0];
	} else if (codepoint >= 0x2320u && codepoint <= 0x2321u) {
		return spleen_block46[codepoint - 0x2320u];
	} else if (codepoint >= 0x2500u && codepoint <= 0x25a0u) {
		return spleen_block47[codepoint - 0x2500u];
	} else if (codepoint == 0x25acu) {
		return spleen_block48[0];
	} else if (codepoint == 0x25b2u) {
		return spleen_block49[0];
	} else if (codepoint == 0x25bcu) {
		return spleen_block50[0];
	} else if (codepoint == 0x25c6u) {
		return spleen_block51[0];
	} else if (codepoint >= 0x25cau && codepoint <= 0x25cbu) {
		return spleen_block52[codepoint - 0x25cau];
	} else if (codepoint == 0x25cfu) {
		return spleen_block53[0];
	} else if (codepoint >= 0x25d8u && codepoint <= 0x25d9u) {
		return spleen_block54[codepoint - 0x25d8u];
	} else if (codepoint >= 0x25e2u && codepoint <= 0x25e5u) {
		return spleen_block55[codepoint - 0x25e2u];
	} else if (codepoint >= 0x2630u && codepoint <= 0x2637u) {
		return spleen_block56[codepoint - 0x2630u];
	} else if (codepoint >= 0x263au && codepoint <= 0x263cu) {
		return spleen_block57[codepoint - 0x263au];
	} else if (codepoint == 0x2640u) {
		return spleen_block58[0];
	} else if (codepoint == 0x2642u) {
		return spleen_block59[0];
	} else if (codepoint == 0x2660u) {
		return spleen_block60[0];
	} else if (codepoint == 0x2663u) {
		return spleen_block61[0];
	} else if (codepoint >= 0x2665u && codepoint <= 0x2666u) {
		return spleen_block62[codepoint - 0x2665u];
	} else if (codepoint >= 0x266au && codepoint <= 0x266bu) {
		return spleen_block63[codepoint - 0x266au];
	} else if (codepoint >= 0x2800u && codepoint <= 0x28ffu) {
		return spleen_block64[codepoint - 0x2800u];
	} else if (codepoint >= 0x2b06u && codepoint <= 0x2b07u) {
		return spleen_block65[codepoint - 0x2b06u];
	} else if (codepoint >= 0x2b60u && codepoint <= 0x2b65u) {
		return spleen_block66[codepoint - 0x2b60u];
	} else if (codepoint >= 0xe0a0u && codepoint <= 0xe0a2u) {
		return spleen_block67[codepoint - 0xe0a0u];
	} else if (codepoint >= 0xe0b0u && codepoint <= 0xe0b3u) {
		return spleen_block68[codepoint - 0xe0b0u];
	} else {
		WARN("Codepoint U+%04X (%s) is not covered by this font", codepoint, u8_cp_to_utf8(codepoint));
		return spleen_block1[0];
	}
}
