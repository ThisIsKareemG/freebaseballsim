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


use lib '../src';

use strict;
use DBI;
use Getopt::Long;
#FBS modules
use Player;
use Hitter_Stat;
use Data_Interface;
use FBS_Config;
if($FBS_Config::CGI){
	require CGI;
	import CGI qw(:standard *table);
}

my $PRINT_NOTHING = 0;
my $PRINT_TO_STDOUT = 1;
my $PRINT_TO_FILE = 2;
my $PRINT_TO_DB = 3;
my $DB_TYPE = 'sqlite';
my $LOG_SQL = 0;
my $SAVE_BOXSCORE = 0;
my $EL = "\n";
my $HTML = 0;
my $CONFIG;

my $sql = 1;
my $print_box =  $PRINT_NOTHING;
my $print_log = $PRINT_NOTHING;

my $game_log_str;

my $dbh;

GetOptions(
'no-sql' => sub {$sql=0},
'stdout' => sub {$print_box = $PRINT_TO_STDOUT; $print_log = $PRINT_TO_STDOUT},
'log-sql' => \$LOG_SQL,
'save-boxscore' => sub {$print_box = $PRINT_TO_DB; $print_log = $PRINT_TO_DB; $SAVE_BOXSCORE=1;},
'html' => sub {
	if($FBS_Config::CGI){
		$EL="<br>"; $HTML=1;
	}else{
		print STDERR "perl module CGI not found html output disabled.\n";
	}
},
'db=s' => \$DB_TYPE,
'config=s' => \$CONFIG
);

sub print_l{
	if($print_log == $PRINT_TO_STDOUT){
		print $_[0];
	}
	elsif($print_log = $PRINT_TO_DB){
		$game_log_str .= $_[0];
	}
}

my $debug=0;

our $BASE_STOLEN = 0;

our $OUT = 0;
our $HIT = 1;
our $SAC = 2;

our $AWAY = 0;
our $HOME = 1;
our $RIGHT = 'R';
our $LEFT ='L';

our %roster;
our %players;

my $half_inning ;
my $bat;
my @score;
my $winner;
my $loser;
my @reliever;

my $HOME_ID;
my $AWAY_ID;
my $GAME_NUM;
my $DATE;

#DB Statements
my $p_stmt;
my $h_stmt;
my $f_stmt;

