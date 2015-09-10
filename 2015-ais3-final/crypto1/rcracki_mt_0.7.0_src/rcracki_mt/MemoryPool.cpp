/*
 * freerainbowtables is a project for generating, distributing, and using
 * perfect rainbow tables
 *
 * Copyright (C) Zhu Shuanglei <shuanglei@hotmail.com>
 * Copyright Martin Westergaard Jørgensen <martinwj2005@gmail.com>
 * Copyright 2009, 2010 Daniël Niggebrugge <niggebrugge@fox-it.com>
 * Copyright 2009, 2010, 2011, 2012 James Nobis <quel@quelrod.net>
 * Copyright 2010 uroskn
 * Copyright 2011 Richard W. Watson <rwatson@therichard.com>
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

#include "MemoryPool.h"

CMemoryPool::CMemoryPool()
{
	m_pMem = NULL;
	m_nMemSize = 0;

	unsigned long nAvailPhys = GetAvailPhysMemorySize();

	if (nAvailPhys < 16 * 1024 * 1024)
		m_nMemMax = nAvailPhys / 2;					// Leave some memory for CChainWalkSet
	else
		m_nMemMax = nAvailPhys - 8 * 1024 * 1024;	// Leave some memory for CChainWalkSet	
}

CMemoryPool::CMemoryPool(unsigned int bytesSaved, bool bDebug, uint64_t maxMem)
{
	m_pMem = NULL;
	m_nMemSize = 0;
	debug = bDebug;

	unsigned long nAvailPhys = GetAvailPhysMemorySize();

	if ( debug )
	  std::cout << "Debug: nAvailPhys: " << nAvailPhys << std::endl;

	if ( maxMem > 0 && maxMem < nAvailPhys )
		nAvailPhys = maxMem;
	
	m_nMemMax = nAvailPhys - bytesSaved;	// Leave memory for CChainWalkSet	

	if (m_nMemMax < 16 * 1024 * 1024)
		m_nMemMax = 16 * 1024 * 1024;
}

CMemoryPool::~CMemoryPool()
{
	if (m_pMem != NULL)
	{
#ifdef _MEMORYDEBUG
		printf("Freeing %i bytes of memory\n", m_nMemSize);
#endif 
		delete [] m_pMem;
		m_pMem = NULL;
		m_nMemSize = 0;
	}
}

unsigned char* CMemoryPool::Allocate(unsigned int nFileLen, uint64_t& nAllocatedSize)
{
	if (nFileLen <= m_nMemSize)
	{
		nAllocatedSize = nFileLen;
		return m_pMem;
	}

	unsigned int nTargetSize;
	if (nFileLen < m_nMemMax)
		nTargetSize = nFileLen;
	else
		nTargetSize = m_nMemMax;

	// Free existing memory
	if (m_pMem != NULL)
	{
#ifdef _MEMORYDEBUG
		printf("Freeing %i bytes of memory\n", m_nMemSize);
#endif 
		delete [] m_pMem;
		m_pMem = NULL;
		m_nMemSize = 0;
	}

	// Allocate new memory
	//printf("allocating %u bytes memory\n", nTargetSize);
#ifdef _MEMORYDEBUG
		printf("Allocating %i bytes of memory - ", nTargetSize);
#endif 

	m_pMem = new (std::nothrow) unsigned char[nTargetSize];
	while (m_pMem == NULL && nTargetSize >= 32 * 1024 * 1024 )
	{
#ifdef _MEMORYDEBUG
		printf("failed!\n");
		printf("Allocating %i bytes of memory (backup) - ", nTargetSize);
#endif 
		nTargetSize -= 16 * 1024 * 1024;
		m_pMem = new (std::nothrow) unsigned char[nTargetSize];
	}

	if (m_pMem != NULL)
	{
#ifdef _MEMORYDEBUG
		printf("success!\n");
#endif
		m_nMemSize = nTargetSize;
		nAllocatedSize = nTargetSize;
		return m_pMem;
	}
	else
	{
		m_nMemSize = 0;
		nAllocatedSize = 0;
		return NULL;
	}
}
