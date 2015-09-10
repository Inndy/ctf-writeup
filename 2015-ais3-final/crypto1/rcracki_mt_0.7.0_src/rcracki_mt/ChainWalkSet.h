/*
 * freerainbowtables is a project for generating, distributing, and using
 * perfect rainbow tables
 *
 * Copyright (C) Zhu Shuanglei <shuanglei@hotmail.com>
 * Copyright Martin Westergaard Jørgensen <martinwj2005@gmail.com>
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

#ifndef _CHAINWALKSET_H
#define _CHAINWALKSET_H

#include "Public.h"

struct ChainWalk
{
	unsigned char Hash[MAX_HASH_LEN];
	//int nHashLen;		// Implied
	uint64_t* pIndexE;	// mapStartPosIndexE, Len = nRainbowChainLen - 1
};

class CChainWalkSet
{
public:
	CChainWalkSet();
	virtual ~CChainWalkSet();

private:
	std::string m_sHashRoutineName;		// Discard all if not match
	std::string m_sPlainCharsetName;		// Discard all if not match
	int    m_nPlainLenMin;			// Discard all if not match
	int    m_nPlainLenMax;			// Discard all if not match
	int    m_nRainbowTableIndex;	// Discard all if not match
	int    m_nRainbowChainLen;		// Discard all if not match
	std::list<ChainWalk> m_lChainWalk;
	bool   debug;
	std::string sPrecalcPathName;
	int    preCalcPart;
	std::vector<std::string> vPrecalcFiles;

private:
	void DiscardAll();
	bool FindInFile(uint64_t* pIndexE, unsigned char* pHash, int nHashLen);
	std::string CheckOrRotatePreCalcFile();
	void updateUsedPrecalcFiles();

public:
	uint64_t* RequestWalk( unsigned char* pHash, int nHashLen
		, std::string sHashRoutineName, std::string sPlainCharsetName
		, int nPlainLenMin, int nPlainLenMax, int nRainbowTableIndex
		, int nRainbowChainLen, bool& fNewlyGenerated, bool setDebug
		, std::string sPrecalc );
	void DiscardWalk(uint64_t* pIndexE);
	void StoreToFile(uint64_t* pIndexE, unsigned char* pHash, int nHashLen);
	void removePrecalcFiles();
};

#endif