while(<STDIN>){
	if($debug){
		print STDERR "$_\n";
	}
	my $first_char = substr($_,0,1);
	if($_ =~ /^HOME\=([0-9]+)$/){
		$HOME_ID = $1;
	}
	elsif($_ =~ /^AWAY\=([0-9]+)$/){
		$AWAY_ID = $1;
	}
	elsif($_ =~ /^DATE\=(.+)/){
		$DATE = $1;
	}
	elsif($_ =~ /^GAME\=([0-9]+)$/){
		$GAME_NUM = $1;
	}
	elsif($first_char eq '`'){
		$bat = !$bat;
	}
	elsif($first_char eq '^' or $first_char eq '*' or $first_char eq ':' or $first_char eq ';' ){
		my @temp = split(/=/,substr($_,1,-1));
		my $player_id = $temp[0];
		my $player_name = $temp[1];
		my $position = $temp[2];
		my $bat_order = $temp[3];
		#CHECKING IF THE STAT EXISTS ALLOWS US TO DO BATCH PROCESSING.
		if(!exists($players{$player_id})){
			$players{$player_id} = Player->new($player_id);
		}
		$players{$player_id}->{name}=$player_name;
		$players{$player_id}->{pos}=$position;

		if($first_char eq '^'){
			$players{$player_id}->{team} = $bat==$HOME?$HOME_ID:$AWAY_ID;
			if($bat_order != 0){
				$roster{$bat}{$bat_order}=[$player_id]; 
			}
			if($temp[2] == 1){
				push (@{$roster{$bat}{0}},$temp[0]);
			}
		}
		
		if($first_char eq '*'){
			print_l $player_name.' replaces ';
			my $old_player_id = get_player_id($bat_order,get_defense_team());
			$players{$player_id}->{team} = $players{$old_player_id}->{team};
			print_l $players{$old_player_id}->{name};
			print_l ' at ';
			print_position($position);
			print_l ".$EL";
			push (@{$roster{get_defense_team()}{$temp[3]}},$temp[0]);
			if($temp[2] == 1 && $bat_order!= 0){
				push (@{$roster{get_defense_team()}{0}},$temp[0]);
				#Determine if starter is in-line for the win.
				if(!$reliever[get_defense_team()]
				&& $players{$old_player_id}->OUTS >=15
				&& $score[get_defense_team()][0] > $score[!get_defense_team()][0]){
					set_winner($old_player_id);
				}
				#Record that a relief pitcher is pitching.  This helps us figure
				#out the winning pitcher.
				$reliever[get_defense_team()] = 1;
			}
		}
		if($first_char eq ';'){
			print_l $temp[1].' is pinch running for ';
			my $old_player_id = get_player_id($temp[3],$bat);
			$players{$player_id}->{team} = $players{$old_player_id}->{team};
			print_l $players{$old_player_id}->{name};
			print_l ".$EL";

			push(@{$roster{$bat}{$temp[3]}},$temp[0]);
		}
		if($first_char eq ':'){
			print_l $temp[1].' is pinch hitting for ';
			my $old_player_id = get_player_id($temp[3],$bat);
			$players{$player_id}->{team} = ($bat==$HOME?$HOME_ID:$AWAY_ID);	
			print_l $players{$old_player_id}->{name};
			print_l ".$EL";
			push(@{$roster{$bat}{$temp[3]}},$temp[0]);
		}
	}
	elsif($first_char eq '~'){
	
		$half_inning++;
		$bat = whos_up();
		if($bat == $AWAY){
			print_l 'Top ';
		}
		else{
			print_l 'Bottom ';
		}
		push(@{$score[$bat]},0);
		print_l "of Inning ".get_inning().$EL;
	}
	#We have an AB.
	elsif($_ =~ /^#([0-9]+)([RL])\>([0-9]+)([RL])\>([A-Z0-9])\>*(.*)$/){
		my $pitcher = $1;
		my $p_hand = $2;
		my $hitter = $3;
		my $h_hand = $4;
		my $result = $5;
		my $fielders = $6;
		if(!defined($roster{get_defense_team()}{0}) ){
			print_l "$players{$pitcher}->{name} is pitching.$EL";
			$roster{get_defense_team()}{0}=[$pitcher];
		}
		
		print_l "$players{$hitter}->{name} ";
		record_stats($pitcher,$p_hand,$hitter,$h_hand,$result,$fielders);
	}
	elsif($first_char eq '['){
		my @temp;
		if(index($_,'+') >-1){
			@temp = split(/\+/,substr($_,1,-1));
			print_l "$players{$temp[0]}->{name} advances to ";
		}
		else{
			@temp = split(/-/,substr($_,1,-1));
			print_l "$players{$temp[0]}->{name} is out at ";
		}
		print_base($temp[1]);
		print_l ".$EL";
	}
	elsif($first_char eq '@'){
		my @temp = split(/>/,substr($_,1,-1));
		my $player = $temp[0];
		my $pitcher = substr($temp[1],0,-1);
		my $earned = substr($temp[1],-1,1);
		print_l $players{$player}->{name}." scores.$EL";
		$players{$player}->{os}{$RIGHT}->{R}++;
		$players{$pitcher}->{ds}{$RIGHT}->{R}++;
		if($earned eq '.'){
			$players{$pitcher}->{ds}{$RIGHT}->{ER}++;
		}
		#Add run to score.
		add_run();
		

	}
	elsif($first_char eq '%'){
		my($player_id,$num_rbi) = split(/>/,substr($_,1,-1));
		$players{$player_id}->{os}{$RIGHT}->{RBI}+=$num_rbi;
		
	}
	elsif($first_char eq '!'){
		my @temp = split(/>/,substr($_,1,-1));
		print_l "$players{$temp[1]}->{name} steals ";
		print_base($temp[2]);	
		print_l ".$EL";
	}
	elsif($first_char eq '?'){
		my @temp = split(/>/,substr($_,1,-1));
		print_l "$players{$temp[1]}->{name} is out stealing ";
		print_base($temp[2]);
		print_l ".$EL";
		my $pitcher = substr($temp[0],0,-1);
		my $hitter_hand = substr($temp[0],-1,1);
		$players{$pitcher}->{ds}{$hitter_hand}->{OUTS}++;
	}
	elsif($first_char eq '/'){
		print_l 'Wild pitch.'.$EL;
	}
	elsif($first_char eq '}'){
		my @temp = split(/>/,substr($_,1,-1));
		print_l "$players{$temp[0]}->{name} is now playing ";
		print_position($temp[1]);
		$players{$temp[0]}->{pos} = $temp[1];
		print_l ".$EL";
	}
	#Game is starting.
	elsif($first_char eq '+'){
		#Init variables.
		#-1 means the game has not started.  This will be incremented to 0
		#before the away team starts bating.
		$half_inning = -1;
		$bat = $AWAY;
		@score=([0], [0]);
		$winner=0;
		$loser=0;
		@reliever = (0,0);
	}
	#Game is over
	elsif($first_char eq '='){
		
	}
	else{
		#THIS SHOULD GO TO STDERR
		print $_;
	}
}
if($print_box == $PRINT_TO_STDOUT){
	print fullbox_str();
}
if($sql){
	$dbh = Data_Interface::connect($DB_TYPE, $CONFIG);
	if($LOG_SQL){
		open(SQLLOG,'>',"gamelog.log") or die("Can't open sql log file: $!");
	}
	if($DB_TYPE eq "sqlite"){
		$dbh->do("BEGIN Transaction;");
	}
	save_stats();
	record_results();
	if($SAVE_BOXSCORE){
		my $box_str = fullbox_str();
		$dbh->do(
			"Insert into t_boxscore (home_id,away_id,game,boxscore,playbyplay)
			values($HOME_ID,$AWAY_ID,$GAME_NUM,'$box_str','$game_log_str')"
		) or die();
	}
	if($DB_TYPE eq "sqlite"){
		$dbh->do("COMMIT;");
	}
	if($LOG_SQL){
		close(SQLLOG);
	}
}	
exit(0);
sub save_stats{
	
	#Determine if it is a complete game.
	if(!$reliever[$HOME]){
		#TODO:Record complete game / shutout.
		if($score[$HOME][0] > $score[$AWAY][0]){
			set_winner($roster{$HOME}{0}[-1]);
		}
	}
	if(!$reliever[$AWAY]){
		#TODO:Record complete game / shutout.
		if($score[$AWAY][0] > $score[$HOME][0] ){
			set_winner($roster{$AWAY}{0}[-1]);
		}
	}
	
	if($players{$winner}){
		$players{$winner}->{ds}{$RIGHT}->{W}++;
	}
	if($players{$loser}){
		$players{$loser}->{ds}{$RIGHT}->{L}++;
	}
	my $year = select_year($HOME_ID);
	foreach my $id (keys %players){
		my $team_id = $players{$id}->{team};
		save_hitter_stat($players{$id},$RIGHT,$year,$team_id);
		save_hitter_stat($players{$id},$LEFT,$year,$team_id);
		save_pitcher_stat($players{$id},$RIGHT,$year,$team_id);
		save_pitcher_stat($players{$id},$LEFT,$year,$team_id);
		save_fielder_stat($players{$id});
	}
}

