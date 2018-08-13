#!/usr/bin/env python3
# -*- coding:utf-8 -*-
#
# Very naive script to build a C array out of Unifont's hex format.
# Assumes 8x8 or 8x16 glyphs
# Tested on Unscii & its fun variants (http://pelulamu.net/unscii/)
# NOTE: You can probably get something working out of BDF fonts, either via bdfe, or Unifont's bdfimplode/unibdf2hex,
#       but if the horizontal resolution is different than 8, that implies code tweaks to handle it right.
#       Right now, 8 means we store an array of uint8_t, a 16xN font would need an array of uint16_t,
#       a 32xN one an array of uint32_t and a 64xN one an array of uint64_t ;).
#
##

import re
import sys

# We'll send the C header to stdout, but our C if/else snippet to stderr... ;).
# And then use your shell to handle the I/O redirections, because I'm lazy :D.
# ./hextoc.py > ../fonts/unscii.h 2>> ../fbink_unscii.c
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

# NOTE: c.f., https://stackoverflow.com/q/5333509/
def hex2f16(v):
	h = int(v, base=16)
	return int(bin(h)[2:].zfill(16)[::-1], 2)

def hex2f32(v):
	h = int(v, base=16)
	return int(bin(h)[2:].zfill(32)[::-1], 2)

fontwidth = 32
fontheight = 32
fontfile = "../fonts/block.hex"
fontname = "block"

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
if fontheight == 32:
	fmt = re.compile(r"([0-9a-fA-F]{{4}}):([0-9a-fA-F]{{{n}}})([0-9a-fA-F]{{{n}}})([0-9a-fA-F]{{{n}}})([0-9a-fA-F]{{{n}}})([0-9a-fA-F]{{{n}}})([0-9a-fA-F]{{{n}}})([0-9a-fA-F]{{{n}}})([0-9a-fA-F]{{{n}}})([0-9a-fA-F]{{{n}}})([0-9a-fA-F]{{{n}}})([0-9a-fA-F]{{{n}}})([0-9a-fA-F]{{{n}}})([0-9a-fA-F]{{{n}}})([0-9a-fA-F]{{{n}}})([0-9a-fA-F]{{{n}}})([0-9a-fA-F]{{{n}}})([0-9a-fA-F]{{{n}}})([0-9a-fA-F]{{{n}}})([0-9a-fA-F]{{{n}}})([0-9a-fA-F]{{{n}}})([0-9a-fA-F]{{{n}}})([0-9a-fA-F]{{{n}}})([0-9a-fA-F]{{{n}}})([0-9a-fA-F]{{{n}}})([0-9a-fA-F]{{{n}}})([0-9a-fA-F]{{{n}}})([0-9a-fA-F]{{{n}}})([0-9a-fA-F]{{{n}}})([0-9a-fA-F]{{{n}}})([0-9a-fA-F]{{{n}}})([0-9a-fA-F]{{{n}}})([0-9a-fA-F]{{{n}}})".format(n=int(fontwidth/4)))
elif fontheight == 16:
	fmt = re.compile(r"([0-9a-fA-F]{{4}}):([0-9a-fA-F]{{{n}}})([0-9a-fA-F]{{{n}}})([0-9a-fA-F]{{{n}}})([0-9a-fA-F]{{{n}}})([0-9a-fA-F]{{{n}}})([0-9a-fA-F]{{{n}}})([0-9a-fA-F]{{{n}}})([0-9a-fA-F]{{{n}}})([0-9a-fA-F]{{{n}}})([0-9a-fA-F]{{{n}}})([0-9a-fA-F]{{{n}}})([0-9a-fA-F]{{{n}}})([0-9a-fA-F]{{{n}}})([0-9a-fA-F]{{{n}}})([0-9a-fA-F]{{{n}}})([0-9a-fA-F]{{{n}}})".format(n=int(fontwidth/4)))
