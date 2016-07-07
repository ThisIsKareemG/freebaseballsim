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
use Data_Interface;
use FBS_Config;
if($FBS_Config::STATDESC){
	require Statistics::Descriptive;
}

my $NUM_SEASONS = $ARGV[0];
my $REGRESSION_TEST = 0;
my $LTRACE = 0;
my $DB_TYPE = 'sqlite';
my $REGRESSION_SEED = 1;
my $PATH_TO_SCRIPTS = "./";
my $LEAGUE_ID = 1;
my $EXTRA_DATA = 0;
my $SLOW = 0;
my $HTML = 0;
my $COMPARE = 1;

GetOptions(
'regression' => \$REGRESSION_TEST,
'ltrace' => sub{$LTRACE =1; $REGRESSION_TEST =1;},
'db=s' => \$DB_TYPE,
'extra-data' => sub{
	if($FBS_Config::STATDESC){
		$EXTRA_DATA=1;
	}
	else{
		print STDERR 'perl module Statistics::Descriptive not installed' ,
			' disabling extra-data.', "\n";
	}
},
'slow' => \$SLOW,
'html' => \$HTML,
'no-compare' => sub{$COMPARE = 0;}
);

my $dbh_sim = Data_Interface::connect($DB_TYPE);

my @memory_tables;
# @memory_tables = ('t_stat_pitcher','t_stat_hitter','t_stat_fielder','t_skill_pitcher','t_skill_hitter','t_lineup','t_rotation','t_schedule','t_player','t_team','t_league');
 
foreach(@memory_tables){
	change_storage_engine($dbh_sim,$_,'memory');
}

my $AB_SIM = 'ab';
my $BB_SIM = 'bb';
my $H_SIM = 'h';
my $IB_SIM = 'Ib';
my $ZB_SIM = 'Zb';
my $EB_SIM = 'Eb';
my $HR_SIM = 'hr';
my $R_SIM = 'r';
my $RBI_SIM = 'rbi';
my $HBP_SIM = 'hbp';
my $SO_SIM = 'so';

my $avg_query_sim = "select sum(ab)+sum(bb)+sum(hbp), sum(h),sum($ZB_SIM), sum($EB_SIM),sum(hr), sum(bb), sum(so) from t_stat_hitter";

my $hr_query_sim = "select count(*) from (select sum(hr) from t_stat_hitter group by player_id having sum(hr)>=30) t_hr";

my @hitter_mean_array = (50);
my @hitter_sd_array = (22.5);
my @pitcher_mean_array = (50);
my @pitcher_sd_array = (22.5);
my @hitter_modifier_array = (1.0);
my @pitcher_modifier_array = (1.0);
my @ab_exponent_array = (1.0);

my $test_run_id = time();
print "TEST RUN: $test_run_id\n";
print "League ID: $LEAGUE_ID\n";
#Remove old data from database.
$dbh_sim->do("DELETE FROM t_player;");
$dbh_sim->do("DELETE FROM t_skill_fielder;");
$dbh_sim->do("DELETE FROM t_skill_hitter;");
$dbh_sim->do("DELETE FROM t_skill_pitcher;");
$dbh_sim->do("DELETE FROM t_stat_fielder;");
$dbh_sim->do("DELETE FROM t_stat_hitter;");
$dbh_sim->do("DELETE FROM t_stat_pitcher;");
$dbh_sim->do("DELETE FROM t_lineup;");
$dbh_sim->do("DELETE FROM t_rotation;");
$dbh_sim->do("DELETE FROM t_draft_pref;");
$dbh_sim->do("DELETE FROM t_draft;");

