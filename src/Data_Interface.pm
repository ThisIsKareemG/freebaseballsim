
use strict;
package Data_Interface;

sub driver{
	
	my $type = $_[0];
	if($type eq 'sqlite'){
		return 'Dbi:SQLite';
	}elsif($type eq 'mysql'){
		return 'Dbi:mysql';
	}else{
		die("Unknown database type");
	}
}

#ARGS:
#Database type
#Config file (optional)
#IsBenchmark (optional)
sub connect{
	
	my ($type, $file, $is_bench) = @_;
	my %config = read_config($type, $file);
	my $host = $config{"host"} ? ":" . $config{"host"} : "";
	if($config{"port"}){
		$host.= ":" . $config{"port"};
	}
	my $db = $is_bench ? $config{"benchmark"} : $config{"master"};

	my %vendor_config;
	if($type eq 'mysql'){
		%vendor_config=("mysql_auto_reconnect"=>1);
	}
	my $dbh = DBI->connect(
		driver($type) . ":" .
		$db .
		$host,
		$config{"username"},
		$config{"password"},
		\%vendor_config
	);
	return $dbh;
}

sub read_config{
	my ($type, $file) = @_;
	my %config;
	if(!$file){
		$file = $type . '.config';
	}
	open(CONFIG, $file) or die("Cannot open config file: $file\n");
	$config{'port'} = 0;
	while(<CONFIG>){
		if($_ =~ /^host\=(.+)$/){
			$config{"host"} = $1;
		}
		elsif($_ =~ /^username\=(.+)$/){
			$config{"username"} = $1;
		}
		elsif($_ =~ /^password\=(.+)$/){
			$config{"password"} = $1;
		}
		elsif($_ =~ /^master=(.+)$/){
			$config{"master"} = $1;
		}
		elsif($_ =~ /^benchmark\=(.+)$/){
			$config{"benchmark"} = $1;
		}
		elsif($_ =~ /^port\=(.+)$/){
			$config{"port"} = $1;
		}
		elsif($_ =~ /^\#/){
			#Ignore the comment
		}
		else{
			die("Error parsing config file line:\n$_");
		}
	}
	close(CONFIG);
	return %config;
}

sub date_to_year{
	my $db_type = $_[0];
	my $date = $_[1];
	if($db_type eq 'mysql'){
		$date =~ s/([0-9]+)\-[0-9]+\-[0-9]+/$1/g;
	}
	else{
		$date =~ s/[0-9]+\-[0-9]+\-([0-9]+)/$1/g
	}
	return $date;
}

sub date_string{
	my $db_type = $_[0];
	my $year = $_[1];
	my $month = $_[2];
	my $day = $_[3];

	if($db_type eq 'mysql'){
		return "$year-$month-$day";
	}
	else{
		return "$month-$day-$year";
	}
}

sub date_add_year{
	my $db_type = $_[0];
	my $date = $_[1];
	my $num_years = $_[2];

	if($date =~ /([0-9]+)\-([0-9]+)\-([0-9]+)/){
		if($db_type eq 'mysql'){
			return date_string($db_type,$1+1,$2,$3);
		}
		else{
			return date_string($db_type,$3+1,$1,$2);
		}
	}
	else{
		die("$date is invalid");
	}
}

sub hand_to_int{

	my $hand = $_[0];
	if($hand eq "R"){
		return 0;
	}
	elsif($hand eq "L"){
		return 1;
	}
	else{
		die("$hand is an invalid argument to hand_to_int\n");
	}
}
1;
