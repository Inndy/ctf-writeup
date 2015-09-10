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
#include "GUI_hash.h"
#include "GUI_file.h"
#include "RainbowCrack.h"
#include "GUI_database.h"
#include "GUI.h"

void LoadLMHashFromPwdumpFile(std::string sPathName, std::vector<std::string>& vUserName, std::vector<std::string>& vLMHash, std::vector<std::string>& vNTLMHash);

extern Database db;
extern GUI_l gui;

bool HashAlreadyInList(const char *buffer)
{
	int n = db.hashes.size(), i;
	
	for (i = 0; i < n; i++)
	{
		if (!strcmp(db.hashes[i].c_str(), buffer))
			return true;
	}
	return false;
}

int AddHash(const char *buffer)
{
	std::string normalized_hash = buffer;
	if (NormalizeHash(normalized_hash) && !HashAlreadyInList(normalized_hash.c_str()))
		db.hashes.push_back((std::string)buffer);
	else
		return -2;
	
	int len = strlen(buffer);
	if (len == 32)
		return HASH_MD5 | HASH_NTLM;
	else if (len == 40)
		return HASH_SHA1;
	else
		return -1;
}

void UpdateHashIters(int n)
{
	int i;
	for (i = n; i < db.n_hashes-1; i++)
		gui.hash.iter[i] = gui.hash.iter[i+1];
	
}

void DeleteHashes(GtkWidget *empty)
{
	GtkTreeSelection *selection;
	
	int i;
	selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(gui.hash.tree));
	
	for (i = db.hashes.size(); i--;)
	{
		if (gtk_tree_selection_iter_is_selected(selection, &gui.hash.iter[i]))
		{
			gtk_list_store_remove(gui.hash.store, &gui.hash.iter[i]);
			UpdateHashIters(i);
			db.hashes.erase(db.hashes.begin()+i);
			
			db.n_hashes--;
		}
	}
	return;
}

void ClearHashes(GtkWidget *empty)
{
	GtkTreeModel *model;
	GtkTreeIter iter;
	
	model = gtk_tree_view_get_model(GTK_TREE_VIEW(gui.hash.tree));
	if (gtk_tree_model_get_iter_first(model, &iter) == false)
		return;
	
	gtk_list_store_clear(gui.hash.store);
	db.hashes.clear();
	db.n_hashes = 0;
}

void LoadHashes(GtkWidget *empty)
{
	const gchar *txt = gtk_entry_get_text(GTK_ENTRY(gui.entries.hash));
	int n = gtk_combo_box_get_active(GTK_COMBO_BOX(gui.cbox.hash));
	
	/* Load single hash */
	if (strlen(txt) > 10 && n == 1)
	{
		if (HashAlreadyInList(txt))
			return;
		
		n = AddHash(txt);
		if (n == (HASH_MD5 | HASH_NTLM))
		{
			gtk_list_store_append(gui.hash.store, &gui.hash.iter[db.n_hashes]);
			gtk_list_store_set(gui.hash.store, &gui.hash.iter[db.n_hashes], 0, "MD5 | NTLM", 1, strlen(txt), 2, txt, -1);
			db.n_hashes++;
		}
		db.fCrackerType = true;
	}
	
	/* Load hashlist */
	else if (n == 0)
		OpenHashlist();
	
	/* Load pwdump file */
	else if (n == 2)
		OpenPwdumpFile();
	
	/* Load cain lst file */
	else if (n == 3)
		OpenCainFile();
	
	gtk_entry_set_text(GTK_ENTRY(gui.entries.hash), "");
}

void HideHashWindow(GtkWidget *empty)
{
	gtk_widget_hide(gui.windows.hash);
	SyncWithHashDatabase();
	
	if (db.hashes.size() > 0)
		MessageStackPush("col-black", "Loaded %d hash%s\n", (int)db.hashes.size(), db.hashes.size() > 1 ? "es" : "");
}

void ShowHashWindow(GtkWidget *empty)
{
	gtk_widget_show_all(gui.windows.hash);
	gtk_widget_show(gui.windows.hash);
}

