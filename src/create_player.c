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
#include <math.h>
#include <string.h>
#include <argp.h>
#include "fbs_argp.h"
#include "tom_print.h"
#include "tom_array_list.h"
#include "linked_list.h"
#include "baseball_objects.h"
#include "baseball_data.h"
#include "tom_math.h"
#include "db_object.h"

#ifdef _USEGSL_
#include <gsl/gsl_rng.h>
#include <gsl/gsl_randist.h>
#define random_normal(mean,sd,min,max) gsl_random_normal(rng,mean,sd,min,max)
gsl_rng* rng;
#endif

const double SD_TWEAK = 0.5;
const int MIN_SKILL = 0;
const int MAX_SKILL = 100;
const int PERCENT_THROW_RIGHT = 70;
const int PERCENT_HIT_SAME = 85;
const int PERCENT_HIT_DIFFERENT = 5;
const int PERCENT_HIT_BOTH = 10;

double HITTER_MEAN = 50.0;
double HITTER_SD = 22.5;
double PITCHER_MEAN = 50.0;
double PITCHER_SD = 22.5;

struct arguments{
	int *seed;
	int *league_id;
	int *num_players;
	double *hitter_mean;
	double *hitter_sd;
	double *pitcher_mean;
	double *pitcher_sd;
};

static int calc_skill(int mean,double sd);
static error_t arg_parser(int key, char *arg, struct argp_state *state);

