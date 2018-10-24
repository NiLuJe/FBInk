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

#ifndef __FBINK_DEVICE_ID_H
#define __FBINK_DEVICE_ID_H

// Mainly to make IDEs happy
#include "fbink.h"
#include "fbink_internal.h"

#if defined(FBINK_FOR_KINDLE)
#	define KINDLE_SERIAL_NO_LENGTH 16

static bool     is_kindle_device(uint32_t, FBInkDeviceQuirks*);
static bool     is_kindle_device_new(uint32_t, FBInkDeviceQuirks*);
static uint32_t from_base(char*, uint8_t);
static void     identify_kindle(FBInkDeviceQuirks*);
#elif defined(FBINK_FOR_CERVANTES)
// NOTE: This is NTX's homegrown hardware tagging, c.f., arch/arm/mach-imx/ntx_hwconfig.h in a Kobo kernel, for instance
#	define HWCONFIG_DEVICE "/dev/mmcblk0"
#	define HWCONFIG_OFFSET (1024 * 512)
#	define HWCONFIG_MAGIC "HW CONFIG "
typedef struct __attribute__((__packed__))
{
#	pragma GCC diagnostic push
#	pragma GCC diagnostic ignored "-Wattributes"
	char    magic[10] __attribute__((nonstring));     // HWCONFIG_MAGIC (i.e., "HW CONFIG ")
	char    version[5] __attribute__((nonstring));    // In Kobo-land, up to "v3.0" on Mk7
#	pragma GCC diagnostic pop
	uint8_t len;    // Length (in bytes) of the full payload, header excluded (up to 69 on v3.0)
	// Header stops here, actual data follows
	uint8_t pcb_id;    // First field is the PCB ID, which dictates the device model, the only thing we care about ;)
} NTXHWConfig;
static void identify_cervantes(FBInkDeviceQuirks*);
#else
static void identify_kobo(FBInkDeviceQuirks*);
#endif

static void identify_device(FBInkDeviceQuirks*);

#endif
