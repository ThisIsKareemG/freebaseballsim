#!/usr/bin/perl -w

use strict;

use lib '../src';
use CGI ':standard','*table';
use DBI;

use Fbs_utils;
use Data_Interface;
use Baseball;
use Links;

#PARAMETERS:
my $LEAGUE = 'league';
my $TEAM = 'team';
my $SORT = 'sort';
#END PARAMETERS

my $stat_type;
my $id;
my $session = get_session();
if($id = param($LEAGUE)){
	$stat_type = $LEAGUE;
}
elsif($id = param($TEAM)){
	$stat_type = $TEAM;
}
else{
#die();
$stat_type = $LEAGUE;
$id =1;
}
my $sort = param($SORT);
if(!$sort){
	$sort = 'IP';
}
my $DB_TYPE = 'mysql';
my $dbh = Data_Interface::connect($DB_TYPE);
my $query = qq{CALL sp_select_stat_skill_pitcher_league($id,'$sort')};
my $stmnt = $dbh->prepare($query);
$stmnt->execute;

print header;
print start_html(-title=>"Free Baseball Simulator - $stat_type stats",-style=>{'src'=>'/style.css'});
print h1({-align=>'center'},"Pitcher Stats");

#####LINKS#####
print div_start("links");
print main_links($session);
print div_end();
##########

#####STATS######
print div_start("content");
print start_table;
print Tr({-align=>"left"}, th ['Name', sort_link('G'), sort_link('W'), sort_link('L'), sort_link('SV'), sort_link('IP'), sort_link('H'), sort_link('BB'),sort_link('R'), sort_link('ER'), sort_link('SO'), sort_link('ERA'),sort_link('VEL'),sort_link('MOV'),sort_link('CTRL'),sort_link('END'),sort_link('INTEL'),sort_link('ENERGY')]);


while(my @row = $stmnt->fetchrow_array){

	print Tr td ["$row[0] $row[1]", $row[2], $row[3], $row[4], $row[5], outs_to_ip($row[6]), $row[7], $row[8], $row[9], $row[10], $row[11], $row[12],$row[13],$row[14],$row[15],$row[16],$row[17],$row[18]];
}
print end_table;
print div_end();
##########
print end_html;
