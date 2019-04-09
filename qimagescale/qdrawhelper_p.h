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

#ifndef QDRAWHELPER_P_H
#define QDRAWHELPER_P_H

#include "qglobal.h"
#if defined(__ARM_NEON) || defined(__ARM_NEON__)
#include <arm_neon.h>
#endif

#if __SIZEOF_POINTER__ == 8 // 64-bit versions

static inline __attribute__((always_inline)) uint INTERPOLATE_PIXEL_256(uint x, uint a, uint y, uint b) {
    quint64 t = (((quint64(x)) | ((quint64(x)) << 24)) & 0x00ff00ff00ff00ff) * a;
    t += (((quint64(y)) | ((quint64(y)) << 24)) & 0x00ff00ff00ff00ff) * b;
    t >>= 8;
    t &= 0x00ff00ff00ff00ff;
    return (uint(t)) | (uint(t >> 24));
}

#else // 32-bit versions

static inline __attribute__((always_inline)) uint INTERPOLATE_PIXEL_256(uint x, uint a, uint y, uint b) {
    uint t = (x & 0xff00ff) * a + (y & 0xff00ff) * b;
    t >>= 8;
    t &= 0xff00ff;

    x = ((x >> 8) & 0xff00ff) * a + ((y >> 8) & 0xff00ff) * b;
    x &= 0xff00ff00;
    x |= t;
    return x;
}

#endif

#if defined(__SSE2__)
static inline __attribute__((always_inline)) uint interpolate_4_pixels_sse2(__m128i vt, __m128i vb, uint distx, uint disty)
{
    // First interpolate top and bottom pixels in parallel.
    vt = _mm_unpacklo_epi8(vt, _mm_setzero_si128());
    vb = _mm_unpacklo_epi8(vb, _mm_setzero_si128());
    vt = _mm_mullo_epi16(vt, _mm_set1_epi16(256 - disty));
    vb = _mm_mullo_epi16(vb, _mm_set1_epi16(disty));
    __m128i vlr = _mm_add_epi16(vt, vb);
    vlr = _mm_srli_epi16(vlr, 8);
    // vlr now contains the result of the first two interpolate calls vlr = unpacked((xright << 64) | xleft)

    // Now the last interpolate between left and right..
    const __m128i vidistx = _mm_shufflelo_epi16(_mm_cvtsi32_si128(256 - distx), _MM_SHUFFLE(0, 0, 0, 0));
    const __m128i vdistx = _mm_shufflelo_epi16(_mm_cvtsi32_si128(distx), _MM_SHUFFLE(0, 0, 0, 0));
    const __m128i vmulx = _mm_unpacklo_epi16(vidistx, vdistx);
    vlr = _mm_unpacklo_epi16(vlr, _mm_srli_si128(vlr, 8));
    // vlr now contains the colors of left and right interleaved { la, ra, lr, rr, lg, rg, lb, rb }
    vlr = _mm_madd_epi16(vlr, vmulx); // Multiply and horizontal add.
    vlr = _mm_srli_epi32(vlr, 8);
    vlr = _mm_packs_epi32(vlr, vlr);
    vlr = _mm_packus_epi16(vlr, vlr);
    return _mm_cvtsi128_si32(vlr);
}

static inline uint interpolate_4_pixels(uint tl, uint tr, uint bl, uint br, uint distx, uint disty)
{
    __m128i vt = _mm_unpacklo_epi32(_mm_cvtsi32_si128(tl), _mm_cvtsi32_si128(tr));
    __m128i vb = _mm_unpacklo_epi32(_mm_cvtsi32_si128(bl), _mm_cvtsi32_si128(br));
    return interpolate_4_pixels_sse2(vt, vb, distx, disty);
}

static inline uint interpolate_4_pixels(const uint t[], const uint b[], uint distx, uint disty)
{
    __m128i vt = _mm_loadl_epi64((const __m128i*)t);
    __m128i vb = _mm_loadl_epi64((const __m128i*)b);
    return interpolate_4_pixels_sse2(vt, vb, distx, disty);
}

static constexpr inline bool hasFastInterpolate4() { return true; }

#elif defined(__ARM_NEON__)
static inline __attribute__((always_inline)) uint interpolate_4_pixels_neon(uint32x2_t vt32, uint32x2_t vb32, uint distx, uint disty)
{
    uint16x8_t vt16 = vmovl_u8(vreinterpret_u8_u32(vt32));
    uint16x8_t vb16 = vmovl_u8(vreinterpret_u8_u32(vb32));
    vt16 = vmulq_n_u16(vt16, 256 - disty);
    vt16 = vmlaq_n_u16(vt16, vb16, disty);
    vt16 = vshrq_n_u16(vt16, 8);
    uint16x4_t vl16 = vget_low_u16(vt16);
    uint16x4_t vr16 = vget_high_u16(vt16);
    vl16 = vmul_n_u16(vl16, 256 - distx);
    vl16 = vmla_n_u16(vl16, vr16, distx);
    vl16 = vshr_n_u16(vl16, 8);
    uint8x8_t vr = vmovn_u16(vcombine_u16(vl16, vl16));
    return vget_lane_u32(vreinterpret_u32_u8(vr), 0);
}

static inline uint interpolate_4_pixels(uint tl, uint tr, uint bl, uint br, uint distx, uint disty)
{
    uint32x2_t vt32 = vmov_n_u32(tl);
    uint32x2_t vb32 = vmov_n_u32(bl);
    vt32 = vset_lane_u32(tr, vt32, 1);
    vb32 = vset_lane_u32(br, vb32, 1);
    return interpolate_4_pixels_neon(vt32, vb32, distx, disty);
}

static inline uint interpolate_4_pixels(const uint t[], const uint b[], uint distx, uint disty)
{
    uint32x2_t vt32 = vld1_u32(t);
    uint32x2_t vb32 = vld1_u32(b);
    return interpolate_4_pixels_neon(vt32, vb32, distx, disty);
}

static constexpr inline bool hasFastInterpolate4() { return true; }

#else
static inline uint interpolate_4_pixels(uint tl, uint tr, uint bl, uint br, uint distx, uint disty)
{
    uint idistx = 256 - distx;
    uint idisty = 256 - disty;
    uint xtop = INTERPOLATE_PIXEL_256(tl, idistx, tr, distx);
    uint xbot = INTERPOLATE_PIXEL_256(bl, idistx, br, distx);
    return INTERPOLATE_PIXEL_256(xtop, idisty, xbot, disty);
}

static inline uint interpolate_4_pixels(const uint t[], const uint b[], uint distx, uint disty)
{
    return interpolate_4_pixels(t[0], t[1], b[0], b[1], distx, disty);
}

static constexpr inline bool hasFastInterpolate4() { return false; }

#endif

#endif // QDRAWHELPER_P_H
