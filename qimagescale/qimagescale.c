/****************************************************************************
**
** Copyright (C) 2016 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
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
#include "qdrawhelper_p.h"
#include "qglobal.h"
#include "qimagescale_p.h"
#include "qrgb.h"

#include <stdio.h>
#include <stdlib.h>

#ifndef FBINK_QIS_NO_SIMD
#	if defined(__ARM_NEON__)
#		include "qimagescale_neon.c"
#	endif
#	if defined(__SSE4_1__)
#		include "qimagescale_sse4.c"
#	endif
#endif

/*
 * Copyright (C) 2004, 2005 Daniel M. Duley
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 */

/* OTHER CREDITS:
 *
 * This is the normal smoothscale method, based on Imlib2's smoothscale.
 *
 * Originally I took the algorithm used in NetPBM and Qt and added MMX/3dnow
 * optimizations. It ran in about 1/2 the time as Qt. Then I ported Imlib's
 * C algorithm and it ran at about the same speed as my MMX optimized one...
 * Finally I ported Imlib's MMX version and it ran in less than half the
 * time as my MMX algorithm, (taking only a quarter of the time Qt does).
 * After further optimization it seems to run at around 1/6th.
 *
 * Changes include formatting, namespaces and other C++'ings, removal of old
 * #ifdef'ed code, and removal of unneeded border calculation code.
 * Later the code has been refactored, an SSE4.1 optimizated path have been
 * added instead of the removed MMX assembler, and scaling of clipped area
 * removed, and an RGBA64 version written
 *
 * Imlib2 is (C) Carsten Haitzler and various contributors. The MMX code
 * is by Willem Monsuwe <willem@stack.nl>. All other modifications are
 * (C) Daniel M. Duley.
 */

static const unsigned int**   qimageCalcYPoints(const unsigned int* src, int sw, int sh, int dh);
static const unsigned char**  qimageCalcYPointsY8(const unsigned char* src, int sw, int sh, int dh);
static const unsigned short** qimageCalcYPointsY8A(const unsigned short* src, int sw, int sh, int dh);
static int*                   qimageCalcXPoints(int sw, int dw);
static int*                   qimageCalcApoints(int s, int d, int up);
static QImageScaleInfo*       qimageFreeScaleInfo(QImageScaleInfo* isi);
static QImageScaleInfo* qimageCalcScaleInfo(const unsigned char* img, int sw, int sh, int sn, int dw, int dh, char aa);

//
// Code ported from Imlib...
//

static const unsigned int**
    qimageCalcYPoints(const unsigned int* src, int sw, int sh, int dh)
{
	const unsigned int** p;
	int                  j = 0, rv = 0;
	qint64               val, inc;

	if (dh < 0) {
		dh = -dh;
		rv = 1;
	}
	p = malloc((dh + 1) * sizeof(src));

	int up = qAbs(dh) >= sh;
	val    = up ? 0x8000 * sh / dh - 0x8000 : 0;
	inc    = (((qint64) sh) << 16) / dh;
	for (int i = 0; i < dh; i++) {
		p[j++] = src + qMax(0LL, val >> 16) * sw;
		val += inc;
	}
	if (rv) {
		for (int i = dh / 2; --i >= 0;) {
			const unsigned int* tmp = p[i];
			p[i]                    = p[dh - i - 1];
			p[dh - i - 1]           = tmp;
		}
	}
	return (p);
}

static const unsigned char**
    qimageCalcYPointsY8(const unsigned char* src, int sw, int sh, int dh)
{
	const unsigned char** p;
	int                   j = 0, rv = 0;
	qint64                val, inc;

	if (dh < 0) {
		dh = -dh;
		rv = 1;
	}
	p = malloc((dh + 1) * sizeof(src));

	int up = qAbs(dh) >= sh;
	val    = up ? 0x8000 * sh / dh - 0x8000 : 0;
	inc    = (((qint64) sh) << 16) / dh;
	for (int i = 0; i < dh; i++) {
		p[j++] = src + qMax(0LL, val >> 16) * sw;
		val += inc;
	}
	if (rv) {
		for (int i = dh / 2; --i >= 0;) {
			const unsigned char* tmp = p[i];
			p[i]                     = p[dh - i - 1];
			p[dh - i - 1]            = tmp;
		}
	}
	return (p);
}

static const unsigned short**
    qimageCalcYPointsY8A(const unsigned short* src, int sw, int sh, int dh)
{
	const unsigned short** p;
	int                    j = 0, rv = 0;
	qint64                 val, inc;

	if (dh < 0) {
		dh = -dh;
		rv = 1;
	}
	p = malloc((dh + 1) * sizeof(src));

	int up = qAbs(dh) >= sh;
	val    = up ? 0x8000 * sh / dh - 0x8000 : 0;
	inc    = (((qint64) sh) << 16) / dh;
	for (int i = 0; i < dh; i++) {
		p[j++] = src + qMax(0LL, val >> 16) * sw;
		val += inc;
	}
	if (rv) {
		for (int i = dh / 2; --i >= 0;) {
			const unsigned short* tmp = p[i];
			p[i]                      = p[dh - i - 1];
			p[dh - i - 1]             = tmp;
		}
	}
	return (p);
}

static int*
    qimageCalcXPoints(int sw, int dw)
{
	int *  p, j = 0, rv = 0;
	qint64 val, inc;

	if (dw < 0) {
		dw = -dw;
		rv = 1;
	}
	p = malloc((dw + 1) * sizeof(int));

	int up = qAbs(dw) >= sw;
	val    = up ? 0x8000 * sw / dw - 0x8000 : 0;
	inc    = (((qint64) sw) << 16) / dw;
	for (int i = 0; i < dw; i++) {
		p[j++] = qMax(0LL, val >> 16);
		val += inc;
	}

	if (rv) {
		for (int i = dw / 2; --i >= 0;) {
			int tmp       = p[i];
			p[i]          = p[dw - i - 1];
			p[dw - i - 1] = tmp;
		}
	}
	return p;
}

