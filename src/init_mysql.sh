source `pwd`/mysql.config

if [ -z $port ]; then
	port=0
fi

GETOPTS=`getopt -o w --long web -- "$@"`
eval set -- "$GETOPTS"

while true
do
	case "$1" in
	-w|--web)
		web=1
		shift
		;;
	--)
		shift
		break
		;;
	esac
done

script_dir="../data/mysql"
master_scripts="\
schema.mysql \
t_boxscore.mysql \
test_league.mysql \
names.mysql"

if [ $web ]; then
	master_scripts="$master_scripts \
	sp_select_stat_skill_hitter_league.mysql \
	sp_select_stat_skill_pitcher_league.mysql \
	t_user.mysql"
fi
benchmark_scripts="\
real_baseball.mysql"

current_dir=`pwd`

cd $script_dir
echo "create database if not exists $master;" > temp.mysql
echo "use $master;" >> temp.mysql
cat temp.mysql $master_scripts | mysql -u $username -h $host -p$password -P $port

if [ -n $benchmark ]
then
	echo "create database if not exists $benchmark;" > temp.mysql
	echo "use $benchmark;" >> temp.mysql
	cat temp.mysql $benchmark_scripts | mysql -u $username -h $host -p$password -P $port
fi

cd $current_dir
./generate_schedule
if [ $? -eq 1 ]; then
	echo "Error generating schedule."
	exit 1
fi


