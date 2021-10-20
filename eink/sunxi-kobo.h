/*
 * Allwinner SoCs display driver.
 *
 * Copyright (C) 2016 Allwinner.
 *
 * This file is licensed under the terms of the GNU General Public
 * License version 2.  This program is licensed "as is" without any
 * warranty of any kind, whether express or implied.
 */

#ifndef _DISP_INCLUDE_H_
#define _DISP_INCLUDE_H_

// NOTE: Dealing with sunxi is exhausting, so, let's pull a few different things together,
//       in order to make the experience slightly less soul crushing...

// Use userland C99 data types instead of kernel data types
#ifndef __KERNEL__
#	include <stdbool.h>
#	include <stdint.h>
#endif

//
// From "drivers/video/fbdev/sunxi/disp2/disp/de/include.h"
//
struct area_info
{
	unsigned int x_top;
	unsigned int y_top;
	unsigned int x_bottom;
	unsigned int y_bottom;
};

struct y8_area_info
{
	int              y8_fd;
	bool             enable;
	struct area_info get_region;
	unsigned int     width;     // Set by the kernel based on get_region
	unsigned int     height;    // Ditto.
	struct dma_buf*  dmabuf;    // Set by the kernel based on y8_fd
	char*            vaddr;     // Ditto.
};

enum eink_flash_mode
{
	LOCAL_MODE,
	GLOBAL_MODE,
	INIT_MODE
	//FULL_MODE = 0,
	//PARTIAL_MODE
};

/***********************************
* bit 31          : 0 -- frame can be merged, 1 -- frame cannot be merged
* bit 30 ~ bit 16 : reserved
* bit 15 ~ bit 0  : update mode
************************************/
// NOTE: Some of these constants are currently somewhat problematic,
//       c.f., https://github.com/NiLuJe/FBInk/pull/64#issuecomment-877842842
enum eink_update_mode
{
	// Waveform modes         ################
	EINK_INIT_MODE        = 0b00000000000000000000000000000001,    // 0x01
	EINK_DU_MODE          = 0b00000000000000000000000000000010,    // 0x02
	EINK_GC16_MODE        = 0b00000000000000000000000000000100,    // 0x04
	EINK_GC4_MODE         = 0b00000000000000000000000000001000,    // 0x08
	EINK_A2_MODE          = 0b00000000000000000000000000010000,    // 0x10
	EINK_GL16_MODE        = 0b00000000000000000000000000100000,    // 0x20
	EINK_GLR16_MODE       = 0b00000000000000000000000001000000,    // 0x40
	EINK_GLD16_MODE       = 0b00000000000000000000000010000000,    // 0x80
	EINK_GU16_MODE        = 0b00000000000000000000000010000100,    // 0x84 :(
	EINK_GCK16_MODE       = 0b00000000000000000000000010010000,    // 0x90 :(
	EINK_GLK16_MODE       = 0b00000000000000000000000010010100,    // 0x94 :(
	EINK_CLEAR_MODE       = 0b00000000000000000000000010001000,    // 0x88 :(
	EINK_GC4L_MODE        = 0b00000000000000000000000010001100,    // 0x8C :(
	EINK_GCC16_MODE       = 0b00000000000000000000000010100000,    // 0xA0 :(
	// Update mode (defaults to flashing/full)
	EINK_PARTIAL_MODE     = 0b00000000000000000000010000000000,    // 0x400
	// AUTO MODE: Runtime selectiion done by the e-Ink driver
	EINK_AUTO_MODE        = 0b00000000000000001000000000000000,    // 0x8000
	// Update flags                           ################
	// Simple software inversion in C
	EINK_NEGATIVE_MODE    = 0b00000000000000010000000000000000,    // 0x10000
	// REGAL MODE: Enable regal algorithms (in theory, ignored in practice).
	EINK_REGAL_MODE       = 0b00000000000010000000000000000000,    // 0x80000
	EINK_GAMMA_CORRECT    = 0b00000000001000000000000000000000,    // 0x200000
	EINK_MONOCHROME       = 0b00000000010000000000000000000000,    // 0x400000
	// Dithering algorithm selection (in theory, in practice the bit sharing means Y1 wins)
	EINK_DITHERING_Y1     = 0b00000001100000000000000000000000,    // 0x01800000 :(
	EINK_DITHERING_Y4     = 0b00000010100000000000000000000000,    // 0x02800000 :(
	EINK_DITHERING_SIMPLE = 0b00000100100000000000000000000000,    // 0x04800000 :(
	EINK_DITHERING_NTX_Y1 = 0b00001000100000000000000000000000,    // 0x08800000 :(
	// Merge flag              ###############################
	EINK_NO_MERGE         = 0b10000000000000000000000000000000    // 0x80000000
};

