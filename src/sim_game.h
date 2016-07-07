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

#ifndef _SIM_GAME_H
#define _SIM_GAME_H

#include "db_object.h"
#include "hash.h"

int sim_game(Db_Object db, int fd, int home_id, int away_id, double hitter_modifier, double pitcher_modifier, double ab_exponent,struct linked_list **tired_pitchers);

int sim_game_batch(Db_Object db, int fd, int home_id, int away_id, double hitter_modifier, double pitcher_modifier, double ab_exponent, Hash skill_hash, struct linked_list **tired_pitchers);

#define LONG_OPTIONS_STRUCT \
{"hit-mod",required_argument,0,'h'},\
{"pitch_mod",required_argument,0,'p'},\
{"ab-exp",required_argument,0,'x'}

#define LONG_OPTIONS_STRING h:p:x:

#define LONG_OPTIONS_CASE \
			case 'h':\
				HITTER_MODIFIER = atof(optarg);\
				break;\
			case 'p':\
				PITCHER_MODIFIER = atof(optarg);\
				break;\
			case 'x':\
				AB_EXPONENT = atof(optarg);\
				break;

#define SKILL_REDUCTION_FACTOR 5

#endif /* _SIM_GAME_H */
