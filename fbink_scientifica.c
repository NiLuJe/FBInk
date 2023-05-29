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

#include "fbink_scientifica.h"

static const unsigned char*
    scientifica_get_bitmap(uint32_t codepoint)
{
	if (codepoint >= 0x20u && codepoint <= 0x7eu) {
		return scientifica_block1[codepoint - 0x20u];
	} else if (codepoint >= 0xa0u && codepoint <= 0x1b6u) {
		return scientifica_block2[codepoint - 0xa0u];
	} else if (codepoint >= 0x1c7u && codepoint <= 0x1d4u) {
		return scientifica_block3[codepoint - 0x1c7u];
	} else if (codepoint >= 0x20cu && codepoint <= 0x21bu) {
		return scientifica_block4[codepoint - 0x20cu];
	} else if (codepoint == 0x296u) {
		return scientifica_block5[0];
	} else if (codepoint == 0x298u) {
		return scientifica_block6[0];
	} else if (codepoint == 0x2c7u) {
		return scientifica_block7[0];
	} else if (codepoint == 0x2dau) {
		return scientifica_block8[0];
	} else if (codepoint == 0x364u) {
		return scientifica_block9[0];
	} else if (codepoint >= 0x391u && codepoint <= 0x3a9u) {
		return scientifica_block10[codepoint - 0x391u];
	} else if (codepoint >= 0x3b1u && codepoint <= 0x3c9u) {
		return scientifica_block11[codepoint - 0x3b1u];
	} else if (codepoint >= 0xf3cu && codepoint <= 0xf3du) {
		return scientifica_block12[codepoint - 0xf3cu];
	} else if (codepoint == 0x1e9eu) {
		return scientifica_block13[0];
	} else if (codepoint >= 0x2010u && codepoint <= 0x2027u) {
		return scientifica_block14[codepoint - 0x2010u];
	} else if (codepoint >= 0x2032u && codepoint <= 0x203cu) {
		return scientifica_block15[codepoint - 0x2032u];
	} else if (codepoint >= 0x203eu && codepoint <= 0x2042u) {
		return scientifica_block16[codepoint - 0x203eu];
	} else if (codepoint >= 0x204fu && codepoint <= 0x2051u) {
		return scientifica_block17[codepoint - 0x204fu];
	} else if (codepoint == 0x205cu) {
		return scientifica_block18[0];
	} else if (codepoint == 0x20a8u) {
		return scientifica_block19[0];
	} else if (codepoint == 0x20aau) {
		return scientifica_block20[0];
	} else if (codepoint == 0x20acu) {
		return scientifica_block21[0];
	} else if (codepoint == 0x2122u) {
		return scientifica_block22[0];
	} else if (codepoint >= 0x2190u && codepoint <= 0x2199u) {
		return scientifica_block23[codepoint - 0x2190u];
	} else if (codepoint >= 0x21a4u && codepoint <= 0x21a7u) {
		return scientifica_block24[codepoint - 0x21a4u];
	} else if (codepoint >= 0x21a9u && codepoint <= 0x21aau) {
		return scientifica_block25[codepoint - 0x21a9u];
	} else if (codepoint >= 0x21b0u && codepoint <= 0x21b7u) {
		return scientifica_block26[codepoint - 0x21b0u];
	} else if (codepoint >= 0x21b9u && codepoint <= 0x21c4u) {
		return scientifica_block27[codepoint - 0x21b9u];
	} else if (codepoint >= 0x21c6u && codepoint <= 0x21cbu) {
		return scientifica_block28[codepoint - 0x21c6u];
	} else if (codepoint >= 0x21d0u && codepoint <= 0x21d3u) {
		return scientifica_block29[codepoint - 0x21d0u];
	} else if (codepoint >= 0x2200u && codepoint <= 0x2214u) {
		return scientifica_block30[codepoint - 0x2200u];
	} else if (codepoint >= 0x2217u && codepoint <= 0x221au) {
		return scientifica_block31[codepoint - 0x2217u];
	} else if (codepoint >= 0x221du && codepoint <= 0x221eu) {
		return scientifica_block32[codepoint - 0x221du];
	} else if (codepoint >= 0x2227u && codepoint <= 0x222bu) {
		return scientifica_block33[codepoint - 0x2227u];
	} else if (codepoint >= 0x2234u && codepoint <= 0x2237u) {
		return scientifica_block34[codepoint - 0x2234u];
	} else if (codepoint == 0x2243u) {
		return scientifica_block35[0];
	} else if (codepoint == 0x2248u) {
		return scientifica_block36[0];
	} else if (codepoint >= 0x2254u && codepoint <= 0x2255u) {
		return scientifica_block37[codepoint - 0x2254u];
	} else if (codepoint >= 0x2260u && codepoint <= 0x2262u) {
		return scientifica_block38[codepoint - 0x2260u];
	} else if (codepoint >= 0x2264u && codepoint <= 0x2265u) {
		return scientifica_block39[codepoint - 0x2264u];
	} else if (codepoint >= 0x2282u && codepoint <= 0x2287u) {
		return scientifica_block40[codepoint - 0x2282u];
	} else if (codepoint >= 0x2295u && codepoint <= 0x2296u) {
		return scientifica_block41[codepoint - 0x2295u];
	} else if (codepoint == 0x2299u) {
		return scientifica_block42[0];
	} else if (codepoint >= 0x229eu && codepoint <= 0x22a9u) {
		return scientifica_block43[codepoint - 0x229eu];
	} else if (codepoint == 0x22abu) {
		return scientifica_block44[0];
	} else if (codepoint >= 0x22c4u && codepoint <= 0x22c6u) {
		return scientifica_block45[codepoint - 0x22c4u];
	} else if (codepoint == 0x2325u) {
		return scientifica_block46[0];
	} else if (codepoint >= 0x239bu && codepoint <= 0x23aeu) {
		return scientifica_block47[codepoint - 0x239bu];
	} else if (codepoint >= 0x23bau && codepoint <= 0x23bdu) {
		return scientifica_block48[codepoint - 0x23bau];
	} else if (codepoint >= 0x23e9u && codepoint <= 0x23ecu) {
		return scientifica_block49[codepoint - 0x23e9u];
	} else if (codepoint >= 0x23f4u && codepoint <= 0x23fbu) {
		return scientifica_block50[codepoint - 0x23f4u];
	} else if (codepoint >= 0x2500u && codepoint <= 0x2503u) {
		return scientifica_block51[codepoint - 0x2500u];
	} else if (codepoint >= 0x2506u && codepoint <= 0x2507u) {
		return scientifica_block52[codepoint - 0x2506u];
	} else if (codepoint >= 0x250au && codepoint <= 0x251du) {
		return scientifica_block53[codepoint - 0x250au];
	} else if (codepoint == 0x2520u) {
		return scientifica_block54[0];
	} else if (codepoint >= 0x2523u && codepoint <= 0x2525u) {
		return scientifica_block55[codepoint - 0x2523u];
	} else if (codepoint == 0x2528u) {
		return scientifica_block56[0];
	} else if (codepoint >= 0x252bu && codepoint <= 0x252cu) {
		return scientifica_block57[codepoint - 0x252bu];
	} else if (codepoint >= 0x252fu && codepoint <= 0x2530u) {
		return scientifica_block58[codepoint - 0x252fu];
	} else if (codepoint >= 0x2533u && codepoint <= 0x2534u) {
		return scientifica_block59[codepoint - 0x2533u];
	} else if (codepoint >= 0x2537u && codepoint <= 0x2538u) {
		return scientifica_block60[codepoint - 0x2537u];
	} else if (codepoint >= 0x253bu && codepoint <= 0x253cu) {
		return scientifica_block61[codepoint - 0x253bu];
	} else if (codepoint == 0x253fu) {
		return scientifica_block62[0];
	} else if (codepoint == 0x2542u) {
		return scientifica_block63[0];
	} else if (codepoint == 0x254bu) {
		return scientifica_block64[0];
	} else if (codepoint >= 0x2550u && codepoint <= 0x2573u) {
		return scientifica_block65[codepoint - 0x2550u];
	} else if (codepoint == 0x257cu) {
		return scientifica_block66[0];
	} else if (codepoint == 0x257eu) {
		return scientifica_block67[0];
	} else if (codepoint >= 0x2581u && codepoint <= 0x258au) {
		return scientifica_block68[codepoint - 0x2581u];
	} else if (codepoint == 0x258fu) {
		return scientifica_block69[0];
	} else if (codepoint >= 0x2591u && codepoint <= 0x2593u) {
		return scientifica_block70[codepoint - 0x2591u];
	} else if (codepoint >= 0x25a1u && codepoint <= 0x25a2u) {
		return scientifica_block71[codepoint - 0x25a1u];
	} else if (codepoint >= 0x25aau && codepoint <= 0x25abu) {
		return scientifica_block72[codepoint - 0x25aau];
	} else if (codepoint >= 0x25b2u && codepoint <= 0x25b9u) {
		return scientifica_block73[codepoint - 0x25b2u];
	} else if (codepoint >= 0x25bcu && codepoint <= 0x25beu) {
		return scientifica_block74[codepoint - 0x25bcu];
	} else if (codepoint >= 0x25c0u && codepoint <= 0x25c4u) {
		return scientifica_block75[codepoint - 0x25c0u];
	} else if (codepoint == 0x25cbu) {
		return scientifica_block76[0];
	} else if (codepoint >= 0x25ceu && codepoint <= 0x25d7u) {
		return scientifica_block77[codepoint - 0x25ceu];
	} else if (codepoint >= 0x25e7u && codepoint <= 0x25ebu) {
		return scientifica_block78[codepoint - 0x25e7u];
	} else if (codepoint >= 0x25f0u && codepoint <= 0x25f3u) {
		return scientifica_block79[codepoint - 0x25f0u];
	} else if (codepoint >= 0x2600u && codepoint <= 0x2602u) {
		return scientifica_block80[codepoint - 0x2600u];
	} else if (codepoint >= 0x2630u && codepoint <= 0x2637u) {
		return scientifica_block81[codepoint - 0x2630u];
	} else if (codepoint == 0x2661u) {
		return scientifica_block82[0];
	} else if (codepoint >= 0x2665u && codepoint <= 0x2666u) {
		return scientifica_block83[codepoint - 0x2665u];
	} else if (codepoint >= 0x2669u && codepoint <= 0x266cu) {
		return scientifica_block84[codepoint - 0x2669u];
	} else if (codepoint == 0x267au) {
		return scientifica_block85[0];
	} else if (codepoint >= 0x26aau && codepoint <= 0x26abu) {
		return scientifica_block86[codepoint - 0x26aau];
	} else if (codepoint >= 0x2713u && codepoint <= 0x2718u) {
		return scientifica_block87[codepoint - 0x2713u];
	} else if (codepoint == 0x272eu) {
		return scientifica_block88[0];
	} else if (codepoint == 0x2744u) {
		return scientifica_block89[0];
	} else if (codepoint >= 0x276eu && codepoint <= 0x2771u) {
		return scientifica_block90[codepoint - 0x276eu];
	} else if (codepoint >= 0x27c2u && codepoint <= 0x27c4u) {
		return scientifica_block91[codepoint - 0x27c2u];
	} else if (codepoint >= 0x27d8u && codepoint <= 0x27d9u) {
		return scientifica_block92[codepoint - 0x27d8u];
	} else if (codepoint >= 0x27dcu && codepoint <= 0x27deu) {
		return scientifica_block93[codepoint - 0x27dcu];
	} else if (codepoint >= 0x2864u && codepoint <= 0x28ffu) {
		return scientifica_block94[codepoint - 0x2864u];
	} else if (codepoint >= 0x2919u && codepoint <= 0x291cu) {
		return scientifica_block95[codepoint - 0x2919u];
	} else if (codepoint >= 0x2b5eu && codepoint <= 0x2b62u) {
		return scientifica_block96[codepoint - 0x2b5eu];
	} else if (codepoint == 0x2b64u) {
		return scientifica_block97[0];
	} else if (codepoint >= 0x2b80u && codepoint <= 0x2b83u) {
		return scientifica_block98[codepoint - 0x2b80u];
	} else if (codepoint >= 0x30c3u && codepoint <= 0x30c4u) {
		return scientifica_block99[codepoint - 0x30c3u];
	} else if (codepoint == 0x5350u) {
		return scientifica_block100[0];
	} else if (codepoint >= 0xe09eu && codepoint <= 0xe0a2u) {
		return scientifica_block101[codepoint - 0xe09eu];
	} else if (codepoint >= 0xe0b0u && codepoint <= 0xe0c6u) {
		return scientifica_block102[codepoint - 0xe0b0u];
	} else if (codepoint >= 0xe0d1u && codepoint <= 0xe0d2u) {
		return scientifica_block103[codepoint - 0xe0d1u];
	} else if (codepoint == 0xe0d4u) {
		return scientifica_block104[0];
	} else if (codepoint == 0xf031u) {
		return scientifica_block105[0];
	} else if (codepoint >= 0xf033u && codepoint <= 0xf03du) {
		return scientifica_block106[codepoint - 0xf033u];
	} else if (codepoint >= 0xf057u && codepoint <= 0xf059u) {
		return scientifica_block107[codepoint - 0xf057u];
	} else if (codepoint == 0xf061u) {
		return scientifica_block108[0];
	} else if (codepoint == 0xf073u) {
		return scientifica_block109[0];
	} else if (codepoint >= 0xf078u && codepoint <= 0xf079u) {
		return scientifica_block110[codepoint - 0xf078u];
	} else if (codepoint == 0xf07eu) {
		return scientifica_block111[0];
	} else if (codepoint >= 0xf0cfu && codepoint <= 0xf0d1u) {
		return scientifica_block112[codepoint - 0xf0cfu];
	} else if (codepoint >= 0xf0d5u && codepoint <= 0xf0dcu) {
		return scientifica_block113[codepoint - 0xf0d5u];
	} else if (codepoint >= 0xf0deu && codepoint <= 0xf0e0u) {
		return scientifica_block114[codepoint - 0xf0deu];
	} else if (codepoint >= 0xf0edu && codepoint <= 0xf0f0u) {
		return scientifica_block115[codepoint - 0xf0edu];
	} else {
		WARN("Codepoint U+%04X (%s) is not covered by this font", codepoint, u8_cp_to_utf8(codepoint));
		return scientifica_block1[0];
	}
}

