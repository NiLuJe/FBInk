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
					fprintf(stdout, "%hhu", fbink_rota_native_to_canonical(rota));
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
						linuxfb_rotate_to_string(fbink_rota_native_to_canonical(rota)));
				}
				fprintf(stdout, "\n");
			default:
				// NOP
				break;
		}
	}

	return EXIT_SUCCESS;
}
