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
			fbink_print(fbfd, string, &fbink_config);
			// NOTE: Don't clobber previous entries if multiple strings were passed...
			fbink_config.row++;
			// NOTE: By design, if you ask for a clear screen, only the final print will stay on screen ;).
		}
	} else {
		printf("Usage!\n");
	}

	// Cleanup
	if (fb_is_mapped) {
		munmap(fbp, screensize);
	}
	close(fbfd);

	return EXIT_SUCCESS;
}
