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

#ifdef _WIN32
#include <time.h>
#else
#include <sys/time.h>
#endif

#include <pthread.h>
#include <string>
#include <vector>
#include <gtk/gtk.h>
#include "GUI_hash.h"
#include "RainbowCrack.h"
#include "GUI_file.h"
#include "GUI.h"

extern Database db;
extern GUI_l gui;

void StartSession(GtkWidget *empty)
{
	pthread_t thread[1];
	pthread_create(&thread[0], NULL, rcracki_mt, NULL);
	return;
}

void PauseResumeSession(GtkWidget *empty)
{
	/* Resume session */
	if (db.status == STATUS_PAUSE)
	{
		db.status = STATUS_RUNNING;
		MessageStackPush("col-black", "Session resumed.\n");
		gtk_button_set_label(GTK_BUTTON(gui.buttons.pause), "Pause");
		strcpy(db.current_action, "Running");
		gettimeofday(&db.pause_stop, 0);
		int diff = db.pause_stop.tv_sec - db.pause_start.tv_sec;
		db.timer_start.tv_sec += diff;
	}
	
	/* Pause session */
	else if (db.status == STATUS_RUNNING)
	{
		db.status = STATUS_PAUSE;
		MessageStackPush("col-black", "Session paused.\n");
		gtk_button_set_label(GTK_BUTTON(gui.buttons.pause), "Resume");
		strcpy(db.current_action, "Pausing");
		gettimeofday(&db.pause_start, 0);
	}
}

void AbortSession(GtkWidget *empty)
{
	if (db.status != STATUS_RUNNING && db.status != STATUS_PAUSE)
		return;
	
	#ifdef _WIN32
	Sleep(100);
	#else
	usleep(10000);
	#endif
	
	db.status = STATUS_IDLE;
	MessageStackPush("col-black", "Session aborted.\n");
	gtk_widget_set_sensitive(gui.buttons.run, true);
	gtk_widget_set_sensitive(gui.buttons.abort, false);
	gtk_widget_set_sensitive(gui.buttons.pause, false);
}

void ClearMessages(GtkWidget *empty)
{
	GtkTextIter start, end;
	gtk_text_buffer_get_bounds(gui.txtview.msg_buffer, &start, &end);
	gtk_text_buffer_delete(gui.txtview.msg_buffer, &start, &end);
}

int ConfirmQuit()
{
	GtkWidget *dialog;
	int response;
	
	dialog = gtk_message_dialog_new(NULL, GTK_DIALOG_DESTROY_WITH_PARENT, GTK_MESSAGE_QUESTION, GTK_BUTTONS_YES_NO, "A rcracki_mt session is still running.\nAre you sure you want to quit?");
	gtk_window_set_title(GTK_WINDOW(dialog), "rcracki_mt still running");
       
	response = gtk_dialog_run(GTK_DIALOG(dialog));
	gtk_widget_destroy(dialog);
       
	return response;
}

void QuitApplication(GtkWidget *empty)
{
	if (db.status != STATUS_IDLE)
	{
		if (ConfirmQuit() == GTK_RESPONSE_NO)
			return;
		
		AbortSession(NULL);
		#ifdef _WIN32
		Sleep(100);
		#else
		usleep(10000);
		#endif
	}
	gtk_main_quit();
}

void GUIShowInfo(GtkWidget *empty)
{
	GdkPixbuf *pix;
	GtkWidget *dialog = gtk_about_dialog_new();
	pix = gdk_pixbuf_new_from_file("freerainbowtables.png", NULL);
	
	gtk_about_dialog_set_name(GTK_ABOUT_DIALOG(dialog), "rcracki_mt-GUI");
	gtk_about_dialog_set_version(GTK_ABOUT_DIALOG(dialog), "0.2b"); 
	gtk_about_dialog_set_copyright(GTK_ABOUT_DIALOG(dialog), "(c) Paragon");
	gtk_about_dialog_set_comments(GTK_ABOUT_DIALOG(dialog), "rcracki_mt-GUI is a graphical user interface for rcracki_mt, a multithreaded hash cracker using rainbow tables.");
	gtk_about_dialog_set_website(GTK_ABOUT_DIALOG(dialog), "http://www.freerainbowtables.com");
	
	gtk_about_dialog_set_logo(GTK_ABOUT_DIALOG(dialog), pix);
	
	gtk_dialog_run(GTK_DIALOG (dialog));
	gtk_widget_destroy(dialog);
}

