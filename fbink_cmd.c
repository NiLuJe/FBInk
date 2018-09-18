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
	    "\t\tPrints 'Hello World!' on the eleventh line, starting at the second column from the left.\n"
	    "\tfbink -pmh -y -5 \"Hello World!\"\n"
	    "\t\tPrints 'Hello World!', highlighted (i.e., white on black with the default colors), centered & padded on both sides, on the fifth line starting from the bottom.\n"
	    "\tfbink -pmM -y -8 \"Hello World!\"\n"
	    "\t\tPrints 'Hello World!', centered & padded on both sides, eight lines above the center of the screen.\n"
	    "\n"
	    "Options affecting the message's position on screen:\n"
	    "\t-x, --col NUM\t\tBegin printing STRING @ column NUM (Default: 0).\n"
	    "\t\t\t\tBeginning at column 1 instead of 0 is recommended, because column 0 (the leftmost one) is often half-obscured by a bezel.\n"
	    "\t\t\t\tUse a negative value to count back from the right edge of the screen.\n"
	    "\t-y, --row NUM\t\tBegin printing STRING @ row NUM (Default: 0).\n"
	    "\t\t\t\tBeginning at row 1 instead of 0 is recommended, because row 0 (the topmost one) is often half-obscured by a bezel, especially on Kobos.\n"
	    "\t\t\t\tUse a negative value to count back from the bottom of the screen.\n"
	    "\t-X, --hoffset NUM\tAdjust final text position by NUM pixels on the horizontal axis (Default: 0).\n"
	    "\t\t\t\tHonors negative values, and will let you push stuff off-screen, often without warning.\n"
	    "\t-Y, --voffset NUM\tAdjust final text position by NUM pixels on the vertical axis (Default: 0).\n"
	    "\t\t\t\tHonors negative values, and will let you push stuff off-screen, often without warning.\n"
	    "\t-m, --centered\t\tDynamically override col to print STRING at the center of the screen.\n"
	    "\t\t\t\tSpecial care is taken to avoid the very edges of the screen, to ensure the complete legibility of the message.\n"
	    "\t-M, --halfway\t\tDynamically adjust row to print STRING in the middle of the screen.\n"
	    "\t\t\t\tThe value specified in row then becomes an offset, starting from the middle of the screen.\n"
	    "\t-p, --padded\t\tLeft pad STRING with blank spaces.\n"
	    "\t\t\t\tMost useful when combined with --centered to ensure a line will be completely filled, while still centering STRING,\n"
	    "\t\t\t\ti.e., padding it on both sides.\n"
	    "\n"
	    "Options affecting the message's appearance:\n"
	    "\t-h, --invert\t\tPrint STRING in <background color> over <foreground color> instead of the reverse.\n"
	    "\t-f, --flash\t\tAsk the eInk driver to do a black flash when refreshing the area of the screen where STRING will be printed.\n"
	    "\t-c, --clear\t\tFully clear the screen before printing (obeys --invert).\n"
	    "\t-S, --size\t\tOverride the automatic font scaling multiplier (Default: 0, automatic selection, ranging from 1 (no scaling), to 4 (4x upscaling), depending on screen resolution).\n"
#ifdef FBINK_WITH_FONTS
	    "\t\t\t\tNote that user-supplied values will be clamped to safe boundaries (from 1 to around 45 for most fonts, and from 1 to around 30 for TALL).\n"
#else
	    "\t\t\t\tNote that user-supplied values will be clamped to safe boundaries (from 1 to around 45).\n"
#endif
	    "\t\t\t\tThe exact upper value depends on the resolution of your screen.\n"
	    "\t-F, --font NAME\t\tRender glyphs from builtin font NAME (Default: IBM).\n"
#ifdef FBINK_WITH_FONTS
	    "\t\t\t\tAvailable font families: IBM, UNSCII, ALT, THIN, FANTASY, MCR, TALL, BLOCK,\n"
	    "\t\t\t\t\t\tLEGGIE, VEGGIE, KATES, FKP, CTRLD, ORP, ORPB, ORPI, SCIENTIFICA, SCIENTIFICAB, SCIENTIFICAI\n"
#else
	    "\t\t\t\tAvailable font families: IBM\n"