void SetupHashWindow()
{
	/* Setup window */
	gui.windows.hash = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	gtk_window_set_title(GTK_WINDOW(gui.windows.hash), "Hashes");
	gtk_window_set_default_size(GTK_WINDOW(gui.windows.hash), 580, 340);
	gtk_window_set_deletable(GTK_WINDOW(gui.windows.hash), false);
	
	gui.fixed.hash = gtk_fixed_new();
	gtk_container_add(GTK_CONTAINER(gui.windows.hash), gui.fixed.hash);
	
	/* Load hashes */
	gui.entries.hash = gtk_entry_new();
	gtk_widget_set_size_request(gui.entries.hash, 380, 32);
	gtk_fixed_put(GTK_FIXED(gui.fixed.hash), gui.entries.hash, 8, 8);
	
	gui.cbox.hash = gtk_combo_box_new_text();
	gtk_combo_box_append_text(GTK_COMBO_BOX(gui.cbox.hash), "Hashlist");
	gtk_combo_box_append_text(GTK_COMBO_BOX(gui.cbox.hash), "Hash");
	gtk_combo_box_append_text(GTK_COMBO_BOX(gui.cbox.hash), "pwdump");
	gtk_combo_box_append_text(GTK_COMBO_BOX(gui.cbox.hash), "Cain");
	gtk_widget_set_size_request(gui.cbox.hash, 90, 32);
	gtk_combo_box_set_active(GTK_COMBO_BOX(gui.cbox.hash), 0);
	gtk_fixed_put(GTK_FIXED(gui.fixed.hash), gui.cbox.hash, 390, 8);
	
	gui.buttons.hash_load = gtk_button_new_with_label("Load");
	gtk_widget_set_size_request(gui.buttons.hash_load, 80, 32);
	gtk_fixed_put(GTK_FIXED(gui.fixed.hash), gui.buttons.hash_load, 485, 8);
	
	/* TreeView */
	gui.hash.scroll = gtk_scrolled_window_new(NULL, NULL);
	gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(gui.hash.scroll), GTK_POLICY_AUTOMATIC, GTK_POLICY_ALWAYS);
	
	gui.hash.tree = gtk_tree_view_new();
	gtk_tree_view_set_rules_hint(GTK_TREE_VIEW(gui.hash.tree), true);
	gtk_tree_view_set_rubber_banding(GTK_TREE_VIEW(gui.hash.tree), true);
	gtk_tree_view_set_headers_visible(GTK_TREE_VIEW(gui.hash.tree), true);
	gtk_tree_view_set_headers_clickable(GTK_TREE_VIEW(gui.hash.tree), true);
	
	gui.hash.store = gtk_list_store_new(3, G_TYPE_STRING, G_TYPE_INT, G_TYPE_STRING);
	gtk_tree_view_set_model(GTK_TREE_VIEW(gui.hash.tree), GTK_TREE_MODEL(gui.hash.store));
	
	gui.hash.render = gtk_cell_renderer_text_new();
	
	GtkTreeViewColumn *col = gtk_tree_view_column_new_with_attributes("Algorithm", gui.hash.render, "text", 0, NULL);
	gtk_tree_view_column_set_sort_column_id(col, 0);
	gtk_tree_view_append_column(GTK_TREE_VIEW(gui.hash.tree), col);
	
	col = gtk_tree_view_column_new_with_attributes("Length", gui.hash.render, "text", 1, NULL);
	gtk_tree_view_column_set_sort_column_id(col, 1);
	gtk_tree_view_append_column(GTK_TREE_VIEW(gui.hash.tree), col);
	
	col = gtk_tree_view_column_new_with_attributes("Hash", gui.hash.render, "text", 2, NULL);
	gtk_tree_view_column_set_sort_column_id(col, 2);
	gtk_tree_view_append_column(GTK_TREE_VIEW(gui.hash.tree), col);
	
	GtkTreeSelection *selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(gui.hash.tree));
	gtk_tree_selection_set_mode(selection, GTK_SELECTION_MULTIPLE);
	
	gtk_container_add(GTK_CONTAINER(gui.hash.scroll), gui.hash.tree);
	
	gtk_widget_set_size_request(gui.hash.tree, 560, 240);
	gtk_fixed_put(GTK_FIXED(gui.fixed.hash), gui.hash.scroll, 8, 48);
	
	/* Buttons */
	gui.buttons.hash_delete = gtk_button_new_with_label("Remove");
	gtk_widget_set_size_request(gui.buttons.hash_delete, 80, 32);
	gtk_fixed_put(GTK_FIXED(gui.fixed.hash), gui.buttons.hash_delete, 8, 300);
	
	gui.buttons.hash_clear = gtk_button_new_with_label("Clear");
	gtk_widget_set_size_request(gui.buttons.hash_clear, 80, 32);
	gtk_fixed_put(GTK_FIXED(gui.fixed.hash), gui.buttons.hash_clear, 100, 300);
	
	gui.buttons.hash_ok = gtk_button_new_with_label("OK");
	gtk_widget_set_size_request(gui.buttons.hash_ok, 80, 32);
	gtk_fixed_put(GTK_FIXED(gui.fixed.hash), gui.buttons.hash_ok, 480, 300);
	
	g_signal_connect(gui.buttons.hash_load, "clicked", G_CALLBACK(LoadHashes), NULL);
	g_signal_connect(gui.buttons.hash_ok, "clicked", G_CALLBACK(HideHashWindow), NULL);
	g_signal_connect(gui.buttons.hash_delete, "clicked", G_CALLBACK(DeleteHashes), NULL);
	g_signal_connect(gui.buttons.hash_clear, "clicked", G_CALLBACK(ClearHashes), NULL);
	
	return;
}
