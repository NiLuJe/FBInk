#!/usr/bin/env python3
# -*- coding:utf-8 -*-
#
# Very naive script to build a C array out of Unifont's hex format.
# Assumes 8x8 or 8x16 glyphs
# Tested on Unscii & its fun variants (http://pelulamu.net/unscii/)
# NOTE: I've yet to get something working out of BDF fonts, either via bdfe, or Unifont's bdfimplode/unibdf2hex...
#
##

import re
import sys

# We'll send the C header to stdout, but our C if/else snippet to stderr... ;).
# And then use your shell to handle the I/O redirections, because I'm lazy :D.
# ./hextoc.py > ../unscii.h 2>> ../fbink_unscii.c
def eprint(*args, **kwargs):
	print(*args, file=sys.stderr, **kwargs)

# NOTE: Turns out Unifont's hex format is *slightly* different from the one used by font8x8...
# I couldn't actually find a human-readable documentation of Unifont's format (http://czyborra.com/unifont/),
# but there is one for font8x8 (in its README).
# Thanks to this side of the documentation, what happened when we fed Unifont's format to the font8x8 renderer
# turned out to start making some sense...
# Long story short: we render glyphs *vertically* mirrored when doing nothing, so the issue lies in the row encoding...
# Turns out, it's as "simple" as doing a bit reversal on each byte (as in 8 bits), so that we get the "mirror" bitmask.
# And boom, magic.
# "Magic" was helped by a lucky Google search, which led me first to: https://stackoverflow.com/q/4245936
# And in turn to https://stackoverflow.com/q/12681945 which has a very Pythonic solution,
# and links to http://graphics.stanford.edu/~seander/bithacks.html#BitReverseObvious for shit'n giggles.
# Which, believe it or not, I already had bookmarked for some mysterious reason? (I'm going to guess KindleTool related).
def hex2f8(v):
	h = int(v, base=16)
	# Pythonic approach, which may not require 64bits maths.
	#return int('{:08b}'.format(h)[::-1], 2)
	# Vintage 1972 magic trick ;)
	return ((h * 0x0202020202 & 0x010884422010) % 1023)

fontheight = 8
fontfile = "../fonts/unscii-8.hex"
fontname = "unscii"

print("/*")
print("* C Header for use with https://github.com/NiLuJe/FBInk")
print("* Converted from Hex font {}".format(fontfile))
print("* With FBInk's tools/hextoc.py")
print("*/")
print("")

blocknum = 0
blockcount = 1
blockcp = 0x0
cp = 0x0
prevcp = 0x0
if fontheight == 16:
	fmt = re.compile(r"([0-9a-fA-F]{4}):([0-9a-fA-F]{2})([0-9a-fA-F]{2})([0-9a-fA-F]{2})([0-9a-fA-F]{2})([0-9a-fA-F]{2})([0-9a-fA-F]{2})([0-9a-fA-F]{2})([0-9a-fA-F]{2})([0-9a-fA-F]{2})([0-9a-fA-F]{2})([0-9a-fA-F]{2})([0-9a-fA-F]{2})([0-9a-fA-F]{2})([0-9a-fA-F]{2})([0-9a-fA-F]{2})([0-9a-fA-F]{2})")
elif fontheight == 8:
	fmt = re.compile(r"([0-9a-fA-F]{4}):([0-9a-fA-F]{2})([0-9a-fA-F]{2})([0-9a-fA-F]{2})([0-9a-fA-F]{2})([0-9a-fA-F]{2})([0-9a-fA-F]{2})([0-9a-fA-F]{2})([0-9a-fA-F]{2})")
else:
	print("Unsupported font height!")
	exit(-1)

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
					print("")
					if blocknum == 1:
						eprint("static const char*")
						eprint("    {}_get_bitmap(uint32_t codepoint)".format(fontname))
						eprint("{")
						eprint("\tif (codepoint <= {:#04x}) {{".format(prevcp))
						eprint("\t\treturn {}_block{}[codepoint];".format(fontname, blocknum))
					else:
						eprint("\t}} else if (codepoint >= {:#04x} && codepoint <= {:#04x}) {{".format(int(blockcp, base=16), prevcp))
						eprint("\t\treturn {}_block{}[codepoint - {:#04x}];".format(fontname, blocknum, int(blockcp, base=16)))
				blocknum += 1
				blockcount = 1
				blockcp = cp
				print("static const char {}_block{}[][{}] = {{".format(fontname, blocknum, fontheight))

			hcp = int(cp, base=16)
			if fontheight == 16:
				print(u"\t{{ {:#04x}, {:#04x}, {:#04x}, {:#04x}, {:#04x}, {:#04x}, {:#04x}, {:#04x}, {:#04x}, {:#04x}, {:#04x}, {:#04x}, {:#04x}, {:#04x}, {:#04x}, {:#04x} }},\t// U+{} ({})".format(hex2f8(m.group(2)), hex2f8(m.group(3)), hex2f8(m.group(4)), hex2f8(m.group(5)), hex2f8(m.group(6)), hex2f8(m.group(7)), hex2f8(m.group(8)), hex2f8(m.group(9)), hex2f8(m.group(10)), hex2f8(m.group(11)), hex2f8(m.group(12)), hex2f8(m.group(13)), hex2f8(m.group(14)), hex2f8(m.group(15)), hex2f8(m.group(16)), hex2f8(m.group(17)), cp, chr(hcp) if hcp >= 0x20 else "ESC"))
			elif fontheight == 8:
				print(u"\t{{ {:#04x}, {:#04x}, {:#04x}, {:#04x}, {:#04x}, {:#04x}, {:#04x}, {:#04x} }},\t// U+{} ({})".format(hex2f8(m.group(2)), hex2f8(m.group(3)), hex2f8(m.group(4)), hex2f8(m.group(5)), hex2f8(m.group(6)), hex2f8(m.group(7)), hex2f8(m.group(8)), hex2f8(m.group(9)), cp, chr(hcp) if hcp >= 0x20 else "ESC"))
			else:
				exit(-1)
			prevcp = int(cp, base=16)
print("}}; // {}".format(blockcount))
print("")

eprint("\t}} else {{".format(int(blockcp, base=16), prevcp))
eprint('\t\tfprintf(stderr, "[FBInk] Codepoint U+%04X is not covered by our font!\\n", codepoint);')
eprint("\t\treturn {}_block1[0];".format(fontname))
eprint("\t}")
eprint("}")
eprint("")
