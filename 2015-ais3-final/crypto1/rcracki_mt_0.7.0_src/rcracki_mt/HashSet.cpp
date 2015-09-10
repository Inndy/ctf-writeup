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

#include "HashSet.h"

CHashSet::CHashSet()
{
}

CHashSet::~CHashSet()
{
}

void CHashSet::AddHash(std::string sHash)
{
	if (sHash == "aad3b435b51404ee")
		return;

	uint32_t i;
	for (i = 0; i < m_vHash.size(); i++)
	{
		if (m_vHash[i] == sHash)
			return;
	}

	//printf("debug: adding hash %s\n", sHash.c_str());

	m_vHash.push_back(sHash);
	m_vFound.push_back(false);
	m_vPlain.push_back("");
	m_vBinary.push_back("");
}

std::string CHashSet::GetHashInfo(int i)
{
	std::string found;
	if (m_vFound[i])
		found = "1";
	else
		found = "0";

	std::string buffer = m_vHash[i] + ":" + found + ":" + m_vPlain[i] + ":" + m_vBinary[i];

	return buffer;
}

bool CHashSet::AnyhashLeft()
{
	uint32_t i;
	for (i = 0; i < m_vHash.size(); i++)
	{
		if (!m_vFound[i])
			return true;
	}

	return false;
}

bool CHashSet::AnyHashLeftWithLen(int nLen)
{
	uint32_t i;
	for (i = 0; i < m_vHash.size(); i++)
	{
		if (!m_vFound[i])
			if (m_vHash[i].size() == (unsigned long)nLen * 2)
				return true;
	}

	return false;
}

void CHashSet::GetLeftHashWithLen(std::vector<std::string>& vHash, int nLen)
{
	vHash.clear();

	uint32_t i;
	for (i = 0; i < m_vHash.size(); i++)
	{
		if (!m_vFound[i])
			if (m_vHash[i].size() == (unsigned long)nLen * 2)
				vHash.push_back(m_vHash[i]);
	}
}

void CHashSet::AddHashInfo(std::string sHash, bool found, std::string sPlain, std::string sBinary)
{
	uint32_t i;
	for (i = 0; i < m_vHash.size(); i++)
	{
		if (m_vHash[i] == sHash)
			return;
	}

	m_vHash.push_back(sHash);
	m_vFound.push_back(found);
	m_vPlain.push_back(sPlain);
	m_vBinary.push_back(sBinary);
}

void CHashSet::SetPlain(std::string sHash, std::string sPlain, std::string sBinary)
{
	uint32_t i;
	for (i = 0; i < m_vHash.size(); i++)
	{
		if (m_vHash[i] == sHash)
		{
			m_vFound[i]    = true;
			m_vPlain[i]    = sPlain;
			m_vBinary[i]   = sBinary;
			return;
		}
	}
}

bool CHashSet::GetPlain(std::string sHash, std::string& sPlain, std::string& sBinary)
{
	if (sHash == "aad3b435b51404ee")
	{
		sPlain  = "";
		sBinary = "";
		return true;
	}

	uint32_t i;
	for (i = 0; i < m_vHash.size(); i++)
	{
		if (m_vHash[i] == sHash)
		{
			if (m_vFound[i])
			{
				sPlain  = m_vPlain[i];
				sBinary = m_vBinary[i];
				return true;
			}
		}
	}

	return false;
}

int CHashSet::GetStatHashFound()
{
	int nHashFound = 0;
	uint32_t i;
	for (i = 0; i < m_vHash.size(); i++)
	{
		if (m_vFound[i])
			nHashFound++;
	}

	return nHashFound;
}

int CHashSet::GetStatHashTotal()
{
	return (int) m_vHash.size();
}
