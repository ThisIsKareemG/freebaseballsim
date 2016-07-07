#!/usr/bin/perl

use strict;
use lib '../src';
use CGI ':standard','*table';
use DBI;

use Fbs_utils;
use Data_Interface;

#PARAMETERS
my $PNAME_HAND = 'hand';
my $PNAME_SUBMIT = 'submit';
#END PARAMETERS

#####CONNECT TO DB#####
my $DB_TYPE = 'mysql';
my $dbh = Data_Interface::connect($DB_TYPE);
if(!$dbh){
	print db_error();
	print end_html;
	exit(1);
}
#####GET PARAMETERS
my $session = get_session();
my $TEAM_ID = get_team_id($session, $dbh);
my $HAND = url_param($PNAME_HAND);
if(!$HAND){
	$HAND = param($PNAME_HAND);
}
my $SUBMIT = param($PNAME_SUBMIT);
my $hand_skills;
my $hand_int;
if($HAND eq 'l'){
	$hand_skills='CvL, PvL,';
	$hand_int = 1;
}
else{
	$hand_skills='CvR, PvR,';
	$hand_int = 0;
}

#####BEGIN HTML#####
print header;
print start_html(-title=>"Free Baseball Simulator - Lineup",-style=>{'src'=>'/style.css'});
print h1({-align=>"center"},"Lineup");

#####CHECK USER PERMISSION#####
my $EDIT = 0;
if($session){
	my $user_id = $session->param('user');
	my $stmt = $dbh->prepare('select owner_id from t_team where active=1 and team_id=?');
	if(!$stmt){
		print db_error();
		exit(1);
	}
	if(!$stmt->execute($TEAM_ID)){
		print db_error();
		exit(1);
	}
	my @row = $stmt->fetchrow_array;
	if($row[0] == $user_id){
		$EDIT = 1;
	}
}

#####VALIDATE FORM#####
if($EDIT && defined($SUBMIT)){
	if(!validate()){
		print "Your lineup has errors" . p;
	}
}

#####LINKS#####
print div_start("links");
print main_links($session);
print div_end();

#####LINEUP OPTIONS#####
print div_start("content");
my %hidden = ($PNAME_HAND=>$HAND);
print 'Select a team: ' , team_popup_form($dbh,1,$TEAM_ID,\%hidden) . p;
print a({href=>url(-relative=>1) . "?$PNAME_TEAM=$TEAM_ID&$PNAME_HAND=r"},"Lineup vs RHP") . " ";
print a({href=>url(-relative=>1) . "?$PNAME_TEAM=$TEAM_ID&$PNAME_HAND=l"},"Lineup vs LHP");
print p;

#####LINEUP#####
my $query = 'select last_name,' . $hand_skills . 'intelligence,spd,' .
't_lineup.position,bat_order,t_lineup.player_id,' .
't_player.position as player_position from t_lineup join t_player on ' .
't_lineup.player_id=t_player.player_id join t_skill_hitter on ' .
't_lineup.player_id=t_skill_hitter.player_id where ' .
't_lineup.team_id=? and t_lineup.hand=? and t_player.position>0 ' .
'order by (bat_order+9) % 10';

if($EDIT){
	print start_form(-method=>"POST",-action=>url(-relative=>1)."?$PNAME_TEAM=$TEAM_ID&$PNAME_HAND=$HAND");
}
print start_table;
print Tr th ['Name','Con','Pow','Intl','Spd','Pos','Lineup','Order'];

my $stmt = $dbh->prepare($query);
if($stmt){
if($stmt->execute($TEAM_ID,$hand_int)){
while(my @row = $stmt->fetchrow_array){
	print Tr td [$row[0], $row[1], $row[2], $row[3], $row[4],
	int_to_pos($row[8]),
	$EDIT?lineup_pos_popup_menu($row[7], $row[5], $row[8]):int_to_roster_pos($row[5]),
	$EDIT?bat_order_popup_menu($row[7], $row[6]):($row[6]==0?'Bench':$row[6])
	];
}
}
}
print end_table;
if($EDIT){
	print submit(-name=>'submit',-value=>'submit');
}
print end_form;
print div_end();
print end_html;

sub validate{

	my %offense;
	my %defense;
	my $lineup = 0;
	my $fielders = 0;
	my @params = param();
	my $stmt = $dbh->prepare('Update t_lineup set bat_order=?,position=?' . 
		'where player_id=? and team_id=? and hand=?');
	if(!$stmt){
		return 0;
	}
	foreach(@params){
		#Get the batting order.
		if($_ =~ /^B([0-9]+)$/){
			my $id=$1;
			my $bat_order=param($_);
			if($bat_order < 1 || $bat_order > 9){
				if($bat_order == 0){
					$offense{$id} = $bat_order;
				}
				next;
			}
			my $bat_bit = (1 << ($bat_order-1));
			if($lineup & $bat_bit){
				print "2 players are hitting in the $bat_order spot." . p;
				return 0;
			}
			$lineup |= $bat_bit;
			$offense{$id} = $bat_order;
		}
		elsif($_ =~ /^P([0-9]+)$/){
			my $id=$1;
			my $pos=param($_);
			if($pos < 1 || $pos > 9){
				if($pos == 0){
					$defense{$id} = $pos;
				}
				next;
			}
			my $pos_bit = (1 << ($pos-1));
			if($fielders & $pos_bit){
				print "2 players are playing " . int_to_pos($pos) . p;
				return 0;
			}
			$fielders |= $pos_bit;
			$defense{$id} = $pos;
		}
	}
	if($lineup != (1 << 9) - 1){
		print "You have empty spot in the batting order.  Error: $lineup" . p;
		return 0;
	}
	elsif($fielders != (1 << 9) - 1){
		print "You have an empty position in the field.  Error: $fielders" . p;
		return 0;
	}
	else{
		#Make sure every batter also has a position in the field.
		foreach(keys(%offense)){
			if($offense{$_} && !$defense{$_}){
				print "One of your hitters is not playing defense." . p;
				return 0;
			}
		}
		foreach(keys(%offense)){
			if(!$stmt->execute($offense{$_},$defense{$_},$_,$TEAM_ID,$hand_int)){
				return 0;
			}
		}
		return 1;
	}
}
