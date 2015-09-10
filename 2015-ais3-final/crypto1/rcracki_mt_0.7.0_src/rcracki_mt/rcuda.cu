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


#include <stdio.h>
#include <cuda.h>  
#include "rcuda.h"

#define   GRID_X_L2     6
#define   GRID_Y_L2     6
#define   BLOCK_X_L2    6
#define   GRID_X_SIZE   (1<<GRID_X_L2)
#define   GRID_Y_SIZE   (1<<GRID_Y_L2)
#define   BLOCK_X_SIZE  (1<<BLOCK_X_L2)
#define   PLAIN_MAX_SIZE     20
//#define   KERN_CHAIN_SIZE   100
#define   CHAR_SET_MAXLEN   512
#define   SHIDX(x)      ((x)<<4)
#define   DATAIDX(vol, idx)  ((((idx)>>4)<<(4+vol))+((idx)&15))
#define   THREADIDX  ((((blockIdx.y<<GRID_X_L2) + blockIdx.x)<<BLOCK_X_L2) + threadIdx.x)


__device__ uint64_t *dataHeap;
__constant__ __device__ int *plCpPos;
__constant__ __device__ int plCpPosSize;
__constant__ __device__ uint64_t reduceOffset;
__constant__ __device__ uint64_t plainSpaceTotal;
__constant__ __device__ uint64_t rPlainSpaceTotal;
__constant__ __device__ unsigned char cplChrSet[CHAR_SET_MAXLEN];
__constant__ __device__ uint2 cplDimVec[2*PLAIN_MAX_SIZE];
__constant__ __device__ uint64_t cplStartIdx;
__constant__ __device__ unsigned int cplTargetHash[8];
__constant__ __device__ int cplTargetHashSize;
__device__ int termKernel;

#define RTGEN_PROLOGUE  \
	unsigned int *hData;  \
	unsigned int idx;  \
	uint64_t idx64;  \
	unsigned int nPos, ii;  \
	unsigned int cpcheck, checkpoint;  \
	uint3 dimItem;  \
	uint64_t uiDiv64, uiVal64, uiMul64;  \
	uint64_t plainSpace, idx64mod;  \
	unsigned int uiVal, uiDiv;  \
	unsigned int dimVecIdx;  \
	\
	__shared__ unsigned int shData[SHIDX(BLOCK_X_SIZE)];  \
	\
	if(threadIdx.x == 0) {  \
		nPos = (THREADIDX<<1);  \
		for(ii = 0; ii < BLOCK_X_SIZE; ii++, nPos+=2) {  \
			hData = shData + DATAIDX(4, ii);  \
			hData[SHIDX(0)] = dataHeap[nPos];  \
			hData[SHIDX(1)] = dataHeap[nPos]>>32;  \
			hData[SHIDX(2)] = dataHeap[nPos+1];  \
		}  \
	}  \
	__syncthreads();  \
	\
	hData = shData + DATAIDX(4, threadIdx.x);  \
	\
	idx64 = hData[SHIDX(1)];  \
	idx64 = (idx64<<32) | hData[SHIDX(0)];  \
	cpcheck = hData[SHIDX(2)];  \
	checkpoint = cpcheck&0x0000ffff;  \
	cpcheck = cpcheck>>16;  \
	\
	for(nPos = chainStart; nPos < chainStop; nPos++) {

#define RTGEN_EPILOGUE  \
		idx64 = hData[SHIDX(1)];  \
		idx64 = (idx64<<32) | hData[SHIDX(0)];  \
		idx64 += reduceOffset + nPos;  \
		uiDiv64 = __umul64hi(idx64, rPlainSpaceTotal);  \
		idx64 -= uiDiv64*plainSpaceTotal;  \
		if(idx64 >= plainSpaceTotal)  \
			idx64 -= plainSpaceTotal;  \
		\
		if(cpcheck < plCpPosSize && nPos == plCpPos[cpcheck]) {  \
			checkpoint |= ((unsigned int)idx64&1) << cpcheck;  \
			cpcheck++;  \
		}  \
	}  \
	\
	hData[SHIDX(0)] = idx64;  \
	hData[SHIDX(1)] = idx64>>32;  \
	hData[SHIDX(2)] = (cpcheck<<16)|(checkpoint&0x0000ffff);  \
	__syncthreads();  \
	\
	if(threadIdx.x == 0) {  \
		nPos = (THREADIDX<<1);  \
		for(ii = 0; ii < BLOCK_X_SIZE; ii++, nPos+=2) {  \
			hData = shData + DATAIDX(4, ii);  \
			dataHeap[nPos] = ((uint64_t)hData[SHIDX(1)]<<32)|(uint64_t)hData[SHIDX(0)];  \
			dataHeap[nPos+1] = hData[SHIDX(2)];  \
		}  \
	}

