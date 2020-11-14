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

#include "fbink_rota_quirks.h"

// Try to make sense out of the mess that are native Kobo rotations...
// Vaguely inspired by Plato's implementation,
// c.f., https://github.com/baskerville/plato/blob/f45c2da65bc556bc22d664b2f9450f95c550dbf5/src/device.rs#L265-L326
// except not really, because that didn't work at all on my quirky devices ;).
uint8_t
    fbink_rota_native_to_canonical(uint32_t rotate UNUSED_BY_NOTKOBO)
{
#if defined(FBINK_FOR_KOBO)
	uint8_t rota = (uint8_t) rotate;

	// First, we'll need to compute the native Portrait rotation
	uint8_t native_portrait = FB_ROTATE_UR;
	// NOTE: For *most* devices, Nickel's Portrait orientation should *always* match BootRota + 1
	//       Thankfully, the Libra appears to be ushering in a new era filled with puppies and rainbows,
	//       and, hopefully, less insane rotation quirks ;).
	if (deviceQuirks.ntxRotaQuirk != NTX_ROTA_SANE) {
		native_portrait = (deviceQuirks.ntxBootRota + 1) & 3;
	} else {
		native_portrait = deviceQuirks.ntxBootRota;
	}

	// Then, if the kernel happens to mangle rotations, we need to account for it, for *both* parties...
	// In this direction, the second party is the input (native) rotation.
	if (deviceQuirks.ntxRotaQuirk == NTX_ROTA_ALL_INVERTED) {
		// NOTE: This should cover the H2O and the few other devices suffering from the same quirk...
		native_portrait ^= 2;
		rotate ^= 2;
	} else if (deviceQuirks.ntxRotaQuirk == NTX_ROTA_ODD_INVERTED) {
		// NOTE: This is for the Forma, which only inverts CW & CCW (i.e., odd numbers)...
		if ((native_portrait & 0x01) == 1) {
			native_portrait ^= 2;
		}
		if ((rotate & 0x01) == 1) {
			rotate ^= 2;
		}
	}

	// Now that we know what the canonical Portrait should look like in native-speak, we should be able to compute the rest...
	if (deviceQuirks.ntxRotaQuirk == NTX_ROTA_ALL_INVERTED) {
		rota = (native_portrait - rotate) & 3;
		// NOTE: If we do NOT invert anything, this works, too:
		//       rota = (4 - (rotate - native_portrait)) & 3;
	} else {
		rota = (rotate - native_portrait) & 3;
	}

	return rota;
#else
	WARN("Rotation quirks are only handled on Kobo");
	return ENOSYS;
#endif
}

// NOTE: As far as NTX_ROTA_ALL_INVERTED is concerned, native->canonical == canonical->native ;).
//       No, don't ask me to explain why: I don't know. Remember, I'm severely maths-impaired.
uint32_t
    fbink_rota_canonical_to_native(uint8_t rotate UNUSED_BY_NOTKOBO)
{
#if defined(FBINK_FOR_KOBO)
	uint32_t rota = rotate;

	// First, we'll need to compute the native Portrait rotation
	uint8_t native_portrait = FB_ROTATE_UR;
	// NOTE: For *most* devices, Nickel's Portrait orientation should *always* match BootRota + 1
	//       Thankfully, the Libra appears to be ushering in a new era filled with puppies and rainbows,
	//       and, hopefully, less insane rotation quirks ;).
	if (deviceQuirks.ntxRotaQuirk != NTX_ROTA_SANE) {
		native_portrait = (deviceQuirks.ntxBootRota + 1) & 3;
	} else {
		native_portrait = deviceQuirks.ntxBootRota;
	}

	// Then, if the kernel happens to mangle rotations, we need to account for it, for *both* parties...
	if (deviceQuirks.ntxRotaQuirk == NTX_ROTA_ALL_INVERTED) {
		// NOTE: This should cover the H2O and the few other devices suffering from the same quirk...
		native_portrait ^= 2;
	} else if (deviceQuirks.ntxRotaQuirk == NTX_ROTA_ODD_INVERTED) {
		// NOTE: This is for the Forma, which only inverts CW & CCW (i.e., odd numbers)...
		if ((native_portrait & 0x01) == 1) {
			native_portrait ^= 2;
		}
	}

	// Now that we know what the canonical Portrait should look like in native-speak, we should be able to compute the rest...
	if (deviceQuirks.ntxRotaQuirk == NTX_ROTA_ALL_INVERTED) {
		rota = (native_portrait - rotate) & 3;
		// NOTE: If we do NOT invert native_portrait (but do invert the final result), this works, too:
		//       rota = (4 - (native_portrait + rotate)) & 3;
	} else {
		rota = (native_portrait + rotate) & 3;
	}

	// As mentioned earlier, we have to handle *both* parties, and in this direction, that's the final result.
	if (deviceQuirks.ntxRotaQuirk == NTX_ROTA_ALL_INVERTED) {
		rota ^= 2;
	} else if (deviceQuirks.ntxRotaQuirk == NTX_ROTA_ODD_INVERTED) {
		if ((rota & 0x01) == 1) {
			rota ^= 2;
		}
	}

	return rota;
#else
	WARN("Rotation quirks are only handled on Kobo");
	return ENOSYS;
#endif
}