static int*
    qimageCalcApoints(int s, int d, int up)
{
	int *p, j = 0, rv = 0;

	if (d < 0) {
		rv = 1;
		d  = -d;
	}
	p = malloc(d * sizeof(int));

	if (up) {
		/* scaling up */
		qint64 val = 0x8000 * s / d - 0x8000;
		qint64 inc = (((qint64) s) << 16) / d;
		for (int i = 0; i < d; i++) {
			int pos = val >> 16;
			if (pos < 0)
				p[j++] = 0;
			else if (pos >= (s - 1))
				p[j++] = 0;
			else
				p[j++] = (val >> 8) - ((val >> 8) & 0xffffff00);
			val += inc;
		}
	} else {
		/* scaling down */
		qint64 val = 0;
		qint64 inc = (((qint64) s) << 16) / d;
		int    Cp  = (((d << 14) + s - 1) / s);
		for (int i = 0; i < d; i++) {
			int ap = ((0x10000 - (val & 0xffff)) * Cp) >> 16;
			p[j]   = ap | (Cp << 16);
			j++;
			val += inc;
		}
	}
	if (rv) {
		int tmp;
		for (int i = d / 2; --i >= 0;) {
			tmp          = p[i];
			p[i]         = p[d - i - 1];
			p[d - i - 1] = tmp;
		}
	}
	return p;
}

static QImageScaleInfo*
    qimageFreeScaleInfo(QImageScaleInfo* isi)
{
	if (isi) {
		free(isi->xpoints);
		free(isi->ypoints);
		free(isi->ypoints_y8);
		free(isi->ypoints_y8a);
		free(isi->xapoints);
		free(isi->yapoints);
		free(isi);
	}
	return NULL;
}

static QImageScaleInfo*
    qimageCalcScaleInfo(const unsigned char* img, int sw, int sh, int sn, int dw, int dh, char aa)
{
	QImageScaleInfo* isi;
	int              scw, sch;

	scw = dw;
	sch = dh;

	isi = calloc(1U, sizeof(QImageScaleInfo));
	if (!isi)
		return NULL;

	isi->xup_yup = (qAbs(dw) >= sw) + ((qAbs(dh) >= sh) << 1);

	isi->xpoints = qimageCalcXPoints(sw, scw);
	if (!isi->xpoints)
		return qimageFreeScaleInfo(isi);
	// NOTE: We use sw directly as a simplification. Technically, it's img bytes-per-lines / bytes-per-pixel
	//       (i.e., img's width (sw) * number of color components (sn) / sizeof(*img) for unpadded packed pixels).
	//       This explains the casts and the need for different functions according to pointer width.
	switch (sn) {
		case 4:
			// sn is 4, as is sizeof(uint32_t), hence using width directly ;).
			isi->ypoints = qimageCalcYPoints((const unsigned int*) img, sw, sh, sch);
			if (!isi->ypoints)
				return qimageFreeScaleInfo(isi);
			break;
		case 2:
			// sn is 2, as is sizeof(unsigned short), hence using width directly ;).
			isi->ypoints_y8a = qimageCalcYPointsY8A((const unsigned short*) img, sw, sh, sch);
			if (!isi->ypoints_y8a)
				return qimageFreeScaleInfo(isi);
			break;
		case 1:
			// sn is 1, as is sizeof(unsigned char), hence using width directly ;).
			isi->ypoints_y8 = qimageCalcYPointsY8((const unsigned char*) img, sw, sh, sch);
			if (!isi->ypoints_y8)
				return qimageFreeScaleInfo(isi);
			break;
		default:
			return qimageFreeScaleInfo(isi);
			break;
	}
	if (aa) {
		isi->xapoints = qimageCalcApoints(sw, scw, isi->xup_yup & 1);
		if (!isi->xapoints)
			return qimageFreeScaleInfo(isi);
		isi->yapoints = qimageCalcApoints(sh, sch, isi->xup_yup & 2);
		if (!isi->yapoints)
			return qimageFreeScaleInfo(isi);
	}
	return isi;
}

#if defined(FBINK_QIS_NO_SIMD) || !(defined(__SSE4_1__) || defined(__ARM_NEON__))
static void qt_qimageScaleAARGBA_up_x_down_y(QImageScaleInfo* isi, unsigned int* dest, int dw, int dh, int dow, int sow);

static void qt_qimageScaleAARGBA_down_x_up_y(QImageScaleInfo* isi, unsigned int* dest, int dw, int dh, int dow, int sow);

static void qt_qimageScaleAARGBA_down_xy(QImageScaleInfo* isi, unsigned int* dest, int dw, int dh, int dow, int sow);
#endif

#ifndef FBINK_QIS_NO_SIMD
#	if defined(__SSE4_1__)
inline static void qt_qimageScaleAARGBA_up_x_down_y_sse4(QImageScaleInfo* isi,
							 unsigned int*    dest,
							 int              dw,
							 int              dh,
							 int              dow,
							 int              sow);
inline static void qt_qimageScaleAARGBA_down_x_up_y_sse4(QImageScaleInfo* isi,
							 unsigned int*    dest,
							 int              dw,
							 int              dh,
							 int              dow,
							 int              sow);
inline static void qt_qimageScaleAARGBA_down_xy_sse4(QImageScaleInfo* isi,
						     unsigned int*    dest,
						     int              dw,
						     int              dh,
						     int              dow,
						     int              sow);

inline static void qt_qimageScaleAARGB_up_x_down_y_sse4(QImageScaleInfo* isi,
							unsigned int*    dest,
							int              dw,
							int              dh,
							int              dow,
							int              sow);
inline static void qt_qimageScaleAARGB_down_x_up_y_sse4(QImageScaleInfo* isi,
							unsigned int*    dest,
							int              dw,
							int              dh,
							int              dow,
							int              sow);
inline static void qt_qimageScaleAARGB_down_xy_sse4(QImageScaleInfo* isi,
						    unsigned int*    dest,
						    int              dw,
						    int              dh,
						    int              dow,
						    int              sow);
#	endif

#	if defined(__ARM_NEON__)
inline static void qt_qimageScaleAARGBA_up_x_down_y_neon(QImageScaleInfo* isi,
							 unsigned int*    dest,
							 int              dw,
							 int              dh,
							 int              dow,
							 int              sow);
