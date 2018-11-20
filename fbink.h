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
#	define DLL_LOCAL __attribute__((visibility("hidden")))
#else
#	define DLL_PUBLIC
#	define DLL_LOCAL
#endif

// Are we actually building the shared lib?
#ifdef FBINK_SHAREDLIB
#	define FBINK_API DLL_PUBLIC
#	define FBINK_LOCAL DLL_LOCAL
#else
#	define FBINK_API
#	define FBINK_LOCAL
#endif

// Magic number for automatic fbfd handling
#define FBFD_AUTO -1

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
	SCIENTIFICAI       // scientifica (italic)
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

// A struct to dump FBInk's internal state into, like fbink_state_dump() would, but in C ;)
typedef struct
{
	uint32_t           view_width;          // viewWidth
	uint32_t           view_height;         // viewHeight
	uint32_t           screen_width;        // screenWidth
	uint32_t           screen_height;       // screenHeight
	uint32_t           bpp;                 // vInfo.bits_per_pixel
	uint8_t            pen_fg_color;        // penFGColor
	uint8_t            pen_bg_color;        // penFGColor
	unsigned short int screen_dpi;          // deviceQuirks.screenDPI
	long int           user_hz;             // USER_HZ
	const char*        font_name;           // fbink_cfg->fontname
	unsigned short int font_w;              // FONTW
	unsigned short int font_h;              // FONTH
	unsigned short int max_cols;            // MAXCOLS
	unsigned short int max_rows;            // MAXROWS
	uint8_t            view_hori_origin;    // viewHoriOrigin
	uint8_t            view_vert_origin;    // viewVertOrigin
	uint8_t            view_vert_offset;    // viewVertOffset
	uint8_t            fontsize_mult;       // FONTSIZE_MULT
	uint8_t            glyph_width;         // glyphWidth
	uint8_t            glyph_height;        // glyphHeight
	bool               is_perfect_fit;      // deviceQuirks.isPerfectFit
} FBInkState;

// What a FBInk config should look like. Perfectly sane when fully zero-initialized.
typedef struct
{
	short int row;             // y axis (i.e., line), counts down from the bottom of the screen if negative
	short int col;             // x axis (i.e., column), counts down from the right edge of the screen if negative
	uint8_t   fontmult;        // Font scaling multiplier (i.e., 4 -> x4), 0 means automatic.
	uint8_t   fontname;        // Request a specific font (c.f., FONT_INDEX_T enum)
	bool      is_inverted;     // Invert colors
	bool      is_flashing;     // Request a black flash on refresh
	bool      is_cleared;      // Clear the screen beforehand (honors is_inverted)
	bool      is_centered;     // Center the text (horizontally)
	short int hoffset;         // Horizontal offset (in pixels) for text position
	short int voffset;         // Vertical offset (in pixels) for text position
	bool      is_halfway;      // Vertically center the text, honoring row offsets
	bool      is_padded;       // Pad the text with blanks (on the left, or on both sides if is_centered)
	uint8_t   fg_color;        // Requested foreground color for text (c.f., FG_COLOR_INDEX_T enum)
	uint8_t   bg_color;        // Requested background color for text (c.f., BG_COLOR_INDEX_T enum)
	bool      is_overlay;      // Don't draw bg, use inverse of fb's underlying pixel as pen fg color
	bool      is_bgless;       // Don't draw bg (mutually exclusive with is_overlay)
	bool      is_fgless;       // Don't draw fg (takes precendence over is_overlay/is_bgless)
	bool      no_viewport;     // Ignore viewport corrections, whether hardware-related on Kobo, or to center rows
	bool      is_verbose;      // Print verbose diagnostic informations on stdout
	bool      is_quiet;        // Hide fbink_init()'s hardware setup info (sent to stderr)
	bool      ignore_alpha;    // Ignore any potential alpha channel in source image (i.e., flatten the image)
	uint8_t   halign;    // Horizontal alignment of images (NONE/LEFT, CENTER, EDGE/RIGHT; c.f., ALIGN_INDEX_T enum)
	uint8_t   valign;    // Vertical alignment of images (NONE/TOP, CENTER, EDGE/BOTTOM; c.f., ALIGN_INDEX_T enum)
	bool      no_refresh;    // Skip actually refreshing the eInk screen (useful when drawing in batch)
} FBInkConfig;

