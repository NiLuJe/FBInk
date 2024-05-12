/*
	FBInk: FrameBuffer eInker, a library to print text & images to an eInk Linux framebuffer
	Copyright (C) 2024 NiLuJe <ninuje@gmail.com>
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

// Because we're pretty much Linux-bound ;).
#ifndef _GNU_SOURCE
#	define _GNU_SOURCE
#endif

#include <errno.h>
#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <syslog.h>

#include "../fbink.h"

// Pilfer our usual macros from FBInk...
// We want to return negative values on failure, always
#define ERRCODE(e) (-(e))

// Likely/Unlikely branch tagging
#define likely(x)   __builtin_expect(!!(x), 1)
#define unlikely(x) __builtin_expect(!!(x), 0)

bool toSysLog  = false;
bool isQuiet   = false;
bool isVerbose = false;
// Handle what we send to stdout (i.e., mostly diagnostic stuff, which tends to be verbose, so no FBInk tag)
#define LOG(fmt, ...)                                                                                                    \
	({                                                                                                               \
		if (unlikely(isVerbose)) {                                                                               \
			if (toSysLog) {                                                                                  \
				syslog(LOG_INFO, "[InputScan] " fmt, ##__VA_ARGS__);                                     \
			} else {                                                                                         \
				fprintf(stdout, "[InputScan] " fmt "\n", ##__VA_ARGS__);                                 \
			}                                                                                                \
		}                                                                                                        \
	})

// And then what we send to stderr (mostly fbink_init stuff, add an FBInk tag to make it clear where it comes from for API users)
#define ELOG(fmt, ...)                                                                                                   \
	({                                                                                                               \
		if (!isQuiet) {                                                                                          \
			if (toSysLog) {                                                                                  \
				syslog(LOG_NOTICE, "[InputScan] " fmt, ##__VA_ARGS__);                                   \
			} else {                                                                                         \
				fprintf(stderr, "[InputScan] " fmt "\n", ##__VA_ARGS__);                                 \
			}                                                                                                \
		}                                                                                                        \
	})

// And a simple wrapper for actual warnings on error codepaths. Should only be used for warnings before a return/exit.
// Always shown, always tagged, and always ends with a bang.
#define WARN(fmt, ...)                                                                                                   \
	({                                                                                                               \
		if (toSysLog) {                                                                                          \
			syslog(LOG_ERR, "[InputScan] " fmt "!", ##__VA_ARGS__);                                          \
		} else {                                                                                                 \
			fprintf(stderr, "[InputScan] " fmt "!\n", ##__VA_ARGS__);                                        \
		}                                                                                                        \
	})

// Same, but with __PRETTY_FUNCTION__ right before fmt
#define PFWARN(fmt, ...) ({ WARN("[%s] " fmt, __PRETTY_FUNCTION__, ##__VA_ARGS__); })

// Help message
static void
    show_helpmsg(void)
{
	printf(
	    "\n"
	    "Input Scan (via FBInk %s)\n"
	    "\n"
	    "Usage: input_scan [-m] <type...> [-p]\n"
	    "\n"
	    "Scan & classify input devices.\n"
	    "\n"
	    "OPTIONS:\n"
	    "\t-h, --help\t\t\t\tShow this help message.\n"
	    "\t-v, --verbose\t\t\t\tToggle printing diagnostic messages.\n"
	    "\t-q, --quiet\t\t\t\tToggle hiding diagnostic messages.\n"
	    "\t-p, --print\t\t\t\tJust print the path of any matches as CSV to stdout.\n"
	    "\t-m, --match <type,type,type,...>\n"
	    "\t\t\t\t\t\tSimulate a match on specific input device types.\n"
	    "\t\t\t\tSupported types: unknown, pointingstick, mouse, touchpad, touchscreen, joystick, tablet, key, keyboard, accelerometer,\n"
	    "\t\t\t\t                 power, sleep, pagination, home, light, menu, dpad, rotation\n"
	    "\t-x, --exclude <type,type,type,...>\n"
	    "\t\t\t\t\t\tExclude input device types from your match request.\n"
	    "\n"
	    "EXAMPLES:\n"
	    "\tinput_scan -m touchscreen,power,pagination\n"
	    "\t\tWill simulate a match on *all* input devices that match *any* of the requested input types (here, a touchscreen, a power button, and pagination buttons)\n"
	    "\tinput_scan -m power -x touchscreen\n"
	    "\t\tWill simulate a match on input devices that provide a power button *and* that aren't also a touchscreen (because, yes, some touchscreens have weird caps)\n"
	    "\n",
	    fbink_version());
	return;
}

int
    main(int argc, char* argv[])
{
	int                        opt;
	// NOTE: getopt_long will only update opt_index when passed a *long* option,
	//       so we need to do the matching ourselves when we're passed *short* options, hence the sentinel value...
	int                        opt_index = -1;
	static const struct option opts[]    = {
                {    "help",       no_argument, NULL, 'h' },
                {   "print",       no_argument, NULL, 'p' },
                {   "match", required_argument, NULL, 'm' },
                { "exclude", required_argument, NULL, 'x' },
                {      NULL,                 0, NULL,   0 }
	};
	enum
	{
		OPT_INPUT_UNKNOWN = 0,
		// Standard udev classification
		OPT_INPUT_POINTINGSTICK,
		OPT_INPUT_MOUSE,
		OPT_INPUT_TOUCHPAD,
		OPT_INPUT_TOUCHSCREEN,
		OPT_INPUT_JOYSTICK,
		OPT_INPUT_TABLET,
		OPT_INPUT_KEY,
		OPT_INPUT_KEYBOARD,
		OPT_INPUT_ACCELEROMETER,
		// Custom classification, tailored for our use-cases
		OPT_INPUT_POWER_BUTTON,
		OPT_INPUT_SLEEP_COVER,
		OPT_INPUT_PAGINATION_BUTTONS,
		OPT_INPUT_HOME_BUTTON,
		OPT_INPUT_LIGHT_BUTTON,
		OPT_INPUT_MENU_BUTTON,
		OPT_INPUT_DPAD,
		OPT_INPUT_ROTATION_EVENT,
	};
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunknown-pragmas"
#pragma clang diagnostic ignored "-Wunknown-warning-option"
#pragma GCC diagnostic ignored "-Wdiscarded-qualifiers"
#pragma clang diagnostic ignored "-Wincompatible-pointer-types-discards-qualifiers"
	char* const match_token[] = { [OPT_INPUT_UNKNOWN]            = "unknown",
				      [OPT_INPUT_POINTINGSTICK]      = "pointingstick",
				      [OPT_INPUT_MOUSE]              = "mouse",
				      [OPT_INPUT_TOUCHPAD]           = "touchpad",
				      [OPT_INPUT_TOUCHSCREEN]        = "touchscreen",
				      [OPT_INPUT_JOYSTICK]           = "joystick",
				      [OPT_INPUT_TABLET]             = "tablet",
				      [OPT_INPUT_KEY]                = "key",
				      [OPT_INPUT_KEYBOARD]           = "keyboard",
				      [OPT_INPUT_ACCELEROMETER]      = "accelerometer",
				      [OPT_INPUT_POWER_BUTTON]       = "power",
				      [OPT_INPUT_SLEEP_COVER]        = "sleep",
				      [OPT_INPUT_PAGINATION_BUTTONS] = "pagination",
				      [OPT_INPUT_HOME_BUTTON]        = "home",
				      [OPT_INPUT_LIGHT_BUTTON]       = "light",
				      [OPT_INPUT_MENU_BUTTON]        = "menu",
				      [OPT_INPUT_DPAD]               = "dpad",
				      [OPT_INPUT_ROTATION_EVENT]     = "rotation",
				      NULL };
#pragma GCC diagnostic pop
	char*               full_subopts = NULL;
	char*               subopts;
	char*               value        = NULL;
	bool                print_only   = false;
	INPUT_DEVICE_TYPE_T scan_mask    = 0U;
	INPUT_DEVICE_TYPE_T exclude_mask = 0U;
	bool                errfnd       = false;

	// NOTE: Enforce line-buffering, to make I/O redirections less confusing (e.g., in DevCap logs),
	//       as we often mix stdout with stderr, and unlike stdout, stderr is always unbuffered (c.f., setvbuf(3)).
	setlinebuf(stdout);

	while ((opt = getopt_long(argc, argv, "vqpm:x:h", opts, &opt_index)) != -1) {
		switch (opt) {
			case 'v':
				isQuiet   = false;
				isVerbose = true;
				break;
			case 'q':
				isQuiet   = true;
				isVerbose = false;
				break;
			case 'p':
				print_only = true;
				break;
			case 'm':
			case 'x': {
				// We'll want our longform name for diagnostic messages...
				const char* opt_longname = NULL;
				// Look it up if we were passed the short form...
				if (opt_index == -1) {
					// Loop until we hit the final NULL entry
					for (opt_index = 0; opts[opt_index].name; opt_index++) {
						if (opts[opt_index].val == opt) {
							opt_longname = opts[opt_index].name;
							break;
						}
					}
					// Reset when we're done
					opt_index = -1;
				} else {
					opt_longname = opts[opt_index].name;
				}

				subopts = optarg;
				// NOTE: We'll need to remember the original, full suboption string for diagnostic messages,
				//       because getsubopt will rewrite it during processing...
				if (subopts && *subopts != '\0') {
					// Only remember the first offending suboption list...
					if (!errfnd) {
						full_subopts = strdupa(subopts);
					}
				}

				// Choose the right mask depending on whether we're in match or exclude
				INPUT_DEVICE_TYPE_T* mask;
				if (opt == 'm') {
					mask = &scan_mask;
				} else {
					mask = &exclude_mask;
				}

				while (subopts && *subopts != '\0' && !errfnd) {
					switch (getsubopt(&subopts, match_token, &value)) {
						case OPT_INPUT_UNKNOWN:
							*mask |= INPUT_UNKNOWN;
							break;
						case OPT_INPUT_POINTINGSTICK:
							*mask |= INPUT_POINTINGSTICK;
							break;
						case OPT_INPUT_MOUSE:
							*mask |= INPUT_MOUSE;
							break;
						case OPT_INPUT_TOUCHPAD:
							*mask |= INPUT_TOUCHPAD;
							break;
						case OPT_INPUT_TOUCHSCREEN:
							*mask |= INPUT_TOUCHSCREEN;
							break;
						case OPT_INPUT_JOYSTICK:
							*mask |= INPUT_JOYSTICK;
							break;
						case OPT_INPUT_TABLET:
							*mask |= INPUT_TABLET;
							break;
						case OPT_INPUT_KEY:
							*mask |= INPUT_KEY;
							break;
						case OPT_INPUT_KEYBOARD:
							*mask |= INPUT_KEYBOARD;
							break;
						case OPT_INPUT_ACCELEROMETER:
							*mask |= INPUT_ACCELEROMETER;
							break;
						case OPT_INPUT_POWER_BUTTON:
							*mask |= INPUT_POWER_BUTTON;
							break;
						case OPT_INPUT_SLEEP_COVER:
							*mask |= INPUT_SLEEP_COVER;
							break;
						case OPT_INPUT_PAGINATION_BUTTONS:
							*mask |= INPUT_PAGINATION_BUTTONS;
							break;
						case OPT_INPUT_HOME_BUTTON:
							*mask |= INPUT_HOME_BUTTON;
							break;
						case OPT_INPUT_LIGHT_BUTTON:
							*mask |= INPUT_LIGHT_BUTTON;
							break;
						case OPT_INPUT_MENU_BUTTON:
							*mask |= INPUT_MENU_BUTTON;
							break;
						case OPT_INPUT_DPAD:
							*mask |= INPUT_DPAD;
							break;
						case OPT_INPUT_ROTATION_EVENT:
							*mask |= INPUT_ROTATION_EVENT;
							break;
						default:
							ELOG("No match found for token: /%s/ for -%c, --%s",
							     value,
							     opt,
							     opt_longname);
							errfnd = true;
							break;
					}
				}

				// Only remember this if there was a parsing error.
				if (!errfnd) {
					full_subopts = NULL;

					// We'll want to enable verbose mode to see our own recap
					isVerbose = true;
				}
				break;
			}
			case 'h':
				show_helpmsg();
				return EXIT_SUCCESS;
				break;
			default:
				WARN("?? Unknown option code 0%o ??", (unsigned int) opt);
				errfnd = true;
				break;
		}
	}

	if (errfnd) {
		show_helpmsg();
		// NOTE: Having the actual error message printed *above* the seven billion lines of the help message
		//       pretty much ensures no one will ever notice it, so remind the user that there's also
		//       an actual error message to read much higher in their terminal backlog ;p
		if (errfnd) {
			if (!toSysLog) {
				fprintf(stderr, "\n****\t****\t****\t****\n\n");
			}
			WARN("Encountered a parsing error, see the top of the output for details");
			// Recap the exact invocation, as seen by getopt,
			// (note that it will reorder argv so that non-option arguments end up at the end).
			if (!toSysLog) {
				fprintf(stderr, "\n");
			}
			// NOTE: Almost, because getsubopt rewrites argv (it replaces commas with NULLs),
			//       which means we don't have access to the original string anymore...
			ELOG("This was the (almost) exact invocation that triggered this error:\n");
			if (!toSysLog) {
				for (int i = 0; i < argc; i++) {
					fprintf(stderr, "%s%s", argv[i], i == argc - 1 ? "\n" : " ");
				}
				// Then detail it...
				fprintf(stderr, "\n");
				ELOG("Broken down argument per argument:\n");
			}
			for (int i = 0; i < optind; i++) {
				ELOG("argv[%d]: `%s`", i, argv[i]);
			}
			// If there was a subopt parsing error, print the original offending suboption list
			if (full_subopts) {
				ELOG("Complete offending suboption string: %s\n", full_subopts);
			}
			// And then non-option arguments
			if (optind < argc) {
				if (!toSysLog) {
					fprintf(stderr, "\n");
				}
				ELOG("And the following non-option arguments:\n");
				for (int i = optind; i < argc; i++) {
					ELOG("argv[%d]: `%s`", i, argv[i]);
				}
			}
		}
		return ERRCODE(EXIT_FAILURE);
	}

	if (print_only) {
		// We need to enforce quiet for this to "work" ;).
		isQuiet   = true;
		isVerbose = false;
	}

	// We don't actually need to init FBInk, but we can update its verbosity state,
	// in order to potentially see the verbose logging from test_key.
	FBInkConfig fbink_cfg = { 0 };
	fbink_cfg.is_verbose  = isVerbose;
	fbink_cfg.is_quiet    = isQuiet;
	fbink_update_verbosity(&fbink_cfg);

	// Assume success, until shit happens ;)
	int rv = EXIT_SUCCESS;

	LOG("Requested match mask: %#.8x", scan_mask);
	LOG("Requested exclude mask: %#.8x", exclude_mask);
	size_t            dev_count;
	FBInkInputDevice* devices = fbink_input_scan(scan_mask, exclude_mask, SCAN_ONLY, &dev_count);
	LOG("Found %zu readable input devices", dev_count);
	if (devices) {
		for (FBInkInputDevice* device = devices; device < devices + dev_count; device++) {
			LOG("Device %s @ %s is classified as %#.8x (matched: %d)",
			    device->name,
			    device->path,
			    device->type,
			    device->matched);

			if (print_only && device->matched) {
				fprintf(stdout, "%s,", device->path);
			}
		}
		free(devices);
	}

	return rv;
}
