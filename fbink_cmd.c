/*
	FBInk: FrameBuffer eInker, a tool to print strings on eInk devices (Kobo/Kindle)
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

#include "fbink_cmd.h"

// Help message
static void
    show_helpmsg(void)
{
	printf(
	    "\n"
#ifdef FBINK_MINIMAL
	    "FBInk %s [Minimalistic build]\n"
#else
	    "FBInk %s\n"
#endif
	    "\n"
	    "Usage: fbink [OPTIONS] [STRING]\n"
	    "\n"
	    "Print STRING on your device's screen.\n"
	    "\n"
	    "EXAMPLES:\n"
	    "\tfbink -x 1 -y 10 \"Hello World!\"\n"
	    "\t\tPrints Hello World on the eleventh line, starting at the second column from the left.\n"
	    "\tfbink -pmh -y -5 \"Hello World!\"\n"
	    "\t\tPrints Hello World, highlighted (i.e., white on black), centered & padded on both sides, on the fifth line starting from the bottom.\n"
	    "\n"
	    "Options affecting the message's position on screen:\n"
	    "\t-x, --col NUM\tBegin printing STRING @ column NUM (Default: 0).\n"
	    "\t\t\tBeginning at column 1 instead of 0 is recommended, because column 0 (the leftmost one) is often half-obscured by a bezel.\n"
	    "\t-y, --row NUM\tBegin printing STRING @ row NUM (Default: 0).\n"
	    "\t\t\tBeginning at row 1 instead of 0 is recommended, because row 0 (the topmost one) is often half-obscured by a bezel, especially on Kobos.\n"
	    "\t-m, --centered\tDynamically override col to print STRING at the center of the screen.\n"
	    "\t\t\tSpecial care is taken to avoid the very edges of the screen, to ensure the complete legibility of the message.\n"
	    "\t-p, --padded\tLeft pad STRING with blank spaces.\n"
	    "\t\t\tMost useful when combined with --centered to ensure a line will be completely filled, while still centering STRING.\n"
	    "\n"
	    "Options affecting the message's appearance:\n"
	    "\t-h, --invert\tPrint STRING in white over a black background instead of the reverse.\n"
	    "\t-f, --flash\tAsk the eInk driver to do a black flash when refreshing the area of the screen where STRING will be printed.\n"
	    "\t-c, --clear\tFully clear the screen before printing (obeys --invert).\n"
	    "\t-S, --size\tOverride the automatic font scaling multiplier (0 means automatic selection, which ranges from 1 (no scaling), to 4 (4x upscaling), depending on screen resolution).\n"
#ifdef FBINK_WITH_UNSCII
	    "\t\t\tNote that user-supplied values will be silently clamped to safe boundaries (from 1 to 31 for most fonts, and from 1 to 22 for TALL).\n"
#else
	    "\t\t\tNote that user-supplied values will be silently clamped to safe boundaries (from 1 to 31).\n"
#endif
	    "\t-F, --font NAME\tRender glyphs from builtin font NAME (Default: IBM).\n"
#ifdef FBINK_WITH_UNSCII
	    "\t\t\tAvailable font families: IBM, UNSCII, ALT, THIN, FANTASY, MCR, TALL\n"
#else
	    "\t\t\tAvailable font families: IBM\n"
#endif
	    "\n"
	    "Options affecting the program's verbosity:\n"
	    "\t-v, --verbose\tToggle printing diagnostic messages.\n"
	    "\t-q, --quiet\tToggle hiding hardware setup messages.\n"
	    "\n"
	    "NOTES:\n"
	    "\tYou can specify multiple STRINGs in a single invocation of fbink, each consecutive one will be printed on consecutive lines.\n"
	    "\t\tAlthough it's worth mentioning that this will lead to undesirable results when combined with --clear,\n"
	    "\t\tbecause the screen is cleared before each STRING, meaning you'll only get to see the final one.\n"
	    "\tIf you want to properly print a long string, better do it in a single argument, fbink will do its best to spread it over multiple lines sanely.\n"
	    "\tIt will also honor the linefeed character (and I do mean the actual control character, not the human-readable escape sequence),\n"
	    "\twhich comes in handy when passing a few lines of logs straight from tail as an argument.\n"
	    "\n\n"
	    "You can also eschew printing a STRING, and simply refresh the screen as per your specification, without touching the framebuffer:\n"
	    "\t-s, --refresh top=NUM,left=NUM,width=NUM,height=NUM,wfm=NAME\n"
	    "\n"
	    "EXAMPLES:\n"
	    "\tfbink -s top=20,left=10,width=500,heigh=600,wfm=GC16\n"
	    "\t\tRefreshes a 500x600 rectangle with its top-left corner at coordinates (10, 20) with a GC16 waveform mode.\n"
	    "\n"
	    "NOTES:\n"
	    "\tThe specified rectangle *must* completely fit on screen, or the ioctl will fail.\n"
#ifndef FBINK_FOR_LEGACY
	    "\tAvailable waveform modes: DU, GC16, GC4, A2, GL16, REAGL, REAGLD & AUTO\n"
#	ifdef FBINK_FOR_KINDLE
	    "\t\tAs well as GC16_FAST, GL16_FAST, DU4, GL4, GL16_INV, GCK16 & GLKW16 on some Kindles, depending on the model & FW version.\n"
	    "\t\tUnsupported modes should safely downgrade to AUTO.\n"
#	endif
	    "\tNote that this will also honor --flash\n"
#else
	    "\tNote that specifying a waveform mode is ignored on legacy einkfb devices, because the hardware doesn't expose such capabilities.\n"
	    "\tBut it does (mostly) honor the --flash flag, though.\n"
#endif
	    "\tSpecifying one or more STRING takes precedence over this mode.\n"
#ifdef FBINK_WITH_IMAGE
	    "\n\n"
	    "You can also eschew printing a STRING, and print an IMAGE at the requested coordinates instead:\n"
	    "\t-g, --image file=PATH,x=NUM,y=NUM\n"
	    "\n"
	    "EXAMPLES:\n"
	    "\tfbink -g file=hello.png\n"
	    "\t\tDraws the content of the image file hello.png, starting at the top left of the screen.\n"
	    "\tfbink -i hello,world.png -g x=-10,y=11 -x 5 -y 8\n"
	    "\t\tDraws the content of the image file hello,world.png, starting at the ninth line plus 11px and the sixth column minus 10px\n"
	    "\n"
	    "NOTES:\n"
	    "\tSupported image formats: JPEG, PNG, TGA, BMP, GIF & PNM\n"
	    "\t\tNote that, in some cases, exotic encoding settings may not be supported.\n"
	    "\t\tAs an additional quirk, you can't pass paths with commas in it to file. Pass those to the -i, --img flag instead.\n"
#	ifdef FBINK_FOR_LEGACY
	    "\t\tOn your legacy device, some files may also decode badly (i.e., JPEG). PNG files should display properly.\n"
#	endif
	    "\tThis honors --flash, as well as --clear & --invert\n"
	    "\t\tNote that this also honors --col & --row (taking --size into account), in addition to the coordinates you specify.\n"
	    "\t\tThe aim is to make it easier to align small images to text.\n"
	    "\t\tAnd to make pixel-perfect adjustments, you can also specifiy negative values for x & y.\n"
	    "\tSpecifying one or more STRING takes precedence over this mode.\n"
	    "\t--refresh also takes precedence over this mode.\n"
#endif
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
	static const struct option opts[] = { { "row", required_argument, NULL, 'y' },
					      { "col", required_argument, NULL, 'x' },
					      { "invert", no_argument, NULL, 'h' },
					      { "flash", no_argument, NULL, 'f' },
					      { "clear", no_argument, NULL, 'c' },
					      { "centered", no_argument, NULL, 'm' },
					      { "padded", no_argument, NULL, 'p' },
					      { "refresh", required_argument, NULL, 's' },
					      { "size", required_argument, NULL, 'S' },
					      { "font", required_argument, NULL, 'F' },
					      { "verbose", no_argument, NULL, 'v' },
					      { "quiet", no_argument, NULL, 'q' },
					      { "image", required_argument, NULL, 'g' },
					      { "img", required_argument, NULL, 'i' },
					      { NULL, 0, NULL, 0 } };

	FBInkConfig fbink_config = { 0 };

	enum
	{
		TOP_OPT = 0,
		LEFT_OPT,
		WIDTH_OPT,
		HEIGHT_OPT,
		WFM_OPT,
	};
	enum
	{
		FILE_OPT = 0,
		XOFF_OPT,
		YOFF_OPT,
	};
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunknown-pragmas"
#pragma clang diagnostic ignored "-Wunknown-warning-option"
#pragma GCC diagnostic ignored "-Wdiscarded-qualifiers"
#pragma clang diagnostic ignored "-Wincompatible-pointer-types-discards-qualifiers"
	char* const refresh_token[] = { [TOP_OPT] = "top",       [LEFT_OPT] = "left", [WIDTH_OPT] = "width",
					[HEIGHT_OPT] = "height", [WFM_OPT] = "wfm",   NULL };
	char* const image_token[]   = { [FILE_OPT] = "file", [XOFF_OPT] = "x", [YOFF_OPT] = "y", NULL };
#pragma GCC diagnostic pop
	char*     subopts;
	char*     value;
	uint32_t  region_top     = 0;
	uint32_t  region_left    = 0;
	uint32_t  region_width   = 0;
	uint32_t  region_height  = 0;
	char*     region_wfm     = NULL;
	bool      is_refresh     = false;
	char*     image_file     = NULL;
	short int image_x_offset = 0;
	short int image_y_offset = 0;
	bool      is_image       = false;
	int       errfnd         = 0;

	while ((opt = getopt_long(argc, argv, "y:x:hfcmps:S:F:vqg:i:", opts, &opt_index)) != -1) {
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
			case 's':
				subopts = optarg;
				while (*subopts != '\0' && !errfnd) {
					switch (getsubopt(&subopts, refresh_token, &value)) {
						case TOP_OPT:
							region_top = (uint32_t) strtoul(value, NULL, 10);
							break;
						case LEFT_OPT:
							region_left = (uint32_t) strtoul(value, NULL, 10);
							break;
						case WIDTH_OPT:
							region_width = (uint32_t) strtoul(value, NULL, 10);
							break;
						case HEIGHT_OPT:
							region_height = (uint32_t) strtoul(value, NULL, 10);
							break;
						case WFM_OPT:
							if (value == NULL) {
								fprintf(stderr,
									"Missing value for suboption '%s'\n",
									refresh_token[WFM_OPT]);
								errfnd = 1;
								continue;
							}

							region_wfm = value;
							break;
						default:
							fprintf(stderr, "No match found for token: /%s/\n", value);
							errfnd = 1;
							break;
					}
				}
				if (region_height == 0 && region_width == 0 && region_wfm == NULL) {
					fprintf(stderr,
						"Must specify at least '%s', '%s' and '%s'\n",
						refresh_token[HEIGHT_OPT],
						refresh_token[WIDTH_OPT],
						refresh_token[WFM_OPT]);
					errfnd = 1;
				} else {
					is_refresh = true;
				}
				break;
			case 'S':
				fbink_config.fontmult = (short unsigned int) strtoul(optarg, NULL, 10);
				break;
			case 'F':
				if (strcasecmp(optarg, "IBM") == 0) {
					fbink_config.fontname = IBM;
				} else if (strcasecmp(optarg, "UNSCII") == 0) {
					fbink_config.fontname = UNSCII;
				} else if (strcasecmp(optarg, "ALT") == 0) {
					fbink_config.fontname = UNSCII_ALT;
				} else if (strcasecmp(optarg, "THIN") == 0) {
					fbink_config.fontname = UNSCII_THIN;
				} else if (strcasecmp(optarg, "FANTASY") == 0) {
					fbink_config.fontname = UNSCII_FANTASY;
				} else if (strcasecmp(optarg, "MCR") == 0) {
					fbink_config.fontname = UNSCII_MCR;
				} else if (strcasecmp(optarg, "TALL") == 0) {
					fbink_config.fontname = UNSCII_TALL;
				} else {
					fprintf(stderr, "Unknown font name '%s'.\n", optarg);
					errfnd = 1;
				}
				break;
			case 'v':
				fbink_config.is_verbose = !fbink_config.is_verbose;
				break;
			case 'q':
				fbink_config.is_quiet = !fbink_config.is_quiet;
				break;
			case 'g':
				subopts = optarg;
				while (*subopts != '\0' && !errfnd) {
					switch (getsubopt(&subopts, image_token, &value)) {
						case FILE_OPT:
							// NOTE: As a hack to support paths with a comma,
							//       which we can't handle via getsubopt,
							//       we have *two* ways of specifying image_file,
							//       so make sure we're not dup'ing twice...
							free(image_file);
							image_file = strdup(value);
							break;
						case XOFF_OPT:
							image_x_offset = (short int) atoi(value);
							break;
						case YOFF_OPT:
							image_y_offset = (short int) atoi(value);
							break;
						default:
							fprintf(stderr, "No match found for token: /%s/\n", value);
							errfnd = 1;
							break;
					}
				}
				if (image_file == NULL) {
					fprintf(stderr, "Must specify at least '%s'\n", image_token[FILE_OPT]);
					errfnd = 1;
				} else {
					is_image = true;
				}
				break;
			case 'i':
				// Free a potentially previously set value...
				free(image_file);
				image_file = strdup(optarg);
				break;
			default:
				fprintf(stderr, "?? Unknown option code 0%o ??\n", (unsigned int) opt);
				errfnd = 1;
				break;
		}
	}

	if (errfnd == 1 || argc == 1) {
		show_helpmsg();
		return EXIT_FAILURE;
	}

	// Open framebuffer and keep it around, then setup globals.
	int fbfd = -1;
	if (-1 == (fbfd = fbink_open())) {
		fprintf(stderr, "Failed to open the framebuffer, aborting . . .\n");
		return EXIT_FAILURE;
	}
	if (EXIT_FAILURE == (fbfd = fbink_init(fbfd, &fbink_config))) {
		fprintf(stderr, "Failed to initialize FBInk, aborting . . .\n");
		return EXIT_FAILURE;
	}

	char* string;
	if (optind < argc) {
		while (optind < argc) {
			int linecount = -1;
			string        = argv[optind++];
			// NOTE: This is probably the point where we'd be validating/converting string to UTF-8,
			//       if we had an easy way to... (c.f., my rant about Kobo's broken libc in fbink_internal.h)
			if (!fbink_config.is_quiet) {
				printf(
				    "Printing string '%s' @ column %hd, row %hd (inverted: %s, flashing: %s, centered: %s, left padded: %s, clear screen: %s, font scaling: x%hu)\n",
				    string,
				    fbink_config.col,
				    fbink_config.row,
				    fbink_config.is_inverted ? "true" : "false",
				    fbink_config.is_flashing ? "true" : "false",
				    fbink_config.is_centered ? "true" : "false",
				    fbink_config.is_padded ? "true" : "false",
				    fbink_config.is_cleared ? "true" : "false",
				    fbink_config.fontmult);
			}
			if ((linecount = fbink_print(fbfd, string, &fbink_config)) < 0) {
				fprintf(stderr, "Failed to print that string!\n");
			}
			// NOTE: Don't clobber previous entries if multiple strings were passed...
			//       We make sure to trust print's return value,
			//       because it knows how much space it already took up ;).
			fbink_config.row = (short int) (fbink_config.row + linecount);
			// NOTE: By design, if you ask for a clear screen, only the final print will stay on screen ;).
		}
	} else {
		if (is_refresh) {
			printf(
			    "Refreshing the screen from top=%u, left=%u for width=%u, height=%u with %swaveform mode %s\n",
			    region_top,
			    region_left,
			    region_width,
			    region_height,
			    fbink_config.is_flashing ? "a flashing " : "",
			    region_wfm);
			if (fbink_refresh(fbfd,
					  region_top,
					  region_left,
					  region_width,
					  region_height,
					  region_wfm,
					  fbink_config.is_flashing) != EXIT_SUCCESS) {
				fprintf(stderr, "Failed to refresh the screen as per your specification!\n");
			}
		} else if (is_image) {
			printf("Displaying image '%s' @ (%hd, %hd)\n", image_file, image_x_offset, image_y_offset);
			if (fbink_print_image(fbfd, image_file, image_x_offset, image_y_offset, &fbink_config) < 0) {
				fprintf(stderr, "Failed to display that image!\n");
			}
		} else {
			show_helpmsg();
		}
	}

	// Cleanup
	if (g_fbink_isFbMapped) {
		munmap(g_fbink_fbp, g_fbink_screensize);
	}
	close(fbfd);
	free(image_file);

	return EXIT_SUCCESS;
}
