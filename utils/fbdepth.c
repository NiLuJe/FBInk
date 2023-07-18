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

// Because we're pretty much Linux-bound ;).
#ifndef _GNU_SOURCE
#	define _GNU_SOURCE
#endif

#include <errno.h>
#include <getopt.h>
#include <linux/fb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <syslog.h>

#include "../fbink.h"

#if defined(FBINK_FOR_KINDLE)
#	include <sys/ioctl.h>
#	include "../eink/einkfb.h"
#endif

// Pilfer our usual macros from FBInk...
// We want to return negative values on failure, always
#define ERRCODE(e) (-(e))

// Likely/Unlikely branch tagging
#define likely(x)   __builtin_expect(!!(x), 1)
#define unlikely(x) __builtin_expect(!!(x), 0)

bool toSysLog  = false;
bool isQuiet   = false;
bool isVerbose = true;
// Handle what we send to stdout (i.e., mostly diagnostic stuff, which tends to be verbose, so no FBInk tag)
#define LOG(fmt, ...)                                                                                                    \
	({                                                                                                               \
		if (unlikely(isVerbose)) {                                                                               \
			if (toSysLog) {                                                                                  \
				syslog(LOG_INFO, "[FBDepth] " fmt, ##__VA_ARGS__);                                       \
			} else {                                                                                         \
				fprintf(stdout, "[FBDepth] " fmt "\n", ##__VA_ARGS__);                                   \
			}                                                                                                \
		}                                                                                                        \
	})

// And then what we send to stderr (mostly fbink_init stuff, add an FBInk tag to make it clear where it comes from for API users)
#define ELOG(fmt, ...)                                                                                                   \
	({                                                                                                               \
		if (!isQuiet) {                                                                                          \
			if (toSysLog) {                                                                                  \
				syslog(LOG_NOTICE, "[FBDepth] " fmt, ##__VA_ARGS__);                                     \
			} else {                                                                                         \
				fprintf(stderr, "[FBDepth] " fmt "\n", ##__VA_ARGS__);                                   \
			}                                                                                                \
		}                                                                                                        \
	})

// And a simple wrapper for actual warnings on error codepaths. Should only be used for warnings before a return/exit.
// Always shown, always tagged, and always ends with a bang.
#define WARN(fmt, ...)                                                                                                   \
	({                                                                                                               \
		if (toSysLog) {                                                                                          \
			syslog(LOG_ERR, "[FBDepth] " fmt "!", ##__VA_ARGS__);                                            \
		} else {                                                                                                 \
			fprintf(stderr, "[FBDepth] " fmt "!\n", ##__VA_ARGS__);                                          \
		}                                                                                                        \
	})

// Same, but with __PRETTY_FUNCTION__ right before fmt
#define PFWARN(fmt, ...) ({ WARN("[%s] " fmt, __PRETTY_FUNCTION__, ##__VA_ARGS__); })

#ifdef FBINK_FOR_KINDLE
static uint32_t
    einkfb_orientation_to_linuxfb_rotate(orientation_t orientation)
{
	switch (orientation) {
		case orientation_portrait:
			return FB_ROTATE_UR;
		case orientation_portrait_upside_down:
			return FB_ROTATE_UD;
		case orientation_landscape:
			return FB_ROTATE_CW;
		case orientation_landscape_upside_down:
			return FB_ROTATE_CCW;
		default:
			// Should never happen.
			return FB_ROTATE_UR;
	}
}
#endif

// Help message
static void
    show_helpmsg(void)
{
	printf(
	    "\n"
	    "FBDepth (via FBInk %s)\n"
	    "\n"
	    "Usage: fbdepth [-d] <bpp> [-r] <rota>\n"
	    "\n"
	    "Tiny tool to set the framebuffer bitdepth and/or rotation on eInk devices.\n"
	    "\n"
	    "OPTIONS:\n"
	    "\t-d, --depth <8|16|24|32>\t\tSwitch the framebuffer to the supplied bitdepth.\n"
	    "\t-h, --help\t\t\t\tShow this help message.\n"
	    "\t-v, --verbose\t\t\t\tToggle printing diagnostic messages.\n"
	    "\t-q, --quiet\t\t\t\tToggle hiding diagnostic messages.\n"
	    "\t-g, --get\t\t\t\tJust output the current bitdepth to stdout.\n"
	    "\t-G, --getcode\t\t\t\tJust exit with the current bitdepth as exit code.\n"
#if defined(FBINK_FOR_KOBO) || defined(FBINK_FOR_CERVANTES)
	    "\t-r, --rota <-1|0|1|2|3> \t\tSwitch the framebuffer to the supplied rotation. -1 is a magic value matching the device-specific Portrait orientation.\n"
#else
	    "\t-r, --rota <0|1|2|3>\t\tSwitch the framebuffer to the supplied rotation (Linux FB convention).\n"
#endif
#if defined(FBINK_FOR_KOBO)
	    "\t-R, --canonicalrota <UR|CW|UD|CCW>\tSwitch the framebuffer to the supplied canonical rotation (Linux FB convention), automagically translating it to the mangled native one. (i.e., requesting UR will ensure the device is actually UR, much like passing -1 to -r, --rota would).\n"
#endif
	    "\t-o, --getrota\t\t\t\tJust output the current rotation to stdout.\n"
	    "\t-O, --getrotacode\t\t\tJust exit with the current rotation as exit code.\n"
#if defined(FBINK_FOR_KOBO)
	    "\t-c, --getcanonicalrota\t\t\tJust output the current rotation (converted to its canonical representation) to stdout.\n"
	    "\t-C, --getcanonicalrotacode\t\tJust exit with the current rotation (converted to its canonical representation) as exit code.\n"
#endif
	    "\t-H, --nightmode <on|off|toggle>\t\tToggle hardware inversion (8bpp only, safely ignored otherwise).\n"
	    "\n",
	    fbink_version());
	return;
}

// Pilfered from KFMon, with some minor tweaks to make it tri-state...
static int
    strtotristate(const char* restrict str, uint8_t* restrict result)
{
	if (!str) {
		WARN("Passed an empty value to a key expecting a tri-state value");
		return ERRCODE(EINVAL);
	}

	switch (str[0]) {
		case 't':
		case 'T':
			if (strcasecmp(str, "true") == 0) {
				*result = true;
				return EXIT_SUCCESS;
			} else if (strcasecmp(str, "toggle") == 0) {
				*result = TOGGLE_GRAYSCALE;
				return EXIT_SUCCESS;
			}
			break;
		case 'y':
		case 'Y':
			if (strcasecmp(str, "yes") == 0) {
				*result = true;
				return EXIT_SUCCESS;
			}
			break;
		case '1':
			if (str[1] == '\0') {
				*result = true;
				return EXIT_SUCCESS;
			}
			break;
		case 'f':
		case 'F':
			if (strcasecmp(str, "false") == 0) {
				*result = false;
				return EXIT_SUCCESS;
			}
			break;
		case 'n':
		case 'N':
			switch (str[1]) {
				case 'o':
				case 'O':
					if (str[2] == '\0') {
						*result = false;
						return EXIT_SUCCESS;
					}
					break;
				default:
					break;
			}
			break;
		case '0':
			if (str[1] == '\0') {
				*result = false;
				return EXIT_SUCCESS;
			}
			break;
		case 'o':
		case 'O':
			switch (str[1]) {
				case 'n':
				case 'N':
					if (str[2] == '\0') {
						*result = true;
						return EXIT_SUCCESS;
					}
					break;
				case 'f':
				case 'F':
					switch (str[2]) {
						case 'f':
						case 'F':
							if (str[3] == '\0') {
								*result = false;
								return EXIT_SUCCESS;
							}
							break;
						default:
							break;
					}
					break;
				default:
					break;
			}
			break;
		case '-':
			if (str[1] == '1') {
				if (str[2] == '\0') {
					*result = TOGGLE_GRAYSCALE;
					return EXIT_SUCCESS;
				}
			}
			break;
		default:
			// NOTE: *result is initialized to a sentinel value, leave it alone.
			break;
	}

	WARN("Assigned an invalid or malformed value (%s) to a flag expecting a tri-state value", str);
	return ERRCODE(EINVAL);
}

static int
    get_fb_info(int                       fbfd,
		FBInkConfig*              fbink_cfg,
		FBInkState*               fbink_state,
		struct fb_var_screeninfo* var_info,
		struct fb_fix_screeninfo* fix_info)
{
	// We're going to need to current state to check what we actually need to do
	fbink_get_state(fbink_cfg, fbink_state);
	size_t buffer_size = 0U;
	fbink_get_fb_pointer(fbfd, &buffer_size);
	fbink_get_fb_info(var_info, fix_info);

	// Print initial status
	LOG("Screen is %ux%u (%ux%zu addressable, fb says %ux%u)",
	    fbink_state->screen_width,
	    fbink_state->screen_height,
	    (fbink_state->scanline_stride << 3U) / fbink_state->bpp,
	    buffer_size / fbink_state->scanline_stride,
	    var_info->xres_virtual,
	    var_info->yres_virtual);
	LOG("Buffer is mapped for %zu bytes with a scanline stride of %u bytes",
	    buffer_size,
	    fbink_state->scanline_stride);

	return EXIT_SUCCESS;
}

int
    main(int argc, char* argv[])
{
	int                        opt;
	int                        opt_index;
	static const struct option opts[] = {
		{               "depth", required_argument, NULL, 'd'},
		{                "help",       no_argument, NULL, 'h'},
		{             "verbose",       no_argument, NULL, 'v'},
		{               "quiet",       no_argument, NULL, 'q'},
		{                 "get",       no_argument, NULL, 'g'},
		{             "getcode",       no_argument, NULL, 'G'},
		{                "rota", required_argument, NULL, 'r'},
		{       "canonicalrota", required_argument, NULL, 'R'},
		{             "getrota",       no_argument, NULL, 'o'},
		{         "getrotacode",       no_argument, NULL, 'O'},
		{    "getcanonicalrota",       no_argument, NULL, 'c'},
		{"getcanonicalrotacode",       no_argument, NULL, 'C'},
		{           "nightmode", required_argument, NULL, 'H'},
		{		  NULL,                 0, NULL,   0}
	};

	uint8_t  req_bpp          = KEEP_CURRENT_BITDEPTH;
	uint32_t req_rota         = KEEP_CURRENT_ROTATE;
	uint8_t  want_nm          = KEEP_CURRENT_GRAYSCALE;
	bool     errfnd           = false;
	bool     print_bpp        = false;
	bool     return_bpp       = false;
	bool     print_rota       = false;
	bool     return_rota      = false;
	bool     print_canonical  = false;
	bool     return_canonical = false;
#if defined(FBINK_FOR_KOBO)
	bool canonical_rota = false;
#endif

	while ((opt = getopt_long(argc, argv, "d:hvqgGr:R:oOcCH:", opts, &opt_index)) != -1) {
		switch (opt) {
			case 'd':
				req_bpp = (uint8_t) strtoul(optarg, NULL, 10);
				// Cheap-ass sanity check
				switch (req_bpp) {
					case 8:
					case 16:
						break;
					case 24:
						// NOTE: Warn that things will probably be wonky...
						//       I'm not quite sure who's to blame: this tool, FBInk, or the Kernel,
						//       but I've never ended up in a useful state on my Kobos.
						//       And I don't have a genuine 24bpp fb device to compare to...
						WARN(
						    "Warning! 24bpp handling appears to be broken *somewhere*, you probably don't want to use it");
						break;
					case 32:
						break;
					default:
						WARN("Unsupported bitdepth '%s'", optarg);
						errfnd = true;
						break;
				}
				break;
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
			case 'g':
				print_bpp = true;
				break;
			case 'G':
				return_bpp = true;
				break;
			case 'r':
				if (strcasecmp(optarg, "UR") == 0 || strcmp(optarg, "0") == 0) {
					req_rota = FB_ROTATE_UR;
				} else if (strcasecmp(optarg, "CW") == 0 || strcmp(optarg, "1") == 0) {
					req_rota = FB_ROTATE_CW;
				} else if (strcasecmp(optarg, "UD") == 0 || strcmp(optarg, "2") == 0) {
					req_rota = FB_ROTATE_UD;
				} else if (strcasecmp(optarg, "CCW") == 0 || strcmp(optarg, "3") == 0) {
					req_rota = FB_ROTATE_CCW;
					// If the automagic Portrait rotation was requested, compute it
#if defined(FBINK_FOR_KOBO) || defined(FBINK_FOR_CERVANTES)
				} else if (strcmp(optarg, "-1") == 0) {
					req_rota = 42U;
#endif
				} else {
					WARN("Invalid rotation '%s'", optarg);
					errfnd = true;
				}
				break;
			case 'R':
#if defined(FBINK_FOR_KOBO)
				if (strcasecmp(optarg, "UR") == 0 || strcmp(optarg, "0") == 0) {
					req_rota = FB_ROTATE_UR;
				} else if (strcasecmp(optarg, "CW") == 0 || strcmp(optarg, "1") == 0) {
					req_rota = FB_ROTATE_CW;
				} else if (strcasecmp(optarg, "UD") == 0 || strcmp(optarg, "2") == 0) {
					req_rota = FB_ROTATE_UD;
				} else if (strcasecmp(optarg, "CCW") == 0 || strcmp(optarg, "3") == 0) {
					req_rota = FB_ROTATE_CCW;
				} else {
					WARN("Invalid rotation '%s'", optarg);
					errfnd = true;
				}
				canonical_rota = true;
#else
				WARN("This option (-R, --canonicalrota) is not supported on your device");
				errfnd = true;
#endif
				break;
			case 'o':
				print_rota = true;
				break;
			case 'O':
				return_rota = true;
				break;
			case 'c':
#if defined(FBINK_FOR_KOBO)
				print_canonical = true;
#else
				WARN("This option (-c, --getcanonicalrota) is not supported on your device");
				errfnd = true;
#endif
				break;
			case 'C':
#if defined(FBINK_FOR_KOBO)
				return_canonical = true;
#else
				WARN("This option (-C, --getcanonicalrotacode) is not supported on your device");
				errfnd = true;
#endif
				break;
			case 'H':
				if (strtotristate(optarg, &want_nm) < 0) {
					WARN("Invalid nightmode state '%s'", optarg);
					errfnd = true;
				}
				break;
			default:
				WARN("?? Unknown option code 0%o ??", (unsigned int) opt);
				errfnd = true;
				break;
		}
	}

	if (errfnd ||
	    ((req_bpp == KEEP_CURRENT_BITDEPTH && req_rota == KEEP_CURRENT_ROTATE && want_nm == KEEP_CURRENT_GRAYSCALE) &&
	     !(print_bpp || return_bpp || print_rota || return_rota || print_canonical || return_canonical))) {
		show_helpmsg();
		return ERRCODE(EXIT_FAILURE);
	}

	// Enforce quiet w/ print_*
	if (print_bpp || print_rota || print_canonical) {
		isQuiet   = true;
		isVerbose = false;
	}

	// Assume success, until shit happens ;)
	int rv = EXIT_SUCCESS;

	// Init FBInk
	FBInkConfig fbink_cfg = { 0 };
	fbink_cfg.is_verbose  = isVerbose;
	fbink_cfg.is_quiet    = isQuiet;
	int fbfd              = fbink_open();
	// Open framebuffer and keep it around, then setup globals.
	if (fbfd == ERRCODE(EXIT_FAILURE)) {
		WARN("Failed to open the framebuffer, aborting");
		rv = ERRCODE(EXIT_FAILURE);
		goto cleanup;
	}
	if (fbink_init(fbfd, &fbink_cfg) != EXIT_SUCCESS) {
		WARN("Failed to initialize FBInk, aborting");
		rv = ERRCODE(EXIT_FAILURE);
		goto cleanup;
	}

	// We're also going to need to current state to check what we actually need to do
	FBInkState               fbink_state = { 0 };
	struct fb_var_screeninfo var_info    = { 0 };
	struct fb_fix_screeninfo fix_info    = { 0 };
	// Print initial status
	if (get_fb_info(fbfd, &fbink_cfg, &fbink_state, &var_info, &fix_info) != EXIT_SUCCESS) {
		rv = ERRCODE(EXIT_FAILURE);
		goto cleanup;
	}

	// If we just wanted to print/return the current bitdepth, abort early
	if (print_bpp || return_bpp) {
		if (print_bpp) {
			fprintf(stdout, "%u", fbink_state.bpp);
		}
		if (return_bpp) {
			rv = (int) fbink_state.bpp;
			goto cleanup;
		} else {
			goto cleanup;
		}
	}

	// If we just wanted to print/return the current rotation, abort early
	if (print_rota || return_rota) {
		if (print_rota) {
			fprintf(stdout, "%u", fbink_state.current_rota);
		}
		if (return_rota) {
			rv = (int) fbink_state.current_rota;
			goto cleanup;
		} else {
			goto cleanup;
		}
	}

#if defined(FBINK_FOR_KOBO)
	// If we just wanted to print/return the current canonical rotation, abort early
	if (print_canonical || return_canonical) {
		if (print_canonical) {
			fprintf(stdout, "%hhu", fbink_rota_native_to_canonical(fbink_state.current_rota));
		}
		if (return_canonical) {
			rv = (int) fbink_rota_native_to_canonical(fbink_state.current_rota);
			goto cleanup;
		} else {
			goto cleanup;
		}
	}
#endif

// If the automagic Portrait rotation was requested, compute it
#if defined(FBINK_FOR_KOBO) || defined(FBINK_FOR_CERVANTES)
	if (req_rota == 42U) {
		// NOTE: For *most* devices, Nickel's Portrait orientation should *always* match BootRota + 1
		//       Thankfully, the Libra appears to be ushering in a new era filled with puppies and rainbows,
		//       and, hopefully, less insane rotation quirks ;).
		if (fbink_state.ntx_rota_quirk != NTX_ROTA_SANE) {
			req_rota = (fbink_state.ntx_boot_rota + 1) & 3;
		} else {
			req_rota = fbink_state.ntx_boot_rota;
		}
		LOG("Device's expected Portrait orientation should be: %u!", req_rota);
	}
#endif

	// Ensure the requested rotation is sane (if all is well, this should never be tripped)
	if (req_rota != KEEP_CURRENT_ROTATE && req_rota > FB_ROTATE_CCW) {
		LOG("Requested rotation (%u) is bogus, discarding it!", req_rota);
		req_rota = KEEP_CURRENT_ROTATE;
	}

	// If a change was requested, do it, but check if it's necessary first
	bool is_change_needed = false;

	// Compute the proper grayscale flag given the current bitdepth and whether we want to enable nightmode or not...
	// We rely on the EPDC feature that *toggles* HW inversion, no matter the bitdepth
	// (by essentially *flipping* the EPDC_FLAG_ENABLE_INVERSION flag, on the kernel side).
	// c.f., epdc_process_update @ mxc_epdc_fb
	// In practice, though, grayscale != 0U is *invalid* for > 8bpp (and it does break rendering),
	// so we only play with this @ 8bpp ;).
	// NOTE: While we technically don't allow switching to 4bpp, make sure we leave it alone,
	//       because there are dedicated GRAYSCALE_4BIT & GRAYSCALE_4BIT_INVERTED constants...
	uint8_t req_gray = KEEP_CURRENT_GRAYSCALE;
	if ((req_bpp == KEEP_CURRENT_BITDEPTH && fbink_state.bpp == 8U) || req_bpp == 8U) {
		if (want_nm == true) {
			req_gray = 2U;    // GRAYSCALE_8BIT_INVERTED
		} else if (want_nm == false) {
			req_gray = 1U;    // GRAYSCALE_8BIT
		} else if (want_nm == TOGGLE_GRAYSCALE) {
			req_gray = TOGGLE_GRAYSCALE;
		}

		// Start by checking that the grayscale flag is flipped properly
		if (req_gray == KEEP_CURRENT_GRAYSCALE || var_info.grayscale == req_gray) {
			if (req_gray != KEEP_CURRENT_GRAYSCALE) {
				LOG("Current grayscale flag is already %u!", var_info.grayscale);
			}
			// No change needed as far as grayscale is concerned...
		} else {
			is_change_needed = true;
		}
	}

	// Then bitdepth...
	if (req_bpp == KEEP_CURRENT_BITDEPTH || fbink_state.bpp == req_bpp) {
		// Also check that the grayscale flag is sane
		if (req_gray == KEEP_CURRENT_GRAYSCALE &&
		    ((fbink_state.bpp == 8U && (var_info.grayscale == 0U || var_info.grayscale > 2U)) ||
		     (fbink_state.bpp > 8U && var_info.grayscale != 0U))) {
			LOG("Current bitdepth is already %ubpp, but the grayscale flag is bogus!", fbink_state.bpp);
			// Continue, we'll need to flip the grayscale flag properly
			is_change_needed = true;

			// And don't leave it set to KEEP_CURRENT_GRAYSCALE, since it's currently bogus...
			if (fbink_state.bpp == 8U) {
				req_gray = 1U;
			} else {
				req_gray = 0U;
			}
		} else {
			if (req_bpp != KEEP_CURRENT_BITDEPTH) {
				LOG("Current bitdepth is already %ubpp!", fbink_state.bpp);
			}
			// No change needed as far as bitdepth is concerned...
		}
	} else {
		is_change_needed = true;
	}

	// Same for rotation, if we requested one...
	if (req_rota != KEEP_CURRENT_ROTATE) {
#ifdef FBINK_FOR_KINDLE
		if (fbink_state.is_kindle_legacy) {
			// We need to check the effective orientation on einkfb...
			orientation_t orientation = orientation_portrait;
			if (ioctl(fbfd, FBIO_EINK_GET_DISPLAY_ORIENTATION, &orientation)) {
				WARN("FBIO_EINK_GET_DISPLAY_ORIENTATION: %m");
				rv = ERRCODE(EXIT_FAILURE);
				goto cleanup;
			}

			uint32_t rotate = einkfb_orientation_to_linuxfb_rotate(orientation);
			if (rotate == req_rota) {
				LOG("Current rotation is already %u!", fbink_state.current_rota);
				// No change needed as far as rotation is concerned...
			} else {
				is_change_needed = true;
			}
		} else {
			// On mxcfb, everything's peachy
			if (fbink_state.current_rota == req_rota) {
				LOG("Current rotation is already %u!", fbink_state.current_rota);
				// No change needed as far as rotation is concerned...
			} else {
				is_change_needed = true;
			}
		}
#else
#	if defined(FBINK_FOR_KOBO)
		// If the requested rota was canonical, translate it to a native one *now*
		if (canonical_rota) {
			LOG("Requested canonical rota %u translates to %u for this device",
			    req_rota,
			    fbink_rota_canonical_to_native((uint8_t) req_rota));
			req_rota = fbink_rota_canonical_to_native((uint8_t) req_rota);
		}
#	endif
		if (fbink_state.current_rota == req_rota) {
			LOG("Current rotation is already %u!", fbink_state.current_rota);
			// No change needed as far as rotation is concerned...
		} else {
			is_change_needed = true;
		}
#endif
	}

	// If it turns out that no actual changes are needed, skip to cleanup, exiting successfully
	if (!is_change_needed) {
		goto cleanup;
	}

	// If we're here, we really want to change the bitdepth and/or rota ;)
	if (req_rota != KEEP_CURRENT_ROTATE) {
		if (req_bpp == KEEP_CURRENT_BITDEPTH) {
			LOG("Switching fb to rotation %u . . .", req_rota);
		} else {
			LOG("Switching fb to %ubpp @ rotation %u . . .", req_bpp, req_rota);
		}
	} else {
		if (req_bpp != KEEP_CURRENT_BITDEPTH) {
			LOG("Switching fb to %ubpp . . .", req_bpp);
		}
	}
	int ret = fbink_set_fb_info(fbfd, req_rota, req_bpp, req_gray, &fbink_cfg);
	if (ret < 0 && ret != ERRCODE(ENOSYS)) {
		rv = ERRCODE(EXIT_FAILURE);
		goto cleanup;
	}
	// Recap
	if (get_fb_info(fbfd, &fbink_cfg, &fbink_state, &var_info, &fix_info) != EXIT_SUCCESS) {
		rv = ERRCODE(EXIT_FAILURE);
		goto cleanup;
	}

cleanup:
	if (fbink_close(fbfd) == ERRCODE(EXIT_FAILURE)) {
		WARN("Failed to close the framebuffer, aborting");
		rv = ERRCODE(EXIT_FAILURE);
	}

	return rv;
}