#define RTGEN_IDX2PLAIN_BEGIN  \
	dimVecIdx = 0;  \
	ii = 0;  \
	do {  \
		plainSpace = (uint64_t)cplDimVec[dimVecIdx].x | ((uint64_t)cplDimVec[dimVecIdx].y<<32);  \
		dimVecIdx++;  \
		uiVal64 = idx64/plainSpace;  \
		idx64mod = idx64 - uiVal64*plainSpace;  \
		idx64 = uiVal64

#define RTGEN_IDX2PLAIN_END  \
		if(!(dimItem.z&512)) {  \
			while(!((cplDimVec[dimVecIdx].x>>16)&512))  \
				dimVecIdx++;  \
			dimVecIdx++;  \
		}  \
		\
		idx64 = idx64mod;  \
	} while(plainSpace > 1)

#define RTGEN_I2P_LOOP64_BEGIN  \
	dimItem.z = 0;  \
	for(; idx64 > 0xfffffff0ull && ii < PLAIN_MAX_SIZE && !(dimItem.z&512); ii++, dimVecIdx++) {  \
		uiVal64 = idx64;  \
		dimItem.x = (cplDimVec[dimVecIdx].x&0xffffu);  \
		dimItem.y = cplDimVec[dimVecIdx].y;  \
		dimItem.z = (cplDimVec[dimVecIdx].x>>16);  \
		\
		uiMul64 = (uint64_t)dimItem.y<<32;  \
		idx64 = __umul64hi(uiVal64, uiMul64);  \
		uiDiv64 = uiVal64 - idx64*(uint64_t)dimItem.x;  \
		uiVal = __umulhi((unsigned int)uiDiv64, dimItem.y);  \
		uiDiv = (unsigned int)uiDiv64 - uiVal * dimItem.x;  \
		idx64 += uiVal;  \
		if(uiDiv >= dimItem.x) {  \
			uiDiv -= dimItem.x;  \
			idx64++;  \
		}  \
		\
		if(dimItem.z&256) {  \
			if(!uiDiv && idx64>0) {  \
				uiDiv = dimItem.x;  \
				idx64--;  \
			}  \
			uiDiv -= (uiDiv>0?1u:0);  \
		}
		
#define RTGEN_I2P_LOOP64_END  \
	}

#define RTGEN_I2P_LOOP32_BEGIN  \
	dimItem.z = 0;  \
	for(idx = (unsigned int)idx64; ii < PLAIN_MAX_SIZE && !(dimItem.z&512); ii++, dimVecIdx++) {  \
		uiVal = idx;  \
		dimItem.x = (cplDimVec[dimVecIdx].x&0xffffu);  \
		dimItem.y = cplDimVec[dimVecIdx].y;  \
		dimItem.z = (cplDimVec[dimVecIdx].x>>16);  \
		\
		idx = __umulhi(uiVal, dimItem.y);  \
		uiDiv = uiVal - idx*dimItem.x;  \
		if(uiDiv >= dimItem.x) {  \
			uiDiv -= dimItem.x;  \
			idx++;  \
		}  \
		\
		if(dimItem.z&256) {  \
			if(!uiDiv && !idx) {  \
				dimVecIdx++;  \
				break;  \
			}  \
			if(!uiDiv && idx>0) {  \
				uiDiv = dimItem.x;  \
				idx--;  \
			}  \
			uiDiv -= (uiDiv>0?1u:0);  \
		}
			
#define RTGEN_I2P_LOOP32_END  \
	}


/***********   PreCalculate  **************/

