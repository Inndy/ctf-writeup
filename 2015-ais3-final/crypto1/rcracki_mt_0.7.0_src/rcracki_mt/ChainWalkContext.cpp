/*
 * freerainbowtables is a multithreaded implementation and fork of the original 
 * RainbowCrack
 *
 * Copyright (C) Zhu Shuanglei <shuanglei@hotmail.com>
 * Copyright Martin Westergaard Jørgensen <martinwj2005@gmail.com>
 * Copyright 2009, 2010 Daniël Niggebrugge <niggebrugge@fox-it.com>
 * Copyright 2009, 2010, 2011, 2012 James Nobis <quel@quelrod.net>
 * Copyright 2010 Yngve AAdlandsvik
 * Copyright 2008, 2009, 2010, 2011 Steve Thomas (Sc00bz)
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

#include "ChainWalkContext.h"

#include <ctype.h>

//////////////////////////////////////////////////////////////////////

std::string CChainWalkContext::m_sHashRoutineName;
HASHROUTINE CChainWalkContext::m_pHashRoutine;
int CChainWalkContext::m_nHashLen;
uint8_t CChainWalkContext::RTfileFormatId;
int CChainWalkContext::m_nPlainLenMinTotal = 0;
int CChainWalkContext::m_nPlainLenMaxTotal = 0;
int CChainWalkContext::m_nHybridCharset = 0;
std::vector<stCharset> CChainWalkContext::m_vCharset;
uint64_t CChainWalkContext::m_nPlainSpaceUpToX[MAX_PLAIN_LEN];
uint64_t CChainWalkContext::m_nPlainSpaceTotal;
unsigned char CChainWalkContext::m_Salt[MAX_SALT_LEN];
int CChainWalkContext::m_nSaltLen = 0;
int CChainWalkContext::m_nRainbowTableIndex;
uint64_t CChainWalkContext::m_nReduceOffset;
KeySpace *CChainWalkContext::m_ks = NULL;

//////////////////////////////////////////////////////////////////////

CChainWalkContext::CChainWalkContext()
{
}

CChainWalkContext::~CChainWalkContext()
{
}

bool CChainWalkContext::LoadCharset( std::string sName )
{
	m_vCharset.clear();
	if (sName == "byte")
	{
		stCharset tCharset;
		int i;
		for (i = 0x00; i <= 0xff; i++)
			tCharset.m_PlainCharset[i] = (unsigned char) i;
		tCharset.m_nPlainCharsetLen = MAX_PLAIN_LEN;
		tCharset.m_sPlainCharsetName = sName;
		tCharset.m_sPlainCharsetContent = "0x00, 0x01, ... 0xff";
		m_vCharset.push_back(tCharset);
		return true;
	}
	if(sName.substr(0, 6) == "hybrid") // Hybrid charset consisting of 2 charsets
	{
		if (sName.substr(6, 2) == "3(")
		{
			size_t pos = sName.find_last_of(')');
			if (pos == std::string::npos)
				return false;
			m_ks = KeySpace::load(sName.substr(8, pos - 8).c_str(), CE_ASCII_BIN8 | CE_NULL_PADDED); // hybrid3(file name)#0-0
			if (m_ks == NULL)
				return false;
			m_nHybridCharset = 3;
			return true;
		}
		else if (sName.substr(6, 2) == "2(")
			m_nHybridCharset = 2;
		else if (sName.substr(6, 1) == "(")
			m_nHybridCharset = 1;		
		else
		{
			printf("hybrid version not supported\n");
			return false;
		}
	}
	else
		m_nHybridCharset = 0;
	
	bool readCharset = false;
	std::vector<std::string> vLine;
	std::vector<std::string> internal_charset;

	vLine.clear();
	internal_charset.clear();
	internal_charset.push_back("byte                        = []");
	internal_charset.push_back("alpha                       = [ABCDEFGHIJKLMNOPQRSTUVWXYZ]");
	internal_charset.push_back("alpha-space                 = [ABCDEFGHIJKLMNOPQRSTUVWXYZ ]");
	internal_charset.push_back("alpha-numeric               = [ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789]");
	internal_charset.push_back("alpha-numeric-space         = [ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789 ]");
	internal_charset.push_back("alpha-numeric-symbol14      = [ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789\x21\x40\x23\x24\x25\x5E\x26\x2A\x28\x29\x2D\x5F\x2B\x3D]");
	internal_charset.push_back("alpha-numeric-symbol14-space= [ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789\x21\x40\x23\x24\x25\x5E\x26\x2A\x28\x29\x2D\x5F\x2B\x3D\x20]");
	internal_charset.push_back("all                         = [ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789\x21\x40\x23\x24\x25\x5E\x26\x2A\x28\x29\x2D\x5F\x2B\x3D\x7E\x60\x5B\x5D\x7B\x7D\x7C\x5C\x3A\x3B\x22\x27\x3C\x3E\x2C\x2E\x3F\x2F]");
	internal_charset.push_back("all-space                   = [ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789\x21\x40\x23\x24\x25\x5E\x26\x2A\x28\x29\x2D\x5F\x2B\x3D\x7E\x60\x5B\x5D\x7B\x7D\x7C\x5C\x3A\x3B\x22\x27\x3C\x3E\x2C\x2E\x3F\x2F\x20]");
	internal_charset.push_back("alpha-numeric-symbol32-space = [ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789\x21\x40\x23\x24\x25\x5E\x26\x2A\x28\x29\x2D\x5F\x2B\x3D\x7E\x60\x5B\x5D\x7B\x7D\x7C\x5C\x3A\x3B\x22\x27\x3C\x3E\x2C\x2E\x3F\x2F\x20]");
	internal_charset.push_back("lm-frt-cp437                = [\x20\x21\x22\x23\x24\x25\x26\x27\x28\x29\x2A\x2B\x2C\x2D\x2E\x2F\x30\x31\x32\x33\x34\x35\x36\x37\x38\x39\x3A\x3B\x3C\x3D\x3E\x3F\x40\x41\x42\x43\x44\x45\x46\x47\x48\x49\x4A\x4B\x4C\x4D\x4E\x4F\x50\x51\x52\x53\x54\x55\x56\x57\x58\x59\x5A\x5B\x5C\x5D\x5E\x5F\x60\x7B\x7C\x7D\x7E\x80\x8E\x8F\x90\x92\x99\x9A\x9B\x9C\x9D\x9E\x9F\xA5\xE0\xE1\xE2\xE3\xE4\xE6\xE7\xE8\xE9\xEA\xEB\xEE]");
	internal_charset.push_back("lm-frt-cp850                = [\x20\x21\x22\x23\x24\x25\x26\x27\x28\x29\x2A\x2B\x2C\x2D\x2E\x2F\x30\x31\x32\x33\x34\x35\x36\x37\x38\x39\x3A\x3B\x3C\x3D\x3E\x3F\x40\x41\x42\x43\x44\x45\x46\x47\x48\x49\x4A\x4B\x4C\x4D\x4E\x4F\x50\x51\x52\x53\x54\x55\x56\x57\x58\x59\x5A\x5B\x5C\x5D\x5E\x5F\x60\x7B\x7C\x7D\x7E\x80\x8E\x8F\x90\x92\x99\x9A\x9C\x9D\x9F\xA5\xB5\xB6\xB7\xBD\xBE\xC7\xCF\xD1\xD2\xD3\xD4\xD5\xD6\xD7\xD8\xDE\xE0\xE1\xE2\xE3\xE5\xE6\xE8\xE9\xEA\xEB\xED\xEF]");
	internal_charset.push_back("lm-frt-cp437-850            = [\x20\x21\x22\x23\x24\x25\x26\x27\x28\x29\x2A\x2B\x2C\x2D\x2E\x2F\x30\x31\x32\x33\x34\x35\x36\x37\x38\x39\x3A\x3B\x3C\x3D\x3E\x3F\x40\x41\x42\x43\x44\x45\x46\x47\x48\x49\x4A\x4B\x4C\x4D\x4E\x4F\x50\x51\x52\x53\x54\x55\x56\x57\x58\x59\x5A\x5B\x5C\x5D\x5E\x5F\x60\x7B\x7C\x7D\x7E\x80\x8E\x8F\x90\x92\x99\x9A\x9B\x9C\x9D\x9E\x9F\xA5\xB5\xB6\xB7\xBD\xBE\xC7\xCF\xD1\xD2\xD3\xD4\xD5\xD6\xD7\xD8\xDE\xE0\xE1\xE2\xE3\xE4\xE5\xE6\xE7\xE8\xE9\xEA\xEB\xED\xEE\xEF]");
	internal_charset.push_back("numeric                     = [0123456789]");
	internal_charset.push_back("numeric-space               = [0123456789 ]");
	internal_charset.push_back("loweralpha                  = [abcdefghijklmnopqrstuvwxyz]");
	internal_charset.push_back("loweralpha-space            = [abcdefghijklmnopqrstuvwxyz ]");
	internal_charset.push_back("loweralpha-numeric          = [abcdefghijklmnopqrstuvwxyz0123456789]");
	internal_charset.push_back("loweralpha-numeric-space    = [abcdefghijklmnopqrstuvwxyz0123456789 ]");
	internal_charset.push_back("loweralpha-numeric-symbol14 = [abcdefghijklmnopqrstuvwxyz0123456789\x21\x40\x23\x24\x25\x5E\x26\x2A\x28\x29\x2D\x5F\x2B\x3D]");
	internal_charset.push_back("loweralpha-numeric-all      = [abcdefghijklmnopqrstuvwxyz0123456789\x21\x40\x23\x24\x25\x5E\x26\x2A\x28\x29\x2D\x5F\x2B\x3D\x7E\x60\x5B\x5D\x7B\x7D\x7C\x5C\x3A\x3B\x22\x27\x3C\x3E\x2C\x2E\x3F\x2F]");
	internal_charset.push_back("loweralpha-numeric-symbol32-space= [abcdefghijklmnopqrstuvwxyz0123456789\x21\x40\x23\x24\x25\x5E\x26\x2A\x28\x29\x2D\x5F\x2B\x3D\x7E\x60\x5B\x5D\x7B\x7D\x7C\x5C\x3A\x3B\x22\x27\x3C\x3E\x2C\x2E\x3F\x2F\x20]");
	internal_charset.push_back("mixalpha                    = [abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ]");
	internal_charset.push_back("mixalpha-space              = [abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ ]");
	internal_charset.push_back("mixalpha-numeric            = [abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789]");
	internal_charset.push_back("mixalpha-numeric-space      = [abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789 ]");
	internal_charset.push_back("mixalpha-numeric-symbol14   = [abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789\x21\x40\x23\x24\x25\x5E\x26\x2A\x28\x29\x2D\x5F\x2B\x3D]");
	internal_charset.push_back("mixalpha-numeric-all        = [abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789\x21\x40\x23\x24\x25\x5E\x26\x2A\x28\x29\x2D\x5F\x2B\x3D\x7E\x60\x5B\x5D\x7B\x7D\x7C\x5C\x3A\x3B\x22\x27\x3C\x3E\x2C\x2E\x3F\x2F]");
	internal_charset.push_back("mixalpha-numeric-symbol32-space  = [abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789\x21\x40\x23\x24\x25\x5E\x26\x2A\x28\x29\x2D\x5F\x2B\x3D\x7E\x60\x5B\x5D\x7B\x7D\x7C\x5C\x3A\x3B\x22\x27\x3C\x3E\x2C\x2E\x3F\x2F\x20]");
	internal_charset.push_back("mixalpha-numeric-all-space  = [abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789\x21\x40\x23\x24\x25\x5E\x26\x2A\x28\x29\x2D\x5F\x2B\x3D\x7E\x60\x5B\x5D\x7B\x7D\x7C\x5C\x3A\x3B\x22\x27\x3C\x3E\x2C\x2E\x3F\x2F\x20]");
#ifdef USE_INTEGRATED_CHARSET
	vLine = internal_charset;
	readCharset = true;
#else
	#ifdef BOINC
		if ( boinc_ReadLinesFromFile( "charset.txt", vLine ) )
			readCharset = true;
		else
		{
			vLine = internal_charset;
			readCharset = true;
		}
	#else
		if ( ReadLinesFromFile("charset.txt", vLine) )
			readCharset = true;
		else if ( ReadLinesFromFile(GetApplicationPath() + "charset.txt", vLine) )
			readCharset = true;
		else
		{
			vLine = internal_charset;
			readCharset = true;
		}
	#endif
#endif

	if ( readCharset )
	{
		int i;
		for (i = 0; (uint32_t)i < vLine.size(); i++)
		{
			// Filter comment
			if (vLine[i][0] == '#')
				continue;

			std::vector<std::string> vPart;
			if (SeperateString(vLine[i], "=", vPart))
			{
				// sCharsetName
				std::string sCharsetName = TrimString(vPart[0]);
				if (sCharsetName == "")
					continue;
								
				// sCharsetName charset check
				bool fCharsetNameCheckPass = true;
				uint32_t j;
				for (j = 0; j < sCharsetName.size(); j++)
				{
					if (   !isalpha(sCharsetName[j])
						&& !isdigit(sCharsetName[j])
						&& (sCharsetName[j] != '-'))
					{
						fCharsetNameCheckPass = false;
						break;
					}
				}
				if (!fCharsetNameCheckPass)
				{
					printf("invalid charset name %s in charset configuration file\n", sCharsetName.c_str());
					continue;
				}

				// sCharsetContent
				std::string sCharsetContent = TrimString(vPart[1]);
				if (sCharsetContent == "" || sCharsetContent == "[]")
					continue;
				if (sCharsetContent[0] != '[' || sCharsetContent[sCharsetContent.size() - 1] != ']')
				{
					printf("invalid charset content %s in charset configuration file\n", sCharsetContent.c_str());
					continue;
				}
				sCharsetContent = sCharsetContent.substr(1, sCharsetContent.size() - 2);
				if (sCharsetContent.size() > MAX_PLAIN_LEN)
				{
					printf("charset content %s too long\n", sCharsetContent.c_str());
					continue;
				}

				//printf("%s = [%s]\n", sCharsetName.c_str(), sCharsetContent.c_str());

				// Is it a hybrid?
				if( m_nHybridCharset != 0 )
				{
					std::vector<tCharset> vCharsets;

					if ( !GetHybridCharsets(sName, vCharsets) )
					{
						std::cout << "Failed to GetHybridCharsets: "
							<< sName << std::endl;
						return false;
					}

					if(sCharsetName == vCharsets[m_vCharset.size()].sName)
					{
						stCharset tCharset;
						tCharset.m_nPlainCharsetLen = sCharsetContent.size();							
						memcpy(tCharset.m_PlainCharset, sCharsetContent.c_str(), tCharset.m_nPlainCharsetLen);
						tCharset.m_sPlainCharsetName = sCharsetName;
						tCharset.m_sPlainCharsetContent = sCharsetContent;	
						tCharset.m_nPlainLenMin = vCharsets[m_vCharset.size()].nPlainLenMin;
						tCharset.m_nPlainLenMax = vCharsets[m_vCharset.size()].nPlainLenMax;
						m_vCharset.push_back(tCharset);
						if(vCharsets.size() == m_vCharset.size())
							return true;
						//i = 0; // Start the lookup over again for the next charset
						// Sc00bz indicates this fixes a bug of skipping line 1
						// of charset.txt
						i = -1; // Start the lookup over again for the next charset
					}						
				}
				else if (sCharsetName == sName)
				{
					stCharset tCharset;
					tCharset.m_nPlainCharsetLen = sCharsetContent.size();							
					memcpy(tCharset.m_PlainCharset, sCharsetContent.c_str(), tCharset.m_nPlainCharsetLen);
					tCharset.m_sPlainCharsetName = sCharsetName;
					tCharset.m_sPlainCharsetContent = sCharsetContent;							
					m_vCharset.push_back(tCharset);
					return true;
				}
			}
		}
		printf("charset %s not found in charset.txt\n", sName.c_str());
	}
	else
		printf("can't open charset configuration file\n");

	return false;
}

//////////////////////////////////////////////////////////////////////

bool CChainWalkContext::SetHashRoutine( std::string sHashRoutineName )
{
	CHashRoutine hr;
	hr.GetHashRoutine(sHashRoutineName, m_pHashRoutine, m_nHashLen);
	if (m_pHashRoutine != NULL)
	{
		m_sHashRoutineName = sHashRoutineName;
		return true;
	}
	else
		return false;
}

bool CChainWalkContext::SetPlainCharset( std::string sCharsetName, int nPlainLenMin, int nPlainLenMax)
{
	// m_PlainCharset, m_nPlainCharsetLen, m_sPlainCharsetName, m_sPlainCharsetContent
	if (!LoadCharset(sCharsetName))
		return false;

	if(m_nHybridCharset == 3) // hybrid3 is already loaded into m_ks
	{
		m_nPlainSpaceTotal = m_ks->getKeySpace();
		return true;
	}
	if(m_nHybridCharset == 0) // Not hybrid charset
	{
		// m_nPlainLenMin, m_nPlainLenMax
		if (nPlainLenMin < 1 || nPlainLenMax > MAX_PLAIN_LEN || nPlainLenMin > nPlainLenMax)
		{
			printf("invalid plaintext length range: %d - %d\n", nPlainLenMin, nPlainLenMax);
			return false;
		}
		m_vCharset[0].m_nPlainLenMin = nPlainLenMin;
		m_vCharset[0].m_nPlainLenMax = nPlainLenMax;
	}
	// m_nPlainSpaceUpToX
	m_nPlainSpaceUpToX[0] = 0;
	m_nPlainLenMaxTotal = 0;
	m_nPlainLenMinTotal = 0;
	m_nPlainSpaceTotal = 0;
	uint64_t nTemp = 1;
	uint32_t j, k = 1;
	int i = 1;
	for(j = 0; j < m_vCharset.size(); j++)
	{
		m_nPlainLenMaxTotal += m_vCharset[j].m_nPlainLenMax;
		m_nPlainLenMinTotal += m_vCharset[j].m_nPlainLenMin;
		m_vCharset[j].m_nPlainSpaceUpToX[0] = 0;
		uint64_t nTemp2 = 1;
			
		for (i = 1; i <= m_vCharset[j].m_nPlainLenMax; i++)
		{			
			nTemp *= m_vCharset[j].m_nPlainCharsetLen;
			nTemp2 *= m_vCharset[j].m_nPlainCharsetLen;

			if (i < m_vCharset[j].m_nPlainLenMin)
			{
				m_nPlainSpaceUpToX[k] = 0;
				m_vCharset[j].m_nPlainSpaceUpToX[i] = 0;
			}
			else
			{
				// special case of the last keyspace in a hybrid2 set
				if ( i == 1 && j > 1 && m_vCharset[j].m_nPlainCharsetLen != 1
					&& j == m_vCharset.size() - 1 )
				{
					m_nPlainSpaceUpToX[k] = nTemp;
				}
				else
				{
					m_nPlainSpaceUpToX[k] = m_nPlainSpaceUpToX[k - 1] + nTemp;
				}

				m_vCharset[j].m_nPlainSpaceUpToX[i] = m_vCharset[j].m_nPlainSpaceUpToX[i - 1] + nTemp2;
			}

			k++;
		}

		m_vCharset[j].m_nPlainSpaceTotal = m_vCharset[j].m_nPlainSpaceUpToX[i-1];

		if ( m_nPlainSpaceTotal == 0 )
			m_nPlainSpaceTotal = m_vCharset[j].m_nPlainSpaceTotal;
		else
			m_nPlainSpaceTotal *= m_vCharset[j].m_nPlainSpaceTotal;
	}

	return true;
}

bool CChainWalkContext::SetRainbowTableIndex(int nRainbowTableIndex)
{
	if (nRainbowTableIndex < 0)
		return false;
	m_nRainbowTableIndex = nRainbowTableIndex;
	m_nReduceOffset = 65536 * nRainbowTableIndex;

	return true;
}

bool CChainWalkContext::SetSalt(unsigned char *Salt, int nSaltLength)
{
	memcpy(&m_Salt[0], Salt, nSaltLength);
	
	m_nSaltLen = nSaltLength;
//	m_sSalt = sSalt;
	return true;
}

bool CChainWalkContext::SetupWithPathName( std::string sPathName, int& nRainbowChainLen, int& nRainbowChainCount)
{
	// something like lm_alpha#1-7_0_100x16_test.rt

#ifdef _WIN32
	std::string::size_type nIndex = sPathName.find_last_of('\\');
#else
	std::string::size_type nIndex = sPathName.find_last_of('/');
#endif
	if (nIndex != std::string::npos)
		sPathName = sPathName.substr(nIndex + 1);

	if (sPathName.size() < 3)
	{
		printf("%s is not a rainbow table\n", sPathName.c_str());
		return false;
	}
	if (sPathName.substr(sPathName.size() - 5) == ".rti2")
	{
		RTfileFormatId = getRTfileFormatId( "RTI2" );
	}
	else if (sPathName.substr(sPathName.size() - 4) == ".rti")
	{
		RTfileFormatId = getRTfileFormatId( "RTI" );
	}
	else if (sPathName.substr(sPathName.size() - 3) == ".rt")
	{
		RTfileFormatId = getRTfileFormatId( "RT" );
	}
	else
	{
		printf("%s is not a rainbow table\n", sPathName.c_str());
		return false;
	}

	// Parse
	std::vector<std::string> vPart;
	if (!SeperateString(sPathName, "___x_", vPart))
	{
		printf("filename %s not identified\n", sPathName.c_str());
		return false;
	}

	std::string sHashRoutineName   = vPart[0];
	int nRainbowTableIndex    = atoi(vPart[2].c_str());
	nRainbowChainLen          = atoi(vPart[3].c_str());
	nRainbowChainCount        = atoi(vPart[4].c_str());

	// Parse charset definition
	std::string sCharsetDefinition = vPart[1];
	std::string sCharsetName;
	int nPlainLenMin = 0, nPlainLenMax = 0;		

//	printf("Charset: %s", sCharsetDefinition.c_str());
	
	if(sCharsetDefinition.substr(0, 6) == "hybrid") // Hybrid table
	{
		sCharsetName = sCharsetDefinition;
	}
	else
	{
		if ( sCharsetDefinition.find('#') == std::string::npos )		// For backward compatibility, "#1-7" is implied
		{			
			sCharsetName = sCharsetDefinition;
			nPlainLenMin = 1;
			nPlainLenMax = 7;
		}
		else
		{
			std::vector<std::string> vCharsetDefinitionPart;
			if (!SeperateString(sCharsetDefinition, "#-", vCharsetDefinitionPart))
			{
				printf("filename %s not identified\n", sPathName.c_str());
				return false;	
			}
			else
			{
				sCharsetName = vCharsetDefinitionPart[0];
				nPlainLenMin = atoi(vCharsetDefinitionPart[1].c_str());
				nPlainLenMax = atoi(vCharsetDefinitionPart[2].c_str());
			}
		}
	}
	// Setup
	if (!SetHashRoutine(sHashRoutineName))
	{
		printf("hash routine %s not supported\n", sHashRoutineName.c_str());
		return false;
	}
	if (!SetPlainCharset(sCharsetName, nPlainLenMin, nPlainLenMax))
		return false;
	if (!SetRainbowTableIndex(nRainbowTableIndex))
	{
		printf("invalid rainbow table index %d\n", nRainbowTableIndex);
		return false;
	}

	return true;
}

std::string CChainWalkContext::GetHashRoutineName()
{
	return m_sHashRoutineName;
}

int CChainWalkContext::GetHashLen()
{
	return m_nHashLen;
}

std::string CChainWalkContext::GetPlainCharsetName()
{
	/*
	 * XXX dumb hack to return the hybrid character set name
	 * in rcracki_mt the call to CChainWalkContext::SetupWithPathname is static
	 * can't modify a static std::string there
	 */
	if ( m_nHybridCharset != 0 )
	{
		std::string characterSet = "hybrid";

		if ( m_nHybridCharset == 3 )
		{
			characterSet += "3(";
			characterSet += m_ks->getName();
		}
		else
		{
			if ( m_nHybridCharset == 2 )
				characterSet += "2";

			characterSet += "(";

			for ( unsigned int i = 0; i < m_vCharset.size(); i++ )
			{
				char tempCharsetMerge[255];
				sprintf( tempCharsetMerge, "%s#%d-%d,"
					, m_vCharset[i].m_sPlainCharsetName.c_str()
					, m_vCharset[i].m_nPlainLenMin, m_vCharset[i].m_nPlainLenMax );

				characterSet += tempCharsetMerge;
			}

			// removing trailing ,
			characterSet = characterSet.substr( 0, characterSet.length() - 1 );
		}
		characterSet += ")";

		return characterSet;
	}

	return m_vCharset[0].m_sPlainCharsetName;
}

