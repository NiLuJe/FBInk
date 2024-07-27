/*****************************************************************************
 * Copyright (C) 2016 MediaTek Inc.
 *
 * ----
 *
 * This is <linux/hwtcon_ioctl_cmd.h>, last updated from the Elipsa 2E kernel
 *
 * NOTE: Upstream kernels available here: https://github.com/kobolabs/Kobo-Reader/tree/master/hw/mt8113-elipsa2e
 *
 * - Frankensteined to play nice w/ MXCFB constants -- NiLuJe
 *
 * ----
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See http://www.gnu.org/licenses/gpl-2.0.html for more details.
 *
 * Accelerometer Sensor Driver
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 *
 *****************************************************************************/

#ifndef __HWTCON_IOCTL_CMD_H__
#define __HWTCON_IOCTL_CMD_H__

#ifndef __KERNEL__
#	include <stdint.h>
#endif

/* HWTCON_FLAG_xx */
#define HWTCON_FLAG_USE_DITHERING         0x1
#define HWTCON_FLAG_FORCE_A2_OUTPUT       0x10    // Mainly used for pen updates, requires HWTCON_WAVEFORM_MODE_A2
#define HWTCON_FLAG_FORCE_A2_OUTPUT_WHITE 0x20    // Black pen, requires HWTCON_FLAG_FORCE_A2_OUTPUT
#define HWTCON_FLAG_FORCE_A2_OUTPUT_BLACK 0x40    // White pen, requires HWTCON_FLAG_FORCE_A2_OUTPUT
// Pen color is auto-detected if only HWTCON_FLAG_FORCE_A2_OUTPUT is provided

// Introduced with Kaleido devices
// NOTE: Used by the driver to convert between the user & kernel representations of the CFA_MODE flags.
//       c.f., HWTCON_FLAG_GET_CFA_MODE/HWTCON_FLAG_SET_CFA_MODE @ drivers/misc/mediatek/hwtcon/hwtcon_fb.h
// NOTE: We pull in the actual kernel representations later, as they're used for the set/get cfa_mode ioctls...
#define HWTCON_FLAG_CFA_FLDS_MASK 0x00007f00

// NOTE: The 'S' variants appear to affect more things than plain saturation
//       (contrast? intensity? levels in general?), hence the polarization & clipping effects.
//       They... rarely look good, but they *do* technically lead to the most vibrant colors.
//       The strength of the filters go crescendo, so banding may not be immediately visible at S4,
//       depending on the content being displayed.
// NOTE: Generally speaking you'll want to keep to the defaults and implement a saturation boost yourself,
//       or play with G2, as it's the least destructive option, and roughly matches a 50% HSP sat boost.
#define HWTCON_FLAG_CFA_EINK_AIE_S4 0x00000200
#define HWTCON_FLAG_CFA_EINK_AIE_S7 0x00000300
#define HWTCON_FLAG_CFA_EINK_AIE_S9 0x00000400
#define HWTCON_FLAG_CFA_EINK_G0     0x00000500    // Desaturate
#define HWTCON_FLAG_CFA_EINK_G1     0x00000100    // Standard behavior (e.g., same results as no flags)
#define HWTCON_FLAG_CFA_EINK_NORMAL HWTCON_FLAG_CFA_EINK_G1
#define HWTCON_FLAG_CFA_EINK_G2     0x00000600    // Boosts saturation without being too destructive
// NOTE: Those two look like test implementations by NTX (as opposed to stuff from eInk's libEink_Kaleido binary blob).
#define HWTCON_FLAG_CFA_EINK_NTX    0x00000a00
#define HWTCON_FLAG_CFA_EINK_NTX_SF 0x00000b00
// NOTE: Disables CFA processing, leading to B&W rendering (i.e., HWTCON_CFA_MODE_NONE).
#define HWTCON_FLAG_CFA_SKIP        0x00008000

/* temperature use sensor. */
// NOTE: No longer set request-by-request, but globally via HWTCON_SET_TEMPERATURE
#define TEMP_USE_SENSOR 0x100000

// Matches MXCFB
#define UPDATE_MODE_PARTIAL 0x0
#define UPDATE_MODE_FULL    0x1