void GUIShowMainWindow()
{
	gui.windows.main = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	gtk_window_set_title(GTK_WINDOW(gui.windows.main), "rcracki_mt-GUI");
	gtk_window_set_default_size(GTK_WINDOW(gui.windows.main), 740, 540);
	gtk_window_set_icon(GTK_WINDOW(gui.windows.main), gdk_pixbuf_new_from_file("icon.png", NULL));
	
	gui.fixed.main = gtk_fixed_new();
	gtk_container_add(GTK_CONTAINER(gui.windows.main), gui.fixed.main);
	
	/* Menu bar */
	gui.menu.bar = gtk_menu_bar_new();
	gtk_widget_set_size_request(gui.menu.bar, 740, 28);
	gui.menu.filemenu = gtk_menu_new();
	gui.menu.settingsmenu = gtk_menu_new();
	gui.menu.helpmenu = gtk_menu_new();
	
	gui.menu.file = gtk_menu_item_new_with_mnemonic("_File");
	gui.menu.tables = gtk_menu_item_new_with_label("Add tables");
	gui.menu.hashes = gtk_menu_item_new_with_label("Add hashes");
	gui.menu.session = gtk_menu_item_new_with_label("Load session");
	gui.menu.quit = gtk_menu_item_new_with_label("Quit");
	
	gui.menu.settings = gtk_menu_item_new_with_mnemonic("_Settings");
	gui.menu.preferences = gtk_menu_item_new_with_label("Preferences");
	
	gui.menu.help = gtk_menu_item_new_with_mnemonic("_Help");
	gui.menu.info = gtk_menu_item_new_with_label("About");
	
	gui.menu.seperator = gtk_separator_menu_item_new();
	
	gtk_menu_item_set_submenu(GTK_MENU_ITEM(gui.menu.file), gui.menu.filemenu);
	gtk_menu_shell_append(GTK_MENU_SHELL(gui.menu.filemenu), gui.menu.tables);
	gtk_menu_shell_append(GTK_MENU_SHELL(gui.menu.filemenu), gui.menu.hashes);
	gtk_menu_shell_append(GTK_MENU_SHELL(gui.menu.filemenu), gui.menu.session);
	gtk_menu_shell_append(GTK_MENU_SHELL(gui.menu.filemenu), gui.menu.seperator);
	gtk_menu_shell_append(GTK_MENU_SHELL(gui.menu.filemenu), gui.menu.quit);
	gtk_menu_shell_append(GTK_MENU_SHELL(gui.menu.bar), gui.menu.file);
	
	gtk_menu_item_set_submenu(GTK_MENU_ITEM(gui.menu.settings), gui.menu.settingsmenu);
	gtk_menu_shell_append(GTK_MENU_SHELL(gui.menu.settingsmenu), gui.menu.preferences);
	gtk_menu_shell_append(GTK_MENU_SHELL(gui.menu.bar), gui.menu.settings);
	
	gtk_menu_item_set_submenu(GTK_MENU_ITEM(gui.menu.help), gui.menu.helpmenu);
	gtk_menu_shell_append(GTK_MENU_SHELL(gui.menu.helpmenu), gui.menu.info);
	gtk_menu_shell_append(GTK_MENU_SHELL(gui.menu.bar), gui.menu.help);
	
	gtk_fixed_put(GTK_FIXED(gui.fixed.main), gui.menu.bar, 0, 0);
	
	/* Control buttons */
	gui.buttons.run = gtk_button_new_with_label("Run");
	gtk_widget_set_size_request(gui.buttons.run, 64, 32);
	gtk_fixed_put(GTK_FIXED(gui.fixed.main), gui.buttons.run, 8, 56);
	
	gui.buttons.pause = gtk_button_new_with_label("Pause");
	gtk_widget_set_size_request(gui.buttons.pause, 64, 32);
	gtk_widget_set_sensitive(gui.buttons.pause, false);
	gtk_fixed_put(GTK_FIXED(gui.fixed.main), gui.buttons.pause, 88, 56);
	
	gui.buttons.abort = gtk_button_new_with_label("Abort");
	gtk_widget_set_size_request(gui.buttons.abort, 64, 32);
	gtk_widget_set_sensitive(gui.buttons.abort, false);
	gtk_fixed_put(GTK_FIXED(gui.fixed.main), gui.buttons.abort, 168, 56);
	
	gui.buttons.clear = gtk_button_new_with_label("Clear");
	gtk_widget_set_size_request(gui.buttons.clear, 64, 32);
	gtk_widget_set_tooltip_text(gui.buttons.clear, "Clear all messages");
	gtk_fixed_put(GTK_FIXED(gui.fixed.main), gui.buttons.clear, 248, 56);
	
	/* Message view */
	GtkWidget *scroll = gtk_scrolled_window_new(NULL, NULL);
	gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scroll), GTK_POLICY_AUTOMATIC, GTK_POLICY_ALWAYS);
	
	gui.frame = gtk_aspect_frame_new("Messages", 0.1, 0.1, 10, true);
	gtk_widget_set_size_request(gui.frame, 740, 165);
	gtk_fixed_put(GTK_FIXED(gui.fixed.main), gui.frame, 5, 100);
	
	gui.txtview.msg = gtk_text_view_new();
	gtk_widget_set_size_request(gui.txtview.msg, 712, 140);
	gtk_text_view_set_left_margin(GTK_TEXT_VIEW(gui.txtview.msg), 5);
	gtk_text_view_set_editable(GTK_TEXT_VIEW(gui.txtview.msg), false);
	gui.txtview.msg_buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(gui.txtview.msg));
	
	gtk_container_add(GTK_CONTAINER(scroll), gui.txtview.msg);
	gtk_fixed_put(GTK_FIXED(gui.fixed.main), scroll, 8, 120);
	
	/* Plaintext view */
	scroll = gtk_scrolled_window_new(NULL, NULL);
	gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scroll), GTK_POLICY_AUTOMATIC, GTK_POLICY_ALWAYS);
	
	gui.frame = gtk_aspect_frame_new("Plaintexts", 0.0, 0.0, 10, true);
	gtk_widget_set_size_request(gui.frame, 740, 165);
	gtk_fixed_put(GTK_FIXED(gui.fixed.main), gui.frame, 5, 280);
	
	gui.txtview.hashes = gtk_text_view_new();
	gtk_widget_set_size_request(gui.txtview.hashes, 712, 140);
	gtk_text_view_set_left_margin(GTK_TEXT_VIEW(gui.txtview.hashes), 5);
	gtk_text_view_set_editable(GTK_TEXT_VIEW(gui.txtview.hashes), false);
	gui.txtview.hashes_buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(gui.txtview.hashes));
	gtk_container_add(GTK_CONTAINER(scroll), gui.txtview.hashes);
	gtk_fixed_put(GTK_FIXED(gui.fixed.main), scroll, 8, 300);
	
	gtk_text_buffer_get_iter_at_offset(gui.txtview.msg_buffer, &gui.txtview.msg_iter, 0);
	gtk_text_buffer_get_iter_at_offset(gui.txtview.hashes_buffer, &gui.txtview.hashes_iter, 0);
	
	GdkColor color;
	gdk_color_parse("red", &color);
	gtk_text_buffer_create_tag(gui.txtview.msg_buffer, "col-red", "foreground-gdk", &color, NULL);
	gdk_color_parse("darkblue", &color);
	gtk_text_buffer_create_tag(gui.txtview.msg_buffer, "col-blue", "foreground-gdk", &color, NULL);
	gdk_color_parse("grey", &color);
	gtk_text_buffer_create_tag(gui.txtview.msg_buffer, "col-grey", "foreground-gdk", &color, NULL);
	gdk_color_parse("black", &color);
	gtk_text_buffer_create_tag(gui.txtview.msg_buffer, "col-black", "foreground-gdk", &color, NULL);
	gdk_color_parse("darkgreen", &color);
	gtk_text_buffer_create_tag(gui.txtview.msg_buffer, "col-green", "foreground-gdk", &color, NULL);
	color.red = color.green = color.blue = 12000;
	gtk_text_buffer_create_tag(gui.txtview.msg_buffer, "col-darkgrey", "foreground-gdk", &color, NULL);
	
	/* Progress bar */
	gui.progbar = gtk_progress_bar_new();
	gtk_widget_set_size_request(gui.progbar, 735, 32);
	gtk_fixed_put(GTK_FIXED(gui.fixed.main), gui.progbar, 8, 450);
	
	/* Information bar */
	gui.label = gtk_label_new("Current:");
	gtk_fixed_put(GTK_FIXED(gui.fixed.main), gui.label, 20, 510);
	gui.entries.current = gtk_entry_new();
	gtk_entry_set_editable(GTK_ENTRY(gui.entries.current), false);
	gtk_entry_set_text(GTK_ENTRY(gui.entries.current), "None");
	gtk_widget_set_tooltip_text(gui.entries.current, "Current action");
	gtk_widget_set_size_request(gui.entries.current, 120, 24);
	gtk_fixed_put(GTK_FIXED(gui.fixed.main), gui.entries.current, 90, 505);
	
	gui.label = gtk_label_new("Found:");
	gtk_fixed_put(GTK_FIXED(gui.fixed.main), gui.label, 250, 510);
	gui.entries.found = gtk_entry_new();
	gtk_entry_set_editable(GTK_ENTRY(gui.entries.found), false);
	gtk_widget_set_tooltip_text(gui.entries.found, "Cracked hashes");
	gtk_widget_set_size_request(gui.entries.found, 120, 24);
	gtk_fixed_put(GTK_FIXED(gui.fixed.main), gui.entries.found, 320, 505);
	
	gui.label = gtk_label_new("Elapsed time:");
	gtk_fixed_put(GTK_FIXED(gui.fixed.main), gui.label, 470, 510);
	gui.entries.time= gtk_entry_new();
	gtk_entry_set_editable(GTK_ENTRY(gui.entries.time), false);
	gtk_widget_set_tooltip_text(gui.entries.time, "Elapsed time since start");
	gtk_widget_set_size_request(gui.entries.time, 120, 24);
	gtk_fixed_put(GTK_FIXED(gui.fixed.main), gui.entries.time, 570, 505);
	
	gtk_widget_show_all(gui.windows.main);
	g_signal_connect(gui.windows.main, "destroy", G_CALLBACK(QuitApplication), NULL);
	g_signal_connect(gui.windows.main, "delete-event", G_CALLBACK(QuitApplication), NULL);
	g_signal_connect(gui.menu.hashes, "activate", G_CALLBACK(ShowHashWindow), NULL);
	g_signal_connect(gui.menu.tables, "activate", G_CALLBACK(GUIShowTableWindow), NULL);
	g_signal_connect(gui.menu.preferences, "activate", G_CALLBACK(GUIShowSettingsWindow), NULL);
	g_signal_connect(gui.buttons.run, "clicked", G_CALLBACK(StartSession), NULL);
	g_signal_connect(gui.buttons.clear, "clicked", G_CALLBACK(ClearMessages), NULL);
	g_signal_connect(gui.menu.session, "activate", G_CALLBACK(BrowseSessionFile), NULL);
	g_signal_connect(gui.buttons.pause, "clicked", G_CALLBACK(PauseResumeSession), NULL);
	g_signal_connect(gui.buttons.abort, "clicked", G_CALLBACK(AbortSession), NULL);
	g_signal_connect(gui.menu.quit, "activate", G_CALLBACK(QuitApplication), NULL);
	g_signal_connect(gui.menu.info, "activate", G_CALLBACK(GUIShowInfo), NULL);
}
