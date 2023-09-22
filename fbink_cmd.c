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

#ifdef FBINK_MINIMAL
#	ifndef FBINK_WITH_BITMAP
#		error Cannot build this tool without fixed-cell font rendering support!
#	endif
#endif

#include "fbink_cmd.h"

static const char*
    has_feature(const uint32_t mask, uint32_t flag)
{
	if (mask & flag) {
		return "Yes";
	} else {
		return "No";
	}
}
// Help message
static void
    show_helpmsg(void)
{
	printf("\nFBInk %s", fbink_version());
	// Display the target platform
	const FBINK_TARGET_T platform = fbink_target();
	switch (platform) {
		case FBINK_TARGET_LINUX:
			printf(" for Linux");
			break;
		case FBINK_TARGET_KOBO:
			printf(" for Kobo");
			break;
		case FBINK_TARGET_KINDLE:
			printf(" for Kindle");
			break;
		case FBINK_TARGET_KINDLE_LEGACY:
			printf(" for Legacy Kindle");
			break;
		case FBINK_TARGET_CERVANTES:
			printf(" for Cervantes");
			break;
		case FBINK_TARGET_REMARKABLE:
			printf(" for reMarkable");
			break;
		case FBINK_TARGET_POCKETBOOK:
			printf(" for PocketBook");
			break;
	}
	// Recap the build features
	const uint32_t features = fbink_features();
	printf(" [Draw=%s, Bitmap=%s, Fonts=%s, Unifont=%s, OpenType=%s, Image=%s, ButtonScan=%s]",
	       has_feature(features, FBINK_FEATURE_DRAW),
	       has_feature(features, FBINK_FEATURE_BITMAP),
	       has_feature(features, FBINK_FEATURE_FONTS),
	       has_feature(features, FBINK_FEATURE_UNIFONT),
	       has_feature(features, FBINK_FEATURE_OPENTYPE),
	       has_feature(features, FBINK_FEATURE_IMAGE),
	       has_feature(features, FBINK_FEATURE_BUTTON_SCAN));
#ifdef DEBUG
	printf(" [DEBUG]");
#endif
	printf(
	    "\n"
	    "\n"
	    "Usage: fbink [OPTIONS] [STRING ...]\n"
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
#ifdef FBINK_FOR_KINDLE
	    "\t\t\t\tNote that on legacy einkfb devices, this may not always be honored by the hardware.\n"
#endif
	    "\t-c, --clear\t\tClear the full screen before printing.\n"
	    "\t\t\t\tHonors -B, --background; -h, --invert; -H, --nightmode; -W, --waveform; -D, --dither; -b, --norefresh; -w, --wait.\n"
	    "\t\t\t\tCan be specified on its own, without any STRING.\n"
	    "\t\t\t\tNOTE: If your intent is to simply clear the screen and *nothing else*, use -k, --cls instead!\n"
#ifndef FBINK_FOR_LINUX
	    "\t-W, --waveform MODE\tRequest a specific waveform update mode from the eInk controller, if supported (mainly useful for images).\n"
	    "\t\t\t\tAvailable waveform modes: A2, DU, GL16, GC16 & AUTO\n"
#	if defined(FBINK_FOR_KINDLE)
	    "\t\t\t\tAs well as REAGL, REAGLD, GC16_FAST, GL16_FAST, DU4, GL4, GL16_INV, GCK16, GLKW16 & DUNM on some Kindles, depending on the model & FW version.\n"
	    "\t\t\t\tNote that specifying a waveform mode is ignored on legacy einkfb devices, because the hardware doesn't expose such capabilities.\n"
#	elif defined(FBINK_FOR_POCKETBOOK)
	    "\t\t\t\tAs well as GC4, A2IN, A2OUT, DU4, REAGL, REAGLD, GC16HQ & GS16.\n"
#	elif defined(FBINK_FOR_KOBO)
	    "\t\t\t\tAs well as GC4, REAGL & REAGLD. And GU16, GCK16, GLK16 & GCC16 on Mk. 8. And DU4, GCK16 & GLKW16 on Mk. 9.\n"
#	elif defined(FBINK_FOR_CERVANTES)
	    "\t\t\t\tAs well as GC4, REAGL & REAGLD.\n"
#	endif
#	if !defined(FBINK_FOR_REMARKABLE)
	    "\t\t\t\tUnsupported modes *should* safely downgrade to AUTO. Operative word being 'should' ;).\n"
#		if defined(FBINK_FOR_POCKETBOOK)
	    "\t\t\t\tOn devices with a B288 SoC, AUTO is *not* supported. FBInk will silently use GC16 instead!\n"
#		endif
	    "\t\t\t\tOn some devices, REAGL & REAGLD expect to be flashing in order to behave properly.\n"
#	endif
	    "\t-D, --dither\t\tRequest a specific hardware dithering mode from the eInk controller, if supported (mainly useful for images).\n"
	    "\t\t\t\tAvailable dithering modes: PASSTHROUGH, FLOYD_STEINBERG, ATKINSON, ORDERED, QUANT_ONLY & LEGACY\n"
	    "\t\t\t\tNote that this is only supported on recent devices, and that only a subset of these options may actually be supported by the HW (usually, PASSTHROUGH & ORDERED, check dmesg).\n"
	    "\t\t\t\tLEGACY may be supported on more devices, but what exactly it does in practice (and how well it works) depends on the exact device and/or FW version.\n"
#	ifdef FBINK_FOR_KINDLE
	    "\t\t\t\tTrue (i.e., not LEGACY) hardware dithering is completely untested on Kindle, and, while the Oasis 2, PaperWhite 4 & Oasis 3 *should* support it, they *may* not, or at least not in the way FBInk expects...\n"
#	endif
	    "\t-H, --nightmode\t\tRequest full hardware inversion from the eInk controller, if supported.\n"
	    "\t\t\t\tNote that this can be used *in combination* with -h, --invert! One does not exclude the other, which may lead to some confusing behavior ;).\n"
#	ifdef FBINK_FOR_KINDLE
	    "\t\t\t\tNote that requesting nightmode is ignored on legacy einkfb devices, because the hardware doesn't (easily) expose such capabilities.\n"
#	endif
	    "\t\t\t\tNote that this may be ignored on some specific devices where it is known to be or have been unstable at some point.\n"
#	ifdef FBINK_FOR_KINDLE
	    "\t-K, --animate direction=DIR,steps=NUM\n"
	    "\t\t\t\t\t\tRequest animated refreshes from the driver, if supported.\n"
	    "\t\t\t\tAvailable directions: DOWN, UP, LEFT & RIGHT\n"
	    "\t\t\t\tThe highest step count currently supported is 60.\n"
	    "\t\t\t\tThis is only supported on devices running on a MTK SoC.\n"
#	endif
	    "\t-b, --norefresh\t\tOnly update the framebuffer, but don't actually refresh the eInk screen (useful when drawing in batch).\n"
#	if defined(FBINK_FOR_KOBO)
	    "\t\t\t\tNOTE: Due to the way buffers are handled on that platform, this will not behave as expected on sunxi SoCs!\n"
#	endif
	    "\t-w, --wait\t\tBlock until the kernel has finished processing the *last* update we sent, if any.\n"
	    "\t\t\t\tThe actual delay depends for the most part on the waveform mode that was used.\n"
	    "\t\t\t\tSee the API documentation around fbink_wait_for_submission & fbink_wait_for_complete for more details.\n"
	    "\t\t\t\tAs a point of reference, eips only does a wait_for_complete after the flashing refresh of an image.\n"
	    "\t\t\t\tWe used to do that by default for *all* flashing updates until FBInk 1.20.0.\n"
#endif    //!FBINK_FOR_LINUX
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
	    "\t\t\t\t\t\tLEGGIE, VEGGIE, KATES, FKP, CTRLD, ORP, ORPB, ORPI, SCIENTIFICA, SCIENTIFICAB, SCIENTIFICAI, TERMINUS, TERMINUSB, FATTY, SPLEEN, TEWI, TEWIB, TOPAZ, MICROKNIGHT, VGA, COZETTE\n"
#	ifdef FBINK_WITH_UNIFONT
	    "\t\t\t\t\t\tAs well as UNIFONT & UNIFONTDW\n"
#	endif
	    "\t\t\t\tNOTE: On low dpi, 600x800 devices, ORP or TEWI's form factor may feel more familiar at default scaling.\n"
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
	    "\t\t\t\tObviously mutually exclusive with -o, --overlay, because it's simply a subset of what overlay does. If both are enabled, -o, --overlay takes precedence.\n"
	    "\t-T, --fgless\t\tDon't draw foreground pixels.\n"
	    "\t\t\t\tMutually exclusive with -o, --overlay or -O, --bgless, and takes precedence over them.\n"
#if defined(FBINK_FOR_KOBO)
	    "\n"
	    "\t\t\t\tNOTE: Due to the way buffers are handled on that platform, -o, --overlay; -O, --bgless and -T, --fgless will not behave as expected on sunxi SoCs!\n"
#endif
	    "\n"
	    "Options affecting the program's verbosity:\n"
	    "\t-v, --verbose\tToggle printing diagnostic messages.\n"
	    "\t-q, --quiet\tToggle hiding hardware setup messages.\n"
	    "\t-G, --syslog\tSend output to syslog instead of stdout & stderr.\n"
	    "\t\t\tOught to be the first flag passed, otherwise, some commandline parsing errors might not honor it.\n"
	    "\n"
	    "Options affecting the program's behavior:\n"
	    "\t-I, --interactive\tEnter a very basic interactive mode.\n"
	    "\t-L, --linecountcode\tWhen successfully printing text, returns the total amount of printed lines as the process exit code.\n"
	    "\t\t\t\tNOTE: Will be inaccurate if there are more than 255 rows on screen!\n"
	    "\t-l, --linecount\t\tWhen successfully printing text, outputs the total amount of printed lines in the final line of output to stdout (NOTE: enforces quiet & non-verbose!).\n"
	    "\t\t\t\tNOTE: With OT/TTF rendering, will output a top margin value to use as-is instead (or 0 if there's no space left on screen)!\n"
	    "\t\t\t\t      The OT/TTF codepath also returns more data, including the results of the line-breaking computations, so it's in an eval-friendly format instead.\n"
	    "\t-E, --coordinates\tWhen printing something, outputs the coordinates & dimensions of what was printed to stdout, in a format easily consumable by eval (NOTE: enforces quiet & non-verbose!).\n"
	    "\t\t\t\tNOTE: For both -l, --linecount & -E, --coordinates, output will only be sent to stdout on *success*. On error, the usual error message is sent to stderr.\n"
	    "\t\t\t\t      Given that, you may want to store stdout only in a variable and check the return code for success before running eval on that var!\n"
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
	    "\tIf you want to properly print a long string, better do it in a single argument, FBInk will do its best to spread it over multiple lines sanely.\n"
	    "\tIt will also honor the linefeed character (and I do mean the actual control character, not the human-readable escape sequence),\n"
	    "\twhich comes in handy when passing a few lines of logs straight from tail as an argument.\n"
#ifdef FBINK_WITH_OPENTYPE
	    "\n"
	    "\n"
	    "\n"
	    "OpenType & TrueType font support:\n"
	    "\t-t, --truetype regular=FILE,bold=FILE,italic=FILE,bolditalic=FILE,size=NUM,px=NUM,top=NUM,bottom=NUM,left=NUM,right=NUM,padding=PAD,style=STYLE,format,notrunc,compute\n"
	    "\t\tregular, bold, italic & bolditalic should point to the font file matching their respective font style. At least one of them MUST be specified.\n"
	    "\t\tsize sets the rendering size, in points. Defaults to 12pt if unset. Can be a decimal value.\n"
	    "\t\tpx sets the rendering size, in pixels. Optional. Takes precedence over size if specified.\n"
	    "\t\ttop, bottom, left & right set the margins used to define the display area. Defaults to 0, i.e., the full screen, starting at the top-left corner.\n"
	    "\t\t\tNOTE: If a negative value is supplied, counts backward from the opposite edge. Mostly useful with top & left to position stuff relative to the bottom right corner.\n"
	    "\t\tpadding can optionally be set to ensure the drawing area on both sides of the printed content is padded with the background color on one or both axis.\n"
	    "\t\t\tAvailable padding axis: HORIZONTAL, VERTICAL, or BOTH (Defaults to NONE). Useful to avoid overlaps on consecutive prints at the same coordinates.\n"
	    "\t\tIf style is specified, it dictates the default font style to use (e.g., REGULAR, BOLD, ITALIC or BOLD_ITALIC). Defaults to REGULAR.\n"
	    "\t\tIf format is specified, instead of the default style, the underscore/star Markdown syntax will be honored to set the font style (i.e., *italic*, **bold** & ***bold italic***).\n"
	    "\t\tIf notrunc is specified, truncation will be considered a failure.\n"
	    "\t\tNOTE: This may not prevent drawing/refreshing the screen if the truncation couldn't be predicted at compute time!\n"
	    "\t\t      On the CLI, this will prevent you from making use of the returned computation info, as this will chain a CLI abort.\n"
	    "\t\tIf compute is specified, no rendering will be done, and only the line-breaking computation pass will run. You'll generally want to use that combined with -l, --linecount.\n"
	    "\n"
	    "\t\tHonors -h, --invert; -f, --flash; -c, --clear; -W, --waveform; -D, --dither; -H, --nightmode; -b, --norefresh; -m, --centered; -M, --halfway; -o, --overlay; -T, --fgless; -O, --bgless; -C, --color; -B, --background; -l, --linecount\n"
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
	    "\t\tHonoring the Markdown syntax, 'Hello' will be printed with the Regular font style, while 'world' will use the Bold font style.\n"
	    "\t\tNOTE: You will NOT be able to use obfuscated or encrypted fonts.\n"
#	ifdef FBINK_FOR_KOBO
	    "\t\tPlease note that this means you will NOT be able to use system fonts, because they're all obfuscated.\n"
#	else
	    "\t\tAlso note that the path shown here is fairly arbitrary, and Kobo-specific ;).\n"
#	endif
	    "\n"
#endif
	    "\n"
	    "\n"
	    "\n"
	    "\n"
	    "You can also eschew printing a STRING, and simply refresh the screen as per your specification, without touching the framebuffer:\n"
	    "\t-s, --refresh [top=NUM,left=NUM,width=NUM,height=NUM]\n"
	    "\n"
	    "EXAMPLES:\n"
	    "\tfbink -s top=20,left=10,width=500,height=600 -W GC16 -D ORDERED\n"
	    "\t\tRefreshes a 500x600 rectangle with its top-left corner at coordinates (10, 20) with a GC16 waveform mode and ORDERED hardware dithering.\n"
	    "\n"
	    "NOTES:\n"
	    "\tThe specified rectangle *must* completely fit on screen, or the ioctl will fail.\n"
	    "\tNote that this will also honor -W, --waveform; -H, --nightmode & -f, --flash\n"
#if defined(FBINK_FOR_KOBO) || defined(FBINK_FOR_CERVANTES)
	    "\tNote that the arguments are passed as-is to the ioctl, no viewport or rotation quirks are applied!\n"
#endif
	    "\tIf you just want a full-screen refresh (which will honor -f, --flash), don't pass any suboptions,\n"
	    "\te.g., fbink -s (if you group short options together, it needs to be the last in its group, i.e., -fs and not -sf).\n"
#if defined(FBINK_FOR_KOBO)
	    "\n"
	    "\tNOTE: Due to the way buffers are handled on that platform, this will not behave as expected on sunxi SoCs!\n"
	    "\tYou'll just end up with solid black inside your refresh region.\n"
#endif
	    "\n"
	    "\tSpecifying one or more STRING takes precedence over this mode.\n"
	    "\n"
	    "\n"
	    "A variant of the above allows you to also clear specific rectangular regions of the screen:\n"
	    "\t-k, --cls [top=NUM,left=NUM,width=NUM,height=NUM]\n"
	    "\t\tClear the screen (or a region of it), and abort early.\n"
	    "\t\tHonors -B, --background; -h, --invert; -H, --nightmode; -W, --waveform; -D, --dither; -b, --norefresh; -w, --wait.\n"
	    "\t\tThis takes precedence over *everything* and will abort as soon as it's done.\n"
	    "\t\tIf you just want a full-screen clear (which will honor -f, --flash), don't pass any suboptions,\n"
	    "\t\te.g., fbink -k (if you group short options together, it needs to be the last in its group, i.e., -fk and not -kf).\n"
	    "\t\tYou can use this to effectively paint arbitrary background colored rectangular shapes.\n"
	    "\n"
#ifdef FBINK_WITH_IMAGE
	    "\n"
	    "\n"
	    "\n"
	    "\n"
	    "You can also eschew printing a STRING, and print an IMAGE at the requested coordinates instead:\n"
	    "\t-g, --image file=PATH,x=NUM,y=NUM,halign=ALIGN,valign=ALIGN,w=NUM,h=NUM,dither [-i, --img PATH]\n"
	    "\t\tSupported ALIGN values: NONE (or LEFT for halign, TOP for valign), CENTER or MIDDLE, EDGE (or RIGHT for halign, BOTTOM for valign).\n"
	    "\t\tIf dither is specified, *software* dithering (ordered, 8x8) will be applied to the image, ensuring it'll match the eInk palette exactly.\n"
	    "\t\tThis is *NOT* mutually exclusive with -D, --dither!\n"
	    "\t\tw & h *may* be used to request scaling. If one of them is set to 0, aspect ratio will be respected.\n"
	    "\t\tSet to -1 to request the viewport's dimension for that side.\n"
	    "\t\tIf either side is set to something lower than -1, the image will be scaled to the largest possible dimension that fits on screen while honoring the original aspect ratio.\n"
	    "\t\tThey both default to 0, meaning no scaling will be done.\n"
	    "\n"
	    "EXAMPLES:\n"
	    "\tfbink -g file=hello.png\n"
	    "\t\tDisplays the image \"hello.png\", starting at the top left of the screen.\n"
	    "\tfbink -i hello,world.png -g x=-10,y=11 -x 5 -y 8\n"
	    "\t\tDisplays the image \"hello,world.png\", starting at the ninth line plus 11px and the sixth column minus 10px.\n"
	    "\tfbink -g file=hello.png,halign=EDGE,valign=CENTER\n"
	    "\t\tDisplays the image \"hello.png\", in the middle of the screen, aligned to the right edge.\n"
	    "\tfbink -g file=hello.png -W A2\n"
	    "\t\tDisplays the image \"hello.png\", in monochrome.\n"
	    "\tfbink -i wheee.png\n"
	    "\t\tDisplays the image \"wheee.png\" with the default settings.\n"
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
	    "\tThis honors -f, --flash, as well as -c, --clear; -W, --waveform; -D, --dither; -H, --nightmode; -b, --norefresh & -h, --invert\n"
	    "\t\tNote that this also honors -x, --col & -y, --row (taking -S, --size into account), in addition to the coordinates you specify.\n"
	    "\t\tThe aim is to make it easier to align small images to text.\n"
	    "\t\tAnd to make pixel-perfect adjustments, you can also specifiy negative values for x & y.\n"
	    "\tSpecifying one or more STRING takes precedence over this mode.\n"
	    "\t-s, --refresh also takes precedence over this mode.\n"
#endif
	    "\n"
	    "\n"
	    "\n"
	    "\n"
	    "NOTES:\n"
	    "\tShell script writers can also use the -e, --eval flag to have FBInk just spit out a few of its internal state variables to stdout,\n"
	    "\t\te.g., eval $(fbink -e)\n"
	    "\n"
	    "\n"
	    "NOTES:\n"
	    "\tFor more complex & long-running use-cases involving *text* only (or a progress/activity bar), you can also switch to daemon mode, via -d, --daemon NUM_LINES\n"
	    "\tIt expects a single argument: the amount of lines consecutive prints can occupy before wrapping back to the original coordinates.\n"
	    "\tIt it's set to 0, the behavior matches what usually happens when you pass multiple strings to FBInk (i.e., the only wrapping happens at screen egde).\n"
	    "\tWhile, for example, setting it to 1 will ensure every print will start at the same coordinates.\n"
	    "\tIn this mode, FBInk will daemonize instantly, and then print its PID to stdout. You should consume stdout, and check the return code:\n"
	    "\tif it's 0, then you have a guarantee that what you've grabbed from stdout is *strictly* a PID.\n"
	    "\tYou can then send a kill -0 to that PID to check for an early abort.\n"
	    "\tBy default, it will create a named pipe for IPC: " FBINK_PIPE
	    " (if the file already exists, whatever type it may be, FBInk will abort).\n"
	    "\tYou can ask for a custom path by setting FBINK_NAMED_PIPE to an absolute path in your environment.\n"
	    "\tCreating and removing that FIFO is FBInk's responsibility. Don't create it yourself.\n"
	    "\tMake sure you kill FBInk via SIGTERM so it has a chance to remove it itself on exit.\n"
	    "\t(Otherwise, you may want to ensure that it doesn't already exist *before* launching a daemon mode session).\n"
	    "\tWith the technicalities out of the way, it's then as simple as writing to that pipe for stuff to show up on screen ;).\n"
	    "\te.g., echo -n 'Hello World!' > " FBINK_PIPE
	    "\n"
	    "\tRemember that LFs are honored!\n"
	    "\tAlso, the daemon will NOT abort on FBInk errors, and it redirects stdout & stderr to /dev/null, so errors & bogus input will be silently ignored!\n"
	    "\tIt can abort on early setup errors, though, before *or* after having redirected stderr...\n"
	    "\tIt does enforce logging to the syslog, though, but, again, early commandline parsing errors may still be sent to stderr...\n"
	    "\n");

	return;
}

