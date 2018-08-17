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

#include "fbink_device_id.h"

#ifdef FBINK_FOR_KINDLE
// NOTE: This is adapted from KindleTool,
//       c.f., https://github.com/NiLuJe/KindleTool/blob/master/KindleTool/kindle_tool.h#L189
static bool
    is_kindle_device(uint32_t dev, FBInkDeviceQuirks* device_quirks)
{
	switch (dev) {
		case 0x01:    // K1
		case 0x02:    // K2
		case 0x03:
		case 0x04:    // KDX
		case 0x05:
		case 0x09:
		case 0x08:    // K3
		case 0x06:
		case 0x0A:
		case 0x0E:    // K4
			device_quirks->isKindleLegacy = true;
			return true;
		case 0x0F:    // K5
		case 0x11:
		case 0x10:
		case 0x12:
			device_quirks->isKindlePearlScreen = true;
			return true;
		case 0x23:    // K4b
			return true;
		case 0x24:    // PW1
		case 0x1B:
		case 0x1C:
		case 0x1D:
		case 0x1F:
		case 0x20:
			device_quirks->isKindlePearlScreen = true;
			return true;
		case 0xD4:    // PW2
		case 0x5A:
		case 0xD5:
		case 0xD6:
		case 0xD7:
		case 0xD8:
		case 0xF2:
		case 0x17:
		case 0x60:
		case 0xF4:
		case 0xF9:
		case 0x62:
		case 0x61:
		case 0x5F:
		case 0xC6:    // KT2
		case 0x13:    // KV
		case 0x16:    // ??
		case 0x21:
		case 0x54:    // KV
		case 0x2A:
		case 0x4F:
		case 0x52:
		case 0x53:
		case 0x07:    // ??
		case 0x0B:
		case 0x0C:
		case 0x0D:
		case 0x99:
		case 0xDD:    // KT2 AUS
			return true;
		default:
			return false;
	}
}

static bool
    is_kindle_device_new(uint32_t dev, FBInkDeviceQuirks* device_quirks)
{
	switch (dev) {
		case 0x201:    // PW3
		case 0x202:
		case 0x204:
		case 0x205:
		case 0x206:
		case 0x207:
		case 0x26B:
		case 0x26C:
		case 0x26D:
		case 0x26E:
		case 0x26F:
		case 0x270:
		case 0x293:
		case 0x294:
		case 0x20C:    // KOA
		case 0x20D:
		case 0x219:
		case 0x21A:
		case 0x21B:
		case 0x21C:
		case 0x1BC:    // KT3
		case 0x269:
		case 0x26A:
			return true;
		case 0x295:    // KOA2
		case 0x296:
		case 0x297:
		case 0x298:
		case 0x2E1:
		case 0x2E2:
		case 0x2E6:
		case 0x2E7:
		case 0x2E8:
		case 0x341:
		case 0x342:
		case 0x343:
		case 0x344:
		case 0x347:
		case 0x34A:
			device_quirks->isKindleOasis2 = true;
			return true;
		default:
			return false;
	}
}

// NOTE: This is from KindleTool,
//       c.f., https://github.com/NiLuJe/KindleTool/blob/master/KindleTool/convert.c#L82
static uint32_t
    from_base(char* num, uint8_t base)
{
	// NOTE: Crockford's Base32, but with the "L" & "U" re-added in?
	const char* tbl    = "0123456789ABCDEFGHJKLMNPQRSTUVWX";
	uint32_t    result = 0;

	if (base > strlen(tbl)) {
		fprintf(stderr, "[FBInk] base %hhu is unsupported (too large).\n", base);
		return 0;
	}

	// Hi, my name is Neo. I know pointers! (Or not.)
	for (char* p = num; *p != '\0'; p++) {
		for (size_t i = 0; tbl[i] != '\0'; i++) {
			if (*p == tbl[i]) {
				result = result * base + i;
				break;
			}
		}
	}

	return result;
}

