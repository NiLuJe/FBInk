// c.f., ../font8x8/render.c
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include "../fonts/block.h"

void
    usage(char* exec)
{
	printf("Usage: %s <char_code>\n", exec);
	printf("       <char_code> Decimal character code between 20 and 255\n");
}

void
    render(const uint32_t* bitmap)
{
	bool set;
	for (uint8_t x = 0U; x < 32; x++) {
		for (uint8_t y = 0U; y < 32; y++) {
			// NOTE: For the non-mirrored Unifont hex format, access rows in reverse order
			//for (int8_t y=32-1; y >= 0; y--) {
			set = bitmap[x] & 1U << y;
			printf("%c", set ? '#' : '.');
		}
		printf("\n");
	}
}

int
    main(int argc, char** argv)
{
	short int ord;
	if (argc != 2) {
		usage(argv[0]);
		return 1;
	}
	ord = (short int) atoi(argv[1]);
	// Try reading it as a char?
	if (ord == 0) {
		ord = argv[1][0];
	}
	if (ord > 255 || ord < 20) {
		fprintf(stderr, "%hd is OOR\n", ord);
		usage(argv[0]);
		return 2;
	}
	// NOTE: First char is 0x20, convert from ASCII code to array index
	const uint32_t* bitmap = block_block1[ord - 0x20];

	render(bitmap);
	return 0;
}
