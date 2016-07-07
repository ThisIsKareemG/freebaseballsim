#!/usr/bin/perl -w
#Free Baseball Simulator - A configurable baseball simulator by Tom Stellard
#Copyright (C) 2009 Tom Stellard
#
#This file is part of Free Baseball Simulator
#
#Free Baseball Simulator is free software: you can redistribute it and/or modify
#it under the terms of the GNU General Public License as published by
#the Free Software Foundation, either version 3 of the License, or
#(at your option) any later version.
#
#Free Baseball Simulator is distributed in the hope that it will be useful,
#but WITHOUT ANY WARRANTY; without even the implied warranty of
#MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#GNU General Public License for more details.
#
#You should have received a copy of the GNU General Public License
#along with Free Baseball Simulator.  If not, see <http://www.gnu.org/licenses/>.


my $i = 1;

for($i;$i<163;$i++){
$command = "./sim_day";
	#print STDERR "Day: $i\n";
	system($command,1,$i,$ARGV[0],$ARGV[1],$ARGV[2]);
#	print stderr "* $?\n";
	if($? == 256){
		exit(1);
	}
}

#print "DONE SIMMING SEASON.\n";