static const unsigned char*
    scientificab_get_bitmap(uint32_t codepoint)
{
	if (codepoint >= 0x20u && codepoint <= 0x7eu) {
		return scientificab_block1[codepoint - 0x20u];
	} else if (codepoint >= 0xa0u && codepoint <= 0x1b6u) {
		return scientificab_block2[codepoint - 0xa0u];
	} else if (codepoint >= 0x1c7u && codepoint <= 0x1d4u) {
		return scientificab_block3[codepoint - 0x1c7u];
	} else if (codepoint >= 0x20cu && codepoint <= 0x211u) {
		return scientificab_block4[codepoint - 0x20cu];
	} else if (codepoint >= 0x213u && codepoint <= 0x21bu) {
		return scientificab_block5[codepoint - 0x213u];
	} else if (codepoint == 0x296u) {
		return scientificab_block6[0];
	} else if (codepoint == 0x2c7u) {
		return scientificab_block7[0];
	} else if (codepoint == 0x2dau) {
		return scientificab_block8[0];
	} else if (codepoint == 0x364u) {
		return scientificab_block9[0];
	} else if (codepoint >= 0x391u && codepoint <= 0x3a9u) {
		return scientificab_block10[codepoint - 0x391u];
	} else if (codepoint >= 0x3b1u && codepoint <= 0x3c9u) {
		return scientificab_block11[codepoint - 0x3b1u];
	} else if (codepoint >= 0xf3cu && codepoint <= 0xf3du) {
		return scientificab_block12[codepoint - 0xf3cu];
	} else if (codepoint == 0x1e9eu) {
		return scientificab_block13[0];
	} else if (codepoint >= 0x2010u && codepoint <= 0x2027u) {
		return scientificab_block14[codepoint - 0x2010u];
	} else if (codepoint >= 0x2032u && codepoint <= 0x203au) {
		return scientificab_block15[codepoint - 0x2032u];
	} else if (codepoint == 0x203cu) {
		return scientificab_block16[0];
	} else if (codepoint >= 0x203eu && codepoint <= 0x2042u) {
		return scientificab_block17[codepoint - 0x203eu];
	} else if (codepoint >= 0x204fu && codepoint <= 0x2051u) {
		return scientificab_block18[codepoint - 0x204fu];
	} else if (codepoint == 0x20acu) {
		return scientificab_block19[0];
	} else if (codepoint == 0x2122u) {
		return scientificab_block20[0];
	} else if (codepoint >= 0x2190u && codepoint <= 0x2199u) {
		return scientificab_block21[codepoint - 0x2190u];
	} else if (codepoint >= 0x21a4u && codepoint <= 0x21a7u) {
		return scientificab_block22[codepoint - 0x21a4u];
	} else if (codepoint >= 0x21a9u && codepoint <= 0x21aau) {
		return scientificab_block23[codepoint - 0x21a9u];
	} else if (codepoint >= 0x21b1u && codepoint <= 0x21b4u) {
		return scientificab_block24[codepoint - 0x21b1u];
	} else if (codepoint >= 0x21b9u && codepoint <= 0x21c4u) {
		return scientificab_block25[codepoint - 0x21b9u];
	} else if (codepoint >= 0x21c6u && codepoint <= 0x21c7u) {
		return scientificab_block26[codepoint - 0x21c6u];
	} else if (codepoint == 0x21c9u) {
		return scientificab_block27[0];
	} else if (codepoint >= 0x21d0u && codepoint <= 0x21d3u) {
		return scientificab_block28[codepoint - 0x21d0u];
	} else if (codepoint >= 0x2200u && codepoint <= 0x2214u) {
		return scientificab_block29[codepoint - 0x2200u];
	} else if (codepoint >= 0x2218u && codepoint <= 0x221au) {
		return scientificab_block30[codepoint - 0x2218u];
	} else if (codepoint >= 0x221du && codepoint <= 0x221eu) {
		return scientificab_block31[codepoint - 0x221du];
	} else if (codepoint >= 0x2229u && codepoint <= 0x222bu) {
		return scientificab_block32[codepoint - 0x2229u];
	} else if (codepoint >= 0x2234u && codepoint <= 0x2237u) {
		return scientificab_block33[codepoint - 0x2234u];
	} else if (codepoint == 0x2248u) {
		return scientificab_block34[0];
	} else if (codepoint >= 0x2260u && codepoint <= 0x2261u) {
		return scientificab_block35[codepoint - 0x2260u];
	} else if (codepoint >= 0x2264u && codepoint <= 0x2265u) {
		return scientificab_block36[codepoint - 0x2264u];
	} else if (codepoint == 0x227au) {
		return scientificab_block37[0];
	} else if (codepoint >= 0x2282u && codepoint <= 0x2287u) {
		return scientificab_block38[codepoint - 0x2282u];
	} else if (codepoint >= 0x2295u && codepoint <= 0x2296u) {
		return scientificab_block39[codepoint - 0x2295u];
	} else if (codepoint == 0x2299u) {
		return scientificab_block40[0];
	} else if (codepoint >= 0x229eu && codepoint <= 0x22a9u) {
		return scientificab_block41[codepoint - 0x229eu];
	} else if (codepoint == 0x22abu) {
		return scientificab_block42[0];
	} else if (codepoint >= 0x22c4u && codepoint <= 0x22c6u) {
		return scientificab_block43[codepoint - 0x22c4u];
	} else if (codepoint >= 0x23bau && codepoint <= 0x23bdu) {
		return scientificab_block44[codepoint - 0x23bau];
	} else if (codepoint >= 0x23e9u && codepoint <= 0x23ecu) {
		return scientificab_block45[codepoint - 0x23e9u];
	} else if (codepoint >= 0x23f4u && codepoint <= 0x23fbu) {
		return scientificab_block46[codepoint - 0x23f4u];
	} else if (codepoint >= 0x2500u && codepoint <= 0x2503u) {
		return scientificab_block47[codepoint - 0x2500u];
	} else if (codepoint >= 0x250au && codepoint <= 0x251du) {
		return scientificab_block48[codepoint - 0x250au];
	} else if (codepoint == 0x2520u) {
		return scientificab_block49[0];
	} else if (codepoint >= 0x2523u && codepoint <= 0x2525u) {
		return scientificab_block50[codepoint - 0x2523u];
	} else if (codepoint == 0x2528u) {
		return scientificab_block51[0];
	} else if (codepoint >= 0x252bu && codepoint <= 0x252cu) {
		return scientificab_block52[codepoint - 0x252bu];
	} else if (codepoint >= 0x252fu && codepoint <= 0x2530u) {
		return scientificab_block53[codepoint - 0x252fu];
	} else if (codepoint >= 0x2533u && codepoint <= 0x2534u) {
		return scientificab_block54[codepoint - 0x2533u];
	} else if (codepoint >= 0x2537u && codepoint <= 0x2538u) {
		return scientificab_block55[codepoint - 0x2537u];
	} else if (codepoint >= 0x253bu && codepoint <= 0x253cu) {
		return scientificab_block56[codepoint - 0x253bu];
	} else if (codepoint == 0x253fu) {
		return scientificab_block57[0];
	} else if (codepoint == 0x2542u) {
		return scientificab_block58[0];
	} else if (codepoint == 0x254bu) {
		return scientificab_block59[0];
	} else if (codepoint >= 0x2550u && codepoint <= 0x2570u) {
		return scientificab_block60[codepoint - 0x2550u];
	} else if (codepoint == 0x257cu) {
		return scientificab_block61[0];
	} else if (codepoint == 0x257eu) {
		return scientificab_block62[0];
	} else if (codepoint >= 0x2581u && codepoint <= 0x258au) {
		return scientificab_block63[codepoint - 0x2581u];
	} else if (codepoint == 0x258fu) {
		return scientificab_block64[0];
	} else if (codepoint >= 0x2591u && codepoint <= 0x2593u) {
		return scientificab_block65[codepoint - 0x2591u];
	} else if (codepoint >= 0x25a1u && codepoint <= 0x25a2u) {
		return scientificab_block66[codepoint - 0x25a1u];
	} else if (codepoint == 0x25aau) {
		return scientificab_block67[0];
	} else if (codepoint >= 0x25b2u && codepoint <= 0x25b4u) {
		return scientificab_block68[codepoint - 0x25b2u];
	} else if (codepoint >= 0x25b6u && codepoint <= 0x25b9u) {
		return scientificab_block69[codepoint - 0x25b6u];
	} else if (codepoint >= 0x25bcu && codepoint <= 0x25beu) {
		return scientificab_block70[codepoint - 0x25bcu];
	} else if (codepoint >= 0x25c0u && codepoint <= 0x25c4u) {
		return scientificab_block71[codepoint - 0x25c0u];
	} else if (codepoint >= 0x25d2u && codepoint <= 0x25d3u) {
		return scientificab_block72[codepoint - 0x25d2u];
	} else if (codepoint >= 0x25d5u && codepoint <= 0x25d7u) {
		return scientificab_block73[codepoint - 0x25d5u];
	} else if (codepoint >= 0x25e7u && codepoint <= 0x25eau) {
		return scientificab_block74[codepoint - 0x25e7u];
	} else if (codepoint >= 0x2600u && codepoint <= 0x2602u) {
		return scientificab_block75[codepoint - 0x2600u];
	} else if (codepoint >= 0x2630u && codepoint <= 0x2637u) {
		return scientificab_block76[codepoint - 0x2630u];
	} else if (codepoint == 0x2661u) {
		return scientificab_block77[0];
	} else if (codepoint == 0x2665u) {
		return scientificab_block78[0];
	} else if (codepoint >= 0x2669u && codepoint <= 0x266cu) {
		return scientificab_block79[codepoint - 0x2669u];
	} else if (codepoint >= 0x2713u && codepoint <= 0x2718u) {
		return scientificab_block80[codepoint - 0x2713u];
	} else if (codepoint == 0x272eu) {
		return scientificab_block81[0];
	} else if (codepoint == 0x2744u) {
		return scientificab_block82[0];
	} else if (codepoint >= 0x276eu && codepoint <= 0x2771u) {
		return scientificab_block83[codepoint - 0x276eu];
	} else if (codepoint >= 0x27c2u && codepoint <= 0x27c4u) {
		return scientificab_block84[codepoint - 0x27c2u];
	} else if (codepoint >= 0x27d8u && codepoint <= 0x27d9u) {
		return scientificab_block85[codepoint - 0x27d8u];
	} else if (codepoint >= 0x27dcu && codepoint <= 0x27deu) {
		return scientificab_block86[codepoint - 0x27dcu];
	} else if (codepoint >= 0x287du && codepoint <= 0x28ffu) {
		return scientificab_block87[codepoint - 0x287du];
	} else if (codepoint >= 0x2919u && codepoint <= 0x291cu) {
		return scientificab_block88[codepoint - 0x2919u];
	} else if (codepoint >= 0x2b5eu && codepoint <= 0x2b62u) {
		return scientificab_block89[codepoint - 0x2b5eu];
	} else if (codepoint == 0x2b64u) {
		return scientificab_block90[0];
	} else if (codepoint >= 0x2b80u && codepoint <= 0x2b83u) {
		return scientificab_block91[codepoint - 0x2b80u];
	} else if (codepoint >= 0x30c3u && codepoint <= 0x30c4u) {
		return scientificab_block92[codepoint - 0x30c3u];
	} else if (codepoint == 0x5350u) {
		return scientificab_block93[0];
	} else if (codepoint >= 0xe09eu && codepoint <= 0xe0a2u) {
		return scientificab_block94[codepoint - 0xe09eu];
	} else if (codepoint >= 0xe0b0u && codepoint <= 0xe0b3u) {
		return scientificab_block95[codepoint - 0xe0b0u];
	} else if (codepoint >= 0xe0c0u && codepoint <= 0xe0c6u) {
		return scientificab_block96[codepoint - 0xe0c0u];
	} else if (codepoint == 0xe0d1u) {
		return scientificab_block97[0];
	} else if (codepoint == 0xf031u) {
		return scientificab_block98[0];
	} else if (codepoint >= 0xf033u && codepoint <= 0xf03du) {
		return scientificab_block99[codepoint - 0xf033u];
	} else if (codepoint >= 0xf057u && codepoint <= 0xf059u) {
		return scientificab_block100[codepoint - 0xf057u];
	} else if (codepoint == 0xf061u) {
		return scientificab_block101[0];
	} else if (codepoint == 0xf073u) {
		return scientificab_block102[0];
	} else if (codepoint >= 0xf078u && codepoint <= 0xf079u) {
		return scientificab_block103[codepoint - 0xf078u];
	} else if (codepoint == 0xf07eu) {
		return scientificab_block104[0];
	} else if (codepoint >= 0xf0cfu && codepoint <= 0xf0d1u) {
		return scientificab_block105[codepoint - 0xf0cfu];
	} else if (codepoint >= 0xf0d5u && codepoint <= 0xf0dcu) {
		return scientificab_block106[codepoint - 0xf0d5u];
	} else if (codepoint >= 0xf0deu && codepoint <= 0xf0e0u) {
		return scientificab_block107[codepoint - 0xf0deu];
	} else if (codepoint >= 0xf0edu && codepoint <= 0xf0f0u) {
		return scientificab_block108[codepoint - 0xf0edu];
	} else {
		WARN("Codepoint U+%04X (%s) is not covered by this font", codepoint, u8_cp_to_utf8(codepoint));
		return scientificab_block1[0];
	}
}

