source "`pwd`/FBS-config.sh"
DB="baseball.db"
BENCH="real_baseball.db"
if [ -n "$1" ]; then
	echo $1
	CONFIG=$1
	source $CONFIG
	DB=$master
	BENCH=$benchmark
fi

DB_SCRIPTS="\
t_boxscore.sqlite \
t_draft.sqlite \
t_draft_pref.sqlite \
t_hand.sqlite \
t_league.sqlite \
t_lineup.sqlite \
t_name.sqlite \
t_player.sqlite \
t_player_status.sqlite \
t_rotation.sqlite \
t_schedule.sqlite \
t_skill_fielder.sqlite \
t_skill_hitter.sqlite \
t_skill_pitcher.sqlite \
t_stat_fielder.sqlite \
t_stat_hitter.sqlite \
t_stat_pitcher.sqlite \
t_team.sqlite \
t_user.sqlite \
insert_names.sqlite \
insert_test_league.sqlite"

SQLITE_PATH="$DATA_DIR/sqlite"
cd $SQLITE_PATH

#Create main database.
if test -f $DB; then
	rm $DB
fi

cat $DB_SCRIPTS | sqlite3 $DB

#Create database with real stats for testing.
if [ -n "$BENCH" ]; then
	if test -f $BENCH; then
		rm $BENCH
	fi
	sqlite3 $BENCH < real_baseball.sqlite
fi

cd $BIN_DIR

#Create config file
if [ -z "$CONFIG" ]; then
	CONFIG="sqlite.config"
	echo "master=$SQLITE_PATH/baseball.db" > $CONFIG
	echo "benchmark=$SQLITE_PATH/real_baseball.db" >> $CONFIG
fi
#Insert the league schedule

$BIN_DIR/generate_schedule --config=$CONFIG