inline static void qt_qimageScaleAARGBA_down_x_up_y_neon(QImageScaleInfo* isi,
							 unsigned int*    dest,
							 int              dw,
							 int              dh,
							 int              dow,
							 int              sow);
inline static void qt_qimageScaleAARGBA_down_xy_neon(QImageScaleInfo* isi,
						     unsigned int*    dest,
						     int              dw,
						     int              dh,
						     int              dow,
						     int              sow);

inline static void qt_qimageScaleAARGB_up_x_down_y_neon(QImageScaleInfo* isi,
							unsigned int*    dest,
							int              dw,
							int              dh,
							int              dow,
							int              sow);
inline static void qt_qimageScaleAARGB_down_x_up_y_neon(QImageScaleInfo* isi,
							unsigned int*    dest,
							int              dw,
							int              dh,
							int              dow,
							int              sow);
inline static void qt_qimageScaleAARGB_down_xy_neon(QImageScaleInfo* isi,
						    unsigned int*    dest,
						    int              dw,
						    int              dh,
						    int              dow,
						    int              sow);
#	endif
#endif

static void
    qt_qimageScaleAARGBA_up_xy(QImageScaleInfo* isi, unsigned int* dest, int dw, int dh, int dow, int sow)
{
	const unsigned int** ypoints  = isi->ypoints;
	int*                 xpoints  = isi->xpoints;
	int*                 xapoints = isi->xapoints;
	int*                 yapoints = isi->yapoints;

	/* go through every scanline in the output buffer */
	for (int y = 0; y < dh; y++) {
		/* calculate the source line we'll scan from */
		const unsigned int* sptr = ypoints[y];
		unsigned int*       dptr = dest + (y * dow);
		const int           yap  = yapoints[y];
		if (yap > 0) {
			for (int x = 0; x < dw; x++) {
				const unsigned int* pix = sptr + xpoints[x];
				const int           xap = xapoints[x];
				if (xap > 0)
					*dptr = interpolate_4_pixels_arr(pix, pix + sow, xap, yap);
				else
					*dptr = INTERPOLATE_PIXEL_256(pix[0], 256 - yap, pix[sow], yap);
				dptr++;
			}
		} else {
			for (int x = 0; x < dw; x++) {
				const unsigned int* pix = sptr + xpoints[x];
				const int           xap = xapoints[x];
				if (xap > 0)
					*dptr = INTERPOLATE_PIXEL_256(pix[0], 256 - xap, pix[1], xap);
				else
					*dptr = pix[0];
				dptr++;
			}
		}
	}
}

/* scale by area sampling - with alpha */
static void
    qt_qimageScaleAARGBA(QImageScaleInfo* isi, unsigned int* dest, int dw, int dh, int dow, int sow)
{
	/* scaling up both ways */
	if (isi->xup_yup == 3) {
		qt_qimageScaleAARGBA_up_xy(isi, dest, dw, dh, dow, sow);
	}
	/* if we're scaling down vertically */
	else if (isi->xup_yup == 1) {
#ifndef FBINK_QIS_NO_SIMD
#	if defined(__SSE4_1__)
		qt_qimageScaleAARGBA_up_x_down_y_sse4(isi, dest, dw, dh, dow, sow);
#	elif defined(__ARM_NEON__)
		qt_qimageScaleAARGBA_up_x_down_y_neon(isi, dest, dw, dh, dow, sow);
#	else
		qt_qimageScaleAARGBA_up_x_down_y(isi, dest, dw, dh, dow, sow);
#	endif
#else
		qt_qimageScaleAARGBA_up_x_down_y(isi, dest, dw, dh, dow, sow);
#endif
	}
	/* if we're scaling down horizontally */
	else if (isi->xup_yup == 2) {
#ifndef FBINK_QIS_NO_SIMD
#	if defined(__SSE4_1__)
		qt_qimageScaleAARGBA_down_x_up_y_sse4(isi, dest, dw, dh, dow, sow);
#	elif defined(__ARM_NEON__)
		qt_qimageScaleAARGBA_down_x_up_y_neon(isi, dest, dw, dh, dow, sow);
#	else
		qt_qimageScaleAARGBA_down_x_up_y(isi, dest, dw, dh, dow, sow);
#	endif
#else
		qt_qimageScaleAARGBA_down_x_up_y(isi, dest, dw, dh, dow, sow);
#endif
	}
	/* if we're scaling down horizontally & vertically */
	else {
#ifndef FBINK_QIS_NO_SIMD
#	if defined(__SSE4_1__)
		qt_qimageScaleAARGBA_down_xy_sse4(isi, dest, dw, dh, dow, sow);
#	elif defined(__ARM_NEON__)
		qt_qimageScaleAARGBA_down_xy_neon(isi, dest, dw, dh, dow, sow);
#	else
		qt_qimageScaleAARGBA_down_xy(isi, dest, dw, dh, dow, sow);
#	endif
#else
		qt_qimageScaleAARGBA_down_xy(isi, dest, dw, dh, dow, sow);
#endif
	}
}

inline static void
    qt_qimageScaleAARGBA_helper(const unsigned int* pix, int xyap, int Cxy, int step, int* r, int* g, int* b, int* a)
{
	*r = qRed(*pix) * xyap;
	*g = qGreen(*pix) * xyap;
	*b = qBlue(*pix) * xyap;
	*a = qAlpha(*pix) * xyap;
	int j;
	for (j = (1 << 14) - xyap; j > Cxy; j -= Cxy) {
		pix += step;
		*r += qRed(*pix) * Cxy;
		*g += qGreen(*pix) * Cxy;
		*b += qBlue(*pix) * Cxy;
		*a += qAlpha(*pix) * Cxy;
	}
	pix += step;
	*r += qRed(*pix) * j;
	*g += qGreen(*pix) * j;
	*b += qBlue(*pix) * j;
	*a += qAlpha(*pix) * j;
}

