/*
 * rcracki_mt is a multithreaded implementation and fork of the original 
 * RainbowCrack
 *
 * Copyright Martin Westergaard Jørgensen <martinwj2005@gmail.com>
 * Copyright Wei Dai <weidai@eskimo.com>
 * Copyright 2009, 2010 Daniël Niggebrugge <niggebrugge@fox-it.com>
 * Copyright 2009, 2010, 2011, 2012 James Nobis <quel@quelrod.net>
 *
 * This file is part of rcracki_mt.
 *
 * rcracki_mt is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 2 of the License, or
 * (at your option) any later version.
 *
 * rcracki_mt is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with rcracki_mt.  If not, see <http://www.gnu.org/licenses/>.
 */

//#include <stdio.h>
#if defined(_WIN32)
	#include <windows.h>
#endif

#include <string.h>

#include "sha1.h"

#define SHA1CircularShift(bits,word) (((word) << (bits)) | ((word) >> (32-(bits))))

// this rotate isn't faster with me
#if defined(_WIN32)
	#define ROTATE(a,n)     _lrotl(a,n)
#else
	#define ROTATE(a,n)     (((a)<<(n))|(((a)&0xffffffff)>>(32-(n))))
#endif

/* A nice byte order reversal from Wei Dai <weidai@eskimo.com> */
#if defined(_WIN32)
/* 5 instructions with rotate instruction, else 9 */
#define Endian_Reverse32(a) \
	{ \
	unsigned long l=(a); \
	(a)=((ROTATE(l,8)&0x00FF00FF)|(ROTATE(l,24)&0xFF00FF00)); \
	}
#else
/* 6 instructions with rotate instruction, else 8 */
#define Endian_Reverse32(a) \
	{ \
	unsigned long l=(a); \
	l=(((l&0xFF00FF00)>>8L)|((l&0x00FF00FF)<<8L)); \
	(a)=ROTATE(l,16L); \
	}
#endif

#define	F_00_19(b,c,d)	((((c) ^ (d)) & (b)) ^ (d)) 
#define	F_20_39(b,c,d)	((b) ^ (c) ^ (d))
#define F_40_59(b,c,d)	(((b) & (c)) | (((b)|(c)) & (d))) 
#define	F_60_79(b,c,d)	F_20_39(b,c,d)

#define K0 0x5A827999
#define K1 0x6ED9EBA1
#define K2 0x8F1BBCDC
#define K3 0xCA62C1D6

#define H0 0x67452301
#define H1 0xEFCDAB89
#define H2 0x98BADCFE
#define H3 0x10325476
#define H4 0xC3D2E1F0

#define SHA1HashSize 20

