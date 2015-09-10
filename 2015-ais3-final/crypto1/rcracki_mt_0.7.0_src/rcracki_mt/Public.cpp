/*
 * freerainbowtables is a project for generating, distributing, and using
 * perfect rainbow tables
 *
 * Copyright (C) Zhu Shuanglei <shuanglei@hotmail.com>
 * Copyright Martin Westergaard Jørgensen <martinwj2005@gmail.com>
 * Copyright 2009, 2010 Daniël Niggebrugge <niggebrugge@fox-it.com>
 * Copyright 2009, 2010, 2011, 2012 James Nobis <quel@quelrod.net>
 * Copyright 2011 Logan Watt <logan.watt@gmail.com>
 * Copyright 2011 Karl Fox <karl@lithik.com>
 *
 * This file is part of freerainbowtables.
 *
 * freerainbowtables is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, version 2 of the License.
 *
 * freerainbowtables is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with freerainbowtables.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "Public.h"

#ifdef _WIN32
	#ifdef BOINC
		#include "boinc_win.h"
	#endif
#else
	#include <cstdio>
	#include <cctype>
	#include <ctime>
	#include <cstring>
	#include <cstdlib>
	#include <csignal>
	#include <unistd.h>
#endif

#include <sys/stat.h>
#include <algorithm>
#include <iostream>
#include <iterator>
#include <map>
#include <sstream>

#ifdef BOINC
	#include "filesys.h"
	#include "boinc_api.h"
#endif

#ifdef _WIN32
	#include <windows.h>
#endif

#if defined(_WIN32) && !defined(__GNUC__)
	#include <windows.h>
	#include <time.h>
	#include <io.h>

	#if defined(_MSC_VER) || defined(_MSC_EXTENSIONS)
		#define DELTA_EPOCH_IN_MICROSECS  11644473600000000Ui64
	#else
		#define DELTA_EPOCH_IN_MICROSECS  11644473600000000ULL
	#endif
 
	struct timezone
	{
		int tz_minuteswest; /* minutes W of Greenwich */
		int tz_dsttime;     /* type of dst correction */
	};
 
	int gettimeofday(struct timeval *tv, struct timezone *tz)
	{
		// Define a structure to receive the current Windows filetime
		FILETIME ft;
 
		// Initialize the present time to 0 and the timezone to UTC
  		unsigned __int64 tmpres = 0;
		static int tzflag = 0;
 
		if (NULL != tv)
		{
			GetSystemTimeAsFileTime(&ft);
 
			// The GetSystemTimeAsFileTime returns the number of 100 nanosecond 
			// intervals since Jan 1, 1601 in a structure. Copy the high bits to 
			// the 64 bit tmpres, shift it left by 32 then or in the low 32 bits.
			tmpres |= ft.dwHighDateTime;
			tmpres <<= 32;
			tmpres |= ft.dwLowDateTime;
 
			// Convert to microseconds by dividing by 10
			tmpres /= 10;
 
			// The Unix epoch starts on Jan 1 1970.  Need to subtract the difference 
			// in seconds from Jan 1 1601.
			tmpres -= DELTA_EPOCH_IN_MICROSECS;
	 
			// Finally change microseconds to seconds and place in the seconds value. 
			// The modulus picks up the microseconds.
			tv->tv_sec = (long)(tmpres / 1000000UL);
			tv->tv_usec = (long)(tmpres % 1000000UL);
		}
	 
		if (NULL != tz)
		{
			if (!tzflag)
			{
				_tzset();
				tzflag++;
			}
	  
			// Adjust for the timezone west of Greenwich
			tz->tz_minuteswest = _timezone / 60;
			tz->tz_dsttime = _daylight;
		}
	 
		return 0;
	}

