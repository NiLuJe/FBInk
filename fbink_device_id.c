/*
	FBInk: FrameBuffer eInker, a library to print text & images to an eInk Linux framebuffer
	Copyright (C) 2018-2024 NiLuJe <ninuje@gmail.com>
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
			strtcpy(deviceQuirks.deviceName, "1", sizeof(deviceQuirks.deviceName));
			strtcpy(deviceQuirks.deviceCodename, "Fiona", sizeof(deviceQuirks.deviceCodename));
			// NOTE: Possibly Fiona, too, as ADS was a proto.
			//       But later models have shown than a proto's board name often ends up naming the platform...
			strtcpy(deviceQuirks.devicePlatform, "ADS", sizeof(deviceQuirks.devicePlatform));
			return true;
		case 0x02u:    // K2
		case 0x03u:
			deviceQuirks.isKindleLegacy = true;
			strtcpy(deviceQuirks.deviceName, "2", sizeof(deviceQuirks.deviceName));
			strtcpy(deviceQuirks.deviceCodename, "Turing", sizeof(deviceQuirks.deviceCodename));
			strtcpy(deviceQuirks.devicePlatform, "Mario", sizeof(deviceQuirks.devicePlatform));
			return true;
		case 0x04u:    // KDX
		case 0x05u:
		case 0x09u:
			deviceQuirks.isKindleLegacy = true;
			deviceQuirks.screenDPI      = 150U;
			strtcpy(deviceQuirks.deviceName, "DX", sizeof(deviceQuirks.deviceName));
			strtcpy(deviceQuirks.deviceCodename, "Nell", sizeof(deviceQuirks.deviceCodename));
			// NOTE: Possibly Banjo? Never figured out if that platform ever saw the light of day...
			strtcpy(deviceQuirks.devicePlatform, "Mario", sizeof(deviceQuirks.devicePlatform));
			return true;
		case 0x08u:    // K3
		case 0x06u:
		case 0x0Au:
			deviceQuirks.isKindleLegacy = true;
			strtcpy(deviceQuirks.deviceName, "3", sizeof(deviceQuirks.deviceName));
			strtcpy(deviceQuirks.deviceCodename, "Shasta", sizeof(deviceQuirks.deviceCodename));
			strtcpy(deviceQuirks.devicePlatform, "Luigi", sizeof(deviceQuirks.devicePlatform));
			return true;
		case 0x0Eu:    // K4
			deviceQuirks.isKindleLegacy = true;
			strtcpy(deviceQuirks.deviceName, "4", sizeof(deviceQuirks.deviceName));
			strtcpy(deviceQuirks.deviceCodename, "Tequila", sizeof(deviceQuirks.deviceCodename));
			strtcpy(deviceQuirks.devicePlatform, "Yoshi", sizeof(deviceQuirks.devicePlatform));
			return true;
		case 0x0Fu:    // K5
		case 0x11u:
		case 0x10u:
		case 0x12u:
			deviceQuirks.isKindlePearlScreen = true;
			strtcpy(deviceQuirks.deviceName, "Touch", sizeof(deviceQuirks.deviceName));
			strtcpy(deviceQuirks.deviceCodename, "Whitney", sizeof(deviceQuirks.deviceCodename));
			strtcpy(deviceQuirks.devicePlatform, "Yoshi", sizeof(deviceQuirks.devicePlatform));
			return true;
		case 0x23u:    // K4b
			deviceQuirks.isKindleLegacy = true;
			strtcpy(deviceQuirks.deviceName, "4", sizeof(deviceQuirks.deviceName));
			strtcpy(deviceQuirks.deviceCodename, "Sauza", sizeof(deviceQuirks.deviceCodename));
			strtcpy(deviceQuirks.devicePlatform, "Yoshi", sizeof(deviceQuirks.devicePlatform));
			return true;
		case 0x24u:    // PW1
		case 0x1Bu:
		case 0x1Cu:
		case 0x1Du:
		case 0x1Fu:
		case 0x20u:
			deviceQuirks.isKindlePearlScreen = true;
			deviceQuirks.screenDPI           = 212U;
			strtcpy(deviceQuirks.deviceName, "PaperWhite", sizeof(deviceQuirks.deviceName));
			strtcpy(deviceQuirks.deviceCodename, "Celeste", sizeof(deviceQuirks.deviceCodename));
			// NOTE: AKA Yoshime3
			strtcpy(deviceQuirks.devicePlatform, "Yoshime", sizeof(deviceQuirks.devicePlatform));
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
			strtcpy(deviceQuirks.deviceName, "PaperWhite 2", sizeof(deviceQuirks.deviceName));
			strtcpy(deviceQuirks.deviceCodename, "Pinot", sizeof(deviceQuirks.deviceCodename));
			strtcpy(deviceQuirks.devicePlatform, "Wario", sizeof(deviceQuirks.devicePlatform));
			return true;
		case 0xC6u:    // KT2
			strtcpy(deviceQuirks.deviceName, "Basic", sizeof(deviceQuirks.deviceName));
			strtcpy(deviceQuirks.deviceCodename, "Bourbon", sizeof(deviceQuirks.deviceCodename));
			strtcpy(deviceQuirks.devicePlatform, "Wario", sizeof(deviceQuirks.devicePlatform));
			return true;
		case 0x13u:    // KV
			deviceQuirks.screenDPI = 300U;
			strtcpy(deviceQuirks.deviceName, "Voyage", sizeof(deviceQuirks.deviceName));
			strtcpy(deviceQuirks.deviceCodename, "Icewine", sizeof(deviceQuirks.deviceCodename));
			strtcpy(deviceQuirks.devicePlatform, "Wario", sizeof(deviceQuirks.devicePlatform));
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
			strtcpy(deviceQuirks.deviceName, "Voyage", sizeof(deviceQuirks.deviceName));
			strtcpy(deviceQuirks.deviceCodename, "Icewine", sizeof(deviceQuirks.deviceCodename));
			strtcpy(deviceQuirks.devicePlatform, "Wario", sizeof(deviceQuirks.devicePlatform));
			return true;
		case 0x07u:    // ??
		case 0x0Bu:
		case 0x0Cu:
		case 0x0Du:
		case 0x99u:
			return true;
		case 0xDDu:    // KT2 AUS
			strtcpy(deviceQuirks.deviceName, "Basic", sizeof(deviceQuirks.deviceName));
			strtcpy(deviceQuirks.deviceCodename, "Bourbon", sizeof(deviceQuirks.deviceCodename));
			strtcpy(deviceQuirks.devicePlatform, "Wario", sizeof(deviceQuirks.devicePlatform));
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
			strtcpy(deviceQuirks.deviceName, "PaperWhite 3", sizeof(deviceQuirks.deviceName));
			strtcpy(deviceQuirks.deviceCodename, "Muscat", sizeof(deviceQuirks.deviceCodename));
			strtcpy(deviceQuirks.devicePlatform, "Wario", sizeof(deviceQuirks.devicePlatform));
			return true;
		case 0x20Cu:    // KOA
		case 0x20Du:
		case 0x219u:
		case 0x21Au:
		case 0x21Bu:
		case 0x21Cu:
			deviceQuirks.screenDPI = 300U;
			strtcpy(deviceQuirks.deviceName, "Oasis", sizeof(deviceQuirks.deviceName));
			strtcpy(deviceQuirks.deviceCodename, "Whisky", sizeof(deviceQuirks.deviceCodename));
			strtcpy(deviceQuirks.devicePlatform, "Duet", sizeof(deviceQuirks.devicePlatform));
			return true;
		case 0x1BCu:    // KT3
		case 0x269u:
		case 0x26Au:
			strtcpy(deviceQuirks.deviceName, "Basic 2", sizeof(deviceQuirks.deviceName));
			strtcpy(deviceQuirks.deviceCodename, "Eanab", sizeof(deviceQuirks.deviceCodename));
			strtcpy(deviceQuirks.devicePlatform, "Heisenberg", sizeof(deviceQuirks.devicePlatform));
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
			deviceQuirks.hasEclipseWfm = true;
			deviceQuirks.screenDPI     = 300U;
			strtcpy(deviceQuirks.deviceName, "Oasis 2", sizeof(deviceQuirks.deviceName));
			strtcpy(deviceQuirks.deviceCodename, "Cognac", sizeof(deviceQuirks.deviceCodename));
			strtcpy(deviceQuirks.devicePlatform, "Zelda", sizeof(deviceQuirks.devicePlatform));
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
			strtcpy(deviceQuirks.deviceName, "PaperWhite 4", sizeof(deviceQuirks.deviceName));
			strtcpy(deviceQuirks.deviceCodename, "Moonshine", sizeof(deviceQuirks.deviceCodename));
			strtcpy(deviceQuirks.devicePlatform, "Rex", sizeof(deviceQuirks.devicePlatform));
			return true;
		case 0x414u:    // KT4
		case 0x3CFu:
		case 0x3D0u:
		case 0x3D1u:
		case 0x3D2u:
		case 0x3ABu:
			deviceQuirks.isKindleRex = true;
			strtcpy(deviceQuirks.deviceName, "Basic 3", sizeof(deviceQuirks.deviceName));
			strtcpy(deviceQuirks.deviceCodename, "Jaeger", sizeof(deviceQuirks.deviceCodename));
			strtcpy(deviceQuirks.devicePlatform, "Rex", sizeof(deviceQuirks.devicePlatform));
			return true;
		case 0x434u:    // KOA3
		case 0x3D8u:
		case 0x3D7u:
		case 0x3D6u:
		case 0x3D5u:
		case 0x3D4u:
			deviceQuirks.isKindleZelda = true;
			deviceQuirks.hasEclipseWfm = true;
			deviceQuirks.screenDPI     = 300U;
			strtcpy(deviceQuirks.deviceName, "Oasis 3", sizeof(deviceQuirks.deviceName));
			strtcpy(deviceQuirks.deviceCodename, "Stinger", sizeof(deviceQuirks.deviceCodename));
			strtcpy(deviceQuirks.devicePlatform, "Zelda", sizeof(deviceQuirks.devicePlatform));
			return true;
		case 0x690u:    // PW5
		case 0x700u:
		case 0x6FFu:
		case 0x7ADu:
		case 0x829u:
		case 0x82Au:
		case 0x971u:
		case 0x972u:
		case 0x9B3u:
			deviceQuirks.isMTK         = true;
			deviceQuirks.hasEclipseWfm = true;
			deviceQuirks.screenDPI     = 300U;
			strtcpy(deviceQuirks.deviceName, "PaperWhite 5", sizeof(deviceQuirks.deviceName));
			strtcpy(deviceQuirks.deviceCodename, "Malbec", sizeof(deviceQuirks.deviceCodename));
			strtcpy(deviceQuirks.devicePlatform, "Bellatrix", sizeof(deviceQuirks.devicePlatform));
			return true;
		case 0x84Du:    // KT5
		case 0x8BBu:
		case 0x86Au:
		case 0x958u:
		case 0x957u:
		case 0x7F1u:
		case 0x84Cu:
			deviceQuirks.isMTK         = true;
			deviceQuirks.hasEclipseWfm = true;
			deviceQuirks.screenDPI     = 300U;
			strtcpy(deviceQuirks.deviceName, "Basic 4", sizeof(deviceQuirks.deviceName));
			strtcpy(deviceQuirks.deviceCodename, "Cava", sizeof(deviceQuirks.deviceCodename));
			strtcpy(deviceQuirks.devicePlatform, "Bellatrix", sizeof(deviceQuirks.devicePlatform));
			return true;
		case 0x8F2u:    // KS
		case 0x974u:
		case 0x8C3u:
		case 0x847u:
		case 0x975u:
		case 0x874u:
		case 0x875u:
		case 0x8E0u:
			deviceQuirks.isMTK         = true;
			deviceQuirks.hasEclipseWfm = true;
			deviceQuirks.screenDPI     = 300U;
			strtcpy(deviceQuirks.deviceName, "Scribe", sizeof(deviceQuirks.deviceName));
			strtcpy(deviceQuirks.deviceCodename, "Barolo", sizeof(deviceQuirks.deviceCodename));
			strtcpy(deviceQuirks.devicePlatform, "Bellatrix3", sizeof(deviceQuirks.devicePlatform));
			return true;
		case 0xE85u:    // KT6
		case 0xE86u:
		case 0xE84u:
		case 0xE83u:
		case 0x2909u:
		case 0xE82u:
		case 0xE75u:
			deviceQuirks.isMTK         = true;
			deviceQuirks.hasEclipseWfm = true;
			deviceQuirks.screenDPI     = 300U;
			strtcpy(deviceQuirks.deviceName, "Basic 5", sizeof(deviceQuirks.deviceName));
			strtcpy(deviceQuirks.deviceCodename, "Rossini", sizeof(deviceQuirks.deviceCodename));
			strtcpy(deviceQuirks.devicePlatform, "Bellatrix", sizeof(deviceQuirks.devicePlatform));
			return true;
		case 0xC89u:    // PW6
		case 0xC86u:
		case 0xC7Fu:
		case 0xC7Eu:
		case 0xE2Au:
		case 0xE25u:
		case 0xE23u:
		case 0xE28u:
		case 0xE45u:
		case 0xE5Au:
			deviceQuirks.isMTK              = true;
			deviceQuirks.isKindleBellatrix4 = true;
			deviceQuirks.hasEclipseWfm      = true;
			deviceQuirks.screenDPI          = 300U;
			strtcpy(deviceQuirks.deviceName, "PaperWhite 5", sizeof(deviceQuirks.deviceName));
			strtcpy(deviceQuirks.deviceCodename, "Sangria", sizeof(deviceQuirks.deviceCodename));
			strtcpy(deviceQuirks.devicePlatform, "Bellatrix4", sizeof(deviceQuirks.devicePlatform));
			return true;
		case 0xFA0u:    // KS2
		case 0xFA1u:
		case 0xFE5u:
		case 0xF9Du:
		case 0xFE4u:
		case 0xFE3u:
		case 0x102Eu:
		case 0x102Du:
			deviceQuirks.isMTK         = true;
			deviceQuirks.hasEclipseWfm = true;
			deviceQuirks.screenDPI     = 300U;
			strtcpy(deviceQuirks.deviceName, "Scribe 2", sizeof(deviceQuirks.deviceName));
			strtcpy(deviceQuirks.deviceCodename, "Pisco", sizeof(deviceQuirks.deviceCodename));
			strtcpy(deviceQuirks.devicePlatform, "Bellatrix3", sizeof(deviceQuirks.devicePlatform));
			return true;
		case 0xE29u:    // CS
		case 0xE24u:
		case 0xE2Bu:
		case 0xE26u:
		case 0xE22u:
		case 0xC9Fu:
		case 0xE27u:
		case 0xE5Bu:
		case 0xE46u:
		case 0x10A6u:
			deviceQuirks.isMTK              = true;
			deviceQuirks.isKindleBellatrix4 = true;
			deviceQuirks.hasEclipseWfm      = true;
			deviceQuirks.screenDPI          = 300U;
			deviceQuirks.hasColorPanel      = true;
			strtcpy(deviceQuirks.deviceName, "ColorSoft", sizeof(deviceQuirks.deviceName));
			strtcpy(deviceQuirks.deviceCodename, "SeaBreeze", sizeof(deviceQuirks.deviceCodename));
			strtcpy(deviceQuirks.devicePlatform, "Bellatrix4", sizeof(deviceQuirks.devicePlatform));
			return true;
		default:
			strtcpy(deviceQuirks.deviceName, "Unknown!", sizeof(deviceQuirks.deviceName));
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
		bool match = false;
		for (uint8_t i = 0; tbl[i] != '\0'; i++) {
			if (*p == tbl[i]) {
				match  = true;
				result = result * base + i;
				break;
			}
		}
		// If we reach this, we ran through the full Base32 table without a match, this is bad!
		if (!match) {
			WARN("Input character '%c' is out of range", *p);
			return 0;
		}
	}

	return result;
}

// NOTE: This is from KindleTool,
//       c.f., https://github.com/NiLuJe/KindleTool/blob/master/KindleTool/convert.c#L51
// Pilfered from http://rosettacode.org/wiki/Non-decimal_radices/Convert#C
static char*
    to_base(int64_t num, uint8_t base, size_t min_output_columns)
{
	// NOTE: Crockford's Base32, but with the "L" & "U" re-added in?
	const char tbl[]   = "0123456789ABCDEFGHJKLMNPQRSTUVWX";
	char       buf[66] = { 0 };
	char*      out     = NULL;
	uint64_t   n;
	size_t     len     = 0U;
	size_t     padding = 0U;
	bool       neg     = false;

	if (base >= sizeof(tbl)) {
		WARN("base %hhu is unsupported (too large).", base);
		return NULL;
	}

	// safe against most negative integer
	n = (neg = (num < 0)) ? (uint64_t) (~num) + 1 : (uint64_t) num;

	do {
		buf[len++] = tbl[n % base];
	} while (n /= base);

	// Ensure we pad with at least min_output_columns zeroes
	if (len < min_output_columns) {
		padding = min_output_columns - len;
	}
	out = calloc(len + padding + neg + 1U, sizeof(*out));
	if (out == NULL) {
		PFWARN("Error allocating base32 output string buffer: %m");
		return NULL;
	}
	for (size_t i = neg + padding; len > 0U; i++) {
		out[i] = buf[--len];
	}
	while (padding) {
		out[neg + --padding] = '0';
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
		PFWARN("Cannot open /proc/usid (%m). Not running on a Kindle?");
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
		PFWARN("Couldn't read from `%s` (%m), unable to identify the Cervantes model", HWCONFIG_DEVICE);
	} else {
#		pragma GCC diagnostic push
#		pragma GCC diagnostic ignored "-Wmissing-braces"
		NTXHWConfig config = { 0 };
#		pragma GCC diagnostic pop

		if (fseek(fp, HWCONFIG_OFFSET, SEEK_SET) != 0) {
			PFWARN("Failed to seek to position 0x%p in `%s`: %m", (void*) HWCONFIG_OFFSET, HWCONFIG_DEVICE);
		} else {
			if (fread(&config, sizeof(config), 1, fp) < 1 || ferror(fp) != 0) {
				WARN("Failed to read the NTX HWConfig entry on `%s`", HWCONFIG_DEVICE);
				fclose(fp);
				return;
			}
		}
		fclose(fp);

		// NOTE: This slice will NOT be NULL-terminated,
		//       so we have to chop the terminating NULL from the string literal's size.
		if (memcmp(config.magic, HWCONFIG_MAGIC, sizeof(HWCONFIG_MAGIC) - 1U) != 0) {
			WARN("Block device `%s` does not appear to contain an NTX HWConfig entry", HWCONFIG_DEVICE);
			return;
		}

		// Store the device ID...
		deviceQuirks.deviceId                   = config.pcb_id;
		// Some devices *may* be based on the same board as the Kobo Aura, so, let's be cautious...
		deviceQuirks.canHWInvert                = false;
		// NOTE: See the comments in set_kobo_quirks about this.
		//       I'm hoping there aren't any special snowflakes in the lineup...
		deviceQuirks.ntxBootRota                = FB_ROTATE_UD;
		// Standard NTX rotation quirks...
		deviceQuirks.ntxRotaQuirk               = NTX_ROTA_STRAIGHT;
		deviceQuirks.rotationMap[FB_ROTATE_UR]  = FB_ROTATE_CW;
		deviceQuirks.rotationMap[FB_ROTATE_CW]  = FB_ROTATE_UD;
		deviceQuirks.rotationMap[FB_ROTATE_UD]  = FB_ROTATE_CCW;
		deviceQuirks.rotationMap[FB_ROTATE_CCW] = FB_ROTATE_UR;
		deviceQuirks.touchSwapAxes              = true;
		deviceQuirks.touchMirrorX               = true;
		// List of supported devices,
		// c.f., https://github.com/bq/cervantes/blob/master/bqHAL/Devices/mx508/src/DeviceInfoMx508.cpp#L33-L37
		switch (config.pcb_id) {
			case DEVICE_CERVANTES_TOUCH:    // BQ Cervantes Touch - Fnac Touch (2012-2013)
				deviceQuirks.screenDPI = 167U;
				strtcpy(deviceQuirks.deviceName, "Touch", sizeof(deviceQuirks.deviceName));
				break;
			case DEVICE_CERVANTES_TOUCHLIGHT:    // BQ Cervantes TouchLight - Fnac Touch Plus (2012-2013)
				deviceQuirks.screenDPI = 167U;
				strtcpy(deviceQuirks.deviceName, "TouchLight", sizeof(deviceQuirks.deviceName));
				break;
			case DEVICE_CERVANTES_2013:    // BQ Cervantes 2013 - Fnac Touch Light (2013)
				deviceQuirks.screenDPI = 212U;
				strtcpy(deviceQuirks.deviceName, "2013", sizeof(deviceQuirks.deviceName));
				break;
			case DEVICE_CERVANTES_3:    // BQ Cervantes 3 - Fnac Touch Light 2 (2016)
				deviceQuirks.screenDPI = 300U;
				strtcpy(deviceQuirks.deviceName, "3", sizeof(deviceQuirks.deviceName));
				break;
			case DEVICE_CERVANTES_4:    // BQ Cervantes 4
				deviceQuirks.screenDPI = 300U;
				strtcpy(deviceQuirks.deviceName, "4", sizeof(deviceQuirks.deviceName));
				break;
			default:
				WARN("Unidentified Cervantes device (%hhu)", config.pcb_id);
				strtcpy(deviceQuirks.deviceName, "Unknown!", sizeof(deviceQuirks.deviceName));
				break;
		}
	}
}
#	elif defined(FBINK_FOR_KOBO)
static void
    set_kobo_quirks(unsigned short int kobo_id)
{
	// Attempt to discriminate between the Touch A & B variants...
	// NOTE: This will only happen if we actually found a Nickel version tag,
	//       the other detection methods will simply report DEVICE_UNKNOWN on a A.
	if (unlikely(kobo_id == DEVICE_KOBO_TOUCH_B)) {
		const char* platform = getenv("PLATFORM");
		if (likely(platform)) {
			// The A is *not* an NTX board
			if (strcmp(platform, "freescale") == 0) {
				kobo_id = DEVICE_KOBO_TOUCH_A;
			}
		} else {
			// If our env doesn't provide PLATFORM, check for the NTX HWConfig block ourselves,
			// with a bit of code duplication from identify_kobo...
			FILE* fp = fopen(HWCONFIG_DEVICE, "re");
			if (!fp) {
				PFWARN("Couldn't read from `%s` (%m)", HWCONFIG_DEVICE);
				kobo_id = DEVICE_UNKNOWN;
			} else {
#		pragma GCC diagnostic push
#		pragma GCC diagnostic ignored "-Wmissing-braces"
				NTXHWConfig config = { 0 };
#		pragma GCC diagnostic pop

				if (fseek(fp, HWCONFIG_OFFSET, SEEK_SET) != 0) {
					PFWARN("Failed to seek to position 0x%p in `%s`: %m",
					       (void*) HWCONFIG_OFFSET,
					       HWCONFIG_DEVICE);
					kobo_id = DEVICE_UNKNOWN;
				} else {
					if (fread(&config, sizeof(config), 1, fp) < 1 || ferror(fp) != 0) {
						WARN("Failed to read the NTX HWConfig entry on `%s`", HWCONFIG_DEVICE);
						kobo_id = DEVICE_UNKNOWN;
					}

					if (memcmp(config.magic, HWCONFIG_MAGIC, sizeof(HWCONFIG_MAGIC) - 1U) != 0) {
						WARN("Block device `%s` does not appear to contain an NTX HWConfig entry",
						     HWCONFIG_DEVICE);
						// Not an NTX board, it's a A :)
						kobo_id = DEVICE_KOBO_TOUCH_A;
					}
				}
				fclose(fp);
			}
		}
	}

	// Store the device ID...
	deviceQuirks.deviceId                   = kobo_id;
	// HW invert should *generally* be safe on Kobo, with a few exceptions...
	deviceQuirks.canHWInvert                = true;
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
	deviceQuirks.ntxBootRota                = FB_ROTATE_UD;
	// NOTE: Most kernels thankfully don't resort to weird rotation quirks ;).
	//       c.f., mxc_epdc_fb_check_var @ drivers/video/fbdev/mxc/mxc_epdc_v2_fb.c
	deviceQuirks.ntxRotaQuirk               = NTX_ROTA_STRAIGHT;
	// NOTE: As far as handling touch coordinates translation, ntxBootRota & ntxRotaQuirk should be considered *deprecated*,
	//       rotationMap, paired with touchSwapAxes & touchMirrorX/touchMirrorY all paint a much clearer picture...
	// Most common native -> canonical rotation mapping; {1, 2, 3, 0}
	// (index is native, value is canonical; beware of that because most custom maps happen to match both ways!)
	deviceQuirks.rotationMap[FB_ROTATE_UR]  = FB_ROTATE_CW;
	deviceQuirks.rotationMap[FB_ROTATE_CW]  = FB_ROTATE_UD;
	deviceQuirks.rotationMap[FB_ROTATE_UD]  = FB_ROTATE_CCW;
	deviceQuirks.rotationMap[FB_ROTATE_CCW] = FB_ROTATE_UR;
	// Most common touch panel setup (while the swap is currently a constant, the mirroring varies on a few devices).
	// NOTE: This one *happens* to match a sane orientation (touch panel is oriented CCW, i.e., origin at the top-right corner),
	//       but this isn't always the case!
	//       This is why we break it down in terms of swap & mirror flags, instead of a simple "touch-to-UR" offset in units of 90° rotations.
	deviceQuirks.touchSwapAxes              = true;
	deviceQuirks.touchMirrorX               = true;
	// NOTE: Device code list pilfered from
	//       https://github.com/pgaskin/KoboStuff/blob/gh-pages/kobofirmware.js#L11
	//       See also https://github.com/pgaskin/koboutils/pull/1 and the links referenced there
	//       for all you ever wanted to know about Kobo codenames ;).
	switch (kobo_id) {
		case DEVICE_KOBO_TOUCH_A:    // Touch A (trilogy, freescale)
			deviceQuirks.isKoboNonMT = true;
			strtcpy(deviceQuirks.deviceName, "Touch A", sizeof(deviceQuirks.deviceCodename));
			strtcpy(deviceQuirks.deviceCodename, "Trilogy", sizeof(deviceQuirks.deviceCodename));
			strtcpy(deviceQuirks.devicePlatform, "Mark 3", sizeof(deviceQuirks.devicePlatform));
			break;
		case DEVICE_KOBO_TOUCH_B:    // Touch B (trilogy)
			deviceQuirks.isKoboNonMT = true;
			strtcpy(deviceQuirks.deviceName, "Touch B", sizeof(deviceQuirks.deviceName));
			strtcpy(deviceQuirks.deviceCodename, "Trilogy", sizeof(deviceQuirks.deviceCodename));
			strtcpy(deviceQuirks.devicePlatform, "Mark 3", sizeof(deviceQuirks.devicePlatform));
			break;
		case DEVICE_KOBO_TOUCH_C:    // Touch C (trilogy)
			deviceQuirks.isKoboNonMT = true;
			strtcpy(deviceQuirks.deviceName, "Touch C", sizeof(deviceQuirks.deviceName));
			strtcpy(deviceQuirks.deviceCodename, "Trilogy", sizeof(deviceQuirks.deviceCodename));
			strtcpy(deviceQuirks.devicePlatform, "Mark 4", sizeof(deviceQuirks.devicePlatform));
			break;
		case DEVICE_KOBO_MINI:    // Mini (pixie)
			deviceQuirks.isKoboNonMT = true;
			deviceQuirks.screenDPI   = 200U;
			strtcpy(deviceQuirks.deviceName, "Mini", sizeof(deviceQuirks.deviceName));
			strtcpy(deviceQuirks.deviceCodename, "Pixie", sizeof(deviceQuirks.deviceCodename));
			strtcpy(deviceQuirks.devicePlatform, "Mark 4", sizeof(deviceQuirks.devicePlatform));
			break;
		case DEVICE_KOBO_GLO:    // Glo (kraken)
			deviceQuirks.isKoboNonMT                = true;
			// {3, 2, 1, 0}, but no ioctl quirks
			deviceQuirks.ntxRotaQuirk               = NTX_ROTA_CCW_TOUCH;
			deviceQuirks.rotationMap[FB_ROTATE_UR]  = FB_ROTATE_CCW;
			deviceQuirks.rotationMap[FB_ROTATE_CW]  = FB_ROTATE_UD;
			deviceQuirks.rotationMap[FB_ROTATE_UD]  = FB_ROTATE_CW;
			deviceQuirks.rotationMap[FB_ROTATE_CCW] = FB_ROTATE_UR;
			deviceQuirks.screenDPI                  = 212U;
			strtcpy(deviceQuirks.deviceName, "Glo", sizeof(deviceQuirks.deviceName));
			strtcpy(deviceQuirks.deviceCodename, "Kraken", sizeof(deviceQuirks.deviceCodename));
			strtcpy(deviceQuirks.devicePlatform, "Mark 4", sizeof(deviceQuirks.devicePlatform));
			break;
		case DEVICE_KOBO_GLO_HD:    // Glo HD (alyssum)
			deviceQuirks.screenDPI = 300U;
			strtcpy(deviceQuirks.deviceName, "Glo HD", sizeof(deviceQuirks.deviceName));
			strtcpy(deviceQuirks.deviceCodename, "Alyssum", sizeof(deviceQuirks.deviceCodename));
			strtcpy(deviceQuirks.devicePlatform, "Mark 6", sizeof(deviceQuirks.devicePlatform));
			break;
		case DEVICE_TOLINO_SHINE_2HD:    // Tolino Shine 2HD (shine2) (~ Glo HD)
			deviceQuirks.screenDPI = 300U;
			strtcpy(deviceQuirks.deviceName, "Shine 2HD", sizeof(deviceQuirks.deviceName));
			strtcpy(deviceQuirks.deviceCodename, "Shine2", sizeof(deviceQuirks.deviceCodename));
			strtcpy(deviceQuirks.devicePlatform, "Mark 6", sizeof(deviceQuirks.devicePlatform));
			break;
		case DEVICE_KOBO_TOUCH_2:    // Touch 2.0 (pika)
			strtcpy(deviceQuirks.deviceName, "Touch 2.0", sizeof(deviceQuirks.deviceName));
			strtcpy(deviceQuirks.deviceCodename, "Pika", sizeof(deviceQuirks.deviceCodename));
			strtcpy(deviceQuirks.devicePlatform, "Mark 6", sizeof(deviceQuirks.devicePlatform));
			break;
		case DEVICE_KOBO_AURA:    // Aura (phoenix)
			// NOTE: The bottom 10 pixels are blacked out by Nickel (behind the bezel)
			deviceQuirks.koboVertOffset = -10;
			// NOTE: According to the nightmode hack, the Aura's kernel *may* be crashy w/ PxP inversion...
			deviceQuirks.canHWInvert    = false;
			deviceQuirks.screenDPI      = 212U;
			strtcpy(deviceQuirks.deviceName, "Aura", sizeof(deviceQuirks.deviceName));
			strtcpy(deviceQuirks.deviceCodename, "Phoenix", sizeof(deviceQuirks.deviceCodename));
			strtcpy(deviceQuirks.devicePlatform, "Mark 5", sizeof(deviceQuirks.devicePlatform));
			break;
		case DEVICE_KOBO_AURA_HD:    // Aura HD (dragon)
			deviceQuirks.isKoboNonMT                = true;
			// NOTE: Boot rotation is FB_ROTATE_UR, pickel is FB_ROTATE_UD, nickel is FB_ROTATE_CW
			deviceQuirks.ntxBootRota                = FB_ROTATE_UR;
			deviceQuirks.ntxRotaQuirk               = NTX_ROTA_ALL_INVERTED;
			// {1, 0, 3, 2}
			deviceQuirks.rotationMap[FB_ROTATE_UR]  = FB_ROTATE_CW;
			deviceQuirks.rotationMap[FB_ROTATE_CW]  = FB_ROTATE_UR;
			deviceQuirks.rotationMap[FB_ROTATE_UD]  = FB_ROTATE_CCW;
			deviceQuirks.rotationMap[FB_ROTATE_CCW] = FB_ROTATE_UD;
			deviceQuirks.screenDPI                  = 265U;
			strtcpy(deviceQuirks.deviceName, "Aura HD", sizeof(deviceQuirks.deviceName));
			strtcpy(deviceQuirks.deviceCodename, "Dragon", sizeof(deviceQuirks.deviceCodename));
			strtcpy(deviceQuirks.devicePlatform, "Mark 4", sizeof(deviceQuirks.devicePlatform));
			break;
		case DEVICE_KOBO_AURA_H2O:    // Aura H2O (dahlia)
			// NOTE: The top 11 pixels are blacked out by Nickel (behind the bezel)
			deviceQuirks.koboVertOffset             = 11;
			// NOTE: Boot rotation is FB_ROTATE_UR, pickel is FB_ROTATE_UD, nickel is FB_ROTATE_CW
			deviceQuirks.ntxBootRota                = FB_ROTATE_UR;
			// Canonical -> native rotation mapping: { UR: 1, CW: 0, UD: 3, CCW: 2 }
			deviceQuirks.ntxRotaQuirk               = NTX_ROTA_ALL_INVERTED;
			// {1, 0, 3, 2}
			deviceQuirks.rotationMap[FB_ROTATE_UR]  = FB_ROTATE_CW;
			deviceQuirks.rotationMap[FB_ROTATE_CW]  = FB_ROTATE_UR;
			deviceQuirks.rotationMap[FB_ROTATE_UD]  = FB_ROTATE_CCW;
			deviceQuirks.rotationMap[FB_ROTATE_CCW] = FB_ROTATE_UD;
			deviceQuirks.screenDPI                  = 265U;
			strtcpy(deviceQuirks.deviceName, "Aura H2O", sizeof(deviceQuirks.deviceName));
			strtcpy(deviceQuirks.deviceCodename, "Dahlia", sizeof(deviceQuirks.deviceCodename));
			strtcpy(deviceQuirks.devicePlatform, "Mark 5", sizeof(deviceQuirks.devicePlatform));
			break;
		case DEVICE_KOBO_AURA_H2O_2:    // Aura H2O² (snow)
			// NOTE: *Might* be UD like the other ROTA_STRAIGHT devices
			deviceQuirks.ntxBootRota                = FB_ROTATE_UR;
			// NOTE: Is indeed NTX_ROTA_STRAIGHT
			// {3, 0, 1, 2}
			deviceQuirks.rotationMap[FB_ROTATE_UR]  = FB_ROTATE_CCW;
			deviceQuirks.rotationMap[FB_ROTATE_CW]  = FB_ROTATE_UR;
			deviceQuirks.rotationMap[FB_ROTATE_UD]  = FB_ROTATE_CW;
			deviceQuirks.rotationMap[FB_ROTATE_CCW] = FB_ROTATE_UD;
			// Beware, this touch panel quirk *only* applies to r1 devices!
			deviceQuirks.touchMirrorX               = false;
			deviceQuirks.screenDPI                  = 265U;
			strtcpy(deviceQuirks.deviceName, "Aura H2O²", sizeof(deviceQuirks.deviceName));
			strtcpy(deviceQuirks.deviceCodename, "Snow", sizeof(deviceQuirks.deviceCodename));
			strtcpy(deviceQuirks.devicePlatform, "Mark 6", sizeof(deviceQuirks.devicePlatform));
			break;
		case DEVICE_KOBO_AURA_H2O_2_R2:    // Aura H2O² r2 (snow)
			deviceQuirks.isKoboMk7                  = true;
			deviceQuirks.ntxBootRota                = FB_ROTATE_UR;
			// NOTE: *Might* be NTX_ROTA_ODD_INVERTED
			// {3, 0, 1, 2}
			deviceQuirks.rotationMap[FB_ROTATE_UR]  = FB_ROTATE_CCW;
			deviceQuirks.rotationMap[FB_ROTATE_CW]  = FB_ROTATE_UR;
			deviceQuirks.rotationMap[FB_ROTATE_UD]  = FB_ROTATE_CW;
			deviceQuirks.rotationMap[FB_ROTATE_CCW] = FB_ROTATE_UD;
			deviceQuirks.screenDPI                  = 265U;
			strtcpy(deviceQuirks.deviceName, "Aura H2O² r2", sizeof(deviceQuirks.deviceName));
			strtcpy(deviceQuirks.deviceCodename, "Snow", sizeof(deviceQuirks.deviceCodename));
			strtcpy(deviceQuirks.devicePlatform, "Mark 7", sizeof(deviceQuirks.devicePlatform));
			break;
		case DEVICE_KOBO_AURA_ONE:    // Aura ONE (daylight)
			deviceQuirks.screenDPI = 300U;
			strtcpy(deviceQuirks.deviceName, "Aura One", sizeof(deviceQuirks.deviceName));
			strtcpy(deviceQuirks.deviceCodename, "Daylight", sizeof(deviceQuirks.deviceCodename));
			strtcpy(deviceQuirks.devicePlatform, "Mark 6", sizeof(deviceQuirks.devicePlatform));
			break;
		case DEVICE_KOBO_AURA_ONE_LE:    // Aura ONE LE (daylight)
			deviceQuirks.screenDPI = 300U;
			strtcpy(deviceQuirks.deviceName, "Aura One LE", sizeof(deviceQuirks.deviceName));
			strtcpy(deviceQuirks.deviceCodename, "Daylight", sizeof(deviceQuirks.deviceCodename));
			strtcpy(deviceQuirks.devicePlatform, "Mark 6", sizeof(deviceQuirks.devicePlatform));
			break;
		case DEVICE_KOBO_AURA_SE:    // Aura SE (star)
			deviceQuirks.screenDPI = 212U;
			strtcpy(deviceQuirks.deviceName, "Aura SE", sizeof(deviceQuirks.deviceName));
			strtcpy(deviceQuirks.deviceCodename, "Star", sizeof(deviceQuirks.deviceCodename));
			strtcpy(deviceQuirks.devicePlatform, "Mark 6", sizeof(deviceQuirks.devicePlatform));
			break;
		case DEVICE_TOLINO_VISION:    // Tolino Vision (vision) (~ Aura SE)
			deviceQuirks.screenDPI = 212U;
			strtcpy(deviceQuirks.deviceName, "Tolino Vision", sizeof(deviceQuirks.deviceName));
			strtcpy(deviceQuirks.deviceCodename, "Vision", sizeof(deviceQuirks.deviceCodename));
			strtcpy(deviceQuirks.devicePlatform, "Mark 6", sizeof(deviceQuirks.devicePlatform));
			break;
		case DEVICE_KOBO_AURA_SE_R2:    // Aura SE r2 (star)
			deviceQuirks.isKoboMk7 = true;
			// NOTE: *Might* be NTX_ROTA_ODD_INVERTED
			deviceQuirks.screenDPI = 212U;
			strtcpy(deviceQuirks.deviceName, "Aura SE r2", sizeof(deviceQuirks.deviceName));
			strtcpy(deviceQuirks.deviceCodename, "Star", sizeof(deviceQuirks.deviceCodename));
			strtcpy(deviceQuirks.devicePlatform, "Mark 7", sizeof(deviceQuirks.devicePlatform));
			break;
		case DEVICE_KOBO_CLARA_HD:    // Clara HD (nova)
			deviceQuirks.isKoboMk7 = true;
			// NOTE: Is indeed NTX_ROTA_STRAIGHT
			deviceQuirks.screenDPI = 300U;
			strtcpy(deviceQuirks.deviceName, "Clara HD", sizeof(deviceQuirks.deviceName));
			strtcpy(deviceQuirks.deviceCodename, "Nova", sizeof(deviceQuirks.deviceCodename));
			strtcpy(deviceQuirks.devicePlatform, "Mark 7", sizeof(deviceQuirks.devicePlatform));
			break;
		case DEVICE_TOLINO_SHINE_3:    // Shine 3 (loki) (~ Clara HD)
			deviceQuirks.isKoboMk7 = true;
			deviceQuirks.screenDPI = 300U;
			strtcpy(deviceQuirks.deviceName, "Shine 3", sizeof(deviceQuirks.deviceName));
			strtcpy(deviceQuirks.deviceCodename, "Loki", sizeof(deviceQuirks.deviceCodename));
			strtcpy(deviceQuirks.devicePlatform, "Mark 7", sizeof(deviceQuirks.devicePlatform));
			break;
		case DEVICE_KOBO_FORMA:    // Forma (frost)
			deviceQuirks.isKoboMk7                  = true;
			deviceQuirks.canRotate                  = true;
			// NOTE: Because Mk.7 and KOBO_HWCFG_DisplayBusWidth (35) is "16Bits_mirror" (3)
			// Canonical -> native rotation mapping: { UR: 3, CW: 2, UD: 1, CCW: 0 }
			deviceQuirks.ntxRotaQuirk               = NTX_ROTA_ODD_INVERTED;
			// {3, 2, 1, 0}
			deviceQuirks.rotationMap[FB_ROTATE_UR]  = FB_ROTATE_CCW;
			deviceQuirks.rotationMap[FB_ROTATE_CW]  = FB_ROTATE_UD;
			deviceQuirks.rotationMap[FB_ROTATE_UD]  = FB_ROTATE_CW;
			deviceQuirks.rotationMap[FB_ROTATE_CCW] = FB_ROTATE_UR;
			deviceQuirks.screenDPI                  = 300U;
			strtcpy(deviceQuirks.deviceName, "Forma", sizeof(deviceQuirks.deviceName));
			strtcpy(deviceQuirks.deviceCodename, "Frost", sizeof(deviceQuirks.deviceCodename));
			strtcpy(deviceQuirks.devicePlatform, "Mark 7", sizeof(deviceQuirks.devicePlatform));
			break;
		case DEVICE_KOBO_FORMA_32GB:    // Forma 32GB (frost)
			deviceQuirks.isKoboMk7                  = true;
			deviceQuirks.canRotate                  = true;
			// NOTE: Because Mk.7 and KOBO_HWCFG_DisplayBusWidth (35) is "16Bits_mirror" (3)
			deviceQuirks.ntxRotaQuirk               = NTX_ROTA_ODD_INVERTED;
			// {3, 2, 1, 0}
			deviceQuirks.rotationMap[FB_ROTATE_UR]  = FB_ROTATE_CCW;
			deviceQuirks.rotationMap[FB_ROTATE_CW]  = FB_ROTATE_UD;
			deviceQuirks.rotationMap[FB_ROTATE_UD]  = FB_ROTATE_CW;
			deviceQuirks.rotationMap[FB_ROTATE_CCW] = FB_ROTATE_UR;
			deviceQuirks.screenDPI                  = 300U;
			strtcpy(deviceQuirks.deviceName, "Forma 32GB", sizeof(deviceQuirks.deviceName));
			strtcpy(deviceQuirks.deviceCodename, "Frost", sizeof(deviceQuirks.deviceCodename));
			strtcpy(deviceQuirks.devicePlatform, "Mark 7", sizeof(deviceQuirks.devicePlatform));
			break;
		case DEVICE_TOLINO_EPOS_2:    // Epos 2 (freya) (~ Forma @ 16GB)
			deviceQuirks.isKoboMk7                  = true;
			deviceQuirks.canRotate                  = true;
			deviceQuirks.ntxRotaQuirk               = NTX_ROTA_ODD_INVERTED;
			// {3, 2, 1, 0}
			deviceQuirks.rotationMap[FB_ROTATE_UR]  = FB_ROTATE_CCW;
			deviceQuirks.rotationMap[FB_ROTATE_CW]  = FB_ROTATE_UD;
			deviceQuirks.rotationMap[FB_ROTATE_UD]  = FB_ROTATE_CW;
			deviceQuirks.rotationMap[FB_ROTATE_CCW] = FB_ROTATE_UR;
			deviceQuirks.screenDPI                  = 300U;
			strtcpy(deviceQuirks.deviceName, "Epos 2", sizeof(deviceQuirks.deviceName));
			strtcpy(deviceQuirks.deviceCodename, "Freya", sizeof(deviceQuirks.deviceCodename));
			strtcpy(deviceQuirks.devicePlatform, "Mark 7", sizeof(deviceQuirks.devicePlatform));
			break;
		case DEVICE_KOBO_LIBRA_H2O:    // Libra H2O (storm)
			deviceQuirks.isKoboMk7                  = true;
			// NOTE: Boot rotation is FB_ROTATE_UR, pickel is FB_ROTATE_UR, nickel is FB_ROTATE_UR
			//       And panel is *actually* in Portrait. Finally!
			//       Despite this, the kernel explicitly mangles the touch translation to match the "usual" layout.
			deviceQuirks.ntxBootRota                = FB_ROTATE_UR;
			deviceQuirks.canRotate                  = true;
			// NOTE: This one deserves a specific entry, because the H2O² also happens to be UR + STRAIGHT,
			//       but it is decidedly *NOT* sane ;).
			deviceQuirks.ntxRotaQuirk               = NTX_ROTA_SANE;
			// {0, 1, 2, 3}
			deviceQuirks.rotationMap[FB_ROTATE_UR]  = FB_ROTATE_UR;
			deviceQuirks.rotationMap[FB_ROTATE_CW]  = FB_ROTATE_CW;
			deviceQuirks.rotationMap[FB_ROTATE_UD]  = FB_ROTATE_UD;
			deviceQuirks.rotationMap[FB_ROTATE_CCW] = FB_ROTATE_CCW;
			// NOTE: The Libra was the first device to exhibit weirdly broken MXCFB_WAIT_FOR_UPDATE_COMPLETE behavior,
			//       where the ioctl would apparently randomly timeout after the full 5s for no reason...
			//       This can be reproduced in Nickel.
			//       In KOReader, we just sleep for a few ms instead as a workaround, c.f.,
			//       https://github.com/koreader/koreader-base/blob/21f4b974c7ab64a149075adc32318f87bf71dcdc/ffi/framebuffer_mxcfb.lua#L230-L235
			deviceQuirks.unreliableWaitFor          = true;
			deviceQuirks.screenDPI                  = 300U;
			strtcpy(deviceQuirks.deviceName, "Libra H2O", sizeof(deviceQuirks.deviceName));
			strtcpy(deviceQuirks.deviceCodename, "Storm", sizeof(deviceQuirks.deviceCodename));
			strtcpy(deviceQuirks.devicePlatform, "Mark 7", sizeof(deviceQuirks.devicePlatform));
			break;
		case DEVICE_TOLINO_VISION_5:    // Tolino Vision 5 (arya) (~ Libra H2O)
			deviceQuirks.isKoboMk7                  = true;
			deviceQuirks.ntxBootRota                = FB_ROTATE_UR;
			deviceQuirks.canRotate                  = true;
			deviceQuirks.ntxRotaQuirk               = NTX_ROTA_SANE;
			// {0, 1, 2, 3}
			deviceQuirks.rotationMap[FB_ROTATE_UR]  = FB_ROTATE_UR;
			deviceQuirks.rotationMap[FB_ROTATE_CW]  = FB_ROTATE_CW;
			deviceQuirks.rotationMap[FB_ROTATE_UD]  = FB_ROTATE_UD;
			deviceQuirks.rotationMap[FB_ROTATE_CCW] = FB_ROTATE_CCW;
			deviceQuirks.unreliableWaitFor          = true;
			deviceQuirks.screenDPI                  = 300U;
			strtcpy(deviceQuirks.deviceName, "Vision 5", sizeof(deviceQuirks.deviceName));
			strtcpy(deviceQuirks.deviceCodename, "Arya", sizeof(deviceQuirks.deviceCodename));
			strtcpy(deviceQuirks.devicePlatform, "Mark 7", sizeof(deviceQuirks.devicePlatform));
			break;
		case DEVICE_KOBO_NIA:    // Nia (luna)
			deviceQuirks.isKoboMk7         = true;
			// The board is similar to the Libra 2, so assume it suffers from the same quirks.
			deviceQuirks.unreliableWaitFor = true;
			deviceQuirks.screenDPI         = 212U;
			strtcpy(deviceQuirks.deviceName, "Nia", sizeof(deviceQuirks.deviceName));
			strtcpy(deviceQuirks.deviceCodename, "Luna", sizeof(deviceQuirks.deviceCodename));
			strtcpy(deviceQuirks.devicePlatform, "Mark 7", sizeof(deviceQuirks.devicePlatform));
			break;
		case DEVICE_KOBO_ELIPSA:    // Elipsa (Europa)
			deviceQuirks.isSunxi                    = true;
			deviceQuirks.hasEclipseWfm              = true;
			// Sunxi means no HW inversion :'(.
			// (And the nightmode_test flag toggled via the debugfs nightenable/nightdisable command doesn't count,
			// it just flips the buffer in C, and forces the *K waveform modes (dubbed "eclipse" mode)).
			deviceQuirks.canHWInvert                = false;
			// Has an accelerometer, but Nickel doesn't update the rotate flag, as it's meaningless.
			// That said, "native" rotation still matches the usual layout, as does the touch panel translation.
			deviceQuirks.ntxBootRota                = FB_ROTATE_CCW;    // e.g., fat bezel side UP.
			deviceQuirks.canRotate                  = true;
			deviceQuirks.ntxRotaQuirk               = NTX_ROTA_SUNXI;
			// There's no real linuxfb rotation support, so, {0, 1, 2, 3}
			// In practice, kobo_sunxi_fb_fixup ensures that the rotate flag returned by fbink_get_state is canonical,
			// despite what the actual, unused/unsupported by the driver value is...
			deviceQuirks.rotationMap[FB_ROTATE_UR]  = FB_ROTATE_UR;
			deviceQuirks.rotationMap[FB_ROTATE_CW]  = FB_ROTATE_CW;
			deviceQuirks.rotationMap[FB_ROTATE_UD]  = FB_ROTATE_UD;
			deviceQuirks.rotationMap[FB_ROTATE_CCW] = FB_ROTATE_CCW;
			deviceQuirks.screenDPI                  = 227U;
			strtcpy(deviceQuirks.deviceName, "Elipsa", sizeof(deviceQuirks.deviceName));
			strtcpy(deviceQuirks.deviceCodename, "Europa", sizeof(deviceQuirks.deviceCodename));
			strtcpy(deviceQuirks.devicePlatform, "Mark 8", sizeof(deviceQuirks.devicePlatform));
			break;
		case DEVICE_KOBO_LIBRA_2:    // Libra 2 (Io)
			deviceQuirks.hasEclipseWfm = true;
			deviceQuirks.isKoboMk7     = true;    // Same MXCFB API ;).
			// Both pickel & nickel then jump to FB_ROTATE_CW...
			deviceQuirks.ntxBootRota   = FB_ROTATE_UR;
			// ...KOBO_HWCFG_DisplayBusWidth (35) is "16Bits" (1),
			// meaning it is indeed NTX_ROTA_STRAIGHT (-ish) ;).
			// NOTE: Touch panel seems to have forgone part of the usual translation, though,
			//       it's UR-ish (x & y swapped as usual, but no mirroring at all).
			deviceQuirks.canRotate     = true;
			// Canonical -> native rotation mapping: { UR: 1, CW: 0, UD: 3, CCW: 2 }
			deviceQuirks.ntxRotaQuirk  = NTX_ROTA_CW_TOUCH;    // c.f., above, don't trust the name
			// {1, 0, 3, 2}
			deviceQuirks.rotationMap[FB_ROTATE_UR]  = FB_ROTATE_CW;
			deviceQuirks.rotationMap[FB_ROTATE_CW]  = FB_ROTATE_UR;
			deviceQuirks.rotationMap[FB_ROTATE_UD]  = FB_ROTATE_CCW;
			deviceQuirks.rotationMap[FB_ROTATE_CCW] = FB_ROTATE_UD;
			// Same touch panel quirk as the H2O²r1
			deviceQuirks.touchMirrorX               = false;
			// It apparently inherited its ancestor's issues...
			deviceQuirks.unreliableWaitFor          = true;
			deviceQuirks.screenDPI                  = 300U;
			strtcpy(deviceQuirks.deviceName, "Libra 2", sizeof(deviceQuirks.deviceName));
			strtcpy(deviceQuirks.deviceCodename, "Io", sizeof(deviceQuirks.deviceCodename));
			strtcpy(deviceQuirks.devicePlatform, "Mark 9", sizeof(deviceQuirks.devicePlatform));
			break;
		case DEVICE_KOBO_SAGE:    // Sage (Cadmus)
			deviceQuirks.isSunxi                    = true;
			deviceQuirks.hasEclipseWfm              = true;
			// NOTE: The EINK_NEGATIVE_MODE flag just does a software inversion
			//       (pixel by pixel, in plain C, in the eink_image_process kthread).
			deviceQuirks.canHWInvert                = false;
			deviceQuirks.ntxBootRota                = FB_ROTATE_CW;    // e.g., fat bezel side DOWN.
			deviceQuirks.canRotate                  = true;
			deviceQuirks.ntxRotaQuirk               = NTX_ROTA_SUNXI;
			// There's no real linuxfb rotation support, so, {0, 1, 2, 3}
			deviceQuirks.rotationMap[FB_ROTATE_UR]  = FB_ROTATE_UR;
			deviceQuirks.rotationMap[FB_ROTATE_CW]  = FB_ROTATE_CW;
			deviceQuirks.rotationMap[FB_ROTATE_UD]  = FB_ROTATE_UD;
			deviceQuirks.rotationMap[FB_ROTATE_CCW] = FB_ROTATE_CCW;
			deviceQuirks.screenDPI                  = 300U;
			strtcpy(deviceQuirks.deviceName, "Sage", sizeof(deviceQuirks.deviceName));
			strtcpy(deviceQuirks.deviceCodename, "Cadmus", sizeof(deviceQuirks.deviceCodename));
			strtcpy(deviceQuirks.devicePlatform, "Mark 8", sizeof(deviceQuirks.devicePlatform));
			break;
		case DEVICE_TOLINO_EPOS_3:    // Epos 3 (rigel) (~ Sage)
			deviceQuirks.isSunxi                    = true;
			deviceQuirks.hasEclipseWfm              = true;
			deviceQuirks.canHWInvert                = false;
			deviceQuirks.ntxBootRota                = FB_ROTATE_CW;    // e.g., fat bezel side DOWN.
			deviceQuirks.canRotate                  = true;
			deviceQuirks.ntxRotaQuirk               = NTX_ROTA_SUNXI;
			// There's no real linuxfb rotation support, so, {0, 1, 2, 3}
			deviceQuirks.rotationMap[FB_ROTATE_UR]  = FB_ROTATE_UR;
			deviceQuirks.rotationMap[FB_ROTATE_CW]  = FB_ROTATE_CW;
			deviceQuirks.rotationMap[FB_ROTATE_UD]  = FB_ROTATE_UD;
			deviceQuirks.rotationMap[FB_ROTATE_CCW] = FB_ROTATE_CCW;
			deviceQuirks.screenDPI                  = 300U;
			strtcpy(deviceQuirks.deviceName, "Epos 3", sizeof(deviceQuirks.deviceName));
			strtcpy(deviceQuirks.deviceCodename, "Rigel", sizeof(deviceQuirks.deviceCodename));
			strtcpy(deviceQuirks.devicePlatform, "Mark 8", sizeof(deviceQuirks.devicePlatform));
			break;
		case DEVICE_KOBO_CLARA_2E:    // Clara 2E (Goldfinch)
			deviceQuirks.hasEclipseWfm              = true;
			deviceQuirks.isKoboMk7                  = true;    // Same MXCFB API ;).
			// NOTE: Touch panel's native orientation is the usual CCW, i.e., origin on the top-right corner.
			// NOTE: NTXHWConfig says there ought to be a KX122 gyro, but apparently not?
			// NOTE: ioctls are straightforward (DisplayBusWidth is 8Bits),
			//       but it appears that landscape orientations are inverted nonetheless?
			//       Either NTX_ROTA_ODD_INVERTED or NTX_ROTA_CW_TOUCH would behave,
			//       but their description doesn't actually match what's happening...
			// Canonical -> native rotation mapping: { UR: 3, CW: 2, UD: 1, CCW: 0 }
			deviceQuirks.ntxRotaQuirk               = NTX_ROTA_CCW_TOUCH;
			// {3, 2, 1, 0}
			deviceQuirks.rotationMap[FB_ROTATE_UR]  = FB_ROTATE_CCW;
			deviceQuirks.rotationMap[FB_ROTATE_CW]  = FB_ROTATE_UD;
			deviceQuirks.rotationMap[FB_ROTATE_UD]  = FB_ROTATE_CW;
			deviceQuirks.rotationMap[FB_ROTATE_CCW] = FB_ROTATE_UR;
			// The board is nearly identical to the Libra 2, and as such, exhibits the same quirks...
			deviceQuirks.unreliableWaitFor          = true;
			deviceQuirks.screenDPI                  = 300U;
			strtcpy(deviceQuirks.deviceName, "Clara 2E", sizeof(deviceQuirks.deviceName));
			strtcpy(deviceQuirks.deviceCodename, "Goldfinch", sizeof(deviceQuirks.deviceCodename));
			strtcpy(deviceQuirks.devicePlatform, "Mark 10", sizeof(deviceQuirks.devicePlatform));
			break;
		case DEVICE_KOBO_ELIPSA_2E:    // Elipsa 2E (Condor)
			deviceQuirks.isMTK                      = true;
			deviceQuirks.hasEclipseWfm              = true;
			// NOTE: It technically can, but not via update flags, it's a global state.
			//       You can toggle it via fbink_set_fb_info if need be.
			deviceQuirks.canHWInvert                = false;
			// Unlike the Libra 2, KOBO_HWCFG_DisplayBusWidth (35) is "16Bits_mirror" (3),
			// but the touch-screen appears to behave similary, and its native orientation is also CW.
			// (Unlike on Mk.7+ mxcfb, hwtcon_fb_check_var doesn't do any kind of rotate trickery,
			// so KOBO_HWCFG_DisplayBusWidth doesn't affect our rota quirks).
			// NOTE: I'm assuming it behaves like later MTK devices,
			//       and is actually *always* at CW (pickel & nickel certainly are),
			//       but since, unlike its later brethren, this device *can* switch bitdepth,
			//       that might actually differ...
			//       In any case, for our purposes, that works out well enough.
			//       (c.f., the Libra Colour comment around its onw ntxBootRota assignment)
			deviceQuirks.ntxBootRota                = FB_ROTATE_UR;
			deviceQuirks.canRotate                  = true;
			// Canonical -> native rotation mapping: { UR: 1, CW: 0, UD: 3, CCW: 2 }
			deviceQuirks.ntxRotaQuirk               = NTX_ROTA_CW_TOUCH;
			// {1, 0, 3, 2}
			deviceQuirks.rotationMap[FB_ROTATE_UR]  = FB_ROTATE_CW;
			deviceQuirks.rotationMap[FB_ROTATE_CW]  = FB_ROTATE_UR;
			deviceQuirks.rotationMap[FB_ROTATE_UD]  = FB_ROTATE_CCW;
			deviceQuirks.rotationMap[FB_ROTATE_CCW] = FB_ROTATE_UD;
			// Mirrors the *other* axis compared to (most) other NTX boards...
			deviceQuirks.touchMirrorX               = false;
			deviceQuirks.touchMirrorY               = true;
			deviceQuirks.screenDPI                  = 227U;
			strtcpy(deviceQuirks.deviceName, "Elipsa 2E", sizeof(deviceQuirks.deviceName));
			strtcpy(deviceQuirks.deviceCodename, "Condor", sizeof(deviceQuirks.deviceCodename));
			strtcpy(deviceQuirks.devicePlatform, "Mark 11", sizeof(deviceQuirks.devicePlatform));
			break;
		case DEVICE_KOBO_LIBRA_COLOUR:    // Libra Colour (Monza)
			deviceQuirks.hasColorPanel              = true;
			deviceQuirks.isMTK                      = true;
			deviceQuirks.hasEclipseWfm              = true;
			// NOTE: It technically can, but not via update flags, it's a global state.
			//       You can toggle it via fbink_set_fb_info if need be.
			deviceQuirks.canHWInvert                = false;
			// NOTE: Appears to actually already match its final form (CW @ 32bpp) at boot,
			//       but this keeps the deprecated fbdepth -r -1 logic accurate...
			deviceQuirks.ntxBootRota                = FB_ROTATE_UR;
			deviceQuirks.canRotate                  = true;
			// Canonical -> native rotation mapping: { UR: 1, CW: 0, UD: 3, CCW: 2 }
			deviceQuirks.ntxRotaQuirk               = NTX_ROTA_CW_TOUCH;
			// {1, 0, 3, 2}
			deviceQuirks.rotationMap[FB_ROTATE_UR]  = FB_ROTATE_CW;
			deviceQuirks.rotationMap[FB_ROTATE_CW]  = FB_ROTATE_UR;
			deviceQuirks.rotationMap[FB_ROTATE_UD]  = FB_ROTATE_CCW;
			deviceQuirks.rotationMap[FB_ROTATE_CCW] = FB_ROTATE_UD;
			// Mirrors the *other* axis compared to (most) other NTX boards...
			deviceQuirks.touchMirrorX               = false;
			deviceQuirks.touchMirrorY               = true;
			deviceQuirks.screenDPI                  = 300U;
			strtcpy(deviceQuirks.deviceName, "Libra Colour", sizeof(deviceQuirks.deviceName));
			strtcpy(deviceQuirks.deviceCodename, "Monza", sizeof(deviceQuirks.deviceCodename));
			// NOTE: Initial FW were flagged Mk. 11 before someone realized that was weird, and they fixed it in 4.39.23027
			strtcpy(deviceQuirks.devicePlatform, "Mark 13", sizeof(deviceQuirks.devicePlatform));
			break;
		case DEVICE_TOLINO_VISION_COLOR:    // Tolino Vision Color (Monza Tolino)
			deviceQuirks.hasColorPanel              = true;
			deviceQuirks.isMTK                      = true;
			deviceQuirks.hasEclipseWfm              = true;
			// NOTE: It technically can, but not via update flags, it's a global state.
			//       You can toggle it via fbink_set_fb_info if need be.
			deviceQuirks.canHWInvert                = false;
			// NOTE: Appears to actually already match its final form (CW @ 32bpp) at boot,
			//       but this keeps the deprecated fbdepth -r -1 logic accurate...
			deviceQuirks.ntxBootRota                = FB_ROTATE_UR;
			deviceQuirks.canRotate                  = true;
			// Canonical -> native rotation mapping: { UR: 1, CW: 0, UD: 3, CCW: 2 }
			deviceQuirks.ntxRotaQuirk               = NTX_ROTA_CW_TOUCH;
			// {1, 0, 3, 2}
			deviceQuirks.rotationMap[FB_ROTATE_UR]  = FB_ROTATE_CW;
			deviceQuirks.rotationMap[FB_ROTATE_CW]  = FB_ROTATE_UR;
			deviceQuirks.rotationMap[FB_ROTATE_UD]  = FB_ROTATE_CCW;
			deviceQuirks.rotationMap[FB_ROTATE_CCW] = FB_ROTATE_UD;
			// Mirrors the *other* axis compared to (most) other NTX boards...
			deviceQuirks.touchMirrorX               = false;
			deviceQuirks.touchMirrorY               = true;
			deviceQuirks.screenDPI                  = 300U;
			strtcpy(deviceQuirks.deviceName, "Vision Color", sizeof(deviceQuirks.deviceName));
			strtcpy(deviceQuirks.deviceCodename, "Monza Tolino", sizeof(deviceQuirks.deviceCodename));
			strtcpy(deviceQuirks.devicePlatform, "Mark 13", sizeof(deviceQuirks.devicePlatform));
			break;
		case DEVICE_KOBO_CLARA_BW:    // Clara B&W (Spa BW)
			deviceQuirks.isMTK                      = true;
			deviceQuirks.hasEclipseWfm              = true;
			// NOTE: It technically can, but not via update flags, it's a global state.
			//       You can toggle it via fbink_set_fb_info if need be.
			deviceQuirks.canHWInvert                = false;
			// NOTE: Assume goldfinch-like for now...
			// Canonical -> native rotation mapping: { UR: 3, CW: 2, UD: 1, CCW: 0 }
			deviceQuirks.ntxRotaQuirk               = NTX_ROTA_CCW_TOUCH;
			// {3, 2, 1, 0}
			deviceQuirks.rotationMap[FB_ROTATE_UR]  = FB_ROTATE_CCW;
			deviceQuirks.rotationMap[FB_ROTATE_CW]  = FB_ROTATE_UD;
			deviceQuirks.rotationMap[FB_ROTATE_UD]  = FB_ROTATE_CW;
			deviceQuirks.rotationMap[FB_ROTATE_CCW] = FB_ROTATE_UR;
			deviceQuirks.screenDPI                  = 300U;
			strtcpy(deviceQuirks.deviceName, "Clara B&W", sizeof(deviceQuirks.deviceName));
			strtcpy(deviceQuirks.deviceCodename, "Spa BW", sizeof(deviceQuirks.deviceCodename));
			strtcpy(deviceQuirks.devicePlatform, "Mark 12", sizeof(deviceQuirks.devicePlatform));
			break;
		case DEVICE_TOLINO_SHINE_BW:    // Tolino Shine B&W (Spa Tolino BW)
			deviceQuirks.isMTK                      = true;
			deviceQuirks.hasEclipseWfm              = true;
			// NOTE: It technically can, but not via update flags, it's a global state.
			//       You can toggle it via fbink_set_fb_info if need be.
			deviceQuirks.canHWInvert                = false;
			// NOTE: Assume goldfinch-like for now...
			// Canonical -> native rotation mapping: { UR: 3, CW: 2, UD: 1, CCW: 0 }
			deviceQuirks.ntxRotaQuirk               = NTX_ROTA_CCW_TOUCH;
			// {3, 2, 1, 0}
			deviceQuirks.rotationMap[FB_ROTATE_UR]  = FB_ROTATE_CCW;
			deviceQuirks.rotationMap[FB_ROTATE_CW]  = FB_ROTATE_UD;
			deviceQuirks.rotationMap[FB_ROTATE_UD]  = FB_ROTATE_CW;
			deviceQuirks.rotationMap[FB_ROTATE_CCW] = FB_ROTATE_UR;
			deviceQuirks.screenDPI                  = 300U;
			// NOTE: Marketed as Tolino Shine, sometimes with a (5. Gen) appended,
			//       which approaches Kindle levels of impractical device names...
			strtcpy(deviceQuirks.deviceName, "Shine B&W", sizeof(deviceQuirks.deviceName));
			strtcpy(deviceQuirks.deviceCodename, "Spa Tolino BW", sizeof(deviceQuirks.deviceCodename));
			strtcpy(deviceQuirks.devicePlatform, "Mark 12", sizeof(deviceQuirks.devicePlatform));
			break;
		case DEVICE_KOBO_CLARA_COLOUR:    // Clara Colour (Spa Colour)
			deviceQuirks.hasColorPanel              = true;
			deviceQuirks.isMTK                      = true;
			deviceQuirks.hasEclipseWfm              = true;
			// NOTE: It technically can, but not via update flags, it's a global state.
			//       You can toggle it via fbink_set_fb_info if need be.
			deviceQuirks.canHWInvert                = false;
			// NOTE: Assume goldfinch-like for now...
			// Canonical -> native rotation mapping: { UR: 3, CW: 2, UD: 1, CCW: 0 }
			deviceQuirks.ntxRotaQuirk               = NTX_ROTA_CCW_TOUCH;
			// {3, 2, 1, 0}
			deviceQuirks.rotationMap[FB_ROTATE_UR]  = FB_ROTATE_CCW;
			deviceQuirks.rotationMap[FB_ROTATE_CW]  = FB_ROTATE_UD;
			deviceQuirks.rotationMap[FB_ROTATE_UD]  = FB_ROTATE_CW;
			deviceQuirks.rotationMap[FB_ROTATE_CCW] = FB_ROTATE_UR;
			deviceQuirks.screenDPI                  = 300U;
			strtcpy(deviceQuirks.deviceName, "Clara Colour", sizeof(deviceQuirks.deviceName));
			strtcpy(deviceQuirks.deviceCodename, "Spa Colour", sizeof(deviceQuirks.deviceCodename));
			strtcpy(deviceQuirks.devicePlatform, "Mark 12", sizeof(deviceQuirks.devicePlatform));
			break;
		case DEVICE_TOLINO_SHINE_COLOR:    // Tolino Shine Color (Spa Tolino Colour)
			deviceQuirks.hasColorPanel              = true;
			deviceQuirks.isMTK                      = true;
			deviceQuirks.hasEclipseWfm              = true;
			// NOTE: It technically can, but not via update flags, it's a global state.
			//       You can toggle it via fbink_set_fb_info if need be.
			deviceQuirks.canHWInvert                = false;
			// NOTE: Assume goldfinch-like for now...
			// Canonical -> native rotation mapping: { UR: 3, CW: 2, UD: 1, CCW: 0 }
			deviceQuirks.ntxRotaQuirk               = NTX_ROTA_CCW_TOUCH;
			// {3, 2, 1, 0}
			deviceQuirks.rotationMap[FB_ROTATE_UR]  = FB_ROTATE_CCW;
			deviceQuirks.rotationMap[FB_ROTATE_CW]  = FB_ROTATE_UD;
			deviceQuirks.rotationMap[FB_ROTATE_UD]  = FB_ROTATE_CW;
			deviceQuirks.rotationMap[FB_ROTATE_CCW] = FB_ROTATE_UR;
			deviceQuirks.screenDPI                  = 300U;
			strtcpy(deviceQuirks.deviceName, "Shine Color", sizeof(deviceQuirks.deviceName));
			strtcpy(deviceQuirks.deviceCodename, "Spa Tolino Colour", sizeof(deviceQuirks.deviceCodename));
			strtcpy(deviceQuirks.devicePlatform, "Mark 12", sizeof(deviceQuirks.devicePlatform));
			break;
		case DEVICE_UNKNOWN:
			// Like kobo_config.sh, assume Trilogy as a fallback
			deviceQuirks.isKoboNonMT = true;
			strtcpy(deviceQuirks.deviceName, "Touch A?", sizeof(deviceQuirks.deviceName));
			strtcpy(deviceQuirks.deviceCodename, "Trilogy?", sizeof(deviceQuirks.deviceCodename));
			strtcpy(deviceQuirks.devicePlatform, "Mark 3?", sizeof(deviceQuirks.devicePlatform));
			break;
		case DEVICE_MAINLINE_TOLINO_SHINE_2HD:    // Tolino Shine 2HD (Glo HD-ish)
			// Mainline kernels expect to use the same set of ioctls as on Mk. 7+, even on older devices.
			deviceQuirks.isKoboMk7 = true;
			deviceQuirks.screenDPI = 300U;
			strtcpy(deviceQuirks.deviceName, "Shine 2HD", sizeof(deviceQuirks.deviceName));
			strtcpy(deviceQuirks.deviceCodename, "Mainline", sizeof(deviceQuirks.deviceCodename));
			strtcpy(deviceQuirks.devicePlatform, "Tolino", sizeof(deviceQuirks.devicePlatform));
			break;
		case DEVICE_MAINLINE_TOLINO_SHINE_3:    // Tolino Shine 3 (Clara HD-ish)
			deviceQuirks.isKoboMk7 = true;
			deviceQuirks.screenDPI = 300U;
			strtcpy(deviceQuirks.deviceName, "Shine 3", sizeof(deviceQuirks.deviceName));
			strtcpy(deviceQuirks.deviceCodename, "Mainline", sizeof(deviceQuirks.deviceCodename));
			strtcpy(deviceQuirks.devicePlatform, "Tolino", sizeof(deviceQuirks.devicePlatform));
			break;
		case DEVICE_MAINLINE_TOLINO_VISION:    // Tolino Vision (Aura SE-ish, maybe?)
			deviceQuirks.screenDPI = 212U;
			strtcpy(deviceQuirks.deviceName, "Vision", sizeof(deviceQuirks.deviceName));
			strtcpy(deviceQuirks.deviceCodename, "Mainline", sizeof(deviceQuirks.deviceCodename));
			strtcpy(deviceQuirks.devicePlatform, "Tolino", sizeof(deviceQuirks.devicePlatform));
			break;
		case DEVICE_MAINLINE_TOLINO_VISION_5:    // Tolino Vision 5 (Libra H2O-ish)
			deviceQuirks.isKoboMk7    = true;
			deviceQuirks.ntxBootRota  = FB_ROTATE_UR;
			deviceQuirks.canRotate    = true;
			deviceQuirks.ntxRotaQuirk = NTX_ROTA_SANE;
			deviceQuirks.screenDPI    = 300U;
			strtcpy(deviceQuirks.deviceName, "Vision 5", sizeof(deviceQuirks.deviceName));
			strtcpy(deviceQuirks.deviceCodename, "Mainline", sizeof(deviceQuirks.deviceCodename));
			strtcpy(deviceQuirks.devicePlatform, "Tolino", sizeof(deviceQuirks.devicePlatform));
			break;
		case DEVICE_MAINLINE_GENERIC_IMX5:
			// Generic fallback for i.MX5 devices on mainline kernels
			deviceQuirks.screenDPI = 212U;
			strtcpy(deviceQuirks.deviceName, "i.MX5", sizeof(deviceQuirks.deviceName));
			strtcpy(deviceQuirks.deviceCodename, "Mainline", sizeof(deviceQuirks.deviceCodename));
			strtcpy(deviceQuirks.devicePlatform, "Mark <= 5", sizeof(deviceQuirks.devicePlatform));
			break;
		case DEVICE_MAINLINE_GENERIC_IMX6:
			// Generic fallback for i.MX6 devices on mainline kernels
			deviceQuirks.isKoboMk7 = true;
			deviceQuirks.screenDPI = 212U;
			strtcpy(deviceQuirks.deviceName, "i.MX6", sizeof(deviceQuirks.deviceName));
			strtcpy(deviceQuirks.deviceCodename, "Mainline", sizeof(deviceQuirks.deviceCodename));
			strtcpy(deviceQuirks.devicePlatform, "Mark >= 6", sizeof(deviceQuirks.devicePlatform));
			break;
		case DEVICE_MAINLINE_GENERIC_SUNXI_B300:
			// Generic fallback for sunxi B300 devices
			deviceQuirks.isSunxi       = true;
			deviceQuirks.hasEclipseWfm = true;
			deviceQuirks.canHWInvert   = false;
			deviceQuirks.ntxBootRota   = FB_ROTATE_UR;    // Assume nothing
			deviceQuirks.canRotate     = true;
			deviceQuirks.ntxRotaQuirk  = NTX_ROTA_SUNXI;
			deviceQuirks.screenDPI     = 227U;
			strtcpy(deviceQuirks.deviceName, "B300", sizeof(deviceQuirks.deviceName));
			strtcpy(deviceQuirks.deviceCodename, "Mainline", sizeof(deviceQuirks.deviceCodename));
			strtcpy(deviceQuirks.devicePlatform, "Mark 8", sizeof(deviceQuirks.devicePlatform));
			break;
		default:
			// Print something slightly different if we completely failed to even compute a kobo_id...
			if (kobo_id == DEVICE_INVALID) {
				WARN("Failed to compute a Kobo device code");
			} else {
				WARN("Unidentified Kobo device code (%hu)", kobo_id);
			}
			strtcpy(deviceQuirks.deviceName, "Unknown!", sizeof(deviceQuirks.deviceName));
			strtcpy(deviceQuirks.deviceCodename, "??", sizeof(deviceQuirks.deviceCodename));
			strtcpy(deviceQuirks.devicePlatform, "Mark ?", sizeof(deviceQuirks.devicePlatform));
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
		ELOG("Couldn't find a Kobo version tag (onboard unmounted or not using Nickel?)!");
	} else {
		// NOTE: I'm not entirely sure this will always have a fixed length, so, give ourselves a bit of room...
		char   line[_POSIX_PATH_MAX] = { 0 };
		size_t size                  = fread(line, sizeof(*line), sizeof(line) - 1U, fp);
		fclose(fp);
		if (size > 0) {
			// The line/file should not contain a trailing LF, but, just in case...
			if (line[size - 1U] == '\n') {
				line[size - 1U] = '\0';
			}

			// Thankfully, the device code is always located in the three
			// final characters, so that's easy enough to extract without
			// having to worry about the formatting...
			const unsigned short int kobo_id = (unsigned short int) strtoul(line + (size - 3), NULL, 10);
			set_kobo_quirks(kobo_id);

			// Get out now, we're done!
			// NOTE: The nickel tag has been found and *successfully* read,
			//       no need to fall back to DTB identification, it's definitely not mainline.
			return;
		} else {
			WARN("Failed to read the Kobo version tag (%zu)", size);
			// NOTE: Make it clear we failed to identify the device...
			//       i.e., by passing DEVICE_INVALID instead of DEVICE_UNKNOWN, which we use to flag old !NTX devices.
			// NOTE: This codepath can genuinely be reached if you're unlucky enough that a crash
			//       leads to zapping the file to 0 bytes (e.g., after an fsck),
			//       which is why we don't abort now, and instead go on to try to read the HWConfig block.
			set_kobo_quirks(DEVICE_INVALID);
		}
	}

	// NOTE: Okay, if we got this far, we failed to open /mnt/onboard/.kobo/version,
	//       which, provided we're really running on a Kobo, can legitimately happen,
	//       if we were launched in the middle of an USBMS session, in which case onboard is obviously not available ;).
	//       So try to do it the hard way, via the NTXHWConfig tag...
#		pragma GCC diagnostic push
#		pragma GCC diagnostic ignored "-Wmissing-braces"
	NTXHWConfig config              = { 0 };
#		pragma GCC diagnostic pop
	unsigned char* restrict payload = NULL;
	uint64_t storagesize            = 0U;

	// We have a couple of locations to check...
	for (size_t i = 0U; i < ARRAY_SIZE(HWCONFIG_BLOCKS); i++) {
		const HWConfigBlockDev hwconfig = HWCONFIG_BLOCKS[i];

		// We want a semi-silent "does it even exist?" check before the actual open w/ read rights check
		if (access(hwconfig.device, F_OK) != 0) {
			PFLOG("Block device `%s` is unavailable, can't look for an NTX HWConfig entry there",
			      hwconfig.device);
			continue;
		}

		fp = fopen(hwconfig.device, "re");
		if (!fp) {
			PFWARN("Couldn't read from `%s` (%m), unable to identify the Kobo model via its NTX board info",
			       hwconfig.device);
			continue;
		}

		if (fseek(fp, hwconfig.offset, SEEK_SET) != 0) {
			PFWARN("Failed to seek to position 0x%p in `%s`: %m", (void*) hwconfig.offset, hwconfig.device);
		} else {
			if (fread(&config, sizeof(config), 1, fp) < 1 || ferror(fp) != 0) {
				WARN("Failed to read the NTX HWConfig entry on `%s`", hwconfig.device);
				fclose(fp);

				// Let us try the next location, and mainline device identification after that...
				continue;
			}

			// NOTE: This slice will NOT be NULL-terminated,
			//       so we have to chop the terminating NULL from the string literal's size.
			if (memcmp(config.magic, HWCONFIG_MAGIC, sizeof(HWCONFIG_MAGIC) - 1U) != 0) {
				WARN("Block device `%s` does not appear to contain an NTX HWConfig entry",
				     hwconfig.device);
				fclose(fp);
				continue;
			}

			// We'll read the full payload, whose size varies depending on the exact kernel being used...
			// Since it's only a couple dozen bytes, do that on the stack to make our life easier.
			payload = alloca(config.len);
			if (fread(payload, sizeof(*payload), config.len, fp) < config.len || ferror(fp) != 0) {
				WARN("Error reading NTX HWConfig payload (unexpected length)");
				fclose(fp);

				// NOTE: This is "fatal", stop there, and make it clear we failed to identify the device...
				set_kobo_quirks(DEVICE_INVALID);
				return;
			}

			// We'll also need the total storage space to discriminate 32GB devices...
			if (strcmp(hwconfig.device, HWCONFIG_DEVICE) == 0) {
				// NOTE: This is only meaningful for HWCONFIG_DEVICE
				//       (and only matters for devices where it's the location of the payload anyway).
				if (ioctl(fileno(fp), BLKGETSIZE64, &storagesize)) {
					// Make that non-fatal, as the distinction is purely cosmetic for our purposes
					PFWARN("Error requesting block device size: %m");
				}
			}

			// We've got a valid-looking payload (*and* we've allocated stack space for it), we're done!
			fclose(fp);
			break;
		}
		fclose(fp);
	}

	// If we successfully got a HWConfig payload in the previous loop, examine it now
	if (payload) {
		/*
		// NOTE: Dump the full payload, for science!
		for (uint8_t i = 0; i < config.len; i++) {
			ELOG("NTXHWConfig[%hhu] -> %hhu", i, payload[i]);
		}
		*/

		if (payload[KOBO_HWCFG_PCB] >= ARRAY_SIZE(kobo_ids)) {
			WARN("Unknown Kobo PCB ID index (%hhu >= %zu)", payload[KOBO_HWCFG_PCB], ARRAY_SIZE(kobo_ids));
		} else {
			// As per /bin/kobo_config.sh, match PCB IDs to Product IDs via a LUT...
			// NOTE: Some Tolinos *will* end up with a Kobo ID here, because of lax matches:
			//       e.g., the Shine 3 will be matched as a Clara HD, and the Vision 5 as a Libra.
			unsigned short int kobo_id = kobo_ids[payload[KOBO_HWCFG_PCB]];

			// And now for the fun part, the few device variants that use the same PCB ID...
			if (kobo_id == DEVICE_KOBO_AURA_H2O_2 || kobo_id == DEVICE_KOBO_AURA_SE) {
				// Discriminate the Mk.7 version for dual rev models by checking the CPU...
				if (payload[KOBO_HWCFG_CPU] == HWCFG_CPU_MX6SLL) {
					// Thankfully, that works for both the H2O² (374 -> 378),
					// and the Aura SE (375 -> 379) ;)
					kobo_id += 4U;
				}
			} else if (kobo_id == DEVICE_KOBO_GLO_HD) {
				// Discriminate Alyssum from Pika, by checking the Display Resolution...
				if (payload[KOBO_HWCFG_DisplayResolution] == HWCFG_DISPLAY_RESOLUTION_800_600) {
					// Glo HD (Alyssum) [371] -> Touch 2.0 (Pika) [372]
					kobo_id = DEVICE_KOBO_TOUCH_2;
				}
			} else if (kobo_id == DEVICE_KOBO_AURA_ONE || kobo_id == DEVICE_KOBO_FORMA) {
				// Discriminate 32GB variants...
				// NOTE: We compare against 8GB, but in practice, given storage shenanigans and
				//       the truncation involved here, we end up with 7 on 8GB devices ;).
				if ((storagesize >> 10U >> 10U >> 10U) > 8U) {
					if (kobo_id == DEVICE_KOBO_AURA_ONE) {
						// Aura ONE (daylight) [373] -> Aura ONE LE (daylight) [381]
						kobo_id = DEVICE_KOBO_AURA_ONE_LE;
					} else if (kobo_id == DEVICE_KOBO_FORMA) {
						// Forma (frost) [377] -> Forma 32GB (frost) [380]
						kobo_id = DEVICE_KOBO_FORMA_32GB;

						if (payload[KOBO_HWCFG_Customer] == HWCFG_CUSTOMER_TOLINO) {
							// Forma (frost) [377] -> Tolino Epos 2 16GB (freya) [677]
							kobo_id += TOLINO_DEVICE_ID_OFFSET;
						}
					}
				}
			} else if (kobo_id == DEVICE_KOBO_CLARA_HD || kobo_id == DEVICE_KOBO_LIBRA_H2O ||
				   kobo_id == DEVICE_KOBO_SAGE) {
				if (payload[KOBO_HWCFG_Customer] == HWCFG_CUSTOMER_TOLINO) {
					// Clara HD (nova) [376] -> Tolino Shine 3 (loki) [676]
					// Libra H2O (storm) [384] -> Tolino Vision 5 (arya) [684]
					// Sage (cadmus) [383] -> Tolino Epos 3 (rigel) [683]
					kobo_id += TOLINO_DEVICE_ID_OFFSET;
				}
			} else if (kobo_id == DEVICE_KOBO_LIBRA_COLOUR) {
				// Discriminate the Kobo Libra Colour from the Tolino Vision Color
				if (payload[KOBO_HWCFG_Customer] == HWCFG_CUSTOMER_TOLINO) {
					// Libra Colour (monza) [390] -> Tolino Vision Color (monza Tolino) [690]
					kobo_id += TOLINO_DEVICE_ID_OFFSET;
				}
			} else if (kobo_id == DEVICE_KOBO_CLARA_BW) {
				// Discriminate the Kobo Clara from the Tolino Shine
				if (payload[KOBO_HWCFG_Customer] == HWCFG_CUSTOMER_TOLINO) {
					// Clara B&W (spa BW) [391] -> Tolino Shine BW (spa Tolino BW) [691]
					kobo_id += TOLINO_DEVICE_ID_OFFSET;
				}
				// Discriminate between the B&W & Color variants
				if (payload[KOBO_HWCFG_EPD_Flags] & HWCFG_EPD_FLAGS_CFA) {
					// Clara B&W (spa BW) [391] -> Clara Colour (spa Colour) [393]
					// Tolino Shine BW (spa Tolino BW) [691] -> Tolino Shine Color (spa Tolino Colour) [693]
					kobo_id += 2U;
				}
			}

			// Assuming we actually *know* about this PCB ID...
			if (kobo_id != DEVICE_UNKNOWN) {
				// ...we can do this, as accurately as if onboard were mounted ;).
				set_kobo_quirks(kobo_id);

				// Get out now, we're done!
				return;
			}
		}
	}

	// NOTE: And if we went this far, it means onboard isn't mounted/Nickel never ran,
	//       *and* either we're not on an NTX board, or the NTXHWConfig block device is unavailable;
	//       or we failed to detect a device with either of those two methods.
	//       That usually points to a device running a mainline kernel, so let's poke at the DTB...
	return identify_mainline();
}

