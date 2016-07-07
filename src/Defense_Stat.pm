#!/usr/bin/perl -w

use strict;
package Defense_Stat;

sub new {
	my $defense_stat = {
		OUTS => 0,
		H => 0,
		R => 0,
		ER => 0,
		SO => 0,
		BB => 0,
		W => 0,
		L => 0,
		SV => 0,
		E => 0,
		PO => 0,
		A => 0,
		GP => 0,
		BP => 0
	};
	bless $defense_stat;
}

sub print{
	my($self) = @_;
	print "Putouts: ".$self->{PO};
	print " Assists: ".$self->{A};
	print "\n";
}
1;