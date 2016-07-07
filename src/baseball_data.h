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

#ifndef _BASEBALL_DATA_H
#define _BASEBALL_DATA_H

#include "db_object.h"
#include "tom_array_list.h"
#include "baseball_objects.h"

Array_List select_team_ids_for_league(Db_Object db, int league_id);

void insert_player(Db_Object db,Player player,Hitter_Skill hitter_skill,Pitcher_Skill pitcher_skill,Fielder_Skill fielder_skill,int league_id);

int select_year_for_league(Db_Object db, int league_id);

Date select_date_for_league(Db_Object db, int league_id);

int** select_schedule(Db_Object db, int league_id, int game_nam);

native_result* select_lineup(Db_Object db, int team_id, Hand opp_hand);

native_result* select_rotation(Db_Object db, int team_id);

Skill build_skill(Db_Object db, native_row row);

const char* SELECT_DRAFT_PREFS_FOR_LEAGUE;

const char* UPDATE_YEAR;

const char* SELECT_LINEUPS_FOR_LEAGUE;

const char* SELECT_ROTATION_FOR_LEAGUE;

const char* SELECT_SKILLS_FOR_LEAGUE;

const char* INSERT_SCHEDULE;

const char* UPDATE_BATTERS_LEFT;
#endif /* _BASEBALL_DATA_H */
