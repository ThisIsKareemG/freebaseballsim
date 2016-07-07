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
#include <stdarg.h>
#include "compiler_macros.h"
#include "baseball_macros.h"
#include "baseball_objects.h"
#include "tom_array_list.h"
#include "linked_list.h"
#include "tom_array.h"
#include "tom_print.h"
#include "key_value.h"
#include "hash.h"
#include "sim_game.h"
#include "baseball_data.h"

#define UPDATE_PITCHER() pitcher_id = defense[!team_hitting][1]; pitcher_skill = Hash_get_value(skill_hash,(void*)pitcher_id);

const int HOME_TEAM = 0;
const int AWAY_TEAM = 1;

const int GB_C = 271;
const int FB_C = 271;
const int K_C = 138;
const int BB_C = 88;
const int Single_C = 740;
const int Double_C = 160;
const int Triple_C = 27;
const int HR_C = 86;

const int R_P_C = 99;
const int R_C_C = 7;
const int R_FB_C = 47;
const int R_SB_C = 172;
const int R_TB_C = 309;
const int R_SS_C = 366;

const int L_P_C = 99;
const int L_C_C = 6;
const int L_FB_C = 266;
const int L_SB_C = 395;
const int L_TB_C = 64;
const int L_SS_C = 170;
		
const int LF_C = 300;
const int CF_C = 400;
const int RF_C = 300;

const int QUERY_SIZE = 100;
const int BUF_SIZE = 64;
const int GAME_LOG_BUFFER_SIZE = 10000;

/*GLOBAL VARIABLES*/
int pitcher_runs[2];/* = {0,0};*/
int team_hitting;/* = AWAY_TEAM;*/
int inning_over;/* = 0;*/
int FD;
int SHOW_QUERY = 1;

double HITTER_MODIFIER= 1.0;
double PITCHER_MODIFIER= 1.0;
double AB_EXPONENT = 1.0;

char game_log_buffer[10000];
int game_log_buffer_length = 0;

typedef enum result_type{IB,ZB,EB,HR,SO,BB,E,HBP,FB,GB}Result_Type;

typedef enum base{HOME,FIRST,SECOND,THIRD}Base;

typedef enum base_result{SAFE,OUT,SCORE}Base_Result;

typedef enum x_result{INFIELD,BUNT,GOOD,BAD,ONE_BASE,TWO_BASE,NONE}X_Result;

typedef struct pitcher_stat{
	
	int outs;
	int runs;
}*Pitcher_Stat;

struct runner_result{
	
	int id;
	Base_Result result;
	Base base;
};

typedef struct ab_result{
	
	Result_Type result_type;
	Array_List defense;
	int outs;
	int runs;
	int distance;
	struct runner_result *runner_results[4];
	X_Result x_result;
		
} ab_result;

void ab_result_clear(ab_result* result){
	
	Array_List_clear(result->defense,0);
}

int is_hit(ab_result* result){
	return result->result_type == IB || result->result_type == ZB || result->result_type ==EB || result->result_type == HR;
}


char result_type_to_char(Result_Type type){
	
	switch(type){
		case GB:
			return 'G';
		case FB:
			return 'F';
		case SO:
			return 'K';
		case BB:
			return 'W';
		case IB:
			return '1';
		case ZB:
			return '2';
		case EB:
			return '3';
		case HR:
			return '4';
		case E:
			return 'E';
		default:
			fprintf(stderr,"Unknown Result_Type\n");
			break;
	}
}

void skill_not_found(int player_id, int team_id){

		PRINT_ERROR("Player with id=%d team_id=%d not found in skill hash\n",player_id, team_id);
}

void write_game_log_buffer(char* str,int str_len){
	
	strncpy(game_log_buffer+game_log_buffer_length,str,str_len);
	game_log_buffer_length+=str_len;
}

void v_write_game_log_buffer(char* fmt, ...){
	va_list args;
	int chars;
	va_start(args,fmt);
	chars = vsprintf(game_log_buffer+game_log_buffer_length,fmt,args);
	game_log_buffer_length+=chars;
}

void flush_game_log_buffer(int FD){

	write(FD,game_log_buffer,game_log_buffer_length*sizeof(char));
	memset(game_log_buffer,0,game_log_buffer_length);
	game_log_buffer_length=0;
}

void print_player(int player_id,int bat_order, int position, char* last_name){
	
	v_write_game_log_buffer("^%d=%s=%d=%d\n",player_id,last_name,position,bat_order);
}

char hand_to_char(Hand hand){
	
	switch(hand){
		
		case RIGHT:
			return 'R';
		case LEFT:
			return 'L';
		default:
			PRINT_ERROR("Invalid argument to function hand_to_char");
			break;
	}
}

