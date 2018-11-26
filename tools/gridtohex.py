#!/usr/bin/env python3
# -*- coding:utf-8 -*-
#
# Quick'n dirty script to convert an ASCII-art grid into Unifont's hex format
# Assumes a 32x32 grid
#
##

import re

# Convert a binary representation (in string form) to hex
# c.f., https://stackoverflow.com/q/2072351
def bin2hex(s):
	return '%0*X' % ((len(s) + 3) // 4, int(s, 2))

fontfile = "../fonts/block.txt"
fontname = "block"

cp = 0x0
prevcp = 0x0
fmt = re.compile(r"([\.#]{32})")
cpre = re.compile(r"(U\+)([0-9a-fA-F]{4})(.*?)")

with open(fontfile, "r") as f:
	for line in f:
		m = cpre.match(line)
		if m:
			cp = m.group(2)
			print("")
			print("{}:".format(cp.upper()), end='')
		else:
			m = fmt.match(line)
			if m:
				row = m.group(1)
				# ASCII art to binary
				row = row.replace('.', '0')
				row = row.replace('#', '1')
				print("{}".format(bin2hex(row)), end='')
print("")
