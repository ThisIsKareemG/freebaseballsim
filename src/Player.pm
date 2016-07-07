use strict;
use Hitter_Stat;
use Defense_Stat;

package Player;

sub new{
	my $player= {
		os => {"R"=>new Hitter_Stat,"L"=>new Hitter_Stat},
		ds => {"R"=>new Defense_Stat,"L"=>new Defense_Stat},
		id => $_[1],
		team => -1,
		name => '',
		pos => -1
	};
	bless $player;
}

sub OUTS{
	my $player = $_[0];
	return $player->{ds}{"R"}->{OUTS} + $player->{ds}{"L"}->{OUTS};
}
1;
