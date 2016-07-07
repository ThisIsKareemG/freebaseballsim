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
#include <unistd.h>
#include <sys/types.h>
#include <gtk/gtk.h>
#include "pages.h"
#include "db_object.h"

static void show_new_file_dialog(GtkWidget *widget, gpointer data);
static void create_user(GtkWidget *widget, gpointer data);
static void current_user_update(GtkWidget *current_user, Db_Object db);
static void new_user_team_update(GtkWidget *new_user_team, Db_Object db);
static int insert_user(Db_Object db, const char *user);
static void update_team_owner(Db_Object db, int team_id, int user_id);

void setup_page_init(struct setup_page *page)
{
	int vpad = 5;
	int hpad = 5;
	GtkWidget *setup;
	GtkWidget *new_file_button;
	GtkWidget *current_file;
	GtkWidget *current_file_label;
	GtkWidget *current_user;
	GtkWidget *current_user_label;
	GtkWidget *new_user_button;
	GtkWidget *new_user_text;
	GtkWidget *new_user_team;
	GtkListStore *store;
	GtkListStore *team_store;
	GtkCellRenderer *text_renderer;

	setup = gtk_table_new(4, 3, FALSE);
	new_file_button = gtk_button_new_with_label("New Simulation");
	current_file_label = gtk_label_new("Load Saved Simulation:");
	gtk_misc_set_alignment(GTK_MISC(current_file_label), 0, 1);
	current_file = gtk_file_chooser_button_new("Open File:",
						GTK_FILE_CHOOSER_ACTION_OPEN);
	current_user = gtk_combo_box_new();
	store = gtk_list_store_new(4, G_TYPE_STRING, G_TYPE_UINT, G_TYPE_UINT,
								G_TYPE_UINT);
	gtk_combo_box_set_model(GTK_COMBO_BOX(current_user), GTK_TREE_MODEL(store));
	text_renderer = gtk_cell_renderer_text_new();
	gtk_cell_layout_pack_start(GTK_CELL_LAYOUT(current_user),
						text_renderer, FALSE );
	gtk_cell_layout_set_attributes(GTK_CELL_LAYOUT(current_user),
					text_renderer, "text", 0, NULL);
	current_user_label = gtk_label_new("Select a User:");
	gtk_misc_set_alignment(GTK_MISC(current_user_label), 0, 1);
	
	/*New user widgets*/
	new_user_button = gtk_button_new_with_label("Add User");
	new_user_text = gtk_entry_new();
	new_user_team = gtk_combo_box_new();
	team_store = gtk_list_store_new(2, G_TYPE_STRING, G_TYPE_UINT);
	gtk_combo_box_set_model(GTK_COMBO_BOX(new_user_team),
					GTK_TREE_MODEL(team_store));
	gtk_cell_layout_pack_start(GTK_CELL_LAYOUT(new_user_team),
						text_renderer, FALSE);
	gtk_cell_layout_set_attributes(GTK_CELL_LAYOUT(new_user_team),
					text_renderer, "text", 0, NULL);
	/*Add widgets to the table.*/
	gtk_table_attach(GTK_TABLE(setup), new_file_button,
		0, 2, 0, 1, GTK_EXPAND | GTK_FILL, GTK_SHRINK, 0, vpad);
	gtk_table_attach(GTK_TABLE(setup), current_file_label,
		0, 1, 1, 2, GTK_FILL, GTK_SHRINK, hpad, vpad);
	gtk_table_attach(GTK_TABLE(setup), current_file,
		1, 2, 1, 2, GTK_EXPAND | GTK_FILL, GTK_SHRINK, 0, vpad);
	gtk_table_attach(GTK_TABLE(setup), current_user_label,
		0, 1, 2, 3, GTK_FILL, GTK_SHRINK, hpad, vpad);
	gtk_table_attach(GTK_TABLE(setup), current_user,
		1, 2, 2, 3, GTK_EXPAND | GTK_FILL, GTK_SHRINK, 0, vpad);
	gtk_table_attach(GTK_TABLE(setup), new_user_text,
		0, 1, 3, 4, GTK_FILL | GTK_EXPAND, GTK_SHRINK, hpad, vpad);
	gtk_table_attach(GTK_TABLE(setup), new_user_button,
		2, 3, 3, 4, GTK_EXPAND | GTK_FILL, GTK_SHRINK, 0, vpad);
	gtk_table_attach(GTK_TABLE(setup), new_user_team,
		1, 2, 3, 4, GTK_EXPAND | GTK_FILL, GTK_SHRINK, 0, vpad);

	g_signal_connect(G_OBJECT(new_file_button), "clicked",
				G_CALLBACK(show_new_file_dialog), page);
	g_signal_connect(G_OBJECT(new_user_button), "clicked",
				G_CALLBACK(create_user), page);
	page->main = setup;
	page->current_file = current_file;
	page->current_user = current_user;
	page->new_user_text = new_user_text;
	page->new_user_team = new_user_team;
}

