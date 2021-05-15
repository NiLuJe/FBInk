/****************************************************************************
**
** Copyright (C) 2016 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
** SPDX-License-Identifier: LGPL-3.0-only
**
** This file is part of the QtGui module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 3 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPL3 included in the
** packaging of this file. Please review the following information to
** ensure the GNU Lesser General Public License version 3 requirements
** will be met: https://www.gnu.org/licenses/lgpl-3.0.html.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 2.0 or (at your option) the GNU General
** Public license version 3 or any later version approved by the KDE Free
** Qt Foundation. The licenses are as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL2 and LICENSE.GPL3
** included in the packaging of this file. Please review the following
** information to ensure the GNU General Public License requirements will
** be met: https://www.gnu.org/licenses/gpl-2.0.html and
** https://www.gnu.org/licenses/gpl-3.0.html.
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include "qimagescale_p.h"

#if defined(__ARM_NEON__)

static inline __attribute__((always_inline)) uint32x4_t
    qt_qimageScaleAARGBA_helper_neon(const unsigned int* restrict pix, const int xyap, const int Cxy, const int step)
{
	uint32x2_t vpix32 = vmov_n_u32(*pix);
	uint16x4_t vpix16 = vget_low_u16(vmovl_u8(vreinterpret_u8_u32(vpix32)));
	uint32x4_t vx     = vmull_n_u16(vpix16, (uint16_t) xyap);
	int        i;
	for (i = (1 << 14) - xyap; i > Cxy; i -= Cxy) {
		pix += step;
		vpix32 = vmov_n_u32(*pix);
		vpix16 = vget_low_u16(vmovl_u8(vreinterpret_u8_u32(vpix32)));
		vx     = vaddq_u32(vx, vmull_n_u16(vpix16, (uint16_t) Cxy));
	}
	pix += step;
	vpix32 = vmov_n_u32(*pix);
	vpix16 = vget_low_u16(vmovl_u8(vreinterpret_u8_u32(vpix32)));
	vx     = vaddq_u32(vx, vmull_n_u16(vpix16, (uint16_t) i));
	return vx;
}

static inline void
    qt_qimageScaleAARGBA_up_x_down_y_neon(QImageScaleInfo* isi,
					  unsigned int* restrict dest,
					  int dw,
					  int dh,
					  int dow,
					  int sow)
{
	const unsigned int** restrict ypoints = isi->ypoints;
	const int* restrict xpoints           = isi->xpoints;
	const int* restrict xapoints          = isi->xapoints;
	const int* restrict yapoints          = isi->yapoints;

	/* go through every scanline in the output buffer */
	for (int y = 0; y < dh; y++) {
		const int Cy  = yapoints[y] >> 16;
		const int yap = yapoints[y] & 0xffff;

		unsigned int* restrict dptr = dest + (y * dow);
		for (int x = 0; x < dw; x++) {
			const unsigned int* restrict sptr = ypoints[y] + xpoints[x];
			uint32x4_t vx                     = qt_qimageScaleAARGBA_helper_neon(sptr, yap, Cy, sow);

			const int xap = xapoints[x];
			if (xap > 0) {
				uint32x4_t vr = qt_qimageScaleAARGBA_helper_neon(sptr + 1, yap, Cy, sow);

				vx = vmulq_n_u32(vx, (uint32_t) (256 - xap));
				vr = vmulq_n_u32(vr, (uint32_t) xap);
				vx = vaddq_u32(vx, vr);
				vx = vshrq_n_u32(vx, 8);
			}
			vx                    = vshrq_n_u32(vx, 14);
			const uint16x4_t vx16 = vmovn_u32(vx);
			const uint8x8_t  vx8  = vmovn_u16(vcombine_u16(vx16, vx16));
			*dptr                 = vget_lane_u32(vreinterpret_u32_u8(vx8), 0);
			dptr++;
		}
	}
}

