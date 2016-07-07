#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.
m4_include(ac_prog_perl_modules.m4)
AC_PREREQ(2.61)
AC_INIT(Free Baseball Simulator, 0.5.1, http://freebaseballsim.sourceforge.net)
AC_CONFIG_SRCDIR([tom_array_list.h])
#AC_CONFIG_HEADER([config.h])

# Checks for programs.
AC_PROG_CC
AC_PROG_CPP

# Checks for libraries.
AC_CHECK_LIB([m], [pow])

# Check for perl
AC_PATH_PROG(PERL,perl,[no])
if test "x$PERL" = "xno"; then
	AC_MSG_FAILURE([Perl not found on your system.  Please install perl and try again.])
fi
AC_SUBST(PERL)

# Check for Getopt::Long
AC_PROG_PERL_MODULES([Getopt::Long], ,
	[AC_MSG_FAILURE([Please install perl module Getopt::Long])])

# Checking for DBI
AC_PROG_PERL_MODULES([DBI], ,
	[AC_MSG_FAILURE([Please install perl module DBI])])

# Check for optional perl modules.
AC_PROG_PERL_MODULES([Statistics::Descriptive],
	[AC_SUBST([PERL_STAT_DESC],[1])],
	[AC_SUBST([PERL_STAT_DESC],[0])])
AC_PROG_PERL_MODULES([CGI],
	[AC_SUBST([PERL_CGI],[1])],
	[AC_SUBST([PERL_CGI],[0])])

AC_ARG_ENABLE(db, 
	AC_HELP_STRING([--enable-db=],
		[Specify the database type (sqlite,mysql) (default=sqlite)]
	),
	default_db=$enableval,
	default_db=sqlite
)

case $default_db in
	mysql)
		AC_CHECK_LIB([mysqlclient], [mysql_init],
				[LIBS="$LIBS -lmysqlclient"
				CPPFLAGS="$CPPFLAGS -D_MYSQL_"
				AC_SUBST([DB_OBJECT],["mysql_data.o"])],
				
			[AC_ERROR([MySQL libraries are not installed.])]
		)
		AC_PROG_PERL_MODULES([DBD::mysql], ,
			AC_MSG_FAILURE([Please install perl module DBD:mysql or use a different database.]))
		AC_CHECK_PROG([MYSQL_CLIENT],[mysql],[yes],[no])
		if test "x$MYSQL_CLIENT" == "xno"; then
			AC_ERROR([MySQL client is not installed])
		fi
		;;
	sqlite)
		AC_CHECK_LIB([sqlite3], [sqlite3_open],
			[LIBS="$LIBS -lsqlite3"
				AC_SUBST([DB_OBJECT],["sqlite_data.o"])],
			[AC_ERROR([SQLite libraries are not installed.])]
		)
		AC_PROG_PERL_MODULES([DBD::SQLite], ,
			AC_MSG_FAILURE([Please install perl module DBD:SQLite or use a different database.]))
		AC_PATH_PROG(SQLITE3,[sqlite3],[no])
		if test "x$SQLITE3" == "xno"; then
			AC_ERROR([SQLite client not installed.])
		else
			#Check sqlite version
			AC_MSG_CHECKING([version of sqlite])
			SQLITE_VERSION=`$SQLITE3 -version`
			AC_MSG_RESULT([$SQLITE_VERSION])
			if test "x$SQLITE_VERSION" == "x3.5.9"; then
				AC_MSG_WARN([Your version of SQLite may have bugs.  Please see the INSTALL file for more info.])
			fi
		fi
		;;
	*)
		AC_ERROR(
			[$default_db is not a supported database.  Try: sqlite or mysql])
		;;
esac

AC_ARG_WITH([gsl],
	[AC_HELP_STRING([--with-gsl],
		[Use GNU Scientific Library for faster normally distributed random numbers. @<:@default=no@:>@])],
	[],
	[with_gsl=no]
)

libgsl_errmsg="Fast calculation of normally distributed random numbers disabled."
if test "x$with_gsl" == "xyes"; then
		AC_CHECK_LIB([gsl],[gsl_rng_alloc],[havegsl=1],
			[AC_CHECK_LIB([gsl],[gsl_rng_free],[havegsl=1; gsl_cblas=1],
				[AC_MSG_WARN([Libgsl not found $libgsl_errmsg])],
				[-lgslcblas]
			)]
		)	
		if test "x$havegsl" == "x1"; then	
			CPPFLAGS="$CPPFLAGS -D_USEGSL_"
			LIBS="$LIBS -lgsl"
			if test "x$gsl_cblas" == "x1"; then
				LIBS="$LIBS -lgslcblas"
			fi
		fi
fi

#Set paths
srcdir=`pwd`
datadir="$srcdir/../data"
bindir="$srcdir/../bin"

AC_SUBST([SRC_DIR],[$srcdir])
AC_SUBST([DATA_DIR],[$datadir])
AC_SUBST([BIN_DIR],[$bindir])

# Checks for header files.
AC_HEADER_STDC
AC_CHECK_HEADERS([fcntl.h limits.h stdlib.h string.h unistd.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_C_CONST
AC_C_INLINE
AC_TYPE_PID_T
AC_HEADER_STDBOOL

# Checks for library functions.
AC_FUNC_FORK
AC_FUNC_MALLOC
AC_FUNC_REALLOC
AC_FUNC_VPRINTF
AC_CHECK_FUNCS([dup2 memset pow regcomp sqrt])

AC_CONFIG_FILES([makefile FBS_Config.pm gtk/makefile FBS-config.sh])
AC_OUTPUT