static const unsigned char*
    scientificai_get_bitmap(uint32_t codepoint)
{
	if (codepoint >= 0x20u && codepoint <= 0x7eu) {
		return scientificai_block1[codepoint - 0x20u];
	} else if (codepoint >= 0xa0u && codepoint <= 0x1b6u) {
		return scientificai_block2[codepoint - 0xa0u];
	} else if (codepoint >= 0x1c7u && codepoint <= 0x1d4u) {
		return scientificai_block3[codepoint - 0x1c7u];
	} else if (codepoint >= 0x20cu && codepoint <= 0x21bu) {
		return scientificai_block4[codepoint - 0x20cu];
	} else if (codepoint == 0x296u) {
		return scientificai_block5[0];
	} else if (codepoint == 0x298u) {
		return scientificai_block6[0];
	} else if (codepoint == 0x2c7u) {
		return scientificai_block7[0];
	} else if (codepoint == 0x2dau) {
		return scientificai_block8[0];
	} else if (codepoint == 0x364u) {
		return scientificai_block9[0];
	} else if (codepoint >= 0x391u && codepoint <= 0x3a9u) {
		return scientificai_block10[codepoint - 0x391u];
	} else if (codepoint >= 0x3b1u && codepoint <= 0x3c9u) {
		return scientificai_block11[codepoint - 0x3b1u];
	} else if (codepoint >= 0xf3cu && codepoint <= 0xf3du) {
		return scientificai_block12[codepoint - 0xf3cu];
	} else if (codepoint == 0x1e9eu) {
		return scientificai_block13[0];
	} else if (codepoint >= 0x2010u && codepoint <= 0x2027u) {
		return scientificai_block14[codepoint - 0x2010u];
	} else if (codepoint >= 0x2032u && codepoint <= 0x203cu) {
		return scientificai_block15[codepoint - 0x2032u];
	} else if (codepoint >= 0x203eu && codepoint <= 0x2042u) {
		return scientificai_block16[codepoint - 0x203eu];
	} else if (codepoint >= 0x204fu && codepoint <= 0x2051u) {
		return scientificai_block17[codepoint - 0x204fu];
	} else if (codepoint == 0x205cu) {
		return scientificai_block18[0];
	} else if (codepoint == 0x20a8u) {
		return scientificai_block19[0];
	} else if (codepoint == 0x20aau) {
		return scientificai_block20[0];
	} else if (codepoint == 0x20acu) {
		return scientificai_block21[0];
	} else if (codepoint == 0x2122u) {
		return scientificai_block22[0];
	} else if (codepoint >= 0x2190u && codepoint <= 0x2199u) {
		return scientificai_block23[codepoint - 0x2190u];
	} else if (codepoint >= 0x21a4u && codepoint <= 0x21a7u) {
		return scientificai_block24[codepoint - 0x21a4u];
	} else if (codepoint >= 0x21a9u && codepoint <= 0x21aau) {
		return scientificai_block25[codepoint - 0x21a9u];
	} else if (codepoint >= 0x21b0u && codepoint <= 0x21b7u) {
		return scientificai_block26[codepoint - 0x21b0u];
	} else if (codepoint >= 0x21b9u && codepoint <= 0x21c4u) {
		return scientificai_block27[codepoint - 0x21b9u];
	} else if (codepoint >= 0x21c6u && codepoint <= 0x21cbu) {
		return scientificai_block28[codepoint - 0x21c6u];
	} else if (codepoint >= 0x21d0u && codepoint <= 0x21d3u) {
		return scientificai_block29[codepoint - 0x21d0u];
	} else if (codepoint >= 0x2200u && codepoint <= 0x2214u) {
		return scientificai_block30[codepoint - 0x2200u];
	} else if (codepoint >= 0x2217u && codepoint <= 0x221au) {
		return scientificai_block31[codepoint - 0x2217u];
	} else if (codepoint >= 0x221du && codepoint <= 0x221eu) {
		return scientificai_block32[codepoint - 0x221du];
	} else if (codepoint >= 0x2227u && codepoint <= 0x222bu) {
		return scientificai_block33[codepoint - 0x2227u];
	} else if (codepoint >= 0x2234u && codepoint <= 0x2237u) {
		return scientificai_block34[codepoint - 0x2234u];
	} else if (codepoint == 0x2243u) {
		return scientificai_block35[0];
	} else if (codepoint == 0x2248u) {
		return scientificai_block36[0];
	} else if (codepoint >= 0x2254u && codepoint <= 0x2255u) {
		return scientificai_block37[codepoint - 0x2254u];
	} else if (codepoint >= 0x2260u && codepoint <= 0x2262u) {
		return scientificai_block38[codepoint - 0x2260u];
	} else if (codepoint >= 0x2264u && codepoint <= 0x2265u) {
		return scientificai_block39[codepoint - 0x2264u];
	} else if (codepoint >= 0x2282u && codepoint <= 0x2287u) {
		return scientificai_block40[codepoint - 0x2282u];
	} else if (codepoint >= 0x2295u && codepoint <= 0x2296u) {
		return scientificai_block41[codepoint - 0x2295u];
	} else if (codepoint == 0x2299u) {
		return scientificai_block42[0];
	} else if (codepoint >= 0x229eu && codepoint <= 0x22a9u) {
		return scientificai_block43[codepoint - 0x229eu];
	} else if (codepoint == 0x22abu) {
		return scientificai_block44[0];
	} else if (codepoint >= 0x22c4u && codepoint <= 0x22c6u) {
		return scientificai_block45[codepoint - 0x22c4u];
	} else if (codepoint == 0x2325u) {
		return scientificai_block46[0];
	} else if (codepoint >= 0x239bu && codepoint <= 0x23aeu) {
		return scientificai_block47[codepoint - 0x239bu];
	} else if (codepoint >= 0x23bau && codepoint <= 0x23bdu) {
		return scientificai_block48[codepoint - 0x23bau];
	} else if (codepoint >= 0x23e9u && codepoint <= 0x23ecu) {
		return scientificai_block49[codepoint - 0x23e9u];
	} else if (codepoint >= 0x23f4u && codepoint <= 0x23fbu) {
		return scientificai_block50[codepoint - 0x23f4u];
	} else if (codepoint >= 0x2500u && codepoint <= 0x2503u) {
		return scientificai_block51[codepoint - 0x2500u];
	} else if (codepoint >= 0x2506u && codepoint <= 0x2507u) {
		return scientificai_block52[codepoint - 0x2506u];
	} else if (codepoint >= 0x250au && codepoint <= 0x251du) {
		return scientificai_block53[codepoint - 0x250au];
	} else if (codepoint == 0x2520u) {
		return scientificai_block54[0];
	} else if (codepoint >= 0x2523u && codepoint <= 0x2525u) {
		return scientificai_block55[codepoint - 0x2523u];
	} else if (codepoint == 0x2528u) {
		return scientificai_block56[0];
	} else if (codepoint >= 0x252bu && codepoint <= 0x252cu) {
		return scientificai_block57[codepoint - 0x252bu];
	} else if (codepoint >= 0x252fu && codepoint <= 0x2530u) {
		return scientificai_block58[codepoint - 0x252fu];
	} else if (codepoint >= 0x2533u && codepoint <= 0x2534u) {
		return scientificai_block59[codepoint - 0x2533u];
	} else if (codepoint >= 0x2537u && codepoint <= 0x2538u) {
		return scientificai_block60[codepoint - 0x2537u];
	} else if (codepoint >= 0x253bu && codepoint <= 0x253cu) {
		return scientificai_block61[codepoint - 0x253bu];
	} else if (codepoint == 0x253fu) {
		return scientificai_block62[0];
	} else if (codepoint == 0x2542u) {
		return scientificai_block63[0];
	} else if (codepoint == 0x254bu) {
		return scientificai_block64[0];
	} else if (codepoint >= 0x2550u && codepoint <= 0x2573u) {
		return scientificai_block65[codepoint - 0x2550u];
	} else if (codepoint == 0x257cu) {
		return scientificai_block66[0];
	} else if (codepoint == 0x257eu) {
		return scientificai_block67[0];
	} else if (codepoint >= 0x2581u && codepoint <= 0x258au) {
		return scientificai_block68[codepoint - 0x2581u];
	} else if (codepoint == 0x258fu) {
		return scientificai_block69[0];
	} else if (codepoint >= 0x2591u && codepoint <= 0x2593u) {
		return scientificai_block70[codepoint - 0x2591u];
	} else if (codepoint >= 0x25a1u && codepoint <= 0x25a2u) {
		return scientificai_block71[codepoint - 0x25a1u];
	} else if (codepoint >= 0x25aau && codepoint <= 0x25abu) {
		return scientificai_block72[codepoint - 0x25aau];
	} else if (codepoint >= 0x25b2u && codepoint <= 0x25b9u) {
		return scientificai_block73[codepoint - 0x25b2u];
	} else if (codepoint >= 0x25bcu && codepoint <= 0x25beu) {
		return scientificai_block74[codepoint - 0x25bcu];
	} else if (codepoint >= 0x25c0u && codepoint <= 0x25c4u) {
		return scientificai_block75[codepoint - 0x25c0u];
	} else if (codepoint == 0x25cbu) {
		return scientificai_block76[0];
	} else if (codepoint >= 0x25ceu && codepoint <= 0x25d7u) {
		return scientificai_block77[codepoint - 0x25ceu];
	} else if (codepoint >= 0x25e7u && codepoint <= 0x25ebu) {
		return scientificai_block78[codepoint - 0x25e7u];
	} else if (codepoint >= 0x25f0u && codepoint <= 0x25f3u) {
		return scientificai_block79[codepoint - 0x25f0u];
	} else if (codepoint >= 0x2600u && codepoint <= 0x2602u) {
		return scientificai_block80[codepoint - 0x2600u];
	} else if (codepoint >= 0x2630u && codepoint <= 0x2637u) {
		return scientificai_block81[codepoint - 0x2630u];
	} else if (codepoint == 0x2661u) {
		return scientificai_block82[0];
	} else if (codepoint >= 0x2665u && codepoint <= 0x2666u) {
		return scientificai_block83[codepoint - 0x2665u];
	} else if (codepoint >= 0x2669u && codepoint <= 0x266cu) {
		return scientificai_block84[codepoint - 0x2669u];
	} else if (codepoint == 0x267au) {
		return scientificai_block85[0];
	} else if (codepoint >= 0x26aau && codepoint <= 0x26abu) {
		return scientificai_block86[codepoint - 0x26aau];
	} else if (codepoint >= 0x2713u && codepoint <= 0x2718u) {
		return scientificai_block87[codepoint - 0x2713u];
	} else if (codepoint == 0x272eu) {
		return scientificai_block88[0];
	} else if (codepoint == 0x2744u) {
		return scientificai_block89[0];
	} else if (codepoint >= 0x276eu && codepoint <= 0x2771u) {
		return scientificai_block90[codepoint - 0x276eu];
	} else if (codepoint >= 0x27c2u && codepoint <= 0x27c4u) {
		return scientificai_block91[codepoint - 0x27c2u];
	} else if (codepoint >= 0x27d8u && codepoint <= 0x27d9u) {
		return scientificai_block92[codepoint - 0x27d8u];
	} else if (codepoint >= 0x27dcu && codepoint <= 0x27deu) {
		return scientificai_block93[codepoint - 0x27dcu];
	} else if (codepoint >= 0x2864u && codepoint <= 0x28ffu) {
		return scientificai_block94[codepoint - 0x2864u];
	} else if (codepoint >= 0x2919u && codepoint <= 0x291cu) {
		return scientificai_block95[codepoint - 0x2919u];
	} else if (codepoint >= 0x2b5eu && codepoint <= 0x2b62u) {
		return scientificai_block96[codepoint - 0x2b5eu];
	} else if (codepoint == 0x2b64u) {
		return scientificai_block97[0];
	} else if (codepoint >= 0x2b80u && codepoint <= 0x2b83u) {
		return scientificai_block98[codepoint - 0x2b80u];
	} else if (codepoint >= 0x30c3u && codepoint <= 0x30c4u) {
		return scientificai_block99[codepoint - 0x30c3u];
	} else if (codepoint == 0x5350u) {
		return scientificai_block100[0];
	} else if (codepoint >= 0xe09eu && codepoint <= 0xe0a2u) {
		return scientificai_block101[codepoint - 0xe09eu];
	} else if (codepoint >= 0xe0b0u && codepoint <= 0xe0b5u) {
		return scientificai_block102[codepoint - 0xe0b0u];
	} else if (codepoint >= 0xe0c0u && codepoint <= 0xe0c6u) {
		return scientificai_block103[codepoint - 0xe0c0u];
	} else if (codepoint == 0xe0d1u) {
		return scientificai_block104[0];
	} else if (codepoint == 0xf031u) {
		return scientificai_block105[0];
	} else if (codepoint >= 0xf033u && codepoint <= 0xf03du) {
		return scientificai_block106[codepoint - 0xf033u];
	} else if (codepoint >= 0xf057u && codepoint <= 0xf059u) {
		return scientificai_block107[codepoint - 0xf057u];
	} else if (codepoint == 0xf061u) {
		return scientificai_block108[0];
	} else if (codepoint == 0xf073u) {
		return scientificai_block109[0];
	} else if (codepoint >= 0xf078u && codepoint <= 0xf079u) {
		return scientificai_block110[codepoint - 0xf078u];
	} else if (codepoint == 0xf07eu) {
		return scientificai_block111[0];
	} else if (codepoint >= 0xf0cfu && codepoint <= 0xf0d1u) {
		return scientificai_block112[codepoint - 0xf0cfu];
	} else if (codepoint >= 0xf0d5u && codepoint <= 0xf0dcu) {
		return scientificai_block113[codepoint - 0xf0d5u];
	} else if (codepoint >= 0xf0deu && codepoint <= 0xf0e0u) {
		return scientificai_block114[codepoint - 0xf0deu];
	} else if (codepoint >= 0xf0edu && codepoint <= 0xf0f0u) {
		return scientificai_block115[codepoint - 0xf0edu];
	} else {
		WARN("Codepoint U+%04X (%s) is not covered by this font", codepoint, u8_cp_to_utf8(codepoint));
		return scientificai_block1[0];
	}
}
