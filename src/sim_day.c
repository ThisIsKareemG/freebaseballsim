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

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <argp.h>
#include <math.h>
#include <signal.h>
#include "fbs_argp.h"
#include "compiler_macros.h"
#include "baseball_constants.h"
#include "baseball_macros.h"
#include "baseball_data.h"
#include "sim_game.h"
#include "tom_math.h"
#include "db_object.h"
#include "key_value.h"

int FD;
static int HTML = 0;


void rest_pitchers_speed(Hash skill_hash){

	int i;
	for(i=0;i<skill_hash->capacity;i++){
		struct linked_list *temp = skill_hash->items[i];
		while(temp!=NULL){
			int print_done = 0;
			Key_Value key_value = temp->value;
			Skill skill = key_value->value;
			if(skill->position <=1){
				if(skill->stamina < 0){
					skill->batters_left = 0;
				}
				int max_batters = 3 + round(3.0*skill->end/10.0);
/*				if(skill->batters_left < max_batters){
					print_done = 1;
					fprintf(stderr,"#Resting player(%d): %s end: %d with batters_left(%d/%d) stamina:%d...",skill->id,skill->last_name,skill->end,skill->batters_left,max_batters,skill->stamina);
				}
*/
			int batters_left = skill->batters_left + divide_ceiling(max_batters,4);
			if(batters_left> max_batters){
				skill->batters_left = max_batters;
			}
			else{
				skill->batters_left = batters_left;
			}
			/*Restore pitcher's skill in case they were tired.*/
			skill->mov -= (skill->stamina * SKILL_REDUCTION_FACTOR);
			skill->vel -= (skill->stamina * SKILL_REDUCTION_FACTOR);
			skill->ctrl -= (skill->stamina * SKILL_REDUCTION_FACTOR);
			skill->stamina = 0;
/*
			if(print_done){
				print_done = 0;
				fprintf(stderr,"Done new batters left = %d\n",skill->batters_left);
			}
*/
/*			fprintf(stderr,"(%d/%d)\n",skill->batters_left,max_batters);*/
			}
			temp= temp->next;
		}
	}

}

void update_stamina(Db_Object db, struct linked_list *tired_pitchers){
	char query[100];
	while(tired_pitchers){
		int query_len = sprintf(query,UPDATE_BATTERS_LEFT,((Key_Value)(tired_pitchers->value))->value,((Key_Value)(tired_pitchers->value))->key);
		db->execute_query(db->conn,query,query_len);
		tired_pitchers = tired_pitchers->next;
	}
}

void update_year(Db_Object db, int league_id){

	Date league_date = select_date_for_league(db, league_id);
	league_date->year++;
	
	char query[DEFAULT_QUERY_LENGTH];
	int query_len = sprintf(query,UPDATE_YEAR,db->date_to_string(league_date), league_id);
	db->execute_query(db->conn, query, query_len);
}


void fill_lineup_batch(int player_id, int bat_order, int position, int* offense,int* defense,struct linked_list **bench){

	PRINT_MESSAGE(1,"Adding %d playing %d hitting %d...\n",player_id,position,bat_order);
	if(bat_order>0){
		offense[bat_order] = player_id;
		defense[position] = player_id;
/*		printf("player_id: %d team: %d position: %ddefense[1][1]: %d\n",player_id,team,position,defense[1][1]);*/
/*		print_player(player_id,bat_order,position,last_name);*/
	}
	else{
		PRINT_MESSAGE(1,"Adding player to bench...\n");
		PRINT_MESSAGE(bench==NULL,"\tBench is empty.\n");
		struct linked_list **temp_list = bench;
		Linked_List_append(temp_list,(void*)player_id);
		PRINT_MESSAGE(1,"Done.\n");
	}
}

void fill_rotation_batch(int player_id, int next_starter, int* defense,struct linked_list **bullpen){

/*	printf("id: %d starting: %d\n",player_id,next_starter);*/
	if(next_starter){
		
		defense[1] = player_id;
	}
	else{
		struct linked_list **temp_list = bullpen;
		Linked_List_append(temp_list,(void*)player_id);
	}
}

