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

namespace RC_SHA1 {

__device__ __constant__ unsigned int h[5] = { 0x67452301, 0xEFCDAB89, 0x98BADCFE, 0x10325476, 0xC3D2E1F0 };

__device__ unsigned int SwapEndian(unsigned int n) {
	return (n<<24)|((n&0x0000ff00)<<8)|((n>>8)&0x0000ff00)|(n>>24);
}

__device__ unsigned int FF(unsigned int a, unsigned int b, unsigned int c, unsigned int d, unsigned int e, int i, unsigned int* data) {
	return ((a<<5)|(a>>27)) + ((b&c)|((~b)&d)) + e + 0x5A827999 + data[SHIDX(i)];
}

__device__ unsigned int FF2(unsigned int a, unsigned int b, unsigned int c, unsigned int d, unsigned int e, int i, unsigned int* data) {
	unsigned int dt;
	dt = data[SHIDX(i&15)]^data[SHIDX((i-3)&15)]^data[SHIDX((i-8)&15)]^data[SHIDX((i-14)&15)];
	data[SHIDX(i&15)] = dt = ((dt<<1)|(dt>>31));
	return ((a<<5)|(a>>27)) + ((b&c)|((~b)&d)) + e + 0x5A827999 + dt;
}

__device__ unsigned int GG(unsigned int a, unsigned int b, unsigned int c, unsigned int d, unsigned int e, int i, unsigned int* data) {
	unsigned int dt;
	dt = data[SHIDX(i&15)]^data[SHIDX((i-3)&15)]^data[SHIDX((i-8)&15)]^data[SHIDX((i-14)&15)];
	data[SHIDX(i&15)] = dt = ((dt<<1)|(dt>>31));
	return ((a<<5)|(a>>27)) + (b^c^d) + e + 0x6ED9EBA1 + dt;
}

__device__ unsigned int HH(unsigned int a, unsigned int b, unsigned int c, unsigned int d, unsigned int e, int i, unsigned int* data) {
	unsigned int dt;
	dt = data[SHIDX(i&15)]^data[SHIDX((i-3)&15)]^data[SHIDX((i-8)&15)]^data[SHIDX((i-14)&15)];
	data[SHIDX(i&15)] = dt = ((dt<<1)|(dt>>31));
	return ((a<<5)|(a>>27)) + ((b&c)|(b&d)|(c&d)) + e + 0x8F1BBCDC + dt;
}

__device__ unsigned int II(unsigned int a, unsigned int b, unsigned int c, unsigned int d, unsigned int e, int i, unsigned int* data) {
	unsigned int dt;
	dt = data[SHIDX(i&15)]^data[SHIDX((i-3)&15)]^data[SHIDX((i-8)&15)]^data[SHIDX((i-14)&15)];
	data[SHIDX(i&15)] = dt = ((dt<<1)|(dt>>31));
	return ((a<<5)|(a>>27)) + (b^c^d) + e + 0xCA62C1D6 + dt;
}

__device__ void SHA1(unsigned int* dataHash) {
	unsigned int a = h[0], b = h[1], c = h[2], d = h[3], e = h[4], x;
	int ii;

	// Round 1
	for(ii = 0; ii < 16; ii++) {
		x = FF(a, b, c, d, e, ii, dataHash);
		e = d; d = c; c = ((b<<30)|(b>>2)); b = a; a = x;
	}
	for(; ii < 20; ii++) {
		x = FF2(a, b, c, d, e, ii, dataHash);
		e = d; d = c; c = ((b<<30)|(b>>2)); b = a; a = x;
	}
	
	// Round 2
	for(; ii < 40; ii++) {
		x = GG(a, b, c, d, e, ii, dataHash);
		e = d; d = c; c = ((b<<30)|(b>>2)); b = a; a = x;
	}

	// Round 3
	for(; ii < 60; ii++) {
		x = HH(a, b, c, d, e, ii, dataHash);
		e = d; d = c; c = ((b<<30)|(b>>2)); b = a; a = x;
	}
	
	// Round 4
	for(; ii < 80; ii++) {
		x = II(a, b, c, d, e, ii, dataHash);
		e = d; d = c; c = ((b<<30)|(b>>2)); b = a; a = x;
	}

	dataHash[SHIDX(0)] = a + h[0];
	dataHash[SHIDX(1)] = b + h[1];
	dataHash[SHIDX(2)] = c + h[2];
	dataHash[SHIDX(3)] = d + h[3];
	dataHash[SHIDX(4)] = e + h[4];
}

}