Hand get_hitter_hand(Hand hitter_hand, Hand pitcher_hand){
	
	if(hitter_hand == SWITCH){
		if(pitcher_hand == RIGHT){
			return LEFT;
		}
		else{
			return RIGHT;
		}
	}
	else{
		return hitter_hand;
	}
}	

/*DATA ACCESS*/

void update_pitcher_stamina(struct linked_list **tired_pitchers, Skill skill){
	
	if(!tired_pitchers){
		return;
	}
	Key_Value info = malloc(sizeof(struct key_value));
	int batters_left;
	if(skill->stamina<0){
		batters_left = 0;
	}
	else{
		batters_left = skill->batters_left;
	}
	info->key = (void*)skill->id;
	info->value = (void*)batters_left;
	Linked_List_append(tired_pitchers,info);
}

void print_skill(Skill skill){
	
	fprintf(stderr,"position: %d cvr: %d pvr: %d cvl: %d pvl: %d ctrl: %d mov: %d vel: %d p-ovr: %d\n",skill->position, skill->cvr,skill->pvr,skill->cvl,skill->pvl,skill->ctrl,skill->mov,skill->vel,(skill->ctrl+skill->mov+skill->vel)/3);
}

void print_int_stderr(void* inte){
	fprintf(stderr,"%d",(int)inte);
}

void fill_lineup(Db_Object db, native_row row,int offense[2][10],int defense[2][10],struct linked_list **bench, int team, Hash skill_hash){
	
	int player_id = db->get_column_int(row, 0);
	int bat_order = db->get_column_int(row, 1);
	int position = db->get_column_int(row, 2);
	char *last_name = ((Skill)Hash_get_value(skill_hash,(void*)player_id))->last_name; 
		
	if(bat_order>0){
		offense[team][bat_order] = player_id;
		defense[team][position] = player_id;
		print_player(player_id,bat_order,position,last_name);
	}
	else{
		Linked_List_append(bench, (void*)player_id);
	}
}

void fill_rotation(Db_Object db, int team_id, int defense[10],struct linked_list **bullpen){
	
	native_row row;
	native_result* result = select_rotation(db, team_id);
	while((row = db->fetch_row(result))){
		int player_id = db->get_column_int(row, 0);
		int next_starter = db->get_column_int(row, 1);
		if(next_starter){
			defense[1] = player_id;
		}
		else{
			Linked_List_append(bullpen, (void*)player_id);
		}
	}
	db->free_result(result);
}

/*!
 * @return 0 for success, something else if there was an error.
 */
int select_lineups(Db_Object db,int home_id,int away_id, int offense[2][10], int defense[2][10],struct linked_list *bench[2],struct linked_list *bullpen[2],Hash skill_hash){
	
	native_result* result;
	native_row row;

	int home_starter;
	int away_starter;

	fill_rotation(db,away_id,defense[AWAY_TEAM],&bullpen[AWAY_TEAM]);
	
	fill_rotation(db,home_id,defense[HOME_TEAM],&bullpen[HOME_TEAM]);

	Skill home_starter_skill = Hash_get_value(skill_hash,(void*)defense[HOME_TEAM][1]);
	Skill away_starter_skill = Hash_get_value(skill_hash,(void*)defense[AWAY_TEAM][1]);
	if(unlikely(!home_starter_skill)){
		skill_not_found(home_starter, home_id);
		return 1;
	}
	if(unlikely(!away_starter_skill)){
		skill_not_found(away_starter, away_id);
		return 1;
	}
	
	result = select_lineup(db, away_id,home_starter_skill->throws);	
	while((row = db->fetch_row(result))){		
		fill_lineup(db,row,offense,defense,&bench[AWAY_TEAM],AWAY_TEAM,skill_hash);
	}
	
	write_game_log_buffer("`\n",2);
	
	db->free_result(result);
	result = select_lineup(db, home_id,away_starter_skill->throws);
	while((row = db->fetch_row(result))){
		fill_lineup(db,row,offense,defense,&bench[HOME_TEAM],HOME_TEAM,skill_hash);
	}
	db->free_result(result);
	return 0;
}

/*!
 * @return 0 if the lineups are correct, something else if they are not.
 */
int verify_lineups(int offense[2][10], int defense[2][10]){
	int i,j;
	for(j=0; j<=1; j++){
		for(i=1; i<=9; i++){
			if(!offense[j][i] || !defense[j][i]){
				return 1;
			}
		}
	}
	return 0;
}
/*-----------*/
/*PRINT FUNCTIONS*/


