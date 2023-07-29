/*
 * Copyright (C) 2004-2015 Freescale Semiconductor, Inc. All Rights Reserved.
 *
 * NOTE: Upstream kernels available here: https://github.com/kobolabs/Kobo-Reader/tree/master/hw
 * - slightly modified (commented out include of fb.h) for Lua integration
 * - Frankensteined w/ Mark 6 stuff -- NiLuJe
 * - Frankensteined w/ Mark 7 stuff -- NiLuJe
 */

/*
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */

/*
 * @file uapi/linux/mxcfb.h
 *
 * @brief Global header file for the MXC frame buffer
 *
 * @ingroup Framebuffer
 */
#ifndef __ASM_ARCH_MXCFB_H__
#define __ASM_ARCH_MXCFB_H__

#ifndef __KERNEL__
#	include <stdint.h>
#endif

#include <linux/fb.h>

#define FB_SYNC_OE_LOW_ACT   0x80000000
#define FB_SYNC_CLK_LAT_FALL 0x40000000
#define FB_SYNC_DATA_INVERT  0x20000000
#define FB_SYNC_CLK_IDLE_EN  0x10000000
#define FB_SYNC_SHARP_MODE   0x08000000
#define FB_SYNC_SWAP_RGB     0x04000000
/* Mark 7 */
#define FB_ACCEL_TRIPLE_FLAG 0x00000000
#define FB_ACCEL_DOUBLE_FLAG 0x00000001

struct mxcfb_gbl_alpha
{
	int enable;
	int alpha;
};

struct mxcfb_loc_alpha
{
	int           enable;
	int           alpha_in_pixel;
	unsigned long alpha_phy_addr0;
	unsigned long alpha_phy_addr1;
};

struct mxcfb_color_key
{
	int      enable;
	uint32_t color_key;
};

struct mxcfb_pos
{
	uint16_t x;
	uint16_t y;
};

struct mxcfb_gamma
{
	int enable;
	int constk[16];
	int slopek[16];
};

/* Mark 7 */
struct mxcfb_gpu_split_fmt
{
	struct fb_var_screeninfo var;
	unsigned long            offset;
};

struct mxcfb_rect
{
	uint32_t top;
	uint32_t left;
	uint32_t width;
	uint32_t height;
};

#define GRAYSCALE_8BIT          0x1
#define GRAYSCALE_8BIT_INVERTED 0x2

/* Mark 7 */
#define GRAYSCALE_4BIT          0x3
#define GRAYSCALE_4BIT_INVERTED 0x4

#define AUTO_UPDATE_MODE_REGION_MODE    0
#define AUTO_UPDATE_MODE_AUTOMATIC_MODE 1

#define UPDATE_SCHEME_SNAPSHOT        0
#define UPDATE_SCHEME_QUEUE           1
#define UPDATE_SCHEME_QUEUE_AND_MERGE 2

#define UPDATE_MODE_PARTIAL 0x0
#define UPDATE_MODE_FULL    0x1

/*
 * Those are sneaked in in drivers/video/mxc/mxc_epdc_fb.c
 * Or drivers/video/fbdev/mxc/mxc_epdc_v2_fb.c since Mark 7.
 * NOTE: They only appeared on Mk. 5, so, on earlier devices,
 *       I wouldn't rely on anything other than AUTO, DU & GC16...
 */
#define NTX_WFM_MODE_INIT    0
#define NTX_WFM_MODE_DU      1
#define NTX_WFM_MODE_GC16    2
#define NTX_WFM_MODE_GC4     3
#define NTX_WFM_MODE_A2      4
#define NTX_WFM_MODE_GL16    5
#define NTX_WFM_MODE_GLR16   6
#define NTX_WFM_MODE_GLD16   7
/* Mark 9 */
#define NTX_WFM_MODE_DU4     8
#define NTX_WFM_MODE_GCK16   9
#define NTX_WFM_MODE_GLKW16  10
#define NTX_WFM_MODE_TOTAL   11
/* Match 'em to the Kindle ones, for sanity's sake... */
#define WAVEFORM_MODE_INIT   NTX_WFM_MODE_INIT
#define WAVEFORM_MODE_DU     NTX_WFM_MODE_DU
#define WAVEFORM_MODE_GC16   NTX_WFM_MODE_GC16
#define WAVEFORM_MODE_GC4    NTX_WFM_MODE_GC4
/*
#define WAVEFORM_MODE_GC16_FAST			0xFFFF
*/
#define WAVEFORM_MODE_A2     NTX_WFM_MODE_A2
#define WAVEFORM_MODE_GL16   NTX_WFM_MODE_GL16
/*
#define WAVEFORM_MODE_GL16_FAST			0xFFFF
#define WAVEFORM_MODE_DU4			0xFFFF
*/
#define WAVEFORM_MODE_REAGL  NTX_WFM_MODE_GLR16
#define WAVEFORM_MODE_REAGLD NTX_WFM_MODE_GLD16
/*
#define WAVEFORM_MODE_GL4			0xFFFF
#define WAVEFORM_MODE_GL16_INV			0xFFFF
*/

