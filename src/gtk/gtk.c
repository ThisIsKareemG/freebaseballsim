/*
Free Baseball Simulator - A configurable baseball simulator by Tom Stellard
Copyright (C) 2009 Tom Stellard

This file is part of Free Baseball Simulator

Free Baseball Simulator is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Free Baseball Simulator is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with Free Baseball Simulator.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <stdlib.h>
#include <stdio.h>
#include <gtk/gtk.h>
#include "pages.h"
#include "db_object.h"
#include "baseball_objects.h"

struct widgets{

	GtkWidget *current_user;
	GtkWidget *current_file;
	GtkWidget *lineup;
};

Db_Object DB = NULL;
int TEAM_ID = 0;
int LEAGUE_ID = 0;
char *FILENAME;

static void destroy(GtkWidget *widget, gpointer data)
{
	gtk_main_quit();
}

static void change_DB(GtkWidget *widget, gpointer data)
{
	FILENAME = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(widget));
	DB = db_connect(FILENAME);
	if(!DB){
		fprintf(stderr, "Error could not load saved file.\n");
		return;
	}
}

static void user_changed(GtkWidget *widget, gpointer data)
{
	GtkTreeIter iter;
	GtkListStore *store;

	store = GTK_LIST_STORE(gtk_combo_box_get_model(GTK_COMBO_BOX(widget)));
	if(gtk_combo_box_get_active_iter(GTK_COMBO_BOX(widget), &iter)){
		gtk_tree_model_get(GTK_TREE_MODEL(store), &iter, 2, &TEAM_ID, 
							3, &LEAGUE_ID, -1);
	}
}

int main(int argc, char **argv)
{
	GtkWidget *window;
	GtkWidget *notebook;
	struct roster_page roster;
	struct setup_page setup;
	struct standings_page standings;
	struct simulate_page simulate;
	struct stats_page stats;
	GtkWidget *setup_label;
	GtkWidget *roster_label;
	GtkWidget *standings_label;
	GtkWidget *simulate_label;
	GtkWidget *stats_label;
	
	gtk_init(&argc, &argv);
	
	/*Create window*/	
	window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	/*Create notebook*/
	notebook = gtk_notebook_new();
	gtk_notebook_set_tab_pos(GTK_NOTEBOOK(notebook), GTK_POS_TOP);
	/*Create setup page*/
	setup_page_init(&setup);
	setup_label = gtk_label_new("Setup");
	gtk_notebook_append_page(GTK_NOTEBOOK(notebook), setup.main,
								setup_label);
	/*Create roster page*/
	roster_page_init(&roster);
	roster_label = gtk_label_new("Roster");
	gtk_notebook_append_page(GTK_NOTEBOOK(notebook), roster.main,
								roster_label);
	/*Create standings page*/
	standings_page_init(&standings);
	standings_label = gtk_label_new("Standings");
	gtk_notebook_append_page(GTK_NOTEBOOK(notebook), standings.main,
							standings_label);
	/*Create simulate page*/
	simulate_page_init(&simulate);
	simulate_label = gtk_label_new("Simulate");
	gtk_notebook_append_page(GTK_NOTEBOOK(notebook), simulate.main,
							simulate_label);
	
	/*Create stats page*/
	stats_page_init(&stats);
	stats_label = gtk_label_new("Stats");
	gtk_notebook_append_page(GTK_NOTEBOOK(notebook), stats.main,
								stats_label);
	
	/*Add widgets to the window.*/
	gtk_container_add(GTK_CONTAINER(window), notebook);
	
	/*Setup signals*/
	/* Application Exit */
	g_signal_connect(G_OBJECT(window), "destroy",
				G_CALLBACK(destroy), NULL);
	/* File changed */
	g_signal_connect(G_OBJECT(setup.current_file), "file-set",
				G_CALLBACK(change_DB), NULL);
	g_signal_connect(G_OBJECT(setup.current_file), "file-set",
				G_CALLBACK(setup_page_update), &setup);
	/* User changed */
	g_signal_connect(G_OBJECT(setup.current_user), "changed",
				G_CALLBACK(user_changed), NULL);
	g_signal_connect(G_OBJECT(setup.current_user), "changed",
				G_CALLBACK(roster_page_update), &roster);
	g_signal_connect(G_OBJECT(setup.current_user), "changed",
				G_CALLBACK(standings_page_update), &standings);
	g_signal_connect(G_OBJECT(setup.current_user), "changed",
				G_CALLBACK(stats_page_update), &stats);
	
	/* Games simulated */
	g_signal_connect(G_OBJECT(simulate.main), "clicked",
				G_CALLBACK(roster_page_update), &roster);
	g_signal_connect(G_OBJECT(simulate.main), "clicked",
				G_CALLBACK(standings_page_update), &standings);
	g_signal_connect(G_OBJECT(simulate.main), "clicked",
				G_CALLBACK(stats_page_update), &stats);
	gtk_widget_show_all(window);

	gtk_main();

	return 0;
}
