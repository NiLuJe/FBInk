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
	    "Usage: fbdepth [-d] <bpp>\n"
	    "\n"
	    "Tiny tool to set the framebuffer bitdepth on eInk devices.\n"
	    "\n"
	    "OPTIONS:\n"
	    "\t-d, --depth <8|16|24|32>\tSwitch the framebuffer to the supplied bitdepth.\n"
	    "\t-h, --help\t\tShow this help message.\n"
	    "\t-v, --verbose\t\tToggle printing diagnostic messages.\n"
	    "\t-q, --quiet\t\tToggle hiding hardware setup messages, as well as the coordinates themselves.\n"
	    "\n",
	    fbink_version());
	return;
}

int fbfd = -1;

static void
    get_fbinfo(void)
{
	// Get variable fb info
	if (ioctl(fbfd, FBIOGET_VSCREENINFO, &vInfo)) {
		perror("ioctl GET_V");
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
	}
	LOG("Fixed fb info: ID is \"%s\", length of fb mem: %u bytes & line length: %u bytes",
	    fInfo.id,
	    fInfo.smem_len,
	    fInfo.line_length);
}

static void
    set_fbinfo(uint32_t bpp)
{
	// Set variable fb info
	// Bitdepth
	vInfo.bits_per_pixel = bpp;
	LOG("Setting bitdepth to %ubpp", vInfo.bits_per_pixel);
	// Grayscale flag
	if (bpp == 8U) {
		// NOTE: 1 for Grayscale, 2 for Inverted Grayscale (like on einkfb)
		vInfo.grayscale = (uint32_t) 1U;
		LOG("Setting grayscale to %u", vInfo.grayscale);
	} else {
		// NOTE: And of course, 0 for color ;)
		vInfo.grayscale = (uint32_t) 0U;
		LOG("Setting grayscale to %u", vInfo.grayscale);
	}

	// NOTE: We have to counteract the rotation shenanigans the Kernel might be enforcing...
	//       c.f., mxc_epdc_fb_check_var @ drivers/video/mxc/mxc_epdc_fb.c OR drivers/video/fbdev/mxc/mxc_epdc_v2_fb.c
	// NOTE: This should cover the H2O and the few other devices suffering from the same quirk...
	//       The goal being to end up in the *same* effective rotation as before.
	if (deviceQuirks.ntxBootRota == FB_ROTATE_UR) {
		vInfo.rotate = (uint32_t) vInfo.rotate ^ 2;
		LOG("Setting rotate to %u (%s) to account for kernel rotation quirks",
		    vInfo.rotate,
		    fb_rotate_to_string(vInfo.rotate));
	}

	if (ioctl(fbfd, FBIOPUT_VSCREENINFO, &vInfo)) {
		perror("ioctl PUT_V");
	}

	LOG("Bitdepth is now %ubpp (grayscale: %u) [rotate: %u (%s)]",
	    vInfo.bits_per_pixel,
	    vInfo.grayscale,
	    vInfo.rotate,
	    fb_rotate_to_string(vInfo.rotate));
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
					      { NULL, 0, NULL, 0 } };

	uint32_t req_bpp = 0U;
	bool     errfnd  = false;

	while ((opt = getopt_long(argc, argv, "d:hvq", opts, &opt_index)) != -1) {
		switch (opt) {
			case 'd':
				req_bpp = strtoul(optarg, NULL, 10);
				// Cheap-ass sanity check
				switch (req_bpp) {
					case 8:
					case 16:
					case 24:
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
			default:
				fprintf(stderr, "?? Unknown option code 0%o ??\n", (unsigned int) opt);
				errfnd = true;
				break;
		}
	}

	if (errfnd || req_bpp == 0U) {
		show_helpmsg();
		return ERRCODE(EXIT_FAILURE);
	}

	// NOTE: We're going to need to identify the device, to handle rotation quirks...
	identify_device();

	// NOTE: We only need this for ioctl, hence O_NONBLOCK (as per open(2)).
	fbfd = open("/dev/fb0", O_RDONLY | O_NONBLOCK | O_CLOEXEC);
	if (!fbfd) {
		perror("open");
		return ERRCODE(EXIT_FAILURE);
	}

	// Print initial status, and store current vInfo
	get_fbinfo();

	// let's check how quirky it is...
	LOG("\nSwitching fb to %ubpp . . .", req_bpp);
	set_fbinfo(req_bpp);
	// Recap
	get_fbinfo();

	close(fbfd);
}
