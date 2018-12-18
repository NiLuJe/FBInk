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
	    "\t\t\t\tYou might consider beginning at column 1 instead of 0, as column 0 (the leftmost one) may sometimes be slightly obscured by a bezel.\n"
	    "\t\t\t\tUse a negative value to count back from the right edge of the screen.\n"
	    "\t-y, --row NUM\t\tBegin printing STRING @ row NUM (Default: 0).\n"
	    "\t\t\t\tYou might consider beginning at row 1 instead of 0, as row 0 (the topmost one) may sometimes be slightly obscured by a bezel, especially on Kobos.\n"
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
	    "\t-r, --rpadded\t\tRight pad STRING with blank spaces.\n"
	    "\n"
	    "Options affecting the message's appearance:\n"
	    "\t-h, --invert\t\tPrint STRING in <background color> over <foreground color> instead of the reverse.\n"
	    "\t-f, --flash\t\tAsk the eInk driver to do a black flash when refreshing the area of the screen where STRING will be printed.\n"
	    "\t-c, --clear\t\tFully clear the screen before printing (obeys --invert).\n"
#ifndef FBINK_FOR_LINUX
	    "\t-b, --norefresh\t\tOnly update the framebuffer, but don't actually refresh the eInk screen (useful when drawing in batch).\n"
#endif
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
#ifdef FBINK_WITH_OPENTYPE
	    "\t\t\t\tNOTE: If you're looking for vector font rendering, see the OpenType section a few lines down!\n"
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
	    "\t-T, --fgless\t\tDon't draw foreground pixels.\n"
	    "\t\t\t\tMutually exclusive with -o, --overlay or -O, --bgless, and takes precedence over them.\n"
	    "\n"
	    "Options affecting the program's verbosity:\n"
	    "\t-v, --verbose\tToggle printing diagnostic messages.\n"
	    "\t-q, --quiet\tToggle hiding hardware setup messages.\n"
	    "\n"
	    "Options affecting the program's behavior:\n"
	    "\t-I, --interactive\tEnter a very basic interactive mode.\n"
	    "\t-L, --linecountcode\tWhen successfully printing text, returns the total amount of printed lines as the process exit code.\n"
	    "\t\t\t\tNOTE: Will be inaccurate if there are more than 255 rows on screen!\n"
	    "\t-l, --linecount\t\tWhen successfully printing text, outputs the total amount of printed lines in the final line of output to stdout (NOTE: enforces quiet & non-verbose!).\n"
	    "\t\t\t\tNOTE: With OT/TTF rendering, will output a top margin value to use as-is instead (or 0 if there's no space left on screen)!\n"
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
#ifdef FBINK_WITH_OPENTYPE
	    "\n"
	    "\n"
	    "\n"
	    "OpenType & TrueType font support:\n"
	    "\t-t, --truetype regular=FILE,bold=FILE,italic=FILE,bolditalic=FILE,size=NUM,top=NUM,bottom=NUM,left=NUM,right=NUM,format\n"
	    "\t\tregular, bold, italic & bolditalic should point to the font file matching their respective font style. At least one of them MUST be specified.\n"
	    "\t\tsize sets the rendering size, in points. Defaults to 12pt if unset.\n"
	    "\t\ttop, bottom, left & right set the margins used to define the display area. Defaults to 0, i.e., the full screen, starting at the top-left corner.\n"
	    "\t\t\tNOTE: If a negative value is supplied, counts backward from the opposite edge. Mostly useful with top & left to position stuff relative to the bottom right corner.\n"
	    "\t\tIf format is specified, the underscore/star MarkDown syntax will be honored to set the font style (i.e., *italic*, **bold** & ***bold italic***).\n"
	    "\n"
	    "\t\tHonors -h, --invert; -f, --flash; -c, --clear; -m, --centered; -M, --halfway; -o, --overlay; -T, --fgless; -O, --bgless; -C, --color; -B, --background; -l, --linecount\n"
	    "\n"
	    "EXAMPLES:\n"
#	ifdef FBINK_FOR_KINDLE
	    "\tfbink -t regular=/usr/java/lib/fonts/Caecilia_LT_65_Medium.ttf,bold=/usr/java/lib/fonts/Caecilia_LT_75_Bold.ttf,size=24,top=100,bottom=500,left=25,right=50,format \"Hello **world**!\"\n"
	    "\t\tWill use Caecilia to print 'Hello world!' at 24pt in a display area starting from 100px down the top of the screen to 500px before the bottom of the screen,\n"
#	else
	    "\tfbink -t regular=/mnt/onboard/fonts/NotoSans-Regular.ttf,bold=/mnt/onboard/fonts/NotoSans-Bold.ttf,size=24,top=100,bottom=500,left=25,right=50,format \"Hello **world**!\"\n"
	    "\t\tWill use NotoSans to print 'Hello world!' at 24pt in a display area starting from 100px down the top of the screen to 500px before the bottom of the screen,\n"
#	endif
	    "\t\tfrom 25px of the left edge of the screen until 50px before the right edge.\n"
	    "\t\tHonoring the MarkDown syntax, 'Hello' will be printed with the Regular font style, while 'world' will use the Bold font style.\n"
	    "\t\tNOTE: You will NOT be able to use obfuscated or encrypted fonts.\n"
