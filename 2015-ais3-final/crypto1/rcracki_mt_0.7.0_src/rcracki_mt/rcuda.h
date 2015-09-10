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

#ifndef RCUDA_H
#define RCUDA_H

#include "Public.h"

namespace rcuda {

enum RHash { RHASH_UNDEF = -1, RHASH_LM, RHASH_MD4, RHASH_MD5, RHASH_SHA1, RHASH_NTLM, RHASH_MYSQLSHA1 };

struct RCudaTask {
	RHash hash;
	uint64_t startIdx;
	int idxCount;
	unsigned int* dimVec;
	int dimVecSize;
	unsigned char* charSet;
	int charSetSize;
	int *cpPositions;
	int cpPosSize;
	int kernChainSize;
	uint64_t reduceOffset;
	uint64_t plainSpaceTotal;
	unsigned int rainbowChainLen;
	unsigned char *targetHash;
};

extern "C" int SetCudaDevice(int device);
extern "C" int GetChainsBufferSize(int minSize);
extern "C" int CalcChainsOnCUDA(const RCudaTask* task, uint64_t *resultBuff);
extern "C" int PreCalculateOnCUDA(const rcuda::RCudaTask* task, uint64_t *resultBuff);
extern "C" int CheckAlarmOnCUDA(const rcuda::RCudaTask* task, uint64_t *resultBuff);

}

#endif //RCUDA_H
