/*
 * freerainbowtables is a project for generating, distributing, and using
 * perfect rainbow tables
 *
 * Copyright (C) Zhu Shuanglei <shuanglei@hotmail.com>
 * Copyright 2009, 2010, 2011 Martin Westergaard Jørgensen <martinwj2005@gmail.com>
 * Copyright 2009, 2010 Daniël Niggebrugge <niggebrugge@fox-it.com>
 * Copyright 2009, 2010, 2011, 2012 James Nobis <quel@quelrod.net>
 * Copyright 2011 Janosch Rux <janosch.rux@web.de>
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
 *
 * Changes: not using OpenSSL routines the slow way anymore, as suggested by jci.
 */

#include "HashAlgorithm.h"

#include "Public.h"

#ifndef LOCALDES
#include <openssl/des.h>
#else
#include "des.h"
#endif
#ifndef GPUONLY
#include <openssl/sha.h>
#endif
//#include <openssl/ripemd.h>
#include "fast_md5.h"
#include "fast_md4.h"
//#include "sha1.h"
#if defined(_WIN32) && !defined(__GNUC__)
	#pragma comment(lib, "libeay32.lib")
#endif

#ifdef __NetBSD__
	#include <des.h>
#endif

#define MSCACHE_HASH_SIZE 16
void setup_des_key(unsigned char key_56[], des_key_schedule &ks)
{
	des_cblock key;

	key[0] = key_56[0];
	key[1] = (key_56[0] << 7) | (key_56[1] >> 1);
	key[2] = (key_56[1] << 6) | (key_56[2] >> 2);
	key[3] = (key_56[2] << 5) | (key_56[3] >> 3);
	key[4] = (key_56[3] << 4) | (key_56[4] >> 4);
	key[5] = (key_56[4] << 3) | (key_56[5] >> 5);
	key[6] = (key_56[5] << 2) | (key_56[6] >> 6);
	key[7] = (key_56[6] << 1);

	//des_set_odd_parity(&key);
	des_set_key(&key, ks);
}


//It doesnt convert to upper case that must be done before 
//otherwise it will produce a wrong a hash
void HashLM(unsigned char* pPlain, int nPlainLen, unsigned char* pHash)
{
	/*
	unsigned char data[7] = {0};
	memcpy(data, pPlain, nPlainLen > 7 ? 7 : nPlainLen);
	*/

	int i;
	for (i = nPlainLen; i < 7; i++)
		pPlain[i] = 0;

	static unsigned char magic[] = {0x4B, 0x47, 0x53, 0x21, 0x40, 0x23, 0x24, 0x25};
	des_key_schedule ks;
	//setup_des_key(data, ks);
	setup_des_key(pPlain, ks);
	des_ecb_encrypt((des_cblock*)magic, (des_cblock*)pHash, ks, DES_ENCRYPT);
}

void HashLMCHALL(unsigned char* pPlain, int nPlainLen, unsigned char* pHash)
{
	unsigned char pass[14];
	unsigned char pre_lmresp[21];
	static unsigned char magic[] = {0x4B, 0x47, 0x53, 0x21, 0x40, 0x23, 0x24, 0x25};
	static unsigned char spoofed_challange[] = {0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88}; 
	des_key_schedule ks;

	memset (pass,0,sizeof(pass));
	memset (pre_lmresp,0,sizeof(pre_lmresp));

	memcpy (pass,pPlain, nPlainLen);

	setup_des_key(pass, ks);
	des_ecb_encrypt((des_cblock*)magic, (des_cblock*)pre_lmresp, ks, DES_ENCRYPT);

	setup_des_key(&pass[7], ks);
	des_ecb_encrypt((des_cblock*)magic, (des_cblock*)&pre_lmresp[8], ks, DES_ENCRYPT);

	setup_des_key(pre_lmresp, ks);
	des_ecb_encrypt((des_cblock*)spoofed_challange, (des_cblock*)pHash, ks, DES_ENCRYPT);

	setup_des_key(&pre_lmresp[7], ks);
	des_ecb_encrypt((des_cblock*)spoofed_challange, (des_cblock*)&pHash[8], ks, DES_ENCRYPT);

	setup_des_key(&pre_lmresp[14], ks);
	des_ecb_encrypt((des_cblock*)spoofed_challange, (des_cblock*)&pHash[16], ks, DES_ENCRYPT);

} 

