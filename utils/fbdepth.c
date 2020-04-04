/*
	FBInk: FrameBuffer eInker, a tool to print text & images on eInk devices (Kobo/Kindle)
	Copyright (C) 2018-2020 NiLuJe <ninuje@gmail.com>
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

#include <getopt.h>
#include <stdio.h>
// I feel dirty.
#include "../fbink.c"

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
	    "\t-d, --depth <8|16|24|32>\tSwitch the framebuffer to the supplied bitdepth.\n"
	    "\t-h, --help\t\t\tShow this help message.\n"
	    "\t-v, --verbose\t\t\tToggle printing diagnostic messages.\n"
	    "\t-q, --quiet\t\t\tToggle hiding diagnostic messages.\n"
	    "\t-g, --get\t\t\tJust output the current bitdepth to stdout.\n"
	    "\t-G, --getcode\t\t\tJust exit with the current bitdepth as exit code.\n"
#if defined(FBINK_FOR_KOBO) || defined(FBINK_FOR_CERVANTES)
	    "\t-r, --rota <-1|0|1|2|3>\t\tSwitch the framebuffer to the supplied rotation. -1 is a magic value matching the device-specific Portrait orientation.\n"
#else
	    "\t-r, --rota <0|1|2|3>\t\tSwitch the framebuffer to the supplied rotation (Linux FB convention).\n"
#endif
	    "\t-o, --getrota\t\t\tJust output the current rotation to stdout.\n"
	    "\t-O, --getrotacode\t\tJust exit with the current rotation as exit code.\n"
	    "\t-H, --nightmode <on|off|toggle>\tToggle hardware inversion (8bpp only, safely ignored otherwise).\n"
	    "\n",
	    fbink_version());
	return;
}

// Pilfered from KFMon, with some minor tweaks to make it tri-state...
static int
    strtotristate(const char* restrict str, int8_t* restrict result)
{
	if (!str) {
		WARN("Passed an empty value to a key expecting a tri-state value");
		return -EINVAL;
	}

	switch (str[0]) {
		case 't':
		case 'T':
			if (strcasecmp(str, "true") == 0) {
				*result = true;
				return EXIT_SUCCESS;
			} else if (strcasecmp(str, "toggle") == 0) {
				*result = -1;
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
					*result = -1;
					return EXIT_SUCCESS;
				}
			}
			break;
		default:
			// NOTE: *result is initialized to a sentinel value, leave it alone.
			break;
	}

	WARN("Assigned an invalid or malformed value (%s) to a flag expecting a tri-state value", str);
	return -EINVAL;
}

#ifdef FBINK_FOR_KINDLE
static const char*
    einkfb_orientation_to_string(orientation_t orientation)
{
	switch (orientation) {
		case orientation_portrait:
			return "Portrait, 0°";
		case orientation_portrait_upside_down:
			return "Inverted Portrait (Upside Down), 180°";
		case orientation_landscape:
			return "Landscape, 90°";
		case orientation_landscape_upside_down:
			return "Inverted Landscape (Upside Down), 270°";
		default:
			return "Unknown?!";
	}
}

static orientation_t
    linuxfb_rotate_to_einkfb_orientation(uint32_t rotate)
{
	switch (rotate) {
		case FB_ROTATE_UR:
			return orientation_portrait;
		case FB_ROTATE_CW:
			return orientation_landscape;
		case FB_ROTATE_UD:
			return orientation_portrait_upside_down;
		case FB_ROTATE_CCW:
			return orientation_landscape_upside_down;
		default:
			// Should never happen.
			return orientation_portrait;
	}
}

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

int fbFd = -1;

static bool
    get_fbinfo(void)
{
	// Get variable fb info
	if (ioctl(fbFd, FBIOGET_VSCREENINFO, &vInfo)) {
		perror("ioctl GET_V");
		return false;
	}
	LOG("Variable fb info: %ux%u (%ux%u), %ubpp @ rotation: %u (%s)",
	    vInfo.xres,
	    vInfo.yres,
	    vInfo.xres_virtual,
	    vInfo.yres_virtual,
	    vInfo.bits_per_pixel,
	    vInfo.rotate,
	    fb_rotate_to_string(vInfo.rotate));
	// Get fixed fb information
	if (ioctl(fbFd, FBIOGET_FSCREENINFO, &fInfo)) {
		perror("ioctl GET_F");
		return false;
	}
	LOG("Fixed fb info: ID is \"%s\", length of fb mem: %u bytes & line length: %u bytes",
	    fInfo.id,
	    fInfo.smem_len,
	    fInfo.line_length);

#ifdef FBINK_FOR_KINDLE
	// NOTE: einkfb devices (even the K4, which only uses it as a shim over mxcfb HW)
	//       don't actually honor the standard Linux fb rotation, and instead rely on a set of custom ioctls...
	if (deviceQuirks.isKindleLegacy) {
		orientation_t orientation = orientation_portrait;
		if (ioctl(fbFd, FBIO_EINK_GET_DISPLAY_ORIENTATION, &orientation)) {
			perror("ioctl FBIO_EINK_GET_DISPLAY_ORIENTATION");
			return false;
		}

		// Because everything is terrible, it's actually not the same mapping as the Linux fb rotate field...
		LOG("Actual einkfb orientation: %u (%s)", orientation, einkfb_orientation_to_string(orientation));
	}
#endif

	return true;
}

static bool
    set_fbinfo(uint32_t bpp, int8_t rota, uint32_t req_gray)
{
	// Set variable fb info
	// Bitdepth
	vInfo.bits_per_pixel = bpp;
	LOG("Setting bitdepth to %ubpp", vInfo.bits_per_pixel);
	// Grayscale flag
	vInfo.grayscale = req_gray;
	LOG("Setting grayscale to %u", vInfo.grayscale);

	// NOTE: We have to counteract the rotation shenanigans the Kernel might be enforcing...
	//       c.f., mxc_epdc_fb_check_var @ drivers/video/mxc/mxc_epdc_fb.c OR drivers/video/fbdev/mxc/mxc_epdc_v2_fb.c
	//       The goal being to end up in the *same* effective rotation as before.
	// First, remember the current rotation as the expected one...
	uint32_t expected_rota = vInfo.rotate;
	// Then, set the requested rotation, if there was one...
	if (rota != -1) {
		vInfo.rotate = (uint32_t) rota;
		LOG("Setting rotate to %u (%s)", vInfo.rotate, fb_rotate_to_string(vInfo.rotate));
		// And flag it as the expected rota for the sanity checks
		expected_rota = (uint32_t) rota;
	}
#if defined(FBINK_FOR_KOBO) || defined(FBINK_FOR_CERVANTES)
	if (deviceQuirks.ntxRotaQuirk == NTX_ROTA_ALL_INVERTED) {
		// NOTE: This should cover the H2O and the few other devices suffering from the same quirk...
		vInfo.rotate ^= 2;
		LOG("Setting rotate to %u (%s) to account for kernel rotation quirks",
		    vInfo.rotate,
		    fb_rotate_to_string(vInfo.rotate));
	} else if (deviceQuirks.ntxRotaQuirk == NTX_ROTA_ODD_INVERTED) {
		// NOTE: This is for the Forma, which only inverts CW & CCW (i.e., odd numbers)...
		if ((vInfo.rotate & 0x01) == 1) {
			vInfo.rotate ^= 2;
			LOG("Setting rotate to %u (%s) to account for kernel rotation quirks",
			    vInfo.rotate,
			    fb_rotate_to_string(vInfo.rotate));
		}
	}
#endif

	if (ioctl(fbFd, FBIOPUT_VSCREENINFO, &vInfo)) {
		perror("ioctl PUT_V");
		return false;
	}

#ifdef FBINK_FOR_KINDLE
	// Deal once again with einkfb properly...
	if (deviceQuirks.isKindleLegacy) {
		orientation_t orientation = linuxfb_rotate_to_einkfb_orientation(expected_rota);
		if (ioctl(fbFd, FBIO_EINK_SET_DISPLAY_ORIENTATION, orientation)) {
			perror("ioctl FBIO_EINK_SET_DISPLAY_ORIENTATION");
			return false;
		}

		LOG("Setting actual einkfb orientation to %u (%s)",
		    orientation,
		    einkfb_orientation_to_string(orientation));
	}
#endif

#if defined(FBINK_FOR_KOBO) || defined(FBINK_FOR_CERVANTES)
	// NOTE: Double-check that we weren't bit by rotation quirks...
	if (vInfo.rotate != expected_rota) {
		LOG("\nCurrent rotation (%u) doesn't match the expected rotation (%u), attempting to fix it . . .",
		    vInfo.rotate,
		    expected_rota);

		// Brute-force it until it matches...
		for (uint32_t i = vInfo.rotate, j = FB_ROTATE_UR; j <= FB_ROTATE_CCW; i = (i + 1U) & 3U, j++) {
			// If we finally got the right orientation, break the loop
			if (vInfo.rotate == expected_rota) {
				break;
			}
			// Do the i -> i + 1 -> i dance to be extra sure...
			// (This is useful on devices where the kernel *always* switches to the invert orientation, c.f., rota.c)
			vInfo.rotate = i;
			if (ioctl(fbFd, FBIOPUT_VSCREENINFO, &vInfo)) {
				perror("ioctl PUT_V");
				return false;
			}
			LOG("Kernel rotation quirk recovery: %u -> %u", i, vInfo.rotate);

			// Don't do anything extra if that was enough...
			if (vInfo.rotate == expected_rota) {
				continue;
			}
			// Now for i + 1 w/ wraparound, since the valid rotation range is [0..3] (FB_ROTATE_UR to FB_ROTATE_CCW).
			// (i.e., a Portrait/Landscape swap to counteract potential side-effects of a kernel-side mandatory invert)
			uint32_t n   = (i + 1U) & 3U;
			vInfo.rotate = n;
			if (ioctl(fbFd, FBIOPUT_VSCREENINFO, &vInfo)) {
				perror("ioctl PUT_V");
				return false;
			}
			LOG("Kernel rotation quirk recovery (intermediary @ %u): %u -> %u", i, n, vInfo.rotate);

			// And back to i, if need be...
			if (vInfo.rotate == expected_rota) {
				continue;
			}
			vInfo.rotate = i;
			if (ioctl(fbFd, FBIOPUT_VSCREENINFO, &vInfo)) {
				perror("ioctl PUT_V");
				return false;
			}
			LOG("Kernel rotation quirk recovery: %u -> %u", i, vInfo.rotate);
		}
	}

	// Finally, warn if things *still* look FUBAR...
	if (vInfo.rotate != expected_rota) {
		LOG("\nCurrent rotation (%u) doesn't match the expected rotation (%u), here be dragons!",
		    vInfo.rotate,
		    expected_rota);
	}
#endif

#ifdef FBINK_FOR_KINDLE
	// And, again, einkfb is a special snowflake...
	if (deviceQuirks.isKindleLegacy) {
		orientation_t orientation = orientation_portrait;
		if (ioctl(fbFd, FBIO_EINK_GET_DISPLAY_ORIENTATION, &orientation)) {
			perror("ioctl FBIO_EINK_GET_DISPLAY_ORIENTATION");
			return false;
		}

		LOG("Actual einkfb orientation is now %u (%s)", orientation, einkfb_orientation_to_string(orientation));
	}
#endif
	LOG("Bitdepth is now %ubpp (grayscale: %u) @ rotate: %u (%s)\n",
	    vInfo.bits_per_pixel,
	    vInfo.grayscale,
	    vInfo.rotate,
	    fb_rotate_to_string(vInfo.rotate));

	return true;
}

int
    main(int argc, char* argv[])
{
	// For the LOG & ELOG macros
	g_isQuiet   = false;
	g_isVerbose = true;

	int                        opt;
	int                        opt_index;
	static const struct option opts[] = { { "depth", required_argument, NULL, 'd' },
					      { "help", no_argument, NULL, 'h' },
					      { "verbose", no_argument, NULL, 'v' },
					      { "quiet", no_argument, NULL, 'q' },
					      { "get", no_argument, NULL, 'g' },
					      { "getcode", no_argument, NULL, 'G' },
					      { "rota", required_argument, NULL, 'r' },
					      { "getrota", no_argument, NULL, 'o' },
					      { "getrotacode", no_argument, NULL, 'O' },
					      { "nightmode", required_argument, NULL, 'H' },
					      { NULL, 0, NULL, 0 } };

	uint32_t req_bpp     = 0U;
	int8_t   req_rota    = 42;
	int8_t   want_nm     = 42;
	bool     errfnd      = false;
	bool     print_bpp   = false;
	bool     return_bpp  = false;
	bool     print_rota  = false;
	bool     return_rota = false;

	while ((opt = getopt_long(argc, argv, "d:hvqgGr:oOH:", opts, &opt_index)) != -1) {
		switch (opt) {
			case 'd':
				req_bpp = (uint32_t) strtoul(optarg, NULL, 10);
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
						fprintf(
						    stderr,
						    "Warning! 24bpp handling appears to be broken *somewhere*, you probably don't want to use it!\n\n");
						break;
					case 32:
						break;
					default:
						fprintf(stderr, "Unsupported bitdepth '%s'!\n", optarg);
						errfnd = true;
						break;
				}
				break;
			case 'v':
				g_isQuiet   = false;
				g_isVerbose = true;
				break;
			case 'q':
				g_isQuiet   = true;
				g_isVerbose = false;
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
				req_rota = (int8_t) strtol(optarg, NULL, 10);
				// Cheap-ass sanity check
				switch (req_rota) {
					case FB_ROTATE_UR:
					case FB_ROTATE_CW:
					case FB_ROTATE_UD:
					case FB_ROTATE_CCW:
						break;
					case -1:
						// NOTE: We'll compute it later, as we need the results from identify_device() ;).
						break;
					default:
						fprintf(stderr, "Invalid rotation '%s'!\n", optarg);
						errfnd = true;
						break;
				}
				break;
			case 'o':
				print_rota = true;
				break;
			case 'O':
				return_rota = true;
				break;
			case 'H':
				if (strtotristate(optarg, &want_nm) < 0) {
					fprintf(stderr, "Invalid nightmode state '%s'!\n", optarg);
					errfnd = true;
				}
				break;
			default:
				fprintf(stderr, "?? Unknown option code 0%o ??\n", (unsigned int) opt);
				errfnd = true;
				break;
		}
	}

	if (errfnd || ((req_bpp == 0U && req_rota == 42 && want_nm == 42) &&
		       !(print_bpp || return_bpp || print_rota || return_rota))) {
		show_helpmsg();
		return ERRCODE(EXIT_FAILURE);
	}

	// Enforce quiet w/ print_*
	if (print_bpp || print_rota) {
		g_isQuiet   = true;
		g_isVerbose = false;
	}

	// Assume success, until shit happens ;)
	int rv = EXIT_SUCCESS;

	// NOTE: We're going to need to identify the device, to handle rotation quirks...
	identify_device();

	// NOTE: We only need this for ioctl, hence O_NONBLOCK (as per open(2)).
	fbFd = open("/dev/fb0", O_RDONLY | O_NONBLOCK | O_CLOEXEC);
	if (fbFd == -1) {
		perror("open");
		return ERRCODE(EXIT_FAILURE);
	}

	// Print initial status, and store current vInfo
	if (!get_fbinfo()) {
		rv = ERRCODE(EXIT_FAILURE);
		goto cleanup;
	}

	// If we just wanted to print/return the current bitdepth, abort early
	if (print_bpp || return_bpp) {
		if (print_bpp) {
			fprintf(stdout, "%u", vInfo.bits_per_pixel);
		}
		if (return_bpp) {
			rv = (int) vInfo.bits_per_pixel;
			goto cleanup;
		} else {
			goto cleanup;
		}
	}

	// If we just wanted to print/return the current rotation, abort early
	if (print_rota || return_rota) {
		if (print_rota) {
			fprintf(stdout, "%u", vInfo.rotate);
		}
		if (return_rota) {
			rv = (int) vInfo.rotate;
			goto cleanup;
		} else {
			goto cleanup;
		}
	}

	// If no bitdepth was requested, set to the current one, we'll be double-checking if changes are actually needed.
	if (req_bpp == 0U) {
		req_bpp = vInfo.bits_per_pixel;
	}

	// If the automagic Portrait rotation was requested, compute it
#if defined(FBINK_FOR_KOBO) || defined(FBINK_FOR_CERVANTES)
	if (req_rota == -1) {
		// NOTE: For *most* devices, Nickel's Portrait orientation should *always* match BootRota + 1
		//       Thankfully, the Libra appears to be ushering in a new era filled with puppies and rainbows,
		//       and, hopefully, less insane rotation quirks ;).
		if (deviceQuirks.ntxRotaQuirk != NTX_ROTA_SANE) {
			req_rota = (deviceQuirks.ntxBootRota + 1) & 3;
		} else {
			req_rota = (int8_t) deviceQuirks.ntxBootRota;
		}
		LOG("Device's expected Portrait orientation should be: %hhd (%s)!",
		    req_rota,
		    fb_rotate_to_string((uint32_t) req_rota));
	}
#endif

	// If no rotation was requested, reset req_rota to our expected sentinel value
	if (req_rota == 42) {
		req_rota = -1;
	}

	// Ensure the requested rotation is sane (if all is well, this should never be tripped)
	if (req_rota < -1 || req_rota > FB_ROTATE_CCW) {
		LOG("Requested rotation (%hhd) is bogus, discarding it!\n", req_rota);
		req_rota = -1;
	}

	// Compute the proper grayscale flag given the current bitdepth and whether we want to enable nightmode or not...
	// We rely on the EPDC feature that *toggles* HW inversion, no matter the bitdepth
	// (by essentially *flipping* the EPDC_FLAG_ENABLE_INVERSION flag, on the kernel side).
	// c.f., epdc_process_update @ mxc_epdc_fb
	// In practice, though, grayscale != 0U is *invalid* for > 8bpp (and it does break rendering),
	// so we only play with this @ 8bpp ;).
	// NOTE: While we technically don't allow switching to 4bpp, make sure we leave it alone,
	//       because there are dedicated GRAYSCALE_4BIT & GRAYSCALE_4BIT_INVERTED constants...
	uint32_t req_gray = vInfo.grayscale;
	if (want_nm == true) {
		if (req_bpp == 8U) {
			req_gray = GRAYSCALE_8BIT_INVERTED;
		} else if (req_bpp > 8U) {
			req_gray = 0U;
		}
	} else if (want_nm == false) {
		if (req_bpp == 8U) {
			req_gray = GRAYSCALE_8BIT;
		} else if (req_bpp > 8U) {
			req_gray = 0U;
		}
	} else if (want_nm == -1) {
		// Toggle...
		if (req_bpp == 8U) {
			// NOTE: We check for 0 in case the current bitdepth is not already 8bpp...
			if (vInfo.grayscale == GRAYSCALE_8BIT || vInfo.grayscale == 0U) {
				req_gray = GRAYSCALE_8BIT_INVERTED;
			} else {
				req_gray = GRAYSCALE_8BIT;
			}
		} else if (req_bpp > 8U) {
			req_gray = 0U;
		}
	} else {
		// Otherwise, make sure we default to sane values for a non-inverted palette...
		if (req_bpp == 8U) {
			req_gray = GRAYSCALE_8BIT;
		} else if (req_bpp > 8U) {
			req_gray = 0U;
		}
	}

	// If a change was requested, do it, but check if it's necessary first
	bool is_change_needed = false;

	// Start by checking that the grayscale flag is flipped properly
	if (vInfo.grayscale == req_gray) {
		LOG("\nCurrent grayscale flag is already %u!", req_gray);
		// No change needed as far as grayscale is concerned...
	} else {
		is_change_needed = true;
	}

	// Then bitdepth...
	if (vInfo.bits_per_pixel == req_bpp) {
		// Also check that the grayscale flag is flipped properly (again)
		if (vInfo.grayscale != req_gray) {
			LOG("\nCurrent bitdepth is already %ubpp, but the grayscale flag is bogus!", req_bpp);
			// Continue, we'll need to flip the grayscale flag properly
			is_change_needed = true;
		} else {
			LOG("\nCurrent bitdepth is already %ubpp!", req_bpp);
			// No change needed as far as bitdepth is concerned...
		}
	} else {
		is_change_needed = true;
	}

	// Same for rotation, if we requested one...
	if (req_rota != -1) {
#ifdef FBINK_FOR_KINDLE
		if (deviceQuirks.isKindleLegacy) {
			// We need to check the effective orientation on einkfb...
			orientation_t orientation = orientation_portrait;
			if (ioctl(fbFd, FBIO_EINK_GET_DISPLAY_ORIENTATION, &orientation)) {
				perror("ioctl FBIO_EINK_GET_DISPLAY_ORIENTATION");
				return false;
			}

			uint32_t rotate = einkfb_orientation_to_linuxfb_rotate(orientation);
			if (rotate == (uint32_t) req_rota) {
				LOG("\nCurrent rotation is already %hhd!", req_rota);
				// No change needed as far as rotation is concerned...
			} else {
				is_change_needed = true;
			}
		} else {
			// On mxcfb, everything's peachy
			if (vInfo.rotate == (uint32_t) req_rota) {
				LOG("\nCurrent rotation is already %hhd!", req_rota);
				// No change needed as far as rotation is concerned...
			} else {
				is_change_needed = true;
			}
		}
#else
		if (vInfo.rotate == (uint32_t) req_rota) {
			LOG("\nCurrent rotation is already %hhd!", req_rota);
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
	if (req_rota != -1) {
		LOG("\nSwitching fb to %ubpp%s @ rotation %hhd . . .",
		    req_bpp,
		    (req_bpp == vInfo.bits_per_pixel) ? " (current bitdepth)" : "",
		    req_rota);
	} else {
		LOG("\nSwitching fb to %ubpp%s . . .",
		    req_bpp,
		    (req_bpp == vInfo.bits_per_pixel) ? " (current bitdepth)" : "");
	}
	if (!set_fbinfo(req_bpp, req_rota, req_gray)) {
		rv = ERRCODE(EXIT_FAILURE);
		goto cleanup;
	}
	// Recap
	if (!get_fbinfo()) {
		rv = ERRCODE(EXIT_FAILURE);
		goto cleanup;
	}

cleanup:
	if (close(fbFd) != 0) {
		perror("close");
		rv = ERRCODE(EXIT_FAILURE);
	}

	return rv;
}