void print_new_inning(){
/*	PRINT_MESSAGE(1,"Function call: new_inning()\n");*/
	write_game_log_buffer("~\n",2);
}

void print_result(ab_result* result,Skill hitter_skill, Skill pitcher_skill){
	
	char hitter_hand = hand_to_char(get_hitter_hand(hitter_skill->bats,pitcher_skill->throws));
	char pitcher_hand = hand_to_char(pitcher_skill->throws);
	char result_char = result_type_to_char(result->result_type);
	v_write_game_log_buffer("#%d%c>%d%c>%c",pitcher_skill->id,pitcher_hand,hitter_skill->id,hitter_hand,result_char);
	int i;
	for(i=0;i<result->defense->length;i++){
		if(i==0){
			write_game_log_buffer(">",1);
		}
		int player_id = (int)gget(result->defense,i);
		v_write_game_log_buffer("%d",player_id);
		/*TODO: Clean this up.*/
		if(result->x_result != NONE && i==0){
			char result_char;
			if(result->x_result == GOOD){
				result_char = '+';
			}
			if(result->x_result == BAD){
				result_char = '_';
			}
			write_game_log_buffer(&result_char,1);
		}
		if(i!=result->defense->length - 1){
			write_game_log_buffer("-",1);
		}
	}
	write_game_log_buffer("\n",1);
	/*Print RBIs*/
	if(result->result_type!= E && result->outs<2 && result->runs>0){
		v_write_game_log_buffer("%%%d>%d\n",hitter_skill->id,result->runs);
	}
	for(i=0;i<4;i++){
		struct runner_result *runner_result = result->runner_results[i];
		if(runner_result->id == 0){
			continue;
		}
		switch( runner_result->result){
			case SCORE:
				v_write_game_log_buffer("@%d>%d.\n",runner_result->id,pitcher_skill->id);
				break;
			case SAFE:
				v_write_game_log_buffer("[%d+%d\n",runner_result->id,runner_result->base);
				break;
			case OUT:
				v_write_game_log_buffer("[%d-%d\n",runner_result->id,runner_result->base);
				break;
		}
		runner_result->id = 0;
	}
	
}

/*---------------*/

int is_game_over(int inning,int team_hitting, int runs[],int inning_over){
	
	if(likely(inning<9)){
		return 0;
	}
	if(runs[HOME_TEAM] > runs[AWAY_TEAM] && team_hitting == HOME_TEAM){
		return 1;
	}
	if(inning>9 && inning_over && runs[HOME_TEAM]!=runs[AWAY_TEAM] && team_hitting == AWAY_TEAM){
		return 1;
	}
	return 0;	
}

char hand_to_string(Hand hand){
	
	if(hand == RIGHT){
		return 'R';
	}
	else{
		return 'L';
	}
}
void apply_defense(ab_result* result, Skill defense_skill, int hitter_speed){
	
	int random = rand();
	if(is_hit(result)){
		/*Check for a good play.*/
		if(random%1000<defense_skill->range){
			if(result->distance == 0){
				result->result_type = GB;
			}
			else{
				result->result_type = FB;
			}
			result->outs =1;
			result->x_result = GOOD;
		}
	}
	else{
		/*Check for a bad play.*/
		if(random%1000 < 100 - defense_skill->range){
			if(result->distance == 0){
				result->result_type = IB;
			}
			else{
				result->result_type = ZB;
			}
			result->outs = 0;
			result->x_result = BAD;
		}
		/*Check for an error.*/
		else if(random%2000 < 200 - defense_skill->range - defense_skill->arm){
			if(result-> distance == 0){
				result->x_result = ONE_BASE;
			}	
			else{
				result->x_result = TWO_BASE;
			}
			result->outs = 0;
			result->result_type = E;
		}
		/*Check for an infield hit.*/
		else if(result->distance == 0 && random%1000 < 200 - defense_skill->arm + hitter_speed){
			result->result_type = IB;
			result->x_result = INFIELD;
			result->outs = 0;
		}
	}
}

double calculate_delta(double a, double b){
	
	if(AB_EXPONENT == 1.0){
		return (a-b);
	}
	if(a == b){
		return 0.0;
	}
	return (a-b) * pow(fabs(a-b), AB_EXPONENT - 1.0);	
}

int get_current_vel(Skill pitcher_skill){
	return pitcher_skill->vel + (pitcher_skill->stamina * SKILL_REDUCTION_FACTOR);
}
int get_current_mov(Skill pitcher_skill){
	return pitcher_skill->mov + (pitcher_skill->stamina * SKILL_REDUCTION_FACTOR);
}
int get_current_ctrl(Skill pitcher_skill){
	return pitcher_skill->ctrl + (pitcher_skill->stamina * SKILL_REDUCTION_FACTOR);
}

