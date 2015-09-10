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
#include <pthread.h>
#include <string>
#include <vector>
#include <sys/stat.h>
#include <sys/types.h>
#include <gtk/gtk.h>
#include "GUI_file.h"
#include "GUI_hash.h"
#include "GUI_table.h"
#include "Public.h"
#include "RainbowCrack.h"
#include "GUI.h"

#ifndef _WIN32
#include <unistd.h>
#include <dirent.h>
#endif

extern Database db;
extern GUI_l gui;
extern TableDir *tabledir;
extern Settings settings;

void GetHashList(const char *filename)
{
	std::vector<std::string>vLine;
	ReadLinesFromFile((std::string)filename, vLine);
	int i, len, max;
	
	if ((int)vLine.size() > MAX_HASHES)
		max = MAX_HASHES;
	else
		max = (int)vLine.size();
	
	for (i = 0; i < max; i++)
	{
		len = strlen(vLine[i].c_str());
		gtk_list_store_append(gui.hash.store, &gui.hash.iter[db.n_hashes]);
		if (len == 32)
			gtk_list_store_set(gui.hash.store, &gui.hash.iter[db.n_hashes], 0, "MD5 | NTLM", 1, len, 2, vLine[i].c_str(), -1);
		else if (len == 40)
			gtk_list_store_set(gui.hash.store, &gui.hash.iter[db.n_hashes], 0, "SHA1", 1, len, 2, vLine[i].c_str(), -1);
		
		db.hashes.push_back(vLine[i]);
		db.n_hashes++;
	}
	db.fCrackerType = true;
	
	if ((int)vLine.size() > MAX_HASHES)
		MessageStackPush("col-red", "Maximum amount of hashes is %d\n", MAX_HASHES);
	
}

/* Open one or more folders */
void OpenDir(GtkWidget *empty)
{
	GtkWidget *chooser;
	GSList *folders;
	std::string tmp;
	   
	chooser = gtk_file_chooser_dialog_new("Open", GTK_WINDOW(gui.windows.table), GTK_FILE_CHOOSER_ACTION_SELECT_FOLDER, GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL, GTK_STOCK_OPEN, GTK_RESPONSE_ACCEPT, NULL);
	gtk_file_chooser_set_select_multiple(GTK_FILE_CHOOSER(chooser), true);

	if (gtk_dialog_run(GTK_DIALOG(chooser)) == GTK_RESPONSE_ACCEPT)
	{
		folders = gtk_file_chooser_get_filenames(GTK_FILE_CHOOSER(chooser));
		GSList *list = folders;
		db.n_folders = 1;
		while (list->next != NULL)
		{
			list = list->next;
			db.n_folders++;
		}
		if (tabledir != NULL)
			free(tabledir);
		
		tabledir = (TableDir*)calloc(sizeof(TableDir), db.n_folders+1);
		
		list = folders;
		strcpy(tabledir[0].dir, (char*)list->data);
	
		tmp = tabledir[0].dir;
		int i = 1, k;
		while (list->next != NULL)
		{
			list = list->next;
			strcat(tabledir[i].dir, (char*)list->data);
			tmp += tabledir[i].dir;
			i++;
		}
		
		std::vector<std::string> old_tables = db.tables;
		
		for (i = 0; i < db.n_folders; i++)
			GetTableList((std::string)tabledir[i].dir, db.tables);

		/* Insert tables into list */
		int max;
		if ((int)db.tables.size() > MAX_TABLES)
			max = MAX_TABLES;
		else
			max = (int)db.tables.size();
		
		for (k = 0; k < max; k++)
		{
			if (TableAlreadyInList(db.tables[k].c_str(), old_tables))
			{
				db.tables.erase(db.tables.begin()+k);
				continue;
			}
			/* Get algorithm & charset */
			std::string algo, n;
			n = db.tables[k];
			for (i = 0; i < (int)n.length(); i++)
				n[i] = tolower(n[i]);
			
			size_t found = n.find("md5");
			if (found != std::string::npos)
				algo = "MD5";
			
			found = n.find("ntlm");
			if (found != std::string::npos)
				algo = "NTLM";
	
			found = n.find("lm");
			if (found != std::string::npos)
				algo = "LM";
			
			found = n.find("sha1");
			if (found != std::string::npos)
				algo = "SHA1";
			
			found = n.find("mysqlsha1");
			if (found != std::string::npos)
				algo = "MySQLSHA1";
			
			#ifndef _WIN32
			found = db.tables[k].find_last_of("/");
			#else
			found = db.tables[k].find_last_of("\\");
			#endif
			
			if (found == std::string::npos)
				found = 0;
			
			char path[256];
			strcpy(path, db.tables[k].substr(0, found).c_str());
			
			gtk_list_store_append(gui.table.store, &gui.table.iter[db.n_tables]);
			gtk_list_store_set(gui.table.store, &gui.table.iter[db.n_tables], 0, algo.c_str(), 1, db.tables[k].substr(found+1).c_str(), 2, path, -1);
			db.n_tables++;
		}
		if (max > (int)db.tables.size())
			MessageStackPush("col-red", "Maximum amount of rainbow table files is %d\n", MAX_TABLES);
	}
	
	gtk_widget_destroy(chooser);
}

