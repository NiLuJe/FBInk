/*
   unibdf2hex - program to convert a BDF file into a unifont.hex file.

   Author: Paul Hardy, January 2008

   Copyright (C) 2008, 2013 Paul Hardy

   NOTE: I tweaked it to deal with the conversion of Cozette's BDF to HEX,
         which was all kinds of broken with gbdfed...

   Building:
       gcc -Wall -Wextra tools/unibdf2hex.c -o tools/unibdf2hex

   Usage:
       tools/unibdf2hex < fonts/cozette.bdf >| fonts/cozette-8x13.hex

   Don't forget to tweak the constants according to the font being converted...

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

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define UNISTOP UINT32_MAX

// Font-specific constants
#define DESCENT     3
#define BBOX_HEIGHT 13
// NOTE: Actually handling > 8px wide fonts requires more tweaking,
//       like restoring uint16_t padding & computations...
//       c.f., the original unibdf2hex version.
#define BBOX_WIDTH  8

#define MAXBUF 256

int
    main()
{
	int      i;
	size_t   rowcnt; /* how many rows we've output in a bitmap */
	uint32_t thispoint;
	uint32_t thishex;
	char     inbuf[MAXBUF];
	int      bbxx, bbxy, bbxxoff, bbxyoff;

	int      descent = DESCENT; /* font descent wrt baseline */
	int      startrow;          /* row to start glyph        */
	unsigned rowout;

	while (fgets(inbuf, MAXBUF - 1, stdin) != NULL) {
		// Cozette goes above U+FFFF, attempt to handle that...
		if (strncmp(inbuf, "STARTCHAR ", 10) == 0) {
			thishex = 0U;
			sscanf(&inbuf[10], "u%X", &thishex); /* get code point if in uFFFF format */
		}
		if (strncmp(inbuf, "ENCODING ", 9) == 0) {
			bool skip = false;
			sscanf(&inbuf[9], "%d", &thispoint); /* get code point (use %d because it may be -1!) */
			// If cp appears invalid, try to use the value from STARTCHAR...
			if (thispoint == UINT32_MAX && thishex != 0U) {
				thispoint = thishex;
			}
			if (thispoint < UNISTOP) {
				/* If we want this code point, get the BBX (bounding box) and BITMAP information. */
				while (fgets(inbuf, MAXBUF - 1, stdin) != NULL && strncmp(inbuf, "BBX ", 4) != 0) {
					/* find bounding box */
					;
				}

				sscanf(&inbuf[4], "%d %d %d %d", &bbxx, &bbxy, &bbxxoff, &bbxyoff);

				// Skip glyph if it's too wide (> BBOX_WIDTHpx)
				if (bbxx > BBOX_WIDTH) {
					fprintf(stderr, "Skipping U+%04X because it's too wide!\n", thispoint);
					skip = true;
				}

				while (fgets(inbuf, MAXBUF - 1, stdin) != NULL && strncmp(inbuf, "BITMAP", 6) != 0) {
					/* find bitmap start */
					;
				}
				if (!skip) {
					fprintf(stdout, "%04X:", thispoint);
				}
				rowcnt   = 0U;
				/* Print initial blank rows */
				startrow = descent + bbxyoff + bbxy;
				// Recap metrics for debugging purposes...
				fprintf(stderr,
					"U+%04X metrics: %02dx%02d @ (%02d, %02d) => startrow: %02d\n",
					thispoint,
					bbxx,
					bbxy,
					bbxxoff,
					bbxyoff,
					startrow);

				/* Force everything to BBOX_HEIGHT pixels tall */
				if (!skip) {
					for (i = BBOX_HEIGHT; i > startrow; i--) {
						fprintf(stdout, "00");
						rowcnt++;
					}
				}
				while (fgets(inbuf, MAXBUF - 1, stdin) != NULL && strncmp(inbuf, "END", 3) != 0) {
					/* copy bitmap until END */
					sscanf(inbuf, "%X", &rowout);
					/* Now force glyph to a (BBOX_WIDTH*2)xBBOX_HEIGHT grid even if they'd fit in BBOX_WIDTHxBBOX_HEIGHT */
					/*
					if (bbxx <= BBOX_WIDTH) {
						// shift left for (BBOX_WIDTH*2)xBBOX_HEIGHT glyph
						rowout <<= BBOX_WIDTH;
					}
					*/
					// Honor the glyph's advance if it's positive (avoids undefined behavior w/ the shift)...
					if (bbxxoff > 0) {
						rowout >>= bbxxoff;
					}
					if (!skip) {
						fprintf(stdout, "%02X", rowout);
						rowcnt++;
					}
				}

				/* Pad for xBBOX_HEIGHT glyph */
				if (!skip) {
					while (rowcnt < BBOX_HEIGHT) {
						fprintf(stdout, "00");
						rowcnt++;
					}
					fprintf(stdout, "\n");
				}
			}
		}
	}
	exit(0);
}