// NOTE: That confusing `enable_night_mode_by_wfm` mapping is never actually used, unless you enable it via the debug procfs knob.
//       FWIW, lab126 does GC16 => GCK16 & GLR16 => GLKW16...
// NOTE: Like it's always been the case on lab126 devices, on newer devices (i.e., not the Elipsa 2E),
//       anything involving REAGL (i.e., GLR16, GLKW16 & GLRC16) is *always* paired with FULL.
//       Likewise, stuff tweaked for Kaleido (GCC16 & GLRC16) is also *always* paired with FULL.
//       GLRC16 *is* a REAGL waveform mode, so it won't actually flash.
enum HWTCON_WAVEFORM_MODE_ENUM
{
	// Matches MXCFB
	HWTCON_WAVEFORM_MODE_INIT   = 0,
	HWTCON_WAVEFORM_MODE_DU     = 1,
	HWTCON_WAVEFORM_MODE_GC16   = 2,    // => GL16 if PARTIAL; => GCK16 if PARTIAL in NM & => GLKW16 if FULL in NM
	// Doesn't match MXCFB
	HWTCON_WAVEFORM_MODE_GL16   = 3,    // => GCK16 in NM
	HWTCON_WAVEFORM_MODE_GLR16  = 4,    // => GCK16 in NM
	HWTCON_WAVEFORM_MODE_REAGL  = 4,    // => GCK16 in NM
	HWTCON_WAVEFORM_MODE_A2     = 6,
	HWTCON_WAVEFORM_MODE_GCK16  = 8,
	HWTCON_WAVEFORM_MODE_GLKW16 = 9,    // AKA. GCKW16; REAGL DARK
	// Introduced with Kaleido devices
	HWTCON_WAVEFORM_MODE_GCC16  = 10,    // Used for images on color panels.
	HWTCON_WAVEFORM_MODE_GLRC16 = 11,    // Used for color highlights on text. (GLKW16 in dark mode).
	// Matches MXCFB
	HWTCON_WAVEFORM_MODE_AUTO   = 257,
};

#define WAVEFORM_TYPE_4BIT 0x1
#define WAVEFORM_TYPE_5BIT (WAVEFORM_TYPE_4BIT << 1)

enum hwtcon_dithering_mode
{
	// Quantize only?
	HWTCON_FLAG_USE_DITHERING_Y8_Y4_Q = 0x100,
	HWTCON_FLAG_USE_DITHERING_Y8_Y2_Q = 0x200,
	HWTCON_FLAG_USE_DITHERING_Y8_Y1_Q = 0x300,
	HWTCON_FLAG_USE_DITHERING_Y4_Y2_Q = 0x10200,
	HWTCON_FLAG_USE_DITHERING_Y4_Y1_Q = 0x10300,

	// Bayer? (i.e., Ordered)
	HWTCON_FLAG_USE_DITHERING_Y8_Y4_B = 0x101,
	HWTCON_FLAG_USE_DITHERING_Y8_Y2_B = 0x201,
	HWTCON_FLAG_USE_DITHERING_Y8_Y1_B = 0x301,
	HWTCON_FLAG_USE_DITHERING_Y4_Y2_B = 0x10201,
	HWTCON_FLAG_USE_DITHERING_Y4_Y1_B = 0x10301,

	// Floyd-Steinberg?
	HWTCON_FLAG_USE_DITHERING_Y8_Y4_S = 0x102,    // Default, matches Kindle (where it... doesn't do anything :D)
	HWTCON_FLAG_USE_DITHERING_Y8_Y2_S = 0x202,
	HWTCON_FLAG_USE_DITHERING_Y8_Y1_S = 0x302,
	HWTCON_FLAG_USE_DITHERING_Y4_Y2_S = 0x10202,
	HWTCON_FLAG_USE_DITHERING_Y4_Y1_S = 0x10302,
};

struct hwtcon_waveform_modes
{
	/* waveform mode index for HWTCON_WAVEFORM_MODE_INIT */
	int mode_init;
	/* waveform mode index for HWTCON_WAVEFORM_MODE_DU */
	int mode_du;
	/* waveform mode index for HWTCON_WAVEFORM_MODE_GC16 */
	int mode_gc16;
	/* waveform mode index for HWTCON_WAVEFORM_MODE_GL16 */
	int mode_gl16;
	/* waveform mode index for HWTCON_WAVEFORM_MODE_A2 */
	int mode_a2;
	/* waveform mode index for HWTCON_WAVEFORM_MODE_REAGL */
	int mode_reagl;
};