foreach(@hitter_mean_array){
	my $hitter_mean = $_;
	
	foreach(@hitter_sd_array){
		my $hitter_sd = $_;
		
		foreach(@pitcher_mean_array){
			my $pitcher_mean = $_;
				
			foreach(@pitcher_sd_array){
				my $pitcher_sd = $_;
				
				foreach(@hitter_modifier_array){
					my $hitter_modifier = $_;
					
					foreach(@pitcher_modifier_array){
						my $pitcher_modifier = $_;
						
						foreach(@ab_exponent_array){
							my $ab_exponent = $_;
							my $i;
							print "Seasons: $NUM_SEASONS\nhitter_mean: $hitter_mean\nhitter_sd: $hitter_sd\npitcher_mean: $pitcher_mean\npitcher_sd: $pitcher_sd\n";
							print "Hitter: $hitter_modifier Pitcher: $pitcher_modifier Exp: $ab_exponent\n";
							#TODO: Print number of players here.
							for($i=0;$i<$NUM_SEASONS;$i++){
								if($REGRESSION_TEST){
									$REGRESSION_SEED += $i;
								}	
								print STDERR "*Season: $i\n";
								my @restart_league_args = ("perl",$PATH_TO_SCRIPTS."restart_league.pl","--hitter-mean=$hitter_mean","--hitter-sd=$hitter_sd","--pitcher-mean=$pitcher_mean","--pitcher-sd=$pitcher_sd","--league-id=$LEAGUE_ID","--db=$DB_TYPE");
								if($REGRESSION_TEST){
									push(@restart_league_args,"--seed=$REGRESSION_SEED");
								}
								system(@restart_league_args) == 0
									or kill_script();
								
								my @sim_day_args = ($PATH_TO_SCRIPTS."sim_day","-h $hitter_modifier","-p $pitcher_modifier","-x $ab_exponent","--season","-l $LEAGUE_ID");
								if($REGRESSION_TEST){
									push(@sim_day_args,'-s '.$REGRESSION_SEED);
								}
								if(!$SLOW){
									push(@sim_day_args,'--fast');
								}
								if($HTML){
									push(@sim_day_args,'--html');
								}
								if($LTRACE){
									system('ltrace','-c',@sim_day_args);
								}
								else{
									system(@sim_day_args);
								}
								if($? ==256){
									kill_script();
								}
								#print STDERR "Done\n";
								#system("perl",$PATH_TO_SCRIPTS."new_season.pl","--db=$DB_TYPE");
							}
							if(!$LTRACE && $COMPARE){
								do_compare($hitter_mean,$hitter_sd,$pitcher_mean,$pitcher_sd,$hitter_modifier,$pitcher_modifier,$ab_exponent);
							}
							
						}
					}	
				}		
			}
		}
	}
}
foreach(@memory_tables){
	change_storage_engine($dbh_sim,$_,'myisam');
}

my $where_real;

