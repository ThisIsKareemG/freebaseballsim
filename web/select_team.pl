#!/usr/bin/perl

use strict;
use lib '../src';
use DBI;
use CGI ':standard';

use Fbs_utils;
use Data_Interface;

#####PARAMETERS#####
my $session = get_session();
my $PNAME_TEAM = 'team';
my $PNAME_SUBMIT = 'submit';


#####BEGIN HTML#####
print header;
print start_html(-title=>'Free Baseball Simulator - Select Team',
	-style=>{'src'=>'/style.css'});
print h1({-align=>'center'},'Select a Team');

#####LINKS#####
print div_start('links');
print main_links($session);
print div_end();

#####VALIDATE CHOICE#####
my $dbh = Data_Interface::connect('mysql');
if(!$dbh){
	print db_error();
	print end_html();
	exit(1);
}
my $ERRMSG;
if(param($PNAME_SUBMIT)){
	if(validate($session)){
		print 'You have sucessfully selected a team.';
	}
	else{
		print $ERRMSG;
	}
	print end_html;
	exit(1);
}
if(!$session){
	exit_early("You need to be logged in to select a team.");
}
my $user_id = $session->param('user');
my $stmt = $dbh->prepare('select count(*) from t_team where owner_id=?');
$stmt->execute($user_id);
my @row = $stmt->fetchrow_array;
if($row[0]){
	exit_early("You have already selected a team.")
}
#####SELECTION FORM#####
$stmt = $dbh->prepare('select team_id,city,mascot from t_team' .
' where active=1 and owner_id=0');
if(!$stmt){
	print db_error();
	print end_html();
	exit(1);
}
if(!$stmt->execute){
	print db_error();
	print end_html();
	exit(1);
}
my @values;
my %labels;
while(my @row = $stmt->fetchrow_array){
	push(@values,$row[0]);
	$labels{$row[0]} = "$row[1] $row[2]";
}
print div_start('content');
print start_form(-method=>'post',-action=>url(-relative=>1));
print popup_menu(-name=>'team',-value=>\@values,-labels=>\%labels) . ' ';
print submit(-name=>'submit',-value=>'submit');
print end_form;
print div_end();
print end_html;

sub exit_early{
	my ($message) = @_;
	print $message;
	print end_html;
	exit(0);
}

sub validate{
	
	my ($session) = @_;
	my $team_id = param($PNAME_TEAM);
	my $user_id = $session->param('user');
	if(!$user_id){
		$ERRMSG='You must be logged in to select a team.  If you think you are ' .
		'already logged in then there might be a problem.  Please try again or ' .
		'contact the administrator.';
		print stderr $0,': session does not contain a value for user ID.',
		'The user should not be able to access this page without being',
		'logged in';
		return 0;
	}
	my $stmt = $dbh->prepare('update t_team set owner_id=? where team_id=? and active=1');
	if(!$stmt){
		$ERRMSG = db_error();
		return 0;
	}
	if(!$stmt->execute($user_id,$team_id)){
		$ERRMSG = db_error();
		return 0;
	}
	return 1;
}