#elif defined(__APPLE__) || \
	((defined(__unix__) || defined(unix)) && !defined(USG))

	#include <sys/param.h>

	#if defined(BSD)
		#include <sys/sysctl.h>
	#elif defined(__linux__) || defined(__sun__)
		#include <sys/sysinfo.h>
	#else
		#error Unsupported Operating System
	#endif
#endif

//////////////////////////////////////////////////////////////////////

enum RTfileFormatValue { RT
	, RTI
	, RTI2
};

static std::map<std::string, RTfileFormatValue> mapRTFileFormatValue;

static void initializeRTfileFormatMap()
{
	mapRTFileFormatValue["RT"] = RT;
	mapRTFileFormatValue["RTI"] = RTI;
	mapRTFileFormatValue["RTI2"] = RTI2;
}

uint8_t getRTfileFormatId( std::string RTfileFormatName )
{
	initializeRTfileFormatMap();
	std::map<std::string, RTfileFormatValue>::iterator iter;

	iter = mapRTFileFormatValue.find( RTfileFormatName );

	if ( iter == mapRTFileFormatValue.end() )
	{
		std::cout << "RT file format " << RTfileFormatName << " is not supported"
			<< std::endl;
		exit( 1 );
	}

	return iter->second;
}

timeval sub_timeofday( timeval tv2, timeval tv )
{
	timeval final;

	final.tv_usec = tv2.tv_usec - tv.tv_usec;
	final.tv_sec = tv2.tv_sec - tv.tv_sec;

	if ( final.tv_usec < 0 )
	{
		final.tv_usec += 1000000;
		--final.tv_sec;
	}

	return final;
}

/*
 * 32-bit this is a problem if the file is > (2^31-1) bytes
 * to get 64-bit behavior on 32 and 64 platforms:
 * for gcc add these before including sys/types.h and sys/stat.h:
 * 	#define __USE_LARGEFILE64
 * 	#define _LARGEFILE_SOURCE
 * 	#define _LARGEFILE64_SOURCE
 * 	then use stat64 instead of stat for the structure and the call
 * for VS
 * 	use _stat64 for the structure and stat64 for the call
 */
long GetFileLen( char* file )
{
	struct stat sb;

	if ( stat(file, &sb ) == -1 )
		return -1;

	return sb.st_size;
}

// 32-bit this is a problem if the file is > (2^31-1) bytes
long GetFileLen( std::string file )
{
	struct stat sb;

	if ( stat(file.c_str(), &sb ) == -1 )
		return -1;

	return sb.st_size;
}

/* 
 * In use by items that resolve boinc filenames
 * 1) boinc_ReadLinesFromFile in this file
 * 2) boinc_software/boinc_client_apps/distrrtgen/distrrtgen.cpp
 * 3) boinc_software/boinc_client_apps/distrrtgen_cuda/distrrtgen.cpp
 */
long GetFileLen(FILE* file)
{
	// XXX on x86/x86_64 linux returns long
	// 32-bit this is a problem if the file is > (2^31-1) bytes
	long pos = ftell(file);
	fseek(file, 0, SEEK_END);
	long len = ftell(file);
	fseek(file, pos, SEEK_SET);

	return len;
}