sub save_hitter_stat{
	my $player = $_[0];
	my $hand = $_[1];
	my $year = $_[2];
	my $team_id = $_[3];
	my $stat = $player->{os}{$hand};
	my $hand_int = hand_to_int($hand);
	my $games = hand_to_game($hand);
	
	#Don't update stats of players who did not get a plate appearance.
	if(!$stat->{AB} && !$stat->{BB} && !$stat->{IB}){
		return;
	}
	if(!$h_stmt){
		$h_stmt = $dbh->prepare('Update t_stat_hitter set g=g+?,ab=ab+?,bb=bb+?,h=h+?,Ib=Ib+?,Zb=Zb+?,Eb=Eb+?,hr=hr+?,r=r+?,rbi=rbi+?,hbp=hbp+?,so=so+? where player_id=? AND team_id=? AND year=? AND hand=?') or die($h_stmt->Statement);
	}
	$h_stmt->execute($games,$stat->{AB},$stat->{BB},$stat->{H},$stat->{IB},$stat->{ZB},$stat->{EB},$stat->{HR},$stat->{R},$stat->{RBI},0,$stat->{SO},$player->{id},$team_id,$year,$hand_int) or die($h_stmt->Statement);
	if($h_stmt->rows == 0){
		print STDERR 'No rows updated by query ',$h_stmt->Statement,"\n";
	}
	if($LOG_SQL){
		print SQLLOG $h_stmt->Statement,"\n";	
	}
}
	

