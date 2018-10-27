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

#ifndef FBINK_FOR_LINUX

#	include "fbink_device_id.h"

#	if defined(FBINK_FOR_KINDLE)
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
			device_quirks->isKindleLegacy = true;
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
#		ifdef FBINK_FOR_LEGACY
	FILE* fp = fopen("/proc/usid", "r");
#		else
	// NOTE: We only need to forgo CLOEXEC on Legacy Kindles ;).
	FILE* fp = fopen("/proc/usid", "re");
#		endif
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
#	elif defined(FBINK_FOR_CERVANTES)
// Read pcb id from NTX_HWCONFIG for BQ/Fnac devices, adapted from OKreader's kobo_hwconfig:
// https://github.com/lgeek/okreader/blob/master/src/kobo_hwconfig/kobo_hwconfig.c
static void
    identify_cervantes(FBInkDeviceQuirks* device_quirks)
{
	FILE* fp = fopen(HWCONFIG_DEVICE, "re");
	if (!fp) {
		fprintf(stderr,
			"[FBInk] Couldn't read from '%s', unable to identify the Cervantes model!\n",
			HWCONFIG_DEVICE);
	} else {
		NTXHWConfig config = { 0 };

		if (fseek(fp, HWCONFIG_OFFSET, SEEK_SET) != 0) {
			fprintf(stderr,
				"[FBInk] Failed to seek to position 0x%p in '%s'!\n",
				(void*) HWCONFIG_OFFSET,
				HWCONFIG_DEVICE);
		} else {
			if (fread(&config, sizeof(config), 1, fp) != 1) {
				fprintf(
				    stderr, "[FBInk] Failed to read the NTX HWConfig entry on '%s'!\n", HWCONFIG_DEVICE);
				fclose(fp);
				return;
			}
		}
		fclose(fp);

		// NOTE: These are NOT NULL-terminated, so we use the size of the storage array,
		//       and not of the string literal (where sizeof would have appended space for a terminating NULL).
		//       (i.e., here, sizeof(config.magic) == 10 == strlen(HWCONFIG_MAGIC) while sizeof(HWCONFIG_MAGIC) == 11)
		if (memcmp(config.magic, HWCONFIG_MAGIC, sizeof(config.magic)) != 0) {
			fprintf(stderr,
				"[FBInk] Input device '%s' does not appear to contain an NTX HWConfig entry!\n",
				HWCONFIG_DEVICE);
			return;
		}

		// supported devices,
		// from https://github.com/bq/cervantes/blob/master/bqHAL/Devices/mx508/src/DeviceInfoMx508.cpp#L33-L37
		switch (config.pcb_id) {
			case 22:    // BQ Cervantes Touch - Fnac Touch (2012-2013)
			case 23:    // BQ Cervantes TouchLight - Fnac Touch Plus (2012-2013)
			case 33:    // BQ Cervantes 2013 - Fnac Touch Light (2013)
				break;
			case 51:    // BQ Cervantes 3 - Fnac Touch Light 2 (2016)
			case 68:    // BQ Cervantes 4
				device_quirks->isCervantesNew = true;
				break;
			default:
				fprintf(stderr, "[FBInk] Unidentified Cervantes device (%hhu)!\n", config.pcb_id);
				break;
		}
	}
}
#	else
static void
    set_kobo_quirks(unsigned short int kobo_id, FBInkDeviceQuirks* device_quirks)
{
	// NOTE: Shaky assumption that almost everything follows the same rotation scheme, with:
	//       Boot rotation is FB_ROTATE_UD, pickel is FB_ROTATE_UR, nickel is FB_ROTATE_CCW
	//       With the exception of the Aura HD and the H2O.
	//       As usual, the H2O² is a mystery, the Rev 1 *may* follow this pattern too...
	//       Or that might be the Rev 2 only, but that would make it diverge from other Mk7, which is weirder.
	//       c.f., the relevant bit of fbink_init for more details...
	device_quirks->koboBootRota = FB_ROTATE_UD;
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
			break;
		case 360:    // Aura (phoenix)
			// NOTE: The bottom 10 pixels *may* be blacked out by Nickel? (TBC!)
			//device_quirks->koboVertOffset = -10;
			break;
		case 350:    // Aura HD (dragon)
			device_quirks->isKoboNonMT = true;
			// NOTE: Boot rotation is FB_ROTATE_UR, pickel is FB_ROTATE_UD, nickel is FB_ROTATE_CW
			device_quirks->koboBootRota = FB_ROTATE_UR;
			break;
		case 370:    // Aura H2O (dahlia)
			// NOTE: The top 11 pixels are blacked out by Nickel (behind the bezel)
			device_quirks->koboVertOffset = 11;
			// NOTE: Boot rotation is FB_ROTATE_UR, pickel is FB_ROTATE_UD, nickel is FB_ROTATE_CW
			device_quirks->koboBootRota = FB_ROTATE_UR;
			break;
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
		case 380:    // Forma (frost)
			device_quirks->isKoboMk7 = true;
			break;
		case 0:
		default:
			fprintf(stderr, "[FBInk] Unidentified Kobo device code (%hu)!\n", kobo_id);
			break;
	}
}

