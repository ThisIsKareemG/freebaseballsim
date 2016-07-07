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
#include <gtk/gtk.h>
#include "pages.h"
#include "db_object.h"
#include "baseball_objects.h"

static GtkWidget *lineup_init();
static GtkWidget *rotation_init();
static void lineup_update(struct roster_page *page, int team_id, Hand hand);
static void rotation_update(struct roster_page *page, int team_id);
static native_result *select_lineup_skill(int team_id, Hand hand);
static native_result *select_rotation_skill(int team_id);

void roster_page_init(struct roster_page *page)
{
	page->main = gtk_table_new(1 , 2, FALSE);
	page->lineup = lineup_init();
	page->rotation = rotation_init();
	
	gtk_table_attach_defaults(GTK_TABLE(page->main), page->lineup,
								0, 1, 0, 1);
	gtk_table_attach_defaults(GTK_TABLE(page->main), page->rotation,
								1, 2, 0, 1);
}


static GtkWidget *lineup_init()
{
	GtkCellRenderer *text_renderer;
	GtkWidget *lineup;
	GtkListStore *store;

	text_renderer = gtk_cell_renderer_text_new();
	/*Create the lineup widget*/
	lineup = gtk_tree_view_new();
	gtk_tree_view_insert_column_with_attributes(GTK_TREE_VIEW(lineup),
				-1, "Name", text_renderer, "text", 0, NULL);
	gtk_tree_view_insert_column_with_attributes(GTK_TREE_VIEW(lineup),
				-1, "CvR", text_renderer, "text", 1, NULL);
	gtk_tree_view_insert_column_with_attributes(GTK_TREE_VIEW(lineup),
				-1, "PvR", text_renderer, "text", 2, NULL);
	gtk_tree_view_insert_column_with_attributes(GTK_TREE_VIEW(lineup),
				-1, "Intl", text_renderer, "text", 3, NULL);
	gtk_tree_view_insert_column_with_attributes(GTK_TREE_VIEW(lineup),
				-1, "Spd", text_renderer, "text", 4, NULL);
	gtk_tree_view_insert_column_with_attributes(GTK_TREE_VIEW(lineup),
				-1, "Lineup", text_renderer, "text", 5, NULL);
	gtk_tree_view_insert_column_with_attributes(GTK_TREE_VIEW(lineup),
				-1, "Order", text_renderer, "text", 6, NULL);
	/*Add the data*/
	store = gtk_list_store_new(7, G_TYPE_STRING, G_TYPE_UINT, G_TYPE_UINT,
			G_TYPE_UINT, G_TYPE_UINT, G_TYPE_UINT, G_TYPE_UINT);
	gtk_tree_view_set_model(GTK_TREE_VIEW(lineup), GTK_TREE_MODEL(store));
	return lineup;
}

static GtkWidget *rotation_init()
{
	GtkCellRenderer *text_renderer;
	GtkWidget *rotation;
	GtkListStore *store;
	
	text_renderer = gtk_cell_renderer_text_new();
	/*Create the rotation widget*/
	rotation = gtk_tree_view_new();
	gtk_tree_view_insert_column_with_attributes(GTK_TREE_VIEW(rotation),
				-1, "Name", text_renderer, "text", 0, NULL);
	gtk_tree_view_insert_column_with_attributes(GTK_TREE_VIEW(rotation),
				-1, "Vel", text_renderer, "text", 1, NULL);
	gtk_tree_view_insert_column_with_attributes(GTK_TREE_VIEW(rotation),
				-1, "Mov", text_renderer, "text", 2, NULL);
	gtk_tree_view_insert_column_with_attributes(GTK_TREE_VIEW(rotation),
				-1, "Ctrl", text_renderer, "text", 3, NULL);
	gtk_tree_view_insert_column_with_attributes(GTK_TREE_VIEW(rotation),
				-1, "Intl", text_renderer, "text", 4, NULL);
	gtk_tree_view_insert_column_with_attributes(GTK_TREE_VIEW(rotation),
				-1, "End", text_renderer, "text", 5, NULL);
	gtk_tree_view_insert_column_with_attributes(GTK_TREE_VIEW(rotation),
				-1, "Energy", text_renderer, "text", 6, NULL);
	/*Add the data*/
	store = gtk_list_store_new(7, G_TYPE_STRING, G_TYPE_UINT,
					G_TYPE_UINT, G_TYPE_UINT, G_TYPE_UINT,
					G_TYPE_UINT,G_TYPE_FLOAT);
	gtk_tree_view_set_model(GTK_TREE_VIEW(rotation),
					GTK_TREE_MODEL(store));
	return rotation;
}

