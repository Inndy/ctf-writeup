/*
 * freerainbowtables is a project for generating, distributing, and using
 * perfect rainbow tables
 *
 * Copyright 2011 Steve Thomas (Sc00bz)
 * Copyright 2011, 2012 James Nobis <quel@quelrod.net>
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

#ifndef _RTI2COMMON_H
#define _RTI2COMMON_H

#include "Public.h"

struct Chain
{
	uint64_t startPoint, endPoint;
//	uint64_t checkBits;
};

struct CharacterSet
{
	std::vector<uint8_t>  characterSet1;
	std::vector<uint16_t> characterSet2;
	std::vector<uint24_t> characterSet3;
	std::vector<uint32_t> characterSet4;
};

struct SubKeySpace
{
	uint8_t hybridSets;
	std::vector<uint8_t> passwordLength;
	std::vector<uint8_t> charSetFlags;
	std::vector<CharacterSet> perPositionCharacterSets;
};

struct RTI20_Header_RainbowTableParameters
{
	uint64_t minimumStartPoint;
	uint32_t chainLength;
	uint32_t tableIndex;
	uint8_t algorithm;
		// 0 - Reserved
		// 1 - LM
		// 2 - NTLM
		// 3 - MD2
		// 4 - MD4
		// 5 - MD5
		// 6 - Double MD5
		// 7 - Binary Double MD5
		// 8 - Cisco Pix (96 bit MD5)
		// 9 - SHA1
		// 10 - MySQL SHA1
		// 11 - SHA256
		// 12 - SHA384
		// 13 - SHA512
		// 14 - RipeMD160
		// 15 - MSCache
		// 16 - Half LM Challenge
		// 17 - Second Half LM Challenge
		// 18 - NTLM Challenge
		// 19 - Oracle
	uint8_t  reductionFunction;
		// 0 - RC   ("RainbowCrack" uses divide - project-rainbowcrack.com)
		// 1 - FPM  ("Fixed Point Multiplication" - tobtu.com)
		// 2 - GRT  ("GPU RT" uses lookup tables - cryptohaze.com)
	std::string salt;
	std::vector<SubKeySpace> subKeySpaces;
	std::vector<uint32_t> checkPointPositions;
};

struct RTI20_Header
{
	uint8_t major, minor; // '2', '0'
	uint8_t startPointBits, endPointBits, checkPointBits;
	uint32_t fileIndex, files;
	RTI20_Header_RainbowTableParameters rtParams;
};

struct RTI20_Index
{
	uint64_t firstPrefix;
	std::vector<uint32_t> prefixIndex;
};

struct RTI20_File
{
	RTI20_Header header;
	RTI20_Index index;
	struct // RTI20_Data
	{
		uint8_t *data;
	};
};

#pragma pack(1)
struct RTI20_File_Header
{
	uint32_t tag; // "RTI2"
	uint8_t minor; // '0'
	uint8_t startPointBits, endPointBits, checkPointBits;
	uint32_t fileIndex, files;
	struct
	{
		uint64_t minimumStartPoint;
		uint32_t chainLength;
		uint32_t tableIndex;
		uint8_t algorithm;
			// 0 - Custom
			// 1 - LM
			// 2 - NTLM
			// 3 - MD2
			// 4 - MD4
			// 5 - MD5
			// 6 - Double MD5
			// 7 - Binary Double MD5
			// 8 - Cisco Pix (96 bit MD5)
			// 9 - SHA1
			// 10 - MySQL SHA1
			// 11 - SHA256
			// 12 - SHA384
			// 13 - SHA512
			// 14 - RipeMD160
			// 15 - MSCache
			// 16 - Half LM Challenge
			// 17 - Second Half LM Challenge
			// 18 - NTLM Challenge
			// 19 - Oracle
		uint8_t  reductionFunction;
			// 0 - RC   ("RainbowCrack" uses divide - project-rainbowcrack.com)
			// 1 - FPM  ("Fixed Point Multiplication" - tobtu.com)
			// 2 - GRT  ("GPU RT" uses lookup tables - cryptohaze.com)
	};
};
#pragma pack()

// you can't use pack(0) - it makes VC++ angry

uint8_t getAlgorithmId( std::string algorithmName );

#endif
