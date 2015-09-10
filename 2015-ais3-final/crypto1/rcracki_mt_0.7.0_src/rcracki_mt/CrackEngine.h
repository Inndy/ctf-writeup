/*
 * rcracki_mt is a multithreaded implementation and fork of the original 
 * RainbowCrack
 *
 * Copyright (C) Zhu Shuanglei <shuanglei@hotmail.com>
 * Copyright Martin Westergaard Jørgensen <martinwj2005@gmail.com>
 * Copyright 2009, 2010 Daniël Niggebrugge <niggebrugge@fox-it.com>
 * Copyright 2009, 2010, 2011, 2012 James Nobis <quel@quelrod.net>
 * Copyright 2010 uroskn
 * Copyright 2011 Jan Kyska
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

#ifndef _CRACKENGINE_H
#define _CRACKENGINE_H

#include "Public.h"
#include "HashSet.h"
#include "ChainWalkContext.h"
#include "MemoryPool.h"
#include "ChainWalkSet.h"
#include "rcrackiThread.h"
#ifdef _WIN32
#include <conio.h>
#include <windows.h>
#endif
#include <pthread.h>

#pragma pack(1)
union RTIrcrackiIndexChain
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

// you can't use pack(0) - it makes VC++ angry

class CCrackEngine
{
public:
	CCrackEngine();
	virtual ~CCrackEngine();

private:
	CChainWalkSet m_cws;
	int maxThreads;
	uint64_t maxMem;
	bool writeOutput;
	bool resumeSession;
	std::string outputFile;
	std::string sSessionPathName;
	std::string sProgressPathName;
	std::string sPrecalcPathName;
	//string sPrecalcIndexPathName;
	bool debug;
	bool keepPrecalcFiles;
	int gpu;

	// Statistics
	float m_fTotalDiskAccessTime;
	float m_fTotalCryptanalysisTime;
	float m_fTotalPrecalculationTime;
	uint64_t m_nTotalChainWalkStep;
	int m_nTotalFalseAlarm;
	uint64_t m_nTotalChainWalkStepDueToFalseAlarm;
	FILE *m_fChains;

private:
	void ResetStatistics();
	RainbowChain *BinarySearch(RainbowChain *pChain, int nChainCountRead, uint64_t nIndex, RTIrcrackiIndexChain *pIndex, int nIndexSize, int nIndexStart);
	int BinarySearchOld(RainbowChainO* pChain, int nRainbowChainCount, uint64_t nIndex);
	void GetChainIndexRangeWithSameEndpoint(RainbowChainO* pChain,
										    int nRainbowChainCount,
										    int nChainIndex,
										    int& nChainIndexFrom,
										    int& nChainIndexTo);
	void SearchTableChunk(RainbowChain* pChain, int nRainbowChainLen, int nRainbowChainCount, CHashSet& hs, RTIrcrackiIndexChain *pIndex, int nIndexSize, int nChainStart);
	void SearchTableChunkOld(RainbowChainO* pChain, int nRainbowChainLen, int nRainbowChainCount, CHashSet& hs);
	//bool CheckAlarm(RainbowChain* pChain, int nGuessedPos, unsigned char* pHash, CHashSet& hs);
	//bool CheckAlarmOld(RainbowChainO* pChain, int nGuessedPos, unsigned char* pHash, CHashSet& hs);

public:
	void SearchRainbowTable(std::string pathName, CHashSet& hs);
	void Run(std::vector<std::string> vPathName, CHashSet& hs, int i_maxThreads, uint64_t i_maxMem, bool resume, bool bDebug, int gpu = 0);
	float GetStatTotalDiskAccessTime();
	float GetStatTotalCryptanalysisTime();
	float GetStatTotalPrecalculationTime();
	uint64_t GetStatTotalChainWalkStep();
	uint64_t GetStatTotalFalseAlarm();
	uint64_t GetStatTotalChainWalkStepDueToFalseAlarm();
	void setOutputFile(std::string pathName);
	void setSession(std::string sSessionPathName, std::string sProgressPathName, std::string sPrecalcPathName, bool keepPrecalc);
};

#endif