sub save_pitcher_stat{
	
	my $player = $_[0];
	my $hand = $_[1];
	my $year = $_[2];
	my $team_id = $_[3];
	my $stat = $player->{ds}{$hand};
	my $hand_int = hand_to_int($hand);
	my $game = hand_to_game($hand);
		
	#Don't update stats of players who did not pitch.
	if(!$stat->{OUTS} && !$stat->{BB} && !$stat->{H}){
		return;
	}
	if(!$p_stmt){
		$p_stmt = $dbh->prepare('update t_stat_pitcher set g=g+?,outs=outs+?,bb=bb+?,h=h+?,r=r+?,er=er+?,hbp=hbp+?,so=so+?,w=w+?,l=l+?,sv=sv+? where player_id=? AND team_id=? AND year=? AND hand=?') or die(p_stmt->Statement);
	}
	$p_stmt->execute($game,$stat->{OUTS},$stat->{BB},$stat->{H},$stat->{R},$stat->{ER},0,$stat->{SO},$stat->{W},$stat->{L},$stat->{SV},$player->{id},$team_id,$year,$hand_int) or die($p_stmt->Statement);
	if($LOG_SQL){
		print SQLLOG $p_stmt->Statement,"\n";
	}
}

sub save_fielder_stat{
	
	my $player = $_[0];
	my $stat = $player->{ds}{$RIGHT};
	my $query = "
		Update t_stat_fielder set a=a+%d,po=po+%d,e=e+%d,dp=dp+%d,
		good=good+%d,bad=bad+%d,cs=cs+%d,sb=sb+%d WHERE player_id=%d AND 
		pos=%d and team_id=%d and year=%d;
	";
	my $DB = 0;
	my $CS = 0;
	my $SB = 0;
	my $POS = 0;
	my $team_id = 0;
	my $year = 0;

	if(!$f_stmt){
		$f_stmt = $dbh->prepare('Update t_stat_fielder set a=a+?,po=po+?,e=e+?,dp=dp+?,good=good+?,bad=bad+?,cs=cs+?,sb=sb+? WHERE player_id=? AND pos=? and team_id=? and year=?') or die($f_stmt->Statement);
	}
	$f_stmt->execute($stat->{A},$stat->{PO},$stat->{E},$DB,$stat->{GP},$stat->{BP},$CS,$SB,$player->{id},$POS,$team_id,$year) or die($f_stmt->Statement);
}

sub hand_to_game{

	if($_[0] eq $RIGHT){
		return 1;
	}
	else{
		return 0;
	}
}	

sub hand_to_int{

	if($_[0] eq $LEFT){
		return 1;
	}
	else{
		return 0;
	}
}

sub print_base{

	if($_[0] eq '1'){
		print_l 'first';
	}
	elsif($_[0] eq '2'){
		print_l 'second';
	}
	elsif($_[0] eq '3'){
		print_l 'third';
	}
	elsif($_[0] eq '4'){
		print_l 'home';
	}
}

sub whos_up{

	return $half_inning%2;
}



sub print_position {

	if($_[0] == 1){
		print_l 'pitcher';
	}
	elsif($_[0] == 2){
		print_l 'catcher';
	}
	elsif($_[0] == 3){
		print_l 'fist base';
	}
	elsif($_[0] == 4){
		print_l 'second base';
	}
	elsif($_[0] == 5){
		print_l 'third base';
	}
	elsif($_[0] == 6){
		print_l 'short stop';
	}
	elsif($_[0] == 7){
		print_l 'left field';
	}
	elsif($_[0] == 8){
		print_l 'center field';
	}
	elsif($_[0] == 9){
		print_l 'right field';
	}
}

