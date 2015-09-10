/*
 * freerainbowtables is a project for generating, distributing, and using
 * perfect rainbow tables
 *
 * Copyright 2011 Sc00bz
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

#include "RTI2Common.h"
#include <map>
#include <iterator>
#include <iostream>

enum algorithmValue { Custom
	, LM
	, NTLM
	, MD2
	, MD4
	, MD5
	, DoubleMD5
	, DoubleBinaryMD5
	, CiscoPIX
	, SHA1
	, MySQLSHA1
	, SHA256
	, SHA384
	, SHA512
	, RIPEMD160
	, MSCache
	, HalfLMChallenge
	, SecondHalfLMChallenge
	, NTLMChallenge
	, Oracle
};

static std::map<std::string, algorithmValue> mapAlgorithmValue;

static void initializeAlgorithmMap()
{
	mapAlgorithmValue["Custom"] = Custom;
	mapAlgorithmValue["lm"] = LM;
	mapAlgorithmValue["ntlm"] = NTLM;
	mapAlgorithmValue["md2"] = MD2;
	mapAlgorithmValue["md4"] = MD4;
	mapAlgorithmValue["md5"] = MD5;
	mapAlgorithmValue["doublemd5"] = DoubleMD5;
	mapAlgorithmValue["doublebinarymd5"] = DoubleBinaryMD5;
	mapAlgorithmValue["ciscopix"] = CiscoPIX;
	mapAlgorithmValue["sha1"] = SHA1;
	mapAlgorithmValue["mysqlsha1"] = MySQLSHA1;
	mapAlgorithmValue["sha256"] = SHA256;
	mapAlgorithmValue["sha384"] = SHA384;
	mapAlgorithmValue["sha512"] = SHA512;
	mapAlgorithmValue["ripemd160"] = RIPEMD160;
	mapAlgorithmValue["mscache"] = MSCache;
	mapAlgorithmValue["halflmchall"] = HalfLMChallenge;
	mapAlgorithmValue["SecondHalfLMChallenge"] = SecondHalfLMChallenge;
	mapAlgorithmValue["ntlmchall"] = NTLMChallenge;
	mapAlgorithmValue["oracle"] = Oracle;
}

uint8_t getAlgorithmId( std::string algorithmName )
{
	initializeAlgorithmMap();
	std::map<std::string, algorithmValue>::iterator iter;

	iter = mapAlgorithmValue.find(algorithmName);

	if ( iter == mapAlgorithmValue.end() )
	{
		std::cout << "Hash Algorithm " << algorithmName << " is not supported"
			<< std::endl;
		exit( 1 );
	}

	return iter->second;
}
