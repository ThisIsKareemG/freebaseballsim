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
my $SEED;
my $league_id;
my $num_players = 1200;
my $hitter_mean;
my $hitter_sd;
my $pitcher_mean;
my $pitcher_sd;
my $config_file;

GetOptions(
'db=s' => \$DB_TYPE,
'hitter-mean=f' => \$hitter_mean,
'hitter-sd=f' => \$hitter_sd,
'pitcher-mean=f' => \$pitcher_mean,
'pitcher-sd=f' => \$pitcher_sd,
'seed=i' => \$SEED,
'league-id=i' => \$league_id,
'players=i' => \$num_players,
'config=s' => \$config_file
);

my $dbh = Data_Interface::connect($DB_TYPE, $config_file);

$dbh->do("DELETE FROM t_lineup;");
$dbh->do("DELETE FROM t_rotation;");
$dbh->do("DELETE FROM t_draft_pref;");
$dbh->do("DELETE FROM t_draft;");

#Retire all the players
$dbh->do("UPDATE t_player set STATUS = 5, team_id = 0 where league_id = $league_id;");

#Update the year
my $query = qq{
	Select league_date from t_league where league_id = $league_id
};
my $stmnt = $dbh->prepare($query) or die($query);
$stmnt->execute or die($query);
my @row = $stmnt->fetchrow_array;
my $date = Data_Interface::date_add_year($DB_TYPE,$row[0],1);
$query="Update t_league set league_date = '$date' where league_id = $league_id";
$dbh->do($query) or die($query);

#Archive old season info, and add new row for each team for the new year.
$query="Select season_id,team_id,division,city,mascot from t_team where league_id = $league_id and active = 1";
$stmnt = $dbh->prepare($query) or die($query);
$stmnt->execute;
my $deactivate = "update t_team set active=0 where season_id=%d";
my $insert="insert into t_team(team_id,league_id,division,city,mascot,year) values(%d,%d,%d,'%s','%s','%s')";
my $new_year = Data_Interface::date_to_year($DB_TYPE,$date);
my $results = $stmnt->fetchall_arrayref;
foreach my $row_ref (@$results){
	my @row = @$row_ref;
	$dbh->do(sprintf($deactivate,$row[0])) or die();
	$dbh->do(sprintf($insert,$row[1],$league_id,$row[2],$row[3],$row[4],$new_year)) or die();
}
	
my @create_player = ("./create_player","--league-id=$league_id","--players=$num_players");
my @run_draft = ("./run_draft");

if(defined($hitter_mean)){
	push(@create_player,"--hitter-mean=$hitter_mean");
}
if(defined($hitter_sd)){
	push(@create_player,"--hitter-sd=$hitter_sd");
}
if(defined($pitcher_mean)){
	push(@create_player,"--pitcher-mean=$pitcher_mean");
}
if(defined($pitcher_sd)){
	push(@create_player,"--pitcher-sd=$hitter_sd");
}
if(defined($SEED)){
	push(@create_player,"--seed=$SEED");
	push(@run_draft,"--seed=$SEED");
}
if(defined($config_file)){
	push(@create_player,"--config=$config_file");
	push(@run_draft,"--config=$config_file");
}

system(@create_player) == 0
	or die "@create_player failed: $?";

system(@run_draft) == 0 
	or die "@run_draft failed: $?";

init_lineups();

exit(0);

sub init_lineups{
	
	$dbh->do("update t_lineup set bat_order=0, position=0;");

	$dbh->do("update t_rotation set next_starter=0;");
	
	insert_lineups("R");

	insert_lineups("L");

	my $stmnt = $dbh->prepare("select player_id,team_id,overall from t_player where position =1 order by overall desc") or die();
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

sub insert_lineups{

	my $hand = $_[0];
	my $hand_int = Data_Interface::hand_to_int($hand);
	my $ovr_formula = "((0.7*cv%s) + (0.3*pv%s) + (0.001*cv%s) + (0.00001*pv%s))";
	
	#TODO: Query should include league_id and exclude free agents. Maybe it is OK to include free agents,
	#It doesn't matter if they have an entry in the lineup table.
	my $select_top_players = "select t_player.player_id,t_player.team_id,t_player.position from t_player inner join t_skill_hitter on t_player.player_id = t_skill_hitter.player_id inner join (select team_id,position,max($ovr_formula) as overall from t_player inner join t_skill_hitter on t_player.player_id = t_skill_hitter.player_id group by team_id,position having position>1) starters on t_player.team_id = starters.team_id and t_player.position = starters.position and $ovr_formula=starters.overall;";
	my @hitters;

	my $query = sprintf($select_top_players,$hand,$hand,$hand,$hand,$hand,$hand,$hand,$hand);
	my $stmnt = $dbh->prepare($query) or die($query);	
	$stmnt->execute;

	while (@hitters = $stmnt->fetchrow_array()){
		
		my $player_id = $hitters[0];
		my $position = $hitters[2];
		my $bat_order = $position - 1;
		$dbh->do("Update t_lineup set bat_order = $bat_order, position = $position where player_id = $player_id and hand=$hand_int;");
	}
	$stmnt->finish;

}