#define RT_SHA1_KERNEL(kerName, kerPrologue, kerEpilogue)  \
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
	/* prepare for SHA1 */  \
	size = ii;  \
	ii = ((((3-(ii&3))<<3)-1)&0x1f)+1;  \
	plain = plain<<ii;  \
	for(jj++, idx = 0; jj <= (PLAIN_MAX_SIZE>>2)+1; plain = hData[SHIDX(jj++)], idx++)  \
		hData[SHIDX(idx)] = RC_SHA1::SwapEndian((plain>>ii)|(hData[SHIDX(jj)]<<(32-ii)));  \
	hData[SHIDX(idx)] = RC_SHA1::SwapEndian(plain>>ii);  \
	for(idx++; idx < 14; idx++)  \
		hData[SHIDX(idx)] = 0;  \
	hData[SHIDX(idx++)] = 0;  \
	hData[SHIDX(idx)] = size<<3;  \
	\
	/* hash */  \
	RC_SHA1::SHA1(hData);  \
	\
	hData[SHIDX(0)] = RC_SHA1::SwapEndian(hData[SHIDX(0)]);  \
	hData[SHIDX(1)] = RC_SHA1::SwapEndian(hData[SHIDX(1)]);  \
	\
	kerEpilogue;  \
}

#define RT_MYSQLSHA1_KERNEL(kerName, kerPrologue, kerEpilogue)  \
__global__ void kerName(unsigned int chainStart, unsigned int chainStop) {  \
	unsigned int size, jj, kk;  \
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
	/* prepare for SHA1 */  \
	size = ii;  \
	ii = ((((3-(ii&3))<<3)-1)&0x1f)+1;  \
	plain = plain<<ii;  \
	for(jj++, idx = 0; jj <= (PLAIN_MAX_SIZE>>2)+1; plain = hData[SHIDX(jj++)], idx++)  \
		hData[SHIDX(idx)] = RC_SHA1::SwapEndian((plain>>ii)|(hData[SHIDX(jj)]<<(32-ii)));  \
	hData[SHIDX(idx)] = RC_SHA1::SwapEndian(plain>>ii);  \
	for(kk = 0; kk < 2; kk++) {  \
		for(idx++; idx < 14; idx++)  \
			hData[SHIDX(idx)] = 0;  \
		hData[SHIDX(idx++)] = 0;  \
		hData[SHIDX(idx)] = size<<3;  \
		\
		/* hash */  \
		RC_SHA1::SHA1(hData);  \
		\
		idx = 5;  \
		hData[SHIDX(idx)] = RC_SHA1::SwapEndian(0x80);  \
		size = 20;  \
	}  \
	hData[SHIDX(0)] = RC_SHA1::SwapEndian(hData[SHIDX(0)]);  \
	hData[SHIDX(1)] = RC_SHA1::SwapEndian(hData[SHIDX(1)]);  \
	\
	kerEpilogue;  \
}


RT_SHA1_KERNEL(RTGenSHA1Kernel, RTGEN_PROLOGUE, RTGEN_EPILOGUE)
RT_SHA1_KERNEL(RTPreCalcSHA1Kernel, RTPRECALC_PROLOGUE, RTPRECALC_EPILOGUE)
RT_SHA1_KERNEL(RTCheckAlarmSHA1Kernel, RTCHKALARM_PROLOGUE, RTCHKALARM_EPILOGUE)

RT_MYSQLSHA1_KERNEL(RTGenMySQLSHA1Kernel, RTGEN_PROLOGUE, RTGEN_EPILOGUE)
RT_MYSQLSHA1_KERNEL(RTPreCalcMySQLSHA1Kernel, RTPRECALC_PROLOGUE, RTPRECALC_EPILOGUE)
RT_MYSQLSHA1_KERNEL(RTCheckAlarmMySQLSHA1Kernel, RTCHKALARM_PROLOGUE, RTCHKALARM_EPILOGUE)
