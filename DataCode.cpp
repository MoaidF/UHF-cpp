#include "DataCode.h"

#pragma pack (push, 8)
#include <stdio.h>
#include <stdlib.h>
#pragma pack(pop)

#define F 0   /* character never appears in text */
#define T 1   /* character appears in plain ASCII text */
#define I 2   /* character appears in ISO-8859 text */
#define X 3   /* character appears in non-ISO extended ASCII (Mac, IBM PC) */

/////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////

static char text_chars[256] = {
	/*                  BEL BS HT LF    FF CR    */
	F, F, F, F, F, F, F, T, T, T, T, F, T, T, F, F,  /* 0x0X */
	/*                              ESC          */
	F, F, F, F, F, F, F, F, F, F, F, T, F, F, F, F,  /* 0x1X */
	T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T,  /* 0x2X */
	T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T,  /* 0x3X */
	T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T,  /* 0x4X */
	T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T,  /* 0x5X */
	T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T,  /* 0x6X */
	T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, F,  /* 0x7X */
	/*            NEL                            */
	X, X, X, X, X, T, X, X, X, X, X, X, X, X, X, X,  /* 0x8X */
	X, X, X, X, X, X, X, X, X, X, X, X, X, X, X, X,  /* 0x9X */
	I, I, I, I, I, I, I, I, I, I, I, I, I, I, I, I,  /* 0xaX */
	I, I, I, I, I, I, I, I, I, I, I, I, I, I, I, I,  /* 0xbX */
	I, I, I, I, I, I, I, I, I, I, I, I, I, I, I, I,  /* 0xcX */
	I, I, I, I, I, I, I, I, I, I, I, I, I, I, I, I,  /* 0xdX */
	I, I, I, I, I, I, I, I, I, I, I, I, I, I, I, I,  /* 0xeX */
	I, I, I, I, I, I, I, I, I, I, I, I, I, I, I, I   /* 0xfX */
};

/////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////

static int looks_ascii(const unsigned char *buf, size_t nbytes)
{
	size_t i;

	for (i = 0; i < nbytes; i++) {
		int t = text_chars[buf[i]];

		if (t != T)
			return 0;
	}

	return 1;
}

static int file_looks_utf8(const unsigned char *buf, size_t nbytes)
{
	size_t i;
	int n;
	wchar_t c;
	int gotone = 0, ctrl = 0;

	for (i = 0; i < nbytes; i++) {
		if ((buf[i] & 0x80) == 0) {	   /* 0xxxxxxx is plain ASCII */
			/*
			 * Even if the whole file is valid UTF-8 sequences,
			 * still reject it if it uses weird control characters.
			 */

			if (text_chars[buf[i]] != T)
				ctrl = 1;

		} else if ((buf[i] & 0x40) == 0) { /* 10xxxxxx never 1st byte */
			return -1;
		} else {			   /* 11xxxxxx begins UTF-8 */
			int following;

			if ((buf[i] & 0x20) == 0) {		/* 110xxxxx */
				c = buf[i] & 0x1f;
				following = 1;
			} else if ((buf[i] & 0x10) == 0) {	/* 1110xxxx */
				c = buf[i] & 0x0f;
				following = 2;
			} else if ((buf[i] & 0x08) == 0) {	/* 11110xxx */
				c = buf[i] & 0x07;
				following = 3;
			} else if ((buf[i] & 0x04) == 0) {	/* 111110xx */
				c = buf[i] & 0x03;
				following = 4;
			} else if ((buf[i] & 0x02) == 0) {	/* 1111110x */
				c = buf[i] & 0x01;
				following = 5;
			} else
				return -1;

			for (n = 0; n < following; n++) {
				i++;
				if (i >= nbytes)
					goto done;

				if ((buf[i] & 0x80) == 0 || (buf[i] & 0x40))
					return -1;

				c = (c << 6) + (buf[i] & 0x3f);
			}

			gotone = 1;
		}
	}
done:
	return ctrl ? 0 : (gotone ? 2 : 1);
}

static int looks_utf8_with_BOM(const unsigned char *buf, size_t nbytes)
{
	if (nbytes > 3 && buf[0] == 0xef && buf[1] == 0xbb && buf[2] == 0xbf)
		return file_looks_utf8(buf + 3, nbytes - 3);
	else
		return -1;
}

static int looks_ucs16(const unsigned char *buf, size_t nbytes)
{
	int bigend;

	if (nbytes < 2)
		return 0;

	if (buf[0] == 0xff && buf[1] == 0xfe)
		bigend = 0;
	else if (buf[0] == 0xfe && buf[1] == 0xff)
		bigend = 1;
	else
		return 0;

	return 1 + bigend;
}

