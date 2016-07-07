#!/usr/bin/perl -w

use lib '../src';

use strict;

use CGI ':standard','*table';
use DBI;

use Baseball;
use Links;
use Data_Interface;
use Fbs_utils;

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
if (!$sort){
	$sort = qq{BA};
}
my $DB_TYPE = "mysql";
my $dbh = Data_Interface::connect($DB_TYPE);
my $query = qq{CALL sp_select_stat_skill_hitter_league($id,'$sort')};
my $stmnt = $dbh->prepare($query);
$stmnt->execute;


print header;
print start_html(-title=>"Free Baseball Simulator - $stat_type stats",-style=>{'src'=>'/style.css'});
print h1({-align=>"center"},"Hitter Stats");
#print p $query;

######LINKS#####
print div_start("links");
print main_links($session);
print div_end();
###########

#####STATS#####
print div_start("content");
print start_table;
print Tr({-align=>"left"}, th ['Name','G',sort_link('AB'),sort_link('H'),sort_link('R'),sort_link('2B'),sort_link('3B'),sort_link('HR'),sort_link('RBI'),sort_link('SO'),sort_link('BB'),sort_link('BA'),sort_link('CVR'),sort_link('PVR'),sort_link('CVL'),sort_link('PVL'),sort_link('SPD'),sort_link('INTEL')]);
while(my @row = $stmnt->fetchrow_array){
	

			print Tr td ["$row[0] $row[1]",$row[2],$row[3],$row[4],$row[5],$row[6],$row[7],$row[8],$row[9],$row[10],$row[11],ba($row[12]),$row[13],$row[14],$row[15],$row[16],$row[17],$row[18]];
}
print end_table;
print div_end();
##########
print end_html;
