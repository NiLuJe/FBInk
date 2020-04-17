#!/usr/bin/env python3
# -*- coding:utf-8 -*-
#
# FBInk related tool, Copyright (C) 2018-2020 NiLuJe <ninuje@gmail.com>
# SPDX-License-Identifier: GPL-3.0-or-later
#
# Somewhat naive script to build a C array out of Unifont's hex format.
# Tested on Unscii & its fun variants (http://pelulamu.net/unscii/)
# NOTE: You can probably get something working out of BDF fonts, either via gbdfed, bdfe, or Unifont's bdfimplode/unibdf2hex,
#       (in order of preference), but if the horizontal resolution is > 8, that may imply code tweaks to handle it right.
#       Right now, fontwidth <= 8 means we store an array of uint8_t, for a <= 16xN font an array of uint16_t,
#       a <= 32xN one an array of uint32_t, and a <= 64xN one an array of uint64_t ;).
#       This script currently handles <= 64, as does FBInk (although, on FBInk's side,
#       the uint64_t codepath is currently commented out, since it's unneeded with our current font panel).
#       As for the conversion process itself, FontForge + gbdfed + a text editor should handle most common cases just fine ;).
#       In case that wasn't clear, width of intermediary values are supported, it's just a bit of a waste of memory ;).
#
##

import os
import re
import sys

# We'll send the C header to stdout, but our C if/else snippet to stderr... ;).
# And then use your shell to handle the I/O redirections, because I'm lazy :D.
# ./hextoc.py unscii > ../fonts/unscii.h 2>> ../fbink_unscii.c
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

# NOTE: Unifont is not really monospaced. We ideally ought to honor per-glyph metrics,
#       but since we can't really do that, we simply seperate single-wide from double-wide glyphs.
#       zcat unifont-13.0.01.hex.gz | grep -E '^([[:xdigit:]]{4}:)([[:xdigit:]]{32})$' >| unifont-8x16.hex
#       zcat unifont-13.0.01.hex.gz | grep -E '^([[:xdigit:]]{4}:)([[:xdigit:]]{64})$' >| unifont-16x16.hex

# NOTE: Cozette should be converted to HEX via unibdf2hex, as gbdfed mangles it to oblivion...
#       The main culprit appears to be the per-glyph advances,
#       which means that the Vector variant will yield far tighter inter-glyph spacing.
# NOTE: The same idea applies to Scientifica Italic, where the Vector variant will yield far more pleasing results,
#       thanks to its aggressive use of negative horizontal advances.

# This is the list of fonts we currently process
font_set = {
	# name		: (file,			w,	h)
	"unscii"	: ("unscii-8.hex",		8,	8),
	"alt"		: ("unscii-8-alt.hex",		8,	8),
	"thin"		: ("unscii-8-thin.hex",		8,	8),
	"fantasy"	: ("unscii-8-fantasy.hex",	8,	8),
	"mcr"		: ("unscii-8-mcr.hex",		8,	8),
	"tall"		: ("unscii-16.hex",		8,	16),
	"block"		: ("block.hex",			32,	32),
	"leggie"	: ("leggie-8x18.hex",		8,	18),
	"veggie"	: ("leggie-8x16v.hex",		8,	16),
	"kates"		: ("kates-7x15.hex",		7,	15),
	"fkp"		: ("fkp-8x16.hex",		8,	16),
	"ctrld"		: ("ctrld-fixed-8x16r.hex",	8,	16),
	"orp"		: ("orp-6x12.hex",		6,	12),
	"orpb"		: ("orp-6x12b.hex",		6,	12),
	"orpi"		: ("orp-6x12i.hex",		6,	12),
	"scientifica"	: ("scientifica-5x12.hex",	5,	12),
	"scientificab"	: ("scientifica-5x12b.hex",	5,	12),
	"scientificai"	: ("scientifica-7x12i.hex",	7,	12),
	"terminus"	: ("ter-u16n.hex",		8,	16),
	"terminusb"	: ("ter-u16b.hex",		8,	16),
	"fatty"		: ("fatty7x16-iso10646-1.hex",	7,	16),
	"spleen"	: ("spleen-16x32.hex",		16,	32),
	"tewi"		: ("tewi-medium-11.hex",	6,	13),
	"tewib"		: ("tewi-bold-11.hex",		6,	13),
	"topaz"		: ("TopazPlus-8x16.hex",	8,	16),
	"microknight"	: ("MicroKnightPlus-8x16.hex",	8,	16),
	"vga"		: ("iv8x16u.hex",		8,	16),
	"unifont"	: ("unifont-8x16.hex",		8,	16),
	"unifontdw"	: ("unifont-16x16.hex",		16,	16),
	"cozette"       : ("cozette-8x13.hex",		8,	13),
}