elif fontheight == 8:
	fmt = re.compile(r"([0-9a-fA-F]{{4}}):([0-9a-fA-F]{{{n}}})([0-9a-fA-F]{{{n}}})([0-9a-fA-F]{{{n}}})([0-9a-fA-F]{{{n}}})([0-9a-fA-F]{{{n}}})([0-9a-fA-F]{{{n}}})([0-9a-fA-F]{{{n}}})([0-9a-fA-F]{{{n}}})".format(n=int(fontwidth/4)))
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
						if fontwidth == 32:
							eprint("static const uint32_t*")
						elif fontwidth == 16:
							eprint("static const uint16_t*")
						else:
							eprint("static const unsigned char*")
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
				if fontwidth == 32:
					print("static const uint32_t {}_block{}[][{}] = {{".format(fontname, blocknum, fontheight))
				elif fontwidth == 16:
					print("static const uint16_t {}_block{}[][{}] = {{".format(fontname, blocknum, fontheight))
				else:
					print("static const unsigned char {}_block{}[][{}] = {{".format(fontname, blocknum, fontheight))

			hcp = int(cp, base=16)
			if fontwidth == 8:
				if fontheight == 32:
					print(u"\t{{ {:#04x}, {:#04x}, {:#04x}, {:#04x}, {:#04x}, {:#04x}, {:#04x}, {:#04x}, {:#04x}, {:#04x}, {:#04x}, {:#04x}, {:#04x}, {:#04x}, {:#04x}, {:#04x}, {:#04x}, {:#04x}, {:#04x}, {:#04x}, {:#04x}, {:#04x}, {:#04x}, {:#04x}, {:#04x}, {:#04x}, {:#04x}, {:#04x}, {:#04x}, {:#04x}, {:#04x}, {:#04x} }},\t// U+{} ({})".format(hex2f8(m.group(2)), hex2f8(m.group(3)), hex2f8(m.group(4)), hex2f8(m.group(5)), hex2f8(m.group(6)), hex2f8(m.group(7)), hex2f8(m.group(8)), hex2f8(m.group(9)), hex2f8(m.group(10)), hex2f8(m.group(11)), hex2f8(m.group(12)), hex2f8(m.group(13)), hex2f8(m.group(14)), hex2f8(m.group(15)), hex2f8(m.group(16)), hex2f8(m.group(17)), hex2f8(m.group(18)), hex2f8(m.group(19)), hex2f8(m.group(20)), hex2f8(m.group(21)), hex2f8(m.group(22)), hex2f8(m.group(23)), hex2f8(m.group(24)), hex2f8(m.group(25)), hex2f8(m.group(26)), hex2f8(m.group(27)), hex2f8(m.group(28)), hex2f8(m.group(29)), hex2f8(m.group(30)), hex2f8(m.group(31)), hex2f8(m.group(32)), hex2f8(m.group(33)), cp, chr(hcp) if hcp >= 0x20 else "ESC"))
				elif fontheight == 16:
					print(u"\t{{ {:#04x}, {:#04x}, {:#04x}, {:#04x}, {:#04x}, {:#04x}, {:#04x}, {:#04x}, {:#04x}, {:#04x}, {:#04x}, {:#04x}, {:#04x}, {:#04x}, {:#04x}, {:#04x} }},\t// U+{} ({})".format(hex2f8(m.group(2)), hex2f8(m.group(3)), hex2f8(m.group(4)), hex2f8(m.group(5)), hex2f8(m.group(6)), hex2f8(m.group(7)), hex2f8(m.group(8)), hex2f8(m.group(9)), hex2f8(m.group(10)), hex2f8(m.group(11)), hex2f8(m.group(12)), hex2f8(m.group(13)), hex2f8(m.group(14)), hex2f8(m.group(15)), hex2f8(m.group(16)), hex2f8(m.group(17)), cp, chr(hcp) if hcp >= 0x20 else "ESC"))
				elif fontheight == 8:
					print(u"\t{{ {:#04x}, {:#04x}, {:#04x}, {:#04x}, {:#04x}, {:#04x}, {:#04x}, {:#04x} }},\t// U+{} ({})".format(hex2f8(m.group(2)), hex2f8(m.group(3)), hex2f8(m.group(4)), hex2f8(m.group(5)), hex2f8(m.group(6)), hex2f8(m.group(7)), hex2f8(m.group(8)), hex2f8(m.group(9)), cp, chr(hcp) if hcp >= 0x20 else "ESC"))
				else:
					exit(-1)
			elif fontwidth == 16:
				if fontheight == 32:
					print(u"\t{{ {:#06x}, {:#06x}, {:#06x}, {:#06x}, {:#06x}, {:#06x}, {:#06x}, {:#06x}, {:#06x}, {:#06x}, {:#06x}, {:#06x}, {:#06x}, {:#06x}, {:#06x}, {:#06x}, {:#06x}, {:#06x}, {:#06x}, {:#06x}, {:#06x}, {:#06x}, {:#06x}, {:#06x}, {:#06x}, {:#06x}, {:#06x}, {:#06x}, {:#06x}, {:#06x}, {:#06x}, {:#06x} }},\t// U+{} ({})".format(hex2f16(m.group(2)), hex2f16(m.group(3)), hex2f16(m.group(4)), hex2f16(m.group(5)), hex2f16(m.group(6)), hex2f16(m.group(7)), hex2f16(m.group(8)), hex2f16(m.group(9)), hex2f16(m.group(10)), hex2f16(m.group(11)), hex2f16(m.group(12)), hex2f16(m.group(13)), hex2f16(m.group(14)), hex2f16(m.group(15)), hex2f16(m.group(16)), hex2f16(m.group(17)), hex2f16(m.group(18)), hex2f16(m.group(19)), hex2f16(m.group(20)), hex2f16(m.group(21)), hex2f16(m.group(22)), hex2f16(m.group(23)), hex2f16(m.group(24)), hex2f16(m.group(25)), hex2f16(m.group(26)), hex2f16(m.group(27)), hex2f16(m.group(28)), hex2f16(m.group(29)), hex2f16(m.group(30)), hex2f16(m.group(31)), hex2f16(m.group(32)), hex2f16(m.group(33)), cp, chr(hcp) if hcp >= 0x20 else "ESC"))
				elif fontheight == 16:
					print(u"\t{{ {:#06x}, {:#06x}, {:#06x}, {:#06x}, {:#06x}, {:#06x}, {:#06x}, {:#06x}, {:#06x}, {:#06x}, {:#06x}, {:#06x}, {:#06x}, {:#06x}, {:#06x}, {:#06x} }},\t// U+{} ({})".format(hex2f16(m.group(2)), hex2f16(m.group(3)), hex2f16(m.group(4)), hex2f16(m.group(5)), hex2f16(m.group(6)), hex2f16(m.group(7)), hex2f16(m.group(8)), hex2f16(m.group(9)), hex2f16(m.group(10)), hex2f16(m.group(11)), hex2f16(m.group(12)), hex2f16(m.group(13)), hex2f16(m.group(14)), hex2f16(m.group(15)), hex2f16(m.group(16)), hex2f16(m.group(17)), cp, chr(hcp) if hcp >= 0x20 else "ESC"))
				elif fontheight == 8:
					print(u"\t{{ {:#06x}, {:#06x}, {:#06x}, {:#06x}, {:#06x}, {:#06x}, {:#06x}, {:#06x} }},\t// U+{} ({})".format(hex2f16(m.group(2)), hex2f16(m.group(3)), hex2f16(m.group(4)), hex2f16(m.group(5)), hex2f16(m.group(6)), hex2f16(m.group(7)), hex2f16(m.group(8)), hex2f16(m.group(9)), cp, chr(hcp) if hcp >= 0x20 else "ESC"))
				else:
					exit(-1)
			elif fontwidth == 32:
				if fontheight == 32:
					print(u"\t{{ {:#010x}, {:#010x}, {:#010x}, {:#010x}, {:#010x}, {:#010x}, {:#010x}, {:#010x}, {:#010x}, {:#010x}, {:#010x}, {:#010x}, {:#010x}, {:#010x}, {:#010x}, {:#010x}, {:#010x}, {:#010x}, {:#010x}, {:#010x}, {:#010x}, {:#010x}, {:#010x}, {:#010x}, {:#010x}, {:#010x}, {:#010x}, {:#010x}, {:#010x}, {:#010x}, {:#010x}, {:#010x} }},\t// U+{} ({})".format(hex2f32(m.group(2)), hex2f32(m.group(3)), hex2f32(m.group(4)), hex2f32(m.group(5)), hex2f32(m.group(6)), hex2f32(m.group(7)), hex2f32(m.group(8)), hex2f32(m.group(9)), hex2f32(m.group(10)), hex2f32(m.group(11)), hex2f32(m.group(12)), hex2f32(m.group(13)), hex2f32(m.group(14)), hex2f32(m.group(15)), hex2f32(m.group(16)), hex2f32(m.group(17)), hex2f32(m.group(18)), hex2f32(m.group(19)), hex2f32(m.group(20)), hex2f32(m.group(21)), hex2f32(m.group(22)), hex2f32(m.group(23)), hex2f32(m.group(24)), hex2f32(m.group(25)), hex2f32(m.group(26)), hex2f32(m.group(27)), hex2f32(m.group(28)), hex2f32(m.group(29)), hex2f32(m.group(30)), hex2f32(m.group(31)), hex2f32(m.group(32)), hex2f32(m.group(33)), cp, chr(hcp) if hcp >= 0x20 else "ESC"))
				elif fontheight == 16:
					print(u"\t{{ {:#010x}, {:#010x}, {:#010x}, {:#010x}, {:#010x}, {:#010x}, {:#010x}, {:#010x}, {:#010x}, {:#010x}, {:#010x}, {:#010x}, {:#010x}, {:#010x}, {:#010x}, {:#010x} }},\t// U+{} ({})".format(hex2f32(m.group(2)), hex2f32(m.group(3)), hex2f32(m.group(4)), hex2f32(m.group(5)), hex2f32(m.group(6)), hex2f32(m.group(7)), hex2f32(m.group(8)), hex2f32(m.group(9)), hex2f32(m.group(10)), hex2f32(m.group(11)), hex2f32(m.group(12)), hex2f32(m.group(13)), hex2f32(m.group(14)), hex2f32(m.group(15)), hex2f32(m.group(16)), hex2f32(m.group(17)), cp, chr(hcp) if hcp >= 0x20 else "ESC"))
				elif fontheight == 8:
					print(u"\t{{ {:#010x}, {:#010x}, {:#010x}, {:#010x}, {:#010x}, {:#010x}, {:#010x}, {:#010x} }},\t// U+{} ({})".format(hex2f32(m.group(2)), hex2f32(m.group(3)), hex2f32(m.group(4)), hex2f32(m.group(5)), hex2f32(m.group(6)), hex2f32(m.group(7)), hex2f32(m.group(8)), hex2f32(m.group(9)), cp, chr(hcp) if hcp >= 0x20 else "ESC"))
				else:
					exit(-1)
			else:
				exit(-1)
			prevcp = int(cp, base=16)