#endif
	    "\t-C, --color NAME\tColor the text will be printed in (Default: BLACK).\n"
	    "\t-B, --background NAME\tColor of the background the text will be printed on (Default: WHITE).\n"
	    "\t\t\t\tAvailable colors: BLACK, GRAY1, GRAY2, GRAY3, GRAY4, GRAY5, GRAY6, GRAY7,\n"
	    "\t\t\t\t\t\tGRAY8, GRAY9, GRAYA, GRAYB, GRAYC, GRAYD, GRAYE, WHITE\n"
	    "\t-o, --overlay\t\tDon't draw background pixels, and compute foreground pixel color based on the inverse of the underlying framebufer pixel.\n"
	    "\t\t\t\tObviously ignores -h, --invert & -C, --color *as far as glyphs are concerned*.\n"
	    "\t\t\t\t-B, --background is still honored if you combine this with -c, --clear\n"
	    "\t-O, --bgless\t\tDon't draw background pixels.\n"
	    "\t\t\t\tObviously mutually exclusive with -o, --overlay, because it's simply a subset of what overlay does.\n"
	    "\n"
	    "Options affecting the program's verbosity:\n"
	    "\t-v, --verbose\tToggle printing diagnostic messages.\n"
	    "\t-q, --quiet\tToggle hiding hardware setup messages.\n"
	    "\n"
	    "Options affecting the program's behavior:\n"
	    "\t-I, --interactive\tEnter a very basic interactive mode.\n"
	    "\t-L, --linecountcode\tWhen successfully printing text, returns the total amount of printed lines as the process exit code.\n"
	    "\t-l, --linecount\t\tWhen successfully printing text, outputs the total amount of printed lines in the final line of output to stdout (NOTE: enforces quiet & non-verbose!).\n"
	    "\t-P, --progressbar NUM\tDraw a NUM%% full progress bar (full-width). Like other alternative modes, does *NOT* have precedence over text printing.\n"
	    "\t\t\t\tIgnores -o, --overlay; -x, --col; -X, --hoffset; as well as -m, --centered & -p, --padded\n"
	    "\t-A, --activitybar NUM\tDraw an activity bar on step NUM (full-width). NUM must be between 0 and 16. Like other alternative modes, does *NOT* have precedence over text printing.\n"
	    "\t\t\t\tNOTE: If NUM is negative, will cycle between each possible value every 750ms, until the death of the sun! Be careful not to be caught in an involuntary infinite loop!\n"
	    "\t\t\t\tIgnores -x, --col; -X, --hoffset; as well as -m, --centered & -p, --padded\n"
	    "\t-V, --noviewport\tIgnore any & all viewport corrections, be it from Kobo devices with rows of pixels hidden by a bezel, or a dynamic offset applied to rows when vertical fit isn't perfect.\n"
	    "\n"
	    "NOTES:\n"
	    "\tYou can specify multiple STRINGs in a single invocation of fbink, each consecutive one will be printed on the subsequent line.\n"
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
	    "\tfbink -s top=20,left=10,width=500,height=600,wfm=GC16\n"
	    "\t\tRefreshes a 500x600 rectangle with its top-left corner at coordinates (10, 20) with a GC16 waveform mode.\n"
	    "\n"
	    "NOTES:\n"
	    "\tThe specified rectangle *must* completely fit on screen, or the ioctl will fail.\n"
	    "\tAvailable waveform modes: DU, GC16, GC4, A2, GL16, REAGL, REAGLD & AUTO\n"
#ifdef FBINK_FOR_KINDLE
	    "\t\tAs well as GC16_FAST, GL16_FAST, DU4, GL4, GL16_INV, GCK16 & GLKW16 on some Kindles, depending on the model & FW version.\n"
	    "\t\tUnsupported modes should safely downgrade to AUTO.\n"
#endif
	    "\tNote that this will also honor --flash\n"
#ifdef FBINK_FOR_KINDLE
	    "\tNote that specifying a waveform mode is ignored on legacy einkfb devices, because the hardware doesn't expose such capabilities.\n"
	    "\tBut it does (mostly) honor the --flash flag, though.\n"
#endif
	    "\tSpecifying one or more STRING takes precedence over this mode.\n"
