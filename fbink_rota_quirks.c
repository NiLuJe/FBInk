/*
	FBInk: FrameBuffer eInker, a library to print text & images to an eInk Linux framebuffer
	Copyright (C) 2018-2021 NiLuJe <ninuje@gmail.com>
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

#if defined(FBINK_FOR_KOBO)
// Figure out on which bus and at which address is the accelerometer driver registered.
static int
    find_accelerometer(const char* driver)
{
	char sysfs_path[PATH_MAX] = { 0 };
	snprintf(sysfs_path, sizeof(sysfs_path) - 1U, "/sys/bus/i2c/drivers/%s/", driver);

	DIR* dir = opendir(sysfs_path);
	if (!dir) {
		PFWARN("opendir: %m");
		return ERRCODE(errno);
	}

	struct dirent* de;
	while ((de = readdir(dir)) != NULL) {
		// We're looking for a symlink...
		if (de->d_type != DT_LNK) {
			continue;
		}

		// Parse it...
		if (sscanf(de->d_name, "%hu-%hx", &sunxiCtx.i2c_dev.bus, &sunxiCtx.i2c_dev.address) != 2) {
			PFWARN("Failed to parse `%s` via sscanf: %m", de->d_name);
			sunxiCtx.i2c_dev.bus     = 0U;
			sunxiCtx.i2c_dev.address = 0U;
			return ERRCODE(EXIT_FAILURE);
		} else {
			break;
		}
	}

	return EXIT_SUCCESS;
}

// Close the I²C fd if necessary
static int
    close_accelerometer_i2c(void)
{
	int rv = EXIT_SUCCESS;

	if (sunxiCtx.i2c_fd != -1) {
		if (close(sunxiCtx.i2c_fd) != 0) {
			PFWARN("close: %m");
			rv = ERRCODE(EXIT_FAILURE);
		} else {
			sunxiCtx.i2c_fd          = -1;
			sunxiCtx.i2c_dev.bus     = 0U;
			sunxiCtx.i2c_dev.address = 0U;
		}
	}

	return rv;
}

// Search for the right accelerometer for the device
static int
    populate_accelerometer_i2c_info(void)
{
	switch (deviceQuirks.deviceId) {
		case 387U:
			// Ellipsa, kx122
			// NOTE: Could be queried via NTXHWConfig ([11] RSensor).
			if (find_accelerometer("kx122") != EXIT_SUCCESS) {
				WARN("Failed to find the I²C bus/address combo for the KX122 accelerometer");
				return ERRCODE(EXIT_FAILURE);
			}
			break;
		default:
			WARN("Unknown accelerometer for this device");
			return ERRCODE(ENOSYS);
			break;
	}

	return EXIT_SUCCESS;
}

// Open the right I²C character device for the device's accelerometer...
static int
    open_accelerometer_i2c(void)
{
	int rv = EXIT_SUCCESS;

	// Check if the address looks sane...
	if (sunxiCtx.i2c_dev.address < 0x08 || sunxiCtx.i2c_dev.address > 0x77) {
		WARN("I²C address for the accelerometer looks fishy (%#hx), aborting", sunxiCtx.i2c_dev.address);
		rv = ERRCODE(ENOTSUP);
		goto cleanup;
	}

	// Okay, we've got a bus and an address, now open it :).
	char dev_path[PATH_MAX] = { 0 };
	snprintf(dev_path, sizeof(dev_path) - 1U, "/dev/i2c-%hu", sunxiCtx.i2c_dev.bus);

	sunxiCtx.i2c_fd = open(dev_path, O_RDONLY | O_NONBLOCK | O_CLOEXEC);
	if (sunxiCtx.i2c_fd == -1) {
		PFWARN("open: %m");
		rv = ERRCODE(EXIT_FAILURE);
		goto cleanup;
	}

	// Poke at the right address (force, because there's already a kernel driver on it).
	if (ioctl(sunxiCtx.i2c_fd, I2C_SLAVE_FORCE, sunxiCtx.i2c_dev.address) < 0) {
		PFWARN("I2C_SLAVE_FORCE: %m");
		rv = ERRCODE(EXIT_FAILURE);
		goto cleanup;
	}

	return EXIT_SUCCESS;

cleanup:
	if (close_accelerometer_i2c() != EXIT_SUCCESS) {
		rv = ERRCODE(EXIT_FAILURE);
	}

	return rv;
}

// Make sense of the register constants...
static int
    translate_kx122(uint16_t val)
{
	int rota = -1;

	// c.f., drivers/input/sensor/kx122.c
	// (NOTE: TSCP & TSPP bits are identical ;)).
	if (val & KX122_TSCP_FU) {
		// Face Up, which doesn't tell us much
		rota = -2;
	} else if (val & KX122_TSCP_FD) {
		// Ditto for Face Down
		rota = -3;
	}

	// NOTE: While the driver code would imply that this is a bitmask and we could get FU/FD | U/D/L/R,
	//       it unfortunately isn't so...
	if (val & KX122_TSCP_UP) {
		rota = FB_ROTATE_UR;
	} else if (val & KX122_TSCP_RI) {
		rota = FB_ROTATE_CW;
	} else if (val & KX122_TSCP_DO) {
		rota = FB_ROTATE_UD;
	} else if (val & KX122_TSCP_LE) {
		rota = FB_ROTATE_CCW;
	}

	// If we got an actionable value (e.g., not FU/FD), translate it accordingly for our device...
	if (rota >= 0) {
		switch (deviceQuirks.deviceId) {
			case 387U:
				// NOTE: The Ellipsa (PCB index 94) is flagged EBRMAIN_ROTATE_R_180 in the kernel driver
				rota = (rota + 2) & 3;
				break;
			default:
				WARN("Unsupported KX122 translation for this device");
				break;
		}
	}

	return rota;
}

// Poke at the right registers for a KX122
static int
    query_kx122(void)
{
	// Start by checking the *current* status
	int32_t state = i2c_smbus_read_byte_data(sunxiCtx.i2c_fd, KX122_TSCP);
	if (state < 0) {
		PFWARN("Failed to read TSCP register: %m");
		return ERRCODE(EXIT_FAILURE);
	}

	int rota = translate_kx122((uint16_t) state);
	LOG("KX122 says the current rotation is: %d", rota);
	// If we got an actionable value, we're done!
	if (rota >= 0) {
		return rota;
	}

	// Otherwise, check the *previous* status...
	state = i2c_smbus_read_byte_data(sunxiCtx.i2c_fd, KX122_TSPP);
	if (state < 0) {
		PFWARN("Failed to read TSPP register: %m");
		return ERRCODE(EXIT_FAILURE);
	}

	rota = translate_kx122((uint16_t) state);
	LOG("KX122 says the previous rotation was: %d", rota);
	// If we got an actionable value, we're done!
	if (rota >= 0) {
		return rota;
	} else {
		WARN("Could not get actionable data out of the KX122 accelerometer");
		return ERRCODE(ENODATA);
	}
}

// And, finally, get the current rotation, according to the accelerometer...
static int
    query_accelerometer(void)
{
	// We need an I²C handle
	if (sunxiCtx.i2c_fd == -1) {
		WARN("No I²C handle, can't query the accelerometer");
		return ERRCODE(ENOENT);
	}

	int rv = FB_ROTATE_UR;

	// We need to know how to handle the accelerometer...
	switch (deviceQuirks.deviceId) {
		case 387U:
			// Ellipsa, kx122
			rv = query_kx122();
			if (rv < 0) {
				WARN("Poking at the KX122 accelerometer over I²C was unfruitful");
				return rv;
			}
			break;
		default:
			WARN("Unknown accelerometer for this device");
			return ERRCODE(ENOSYS);
			break;
	}

	return rv;
}
#endif    // FBINK_FOR_KOBO

// Try to make sense out of the mess that are native Kobo rotations...
// Vaguely inspired by Plato's implementation,
// c.f., https://github.com/baskerville/plato/blob/f45c2da65bc556bc22d664b2f9450f95c550dbf5/src/device.rs#L265-L326
// except not really, because that didn't work at all on my quirky devices ;).
// See also rotate_touch_coordinates @ fbink.c for another attempt at this, which may or may not be worse ;p.
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