#if defined(FBINK_QIS_NO_SIMD) || !(defined(__SSE4_1__) || defined(__ARM_NEON__))
static void
    qt_qimageScaleAARGBA_up_x_down_y(QImageScaleInfo* isi, unsigned int* dest, int dw, int dh, int dow, int sow)
{
	const unsigned int** ypoints  = isi->ypoints;
	int*                 xpoints  = isi->xpoints;
	int*                 xapoints = isi->xapoints;
	int*                 yapoints = isi->yapoints;

	/* go through every scanline in the output buffer */
	for (int y = 0; y < dh; y++) {
		int Cy  = yapoints[y] >> 16;
		int yap = yapoints[y] & 0xffff;

		unsigned int* dptr = dest + (y * dow);
		for (int x = 0; x < dw; x++) {
			const unsigned int* sptr = ypoints[y] + xpoints[x];
			int                 r, g, b, a;
			qt_qimageScaleAARGBA_helper(sptr, yap, Cy, sow, &r, &g, &b, &a);

			int xap = xapoints[x];
			if (xap > 0) {
				int rr, gg, bb, aa;
				qt_qimageScaleAARGBA_helper(sptr + 1, yap, Cy, sow, &rr, &gg, &bb, &aa);

				r = r * (256 - xap);
				g = g * (256 - xap);
				b = b * (256 - xap);
				a = a * (256 - xap);
				r = (r + (rr * xap)) >> 8;
				g = (g + (gg * xap)) >> 8;
				b = (b + (bb * xap)) >> 8;
				a = (a + (aa * xap)) >> 8;
			}
			*dptr++ = qRgba(r >> 14, g >> 14, b >> 14, a >> 14);
		}
	}
}
#endif

#if defined(FBINK_QIS_NO_SIMD) || !(defined(__SSE4_1__) || defined(__ARM_NEON__))
static void
    qt_qimageScaleAARGBA_down_x_up_y(QImageScaleInfo* isi, unsigned int* dest, int dw, int dh, int dow, int sow)
{
	const unsigned int** ypoints  = isi->ypoints;
	int*                 xpoints  = isi->xpoints;
	int*                 xapoints = isi->xapoints;
	int*                 yapoints = isi->yapoints;

	/* go through every scanline in the output buffer */
	for (int y = 0; y < dh; y++) {
		unsigned int* dptr = dest + (y * dow);
		for (int x = 0; x < dw; x++) {
			int Cx  = xapoints[x] >> 16;
			int xap = xapoints[x] & 0xffff;

			const unsigned int* sptr = ypoints[y] + xpoints[x];
			int                 r, g, b, a;
			qt_qimageScaleAARGBA_helper(sptr, xap, Cx, 1, &r, &g, &b, &a);

			int yap = yapoints[y];
			if (yap > 0) {
				int rr, gg, bb, aa;
				qt_qimageScaleAARGBA_helper(sptr + sow, xap, Cx, 1, &rr, &gg, &bb, &aa);

				r = r * (256 - yap);
				g = g * (256 - yap);
				b = b * (256 - yap);
				a = a * (256 - yap);
				r = (r + (rr * yap)) >> 8;
				g = (g + (gg * yap)) >> 8;
				b = (b + (bb * yap)) >> 8;
				a = (a + (aa * yap)) >> 8;
			}
			*dptr = qRgba(r >> 14, g >> 14, b >> 14, a >> 14);
			dptr++;
		}
	}
}
#endif

#if defined(FBINK_QIS_NO_SIMD) || !(defined(__SSE4_1__) || defined(__ARM_NEON__))
static void
    qt_qimageScaleAARGBA_down_xy(QImageScaleInfo* isi, unsigned int* dest, int dw, int dh, int dow, int sow)
{
	const unsigned int** ypoints  = isi->ypoints;
	int*                 xpoints  = isi->xpoints;
	int*                 xapoints = isi->xapoints;
	int*                 yapoints = isi->yapoints;

	for (int y = 0; y < dh; y++) {
		int Cy  = (yapoints[y]) >> 16;
		int yap = (yapoints[y]) & 0xffff;

		unsigned int* dptr = dest + (y * dow);
		for (int x = 0; x < dw; x++) {
			int Cx  = xapoints[x] >> 16;
			int xap = xapoints[x] & 0xffff;

			const unsigned int* sptr = ypoints[y] + xpoints[x];
			int                 rx, gx, bx, ax;
			qt_qimageScaleAARGBA_helper(sptr, xap, Cx, 1, &rx, &gx, &bx, &ax);

			int r = ((rx >> 4) * yap);
			int g = ((gx >> 4) * yap);
			int b = ((bx >> 4) * yap);
			int a = ((ax >> 4) * yap);

			int j;
			for (j = (1 << 14) - yap; j > Cy; j -= Cy) {
				sptr += sow;
				qt_qimageScaleAARGBA_helper(sptr, xap, Cx, 1, &rx, &gx, &bx, &ax);
				r += ((rx >> 4) * Cy);
				g += ((gx >> 4) * Cy);
				b += ((bx >> 4) * Cy);
				a += ((ax >> 4) * Cy);
			}
			sptr += sow;
			qt_qimageScaleAARGBA_helper(sptr, xap, Cx, 1, &rx, &gx, &bx, &ax);

			r += ((rx >> 4) * j);
			g += ((gx >> 4) * j);
			b += ((bx >> 4) * j);
			a += ((ax >> 4) * j);

			*dptr = qRgba(r >> 24, g >> 24, b >> 24, a >> 24);
			dptr++;
		}
	}
}
#endif

#if defined(FBINK_QIS_NO_SIMD) || !(defined(__SSE4_1__) || defined(__ARM_NEON__))
static void qt_qimageScaleAARGB_up_x_down_y(QImageScaleInfo* isi, unsigned int* dest, int dw, int dh, int dow, int sow);

static void qt_qimageScaleAARGB_down_x_up_y(QImageScaleInfo* isi, unsigned int* dest, int dw, int dh, int dow, int sow);

static void qt_qimageScaleAARGB_down_xy(QImageScaleInfo* isi, unsigned int* dest, int dw, int dh, int dow, int sow);
#endif

