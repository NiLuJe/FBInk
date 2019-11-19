/*
  Basic UTF-8 manipulation routines
  by Jeff Bezanson
  placed in the public domain Fall 2005

  This code is designed to provide the utilities you need to manipulate
  UTF-8 as an internal string encoding. These functions do not perform the
  error checking normally needed when handling UTF-8 data, so if you happen
  to be from the Unicode Consortium you will want to flay me alive.
  I do this because error checking can be performed at the boundaries (I/O),
  with these routines reserved for higher performance on data known to be
  valid.
  A UTF-8 validation routine is included.
*/

#include "utf8.h"

static const uint32_t offsetsFromUTF8[6] = { 0x00000000UL, 0x00003080UL, 0x000E2080UL,
					     0x03C82080UL, 0xFA082080UL, 0x82082080UL };

static const unsigned char trailingBytesForUTF8[256] = {
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
	1, 1, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 3, 3, 3, 3, 3, 3, 3, 3, 4, 4, 4, 4, 5, 5, 5, 5
};

/* returns length of next utf-8 sequence */
size_t
    u8_seqlen(const char* restrict s)
{
	return trailingBytesForUTF8[(unsigned int) (unsigned char) s[0]] + 1U;
}

/* returns the # of bytes needed to encode a certain character
   0 means the character cannot (or should not) be encoded. */
size_t
    u8_charlen(uint32_t ch)
{
	if (ch < 0x80) {
		return 1U;
	} else if (ch < 0x800) {
		return 2U;
	} else if (ch < 0x10000) {
		return 3U;
	} else if (ch < 0x110000) {
		return 4U;
	}
	return 0U;
}

size_t
    u8_codingsize(uint32_t* wcstr, size_t n)
{
	size_t c = 0;

	for (size_t i = 0; i < n; i++) {
		size_t cl = u8_charlen(wcstr[i]);
		if (cl == 0) {
			// invalid: encoded as replacement char
			cl = 3;
		}
		c += cl;
	}
	return c;
}

/* conversions without error checking
   only works for valid UTF-8, i.e. no 5- or 6-byte sequences
   srcsz = source size in bytes
   sz = dest size in # of wide characters
   returns # characters converted
   if sz == srcsz+1 (i.e. 4*srcsz+4 bytes), there will always be enough space.
*/
size_t
    u8_toucs(uint32_t* dest, size_t sz, const char* src, size_t srcsz)
{
	uint32_t    ch;
	const char* src_end = src + srcsz;
	size_t      nb;
	size_t      i = 0;

	if (sz == 0 || srcsz == 0) {
		return 0;
	}

	while (i < sz) {
		if (!isutf(*src)) {    // invalid sequence
			dest[i++] = 0xFFFD;
			src++;
			if (src >= src_end) {
				break;
			}
			continue;
		}
		nb = trailingBytesForUTF8[(unsigned char) *src];
		if (src + nb >= src_end) {
			break;
		}
		ch = 0;
		switch (nb) {
			// fall through
			case 5:
				ch += (unsigned char) *src++;
				ch <<= 6U;
			// fall through
			case 4:
				ch += (unsigned char) *src++;
				ch <<= 6U;
			// fall through
			case 3:
				ch += (unsigned char) *src++;
				ch <<= 6U;
			// fall through
			case 2:
				ch += (unsigned char) *src++;
				ch <<= 6U;
			// fall through
			case 1:
				ch += (unsigned char) *src++;
				ch <<= 6U;
			// fall through
			case 0:
				ch += (unsigned char) *src++;
		}
		ch -= offsetsFromUTF8[nb];
		dest[i++] = ch;
	}
	return i;
}