#define GET_UPDATE_MODE(mode)   ((mode) &0x0000ffff)
#define GET_UPDATE_INFO(mode)   ((mode) &0xffff0000)
#define IS_NO_MERGE(mode)       ((mode) &EINK_NO_MERGE)
#define IS_PARTIAL_UPDATE(mode) ((mode) &EINK_PARTIAL_MODE)
#define IS_AUTO_MODE(mode)      ((mode) &EINK_AUTO_MODE)
#define IS_REGAL_MODE(mode)     ((mode) &EINK_REGAL_MODE)

typedef enum eink_update_strategy
{
	EINK_UPDATE_STRATEGY_ONE_FRAME = 0,
	EINK_UPDATE_STRATEGY_MORE_FRAME_WITHOUT_FLIP,
	EINK_UPDATE_STRATEGY_MORE_RRAME_WITH_FLIP,
} eink_update_strategy_e;

typedef enum
{
	EINK_NO_DITHER           = 0,
	EINK_DITHER_Y8_Y4        = 1,
	EINK_DITHER_Y8_Y1        = 2,
	EINK_SIMPLE_DITHER_Y8_Y1 = 3,
	EINK_NTX_DITHER_Y8_Y1    = 4
} eink_dither_type;

typedef enum
{
	EINK_MIRROR        = 0,
	EINK_MIRROR_REGION = 1,
	EINK_FLIP          = 2
} eink_image_process;

//
// From "drivers/video/fbdev/sunxi/disp2/disp/de/disp_eink.h"
//
enum em_overlap_type
{
	OVERLAP_NOT_COLLISION     = 0,
	OVERLAP_WHOLE_COLLISION   = 1,
	OVERLAP_PARTIAL_COLLISION = 2
};

//#define EINK_PARTIAL_MODE 0x400

//
// From "<video/sunxi_display2.h>", here comes the nasty stuff.
//
struct disp_rectsz
{
	unsigned int width;
	unsigned int height;
};

enum disp_pixel_format
{
	DISP_FORMAT_ARGB_8888   = 0x00, /* MSB  A-R-G-B  LSB */
	DISP_FORMAT_ABGR_8888   = 0x01,
	DISP_FORMAT_RGBA_8888   = 0x02,
	DISP_FORMAT_BGRA_8888   = 0x03,
	DISP_FORMAT_XRGB_8888   = 0x04,
	DISP_FORMAT_XBGR_8888   = 0x05,
	DISP_FORMAT_RGBX_8888   = 0x06,
	DISP_FORMAT_BGRX_8888   = 0x07,
	DISP_FORMAT_RGB_888     = 0x08,
	DISP_FORMAT_BGR_888     = 0x09,
	DISP_FORMAT_RGB_565     = 0x0a,
	DISP_FORMAT_BGR_565     = 0x0b,
	DISP_FORMAT_ARGB_4444   = 0x0c,
	DISP_FORMAT_ABGR_4444   = 0x0d,
	DISP_FORMAT_RGBA_4444   = 0x0e,
	DISP_FORMAT_BGRA_4444   = 0x0f,
	DISP_FORMAT_ARGB_1555   = 0x10,
	DISP_FORMAT_ABGR_1555   = 0x11,
	DISP_FORMAT_RGBA_5551   = 0x12,
	DISP_FORMAT_BGRA_5551   = 0x13,
	DISP_FORMAT_A2R10G10B10 = 0x14,
	DISP_FORMAT_A2B10G10R10 = 0x15,
	DISP_FORMAT_R10G10B10A2 = 0x16,
	DISP_FORMAT_B10G10R10A2 = 0x17,