std::string TrimString( std::string s )
{
	while (s.size() > 0)
	{
		if (s[0] == ' ' || s[0] == '\t')
			s = s.substr(1);
		else
			break;
	}

	while (s.size() > 0)
	{
		if (s[s.size() - 1] == ' ' || s[s.size() - 1] == '\t')
			s = s.substr(0, s.size() - 1);
		else
			break;
	}

	return s;
}
bool GetHybridCharsets( std::string sCharset, std::vector<tCharset>& vCharset )
{
	// Example: hybrid(mixalpha-numeric-all-space#6-6,numeric#1-4)
	if(sCharset.substr(0, 6) != "hybrid") // Not hybrid charset
		return false;

	std::string::size_type nEnd = sCharset.rfind(')');
	std::string::size_type nStart = sCharset.rfind('(');
	std::string sChar = sCharset.substr(nStart + 1, nEnd - nStart - 1);
	std::string commas = "";

	int commaCount = std::count( sChar.begin(), sChar.end(), ',' );

	for ( int i = 0; i < commaCount; i++ )
		commas += ",";

	std::vector<std::string> vParts;

	if ( !SeperateString(sChar, commas, vParts) )
	{
		std::cout << "Failed to SeperateString: " << sChar << std::endl;
		return false;
	}

	for(uint32_t i = 0; i < vParts.size(); i++)
	{
		tCharset stCharset;
		std::vector<std::string> vParts2;

		if ( !SeperateString(vParts[i], "#", vParts2) )
		{
			std::cout << "Failed to SeperateString: " << vParts[i] << std::endl;
			return false;
		}

		stCharset.sName = vParts2[0];
		std::vector<std::string> vParts3;

		if( !SeperateString(vParts2[1], "-", vParts3) )
		{
			std::cout << "Failed to SeperateString: " << vParts2[1] << std::endl;
			return false;
		}

		stCharset.nPlainLenMin = atoi(vParts3[0].c_str());
		stCharset.nPlainLenMax = atoi(vParts3[1].c_str());

		vCharset.push_back(stCharset);
	}

	return true;
}
#ifdef BOINC
bool boinc_ReadLinesFromFile( std::string sPathName, std::vector<std::string>& vLine )
{
	vLine.clear();
#ifdef USE_INTEGRATED_CHARSET
	vLine.push_back("byte                        = []");
	vLine.push_back("alpha                       = [ABCDEFGHIJKLMNOPQRSTUVWXYZ]");
	vLine.push_back("alpha-space                 = [ABCDEFGHIJKLMNOPQRSTUVWXYZ ]");
	vLine.push_back("alpha-numeric               = [ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789]");
	vLine.push_back("alpha-numeric-space         = [ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789 ]");
	vLine.push_back("alpha-numeric-symbol14      = [ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789\x21\x40\x23\x24\x25\x5E\x26\x2A\x28\x29\x2D\x5F\x2B\x3D]");
	vLine.push_back("alpha-numeric-symbol14-space= [ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789\x21\x40\x23\x24\x25\x5E\x26\x2A\x28\x29\x2D\x5F\x2B\x3D\x20]");
	vLine.push_back("all                         = [ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789\x21\x40\x23\x24\x25\x5E\x26\x2A\x28\x29\x2D\x5F\x2B\x3D\x7E\x60\x5B\x5D\x7B\x7D\x7C\x5C\x3A\x3B\x22\x27\x3C\x3E\x2C\x2E\x3F\x2F]");
	vLine.push_back("all-space                   = [ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789\x21\x40\x23\x24\x25\x5E\x26\x2A\x28\x29\x2D\x5F\x2B\x3D\x7E\x60\x5B\x5D\x7B\x7D\x7C\x5C\x3A\x3B\x22\x27\x3C\x3E\x2C\x2E\x3F\x2F\x20]");
	vLine.push_back("alpha-numeric-symbol32-space = [ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789\x21\x40\x23\x24\x25\x5E\x26\x2A\x28\x29\x2D\x5F\x2B\x3D\x7E\x60\x5B\x5D\x7B\x7D\x7C\x5C\x3A\x3B\x22\x27\x3C\x3E\x2C\x2E\x3F\x2F\x20]");
	vLine.push_back("lm-frt-cp437                = [\x20\x21\x22\x23\x24\x25\x26\x27\x28\x29\x2A\x2B\x2C\x2D\x2E\x2F\x30\x31\x32\x33\x34\x35\x36\x37\x38\x39\x3A\x3B\x3C\x3D\x3E\x3F\x40\x41\x42\x43\x44\x45\x46\x47\x48\x49\x4A\x4B\x4C\x4D\x4E\x4F\x50\x51\x52\x53\x54\x55\x56\x57\x58\x59\x5A\x5B\x5C\x5D\x5E\x5F\x60\x7B\x7C\x7D\x7E\x80\x8E\x8F\x90\x92\x99\x9A\x9B\x9C\x9D\x9E\x9F\xA5\xE0\xE1\xE2\xE3\xE4\xE6\xE7\xE8\xE9\xEA\xEB\xEE]");
	vLine.push_back("lm-frt-cp850                = [\x20\x21\x22\x23\x24\x25\x26\x27\x28\x29\x2A\x2B\x2C\x2D\x2E\x2F\x30\x31\x32\x33\x34\x35\x36\x37\x38\x39\x3A\x3B\x3C\x3D\x3E\x3F\x40\x41\x42\x43\x44\x45\x46\x47\x48\x49\x4A\x4B\x4C\x4D\x4E\x4F\x50\x51\x52\x53\x54\x55\x56\x57\x58\x59\x5A\x5B\x5C\x5D\x5E\x5F\x60\x7B\x7C\x7D\x7E\x80\x8E\x8F\x90\x92\x99\x9A\x9C\x9D\x9F\xA5\xB5\xB6\xB7\xBD\xBE\xC7\xCF\xD1\xD2\xD3\xD4\xD5\xD6\xD7\xD8\xDE\xE0\xE1\xE2\xE3\xE5\xE6\xE8\xE9\xEA\xEB\xED\xEF]");
	vLine.push_back("lm-frt-cp437-850            = [\x20\x21\x22\x23\x24\x25\x26\x27\x28\x29\x2A\x2B\x2C\x2D\x2E\x2F\x30\x31\x32\x33\x34\x35\x36\x37\x38\x39\x3A\x3B\x3C\x3D\x3E\x3F\x40\x41\x42\x43\x44\x45\x46\x47\x48\x49\x4A\x4B\x4C\x4D\x4E\x4F\x50\x51\x52\x53\x54\x55\x56\x57\x58\x59\x5A\x5B\x5C\x5D\x5E\x5F\x60\x7B\x7C\x7D\x7E\x80\x8E\x8F\x90\x92\x99\x9A\x9B\x9C\x9D\x9E\x9F\xA5\xB5\xB6\xB7\xBD\xBE\xC7\xCF\xD1\xD2\xD3\xD4\xD5\xD6\xD7\xD8\xDE\xE0\xE1\xE2\xE3\xE4\xE5\xE6\xE7\xE8\xE9\xEA\xEB\xED\xEE\xEF]");
	vLine.push_back("numeric                     = [0123456789]");
	vLine.push_back("numeric-space               = [0123456789 ]");
	vLine.push_back("loweralpha                  = [abcdefghijklmnopqrstuvwxyz]");
	vLine.push_back("loweralpha-space            = [abcdefghijklmnopqrstuvwxyz ]");
	vLine.push_back("loweralpha-numeric          = [abcdefghijklmnopqrstuvwxyz0123456789]");
	vLine.push_back("loweralpha-numeric-space    = [abcdefghijklmnopqrstuvwxyz0123456789 ]");
	vLine.push_back("loweralpha-numeric-symbol14 = [abcdefghijklmnopqrstuvwxyz0123456789\x21\x40\x23\x24\x25\x5E\x26\x2A\x28\x29\x2D\x5F\x2B\x3D]");
	vLine.push_back("loweralpha-numeric-all      = [abcdefghijklmnopqrstuvwxyz0123456789\x21\x40\x23\x24\x25\x5E\x26\x2A\x28\x29\x2D\x5F\x2B\x3D\x7E\x60\x5B\x5D\x7B\x7D\x7C\x5C\x3A\x3B\x22\x27\x3C\x3E\x2C\x2E\x3F\x2F]");
	vLine.push_back("loweralpha-numeric-symbol32-space= [abcdefghijklmnopqrstuvwxyz0123456789\x21\x40\x23\x24\x25\x5E\x26\x2A\x28\x29\x2D\x5F\x2B\x3D\x7E\x60\x5B\x5D\x7B\x7D\x7C\x5C\x3A\x3B\x22\x27\x3C\x3E\x2C\x2E\x3F\x2F\x20]");
	vLine.push_back("mixalpha                    = [abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ]");
	vLine.push_back("mixalpha-space              = [abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ ]");
	vLine.push_back("mixalpha-numeric            = [abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789]");
	vLine.push_back("mixalpha-numeric-space      = [abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789 ]");
	vLine.push_back("mixalpha-numeric-symbol14   = [abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789\x21\x40\x23\x24\x25\x5E\x26\x2A\x28\x29\x2D\x5F\x2B\x3D]");
	vLine.push_back("mixalpha-numeric-all        = [abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789\x21\x40\x23\x24\x25\x5E\x26\x2A\x28\x29\x2D\x5F\x2B\x3D\x7E\x60\x5B\x5D\x7B\x7D\x7C\x5C\x3A\x3B\x22\x27\x3C\x3E\x2C\x2E\x3F\x2F]");
	vLine.push_back("mixalpha-numeric-symbol32-space  = [abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789\x21\x40\x23\x24\x25\x5E\x26\x2A\x28\x29\x2D\x5F\x2B\x3D\x7E\x60\x5B\x5D\x7B\x7D\x7C\x5C\x3A\x3B\x22\x27\x3C\x3E\x2C\x2E\x3F\x2F\x20]");
	vLine.push_back("mixalpha-numeric-all-space  = [abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789\x21\x40\x23\x24\x25\x5E\x26\x2A\x28\x29\x2D\x5F\x2B\x3D\x7E\x60\x5B\x5D\x7B\x7D\x7C\x5C\x3A\x3B\x22\x27\x3C\x3E\x2C\x2E\x3F\x2F\x20]");
#endif

	char input_path[512];
	boinc_resolve_filename(sPathName.c_str(), input_path, sizeof(input_path));
	FILE *file = boinc_fopen(input_path, "rb");
	if (!file) {
		fprintf(stderr,
			"Couldn't find input file, resolved name %s.\n", input_path
		);
		exit(-1);
	}

	if (file != NULL)
	{
		long len = GetFileLen(file);
		char* data = new char[len + 1];
		fread(data, 1, len, file);
		data[len] = '\0';
		std::string content = data;
		content += "\n";
		delete [] data;

		unsigned int i;
		for (i = 0; i < content.size(); i++)
		{
			if (content[i] == '\r')
				content[i] = '\n';
		}

		std::string::size_type n;
		while ((n = content.find("\n", 0)) != std::string::npos)
		{
			std::string line = content.substr(0, n);
			line = TrimString(line);
			if (line != "")
				vLine.push_back(line);
			content = content.substr(n + 1);
		}

		fclose(file);
	}
	else
		return false;

	return true;
}
#endif
bool ReadLinesFromFile( std::string sPathName, std::vector<std::string>& vLine )
{
	vLine.clear();

	FILE* file = fopen(sPathName.c_str(), "rb");
	if (file != NULL)
	{
		long len = GetFileLen( sPathName );
		char* data = new char[len + 1];
		fread(data, 1, len, file);
		data[len] = '\0';
		std::string content = data;
		content += "\n";
		delete [] data;

		unsigned int i;
		for (i = 0; i < content.size(); i++)
		{
			if (content[i] == '\r')
				content[i] = '\n';
		}

		std::string::size_type n;
		while ((n = content.find("\n", 0)) != std::string::npos)
		{
			std::string line = content.substr(0, n);
			line = TrimString(line);
			if (line != "")
				vLine.push_back(line);
			content = content.substr(n + 1);
		}

		fclose(file);
	}
	else
		return false;

	return true;
}

