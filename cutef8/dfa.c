// Copyright (c) 2008-2010 Bjoern Hoehrmann <bjoern@hoehrmann.de>
// See http://bjoern.hoehrmann.de/utf-8/decoder/dfa/ for details.
// NOTE: See also https://github.com/hoehrmann/utf-8-misc
//       (FWIW, a quick test of those two variants shows similar performance for my use-cases).

#include "dfa.h"

static const uint8_t utf8d[] = {
	0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
	0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
	0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
	0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
	0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,
	1,  1,  1,  1,  9,  9,  9,  9,  9,  9,  9,  9,  9,  9,  9,  9,  9,  9,  9,  9,  7,  7,  7,  7,  7,  7,  7,  7,
	7,  7,  7,  7,  7,  7,  7,  7,  7,  7,  7,  7,  7,  7,  7,  7,  7,  7,  7,  7,  7,  7,  7,  7,  8,  8,  2,  2,
	2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,
	10, 3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  4,  3,  3,  11, 6,  6,  6,  5,  8,  8,  8,  8,  8,  8,  8,
	8,  8,  8,  8,  0,  12, 24, 36, 60, 96, 84, 12, 12, 12, 48, 72, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12,
	12, 0,  12, 12, 12, 12, 12, 0,  12, 0,  12, 12, 12, 24, 12, 12, 12, 12, 12, 24, 12, 24, 12, 12, 12, 12, 12, 12,
	12, 12, 12, 24, 12, 12, 12, 12, 12, 24, 12, 12, 12, 12, 12, 12, 12, 24, 12, 12, 12, 12, 12, 12, 12, 12, 12, 36,
	12, 36, 12, 12, 12, 36, 12, 12, 12, 12, 12, 36, 12, 36, 12, 12, 12, 36, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12,
};

// NOTE: See https://lists.w3.org/Archives/Public/www-archive/2013Apr/0041.html
//       for an explanation why the original code used uint32_t for byte (and in turn type & state) ;).
//       FWIW, I'm not seeing a noticeable shift in performance on ARM with my GCC 7 TCs.
//       And more importantly, the original version was crashy on x86_64 (GCC 8.2) ;).
inline static uint8_t
    decode(uint8_t* restrict state, uint32_t* restrict codep, uint8_t byte)
{
	uint8_t type = utf8d[byte];

	// NOTE: The > UTF8_REJECT variant is a tiny bit slower, but we hopefully shouldn't need the extra safety it'd provide ;)
	*codep = (*state != UTF8_ACCEPT) ? (byte & 0x3fu) | (*codep << 6) : (0xffu >> type) & (byte);
	*state = utf8d[256 + *state + type];
	return *state;
}

// Same as decode, but without actually decoding the codepoints, because that's unneeded for validation/accounting
inline static uint8_t
    check(uint8_t* restrict state, uint8_t byte)
{
	uint8_t type = utf8d[byte];

	*state = utf8d[256 + *state + type];
	return *state;
}

inline static int
    count_codepoints(const char* restrict str, size_t* restrict count)
{
	uint8_t state = 0;

	for (*count = 0; *str; ++str) {
		if (!check(&state, *(const uint8_t*) str)) {
			*count += 1;
		}
	}
	return state != UTF8_ACCEPT;
}

// And now what we actually expose...
size_t
    u8_strlen2(const char* restrict str)
{
	size_t count = 0;

	if (count_codepoints(str, &count)) {
		// NOTE: Malformed UTF-8 sequence! Return 0, as we treat this as fatal.
		return 0;
	} else {
		return count;
	}
}

bool
    u8_isvalid2(const char* restrict str)
{
	uint8_t state = 0;

	while (*str) {
		check(&state, *(const uint8_t*) str++);
	}

	return state == UTF8_ACCEPT;
}

// Take a stab at reimplementing u8_nextchar with the dfa decoder...
// NOTE: For shit'n giggles, libunibreak also has its own next_char implementation... (ub_get_next_char_utf8 @ unibreakdef.c)
uint32_t
    u8_nextchar2(const char* restrict s, size_t* restrict i)
{
	uint32_t ch    = 0;
	uint8_t  state = 0;

	// NOTE: This *should* be safe enough, since we validate our input...
	//       Otherwise, might want to cap at 4 iterations/bytes? Or @ u8_seqlen iters, like ut8_nextchar?
	//       With a hard return of 0 if state still != UTF8_ACCEPT once the cap is hit...
	while (s[*i]) {
		if (!decode(&state, &ch, (const uint8_t) s[(*i)++])) {
			break;
		}
	}

	return ch;
}
