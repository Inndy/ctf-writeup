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
#include "GUI.h"
#include "GUI_file.h"
#include "GUI_table.h"

extern Database db;
extern GUI_l gui;

bool TableAlreadyInList(const char *path, std::vector<std::string>list)
{
	int i;
	for (i = 0; i < (int)list.size(); i++)
	{
		if (!strcmp(path, list[i].c_str()))
			return true;
	}
	return false;
}

void ClearTables(GtkWidget *empty)
{
	GtkTreeModel *model;
	GtkTreeIter iter;
	
	model = gtk_tree_view_get_model(GTK_TREE_VIEW(gui.table.tree));
	if (gtk_tree_model_get_iter_first(model, &iter) == false)
		return;
	
	gtk_list_store_clear(gui.table.store);
	db.tables.clear();
	db.n_tables = db.n_folders = 0;
}

void  UpdateTableIters(int n)
{
	int i;
	for (i = n; i < db.n_tables-1; i++)
		gui.table.iter[i] = gui.table.iter[i+1];
}

void DeleteTables(GtkWidget *empty)
{
	GtkTreeSelection *selection;
	
	int i;
	selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(gui.table.tree));
	
	for (i = db.tables.size(); i--;)
	{
		if (gtk_tree_selection_iter_is_selected(selection, &gui.table.iter[i]))
		{
			gtk_list_store_remove(gui.table.store, &gui.table.iter[i]);
			UpdateTableIters(i);
			db.tables.erase(db.tables.begin()+i);
			db.n_tables--;
		}
	}
	return;
}

void GUIShowTableWindow()
{
	gtk_widget_show_all(gui.windows.table);
	gtk_widget_show(gui.windows.table);
}

void GUIHideTableWindow()
{
	gtk_widget_hide(gui.windows.table);
	if (db.tables.size() > 0)
		MessageStackPush("col-black", "Loaded %ld rainbow table data file%s\n", db.tables.size(), db.tables.size() > 1 ? "s" : "");
}

void GUISetupTableWindow()
{
	/* Hash window */
	gui.windows.table = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	gtk_window_set_title(GTK_WINDOW(gui.windows.table), "Rainbow Tables");
	gtk_container_set_border_width(GTK_CONTAINER(gui.windows.table), 5);
	
	gui.fixed.table = gtk_fixed_new();
	gtk_container_add(GTK_CONTAINER(gui.windows.table), gui.fixed.table);
	
	/* Buttons */
	gui.buttons.table_load = gtk_button_new_with_label("Load Tables");
	gtk_widget_set_size_request(gui.buttons.table_load, 88, 32);
	gtk_fixed_put(GTK_FIXED(gui.fixed.table), gui.buttons.table_load, 140, 8);
	
	gui.buttons.table_delete = gtk_button_new_with_label("Remove");
	gtk_widget_set_size_request(gui.buttons.table_delete, 88, 32);
	gtk_fixed_put(GTK_FIXED(gui.fixed.table), gui.buttons.table_delete, 250, 8);
	
	gui.buttons.table_clear = gtk_button_new_with_label("Clear");
	gtk_widget_set_size_request(gui.buttons.table_clear, 88, 32);
	gtk_fixed_put(GTK_FIXED(gui.fixed.table), gui.buttons.table_clear, 360, 8);
	
	gui.buttons.table_ok = gtk_button_new_with_label("OK");
	gtk_widget_set_size_request(gui.buttons.table_ok, 88, 32);
	gtk_fixed_put(GTK_FIXED(gui.fixed.table), gui.buttons.table_ok, 580, 8);
	
	/* TreeView */
	gui.table.scroll = gtk_scrolled_window_new(NULL, NULL);
	gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(gui.table.scroll), GTK_POLICY_AUTOMATIC, GTK_POLICY_ALWAYS);
	
	gui.table.tree = gtk_tree_view_new();
	gtk_tree_view_set_rules_hint(GTK_TREE_VIEW(gui.table.tree), true);
	
	gui.table.store = gtk_list_store_new(3, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING);
	gtk_tree_view_set_model(GTK_TREE_VIEW(gui.table.tree), GTK_TREE_MODEL(gui.table.store));
	
	gui.table.render = gtk_cell_renderer_text_new();
	
	GtkTreeViewColumn *col = gtk_tree_view_column_new_with_attributes("Algorithm", gui.table.render, "text", 0, NULL);
	gtk_tree_view_column_set_sort_column_id(col, 0);
	gtk_tree_view_append_column(GTK_TREE_VIEW(gui.table.tree), col);
	
	col = gtk_tree_view_column_new_with_attributes("Rainbow Table", gui.table.render, "text", 1, NULL);
	gtk_tree_view_column_set_sort_column_id(col, 1);
	gtk_tree_view_append_column(GTK_TREE_VIEW(gui.table.tree), col);
	
	col = gtk_tree_view_column_new_with_attributes("Path", gui.table.render, "text", 2, NULL);
	gtk_tree_view_column_set_sort_column_id(col, 2);
	gtk_tree_view_append_column(GTK_TREE_VIEW(gui.table.tree), col);
	
	GtkTreeSelection *selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(gui.table.tree));
	gtk_tree_selection_set_mode(selection, GTK_SELECTION_MULTIPLE);
	
	gtk_container_add(GTK_CONTAINER(gui.table.scroll), gui.table.tree);
	
	gtk_widget_set_size_request(gui.table.tree, 820, 350);
	gtk_fixed_put(GTK_FIXED(gui.fixed.table), gui.table.scroll, 0, 48);
	
	g_signal_connect(gui.buttons.table_load, "clicked", G_CALLBACK(OpenDir), NULL);
	g_signal_connect(gui.buttons.table_clear, "clicked", G_CALLBACK(ClearTables), NULL);
	g_signal_connect(gui.buttons.table_delete, "clicked", G_CALLBACK(DeleteTables), NULL);
	g_signal_connect(gui.buttons.table_ok, "clicked", G_CALLBACK(GUIHideTableWindow), NULL);
	return;
}