/* srcsz = number of source characters
   sz = size of dest buffer in bytes
   returns # bytes stored in dest
   the destination string will never be bigger than the source string.
*/
size_t
    u8_toutf8(char* dest, size_t sz, const uint32_t* src, size_t srcsz)
{
	uint32_t ch;
	size_t   i        = 0;
	char*    dest0    = dest;
	char*    dest_end = dest + sz;

	while (i < srcsz) {
		ch = src[i];
		if (ch < 0x80) {
			if (dest >= dest_end) {
				break;
			}
			*dest++ = (char) ch;
		} else if (ch < 0x800) {
			if (dest >= dest_end - 1) {
				break;
			}
			*dest++ = (char) ((ch >> 6) | 0xC0);
			*dest++ = (char) ((ch & 0x3F) | 0x80);
		} else if (ch - 0xd800u < 0x800) {
			if (dest >= dest_end - 2) {
				break;
			}
			// invalid: use replacement char \ufffd
			*dest++ = (char) 0xef;
			*dest++ = (char) 0xbf;
			*dest++ = (char) 0xbd;
		} else if (ch < 0x10000) {
			if (dest >= dest_end - 2) {
				break;
			}
			*dest++ = (char) ((ch >> 12) | 0xE0);
			*dest++ = (char) (((ch >> 6) & 0x3F) | 0x80);
			*dest++ = (char) ((ch & 0x3F) | 0x80);
		} else if (ch < 0x110000) {
			if (dest >= dest_end - 3) {
				break;
			}
			*dest++ = (char) ((ch >> 18) | 0xF0);
			*dest++ = (char) (((ch >> 12) & 0x3F) | 0x80);
			*dest++ = (char) (((ch >> 6) & 0x3F) | 0x80);
			*dest++ = (char) ((ch & 0x3F) | 0x80);
		} else {
			if (dest >= dest_end - 2) {
				break;
			}
			// invalid: use replacement char \ufffd
			*dest++ = (char) 0xef;
			*dest++ = (char) 0xbf;
			*dest++ = (char) 0xbd;
		}
		i++;
	}
	return (size_t)(dest - dest0);
}

// NOTE: See also https://stackoverflow.com/a/38492214
//                https://stackoverflow.com/a/4609989
//                https://gist.github.com/MightyPork/52eda3e5677b4b03524e40c9f0ab1da5
//                https://gist.github.com/tylerneylon/9773800
//              & https://rosettacode.org/wiki/UTF-8_encode_and_decode#C
size_t
    u8_wc_toutf8(char* dest, uint32_t ch)
{
	if (ch < 0x80) {
		dest[0] = (char) ch;
		return 1U;
	}
	if (ch < 0x800) {
		dest[0] = (char) ((ch >> 6) | 0xC0);
		dest[1] = (char) ((ch & 0x3F) | 0x80);
		return 2U;
	}
	if (ch - 0xd800u < 0x800) {
		goto fffd;
	}
	if (ch < 0x10000) {
		dest[0] = (char) ((ch >> 12) | 0xE0);
		dest[1] = (char) (((ch >> 6) & 0x3F) | 0x80);
		dest[2] = (char) ((ch & 0x3F) | 0x80);
		return 3U;
	}
	if (ch < 0x110000) {
		dest[0] = (char) ((ch >> 18) | 0xF0);
		dest[1] = (char) (((ch >> 12) & 0x3F) | 0x80);
		dest[2] = (char) (((ch >> 6) & 0x3F) | 0x80);
		dest[3] = (char) ((ch & 0x3F) | 0x80);
		return 4U;
	}
fffd:
	dest[0] = (char) 0xef;
	dest[1] = (char) 0xbf;
	dest[2] = (char) 0xbd;
	return 3U;
}

// Variant that uses a static internal buffer, and returns a pointer to it.
// Mainly used for debugging/verbose logging.
char*
    u8_cp_to_utf8(uint32_t cp)
{
	static char utf8[4] = { 0 };
	char*       b       = utf8;

	if (cp < 0x80) {
		*b++ = (char) cp;
	} else if (cp < 0x800) {
		*b++ = (char) (192 + cp / 64);
		*b++ = (char) (128 + cp % 64);
	} else if (cp - 0xD800u < 0x800) {
		// \uFFFD
		*b++ = (char) 0xEF;
		*b++ = (char) 0xBF;
		*b++ = (char) 0xBD;
	} else if (cp < 0x10000) {
		*b++ = (char) (224 + cp / 4096);
		*b++ = (char) (128 + cp / 64 % 64);
		*b++ = (char) (128 + cp % 64);
	} else if (cp < 0x110000) {
		*b++ = (char) (240 + cp / 262144);
		*b++ = (char) (128 + cp / 4096 % 64);
		*b++ = (char) (128 + cp / 64 % 64);
		*b++ = (char) (128 + cp % 64);
	} else {
		// \uFFFD
		*b++ = (char) 0xEF;
		*b++ = (char) 0xBF;
		*b++ = (char) 0xBD;
	}

	// NUL terminate and return pointer to start
	*b++ = '\0';
	return utf8;
}