/* Nickel */
//#define WAVEFORM_MODE_GL16			NTX_WFM_MODE_GL16
#define WAVEFORM_MODE_GLR32 NTX_WFM_MODE_GLR16

/* Mark 7 */
#define WAVEFORM_MODE_GLR16 NTX_WFM_MODE_GLR16
#define WAVEFORM_MODE_GLD16 NTX_WFM_MODE_GLD16

/* Mark 9 */
#define WAVEFORM_MODE_DU4    NTX_WFM_MODE_DU4
#define WAVEFORM_MODE_GCK16  NTX_WFM_MODE_GCK16
#define WAVEFORM_MODE_GLKW16 NTX_WFM_MODE_GLKW16

#define WAVEFORM_MODE_AUTO 257

#define TEMP_USE_AMBIENT 0x1000

#define EPDC_FLAG_ENABLE_INVERSION 0x01
#define EPDC_FLAG_FORCE_MONOCHROME 0x02

/* Aura */
#define EPDC_FLAG_USE_CMAP 0x04

#define EPDC_FLAG_USE_ALT_BUFFER 0x100

/* Aura ONLY */
#define EPDC_FLAG_USE_AAD 0x1000

/* Mark 7 */
#define EPDC_FLAG_TEST_COLLISION   0x200
#define EPDC_FLAG_GROUP_UPDATE     0x400
/* Nickel: only for alyssum and above (i.e., Mk. 6) */
#define EPDC_FLAG_USE_DITHERING_Y1 0x2000
#define EPDC_FLAG_USE_DITHERING_Y4 0x4000
#define EPDC_FLAG_USE_REGAL        0x8000

/* Nickel (gone on Mk. 7) */
#define EPDC_FLAG_USE_DITHERING_NTX_D8 0x100000

/* Mark 7 */
enum mxcfb_dithering_mode
{
	EPDC_FLAG_USE_DITHERING_PASSTHROUGH = 0x0,
	EPDC_FLAG_USE_DITHERING_FLOYD_STEINBERG,
	EPDC_FLAG_USE_DITHERING_ATKINSON,
	EPDC_FLAG_USE_DITHERING_ORDERED,
	EPDC_FLAG_USE_DITHERING_QUANT_ONLY,
	EPDC_FLAG_USE_DITHERING_MAX,
};

#define FB_POWERDOWN_DISABLE -1

/* Mark 7 */
#define FB_TEMP_AUTO_UPDATE_DISABLE -1

/*
 * NOTE: Mark 7 renamed some of these to maintain backwards compatibility while providing a newer interface
 *       Was: mxcfb_alt_buffer_data
 *       Nickel: imx5/imx6
 */
struct mxcfb_alt_buffer_data_ntx
{
	void*             virt_addr;
	uint32_t          phys_addr;
	uint32_t          width;             /* width of entire buffer */
	uint32_t          height;            /* height of entire buffer */
	struct mxcfb_rect alt_update_region; /* region within buffer to update */
};

/*
 * NOTE: Was: mxcfb_alt_buffer_data_org (appeared w/ the Aura)
 *       Nickel: mxcfb_alt_buffer_data_v2
 */
struct mxcfb_alt_buffer_data
{
	uint32_t          phys_addr;
	uint32_t          width;             /* width of entire buffer */
	uint32_t          height;            /* height of entire buffer */
	struct mxcfb_rect alt_update_region; /* region within buffer to update */
};

/*
 * Mark 7
 * NOTE: Was: mxcfb_update_data
 * Nickel: imx5/imx6
 */
// mxcfb_update_data v1 for NTX linux since from mx50/mx6sl .
struct mxcfb_update_data_v1_ntx
{
	struct mxcfb_rect                update_region;
	uint32_t                         waveform_mode;
	uint32_t                         update_mode;
	uint32_t                         update_marker;
	int                              temp;
	unsigned int                     flags;
	struct mxcfb_alt_buffer_data_ntx alt_buffer_data;
};

