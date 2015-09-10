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

#ifndef _WIN32
#include <sys/time.h>
#endif

#include <sys/stat.h>
#include <string>
#include <vector>
#include <unistd.h>
#include <gtk/gtk.h>
#include "GUI_hash.h"
#include "GUI_settings.h"
#include "GUI_table.h"
#include "GUI_database.h"
#include "GUI.h"

Database db;
GUI_l gui;
MessageStack msgstack[256];
MessageStack plainstack[16];
TableDir *tabledir;
Settings settings;

/* Add message to message stack (use this function if you want to print something on the text view) */
void MessageStackPush(const char *tag, const char *fmt, ...)
{
	if (fmt == NULL)
		return;
	
	char text[256];
	va_list ap;
	
	va_start(ap, fmt);
	vsprintf(text, fmt, ap);	
	va_end(ap);
	
	strcpy(msgstack[db.msgiter].msg, text);
	strcpy(msgstack[db.msgiter].tag, tag);
	db.msgiter++;
}

void PlainStackPush(const char *plain)
{
	strcpy(plainstack[db.plainiter].msg, plain);
	strcpy(plainstack[db.plainiter].tag, "col-black");
	db.plainiter++;
}

/* Print text from Message stack */
gboolean PrintMessageStack(void *empty)
{
	int i, j;
	j = db.msgiter;
	
	if (j <= 0)
		return true;
	
	char cur_time[256];
	time_t Time;
	struct tm *tme;
	Time = time(0);
	tme = localtime(&Time);
	if (tme->tm_sec > 10)
		sprintf(cur_time, "[%d:%d:%d] ", tme->tm_hour, tme->tm_min, tme->tm_sec);
	else
		sprintf(cur_time, "[%d:%d:0%d] ", tme->tm_hour, tme->tm_min, tme->tm_sec);
	
	/* Print messages */
	for (i = 0; i < j; i++)
	{
		GtkTextIter enditer;
		GtkTextBuffer *buffer;
		GtkTextMark *mark;
		
		buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(gui.txtview.msg));
		gtk_text_buffer_get_end_iter(buffer, &enditer);
		gtk_text_buffer_insert_with_tags_by_name(buffer, &enditer, cur_time, -1, "col-grey", NULL);
		gtk_text_buffer_insert_with_tags_by_name(buffer, &enditer, msgstack[i].msg, -1, msgstack[i].tag, NULL);
		gtk_text_buffer_get_end_iter(buffer, &enditer);
		mark = gtk_text_buffer_get_insert(buffer);
		gtk_text_buffer_place_cursor(buffer, &enditer);
		gtk_text_view_scroll_to_mark(GTK_TEXT_VIEW(gui.txtview.msg), mark, 0.0, true, 0.0, 1.0);
		
		db.msgiter--;
	}
	
	/* Print plains */
	j = db.plainiter;
	for (i = 0; i < j; i++)
	{
		GtkTextIter enditer;
		GtkTextBuffer *buffer;
		GtkTextMark *mark;
		
		buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(gui.txtview.hashes));
		gtk_text_buffer_get_end_iter(buffer, &enditer);
		gtk_text_buffer_insert(buffer, &enditer, plainstack[i].msg, -1);
		gtk_text_buffer_get_end_iter(buffer, &enditer);
		mark = gtk_text_buffer_get_insert(buffer);
		gtk_text_buffer_place_cursor(buffer, &enditer);
		gtk_text_view_scroll_to_mark(GTK_TEXT_VIEW(gui.txtview.hashes), mark, 0.0, true, 0.0, 1.0);
		
		db.plainiter--;
	}
	return true;
}

