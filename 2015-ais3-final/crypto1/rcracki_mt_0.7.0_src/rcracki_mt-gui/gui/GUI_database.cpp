/*
 * rcracki_mt-gui is a GTK GUI for rcracki_mt
 *
 * Copyright 2009, 2010, 2011, 2012 James Nobis <quel@quelrod.net>
 * Copyright 2010, 2011, 2012 by Paragon
 *
 * This file is part of rcracki_mt-gui.
 *
 * rcracki_mt-gui is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 2 of the License, or
 * (at your option) any later version.
 *
 * rcracki_mt-gui is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with rcracki_mt-gui.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <string>
#include <vector>
#include <gtk/gtk.h>
#include "GUI_database.h"
#include "GUI_hash.h"
#include "GUI.h"

extern Database db;
extern GUI_l gui;
extern Settings settings;

void SyncWithHashDatabase()
{
	if (db.n_hashdb_entries <= 0 || !settings.use_hashdb)
		return;
	
	int i, j;
	for (i = 0;  i < db.n_hashdb_entries; i++)
	{
		for (j = 0; j < (int)db.hashes.size(); j++)
		{
			if (!strcmp(db.hashes[j].c_str(), db.hashdb[i].hash))
			{
				MessageStackPush("col-green", "Plaintext of %s is %s\n", db.hashes[j].c_str(), db.hashdb[i].plain);
				char msg[256];
				sprintf(msg, "%s:%s\n", db.hashes[j].c_str(), db.hashdb[i].plain);
				PlainStackPush(msg);
				db.hashes.erase(db.hashes.begin() + j);
				db.n_hashes--;
				gtk_list_store_remove(gui.hash.store, &gui.hash.iter[j]);
				UpdateHashIters(j);
			}
		}
	}
}

void AppendToHashDatabase(const char *hash, const char *plain)
{
	if (!settings.use_hashdb)
		return;
	
	FILE *file;
	char db_path[256];
	
	char *home = getenv("HOME");
	sprintf(db_path, "%s/.rcracki/hashdb", home);
	
	file = fopen(db_path, "a");
	if (!file)
	{
		printf("Couldn't append to hash database\n");
		return;
	}
	
	fprintf(file, "%s %s\n", hash, plain);
	fclose(file);
	return;
}

void ReadHashDatabase()
{
	if (!settings.use_hashdb)
		return;
	
	FILE *file;
	char db_path[256], buffer[256];
	
	char *home = getenv("HOME");
	sprintf(db_path, "%s/.rcracki/hashdb", home);
	
	file = fopen(db_path, "rb");
	if (!file)
	{
		if (db.debug)
			MessageStackPush("col-grey", "%s does not exist.\n", db_path);
		
		file = fopen(db_path, "wb");
		fclose(file);
		
		db.hashdb = NULL;
		db.n_hashdb_entries = -1;
		return;
	}
	
	db.n_hashdb_entries = 0;
	while (!feof(file))
	{
		fgets(buffer, 256, file);
		db.n_hashdb_entries++;
	}
	rewind(file);
	
	db.hashdb = (HashDatabase*)malloc(sizeof(HashDatabase)*db.n_hashdb_entries);
	
	int i = 0;
	while (!feof(file))
	{
		fgets(buffer, 256, file);
		sscanf(buffer, "%s %s", db.hashdb[i].hash, db.hashdb[i].plain);
		i++;
	}
	fclose(file);
	
	if (db.debug || settings.verbose)
		MessageStackPush("col-darkgrey", "Read %d entries from hashdb\n", i);
	
	return;
}