std::string CChainWalkContext::GetPlainCharsetContent()
{
	return m_vCharset[0].m_sPlainCharsetContent;
}

int CChainWalkContext::GetPlainLenMin()
{
	if ( m_nHybridCharset != 0 )
		return 0;

	return m_vCharset[0].m_nPlainLenMin;
}

int CChainWalkContext::GetPlainLenMax()
{
	if ( m_nHybridCharset != 0 )
		return 0;

	return m_vCharset[0].m_nPlainLenMax;
}

uint64_t CChainWalkContext::GetPlainSpaceTotal()
{
	return m_nPlainSpaceTotal;
}

int CChainWalkContext::GetRainbowTableIndex()
{
	return m_nRainbowTableIndex;
}

void CChainWalkContext::Dump()
{
	printf("hash routine: %s\n", m_sHashRoutineName.c_str());
	printf("hash length: %d\n", m_nHashLen);

	for ( uint32_t i = 0; i < m_vCharset.size(); i++ )
	{
		printf( "m_vCharset[%d].m_nPlainCharSetLen: %d\n", i, m_vCharset[i].m_nPlainCharsetLen );

		printf("plain charset: ");
		
		for ( uint32_t j = 0; j < m_vCharset[i].m_nPlainCharsetLen; j++ )
		{
			if (isprint(m_vCharset[i].m_PlainCharset[j]))
				printf("%c", m_vCharset[i].m_PlainCharset[j]);
			else
				printf("?");
		}
		printf("\n");

		for ( int j = 0; j <= m_vCharset[i].m_nPlainLenMax; j++ )
		{
			printf( "m_vCharset[%d].m_nPlainSpaceUpToX[%d]: %"PRIu64"\n"
				, i, j, m_vCharset[i].m_nPlainSpaceUpToX[j] );
		}
		
		printf("plain charset in hex: ");

		for ( uint32_t j = 0; j < m_vCharset[i].m_nPlainCharsetLen; j++ )
			printf("%02x ", m_vCharset[i].m_PlainCharset[j]);
		printf("\n");

		printf("plain length range: %d - %d\n", m_vCharset[i].m_nPlainLenMin, m_vCharset[i].m_nPlainLenMax);
		printf("plain charset name: %s\n", m_vCharset[i].m_sPlainCharsetName.c_str());
		printf("plain subkey space total: %s\n", uint64tostr(m_vCharset[i].m_nPlainSpaceTotal).c_str());
	}
		
	for ( int i = 0; i <= m_nPlainLenMaxTotal; i++ )
	{
		printf( "m_nPlainSpaceUpToX[%d]: %"PRIu64"\n"
			, i, m_nPlainSpaceUpToX[i] );
	}

	//printf("plain charset content: %s\n", m_sPlainCharsetContent.c_str());
	//for (i = 0; i <= m_nPlainLenMax; i++)
	//	printf("plain space up to %d: %s\n", i, uint64tostr(m_nPlainSpaceUpToX[i]).c_str());
	printf("plain space total: %s\n", uint64tostr(m_nPlainSpaceTotal).c_str());

	printf("rainbow table index: %d\n", m_nRainbowTableIndex);
	printf("reduce offset: %s\n", uint64tostr(m_nReduceOffset).c_str());
	printf("\n");
}