void HashHALFLMCHALL(unsigned char* pPlain, int nPlainLen, unsigned char* pHash)
{	
	unsigned char pre_lmresp[8];
	static unsigned char magic[] = {0x4B, 0x47, 0x53, 0x21, 0x40, 0x23, 0x24, 0x25};
	static unsigned char salt[] = {0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88};

	des_key_schedule ks;
	unsigned char plain[8] = {0};	
	memcpy(plain, pPlain, nPlainLen);
	setup_des_key(plain, ks);
	des_ecb_encrypt((des_cblock*)magic, (des_cblock*)pre_lmresp, ks, DES_ENCRYPT);

	setup_des_key(pre_lmresp, ks);
	des_ecb_encrypt((des_cblock*)salt, (des_cblock*)pHash, ks, DES_ENCRYPT);
} 



void HashNTLMCHALL(unsigned char* pPlain, int nPlainLen, unsigned char* pHash)
{
	unsigned char UnicodePlain[MAX_PLAIN_LEN * 2];
	static unsigned char spoofed_challange[] = {0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88}; 
	
	int len = (nPlainLen < 127) ? nPlainLen : 127;
	int i;
	
	for (i = 0; i < len; i++)
	{
	UnicodePlain[i * 2] = pPlain[i];
	UnicodePlain[i * 2 + 1] = 0x00;
	}
	
	des_key_schedule ks;
	unsigned char lm[21];
	
	/*MD4_CTX ctx;
	MD4_Init(&ctx);
	MD4_Update(&ctx, UnicodePlain, len * 2);
	MD4_Final(lm, &ctx);  */
	MD4_NEW(UnicodePlain, len * 2, lm);
	
	//MD4(UnicodePlain, len * 2, lm);
	lm[16] = lm[17] = lm[18] = lm[19] = lm[20] = 0;
	
	setup_des_key(lm, ks);
	des_ecb_encrypt((des_cblock*)spoofed_challange, (des_cblock*)pHash, ks, DES_ENCRYPT);
	
	setup_des_key(&lm[7], ks);
	des_ecb_encrypt((des_cblock*)spoofed_challange, (des_cblock*)&pHash[8], ks, DES_ENCRYPT);
	
	setup_des_key(&lm[14], ks);
	des_ecb_encrypt((des_cblock*)spoofed_challange, (des_cblock*)&pHash[16], ks, DES_ENCRYPT);
}

/*
void HashORACLE(unsigned char* pPlain, int nPlainLen, unsigned char* pHash)
{
	char ToEncrypt[256];
	char temp[256];
	char username[256];

	DES_cblock iv,iv2;
	DES_key_schedule ks1,ks2;
	unsigned char deskey_fixed[]={ 0x01, 0x23, 0x45, 0x67, 0x89, 0xab, 0xcd, 0xef};
	int i,j;
#if defined(_WIN32) && !defined(__GNUC__)
	strcpy_s(username, sizeof(username), "SYS");
#else
	strcpy(username, "SYS");
#endif
	int userlen = 3;
#if defined(_WIN32) && !defined(__GNUC__)
	_strupr((char*) pPlain);
#else
	strupr((char*) pPlain);
#endif
	memset (ToEncrypt,0,sizeof(ToEncrypt));

	for (i=1,j=0; j<userlen; i++,j++)
	{
		ToEncrypt[i] = username[j];
		i++;
	}

	for (j=0; j<nPlainLen; i++,j++)
	{
		ToEncrypt[i] = pPlain[j];
		i++;
	}

	i=i-1;
	memset (iv,0,8);
	memset (iv2,0,8);
	DES_set_key((DES_cblock*) deskey_fixed, &ks1);
	DES_ncbc_encrypt((unsigned char*) ToEncrypt, (unsigned char*) temp, i, &ks1, &iv, DES_ENCRYPT);
	DES_set_key((DES_cblock*) &iv, &ks2);
	DES_ncbc_encrypt((unsigned char*) ToEncrypt, (unsigned char*) temp, i, &ks2, &iv2, DES_ENCRYPT);
	memcpy (pHash,iv2,8);
}
*/

