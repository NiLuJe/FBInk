/*
	FBInk: FrameBuffer eInker, a tool to print text & images on eInk devices (Kobo/Kindle)
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

#ifndef FBINK_FOR_LINUX
#	include "fbink_device_id.h"

#	if defined(FBINK_FOR_KINDLE)
// NOTE: This is adapted from KindleTool,
//       c.f., https://github.com/NiLuJe/KindleTool/blob/master/KindleTool/kindle_tool.h#L189
static bool
    is_kindle_device(uint32_t dev)
{
	switch (dev) {
		case 0x01u:    // K1
			deviceQuirks.isKindleLegacy = true;
			// Flawfinder: ignore
			strncpy(deviceQuirks.deviceName, "1", sizeof(deviceQuirks.deviceName) - 1U);
			// Flawfinder: ignore
			strncpy(deviceQuirks.deviceCodename, "Fiona", sizeof(deviceQuirks.deviceCodename) - 1U);
			// NOTE: Possibly Fiona, too, as ADS was a proto.
			//       But later models have shown than a proto's board name often ends up naming the platform...
			// Flawfinder: ignore
			strncpy(deviceQuirks.devicePlatform, "ADS", sizeof(deviceQuirks.devicePlatform) - 1U);
			return true;
		case 0x02u:    // K2
		case 0x03u:
			deviceQuirks.isKindleLegacy = true;
			// Flawfinder: ignore
			strncpy(deviceQuirks.deviceName, "2", sizeof(deviceQuirks.deviceName) - 1U);
			// Flawfinder: ignore
			strncpy(deviceQuirks.deviceCodename, "Turing", sizeof(deviceQuirks.deviceCodename) - 1U);
			// Flawfinder: ignore
			strncpy(deviceQuirks.devicePlatform, "Mario", sizeof(deviceQuirks.devicePlatform) - 1U);
			return true;
		case 0x04u:    // KDX
		case 0x05u:
		case 0x09u:
			deviceQuirks.isKindleLegacy = true;
			deviceQuirks.screenDPI      = 150U;
			// Flawfinder: ignore
			strncpy(deviceQuirks.deviceName, "DX", sizeof(deviceQuirks.deviceName) - 1U);
			// Flawfinder: ignore
			strncpy(deviceQuirks.deviceCodename, "Nell", sizeof(deviceQuirks.deviceCodename) - 1U);
			// NOTE: Possibly Banjo? Never figured out if that platform ever saw the light of day...
			// Flawfinder: ignore
			strncpy(deviceQuirks.devicePlatform, "Mario", sizeof(deviceQuirks.devicePlatform) - 1U);
			return true;
		case 0x08u:    // K3
		case 0x06u:
		case 0x0Au:
			deviceQuirks.isKindleLegacy = true;
			// Flawfinder: ignore
			strncpy(deviceQuirks.deviceName, "3", sizeof(deviceQuirks.deviceName) - 1U);
			// Flawfinder: ignore
			strncpy(deviceQuirks.deviceCodename, "Shasta", sizeof(deviceQuirks.deviceCodename) - 1U);
			// Flawfinder: ignore
			strncpy(deviceQuirks.devicePlatform, "Luigi", sizeof(deviceQuirks.devicePlatform) - 1U);
			return true;
		case 0x0Eu:    // K4
			deviceQuirks.isKindleLegacy = true;
			// Flawfinder: ignore
			strncpy(deviceQuirks.deviceName, "4", sizeof(deviceQuirks.deviceName) - 1U);
			// Flawfinder: ignore
			strncpy(deviceQuirks.deviceCodename, "Tequila", sizeof(deviceQuirks.deviceCodename) - 1U);
			// Flawfinder: ignore
			strncpy(deviceQuirks.devicePlatform, "Yoshi", sizeof(deviceQuirks.devicePlatform) - 1U);
			return true;
		case 0x0Fu:    // K5
		case 0x11u:
		case 0x10u:
		case 0x12u:
			deviceQuirks.isKindlePearlScreen = true;
			// Flawfinder: ignore
			strncpy(deviceQuirks.deviceName, "Touch", sizeof(deviceQuirks.deviceName) - 1U);
			// Flawfinder: ignore
			strncpy(deviceQuirks.deviceCodename, "Whitney", sizeof(deviceQuirks.deviceCodename) - 1U);
			// Flawfinder: ignore
			strncpy(deviceQuirks.devicePlatform, "Yoshi", sizeof(deviceQuirks.devicePlatform) - 1U);
			return true;
		case 0x23u:    // K4b
			deviceQuirks.isKindleLegacy = true;
			// Flawfinder: ignore
			strncpy(deviceQuirks.deviceName, "4", sizeof(deviceQuirks.deviceName) - 1U);
			// Flawfinder: ignore
			strncpy(deviceQuirks.deviceCodename, "Sauza", sizeof(deviceQuirks.deviceCodename) - 1U);
			// Flawfinder: ignore
			strncpy(deviceQuirks.devicePlatform, "Yoshi", sizeof(deviceQuirks.devicePlatform) - 1U);
			return true;
		case 0x24u:    // PW1
		case 0x1Bu:
		case 0x1Cu:
		case 0x1Du:
		case 0x1Fu:
		case 0x20u:
			deviceQuirks.isKindlePearlScreen = true;
			deviceQuirks.screenDPI           = 212U;
			// Flawfinder: ignore
			strncpy(deviceQuirks.deviceName, "PaperWhite", sizeof(deviceQuirks.deviceName) - 1U);
			// Flawfinder: ignore
			strncpy(deviceQuirks.deviceCodename, "Celeste", sizeof(deviceQuirks.deviceCodename) - 1U);
			// NOTE: AKA Yoshime3
			// Flawfinder: ignore
			strncpy(deviceQuirks.devicePlatform, "Yoshime", sizeof(deviceQuirks.devicePlatform) - 1U);
			return true;
		case 0xD4u:    // PW2
		case 0x5Au:
		case 0xD5u:
		case 0xD6u:
		case 0xD7u:
		case 0xD8u:
		case 0xF2u:
		case 0x17u:
		case 0x60u:
		case 0xF4u:
		case 0xF9u:
		case 0x62u:
		case 0x61u:
		case 0x5Fu:
			deviceQuirks.screenDPI = 212U;
			// Flawfinder: ignore
			strncpy(deviceQuirks.deviceName, "PaperWhite 2", sizeof(deviceQuirks.deviceName) - 1U);
			// Flawfinder: ignore
			strncpy(deviceQuirks.deviceCodename, "Pinot", sizeof(deviceQuirks.deviceCodename) - 1U);
			// Flawfinder: ignore
			strncpy(deviceQuirks.devicePlatform, "Wario", sizeof(deviceQuirks.devicePlatform) - 1U);
			return true;
		case 0xC6u:    // KT2
			// Flawfinder: ignore
			strncpy(deviceQuirks.deviceName, "Basic", sizeof(deviceQuirks.deviceName) - 1U);
			// Flawfinder: ignore
			strncpy(deviceQuirks.deviceCodename, "Bourbon", sizeof(deviceQuirks.deviceCodename) - 1U);
			// Flawfinder: ignore
			strncpy(deviceQuirks.devicePlatform, "Wario", sizeof(deviceQuirks.devicePlatform) - 1U);
			return true;
		case 0x13u:    // KV
			deviceQuirks.screenDPI = 300U;
			// Flawfinder: ignore
			strncpy(deviceQuirks.deviceName, "Voyage", sizeof(deviceQuirks.deviceName) - 1U);
			// Flawfinder: ignore
			strncpy(deviceQuirks.deviceCodename, "Icewine", sizeof(deviceQuirks.deviceCodename) - 1U);
			// Flawfinder: ignore
			strncpy(deviceQuirks.devicePlatform, "Wario", sizeof(deviceQuirks.devicePlatform) - 1U);
			return true;
		case 0x16u:    // ??
		case 0x21u:
			return true;
		case 0x54u:    // KV
		case 0x2Au:
		case 0x4Fu:
		case 0x52u:
		case 0x53u:
			deviceQuirks.screenDPI = 300U;
			// Flawfinder: ignore
			strncpy(deviceQuirks.deviceName, "Voyage", sizeof(deviceQuirks.deviceName) - 1U);
			// Flawfinder: ignore
			strncpy(deviceQuirks.deviceCodename, "Icewine", sizeof(deviceQuirks.deviceCodename) - 1U);
			// Flawfinder: ignore
			strncpy(deviceQuirks.devicePlatform, "Wario", sizeof(deviceQuirks.devicePlatform) - 1U);
			return true;
		case 0x07u:    // ??
		case 0x0Bu:
		case 0x0Cu:
		case 0x0Du:
		case 0x99u:
			return true;
		case 0xDDu:    // KT2 AUS
			// Flawfinder: ignore
			strncpy(deviceQuirks.deviceName, "Basic", sizeof(deviceQuirks.deviceName) - 1U);
			// Flawfinder: ignore
			strncpy(deviceQuirks.deviceCodename, "Bourbon", sizeof(deviceQuirks.deviceCodename) - 1U);
			// Flawfinder: ignore
			strncpy(deviceQuirks.devicePlatform, "Wario", sizeof(deviceQuirks.devicePlatform) - 1U);
			return true;
		default:
			return false;
	}
}

static bool
    is_kindle_device_new(uint32_t dev)
{
	switch (dev) {
		case 0x201u:    // PW3
		case 0x202u:
		case 0x204u:
		case 0x205u:
		case 0x206u:
		case 0x207u:
		case 0x26Bu:
		case 0x26Cu:
		case 0x26Du:
		case 0x26Eu:
		case 0x26Fu:
		case 0x270u:
		case 0x293u:
		case 0x294u:
			deviceQuirks.screenDPI = 300U;
			// Flawfinder: ignore
			strncpy(deviceQuirks.deviceName, "PaperWhite 3", sizeof(deviceQuirks.deviceName) - 1U);
			// Flawfinder: ignore
			strncpy(deviceQuirks.deviceCodename, "Muscat", sizeof(deviceQuirks.deviceCodename) - 1U);
			// Flawfinder: ignore
			strncpy(deviceQuirks.devicePlatform, "Wario", sizeof(deviceQuirks.devicePlatform) - 1U);
			return true;
		case 0x20Cu:    // KOA
		case 0x20Du:
		case 0x219u:
		case 0x21Au:
		case 0x21Bu:
		case 0x21Cu:
			deviceQuirks.screenDPI = 300U;
			// Flawfinder: ignore
			strncpy(deviceQuirks.deviceName, "Oasis", sizeof(deviceQuirks.deviceName) - 1U);
			// Flawfinder: ignore
			strncpy(deviceQuirks.deviceCodename, "Whisky", sizeof(deviceQuirks.deviceCodename) - 1U);
			// Flawfinder: ignore
			strncpy(deviceQuirks.devicePlatform, "Duet", sizeof(deviceQuirks.devicePlatform) - 1U);
			return true;
		case 0x1BCu:    // KT3
		case 0x269u:
		case 0x26Au:
			// Flawfinder: ignore
			strncpy(deviceQuirks.deviceName, "Basic 2", sizeof(deviceQuirks.deviceName) - 1U);
			// Flawfinder: ignore
			strncpy(deviceQuirks.deviceCodename, "Eanab", sizeof(deviceQuirks.deviceCodename) - 1U);
			// Flawfinder: ignore
			strncpy(deviceQuirks.devicePlatform, "Heisenberg", sizeof(deviceQuirks.devicePlatform) - 1U);
			return true;
		case 0x295u:    // KOA2
		case 0x296u:
		case 0x297u:
		case 0x298u:
		case 0x2E1u:
		case 0x2E2u:
		case 0x2E6u:
		case 0x2E7u:
		case 0x2E8u:
		case 0x341u:
		case 0x342u:
		case 0x343u:
		case 0x344u:
		case 0x347u:
		case 0x34Au:
			deviceQuirks.isKindleZelda = true;
			deviceQuirks.screenDPI     = 300U;
			// Flawfinder: ignore
			strncpy(deviceQuirks.deviceName, "Oasis 2", sizeof(deviceQuirks.deviceName) - 1U);
			// Flawfinder: ignore
			strncpy(deviceQuirks.deviceCodename, "Cognac", sizeof(deviceQuirks.deviceCodename) - 1U);
			// Flawfinder: ignore
			strncpy(deviceQuirks.devicePlatform, "Zelda", sizeof(deviceQuirks.devicePlatform) - 1U);
			return true;
		case 0x2F7u:    // PW4
		case 0x361u:
		case 0x362u:
		case 0x363u:
		case 0x364u:
		case 0x365u:
		case 0x366u:
		case 0x367u:
		case 0x372u:
		case 0x373u:
		case 0x374u:
		case 0x375u:
		case 0x376u:
		case 0x402u:
		case 0x403u:
		case 0x4D8u:
		case 0x4D9u:
		case 0x4DAu:
		case 0x4DBu:
		case 0x4DCu:
		case 0x4DDu:
			deviceQuirks.isKindleRex = true;
			deviceQuirks.screenDPI   = 300U;
			// Flawfinder: ignore
			strncpy(deviceQuirks.deviceName, "PaperWhite 4", sizeof(deviceQuirks.deviceName) - 1U);
			// Flawfinder: ignore
			strncpy(deviceQuirks.deviceCodename, "Moonshine", sizeof(deviceQuirks.deviceCodename) - 1U);
			// Flawfinder: ignore
			strncpy(deviceQuirks.devicePlatform, "Rex", sizeof(deviceQuirks.devicePlatform) - 1U);
			return true;
		case 0x414u:    // KT4
		case 0x3CFu:
		case 0x3D0u:
		case 0x3D1u:
		case 0x3D2u:
		case 0x3ABu:
			deviceQuirks.isKindleRex = true;
			// Flawfinder: ignore
			strncpy(deviceQuirks.deviceName, "Basic 3", sizeof(deviceQuirks.deviceName) - 1U);
			// Flawfinder: ignore
			strncpy(deviceQuirks.deviceCodename, "Jaeger", sizeof(deviceQuirks.deviceCodename) - 1U);
			// Flawfinder: ignore
			strncpy(deviceQuirks.devicePlatform, "Rex", sizeof(deviceQuirks.devicePlatform) - 1U);
			return true;
		case 0x434u:    // KOA3
		case 0x3D8u:
		case 0x3D7u:
		case 0x3D6u:
		case 0x3D5u:
		case 0x3D4u:
			deviceQuirks.isKindleZelda = true;
			deviceQuirks.screenDPI     = 300U;
			// Flawfinder: ignore
			strncpy(deviceQuirks.deviceName, "Oasis 3", sizeof(deviceQuirks.deviceName) - 1U);
			// Flawfinder: ignore
			strncpy(deviceQuirks.deviceCodename, "Stinger", sizeof(deviceQuirks.deviceCodename) - 1U);
			// Flawfinder: ignore
			strncpy(deviceQuirks.devicePlatform, "Zelda", sizeof(deviceQuirks.devicePlatform) - 1U);
			return true;
		default:
			// Flawfinder: ignore
			strncpy(deviceQuirks.deviceName, "Unknown!", sizeof(deviceQuirks.deviceName) - 1U);
			return false;
	}
}

// NOTE: This is from KindleTool,
//       c.f., https://github.com/NiLuJe/KindleTool/blob/master/KindleTool/convert.c#L82
static uint32_t
    from_base(const char* num, uint8_t base)
{
	// NOTE: Crockford's Base32, but with the "L" & "U" re-added in?
	const char tbl[]  = "0123456789ABCDEFGHJKLMNPQRSTUVWX";
	uint32_t   result = 0U;

	if (base >= sizeof(tbl)) {
		WARN("base %hhu is unsupported (too large)", base);
		return 0;
	}

	// Hi, my name is Neo. I know pointers! (Or not.)
	for (const char* restrict p = num; *p != '\0'; p++) {
		for (uint8_t i = 0; tbl[i] != '\0'; i++) {
			if (*p == tbl[i]) {
				result = result * base + i;
				break;
			}
		}
	}

	return result;
}

// NOTE: This is from KindleTool,
//       c.f., https://github.com/NiLuJe/KindleTool/blob/master/KindleTool/convert.c#L51
// Pilfered from http://rosettacode.org/wiki/Non-decimal_radices/Convert#C
static char*
    to_base(int64_t num, uint8_t base)
{
	// NOTE: Crockford's Base32, but with the "L" & "U" re-added in?
	const char tbl[]   = "0123456789ABCDEFGHJKLMNPQRSTUVWX";
	char       buf[66] = { 0 };
	char*      out     = NULL;
	uint64_t   n;
	uint32_t   len = 0U;
	bool       neg = false;

	if (base >= sizeof(tbl)) {
		WARN("base %hhu is unsupported (too large).", base);
		return NULL;
	}

	// safe against most negative integer
	n = (neg = (num < 0)) ? (uint64_t)(~num) + 1 : (uint64_t) num;

	do {
		buf[len++] = tbl[n % base];
	} while (n /= base);

	out = calloc(len + neg + 1U, sizeof(*out));
	if (out == NULL) {
		WARN("Error allocating base32 output string buffer: %m");
		return NULL;
	}
	for (uint32_t i = neg; len > 0U; i++) {
		out[i] = buf[--len];
	}
	if (neg) {
		out[0] = '-';
	}

	return out;
}

// NOTE: This is adapted from KindleTool,
//       c.f., https://github.com/NiLuJe/KindleTool/blob/master/KindleTool/create.c#L1915
static void
    identify_kindle(void)
{
	FILE* fp = fopen("/proc/usid", "r" STDIO_CLOEXEC);
	if (!fp) {
		WARN("Cannot open /proc/usid (%m). Not running on a Kindle?");
	} else {
		unsigned char serial_no[KINDLE_SERIAL_NO_LENGTH] = { 0 };
		if (fread(serial_no, sizeof(unsigned char), KINDLE_SERIAL_NO_LENGTH, fp) < KINDLE_SERIAL_NO_LENGTH ||
		    ferror(fp) != 0) {
			WARN("Error reading /proc/usid (unexpected length)");
		}
		fclose(fp);

		// Get the device code...
		char     device_code[3 + 1] = { 0 };
		uint32_t dev                = 0U;
		// NOTE: If the S/N starts with B or 9, assume it's an older device with an hexadecimal device code
		if (serial_no[0] == 'B' || serial_no[0] == '9') {
			// NOTE: Slice the bracketed section out of the S/N: B0[17]NNNNNNNNNNNN
			snprintf(device_code, 2 + 1, "%.*s", 2, serial_no + 2);
			// It's in hex, easy peasy.
			dev = (uint32_t) strtoul(device_code, NULL, 16);
			// Check if that looks like a known id in the old device id scheme...
			if (!is_kindle_device(dev)) {
				WARN("Unidentified Kindle device %s (0x%02X) [%.6s]", device_code, dev, serial_no);
			}
		} else {
			// Otherwise, assume it's the new base32-ish format (so far, all of those S/N start with a 'G').
			// NOTE: Slice the bracketed section out of the S/N: (G09[0G1]NNNNNNNNNN)
			snprintf(device_code, 3 + 1, "%.*s", 3, serial_no + 3);
			// (these ones are encoded in a slightly custom base 32)
			dev = from_base(device_code, 32U);
			// Check if that looks like a known id in the new device id scheme...
			if (!is_kindle_device_new(dev)) {
				WARN("Unidentified Kindle device %s (0x%03X) [%.6s]", device_code, dev, serial_no);
			}
		}
		// Store the device ID...
		deviceQuirks.deviceId = (unsigned short int) dev;
		// HW invert should be safe on Kindle, so just blacklist einkfb devices
		if (deviceQuirks.isKindleLegacy) {
			deviceQuirks.canHWInvert = false;
		} else {
			deviceQuirks.canHWInvert = true;
		}
	}
}
#	elif defined(FBINK_FOR_CERVANTES)
// Read pcb id from NTX_HWCONFIG for BQ/Fnac devices, adapted from OKreader's kobo_hwconfig:
// https://github.com/lgeek/okreader/blob/master/src/kobo_hwconfig/kobo_hwconfig.c
static void
    identify_cervantes(void)
{
	FILE* fp = fopen(HWCONFIG_DEVICE, "re");
	if (!fp) {
		WARN("Couldn't read from '%s' (%m), unable to identify the Cervantes model", HWCONFIG_DEVICE);
	} else {
#		pragma GCC diagnostic push
#		pragma GCC diagnostic ignored "-Wmissing-braces"
		NTXHWConfig config = { 0 };
#		pragma GCC diagnostic pop

		if (fseek(fp, HWCONFIG_OFFSET, SEEK_SET) != 0) {
			WARN("Failed to seek to position 0x%p in '%s': %m", (void*) HWCONFIG_OFFSET, HWCONFIG_DEVICE);
		} else {
			if (fread(&config, sizeof(config), 1, fp) < 1 || ferror(fp) != 0) {
				WARN("Failed to read the NTX HWConfig entry on '%s'", HWCONFIG_DEVICE);
				fclose(fp);
				return;
			}
		}
		fclose(fp);

		// NOTE: These are NOT NULL-terminated, so we use the size of the storage array,
		//       and not of the string literal (where sizeof would have appended space for a terminating NULL).
		//       (i.e., here, sizeof(config.magic) == 10 == strlen(HWCONFIG_MAGIC) while sizeof(HWCONFIG_MAGIC) == 11)
		if (memcmp(config.magic, HWCONFIG_MAGIC, sizeof(config.magic)) != 0) {
			WARN("Input device '%s' does not appear to contain an NTX HWConfig entry", HWCONFIG_DEVICE);
			return;
		}

		// Store the device ID...
		deviceQuirks.deviceId = config.pcb_id;
		// Some devices *may* be based on the same board as the Kobo Aura, so, let's be cautious...
		deviceQuirks.canHWInvert = false;
		// NOTE: See the comments in set_kobo_quirks about this.
		//       I'm hoping there aren't any special snowflakes in the lineup...
		deviceQuirks.ntxBootRota = FB_ROTATE_UD;
		// supported devices,
		// from https://github.com/bq/cervantes/blob/master/bqHAL/Devices/mx508/src/DeviceInfoMx508.cpp#L33-L37
		switch (config.pcb_id) {
			case 22U:    // BQ Cervantes Touch - Fnac Touch (2012-2013)
				deviceQuirks.screenDPI = 167U;
				// Flawfinder: ignore
				strncpy(deviceQuirks.deviceName, "Touch", sizeof(deviceQuirks.deviceName) - 1U);
				break;
			case 23U:    // BQ Cervantes TouchLight - Fnac Touch Plus (2012-2013)
				deviceQuirks.screenDPI = 167U;
				// Flawfinder: ignore
				strncpy(deviceQuirks.deviceName, "TouchLight", sizeof(deviceQuirks.deviceName) - 1U);
				break;
			case 33U:    // BQ Cervantes 2013 - Fnac Touch Light (2013)
				deviceQuirks.screenDPI = 212U;
				// Flawfinder: ignore
				strncpy(deviceQuirks.deviceName, "2013", sizeof(deviceQuirks.deviceName) - 1U);
				break;
			case 51U:    // BQ Cervantes 3 - Fnac Touch Light 2 (2016)
				deviceQuirks.screenDPI = 300U;
				// Flawfinder: ignore
				strncpy(deviceQuirks.deviceName, "3", sizeof(deviceQuirks.deviceName) - 1U);
				break;
			case 68U:    // BQ Cervantes 4
				deviceQuirks.screenDPI = 300U;
				// Flawfinder: ignore
				strncpy(deviceQuirks.deviceName, "4", sizeof(deviceQuirks.deviceName) - 1U);
				break;
			default:
				WARN("Unidentified Cervantes device (%hhu)", config.pcb_id);
				// Flawfinder: ignore
				strncpy(deviceQuirks.deviceName, "Unknown!", sizeof(deviceQuirks.deviceName) - 1U);
				break;
		}
	}
}
#	elif defined(FBINK_FOR_KOBO)
static void
    set_kobo_quirks(unsigned short int kobo_id)
{
	// Store the device ID...
	deviceQuirks.deviceId = kobo_id;
	// HW invert should *generally* be safe on Kobo, with a few exceptions...
	deviceQuirks.canHWInvert = true;
	// NOTE: Shaky assumption that almost everything follows the same rotation scheme, with:
	//       Boot rotation is FB_ROTATE_UD, pickel is FB_ROTATE_UR, nickel is FB_ROTATE_CCW
	//       With the exception of the Aura HD and the H2O.
	//       As usual, the H2O² is a mystery, the Rev 1 *may* follow this pattern too...
	//       Or that might be the Rev 2 only, but that would make it diverge from other Mk7, which is weirder.
	//       c.f., the relevant bit of fbink_init for more details...
	// NOTE: On my Forma, pickel behaves a bit differently: it still drops to 16bpp, but appears to prefer FB_ROTATE_CCW,
	//       as does nickel in the default Portrait orientation (buttons on the right).
	//       I *think* the boot rota is FB_ROTATE_UR,
	//       but detecting it as pickel instead appears to do the right thing right now, so I'm not going to mess with it...
	deviceQuirks.ntxBootRota = FB_ROTATE_UD;
	// NOTE: Most kernels thankfully don't resort to weird rotation quirks ;).
	//       c.f., mxc_epdc_fb_check_var @ drivers/video/fbdev/mxc/mxc_epdc_v2_fb.c
	deviceQuirks.ntxRotaQuirk = NTX_ROTA_STRAIGHT;
	// NOTE: Device code list pilfered from
	//       https://github.com/geek1011/KoboStuff/blob/gh-pages/kobofirmware.js#L11
	//       See also https://github.com/geek1011/koboutils/pull/1 and the links referenced there
	//       for all you ever wanted to know about Kobo codenames ;).
	switch (kobo_id) {
		case 310U:    // Touch A/B (trilogy)
			deviceQuirks.isKoboNonMT = true;
			// Flawfinder: ignore
			strncpy(deviceQuirks.deviceName, "Touch A/B", sizeof(deviceQuirks.deviceName) - 1U);
			// Flawfinder: ignore
			strncpy(deviceQuirks.deviceCodename, "Trilogy", sizeof(deviceQuirks.deviceCodename) - 1U);
			// Flawfinder: ignore
			strncpy(deviceQuirks.devicePlatform, "Mark 3", sizeof(deviceQuirks.devicePlatform) - 1U);
			break;
		case 320U:    // Touch C (trilogy)
			deviceQuirks.isKoboNonMT = true;
			// Flawfinder: ignore
			strncpy(deviceQuirks.deviceName, "Touch C", sizeof(deviceQuirks.deviceName) - 1U);
			// Flawfinder: ignore
			strncpy(deviceQuirks.deviceCodename, "Trilogy", sizeof(deviceQuirks.deviceCodename) - 1U);
			// Flawfinder: ignore
			strncpy(deviceQuirks.devicePlatform, "Mark 4", sizeof(deviceQuirks.devicePlatform) - 1U);
			break;
		case 340U:    // Mini (pixie)
			deviceQuirks.isKoboNonMT = true;
			deviceQuirks.screenDPI   = 200U;
			// Flawfinder: ignore
			strncpy(deviceQuirks.deviceName, "Mini", sizeof(deviceQuirks.deviceName) - 1U);
			// Flawfinder: ignore
			strncpy(deviceQuirks.deviceCodename, "Pixie", sizeof(deviceQuirks.deviceCodename) - 1U);
			// Flawfinder: ignore
			strncpy(deviceQuirks.devicePlatform, "Mark 4", sizeof(deviceQuirks.devicePlatform) - 1U);
			break;
		case 330U:    // Glo (kraken)
			deviceQuirks.isKoboNonMT = true;
			deviceQuirks.screenDPI   = 212U;
			// Flawfinder: ignore
			strncpy(deviceQuirks.deviceName, "Glo", sizeof(deviceQuirks.deviceName) - 1U);
			// Flawfinder: ignore
			strncpy(deviceQuirks.deviceCodename, "Kraken", sizeof(deviceQuirks.deviceCodename) - 1U);
			// Flawfinder: ignore
			strncpy(deviceQuirks.devicePlatform, "Mark 4", sizeof(deviceQuirks.devicePlatform) - 1U);
			break;
		case 371U:    // Glo HD (alyssum)
			deviceQuirks.screenDPI = 300U;
			// Flawfinder: ignore
			strncpy(deviceQuirks.deviceName, "Glo HD", sizeof(deviceQuirks.deviceName) - 1U);
			// Flawfinder: ignore
			strncpy(deviceQuirks.deviceCodename, "Alyssum", sizeof(deviceQuirks.deviceCodename) - 1U);
			// Flawfinder: ignore
			strncpy(deviceQuirks.devicePlatform, "Mark 6", sizeof(deviceQuirks.devicePlatform) - 1U);
			break;
		case 372U:    // Touch 2.0 (pika)
			// Flawfinder: ignore
			strncpy(deviceQuirks.deviceName, "Touch 2.0", sizeof(deviceQuirks.deviceName) - 1U);
			// Flawfinder: ignore
			strncpy(deviceQuirks.deviceCodename, "Pika", sizeof(deviceQuirks.deviceCodename) - 1U);
			// Flawfinder: ignore
			strncpy(deviceQuirks.devicePlatform, "Mark 6", sizeof(deviceQuirks.devicePlatform) - 1U);
			break;
		case 360U:    // Aura (phoenix)
			// NOTE: The bottom 10 pixels are blacked out by Nickel (behind the bezel)
			deviceQuirks.koboVertOffset = -10;
			// NOTE: According to the nightmode hack, the Aura's kernel *may* be crashy w/ PxP inversion...
			deviceQuirks.canHWInvert = false;
			deviceQuirks.screenDPI   = 212U;
			// Flawfinder: ignore
			strncpy(deviceQuirks.deviceName, "Aura", sizeof(deviceQuirks.deviceName) - 1U);
			// Flawfinder: ignore
			strncpy(deviceQuirks.deviceCodename, "Phoenix", sizeof(deviceQuirks.deviceCodename) - 1U);
			// Flawfinder: ignore
			strncpy(deviceQuirks.devicePlatform, "Mark 5", sizeof(deviceQuirks.devicePlatform) - 1U);
			break;
		case 350U:    // Aura HD (dragon)
			deviceQuirks.isKoboNonMT = true;
			// NOTE: Boot rotation is FB_ROTATE_UR, pickel is FB_ROTATE_UD, nickel is FB_ROTATE_CW
			deviceQuirks.ntxBootRota  = FB_ROTATE_UR;
			deviceQuirks.ntxRotaQuirk = NTX_ROTA_ALL_INVERTED;
			deviceQuirks.screenDPI    = 265U;
			// Flawfinder: ignore
			strncpy(deviceQuirks.deviceName, "Aura HD", sizeof(deviceQuirks.deviceName) - 1U);
			// Flawfinder: ignore
			strncpy(deviceQuirks.deviceCodename, "Dragon", sizeof(deviceQuirks.deviceCodename) - 1U);
			// Flawfinder: ignore
			strncpy(deviceQuirks.devicePlatform, "Mark 4", sizeof(deviceQuirks.devicePlatform) - 1U);
			break;
		case 370U:    // Aura H2O (dahlia)
			// NOTE: The top 11 pixels are blacked out by Nickel (behind the bezel)
			deviceQuirks.koboVertOffset = 11;
			// NOTE: Boot rotation is FB_ROTATE_UR, pickel is FB_ROTATE_UD, nickel is FB_ROTATE_CW
			deviceQuirks.ntxBootRota  = FB_ROTATE_UR;
			deviceQuirks.ntxRotaQuirk = NTX_ROTA_ALL_INVERTED;
			deviceQuirks.screenDPI    = 265U;
			// Flawfinder: ignore
			strncpy(deviceQuirks.deviceName, "Aura H2O", sizeof(deviceQuirks.deviceName) - 1U);
			// Flawfinder: ignore
			strncpy(deviceQuirks.deviceCodename, "Dahlia", sizeof(deviceQuirks.deviceCodename) - 1U);
			// Flawfinder: ignore
			strncpy(deviceQuirks.devicePlatform, "Mark 5", sizeof(deviceQuirks.devicePlatform) - 1U);
			break;
		case 374U:    // Aura H2O² (snow)
			deviceQuirks.ntxBootRota = FB_ROTATE_UR;
			// NOTE: Is indeed NTX_ROTA_STRAIGHT
			deviceQuirks.screenDPI = 265U;
			// Flawfinder: ignore
			strncpy(deviceQuirks.deviceName, "Aura H2O²", sizeof(deviceQuirks.deviceName) - 1U);
			// Flawfinder: ignore
			strncpy(deviceQuirks.deviceCodename, "Snow", sizeof(deviceQuirks.deviceCodename) - 1U);
			// Flawfinder: ignore
			strncpy(deviceQuirks.devicePlatform, "Mark 6", sizeof(deviceQuirks.devicePlatform) - 1U);
			break;
		case 378U:    // Aura H2O² r2 (snow)
			deviceQuirks.isKoboMk7   = true;
			deviceQuirks.ntxBootRota = FB_ROTATE_UR;
			// NOTE: *Might* be NTX_ROTA_ODD_INVERTED
			deviceQuirks.screenDPI = 265U;
			// Flawfinder: ignore
			strncpy(deviceQuirks.deviceName, "Aura H2O² r2", sizeof(deviceQuirks.deviceName) - 1U);
			// Flawfinder: ignore
			strncpy(deviceQuirks.deviceCodename, "Snow", sizeof(deviceQuirks.deviceCodename) - 1U);
			// Flawfinder: ignore
			strncpy(deviceQuirks.devicePlatform, "Mark 7", sizeof(deviceQuirks.devicePlatform) - 1U);
			break;
		case 373U:    // Aura ONE (daylight)
			deviceQuirks.screenDPI = 300U;
			// Flawfinder: ignore
			strncpy(deviceQuirks.deviceName, "Aura One", sizeof(deviceQuirks.deviceName) - 1U);
			// Flawfinder: ignore
			strncpy(deviceQuirks.deviceCodename, "Daylight", sizeof(deviceQuirks.deviceCodename) - 1U);
			// Flawfinder: ignore
			strncpy(deviceQuirks.devicePlatform, "Mark 6", sizeof(deviceQuirks.devicePlatform) - 1U);
			break;
		case 381U:    // Aura ONE LE (daylight)
			deviceQuirks.screenDPI = 300U;
			// Flawfinder: ignore
			strncpy(deviceQuirks.deviceName, "Aura One LE", sizeof(deviceQuirks.deviceName) - 1U);
			// Flawfinder: ignore
			strncpy(deviceQuirks.deviceCodename, "Daylight", sizeof(deviceQuirks.deviceCodename) - 1U);
			// Flawfinder: ignore
			strncpy(deviceQuirks.devicePlatform, "Mark 6", sizeof(deviceQuirks.devicePlatform) - 1U);
			break;
		case 375U:    // Aura SE (star)
			deviceQuirks.screenDPI = 212U;
			// Flawfinder: ignore
			strncpy(deviceQuirks.deviceName, "Aura SE", sizeof(deviceQuirks.deviceName) - 1U);
			// Flawfinder: ignore
			strncpy(deviceQuirks.deviceCodename, "Star", sizeof(deviceQuirks.deviceCodename) - 1U);
			// Flawfinder: ignore
			strncpy(deviceQuirks.devicePlatform, "Mark 6", sizeof(deviceQuirks.devicePlatform) - 1U);
			break;
		case 379U:    // Aura SE r2 (star)
			deviceQuirks.isKoboMk7 = true;
			// NOTE: *Might* be NTX_ROTA_ODD_INVERTED
			deviceQuirks.screenDPI = 212U;
			// Flawfinder: ignore
			strncpy(deviceQuirks.deviceName, "Aura SE r2", sizeof(deviceQuirks.deviceName) - 1U);
			// Flawfinder: ignore
			strncpy(deviceQuirks.deviceCodename, "Star", sizeof(deviceQuirks.deviceCodename) - 1U);
			// Flawfinder: ignore
			strncpy(deviceQuirks.devicePlatform, "Mark 7", sizeof(deviceQuirks.devicePlatform) - 1U);
			break;
		case 376U:    // Clara HD (nova)
			deviceQuirks.isKoboMk7 = true;
			// NOTE: Is indeed NTX_ROTA_STRAIGHT
			deviceQuirks.screenDPI = 300U;
			// Flawfinder: ignore
			strncpy(deviceQuirks.deviceName, "Clara HD", sizeof(deviceQuirks.deviceName) - 1U);
			// Flawfinder: ignore
			strncpy(deviceQuirks.deviceCodename, "Nova", sizeof(deviceQuirks.deviceCodename) - 1U);
			// Flawfinder: ignore
			strncpy(deviceQuirks.devicePlatform, "Mark 7", sizeof(deviceQuirks.devicePlatform) - 1U);
			break;
		case 377U:    // Forma (frost)
			deviceQuirks.isKoboMk7 = true;
			deviceQuirks.canRotate = true;
			// NOTE: Because Mk.7 and KOBO_HWCFG_DisplayBusWidth (35) is "16Bits_mirror" (3)
			deviceQuirks.ntxRotaQuirk = NTX_ROTA_ODD_INVERTED;
			deviceQuirks.screenDPI    = 300U;
			// Flawfinder: ignore
			strncpy(deviceQuirks.deviceName, "Forma", sizeof(deviceQuirks.deviceName) - 1U);
			// Flawfinder: ignore
			strncpy(deviceQuirks.deviceCodename, "Frost", sizeof(deviceQuirks.deviceCodename) - 1U);
			// Flawfinder: ignore
			strncpy(deviceQuirks.devicePlatform, "Mark 7", sizeof(deviceQuirks.devicePlatform) - 1U);
			break;
		case 380U:    // Forma 32GB (frost)
			deviceQuirks.isKoboMk7 = true;
			deviceQuirks.canRotate = true;
			// NOTE: Because Mk.7 and KOBO_HWCFG_DisplayBusWidth (35) is "16Bits_mirror" (3)
			deviceQuirks.ntxRotaQuirk = NTX_ROTA_ODD_INVERTED;
			deviceQuirks.screenDPI    = 300U;
			// Flawfinder: ignore
			strncpy(deviceQuirks.deviceName, "Forma 32GB", sizeof(deviceQuirks.deviceName) - 1U);
			// Flawfinder: ignore
			strncpy(deviceQuirks.deviceCodename, "Frost", sizeof(deviceQuirks.deviceCodename) - 1U);
			// Flawfinder: ignore
			strncpy(deviceQuirks.devicePlatform, "Mark 7", sizeof(deviceQuirks.devicePlatform) - 1U);
			break;
		case 384U:    // Libra H2O (storm)
			deviceQuirks.isKoboMk7 = true;
			// NOTE: Boot rotation is FB_ROTATE_UR, pickel is FB_ROTATE_UR, nickel is FB_ROTATE_UR
			//       And panel is *actually* in Portrait. Finally!
			deviceQuirks.ntxBootRota = FB_ROTATE_UR;
			deviceQuirks.canRotate   = true;
			// NOTE: This one deserves a specific entry, because the H2O² also happens to be UR + STRAIGHT,
			//       but it is decidedly *NOT* sane ;).
			deviceQuirks.ntxRotaQuirk = NTX_ROTA_SANE;
			deviceQuirks.screenDPI    = 300U;
			// Flawfinder: ignore
			strncpy(deviceQuirks.deviceName, "Libra H2O", sizeof(deviceQuirks.deviceName) - 1U);
			// Flawfinder: ignore
			strncpy(deviceQuirks.deviceCodename, "Storm", sizeof(deviceQuirks.deviceCodename) - 1U);
			// Flawfinder: ignore
			strncpy(deviceQuirks.devicePlatform, "Mark 7", sizeof(deviceQuirks.devicePlatform) - 1U);
			break;
		case 0U:
			// Like kobo_config.sh, assume Trilogy as a fallback
			deviceQuirks.isKoboNonMT = true;
			// Flawfinder: ignore
			strncpy(deviceQuirks.deviceName, "Trilogy?", sizeof(deviceQuirks.deviceName) - 1U);
			/* FALLTHROUGH */
		default:
			WARN("Unidentified Kobo device code (%hu)", kobo_id);
			// Flawfinder: ignore
			strncpy(deviceQuirks.deviceName, "Unknown!", sizeof(deviceQuirks.deviceName) - 1U);
			break;
	}
}