void CChainWalkContext::SetIndex(uint64_t nIndex)
{
	m_nIndex = nIndex;
}

void CChainWalkContext::SetHash(unsigned char* pHash)
{
	memcpy(m_Hash, pHash, m_nHashLen);
}

int CChainWalkContext::normalIndexToPlain(uint64_t index, uint64_t *plainSpaceUpToX, unsigned char *charSet, int charSetLen, int min, int max, unsigned char *plain)
{
	int a;

	for ( a = max - 1; a >= min; a-- )
	{
		if ( index >= plainSpaceUpToX[a])
			break;
	}

	uint32_t plainLen = a + 1;

	index -= plainSpaceUpToX[a]; // plainLen - 1 == a

	for ( a = plainLen - 1; a >= 0; a-- )
	{
		// XXX this is optimized for 32-bit platforms
#if defined(_WIN32) && !defined(__GNUC__)
		if (index < 0x100000000I64)
			break;
#else
		if (index < 0x100000000llu)
			break;
#endif
		plain[a] = charSet[index % charSetLen];
		index /= charSetLen;
	}

	unsigned int index32 = (unsigned int) index;
	for ( ; a >= 0; a-- )
	{
		// remarks from Sc00bz
		// Note the lack of assembly code.
		// Assembly code is not needed since all the variables are in the stack.
		// If you add in assembly code it will be slower than the compiler's code.

		plain[a] = charSet[index32 % charSetLen];
		index32 /= charSetLen;
	}

	return plainLen;
}

