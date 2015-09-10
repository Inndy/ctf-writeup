/*
 * freerainbowtables is a project for generating, distributing, and using
 * perfect rainbow tables
 *
 * Copyright (C) Zhu Shuanglei <shuanglei@hotmail.com>
 * Copyright Martin Westergaard Jørgensen <martinwj2005@gmail.com>
 * Copyright 2009, 2010 Daniël Niggebrugge <niggebrugge@fox-it.com>
 * Copyright 2009, 2010, 2011, 2012 James Nobis <quel@quelrod.net>
 * Copyright 2011 Logan Watt <logan.watt@gmail.com>
 *
 * This file is part of freerainbowtables.
 *
 * freerainbowtables is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, version 2 of the License.
 *
 * freerainbowtables is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with freerainbowtables.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef _PUBLIC_H
#define _PUBLIC_H

#define __STDC_FORMAT_MACROS /* for PRI macros */
#define __STDC_LIMIT_MACROS /* for uint64_t, UINT64_MAX, etc. */

#include "global.h"

#include <stdio.h>
#include <stdint.h>
#if defined(_WIN32) && !defined(__GNUC__)
	#include "inttypes.h"
#else
	#include "/usr/include/inttypes.h"
#endif
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <sys/types.h>
#include <sys/stat.h>

#include <string>
#include <vector>
#include <list>
#include <iterator>
#include <iostream>
#include <fstream>
#include <map>
#include <algorithm>

#if defined(_WIN32) && !defined(__GNUC__)
	#include <io.h>
#endif

struct RainbowChainO
{
	uint64_t nIndexS;
	uint64_t nIndexE;
};

#if defined(_WIN32) && !defined(__GNUC__)
	#pragma warning(disable: 4201)
#endif
union RainbowChain
{
	uint64_t nIndexS;
	struct
	{
		unsigned short foo[3];
		unsigned short nIndexE;
	};
	//int nCheckPoint;
};
#if defined(_WIN32) && !defined(__GNUC__)
	#pragma warning(default : 4201) 
#endif

struct RainbowChainCP
{
	uint64_t startpt;
	uint64_t endpt;
	unsigned short checkpts;
};

struct IndexChain
{
	uint64_t nPrefix;
	uint32_t nFirstChain;
	uint32_t nChainCount;
};

/* rcracki_mt IndexChain
#pragma pack(1)
union IndexChain
{
	uint64_t nPrefix; //5
	struct
	{
		unsigned char foo[5];
		unsigned int nFirstChain; //4
		unsigned short nChainCount; //2
	};
	//unsigned short nChainCount; (maybe union with nPrefix, 1 byte spoiled, no pack(1) needed)
};
#pragma pack()
*/

struct IndexRow
{
	uint64_t prefix;
	unsigned int prefixstart, numchains;
};

typedef struct
{
	std::string sName;
	int nPlainLenMin;
	int nPlainLenMax;
} tCharset;

#define MAX_PLAIN_LEN 256
#define MIN_HASH_LEN  8
#define MAX_HASH_LEN  256
#define MAX_SALT_LEN  256

typedef struct 
{
	uint64_t m_nPlainSpaceUpToX[MAX_PLAIN_LEN];		// Performance consideration
	//unsigned char m_PlainCharset[255];
	unsigned char m_PlainCharset[MAX_PLAIN_LEN];
	uint64_t m_nPlainSpaceTotal;
	uint64_t m_nIndexX;
	unsigned int m_nPlainCharsetLen;
	int m_nPlainLenMin;
	int m_nPlainLenMax;
	std::string m_sPlainCharsetName;
	std::string m_sPlainCharsetContent;
} stCharset;

// XXX nmap is GPL2, will check newer releases regarding license
// Code comes from nmap, used for the linux implementation of kbhit()
#ifndef _WIN32
#include <unistd.h>
#include <termios.h>
#include <fcntl.h>

int tty_getchar();
void tty_done();
void tty_init();
void tty_flush(void);
// end nmap code

#endif

#if defined(_WIN32) && !defined(__GNUC__)
	int gettimeofday( struct timeval *tv, struct timezone *tz );
#else
	#include <sys/time.h>
#endif

timeval sub_timeofday( timeval tv2, timeval tv );

long GetFileLen(FILE* file);
long GetFileLen(char* file);
long GetFileLen( std::string file );
uint8_t getRTfileFormatId( std::string RTfileFormatName );
std::string TrimString( std::string s );
bool boinc_ReadLinesFromFile( std::string sPathName, std::vector<std::string>& vLine );
bool ReadLinesFromFile( std::string sPathName, std::vector<std::string>& vLine);
bool SeperateString( std::string s, std::string sSeperator, std::vector<std::string>& vPart);
std::string uint64tostr(uint64_t n);
std::string uint64tohexstr(uint64_t n);
std::string HexToBinary( const char* data, int len );
std::string HexToStr(const unsigned char* pData, int nLen);
unsigned long GetAvailPhysMemorySize();
std::string GetApplicationPath();
void ParseHash( std::string sHash, unsigned char* pHash, int& nHashLen );
bool GetHybridCharsets( std::string sCharset, std::vector<tCharset>& vCharset );
void Logo();
bool writeResultLineToFile( std::string sOutputFile, std::string sHash, std::string sPlain, std::string sBinary );

#endif
