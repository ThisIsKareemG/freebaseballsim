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

#define SELECT_DAY_NUM "select wins+loses from t_team where active=1"
static void sim_days(GtkWidget *widget, gpointer data)
{
	extern Db_Object DB;
	extern int LEAGUE_ID;
	extern char *FILENAME;
	native_result *result;
	native_row row;
	int day;
	pid_t child_pid;
	char league_arg[30];
	char day_arg[20];
	char config_arg[200];	
	/*HACK: Temporary way to get next day number*/
	result = DB->execute_query_result(DB->conn, SELECT_DAY_NUM,
						sizeof(SELECT_DAY_NUM) - 1);
	row = DB->fetch_row(result);
	day = DB->get_column_int(row, 0);
	DB->free_result(result);
	day++;
	/*END HACK*/
	sprintf(league_arg, "--league-id=%d", LEAGUE_ID);
	sprintf(day_arg, "--day=%d", day);
	sprintf(config_arg, "--config=%s", FILENAME);
	child_pid = fork();
	if(child_pid == 0){
		if(execlp("./sim_day", "sim_day", league_arg, day_arg,
						config_arg, NULL) == -1){
			fprintf(stderr, "Error simulating day %d with args "
			"%s %s %s\n", day, league_arg, day_arg, config_arg);
		}
		exit(EXIT_FAILURE);
	}
	else{
		wait(child_pid);
		/*TODO: Check return value of child. */
	}
}

void simulate_page_init(struct simulate_page *page)
{
	GtkWidget *simulate;

	simulate = gtk_button_new_with_label("Play Next Game");
	g_signal_connect(G_OBJECT(simulate), "clicked", G_CALLBACK(sim_days),
									NULL);	
	page->main = simulate;
}