	/*
	 * SP: semi-planar
	 * P:planar
	 * I:interleaved
	 * UVUV: U in the LSBs;
	 * VUVU: V in the LSBs
	 */
	DISP_FORMAT_YUV444_I_AYUV        = 0x40, /* MSB  A-Y-U-V  LSB */
	DISP_FORMAT_YUV444_I_VUYA        = 0x41, /* MSB  V-U-Y-A  LSB */
	DISP_FORMAT_YUV422_I_YVYU        = 0x42, /* MSB  Y-V-Y-U  LSB */
	DISP_FORMAT_YUV422_I_YUYV        = 0x43, /* MSB  Y-U-Y-V  LSB */
	DISP_FORMAT_YUV422_I_UYVY        = 0x44, /* MSB  U-Y-V-Y  LSB */
	DISP_FORMAT_YUV422_I_VYUY        = 0x45, /* MSB  V-Y-U-Y  LSB */
	DISP_FORMAT_YUV444_P             = 0x46, /* MSB  P3-2-1-0 LSB,  YYYY UUUU VVVV */
	DISP_FORMAT_YUV422_P             = 0x47, /* MSB  P3-2-1-0 LSB   YYYY UU   VV */
	DISP_FORMAT_YUV420_P             = 0x48, /* MSB  P3-2-1-0 LSB   YYYY U    V */
	DISP_FORMAT_YUV411_P             = 0x49, /* MSB  P3-2-1-0 LSB   YYYY U    V */
	DISP_FORMAT_YUV422_SP_UVUV       = 0x4a, /* MSB  V-U-V-U  LSB */
	DISP_FORMAT_YUV422_SP_VUVU       = 0x4b, /* MSB  U-V-U-V  LSB */
	DISP_FORMAT_YUV420_SP_UVUV       = 0x4c,
	DISP_FORMAT_YUV420_SP_VUVU       = 0x4d,
	DISP_FORMAT_YUV411_SP_UVUV       = 0x4e,
	DISP_FORMAT_YUV411_SP_VUVU       = 0x4f,
	DISP_FORMAT_8BIT_GRAY            = 0x50,
	DISP_FORMAT_YUV444_I_AYUV_10BIT  = 0x51,
	DISP_FORMAT_YUV444_I_VUYA_10BIT  = 0x52,
	DISP_FORMAT_YUV422_I_YVYU_10BIT  = 0x53,
	DISP_FORMAT_YUV422_I_YUYV_10BIT  = 0x54,
	DISP_FORMAT_YUV422_I_UYVY_10BIT  = 0x55,
	DISP_FORMAT_YUV422_I_VYUY_10BIT  = 0x56,
	DISP_FORMAT_YUV444_P_10BIT       = 0x57,
	DISP_FORMAT_YUV422_P_10BIT       = 0x58,
	DISP_FORMAT_YUV420_P_10BIT       = 0x59,
	DISP_FORMAT_YUV411_P_10BIT       = 0x5a,
	DISP_FORMAT_YUV422_SP_UVUV_10BIT = 0x5b,
	DISP_FORMAT_YUV422_SP_VUVU_10BIT = 0x5c,
	DISP_FORMAT_YUV420_SP_UVUV_10BIT = 0x5d,
	DISP_FORMAT_YUV420_SP_VUVU_10BIT = 0x5e,
	DISP_FORMAT_YUV411_SP_UVUV_10BIT = 0x5f,
	DISP_FORMAT_YUV411_SP_VUVU_10BIT = 0x60,
	DISP_FORMAT_MAX,
};

