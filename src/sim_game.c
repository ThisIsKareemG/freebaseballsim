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
#include <getopt.h>
#include "baseball_macros.h"
#include "baseball_objects.h"
#include "hash.h"
#include "sim_game.h"
#include "db_object.h"


void print_usage(){
	
	printf("Usage: sim_game <home_id> <away_id>\n\n");
}

int main(int argc, char **argv){
	
	int seed = 0;
	int c;
	int option_index = 0;
	double HITTER_MODIFIER= 1.0;
	double PITCHER_MODIFIER= 1.0;
	double AB_EXPONENT = 1.0;
	static struct option long_options[] = 
	{
	{"seed",required_argument,0,'s'},
	LONG_OPTIONS_STRUCT,
	{0,0,0,0}
	};
		if(argc < 2){
		print_usage();
		exit(1);
	}
	int home_id = atoi(argv[1]);
	int away_id = atoi(argv[2]);
	while(1){
		c = getopt_long(argc,argv,"s:h:p:x:",long_options,&option_index);
		if(c == -1){
			break;
		}
		switch(c){
			LONG_OPTIONS_CASE
			case 's':
				seed = atoi(optarg);
				break;
			
			case '?':
				printf("????\n");
				break;
			default:
				printf("default\n");
				break;
		}
	}
	
	Db_Object db = db_connect(CONFIG_FILE);
	
	PRINT_MESSAGE(1,"home: %d away: %d hit: %lf pitch: %lf ab_exp: %lf\n",home_id,away_id,HITTER_MODIFIER,PITCHER_MODIFIER,AB_EXPONENT);
	sim_game(db,1,home_id,away_id,HITTER_MODIFIER,PITCHER_MODIFIER,AB_EXPONENT,NULL);
	db->close_connection(db->conn);
	free(db);
	return 0;
}
