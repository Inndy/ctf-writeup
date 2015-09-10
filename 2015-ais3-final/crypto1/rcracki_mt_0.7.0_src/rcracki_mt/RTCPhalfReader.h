/*
 * freerainbowtables is a project for generating, distributing, and using
 * perfect rainbow tables
 *
 * Copyright 2010, 2011 Martin Westergaard Jørgensen <martinwj2005@gmail.com>
 * Copyright 2010, 2011 James Nobis <quel@quelrod.net>
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

#ifndef _RTCPHALFREADER_H
#define _RTCPHALFREADER_H

#include <string>

#if defined(_WIN32) && !defined(__GNUC__)
	#include <io.h>
#endif

#include "Public.h"
#include "BaseRTReader.h"

class RTCPhalfReader
{
private:
	FILE *dataFile;
	uint32_t chainPosition;
	uint32_t chainSizeBytes;
	uint64_t minimumStartPoint;
	std::string filename;
	
	void setChainSizeBytes( uint32_t chainSizeBytes );
	void setFilename( std::string filename );

public:
	RTCPhalfReader( std::string filename );
	~RTCPhalfReader() { };

	void dump();
	uint32_t getChainsLeft();
	std::string getFilename();
	int readChains(uint32_t &numChains, RainbowChainCPhalf *pData);
	void setMinimumStartPoint();
};

#endif