#define RTPRECALC_PROLOGUE  \
	unsigned int *hData;  \
	unsigned int idx;  \
	uint64_t idx64;  \
	unsigned int nPos, ii;  \
	unsigned int startNPos;  \
	uint3 dimItem;  \
	uint64_t uiDiv64, uiVal64, uiMul64;  \
	uint64_t plainSpace, idx64mod;  \
	unsigned int uiVal, uiDiv;  \
	unsigned int dimVecIdx;  \
	\
	__shared__ unsigned int shData[SHIDX(BLOCK_X_SIZE)];  \
	\
	if(threadIdx.x == 0) {  \
		nPos = THREADIDX;  \
		for(ii = 0; ii < BLOCK_X_SIZE; ii++, nPos++) {  \
			hData = shData + DATAIDX(4, ii);  \
			hData[SHIDX(0)] = dataHeap[nPos];  \
			hData[SHIDX(1)] = dataHeap[nPos]>>32;  \
			hData[SHIDX(2)] = (unsigned int)cplStartIdx + nPos + 1;  \
		}  \
	}  \
	__syncthreads();  \
	\
	hData = shData + DATAIDX(4, threadIdx.x);  \
	\
	idx64 = hData[SHIDX(1)];  \
	idx64 = (idx64<<32) | hData[SHIDX(0)];  \
	startNPos = hData[SHIDX(2)];  \
	\
	for(nPos = (startNPos>chainStart? startNPos : chainStart); nPos < chainStop; nPos++) {

#define RTPRECALC_EPILOGUE  \
		idx64 = hData[SHIDX(1)];  \
		idx64 = (idx64<<32) | hData[SHIDX(0)];  \
		idx64 += reduceOffset + nPos;  \
		uiDiv64 = __umul64hi(idx64, rPlainSpaceTotal);  \
		idx64 -= uiDiv64*plainSpaceTotal;  \
		if(idx64 >= plainSpaceTotal)  \
			idx64 -= plainSpaceTotal;  \
		\
	}  \
	\
	hData[SHIDX(0)] = idx64;  \
	hData[SHIDX(1)] = idx64>>32;  \
	__syncthreads();  \
	\
	if(threadIdx.x == 0) {  \
		nPos = THREADIDX;  \
		for(ii = 0; ii < BLOCK_X_SIZE; ii++, nPos++) {  \
			hData = shData + DATAIDX(4, ii);  \
			dataHeap[nPos] = ((uint64_t)hData[SHIDX(1)]<<32)|(uint64_t)hData[SHIDX(0)];  \
		}  \
	}


/***********   CheckAlarm  **************/
#define  PARAMIDX_ALARM         0
#define  PARAMIDX_GUESSEDPOS    1

