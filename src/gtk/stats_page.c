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
#include "gtk_util.h"
#include "pages.h"
#include "db_object.h"

static GtkWidget *hitter_stats_init();
static GtkWidget *pitcher_stats_init();
static void hitter_stats_update(GtkWidget *hitter_stats, Db_Object db,
								int team_id);
static void pitcher_stats_update(GtkWidget *pitcher_stats, Db_Object db,
								int team_id);
static native_result *select_hitter_stats(Db_Object db, int team_id);
static native_result *select_pitcher_stats(Db_Object db, int team_id);


void stats_page_init(struct stats_page *page)
{
	page->main = gtk_table_new(1, 2, FALSE);
	page->hitter_stats = hitter_stats_init();
	page->pitcher_stats = pitcher_stats_init();

	gtk_table_attach_defaults(GTK_TABLE(page->main), page->hitter_stats,
								0, 1, 0, 1);
	gtk_table_attach_defaults(GTK_TABLE(page->main), page->pitcher_stats,
								1, 2, 0, 1);
}

static GtkWidget *hitter_stats_init()
{
	GtkCellRenderer *text_renderer;
	GtkWidget *hitter_stats;
	GtkListStore *store;

	const char *names[] = {
		"Name", "G", "AB", "H", "R", "2B", "3B", "HR", "RBI", "SO",
		"BB", "BA", NULL
	};
	text_renderer = gtk_cell_renderer_text_new();
	hitter_stats = gtk_tree_view_new();
	fbs_gtk_tree_view_create_columns(GTK_TREE_VIEW(hitter_stats),
							names, text_renderer);
	store = gtk_list_store_new(12, G_TYPE_STRING, G_TYPE_UINT, G_TYPE_UINT,
		G_TYPE_UINT, G_TYPE_UINT, G_TYPE_UINT, G_TYPE_UINT,
		G_TYPE_UINT, G_TYPE_UINT, G_TYPE_UINT, G_TYPE_UINT,
		G_TYPE_FLOAT);
	gtk_tree_view_set_model(GTK_TREE_VIEW(hitter_stats),
							GTK_TREE_MODEL(store));
	return hitter_stats;
}

static GtkWidget *pitcher_stats_init()
{
	GtkCellRenderer *text_renderer;
	GtkWidget *pitcher_stats;
	GtkListStore *store;
	
	const char *names[] = {
		"Name", "IP", "H", "BB", "R", "ER", "SO", "ERA", NULL
	};
	text_renderer = gtk_cell_renderer_text_new();
	pitcher_stats = gtk_tree_view_new();
	fbs_gtk_tree_view_create_columns(GTK_TREE_VIEW(pitcher_stats),
							names, text_renderer);
	store = gtk_list_store_new(8, G_TYPE_STRING, G_TYPE_FLOAT, G_TYPE_INT,
		G_TYPE_INT, G_TYPE_INT, G_TYPE_INT, G_TYPE_INT, G_TYPE_FLOAT);
	gtk_tree_view_set_model(GTK_TREE_VIEW(pitcher_stats),
							GTK_TREE_MODEL(store));
	return pitcher_stats;
}

void stats_page_update(GtkWidget *widget, gpointer data)
{
	extern Db_Object DB;
	extern int TEAM_ID;
	struct stats_page *page = (struct stats_page *)data;
	hitter_stats_update(page->hitter_stats, DB, TEAM_ID);
	pitcher_stats_update(page->pitcher_stats, DB, TEAM_ID);
}

static void hitter_stats_update(GtkWidget *hitter_stats, Db_Object db,
								int team_id)
{
	GtkListStore *store;
	native_result *result;
	native_row row;
	GtkTreeIter iter;
	store = GTK_LIST_STORE(gtk_tree_view_get_model(
						GTK_TREE_VIEW(hitter_stats)));
	gtk_list_store_clear(store);
	result = select_hitter_stats(db, team_id);
	if(!result){
		return;
	}
	while(row = db->fetch_row(result)){
		int hits = db->get_column_int(row, 3);
		int at_bats = db->get_column_int(row, 2);
		float batting_average = hits / (float)at_bats;
		gtk_list_store_append(store, &iter);
		gtk_list_store_set(store, &iter,
			0, db->get_column_string(row, 0),
			1, db->get_column_int(row, 1),
			2, db->get_column_int(row, 2),
			3, hits,
			4, at_bats,
			5, db->get_column_int(row, 5),
			6, db->get_column_int(row, 6),
			7, db->get_column_int(row, 7),
			8, db->get_column_int(row, 8),
			9, db->get_column_int(row, 9),
			10, db->get_column_int(row, 10),
			11, batting_average, -1);
	}
	db->free_result(result);
}

static void pitcher_stats_update(GtkWidget *pitcher_stats, Db_Object db,
								int team_id)
{
	GtkListStore *store;
	native_result *result;
	native_row row;
	GtkTreeIter iter;
	store = GTK_LIST_STORE(gtk_tree_view_get_model(
						GTK_TREE_VIEW(pitcher_stats)));
	gtk_list_store_clear(store);
	result = select_pitcher_stats(db, team_id);
	if(!result){
		return;
	}
	while(row = db->fetch_row(result)){
		int outs = db->get_column_int(row, 1);
		float ip = (outs / 3) + (.1 * (outs % 3));
		int er = db->get_column_int(row, 5);
		float era = (er * 27) / (float)outs;
		gtk_list_store_append(store, &iter);
		gtk_list_store_set(store, &iter,
			0, db->get_column_string(row, 0), /* last name */
			1, ip,
			2, db->get_column_int(row, 2), /* H */
			3, db->get_column_int(row, 3), /* BB */
			4, db->get_column_int(row, 4), /* R */
			5, er,
			6, db->get_column_int(row, 5), /* SO */
			7, era, -1);
	}
	db->free_result(result);
}

#define SELECT_HITTER_STATS "Select last_name,sum(g),sum(ab),sum(h),sum(r)," \
"sum(zb),sum(eb),sum(hr),sum(rbi),sum(so),sum(bb)" \
"from t_player inner join t_stat_hitter on t_player.player_id=" \
"t_stat_hitter.player_id where t_player.team_id=%d group by " \
"t_stat_hitter.player_id"
static native_result *select_hitter_stats(Db_Object db, int team_id)
{
	char query[sizeof(SELECT_HITTER_STATS) + 10];
	int len;

	len = sprintf(query, SELECT_HITTER_STATS, team_id);
	return db->execute_query_result(db->conn, query, len);
}

#define SELECT_PITCHER_STATS "select last_name, sum(outs), sum(h), sum(bb)," \
"sum(r), sum(er) from t_player inner join t_stat_pitcher on " \
"t_player.player_id=t_stat_pitcher.player_id where t_player.team_id=%d " \
"group by t_stat_pitcher.player_id"
static native_result *select_pitcher_stats(Db_Object db, int team_id)
{
	char query[sizeof(SELECT_PITCHER_STATS) + 10];
	int len;

	len = sprintf(query, SELECT_PITCHER_STATS, team_id);
	return db->execute_query_result(db->conn, query, len);
}
