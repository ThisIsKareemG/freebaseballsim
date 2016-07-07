#!/usr/bin/perl -w


package Links;


require Exporter;
our @ISA=qw(Exporter);

our @EXPORT=qw(sort_link);
our @EXPORT_OK = qw(sort_link);

use strict;
use CGI ':standard',':cgi-lib';

my $SORT = 'sort';

sub sort_link{

	my ($column) = @_;
	my $url = url().'?';
	my %hash =Vars();
	my $has_sort = 0;
	foreach my $key(keys %hash){
		
		if($key ne $SORT){
			$url.="$key=".param($key)."&";
		}		
	}
	$url.=$SORT."=".$column."&";
	return a({href=>substr($url,0,-1)},$column);
}