enum disp_color_space
{
	DISP_UNDEF       = 0x00,
	DISP_UNDEF_F     = 0x01,
	DISP_GBR         = 0x100,
	DISP_BT709       = 0x101,
	DISP_FCC         = 0x102,
	DISP_BT470BG     = 0x103,
	DISP_BT601       = 0x104,
	DISP_SMPTE240M   = 0x105,
	DISP_YCGCO       = 0x106,
	DISP_BT2020NC    = 0x107,
	DISP_BT2020C     = 0x108,
	DISP_GBR_F       = 0x200,
	DISP_BT709_F     = 0x201,
	DISP_FCC_F       = 0x202,
	DISP_BT470BG_F   = 0x203,
	DISP_BT601_F     = 0x204,
	DISP_SMPTE240M_F = 0x205,
	DISP_YCGCO_F     = 0x206,
	DISP_BT2020NC_F  = 0x207,
	DISP_BT2020C_F   = 0x208,
	DISP_RESERVED    = 0x300,
	DISP_RESERVED_F  = 0x301,
};

struct disp_rect64
{
	long long x;
	long long y;
	long long width;
	long long height;
};

enum disp_buffer_flags
{
	DISP_BF_NORMAL          = 0,      /* non-stereo */
	DISP_BF_STEREO_TB       = 1 << 0, /* stereo top-bottom */
	DISP_BF_STEREO_FP       = 1 << 1, /* stereo frame packing */
	DISP_BF_STEREO_SSH      = 1 << 2, /* stereo side by side half */
	DISP_BF_STEREO_SSF      = 1 << 3, /* stereo side by side full */
	DISP_BF_STEREO_LI       = 1 << 4, /* stereo line interlace */
	/*
	 * 2d plus depth to convert into 3d,
	 * left and right image using the same frame buffer
	 */
	DISP_BF_STEREO_2D_DEPTH = 1 << 5,
};

enum disp_scan_flags
{
	DISP_SCAN_PROGRESSIVE               = 0,
	DISP_SCAN_INTERLACED_ODD_FLD_FIRST  = 1 << 0,
	DISP_SCAN_INTERLACED_EVEN_FLD_FIRST = 1 << 1,
};

enum disp_layer_mode
{
	LAYER_MODE_BUFFER = 0,
	LAYER_MODE_COLOR  = 1,
};

struct disp_rect
{
	int          x;
	int          y;
	unsigned int width;
	unsigned int height;
};

enum disp_3d_out_mode
{
	DISP_3D_OUT_MODE_CI_1  = 0x5, /* column interlaved 1 */
	DISP_3D_OUT_MODE_CI_2  = 0x6, /* column interlaved 2 */
	DISP_3D_OUT_MODE_CI_3  = 0x7, /* column interlaved 3 */
	DISP_3D_OUT_MODE_CI_4  = 0x8, /* column interlaved 4 */
	DISP_3D_OUT_MODE_LIRGB = 0x9, /* line interleaved rgb */

	DISP_3D_OUT_MODE_TB  = 0x0, /* top bottom */
	DISP_3D_OUT_MODE_FP  = 0x1, /* frame packing */
	DISP_3D_OUT_MODE_SSF = 0x2, /* side by side full */
	DISP_3D_OUT_MODE_SSH = 0x3, /* side by side half */
	DISP_3D_OUT_MODE_LI  = 0x4, /* line interleaved */
	DISP_3D_OUT_MODE_FA  = 0xa, /* field alternative */
};

struct disp_fb_info
{
	/* address of frame buffer,
	 * single addr for interleaved fomart,
	 * double addr for semi-planar fomart
	 * triple addr for planar format
	 */
	unsigned long long     addr[3];
	struct disp_rectsz     size[3];
	/* align for 3 comonent,unit: bytes */
	unsigned int           align[3];
	enum disp_pixel_format format;
	enum disp_color_space  color_space;       /* color space */
	unsigned int           trd_right_addr[3]; /* right address of 3d fb */
	bool                   pre_multiply;      /* true: pre-multiply fb */
	struct disp_rect64     crop;              /* crop rectangle boundaries */
	enum disp_buffer_flags flags;
	enum disp_scan_flags   scan;
};

