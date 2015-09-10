/*
 * rcracki_mt is a multithreaded implementation and fork of the original 
 * RainbowCrack
 *
 * Copyright (C) Zhu Shuanglei <shuanglei@hotmail.com>
 * Copyright Martin Westergaard Jørgensen <martinwj2005@gmail.com>
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

#ifndef _HASHSET_H
#define _HASHSET_H

#include "Public.h"

class CHashSet
{
public:
	CHashSet();
	virtual ~CHashSet();

private:
	std::vector<std::string> m_vHash;
	std::vector<bool>   m_vFound;
	std::vector<std::string> m_vPlain;
	std::vector<std::string> m_vBinary;

public:
	void AddHash(std::string sHash);		// lowercase, len % 2 == 0, MIN_HASH_LEN * 2 <= len <= MAX_HASH_LEN * 2
	bool AnyhashLeft();
	bool AnyHashLeftWithLen(int nLen);
	void GetLeftHashWithLen(std::vector<std::string>& vHash, int nLen);
	
	void SetPlain(std::string sHash, std::string sPlain, std::string sBinary);
	bool GetPlain(std::string sHash, std::string& sPlain, std::string& sBinary);

	int GetStatHashFound();
	int GetStatHashTotal();

	std::string GetHashInfo(int i);
	void AddHashInfo(std::string sHash, bool found, std::string sPlain, std::string sBinary);
};

#endif
