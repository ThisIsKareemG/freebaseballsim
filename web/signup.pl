#!/usr/bin/perl

use strict;
use lib '../src';
use CGI ':standard';
use DBI;

use Fbs_utils;
use Data_Interface;
#####PARAMETERS#####
my $PNAME_USER = 'user';
my $PNAME_PWORD = 'password';
my $PNAME_PWORD2 = 'password2';
#my $PNAME_EMAIL = 'email';
my $PNAME_SUBMIT = 'submit';

#####VALIDATE FORM#####
my $ERRMSG;
my $user_id;
my $cookie;
if(param($PNAME_SUBMIT)){
	$user_id = validate();
	if($user_id){
		$cookie = init_session($user_id);
	}
}
#####BEGIN HTML#####
print header(-cookie=>$cookie);
print start_html(-title=>"Free Baseball Simulator - Signup",-style=>{'src'=>'/style.css'});
print h1({-align=>"center"},"Signup");

#####LINKS#####
print div_start('links');
print main_links($user_id);
print div_end();

#####SIGNUP COMPLETED#####
if($user_id){
	print 'You have successfully created an account!  Click ',
	a({href=>'select_team.pl'},'here'), ' to select a team.', br;
	print end_html();
	exit(0);
}

#####SIGNUP FORM#####
print div_start("content");
if($ERRMSG){
	print $ERRMSG,p;
	Delete($PNAME_PWORD);
	Delete($PNAME_PWORD2);
}
print start_form(-method=>"POST",-action=>url(-relative=>1));
print "Login Name";
print textfield(-class=>'register',-name=>$PNAME_USER,-size=>20,-maxlength=>20),p;
print "Password";
print password_field(-class=>'register',-name=>$PNAME_PWORD,-size=>20,-maxlength=>20),p;
print 'Re-enter Password',
password_field(-class=>'register',-name=>$PNAME_PWORD2,-size=>20,-maxlength=>20),p;
#print 'E-Mail',
#textfield(-class=>'register',-name=>$PNAME_EMAIL,-size=>20,-maxlength=>20), p;
print submit(-class=>'register',-name=>$PNAME_SUBMIT,-value=>'submit');
print end_form();
print div_end();
print end_html();

sub validate{

	my $user = param($PNAME_USER);
	my $pword = param($PNAME_PWORD);
	my $pword2 = param($PNAME_PWORD2);
#	my $email = param($PNAME_EMAIL);
	if(!$user){
		$ERRMSG = 'Please enter a login name.';
		return 0;
	}
	if(!$pword){
		$ERRMSG = 'Please enter a password.';
		return 0;
	}
	if($pword ne $pword2){
		$ERRMSG = 'Passwords don\'t match.';
		return 0;
	}
	my $dbh = Data_Interface::connect('mysql');
	if(!$dbh){
		print stderr 'Cannot connect to database from ',$0;
		$ERRMSG = db_error();
		return 0;
	}
	my $stmt = $dbh->prepare('Select count(*) from t_user where username=?');
	if(!$stmt){
		$ERRMSG = db_error();
		return 0;
	}
	if(!$stmt->execute($user)){
		$ERRMSG = db_error();
		return 0;
	}
	my @row = $stmt->fetchrow_array;
	if($row[0] > 0){
		$ERRMSG = 'Login name already exists.';
		return 0;
	}
	my $phash = pword_hash($pword);
	my $ip = ip_to_int(remote_host());
	if(!$dbh->do('Insert into t_user(username,password,ip) values(?,?,?)',
		undef,$user,$phash,$ip)){
			$ERRMSG = db_error();
			return 0;
		}
	return $dbh->last_insert_id(undef,undef,undef,undef);
}