static void
    identify_mainline(void)
{
	// This is aimed at mainline kernels, c.f., https://github.com/NiLuJe/FBInk/issues/70
	// NOTE: Like rcS, if all else fails, assume it's an old Freescale Trilogy...
	unsigned short int kobo_id = DEVICE_UNKNOWN;
	FILE*              fp      = fopen(MAINLINE_DEVICE_ID_SYSFS, "re");
	if (!fp) {
		ELOG("Couldn't find a DTB sysfs entry!");
	} else {
		char*   line = NULL;
		size_t  len  = 0;
		ssize_t nread;
		while ((nread = getdelim(&line, &len, '\0', fp)) != -1) {
			// Keep this in the same order as Documentation/devicetree/bindings/arm/fsl.yaml to ease updates
			if (strcmp(line, "kobo,aura") == 0) {
				kobo_id = DEVICE_KOBO_AURA;
				break;
			} else if (strcmp(line, "kobo,aura2") == 0) {
				kobo_id = DEVICE_KOBO_AURA_SE;
				break;
			} else if (strcmp(line, "kobo,tolino-shine2hd") == 0) {
				kobo_id = DEVICE_MAINLINE_TOLINO_SHINE_2HD;
				break;
			} else if (strcmp(line, "kobo,tolino-shine3") == 0) {
				kobo_id = DEVICE_MAINLINE_TOLINO_SHINE_3;
				break;
			} else if (strcmp(line, "kobo,tolino-vision") == 0) {
				kobo_id = DEVICE_MAINLINE_TOLINO_VISION;
				break;
			} else if (strcmp(line, "kobo,tolino-vision5") == 0) {
				kobo_id = DEVICE_MAINLINE_TOLINO_VISION_5;
				break;
			} else if (strcmp(line, "kobo,clarahd") == 0) {
				kobo_id = DEVICE_KOBO_CLARA_HD;
				break;
			} else if (strcmp(line, "kobo,librah2o") == 0) {
				kobo_id = DEVICE_KOBO_LIBRA_H2O;
				break;
			} else if (strcmp(line, "kobo,nia") == 0) {
				kobo_id = DEVICE_KOBO_NIA;
				break;
			} else if (strcmp(line, "fsl,imx6sl") == 0) {
				kobo_id = DEVICE_MAINLINE_GENERIC_IMX6;
				break;
			} else if (strcmp(line, "fsl,imx6sll") == 0) {
				kobo_id = DEVICE_MAINLINE_GENERIC_IMX6;
				break;
			} else if (strcmp(line, "fsl,imx6ull") == 0) {
				kobo_id = DEVICE_MAINLINE_GENERIC_IMX6;
				break;
			} else if (strcmp(line, "fsl,imx50") == 0) {
				kobo_id = DEVICE_MAINLINE_GENERIC_IMX5;
				break;
			} else if (strcmp(line, "allwinner,sun8iw15p1") == 0) {
				kobo_id = DEVICE_MAINLINE_GENERIC_SUNXI_B300;
				break;
			}
		}
		free(line);
		fclose(fp);
	}

	set_kobo_quirks(kobo_id);

	// NOTE: We enforce NTX_ROTA_STRAIGHT on all the mxcfb devices here, as Andrea's kernels all behave sanely on that front...
	//       c.f., https://github.com/akemnade/linux/blob/c240ea6209ebea7d577db3f54abaa1658fd5f15c/drivers/video/fbdev/mxc/mxc_epdc_v2_fb.c#L2654
	if (deviceQuirks.isSunxi == false && deviceQuirks.ntxRotaQuirk != NTX_ROTA_SANE) {
		// NOTE: The Libra is flagged NTX_ROTA_SANE, and there *are* a few deprecated codepaths that will make use of this,
		//       so we leave it alone, given that it doesn't really affect the ioctl behavior.
		deviceQuirks.ntxRotaQuirk = NTX_ROTA_STRAIGHT;
	}
}