struct disp_layer_info
{
	enum disp_layer_mode  mode;
	unsigned char         zorder;
	/* 0: pixel alpha;  1: global alpha;  2: global pixel alpha */
	unsigned char         alpha_mode;
	unsigned char         alpha_value;  /* global alpha value */
	struct disp_rect      screen_win;   /* display window on the screen */
	bool                  b_trd_out;    /* 3d display */
	enum disp_3d_out_mode out_trd_mode; /* 3d display mode */
	union
	{
		unsigned int        color; /* valid when LAYER_MODE_COLOR */
		struct disp_fb_info fb;    /* valid when LAYER_MODE_BUFFER */
	};

	unsigned int id; /* frame id, the id of frame display currently */
};

struct disp_layer_config
{
	struct disp_layer_info info;
	bool                   enable;
	unsigned int           channel;
	unsigned int           layer_id;
};

enum disp_eotf
{
	DISP_EOTF_RESERVED     = 0x000,
	DISP_EOTF_BT709        = 0x001,
	DISP_EOTF_UNDEF        = 0x002,
	DISP_EOTF_GAMMA22      = 0x004, /* SDR */
	DISP_EOTF_GAMMA28      = 0x005,
	DISP_EOTF_BT601        = 0x006,
	DISP_EOTF_SMPTE240M    = 0x007,
	DISP_EOTF_LINEAR       = 0x008,
	DISP_EOTF_LOG100       = 0x009,
	DISP_EOTF_LOG100S10    = 0x00a,
	DISP_EOTF_IEC61966_2_4 = 0x00b,
	DISP_EOTF_BT1361       = 0x00c,
	DISP_EOTF_IEC61966_2_1 = 0X00d,
	DISP_EOTF_BT2020_0     = 0x00e,
	DISP_EOTF_BT2020_1     = 0x00f,
	DISP_EOTF_SMPTE2084    = 0x010, /* HDR10 */
	DISP_EOTF_SMPTE428_1   = 0x011,
	DISP_EOTF_ARIB_STD_B67 = 0x012, /* HLG */
};

/* disp_atw_mode - mode for asynchronous time warp
 *
 * @NORMAL_MODE: dual buffer, left eye and right eye buffer is individual
 * @LEFT_RIGHT_MODE: single buffer, the left half of each line buffer
 *		     is for left eye, the right half is for the right eye
 * @UP_DOWN_MODE: single buffer, the first half of the total buffer
 *		  is for the left eye, the second half is for the right eye
 */
enum disp_atw_mode
{
	NORMAL_MODE,
	LEFT_RIGHT_MODE,
	UP_DOWN_MODE,
};

/* disp_atw_info - asynchronous time wrap infomation
 *
 * @used: indicate if the atw funtion is used
 * @mode: atw mode
 * @b_row: the row number of the micro block
 * @b_col: the column number of the micro block
 * @cof_fd: dma_buf fd for the buffer contaied coefficient for atw
 */
struct disp_atw_info
{
	bool               used;
	enum disp_atw_mode mode;
	unsigned int       b_row;
	unsigned int       b_col;
	int                cof_fd;
};

/* disp_fb_info2 - image buffer info v2
 *
 * @fd: dma_buf  fd for frame buffer
 * @size: size<width,height> for each buffer, unit:pixels
 * @align: align for each buffer, unit:bytes
 * @format: pixel format
 * @color_space: color space
 * @trd_right_fd: dma_buf fd for the right-eye frame buffer,
 *                  valid when frame-packing 3d buffer input
 * @pre_multiply: indicate the pixel use premultiplied alpha
 * @crop: crop rectangle for buffer to be display
 * @flag: indicate stereo/non-stereo buffer
 * @scan: indicate interleave/progressive scan type, and the scan order
 * @depth: depth perception for stereo image, only valid when stereo image input
 *            unit: pixel
 * @fbd_en: indicate if enable fbd function
 * @metadata_fd: dma_buf fd for the buffer contained metadata for fbc/hdr
 * @metadata_size: the size of metadata buffer, unit:bytes
 * @metadata_flag: the flag to indicate the type of metadata buffer
 *	0     : no metadata
 *	1 << 0: hdr static metadata
 *	1 << 1: hdr dynamic metadata
 *	1 << 4:	frame buffer compress(fbc) metadata
 *	x     : all type could be "or" together
 */