void setup_page_update(GtkWidget *widget, gpointer data)
{
	extern Db_Object DB;
	struct setup_page *page = (struct setup_page *)data;
	if(!DB){
		return;
	}
	current_user_update(page->current_user, DB);
	new_user_team_update(page->new_user_team, DB);
}

static void show_new_file_dialog(GtkWidget *widget, gpointer data)
{
	struct setup_page *page;
	GtkWidget *parent;
	GtkWidget *new_file;
	char *filename;
	FILE* config;
	extern Db_Object DB;
	extern char *FILENAME;
	pid_t child_pid;
	int status;
	char *config_opt;

	page = (struct setup_page *)data;
	parent = gtk_widget_get_toplevel(widget);
	new_file = gtk_file_chooser_dialog_new(
		"Create New Simulation:", GTK_WINDOW(parent),
		GTK_FILE_CHOOSER_ACTION_SAVE,
		GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
		GTK_STOCK_SAVE, GTK_RESPONSE_ACCEPT, NULL);
	if(gtk_dialog_run(GTK_DIALOG(new_file)) == GTK_RESPONSE_ACCEPT){
		filename = gtk_file_chooser_get_filename(
						GTK_FILE_CHOOSER(new_file));
	}
	else{
		gtk_widget_destroy(new_file);
		return;
	}
	gtk_widget_destroy(new_file);
	/*Write config file*/
	config = fopen(filename, "w");
	fprintf(config, "master=%s.db\n", filename);
	fclose(config);
	FILENAME = filename;
	/*Initialize the database*/
	child_pid = fork();
	if(child_pid == 0){
		if(execlp("bash","bash","init_sqlite.sh",filename,NULL) == -1){
			fprintf(stderr, "Error creating saved new database\n");
			exit(EXIT_FAILURE);
		}
		
	}
	wait(&status);
	child_pid = fork();
	config_opt = malloc((strlen(filename) + 9 ) * sizeof(char));
	sprintf(config_opt,"--config=%s", filename);
	if(child_pid == 0){
		if(execlp("perl", "perl", "restart_league.pl", "--league-id=1",
						config_opt, NULL) == -1 ){
			fprintf(stderr,"Error creating new league.\n");
			exit(EXIT_FAILURE);
		}
	}
	wait(&status);
	/*TODO:Check child return value.*/
	/*Open DB connection*/
	/*TODO: Maybe we can have a util function that will open the db
	 * connection, so we aren't doing it in more than one place.
	 */
	DB = db_connect(filename);
	if(!DB){
		return;
	}
	current_user_update(page->current_user, DB);
	new_user_team_update(page->new_user_team, DB);
}

#define SELECT_USERS "Select username, user_id, team_id, league_id from " \
"t_user join t_team on t_user.user_id = t_team.owner_id where active=1"
static void current_user_update(GtkWidget *current_user, Db_Object db)
{
	GtkTreeIter iter;
	GtkTreeIter current_iter;
	GtkListStore *store;
	native_result *result;
	native_row row;
	/*TODO: Save the current iter. */
	store = GTK_LIST_STORE(gtk_combo_box_get_model(
					GTK_COMBO_BOX(current_user)));
	gtk_list_store_clear(store);
	result = db->execute_query_result(db->conn, SELECT_USERS,
					sizeof(SELECT_USERS) -1);
	if(!result){
		fprintf(stderr, "Could not load users from database.\n");
		return;
	}
	while(row = db->fetch_row(result)){
		gtk_list_store_append(store, &iter);
		gtk_list_store_set(store, &iter,
					0, db->get_column_string(row, 0),
					1, db->get_column_int(row, 1),
					2, db->get_column_int(row, 2),
					3, db->get_column_int(row, 3), -1);
	}
}

