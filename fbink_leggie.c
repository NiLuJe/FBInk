/*
	FBInk: FrameBuffer eInker, a tool to print text & images on eInk devices (Kobo/Kindle)
	Copyright (C) 2018 NiLuJe <ninuje@gmail.com>

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

#include "fbink_leggie.h"

static const unsigned char*
    leggie_get_bitmap(uint32_t codepoint)
{
	if (codepoint >= 0x20 && codepoint <= 0x7e) {
		return leggie_block1[codepoint - 0x20];
	} else if (codepoint >= 0xa0 && codepoint <= 0x17f) {
		return leggie_block2[codepoint - 0xa0];
	} else if (codepoint >= 0x18e && codepoint <= 0x18f) {
		return leggie_block3[codepoint - 0x18e];
	} else if (codepoint >= 0x192 && codepoint <= 0x192) {
		return leggie_block4[codepoint - 0x192];
	} else if (codepoint >= 0x1a0 && codepoint <= 0x1a1) {
		return leggie_block5[codepoint - 0x1a0];
	} else if (codepoint >= 0x1af && codepoint <= 0x1b0) {
		return leggie_block6[codepoint - 0x1af];
	} else if (codepoint >= 0x1b5 && codepoint <= 0x1b7) {
		return leggie_block7[codepoint - 0x1b5];
	} else if (codepoint >= 0x1cd && codepoint <= 0x1dd) {
		return leggie_block8[codepoint - 0x1cd];
	} else if (codepoint >= 0x1e4 && codepoint <= 0x1e9) {
		return leggie_block9[codepoint - 0x1e4];
	} else if (codepoint >= 0x1ee && codepoint <= 0x1ef) {
		return leggie_block10[codepoint - 0x1ee];
	} else if (codepoint >= 0x1fa && codepoint <= 0x1ff) {
		return leggie_block11[codepoint - 0x1fa];
	} else if (codepoint >= 0x218 && codepoint <= 0x21b) {
		return leggie_block12[codepoint - 0x218];
	} else if (codepoint >= 0x250 && codepoint <= 0x2ee) {
		return leggie_block13[codepoint - 0x250];
	} else if (codepoint >= 0x37a && codepoint <= 0x37a) {
		return leggie_block14[codepoint - 0x37a];
	} else if (codepoint >= 0x37e && codepoint <= 0x37e) {
		return leggie_block15[codepoint - 0x37e];
	} else if (codepoint >= 0x384 && codepoint <= 0x386) {
		return leggie_block16[codepoint - 0x384];
	} else if (codepoint >= 0x388 && codepoint <= 0x38a) {
		return leggie_block17[codepoint - 0x388];
	} else if (codepoint >= 0x38c && codepoint <= 0x38c) {
		return leggie_block18[codepoint - 0x38c];
	} else if (codepoint >= 0x38e && codepoint <= 0x3a1) {
		return leggie_block19[codepoint - 0x38e];
	} else if (codepoint >= 0x3a3 && codepoint <= 0x3ce) {
		return leggie_block20[codepoint - 0x3a3];
	} else if (codepoint >= 0x400 && codepoint <= 0x477) {
		return leggie_block21[codepoint - 0x400];
	} else if (codepoint >= 0x480 && codepoint <= 0x481) {
		return leggie_block22[codepoint - 0x480];
	} else if (codepoint >= 0x48a && codepoint <= 0x493) {
		return leggie_block23[codepoint - 0x48a];
	} else if (codepoint >= 0x496 && codepoint <= 0x49d) {
		return leggie_block24[codepoint - 0x496];
	} else if (codepoint >= 0x4a0 && codepoint <= 0x4a3) {
		return leggie_block25[codepoint - 0x4a0];
	} else if (codepoint >= 0x4aa && codepoint <= 0x4ab) {
		return leggie_block26[codepoint - 0x4aa];
	} else if (codepoint >= 0x4ae && codepoint <= 0x4b1) {
		return leggie_block27[codepoint - 0x4ae];
	} else if (codepoint >= 0x4ba && codepoint <= 0x4bb) {
		return leggie_block28[codepoint - 0x4ba];
	} else if (codepoint >= 0x4c0 && codepoint <= 0x4c0) {
		return leggie_block29[codepoint - 0x4c0];
	} else if (codepoint >= 0x4c5 && codepoint <= 0x4ca) {
		return leggie_block30[codepoint - 0x4c5];
	} else if (codepoint >= 0x4cd && codepoint <= 0x4d9) {
		return leggie_block31[codepoint - 0x4cd];
	} else if (codepoint >= 0x4e2 && codepoint <= 0x4e3) {
		return leggie_block32[codepoint - 0x4e2];
	} else if (codepoint >= 0x4e6 && codepoint <= 0x4e9) {
		return leggie_block33[codepoint - 0x4e6];
	} else if (codepoint >= 0x4ec && codepoint <= 0x4f3) {
		return leggie_block34[codepoint - 0x4ec];
	} else if (codepoint >= 0x4f8 && codepoint <= 0x4f9) {
		return leggie_block35[codepoint - 0x4f8];
	} else if (codepoint >= 0x531 && codepoint <= 0x556) {
		return leggie_block36[codepoint - 0x531];
	} else if (codepoint >= 0x559 && codepoint <= 0x55f) {
		return leggie_block37[codepoint - 0x559];
	} else if (codepoint >= 0x561 && codepoint <= 0x587) {
		return leggie_block38[codepoint - 0x561];
	} else if (codepoint >= 0x589 && codepoint <= 0x58a) {
		return leggie_block39[codepoint - 0x589];
	} else if (codepoint >= 0x58d && codepoint <= 0x58f) {
		return leggie_block40[codepoint - 0x58d];
	} else if (codepoint >= 0x5d0 && codepoint <= 0x5ea) {
		return leggie_block41[codepoint - 0x5d0];
	} else if (codepoint >= 0xca0 && codepoint <= 0xca0) {
		return leggie_block42[codepoint - 0xca0];
	} else if (codepoint >= 0x10d0 && codepoint <= 0x10f0) {
		return leggie_block43[codepoint - 0x10d0];
	} else if (codepoint >= 0x10f6 && codepoint <= 0x10ff) {
		return leggie_block44[codepoint - 0x10f6];
	} else if (codepoint >= 0x1e02 && codepoint <= 0x1e03) {
		return leggie_block45[codepoint - 0x1e02];
	} else if (codepoint >= 0x1e0a && codepoint <= 0x1e0b) {
		return leggie_block46[codepoint - 0x1e0a];
	} else if (codepoint >= 0x1e1e && codepoint <= 0x1e1f) {
		return leggie_block47[codepoint - 0x1e1e];
	} else if (codepoint >= 0x1e24 && codepoint <= 0x1e25) {
		return leggie_block48[codepoint - 0x1e24];
	} else if (codepoint >= 0x1e36 && codepoint <= 0x1e37) {
		return leggie_block49[codepoint - 0x1e36];
	} else if (codepoint >= 0x1e40 && codepoint <= 0x1e41) {
		return leggie_block50[codepoint - 0x1e40];
	} else if (codepoint >= 0x1e56 && codepoint <= 0x1e57) {
		return leggie_block51[codepoint - 0x1e56];
	} else if (codepoint >= 0x1e60 && codepoint <= 0x1e61) {
		return leggie_block52[codepoint - 0x1e60];
	} else if (codepoint >= 0x1e6a && codepoint <= 0x1e6b) {
		return leggie_block53[codepoint - 0x1e6a];
	} else if (codepoint >= 0x1e80 && codepoint <= 0x1e85) {
		return leggie_block54[codepoint - 0x1e80];
	} else if (codepoint >= 0x1e8a && codepoint <= 0x1e8b) {
		return leggie_block55[codepoint - 0x1e8a];
	} else if (codepoint >= 0x1ea0 && codepoint <= 0x1ef9) {
		return leggie_block56[codepoint - 0x1ea0];
	} else if (codepoint >= 0x2010 && codepoint <= 0x2027) {
		return leggie_block57[codepoint - 0x2010];
	} else if (codepoint >= 0x2030 && codepoint <= 0x203a) {
		return leggie_block58[codepoint - 0x2030];
	} else if (codepoint >= 0x203c && codepoint <= 0x205e) {
		return leggie_block59[codepoint - 0x203c];
	} else if (codepoint >= 0x2061 && codepoint <= 0x2064) {
		return leggie_block60[codepoint - 0x2061];
	} else if (codepoint >= 0x2070 && codepoint <= 0x2071) {
		return leggie_block61[codepoint - 0x2070];
	} else if (codepoint >= 0x2074 && codepoint <= 0x208e) {
		return leggie_block62[codepoint - 0x2074];
	} else if (codepoint >= 0x2090 && codepoint <= 0x209c) {
		return leggie_block63[codepoint - 0x2090];
	} else if (codepoint >= 0x20a1 && codepoint <= 0x20a1) {
		return leggie_block64[codepoint - 0x20a1];
	} else if (codepoint >= 0x20a5 && codepoint <= 0x20af) {
		return leggie_block65[codepoint - 0x20a5];
	} else if (codepoint >= 0x20b1 && codepoint <= 0x20b2) {
		return leggie_block66[codepoint - 0x20b1];
	} else if (codepoint >= 0x20b4 && codepoint <= 0x20b5) {
		return leggie_block67[codepoint - 0x20b4];
	} else if (codepoint >= 0x20b8 && codepoint <= 0x20ba) {
		return leggie_block68[codepoint - 0x20b8];
	} else if (codepoint >= 0x20bc && codepoint <= 0x20bd) {
		return leggie_block69[codepoint - 0x20bc];
	} else if (codepoint >= 0x2116 && codepoint <= 0x2116) {
		return leggie_block70[codepoint - 0x2116];
	} else if (codepoint >= 0x2122 && codepoint <= 0x2122) {
		return leggie_block71[codepoint - 0x2122];
	} else if (codepoint >= 0x212b && codepoint <= 0x212b) {
		return leggie_block72[codepoint - 0x212b];
	} else if (codepoint >= 0x2190 && codepoint <= 0x2196) {
		return leggie_block73[codepoint - 0x2190];
	} else if (codepoint >= 0x2198 && codepoint <= 0x2198) {
		return leggie_block74[codepoint - 0x2198];
	} else if (codepoint >= 0x21a4 && codepoint <= 0x21a4) {
		return leggie_block75[codepoint - 0x21a4];
	} else if (codepoint >= 0x21a6 && codepoint <= 0x21a6) {
		return leggie_block76[codepoint - 0x21a6];
	} else if (codepoint >= 0x21a8 && codepoint <= 0x21a9) {
		return leggie_block77[codepoint - 0x21a8];
	} else if (codepoint >= 0x21b5 && codepoint <= 0x21b5) {
		return leggie_block78[codepoint - 0x21b5];
	} else if (codepoint >= 0x21b8 && codepoint <= 0x21b9) {
		return leggie_block79[codepoint - 0x21b8];
	} else if (codepoint >= 0x21c6 && codepoint <= 0x21c6) {
		return leggie_block80[codepoint - 0x21c6];
	} else if (codepoint >= 0x21d0 && codepoint <= 0x21d5) {
		return leggie_block81[codepoint - 0x21d0];
	} else if (codepoint >= 0x21de && codepoint <= 0x21df) {
		return leggie_block82[codepoint - 0x21de];
	} else if (codepoint >= 0x21e4 && codepoint <= 0x21e5) {
		return leggie_block83[codepoint - 0x21e4];
	} else if (codepoint >= 0x21e7 && codepoint <= 0x21e7) {
		return leggie_block84[codepoint - 0x21e7];
	} else if (codepoint >= 0x21ea && codepoint <= 0x21ea) {
		return leggie_block85[codepoint - 0x21ea];
	} else if (codepoint >= 0x21f1 && codepoint <= 0x21f2) {
		return leggie_block86[codepoint - 0x21f1];
	} else if (codepoint >= 0x2203 && codepoint <= 0x2203) {
		return leggie_block87[codepoint - 0x2203];
	} else if (codepoint >= 0x2205 && codepoint <= 0x2205) {
		return leggie_block88[codepoint - 0x2205];
	} else if (codepoint >= 0x2208 && codepoint <= 0x2208) {
		return leggie_block89[codepoint - 0x2208];
	} else if (codepoint >= 0x2219 && codepoint <= 0x221a) {
		return leggie_block90[codepoint - 0x2219];
	} else if (codepoint >= 0x221e && codepoint <= 0x221f) {
		return leggie_block91[codepoint - 0x221e];
	} else if (codepoint >= 0x2227 && codepoint <= 0x222a) {
		return leggie_block92[codepoint - 0x2227];
	} else if (codepoint >= 0x2248 && codepoint <= 0x2248) {
		return leggie_block93[codepoint - 0x2248];
	} else if (codepoint >= 0x2260 && codepoint <= 0x2261) {
		return leggie_block94[codepoint - 0x2260];
	} else if (codepoint >= 0x2264 && codepoint <= 0x2265) {
		return leggie_block95[codepoint - 0x2264];
	} else if (codepoint >= 0x2296 && codepoint <= 0x2297) {
		return leggie_block96[codepoint - 0x2296];
	} else if (codepoint >= 0x229d && codepoint <= 0x229d) {
		return leggie_block97[codepoint - 0x229d];
	} else if (codepoint >= 0x2302 && codepoint <= 0x2303) {
		return leggie_block98[codepoint - 0x2302];
	} else if (codepoint >= 0x2305 && codepoint <= 0x2305) {
		return leggie_block99[codepoint - 0x2305];
	} else if (codepoint >= 0x2310 && codepoint <= 0x2310) {
		return leggie_block100[codepoint - 0x2310];
	} else if (codepoint >= 0x2318 && codepoint <= 0x2318) {
		return leggie_block101[codepoint - 0x2318];
	} else if (codepoint >= 0x2320 && codepoint <= 0x2321) {
		return leggie_block102[codepoint - 0x2320];
	} else if (codepoint >= 0x2324 && codepoint <= 0x2328) {
		return leggie_block103[codepoint - 0x2324];
	} else if (codepoint >= 0x232b && codepoint <= 0x232b) {
		return leggie_block104[codepoint - 0x232b];
	} else if (codepoint >= 0x233d && codepoint <= 0x233d) {
		return leggie_block105[codepoint - 0x233d];
	} else if (codepoint >= 0x2380 && codepoint <= 0x2380) {
		return leggie_block106[codepoint - 0x2380];
	} else if (codepoint >= 0x2384 && codepoint <= 0x2384) {
		return leggie_block107[codepoint - 0x2384];
	} else if (codepoint >= 0x2386 && codepoint <= 0x2388) {
		return leggie_block108[codepoint - 0x2386];
	} else if (codepoint >= 0x238b && codepoint <= 0x238b) {
		return leggie_block109[codepoint - 0x238b];
	} else if (codepoint >= 0x23ba && codepoint <= 0x23bd) {
		return leggie_block110[codepoint - 0x23ba];
	} else if (codepoint >= 0x23ce && codepoint <= 0x23cf) {
		return leggie_block111[codepoint - 0x23ce];
	} else if (codepoint >= 0x2400 && codepoint <= 0x2426) {
		return leggie_block112[codepoint - 0x2400];
	} else if (codepoint >= 0x2500 && codepoint <= 0x2500) {
		return leggie_block113[codepoint - 0x2500];
	} else if (codepoint >= 0x2502 && codepoint <= 0x2502) {
		return leggie_block114[codepoint - 0x2502];
	} else if (codepoint >= 0x250c && codepoint <= 0x250c) {
		return leggie_block115[codepoint - 0x250c];
	} else if (codepoint >= 0x2510 && codepoint <= 0x2510) {
		return leggie_block116[codepoint - 0x2510];
	} else if (codepoint >= 0x2514 && codepoint <= 0x2514) {
		return leggie_block117[codepoint - 0x2514];
	} else if (codepoint >= 0x2518 && codepoint <= 0x2518) {
		return leggie_block118[codepoint - 0x2518];
	} else if (codepoint >= 0x251c && codepoint <= 0x251c) {
		return leggie_block119[codepoint - 0x251c];
	} else if (codepoint >= 0x2524 && codepoint <= 0x2524) {
		return leggie_block120[codepoint - 0x2524];
	} else if (codepoint >= 0x252c && codepoint <= 0x252c) {
		return leggie_block121[codepoint - 0x252c];
	} else if (codepoint >= 0x2534 && codepoint <= 0x2534) {
		return leggie_block122[codepoint - 0x2534];
	} else if (codepoint >= 0x253c && codepoint <= 0x253c) {
		return leggie_block123[codepoint - 0x253c];
	} else if (codepoint >= 0x2550 && codepoint <= 0x256c) {
		return leggie_block124[codepoint - 0x2550];
	} else if (codepoint >= 0x2580 && codepoint <= 0x2580) {
		return leggie_block125[codepoint - 0x2580];
	} else if (codepoint >= 0x2584 && codepoint <= 0x2584) {
		return leggie_block126[codepoint - 0x2584];
	} else if (codepoint >= 0x2588 && codepoint <= 0x2588) {
		return leggie_block127[codepoint - 0x2588];
	} else if (codepoint >= 0x258c && codepoint <= 0x258c) {
		return leggie_block128[codepoint - 0x258c];
	} else if (codepoint >= 0x2590 && codepoint <= 0x2593) {
		return leggie_block129[codepoint - 0x2590];
	} else if (codepoint >= 0x25a0 && codepoint <= 0x25a1) {
		return leggie_block130[codepoint - 0x25a0];
	} else if (codepoint >= 0x25a4 && codepoint <= 0x25a4) {
		return leggie_block131[codepoint - 0x25a4];
	} else if (codepoint >= 0x25aa && codepoint <= 0x25ac) {
		return leggie_block132[codepoint - 0x25aa];
	} else if (codepoint >= 0x25b2 && codepoint <= 0x25b2) {
		return leggie_block133[codepoint - 0x25b2];
	} else if (codepoint >= 0x25b6 && codepoint <= 0x25b8) {
		return leggie_block134[codepoint - 0x25b6];
	} else if (codepoint >= 0x25ba && codepoint <= 0x25ba) {
		return leggie_block135[codepoint - 0x25ba];
	} else if (codepoint >= 0x25bc && codepoint <= 0x25bc) {
		return leggie_block136[codepoint - 0x25bc];
	} else if (codepoint >= 0x25c1 && codepoint <= 0x25c1) {
		return leggie_block137[codepoint - 0x25c1];
	} else if (codepoint >= 0x25c4 && codepoint <= 0x25c4) {
		return leggie_block138[codepoint - 0x25c4];
	} else if (codepoint >= 0x25c6 && codepoint <= 0x25c7) {
		return leggie_block139[codepoint - 0x25c6];
	} else if (codepoint >= 0x25ca && codepoint <= 0x25cb) {
		return leggie_block140[codepoint - 0x25ca];
	} else if (codepoint >= 0x25cf && codepoint <= 0x25cf) {
		return leggie_block141[codepoint - 0x25cf];
	} else if (codepoint >= 0x25d8 && codepoint <= 0x25d9) {
		return leggie_block142[codepoint - 0x25d8];
	} else if (codepoint >= 0x25ef && codepoint <= 0x25ef) {
		return leggie_block143[codepoint - 0x25ef];
	} else if (codepoint >= 0x2610 && codepoint <= 0x2612) {
		return leggie_block144[codepoint - 0x2610];
	} else if (codepoint >= 0x263a && codepoint <= 0x263c) {
		return leggie_block145[codepoint - 0x263a];
	} else if (codepoint >= 0x2640 && codepoint <= 0x2640) {
		return leggie_block146[codepoint - 0x2640];
	} else if (codepoint >= 0x2642 && codepoint <= 0x2642) {
		return leggie_block147[codepoint - 0x2642];
	} else if (codepoint >= 0x2660 && codepoint <= 0x2660) {
		return leggie_block148[codepoint - 0x2660];
	} else if (codepoint >= 0x2663 && codepoint <= 0x2663) {
		return leggie_block149[codepoint - 0x2663];
	} else if (codepoint >= 0x2665 && codepoint <= 0x2666) {
		return leggie_block150[codepoint - 0x2665];
	} else if (codepoint >= 0x266a && codepoint <= 0x266b) {
		return leggie_block151[codepoint - 0x266a];
	} else if (codepoint >= 0x2713 && codepoint <= 0x2713) {
		return leggie_block152[codepoint - 0x2713];
	} else if (codepoint >= 0x2717 && codepoint <= 0x2717) {
		return leggie_block153[codepoint - 0x2717];
	} else if (codepoint >= 0x2726 && codepoint <= 0x2727) {
		return leggie_block154[codepoint - 0x2726];
	} else if (codepoint >= 0x2732 && codepoint <= 0x2732) {
		return leggie_block155[codepoint - 0x2732];
	} else if (codepoint >= 0x2756 && codepoint <= 0x2756) {
		return leggie_block156[codepoint - 0x2756];
	} else if (codepoint >= 0x2800 && codepoint <= 0x28ff) {
		return leggie_block157[codepoint - 0x2800];
	} else if (codepoint >= 0xa640 && codepoint <= 0xa643) {
		return leggie_block158[codepoint - 0xa640];
	} else if (codepoint >= 0xa64a && codepoint <= 0xa64b) {
		return leggie_block159[codepoint - 0xa64a];
	} else if (codepoint >= 0xa650 && codepoint <= 0xa651) {
		return leggie_block160[codepoint - 0xa650];
	} else if (codepoint >= 0xa656 && codepoint <= 0xa657) {
		return leggie_block161[codepoint - 0xa656];
	} else if (codepoint >= 0xa790 && codepoint <= 0xa791) {
		return leggie_block162[codepoint - 0xa790];
	} else if (codepoint >= 0xe000 && codepoint <= 0xe005) {
		return leggie_block163[codepoint - 0xe000];
	} else if (codepoint >= 0xe010 && codepoint <= 0xe01a) {
		return leggie_block164[codepoint - 0xe010];
	} else if (codepoint >= 0xe020 && codepoint <= 0xe025) {
		return leggie_block165[codepoint - 0xe020];
	} else if (codepoint >= 0xe030 && codepoint <= 0xe039) {
		return leggie_block166[codepoint - 0xe030];
	} else if (codepoint >= 0xe0a0 && codepoint <= 0xe0a2) {
		return leggie_block167[codepoint - 0xe0a0];
	} else if (codepoint >= 0xe0b0 && codepoint <= 0xe0b3) {
		return leggie_block168[codepoint - 0xe0b0];
	} else if (codepoint >= 0xf000 && codepoint <= 0xf002) {
		return leggie_block169[codepoint - 0xf000];
	} else if (codepoint >= 0xf800 && codepoint <= 0xf803) {
		return leggie_block170[codepoint - 0xf800];
	} else if (codepoint >= 0xf810 && codepoint <= 0xf813) {
		return leggie_block171[codepoint - 0xf810];
	} else if (codepoint >= 0xf8ff && codepoint <= 0xf8ff) {
		return leggie_block172[codepoint - 0xf8ff];
	} else if (codepoint >= 0xfb00 && codepoint <= 0xfb06) {
		return leggie_block173[codepoint - 0xfb00];
	} else if (codepoint >= 0xfb13 && codepoint <= 0xfb17) {
		return leggie_block174[codepoint - 0xfb13];
	} else if (codepoint >= 0xfe50 && codepoint <= 0xfe52) {
		return leggie_block175[codepoint - 0xfe50];
	} else if (codepoint >= 0xfe54 && codepoint <= 0xfe66) {
		return leggie_block176[codepoint - 0xfe54];
	} else if (codepoint >= 0xfe68 && codepoint <= 0xfe6b) {
		return leggie_block177[codepoint - 0xfe68];
	} else if (codepoint >= 0xfffd && codepoint <= 0xfffd) {
		return leggie_block178[codepoint - 0xfffd];
	} else if (codepoint >= 0xffff && codepoint <= 0xffff) {
		return leggie_block179[codepoint - 0xffff];
	} else {
		WARN("Codepoint U+%04X is not covered by this font", codepoint);
		return leggie_block1[0];
	}
}

static const unsigned char*
    veggie_get_bitmap(uint32_t codepoint)
{
	if (codepoint >= 0x20 && codepoint <= 0x7e) {
		return veggie_block1[codepoint - 0x20];
	} else if (codepoint >= 0xa0 && codepoint <= 0x113) {
		return veggie_block2[codepoint - 0xa0];
	} else if (codepoint >= 0x116 && codepoint <= 0x12b) {
		return veggie_block3[codepoint - 0x116];
	} else if (codepoint >= 0x12e && codepoint <= 0x131) {
		return veggie_block4[codepoint - 0x12e];
	} else if (codepoint >= 0x134 && codepoint <= 0x13e) {
		return veggie_block5[codepoint - 0x134];
	} else if (codepoint >= 0x141 && codepoint <= 0x148) {
		return veggie_block6[codepoint - 0x141];
	} else if (codepoint >= 0x14a && codepoint <= 0x14d) {
		return veggie_block7[codepoint - 0x14a];
	} else if (codepoint >= 0x150 && codepoint <= 0x17e) {
		return veggie_block8[codepoint - 0x150];
	} else if (codepoint >= 0x192 && codepoint <= 0x192) {
		return veggie_block9[codepoint - 0x192];
	} else if (codepoint >= 0x218 && codepoint <= 0x21b) {
		return veggie_block10[codepoint - 0x218];
	} else if (codepoint >= 0x2c7 && codepoint <= 0x2c7) {
		return veggie_block11[codepoint - 0x2c7];
	} else if (codepoint >= 0x2d8 && codepoint <= 0x2d9) {
		return veggie_block12[codepoint - 0x2d8];
	} else if (codepoint >= 0x2db && codepoint <= 0x2db) {
		return veggie_block13[codepoint - 0x2db];
	} else if (codepoint >= 0x2dd && codepoint <= 0x2dd) {
		return veggie_block14[codepoint - 0x2dd];
	} else if (codepoint >= 0x37a && codepoint <= 0x37a) {
		return veggie_block15[codepoint - 0x37a];
	} else if (codepoint >= 0x37e && codepoint <= 0x37e) {
		return veggie_block16[codepoint - 0x37e];
	} else if (codepoint >= 0x384 && codepoint <= 0x386) {
		return veggie_block17[codepoint - 0x384];
	} else if (codepoint >= 0x388 && codepoint <= 0x38a) {
		return veggie_block18[codepoint - 0x388];
	} else if (codepoint >= 0x38c && codepoint <= 0x38c) {
		return veggie_block19[codepoint - 0x38c];
	} else if (codepoint >= 0x38e && codepoint <= 0x3a1) {
		return veggie_block20[codepoint - 0x38e];
	} else if (codepoint >= 0x3a3 && codepoint <= 0x3ce) {
		return veggie_block21[codepoint - 0x3a3];
	} else if (codepoint >= 0x401 && codepoint <= 0x44f) {
		return veggie_block22[codepoint - 0x401];
	} else if (codepoint >= 0x451 && codepoint <= 0x45f) {
		return veggie_block23[codepoint - 0x451];
	} else if (codepoint >= 0x490 && codepoint <= 0x491) {
		return veggie_block24[codepoint - 0x490];
	} else if (codepoint >= 0x1e02 && codepoint <= 0x1e03) {
		return veggie_block25[codepoint - 0x1e02];
	} else if (codepoint >= 0x1e0a && codepoint <= 0x1e0b) {
		return veggie_block26[codepoint - 0x1e0a];
	} else if (codepoint >= 0x1e1e && codepoint <= 0x1e1f) {
		return veggie_block27[codepoint - 0x1e1e];
	} else if (codepoint >= 0x1e40 && codepoint <= 0x1e41) {
		return veggie_block28[codepoint - 0x1e40];
	} else if (codepoint >= 0x1e56 && codepoint <= 0x1e57) {
		return veggie_block29[codepoint - 0x1e56];
	} else if (codepoint >= 0x1e60 && codepoint <= 0x1e61) {
		return veggie_block30[codepoint - 0x1e60];
	} else if (codepoint >= 0x1e6a && codepoint <= 0x1e6b) {
		return veggie_block31[codepoint - 0x1e6a];
	} else if (codepoint >= 0x1e80 && codepoint <= 0x1e85) {
		return veggie_block32[codepoint - 0x1e80];
	} else if (codepoint >= 0x1ef2 && codepoint <= 0x1ef3) {
		return veggie_block33[codepoint - 0x1ef2];
	} else if (codepoint >= 0x2015 && codepoint <= 0x2015) {
		return veggie_block34[codepoint - 0x2015];
	} else if (codepoint >= 0x2018 && codepoint <= 0x2019) {
		return veggie_block35[codepoint - 0x2018];
	} else if (codepoint >= 0x201b && codepoint <= 0x2022) {
		return veggie_block36[codepoint - 0x201b];
	} else if (codepoint >= 0x2026 && codepoint <= 0x2026) {
		return veggie_block37[codepoint - 0x2026];
	} else if (codepoint >= 0x2030 && codepoint <= 0x2030) {
		return veggie_block38[codepoint - 0x2030];
	} else if (codepoint >= 0x203c && codepoint <= 0x203d) {
		return veggie_block39[codepoint - 0x203c];
	} else if (codepoint >= 0x207f && codepoint <= 0x207f) {
		return veggie_block40[codepoint - 0x207f];
	} else if (codepoint >= 0x20a7 && codepoint <= 0x20a7) {
		return veggie_block41[codepoint - 0x20a7];
	} else if (codepoint >= 0x20ac && codepoint <= 0x20ac) {
		return veggie_block42[codepoint - 0x20ac];
	} else if (codepoint >= 0x20af && codepoint <= 0x20af) {
		return veggie_block43[codepoint - 0x20af];
	} else if (codepoint >= 0x2116 && codepoint <= 0x2116) {
		return veggie_block44[codepoint - 0x2116];
	} else if (codepoint >= 0x2122 && codepoint <= 0x2122) {
		return veggie_block45[codepoint - 0x2122];
	} else if (codepoint >= 0x2190 && codepoint <= 0x2195) {
		return veggie_block46[codepoint - 0x2190];
	} else if (codepoint >= 0x21a8 && codepoint <= 0x21a8) {
		return veggie_block47[codepoint - 0x21a8];
	} else if (codepoint >= 0x21b5 && codepoint <= 0x21b5) {
		return veggie_block48[codepoint - 0x21b5];
	} else if (codepoint >= 0x2219 && codepoint <= 0x221a) {
		return veggie_block49[codepoint - 0x2219];
	} else if (codepoint >= 0x221e && codepoint <= 0x221f) {
		return veggie_block50[codepoint - 0x221e];
	} else if (codepoint >= 0x2229 && codepoint <= 0x2229) {
		return veggie_block51[codepoint - 0x2229];
	} else if (codepoint >= 0x2248 && codepoint <= 0x2248) {
		return veggie_block52[codepoint - 0x2248];
	} else if (codepoint >= 0x2260 && codepoint <= 0x2261) {
		return veggie_block53[codepoint - 0x2260];
	} else if (codepoint >= 0x2264 && codepoint <= 0x2265) {
		return veggie_block54[codepoint - 0x2264];
	} else if (codepoint >= 0x2302 && codepoint <= 0x2302) {
		return veggie_block55[codepoint - 0x2302];
	} else if (codepoint >= 0x2310 && codepoint <= 0x2310) {
		return veggie_block56[codepoint - 0x2310];
	} else if (codepoint >= 0x2320 && codepoint <= 0x2321) {
		return veggie_block57[codepoint - 0x2320];
	} else if (codepoint >= 0x2500 && codepoint <= 0x2500) {
		return veggie_block58[codepoint - 0x2500];
	} else if (codepoint >= 0x2502 && codepoint <= 0x2502) {
		return veggie_block59[codepoint - 0x2502];
	} else if (codepoint >= 0x250c && codepoint <= 0x250c) {
		return veggie_block60[codepoint - 0x250c];
	} else if (codepoint >= 0x2510 && codepoint <= 0x2510) {
		return veggie_block61[codepoint - 0x2510];
	} else if (codepoint >= 0x2514 && codepoint <= 0x2514) {
		return veggie_block62[codepoint - 0x2514];
	} else if (codepoint >= 0x2518 && codepoint <= 0x2518) {
		return veggie_block63[codepoint - 0x2518];
	} else if (codepoint >= 0x251c && codepoint <= 0x251c) {
		return veggie_block64[codepoint - 0x251c];
	} else if (codepoint >= 0x2524 && codepoint <= 0x2524) {
		return veggie_block65[codepoint - 0x2524];
	} else if (codepoint >= 0x252c && codepoint <= 0x252c) {
		return veggie_block66[codepoint - 0x252c];
	} else if (codepoint >= 0x2534 && codepoint <= 0x2534) {
		return veggie_block67[codepoint - 0x2534];
	} else if (codepoint >= 0x253c && codepoint <= 0x253c) {
		return veggie_block68[codepoint - 0x253c];
	} else if (codepoint >= 0x2550 && codepoint <= 0x256c) {
		return veggie_block69[codepoint - 0x2550];
	} else if (codepoint >= 0x2580 && codepoint <= 0x2580) {
		return veggie_block70[codepoint - 0x2580];
	} else if (codepoint >= 0x2584 && codepoint <= 0x2584) {
		return veggie_block71[codepoint - 0x2584];
	} else if (codepoint >= 0x2588 && codepoint <= 0x2588) {
		return veggie_block72[codepoint - 0x2588];
	} else if (codepoint >= 0x258c && codepoint <= 0x258c) {
		return veggie_block73[codepoint - 0x258c];
	} else if (codepoint >= 0x2590 && codepoint <= 0x2593) {
		return veggie_block74[codepoint - 0x2590];
	} else if (codepoint >= 0x25a0 && codepoint <= 0x25a0) {
		return veggie_block75[codepoint - 0x25a0];
	} else if (codepoint >= 0x25ac && codepoint <= 0x25ac) {
		return veggie_block76[codepoint - 0x25ac];
	} else if (codepoint >= 0x25b2 && codepoint <= 0x25b2) {
		return veggie_block77[codepoint - 0x25b2];
	} else if (codepoint >= 0x25b6 && codepoint <= 0x25b6) {
		return veggie_block78[codepoint - 0x25b6];
	} else if (codepoint >= 0x25bc && codepoint <= 0x25bc) {
		return veggie_block79[codepoint - 0x25bc];
	} else if (codepoint >= 0x25c0 && codepoint <= 0x25c0) {
		return veggie_block80[codepoint - 0x25c0];
	} else if (codepoint >= 0x25cb && codepoint <= 0x25cb) {
		return veggie_block81[codepoint - 0x25cb];
	} else if (codepoint >= 0x25d8 && codepoint <= 0x25d9) {
		return veggie_block82[codepoint - 0x25d8];
	} else if (codepoint >= 0x263a && codepoint <= 0x263c) {
		return veggie_block83[codepoint - 0x263a];
	} else if (codepoint >= 0x2640 && codepoint <= 0x2640) {
		return veggie_block84[codepoint - 0x2640];
	} else if (codepoint >= 0x2642 && codepoint <= 0x2642) {
		return veggie_block85[codepoint - 0x2642];
	} else if (codepoint >= 0x2660 && codepoint <= 0x2660) {
		return veggie_block86[codepoint - 0x2660];
	} else if (codepoint >= 0x2663 && codepoint <= 0x2663) {
		return veggie_block87[codepoint - 0x2663];
	} else if (codepoint >= 0x2665 && codepoint <= 0x2666) {
		return veggie_block88[codepoint - 0x2665];
	} else if (codepoint >= 0x266a && codepoint <= 0x266b) {
		return veggie_block89[codepoint - 0x266a];
	} else if (codepoint >= 0xe0a0 && codepoint <= 0xe0a2) {
		return veggie_block90[codepoint - 0xe0a0];
	} else if (codepoint >= 0xe0b0 && codepoint <= 0xe0b3) {
		return veggie_block91[codepoint - 0xe0b0];
	} else if (codepoint >= 0xfffd && codepoint <= 0xfffd) {
		return veggie_block92[codepoint - 0xfffd];
	} else if (codepoint >= 0xffff && codepoint <= 0xffff) {
		return veggie_block93[codepoint - 0xffff];
	} else {
		WARN("Codepoint U+%04X is not covered by this font", codepoint);
		return veggie_block1[0];
	}
}