void select_lineups_league(Db_Object db, int league_id, Hash offense_hash, Hash defense_hash,Hash bench_hash, Hash bullpen_hash){
	
	char query[DEFAULT_QUERY_LENGTH];
	
	int query_len = sprintf(query,SELECT_LINEUPS_FOR_LEAGUE,league_id);
	PRINT_MESSAGE(1,"Executing query %s...\n",query);
	native_result* result = db->execute_query_result(db->conn,query,query_len);
	PRINT_MESSAGE(1,"Done.\n");
	native_row row;
	while((row = db->fetch_row(result))){
		int team_id = db->get_column_int(row,0);
		int player_id = db->get_column_int(row,1);
		int bat_order = db->get_column_int(row,2);
		int position = db->get_column_int(row,3);
		if(Hash_get_value(offense_hash,(void*)team_id) == NULL){
			Hash_add_key(offense_hash,(void*)team_id,malloc(10*sizeof(int)));
		}
		if(Hash_get_value(defense_hash,(void*)team_id) == NULL){
			Hash_add_key(defense_hash,(void*)team_id,malloc(10*sizeof(int)));
			
		}
		struct linked_list *bench = Hash_get_value(bench_hash,(void*)team_id);
		PRINT_MESSAGE(1,"Filling lineup for team: %d...\n",team_id);
		fill_lineup_batch(player_id, bat_order, position, Hash_get_value(offense_hash,(void*)team_id),Hash_get_value(defense_hash,(void*)team_id),&bench);
		PRINT_MESSAGE(1,"Done.\n");
	}
	db->free_result(result);
	/*printf("`\n");*/
/*	write(FD,"`\n",2*sizeof(char));*/

/*	This is needed if we are calling a mysql stored procedure.*/
/*	mysql_next_result(conn);*/
	
	query_len = sprintf(query,SELECT_ROTATION_FOR_LEAGUE,league_id);
	PRINT_MESSAGE(1,"Executing query %s\n",query);
	result = db->execute_query_result(db->conn,query,query_len);
	PRINT_MESSAGE(1,"Done.\n");
	
	while((row = db->fetch_row(result))){
		int team_id = db->get_column_int(row,0); 
		int player_id = db->get_column_int(row,1);
		int next_starter = db->get_column_int(row,2);
		struct linked_list *bullpen = Hash_get_value(bullpen_hash,(void*)team_id);
		if(bullpen == NULL){
			PRINT_MESSAGE(1,"Bullpen is empty, initializing...\n");
			Hash_add_key(bullpen_hash,(void*)team_id,(void*)bullpen);
		}
		fill_rotation_batch(player_id, next_starter, Hash_get_value(defense_hash,(void*)team_id),&bullpen);
		Hash_set_value(bullpen_hash,team_id,bullpen);
	}
	db->free_result(result);
/*	This is needed if we are calling a mysql stored procedure.*/
/*	mysql_next_result(conn);*/
	
}


void select_skills_league(Db_Object db, int league_id, Hash skill_hash){

	char query[700];
	int query_len = sprintf(query,SELECT_SKILLS_FOR_LEAGUE,league_id);	
	native_result* result = db->execute_query_result(db->conn,query,query_len);

	native_row row;
	while((row = db->fetch_row(result))){
		Skill skill = build_skill(db, row);
		PRINT_MESSAGE(skill->position == 1,"#Last name: %s Batters Left: %d\n",skill->last_name,skill->batters_left);
		Hash_add_key(skill_hash,(void*)skill->id,skill);
/*		printf("Adding key: %d, name: %s\n",skill->id,skill->last_name);*/
	}
	db->free_result(result);
	/*This is need if we have execute a myslq store procedure.*/
/*	mysql_next_result(conn);*/
}

void print_batters_left(Hash hash,char marker){

	int i;
	for(i=0;i<hash->capacity;i++){
		struct linked_list *temp = hash->items[i];
		while(temp != NULL){
			Key_Value key_value = temp->value;
			Skill skill = key_value->value;
			fprintf(stderr,"%c Name: %s batters_left: %d\n",marker,skill->last_name,skill->batters_left);
			temp = temp->next;
		}
	}
}