# Get the font name from the first arg passed to the script
fontname = len(sys.argv) > 1 and sys.argv[1].lower() or "NULL"
if fontname not in font_set.keys():
	print("Unknown font name '{}'!".format(fontname))
	print("Available: {}".format(', '.join([k for k in font_set.keys()])))
	sys.exit(-1)

# And populate that specific font's settings
fontfile = os.path.dirname(os.path.abspath(__file__)) + "/../fonts/" + font_set[fontname][0]
fontwidth = font_set[fontname][1]
fontheight = font_set[fontname][2]

print("/*")
print("* C Header for use with https://github.com/NiLuJe/FBInk")
print("* Converted from Hex font {}".format(font_set[fontname][0]))
print("* With FBInk's tools/hextoc.py")
print("*/")
print("")
print("#pragma once")
print("")

blocknum = 0
blockcount = 1
blockcp = 0x0
cp = 0x0
prevcp = 0x0

# Cozette actually goes > 0xFFFF, so handle a proper uint32_t range.
pat_cp = "([0-9a-fA-F]{4,8})"
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
			hcp = int(cp, base=16)
			if hcp == prevcp + 0x1:
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
						if blockcp > 0 :
							if blockcp == prevcp:
								eprint("\tif (codepoint == {:#04x}u) {{".format(blockcp))
								eprint("\t\treturn {}_block{}[0];".format(fontname, blocknum))
							else:
								eprint("\tif (codepoint >= {:#04x}u && codepoint <= {:#04x}u) {{".format(blockcp, prevcp))
								eprint("\t\treturn {}_block{}[codepoint - {:#04x}u];".format(fontname, blocknum, blockcp))
						else:
							if prevcp == 0:
								eprint("\tif (codepoint == {:#04x}u) {{".format(prevcp))
							else:
								eprint("\tif (codepoint <= {:#04x}u) {{".format(prevcp))
							eprint("\t\treturn {}_block{}[codepoint];".format(fontname, blocknum))
					else:
						if blockcp == prevcp:
							eprint("\t}} else if (codepoint == {:#04x}u) {{".format(blockcp))
							eprint("\t\treturn {}_block{}[0];".format(fontname, blocknum))
						else:
							eprint("\t}} else if (codepoint >= {:#04x}u && codepoint <= {:#04x}u) {{".format(blockcp, prevcp))
							eprint("\t\treturn {}_block{}[codepoint - {:#04x}u];".format(fontname, blocknum, blockcp))
				blocknum += 1
				blockcount = 1
				blockcp = hcp
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
			prevcp = hcp
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
	if blockcp > 0:
		if blockcp == prevcp:
			eprint("\tif (codepoint == {:#04x}u) {{".format(blockcp))
			eprint("\t\treturn {}_block{}[0];".format(fontname, blocknum))
		else:
			eprint("\tif (codepoint >= {:#04x}u && codepoint <= {:#04x}u) {{".format(blockcp, prevcp))
			eprint("\t\treturn {}_block{}[codepoint - {:#04x}u];".format(fontname, blocknum, blockcp))
	else:
		if prevcp == 0:
			eprint("\tif (codepoint == {:#04x}u) {{".format(prevcp))
		else:
			eprint("\tif (codepoint <= {:#04x}u) {{".format(prevcp))
		eprint("\t\treturn {}_block{}[codepoint];".format(fontname, blocknum))
else:
	# Otherwise, don't forget the final block ;)
	if blockcp == prevcp:
		eprint("\t}} else if (codepoint == {:#04x}u) {{".format(blockcp))
		eprint("\t\treturn {}_block{}[0];".format(fontname, blocknum))
	else:
		eprint("\t}} else if (codepoint >= {:#04x}u && codepoint <= {:#04x}u) {{".format(blockcp, prevcp))
		eprint("\t\treturn {}_block{}[codepoint - {:#04x}u];".format(fontname, blocknum, blockcp))

eprint("\t} else {")
eprint('\t\tWARN("Codepoint U+%04X (%s) is not covered by this font", codepoint, u8_cp_to_utf8(codepoint));')
eprint("\t\treturn {}_block1[0];".format(fontname))
eprint("\t}")
eprint("}")
eprint("")
