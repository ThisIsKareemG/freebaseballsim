/*
Free Baseball Simulator - A configurable baseball simulator by Tom Stellard
Copyright (C) 2009 Tom Stellard

This file is part of Free Baseball Simulator

Free Baseball Simulator is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Free Baseball Simulator is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with Free Baseball Simulator.  If not, see <http://www.gnu.org/licenses/>.
*/

/*! \file db_object.h */

#ifndef DB_OBJECT_H
#define DB_OBJECT_H

#include <stdio.h>
#include "baseball_objects.h"
#include "date.h"
#ifdef _MYSQL_
#include "mysql_data.h"
#else
#include "sqlite_data.h"
#endif

#define DBOK 0
#define DBERR -1

extern const char *CONFIG_FILE;

/*!
 * \brief This struct represents a connection to a database.
 */
typedef struct db_object{

	native_conn* conn;
	
	/*!
	 * This function opens a database connection based on the configuration
	 * information in config.
	 * @param config Open file handle with configuration information.
	 * @return On success a pointer to the open connection object, or NULL
	 * if an error occured.  This function may print out any errors to
	 * STDERR.
	 */
	native_conn* (*open_connection)(FILE*);

	/*!
	 * This function executes a query that does not return a result set.
	 * @param conn Database connection
	 * @param query SQL query to execute
	 * @param query_length The number of characters in the query not
	 * including the terminating '\\0' character.
	 * @return 0 on success, DBERR if an error occured.
	 */
	int (*execute_query)(native_conn*, const char*, int);

	/*!
	 * This function executes a query and returns a result object.
	 * @param conn Database connection
	 * @param query SQL query to execute
	 * @param query_length The number of characters in the query not
	 * including the terminating '\\0' character.
	 * @return A pointer to a result object or NULL if an error occured.
	 */
	native_result* (*execute_query_result)(native_conn*, const char*, int);

	native_row (*fetch_row)(native_result*);

	void (*free_result)(native_result*);
	
	int (*last_insert_id)(native_conn*);
	
	int (*get_column_int)(native_row,int);
	
	char* (*get_column_string)(native_row, int);
	
	Date (*get_column_date)(native_row, int);
	
	/*!
	 * @return 0 on success, DBERR if an error occured.
	 */
	int (*begin_transaction)(native_conn*);

	int (*commit)(native_conn*);

	int (*rollback)(native_conn*);
	
	void (*close_connection)(native_conn*);

	int (*next_result)(native_conn*);

	char* (*date_to_string)(Date);

}*Db_Object;

Db_Object db_connect(const char* config_path);

#endif /* DB_OBJECT_H */