bool writeResultLineToFile( std::string sOutputFile, std::string sHash, std::string sPlain, std::string sBinary )
{
	FILE* file = fopen(sOutputFile.c_str(), "a");
	if (file!=NULL)
	{
		std::string buffer = sHash + ":" + sPlain + ":" + sBinary + "\n";
		fputs (buffer.c_str(), file);
		fclose (file);
		return true;
	}
	else
		return false;
}

bool SeperateString( std::string s, std::string sSeperator, std::vector<std::string>& vPart )
{
	vPart.clear();

	unsigned int i;
	for (i = 0; i < sSeperator.size(); i++)
	{
		std::string::size_type n;
		if ( (n = s.find(sSeperator[i])) != std::string::npos)
		{
			vPart.push_back(s.substr(0, n));
			s = s.substr(n + 1);
		}
		else
		{
			printf("not found: %c\n", sSeperator[i]);
			printf("s: %s\n", s.c_str());
			return false;
		}
	}
	vPart.push_back(s);

	return true;
}

std::string uint64tostr(uint64_t n)
{
	char str[32];

	sprintf(str, "%"PRIu64, n);

	return str;
}

std::string uint64tohexstr(uint64_t n)
{
	char str[32];

	//sprintf(str, "%016llx", n);
	sprintf(str, "%016"PRIx64, n);

	return str;
}

