#ifndef __CUTEF8_DFA_H
#define __CUTEF8_DFA_H

// Because we're pretty much Linux-bound ;).
#ifndef _GNU_SOURCE
#	define _GNU_SOURCE
#endif

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

#define UTF8_ACCEPT 0
#define UTF8_REJECT 12

// Returns the # of codepoints in UTF-8 encoded string s (or 0 if s is malformed)
size_t u8_strlen2(const char* restrict s) __attribute__((pure));
// Returns true if UTF-8 encoded string s is not malformed
bool   u8_isvalid2(const char* restrict s) __attribute__((pure));

// Like u8_nextchar, but using the dfa decoder
uint32_t u8_nextchar2(const char* restrict s, size_t* restrict i);

#endif
