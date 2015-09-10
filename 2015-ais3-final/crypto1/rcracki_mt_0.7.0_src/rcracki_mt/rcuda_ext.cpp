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

#include "rcuda_ext.h"
#include <algorithm>
#include <string>
#include <time.h>
#include <fstream>
#include <iostream>

CudaCWCExtender::CudaCWCExtender(CChainWalkContext *cwc) { 
	this->cwc = cwc; 
	hash = rcuda::RHASH_UNDEF;
}

// sets rcuda kernel hash name
// sets basic charset lengths and keyspaces per position of the plaintext
// plainDimVec represents the primary data passed to CUDA
void CudaCWCExtender::Init(void) { 
	std::string hashName;
	int ii, jj;

	plainDimVec.clear();
	plainCharSet.clear();

	hashName = CChainWalkContext::m_sHashRoutineName;
	std::transform(hashName.begin(), hashName.end(), hashName.begin(), ::tolower);
	if(hashName.compare("lm") == 0)
		hash = rcuda::RHASH_LM;
	else if(hashName.compare("md4") == 0)
		hash = rcuda::RHASH_MD4;
	else if(hashName.compare("md5") == 0)
		hash = rcuda::RHASH_MD5;
	else if(hashName.compare("sha1") == 0)
		hash = rcuda::RHASH_SHA1;
	else if(hashName.compare("mysqlsha1") == 0)
		hash = rcuda::RHASH_MYSQLSHA1;
	else if(hashName.compare("ntlm") == 0)
		hash = rcuda::RHASH_NTLM;
	else
		hash = rcuda::RHASH_UNDEF;

	for(ii = (int)CChainWalkContext::m_vCharset.size()-1; !(ii < 0); ii--) {
		stCharset &chs = CChainWalkContext::m_vCharset[ii];
		int chSetOffset = plainCharSet.size();
		plainCharSet.append((char*)chs.m_PlainCharset, chs.m_nPlainCharsetLen);
		uint64_t plainSpace = 1;
		for(jj = 0; jj < ii; jj++)
			plainSpace *= CChainWalkContext::m_vCharset[jj].m_nPlainSpaceTotal;
		plainDimVec.push_back((unsigned int)plainSpace);
		plainDimVec.push_back((unsigned int)(plainSpace>>32));
		for(jj = 0; jj < chs.m_nPlainLenMax; jj++) {
			plainDimVec.push_back((unsigned int)chs.m_nPlainCharsetLen|((unsigned int)chSetOffset<<16)|(jj<chs.m_nPlainLenMin?0:(1u<<24))|(jj+1==chs.m_nPlainLenMax?(1u<<25):0));
			plainDimVec.push_back((unsigned int)-1/(unsigned int)chs.m_nPlainCharsetLen);
		}
	}
}
