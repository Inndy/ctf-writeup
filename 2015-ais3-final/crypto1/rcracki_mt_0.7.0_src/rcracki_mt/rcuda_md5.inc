// freerainbowtables is a project for generating, distributing, and using
// perfect rainbow tables
//
// Copyright 2010, 2011 Jan Kyska
// Copyright 2010 Martin Westergaard Jørgensen <martinwj2005@gmail.com>
// Copyright 2010, 2011, 2012 James Nobis <quel@quelrod.net>
//
// This file is part of freerainbowtables.
//
// freerainbowtables is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 2 of the License, or
// (at your option) any later version.
//
// freerainbowtables is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with freerainbowtables.  If not, see <http://www.gnu.org/licenses/>.

namespace RC_MD5 {

__device__ __constant__ unsigned int h[4] = { 0x67452301, 0xEFCDAB89, 0x98BADCFE, 0x10325476 };
__device__ __constant__ unsigned char r[64] = { \
							7, 12, 17, 22,  7, 12, 17, 22,  7, 12, 17, 22,  7, 12, 17, 22, \
							5,  9, 14, 20,  5,  9, 14, 20,  5,  9, 14, 20,  5,  9, 14, 20, \
							4, 11, 16, 23,  4, 11, 16, 23,  4, 11, 16, 23,  4, 11, 16, 23, \
							6, 10, 15, 21,  6, 10, 15, 21,  6, 10, 15, 21,  6, 10, 15, 21 };
__device__ __constant__ unsigned char g[64] = { \
							0, 1,  2,  3,   4,  5,  6,  7,   8,  9, 10, 11,  12, 13, 14, 15, \
							1, 6, 11,  0,   5, 10, 15,  4,   9, 14,  3,  8,  13,  2,  7, 12, \
							5, 8, 11, 14,   1,  4,  7, 10,  13,  0,  3,  6,   9, 12, 15,  2, \
							0, 7, 14,  5,  12,  3, 10,  1,   8, 15,  6, 13,   4, 11,  2,  9 };
__device__ __constant__ unsigned int ac[64] = { \
							0xd76aa478, 0xe8c7b756, 0x242070db, 0xc1bdceee, \
							0xf57c0faf, 0x4787c62a, 0xa8304613, 0xfd469501, \
							0x698098d8, 0x8b44f7af, 0xffff5bb1, 0x895cd7be, \
							0x6b901122, 0xfd987193, 0xa679438e, 0x49b40821, \
							\
							0xf61e2562, 0xc040b340, 0x265e5a51, 0xe9b6c7aa, \
							0xd62f105d, 0x02441453, 0xd8a1e681, 0xe7d3fbc8, \
							0x21e1cde6, 0xc33707d6, 0xf4d50d87, 0x455a14ed, \
							0xa9e3e905, 0xfcefa3f8, 0x676f02d9, 0x8d2a4c8a, \
							\
							0xfffa3942, 0x8771f681, 0x6d9d6122, 0xfde5380c, \
							0xa4beea44, 0x4bdecfa9, 0xf6bb4b60, 0xbebfbc70, \
							0x289b7ec6, 0xeaa127fa, 0xd4ef3085, 0x04881d05, \
							0xd9d4d039, 0xe6db99e5, 0x1fa27cf8, 0xc4ac5665, \
							\
							0xf4292244, 0x432aff97, 0xab9423a7, 0xfc93a039, \
							0x655b59c3, 0x8f0ccc92, 0xffeff47d, 0x85845dd1, \
							0x6fa87e4f, 0xfe2ce6e0, 0xa3014314, 0x4e0811a1, \
							0xf7537e82, 0xbd3af235, 0x2ad7d2bb, 0xeb86d391 };

__device__ unsigned int FF(unsigned int a, unsigned int b, unsigned int c, unsigned int d, int i, const unsigned int* data) {
	unsigned int ret;
	ret = a + ((b&c)|((~b)&d)) + ac[i] + data[SHIDX(g[i])];
	ret = (ret<<r[i])|(ret>>(32-r[i]));
	ret += b;
	return ret;
}

__device__ unsigned int GG(unsigned int a, unsigned int b, unsigned int c, unsigned int d, int i, const unsigned int* data) {
	unsigned int ret;
	ret = a + ((b&d)|(c&(~d))) + ac[i] + data[SHIDX(g[i])];
	ret = (ret<<r[i])|(ret>>(32-r[i]));
	ret += b;
	return ret;
}

__device__ unsigned int HH(unsigned int a, unsigned int b, unsigned int c, unsigned int d, int i, const unsigned int* data) {
	unsigned int ret;
	ret = a + (b^c^d) + ac[i] + data[SHIDX(g[i])];
	ret = (ret<<r[i])|(ret>>(32-r[i]));
	ret += b;
	return ret;
}

__device__ unsigned int II(unsigned int a, unsigned int b, unsigned int c, unsigned int d, int i, const unsigned int* data) {
	unsigned int ret;
	ret = a + (c^(b|(~d))) + ac[i] + data[SHIDX(g[i])];
	ret = (ret<<r[i])|(ret>>(32-r[i]));
	ret += b;
	return ret;
}

__device__ void MD5(unsigned int* dataHash) {
	unsigned int a = h[0], b = h[1], c = h[2], d = h[3], x;
	int ii;

	// Round 1
	for(ii = 0; ii < 16; ii++) {
		x = b;
		b = FF(a, b, c, d, ii, dataHash);
		a = d; d = c; c = x;
	}

	// Round 2
	for(; ii < 32; ii++) {
		x = b;
		b = GG(a, b, c, d, ii, dataHash);
		a = d; d = c; c = x;
	}
	
	// Round 3
	for(; ii < 48; ii++) {
		x = b;
		b = HH(a, b, c, d, ii, dataHash);
		a = d; d = c; c = x;
	}
	
	// Round 4
	for(; ii < 64; ii++) {
		x = b;
		b = II(a, b, c, d, ii, dataHash);
		a = d; d = c; c = x;
	}

	dataHash[SHIDX(0)] = a + h[0];
	dataHash[SHIDX(1)] = b + h[1];
	dataHash[SHIDX(2)] = c + h[2];
	dataHash[SHIDX(3)] = d + h[3];
}

}

