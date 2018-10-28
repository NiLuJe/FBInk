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

#ifndef FBINK_FOR_LINUX
#	ifndef FBINK_FOR_KINDLE
// NOTE: This is NTX's homegrown hardware tagging, c.f., arch/arm/mach-imx/ntx_hwconfig.h in a Kobo kernel, for instance
#		define HWCONFIG_DEVICE "/dev/mmcblk0"
#		define HWCONFIG_OFFSET (1024 * 512)
#		define HWCONFIG_MAGIC "HW CONFIG "
typedef struct __attribute__((__packed__))
{
#		pragma GCC diagnostic push
#		pragma GCC diagnostic ignored "-Wattributes"
	char magic[10] __attribute__((nonstring));     // HWCONFIG_MAGIC (i.e., "HW CONFIG ")
	char version[5] __attribute__((nonstring));    // In Kobo-land, up to "v3.1" on Mk7
#		pragma GCC diagnostic pop
	uint8_t len;    // Length (in bytes) of the full payload, header excluded (up to 70 on v3.1)
	// Header stops here, actual data follows
	uint8_t pcb_id;    // First field is the PCB ID, which dictates the device model, the only thing we care about ;)
} NTXHWConfig;
#	endif    // !FBINK_FOR_KINDLE

#	if defined(FBINK_FOR_KINDLE)
#		define KINDLE_SERIAL_NO_LENGTH 16
static bool     is_kindle_device(uint32_t, FBInkDeviceQuirks*);
static bool     is_kindle_device_new(uint32_t, FBInkDeviceQuirks*);
static uint32_t from_base(char*, uint8_t);
static void     identify_kindle(FBInkDeviceQuirks*);
#        elif defined(FBINK_FOR_CERVANTES)
static void identify_cervantes(FBInkDeviceQuirks*);
#        else
// List of NTX/Kobo PCB IDs... For a given device, what we get in NTXHWConfig.pcb_id corresponds to an index in this array.
// Can thankfully be populated from /bin/ntx_hwconfig with the help of strings and a bit of sed, i.e.,
// sed -re 's/(^)(.*?)($)/"\2",/g' PCB_IDs.txt
// NOTE: Last updated on 10/27/18, from FW 4.11.11911
/*
static const char* kobo_pcbs[] = { "E60800", "E60810", "E60820", "E90800",  "E90810",  "E60830",  "E60850", "E50800",
				   "E50810", "E60860", "E60MT2", "E60M10",  "E60610",  "E60M00",  "E60M30", "E60620",
				   "E60630", "E60640", "E50600", "E60680",  "E60610C", "E60610D", "E606A0", "E60670",
				   "E606B0", "E50620", "Q70Q00", "E50610",  "E606C0",  "E606D0",  "E606E0", "E60Q00",
				   "E60Q10", "E60Q20", "E606F0", "E606F0B", "E60Q30",  "E60QB0",  "E60QC0", "A13120",
				   "E60Q50", "E606G0", "E60Q60", "E60Q80",  "A13130",  "E606H2",  "E60Q90", "ED0Q00",
				   "E60QA0", "E60QD0", "E60QF0", "E60QH0",  "E60QG0",  "H70000",  "ED0Q10", "E70Q00",
				   "H40000", "E60QJ0", "E60QL0", "E60QM0",  "E60QK0",  "E70S00",  "T60Q00", "C31Q00",
				   "E60QN0", "E60U00", "E70Q10", "E60QP0",  "E60QQ0",  "E70Q20",  "T05R00", "M31Q00",
				   "E60U10", "E60K00", "E80K00", "E70Q30",  "EA0Q00",  "E60R00" };
*/
// And match (more or less accurately, for some devices) that to what we've come to know as a device code,
// because that's what we actually care about...
// c.f., tools/pcb_to_ids.py
static const unsigned short int kobo_ids[] = { 0, 0,   0,   0,   0,   0,   0, 0,   0,   0,   0,   0,   310, 0, 0,   0,
					       0, 0,   0,   0,   310, 310, 0, 0,   330, 0,   0,   340, 350, 0, 0,   0,
					       0, 0,   360, 360, 0,   330, 0, 0,   0,   370, 0,   0,   0,   0, 371, 0,
					       0, 0,   0,   0,   0,   0,   0, 373, 0,   0,   375, 374, 0,   0, 375, 0,
					       0, 375, 0,   0,   0,   0,   0, 0,   376, 376, 380, 0,   0,   0 };

// List of NTX/Kobo Display Resolutions...
// NOTE: If PCB is field 0, DisplayResolution is field 31
//       The good news is, we'd only need it to differentiate pika from alyssum (as per /bin/kobo_config.sh),
//       and it's a good news because we don't care about that distinction.
//       The bad news is, it's a field that was added in v1.0 of that whole NTXHWConfig shenanigan,
//       and since it started at v0.1, I'm not going to risk it... ;)
/*
static const char* kobo_disp_res[] = { "800x600",   "1024x758",   "1024x768",    "1440x1080", "1366x768",
				       "1448x1072", "1600x1200",  "400x375x2",   "1872x1404", "960x540",
				       "2200x1650", "1440x640x4", "1600x1200x4", "1920x1440" };
*/

static void set_kobo_quirks(unsigned short int, FBInkDeviceQuirks*);
static void identify_kobo(FBInkDeviceQuirks*);
#        endif    // FBINK_FOR_KINDLE

static void identify_device(FBInkDeviceQuirks*);
#endif    // !FBINK_FOR_LINUX

#endif