void HashNTLM(unsigned char* pPlain, int nPlainLen, unsigned char* pHash)
{
	unsigned char UnicodePlain[MAX_PLAIN_LEN * 2];
	int i;
	for (i = 0; i < nPlainLen; i++)
	{
		UnicodePlain[i * 2] = pPlain[i];
		UnicodePlain[i * 2 + 1] = 0x00;
	}

	MD4_NEW(UnicodePlain, nPlainLen * 2, pHash);
}

/*
void HashMD2(unsigned char* pPlain, int nPlainLen, unsigned char* pHash)
{
	MD2_CTX ctx;
	MD2_Init(&ctx);
	MD2_Update(&ctx, pPlain, nPlainLen);
	MD2_Final(pHash, &ctx);

	//MD2(pPlain, nPlainLen, pHash);
}
*/

void HashMD4(unsigned char* pPlain, int nPlainLen, unsigned char* pHash)
{
	MD4_NEW(pPlain, nPlainLen, pHash);
}

void HashMD5(unsigned char* pPlain, int nPlainLen, unsigned char* pHash)
{
	fast_MD5(pPlain, nPlainLen, pHash);
}
void HashDoubleMD5(unsigned char* pPlain, int nPlainLen, unsigned char* pHash)
{
	fast_MD5(pPlain, nPlainLen, pHash);

	unsigned char hash[MD5_DIGEST_LENGTH];
	memcpy(hash, pHash, MD5_DIGEST_LENGTH);

	fast_MD5(hash, MD5_DIGEST_LENGTH, pHash);
}

#ifndef GPUONLY
void HashSHA1(unsigned char* pPlain, int nPlainLen, unsigned char* pHash)
{
	SHA_CTX ctx;
	SHA1_Init(&ctx);
	SHA1_Update(&ctx, (unsigned char *) pPlain, nPlainLen);
	SHA1_Final(pHash, &ctx);
}
#else
void HashSHA1(unsigned char* pPlain, int nPlainLen, unsigned char* pHash) { }
#endif

/*
void HashRIPEMD160(unsigned char* pPlain, int nPlainLen, unsigned char* pHash)
{
	RIPEMD160_CTX ctx;
	RIPEMD160_Init(&ctx);
	RIPEMD160_Update(&ctx, pPlain, nPlainLen);
	RIPEMD160_Final(pHash, &ctx);  

	//RIPEMD160(pPlain, nPlainLen, pHash);
}
*/

/*
void HashMSCACHE(unsigned char *pPlain, int nPlainLen, unsigned char* pHash)
{
	char unicode_pwd[256];
	char unicode_user[256];
	static unsigned char username[] = "administrator";
	static int userlen = 13;
	unsigned char	final1[MD4_DIGEST_LENGTH];
	MD4_CTX ctx;
	int i;

//	strcpy (username, "administrator");
//	userlen = 13;

	for (i=0; i<nPlainLen; i++)
	{
		unicode_pwd[i*2] = pPlain[i];
		unicode_pwd[i*2+1] = 0x00;
	}

	for (i=0; i<userlen; i++)
	{
		unicode_user[i*2] = username[i];
		unicode_user[i*2+1] = 0x00;
	}

	MD4_NEW( (unsigned char*)unicode_pwd, nPlainLen*2, final1 );

	MD4_Init(&ctx);
	MD4_Update(&ctx,final1,MD4_DIGEST_LENGTH);
	MD4_Update(&ctx,(unsigned char*) unicode_user,userlen*2);
	MD4_Final(pHash,&ctx);

	/*
	unsigned char unicode_pwd[256];
	for (int i=0; i<nPlainLen; i++)
	{
		unicode_pwd[i*2] = pPlain[i];
		unicode_pwd[i*2+1] = 0x00;
	}*/
	/*
	unsigned char *buf = (unsigned char*)calloc(MSCACHE_HASH_SIZE + nSaltLength, sizeof(unsigned char));	
	HashNTLM(pPlain, nPlainLen, buf, NULL);
	//MD4(unicode_pwd, nPlainLen*2, buf);
	memcpy(buf + MSCACHE_HASH_SIZE, pSalt, nSaltLength);
	MD4(buf, MSCACHE_HASH_SIZE + nSaltLength, pHash); 
	free(buf);
	*/
