/*
   unibdf2hex - program to convert a BDF file into a unifont.hex file.

   Author: Paul Hardy, January 2008

   Copyright (C) 2008, 2013 Paul Hardy

   LICENSE:

      This program is free software: you can redistribute it and/or modify
      it under the terms of the GNU General Public License as published by
      the Free Software Foundation, either version 2 of the License, or
      (at your option) any later version.

      This program is distributed in the hope that it will be useful,
      but WITHOUT ANY WARRANTY; without even the implied warranty of
      MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
      GNU General Public License for more details.

      You should have received a copy of the GNU General Public License
      along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define UNISTART 0x0000
#define UNISTOP  0xFFFF

#define MAXBUF 256

int
    main()
{
	int  i;
	int  digitsout; /* how many hex digits we output in a bitmap */
	int  thispoint;
	char inbuf[MAXBUF];
	int  bbxx, bbxy, bbxxoff, bbxyoff;

	int      descent = 3; /* font descent wrt baseline */
	int      startrow;    /* row to start glyph        */
	unsigned rowout;

	while (fgets(inbuf, MAXBUF - 1, stdin) != NULL) {
		if (strncmp(inbuf, "ENCODING ", 9) == 0) {
			sscanf(&inbuf[9], "%d", &thispoint); /* get code point */
			/* If we want this code point, get the BBX (bounding box) and BITMAP information. */
			if (1)    // NOTE: Squished cp range check (might want to leave a thispoint <= UNISTOP?).
			{
				while (fgets(inbuf, MAXBUF - 1, stdin) != NULL && strncmp(inbuf, "BBX ", 4) != 0) {
					/* find bounding box */
					;
				}

				sscanf(&inbuf[4], "%d %d %d %d", &bbxx, &bbxy, &bbxxoff, &bbxyoff);
				while (fgets(inbuf, MAXBUF - 1, stdin) != NULL && strncmp(inbuf, "BITMAP", 6) != 0) {
					/* find bitmap start */
					;
				}
				fprintf(stdout, "%04X:", thispoint);
				digitsout = 0;
				/* Print initial blank rows */
				startrow = descent + bbxyoff + bbxy;
				// Recap metrics for debugging purposes...
				fprintf(stderr, "U+%04X metrics: %02dx%02d @ (%02d, %02d) => startrow: %02d\n", thispoint, bbxx, bbxy, bbxxoff, bbxyoff, startrow);

				/* Force everything to 13 pixels tall */
				for (i = 13; i > startrow; i--) {
					fprintf(stdout, "0000");
					digitsout += 4;
				}
				while (fgets(inbuf, MAXBUF - 1, stdin) != NULL && strncmp(inbuf, "END", 3) != 0) {
					/* copy bitmap until END */
					sscanf(inbuf, "%X", &rowout);
					/* Now force glyph to a 16x13 grid even if they'd fit in 8x13 */
					if (bbxx <= 8) {
						/* shift left for 16x13 glyph */
						rowout <<= 8;
					}
					rowout >>= bbxxoff;
					fprintf(stdout, "%04X", rowout);
					digitsout += 4;
				}

				/* Pad for x13 glyph */
				while (digitsout < (13 * 4)) {
					fprintf(stdout, "0000");
					digitsout += 4;
				}
				fprintf(stdout, "\n");
			}
		}
	}
	exit(0);
}
