/*
 * Copyright (c) 2021 Amazon.com, Inc. or its affiliates. All Rights Reserved.
 *
 * ----
 *
 * This is <linux/frontlight.h>, last updated from the KS kernel for FW 5.16.1
 *
 * NOTE: Upstream kernels available here: https://www.amazon.com/gp/help/customer/display.html?nodeId=200203720
 *
 * ----
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * version 2, and only version 2, as published by the
 * Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#ifndef _LINUX_LAB126_STYLUS_H
#define _LINUX_LAB126_STYLUS_H

#define STYLUS_MISC_DEV_NAME "stylus"
#define PATH_DEV_STYLUS      "/dev/" STYLUS_MISC_DEV_NAME

#define STYLUS_MAGIC_NUMBER   'S'
// Standard API
#define STYLUS_IOCTL_GET_LOCK _IOR(STYLUS_MAGIC_NUMBER, 0x01, int)
#define STYLUS_IOCTL_SET_LOCK _IOW(STYLUS_MAGIC_NUMBER, 0x02, int)

#endif /* _LINUX_LAB126_STYLUS_H */
