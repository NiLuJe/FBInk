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

#ifndef FBINK_FOR_LINUX
#	include "fbink_device_id.h"

#	if defined(FBINK_FOR_KINDLE)
// NOTE: This is adapted from KindleTool,
//       c.f., https://github.com/NiLuJe/KindleTool/blob/master/KindleTool/kindle_tool.h#L189
static bool
    is_kindle_device(uint32_t dev)
{
	switch (dev) {
		case 0x01:    // K1
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
		case 0x02:    // K2
		case 0x03:
			deviceQuirks.isKindleLegacy = true;
			// Flawfinder: ignore
			strncpy(deviceQuirks.deviceName, "2", sizeof(deviceQuirks.deviceName) - 1U);
			// Flawfinder: ignore
			strncpy(deviceQuirks.deviceCodename, "Turing", sizeof(deviceQuirks.deviceCodename) - 1U);
			// Flawfinder: ignore
			strncpy(deviceQuirks.devicePlatform, "Mario", sizeof(deviceQuirks.devicePlatform) - 1U);
			return true;
		case 0x04:    // KDX
		case 0x05:
		case 0x09:
			deviceQuirks.isKindleLegacy = true;
			// Flawfinder: ignore
			strncpy(deviceQuirks.deviceName, "DX", sizeof(deviceQuirks.deviceName) - 1U);
			// Flawfinder: ignore
			strncpy(deviceQuirks.deviceCodename, "Nell", sizeof(deviceQuirks.deviceCodename) - 1U);
			// NOTE: Possibly Banjo? Never figured out if that platform ever saw the light of day...
			// Flawfinder: ignore
			strncpy(deviceQuirks.devicePlatform, "Mario", sizeof(deviceQuirks.devicePlatform) - 1U);
			return true;
		case 0x08:    // K3
		case 0x06:
		case 0x0A:
			deviceQuirks.isKindleLegacy = true;
			// Flawfinder: ignore
			strncpy(deviceQuirks.deviceName, "3", sizeof(deviceQuirks.deviceName) - 1U);
			// Flawfinder: ignore
			strncpy(deviceQuirks.deviceCodename, "Shasta", sizeof(deviceQuirks.deviceCodename) - 1U);
			// Flawfinder: ignore
			strncpy(deviceQuirks.devicePlatform, "Luigi", sizeof(deviceQuirks.devicePlatform) - 1U);
			return true;
		case 0x0E:    // K4
			deviceQuirks.isKindleLegacy = true;
			// Flawfinder: ignore
			strncpy(deviceQuirks.deviceName, "4", sizeof(deviceQuirks.deviceName) - 1U);
			// Flawfinder: ignore
			strncpy(deviceQuirks.deviceCodename, "Tequila", sizeof(deviceQuirks.deviceCodename) - 1U);
			// Flawfinder: ignore
			strncpy(deviceQuirks.devicePlatform, "Yoshi", sizeof(deviceQuirks.devicePlatform) - 1U);
			return true;
		case 0x0F:    // K5
		case 0x11:
		case 0x10:
		case 0x12:
			deviceQuirks.isKindlePearlScreen = true;
			// Flawfinder: ignore
			strncpy(deviceQuirks.deviceName, "Touch", sizeof(deviceQuirks.deviceName) - 1U);
			// Flawfinder: ignore
			strncpy(deviceQuirks.deviceCodename, "Whitney", sizeof(deviceQuirks.deviceCodename) - 1U);
			// Flawfinder: ignore
			strncpy(deviceQuirks.devicePlatform, "Yoshi", sizeof(deviceQuirks.devicePlatform) - 1U);
			return true;
		case 0x23:    // K4b
			deviceQuirks.isKindleLegacy = true;
			// Flawfinder: ignore
			strncpy(deviceQuirks.deviceName, "4", sizeof(deviceQuirks.deviceName) - 1U);
			// Flawfinder: ignore
			strncpy(deviceQuirks.deviceCodename, "Sauza", sizeof(deviceQuirks.deviceCodename) - 1U);
			// Flawfinder: ignore
			strncpy(deviceQuirks.devicePlatform, "Yoshi", sizeof(deviceQuirks.devicePlatform) - 1U);
			return true;
		case 0x24:    // PW1
		case 0x1B:
		case 0x1C:
		case 0x1D:
		case 0x1F:
		case 0x20:
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
			deviceQuirks.screenDPI = 212U;
			// Flawfinder: ignore
			strncpy(deviceQuirks.deviceName, "PaperWhite 2", sizeof(deviceQuirks.deviceName) - 1U);
			// Flawfinder: ignore
			strncpy(deviceQuirks.deviceCodename, "Pinot", sizeof(deviceQuirks.deviceCodename) - 1U);
			// Flawfinder: ignore
			strncpy(deviceQuirks.devicePlatform, "Wario", sizeof(deviceQuirks.devicePlatform) - 1U);
			return true;
		case 0xC6:    // KT2
			// Flawfinder: ignore
			strncpy(deviceQuirks.deviceName, "Basic", sizeof(deviceQuirks.deviceName) - 1U);
			// Flawfinder: ignore
			strncpy(deviceQuirks.deviceCodename, "Bourbon", sizeof(deviceQuirks.deviceCodename) - 1U);
			// Flawfinder: ignore
			strncpy(deviceQuirks.devicePlatform, "Wario", sizeof(deviceQuirks.devicePlatform) - 1U);
			return true;
		case 0x13:    // KV
			deviceQuirks.screenDPI = 300U;
			// Flawfinder: ignore
			strncpy(deviceQuirks.deviceName, "Voyage", sizeof(deviceQuirks.deviceName) - 1U);
			// Flawfinder: ignore
			strncpy(deviceQuirks.deviceCodename, "Icewine", sizeof(deviceQuirks.deviceCodename) - 1U);
			// Flawfinder: ignore
			strncpy(deviceQuirks.devicePlatform, "Wario", sizeof(deviceQuirks.devicePlatform) - 1U);
			return true;
		case 0x16:    // ??
		case 0x21:
			return true;
		case 0x54:    // KV
		case 0x2A:
		case 0x4F:
		case 0x52:
		case 0x53:
			deviceQuirks.screenDPI = 300U;
			// Flawfinder: ignore
			strncpy(deviceQuirks.deviceName, "Voyage", sizeof(deviceQuirks.deviceName) - 1U);
			// Flawfinder: ignore
			strncpy(deviceQuirks.deviceCodename, "Icewine", sizeof(deviceQuirks.deviceCodename) - 1U);
			// Flawfinder: ignore
			strncpy(deviceQuirks.devicePlatform, "Wario", sizeof(deviceQuirks.devicePlatform) - 1U);
			return true;
		case 0x07:    // ??
		case 0x0B:
		case 0x0C:
		case 0x0D:
		case 0x99:
			return true;
		case 0xDD:    // KT2 AUS
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
			deviceQuirks.screenDPI = 300U;
			// Flawfinder: ignore
			strncpy(deviceQuirks.deviceName, "PaperWhite 3", sizeof(deviceQuirks.deviceName) - 1U);
			// Flawfinder: ignore
			strncpy(deviceQuirks.deviceCodename, "Muscat", sizeof(deviceQuirks.deviceCodename) - 1U);
			// Flawfinder: ignore
			strncpy(deviceQuirks.devicePlatform, "Wario", sizeof(deviceQuirks.devicePlatform) - 1U);
			return true;
		case 0x20C:    // KOA
		case 0x20D:
		case 0x219:
		case 0x21A:
		case 0x21B:
		case 0x21C:
			deviceQuirks.screenDPI = 300U;
			// Flawfinder: ignore
			strncpy(deviceQuirks.deviceName, "Oasis", sizeof(deviceQuirks.deviceName) - 1U);
			// Flawfinder: ignore
			strncpy(deviceQuirks.deviceCodename, "Whisky", sizeof(deviceQuirks.deviceCodename) - 1U);
			// Flawfinder: ignore
			strncpy(deviceQuirks.devicePlatform, "Duet", sizeof(deviceQuirks.devicePlatform) - 1U);
			return true;
		case 0x1BC:    // KT3
		case 0x269:
		case 0x26A:
			// Flawfinder: ignore
			strncpy(deviceQuirks.deviceName, "Basic 2", sizeof(deviceQuirks.deviceName) - 1U);
			// Flawfinder: ignore
			strncpy(deviceQuirks.deviceCodename, "Eanab", sizeof(deviceQuirks.deviceCodename) - 1U);
			// Flawfinder: ignore
			strncpy(deviceQuirks.devicePlatform, "Heisenberg", sizeof(deviceQuirks.devicePlatform) - 1U);
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
			deviceQuirks.isKindleOasis2 = true;
			deviceQuirks.screenDPI      = 300U;
			// Flawfinder: ignore
			strncpy(deviceQuirks.deviceName, "Oasis 2", sizeof(deviceQuirks.deviceName) - 1U);
			// Flawfinder: ignore
			strncpy(deviceQuirks.deviceCodename, "Cognac", sizeof(deviceQuirks.deviceCodename) - 1U);
			// Flawfinder: ignore
			strncpy(deviceQuirks.devicePlatform, "Zelda", sizeof(deviceQuirks.devicePlatform) - 1U);
			return true;
		case 0x2F7:    // PW4
		case 0x361:
		case 0x362:
		case 0x363:
		case 0x364:
		case 0x365:
		case 0x366:
		case 0x367:
		case 0x372:
		case 0x373:
		case 0x374:
		case 0x375:
		case 0x376:
		case 0x402:
		case 0x403:
			deviceQuirks.isKindleRex = true;
			deviceQuirks.screenDPI   = 300U;
			// Flawfinder: ignore
			strncpy(deviceQuirks.deviceName, "PaperWhite 4", sizeof(deviceQuirks.deviceName) - 1U);
			// Flawfinder: ignore
			strncpy(deviceQuirks.deviceCodename, "Moonshine", sizeof(deviceQuirks.deviceCodename) - 1U);
			// Flawfinder: ignore
			strncpy(deviceQuirks.devicePlatform, "Rex", sizeof(deviceQuirks.devicePlatform) - 1U);
			return true;
		case 0x414:
		case 0x3CF:
		case 0x3D0:
		case 0x3D1:
		case 0x3D2:
		case 0x3AB:
			deviceQuirks.isKindleRex = true;
			// Flawfinder: ignore
			strncpy(deviceQuirks.deviceName, "Basic 3", sizeof(deviceQuirks.deviceName) - 1U);
			// Flawfinder: ignore
			strncpy(deviceQuirks.deviceCodename, "Jaeger", sizeof(deviceQuirks.deviceCodename) - 1U);
			// Flawfinder: ignore
			strncpy(deviceQuirks.devicePlatform, "Rex", sizeof(deviceQuirks.devicePlatform) - 1U);
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
	n = ((neg = (num < 0))) ? (uint64_t)(~num) + 1 : (uint64_t) num;

	do {
		buf[len++] = tbl[n % base];
	} while (n /= base);

	out = calloc(len + neg + 1U, sizeof(*out));
	if (out == NULL) {
		WARN("Error allocating base32 output string buffer");
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
		WARN("Cannot open /proc/usid (not running on a Kindle?)");
	} else {
		unsigned char serial_no[KINDLE_SERIAL_NO_LENGTH] = { 0 };
		if (fread(serial_no, sizeof(unsigned char), KINDLE_SERIAL_NO_LENGTH, fp) < KINDLE_SERIAL_NO_LENGTH ||
		    ferror(fp) != 0) {
			WARN("Error reading /proc/usid (unexpected length)");
		}
		fclose(fp);

		// Get the device code...
		char device_code[3 + 1] = { 0 };
		// NOTE: Slice the bracketed section out of the S/N: B0[17]NNNNNNNNNNNN
		snprintf(device_code, 2 + 1, "%.*s", 2, serial_no + 2);
		// It's in hex, easy peasy.
		uint32_t dev = (uint32_t) strtoul(device_code, NULL, 16);
		// Check if it looks like the old device id scheme...
		if (!is_kindle_device(dev)) {
			// ... try the new device ID scheme if it doesn't... (G09[0G1]NNNNNNNNNN)
			snprintf(device_code, 3 + 1, "%.*s", 3, serial_no + 3);
			// (these ones are encoded in a slightly custom base 32)
			dev = from_base(device_code, 32);
			// ... And if it's not either list, it's truly unknown.
			if (!is_kindle_device_new(dev)) {
				WARN("Unidentified Kindle device %s (0x%03X)", device_code, dev);
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
		WARN("Couldn't read from '%s', unable to identify the Cervantes model", HWCONFIG_DEVICE);
	} else {
#		pragma GCC diagnostic push
#		pragma GCC diagnostic ignored "-Wmissing-braces"
		NTXHWConfig config = { 0 };
#		pragma GCC diagnostic pop

		if (fseek(fp, HWCONFIG_OFFSET, SEEK_SET) != 0) {
			WARN("Failed to seek to position 0x%p in '%s'", (void*) HWCONFIG_OFFSET, HWCONFIG_DEVICE);
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
			case 22:    // BQ Cervantes Touch - Fnac Touch (2012-2013)
				deviceQuirks.screenDPI = 167U;
				// Flawfinder: ignore
				strncpy(deviceQuirks.deviceName, "Touch", sizeof(deviceQuirks.deviceName) - 1U);
				break;
			case 23:    // BQ Cervantes TouchLight - Fnac Touch Plus (2012-2013)
				deviceQuirks.screenDPI = 167U;
				// Flawfinder: ignore
				strncpy(deviceQuirks.deviceName, "TouchLight", sizeof(deviceQuirks.deviceName) - 1U);
				break;
			case 33:    // BQ Cervantes 2013 - Fnac Touch Light (2013)
				deviceQuirks.screenDPI = 212U;
				// Flawfinder: ignore
				strncpy(deviceQuirks.deviceName, "2013", sizeof(deviceQuirks.deviceName) - 1U);
				break;
			case 51:    // BQ Cervantes 3 - Fnac Touch Light 2 (2016)
				deviceQuirks.screenDPI = 300U;
				// Flawfinder: ignore
				strncpy(deviceQuirks.deviceName, "3", sizeof(deviceQuirks.deviceName) - 1U);
				break;
			case 68:    // BQ Cervantes 4
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
#	else
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
	switch (kobo_id) {
		case 310:    // Touch A/B (trilogy)
			deviceQuirks.isKoboNonMT = true;
			// Flawfinder: ignore
			strncpy(deviceQuirks.deviceName, "Touch A/B", sizeof(deviceQuirks.deviceName) - 1U);
			// Flawfinder: ignore
			strncpy(deviceQuirks.deviceCodename, "Trilogy", sizeof(deviceQuirks.deviceCodename) - 1U);
			// Flawfinder: ignore
			strncpy(deviceQuirks.devicePlatform, "Mark 3", sizeof(deviceQuirks.devicePlatform) - 1U);
			break;
		case 320:    // Touch C (trilogy)
			deviceQuirks.isKoboNonMT = true;
			// Flawfinder: ignore
			strncpy(deviceQuirks.deviceName, "Touch C", sizeof(deviceQuirks.deviceName) - 1U);
			// Flawfinder: ignore
			strncpy(deviceQuirks.deviceCodename, "Trilogy", sizeof(deviceQuirks.deviceCodename) - 1U);
			// Flawfinder: ignore
			strncpy(deviceQuirks.devicePlatform, "Mark 4", sizeof(deviceQuirks.devicePlatform) - 1U);
			break;
		case 340:    // Mini (pixie)
			deviceQuirks.isKoboNonMT = true;
			deviceQuirks.screenDPI   = 200U;
			// Flawfinder: ignore
			strncpy(deviceQuirks.deviceName, "Mini", sizeof(deviceQuirks.deviceName) - 1U);
			// Flawfinder: ignore
			strncpy(deviceQuirks.deviceCodename, "Pixie", sizeof(deviceQuirks.deviceCodename) - 1U);
			// Flawfinder: ignore
			strncpy(deviceQuirks.devicePlatform, "Mark 4", sizeof(deviceQuirks.devicePlatform) - 1U);
			break;
		case 330:    // Glo (kraken)
			deviceQuirks.isKoboNonMT = true;
			deviceQuirks.screenDPI   = 212U;
			// Flawfinder: ignore
			strncpy(deviceQuirks.deviceName, "Glo", sizeof(deviceQuirks.deviceName) - 1U);
			// Flawfinder: ignore
			strncpy(deviceQuirks.deviceCodename, "Kraken", sizeof(deviceQuirks.deviceCodename) - 1U);
			// Flawfinder: ignore
			strncpy(deviceQuirks.devicePlatform, "Mark 4", sizeof(deviceQuirks.devicePlatform) - 1U);
			break;
		case 371:    // Glo HD (alyssum)
			deviceQuirks.screenDPI = 300U;
			// Flawfinder: ignore
			strncpy(deviceQuirks.deviceName, "Glo HD", sizeof(deviceQuirks.deviceName) - 1U);
			// Flawfinder: ignore
			strncpy(deviceQuirks.deviceCodename, "Alyssum", sizeof(deviceQuirks.deviceCodename) - 1U);
			// Flawfinder: ignore
			strncpy(deviceQuirks.devicePlatform, "Mark 6", sizeof(deviceQuirks.devicePlatform) - 1U);
			break;
		case 372:    // Touch 2.0 (pika)
			// Flawfinder: ignore
			strncpy(deviceQuirks.deviceName, "Touch 2.0", sizeof(deviceQuirks.deviceName) - 1U);
			// Flawfinder: ignore
			strncpy(deviceQuirks.deviceCodename, "Pika", sizeof(deviceQuirks.deviceCodename) - 1U);
			// Flawfinder: ignore
			strncpy(deviceQuirks.devicePlatform, "Mark 6", sizeof(deviceQuirks.devicePlatform) - 1U);
			break;
		case 360:    // Aura (phoenix)
			// NOTE: The bottom 10 pixels *may* be blacked out by Nickel? (TBC!)
			//deviceQuirks.koboVertOffset = -10;
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
		case 350:    // Aura HD (dragon)
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
		case 370:    // Aura H2O (dahlia)
			// NOTE: The top 11 pixels are blacked out by Nickel (behind the bezel)
			deviceQuirks.koboVertOffset = 11;
			// NOTE: Boot rotation is FB_ROTATE_UR, pickel is FB_ROTATE_UD, nickel is FB_ROTATE_CW
			deviceQuirks.ntxBootRota  = FB_ROTATE_UR;
			deviceQuirks.ntxRotaQuirk = NTX_ROTA_ALL_INVERTED;
			deviceQuirks.screenDPI    = 265U;
			// Flawfinder: ignore
			strncpy(deviceQuirks.deviceName, "H2O", sizeof(deviceQuirks.deviceName) - 1U);
			// Flawfinder: ignore
			strncpy(deviceQuirks.deviceCodename, "Dahlia", sizeof(deviceQuirks.deviceCodename) - 1U);
			// Flawfinder: ignore
			strncpy(deviceQuirks.devicePlatform, "Mark 5", sizeof(deviceQuirks.devicePlatform) - 1U);
			break;
		case 374:    // Aura H2O² (snow)
			deviceQuirks.ntxBootRota = FB_ROTATE_UR;
			// NOTE: Is indeed NTX_ROTA_STRAIGHT
			deviceQuirks.screenDPI = 265U;
			// Flawfinder: ignore
			strncpy(deviceQuirks.deviceName, "H2O²", sizeof(deviceQuirks.deviceName) - 1U);
			// Flawfinder: ignore
			strncpy(deviceQuirks.deviceCodename, "Snow", sizeof(deviceQuirks.deviceCodename) - 1U);
			// Flawfinder: ignore
			strncpy(deviceQuirks.devicePlatform, "Mark 6", sizeof(deviceQuirks.devicePlatform) - 1U);
			break;
		case 378:    // Aura H2O² r2 (snow)
			deviceQuirks.isKoboMk7   = true;
			deviceQuirks.ntxBootRota = FB_ROTATE_UR;
			// NOTE: *Might* be NTX_ROTA_ODD_INVERTED
			deviceQuirks.screenDPI = 265U;
			// Flawfinder: ignore
			strncpy(deviceQuirks.deviceName, "H2O² r2", sizeof(deviceQuirks.deviceName) - 1U);
			// Flawfinder: ignore
			strncpy(deviceQuirks.deviceCodename, "Snow", sizeof(deviceQuirks.deviceCodename) - 1U);
			// Flawfinder: ignore
			strncpy(deviceQuirks.devicePlatform, "Mark 7", sizeof(deviceQuirks.devicePlatform) - 1U);
			break;
		case 373:    // Aura ONE (daylight)
			deviceQuirks.screenDPI = 300U;
			// Flawfinder: ignore
			strncpy(deviceQuirks.deviceName, "Aura One", sizeof(deviceQuirks.deviceName) - 1U);
			// Flawfinder: ignore
			strncpy(deviceQuirks.deviceCodename, "Daylight", sizeof(deviceQuirks.deviceCodename) - 1U);
			// Flawfinder: ignore
			strncpy(deviceQuirks.devicePlatform, "Mark 6", sizeof(deviceQuirks.devicePlatform) - 1U);
			break;
		case 381:    // Aura ONE LE (daylight)
			deviceQuirks.screenDPI = 300U;
			// Flawfinder: ignore
			strncpy(deviceQuirks.deviceName, "Aura One LE", sizeof(deviceQuirks.deviceName) - 1U);
			// Flawfinder: ignore
			strncpy(deviceQuirks.deviceCodename, "Daylight", sizeof(deviceQuirks.deviceCodename) - 1U);
			// Flawfinder: ignore
			strncpy(deviceQuirks.devicePlatform, "Mark 6", sizeof(deviceQuirks.devicePlatform) - 1U);
			break;
		case 375:    // Aura SE (star)
			deviceQuirks.screenDPI = 212U;
			// Flawfinder: ignore
			strncpy(deviceQuirks.deviceName, "Aura SE", sizeof(deviceQuirks.deviceName) - 1U);
			// Flawfinder: ignore
			strncpy(deviceQuirks.deviceCodename, "Star", sizeof(deviceQuirks.deviceCodename) - 1U);
			// Flawfinder: ignore
			strncpy(deviceQuirks.devicePlatform, "Mark 6", sizeof(deviceQuirks.devicePlatform) - 1U);
			break;
		case 379:    // Aura SE r2 (star)
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
		case 376:    // Clara HD (nova)
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
		case 377:    // Forma (frost)
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
		case 380:    // Forma 32GB (frost)
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
		case 0:
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
		char*              line = NULL;
		size_t             len  = 0;
		ssize_t            nread;
		unsigned short int kobo_id = 0;
		while ((nread = getline(&line, &len, fp)) != -1) {
			// Thankfully, the device code is always located in the three
			// final characters, so that's easy enough to extract without
			// having to worry about the formatting...
			kobo_id = (unsigned short int) strtoul(line + (nread - 3), NULL, 10);
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
		WARN("Couldn't read from '%s', unable to identify the Kobo model", HWCONFIG_DEVICE);
	} else {
#		pragma GCC diagnostic push
#		pragma GCC diagnostic ignored "-Wmissing-braces"
		NTXHWConfig    config           = { 0 };
#		pragma GCC diagnostic pop
		unsigned char* restrict payload = NULL;

		if (fseek(fp, HWCONFIG_OFFSET, SEEK_SET) != 0) {
			WARN("Failed to seek to position 0x%p in '%s'", (void*) HWCONFIG_OFFSET, HWCONFIG_DEVICE);
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
			payload = alloca(config.len);
			if (fread(payload, sizeof(*payload), config.len, fp) < config.len || ferror(fp) != 0) {
				WARN("Error reading NTX HWConfig payload (unexpected length)");
				fclose(fp);
				// NOTE: Make it clear we failed to identify the device...
				set_kobo_quirks(0);
				return;
			}
		}
		fclose(fp);

		// As per /bin/kobo_config.sh, match PCB IDs to Product IDs via a LUT...
		unsigned short int kobo_id = 0;
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
				if (kobo_id == 374 || kobo_id == 375) {
					// Discriminate the Mk.7 version for dual rev models by checking the CPU...
					// NOTE: kobo_cpus[10] == "mx6sll"
					if (payload[KOBO_HWCFG_CPU] == 10) {
						// Thankfully, that works for both the H2O² (374 -> 378),
						// and the Aura SE (375 -> 379) ;)
						kobo_id = (unsigned short int) (kobo_id + 4U);
					}
				} else if (kobo_id == 371) {
					// Discriminate Alyssum from Pika, by checking the Display Resolution...
					// NOTE: kobo_disp_res[0] == "800x600"
					if (payload[KOBO_HWCFG_DisplayResolution] == 0) {
						// Glo HD (Alyssum) [371] -> Touch 2.0 (Pika) [372]
						kobo_id = 372;
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
#	endif    // FBINK_FOR_KINDLE

static void
    identify_device(void)
{
#	if defined(FBINK_FOR_KINDLE)
	identify_kindle();
	if (deviceQuirks.deviceId > 0xFF) {
		char* restrict dev_id    = NULL;
		dev_id                   = to_base(deviceQuirks.deviceId, 32);
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
#	else
	identify_kobo();
	ELOG("Detected a Kobo %s (%hu => %s @ %s)",
	     deviceQuirks.deviceName,
	     deviceQuirks.deviceId,
	     deviceQuirks.deviceCodename,
	     deviceQuirks.devicePlatform);
#	endif
	// Warn if canHWInvert was flipped
	if (!deviceQuirks.canHWInvert) {
		ELOG("This device does not support HW inversion");
	}
}
#endif    // !FBINK_FOR_LINUX