#	ifndef FBINK_FOR_LINUX
#		ifndef FBINK_FOR_KINDLE
#			ifndef FBINK_FOR_CERVANTES
	    "\t\tPlease note that this means you will NOT be able to use system fonts on Kobo, because they're all obfuscated.\n"
#			endif
#		endif
#	endif
	    "\n"
#endif
	    "\n"
	    "\n"
	    "\n"
	    "\n"
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
#else
	    "\tNote that the arguments are passed as-is to the ioctl, no viewport or rotation quirks are applied!\n"
#endif
	    "\tSpecifying one or more STRING takes precedence over this mode.\n"
#ifdef FBINK_WITH_IMAGE
	    "\n"
	    "\n"
	    "\n"
	    "\n"
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
	    "\n"
	    "\n"
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
    do_infinite_progress_bar(int fbfd, const FBInkConfig* fbink_cfg)
{
	int rv = EXIT_SUCCESS;

	const struct timespec zzz = { 0L, 750000000L };
	for (;;) {
		for (uint8_t i = 0; i < 16; i++) {
			rv = fbink_print_activity_bar(fbfd, i, fbink_cfg);
			if (rv != EXIT_SUCCESS) {
				break;
			}
			nanosleep(&zzz, NULL);
		}
		for (uint8_t i = 16; i > 0; i--) {
			rv = fbink_print_activity_bar(fbfd, i, fbink_cfg);
			if (rv != EXIT_SUCCESS) {
				break;
			}
			nanosleep(&zzz, NULL);
		}
	}

	return rv;
}

// Small helper to handle loading OT fonts
static void
    load_ot_fonts(const char*        reg_ot_file,
		  const char*        bd_ot_file,
		  const char*        it_ot_file,
		  const char*        bdit_ot_file,
		  const FBInkConfig* fbink_cfg)
{
	if (reg_ot_file) {
		if (!fbink_cfg->is_quiet) {
			printf("Loading font '%s' for the Regular style\n", reg_ot_file);
		}
		if (fbink_add_ot_font(reg_ot_file, FNT_REGULAR) < 0) {
			fprintf(stderr, "Failed to open font file '%s'!\n", reg_ot_file);
		}
	}
	if (bd_ot_file) {
		if (!fbink_cfg->is_quiet) {
			printf("Loading font '%s' for the Bold style\n", bd_ot_file);
		}
		if (fbink_add_ot_font(bd_ot_file, FNT_BOLD) < 0) {
			fprintf(stderr, "Failed to open font file '%s'!\n", bd_ot_file);
		}
	}
	if (it_ot_file) {
		if (!fbink_cfg->is_quiet) {
			printf("Loading font '%s' for the Italic style\n", it_ot_file);
		}
		if (fbink_add_ot_font(it_ot_file, FNT_ITALIC) < 0) {
			fprintf(stderr, "Failed to open font file '%s'!\n", it_ot_file);
		}
	}
	if (bdit_ot_file) {
		if (!fbink_cfg->is_quiet) {
			printf("Loading font '%s' for the Bold Italic style\n", bdit_ot_file);
		}
		if (fbink_add_ot_font(bdit_ot_file, FNT_BOLD_ITALIC) < 0) {
			fprintf(stderr, "Failed to open font file '%s'!\n", bdit_ot_file);
		}
	}
}

// Input validation via strtoul, for an uint32_t
// Adapted from the same in KFMon ;).
static int
    strtoul_u(int opt, const char* subopt, const char* str, uint32_t* result)
{
	strtoul_chk(opt, subopt, str, result);
}

// Input validation via strtoul, for an unsigned short int
static int
    strtoul_hu(int opt, const char* subopt, const char* str, unsigned short int* result)
{
	strtoul_chk(opt, subopt, str, result);
}

// Input validation via strtoul, for an uint8_t
static int
    strtoul_hhu(int opt, const char* subopt, const char* str, uint8_t* result)
{
	strtoul_chk(opt, subopt, str, result);
}

// Input validation via strtol, for a short int
static int
    strtol_hi(int opt, const char* subopt, const char* str, short int* result)
{
	strtol_chk(opt, subopt, str, result);
}

// Input validation via strtol, for an int8_t
static int
    strtol_hhi(int opt, const char* subopt, const char* str, int8_t* result)
{
	strtol_chk(opt, subopt, str, result);
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
					      { "rpadded", no_argument, NULL, 'r' },
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
					      { "fgless", no_argument, NULL, 'T' },
					      { "truetype", required_argument, NULL, 't' },
					      { "norefresh", no_argument, NULL, 'b' },
					      { NULL, 0, NULL, 0 } };

	FBInkConfig fbink_cfg = { 0 };
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wmissing-braces"
	FBInkOTConfig ot_config = { 0 };