#define RTCHKALARM_PROLOGUE  \
	unsigned int *hData, *hDataX;  \
	unsigned int idx;  \
	uint64_t idx64, lastIdx64;  \
	unsigned int nPos, ii;  \
	unsigned int stopNPos;  \
	uint3 dimItem;  \
	uint64_t uiDiv64, uiVal64, uiMul64;  \
	uint64_t plainSpace, idx64mod;  \
	unsigned int uiVal, uiDiv;  \
	unsigned int dimVecIdx;  \
	\
	__shared__ unsigned int shData[SHIDX(BLOCK_X_SIZE)];  \
	__shared__ unsigned int shDataX[BLOCK_X_SIZE<<1];  \
	\
	if(threadIdx.x == 0) {  \
		nPos = (THREADIDX<<1);  \
		for(ii = 0; ii < BLOCK_X_SIZE; ii++, nPos+=2) {  \
			hData = shData + DATAIDX(4, ii);  \
			hDataX = shDataX + DATAIDX(1, ii);  \
			hData[SHIDX(0)] = dataHeap[nPos];  \
			hData[SHIDX(1)] = dataHeap[nPos]>>32;  \
			hDataX[SHIDX(PARAMIDX_ALARM)] = dataHeap[nPos+1]>>32;  \
			hDataX[SHIDX(PARAMIDX_GUESSEDPOS)] = dataHeap[nPos+1];  \
		}  \
	}  \
	__syncthreads();  \
	\
	hData = shData + DATAIDX(4, threadIdx.x);  \
	hDataX = shDataX + DATAIDX(1, threadIdx.x);  \
	\
	idx64 = hData[SHIDX(1)];  \
	idx64 = (idx64<<32) | hData[SHIDX(0)];  \
	stopNPos = hDataX[SHIDX(PARAMIDX_GUESSEDPOS)];  \
	if(chainStart > stopNPos || (stopNPos == 0xfffffffful && hDataX[SHIDX(PARAMIDX_ALARM)] == 0xfffffffful))  \
		return; \
	chainStop = (chainStop>stopNPos? stopNPos : chainStop); \
	\
	lastIdx64 = 0; \
	for(nPos = chainStart; nPos <= chainStop; nPos++) { \
		lastIdx64 = idx64;

#define RTCHKALARM_EPILOGUE  \
		idx64 = hData[SHIDX(1)];  \
		idx64 = (idx64<<32) | hData[SHIDX(0)];  \
		idx64 += reduceOffset + nPos;  \
		uiDiv64 = __umul64hi(idx64, rPlainSpaceTotal);  \
		idx64 -= uiDiv64*plainSpaceTotal;  \
		if(idx64 >= plainSpaceTotal)  \
			idx64 -= plainSpaceTotal;  \
		\
	}  \
	\
	if(nPos <= stopNPos) {  \
		hData[SHIDX(0)] = idx64;  \
		hData[SHIDX(1)] = idx64>>32;  \
		hDataX[SHIDX(PARAMIDX_ALARM)] = 0;  \
	} else {  \
		uiVal = 1;  \
		for(nPos = 0; nPos < cplTargetHashSize; nPos++)  \
			uiVal = (cplTargetHash[nPos]==hData[SHIDX(nPos)]? uiVal : 0);  \
		hData[SHIDX(0)] = lastIdx64;  \
		hData[SHIDX(1)] = lastIdx64>>32;  \
		hDataX[SHIDX(PARAMIDX_ALARM)] = uiVal;  \
	}  \
	__syncthreads();  \
	\
	if(threadIdx.x == 0) {  \
		nPos = (THREADIDX<<1);  \
		for(ii = 0; ii < BLOCK_X_SIZE; ii++, nPos+=2) {  \
			hData = shData + DATAIDX(4, ii);  \
			hDataX = shDataX + DATAIDX(1, ii);  \
			dataHeap[nPos] = ((uint64_t)hData[SHIDX(1)]<<32)|(uint64_t)hData[SHIDX(0)];  \
			if(hDataX[SHIDX(PARAMIDX_ALARM)]) {  \
				dataHeap[nPos+1] = 0xffffffffffffffffull;  \
				termKernel = 1;  \
			}  \
		}  \
	}


#include "rcuda_md5.inc"
#include "rcuda_md4.inc"
#include "rcuda_ntlm.inc"
#include "rcuda_sha1.inc"
#include "rcuda_lm.inc"

