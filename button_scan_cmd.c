/*
	FBInk: FrameBuffer eInker, a tool to print text & images on eInk devices (Kobo/Kindle)
	Copyright (C) 2018 NiLuJe <ninuje@gmail.com>

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

#include "button_scan_cmd.h"

// Help message
static void
    show_helpmsg(void)
{
	printf(
	    "\n"
	    "Button Scan (w/ FBInk %s)\n"
	    "\n"
	    "Usage: button_scan\n"
	    "\n"
	    "Spits out x, y coordinates for the USB Connect button from Kobo's 'USB Plugged In' popup.\n"
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
		{ "press", no_argument, NULL, 'p' },   { NULL, 0, NULL, 0 }
	};

	FBInkConfig fbink_config = { 0U };

	bool      press_button   = false;
	int       errfnd         = 0;

	while ((opt = getopt_long(argc, argv, "p", opts, &opt_index)) != -1) {
		switch (opt) {
			case 'p':
				press_button = true;
				break;
			default:
				fprintf(stderr, "?? Unknown option code 0%o ??\n", (unsigned int) opt);
				errfnd = 1;
				break;
		}
	}

	if (errfnd == 1 || argc == 1) {
		show_helpmsg();
		return ERRCODE(EXIT_FAILURE);
	}

	// Open framebuffer and keep it around, then setup globals.
	int fbfd = -1;
	if (ERRCODE(EXIT_FAILURE) == (fbfd = fbink_open())) {
		fprintf(stderr, "Failed to open the framebuffer, aborting . . .\n");
		return ERRCODE(EXIT_FAILURE);
	}
	if (fbink_init(fbfd, &fbink_config) == ERRCODE(EXIT_FAILURE)) {
		fprintf(stderr, "Failed to initialize FBInk, aborting . . .\n");
		return ERRCODE(EXIT_FAILURE);
	}

	fbink_button_scan(fbfd, press_button);

	// Cleanup
	if (fbink_close(fbfd) == ERRCODE(EXIT_FAILURE)) {
		fprintf(stderr, "Failed to close the framebuffer, aborting . . .\n");
		return ERRCODE(EXIT_FAILURE);
	}

	return EXIT_SUCCESS;
}
