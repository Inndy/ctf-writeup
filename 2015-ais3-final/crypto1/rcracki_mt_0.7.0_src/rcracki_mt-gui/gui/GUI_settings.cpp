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
#include "GUI_settings.h"
#include "GUI_file.h"
#include "GUI.h"

extern Database db;
extern GUI_l gui;
extern Settings settings;

void WriteSettings(GtkWidget *empty)
{
	settings.n_threads = gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(gui.spin.threads));
	settings.memlimit = gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(gui.spin.mem));
	settings.verbose = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(gui.check.verbose));
	settings.keep_precalcs = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(gui.check.precalc));
	settings.save_plains = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(gui.check.save_cracked));
	settings.save_session = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(gui.check.session));
	settings.use_hashdb = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(gui.check.hashdb));
	
	strcpy(settings.hashdb_file, gtk_entry_get_text(GTK_ENTRY(gui.entries.hashdb_file)));
	strcpy(settings.plainfile, gtk_entry_get_text(GTK_ENTRY(gui.entries.save_cracked)));
	strcpy(settings.sessionfile, gtk_entry_get_text(GTK_ENTRY(gui.entries.session_file)));
	
	
	FILE *file;
	file = fopen(settings.file, "wb");
	if (!file)
	{
		printf("Error opening file %s for writing\n", settings.file);
		gtk_widget_destroy(gui.windows.settings);
		return;
	}
	
	fprintf(file, "#rcracki_mt-GUI 0.2b config file\n");
	fprintf(file, "Threads: %d\n", settings.n_threads);
	fprintf(file, "Memlimit: %d\n", settings.memlimit);
	fprintf(file, "Keep_Precalcs: %d\n", settings.keep_precalcs);
	fprintf(file, "Verbose: %d\n", settings.verbose);
	fprintf(file, "Save_Plains: %d %s\n", settings.save_plains, settings.plainfile);
	fprintf(file, "Save_Session: %d %s\n", settings.save_session, settings.sessionfile);
	fprintf(file, "Use_Hashdb: %d %s\n", settings.use_hashdb, settings.hashdb_file);
	
	fclose(file);
	
	if (db.debug)
		printf("Wrote config file %s\n", settings.file);
	
	gtk_widget_destroy(gui.windows.settings);
}

void ReadSettingsFile()
{
	FILE *file;
	file = fopen(settings.file, "rb");
	if (!file)
	{
		printf("Settings file %s does not exist. Using default settings.\n", settings.file);
		return;
	}
	char buffer[256];
	while (!feof(file))
	{
		fgets(buffer, 256, file);
		if (!strncmp(buffer, "Threads: ", 9))
			sscanf(buffer, "Threads: %d", &settings.n_threads);
		else if (!strncmp(buffer, "Memlimit: ", 10))
			sscanf(buffer, "Memlimit: %d", &settings.memlimit);
		else if (!strncmp(buffer, "Keep_Precalcs: ", 15))
			sscanf(buffer, "Keep_Precalcs: %d", &settings.keep_precalcs);
		else if (!strncmp(buffer, "Verbose: ", 9))
			sscanf(buffer, "Verbose: %d", &settings.verbose);
		else if (!strncmp(buffer, "Save_Plains: ", 13))
			sscanf(buffer, "Save_Plains: %d %s", &settings.save_plains, settings.plainfile);
		else if (!strncmp(buffer, "Save_Session: ", 14))
			sscanf(buffer, "Save_Session: %d %s", &settings.save_session, settings.sessionfile);
		else if (!strncmp(buffer, "Use_Hashdb: ", 12))
			sscanf(buffer, "Use_Hashdb: %d %s", &settings.use_hashdb, settings.hashdb_file);
	}
	fclose(file);
	return;
}

void DefaultSettings(GtkWidget *empty)
{
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(gui.check.precalc), false);
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(gui.check.verbose), false);
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(gui.check.save_cracked), false);
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(gui.check.session), false);
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(gui.check.hashdb), true);
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(gui.spin.threads), 1);
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(gui.spin.mem), 2048);
	
}

void UpdatePlainEntry(GtkWidget *empty)
{
	gtk_entry_set_text(GTK_ENTRY(gui.entries.save_cracked), settings.plainfile);
}

void UpdateSessionEntry(GtkWidget *empty)
{
	gtk_entry_set_text(GTK_ENTRY(gui.entries.session_file), settings.sessionfile);
}

void UpdateHashdbEntry(GtkWidget *empty)
{
	gtk_entry_set_text(GTK_ENTRY(gui.entries.hashdb_file), settings.hashdb_file);
}