/* scale by area sampling - IGNORE the ALPHA byte*/
static void
    qt_qimageScaleAARGB(QImageScaleInfo* isi, unsigned int* dest, int dw, int dh, int dow, int sow)
{
	/* scaling up both ways */
	if (isi->xup_yup == 3) {
		qt_qimageScaleAARGBA_up_xy(isi, dest, dw, dh, dow, sow);
	}
	/* if we're scaling down vertically */
	else if (isi->xup_yup == 1) {
#ifndef FBINK_QIS_NO_SIMD
#	if defined(__SSE4_1__)
		qt_qimageScaleAARGB_up_x_down_y_sse4(isi, dest, dw, dh, dow, sow);
#	elif defined(__ARM_NEON__)
		qt_qimageScaleAARGB_up_x_down_y_neon(isi, dest, dw, dh, dow, sow);
#	else
		qt_qimageScaleAARGB_up_x_down_y(isi, dest, dw, dh, dow, sow);
#	endif
#else
		qt_qimageScaleAARGB_up_x_down_y(isi, dest, dw, dh, dow, sow);
#endif
	}
	/* if we're scaling down horizontally */
	else if (isi->xup_yup == 2) {
#ifndef FBINK_QIS_NO_SIMD
#	if defined(__SSE4_1__)
		qt_qimageScaleAARGB_down_x_up_y_sse4(isi, dest, dw, dh, dow, sow);
#	elif defined(__ARM_NEON__)
		qt_qimageScaleAARGB_down_x_up_y_neon(isi, dest, dw, dh, dow, sow);
#	else
		qt_qimageScaleAARGB_down_x_up_y(isi, dest, dw, dh, dow, sow);
#	endif
#else
		qt_qimageScaleAARGB_down_x_up_y(isi, dest, dw, dh, dow, sow);
#endif
	}
	/* if we're scaling down horizontally & vertically */
	else {
#ifndef FBINK_QIS_NO_SIMD
#	if defined(__SSE4_1__)
		qt_qimageScaleAARGB_down_xy_sse4(isi, dest, dw, dh, dow, sow);
#	elif defined(__ARM_NEON__)
		qt_qimageScaleAARGB_down_xy_neon(isi, dest, dw, dh, dow, sow);
#	else
		qt_qimageScaleAARGB_down_xy(isi, dest, dw, dh, dow, sow);
#	endif
#else
		qt_qimageScaleAARGB_down_xy(isi, dest, dw, dh, dow, sow);
#endif
	}
}

inline static void
    qt_qimageScaleAARGB_helper(const unsigned int* pix, int xyap, int Cxy, int step, int* r, int* g, int* b)
{
	*r = qRed(*pix) * xyap;
	*g = qGreen(*pix) * xyap;
	*b = qBlue(*pix) * xyap;
	int j;
	for (j = (1 << 14) - xyap; j > Cxy; j -= Cxy) {
		pix += step;
		*r += qRed(*pix) * Cxy;
		*g += qGreen(*pix) * Cxy;
		*b += qBlue(*pix) * Cxy;
	}
	pix += step;
	*r += qRed(*pix) * j;
	*g += qGreen(*pix) * j;
	*b += qBlue(*pix) * j;
}

#if defined(FBINK_QIS_NO_SIMD) || !(defined(__SSE4_1__) || defined(__ARM_NEON__))
static void
    qt_qimageScaleAARGB_up_x_down_y(QImageScaleInfo* isi, unsigned int* dest, int dw, int dh, int dow, int sow)
{
	const unsigned int** ypoints  = isi->ypoints;
	int*                 xpoints  = isi->xpoints;
	int*                 xapoints = isi->xapoints;
	int*                 yapoints = isi->yapoints;

	/* go through every scanline in the output buffer */
	for (int y = 0; y < dh; y++) {
		int Cy  = yapoints[y] >> 16;
		int yap = yapoints[y] & 0xffff;

		unsigned int* dptr = dest + (y * dow);
		for (int x = 0; x < dw; x++) {
			const unsigned int* sptr = ypoints[y] + xpoints[x];
			int                 r, g, b;
			qt_qimageScaleAARGB_helper(sptr, yap, Cy, sow, &r, &g, &b);

			int xap = xapoints[x];
			if (xap > 0) {
				int rr, bb, gg;
				qt_qimageScaleAARGB_helper(sptr + 1, yap, Cy, sow, &rr, &gg, &bb);

				r = r * (256 - xap);
				g = g * (256 - xap);
				b = b * (256 - xap);
				r = (r + (rr * xap)) >> 8;
				g = (g + (gg * xap)) >> 8;
				b = (b + (bb * xap)) >> 8;
			}
			*dptr++ = qRgb(r >> 14, g >> 14, b >> 14);
		}
	}
}
#endif

#if defined(FBINK_QIS_NO_SIMD) || !(defined(__SSE4_1__) || defined(__ARM_NEON__))
static void
    qt_qimageScaleAARGB_down_x_up_y(QImageScaleInfo* isi, unsigned int* dest, int dw, int dh, int dow, int sow)
{
	const unsigned int** ypoints  = isi->ypoints;
	int*                 xpoints  = isi->xpoints;
	int*                 xapoints = isi->xapoints;
	int*                 yapoints = isi->yapoints;

	/* go through every scanline in the output buffer */
	for (int y = 0; y < dh; y++) {
		unsigned int* dptr = dest + (y * dow);
		for (int x = 0; x < dw; x++) {
			int Cx  = xapoints[x] >> 16;
			int xap = xapoints[x] & 0xffff;

			const unsigned int* sptr = ypoints[y] + xpoints[x];
			int                 r, g, b;
			qt_qimageScaleAARGB_helper(sptr, xap, Cx, 1, &r, &g, &b);

			int yap = yapoints[y];
			if (yap > 0) {
				int rr, bb, gg;
				qt_qimageScaleAARGB_helper(sptr + sow, xap, Cx, 1, &rr, &gg, &bb);

				r = r * (256 - yap);
				g = g * (256 - yap);
				b = b * (256 - yap);
				r = (r + (rr * yap)) >> 8;
				g = (g + (gg * yap)) >> 8;
				b = (b + (bb * yap)) >> 8;
			}
			*dptr++ = qRgb(r >> 14, g >> 14, b >> 14);
		}
	}
}
#endif

