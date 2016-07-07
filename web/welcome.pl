#!/usr/bin/perl

use strict;
use lib '../src';
use CGI ':standard';

use Fbs_utils;
#####BEGIN HTML#####
print header;
print start_html(-title=>'Free Baseball Simulator - Account Created',
	-style=>{'src'=>'/style.css'});
print h1({-align=>'center'},'Account Created');

#####LINKS#####
print div_start('links');
print main_links();
print div_end();

#####CONTENT#####
print div_start('content');
print 'Your account has been successfully created.';
print div_end();

print end_html();