#define RT_MD5_KERNEL(kerName, kerPrologue, kerEpilogue)  \
__global__ void kerName(unsigned int chainStart, unsigned int chainStop) {  \
	unsigned int size, jj;  \
	unsigned int plain;  \
	\
	kerPrologue;  \
	\
	/* transform to the plain text */ \
	plain = 0x80;  \
	jj = (PLAIN_MAX_SIZE>>2)+1;  \
	\
	RTGEN_IDX2PLAIN_BEGIN;  \
		RTGEN_I2P_LOOP64_BEGIN;  \
			plain = (plain<<8) | cplChrSet[(dimItem.z&255u) + uiDiv];  \
			if((ii&3) == 2) {  \
				hData[SHIDX(jj--)] = plain;  \
				plain = 0;  \
			}  \
		RTGEN_I2P_LOOP64_END;  \
		\
		RTGEN_I2P_LOOP32_BEGIN;  \
			plain = (plain<<8) | cplChrSet[(dimItem.z&255u) + uiDiv];  \
			if((ii&3) == 2) {  \
				hData[SHIDX(jj--)] = plain;  \
				plain = 0;  \
			}  \
		RTGEN_I2P_LOOP32_END;  \
	RTGEN_IDX2PLAIN_END;  \
	\
	/* prepare for MD5 */  \
	size = ii;  \
	ii = ((((3-(ii&3))<<3)-1)&0x1f)+1;  \
	plain = plain<<ii;  \
	for(jj++, idx = 0; jj <= (PLAIN_MAX_SIZE>>2)+1; plain = hData[SHIDX(jj++)], idx++)  \
		hData[SHIDX(idx)] = (plain>>ii)|(hData[SHIDX(jj)]<<(32-ii));  \
	hData[SHIDX(idx)] = plain>>ii;  \
	for(idx++; idx < 14; idx++)  \
		hData[SHIDX(idx)] = 0;  \
	hData[SHIDX(idx++)] = size<<3;  \
	hData[SHIDX(idx)] = 0;  \
	\
	/* hash */  \
	RC_MD5::MD5(hData);  \
	\
	kerEpilogue;  \
}


RT_MD5_KERNEL(RTGenMD5Kernel, RTGEN_PROLOGUE, RTGEN_EPILOGUE)
RT_MD5_KERNEL(RTPreCalcMD5Kernel, RTPRECALC_PROLOGUE, RTPRECALC_EPILOGUE)
RT_MD5_KERNEL(RTCheckAlarmMD5Kernel, RTCHKALARM_PROLOGUE, RTCHKALARM_EPILOGUE)