/* charnum => byte offset */
size_t
    u8_offset(const char* s, size_t charnum)
{
	size_t i = 0;

	while (charnum > 0) {
		if (s[i++] & 0x80) {
			(void) (isutf(s[++i]) || isutf(s[++i]) || ++i);
		}
		charnum--;
	}
	return i;
}

/* byte offset => charnum */
size_t
    u8_charnum(const char* s, size_t offset)
{
	size_t charnum = 0;

	if (offset) {
		do {
			// Simply not count continuation bytes
			// Since we are not doing validation anyway, we can just
			// assume this is a valid UTF-8 string
			charnum += isutf(*(const unsigned char*) s++);
		} while (--offset);
	}
	return charnum;
}

size_t
    u8_strwidth(const char* s)
{
	uint32_t    ch;
	size_t      nb;
	size_t      tot = 0;
	int         w;
	signed char sc;

	while ((sc = (signed char) *s) != 0) {
		if (sc >= 0) {
			s++;
			if (sc) {
				tot++;
			}
		} else {
			if (!isutf(sc)) {
				tot++;
				s++;
				continue;
			}
			nb = trailingBytesForUTF8[(unsigned char) sc];
			ch = 0;
			switch (nb) {
				// fall through
				case 5:
					ch += (unsigned char) *s++;
					ch <<= 6U;
				// fall through
				case 4:
					ch += (unsigned char) *s++;
					ch <<= 6U;
				// fall through
				case 3:
					ch += (unsigned char) *s++;
					ch <<= 6U;
				// fall through
				case 2:
					ch += (unsigned char) *s++;
					ch <<= 6U;
				// fall through
				case 1:
					ch += (unsigned char) *s++;
					ch <<= 6U;
				// fall through
				case 0:
					ch += (unsigned char) *s++;
			}
			ch -= offsetsFromUTF8[nb];
			w = wcwidth(
			    (wchar_t) ch);    // might return -1, locale-dependent (Julia now uses utf8proc_charwidth)
			if (w > 0) {
				tot += (size_t) w;
			}
		}
	}
	return tot;
}

/* reads the next utf-8 sequence out of a string, updating an index */
// NOTE: See also https://github.com/JuliaStrings/LegacyStrings.jl/blob/master/src/utf8.jl
//       & https://github.com/JuliaLang/julia/blob/master/base/strings/string.jl
uint32_t
    u8_nextchar(const char* s, size_t* i)
{
	uint32_t ch = 0;
	size_t   sz;

	sz = u8_seqlen(&s[*i]);
	for (size_t j = sz; j > 0; j--) {
		ch <<= 6U;
		ch += (unsigned char) s[(*i)++];
	}
	ch -= offsetsFromUTF8[sz - 1];

	return ch;
}

// NOTE: Actually inspired from https://github.com/JuliaStrings/LegacyStrings.jl/blob/master/src/utf8.jl,
//       after https://github.com/JuliaLang/julia/pull/11107
size_t
    u8_strlen(const char* s)
{
	size_t count = 0;

	while (*s) {
		count += isutf(*(const unsigned char*) s++);
	}
	return count;
}

void
    u8_inc(const char* restrict s, size_t* restrict i)
{
	(void) (isutf(s[++(*i)]) || isutf(s[++(*i)]) || isutf(s[++(*i)]) || ++(*i));
}

void
    u8_dec(const char* restrict s, size_t* restrict i)
{
	(void) (isutf(s[--(*i)]) || isutf(s[--(*i)]) || isutf(s[--(*i)]) || --(*i));
}

int
    octal_digit(char c)
{
	return (c >= '0' && c <= '7');
}

int
    hex_digit(char c)
{
	return ((c >= '0' && c <= '9') || (c >= 'A' && c <= 'F') || (c >= 'a' && c <= 'f'));
}

static char
    read_escape_control_char(char c)
{
	if (c == 'n') {
		return '\n';
	} else if (c == 't') {
		return '\t';
	} else if (c == 'r') {
		return '\r';
	} else if (c == 'e') {
		return '\x1B';
	} else if (c == 'b') {
		return '\b';
	} else if (c == 'f') {
		return '\f';
	} else if (c == 'v') {
		return '\v';
	} else if (c == 'a') {
		return '\a';
	}
	return c;
}

/* assumes that src points to the character after a backslash
   returns number of input characters processed, 0 if error */