std::string HexToBinary( const char* data, int len  )
{
	std::string tmpData = std::string( data, len );
	std::string binary;

	for ( int i = 0; i <= len; i += 2 )
	{
		std::string dataPart = tmpData.substr( i, 2 );
		std::istringstream iss( dataPart );
		int j;

		iss >> std::hex >> j;
		binary += j;
	}
		
	return binary;
}

std::string HexToStr(const unsigned char* pData, int nLen)
{
	std::string sRet;
	int i;
	for (i = 0; i < nLen; i++)
	{
		char szByte[3];
		sprintf(szByte, "%02x", pData[i]);
		sRet += szByte;
	}

	return sRet;
}

unsigned long GetAvailPhysMemorySize()
{
#ifdef _WIN32
	MEMORYSTATUS ms;
	GlobalMemoryStatus(&ms);
	return ms.dwAvailPhys;
#elif defined(BSD)
	int mib[2] = { CTL_HW, HW_PHYSMEM };
	uint64_t physMem;
	//XXX warning size_t isn't portable
	size_t len;
	len = sizeof(physMem);
	sysctl(mib, 2, &physMem, &len, NULL, 0);
	return physMem;
#elif defined(__linux__)
	FILE *procfd = NULL;

	if ( (procfd = fopen("/proc/meminfo", "r")) != NULL )
	{
		char result[256]={0};
		char *tmp = NULL;
		unsigned int cachedram = 0, freeram = 0, bufferram = 0;
		uint64_t tempram = 0;

		while( fgets(result,sizeof(char)*256,procfd) != NULL )
		{
			tmp = strtok(result, " ");
			if( (strncmp(tmp,"MemFree:" , 8)) == 0 )
			{
				tmp = strtok(NULL, " ");
				freeram = atoi(tmp);
			}
			else if( (strncmp(tmp, "Buffers:", 8)) == 0 )
			{
				tmp = strtok(NULL, " ");
				bufferram = atoi(tmp);
			}
			else if( (strncmp(tmp, "Cached:", 7)) == 0 )
			{
				tmp = strtok(NULL, " ");
				cachedram = atoi(tmp);
				/*
				 * in 2.6 and 3.0 kernels the order is maintained and this is the
				 * last value to read.  Break and don't read more lines
				 */
				break;
			}
		}

		fclose(procfd);

		tempram = (uint64_t)(freeram + bufferram + cachedram) * 1024;

		if ( sizeof(long) == 4 )
		{
			// ensure that we don't return more than 2^31-1 on 32-bit platforms
			if ( tempram > 0x7FFFFFFFLLU )
				return (unsigned long) 0x7FFFFFFFLLU;
			else
				return (unsigned long) tempram;
		}
		
		return tempram;
	}

	struct sysinfo info;
	sysinfo(&info);
	return ( info.freeram + info.bufferram ) * (unsigned long) info.mem_unit;
#elif defined(__sun__)
	return ((unsigned long)sysconf(_SC_AVPHYS_PAGES) * (unsigned long)sysconf(_SC_PAGESIZE));
#else
	return 0;
	#error Unsupported Operating System
#endif
}