#	elif defined(FBINK_FOR_REMARKABLE)
static void
    identify_remarkable(void)
{
	// NOTE: Follow the lead of https://github.com/reMarkable/update_engine (utils::GetMachineModel)
	FILE* fp               = fopen("/sys/devices/soc0/machine", "re");
	// Random buffer size is random!
	char  machine_tag[32U] = { 0 };
	if (!fp) {
		ELOG("Couldn't open the sysfs entry for the SoC machine tag!");
	} else {
		size_t size = fread(machine_tag, sizeof(*machine_tag), sizeof(machine_tag) - 1U, fp);
		fclose(fp);
		if (size > 0) {
			// Strip trailing LF
			if (machine_tag[size - 1U] == '\n') {
				machine_tag[size - 1U] = '\0';
			}
		}
	}

	if (strcmp(machine_tag, "reMarkable 2.0") == 0) {
		// NOTE: This is currently a very weird and mostly unsupported beast...
		//       c.f., https://rmkit.dev/news/rm2-status/
		deviceQuirks.screenDPI    = 226U;
		deviceQuirks.canHWInvert  = false;    // No EPDC :(
		// NOTE: Not actually an NTX board, but useful nonetheless for fbdepth ;).
		deviceQuirks.ntxBootRota  = FB_ROTATE_CW;
		deviceQuirks.ntxRotaQuirk = NTX_ROTA_SANE;
		deviceQuirks.canRotate    = true;
		strtcpy(deviceQuirks.deviceName, "reMarkable 2", sizeof(deviceQuirks.deviceName));
		strtcpy(deviceQuirks.deviceCodename, "Zero Sugar", sizeof(deviceQuirks.deviceCodename));
		deviceQuirks.deviceId = DEVICE_REMARKABLE_2;
	} else {
		deviceQuirks.screenDPI    = 226U;
		deviceQuirks.canHWInvert  = true;
		// NOTE: Not actually an NTX board, but useful nonetheless for fbdepth ;).
		deviceQuirks.ntxBootRota  = FB_ROTATE_CW;
		deviceQuirks.ntxRotaQuirk = NTX_ROTA_SANE;
		deviceQuirks.canRotate    = true;
		strtcpy(deviceQuirks.deviceName, "reMarkable", sizeof(deviceQuirks.deviceName));
		strtcpy(deviceQuirks.deviceCodename, "Zero Gravitas", sizeof(deviceQuirks.deviceCodename));
		deviceQuirks.deviceId = DEVICE_REMARKABLE_1;
	}
}
#	elif defined(FBINK_FOR_POCKETBOOK)
static void
    identify_pocketbook(void)
{
	char* model_name = NULL;
	// NOTE: I'm not super happy about calling InkView here,
	//       so do it in a slightly roundabout way to try to prevent it from wreaking too much havoc...
	if (!getenv("FBINK_NO_INKVIEW")) {
		void* inkview = dlopen("libinkview.so", RTLD_LAZY | RTLD_LOCAL);
		if (!inkview) {
			ELOG("Failed to load InkView: %s", dlerror());
		} else {
			dlerror();

			// Try to grab the adress for InkView's GetDeviceModel function...
			char* (*inkview_GetDeviceModel)(void) = NULL;
			inkview_GetDeviceModel                = dlsym(inkview, "GetDeviceModel");

			char* err = dlerror();
			if (err != NULL) {
				ELOG("Failed to obtain GetDeviceModel symbol: %s", err);
			} else {
				// NOTE: We may be leaking the string here, since I have no idea what InkView does...
				char* model = (*inkview_GetDeviceModel)();
				//       Which is why we make a copy, just in case...
				model_name  = strdupa(model);
			}

			// Bye InkView! Hopefully your crappy dependencies haven't wreaked too much havoc...
			dlclose(inkview);
		}
	}

	// Unless otherwise specified, let's allow HW inversion everywhere for now...
	deviceQuirks.canHWInvert = true;

	// NOTE: PB apparently has a bad habit of reporting the device name in a somewhat erratic fashion, so,
	//       attempt to cover all known variants...
	//       c.f., https://github.com/koreader/koreader/pull/11721
#		define PB_CODENAME_MATCH(id)                                                                            \
			strcmp(model_name, "PB" id) == 0 || strcmp(model_name, "PocketBook " id) == 0 ||                 \
			    strstr(model_name, " (PB" id ")") != NULL

	if (model_name) {
		// Keep the model name dance in the same order as KOReader...
		if (PB_CODENAME_MATCH("515")) {
			deviceQuirks.screenDPI = 200U;
			strtcpy(deviceQuirks.deviceCodename, "PBMini", sizeof(deviceQuirks.deviceCodename));
			deviceQuirks.deviceId = DEVICE_POCKETBOOK_MINI;
		} else if (PB_CODENAME_MATCH("606")) {
			deviceQuirks.screenDPI = 212U;
			strtcpy(deviceQuirks.deviceCodename, "PB606", sizeof(deviceQuirks.deviceCodename));
			deviceQuirks.deviceId = DEVICE_POCKETBOOK_606;
		} else if (PB_CODENAME_MATCH("611")) {
			deviceQuirks.screenDPI = 167U;
			strtcpy(deviceQuirks.deviceCodename, "PB611", sizeof(deviceQuirks.deviceCodename));
			deviceQuirks.deviceId = DEVICE_POCKETBOOK_611;
		} else if (PB_CODENAME_MATCH("613")) {
			deviceQuirks.screenDPI = 167U;
			strtcpy(deviceQuirks.deviceCodename, "PB613B", sizeof(deviceQuirks.deviceCodename));
			deviceQuirks.deviceId = DEVICE_POCKETBOOK_613;
		} else if (PB_CODENAME_MATCH("614") || PB_CODENAME_MATCH("614W")) {
			deviceQuirks.screenDPI = 167U;
			strtcpy(deviceQuirks.deviceCodename, "PB614W", sizeof(deviceQuirks.deviceCodename));
			deviceQuirks.deviceId = DEVICE_POCKETBOOK_614;
		} else if (PB_CODENAME_MATCH("615") || PB_CODENAME_MATCH("615W")) {
			deviceQuirks.screenDPI = 212U;
			strtcpy(deviceQuirks.deviceCodename, "PBBLux", sizeof(deviceQuirks.deviceCodename));
			deviceQuirks.deviceId = DEVICE_POCKETBOOK_615;
		} else if (PB_CODENAME_MATCH("616") || PB_CODENAME_MATCH("616W")) {
			deviceQuirks.screenDPI = 212U;
			strtcpy(deviceQuirks.deviceCodename, "PBBLux2", sizeof(deviceQuirks.deviceCodename));
			deviceQuirks.deviceId = DEVICE_POCKETBOOK_616;
		} else if (PB_CODENAME_MATCH("617")) {
			deviceQuirks.screenDPI = 212U;
			strtcpy(deviceQuirks.deviceCodename, "PBBLux3", sizeof(deviceQuirks.deviceCodename));
			deviceQuirks.deviceId = DEVICE_POCKETBOOK_617;
		} else if (PB_CODENAME_MATCH("618")) {
			deviceQuirks.screenDPI = 212U;
			strtcpy(deviceQuirks.deviceCodename, "PBBLux4", sizeof(deviceQuirks.deviceCodename));
			deviceQuirks.deviceId = DEVICE_POCKETBOOK_618;
		} else if (PB_CODENAME_MATCH("622")) {
			deviceQuirks.screenDPI = 167U;
			strtcpy(deviceQuirks.deviceCodename, "PBTouch", sizeof(deviceQuirks.deviceCodename));
			deviceQuirks.deviceId = DEVICE_POCKETBOOK_TOUCH;
		} else if (PB_CODENAME_MATCH("623")) {
			deviceQuirks.screenDPI = 212U;
			strtcpy(deviceQuirks.deviceCodename, "PBLux", sizeof(deviceQuirks.deviceCodename));
			deviceQuirks.deviceId = DEVICE_POCKETBOOK_LUX;
		} else if (PB_CODENAME_MATCH("624")) {
			deviceQuirks.screenDPI = 167U;
			strtcpy(deviceQuirks.deviceCodename, "PBBTouch", sizeof(deviceQuirks.deviceCodename));
			deviceQuirks.deviceId = DEVICE_POCKETBOOK_BASIC_TOUCH;
		} else if (PB_CODENAME_MATCH("625")) {
			deviceQuirks.screenDPI = 167U;
			strtcpy(deviceQuirks.deviceCodename, "PBBTouch2", sizeof(deviceQuirks.deviceCodename));
			deviceQuirks.deviceId = DEVICE_POCKETBOOK_BASIC_TOUCH_2;
		} else if (PB_CODENAME_MATCH("626") || PB_CODENAME_MATCH("626(2)-TL3")) {
			deviceQuirks.screenDPI = 212U;
			strtcpy(deviceQuirks.deviceCodename, "PBLux3", sizeof(deviceQuirks.deviceCodename));
			deviceQuirks.deviceId = DEVICE_POCKETBOOK_LUX_3;
		} else if (PB_CODENAME_MATCH("627")) {
			deviceQuirks.screenDPI = 212U;
			strtcpy(deviceQuirks.deviceCodename, "PBLux4", sizeof(deviceQuirks.deviceCodename));
			deviceQuirks.deviceId = DEVICE_POCKETBOOK_LUX_4;
		} else if (PB_CODENAME_MATCH("628")) {
			deviceQuirks.screenDPI = 212U;
			strtcpy(deviceQuirks.deviceCodename, "PBLux5", sizeof(deviceQuirks.deviceCodename));
			deviceQuirks.deviceId = DEVICE_POCKETBOOK_LUX_5;
		} else if (PB_CODENAME_MATCH("629")) {
			deviceQuirks.screenDPI = 212U;
			strtcpy(deviceQuirks.deviceCodename, "PBVerse", sizeof(deviceQuirks.deviceCodename));
			deviceQuirks.deviceId = DEVICE_POCKETBOOK_VERSE;
		} else if (PB_CODENAME_MATCH("630")) {
			deviceQuirks.screenDPI = 212U;
			strtcpy(deviceQuirks.deviceCodename, "PBSense", sizeof(deviceQuirks.deviceCodename));
			deviceQuirks.deviceId = DEVICE_POCKETBOOK_SENSE;
		} else if (PB_CODENAME_MATCH("631")) {
			deviceQuirks.screenDPI = 300U;
			strtcpy(deviceQuirks.deviceCodename, "PBTouchHD", sizeof(deviceQuirks.deviceCodename));
			deviceQuirks.deviceId = DEVICE_POCKETBOOK_TOUCH_HD;
		} else if (PB_CODENAME_MATCH("632")) {
			deviceQuirks.screenDPI = 300U;
			strtcpy(deviceQuirks.deviceCodename, "PBTouchHD+", sizeof(deviceQuirks.deviceCodename));
			deviceQuirks.deviceId = DEVICE_POCKETBOOK_TOUCH_HD_PLUS;
		} else if (PB_CODENAME_MATCH("633")) {
			deviceQuirks.hasColorPanel = true;
			deviceQuirks.screenDPI     = 300U;
			strtcpy(deviceQuirks.deviceCodename, "PBColor", sizeof(deviceQuirks.deviceCodename));
			deviceQuirks.deviceId = DEVICE_POCKETBOOK_COLOR;
		} else if (PB_CODENAME_MATCH("634")) {
			deviceQuirks.screenDPI = 300U;
			strtcpy(deviceQuirks.deviceCodename, "PBVersePro", sizeof(deviceQuirks.deviceCodename));
			deviceQuirks.deviceId = DEVICE_POCKETBOOK_VERSE_PRO;
		} else if (PB_CODENAME_MATCH("634K3")) {
			deviceQuirks.hasColorPanel = true;
			deviceQuirks.screenDPI     = 300U;
			strtcpy(deviceQuirks.deviceCodename, "PBVerseProColor", sizeof(deviceQuirks.deviceCodename));
			deviceQuirks.deviceId = DEVICE_POCKETBOOK_VERSE_PRO_COLOR;
		} else if (PB_CODENAME_MATCH("640")) {
			deviceQuirks.screenDPI = 167U;
			strtcpy(deviceQuirks.deviceCodename, "PBAqua", sizeof(deviceQuirks.deviceCodename));
			deviceQuirks.deviceId = DEVICE_POCKETBOOK_AQUA;
		} else if (PB_CODENAME_MATCH("641")) {
			deviceQuirks.screenDPI = 212U;
			strtcpy(deviceQuirks.deviceCodename, "PBAqua2", sizeof(deviceQuirks.deviceCodename));
			deviceQuirks.deviceId = DEVICE_POCKETBOOK_AQUA2;
		} else if (PB_CODENAME_MATCH("650")) {
			deviceQuirks.screenDPI = 212U;
			strtcpy(deviceQuirks.deviceCodename, "PBUltra", sizeof(deviceQuirks.deviceCodename));
			deviceQuirks.deviceId = DEVICE_POCKETBOOK_ULTRA;
		} else if (PB_CODENAME_MATCH("700")) {
			deviceQuirks.screenDPI = 300U;
			strtcpy(deviceQuirks.deviceCodename, "PBEra", sizeof(deviceQuirks.deviceCodename));
			deviceQuirks.deviceId = DEVICE_POCKETBOOK_ERA;
		} else if (PB_CODENAME_MATCH("700K3")) {
			deviceQuirks.hasColorPanel = true;
			deviceQuirks.screenDPI     = 300U;
			strtcpy(deviceQuirks.deviceCodename, "PBEraColor", sizeof(deviceQuirks.deviceCodename));
			deviceQuirks.deviceId = DEVICE_POCKETBOOK_ERA_COLOR;
		} else if (PB_CODENAME_MATCH("740")) {
			deviceQuirks.screenDPI = 300U;
			strtcpy(deviceQuirks.deviceCodename, "PBInkPad3", sizeof(deviceQuirks.deviceCodename));
			deviceQuirks.deviceId = DEVICE_POCKETBOOK_INKPAD_3;
		} else if (PB_CODENAME_MATCH("740-2") || PB_CODENAME_MATCH("740-3")) {
			deviceQuirks.screenDPI = 300U;
			strtcpy(deviceQuirks.deviceCodename, "PBInkPad3Pro", sizeof(deviceQuirks.deviceCodename));
			deviceQuirks.deviceId = DEVICE_POCKETBOOK_INKPAD_3_PRO;
		} else if (PB_CODENAME_MATCH("741")) {
			deviceQuirks.hasColorPanel = true;
			deviceQuirks.screenDPI     = 300U;
			strtcpy(deviceQuirks.deviceCodename, "PBInkPadColor", sizeof(deviceQuirks.deviceCodename));
			deviceQuirks.deviceId = DEVICE_POCKETBOOK_INKPAD_COLOR;
		} else if (PB_CODENAME_MATCH("743C")) {
			deviceQuirks.hasColorPanel = true;
			deviceQuirks.screenDPI     = 300U;
			strtcpy(deviceQuirks.deviceCodename, "PBInkPadColor2", sizeof(deviceQuirks.deviceCodename));
			deviceQuirks.deviceId = DEVICE_POCKETBOOK_INKPAD_COLOR_2;
		} else if (PB_CODENAME_MATCH("743K")) {
			deviceQuirks.hasColorPanel = true;
			deviceQuirks.screenDPI     = 300U;
			strtcpy(deviceQuirks.deviceCodename, "PBInkPadColor3", sizeof(deviceQuirks.deviceCodename));
			deviceQuirks.deviceId = DEVICE_POCKETBOOK_INKPAD_COLOR_3;
		} else if (PB_CODENAME_MATCH("743G") || PB_CODENAME_MATCH("743g")) {
			deviceQuirks.screenDPI = 300U;
			strtcpy(deviceQuirks.deviceCodename, "PBInkPad4", sizeof(deviceQuirks.deviceCodename));
			deviceQuirks.deviceId = DEVICE_POCKETBOOK_INKPAD_4;
		} else if (PB_CODENAME_MATCH("840") || strcmp(model_name, "Reader InkPad") == 0) {
			deviceQuirks.screenDPI = 250U;
			strtcpy(deviceQuirks.deviceCodename, "PBInkPad", sizeof(deviceQuirks.deviceCodename));
			deviceQuirks.deviceId = DEVICE_POCKETBOOK_INKPAD;
		} else if (PB_CODENAME_MATCH("970")) {
			deviceQuirks.screenDPI = 150U;
			strtcpy(deviceQuirks.deviceCodename, "PBInkPadLite", sizeof(deviceQuirks.deviceCodename));
			deviceQuirks.deviceId = DEVICE_POCKETBOOK_INKPAD_LITE;
		} else if (PB_CODENAME_MATCH("1040")) {
			deviceQuirks.screenDPI = 227U;
			strtcpy(deviceQuirks.deviceCodename, "PBInkPadX", sizeof(deviceQuirks.deviceCodename));
			deviceQuirks.deviceId = DEVICE_POCKETBOOK_INKPAD_X;
		} else if (PB_CODENAME_MATCH("Color Lux")) {
			deviceQuirks.hasColorPanel = true;
			deviceQuirks.screenDPI     = 125U;
			strtcpy(deviceQuirks.deviceCodename, "PBColorLux", sizeof(deviceQuirks.deviceCodename));
			deviceQuirks.deviceId = DEVICE_POCKETBOOK_COLOR_LUX;
		} else {
			WARN("Unidentified PocketBook model: `%s`", model_name);
			deviceQuirks.screenDPI = 212U;
			strtcpy(deviceQuirks.deviceCodename, "Unidentified", sizeof(deviceQuirks.deviceCodename));
		}
	} else {
		// NOTE: Failed to query DeviceModel via InkView, so, fake something...
		WARN("Couldn't query PocketBook model name via InkView");
		deviceQuirks.screenDPI = 212U;
		strtcpy(deviceQuirks.deviceCodename, "Unknown", sizeof(deviceQuirks.deviceCodename));
	}
}
#	endif    // FBINK_FOR_KINDLE

