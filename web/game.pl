#!/usr/bin/perl

use strict;
use lib '../src';
use CGI ':standard';
use DBI;
use Data_Interface;
use Fbs_utils;

#####PARAMETERS#####
my $PNAME_GAME = 'game';

#####GET PARAMETERS#####
my $session = get_session();
my $GAME_ID = param($PNAME_GAME);

#####CONNECT TO DB#####
my $DB_TYPE = 'mysql';
my $dbh = Data_Interface::connect($DB_TYPE);

#####BEGIN HTML#####
my $css = "<style type=\"text/css\">#play{position:absolute; left:350px; top:0px; width:400px; height:500px;}</style>";
print header;
print start_html(-title=>"Free Baseball Simulator - Box Score",
				-style=>{'src'=>'/style.css'});
print $css;
print h1({-align=>"center"},"Box Score");

#####LINKS#####
print div_start("links");
print main_links($session);
print div_end();

#####BOX SCORE#####
print div_start("content");
my $query = "select boxscore,playbyplay from t_boxscore where boxscore_id=$GAME_ID";
my $stmnt = $dbh->prepare($query) or die($query);
$stmnt->execute or die();
my @row = $stmnt->fetchrow_array;
print $row[0];
print div_start("play");
print $row[1];
print div_end();

print div_end();
print end_html();