#ifdef FBINK_WITH_IMAGE
	    "\n\n"
	    "You can also eschew printing a STRING, and print an IMAGE at the requested coordinates instead:\n"
	    "\t-g, --image file=PATH,x=NUM,y=NUM,halign=ALIGN,valign=ALIGN\n"
	    "\t\tSupported ALIGN values: NONE (or LEFT for halign, TOP for valign), CENTER or MIDDLE, EDGE (or RIGHT for halign, BOTTOM for valign)\n"
	    "\n"
	    "EXAMPLES:\n"
	    "\tfbink -g file=hello.png\n"
	    "\t\tDisplays the image \"hello.png\", starting at the top left of the screen.\n"
	    "\tfbink -i hello,world.png -g x=-10,y=11 -x 5 -y 8\n"
	    "\t\tDisplays the image \"hello,world.png\", starting at the ninth line plus 11px and the sixth column minus 10px\n"
	    "\tfbink -g file=hello.png,halign=EDGE,valign=CENTER\n"
	    "\t\tDisplays the image \"hello.png\", in the middle of the screen, aligned to the right edge.\n"
	    "\n"
	    "Options affecting the image's appearance:\n"
	    "\t-a, --flatten\tIgnore the alpha channel.\n"
	    "\n"
	    "NOTES:\n"
	    "\tSupported image formats: JPEG, PNG, TGA, BMP, GIF & PNM\n"
	    "\t\tNote that, in some cases, exotic encoding settings may not be supported.\n"
	    "\t\tTransparency is supported, but it may be slightly slower (because we may need to do alpha blending).\n"
	    "\t\t\tYou can use the --flatten flag to avoid the potential performance penalty by always ignoring alpha.\n"
	    "\t\tAs an additional quirk, you can't pass paths with commas in it to file. Pass those to the -i, --img flag instead.\n"
	    "\t\tAnd if you want to read image data from stdin, make sure to pass \"-\" as the file name.\n"
	    "\tThis honors --flash, as well as --clear & --invert\n"
	    "\t\tNote that this also honors --col & --row (taking --size into account), in addition to the coordinates you specify.\n"
	    "\t\tThe aim is to make it easier to align small images to text.\n"
	    "\t\tAnd to make pixel-perfect adjustments, you can also specifiy negative values for x & y.\n"
	    "\tSpecifying one or more STRING takes precedence over this mode.\n"
	    "\t--refresh also takes precedence over this mode.\n"
#endif
	    "\n"
	    "NOTES:\n"
	    "\tShell script writers can also use the -e, --eval flag to have fbink just spit out a few of its internal state variables to stdout,\n"
	    "\t\tf.g., eval $(fbink -qe)\n"
	    "\n",
	    fbink_version());

	return;
}

// Truly infinite progress bar
// NOTE: Punted off to a dedicated function to workaround an amazingly weird & obscure performance issue:
//       keeping this inlined in main massively tanks *image* processing performance (by ~50%!),
//       when built w/ LTO... o_O.
static int
    do_infinite_progress_bar(int fbfd, const FBInkConfig* fbink_config)
{
	int rv = EXIT_SUCCESS;

	const struct timespec zzz = { 0L, 750000000L };
	for (;;) {
		for (uint8_t i = 0; i < 16; i++) {
			rv = fbink_print_activity_bar(fbfd, i, fbink_config);
			if (rv != EXIT_SUCCESS) {
				break;
			}
			nanosleep(&zzz, NULL);
		}
		for (uint8_t i = 16; i > 0; i--) {
			rv = fbink_print_activity_bar(fbfd, i, fbink_config);
			if (rv != EXIT_SUCCESS) {
				break;
			}
			nanosleep(&zzz, NULL);
		}
	}

	return rv;
}