sub record_stats{

	my $pitcher = $_[0];
	my $p_hand = $_[1];
	my $hitter = $_[2];
	my $h_hand = $_[3];
	my $result = $_[4];
	my $defense = $_[5];
	my $result_category = -1;
	
	if($result eq '4'){
		$players{$hitter}->{os}{$p_hand}->{HR}++;
		$players{$hitter}->{os}{$RIGHT}->{R}++;
		$players{$pitcher}->{ds}{$RIGHT}->{R}++;
		$players{$pitcher}->{ds}{$RIGHT}->{ER}++;
		$result_category = $HIT;
		print_l 'hits a home run';
		add_run();
	}
	else{

		my @fielders = split(/-/,$defense);
		my $fielder_bonus = substr($fielders[0],-1,1);
		if($fielder_bonus eq '_' || $fielder_bonus eq '+'){
			$fielders[0] = substr($fielders[0],0,-1);
		}

		if($result eq 'G'){
			$result_category = $OUT;
			print_l 'grounds out to ';
			print_position($players{$fielders[0]}->{pos});
		}
		elsif($result eq 'K'){
			record_strikeout($hitter,$h_hand,$pitcher,$p_hand);
			$result_category = $OUT;	
			print_l 'strikes out swinging';
		}
		elsif($result eq 'F'){
			$result_category = $OUT;
			print_l 'flies out to ';
			print_position($players{$fielders[0]}->{pos});	
		}
		elsif($result eq 'W'){		
			$players{$pitcher}->{ds}{$h_hand}->{BB}++;
			$players{$hitter}->{os}{$p_hand}->{BB}++;
			print_l 'walks';
		}
		elsif($result eq 'L'){	
			record_strikeout($hitter,$h_hand,$pitcher,$p_hand);
			$result_category = $OUT;
			print_l 'stikes out looking';
		}
		elsif($result eq '1'){
			$result_category = $HIT;
			$players{$hitter}->{os}{$p_hand}->{IB}++;
			print_l 'singles to ';
			print_position($players{$fielders[0]}->{pos});		
		}
		elsif($result eq '2'){
			$result_category = $HIT;
			$players{$hitter}->{os}{$p_hand}->{ZB}++;
			print_l 'doubles to ';
			print_position($players{$fielders[0]}->{pos});
		}
		elsif($result eq '3'){
			$result_category = $HIT;
			$players{$hitter}->{os}{$p_hand}->{EB}++;
			print_l 'triples to ';
			print_position($players{$fielders[0]}->{pos});
		}
		elsif($result eq 'C'){
			$result_category = $OUT;
			print_l 'grounds into a fielder\'s choice to ';
			print_position($players{$fielders[0]}->{pos});
		}
		elsif($result eq 'P'){
			$result_category = $OUT;
			print_l 'pops out to ';
			print_position($players{$fielders[0]}->{pos});
		}
		elsif($result eq 'S'){
			$result_category = $SAC;
			print_l 'is out on a sacrifice to ';
			print_position($players{$fielders[0]}->{pos});
		}
		elsif($result eq 'E'){
			print_l 'reaches on an error by ';
			print_position($players{$fielders[0]}->{pos});
			$players{$fielders[0]}->{ds}{$RIGHT}->{E}++;
		}
		elsif($result eq 'D'){
			print_l 'grounds into a double play to ';
			print_position($players{$fielders[0]}->{pos});
			$result_category = $OUT;
			#We only add one out here becuase another will be added later.
			$players{$pitcher}->{ds}{$h_hand}->{OUTS}++;
			$players{$fielders[-2]}->{ds}{$RIGHT}->{PO}++;
		}
		else{
			#THIS SHOULD GO TO STDERR
			print "$result";
		}
		record_field_stats($result_category,\@fielders,$fielder_bonus);
	}
	print_l ".$EL";
	if($result_category == $OUT or $result_category == $SAC){
		$players{$pitcher}->{ds}{$h_hand}->{OUTS}++;
	}
	if($result_category == $HIT){
		$players{$pitcher}->{ds}{$h_hand}->{H}++;
		$players{$hitter}->{os}{$p_hand}->{H}++;	
	}
	if(not $result eq 'W' and not $result eq 'S'){
		$players{$hitter}->{os}{$p_hand}->{AB}++;
	}
	
}

sub record_field_stats{

	my $result = $_[0];
	my $fielder_ref = $_[1];
	my @fielders = @{$fielder_ref};
	my $fielder_bonus = $_[2];
	if($result == $OUT){
		$players{$fielders[-1]}->{ds}{$RIGHT}->{PO}++;
		if($#fielders > 0){
			for(my $i=0;$i<$#fielders;$i++){
				my $fielder = $fielders[$i];
				$players{$fielder}->{ds}{$RIGHT}->{A}++;
			}
		}
	}
	if($fielder_bonus eq '+'){
		$players{$fielders[0]}->{ds}{$RIGHT}->{GP}++;
	}
	if($fielder_bonus eq '_'){
		$players{$fielders[0]}->{ds}{$RIGHT}->{BP}++;
	}
}

