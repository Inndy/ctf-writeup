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

#ifndef _BASERTREADER_H
#define _BASERTREADER_H

#include "Public.h"

/**
 * The foundation for all the RT reader implementations
 */

class BaseRTReader
{
	private:
		/// Members
		uint32_t chainCount;		// number of chains in file
		uint32_t chainLength;		// size of chain
		uint32_t tableIndex;		// reduction function index offset
		uint32_t startPointBits;	// start point in the chain
		uint32_t endPointBits;		// end point in the chain
		std::string fileName; 	// name of file on disk
		std::string salt;			// salt used for hash

	protected:
		FILE *data;					// binary data
		uint32_t chainPosition;
		/// Set Methods
		virtual void setChainCount(uint32_t);
		virtual void setChainLength(uint32_t);
		virtual void setTableIndex(uint32_t);
		virtual void setStartPointBits(uint32_t);
		virtual void setEndPointBits(uint32_t);
		virtual void setFileName(std::string);
		virtual void setSalt(std::string);

	public:
		/// Default constructor
		BaseRTReader();

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
		BaseRTReader(uint32_t, uint32_t, uint32_t, uint32_t, uint32_t, std::string, std::string);

		/// Default destructor
		virtual ~BaseRTReader();

		/**
		 * reads data chains into memory
		 * @param uint32_t reference to the number of chains to read
		 * @param RanbowChain0* pointer in memory to read the chains to
		 * @return EXIT_SUCCESS or EXIT_FAILURE
		 */
		virtual int readChains(uint32_t &numChains, RainbowChainO *pData) = 0;
		virtual uint32_t getChainsLeft() = 0;
		virtual uint64_t getMinimumStartPoint() = 0;

		/// Get Methods
		virtual uint32_t getChainCount();
		virtual uint32_t getChainLength();
		virtual uint32_t getChainPosition();
		virtual uint32_t getTableIndex();
		virtual uint32_t getStartPointBits();
		virtual uint32_t getEndPointBits();
		virtual std::string getFileName();
		virtual std::string getSalt();

		/**
		 * Debugging function
		 */
		virtual void Dump();
};

#endif
