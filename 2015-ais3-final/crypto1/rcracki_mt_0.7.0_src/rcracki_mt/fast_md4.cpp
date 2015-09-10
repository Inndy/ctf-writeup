/*
 * freerainbowtables is a project for generating, distributing, and using
 * perfect rainbow tables
 *
 * Copyright Bitweasil
 * Copyright 2009, 2010 Daniël Niggebrugge <niggebrugge@fox-it.com>
 * Copyright 2009, 2010, 2011, 2012 James Nobis <quel@quelrod.net>
 *
 * This file is part of freerainbowtables.
 *
 * freerainbowtables is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 2 of the License, or
 * (at your option) any later version.
 *
 * freerainbowtables is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with freerainbowtables.  If not, see <http://www.gnu.org/licenses/>.
 *
 * This code implements the MD4 message-digest algorithm.
 * "Just the reference implementation, single stage. Hardly "optimized." Though a good bit faster than libssl's MD4, as it isn't doing nearly the same amount of work." - Bitweasil
 * 
 * little bit optimized (or at least attempted) for NTLM (unicode) by neinbrucke
 */

//#include <cstdlib>
//#include <cstring>
#include "fast_md4.h"

/* MD4 Defines as per RFC reference implementation */
#define F(x, y, z) (((x) & (y)) | ((~x) & (z)))
#define G(x, y, z) (((x) & (y)) | ((x) & (z)) | ((y) & (z)))
#define H(x, y, z) ((x) ^ (y) ^ (z))
#define ROTATE_LEFT(x, n) (((x) << (n)) | ((x) >> (32-(n))))
#define FF(a, b, c, d, x, s) { \
		(a) += F ((b), (c), (d)) + (x); \
		(a) = ROTATE_LEFT ((a), (s)); \
	}
#define GG(a, b, c, d, x, s) { \
		(a) += G ((b), (c), (d)) + (x) + (uint32_t)0x5a827999; \
		(a) = ROTATE_LEFT ((a), (s)); \
	}
#define HH(a, b, c, d, x, s) { \
		(a) += H ((b), (c), (d)) + (x) + (uint32_t)0x6ed9eba1; \
		(a) = ROTATE_LEFT ((a), (s)); \
	}
#define S11 3
#define S12 7
#define S13 11
#define S14 19
#define S21 3
#define S22 5
#define S23 9
#define S24 13
#define S31 3
#define S32 9
#define S33 11
#define S34 15
/* End MD4 Defines */

