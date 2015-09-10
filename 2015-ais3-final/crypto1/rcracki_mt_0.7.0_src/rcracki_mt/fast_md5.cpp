/*
 * Fast implementation of the MD5 message-digest algorithm as per RFC
 * (see http://tools.ietf.org/html/rfc1321)
 *
 * Author: Joao Inacio <jcinacio at gmail.com>
 * License: Use and share as you wish at your own risk, please keep this header ;)
 *
 * Optimizations:
 *  - For lengths < 16, transformation steps are "unrolled" using macros/defines
 *  - Constants used whenever possible, it's the compiler's job to sort them out
 *  - Padding is done on 4-byte words, and memory copied as last resort.
 *
 * freerainbowtables is a project for generating, distributing, and using
 * perfect rainbow tables
 *
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
 */

#include "fast_md5.h"

/* MD5 defines as per RFC reference implementation */

#define AC1				0xd76aa478
#define AC2				0xe8c7b756
#define AC3				0x242070db
#define AC4				0xc1bdceee
#define AC5				0xf57c0faf
#define AC6				0x4787c62a
#define AC7				0xa8304613
#define AC8				0xfd469501
#define AC9				0x698098d8
#define AC10			0x8b44f7af
#define AC11			0xffff5bb1
#define AC12			0x895cd7be
#define AC13			0x6b901122
#define AC14			0xfd987193
#define AC15			0xa679438e
#define AC16			0x49b40821
#define AC17			0xf61e2562
#define AC18			0xc040b340
#define AC19			0x265e5a51
#define AC20			0xe9b6c7aa
#define AC21			0xd62f105d
#define AC22			0x02441453
#define AC23			0xd8a1e681
#define AC24			0xe7d3fbc8
#define AC25			0x21e1cde6
#define AC26			0xc33707d6
#define AC27			0xf4d50d87
#define AC28			0x455a14ed
#define AC29			0xa9e3e905
#define AC30			0xfcefa3f8
#define AC31			0x676f02d9
#define AC32			0x8d2a4c8a
#define AC33			0xfffa3942
#define AC34			0x8771f681
#define AC35			0x6d9d6122
#define AC36			0xfde5380c
#define AC37			0xa4beea44
#define AC38			0x4bdecfa9
#define AC39			0xf6bb4b60
#define AC40			0xbebfbc70
#define AC41			0x289b7ec6
#define AC42			0xeaa127fa
#define AC43			0xd4ef3085
#define AC44			0x04881d05
#define AC45			0xd9d4d039
#define AC46			0xe6db99e5
#define AC47			0x1fa27cf8
#define AC48			0xc4ac5665
#define AC49			0xf4292244
#define AC50			0x432aff97
#define AC51			0xab9423a7
#define AC52			0xfc93a039
#define AC53			0x655b59c3
#define AC54			0x8f0ccc92
#define AC55			0xffeff47d
#define AC56			0x85845dd1
#define AC57			0x6fa87e4f
#define AC58			0xfe2ce6e0
#define AC59			0xa3014314
#define AC60			0x4e0811a1
#define AC61			0xf7537e82
#define AC62			0xbd3af235
#define AC63			0x2ad7d2bb
#define AC64			0xeb86d391

#define S11				 7
#define S12				12
#define S13				17
#define S14				22
#define S21				 5
#define S22				 9
#define S23				14
#define S24				20
#define S31				 4
#define S32				11
#define S33				16
#define S34				23
#define S41				 6
#define S42				10
#define S43				15
#define S44				21

#define Ca 				0x67452301
#define Cb 				0xefcdab89
#define Cc 				0x98badcfe
#define Cd 				0x10325476


#define F(x, y, z)			((z) ^ ((x) & ((y) ^ (z))))
#define G(x, y, z)			((y) ^ ((z) & ((x) ^ (y))))
//#define G(x, y, z)			F((z), (x), (y))
#define H(x, y, z)			((x) ^ (y) ^ (z))
#define I(x, y, z)			((y) ^ ((x) | ~(z)))

#define ROTATE_LEFT(x, n)	(((x) << (n)) | ((x) >> (32-(n))))

// md5 step
#define MD5STEP(f, a, b, c, d, AC, x, s) {	\
	(a) += f ((b), (c), (d));		\
	(a) += (AC) + (x);				\
	(a) = ROTATE_LEFT ((a), (s));	\
	(a) += (b);						\
}