typedef struct
{
	struct
	{
		short int top;       // Top margin in pixels (if negative, count backwards from the bottom edge)
		short int bottom;    // Bottom margin in pixels (supports negative values, too)
		short int left;      // Left margin in pixels (if negative, count backwards from the right edge)
		short int right;     // Right margin in pixels (supports negative values, too)
	} margins;
	unsigned short int size_pt;         // Size of text in points. If not set (0), defaults to 12pt
	bool               is_centered;     // Horizontal centering
	bool               is_formatted;    // Is string "formatted"? Bold/Italic support only, markdown like syntax
} FBInkOTConfig;

// NOTE: Unless otherwise specified,
//       stuff returns a negative value (usually -(EXIT_FAILURE)) on failure & EXIT_SUCCESS otherwise ;).

// Returns the version of the currently loaded FBInk library
FBINK_API const char* fbink_version(void) __attribute__((const));

// Open the framebuffer character device,
// Returns the newly opened file descriptor
FBINK_API int fbink_open(void);

// Unmap the framebuffer (if need be) and close its file descriptor
// (c.f., the recap at the bottom if you're concerned about mmap handling).
// fbfd:		Open file descriptor to the framebuffer character device, as returned by fbink_open()
FBINK_API int fbink_close(int fbfd);

// Initialize internal variables keeping track of the framebuffer's configuration and state, as well as the device's hardware.
// MUST be called at least *once* before any fbink_print* functions.
// CAN safely be called multiple times, but doing so is only necessary if the framebuffer's state has changed,
//     or if you modified one of the FBInkConfig fields that affects its results (listed below).
// fbfd:		Open file descriptor to the framebuffer character device,
//				if set to FBFD_AUTO, the fb is opened & mmap'ed for the duration of this call
// fbink_cfg:		Pointer to an FBInkConfig struct
//				If you wish to customize them, the fields:
//				is_centered, fontmult, fontname, fg_color, bg_color, no_viewport, is_verbose & is_quiet
//				MUST be set beforehand.
//				This means you MUST call fbink_init() again when you update them, too!
// NOTE: By virtue of, well, setting global variables, do NOT consider this thread-safe.
//       The rest of the API should be, though, so make sure you init in your main thread *before* threading begins...
// NOTE: On devices where the fb state can change (i.e., Kobos switching between 16bpp & 32bpp),
//       this needs to be called as many times as necessary to ensure that every following fbink_* call will be made
//       against a fb state that matches the state it was in during the last fbink_init() call...
//       c.f., KFMon's handling of this via fbink_is_fb_quirky() to detect the initial 16bpp -> 32bpp switch.
FBINK_API int fbink_init(int fbfd, const FBInkConfig* fbink_cfg);

// Add an OpenType font to FBInk. Note that at least one font must be added in order to use fbink_print_ot()
// Returns -(EXIT_FAILURE) on failure, or EXIT_SUCCESS otherwise
// filename:		The font file path. This should be a valid *.otf or *.ttf font
// style:		Defines the specific style of the specified font (FNT_REGULAR, FNT_ITALIC, FNT_BOLD, FNT_BOLD_ITALIC)
// NOTE: You MUST free the fonts loaded when you are done by calling fbink_free_ot_fonts()
// NOTE: You may replace a font without first calling free
// NOTE: Default fonts are secreted away in /usr/java/lib/fonts on Kindle,
//       and in /usr/local/Trolltech/QtEmbedded-4.6.2-arm/lib/fonts on Kobo,
//       but you can't use the Kobo ones because they're obfuscated...
//       Which leads me to a final, critical warning:
// NOTE: Don't try to pass non-font files or encrypted/obfuscated font files, because it *will* horribly segfault!
FBINK_API int fbink_add_ot_font(const char* filename, FONT_STYLE_T style);

// Free all loaded OpenType fonts. You MUST call this when you have finished all OT printing.
FBINK_API int fbink_free_ot_fonts(void);