sub do_compare{

	my $dbh_real = Data_Interface::connect($DB_TYPE, undef, 1);
	
	$where_real = ' WHERE yearID >= 1970 AND yearID<1980';
	my $avg_query_real = "select sum(ab)+sum(bb)+sum(hbp), sum(h),sum(Zb), sum(Eb),sum(hr), sum(bb), sum(so) from batting".$where_real;
	my $hr_query_real = "select count(*)/10 from batting ".$where_real." AND hr>=30";
	#-------------------#
	#COMPARE PERCENTAGES#
	#-------------------#
	
	my ($hitter_mean,$hitter_sd,$pitcher_mean,$pitcher_sd,$hitter_modifier,$pitcher_modifier,$ab_exponent)=@_;


	my @row_sim = execute_query_row($dbh_sim,$avg_query_sim);
	my @row_real = execute_query_row($dbh_real,$avg_query_real);
	
	my $sim_pa = $row_sim[0];
	print_compare('PA',$row_sim[0],$row_real[0]);

	my $h_per_dif = print_compare('H',$row_sim[1]/$row_sim[0],$row_real[1]/$row_real[0]);
	my $Zb_per_dif = print_compare('2B',$row_sim[2]/$row_sim[0],$row_real[2]/$row_real[0]);
	my $Eb_per_dif = print_compare('3B',$row_sim[3]/$row_sim[0],$row_real[3]/$row_real[0]);
	my $hr_per_dif = print_compare('HR',$row_sim[4]/$row_sim[0],$row_real[4]/$row_real[0]);
	my $bb_per_dif = print_compare('BB',$row_sim[5]/$row_sim[0],$row_real[5]/$row_real[0]);
	my $so_per_dif = print_compare('SO',$row_sim[6]/$row_sim[0],$row_real[6]/$row_real[0]);
	
	#my $hr_sim = execute_query_value($dbh_sim,$hr_query_sim);
	#my $hr_real = execute_query_value($dbh_real,$hr_query_real);
	
	#print_compare('HR>=15',$hr_sim,$hr_real);
	
	#-------------------#
	#COMPARE MEAN AND SD#
	#-------------------#
	
	#HITTERS#

if($FBS_Config::STATDESC){
	my %stat_hash;	
	
	my $indvd_hit_sim_query =
		"select sum(H) as H,sum(HR) as HR,sum(ZB) as ZB,sum(EB) as EB,
		sum(BB) as BB,sum(SO) as SO,sum(AB)as AB,sum(HBP) as HBP
		from t_stat_hitter group by player_id,year having (sum(AB) 
		+ sum(BB) + sum(HBP))>=503;";
		
	#select H as H may make no sense, but some databases (sqlite, maybe others)
	#if you have a lowercase column name it can reference it with an uppercase
	#reference, but it will still retrun the column name as lowercase.
	my $indvd_hit_real_query = "select H as H,HR as HR,ZB as ZB,EB as EB,
		BB as BB,SO as SO,AB as AB,HBP as HBP,IBB as IBB,SH as SH,SF as SF
	from batting ".$where_real." AND (AB+BB+IBB+SH+SF)>=503";

	my @indvd_hit_stats_print = ('H','HR','ZB','EB','BB','SO');
	my @indvd_hit_stats = (@indvd_hit_stats_print, ('AB','HBP','IBB','SH','SF'));
	
	my %indvd_hit_data_sim = get_stats($dbh_sim,$indvd_hit_sim_query,
		\@indvd_hit_stats);
	my %indvd_hit_data_real = get_stats($dbh_real,$indvd_hit_real_query,
		\@indvd_hit_stats);
	print "Averages: All percents are stat/PA except for H which is H/AB.\n";
	foreach(@indvd_hit_stats_print){
		print_compare($_.'(MEAN)',$indvd_hit_data_sim{$_}->mean(),
			$indvd_hit_data_real{$_}->mean());
		print_compare($_.'(STD)',$indvd_hit_data_sim{$_}->standard_deviation(),
			$indvd_hit_data_real{$_}->standard_deviation());
	}
}
	###CALCULATE AVERGAGE CONTACT########
	my @pitcher_weights = execute_query_row($dbh_sim,"select sum(ctrl_sum),sum(vel_sum),sum(mov_sum) from (select sum(ctrl*(outs+bb+h)) as ctrl_sum ,sum(vel*(outs+bb+h)) as vel_sum,sum(mov*(outs+bb+h)) as mov_sum from t_skill_pitcher inner join t_stat_pitcher on t_skill_pitcher.player_id = t_stat_pitcher.player_id group by t_stat_pitcher.player_id) t_sum;    ");
	my $avg_ctrl = $pitcher_weights[0]/ $sim_pa;
	my $avg_vel = $pitcher_weights[1]/ $sim_pa;
	my $avg_mov = $pitcher_weights[2]/ $sim_pa;

if($EXTRA_DATA){
	my $con_data = Statistics::Descriptive::Sparse->new();
	my $pow_data = Statistics::Descriptive::Sparse->new();
	my $intl_data = Statistics::Descriptive::Sparse->new();
	
	get_skills_data("R",$con_data,$pow_data,$intl_data);
	get_skills_data("L",$con_data,$pow_data,$intl_data);
	
	my $avg_contact = $con_data->mean();
	my $avg_intl = $intl_data->mean();
	my $avg_pow = $pow_data->mean();
	print "con (MEAN): " . $con_data->mean() . " (STD): " . $con_data->standard_deviation() . "\n";
	print "pow (MEAN): " . $pow_data->mean() . " (STD): " . $pow_data->standard_deviation() . "\n";
	print "intl (MEAN): " . $intl_data->mean() . " (STD: " . $intl_data->standard_deviation() . "\n";
}
else{
	my $h_skill_query = "select sum(Cv%s*(AB+BB)), sum(Pv%s*(AB+BB)), sum(intelligence * (AB+BB)) from t_skill_hitter inner join t_stat_hitter on t_skill_hitter.player_id = t_stat_hitter.player_id inner join t_player on t_skill_hitter.player_id = t_player.player_id where ab>0 and hand = %d;";
	my @hvR_skill = execute_query_row($dbh_sim,sprintf($h_skill_query,'R','R',0));

	my $PA_CvR = $hvR_skill[0];
	my $PA_PvR = $hvR_skill[1];
	my $PAvR_int = $hvR_skill[2];

	my @hvL_skill = execute_query_row($dbh_sim,sprintf($h_skill_query,'L','L',1));

	my $PA_CvL = $hvL_skill[0];
	my $PA_PvL = $hvL_skill[1];
	my $PAvL_int = $hvL_skill[2];
	my $avg_contact = ($PA_CvR + $PA_CvL)  / $sim_pa;
	my $avg_intl = ($PAvR_int + $PAvL_int) / $sim_pa;
	my $avg_pow = ($PA_PvR + $PA_PvL) / $sim_pa;
	print "Hitter averages con: $avg_contact pow: $avg_pow intl: $avg_intl \n";
}
	print "Pitcher averages ctrl: $avg_ctrl vel: $avg_vel mov: $avg_mov\n";

#Prepare stats to save them in the database.

#	$stats_to_compare[1] = 'BA';
#	my @per_difs;
#	foreach(@stats_to_compare){
#		push(@per_difs,$stat_hash{$_}{'MEAN'});
#		push(@per_difs,$stat_hash{$_}{'STD'});	
#	}
	
	#-----------------#
	#Save test results#
	#-----------------#

#	my $query = 'Insert into t_test_result (test_run_id, hitter_mean,hitter_sd,pitcher_mean,pitcher_sd,hitter_modifier,pitcher_modifier,ab_exponent, h_percent, 2b_percent, 3b_percent, hr_percent, bb_percent,so_percent, hr_mean, hr_sd,ba_mean, ba_sd,   2b_mean, 2b_sd, 3b_mean, 3b_sd, bb_mean, bb_sd, so_mean, so_sd) values('.add_commas_to_list($test_run_id,$hitter_mean,$hitter_sd,$pitcher_mean,$pitcher_sd,$hitter_modifier,$pitcher_modifier,$ab_exponent,$h_per_dif,$Zb_per_dif, $Eb_per_dif,$hr_per_dif,$bb_per_dif, $so_per_dif, @per_difs).')';
#	print $query."\n";
#	my $stmnt_save = $dbh_sim->prepare($query);

#	$stmnt_save->execute;
	
}
sub get_stats{

	my $dbh = $_[0];
	my $query = $_[1];
	my @stat_list = @{$_[2]};

	my %stats;
	foreach(@stat_list){
		$stats{$_} = Statistics::Descriptive::Sparse->new();
	}

	my $stmnt = $dbh->prepare($query) or die($query);
	$stmnt->execute or die($query);
	while(my $row = $stmnt->fetchrow_hashref){
		my %row_hash = %{$row};
#		print STDERR %row_hash;
		#Some databases (sqlite) return 0 values as undefined, so we need
		#to check that the variables are defined.
		my $row_pa = $row_hash{"AB"} + 
			($row_hash{"BB"}?$row_hash{"BB"}:0) + 
			($row_hash{"HBP"}?$row_hash{"HBP"}:0) +
			($row_hash{"IBB"}?$row_hash{"IBB"}:0) + 
			($row_hash{"SH"}?$row_hash{"SH"}:0)  + 
			($row_hash{"SF"}?$row_hash{"SF"}:0);
		foreach(@stat_list){
			my $value;
#			print "$_ = $row_hash{$_}\n";
			if($_ eq "H"){
				$value = $row_hash{$_}?$row_hash{$_}/ $row_hash{"AB"}:0;
			}
			else{
				$value = $row_hash{$_}?$row_hash{$_}/$row_pa:0;
			}
#			print "Row=$_ Value=$value\n PA=$row_pa\n";
			$stats{$_}->add_data($value);
		}
	}
	return %stats;
}

