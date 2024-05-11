/*
	FBInk: FrameBuffer eInker, a library to print text & images to an eInk Linux framebuffer
	Copyright (C) 2024 NiLuJe <ninuje@gmail.com>
	SPDX-License-Identifier: GPL-3.0-or-later

	----

	This program is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/

#ifndef __FBINK_STRING_UTILS_H
#define __FBINK_STRING_UTILS_H

// Mainly to make IDEs happy
#include "fbink.h"
#include "fbink_internal.h"

#include <string.h>

// NOTE: We could mostly use strcpy instead in fbink_device_id,
//       but for some reason -Wstringop-overflow warnings disappear as soon as optimization is involved...
static ssize_t
    strtcpy(char* restrict dst, const char* restrict src, size_t dsize)
{
	if (dsize == 0U) {
		errno = ENOBUFS;
		return -1;
	}

	size_t slen  = strnlen(src, dsize);
	bool   trunc = (slen == dsize);
	size_t dlen  = slen - trunc;

	stpcpy(mempcpy(dst, src, dlen), "");
	if (trunc) {
		errno = E2BIG;
	}
	return trunc ? -1 : (ssize_t) slen;
}

// c.f., string_copying(7)
static char*
    stpecpy(char* dst, char end[0], const char* restrict src)
{
	if (dst == NULL) {
		return NULL;
	}

	ssize_t dlen = strtcpy(dst, src, (uintptr_t) (end - dst));
	return (dlen == -1) ? NULL : dst + dlen;
}

// Based on Linux's strscpy_pad
// NOTE: We don't actually have a use for it right now, but I like it and my brain is a sieve, so, in it goes.
/*
static ssize_t
    strtcpy_pad(char* restrict dst, const char* restrict src, size_t dsize)
{
	ssize_t written;

	written = strtcpy(dst, src, dsize);
	if (written < 0 || written == dsize - 1) {
		return written;
	}

	memset(dst + written + 1, 0, dsize - written - 1);

	return written;
}
*/

#endif