// full MD5 transformation
#define MD5_steps(w0, w1, w2, w3, w4, w5, w6, w7, w8, w9, w10, w11, w12, w13, w14, w15) { \
	\
	MD5STEP(F, a, b, c, d,  AC1,  w0, S11);\
	MD5STEP(F, d, a, b, c,  AC2,  w1, S12);\
	MD5STEP(F, c, d, a, b,  AC3,  w2, S13);\
	MD5STEP(F, b, c, d, a,  AC4,  w3, S14);\
	MD5STEP(F, a, b, c, d,  AC5,  w4, S11);\
	MD5STEP(F, d, a, b, c,  AC6,  w5, S12);\
	MD5STEP(F, c, d, a, b,  AC7,  w6, S13);\
	MD5STEP(F, b, c, d, a,  AC8,  w7, S14);\
	MD5STEP(F, a, b, c, d,  AC9,  w8, S11);\
	MD5STEP(F, d, a, b, c, AC10,  w9, S12);\
	MD5STEP(F, c, d, a, b, AC11, w10, S13);\
	MD5STEP(F, b, c, d, a, AC12, w11, S14);\
	MD5STEP(F, a, b, c, d, AC13, w12, S11);\
	MD5STEP(F, d, a, b, c, AC14, w13, S12);\
	MD5STEP(F, c, d, a, b, AC15, w14, S13);\
	MD5STEP(F, b, c, d, a, AC16, w15, S14);\
	\
	MD5STEP(G, a, b, c, d, AC17,  w1, S21);\
	MD5STEP(G, d, a, b, c, AC18,  w6, S22);\
	MD5STEP(G, c, d, a, b, AC19, w11, S23);\
	MD5STEP(G, b, c, d, a, AC20,  w0, S24);\
	MD5STEP(G, a, b, c, d, AC21,  w5, S21);\
	MD5STEP(G, d, a, b, c, AC22, w10, S22);\
	MD5STEP(G, c, d, a, b, AC23, w15, S23);\
	MD5STEP(G, b, c, d, a, AC24,  w4, S24);\
	MD5STEP(G, a, b, c, d, AC25,  w9, S21);\
	MD5STEP(G, d, a, b, c, AC26, w14, S22);\
	MD5STEP(G, c, d, a, b, AC27,  w3, S23);\
	MD5STEP(G, b, c, d, a, AC28,  w8, S24);\
	MD5STEP(G, a, b, c, d, AC29, w13, S21);\
	MD5STEP(G, d, a, b, c, AC30,  w2, S22);\
	MD5STEP(G, c, d, a, b, AC31,  w7, S23);\
	MD5STEP(G, b, c, d, a, AC32, w12, S24);\
	\
	MD5STEP(H, a, b, c, d, AC33,  w5, S31);\
	MD5STEP(H, d, a, b, c, AC34,  w8, S32);\
	MD5STEP(H, c, d, a, b, AC35, w11, S33);\
	MD5STEP(H, b, c, d, a, AC36, w14, S34);\
	MD5STEP(H, a, b, c, d, AC37,  w1, S31);\
	MD5STEP(H, d, a, b, c, AC38,  w4, S32);\
	MD5STEP(H, c, d, a, b, AC39,  w7, S33);\
	MD5STEP(H, b, c, d, a, AC40, w10, S34);\
	MD5STEP(H, a, b, c, d, AC41, w13, S31);\
	MD5STEP(H, d, a, b, c, AC42,  w0, S32);\
	MD5STEP(H, c, d, a, b, AC43,  w3, S33);\
	MD5STEP(H, b, c, d, a, AC44,  w6, S34);\
	MD5STEP(H, a, b, c, d, AC45,  w9, S31);\
	MD5STEP(H, d, a, b, c, AC46, w12, S32);\
	MD5STEP(H, c, d, a, b, AC47, w15, S33);\
	MD5STEP(H, b, c, d, a, AC48,  w2, S34);\
	\
	MD5STEP(I, a, b, c, d, AC49,  w0, S41);\
	MD5STEP(I, d, a, b, c, AC50,  w7, S42);\
	MD5STEP(I, c, d, a, b, AC51, w14, S43);\
	MD5STEP(I, b, c, d, a, AC52,  w5, S44);\
	MD5STEP(I, a, b, c, d, AC53, w12, S41);\
	MD5STEP(I, d, a, b, c, AC54,  w3, S42);\
	MD5STEP(I, c, d, a, b, AC55, w10, S43);\
	MD5STEP(I, b, c, d, a, AC56,  w1, S44);\
	MD5STEP(I, a, b, c, d, AC57,  w8, S41);\
	MD5STEP(I, d, a, b, c, AC58, w15, S42);\
	MD5STEP(I, c, d, a, b, AC59,  w6, S43);\
	MD5STEP(I, b, c, d, a, AC60, w13, S44);\
	MD5STEP(I, a, b, c, d, AC61,  w4, S41);\
	MD5STEP(I, d, a, b, c, AC62, w11, S42);\
	MD5STEP(I, c, d, a, b, AC63,  w2, S43);\
	MD5STEP(I, b, c, d, a, AC64,  w9, S44);\
}

