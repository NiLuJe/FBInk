/*
 * Copyright 2004-2013 Freescale Semiconductor, Inc. All Rights Reserved.
 */

/*
 * The code contained herein is licensed under the GNU Lesser General
 * Public License.  You may obtain a copy of the GNU Lesser General
 * Public License Version 2.1 or later at the following locations:
 *
 * http://www.opensource.org/licenses/lgpl-license.html
 * http://www.gnu.org/copyleft/lgpl.html
 */

/*
 * NOTE: Upstream kernels available here: https://github.com/pocketbook/Platform_MX6
 * - slightly modified (commented out include of fb.h) for Lua integration
 * - Attempted to frankenstein the two variants together like we do on other platforms...
*/

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

// NOTE: PB631
#if defined(CONFIG_ANDROID) || defined(ANDROID) //[
#else //][ !CONFIG_ANDROID||!ANDROID
	#define MX50_IOCTL_IF	1
#endif //] !CONFIG_ANDROID

#define FB_SYNC_OE_LOW_ACT	0x80000000
#define FB_SYNC_CLK_LAT_FALL	0x40000000
#define FB_SYNC_DATA_INVERT	0x20000000
#define FB_SYNC_CLK_IDLE_EN	0x10000000
#define FB_SYNC_SHARP_MODE	0x08000000
#define FB_SYNC_SWAP_RGB	0x04000000
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

#define GRAYSCALE_8BIT				0x1
#define GRAYSCALE_8BIT_INVERTED			0x2
#define GRAYSCALE_4BIT                          0x3
#define GRAYSCALE_4BIT_INVERTED                 0x4

#define AUTO_UPDATE_MODE_REGION_MODE		0
#define AUTO_UPDATE_MODE_AUTOMATIC_MODE		1

#define UPDATE_SCHEME_SNAPSHOT			0
#define UPDATE_SCHEME_QUEUE			1
#define UPDATE_SCHEME_QUEUE_AND_MERGE		2

#define UPDATE_MODE_PARTIAL			0x0
#define UPDATE_MODE_FULL			0x1

// NOTE: PB631
#define UPDATE_MODE_PARTIALHQ			0x2
// PARTIALHQ || PARTIAL + GC16HQ => FULL + AA
#define UPDATE_MODE_FULLHQ			0x3
// FULLHQ || FULL + GC16HQ => FULL + AAD

// NOTE: Buried in drivers/video/mxc/mxc_epdc_fb.c
//       PB only defines DU, A2, A2IN & A2OUT
#define EPDC_WFTYPE_INIT			0
#define EPDC_WFTYPE_DU				1
#define EPDC_WFTYPE_GC16			2
#define EPDC_WFTYPE_GC4				3
#define EPDC_WFTYPE_A2				4
#define EPDC_WFTYPE_GL16			5  // On B288 this is very fast, but flickers, interferes with HWROT.
#define EPDC_WFTYPE_A2IN			6
#define EPDC_WFTYPE_A2OUT			7
#define EPDC_WFTYPE_DU4				8  // !B888
#define EPDC_WFTYPE_AA				9  // !B288
#define EPDC_WFTYPE_AAD				10 // !B288
#define EPDC_WFTYPE_GS16			14  // B288 only. Should sit between GC16 and GL16.
#define EPDC_WFTYPE_GC16HQ			15  // !B288
// NOTE: Alias that to our usual constant names...
#define WAVEFORM_MODE_INIT			EPDC_WFTYPE_INIT
#define WAVEFORM_MODE_DU			EPDC_WFTYPE_DU
#define WAVEFORM_MODE_GC16			EPDC_WFTYPE_GC16
#define WAVEFORM_MODE_GC4			EPDC_WFTYPE_GC4
#define WAVEFORM_MODE_A2			EPDC_WFTYPE_A2
#define WAVEFORM_MODE_GL16			EPDC_WFTYPE_GL16
#define WAVEFORM_MODE_A2IN			EPDC_WFTYPE_A2IN
#define WAVEFORM_MODE_A2OUT			EPDC_WFTYPE_A2OUT
#define WAVEFORM_MODE_DU4			EPDC_WFTYPE_DU4
#define WAVEFORM_MODE_REAGL			EPDC_WFTYPE_AA
#define WAVEFORM_MODE_REAGLD			EPDC_WFTYPE_AAD
#define WAVEFORM_MODE_GS16			EPDC_WFTYPE_GS16
#define WAVEFORM_MODE_GC16HQ			EPDC_WFTYPE_GC16HQ

#define WAVEFORM_MODE_AUTO			257 // !B288 (Ouch. That one hurts.)

#define TEMP_USE_AMBIENT			0x1000

#define EPDC_FLAG_ENABLE_INVERSION		0x01
#define EPDC_FLAG_FORCE_MONOCHROME		0x02
#define EPDC_FLAG_USE_CMAP			0x04
#define EPDC_FLAG_USE_ALT_BUFFER		0x100
#define EPDC_FLAG_TEST_COLLISION		0x200
#define EPDC_FLAG_GROUP_UPDATE			0x400

// NOTE: PB631
#define EPDC_FLAG_USE_AAD			0x1000

