/*
 * Copyright (c) 2020 Amazon.com, Inc. or its affiliates. All Rights Reserved.
 *
 * ----
 *
 * This is <linux/frontlight.h>, last updated from the PW5 kernel for FW 5.14.1.1
 *
 * NOTE: Upstream kernels available here: https://www.amazon.com/gp/help/customer/display.html?nodeId=200203720
 *
 * - Trimmed down to avoid pulling in other non-standard kernel headers -- NiLuJe
 *
 * ----
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See http://www.gnu.org/licenses/gpl-2.0.html for more details.
 *
 * Accelerometer Sensor Driver
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 */
#ifndef __LINUX_FRONTLIGHT_H
#define __LINUX_FRONTLIGHT_H

#include <linux/ioctl.h>

#define FL_DEV_FILE "/dev/frontlight"

#define FL_MAGIC_NUMBER 'L'

#define FL_IOCTL_SET_INTENSITY                _IOW(FL_MAGIC_NUMBER, 0x01, int)
#define FL_IOCTL_GET_INTENSITY                _IOR(FL_MAGIC_NUMBER, 0x02, int)
#define FL_IOCTL_GET_RANGE_MAX                _IOR(FL_MAGIC_NUMBER, 0x03, int)
#define FL_IOCTL_SET_INTENSITY_FORCED         _IOW(FL_MAGIC_NUMBER, 0x04, int)
#define FL_IOCTL_SET_INTENSITY_AMBER_1        _IOW(FL_MAGIC_NUMBER, 0x05, int)
#define FL_IOCTL_GET_INTENSITY_AMBER_1        _IOR(FL_MAGIC_NUMBER, 0x06, int)
#define FL_IOCTL_GET_RANGE_MAX_AMBER_1        _IOR(FL_MAGIC_NUMBER, 0x07, int)
#define FL_IOCTL_SET_INTENSITY_FORCED_AMBER_1 _IOW(FL_MAGIC_NUMBER, 0x08, int)
#define FL_IOCTL_SET_INTENSITY_AMBER_2        _IOW(FL_MAGIC_NUMBER, 0x09, int)
#define FL_IOCTL_GET_INTENSITY_AMBER_2        _IOR(FL_MAGIC_NUMBER, 0x0a, int)
#define FL_IOCTL_GET_RANGE_MAX_AMBER_2        _IOR(FL_MAGIC_NUMBER, 0x0b, int)
#define FL_IOCTL_SET_INTENSITY_FORCED_AMBER_2 _IOW(FL_MAGIC_NUMBER, 0x0c, int)
#define FL_IOCTL_SET_INTENSITY_AMBER          _IOW(FL_MAGIC_NUMBER, 0x0d, int)
#define FL_IOCTL_GET_INTENSITY_AMBER          _IOR(FL_MAGIC_NUMBER, 0x0e, int)
#define FL_IOCTL_GET_RANGE_MAX_AMBER          _IOR(FL_MAGIC_NUMBER, 0x0f, int)
#define FL_IOCTL_SET_INTENSITY_FORCED_AMBER   _IOW(FL_MAGIC_NUMBER, 0x10, int)

#define WARIO_FL_LEVEL0              0
#define WARIO_FL_LEVEL12_MID         512
#define DUET_FL_LEVEL12_MID          240
#define WARIO_FL_LO_TRANSITION_LEVEL 42
#define WARIO_FL_LO_GRP_HOP_LEVEL    1
#define WARIO_FL_MED_GRP_HOP_LEVEL   10
#define WARIO_FL_LO_GRP_DELAY_US     1000

#endif