void CChainWalkContext::IndexToPlain()
{
	if (m_nHybridCharset == 3)
	{
		m_nPlainLen = m_ks->get((char*) m_Plain, m_nIndex);
	}
	else
	{
		m_nPlainLen = 0;
		uint64_t indexTmp = m_nIndex;

		uint32_t numKeySpaces = m_vCharset.size();

		for ( uint32_t a = 0; a < numKeySpaces - 1; a++ )
		{
			m_vCharset[a].m_nIndexX = indexTmp % m_vCharset[a].m_nPlainSpaceTotal;
			indexTmp /= m_vCharset[a].m_nPlainSpaceTotal;
			m_nPlainLen += normalIndexToPlain(m_vCharset[a].m_nIndexX, m_vCharset[a].m_nPlainSpaceUpToX, m_vCharset[a].m_PlainCharset, m_vCharset[a].m_nPlainCharsetLen, m_vCharset[a].m_nPlainLenMin, m_vCharset[a].m_nPlainLenMax, m_Plain + m_nPlainLen);
		}

		m_vCharset[numKeySpaces-1].m_nIndexX = indexTmp;
		m_nPlainLen += normalIndexToPlain(m_vCharset[numKeySpaces-1].m_nIndexX, m_vCharset[numKeySpaces-1].m_nPlainSpaceUpToX, m_vCharset[numKeySpaces-1].m_PlainCharset, m_vCharset[numKeySpaces-1].m_nPlainCharsetLen, m_vCharset[numKeySpaces-1].m_nPlainLenMin, m_vCharset[numKeySpaces-1].m_nPlainLenMax, m_Plain + m_nPlainLen);
	}
}


