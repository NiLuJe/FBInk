#ifndef __CUTEF8_DFA_H
#define __CUTEF8_DFA_H

// Because we're pretty much Linux-bound ;).
#ifndef _GNU_SOURCE
#	define _GNU_SOURCE
#endif

#include <stdio.h>
#include <stdint.h>

#define UTF8_ACCEPT 0
#define UTF8_REJECT 12

size_t u8_dfa_strlen(const char* s);
int u8_dfa_isvalid(const char* str);

#endif