// Fun helpers for the daemon mode...
static void
    cleanup_handler(int signum __attribute__((unused)), siginfo_t* siginfo, void* context __attribute__((unused)))
{
	// Our main loop handles EINTR, and will abort cleanly once it sees that flag
	g_timeToDie       = 1;
	// NOTE: I have no idea how long that pointer is supposed to be safe to use, so, make a copy of the fields we care about.
	g_sigCaught.signo = siginfo->si_signo;
	g_sigCaught.pid   = siginfo->si_pid;
	g_sigCaught.uid   = siginfo->si_uid;
}

// Because daemon() only appeared in glibc 2.21 (and doesn't double-fork anyway)
static int
    daemonize(void)
{
	switch (fork()) {
		case -1:
			PFWARN("initial fork: %m");
			return -1;
		case 0:
			break;
		default:
			_exit(EXIT_SUCCESS);
	}

	if (setsid() == -1) {
		PFWARN("setsid: %m");
		return -1;
	}

	// Double fork, for... reasons!
	// In practical terms, this ensures we get re-parented to init *now*.
	// Ignore SIGHUP while we're there, since we don't want to be killed by it.
	struct sigaction sa = { .sa_handler = SIG_IGN, .sa_flags = SA_RESTART };
	if (sigaction(SIGHUP, &sa, NULL) == -1) {
		PFWARN("sigaction: %m");
		return -1;
	}
	switch (fork()) {
		case -1:
			PFWARN("final fork: %m");
			return -1;
		case 0:
			break;
		default:
			_exit(EXIT_SUCCESS);
	}

	if (chdir("/tmp") == -1) {
		PFWARN("chdir: %m");
		return -1;
	}

	// Make sure we keep honoring rcS's umask
	umask(022);    // Flawfinder: ignore

	// Before we close the shop down, output our final PID to stdout...
	fprintf(stdout, "%ld\n", (long) getpid());
	fflush(stdout);

	// Redirect stdin, stdout & stderr to /dev/null
	int fd = open("/dev/null", O_RDWR);
	if (fd != -1) {
		dup2(fd, fileno(stdin));
		dup2(fd, fileno(stdout));
		dup2(fd, fileno(stderr));
		if (fd > 3) {
			close(fd);
		}
	} else {
		PFWARN("Failed to redirect stdin, stdout & stderr to /dev/null (open: %m)");
		return -1;
	}

	return 0;
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
		  const FBInkConfig* fbink_cfg,
		  FBInkOTConfig*     ot_config)
{
	if (reg_ot_file) {
		if (!fbink_cfg->is_quiet) {
			LOG("Loading font '%s' for the Regular style", reg_ot_file);
		}
		if (fbink_add_ot_font(reg_ot_file, FNT_REGULAR) < 0) {
			WARN("Failed to open font file '%s'", reg_ot_file);
		}
	}
	if (bd_ot_file) {
		if (!fbink_cfg->is_quiet) {
			LOG("Loading font '%s' for the Bold style", bd_ot_file);
		}
		if (fbink_add_ot_font(bd_ot_file, FNT_BOLD) < 0) {
			WARN("Failed to open font file '%s'", bd_ot_file);
		}
	}
	if (it_ot_file) {
		if (!fbink_cfg->is_quiet) {
			LOG("Loading font '%s' for the Italic style", it_ot_file);
		}
		if (fbink_add_ot_font(it_ot_file, FNT_ITALIC) < 0) {
			WARN("Failed to open font file '%s'", it_ot_file);
		}
	}
	if (bdit_ot_file) {
		if (!fbink_cfg->is_quiet) {
			LOG("Loading font '%s' for the Bold Italic style", bdit_ot_file);
		}
		if (fbink_add_ot_font(bdit_ot_file, FNT_BOLD_ITALIC) < 0) {
			WARN("Failed to open font file '%s'", bdit_ot_file);
		}
	}

	// NOTE: Backward compatibility to match the behavior of version < 1.22.3 in most cases:
	//       if a *single* non-Regular style has been loaded, make it the default style.
	if (bd_ot_file && !reg_ot_file && !it_ot_file && !bdit_ot_file) {
		ot_config->style = FNT_BOLD;
		LOG("The only loaded font style was Bold: make it the default style!");
	} else if (it_ot_file && !reg_ot_file && !bd_ot_file && !bdit_ot_file) {
		ot_config->style = FNT_ITALIC;
		LOG("The only loaded font style was Italic: make it the default style!");
	} else if (bdit_ot_file && !reg_ot_file && !bd_ot_file && !it_ot_file) {
		ot_config->style = FNT_BOLD_ITALIC;
		LOG("The only loaded font style was Bold Italic: make it the default style!");
	}
}

