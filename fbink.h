/*
	FBInk: FrameBuffer eInker, a tool to print text & images on eInk devices (Kobo/Kindle)
	Copyright (C) 2018-2019 NiLuJe <ninuje@gmail.com>
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

#ifndef __FBINK_H
#define __FBINK_H

// Because we're pretty much Linux-bound ;).
#ifndef _GNU_SOURCE
#	define _GNU_SOURCE
#endif

#include <stdarg.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

// Symbol visibility shenanigans...
// c.f., https://gcc.gnu.org/wiki/Visibility
#if __GNUC__ >= 4
#	define DLL_PUBLIC __attribute__((visibility("default")))
#	define DLL_LOCAL  __attribute__((visibility("hidden")))
#else
#	define DLL_PUBLIC
#	define DLL_LOCAL
#endif

// Are we actually building the shared lib?
#ifdef FBINK_SHAREDLIB
#	define FBINK_API   DLL_PUBLIC
#	define FBINK_LOCAL DLL_LOCAL
#else
#	define FBINK_API
#	define FBINK_LOCAL
#endif

//
////
//
// Magic number for automatic fbfd handling
#define FBFD_AUTO -1
// As 0 is an invalid marker value, we can coopt it to try to retrieve our own last sent marker
#define LAST_MARKER 0U

//
// List of available fonts
typedef enum
{
	IBM = 0U,          // font8x8
	UNSCII,            // unscii-8
	UNSCII_ALT,        // unscii-8-alt
	UNSCII_THIN,       // unscii-8-thin
	UNSCII_FANTASY,    // unscii-8-fantasy
	UNSCII_MCR,        // unscii-8-mcr
	UNSCII_TALL,       // unscii-16
	BLOCK,             // block
	LEGGIE,            // leggie (regular)
	VEGGIE,            // leggie EGA/VGA/FB
	KATES,             // kates (nexus)
	FKP,               // fkp
	CTRLD,             // ctrld
	ORP,               // orp (regular)
	ORPB,              // orp (bold)
	ORPI,              // orp (italic)
	SCIENTIFICA,       // scientifica (regular)
	SCIENTIFICAB,      // scientifica (bold)
	SCIENTIFICAI,      // scientifica (italic)
	TERMINUS,          // terminus (regular)
	TERMINUSB,         // terminus (bold)
	FATTY,             // fatty
	SPLEEN,            // spleen
	TEWI,              // tewi (medium)
	TEWIB,             // tewi (bold)
	TOPAZ,             // Topaz+ A1200
	MICROKNIGHT,       // MicroKnight+
	VGA                // IBM VGA 8x16
} FONT_INDEX_T;

// List of supported font styles
typedef enum
{
	FNT_REGULAR = 0U,
	FNT_ITALIC,
	FNT_BOLD,
	FNT_BOLD_ITALIC
} FONT_STYLE_T;

// List of available halign/valign values
typedef enum
{
	NONE = 0U,    // i.e., LEFT for halign, TOP for valign
	CENTER,       //
	EDGE          // i.e., RIGHT for halign, BOTTOM for valign
} ALIGN_INDEX_T;

// List of available colors in the eInk color map
// NOTE: This is split in FG & BG to ensure that the default values lead to a sane result (i.e., black on white)
typedef enum
{
	FG_BLACK = 0U,
	FG_GRAY1,
	FG_GRAY2,
	FG_GRAY3,
	FG_GRAY4,
	FG_GRAY5,
	FG_GRAY6,
	FG_GRAY7,
	FG_GRAY8,
	FG_GRAY9,
	FG_GRAYA,
	FG_GRAYB,
	FG_GRAYC,
	FG_GRAYD,
	FG_GRAYE,
	FG_WHITE
} FG_COLOR_INDEX_T;

typedef enum
{
	BG_WHITE = 0U,
	BG_GRAYE,
	BG_GRAYD,
	BG_GRAYC,
	BG_GRAYB,
	BG_GRAYA,
	BG_GRAY9,
	BG_GRAY8,
	BG_GRAY7,
	BG_GRAY6,
	BG_GRAY5,
	BG_GRAY4,
	BG_GRAY3,
	BG_GRAY2,
	BG_GRAY1,
	BG_BLACK
} BG_COLOR_INDEX_T;

// List of *potentially* available waveform modes
typedef enum
{
	WFM_AUTO = 0U,
	WFM_DU,
	WFM_GC16,
	WFM_GC4,
	WFM_A2,
	WFM_GL16,
	WFM_REAGL,
	WFM_REAGLD,
	WFM_GC16_FAST,
	WFM_GL16_FAST,
	WFM_DU4,
	WFM_GL4,
	WFM_GL16_INV,
	WFM_GCK16,
	WFM_GLKW16,
	WFM_INIT,
} WFM_MODE_INDEX_T;

// List of *potentially* available HW dithering modes
typedef enum
{
	HWD_PASSTHROUGH = 0U,
	HWD_FLOYD_STEINBERG,
	HWD_ATKINSON,
	HWD_ORDERED,
	HWD_QUANT_ONLY
} HW_DITHER_INDEX_T;

// List of NTX rotation quirk types (c.f., mxc_epdc_fb_check_var @ drivers/video/fbdev/mxc/mxc_epdc_v2_fb.c)...
typedef enum
{
	NTX_ROTA_STRAIGHT = 0U,    // No shenanigans (at least as far as ioctls are concerned)
	NTX_ROTA_ALL_INVERTED,     // Every rotation is inverted by the kernel
	NTX_ROTA_ODD_INVERTED,     // Only Landscape (odd) rotations are inverted by the kernel
	NTX_ROTA_SANE              // NTX_ROTA_STRAIGHT + boot rota is UR + Panel is natively UR
} NTX_ROTA_INDEX_T;

//
// A struct to dump FBInk's internal state into, like fbink_state_dump() would, but in C ;)
typedef struct
{
	long int             user_hz;        // USER_HZ (should pretty much always be 100)
	const char* restrict font_name;      // fbink_cfg->fontname (c.f., fontname_to_string())
	uint32_t             view_width;     // viewWidth (MAY be different than screen_width on devices with a viewport)
	uint32_t             view_height;    // viewHeight (ditto)
	uint32_t screen_width;       // screenWidth (Effective width, c.f., is_ntx_quirky_landscape & initialize_fbink())
	uint32_t screen_height;      // screenHeight (ditto)
	uint32_t bpp;                // vInfo.bits_per_pixel
	char     device_name[16];    // deviceQuirks.deviceName (short common name, no brand)
	char     device_codename[16];       // deviceQuirks.deviceCodename
	char     device_platform[16];       // deviceQuirks.devicePlatform (often a codename, too)
	unsigned short int device_id;       // deviceQuirks.deviceId (decimal value, c.f., identify_device() on Kindle!)
	uint8_t            pen_fg_color;    // penFGColor (grayscale)
	uint8_t            pen_bg_color;    // penBGColor (ditto)
	unsigned short int screen_dpi;      // deviceQuirks.screenDPI
	unsigned short int font_w;          // FONTW (effective width of a glyph cell, i.e. scaled)
	unsigned short int font_h;          // FONTH (effective height of a glyph cell, i.e. scaled)
	unsigned short int max_cols;        // MAXCOLS (at current cell size)
	unsigned short int max_rows;        // MAXROWS (ditto)
	uint8_t view_hori_origin;           // viewHoriOrigin (would be non-zero on devices with a horizontal viewport)
	uint8_t view_vert_origin;           // viewVertOrigin (origin in px of row 0, includes viewport + viewVertOffset)
	uint8_t view_vert_offset;    // viewVertOffset (shift in px needed to vertically balance rows over viewHeight)
	uint8_t fontsize_mult;       // FONTSIZE_MULT (current cell scaling multiplier)
	uint8_t glyph_width;         // glyphWidth (native width of a glyph cell, i.e. unscaled)
	uint8_t glyph_height;        // glyphHeight (native height of a glyph cell, i.e. unscaled)
	bool    is_perfect_fit;      // deviceQuirks.isPerfectFit (horizontal column balance is perfect over viewWidth)
	bool    is_kobo_non_mt;      // deviceQuirks.isKoboNonMT (device is a Kobo with no MultiTouch input support)
	uint8_t ntx_boot_rota;       // deviceQuirks.ntxBootRota (Native rotation at boot)
	uint8_t ntx_rota_quirk;      // deviceQuirks.ntxRotaQuirk (c.f., NTX_ROTA_INDEX_T & utils/dump.c)
	bool    is_ntx_quirky_landscape;    // deviceQuirks.isNTX16bLandscape (rotation compensation is in effect)
	uint8_t current_rota;               // vInfo.rotate (current rotation, c.f., <linux/fb.h>)
	bool    can_rotate;                 // deviceQuirks.canRotate (device has a gyro)
} FBInkState;

// What a FBInk config should look like. Perfectly sane when fully zero-initialized.
typedef struct
{
	short int row;              // y axis (i.e., line), counts down from the bottom of the screen if negative
	short int col;              // x axis (i.e., column), counts down from the right edge of the screen if negative
	uint8_t   fontmult;         // Font scaling multiplier (i.e., 4 -> x4), 0 means automatic.
	uint8_t   fontname;         // Request a specific font (c.f., FONT_INDEX_T enum)
	bool      is_inverted;      // Invert colors.
				    // This is *NOT* mutually exclusive with is_nightmode, and is *always* supported.
	bool      is_flashing;      // Request a black flash on refresh
	bool      is_cleared;       // Clear the screen beforehand (honors is_inverted)
	bool      is_centered;      // Center the text (horizontally)
	short int hoffset;          // Horizontal offset (in pixels) for text position
	short int voffset;          // Vertical offset (in pixels) for text position
	bool      is_halfway;       // Vertically center the text, honoring row offsets
	bool      is_padded;        // Pad the text with blanks (on the left, or on both sides if is_centered)
	bool      is_rpadded;       // Right pad the text with blanks
	uint8_t   fg_color;         // Requested foreground color for text (c.f., FG_COLOR_INDEX_T enum)
	uint8_t   bg_color;         // Requested background color for text (c.f., BG_COLOR_INDEX_T enum)
	bool      is_overlay;       // Don't draw bg and use inverse of fb's underlying pixel as pen fg color
	bool      is_bgless;        // Don't draw bg (mutually exclusive with is_overlay)
	bool      is_fgless;        // Don't draw fg (takes precendence over is_overlay/is_bgless)
	bool      no_viewport;      // Ignore viewport corrections, whether hardware-related on Kobo, or to center rows
	bool      is_verbose;       // Print verbose diagnostic informations on stdout
	bool      is_quiet;         // Hide fbink_init()'s hardware setup info (sent to stderr)
	bool      ignore_alpha;     // Ignore any potential alpha channel in source image (i.e., flatten the image)
	uint8_t   halign;           // Horizontal alignment of images/dumps (c.f., ALIGN_INDEX_T enum)
	uint8_t   valign;           // Vertical alignment of images/dumps (c.f., ALIGN_INDEX_T enum)
	short int scaled_width;     // Output width of images/dumps (0 for no scaling, -1 for viewport width)
	short int scaled_height;    // Output height of images/dumps (0 for no scaling, -1 for viewport height)
				    // If only *one* of them is left at 0, the image's aspect ratio will be honored.
				    // If *either* of them is set to < -1, fit to screen while respecting AR.
				    // NOTE: Scaling is inherently costly. I highly recommend not relying on it,
				    //       preferring instead proper preprocessing of your input images,
				    //       c.f., https://www.mobileread.com/forums/showpost.php?p=3728291&postcount=17
	uint8_t wfm_mode;           // Request a specific waveform mode (c.f., WFM_MODE_INDEX_T enum; defaults to AUTO)
	bool    is_dithered;        // Request (ordered) hardware dithering (if supported).
	bool    sw_dithering;       // Request (ordered) *software* dithering when printing an image.
				    // This is *NOT* mutually exclusive with is_dithered!
	bool is_nightmode;          // Request hardware inversion (if supported/safe).
				    // This is *NOT* mutually exclusive with is_inverted!
	bool no_refresh;            // Skip actually refreshing the eInk screen (useful when drawing in batch)
} FBInkConfig;

// Same, but for OT/TTF specific stuff
typedef struct
{
	struct
	{
		short int top;       // Top margin in pixels (if negative, counts backwards from the bottom edge)
		short int bottom;    // Bottom margin in pixels (supports negative values, too)
		short int left;      // Left margin in pixels (if negative, counts backwards from the right edge)
		short int right;     // Right margin in pixels (supports negative values, too)
	} margins;
	float              size_pt;         // Size of text in points. If not set (0.0f), defaults to 12pt
	unsigned short int size_px;         // Size of text in pixels. Optional, but takes precedence over size_pt.
	bool               is_centered;     // Horizontal centering
	bool               is_formatted;    // Is string "formatted"? Bold/Italic support only, markdown like syntax
	bool               compute_only;    // Abort early after the line-break computation pass (no actual rendering).
	//                                     NOTE: This is early enough that it will *NOT* be able to predict *every*
	//                                           potential case of truncation.
	//                                           In particular, broken metrics may yield a late truncation at rendering time.
	bool no_truncation;    // Abort as early as possible (but not necessarily before the rendering pass),
			       // if the string cannot fit in the available area at the current font size.
} FBInkOTConfig;

// Optionally used with fbink_print_ot, if you need more details about the line-breaking computations,
// for instance if you want to dynamically compute a best-fit font size for n lines in a specific area.
typedef struct
{
	unsigned short int computed_lines;    // Expected amount of lines needed, according to font metrics.
	unsigned short int rendered_lines;    // Actually rendered amount of lines.
	//                                       Will stay 0 in case of an early abort (or a compute_only run),
	//                                       or < computed_lines in case of an unexpected truncation due to broken metrics.
	bool truncated;    // true if the string was truncated (at computation or rendering time).
} FBInkOTFit;

// This maps to an mxcfb rectangle, used for fbink_get_last_rect, as well as in FBInkDump
// NOTE: Unlike an mxcfb rectangle, left (x) comes *before* top (y)!
typedef struct
{
	unsigned short int left;    // x
	unsigned short int top;     // y
	unsigned short int width;
	unsigned short int height;
} FBInkRect;

// For use with fbink_dump & fbink_restore
typedef struct
{
	unsigned char* restrict data;
	size_t                  size;
	FBInkRect               area;
	FBInkRect clip;    // Only restore this rectangular area of the screen (has to intersect w/ the dump's area)
	uint8_t   rota;
	uint8_t   bpp;
	bool      is_full;
} FBInkDump;

//
////
//
// NOTE: Unless otherwise specified,
//       stuff returns a negative value (-(EXIT_FAILURE) by default) on failure & EXIT_SUCCESS otherwise ;).

// Returns the version of the currently loaded FBInk library.
FBINK_API const char* fbink_version(void) __attribute__((const));

//
// Open the framebuffer character device,
// and returns the newly opened file descriptor.
FBINK_API int fbink_open(void);

// Unmap the framebuffer (if need be) and close its file descriptor,
// (c.f., the recap at the bottom if you're concerned about mmap handling).
// fbfd:		Open file descriptor to the framebuffer character device, as returned by fbink_open()
// NOTE: This is safe to call if fbfd is FBFD_AUTO (i.e., -1, which means this is also safe to call after an fbink_open failure).
FBINK_API int fbink_close(int fbfd);

// Initialize internal variables keeping track of the framebuffer's configuration and state, as well as the device's hardware.
// MUST be called at least *once* before any fbink_print* or fbink_dump/restore functions.
// CAN safely be called multiple times,
//     but doing so is only necessary if the framebuffer's state has changed (although fbink_reinit is preferred in this case),
//     or if you modified one of the FBInkConfig fields that affects its results (listed below).
// fbfd:		Open file descriptor to the framebuffer character device,
//				if set to FBFD_AUTO, the fb is opened for the duration of this call.
// fbink_cfg:		Pointer to an FBInkConfig struct.
//				If you wish to customize them, the fields:
//				is_centered, fontmult, fontname, fg_color, bg_color,
//				no_viewport, is_verbose & is_quiet
//				MUST be set beforehand.
//				This means you MUST call fbink_init() again when you update them, too!
//				(This also means the effects from those fields "stick" across the lifetime of your application,
//				or until a subsequent fbink_init() (or effective fbink_reinit()) call gets fed different values).
// NOTE: By virtue of, well, setting global variables, do NOT consider this thread-safe.
//       The rest of the API should be, though, so make sure you init in your main thread *before* threading begins...
// NOTE: If you just need to make sure the framebuffer state is still up to date before an fbink_* call,
//       (f.g., because you're running on a Kobo, which may switch from 16bpp to 32bpp, or simply change orientation),
//       prefer using fbink_reinit instead of calling fbink_init *again*, as it's tailored for this use case.
//       c.f., KFMon for an example of this use case in the wild.
// NOTE: You can perfectly well keep a few different FBInkConfig structs around, instead of modifying the same one over and over.
//       Just remember that some fields *require* an fbink_init() call to be taken into account (see above),
//       but if the only fields that differ don't fall into that category, you do *NOT* need an fbink_init() per FBInkConfig...
FBINK_API int fbink_init(int fbfd, const FBInkConfig* restrict fbink_cfg);

//
// Dump a few of our internal state variables to stdout, in a format easily consumable by a shell (i.e., eval).
FBINK_API void fbink_state_dump(const FBInkConfig* restrict fbink_cfg);

// Dump a few of our internal state variables to the FBInkState struct pointed to by fbink_state.
// NOTE: This includes quite a few useful things related to device identification, c.f., the FBInkState struct ;).
//       You can also peek at the output of fbink -e to get a hint of what the data actually looks like.
FBINK_API void fbink_get_state(const FBInkConfig* restrict fbink_cfg, FBInkState* restrict fbink_state);

//
// Print a string on screen.
// NOTE: The string is expected to be encoded in valid UTF-8:
//         * Invalid UTF-8 sequences will be *rejected* and the call will abort early with -(EILSEQ)
//         * We assume a single multibyte sequence will occupy a maximum of 4 bytes.
//       c.f., my rant about Kobo's broken libc in fbink_internal.h for more details behind this choice.
//       Since any decent system built in the last decade should default to UTF-8, that should be pretty much transparent...
// Returns the amount of lines printed on success (helpful when you keep track of which row you're printing to).
// Returns -(EINVAL) if string is empty.
// Returns -(EILSEQ) if string is not a valid UTF-8 sequence.
// fbfd:		Open file descriptor to the framebuffer character device,
//				if set to FBFD_AUTO, the fb is opened & mmap'ed for the duration of this call.
// string:		UTF-8 encoded string to print.
// fbink_cfg:		Pointer to an FBInkConfig struct.
//				Honors every field not specifically related to image/dump support.
FBINK_API int fbink_print(int fbfd, const char* restrict string, const FBInkConfig* restrict fbink_cfg);

//
// Add an OpenType font to FBInk.
// NOTE: At least one font must be added in order to use fbink_print_ot().
// filename:		Path to the font file. This should be a valid *.otf or *.ttf font.
// style:		Defines the specific style of the specified font (FNT_REGULAR, FNT_ITALIC, FNT_BOLD or FNT_BOLD_ITALIC).
// NOTE: You MUST free the fonts loaded when you are done with all of them by calling fbink_free_ot_fonts().
// NOTE: You MAY replace a font without first calling fbink_free_ot_fonts().
// NOTE: Default fonts are secreted away in /usr/java/lib/fonts on Kindle,
//       and in /usr/local/Trolltech/QtEmbedded-4.6.2-arm/lib/fonts on Kobo,
//       but you can't use the Kobo ones because they're obfuscated...
//       Which leads me to a final, critical warning:
// NOTE: Don't try to pass non-font files or encrypted/obfuscated font files, because it *will* horribly segfault!
FBINK_API int fbink_add_ot_font(const char* filename, FONT_STYLE_T style);

// Free all loaded OpenType fonts. You MUST call this when you have finished all OT printing.
FBINK_API int fbink_free_ot_fonts(void);

// Print a string using an OpenType font.
// NOTE: The caller MUST have loaded at least one font via fbink_add_ot_font() FIRST.
// This function uses margins (in pixels) instead of rows/columns for positioning and setting the printable area.
// Returns a new top margin for use in subsequent calls, if the return value is positive.
// NOTE: A zero return value indicates there is no room left to print another row of text at the current margins or font size.
// Returns -(ERANGE) if the provided margins are out of range, or sum to < view height or width.
// Returns -(ENOSYS) when OT support is disabled (MINIMAL build).
// Returns -(ENODATA) if fbink_add_ot_font() hasn't been called yet.
// Returns -(EINVAL) if string is empty.
// Returns -(EILSEQ) if string is not a valid UTF-8 sequence.
// Returns -(ENOSPC) if no_truncation is true, and string needs to be truncated to fit in the available draw area.
//		     NOTE: This *cannot* prevent *drawing* truncated content on screen in *every* case,
//			   because broken metrics may skew our initial computations.
//			   As such, if the intent is to compute a "best fit" font size,
//			   no_truncation ought to be combined with no_refresh on eInk,
//			   (as we otherwise do *NOT* inhibit the refresh, in order to preserve get_last_rect's accuracy).
//			   You'll also probably want to do a cheaper compute_only pass first,
//			   to catch more obviously predictable truncations.
// fbfd:		Open file descriptor to the framebuffer character device,
//				if set to FBFD_AUTO, the fb is opened & mmap'ed for the duration of this call.
// string:		UTF-8 encoded string to print.
// cfg:			Pointer to an FBInkOTConfig struct.
// fbink_cfg:		Optional pointer to an FBInkConfig struct. If set, the fields
//				is_inverted, is_flashing, is_cleared, is_centered, is_halfway,
//				is_overlay, is_fgless, is_bgless, fg_color, bg_color, valign, halign,
//				wfm_mode, is_dithered, is_nightmode, no_refresh will be honored.
//				Pass a NULL pointer if unneeded.
// fit:			Optional pointer to an FBInkOTFit struct.
//				If set, it will be used to return information about the amount of lines needed to render
//				the string at the requested font size, and whether it was truncated or not.
//				Pass a NULL pointer if unneeded.
// NOTE: Alignment is relative to the printable area, as defined by the margins.
//       As such, it only makes sense in the context of a single, specific print call.
FBINK_API int fbink_print_ot(int                           fbfd,
			     const char* restrict          string,
			     const FBInkOTConfig* restrict cfg,
			     const FBInkConfig* restrict   fbink_cfg,
			     FBInkOTFit* restrict          fit);

//
// Brings printf formatting to fbink_print and fbink_print_ot ;).
// fbfd:		Open file descriptor to the framebuffer character device,
//				if set to FBFD_AUTO, the fb is opened & mmap'ed for the duration of this call.
// cfg:			Optional pointer to an FBInkOTConfig struct.
// fbink_cfg:		Optional pointer to an FBInkConfig struct.
// NOTE: If cfg is NULL, will call fbink_print, otherwise, fbink_print_ot!
//       If cfg is valid, fbink_cfg MAY be NULL (same behavior as fbink_print_ot).
//       If cfg is NULL, fbink_cfg MUST be valid.
// NOTE: Meaning at least one of those two pointers MUST be valid!
FBINK_API int fbink_printf(int                           fbfd,
			   const FBInkOTConfig* restrict cfg,
			   const FBInkConfig* restrict   fbink_cfg,
			   const char*                   fmt,
			   ...) __attribute__((format(printf, 4, 5)));

//
// A simple wrapper around the internal screen refresh handling, without requiring you to include einkfb/mxcfb headers.
// NOTE: Unlike FBInkRect, we *do* honor the original mxcfb rect order here (top before left).
// Returns -(ENOSYS) on non-eInk devices (i.e., pure Linux builds)
// fbfd:		Open file descriptor to the framebuffer character device,
//				if set to FBFD_AUTO, the fb is opened for the duration of this call.
// region_top:		top (y) field of an mxcfb rectangle.
// region_left:		left (x) field of an mxcfb rectangle.
// region_width:	width field of an mxcfb rectangle.
// region_height:	height field of an mxcfb rectangle.
// dithering_mode:	dithering mode (f.g., HWD_ORDERED, c.f., HW_DITHER_INDEX_T enum).
//			NOTE: Only supported on devices with a recent EPDC (>= v2)!
//			      On Kindle, that's everything since the KOA2 (KOA2, PW4, KT4, KOA3),
//			      On Kobo, that's everything since Mk.7.
//			NOTE: Even then, your device may not actually support anything other than PASSTHROUGH & ORDERED!
// fbink_cfg:		Pointer to an FBInkConfig struct. Honors wfm_mode, is_nightmode, is_flashing.
// NOTE: If you request an empty region (0x0 @ (0, 0), a full-screen refresh will be performed!
// NOTE: This *ignores* is_dithered & no_refresh ;).
// NOTE: If you do NOT want to request hardware dithering, set dithering_mode to HWD_PASSTHROUGH (i.e., 0).
//       This is also the fallback value.
FBINK_API int fbink_refresh(int                         fbfd,
			    uint32_t                    region_top,
			    uint32_t                    region_left,
			    uint32_t                    region_width,
			    uint32_t                    region_height,
			    uint8_t                     dithering_mode,
			    const FBInkConfig* restrict fbink_cfg);

// A simple wrapper around the MXCFB_WAIT_FOR_UPDATE_SUBMISSION ioctl, without requiring you to include mxcfb headers.
// Returns -(EINVAL) when the update marker is invalid.
// Returns -(ENOSYS) on devices where this ioctl is unsupported.
// NOTE: It is only implemented by Kindle kernels (K5+)!
// fbfd:		Open file descriptor to the framebuffer character device,
//				if set to FBFD_AUTO, the fb is opened for the duration of this call.
// marker:		The update marker you want to wait for.
// NOTE: If marker is set to LAST_MARKER (0U), the one from the last update sent by this FBInk session will be used instead.
//       If there aren't any, the call will fail and return -(EINVAL)!
// NOTE: Waiting for a random marker *should* simply return early.
FBINK_API int fbink_wait_for_submission(int fbfd, uint32_t marker);

// A simple wrapper around the MXCFB_WAIT_FOR_UPDATE_COMPLETE ioctl, without requiring you to include mxcfb headers.
// Returns -(EINVAL) when the update marker is invalid.
// Returns -(ENOSYS) on non-eInk devices (i.e., pure Linux builds).
// fbfd:		Open file descriptor to the framebuffer character device,
//				if set to FBFD_AUTO, the fb is opened for the duration of this call.
// marker:		The update marker you want to wait for.
// NOTE: If marker is set to LAST_MARKER (0U), the one from the last update sent by this FBInk session will be used instead.
//       If there aren't any, the call will fail and return -(EINVAL)!
// NOTE: Waiting for a random marker *should* simply return early.
FBINK_API int fbink_wait_for_complete(int fbfd, uint32_t marker);
// NOTE: For most single-threaded use-cases, you *probably* don't need to bother with this,
//       as all your writes to the framebuffer should obviously be serialized.
// NOTE: All our target devices should default to the QUEUE_AND_MERGE update scheme,
//       which means the EPDC itself will attempt to bundle updates together in order to do the least amount of work possible.
//       (By work, I mean moving the eInk capsules around, i.e., limiting to amount of effective refreshes).
//       A fun example of this can be seen with the dump/restore tests in utils/dump.c:
//       By default, it will call fbink_wait_for_complete at sensible times, but if you pass a random argument to it,
//       it won't: the difference that makes in practcie should be extremely obvious, especially for the first set of tests!
// NOTE: I encourage you to strace your stock reader to see how it makes use of those ioctls:
//       they're mostly used before and/or after FULL (i.e., flashing) updates,
//       to make sure they don't get affected by surrounding updates.
//       They can also be used to more predictably fence A2 updates.
//       In fact, for the most part, you can think of them as a kind of vsync fence.
//       Be aware that the ioctl will block for (relatively) longer than it takes for the refresh to visually end,
//       and that the delay depends for the most part on the waveform mode (flashing & region size have a much smaller impact).
//       With some waveform modes (mainly A2/DU), it'll return significantly earlier if the region's fb content hasn't changed.
// NOTE: See KOReader's mxc_update @ https://github.com/koreader/koreader-base/blob/master/ffi/framebuffer_mxcfb.lua
//       for some fancier examples in a complex application, where one might want to wait for completion of previous updates
//       right before sending a flashing one, for example.
// NOTE: Prior to FBInk 1.20.0, we used to *enforce* a wait_for_complete *after* *every* flashing (FULL) update.
//       This was originally done to mimic eips's behavior when displaying an image.
//       While blocking right after a refresh made sense for a one-off CLI tool, it's different for API users:
//       in most cases, it probably makes more sense to only block *before* the *following* flashing refresh,
//       thus ensuring that the wait will be shorter (or near zero), since time has probably passed between those two refreshes.
//       But, if you know that you won't be busy for a while after a flashing update, it might make sense to wait right after it,
//       in order to avoid an ioctl on the next refresh that might end up hurting reactivity...
//       Incidentally, as all of this depends on specific use-cases, this is why it is entirely left to the user,
//       and why there's no compatibility flag in FBInkConfig to restore the FBInk < 1.20 behavior ;).

// Return the update marker from the last *refresh* (explicit or implicit) done in this FBInk session.
// NOTE: Returns LAST_MARKER (0U) if there wasn't any, or on non-eInk devices (i.e., pure Linux builds).
// NOTE: Mainly useful if you want to do fairly fancy stuff with wait_for_complete/wait_for_submission,
//       otherwise, simply passing LAST_MARKER to 'em should do the trick.
FBINK_API uint32_t fbink_get_last_marker(void);

//
// Returns true if the device appears to be in a quirky framebuffer state that *may* require a reinit to produce sane results.
// NOTE: The intended use-case is for long running apps which may trigger prints across different framebuffer states,
//       to allow them to call fbink_init again at specific points only (instead of enforcing a reinit on every print).
//       This is of interest on a few devices, where trying to print based on a "stale" init state would fail,
//       or produce unwanted results (f.g., rotation).
// NOTE: Right now, this only checks for the isNTX16bLandscape device quirk,
//       because that's the only one that is not permanent (i.e., hardware specific),
//       but instead software specific (here, because of pickel).
//       In practical terms, this means the Kobo's fb is in 16bpp mode, with its origin in the top-right corner (i.e., Landscape).
// NOTE: Deprecated in favor of fbink_reinit ;).
FBINK_API bool fbink_is_fb_quirky(void) __attribute__((pure, deprecated));

// Attempt to detect changes in framebuffer states (between this call and the last time fbink_init/fbink_reinit was called),
// doing a reinit (i.e., calling fbink_init again) if needed, while doing the least amount of work possible in the process.
// NOTE: The intended use-case is for long running apps which may trigger prints across different framebuffer states,
//       to allow them to ensure they'll be using up-to-date init data at key points in their lifecycle
//       (without needing to bruteforce a full reinit on every print).
//       This is of interest on a few devices, where trying to print based on a "stale" init state would at worst fail,
//       at best produce unwanted results (f.g., after a bitdepth change or a hw rotation).
// NOTE: This obviously supercedes fbink_is_fb_quirky, because it should be smarter,
//       by catching more scenarios where a reinit would be useful,
//       and it can avoid running the same ioctl twice when an ioctl already done by init is needed to detect a state change.
// NOTE: Using fbink_reinit does NOT lift the requirement of having to run fbink_init at least ONCE,
//       i.e., you cannot replace the initial fbink_init call by fbink_reinit!
// Returns -(ENOSYS) on Kindle, where this is not needed.
// fbfd:		Open file descriptor to the framebuffer character device,
//				if set to FBFD_AUTO, the fb is opened for the duration of this call.
// fbink_cfg:		Pointer to an FBInkConfig struct.
FBINK_API int fbink_reinit(int fbfd, const FBInkConfig* restrict fbink_cfg);

//
// Print a full-width progress bar on screen.
// fbfd:		Open file descriptor to the framebuffer character device,
//				if set to FBFD_AUTO, the fb is opened & mmap'ed for the duration of this call.
// percentage:		0-100 value to set the progress bar's progression.
// fbink_cfg:		Pointer to an FBInkConfig struct (ignores is_overlay, col & hoffset;
//				as well as is_centered & is_padded).
FBINK_API int fbink_print_progress_bar(int fbfd, uint8_t percentage, const FBInkConfig* restrict fbink_cfg);

// Print a full-width activity bar on screen (i.e., an infinite progress bar).
// fbfd:		Open file descriptor to the framebuffer character device,
//				if set to FBFD_AUTO, the fb is opened & mmap'ed for the duration of this call.
// progress:		0-16 value to set the progress thumb's position in the bar.
// fbink_cfg:		Pointer to an FBInkConfig struct (ignores is_overlay, is_fgless, col & hoffset;
//				as well as is_centered & is_padded).
FBINK_API int fbink_print_activity_bar(int fbfd, uint8_t progress, const FBInkConfig* restrict fbink_cfg);

//
// Print an image on screen.
// Returns -(ENOSYS) when image support is disabled (MINIMAL build).
// fbfd:		Open file descriptor to the framebuffer character device,
//				if set to FBFD_AUTO, the fb is opened & mmap'ed for the duration of this call.
// filename:		Path to the image file (Supported formats: JPEG, PNG, TGA, BMP, GIF & PNM).
//				If set to "-" and stdin is not attached to a terminal,
//				will attempt to read image data from stdin.
// x_off:		Target coordinates, x (honors negative offsets).
// y_off:		Target coordinates, y (honors negative offsets).
// fbink_cfg:		Pointer to an FBInkConfig struct.
//				Where positioning is concerned, honors any combination of halign/valign, row/col & x_off/y_off;
//				otherwise, honors pretty much every other field not specifically concerned with text rendering.
// NOTE: Much like fbink_print_raw_data, for best performance,
//       an image that decodes in a pixel format close to the one used by the target device fb is best.
//       Generally, that'd be a Grayscale (color-type 0) PNG, ideally dithered down to the eInk palette
//       (c.f., https://www.mobileread.com/forums/showpost.php?p=3728291&postcount=17).
//       If you can't pre-process your images, dithering can be handled by the hardware on recent devices (c.f. is_dithered),
//       or by FBInk itself (c.f., sw_dithering), but the pixel format still matters:
//       On a 32bpp fb, Gray will still be faster than RGB.
//       On a 8bpp fb, try to only use Gray for the best performance possible,
//       as an RGB input will need to be grayscaled, making it slower than if it were rendered on a 32bpp fb!
//       Try to avoid using a 16bpp fb, as conversion to/from RGB565 will generally slow things down.
//       If you know you won't need to handle an alpha channel, don't forget ignore_alpha, too ;).
//       As expected, the fastest codepath is Gray on an 8bpp fb ;).
// NOTE: There's a direct copy fast path in the very specific case of printing a Grayscale image *without* alpha,
//       inversion or dithering on an 8bpp fb.
// NOTE: No such luck on 32bpp, because of a mandatory RGB <-> BGR conversion ;).
FBINK_API int fbink_print_image(int                         fbfd,
				const char*                 filename,
				short int                   x_off,
				short int                   y_off,
				const FBInkConfig* restrict fbink_cfg);

// Print raw scanlines on screen (packed pixels).
// Returns -(ENOSYS) when image support is disabled (MINIMAL build).
// fbfd:		Open file descriptor to the framebuffer character device,
//				if set to FBFD_AUTO, the fb is opened & mmap'ed for the duration of this call.
// data:		Pointer to a buffer holding the image data (Supported pixel formats: Y/YA/RGB/RGBA,
//				8-bit components, the first pixel should be the top-left of the image).
// w:			Width (in pixels) of a single scanline of the input image data.
// h:			Height (in pixels) of the full image data (i.e., amount of scanlines).
// len:			*Exact* size of the input buffer.
//				Input pixel format is simply computed as len / h / w, so this *needs* to be exact,
//				do not pass a padded length (or pad the data itself in any way)!
// x_off:		Target coordinates, x (honors negative offsets).
// y_off:		Target coordinates, y (honors negative offsets).
// fbink_cfg:		Pointer to an FBInkConfig struct.
//				Where positioning is concerned, honors any combination of halign/valign, row/col & x_off/y_off;
//				otherwise, honors pretty much every other field not specifically concerned with text rendering.
// NOTE: While we do accept a various range of input formats (as far as component interleaving is concerned),
//       our display code only handles a few specific combinations, depending on the target hardware.
//       To make everyone happy, this will transparently handle the pixel format conversion *as needed*,
//       a process which incurs a single copy of the input buffer (same behavior as in the non-raw image codepath).
//       If this is a concern to you, make sure your input buffer is formatted in a manner adapted to your output device:
//       Generally, that'd be RGBA (32bpp) on Kobo (or RGB (24bpp) with ignore_alpha),
//       and YA (grayscale + alpha) on Kindle (or Y (8bpp) with ignore_alpha).
FBINK_API int fbink_print_raw_data(int                         fbfd,
				   unsigned char*              data,
				   const int                   w,
				   const int                   h,
				   const size_t                len,
				   short int                   x_off,
				   short int                   y_off,
				   const FBInkConfig* restrict fbink_cfg);

//
// Just clear the screen, eInk refresh included (or not ;)).
// fbfd:		Open file descriptor to the framebuffer character device,
//				if set to FBFD_AUTO, the fb is opened & mmap'ed for the duration of this call.
// fbink_cfg:		Pointer to an FBInkConfig struct (honors is_inverted, wfm_mode, is_dithered, is_nightmode, is_flashing,
//				as well as no_refresh, pen_fg_color & pen_bg_color).
FBINK_API int fbink_cls(int fbfd, const FBInkConfig* restrict fbink_cfg);

//
// Dump the full screen.
// Returns -(ENOSYS) when image support is disabled (MINIMAL build).
// fbfd:		Open file descriptor to the framebuffer character device,
//				if set to FBFD_AUTO, the fb is opened & mmap'ed for the duration of this call.
// dump:		Pointer to an FBInkDump struct (will be recycled if already used).
// NOTE: As with all FBInk structs, FBInkDump *must* be zero-initialized.
//       Storage for the dump will be allocated on the heap by FBInk,
//       but releasing that memory (i.e., free(dump.data);) is the caller's burden.
//       Care should be taken not to leave that pointer dangling (i.e., dump.data = NULL;),
//       as a subsequent call to fbink_*_dump with that same struct would otherwise trip the recycling check,
//       causing a double free!
//       You can use the fbink_free_dump_data() helper function to do just that.
//       There are no error codepaths after storage allocation (i.e., you are assured that it has NOT been allocated on error).
//       Note that a recycling *will* clear the clip FBInkRect!
// NOTE: On *most* devices (the exceptions being 4bpp & 16bpp fbs),
//       the data being dumped is perfectly valid input for fbink_print_raw_data,
//       in case you'd ever want to do some more exotic things with it...
FBINK_API int fbink_dump(int fbfd, FBInkDump* restrict dump);

// Dump a specific region of the screen.
// Returns -(ENOSYS) when image support is disabled (MINIMAL build).
// fbfd:		Open file descriptor to the framebuffer character device,
//				if set to FBFD_AUTO, the fb is opened & mmap'ed for the duration of this call.
// x_off:		Dump coordinates, x (honors negative offsets).
// y_off:		Dump coordinates, y (honors negative offsets).
// w:			Width of the region to dump.
// h:			Height of the region to dump.
// fbink_cfg:		Pointer to an FBInkConfig struct (honors any combination of halign/valign, row/col & x_off/y_off).
// dump:		Pointer to an FBInkDump struct (will be recycled if already used).
// NOTE: The same considerations as in fbink_dump should be taken regarding the handling of FBInkDump structs.
FBINK_API int fbink_region_dump(int                         fbfd,
				short int                   x_off,
				short int                   y_off,
				unsigned short int          w,
				unsigned short int          h,
				const FBInkConfig* restrict fbink_cfg,
				FBInkDump* restrict         dump);

// Restore a framebuffer dump made by fbink_dump/fbink_region_dump.
// Returns -(ENOSYS) when image support is disabled (MINIMAL build).
// Otherwise, returns a few different things on failure:
//	-(ENOTSUP)	when the dump cannot be restored because it wasn't taken at the current bitdepth and/or rotation,
//			or because it's wider/taller/larger than the current framebuffer, or if the crop is invalid (OOB).
//	-(EINVAL)	when there's no data to restore.
// fbfd:		Open file descriptor to the framebuffer character device,
//				if set to FBFD_AUTO, the fb is opened & mmap'ed for the duration of this call.
// fbink_cfg:		Pointer to an FBInkConfig struct (honors wfm_mode, is_dithered, is_nightmode, is_flashing & no_refresh).
// dump:		Pointer to an FBInkDump struct, as setup by fbink_dump or fbink_region_dump.
// NOTE: In case the dump was regional, it will be restored in the exact same coordinates it was taken from,
//       no actual positioning is needed/supported at restore time.
// NOTE: This does not support any kind of software processing, at all!
//       If you somehow need inversion or dithering, it has to be supported at the hardware level at refresh time by your device,
//       (i.e., is_dithered vs. sw_dithering, and is_nightmode vs. is_inverted).
//       At most common bitdepths, you can somewhat work around these restrictions, obviously at a performance premium,
//       by using fbink_print_raw_data instead (see the relevant notes for fbink_dump), with a few quirky caveats...
//       c.f., the last few tests in utils/dump.c for highly convoluted examples that I don't recommend replicating in production.
// NOTE: "current" actually means "at last init/reinit time".
//       Call fbink_reinit first if you really want to make sure bitdepth/rotation still match.
// NOTE: If you need to restore only part of a dump, you can do so via the clip field of the FBInkDump struct.
//       This FBInkRect is the only field you should ever modify yourself.
//       This clip rectangle is relative to the *screen*, not the dump's area (i.e., these are absolute screen coordinates).
//       As such, it has to intersect with the dump's area, or the call will fail.
//       And while it can safely completely overlap the dump's area, it still needs to be constrained to the screen's dimension.
//       Of course, only the intersection of this rectangle with the dump's area will be restored.
//       Be aware that you'll also need to flip the is_full field yourself first if you ever need to crop a full dump.
// NOTE: This does *NOT* free data.dump!
FBINK_API int fbink_restore(int fbfd, const FBInkConfig* restrict fbink_cfg, const FBInkDump* restrict dump);

// Free the data allocated by a previous fbink_dump() or fbink_region_dump() call.
// Returns -(ENOSYS) when image support is disabled (MINIMAL build).
// Otherwise, returns a few different things on failure:
//	-(EINVAL)	when the dump has already been freed.
// dump:		Pointer to an FBInkDump struct.
// NOTE: You MUST call this when you have no further use for this specific data.
// NOTE: But, you MAY re-use a single FBInkDump struct across different dump() calls *without* calling this in between,
//       as dump() will implicitly free a dirty struct in order to recycle it.
FBINK_API int fbink_free_dump_data(FBInkDump* restrict dump);

//
// Return the coordinates & dimensions of the last thing that was *drawn*.
// Returns an empty (i.e., {0, 0, 0, 0}) rectangle if nothing was drawn.
// NOTE: These are unfiltered *framebuffer* coordinates.
//       If your goal is to use that for input detection, mapping that to input coordinates is your responsibility.
//       On Kobo, fbink_get_state should contain enough data to help you figure out what kinds of quirks you need to account for.
// NOTE: While this *generally* maps to the refresh region, this does not always hold true:
//       this will get updated regardless of no_refresh,
//       and will ignore what is_flashing might do to make the refresh region fullscreen.
//       i.e., it corresponds to what's drawn to the fb, not necessarily to what's refreshed on screen.
// NOTE: On devices where we may fudge the coordinates to account for broken rotation (i.e., most Kobos @ 16bpp),
//       these are the *rotated* coordinates!
//       i.e., they *will* match with what we actually send to mxcfb (and where we actually drew on the fb)!
FBINK_API FBInkRect fbink_get_last_rect(void);

//
// Scan the screen for Kobo's "Connect" button in the "USB plugged in" popup,
// and optionally generate an input event to press that button.
// KOBO Only! Returns -(ENOSYS) when disabled (!KOBO, as well as MINIMAL builds).
// Otherwise, returns a few different things on failure:
//	-(EXIT_FAILURE)	when the button was not found.
//	With press_button:
//	-(ENODEV)	when we couldn't generate a touch event at all (unlikely to ever happen on current HW).
//	-(ENOTSUP)	when the generated touch event appeared to have failed to actually tap the button.
//				Emphasis on "appeared to", it's tricky to be perfectly sure the right thing happened...
//				CANNOT happen when nosleep is true (because it skips this very codepath).
// NOTE: For the duration of this call, screen updates should be kept to a minimum: in particular,
//       we of course expect to be able to see the "Connect" button,
//       but we also expect the middle section of the final line to be untouched!
// fbfd:		Open file descriptor to the framebuffer character device,
//				if set to FBFD_AUTO, the fb is opened & mmap'ed for the duration of this call.
// press_button:	Generate an input event to press the button if true,
//				MAY sleep up to 5s to confirm that input was successful! (unless nosleep is true).
// nosleep:		If true, don't try to confirm that press_button's input event was successful,
//				avoiding the nanosleep() calls that would incur...
// NOTE: Thread-safety obviously goes out the window with press_button enabled,
//       since you can then only reasonably expect to be able to concurrently run a single instance of that function ;).
FBINK_API int fbink_button_scan(int fbfd, bool press_button, bool nosleep);

// Wait for the end of a Kobo USBMS session, trying to detect a successful content import in the process.
// NOTE: Expects to be called while in the "Connected" state (like after a successful fbink_button_scan() call w/ press_buton)!
//       It will abort early if that's not the case.
// NOTE: For the duration of this call (which is obviously blocking!), screen updates should be kept to a minimum:
//       in particular, we expect the middle section of the final line to be untouched!
// KOBO Only! Returns -(ENOSYS) when disabled (!KOBO, as well as MINIMAL builds).
// Otherwise, returns a few different things on failure:
//	-(EXIT_FAILURE)	when the expected chain of events fails to be detected properly.
//	-(ENODATA)	when there was no new content to import at the end of the USBMS session.
//	-(ETIME)	when we failed to detect the end of the import session itself, because it ran longer than 5 minutes.
// fbfd:		Open file descriptor to the framebuffer character device,
//				if set to FBFD_AUTO, the fb is opened & mmap'ed for the duration of this call.
// force_unplug:	After having made sure to be in USBMS mode, generate a fake USB unplug event to force Nickel to wake up.
//				This makes sense if you want to do stuff behind Nickel's back during the USBMS session,
//				instead of simply monitoring it, especially with fake USBMS sessions ;).
//				NOTE: Obviously, if this was a real USBMS session, and not an entirely faked one,
//				      if you force an unplug while onboard is still mounted on the connected to machine,
//				      shit will go horribly wrong!
// NOTE: Thread-safety obviously goes out the window with force_unplug enabled,
//       since you can then only reasonably expect to be able to concurrently run a single instance of that function ;).
FBINK_API int fbink_wait_for_usbms_processing(int fbfd, bool force_unplug);

//
///
//
// When you intend to keep the framebuffer fd open for the lifecycle of your program:
// fd = open() -> init(fd, ...) -> print*(fd, ...) -> ... -> close(fd)
// NOTE: This implies keeping the framebuffer's mmap around, too.
//       The initial mmap will only happen on the first function call that actually needs to write to the fb, i.e., print*.
//       On the upside, that's going to be the only mmap to ever happen, as subsequent print* calls will re-use it.
//
// Otherwise, you can simply forget about open() & close(), and just do:
// init(FBFD_AUTO, ...)
// And then whenever you want to print something:
// print*(FBFD_AUTO, ...)
//
// See fbink_cmd.c for an example of the former, and KFMon for an example of the latter.
// NOTE: Although fairly stupid in practice, utils/dump.c is less convoluted than fbink_cmd.c, making it worth a look...

#endif