void UpdateSettingsSensitives()
{
	bool n = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(gui.check.save_cracked));
	settings.save_plains = n;
	gtk_widget_set_sensitive(gui.entries.save_cracked, n);
	gtk_widget_set_sensitive(gui.buttons.save_cracked, n);
	
	n = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(gui.check.session));
	settings.save_session = n;
	gtk_widget_set_sensitive(gui.entries.session_file, n);
	gtk_widget_set_sensitive(gui.buttons.session_file, n);

	n = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(gui.check.hashdb));
	settings.use_hashdb = n;
	gtk_widget_set_sensitive(gui.entries.hashdb_file, n);
	gtk_widget_set_sensitive(gui.buttons.hashdb, n);
}

void GUIShowSettingsWindow()
{
	/* Window */
	gui.windows.settings = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	gtk_window_set_title(GTK_WINDOW(gui.windows.settings), "Settings");
	gtk_window_set_default_size(GTK_WINDOW(gui.windows.settings), 460, 260);
	
	gui.fixed.settings = gtk_fixed_new();
	gtk_container_add(GTK_CONTAINER(gui.windows.settings), gui.fixed.settings);
	
	/* Threads & Mem limit */
	GtkWidget *frame = gtk_frame_new(NULL);
	gtk_widget_set_size_request(frame, 450, 80);
	gtk_fixed_put(GTK_FIXED(gui.fixed.settings), frame, 5, 5);
	
	gui.label = gtk_label_new("Threads:");
	gtk_fixed_put(GTK_FIXED(gui.fixed.settings), gui.label, 10, 20);
	
	gui.spin.threads = gtk_spin_button_new_with_range(1.0f, 99.0f, 1.0f);
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(gui.spin.threads), (float)settings.n_threads);
	gtk_fixed_put(GTK_FIXED(gui.fixed.settings), gui.spin.threads, 80, 15);
	
	gui.label = gtk_label_new("Memory limit (MB):");
	gtk_fixed_put(GTK_FIXED(gui.fixed.settings), gui.label, 200, 20);
	
	gui.spin.mem = gtk_spin_button_new_with_range(128.0f, 8192.0f, 32.0f);
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(gui.spin.mem), (float)settings.memlimit);
	gtk_fixed_put(GTK_FIXED(gui.fixed.settings), gui.spin.mem, 340, 15);
	
	gui.check.precalc = gtk_check_button_new_with_label("Keep precalcs");
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(gui.check.precalc), settings.keep_precalcs);
	gtk_widget_set_tooltip_text(gui.check.precalc, "Do not delete precalculations");
	gtk_fixed_put(GTK_FIXED(gui.fixed.settings), gui.check.precalc, 8, 50);
	
	gui.check.verbose = gtk_check_button_new_with_label("Verbose output");
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(gui.check.verbose), settings.verbose);
	gtk_fixed_put(GTK_FIXED(gui.fixed.settings), gui.check.verbose, 200, 50);
	
	gui.check.save_cracked = gtk_check_button_new_with_label("Save cracked hashes");
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(gui.check.save_cracked), settings.save_plains);
	gtk_widget_set_tooltip_text(gui.check.save_cracked, "Write hashes and plains to this file");
	gtk_fixed_put(GTK_FIXED(gui.fixed.settings), gui.check.save_cracked, 8, 100);
	
	frame = gtk_frame_new(NULL);
	gtk_widget_set_size_request(frame, 450, 120);
	gtk_fixed_put(GTK_FIXED(gui.fixed.settings), frame, 5, 90);
	
	gui.entries.save_cracked = gtk_entry_new();
	gtk_widget_set_size_request(gui.entries.save_cracked, 180, 24);
	if (!settings.save_plains)
		gtk_widget_set_sensitive(gui.entries.save_cracked, false);
	gtk_entry_set_text(GTK_ENTRY(gui.entries.save_cracked), settings.plainfile);
	gtk_fixed_put(GTK_FIXED(gui.fixed.settings), gui.entries.save_cracked, 200, 100);
	
	gui.buttons.save_cracked = gtk_button_new_with_label("Browse");
	gtk_widget_set_size_request(gui.buttons.save_cracked, 70, 24);
	if (!settings.save_plains)
		gtk_widget_set_sensitive(gui.buttons.save_cracked, false);
	gtk_fixed_put(GTK_FIXED(gui.fixed.settings), gui.buttons.save_cracked, 380, 100);
	
	gui.check.session = gtk_check_button_new_with_label("Save session");
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(gui.check.session), settings.save_session);
	gtk_widget_set_tooltip_text(gui.check.session, "Saved sessions can be resumed");
	gtk_fixed_put(GTK_FIXED(gui.fixed.settings), gui.check.session, 8, 140);
	
	gui.entries.session_file = gtk_entry_new();
	gtk_widget_set_size_request(gui.entries.session_file, 180, 24);
	if (!settings.save_session)
		gtk_widget_set_sensitive(gui.entries.session_file, false);
	gtk_entry_set_text(GTK_ENTRY(gui.entries.session_file), settings.sessionfile);
	gtk_fixed_put(GTK_FIXED(gui.fixed.settings), gui.entries.session_file, 200, 140);
	
	gui.buttons.session_file = gtk_button_new_with_label("Browse");
	gtk_widget_set_size_request(gui.buttons.session_file, 70, 24);
	if (!settings.save_session)
		gtk_widget_set_sensitive(gui.buttons.session_file, false);
	gtk_fixed_put(GTK_FIXED(gui.fixed.settings), gui.buttons.session_file, 380, 140);
	
	gui.check.hashdb = gtk_check_button_new_with_label("Use hashdb");
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(gui.check.hashdb), settings.use_hashdb);
	gtk_widget_set_tooltip_text(gui.check.hashdb, "Remember cracked hashes");
	gtk_fixed_put(GTK_FIXED(gui.fixed.settings), gui.check.hashdb, 8, 180);
	
	gui.entries.hashdb_file = gtk_entry_new();
	gtk_widget_set_size_request(gui.entries.hashdb_file, 180, 24);
	if (!settings.use_hashdb)
		gtk_widget_set_sensitive(gui.entries.hashdb_file, false);
	gtk_entry_set_text(GTK_ENTRY(gui.entries.hashdb_file), settings.hashdb_file);
	gtk_fixed_put(GTK_FIXED(gui.fixed.settings), gui.entries.hashdb_file, 200, 180);
	
	gui.buttons.hashdb = gtk_button_new_with_label("Browse");
	gtk_widget_set_size_request(gui.buttons.hashdb, 70, 24);
	if (!settings.use_hashdb)
		gtk_widget_set_sensitive(gui.buttons.hashdb, false);
	gtk_fixed_put(GTK_FIXED(gui.fixed.settings), gui.buttons.hashdb, 380, 180);
	
	gui.buttons.pref_ok = gtk_button_new_with_label("OK");
	gtk_widget_set_size_request(gui.buttons.pref_ok, 70, 32);
	gtk_widget_set_tooltip_text(gui.buttons.pref_ok, "Save settings and exit");
	gtk_fixed_put(GTK_FIXED(gui.fixed.settings), gui.buttons.pref_ok, 140, 220);
	
	GtkWidget *set_def = gtk_button_new_with_label("Default");
	gtk_widget_set_size_request(set_def, 70, 32);
	gtk_widget_set_tooltip_text(set_def, "Restore default settings");
	gtk_fixed_put(GTK_FIXED(gui.fixed.settings), set_def, 240, 220);
	
	g_signal_connect(G_OBJECT(gui.buttons.pref_ok), "clicked", G_CALLBACK(WriteSettings), NULL);
	g_signal_connect(G_OBJECT(gui.buttons.save_cracked), "clicked", G_CALLBACK(BrowseFile), (void*)settings.plainfile);
	g_signal_connect(G_OBJECT(gui.buttons.save_cracked), "clicked", G_CALLBACK(UpdatePlainEntry), NULL);
	g_signal_connect(G_OBJECT(gui.buttons.session_file), "clicked", G_CALLBACK(BrowseFile), (void*)settings.sessionfile);
	g_signal_connect(G_OBJECT(gui.buttons.session_file), "clicked", G_CALLBACK(UpdateSessionEntry), NULL);
	g_signal_connect(G_OBJECT(gui.buttons.hashdb), "clicked", G_CALLBACK(BrowseFile), (void*)settings.hashdb_file);
	g_signal_connect(G_OBJECT(gui.buttons.hashdb), "clicked", G_CALLBACK(UpdateHashdbEntry), NULL);
	g_signal_connect(G_OBJECT(gui.check.save_cracked), "clicked", G_CALLBACK(UpdateSettingsSensitives), NULL);
	g_signal_connect(G_OBJECT(gui.check.session), "clicked", G_CALLBACK(UpdateSettingsSensitives), NULL);
	g_signal_connect(G_OBJECT(gui.check.hashdb), "clicked", G_CALLBACK(UpdateSettingsSensitives), NULL);
	g_signal_connect(G_OBJECT(set_def), "clicked", G_CALLBACK(DefaultSettings), NULL);
	
	gtk_widget_show_all(gui.windows.settings);
}