static inline void
    qt_qimageScaleAARGB_up_x_down_y_neon(QImageScaleInfo* isi,
					 unsigned int* restrict dest,
					 int dw,
					 int dh,
					 int dow,
					 int sow)
{
	const unsigned int** restrict ypoints = isi->ypoints;
	const int* restrict xpoints           = isi->xpoints;
	const int* restrict xapoints          = isi->xapoints;
	const int* restrict yapoints          = isi->yapoints;

	/* go through every scanline in the output buffer */
	for (int y = 0; y < dh; y++) {
		const int Cy  = yapoints[y] >> 16;
		const int yap = yapoints[y] & 0xffff;

		unsigned int* restrict dptr = dest + (y * dow);
		for (int x = 0; x < dw; x++) {
			const unsigned int* restrict sptr = ypoints[y] + xpoints[x];
			uint32x4_t vx                     = qt_qimageScaleAARGBA_helper_neon(sptr, yap, Cy, sow);

			const int xap = xapoints[x];
			if (xap > 0) {
				uint32x4_t vr = qt_qimageScaleAARGBA_helper_neon(sptr + 1, yap, Cy, sow);

				vx = vmulq_n_u32(vx, (uint32_t) (256 - xap));
				vr = vmulq_n_u32(vr, (uint32_t) xap);
				vx = vaddq_u32(vx, vr);
				vx = vshrq_n_u32(vx, 8);
			}
			vx                    = vshrq_n_u32(vx, 14);
			const uint16x4_t vx16 = vmovn_u32(vx);
			const uint8x8_t  vx8  = vmovn_u16(vcombine_u16(vx16, vx16));
			*dptr                 = vget_lane_u32(vreinterpret_u32_u8(vx8), 0);
			*dptr |= 0xff000000;
			dptr++;
		}
	}
}

static inline void
    qt_qimageScaleAARGBA_down_x_up_y_neon(QImageScaleInfo* isi,
					  unsigned int* restrict dest,
					  int dw,
					  int dh,
					  int dow,
					  int sow)
{
	const unsigned int** restrict ypoints = isi->ypoints;
	const int* restrict xpoints           = isi->xpoints;
	const int* restrict xapoints          = isi->xapoints;
	const int* restrict yapoints          = isi->yapoints;

	/* go through every scanline in the output buffer */
	for (int y = 0; y < dh; y++) {
		unsigned int* restrict dptr = dest + (y * dow);
		for (int x = 0; x < dw; x++) {
			const int Cx  = xapoints[x] >> 16;
			const int xap = xapoints[x] & 0xffff;

			const unsigned int* restrict sptr = ypoints[y] + xpoints[x];
			uint32x4_t vx                     = qt_qimageScaleAARGBA_helper_neon(sptr, xap, Cx, 1);

			const int yap = yapoints[y];
			if (yap > 0) {
				uint32x4_t vr = qt_qimageScaleAARGBA_helper_neon(sptr + sow, xap, Cx, 1);

				vx = vmulq_n_u32(vx, (uint32_t) (256 - yap));
				vr = vmulq_n_u32(vr, (uint32_t) yap);
				vx = vaddq_u32(vx, vr);
				vx = vshrq_n_u32(vx, 8);
			}
			vx                    = vshrq_n_u32(vx, 14);
			const uint16x4_t vx16 = vmovn_u32(vx);
			const uint8x8_t  vx8  = vmovn_u16(vcombine_u16(vx16, vx16));
			*dptr                 = vget_lane_u32(vreinterpret_u32_u8(vx8), 0);
			dptr++;
		}
	}
}

static inline void
    qt_qimageScaleAARGB_down_x_up_y_neon(QImageScaleInfo* isi,
					 unsigned int* restrict dest,
					 int dw,
					 int dh,
					 int dow,
					 int sow)
{
	const unsigned int** restrict ypoints = isi->ypoints;
	const int* restrict xpoints           = isi->xpoints;
	const int* restrict xapoints          = isi->xapoints;
	const int* restrict yapoints          = isi->yapoints;

	/* go through every scanline in the output buffer */
	for (int y = 0; y < dh; y++) {
		unsigned int* restrict dptr = dest + (y * dow);
		for (int x = 0; x < dw; x++) {
			const int Cx  = xapoints[x] >> 16;
			const int xap = xapoints[x] & 0xffff;

			const unsigned int* restrict sptr = ypoints[y] + xpoints[x];
			uint32x4_t vx                     = qt_qimageScaleAARGBA_helper_neon(sptr, xap, Cx, 1);

			const int yap = yapoints[y];
			if (yap > 0) {
				uint32x4_t vr = qt_qimageScaleAARGBA_helper_neon(sptr + sow, xap, Cx, 1);

				vx = vmulq_n_u32(vx, (uint32_t) (256 - yap));
				vr = vmulq_n_u32(vr, (uint32_t) yap);
				vx = vaddq_u32(vx, vr);
				vx = vshrq_n_u32(vx, 8);
			}
			vx                    = vshrq_n_u32(vx, 14);
			const uint16x4_t vx16 = vmovn_u32(vx);
			const uint8x8_t  vx8  = vmovn_u16(vcombine_u16(vx16, vx16));
			*dptr                 = vget_lane_u32(vreinterpret_u32_u8(vx8), 0);
			*dptr |= 0xff000000;
			dptr++;
		}
	}
}

