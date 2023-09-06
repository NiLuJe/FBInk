/*
	FBInk: FrameBuffer eInker, a library to print text & images to an eInk Linux framebuffer
	Copyright (C) 2018-2023 NiLuJe <ninuje@gmail.com>
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
		case DEVICE_KOBO_SAGE:
			// kx122
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
				// NOTE: The Elipsa (PCB index 94) is flagged EBRMAIN_ROTATE_R_180 in the kernel driver
				rota = (rota + 2) & 3;
			} else {
				// And defaults to FACE_INVERSE_NONE, which actually means we *do* invert them...
				rota = rota ^ 3;
			}
			break;
		case DEVICE_KOBO_SAGE:
			// NOTE: On the Sage (PCB index 98), this is left at EBRMAIN_ROTATE_R_0.
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
	} else {
		PFWARN("fopen: %m");
		return ERRCODE(ENOENT);
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
		case DEVICE_KOBO_SAGE:
			// kx122
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

// This tries to abstract the mess away by using a simple LUT for the rotation mapping,
// which can then be paired with simple swap & invert flags for touch input translations,
// much like what is done in KOReader or rmkit (c.f., utils/finger_trace.c for a practical example).
// For other approaches, you can look at Plato:
// https://github.com/baskerville/plato/blob/f45c2da65bc556bc22d664b2f9450f95c550dbf5/src/device.rs#L265-L326
// See also rotate_touch_coordinates @ fbink.c for an earlier and gnarlier attempt at this.
// And if you liked that one, you can look at the old implementations, which are archived in tools/rota_map.c ;).
uint8_t
    fbink_rota_native_to_canonical(uint32_t rotate UNUSED_BY_NOTKOBO)
{
#if defined(FBINK_FOR_KOBO)
	// Safety net for bogus input
	if (rotate > FB_ROTATE_CCW) {
		return ERANGE;
	}

	return deviceQuirks.rotationMap[rotate];
#else
	WARN("Rotation quirks are only handled on Kobo");
	return ENOSYS;
#endif
}

uint32_t
    fbink_rota_canonical_to_native(uint8_t rotate UNUSED_BY_NOTKOBO)
{
#if defined(FBINK_FOR_KOBO)
	// This is a less common operation, so we don't actually keep a reverse LUT around.
	for (uint8_t rota = FB_ROTATE_UR; rota <= FB_ROTATE_CCW; rota++) {
		if (deviceQuirks.rotationMap[rota] == rotate) {
			return rota;
		}
	}

	// No match means input was out of range
	return ERANGE;
#else
	WARN("Rotation quirks are only handled on Kobo");
	return ENOSYS;
#endif
}

#ifdef FBINK_FOR_KINDLE
static orientation_t
    linuxfb_rotate_to_einkfb_orientation(uint32_t rotate)
{
	switch (rotate) {
		case FB_ROTATE_UR:
			return orientation_portrait;
		case FB_ROTATE_CW:
			return orientation_landscape;
		case FB_ROTATE_UD:
			return orientation_portrait_upside_down;
		case FB_ROTATE_CCW:
			return orientation_landscape_upside_down;
		default:
			// Should never happen.
			return orientation_portrait;
	}
}
#endif

#if defined(FBINK_FOR_KOBO)
static int
    kobo_mtk_invert_fb(bool toggle)
{
	int fd = open("/proc/hwtcon/cmd", O_WRONLY | O_NONBLOCK | O_CLOEXEC);
	if (fd == -1) {
		PFWARN("Unable to open hwtcon/cmd procfs knob: %m");
		return EXIT_FAILURE;
	}

	ssize_t wlen;
	if (toggle) {
		const unsigned char cmd[] = "night_mode 4";
		wlen                      = write(fd, cmd, sizeof(cmd));
	} else {
		const unsigned char cmd[] = "night_mode 0";
		wlen                      = write(fd, cmd, sizeof(cmd));
	}
	int rv = EXIT_SUCCESS;
	if (wlen < 0) {
		PFWARN("Failed to write to hwtcon/cmd procfs knob: %m");
		rv = EXIT_FAILURE;
	}

	close(fd);
	return rv;
}
#endif

// See utils/fbdepth.c for all the gory details ;).
int
    fbink_set_fb_info(int fbfd, uint32_t rota, uint8_t bpp, uint8_t grayscale, const FBInkConfig* restrict fbink_cfg)
{
	if (!deviceQuirks.skipId) {
		PFWARN("FBInk hasn't been initialized yet");
		return ERRCODE(ENODEV);
	}

	// Validate...
	switch (rota) {
		case FB_ROTATE_UR:
		case FB_ROTATE_CW:
		case FB_ROTATE_UD:
		case FB_ROTATE_CCW:
		case KEEP_CURRENT_ROTATE:
			// NOP
			break;
		default:
			PFWARN("Unsupported rotation value: %u", rota);
			return ERRCODE(EINVAL);
	}

	// We have our own private buffer on sunxi, so we can only deal with rotation...
	if (deviceQuirks.isSunxi) {
		return fbink_sunxi_ntx_enforce_rota(fbfd, (SUNXI_FORCE_ROTA_INDEX_T) rota, fbink_cfg);
	}

	// Validate the rest...
	switch (bpp) {
		case 4U:
		case 8U:
		case 16U:
		case 24U:    // Technically supported here, but often hilariously broken in practice.
		case 32U:
		case KEEP_CURRENT_BITDEPTH:
			// NOP
			break;
		default:
			PFWARN("Unsupported bitdepth value: %hhu", bpp);
			return ERRCODE(EINVAL);
	}

	switch (grayscale) {
		case 0U:
		case GRAYSCALE_8BIT:
		case GRAYSCALE_8BIT_INVERTED:
		case GRAYSCALE_4BIT:
		case GRAYSCALE_4BIT_INVERTED:
		case KEEP_CURRENT_GRAYSCALE:
		case TOGGLE_GRAYSCALE:
			// NOP
			break;
		default:
			PFWARN("Unsupported grayscale value: %hhu", grayscale);
			return ERRCODE(EINVAL);
	}

	// Work on copy to avoid breaking fbink_reinit...
	struct fb_var_screeninfo new_vinfo = vInfo;

	// Start with the easy stuff...
	if (bpp == KEEP_CURRENT_BITDEPTH) {
		LOG("Keeping current bitdepth: %ubpp", vInfo.bits_per_pixel);
	} else {
		LOG("Updating bitdepth from %ubpp to %hhubpp", vInfo.bits_per_pixel, bpp);
		new_vinfo.bits_per_pixel = (uint32_t) bpp;
	}

	if (new_vinfo.bits_per_pixel == 8U) {
		if (grayscale == KEEP_CURRENT_GRAYSCALE) {
			if (bpp == KEEP_CURRENT_BITDEPTH) {
				LOG("Keeping current grayscale value: %u", vInfo.grayscale);
			} else {
				LOG("Sanitizing grayscale flag for 8bpp");
				new_vinfo.grayscale = GRAYSCALE_8BIT;
			}
		} else {
			if (grayscale == TOGGLE_GRAYSCALE) {
				if (vInfo.grayscale == GRAYSCALE_8BIT) {
					grayscale = GRAYSCALE_8BIT_INVERTED;
				} else {
					grayscale = GRAYSCALE_8BIT;
				}
			}
			LOG("Updating grayscale value from %u to %hhu", vInfo.grayscale, grayscale);
			new_vinfo.grayscale = (uint32_t) grayscale;
		}
	} else if (new_vinfo.bits_per_pixel == 4U) {
		LOG("Sanitizing grayscale flag for 4bpp");
		new_vinfo.grayscale = 1U;
	} else {
		LOG("Sanitizing grayscale flag for bitdepths > 8bpp");
		new_vinfo.grayscale = 0U;
	}

#if defined(FBINK_FOR_KOBO)
	// On MTK, flip the `invert_fb` flag accordingly, as the driver doesn't do that for us.
	// NOTE: So far, the driver itself doesn't seem to care all that much about the state of the grayscale flag itself,
	//       so we're free to actually modify it like if we were on mxcfb.
	//       That allows us to handle a toggle sanely, as we don't actually have a getter for the `invert_fb` flag...
	if (deviceQuirks.isMTK && new_vinfo.grayscale != vInfo.grayscale) {
		if (new_vinfo.grayscale == GRAYSCALE_8BIT_INVERTED) {
			// Inverted grayscale, flip `invert_fb` to *on*
			// NOTE: Technically, we could actually very much have it enabled at 16bpp or 32bpp,
			//       but for simplicity's sake, we follow our usual "Y8 only" API.
			if (kobo_mtk_invert_fb(true) == EXIT_SUCCESS) {
				LOG("Enabled the MTK invert_fb flag");
			}
		} else {
			// Color or standard grayscale, flip `invert_fb` to *off*
			if (kobo_mtk_invert_fb(false) == EXIT_SUCCESS) {
				LOG("Disabled the MTK invert_fb flag");
			}
		}
	}
#endif

	if (rota == KEEP_CURRENT_ROTATE) {
		LOG("Keeping current rotation: %u (%s)", vInfo.rotate, fb_rotate_to_string(vInfo.rotate));
	} else {
		LOG("Updating rotation from %u (%s) to %u (%s)",
		    vInfo.rotate,
		    fb_rotate_to_string(vInfo.rotate),
		    rota,
		    fb_rotate_to_string(rota));
		new_vinfo.rotate = rota;
	}

	// Open the framebuffer if need be (nonblock, we'll only do ioctls)...
	bool keep_fd = true;
	if (open_fb_fd_nonblock(&fbfd, &keep_fd) != EXIT_SUCCESS) {
		return ERRCODE(EXIT_FAILURE);
	}

	// Assume success, until shit happens ;)
	int rv = EXIT_SUCCESS;

	// NOTE: We have to counteract the rotation shenanigans the Kernel might be enforcing...
	//       c.f., mxc_epdc_fb_check_var @ drivers/video/mxc/mxc_epdc_fb.c OR drivers/video/fbdev/mxc/mxc_epdc_v2_fb.c
	//       The goal being to end up in the *same* effective rotation as before.
	// First, remember the current rotation as the expected one...
#if defined(FBINK_FOR_KOBO) || defined(FBINK_FOR_CERVANTES) || defined(FBINK_FOR_KINDLE)
	uint32_t expected_rota = new_vinfo.rotate;
#endif

#if defined(FBINK_FOR_KOBO)
	if (deviceQuirks.ntxRotaQuirk == NTX_ROTA_ALL_INVERTED) {
		// NOTE: This should cover the H2O and the few other devices suffering from the same quirk...
		new_vinfo.rotate ^= 2;
		LOG("Mangling rotate to %u (%s) to account for kernel rotation quirks",
		    new_vinfo.rotate,
		    fb_rotate_to_string(new_vinfo.rotate));
	} else if (deviceQuirks.ntxRotaQuirk == NTX_ROTA_ODD_INVERTED) {
		// NOTE: This is for the Forma, which only inverts CW & CCW (i.e., odd numbers)...
		if ((new_vinfo.rotate & 0x01) == 1) {
			new_vinfo.rotate ^= 2;
			LOG("Mangling rotate to %u (%s) to account for kernel rotation quirks",
			    new_vinfo.rotate,
			    fb_rotate_to_string(new_vinfo.rotate));
		}
	}
#endif

	if (ioctl(fbfd, FBIOPUT_VSCREENINFO, &new_vinfo)) {
		PFWARN("FBIOPUT_VSCREENINFO: %m");
		rv = ERRCODE(ECANCELED);
		goto cleanup;
	}

#ifdef FBINK_FOR_KINDLE
	// Deal once again with einkfb properly...
	if (deviceQuirks.isKindleLegacy) {
		orientation_t orientation = linuxfb_rotate_to_einkfb_orientation(expected_rota);
		if (ioctl(fbfd, FBIO_EINK_SET_DISPLAY_ORIENTATION, orientation)) {
			PFWARN("FBIO_EINK_SET_DISPLAY_ORIENTATION: %m");
			rv = ERRCODE(ECANCELED);
			goto cleanup;
		}

		LOG("Setting actual einkfb orientation to %u (%s)",
		    orientation,
		    einkfb_orientation_to_string(orientation));
	}
#endif

#if defined(FBINK_FOR_KOBO) || defined(FBINK_FOR_CERVANTES)
	// NOTE: Double-check that we weren't bit by rotation quirks...
	if (new_vinfo.rotate != expected_rota) {
		LOG("Current rotation (%u) doesn't match the expected rotation (%u), attempting to fix it . . .",
		    new_vinfo.rotate,
		    expected_rota);

		// Brute-force it until it matches...
		for (uint32_t i = new_vinfo.rotate, j = FB_ROTATE_UR; j <= FB_ROTATE_CCW; i = (i + 1U) & 3U, j++) {
			// If we finally got the right orientation, break the loop
			if (new_vinfo.rotate == expected_rota) {
				break;
			}
			// Do the i -> i + 1 -> i dance to be extra sure...
			// (This is useful on devices where the kernel *always* switches to the invert orientation, c.f., rota.c)
			new_vinfo.rotate = i;
			if (ioctl(fbfd, FBIOPUT_VSCREENINFO, &new_vinfo)) {
				PFWARN("FBIOPUT_VSCREENINFO: %m");
				rv = ERRCODE(ECANCELED);
				goto cleanup;
			}
			LOG("Kernel rotation quirk recovery: %u -> %u", i, new_vinfo.rotate);

			// Don't do anything extra if that was enough...
			if (new_vinfo.rotate == expected_rota) {
				continue;
			}
			// Now for i + 1 w/ wraparound, since the valid rotation range is [0..3] (FB_ROTATE_UR to FB_ROTATE_CCW).
			// (i.e., a Portrait/Landscape swap to counteract potential side-effects of a kernel-side mandatory invert)
			uint32_t n       = (i + 1U) & 3U;
			new_vinfo.rotate = n;
			if (ioctl(fbfd, FBIOPUT_VSCREENINFO, &new_vinfo)) {
				PFWARN("FBIOPUT_VSCREENINFO: %m");
				rv = ERRCODE(ECANCELED);
				goto cleanup;
			}
			LOG("Kernel rotation quirk recovery (intermediary @ %u): %u -> %u", i, n, new_vinfo.rotate);

			// And back to i, if need be...
			if (new_vinfo.rotate == expected_rota) {
				continue;
			}
			new_vinfo.rotate = i;
			if (ioctl(fbfd, FBIOPUT_VSCREENINFO, &new_vinfo)) {
				PFWARN("FBIOPUT_VSCREENINFO: %m");
				rv = ERRCODE(ECANCELED);
				goto cleanup;
			}
			LOG("Kernel rotation quirk recovery: %u -> %u", i, new_vinfo.rotate);
		}
	}

	// Finally, warn if things *still* look FUBAR...
	if (new_vinfo.rotate != expected_rota) {
		LOG("Current rotation (%u) doesn't match the expected rotation (%u), here be dragons!",
		    new_vinfo.rotate,
		    expected_rota);
	}
#endif

#ifdef FBINK_FOR_KINDLE
	// And, again, einkfb is a special snowflake...
	if (deviceQuirks.isKindleLegacy) {
		orientation_t orientation = orientation_portrait;
		if (ioctl(fbfd, FBIO_EINK_GET_DISPLAY_ORIENTATION, &orientation)) {
			PFWARN("FBIO_EINK_GET_DISPLAY_ORIENTATION: %m");
			rv = ERRCODE(ECANCELED);
			goto cleanup;
		}

		LOG("Actual einkfb orientation is now %u (%s)", orientation, einkfb_orientation_to_string(orientation));
	}
#endif

	// Recap
	LOG("Bitdepth is now %ubpp (grayscale: %u) @ rotate: %u (%s)",
	    new_vinfo.bits_per_pixel,
	    new_vinfo.grayscale,
	    new_vinfo.rotate,
	    fb_rotate_to_string(new_vinfo.rotate));

	rv = fbink_reinit(fbfd, fbink_cfg);

cleanup:
	if (!keep_fd) {
		close_fb(fbfd);
	}

	return rv;
}
