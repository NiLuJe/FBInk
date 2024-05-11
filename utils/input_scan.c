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
	    "Usage: input_scan\n"
	    "\n"
	    "Scan & classify input devices.\n"
	    "\n"
	    "OPTIONS:\n"
	    "\t-h, --help\t\t\t\tShow this help message.\n"
	    "\t-v, --verbose\t\t\t\tToggle printing diagnostic messages.\n"
	    "\t-q, --quiet\t\t\t\tToggle hiding diagnostic messages.\n"
	    "\n",
	    fbink_version());
	return;
}

int
    main(int argc, char* argv[])
{
	int                        opt;
	int                        opt_index;
	static const struct option opts[] = {
		{ "help", no_argument, NULL, 'h' },
                {   NULL,           0, NULL,   0 }
	};
	bool errfnd = false;

	// NOTE: Enforce line-buffering, to make I/O redirections less confusing (e.g., in DevCap logs),
	//       as we often mix stdout with stderr, and unlike stdout, stderr is always unbuffered (c.f., setvbuf(3)).
	setlinebuf(stdout);

	while ((opt = getopt_long(argc, argv, "hvq", opts, &opt_index)) != -1) {
		switch (opt) {
			case 'v':
				isQuiet   = false;
				isVerbose = true;
				break;
			case 'q':
				isQuiet   = true;
				isVerbose = false;
				break;
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
		return ERRCODE(EXIT_FAILURE);
	}

	// We don't actually need to init FBInk, but we can update its verbosity state,
	// in order to potentially see the verbose logging from test_key.
	FBInkConfig fbink_cfg = { 0 };
	fbink_cfg.is_verbose  = isVerbose;
	fbink_cfg.is_quiet    = isQuiet;
	fbink_update_verbosity(&fbink_cfg);

	// Assume success, until shit happens ;)
	int rv = EXIT_SUCCESS;

	size_t            dev_count;
	FBInkInputDevice* devices = fbink_input_scan(SCAN_ONLY, &dev_count);
	// FIXME: Debug
	LOG("Devices: %p (count: %zu)", devices, dev_count);
	if (devices) {
		for (FBInkInputDevice* device = devices; device < devices + dev_count; device++) {
			LOG("Device %s @ %s is of type %u", device->name, device->path, device->type);
		}
		free(devices);
	}

	return rv;
}