static int looks_latin1(const unsigned char *buf, size_t nbytes)
{
	size_t i;

	for (i = 0; i < nbytes; i++) {
		int t = text_chars[buf[i]];

		if (t != T && t != I)
			return 0;
	}

	return 1;
}

static int looks_extended(const unsigned char *buf, size_t nbytes)
{
	size_t i;

	for (i = 0; i < nbytes; i++) {
		int t = text_chars[buf[i]];

		if (t != T && t != I && t != X)
			return 0;
	}

	return 1;
}

static unsigned char ebcdic_to_ascii[] = {
  0,   1,   2,   3, 156,   9, 134, 127, 151, 141, 142,  11,  12,  13,  14,  15,
 16,  17,  18,  19, 157, 133,   8, 135,  24,  25, 146, 143,  28,  29,  30,  31,
128, 129, 130, 131, 132,  10,  23,  27, 136, 137, 138, 139, 140,   5,   6,   7,
144, 145,  22, 147, 148, 149, 150,   4, 152, 153, 154, 155,  20,  21, 158,  26,
' ', 160, 161, 162, 163, 164, 165, 166, 167, 168, 213, '.', '<', '(', '+', '|',
'&', 169, 170, 171, 172, 173, 174, 175, 176, 177, '!', '$', '*', ')', ';', '~',
'-', '/', 178, 179, 180, 181, 182, 183, 184, 185, 203, ',', '%', '_', '>', '?',
186, 187, 188, 189, 190, 191, 192, 193, 194, '`', ':', '#', '@', '\'','=', '"',
195, 'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 196, 197, 198, 199, 200, 201,
202, 'j', 'k', 'l', 'm', 'n', 'o', 'p', 'q', 'r', '^', 204, 205, 206, 207, 208,
209, 229, 's', 't', 'u', 'v', 'w', 'x', 'y', 'z', 210, 211, 212, '[', 214, 215,
216, 217, 218, 219, 220, 221, 222, 223, 224, 225, 226, 227, 228, ']', 230, 231,
'{', 'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 232, 233, 234, 235, 236, 237,
'}', 'J', 'K', 'L', 'M', 'N', 'O', 'P', 'Q', 'R', 238, 239, 240, 241, 242, 243,
'\\',159, 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z', 244, 245, 246, 247, 248, 249,
'0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 250, 251, 252, 253, 254, 255
};

static void from_ebcdic(const unsigned char *buf, size_t nbytes, unsigned char *out)
{
	size_t i;

	for (i = 0; i < nbytes; i++) {
		out[i] = ebcdic_to_ascii[buf[i]];
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////

DataCode ParseDataCode(const unsigned char * p_pcBuffer, size_t p_nBytes)
{
	DataCode l_DataCodeRet = FC_Unknown;

	int ucs_type;

	if (looks_ascii(p_pcBuffer, p_nBytes)) // 检查是否是ascii编码
	{
		l_DataCodeRet = FC_ASCII;
	}
	else if (looks_utf8_with_BOM(p_pcBuffer, p_nBytes) > 0)  // 检查是否是utf8带bom编码
	{
		l_DataCodeRet = FC_UTF8_Unicode_withBOM;
	}
	else if (file_looks_utf8(p_pcBuffer, p_nBytes) > 1)  // 检查是否是utf8不带bom编码
	{
		l_DataCodeRet = FC_UTF8_Unicode;
	}
	else if ((ucs_type = looks_ucs16(p_pcBuffer, p_nBytes)) != 0)   // 检查是否是unicode编码
	{
		if (ucs_type == 1)
		{
			l_DataCodeRet = FC_Little_endian_UTF16_Unicode; //小端
		}
		else
		{
			l_DataCodeRet = FC_Big_endian_UTF16_Unicode;  // 大端
		}
	}
	else if (looks_latin1(p_pcBuffer, p_nBytes))  // 检查是否是8859编码
	{
		l_DataCodeRet = FC_ISO_8859;
	}
	else if (looks_extended(p_pcBuffer, p_nBytes)) // 检查是否是扩展ASCII;编码
	{
		l_DataCodeRet = FC_Non_ISO_extended_ASCII;
	}
	else
	{
		size_t mlen;
		unsigned char * l_pcBuffer = NULL;

		mlen = (p_nBytes + 1) * sizeof(unsigned char);
		l_pcBuffer = (unsigned char *)calloc((size_t)1, mlen);

		from_ebcdic(p_pcBuffer, p_nBytes, l_pcBuffer);

		if (looks_ascii(l_pcBuffer, p_nBytes))  // 检查是否是EBCDIC编码
		{
			l_DataCodeRet = FC_EBCDIC;
		}
		else if (looks_latin1(l_pcBuffer, p_nBytes)) // 检查是否是国际EBCDIC编码
		{
			l_DataCodeRet = FC_International_EBCDIC;
		}

		free(l_pcBuffer);
	}

	return l_DataCodeRet;
}