size_t
    u8_read_escape_sequence(const char* str, size_t ssz, uint32_t* dest)
{
	assert(ssz > 0);
	uint32_t ch;
	char     digs[10];
	int      dno = 0;
	int      ndig;
	size_t   i  = 1;
	char     c0 = str[0];

	if (octal_digit(c0)) {
		i = 0;
		do {
			digs[dno++] = str[i++];
		} while (i < ssz && octal_digit(str[i]) && dno < 3);
		digs[dno] = '\0';
		ch        = (uint32_t) strtoul(digs, NULL, 8);
	} else if ((c0 == 'x' && (ndig = 2)) || (c0 == 'u' && (ndig = 4)) || (c0 == 'U' && (ndig = 8))) {
		while (i < ssz && hex_digit(str[i]) && dno < ndig) {
			digs[dno++] = str[i++];
		}
		if (dno == 0) {
			return 0;
		}
		digs[dno] = '\0';
		ch        = (uint32_t) strtoul(digs, NULL, 16);
	} else {
		ch = (uint32_t) read_escape_control_char(c0);
	}
	*dest = ch;

	return i;
}

/* convert a string with literal \uxxxx or \Uxxxxxxxx characters to UTF-8
   example: u8_unescape(mybuf, 256, "hello\\u220e")
   note the double backslash is needed if called on a C string literal */
size_t
    u8_unescape(char* buf, size_t sz, const char* src)
{
	size_t   c = 0;
	size_t   amt;
	uint32_t ch = 0;
	char     temp[4];

	while (*src && c < sz) {
		if (*src == '\\') {
			src++;
			amt = u8_read_escape_sequence(src, 1000, &ch);
		} else {
			ch  = (uint32_t) *src;
			amt = 1;
		}
		src += amt;
		amt = u8_wc_toutf8(temp, ch);
		if (amt > sz - c) {
			break;
		}
		memcpy(&buf[c], temp, amt);
		c += amt;
	}
	if (c < sz) {
		buf[c] = '\0';
	}
	return c;
}

inline static int
    buf_put2c(char* buf, const char* src)
{
	buf[0] = src[0];
	buf[1] = src[1];
	buf[2] = '\0';
	return 2;
}

int
    u8_escape_wchar(char* buf, size_t sz, uint32_t ch)
{
	assert(sz > 2);
	if (ch == L'\n') {
		return buf_put2c(buf, "\\n");
	} else if (ch == L'\t') {
		return buf_put2c(buf, "\\t");
	} else if (ch == L'\r') {
		return buf_put2c(buf, "\\r");
	} else if (ch == L'\x1B') {
		return buf_put2c(buf, "\\e");
	} else if (ch == L'\b') {
		return buf_put2c(buf, "\\b");
	} else if (ch == L'\f') {
		return buf_put2c(buf, "\\f");
	} else if (ch == L'\v') {
		return buf_put2c(buf, "\\v");
	} else if (ch == L'\a') {
		return buf_put2c(buf, "\\a");
	} else if (ch == L'\\') {
		return buf_put2c(buf, "\\\\");
	} else if (ch < 32 || ch == 0x7f) {
		return snprintf(buf, sz, "\\x%.2hhx", (unsigned char) ch);
	} else if (ch > 0xFFFF) {
		return snprintf(buf, sz, "\\U%.8x", (uint32_t) ch);
	} else if (ch >= 0x80) {
		return snprintf(buf, sz, "\\u%.4hx", (unsigned short) ch);
	}

	buf[0] = (char) ch;
	buf[1] = '\0';
	return 1;
}

size_t
    u8_escape(char* buf, size_t sz, const char* src, size_t* pi, size_t end, bool escape_quotes, bool ascii)
{
	assert(sz > 11);
	size_t   i = *pi;
	size_t   i0;
	uint32_t ch;
	char*    start = buf;
	char*    blim  = start + sz - 11;

	while (i < end && buf < blim) {
		// sz-11: leaves room for longest escape sequence
		if (escape_quotes && src[i] == '"') {
			buf += buf_put2c(buf, "\\\"");
			i++;
		} else if (src[i] == '\\') {
			buf += buf_put2c(buf, "\\\\");
			i++;
		} else {
			i0 = i;
			ch = u8_nextchar(src, &i);
			if (ascii || !iswprint((wint_t) ch)) {
				buf += u8_escape_wchar(buf, sz - (size_t)(buf - start), ch);
			} else {
				i = i0;
				do {
					*buf++ = src[i++];
				} while (!isutf(src[i]));
			}
		}
	}
	*buf++ = '\0';
	*pi    = i;
	return (size_t)(buf - start);
}

