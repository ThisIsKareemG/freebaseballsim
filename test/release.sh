print_conf_opts()
{
	echo "Configure options: $1" >> $LOGFILE
}
LOGFILE=`pwd`/release.log
DBS="NULL sqlite mysql"
OPTS="NULL --with-gsl"
REPS="1 10"
: > $LOGFILE

case $1 in
	"quick")
		DBS="sqlite mysql"
		OPTS="NULL"
		REPS="1"
		echo "Running in quick mode" >> $LOGFILE
		;;
	*)
		echo "Running in regular mode" >> $LOGFILE
		;;
esac
		
echo "Entering src directory..."
cd ../src
echo "Running autoconf..."
autoconf
if [ $? -eq 127 ]; then
	echo "You need autoconf to configure Free Baseball Simulator."
	echo "Ubuntu: apt-get install autoconf"
	echo "Gentoo: emerge autoconf"
	exit 1
fi
for db in $DBS
do
	case $db in
	"NULL")
		db_opt=''
		db=sqlite
		;;
	*)
		db_opt="--enable-db=$db"
		;;
	esac
	for opt in $OPTS
	do
		if [ $opt = "NULL" ]; then
			opt=''
		fi
		conf_opts="$opt $db_opt"
		cd ../src
		print_conf_opts "$conf_opts"
		./configure $conf_opts 1>/dev/null 2>>$LOGFILE
		if [ $? -eq 1 ]; then
			echo "Configure failed!!!" >> $LOGFILE
			continue
		fi
		echo "Running make..."
		make -B all 1>/dev/null
		case $? in
		2)
			echo "Make failed!!!" >> $LOGFILE
			continue
		;;
		127)
			echo "You need make to build Free Baseball Simlulator."
			echo "Ubuntu: apt-get install make"
			echo "Gentoo: emerge make"
			exit 1
		;;
		esac
		cd ../bin
		echo "Creating database..."
		if [ $db = mysql ]; then
			bash init_mysql.sh
			if [ $? -eq 1 ]; then
				echo "MySQL database creation failed!!!" >> $LOGFILE
				break
			fi
		else
			bash init_sqlite.sh
			if [ $? -eq 1 ]; then
				echo "SQLite database creation failed!!!" >> $LOGFILE
				break
			fi
		fi
		for seasons in $REPS
		do
			echo "Running simulation..."
			echo "Simulation options: seasons=$seasons db=$db" >> $LOGFILE
			perl goals.pl $seasons --regression "--db=$db" \
						1>$db.$seasons 2>$db.$seasons.err
			if [ $? -eq 1 ]; then
				echo "Simulation failed!!!" >> $LOGFILE
				break
			fi
			echo "Checking results..."
			diff -I 'TEST' $db.$seasons ../test/benchmark.$seasons
			if [ $? -eq 0 ]; then
				echo "Stats OK" >> $LOGFILE
			else
				echo "Stats check failed!!!" >> $LOGFILE
				break
			fi
			diff $db.$seasons.err ../test/benchmark.$seasons.err
			if [ $? -eq 0 ]; then
				echo "Messages OK" >> $LOGFILE
			else
				echo "Message check failed!!!">> $LOGFILE
				break
			fi	
		done
	done
done