/*!
 * @return 0 for success or something else if there is a failiure.
 */
int simulate_day(Db_Object db,int league_id, int game, double hitter_mod, double pitcher_mod,double ab_exp,Hash skill_hash, struct linked_list **tired_pitchers){
	int i;
	char game_num[10];
	int fd[2];
	int num_games = 4;
	int **games= select_schedule(db,league_id,game);
	int num_chars = sprintf(game_num,"GAME=%d\n",game);
	char config_arg[200];
	char* html_arg;
	int rc;
	if(HTML){
		html_arg = "--html";
	}
	else{
		html_arg = NULL;
	}
	for(i=0;i<num_games;i++){
		pipe(fd);
		pid_t child_pid = fork();
		if(child_pid == 0){/*the child*/
			/*Close write end of pipe.*/
			close(fd[1]);
			dup2(fd[0],0);
			char game_log_options[20];
			sprintf(game_log_options,"--db=%s",PERL_DB_TYPE);
			sprintf(config_arg,"--config=%s",CONFIG_FILE);
			execlp("perl","perl","game_log.pl",game_log_options,"--save-boxscore",config_arg, html_arg,NULL);
		}
		else{
			close(fd[0]); /* Close read end of pipe.*/
			write(fd[1], game_num, num_chars);
			if(skill_hash){
				rc = sim_game_batch(db,fd[1],games[i][0],games[i][1],
					hitter_mod,pitcher_mod,ab_exp,skill_hash,
					tired_pitchers);
			}
			else{
				rc = sim_game(db,fd[1],games[i][0],games[i][1],
				hitter_mod,pitcher_mod,ab_exp,tired_pitchers);
			}
			close(fd[1]);
			if(rc){
				kill(child_pid, SIGINT);
				return 1;
			}
			int status;
			wait(&status);
		}
	}
	free(games);
	return 0;
}

/*!
 * @return \sa simulate_season
 */
