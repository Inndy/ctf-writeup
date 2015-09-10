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

#ifdef _WIN32
#include <windows.h>
#endif

#define STATUS_IDLE 0
#define STATUS_RUNNING 1
#define STATUS_PAUSE 2

#define HASH_MD5 0
#define HASH_NTLM 1
#define HASH_SHA1 2

#define MAX_TABLES 2048
#define MAX_HASHES 512

void GUIShowMainWindow();
void GUIShowHashWindow();
void GUIShowTableWindow();
void GUIShowSettingsWindow();
gboolean PrintMessageStack(void*);
void MessageStackPush(const char*, const char*, ...);
void PlainStackPush(const char*);

struct GUI_Buttons
{
	GtkWidget *run, *pause, *abort, *clear;
	GtkWidget *hash_load, *hash_delete, *hash_clear, *hash_ok;
	GtkWidget *table_load, *table_delete, *table_ok, *table_clear;
	GtkWidget *save_cracked, *session_file, *pref_ok, *hashdb;
};
typedef struct GUI_Buttons GUI_Buttons;

struct GUI_Windows
{
	GtkWidget *main, *hash, *table, *settings;
};
typedef struct GUI_Windows GUI_Windows;

struct GUI_Entries
{
	GtkWidget *current, *found, *time, *hash, *save_cracked, *session_file, *hashdb_file;
};
typedef struct GUI_Entries GUI_Entries;

struct GUI_Fixed
{
	GtkWidget *main, *hash, *table, *settings;
};
typedef struct GUI_Fixed GUI_Fixed;

struct GUI_ComboBox
{
	GtkWidget *hash;
};
typedef struct GUI_ComboBox GUI_ComboBox;

struct GUI_SpinButton
{
	GtkWidget *threads, *mem;
};
typedef struct GUI_SpinButton GUI_SpinButton;

struct GUI_CheckBox
{
	GtkWidget *precalc, *verbose, *save_cracked, *session, *hashdb;
};
typedef struct GUI_CheckBox GUI_CheckBox;

struct GUI_Menu
{
	GtkWidget *bar, *file, *filemenu, *tables, *hashes, *seperator, *quit, *settings, *settingsmenu, *preferences, *help, *helpmenu, *info, *session;
};
typedef struct GUI_Menu GUI_Menu;

struct GUI_Textview
{
	GtkWidget *msg, *hashes;
	GtkTextBuffer *msg_buffer, *hashes_buffer;
	GtkTextIter msg_iter, hashes_iter;
};
typedef struct GUI_Textview GUI_Textview;

struct GUI_TreeView
{
	GtkWidget *scroll, *tree;
	GtkCellRenderer *render;
	GtkListStore *store;
	GtkTreeIter *iter;
};
typedef struct GUI_TreeView TreeView;

struct GUI_l
{
	GUI_Windows windows;
	GUI_Buttons buttons;
	GUI_Entries entries;
	GUI_Fixed fixed;
	GUI_Menu menu;
	GUI_Textview txtview;
	GUI_TreeView hash, table;
	GUI_ComboBox cbox;
	GUI_SpinButton spin;
	GUI_CheckBox check;
	GtkWidget *label, *frame, *progbar;
};
typedef struct GUI_l GUI_l;

struct HashDatabase
{
	char hash[256];
	char plain[256];
};
typedef struct HashDatabase HashDatabase;

struct Database
{
	int status, msgiter, plainiter, n_folders, n_tables, n_hashes, n_found, button_state, n_hashdb_entries;
	std::vector<std::string> tables, hashes;
	float progress;
	struct timeval timer_start, timer_stop, pause_start, pause_stop;
	bool debug, fCrackerType; // true: hash cracker, false: lm cracker
	char progbartxt[256], current_action[256], time[256];
	HashDatabase *hashdb;
};
typedef struct Database Database;

struct Settings
{
	int n_threads, memlimit, keep_precalcs, verbose, save_plains, save_session, resume_session, use_hashdb;
	int debug;
	char plainfile[256], sessionfile[256], file[256], hashdb_file[256];
};
typedef struct Settings Settings;

struct MessageStack
{
	char msg[256], tag[16];
};

struct TableDir
{
	char dir[256];
};
