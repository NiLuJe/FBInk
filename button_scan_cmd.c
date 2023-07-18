/*
	FBInk: FrameBuffer eInker, a library to print text & images to an eInk Linux framebuffer
	Copyright (C) 2018-2023 NiLuJe <ninuje@gmail.com>
	SPDX-License-Identifier: GPL-3.0-or-later

	----

	This program is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/

#include "button_scan_cmd.h"

// Help message
static void
    show_helpmsg(void)
{
	printf(
	    "\n"
	    "Button Scan (w/ FBInk %s)\n"
	    "\n"
	    "Usage: button_scan [-pwubhvq]\n"
	    "\n"
	    "Spits out x, y coordinates for the USB Connect button from Kobo's 'USB Plugged In' popup, optionally pressing it in the process.\n"
	    "\n"
	    "OPTIONS:\n"
	    "\t-p, --press\tGenerate an input event to automatically press the button.\n"
	    "\t-w, --wait\tWhile in the \"Connected\" state, wait for the end of this USBMS session, and detect the end of the content import process.\n"
	    "\t-u, --unplug\tExplicitly fake an USB unplug event to exit the USBMS session as soon as possible.\n"
	    "\t\t\tDoing this with a *real* USBMS sessions is a potentially terrible idea! This is aimed at purely faked USBMS sessions.\n"
	    "\t-b, --both\tALL THE THINGS! Do everything in a single shot (scan for the button, press it, and wait for the end of the USBMS session).\n"
	    "\t-h, --help\tShow this help message.\n"
	    "\t-v, --verbose\tToggle printing diagnostic messages.\n"
	    "\t-q, --quiet\tToggle hiding hardware setup messages, as well as the coordinates themselves.\n"
	    "\t-G, --syslog\tSend output to syslog instead of stdout & stderr.\n"
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
		{  "press", no_argument, NULL, 'p'},
		{   "wait", no_argument, NULL, 'w'},
		{ "unplug", no_argument, NULL, 'u'},
		{   "both", no_argument, NULL, 'b'},
		{   "help", no_argument, NULL, 'h'},
		{"verbose", no_argument, NULL, 'v'},
		{  "quiet", no_argument, NULL, 'q'},
		{ "syslog", no_argument, NULL, 'G'},
		{     NULL,           0, NULL,   0}
	};

	FBInkConfig fbink_cfg = { 0U };
	// Default to verbose for now
	fbink_cfg.is_verbose  = true;

	bool press_button      = false;
	bool detect_import     = false;
	bool do_wait_for_usbms = false;
	bool force_unplug      = false;
	bool errfnd            = false;

	while ((opt = getopt_long(argc, argv, "pwubhvqG", opts, &opt_index)) != -1) {
		switch (opt) {
			case 'p':
				press_button = true;
				break;
			case 'w':
				do_wait_for_usbms = true;
				break;
			case 'u':
				force_unplug = true;
				break;
			case 'b':
				press_button  = true;
				detect_import = true;
				break;
			case 'v':
				fbink_cfg.is_verbose = !fbink_cfg.is_verbose;
				break;
			case 'q':
				fbink_cfg.is_quiet = !fbink_cfg.is_quiet;
				break;
			case 'G':
				fbink_cfg.to_syslog = !fbink_cfg.to_syslog;
				break;
			case 'h':
				show_helpmsg();
				return EXIT_SUCCESS;
				break;
			default:
				fprintf(stderr, "?? Unknown option code 0%o ??\n", (unsigned int) opt);
				errfnd = true;
				break;
		}
	}

	if (errfnd || argc > optind) {
		show_helpmsg();
		return ERRCODE(EXIT_FAILURE);
	}

	// Assume success, until shit happens ;)
	int rv = EXIT_SUCCESS;

	// Open framebuffer and keep it around, then setup globals.
	int fbfd = fbink_open();
	if (fbfd == ERRCODE(EXIT_FAILURE)) {
		fprintf(stderr, "Failed to open the framebuffer, aborting . . .\n");
		return ERRCODE(EXIT_FAILURE);
	}
	if (fbink_init(fbfd, &fbink_cfg) != EXIT_SUCCESS) {
		fprintf(stderr, "Failed to initialize FBInk, aborting . . .\n");
		rv = ERRCODE(EXIT_FAILURE);
		goto cleanup;
	}

	// And actually do stuff :)
	if (do_wait_for_usbms) {
		rv = fbink_wait_for_usbms_processing(fbfd, force_unplug);
	} else {
		rv = fbink_button_scan(fbfd, press_button, false);
		// If the button press was successful, optionally wait for the end of the USBMS session
		if (press_button && rv == EXIT_SUCCESS && detect_import) {
			rv = fbink_wait_for_usbms_processing(fbfd, force_unplug);
		}
	}

	// Cleanup
cleanup:
	if (fbink_close(fbfd) == ERRCODE(EXIT_FAILURE)) {
		fprintf(stderr, "Failed to close the framebuffer, aborting . . .\n");
		rv = ERRCODE(EXIT_FAILURE);
	}

	return rv;
}