/* NOTE: Was: mxcfb_update_data_org (appeared w/ the Aura) */
// mxcfb_update_data v1 since from mx50/mx6sl .
struct mxcfb_update_data_v1
{
	struct mxcfb_rect            update_region;
	uint32_t                     waveform_mode;
	uint32_t                     update_mode;
	uint32_t                     update_marker;
	int                          temp;
	unsigned int                 flags;
	struct mxcfb_alt_buffer_data alt_buffer_data;
};

/*
 * Mark 7
 * Nickel: mxcfb_update_data_v2
 */
// mxcfb_update_data v2 since from mx7d .
#define mxcfb_update_data_v2 mxcfb_update_data
struct mxcfb_update_data
{
	struct mxcfb_rect            update_region;
	uint32_t                     waveform_mode;
	uint32_t                     update_mode;
	uint32_t                     update_marker;
	int                          temp;
	unsigned int                 flags;
	int                          dither_mode;
	int                          quant_bit;
	struct mxcfb_alt_buffer_data alt_buffer_data;
};

/* Mark 7 */
struct mxcfb_update_marker_data
{
	uint32_t update_marker;
	uint32_t collision_test;
};

/* Mark 7 */
#define WFM_ENABLE_AA  1
#define WFM_ENABLE_AAD 1

/*
 * Structure used to define waveform modes for driver
 * Needed for driver to perform auto-waveform selection
 */
/*
 * NOTE: Once again, Mark 7 renamed some stuff
 *       Was mxcfb_waveform_modes,
 *       we've split it into each variant for strace's benefit.
 */
struct mxcfb_waveform_modes
{
	int mode_init;
	int mode_du;
	int mode_gc4;
	int mode_gc8;
	int mode_gc16;
	int mode_gc32;
};

struct mxcfb_waveform_modes_mk5
{
	int mode_init;
	int mode_du;
	int mode_gc4;
	int mode_gc8;
	int mode_gc16;
	int mode_gc32;

	int mode_a2;
	int mode_gl16;
	/*
         * reagl_flow
         */
	int mode_aa;
	int mode_aad;
};

// NOTE: Same total size as the mk5 variant,
//       but gl16 & a2 are inverted... -_-".
struct mxcfb_waveform_modes_mk7
{
	int mode_init;
	int mode_du;
	int mode_gc4;
	int mode_gc8;
	int mode_gc16;
	int mode_gc32;
	int mode_gl16;
	int mode_a2;

	int mode_aa;
	int mode_aad;
};

struct mxcfb_waveform_modes_mk9
{
	int mode_init;
	int mode_du;
	int mode_gc4;
	int mode_gc8;
	int mode_gc16;
	int mode_gc32;
	int mode_gl16;
	int mode_a2;

	int mode_aa;
	int mode_aad;
	int mode_du4;
	int mode_gck16;
	int mode_glkw16;
};

/* Mark 7 */
/*
 * Structure used to define a 5*3 matrix of parameters for
 * setting IPU DP CSC module related to this framebuffer.
 */
struct mxcfb_csc_matrix
{
	int param[5][3];
};

#define MXCFB_WAIT_FOR_VSYNC  _IOW('F', 0x20, uint32_t)
#define MXCFB_SET_GBL_ALPHA   _IOW('F', 0x21, struct mxcfb_gbl_alpha)
#define MXCFB_SET_CLR_KEY     _IOW('F', 0x22, struct mxcfb_color_key)
#define MXCFB_SET_OVERLAY_POS _IOWR('F', 0x24, struct mxcfb_pos)
#define MXCFB_GET_FB_IPU_CHAN _IOR('F', 0x25, uint32_t)
#define MXCFB_SET_LOC_ALPHA   _IOWR('F', 0x26, struct mxcfb_loc_alpha)
#define MXCFB_SET_LOC_ALP_BUF _IOW('F', 0x27, unsigned long)
#define MXCFB_SET_GAMMA       _IOW('F', 0x28, struct mxcfb_gamma)
#define MXCFB_GET_FB_IPU_DI   _IOR('F', 0x29, uint32_t)
#define MXCFB_GET_DIFMT       _IOR('F', 0x2A, uint32_t)
#define MXCFB_GET_FB_BLANK    _IOR('F', 0x2B, uint32_t)
// NOTE: Not actually implemented for EPDCs, clashes w/ MXCFB_SET_TEMPERATURE, which *is* implemented.
//#define MXCFB_SET_DIFMT       _IOW('F', 0x2C, uint32_t)

