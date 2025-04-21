/*
	FBInk: FrameBuffer eInker, a library to print text & images to an eInk Linux framebuffer
	Copyright (C) 2018-2024 NiLuJe <ninuje@gmail.com>
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

#include <linux/fb.h>

// Be friendly with C++ compilers (both GCC & Clang support __restrict__).
#ifdef __cplusplus
#	define restrict __restrict__
extern "C" {
#endif

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
#define FBFD_AUTO   -1
// As 0 is an invalid marker value, we can coopt it to try to retrieve our own last sent marker
#define LAST_MARKER 0U

// NOTE: There's a dirty bit of trickery involved here to coerce enums into a specific data type (instead of an int):
//       * The packed attribute, which does locally what GCC's -fshort-enums does globally,
//         ensuring the enum's data type is only as wide as the actual values require.
//       * A tail element with a value corresponding to the MAX of the target data type,
//         mainly used to enforce an unsigned data type.
//       BUT, while this "works" in C, this is essentially non-standard, and would break most automatic bindings,
//       which happily assume we're not breaking the C standard, and as such expect an enum to be int-sized...
//       So, in addition to that, instead of using the enum's typedef directly, we go through an explicit typedef.
//       This obviously still works in C because enums are dumb, they're basically unscoped constants,
//       and this makes bindings happy because they'll have an explicit typedef to rely on.
//       Fun fact: this is essentially what the Go bindings were already doing ;).
//
// Supported targets
typedef enum
{
	FBINK_TARGET_LINUX = 0U,
	FBINK_TARGET_KOBO,
	FBINK_TARGET_KINDLE,
	FBINK_TARGET_KINDLE_LEGACY,
	FBINK_TARGET_CERVANTES,
	FBINK_TARGET_REMARKABLE,
	FBINK_TARGET_POCKETBOOK,
	FBINK_TARGET_MAX = UINT8_MAX,
} __attribute__((packed)) FBINK_TARGET_E;
typedef uint8_t           FBINK_TARGET_T;

// Supported feature flags
#define FBINK_FEATURE_MINIMAL     0
#define FBINK_FEATURE_DRAW        (1 << 0)    // Basic draw primitives
#define FBINK_FEATURE_BITMAP      (1 << 1)    // Fixed-cell font rendering, plus the base IBM font
#define FBINK_FEATURE_FONTS       (1 << 2)    // The full set of fixed-cell fonts
#define FBINK_FEATURE_UNIFONT     (1 << 3)    // Unifont for the fixed-cell font rendering
#define FBINK_FEATURE_OPENTYPE    (1 << 4)    // TrueType/OpenType font rendering
#define FBINK_FEATURE_IMAGE       (1 << 5)    // Image support
#define FBINK_FEATURE_BUTTON_SCAN (1 << 6)    // Button scan support (Kobo only, deprecated)
#define FBINK_FEATURE_INPUT       (1 << 7)    // Input utilities (e.g., fbink_input_scan)
#define FBINK_FEATURE_FULL                                                                                               \
	(FBINK_FEATURE_DRAW | FBINK_FEATURE_BITMAP | FBINK_FEATURE_FONTS | FBINK_FEATURE_OPENTYPE |                      \
	 FBINK_FEATURE_IMAGE | FBINK_FEATURE_INPUT)

// List of available fonts
typedef enum
{
	IBM = 0U,                // font8x8
	UNSCII,                  // unscii-8
	UNSCII_ALT,              // unscii-8-alt
	UNSCII_THIN,             // unscii-8-thin
	UNSCII_FANTASY,          // unscii-8-fantasy
	UNSCII_MCR,              // unscii-8-mcr
	UNSCII_TALL,             // unscii-16
	BLOCK,                   // block
	LEGGIE,                  // leggie (regular)
	VEGGIE,                  // leggie EGA/VGA/FB
	KATES,                   // kates (nexus)
	FKP,                     // fkp
	CTRLD,                   // ctrld
	ORP,                     // orp (regular)
	ORPB,                    // orp (bold)
	ORPI,                    // orp (italic)
	SCIENTIFICA,             // scientifica (regular)
	SCIENTIFICAB,            // scientifica (bold)
	SCIENTIFICAI,            // scientifica (italic)
	TERMINUS,                // terminus (regular)
	TERMINUSB,               // terminus (bold)
	FATTY,                   // fatty
	SPLEEN,                  // spleen
	TEWI,                    // tewi (medium)
	TEWIB,                   // tewi (bold)
	TOPAZ,                   // Topaz+ A1200
	MICROKNIGHT,             // MicroKnight+
	VGA,                     // IBM VGA 8x16
	UNIFONT,                 // Unifont (single-wide glyphs only)
	UNIFONTDW,               // Unifont (double-wide glyphs only)
	COZETTE,                 // Cozette
	FONT_MAX = UINT8_MAX,    // uint8_t
} __attribute__((packed)) FONT_INDEX_E;
typedef uint8_t           FONT_INDEX_T;

// List of supported font styles
typedef enum
{
	FNT_REGULAR = 0U,
	FNT_ITALIC,
	FNT_BOLD,
	FNT_BOLD_ITALIC,
} FONT_STYLE_E;
// NOTE: This one is not packed to avoid breaking ABI w/ fbink_add_ot_font
typedef int FONT_STYLE_T;

// List of available halign/valign values
typedef enum
{
	NONE = 0U,                // i.e., LEFT for halign, TOP for valign
	CENTER,                   //
	EDGE,                     // i.e., RIGHT for halign, BOTTOM for valign
	ALIGN_MAX = UINT8_MAX,    // uint8_t
} __attribute__((packed)) ALIGN_INDEX_E;
typedef uint8_t           ALIGN_INDEX_T;

// List of available padding values
typedef enum
{
	NO_PADDING = 0U,
	HORI_PADDING,
	VERT_PADDING,
	FULL_PADDING,
	MAX_PADDING = UINT8_MAX,    // uint8_t
} __attribute__((packed)) PADDING_INDEX_E;
typedef uint8_t           PADDING_INDEX_T;

// List of available colors in the eInk color map
// NOTE: This is split in FG & BG to ensure that the default values lead to a sane result (i.e., black on white)
typedef enum
{
	FG_BLACK = 0U,         // 0x00
	FG_GRAY1,              // 0x11
	FG_GRAY2,              // 0x22
	FG_GRAY3,              // 0x33
	FG_GRAY4,              // 0x44
	FG_GRAY5,              // 0x55
	FG_GRAY6,              // 0x66
	FG_GRAY7,              // 0x77
	FG_GRAY8,              // 0x88
	FG_GRAY9,              // 0x99
	FG_GRAYA,              // 0xAA
	FG_GRAYB,              // 0xBB
	FG_GRAYC,              // 0xCC
	FG_GRAYD,              // 0xDD
	FG_GRAYE,              // 0xEE
	FG_WHITE,              // 0xFF
	FG_MAX = UINT8_MAX,    // uint8_t
} __attribute__((packed)) FG_COLOR_INDEX_E;
typedef uint8_t           FG_COLOR_INDEX_T;

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
	BG_BLACK,
	BG_MAX = UINT8_MAX,    // uint8_t
} __attribute__((packed)) BG_COLOR_INDEX_E;
typedef uint8_t           BG_COLOR_INDEX_T;

// List of Cervantes device IDs (HWConfig PCB index)
typedef enum
{
	DEVICE_CERVANTES_TOUCH      = 22U,
	DEVICE_CERVANTES_TOUCHLIGHT = 23U,
	DEVICE_CERVANTES_2013       = 33U,
	DEVICE_CERVANTES_3          = 51U,
	DEVICE_CERVANTES_4          = 68U,
	DEVICE_CERVANTES_MAX        = UINT16_MAX,    // uint16_t
} __attribute__((packed)) CERVANTES_DEVICE_ID_E;

// List of Kobo device IDs
typedef enum
{
	DEVICE_KOBO_TOUCH_A        = 300U,    // Not an actual Kobo ID, Nickel bundles it with the B
	DEVICE_KOBO_TOUCH_B        = 310U,
	DEVICE_KOBO_TOUCH_C        = 320U,
	DEVICE_KOBO_MINI           = 340U,
	DEVICE_KOBO_GLO            = 330U,
	DEVICE_KOBO_GLO_HD         = 371U,
	DEVICE_TOLINO_SHINE_2HD    = DEVICE_KOBO_GLO_HD + 300U,
	DEVICE_KOBO_TOUCH_2        = 372U,
	DEVICE_KOBO_AURA           = 360U,
	DEVICE_KOBO_AURA_HD        = 350U,
	DEVICE_KOBO_AURA_H2O       = 370U,
	DEVICE_KOBO_AURA_H2O_2     = 374U,
	DEVICE_KOBO_AURA_H2O_2_R2  = 378U,
	DEVICE_KOBO_AURA_ONE       = 373U,
	DEVICE_KOBO_AURA_ONE_LE    = 381U,
	DEVICE_KOBO_AURA_SE        = 375U,
	DEVICE_TOLINO_VISION       = DEVICE_KOBO_AURA_SE + 300U,
	DEVICE_KOBO_AURA_SE_R2     = 379U,
	DEVICE_KOBO_CLARA_HD       = 376U,
	DEVICE_TOLINO_SHINE_3      = DEVICE_KOBO_CLARA_HD + 300U,
	DEVICE_KOBO_FORMA          = 377U,
	DEVICE_TOLINO_EPOS_2       = DEVICE_KOBO_FORMA + 300U,
	DEVICE_KOBO_FORMA_32GB     = 380U,
	DEVICE_KOBO_LIBRA_H2O      = 384U,
	DEVICE_TOLINO_VISION_5     = DEVICE_KOBO_LIBRA_H2O + 300U,
	DEVICE_KOBO_NIA            = 382U,
	DEVICE_KOBO_ELIPSA         = 387U,
	DEVICE_KOBO_LIBRA_2        = 388U,
	DEVICE_KOBO_SAGE           = 383U,
	DEVICE_TOLINO_EPOS_3       = DEVICE_KOBO_SAGE + 300U,
	DEVICE_KOBO_CLARA_2E       = 386U,
	DEVICE_KOBO_ELIPSA_2E      = 389U,
	DEVICE_KOBO_LIBRA_COLOUR   = 390U,
	DEVICE_TOLINO_VISION_COLOR = DEVICE_KOBO_LIBRA_COLOUR + 300U,
	DEVICE_KOBO_CLARA_BW       = 391U,
	DEVICE_TOLINO_SHINE_BW     = DEVICE_KOBO_CLARA_BW + 300U,
	DEVICE_KOBO_CLARA_COLOUR   = 393U,
	DEVICE_TOLINO_SHINE_COLOR  = DEVICE_KOBO_CLARA_COLOUR + 300U,
	DEVICE_KOBO_CLARA_BW_TPV   = 395U,
	DEVICE_KOBO_MAX            = UINT16_MAX,    // uint16_t
} __attribute__((packed)) KOBO_DEVICE_ID_E;

// List of device IDs for mainline kernels
// c.f., https://github.com/NiLuJe/FBInk/issues/70#issuecomment-1242274710 for Tolinos
typedef enum
{
	DEVICE_MAINLINE_TOLINO_SHINE_2HD   = ('T' << 8U) | ('o' << 8U) | ('l' << 8U) | DEVICE_KOBO_GLO_HD,
	DEVICE_MAINLINE_TOLINO_SHINE_3     = ('T' << 8U) | ('o' << 8U) | ('l' << 8U) | DEVICE_KOBO_CLARA_HD,
	DEVICE_MAINLINE_TOLINO_VISION      = ('T' << 8U) | ('o' << 8U) | ('l' << 8U) | DEVICE_KOBO_AURA_SE,
	DEVICE_MAINLINE_TOLINO_VISION_5    = ('T' << 8U) | ('o' << 8U) | ('l' << 8U) | DEVICE_KOBO_LIBRA_H2O,
	DEVICE_MAINLINE_GENERIC_IMX5       = ('i' << 8U) | ('.' << 8U) | 'M' | 'X' | '5',
	DEVICE_MAINLINE_GENERIC_IMX6       = ('i' << 8U) | ('.' << 8U) | 'M' | 'X' | '6',
	DEVICE_MAINLINE_GENERIC_SUNXI_B300 = ('A' << 8U) | ('W' << 8U) | 'B' | '3' | '0' | '0',
	DEVICE_MAINLINE_MAX                = UINT16_MAX,    // uint16_t
} __attribute__((packed)) MAINLINE_DEVICE_ID_E;

// List of reMarkable device IDs
typedef enum
{
	DEVICE_REMARKABLE_1   = 1U,
	DEVICE_REMARKABLE_2   = 2U,
	DEVICE_REMARKABLE_MAX = UINT16_MAX,    // uint16_t
} __attribute__((packed)) REMARKABLE_DEVICE_ID_E;

// List of PocketBook device IDs
typedef enum
{
	DEVICE_POCKETBOOK_MINI            = 515U,
	DEVICE_POCKETBOOK_606             = 606U,
	DEVICE_POCKETBOOK_611             = 611U,
	DEVICE_POCKETBOOK_613             = 613U,
	DEVICE_POCKETBOOK_614             = 614U,
	DEVICE_POCKETBOOK_615             = 615U,
	DEVICE_POCKETBOOK_616             = 616U,
	DEVICE_POCKETBOOK_617             = 617U,
	DEVICE_POCKETBOOK_618             = 618U,
	DEVICE_POCKETBOOK_TOUCH           = 622U,
	DEVICE_POCKETBOOK_LUX             = 623U,
	DEVICE_POCKETBOOK_BASIC_TOUCH     = 624U,
	DEVICE_POCKETBOOK_BASIC_TOUCH_2   = 625U,
	DEVICE_POCKETBOOK_LUX_3           = 626U,
	DEVICE_POCKETBOOK_LUX_4           = 627U,
	DEVICE_POCKETBOOK_LUX_5           = 628U,
	DEVICE_POCKETBOOK_VERSE           = 629U,
	DEVICE_POCKETBOOK_SENSE           = 630U,
	DEVICE_POCKETBOOK_TOUCH_HD        = 631U,
	DEVICE_POCKETBOOK_TOUCH_HD_PLUS   = 632U,
	DEVICE_POCKETBOOK_COLOR           = 633U,
	DEVICE_POCKETBOOK_VERSE_PRO       = 634U,
	DEVICE_POCKETBOOK_VERSE_PRO_COLOR = 634U | 'K',
	DEVICE_POCKETBOOK_AQUA            = 640U,
	DEVICE_POCKETBOOK_AQUA2           = 641U,
	DEVICE_POCKETBOOK_ULTRA           = 650U,
	DEVICE_POCKETBOOK_ERA             = 700U,
	DEVICE_POCKETBOOK_ERA_COLOR       = 700U | 'K',
	DEVICE_POCKETBOOK_INKPAD_3        = 740U,
	DEVICE_POCKETBOOK_INKPAD_3_PRO    = 742U,
	DEVICE_POCKETBOOK_INKPAD_COLOR    = 741U,
	DEVICE_POCKETBOOK_INKPAD_COLOR_2  = 743U | 'C',
	DEVICE_POCKETBOOK_INKPAD_COLOR_3  = 743U | 'K',
	DEVICE_POCKETBOOK_INKPAD          = 840U,
	DEVICE_POCKETBOOK_INKPAD_X        = 1040U,
	DEVICE_POCKETBOOK_INKPAD_4        = 743U | 'G',
	DEVICE_POCKETBOOK_COLOR_LUX =
	    ('C' << 8U) | ('o' << 8U) | ('l' << 8U) | ('o' << 8U) | ('r' << 8U) | 'L' | 'u' | 'x',
	DEVICE_POCKETBOOK_INKPAD_LITE = 970U,
	DEVICE_POCKETBOOK_MAX         = UINT16_MAX,    // uint16_t
} __attribute__((packed)) POCKETBOOK_DEVICE_ID_E;

// If device detection failed...
#define DEVICE_UNKNOWN 0U

// NOTE: There's no enum for Kindles, because there are an insane number of device IDs per model,
//       so it doesn't really fit into this model. Use the deviceName instead.
typedef uint16_t DEVICE_ID_T;

// List of *potentially* available waveform modes.
// NOTE: On mxcfb EPDC v1 (as well as all Kindle) & most MTK devices, REAGL & REAGLD generally expect to *always* be flashing.
//       The same is also true for Kaleido-specific modes (GCC16 & GLRC16).
//       This is currently left at your own discretion, though.
//       c.f., https://github.com/NiLuJe/FBInk/commit/32acece78f7cc92b06faa4a668feead260b8ce24
//       See also the comments around the relevant refresh_* functions in fbink.c
// NOTE: On very old devices (e.g., Kobo Mk. 3 & 4; possibly early PB), only AUTO, DU & GC16 may be relied on.
//       GC4 will probably behave, but A2 & GL16 are not a given at all:
//       e.g., GL16 is actively broken on Kobo <= Mk. 4: c.f., https://github.com/baskerville/plato/issues/158#issuecomment-787520759.
//       If a waveform mode produces unexpected/broken results, and/or if you start to hit unexpected EPDC timeouts (or even an OOPS),
//       that's usually a strong hint that you're trying to use something you shouldn't ;).
// NOTE: See the various mxcfb headers in the eink folder for more details about what's available on your platform.
//       Platform-specific quirks, if any, are also commented upon in the relevant refresh_* functions in fbink.c
// NOTE: If you're curious about how to deal with all this stuff in practice for real world use-cases,
//       I'd recommend looking at my various comments in the KOReader backends for some more context,
//       c.f., https://github.com/koreader/koreader-base/blob/master/ffi/framebuffer_mxcfb.lua
typedef enum
{
	WFM_AUTO = 0U,    // Let the EPDC choose, via histogram analysis of the refresh region.
	//                   May *not* always (or ever) opt to use REAGL on devices where it is otherwise available.
	//                   This is the default.
	//                   If you request a flashing update w/ AUTO, FBInk automatically uses GC16 instead.
	//                   NOTE: On sunxi SoCs, this analysis is done on CPU, instead of by the PxP.
	//                         As such, it's going to be slower. Prefer explicitly choosing a mode instead.
	//                         (When in doubt, GL16 is usually a good middle ground).
	// Common
	WFM_DU,    // From any to B&W, fast (~260ms), some light ghosting.
	//            On-screen pixels will be left as-is for new content that is *not* B&W.
	//            Great for UI highlights, or tracing touch/pen input.
	//            Will never flash.
	//            DU stands for "Direct Update".
	WFM_GC16,    // From any to any, ~450ms, high fidelity (i.e., lowest risk of ghosting).
	//              Ideal for image content.
	//              If flashing, will flash and update the full region.
	//              If not, only changed pixels will update.
	//              GC stands for "Grayscale Clearing"
	WFM_GC4,    // From any to B/W/GRAYA/GRAY5, (~290ms), some ghosting. (may be implemented as DU4 on some devices).
	//             Will *probably* never flash, especially if the device doesn't implement any other 4 color modes.
	//             Limited use-cases in practice.
	WFM_A2,    // From B&W to B&W, fast (~120ms), some ghosting.
	//            On-screen pixels will be left as-is for new content that is *not* B&W.
	//            FBInk will ask the EPDC to enforce quantization to B&W to honor the "to" requirement,
	//            (via EPDC_FLAG_FORCE_MONOCHROME).
	//            Will never flash.
	//            Consider bracketing a series of A2 refreshes between white screens to transition in and out of A2,
	//            so as to honor the "from" requirement,
	//            (especially given that FORCE_MONOCHROME may not be reliably able to do so, c.f., refresh_kobo_mk7):
	//            non-flashing GC16 for the in transition, A2 or GC16 for the out transition.
	//            A stands for "Animation"
	WFM_GL16,    // From white to any, ~450ms, some ghosting.
	//              Typically optimized for text on a white background.
	// Newer generation devices only
	WFM_REAGL,    // From white to any, ~450ms, with ghosting and flashing reduction.
	//               When available, best option for text (in place of GL16).
	//               May enforce timing constraints if in collision with another waveform mode, e.g.,
	//               it may, to some extent, wait for completion of previous updates to have access to HW resources.
	//               Marketing term for the feature is "Regal". Technically called 5-bit waveform modes.
	WFM_REAGLD,    // From white to any, ~450ms, with more ghosting reduction, but less flashing reduction.
	//                Should only be used when flashing, which should yield a less noticeable flash than GC16.
	//                Rarely used in practice, because still optimized for text or lightly mixed content,
	//                not pure image content.
	// (Mostly) Kindle only
	WFM_GC16_FAST,    // Better latency at the expense of lower fidelity than GC16.
	WFM_GL16_FAST,    // Better latency at the expense of lower fidelity than GL16.
	WFM_DU4,          // From any to B/W/GRAYA/GRAY5. (e.g., GC4. Will never flash. Also available on Kobo Mk. 9).
	WFM_GL4,          // From white to B/W/GRAYA/GRAY5.
	WFM_GL16_INV,     // From black to any. Optimized for text on a black background (e.g., nightmode).
	// "Nightmode" waveform modes (dubbed "eclipse" in Kobo-land).
	// Only available on some devices (Zelda on Kindle, Mk. 8+ on Kobo).
	// If you need to check at runtime whether it's actually supported, on an i.MX board,
	// check if /sys/class/graphics/fb0/waveform_mode_gck16 exists ;).
	// Otherwise, refer to the hasEclipseWfm deviceQuirks.
	WFM_GCK16,    // From black to any. Goes hand-in-hand with GLKW16, should only be used when flashing.
	WFM_GLKW16,    // From black to any. Newer variant of GL16_INV. (On Kobo, Mk. 9, 11 & 12 only. It's GLK16 on sunxi).
	// For documentation purposes
	WFM_INIT,    // May flash several times to end up with a white screen, slow (~2000ms).
	WFM_UNKNOWN,
	// reMarkable only
	WFM_INIT2,
	// PocketBook only
	WFM_A2IN,
	WFM_A2OUT,
	WFM_GC16HQ,    // Only available on i.MX SoCs. Alias for REAGL, or REAGLD when flashing.
	WFM_GS16,      // Only available on B288 SoCs. Fidelity supposedly somewhere between GL16 and GC16.
	// Kobo Sunxi only
	WFM_GU16,    // GL16, but honoring the in-kernel DISP_EINK_SET_GC_CNT
	//WFM_GCK16,	// GC16, but for white-on-black.
	WFM_GLK16,    // GL16, but for white-on-black.
	WFM_CLEAR,    // GC16 local (NOTE: Appears to crash the EPDC... [Elipsa on FW 4.28.17826])
	WFM_GC4L,     // GC4 local (NOTE: Appears to crash the EPDC... [Elipsa on FW 4.28.17826])
	// Kobo Sunxi; MTK only
	WFM_GCC16,     // GCC16, for color image content on Kaleido panels.
	WFM_GLRC16,    // GLRC16, for color highlights on text on Kaleido panels.
	// Kindle MTK only
	WFM_GC16_PARTIAL,     // Internal use only, GC16 + PARTIAL
	WFM_GCK16_PARTIAL,    // Internal use only, GCK16 + PARTIAL
	WFM_DUNM,             // DU, but for white-on-black.
	WFM_P2SW,             // Internal use only, used by the swipe animation.
	// Kindle MTK + CFA only
	WFM_GCCK16,     // Nightmode GCC16
	WFM_GLRCK16,    // Nightmode GLRC16

	WFM_MAX = UINT8_MAX,    // uint8_t
} __attribute__((packed)) WFM_MODE_INDEX_E;
typedef uint8_t           WFM_MODE_INDEX_T;

// List of *potentially* available HW dithering modes
typedef enum
{
	HWD_PASSTHROUGH = 0U,
	HWD_FLOYD_STEINBERG,
	HWD_ATKINSON,
	HWD_ORDERED,    // Generally the only supported HW variant on EPDC v2
	HWD_QUANT_ONLY,
	HWD_LEGACY = UINT8_MAX,    // Use legacy EPDC v1 dithering instead (if available).
	//                        Note that it is *not* offloaded to the PxP, it's purely software, in-kernel.
	//                        Usually based on Atkinson's algo. The most useful one being the Y8->Y1 one,
	//                        which we request with A2/DU refreshes.
} __attribute__((packed)) HW_DITHER_INDEX_E;
typedef uint8_t           HW_DITHER_INDEX_T;

// List of *potentially* available CFA post-process modes
typedef enum
{
	CFA_DEFAULT = 0U,
	CFA_AIE_S4,
	CFA_AIE_S7,
	CFA_AIE_S9,
	CFA_G0,
	CFA_G1,
	CFA_G2,
	CFA_NTX,
	CFA_NTX_SF,
	CFA_SKIP,
	CFA_MAX = UINT8_MAX,
} __attribute__((packed)) CFA_MODE_INDEX_E;
typedef uint8_t           CFA_MODE_INDEX_T;

// List of NTX rotation quirk types (c.f., mxc_epdc_fb_check_var @ drivers/video/fbdev/mxc/mxc_epdc_v2_fb.c)...
typedef enum
{
	NTX_ROTA_STRAIGHT = 0U,    // No shenanigans (at least as far as ioctls are concerned)
	NTX_ROTA_ALL_INVERTED,     // Every rotation is inverted by the kernel
	NTX_ROTA_ODD_INVERTED,     // Only Landscape (odd) rotations are inverted by the kernel
	// NOTE: Everything below this line should be considered mostly deprecated, as those were attempts to piggyback on this flag
	//       to handle input translations. That was a mistake, we've since switched to dedicated flags for touch input quirks.
	//       As the comment above implies, this is only meant to deal with the kernel screwing with us when setting the fb rotation.
	NTX_ROTA_SANE,    // NTX_ROTA_STRAIGHT, and ntxBootRota is the native Portrait orientation.
	//                            Optionally, bonus points if that's actually UR, and the panel is natively mounted UR,
	//                            like on the Kobo Libra.
	//                            Triple whammy if the touch layer rotation matches!
	NTX_ROTA_SUNXI,    // The rotate flag is technically meaningless, but *may* be set by third-party code (we don't).
	NTX_ROTA_CW_TOUCH,     // No kernel shenanigans, and Touch panel mounted in the invert of the usual rotation.
	NTX_ROTA_CCW_TOUCH,    // No kernel shenanigans, and Touch panel mounted in the usual rotation.
	NTX_ROTA_MAX = UINT8_MAX,    // uint8_t
} __attribute__((packed)) NTX_ROTA_INDEX_E;
typedef uint8_t           NTX_ROTA_INDEX_T;

// Available states for fbink_sunxi_ntx_enforce_rota
typedef enum
{
	FORCE_ROTA_NOTSUP = INT8_MIN,    // For FBInkState on non-sunxi platforms
	FORCE_ROTA_CURRENT_ROTA =
	    -5,    // Honor the gyro if it matches the working buffer's rotation; match the wb otherwise (NOTE: Requires fbdamage)
	FORCE_ROTA_CURRENT_LAYOUT =
	    -4,    // Honor the gyro if it matches the working buffer's layout; match the wb otherwise (NOTE: Requires fbdamage)
	FORCE_ROTA_PORTRAIT  = -3,          // Honor the gyro if it matches a Portrait layout
	FORCE_ROTA_LANDSCAPE = -2,          // Honor the gyro if it matches a Landscape layout
	FORCE_ROTA_GYRO      = -1,          // Honor the gyro (NOTE: default)
	FORCE_ROTA_UR        = 0,           // FB_ROTATE_UR
	FORCE_ROTA_CW        = 1,           // FB_ROTATE_CW
	FORCE_ROTA_UD        = 2,           // FB_ROTATE_UD
	FORCE_ROTA_CCW       = 3,           // FB_ROTATE_CCW
	FORCE_ROTA_WORKBUF   = 4,           // Match the working buffer's rotation (NOTE: Requires fbdamage)
	FORCE_ROTA_MAX       = INT8_MAX,    // int8_t
} __attribute__((packed)) SUNXI_FORCE_ROTA_INDEX_E;
typedef int8_t            SUNXI_FORCE_ROTA_INDEX_T;

// List of swipe directions for fbink_mtk_set_swipe_data
typedef enum
{
	MTK_SWIPE_DIR_DOWN  = 0,
	MTK_SWIPE_DIR_UP    = 1,
	MTK_SWIPE_DIR_LEFT  = 2,
	MTK_SWIPE_DIR_RIGHT = 3,
	MTK_SWIPE_DIR_MAX   = UINT8_MAX,    // uint8_t
} __attribute__((packed)) MTK_SWIPE_DIRECTION_INDEX_E;
typedef uint8_t           MTK_SWIPE_DIRECTION_INDEX_T;

// List of halftone pattern modes for fbink_mtk_set_halftone
typedef enum
{
	MTK_HALFTONE_DISABLED             = 0,
	MTK_HALFTONE_DEFAULT_CHECKER_SIZE = 1,
	MTK_HALFTONE_MAX_CHECKER_SIZE     = INT32_MAX,    // int
} __attribute__((packed)) MTK_HALFTONE_MODE_INDEX_E;
typedef int32_t           MTK_HALFTONE_MODE_INDEX_T;

// List of supported pixel formats
typedef enum
{
	FBINK_PXFMT_UNKNOWN = 0,
	FBINK_PXFMT_Y4      = 4,
	FBINK_PXFMT_Y8      = 8,
	FBINK_PXFMT_BGR565  = 16,
	FBINK_PXFMT_RGB565,
	FBINK_PXFMT_BGR24 = 24,
	FBINK_PXFMT_RGB24,
	FBINK_PXFMT_BGRA = 32,
	FBINK_PXFMT_RGBA,
	FBINK_PXFMT_BGR32,
	FBINK_PXFMT_RGB32,
	FBINK_PXFMT_MAX = UINT8_MAX,    // uint8_t
} __attribute__((packed)) FBINK_PXFMT_INDEX_E;
typedef uint8_t           FBINK_PXFMT_INDEX_T;

//
// A struct to dump FBInk's internal state into, like fbink_state_dump() would, but in C ;)
typedef struct
{
	long int user_hz;                  // USER_HZ (should pretty much always be 100)
	const char* restrict font_name;    // fbink_cfg->fontname (c.f., fontname_to_string())
	uint32_t    view_width;            // viewWidth (MAY be different than screen_width on devices with a viewport)
	uint32_t    view_height;           // viewHeight (ditto)
	uint32_t    screen_width;     // screenWidth (Effective width, c.f., is_ntx_quirky_landscape & initialize_fbink())
	uint32_t    screen_height;    // screenHeight (ditto)
	uint32_t    scanline_stride;        // fInfo.line_length (scanline length in bytes, padding included)
	uint32_t    bpp;                    // vInfo.bits_per_pixel
	bool        inverted_grayscale;     // true if vInfo.grayscale is set to GRAYSCALE_8BIT_INVERTED (@ 8bpp)
	char        device_name[32];        // deviceQuirks.deviceName (short common name, no brand)
	char        device_codename[32];    // deviceQuirks.deviceCodename
	char        device_platform[32];    // deviceQuirks.devicePlatform (often a codename, too)
	DEVICE_ID_T device_id;              // deviceQuirks.deviceId (decimal value, c.f., identify_device() on Kindle!)
	uint8_t     pen_fg_color;           // penFGColor (Actual grayscale value, not FG_COLOR_INDEX_E)
	uint8_t     pen_bg_color;           // penBGColor (ditto)
	unsigned short int screen_dpi;      // deviceQuirks.screenDPI
	unsigned short int font_w;          // FONTW (effective width of a glyph cell, i.e. scaled)
	unsigned short int font_h;          // FONTH (effective height of a glyph cell, i.e. scaled)
	unsigned short int max_cols;        // MAXCOLS (at current cell size)
	unsigned short int max_rows;        // MAXROWS (ditto)
	uint8_t view_hori_origin;           // viewHoriOrigin (would be non-zero on devices with a horizontal viewport)
	uint8_t view_vert_origin;           // viewVertOrigin (origin in px of row 0, includes viewport + viewVertOffset)
	uint8_t view_vert_offset;      // viewVertOffset (shift in px needed to vertically balance rows over viewHeight)
	uint8_t fontsize_mult;         // FONTSIZE_MULT (current cell scaling multiplier)
	uint8_t glyph_width;           // glyphWidth (native width of a glyph cell, i.e. unscaled)
	uint8_t glyph_height;          // glyphHeight (native height of a glyph cell, i.e. unscaled)
	bool    is_perfect_fit;        // deviceQuirks.isPerfectFit (horizontal column balance is perfect over viewWidth)
	bool    is_mtk;                // deviceQuirks.isMTK (device is running on a MediaTek SoC)
	bool    is_sunxi;              // deviceQuirks.isSunxi (device is running on an AllWinner SoC)
	bool    sunxi_has_fbdamage;    // sunxiCtx.has_fbdamage (true when fbdamage module is loaded)
	SUNXI_FORCE_ROTA_INDEX_T sunxi_force_rota;    // sunxiCtx.force_rota (current effective value)
	bool is_kindle_legacy;    // deviceQuirks.isKindleLegacy (device is a Kindle using the original einkfb EPDC API)
	bool is_kobo_non_mt;      // deviceQuirks.isKoboNonMT (device is a Kobo with no MultiTouch input support)
	bool unreliable_wait_for;           // deviceQuirks.unreliableWaitFor (MXCFB_WAIT_FOR_UPDATE_COMPLETE may timeout)
	bool can_wake_epdc;                 // deviceQuirks.canWakeEPDC (i.e., fbink_wakeup_epdc is *NOT* a NOP)
	uint8_t          ntx_boot_rota;     // deviceQuirks.ntxBootRota (Native rotation at boot)
	NTX_ROTA_INDEX_T ntx_rota_quirk;    // deviceQuirks.ntxRotaQuirk (c.f., tools/rota_map.c)
	uint8_t          rotation_map[4];    // deviceQuirks.rotationMap (index is native, value is canonical)
	bool touch_swap_axes;    // deviceQuirks.touchSwapAxes (panel reports swapped coordinates axes; handle this first)
	bool touch_mirror_x;     // deviceQuirks.touchMirrorX (post-swap, panel reports inverted x coordinates)
	bool touch_mirror_y;     // deviceQuirks.touchMirrorY (post-swap, panel reports inverted y coordinates)
	bool is_ntx_quirky_landscape;    // deviceQuirks.isNTX16bLandscape (rotation compensation is in effect)
	uint8_t current_rota;       // vInfo.rotate (current native rotation, <linux/fb.h>; Device rotation, not buffer!)
	bool    can_rotate;         // deviceQuirks.canRotate (device has a gyro)
	bool    can_hw_invert;      // deviceQuirks.canHWInvert (device can use EPDC inversion)
	bool    has_eclipse_wfm;    // deviceQuirks.hasEclipseWfm (device can use nightmode waveform modes)
	bool    has_color_panel;    // deviceQuirks.hasColorPanel (device features a Kaleido/CFA color panel)
	FBINK_PXFMT_INDEX_T pixel_format;    // deviceQuirks.pixelFormat
	bool can_wait_for_submission;    // deviceQuirks.canWaitForSubmission (devices supports fbink_wait_for_submission)
} FBInkState;

// What a FBInk config should look like. Perfectly sane when fully zero-initialized.
typedef struct
{
	short int        row;         // y axis (i.e., line), counts down from the bottom of the screen if negative
	short int        col;         // x axis (i.e., column), counts down from the right edge of the screen if negative
	uint8_t          fontmult;    // Font scaling multiplier (i.e., 4 -> x4), 0 means automatic.
	FONT_INDEX_T     fontname;    // Request a specific bundled font
	bool             is_inverted;    // Invert colors when *drawing* stuff (e.g., text, clear & images).
	//				This is *NOT* mutually exclusive with is_nightmode, and is *always* supported.
	bool             is_flashing;    // Request a black flash on refresh (e.g., UPDATE_MODE_FULL instead of PARTIAL)
	bool             is_cleared;     // Clear the full screen beforehand (honors bg_color & is_inverted)
	bool             is_centered;    // Center the text (horizontally)
	short int        hoffset;        // Horizontal offset (in pixels) for text position
	short int        voffset;        // Vertical offset (in pixels) for text position
	bool             is_halfway;     // Vertically center the text, honoring row offsets
	bool             is_padded;      // Pad the text with blanks (on the left, or on both sides if is_centered)
	bool             is_rpadded;     // Right pad the text with blanks
	FG_COLOR_INDEX_T fg_color;       // Requested foreground color for text (palette index)
	BG_COLOR_INDEX_T bg_color;       // Requested background color for text (palette index)
	bool             is_overlay;     // Don't draw bg and use inverse of fb's underlying pixel as pen fg color
	bool             is_bgless;      // Don't draw bg (mutually exclusive with is_overlay, which will take precedence)
	bool             is_fgless;      // Don't draw fg (takes precendence over is_overlay/is_bgless)
	bool          no_viewport;     // Ignore viewport corrections, whether hardware-related on Kobo, or to center rows
	bool          is_verbose;      // Print verbose diagnostic informations on stdout
	bool          is_quiet;        // Hide fbink_init()'s hardware setup info (sent to stderr)
	bool          ignore_alpha;    // Ignore any potential alpha channel in source image (i.e., flatten the image)
	ALIGN_INDEX_T halign;          // Horizontal alignment of images/dumps
	ALIGN_INDEX_T valign;          // Vertical alignment of images/dumps
	short int     scaled_width;    // Output width of images/dumps (0 for no scaling, -1 for viewport width)
	short int     scaled_height;    // Output height of images/dumps (0 for no scaling, -1 for viewport height)
	//				    If only *one* of them is left at 0, the image's aspect ratio will be honored.
	//				    If *either* of them is set to < -1, fit to screen while respecting AR.
	//				    NOTE: Scaling is inherently costly. I highly recommend not relying on it,
	//				          preferring instead proper preprocessing of your input images,
	//				          c.f., https://www.mobileread.com/forums/showpost.php?p=3728291&postcount=17
	WFM_MODE_INDEX_T  wfm_mode;          // Request a specific waveform mode (defaults to AUTO)
	HW_DITHER_INDEX_T dithering_mode;    // Request a specific dithering mode (defaults to PASSTHROUGH)
	bool              sw_dithering;      // Request (ordered) *software* dithering when printing an image.
	//                                      This is *NOT* mutually exclusive with dithering_mode!
	CFA_MODE_INDEX_T  cfa_mode;    // Request a specific CFA post-process mode (defaults to NONE, relevant wfm only).
	bool    is_nightmode;          // Request hardware inversion (via EPDC_FLAG_ENABLE_INVERSION, if supported/safe).
	//			 This is *NOT* mutually exclusive with is_inverted!
	//			 NOTE: If the HW doesn't support inversion, a warning is printed during init.
	//			       If you're convinced this is in error (i.e., up to date kernel),
	//			       you can bypass that check by setting FBINK_ALLOW_HW_INVERT in your env.
	bool    no_refresh;          // Skip actually refreshing the eInk screen (useful when drawing in batches)
	bool    no_merge;            // Set the EINK_NO_MERGE flag (Kobo sunxi only)
	bool    is_animated;         // Enable refresh animation, following fbink_mtk_set_swipe_data (Kindle MTK only)
	uint8_t saturation_boost;    // Boost image saturation, in %. Useful on Kaleido panels. Only affects 32bpp.
	bool    to_syslog;           // Send messages & errors to the syslog instead of stdout/stderr
} FBInkConfig;

// Same, but for OT/TTF specific stuff. MUST be zero-initialized.
typedef struct
{
	void* font;    // NOTE: This is essentially a pointer to a local FBInkOTFonts instance,
	//                      in order to use a set of fonts specific to an FBInkOTConfig,
	//                      via fbink_add_ot_font_v2() & fbink_free_ot_fonts_v2().
	//                      Consider it *private*: it needs to be NULL on init to be sane, but after that,
	//                      it's only used & memory managed by FBInk itself (via the aforemented _v2 API), not the user.
	struct
	{
		short int top;       // Top margin in pixels (if negative, counts backwards from the bottom edge)
		short int bottom;    // Bottom margin in pixels (supports negative values, too)
		short int left;      // Left margin in pixels (if negative, counts backwards from the right edge)
		short int right;     // Right margin in pixels (supports negative values, too)
	} margins;    // Margins are to the top-left edge of the bounding box, e.g., assuming no alignment/centering,
		      // zero margins will lead to a bounding box flush with the top-left corner of the screen.
	FONT_STYLE_T       style;          // Default font style to use when !is_formatted (defaults to Regular)
	float              size_pt;        // Size of text in points. If not set (0.0f), defaults to 12pt
	unsigned short int size_px;        // Size of text in pixels. Optional, but takes precedence over size_pt.
	bool               is_centered;    // Horizontal centering
	PADDING_INDEX_T    padding;        // Pad the drawing area (i.e., paint it in the background color).
	//                                    Unlike in the fixed-cell codepath, this always applies to both sides (L&R/T&B),
	//                                    no matter the chosen axis.
	//                                    e.g., HORI_PADDING is useful to prevent overlaps when drawing
	//                                    consecutive strings on the same line(s).
	bool               is_formatted;    // Is string "formatted"? Bold/Italic support only, markdown like syntax
	bool               compute_only;    // Abort early after the line-break computation pass (no actual rendering).
	//                       NOTE: This is early enough that it will *NOT* be able to predict *every*
	//                             potential case of truncation.
	//                             In particular, broken metrics may yield a late truncation at rendering time.
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
	struct
	{
		unsigned short int width;
		unsigned short int height;
	} bbox;            // Bounding box of the string (at computation time, padding excluded).
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
	size_t    stride;
	size_t    size;
	FBInkRect area;
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

// Returns the target platform of the currently loaded FBInk library.
// c.f., FBINK_TARGET_E enum
FBINK_API FBINK_TARGET_T fbink_target(void) __attribute__((const));

// Returns a bitmask of the features available in the currently loaded FBInk library.
// c.f., FBINK_FEATURE_ defines
FBINK_API uint32_t fbink_features(void) __attribute__((const));

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
// MUST be called at least *once* before any fbink_print*, fbink_dump/restore, fbink_cls or fbink_grid* functions.
// CAN safely be called multiple times,
//     but doing so is only necessary if the framebuffer's state has changed (although fbink_reinit is preferred in this case),
//     or if you modified one of the FBInkConfig fields that affects its results (listed below).
// Returns -(ENOSYS) if the device is unsupported (NOTE: Only on reMarkable!)
// fbfd:		Open file descriptor to the framebuffer character device,
//				if set to FBFD_AUTO, the fb is opened for the duration of this call.
// fbink_cfg:		Pointer to an FBInkConfig struct.
//				If you wish to customize them, the fields:
//				is_centered, fontmult, fontname, fg_color, bg_color,
//				no_viewport, is_verbose, is_quiet & to_syslog
//				MUST be set beforehand.
//				This means you MUST call fbink_init() again when you update them, too!
//				(This also means the effects from those fields "stick" across the lifetime of your application,
//				or until a subsequent fbink_init() (or effective fbink_reinit()) call gets fed different values).
//				NOTE: For fg_color & bg_color, see fbink_update_pen_colors().
//				NOTE: For is_verbose, is_quiet & to_syslog, see fbink_update_verbosity().
// NOTE: By virtue of, well, setting global variables, do NOT consider this thread-safe.
//       The rest of the API should be, though, so make sure you init in your main thread *before* threading begins...
// NOTE: If you just need to make sure the framebuffer state is still up to date before an fbink_* call,
//       (e.g., because you're running on a Kobo, which may switch from 16bpp to 32bpp, or simply change orientation),
//       prefer using fbink_reinit instead of calling fbink_init *again*, as it's tailored for this use case.
//       c.f., KFMon for an example of this use case in the wild.
// NOTE: You can perfectly well keep a few different FBInkConfig structs around, instead of modifying the same one over and over.
//       Just remember that some fields *require* an fbink_init() call to be taken into account (see above),
//       but if the only fields that differ don't fall into that category, you do *NOT* need an fbink_init() per FBInkConfig...
FBINK_API int fbink_init(int fbfd, const FBInkConfig* restrict fbink_cfg) __attribute__((nonnull));

//
// Dump a few of our internal state variables to stdout, in a format easily consumable by a shell (i.e., eval).
FBINK_API void fbink_state_dump(const FBInkConfig* restrict fbink_cfg) __attribute__((nonnull));

// Dump a few of our internal state variables to the FBInkState struct pointed to by fbink_state.
// NOTE: This includes quite a few useful things related to device identification, c.f., the FBInkState struct ;).
//       You can also peek at the output of fbink -e to get a hint of what the data actually looks like.
FBINK_API void fbink_get_state(const FBInkConfig* restrict fbink_cfg, FBInkState* restrict fbink_state)
    __attribute__((nonnull));

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
// Returns -(ENOSYS) when fixed-cell font support is disabled (MINIMAL build w/o BITMAP).
// fbfd:		Open file descriptor to the framebuffer character device,
//				if set to FBFD_AUTO, the fb is opened & mmap'ed for the duration of this call.
// string:		UTF-8 encoded string to print.
// fbink_cfg:		Pointer to an FBInkConfig struct.
//				Honors every field not specifically related to image/dump support.
FBINK_API int fbink_print(int fbfd, const char* restrict string, const FBInkConfig* restrict fbink_cfg)
    __attribute__((nonnull));

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
FBINK_API int fbink_add_ot_font(const char* filename, FONT_STYLE_T style) __attribute__((nonnull));
// Same API and behavior, except that the set of fonts being loaded is tied to this specific FBInkOTConfig instance,
// instead of being global.
// In which case, resources MUST be released via fbink_free_ot_fonts_v2()!
// NOTE: You can mix & match the v2 and legacy API, but for every fbink_add_ot_font() there must be an fbink_free_ot_fonts(),
//       and for every fbink_add_ot_font_v2(), there must be an fbink_free_ot_fonts_v2()
//       (for each matching FBInkOTConfig instance).
FBINK_API int fbink_add_ot_font_v2(const char* filename, FONT_STYLE_T style, FBInkOTConfig* restrict cfg)
    __attribute__((nonnull));

// Free all loaded OpenType fonts. You MUST call this when you have finished all OT printing.
// NOTE: Safe to call even if no fonts were actually loaded.
FBINK_API int fbink_free_ot_fonts(void);
// Same, but for a specific FBInkOTConfig instance if fbink_add_ot_font_v2 was used.
// NOTE: Safe to call even if no fonts were actually loaded, in which case it'll return -(EINVAL)!
FBINK_API int fbink_free_ot_fonts_v2(FBInkOTConfig* restrict cfg) __attribute__((nonnull));

// Print a string using an OpenType font.
// NOTE: The caller MUST have loaded at least one font via fbink_add_ot_font() FIRST.
// This function uses margins (in pixels) instead of rows/columns for positioning and setting the printable area.
// Returns a new top margin for use in subsequent calls, if the return value is positive.
// NOTE: A zero return value indicates there is no room left to print another row of text at the current margins or font size.
// Returns -(ERANGE) if the provided margins are out of range, or sum to < view height or width.
// Returns -(ENOSYS) when OT support is disabled (MINIMAL build w/o OPENTYPE).
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
//				wfm_mode, dithering_mode, is_nightmode, no_refresh will be honored.
//				Pass a NULL pointer if unneeded.
// fit:			Optional pointer to an FBInkOTFit struct.
//				If set, it will be used to return information about the amount of lines needed to render
//				the string at the requested font size, and whether it was truncated or not.
//				Pass a NULL pointer if unneeded.
// NOTE: Alignment is relative to the printable area, as defined by the margins.
//       As such, it only makes sense in the context of a single, specific print call.
FBINK_API int fbink_print_ot(int fbfd,
			     const char* restrict string,
			     const FBInkOTConfig* restrict cfg,
			     const FBInkConfig* restrict fbink_cfg,
			     FBInkOTFit* restrict fit) __attribute__((nonnull(2)));

//
// Brings printf formatting to fbink_print and fbink_print_ot ;).
// fbfd:		Open file descriptor to the framebuffer character device,
//				if set to FBFD_AUTO, the fb is opened & mmap'ed for the duration of this call.
// ot_cfg:		Optional pointer to an FBInkOTConfig struct.
// fbink_cfg:		Optional pointer to an FBInkConfig struct.
// ot_fit:		Optional pointer to an FBInkOTFit struct.
// NOTE: If ot_cfg is NULL, will call fbink_print, otherwise, fbink_print_ot!
//       If ot_cfg is valid, fbink_cfg MAY be NULL (same behavior as fbink_print_ot).
//       If ot_cfg is NULL, fbink_cfg MUST be valid.
// NOTE: Meaning you MUST at least pass either an fbink_cfg or an ot_cfg pointer!
// NOTE: ot_fit behaves like in fbink_print_ot (i.e., it's only relevant if you pass an ot_cfg pointer, but it may be NULL).
FBINK_API int fbink_printf(int fbfd,
			   const FBInkOTConfig* restrict ot_cfg,
			   const FBInkConfig* restrict fbink_cfg,
			   FBInkOTFit* restrict ot_fit,
			   const char* fmt,
			   ...) __attribute__((format(printf, 5, 6)));

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
// fbink_cfg:		Pointer to an FBInkConfig struct. Honors wfm_mode, dithering_mode, is_nightmode, is_flashing.
// NOTE: If you request an empty region (0x0 @ (0, 0), a full-screen refresh will be performed!
// NOTE: This *ignores* no_refresh ;).
// NOTE: As far as dithering is concerned, c.f., HW_DITHER_INDEX_E enum.
//	 True HW dithering is only supported on devices with a recent EPDC (>= v2)!
//	 On Kindle, that's everything since the KOA2 (KOA2, PW4, KT4, KOA3),
//	 On Kobo, that's everything since Mk.7.
// NOTE: Even then, your device may not actually support anything other than PASSTHROUGH & ORDERED!
//	 On slightly older devices, the EPDC may support some sort of in-kernel software dithering, hence HWD_LEGACY.
// NOTE: If you do NOT want to request any dithering, set FBInkConfig's dithering_mode field to HWD_PASSTHROUGH (i.e., 0).
//       This is also the fallback value.
// NOTE: On Kobo devices with a sunxi SoC, you will not be able to refresh content that you haven't drawn yourself first.
//       (There's no "shared" framebuffer, each process gets its own private, zero-initialized (i.e., solid black) buffer).
// NOTE: In case of ioctl failure, errno *should* be preserved,
//       allowing the caller to possibly handle some very specific edge-cases.
// NOTE: The confusing coordinates order is inherited from the layout of an mxcfb_rect struct.
//       See fbink_refresh_rect below for a variant that takes an FBInkRect instead.
FBINK_API int fbink_refresh(int      fbfd,
			    uint32_t region_top,
			    uint32_t region_left,
			    uint32_t region_width,
			    uint32_t region_height,
			    const FBInkConfig* restrict fbink_cfg) __attribute__((nonnull));

//
// Variant of fbink_refresh that takes an FBInkRect instead of broken out coordinates.
// Completely identical behavior.
FBINK_API int fbink_refresh_rect(int fbfd, const FBInkRect* restrict rect, const FBInkConfig* restrict fbink_cfg)
    __attribute__((nonnull));

// A simple wrapper around the MXCFB_WAIT_FOR_UPDATE_SUBMISSION ioctl, without requiring you to include mxcfb headers.
// Returns -(EINVAL) when the update marker is invalid.
// Returns -(ENOSYS) on devices where this ioctl is unsupported.
// NOTE: It is only implemented by Kindle kernels (K5+), and Kobo kernels for MTK SoCs (Mk. 11)!
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
// NOTE: Beware that on a few NTX boards, this ioctl appears to be buggy, and *may* randomly timeout.
//       c.f., devices flagged as deviceQuirks.unreliableWaitFor in fbink_device_id.c

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
//       or produce unwanted results (e.g., rotation).
// NOTE: Right now, this only checks for the isNTX16bLandscape device quirk,
//       because that's the only one that is not permanent (i.e., hardware specific),
//       but instead software specific (here, because of pickel).
//       In practical terms, this means the Kobo's fb is in 16bpp mode, with its origin in the top-right corner (i.e., Landscape).
// NOTE: Deprecated in favor of fbink_reinit ;).
FBINK_API bool fbink_is_fb_quirky(void) __attribute__((pure, deprecated));

// We'll need those for fbink_reinit (start > 256 to stay clear of errno values)
#define OK_BPP_CHANGE       (1 << 9)
#define OK_ROTA_CHANGE      (1 << 10)
#define OK_LAYOUT_CHANGE    (1 << 11)
#define OK_GRAYSCALE_CHANGE (1 << 12)
// Attempt to detect changes in framebuffer states (between this call and the last time fbink_init/fbink_reinit was called),
// doing a reinit (i.e., calling fbink_init again) if needed, while doing the least amount of work possible in the process.
// NOTE: The intended use-case is for long running apps which may trigger prints across different framebuffer states,
//       to allow them to ensure they'll be using up-to-date init data at key points in their lifecycle
//       (without needing to bruteforce a full reinit on every print).
//       This is of interest on a few devices, where trying to print based on a "stale" init state would at worst fail,
//       at best produce unwanted results (e.g., after a bitdepth change or a hw rotation).
// NOTE: This obviously supercedes fbink_is_fb_quirky, because it should be smarter,
//       by catching more scenarios where a reinit would be useful,
//       and it can avoid running the same ioctl twice when an ioctl already done by init is needed to detect a state change.
// NOTE: Using fbink_reinit does NOT lift the requirement of having to run fbink_init at least ONCE,
//       i.e., you cannot replace the initial fbink_init call by fbink_reinit!
// If reinitialization was *successful*, returns a bitmask with one or more of these flags set:
// bit OK_BPP_CHANGE is set if there was a bitdepth change.
// bit OK_ROTA_CHANGE is set if there was a rotation change.
// bit OK_LAYOUT_CHANGE is set if a rotation change caused a layout change (i.e., an orientation swap, Portrait <-> Landscape),
//     this obviously implies OK_ROTA_CHANGE.
//     If *only* OK_ROTA_CHANGE is set, it means the rotation change was a simple inversion of the current orientation,
//     (i.e., Portrait <-> Inverted Portrait or Landscape <-> Inverted Landscape).
// bit OK_GRAYSCALE_CHANGE is set if there was a grayscale flag change.
//     This is only set if the current & last known bitdepth is 8bpp.
//     On mxcfb-like platforms, this flag is used by the epdc driver to toggle global HW inversion (a.k.a., night mode).
// NOTE: This means that it may return a *positive* non-zero value on *success*.
//       This is helpful for callers that need to track FBInk's internal state via fbink_get_state or fbink_get_fb_pointer,
//       because a reinit *might* affect the screen layout, signaling that their current state copy *may* be stale.
//       TL;DR: Assume that *any* OK_*_CHANGE return value means that you need to refresh your state tracking.
// NOTE: You'll probably want to take action (changing pen colors or enabling inversion) after an OK_GRAYSCALE_CHANGE,
//       especially if it's unexpected.
// NOTE: In turn, this means that a simple EXIT_SUCCESS means that no reinitialization was needed.
// NOTE: On Kobo devices with a sunxi SoC, OK_BPP_CHANGE will *never* happen,
//       as the state of the actual framebuffer device is (unfortunately) meaningless there.
// fbfd:		Open file descriptor to the framebuffer character device,
//				if set to FBFD_AUTO, the fb is opened for the duration of this call.
// fbink_cfg:		Pointer to an FBInkConfig struct.
FBINK_API int fbink_reinit(int fbfd, const FBInkConfig* restrict fbink_cfg) __attribute__((warn_unused_result, nonnull));

// Update FBInk's internal verbosity flags
// As mentioned in fbink_init(), the is_verbose, is_quiet & to_syslog fields in an FBInkConfig
// are only processed at initialization time.
// This function allows doing *just* that, without having to go through a more costly full (re)-init.
// fbink_cfg:		Pointer to an FBInkConfig struct (is_verbose, is_quiet & to_syslog).
FBINK_API void fbink_update_verbosity(const FBInkConfig* restrict fbink_cfg) __attribute__((nonnull));

// Update FBInk's internal representation of pen colors
// As mentioned in fbink_init(), the fg_color & bg_color fields in an FBInkConfig are only processed at initialization time.
// This is because they're not used as-is (because they're not actually colors, just a custom palette index),
// they're just used to pack the matching palette color value into the right pixel format for the target framebuffer.
// This function allows doing *just* that, without having to go through a more costly full (re)-init.
// Returns -(ENOSYS) when drawing primitives are disabled (MINIMAL build w/o DRAW).
// fbink_cfg:		Pointer to an FBInkConfig struct (honors fg_color & bg_color).
FBINK_API int fbink_update_pen_colors(const FBInkConfig* restrict fbink_cfg) __attribute__((nonnull));

// We'll need those for fbink_set_*_pen_* (start > 256 to stay clear of errno values)
#define OK_ALREADY_SAME (1 << 9)
// Alternatively, you can choose to set the pen colors *directly*, without relying on FBInk's eInk palette handling.
// This is mostly of interest if you want to use color values you're getting from somewhere outside FBInk.
// You will *NOT* have to call fbink_update_pen_colors() when using these, they'll take care of updating the internal state.
// NOTE: The *optional* quantization pass *should* match what the EPDC itself will do anyway (i.e., it's redundant).
// Returns -(ENOSYS) when drawing primitives are disabled (MINIMAL build w/o DRAW).
// y:			8-bit luminance value
// quantize:		If true, round to the nearest eInk palette color.
// update:		If true, abort early and return OK_ALREADY_SAME if that's already the current color.
FBINK_API int fbink_set_fg_pen_gray(uint8_t y, bool quantize, bool update);
FBINK_API int fbink_set_bg_pen_gray(uint8_t y, bool quantize, bool update);
// NOTE: You should be aware that subsequent fbink_init calls (and fbink_reinit when it leads to a reinit) *will*
//       reset the pen colors to a grayscale representation of whatever RGB values you set this way!
// r:			8-bit red component value
// g:			8-bit green component value
// b:			8-bit blue component value
// a:			8-bit alpha component value (opaque is 0xFFu).
// quantize:		If true, round to the nearest eInk palette color. This implies a grayscaling pass!
// update:		If true, abort early and return OK_ALREADY_SAME if that's already the current color.
//			Keep in mind that the comparison is done *after* grayscaling, even without quantize set.
FBINK_API int fbink_set_fg_pen_rgba(uint8_t r, uint8_t g, uint8_t b, uint8_t a, bool quantize, bool update);
FBINK_API int fbink_set_bg_pen_rgba(uint8_t r, uint8_t g, uint8_t b, uint8_t a, bool quantize, bool update);

//
// Print a full-width progress bar on screen.
// Returns -(ENOSYS) when fixed-cell font support is disabled (MINIMAL build w/o BITMAP).
// fbfd:		Open file descriptor to the framebuffer character device,
//				if set to FBFD_AUTO, the fb is opened & mmap'ed for the duration of this call.
// percentage:		0-100 value to set the progress bar's progression.
// fbink_cfg:		Pointer to an FBInkConfig struct (ignores is_overlay, col & hoffset;
//				as well as is_centered & is_padded).
FBINK_API int fbink_print_progress_bar(int fbfd, uint8_t percentage, const FBInkConfig* restrict fbink_cfg)
    __attribute__((nonnull));

// Print a full-width activity bar on screen (i.e., an infinite progress bar).
// Returns -(ENOSYS) when fixed-cell font support is disabled (MINIMAL build w/o BITMAP).
// fbfd:		Open file descriptor to the framebuffer character device,
//				if set to FBFD_AUTO, the fb is opened & mmap'ed for the duration of this call.
// progress:		0-16 value to set the progress thumb's position in the bar.
// fbink_cfg:		Pointer to an FBInkConfig struct (ignores is_overlay, is_fgless, col & hoffset;
//				as well as is_centered & is_padded).
FBINK_API int fbink_print_activity_bar(int fbfd, uint8_t progress, const FBInkConfig* restrict fbink_cfg)
    __attribute__((nonnull));

//
// Print an image on screen.
// Returns -(ENOSYS) when image support is disabled (MINIMAL build w/o IMAGE).
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
//       If you can't pre-process your images, dithering can be handled by the hardware on recent devices (c.f. dithering_mode),
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
FBINK_API int fbink_print_image(int         fbfd,
				const char* filename,
				short int   x_off,
				short int   y_off,
				const FBInkConfig* restrict fbink_cfg) __attribute__((nonnull));

// Print raw scanlines on screen (packed pixels).
// Returns -(ENOSYS) when image support is disabled (MINIMAL build w/o IMAGE).
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
FBINK_API int fbink_print_raw_data(int fbfd,
				   const unsigned char* restrict data,
				   const int    w,
				   const int    h,
				   const size_t len,
				   short int    x_off,
				   short int    y_off,
				   const FBInkConfig* restrict fbink_cfg) __attribute__((nonnull));

//
// Just clear the screen (or a region of it), using the background pen color, eInk refresh included (or not ;)).
// Returns -(ENOSYS) when drawing primitives are disabled (MINIMAL build w/o DRAW).
// fbfd:		Open file descriptor to the framebuffer character device,
//				if set to FBFD_AUTO, the fb is opened & mmap'ed for the duration of this call.
// fbink_cfg:		Pointer to an FBInkConfig struct (honors is_inverted, wfm_mode, dithering_mode, is_nightmode, is_flashing,
//				as well as no_refresh & bg_color).
// rect:		Optional pointer to an FBInkRect rectangle (as, say, returned by fbink_get_last_rect),
//				describing the specific region of screen to clear (in absolute coordinates).
//				If the rectangle is empty (i.e., width or height is zero) or the pointer is NULL,
//				the full screen will be cleared.
// no_rota:		Optional, and only useful in very limited cases. When in doubt, set to false.
//				When passing a rect, this requests *not* applying any further rotation hacks,
//				(e.g., isNTX16bLandscape).
//				This is mildly useful if you got a *rotated* rect out of fbink_get_last_rect
//				on such a quirky framebuffer state,
//				and just want to re-use it as-is without mangling the rotation again.
// NOTE: This can be used to draw arbitrary filled rectangles (using the bg pen color),
//       but, for convenience, fbink_fill_rect_gray & fbink_fill_rect_rgba are also available.
FBINK_API int fbink_cls(int fbfd, const FBInkConfig* restrict fbink_cfg, const FBInkRect* restrict rect, bool no_rota)
    __attribute__((nonnull(2)));

// Like fbink_cls, but instead of absolute coordinates, rely on grid coordinates like fbink_print.
// Honors all the same positioning trickery than fbink_print (i.e., row/col mixed w/ hoffset/voffset).
// fbfd:		Open file descriptor to the framebuffer character device,
//				if set to FBFD_AUTO, the fb is opened & mmap'ed for the duration of this call.
// fbink_cfg:		Pointer to an FBInkConfig struct (honors col, row, is_halfway, is_centered, is_padded, is_rpadded,
//				voffset, hoffset, is_overlay, is_bgless,
//				wfm_mode, dithering_mode, is_nightmode, is_flashing, no_refresh).
// cols:		Amount of columns to clear (i.e., width).
// rows:		Amount of rows to clear (i.e., height).
FBINK_API int fbink_grid_clear(int                fbfd,
			       unsigned short int cols,
			       unsigned short int rows,
			       const FBInkConfig* restrict fbink_cfg) __attribute__((nonnull));

// Like fbink_refresh, but instead of absolute coordinates, rely on grid coordinates like fbink_print.
// Honors all the same positioning trickery than fbink_print (i.e., row/col mixed w/ hoffset/voffset).
// fbfd:		Open file descriptor to the framebuffer character device,
//				if set to FBFD_AUTO, the fb is opened for the duration of this call.
// fbink_cfg:		Pointer to an FBInkConfig struct (honors col, row, is_halfway, is_centered, is_padded, is_rpadded,
//				voffset, hoffset, is_overlay, is_bgless,
//				wfm_mode, dithering_mode, is_nightmode, is_flashing).
// cols:		Amount of columns to refresh (i.e., width).
// rows:		Amount of rows to refresh (i.e., height).
// NOTE: This *ignores* no_refresh ;).
FBINK_API int fbink_grid_refresh(int                fbfd,
				 unsigned short int cols,
				 unsigned short int rows,
				 const FBInkConfig* restrict fbink_cfg) __attribute__((nonnull));

//
// Dump the full screen.
// Returns -(ENOSYS) when image support is disabled (MINIMAL build w/o IMAGE).
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
// NOTE: On Kobo devices with a sunxi SoC, you will not be able to capture content that you haven't drawn yourself first.
//       (There's no "shared" framebuffer, each process gets its own private, zero-initialized (i.e., solid black) buffer).
FBINK_API int fbink_dump(int fbfd, FBInkDump* restrict dump) __attribute__((nonnull));

// Dump a specific region of the screen.
// Returns -(ENOSYS) when image support is disabled (MINIMAL build w/o IMAGE).
// Returns -(EINVAL) when trying to dump an empty region.
// fbfd:		Open file descriptor to the framebuffer character device,
//				if set to FBFD_AUTO, the fb is opened & mmap'ed for the duration of this call.
// x_off:		Dump coordinates, x (honors negative offsets).
// y_off:		Dump coordinates, y (honors negative offsets).
// w:			Width of the region to dump.
// h:			Height of the region to dump.
// fbink_cfg:		Pointer to an FBInkConfig struct (honors any combination of halign/valign, row/col & x_off/y_off).
// dump:		Pointer to an FBInkDump struct (will be recycled if already used).
// NOTE: The same considerations as in fbink_dump should be taken regarding the handling of FBInkDump structs.
FBINK_API int fbink_region_dump(int                fbfd,
				short int          x_off,
				short int          y_off,
				unsigned short int w,
				unsigned short int h,
				const FBInkConfig* restrict fbink_cfg,
				FBInkDump* restrict dump) __attribute__((nonnull));

// Like fbink_region_dump, but takes an FBInkRect as input, and uses it *as is* (i.e., no rotation/positioning tricks).
// Returns -(ENOSYS) when image support is disabled (MINIMAL build w/o IMAGE).
// Returns -(EINVAL) when trying to dump an OOB region.
// The intended use case is being able to use a rect returned by fbink_get_last_rect
// without having to think about the potential fallout from positioning or rotation hacks.
// (c.f., also the "no_rota" flag for fbink_cls).
// NOTE: If NULL or an empty rect is passed, a full dump will be made instead.
// NOTE: The same considerations as in fbink_dump should be taken regarding the handling of FBInkDump structs.
FBINK_API int fbink_rect_dump(int fbfd, const FBInkRect* restrict rect, FBInkDump* restrict dump)
    __attribute__((nonnull(3)));

// Restore a framebuffer dump made by fbink_dump/fbink_region_dump/fbink_rect_dump.
// Returns -(ENOSYS) when image support is disabled (MINIMAL build w/o IMAGE).
// Otherwise, returns a few different things on failure:
//	-(ENOTSUP)	when the dump cannot be restored because it wasn't taken at the current bitdepth and/or rotation,
//			or because it's wider/taller/larger than the current framebuffer, or if the crop is invalid (OOB).
//	-(EINVAL)	when there's no data to restore.
// fbfd:		Open file descriptor to the framebuffer character device,
//				if set to FBFD_AUTO, the fb is opened & mmap'ed for the duration of this call.
// fbink_cfg:		Pointer to an FBInkConfig struct (honors wfm_mode, dithering_mode, is_nightmode,
//				is_flashing & no_refresh).
// dump:		Pointer to an FBInkDump struct, as setup by fbink_dump or fbink_region_dump.
// NOTE: In case the dump was regional, it will be restored in the exact same coordinates it was taken from,
//       no actual positioning is needed/supported at restore time.
// NOTE: This does not support any kind of software processing, at all!
//       If you somehow need inversion or dithering, it has to be supported at the hardware level at refresh time by your device,
//       (i.e., dithering_mode vs. sw_dithering, and is_nightmode vs. is_inverted).
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
FBINK_API int fbink_restore(int fbfd, const FBInkConfig* restrict fbink_cfg, const FBInkDump* restrict dump)
    __attribute__((nonnull));

// Free the data allocated by a previous fbink_dump() or fbink_region_dump() call.
// Returns -(ENOSYS) when image support is disabled (MINIMAL build w/o IMAGE).
// Otherwise, returns a few different things on failure:
//	-(EINVAL)	when the dump has already been freed.
// dump:		Pointer to an FBInkDump struct.
// NOTE: You MUST call this when you have no further use for this specific data.
// NOTE: But, you MAY re-use a single FBInkDump struct across different dump() calls *without* calling this in between,
//       as dump() will implicitly free a dirty struct in order to recycle it.
FBINK_API int fbink_free_dump_data(FBInkDump* restrict dump) __attribute__((nonnull));

//
// Return the coordinates & dimensions of the last thing that was *drawn*.
// Returns an empty (i.e., {0, 0, 0, 0}) rectangle if nothing was drawn.
// rotated:		Returns rotated coordinates if applicable.
// NOTE: These are unfiltered *framebuffer* coordinates.
//       If your goal is to use that for input detection, mapping that to input coordinates is your responsibility.
//       On Kobo, fbink_get_state should contain enough data to help you figure out what kinds of quirks you need to account for.
// NOTE: While this *generally* maps to the refresh region, this does not always hold true:
//       this will get updated regardless of no_refresh,
//       and will ignore what is_flashing might do to make the refresh region fullscreen.
//       i.e., it corresponds to what's drawn to the fb, not necessarily to what's refreshed on screen.
// NOTE: On devices where we may fudge the coordinates to account for broken rotation (i.e., most Kobos @ 16bpp),
//       these are, by default, the *unrotated* coordinates!
//       i.e., they will *NOT* match with what we actually send to mxcfb (and where we actually drew on the fb)!
//       Nothing in our public API actually expects any other kind of coordinates,
//       so having this return the rotated coordinates would be confusing...
//       If, for some reason (e.g., comparing against actual ioctl values),
//       you *do* need the rotated variant, set rotated to true.
FBINK_API FBInkRect fbink_get_last_rect(bool rotated);

//
// Scan the screen for Kobo's "Connect" button in the "USB plugged in" popup,
// and optionally generate an input event to press that button.
// NOTE: This is deprecated, and no longer built by default on Kobo.
//       NickelMenu and/or NickelDBus provide far more robust solutions for this.
// KOBO i.MX Only! Returns -(ENOSYS) when disabled (!KOBO, or Kobo on a sunxi SoC, as well as MINIMAL builds w/o BUTTON_SCAN).
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
FBINK_API int fbink_button_scan(int fbfd, bool press_button, bool nosleep) __attribute__((deprecated));

// Wait for the end of a Kobo USBMS session, trying to detect a successful content import in the process.
// NOTE: Expects to be called while in the "Connected" state (like after a successful fbink_button_scan() call w/ press_buton)!
//       It will abort early if that's not the case.
// NOTE: For the duration of this call (which is obviously blocking!), screen updates should be kept to a minimum:
//       in particular, we expect the middle section of the final line to be untouched!
// KOBO i.MX Only! Returns -(ENOSYS) when disabled (!KOBO, or Kobo on a sunxi SoC, as well as MINIMAL builds w/o BUTTON_SCAN).
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
// Attempt to untangle the rotation state on Kobo devices, converting between the murky "native" value
// (i.e., what's in the fb vinfo), and a "canonical" one, representing how the device is actually physically laid out.
// KOBO Only! Returns ENOSYS when disabled (!KOBO), or ERANGE if the input rotation is invalid.
// Yes, those are positive values, given the function's signature ;).
// See fbink_rota_quirks.c & utils/fbdepth.c for more details.
FBINK_API uint8_t  fbink_rota_native_to_canonical(uint32_t rotate);
FBINK_API uint32_t fbink_rota_canonical_to_native(uint8_t rotate);

//
// Inverts the *existing* content of the *full* screen.
// This is mildly useful on devices with no HW inversion support,
// to trigger a full "nightmode" swap, without actually having to redraw anything.
// fbfd:		Open file descriptor to the framebuffer character device,
//				if set to FBFD_AUTO, the fb is opened & mmap'ed for the duration of this call.
// fbink_cfg:		Pointer to an FBInkConfig struct.
// NOTE: On Kobo devices with a sunxi SoC, you will not be able to affect content that you haven't drawn yourself first.
//       i.e., this will only apply to stuff drawn via FBInk's own framebuffer pointer (be it by FBInk or yourself).
FBINK_API int fbink_invert_screen(int fbfd, const FBInkConfig* restrict fbink_cfg) __attribute__((nonnull));

//
// Inverts the *existing* content of a specific *region* of the screen.
// This is mildly useful on devices with no HW inversion support,
// to implement inversion after the fact when you don't necessarily control the drawing.
// NOTE: Unlike fbink_invert_screen, this does *NOT* trigger a refresh!
// fbfd:		Open file descriptor to the framebuffer character device,
//				if set to FBFD_AUTO, the fb is opened & mmap'ed for the duration of this call.
// rect:		Optional pointer to an FBInkRect rectangle (as, say, returned by fbink_get_last_rect),
//				describing the specific region of screen to invert (in absolute coordinates).
//				If the rectangle is empty (i.e., width or height is zero) or the pointer is NULL,
//				the full screen will be inverted.
// no_rota:		Optional, and only useful in very limited cases. When in doubt, set to false.
//				When passing a rect, this requests *not* applying any further rotation hacks,
//				(e.g., isNTX16bLandscape).
//				This is mildly useful if you got a *rotated* rect out of fbink_get_last_rect
//				on such a quirky framebuffer state,
//				and just want to re-use it as-is without mangling the rotation again.
// NOTE: On Kobo devices with a sunxi SoC, you will not be able to affect content that you haven't drawn yourself first.
//       i.e., this will only apply to stuff drawn via FBInk's own framebuffer pointer (be it by FBInk or yourself).
FBINK_API int fbink_invert_rect(int fbfd, const FBInkRect* restrict rect, bool no_rota);

//
// The functions below are much lower level than the rest of the API:
// outside of GUI toolkit implementations and very specific workflows, you shouldn't need to rely on them.
//
// Grants direct access to the backing buffer's base pointer, as well as its size (in bytes; e.g., smem_len).
// MUST NOT be called before fbink_init
// MUST NOT be called with an FBFD_AUTO fbfd
// MAY be called before before any fbink_print*, fbink_dump/restore, fbink_cls or fbink_grid* functions.
//     (i.e., it'll implicitly setup the backing buffer if necessary).
// Returns NULL on failure (in which case, *buffer_size is set to 0).
// NOTE: This *may* need to be refreshed after a framebuffer state change, c.f., fbink_reinit!
//       (In practice, though, the pointer itself is stable;
//        only the buffer/mapping size may change on some quirky platforms (usually, PB)).
// fbfd:		Open file descriptor to the framebuffer character device,
//				cannot be set to FBFD_AUTO!
// buffer_size:		Out parameter. On success, will be set to the buffer's size, in bytes.
FBINK_API unsigned char* fbink_get_fb_pointer(int fbfd, size_t* buffer_size) __attribute__((nonnull));

// For when you *really* need a mostly untouched copy of the full linuxfb structs...
// NOTE: Prefer fbink_get_state, unless you *really* have no other choices...
FBINK_API void fbink_get_fb_info(struct fb_var_screeninfo* var_info, struct fb_fix_screeninfo* fix_info)
    __attribute__((nonnull));

// Magic constants for fbink_set_fb_info (> INT8_MAX to steer clear of legitimate values)
#define KEEP_CURRENT_ROTATE    (1 << 7)
#define KEEP_CURRENT_BITDEPTH  (1 << 7)
#define KEEP_CURRENT_GRAYSCALE (1 << 7)
#define TOGGLE_GRAYSCALE       (1 << 6)
// Sets the framebuffer's bitdepth and/or native rotation.
// MUST NOT be called before fbink_init
// Only tested on Kobo & Kindle, here be dragons on other platforms!
// Returns a few different things on failure:
//	-(ENODEV)	if called before fbink_init
//	-(EINVAL)	when one of rota/bpp/grayscale is invalid
//	-(ECANCELED)	if an ioctl failed, meaning the fb state may be left in an undefined state.
//			This is *highly* unlikely, but, if it happens,
//			checking the sanity of the fb state is the caller's responsibilty!
//			(i.e., you'll have to *at least* run fbink_reinit yourself).
// NOTE: On sunxi, only the rotation can be controlled: i.e., this will simply invoke fbink_sunxi_ntx_enforce_rota,
//       except we only accept values matching linuxfb rotation constants.
//       Prefer using fbink_sunxi_ntx_enforce_rota directly yourself.
// NOTE: On success, this will reinit the state *now* (returning the exact same values as fbink_reinit).
//       In particular, if you're using fbink_get_state and/or fbink_get_fb_pointer,
//       check and handle that return value properly (as you would an actual fbink_reinit call),
//       or you may be left with stale state data if you don't refresh it when necessary :).
// fbfd:		Open file descriptor to the framebuffer character device.
//				if set to FBFD_AUTO, the fb is opened for the duration of this call.
// rota:		*native* linuxfb rotation value (c.f., fbink_rota_canonical_to_native).
//				Untouched if set to KEEP_CURRENT_ROTATE
// bpp:			bitdepth value (in bits).
//				Supported values: 4, 8, 16, 32
//				Untouched if set to KEEP_CURRENT_BITDEPTH
// grayscale:		grayscale value.
//				(enforced to 0 if bpp != 8).
//				If bpp == 8, only meaningful on mxcfb or mtk:
//				Generally set to GRAYSCALE_8BIT (1),
//				setting it to GRAYSCALE_8BIT_INVERTED (2)
//				will automagically enforce HW inversion via EPDC_FLAG_ENABLE_INVERSION (or similar).
//				On Kindle MTK+Bellatrix4+CFA, GRAYSCALE_COLOR (0) and GRAYSCALE_COLOR_NIGHTMODE (3) are also supported.
//				Untouched if set to KEEP_CURRENT_GRAYSCALE
//				If set to TOGGLE_GRAYSCALE, will toggle between INVERTED & not @ 8bpp
// fbink_cfg:		Pointer to an FBInkConfig struct.
FBINK_API int fbink_set_fb_info(int      fbfd,
				uint32_t rota,
				uint8_t  bpp,
				uint8_t  grayscale,
				const FBInkConfig* restrict fbink_cfg) __attribute__((warn_unused_result, nonnull));

// These behave exactly like fbink_cls, but allow you to choose a color directly, instead of relying on the pen's bg color.
// Mostly useful for GUI toolkit backends, but depending on how it's actually used,
// remember that this honors `no_refresh` & `is_inverted`!
// Returns -(ENOSYS) when drawing primitives are disabled (MINIMAL build w/o DRAW).
// c.f., `fbink_cls` for documentation of the initial parameters they share.
// y:			8-bit luminance value
FBINK_API int fbink_fill_rect_gray(int fbfd,
				   const FBInkConfig* restrict fbink_cfg,
				   const FBInkRect* restrict rect,
				   bool    no_rota,
				   uint8_t y) __attribute__((nonnull(2)));
// r:			8-bit red component value
// g:			8-bit green component value
// b:			8-bit blue component value
// a:			8-bit alpha component value (opaque is 0xFFu).
FBINK_API int fbink_fill_rect_rgba(int fbfd,
				   const FBInkConfig* restrict fbink_cfg,
				   const FBInkRect* restrict rect,
				   bool    no_rota,
				   uint8_t r,
				   uint8_t g,
				   uint8_t b,
				   uint8_t a) __attribute__((nonnull(2)));

// Convenience public wrappers for a per-pixel put/get.
// These are designed with *convenience* in mind, *not* performance.
// (In particular, a pixel needs to be packed on *each* call).
// I'd highly recommend handling drawing yourself if you can ;).
// NOTE: Unlike the fbink_fill_rect family of functions,
//       the fbink_put_pixel family will *never* trigger a refresh.
//       (Which explains why they don't need to take an FBInkConfig pointer).
// Returns -(ENOSYS) when drawing primitives are disabled (MINIMAL build w/o DRAW).
// x:                   x coordinates
// y:                   y coordinates
// v:			8-bit luminance value
FBINK_API int fbink_put_pixel_gray(int fbfd, uint16_t x, uint16_t y, uint8_t v);
// r:			8-bit red component value
// g:			8-bit green component value
// b:			8-bit blue component value
// a:			8-bit alpha component value (opaque is 0xFFu).
FBINK_API int fbink_put_pixel_rgba(int fbfd, uint16_t x, uint16_t y, uint8_t r, uint8_t g, uint8_t b, uint8_t a);
// *r:			out pointer, 8-bit red component value
// *g:			out pointer, 8-bit green component value
// *b:			out pointer, 8-bit blue component value
// *a:			out pointer, 8-bit alpha component value (opaque is 0xFFu).
// NOTE: If pixelformat is grayscale, r = g = b and a = 0xFF
// NOTE: Red always means red, if there's a BGR swap involved, it's handled for you.
//       Similarly, BGR565/RBG565 is unpacked to RGB32.
FBINK_API int fbink_get_pixel(int fbfd, uint16_t x, uint16_t y, uint8_t* r, uint8_t* g, uint8_t* b, uint8_t* a);

// As a means to alleviate *some* of the pixel packing overhead mentioned above,
// the following set of functions allow you to actually *save* a packed pixel,
// and pass it to dedicated variants of put_pixel & fill_rect.
// This is helpful if you often reuse the same color.
// NOTE: The packing is only accurate for the *current* pixel format,
//       consider re-packing after a potential bitdepth change!
//       (e.g., when handling an fbink_reinit call).
// Returns -(ENOSYS) when drawing primitives are disabled (MINIMAL build w/o DRAW).
FBINK_API int fbink_pack_pixel_gray(uint8_t y, uint32_t* px);
FBINK_API int fbink_pack_pixel_rgba(uint8_t r, uint8_t g, uint8_t b, uint8_t a, uint32_t* px);
// c.f., `fbink_put_pixel_*` for documentation of the initial parameters they share.
// px:                   out pointer, packed pixel in the *current* framebuffer pixel format
FBINK_API int fbink_put_pixel(int fbfd, uint16_t x, uint16_t y, void* px);
FBINK_API int fbink_fill_rect(int fbfd,
			      const FBInkConfig* restrict fbink_cfg,
			      const FBInkRect* restrict rect,
			      bool  no_rota,
			      void* px) __attribute__((nonnull(2)));
// c.f., `fbink_put_pixel_*` & `fbink_fill_rect_*` for documentation of the initial parameters they share.
// px:                   pointer to a packed pixel, as provided by the fbink_pack_pixel_* family of functions.

// Forcefully wakeup the EPDC (Kobo Mk.8+ only)
// We've found this to be helpful on a few otherwise crashy devices,
// c.f., https://github.com/koreader/koreader-base/pull/1645 & https://github.com/koreader/koreader/pull/10771
// Keep in mind that a few Mk. 7 devices can also be mildly temperamental in some circumstances,
// and that this feature isn't supported on those, c.f., https://github.com/koreader/koreader/issues/11511
// NOTE: Nickel fires this off from its input handler, debounced at roughly 1.5s or 4s intervals,
//       which sounds like a slightly more elegant approach than the one I opted for in KOReader ;).
//       (As a fun quirk, it only does this after *touch* inputs, not *key* inputs).
// Returns -(ENOSYS) on unsupported platforms.
FBINK_API int fbink_wakeup_epdc(void);

//
// The functions below are tied to specific capabilities on Kobo devices with a sunxi SoC (e.g., the Elipsa & Sage).
//
// Toggle the "pen" refresh mode. c.f., eink/sunxi-kobo.h @ DISP_EINK_SET_NTX_HANDWRITE_ONOFF for more details.
// The TL;DR being that it's only truly active when using A2 & DU waveform modes.
// And since, on sunxi, A2's MONOCHROME flag is just *software* dithering, you might actually prefer DU.
// Returns -(ENOSYS) on unsupported platforms.
// fbfd:		Open file descriptor to the framebuffer character device,
//				if set to FBFD_AUTO, the fb is opened & mmap'ed for the duration of this call.
// NOTE: Outside of tracing pen input, it has another interesting side-effect:
//       since it disables the layer overlap check, it allows you to display stuff in a different layout
//       than the current working buffer without the (heavy) visual artifacts that would otherwise imply.
//       Then again, it also leaves an eink kernel thread spinning at 100% CPU until the next standard update,
//       so this might not be such a great idea after all...
//       c.f., kobo_sunxi_fb_fixup @ fbink.c for more details.
// NOTE: Another option for "dealing" with these rotation mishaps is to just assume the screen is always Upright.
//       You can achieve that by making sure FBINK_FORCE_ROTA is set properly in your env (*before* initializing FBInk).
// NOTE: Or, you can also affect how FBInk tries to honor the gyro (or not) at runtime, via fbink_sunxi_ntx_enforce_rota.
FBINK_API int fbink_sunxi_toggle_ntx_pen_mode(int fbfd, bool toggle);

// Allows controlling at runtime how fbink_init & fbink_reinit handle rotation,
// potentially bypassing and/or selectively overriding the state returned by the accelerometer.
// Returns -(ENOSYS) on unsupported platforms.
// Otherwise, returns a few different things on failure:
//	-(EINVAL)	when mode is invalid
//	-(ENOTSUP)	when mode is unsupported
// NOTE: See the comments in the SUNXI_FORCE_ROTA_INDEX_E enum.
//       In particular, the fact that the most interesting modes aren't actually supported because of technical limitations,
//       unless the custom fbdamage module has been loaded (earlier than the disp client you're trying to match)...
//       c.f., <https://github.com/NiLuJe/mxc_epdc_fb_damage>.
// NOTE: On success, this will reinit the state *now* (returning the exact same values as fbink_reinit).
FBINK_API int fbink_sunxi_ntx_enforce_rota(int fbfd, SUNXI_FORCE_ROTA_INDEX_T mode, const FBInkConfig* restrict fbink_cfg)
    __attribute__((warn_unused_result, nonnull));

//
// The functions below are tied to specific capabilities on Kindle devices with a MediaTek SoC (e.g., the PW5).
//
// Setup the swipe animation direction & duration used by every refresh when is_animated is set in the FBInkConfig struct.
// Returns -(ENOSYS) on unsupported platforms.
// NOTE: Maximum value for steps is 60 on Bellatrix. The ioctl will throw an EBADF otherwise.
// NOTE: The refresh dimension that matches the animation direction (i.e., width for L/R & height for T/B)
//       needs to be >= than the amount of steps, or the animation will be disabled.
//       (Unlike the max steps, FBInk will sanity check this, as it currently crashes the driver ;)).
// NOTE: Malbec currrently uses 12 in the Reader.
FBINK_API int fbink_mtk_set_swipe_data(MTK_SWIPE_DIRECTION_INDEX_T direction, uint8_t steps);

// Wait (up to 2s) for *every* pending refresh!
// Returns -(ENOSYS) on unsupported platforms.
// NOTE: When "fast mode" is enabled (c.f., fbink_mtk_toggle_auto_reagl below),
//       no updates are able to be collected, meaning this will *always* timeout!
//       TL;DR: Don't mix with fbink_mtk_toggle_auto_reagl(false)!
FBINK_API int fbink_wait_for_any_complete(int fbfd);

// Setup the screen regions to gray out with a checkered pattern (Kindle only).
// Returns -(ENOSYS) on unsupported platforms.
// NOTE: Both of the regions are the *excluded* regions (i.e., the regions that will *NOT* be checkered).
//       You *MAY* only set a single exclude region, in which case, use the first array member.
// NOTE: Setting the first region to an empty rectangle will disable the feature (as will setting size to MTK_HALFTONE_DISABLED).
// NOTE: For non-default sizes, the actual size is in pixel, and equal to size - 1
// NOTE: This does *NOT* refresh the screen, it just sets things up for subsequent refreshes.
// NOTE: This will *NOT* apply to EPDC_FLAG_USE_ALT_BUFFER updates.
FBINK_API int fbink_mtk_set_halftone(int fbfd, const FBInkRect exclude_regions[2], MTK_HALFTONE_MODE_INDEX_T size);

// Toggle whether large enough refresh regions will automatically be upgraded to REAGL (Kindle only).
// NOTE: Only applies to DU, GL16 & GC16 PARTIAL updates.
// NOTE: Only applies in day mode (e.g., GRAYSCALE_8BIT, not GRAYSCALE_8BIT_INVERTED).
// NOTE: Currently applies to regions both over a third of the screen's width and a fourth of the screen's height.
// NOTE: When this is *disabled* (i.e., the so-called "fast mode" is enabled),
//       updates will *NOT* be collected by the WAIIT_FOR_ANY_UPDATE_COMPLETE ioctl!
// Returns -(ENOSYS) on unsupported platforms.
FBINK_API int fbink_mtk_toggle_auto_reagl(int fbfd, bool toggle);

// Toggle the "pen" refresh mode (Kindle on Bellatrix3 only).
// Returns -(ENOSYS) on unsupported platforms (NOTE: We currently *allow* this on Bellatrix, where it will fail).
// fbfd:		Open file descriptor to the framebuffer character device,
//				if set to FBFD_AUTO, the fb is opened & mmap'ed for the duration of this call.
// NOTE: Among other things, this enforces similar behavior as in "fast" mode re: automatic REAGL upgrades.
// NOTE: There is a *strong* "no collision with standard mode" constraint that is left to *userland* to enforce (i.e., you).
//       Pen mode updates *must* use DU (or DUNM iff in nightmode (e.g., GRAYSCALE_8BIT_INVERTED)),
//       failing to do so will temporarily disable pen mode.
FBINK_API int fbink_mtk_toggle_pen_mode(int fbfd, bool toggle);

//
// The functions below are small utilities to make working with input devices slightly less painful.
//

// Input device classification
typedef enum
{
	INPUT_UNKNOWN            = 0U,
	// Standard udev classification
	INPUT_POINTINGSTICK      = 1U << 0U,
	INPUT_MOUSE              = 1U << 1U,
	INPUT_TOUCHPAD           = 1U << 2U,
	INPUT_TOUCHSCREEN        = 1U << 3U,
	INPUT_JOYSTICK           = 1U << 4U,
	INPUT_TABLET             = 1U << 5U,    // Includes touchscreens w/ stylus support
	INPUT_KEY                = 1U << 6U,    // Supports at least *1* EV_KEY keycode (may include touchscreens)
	INPUT_KEYBOARD           = 1U << 7U,
	INPUT_ACCELEROMETER      = 1U << 8U,
	// Custom classification, tailored for our use-cases
	INPUT_POWER_BUTTON       = 1U << 16U,
	INPUT_SLEEP_COVER        = 1U << 17U,
	INPUT_PAGINATION_BUTTONS = 1U << 18U,
	INPUT_HOME_BUTTON        = 1U << 19U,
	INPUT_LIGHT_BUTTON       = 1U << 20U,    // e.g., the frontlight toggle button on some early Kobos
	INPUT_MENU_BUTTON        = 1U << 21U,
	INPUT_DPAD               = 1U << 22U,
	INPUT_ROTATION_EVENT     = 1U << 23U,    // Where "device was rotated" events *may* be sent (gyro or not)
	INPUT_SCALED_TABLET      = 1U << 24U,    // INPUT_TABLET, with max ABS_X/ABS_Y that match the fb's resolution
	INPUT_VOLUME_BUTTONS     = 1U << 25U,
} __attribute__((packed)) INPUT_DEVICE_TYPE_E;
typedef uint32_t          INPUT_DEVICE_TYPE_T;

// Input classification settings
typedef enum
{
	SCAN_ONLY     = 1U << 0U,    // Do *NOT* leave any fd's open'ed
	OPEN_BLOCKING = 1U << 1U,    // Do *NOT* open fd's with O_NONBLOCK
	MATCH_ALL     = 1U << 2U,    // Match on *all* the match_types bits instead of *any*
	EXCLUDE_ALL   = 1U << 3U,    // Exclude on *all* the exclude_types bits instead of *any*
	NO_RECAP      = 1U << 4U,    // Do *NOT* print a recap of the results of input device classification
} __attribute__((packed)) INPUT_SETTINGS_TYPE_E;
typedef uint32_t          INPUT_SETTINGS_TYPE_T;

typedef struct
{
	INPUT_DEVICE_TYPE_T type;          // bitmask
	int                 fd;            // Set to -1 when not open
	bool                matched;       // true if type matches the match/exclude combo
	char                name[256];     // As reported by EVIOCGNAME
	char                path[4096];    // e.g., /dev/input/event%d
} FBInkInputDevice;

// Scan & classify input devices into actionable categories.
// Returns a pointer to the first element of an array of FBInkInputDevice structs, containing `dev_count` elements.
// Regardless of the filter you request, this will always contain *all* the device's input devices.
// The `matched` field will be set to true if that device matches *any/all* (depending on `MATCH_ALL`) of the bits in `match_types`
// and *not* *any/all* (depending on `EXCLUDE_ALL`) of the bits in `exclude_types`, meaning you can either cast a fairly wide net,
// and still catch everything you care about; or tackle an exclude mask on top for more fine-grained filtering.
// You *MUST* free the returned pointer after use (it's heap allocated).
// Returns NULL on failure (no input devices can be read, or MINIMAL build w/o INPUT).
// match_types:		Bitmask used to filter the type of input devices you want to open.
// exclude_types:	Bitmask used to filter *out* some input device types from results that matched match_types.
//			Set to 0 to forgo an exclude mask.
// settings:		Bitmask that controls some of the scan's behavior.
//				if the OPEN_BLOCKING bit is set, fds will be opened in *blocking* mode.
//					Otherwise, the default open flags are O_RDONLY|O_NONBLOCK|O_CLOEXEC
//				if the SCAN_ONLY bit is set, *no* fds will be returned, regardless of the filter.
//				if the MATCH_ALL bit is set,
//				a device must feature *all* of the bits in match_types to be considered a match.
//					Otherwise, any of each individal bit will be enough.
//				if the EXCLUDE_ALL bit is set,
//				a device must *not* feature *all* of the bits in exclude_types to be considered a match.
//					Otherwise, any of each individal bit will be enough to warrant an exclusion.
//			Set to 0 for default settings.
// dev_count:		out pointer, will be set to the amount of array elements in the returned data.
// NOTE: This does *NOT* require fbink to be initialized, but *does* honor its internal verbosity state.
FBINK_API FBInkInputDevice* fbink_input_scan(INPUT_DEVICE_TYPE_T   match_types,
					     INPUT_DEVICE_TYPE_T   exclude_types,
					     INPUT_SETTINGS_TYPE_T settings,
					     size_t*               dev_count);

// Variant of the above that takes a filepath instead of scanning /dev/input/event*
// This is useful to handle hotplug events, for instance.
// Returns a pointer to an FBInkInputDevice struct.
// You *MUST* free the returned pointer after use (it's heap allocated).
// Returns NULL on failure (filepath cannot be read, or MINIMAL build w/o INPUT).
FBINK_API FBInkInputDevice* fbink_input_check(const char*           filepath,
					      INPUT_DEVICE_TYPE_T   match_types,
					      INPUT_DEVICE_TYPE_T   exclude_types,
					      INPUT_SETTINGS_TYPE_T settings);
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

#ifdef __cplusplus
}
#endif

#endif
