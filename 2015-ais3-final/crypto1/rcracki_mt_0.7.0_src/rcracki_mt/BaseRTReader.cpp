/*
 * freerainbowtables is a project for generating, distributing, and using
 * perfect rainbow tables
 *
 * Copyright 2010, 2011 Martin Westergaard Jørgensen <martinwj2005@gmail.com>
 * Copyright 2010 Daniël Niggebrugge <niggebrugge@fox-it.com>
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

#include "BaseRTReader.h"

/// Default Constructor
BaseRTReader::BaseRTReader()
{
	data = NULL;
	this->chainPosition = 0;
	this->chainCount = 0;
	this->chainLength = 0;
	this->tableIndex = 0;
	this->startPointBits = 0;
	this->endPointBits = 0;
	fileName = "";
	salt = "";	
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
BaseRTReader::BaseRTReader(uint32_t chCount, uint32_t chLength, uint32_t tblIdx, uint32_t stPt, uint32_t endPt, std::string fname, std::string slt)
{
	this->chainCount = chCount;
	this->chainLength = chLength;
	this->tableIndex = tblIdx;
	this->startPointBits = stPt;
	this->endPointBits = endPt;
	this->fileName = fname;
	this->data = NULL;

	this->data = fopen( fileName.c_str(), "rb" );
	if( data == NULL )
	{
		std::cerr << "ERROR: could not open file: " << fileName.c_str() << " EXITING!" << std::endl;
		exit(-1);
	}

	this->salt = slt;
	this->chainPosition = 0;
}

/// Destructor
BaseRTReader::~BaseRTReader()
{
	if( data != NULL )
	{
		fclose( data );
	}
}

/// getChainCount
uint32_t BaseRTReader::getChainCount()
{
	return this->chainCount;
}

/// getChainLength
uint32_t BaseRTReader::getChainLength()
{
	return this->chainLength;
}

/// getChainPosition
uint32_t BaseRTReader::getChainPosition()
{
	return this->chainPosition;
}

/// getTableIndex
uint32_t BaseRTReader::getTableIndex()
{
	return this->tableIndex;
}

/// getStartPointBits
uint32_t BaseRTReader::getStartPointBits()
{
	return this->startPointBits;
}

/// getEndPointBits
uint32_t BaseRTReader::getEndPointBits()
{
	return this->endPointBits;
}

/// getFileName
std::string BaseRTReader::getFileName()
{
	return this->fileName;
}

/// getSalt
std::string BaseRTReader::getSalt()
{
	return this->salt;
}

/// setChainCount
void BaseRTReader::setChainCount(uint32_t chCount)
{
	this->chainCount = chCount;
}

/// setChainLength
void BaseRTReader::setChainLength(uint32_t chLength)
{
	this->chainLength = chLength;
}

/// setTableIndex
void BaseRTReader::setTableIndex(uint32_t tblIdx)
{
	this->tableIndex = tblIdx;
}

/// setStartPointBits
void BaseRTReader::setStartPointBits(uint32_t stPt)
{
	this->startPointBits = stPt;
}

/// setEndPointBits
void BaseRTReader::setEndPointBits(uint32_t endPt)
{
	this->endPointBits = endPt;
}

/// setFileName
void BaseRTReader::setFileName(std::string fname)
{
	this->fileName = fname;
}

/// setSalt
void BaseRTReader::setSalt(std::string slt)
{
	this->salt = slt;
}

/// Dump
void BaseRTReader::Dump()
{
}
