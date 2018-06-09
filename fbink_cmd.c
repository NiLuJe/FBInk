/*
	FBInk: FrameBuffer eInker, a tool to print strings on eInk devices (Kobo/Kindle)
	Copyright (C) 2018 NiLuJe <ninuje@gmail.com>

	Linux framebuffer routines based on: fbtestfnt.c & fbtest6.c, from
	http://raspberrycompote.blogspot.com/2014/04/low-level-graphics-on-raspberry-pi-text.html &
	https://raspberrycompote.blogspot.com/2013/03/low-level-graphics-on-raspberry-pi-part_8.html
	Original works by J-P Rosti (a.k.a -rst- and 'Raspberry Compote'),
	Licensed under the Creative Commons Attribution 3.0 Unported License
	(http://creativecommons.org/licenses/by/3.0/deed.en_US)

	----

	This program is free software: you can redistribute it and/or modify
	it under the terms of the GNU Affero General Public License as
	published by the Free Software Foundation, either version 3 of the
	License, or (at your option) any later version.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU Affero General Public License for more details.

	You should have received a copy of the GNU Affero General Public License
	along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "fbink.h"

#include "fbink_cmd.h"

// Help message
static void
    show_helpmsg(void)
{
	printf(
	    "\n"
	    "FBInk %s\n"
	    "\n"
	    "Usage: fbink [OPTIONS] [STRING]\n"
	    "\n"
	    "Print STRING on your device's screen.\n"
	    "\n"
	    "Example: fbink -x 1 -y 10 \"Hello World!\"\n"
	    "\n"
	    "Options affecting the message's position on screen:\n"
	    "\t-x, --col NUM\tBegin printing STRING @ column NUM (Default: 0).\n"
	    "\t\t\tBeginning at column 1 instead of 0 is recommended, because column 0 is often half-obscured by a bezel.\n"
	    "\t-y, --row NUM\tBegin printing STRING @ row NUM (Default: 0).\n"
	    "\t\t\tBeginning at row 1 instead of 0 is recommended, because row 0 is often half-obscured by a bezel, especially on Kobos.\n"
	    "\t-m, --centered\tDynamically override col to print STRING at the center of the screen.\n"
	    "\t-p, --padded\tLeft pad STRING with blank spaces.\n"
	    "\t\t\tMost useful when combined with --centered to ensure a line will be completely filled, while still centering STRING.\n"
	    "\n"
	    "Options affecting the message's appearance:\n"
	    "\t-h, --invert\tPrint STRING in white over a black background instead of the reverse.\n"
	    "\t-f, --flash\tAsk the eInk driver to do a black flash when refreshing the area of the screen where STRING will be printed.\n"
	    "\t-c, --clear\tFully clear the screen before printing STRING (obeys --invert).\n"
	    "\n"
	    "NOTE:\n"
	    "\tYou can specify multiple STRINGs in a single invocation of fbink, each consecutive one will be printed on consecutive lines.\n"
	    "\t\tAlthough it's worth mentioning that this will lead to undesirable results with multi-line STRINGs,\n"
	    "\t\tas well as in combination with --clear, because the screen is cleared before each STRING, meaning you'll only get to see the final one.\n"
	    "\tIf you want to properly print a long string, better do it in a single argument, fbink will do its best to spread it over multiple lines sanely.\n"
	    "\n",
	    fbink_version());

	return;
}

// Application entry point
int
    main(int argc, char* argv[])
{
	int                        opt;
	int                        opt_index;
	static const struct option opts[] = {
		{ "row", required_argument, NULL, 'y' }, { "col", required_argument, NULL, 'x' },
		{ "invert", no_argument, NULL, 'h' },    { "flash", no_argument, NULL, 'f' },
		{ "clear", no_argument, NULL, 'c' },     { "centered", no_argument, NULL, 'm' },
		{ "padded", no_argument, NULL, 'p' },    { NULL, 0, NULL, 0 }
	};

	FBInkConfig fbink_config = { 0 };

	while ((opt = getopt_long(argc, argv, "y:x:hfcmp", opts, &opt_index)) != -1) {
		switch (opt) {
			case 'y':
				fbink_config.row = (short int) atoi(optarg);
				break;
			case 'x':
				fbink_config.col = (short int) atoi(optarg);
				break;
			case 'h':
				fbink_config.is_inverted = true;
				break;
			case 'f':
				fbink_config.is_flashing = true;
				break;
			case 'c':
				fbink_config.is_cleared = true;
				break;
			case 'm':
				fbink_config.is_centered = true;
				break;
			case 'p':
				fbink_config.is_padded = true;
				break;
			default:
				fprintf(stderr, "?? Unknown option code 0%o ??\n", opt);
				return EXIT_FAILURE;
				break;
		}
	}

	// Open framebuffer and keep it around, then setup globals.
	int fbfd = -1;
	if (-1 == (fbfd = fbink_open())) {
		fprintf(stderr, "Failed to open the framebuffer, aborting . . .\n");
		return EXIT_FAILURE;
	}
	if (EXIT_FAILURE == (fbfd = fbink_init(fbfd))) {
		fprintf(stderr, "Failed to initialize FBInk, aborting . . .\n");
		return EXIT_FAILURE;
	}

	char* string;
	if (optind < argc) {
		while (optind < argc) {
			string = argv[optind++];
			printf(
			    "Printing string '%s' @ column %hu, row %hu (inverted: %s, flashing: %s, centered: %s, left padded: %s, clear screen: %s)\n",
			    string,
			    fbink_config.col,
			    fbink_config.row,
			    fbink_config.is_inverted ? "true" : "false",
			    fbink_config.is_flashing ? "true" : "false",
			    fbink_config.is_centered ? "true" : "false",
			    fbink_config.is_padded ? "true" : "false",
			    fbink_config.is_cleared ? "true" : "false");
			if (fbink_print(fbfd, string, &fbink_config) < 0) {
				fprintf(stderr, "Failed to print that string!\n");
			}
			// NOTE: Don't clobber previous entries if multiple strings were passed...
			fbink_config.row++;
			// NOTE: By design, if you ask for a clear screen, only the final print will stay on screen ;).
		}
	} else {
		show_helpmsg();
	}

	// Cleanup
	if (fb_is_mapped) {
		munmap(fbp, screensize);
	}
	close(fbfd);

	return EXIT_SUCCESS;
}