std::string GetApplicationPath()
{
	char fullPath[FILENAME_MAX];

#ifdef _WIN32
	GetModuleFileName(NULL, fullPath, FILENAME_MAX);
#else
	char szTmp[32];
	// XXX linux/proc file system dependent
	sprintf(szTmp, "/proc/%d/exe", (int)getpid());
	int bytes = readlink(szTmp, fullPath, FILENAME_MAX);

	if( bytes >= 0 )
		fullPath[bytes] = '\0';
#endif

	std::string sApplicationPath = fullPath;
#ifdef _WIN32
	std::string::size_type nIndex = sApplicationPath.find_last_of('\\');
#else
	std::string::size_type nIndex = sApplicationPath.find_last_of('/');
#endif

	if ( nIndex != std::string::npos )
		sApplicationPath = sApplicationPath.substr(0, nIndex+1);

	return sApplicationPath;
}

void ParseHash( std::string sHash, unsigned char* pHash, int& nHashLen )
{
	uint32_t i;
	for (i = 0; i < sHash.size() / 2; i++)
	{
		std::string sSub = sHash.substr(i * 2, 2);
		unsigned int nValue;
		sscanf(sSub.c_str(), "%02x", &nValue);
		pHash[i] = (unsigned char)nValue;
	}

	nHashLen = (int) sHash.size() / 2;
}