#if defined(FBINK_QIS_NO_SIMD) || !(defined(__SSE4_1__) || defined(__ARM_NEON__))
static void
    qt_qimageScaleAARGB_down_xy(QImageScaleInfo* isi, unsigned int* dest, int dw, int dh, int dow, int sow)
{
	const unsigned int** ypoints  = isi->ypoints;
	int*                 xpoints  = isi->xpoints;
	int*                 xapoints = isi->xapoints;
	int*                 yapoints = isi->yapoints;

	for (int y = 0; y < dh; y++) {
		int Cy  = yapoints[y] >> 16;
		int yap = yapoints[y] & 0xffff;

		unsigned int* dptr = dest + (y * dow);
		for (int x = 0; x < dw; x++) {
			int Cx  = xapoints[x] >> 16;
			int xap = xapoints[x] & 0xffff;

			const unsigned int* sptr = ypoints[y] + xpoints[x];
			int                 rx, gx, bx;
			qt_qimageScaleAARGB_helper(sptr, xap, Cx, 1, &rx, &gx, &bx);

			int r = (rx >> 4) * yap;
			int g = (gx >> 4) * yap;
			int b = (bx >> 4) * yap;

			int j;
			for (j = (1 << 14) - yap; j > Cy; j -= Cy) {
				sptr += sow;
				qt_qimageScaleAARGB_helper(sptr, xap, Cx, 1, &rx, &gx, &bx);

				r += (rx >> 4) * Cy;
				g += (gx >> 4) * Cy;
				b += (bx >> 4) * Cy;
			}
			sptr += sow;
			qt_qimageScaleAARGB_helper(sptr, xap, Cx, 1, &rx, &gx, &bx);

			r += (rx >> 4) * j;
			g += (gx >> 4) * j;
			b += (bx >> 4) * j;

			*dptr = qRgb(r >> 24, g >> 24, b >> 24);
			dptr++;
		}
	}
}
#endif

inline static void
    qt_qimageScaleAAY8_helper(const unsigned char* pix, int xyap, int Cxy, int step, int* v)
{
	*v = *pix * xyap;
	int j;
	for (j = (1 << 14) - xyap; j > Cxy; j -= Cxy) {
		pix += step;
		*v += *pix * Cxy;
	}
	pix += step;
	*v += *pix * j;
}

static void
    qt_qimageScaleAAY8_up_xy(QImageScaleInfo* isi, unsigned char* dest, int dw, int dh, int dow, int sow)
{
	const unsigned char** ypoints  = (const unsigned char**) isi->ypoints_y8;
	int*                  xpoints  = isi->xpoints;
	int*                  xapoints = isi->xapoints;
	int*                  yapoints = isi->yapoints;

	/* go through every scanline in the output buffer */
	for (int y = 0; y < dh; y++) {
		/* calculate the source line we'll scan from */
		const unsigned char* sptr = ypoints[y];
		unsigned char*       dptr = dest + (y * dow);
		const int            yap  = yapoints[y];
		if (yap > 0) {
			for (int x = 0; x < dw; x++) {
				const unsigned char* pix = sptr + xpoints[x];
				const int            xap = xapoints[x];
				if (xap > 0)
					*dptr = interpolate_4_8bpp_pixels_arr(pix, pix + sow, xap, yap);
				else
					*dptr = INTERPOLATE_8BPP_PIXEL_256(pix[0], 256 - yap, pix[sow], yap);
				dptr++;
			}
		} else {
			for (int x = 0; x < dw; x++) {
				const unsigned char* pix = sptr + xpoints[x];
				const int            xap = xapoints[x];
				if (xap > 0)
					*dptr = INTERPOLATE_8BPP_PIXEL_256(pix[0], 256 - xap, pix[1], xap);
				else
					*dptr = pix[0];
				dptr++;
			}
		}
	}
}

static void
    qt_qimageScaleAAY8_up_x_down_y(QImageScaleInfo* isi, unsigned char* dest, int dw, int dh, int dow, int sow)
{
	const unsigned char** ypoints  = (const unsigned char**) isi->ypoints_y8;
	int*                  xpoints  = isi->xpoints;
	int*                  xapoints = isi->xapoints;
	int*                  yapoints = isi->yapoints;

	for (int y = 0; y < dh; y++) {
		int Cy  = (yapoints[y]) >> 16;
		int yap = (yapoints[y]) & 0xffff;

		unsigned char* dptr = dest + (y * dow);
		for (int x = 0; x < dw; x++) {
			const unsigned char* sptr = ypoints[y] + xpoints[x];
			int                  v;
			qt_qimageScaleAAY8_helper(sptr, yap, Cy, sow, &v);

			int xap = xapoints[x];
			if (xap > 0) {
				int vv;
				qt_qimageScaleAAY8_helper(sptr + 1, yap, Cy, sow, &vv);

				v = v * (256 - xap);
				v = (v + (vv * xap)) >> 8;
			}
			*dptr++ = (v >> 14);
		}
	}
}

static void
    qt_qimageScaleAAY8_down_x_up_y(QImageScaleInfo* isi, unsigned char* dest, int dw, int dh, int dow, int sow)
{
	const unsigned char** ypoints  = (const unsigned char**) isi->ypoints_y8;
	int*                  xpoints  = isi->xpoints;
	int*                  xapoints = isi->xapoints;
	int*                  yapoints = isi->yapoints;

	/* go through every scanline in the output buffer */
	for (int y = 0; y < dh; y++) {
		unsigned char* dptr = dest + (y * dow);
		for (int x = 0; x < dw; x++) {
			int Cx  = xapoints[x] >> 16;
			int xap = xapoints[x] & 0xffff;

			const unsigned char* sptr = ypoints[y] + xpoints[x];
			int                  v;
			qt_qimageScaleAAY8_helper(sptr, xap, Cx, 1, &v);

			int yap = yapoints[y];
			if (yap > 0) {
				int vv;
				qt_qimageScaleAAY8_helper(sptr + sow, xap, Cx, 1, &vv);

				v = v * (256 - yap);
				v = (v + (vv * yap)) >> 8;
			}
			*dptr = (v >> 14);
			dptr++;
		}
	}
}