// len >= 56
#define MD5_transform_add(w0, w1, w2, w3, w4, w5, w6, w7, w8, w9, w10, w11, w12, w13, w14, w15) { \
	\
	a = wOut[0]; b = wOut[1]; c = wOut[2]; d = wOut[3];\
	\
	MD5_steps(w0, w1, w2, w3, w4, w5, w6, w7, w8, w9, w10, w11, w12, w13, w14, w15);\
	\
	wOut[0] += a; wOut[1] += b; wOut[2] += c; wOut[3] += d;\
}

// len < 56
#define MD5_transform_single(w0, w1, w2, w3, w4, w5, w6, w7, w8, w9, w10, w11, w12, w13, w14, w15) { \
	\
	a = CC[0]; b=CC[1]; c=CC[2]; d=CC[3];\
	\
	MD5_steps(w0, w1, w2, w3, w4, w5, w6, w7, w8, w9, w10, w11, w12, w13, w14, w15);\
	\
	wOut[0] = a+Ca; wOut[1] = b+Cb; wOut[2] = c+Cc; wOut[3] = d+Cd;\
}

// len < 16
#define MD5_transform_16(w0, w1, w2, w3, w14) \
	 MD5_transform_single(w0, w1, w2, w3, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, w14, 0);


// pad word and append 0x80 at appropriate location
#define MD5_pad_w0()		(0x00000080)
#define MD5_pad_w1(data)	(((data) & 0x000000FF) | 0x00008000)
#define MD5_pad_w2(data)	(((data) & 0x0000FFFF) | 0x00800000)
#define MD5_pad_w3(data)	(((data) & 0x00FFFFFF) | 0x80000000)


#ifndef MD5_pad_w1
static inline uint32_t MD5_pad_w1(uint32_t data)
{
// XXX x86 specific
	__asm__ (
		"movb 	%%al, 	%%cl	\n\t"
		"xorl 	%1,		%1		\n\t"
		"orb	$128, 	%%ah	\n\t"
		"movb	%%cl, 	%%al	\n\t"
		"movl 	%1, 	%0		\n\t"
		: "=r" (data)
		: "a" (data)
		: "cl"
	);
	return data;
}
#endif

#ifndef MD5_pad_w3
static inline uint32_t MD5_pad_w3(uint32_t data)
{
// XXX x86 specific
	__asm__ (
		"roll 	$8,		%1		\n\t"
		"movb	$128, 	%%al	\n\t"
		"rorl 	$8,		%1		\n\t"
		"movl 	%1, 	%0		\n\t"
		: "=r" (data)
		: "a" (data)
	);
	return data;
}
#endif


static inline void MD5_copy_pad_block(uint32_t *dData, uint32_t *wIn, int blocklen, int len)
{
	register int cl;

	// copy full words
	for (cl = 0; cl < blocklen; cl++)
		dData[cl] = wIn[cl];

	// copy with padding
	switch (len & 0x03) {
		case 0:
			dData[cl] = MD5_pad_w0();
			break;
		case 1:
			dData[cl] = MD5_pad_w1(wIn[cl]);
			break;
		case 2:
			dData[cl] = MD5_pad_w2(wIn[cl]);
			break;
		case 3:
			dData[cl] = MD5_pad_w3(wIn[cl]);
			break;
	}
	// append 0's
	for (cl++; cl < 14; cl++)
		dData[cl] = 0;
	// append len
	dData[cl++] = (len << 3);
	dData[cl] = (len >> 29);
}

// fast initializer array
//__attribute__((aligned(16)))
//__declspec(align(16))
static const uint32_t CC[4] = {Ca, Cb, Cc, Cd};



/*
 * fast_MD5()
 *
 */