//}

//*********************************************************************************
// Code for MySQL password hashing
//*********************************************************************************

inline void mysql_hash_password_323(unsigned long *result, const char *password) 
{
	register unsigned long nr=1345345333L, add=7, nr2=0x12345671L;
	unsigned long tmp;
	for (; *password ; password++) 
	{
		if (*password == ' ' || *password == '\t') continue;
		tmp= (unsigned long) (unsigned char) *password;
		nr^= (((nr & 63)+add)*tmp)+ (nr << 8);
		nr2+=(nr2 << 8) ^ nr;
		add+=tmp;
	}
	result[0]=nr & (((unsigned long) 1L << 31) -1L); // Don't use sign bit (str2int)
	result[1]=nr2 & (((unsigned long) 1L << 31) -1L);
	return;
}

void HashMySQL323(unsigned char* pPlain, int nPlainLen, unsigned char* pHash)
{
	unsigned long hash_pass[2];	
	unsigned char* f = (unsigned char*) hash_pass;

	unsigned char* pass = (unsigned char*) calloc (nPlainLen+4,sizeof(unsigned char));
	memcpy(pass,pPlain,nPlainLen);

	mysql_hash_password_323(hash_pass, (char*) pass);
	pHash[0]=*(f+3); pHash[1]=*(f+2); pHash[2]=*(f+1); pHash[3]=*(f+0);
	pHash[4]=*(f+7); pHash[5]=*(f+6); pHash[6]=*(f+5); pHash[7]=*(f+4);

	free (pass);
}

#ifndef GPUONLY
void HashMySQLSHA1(unsigned char* pPlain, int nPlainLen, unsigned char* pHash)
{
	unsigned char hash_stage1[SHA_DIGEST_LENGTH];
	SHA_CTX ctx;

	SHA1_Init(&ctx);
	SHA1_Update(&ctx, (unsigned char *) pPlain, nPlainLen);
	SHA1_Final(hash_stage1, &ctx);
	SHA1_Init(&ctx);
	SHA1_Update(&ctx, hash_stage1, SHA_DIGEST_LENGTH);
	SHA1_Final(pHash, &ctx);
}
#else
void HashMySQLSHA1(unsigned char* pPlain, int nPlainLen, unsigned char* pHash) {}
#endif

//*********************************************************************************
// Code for PIX password hashing
//*********************************************************************************
static char itoa64[] =          /* 0 ... 63 => ascii - 64 */
	"./0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";

void _crypt_to64(char *s, unsigned long v, int n)
{
	while (--n >= 0) {
		*s++ = itoa64[v&0x3f];
		v >>= 6;
	}
}

/*
void HashPIX(unsigned char* pPlain, int nPlainLen, unsigned char* pHash)
{
	char temp[MD5_DIGEST_LENGTH+1];
	unsigned char final[MD5_DIGEST_LENGTH];
	char* pass = (char*) calloc (nPlainLen+MD5_DIGEST_LENGTH,sizeof(char));

	memcpy (pass,pPlain,nPlainLen);

	fast_MD5((unsigned char *) pass, MD5_DIGEST_LENGTH, final);

	char* p = (char*) temp;
	_crypt_to64(p,*(unsigned long*) (final+0),4); p += 4;
	_crypt_to64(p,*(unsigned long*) (final+4),4); p += 4;
	_crypt_to64(p,*(unsigned long*) (final+8),4); p += 4;
	_crypt_to64(p,*(unsigned long*) (final+12),4); p += 4;
	*p=0;

	memcpy(pHash,temp,MD5_DIGEST_LENGTH);

	free (pass);
}
*/

#if !defined(_WIN32) || defined(__GNUC__)
char *strupr(char *s1)
{
	char *p = s1;
	while(*p)
	{
		*p = (char) toupper(*p);
		p++;
	}
	return s1;
}
#endif
