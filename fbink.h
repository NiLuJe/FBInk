/*
	FBInk: FrameBuffer eInker, a tool to print strings on eInk devices (Kobo/Kindle)
	Copyright (C) 2018 NiLuJe <ninuje@gmail.com>

	Linux framebuffer routines based on: fbtestfnt.c & fbtest6.c, from
	http://raspberrycompote.blogspot.com/2014/04/low-level-graphics-on-raspberry-pi-text.html &
	https://raspberrycompote.blogspot.com/2013/03/low-level-graphics-on-raspberry-pi-part_8.html
	Original works by J-P Rosti (a.k.a -rst- and 'Raspberry Compote'),
	Licensed under the Creative Commons Attribution 3.0 Unported License
	(http://creativecommons.org/licenses/by/3.0/deed.en_US)

	----

	This program is free software: you can redistribute it and/or modify
	it under the terms of the GNU Affero General Public License as
	published by the Free Software Foundation, either version 3 of the
	License, or (at your option) any later version.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU Affero General Public License for more details.

	You should have received a copy of the GNU Affero General Public License
	along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef __FBINK_H
#define __FBINK_H

// What a FBInk Print config should look like
typedef struct {
	short int row;
	short int col;
	bool      is_inverted;
	bool      is_flashing;
	bool      is_cleared;
	bool      is_centered;
	bool      is_padded;
} FBInkConfig;

// Open the framebuffer device and returns its fd
int fbink_open(void);

// Initialize the global variables.
// If fd is -1, the fb is opened for the duration of this call
int fbink_init(int);

// Print a string on screen.
// if fd is -1, the fb is opened for the duration of this call
void fbink_print(int, char*, FBInkConfig*);

// When you intend to keep fd open for the lifecycle of your program:
// fd = open() -> init(fd) -> print(fd, ...)
//
// Otherwise:
// init(-1)
// And then whenever you want to print something:
// print(-1, ...)

#endif
