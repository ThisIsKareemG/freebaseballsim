#!/usr/bin/perl -w
#Free Baseball Simulator - A configurable baseball simulator by Tom Stellard
#Copyright (C) 2009 Tom Stellard
#
#This file is part of Free Baseball Simulator
#
#Free Baseball Simulator is free software: you can redistribute it and/or modify
#it under the terms of the GNU General Public License as published by
#the Free Software Foundation, either version 3 of the License, or
#(at your option) any later version.
#
#Free Baseball Simulator is distributed in the hope that it will be useful,
#but WITHOUT ANY WARRANTY; without even the implied warranty of
#MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#GNU General Public License for more details.
#
#You should have received a copy of the GNU General Public License
#along with Free Baseball Simulator.  If not, see <http://www.gnu.org/licenses/>.
use strict;

use lib '../src';

use DBI;
use Getopt::Long;

use Data_Interface;

my $DB_TYPE = 'sqlite';

my $league_id = 1;

GetOptions(
'db=s' => \$DB_TYPE
);

my @connect_info = Data_Interface::read_config($DB_TYPE);

my $dbh = DBI->connect(Data_Interface::driver($DB_TYPE) . ":dbname=$connect_info[0]", $connect_info[1], $connect_info[2]);

#NOTE: The league's year is updated at the end of a season by the sim_season
#function, so we don't need to do it here.

my $stmnt = $dbh->prepare("Select t_player.player_id,t_player.team_id,t_league.league_date from t_player inner join t_team on t_player.team_id = t_team.team_id inner join t_league on t_team.league_id = t_league.league_id");

$stmnt->execute;

while(my @row = $stmnt->fetchrow_array){
	#TODO: Get the current year outside of this loop.
	my $year = Data_Interface::date_to_year($DB_TYPE, $row[2]);
	my $insert_hit = "Insert into t_stat_hitter(player_id,team_id,hand,year) values($row[0],$row[1],%d,$year)";
	my $insert_pitch = "Insert into t_stat_pitcher(player_id,team_id,hand,year) values($row[0], $row[1], %d, $year)";
	my $insert_field = "Insert into t_stat_fielder(player_id,team_id,year) values($row[0],$row[1],$year)";

	#Insert new hitter stats
	my $query = sprintf($insert_hit, 0);
	my $insert_stmnt = $dbh->prepare($query) or die($query);
	$insert_stmnt->execute;
	$query = sprintf($insert_hit, 1);
	$insert_stmnt = $dbh->prepare($query) or die($query);
	$insert_stmnt->execute;
	
	#Insert new pitcher stats
	$query = sprintf($insert_pitch, 0);
	$insert_stmnt = $dbh->prepare($query) or die($query);
	$insert_stmnt->execute;
	$query = sprintf($insert_pitch, 1);
	$insert_stmnt = $dbh->prepare($query) or die($query);
	$insert_stmnt->execute;
	
	#Insert new fielder stats
	$insert_stmnt = $dbh->prepare($insert_field) or die($insert_field);
	$insert_stmnt->execute;
}

init_lineups();


$stmnt = $dbh->prepare("Update t_skill_pitcher set batters_left = 3 + round((end * 3) / 10.0)");
$stmnt->execute;

sub init_lineups{
	
	my @hitters;

	$dbh->do("update t_lineup set bat_order=0, position=0;");

	$dbh->do("update t_rotation set next_starter=0;");
	
	#TODO: Query should include league_id and exclude free agents. Maybe it is OK to include free agents,
	#It doesn't matter if they have an entry in the lineup table.
	my $stmnt = $dbh->prepare("select player_id,t_player.team_id,t_player.position from t_player inner join (select team_id,position, max(overall) as overall from t_player group by team_id,position having position>1) starters on t_player.team_id = starters.team_id and t_player.position = starters.position and t_player.overall = starters.overall;");
	
	$stmnt->execute;

	while (@hitters = $stmnt->fetchrow_array()){
		
		my $player_id = $hitters[0];
		my $position = $hitters[2];
		my $bat_order = $position - 1;
		$dbh->do("Update t_lineup set bat_order = $bat_order, position = $position where player_id = $player_id");
	}
	$stmnt->finish;

	$stmnt = $dbh->prepare("select player_id,team_id,overall from t_player where position =1 order by overall desc") or die();
	$stmnt->execute;

	my %starters;

	my @pitchers;
	while (@pitchers = $stmnt->fetchrow_array()){

		my $player_id = $pitchers[0];
		my $team_id = $pitchers[1];
		my $order;
		my $next_starter = 0;
		if(!defined($starters{$team_id})){
			$starters{$team_id} = 1;
			$order = 1;
			$next_starter = 1;
			$dbh->do("update t_lineup set bat_order = 9, position = 1 where player_id = $player_id");
		}
		else{
			$order = ++$starters{$team_id};
		}

		$dbh->do("update t_rotation set position = $order, next_starter = $next_starter where player_id = $player_id");
	}
	$stmnt->finish;
}	