struct disp_fb_info2
{
	int                    fd;
	int                    y8_fd;
	struct disp_rectsz     size[3];
	unsigned int           align[3];
	enum disp_pixel_format format;
	enum disp_color_space  color_space;
	int                    trd_right_fd;
	bool                   pre_multiply;
	struct disp_rect64     crop;
	enum disp_buffer_flags flags;
	enum disp_scan_flags   scan;
	enum disp_eotf         eotf;
	int                    depth;
	unsigned int           fbd_en;
	int                    metadata_fd;
	unsigned int           metadata_size;
	unsigned int           metadata_flag;
};

/* disp_layer_info2 - layer info v2
 *
 * @mode: buffer/clolor mode, when in color mode, the layer is widthout buffer
 * @zorder: the zorder of layer, 0~max-layer-number
 * @alpha_mode:
 *	0: pixel alpha;
 *	1: global alpha
 *	2: mixed alpha, compositing width pixel alpha before global alpha
 * @alpha_value: global alpha value, valid when alpha_mode is not pixel alpha
 * @screen_win: the rectangle on the screen for fb to be display
 * @b_trd_out: indicate if 3d display output
 * @out_trd_mode: 3d output mode, valid when b_trd_out is true
 * @color: the color value to be display, valid when layer is in color mode
 * @fb: the framebuffer info related width the layer, valid when in buffer mode
 * @id: frame id, the user could get the frame-id display currently by
 *	DISP_LAYER_GET_FRAME_ID ioctl
 * @atw: asynchronous time wrap information
 */
struct disp_layer_info2
{
	enum disp_layer_mode  mode;
	unsigned char         zorder;
	unsigned char         alpha_mode;
	unsigned char         alpha_value;
	struct disp_rect      screen_win;
	bool                  b_trd_out;
	enum disp_3d_out_mode out_trd_mode;
	union
	{
		unsigned int         color;
		struct disp_fb_info2 fb;
	};

	unsigned int         id;
	struct disp_atw_info atw;
};

/* disp_layer_config2 - layer config v2
 *
 * @info: layer info
 * @enable: indicate to enable/disable the layer
 * @channel: the channel index of the layer, 0~max-channel-number
 * @layer_id: the layer index of the layer widthin it's channel
 */
struct disp_layer_config2
{
	struct disp_layer_info2 info;
	bool                    enable;
	unsigned int            channel;
	unsigned int            layer_id;
};

//
// From "drivers/video/fbdev/sunxi/disp2/disp/dev_disp.c"
//
struct gamma_correction_lut
{
	uint32_t lut[256];
};

struct cfa_enable
{
	bool bg_enable;
	int  is_cfa;
};

//
// And finally, massage stuff into sane structs & ioctl macros, both for our and strace's sake...
//

// Convert <video/sunxi_display2.h>'s tag_DISP_CMD enum to defines.
#define DISP_EINK_UPDATE       0x0402    // Return value is a frame_id
#define DISP_EINK_SET_TEMP     0x0403
#define DISP_EINK_GET_TEMP     0x0404    // No args, temp is the ioctl's return value (C°)
#define DISP_EINK_OVERLAP_SKIP 0x0405    // NOP on the release kernel (returns -1).
#define DISP_EINK_UPDATE2      0x0406
#define DISP_EINK_SET_GC_CNT   0x0407    // Defaults to 0 (disabled). Only affects *consecutive* GU16 updates.