// Small utility functions for want_lastrect
static void
    compute_lastrect(void)
{
	// No need to check for error, it will return {0, 0, 0, 0} on failure anyway ;).
	const FBInkRect           last_rect = fbink_get_last_rect(false);
	static unsigned short int max_y2    = 0U;

	// If that's the first call, simply use last_rect as-is
	if (totalRect.width == 0U && totalRect.height == 0U) {
		totalRect = last_rect;

		// Don't forget to keep track of the bottom of our first rectangle, though ;).
		const unsigned short int y2 = (unsigned short int) (last_rect.top + last_rect.height);
		max_y2                      = y2;
	} else {
		// Otherwise, build a rect that overlaps w/ every previous rects...
		totalRect.top   = (unsigned short int) MIN(totalRect.top, last_rect.top);
		totalRect.left  = (unsigned short int) MIN(totalRect.left, last_rect.left);
		totalRect.width = (unsigned short int) MAX(totalRect.width, last_rect.width);
		// Height is a wee bit trickier, as we *can* wraparound, so last_rect might be *above* totalRect...
		// Se we compute the absolute y coordinate of the bottom of both rectangles, keep the largest one,
		// and re-compute height.
		const unsigned short int y2 =
		    (unsigned short int) MAX(totalRect.top + totalRect.height, last_rect.top + last_rect.height);
		// Remember the largest y2 we ever encountered so we can compute height properly
		max_y2           = (unsigned short int) MAX(max_y2, y2);
		totalRect.height = (unsigned short int) (max_y2 - totalRect.top);
	}
}

static void
    recap_lastrect(void)
{
	printf("lastRect_Left=%hu;lastRect_Top=%hu;lastRect_Width=%hu;lastRect_Height=%hu;",
	       totalRect.left,
	       totalRect.top,
	       totalRect.width,
	       totalRect.height);
}

static void
    print_lastrect(void)
{
	// No need to check for error, it will return {0, 0, 0, 0} on failure anyway ;).
	const FBInkRect last_rect = fbink_get_last_rect(false);
	printf("lastRect_Left=%hu;lastRect_Top=%hu;lastRect_Width=%hu;lastRect_Height=%hu;",
	       last_rect.left,
	       last_rect.top,
	       last_rect.width,
	       last_rect.height);
}

// Input validation via strtoul, for an uint32_t
// Adapted from the same in KFMon ;).
static int
    strtoul_u(int opt, const char* subopt, const char* str, uint32_t* result)
{
	strtoul_chk(opt, subopt, str, result);
}

// Input validation via strtoul, for an uint16_t
static int
    strtoul_hu(int opt, const char* subopt, const char* str, uint16_t* result)
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

// In the same vein, but with strtof, for a *positive* float
static int
    strtof_pos(int opt, const char* subopt, const char* str, float* result)
{
	strtof_chk(opt, subopt, str, result);
}

