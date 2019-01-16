#ifndef __CUTEF8_H
#define __CUTEF8_H

// Because we're pretty much Linux-bound ;).
#ifndef _GNU_SOURCE
#	define _GNU_SOURCE
#endif

#include <alloca.h>
#include <assert.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <wchar.h>
#include <wctype.h>

/* is c the start of a utf8 sequence? */
#define isutf(c) (((c) &0xC0) != 0x80)

#define UEOF ((uint32_t) -1)

/* returns length of next utf-8 sequence */
size_t u8_seqlen(const char* s) __attribute__((pure));

/* returns the # of bytes needed to encode a certain character */
size_t u8_charlen(uint32_t ch) __attribute__((const));

/* computes the # of bytes needed to encode a WC string as UTF-8 */
size_t u8_codingsize(uint32_t* wcstr, size_t n) __attribute__((pure));

/* convert UTF-8 data to wide character */
size_t u8_toucs(uint32_t* dest, size_t sz, const char* src, size_t srcsz);

/* the opposite conversion */
size_t u8_toutf8(char* dest, size_t sz, const uint32_t* src, size_t srcsz);

/* single character to UTF-8, returns # bytes written */
size_t u8_wc_toutf8(char* dest, uint32_t ch);

/* character number to byte offset */
size_t u8_offset(const char* str, size_t charnum) __attribute__((pure));

/* byte offset to character number */
size_t u8_charnum(const char* s, size_t offset) __attribute__((pure));

/* number of columns occupied by a string */
size_t u8_strwidth(const char* s);

/* return next character, updating an index variable */
uint32_t u8_nextchar(const char* s, size_t* i);

/* count the number of characters in a UTF-8 string */
size_t u8_strlen(const char* s) __attribute__((pure));

/* move to next character */
void u8_inc(const char* s, size_t* i);

/* move to previous character */
void u8_dec(const char* s, size_t* i);

/* utility predicates used by the following */
int octal_digit(const char c) __attribute__((const));
int hex_digit(const char c) __attribute__((const));

/* assuming src points to the character after a backslash, read an
   escape sequence, storing the result in dest and returning the number of
   input characters processed */
size_t u8_read_escape_sequence(const char* src, size_t ssz, uint32_t* dest);

/* convert a string "src" containing escape sequences to UTF-8 */
size_t u8_unescape(char* buf, size_t sz, const char* src);

/* given a wide character, convert it to an ASCII escape sequence stored in
   buf, where buf is "sz" bytes. returns the number of characters output.
   sz must be at least 3. */
int u8_escape_wchar(char* buf, size_t sz, uint32_t ch);

/* convert UTF-8 "src" to escape sequences.

   sz is buf size in bytes. must be at least 12.

   if escape_quotes is true, quote characters will be escaped.

   if ascii is true, the output is 7-bit ASCII, no UTF-8 survives.

   starts at src[*pi], updates *pi to point to the first unprocessed
   byte of the input.

   end is one more than the last allowable value of *pi.

   returns number of bytes placed in buf, including a NUL terminator.
*/
size_t u8_escape(char* buf, size_t sz, const char* src, size_t* pi, size_t end, bool escape_quotes, bool ascii);

/* return a pointer to the first occurrence of ch in s, or NULL if not
   found. character index of found character returned in *charn. */
char* u8_strchr(const char* s, uint32_t ch, size_t* charn);

/* same as the above, but searches a buffer of a given size instead of
   a NUL-terminated string. */
char* u8_memchr(const char* s, uint32_t ch, size_t sz, size_t* charn);

char* u8_memrchr(const char* s, uint32_t ch, size_t sz);

bool u8_is_locale_utf8(const char* locale) __attribute__((pure));

/* printf where the format string and arguments may be in UTF-8.
   you can avoid this function and just use ordinary printf() if the current
   locale is UTF-8. */
size_t u8_vprintf(const char* fmt, va_list ap) __attribute__((format(printf, 1, 0)));
size_t u8_printf(const char* fmt, ...) __attribute__((format(printf, 1, 2)));

/* determine whether a sequence of bytes is valid UTF-8. length is in bytes */
typedef enum
{
	CUTEF8_IS_INVALID = 0,    // neither valid ASCII nor UTF-8
	CUTEF8_IS_ASCII   = 1,    // valid ASCII
	CUTEF8_IS_UTF8    = 2     // valid UTF-8
} CUTEF8_ISVALID_T;
CUTEF8_ISVALID_T u8_isvalid(const char* str, size_t length) __attribute__((pure));

/* reverse a UTF-8 string. len is length in bytes. dest and src must both
   be allocated to at least len+1 bytes. returns 1 for error, 0 otherwise */
int u8_reverse(char* dest, const char* src, size_t len);

#endif