// Dump a few of our internal state variables to stdout, in a format easily consumable by a shell (i.e., eval)
FBINK_API void fbink_state_dump(const FBInkConfig* fbink_cfg);

// Dump a few of our internal state variables to the FBInkState struct pointed to by fbink_state
FBINK_API void fbink_get_state(const FBInkConfig* fbink_cfg, FBInkState* fbink_state);

// Print a string on screen.
// NOTE: The string is expected to be encoded in valid UTF-8, no validation of any kind is done by the library,
//       and we assume a single multibyte sequence will occupy a maximum of 4 bytes.
//       c.f., my rant about Kobo's broken libc in fbink_internal.h for more details behind this choice.
//       Since any decent system built in the last decade should default to UTF-8, that should be pretty much transparent...
// Returns the amount of lines printed on success (helpful when you keep track of which row you're printing to).
// fbfd:		Open file descriptor to the framebuffer character device,
//				if set to FBFD_AUTO, the fb is opened & mmap'ed for the duration of this call
// string:		UTF-8 encoded string to print
// fbink_cfg:		Pointer to an FBInkConfig struct
FBINK_API int fbink_print(int fbfd, const char* string, const FBInkConfig* fbink_cfg);

// Print a string using an OpenType font. Note the caller MUST init with fbink_init_ot() FIRST.
// This function uses positive margins (in pixels) instead of rows/columns for positioning and setting the printable area.
// Returns new top margin for use in subsequent calls, if the return value is positive.
// 		A zero return value indicates there is no room left to print another row of text at the current
// 		margins or font size.
// Returns -(ERANGE) if the provided margins are out of range, or sum to < view height or width
// Returns -(ENOSYS) if compiled with MINIMAL
// Returns -(ENODAT) if fbink_init_ot() hasn't yet been called.
// fbfd:		Open file descriptor to the framebuffer character device,
//				if set to FBFD_AUTO, the fb is opened & mmap'ed for the duration of this call
// string:		UTF-8 encoded string to print
// cfg:			Pointer to a FBInkOTConfig struct.
// fbink_cfg:		Optional pointer to a FBInkConfig struct. If set, the options
//				is_inverted, is_flashing, is_cleared, is_centered, is_halfway, is_overlay, is_fgless, is_bgless,
//				fg_color, bg_color, valign, halign will be honored.
//				Pass a NULL pointer if unneeded.
// NOTE: Alignment is relative to the printable area, as defined by the margins.
//       As such, it only makes sense in the context of a single, specific print call.
FBINK_API int fbink_print_ot(int fbfd, const char* string, const FBInkOTConfig* cfg, const FBInkConfig* fbink_cfg);

// Brings printf formatting to fbink_print and fbink_print_ot ;).
// fbfd:		Open file descriptor to the framebuffer character device,
//				if set to FBFD_AUTO, the fb is opened & mmap'ed for the duration of this call
// cfg:			Optional pointer to an FBInkOTConfig struct.
// fbink_cfg:		Optional pointer to an FBInkConfig struct.
// NOTE: If cfg is NULL, will call fbink_print, otherwise, fbink_print_ot!
//       If cfg is valid, fbink_cfg MAY be NULL (same behavior as fbink_print_ot)
//       If cfg is NULL, fbink_cfg MUST be valid
// NOTE: Meaning at least one of those two pointers MUST be valid!
FBINK_API int fbink_printf(int fbfd, const FBInkOTConfig* cfg, const FBInkConfig* fbink_cfg, const char* fmt, ...)
    __attribute__((format(printf, 4, 5)));

