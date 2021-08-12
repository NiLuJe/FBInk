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

	int rv = EXIT_SUCCESS;

	struct dirent* de;
	while ((de = readdir(dir)) != NULL) {
		// We're looking for a symlink...
		if (de->d_type != DT_LNK) {
			continue;
		}

		// There should only one, so, just parse it...
		if (sscanf(de->d_name, "%hu-%hx", &sunxiCtx.i2c_dev.bus, &sunxiCtx.i2c_dev.address) != 2) {
			PFWARN("Failed to parse `%s` via sscanf: %m", de->d_name);
			sunxiCtx.i2c_dev.bus     = 0U;
			sunxiCtx.i2c_dev.address = 0U;
			rv                       = ERRCODE(EXIT_FAILURE);
			goto cleanup;
		} else {
			break;
		}
	}

cleanup:
	closedir(dir);

	return rv;
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
			sunxiCtx.i2c_fd = -1;
		}
	}

	return rv;
}

// Search for the right accelerometer for the device
static int
    populate_accelerometer_i2c_info(void)
{
	switch (deviceQuirks.deviceId) {
		case DEVICE_KOBO_ELIPSA:
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

static __attribute__((cold)) const char*
    gyro_state_to_string(int state)
{
	switch (state) {
		case GYRO_STATE_FACE_UP:
			return "Face Up";
		case GYRO_STATE_FACE_DOWN:
			return "Face Down";
		case FB_ROTATE_UR:
			return "Upright, 0°";
		case FB_ROTATE_CW:
			return "Clockwise, 90°";
		case FB_ROTATE_UD:
			return "Upside Down, 180°";
		case FB_ROTATE_CCW:
			return "Counter Clockwise, 270°";
		default:
			return "Unknown?!";
	}
}

// Make sense of the register constants...
static int
    translate_kx122(uint16_t val)
{
	int rota = GYRO_STATE_UNKNOWN;

	// c.f., drivers/input/sensor/kx122.c
	// (NOTE: TSCP & TSPP bits are identical ;)).
	if (val & KX122_TSCP_FU) {
		// Face Up, which doesn't tell us much
		rota = GYRO_STATE_FACE_UP;
	} else if (val & KX122_TSCP_FD) {
		// Ditto for Face Down
		rota = GYRO_STATE_FACE_DOWN;
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

	// Translate it accordingly for our device...
	switch (deviceQuirks.deviceId) {
		case DEVICE_KOBO_ELIPSA:
			if (rota >= 0) {
				// NOTE: The Ellipsa (PCB index 94) is flagged EBRMAIN_ROTATE_R_180 in the kernel driver
				rota = (rota + 2) & 3;
			} else {
				// And defaults to FACE_INVERSE_NONE, which actually means we *do* invert them...
				rota = rota ^ 3;
			}
			break;
		default:
			WARN("Unsupported KX122 translation for this device");
			break;
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
	LOG("KX122 says the current rotation is: %#hx -> %d (%s)", (uint16_t) state, rota, gyro_state_to_string(rota));
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
	LOG("KX122 says the previous rotation was: %#hx -> %d (%s)", (uint16_t) state, rota, gyro_state_to_string(rota));
	// If we got an actionable value, we're done!
	if (rota >= 0) {
		return rota;
	} else {
		LOG("Could not get actionable data out of the KX122 accelerometer");
		return ERRCODE(ENODATA);
	}
}

// Get the current G2D rotation angle of the working buffer, and translate it to a linuxfb rotation constant
static int
    query_fbdamage(void)
{
	char g2d_rotate[8] = { 0 };

	// We should never be called without fbdamage support.
	FILE* f = fopen(FBDAMAGE_ROTATE_SYSFS, "re");
	if (f) {
		size_t size = fread(g2d_rotate, sizeof(*g2d_rotate), sizeof(g2d_rotate) - 1U, f);
		fclose(f);
		if (size > 0) {
			// Strip trailing LF
			if (g2d_rotate[size - 1U] == '\n') {
				g2d_rotate[size - 1U] = '\0';
			}
		} else {
			PFWARN("Failed to read G2D rotation angle from sysfs");
			return ERRCODE(EINVAL);
		}
	}

	// We should be able to get by with an unchecked strtoul...
	uint32_t rota_angle = (uint32_t) strtoul(g2d_rotate, NULL, 10);

	// Convert that angle to a linuxfb rotation, according to device quirks...
	// (kobo_sunxi_fb_fixup does the reverse).
	uint32_t rota = (deviceQuirks.ntxBootRota - (rota_angle / 90U)) & 3;
	LOG("FBDamage says the working buffer's rotation is: %u (%s)", rota, fb_rotate_to_string(rota));

	return (int) rota;
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
		case DEVICE_KOBO_ELIPSA:
			// Ellipsa, kx122
			rv = query_kx122();
			if (rv < 0) {
				LOG("Poking at the KX122 accelerometer over I²C was unfruitful");
				return rv;
			}
			break;
		default:
			WARN("Unknown accelerometer for this device");
			return ERRCODE(ENOSYS);
			break;
	}

	// Check if what we got matches the force_rota constraints...
	if (sunxiCtx.force_rota == FORCE_ROTA_PORTRAIT) {
		if ((rv & 0x01) == 1) {
			// Odd, Landscape :(
			LOG("Gyro state falls outside of the requested constraints (Landscape instead of Portrait), ignoring it");
			rv = GYRO_STATE_OUTSIDE_CONSTRAINTS;
		}
	} else if (sunxiCtx.force_rota == FORCE_ROTA_LANDSCAPE) {
		if ((rv & 0x01) == 0) {
			// Even, Portrait :(
			LOG("Gyro state falls outside of the requested constraints (Portrait instead of Landscape), ignoring it");
			rv = GYRO_STATE_OUTSIDE_CONSTRAINTS;
		}
	} else if (sunxiCtx.force_rota == FORCE_ROTA_CURRENT_ROTA || sunxiCtx.force_rota == FORCE_ROTA_CURRENT_LAYOUT) {
		int wb_rotate = query_fbdamage();
		if (wb_rotate < 0) {
			LOG("FBDamage is inconclusive, assuming Upright");
			wb_rotate = FB_ROTATE_UR;
		}

		if (sunxiCtx.force_rota == FORCE_ROTA_CURRENT_ROTA) {
			if (rv != wb_rotate) {
				LOG("Gyro state falls outside of the requested constraints (same rotation as working buffer), honoring working buffer's state instead");
				rv = wb_rotate;
			}
		} else if (sunxiCtx.force_rota == FORCE_ROTA_CURRENT_LAYOUT) {
			if ((rv & 0x01) != (wb_rotate & 0x01)) {
				LOG("Gyro state falls outside of the requested constraints (same layout as working buffer), honoring working buffer's state instead");
				rv = wb_rotate;
			}
		}
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
