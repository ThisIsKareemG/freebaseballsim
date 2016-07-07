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
#include <string.h>
#include <argp.h>
#include "fbs_argp.h"
#include "tom_array_list.h"
#include "baseball_data.h"
#include "db_object.h"
#include "baseball_constants.h"

int insert_schedule(Db_Object db,int league_id,int series,int games[4][2], Array_List team_ids){	
	
	int i,j, home_id, away_id, game_num, query_length;
	char query[DEFAULT_QUERY_LENGTH];
	for(j=0;j<4;j++){
		home_id = (int)gget(team_ids,games[j][0]);
		away_id = (int)gget(team_ids,games[j][1]);
		for(i=0;i<GAMES_PER_SERIES;i++){
			game_num = (series * GAMES_PER_SERIES) + i + 1;
			query_length = sprintf(query,INSERT_SCHEDULE,league_id,game_num,home_id,away_id);
			if(db->execute_query(db->conn,query,query_length)){
				return DBERR;
			}
		}
	}
	return 0;
}

void init_weeks(int weeks[14]){
	
	int i;
	for(i=0;i<14;i++){
		weeks[i] = i;
	}
}

void print_weeks(int weeks[14]){
	
	int i;
	for(i=0;i<14;i++){
		printf("%d ",weeks[i]);
	}
	printf("\n");
}

int main(int argc, char** argv){

	int schedule_grid[14][4][2] = {
	
		{{0,1},{2,3},{4,5},{6,7}},
		{{0,2},{1,3},{4,6},{5,7}},
		{{0,3},{1,2},{4,7},{5,6}},
		{{0,4},{1,5},{2,6},{3,7}},
		{{0,5},{1,4},{2,7},{3,6}},
		{{0,6},{3,5},{1,7},{2,4}},
		{{0,7},{1,6},{2,5},{3,4}},
		{{1,0},{3,2},{5,4},{7,6}},
		{{2,0},{3,1},{6,4},{7,5}},
		{{3,0},{2,1},{7,4},{6,5}},
		{{4,0},{5,1},{6,2},{7,3}},
		{{5,0},{4,1},{7,2},{6,3}},
		{{6,0},{5,3},{7,1},{4,2}},
		{{7,0},{6,1},{5,2},{4,3}}
	};
	
	int weeks[14];
	int series = 0;
	int league_id = 1;
	int i;
	int rounds, num;
	Array_List team_ids;

	argp_parse(&global_argp, argc, argv, 0, 0, NULL);
	Db_Object db = db_connect(CONFIG_FILE);
	if(!db){
		return EXIT_FAILURE;
	}
	team_ids = select_team_ids_for_league(db,league_id);
	if(!team_ids){
		return EXIT_FAILURE;
	}
	if(db->begin_transaction(db->conn)){
		return EXIT_FAILURE;
	}
	while(series<54){
		rounds = 14;
		init_weeks(weeks);
		while(rounds>0){
			num = rand()%rounds;
			if(insert_schedule(db,league_id,series,
					schedule_grid[weeks[num]],team_ids)){
				goto rollback;
			}
			weeks[num] = weeks[rounds-1];
			rounds--;
			series++;
			if(series>=54){
				break;
			}
		}
	}
	if(db->commit(db->conn)){
		goto rollback;
	}
	free(db);
	return EXIT_SUCCESS;

rollback:
	db->rollback(db->conn);
	free(db);
	return EXIT_FAILURE;
}
