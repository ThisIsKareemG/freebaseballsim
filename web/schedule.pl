#!/usr/bin/perl

use strict;
use lib '../src';
use CGI ':standard','*table';
use DBI;

use Fbs_utils;
use Data_Interface;

#####CONNECT TO DB#####
my $DB_TYPE = 'mysql';
my $dbh = Data_Interface::connect($DB_TYPE);

#####GET PARAMETERS#####
my $session = get_session();
my $TEAM_ID = get_team_id($session,$dbh);

#####BEGIN HTML#####
print header;
print start_html(-title=>"Free Baseball Simulator - Schedule",-style=>{'src'=>'/style.css'});
print h1({-align=>"center"},"Schedule");

#####LINKS#####
print div_start("links");
print main_links($session);
print div_end();

#####SCHEDULE#####
print div_start("content");
print 'Select a team:';
print team_popup_form($dbh,1,$TEAM_ID), p;
my $query= 'select boxscore_id,home_id,away_id,game,home_team.city,' .
	'away_team.city from t_boxscore inner join t_team home_team ' .
	'on t_boxscore.home_id = home_team.team_id inner join ' .
	't_team away_team on t_boxscore.away_id = away_team.team_id	where '.
	'(home_id=? or away_id=?) and home_team.active=1 and away_team.active=1';
#print $query,' ', $TEAM_ID;
my $stmt = $dbh->prepare($query);
if($stmt){
if($stmt->execute($TEAM_ID,$TEAM_ID)){

while(my @row = $stmt->fetchrow_array){
	my $links = "game.pl?game=$row[0]";
	print $row[3] . " ";
	if($row[1] == $TEAM_ID){#Home game
		print a({href=>$links},uc($row[5]));
	}
	else{					#Away game
		print a({href=>$links},"@" . $row[4]);
	}
	print br;
}
}
}
print div_end();
print end_html();
