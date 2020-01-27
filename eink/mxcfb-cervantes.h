/*
 * Copyright 2004-2013 Freescale Semiconductor, Inc. All Rights Reserved.
 *
 * Unified header for BQ Cervantes/Fnac Touchlight devices
 * Heavily based on mxcfb-kobo.h, with some notes from bq kernel sources.
 * c.f., https://blog.bq.com/es/bq-ereaders-developers-program/
 * When I'm not using the proper model names,
 * C1 means the oldest kernel, and C4 the latest.
 *
 * The code contained herein is licensed under the GNU Lesser General
 * Public License.  You may obtain a copy of the GNU Lesser General
 * Public License Version 2.1 or later at the following locations:
 *
 * http://www.opensource.org/licenses/lgpl-license.html
 * http://www.gnu.org/copyleft/lgpl.html
 */

// NOTE: Upstream kernels available here: https://github.com/bq/linux-e60qh2/tree/e60qh2 (as well as the other linux- repos).
//       See also https://blog.bq.com/es/bq-ereaders-developers-program/

/*
 * @file arch-mxc/   mxcfb.h
 *
 * @brief Global header file for the MXC Frame buffer
 *
 * @ingroup Framebuffer
 */
#ifndef __ASM_ARCH_MXCFB_H__
#define __ASM_ARCH_MXCFB_H__

//#include <linux/fb.h>

#define FB_SYNC_OE_LOW_ACT	0x80000000
#define FB_SYNC_CLK_LAT_FALL	0x40000000
#define FB_SYNC_DATA_INVERT	0x20000000
#define FB_SYNC_CLK_IDLE_EN	0x10000000
#define FB_SYNC_SHARP_MODE	0x08000000
#define FB_SYNC_SWAP_RGB	0x04000000

// Cervantes 2013/Fnac Touch Light and higher
#define FB_ACCEL_TRIPLE_FLAG	0x00000000
#define FB_ACCEL_DOUBLE_FLAG	0x00000001

struct mxcfb_gbl_alpha {
	int enable;
	int alpha;
};

struct mxcfb_loc_alpha {
	int enable;
	int alpha_in_pixel;
	unsigned long alpha_phy_addr0;
	unsigned long alpha_phy_addr1;
};

struct mxcfb_color_key {
	int enable;
	__u32 color_key;
};

struct mxcfb_pos {
	__u16 x;
	__u16 y;
};

struct mxcfb_gamma {
	int enable;
	int constk[16];
	int slopek[16];
};

struct mxcfb_rect {
	__u32 top;
	__u32 left;
	__u32 width;
	__u32 height;
};

#define GRAYSCALE_8BIT			0x1
#define GRAYSCALE_8BIT_INVERTED		0x2
// C2+
#define GRAYSCALE_4BIT                  0x3
#define GRAYSCALE_4BIT_INVERTED         0x4

#define AUTO_UPDATE_MODE_REGION_MODE	0
#define AUTO_UPDATE_MODE_AUTOMATIC_MODE 1

#define UPDATE_SCHEME_SNAPSHOT		0
#define UPDATE_SCHEME_QUEUE		1
#define UPDATE_SCHEME_QUEUE_AND_MERGE	2

#define UPDATE_MODE_PARTIAL		0x0
#define UPDATE_MODE_FULL		0x1

/* Those are sneaked in in drivers/video/mxc/mxc_epdc_fb.c, same as Kobo */
#define NTX_WFM_MODE_INIT		0
#define NTX_WFM_MODE_DU			1
#define NTX_WFM_MODE_GC16		2
#define NTX_WFM_MODE_GC4		3
#define NTX_WFM_MODE_A2			4
#define NTX_WFM_MODE_GL16		5
#define NTX_WFM_MODE_GLR16		6
#define NTX_WFM_MODE_GLD16		7
#define NTX_WFM_MODE_TOTAL		8

/* Match 'em to the Kindle ones, for sanity's sake... */
#define WAVEFORM_MODE_INIT      	NTX_WFM_MODE_INIT
#define WAVEFORM_MODE_DU        	NTX_WFM_MODE_DU
#define WAVEFORM_MODE_GC4      		NTX_WFM_MODE_GC4
#define WAVEFORM_MODE_GC16		NTX_WFM_MODE_GC16
#define WAVEFORM_MODE_A2        	NTX_WFM_MODE_A2
#define WAVEFORM_MODE_GL16      	NTX_WFM_MODE_GL16