void SHA1_NEW( unsigned char * pData, int length, unsigned char * pDigest)
{
	if (length > 16)
		return;

	uint32_t Message_Block_Index    = 0;

	union
	{
		unsigned char Message_Block[64];
		uint32_t Message_Block_W[16];
	};

	Message_Block_W[0] = 0x00000000;
	Message_Block_W[1] = 0x00000000;
	Message_Block_W[2] = 0x00000000;
	Message_Block_W[3] = 0x00000000;
	Message_Block_W[4] = 0x00000000;

	uint32_t Intermediate_Hash[5] = { H0, H1, H2, H3, H4 };
	
	memcpy(Message_Block, pData, length);
	Message_Block_Index += length;

	//padMessage
	Message_Block[length] = 0x80;
	
	uint32_t W_15 = length << 3;

	int           t;                 /* Loop counter                */
	uint32_t      temp;              /* Temporary word value        */
	uint32_t      W[80];             /* Word sequence               */
	uint32_t      A, B, C, D, E;     /* Word buffers                */

    /*
     *  Initialize the first 16 words in the array W
     */

	#define INIT(x) W[x] = Message_Block_W[x];
	
	#define INIT_NULL(x) W[x] = 0;

	
	Endian_Reverse32(Message_Block_W[0]);
	INIT(0);

	#define INIT_NULL_1_14 \
		INIT_NULL(1);  INIT_NULL_2_14;

	#define INIT_NULL_2_14 \
		INIT_NULL(2);  INIT_NULL_3_14;

	#define INIT_NULL_3_14 \
		INIT_NULL(3);  INIT_NULL_4_14;

	#define INIT_NULL_4_14 \
		INIT_NULL(4); INIT_NULL_5_14;

	#define INIT_NULL_5_14 \
		INIT_NULL(5);  INIT_NULL(6);  INIT_NULL(7); \
		INIT_NULL(8);  INIT_NULL(9);  INIT_NULL(10); INIT_NULL(11); \
		INIT_NULL(12); INIT_NULL(13); INIT_NULL(14);

	#define ROTATE1_NULL_5_14 \
		ROTATE1_NULL; ROTATE1_NULL; ROTATE1_NULL; \
		ROTATE1_NULL; ROTATE1_NULL; ROTATE1_NULL; ROTATE1_NULL; \
		ROTATE1_NULL; ROTATE1_NULL; ROTATE1_NULL;


	#define EXPAND(t) \
		W[t] = SHA1CircularShift(1,W[t-3] ^ W[t-8] ^ W[t-14] ^ W[t-16]); \

	#define EXPAND_3(t) W[t] = SHA1CircularShift(1,W[t-3]);
	#define EXPAND_16(t) W[t] = SHA1CircularShift(1,W[t-16]);
	#define EXPAND_3_8(t) W[t] = SHA1CircularShift(1,W[t-3] ^ W[t-8]);

	if (length < 4) {
		INIT_NULL_1_14;
		W[15] = W_15;
		EXPAND_16(16);
		W[17] = 0;
		W[18] = W_15<<1;
	}
	else if (length < 8) {
		Endian_Reverse32(Message_Block_W[1]);
		INIT(1);
		INIT_NULL_2_14;
		W[15] = W_15;
		EXPAND_16(16);
		EXPAND_16(17);
		W[18] = W_15<<1;
	}
	else {
		Endian_Reverse32(Message_Block_W[1]);
		Endian_Reverse32(Message_Block_W[2]);
		Endian_Reverse32(Message_Block_W[3]);
		Endian_Reverse32(Message_Block_W[4]);
		INIT(1); INIT(2); INIT(3); INIT(4); 
		INIT_NULL_5_14;
		W[15] = W_15;
		EXPAND(16);
		EXPAND(17);
		EXPAND(18);
	}

	if (length < 12) {
		EXPAND_3(19);
	}
	else {
		EXPAND(19);
	}

	if (length < 16) {
		EXPAND_3(20);
	}
	else {
		EXPAND(20);
	}
	EXPAND_3(21); EXPAND_3(22);
	EXPAND(23);

	EXPAND(24); EXPAND(25); EXPAND_3_8(26);	EXPAND_3_8(27);
	EXPAND(28);	EXPAND(29);	EXPAND(30);	EXPAND(31);
	EXPAND(32);	EXPAND(33);	EXPAND(34);	EXPAND(35);
	EXPAND(36);	EXPAND(37);	EXPAND(38);	EXPAND(39);
	EXPAND(40);	EXPAND(41);	EXPAND(42);	EXPAND(43);
	EXPAND(44);	EXPAND(45);	EXPAND(46);	EXPAND(47);
	EXPAND(48);	EXPAND(49);	EXPAND(50);	EXPAND(51);
	EXPAND(52);	EXPAND(53);	EXPAND(54);	EXPAND(55);
	EXPAND(56);	EXPAND(57);	EXPAND(58);	EXPAND(59);
	EXPAND(60);	EXPAND(61);	EXPAND(62);	EXPAND(63);
	EXPAND(64);	EXPAND(65);	EXPAND(66);	EXPAND(67);
	EXPAND(68);	EXPAND(69);	EXPAND(70);	EXPAND(71);
	EXPAND(72);	EXPAND(73);	EXPAND(74);	EXPAND(75);
	EXPAND(76);	EXPAND(77);	EXPAND(78);	EXPAND(79);


	#define ROTATE1_NEW(a, b, c, d, e, x) \
			e += SHA1CircularShift(5,a) + F_00_19(b,c,d) + x + K0; \
			b = SHA1CircularShift(30,b);

	#define ROTATE1_NULL \
			temp = SHA1CircularShift(5,A) + F_00_19(B,C,D) + E + K0; \
			E = D; D = C; \
			C = SHA1CircularShift(30,B); \
			B = A; A = temp; \
			
	#define ROTATE2_NEW(a, b, c, d, e, x) \
			e += SHA1CircularShift(5,a) + F_20_39(b,c,d) + x + K1; \
			b = SHA1CircularShift(30,b);
	
	#define ROTATE2(t) \
		temp = SHA1CircularShift(5,A) + F_20_39(B,C,D) + E + W[t] + K1; \
		E = D; D = C; \
		C = SHA1CircularShift(30,B); \
		B = A; A = temp;

	#define ROTATE2_W(w) \
		temp = SHA1CircularShift(5,A) + F_20_39(B,C,D) + E + w + K1; \
		E = D; D = C; \
		C = SHA1CircularShift(30,B); \
		B = A; A = temp;

	#define ROTATE3(t) \
		temp = SHA1CircularShift(5,A) + F_40_59(B,C,D) + E + W[t] + K2; \
		E = D; D = C; \
		C = SHA1CircularShift(30,B); \
		B = A; A = temp;

	#define ROTATE4(t) \
		temp = SHA1CircularShift(5,A) + F_60_79(B,C,D) + E + W[t] + K3; \
		E = D; D = C; \
		C = SHA1CircularShift(30,B); \
		B = A; A = temp;

	A = H0;
	B = H1;
	C = H2;
	D = H3;
	E = H4;


	E = H2;
	//D = 2079550178;
	//C = 1506887872;
	B = 2679412915u + W[0];
	if (length < 4) {
		A = SHA1CircularShift(5,B) + 1722862861;
	}
	else {
		A = SHA1CircularShift(5,B) + 1722862861 + W[1];
	}

	if (length < 8) {
		temp = SHA1CircularShift(5,A) + ((((1506887872) ^ (2079550178)) & (B)) ^ (2079550178)) + H2 + K0;
	}
	else {
		temp = SHA1CircularShift(5,A) + (((572662306) & (B)) ^ (2079550178)) + H2 + K0 + W[2];
	}
	C = SHA1CircularShift(30,B);  //SHA1CircularShift(30,(2679412915 + W[0]));
	B = A;
	A = temp;
	
	if (length < 12) {
		temp = SHA1CircularShift(5,A) + ((((C) ^ (1506887872)) & (B)) ^ (1506887872)) + 2079550178 + K0;
	}
	else {
		temp = SHA1CircularShift(5,A) + ((((C) ^ (1506887872)) & (B)) ^ (1506887872)) + 2079550178 + K0 + W[3];
	}
	E = 1506887872;
	D = C;
	C = SHA1CircularShift(30,B);
	B = A;
	A = temp;

	if (length < 16) {
		temp = SHA1CircularShift(5,A) + F_00_19(B,C,D) + 1506887872 + K0;
	}
	else {
		temp = SHA1CircularShift(5,A) + F_00_19(B,C,D) + 1506887872 + K0 + W[4];
	}
	E = D;
	D = C;
	C = SHA1CircularShift(30,B);
	B = A;
	A = temp;

	ROTATE1_NULL_5_14;

	ROTATE1_NEW( A, B, C, D, E, W_15 );
	ROTATE1_NEW( E, A, B, C, D, W[16] );
	ROTATE1_NEW( D, E, A, B, C, W[17] );
	ROTATE1_NEW( C, D, E, A, B, W[18] );
	ROTATE1_NEW( B, C, D, E, A, W[19] );
		
	for(t = 20; t < 40; t++)
	{
		if (t == 21 && length < 8) {
			ROTATE2_W((length<<5)); // *32
		}
		else {
			ROTATE2(t);
		}
	}

	for(t = 40; t < 60; t++)
	{
		ROTATE3(t);
	}
	
	for(t = 60; t < 80; t++)
	{
		ROTATE4(t);
	}
	
	Intermediate_Hash[0] += A;
	Intermediate_Hash[1] += B;
	Intermediate_Hash[2] += C;
	Intermediate_Hash[3] += D;
	Intermediate_Hash[4] += E;

	Endian_Reverse32(Intermediate_Hash[0]);
	Endian_Reverse32(Intermediate_Hash[1]);
	Endian_Reverse32(Intermediate_Hash[2]);
	Endian_Reverse32(Intermediate_Hash[3]);
	Endian_Reverse32(Intermediate_Hash[4]);

	memcpy(pDigest, Intermediate_Hash, 20);
}
