/*
 * rcracki_mt is a multithreaded implementation and fork of the original 
 * RainbowCrack
 *
 * Copyright 2009, 2010 Daniël Niggebrugge <niggebrugge@fox-it.com>
 * Copyright 2009, 2010, 2011, 2012 James Nobis <quel@quelrod.net>
 * Copyright 2011 Jan Kyska
 *
 * This file is part of rcracki_mt.
 *
 * rcracki_mt is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 2 of the License, or
 * (at your option) any later version.
 *
 * rcracki_mt is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with rcracki_mt.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "rcrackiThread.h"


#if GPU
#include "rcuda.h"
#include "rcuda_ext.h"
#include <cuda.h> 
#include <cuda_runtime_api.h>
#endif

// create job for pre-calculation
rcrackiThread::rcrackiThread(unsigned char* TargetHash, int thread_id, int nRainbowChainLen, int thread_count, uint64_t* pStartPosIndexE)
{
	t_TargetHash = TargetHash;
	t_nRainbowChainLen = nRainbowChainLen;
	t_ID = thread_id;
	t_count = thread_count;
	t_pStartPosIndexE = pStartPosIndexE;
	t_nChainWalkStep = 0;
	falseAlarmChecker = false;
	falseAlarmCheckerO = false;
	gpu = 0;
	cudaDevId = -1;
}

// create job for false alarm checking
rcrackiThread::rcrackiThread(unsigned char* pHash, bool oldFormat)
{
	falseAlarmChecker = true;
	falseAlarmCheckerO = oldFormat;
	t_pChainsFound.clear();
	t_nGuessedPoss.clear();
	t_pHash = pHash;
	t_nChainWalkStepDueToFalseAlarm = 0;
	t_nFalseAlarm = 0;
	foundHash = false;
	gpu = 0;
	cudaDevId = -1;
}

void rcrackiThread::AddAlarmCheck(RainbowChain* pChain, int nGuessedPos)
{
	t_pChainsFound.push_back(pChain);
	t_nGuessedPoss.push_back(nGuessedPos);
}

void rcrackiThread::AddAlarmCheckO(RainbowChainO* pChain, int nGuessedPos)
{
	t_pChainsFoundO.push_back(pChain);
	t_nGuessedPoss.push_back(nGuessedPos);
}

// Windows (beginthreadex) way of threads
//unsigned __stdcall rcrackiThread::rcrackiThreadStaticEntryPoint(void * pThis)
//{
//	rcrackiThread* pTT = (rcrackiThread*)pThis;
//	pTT->rcrackiThreadEntryPoint();
//	_endthreadex( 2 );
//	return 2;
//}

// entry point for the posix thread
void * rcrackiThread::rcrackiThreadStaticEntryPointPthread(void * pThis)
{
	rcrackiThread* pTT = (rcrackiThread*)pThis;
	pTT->rcrackiThreadEntryPoint();
	pthread_exit(NULL);
	return NULL;
}

// start processing of jobs
void rcrackiThread::rcrackiThreadEntryPoint()
{
#if GPU
	if(gpu != 0 && cudaGetDevice(&cudaDevId) == CUDA_SUCCESS) {
		cudaBuffCount = 0x2000;
		cudaChainSize = 100;

		cudaDeviceProp deviceProp;
		if(cudaGetDeviceProperties(&deviceProp, cudaDevId) == CUDA_SUCCESS) {
			switch(deviceProp.major) {
			case 1: ; break;
			case 2:
				cudaBuffCount = 0x4000;
				cudaChainSize = 200;
				break;
			}
		}
		cudaBuffCount = rcuda::GetChainsBufferSize(cudaBuffCount);
	}
	else
#endif
		cudaDevId = -1;

	if (falseAlarmChecker) {
		if (falseAlarmCheckerO) {
			CheckAlarmO();
		}
		else {
			CheckAlarm();
		}
	}
	else {
		PreCalculate();
	}
}

uint64_t rcrackiThread::GetIndex(int nPos)
{
	uint64_t t_index = t_vStartPosIndexE[nPos - t_ID];
	return t_index;
}

int rcrackiThread::GetChainWalkStep()
{
	return t_nChainWalkStep;
}

int rcrackiThread::GetIndexCount()
{
	return t_vStartPosIndexE.size();
}

rcrackiThread::~rcrackiThread(void)
{
}

void rcrackiThread::PreCalculate()
{
	if(cudaDevId < 0) {
		for (t_nPos = t_nRainbowChainLen - 2 - t_ID; t_nPos >= 0; t_nPos -= t_count)
		{
			t_cwc.SetHash(t_TargetHash);
			t_cwc.HashToIndex(t_nPos);
			int i;
			for ( i = t_nPos + 1; i <= t_nRainbowChainLen - 2; i++ )
			{
				t_cwc.IndexToPlain();
				t_cwc.PlainToHash();
				t_cwc.HashToIndex(i);
			}

			t_pStartPosIndexE[t_nPos] = t_cwc.GetIndex();
			t_nChainWalkStep += t_nRainbowChainLen - 2 - t_nPos;
		}
	}
#if GPU
	else
	{
		CudaCWCExtender ex(&t_cwc);
		rcuda::RCudaTask cuTask;
		int ii, calcSize;

		ex.Init();
		for (t_nPos = 0; t_nPos < t_nRainbowChainLen - 1; ) {
			calcSize = std::min<int>(t_nPos + cudaBuffCount, t_nRainbowChainLen - 1);
			for (ii = t_nPos; ii < calcSize; ii++) {
				t_cwc.SetHash(t_TargetHash);
				t_cwc.HashToIndex(ii);
				t_pStartPosIndexE[ii] = t_cwc.GetIndex();
			}
			calcSize -= t_nPos;

			cuTask.hash = ex.GetHash();
			cuTask.startIdx = t_nPos;
			cuTask.idxCount = calcSize;
			cuTask.dimVec = ex.GetPlainDimVec();
			cuTask.dimVecSize = ex.GetPlainDimVecSize()/2;
			cuTask.charSet = ex.GetCharSet();
			cuTask.charSetSize = ex.GetCharSetSize();
			cuTask.reduceOffset = ex.GetReduceOffset();
			cuTask.plainSpaceTotal = ex.GetPlainSpaceTotal();
			cuTask.rainbowChainLen = t_nRainbowChainLen;
			cuTask.kernChainSize = cudaChainSize;

			calcSize = rcuda::PreCalculateOnCUDA(&cuTask, t_pStartPosIndexE + cuTask.startIdx);
			if(calcSize > 0 && calcSize == cuTask.idxCount) {
				t_nChainWalkStep += calcSize*(t_nRainbowChainLen - 2) - (calcSize*(2*t_nPos + calcSize - 1))/2;
				t_nPos += calcSize;
			} else {
				printf("PreCalculate() on CUDA failed!\n");
				exit(100);
			}
		}
	}
#endif
}

void rcrackiThread::CheckAlarm()
{
	uint32_t i;

	if(cudaDevId < 0) {
		for (i = 0; i < t_pChainsFound.size(); i++)
		{
			RainbowChain* t_pChain = t_pChainsFound[i];
			int t_nGuessedPos = t_nGuessedPoss[i];		
			
			CChainWalkContext cwc;
			//uint64_t nIndexS = t_pChain->nIndexS & 0x0000FFFFFFFFFFFF; // for first 6 bytes
			//uint64_t nIndexS = t_pChain->nIndexS >> 16;
			uint64_t nIndexS = t_pChain->nIndexS & 0x0000FFFFFFFFFFFFULL; // for first 6 bytes
			cwc.SetIndex(nIndexS);
			//cwc.SetIndex(t_pChain->nIndexS);	
			int nPos;
			for (nPos = 0; nPos < t_nGuessedPos; nPos++)
			{
				cwc.IndexToPlain();
				cwc.PlainToHash();
				cwc.HashToIndex(nPos);
			}
			cwc.IndexToPlain();
			cwc.PlainToHash();
			if (cwc.CheckHash(t_pHash))
			{
				t_Hash = cwc.GetHash();
				t_Plain = cwc.GetPlain();
				t_Binary = cwc.GetBinary();

				foundHash = true;
				break;
			}
			else {
				foundHash = false;
				t_nChainWalkStepDueToFalseAlarm += t_nGuessedPos + 1;
				t_nFalseAlarm++;
			}
		}
	}
#if GPU
	else {
		uint64_t *calcBuff = new uint64_t[2*cudaBuffCount];
		CudaCWCExtender ex(&t_cwc);
		rcuda::RCudaTask cuTask;
		int nGuessedPos, nMaxGuessedPos;
		int ii, calcSize;
		size_t chainSize;

		ex.Init();
		chainSize = t_pChainsFound.size();
		foundHash = false;
		for(i = 0; !foundHash && i < chainSize; ) {
			calcSize = std::min<int>(i + cudaBuffCount, chainSize);
			nMaxGuessedPos = 0;
			for(ii = i; ii < calcSize; ii++) {
				calcBuff[(ii<<1)] = (t_pChainsFound[ii]->nIndexS & 0x0000FFFFFFFFFFFFull); // for first 6 bytes
				nGuessedPos = t_nGuessedPoss[ii];
				calcBuff[(ii<<1)+1] = (unsigned int)nGuessedPos;
				nMaxGuessedPos = (nMaxGuessedPos>=nGuessedPos? nMaxGuessedPos : nGuessedPos);
			}
			calcSize -= i;

			cuTask.hash = ex.GetHash();
			cuTask.startIdx = i;
			cuTask.idxCount = calcSize;
			cuTask.dimVec = ex.GetPlainDimVec();
			cuTask.dimVecSize = ex.GetPlainDimVecSize()/2;
			cuTask.charSet = ex.GetCharSet();
			cuTask.charSetSize = ex.GetCharSetSize();
			cuTask.reduceOffset = ex.GetReduceOffset();
			cuTask.plainSpaceTotal = ex.GetPlainSpaceTotal();
			cuTask.rainbowChainLen = nMaxGuessedPos;
			cuTask.kernChainSize = cudaChainSize;
			cuTask.targetHash = t_pHash;

			calcSize = rcuda::CheckAlarmOnCUDA(&cuTask, calcBuff);
			if(calcSize > 0 && calcSize == cuTask.idxCount) {
				for(ii = 0; ii < calcSize; ii++) {
					if(calcBuff[(ii<<1)|1] >= (1ull<<63)) {
						CChainWalkContext cwc;
						cwc.SetIndex(calcBuff[ii<<1]);
						cwc.IndexToPlain();
						cwc.PlainToHash();
						t_Hash = cwc.GetHash();
						t_Plain = cwc.GetPlain();
						t_Binary = cwc.GetBinary();
						foundHash = true;
						break;
					} else {
						t_nChainWalkStepDueToFalseAlarm += calcBuff[(ii<<1)|1] + 1;
						t_nFalseAlarm++;
					}
				}
				i += calcSize;
			} else {
				printf("CheckAlarm() on CUDA failed!\n");
				exit(101);
			}
		}
		delete [] calcBuff;
	}
#endif
}

void rcrackiThread::CheckAlarmO()
{
	uint32_t i;

	if(cudaDevId < 0) {
		for (i = 0; i < t_pChainsFoundO.size(); i++)
		{
			RainbowChainO* t_pChain = t_pChainsFoundO[i];
			int t_nGuessedPos = t_nGuessedPoss[i];		
			
			CChainWalkContext cwc;

			uint64_t nIndexS = t_pChain->nIndexS;
			cwc.SetIndex(nIndexS);

			int nPos;
			for (nPos = 0; nPos < t_nGuessedPos; nPos++)
			{
				cwc.IndexToPlain();
				cwc.PlainToHash();
				cwc.HashToIndex(nPos);
			}
			cwc.IndexToPlain();
			cwc.PlainToHash();
			if (cwc.CheckHash(t_pHash))
			{
				t_Hash = cwc.GetHash();
				t_Plain = cwc.GetPlain();
				t_Binary = cwc.GetBinary();

				foundHash = true;
				break;
			}
			else {
				foundHash = false;
				t_nChainWalkStepDueToFalseAlarm += t_nGuessedPos + 1;
				t_nFalseAlarm++;
			}
		}
	}
#if GPU
	else {
		uint64_t *calcBuff = new uint64_t[2*cudaBuffCount];
		CudaCWCExtender ex(&t_cwc);
		rcuda::RCudaTask cuTask;
		int nGuessedPos, nMaxGuessedPos;
		int ii, calcSize;
		size_t chainSize;

		ex.Init();
		chainSize = t_pChainsFoundO.size();
		foundHash = false;
		for(i = 0; !foundHash && i < chainSize; ) {
			calcSize = std::min<int>(i + cudaBuffCount, chainSize);
			nMaxGuessedPos = 0;
			for(ii = i; ii < calcSize; ii++) {
				calcBuff[(ii<<1)] = t_pChainsFoundO[ii]->nIndexS;
				nGuessedPos = t_nGuessedPoss[ii];
				calcBuff[(ii<<1)+1] = (unsigned int)nGuessedPos;
				nMaxGuessedPos = (nMaxGuessedPos>=nGuessedPos? nMaxGuessedPos : nGuessedPos);
			}
			calcSize -= i;

			cuTask.hash = ex.GetHash();
			cuTask.startIdx = i;
			cuTask.idxCount = calcSize;
			cuTask.dimVec = ex.GetPlainDimVec();
			cuTask.dimVecSize = ex.GetPlainDimVecSize()/2;
			cuTask.charSet = ex.GetCharSet();
			cuTask.charSetSize = ex.GetCharSetSize();
			cuTask.reduceOffset = ex.GetReduceOffset();
			cuTask.plainSpaceTotal = ex.GetPlainSpaceTotal();
			cuTask.rainbowChainLen = nMaxGuessedPos;
			cuTask.kernChainSize = cudaChainSize;
			cuTask.targetHash = t_pHash;

			calcSize = rcuda::CheckAlarmOnCUDA(&cuTask, calcBuff);
			if(calcSize > 0 && calcSize == cuTask.idxCount) {
				for(ii = 0; ii < calcSize; ii++) {
					if(calcBuff[(ii<<1)|1] >= (1ull<<63)) {
						CChainWalkContext cwc;
						cwc.SetIndex(calcBuff[ii<<1]);
						cwc.IndexToPlain();
						cwc.PlainToHash();
						t_Hash = cwc.GetHash();
						t_Plain = cwc.GetPlain();
						t_Binary = cwc.GetBinary();
						foundHash = true;
						break;
					} else {
						t_nChainWalkStepDueToFalseAlarm += calcBuff[(ii<<1)|1] + 1;
						t_nFalseAlarm++;
					}
				}
				i += calcSize;
			} else {
				printf("CheckAlarmO() on CUDA failed!\n");
				exit(101);
			}
		}
		delete [] calcBuff;
	}
#endif
}

bool rcrackiThread::FoundHash()
{
	return foundHash;
}

int rcrackiThread::GetChainWalkStepDueToFalseAlarm()
{
	return t_nChainWalkStepDueToFalseAlarm;
}

int rcrackiThread::GetnFalseAlarm()
{
	return t_nFalseAlarm;
}

std::string rcrackiThread::GetHash()
{
	return t_Hash;
}

std::string rcrackiThread::GetPlain()
{
	return t_Plain;
}

std::string rcrackiThread::GetBinary()
{
	return t_Binary;
}

void rcrackiThread::Configure(int gpu) 
{
	this->gpu = gpu;
}
