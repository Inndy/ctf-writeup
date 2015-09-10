/*
 * This file is part of Advanced RTGen.
 * Copyright (C) 2011 Steve Thomas <steve AT tobtu DOT com>
 * Copyright 2011, 2012 James Nobis <quel@quelrod.net>
 * 
 * Advanced RTGen is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * Advanced RTGen is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with Advanced RTGen.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "Public.h"
#include "keyspace.h"
#include <string.h>
#include <stdlib.h>
#include <vector>
#include <fstream>
#include <sstream>
#include <string>
#include <map>
#include <list>

#ifndef UINT64_C
	#ifdef _WIN32
		#define UINT64_C(c)      (c)
	#else
		#define UINT64_C(c)      c##ull
	#endif
#endif

KeySpace::KeySpace(const KeySpace &ks)
{
	copyObject(ks);
}

KeySpace *KeySpace::init(std::list<std::list<std::string> > subKeySpaces, uint32_t charaterEncoding)
{
	uint64_t keySpace, keySpaceBoundary = 0;
	KeySpace *ks = new KeySpace();
	std::list<std::string> uniqueCharSets;
	std::list<std::string>::iterator uniqueCharSetsIt;
	std::list<std::string>::reverse_iterator charSetItRev;
	std::list<std::string>::iterator charSetIt;
	std::list<std::list<std::string> >::iterator subKeySpaceIt;
	uint32_t a, b, charSetLen = 0, maxPwLen = 1, minPwLen = 0xffffffff;
	int charSetDiffLen;

	ks->m_curSubKeySpace = 0;
	ks->m_numSubKeySpaces = subKeySpaces.size();
	ks->m_charaterEncoding = charaterEncoding;
	ks->m_offset = 0;
	if ((charaterEncoding & CE_UTF16_BE) != 0)
	{
		ks->m_offset = 1;
	}
	else if ((charaterEncoding & CE_UTF32_BE) != 0)
	{
		ks->m_offset = 3;
	}
	ks->m_keySpaceBoundaries = new uint64_t [ks->m_numSubKeySpaces + 1];
	ks->m_passwordLengths    = new uint32_t [ks->m_numSubKeySpaces];
	ks->m_divShortCut        = new uint32_t [ks->m_numSubKeySpaces];
	ks->m_numShortCutChars   = new int      [ks->m_numSubKeySpaces];
	ks->m_charSetLengths     = new uint32_t*[ks->m_numSubKeySpaces];
	ks->m_subKeySpaces       = new char**   [ks->m_numSubKeySpaces];

	ks->m_keySpaceBoundaries[0] = 0;
	for (a = 0, subKeySpaceIt = subKeySpaces.begin(); subKeySpaceIt != subKeySpaces.end(); ++subKeySpaceIt, a++)
	{
		if (maxPwLen < subKeySpaceIt->size())
		{
			maxPwLen = subKeySpaceIt->size();
		}
		if (minPwLen > subKeySpaceIt->size())
		{
			minPwLen = subKeySpaceIt->size();
		}
		keySpace = 1;
		ks->m_passwordLengths[a] = subKeySpaceIt->size();
		ks->m_charSetLengths[a] = new uint32_t[subKeySpaceIt->size()];
		for (b = subKeySpaceIt->size() - 1, charSetItRev = subKeySpaceIt->rbegin(); charSetItRev != subKeySpaceIt->rend(); ++charSetItRev, b--)
		{
			// Calculate key space
			keySpace *= charSetItRev->length();
			if (keySpace < UINT64_C(0x100000000))
			{
				ks->m_divShortCut[a] = (uint32_t) keySpace;
				ks->m_numShortCutChars[a] = subKeySpaceIt->size() - b;
			}

			// Find unique character sets
			ks->m_charSetLengths[a][b] = charSetItRev->length();
			for (uniqueCharSetsIt = uniqueCharSets.begin(); uniqueCharSetsIt != uniqueCharSets.end(); ++uniqueCharSetsIt)
			{
				charSetDiffLen = ks->m_charSetLengths[a][b] - uniqueCharSetsIt->length();
				if (charSetDiffLen > 0)
				{
					if (charSetItRev->find(*uniqueCharSetsIt) != std::string::npos)
					{
						*uniqueCharSetsIt = *charSetItRev;
						charSetLen += charSetDiffLen;
						break;
					}
				}
				else if (charSetDiffLen < 0)
				{
					if (uniqueCharSetsIt->find(*charSetItRev) != std::string::npos)
					{
						break;
					}
				}
				else if (*uniqueCharSetsIt == *charSetItRev)
				{
					break;
				}
			}
			if (uniqueCharSetsIt == uniqueCharSets.end())
			{
				uniqueCharSets.push_back(charSetItRev->c_str());
				charSetLen += charSetItRev->length();
			}
		}
		if (keySpace == ks->m_divShortCut[a])
		{
			ks->m_numShortCutChars[a] = 0;
		}
		else if (keySpace / ks->m_divShortCut[a] >= UINT64_C(0x100000000))
		{
			ks->m_numShortCutChars[a] = -ks->m_numShortCutChars[a];
		}
		keySpaceBoundary += keySpace;
		ks->m_keySpaceBoundaries[a + 1] = keySpaceBoundary;
	}
	ks->m_keySpace = keySpaceBoundary;
	ks->m_maxPwLen = maxPwLen;
	ks->m_minPwLen = minPwLen;
	ks->m_charSetPoses = new uint32_t[maxPwLen];
	ks->m_charSetPosesTemp = new uint32_t[maxPwLen];
	for (a = 0; a < maxPwLen; a++)
	{
		ks->m_charSetPoses[0] = 0;
	}
	if ((ks->m_charaterEncoding & CE_80_NULL_PADDED) != 0)
	{
		maxPwLen++;
	}
	if ((ks->m_charaterEncoding & (CE_UTF16_LE | CE_UTF16_BE)) != 0)
	{
		maxPwLen *= 2;
	}
	else if ((ks->m_charaterEncoding & (CE_UTF32_LE | CE_UTF32_BE)) != 0)
	{
		maxPwLen *= 4;
	}
	else if ((ks->m_charaterEncoding & CE_ASCII_BIN8) == 0)
	{
		fprintf(stderr, "Charater encoding not supported\n");
		delete ks;
		return NULL;
	}
	maxPwLen = (maxPwLen + 3) / 4;
	ks->m_pwBlocks = maxPwLen;
	ks->m_pw = new uint32_t[maxPwLen + 1];
	ks->m_pwTemp = new uint32_t[maxPwLen + 1];

	ks->m_charSet = new char[charSetLen + 1];
	ks->m_charSet[charSetLen] = 0;
	a = 0;
	for (uniqueCharSetsIt = uniqueCharSets.begin(); uniqueCharSetsIt != uniqueCharSets.end(); ++uniqueCharSetsIt)
	{
		charSetLen = uniqueCharSetsIt->length();
		memcpy(ks->m_charSet + a, uniqueCharSetsIt->c_str(), charSetLen);
		a += charSetLen;
	}
	uniqueCharSets.clear();
	for (a = 0, subKeySpaceIt = subKeySpaces.begin(); subKeySpaceIt != subKeySpaces.end(); ++subKeySpaceIt, a++)
	{
		ks->m_subKeySpaces[a] = new char*[subKeySpaceIt->size()];
		for (b = 0, charSetIt = subKeySpaceIt->begin(); charSetIt != subKeySpaceIt->end(); ++charSetIt, b++)
		{
			ks->m_subKeySpaces[a][b] = strstr(ks->m_charSet, charSetIt->c_str());
		}
	}

	ks->m_name = "inline";
	ks->reset(0);
	return ks;
}

KeySpace::~KeySpace()
{
	cleanUp();
}

KeySpace *KeySpace::load(const char *fileName, uint32_t charaterEncoding)
{
	std::vector<std::string> lines;
	std::ifstream fin(fileName, std::ifstream::in);
	std::string line, name, charSet;
	std::map<std::string,std::string> charSets;
	std::map<std::string,std::string>::iterator charSetMapIt;
	std::list<std::string> subKeySpace;
	std::list<std::list<std::string> > subKeySpaces;
	std::list<std::string>::iterator charSetIt;
	std::list<std::list<std::string> >::iterator subKeySpaceIt;
	size_t pos, pos2;
	uint32_t len, lineNumber = 0;
	bool multiCharNames = false;
	char ch[2] = {0, 0};

#ifdef BOINC
	if (!boinc_ReadLinesFromFile(fileName, lines))
#else
	if (!ReadLinesFromFile(fileName, lines))
#endif
	{
		fprintf(stderr, "ERROR: Can't open key space file '%s'\n", fileName);
		return NULL;
	}

	// Read character sets
	while (1)
	{
		// Read line
		if (lineNumber >= lines.size())
		{
			// eof
			fprintf(stderr, "ERROR: '%s' is not a valid key space file\n", fileName);
			return NULL;
		}
		line = lines[lineNumber];
		lineNumber++;
		if (line.length() == 0 || line[0] == '#')
		{
			continue;
		}
		if (line[line.length() - 1] != ']')
		{
			break;
		}

		// Get character set name
		pos = line.find_first_of(" \t", 0, 2);
		if (pos == std::string::npos)
		{
			break;
		}
		name = line.substr(0, pos);

		// Get character set
		pos = line.find_first_not_of(" \t", pos, 2);
		if (pos == std::string::npos)
		{
			break; // this is not even possible... meh
		}
		if (line[pos] == '[')
		{
			charSet = line.substr(pos + 1, line.length() - pos - 2);
		}
		else if (line[pos] == 'h' && line[pos + 1] == '[')
		{
			charSet = HexToStr((unsigned char*)line.substr(pos + 2, line.length() - pos - 3).c_str(), line.length() - pos - 3);
			if (charSet.length() == 0 && line.length() - pos - 3 != 0)
			{
				fprintf(stderr, "ERROR: '%s' has an invalid character set on line #%u\n", fileName, lineNumber);
				return NULL;
			}
		}
		else
		{
			break;
		}
		if (charSet.length() == 0)
		{
			fprintf(stderr, "ERROR: '%s' has an empty character set on line #%u\n", fileName, lineNumber);
			return NULL;
		}

		// Insert into map
		if (charSets.find(name) == charSets.end())
		{
			if (name.length() != 1)
			{
				multiCharNames = true;
			}
			charSets[name] = charSet;
		}
		else
		{
			fprintf(stderr, "ERROR: '%s' has a duplicate character set name on line #%u\n", fileName, lineNumber);
			return NULL;
		}
	}
	if (line[line.length() - 1] == ']')
	{
		fprintf(stderr, "ERROR line #%u: '%s' is not a valid key space file\n", lineNumber, fileName);
		return NULL;
	}

	// Read sub key spaces
	do
	{
		if (multiCharNames)
		{
			pos = 0;
			while (pos != std::string::npos)
			{
				// Get character set name
				pos2 = line.find_first_of(" \t", pos, 2);
				if (pos2 == std::string::npos)
				{
					pos2 = line.length();
				}
				name = line.substr(pos, pos2 - pos);

				// Get character set
				charSetMapIt = charSets.find(name);
				if (charSetMapIt == charSets.end())
				{
					fprintf(stderr, "ERROR: Character set name '%s' not found on line #%u from key space file '%s'\n", name.c_str(), lineNumber, fileName);
					return NULL;
				}
				subKeySpace.push_back(charSetMapIt->second);

				if (pos2 == line.length())
				{
					break;
				}
				pos = line.find_first_not_of(" \t", pos2, 2);
			}
		}
		else
		{
			len = line.length();
			for (pos = 0; pos < len; pos++)
			{
				*ch = line[pos];
				if (*ch != ' ' && *ch != '\t')
				{
					charSetMapIt = charSets.find(ch);
					if (charSetMapIt == charSets.end())
					{
						fprintf(stderr, "ERROR: Character set name '%c' not found on line #%u from key space file '%s'\n", *ch, lineNumber, fileName);
						return NULL;
					}
					subKeySpace.push_back(charSetMapIt->second);
				}
			}
		}
		subKeySpaces.push_back(subKeySpace);
		subKeySpace.clear();

		// Read line
		do
		{
			if (lineNumber >= lines.size())
			{
				line = "";
				break; // eof
			}
			line = lines[lineNumber];
			lineNumber++;
		} while ((line.length() == 0 || line[0] == '#'));
	} while (line.length() != 0);

	KeySpace *ks = KeySpace::init(subKeySpaces, charaterEncoding);
	if (ks != NULL)
	{
		ks->m_name = fileName;
	}
	return ks;
}

KeySpace *KeySpace::load(std::list<std::list<std::string> > subKeySpaces, uint32_t charaterEncoding)
{
	return KeySpace::init(subKeySpaces, charaterEncoding);
}

KeySpace& KeySpace::operator=(const KeySpace &ks)
{
	if (this != &ks)
	{
		cleanUp();
		copyObject(ks);
	}
	return *this;
}

void KeySpace::cleanUp()
{
	uint32_t a;

	for (a = 0; a < m_numSubKeySpaces; a++)
	{
		delete [] m_subKeySpaces[a];
		delete [] m_charSetLengths[a];
	}
	delete [] m_keySpaceBoundaries;
	delete [] m_passwordLengths;
	delete [] m_divShortCut;
	delete [] m_numShortCutChars;
	delete [] m_charSetPoses;
	delete [] m_charSetPosesTemp;
	delete [] m_charSetLengths;
	delete [] m_charSet;
	delete [] m_subKeySpaces;
	delete [] m_pw;
	delete [] m_pwTemp;
}

void KeySpace::copyObject(const KeySpace &ks)
{
	uint32_t a, b, tmp, charSetLen = 1;

	m_keySpace = ks.m_keySpace;
	m_numSubKeySpaces = ks.m_numSubKeySpaces;
	m_curSubKeySpace = ks.m_curSubKeySpace;
	m_maxPwLen = ks.m_maxPwLen;
	m_minPwLen = ks.m_minPwLen;
	m_charaterEncoding = ks.m_charaterEncoding;
	m_offset = ks.m_offset;
	m_pwBlocks = ks.m_pwBlocks;
	m_name = ks.m_name;

	m_keySpaceBoundaries  = new uint64_t [m_numSubKeySpaces + 1];
	m_passwordLengths     = new uint32_t [m_numSubKeySpaces];
	m_divShortCut         = new uint32_t [m_numSubKeySpaces];
	m_numShortCutChars    = new int      [m_numSubKeySpaces];
	m_charSetLengths      = new uint32_t*[m_numSubKeySpaces];
	m_subKeySpaces        = new char**   [m_numSubKeySpaces];
	m_charSetPoses        = new uint32_t [m_maxPwLen];
	m_charSetPosesTemp    = new uint32_t [m_maxPwLen];
	m_pw                  = new uint32_t [m_pwBlocks + 1];
	m_pwTemp              = new uint32_t [m_pwBlocks + 1];
	memcpy(m_keySpaceBoundaries,  ks.m_keySpaceBoundaries,  sizeof(uint64_t) * (m_numSubKeySpaces + 1));
	memcpy(m_passwordLengths,     ks.m_passwordLengths,     sizeof(uint32_t) * m_numSubKeySpaces);
	memcpy(m_divShortCut,         ks.m_divShortCut,         sizeof(uint32_t) * m_numSubKeySpaces);
	memcpy(m_numShortCutChars,    ks.m_numShortCutChars,    sizeof(uint32_t) * m_numSubKeySpaces);
	memcpy(m_charSetPoses,        ks.m_charSetPoses,        sizeof(uint32_t) * m_maxPwLen);
	memcpy(m_pw,                  ks.m_pw,                  sizeof(uint32_t) * (m_pwBlocks + 1));
	for (a = 0; a < m_numSubKeySpaces; a++)
	{
		m_charSetLengths[a] = new uint32_t[m_passwordLengths[a]];
		memcpy(m_charSetLengths[a], ks.m_charSetLengths[a], sizeof(uint32_t) * m_passwordLengths[a]);
		for (b = 0; b < m_passwordLengths[a]; b++)
		{
			tmp = m_charSetLengths[a][b] + (ks.m_subKeySpaces[a][b] - ks.m_charSet);
			if (charSetLen < tmp)
			{
				charSetLen = tmp;
			}
		}
	}
	m_charSet = new char[charSetLen];
	memcpy(m_charSet, ks.m_charSet, sizeof(char) * charSetLen);
	for (a = 0; a < m_numSubKeySpaces; a++)
	{
		m_subKeySpaces[a] = new char*[m_passwordLengths[a]];
		for (b = 0; b < m_passwordLengths[a]; b++)
		{
			m_subKeySpaces[a][b] = m_charSet + (ks.m_subKeySpaces[a][b] - ks.m_charSet);
		}
	}
}

bool KeySpace::reset(uint64_t pwNum)
{
	return reset(pwNum, m_pw, m_charSetPoses, m_curSubKeySpace);
}

const bool KeySpace::reset(uint64_t pwNum, uint32_t *pwOut, uint32_t *charSetPoses, uint32_t &curSubKeySpace)
{
	uint32_t *charSetLengths, *pw;
	char **subKeySpaces;
	uint32_t curSubKeySpaceTemp, charSetLength, divisor, remainder, quotient, offset;
	int a, t, passwordLength, numShortCutChars;

	if (pwNum >= m_keySpace)
	{
		return true;
	}
	pw = pwOut + 1;
	offset = m_offset;
	curSubKeySpaceTemp = 0;
	while (pwNum >= m_keySpaceBoundaries[curSubKeySpaceTemp + 1])
	{
		curSubKeySpaceTemp++;
	}
	pwNum           -= m_keySpaceBoundaries[curSubKeySpaceTemp];
	passwordLength   = (int) m_passwordLengths[curSubKeySpaceTemp];
	numShortCutChars = (int) m_numShortCutChars[curSubKeySpaceTemp];
	charSetLengths   = m_charSetLengths[curSubKeySpaceTemp];
	subKeySpaces     = m_subKeySpaces[curSubKeySpaceTemp];
	curSubKeySpace   = curSubKeySpaceTemp;
	t = (int) m_pwBlocks;
	for (a = passwordLength >> 2; a < t; a++)
	{
		pw[a] = 0;
	}
	if (numShortCutChars > 0)
	{
		divisor = m_divShortCut[curSubKeySpaceTemp];
		DIV_MOD_64_RET_32_32(pwNum, divisor, remainder, quotient);

		numShortCutChars = passwordLength - numShortCutChars;
		if ((m_charaterEncoding & CE_ASCII_BIN8) != 0)
		{
			// 8 bit
			for (a = passwordLength - 1; a >= numShortCutChars; a--)
			{
				charSetLength = charSetLengths[a];
				charSetPoses[a] = t = remainder % charSetLength;
				remainder /= charSetLength;
				((char*)pw)[a] = subKeySpaces[a][t];
			}
			for (; a >= 0; a--)
			{
				charSetLength = charSetLengths[a];
				charSetPoses[a] = t = quotient % charSetLength;
				quotient /= charSetLength;
				((char*)pw)[a] = subKeySpaces[a][t];
			}
		}
		else if ((m_charaterEncoding & (CE_UTF16_LE | CE_UTF16_BE)) != 0)
		{
			// 16 bit
			for (a = passwordLength - 1; a >= numShortCutChars; a--)
			{
				charSetLength = charSetLengths[a];
				charSetPoses[a] = t = remainder % charSetLength;
				remainder /= charSetLength;
				((char*)pw)[2 * a + offset] = subKeySpaces[a][t];
			}
			for (; a >= 0; a--)
			{
				charSetLength = charSetLengths[a];
				charSetPoses[a] = t = quotient % charSetLength;
				quotient /= charSetLength;
				((char*)pw)[2 * a + offset] = subKeySpaces[a][t];
			}
		}
		else if ((m_charaterEncoding & (CE_UTF32_LE | CE_UTF32_BE)) != 0)
		{
			// 32 bit
			for (a = passwordLength - 1; a >= numShortCutChars; a--)
			{
				charSetLength = charSetLengths[a];
				charSetPoses[a] = t = remainder % charSetLength;
				remainder /= charSetLength;
				((char*)pw)[4 * a + offset] = subKeySpaces[a][t];
			}
			for (; a >= 0; a--)
			{
				charSetLength = charSetLengths[a];
				charSetPoses[a] = t = quotient % charSetLength;
				quotient /= charSetLength;
				((char*)pw)[4 * a + offset] = subKeySpaces[a][t];
			}
		}
	}
	else if (passwordLength > 1)
	{
		quotient = (uint32_t) pwNum;
		if ((m_charaterEncoding & CE_ASCII_BIN8) != 0)
		{
			// 8 bit
			for (a = passwordLength - 1; a >= 0; a--)
			{
				charSetLength = charSetLengths[a];
				charSetPoses[a] = t = quotient % charSetLength;
				quotient /= charSetLength;
				((char*)pw)[a] = subKeySpaces[a][t];
			}
		}
		else if ((m_charaterEncoding & (CE_UTF16_LE | CE_UTF16_BE)) != 0)
		{
			// 16 bit
			for (a = passwordLength - 1; a >= 0; a--)
			{
				charSetLength = charSetLengths[a];
				charSetPoses[a] = t = quotient % charSetLength;
				quotient /= charSetLength;
				((char*)pw)[2 * a + offset] = subKeySpaces[a][t];
			}
		}
		else if ((m_charaterEncoding & (CE_UTF32_LE | CE_UTF32_BE)) != 0)
		{
			// 32 bit
			for (a = passwordLength - 1; a >= 0; a--)
			{
				charSetLength = charSetLengths[a];
				charSetPoses[a] = t = quotient % charSetLength;
				quotient /= charSetLength;
				((char*)pw)[4 * a + offset] = subKeySpaces[a][t];
			}
		}
	}
	else
	{
		charSetPoses[0] = (uint32_t) pwNum;
		((char*)pw)[offset] = subKeySpaces[0][pwNum];
	}
	pwOut[0] = passwordLength;
	if ((m_charaterEncoding & CE_80_NULL_PADDED) != 0)
	{
		if (m_charaterEncoding == (CE_ASCII_BIN8 | CE_80_NULL_PADDED))
		{
			((char*)pw)[passwordLength] = (char) 0x80;
		}
		else if ((m_charaterEncoding & (CE_UTF16_LE | CE_UTF16_BE)) != 0)
		{
			((char*)pw)[2 * passwordLength + offset] = (char) 0x80;
		}
		else if ((m_charaterEncoding & (CE_UTF32_LE | CE_UTF32_BE)) != 0)
		{
			((char*)pw)[4 * passwordLength + offset] = (char) 0x80;
		}
	}
	return false;
}

uint32_t KeySpace::next(vector4 *pwOut, uint32_t numPws, uint32_t vecSize)
{
	char *charSet;
	uint32_t *pw;
	uint32_t a, b, c, curSubKeySpace, passwordLength, charSetPos, charSetLength, offset, pwBlocks;

	pw             = m_pw;
	offset         = m_offset;
	pwBlocks       = m_pwBlocks;
	curSubKeySpace = m_curSubKeySpace;
	passwordLength = m_passwordLengths[curSubKeySpace];
	charSet        = m_subKeySpaces[curSubKeySpace][passwordLength - 1];
	charSetPos     = m_charSetPoses[passwordLength - 1];
	charSetLength  = m_charSetLengths[curSubKeySpace][passwordLength - 1];
	for (a = 0; a < numPws; a++)
	{
		if ((m_charaterEncoding & CE_ASCII_BIN8) != 0)
		{
			// 8 bit
			((char*)(pw+1))[passwordLength - 1] = charSet[charSetPos];
		}
		else if ((m_charaterEncoding & (CE_UTF16_LE | CE_UTF16_BE)) != 0)
		{
			// 16 bit
			((char*)(pw+1))[2 * (passwordLength - 1) + offset] = charSet[charSetPos];
		}
		else if ((m_charaterEncoding & (CE_UTF32_LE | CE_UTF32_BE)) != 0)
		{
			// 32 bit
			((char*)(pw+1))[4 * (passwordLength - 1) + offset] = charSet[charSetPos];
		}
		for (b = 0, c = 0; b < pwBlocks; b++, c += vecSize)
		{
			pwOut[c].data[a] = pw[b];
		}
		charSetPos++;
		if (charSetPos >= charSetLength)
		{
			if (cycleLastChar())
			{
				reset(0);
			}
			charSetPos = 0;
			curSubKeySpace = m_curSubKeySpace;
			passwordLength = m_passwordLengths[curSubKeySpace];
			charSet        = m_subKeySpaces[curSubKeySpace][passwordLength - 1];
			charSetLength  = m_charSetLengths[curSubKeySpace][passwordLength - 1];
		}
	}
	m_charSetPoses[passwordLength - 1] = charSetPos;
	return a;
}

uint32_t KeySpace::get(vector4 *pwOut, uint32_t numPws, uint32_t vecSize, uint64_t *pwNums)
{
	uint32_t *pw, *charSetPoses;
	uint32_t a, b, c, curSubKeySpace = 0, pwBlocks;

	pwBlocks     = m_pwBlocks;
	pw           = m_pwTemp;
	charSetPoses = m_charSetPosesTemp; // Thrown away
	for (a = 0; a < numPws; a++)
	{
		if (reset(pwNums[a], pw, charSetPoses, curSubKeySpace))
		{
			break;
		}
		for (b = 0, c = 0; b <= pwBlocks; b++, c += vecSize)
		{
			pwOut[c].data[a] = pw[b];
		}
	}
	return a;
}

uint32_t KeySpace::get(char *pwOut, uint64_t pwNum)
{
	uint32_t *pw, *charSetPoses;
	uint32_t curSubKeySpace = 0, pwLen;

	pw           = m_pwTemp;
	charSetPoses = m_charSetPosesTemp; // Thrown away
	reset(pwNum, pw, charSetPoses, curSubKeySpace);
	pwLen = pw[0];
	pw++;
	for (uint32_t a = 0; a < pwLen; a++)
	{
		pwOut[a] = ((char*)pw)[a];
	}
	pwOut[pwLen] = 0;
	return pwLen;
}

bool KeySpace::cycleLastChar()
{
	uint32_t *charSetPoses, *pw;
	uint32_t offset, curSubKeySpace;
	int a, passwordLength;
	bool nextKeySpace = true;

	pw = m_pw + 1;
	curSubKeySpace = m_curSubKeySpace;
	passwordLength = (int) m_passwordLengths[curSubKeySpace];
	a = passwordLength - 2; // supress g++ warning of unitialized use
	offset = m_offset;
	if (passwordLength > 1)
	{
		charSetPoses = m_charSetPoses;
		if ((m_charaterEncoding & CE_ASCII_BIN8) != 0)
		{
			// 8 bit
			for (a = passwordLength - 2; a >= 0; a--)
			{
				if (++charSetPoses[a] < m_charSetLengths[curSubKeySpace][a])
				{
					((char*)pw)[a] = m_subKeySpaces[curSubKeySpace][a][charSetPoses[a]];
					break;
				}
				charSetPoses[a] = 0;
				((char*)pw)[a] = m_subKeySpaces[curSubKeySpace][a][0];
			}
		}
		else if ((m_charaterEncoding & (CE_UTF16_LE | CE_UTF16_BE)) != 0)
		{
			// 16 bit
			for (a = passwordLength - 2; a >= 0; a--)
			{
				if (++charSetPoses[a] < m_charSetLengths[curSubKeySpace][a])
				{
					((char*)pw)[2 * a + offset] = m_subKeySpaces[curSubKeySpace][a][charSetPoses[a]];
					break;
				}
				charSetPoses[a] = 0;
				((char*)pw)[2 * a + offset] = m_subKeySpaces[curSubKeySpace][a][0];
			}
		}
		else if ((m_charaterEncoding & (CE_UTF32_LE | CE_UTF32_BE)) != 0)
		{
			// 32 bit
			for (a = passwordLength - 2; a >= 0; a--)
			{
				if (++charSetPoses[a] < m_charSetLengths[curSubKeySpace][a])
				{
					((char*)pw)[4 * a + offset] = m_subKeySpaces[curSubKeySpace][a][charSetPoses[a]];
					break;
				}
				charSetPoses[a] = 0;
				((char*)pw)[4 * a + offset] = m_subKeySpaces[curSubKeySpace][a][0];
			}
		}
		nextKeySpace = a == -1;
	}
	if (nextKeySpace)
	{
		if (++curSubKeySpace >= m_numSubKeySpaces)
		{
			return true;
		}
		m_curSubKeySpace = curSubKeySpace;
		passwordLength = m_passwordLengths[curSubKeySpace];
		charSetPoses = m_charSetPoses;
		m_pw[0] = passwordLength;

		int pwBlocks = (int) m_pwBlocks;
		for (a = passwordLength >> 2; a < pwBlocks; a++)
		{
			pw[a] = 0;
		}

		if ((m_charaterEncoding & CE_ASCII_BIN8) != 0)
		{
			// 8 bit
			for (a = 0; a < passwordLength; a++)
			{
				charSetPoses[a] = 0;
				((char*)pw)[a] = m_subKeySpaces[curSubKeySpace][a][0];
			}
		}
		else if ((m_charaterEncoding & (CE_UTF16_LE | CE_UTF16_BE)) != 0)
		{
			// 16 bit
			for (a = 0; a < passwordLength; a++)
			{
				charSetPoses[a] = 0;
				((char*)pw)[2 * a + offset] = m_subKeySpaces[curSubKeySpace][a][0];
			}
		}
		else if ((m_charaterEncoding & (CE_UTF32_LE | CE_UTF32_BE)) != 0)
		{
			// 32 bit
			for (a = 0; a < passwordLength; a++)
			{
				charSetPoses[a] = 0;
				((char*)pw)[4 * a + offset] = m_subKeySpaces[curSubKeySpace][a][0];
			}
		}
		if ((m_charaterEncoding & CE_80_NULL_PADDED) != 0)
		{
			if (m_charaterEncoding == (CE_ASCII_BIN8 | CE_80_NULL_PADDED))
			{
				((char*)pw)[a] = (char) 0x80;
			}
			else if ((m_charaterEncoding & (CE_UTF16_LE | CE_UTF16_BE)) != 0)
			{
				((char*)pw)[2 * a + offset] = (char) 0x80;
			}
			else if ((m_charaterEncoding & (CE_UTF32_LE | CE_UTF32_BE)) != 0)
			{
				((char*)pw)[4 * a + offset] = (char) 0x80;
			}
		}
	}
	return false;
}

const uint64_t KeySpace::getKeySpace()
{
	return m_keySpace;
}

const uint32_t KeySpace::getMaxPwLen()
{
	return m_maxPwLen;
}

const uint32_t KeySpace::getMinPwLen()
{
	return m_minPwLen;
}

const std::string KeySpace::getName()
{
	return m_name;
}
