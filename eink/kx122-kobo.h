/*
The MIT License (MIT)
Copyright (c) 2016 Kionix Inc.
Permission is hereby granted, free of charge, to any person obtaining a
copy of this software and associated documentation files (the
"Software"), to deal in the Software without restriction, including
without limitation the rights to use, copy, modify, merge, publish,
distribute, sublicense, and/or sell copies of the Software, and to
permit persons to whom the Software is furnished to do so, subject to
the following conditions:
The above copyright notice and this permission notice shall be included
in all copies or substantial portions of the Software.
THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#ifndef __KX122_REGISTERS_H__
#define __KX122_REGISTERS_H__

//
// Excerpt from "drivers/input/sensor/kx122_registers.h"
//

/* registers */
// WHO_AM_I
#define KX122_WHO_AM_I 0x0F
// current sixfacet posititions
#define KX122_TSCP 0x10
// previous six facet positions
#define KX122_TSPP 0x11

/* registers bits */
// WHO_AM_I -value for KX122
#define KX122_WHO_AM_I_WIA_ID (0x1B << 0)

// x-left
#define KX122_TSCP_LE (0x01 << 5)
// x+right
#define KX122_TSCP_RI (0x01 << 4)
// y-down
#define KX122_TSCP_DO (0x01 << 3)
// y+up
#define KX122_TSCP_UP (0x01 << 2)
// z-facedown
#define KX122_TSCP_FD (0x01 << 1)
// z+faceup
#define KX122_TSCP_FU (0x01 << 0)

// x-left
#define KX122_TSPP_LE (0x01 << 5)
// x+right
#define KX122_TSPP_RI (0x01 << 4)
// y-down
#define KX122_TSPP_DO (0x01 << 3)
// y+up
#define KX122_TSPP_UP (0x01 << 2)
// z-facedown
#define KX122_TSPP_FD (0x01 << 1)
// z+faceup
#define KX122_TSPP_FU (0x01 << 0)

#endif // __KX122_REGISTERS_H__
