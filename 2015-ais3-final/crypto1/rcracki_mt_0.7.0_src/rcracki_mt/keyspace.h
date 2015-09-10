/*
 * This file is part of Advanced RTGen.
 * Copyright (C) 2011 Steve Thomas <steve AT tobtu DOT com>
 * Copyright 2011, 2012 James Nobis <quel@quelrod.net>
 * 
 * Advanced RTGen is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * Advanced RTGen is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with Advanced RTGen.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef KEY_SPACE_H
#define KEY_SPACE_H

#include "global.h"
//#include "common.h"
#include "architecture.h"

union uint16u
{
	uint16_t u16;
	uint8_t  u8[2];
	struct
	{
#ifdef ARC_LITTLE_ENDIAN
		uint8_t lo8, hi8;
#else
		uint8_t hi8, lo8;
#endif
	};
};

union uint32u
{
	uint32_t u32;
	uint16_t u16[2];
	uint8_t u8[4];
	struct
	{
#ifdef ARC_LITTLE_ENDIAN
		uint16u lo16, hi16;
#else
		uint16u hi16, lo16;
#endif
	};
};

union uint64u
{
	uint64_t u64;
	uint32_t u32[2];
	uint16_t u16[4];
	uint8_t u8[8];
	struct
	{
#ifdef ARC_LITTLE_ENDIAN
		uint32u lo32, hi32;
#else
		uint32u hi32, lo32;
#endif
	};
};
#ifdef _WIN32
	#define WIN_ALIGN_16 __declspec(align(16))
	#define ALIGN_16
#else
	#define WIN_ALIGN_16
	#define ALIGN_16 __attribute__((aligned(16)))
#endif

#if defined(_M_X64) || defined(__x86_64__) || defined(_M_IX86) || defined(__i386__)
	#if defined(_WIN32) && !defined(__GNUC__)
		#define DIV_MOD_64_RET_32_32(n64,d32,r32,q32) \
			{ \
				__asm mov eax,[DWORD PTR n64] \
				__asm mov edx,[DWORD PTR n64+4] \
				__asm div d32 \
				__asm mov q32,eax \
				__asm mov r32,edx \
			}
	#else
		#define DIV_MOD_64_RET_32_32(n64,d32,r32,q32) \
			asm( \
				"divl %2" \
				: "=a"(q32), "=d"(r32) \
				: "rm"(d32), "a"(((uint64u*)&n64)->lo32), "d"(((uint64u*)&n64)->hi32) );
	#endif
#else
	#define DIV_MOD_64_RET_32_32(n64,d32,r32,q32) \
		r32 = (uint32_t) (n64 % d32); \
		q32 = (uint32_t) (n64 / d32);
#endif

struct vector4
{
	WIN_ALIGN_16
	uint32_t data[4] ALIGN_16;
};

enum charaterEncoding
{
	CE_ASCII_BIN8     = 1,
	CE_UTF8           = 2,
	CE_UTF16_LE       = 4,
	CE_UTF16_BE       = 8,
	CE_UCS2_LE        = 4,
	CE_UCS2_BE        = 8,
	CE_UTF32_LE       = 16,
	CE_UTF32_BE       = 32,
	CE_NULL_PADDED    = 64,
	CE_80_NULL_PADDED = 128
};

#include <list>
#include <string>

class KeySpace
{
public:
	KeySpace(const KeySpace &ks);
	virtual ~KeySpace();

	static KeySpace *load(const char *fileName, uint32_t charaterEncoding);
	static KeySpace *load(std::list<std::list<std::string> > subKeySpaces, uint32_t charaterEncoding);
	KeySpace& operator=(const KeySpace &ks);

	bool reset(uint64_t pwNum);
	uint32_t next(vector4 *pwOut, uint32_t numPws, uint32_t vecSize);
	uint32_t get(vector4 *pwOut, uint32_t numPws, uint32_t vecSize, uint64_t *pwNums);
	uint32_t get(char *pwOut, uint64_t pwNum);
	const uint64_t getKeySpace();
	const uint32_t getMaxPwLen();
	const uint32_t getMinPwLen();
	const std::string getName();

protected:
	KeySpace() {}
	static KeySpace *init(std::list<std::list<std::string> > subKeySpaces, uint32_t charaterEncoding);
	void cleanUp();
	void copyObject(const KeySpace &ks);
	inline const bool reset(uint64_t pwNum, uint32_t *pwOut, uint32_t *charSetPoses, uint32_t &curSubKeySpace);
	bool cycleLastChar();

	uint64_t m_keySpace;
	uint64_t *m_keySpaceBoundaries;
	uint32_t *m_passwordLengths, *m_divShortCut, *m_charSetPoses, **m_charSetLengths, *m_pw, *m_pwTemp, *m_charSetPosesTemp;
	int *m_numShortCutChars;
	char *m_charSet, ***m_subKeySpaces;
	uint32_t m_numSubKeySpaces, m_curSubKeySpace, m_maxPwLen, m_minPwLen;
	uint32_t m_charaterEncoding, m_offset, m_pwBlocks;
	std::string m_name;
};

#endif
