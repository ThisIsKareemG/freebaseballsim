#!/usr/bin/perl -w

use strict;
package Baseball;

require Exporter;
our @ISA=qw(Exporter);

our @EXPORT=qw(ba outs_to_ip);
our @EXPORT_OK = qw(ba outs_to_ip);





sub ba{
	my($ba) = @_;
	if(!$ba){
		return '.000';
	}
	elsif($ba == 1){
		return '1.000';
	}
	else{
		return substr($ba,1);
	}
}

sub outs_to_ip{
	my($outs) = @_;
	my $ip = int($outs/3).'.'.$outs%3;
	return $ip;
}

1;