void decrease_pitcher_skill(Skill pitcher_skill){
	
	pitcher_skill->vel -= SKILL_REDUCTION_FACTOR;
	pitcher_skill->mov -= SKILL_REDUCTION_FACTOR;
	pitcher_skill->ctrl -= SKILL_REDUCTION_FACTOR;
}

void tire_pitcher(Skill pitcher_skill){
	
	pitcher_skill->batters_left--;
	if(pitcher_skill->batters_left <= 0)
	{
		pitcher_skill->stamina--;
		pitcher_skill->batters_left = 3 + (pitcher_skill->end/20);
		decrease_pitcher_skill(pitcher_skill);
	}
}

void advance_to(Base base, struct runner_result *runner, ab_result* ab, int runner_id){
	
	runner->id = runner_id;
	runner->base = base;
	if(base == HOME){
		runner->result = SCORE;
		ab->runs++;
	}
	else{
		runner->result = SAFE;
	}
}

void advance_runners(ab_result* result,int runners[4],Hash skill_hash,int outs){
	
	struct runner_result *third = result->runner_results[3];
	struct runner_result *second = result->runner_results[2];
	struct runner_result *first = result->runner_results[1];
	
	if(runners[3]>0){
		switch(result->result_type){
			
			case BB:
				if(runners[2]==0 || runners[1]==0){
					break;
				}
			case IB:
			case ZB:
			case EB:
			case HR:
				advance_to(HOME, third, result, runners[3]);
				runners[3] = 0;
				break;
			default:
				break;
				
		}
		
	}
	if(runners[2]>0){
		Skill second_skill;
		int advance_prob;
		switch(result->result_type){
			case BB:
				if(runners[1]){
					advance_to(THIRD,second,result,runners[2]);
					runners[3] = runners[2];
					runners[2] = 0;
				}
				break;
				
			case IB:
/*Runners scoring from second on single:
 * We are assuming that runners try to score 60% of the time with <2 outs and
 * 80% of the time with 2 outs(These are guesses, not based on data).
 * We are also assuming that on average runners get throw out at the plate
 * 1% of the time when trying to score from second.
 */
				second_skill = Hash_get_value(skill_hash,(void*)runners[2]);
/*Based on the above comment, if advance_prop averages 200(which we assume is true)*/
/*then 200/334 = 60% and (200+67)/334 = 80%.*/
				advance_prob =  (3*second_skill->spd) + second_skill->intelligence;
				outs==2?advance_prob+=67:1;
				
				/*runner advances*/
				if(result->distance==1 && rand() % 334 < advance_prob){
					/*TODO: Factor in fielder's arm.*/
					/*TODO: Have runner thrown out sometimes.*/
					/*No break here, so we can fall through to ZB EB HR which*/
					/*handles the scoring for us.*/
				}
				else{
					advance_to(THIRD, second, result, runners[2]);
					runners[3] = runners[2];
					runners[2] = 0;
					break;
				}
			case ZB:
			case EB:
			case HR:
				advance_to(HOME, second, result, runners[2]);
				runners[2] = 0;
				break;
			default:
				break;
		}
		
	}
	if(runners[1] > 0){
		int advance_prob;
		Skill first_skill;
		switch(result->result_type){
			case BB:
				advance_to(SECOND, first, result, runners[1]);
				runners[2] = runners[1];
				runners[1] = 0;
				break;
			case IB:
/*Runners advancing from first to third on a single:
 * We are assuming that runners try to advance to third on a single 40% of the
 * time(This is a guess not based on any data).  In the future we will adjust
 * this percentage based on where the single is hit.
 */
				first_skill = Hash_get_value(skill_hash,(void*)runners[1]);
/*advance_prob should average 200, so 200/500 = 40%*/
				advance_prob = (3*first_skill->spd) + first_skill->intelligence; 
				/*runner advances*/
				if(!runners[3] && result->distance == 1 && rand()%500 <advance_prob){
					advance_to(THIRD, first, result, runners[1]);
					runners[3] = runners[1];
				}
				else{
					advance_to(SECOND, first, result, runners[1]);
					runners[2] = runners[1];
				}
				runners[1] = 0;
				break;
			case ZB:
/* Runners scoring from first on a double.
 * We are assuming that runners score from first on a double 80% of the time
 * (this is a guess and not supported by data) advance_prob should average
 * 200, so 200/250 = 80%
 */
				first_skill = Hash_get_value(skill_hash,(void*)runners[1]);
				advance_prob = (3*first_skill->spd) + first_skill->intelligence;
				/*runner advances*/
				if(result->distance == 1 && rand()%250 < advance_prob){
					advance_to(HOME,first,result,runners[1]);
				}
				else{
					advance_to(THIRD,first,result,runners[1]);
					runners[3] = runners[1];
				}
				runners[1] = 0;
				break;
			case EB:
			case HR:
				advance_to(HOME, first, result, runners[1]);
				runners[1] = 0;
				break;
			default:
				break;
		}
		
		
		
	}
	switch(result->result_type){
		case IB:
		case BB:
				
			runners[1] = runners[0];
			break;
		case ZB:
			runners[2] = runners[0];
			break;
		case EB:
			runners[3] = runners[0];
			break;
	}
}