extern "C" int CalcChainsOnCUDA(const rcuda::RCudaTask* task, uint64_t *resultBuff) {
	cudaSetDeviceFlags(cudaDeviceBlockingSync);

	cudaError_t cuErr;
	uint64_t *data;
	int *cpPos;
	uint64_t uiVal64;

	if(task->charSetSize > CHAR_SET_MAXLEN)
		return -1;

	switch(task->hash) {
	case rcuda::RHASH_MD5:
	case rcuda::RHASH_MD4:
	case rcuda::RHASH_NTLM:
	case rcuda::RHASH_SHA1:
	case rcuda::RHASH_MYSQLSHA1:
	case rcuda::RHASH_LM:
		break;
	default:	
		return 0;
	}
	
	cudaMalloc((void**)&data, (task->idxCount+BLOCK_X_SIZE)*2*sizeof(uint64_t));
	cudaMalloc((void**)&cpPos, task->cpPosSize*sizeof(int));

	cudaMemcpy(data, resultBuff, task->idxCount*2*sizeof(uint64_t), cudaMemcpyHostToDevice);
	cudaMemcpy(cpPos, task->cpPositions, task->cpPosSize*sizeof(int), cudaMemcpyHostToDevice);

	cudaMemcpyToSymbol(dataHeap, &data, sizeof(data));
	cudaMemcpyToSymbol(cplDimVec, task->dimVec, min((unsigned long long)task->dimVecSize*sizeof(uint2), (unsigned long long)sizeof(cplDimVec)));
	cudaMemcpyToSymbol(cplChrSet, task->charSet, min(task->charSetSize, CHAR_SET_MAXLEN));
	cudaMemcpyToSymbol(plCpPos, &cpPos, sizeof(cpPos));
	cudaMemcpyToSymbol(plCpPosSize, &task->cpPosSize, sizeof(task->cpPosSize));
	cudaMemcpyToSymbol(reduceOffset, &task->reduceOffset, sizeof(task->reduceOffset));
	cudaMemcpyToSymbol(plainSpaceTotal, &task->plainSpaceTotal, sizeof(task->plainSpaceTotal));
	uiVal64 = (uint64_t)-1/task->plainSpaceTotal;
	cudaMemcpyToSymbol(rPlainSpaceTotal, &uiVal64, sizeof(uiVal64));

	int grSizeX = (task->idxCount-1)/BLOCK_X_SIZE + 1;
	int grSizeY = (grSizeX-1)/GRID_X_SIZE + 1;
	grSizeX = GRID_X_SIZE;
	dim3 numBlocks(grSizeX, grSizeY);
	cuErr = cudaSuccess;

	for(unsigned int idx = 0; idx < task->rainbowChainLen-1 && cuErr == cudaSuccess; idx+=task->kernChainSize) {
		switch(task->hash) {
		case rcuda::RHASH_MD5:
			RTGenMD5Kernel<<<numBlocks, BLOCK_X_SIZE>>>(idx, min(idx+task->kernChainSize, task->rainbowChainLen-1));
			break;
		case rcuda::RHASH_MD4:
			RTGenMD4Kernel<<<numBlocks, BLOCK_X_SIZE>>>(idx, min(idx+task->kernChainSize, task->rainbowChainLen-1));
			break;
		case rcuda::RHASH_NTLM:
			RTGenNTLMKernel<<<numBlocks, BLOCK_X_SIZE>>>(idx, min(idx+task->kernChainSize, task->rainbowChainLen-1));
			break;
		case rcuda::RHASH_SHA1:
			RTGenSHA1Kernel<<<numBlocks, BLOCK_X_SIZE>>>(idx, min(idx+task->kernChainSize, task->rainbowChainLen-1));
			break;
		case rcuda::RHASH_MYSQLSHA1:
			RTGenMySQLSHA1Kernel<<<numBlocks, BLOCK_X_SIZE>>>(idx, min(idx+task->kernChainSize, task->rainbowChainLen-1));
			break;
		case rcuda::RHASH_LM:
			RTGenLMKernel<<<numBlocks, BLOCK_X_SIZE>>>(idx, min(idx+task->kernChainSize, task->rainbowChainLen-1));
			break;
		case rcuda::RHASH_UNDEF:
			break;
		}

		cuErr = cudaThreadSynchronize();
	}

	if(cuErr == cudaSuccess)
		cudaMemcpy(resultBuff, data, task->idxCount*2*sizeof(uint64_t), cudaMemcpyDeviceToHost);
	else
		fprintf(stderr, "Error happened: %d (%s)\n", cuErr, cudaGetErrorString(cuErr));	

	if(task->cpPosSize > 0) {
		cudaFree(cpPos);
	}
	cudaFree(data);
	return cuErr==cudaSuccess? task->idxCount : -1;
}