#define EPDC_FLAG_USE_DITHERING_Y1		0x2000
#define EPDC_FLAG_USE_DITHERING_Y4		0x4000

// NOTE: PB631
#define EPDC_FLAG_USE_DITHERING_NTX_D8		0x100000

#define FB_POWERDOWN_DISABLE			-1

// NOTE: The virt_addr mess that we mercifully don't have to deal with is from PB631
struct mxcfb_alt_buffer_data {
//#if defined(CONFIG_ANDROID) || defined (ANDROID)//[
//#else//][!CONFIG_ANDROID
//	void *virt_addr;
//#endif//]!CONFIG_ANDROID
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
	// NOTE: PB631
	int mode_aa;
	int mode_aad;
	int mode_gl16;
	int mode_a2;
	int mode_a2in;
	int mode_a2out;
};

/*
 * Structure used to define a 5*3 matrix of parameters for
 * setting IPU DP CSC module related to this framebuffer.
 */
struct mxcfb_csc_matrix {
	int param[5][3];
};

#define MXCFB_WAIT_FOR_VSYNC	_IOW('F', 0x20, u_int32_t)
#define MXCFB_SET_GBL_ALPHA     _IOW('F', 0x21, struct mxcfb_gbl_alpha)
#define MXCFB_SET_CLR_KEY       _IOW('F', 0x22, struct mxcfb_color_key)
#define MXCFB_SET_OVERLAY_POS   _IOWR('F', 0x24, struct mxcfb_pos)
#define MXCFB_GET_FB_IPU_CHAN 	_IOR('F', 0x25, u_int32_t)
#define MXCFB_SET_LOC_ALPHA     _IOWR('F', 0x26, struct mxcfb_loc_alpha)
#define MXCFB_SET_LOC_ALP_BUF    _IOW('F', 0x27, unsigned long)
#define MXCFB_SET_GAMMA	       _IOW('F', 0x28, struct mxcfb_gamma)
#define MXCFB_GET_FB_IPU_DI 	_IOR('F', 0x29, u_int32_t)
#define MXCFB_GET_DIFMT	       _IOR('F', 0x2A, u_int32_t)
#define MXCFB_GET_FB_BLANK     _IOR('F', 0x2B, u_int32_t)
#define MXCFB_SET_DIFMT		_IOW('F', 0x2C, u_int32_t)

// NOTE: PB631
#define MXCFB_ENABLE_VSYNC_EVENT	_IOW('F', 0x33, int32_t)

#define MXCFB_CSC_UPDATE	_IOW('F', 0x2D, struct mxcfb_csc_matrix)

/* IOCTLs for E-ink panel updates */
#define MXCFB_SET_WAVEFORM_MODES	_IOW('F', 0x2B, struct mxcfb_waveform_modes)
#define MXCFB_SET_TEMPERATURE		_IOW('F', 0x2C, int32_t)
#define MXCFB_SET_AUTO_UPDATE_MODE	_IOW('F', 0x2D, __u32)
#define MXCFB_SEND_UPDATE		_IOW('F', 0x2E, struct mxcfb_update_data)

// NOTE: PB
#define MXCFB_WAIT_FOR_UPDATE_COMPLETE _IOWR('F', 0x2F, struct mxcfb_update_marker_data)

// NOTE: PB631
#ifdef MX50_IOCTL_IF//[
#define MXCFB_WAIT_FOR_UPDATE_COMPLETE_PB	_IOW('F', 0x2F, __u32)
#define MXCFB_WAIT_FOR_UPDATE_COMPLETE_PB631_V2 _IOWR('F', 0x35, struct mxcfb_update_marker_data)
#else //][!MX50_IOCTL_IF
#define MXCFB_WAIT_FOR_UPDATE_COMPLETE_ANDROID _IOWR('F', 0x35, struct mxcfb_update_marker_data)
#endif//] MX50_IOCTL_IF

// NOTE: Apparently, everything is terrible, and, while kernels appear to always expect the MXCFB_WAIT_FOR_UPDATE_COMPLETE_PB
//       command, *some* kernels will apparently try to write back as if it were MXCFB_WAIT_FOR_UPDATE_COMPLETE...
//       The workaround is to actually setup a mxcfb_update_marker_data struct but use MXCFB_WAIT_FOR_UPDATE_COMPLETE_PB.
//       >_<".
//       c.f., https://github.com/koreader/koreader-base/pull/1188 & https://github.com/koreader/koreader/pull/6669

// NOTE: This only probes whether the EPDC is busy, we use this to detect B288.
#define EPDC_GET_UPDATE_STATE		_IOR('F', 0x55, __u32)

#define MXCFB_SET_PWRDOWN_DELAY		_IOW('F', 0x30, int32_t)
#define MXCFB_GET_PWRDOWN_DELAY		_IOR('F', 0x31, int32_t)
#define MXCFB_SET_UPDATE_SCHEME		_IOW('F', 0x32, __u32)
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
int mxc_elcdif_frame_addr_setup(dma_addr_t phys);
void mxcfb_elcdif_register_mode(const struct fb_videomode *modedb,
		int num_modes, int dev_mode);

#endif				/* __KERNEL__ */
#endif
