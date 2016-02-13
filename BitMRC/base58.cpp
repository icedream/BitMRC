/**
* Copyright (c) 2012-2014 Luke Dashjr
* Copyright (c) 2013-2014 Pavol Rusnak
*
* Permission is hereby granted, free of charge, to any person obtaining
* a copy of this software and associated documentation files (the "Software"),
* to deal in the Software without restriction, including without limitation
* the rights to use, copy, modify, merge, publish, distribute, sublicense,
* and/or sell copies of the Software, and to permit persons to whom the
* Software is furnished to do so, subject to the following conditions:
*
* The above copyright notice and this permission notice shall be included
* in all copies or substantial portions of the Software.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
* OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
* THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES
* OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
* ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
* OTHER DEALINGS IN THE SOFTWARE.
*/

#ifndef _SSIZE_T_DEFINED
#ifdef  _WIN64
typedef unsigned __int64    ssize_t;
#else
typedef unsigned int   ssize_t;
#endif
#define _SSIZE_T_DEFINED
#endif

#include <string.h>
#include <stdbool.h>
#include <sys/types.h>
#include "base58.h"

static const int8_t b58digits_map[] = {
	-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
	-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
	-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
	-1, 0, 1, 2, 3, 4, 5, 6, 7, 8,-1,-1,-1,-1,-1,-1,
	-1, 9,10,11,12,13,14,15,16,-1,17,18,19,20,21,-1,
	22,23,24,25,26,27,28,29,30,31,32,-1,-1,-1,-1,-1,
	-1,33,34,35,36,37,38,39,40,41,42,43,-1,44,45,46,
	47,48,49,50,51,52,53,54,55,56,57,-1,-1,-1,-1,-1,
};

bool b58tobin(void *bin, size_t *binszp, const char *b58)
{
	size_t binsz = *binszp;
	const unsigned char *b58u = (const unsigned char*)b58;
	unsigned char *binu =(unsigned char*) bin;
	size_t outisz = (binsz + 3) / 4;
	uint32_t * outi=new uint32_t[outisz];
	uint64_t t;
	uint32_t c;
	size_t i, j;
	uint8_t bytesleft = binsz % 4;
	uint32_t zeromask = bytesleft ? (0xffffffff << (bytesleft * 8)) : 0;
	unsigned zerocount = 0;
	size_t b58sz;

	b58sz = strlen(b58);

	memset(outi, 0, outisz * sizeof(*outi));

	// Leading zeros, just count
	for (i = 0; i < b58sz && !b58digits_map[b58u[i]]; ++i)
		++zerocount;

	for (; i < b58sz; ++i)
	{
		if (b58u[i] & 0x80)
		{
			// High-bit set on invalid digit
			delete outi;
			return false;
		}
		if (b58digits_map[b58u[i]] == -1)
		{
			// Invalid base58 digit
			delete outi;
			return false;
		}
		c = (unsigned)b58digits_map[b58u[i]];
		for (j = outisz; j--; )
		{
			t = ((uint64_t)outi[j]) * 58 + c;
			c = (t & 0x3f00000000) >> 32;
			outi[j] = t & 0xffffffff;
		}
		if (c)
		{
			// Output number too big (carry to the next int32)
			delete outi;
			return false;
		}
		if (outi[0] & zeromask)
		{
			// Output number too big (last int32 filled too far)
			delete outi;
			return false;
		}
	}

	j = 0;
	switch (bytesleft) {
	case 3:
		*(binu++) = (outi[0] & 0xff0000) >> 16;
	case 2:
		*(binu++) = (outi[0] & 0xff00) >> 8;
	case 1:
		*(binu++) = (outi[0] & 0xff);
		++j;
	default:
		break;
	}

	for (; j < outisz; ++j)
	{
		*(binu++) = (outi[j] >> 0x18) & 0xff;
		*(binu++) = (outi[j] >> 0x10) & 0xff;
		*(binu++) = (outi[j] >> 8) & 0xff;
		*(binu++) = (outi[j] >> 0) & 0xff;
	}

	// Count canonical base58 byte count
	binu =(unsigned char*) bin;
	for (i = 0; i < binsz; ++i)
	{
		if (binu[i])
			break;
		--*binszp;
	}
	*binszp += zerocount;

	//removing leading zeros
	j = binsz-*binszp;
	for (i = 0; i < binsz; i++)
	{
		((char*)bin)[i] = binu[j + i];
	}
	delete outi;
	return true;
}


static const char b58digits_ordered[] = "123456789ABCDEFGHJKLMNPQRSTUVWXYZabcdefghijkmnopqrstuvwxyz";

bool b58enc(char *b58, size_t *b58sz, const void *data, size_t binsz)
{
	const uint8_t *bin =(uint8_t*) data;
	int carry;
	ssize_t i, j, high, zcount = 0;
	size_t size;

	while (zcount < (ssize_t)binsz && !bin[zcount])
		++zcount;

	size = (binsz - zcount) * 138 / 100 + 1;
	uint8_t *buf= new uint8_t[size];
	memset(buf, 0, size);

	for (i = zcount, high = size - 1; i < (ssize_t)binsz; ++i, high = j)
	{
		for (carry = bin[i], j = size - 1; (j > high) || carry; --j)
		{
			carry += 256 * buf[j];
			buf[j] = carry % 58;
			carry /= 58;
		}
	}

	for (j = 0; j < (ssize_t)size && !buf[j]; ++j);

	if (*b58sz <= zcount + size - j)
	{
		*b58sz = zcount + size - j + 1;
		delete buf;
		return false;
	}

	if (zcount)
		memset(b58, '1', zcount);
	for (i = zcount; j < (ssize_t)size; ++i, ++j)
		b58[i] = b58digits_ordered[buf[j]];
	b58[i] = '\0';
	*b58sz = i + 1;

	delete buf;
	return true;
}
