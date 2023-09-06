/*
	FBInk: FrameBuffer eInker, a library to print text & images to an eInk Linux framebuffer
	Copyright (C) 2023 NiLuJe <ninuje@gmail.com>
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

// Because we're pretty much Linux-bound ;).
#ifndef _GNU_SOURCE
#	define _GNU_SOURCE
#endif

#include <errno.h>
#include <linux/fb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// I feel dirty.
#include "../fbink.c"

// These are the old, non-LUT implementations. Here be dragons.
static uint8_t
    fbink_old_rota_native_to_canonical(uint32_t rotate UNUSED_BY_NOTKOBO)
{
#if defined(FBINK_FOR_KOBO)
	// NOTE: When we care about rotation at all,
	//       kobo_sunxi_fb_fixup ensures that the rotate flag returned by fbink_get_state
	//       is *already* canonical, so we don't have anything to do!
	//       That said, since on those platforms, native_portrait == 0,
	//       the whole logic below is still sound.
	if (deviceQuirks.ntxRotaQuirk == NTX_ROTA_SUNXI) {
		return (uint8_t) rotate;
	}

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
	} else if (deviceQuirks.ntxRotaQuirk == NTX_ROTA_CW_TOUCH || deviceQuirks.ntxRotaQuirk == NTX_ROTA_CCW_TOUCH) {
		// NOTE: NTX_ROTA_CW_TOUCH was originally for the Libra 2,
		//       whose Touch panel is mounted in the invert of the usual rotation (CW instead of CCW),
		//       which makes for a native -> canonical mapping of "1 -> UR (0); 2 -> CCW (3); 3 -> UD (2); 0 -> CW (1)".
		//       NTX_ROTA_CCW_TOUCH is for the Clara 2E (possibly because I got something wrong ^^).
		rota = (native_portrait ^ rotate) & 3;
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
static uint32_t
    fbink_old_rota_canonical_to_native(uint8_t rotate UNUSED_BY_NOTKOBO)
{
#if defined(FBINK_FOR_KOBO)
	// Same as above, nothing to do on those devices.
	if (deviceQuirks.ntxRotaQuirk == NTX_ROTA_SUNXI) {
		return (uint32_t) rotate;
	}

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
	} else if (deviceQuirks.ntxRotaQuirk == NTX_ROTA_CW_TOUCH || deviceQuirks.ntxRotaQuirk == NTX_ROTA_CCW_TOUCH) {
		// NOTE: native->canonical == canonical->native
		rota = (native_portrait ^ rotate) & 3;
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

static const char*
    linuxfb_rotate_to_string(uint32_t rotate)
{
	switch (rotate) {
		case FB_ROTATE_UR:
			return "FB_ROTATE_UR";
		case FB_ROTATE_CW:
			return "FB_ROTATE_CW";
		case FB_ROTATE_UD:
			return "FB_ROTATE_UD";
		case FB_ROTATE_CCW:
			return "FB_ROTATE_CCW";
		default:
			return "Unknown?!";
	}
}

int
    main(void)
{
	// Loop over every device... Enum is sparse, so, DIY...
	// NOTE: Make sure the upper bound is actually the highest *value*, because that's not necessarily the *latest* device ;).
	for (uint16_t id = DEVICE_KOBO_TOUCH_A; id <= DEVICE_KOBO_ELIPSA_2E; id++) {
		switch (id) {
			case DEVICE_KOBO_TOUCH_A:
			//case DEVICE_KOBO_TOUCH_B: // Tries to poke at the HWConfig block, and matches A for our purposes anyway.
			case DEVICE_KOBO_TOUCH_C:
			case DEVICE_KOBO_MINI:
			case DEVICE_KOBO_GLO:
			case DEVICE_KOBO_GLO_HD:
			case DEVICE_KOBO_TOUCH_2:
			case DEVICE_KOBO_AURA:
			case DEVICE_KOBO_AURA_HD:
			case DEVICE_KOBO_AURA_H2O:
			case DEVICE_KOBO_AURA_H2O_2:
			case DEVICE_KOBO_AURA_H2O_2_R2:
			case DEVICE_KOBO_AURA_ONE:
			case DEVICE_KOBO_AURA_ONE_LE:
			case DEVICE_KOBO_AURA_SE:
			case DEVICE_KOBO_AURA_SE_R2:
			case DEVICE_KOBO_CLARA_HD:
			case DEVICE_KOBO_FORMA:
			case DEVICE_KOBO_FORMA_32GB:
			case DEVICE_KOBO_LIBRA_H2O:
			case DEVICE_KOBO_NIA:
			case DEVICE_KOBO_ELIPSA:
			case DEVICE_KOBO_LIBRA_2:
			case DEVICE_KOBO_SAGE:
			case DEVICE_KOBO_CLARA_2E:
			case DEVICE_KOBO_ELIPSA_2E:
				// Print the Native -> Canonical mapping
				set_kobo_quirks(id);
				fprintf(stdout, "%s: {", deviceQuirks.deviceName);
				for (uint32_t rota = FB_ROTATE_UR; rota <= FB_ROTATE_CCW; rota++) {
					fprintf(stdout, "%hhu", fbink_old_rota_native_to_canonical(rota));
					if (rota != FB_ROTATE_CCW) {
						fprintf(stdout, ", ");
					}
				}
				fprintf(stdout, "}\n");

				// And in terms of code
				fprintf(stdout, "\n");
				for (uint32_t rota = FB_ROTATE_UR; rota <= FB_ROTATE_CCW; rota++) {
					fprintf(stdout,
						"deviceQuirks.rotationMap[%s]  = %s;\n",
						linuxfb_rotate_to_string(rota),
						linuxfb_rotate_to_string(fbink_old_rota_native_to_canonical(rota)));
				}
				fprintf(stdout, "\n");
			default:
				// NOP
				break;
		}
	}

	return EXIT_SUCCESS;
}