sub get_skills_data{

	my $hand = $_[0];
	my $hand_int;
	if($hand eq "R"){
		$hand_int = 0;
	}
	else{
		$hand_int = 1;
	}
	my $con_data = $_[1];
	my $pow_data = $_[2];
	my $intl_data = $_[3];
	my $h_skill_query = "select Cv%s, Pv%s, intelligence, (AB+BB+HBP) from t_skill_hitter inner join t_stat_hitter on t_skill_hitter.player_id = t_stat_hitter.player_id inner join t_player on t_skill_hitter.player_id = t_player.player_id where ab>0 and hand = %d;";
	my $stmnt = $dbh_sim->prepare(sprintf($h_skill_query,$hand,$hand,$hand_int)) or die($h_skill_query);
	$stmnt->execute or die($h_skill_query);
	while(my @row = $stmnt->fetchrow_array){
		my $pa = $row[3];
		for(my $i=0;$i<$pa;$i++){
			$con_data->add_data($row[0]);
			$pow_data->add_data($row[1]);
			$intl_data->add_data($row[2]);
		}
	}
}

sub add_commas_to_list{

	my $str;
	foreach(@_){
		$str.=$_.', ';
	}
	return substr($str,0,-2);
}

sub compare_mean_sd_hitters_query_real{

	my $stat = $_[0];
	return "select avg($stat), " . stdev($stat) . " from batting ".$where_real." AND AB>503";
	
}

