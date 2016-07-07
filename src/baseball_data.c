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
#include <math.h>
#include "tom_print.h"
#include "tom_array_list.h"
#include "tom_math.h"
#include "linked_list.h"
#include "key_value.h"
#include "compiler_macros.h"
#include "baseball_objects.h"
#include "baseball_constants.h"
#include "baseball_macros.h"
#include "hash.h"
#include "db_object.h"
#include "baseball_data.h"

const char* INSERT_SCHEDULE = "INSERT into t_schedule(league_id,game_number,home_id,away_id)values(%d,%d,%d,%d);";
const char* INSERT_PLAYER = "Insert into t_player(first_name,last_name,dob,position,intelligence,health,bats,throws,league_id,team_id,status,overall) values('%s','%s','%s',%d,%d,%d,%d,%d,%d,%d,%d,%d);";
const char* INSERT_SKILLS = "insert into t_skill_hitter(player_id,cvr,pvr,cvl,pvl,spd,energy) values(%d,%d,%d,%d,%d,%d,%d); insert into t_skill_pitcher(player_id,vel,mov,ctrl,end,batters_left,energy) values(%d,%d,%d,%d,%d,%d,%d); insert into t_skill_fielder(player_id,range,arm,field) values(%d,%d,%d,%d);";
const char* SELECT_DRAFT_PREFS_FOR_LEAGUE = "Select player_id FROM t_player WHERE league_id = %d AND position = %d AND status = 0 order by overall desc,intelligence desc,player_id desc;";
const char* SELECT_DATE_FOR_LEAGUE = "Select league_date from t_league where league_id = %d;";
const char* INSERT_DRAFT_PICK = "insert into t_draft (team_id,player_id,round,pick,year) values(%d,%d,%d,%d,%d); update t_player set team_id = %d, status = %d WHERE player_id = %d;";
const char* INSERT_LINEUP = "insert into t_lineup(team_id,player_id,bat_order,position,hand) values(%d,%d,0,0,1); insert into t_lineup(team_id,player_id,bat_order,position,hand) values(%d,%d,0,0,0);";
const char* INSERT_ROTATION = "insert into t_rotation(team_id,player_id,position,next_starter) values(%d,%d,%d,0);";
const char* INSERT_STAT_HITTER = "insert into t_stat_hitter(player_id,hand,year,team_id) values(%d,0,%d,%d); insert into t_stat_hitter(player_id,hand,year,team_id) values(%d,1,%d,%d);";
const char* INSERT_STAT_PITCHER = "insert into t_stat_pitcher(player_id,hand,year,team_id) values(%d,0,%d,%d); insert into t_stat_pitcher(player_id,hand,year,team_id) values(%d,1,%d,%d);";
const char* INSERT_STAT_FIELDER = "insert into t_stat_fielder(player_id,year,team_id) values (%d,%d,%d);";
const char* SELECT_BL_END = "select t_skill_pitcher.player_id, batters_left, end from t_skill_pitcher inner join t_player on t_skill_pitcher.player_id = t_player.player_id  where league_id = %d and position<2;";
const char* UPDATE_BATTERS_LEFT = "update t_skill_pitcher set batters_left = %d where player_id = %d;";
const char* SELECT_SCHEDULE = "Select home_id,away_id from t_schedule where league_id = %d AND game_number = %d;";
const char* SELECT_SKILLS_FOR_GAME = "SELECT t_player.player_id, cvr, pvr, cvl, pvl, spd, t_skill_hitter.energy as hitter_energy, vel, mov, ctrl, end, t_skill_pitcher.energy as pitcher_energy, range, arm, field, one, two, three, four, five, six, seven, eight, nine, intelligence, bats, throws, last_name, position, batters_left FROM t_player inner join t_skill_hitter on t_player.player_id = t_skill_hitter.player_id inner join t_skill_pitcher on t_player.player_id = t_skill_pitcher.player_id inner join t_skill_fielder on t_player.player_id = t_skill_fielder.player_id WHERE team_id = %d OR team_id = %d;";
const char* SELECT_LINEUP = "SELECT player_id,bat_order,position From t_lineup WHERE team_id=%d and hand=%d;";
const char* SELECT_ROTATION = "SELECT t_rotation.player_id, next_starter FROM t_rotation inner join t_player on t_rotation.player_id = t_player.player_id inner join t_skill_pitcher on t_player.player_id = t_skill_pitcher.player_id where t_rotation.team_id = %d order by overall desc, end desc;";
const char* SELECT_SKILLS_FOR_LEAGUE = "SELECT t_player.player_id, cvr, pvr, cvl, pvl, spd, t_skill_hitter.energy as hitter_energy, vel, mov, ctrl, end, t_skill_pitcher.energy as pitcher_energy, range, arm, field, one, two, three, four, five, six, seven, eight, nine, intelligence, bats, throws, last_name, position, batters_left FROM t_player left join t_skill_hitter on t_player.player_id = t_skill_hitter.player_id left join t_skill_pitcher on t_player.player_id = t_skill_pitcher.player_id left join t_skill_fielder on t_player.player_id = t_skill_fielder.player_id WHERE league_id = %d and team_id > 0;";

