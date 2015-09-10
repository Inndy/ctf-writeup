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

#ifndef _RTREADER_H
#define _RTREADER_H

#include "BaseRTReader.h"

/**
 * Reader class for original .rt file format
 */
class RTReader : public BaseRTReader
{
	private:
		uint32_t chainSize;
		struct stat fileStats;
		void RTReaderInit();

	protected:

	public:
		/// Default Constructor
		RTReader();

		/** Constructor with filename
		 * @param std::string file name on disk
		 */
		RTReader(std::string);

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
		RTReader(uint32_t, uint32_t, uint32_t, uint32_t, uint32_t, std::string, std::string);

		/// Destructor
		~RTReader();

		/// Get Methods
		uint32_t getChainSize();
		uint32_t getDataFileSize();

		int readChains(uint32_t&, RainbowChainO*);
		uint32_t getChainsLeft();
		uint64_t getMinimumStartPoint();
};

#endif