void OpenHashlist()
{
	GtkWidget *chooser;
	   
	chooser = gtk_file_chooser_dialog_new("Open hashlist", GTK_WINDOW(gui.windows.hash), GTK_FILE_CHOOSER_ACTION_OPEN, GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL, GTK_STOCK_OPEN, GTK_RESPONSE_ACCEPT, NULL);
	gtk_file_chooser_set_select_multiple(GTK_FILE_CHOOSER(chooser), true);

	if (gtk_dialog_run(GTK_DIALOG(chooser)) == GTK_RESPONSE_ACCEPT)
	{
		const gchar *txt = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(chooser));
		GetHashList(txt);
	}
	gtk_widget_destroy(chooser);
}

void OpenPwdumpFile()
{
	GtkWidget *chooser;
	GtkFileFilter *filter;
	   
	chooser = gtk_file_chooser_dialog_new("Open pwdump", GTK_WINDOW(gui.windows.hash), GTK_FILE_CHOOSER_ACTION_OPEN, GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL, GTK_STOCK_OPEN, GTK_RESPONSE_ACCEPT, NULL);

	if (gtk_dialog_run(GTK_DIALOG(chooser)) == GTK_RESPONSE_ACCEPT)
	{
		const gchar *txt = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(chooser));
		std::vector<std::string> tmp, tmp2;
		LoadLMHashFromPwdumpFile(txt, tmp, db.hashes, db.hashes);
	}
	gtk_widget_destroy(chooser);
}

void OpenCainFile()
{
	GtkWidget *chooser;
	   
	chooser = gtk_file_chooser_dialog_new("Open cain file", GTK_WINDOW(gui.windows.hash), GTK_FILE_CHOOSER_ACTION_OPEN, GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL, GTK_STOCK_OPEN, GTK_RESPONSE_ACCEPT, NULL);

	if (gtk_dialog_run(GTK_DIALOG(chooser)) == GTK_RESPONSE_ACCEPT)
	{
		const gchar *txt = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(chooser));
		std::vector<std::string> tmp, tmp2;
		LoadLMHashFromCainLSTFile(txt, tmp, tmp2, db.hashes);
	}
	gtk_widget_destroy(chooser);
}

void BrowseSessionFile(GtkWidget *empty)
{
	GtkWidget *chooser;
	GtkFileFilter *filter;
	   
	chooser = gtk_file_chooser_dialog_new("Open session file", GTK_WINDOW(gui.windows.hash), GTK_FILE_CHOOSER_ACTION_OPEN, GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL, GTK_STOCK_OPEN, GTK_RESPONSE_ACCEPT, NULL);
	filter = gtk_file_filter_new();
	gtk_file_filter_set_name(filter, "Session files");
	gtk_file_filter_add_pattern(filter, "*.session");
	gtk_file_chooser_add_filter(GTK_FILE_CHOOSER(chooser), filter);
	
	filter = gtk_file_filter_new();
	gtk_file_filter_set_name(filter, "All files");
	gtk_file_filter_add_pattern(filter, "*");
	gtk_file_chooser_add_filter(GTK_FILE_CHOOSER(chooser), filter);
	
	if (gtk_dialog_run(GTK_DIALOG(chooser)) == GTK_RESPONSE_ACCEPT)
	{
		const gchar *txt = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(chooser));
		settings.resume_session = true;
		db.tables.clear();
		db.hashes.clear();
		strcpy(settings.sessionfile, txt);
		pthread_t thread[1];
		pthread_create(&thread[0], NULL, rcracki_mt, NULL);
	}
	gtk_widget_destroy(chooser);
}

void BrowseFile(GtkWidget *empty, void *data)
{
	GtkWidget *chooser;
	
	char *buffer = (char*)data;
	   
	chooser = gtk_file_chooser_dialog_new("Save as", GTK_WINDOW(gui.windows.hash), GTK_FILE_CHOOSER_ACTION_SAVE, GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL, GTK_STOCK_SAVE, GTK_RESPONSE_ACCEPT, NULL);

	if (gtk_dialog_run(GTK_DIALOG(chooser)) == GTK_RESPONSE_ACCEPT)
	{
		const gchar *txt = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(chooser));
		strcpy(buffer, txt);
	}
	gtk_widget_destroy(chooser);
}