#pragma GCC diagnostic pop

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
	enum
	{
		REGULAR_OPT = 0,
		BOLD_OPT,
		ITALIC_OPT,
		BOLDITALIC_OPT,
		SIZE_OPT,
		TM_OPT,
		BM_OPT,
		LM_OPT,
		RM_OPT,
		FMT_OPT,
	};
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunknown-pragmas"
#pragma clang diagnostic ignored "-Wunknown-warning-option"
#pragma GCC diagnostic ignored "-Wdiscarded-qualifiers"
#pragma clang diagnostic ignored "-Wincompatible-pointer-types-discards-qualifiers"
	char* const refresh_token[]  = { [TOP_OPT] = "top",       [LEFT_OPT] = "left", [WIDTH_OPT] = "width",
                                        [HEIGHT_OPT] = "height", [WFM_OPT] = "wfm",   NULL };
	char* const image_token[]    = { [FILE_OPT] = "file",     [XOFF_OPT] = "x",        [YOFF_OPT] = "y",
                                      [HALIGN_OPT] = "halign", [VALIGN_OPT] = "valign", NULL };
	char* const truetype_token[] = { [REGULAR_OPT]    = "regular",
					 [BOLD_OPT]       = "bold",
					 [ITALIC_OPT]     = "italic",
					 [BOLDITALIC_OPT] = "bolditalic",
					 [SIZE_OPT]       = "size",
					 [TM_OPT]         = "top",
					 [BM_OPT]         = "bottom",
					 [LM_OPT]         = "left",
					 [RM_OPT]         = "right",
					 [FMT_OPT]        = "format",
					 NULL };