struct hwtcon_rect
{
	uint32_t top;
	uint32_t left;
	uint32_t width;
	uint32_t height;
};

// NOTE: Unused
struct hwtcon_update_marker_data
{
	uint32_t update_marker;
	uint32_t collision_test;    // Unimplemented, for good reason, see HWTCON_WAIT_FOR_UPDATE_COMPLETE handler
};

struct hwtcon_update_data
{
	struct hwtcon_rect update_region;
	/* which waveform to use for the update, du, gc4, gc8 gc16 etc */
	uint32_t           waveform_mode;
	uint32_t           update_mode; /* full update or partial update */
	/* Unique number used by both application
	 * and driver to identify an update
	 */
	uint32_t           update_marker;
	unsigned int       flags;       /* one or more HWTCON_FLAGs defined above */
	int                dither_mode; /* one of the dither modes defined above */
};

struct hwtcon_panel_info
{
	char wf_file_name[100];
	int  vcom_value;
	/* temperature */
	int  temp;
	/* temperature zone */
	int  temp_zone;
};

/* ioctl commds */
#define HWTCON_IOCTL_MAGIC_NUMBER 'F'

// Flips the nightmode flag, prevents GCK16 & GLKW16 from automatically enabling nightmode, and inverts the fb.
// NOTE: Except you can't reset the enable_night_mode_by_wfm & invert_fb flags without resorting to the debug procfs knob,
//       because the ioctl handler is one-way for those two...
//       c.f., hwtcon_fb_ioctl @ drivers/misc/mediatek/hwtcon/hwtcon_fb.c
//       c.f., debug_enable_nightmode @ drivers/misc/mediatek/hwtcon/hwtcon_debug.c
// NOTE: Speaking of, the invert_fb flag can only be toggled on its own via procfs:
//       echo "night_mode 4" > /proc/hwtcon/cmd for on, 0 for off.
#define HWTCON_SET_NIGHTMODE _IOW(HWTCON_IOCTL_MAGIC_NUMBER, 0x26, int32_t)

/* Set the mapping between waveform types and waveform mode index */
#define HWTCON_SET_WAVEFORM_MODES _IOW(HWTCON_IOCTL_MAGIC_NUMBER, 0x2B, struct hwtcon_waveform_modes)

/* Set the temperature for screen updates.
 * If temperature specified is TEMP_USE_SENSOR,
 * use the temperature read from the temperature sensor.
 * Otherwise use the temperature specified
 */
#define HWTCON_SET_TEMPERATURE _IOW(HWTCON_IOCTL_MAGIC_NUMBER, 0x2C, int32_t)

// NOTE: Unimplemented
#define HWTCON_SET_AUTO_UPDATE_MODE _IOW(HWTCON_IOCTL_MAGIC_NUMBER, 0x2D, uint32_t)

/* Get the temperature currently used for screen updates.
 * If the temperature set by command FB_SET_TEMPERATURE
 * is not equal to TEMP_USE_SENSOR,
 * return that temperature value.
 * Otherwise, return the temperature read from the temperature sensor
 */
#define HWTCON_GET_TEMPERATURE _IOR(HWTCON_IOCTL_MAGIC_NUMBER, 0x38, int32_t)

/* Send update info to update the Eink panel display */
#define HWTCON_SEND_UPDATE _IOW(HWTCON_IOCTL_MAGIC_NUMBER, 0x2E, struct hwtcon_update_data)

/* Wait until the specified send_update request
 * (specified by hwtcon_update_marker_data) is
 * submitted to HWTCON to display or timeout (5 seconds)
 */
// NOTE: Backend support may not be entirely implemented (MARKER_V2_ENABLE appears to be unset)
#define HWTCON_WAIT_FOR_UPDATE_SUBMISSION _IOW(HWTCON_IOCTL_MAGIC_NUMBER, 0x37, uint32_t)