/*
void CChainWalkContext::IndexToPlain()
{
	int i;
	m_nPlainLen = 0;
	for (i = m_nPlainLenMaxTotal - 1; i >= m_nPlainLenMinTotal - 1; i--)
	{
		if (m_nIndex >= m_nPlainSpaceUpToX[i])
		{
			m_nPlainLen = i + 1;
			break;
		}
	}
	if(m_nPlainLen == 0)
		m_nPlainLen = m_nPlainLenMinTotal;
	uint64_t nIndexOfX = m_nIndex - m_nPlainSpaceUpToX[m_nPlainLen - 1];

// this is the generic code for non x86/x86_64 platforms
#if !defined(_M_X64) && !defined(_M_IX86) && !defined(__i386__) && !defined(__x86_64__)
	
	// generic version (slow for non 64-bit platforms and gcc < 4.5.x)
	for (i = m_nPlainLen - 1; i >= 0; i--)
	{
		int nCharsetLen = 0;
		for(uint32_t j = 0; j < m_vCharset.size(); j++)
		{
			nCharsetLen += m_vCharset[j].m_nPlainLenMax;
			if(i < nCharsetLen) // We found the correct charset
			{
				m_Plain[i] = m_vCharset[j].m_PlainCharset[nIndexOfX % m_vCharset[j].m_nPlainCharsetLen];
				nIndexOfX /= m_vCharset[j].m_nPlainCharsetLen;
				break;
			}
		}
	}

#elif defined(_M_X64) || defined(_M_IX86) || defined(__i386__) || defined(__x86_64__)

	// Fast ia32 version
	for (i = m_nPlainLen - 1; i >= 0; i--)
	{
		// 0x100000000 = 2^32
#ifdef _M_IX86
		if (nIndexOfX < 0x100000000I64)
			break;
#else
		if (nIndexOfX < 0x100000000llu)
			break;
#endif

		int nCharsetLen = 0;
		for(uint32_t j = 0; j < m_vCharset.size(); j++)
		{
			nCharsetLen += m_vCharset[j].m_nPlainLenMax;
			if(i < nCharsetLen) // We found the correct charset
			{
				m_Plain[i] = m_vCharset[j].m_PlainCharset[nIndexOfX % m_vCharset[j].m_nPlainCharsetLen];
				nIndexOfX /= m_vCharset[j].m_nPlainCharsetLen;
				break;
			}
		}
	}

	uint32_t nIndexOfX32 = (uint32_t)nIndexOfX;
	for (; i >= 0; i--)
	{
		int nCharsetLen = 0;
		for(uint32_t j = 0; j < m_vCharset.size(); j++)
		{
			nCharsetLen += m_vCharset[j].m_nPlainLenMax;
			if(i < nCharsetLen) // We found the correct charset
			{

				m_Plain[i] = m_vCharset[j].m_PlainCharset[nIndexOfX % m_vCharset[j].m_nPlainCharsetLen];
				nIndexOfX /= m_vCharset[j].m_nPlainCharsetLen;
				break;
			}
		}
	}

	uint32_t nIndexOfX32 = (uint32_t)nIndexOfX;
	for (; i >= 0; i--)
	{
		int nCharsetLen = 0;
		for(uint32_t j = 0; j < m_vCharset.size(); j++)
		{
			nCharsetLen += m_vCharset[j].m_nPlainLenMax;
			if(i < nCharsetLen) // We found the correct charset
			{

//		m_Plain[i] = m_vCharset[j].m_PlainCharset[nIndexOfX32 % m_vCharset[j].m_nPlainCharsetLen];
//		nIndexOfX32 /= m_vCharset[j].m_nPlainCharsetLen;

//	moving nPlainCharsetLen into the asm body and avoiding the extra temp
//	variable results in a performance gain
//				unsigned int nPlainCharsetLen = m_vCharset[j].m_nPlainCharsetLen;
				unsigned int nTemp;

#if defined(_WIN32) && !defined(__GNUC__)
		// VC++ still needs this
		unsigned int nPlainCharsetLen = m_vCharset[j].m_nPlainCharsetLen;

		__asm
		{
			mov eax, nIndexOfX32
			xor edx, edx
			div nPlainCharsetLen
			mov nIndexOfX32, eax
			mov nTemp, edx
		}
		m_Plain[i] = m_vCharset[j].m_PlainCharset[nTemp];
#else
		__asm__ __volatile__ ("xor %%edx, %%edx;"
								"divl %3;"
								: "=a"(nIndexOfX32), "=d"(nTemp)
								: "a"(nIndexOfX32), "rm"(m_vCharset[j].m_nPlainCharsetLen)
								: );
		m_Plain[i] = m_vCharset[j].m_PlainCharset[nTemp];
#endif
		break;
			}
		}
	}
#endif
}
*/