static void
    qt_qimageScaleAAY8_down_xy(QImageScaleInfo* isi, unsigned char* dest, int dw, int dh, int dow, int sow)
{
	const unsigned char** ypoints  = (const unsigned char**) isi->ypoints_y8;
	int*                  xpoints  = isi->xpoints;
	int*                  xapoints = isi->xapoints;
	int*                  yapoints = isi->yapoints;

	for (int y = 0; y < dh; y++) {
		int Cy  = (yapoints[y]) >> 16;
		int yap = (yapoints[y]) & 0xffff;

		unsigned char* dptr = dest + (y * dow);
		for (int x = 0; x < dw; x++) {
			int Cx  = xapoints[x] >> 16;
			int xap = xapoints[x] & 0xffff;

			const unsigned char* sptr = ypoints[y] + xpoints[x];
			int                  vx;
			qt_qimageScaleAAY8_helper(sptr, xap, Cx, 1, &vx);

			int v = ((vx >> 6) * yap);

			int j;
			for (j = (1 << 14) - yap; j > Cy; j -= Cy) {
				sptr += sow;
				qt_qimageScaleAAY8_helper(sptr, xap, Cx, 1, &vx);
				v += ((vx >> 6) * Cy);
			}
			sptr += sow;
			qt_qimageScaleAAY8_helper(sptr, xap, Cx, 1, &vx);

			v += ((vx >> 6) * j);

			v     = DIV255(v >> 14);
			*dptr = v > 0xFF ? 0xFF : v < 0 ? 0 : v;
			dptr++;
		}
	}
}

static void
    qt_qimageScaleAAY8(QImageScaleInfo* isi, unsigned char* dest, int dw, int dh, int dow, int sow)
{
	if (isi->xup_yup == 3)
		qt_qimageScaleAAY8_up_xy(isi, dest, dw, dh, dow, sow);
	else if (isi->xup_yup == 1)
		qt_qimageScaleAAY8_up_x_down_y(isi, dest, dw, dh, dow, sow);
	else if (isi->xup_yup == 2)
		qt_qimageScaleAAY8_down_x_up_y(isi, dest, dw, dh, dow, sow);
	else
		qt_qimageScaleAAY8_down_xy(isi, dest, dw, dh, dow, sow);
}

inline static void
    qt_qimageScaleAAY8A_helper(const unsigned short* pix, int xyap, int Cxy, int step, int* v, int* a)
{
	*v = qY(*pix) * xyap;
	*a = qA(*pix) * xyap;
	int j;
	for (j = (1 << 14) - xyap; j > Cxy; j -= Cxy) {
		pix += step;
		*v += qY(*pix) * Cxy;
		*a += qA(*pix) * Cxy;
	}
	pix += step;
	*v += qY(*pix) * j;
	*a += qA(*pix) * j;
}

static void
    qt_qimageScaleAAY8A_up_xy(QImageScaleInfo* isi, unsigned short* dest, int dw, int dh, int dow, int sow)
{
	const unsigned short** ypoints  = (const unsigned short**) isi->ypoints_y8a;
	int*                   xpoints  = isi->xpoints;
	int*                   xapoints = isi->xapoints;
	int*                   yapoints = isi->yapoints;

	/* go through every scanline in the output buffer */
	for (int y = 0; y < dh; y++) {
		/* calculate the source line we'll scan from */
		const unsigned short* sptr = ypoints[y];
		unsigned short*       dptr = dest + (y * dow);
		const int             yap  = yapoints[y];
		if (yap > 0) {
			for (int x = 0; x < dw; x++) {
				const unsigned short* pix = sptr + xpoints[x];
				const int             xap = xapoints[x];
				if (xap > 0)
					*dptr = interpolate_4_16bpp_pixels_arr(pix, pix + sow, xap, yap);
				else
					*dptr = INTERPOLATE_16BPP_PIXEL_256(pix[0], 256 - yap, pix[sow], yap);
				dptr++;
			}
		} else {
			for (int x = 0; x < dw; x++) {
				const unsigned short* pix = sptr + xpoints[x];
				const int             xap = xapoints[x];
				if (xap > 0)
					*dptr = INTERPOLATE_16BPP_PIXEL_256(pix[0], 256 - xap, pix[1], xap);
				else
					*dptr = pix[0];
				dptr++;
			}
		}
	}
}

static void
    qt_qimageScaleAAY8A_up_x_down_y(QImageScaleInfo* isi, unsigned short* dest, int dw, int dh, int dow, int sow)
{
	const unsigned short** ypoints  = (const unsigned short**) isi->ypoints_y8a;
	int*                   xpoints  = isi->xpoints;
	int*                   xapoints = isi->xapoints;
	int*                   yapoints = isi->yapoints;

	for (int y = 0; y < dh; y++) {
		int Cy  = (yapoints[y]) >> 16;
		int yap = (yapoints[y]) & 0xffff;

		unsigned short* dptr = dest + (y * dow);
		for (int x = 0; x < dw; x++) {
			const unsigned short* sptr = ypoints[y] + xpoints[x];
			int                   v, a;
			qt_qimageScaleAAY8A_helper(sptr, yap, Cy, sow, &v, &a);

			int xap = xapoints[x];
			if (xap > 0) {
				int vv, aa;
				qt_qimageScaleAAY8A_helper(sptr + 1, yap, Cy, sow, &vv, &aa);

				v = v * (256 - xap);
				a = a * (256 - xap);
				v = (v + (vv * xap)) >> 8;
				a = (a + (aa * xap)) >> 8;
			}
			*dptr++ = qY8A(v >> 14, a >> 14);
		}
	}
}

