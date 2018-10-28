#!/usr/bin/env python3
# -*- coding:utf-8 -*-
#
# Quick'n dirty script to generate the Kobo PCB to Product LUT
# c.f., /bin/kobo_config.sh
#
##

import sys

data = "./Kobo_PCB_IDs.txt"

with open(data, "r") as f:
	for line in f:
		if line.startswith("E60610"):
			print("TRILOGY,")
		elif line.startswith("E60QB") or line.startswith("E606B"):
			print("KRAKEN,")
		elif line.startswith("E5061"):
			print("PIXIE,")
		elif line.startswith("E60Q9"):
			print("PIKA_ALYSSUM,")
		elif line.startswith("E606C"):
			print("DRAGON,")
		elif line.startswith("E606G"):
			print("DAHLIA,")
		elif line.startswith("E606F"):
			print("PHOENIX,")
		elif line.startswith("E70Q0"):
			print("DAYLIGHT,")
		elif line.startswith("E60K0") or line.startswith("E60U1"):
			print("NOVA,")
		elif line.startswith("E60QL") or line.startswith("E60U0") or line.startswith("T60Q0"):
			print("STAR,")
		elif line.startswith("E60QM"):
			print("SNOW,")
		elif line.startswith("E80K0"):
			print("FROST,")
		else:
			print("UNKNOWN,")