extern "C" int PreCalculateOnCUDA(const rcuda::RCudaTask* task, uint64_t *resultBuff) {
	cudaSetDeviceFlags(cudaDeviceBlockingSync);

	cudaError_t cuErr;
	uint64_t *data;
	uint64_t uiVal64;

	if(task->charSetSize > CHAR_SET_MAXLEN)
		return -1;

	switch(task->hash) {
	case rcuda::RHASH_MD5:
	case rcuda::RHASH_MD4:
	case rcuda::RHASH_NTLM:
	case rcuda::RHASH_SHA1:
	case rcuda::RHASH_MYSQLSHA1:
	case rcuda::RHASH_LM:
		break;
	default:	
		return 0;
	}

	cudaMalloc((void**)&data, (task->idxCount+BLOCK_X_SIZE)*sizeof(uint64_t));
	cudaMemcpy(data, resultBuff, task->idxCount*sizeof(uint64_t), cudaMemcpyHostToDevice);

	cudaMemcpyToSymbol(dataHeap, &data, sizeof(data));
	cudaMemcpyToSymbol(cplDimVec, task->dimVec, min((unsigned long long)task->dimVecSize*sizeof(uint2),(unsigned long long)sizeof(cplDimVec)));
	cudaMemcpyToSymbol(cplChrSet, task->charSet, min(task->charSetSize, CHAR_SET_MAXLEN));
	cudaMemcpyToSymbol(reduceOffset, &task->reduceOffset, sizeof(task->reduceOffset));
	cudaMemcpyToSymbol(plainSpaceTotal, &task->plainSpaceTotal, sizeof(task->plainSpaceTotal));
	uiVal64 = (uint64_t)-1/task->plainSpaceTotal;
	cudaMemcpyToSymbol(rPlainSpaceTotal, &uiVal64, sizeof(uiVal64));
	cudaMemcpyToSymbol(cplStartIdx, &task->startIdx, sizeof(task->startIdx));
	
	int grSizeX = (task->idxCount-1)/BLOCK_X_SIZE + 1;
	int grSizeY = (grSizeX-1)/GRID_X_SIZE + 1;
	grSizeX = GRID_X_SIZE;
	dim3 numBlocks(grSizeX, grSizeY);
	cuErr = cudaSuccess;

	for(unsigned int idx = (unsigned int)task->startIdx; idx < task->rainbowChainLen-1 && cuErr == cudaSuccess; idx+=task->kernChainSize) {
		switch(task->hash) {
		case rcuda::RHASH_MD5:
			RTPreCalcMD5Kernel<<<numBlocks, BLOCK_X_SIZE>>>(idx, min(idx+task->kernChainSize, task->rainbowChainLen-1));
			break;
		case rcuda::RHASH_MD4:
			RTPreCalcMD4Kernel<<<numBlocks, BLOCK_X_SIZE>>>(idx, min(idx+task->kernChainSize, task->rainbowChainLen-1));
			break;
		case rcuda::RHASH_NTLM:
			RTPreCalcNTLMKernel<<<numBlocks, BLOCK_X_SIZE>>>(idx, min(idx+task->kernChainSize, task->rainbowChainLen-1));
			break;
		case rcuda::RHASH_SHA1:
			RTPreCalcSHA1Kernel<<<numBlocks, BLOCK_X_SIZE>>>(idx, min(idx+task->kernChainSize, task->rainbowChainLen-1));
			break;
		case rcuda::RHASH_MYSQLSHA1:
			RTPreCalcMySQLSHA1Kernel<<<numBlocks, BLOCK_X_SIZE>>>(idx, min(idx+task->kernChainSize, task->rainbowChainLen-1));
			break;
		case rcuda::RHASH_LM:
			RTPreCalcKernel<<<numBlocks, BLOCK_X_SIZE>>>(idx, min(idx+task->kernChainSize, task->rainbowChainLen-1));
			break;
		case rcuda::RHASH_UNDEF:
			break;
		}

		cuErr = cudaThreadSynchronize();
	}

	if(cuErr == cudaSuccess)
		cudaMemcpy(resultBuff, data, task->idxCount*sizeof(uint64_t), cudaMemcpyDeviceToHost);
	else
		fprintf(stderr, "Error happened: %d (%s)\n", cuErr, cudaGetErrorString(cuErr));	

	cudaFree(data);
	return cuErr==cudaSuccess? task->idxCount : -1;
}