const char* UPDATE_YEAR = "update t_league set league_date='%s' where league_id=%d;";

const char* SELECT_LINEUPS_FOR_LEAGUE = "Select team_id,player_id,bat_order,position,from t_lineup where league_id=%d;";

const char* SELECT_ROTATION_FOR_LEAGUE = "select team_id,player_id,next_starter from t_rotation where league_id=%d;";

int calc_max_batters(int endurance){

	return 3 + round(3.0*endurance/10.0);
}

int calc_overall(int position,Hitter_Skill hitter_skill,Pitcher_Skill pitcher_skill){
	
	if(position<2){
		return (pitcher_skill->vel+pitcher_skill->mov+pitcher_skill->ctrl)/3;
	}
	else{
		return (hitter_skill->cvr + hitter_skill->pvr + hitter_skill->cvl + hitter_skill->pvl)/4;
	}
}

#define SELECT_TEAM_IDS_FOR_LEAGUE "SELECT team_id FROM t_team WHERE league_id  = %d and active=1 order by team_id;"
Array_List select_team_ids_for_league(Db_Object db, int league_id){
	
	char query[DEFAULT_QUERY_LENGTH];
	int query_length;
	native_result* result;
	Array_List team_list;
	native_row row;
	
	query_length = sprintf(query,SELECT_TEAM_IDS_FOR_LEAGUE,league_id);
	result = db->execute_query_result(db->conn,query,query_length);
	if(!result){
		return NULL;
	}
	team_list = Array_List_create(sizeof(int));
	while((row=db->fetch_row(result))){
		Array_List_add(team_list,(void*)db->get_column_int(row,0));
	}
	db->free_result(result);	
	db->next_result(db->conn);
	return team_list;
}

#define SELECT_NAMES "SELECT first,last from t_name"

void select_names(Db_Object db, Array_List first_names, Array_List last_names){

	native_result* result = db->execute_query_result(db->conn,SELECT_NAMES,sizeof(SELECT_NAMES) - 1);
	native_row row;
	while((row = db->fetch_row(result))){
		Array_List_add(first_names,(void*)db->get_column_string(row,0));
		Array_List_add(last_names,(void*)db->get_column_string(row,1));	
	}
	db->free_result(result);
}

void insert_player(Db_Object db, Player player,Hitter_Skill hitter_skill,Pitcher_Skill pitcher_skill,Fielder_Skill fielder_skill,int league_id){
	
#ifdef _PRINT_NEWPLAYER_
	fprintf(stderr,"New Player CvR=%d PvR=%d CvL=%d PvL=%d spd=%d int=%d vel=%d mov=%d ctrl=%d end=%d rng=%d arm=%d fld=%d pos=%d\n",
	hitter_skill->cvr,hitter_skill->pvr,hitter_skill->cvl,hitter_skill->pvl,
	hitter_skill->spd,player->intelligence, pitcher_skill->vel, 
	pitcher_skill->mov, pitcher_skill->ctrl,pitcher_skill->end, 
	fielder_skill->range, fielder_skill->arm, fielder_skill->field,
	player->position);
#endif
	
	char query[4*DEFAULT_QUERY_LENGTH];
	int num_char = sprintf(query,INSERT_PLAYER,
		player->first_name,
		player->last_name,
		"Not Implemented",/*DOB*/
		player->position,
  		player->intelligence,
		0,/*player->health*/
		player->bats,
  		player->throws,
		league_id,
		0,/*team_id*/
		DRAFTEE,
		calc_overall(player->position,hitter_skill,pitcher_skill));
	db->execute_query(db->conn,query,num_char);
	int player_id = db->last_insert_id(db->conn);
	num_char = sprintf(query,INSERT_SKILLS,
		player_id,
  		hitter_skill->cvr,
		hitter_skill->pvr,
  		hitter_skill->cvl,
		hitter_skill->pvl,
  		hitter_skill->spd,
		0, /*energy*/
		player_id,
		pitcher_skill->vel,
  		pitcher_skill->mov,
		pitcher_skill->ctrl,
  		pitcher_skill->end,
		calc_max_batters(pitcher_skill->end),
		0, /*energy*/
		player_id,
		fielder_skill->range,
  		fielder_skill->arm,
		fielder_skill->field
	);
	db->execute_query(db->conn,query,num_char);
}
	
