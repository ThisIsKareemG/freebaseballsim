#!/usr/bin/perl

use strict;
use lib '../src';
use CGI ':standard','*table';
use DBI;

use Fbs_utils;
use Data_Interface;

#####GET PARAMETERS#####
my $session = get_session();
my $LEAGUE_ID = get_league_id($session);

#####CONNECT TO DB#####
my $DB_TYPE = 'mysql';
my $dbh = Data_Interface::connect($DB_TYPE);

#####BEGIN HTML#####
print header;
print start_html(-title=>"Free Baseball Simulator - Standings",-style=>{'src'=>'/style.css'});
print h1({-align=>"center"},"Standings");

#####LINKS#####
print div_start("links");
#TODO: When we have more than one league display this drop down.
league_popup_form($dbh,\$LEAGUE_ID);
print main_links($session);
print div_end();

#####STANDINGS#####
my $query = qq{
	select city,mascot,wins,loses from t_team where league_id=$LEAGUE_ID and active=1
	order by wins desc
};
my $stmnt = $dbh->prepare($query) or die($query);
$stmnt->execute or die($query);

print div_start("content");
print start_table;
print Tr({-align=>"left"},th ['Team','Wins','Loses']);
while(my @row = $stmnt->fetchrow_array){
	print Tr td ["$row[0] $row[1]",$row[2],$row[3]];
}
print div_end();
print end_html;