gboolean GUIUpdateStatus(void *empty)
{
	gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR(gui.progbar), db.progress);
	gtk_progress_bar_set_text(GTK_PROGRESS_BAR(gui.progbar), db.progbartxt);
	
	gtk_entry_set_text(GTK_ENTRY(gui.entries.current), db.current_action);
	
	char buf[256];
	sprintf(buf, "%d / %d", db.n_found, db.n_hashes);
	gtk_entry_set_text(GTK_ENTRY(gui.entries.found), buf);
	
	if (db.status == STATUS_RUNNING)
	{		
		gettimeofday(&db.timer_stop, 0);
		int diff = db.timer_stop.tv_sec - db.timer_start.tv_sec;
		
		if (diff < 60)
			sprintf(buf, "00:00:%d", diff);
		else if (diff > 60)
			sprintf(buf, "00:%d:%d", diff/60, diff%60);
		else if (diff/60 > 60)
			sprintf(buf, "%d:%d:%d", diff/60/60, diff/60%60, diff%60);
		
		gtk_entry_set_text(GTK_ENTRY(gui.entries.time), buf);
		
		sprintf(buf, "%d / %d", db.n_found, db.n_hashes);
		gtk_entry_set_text(GTK_ENTRY(gui.entries.found), buf);
	}
	
	if (db.button_state == STATUS_RUNNING)
	{
		gtk_widget_set_sensitive(gui.buttons.run, false);
		gtk_widget_set_sensitive(gui.buttons.pause, true);
		gtk_widget_set_sensitive(gui.buttons.abort, true);
		db.button_state = -1;
	}
	
	else if (db.button_state == STATUS_IDLE)
	{
		if (gtk_widget_get_sensitive(gui.buttons.run) == false)
		{
			gtk_widget_set_sensitive(gui.buttons.run, true);
			gtk_widget_set_sensitive(gui.buttons.pause, false);
			gtk_widget_set_sensitive(gui.buttons.abort, false);
		}
		db.button_state = -1;
	}
	
	return true;
}

bool FileExists(const char *path)
{
	FILE *file;
	file = fopen(path, "rb");
	if (!file)
		return false;
	
	fclose(file);
	return true;
}

int main(int argc, char *argv[])
{
	if (argc == 2 && !strcmp(argv[1], "--debug"))
		db.debug = true;
	
	gtk_init(&argc, &argv);
	
	db.msgiter = db.plainiter = db.n_tables = db.n_folders = db.n_hashes = db.n_found = 0;
	db.status = STATUS_IDLE;
	db.progress = 0.0f;
	
	#ifndef _WIN32
	settings.n_threads = sysconf(_SC_NPROCESSORS_ONLN);
	settings.memlimit = sysconf(_SC_PHYS_PAGES) * sysconf(_SC_PAGE_SIZE)/1024/1024;
	#else
	SYSTEM_INFO sysinfo;
	GetSystemInfo(&sysinfo);
	settings.n_threads = sysinfo.dwNumberOfProcessors;
	MEMORYSTATUSEX memstat;
	memstat.dwLength = siezof(memstat);
	GlobalMemoryStatusEx(&memstat);
	settings.memlimit = memstat.ullTotalPhys;
	#endif
	
	if (db.debug)
		printf("MaxMem: %d - MaxThreads: %d\n", settings.memlimit, settings.n_threads);
	
	gui.hash.iter = (GtkTreeIter*)calloc(sizeof(GtkTreeIter), MAX_HASHES);
	gui.table.iter = (GtkTreeIter*)calloc(sizeof(GtkTreeIter), MAX_TABLES);

	strcpy(db.progbartxt, "Progress");
	strcpy(db.current_action, "Idle");
	
	#ifndef _WIN32
	char *home = getenv("HOME");
	strcpy(settings.file, home);
	strcat(settings.file, "/.rcracki/rcracki_mt-gui.conf");
	sprintf(settings.hashdb_file, "%s/.rcracki/hashes.db", home);
	
	if (!FileExists(settings.file))
	{
		char appdir[256];
		sprintf(appdir, "%s/.rcracki", home);
		if (mkdir(appdir, S_IRWXU) != 0)
			printf("Failed to create directory %s\n", appdir);
		
	}
	#else
	{
		char *appdata = getenv("APPDATA");
		strcpy(settings.file, appdata);
		strcat(settings.file, "/rcracki/rcracki_mt-gui.conf");
		if (!FileExists(settings.file))
		{
			char appdir[256], cmd[256];
			strcpy(appdir, appdata);
			strcat(appdir, "/rcracki");
			sprintf(cmd, "mkdir \"%s\"", appdir);
			system(cmd);
		}
	}
	#endif
	
	ReadSettingsFile();
	ReadHashDatabase();
	GUIShowMainWindow();
	SetupHashWindow();
	GUISetupTableWindow();
	
	g_timeout_add(50, PrintMessageStack, NULL);
	g_timeout_add(50, GUIUpdateStatus, NULL);
	
	gtk_main();
	
	return 0;
}