// NOTE: This is lifted from FBGrab,
//       c.f., http://trac.ak-team.com/trac/browser/niluje/Configs/trunk/Kindle/Misc/FBGrab/fbgrab.c#L808
static void
    identify_kobo(FBInkDeviceQuirks* device_quirks)
{
	// Get the model from Nickel's version tag file...
	FILE* fp = fopen("/mnt/onboard/.kobo/version", "re");
	if (!fp) {
		fprintf(stderr,
			"[FBInk] Couldn't find a Kobo version tag (onboard unmounted or not running on a Kobo?)!\n");
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
			set_kobo_quirks(kobo_id, device_quirks);
		}
		free(line);
		fclose(fp);

		// Get out now, we're done!
		return;
	}

	// NOTE: Okay, if we got this far, we failed to open /mnt/onboard/.kobo/version,
	//       which, provided we're really running on a Kobo, can legitimately happen,
	//       if we were launched in the middle of an USBMS session, in which case onboard is obviously not available ;).
	//       So try to do it the hard way, via the NTXHWConfig tag...
	fp = fopen(HWCONFIG_DEVICE, "re");
	if (!fp) {
		fprintf(stderr, "[FBInk] Couldn't read from '%s', unable to identify the Kobo model!\n", HWCONFIG_DEVICE);
	} else {
		NTXHWConfig config = { 0 };

		if (fseek(fp, HWCONFIG_OFFSET, SEEK_SET) != 0) {
			fprintf(stderr,
				"[FBInk] Failed to seek to position 0x%p in '%s'!\n",
				(void*) HWCONFIG_OFFSET,
				HWCONFIG_DEVICE);
		} else {
			if (fread(&config, sizeof(config), 1, fp) != 1) {
				fprintf(
				    stderr, "[FBInk] Failed to read the NTX HWConfig entry on '%s'!\n", HWCONFIG_DEVICE);
				fclose(fp);
				return;
			}
		}
		fclose(fp);

		// NOTE: These are NOT NULL-terminated, so we use the size of the storage array,
		//       and not of the string literal (where sizeof would have appended space for a terminating NULL).
		//       (i.e., here, sizeof(config.magic) == 10 == strlen(HWCONFIG_MAGIC) while sizeof(HWCONFIG_MAGIC) == 11)
		if (memcmp(config.magic, HWCONFIG_MAGIC, sizeof(config.magic)) != 0) {
			fprintf(stderr,
				"[FBInk] Input device '%s' does not appear to contain an NTX HWConfig entry!\n",
				HWCONFIG_DEVICE);
			return;
		}

		// As per /bin/kobo_config.sh, match PCB IDs to production models...
		unsigned short int kobo_id = 0;
		if (!strncmp(kobo_pcbs[config.pcb_id], "E60610", 6)) {
			// Touch A/B (trilogy) [310]
			// Touch C (trilogy) [320]
			kobo_id = 310;
		} else if (!strncmp(kobo_pcbs[config.pcb_id], "E60QB", 5) ||
			   !strncmp(kobo_pcbs[config.pcb_id], "E606B", 5)) {
			// Glo (kraken) [330]
			kobo_id = 330;
		} else if (!strncmp(kobo_pcbs[config.pcb_id], "E5061", 5)) {
			// Mini (pixie) [340]
			kobo_id = 340;
		} else if (!strncmp(kobo_pcbs[config.pcb_id], "E60Q9", 5)) {
			// Touch 2.0 (pika) [372] (if 800x600)
			// Glo HD (alyssum) [371] (if !800x600)
			kobo_id = 372;
		} else if (!strncmp(kobo_pcbs[config.pcb_id], "E606C", 5)) {
			// Aura HD (dragon) [350]
			kobo_id = 350;
		} else if (!strncmp(kobo_pcbs[config.pcb_id], "E606G", 5)) {
			// Aura H2O (dahlia) [370]
			kobo_id = 370;
		} else if (!strncmp(kobo_pcbs[config.pcb_id], "E606F", 5)) {
			// Aura (phoenix) [360]
			kobo_id = 360;
		} else if (!strncmp(kobo_pcbs[config.pcb_id], "E70Q0", 5)) {
			// Aura ONE (daylight) [373]
			// Aura ONE LE (daylight) [381]
			kobo_id = 373;
		} else if (!strncmp(kobo_pcbs[config.pcb_id], "E60K0", 5) ||
			   !strncmp(kobo_pcbs[config.pcb_id], "E60U1", 5)) {
			// Clara HD (nova) [376]
			kobo_id = 376;
		} else if (!strncmp(kobo_pcbs[config.pcb_id], "E60QL", 5) ||
			   !strncmp(kobo_pcbs[config.pcb_id], "E60U0", 5) ||
			   !strncmp(kobo_pcbs[config.pcb_id], "T60Q0", 5)) {
			// Aura SE (star) [375]
			// Aura SE r2 (star) [379] (NOTE: Mildy inaccurate, because the r2 is a Mk7, unlike the r1)
			kobo_id = 375;
		} else if (!strncmp(kobo_pcbs[config.pcb_id], "E60QM", 5)) {
			// Aura H2O² (snow) [374]
			// Aura H2O² r2 (snow) [378] (NOTE: Mildy inaccurate, because the r2 is a Mk7, unlike the r1)
			kobo_id = 374;
		} else if (!strncmp(kobo_pcbs[config.pcb_id], "E80K0", 5)) {
			// Forma (frost) [380]
			kobo_id = 380;
		} else {
			// NOTE: kobo_config.sh defaults to trilogy, which is probably a safety precaution more than anything...
			fprintf(stderr,
				"[FBInk] Unidentified Kobo PCB ID (%hhu -> %s)!\n",
				config.pcb_id,
				kobo_pcbs[config.pcb_id]);
		}

		// And now we can do this, almost as accurately as if onboard were mounted ;).
		set_kobo_quirks(kobo_id, device_quirks);
	}
}
#	endif    // FBINK_FOR_KINDLE

static void
    identify_device(FBInkDeviceQuirks* device_quirks)
{
#	if defined(FBINK_FOR_KINDLE)
	identify_kindle(device_quirks);
#	elif defined(FBINK_FOR_CERVANTES)
	identify_cervantes(device_quirks);
#	else
	identify_kobo(device_quirks);
#	endif
}

#endif    // !FBINK_FOR_LINUX