int simulate_season_speed(Db_Object db,int league_id,double hitter_modifier,double pitcher_modifier,double ab_exponent){
	
	int num_games = 4;
	int fd[2];
	pipe(fd);
	pid_t child_pid = fork();
	if(child_pid == 0){/*the child*/
		/*Close write end of pipe.*/
		close(fd[1]);
		dup2(fd[0],0);
		char game_log_options[20];
		sprintf(game_log_options,"--db=%s",PERL_DB_TYPE);
		execlp("perl","perl","game_log.pl",game_log_options,NULL);
	}	
	else{
		db->begin_transaction(db->conn);
		int j,i;
		PRINT_MESSAGE(1,"Loading Skills.\n");
		Hash skill_hash = Hash_create(100);
		select_skills_league(db,league_id,skill_hash);
		PRINT_MESSAGE(1,"Done.\n");
		for(j=1;j<163;j++){
				
/*		Hash skill_hash = Hash_create(100);*/
/*		select_skills_league(conn,league_id,skill_hash);*/
#ifdef _SPEEDY_DEBUG_
/*		fprintf(stderr,"Before games.\n");*/
/*		print_batters_left(skill_hash,'+');*/
#endif
			/*struct linked_list *bench[2];*/
			/*struct linked_list *bullpen[2];*/
			/*int **offense = malloc(2*sizeof(int*));*/
			/*int **defense = malloc(2*sizeof(int*));*/

			/*Hash offense_hash = Hash_create(sizeof(int),sizeof(int*));*/
			/*Hash defense_dict = Hash_create(sizeof(int),sizeof(int*));*/
			/*Hash bench_dict = Hash_create(sizeof(int),sizeof(Linked_List));*/
			/*Hash bullpen_dict = Hash_create(sizeof(int),sizeof(Linked_List));*/
			PRINT_MESSAGE(1,"Day: %d\n",j);			
			PRINT_MESSAGE(1,"Loading Lineups...\n");
/*select_lineups_league(conn,league_id,offense_hash,defense_dict,bench_dict,bullpen_dict);*/
			PRINT_MESSAGE(1,"Done.\n");
			PRINT_MESSAGE(1,"Loading schedule...\n");
			int **games = select_schedule(db,league_id,j);
			PRINT_MESSAGE(1,"Done.\n");
			/*Hash_print(skill_hash,print_skill);*/
			for(i=0;i<num_games;i++){
				PRINT_MESSAGE(1,"Game: %d\n",i);
				int home_id = games[i][0];
				int away_id = games[i][1];
				PRINT_MESSAGE(1,"Loading offense...\n");
				/*dprint(offense_hash,print_int,print_int);*/
/*				offense[0] = dgetValue(offense_hash,(void*)home_id);*/
/*				offense[1] = dgetValue(offense_hash,(void*)away_id);*/
/*				printIntArray(offense[0],10);*/
/*				printIntArray(offense[1],10);*/
/*				PRINT_MESSAGE(1,"Loading defense...\n");*/
/*				dprint(defense_dict,print_int,print_int);*/
/*				PRINT_MESSAGE(1,"Home team.\n");*/
/*				defense[0] = dgetValue(defense_dict,(void*)home_id);*/
/*				PRINT_MESSAGE(1,"Away team.\n");*/
/*				defense[1] = dgetValue(defense_dict,(void*)away_id);*/
/*				PRINT_MESSAGE(1,"Loading bench...\n");*/
/*				bench[0] = dgetValue(bench_dict,(void*)home_id);*/
/*				bench[1] = dgetValue(bench_dict,(void*)away_id);*/
/*				PRINT_MESSAGE(1,"Bullpen bench...\n");*/
/*				bullpen[0] = dgetValue(bullpen_dict,(void*)home_id);*/
/*				bullpen[1] = dgetValue(bullpen_dict,(void*)away_id);*/
/*				if(bullpen[0] == NULL || bullpen[1] == NULL){*/
/*					PRINT_ERROR("No bullpen\n");*/
/*					exit(1);*/
/*				}*/
				PRINT_MESSAGE(1,"Calling sim_game_batch...\n");
				if(sim_game_batch(db,fd[1],home_id,away_id,hitter_modifier,pitcher_modifier,ab_exponent,skill_hash,NULL)){
					return 1;
				}
				PRINT_MESSAGE(1,"Done.\n");
			}
			free(games);
			PRINT_MESSAGE(1,"Resting pitchers...\n");
			rest_pitchers_speed(skill_hash);
#ifdef _SPEEDY_DEBUG_
/*			fprintf(stderr,"After games.\n");*/
/*			print_batters_left(skill_hash,'#');*/
#endif
/*			rest_pitchers(conn,league_id);*/
			PRINT_MESSAGE(1,"Done.\n");
			PRINT_MESSAGE(1,"Updating rotation...\n");
			update_rotation(db,league_id);
			PRINT_MESSAGE(1,"Done.\n");
		}
		db->commit(db->conn);
		Hash_free(skill_hash, Skill_free);
		close(fd[1]);
		int status;
		wait(&status);
		return 0;
	}
}

/* !
 * @return 0 for success, something else if there is an error.
 */
int simulate_season(Db_Object db,int league_id,double hitter_modifier,double pitcher_modifier,double ab_exponent){

	int num_games = 4;
	int fd[2];
	int j,i;
	Hash skill_hash = Hash_create(100);
	struct linked_list *tired_pitchers = NULL;
	select_skills_league(db,league_id,skill_hash);
	

	for(j=1;j<163;j++){	
		if(simulate_day(db,league_id,j,hitter_modifier,pitcher_modifier,ab_exponent,skill_hash,&tired_pitchers)){
			return 1;
		}
		rest_pitchers_speed(skill_hash);
		db->begin_transaction(db->conn);
		/*We probably don't need to call update_stamina here.*/
		update_stamina(db,tired_pitchers);
		update_rotation(db,league_id);
		db->commit(db->conn);
	}
	Hash_free(skill_hash, Skill_free);
	return 0;
}

