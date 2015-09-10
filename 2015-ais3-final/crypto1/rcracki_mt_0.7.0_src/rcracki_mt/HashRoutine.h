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

#ifndef _HASHROUTINE_H
#define _HASHROUTINE_H

#include "global.h"

#include <string>
#include <vector>

typedef void (*HASHROUTINE)(unsigned char* pPlain, int nPlainLen, unsigned char* pHash);

class CHashRoutine  
{
public:
	CHashRoutine();
	virtual ~CHashRoutine();

	//XXX change both to private again and make function for it
	std::vector<std::string> vHashRoutineName;
	//-1 means unlimited
	std::vector<int> vMaxPlainLen;
private:
	std::vector<HASHROUTINE> vHashRoutine;
        std::vector<int> vHashLen;
	void AddHashRoutine( std::string sHashRoutineName, HASHROUTINE pHashRoutine, int nHashLen, int nMaxPlainLen);

public:
	std::string GetAllHashRoutineName();
	//std::vector<std::string> GetAllHashRoutineNameV();
	void GetHashRoutine( std::string sHashRoutineName, HASHROUTINE& pHashRoutine, int& nHashLen );
};

#endif