print("}}; // {}".format(blockcount))
print("")

# Handle single block fonts, even when they don't start at codepoint U+0000
if blocknum == 1:
	if fontwidth == 32:
		eprint("static const uint32_t*")
	elif fontwidth == 16:
		eprint("static const uint16_t*")
	else:
		eprint("static const unsigned char*")
	eprint("    {}_get_bitmap(uint32_t codepoint)".format(fontname))
	eprint("{")
	eprint("\tif (codepoint >= {:#04x} && codepoint <= {:#04x}) {{".format(int(blockcp, base=16), prevcp))
	eprint("\t\treturn {}_block{}[codepoint - {:#04x}];".format(fontname, blocknum, int(blockcp, base=16)))
else:
	# Don't forget the final block
	eprint("\t}} else if (codepoint >= {:#04x} && codepoint <= {:#04x}) {{".format(int(blockcp, base=16), prevcp))
	eprint("\t\treturn {}_block{}[codepoint - {:#04x}];".format(fontname, blocknum, int(blockcp, base=16)))

eprint("\t} else {")
eprint('\t\tfprintf(stderr, "[FBInk] Codepoint U+%04X is not covered by this font!\\n", codepoint);')
eprint("\t\treturn {}_block1[0];".format(fontname))
eprint("\t}")
eprint("}")
eprint("")
