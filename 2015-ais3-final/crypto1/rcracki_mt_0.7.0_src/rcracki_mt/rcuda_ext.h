// freerainbowtables is a project for generating, distributing, and using
// perfect rainbow tables
//
// Copyright 2010, 2011 Jan Kyska
// Copyright 2010 Martin Westergaard Jørgensen <martinwj2005@gmail.com>
// Copyright 2010, 2011, 2012 James Nobis <quel@quelrod.net>
//
// This file is part of freerainbowtables.
//
// freerainbowtables is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 2 of the License, or
// (at your option) any later version.
//
// freerainbowtables is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with freerainbowtables.  If not, see <http://www.gnu.org/licenses/>.

#ifndef RCUDA_EXT_H
#define RCUDA_EXT_H

#include "rcuda.h"
#include "ChainWalkContext.h"

class CudaCWCExtender {
public:
	CudaCWCExtender(CChainWalkContext *cwc);
	void Init(void);

	inline rcuda::RHash GetHash(void) { return hash; }
	inline int GetHashLen(void) { return CChainWalkContext::m_nHashLen; }
	inline unsigned int* GetPlainDimVec(void) { return &plainDimVec[0]; }
	inline int GetPlainDimVecSize(void) { return plainDimVec.size(); }
	inline unsigned char* GetCharSet(void) { return (unsigned char*)plainCharSet.c_str(); }
	inline int GetCharSetSize(void) { return plainCharSet.size(); }
	inline uint64_t GetPlainSpaceTotal(void) { return CChainWalkContext::m_nPlainSpaceTotal; }
	inline uint64_t GetRainbowTableIndex(void) { return CChainWalkContext::m_nRainbowTableIndex; }
	inline uint64_t GetReduceOffset(void) { return CChainWalkContext::m_nReduceOffset; }

protected:
	CChainWalkContext *cwc;
	rcuda::RHash hash;
	std::vector<unsigned int> plainDimVec;
	std::string plainCharSet;
};

#endif //RCUDA_EXT_H
