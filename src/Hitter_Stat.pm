#/usr/bin/perl -w

use strict;
package Hitter_Stat;

sub new {
	my $hitter_stat = {
		AB => 0,
		H => 0,
		R => 0,
		RBI => 0,
		SO => 0,
		BB => 0,
		IB => 0,
		ZB => 0,
		EB => 0,
		HR => 0,
	};
	bless $hitter_stat;
}
1;