// Application entry point
int
    main(int argc, char* argv[])
{
	int                        opt;
	// NOTE: getopt_long will only update opt_index when passed a *long* option,
	//       so we need to do the matching ourselves when we're passed *short* options, hence the sentinel value...
	int                        opt_index = -1;
	static const struct option opts[]    = {
                {          "row", required_argument, NULL, 'y'},
                {          "col", required_argument, NULL, 'x'},
                {      "voffset", required_argument, NULL, 'Y'},
                {      "hoffset", required_argument, NULL, 'X'},
                {       "invert",       no_argument, NULL, 'h'},
                {        "flash",       no_argument, NULL, 'f'},
                {        "clear",       no_argument, NULL, 'c'},
                {     "centered",       no_argument, NULL, 'm'},
                {      "halfway",       no_argument, NULL, 'M'},
                {       "padded",       no_argument, NULL, 'p'},
                {      "rpadded",       no_argument, NULL, 'r'},
                {      "refresh", optional_argument, NULL, 's'},
                {         "size", required_argument, NULL, 'S'},
                {         "font", required_argument, NULL, 'F'},
                {      "verbose",       no_argument, NULL, 'v'},
                {        "quiet",       no_argument, NULL, 'q'},
                {        "image", required_argument, NULL, 'g'},
                {          "img", required_argument, NULL, 'i'},
                {      "flatten",       no_argument, NULL, 'a'},
                {         "eval",       no_argument, NULL, 'e'},
                {  "interactive",       no_argument, NULL, 'I'},
                {        "color", required_argument, NULL, 'C'},
                {   "background", required_argument, NULL, 'B'},
                {"linecountcode",       no_argument, NULL, 'L'},
                {    "linecount",       no_argument, NULL, 'l'},
                {  "progressbar", required_argument, NULL, 'P'},
                {  "activitybar", required_argument, NULL, 'A'},
                {   "noviewport",       no_argument, NULL, 'V'},
                {      "overlay",       no_argument, NULL, 'o'},
                {       "bgless",       no_argument, NULL, 'O'},
                {       "fgless",       no_argument, NULL, 'T'},
                {     "truetype", required_argument, NULL, 't'},
                {    "norefresh",       no_argument, NULL, 'b'},
                {       "dither", optional_argument, NULL, 'D'},
                {     "waveform", required_argument, NULL, 'W'},
                {    "nightmode",       no_argument, NULL, 'H'},
                {  "coordinates",       no_argument, NULL, 'E'},
                {        "mimic",       no_argument, NULL, 'Z'},
                {     "koreader",       no_argument, NULL, 'z'},
                {          "cls", optional_argument, NULL, 'k'},
                {      "animate", required_argument, NULL, 'K'},
                {         "wait",       no_argument, NULL, 'w'},
                {       "daemon", required_argument, NULL, 'd'},
                {       "syslog",       no_argument, NULL, 'G'},
                {         "help",       no_argument, NULL, 'Q'},
                {           NULL,                 0, NULL,   0}
	};

	FBInkConfig fbink_cfg = { 0 };
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wmissing-braces"
	FBInkOTConfig ot_config = { 0 };
#pragma GCC diagnostic pop
	FBInkOTFit ot_fit = { 0 };

	enum
	{
		TOP_OPT = 0,
		LEFT_OPT,
		WIDTH_OPT,
		HEIGHT_OPT,
	};
	enum
	{
		FILE_OPT = 0,
		XOFF_OPT,
		YOFF_OPT,
		HALIGN_OPT,
		VALIGN_OPT,
		SCALED_WIDTH_OPT,
		SCALED_HEIGHT_OPT,
		SW_DITHER_OPT,
	};
	enum
	{
		REGULAR_OPT = 0,
		BOLD_OPT,
		ITALIC_OPT,
		BOLDITALIC_OPT,
		SIZE_OPT,
		PX_OPT,
		TM_OPT,
		BM_OPT,
		LM_OPT,
		RM_OPT,
		PADDING_OPT,
		FMT_OPT,
		COMPUTE_OPT,
		NOTRUNC_OPT,
		STYLE_OPT,
	};
	enum
	{
		DIRECTION_OPT = 0,
		STEPS_OPT,
	};
#pragma GCC diagnostic   push
#pragma GCC diagnostic   ignored "-Wunknown-pragmas"
#pragma clang diagnostic ignored "-Wunknown-warning-option"
#pragma GCC diagnostic   ignored "-Wdiscarded-qualifiers"
#pragma clang diagnostic ignored "-Wincompatible-pointer-types-discards-qualifiers"
	char* const refresh_token[] = {
		[TOP_OPT] = "top", [LEFT_OPT] = "left", [WIDTH_OPT] = "width", [HEIGHT_OPT] = "height", NULL
	};
	char* const image_token[]    = { [FILE_OPT] = "file",       [XOFF_OPT] = "x",           [YOFF_OPT] = "y",
					 [HALIGN_OPT] = "halign",   [VALIGN_OPT] = "valign",    [SCALED_WIDTH_OPT] = "w",
					 [SCALED_HEIGHT_OPT] = "h", [SW_DITHER_OPT] = "dither", NULL };
	char* const truetype_token[] = { [REGULAR_OPT] = "regular", [BOLD_OPT] = "bold",
					 [ITALIC_OPT] = "italic",   [BOLDITALIC_OPT] = "bolditalic",
					 [SIZE_OPT] = "size",       [PX_OPT] = "px",
					 [TM_OPT] = "top",          [BM_OPT] = "bottom",
					 [LM_OPT] = "left",         [RM_OPT] = "right",
					 [PADDING_OPT] = "padding", [FMT_OPT] = "format",
					 [COMPUTE_OPT] = "compute", [NOTRUNC_OPT] = "notrunc",
					 [STYLE_OPT] = "style",     NULL };
	// Recycle the refresh enum ;).
	char* const cls_token[]      = {
                [TOP_OPT] = "top", [LEFT_OPT] = "left", [WIDTH_OPT] = "width", [HEIGHT_OPT] = "height", NULL
	};
	char* const anim_token[] = { [DIRECTION_OPT] = "direction", [STEPS_OPT] = "steps", NULL };
#pragma GCC diagnostic pop
	char*                       full_subopts = NULL;
	char*                       subopts;
	char*                       value          = NULL;
	uint32_t                    region_top     = 0;
	uint32_t                    region_left    = 0;
	uint32_t                    region_width   = 0;
	uint32_t                    region_height  = 0;
	char*                       hwd_name       = NULL;
	char*                       wfm_name       = NULL;
	bool                        is_refresh     = false;
	char*                       image_file     = NULL;
	short int                   image_x_offset = 0;
	short int                   image_y_offset = 0;
	bool                        is_image       = false;
	bool                        is_eval        = false;
	bool                        is_interactive = false;
	bool                        want_linecode  = false;
	bool                        want_linecount = false;
	bool                        want_lastrect  = false;
	bool                        is_progressbar = false;
	bool                        is_activitybar = false;
	bool                        is_infinite    = false;
	bool                        is_mimic       = false;
	bool                        is_koreader    = false;
	bool                        is_cls         = false;
	bool                        is_help        = false;
	const char*                 pipe_path      = NULL;
	bool                        is_daemon      = false;
	uint8_t                     daemon_lines   = 0U;
	bool                        wait_for       = false;
	uint8_t                     progress       = 0;
	bool                        is_truetype    = false;
	char*                       reg_ot_file    = NULL;
	char*                       bd_ot_file     = NULL;
	char*                       it_ot_file     = NULL;
	char*                       bdit_ot_file   = NULL;
	// Default to a 12 steps right-to-left swipe, àla Malbec.
	MTK_SWIPE_DIRECTION_INDEX_T direction      = MTK_SWIPE_DIR_LEFT;
	uint8_t                     steps          = 12U;
	bool                        errfnd         = false;

	// NOTE: c.f., https://codegolf.stackexchange.com/q/148228 to sort this mess when I need to find an available letter ;p
	//       In fact, that's the current tally of alnum entries left: JjNnRUu
	while (
	    (opt = getopt_long(
		 argc, argv, "y:x:Y:X:hfcmMprs::S:F:vqg:i:aeIC:B:LlP:A:oOTVt:bD::W:HEZzk::wd:GQK:", opts, &opt_index)) !=
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
			case 's': {
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
				} else {
					opt_longname = opts[opt_index].name;
				}

				// NOTE: Nasty bit of trickery to make getopt's optional_argument actually useful...
				//       Hat trick (& explanation) courtesy of https://stackoverflow.com/a/32575314
				//       If `optarg` isn't set and argv[optind] doesn't look like another option,
				//       then assume it's our parameter and overtly modify optind to compensate.
				if (!optarg && argv[optind] != NULL && argv[optind][0] != '-') {
					subopts = argv[optind++];
				} else {
					subopts = optarg;
				}

				// NOTE: We'll need to remember the original, full suboption string for diagnostic messages,
				//       because getsubopt will rewrite it during processing...
				if (subopts && *subopts != '\0') {
					// Only remember the first offending suboption list...
					if (!errfnd) {
						full_subopts = strdupa(subopts);    // lgtm [cpp/alloca-in-loop]
					}
				}

				while (subopts && *subopts != '\0' && !errfnd) {
					switch (getsubopt(&subopts, refresh_token, &value)) {
						case TOP_OPT:
							if (value == NULL) {
								ELOG("Missing value for suboption '%s' of -%c, --%s",
								     refresh_token[TOP_OPT],
								     opt,
								     opt_longname);
								errfnd = true;
								break;
							}
							if (strtoul_u(opt, refresh_token[TOP_OPT], value, &region_top) <
							    0) {
								errfnd = true;
							}
							break;
						case LEFT_OPT:
							if (value == NULL) {
								ELOG("Missing value for suboption '%s' of -%c, --%s",
								     refresh_token[LEFT_OPT],
								     opt,
								     opt_longname);
								errfnd = true;
								break;
							}
							if (strtoul_u(opt, refresh_token[LEFT_OPT], value, &region_left) <
							    0) {
								errfnd = true;
							}
							break;
						case WIDTH_OPT:
							if (value == NULL) {
								ELOG("Missing value for suboption '%s' of -%c, --%s",
								     refresh_token[WIDTH_OPT],
								     opt,
								     opt_longname);
								errfnd = true;
								break;
							}
							if (strtoul_u(
								opt, refresh_token[WIDTH_OPT], value, &region_width) <
							    0) {
								errfnd = true;
							}
							break;
						case HEIGHT_OPT:
							if (value == NULL) {
								ELOG("Missing value for suboption '%s' of -%c, --%s",
								     refresh_token[HEIGHT_OPT],
								     opt,
								     opt_longname);
								errfnd = true;
								break;
							}
							if (strtoul_u(
								opt, refresh_token[HEIGHT_OPT], value, &region_height) <
							    0) {
								errfnd = true;
							}
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
				// Make sure we won't pass an invalid rectangle to the driver, because that'd soft lock.
				if ((region_height == 0 || region_width == 0) &&
				    !(region_top == 0 && region_left == 0 && region_height == 0 && region_width == 0)) {
					ELOG(
					    "Non-zero values must be specified for suboptions '%s' and '%s' of -%c, --%s",
					    refresh_token[HEIGHT_OPT],
					    refresh_token[WIDTH_OPT],
					    opt,
					    opt_longname);
					errfnd = true;
				} else {
					is_refresh = true;
				}
				// Only remember this if there was a parsing error.
				if (!errfnd) {
					full_subopts = NULL;
				}
				break;
			}
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
				} else if (strcasecmp(optarg, "TERMINUS") == 0) {
					fbink_cfg.fontname = TERMINUS;
				} else if (strcasecmp(optarg, "TERMINUSB") == 0) {
					fbink_cfg.fontname = TERMINUSB;
				} else if (strcasecmp(optarg, "FATTY") == 0) {
					fbink_cfg.fontname = FATTY;
				} else if (strcasecmp(optarg, "SPLEEN") == 0) {
					fbink_cfg.fontname = SPLEEN;
				} else if (strcasecmp(optarg, "TEWI") == 0) {
					fbink_cfg.fontname = TEWI;
				} else if (strcasecmp(optarg, "TEWIB") == 0) {
					fbink_cfg.fontname = TEWIB;
				} else if (strcasecmp(optarg, "TOPAZ") == 0) {
					fbink_cfg.fontname = TOPAZ;
				} else if (strcasecmp(optarg, "MICROKNIGHT") == 0) {
					fbink_cfg.fontname = MICROKNIGHT;
				} else if (strcasecmp(optarg, "VGA") == 0) {
					fbink_cfg.fontname = VGA;
				} else if (strcasecmp(optarg, "UNIFONT") == 0) {
					fbink_cfg.fontname = UNIFONT;
				} else if (strcasecmp(optarg, "UNIFONTDW") == 0) {
					fbink_cfg.fontname = UNIFONTDW;
				} else if (strcasecmp(optarg, "COZETTE") == 0) {
					fbink_cfg.fontname = COZETTE;
				} else {
					ELOG("Unknown font name '%s'.", optarg);
					errfnd = true;
				}
				break;
			case 'v':
				fbink_cfg.is_verbose = !fbink_cfg.is_verbose;
				break;
			case 'q':
				fbink_cfg.is_quiet = !fbink_cfg.is_quiet;
				break;
			case 'g': {
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
				} else {
					opt_longname = opts[opt_index].name;
				}

				subopts = optarg;
				// NOTE: We'll need to remember the original, full suboption string for diagnostic messages,
				//       because getsubopt will rewrite it during processing...
				if (subopts && *subopts != '\0') {
					// Only remember the first offending suboption list...
					if (!errfnd) {
						full_subopts = strdupa(subopts);    // lgtm [cpp/alloca-in-loop]
					}
				}
				// NOTE: I'm not terribly fond of getsubopt in general, especially here with the comma limitation
				//       for filenames, but it does make sense to keep image-specific options separate...
				//       The same argument could be made against moving x_off/y_off to hoffset/voffset now
				//       that we have those in FBInkConfig... (Plus, getting rid of that would break ABI compat).
				while (subopts && *subopts != '\0' && !errfnd) {
					switch (getsubopt(&subopts, image_token, &value)) {
						case FILE_OPT:
							if (value == NULL) {
								ELOG("Missing value for suboption '%s' of -%c, --%s",
								     image_token[FILE_OPT],
								     opt,
								     opt_longname);
								errfnd = true;
								break;
							}
							image_file = value;
							break;
						case XOFF_OPT:
							if (value == NULL) {
								ELOG("Missing value for suboption '%s' of -%c, --%s",
								     image_token[XOFF_OPT],
								     opt,
								     opt_longname);
								errfnd = true;
								break;
							}
							if (strtol_hi(
								opt, image_token[XOFF_OPT], value, &image_x_offset) < 0) {
								errfnd = true;
							}
							break;
						case YOFF_OPT:
							if (value == NULL) {
								ELOG("Missing value for suboption '%s' of -%c, --%s",
								     image_token[YOFF_OPT],
								     opt,
								     opt_longname);
								errfnd = true;
								break;
							}
							if (strtol_hi(
								opt, image_token[YOFF_OPT], value, &image_y_offset) < 0) {
								errfnd = true;
							}
							break;
						case HALIGN_OPT:
							if (value == NULL) {
								ELOG("Missing value for suboption '%s' of -%c, --%s",
								     image_token[HALIGN_OPT],
								     opt,
								     opt_longname);
								errfnd = true;
								break;
							}
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
								ELOG("Unknown alignment value '%s'.", value);
								errfnd = true;
							}
							break;
						case VALIGN_OPT:
							if (value == NULL) {
								ELOG("Missing value for suboption '%s' of -%c, --%s",
								     image_token[VALIGN_OPT],
								     opt,
								     opt_longname);
								errfnd = true;
								break;
							}
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
								ELOG("Unknown alignment value '%s'.", value);
								errfnd = true;
							}
							break;
						case SCALED_WIDTH_OPT:
							if (value == NULL) {
								ELOG("Missing value for suboption '%s' of -%c, --%s",
								     image_token[SCALED_WIDTH_OPT],
								     opt,
								     opt_longname);
								errfnd = true;
								break;
							}
							if (strtol_hi(opt,
								      image_token[SCALED_WIDTH_OPT],
								      value,
								      &fbink_cfg.scaled_width) < 0) {
								errfnd = true;
							}
							break;
						case SCALED_HEIGHT_OPT:
							if (value == NULL) {
								ELOG("Missing value for suboption '%s' of -%c, --%s",
								     image_token[SCALED_HEIGHT_OPT],
								     opt,
								     opt_longname);
								errfnd = true;
								break;
							}
							if (strtol_hi(opt,
								      image_token[SCALED_HEIGHT_OPT],
								      value,
								      &fbink_cfg.scaled_height) < 0) {
								errfnd = true;
							}
							break;
						case SW_DITHER_OPT:
							fbink_cfg.sw_dithering = true;
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
				// NOTE: We delay checking image_file to allow using -i both before AND after -g,
				//       or even on its own!
				is_image = true;
				// Only remember this if there was a parsing error.
				if (!errfnd) {
					full_subopts = NULL;
				}
				break;
			}
			case 'i':
				image_file = optarg;
				is_image   = true;
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
					ELOG("Unknown color name '%s'.", optarg);
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
					ELOG("Unknown color name '%s'.", optarg);
					errfnd = true;
				}
				break;
			case 'L':
				want_linecode = true;
				break;
			case 'l':
				want_linecount = true;
				break;
			case 'E':
				want_lastrect = true;
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
			case 't': {
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
				} else {
					opt_longname = opts[opt_index].name;
				}

				subopts = optarg;
				// NOTE: We'll need to remember the original, full suboption string for diagnostic messages,
				//       because getsubopt will rewrite it during processing...
				if (subopts && *subopts != '\0') {
					// Only remember the first offending suboption list...
					if (!errfnd) {
						full_subopts = strdupa(subopts);    // lgtm [cpp/alloca-in-loop]
					}
				}
				while (subopts && *subopts != '\0' && !errfnd) {
					switch (getsubopt(&subopts, truetype_token, &value)) {
						case REGULAR_OPT:
							if (value == NULL) {
								ELOG("Missing value for suboption '%s' of -%c, --%s",
								     truetype_token[REGULAR_OPT],
								     opt,
								     opt_longname);
								errfnd = true;
								break;
							}
							reg_ot_file = value;
							break;
						case BOLD_OPT:
							if (value == NULL) {
								ELOG("Missing value for suboption '%s' of -%c, --%s",
								     truetype_token[BOLD_OPT],
								     opt,
								     opt_longname);
								errfnd = true;
								break;
							}
							bd_ot_file = value;
							break;
						case ITALIC_OPT:
							if (value == NULL) {
								ELOG("Missing value for suboption '%s' of -%c, --%s",
								     truetype_token[ITALIC_OPT],
								     opt,
								     opt_longname);
								errfnd = true;
								break;
							}
							it_ot_file = value;
							break;
						case BOLDITALIC_OPT:
							if (value == NULL) {
								ELOG("Missing value for suboption '%s' of -%c, --%s",
								     truetype_token[BOLDITALIC_OPT],
								     opt,
								     opt_longname);
								errfnd = true;
								break;
							}
							bdit_ot_file = value;
							break;
						case SIZE_OPT:
							if (value == NULL) {
								ELOG("Missing value for suboption '%s' of -%c, --%s",
								     truetype_token[SIZE_OPT],
								     opt,
								     opt_longname);
								errfnd = true;
								break;
							}
							if (strtof_pos(opt,
								       truetype_token[SIZE_OPT],
								       value,
								       &ot_config.size_pt) < 0) {
								errfnd = true;
							}
							break;
						case PX_OPT:
							if (value == NULL) {
								ELOG("Missing value for suboption '%s' of -%c, --%s",
								     truetype_token[PX_OPT],
								     opt,
								     opt_longname);
								errfnd = true;
								break;
							}
							if (strtoul_hu(
								opt, truetype_token[PX_OPT], value, &ot_config.size_px) <
							    0) {
								errfnd = true;
							}
							break;
						case TM_OPT:
							if (value == NULL) {
								ELOG("Missing value for suboption '%s' of -%c, --%s",
								     truetype_token[TM_OPT],
								     opt,
								     opt_longname);
								errfnd = true;
								break;
							}
							if (strtol_hi(opt,
								      truetype_token[TM_OPT],
								      value,
								      &ot_config.margins.top) < 0) {
								errfnd = true;
							}
							break;
						case BM_OPT:
							if (value == NULL) {
								ELOG("Missing value for suboption '%s' of -%c, --%s",
								     truetype_token[BM_OPT],
								     opt,
								     opt_longname);
								errfnd = true;
								break;
							}
							if (strtol_hi(opt,
								      truetype_token[BM_OPT],
								      value,
								      &ot_config.margins.bottom) < 0) {
								errfnd = true;
							}
							break;
						case LM_OPT:
							if (value == NULL) {
								ELOG("Missing value for suboption '%s' of -%c, --%s",
								     truetype_token[LM_OPT],
								     opt,
								     opt_longname);
								errfnd = true;
								break;
							}
							if (strtol_hi(opt,
								      truetype_token[LM_OPT],
								      value,
								      &ot_config.margins.left) < 0) {
								errfnd = true;
							}
							break;
						case RM_OPT:
							if (value == NULL) {
								ELOG("Missing value for suboption '%s' of -%c, --%s",
								     truetype_token[RM_OPT],
								     opt,
								     opt_longname);
								errfnd = true;
								break;
							}
							if (strtol_hi(opt,
								      truetype_token[RM_OPT],
								      value,
								      &ot_config.margins.right) < 0) {
								errfnd = true;
							}
							break;
						case PADDING_OPT:
							if (value == NULL) {
								ELOG("Missing value for suboption '%s' of -%c, --%s",
								     truetype_token[PADDING_OPT],
								     opt,
								     opt_longname);
								errfnd = true;
								break;
							}
							if (strcasecmp(value, "NONE") == 0 ||
							    strcasecmp(value, "NO") == 0) {
								ot_config.padding = NO_PADDING;
							} else if (strcasecmp(value, "HORIZONTAL") == 0 ||
								   strcasecmp(value, "HORI") == 0) {
								ot_config.padding = HORI_PADDING;
							} else if (strcasecmp(value, "VERTICAL") == 0 ||
								   strcasecmp(value, "VERT") == 0) {
								ot_config.padding = VERT_PADDING;
							} else if (strcasecmp(value, "BOTH") == 0 ||
								   strcasecmp(value, "FULL") == 0) {
								ot_config.padding = FULL_PADDING;
							} else {
								ELOG("Unknown padding value '%s'.", value);
								errfnd = true;
							}
							break;
						case FMT_OPT:
							ot_config.is_formatted = true;
							break;
						case COMPUTE_OPT:
							ot_config.compute_only = true;
							break;
						case NOTRUNC_OPT:
							ot_config.no_truncation = true;
							break;
						case STYLE_OPT:
							if (value == NULL) {
								ELOG("Missing value for suboption '%s' of -%c, --%s",
								     truetype_token[STYLE_OPT],
								     opt,
								     opt_longname);
								errfnd = true;
								break;
							}
							if (strcasecmp(value, "REGULAR") == 0 ||
							    strcasecmp(value, "R") == 0) {
								ot_config.style = FNT_REGULAR;
							} else if (strcasecmp(value, "BOLD") == 0 ||
								   strcasecmp(value, "B") == 0) {
								ot_config.style = FNT_BOLD;
							} else if (strcasecmp(value, "ITALIC") == 0 ||
								   strcasecmp(value, "I") == 0) {
								ot_config.style = FNT_ITALIC;
							} else if (strcasecmp(value, "BOLDITALIC") == 0 ||
								   strcasecmp(value, "BOLD_ITALIC") == 0 ||
								   strcasecmp(value, "BOLD-ITALIC") == 0 ||
								   strcasecmp(value, "Z") == 0) {
								ot_config.style = FNT_BOLD_ITALIC;
							} else {
								ELOG("Unknown style value '%s'.", value);
								errfnd = true;
							}
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
				// Make sure we've passed at least one font style
				if (reg_ot_file == NULL && bd_ot_file == NULL && it_ot_file == NULL &&
				    bdit_ot_file == NULL) {
					WARN("At least one font style must be specified");
					errfnd = true;
				} else {
					is_truetype = true;
				}
				// Only remember this if there was a parsing error.
				if (!errfnd) {
					full_subopts = NULL;
				}
				break;
			}
			case 'b':
				fbink_cfg.no_refresh = true;
				break;
			case 'D':
				// NOTE: Nasty bit of trickery to make getopt's optional_argument actually useful...
				//       Hat trick (& explanation) courtesy of https://stackoverflow.com/a/32575314
				//       If `optarg` isn't set and argv[optind] doesn't look like another option,
				//       then assume it's our parameter and overtly modify optind to compensate.
				if (!optarg && argv[optind] != NULL && argv[optind][0] != '-') {
					optarg = argv[optind++];
				}

				if (!optarg) {
					// If no specific mode was passed, assume ORDERED
					fbink_cfg.dithering_mode = HWD_ORDERED;
				} else {
					if (strcasecmp(optarg, "PASSTHROUGH") == 0) {
						fbink_cfg.dithering_mode = HWD_PASSTHROUGH;
					} else if (strcasecmp(optarg, "FLOYD_STEINBERG") == 0) {
						fbink_cfg.dithering_mode = HWD_FLOYD_STEINBERG;
					} else if (strcasecmp(optarg, "ATKINSON") == 0) {
						fbink_cfg.dithering_mode = HWD_ATKINSON;
					} else if (strcasecmp(optarg, "ORDERED") == 0) {
						fbink_cfg.dithering_mode = HWD_ORDERED;
					} else if (strcasecmp(optarg, "QUANT_ONLY") == 0) {
						fbink_cfg.dithering_mode = HWD_QUANT_ONLY;
					} else if (strcasecmp(optarg, "LEGACY") == 0) {
						fbink_cfg.dithering_mode = HWD_LEGACY;
					} else {
						ELOG("Unknown hardware dithering algorithm '%s'.", optarg);
						errfnd = true;
					}
					// Remember non-default values in a human-readable format...
					if (fbink_cfg.dithering_mode != HWD_PASSTHROUGH) {
						hwd_name = optarg;
					}
				}
				break;
			case 'W':
				if (strcasecmp(optarg, "AUTO") == 0) {
					fbink_cfg.wfm_mode = WFM_AUTO;
				} else if (strcasecmp(optarg, "DU") == 0) {
					fbink_cfg.wfm_mode = WFM_DU;
				} else if (strcasecmp(optarg, "GC16") == 0) {
					fbink_cfg.wfm_mode = WFM_GC16;
				} else if (strcasecmp(optarg, "GC4") == 0) {
					fbink_cfg.wfm_mode = WFM_GC4;
				} else if (strcasecmp(optarg, "A2") == 0) {
					fbink_cfg.wfm_mode = WFM_A2;
				} else if (strcasecmp(optarg, "GL16") == 0) {
					fbink_cfg.wfm_mode = WFM_GL16;
				} else if (strcasecmp(optarg, "REAGL") == 0) {
					fbink_cfg.wfm_mode = WFM_REAGL;
				} else if (strcasecmp(optarg, "REAGLD") == 0) {
					fbink_cfg.wfm_mode = WFM_REAGLD;
				} else if (strcasecmp(optarg, "GC16_FAST") == 0) {
					fbink_cfg.wfm_mode = WFM_GC16_FAST;
				} else if (strcasecmp(optarg, "GL16_FAST") == 0) {
					fbink_cfg.wfm_mode = WFM_GL16_FAST;
				} else if (strcasecmp(optarg, "DU4") == 0) {
					fbink_cfg.wfm_mode = WFM_DU4;
				} else if (strcasecmp(optarg, "GL4") == 0) {
					fbink_cfg.wfm_mode = WFM_GL4;
				} else if (strcasecmp(optarg, "GL16_INV") == 0) {
					fbink_cfg.wfm_mode = WFM_GL16_INV;
				} else if (strcasecmp(optarg, "GCK16") == 0) {
					fbink_cfg.wfm_mode = WFM_GCK16;
				} else if (strcasecmp(optarg, "GLKW16") == 0) {
					fbink_cfg.wfm_mode = WFM_GLKW16;
				} else if (strcasecmp(optarg, "INIT") == 0) {
					fbink_cfg.wfm_mode = WFM_INIT;
				} else if (strcasecmp(optarg, "UNKNOWN") == 0) {
					fbink_cfg.wfm_mode = WFM_UNKNOWN;
				} else if (strcasecmp(optarg, "INIT2") == 0) {
					fbink_cfg.wfm_mode = WFM_INIT2;
				} else if (strcasecmp(optarg, "A2IN") == 0) {
					fbink_cfg.wfm_mode = WFM_A2IN;
				} else if (strcasecmp(optarg, "A2OUT") == 0) {
					fbink_cfg.wfm_mode = WFM_A2OUT;
				} else if (strcasecmp(optarg, "GC16HQ") == 0) {
					fbink_cfg.wfm_mode = WFM_GC16HQ;
				} else if (strcasecmp(optarg, "GS16") == 0) {
					fbink_cfg.wfm_mode = WFM_GS16;
				} else if (strcasecmp(optarg, "GU16") == 0) {
					fbink_cfg.wfm_mode = WFM_GU16;
				} else if (strcasecmp(optarg, "GLK16") == 0) {
					fbink_cfg.wfm_mode = WFM_GLK16;
					// NOTE: These two currently violently murder the EPDC...
					/*
				} else if (strcasecmp(optarg, "CLEAR") == 0) {
					fbink_cfg.wfm_mode = WFM_CLEAR;
				} else if (strcasecmp(optarg, "GC4L") == 0) {
					fbink_cfg.wfm_mode = WFM_GC4L;
				*/
				} else if (strcasecmp(optarg, "GCC16") == 0) {
					fbink_cfg.wfm_mode = WFM_GCC16;
				} else if (strcasecmp(optarg, "DUNM") == 0) {
					fbink_cfg.wfm_mode = WFM_DUNM;
				} else {
					ELOG("Unknown waveform update mode '%s'.", optarg);
					errfnd = true;
				}
				// Remember non-default values in a human-readable format...
				if (fbink_cfg.wfm_mode != WFM_AUTO) {
					wfm_name = optarg;
				}
				break;
			case 'H':
				fbink_cfg.is_nightmode = true;
				break;
			case 'Z':
				is_mimic = true;
				break;
			case 'z':
				is_koreader = true;
				break;
			case 'k': {
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
				} else {
					opt_longname = opts[opt_index].name;
				}

				// NOTE: Nasty bit of trickery to make getopt's optional_argument actually useful...
				//       Hat trick (& explanation) courtesy of https://stackoverflow.com/a/32575314
				//       If `optarg` isn't set and argv[optind] doesn't look like another option,
				//       then assume it's our parameter and overtly modify optind to compensate.
				if (!optarg && argv[optind] != NULL && argv[optind][0] != '-') {
					subopts = argv[optind++];
				} else {
					subopts = optarg;
				}

				// NOTE: We'll need to remember the original, full suboption string for diagnostic messages,
				//       because getsubopt will rewrite it during processing...
				if (subopts && *subopts != '\0') {
					// Only remember the first offending suboption list...
					if (!errfnd) {
						full_subopts = strdupa(subopts);    // lgtm [cpp/alloca-in-loop]
					}
				}

				while (subopts && *subopts != '\0' && !errfnd) {
					switch (getsubopt(&subopts, cls_token, &value)) {
						case TOP_OPT:
							if (value == NULL) {
								ELOG("Missing value for suboption '%s' of -%c, --%s",
								     cls_token[TOP_OPT],
								     opt,
								     opt_longname);
								errfnd = true;
								break;
							}
							if (strtoul_u(opt, cls_token[TOP_OPT], value, &region_top) < 0) {
								errfnd = true;
							}
							break;
						case LEFT_OPT:
							if (value == NULL) {
								ELOG("Missing value for suboption '%s' of -%c, --%s",
								     cls_token[LEFT_OPT],
								     opt,
								     opt_longname);
								errfnd = true;
								break;
							}
							if (strtoul_u(opt, cls_token[LEFT_OPT], value, &region_left) <
							    0) {
								errfnd = true;
							}
							break;
						case WIDTH_OPT:
							if (value == NULL) {
								ELOG("Missing value for suboption '%s' of -%c, --%s",
								     cls_token[WIDTH_OPT],
								     opt,
								     opt_longname);
								errfnd = true;
								break;
							}
							if (strtoul_u(opt, cls_token[WIDTH_OPT], value, &region_width) <
							    0) {
								errfnd = true;
							}
							break;
						case HEIGHT_OPT:
							if (value == NULL) {
								ELOG("Missing value for suboption '%s' of -%c, --%s",
								     cls_token[HEIGHT_OPT],
								     opt,
								     opt_longname);
								errfnd = true;
								break;
							}
							if (strtoul_u(opt, cls_token[HEIGHT_OPT], value, &region_height) <
							    0) {
								errfnd = true;
							}
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
				// Minor sanity check of the rectangle now,
				// allowing us later to simply unconditionally build an FBInkRect out of it.
				if ((region_height == 0 || region_width == 0) &&
				    !(region_top == 0 && region_left == 0 && region_height == 0 && region_width == 0)) {
					ELOG(
					    "Non-zero values must be specified for suboptions '%s' and '%s' of -%c, --%s",
					    cls_token[HEIGHT_OPT],
					    cls_token[WIDTH_OPT],
					    opt,
					    opt_longname);
					errfnd = true;
				} else {
					is_cls = true;
				}
				// Only remember this if there was a parsing error.
				if (!errfnd) {
					full_subopts = NULL;
				}
				break;
			}
			case 'K': {
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
				} else {
					opt_longname = opts[opt_index].name;
				}

				subopts = optarg;
				// NOTE: We'll need to remember the original, full suboption string for diagnostic messages,
				//       because getsubopt will rewrite it during processing...
				if (subopts && *subopts != '\0') {
					// Only remember the first offending suboption list...
					if (!errfnd) {
						full_subopts = strdupa(subopts);    // lgtm [cpp/alloca-in-loop]
					}
				}

				while (subopts && *subopts != '\0' && !errfnd) {
					switch (getsubopt(&subopts, anim_token, &value)) {
						case DIRECTION_OPT:
							if (value == NULL) {
								ELOG("Missing value for suboption '%s' of -%c, --%s",
								     anim_token[DIRECTION_OPT],
								     opt,
								     opt_longname);
								errfnd = true;
								break;
							}
							if (strcasecmp(value, "DOWN") == 0) {
								direction = MTK_SWIPE_DIR_DOWN;
							} else if (strcasecmp(value, "UP") == 0) {
								direction = MTK_SWIPE_DIR_UP;
							} else if (strcasecmp(value, "LEFT") == 0) {
								direction = MTK_SWIPE_DIR_LEFT;
							} else if (strcasecmp(value, "RIGHT") == 0) {
								direction = MTK_SWIPE_DIR_RIGHT;
							} else {
								ELOG(
								    "Unknown direction '%s' for suboption '%s' of -%c, --%s",
								    value,
								    anim_token[STEPS_OPT],
								    opt,
								    opt_longname);
								errfnd = true;
							}
							break;
						case STEPS_OPT:
							if (value == NULL) {
								ELOG("Missing value for suboption '%s' of -%c, --%s",
								     anim_token[STEPS_OPT],
								     opt,
								     opt_longname);
								errfnd = true;
								break;
							}
							if (strtoul_hhu(opt, anim_token[STEPS_OPT], value, &steps) < 0) {
								errfnd = true;
							}
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

					// We've got everything we need, do the thing!
					fbink_cfg.is_animated = true;
				}
				break;
			}
			case 'w':
				wait_for           = true;
				// Also disable merging on sunxi
				fbink_cfg.no_merge = true;
				break;
			case 'd':
				if (strtoul_hhu(opt, NULL, optarg, &daemon_lines) < 0) {
					errfnd = true;
				}
				is_daemon = true;
				break;
			case 'G':
				fbink_cfg.to_syslog = !fbink_cfg.to_syslog;
				toSysLog            = fbink_cfg.to_syslog;
				break;
			case 'Q':
				is_help = true;
				break;
			default:
				ELOG("?? Unknown option code 0%o ??", (unsigned int) opt);
				errfnd = true;
				break;
		}
		// Reset opt_index to our sentinel value, so we can lookup the longform's name when the shortform is passed...
		opt_index = -1;
	}

	// Assume success, until shit happens ;)
	int rv     = EXIT_SUCCESS;
	// Declare it a tiny bit early to make cleanup handling safe
	// (fbink_close is safe to call with fbfd set to -1 and/or the mmap not actually done).
	int fbfd   = -1;
	// Same idea for the pipe fd in daemon mode
	int pipefd = -1;

	// Show the "help" message
	if (is_help) {
		show_helpmsg();
		rv = EXIT_SUCCESS;
		goto cleanup;
	}

	// Enforce logging to syslog for is_daemon
	if (is_daemon) {
		fbink_cfg.to_syslog = true;
		toSysLog            = true;
	}

	// If we're logging to syslog, do an explicit openlog with our ID as soon as possible...
	if (toSysLog) {
		openlog("fbink", LOG_CONS | LOG_PID | LOG_NDELAY, LOG_DAEMON);
	}

	// Now we can make sure we passed an image file to print, one way or another
	if (is_image && image_file == NULL) {
		WARN(
		    "A path to an image file *must* be specified, either via the '%s' suboption of the -g, --image flag; or via the -i, --img flag",
		    image_token[FILE_OPT]);
		errfnd = true;
	}

	// We can't have two different types of consumable metadata being sent to stdout.
	// Use the API if you need more flexibility.
	if (want_linecount && want_lastrect) {
		WARN("Incompatible options: -E, --coordinates cannot be used in conjunction with -l, --linecount");
		errfnd = true;
	}

	// Error out if daemon mode is enabled with incompatible options
	// (basically anything that isn't is_truetype, is_*bar or nothing).
	if (is_daemon &&
	    (is_image || want_linecode || want_linecount || want_lastrect || is_eval || is_interactive || is_cls)) {
		WARN("Incompatible options: -d, --daemon can only be used for simple text or bar only workflows");
		errfnd = true;
	}

	// Enforce quiet output when asking for is_daemon, is_mimic, is_eval, want_linecount or want_lastrect,
	// to avoid polluting the output...
	if (is_daemon | is_mimic || is_eval || want_linecount || want_lastrect) {
		fbink_cfg.is_quiet   = true;
		fbink_cfg.is_verbose = false;
	}

	// Don't abort if we piped something without passing any arguments!
	if (errfnd || (argc == 1 && isatty(fileno(stdin)))) {
		if (errfnd) {
			if (!toSysLog) {
				fprintf(stderr, "\n****\t****\t****\t****\n");
			}
		}
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
		rv = ERRCODE(EXIT_FAILURE);
		goto cleanup;
	}

	// Open framebuffer and keep it around, then setup globals.
	if ((fbfd = fbink_open()) == ERRCODE(EXIT_FAILURE)) {
		ELOG("Failed to open the framebuffer, aborting . . .");
		rv = ERRCODE(EXIT_FAILURE);
		goto cleanup;
	}
	if (fbink_init(fbfd, &fbink_cfg) != EXIT_SUCCESS) {
		ELOG("Failed to initialize FBInk, aborting . . .");
		rv = ERRCODE(EXIT_FAILURE);
		goto cleanup;
	}

	// Did we request an animated refresh?
	if (fbink_cfg.is_animated) {
		if (fbink_mtk_set_swipe_data(direction, steps) != EXIT_SUCCESS) {
			fbink_cfg.is_animated = false;
		}
	}

	// If we're asking for a simple clear screen *only*, do it now, and then abort early.
	if (is_cls) {
		FBInkRect cls_rect = { 0 };
		cls_rect.left      = (unsigned short int) region_left;
		cls_rect.top       = (unsigned short int) region_top;
		cls_rect.width     = (unsigned short int) region_width;
		cls_rect.height    = (unsigned short int) region_height;
		rv                 = fbink_cls(fbfd, &fbink_cfg, &cls_rect, false);

		if (wait_for) {
#ifdef FBINK_FOR_KINDLE
			rv = fbink_wait_for_submission(fbfd, LAST_MARKER);
#endif
			rv = fbink_wait_for_complete(fbfd, LAST_MARKER);
		}

		if (want_lastrect) {
			print_lastrect();
		}
		goto cleanup;
	}

	// If we're asking to mimic on-animator, set the relevant options...
	if (is_mimic) {
		// We'll need to know a few things about the current device...
		FBInkState fbink_state = { 0 };
		fbink_get_state(&fbink_cfg, &fbink_state);

		// We'll want the manual grid clear to be centered, like the bar itself
		fbink_cfg.is_centered = true;
		// Double the usual size
		fbink_cfg.fontmult    = (uint8_t) (fbink_state.fontsize_mult << 1U);
		// Don't forget that fontmult & is_centered require a reinit...
		fbink_init(fbfd, &fbink_cfg);
		// Refresh the updated state
		fbink_get_state(&fbink_cfg, &fbink_state);

		// In the middle of the screen
		fbink_cfg.is_halfway = true;

		// Start by clearing the bar's region to white, to make sure A2 will behave...
		// NOTE: Commented out for now, as the chance of hitting the flags bug for this on Mk. 7 is low enough,
		//       and the quick succession of GC16 + A2 with no fencing causes a tearing artifact on the first refresh.
		/*
		fbink_cfg.wfm_mode = WFM_GC16;
		// We could use a fbink_grid_clear, but instead duplicate a bit of positioning logic instead for exact results...
		//fbink_grid_clear(fbfd, (unsigned short int) (fbink_state.max_cols - 1U), 1U, &fbink_cfg);
		short int                row = (short int) (fbink_state.max_rows / 2U);
		const unsigned short int top_pos =
		    (unsigned short int) MAX(0 + (fbink_state.view_vert_origin - fbink_state.view_vert_offset),
					     ((row * fbink_state.font_h) + fbink_state.view_vert_origin));
		const unsigned short int left_pos = fbink_state.view_hori_origin;
		const unsigned short int bar_width =
		    (unsigned short int) ((0.90f * (float) fbink_state.view_width) + 0.5f);
		const unsigned short int bar_left =
		    (unsigned short int) (left_pos + (0.05f * (float) fbink_state.view_width) + 0.5f);
		const FBInkRect region = {
			.left   = bar_left,
			.top    = top_pos,
			.width  = bar_width,
			.height = fbink_state.font_h,
		};
		fbink_cls(fbfd, &fbink_cfg, &region, false);
		*/

		// Make it backgroundless, to play nicer with whatever might already be on the edge of the screen
		fbink_cfg.is_bgless = true;
		// Fast
		fbink_cfg.wfm_mode  = WFM_A2;

		// Infinite activity bar
		is_activitybar = true;
		is_infinite    = true;
	}

	// If we're asking to run in daemon mode, that takes precedence over nearly everything.
	if (is_daemon) {
		// Fly, little daemon!
		if (daemonize() != 0) {
			WARN("Failed to daemonize, aborting");
			rv = ERRCODE(EXIT_FAILURE);
			goto cleanup;
		}

		// Ensure we'll cleanup behind us...
		struct sigaction new_action = { 0 };
		new_action.sa_sigaction     = &cleanup_handler;
		sigemptyset(&new_action.sa_mask);
		new_action.sa_flags = SA_SIGINFO;
		if ((rv = sigaction(SIGTERM, &new_action, NULL)) != 0) {
			PFWARN("sigaction (TERM): %m");
			goto cleanup;
		}
		if ((rv = sigaction(SIGINT, &new_action, NULL)) != 0) {
			PFWARN("sigaction (INT): %m");
			goto cleanup;
		}
		if ((rv = sigaction(SIGQUIT, &new_action, NULL)) != 0) {
			PFWARN("sigaction (QUIT): %m");
			goto cleanup;
		}

		// If we want to use a custom pipe name, honor that...
		const char* custom_pipe = getenv("FBINK_NAMED_PIPE");
		if (custom_pipe) {
			pipe_path = custom_pipe;
		} else {
			pipe_path = FBINK_PIPE;
		}

		// Start by creating our named pipe.
		// NOTE: You cannot re-use an existing pipe!
		rv = mkfifo(pipe_path, 0666);
		if (rv != 0) {
			PFWARN("mkfifo(%s): %m", pipe_path);
			// Make sure we won't delete the pipe, in case it's not ours...
			pipe_path = NULL;
			goto cleanup;
		}

		// Then poll it to react when someone writes to it...
		// NOTE: Since the write end will only be open for very short amount of times, we prefer polling,
		//       otherwise, read would spend most of its time busy-looping on EOF...
		// NOTE: See the POLLHUP note below for the reasoning behing opening it RW and not RO...
		// NOTE: See also this terrific recap: https://stackoverflow.com/a/17384067
		pipefd = open(pipe_path, O_RDWR | O_NONBLOCK | O_CLOEXEC);
		if (pipefd == -1) {
			PFWARN("open(%s): %m", pipe_path);
			// Same here, don't delete the pipe in case it's not ours...
			pipe_path = NULL;
			goto cleanup;
		}

		// Make sure we only load fonts once...
		if (is_truetype) {
			load_ot_fonts(reg_ot_file, bd_ot_file, it_ot_file, bdit_ot_file, &fbink_cfg, &ot_config);
		}

		// We'll need to keep track of the amount of printed lines to honor daemon_lines...
		int                linecount   = -1;
		unsigned short int total_lines = 0U;
		short int          initial_row = fbink_cfg.row;
		short int          initial_top = ot_config.margins.top;

		struct pollfd pfd = { 0 };
		pfd.fd            = pipefd;
		pfd.events        = POLLIN;
		// Forevah'!
		while (1) {
			// If we caught one of the signals we setup earlier, it's time to die ;).
			if (g_timeToDie != 0) {
				ELOG("Caught a cleanup signal (%s by UID: %ld, PID: %ld), winding down . . .",
				     strsignal(g_sigCaught.signo),
				     (long int) g_sigCaught.uid,
				     (long int) g_sigCaught.pid);
				goto cleanup;
			}

			int pn = poll(&pfd, 1, -1);
			if (pn == -1) {
				if (errno == EINTR) {
					continue;
				}
				PFWARN("poll: %m");
				rv = pn;
				goto cleanup;
			}

			if (pn > 0) {
				if (pfd.revents & POLLIN) {
					// We've got data to read, do it!
					char    buf[PIPE_BUF] = { 0 };
					ssize_t bytes_read    = 0;
					// NOTE: Retry on EINTR (should never happen on Linux,
					//       c.f., https://stackoverflow.com/a/59795677)
					do {
						// Flawfinder: ignore
						bytes_read = read(pfd.fd, buf, sizeof(buf));
					} while (bytes_read == -1 && errno == EINTR);
					if (bytes_read == -1) {
						if (errno == EAGAIN) {
							// Back to poll()!
							continue;
						}
						PFWARN("read: %m");
						rv = ERRCODE(EXIT_FAILURE);
						goto cleanup;
					}

					// This should never actually happen.
					if (bytes_read <= 0) {
						continue;
					}

					// First things first, do an explicit reinit, as we might have been running for a while.
					if (unlikely(fbink_reinit(fbfd, &fbink_cfg) < 0)) {
						// We don't track state, so we only need to handle plain failures.
						PFWARN("fbink_reinit");
						rv = ERRCODE(EXIT_FAILURE);
						goto cleanup;
					}

					// NOTE: In every case, we *ignore* errors in order not to silently die on bogus input...
					// If we're drawing a bar, make sure we were fed vaguely valid input...
					if (is_progressbar || is_activitybar) {
						uint8_t bar_val = 0U;
						if (strtoul_hhu('d', NULL, buf, &bar_val) == 0) {
							// It's a number, let the API deal with OOB values.
							if (is_progressbar) {
								fbink_print_progress_bar(fbfd, bar_val, &fbink_cfg);
							} else {
								fbink_print_activity_bar(fbfd, bar_val, &fbink_cfg);
							}
						}
					} else if (is_truetype) {
						linecount = fbink_print_ot(fbfd, buf, &ot_config, &fbink_cfg, &ot_fit);

						// Move to the next line, unless it'd make us blow past daemon_lines...
						total_lines = (unsigned short int) (total_lines + ot_fit.rendered_lines);
						// NOTE: A return value of 0 means not enough space for a new line...
						if ((daemon_lines == 0U || total_lines < daemon_lines) && linecount > 0) {
							ot_config.margins.top = (short int) linecount;
						} else {
							// Reset to original settings...
							total_lines           = 0U;
							ot_config.margins.top = (short int) initial_top;
						}
					} else {
						linecount = fbink_print(fbfd, buf, &fbink_cfg);

						// Move to the next line, unless it'd make us blow past daemon_lines...
						if (linecount > 0) {
							total_lines = (unsigned short int) (total_lines + linecount);
						}
						if ((daemon_lines == 0U || total_lines < daemon_lines) &&
						    linecount >= 0) {
							fbink_cfg.row = (short int) (fbink_cfg.row + linecount);
						} else {
							// Reset to original settings...
							total_lines   = 0U;
							fbink_cfg.row = initial_row;
						}
					}
				}
				// NOTE: The first writer will invariably end up closing its end of the pipe.
				//       This means POLLHUP will be set from that point on,
				//       and the only way to clear it would be to close our reader...
				//       We don't want to do that, so, just ignore POLLHUP,
				//       and prevent it from ever happening in the first place,
				//       by ensuring we ourselves are considered as a writer (by opening the pipe RDWR) ;).
			}
		}

		// Unreachable ;).
		goto cleanup;
	}

	if (optind < argc) {
		// We'll need that in the cell rendering codepath
		unsigned short int total_lines = 0U;

		// And for the OpenType codepath, we'll want to load the fonts only once ;)
		if (is_truetype) {
			load_ot_fonts(reg_ot_file, bd_ot_file, it_ot_file, bdit_ot_file, &fbink_cfg, &ot_config);
		}

		// Now that this is out of the way, loop over the leftover arguments, i.e.: the strings ;)
		while (optind < argc) {
			int   linecount = -1;
			char* string    = argv[optind++];
			// NOTE: This is probably the point where we'd be validating/converting string to UTF-8,
			//       if we had an easy way to... (c.f., my rant about Kobo's broken libc in fbink_internal.h)

			// Did we want to use the OpenType codepath?
			if (is_truetype) {
				if (!fbink_cfg.is_quiet) {
					LOG("Printing string '%s' @ %.1fpt (or %hupx), honoring the following margins { Top: %hdpx, Bottom: %hdpx, Left: %hdpx, Right: %hdpx } (default style: %d, formatted: %s, compute only: %s, no truncation: %s, overlay: %s, no BG: %s, no FG: %s, inverted: %s, flashing: %s, centered: %s, H align: %hhu, halfway: %s, V align: %hhu, clear screen: %s, waveform: %s, dithering: %s, nightmode: %s, skip refresh: %s)",
					    string,
					    ot_config.size_pt,
					    ot_config.size_px,
					    ot_config.margins.top,
					    ot_config.margins.bottom,
					    ot_config.margins.left,
					    ot_config.margins.right,
					    ot_config.style,
					    ot_config.is_formatted ? "Y" : "N",
					    ot_config.compute_only ? "Y" : "N",
					    ot_config.no_truncation ? "Y" : "N",
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
					    wfm_name ? wfm_name : "AUTO",
					    hwd_name ? hwd_name : "PASSTHROUGH",
					    fbink_cfg.is_nightmode ? "Y" : "N",
					    fbink_cfg.no_refresh ? "Y" : "N");
				}

				if ((linecount = fbink_print_ot(fbfd, string, &ot_config, &fbink_cfg, &ot_fit)) < 0) {
					WARN("Failed to print that string");
					// In case we asked to flag truncation as a failure, figure out what happened...
					if (linecount == ERRCODE(ENOSPC)) {
						if (ot_fit.rendered_lines > 0) {
							ELOG("Because it was truncated at rendering time.");
						} else {
							ELOG("Because it would have been rendered truncated.");
						}
					}
					rv = ERRCODE(EXIT_FAILURE);
					goto cleanup;
				}

				// NOTE: Don't clobber previous entries if multiple strings were passed...
				//       We make sure to trust print's return value,
				//       because it knows how much space it already took up ;).
				ot_config.margins.top = (short int) linecount;
				if (!fbink_cfg.is_quiet) {
					LOG("Next line should start @ top=%hd", ot_config.margins.top);
					LOG("Rendered %hu lines out of the %hu expected%s",
					    ot_fit.rendered_lines,
					    ot_fit.computed_lines,
					    ot_fit.truncated ? ", string was truncated." : ".");
				}
				// NOTE: By design, if you ask for a clear screen, only the final print will stay on screen ;).

				// If we were asked to return the amount of printed lines, honor that...
				// NOTE: We don't support linecode, because POSIX exit codes cap at 255, which is much too low.
				if (want_linecount) {
					// NOTE: fbink_print_ot returns a new top margin value directly,
					//       no need for any extra computations!
					total_lines = (unsigned short int) (linecount);
				}
			} else {
				if (!fbink_cfg.is_quiet) {
					LOG("Printing string '%s' @ column %hd + %hdpx, row %hd + %hdpx (overlay: %s, no BG: %s, no FG: %s, inverted: %s, flashing: %s, centered: %s, halfway: %s, left padded: %s, right padded: %s, clear screen: %s, waveform: %s, dithering: %s, nightmode: %s, skip refresh: %s, font: %hhu, font scaling: x%hhu)",
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
					    wfm_name ? wfm_name : "AUTO",
					    hwd_name ? hwd_name : "PASSTHROUGH",
					    fbink_cfg.is_nightmode ? "Y" : "N",
					    fbink_cfg.no_refresh ? "Y" : "N",
					    fbink_cfg.fontname,
					    fbink_cfg.fontmult);
				}

				if ((linecount = fbink_print(fbfd, string, &fbink_cfg)) < 0) {
					WARN("Failed to print that string");
					rv = ERRCODE(EXIT_FAILURE);
					goto cleanup;
				}

				// NOTE: Don't clobber previous entries if multiple strings were passed...
				//       We make sure to trust print's return value,
				//       because it knows how much space it already took up ;).
				fbink_cfg.row = (short int) (fbink_cfg.row + linecount);
				if (!fbink_cfg.is_quiet) {
					LOG("Next line should start @ row %hd", fbink_cfg.row);
				}
				// NOTE: By design, if you ask for a clear screen, only the final print will stay on screen ;).

				// If we were asked to return the amount of printed lines, honor that...
				if (want_linecode) {
					rv += linecount;
				}
				if (want_linecount) {
					total_lines = (unsigned short int) (total_lines + linecount);
				}
			}
			// This is slightly tricky for multi-args prints...
			// We'll compute a super rectangle that includes every line's rectangle.
			if (want_lastrect) {
				compute_lastrect();
			}
		}
		// NOTE: For multi-args prints, we'll only wait for the *last* print
		if (wait_for) {
#ifdef FBINK_FOR_KINDLE
			fbink_wait_for_submission(fbfd, LAST_MARKER);
#endif
			fbink_wait_for_complete(fbfd, LAST_MARKER);
		}
		// Print the coordinates & dimensions of *everything* we wrote, if requested
		// NOTE: For multi-args prints, that means a synthetic rect that encompasses the full thing.
		if (want_lastrect) {
			recap_lastrect();
		}
		// And print the total amount of lines we printed, if requested...
		if (want_linecount) {
			if (is_truetype) {
				// OT has a more detailed feedback, with the line-breaking computation results,
				// so it's in an eval friendly format instead...
				printf(
				    "next_top=%hu;computed_lines=%hu;rendered_lines=%hu;bbox_width=%hu;bbox_height=%hu;truncated=%d;",
				    total_lines,
				    ot_fit.computed_lines,
				    ot_fit.rendered_lines,
				    ot_fit.bbox.width,
				    ot_fit.bbox.height,
				    ot_fit.truncated);
			} else {
				printf("%hu", total_lines);
			}
		}
	} else {
		if (is_refresh) {
			if (!fbink_cfg.is_quiet) {
				LOG("Refreshing the screen from top=%u, left=%u for width=%u, height=%u with %swaveform mode %s and dithering mode %s (nightmode: %s)",
				    region_top,
				    region_left,
				    region_width,
				    region_height,
				    fbink_cfg.is_flashing ? "a flashing " : "",
				    wfm_name ? wfm_name : "AUTO",
				    hwd_name ? hwd_name : "PASSTHROUGH",
				    fbink_cfg.is_nightmode ? "Y" : "N");
			}
			if (fbink_refresh(fbfd, region_top, region_left, region_width, region_height, &fbink_cfg) !=
			    EXIT_SUCCESS) {
				WARN("Failed to refresh the screen as per your specification");
				rv = ERRCODE(EXIT_FAILURE);
				goto cleanup;
			} else {
				if (wait_for) {
#ifdef FBINK_FOR_KINDLE
					fbink_wait_for_submission(fbfd, LAST_MARKER);
#endif
					fbink_wait_for_complete(fbfd, LAST_MARKER);
				}
			}
		} else if (is_image) {
			if (!fbink_cfg.is_quiet) {
				LOG("Displaying image '%s' @ column %hd + %hdpx, row %hd + %dpx (scaling: %hdx%hd, H align: %hhu, V align: %hhu, inverted: %s, flattened: %s, waveform: %s, HW dithering: %s, SW dithered: %s, nightmode: %s, skip refresh: %s)",
				    image_file,
				    fbink_cfg.col,
				    image_x_offset,
				    fbink_cfg.row,
				    image_y_offset,
				    fbink_cfg.scaled_width,
				    fbink_cfg.scaled_height,
				    fbink_cfg.halign,
				    fbink_cfg.valign,
				    fbink_cfg.is_inverted ? "Y" : "N",
				    fbink_cfg.ignore_alpha ? "Y" : "N",
				    wfm_name ? wfm_name : "AUTO",
				    hwd_name ? hwd_name : "PASSTHROUGH",
				    fbink_cfg.sw_dithering ? "Y" : "N",
				    fbink_cfg.is_nightmode ? "Y" : "N",
				    fbink_cfg.no_refresh ? "Y" : "N");
			}
			if (fbink_print_image(fbfd, image_file, image_x_offset, image_y_offset, &fbink_cfg) !=
			    EXIT_SUCCESS) {
				WARN("Failed to display that image");
				rv = ERRCODE(EXIT_FAILURE);
				goto cleanup;
			} else {
				if (wait_for) {
#ifdef FBINK_FOR_KINDLE
					fbink_wait_for_submission(fbfd, LAST_MARKER);
#endif
					fbink_wait_for_complete(fbfd, LAST_MARKER);
				}
				// Print the coordinates & dimensions of what we've drawn, if requested
				if (want_lastrect) {
					print_lastrect();
				}
			}
		} else if (is_progressbar) {
			if (!fbink_cfg.is_quiet) {
				LOG("Displaying a %hhu%% full progress bar @ row %hd + %hdpx (no FG: %s, no BG: %s, inverted: %s, flashing: %s, halfway: %s, clear screen: %s, waveform: %s, dithering: %s, nightmode: %s, skip refresh: %s, font: %hhu, font scaling: x%hhu)",
				    progress,
				    fbink_cfg.row,
				    fbink_cfg.voffset,
				    fbink_cfg.is_fgless ? "Y" : "N",
				    fbink_cfg.is_bgless ? "Y" : "N",
				    fbink_cfg.is_inverted ? "Y" : "N",
				    fbink_cfg.is_flashing ? "Y" : "N",
				    fbink_cfg.is_halfway ? "Y" : "N",
				    fbink_cfg.is_cleared ? "Y" : "N",
				    wfm_name ? wfm_name : "AUTO",
				    hwd_name ? hwd_name : "PASSTHROUGH",
				    fbink_cfg.is_nightmode ? "Y" : "N",
				    fbink_cfg.no_refresh ? "Y" : "N",
				    fbink_cfg.fontname,
				    fbink_cfg.fontmult);
			}
			if (fbink_print_progress_bar(fbfd, progress, &fbink_cfg) != EXIT_SUCCESS) {
				WARN("Failed to display a progressbar");
				rv = ERRCODE(EXIT_FAILURE);
				goto cleanup;
			} else {
				if (wait_for) {
#ifdef FBINK_FOR_KINDLE
					fbink_wait_for_submission(fbfd, LAST_MARKER);
#endif
					fbink_wait_for_complete(fbfd, LAST_MARKER);
				}
				// Print the coordinates & dimensions of what we've drawn, if requested
				if (want_lastrect) {
					print_lastrect();
				}
			}
		} else if (is_activitybar) {
			// Were we asked to loop forever?
			if (is_infinite) {
				if (!fbink_cfg.is_quiet) {
					LOG("Displaying an activity bar cycling forever @ row %hd + %hdpx (no BG: %s, inverted: %s, flashing: %s, halfway: %s, clear screen: %s, waveform: %s, dithering: %s, nightmode: %s, skip refresh: %s)",
					    fbink_cfg.row,
					    fbink_cfg.voffset,
					    fbink_cfg.is_bgless ? "Y" : "N",
					    fbink_cfg.is_inverted ? "Y" : "N",
					    fbink_cfg.is_flashing ? "Y" : "N",
					    fbink_cfg.is_halfway ? "Y" : "N",
					    fbink_cfg.is_cleared ? "Y" : "N",
					    wfm_name ? wfm_name : "AUTO",
					    hwd_name ? hwd_name : "PASSTHROUGH",
					    fbink_cfg.is_nightmode ? "Y" : "N",
					    fbink_cfg.no_refresh ? "Y" : "N");
				}
				// NOTE: In a dedicated function,
				//       because keeping it inline massively tanks performance in the image codepath,
				//       for an amazingly weird LTO-related reason :?
				if (do_infinite_progress_bar(fbfd, &fbink_cfg) != EXIT_SUCCESS) {
					WARN("Failed to display a progressbar");
					rv = ERRCODE(EXIT_FAILURE);
					goto cleanup;
				}
			} else {
				if (!fbink_cfg.is_quiet) {
					LOG("Displaying an activity bar on step %hhu @ row %hd + %hdpx (no BG: %s, inverted: %s, flashing: %s, halfway: %s, clear screen: %s, waveform: %s, dithering: %s, nightmode: %s, skip refresh: %s)",
					    progress,
					    fbink_cfg.row,
					    fbink_cfg.voffset,
					    fbink_cfg.is_bgless ? "Y" : "N",
					    fbink_cfg.is_inverted ? "Y" : "N",
					    fbink_cfg.is_flashing ? "Y" : "N",
					    fbink_cfg.is_halfway ? "Y" : "N",
					    fbink_cfg.is_cleared ? "Y" : "N",
					    wfm_name ? wfm_name : "AUTO",
					    hwd_name ? hwd_name : "PASSTHROUGH",
					    fbink_cfg.is_nightmode ? "Y" : "N",
					    fbink_cfg.no_refresh ? "Y" : "N");
				}
				if (fbink_print_activity_bar(fbfd, progress, &fbink_cfg) != EXIT_SUCCESS) {
					WARN("Failed to display an activitybar");
					rv = ERRCODE(EXIT_FAILURE);
					goto cleanup;
				} else {
					if (wait_for) {
#ifdef FBINK_FOR_KINDLE
						fbink_wait_for_submission(fbfd, LAST_MARKER);
#endif
						fbink_wait_for_complete(fbfd, LAST_MARKER);
					}
					// Print the coordinates & dimensions of what we've drawn, if requested
					if (want_lastrect) {
						print_lastrect();
					}
				}
			}
		} else if (is_eval) {
			// Allow doing a clear at the same time
			if (fbink_cfg.is_cleared) {
				rv = fbink_cls(fbfd, &fbink_cfg, NULL, false);
			}
			// NOTE: Do not honor wait_for here, as it'd probably be unwanted...
			fbink_state_dump(&fbink_cfg);
		} else if (is_interactive && isatty(fileno(stdin))) {
			// We asked for interactive mode, and we're really running from a terminal, so, go ahead.
			char*   line = NULL;
			size_t  len  = 0;
			ssize_t nread;
			// Draw a poor man's prompt, which works fairly okay, until we enable verbose mode :D.
			// NOTE: We limit this to the fixed-cell renderer, as it has a few tweaks to handle this better.
			printf(">>> ");
			while ((nread = getline(&line, &len, stdin)) != -1) {
				printf(">>> ");
				int linecnt;
				if ((linecnt = fbink_print(fbfd, line, &fbink_cfg)) < 0) {
					WARN("Failed to print that string");
					rv = ERRCODE(EXIT_FAILURE);
				}
				fbink_cfg.row = (short int) (fbink_cfg.row + linecnt);
			}
			free(line);
		} else {
			// If all else failed, try reading from stdin, provided we're not running from a terminal ;).
			if (!isatty(fileno(stdin))) {
				char*          line = NULL;
				size_t         len  = 0;
				ssize_t        nread;
				int            linecnt    = -1;
				unsigned short totallines = 0U;

				// If we're being run from a non-interactive SSH session, abort early if stdin is empty,
				// as we do *NOT* want an interactive behavior here (c.f., #32).
				// We have to jump through a few extra hoops,
				// because we *do* want to allow blocking behavior in general, as it can be useful,
				// for instance when piping the output of a tail -f like tool.
				// NOTE: The obvious downside is that, over SSH,
				//       this specific "follow" use-case will only reliably work from an *interactive* SSH session!
				if (getenv("SSH_CLIENT") != NULL && getenv("SSH_TTY") == NULL) {
					// Okay, we're a non-interactive SSH session...
					int n;
					if (ioctl(fileno(stdin), FIONREAD, &n) == 0 && n == 0) {
						// Okay, stdin was empty *at call time*,
						// don't even try to wait for us to have someting to read from it...
						WARN(
						    "This is a non-interactive SSH session and stdin is *currently* empty, enforcing non-blocking behavior by aborting early");
						// We're not printing anything, so, do the usual clear or help dance.
						if (fbink_cfg.is_cleared) {
							rv = fbink_cls(fbfd, &fbink_cfg, NULL, false);
							// NOTE: Same thing here, we'll forgo wait_for in this edge case...
						} else {
							show_helpmsg();
						}
						// And abort early, if still possible ;).
						ioctl(fileno(stdin), FIONREAD, &n);
						if (n == 0) {
							// Okay, stdin is still empty, we're reasonably sure we can exit safely.
							goto cleanup;
						} else {
							// NOTE: If you're piping something that doesn't *immediately*
							//       provide us with something to read, this mostly ensures we'll try
							//       to honor said delayed input.
							//       This is obviously race-y, but may happen with tail -f like stuff.
							//       Using this kind of tool in a non-interactive SSH session
							//       is somewhat discouraged, as you'll have to take care of killing
							//       said tool yourself!
							//       (Killing the tool will allow FBInk to exit sanely,
							//       but FBInk exiting will *NOT* kill the tool itself!
							//       It'll most likely get re-parented to init instead!).
							WARN(
							    "stdin is no longer empty, re-engaging classic blocking behavior");
						}
					}
				}

				// Did we ask for OT rendering?
				if (is_truetype) {
					load_ot_fonts(
					    reg_ot_file, bd_ot_file, it_ot_file, bdit_ot_file, &fbink_cfg, &ot_config);
					while ((nread = getline(&line, &len, stdin)) != -1) {
						if ((linecnt = fbink_print_ot(
							 fbfd, line, &ot_config, &fbink_cfg, &ot_fit)) < 0) {
							WARN("Failed to print that string");
							// In case we asked to flag truncation as a failure, figure out what happened...
							if (linecnt == ERRCODE(ENOSPC)) {
								if (ot_fit.rendered_lines > 0) {
									ELOG(
									    "Because it was truncated at rendering time.");
								} else {
									ELOG(
									    "Because it would have been rendered truncated.");
								}
							}
							rv = ERRCODE(EXIT_FAILURE);
						}
						ot_config.margins.top = (short int) linecnt;
						if (!fbink_cfg.is_quiet) {
							LOG("Next line should start @ top=%hd", ot_config.margins.top);
							LOG("Rendered %hu lines out of the %hu expected%s",
							    ot_fit.rendered_lines,
							    ot_fit.computed_lines,
							    ot_fit.truncated ? ", string was truncated." : ".");
						}

						if (want_linecount) {
							totallines = (unsigned short int) (linecnt);
						}
						if (want_lastrect) {
							compute_lastrect();
						}
					}
				} else {
					// NOTE: In KOReader, we'll be piping zsync2's output, which includes a progressbar...
					//       That progressbar refreshes itself via a simple CR. This means there's no LF.
					//       But that's what getline looks for in order to return a line...
					//       Instead, we've mangled zsync2's logging to end on a NULL byte,
					//       which is what we'll use as a line separator instead...
					int line_delim;
					if (is_koreader) {
						line_delim = '\0';
					} else {
						line_delim = '\n';
					}
					bool cr_seen = false;

					while ((nread = getdelim(&line, &len, line_delim, stdin)) != -1) {
						// As long as we're not the first printed line, or the first CR in a while,
						// go back to the previous line if the first character of this "new" line is a CR.
						if (is_koreader) {
							if (cr_seen && *line == '\r') {
								fbink_cfg.row = (short int) (fbink_cfg.row - linecnt);
							}
							if (*line == '\r') {
								cr_seen = true;
							} else {
								cr_seen = false;
							}
						}
						if ((linecnt = fbink_print(fbfd, line + cr_seen, &fbink_cfg)) < 0) {
							WARN("Failed to print that string");
							rv = ERRCODE(EXIT_FAILURE);
						}
						fbink_cfg.row = (short int) (fbink_cfg.row + linecnt);
						if (!fbink_cfg.is_quiet) {
							LOG("Next line should start @ row %hd", fbink_cfg.row);
						}

						if (want_linecode) {
							rv += linecnt;
						}
						if (want_linecount) {
							totallines = (unsigned short int) (totallines + linecnt);
						}
						if (want_lastrect) {
							compute_lastrect();
						}
					}
				}
				free(line);

				// If nothing was read, show the help
				if (linecnt == -1) {
					// Except that if we asked for a clear, do it!
					if (fbink_cfg.is_cleared) {
						rv = fbink_cls(fbfd, &fbink_cfg, NULL, false);

						if (wait_for) {
#ifdef FBINK_FOR_KINDLE
							rv = fbink_wait_for_submission(fbfd, LAST_MARKER);
#endif
							rv = fbink_wait_for_complete(fbfd, LAST_MARKER);
						}
					} else {
						show_helpmsg();
					}
				} else {
					if (wait_for) {
#ifdef FBINK_FOR_KINDLE
						fbink_wait_for_submission(fbfd, LAST_MARKER);
#endif
						fbink_wait_for_complete(fbfd, LAST_MARKER);
					}
					if (want_lastrect) {
						recap_lastrect();
					}
					if (want_linecount) {
						if (is_truetype) {
							printf(
							    "next_top=%hu;computed_lines=%hu;rendered_lines=%hu;bbox_width=%hu;bbox_height=%hu;truncated=%d;",
							    totallines,
							    ot_fit.computed_lines,
							    ot_fit.rendered_lines,
							    ot_fit.bbox.width,
							    ot_fit.bbox.height,
							    ot_fit.truncated);
						} else {
							printf("%hu", totallines);
						}
					}
				}
			} else {
				// If we didn't pass *any* non-positional arguments, but we asked for a screen clear, do it!
				// This avoids the clunky need to pass an empty string to do that otherwise ;).
				// This is at the bottom rung of the if ladder so as not to mess with anything else,
				// in particular stdin handling...
				if (fbink_cfg.is_cleared) {
					rv = fbink_cls(fbfd, &fbink_cfg, NULL, false);

					if (wait_for) {
#ifdef FBINK_FOR_KINDLE
						rv = fbink_wait_for_submission(fbfd, LAST_MARKER);
#endif
						rv = fbink_wait_for_complete(fbfd, LAST_MARKER);
					}
				} else {
					show_helpmsg();
				}
			}
		}
	}

	// Cleanup
cleanup:
	if (is_truetype) {
		fbink_free_ot_fonts();
	}

	if (is_daemon) {
		if (pipefd != -1) {
			if (close(pipefd) != 0) {
				PFWARN("close: %m");
			}
		}
		if (pipe_path) {
			if (unlink(pipe_path) != 0) {
				PFWARN("unlink(%s): %m", pipe_path);
			}
		}
	}

	if (fbink_close(fbfd) == ERRCODE(EXIT_FAILURE)) {
		ELOG("Failed to close the framebuffer, aborting . . .");
		rv = ERRCODE(EXIT_FAILURE);
	}

	if (toSysLog) {
		closelog();
	}

	return rv;
}
