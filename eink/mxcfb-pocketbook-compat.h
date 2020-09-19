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
 * NOTE: Pulled from an EPDCv2 kernel, because we need those constants to build FBInk ;).
*/

#ifndef __ASM_ARCH_MXCFB_COMPAT_H__
#define __ASM_ARCH_MXCFB_COMPAT_H__

enum mxcfb_dithering_mode {
	EPDC_FLAG_USE_DITHERING_PASSTHROUGH = 0x0,
	EPDC_FLAG_USE_DITHERING_FLOYD_STEINBERG,
	EPDC_FLAG_USE_DITHERING_ATKINSON,
	EPDC_FLAG_USE_DITHERING_ORDERED,
	EPDC_FLAG_USE_DITHERING_QUANT_ONLY,
	PDC_FLAG_USE_DITHERING_MAX,
};

#endif // __ASM_ARCH_MXCFB_COMPAT_H__