sub record_strikeout{

	my ($hitter,$h_hand,$pitcher,$p_hand) = @_;
	$players{$pitcher}->{ds}{$h_hand}->{SO}++;
	$players{$hitter}->{os}{$p_hand}->{SO}++; 
}

sub score_str{

	my $i;
	my $ret= "AWAY ";
	for($i=1;$i<=get_inning();$i++){
		$ret.= $score[$AWAY][$i]." ";
	}
	$ret.= "| " . $score[$AWAY][0] . $EL;

	$ret.= "HOME ";
	foreach($i=1;$i<=get_inning();$i++){
		$ret.= (defined($score[$HOME][$i])?$score[$HOME][$i]:"x");
		$ret.=" ";
	}
	$ret.= "| " . $score[$HOME][0] . $EL;

	return $ret;
}

sub boxscore_str{

	my $team = $_[0];
	my $h_header = 'AB R H RBI BB SO PO A';
	my $ret;
	if($HTML){
		$ret.= start_table();
		my @table_h = ("Name");
		push(@table_h,split(/ /,$h_header));

		$ret.= Tr({-align=>"left"},th(\@table_h));
	}
	else{
		$ret.= spaces_str(15);
		$ret.= $h_header;
		$ret.= $EL;
	}
	my $n=0;
	for(my $i =1;$i<=9;$i++){
		foreach(@{$roster{$team}{$i}}){
			my $boxline = boxline_str($_,$n);
			if(!$HTML){
				#Indent for subs.
				$ret.= spaces_str($n);
			}
			$ret.= $boxline;
			if($n==0){
				$n++;
			}		
		}
		$n=0
	}
	if($HTML){
		$ret.= end_table();
		$ret.= start_table();
		$ret.= Tr({-align=>"left"},th(['Name','IP','H','ER','R','SO','BB']));
	}
	else{
		$ret.= $EL;
		$ret.= spaces_str(15);
		$ret.= 'IP  H ER R SO BB';
		$ret.= $EL;
	}
	foreach(@{$roster{$team}{0}}){
		$ret.= pitcher_stats_str($_);
	}
	if($HTML){
		$ret.= end_table();
	}
	else{
		$ret.= $EL;
	}

	return $ret;
}
sub boxline_str{
	
	my $player = $_[0];
	my $spaces = $_[1];
	my $AB = $players{$player}->{os}{$RIGHT}->{AB} + $players{$player}->{os}{$LEFT}->{AB};
	my $R = $players{$player}->{os}{$RIGHT}->{R} + $players{$player}->{os}{$LEFT}->{R};
	my $H = $players{$player}->{os}{$RIGHT}->{H} + $players{$player}->{os}{$LEFT}->{H};
	my $RBI	= $players{$player}->{os}{$RIGHT}->{RBI} + $players{$player}->{os}{$LEFT}->{RBI};
	my $BB = $players{$player}->{os}{$RIGHT}->{BB} + $players{$player}->{os}{$LEFT}->{BB};
	my $SO = $players{$player}->{os}{$RIGHT}->{SO} + $players{$player}->{os}{$LEFT}->{SO};
	my $PO = $players{$player}->{ds}{$RIGHT}->{PO};
	my $A = $players{$player}->{ds}{$RIGHT}->{A};

	my $ret;
	my $name = $players{$player}->{name};
	if($HTML){
		$ret .= Tr(td([$name,$AB,$R,$H,$RBI,$BB,$SO,$PO,$A]));
	}
	else{
		$ret.= $name; 
		$ret.= spaces_str(15 - $spaces - length($players{$player}->{name}));
		$ret.= "$AB  $R $H $RBI   $BB  $SO  $PO  $A";
	
		$ret.= "$EL";
	}
	return $ret;
}

