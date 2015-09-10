/*
 * rcracki_mt is a multithreaded implementation and fork of the original 
 * RainbowCrack
 *
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

#ifndef _LM2NTLM_H
#define _LM2NTLM_H

#include "Public.h"

#include <stdio.h>
#include <string>
#include <map>
#ifdef _WIN32
	#include <conio.h>
	#include <time.h>
	#include <windows.h>
#endif
#include "signal.h"
#include "fast_md4.h"

class LM2NTLMcorrector
{
public:
	LM2NTLMcorrector();

private:
	std::map<unsigned char, std::map<int, unsigned char> > m_mapChar;
	uint64_t progressCurrentCombination;
	uint64_t totalCurrentCombination;
	uint64_t counterOverall;
	unsigned char NTLMHash[16];
	timeval tvStart, tvEnd, tvFinal;
	int countCombinations;
	int countTotalCombinations;
	int counter;
	clock_t previousClock;
	unsigned char currentCharmap[16][128];
	bool aborting;
	std::string sBinary;

private:
	bool checkNTLMPassword(unsigned char* pLMPassword, int nLMPasswordLen, std::string& sNTLMPassword);
	bool startCorrecting(std::string sLMPassword, std::string& sNTLMPassword, unsigned char* pLMPassword);
	void printString(unsigned char* muteThis, int length);
	void setupCombinationAtPositions(int length, unsigned char* pMuteMe, unsigned char* pTempMute, int* jAtPos, bool* fullAtPos, int* sizeAtPos);
	bool checkPermutations(int length, unsigned char* pTempMute, int* jAtPos, int* sizeAtPos, unsigned char* pLMPassword, std::string& sNTLMPassword);

	int calculateTotalCombinations(int length, int setSize);
	uint64_t factorial (int num);

	bool parseHexPassword(std::string hexPassword, std::string& sPlain);
	bool NormalizeHexString(std::string& sHash);
	void ParseHash(std::string sHash, unsigned char* pHash, int& nHashLen);
	std::string ByteToStr(const unsigned char* pData, int nLen);
	void addToMapW(unsigned char key, unsigned char value1, unsigned char value2);
	void fillMapW();
	void checkAbort();
	void writeEndStats();
public:
	bool LMPasswordCorrectUnicode(std::string hexPassword, unsigned char* NTLMHash, std::string& sNTLMPassword);
	std::string getBinary();
};

#endif