// A simple wrapper around the internal screen refresh handling, without requiring you to include einkfb/mxcfb headers
// fbfd:		Open file descriptor to the framebuffer character device,
//				if set to FBFD_AUTO, the fb is opened & mmap'ed for the duration of this call
// region_top:		top (y) field of an mxcfb rectangle
// region_left:		left (x) field of an mxcfb rectangle
// region_width:	width field of an mxcfb rectangle
// region_height:	height field of an mxcfb rectangle
// waveform_mode:	waveform mode (i.e, "GC16")
// is_flashing:		will ask for a black flash if true
// NOTE: If you request an empty region (0x0 @ (0, 0), a full-screen refresh will be performed!
// NOTE: As you've probably guessed, since it doesn't take an FBInkConfig pointer, this *ignores* no_refresh ;)
FBINK_API int fbink_refresh(int         fbfd,
			    uint32_t    region_top,
			    uint32_t    region_left,
			    uint32_t    region_width,
			    uint32_t    region_height,
			    const char* waveform_mode,
			    bool        is_flashing);

// Returns true if the device appears to be in a quirky framebuffer state that *may* require a reinit to produce sane results.
// NOTE: The intended use-case is for long running apps which may trigger prints across different framebuffer states,
//       to allow them to call fbink_init again at specific points only (instead of enforcing a reinit on every print).
//       This is of interest on a few devices, where trying to print based on a "stale" init state would fail,
//       or produce unwanted results (f.g., rotation).
// NOTE: Right now, this only checks for the isNTX16bLandscape Device Quirk,
//       because that's the only one that is not permanent (i.e., hardware specific),
//       but instead software specific (here, because of pickel).
//       In practical terms, this means the Kobo's fb is in 16bpp mode, with its origin in the top-right corner (i.e., Landscape).
// NOTE: Deprecated in favor of fbink_reinit ;).
FBINK_API bool fbink_is_fb_quirky(void) __attribute__((deprecated));

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
// Returns -(ENOSYS) on Kindle, where this is not needed
// fdfd:		Open file descriptor to the framebuffer character device,
//				if set to FBFD_AUTO, the fb is opened & mmap'ed for the duration of this call
// fbink_cfg:		Pointer to an FBInkConfig struct
FBINK_API int fbink_reinit(int fbfd, const FBInkConfig* fbink_cfg);

// Print a full-width progress bar on screen
// fdfd:		Open file descriptor to the framebuffer character device,
//				if set to FBFD_AUTO, the fb is opened & mmap'ed for the duration of this call
// percentage:		0-100 value to set the progress bar's progression
// fbink_cfg:		Pointer to an FBInkConfig struct (ignores is_overlay, col & hoffset; as well as is_centered & is_padded)
FBINK_API int fbink_print_progress_bar(int fbfd, uint8_t percentage, const FBInkConfig* fbink_cfg);

// Print a full-width activity bar on screen (i.e., an infinite progress bar)
// fdfd:		Open file descriptor to the framebuffer character device,
//				if set to FBFD_AUTO, the fb is opened & mmap'ed for the duration of this call
// progress:		0-16 value to set the progress thumb's position in the bar
// fbink_cfg:		Pointer to an FBInkConfig struct (ignores col & hoffset; as well as is_centered & is_padded)
FBINK_API int fbink_print_activity_bar(int fbfd, uint8_t progress, const FBInkConfig* fbink_cfg);

// Print an image on screen
// Returns -(ENOSYS) when image support is disabled (MINIMAL build)
// fdfd:		Open file descriptor to the framebuffer character device,
//				if set to FBFD_AUTO, the fb is opened & mmap'ed for the duration of this call
// filename:		Path to the image file (Supported formats: JPEG, PNG, TGA, BMP, GIF & PNM)
//				if set to "-" and stdin is not attached to a terminal,
//				will attempt to read image data from stdin.
// x_off:		Target coordinates, x (honors negative offsets)
// y_off:		Target coordinates, y (honors negative offsets)
// fbink_cfg:		Pointer to an FBInkConfig struct (honors any combination of halign/valign, row/col & x_off/y_off)
FBINK_API int fbink_print_image(int                fbfd,
				const char*        filename,
				short int          x_off,
				short int          y_off,
				const FBInkConfig* fbink_cfg);