static void
    qt_qimageScaleAAY8A_down_x_up_y(QImageScaleInfo* isi, unsigned short* dest, int dw, int dh, int dow, int sow)
{
	const unsigned short** ypoints  = (const unsigned short**) isi->ypoints_y8a;
	int*                   xpoints  = isi->xpoints;
	int*                   xapoints = isi->xapoints;
	int*                   yapoints = isi->yapoints;

	/* go through every scanline in the output buffer */
	for (int y = 0; y < dh; y++) {
		unsigned short* dptr = dest + (y * dow);
		for (int x = 0; x < dw; x++) {
			int Cx  = xapoints[x] >> 16;
			int xap = xapoints[x] & 0xffff;

			const unsigned short* sptr = ypoints[y] + xpoints[x];
			int                   v, a;
			qt_qimageScaleAAY8A_helper(sptr, xap, Cx, 1, &v, &a);

			int yap = yapoints[y];
			if (yap > 0) {
				int vv, aa;
				qt_qimageScaleAAY8A_helper(sptr + sow, xap, Cx, 1, &vv, &aa);

				v = v * (256 - yap);
				a = a * (256 - yap);
				v = (v + (vv * yap)) >> 8;
				a = (a + (aa * yap)) >> 8;
			}
			*dptr = qY8A(v >> 14, a >> 14);
			dptr++;
		}
	}
}

static void
    qt_qimageScaleAAY8A_down_xy(QImageScaleInfo* isi, unsigned short* dest, int dw, int dh, int dow, int sow)
{
	const unsigned short** ypoints  = (const unsigned short**) isi->ypoints_y8a;
	int*                   xpoints  = isi->xpoints;
	int*                   xapoints = isi->xapoints;
	int*                   yapoints = isi->yapoints;

	for (int y = 0; y < dh; y++) {
		int Cy  = (yapoints[y]) >> 16;
		int yap = (yapoints[y]) & 0xffff;

		unsigned short* dptr = dest + (y * dow);
		for (int x = 0; x < dw; x++) {
			int Cx  = xapoints[x] >> 16;
			int xap = xapoints[x] & 0xffff;

			const unsigned short* sptr = ypoints[y] + xpoints[x];
			int                   vx, ax;
			qt_qimageScaleAAY8A_helper(sptr, xap, Cx, 1, &vx, &ax);

			int v = ((vx >> 6) * yap);
			int a = ((ax >> 6) * yap);

			int j;
			for (j = (1 << 14) - yap; j > Cy; j -= Cy) {
				sptr += sow;
				qt_qimageScaleAAY8A_helper(sptr, xap, Cx, 1, &vx, &ax);
				v += ((vx >> 6) * Cy);
				a += ((ax >> 6) * Cy);
			}
			sptr += sow;
			qt_qimageScaleAAY8A_helper(sptr, xap, Cx, 1, &vx, &ax);

			v += ((vx >> 6) * j);
			v = DIV255(v >> 14);
			v = v > 0xFF ? 0xFF : v < 0 ? 0 : v;
			a += ((ax >> 6) * j);
			a = DIV255(a >> 14);
			a = a > 0xFF ? 0xFF : a < 0 ? 0 : a;

			*dptr = qY8A(v, a);
			dptr++;
		}
	}
}

static void
    qt_qimageScaleAAY8A(QImageScaleInfo* isi, unsigned short* dest, int dw, int dh, int dow, int sow)
{
	if (isi->xup_yup == 3)
		qt_qimageScaleAAY8A_up_xy(isi, dest, dw, dh, dow, sow);
	else if (isi->xup_yup == 1)
		qt_qimageScaleAAY8A_up_x_down_y(isi, dest, dw, dh, dow, sow);
	else if (isi->xup_yup == 2)
		qt_qimageScaleAAY8A_down_x_up_y(isi, dest, dw, dh, dow, sow);
	else
		qt_qimageScaleAAY8A_down_xy(isi, dest, dw, dh, dow, sow);
}

unsigned char*
    qSmoothScaleImage(const unsigned char* src, int sw, int sh, int sn, bool ignore_alpha, int dw, int dh)
{
	unsigned char* buffer = NULL;
	if (src == NULL || dw <= 0 || dh <= 0)
		return buffer;

	QImageScaleInfo* scaleinfo = qimageCalcScaleInfo(src, sw, sh, sn, dw, dh, true);
	if (!scaleinfo)
		return buffer;

	// NOTE: For RGB/RGBA input, output format is always RGBA!
	//       In case our input was RGB, we've already ensured that our input buffer is already 32bpp,
	//       c.f., comments below.
	// SSE/NEON friendly alignment, just in case...
	void* ptr;
	if (posix_memalign(&ptr, 16, (size_t)(dw * dh * sn)) != 0) {
		fprintf(stderr, "qSmoothScaleImage: out of memory, returning null!\n");
		qimageFreeScaleInfo(scaleinfo);
		return NULL;
	} else {
		buffer = (unsigned char*) ptr;
	}

	// NOTE: In the same way, we enforce 32bpp input buffers for RGB,
	//       because that's what Qt uses, even for RGB with no alpha.
	//       (the pixelformat constant is helpfully named RGB32 to remind you of that ;)).
	//       This is why we'll never get sn == 3 here, FBInk takes care of never allowing that to happen.
	// NOTE: See comment in qimageCalcScaleInfo regarding our simplification of using sw directly.
	switch (sn) {
		case 4:
			if (ignore_alpha) {
				// NOTE: Input buffer is still 32bpp, we just skip *processing* of the alpha channel.
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wcast-align"
				qt_qimageScaleAARGB(scaleinfo, (unsigned int*) buffer, dw, dh, dw, sw);
#pragma GCC diagnostic pop
			} else {
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wcast-align"
				qt_qimageScaleAARGBA(scaleinfo, (unsigned int*) buffer, dw, dh, dw, sw);
#pragma GCC diagnostic pop
			}
			break;
		case 2:
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wcast-align"
			qt_qimageScaleAAY8A(scaleinfo, (unsigned short*) buffer, dw, dh, dw, sw);
#pragma GCC diagnostic pop
			break;
		case 1:
			qt_qimageScaleAAY8(scaleinfo, (unsigned char*) buffer, dw, dh, dw, sw);
			break;
	}

	qimageFreeScaleInfo(scaleinfo);
	return buffer;
}
