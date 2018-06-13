#!/usr/bin/env python3
# -*- coding:utf-8 -*-
#
# Very naive script to build a C array out of Unifont's hex format.
# Assumes 8x8 glyphs
# Tested on Unscii (http://pelulamu.net/unscii/)
# And it "works", but, glyphs are rendered vertically mirrored :?
# Or mirrored on the horizontal axis if I reverse their order (i.e., group 9 to 2 instead of 2 to 9) :?
#
##

import re

blocknum = 0
blockcount = 1
cp = 0x0
prevcp = 0x0
fmt = re.compile(r"([0-9a-fA-F]{4}):([0-9a-fA-F]{2})([0-9a-fA-F]{2})([0-9a-fA-F]{2})([0-9a-fA-F]{2})([0-9a-fA-F]{2})([0-9a-fA-F]{2})([0-9a-fA-F]{2})([0-9a-fA-F]{2})")
with open("unscii-8.hex", "r") as f:
	for line in f:
		m = fmt.match(line)
		if m:
			cp = m.group(1)
			if int(cp, base=16) == prevcp + 0x1:
				blockcount += 1
			else:
				if blocknum > 0:
					print("}}; // {}".format(blockcount))
				blocknum += 1
				blockcount = 1
				print("char unscii_block{}[][8] = {{".format(blocknum))

			print("\t{{ {:#04x}, {:#04x}, {:#04x}, {:#04x}, {:#04x}, {:#04x}, {:#04x}, {:#04x} }},\t// U+{}".format(int(m.group(2), base=16), int(m.group(3), base=16), int(m.group(4), base=16), int(m.group(5), base=16), int(m.group(6), base=16), int(m.group(7), base=16), int(m.group(8), base=16), int(m.group(9), base=16), cp))
			prevcp = int(cp, base=16)
print("}}; // {}".format(blockcount))