static inline void
    qt_qimageScaleAARGBA_down_xy_neon(QImageScaleInfo* isi, unsigned int* restrict dest, int dw, int dh, int dow, int sow)
{
	const unsigned int** restrict ypoints = isi->ypoints;
	const int* restrict xpoints           = isi->xpoints;
	const int* restrict xapoints          = isi->xapoints;
	const int* restrict yapoints          = isi->yapoints;

	for (int y = 0; y < dh; y++) {
		const int Cy  = yapoints[y] >> 16;
		const int yap = yapoints[y] & 0xffff;

		unsigned int* restrict dptr = dest + (y * dow);
		for (int x = 0; x < dw; x++) {
			const int Cx  = xapoints[x] >> 16;
			const int xap = xapoints[x] & 0xffff;

			const unsigned int* restrict sptr = ypoints[y] + xpoints[x];
			uint32x4_t vx                     = qt_qimageScaleAARGBA_helper_neon(sptr, xap, Cx, 1);
			vx                                = vshrq_n_u32(vx, 4);
			uint32x4_t vr                     = vmulq_n_u32(vx, (uint32_t) yap);

			int j;
			for (j = (1 << 14) - yap; j > Cy; j -= Cy) {
				sptr += sow;
				vx = qt_qimageScaleAARGBA_helper_neon(sptr, xap, Cx, 1);
				vx = vshrq_n_u32(vx, 4);
				vx = vmulq_n_u32(vx, (uint32_t) Cy);
				vr = vaddq_u32(vr, vx);
			}
			sptr += sow;
			vx = qt_qimageScaleAARGBA_helper_neon(sptr, xap, Cx, 1);
			vx = vshrq_n_u32(vx, 4);
			vx = vmulq_n_u32(vx, (uint32_t) j);
			vr = vaddq_u32(vr, vx);

			vx                    = vshrq_n_u32(vr, 24);
			const uint16x4_t vx16 = vmovn_u32(vx);
			const uint8x8_t  vx8  = vmovn_u16(vcombine_u16(vx16, vx16));
			*dptr                 = vget_lane_u32(vreinterpret_u32_u8(vx8), 0);
			dptr++;
		}
	}
}

static inline void
    qt_qimageScaleAARGB_down_xy_neon(QImageScaleInfo* isi, unsigned int* restrict dest, int dw, int dh, int dow, int sow)
{
	const unsigned int** restrict ypoints = isi->ypoints;
	const int* restrict xpoints           = isi->xpoints;
	const int* restrict xapoints          = isi->xapoints;
	const int* restrict yapoints          = isi->yapoints;

	for (int y = 0; y < dh; y++) {
		const int Cy  = yapoints[y] >> 16;
		const int yap = yapoints[y] & 0xffff;

		unsigned int* restrict dptr = dest + (y * dow);
		for (int x = 0; x < dw; x++) {
			const int Cx  = xapoints[x] >> 16;
			const int xap = xapoints[x] & 0xffff;

			const unsigned int* restrict sptr = ypoints[y] + xpoints[x];
			uint32x4_t vx                     = qt_qimageScaleAARGBA_helper_neon(sptr, xap, Cx, 1);
			vx                                = vshrq_n_u32(vx, 4);
			uint32x4_t vr                     = vmulq_n_u32(vx, (uint32_t) yap);

			int j;
			for (j = (1 << 14) - yap; j > Cy; j -= Cy) {
				sptr += sow;
				vx = qt_qimageScaleAARGBA_helper_neon(sptr, xap, Cx, 1);
				vx = vshrq_n_u32(vx, 4);
				vx = vmulq_n_u32(vx, (uint32_t) Cy);
				vr = vaddq_u32(vr, vx);
			}
			sptr += sow;
			vx = qt_qimageScaleAARGBA_helper_neon(sptr, xap, Cx, 1);
			vx = vshrq_n_u32(vx, 4);
			vx = vmulq_n_u32(vx, (uint32_t) j);
			vr = vaddq_u32(vr, vx);

			vx                    = vshrq_n_u32(vr, 24);
			const uint16x4_t vx16 = vmovn_u32(vx);
			const uint8x8_t  vx8  = vmovn_u16(vcombine_u16(vx16, vx16));
			*dptr                 = vget_lane_u32(vreinterpret_u32_u8(vx8), 0);
			*dptr |= 0xff000000;
			dptr++;
		}
	}
}

#endif
