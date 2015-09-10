/*
 * freerainbowtables is a project for generating, distributing, and using
 * perfect rainbow tables
 *
 * Copyright (C) Zhu Shuanglei <shuanglei@hotmail.com>
 * Copyright 2010 Martin Westergaard Jørgensen <martinwj2005@gmail.com>
 * Copyright 2009, 2010 Daniël Niggebrugge <niggebrugge@fox-it.com>
 * Copyright 2009, 2010, 2011, 2012 James Nobis <quel@quelrod.net>
 * Copyright 2010 uroskn
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

#ifndef _MEMORYPOOL_H
#define _MEMORYPOOL_H

#include "Public.h"

class CMemoryPool  
{
public:
	CMemoryPool();
	CMemoryPool(unsigned int bytesSaved, bool bDebug, uint64_t maxMem);
	virtual ~CMemoryPool();

private:
	bool debug;
	unsigned char* m_pMem;
	uint64_t m_nMemSize;

	uint64_t m_nMemMax;

public:
	unsigned char* Allocate(unsigned int nFileLen, uint64_t& nAllocatedSize);
};

#endif
