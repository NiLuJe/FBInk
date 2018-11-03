#ifndef __LIBU8_H
#define __LIBU8_H

#include <stdarg.h>
#include <stdbool.h>
#include <stdint.h>

/* is c the start of a utf8 sequence? */
#define isutf(c) (((c) &0xC0) != 0x80)

/* returns length of next utf-8 sequence */
unsigned int u8_seqlen(const char* s) __attribute__((pure));

/* convert UTF-8 data to wide character */
unsigned int u8_toucs(uint32_t* dest, unsigned int sz, const char* src, int srcsz);

/* the opposite conversion */
int u8_toutf8(char* dest, unsigned int sz, const uint32_t* src, int srcsz);

/* single character to UTF-8 */
unsigned int u8_wc_toutf8(char* dest, uint32_t ch);

/* character number to byte offset */
unsigned int u8_offset(const char* str, unsigned int charnum) __attribute__((pure));

/* byte offset to character number */
unsigned int u8_charnum(const char* s, unsigned int offset) __attribute__((pure));

/* count the number of characters in a UTF-8 string */
unsigned int u8_strlen(const char* s);

/* return next character, updating an index variable */
uint32_t u8_nextchar(const char* s, unsigned int* i);

/* move to next character */
void u8_inc(const char* s, unsigned int* i);

/* move to previous character */
void u8_dec(const char* s, unsigned int* i);

/* utility predicates used by the above */
unsigned int octal_digit(const char c) __attribute__((const));
unsigned int hex_digit(const char c) __attribute__((const));

/* assuming src points to the character after a backslash, read an
   escape sequence, storing the result in dest and returning the number of
   input characters processed */
unsigned int u8_read_escape_sequence(const char* src, uint32_t* dest);

/* convert a string "src" containing escape sequences to UTF-8 */
unsigned int u8_unescape(char* buf, unsigned int sz, const char* src);

/* given a wide character, convert it to an ASCII escape sequence stored in
   buf, where buf is "sz" bytes. returns the number of characters output. */
int u8_escape_wchar(char* buf, unsigned int sz, uint32_t ch);

/* convert UTF-8 "src" to ASCII with escape sequences.
   if escape_quotes is true, quote characters will be preceded by
   backslashes as well. */
unsigned int u8_escape(char* buf, unsigned int sz, const char* src, bool escape_quotes);

/* return a pointer to the first occurrence of ch in s, or NULL if not
   found. character index of found character returned in *charn. */
char* u8_strchr(char* s, uint32_t ch, unsigned int* charn);

/* same as the above, but searches a buffer of a given size instead of
   a NUL-terminated string. */
char* u8_memchr(char* s, uint32_t ch, size_t sz, unsigned int* charn);

bool u8_is_locale_utf8(char* locale) __attribute__((pure));

/* printf where the format string and arguments may be in UTF-8.
   you can avoid this function and just use ordinary printf() if the current
   locale is UTF-8. */
int u8_vprintf(const char* fmt, va_list ap) __attribute__((format(printf, 1, 0)));
int u8_printf(const char* fmt, ...) __attribute__((format(printf, 1, 2)));

#endif
