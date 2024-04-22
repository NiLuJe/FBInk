#!/usr/bin/env python3
# -*- coding:utf-8 -*-
#
# FBInk related tool, Copyright (C) 2018-2024 NiLuJe <ninuje@gmail.com>
# SPDX-License-Identifier: GPL-3.0-or-later
#
# Y8 to RGB565 LUT
# c.f., pack_rgb565 @ fbink.c
#
##

def pack_rgb565(v):
	# Grayscale, so r = g = b = v
	return (((v >> 3) << 11) | ((v >> 2) << 5) | (v >> 3))

print("/*")
print("* C Header for use with https://github.com/NiLuJe/FBInk")
print("* c.f., FBInk's tools/rgb565_lut.py")
print("*/")
print("")

print("static const uint16_t y8ToRGB565[256] = { ")
for i in range(256):
	if i < 255:
		print("{:#06x}u, ".format(pack_rgb565(i)))
	else:
		print("{:#06x}u".format(pack_rgb565(i)))
print("};")
print("")