sub compare_mean_sd_hitters_query_sim{

	my $stat = $_[0];
	return "select avg($stat), " . stdev($stat) . " from (select sum($stat) as $stat,sum(AB) as AB from t_stat_hitter group by player_id,year having sum(AB)>503 )t_$stat";
}

sub calc_percent_dif{

	my $initial = $_[0];
	my $final = $_[1];
	if($initial == 0){
		return -1;
	}
	return ($final-$initial)/$initial;
}

sub print_compare{

my $stat = $_[0];
my $sim_value = $_[1];
my $real_value = $_[2];

my $percent_dif = calc_percent_dif($real_value,$sim_value);

print $stat.': sim-> '.$sim_value.' real-> '.$real_value.' %dif-> '.$percent_dif."\n";
return $percent_dif;
}


sub execute_query_row{
	my $dbh = $_[0];
	my $query = $_[1];
	my $stmnt = $dbh->prepare($query) or die($query);
	$stmnt->execute or die($query);
	return $stmnt->fetchrow_array;
}

sub execute_query_value{

	my $dbh = $_[0];
	my $query = $_[1];
	my @row = execute_query_row($dbh,$query);
	return $row[0];
}

sub kill_script{

	foreach(@memory_tables){
		change_storage_engine($dbh_sim,$_,'myisam');
	}
	print STDERR "**ERROR: Exiting goals.pl.\n";
	die();	
}

sub change_storage_engine{

	my $dbh = $_[0];
	my $table = $_[1];
	my $engine = $_[2];

	my $stmnt = $dbh->prepare('alter table '.$table.' type='.$engine);
	$stmnt->execute;
}

sub stdev{
	my $column = $_[0];
	if($DB_TYPE eq 'sqlite'){
		return 0;
	}
	else{
		return "std($column)";
	}
}
