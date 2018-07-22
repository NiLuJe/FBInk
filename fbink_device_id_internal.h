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

#ifndef __FBINK_DEVICE_ID_INTERNAL_H
#define __FBINK_DEVICE_ID_INTERNAL_H

// Make KDevelop happy (for getline)
#ifndef _DEFAULT_SOURCE
#	define _DEFAULT_SOURCE
#endif

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>

// For FBInkDeviceQuirks
#include "fbink_types.h"

#define KINDLE_SERIAL_NO_LENGTH 16

#if defined(FBINK_FOR_KINDLE) || defined(FBINK_FOR_LEGACY)
static bool     is_kindle_device(uint32_t, FBInkDeviceQuirks*);
static bool     is_kindle_device_new(uint32_t, FBInkDeviceQuirks*);
static uint32_t from_base(char*, unsigned short int);
static void     identify_kindle(FBInkDeviceQuirks*);
#else
static void identify_kobo(FBInkDeviceQuirks*);
#endif

#endif