#define DISP_EINK_SET_GAMMA                    0x4099
#define DISP_EINK_SET_BG_SETTING               0x4010
#define DISP_EINK_SET_BG_ONOFF                 0x4011
#define DISP_EINK_WAIT_BEFORE_LCD_INT_COMPLETE 0x4012    // Returns a jiffy count, timeout 1000ms
#define DISP_EINK_SET_UPDATE_CONTROL           0x4013    // HRTIMER shenanigans :?
#define DISP_EINK_WAIT_FRAME_SYNC_COMPLETE     0x4014    // Returns a jiffy count, timeout 3000ms
// Affects code flow, I imagine to speed up pen drawing. Also chains a call to EINK_SET_UPDATE_CONTROL.
// It's only fully operational for A2 & DU waveform modes,
// at which point we no longer get a meaningful frame_id out of DISP_EINK_UPDATE2, only 0.
// On the upside, it appears to completely disable overlap checks and/or blending with the existing working buffer,
// which might come in handy when dealing with rotation...
#define DISP_EINK_SET_NTX_HANDWRITE_ONOFF      0x4015
#define DISP_EINK_SET_WAIT_MODE_ONOFF          0x4016

// Not directly e-Ink related, but possibly useful nonetheless on paper,
// although in practice, they don't appear to be doing anything useful on eInk devices :/.
#define DISP_GET_SCN_WIDTH     0x0007
#define DISP_GET_SCN_HEIGHT    0x0008
#define DISP_LAYER_GET_CONFIG  0x0048
#define DISP_LAYER_GET_CONFIG2 0x004a

// And now, massage the insanity that is the disp's character device ioctl handler into some sort of actually usable API...
// The rather loose typing and lack of proper IOC macros usage make the whole thing even more "interesting"...

// Things start in a crazy blob of 7 ulongs, no matter the command...
typedef struct
{
	unsigned long int u0;    // i.e., ubuffer[0] in disp_ioctl
	unsigned long int u1;    // etc.
	unsigned long int u2;
	unsigned long int u3;
	unsigned long int u4;
	unsigned long int u5;
	unsigned long int u6;
} sunxi_disp_raw_ioctl;

// Note that, much like on disp v1, for non-eInk commands, u0 is expected to be the screen id.
// The eInk commands apparently only support being attached to screen 0, so, we're spared this...
// (The field used to be called sel, you can still find traces of it under that name in the code).

// And the actual layout for the commands we care about...
typedef struct
{
	struct area_info*         area;    // ubuffer[0]
	// ubuffer[1] (used alternatively as a size_t for the element count of the layer config copy, and an unsigned int for the actual eink_update call. Must be > 0 and <= 16).
	unsigned long int         layer_num;
	unsigned long int         update_mode;    // ubuffer[2] (bitmask, eink_update_mode)
	// ubuffer[3] (Must point to the first disp_layer_config out of at least layer_num...)
	struct disp_layer_config* lyr_cfg;
	unsigned long int         u4;        // ubuffer[4], Unused
	unsigned long int         rotate;    // ubuffer[5] (0, 90, 180, 270; cast to unsigned int)
	unsigned long int cfa_use;    // ubuffer[6] (0, 1; cast to unsigned int, because bools are for losers, I guess?)
} sunxi_disp_eink_update;

typedef struct
{
	struct area_info*          area;    // ubuffer[0]
	// ubuffer[1] (used alternatively as a size_t for the element count of the layer config copy, and an unsigned int for the actual eink_update call. Must be > 0 and <= 16).
	unsigned long int          layer_num;
	unsigned long int          update_mode;    // ubuffer[2] (bitmask, eink_update_mode)
	// ubuffer[3] (Must point to the first disp_layer_config2 out of at least layer_num...).
	// (i.e., technically, this a pointer to a contiguous array of layer_num disp_layer_config2 structs).
	struct disp_layer_config2* lyr_cfg2;
	// ubuffer[4], *outarg*, set on success (update_order in the eink buffer/pipeline manager; unrelated to the layer_info's id).
	unsigned int*              frame_id;
	uint32_t*                  rotate;    // ubuffer[5] (0, 90, 180, 270) NOTE: Yep, pointer, *sigh*...
	// ubuffer[6] (0, 1) NOTE: Nickel appears to be passing a pointer here. That'd be a bug.
	unsigned long int          cfa_use;
} sunxi_disp_eink_update2;

