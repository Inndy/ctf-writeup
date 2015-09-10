/*
 * rcracki_mt is a multithreaded implementation and fork of the original 
 * RainbowCrack
 *
 * Copyright (C) Zhu Shuanglei <shuanglei@hotmail.com>
 * Copyright Martin Westergaard Jørgensen <martinwj2005@gmail.com>
 * Copyright 2009, 2010  Daniël Niggebrugge <niggebrugge@fox-it.com>
 * Copyright 2009 James Dickson
 * Copyright 2009, 2010, 2011, 2012 James Nobis <quel@quelrod.net>
 * Copyright 2010 uroskn
 * Copyright 2010, 2011, 2012 Paragon
 * Copyright 2011 Logan Watt <logan.watt@gmail.com>
 * Copyright 2011 Jan Kyska
 *
 * Modified by Martin Westergaard Jørgensen <martinwj2005@gmail.com> to support  * indexed and hybrid tables
 *
 * Modified by neinbrucke to support multi threading and a bunch of other stuff :)
 *
 * 2009-01-04 - <james.dickson@comhem.se> - Slightly modified (or "fulhack" as 
 * we say in sweden)  to support cain .lst files.
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

#include "Public.h"

#include "CrackEngine.h"
#include "lm2ntlm.h"

#ifdef GUI
	#include <gtk/gtk.h>
	#include "GUI.h"
	#include <algorithm>
	#include "RainbowCrack.h"

	extern GUI_l gui;
	extern Database db;
	extern Settings settings;
	extern TableDir *tabledir;
#endif

#ifdef _WIN32
	#include <io.h>
#else
	#include <unistd.h>
	#include <dirent.h>
#endif

#if defined(_WIN32) && !defined(__GNUC__)
	#pragma comment(lib, "libeay32.lib")
#endif

//////////////////////////////////////////////////////////////////////

#ifdef _WIN32
void GetTableList( std::string sWildCharPathName, std::vector<std::string>& vPathName )
{
	std::string sPath;
	std::string::size_type n = sWildCharPathName.find_last_of('\\');

	if ( n == (sWildCharPathName.size() - 1) )
	{
		sWildCharPathName = sWildCharPathName.substr(0, n);
		n = sWildCharPathName.find_last_of('\\');
	}

	if (n != std::string::npos)
		sPath = sWildCharPathName.substr(0, n + 1);

	_finddata_t fd;

	long handle = _findfirst(sWildCharPathName.c_str(), &fd);
	if (handle != -1)
	{
		do
		{
			std::string sName = fd.name;
			if (sName.size()>3) {
				if (sName.substr(sName.size()-3, 3) == ".rt" && !(fd.attrib & _A_SUBDIR))
				{
					std::string sPathName = sPath + sName;
					vPathName.push_back(sPathName);
				}
			}
			if (sName.size()>4) {
				if (sName.substr(sName.size()-4, 4) == ".rti" && !(fd.attrib & _A_SUBDIR))
				{
					std::string sPathName = sPath + sName;
					vPathName.push_back(sPathName);
				}
			}
			if (sName.size()>5) {
				if (sName.substr(sName.size()-5, 5) == ".rti2" && !(fd.attrib & _A_SUBDIR))
				{
					std::string sPathName = sPath + sName;
					vPathName.push_back(sPathName);
				}
			}

			if (sName != "." && sName != ".." && (fd.attrib & _A_SUBDIR))
			{
				std::string sPath_sub = sPath + sName + '\\';
				std::string sWildCharPathName_sub = sPath_sub + '*';
				GetTableList(sWildCharPathName_sub, vPathName);
			}

		} while (_findnext(handle, &fd) == 0);

		_findclose(handle);
	}
	//printf("Found %d rainbowtables (files) in %d sub directories...\n", vPathName.size(), subDir_count);
}
#else
//void GetTableList(int argc, char* argv[], vector<string>& vPathName)
void GetTableList( std::string sWildCharPathName, std::vector<std::string>& vPathName )
{
	struct stat buf;
	if (lstat(sWildCharPathName.c_str(), &buf) == 0)
	{
		if (S_ISDIR(buf.st_mode))
		{
			DIR *dir = opendir(sWildCharPathName.c_str());
			if(dir)
			{
				struct dirent *pDir=NULL;
				while((pDir = readdir(dir)) != NULL)
				{
					std::string filename = "";
					filename += (*pDir).d_name;
					if (filename != "." && filename != "..")
					{
						std::string new_filename = sWildCharPathName + '/' + filename;
						GetTableList(new_filename, vPathName);
					}
				}
				closedir(dir);
			}
		}
		else if (S_ISREG(buf.st_mode))
		{
			if (sWildCharPathName.size()>3)
			{
				if (sWildCharPathName.substr(sWildCharPathName.size()-3, 3) == ".rt")
				{
					vPathName.push_back(sWildCharPathName);
				}
			}
			if (sWildCharPathName.size()>4)
			{
				if (sWildCharPathName.substr(sWildCharPathName.size()-4, 4) == ".rti")
				{
					//string sPathName_sub = sPath_sub + sName_sub;
					vPathName.push_back(sWildCharPathName);
					//printf("sPathName_sub: %s\n", sPathName_sub.c_str());
				}
			}
			if ( sWildCharPathName.size() > 5 )
			{
				if ( sWildCharPathName.substr( sWildCharPathName.size() - 5, 5 ) == ".rti2" )
				{
					vPathName.push_back( sWildCharPathName );
				}
			}
		}
	}
}
#endif

bool NormalizeHash(std::string& sHash)
{
	std::string sNormalizedHash = sHash;

	if (   sNormalizedHash.size() % 2 != 0
		|| sNormalizedHash.size() < MIN_HASH_LEN * 2
		|| sNormalizedHash.size() > MAX_HASH_LEN * 2)
		return false;

	// Make lower
	uint32_t i;
	for (i = 0; i < sNormalizedHash.size(); i++)
	{
		if (sNormalizedHash[i] >= 'A' && sNormalizedHash[i] <= 'F')
			sNormalizedHash[i] = (char) sNormalizedHash[i] - 'A' + 'a';
	}

	// Character check
	for (i = 0; i < sNormalizedHash.size(); i++)
	{
		if (   (sNormalizedHash[i] < 'a' || sNormalizedHash[i] > 'f')
			&& (sNormalizedHash[i] < '0' || sNormalizedHash[i] > '9'))
			return false;
	}

	sHash = sNormalizedHash;
	return true;
}

void LoadLMHashFromPwdumpFile( std::string sPathName, std::vector<std::string>& vUserName, std::vector<std::string>& vLMHash, std::vector<std::string>& vNTLMHash )
{
	std::vector<std::string> vLine;
	if (ReadLinesFromFile(sPathName, vLine))
	{
		uint32_t i;
		for (i = 0; i < vLine.size(); i++)
		{
			std::vector<std::string> vPart;
			if (SeperateString(vLine[i], "::::", vPart))
			{
				std::string sUserName = vPart[0];
				std::string sLMHash   = vPart[2];
				std::string sNTLMHash = vPart[3];

				if (sLMHash.size() == 32 && sNTLMHash.size() == 32)
				{
					if (NormalizeHash(sLMHash) && NormalizeHash(sNTLMHash))
					{
						vUserName.push_back(sUserName);
						vLMHash.push_back(sLMHash);
						vNTLMHash.push_back(sNTLMHash);
#ifdef GUI
						gtk_list_store_append(gui.hash.store, &gui.hash.iter[db.n_hashes]);
						gtk_list_store_set(gui.hash.store, &gui.hash.iter[db.n_hashes], 0, "NTLM", 1, 32, 2, sNTLMHash.c_str(), -1);
						gtk_list_store_set(gui.hash.store, &gui.hash.iter[db.n_hashes], 0, "LM", 1, 32, 2, sLMHash.c_str(), -1);
						db.n_hashes++;
#endif
					}
					else
					{
#ifdef GUI
						MessageStackPush("col-grey", "Invalid lm/ntlm hash: %s:%s\n", sLMHash.c_str(), sNTLMHash.c_str());
#else
						std::cout << "invalid lm/ntlm hash " << sLMHash.c_str()
							<< ":" << sNTLMHash.c_str() << std::endl;
#endif
					}
				}
			}
		}
	}
	else
	{
#ifdef GUI
		MessageStackPush("col-red", "Can't open %s\n", sPathName.c_str());
#else
		std::cout << "can't open " << sPathName.c_str() << std::endl;
#endif
	}
}

// 2009-01-04 - james.dickson - Added this so we can load hashes from cain .LST files.
void LoadLMHashFromCainLSTFile( std::string sPathName, std::vector<std::string>& vUserName, std::vector<std::string>& vLMHash, std::vector<std::string>& vNTLMHash )
{
	std::vector<std::string> vLine;
	if (ReadLinesFromFile(sPathName, vLine))
	{
		uint32_t i;
		for (i = 0; i < vLine.size(); i++)
		{
			std::vector<std::string> vPart;
			if (SeperateString(vLine[i], "\t\t\t\t\t\t", vPart))
			{
				std::string sUserName = vPart[0];
				std::string sLMHash   = vPart[4];
				std::string sNTLMHash = vPart[5];

				if (sLMHash.size() == 32 && sNTLMHash.size() == 32)
				{
					if (NormalizeHash(sLMHash) && NormalizeHash(sNTLMHash))
					{
						vUserName.push_back(sUserName);
						vLMHash.push_back(sLMHash);
						vNTLMHash.push_back(sNTLMHash);
#ifdef GUI
						gtk_list_store_append(gui.hash.store, &gui.hash.iter[db.n_hashes]);
						gtk_list_store_set(gui.hash.store, &gui.hash.iter[db.n_hashes], 0, "NTLM", 1, 32, 2, sNTLMHash.c_str(), -1);
						db.n_hashes++;
						
						gtk_list_store_append(gui.hash.store, &gui.hash.iter[db.n_hashes]);
						gtk_list_store_set(gui.hash.store, &gui.hash.iter[db.n_hashes], 0, "LM", 1, 32, 2, sLMHash.c_str(), -1);
						db.n_hashes++;
#endif
					}
					else
					{
#ifdef GUI
						MessageStackPush("col-grey", "Invalid lm/ntlm hash %s:%s\n", sLMHash.c_str(), sNTLMHash.c_str());
#else
						std::cout << "invalid lm/ntlm hash " << sLMHash.c_str()
							<< ":" << sNTLMHash.c_str() << std::endl;
#endif
					}
				}
			}
		}
	}
	else
	{
#ifdef GUI
		MessageStackPush("col-red", "Can't open %s\n", sPathName.c_str());
#else
		std::cout << "can't open " << sPathName.c_str() << std::endl;
#endif
	}
}

bool NTLMPasswordSeek(unsigned char* pLMPassword, int nLMPasswordLen, int nLMPasswordNext,
					  unsigned char* pNTLMHash, std::string& sNTLMPassword)
{
	if (nLMPasswordNext == nLMPasswordLen)
	{
		unsigned char md[MD4_DIGEST_LENGTH];
		MD4_NEW(pLMPassword, nLMPasswordLen * 2, md);

		if (memcmp(md, pNTLMHash, MD4_DIGEST_LENGTH) == 0)
		{
			sNTLMPassword = "";
			int i;
			for (i = 0; i < nLMPasswordLen; i++)
				sNTLMPassword += char(pLMPassword[i * 2]);
			return true;
		}
		else
			return false;
	}

	if (NTLMPasswordSeek(pLMPassword, nLMPasswordLen, nLMPasswordNext + 1, pNTLMHash, sNTLMPassword))
		return true;

	if (   pLMPassword[nLMPasswordNext * 2] >= 'A'
		&& pLMPassword[nLMPasswordNext * 2] <= 'Z')
	{
		pLMPassword[nLMPasswordNext * 2] = (unsigned char) pLMPassword[nLMPasswordNext * 2] - 'A' + 'a';
		if (NTLMPasswordSeek(pLMPassword, nLMPasswordLen, nLMPasswordNext + 1, pNTLMHash, sNTLMPassword))
			return true;
		pLMPassword[nLMPasswordNext * 2] = (unsigned char) pLMPassword[nLMPasswordNext * 2] - 'a' + 'A';
	}

	return false;
}

bool LMPasswordCorrectCase( std::string sLMPassword, unsigned char* pNTLMHash, std::string& sNTLMPassword )
{
	if (sLMPassword.size() == 0)
	{
		sNTLMPassword = "";
		return true;
	}

	unsigned char* pLMPassword = new unsigned char[sLMPassword.size() * 2];
	uint32_t i;
	for (i = 0; i < sLMPassword.size(); i++)
	{
		pLMPassword[i * 2    ] = sLMPassword[i];
		pLMPassword[i * 2 + 1] = 0x00;
	}
	bool fRet = NTLMPasswordSeek(pLMPassword, sLMPassword.size(), 0, pNTLMHash, sNTLMPassword);

	delete pLMPassword;

	return fRet;
}

#ifndef GUI
void Usage()
{
	Logo();

	std::cout << "usage: rcracki_mt -h hash rainbow_table_pathname" << std::endl
		<< "       rcracki_mt -l hash_list_file rainbow_table_pathname"
		<< std::endl
		<< "       rcracki_mt -f pwdump_file rainbow_table_pathname" << std::endl
		<< "       rcracki_mt -c lst_file rainbow_table_pathname" << std::endl
		<< std::endl
		<< "-h hash:                use raw hash as input" << std::endl
		<< "-l hash_list_file:      use hash list file as input, each hash in a line"
		<< std::endl
		<< "-f pwdump_file:         use pwdump file as input, handles lanmanager hash only"
		<< std::endl
		<< "-c lst_file:            use .lst (cain format) file as input"
		<< std::endl
		<< "-r [-s session_name]:   resume from previous session, optional session name"
		<< std::endl
		<< "rainbow_table_pathname: pathname(s) of the rainbow table(s)"
		<< std::endl << std::endl
		<< "Extra options:    -t [nr] use this amount of threads/cores, default is 1"
		<< std::endl
		<< "                  -o [output_file] write (temporary) results to this file"
		<< std::endl
		<< "                  -s [session_name] write session data with this name"
		<< std::endl
		<< "                  -k keep precalculation on disk" << std::endl
		<< "                  -d run sha1 hashes against mysqlsha1 tables" << std::endl
		<< "                  -m [megabytes] limit memory usage" << std::endl
		<< "                  -v show debug information" << std::endl
#ifdef GPU
		<< "                  -g enable calculations on GPU" << std::endl
#endif
		<< std::endl;
#ifdef _WIN32
	std::cout << "example: rcracki_mt -h 5d41402abc4b2a76b9719d911017c592 -t 2 [path]\\MD5"
		<< std::endl
		<< "         rcracki_mt -l hash.txt [path_to_specific_table]\\*"
		<< std::endl;
#else
	std::cout << "example: rcracki_mt -h 5d41402abc4b2a76b9719d911017c592 -t 2 [path]/MD5"
		<< std::endl
		<< "         rcracki_mt -l hash.txt [path_to_specific_table]/*"
		<< std::endl;
#endif

	std::cout << "         rcracki_mt -f hash.txt -t 4 -o results.txt *.rti"
		<< std::endl;
}

int main(int argc, char* argv[])
{
	if (argc < 2)
	{
		Usage();
		return 0;
	}

	std::vector<std::string> vPathName;
	std::vector<std::string> vDefaultRainbowTablePath;
	std::string sWildCharPathName			= "";
	std::string sInputType				= "";
	std::string sInput				= "";
	std::string outputFile				= "";
	std::string sApplicationPath			= "";
	std::string sIniPathName			= "rcracki_mt.ini";
	bool writeOutput				= false;
	std::string sSessionPathName			= "rcracki.session";
	std::string sProgressPathName			= "rcracki.progress";
	std::string sPrecalcPathName			= "rcracki.precalc";
	bool resumeSession				= false;
	bool useDefaultRainbowTablePath			= false;
	bool debug					= false;
	bool keepPrecalcFiles				= false;
	bool runSha1AgainstMysqlSha1 = false;
	int enableGPU					= 0;
	std::string sAlgorithm				= "";
	int maxThreads					= 1;
	uint64_t maxMem					= 0;
	CHashSet hs;

	// Read defaults from ini file;
	bool readFromIni = false;
	std::vector<std::string> vLine;
	if (ReadLinesFromFile(sIniPathName, vLine)) {
		readFromIni = true;
	}
	else if (ReadLinesFromFile(GetApplicationPath() + sIniPathName, vLine)) {
		readFromIni = true;
	}
	if (readFromIni)
	{
		uint32_t i;
		for (i = 0; i < vLine.size(); i++)
		{
			if (vLine[i].substr(0,1) != "#")
			{
				std::vector<std::string> vPart;
				if (SeperateString(vLine[i], "=", vPart))
				{
					std::string sOption = vPart[0];
					std::string sValue  = vPart[1];
					
					if (sOption == "Threads") {
						maxThreads = atoi(sValue.c_str());
					}
					else if (sOption == "MaxMemoryUsage" ) {
						maxMem = atoi(sValue.c_str()) * 1024 *1024;
					}
					else if (sOption == "DefaultResultsFile") {
						outputFile = sValue;
					}
					else if (sOption == "AlwaysStoreResultsToFile") {
						if (sValue == "1")
							writeOutput = true;
					}
					else if (sOption.substr(0,24) == "DefaultRainbowTablePath.") {
						//printf("Default RT path: %s\n", sValue.c_str());
						vDefaultRainbowTablePath.push_back(vLine[i]);
					}
					else if (sOption == "DefaultAlgorithm") {
						useDefaultRainbowTablePath = true;
						sAlgorithm = sValue;
					}
					else if (sOption == "AlwaysDebug") {
						if (sValue == "1")
							debug = true;
					}
					else if (sOption == "EnableGPU") {
						if (sValue == "1")
							enableGPU = 1;
					}
					else if (sOption == "AlwaysKeepPrecalcFiles") {
						if (sValue == "1")
							keepPrecalcFiles = true;
					}
					else {
						std::cout << "illegal option " << sOption.c_str()
							<< " in ini file " << sIniPathName.c_str() << std::endl;
						return 0;
					}
				}
			}
		}
		if (writeOutput && outputFile == "")
		{
			std::cout << "You need to specify a 'DefaultResultsFile' "
				<< "with 'AlwaysStoreResultsToFile=1'" << std::endl;
			writeOutput = false;
		}
	}

	// Parse command line arguments
	int i;
	for (i = 1; i < argc; i++)
	{
		std::string cla = argv[i];
		if (cla == "-h") {
			sInputType = cla;
			i++;
			if (i < argc)
				sInput = argv[i];
		}
		else if (cla == "-l") {
			sInputType = cla;
			i++;
			if (i < argc)
				sInput = argv[i];
		}
		else if (cla == "-f") {
			sInputType = cla;
			i++;
			if (i < argc)
				sInput = argv[i];
		}
		else if (cla == "-c") {
			sInputType = cla;
			i++;
			if (i < argc)
				sInput = argv[i];
		}
		else if (cla == "-t") {
			i++;
			if (i < argc)
				maxThreads = atoi(argv[i]);
		}
		else if ( cla == "-d" ) {
			runSha1AgainstMysqlSha1 = true;
		}
		else if ( cla == "-m" ) {
			i++;
			if ( i < argc )
				maxMem = atoi(argv[i]) * 1024 * 1024;
		}
		else if (cla == "-o") {
			writeOutput = true;
			i++;
			if (i < argc)
				outputFile = argv[i];
		}
		else if (cla == "-r") {
			resumeSession = true;
		}
		else if (cla == "-s") {
			i++;
			if (i < argc)
			{
				sSessionPathName		=  argv[i];
				sSessionPathName		+= ".session";
				sProgressPathName		=  argv[i];
				sProgressPathName		+= ".progress";
				sPrecalcPathName		=  argv[i];
				sPrecalcPathName		+= ".precalc";
			}
		}
		else if (cla == "-v") {
			debug = true;
		}
		else if (cla == "-g") {
			enableGPU = 1;
		}
		else if (cla == "-k") {
			keepPrecalcFiles = true;
		}
		else if (cla == "-a") {
			useDefaultRainbowTablePath = true;
			i++;
			if (i < argc)
				sAlgorithm = argv[i];
		}
		else {
			GetTableList(cla, vPathName);
		}
	}

	if (debug && !readFromIni)
		std::cout << "Debug: Couldn't read rcracki_mt.ini, continuing anyway."
			<< std::endl;

	// Load session data if we are resuming
	if (resumeSession)
	{
		vPathName.clear();
		std::vector<std::string> sSessionData;
		if (ReadLinesFromFile(sSessionPathName.c_str(), sSessionData))
		{
			std::cout << "Resuming from session: " << sSessionPathName.c_str() << std::endl;
			uint32_t i;
			for (i = 0; i < sSessionData.size(); i++)
			{
				std::vector<std::string> vPart;
				if (SeperateString(sSessionData[i], "=", vPart))
				{
					std::string sOption = vPart[0];
					std::string sValue  = vPart[1];
					
					if (sOption == "sPathName") {
						vPathName.push_back(sValue);
					}
					else if (sOption == "sInputType") {
						sInputType = sValue;
					}
					else if (sOption == "sInput") {
						sInput = sValue;
					}
					else if (sOption == "outputFile") {
						writeOutput = true;
						outputFile = sValue;
					}
					else if (sOption == "keepPrecalcFiles") {
						if (sValue == "1")
							keepPrecalcFiles = true;
					}
				}
			}
		}
		else {
			std::cout << "Couldn't open session file " << sSessionPathName.c_str()
				<< std::endl;
			return 0;
		}
	}

	if (maxThreads<1)
		maxThreads = 1;

	// don't load these if we are resuming a session that already has a list of tables
	if (useDefaultRainbowTablePath && !resumeSession)
	{
		uint32_t i;
		for (i = 0; i < vDefaultRainbowTablePath.size(); i++)
		{
			std::vector<std::string> vPart;
			if (SeperateString(vDefaultRainbowTablePath[i], ".=", vPart))
			{
				std::string lineAlgorithm = vPart[1];
				std::string linePath = vPart[2];

				if (lineAlgorithm == sAlgorithm)
					GetTableList(linePath, vPathName);
			}
		}
	}

	std::cout << "Using " << maxThreads << " threads for pre-calculation "
		<< "and false alarm checking..." << std::endl;

	setvbuf(stdout, NULL, _IONBF,0);
	if (vPathName.size() == 0)
	{
		std::cout << "no rainbow table found" << std::endl;
		return 0;
	}

	std::cout << "Found " << vPathName.size() << " rainbowtable files..."
		<< std::endl << std::endl;

	bool fCrackerType;			// true: hash cracker, false: lm cracker
	std::vector<std::string> vHash;		// hash cracker
	std::vector<std::string> vUserName;	// lm cracker
	std::vector<std::string> vLMHash;	// lm cracker
	std::vector<std::string> vNTLMHash;	// lm cracker
	if (sInputType == "-h")
	{
		fCrackerType = true;

		std::string sHash = sInput;
		if (NormalizeHash(sHash))
			vHash.push_back(sHash);
		else
			std::cout << "invalid hash: " << sHash.c_str() << std::endl;
	}
	else if (sInputType == "-l")
	{
		fCrackerType = true;

		std::string sPathName = sInput;
		std::vector<std::string> vLine;
		if (ReadLinesFromFile(sPathName, vLine))
		{
			uint32_t i;
			for (i = 0; i < vLine.size(); i++)
			{
				std::string sHash = vLine[i];
				if (NormalizeHash(sHash))
					vHash.push_back(sHash);
				else
					std::cout << "invalid hash: " << sHash.c_str() << std::endl;
			}
		}
		else
			std::cout << "can't open " << sPathName.c_str() << std::endl;
	}
	else if (sInputType == "-f")
	{
		fCrackerType = false;

		std::string sPathName = sInput;
		LoadLMHashFromPwdumpFile(sPathName, vUserName, vLMHash, vNTLMHash);
	}
	else if (sInputType == "-c")
	{
		// 2009-01-04 - james.dickson - Added this for cain-files.
		fCrackerType = false;
		std::string sPathName = sInput;
		LoadLMHashFromCainLSTFile(sPathName, vUserName, vLMHash, vNTLMHash);
	}
	else
	{
		Usage();
		return 0;
	}

	if (fCrackerType && vHash.size() == 0)
	{
		std::cout << "no hashes found";
		return 0;
	}
	if (!fCrackerType && vLMHash.size() == 0)
	{
		std::cout << "no hashes found";
		return 0;
	}
	
	std::vector<std::string> sha1AsMysqlSha1;

	if (fCrackerType)
	{
		uint32_t i;
		for (i = 0; i < vHash.size(); i++)
		{
			if ( runSha1AgainstMysqlSha1 )
			{
				HASHROUTINE hashRoutine;
				CHashRoutine hr;
				std::string hashName = "sha1";
				int hashLen = 20;
				hr.GetHashRoutine( hashName, hashRoutine, hashLen );
				unsigned char* plain = new unsigned char[hashLen*2];
				memcpy( plain, HexToBinary(vHash[i].c_str(), hashLen*2 ).c_str(), hashLen );
				unsigned char hash_output[MAX_HASH_LEN];
				hashRoutine( plain, hashLen, hash_output);
				sha1AsMysqlSha1.push_back(HexToStr(hash_output, hashLen));
				hs.AddHash( sha1AsMysqlSha1[i] );
			}
			else
				hs.AddHash(vHash[i]);
		}
	}
	else
	{
		uint32_t i;
		for (i = 0; i < vLMHash.size(); i++)
		{
			hs.AddHash(vLMHash[i].substr(0, 16));
			hs.AddHash(vLMHash[i].substr(16, 16));
		}
	}

	// Load found hashes from session file
	if (resumeSession)
	{
		std::vector<std::string> sSessionData;
		if (ReadLinesFromFile(sSessionPathName.c_str(), sSessionData))
		{
			uint32_t i;
			for (i = 0; i < sSessionData.size(); i++)
			{
				std::vector<std::string> vPart;
				if (SeperateString(sSessionData[i], "=", vPart))
				{
					std::string sOption = vPart[0];
					std::string sValue  = vPart[1];
					
					if (sOption == "sHash") {
						std::vector<std::string> vPartHash;
						if (SeperateString(sValue, "::", vPartHash))
						{
							std::string sHash = vPartHash[0];
							std::string sBinary = vPartHash[1];
							std::string sPlain = vPartHash[2];
							
							hs.SetPlain(sHash, sPlain, sBinary);
						}
					}
				}
			}
		}
	}
	else
	{
		FILE* file;
		
		std::string buffer = "";

		// (Over)write session data if we are not resuming
		if ( ( file = fopen( sSessionPathName.c_str(), "w" ) ) != NULL )
		{
			buffer += "sInputType=" + sInputType + "\n";
			buffer += "sInput=" + sInput + "\n";

			uint32_t i;
			for (i = 0; i < vPathName.size(); i++)
			{
				buffer += "sPathName=" + vPathName[i] + "\n";
			}

			if (writeOutput)
				buffer += "outputFile=" + outputFile + "\n";

			if (keepPrecalcFiles)
				buffer += "keepPrecalcFiles=1\n";

			fputs (buffer.c_str(), file);
			fclose (file);
		}
		else
		{
			std::cout << "Error opening file " << sSessionPathName.c_str()
				<< ". Check that you have write permission to the directory "
				<< "that the application is run. Exiting Application." 
				<< std::endl;
			return  -1;
		}

		if( (file = fopen( sProgressPathName.c_str(), "w" )) == NULL )
		{
			std::cout << "Error opening file " << sProgressPathName.c_str()
				<< ". Check that you have write permission to the directory "
				<< "that the application is run. Exiting Application." 
				<< std::endl;
			return  -1;
		}
		else
			fclose( file );
	}

	// Run
	CCrackEngine ce;
	if (writeOutput)
		ce.setOutputFile(outputFile);
	ce.setSession(sSessionPathName, sProgressPathName, sPrecalcPathName, keepPrecalcFiles);
	ce.Run(vPathName, hs, maxThreads, maxMem, resumeSession, debug, enableGPU);

	// Remove session files
	if (debug)
		std::cout << "Debug: Removing session files." << std::endl;

	if (remove(sSessionPathName.c_str()) == 0)
		remove(sProgressPathName.c_str());
	else
	{
		if (debug)
			std::cout << "Debug: Failed removing session files." << std::endl;
	}

	// Statistics
	std::cout << "statistics" << std::endl
		<< "-------------------------------------------------------" << std::endl
		<< "plaintext found:                          " << hs.GetStatHashFound()
		<< " of " << hs.GetStatHashTotal()
		<< "(" << 100.0f * hs.GetStatHashFound() / hs.GetStatHashTotal() << "%)"
		<< std::endl << "total disk access time:                   "
		<< ce.GetStatTotalDiskAccessTime() << "s" << std::endl
		<< "total cryptanalysis time:                 "
		<< ce.GetStatTotalCryptanalysisTime() << "s" << std::endl
		<< "total pre-calculation time:               "
		<< ce.GetStatTotalPrecalculationTime() << "s" << std::endl
		<< "total chain walk step:                    "
		<< ce.GetStatTotalChainWalkStep() << std::endl
		<< "total false alarm:                        "
		<< ce.GetStatTotalFalseAlarm() << std::endl
		<< "total chain walk step due to false alarm: "
		<< ce.GetStatTotalChainWalkStepDueToFalseAlarm() << std::endl
		//<< "total chain walk step skipped due to checkpoints: " << ce.GetStatTotalFalseAlarmSkipped(); // Checkpoints not used - yet
		<< std::endl;

	// Result
	std::cout << "result" << std::endl
		<< "-------------------------------------------------------" << std::endl;

	if (fCrackerType)
	{
		uint32_t i;
		for (i = 0; i < vHash.size(); i++)
		{
			std::string sPlain, sBinary;
			std::string tmpHash = vHash[i];

			if ( runSha1AgainstMysqlSha1 )
			{
				tmpHash = sha1AsMysqlSha1[i];
			}

			if (!hs.GetPlain(tmpHash, sPlain, sBinary))
			{
				sPlain  = "<notfound>";
				sBinary = "<notfound>";
			}

			std::cout << vHash[i].c_str() << "\t" << sPlain.c_str() <<"\thex:"
				<< sBinary.c_str() << std::endl;
		}
	}
	else
	{
		uint32_t i;
		for (i = 0; i < vLMHash.size(); i++)
		{
			std::string sPlain1, sBinary1;
			bool fPart1Found = hs.GetPlain(vLMHash[i].substr(0, 16), sPlain1, sBinary1);
			if (!fPart1Found)
			{
				sPlain1  = "<notfound>";
				sBinary1 = "<notfound>";
			}

			std::string sPlain2, sBinary2;
			bool fPart2Found = hs.GetPlain(vLMHash[i].substr(16, 16), sPlain2, sBinary2);
			if (!fPart2Found)
			{
				sPlain2  = "<notfound>";
				sBinary2 = "<notfound>";
			}

			std::string sPlain = sPlain1 + sPlain2;
			std::string sBinary = sBinary1 + sBinary2;

			// Correct case
			if (fPart1Found && fPart2Found)
			{
				unsigned char NTLMHash[16];
				int nHashLen;
				ParseHash(vNTLMHash[i], NTLMHash, nHashLen);
				if (nHashLen != 16)
					std::cout << "debug: nHashLen mismatch" << std::endl;
				std::string sNTLMPassword;
				if (LMPasswordCorrectCase(sPlain, NTLMHash, sNTLMPassword))
				{
					sPlain = sNTLMPassword;
					sBinary = HexToStr((const unsigned char*)sNTLMPassword.c_str(), sNTLMPassword.size());
					if (writeOutput)
					{
						if (!writeResultLineToFile(outputFile, vNTLMHash[i].c_str(), sPlain.c_str(), sBinary.c_str()))
							std::cout << "Couldn't write final result to file!"
								<< std::endl;
					}
				}
				else
				{
					std::cout << vUserName[i].c_str() << "\t" << sPlain.c_str()
						<< "\thex:" << sBinary.c_str() << std::endl
						<< "Failed case correction, trying unicode correction for: "
						<< sPlain.c_str() << std::endl;

					LM2NTLMcorrector corrector;
					if (corrector.LMPasswordCorrectUnicode(sBinary, NTLMHash, sNTLMPassword))
					{
						sPlain = sNTLMPassword;
						sBinary = corrector.getBinary();
						if (writeOutput)
						{
							if (!writeResultLineToFile(outputFile, vNTLMHash[i].c_str(), sPlain.c_str(), sBinary.c_str()))
								std::cout << "Couldn't write final result to file!"
									<< std::endl;
						}
					}
					else {
						std::cout << "unicode correction for password "
							<< sPlain.c_str() << " failed!" << std::endl;
					}
				}
			}

			std::cout << vUserName[i].c_str() << "\t" << sPlain.c_str()
				<< "\thex:" << sBinary.c_str() << std::endl;
		}
	}

	return 0;
}
#else
void Exit(int c)
{
	pthread_exit((void*)(uint64_t)c);
}

void *rcracki_mt(void*)
{
	std::vector<std::string> vPathName;
	std::string outputFile						= settings.plainfile;
	bool writeOutput						= settings.save_plains;
	std::string sSessionPathName				= (std::string)settings.sessionfile + ".session";
	std::string sProgressPathName			= (std::string)settings.sessionfile + ".progress";
	std::string sPrecalcPathName				= (std::string)settings.sessionfile + ".precalc";
	bool resumeSession					= settings.resume_session;
	bool debug								= settings.debug;
	bool keepPrecalcFiles				= settings.keep_precalcs;
	std::string sAlgorithm						= "";
	int maxThreads							= settings.n_threads;
	uint64_t maxMem							= settings.memlimit * 1024 * 1024;
	CHashSet hs;
	std::vector<std::string> vHash;		// hash cracker
	std::vector<std::string> vUserName;	// lm cracker
	std::vector<std::string> vLMHash;		// lm cracker
	std::vector<std::string> vNTLMHash;	// lm cracker
	db.fCrackerType = true;

	// Load session data if we are resuming
	if (resumeSession)
	{
		std::vector<std::string> vPart;
		SeperateString(sSessionPathName, ".", vPart);
		sSessionPathName = vPart[0] + ".session";
		sProgressPathName = vPart[0] + ".progress";
		sPrecalcPathName = vPart[0] + ".precalc";
		vPathName.clear();
		vHash.clear();
		db.n_hashes = 0;
		db.n_found = 0;
		std::vector<std::string> sSessionData;
		if (ReadLinesFromFile(sSessionPathName.c_str(), sSessionData))
		{
			MessageStackPush("col-black", "Resuming from session %s\n", sSessionPathName.c_str());
			uint32_t i;
			for (i = 0; i < sSessionData.size(); i++)
			{
				std::vector<std::string> vPart;
				if (SeperateString(sSessionData[i], "=", vPart))
				{
					std::string sOption = vPart[0];
					std::string sValue  = vPart[1];
					
					if (sOption == "sPathName") {
						db.tables.push_back(sValue);
					}
					else if (sOption == "outputFile") {
						writeOutput = true;
						outputFile = sValue;
					}
					else if (sOption == "keepPrecalcFiles") {
						if (sValue == "1")
							keepPrecalcFiles = true;
					}
					else if (sOption == "sHash")
					{
						db.hashes.push_back(sValue);
						db.fCrackerType = true;
						db.n_hashes++;
					}
					else if (sOption == "sPlain")
					{
						std::vector<std::string> vPartHash;
						if (SeperateString(sValue, "::", vPartHash))
						{
							std::string sHash = vPartHash[0];
							std::string sBinary = vPartHash[1];
							std::string sPlain = vPartHash[2];
										
							hs.SetPlain(sHash, sPlain, sBinary);
							int j;
							for (j = 0; j < (int)db.hashes.size(); j++)
							{
								if (db.hashes[j] == sHash)
									db.hashes.erase(db.hashes.begin()+j);
							}
							PlainStackPush((sHash + ":" + sPlain).c_str());
							db.n_found++;
						}
					}
					else if (sOption == "LMCracker")
					{
						if (sValue == "true")
							db.fCrackerType = false;
						else
							db.fCrackerType = true;
					}
				}
			}
		}
		else 
		{
			MessageStackPush("col-red", "Couldn't open session file %s. Exiting.\n", sSessionPathName.c_str());
			Exit(0);
		}
	}
	
	vPathName = db.tables;
	vHash = db.hashes;

	if (maxThreads < 1)
		maxThreads = 1;
	
	setvbuf(stdout, NULL, _IONBF,0);
	if (vPathName.size() == 0)
	{
		MessageStackPush("col-red", "No rainbow table(s) found. Exiting.\n");
		Exit(0);
	}
	
	if (vHash.size() == 0)
	{
		MessageStackPush("col-red", "No hashes found. Exiting.\n");
		Exit(0);
	}
	
	if (db.fCrackerType)
	{
		uint32_t i;
		for (i = 0; i < vHash.size(); i++)
			hs.AddHash(vHash[i]);
	}
	else
	{
		uint32_t i;
		for (i = 0; i < vLMHash.size(); i++)
		{
			hs.AddHash(vLMHash[i].substr(0, 16));
			hs.AddHash(vLMHash[i].substr(16, 16));
		}
	}

	// (Over)write session data if we are not resuming
	if (!resumeSession && settings.save_session)
	{
		FILE* file = fopen(sSessionPathName.c_str(), "w");
		std::string buffer = "";

		if (file != NULL)
		{
			uint32_t i;
			for (i = 0; i < vPathName.size(); i++)
			{
				buffer += "sPathName=" + vPathName[i] + "\n";
			}

			if (writeOutput)
				buffer += "outputFile=" + outputFile + "\n";

			if (keepPrecalcFiles)
				buffer += "keepPrecalcFiles=1\n";
			
			for (i = 0; i < vHash.size(); i++)
				buffer += "sHash=" + vHash[i] + "\n";

			if (db.fCrackerType)
				buffer += "LMCracker=false\n";
			else
				buffer += "LMCracker=true\n";
			
			fputs (buffer.c_str(), file);
			fclose (file);
		}
		file = fopen(sProgressPathName.c_str(), "w");
		fclose (file);
	}
	
	MessageStackPush("col-black", "Using %d thread%s for pre-calculation and false alarm checking\n", maxThreads, maxThreads > 1 ? "s" : "");
	MessageStackPush("col-black", "Searching for %lu hash%s ...\n", (unsigned long)vHash.size(), vHash.size() > 1 ? "es" : "");
	
	strcpy(db.current_action, "Running");
	gettimeofday(&db.timer_start, 0);
	
	// Run
	db.status = STATUS_RUNNING;
	db.button_state = STATUS_RUNNING;

	CCrackEngine ce;
	if (writeOutput)
		ce.setOutputFile(outputFile);
	
	ce.setSession(sSessionPathName, sProgressPathName, sPrecalcPathName, keepPrecalcFiles);
	ce.Run(vPathName, hs, maxThreads, maxMem, resumeSession, debug);

	/* Remove session files*/
	if (!settings.save_session)
	{
		if (debug)
			MessageStackPush("col-grey", "Debug: Removing session files.\n");

		if (remove(sSessionPathName.c_str()) == 0)
			remove(sProgressPathName.c_str());
		else
			if (debug)
				MessageStackPush("col-grey", "Debug: Failed removing session files.\n");
		}

	// Statistics
	MessageStackPush("col-blue", "Statistics\n");
	MessageStackPush("col-black", "-------------------------------------------------------\n");
	MessageStackPush("col-black", "plaintext found:            %d of %d (%.2f%%)\n", hs.GetStatHashFound(),
															hs.GetStatHashTotal(),
															100.0f * hs.GetStatHashFound() / hs.GetStatHashTotal());
	MessageStackPush("col-black", "total disk access time:     %.2f s\n", ce.GetStatTotalDiskAccessTime());
	MessageStackPush("col-black", "total cryptanalysis time:   %.2f s\n", ce.GetStatTotalCryptanalysisTime());
	MessageStackPush("col-black", "total pre-calculation time: %.2f s\n", ce.GetStatTotalPrecalculationTime());
	MessageStackPush("col-black", "total chain walk step:      %d\n",     ce.GetStatTotalChainWalkStep());
	MessageStackPush("col-black", "total false alarm:          %d\n",     ce.GetStatTotalFalseAlarm());
	MessageStackPush("col-black", "total chain walk step due to false alarm: %d\n\n", ce.GetStatTotalChainWalkStepDueToFalseAlarm());

	// Result
	MessageStackPush("col-blue", "Result\n");
	MessageStackPush("col-black", "-------------------------------------------------------\n");
	
	if (db.fCrackerType)
	{
		uint32_t i;
		for (i = 0; i < vHash.size(); i++)
		{
			std::string sPlain, sBinary;
			if (!hs.GetPlain(vHash[i], sPlain, sBinary))
			{
				sPlain  = "<notfound>";
				sBinary = "<notfound>";
			}
			MessageStackPush("col-black", "%s:%s\n", vHash[i].c_str(), sPlain.c_str());
		}
	}
	else
	{
		uint32_t i;
		for (i = 0; i < vLMHash.size(); i++)
		{
			std::string sPlain1, sBinary1;
			bool fPart1Found = hs.GetPlain(vLMHash[i].substr(0, 16), sPlain1, sBinary1);
			if (!fPart1Found)
			{
				sPlain1  = "<notfound>";
				sBinary1 = "<notfound>";
			}

			std::string sPlain2, sBinary2;
			bool fPart2Found = hs.GetPlain(vLMHash[i].substr(16, 16), sPlain2, sBinary2);
			if (!fPart2Found)
			{
				sPlain2  = "<notfound>";
				sBinary2 = "<notfound>";
			}

			std::string sPlain = sPlain1 + sPlain2;
			std::string sBinary = sBinary1 + sBinary2;

			// Correct case
			if (fPart1Found && fPart2Found)
			{
				unsigned char NTLMHash[16];
				int nHashLen;
				ParseHash(vNTLMHash[i], NTLMHash, nHashLen);
				if (nHashLen != 16 && debug)
					MessageStackPush("col-grey", "Debug: nHashLen mismatch\n");
				std::string sNTLMPassword;
				if (LMPasswordCorrectCase(sPlain, NTLMHash, sNTLMPassword))
				{
					sPlain = sNTLMPassword;
					sBinary = HexToStr((const unsigned char*)sNTLMPassword.c_str(), sNTLMPassword.size());
				}
				else
				{
					MessageStackPush("col-black", "%-14s:%s\n", vUserName[i].c_str(), sPlain.c_str());
					LM2NTLMcorrector corrector;
					if (corrector.LMPasswordCorrectUnicode(sBinary, NTLMHash, sNTLMPassword))
					{
						sPlain = sNTLMPassword;
						sBinary = corrector.getBinary();
						if (writeOutput)
						{
							if (!writeResultLineToFile(outputFile, vNTLMHash[i].c_str(), sPlain.c_str(), sBinary.c_str()))
								MessageStackPush("col-grey", "Debug: Couldn't write final result to file!\n");
						}
					}
					else 
					{
						if (debug)
							MessageStackPush("col-grey", "Debug: case correction for password %s failed!\n", sPlain.c_str());
					}
				}
			}

			/* Display */
			MessageStackPush("col-black", "%-14s:%s\n", vUserName[i].c_str(), sPlain.c_str());
		}
	}
	
	db.status = STATUS_IDLE;
	db.button_state = STATUS_IDLE;
	
	MessageStackPush("col-green", "RainbowCrack done.\n\n");
	strcpy(db.current_action, "Idle");
	db.progress = 1.0f;
	strcpy(db.progbartxt, "Finished");
	
	pthread_exit(0);
}
#endif