#pragma GCC diagnostic pop
	char*     subopts;
	char*     value          = NULL;
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
	bool      is_truetype    = false;
	char*     reg_ot_file    = NULL;
	char*     bd_ot_file     = NULL;
	char*     it_ot_file     = NULL;
	char*     bdit_ot_file   = NULL;
	bool      errfnd         = false;

	while ((opt = getopt_long(argc, argv, "y:x:Y:X:hfcmMprs:S:F:vqg:i:aeIC:B:LlP:A:oOTVt:b", opts, &opt_index)) !=
	       -1) {
		switch (opt) {
			case 'y':
				if (strtol_hi(opt, NULL, optarg, &fbink_cfg.row) < 0) {
					errfnd = true;
				}
				break;
			case 'x':
				if (strtol_hi(opt, NULL, optarg, &fbink_cfg.col) < 0) {
					errfnd = true;
				}
				break;
			case 'Y':
				if (strtol_hi(opt, NULL, optarg, &fbink_cfg.voffset) < 0) {
					errfnd = true;
				}
				break;
			case 'X':
				if (strtol_hi(opt, NULL, optarg, &fbink_cfg.hoffset) < 0) {
					errfnd = true;
				}
				break;
			case 'h':
				fbink_cfg.is_inverted = true;
				break;
			case 'f':
				fbink_cfg.is_flashing = true;
				break;
			case 'c':
				fbink_cfg.is_cleared = true;
				break;
			case 'm':
				fbink_cfg.is_centered = true;
				break;
			case 'M':
				fbink_cfg.is_halfway = true;
				break;
			case 'p':
				fbink_cfg.is_padded = true;
				break;
			case 'r':
				fbink_cfg.is_rpadded = true;
				break;
			case 's':
				subopts = optarg;
				while (*subopts != '\0' && !errfnd) {
					switch (getsubopt(&subopts, refresh_token, &value)) {
						case TOP_OPT:
							if (strtoul_u(opt, refresh_token[TOP_OPT], value, &region_top) <
							    0) {
								errfnd = true;
							}
							break;
						case LEFT_OPT:
							if (strtoul_u(opt, refresh_token[LEFT_OPT], value, &region_left) <
							    0) {
								errfnd = true;
							}
							break;
						case WIDTH_OPT:
							if (strtoul_u(
								opt, refresh_token[WIDTH_OPT], value, &region_width) <
							    0) {
								errfnd = true;
							}
							break;
						case HEIGHT_OPT:
							if (strtoul_u(
								opt, refresh_token[HEIGHT_OPT], value, &region_height) <
							    0) {
								errfnd = true;
							}
							break;
						case WFM_OPT:
							if (value == NULL) {
								fprintf(stderr,
									"Missing value for suboption '%s'\n",
									refresh_token[WFM_OPT]);
								errfnd = true;
								continue;
							}

							region_wfm = value;
							break;
						default:
							fprintf(stderr, "No match found for token: /%s/\n", value);
							errfnd = true;
							break;
					}
				}
				// Make sure we won't pass an invalid rectangle to the driver, because that'd soft lock,
				// or a NULL pointer to fbink_refresh, because that would segfault in strcmp ;).
				if (((region_height == 0 || region_width == 0) &&
				     !(region_top == 0 && region_left == 0 && region_height == 0 && region_width == 0)) ||
				    region_wfm == NULL) {
					fprintf(
					    stderr,
					    "Suboption '%s' must be specified, as well as non-zero values for '%s' and '%s'\n",
					    refresh_token[WFM_OPT],
					    refresh_token[HEIGHT_OPT],
					    refresh_token[WIDTH_OPT]);
					errfnd = true;
				} else {
					is_refresh = true;
				}
				break;
			case 'S':
				if (strtoul_hhu(opt, NULL, optarg, &fbink_cfg.fontmult) < 0) {
					errfnd = true;
				}
				break;
			case 'F':
				if (strcasecmp(optarg, "IBM") == 0) {
					fbink_cfg.fontname = IBM;
				} else if (strcasecmp(optarg, "UNSCII") == 0) {
					fbink_cfg.fontname = UNSCII;
				} else if (strcasecmp(optarg, "ALT") == 0) {
					fbink_cfg.fontname = UNSCII_ALT;
				} else if (strcasecmp(optarg, "THIN") == 0) {
					fbink_cfg.fontname = UNSCII_THIN;
				} else if (strcasecmp(optarg, "FANTASY") == 0) {
					fbink_cfg.fontname = UNSCII_FANTASY;
				} else if (strcasecmp(optarg, "MCR") == 0) {
					fbink_cfg.fontname = UNSCII_MCR;
				} else if (strcasecmp(optarg, "TALL") == 0) {
					fbink_cfg.fontname = UNSCII_TALL;
				} else if (strcasecmp(optarg, "BLOCK") == 0) {
					fbink_cfg.fontname = BLOCK;
				} else if (strcasecmp(optarg, "LEGGIE") == 0) {
					fbink_cfg.fontname = LEGGIE;
				} else if (strcasecmp(optarg, "VEGGIE") == 0) {
					fbink_cfg.fontname = VEGGIE;
				} else if (strcasecmp(optarg, "KATES") == 0) {
					fbink_cfg.fontname = KATES;
				} else if (strcasecmp(optarg, "FKP") == 0) {
					fbink_cfg.fontname = FKP;
				} else if (strcasecmp(optarg, "CTRLD") == 0) {
					fbink_cfg.fontname = CTRLD;
				} else if (strcasecmp(optarg, "ORP") == 0) {
					fbink_cfg.fontname = ORP;
				} else if (strcasecmp(optarg, "ORPB") == 0) {
					fbink_cfg.fontname = ORPB;
				} else if (strcasecmp(optarg, "ORPI") == 0) {
					fbink_cfg.fontname = ORPI;
				} else if (strcasecmp(optarg, "SCIENTIFICA") == 0) {
					fbink_cfg.fontname = SCIENTIFICA;
				} else if (strcasecmp(optarg, "SCIENTIFICAB") == 0) {
					fbink_cfg.fontname = SCIENTIFICAB;
				} else if (strcasecmp(optarg, "SCIENTIFICAI") == 0) {
					fbink_cfg.fontname = SCIENTIFICAI;
				} else {
					fprintf(stderr, "Unknown font name '%s'.\n", optarg);
					errfnd = true;
				}
				break;
			case 'v':
				fbink_cfg.is_verbose = !fbink_cfg.is_verbose;
				break;
			case 'q':
				fbink_cfg.is_quiet = !fbink_cfg.is_quiet;
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
							if (strtol_hi(
								opt, image_token[XOFF_OPT], value, &image_x_offset) < 0) {
								errfnd = true;
							}
							break;
						case YOFF_OPT:
							if (strtol_hi(
								opt, image_token[YOFF_OPT], value, &image_y_offset) < 0) {
								errfnd = true;
							}
							break;
						case HALIGN_OPT:
							if (strcasecmp(value, "NONE") == 0 ||
							    strcasecmp(value, "LEFT") == 0) {
								fbink_cfg.halign = NONE;
							} else if (strcasecmp(value, "CENTER") == 0 ||
								   strcasecmp(value, "MIDDLE") == 0) {
								fbink_cfg.halign = CENTER;
							} else if (strcasecmp(value, "EDGE") == 0 ||
								   strcasecmp(value, "RIGHT") == 0) {
								fbink_cfg.halign = EDGE;
							} else {
								fprintf(stderr, "Unknown alignment value '%s'.\n", value);
								errfnd = true;
							}
							break;
						case VALIGN_OPT:
							if (strcasecmp(value, "NONE") == 0 ||
							    strcasecmp(value, "TOP") == 0) {
								fbink_cfg.valign = NONE;
							} else if (strcasecmp(value, "CENTER") == 0 ||
								   strcasecmp(value, "MIDDLE") == 0) {
								fbink_cfg.valign = CENTER;
							} else if (strcasecmp(value, "EDGE") == 0 ||
								   strcasecmp(value, "BOTTOM") == 0) {
								fbink_cfg.valign = EDGE;
							} else {
								fprintf(stderr, "Unknown alignment value '%s'.\n", value);
								errfnd = true;
							}
							break;
						default:
							fprintf(stderr, "No match found for token: /%s/\n", value);
							errfnd = true;
							break;
					}
				}
				if (image_file == NULL) {
					fprintf(stderr, "Must specify at least '%s'\n", image_token[FILE_OPT]);
					errfnd = true;
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
				fbink_cfg.ignore_alpha = true;
				break;
			case 'e':
				is_eval = true;
				break;
			case 'I':
				is_interactive = true;
				break;
			case 'C':
				if (strcasecmp(optarg, "BLACK") == 0) {
					fbink_cfg.fg_color = FG_BLACK;
				} else if (strcasecmp(optarg, "GRAY1") == 0) {
					fbink_cfg.fg_color = FG_GRAY1;
				} else if (strcasecmp(optarg, "GRAY2") == 0) {
					fbink_cfg.fg_color = FG_GRAY2;
				} else if (strcasecmp(optarg, "GRAY3") == 0) {
					fbink_cfg.fg_color = FG_GRAY3;
				} else if (strcasecmp(optarg, "GRAY4") == 0) {
					fbink_cfg.fg_color = FG_GRAY4;
				} else if (strcasecmp(optarg, "GRAY5") == 0) {
					fbink_cfg.fg_color = FG_GRAY5;
				} else if (strcasecmp(optarg, "GRAY6") == 0) {
					fbink_cfg.fg_color = FG_GRAY6;
				} else if (strcasecmp(optarg, "GRAY7") == 0) {
					fbink_cfg.fg_color = FG_GRAY7;
				} else if (strcasecmp(optarg, "GRAY8") == 0) {
					fbink_cfg.fg_color = FG_GRAY8;
				} else if (strcasecmp(optarg, "GRAY9") == 0) {
					fbink_cfg.fg_color = FG_GRAY9;
				} else if (strcasecmp(optarg, "GRAYA") == 0) {
					fbink_cfg.fg_color = FG_GRAYA;
				} else if (strcasecmp(optarg, "GRAYB") == 0) {
					fbink_cfg.fg_color = FG_GRAYB;
				} else if (strcasecmp(optarg, "GRAYC") == 0) {
					fbink_cfg.fg_color = FG_GRAYC;
				} else if (strcasecmp(optarg, "GRAYD") == 0) {
					fbink_cfg.fg_color = FG_GRAYD;
				} else if (strcasecmp(optarg, "GRAYE") == 0) {
					fbink_cfg.fg_color = FG_GRAYE;
				} else if (strcasecmp(optarg, "WHITE") == 0) {
					fbink_cfg.fg_color = FG_WHITE;
				} else {
					fprintf(stderr, "Unknown color name '%s'.\n", optarg);
					errfnd = true;
				}
				break;
			case 'B':
				if (strcasecmp(optarg, "BLACK") == 0) {
					fbink_cfg.bg_color = BG_BLACK;
				} else if (strcasecmp(optarg, "GRAY1") == 0) {
					fbink_cfg.bg_color = BG_GRAY1;
				} else if (strcasecmp(optarg, "GRAY2") == 0) {
					fbink_cfg.bg_color = BG_GRAY2;
				} else if (strcasecmp(optarg, "GRAY3") == 0) {
					fbink_cfg.bg_color = BG_GRAY3;
				} else if (strcasecmp(optarg, "GRAY4") == 0) {
					fbink_cfg.bg_color = BG_GRAY4;
				} else if (strcasecmp(optarg, "GRAY5") == 0) {
					fbink_cfg.bg_color = BG_GRAY5;
				} else if (strcasecmp(optarg, "GRAY6") == 0) {
					fbink_cfg.bg_color = BG_GRAY6;
				} else if (strcasecmp(optarg, "GRAY7") == 0) {
					fbink_cfg.bg_color = BG_GRAY7;
				} else if (strcasecmp(optarg, "GRAY8") == 0) {
					fbink_cfg.bg_color = BG_GRAY8;
				} else if (strcasecmp(optarg, "GRAY9") == 0) {
					fbink_cfg.bg_color = BG_GRAY9;
				} else if (strcasecmp(optarg, "GRAYA") == 0) {
					fbink_cfg.bg_color = BG_GRAYA;
				} else if (strcasecmp(optarg, "GRAYB") == 0) {
					fbink_cfg.bg_color = BG_GRAYB;
				} else if (strcasecmp(optarg, "GRAYC") == 0) {
					fbink_cfg.bg_color = BG_GRAYC;
				} else if (strcasecmp(optarg, "GRAYD") == 0) {
					fbink_cfg.bg_color = BG_GRAYD;
				} else if (strcasecmp(optarg, "GRAYE") == 0) {
					fbink_cfg.bg_color = BG_GRAYE;
				} else if (strcasecmp(optarg, "WHITE") == 0) {
					fbink_cfg.bg_color = BG_WHITE;
				} else {
					fprintf(stderr, "Unknown color name '%s'.\n", optarg);
					errfnd = true;
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
				if (strtoul_hhu(opt, NULL, optarg, &progress) < 0) {
					errfnd = true;
				}
				break;
			case 'A':
				is_activitybar = true;
				int8_t val     = 0;
				if (strtol_hhi(opt, NULL, optarg, &val) < 0) {
					errfnd = true;
				}
				if (val < 0) {
					is_infinite = true;
				} else {
					progress = (uint8_t) val;
				}
				break;
			case 'o':
				fbink_cfg.is_overlay = true;
				break;
			case 'O':
				fbink_cfg.is_bgless = true;
				break;
			case 'T':
				fbink_cfg.is_fgless = true;
				break;
			case 'V':
				fbink_cfg.no_viewport = true;
				break;
			case 't':
				subopts = optarg;
				while (*subopts != '\0' && !errfnd) {
					switch (getsubopt(&subopts, truetype_token, &value)) {
						case REGULAR_OPT:
							reg_ot_file = strdup(value);
							break;
						case BOLD_OPT:
							bd_ot_file = strdup(value);
							break;
						case ITALIC_OPT:
							it_ot_file = strdup(value);
							break;
						case BOLDITALIC_OPT:
							bdit_ot_file = strdup(value);
							break;
						case SIZE_OPT:
							if (strtoul_hu(opt,
								       truetype_token[SIZE_OPT],
								       value,
								       &ot_config.size_pt) < 0) {
								errfnd = true;
							}
							break;
						case TM_OPT:
							if (strtol_hi(opt,
								      truetype_token[TM_OPT],
								      value,
								      &ot_config.margins.top) < 0) {
								errfnd = true;
							}
							break;
						case BM_OPT:
							if (strtol_hi(opt,
								      truetype_token[BM_OPT],
								      value,
								      &ot_config.margins.bottom) < 0) {
								errfnd = true;
							}
							break;
						case LM_OPT:
							if (strtol_hi(opt,
								      truetype_token[LM_OPT],
								      value,
								      &ot_config.margins.left) < 0) {
								errfnd = true;
							}
							break;
						case RM_OPT:
							if (strtol_hi(opt,
								      truetype_token[RM_OPT],
								      value,
								      &ot_config.margins.right) < 0) {
								errfnd = true;
							}
							break;
						case FMT_OPT:
							ot_config.is_formatted = true;
							break;
						default:
							fprintf(stderr, "No match found for token: /%s/\n", value);
							errfnd = true;
							break;
					}
				}
				// Make sure we've passed at least one font style
				if (reg_ot_file == NULL && bd_ot_file == NULL && it_ot_file == NULL &&
				    bdit_ot_file == NULL) {
					fprintf(stderr, "At least one font style must be specified!\n");
					errfnd = true;
				} else {
					is_truetype = true;
				}
				break;
			case 'b':
				fbink_cfg.no_refresh = true;
				break;
			default:
				fprintf(stderr, "?? Unknown option code 0%o ??\n", (unsigned int) opt);
				errfnd = true;
				break;
		}
	}

	// Enforce quiet output when asking for want_linecount, to avoid polluting the output...
	if (want_linecount) {
		fbink_cfg.is_quiet   = true;
		fbink_cfg.is_verbose = false;
	}

	// Assume success, until shit happens ;)
	int rv = EXIT_SUCCESS;
	// Declare it a tiny bit early to make cleanup handling safe
	// (fbink_close is safe to call with fbfd set to -1 and/or the mmap not actually done).
	int fbfd = -1;

	// Don't abort if we piped something without passing any arguments!
	if (errfnd || (argc == 1 && isatty(fileno(stdin)))) {
		if (errfnd) {
			fprintf(stderr, "\n****\t****\t****\t****\n");
		}
		show_helpmsg();
		// NOTE: Having the actual error message printed *above* the seven billion lines of the help message
		//       pretty much ensures no one will ever notice it, so remind the user that there's also
		//       an actual error message to read much higher in their terminal backlog ;p
		if (errfnd) {
			fprintf(stderr, "\n****\t****\t****\t****\n\n");
			fprintf(stderr, "Encountered a parsing error, see the top of the output for details!\n");
		}
		rv = ERRCODE(EXIT_FAILURE);
		goto cleanup;
	}

	// Open framebuffer and keep it around, then setup globals.
	if (ERRCODE(EXIT_FAILURE) == (fbfd = fbink_open())) {
		fprintf(stderr, "Failed to open the framebuffer, aborting . . .\n");
		rv = ERRCODE(EXIT_FAILURE);
		goto cleanup;
	}
	if (fbink_init(fbfd, &fbink_cfg) == ERRCODE(EXIT_FAILURE)) {
		fprintf(stderr, "Failed to initialize FBInk, aborting . . .\n");
		rv = ERRCODE(EXIT_FAILURE);
		goto cleanup;
	}

	char* string;
	if (optind < argc) {
		// We'll need that in the cell rendering codepath
		unsigned short int total_lines = 0U;

		// And for the OpenType codepath, we'll want to load the fonts only once ;)
		if (is_truetype) {
			load_ot_fonts(reg_ot_file, bd_ot_file, it_ot_file, bdit_ot_file, &fbink_cfg);
		}

		// Now that this is out of the way, loop over the leftover arguments, i.e.: the strings ;)
		while (optind < argc) {
			int linecount = -1;
			string        = argv[optind++];
			// NOTE: This is probably the point where we'd be validating/converting string to UTF-8,
			//       if we had an easy way to... (c.f., my rant about Kobo's broken libc in fbink_internal.h)

			// Did we want to use the OpenType codepath?
			if (is_truetype) {
				if (!fbink_cfg.is_quiet) {
					printf(
					    "Printing string '%s' @ %hupt, honoring the following margins { Top: %hdpx, Bottom: %hdpx, Left: %hdpx, Right: %hdpx } (formatted: %s, overlay: %s, no BG: %s, no FG: %s, inverted: %s, flashing: %s, centered: %s, H align: %hhu, halfway: %s, V align: %hhu, clear screen: %s, skip refresh: %s)\n",
					    string,
					    ot_config.size_pt,
					    ot_config.margins.top,
					    ot_config.margins.bottom,
					    ot_config.margins.left,
					    ot_config.margins.right,
					    ot_config.is_formatted ? "Y" : "N",
					    fbink_cfg.is_overlay ? "Y" : "N",
					    fbink_cfg.is_bgless ? "Y" : "N",
					    fbink_cfg.is_fgless ? "Y" : "N",
					    fbink_cfg.is_inverted ? "Y" : "N",
					    fbink_cfg.is_flashing ? "Y" : "N",
					    fbink_cfg.is_centered ? "Y" : "N",
					    fbink_cfg.halign,
					    fbink_cfg.is_halfway ? "Y" : "N",
					    fbink_cfg.valign,
					    fbink_cfg.is_cleared ? "Y" : "N",
					    fbink_cfg.no_refresh ? "Y" : "N");
				}

				if ((linecount = fbink_print_ot(fbfd, string, &ot_config, &fbink_cfg)) < 0) {
					fprintf(stderr, "Failed to print that string!\n");
					rv = ERRCODE(EXIT_FAILURE);
					goto cleanup;
				}

				// NOTE: Don't clobber previous entries if multiple strings were passed...
				//       We make sure to trust print's return value,
				//       because it knows how much space it already took up ;).
				ot_config.margins.top = (short int) linecount;
				// NOTE: By design, if you ask for a clear screen, only the final print will stay on screen ;).

				// If we were asked to return the amount of printed lines, honor that,
				// provided we actually successfully printed something...
				// NOTE: We don't support linecode, because POSIX exit codes cap at 255, which is much too low.
				if (want_linecount && rv >= EXIT_SUCCESS) {
					// NOTE: fbink_print_ot returns a new top margin value directly,
					//       no need for any extra computations!
					total_lines = (unsigned short int) (linecount);
				}
			} else {
				if (!fbink_cfg.is_quiet) {
					printf(
					    "Printing string '%s' @ column %hd + %hdpx, row %hd + %hdpx (overlay: %s, no BG: %s, no FG: %s, inverted: %s, flashing: %s, centered: %s, halfway: %s, left padded: %s, right padded: %s, clear screen: %s, skip refresh: %s, font: %hhu, font scaling: x%hhu)\n",
					    string,
					    fbink_cfg.col,
					    fbink_cfg.hoffset,
					    fbink_cfg.row,
					    fbink_cfg.voffset,
					    fbink_cfg.is_overlay ? "Y" : "N",
					    fbink_cfg.is_bgless ? "Y" : "N",
					    fbink_cfg.is_fgless ? "Y" : "N",
					    fbink_cfg.is_inverted ? "Y" : "N",
					    fbink_cfg.is_flashing ? "Y" : "N",
					    fbink_cfg.is_centered ? "Y" : "N",
					    fbink_cfg.is_halfway ? "Y" : "N",
					    fbink_cfg.is_padded ? "Y" : "N",
					    fbink_cfg.is_rpadded ? "Y" : "N",
					    fbink_cfg.is_cleared ? "Y" : "N",
					    fbink_cfg.no_refresh ? "Y" : "N",
					    fbink_cfg.fontname,
					    fbink_cfg.fontmult);
				}

				if ((linecount = fbink_print(fbfd, string, &fbink_cfg)) < 0) {
					fprintf(stderr, "Failed to print that string!\n");
					rv = ERRCODE(EXIT_FAILURE);
					goto cleanup;
				}

				// NOTE: Don't clobber previous entries if multiple strings were passed...
				//       We make sure to trust print's return value,
				//       because it knows how much space it already took up ;).
				fbink_cfg.row = (short int) (fbink_cfg.row + linecount);
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
			if (!fbink_cfg.is_quiet) {
				printf(
				    "Refreshing the screen from top=%u, left=%u for width=%u, height=%u with %swaveform mode %s\n",
				    region_top,
				    region_left,
				    region_width,
				    region_height,
				    fbink_cfg.is_flashing ? "a flashing " : "",
				    region_wfm);
			}
			if (fbink_refresh(fbfd,
					  region_top,
					  region_left,
					  region_width,
					  region_height,
					  region_wfm,
					  fbink_cfg.is_flashing) != EXIT_SUCCESS) {
				fprintf(stderr, "Failed to refresh the screen as per your specification!\n");
				rv = ERRCODE(EXIT_FAILURE);
				goto cleanup;
			}
		} else if (is_image) {
			if (!fbink_cfg.is_quiet) {
				printf(
				    "Displaying image '%s' @ column %hd + %hdpx, row %hd + %dpx (H align: %hhu, V align: %hhu, inverted: %s, flattened: %s, skip refresh: %s)\n",
				    image_file,
				    fbink_cfg.col,
				    image_x_offset,
				    fbink_cfg.row,
				    image_y_offset,
				    fbink_cfg.halign,
				    fbink_cfg.valign,
				    fbink_cfg.is_inverted ? "Y" : "N",
				    fbink_cfg.ignore_alpha ? "Y" : "N",
				    fbink_cfg.no_refresh ? "Y" : "N");
			}
			if (fbink_print_image(fbfd, image_file, image_x_offset, image_y_offset, &fbink_cfg) !=
			    EXIT_SUCCESS) {
				fprintf(stderr, "Failed to display that image!\n");
				rv = ERRCODE(EXIT_FAILURE);
				goto cleanup;
			}
		} else if (is_progressbar) {
			if (!fbink_cfg.is_quiet) {
				printf(
				    "Displaying a %hhu%% full progress bar @ row %hd + %hdpx (inverted: %s, flashing: %s, clear screen: %s, skip refresh: %s, font: %hhu, font scaling: x%hhu)\n",
				    progress,
				    fbink_cfg.row,
				    fbink_cfg.voffset,
				    fbink_cfg.is_inverted ? "Y" : "N",
				    fbink_cfg.is_flashing ? "Y" : "N",
				    fbink_cfg.is_cleared ? "Y" : "N",
				    fbink_cfg.no_refresh ? "Y" : "N",
				    fbink_cfg.fontname,
				    fbink_cfg.fontmult);
			}
			if (fbink_print_progress_bar(fbfd, progress, &fbink_cfg) != EXIT_SUCCESS) {
				fprintf(stderr, "Failed to display a progressbar!\n");
				rv = ERRCODE(EXIT_FAILURE);
				goto cleanup;
			}
		} else if (is_activitybar) {
			// Were we asked to loop forever?
			if (is_infinite) {
				if (!fbink_cfg.is_quiet) {
					printf(
					    "Displaying an activity bar cycling forever @ row %hd + %hdpx (inverted: %s, flashing: %s, clear screen: %s, skip refresh: %s)\n",
					    fbink_cfg.row,
					    fbink_cfg.voffset,
					    fbink_cfg.is_inverted ? "Y" : "N",
					    fbink_cfg.is_flashing ? "Y" : "N",
					    fbink_cfg.is_cleared ? "Y" : "N",
					    fbink_cfg.no_refresh ? "Y" : "N");
				}
				// NOTE: In a dedicated function,
				//       because keeping it inline massively tanks performance in the image codepath,
				//       for an amazingly weird LTO-related reason :?
				if (do_infinite_progress_bar(fbfd, &fbink_cfg) != EXIT_SUCCESS) {
					fprintf(stderr, "Failed to display a progressbar!\n");
					rv = ERRCODE(EXIT_FAILURE);
					goto cleanup;
				}
			} else {
				if (!fbink_cfg.is_quiet) {
					printf(
					    "Displaying an activity bar on step %hhu @ row %hd + %hdpx (inverted: %s, flashing: %s, clear screen: %s, skip refresh: %s)\n",
					    progress,
					    fbink_cfg.row,
					    fbink_cfg.voffset,
					    fbink_cfg.is_inverted ? "Y" : "N",
					    fbink_cfg.is_flashing ? "Y" : "N",
					    fbink_cfg.is_cleared ? "Y" : "N",
					    fbink_cfg.no_refresh ? "Y" : "N");
				}
				if (fbink_print_activity_bar(fbfd, progress, &fbink_cfg) != EXIT_SUCCESS) {
					fprintf(stderr, "Failed to display an activitybar!\n");
					rv = ERRCODE(EXIT_FAILURE);
					goto cleanup;
				}
			}
		} else if (is_eval) {
			fbink_state_dump(&fbink_cfg);
		} else if (is_interactive && isatty(fileno(stdin))) {
			// We asked for interactive mode, and we're really running from a terminal, so, go ahead.
			char*   line = NULL;
			size_t  len  = 0;
			ssize_t nread;
			int     linecnt = -1;
			// Draw a poor man's prompt, which works fairly okay, until we enable verbose mode :D.
			// NOTE: We limit this to the fixed-cell renderer, as it has a few tweaks to handle this better.
			printf(">>> ");
			while ((nread = getline(&line, &len, stdin)) != -1) {
				printf(">>> ");
				if ((linecnt = fbink_print(fbfd, line, &fbink_cfg)) < 0) {
					fprintf(stderr, "Failed to print that string!\n");
					rv = ERRCODE(EXIT_FAILURE);
				}
				fbink_cfg.row = (short int) (fbink_cfg.row + linecnt);
			}
			free(line);
		} else {
			// If all else failed, try reading from stdin, provided we're not running from a terminal ;).
			if (!isatty(fileno(stdin))) {
				char*   line = NULL;
				size_t  len  = 0;
				ssize_t nread;
				int     linecnt = -1;
				// Did we ask for OT rendering?
				if (is_truetype) {
					load_ot_fonts(reg_ot_file, bd_ot_file, it_ot_file, bdit_ot_file, &fbink_cfg);
					while ((nread = getline(&line, &len, stdin)) != -1) {
						if ((linecnt = fbink_print_ot(fbfd, line, &ot_config, &fbink_cfg)) < 0) {
							fprintf(stderr, "Failed to print that string!\n");
							rv = ERRCODE(EXIT_FAILURE);
						}
						ot_config.margins.top = (short int) linecnt;
					}
				} else {
					while ((nread = getline(&line, &len, stdin)) != -1) {
						if ((linecnt = fbink_print(fbfd, line, &fbink_cfg)) < 0) {
							fprintf(stderr, "Failed to print that string!\n");
							rv = ERRCODE(EXIT_FAILURE);
						}
						fbink_cfg.row = (short int) (fbink_cfg.row + linecnt);
					}
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

	if (is_truetype) {
		fbink_free_ot_fonts();
	}
	free(reg_ot_file);
	free(bd_ot_file);
	free(it_ot_file);
	free(bdit_ot_file);

	if (fbink_close(fbfd) == ERRCODE(EXIT_FAILURE)) {
		fprintf(stderr, "Failed to close the framebuffer, aborting . . .\n");
		rv = ERRCODE(EXIT_FAILURE);
	}

	return rv;
}
