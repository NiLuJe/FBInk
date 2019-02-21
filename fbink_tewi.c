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

#include "fbink_tewi.h"

static const unsigned char*
    tewi_get_bitmap(uint32_t codepoint)
{
	if (codepoint >= 0x20 && codepoint <= 0x7e) {
		return tewi_block1[codepoint - 0x20];
	} else if (codepoint >= 0xa1 && codepoint <= 0xac) {
		return tewi_block2[codepoint - 0xa1];
	} else if (codepoint >= 0xae && codepoint <= 0x2bd) {
		return tewi_block3[codepoint - 0xae];
	} else if (codepoint >= 0x2c2 && codepoint <= 0x2cb) {
		return tewi_block4[codepoint - 0x2c2];
	} else if (codepoint >= 0x2d0 && codepoint <= 0x2d1) {
		return tewi_block5[codepoint - 0x2d0];
	} else if (codepoint >= 0x2d8 && codepoint <= 0x2de) {
		return tewi_block6[codepoint - 0x2d8];
	} else if (codepoint == 0x2e0) {
		return tewi_block7[0];
	} else if (codepoint == 0x2e4) {
		return tewi_block8[0];
	} else if (codepoint == 0x2ee) {
		return tewi_block9[0];
	} else if (codepoint >= 0x300 && codepoint <= 0x30e) {
		return tewi_block10[codepoint - 0x300];
	} else if (codepoint >= 0x310 && codepoint <= 0x315) {
		return tewi_block11[codepoint - 0x310];
	} else if (codepoint == 0x31a) {
		return tewi_block12[0];
	} else if (codepoint == 0x31c) {
		return tewi_block13[0];
	} else if (codepoint >= 0x31f && codepoint <= 0x320) {
		return tewi_block14[codepoint - 0x31f];
	} else if (codepoint >= 0x324 && codepoint <= 0x325) {
		return tewi_block15[codepoint - 0x324];
	} else if (codepoint == 0x32a) {
		return tewi_block16[0];
	} else if (codepoint == 0x32c) {
		return tewi_block17[0];
	} else if (codepoint == 0x330) {
		return tewi_block18[0];
	} else if (codepoint >= 0x339 && codepoint <= 0x33c) {
		return tewi_block19[codepoint - 0x339];
	} else if (codepoint >= 0x343 && codepoint <= 0x344) {
		return tewi_block20[codepoint - 0x343];
	} else if (codepoint == 0x35c) {
		return tewi_block21[0];
	} else if (codepoint == 0x361) {
		return tewi_block22[0];
	} else if (codepoint >= 0x370 && codepoint <= 0x377) {
		return tewi_block23[codepoint - 0x370];
	} else if (codepoint >= 0x37a && codepoint <= 0x37f) {
		return tewi_block24[codepoint - 0x37a];
	} else if (codepoint >= 0x384 && codepoint <= 0x38a) {
		return tewi_block25[codepoint - 0x384];
	} else if (codepoint == 0x38c) {
		return tewi_block26[0];
	} else if (codepoint >= 0x38e && codepoint <= 0x3a1) {
		return tewi_block27[codepoint - 0x38e];
	} else if (codepoint >= 0x3a3 && codepoint <= 0x486) {
		return tewi_block28[codepoint - 0x3a3];
	} else if (codepoint >= 0x48a && codepoint <= 0x4ff) {
		return tewi_block29[codepoint - 0x48a];
	} else if (codepoint == 0x5be) {
		return tewi_block30[0];
	} else if (codepoint == 0x5c0) {
		return tewi_block31[0];
	} else if (codepoint == 0x5c3) {
		return tewi_block32[0];
	} else if (codepoint == 0x5c6) {
		return tewi_block33[0];
	} else if (codepoint >= 0x5d0 && codepoint <= 0x5ef) {
		return tewi_block34[codepoint - 0x5d0];
	} else if (codepoint == 0xca0) {
		return tewi_block35[0];
	} else if (codepoint == 0xe3f) {
		return tewi_block36[0];
	} else if (codepoint == 0x16a0) {
		return tewi_block37[0];
	} else if (codepoint >= 0x16a2 && codepoint <= 0x16a3) {
		return tewi_block38[codepoint - 0x16a2];
	} else if (codepoint >= 0x16a5 && codepoint <= 0x16a6) {
		return tewi_block39[codepoint - 0x16a5];
	} else if (codepoint >= 0x16a8 && codepoint <= 0x16ac) {
		return tewi_block40[codepoint - 0x16a8];
	} else if (codepoint >= 0x1e00 && codepoint <= 0x1ea9) {
		return tewi_block41[codepoint - 0x1e00];
	} else if (codepoint >= 0x1eab && codepoint <= 0x1eb3) {
		return tewi_block42[codepoint - 0x1eab];
	} else if (codepoint >= 0x1eb5 && codepoint <= 0x1ec3) {
		return tewi_block43[codepoint - 0x1eb5];
	} else if (codepoint >= 0x1ec5 && codepoint <= 0x1f15) {
		return tewi_block44[codepoint - 0x1ec5];
	} else if (codepoint >= 0x1f18 && codepoint <= 0x1f1d) {
		return tewi_block45[codepoint - 0x1f18];
	} else if (codepoint >= 0x1f20 && codepoint <= 0x1f45) {
		return tewi_block46[codepoint - 0x1f20];
	} else if (codepoint >= 0x1f48 && codepoint <= 0x1f4d) {
		return tewi_block47[codepoint - 0x1f48];
	} else if (codepoint >= 0x1f50 && codepoint <= 0x1f57) {
		return tewi_block48[codepoint - 0x1f50];
	} else if (codepoint == 0x1f59) {
		return tewi_block49[0];
	} else if (codepoint == 0x1f5b) {
		return tewi_block50[0];
	} else if (codepoint == 0x1f5d) {
		return tewi_block51[0];
	} else if (codepoint >= 0x1f5f && codepoint <= 0x1f7d) {
		return tewi_block52[codepoint - 0x1f5f];
	} else if (codepoint >= 0x1f80 && codepoint <= 0x1fb4) {
		return tewi_block53[codepoint - 0x1f80];
	} else if (codepoint >= 0x1fb6 && codepoint <= 0x1fbf) {
		return tewi_block54[codepoint - 0x1fb6];
	} else if (codepoint >= 0x1fc2 && codepoint <= 0x1fc4) {
		return tewi_block55[codepoint - 0x1fc2];
	} else if (codepoint >= 0x1fc6 && codepoint <= 0x1fd3) {
		return tewi_block56[codepoint - 0x1fc6];
	} else if (codepoint >= 0x1fd6 && codepoint <= 0x1fdb) {
		return tewi_block57[codepoint - 0x1fd6];
	} else if (codepoint >= 0x1fdd && codepoint <= 0x1fef) {
		return tewi_block58[codepoint - 0x1fdd];
	} else if (codepoint >= 0x1ff2 && codepoint <= 0x1ff4) {
		return tewi_block59[codepoint - 0x1ff2];
	} else if (codepoint >= 0x1ff6 && codepoint <= 0x1ffe) {
		return tewi_block60[codepoint - 0x1ff6];
	} else if (codepoint == 0x2010) {
		return tewi_block61[0];
	} else if (codepoint >= 0x2012 && codepoint <= 0x2027) {
		return tewi_block62[codepoint - 0x2012];
	} else if (codepoint == 0x2030) {
		return tewi_block63[0];
	} else if (codepoint >= 0x2032 && codepoint <= 0x203a) {
		return tewi_block64[codepoint - 0x2032];
	} else if (codepoint >= 0x203c && codepoint <= 0x2056) {
		return tewi_block65[codepoint - 0x203c];
	} else if (codepoint >= 0x2058 && codepoint <= 0x205e) {
		return tewi_block66[codepoint - 0x2058];
	} else if (codepoint >= 0x2070 && codepoint <= 0x2071) {
		return tewi_block67[codepoint - 0x2070];
	} else if (codepoint >= 0x2074 && codepoint <= 0x208e) {
		return tewi_block68[codepoint - 0x2074];
	} else if (codepoint >= 0x2090 && codepoint <= 0x209c) {
		return tewi_block69[codepoint - 0x2090];
	} else if (codepoint >= 0x20a0 && codepoint <= 0x20a6) {
		return tewi_block70[codepoint - 0x20a0];
	} else if (codepoint >= 0x20a8 && codepoint <= 0x20af) {
		return tewi_block71[codepoint - 0x20a8];
	} else if (codepoint >= 0x20b1 && codepoint <= 0x20b3) {
		return tewi_block72[codepoint - 0x20b1];
	} else if (codepoint >= 0x20b5 && codepoint <= 0x20b6) {
		return tewi_block73[codepoint - 0x20b5];
	} else if (codepoint >= 0x20b8 && codepoint <= 0x20ba) {
		return tewi_block74[codepoint - 0x20b8];
	} else if (codepoint >= 0x20bc && codepoint <= 0x20bd) {
		return tewi_block75[codepoint - 0x20bc];
	} else if (codepoint >= 0x2100 && codepoint <= 0x210b) {
		return tewi_block76[codepoint - 0x2100];
	} else if (codepoint >= 0x2116 && codepoint <= 0x2117) {
		return tewi_block77[codepoint - 0x2116];
	} else if (codepoint == 0x2122) {
		return tewi_block78[0];
	} else if (codepoint >= 0x2125 && codepoint <= 0x2127) {
		return tewi_block79[codepoint - 0x2125];
	} else if (codepoint >= 0x212a && codepoint <= 0x212b) {
		return tewi_block80[codepoint - 0x212a];
	} else if (codepoint == 0x212f) {
		return tewi_block81[0];
	} else if (codepoint >= 0x2133 && codepoint <= 0x2134) {
		return tewi_block82[codepoint - 0x2133];
	} else if (codepoint >= 0x2160 && codepoint <= 0x2166) {
		return tewi_block83[codepoint - 0x2160];
	} else if (codepoint >= 0x2168 && codepoint <= 0x2176) {
		return tewi_block84[codepoint - 0x2168];
	} else if (codepoint >= 0x2178 && codepoint <= 0x2180) {
		return tewi_block85[codepoint - 0x2178];
	} else if (codepoint == 0x2183) {
		return tewi_block86[0];
	} else if (codepoint >= 0x2190 && codepoint <= 0x2193) {
		return tewi_block87[codepoint - 0x2190];
	} else if (codepoint >= 0x2195 && codepoint <= 0x21ac) {
		return tewi_block88[codepoint - 0x2195];
	} else if (codepoint >= 0x21af && codepoint <= 0x21cd) {
		return tewi_block89[codepoint - 0x21af];
	} else if (codepoint >= 0x21cf && codepoint <= 0x21d3) {
		return tewi_block90[codepoint - 0x21cf];
	} else if (codepoint >= 0x21d5 && codepoint <= 0x21ef) {
		return tewi_block91[codepoint - 0x21d5];
	} else if (codepoint >= 0x21f1 && codepoint <= 0x21f8) {
		return tewi_block92[codepoint - 0x21f1];
	} else if (codepoint == 0x21fa) {
		return tewi_block93[0];
	} else if (codepoint >= 0x21fd && codepoint <= 0x21fe) {
		return tewi_block94[codepoint - 0x21fd];
	} else if (codepoint >= 0x2200 && codepoint <= 0x222c) {
		return tewi_block95[codepoint - 0x2200];
	} else if (codepoint >= 0x222e && codepoint <= 0x222f) {
		return tewi_block96[codepoint - 0x222e];
	} else if (codepoint >= 0x2231 && codepoint <= 0x225c) {
		return tewi_block97[codepoint - 0x2231];
	} else if (codepoint >= 0x225e && codepoint <= 0x2299) {
		return tewi_block98[codepoint - 0x225e];
	} else if (codepoint == 0x229b) {
		return tewi_block99[0];
	} else if (codepoint >= 0x229d && codepoint <= 0x2320) {
		return tewi_block100[codepoint - 0x229d];
	} else if (codepoint >= 0x2322 && codepoint <= 0x2328) {
		return tewi_block101[codepoint - 0x2322];
	} else if (codepoint == 0x232b) {
		return tewi_block102[0];
	} else if (codepoint == 0x232d) {
		return tewi_block103[0];
	} else if (codepoint >= 0x232f && codepoint <= 0x237e) {
		return tewi_block104[codepoint - 0x232f];
	} else if (codepoint == 0x2380) {
		return tewi_block105[0];
	} else if (codepoint == 0x2388) {
		return tewi_block106[0];
	} else if (codepoint == 0x238b) {
		return tewi_block107[0];
	} else if (codepoint >= 0x23ba && codepoint <= 0x23bd) {
		return tewi_block108[codepoint - 0x23ba];
	} else if (codepoint >= 0x23e9 && codepoint <= 0x23ef) {
		return tewi_block109[codepoint - 0x23e9];
	} else if (codepoint >= 0x2408 && codepoint <= 0x240f) {
		return tewi_block110[codepoint - 0x2408];
	} else if (codepoint >= 0x2423 && codepoint <= 0x2426) {
		return tewi_block111[codepoint - 0x2423];
	} else if (codepoint >= 0x2440 && codepoint <= 0x244a) {
		return tewi_block112[codepoint - 0x2440];
	} else if (codepoint >= 0x2460 && codepoint <= 0x24b5) {
		return tewi_block113[codepoint - 0x2460];
	} else if (codepoint >= 0x2500 && codepoint <= 0x2609) {
		return tewi_block114[codepoint - 0x2500];
	} else if (codepoint >= 0x2610 && codepoint <= 0x2611) {
		return tewi_block115[codepoint - 0x2610];
	} else if (codepoint == 0x2614) {
		return tewi_block116[0];
	} else if (codepoint >= 0x2616 && codepoint <= 0x2617) {
		return tewi_block117[codepoint - 0x2616];
	} else if (codepoint == 0x261e) {
		return tewi_block118[0];
	} else if (codepoint >= 0x2630 && codepoint <= 0x2637) {
		return tewi_block119[codepoint - 0x2630];
	} else if (codepoint >= 0x263a && codepoint <= 0x263c) {
		return tewi_block120[codepoint - 0x263a];
	} else if (codepoint >= 0x263f && codepoint <= 0x2642) {
		return tewi_block121[codepoint - 0x263f];
	} else if (codepoint >= 0x2660 && codepoint <= 0x266f) {
		return tewi_block122[codepoint - 0x2660];
	} else if (codepoint >= 0x26a2 && codepoint <= 0x26a9) {
		return tewi_block123[codepoint - 0x26a2];
	} else if (codepoint == 0x2708) {
		return tewi_block124[0];
	} else if (codepoint >= 0x2713 && codepoint <= 0x2718) {
		return tewi_block125[codepoint - 0x2713];
	} else if (codepoint >= 0x271a && codepoint <= 0x2721) {
		return tewi_block126[codepoint - 0x271a];
	} else if (codepoint >= 0x2724 && codepoint <= 0x2727) {
		return tewi_block127[codepoint - 0x2724];
	} else if (codepoint >= 0x2729 && codepoint <= 0x272b) {
		return tewi_block128[codepoint - 0x2729];
	} else if (codepoint == 0x2733) {
		return tewi_block129[0];
	} else if (codepoint == 0x2736) {
		return tewi_block130[0];
	} else if (codepoint == 0x273f) {
		return tewi_block131[0];
	} else if (codepoint == 0x2741) {
		return tewi_block132[0];
	} else if (codepoint >= 0x274f && codepoint <= 0x2752) {
		return tewi_block133[codepoint - 0x274f];
	} else if (codepoint >= 0x275b && codepoint <= 0x2760) {
		return tewi_block134[codepoint - 0x275b];
	} else if (codepoint >= 0x2764 && codepoint <= 0x2766) {
		return tewi_block135[codepoint - 0x2764];
	} else if (codepoint >= 0x276c && codepoint <= 0x2771) {
		return tewi_block136[codepoint - 0x276c];
	} else if (codepoint >= 0x278a && codepoint <= 0x279b) {
		return tewi_block137[codepoint - 0x278a];
	} else if (codepoint >= 0x27e8 && codepoint <= 0x27e9) {
		return tewi_block138[codepoint - 0x27e8];
	} else if (codepoint >= 0x27f2 && codepoint <= 0x27f3) {
		return tewi_block139[codepoint - 0x27f2];
	} else if (codepoint >= 0x27f5 && codepoint <= 0x27f6) {
		return tewi_block140[codepoint - 0x27f5];
	} else if (codepoint >= 0x27f8 && codepoint <= 0x27f9) {
		return tewi_block141[codepoint - 0x27f8];
	} else if (codepoint >= 0x27fb && codepoint <= 0x28ff) {
		return tewi_block142[codepoint - 0x27fb];
	} else if (codepoint >= 0x2902 && codepoint <= 0x2903) {
		return tewi_block143[codepoint - 0x2902];
	} else if (codepoint >= 0x2906 && codepoint <= 0x2909) {
		return tewi_block144[codepoint - 0x2906];
	} else if (codepoint >= 0x290c && codepoint <= 0x290f) {
		return tewi_block145[codepoint - 0x290c];
	} else if (codepoint >= 0x2912 && codepoint <= 0x2913) {
		return tewi_block146[codepoint - 0x2912];
	} else if (codepoint >= 0x2919 && codepoint <= 0x291e) {
		return tewi_block147[codepoint - 0x2919];
	} else if (codepoint >= 0x2921 && codepoint <= 0x2926) {
		return tewi_block148[codepoint - 0x2921];
	} else if (codepoint >= 0x2933 && codepoint <= 0x2946) {
		return tewi_block149[codepoint - 0x2933];
	} else if (codepoint >= 0x2949 && codepoint <= 0x294d) {
		return tewi_block150[codepoint - 0x2949];
	} else if (codepoint == 0x294f) {
		return tewi_block151[0];
	} else if (codepoint >= 0x2951 && codepoint <= 0x2976) {
		return tewi_block152[codepoint - 0x2951];
	} else if (codepoint >= 0x2978 && codepoint <= 0x297f) {
		return tewi_block153[codepoint - 0x2978];
	} else if (codepoint >= 0x2b00 && codepoint <= 0x2b03) {
		return tewi_block154[codepoint - 0x2b00];
	} else if (codepoint >= 0x2b05 && codepoint <= 0x2b0b) {
		return tewi_block155[codepoint - 0x2b05];
	} else if (codepoint >= 0x2b0d && codepoint <= 0x2b32) {
		return tewi_block156[codepoint - 0x2b0d];
	} else if (codepoint == 0x2b38) {
		return tewi_block157[0];
	} else if (codepoint >= 0x2b3f && codepoint <= 0x2b55) {
		return tewi_block158[codepoint - 0x2b3f];
	} else if (codepoint == 0x2b58) {
		return tewi_block159[0];
	} else if (codepoint >= 0x2b5a && codepoint <= 0x2b63) {
		return tewi_block160[codepoint - 0x2b5a];
	} else if (codepoint >= 0x2b65 && codepoint <= 0x2b73) {
		return tewi_block161[codepoint - 0x2b65];
	} else if (codepoint >= 0x2b76 && codepoint <= 0x2b79) {
		return tewi_block162[codepoint - 0x2b76];
	} else if (codepoint == 0x2b7e) {
		return tewi_block163[0];
	} else if (codepoint == 0x2b80) {
		return tewi_block164[0];
	} else if (codepoint == 0x2b82) {
		return tewi_block165[0];
	} else if (codepoint >= 0x2b88 && codepoint <= 0x2b93) {
		return tewi_block166[codepoint - 0x2b88];
	} else if (codepoint == 0x2b95) {
		return tewi_block167[0];
	} else if (codepoint >= 0x2ba0 && codepoint <= 0x2baf) {
		return tewi_block168[codepoint - 0x2ba0];
	} else if (codepoint == 0x2bb8) {
		return tewi_block169[0];
	} else if (codepoint >= 0x2bc0 && codepoint <= 0x2bc2) {
		return tewi_block170[codepoint - 0x2bc0];
	} else if (codepoint >= 0x2bc5 && codepoint <= 0x2bc8) {
		return tewi_block171[codepoint - 0x2bc5];
	} else if (codepoint >= 0x2bca && codepoint <= 0x2bcf) {
		return tewi_block172[codepoint - 0x2bca];
	} else if (codepoint >= 0x2bec && codepoint <= 0x2bef) {
		return tewi_block173[codepoint - 0x2bec];
	} else if (codepoint >= 0x2c60 && codepoint <= 0x2c7f) {
		return tewi_block174[codepoint - 0x2c60];
	} else if (codepoint >= 0x2e00 && codepoint <= 0x2e0d) {
		return tewi_block175[codepoint - 0x2e00];
	} else if (codepoint >= 0x2e0f && codepoint <= 0x2e18) {
		return tewi_block176[codepoint - 0x2e0f];
	} else if (codepoint >= 0x2e1a && codepoint <= 0x2e40) {
		return tewi_block177[codepoint - 0x2e1a];
	} else if (codepoint >= 0xe0a0 && codepoint <= 0xe0a2) {
		return tewi_block178[codepoint - 0xe0a0];
	} else if (codepoint >= 0xe0a5 && codepoint <= 0xe0ac) {
		return tewi_block179[codepoint - 0xe0a5];
	} else if (codepoint >= 0xe0b0 && codepoint <= 0xe0b3) {
		return tewi_block180[codepoint - 0xe0b0];
	} else if (codepoint >= 0xe0b5 && codepoint <= 0xe0b8) {
		return tewi_block181[codepoint - 0xe0b5];
	} else if (codepoint >= 0xe0c0 && codepoint <= 0xe0c7) {
		return tewi_block182[codepoint - 0xe0c0];
	} else if (codepoint >= 0xfb01 && codepoint <= 0xfb02) {
		return tewi_block183[codepoint - 0xfb01];
	} else if (codepoint == 0xfffd) {
		return tewi_block184[0];
	} else {
		WARN("Codepoint U+%04X is not covered by this font", codepoint);
		return tewi_block1[0];
	}
}

static const unsigned char*
    tewib_get_bitmap(uint32_t codepoint)
{
	if (codepoint >= 0x20 && codepoint <= 0x7e) {
		return tewib_block1[codepoint - 0x20];
	} else if (codepoint >= 0xa0 && codepoint <= 0x131) {
		return tewib_block2[codepoint - 0xa0];
	} else if (codepoint >= 0x134 && codepoint <= 0x1b2) {
		return tewib_block3[codepoint - 0x134];
	} else {
		WARN("Codepoint U+%04X is not covered by this font", codepoint);
		return tewib_block1[0];
	}
}