// for Cervantes 2013
#define WAVEFORM_MODE_REAGL     	NTX_WFM_MODE_GLR16
#define WAVEFORM_MODE_REAGLD    	NTX_WFM_MODE_GLD16

// for Cervantes 3+
#define WAVEFORM_MODE_AA        	NTX_WFM_MODE_GLR16
#define WAVEFORM_MODE_GLR16		NTX_WFM_MODE_GLR16
#define WAVEFORM_MODE_AAD       	NTX_WFM_MODE_GLD16
#define WAVEFORM_MODE_GLD16		NTX_WFM_MODE_GLD16

#define WAVEFORM_MODE_AUTO		257

#define TEMP_USE_AMBIENT		0x1000

#define EPDC_FLAG_ENABLE_INVERSION	0x01
#define EPDC_FLAG_FORCE_MONOCHROME	0x02
// C2+
#define EPDC_FLAG_USE_CMAP		0x04

#define EPDC_FLAG_USE_ALT_BUFFER	0x100
// C2+
#define EPDC_FLAG_TEST_COLLISION	0x200
#define EPDC_FLAG_GROUP_UPDATE		0x400
#define EPDC_FLAG_USE_DITHERING_Y1	0x2000
#define EPDC_FLAG_USE_DITHERING_Y4	0x4000

// New stuff (2016+)
#define EPDC_FLAG_USE_AAD		0x1000
#define EPDC_FLAG_USE_DITHERING_NTX_D8	0x100000

// NOTE: This is *UNSUPPORTED* on current devices!
//       We just happen to need these defined to build ;).
enum mxcfb_dithering_mode {
	EPDC_FLAG_USE_DITHERING_PASSTHROUGH = 0x0,
	EPDC_FLAG_USE_DITHERING_FLOYD_STEINBERG,
	EPDC_FLAG_USE_DITHERING_ATKINSON,
	EPDC_FLAG_USE_DITHERING_ORDERED,
	EPDC_FLAG_USE_DITHERING_QUANT_ONLY,
	EPDC_FLAG_USE_DITHERING_MAX,
};

#define FB_POWERDOWN_DISABLE			-1

struct mxcfb_alt_buffer_data {
	void *virt_addr;
	__u32 phys_addr;
	__u32 width;	/* width of entire buffer */
	__u32 height;	/* height of entire buffer */
	struct mxcfb_rect alt_update_region;	/* region within buffer to update */
};

struct mxcfb_update_data {
	struct mxcfb_rect update_region;
	__u32 waveform_mode;
	__u32 update_mode;
	__u32 update_marker;
	int temp;
	unsigned int flags;
	struct mxcfb_alt_buffer_data alt_buffer_data;
};

// NTX Shenanigans, like on the Kobo Aura.
// Apparently applies to the Cervantes 2013 & Cervantes 4, but not the Cervantes 3...
// Use the Cervantes 4 nomenclature, as it matches Kobo.
struct mxcfb_alt_buffer_data_org {
	__u32 phys_addr;
	__u32 width;	/* width of entire buffer */
	__u32 height;	/* height of entire buffer */
	struct mxcfb_rect alt_update_region;	/* region within buffer to update */
};

struct mxcfb_update_data_org {
	struct mxcfb_rect update_region;
	__u32 waveform_mode;
	__u32 update_mode;
	__u32 update_marker;
	int temp;
	unsigned int flags;
	struct mxcfb_alt_buffer_data_org alt_buffer_data;
};

// C2+
struct mxcfb_update_marker_data {
	__u32 update_marker;
	__u32 collision_test;
};

/*
 * Structure used to define waveform modes for driver
 * Needed for driver to perform auto-waveform selection
 */
struct mxcfb_waveform_modes {
	int mode_init;
	int mode_du;
	int mode_gc4;
	int mode_gc8;
	int mode_gc16;
	int mode_gc32;
	int mode_aa;    // was mode_reagl on Cervantes 2013/Fnac Touch Light
	int mode_aad;   // was mode_reagld on Cervantes 2013/Fnac Touch Light
	int mode_gl16;
	int mode_a2;
};

// Cervantes < 2013 ONLY (C1)
struct mxcfb_waveform_modes_old {
	int mode_init;
	int mode_du;
	int mode_gc4;
	int mode_gc8;
	int mode_gc16;
	int mode_gc32;
};

// C2+
/*
 * Structure used to define a 5*3 matrix of parameters for
 * setting IPU DP CSC module related to this framebuffer.
 */
struct mxcfb_csc_matrix {
	int param[5][3];
};

