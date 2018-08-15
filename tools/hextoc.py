#!/usr/bin/env python3
# -*- coding:utf-8 -*-
#
# Very naive script to build a C array out of Unifont's hex format.
# Assumes 8x8 or 8x16 glyphs
# Tested on Unscii & its fun variants (http://pelulamu.net/unscii/)
# NOTE: You can probably get something working out of BDF fonts, either via gbdfed, bdfe, or Unifont's bdfimplode/unibdf2hex,
#       but if the horizontal resolution is > 8, that implies code tweaks to handle it right.
#       Right now, fontwidth <= 8 means we store an array of uint8_t, for a <= 16xN font an array of uint16_t,
#       a <= 32xN one an array of uint32_t, and a <= 64xN one an array of uint64_t ;).
#       This script currently handles <= 64, as does FBInk.
#       As for the conversion process itself, FontForge + gbdfed + a text editor should handle most common cases just fine ;).
#       In case that wasn't clear, width of intermediary values are supported, it's just a bit of a waste of memory ;).
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

def hex2f64(v):
	h = int(v, base=16)
	return int(bin(h)[2:].zfill(64)[::-1], 2)

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

pat_cp = "([0-9a-fA-F]{4})"
if fontwidth <= 8:
	pat_rows = "([0-9a-fA-F]{2})" * fontheight
elif fontwidth <= 16:
	pat_rows = "([0-9a-fA-F]{4})" * fontheight
elif fontwidth <= 32:
	pat_rows = "([0-9a-fA-F]{8})" * fontheight
elif fontwidth <= 64:
	pat_rows = "([0-9a-fA-F]{16})" * fontheight
else:
	print("Unsupported font width (Must be <= 64)!")
	exit(-1)
fmt = re.compile(r"{}:{}".format(pat_cp, pat_rows))

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
						if fontwidth <= 8:
							eprint("static const unsigned char*")
						elif fontwidth <= 16:
							eprint("static const uint16_t*")
						elif fontwidth <= 32:
							eprint("static const uint32_t*")
						elif fontwidth <= 64:
							eprint("static const uint64_t*")
						else:
							exit(-1)
						eprint("    {}_get_bitmap(uint32_t codepoint)".format(fontname))
						eprint("{")
						if int(blockcp, base=16) > 0:
							eprint("\tif (codepoint >= {:#04x} && codepoint <= {:#04x}) {{".format(int(blockcp, base=16), prevcp))
							eprint("\t\treturn {}_block{}[codepoint - {:#04x}];".format(fontname, blocknum, int(blockcp, base=16)))
						else:
							eprint("\tif (codepoint <= {:#04x}) {{".format(prevcp))
							eprint("\t\treturn {}_block{}[codepoint];".format(fontname, blocknum))
					else:
						eprint("\t}} else if (codepoint >= {:#04x} && codepoint <= {:#04x}) {{".format(int(blockcp, base=16), prevcp))
						eprint("\t\treturn {}_block{}[codepoint - {:#04x}];".format(fontname, blocknum, int(blockcp, base=16)))
				blocknum += 1
				blockcount = 1
				blockcp = cp
				if fontwidth <= 8:
					print("static const unsigned char {}_block{}[][{}] = {{".format(fontname, blocknum, fontheight))
				elif fontwidth <= 16:
					print("static const uint16_t {}_block{}[][{}] = {{".format(fontname, blocknum, fontheight))
				elif fontwidth <= 32:
					print("static const uint32_t {}_block{}[][{}] = {{".format(fontname, blocknum, fontheight))
				elif fontwidth <= 64:
					print("static const uint64_t {}_block{}[][{}] = {{".format(fontname, blocknum, fontheight))
				else:
					exit(-1)

			hcp = int(cp, base=16)
			print(u"\t{", end='')
			if fontwidth <= 8:
				for i in range(fontheight):
					print(u" {:#04x}".format(hex2f8(m.group(i+2))), end='' if i+1 == fontheight else ',')
			elif fontwidth <= 16:
				for i in range(fontheight):
					print(u" {:#06x}".format(hex2f16(m.group(i+2))), end='' if i+1 == fontheight else ',')
			elif fontwidth <= 32:
				for i in range(fontheight):
					print(u" {:#010x}".format(hex2f32(m.group(i+2))), end='' if i+1 == fontheight else ',')
			elif fontwidth <= 64:
				for i in range(fontheight):
					print(u" {:#018x}".format(hex2f64(m.group(i+2))), end='' if i+1 == fontheight else ',')
			else:
				exit(-1)
			print(u" }},\t// U+{} ({})".format(cp, chr(hcp) if hcp >= 0x20 else "ESC"))
			prevcp = int(cp, base=16)
print("}}; // {}".format(blockcount))
print("")

# Handle single block fonts
if blocknum == 1:
	if fontwidth <= 8:
		eprint("static const unsigned char*")
	elif fontwidth <= 16:
		eprint("static const uint16_t*")
	elif fontwidth <= 32:
		eprint("static const uint32_t*")
	elif fontwidth <= 64:
		eprint("static const uint64_t*")
	else:
		exit(-1)
	eprint("    {}_get_bitmap(uint32_t codepoint)".format(fontname))
	eprint("{")
	if int(blockcp, base=16) > 0:
		eprint("\tif (codepoint >= {:#04x} && codepoint <= {:#04x}) {{".format(int(blockcp, base=16), prevcp))
		eprint("\t\treturn {}_block{}[codepoint - {:#04x}];".format(fontname, blocknum, int(blockcp, base=16)))
	else:
		eprint("\tif (codepoint <= {:#04x}) {{".format(prevcp))
		eprint("\t\treturn {}_block{}[codepoint];".format(fontname, blocknum))
else:
	# Otherwise, don't forget the final block ;)
	eprint("\t}} else if (codepoint >= {:#04x} && codepoint <= {:#04x}) {{".format(int(blockcp, base=16), prevcp))
	eprint("\t\treturn {}_block{}[codepoint - {:#04x}];".format(fontname, blocknum, int(blockcp, base=16)))

eprint("\t} else {")
eprint('\t\tfprintf(stderr, "[FBInk] Codepoint U+%04X is not covered by this font!\\n", codepoint);')
eprint("\t\treturn {}_block1[0];".format(fontname))
eprint("\t}")
eprint("}")
eprint("")
