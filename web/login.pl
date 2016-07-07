#!/usr/bin/perl

use strict;
use lib '../src';
use CGI ':standard';
use CGI::Session;
use DBI;

use Fbs_utils;
use Data_Interface;

#####PARAMETERS#####
my $PNAME_LOGIN = 'login';
my $PNAME_USER = 'user';
my $PNAME_PWORD = 'password';

#####VALIDATE LOGIN#####
my $ERRMSG;
my $cookie;
my $user;
if(param($PNAME_LOGIN)){
	$user = validate();
	if($user){
		$cookie = init_session($user);
	}
	else{
		Delete($PNAME_USER);
		Delete($PNAME_PWORD);
		$ERRMSG = 'Login failed.'
	}
}

#####BEGIN HTML#####
print header(-cookie=>$cookie);
print start_html(-title=>"Free Baseball Simulator - Login",
	-style=>{'src'=>'/style.css'});
print h1({-align=>'center'},'Login');
if($ERRMSG){
	print $ERRMSG, p;
}

#####LINKS#####
print div_start('links');
print main_links($user);
print div_end();

#####LOGIN FORM#####
print div_start('content');
if($user){
	print 'Login successful.';

}
else{
	print login_form();
}
print div_end();
print end_html();

sub validate{
	
	my $user = param($PNAME_USER);
	my $pass = param($PNAME_PWORD);
	my $phash = pword_hash($pass);
	my $dbh = Data_Interface::connect('mysql');
	return login($dbh, $user, $phash);
}
