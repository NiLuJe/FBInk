/*
	FBInk: FrameBuffer eInker, a tool to print strings on eInk devices (Kobo/Kindle)
	Copyright (C) 2018 NiLuJe <ninuje@gmail.com>

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

#include "fbink_device_id.h"

// NOTE: This is lifted from FBGrab,
//       c.f., http://trac.ak-team.com/trac/browser/niluje/Configs/trunk/Kindle/Misc/FBGrab/fbgrab.c#L808
void
    identify_kobo(FBInkDeviceQuirks* deviceQuirks)
{
	// Get the model from Nickel's version tag file...
	FILE* fp = fopen("/mnt/onboard/.kobo/version", "r");
	if (!fp) {
		fprintf(stderr, "Couldn't find a Kobo version tag (not running on a Kobo?)!\n");
	} else {
		// NOTE: I'm not entirely sure this will always have a fixed length, so,
		// rely on getline()'s dynamic allocation to be safe...
		char*              line = NULL;
		size_t             len  = 0;
		ssize_t            nread;
		unsigned short int kobo_id = 0;
		while ((nread = getline(&line, &len, fp)) != -1) {
			// Thankfully, the device code is always located in the three
			// final characters, so that's easy enough to extract without
			// having to worry about the formatting...
			kobo_id = (unsigned short int) atoi(line + (nread - 3));
			// NOTE: Device code list pilfered from
			// https://github.com/geek1011/KoboStuff/blob/gh-pages/kobofirmware.js#L11
			switch (kobo_id) {
				case 310:
					// Touch A/B (trilogy)
					break;
				case 320:
					// Touch C (trilogy)
					break;
				case 340:
					// Mini (pixie)
					break;
				case 330:
					// Glo (kraken)
					break;
				case 371:
					// Glo HD (alyssum)
					break;
				case 372:
					// Touch 2.0 (pika)
					break;
				case 360:
					// Aura (phoenix)
					break;
				case 350:
					// Aura HD (dragon)
					break;
				case 370:
					// Aura H2O (dahlia)
					break;
				case 374:
					// Aura H2O² (snow)
					break;
				case 378:
					// Aura H2O² r2 (snow)
					deviceQuirks->isKoboMk7 = true;
					break;
				case 373:
					// Aura ONE (daylight)
					break;
				case 381:
					// Aura ONE LE (daylight)
					break;
				case 375:
					// Aura SE (star)
					break;
				case 379:
					// Aura SE r2 (star)
					deviceQuirks->isKoboMk7 = true;
					break;
				case 376:
					// Clara HD (nova)
					deviceQuirks->isKoboMk7 = true;
					break;
				case 0:
				default:
					fprintf(stderr, "Unidentified Kobo device code (%u)!\n", kobo_id);
					break;
			}
		}
		free(line);
		fclose(fp);
	}
}