// NOTE: This is lifted from FBGrab,
//       c.f., http://trac.ak-team.com/trac/browser/niluje/Configs/trunk/Kindle/Misc/FBGrab/fbgrab.c#L808
static void
    identify_kobo(void)
{
	// Get the model from Nickel's version tag file...
	FILE* fp = fopen("/mnt/onboard/.kobo/version", "re");
	if (!fp) {
		ELOG("Couldn't find a Kobo version tag (onboard unmounted or not running on a Kobo?)!");
	} else {
		// NOTE: I'm not entirely sure this will always have a fixed length, so,
		//       rely on getline()'s dynamic allocation to be safe...
		char*   line = NULL;
		size_t  len  = 0;
		ssize_t nread;
		while ((nread = getline(&line, &len, fp)) != -1) {
			// Thankfully, the device code is always located in the three
			// final characters, so that's easy enough to extract without
			// having to worry about the formatting...
			unsigned short int kobo_id = (unsigned short int) strtoul(line + (nread - 3), NULL, 10);
			set_kobo_quirks(kobo_id);
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
		WARN("Couldn't read from '%s' (%m), unable to identify the Kobo model", HWCONFIG_DEVICE);
	} else {
#		pragma GCC diagnostic push
#		pragma GCC diagnostic ignored "-Wmissing-braces"
		NTXHWConfig             config      = { 0 };
#		pragma GCC diagnostic pop
		unsigned char* restrict payload     = NULL;
		uint64_t                storagesize = 0U;

		if (fseek(fp, HWCONFIG_OFFSET, SEEK_SET) != 0) {
			WARN("Failed to seek to position 0x%p in '%s': %m", (void*) HWCONFIG_OFFSET, HWCONFIG_DEVICE);
		} else {
			if (fread(&config, sizeof(config), 1, fp) < 1 || ferror(fp) != 0) {
				WARN("Failed to read the NTX HWConfig entry on '%s'", HWCONFIG_DEVICE);
				fclose(fp);
				// NOTE: Make it clear we failed to identify the device...
				set_kobo_quirks(0);
				return;
			}

			// NOTE: These are NOT NULL-terminated, so we use the size of the storage array,
			//       and not of the string literal (where sizeof would have appended space for a terminating NULL).
			//       (i.e., here, sizeof(config.magic) == 10 == strlen(HWCONFIG_MAGIC) while sizeof(HWCONFIG_MAGIC) == 11)
			if (memcmp(config.magic, HWCONFIG_MAGIC, sizeof(config.magic)) != 0) {
				WARN("Input device '%s' does not appear to contain an NTX HWConfig entry",
				     HWCONFIG_DEVICE);
				fclose(fp);
				// NOTE: Like rcS, assume it's an old Freescale Trilogy if we can't find an NTX HW tag
				set_kobo_quirks(0);
				return;
			}

			// We'll read the full payload, whose size varies depending on the exact kernel being used...
			// Since it's only a couple dozen bytes, do that on the stack to make our life easier.
			// cppcheck-suppress allocaCalled
			payload = alloca(config.len);
			if (fread(payload, sizeof(*payload), config.len, fp) < config.len || ferror(fp) != 0) {
				WARN("Error reading NTX HWConfig payload (unexpected length)");
				fclose(fp);
				// NOTE: Make it clear we failed to identify the device...
				set_kobo_quirks(0);
				return;
			}

			// We'll also need the total storage space to discriminate 32GB devices...
			if (ioctl(fileno(fp), BLKGETSIZE64, &storagesize)) {
				// Make that non-fatal, as the distinction is purely cosmetic for our purposes
				WARN("Error requesting block device size: %m");
			}
		}
		fclose(fp);

		// As per /bin/kobo_config.sh, match PCB IDs to Product IDs via a LUT...
		unsigned short int kobo_id = 0U;
		// Mainly to make GCC happy, because if alloca failed, we're screwed anyway.
		if (payload) {
			/*
			// NOTE: Dump the full payload, for science!
			for (uint8_t i = 0; i < config.len; i++) {
				ELOG("NTXHWConfig[%hhu] -> %hhu", i, payload[i]);
			}
			*/

			if (payload[KOBO_HWCFG_PCB] >= (sizeof(kobo_ids) / sizeof(*kobo_ids))) {
				WARN("Unknown Kobo PCB ID index (%hhu >= %zu)",
				     payload[KOBO_HWCFG_PCB],
				     (sizeof(kobo_ids) / sizeof(*kobo_ids)));
			} else {
				kobo_id = kobo_ids[payload[KOBO_HWCFG_PCB]];

				// And now for the fun part, the few device variants that use the same PCB ID...
				if (kobo_id == 374U || kobo_id == 375U) {
					// Discriminate the Mk.7 version for dual rev models by checking the CPU...
					// NOTE: kobo_cpus[10] == "mx6sll"
					if (payload[KOBO_HWCFG_CPU] == 10U) {
						// Thankfully, that works for both the H2O² (374 -> 378),
						// and the Aura SE (375 -> 379) ;)
						kobo_id = (unsigned short int) (kobo_id + 4U);
					}
				} else if (kobo_id == 371U) {
					// Discriminate Alyssum from Pika, by checking the Display Resolution...
					// NOTE: kobo_disp_res[0] == "800x600"
					if (payload[KOBO_HWCFG_DisplayResolution] == 0U) {
						// Glo HD (Alyssum) [371] -> Touch 2.0 (Pika) [372]
						kobo_id = 372U;
					}
				} else if (kobo_id == 373U || kobo_id == 377U) {
					// Discriminate 32GB variants...
					// NOTE: We compare against 8GB, but in practice, given storage shenanigans and
					//       the truncation involved here, we end up with 7 on 8GB devices ;).
					if ((storagesize / (1024U * 1024U * 1024U)) > 8U) {
						if (kobo_id == 373U) {
							// Aura ONE (daylight) [373] -> Aura ONE LE (daylight) [381]
							kobo_id = 381U;
						} else if (kobo_id == 377U) {
							// Forma (frost) [377] -> Forma 32GB (frost) [380]
							kobo_id = 380U;
						}
					}
				}
			}
		} else {
			// Should hopefully never happen, since there's a good chance we'd have caught a SIGSEGV before that,
			// if alloca failed ;).
			WARN("Empty NTX HWConfig payload?");
		}
		// And now we can do this, as accurately as if onboard were mounted ;).
		set_kobo_quirks(kobo_id);
	}
}
#	elif defined(FBINK_FOR_REMARKABLE)
static void
    identify_remarkable(void)
{
	deviceQuirks.screenDPI   = 226;
	deviceQuirks.canHWInvert = true;
	// NOTE: Not actually an NTX board (AFAIK), but useful nonetheless for fbdepth ;).
	deviceQuirks.ntxBootRota  = FB_ROTATE_CW;
	deviceQuirks.ntxRotaQuirk = NTX_ROTA_SANE;
	deviceQuirks.canRotate    = true;
	// Flawfinder: ignore
	strncpy(deviceQuirks.deviceName, "reMarkable", sizeof(deviceQuirks.deviceName) - 1U);
	// Flawfinder: ignore
	strncpy(deviceQuirks.deviceCodename, "Zero Gravitas", sizeof(deviceQuirks.deviceCodename) - 1U);
}
#	endif    // FBINK_FOR_KINDLE

static void
    identify_device(void)
{
#	if defined(FBINK_FOR_KINDLE)
	identify_kindle();
	if (deviceQuirks.deviceId > 0xFFu) {
		char* restrict dev_id    = NULL;
		dev_id                   = to_base(deviceQuirks.deviceId, 32U);
		const char* restrict pad = "000";
		ELOG("Detected a Kindle %s (%.*s%s -> 0x%03X => %s on %s)",
		     deviceQuirks.deviceName,
		     ((int) strlen(pad) < (int) strlen(dev_id))    // Flawfinder: ignore
			 ? 0
			 : (int) strlen(pad) - (int) strlen(dev_id),    // Flawfinder: ignore
		     pad,
		     dev_id,
		     deviceQuirks.deviceId,
		     deviceQuirks.deviceCodename,
		     deviceQuirks.devicePlatform);
		free(dev_id);
	} else {
		ELOG("Detected a Kindle %s (0x%02X => %s on %s)",
		     deviceQuirks.deviceName,
		     deviceQuirks.deviceId,
		     deviceQuirks.deviceCodename,
		     deviceQuirks.devicePlatform);
	}
#	elif defined(FBINK_FOR_CERVANTES)
	identify_cervantes();
	ELOG("Detected a BQ Cervantes %s (%hu)", deviceQuirks.deviceName, deviceQuirks.deviceId);
#	elif defined(FBINK_FOR_KOBO)
	identify_kobo();
	ELOG("Detected a Kobo %s (%hu => %s @ %s)",
	     deviceQuirks.deviceName,
	     deviceQuirks.deviceId,
	     deviceQuirks.deviceCodename,
	     deviceQuirks.devicePlatform);
#	elif defined(FBINK_FOR_REMARKABLE)
	identify_remarkable();
	ELOG("Detected a reMarkable (%s)", deviceQuirks.deviceCodename);
#	endif
	// Warn if canHWInvert was flipped
	if (!deviceQuirks.canHWInvert) {
		ELOG("This device does not support HW inversion");
	}
}
#endif    // !FBINK_FOR_LINUX
