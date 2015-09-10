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

namespace RC_MD4 {

__device__ __constant__ unsigned int h[4] = { 0x67452301, 0xEFCDAB89, 0x98BADCFE, 0x10325476 };
__device__ __constant__ unsigned char r[48] = { \
							3,  7, 11, 19,  3,  7, 11, 19,  3,  7, 11, 19,  3,  7, 11, 19, \
							3,  5,  9, 13,  3,  5,  9, 13,  3,  5,  9, 13,  3,  5,  9, 13, \
							3,  9, 11, 15,  3,  9, 11, 15,  3,  9, 11, 15,  3,  9, 11, 15 };
__device__ __constant__ unsigned char g[48] = { \
							0, 1,  2,  3,   4,  5,  6,  7,   8,  9, 10, 11,  12, 13, 14, 15, \
							0, 4,  8, 12,   1,  5,  9, 13,   2,  6, 10, 14,   3,  7, 11, 15, \
							0, 8,  4, 12,   2, 10,  6, 14,   1,  9,  5, 13,   3, 11,  7, 15 };

__device__ unsigned int FF(unsigned int a, unsigned int b, unsigned int c, unsigned int d, int i, const unsigned int* data) {
	unsigned int ret;
	ret = a + ((b&c)|((~b)&d)) + data[SHIDX(g[i])];
	ret = (ret<<r[i])|(ret>>(32-r[i]));
	return ret;
}

__device__ unsigned int GG(unsigned int a, unsigned int b, unsigned int c, unsigned int d, int i, const unsigned int* data) {
	unsigned int ret;
	ret = a + ((b&c)|(b&d)|(c&d)) + data[SHIDX(g[i])] + 0x5a827999u;
	ret = (ret<<r[i])|(ret>>(32-r[i]));
	return ret;
}

__device__ unsigned int HH(unsigned int a, unsigned int b, unsigned int c, unsigned int d, int i, const unsigned int* data) {
	unsigned int ret;
	ret = a + (b^c^d) + data[SHIDX(g[i])] + 0x6ed9eba1u;
	ret = (ret<<r[i])|(ret>>(32-r[i]));
	return ret;
}

__device__ void MD4(unsigned int* dataHash) {
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
	
	dataHash[SHIDX(0)] = a + h[0];
	dataHash[SHIDX(1)] = b + h[1];
	dataHash[SHIDX(2)] = c + h[2];
	dataHash[SHIDX(3)] = d + h[3];
}

}

#define RT_MD4_KERNEL(kerName, kerPrologue, kerEpilogue)  \
__global__ void kerName(unsigned int chainStart, unsigned int chainStop) {  \
	unsigned int size, jj;  \
	unsigned int plain;  \
	\
	kerPrologue;  \
	\
	/* transform to the plain text */  \
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
	/* prepare for MD4 */  \
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
	RC_MD4::MD4(hData);  \
	\
	kerEpilogue;  \
}


RT_MD4_KERNEL(RTGenMD4Kernel, RTGEN_PROLOGUE, RTGEN_EPILOGUE)
RT_MD4_KERNEL(RTPreCalcMD4Kernel, RTPRECALC_PROLOGUE, RTPRECALC_EPILOGUE)
RT_MD4_KERNEL(RTCheckAlarmMD4Kernel, RTCHKALARM_PROLOGUE, RTCHKALARM_EPILOGUE)