sub pitcher_stats_str{

	my $player = $_[0];
	my $outs = $players{$player}->{ds}{$RIGHT}->{OUTS} + $players{$player}->{ds}{$LEFT}->{OUTS};
	my $H = $players{$player}->{ds}{$RIGHT}->{H} + $players{$player}->{ds}{$LEFT}->{H};
	my $BB = $players{$player}->{ds}{$RIGHT}->{BB} + $players{$player}->{ds}{$LEFT}->{BB};
	my $R = $players{$player}->{ds}{$RIGHT}->{R};
	my $ER = $players{$player}->{ds}{$RIGHT}->{ER};
	my $SO = $players{$player}->{ds}{$RIGHT}->{SO} + $players{$player}->{ds}{$LEFT}->{SO};
	my $innings = int($outs/3);
	my $remainder = $outs % 3;
	my $ret;
	my $name = $players{$player}->{name};
	if($HTML){
		$ret.= Tr(td([$name,"$innings.$remainder",$H,$ER,$R,$SO,$BB]));
	}
	else{
		$ret.= $name;
		$ret.= spaces_str(15- length($players{$player}->{name}));
		$ret.= "$innings.$remainder $H $ER  $R $SO  $BB";
		$ret.= "$EL";
	}
	return $ret;
}
sub spaces_str{

	my $space;
	if($HTML){
		$space = '&nbsp;'
	}
	else{
		$space = ' ';
	}
	my $ret = '';
	for(my $i=0;$i<$_[0];$i++){
		$ret .= $space;
	}
	return $ret;
}

sub get_defense_team{

	if($bat == $HOME){
		return $AWAY;
	}
	else{
		return $HOME;
	}
}
#TODO: Change this function to return player_id
sub get_player_id{

	my $bat_order = $_[0];
	my $team = $_[1];
#TODO: pass in positon as a third argument
	my $position = $_[2];
	my $last_index = $#{$roster{$team}{$bat_order}};
	if($last_index > -1){
		my %bat_hash = %{$roster{$team}};
		my @bat_array = @{$bat_hash{$bat_order}};
		my $ply_id = $bat_array[$last_index];
		return $ply_id;
	}
	else{
#There is no player listed at this spot in the batting order, so we need to search by positon.
		foreach(keys(%players)){
			if($players{$_}->{pos} == $position){
				return $_;
			}
		}
		print "ERROR: No player_id found for team: $team, order: $bat_order, position: $position\n";
		return "ERROR";
	}
}

sub select_year{

	my $team_id = $_[0];
	my $query = "Select t_league.league_date from t_team join t_league on t_team.league_id=t_league.league_id where team_id = $team_id";
	my $stmnt = $dbh->prepare($query) or die($query);
	$stmnt->execute or die ($stmnt->errstr);
	if($LOG_SQL){
		print SQLLOG $query . "\n";
	}
	my @data = $stmnt->fetchrow_array;
	$data[0] = Data_Interface::date_to_year($DB_TYPE,$data[0]);
	return $data[0];
}

sub get_inning{
	return int($half_inning/2) + 1;
}

sub add_run{
	
	$score[$bat][get_inning()]++;
	$score[$bat][0]++;

	#Determine the current winning/losing pitcher.
	my $home_pitcher = $roster{$HOME}{0}[-1];
	my $away_pitcher = $roster{$AWAY}{0}[-1];

	#We will adjust this w/l stats in this function.  This is a lot of
	#extra work, and it would probably be more efficient to wait
	#until the end of the game to adjust the stats, but doing it here
	#makes it easier when simulating a whole season.
	if($score[$HOME][0] == $score[$AWAY][0]){
		set_winner(0);
		set_loser(0);
	}
	#Home team is winning.
	elsif($score[$HOME][0] > $score[$AWAY][0]){
		set_loser($away_pitcher);
		if(!$winner && $reliever[$HOME]){
			set_winner($home_pitcher);
		}
	}
	#Away team is winning.
	else{
		set_loser($home_pitcher);
		if(!$winner &&  $reliever[$AWAY]){
			set_winner($away_pitcher);
		}
	}
}

sub set_winner{
	my $id = $_[0];
	$winner=$id;
}

sub set_loser{
	my $id = $_[0];
	$loser=$id;
}

sub record_results{
	my $w_team;
	my $l_team;
	if($score[$HOME][0] > $score[$AWAY][0]){
		$w_team =  $HOME_ID;
		$l_team = $AWAY_ID;
	}
	else{
		$w_team = $AWAY_ID;
		$l_team = $HOME_ID;
	}
	if($w_team){
		$dbh->do("update t_team set wins=wins+1 where team_id=$w_team and active=1");
	}
	if($l_team){
		$dbh->do("update t_team set loses=loses+1 where team_id=$l_team and active=1");
	}
}

sub fullbox_str{
return score_str() .
"W: " . $players{$winner}->{name} . " L: " . $players{$loser}->{name} . $EL .
boxscore_str($AWAY) .
boxscore_str($HOME);
}
