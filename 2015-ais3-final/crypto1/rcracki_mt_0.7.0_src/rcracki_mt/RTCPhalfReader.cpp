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

#include "RTCPhalfReader.h"

RTCPhalfReader::RTCPhalfReader( std::string filename )
{
	chainPosition = 0;
	// set it to the maximum possible value
	minimumStartPoint = (uint64_t)-1;

	setChainSizeBytes( 10 );
	setFilename( filename );
	dataFile = fopen(filename.c_str(), "rb");
}

void RTCPhalfReader::dump()
{

}

uint32_t RTCPhalfReader::getChainsLeft()
{
	return (GetFileLen( getFilename() ) / chainSizeBytes) - chainPosition;
}

std::string RTCPhalfReader::getFilename()
{
	return filename;
}

int RTCPhalfReader::readChains(uint32_t &numChains, RainbowChainCPhalf *pData)
{
	unsigned int numRead = fread(pData, 1, chainSizeBytes * numChains, dataFile);
	numChains = numRead / chainSizeBytes;
	chainPosition += numChains;
	return 0;
}

void RTCPhalfReader::setChainSizeBytes( uint32_t chainSizeBytes )
{
	this->chainSizeBytes = chainSizeBytes;
}

void RTCPhalfReader::setFilename( std::string filename )
{
	this->filename = filename;
}

void RTCPhalfReader::setMinimumStartPoint()
{
	uint64_t tmpStartPoint;
	uint64_t tmpEndPoint;
	long originalFilePos = ftell( dataFile );

	//fseek( dataFile, 0, SEEK_SET );
	rewind( dataFile );

	while ( !feof( dataFile ) )
	{
		fread( &tmpStartPoint, 8, 1, dataFile );
		fread( &tmpEndPoint, 8, 1, dataFile );

		if ( tmpStartPoint < minimumStartPoint )
			minimumStartPoint = tmpStartPoint;
	}

	fseek( dataFile, originalFilePos, SEEK_SET );
}