void roster_page_update(GtkWidget *widget, gpointer data)
{
	struct roster_page *page;
	extern int TEAM_ID;

	page = (struct roster_page *)data;
	lineup_update(page, TEAM_ID, RIGHT);
	rotation_update(page, TEAM_ID);
}

static void lineup_update(struct roster_page *page, int team_id, Hand hand)
{
	GtkTreeIter iter;
	GtkTreeModel *model;	
	GtkListStore *store;
	extern Db_Object DB;
	native_result *result;
	native_row row;

	model = gtk_tree_view_get_model(GTK_TREE_VIEW(page->lineup));
	if(!model){
		fprintf(stderr, "Model has not been set.\n");
		return;
	}
	store = GTK_LIST_STORE(model);
	gtk_list_store_clear(store);
	result = select_lineup_skill(team_id, hand);
	if(!result){
		return;
	}
	while(row = DB->fetch_row(result)){
		gtk_list_store_append(store, &iter);
		gtk_list_store_set(store, &iter,
			0, DB->get_column_string(row, 0),
			1, DB->get_column_int(row, 1),
			2, DB->get_column_int(row, 2),
			3, DB->get_column_int(row, 3),
			4, DB->get_column_int(row, 4),
			5, DB->get_column_int(row, 5),
			6, DB->get_column_int(row, 6), -1);
	}
	DB->free_result(result);
}


static void rotation_update(struct roster_page *page, int team_id)
{
	GtkTreeIter iter;
	GtkListStore *store;
	extern Db_Object DB;
	native_result *result;
	native_row row;

	store = GTK_LIST_STORE(gtk_tree_view_get_model(
						GTK_TREE_VIEW(page->rotation)));
	gtk_list_store_clear(store);
	result = select_rotation_skill(team_id);
	if(!result){
		return;
	}
	while(row = DB->fetch_row(result)){
		int endurance = DB->get_column_int(row,5);
		float energy = (100 * DB->get_column_int(row, 6)) / 
					(float)calc_max_batters(endurance);
		gtk_list_store_append(store, &iter);
		gtk_list_store_set(store, &iter,
			0, DB->get_column_string(row, 0),
			1, DB->get_column_int(row, 1),
			2, DB->get_column_int(row, 2),
			3, DB->get_column_int(row, 3),
			4, DB->get_column_int(row, 4),
			5, endurance,
			6, energy,
			-1);
	}
	DB->free_result(result);
}

#define SELECT_LINEUP_SKILL "select last_name,%s,intelligence,spd," \
"t_lineup.position,bat_order,t_lineup.player_id,t_player.position from " \
"t_lineup join t_player on t_lineup.player_id=t_player.player_id join " \
"t_skill_hitter on t_lineup.player_id=t_skill_hitter.player_id where " \
"t_lineup.team_id=%d and t_lineup.hand=%d and t_player.position > 0 " \
"order by (bat_order+9) % 10"

static native_result *select_lineup_skill(int team_id, Hand hand)
{
	extern Db_Object DB;
	char query[500];
	char hand_skills[8];
	int len;
	native_result *result;

	switch(hand){
	case RIGHT:
		sprintf(hand_skills, "CvR,PvR");
		break;
	case LEFT:
		sprintf(hand_skills, "CvL,PvL");
		break;
	default:
		fprintf(stderr, "Unkown hand type.\n");
		return NULL;
	}
	len = sprintf(query, SELECT_LINEUP_SKILL, hand_skills, team_id, hand);
	result = DB->execute_query_result(DB->conn, query, len);
	return result;
}

#define SELECT_ROTATION_SKILL "select last_name, vel, mov, ctrl, " \
"intelligence, end, batters_left from t_rotation join t_player on " \
"t_rotation.player_id=t_player.player_id join t_skill_pitcher on " \
"t_rotation.player_id=t_skill_pitcher.player_id where t_rotation.team_id=%d"

static native_result *select_rotation_skill(int team_id)
{
	extern Db_Object DB;
	char query[320];
	int len;
	
	len = sprintf(query, SELECT_ROTATION_SKILL, team_id);
	return DB->execute_query_result(DB->conn, query, len);
}