// NOTE: This is adapted from KindleTool,
//       c.f., https://github.com/NiLuJe/KindleTool/blob/master/KindleTool/create.c#L1915
static void
    identify_kindle(FBInkDeviceQuirks* device_quirks)
{
	FILE* fp = fopen("/proc/usid", "r");
	if (!fp) {
		fprintf(stderr, "[FBInk] Cannot open /proc/usid (not running on a Kindle?)!\n");
	} else {
		unsigned char serial_no[KINDLE_SERIAL_NO_LENGTH] = { '\0' };
		if (fread(serial_no, sizeof(unsigned char), KINDLE_SERIAL_NO_LENGTH, fp) < KINDLE_SERIAL_NO_LENGTH ||
		    ferror(fp) != 0) {
			fprintf(stderr, "[FBInk] Error reading /proc/usid (unexpected length)!\n");
		}
		fclose(fp);

		// Get the device code...
		char device_code[3 + 1] = { '\0' };
		// NOTE: Slice the bracketed section out of the S/N: B0[17]NNNNNNNNNNNN
		snprintf(device_code, 2 + 1, "%.*s", 2, serial_no + 2);
		// It's in hex, easy peasy.
		uint32_t dev = (uint32_t) strtoul(device_code, NULL, 16);
		// Check if it looks like the old device id scheme...
		if (!is_kindle_device(dev, device_quirks)) {
			// ... try the new device ID scheme if it doesn't... (G09[0G1]NNNNNNNNNN)
			snprintf(device_code, 3 + 1, "%.*s", 3, serial_no + 3);
			// (these ones are encoded in a slightly custom base 32)
			dev = from_base(device_code, 32);
			// ... And if it's not either list, it's truly unknown.
			if (!is_kindle_device_new(dev, device_quirks)) {
				fprintf(stderr, "[FBInk] Unidentified Kindle device %s (0x%03X)!\n", device_code, dev);
			}
		}
	}
}
#else
// NOTE: This is lifted from FBGrab,
//       c.f., http://trac.ak-team.com/trac/browser/niluje/Configs/trunk/Kindle/Misc/FBGrab/fbgrab.c#L808
static void
    identify_kobo(FBInkDeviceQuirks* device_quirks)
{
	// Get the model from Nickel's version tag file...
	FILE* fp = fopen("/mnt/onboard/.kobo/version", "re");
	if (!fp) {
		fprintf(stderr, "[FBInk] Couldn't find a Kobo version tag (not running on a Kobo?)!\n");
	} else {
		// NOTE: I'm not entirely sure this will always have a fixed length, so,
		//       rely on getline()'s dynamic allocation to be safe...
		char*              line = NULL;
		size_t             len  = 0;
		ssize_t            nread;
		unsigned short int kobo_id = 0;
		while ((nread = getline(&line, &len, fp)) != -1) {
			// Thankfully, the device code is always located in the three
			// final characters, so that's easy enough to extract without
			// having to worry about the formatting...
			kobo_id = (unsigned short int) strtoul(line + (nread - 3), NULL, 10);
			// NOTE: Device code list pilfered from
			//       https://github.com/geek1011/KoboStuff/blob/gh-pages/kobofirmware.js#L11
			switch (kobo_id) {
				case 310:    // Touch A/B (trilogy)
				case 320:    // Touch C (trilogy)
				case 340:    // Mini (pixie)
				case 330:    // Glo (kraken)
					device_quirks->isKoboNonMT = true;
					break;
				case 371:    // Glo HD (alyssum)
				case 372:    // Touch 2.0 (pika)
				case 360:    // Aura (phoenix)
					break;
				case 350:    // Aura HD (dragon)
					device_quirks->isKoboNonMT = true;
					break;
				case 370:    // Aura H2O (dahlia)
				case 374:    // Aura H2O² (snow)
					break;
				case 378:    // Aura H2O² r2 (snow)
					device_quirks->isKoboMk7 = true;
					break;
				case 373:    // Aura ONE (daylight)
				case 381:    // Aura ONE LE (daylight)
				case 375:    // Aura SE (star)
					break;
				case 379:    // Aura SE r2 (star)
				case 376:    // Clara HD (nova)
					device_quirks->isKoboMk7 = true;
					break;
				case 0:
				default:
					fprintf(stderr, "[FBInk] Unidentified Kobo device code (%hu)!\n", kobo_id);
					break;
			}
		}
		free(line);
		fclose(fp);
	}
}
#endif

static void
    identify_device(FBInkDeviceQuirks* device_quirks)
{
#ifdef FBINK_FOR_KINDLE
	identify_kindle(device_quirks);
#else
	identify_kobo(device_quirks);
#endif
}