int get_player_bat_order(int player_id,int lineup[10]){
	
	int i;
	/*Start at 1 here since there is no 0 spot in the lineup.*/
	for(i=1;likely(i<10);i++){
		if(lineup[i] == player_id){
			return i;
		}
	}
	return -1;
}

int is_pitcher_tired(Skill pitcher_skill){
	
	
	int runs_allowed = pitcher_runs[!team_hitting];
	PRINT_MESSAGE(1,"Is pitcher tired? runs: %d position: %d stamina: %d\n",runs_allowed,pitcher_skill->position,pitcher_skill->stamina);
	if(runs_allowed>=6){
		return 1;
	}
	if(runs_allowed>3 && pitcher_skill->position == 0){
		return 1;
	}
			
	if(inning_over){/*We are at the end of an inning.*/

		if(runs_allowed >3 && pitcher_skill->stamina<-1){
			return 1;
		}
		if(runs_allowed >0 && pitcher_skill->stamina<0 && pitcher_skill->position == 0){
			return 1;
		}

	}
	if(pitcher_skill->stamina <=-2){
		return 1;
	}
	return 0;
}
/*DEBUG*/
void print_bullpen(Hash skill_hash,struct linked_list *bullpen){

	struct linked_list *temp = bullpen;
	while(temp!=NULL){
		Skill pitcher_skill = Hash_get_value(skill_hash,temp->value);
		fprintf(stderr,"\t%s(%d) batters_left: %d\n",pitcher_skill->last_name,pitcher_skill->id,pitcher_skill->batters_left);
		temp = temp->next;
	}

	
}
/*DEBUG*/

int sub_pitcher(Db_Object db,Skill current_pitcher_skill,int* pitcher_id,int lineup[10],struct linked_list **bullpen,Hash skill_hash,struct linked_list **tired_pitchers){

	struct linked_list *l;
	if(is_pitcher_tired(current_pitcher_skill)){
		struct linked_list **temp = bullpen;
		while(*temp!=NULL){
			Skill sub_skill = (Skill)Hash_get_value(skill_hash,(void*)(*temp)->value);
			if(sub_skill->position == 0){
				update_pitcher_stamina(tired_pitchers,current_pitcher_skill);
				pitcher_runs[!team_hitting] = 0;
				int bat_order = get_player_bat_order(*pitcher_id,lineup);
				*pitcher_id = sub_skill->id;
				v_write_game_log_buffer("*%d=%s=%d=%d\n",sub_skill->id,sub_skill->last_name,1,bat_order);
				l = Linked_List_remove_value(bullpen,(void*)sub_skill->id);
				free(l);
				lineup[bat_order] = sub_skill->id;
#ifdef _PRINTPITCHINGCHANGES_
				fprintf(stderr,"Pitching change: old_batters_left=%d old_endurance=%d new_batters_left=%d new_endurance=%d\n",current_pitcher_skill->batters_left,current_pitcher_skill->end, sub_skill->batters_left, sub_skill->end);
#endif
				return 1;
			}
			temp = &(*temp)->next;	
		}
	}
	return 0;
}

