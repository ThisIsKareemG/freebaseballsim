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

/*! \file run_draft.c */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <argp.h>
#include <string.h>
#include "fbs_argp.h"
#include "tom_array_list.h"
#include "linked_list.h"
#include "baseball_objects.h"
#include "draft_pref.h"
#include "baseball_data.h"
#include "baseball_macros.h"
#include "baseball_constants.h"
#include "db_object.h"

struct arguments{
	int seed;
};

static Array_List select_draft_prefs_for_league(Db_Object db, int league_id);
static int draft_player(Db_Object db, Draft_Pref pref, int position, int pick,
							int year, int round);
static void remove_player(Array_List draft_pref_list, int player_id,
								int position);
static error_t arg_parser(int key, char *arg, struct argp_state *state);

int main(int argc,char** argv)
{
	int c,i,year,j,k,pick,current_round_overall,rounds,drafted_player;
	int num_chars;
	char query[DEFAULT_QUERY_LENGTH];
	Db_Object db;
	Array_List draft_pref_list;
	int option_index = 0;
	int league_id = 1;	
	int number_of_teams = 8;
	struct argp_option options[] = {
		{"seed",'s', "int", 0, 0},
		{0}
	};
	struct arguments args = {getpid()}; 
	struct argp argp = {options, arg_parser, 0, 0, global_child};
	argp_parse(&argp, argc, argv, 0, 0, &args);
	
	srand(args.seed);
	fprintf(stderr,"Seed: %d\n",args.seed);

	db = db_connect(CONFIG_FILE);
	if(!db){
		return EXIT_FAILURE;
	}
	year = select_year_for_league(db, league_id);
	draft_pref_list = select_draft_prefs_for_league(db,league_id);
	current_round_overall = 1;
	/* int starting_point = rand()%draft_pref_list->length;
	 * randomize_list(circular_team_list,number_of_teams);
	 * gprint(draft_pref_list,Draft_Pref_print);
	 */
	db->begin_transaction(db->conn);
	for(i=0;i<10;i++){
		pick = 1;
		switch(i){
		case 0: 
			rounds = 6;
			break;	

		case 1:
			rounds = 5;
			break;

		default:
			rounds = 2;
			break;
		}
		for(k=0;k<rounds;k++){
			for(j=0;j<number_of_teams;j++){
				drafted_player = draft_player(db, (Draft_Pref)
							gget(draft_pref_list,j),
							i,pick,year,
							current_round_overall);
				if(drafted_player == -1){
					fprintf(stderr,
					"Could not draft player: RD=%d Pick=%d\n",
					current_round_overall,
					pick);
					return EXIT_FAILURE;
				}


				remove_player(draft_pref_list,drafted_player,i);
				pick++;
			}
			current_round_overall++;
		}
	}
	/* Set the status of undrafted players to free agent */
	num_chars = sprintf(query,
			"Update t_player set status=%d where status=%d;",
			FA,DRAFTEE);
	db->execute_query(db->conn,query,num_chars);
	db->commit(db->conn);
	db->close_connection(db->conn);
	return 0;		
}

static Array_List select_draft_prefs_for_league(Db_Object db, int league_id)
{
	char query[DEFAULT_QUERY_LENGTH];
	int j,i,x,position,num_chars,player_id;
	Draft_Pref pref;
	native_result *result;
	native_row row;
	Array_List team_list = select_team_ids_for_league(db, league_id);
	Array_List draft_list = Array_List_create(sizeof(struct linked_list**));
	for(i=0;i<team_list->length;i++){
		pref = Draft_Pref_create((int)gget(team_list,i));
		Array_List_add(draft_list,pref);
	}
	for(position=0;position<10;position++){
		num_chars = sprintf(query,SELECT_DRAFT_PREFS_FOR_LEAGUE,
					league_id,position);
		result = db->execute_query_result(db->conn,query,num_chars);
		while(row = db->fetch_row(result)){
			player_id = db->get_column_int(row,0);
			for(j=0;j<team_list->length;j++){
				Linked_List_append(((Draft_Pref)
					gget(draft_list,j))->prefs+position,
					(void*)player_id);	
			}
		}
		db->free_result(result);
	}
	return draft_list;
}

/*!
 * @return A positive integer (might be 0) representing the ID of the player
 * that was drafted.  If there is an error -1 is returned.
 */
static int draft_player(Db_Object db, Draft_Pref pref,
				int position, int pick, int year, int round)
{
	struct linked_list *draft_list = pref->prefs[position];
	if(!draft_list){
		return -1;
	}
	int player_id = (int)(draft_list)->value;
	insert_draft_pick(db,pref->team_id,player_id,position,pick,year, round);
#ifdef DEBUG
	Skill skill = (Skill)dgetValue(skill_dict,(void*)player_id);
	fprintf(stderr,
		"Team: %d selects cvr: %d pvr: %d cvl: %d pvl: %d at pos: %d\n",
		pref->team_id,skill->cvr,skill->pvr,skill->cvl,skill->pvl,
		position);
#endif
	return player_id;
}

static void remove_player(Array_List draft_pref_list, int player_id,
								int position)
{
	int i;
	struct linked_list **player_list;
	Draft_Pref pref;
	for(i=0;i<draft_pref_list->length;i++){
		pref = (Draft_Pref)gget(draft_pref_list, i);
		player_list = pref->prefs + position;
		Linked_List_remove_value(player_list, (void*)player_id);
	}
}	

static error_t arg_parser(int key, char *arg, struct argp_state *state)
{
	struct arguments *args = state->input;
	switch(key){
	case 's':
		args->seed = atoi(arg);
		break;
	defaut:
		return ARGP_ERR_UNKNOWN;
	}
	return 0;
}
