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
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <sys/ioctl.h>

// We need i2c-dev ;)
//#include "../i2c-tools/include/i2c/smbus.h"
//#include "../i2c-tools/lib/smbus.c"
// We need the KX122 register constants :).
//#include "../eink/kx122-kobo.h"

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

	DIR *dir = opendir(sysfs_path);
	if (!dir) {
		fprintf(stderr, "opendir: %m\n");
		return -(errno);
	}

	struct dirent *de;
	while ((de = readdir(dir)) != NULL) {
		printf("Iterating on `%s`\n", de->d_name);
		// We're looking for a symlink...
		if (de->d_type != DT_LNK) {
			continue;
		}
		printf("It's a symlink!\n");

		char i2c_sym[NAME_MAX] = { 0 };
		strncpy(i2c_sym, de->d_name, sizeof(i2c_sym) - 1U);

		// Parse it...
		if (sscanf(i2c_sym, "%hu-%hx", &i2c_dev->bus, &i2c_dev->address) != 2) {
			fprintf(stderr, "sscanf\n");
			i2c_dev->bus = 0U;
			i2c_dev->address = 0U;
			return EXIT_FAILURE;
		} else {
			break;
		}
	}

	return EXIT_SUCCESS;
}

int
    main(void)
{
	FBInkI2CDev i2c_dev = { 0 };

	find_accelerometer("kx122", &i2c_dev);
	printf("KX122 is on bus %hu @ addr %#x\n", i2c_dev.bus, i2c_dev.address);

	return EXIT_SUCCESS;
}