void fast_MD5(unsigned char *pData, int len, unsigned char *pDigest)
{
	#define wIn		((uint32_t *)pData)
	#define wOut	((uint32_t *)pDigest)

	register uint32_t a;
	register uint32_t b;
	register uint32_t c;
	register uint32_t d;

	switch (len) {
		case 0:
			MD5_transform_16(MD5_pad_w0(), 0, 0, 0, 8*0);
			return;
		case 1:
			MD5_transform_16(MD5_pad_w1(wIn[0]), 0, 0, 0, 8*1);
			return;
		case 2:
			MD5_transform_16(MD5_pad_w2(wIn[0]), 0, 0, 0, 8*2);
			return;
		case 3:
			MD5_transform_16(MD5_pad_w3(wIn[0]), 0, 0, 0, 8*3);
			return;
		case 4:
			MD5_transform_16(wIn[0], MD5_pad_w0(), 0, 0, 8*4);
			return;
		case 5:
			MD5_transform_16(wIn[0], MD5_pad_w1(wIn[1]), 0, 0, 8*5);
			return;
		case 6:
			MD5_transform_16(wIn[0], MD5_pad_w2(wIn[1]), 0, 0, 8*6);
			return;
		case 7:
			MD5_transform_16(wIn[0], MD5_pad_w3(wIn[1]), 0, 0, 8*7);
			return;
		case 8:
			MD5_transform_16(wIn[0], wIn[1], MD5_pad_w0(), 0, 8*8);
			return;
		case 9:
			MD5_transform_16(wIn[0], wIn[1], MD5_pad_w1(wIn[2]), 0, 8*9);
			return;
		case 10:
			MD5_transform_16(wIn[0], wIn[1], MD5_pad_w2(wIn[2]), 0, 8*10);
			return;
		case 11:
			MD5_transform_16(wIn[0], wIn[1], MD5_pad_w3(wIn[2]), 0, 8*11);
			return;
		case 12:
			MD5_transform_16(wIn[0], wIn[1], wIn[2], MD5_pad_w0(), 8*12);
			return;
		case 13:
			MD5_transform_16(wIn[0], wIn[1], wIn[2], MD5_pad_w1(wIn[3]), 8*13);
			return;
		case 14:
			MD5_transform_16(wIn[0], wIn[1], wIn[2], MD5_pad_w2(wIn[3]), 8*14)
			return;
		case 15:
			MD5_transform_16(wIn[0], wIn[1], wIn[2], MD5_pad_w3(wIn[3]), 8*15)
			return;
	}

	// data block used for padding
	uint32_t dData[16];

	if (len < 56) {
		// 16 < length < 56

		MD5_copy_pad_block(dData, wIn, (len >> 2), len);

		// redefine data input, point to padded data
		#undef	wIn
		#define wIn		dData

		MD5_transform_single (
			wIn[ 0], wIn[ 1], wIn[ 2], wIn[ 3], wIn[ 4], wIn[ 5], wIn[ 6], wIn[ 7],
			wIn[ 8], wIn[ 9], wIn[10], wIn[11], wIn[12], wIn[13], wIn[14], wIn[15]
		);

		#undef	wIn
		return;
	} else {
		// len >= 56

		#define wIn		((uint32_t *)pData)

		// original len
		int tlen = len;

		// init digest for long lens
		wOut[0] = Ca; wOut[1] = Cb; wOut[2] = Cc; wOut[3] = Cd;

		while (tlen >= 64) {
			// Process 64-byte chunks
			MD5_transform_add(
				wIn[ 0], wIn[ 1], wIn[ 2], wIn[ 3], wIn[ 4], wIn[ 5], wIn[ 6], wIn[ 7],
				wIn[ 8], wIn[ 9], wIn[10], wIn[11], wIn[12], wIn[13], wIn[14], wIn[15]
			);

			tlen -= 64;
			pData += 64;
		}

		if (tlen >= 56) {
			// Process > 56-byte chunk

			int cl = (tlen >> 2);
			// perform padding on last 2 byte
			if (cl > 14) {
				dData[14] = wIn[14];
			} else {
				dData[15] = 0;
			}
			// copy 1 word with padding byte
			switch (len & 0x03) {
				case 0:
					dData[cl] = MD5_pad_w0();
					break;
				case 1:
					dData[cl] = MD5_pad_w1(wIn[cl]);
					break;
				case 2:
					dData[cl] = MD5_pad_w2(wIn[cl]);
					break;
				case 3:
					dData[cl] = MD5_pad_w3(wIn[cl]);
					break;
			}

			// transform
			MD5_transform_add(
				wIn[ 0], wIn[ 1], wIn[ 2], wIn[ 3], wIn[ 4], wIn[ 5], wIn[ 6], wIn[ 7],
				wIn[ 8], wIn[ 9], wIn[10], wIn[11], wIn[12], wIn[13], dData[14], dData[15]
			);
			// final transform
			#define w14		(len << 3)
			#define w15		(len >> 29)
			MD5_transform_add(
					0,	    0,		0,		0,		0, 		0,		0,		0,
					0, 		0,		0,		0,		0,		0,    w14,	  w15
			);
			#undef	w14
			#undef	w15
			return;
		} else {
			// (len mod 64) < 56

			MD5_copy_pad_block(dData, wIn, (tlen >> 2), len);

			#undef	wIn
			#define wIn		dData

			// transform
			MD5_transform_add(
				wIn[ 0], wIn[ 1], wIn[ 2], wIn[ 3], wIn[ 4], wIn[ 5], wIn[ 6], wIn[ 7],
				wIn[ 8], wIn[ 9], wIn[10], wIn[11], wIn[12], wIn[13], wIn[14], wIn[15]
			);

			#undef 	wIn
			#define wIn		((uint32_t *)pData)
			return;
		}
	}

	/* end of fast_MD5() */
}

