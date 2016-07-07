#!/usr/bin/perl

use strict;
use lib '../src';
use CGI ':standard';
use CGI::Session;

use Fbs_utils;

#####LOGOUT#####
my $session = get_session();
undef($session);

#####BEGIN HTML#####
print header;
print start_html(-title=>'Free Baseball Simulator - Logout',
	-style=>{'src'=>'/style.css'});

#####LINKS#####
print div_start('links');
print main_links($session);
print div_end();

#####MESSAGE#####
print div_start('content');
print 'You have been successfully logged out.';
print div_end();

print end_html();