/* Wait until the specified send_update request
 * (specified by hwtcon_update_marker_data) is
 * already completed (Eink panel updated) or timeout (5 seconds)
 */
// NOTE: Handler actually takes a pointer to a simple uint32_t!
#define HWTCON_WAIT_FOR_UPDATE_COMPLETE _IOWR(HWTCON_IOCTL_MAGIC_NUMBER, 0x2F, struct hwtcon_update_marker_data)

/* Copy the content of the working buffer to user space */
#define HWTCON_GET_WORK_BUFFER _IOWR(HWTCON_IOCTL_MAGIC_NUMBER, 0x34, unsigned long)

/* Set the power down delay so the driver won't shut down the HWTCON immediately
 * after all the updates are done.
 * Instead it will wait until the "DELAY" time has elapsed to skip the
 * powerdown and powerup sequences if an update comes before that.
 */
// NOTE: Default is 500ms, -1 means never power down.
#define HWTCON_SET_PWRDOWN_DELAY _IOW(HWTCON_IOCTL_MAGIC_NUMBER, 0x30, int32_t)

/* Get the power down delay set in HWTCON_SET_PWRDOWN_DELAY command */
#define HWTCON_GET_PWRDOWN_DELAY _IOR(HWTCON_IOCTL_MAGIC_NUMBER, 0x31, int32_t)

/* Pause updating the screen.
 * Any HWTCON_SEND_UPDATE request will be discarded.
 */
// NOTE: Argument is irrelevant
#define HWTCON_SET_PAUSE _IOW(HWTCON_IOCTL_MAGIC_NUMBER, 0x33, uint32_t)

/* Resume updating the screen. */
// NOTE: Argument is irrelevant
#define HWTCON_SET_RESUME _IOW(HWTCON_IOCTL_MAGIC_NUMBER, 0x35, uint32_t)

/* Get the screen updating flag set by HWTCON_SET_PAUSE or HWTCON_SET_RESUME */
#define HWTCON_GET_PAUSE _IOW(HWTCON_IOCTL_MAGIC_NUMBER, 0x34, uint32_t)

// Introduced with Kaleido devices
// Kernel representation of the CFA_MODE constants, for use with the two ioctls below (from hwtcon_fb.h)
#define HWTCON_CFA_MODE_NONE        0
#define HWTCON_CFA_MODE_EINK_G1     1     // eink color enhance gain=1
#define HWTCON_CFA_MODE_EINK_AIE_S4 2     // eink AIE algorithm S4
#define HWTCON_CFA_MODE_EINK_AIE_S7 3     // eink AIE algorithm S7
#define HWTCON_CFA_MODE_EINK_AIE_S9 4     // eink AIE algorithm S9
#define HWTCON_CFA_MODE_EINK_G0     5     // eink color enhance gain=0
#define HWTCON_CFA_MODE_EINK_G2     6     // eink color enhance gain=2
#define HWTCON_CFA_MODE_NTX         10    // color mapping by epdfbdc
#define HWTCON_CFA_MODE_NTX_SF      11    // simple fast color mapping
#define HWTCON_CFA_MODE_EINK_NORMAL HWTCON_CFA_MODE_EINK_G1
#define HWTCON_CFA_MODE_DEFAULT     HWTCON_CFA_MODE_EINK_NORMAL

/* Get the current default CFA mode */
// NOTE: Arg is a pointer to an uint32_t
// NOTE: This is the CFA_MODE that is used when you do *NOT* set an HWTCON_FLAG_CFA_* flag in the update request.
//       Defaults to EINK_NORMAL.
#define HWTCON_SET_CFA_MODE _IOW(HWTCON_IOCTL_MAGIC_NUMBER, 0x50, uint32_t)
/* Set the current default CFA mode */
// NOTE: Arg is a pointer to an uint32_t
#define HWTCON_GET_CFA_MODE _IOW(HWTCON_IOCTL_MAGIC_NUMBER, 0x51, uint32_t)

#define HWTCON_GET_PANEL_INFO _IOR(HWTCON_IOCTL_MAGIC_NUMBER, 0x130, struct hwtcon_panel_info)

#endif /* __HWTCON_IOCTL_CMD_H__ */
