/*
 * freerainbowtables is a project for generating, distributing, and using
 * perfect rainbow tables
 *
 * Copyright (C) Zhu Shuanglei <shuanglei@hotmail.com>
 * Copyright Martin Westergaard Jørgensen <martinwj2005@gmail.com>
 * Copyright 2009, 2010 Daniël Niggebrugge <niggebrugge@fox-it.com>
 * Copyright 2009, 2010, 2011, 2012 James Nobis <quel@quelrod.net>
 * Copyright 2011 Janosch Rux <janosch.rux@web.de>
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

#include "HashRoutine.h"
#include "HashAlgorithm.h"


//////////////////////////////////////////////////////////////////////

CHashRoutine::CHashRoutine()
{
	// Notice: MIN_HASH_LEN <= nHashLen <= MAX_HASH_LEN


	AddHashRoutine("lm",   HashLM,   8, 14);
	AddHashRoutine("ntlm", HashNTLM, 16, 15);
//	AddHashRoutine("md2",  HashMD2,  16);
	AddHashRoutine("md4",  HashMD4,  16, 31);
	AddHashRoutine("md5",  HashMD5,  16, -1);
	AddHashRoutine("doublemd5",  HashDoubleMD5,  16, -1);
	AddHashRoutine("sha1", HashSHA1, 20, -1);
//	AddHashRoutine("ripemd160", HashRIPEMD160, 20);
	AddHashRoutine("mysql323", HashMySQL323, 8, -1);
	AddHashRoutine("mysqlsha1", HashMySQLSHA1, 20, -1);
//	AddHashRoutine("ciscopix", HashPIX, 16);
//	AddHashRoutine("mscache", HashMSCACHE, 16);
	AddHashRoutine("halflmchall", HashHALFLMCHALL, 8, -1);

	// Added from mao
	AddHashRoutine("lmchall", HashLMCHALL, 24, -1);
	AddHashRoutine("ntlmchall", HashNTLMCHALL, 24, -1);
//	AddHashRoutine("oracle", HashORACLE, 8, -1);
}

CHashRoutine::~CHashRoutine()
{
}

void CHashRoutine::AddHashRoutine( std::string sHashRoutineName, HASHROUTINE pHashRoutine, int nHashLen, int nMaxPlainLen )
{
	vHashRoutineName.push_back(sHashRoutineName);
	vHashRoutine.push_back(pHashRoutine);
	vHashLen.push_back(nHashLen);
	vMaxPlainLen.push_back(nMaxPlainLen);
}

std::string CHashRoutine::GetAllHashRoutineName()
{
	std::string sRet;
	uint32_t i;
	for (i = 0; i < vHashRoutineName.size(); i++)
		sRet += vHashRoutineName[i] + " ";

	return sRet;
}
/*
std::vector<std::string> CHashRoutine::GetAllHashRoutineNameV()
{

	return vHashRoutineName;
}
*/
void CHashRoutine::GetHashRoutine( std::string sHashRoutineName, HASHROUTINE& pHashRoutine, int& nHashLen )
{
	uint32_t i;
	for (i = 0; i < vHashRoutineName.size(); i++)
	{
		if (sHashRoutineName == vHashRoutineName[i])
		{
			pHashRoutine = vHashRoutine[i];
			nHashLen = vHashLen[i];
			return;
		}
	}

	pHashRoutine = NULL;
	nHashLen = 0;
}
