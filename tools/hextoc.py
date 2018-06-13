#!/usr/bin/env python3
# -*- coding:utf-8 -*-
#
# Very naive script to build a C array out of Unifont's hex format.
# Assumes 8x8 glyphs
# Tested on Unscii & its fun variants (http://pelulamu.net/unscii/)
# And it "works", but, glyphs are rendered vertically mirrored :?
# Or mirrored on the horizontal axis if I parse the hex from RTL (i.e., group 9 to 2 instead of 2 to 9) :?
# Tweaking draw() a tiny bit yields acceptable results (c.f., the relevant comments there),
# but might be messing with kerning a tiny bit? ...
#
##

import re
import sys

# We'll send the C header to stdout, but our C if/else snippet to stderr... ;).
# And then use your shell to handle the I/O redirections, because I'm lazy :D.
# ./hextoc.py > unscii.h 2> unscii.c
def eprint(*args, **kwargs):
    print(*args, file=sys.stderr, **kwargs)

fontfile = "unscii-8.hex"
fontname = "unscii"

blocknum = 0
blockcount = 1
blockcp = 0x0
cp = 0x0
prevcp = 0x0
fmt = re.compile(r"([0-9a-fA-F]{4}):([0-9a-fA-F]{2})([0-9a-fA-F]{2})([0-9a-fA-F]{2})([0-9a-fA-F]{2})([0-9a-fA-F]{2})([0-9a-fA-F]{2})([0-9a-fA-F]{2})([0-9a-fA-F]{2})")
with open(fontfile, "r") as f:
	for line in f:
		m = fmt.match(line)
		if m:
			cp = m.group(1)
			if int(cp, base=16) == prevcp + 0x1:
				blockcount += 1
			else:
				if blocknum > 0:
					print("}}; // {}".format(blockcount))
					if blocknum == 1:
						eprint("\tif (codepoint <= {:#04x}) {{".format(prevcp))
						eprint("\t\treturn {}_block{}[codepoint];".format(fontname, blocknum))
					else:
						eprint("\t}} else if (codepoint >= {:#04x} && codepoint <= {:#04x}) {{".format(int(blockcp, base=16), prevcp))
						eprint("\t\treturn {}_block{}[codepoint - {:#04x}];".format(fontname, blocknum, int(blockcp, base=16)))
				blocknum += 1
				blockcount = 1
				blockcp = cp
				print("char {}_block{}[][8] = {{".format(fontname, blocknum))

			print("\t{{ {:#04x}, {:#04x}, {:#04x}, {:#04x}, {:#04x}, {:#04x}, {:#04x}, {:#04x} }},\t// U+{}".format(int(m.group(2), base=16), int(m.group(3), base=16), int(m.group(4), base=16), int(m.group(5), base=16), int(m.group(6), base=16), int(m.group(7), base=16), int(m.group(8), base=16), int(m.group(9), base=16), cp))
			prevcp = int(cp, base=16)
print("}}; // {}".format(blockcount))

eprint("\t}} else {{".format(int(blockcp, base=16), prevcp))
eprint("\t\treturn {}_block1[0];".format(fontname))
eprint("\t}")