void Logo()
{
	std::cout	<< "RainbowCrack (improved, multi-threaded) - Making a Faster Cryptanalytic Time-Memory Trade-Off" << std::endl
			<< "by Martin Westergaard <martinwj2005@gmail.com>" << std::endl
			<< "multi-threaded and enhanced by neinbrucke" << std::endl
			<< "*nix/64-bit compatibility and co-maintainer - James Nobis <quel@quelrod.net>" << std::endl
			<< "http://www.freerainbowtables.com/" << std::endl
			<< "All code/binaries are under GPL2 Copyright at a minimum" << std::endl
			<< "original code by Zhu Shuanglei <shuanglei@hotmail.com>" << std::endl << std::endl;
}

// XXX nmap is GPL2, will check newer releases regarding license
// Code comes from nmap, used for the linux implementation of kbhit()
#ifndef _WIN32

static int tty_fd = 0;
struct termios saved_ti;

int tty_getchar()
{
	int c, numChars;

	if (tty_fd && tcgetpgrp(tty_fd) == getpid()) {
		c = 0;
		numChars = read(tty_fd, &c, 1);
		if (numChars > 0) return c;
	}

	return -1;
}

void tty_done()
{
	if (!tty_fd) return;

	tcsetattr(tty_fd, TCSANOW, &saved_ti);

	close(tty_fd);
	tty_fd = 0;
}

void tty_init()
{
	struct termios ti;

	if (tty_fd)
		return;

	if ((tty_fd = open("/dev/tty", O_RDONLY | O_NONBLOCK)) < 0) return;

	tcgetattr(tty_fd, &ti);
	saved_ti = ti;
	ti.c_lflag &= ~(ICANON | ECHO);
	ti.c_cc[VMIN] = 1;
	ti.c_cc[VTIME] = 0;
	tcsetattr(tty_fd, TCSANOW, &ti);

	atexit(tty_done);
}

void tty_flush(void)
{
	tcflush(tty_fd, TCIFLUSH);
}
// end nmap code
#endif