#define SELECT_FREE_TEAMS "Select mascot, team_id from t_team where active=1 " \
				"and owner_id=0"
static void new_user_team_update(GtkWidget *new_user_team, Db_Object db)
{
	GtkTreeIter iter;
	GtkListStore *store;
	native_result *result;
	native_row row;

	store = GTK_LIST_STORE(gtk_combo_box_get_model(
					GTK_COMBO_BOX(new_user_team)));
	gtk_list_store_clear(store);
	result = db->execute_query_result(db->conn, SELECT_FREE_TEAMS,
					sizeof(SELECT_FREE_TEAMS) - 1);
	if(!result){
		return;
	}
	while(row = db->fetch_row(result)){
		gtk_list_store_append(store, &iter);
		gtk_list_store_set(store, &iter,
					0, db->get_column_string(row, 0),
					1, db->get_column_int(row, 1), -1);
	}
}

static void create_user(GtkWidget *widget, gpointer data)
{
	struct setup_page *page;
	int user_id, team_id;
	extern Db_Object DB;
	const char *user_name;
	GtkWidget *message;
	GtkWidget *main_window;
	GtkTreeIter iter;
	GtkListStore *store;
	GtkListStore *current_user_store;

	if(!DB){
		return;
	}
	/*TODO: Make sure user has selected a team before adding the user.*/
	page = (struct setup_page *)data;
	user_name = gtk_entry_get_text(GTK_ENTRY(page->new_user_text));
	if(!user_name[0]){
		main_window = gtk_widget_get_toplevel(widget);
		message = gtk_message_dialog_new(GTK_WINDOW(main_window),
			GTK_DIALOG_DESTROY_WITH_PARENT,
			GTK_MESSAGE_ERROR,
			GTK_BUTTONS_CLOSE,
			"Please enter a user name.");
			gtk_dialog_run(GTK_DIALOG(message));
			gtk_widget_destroy(message);
			return;
	}
	user_id = insert_user(DB, user_name);
	store = GTK_LIST_STORE(gtk_combo_box_get_model(
					GTK_COMBO_BOX(page->new_user_team)));
	gtk_combo_box_get_active_iter(
				GTK_COMBO_BOX(page->new_user_team), &iter);
	gtk_tree_model_get(GTK_TREE_MODEL(store), &iter, 1, &team_id, -1); 
	update_team_owner(DB, team_id, user_id);
	current_user_update(page->current_user, DB);
	current_user_store = GTK_LIST_STORE(gtk_combo_box_get_model(
					GTK_COMBO_BOX(page->current_user)));
	if(!gtk_combo_box_get_active_iter(
				GTK_COMBO_BOX(page->current_user), &iter)){
		if(gtk_tree_model_get_iter_first(
				GTK_TREE_MODEL(current_user_store), &iter)){
			int temp;
			do{
				gtk_tree_model_get(
					GTK_TREE_MODEL(current_user_store),
					&iter, 1, &temp, -1);
				if(temp == user_id){
					/*TODO: Have current_user_update save
					 * the current iter.*/
					gtk_combo_box_set_active_iter(
						GTK_COMBO_BOX(
							page->current_user),
						&iter);
					break;
				}
			}while(gtk_tree_model_iter_next(
				GTK_TREE_MODEL(current_user_store), &iter));
		}

	}
}

#define INSERT_USER "insert into t_user(username) values('%s')"
static int insert_user(Db_Object db, const char *user)
{
	char query[80];
	int len;
	int id;

	len = snprintf(query, 80, INSERT_USER, user);
	db->execute_query(db->conn, query, len);
	id = db->last_insert_id(db->conn);
	return id;
}

#define UPDATE_TEAM_OWNER "update t_team set owner_id=%d where team_id=%d " \
								"and active=1" 
static void update_team_owner(Db_Object db, int team_id, int user_id)
{
	char query[200];
	int len;
	
	len = snprintf(query, 200, UPDATE_TEAM_OWNER, user_id, team_id);
	db->execute_query(db->conn, query, len);
}