char*
    u8_strchr(const char* s, uint32_t ch, size_t* charn)
{
	size_t   i     = 0;
	size_t   lasti = 0;
	uint32_t c;

	*charn = 0;
	while (s[i]) {
		c = u8_nextchar(s, &i);
		if (c == ch) {
			/* it's const for us, but not necessarily the caller */
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wcast-qual"
			return (char*) &s[lasti];
#pragma GCC diagnostic pop
		}
		lasti = i;
		(*charn)++;
	}
	return NULL;
}

char*
    u8_memchr(const char* s, uint32_t ch, size_t sz, size_t* charn)
{
	size_t   i     = 0;
	size_t   lasti = 0;
	uint32_t c;
	uint32_t csz;

	*charn = 0;
	while (i < sz) {
		c = csz = 0;
		do {
			c <<= 6U;
			c += (unsigned char) s[i++];
			csz++;
		} while (i < sz && !isutf(s[i]));
		c -= offsetsFromUTF8[csz - 1];

		if (c == ch) {
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wcast-qual"
			return (char*) &s[lasti];
#pragma GCC diagnostic pop
		}
		lasti = i;
		(*charn)++;
	}
	return NULL;
}

char*
    u8_memrchr(const char* s, uint32_t ch, size_t sz)
{
	size_t   i     = sz - 1;
	size_t   tempi = 0;
	uint32_t c;

	if (sz == 0) {
		return NULL;
	}

	while (i && !isutf(s[i])) {
		i--;
	}

	while (1) {
		tempi = i;
		c     = u8_nextchar(s, &tempi);
		if (c == ch) {
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wcast-qual"
			return (char*) &s[i];
#pragma GCC diagnostic pop
		}
		if (i == 0) {
			break;
		}
		tempi = i;
		u8_dec(s, &i);
		if (i > tempi) {
			break;
		}
	}
	return NULL;
}

bool
    u8_is_locale_utf8(const char* locale)
{
	if (locale == NULL) {
		return false;
	}

	/* this code based on libutf8 */
	const char* cp = locale;

	for (; *cp != '\0' && *cp != '@' && *cp != '+' && *cp != ',' && *cp != ';'; cp++) {
		if (*cp == '.') {
			const char* encoding = ++cp;
			for (; *cp != '\0' && *cp != '@' && *cp != '+' && *cp != ',' && *cp != ';'; cp++) {
				;
			}
			if ((cp - encoding == 5 && !strncmp(encoding, "UTF-8", 5)) ||
			    (cp - encoding == 4 && !strncmp(encoding, "utf8", 4))) {
				return true; /* it's UTF-8 */
			}
			break;
		}
	}
	return false;
}

size_t
    u8_vprintf(const char* fmt, va_list ap)
{
	int       ret;
	size_t    cnt;
	size_t    sz = 0;
	size_t    nc;
	bool      needfree = false;
	char*     buf;
	uint32_t* wcs;

	sz  = 512;
	buf = (char*) alloca(sz);
	ret = vsnprintf(buf, sz, fmt, ap);
	if (ret < 0) {
		return 0;
	} else {
		cnt = (size_t) ret;
	}
	if (cnt >= sz) {
		buf      = (char*) malloc(cnt + 1U);
		needfree = true;
		vsnprintf(buf, cnt + 1U, fmt, ap);
	}
	wcs     = (uint32_t*) alloca((cnt + 1U) * sizeof(uint32_t));
	nc      = u8_toucs(wcs, cnt + 1U, buf, cnt);
	wcs[nc] = 0;
	printf("%ls", (wchar_t*) wcs);
	if (needfree) {
		free(buf);
	}
	return nc;
}

size_t
    u8_printf(const char* fmt, ...)
{
	size_t  cnt;
	va_list args;

	va_start(args, fmt);

	cnt = u8_vprintf(fmt, args);

	va_end(args);
	return cnt;
}

/* Rewritten completely, original code not based on anything else
   length is in bytes, since without knowing whether the string is valid
   it's hard to know how many characters there are! */
