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

// PoC for reading the Elipsa's accelerometer status over I2C,
// because the sysfs attributes only expose the *current* status,
// not the *previous*, which makes handling Face Up/Face Down states
// because of the lakc of history ;).

// Because we're pretty much Linux-bound ;).
#ifndef _GNU_SOURCE
#	define _GNU_SOURCE
#endif

#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <linux/i2c-dev.h>
#include <linux/i2c.h>

#include <sys/ioctl.h>

// We need i2c-dev ;)
#include <i2c/smbus.h>
// We need the KX122 register constants :).
#include "../eink/kx122-kobo.h"

typedef struct
{
	uint16_t bus;
	uint16_t address;
} FBInkI2CDev;

// Figure out on which bus and at which address is the KX122 driver registered.
static int
    find_accelerometer(const char* driver, FBInkI2CDev* i2c_dev)
{
	char sysfs_path[PATH_MAX] = { 0 };
	snprintf(sysfs_path, sizeof(sysfs_path) - 1U, "/sys/bus/i2c/drivers/%s/", driver);

	DIR* dir = opendir(sysfs_path);
	if (!dir) {
		fprintf(stderr, "opendir: %m\n");
		return -(errno);
	}

	int rv = EXIT_SUCCESS;

	struct dirent* de;
	while ((de = readdir(dir)) != NULL) {
		printf("Iterating on `%s`\n", de->d_name);
		// We're looking for a symlink...
		if (de->d_type != DT_LNK) {
			continue;
		}
		printf("It's a symlink!\n");

		// There should only one, so, just parse it...
		if (sscanf(de->d_name, "%hu-%hx", &i2c_dev->bus, &i2c_dev->address) != 2) {
			fprintf(stderr, "sscanf\n");
			i2c_dev->bus     = 0U;
			i2c_dev->address = 0U;
			rv               = -(EXIT_FAILURE);
			goto cleanup;
		} else {
			break;
		}
	}

cleanup:
	closedir(dir);

	return rv;
}

int
    main(void)
{
	int rv = EXIT_SUCCESS;

	// Find where to poke at...
	FBInkI2CDev i2c_dev = { 0 };

	if (find_accelerometer("kx122", &i2c_dev) != EXIT_SUCCESS) {
		fprintf(stderr, "Failed to find the accelerometer!\n");
		return EXIT_FAILURE;
	}
	printf("KX122 is on bus %hu @ addr %#x\n", i2c_dev.bus, i2c_dev.address);

	// Open the right I2C character device
	char dev_path[PATH_MAX] = { 0 };
	snprintf(dev_path, sizeof(dev_path) - 1U, "/dev/i2c-%hu", i2c_dev.bus);

	int i2c_fd = open(dev_path, O_RDONLY | O_NONBLOCK | O_CLOEXEC);
	if (i2c_fd == -1) {
		fprintf(stderr, "open: %m\n");
		return EXIT_FAILURE;
	}

	// Poke at the right address (force, because there's already a kernel driver on it).
	if (ioctl(i2c_fd, I2C_SLAVE_FORCE, i2c_dev.address) < 0) {
		fprintf(stderr, "I2C_SLAVE_FORCE: %m\n");
		rv = EXIT_FAILURE;
		goto cleanup;
	}

	// And, finally, poke at the registers...
	int32_t current_state = i2c_smbus_read_byte_data(i2c_fd, KX122_TSCP);
	if (current_state < 0) {
		fprintf(stderr, "Failed to read TSCP register: %m!\n");
		rv = EXIT_FAILURE;
		goto cleanup;
	}

	int32_t previous_state = i2c_smbus_read_byte_data(i2c_fd, KX122_TSPP);
	if (previous_state < 0) {
		fprintf(stderr, "Failed to read TSPP register: %m!\n");
		rv = EXIT_FAILURE;
		goto cleanup;
	}

	printf("TSCP: %#hx // TSPP: %#hx\n", (uint16_t) current_state, (uint16_t) previous_state);

cleanup:
	if (i2c_fd != -1) {
		close(i2c_fd);
	}

	return rv;
}