extern "C" int CheckAlarmOnCUDA(const rcuda::RCudaTask* task, uint64_t *resultBuff) {
	cudaSetDeviceFlags(cudaDeviceBlockingSync);

	cudaError_t cuErr;
	uint64_t *data;
	uint64_t uiVal64;
	int isize;

	if(task->charSetSize > CHAR_SET_MAXLEN)
		return -1;

	isize = 0;
	switch(task->hash) {
	case rcuda::RHASH_MD5:
	case rcuda::RHASH_MD4:
	case rcuda::RHASH_NTLM:
		isize = (isize==0? 16 : isize);
	case rcuda::RHASH_SHA1:
	case rcuda::RHASH_MYSQLSHA1:
		isize = (isize==0? 20 : isize);
	case rcuda::RHASH_LM:
		isize = (isize==0? 8 : isize);
		cudaMemcpyToSymbol(cplTargetHash, task->targetHash, isize);
		isize = (isize>>2);
		cudaMemcpyToSymbol(cplTargetHashSize, &isize, sizeof(isize));
		break;
	default:	
		return 0;
	}

	cudaMalloc((void**)&data, (task->idxCount+BLOCK_X_SIZE)*2*sizeof(uint64_t));
	cudaMemcpy(data, resultBuff, task->idxCount*2*sizeof(uint64_t), cudaMemcpyHostToDevice);

	cudaMemcpyToSymbol(dataHeap, &data, sizeof(data));
	cudaMemcpyToSymbol(cplDimVec, task->dimVec, min((unsigned long long)task->dimVecSize*sizeof(uint2), (unsigned long long)sizeof(cplDimVec)));
	cudaMemcpyToSymbol(cplChrSet, task->charSet, min(task->charSetSize, CHAR_SET_MAXLEN));
	cudaMemcpyToSymbol(reduceOffset, &task->reduceOffset, sizeof(task->reduceOffset));
	cudaMemcpyToSymbol(plainSpaceTotal, &task->plainSpaceTotal, sizeof(task->plainSpaceTotal));
	uiVal64 = (uint64_t)-1/task->plainSpaceTotal;
	cudaMemcpyToSymbol(rPlainSpaceTotal, &uiVal64, sizeof(uiVal64));
	
	int grSizeX = (task->idxCount-1)/BLOCK_X_SIZE + 1;
	int grSizeY = (grSizeX-1)/GRID_X_SIZE + 1;
	grSizeX = GRID_X_SIZE;
	dim3 numBlocks(grSizeX, grSizeY);
	cuErr = cudaSuccess;

	isize = 0;
	cudaMemcpyToSymbol(termKernel, &isize, sizeof(isize));
	for(unsigned int idx = 0; idx <= task->rainbowChainLen && cuErr == cudaSuccess; idx+=task->kernChainSize) {
		switch(task->hash) {
		case rcuda::RHASH_MD5:
			RTCheckAlarmMD5Kernel<<<numBlocks, BLOCK_X_SIZE>>>(idx, min(idx+task->kernChainSize-1, task->rainbowChainLen));
			break;
		case rcuda::RHASH_MD4:
			RTCheckAlarmMD4Kernel<<<numBlocks, BLOCK_X_SIZE>>>(idx, min(idx+task->kernChainSize-1, task->rainbowChainLen));
			break;
		case rcuda::RHASH_NTLM:
			RTCheckAlarmNTLMKernel<<<numBlocks, BLOCK_X_SIZE>>>(idx, min(idx+task->kernChainSize-1, task->rainbowChainLen));
			break;
		case rcuda::RHASH_SHA1:
			RTCheckAlarmSHA1Kernel<<<numBlocks, BLOCK_X_SIZE>>>(idx, min(idx+task->kernChainSize-1, task->rainbowChainLen));
			break;
		case rcuda::RHASH_MYSQLSHA1:
			RTCheckAlarmMySQLSHA1Kernel<<<numBlocks, BLOCK_X_SIZE>>>(idx, min(idx+task->kernChainSize-1, task->rainbowChainLen));
			break;
		case rcuda::RHASH_LM:
			RTCheckAlarmLMKernel<<<numBlocks, BLOCK_X_SIZE>>>(idx, min(idx+task->kernChainSize-1, task->rainbowChainLen));
			break;
		case rcuda::RHASH_UNDEF:
			break;
		}

		cuErr = cudaThreadSynchronize();
		cudaMemcpyFromSymbol(&isize, termKernel, sizeof(isize));
		if(isize) 
			break;
	}

	if(cuErr == cudaSuccess)
		cudaMemcpy(resultBuff, data, task->idxCount*2*sizeof(uint64_t), cudaMemcpyDeviceToHost);
	else
		fprintf(stderr, "Error happened: %d (%s)\n", cuErr, cudaGetErrorString(cuErr));	

	cudaFree(data);
	return cuErr==cudaSuccess? task->idxCount : -1;
}

extern "C" int GetChainsBufferSize(int minSize) {
	int grSizeX = (minSize-1)/BLOCK_X_SIZE + 1;
	int grSizeY = (grSizeX-1)/GRID_X_SIZE + 1;
	grSizeX = GRID_X_SIZE;
	return grSizeX*grSizeY*BLOCK_X_SIZE;
}

extern "C" int SetCudaDevice(int device) {
	return cudaSetDevice(device);
}
