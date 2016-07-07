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

#ifndef BASEBALL_OBJECTS_H
#define BASEBALL_OBJECTS_H

typedef enum hand{RIGHT,LEFT,SWITCH}Hand;

typedef enum player_status{DRAFTEE,ROSTER,MINOR,WAIVER,FA,REITRED}Player_Status;

typedef struct skill{
	
	int id;
	int cvr;
	int pvr;
	int cvl;
	int pvl;
	int spd;
	int h_energy;
	
	int vel;
	int mov;
	int ctrl;
	int end;
	int batters_left;

	int range;
	int arm;
	int field;
	int one;
	int two;
	int three;
	int four;
	int five;
	int six;
	int seven;
	int eight;
	int nine;
	
	int intelligence;
	Hand bats;
	Hand throws;
	
	char *last_name;
	int position;
	
	int stamina;
}*Skill;

void Skill_free(Skill skill);

/*
typedef struct team{
	
	int id;
	int league_id;
	int division;
	int city;
	int mascot;
}*Team;

typedef struct hitter_stat{
	
	int id;
	int g;
	int ab;
	int bb;
	int h;
	int Ib;
	int Zb;
	int Eb;
	int hr;
	int r;
	int rbi;
	int hbp;
	int so;
	int hand;
	
}*Hitter_Stat;

typedef struct pitcher_stat_common{
	
	int id;
	int g;
	int outs;
	int bb;
	int h;
	int r;
	int er;
	int hbp;
	int so;
	int w;
	int l;
	int sv;
}*Pitcher_Stat_Common;

typedef struct fielder_stat{
	
	int id;
	int a;
	int po;
	int e;
	int dp;
	int good;
	int bad;
	int cs;
	int sb;
	int pos;
	
}*Fielder_Stat;
*/
typedef struct hitter_skill{
	
	int id;
	int cvr;
	int pvr;
	int cvl;
	int pvl;
	int spd;
	int energy;
}*Hitter_Skill;

typedef struct pitcher_skill{
	
	int id;
	int vel;
	int mov;
	int ctrl;
	int end;
	int energy;
}* Pitcher_Skill;

typedef struct fielder_skill{
	int id;
	int range;
	int arm;
	int field;
	double one;
	double two;
	double three;
	double four;
	double five;
	double six;
	double seven;
	double eight;
	double nine;
}*Fielder_Skill;
	
typedef struct player{
	
	int id;
	int team_id;
	char* first_name;
	char* last_name;
	int position;
	/*Date birth_day;*/
	int intelligence;
	int health;
	Hitter_Skill hitter_skill;
	Pitcher_Skill pitcher_skill;
	Fielder_Skill fielder_skill; 
	/*Hitter_Stat hitter_stat[2];
	Pitcher_Stat_Common pitcher_stat[2];
	Fielder_Stat fielder_stat;*/
	Hand bats;
	Hand throws;
	
}*Player;

#endif /* BASEBALL_OBJECTS_H */