void CChainWalkContext::PlainToHash()
{	
	m_pHashRoutine(m_Plain, m_nPlainLen, m_Hash);
}

void CChainWalkContext::HashToIndex(int nPos)
{
	// breaks strict aliasing
	//m_nIndex = (*(uint64_t*)m_Hash + m_nReduceOffset + nPos) % m_nPlainSpaceTotal;

	memcpy( m_Hash_alias.m_Hash, m_Hash, 8 );

	m_nIndex = (m_Hash_alias.alias + m_nReduceOffset + nPos) % m_nPlainSpaceTotal;
}

uint64_t CChainWalkContext::GetIndex()
{
	return m_nIndex;
}
const uint64_t *CChainWalkContext::GetIndexPtr()
{
	return &m_nIndex;
}

std::string CChainWalkContext::GetPlain()
{
	std::string sRet;
	int i;
	for (i = 0; i < m_nPlainLen; i++)
	{
		char c = m_Plain[i];
		if (c >= 32 && c <= 126)
			sRet += c;
		else
			sRet += '?';
	}
	
	return sRet;
}

std::string CChainWalkContext::GetBinary()
{
	return HexToStr(m_Plain, m_nPlainLen);
}

std::string CChainWalkContext::GetHash()
{
	return HexToStr(m_Hash, m_nHashLen);
}

bool CChainWalkContext::CheckHash(unsigned char* pHash)
{
	if (memcmp(m_Hash, pHash, m_nHashLen) == 0)
		return true;

	return false;
}

uint8_t CChainWalkContext::getRTfileFormat()
{
	return RTfileFormatId;
}
