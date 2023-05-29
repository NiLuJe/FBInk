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

#include "fbink_vga.h"

static const unsigned char*
    vga_get_bitmap(uint32_t codepoint)
{
	if (codepoint <= 0x7fu) {
		return vga_block1[codepoint];
	} else if (codepoint >= 0xa0u && codepoint <= 0x17fu) {
		return vga_block2[codepoint - 0xa0u];
	} else if (codepoint == 0x192u) {
		return vga_block3[0];
	} else if (codepoint == 0x1a1u) {
		return vga_block4[0];
	} else if (codepoint == 0x1b7u) {
		return vga_block5[0];
	} else if (codepoint >= 0x1fau && codepoint <= 0x1ffu) {
		return vga_block6[codepoint - 0x1fau];
	} else if (codepoint >= 0x218u && codepoint <= 0x21bu) {
		return vga_block7[codepoint - 0x218u];
	} else if (codepoint == 0x251u) {
		return vga_block8[0];
	} else if (codepoint == 0x278u) {
		return vga_block9[0];
	} else if (codepoint >= 0x2c6u && codepoint <= 0x2c7u) {
		return vga_block10[codepoint - 0x2c6u];
	} else if (codepoint == 0x2c9u) {
		return vga_block11[0];
	} else if (codepoint >= 0x2d8u && codepoint <= 0x2ddu) {
		return vga_block12[codepoint - 0x2d8u];
	} else if (codepoint == 0x37eu) {
		return vga_block13[0];
	} else if (codepoint >= 0x384u && codepoint <= 0x38au) {
		return vga_block14[codepoint - 0x384u];
	} else if (codepoint == 0x38cu) {
		return vga_block15[0];
	} else if (codepoint >= 0x38eu && codepoint <= 0x3a1u) {
		return vga_block16[codepoint - 0x38eu];
	} else if (codepoint >= 0x3a3u && codepoint <= 0x3ceu) {
		return vga_block17[codepoint - 0x3a3u];
	} else if (codepoint == 0x3d0u) {
		return vga_block18[0];
	} else if (codepoint == 0x3f4u) {
		return vga_block19[0];
	} else if (codepoint >= 0x400u && codepoint <= 0x45fu) {
		return vga_block20[codepoint - 0x400u];
	} else if (codepoint >= 0x490u && codepoint <= 0x491u) {
		return vga_block21[codepoint - 0x490u];
	} else if (codepoint == 0x5beu) {
		return vga_block22[0];
	} else if (codepoint >= 0x5d0u && codepoint <= 0x5eau) {
		return vga_block23[codepoint - 0x5d0u];
	} else if (codepoint >= 0x5f0u && codepoint <= 0x5f4u) {
		return vga_block24[codepoint - 0x5f0u];
	} else if (codepoint == 0x1d1bu) {
		return vga_block25[0];
	} else if (codepoint == 0x1d26u) {
		return vga_block26[0];
	} else if (codepoint == 0x1d28u) {
		return vga_block27[0];
	} else if (codepoint >= 0x1e80u && codepoint <= 0x1e85u) {
		return vga_block28[codepoint - 0x1e80u];
	} else if (codepoint == 0x1e9fu) {
		return vga_block29[0];
	} else if (codepoint >= 0x1ef2u && codepoint <= 0x1ef3u) {
		return vga_block30[codepoint - 0x1ef2u];
	} else if (codepoint == 0x2010u) {
		return vga_block31[0];
	} else if (codepoint >= 0x2012u && codepoint <= 0x2015u) {
		return vga_block32[codepoint - 0x2012u];
	} else if (codepoint >= 0x2017u && codepoint <= 0x2022u) {
		return vga_block33[codepoint - 0x2017u];
	} else if (codepoint >= 0x2026u && codepoint <= 0x2027u) {
		return vga_block34[codepoint - 0x2026u];
	} else if (codepoint == 0x2030u) {
		return vga_block35[0];
	} else if (codepoint >= 0x2032u && codepoint <= 0x2033u) {
		return vga_block36[codepoint - 0x2032u];
	} else if (codepoint == 0x2035u) {
		return vga_block37[0];
	} else if (codepoint >= 0x2039u && codepoint <= 0x203au) {
		return vga_block38[codepoint - 0x2039u];
	} else if (codepoint == 0x203cu) {
		return vga_block39[0];
	} else if (codepoint >= 0x203eu && codepoint <= 0x2040u) {
		return vga_block40[codepoint - 0x203eu];
	} else if (codepoint == 0x2044u) {
		return vga_block41[0];
	} else if (codepoint == 0x2054u) {
		return vga_block42[0];
	} else if (codepoint >= 0x2074u && codepoint <= 0x207bu) {
		return vga_block43[codepoint - 0x2074u];
	} else if (codepoint == 0x207fu) {
		return vga_block44[0];
	} else if (codepoint >= 0x2081u && codepoint <= 0x208bu) {
		return vga_block45[codepoint - 0x2081u];
	} else if (codepoint >= 0x20a3u && codepoint <= 0x20a4u) {
		return vga_block46[codepoint - 0x20a3u];
	} else if (codepoint == 0x20a7u) {
		return vga_block47[0];
	} else if (codepoint == 0x20aau) {
		return vga_block48[0];
	} else if (codepoint == 0x20acu) {
		return vga_block49[0];
	} else if (codepoint == 0x2105u) {
		return vga_block50[0];
	} else if (codepoint == 0x2113u) {
		return vga_block51[0];
	} else if (codepoint == 0x2116u) {
		return vga_block52[0];
	} else if (codepoint == 0x2122u) {
		return vga_block53[0];
	} else if (codepoint == 0x2126u) {
		return vga_block54[0];
	} else if (codepoint == 0x212eu) {
		return vga_block55[0];
	} else if (codepoint >= 0x2150u && codepoint <= 0x2151u) {
		return vga_block56[codepoint - 0x2150u];
	} else if (codepoint >= 0x2153u && codepoint <= 0x215eu) {
		return vga_block57[codepoint - 0x2153u];
	} else if (codepoint >= 0x2190u && codepoint <= 0x2195u) {
		return vga_block58[codepoint - 0x2190u];
	} else if (codepoint == 0x21a8u) {
		return vga_block59[0];
	} else if (codepoint == 0x2202u) {
		return vga_block60[0];
	} else if (codepoint >= 0x2205u && codepoint <= 0x2206u) {
		return vga_block61[codepoint - 0x2205u];
	} else if (codepoint == 0x2208u) {
		return vga_block62[0];
	} else if (codepoint == 0x220fu) {
		return vga_block63[0];
	} else if (codepoint >= 0x2211u && codepoint <= 0x2212u) {
		return vga_block64[codepoint - 0x2211u];
	} else if (codepoint == 0x2215u) {
		return vga_block65[0];
	} else if (codepoint >= 0x2219u && codepoint <= 0x221au) {
		return vga_block66[codepoint - 0x2219u];
	} else if (codepoint >= 0x221eu && codepoint <= 0x221fu) {
		return vga_block67[codepoint - 0x221eu];
	} else if (codepoint == 0x2229u) {
		return vga_block68[0];
	} else if (codepoint == 0x222bu) {
		return vga_block69[0];
	} else if (codepoint == 0x2248u) {
		return vga_block70[0];
	} else if (codepoint >= 0x2260u && codepoint <= 0x2261u) {
		return vga_block71[codepoint - 0x2260u];
	} else if (codepoint >= 0x2264u && codepoint <= 0x2265u) {
		return vga_block72[codepoint - 0x2264u];
	} else if (codepoint == 0x2299u) {
		return vga_block73[0];
	} else if (codepoint == 0x2300u) {
		return vga_block74[0];
	} else if (codepoint == 0x2302u) {
		return vga_block75[0];
	} else if (codepoint == 0x2310u) {
		return vga_block76[0];
	} else if (codepoint >= 0x2320u && codepoint <= 0x2321u) {
		return vga_block77[codepoint - 0x2320u];
	} else if (codepoint == 0x2500u) {
		return vga_block78[0];
	} else if (codepoint == 0x2502u) {
		return vga_block79[0];
	} else if (codepoint == 0x250cu) {
		return vga_block80[0];
	} else if (codepoint == 0x2510u) {
		return vga_block81[0];
	} else if (codepoint == 0x2514u) {
		return vga_block82[0];
	} else if (codepoint == 0x2518u) {
		return vga_block83[0];
	} else if (codepoint == 0x251cu) {
		return vga_block84[0];
	} else if (codepoint == 0x2524u) {
		return vga_block85[0];
	} else if (codepoint == 0x252cu) {
		return vga_block86[0];
	} else if (codepoint == 0x2534u) {
		return vga_block87[0];
	} else if (codepoint == 0x253cu) {
		return vga_block88[0];
	} else if (codepoint >= 0x2550u && codepoint <= 0x256cu) {
		return vga_block89[codepoint - 0x2550u];
	} else if (codepoint >= 0x2580u && codepoint <= 0x25a1u) {
		return vga_block90[codepoint - 0x2580u];
	} else if (codepoint >= 0x25aau && codepoint <= 0x25acu) {
		return vga_block91[codepoint - 0x25aau];
	} else if (codepoint == 0x25b2u) {
		return vga_block92[0];
	} else if (codepoint == 0x25bau) {
		return vga_block93[0];
	} else if (codepoint == 0x25bcu) {
		return vga_block94[0];
	} else if (codepoint == 0x25c4u) {
		return vga_block95[0];
	} else if (codepoint >= 0x25cau && codepoint <= 0x25cbu) {
		return vga_block96[codepoint - 0x25cau];
	} else if (codepoint == 0x25cfu) {
		return vga_block97[0];
	} else if (codepoint >= 0x25d8u && codepoint <= 0x25d9u) {
		return vga_block98[codepoint - 0x25d8u];
	} else if (codepoint == 0x25e6u) {
		return vga_block99[0];
	} else if (codepoint >= 0x263au && codepoint <= 0x263cu) {
		return vga_block100[codepoint - 0x263au];
	} else if (codepoint == 0x2640u) {
		return vga_block101[0];
	} else if (codepoint == 0x2642u) {
		return vga_block102[0];
	} else if (codepoint == 0x2660u) {
		return vga_block103[0];
	} else if (codepoint == 0x2663u) {
		return vga_block104[0];
	} else if (codepoint >= 0x2665u && codepoint <= 0x2666u) {
		return vga_block105[codepoint - 0x2665u];
	} else if (codepoint >= 0x266au && codepoint <= 0x266bu) {
		return vga_block106[codepoint - 0x266au];
	} else if (codepoint == 0x2713u) {
		return vga_block107[0];
	} else if (codepoint >= 0xfb01u && codepoint <= 0xfb02u) {
		return vga_block108[codepoint - 0xfb01u];
	} else if (codepoint == 0xfffdu) {
		return vga_block109[0];
	} else {
		WARN("Codepoint U+%04X (%s) is not covered by this font", codepoint, u8_cp_to_utf8(codepoint));
		return vga_block1[0];
	}
}