typedef struct
{
	unsigned long int
	    temp;    // ubuffer[0] (Cast to unsigned int, while the get variant returns a value stored in an int32_t...).
} sunxi_disp_eink_set_temp;

typedef struct
{
	unsigned long int skip;    // ubuffer[0] (Cast to unsigned int).
} sunxi_disp_eink_overlap_skip;

typedef struct
{
	unsigned long int count;    // ubuffer[0] (Cast to uint32_t, then to an unsigned int). Must be <= 20.
} sunxi_disp_eink_set_gc_count;

// NOTE: While the individual command handlers for those started using pointers properly,
//       they're unfortunately still behind the function's prologue that does the nasty 7 ulongs copy...
typedef struct
{
	struct gamma_correction_lut gamma_lut;    // NOTE: Larger than sunxi_disp_raw_ioctl
} sunxi_disp_eink_set_gamma;

typedef struct
{
	struct y8_area_info background_area;    // NOTE: Larger than sunxi_disp_raw_ioctl
} sunxi_disp_eink_set_bg_setting;

typedef struct
{
	struct cfa_enable cfa;
} sunxi_disp_eink_set_bg_onoff;

typedef struct
{
	bool enable;
} sunxi_disp_eink_set_update_control;

typedef struct
{
	uint32_t frame_id;    // Matches an update_order/frame_id returned by DISP_EINK_UPDATE(2)
} sunxi_disp_eink_wait_frame_sync_complete;

typedef struct
{
	bool enable;
} sunxi_disp_eink_set_ntx_handwrite_onoff;

typedef struct
{
	bool enable;
} sunxi_disp_eink_set_wait_mode_onoff;

typedef struct
{
	int                        screen_id;    // ubuffer[0], handled in the prologue
	// In a different order than EINK_UPDATE* commands...
	struct disp_layer_config2* lyr_cfg2;     // ubuffer[1], set channel & layer_id to identify the layer to return
	unsigned long int          layer_num;    // ubuffer[2]
} sunxi_disp_layer_get_config2;

typedef struct
{
	int                       screen_id;    // ubuffer[0], handled in the prologue
	// In a different order than EINK_UPDATE* commands...
	struct disp_layer_config* lyr_cfg;      // ubuffer[1], set channel & layer_id to identify the layer to return
	unsigned long int         layer_num;    // ubuffer[2]
} sunxi_disp_layer_get_config;

typedef struct
{
	int screen_id;    // ubuffer[0], handled in the prologue
} sunxi_disp_layer_generic_get;

// Shove everything into a union to ensure we never feed garbage to the ioctl handler for commands that actually read *less* than the prologue...
typedef union
{
	// Actual memory layout (e.g., our target size).
	sunxi_disp_raw_ioctl raw;

	// And every subcommand
	sunxi_disp_eink_update                   update;
	sunxi_disp_eink_update2                  update2;
	sunxi_disp_eink_set_temp                 set_temp;
	sunxi_disp_eink_overlap_skip             op_skip;
	sunxi_disp_eink_set_gc_count             gc_cnt;
	//sunxi_disp_eink_set_gamma                set_gamma;
	//sunxi_disp_eink_set_bg_setting           set_bg;
	sunxi_disp_eink_set_bg_onoff             toggle_bg;
	sunxi_disp_eink_set_update_control       upd_ctrl;
	sunxi_disp_eink_wait_frame_sync_complete wait_for;
	sunxi_disp_eink_set_ntx_handwrite_onoff  toggle_handw;
	sunxi_disp_eink_set_wait_mode_onoff      toggle_wait;

	sunxi_disp_layer_get_config  get_layer;
	sunxi_disp_layer_get_config2 get_layer2;
	sunxi_disp_layer_generic_get get;
} sunxi_disp_eink_ioctl;

#endif    // _DISP_INCLUDE_H_