Date select_date_for_league(Db_Object db, int league_id){
	
	char query[DEFAULT_QUERY_LENGTH];
	int num_chars = sprintf(query,SELECT_DATE_FOR_LEAGUE,league_id);
	native_result* result = db->execute_query_result(db->conn,query,num_chars);
	native_row row = db->fetch_row(result);
	Date date = db->get_column_date(row, 0);
	db->free_result(result);

	return date; 
}

int select_year_for_league(Db_Object db, int league_id){

	Date date = select_date_for_league(db, league_id);
	return date->year;
}

void insert_stats(Db_Object db, int player_id, int team_id, int position, int year){
	
	char query[3*DEFAULT_QUERY_LENGTH];
	int num_chars;
	num_chars = sprintf(query,INSERT_STAT_HITTER,player_id,year,team_id,player_id,year,team_id);
	db->execute_query(db->conn,query,num_chars);
	num_chars = sprintf(query,INSERT_STAT_FIELDER,player_id,year,team_id);
	db->execute_query(db->conn,query,num_chars);
	if(position < 2){
		num_chars = sprintf(query,INSERT_STAT_PITCHER,player_id,year,team_id,player_id,year,team_id);
		db->execute_query(db->conn,query,num_chars);
	}
}

void insert_draft_pick(Db_Object db, int team_id,int player_id,int position,int pick,int year, int round){
	
	char query[3*DEFAULT_QUERY_LENGTH];
	int num_chars = sprintf(query,INSERT_DRAFT_PICK,
		team_id,
		player_id,
		round,
		pick,
		year,
		
		team_id,
		ROSTER,
		player_id
	);
	db->execute_query(db->conn,query,num_chars);
	num_chars = sprintf(query,INSERT_LINEUP,team_id,player_id,team_id,player_id);
	db->execute_query(db->conn,query,num_chars);
	insert_stats(db, player_id, team_id, position, year);
	if(position < 2){
		num_chars = sprintf(query,INSERT_ROTATION,team_id,player_id,position);
		db->execute_query(db->conn,query,num_chars);
	}
}

void rest_pitchers(Db_Object db, int league_id){

	char query[4*DEFAULT_QUERY_LENGTH];
	int num_chars = sprintf(query,SELECT_BL_END,league_id);
	native_result* result = db->execute_query_result(db->conn,query,num_chars);
	int i;
	native_row row;
	Array_List list = Array_List_create(sizeof(Key_Value));
	while(row = db->fetch_row(result)){
		int player_id = db->get_column_int(row,0);
		int batters_left = db->get_column_int(row,1);
		int endurance = db->get_column_int(row,2);
		int max_batters = calc_max_batters(endurance);
		int new_batters_left = batters_left + divide_ceiling(max_batters,4);
		if(new_batters_left> max_batters){
			new_batters_left = max_batters;
		}
		Key_Value key_value = malloc(sizeof(struct key_value));
		key_value->key = (void*)player_id;
		key_value->value = (void*)new_batters_left;
		Array_List_add(list,key_value);
	}
	db->free_result(result);
	for(i=0;i<list->length;i++){
		Key_Value key_value = gget(list,i);
		num_chars = sprintf(query,UPDATE_BATTERS_LEFT,key_value->value,key_value->key);
		db->execute_query(db->conn,query,num_chars);
		free(key_value);
	}
}

void update_rotation(Db_Object db, int league_id){
	
	char query[DEFAULT_QUERY_LENGTH];
	const char* SELECT_CURRENT_STARTERS = "select t_rotation.team_id,position from t_rotation join t_team on t_rotation.team_id = t_team.team_id where next_starter = 1 AND league_id = %d and t_team.active = 1;";
	int query_len = sprintf(query,SELECT_CURRENT_STARTERS,league_id);
	native_result* result = db->execute_query_result(db->conn,query,query_len);
	native_row row;
	Array_List list = Array_List_create(sizeof(Key_Value));
	while(row = db->fetch_row(result)){
		int team_id = db->get_column_int(row,0);
		int position = db->get_column_int(row,1);
		PRINT_MESSAGE(1,"team_id: %d starter: %d\n",team_id,position);
		Key_Value starter = malloc(sizeof(struct key_value));
		starter->key = (void*)team_id;
		starter->value = (void*)position;
		PRINT_MESSAGE(1,"starter,key: %d, starter.value: %d\n",starter->key,starter->value);
		Array_List_add(list,starter);
	}
	if(list->length>8){
		exit(1);
	}
	db->free_result(result);
	const char* RESET_NEXT_STARTER = "Update t_rotation set next_starter = 0 where team_id = %d; update t_lineup set bat_order=0, position=0 where team_id=%d and position=1;";
	const char* SELECT_NEW_STARTER = "select player_id from t_rotation where team_id = %d and position = %d;";
	const char* UPDATE_ROTATION = "update t_rotation set next_starter = 1 where team_id = %d and position = %d; update t_lineup set position =1, bat_order=9 where player_id=%d;";
	int i;
	for(i=0;i<list->length;i++){
		int team_id = (int)((Key_Value)gget(list,i))->key;
		int position = (int)((Key_Value)gget(list,i))->value;
		position = (position%5) + 1;
		query_len = sprintf(query,RESET_NEXT_STARTER,team_id,team_id);
		db->execute_query(db->conn,query,query_len);
		int query_len = sprintf(query,SELECT_NEW_STARTER,team_id,position);
		result = db->execute_query_result(db->conn,query,query_len);
		row = db->fetch_row(result);
		int new_starter_id = db->get_column_int(row,0);
		db->free_result(result);
		query_len = sprintf(query,UPDATE_ROTATION,team_id,position,new_starter_id);
		db->execute_query(db->conn,query,query_len);		
	}
	Array_List_free(list,1);
}

