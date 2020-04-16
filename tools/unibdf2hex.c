/*
   unibdf2hex - program to convert a BDF file into a unifont.hex file.

   Author: Paul Hardy, January 2008

   Note: currently this has hard-coded code points for glyphs extracted
   from Wen Quan Yi to create the Unifont source file "wqy.hex".

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

#define UNISTART 0x3400
#define UNISTOP 0x4DBF

#define MAXBUF 256


int
main()
{
   int i;
   int digitsout;  /* how many hex digits we output in a bitmap */
   int thispoint;
   char inbuf[MAXBUF];
   int bbxx, bbxy, bbxxoff, bbxyoff;

   int descent=4; /* font descent wrt baseline */
   int startrow;  /* row to start glyph        */
   unsigned rowout;

   while (fgets (inbuf, MAXBUF - 1, stdin) != NULL) {
      if (strncmp (inbuf, "ENCODING ", 9) == 0) {
         sscanf (&inbuf[9], "%d", &thispoint); /* get code point */
         /*
            If we want this code point, get the BBX (bounding box) and
            BITMAP information.
         */
         if ((thispoint >= 0x2E80 && thispoint <= 0x2EFF) || // CJK Radicals Supplement
             (thispoint >= 0x2F00 && thispoint <= 0x2FDF) || // Kangxi Radicals
             (thispoint >= 0x2FF0 && thispoint <= 0x2FFF) || // Ideographic Description Characters
             (thispoint >= 0x3001 && thispoint <= 0x303F) || // CJK Symbols and Punctuation (U+3000 is a space)
             (thispoint >= 0x3100 && thispoint <= 0x312F) || // Bopomofo
             (thispoint >= 0x31A0 && thispoint <= 0x31BF) || // Bopomofo extend
             (thispoint >= 0x31C0 && thispoint <= 0x31EF) || // CJK Strokes
             (thispoint >= 0x3400 && thispoint <= 0x4DBF) || // CJK Unified Ideographs Extension A
             (thispoint >= 0x4E00 && thispoint <= 0x9FCF) || // CJK Unified Ideographs
             (thispoint >= 0xF900 && thispoint <= 0xFAFF))   // CJK Compatibility Ideographs
            {
            while (fgets (inbuf, MAXBUF - 1, stdin) != NULL &&
                   strncmp (inbuf, "BBX ", 4) != 0); /* find bounding box */

            sscanf (&inbuf[4], "%d %d %d %d", &bbxx, &bbxy, &bbxxoff, &bbxyoff);
            while (fgets (inbuf, MAXBUF - 1, stdin) != NULL &&
                   strncmp (inbuf, "BITMAP", 6) != 0); /* find bitmap start */
            fprintf (stdout, "%04X:", thispoint);
            digitsout = 0;
            /* Print initial blank rows */
            startrow = descent + bbxyoff + bbxy;

            /* Force everything to 16 pixels wide */
            for (i = 16; i > startrow; i--) {
               fprintf (stdout,"0000");
               digitsout += 4;
            }
            while (fgets (inbuf, MAXBUF - 1, stdin) != NULL &&
                   strncmp (inbuf, "END", 3) != 0) { /* copy bitmap until END */
               sscanf (inbuf, "%X", &rowout);
               /* Now force glyph to a 16x16 grid even if they'd fit in 8x16 */
               if (bbxx <= 8) rowout <<= 8;  /* shift left for 16x16 glyph */
               rowout >>= bbxxoff;
               fprintf (stdout, "%04X", rowout);
               digitsout += 4;
            }

            /* Pad for 16x16 glyph */
            while (digitsout < 64) {
               fprintf (stdout,"0000");
               digitsout += 4;
            }
            fprintf (stdout,"\n");
         }
      }
   }
   exit (0);
}