// Print raw scanlines on screen
// Returns -(ENOSYS) when image support is disabled (MINIMAL build)
// fdfd:		Open file descriptor to the framebuffer character device,
//				if set to FBFD_AUTO, the fb is opened & mmap'ed for the duration of this call
// data:		Pointer to a buffer holding the image data (Supported pixel formats: Y/YA/RGB/RGBA,
//				8-bit components, the first pixel should be the top-left of the image).
// w:			Width (in pixels) of a single scanline of the input image data
// h:			Height (in pixels) of the full image data (i.e., amount of scanlines)
// len:			*Exact* size of the input buffer.
//				Input pixel format is simply computed as len / h / w, so this *needs* to be exact,
//				do not pass a padded length (or pad the data itself in any way)!
// x_off:		Target coordinates, x (honors negative offsets)
// y_off:		Target coordinates, y (honors negative offsets)
// fbink_cfg:		Pointer to an FBInkConfig struct (honors any combination of halign/valign, row/col & x_off/y_off)
// NOTE: While we do accept a various range of input formats (as far as component interleaving is concerned),
//       our display code only handles a few specific combinations, depending on the target hardware.
//       To make everyone happy, this will transparently handle the pixel format conversion *as needed*,
//       a process which incurs a single copy of the input buffer (same behavior as in the non-raw image codepath).
//       If this is a concern to you, make sure your input buffer is formatted in a manner adapted to your output device:
//       RGBA (32bpp) on Kobo (or RGB (24bpp) with ignore_alpha),
//       and YA (grayscale + alpha) on Kindle (or Y (8bpp) with ignore_alpha).
FBINK_API int fbink_print_raw_data(int                fbfd,
				   unsigned char*     data,
				   const int          w,
				   const int          h,
				   const size_t       len,
				   short int          x_off,
				   short int          y_off,
				   const FBInkConfig* fbink_cfg);

// Scan the screen for Kobo's "Connect" button in the "USB plugged in" popup,
// and optionally generate an input event to press that button.
// KOBO Only! Returns -(ENOSYS) when disabled (!KOBO, as well as MINIMAL builds).
// Otherwise, returns a few different things on failure:
//	-(EXIT_FAILURE)	when the button was not found
//	With press_button:
//	-(ENODEV)	when we couldn't generate a touch event at all (unlikely to ever happen on current HW)
//	-(ENOTSUP)	when the generated touch event appeared to have failed to actually tap the button
//				emphasis on "appeared to", it's tricky to be perfectly sure the right thing happened...
//				CANNOT happen when nosleep is true (because it skips this very codepath).
// NOTE: For the duration of this call, screen updates should be kept to a minimum: in particular,
//       we of course expect to be able to see the "Connect" button,
//       but we also expect the middle section of the final line to be untouched!
// fdfd:		Open file descriptor to the framebuffer character device,
//				if set to FBFD_AUTO, the fb is opened & mmap'ed for the duration of this call
// press_button:	Generate an input event to press the button if true,
//				MAY sleep up to 5s to confirm that input was successful! (unless nosleep is true)
// nosleep:		If true, don't try to confirm that press_button's input event was successful,
//				avoiding the nanosleep() calls that might incur...
// NOTE: Thread-safety obviously goes out the window with press_button enabled,
//       since you can then only reasonably expect to be able to concurrently run a single instance of that function ;).
FBINK_API int fbink_button_scan(int fbfd, bool press_button, bool nosleep);

// Wait for the end of a Kobo USBMS session, trying to detect a successful content import in the process.
// NOTE: Expects to be called while in the "Connected" state (like after a successful fbink_button_scan() call w/ press_buton)!
//       It will abort early if that's not the case.
// NOTE: For the duration of this call (which is obviously blocking!), screen updates should be kept to a minimum:
//       in particular, we expect the middle section of the final line to be untouched!
// KOBO Only! Returns -(ENOSYS) when disabled (!KOBO, as well as MINIMAL builds)
// Otherwise, returns a few different things on failure:
//	-(EXIT_FAILURE)	when the expected chain of events fails to be detected properly
//	-(ENODATA)	when there was no new content to import at the end of the USBMS session
//	-(ETIME)	when we failed to detect the end of the import session itself, because it ran longer than 5 minutes.
// fdfd:		Open file descriptor to the framebuffer character device,
//				if set to FBFD_AUTO, the fb is opened & mmap'ed for the duration of this call
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

#endif