struct arguments{
	int *fast;
	int *season;
	int *league_id;
	int *day;
	int *html;
	int *seed;
	double *hitter_modifier;
	double *pitcher_modifier;
	double  *ab_exponent;
};

static error_t arg_parser(int key, char *arg, struct argp_state *state)
{
	struct arguments *args = state->input;
	switch(key){
	case 'f':
		*args->fast = 1;
		break;
	case 'a':
		*args->season = 1;
		break;
	case 's':
		*args->seed = atoi(arg);
		break;
	case 'd':
		*args->day = atoi(arg);
		break;
	case 'l':
		*args->league_id = atoi(arg);
		break;
	case 'h':
		*args->hitter_modifier = atof(arg);
		break;
	case 'p':
		*args->pitcher_modifier = atof(arg);
		break;
	case 'x':
		*args->ab_exponent = atof(arg);
		break;
	default:
		return ARGP_ERR_UNKNOWN;
	}
	return 0;
}

/*In the future let's pass a day here instead of*/
/*a game number.*/
int main(int argc, char** argv){
	
	int SEED = getpid();
	double HITTER_MODIFIER= 1.0;
	double PITCHER_MODIFIER= 1.0;
	double AB_EXPONENT = 1.0;
	static int FAST = 0;
	static int SEASON = 0;
	int LEAGUE_ID = 0;
	int DAY = 0;
	
	struct argp_option options[] = {
		{"fast",'f'},
		{"season",'a'},
		{"league-id",'l',"int"},
		{"day",'d',"int"},
		{"html",'w'},
		{"seed",'s',"int"},
		{"hit-mod",'h',"double"},
		{"pitch-mod",'p',"double"},
		{"ab-exp",'x',"double"},
		{0}
	};
	struct arguments args = {&FAST, &SEASON, &LEAGUE_ID, &DAY, &HTML, &SEED,
			&HITTER_MODIFIER, &PITCHER_MODIFIER, &AB_EXPONENT};
	struct argp argp = {options, arg_parser, 0, 0, global_child};
	argp_parse(&argp, argc, argv, 0, 0, &args);
	double hitter_modifier = HITTER_MODIFIER;
	double pitcher_modifier = PITCHER_MODIFIER;
	double ab_exponent = AB_EXPONENT;
/*	printf("Connecting to database...\n");*/
	Db_Object db = db_connect(CONFIG_FILE);
	if(!db){
		return EXIT_FAILURE;
	}
/*	printf("Done.\n");*/
	int num_games = 4;
	struct linked_list *tired_pitchers = NULL;
	int i;
	if(LEAGUE_ID <= 0){
		fprintf(stderr, "%d is not a valid league-id.  Use --league-id=N to specify league id.\n",LEAGUE_ID);
		exit(1);
	}
	if(!DAY && !SEASON){
		fprintf(stderr,"Please specify a day number --day=N or use --season\n");
		exit(1);
	}

	srand(SEED);
	fprintf(stderr,"Seed: %d\n",SEED);
	if(SEASON){
		if(FAST){
			if(simulate_season_speed(db,LEAGUE_ID,hitter_modifier,pitcher_modifier,ab_exponent)){
				return EXIT_FAILURE;
			}
		}
		else{
			if(simulate_season(db,LEAGUE_ID,hitter_modifier,pitcher_modifier,ab_exponent)){
				return EXIT_FAILURE;
			}
		}
	}
	else{		

		/* Play the Games. */
		if(simulate_day(db,LEAGUE_ID,DAY,hitter_modifier,
			pitcher_modifier,ab_exponent,NULL,&tired_pitchers)){

			return EXIT_FAILURE;
		}
		db->begin_transaction(db->conn);
		
		/*Update stamina of pitchers who pitched */
		update_stamina(db, tired_pitchers);

		/* Rest the pitchers */
		rest_pitchers(db, LEAGUE_ID);

		/*Update the pitching rotation*/
		update_rotation(db, LEAGUE_ID);
		
		db->commit(db->conn);
	}
	db->close_connection(db->conn);
	return 0;
}