/* Mark 7 */
#define MXCFB_CSC_UPDATE        _IOW('F', 0x2D, struct mxcfb_csc_matrix)
#define MXCFB_SET_GPU_SPLIT_FMT _IOW('F', 0x2F, struct mxcfb_gpu_split_fmt)
// NOTE: Not actually implemented for EPDCs, these clash w/ the PWRDOWN_DELAY ones, which *are* implemented.
//#define MXCFB_SET_PREFETCH      _IOW('F', 0x30, int)
//#define MXCFB_GET_PREFETCH      _IOR('F', 0x31, int)

/* IOCTLs for E-ink panel updates */
#define MXCFB_SET_WAVEFORM_MODES     _IOW('F', 0x2B, struct mxcfb_waveform_modes)
#define MXCFB_SET_WAVEFORM_MODES_MK5 _IOW('F', 0x2B, struct mxcfb_waveform_modes_mk5)
#define MXCFB_SET_WAVEFORM_MODES_MK7 _IOW('F', 0x2B, struct mxcfb_waveform_modes_mk7)
#define MXCFB_SET_WAVEFORM_MODES_MK9 _IOW('F', 0x2B, struct mxcfb_waveform_modes_mk9)

#define MXCFB_SET_TEMPERATURE      _IOW('F', 0x2C, int32_t)
#define MXCFB_SET_AUTO_UPDATE_MODE _IOW('F', 0x2D, uint32_t)

/* Mark 7 */
/* NOTE: Was MXCFB_SEND_UPDATE before Mark 7! Is still that in Nickel's header */
#define MXCFB_SEND_UPDATE_V1_NTX _IOW('F', 0x2E, struct mxcfb_update_data_v1_ntx)
/* NOTE: Was: MXCFB_SEND_UPDATE_ORG before Mark 7 (appeared w/ Aura)! */
#define MXCFB_SEND_UPDATE_V1     _IOW('F', 0x2E, struct mxcfb_update_data_v1)
/* NOTE: -> MXCFB_SEND_UPDATE_V2 ! */
#define MXCFB_SEND_UPDATE        _IOW('F', 0x2E, struct mxcfb_update_data)

/* Mark 7 */
#define MXCFB_SEND_UPDATE_V2 _IOW('F', 0x2E, struct mxcfb_update_data)
/* NOTE: -> MXCFB_WAIT_FOR_UPDATE_COMPLETE_V3 ! */
#define MXCFB_WAIT_FOR_UPDATE_COMPLETE                                                                                   \
	_IOWR('F', 0x2F, struct mxcfb_update_marker_data)    // mx7d/mx6ull/mx6sll interface .
/* NOTE: Nickel: MXCFB_WAIT_FOR_UPDATE_COMPLETE_V2 */
#define MXCFB_WAIT_FOR_UPDATE_COMPLETE_V3                                                                                \
	_IOWR('F', 0x2F, struct mxcfb_update_marker_data)    // mx7d/mx6ull/mx6sll interface .
/* NOTE: Was MXCFB_WAIT_FOR_UPDATE_COMPLETE before Mark 7! Is still that in Nickel's header */
#define MXCFB_WAIT_FOR_UPDATE_COMPLETE_V1 _IOW('F', 0x2F, uint32_t)                            // mx50/NTX interface .
#define MXCFB_WAIT_FOR_UPDATE_COMPLETE_V2 _IOWR('F', 0x35, struct mxcfb_update_marker_data)    // mx6sl BSP interface .

#define MXCFB_SET_PWRDOWN_DELAY _IOW('F', 0x30, int32_t)
#define MXCFB_GET_PWRDOWN_DELAY _IOR('F', 0x31, int32_t)
#define MXCFB_SET_UPDATE_SCHEME _IOW('F', 0x32, uint32_t)

/* Went poof w/ Mark 7 */
#define MXCFB_SET_MERGE_ON_WAVEFORM_MISMATCH _IOW('F', 0x37, int32_t)

/* Mark 7 */
#define MXCFB_GET_WORK_BUFFER             _IOWR('F', 0x34, unsigned long)
#define MXCFB_DISABLE_EPDC_ACCESS         _IO('F', 0x35)
#define MXCFB_ENABLE_EPDC_ACCESS          _IO('F', 0x36)
#define MXCFB_SET_TEMP_AUTO_UPDATE_PERIOD _IOW('F', 0x37, int32_t)

#endif    // __ASM_ARCH_MXCFB_H__
