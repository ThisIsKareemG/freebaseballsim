package Fbs_utils;

use CGI ':standard';
use Digest::SHA 'sha256_base64';
use CGI::Session;

require Exporter;
our @ISA=qw(Exporter);
our @EXPORT=qw(
	div_start div_end main_links int_to_pos int_to_roster_pos
	bat_order_popup_menu lineup_pos_popup_menu league_popup_form
	team_popup_form pword_hash login_form get_session login init_session
	db_error $PNAME_TEAM $PNAME_USER $PNAME_LEAGUE get_team_id get_league_id
	ip_to_int);

$PNAME_TEAM = 'team';
$PNAME_USER = 'user';
$PNAME_LEAGUE ='league';


sub div_start{
	return qq{<div id="$_[0]">};
}

sub div_end{
	return qq{</div>};
}

sub main_links{
	my ($session) = @_;
	my $links = p(a({href=>'/'},'Home')) .
	p(a({href=>'standings.pl'},'Standings') . br .
	a({href=>'hitter_stats.pl'},'Hitter Stats') . br .
	a({href=>'pitcher_stats.pl'},'Pitcher Stats') . br .
	a({href=>'lineup.pl'},'Roster') . br .
	a({href=>'schedule.pl'},'Scores'));
	if(!$session){
		$links .= p(a({href=>'login.pl'},'Login') . br .
		a({href=>'signup.pl'},'Signup'));
	}
	else{
		$links .= p(a({href=>'logout.pl'},'Logout'));
	}
	return $links;
}

sub int_to_pos{
	
	my $pos = $_[0];
	if($pos == 0){
		return 'RP';
	}elsif($pos == 1){
		return 'SP';
	}elsif($pos == 2){
		return 'C';
	}elsif($pos == 3){
		return '1B';
	}elsif($pos == 4){
		return '2B';
	}elsif($pos == 5){
		return '3B';
	}elsif($pos == 6){
		return 'SS';
	}elsif($pos == 7){
		return 'LF';
	}elsif($pos == 8){
		return 'CF';
	}elsif($pos == 9){
		return 'RF';
	}else{
		die("int_to_pos: $pos is not a valid argument.\n");
	}
}

sub int_to_roster_pos{
	my $pos = $_[0];
	if($pos == 0){
		return "Bench";
	}else{
		return int_to_pos($pos);
	}
}

sub bat_order_popup_menu{

	my $player_id = $_[0];
	my $bat_order = $_[1];

	return popup_menu(
		-name=> "B$player_id",
		-values=> [1,2,3,4,5,6,7,8,9,0],
		-default=> $bat_order,
		-labels=> {1=>1,2=>2,3=>3,4=>4,5=>5,6=>6,7=>7,8=>8,9=>9,0=>"Bench"}
	);
}

sub lineup_pos_popup_menu{
	
	my $player_id = $_[0];
	my $current_position = $_[1];
	#The position argument is only temporary, in the future any
	#position will be allowed.
	my $position = $_[2];

	return popup_menu(
		-name=> "P$player_id",
		-values=> [$position,0],
		-default=> $current_position,
		-labels=> {$position=>int_to_roster_pos($position),0=>"Bench"}
	);
}

sub league_popup_form{
	my ($dbh,$league_id_ref) = @_;
	
	my $query=qq{
		select league_id, league_name from t_league
	};
	my $stmnt = $dbh->prepare($query) or die($query);
	$stmnt->execute or die($query);
	my @values;
	my %labels;
	while(my @row = $stmnt->fetchrow_array){
		push(@values,$row[0]);
		$labels{$row[0]} = $row[1];
	}
	if(!$$league_id_ref){
		$$league_id_ref = $values[0];
	}
	return
	start_form(-method=>"POST",-action=>url(-relative=>1)) . 
	popup_menu(-name=>"league", -values=>\@values, -default=>$$league_id_ref, -labels=>\%labels) .
	" " . submit(-name=>"leaguesubmit", -value=>"submit") .
	end_form;
	}