int **select_schedule(Db_Object db, int league_id, int game_number){
	
	int num_games = 4;
	char query[DEFAULT_QUERY_LENGTH];
	int i,j,query_len;
	int *games;
	native_result *res;
	native_row row;
	
	/* games will be returned as an int**, so we will use the first
	 * num_games indexes of games to store pointers to an array of
	 * length 2, and the values of these arrays will be stored in the rest
	 * of the games array.  This is done to avoid an annoying initialization
	 * loop that is often necessary for multi-dimensional arrays.
	 */
	games = malloc(num_games * 3 * sizeof(int));
	query_len = sprintf(query,SELECT_SCHEDULE,league_id,game_number);
	res = db->execute_query_result(db->conn,query,query_len);
	i =0;
	while((row = db->fetch_row(res))){
		games[num_games + (2 * i) + 0] = db->get_column_int(row,0);
		games[num_games + (2 * i) + 1] = db->get_column_int(row,1);
		games[i] = (int)(games + num_games + (2 * i));
		i++;
	}
	db->free_result(res);
	return (int**)games;
}

void select_skills(Db_Object db, int home_id, int away_id, Hash skill_hash){

	char query[1000];
	int query_len = sprintf(query,SELECT_SKILLS_FOR_GAME,home_id,away_id);
	native_result* result = db->execute_query_result(db->conn,query,query_len);
	native_row row;
	while((row = db->fetch_row(result))){
        	Skill skill = build_skill(db, row);
            Hash_add_key(skill_hash,(void*)skill->id,skill);
    }
    db->free_result(result);
}

native_result* select_lineup(Db_Object db, int team_id, Hand opp_hand){
	
	char query[DEFAULT_QUERY_LENGTH];
	
	int query_len = sprintf(query,SELECT_LINEUP,team_id,opp_hand);
	native_result* result = db->execute_query_result(db->conn,query,query_len);
	return result;
}

native_result* select_rotation(Db_Object db, int team_id){

	char query[DEFAULT_QUERY_LENGTH];	
	int query_len = sprintf(query, SELECT_ROTATION, team_id);
	native_result* result = db->execute_query_result(db->conn,query,query_len);
	return result;
	
}

Skill build_skill(Db_Object db, native_row row){

        Skill skill = malloc(sizeof(struct skill));
        skill->id = db->get_column_int(row,0);
        skill->cvr = db->get_column_int(row,1);
        skill->pvr = db->get_column_int(row,2);
        skill->cvl = db->get_column_int(row,3);
        skill->pvl = db->get_column_int(row,4);
        skill->spd = db->get_column_int(row,5);
        skill->h_energy = db->get_column_int(row,6);

        skill->vel = db->get_column_int(row,7);
        skill->mov = db->get_column_int(row,8);
        skill->ctrl = db->get_column_int(row,9);
        skill->end = db->get_column_int(row,10);
        skill->batters_left = db->get_column_int(row,11);

        skill->range = db->get_column_int(row,12);
        skill->arm = db->get_column_int(row,13);
        skill->field = db->get_column_int(row,14);
        skill->one = db->get_column_int(row,15);
        skill->two = db->get_column_int(row,16);
        skill->three = db->get_column_int(row,17);
        skill->four = db->get_column_int(row,18);
        skill->five = db->get_column_int(row,19);
        skill->six = db->get_column_int(row,20);
        skill->seven = db->get_column_int(row,21);
        skill->eight = db->get_column_int(row,22);
        skill->nine = db->get_column_int(row,23);

        skill->intelligence = db->get_column_int(row,24);
        skill-> bats = (Hand)db->get_column_int(row,25);
        skill-> throws = (Hand)db->get_column_int(row,26);

        skill->last_name = db->get_column_string(row,27);
        skill->position = db->get_column_int(row,28);
        skill->batters_left = db->get_column_int(row,29);

        skill->stamina = 0;
	
	return skill;
}
