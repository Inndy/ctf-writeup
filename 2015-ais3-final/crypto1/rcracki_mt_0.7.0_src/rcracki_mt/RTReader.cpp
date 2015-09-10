/*
 * freerainbowtables is a project for generating, distributing, and using
 * perfect rainbow tables
 *
 * Copyright 2010, 2011 Martin Westergaard Jørgensen <martinwj2005@gmail.com>
 * Copyright 2010, 2011, 2012 James Nobis <quel@quelrod.net>
 * Copyright 2011 Logan Watt <logan.watt@gmail.com>
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

#include "RTReader.h"

/// Default Constructor
RTReader::RTReader()
{
	RTReaderInit();
}

/** Constructor with filename
 * @param std::string file name on disk
 */
RTReader::RTReader(std::string fname)
{
	RTReaderInit();

	//bytes per chain
	setFileName( fname );

	if( stat( getFileName().c_str(), &fileStats ) == -1 )
	{
		std::cerr << "ERROR: stat() for file: " << getFileName() << " FAILED! " << std::endl;	
		exit(-1);
	}

	data = fopen( getFileName().c_str(), "rb" );

	if( data == NULL )
	{
		std::cerr << "ERROR: could not open table file: " << getFileName() << " EXITING!" << std::endl;
		exit(-1);
	}
}

/**
 * Argument Constructor
 * @param uint32_t number of chains in the file
 * @param uint32_t size of the chain
 * @param uint32_t reduction function index offset
 * @param uint32_t start point in the chain
 * @param uint32_t end point in the chain
 * @param std::string name of the file on disk
 * @param std::string salt used for hash
 */
RTReader::RTReader(uint32_t chCount, uint32_t chLength, uint32_t tblIdx, uint32_t stPt, uint32_t endPt, std::string fname, std::string slt)
{
	RTReaderInit();
	setFileName( fname );

	if( stat( getFileName().c_str(), &fileStats ) == -1 )
	{
		std::cerr << "ERROR: stat() for file: " << getFileName() << " FAILED! " << std::endl;	
		exit(-1);
	}

	setChainCount( chCount );
	setChainLength( chLength );
	setTableIndex( tblIdx );
	setStartPointBits( stPt );
	setEndPointBits( endPt );
	setSalt( slt );

	data = fopen( getFileName().c_str(), "rb" );
	if( data == NULL )
	{
		std::cerr << "ERROR: could not open table file: " << getFileName() << " EXITING!" << std::endl;
		exit(-1);
	}
}

/// shared Init method
void RTReader::RTReaderInit()
{
	// bytes per chain
	this->chainSize = 16;
	setStartPointBits(8);
	setEndPointBits(8);
}

/// Deconstructor
RTReader::~RTReader()
{
}

/// getChainSize
uint32_t RTReader::getChainSize()
{
	return this->chainSize;
}

/// getDataFileSize
uint32_t RTReader::getDataFileSize()
{
	return fileStats.st_size;
}

/**
 * reads data chains into memory
 * @param uint32_t reference to the number of chains to read
 * @param RanbowChain0* pointer in memory to read the chains to
 * @return EXIT_SUCCESS or EXIT_FAILURE
 */
int RTReader::readChains(uint32_t &numChains, RainbowChainO *pData)
{
	unsigned int numRead = fread(pData, 1, chainSize * numChains, data);

	if( numRead == 0 )
	{
		chainPosition = 0;
		return EXIT_FAILURE;
	}

	numChains = numRead / chainSize;
	chainPosition += numChains;

	return EXIT_SUCCESS;
}

/// getChainsLeft
uint32_t RTReader::getChainsLeft()
{
	return ( fileStats.st_size / chainSize ) - chainPosition;
}

/// getMinimumStartPoint
uint64_t RTReader::getMinimumStartPoint()
{
	uint64_t tmpStartPoint;
	uint64_t tmpEndPoint;
	uint64_t minimumStartPoint = (uint64_t)-1;
	long originalFilePos = ftell( data );

	rewind( data );

	while ( !feof( data ) )
	{
		fread( &tmpStartPoint, 8, 1, data );
		fread( &tmpEndPoint, 8, 1, data );

		if ( tmpStartPoint < minimumStartPoint )
			minimumStartPoint = tmpStartPoint;
	}

	fseek( data, originalFilePos, SEEK_SET );
	return minimumStartPoint;
}