static void
    identify_device(void)
{
#	if defined(FBINK_FOR_KINDLE)
	identify_kindle();
	if (deviceQuirks.deviceId > 0xFFu) {
		char* restrict dev_id = to_base(deviceQuirks.deviceId, 32U, 3U);
		ELOG("Detected a Kindle %s (%s -> 0x%03X => %s on %s)",
		     deviceQuirks.deviceName,
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
	ELOG("Detected a %s %s (%hu => %s @ %s)",
	     deviceQuirks.deviceId >= (TOLINO_DEVICE_ID_OFFSET * 2) ? "Tolino" : "Kobo",
	     deviceQuirks.deviceName,
	     deviceQuirks.deviceId,
	     deviceQuirks.deviceCodename,
	     deviceQuirks.devicePlatform);
#	elif defined(FBINK_FOR_REMARKABLE)
	identify_remarkable();
	ELOG("Detected a %s (%s)", deviceQuirks.deviceName, deviceQuirks.deviceCodename);
#	elif defined(FBINK_FOR_POCKETBOOK)
	identify_pocketbook();
	ELOG("Detected a PocketBook (%s)", deviceQuirks.deviceCodename);
#	endif
	// Warn if canHWInvert was flipped
	if (!deviceQuirks.canHWInvert) {
		// NOTE: Allow overriding that via an env var
		if (getenv("FBINK_ALLOW_HW_INVERT")) {
			ELOG("Enforced HW inversion support on a device where it may be buggy!");
			deviceQuirks.canHWInvert = true;
		} else {
			ELOG("This device does not support HW inversion");
		}
	}
}
#endif    // !FBINK_FOR_LINUX