#define MXCFB_WAIT_FOR_VSYNC	    	_IOW('F', 0x20, u_int32_t)
#define MXCFB_SET_GBL_ALPHA         	_IOW('F', 0x21, struct mxcfb_gbl_alpha)
#define MXCFB_SET_CLR_KEY           	_IOW('F', 0x22, struct mxcfb_color_key)
#define MXCFB_SET_OVERLAY_POS       	_IOWR('F', 0x24, struct mxcfb_pos)
#define MXCFB_GET_FB_IPU_CHAN 	    	_IOR('F', 0x25, u_int32_t)
#define MXCFB_SET_LOC_ALPHA         	_IOWR('F', 0x26, struct mxcfb_loc_alpha)
#define MXCFB_SET_LOC_ALP_BUF       	_IOW('F', 0x27, unsigned long)
#define MXCFB_SET_GAMMA	            	_IOW('F', 0x28, struct mxcfb_gamma)
#define MXCFB_GET_FB_IPU_DI 	    	_IOR('F', 0x29, u_int32_t)
#define MXCFB_GET_DIFMT	           	_IOR('F', 0x2A, u_int32_t)
#define MXCFB_GET_FB_BLANK          	_IOR('F', 0x2B, u_int32_t)
#define MXCFB_SET_DIFMT		        _IOW('F', 0x2C, u_int32_t)
// C3+
#define MXCFB_ENABLE_VSYNC_EVENT	_IOW('F', 0x33, int32_t)
// C2+
#define MXCFB_CSC_UPDATE	        _IOW('F', 0x2D, struct mxcfb_csc_matrix)

/* IOCTLs for E-ink panel updates */
#define MXCFB_SET_WAVEFORM_MODES	_IOW('F', 0x2B, struct mxcfb_waveform_modes)
// C1 ONLY
#define MXCFB_SET_WAVEFORM_MODES_OLD	_IOW('F', 0x2B, struct mxcfb_waveform_modes_old)

#define MXCFB_SET_TEMPERATURE		_IOW('F', 0x2C, int32_t)
#define MXCFB_SET_AUTO_UPDATE_MODE	_IOW('F', 0x2D, __u32)
#define MXCFB_SEND_UPDATE		_IOW('F', 0x2E, struct mxcfb_update_data)
// C2 & C4 ONLY
#define MXCFB_SEND_UPDATE_ORG		_IOW('F', 0x2E, struct mxcfb_update_data_org)

#define MXCFB_WAIT_FOR_UPDATE_COMPLETE	_IOW('F', 0x2F, __u32)
// C2+
// Default if !MX50_COMPAT
#define MXCFB_WAIT_FOR_UPDATE_COMPLETE2	_IOWR('F', 0x35, struct mxcfb_update_marker_data)
// C2 ONLY
#define MXCFB_WAIT_FOR_UPDATE_COMPLETE3	_IOWR('F', 0x2F, struct mxcfb_update_marker_data)

#define MXCFB_SET_PWRDOWN_DELAY		_IOW('F', 0x30, int32_t)
#define MXCFB_GET_PWRDOWN_DELAY		_IOR('F', 0x31, int32_t)
#define MXCFB_SET_UPDATE_SCHEME		_IOW('F', 0x32, __u32)
// C1 ONLY
#define MXCFB_SET_MERGE_ON_WAVEFORM_MISMATCH	_IOW('F', 0x37, int32_t)
// C2+
#define MXCFB_GET_WORK_BUFFER		_IOWR('F', 0x34, unsigned long)

#ifdef __KERNEL__

extern struct fb_videomode mxcfb_modedb[];
extern int mxcfb_modedb_sz;

enum {
	MXC_DISP_SPEC_DEV = 0,
	MXC_DISP_DDC_DEV = 1,
};

enum {
	MXCFB_REFRESH_OFF,
	MXCFB_REFRESH_AUTO,
	MXCFB_REFRESH_PARTIAL,
};

int mxcfb_set_refresh_mode(struct fb_info *fbi, int mode,
			   struct mxcfb_rect *update_region);
// C1 ONLY
void mxcfb_register_mode(int disp_port,
		const struct fb_videomode *modedb,
		int num_modes, int dev_mode);

int mxc_elcdif_frame_addr_setup(dma_addr_t phys);

// C2+
void mxcfb_elcdif_register_mode(const struct fb_videomode *modedb,
		int num_modes, int dev_mode);

// C1 ONLY
void mxcfb_register_presetup(int disp_port,
		int (*pre_setup)(struct fb_info *info));

#endif				/* __KERNEL__ */
#endif