sub team_popup_form{
	my $dbh = $_[0];
	my $league_id = $_[1];
	my $default = $_[2];
	my $hidden_ref = $_[3];
	my %hidden = %$hidden_ref;

	my $hidden_fields = "";
	for(keys(%hidden)){
		$hidden_fields .= hidden(-name=>$_, -default=>$hidden{$_});
	}
	my $query=qq{
		select team_id, city, mascot from t_team where league_id=$league_id and active=1
	};
	my $stmnt = $dbh->prepare($query) or die($query);
	$stmnt->execute;
	my @values;
	my %labels;
	while(my @row = $stmnt->fetchrow_array){
		push(@values,$row[0]);
		$labels{$row[0]} = "$row[1] $row[2]";
	}
	return
	start_form(-method=>"POST",-action=>url(-relative=>1),-style=>'display:inline;') .
	popup_menu(-name=>"team",-value=>\@values,-default=>$default,-labels=>\%labels) .
	" " . submit(-name=>"teamsubmit", -value=>"submit") .
	$hidden_fields .
	end_form;
}

sub pword_hash{
	(my $password) = @_;
	return sha256_base64($password);
}

sub login_form{
	return 
	start_form(-method=>'POST',-action=>url(-base=>1) . '/login.pl') . 
	'Username ' . textfield(-class=>'login',-name=>'user',-size=>20,-maxlength=>20) . p .
	'Password ' . password_field(-class=>'login',-name=>'password',-size=>20,-maxlength=>20) . p .
#	We might use this if we put login boxes on each page.
#	hidden(-name=>'redirect',-value=>url()) .
	submit(-name=>'login"=', -value=>'submit');
	end_form();
}

sub login{

	my ($dbh, $user, $phash) = @_;
	my $stmt = $dbh->prepare('Select user_id from t_user where username=? and password=?');
	$stmt->execute($user,$phash);
	my @row = $stmt->fetchrow_array;
	return $row[0];
}

sub init_session{
	my ($user_id) = @_;
	my $session = new CGI::Session('driver:File', undef,
		{Directory=>'/tmp'});
	my $cookie = cookie(CGISESSID=>$session->id);
	$session->param('user',$user_id);
	return $cookie;
}

sub get_session{
	my $sid = cookie('CGISESSID') || param('CGISESSID');
	if($sid){
		$session = new CGI::Session('driver:File', $sid, {Directory=>'/tmp'});
		return $session;
	}
}

sub db_error{
	return 'Due to a database error, we are unable to complete your request. ' . 
	'Please contact the administrator.';
}
sub get_team_id{
	my ($session,$dbh,$league_id) = @_;
	my $team_id = url_param($PNAME_TEAM);
	if($team_id){
		return $team_id;
	}
	$team_id = param($PNAME_TEAM);
	if($team_id){
		return $team_id;
	}
	if($session && $dbh){
		my $user_id = $session->param($PNAME_USER);
		my $stmt;
		if($league_id){
			$stmt = $dbh->prepare('select team_id from t_team' .
				' where owner_id=? and league_id=?');
			$stmt->execute($user_id, $league_id);
		}
		else{
			$stmt = $dbh->prepare('select team_id from t_team' .
				' where owner_id=?');
			$stmt->execute($user_id);
		}
		my @row = $stmt->fetchrow_array;
		return $row[0];
	}
}

sub get_league_id{
	my ($session) = @_;
	my $league_id = url_param($PNAME_LEAGUE);
	if(!$league_id){
		$league_id = param($PNAME_LEAGUE);
	}
	if($session){
		if(!$league_id){
			$league_id = $session->param($PNAME_LEAGUE);
		}
		$session->param($PNAME_LEAGUE,$league_id);
	}
	return $league_id;
}
sub ip_to_int{
	my ($string) = @_;
	my @ip = split(/\./,$string);
	return ($ip[0] << 24) | ($ip[1] << 16) | ($ip[2] << 8) | $ip[3];
}

1;