void MD4_NEW( unsigned char * pData, int length, unsigned char * pDigest)
{
	// access data as 4-byte word
	#define	uData 				((uint32_t *)pData)
	#define	uDigest				((uint32_t *)pDigest)

	// pad word and append bit at appropriate location
	#define MD4_pad_w0()		(0x00000080)
	#define MD4_pad_w1(data)	(((data) & 0x000000FF) | 0x00008000)
	#define MD4_pad_w2(data)	(((data) & 0x0000FFFF) | 0x00800000)
	#define MD4_pad_w3(data)	(((data) & 0x00FFFFFF) | 0x80000000)

	// For the hash working space
	//__attribute__((aligned(16))) uint32_t data[16] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
	//__declspec(align(16)) uint32_t data[16] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
	uint32_t data[MD4_DIGEST_LENGTH] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};

	// For the output result
	uint32_t a,b,c,d;

	switch (length)
	{
		case 0:
		{
			data[ 0] = MD4_pad_w0();

			data[14] = 0;
		}
		break;
		case 1:
		{
			data[ 0] = MD4_pad_w1(uData[0]);

			data[14] = 1 << 3;
		}
		break;
		case 2:
		{
			data[ 0] = MD4_pad_w2(uData[0]);

			data[14] = 2 << 3;
		}
		break;
		case 3:
		{
			data[ 0] = MD4_pad_w3(uData[0]);

			data[14] = 3 << 3;
		}
		break;
		case 4:
		{
			data[ 0] = uData[0];
			data[ 1] = MD4_pad_w0();

			data[14] = 4 << 3;
		}
		break;
		case 5:
		{
			data[ 0] = uData[0];
			data[ 1] = MD4_pad_w1(uData[1]);

			data[14] = 5 << 3;
		}
		break;
		case 6:
		{
			data[ 0] = uData[0];
			data[ 1] = MD4_pad_w2(uData[1]);

			data[14] = 6 << 3;
		}
		break;
		case 7:
		{
			data[ 0] = uData[0];
			data[ 1] = MD4_pad_w3(uData[1]);

			data[14] = 7 << 3;
		}
		break;
		case 8:
		{
			data[ 0] = uData[0];
			data[ 1] = uData[1];
			data[ 2] = MD4_pad_w0();

			data[14] = 8 << 3;
		}
		break;
		case 9:
		{
			data[ 0] = uData[0];
			data[ 1] = uData[1];
			data[ 2] = MD4_pad_w1(uData[2]);

			data[14] = 9 << 3;
		}
		break;
		case 10:
		{
			data[ 0] = uData[0];
			data[ 1] = uData[1];
			data[ 2] = MD4_pad_w2(uData[2]);

			data[14] = 10 << 3;
		}
		break;
		case 11:
		{
			data[ 0] = uData[0];
			data[ 1] = uData[1];
			data[ 2] = MD4_pad_w3(uData[2]);

			data[14] = 11 << 3;
		}
		break;
		case 12:
		{
			data[ 0] = uData[0];
			data[ 1] = uData[1];
			data[ 2] = uData[2];
			data[ 3] = MD4_pad_w0();

			data[14] = 12 << 3;
		}
		break;
		case 13:
		{
			data[ 0] = uData[0];
			data[ 1] = uData[1];
			data[ 2] = uData[2];
			data[ 3] = MD4_pad_w1(uData[3]);

			data[14] = 13 << 3;
		}
		break;
		case 14:
		{
			data[ 0] = uData[0];
			data[ 1] = uData[1];
			data[ 2] = uData[2];
			data[ 3] = MD4_pad_w2(uData[3]);

			data[14] = 14 << 3;
		}
		break;
		case 15:
		{
			data[ 0] = uData[0];
			data[ 1] = uData[1];
			data[ 2] = uData[2];
			data[ 3] = MD4_pad_w3(uData[3]);

			data[14] = 15 << 3;
		}
		break;

		default:
		{
			length = length % 32; // lenght >= 32 not suported

			int word = length >> 2;

			int i = 0;
			while (i < word) {
				data[i] = uData[i];
				i++;
			}

			switch (length & 0x3) {
				case 0:
				{
					data[word] = MD4_pad_w0();
				}
				break;
				case 1:
				{
					data[word] = MD4_pad_w1(uData[word]);
				}
				break;
				case 2:
				{
					data[word] = MD4_pad_w2(uData[word]);
				}
				break;
				case 3:
				{
					data[word] = MD4_pad_w3(uData[word]);
				}
				break;
			}

			data[14] = length << 3;
		}
		break;
	}

	a = 0x67452301;
	b = 0xefcdab89;
	c = 0x98badcfe;
	d = 0x10325476;

	/* Round 1 */
	FF (a, b, c, d, data[ 0], S11); /* 1 */
	FF (d, a, b, c, data[ 1], S12); /* 2 */
	FF (c, d, a, b, data[ 2], S13); /* 3 */
	FF (b, c, d, a, data[ 3], S14); /* 4 */
	FF (a, b, c, d, data[ 4], S11); /* 5 */
	FF (d, a, b, c, data[ 5], S12); /* 6 */
	FF (c, d, a, b, data[ 6], S13); /* 7 */
	FF (b, c, d, a, data[ 7], S14); /* 8 */
	FF (a, b, c, d,        0, S11); /* 9 */
	FF (d, a, b, c,        0, S12); /* 10 */
	FF (c, d, a, b,        0, S13); /* 11 */
	FF (b, c, d, a,        0, S14); /* 12 */
	FF (a, b, c, d,        0, S11); /* 13 */
	FF (d, a, b, c,        0, S12); /* 14 */
	FF (c, d, a, b, data[14], S13); /* 15 */
	FF (b, c, d, a,        0, S14); /* 16 */

	/* Round 2 */
	GG (a, b, c, d, data[ 0], S21); /* 17 */
	GG (d, a, b, c, data[ 4], S22); /* 18 */
	GG (c, d, a, b,        0, S23); /* 19 */
	GG (b, c, d, a,        0, S24); /* 20 */
	GG (a, b, c, d, data[ 1], S21); /* 21 */
	GG (d, a, b, c, data[ 5], S22); /* 22 */
	GG (c, d, a, b,        0, S23); /* 23 */
	GG (b, c, d, a,        0, S24); /* 24 */
	GG (a, b, c, d, data[ 2], S21); /* 25 */
	GG (d, a, b, c, data[ 6], S22); /* 26 */
	GG (c, d, a, b,        0, S23); /* 27 */
	GG (b, c, d, a, data[14], S24); /* 28 */
	GG (a, b, c, d, data[ 3], S21); /* 29 */
	GG (d, a, b, c, data[ 7], S22); /* 30 */
	GG (c, d, a, b,        0, S23); /* 31 */
	GG (b, c, d, a,        0, S24); /* 32 */

	/* Round 3 */
	HH (a, b, c, d, data[ 0], S31); /* 33 */
	HH (d, a, b, c,        0, S32); /* 34 */
	HH (c, d, a, b, data[ 4], S33); /* 35 */
	HH (b, c, d, a,        0, S34); /* 36 */
	HH (a, b, c, d, data[ 2], S31); /* 37 */
	HH (d, a, b, c,        0, S32); /* 38 */
	HH (c, d, a, b, data[ 6], S33); /* 39 */
	HH (b, c, d, a, data[14], S34); /* 40 */
	HH (a, b, c, d, data[ 1], S31); /* 41 */
	HH (d, a, b, c,        0, S32); /* 42 */
	HH (c, d, a, b, data[ 5], S33); /* 43 */
	HH (b, c, d, a,        0, S34); /* 44 */
	HH (a, b, c, d, data[ 3], S31); /* 45 */
	HH (d, a, b, c,        0, S32); /* 46 */
	HH (c, d, a, b, data[ 7], S33); /* 47 */
	HH (b, c, d, a,        0, S34); /* 48 */

	// Finally, add initial values, as this is the only pass we make.
	a += 0x67452301;
	b += 0xefcdab89;
	c += 0x98badcfe;
	d += 0x10325476;

	uDigest[0] = a;
	uDigest[1] = b;
	uDigest[2] = c;
	uDigest[3] = d;
}
