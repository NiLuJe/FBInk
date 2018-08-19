// c.f., ../font8x8/render.c
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>

#include "../fonts/block.h"

void usage(char *exec) {
    printf("Usage: %s <char_code>\n", exec);
    printf("       <char_code> Decimal character code between 0 and 255\n");
}

void render(const uint32_t *bitmap) {
    bool set;
    for (uint8_t x=0U; x < 32; x++) {
        for (uint8_t y=0U; y < 32; y++) {
            set = bitmap[x] & 1U << y;
            printf("%c", set ? '#' : '.');
        }
        printf("\n");
    }
}

int main(int argc, char **argv) {
    short int ord;
    if (argc != 2) {
        usage(argv[0]);
        return 1;
    }
    ord = (short int) atoi(argv[1]);
    if (ord > 255 || ord < 0) {
        usage(argv[0]);
        return 2;
    }
    const uint32_t *bitmap = block_block1[ord];

    render(bitmap);
    return 0;
}
