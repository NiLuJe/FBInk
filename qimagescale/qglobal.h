/****************************************************************************
**
** Copyright (C) 2016 The Qt Company Ltd.
** Copyright (C) 2016 Intel Corporation.
** Contact: https://www.qt.io/licensing/
** SPDX-License-Identifier: LGPL-3.0-only
**
** This file is part of the QtCore module of the Qt Toolkit.
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

#ifndef QGLOBAL_H
#define QGLOBAL_H

// NOTE: Using stdint.h will ensure consistent results, no matter the data model.
//       Not that we care terribly about that, since we're Linux only,
//       and the only "tricky" case for this is usually Win64 ;).
//       c.f., https://en.cppreference.com/w/cpp/language/types
//           & https://docs.microsoft.com/en-us/cpp/c-runtime-library/standard-types?view=vs-2019
#include <stdint.h>
typedef int64_t  qint64;  /* 64 bit signed */
typedef uint64_t quint64; /* 64 bit unsigned */

/*
   Useful type definitions for Qt
*/

typedef unsigned char  uchar;
typedef unsigned short ushort;
typedef unsigned int   uint;
typedef unsigned long  ulong;

/*
   Utility macros and inline functions
*/

#define qAbs(T)                                                                                                          \
	({                                                                                                               \
		__auto_type __t__ = (T);                                                                                 \
		__t__ >= 0 ? __t__ : -__t__;                                                                             \
	})

#define qMin(A, B)                                                                                                       \
	({                                                                                                               \
		__auto_type __a = (A);                                                                                   \
		__auto_type __b = (B);                                                                                   \
		(__a < __b) ? __a : __b;                                                                                 \
	})

#define qMax(A, B)                                                                                                       \
	({                                                                                                               \
		__auto_type a__ = (A);                                                                                   \
		__auto_type b__ = (B);                                                                                   \
		(a__ < b__) ? b__ : a__;                                                                                 \
	})

#endif /* QGLOBAL_H */