// NOTE: It's very close to what utf8proc does (which makes sense, given the Julia background),
//       as well as what gnulib/libunistring does, too (https://git.savannah.gnu.org/cgit/gnulib.git/tree/lib/unistr/u8-check.c)
//       In fact, trying to match the very few discrepancies between this and gnulib
//       (c.f., https://gist.github.com/NiLuJe/f3056bfaeabd1569a52a1eb9fab0e40d)
//       yielded the very same results with a quick check against https://www.cl.cam.ac.uk/~mgk25/ucs/examples/UTF-8-test.txt ;).
// NOTE: FWIW, we also get the same results when using http://bjoern.hoehrmann.de/utf-8/decoder/dfa
//       (c.f., u8_isvalid2 @ cutef8/dfa.c)
// TL;DR: Which means I'm keeping this logic untouched, given the high probabilty I'd have of subtly messing something up ;).
CUTEF8_ISVALID_T
u8_isvalid(const char* str, size_t len)
{
	const unsigned char* pnt;     // Current pointer in string
	const unsigned char* pend;    // End of string
	unsigned char        byt;     // Current byte

	// Empty strings can be considered valid ASCII
	if (!len) {
		return CUTEF8_IS_ASCII;
	}
	pnt  = (const unsigned char*) str;
	pend = (const unsigned char*) str + len;
	// First scan for non-ASCII characters as fast as possible
	do {
		if (*pnt++ & 0x80)
			goto chkutf8;
	} while (pnt < pend);
	return CUTEF8_IS_ASCII;

	// Check validity of UTF-8 sequences
chkutf8:
	if (pnt == pend) {
		return CUTEF8_IS_INVALID;    // Last byte can't be > 127
	}
	byt = pnt[-1];
	// Must be between 0xc2 and 0xf4 inclusive to be valid
	if (((uint32_t) byt - 0xc2) > (0xf4 - 0xc2)) {
		return CUTEF8_IS_INVALID;
	}
	if (byt < 0xe0) {    // 2-byte sequence
		// Must have valid continuation character
		if (isutf(*pnt++)) {
			return CUTEF8_IS_INVALID;
		}
	} else if (byt < 0xf0) {    // 3-byte sequence
		if ((pnt + 1U >= pend) || isutf(*pnt) || isutf(pnt[1])) {
			return CUTEF8_IS_INVALID;
		}
		// Check for surrogate chars
		if (byt == 0xed && *pnt > 0x9f) {
			return CUTEF8_IS_INVALID;
		}
		// Check for overlong encoding
		if (byt == 0xe0 && *pnt < 0xa0) {
			return CUTEF8_IS_INVALID;
		}
		pnt += 2U;
	} else {    // 4-byte sequence
		// Must have 3 valid continuation characters
		if ((pnt + 2U >= pend) || isutf(*pnt) || isutf(pnt[1]) || isutf(pnt[2])) {
			return CUTEF8_IS_INVALID;
		}
		// Make sure in correct range (0x10000 - 0x10ffff)
		if (byt == 0xf0) {
			if (*pnt < 0x90) {
				return CUTEF8_IS_INVALID;
			}
		} else if (byt == 0xf4) {
			if (*pnt > 0x8f) {
				return CUTEF8_IS_INVALID;
			}
		}
		pnt += 3U;
	}
	// Find next non-ASCII characters as fast as possible
	while (pnt < pend) {
		if (*pnt++ & 0x80) {
			goto chkutf8;
		}
	}
	return CUTEF8_IS_UTF8;    // Valid UTF-8
}

int
    u8_reverse(char* dest, const char* src, size_t len)
{
	size_t        si = 0;
	size_t        di = len;
	unsigned char c;

	dest[di] = '\0';
	while (si < len) {
		c = (unsigned char) src[si];
		if ((~c) & 0x80) {
			di--;
			dest[di] = (char) c;
			si++;
		} else {
			switch (c >> 4) {
				case 0xC:
				case 0xD:
					di -= 2U;
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wcast-align"
					*((int16_t*) &dest[di]) = *((const int16_t*) &src[si]);
#pragma GCC diagnostic pop
					si += 2U;
					break;
				case 0xE:
					di -= 3U;
					dest[di] = src[si];
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wcast-align"
					*((int16_t*) &dest[di + 1]) = *((const int16_t*) &src[si + 1]);
#pragma GCC diagnostic pop
					si += 3U;
					break;
				case 0xF:
					di -= 4U;
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wcast-align"
					*((int32_t*) &dest[di]) = *((const int32_t*) &src[si]);
#pragma GCC diagnostic pop
					si += 4U;
					break;
				default:
					return 1;
			}
		}
	}
	return 0;
}