// Application entry point
int
    main(int argc, char* argv[])
{
	int                        opt;
	int                        opt_index;
	static const struct option opts[] = { { "row", required_argument, NULL, 'y' },
					      { "col", required_argument, NULL, 'x' },
					      { "voffset", required_argument, NULL, 'Y' },
					      { "hoffset", required_argument, NULL, 'X' },
					      { "invert", no_argument, NULL, 'h' },
					      { "flash", no_argument, NULL, 'f' },
					      { "clear", no_argument, NULL, 'c' },
					      { "centered", no_argument, NULL, 'm' },
					      { "halfway", no_argument, NULL, 'M' },
					      { "padded", no_argument, NULL, 'p' },
					      { "refresh", required_argument, NULL, 's' },
					      { "size", required_argument, NULL, 'S' },
					      { "font", required_argument, NULL, 'F' },
					      { "verbose", no_argument, NULL, 'v' },
					      { "quiet", no_argument, NULL, 'q' },
					      { "image", required_argument, NULL, 'g' },
					      { "img", required_argument, NULL, 'i' },
					      { "flatten", no_argument, NULL, 'a' },
					      { "eval", no_argument, NULL, 'e' },
					      { "interactive", no_argument, NULL, 'I' },
					      { "color", required_argument, NULL, 'C' },
					      { "background", required_argument, NULL, 'B' },
					      { "linecountcode", no_argument, NULL, 'L' },
					      { "linecount", no_argument, NULL, 'l' },
					      { "progressbar", required_argument, NULL, 'P' },
					      { "activitybar", required_argument, NULL, 'A' },
					      { "noviewport", no_argument, NULL, 'V' },
					      { "overlay", no_argument, NULL, 'o' },
					      { "bgless", no_argument, NULL, 'O' },
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
		HALIGN_OPT,
		VALIGN_OPT,
	};
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunknown-pragmas"
#pragma clang diagnostic ignored "-Wunknown-warning-option"
#pragma GCC diagnostic ignored "-Wdiscarded-qualifiers"
#pragma clang diagnostic ignored "-Wincompatible-pointer-types-discards-qualifiers"
	char* const refresh_token[] = { [TOP_OPT] = "top",       [LEFT_OPT] = "left", [WIDTH_OPT] = "width",
					[HEIGHT_OPT] = "height", [WFM_OPT] = "wfm",   NULL };
	char* const image_token[]   = { [FILE_OPT] = "file",     [XOFF_OPT] = "x",        [YOFF_OPT] = "y",
                                      [HALIGN_OPT] = "halign", [VALIGN_OPT] = "valign", NULL };
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
	bool      is_eval        = false;
	bool      is_interactive = false;
	bool      want_linecode  = false;
	bool      want_linecount = false;
	bool      is_progressbar = false;
	bool      is_activitybar = false;
	bool      is_infinite    = false;
	uint8_t   progress       = 0;
	int       errfnd         = 0;

	while ((opt = getopt_long(argc, argv, "y:x:Y:X:hfcmMps:S:F:vqg:i:aeIC:B:LlP:A:oOV", opts, &opt_index)) != -1) {
		switch (opt) {
			case 'y':
				fbink_config.row = (short int) atoi(optarg);
				break;
			case 'x':
				fbink_config.col = (short int) atoi(optarg);
				break;
			case 'Y':
				fbink_config.voffset = (short int) atoi(optarg);
				break;
			case 'X':
				fbink_config.hoffset = (short int) atoi(optarg);
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
			case 'M':
				fbink_config.is_halfway = true;
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
				fbink_config.fontmult = (uint8_t) strtoul(optarg, NULL, 10);
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
				} else if (strcasecmp(optarg, "BLOCK") == 0) {
					fbink_config.fontname = BLOCK;
				} else if (strcasecmp(optarg, "LEGGIE") == 0) {
					fbink_config.fontname = LEGGIE;
				} else if (strcasecmp(optarg, "VEGGIE") == 0) {
					fbink_config.fontname = VEGGIE;
				} else if (strcasecmp(optarg, "KATES") == 0) {
					fbink_config.fontname = KATES;
				} else if (strcasecmp(optarg, "FKP") == 0) {
					fbink_config.fontname = FKP;
				} else if (strcasecmp(optarg, "CTRLD") == 0) {
					fbink_config.fontname = CTRLD;
				} else if (strcasecmp(optarg, "ORP") == 0) {
					fbink_config.fontname = ORP;
				} else if (strcasecmp(optarg, "ORPB") == 0) {
					fbink_config.fontname = ORPB;
				} else if (strcasecmp(optarg, "ORPI") == 0) {
					fbink_config.fontname = ORPI;
				} else if (strcasecmp(optarg, "SCIENTIFICA") == 0) {
					fbink_config.fontname = SCIENTIFICA;
				} else if (strcasecmp(optarg, "SCIENTIFICAB") == 0) {
					fbink_config.fontname = SCIENTIFICAB;
				} else if (strcasecmp(optarg, "SCIENTIFICAI") == 0) {
					fbink_config.fontname = SCIENTIFICAI;
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
				// NOTE: I'm not terribly fond of getsubopt in general, especially here with the comma limitation
				//       for filenames, but it does make sense to keep image-specific options separate...
				//       The same argument could be made against moving x_off/y_off to hoffset/voffset now
				//       that we have those in FBInkConfig... (Plus, getting rid of that would break ABI compat).
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
						case HALIGN_OPT:
							if (strcasecmp(value, "NONE") == 0 ||
							    strcasecmp(value, "LEFT") == 0) {
								fbink_config.halign = NONE;
							} else if (strcasecmp(value, "CENTER") == 0 ||
								   strcasecmp(value, "MIDDLE") == 0) {
								fbink_config.halign = CENTER;
							} else if (strcasecmp(value, "EDGE") == 0 ||
								   strcasecmp(value, "RIGHT") == 0) {
								fbink_config.halign = EDGE;
							} else {
								fprintf(stderr, "Unknown alignment value '%s'.\n", value);
								errfnd = 1;
							}
							break;
						case VALIGN_OPT:
							if (strcasecmp(value, "NONE") == 0 ||
							    strcasecmp(value, "TOP") == 0) {
								fbink_config.valign = NONE;
							} else if (strcasecmp(value, "CENTER") == 0 ||
								   strcasecmp(value, "MIDDLE") == 0) {
								fbink_config.valign = CENTER;
							} else if (strcasecmp(value, "EDGE") == 0 ||
								   strcasecmp(value, "BOTTOM") == 0) {
								fbink_config.valign = EDGE;
							} else {
								fprintf(stderr, "Unknown alignment value '%s'.\n", value);
								errfnd = 1;
							}
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
			case 'a':
				fbink_config.ignore_alpha = true;
				break;
			case 'e':
				is_eval = true;
				break;
			case 'I':
				is_interactive = true;
				break;
			case 'C':
				if (strcasecmp(optarg, "BLACK") == 0) {
					fbink_config.fg_color = FG_BLACK;
				} else if (strcasecmp(optarg, "GRAY1") == 0) {
					fbink_config.fg_color = FG_GRAY1;
				} else if (strcasecmp(optarg, "GRAY2") == 0) {
					fbink_config.fg_color = FG_GRAY2;
				} else if (strcasecmp(optarg, "GRAY3") == 0) {
					fbink_config.fg_color = FG_GRAY3;
				} else if (strcasecmp(optarg, "GRAY4") == 0) {
					fbink_config.fg_color = FG_GRAY4;
				} else if (strcasecmp(optarg, "GRAY5") == 0) {
					fbink_config.fg_color = FG_GRAY5;
				} else if (strcasecmp(optarg, "GRAY6") == 0) {
					fbink_config.fg_color = FG_GRAY6;
				} else if (strcasecmp(optarg, "GRAY7") == 0) {
					fbink_config.fg_color = FG_GRAY7;
				} else if (strcasecmp(optarg, "GRAY8") == 0) {
					fbink_config.fg_color = FG_GRAY8;
				} else if (strcasecmp(optarg, "GRAY9") == 0) {
					fbink_config.fg_color = FG_GRAY9;
				} else if (strcasecmp(optarg, "GRAYA") == 0) {
					fbink_config.fg_color = FG_GRAYA;
				} else if (strcasecmp(optarg, "GRAYB") == 0) {
					fbink_config.fg_color = FG_GRAYB;
				} else if (strcasecmp(optarg, "GRAYC") == 0) {
					fbink_config.fg_color = FG_GRAYC;
				} else if (strcasecmp(optarg, "GRAYD") == 0) {
					fbink_config.fg_color = FG_GRAYD;
				} else if (strcasecmp(optarg, "GRAYE") == 0) {
					fbink_config.fg_color = FG_GRAYE;
				} else if (strcasecmp(optarg, "WHITE") == 0) {
					fbink_config.fg_color = FG_WHITE;
				} else {
					fprintf(stderr, "Unknown color name '%s'.\n", optarg);
					errfnd = 1;
				}
				break;
			case 'B':
				if (strcasecmp(optarg, "BLACK") == 0) {
					fbink_config.bg_color = BG_BLACK;
				} else if (strcasecmp(optarg, "GRAY1") == 0) {
					fbink_config.bg_color = BG_GRAY1;
				} else if (strcasecmp(optarg, "GRAY2") == 0) {
					fbink_config.bg_color = BG_GRAY2;
				} else if (strcasecmp(optarg, "GRAY3") == 0) {
					fbink_config.bg_color = BG_GRAY3;
				} else if (strcasecmp(optarg, "GRAY4") == 0) {
					fbink_config.bg_color = BG_GRAY4;
				} else if (strcasecmp(optarg, "GRAY5") == 0) {
					fbink_config.bg_color = BG_GRAY5;
				} else if (strcasecmp(optarg, "GRAY6") == 0) {
					fbink_config.bg_color = BG_GRAY6;
				} else if (strcasecmp(optarg, "GRAY7") == 0) {
					fbink_config.bg_color = BG_GRAY7;
				} else if (strcasecmp(optarg, "GRAY8") == 0) {
					fbink_config.bg_color = BG_GRAY8;
				} else if (strcasecmp(optarg, "GRAY9") == 0) {
					fbink_config.bg_color = BG_GRAY9;
				} else if (strcasecmp(optarg, "GRAYA") == 0) {
					fbink_config.bg_color = BG_GRAYA;
				} else if (strcasecmp(optarg, "GRAYB") == 0) {
					fbink_config.bg_color = BG_GRAYB;
				} else if (strcasecmp(optarg, "GRAYC") == 0) {
					fbink_config.bg_color = BG_GRAYC;
				} else if (strcasecmp(optarg, "GRAYD") == 0) {
					fbink_config.bg_color = BG_GRAYD;
				} else if (strcasecmp(optarg, "GRAYE") == 0) {
					fbink_config.bg_color = BG_GRAYE;
				} else if (strcasecmp(optarg, "WHITE") == 0) {
					fbink_config.bg_color = BG_WHITE;
				} else {
					fprintf(stderr, "Unknown color name '%s'.\n", optarg);
					errfnd = 1;
				}
				break;
			case 'L':
				want_linecode = true;
				break;
			case 'l':
				want_linecount = true;
				break;
			case 'P':
				is_progressbar = true;
				progress       = (uint8_t) strtoul(optarg, NULL, 10);
				break;
			case 'A':
				is_activitybar = true;
				int8_t val     = (int8_t) atoi(optarg);
				if (val < 0) {
					is_infinite = true;
				} else {
					progress = (uint8_t) val;
				}
				break;
			case 'o':
				fbink_config.is_overlay = true;
				break;
			case 'O':
				fbink_config.is_bgless = true;
				break;
			case 'V':
				fbink_config.ignore_viewport = true;
				break;
			default:
				fprintf(stderr, "?? Unknown option code 0%o ??\n", (unsigned int) opt);
				errfnd = 1;
				break;
		}
	}

	// Enforce quiet output when asking for want_linecount, to avoid polluting the output...
	if (want_linecount) {
		fbink_config.is_quiet   = true;
		fbink_config.is_verbose = false;
	}

	// Assume success, until shit happens ;)
	int rv = EXIT_SUCCESS;
	// Declare it a tiny bit early to make cleanup handling safe
	// (fbink_close is safe to call with fbfd set to -1 and/or the mmap not actually done).
	int fbfd = -1;

	// Don't abort if we piped something without passing any arguments!
	if (errfnd == 1 || (argc == 1 && isatty(fileno(stdin)))) {
		show_helpmsg();
		rv = ERRCODE(EXIT_FAILURE);
		goto cleanup;
	}

	// Open framebuffer and keep it around, then setup globals.
	if (ERRCODE(EXIT_FAILURE) == (fbfd = fbink_open())) {
		fprintf(stderr, "Failed to open the framebuffer, aborting . . .\n");
		rv = ERRCODE(EXIT_FAILURE);
		goto cleanup;
	}
	if (fbink_init(fbfd, &fbink_config) == ERRCODE(EXIT_FAILURE)) {
		fprintf(stderr, "Failed to initialize FBInk, aborting . . .\n");
		rv = ERRCODE(EXIT_FAILURE);
		goto cleanup;
	}

	char* string;
	if (optind < argc) {
		unsigned short int total_lines = 0U;
		while (optind < argc) {
			int linecount = -1;
			string        = argv[optind++];
			// NOTE: This is probably the point where we'd be validating/converting string to UTF-8,
			//       if we had an easy way to... (c.f., my rant about Kobo's broken libc in fbink_internal.h)
			if (!fbink_config.is_quiet) {
				printf(
				    "Printing string '%s' @ column %hd + %hdpx, row %hd + %hdpx (overlay: %s, backgroundless: %s, inverted: %s, flashing: %s, centered: %s, left padded: %s, clear screen: %s, font: %hhu, font scaling: x%hhu)\n",
				    string,
				    fbink_config.col,
				    fbink_config.hoffset,
				    fbink_config.row,
				    fbink_config.voffset,
				    fbink_config.is_overlay ? "true" : "false",
				    fbink_config.is_bgless ? "true" : "false",
				    fbink_config.is_inverted ? "true" : "false",
				    fbink_config.is_flashing ? "true" : "false",
				    fbink_config.is_centered ? "true" : "false",
				    fbink_config.is_padded ? "true" : "false",
				    fbink_config.is_cleared ? "true" : "false",
				    fbink_config.fontname,
				    fbink_config.fontmult);
			}
			if ((linecount = fbink_print(fbfd, string, &fbink_config)) < 0) {
				fprintf(stderr, "Failed to print that string!\n");
				rv = ERRCODE(EXIT_FAILURE);
				goto cleanup;
			}
			// NOTE: Don't clobber previous entries if multiple strings were passed...
			//       We make sure to trust print's return value,
			//       because it knows how much space it already took up ;).
			fbink_config.row = (short int) (fbink_config.row + linecount);
			// NOTE: By design, if you ask for a clear screen, only the final print will stay on screen ;).

			// If we were asked to return the amount of printed lines, honor that,
			// provided we actually successfully printed something...
			if (want_linecode && rv >= EXIT_SUCCESS) {
				rv += linecount;
			}
			if (want_linecount && rv >= EXIT_SUCCESS) {
				total_lines = (unsigned short int) (total_lines + linecount);
			}
		}
		// And print the total amount of lines we printed, if requested...
		if (want_linecount) {
			if (rv == ERRCODE(EXIT_FAILURE)) {
				printf("0");
			} else {
				printf("%u", total_lines);
			}
		}
	} else {
		if (is_refresh) {
			if (!fbink_config.is_quiet) {
				printf(
				    "Refreshing the screen from top=%u, left=%u for width=%u, height=%u with %swaveform mode %s\n",
				    region_top,
				    region_left,
				    region_width,
				    region_height,
				    fbink_config.is_flashing ? "a flashing " : "",
				    region_wfm);
			}
			if (fbink_refresh(fbfd,
					  region_top,
					  region_left,
					  region_width,
					  region_height,
					  region_wfm,
					  fbink_config.is_flashing) != EXIT_SUCCESS) {
				fprintf(stderr, "Failed to refresh the screen as per your specification!\n");
				rv = ERRCODE(EXIT_FAILURE);
				goto cleanup;
			}
		} else if (is_image) {
			if (!fbink_config.is_quiet) {
				printf(
				    "Displaying image '%s' @ column %hd + %hdpx, row %hd + %dpx (halign: %hhu, valign: %hhu, inverted: %s, flattened: %s)\n",
				    image_file,
				    fbink_config.col,
				    image_x_offset,
				    fbink_config.row,
				    image_y_offset,
				    fbink_config.halign,
				    fbink_config.valign,
				    fbink_config.is_inverted ? "true" : "false",
				    fbink_config.ignore_alpha ? "true" : "false");
			}
			if (fbink_print_image(fbfd, image_file, image_x_offset, image_y_offset, &fbink_config) !=
			    EXIT_SUCCESS) {
				fprintf(stderr, "Failed to display that image!\n");
				rv = ERRCODE(EXIT_FAILURE);
				goto cleanup;
			}
		} else if (is_progressbar) {
			if (!fbink_config.is_quiet) {
				printf(
				    "Displaying a %hhu%% full progress bar @ row %hd + %hdpx (inverted: %s, flashing: %s, clear screen: %s, font: %hhu, font scaling: x%hhu)\n",
				    progress,
				    fbink_config.row,
				    fbink_config.voffset,
				    fbink_config.is_inverted ? "true" : "false",
				    fbink_config.is_flashing ? "true" : "false",
				    fbink_config.is_cleared ? "true" : "false",
				    fbink_config.fontname,
				    fbink_config.fontmult);
			}
			if (fbink_print_progress_bar(fbfd, progress, &fbink_config) != EXIT_SUCCESS) {
				fprintf(stderr, "Failed to display a progressbar!\n");
				rv = ERRCODE(EXIT_FAILURE);
				goto cleanup;
			}
		} else if (is_activitybar) {
			// Were we asked to loop forever?
			if (is_infinite) {
				if (!fbink_config.is_quiet) {
					printf(
					    "Displaying an activity bar cycling forever @ row %hd + %hdpx (inverted: %s, flashing: %s, clear screen: %s)\n",
					    fbink_config.row,
					    fbink_config.voffset,
					    fbink_config.is_inverted ? "true" : "false",
					    fbink_config.is_flashing ? "true" : "false",
					    fbink_config.is_cleared ? "true" : "false");
				}
				// NOTE: In a dedicated function,
				//       because keeping it inline massively tanks performance in the image codepath,
				//       for an amazingly weird LTO-related reason :?
				if (do_infinite_progress_bar(fbfd, &fbink_config) != EXIT_SUCCESS) {
					fprintf(stderr, "Failed to display a progressbar!\n");
					rv = ERRCODE(EXIT_FAILURE);
					goto cleanup;
				}
			} else {
				if (!fbink_config.is_quiet) {
					printf(
					    "Displaying an activity bar on step %hhu @ row %hd + %hdpx (inverted: %s, flashing: %s, clear screen: %s)\n",
					    progress,
					    fbink_config.row,
					    fbink_config.voffset,
					    fbink_config.is_inverted ? "true" : "false",
					    fbink_config.is_flashing ? "true" : "false",
					    fbink_config.is_cleared ? "true" : "false");
				}
				if (fbink_print_activity_bar(fbfd, progress, &fbink_config) != EXIT_SUCCESS) {
					fprintf(stderr, "Failed to display an activitybar!\n");
					rv = ERRCODE(EXIT_FAILURE);
					goto cleanup;
				}
			}
		} else if (is_eval) {
			fbink_state_dump(&fbink_config);
		} else if (is_interactive && isatty(fileno(stdin))) {
			// We asked for interactive mode, and we're really running from a terminal, so, go ahead.
			char*   line = NULL;
			size_t  len  = 0;
			ssize_t nread;
			int     linecnt = -1;
			// Draw a poor man's prompt, which works fairly okay, until we enable verbose mode :D.
			printf(">>> ");
			while ((nread = getline(&line, &len, stdin)) != -1) {
				printf(">>> ");
				if ((linecnt = fbink_print(fbfd, line, &fbink_config)) < 0) {
					fprintf(stderr, "Failed to print that string!\n");
					rv = ERRCODE(EXIT_FAILURE);
				}
				fbink_config.row = (short int) (fbink_config.row + linecnt);
			}
			free(line);
		} else {
			// If all else failed, try reading from stdin, provided we're not running from a terminal ;).
			if (!isatty(fileno(stdin))) {
				char*   line = NULL;
				size_t  len  = 0;
				ssize_t nread;
				int     linecnt = -1;
				while ((nread = getline(&line, &len, stdin)) != -1) {
					if ((linecnt = fbink_print(fbfd, line, &fbink_config)) < 0) {
						fprintf(stderr, "Failed to print that string!\n");
						rv = ERRCODE(EXIT_FAILURE);
					}
					fbink_config.row = (short int) (fbink_config.row + linecnt);
				}
				free(line);

				// If nothing was read, show the help
				if (linecnt == -1) {
					show_helpmsg();
				}
			} else {
				show_helpmsg();
			}
		}
	}

	// Cleanup
cleanup:
	free(image_file);
	if (fbink_close(fbfd) == ERRCODE(EXIT_FAILURE)) {
		fprintf(stderr, "Failed to close the framebuffer, aborting . . .\n");
		rv = ERRCODE(EXIT_FAILURE);
	}

	return rv;
}