void sim_ab(ab_result* result,Skill hitter_skill, Skill pitcher_skill,int defense[10],Hash skill_hash){

	int contact;
	int power;
	
	result->outs =0;
	result->runs =0;
	result->x_result = NONE;
	Hand hitter_hand = get_hitter_hand(hitter_skill->bats,pitcher_skill->throws);
	if(hitter_hand == RIGHT){
		contact = hitter_skill->cvr;
		power = hitter_skill->pvr;
	}
	else{
		contact = hitter_skill->cvl;
		power = hitter_skill->pvl;
	}
	int hitter_intel = hitter_skill->intelligence;
	int pitcher_intel = pitcher_skill->intelligence;
	int control = pitcher_skill->ctrl;
	int movement = pitcher_skill->mov;
	int velocity = pitcher_skill->vel;
	int speed = hitter_skill->spd;
	
	/*CALCULATE RESULT*/
	int random = rand()%1000;
	PRINT_RANDOM(random);
	
	double gb = GB_C + (0.25 * calculate_delta(((control * .2) + (movement * .8))*PITCHER_MODIFIER, ((0.9 * contact) + (0.1 * hitter_intel)) * HITTER_MODIFIER));
	if(random<=gb){
		result->result_type = GB;
		result->outs = 1;
	}
	else{
		double fb = gb + FB_C + (0.25 * calculate_delta(((control * .2) + (velocity * .8))*PITCHER_MODIFIER, ((0.9*contact) + (0.1 *hitter_intel)) * HITTER_MODIFIER));
		if(random<=fb){
			result->result_type = FB;
			result->outs = 1;
		}
		else{
			double k = fb + K_C + (1*calculate_delta(((.35*movement)+(.35*velocity)+(.15*control)+ (.15*pitcher_intel)) * PITCHER_MODIFIER,((.9*contact) + (.1*hitter_intel)) * HITTER_MODIFIER));
			if(random<= k){
				result->result_type = SO;
				result->outs = 1;
			}
			else{
				double walk = k + BB_C + (1*calculate_delta(((power*.4)+(hitter_intel*.3)+(.3*contact))*HITTER_MODIFIER,((.5*control) + (.5*pitcher_intel))*PITCHER_MODIFIER));
				if(random<= walk){
					result->result_type = BB;
				}
				else{
					double hr = HR_C + (4.5 * calculate_delta(power,63));
					double triple = hr + Triple_C +(0.75*calculate_delta((.8*speed) + (.2 * power),48));
					double doub = triple + Double_C + (2 *calculate_delta((.7*power) + (.3 * speed),47));
					int rand_hit = rand()%1000;
					if(rand_hit<= hr){
						result->result_type = HR;
						result->runs =1;
					}
					else if(rand_hit<= triple){
						result->result_type = EB;

					}
					else if(rand_hit<= doub){
						result->result_type = ZB;
					}
					else{
						result->result_type = IB;
					}
				}
			}
		}
	}
/*FIELDER STUFF*/
	
	int hit_to;
	int ran = rand();
	switch(result->result_type){
		case IB:
		case ZB:
		case EB:
		case HR:
			result->distance = 1;
			break;

		case FB:
			if(ran%100<(power*100/(power+contact))){
				result->distance =1;
			}
			else{
				result->distance = 0;
			}
			break;
		default:
			result->distance = 0;
	}
	
	int p_prob;
	int c_prob;
	int fb_prob;
	int sb_prob;
	int tb_prob;
	int ss_prob;
			
	int lf_prob;
	int cf_prob;
	int rf_prob;
			
	if(result->distance == 0){
			
		if(get_hitter_hand(hitter_skill->bats,pitcher_skill->throws) == RIGHT){
				
			p_prob = R_P_C;
			c_prob = R_C_C+p_prob;
			fb_prob = R_FB_C+c_prob;
			sb_prob = R_SB_C+fb_prob;
			tb_prob = R_TB_C+sb_prob;
			ss_prob = R_SS_C+tb_prob;
		}
		else{
			p_prob = L_P_C;
			c_prob = L_C_C+p_prob;
			fb_prob = L_FB_C+c_prob;
			sb_prob = L_SB_C+fb_prob;
			tb_prob = L_TB_C+sb_prob;
			ss_prob = L_SS_C+tb_prob;
		}
		int position = rand()%1000;
		if(position<p_prob){
			hit_to = 1;
		}
		else if(position<c_prob){
			hit_to = 2;
		}
		else if(position<fb_prob){
			hit_to = 3;
		}
		else if(position<sb_prob){
			hit_to = 4;
		}
		else if(position<tb_prob){
			hit_to = 5;
		}
		else{
			hit_to = 6;
		}			
	}
	else{
		lf_prob = LF_C;
		cf_prob = CF_C + lf_prob;
		rf_prob = RF_C + cf_prob;
				
		int position = rand()%1000;
		if(position<lf_prob){
			hit_to = 7;
		}
		else if(position<cf_prob){
			hit_to = 8;
		}
		else{
			hit_to = 9;
		}
	}	
	Skill defense_skill = (Skill)Hash_get_value(skill_hash,(void*)defense[hit_to]);
#ifdef _ADVANCED_DEFENSE_
	apply_defense(result,defense_skill,hitter_skill->spd);
#endif
	switch(result->result_type){
		
		case GB:
			Array_List_add(result->defense,(void*)defense_skill->id);
			Array_List_add(result->defense,(void*)defense[3]);
			break;
		case HR:
			break;
		case SO:
			Array_List_add(result->defense,(void*)defense[2]);
			break;
		default:
			Array_List_add(result->defense,(void*)defense_skill->id);
			break;
	}
}

