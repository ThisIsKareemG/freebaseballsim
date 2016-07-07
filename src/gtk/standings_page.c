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

static native_result *select_standings(Db_Object db, int league_id);

void standings_page_init(struct standings_page *page)
{
	GtkCellRenderer *text_renderer;
	GtkListStore *store;

	text_renderer = gtk_cell_renderer_text_new();
	page->main = gtk_tree_view_new();
	gtk_tree_view_insert_column_with_attributes(GTK_TREE_VIEW(page->main),
				-1, "Team", text_renderer, "text", 0, NULL);
	gtk_tree_view_insert_column_with_attributes(GTK_TREE_VIEW(page->main),
				-1, "Wins", text_renderer, "text", 1, NULL);
	gtk_tree_view_insert_column_with_attributes(GTK_TREE_VIEW(page->main),
				-1, "Loses", text_renderer, "text", 2, NULL);
	store = gtk_list_store_new(3, G_TYPE_STRING, G_TYPE_UINT, G_TYPE_UINT);
	gtk_tree_view_set_model(GTK_TREE_VIEW(page->main),
							GTK_TREE_MODEL(store));
}

void standings_page_update(GtkWidget *widget, gpointer data)
{
	struct standings_page *page;
	GtkTreeIter iter;
	GtkListStore *store;
	extern Db_Object DB;
	extern int LEAGUE_ID;
	native_result *result;
	native_row row;

	page = (struct standings_page *)data;
	store = GTK_LIST_STORE(gtk_tree_view_get_model(
						GTK_TREE_VIEW(page->main)));
	gtk_list_store_clear(store);
	result = select_standings(DB, LEAGUE_ID);
	if(!result){
		return;
	}
	while(row = DB->fetch_row(result)){
		gtk_list_store_append(store, &iter);
		gtk_list_store_set(store, &iter,
			0, DB->get_column_string(row, 0),
			1, DB->get_column_int(row, 1),
			2, DB->get_column_int(row, 2), -1);
	}
	DB->free_result(result);
}

#define SELECT_STANDINGS "select mascot, wins, loses from t_team " \
"where league_id=%d and active=1 order by wins"

static native_result *select_standings(Db_Object db, int league_id)
{
	char query[100];
	int len;
	
	len = sprintf(query, SELECT_STANDINGS, league_id);
	return db->execute_query_result(db->conn, query, len);
}
