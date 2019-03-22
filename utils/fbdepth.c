/*
	FBInk: FrameBuffer eInker, a tool to print text & images on eInk devices (Kobo/Kindle)
	Copyright (C) 2018-2019 NiLuJe <ninuje@gmail.com>

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
	    "Tiny tool to set the framebuffer bitdepth on eInk devices.\n"
	    "\n"
	    "OPTIONS:\n"
	    "\t-d, --depth <8|16|24|32>\tSwitch the framebuffer to the supplied bitdepth.\n"
	    "\t-h, --help\t\t\tShow this help message.\n"
	    "\t-v, --verbose\t\t\tToggle printing diagnostic messages.\n"
	    "\t-q, --quiet\t\t\tToggle hiding diagnostic messages.\n"
	    "\t-g, --get\t\t\tJust output the current bitdepth to stdout.\n"
	    "\t-G, --getcode\t\t\tJust exit with the current bitdepth as exit code.\n"
	    "\t-r, --rota <-1|0|1|2|3>\tSwitch the framebuffer to the supplied rotation. -1 is a magic value matching the device-specific Portrait orientation.\n"
	    "\t-o, --getrota\t\t\tJust output the current rotation to stdout.\n"
	    "\t-O, --getrotacode\t\t\tJust exit with the current rotation as exit code.\n"
	    "\n",
	    fbink_version());
	return;
}

int fbfd = -1;

static bool
    get_fbinfo(void)
{
	// Get variable fb info
	if (ioctl(fbfd, FBIOGET_VSCREENINFO, &vInfo)) {
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
	if (ioctl(fbfd, FBIOGET_FSCREENINFO, &fInfo)) {
		perror("ioctl GET_F");
		return false;
	}
	LOG("Fixed fb info: ID is \"%s\", length of fb mem: %u bytes & line length: %u bytes",
	    fInfo.id,
	    fInfo.smem_len,
	    fInfo.line_length);

	return true;
}

static bool
    set_fbinfo(uint32_t bpp, int8_t rota)
{
	// Set variable fb info
	// Bitdepth
	vInfo.bits_per_pixel = bpp;
	LOG("Setting bitdepth to %ubpp", vInfo.bits_per_pixel);
	// Grayscale flag
	if (bpp == 8U) {
		// NOTE: 1 for Grayscale, 2 for Inverted Grayscale (like on einkfb).
		//       We obviously don't want to inflict an inverted palette on ourselves ;).
		//       c.f., GRAYSCALE_* defines @ mxcfb.h
		vInfo.grayscale = (uint32_t) GRAYSCALE_8BIT;
		LOG("Setting grayscale to %u", vInfo.grayscale);
	} else {
		// NOTE: And of course, 0 for color ;)
		vInfo.grayscale = (uint32_t) 0U;
		LOG("Setting grayscale to %u", vInfo.grayscale);
	}

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
	if (deviceQuirks.ntxBootRota == FB_ROTATE_UR) {
		// NOTE: This should cover the H2O and the few other devices suffering from the same quirk...
		vInfo.rotate ^= 2;
		LOG("Setting rotate to %u (%s) to account for kernel rotation quirks",
		    vInfo.rotate,
		    fb_rotate_to_string(vInfo.rotate));
	} else if (deviceQuirks.canRotate) {
		// NOTE: This is for the Forma, which only inverts CW & CCW (i.e., odd numbers)...
		if ((vInfo.rotate & 0x01) == 1) {
			vInfo.rotate ^= 2;
			LOG("Setting rotate to %u (%s) to account for kernel rotation quirks",
			    vInfo.rotate,
			    fb_rotate_to_string(vInfo.rotate));
		}
	}

	if (ioctl(fbfd, FBIOPUT_VSCREENINFO, &vInfo)) {
		perror("ioctl PUT_V");
		return false;
	}

	// NOTE: Double-check that we weren't bit by rotation quirks...
	if (vInfo.rotate != expected_rota) {
		LOG("\nCurrent rotation (%u) doesn't match the expected rotation (%u), attempting to fix it . . .",
		    vInfo.rotate,
		    expected_rota);

		// Brute-force it until it matches...
		for (int i = FB_ROTATE_UR; i <= FB_ROTATE_CCW; i++) {
			// If we finally got the right orientation, break the loop
			if (vInfo.rotate == expected_rota) {
				break;
			}
			// Do the i -> i + 1 -> i dance to be extra sure...
			// (This is useful on devices where the kernel *always* switches to the invert orientation, c.f., rota.c)
			vInfo.rotate = (uint32_t) i;
			if (ioctl(fbfd, FBIOPUT_VSCREENINFO, &vInfo)) {
				perror("ioctl PUT_V");
				return false;
			}
			LOG("Kernel rotation quirk recovery: %d -> %u", i, vInfo.rotate);

			// Don't do anything extra if that was enough...
			if (vInfo.rotate == expected_rota) {
				continue;
			}
			// Now for i + 1 w/ wraparound, since the valid rotation range is [0..3] (FB_ROTATE_UR to FB_ROTATE_CCW).
			// (i.e., a Portrait/Landscape swap to counteract potential side-effects of a kernel-side mandatory invert)
			uint32_t n   = (uint32_t)((i + 1) % 4);
			vInfo.rotate = n;
			if (ioctl(fbfd, FBIOPUT_VSCREENINFO, &vInfo)) {
				perror("ioctl PUT_V");
				return false;
			}
			LOG("Kernel rotation quirk recovery (intermediary @ %d): %u -> %u", i, n, vInfo.rotate);

			// And back to i, if need be...
			if (vInfo.rotate == expected_rota) {
				continue;
			}
			vInfo.rotate = (uint32_t) i;
			if (ioctl(fbfd, FBIOPUT_VSCREENINFO, &vInfo)) {
				perror("ioctl PUT_V");
				return false;
			}
			LOG("Kernel rotation quirk recovery: %d -> %u", i, vInfo.rotate);
		}
	}

	// Finally, warn if things *still* look FUBAR...
	if (vInfo.rotate != expected_rota) {
		LOG("\nCurrent rotation (%u) doesn't match the expected rotation (%u), here be dragons!",
		    vInfo.rotate,
		    expected_rota);
	}

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
	static const struct option opts[] = {
		{ "depth", required_argument, NULL, 'd' }, { "help", no_argument, NULL, 'h' },
		{ "verbose", no_argument, NULL, 'v' },     { "quiet", no_argument, NULL, 'q' },
		{ "get", no_argument, NULL, 'g' },         { "getcode", no_argument, NULL, 'G' },
		{ "rota", required_argument, NULL, 'r' },  { "getrota", no_argument, NULL, 'o' },
		{ "getrotacode", no_argument, NULL, 'O' }, { NULL, 0, NULL, 0 }
	};

	uint32_t req_bpp     = 0U;
	int8_t   req_rota    = -1;
	bool     errfnd      = false;
	bool     print_bpp   = false;
	bool     return_bpp  = false;
	bool     print_rota  = false;
	bool     return_rota = false;

	while ((opt = getopt_long(argc, argv, "d:hvqgGr:oO", opts, &opt_index)) != -1) {
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
					case 0:
					case 1:
					case 2:
					case 3:
						break;
					case -1:
						// NOTE: Nickel's Portrait orientation should *always* match BootRota + 1
						req_rota = (deviceQuirks.ntxBootRota + 1) % 4;
						LOG("Device's expected Portrait orientation should be '%hhd'!\n",
						    req_rota);
						break;
					default:
						fprintf(stderr, "Unsupported rotation '%s'!\n", optarg);
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
			default:
				fprintf(stderr, "?? Unknown option code 0%o ??\n", (unsigned int) opt);
				errfnd = true;
				break;
		}
	}

	if (errfnd || (req_bpp == 0U && !(print_bpp || return_bpp || print_rota || return_rota))) {
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
	fbfd = open("/dev/fb0", O_RDONLY | O_NONBLOCK | O_CLOEXEC);
	if (!fbfd) {
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

	// FIXME: Allow setting rota without bpp...
	//	  Ditch depth 0 check for showhelp, set it to current if req == 0 here, and fix the "Switching" recap to handle that... (current?)

	// If a change was requested, do it, but check if it's necessary first
	bool is_change_needed = false;
	if (vInfo.bits_per_pixel == req_bpp) {
		// Also check that the grayscale flag is flipped properly
		if ((vInfo.bits_per_pixel == 8U && vInfo.grayscale != GRAYSCALE_8BIT) ||
		    (vInfo.bits_per_pixel > 8U && vInfo.grayscale != 0U)) {
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

	if (vInfo.rotate == req_rota) {
		LOG("\nCurrent rotation is already %hhd!", req_rota);
		// No change needed as far as bitdepth is concerned...
	} else {
		is_change_needed = true;
	}

	// If it turns out that no actual change is needed, skip to cleanup, exiting successfully
	if (!is_change_needed) {
		goto cleanup;
	}

	// If we're here, we really want to change the bitdepth and/or rota ;)
	if (req_rota != -1) {
		LOG("\nSwitching fb to %ubpp @ rotation %hhd . . .", req_bpp, req_rota);
	} else {
		LOG("\nSwitching fb to %ubpp . . .", req_bpp);
	}
	if (!set_fbinfo(req_bpp, req_rota)) {
		rv = ERRCODE(EXIT_FAILURE);
		goto cleanup;
	}
	// Recap
	if (!get_fbinfo()) {
		rv = ERRCODE(EXIT_FAILURE);
		goto cleanup;
	}

cleanup:
	if (close(fbfd) != 0) {
		perror("close");
		rv = ERRCODE(EXIT_FAILURE);
	}

	return rv;
}