void increment_bat_order(int order[2],int team){
	
	order[team]++;
	if(order[team] == 10){
		order[team] =1;
	}
}

void check_starter(Skill skill){

	int max_batters_left = 3 + ((3*skill->end)/10);
	if(skill->batters_left < max_batters_left){
		PRINT_WARNING("SP %s(%d) has %d/%d batters left\n",skill->last_name,skill->id,skill->batters_left,max_batters_left);
	}
}


/*
 * @return \sa sim_game_batch
 */
int sim_game(Db_Object db, int fd, int home_id, int away_id, double hitter_modifier, double pitcher_modifier, double ab_exponent, struct linked_list **tired_pitchers){

	int rc;
	Hash skill_hash = Hash_create(100);
	select_skills(db,home_id,away_id,skill_hash);
	rc = sim_game_batch(db,fd,home_id,away_id,hitter_modifier,pitcher_modifier,ab_exponent,skill_hash, tired_pitchers);
	Hash_free(skill_hash, Skill_free);
}

/*!
 * @return 0 for success, something else if there is an error.
 */
int sim_game_batch(Db_Object db, int fd, int home_id, int away_id, double hitter_modifier, double pitcher_modifier, double ab_exponent,Hash skill_hash, struct linked_list **tired_pitchers){

	ab_result result;
	int x;
	int rc = 0;
	for(x=0;x<4;x++){
		result.runner_results[x] = calloc(1, sizeof(struct runner_result));
	}
	result.defense = Array_List_create(sizeof(int));
	PRINT_MESSAGE(1,"Starting game home: %d away: %d\n",home_id,away_id);
	HITTER_MODIFIER = hitter_modifier;
	PITCHER_MODIFIER = pitcher_modifier;
	AB_EXPONENT = ab_exponent;
	FD = fd;
	
	pitcher_runs[0] = 0; pitcher_runs[1] =  0;
	team_hitting = AWAY_TEAM;
	inning_over = 0;
	
	int outs =0;
	int inning =1;

	int runs[2] = {0,0};
	int order_at_bat[2] = {1,1};
	int base_runners[4] = {0,0,0,0};

	struct pitcher_stat home_stat = {0,0};
	struct pitcher_stat away_stat = {0,0};
	Pitcher_Stat pitcher_stats[2] = {&home_stat,&away_stat};
	
/*Stuff to move to sim_day to speed this up.*/	
	int offense[2][10];
	int defense[2][10];
	struct linked_list *bench[2] = {NULL,NULL};
	struct linked_list *bullpen[2] = {NULL,NULL};
	
	memset(offense, 0, sizeof(int) * 20);
	memset(defense, 0, sizeof(int) * 20);

	v_write_game_log_buffer("+\nHOME=%d\nAWAY=%d\n",home_id,away_id);

	if(select_lineups(db,home_id,away_id,offense,defense,bench,bullpen,skill_hash)){
		rc = 1;
		goto exit;
	}
	if(verify_lineups(offense, defense)){
		fprintf(stderr, "Invalid lineup %d vs %d\n",home_id, away_id);
		rc = 1;
		goto exit;
	}
#ifdef DEBUG
			int x,y,team_id;
			for(y=0;y<2;y++)
				if(y==0){
					team_id = home_id;
				}
				else{
					team_id = away_id;
				}
				for(x=1;x<10;x++){
				fprintf(stderr,"Team: %d",team_id);
				Skill skill = Hash_get_value(skill_hash,(void*)defense[y][x]);
				print_skill(skill);
			}
#endif
/*-------------------------------------------*/
	
	int sub_allowed = 1;
	int i,j;
#ifdef _SPEEDY_
	PRINT_MESSAGE(1,"Printing lineups...\n");
	for(j=0;j<=1;j++){
		PRINT_MESSAGE(1,"Team: %d\n",0);
		for(i=1;i<10;i++){
			int player_id = defense[j][i];
			PRINT_MESSAGE(1,"Player: %d is playing %d\n",player_id,i);
			int bat_order = indexOf((void**)offense[j],(void*)player_id,10);
			PRINT_MESSAGE(1,"Bat order is %d\n",bat_order);
			Skill skill = Hash_get_value(skill_hash,(void*)player_id);
			print_player(player_id,bat_order,i,skill->last_name);
			PRINT_MESSAGE(1,"Done printing player.\n");
		}
		if(j==0){
			write_game_log_buffer("`\n",2);
		}
	}
	PRINT_MESSAGE(1,"Done.\n");
#endif

	/*DEBUG*/
	Skill home_starter_skill = Hash_get_value(skill_hash,(void*)defense[HOME_TEAM][1]);
	if(unlikely(!home_starter_skill)){
		skill_not_found(defense[HOME_TEAM][1], home_id);
		rc = 1;
		goto exit;
	}
	Skill away_starter_skill = Hash_get_value(skill_hash,(void*)defense[AWAY_TEAM][1]);
	if(unlikely(!away_starter_skill)){
		skill_not_found(defense[AWAY_TEAM][1], away_id);
		rc = 1;
		goto exit;
	}
	check_starter(home_starter_skill);
	check_starter(away_starter_skill);
	/*DEBUG*/
	int pitcher_id;
	Skill pitcher_skill;
	print_new_inning();
	UPDATE_PITCHER()
	while(!is_game_over(inning,team_hitting,runs,inning_over)){
		if(inning_over){
			print_new_inning();
			base_runners[0]=base_runners[1]=base_runners[2]=base_runners[3] = 0;
			UPDATE_PITCHER()
		}
		if(sub_pitcher(db,pitcher_skill,&defense[!team_hitting][1],offense[!team_hitting],&bullpen[!team_hitting],skill_hash,tired_pitchers)){
			UPDATE_PITCHER()
		}
			
		inning_over = 0;
		
		/*sub_hitter();*/

		/*steal_base();*/
		
		int hitter_id = offense[team_hitting][order_at_bat[team_hitting]];
		Skill hitter_skill = Hash_get_value(skill_hash,(void*)hitter_id);
		if(unlikely(!hitter_skill)){
			PRINT_ERROR("Hitter not in skill_hash: hitter_id=%d home=%d away=%d team_hitting =%d order=%d\n",hitter_id,home_id,away_id,team_hitting,order_at_bat[team_hitting]);
			rc = 1;
			goto exit;
		}

		base_runners[0] = hitter_id;
#ifdef _PRINTAB_
		fprintf(stderr,"At bat: cvr=%d cvl=%d vs. ctrl=%d mov=%d vel=%d end=%d bl=%d\n",hitter_skill->cvr,hitter_skill->cvl,pitcher_skill->ctrl,pitcher_skill->mov,pitcher_skill->vel, pitcher_skill->end, pitcher_skill->batters_left);
#endif
		sim_ab(&result,hitter_skill,pitcher_skill,defense[!team_hitting],skill_hash);
#ifdef _PRINTAB_
		fprintf(stderr,"At bat: result is %c\n", result_type_to_char(result->result_type));
#endif
		advance_runners(&result,base_runners,skill_hash,outs);
		tire_pitcher(pitcher_skill);
		/*DEBUG*/
		if(unlikely(pitcher_skill->stamina <=-10)){
			PRINT_WARNING("Pitcher is dangerously tired: %d\n",pitcher_skill->stamina);
			if(pitcher_skill->stamina<=-20){
				PRINT_ERROR("Pitcher is too tired.\n");
				rc = 1;
				goto exit;
			}
		}
		/*DEBUG*/
		increment_bat_order(order_at_bat,team_hitting);
		print_result(&result,hitter_skill,pitcher_skill);
		runs[team_hitting]+=result.runs;
		outs+= result.outs;
		pitcher_runs[!team_hitting] += result.runs;
		ab_result_clear(&result);
		/*DEBUG*/
		if(unlikely(runs[team_hitting] > 20)){
			PRINT_WARNING("Team hitting has %d runs.\n",runs[team_hitting]);
		}
		/*DEBUG*/
		if(outs == 3){
			if(team_hitting == HOME_TEAM){
				inning ++;
				team_hitting = AWAY_TEAM;
			}
			else{
				team_hitting = HOME_TEAM;
			}
			inning_over = 1;
			outs = 0;
			if(unlikely(inning>20)){
				PRINT_WARNING("The current inning is %d.\n",inning);
			}
		}
	}
	update_pitcher_stamina(tired_pitchers,Hash_get_value(skill_hash,(void*)defense[HOME_TEAM][1]));
	update_pitcher_stamina(tired_pitchers,Hash_get_value(skill_hash,(void*)defense[AWAY_TEAM][1]));
	write_game_log_buffer("=\n",2);	
	flush_game_log_buffer(FD);
exit:
	Linked_List_free(bench[0], NULL);
	Linked_List_free(bench[1], NULL);
	Linked_List_free(bullpen[0], NULL);
	Linked_List_free(bullpen[1], NULL);
	return rc;
}