int main(int argc, char** argv){
	
	int seed = getpid();
	int c;
	int option_index = 0;
	int league_id;
	int num_players;
	
	struct argp_option options[] = {
		{"seed",'s', "int", 0, 0},
		{"league-id", 'l', "int", 0, 0},
		{"players", 'n', "int", 0, 0},
		{"hitter-mean",'h', "double", 0, 0},
		{"hitter-sd", 'i', "double", 0, 0},
		{"pitcher-mean", 'p', "double", 0, 0},
		{"pitcher-sd", 'r', "double", 0, 0},
		{0}
	};

	struct arguments args = {&seed, &league_id, &num_players, &HITTER_MEAN,
					&HITTER_SD, &PITCHER_MEAN, &PITCHER_SD};
	struct argp argp = {options, arg_parser, 0, 0, global_child}; 
	argp_parse(&argp, argc, argv, 0, 0, &args);

#ifdef _USEGSL_
	gsl_rng_env_setup();
	const gsl_rng_type *type = gsl_rng_default;
	rng = gsl_rng_alloc(type);
	gsl_rng_set(rng,seed);
#else
	srand(seed);
#endif
	Db_Object db = db_connect(CONFIG_FILE);
	db->begin_transaction(db->conn);
	fprintf(stderr,"Seed: %d\n",seed);

	Array_List first_names = Array_List_create(sizeof(char*));
	Array_List last_names = Array_List_create(sizeof(char*));

	select_names(db,first_names,last_names);
	int i;

	struct player new_player;
	struct hitter_skill hitter_skill;
	struct pitcher_skill pitcher_skill;
	struct fielder_skill fielder_skill;

	for(i=0;i<num_players;i++){
		
		int power= random_normal(HITTER_MEAN,HITTER_SD,MIN_SKILL,MAX_SKILL);
		int contact = random_normal(HITTER_MEAN,HITTER_SD,MIN_SKILL,MAX_SKILL);
		int fielding = random_normal(HITTER_MEAN,HITTER_SD,MIN_SKILL,MAX_SKILL);
		int speed = random_normal(HITTER_MEAN,HITTER_SD,MIN_SKILL,MAX_SKILL);
		int intelligence = random_normal(HITTER_MEAN,HITTER_SD,MIN_SKILL,MAX_SKILL);
		int control = random_normal(PITCHER_MEAN,PITCHER_SD,MIN_SKILL,MAX_SKILL);
		int movement = random_normal(PITCHER_MEAN,PITCHER_SD,MIN_SKILL,MAX_SKILL);
		int velocity = random_normal(PITCHER_MEAN,PITCHER_SD,MIN_SKILL,MAX_SKILL);
		int endurance = 0;
		int bats = (rand()%100)+1;
		int throws = (rand()%100)+1;
		int position = rand()%18;

/*TODO: Make the choosing of names random.*/
		int first_name = 8 * (power + contact + fielding + speed + bats) % first_names->length;
		int last_name = 8 * (first_name + speed + intelligence + control + movement + velocity + bats + throws + control + movement + position) % last_names->length;
		
		if(throws <= PERCENT_THROW_RIGHT){
			new_player.throws = RIGHT;
		}
		else{
			new_player.throws = LEFT;
		}
		
		if(bats<= PERCENT_HIT_SAME){
			new_player.bats = new_player.throws;
		}
		else if(bats<PERCENT_HIT_SAME+PERCENT_HIT_DIFFERENT){
			if(new_player.throws == RIGHT){
				new_player.bats = LEFT;
			}
			else{
				new_player.bats = RIGHT;
			}
		}
		else{
			new_player.bats = SWITCH;
		}
		if(position <= 9 && position > 1){
			new_player. position = position;
			pitcher_skill.end = random_normal(30.0,PITCHER_SD*SD_TWEAK,0,49);
		}
		else if(position>=15 || position == 1){
			new_player.position =1;
			pitcher_skill.end = random_normal(80.0,PITCHER_SD*SD_TWEAK,50,100);
		}
		else{
			new_player.position = 0;
			pitcher_skill.end = random_normal(30.0,PITCHER_SD*SD_TWEAK,0,49);
		}
		int vs_right_mod = 5;
		int vs_left_mod = 5;
		if(new_player.bats == RIGHT){
			vs_right_mod = -5;
		}
		if(new_player.bats ==LEFT){
			vs_left_mod = -5;
		}
		
		new_player.first_name = gget(first_names,first_name);
		new_player.last_name = gget(last_names,last_name);
		
		hitter_skill.cvr = calc_skill(contact+vs_right_mod,HITTER_SD);
		hitter_skill.pvr = calc_skill(power+vs_right_mod,HITTER_SD);
		hitter_skill.cvl = calc_skill(contact+vs_left_mod,HITTER_SD);
		hitter_skill.pvl = calc_skill(power+vs_left_mod,HITTER_SD);
		hitter_skill.spd = speed;	
		
		fielder_skill.range = calc_skill(fielding,HITTER_SD);
		fielder_skill.arm = calc_skill(fielding,HITTER_SD);
		fielder_skill.field = calc_skill(fielding,HITTER_SD);
		
		new_player.intelligence = intelligence;
		
		pitcher_skill.mov = calc_skill(movement,PITCHER_SD);
		pitcher_skill.vel = calc_skill(velocity,PITCHER_SD);
		pitcher_skill.ctrl = calc_skill(control,PITCHER_SD);
		
		insert_player(db,&new_player,&hitter_skill,&pitcher_skill,&fielder_skill,league_id);
		
	}
	db->commit(db->conn);
	db->close_connection(db->conn);	
#ifdef _USEGSL_
	gsl_rng_free(rng);
#endif

	return 0;	
}

static int calc_skill(int mean,double sd)
{
	return random_normal(mean*1.0,sd*SD_TWEAK,MIN_SKILL,MAX_SKILL);
}

static error_t arg_parser(int key, char *arg, struct argp_state *state)
{
	struct arguments *args = state->input;
	switch(key){
	case 's':
		*args->seed = atoi(arg);
		break;
	case 'l':
		*args->league_id = atoi(arg);
		break;
	case 'n':
		*args->num_players = atoi(arg);
		break;
	case 'h':
		*args->hitter_mean = atof(arg);
		break;
	case 'i':
		*args->hitter_sd = atof(arg);
		break;
	case 'p':
		*args->pitcher_mean = atof(arg);
		break;
	case 'r':
		*args->pitcher_sd = atof(arg);
		break;
	default:
		return ARGP_ERR_UNKNOWN;
	}
	return 0;
}
