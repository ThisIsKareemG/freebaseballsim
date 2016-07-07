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

#include <stdlib.h>
#include "db_object.h"
#include "compiler_macros.h"
#include "baseball_data.h"

Db_Object db_connect(const char* config_path){

	Db_Object db = malloc(sizeof(struct db_object));


#ifdef _MYSQL_
	db->open_connection = &MYSQL_open_connection;
	db->execute_query = &MYSQL_execute_query;
	db->execute_query_result = &MYSQL_execute_query_result;
	db->fetch_row = &MYSQL_fetch_row;
	db->free_result = &MYSQL_free_result;
	db->begin_transaction = &MYSQL_begin_transaction;
	db->commit = &MYSQL_commit;
	db->rollback = &MYSQL_rollback;
	db->close_connection = &MYSQL_close_connection;
	db->last_insert_id = &MYSQL_last_insert_id;
	db->get_column_int = &MYSQL_get_column_int;
	db->get_column_string = &MYSQL_get_column_string;
	db->get_column_date = &MYSQL_get_column_date;
	db->next_result = &MYSQL_next_result;
	db->date_to_string = &MYSQL_date_to_string;	
#else
	db->open_connection = &sqlite_open_connection;
	db->execute_query = &sqlite_execute_query;
	db->execute_query_result = &sqlite_execute_query_result;
	db->fetch_row = &sqlite_fetch_row;
	db->free_result = &sqlite_free_result;
	db->begin_transaction = &sqlite_begin_transaction;
	db->commit = &sqlite_commit;
	db->rollback = &sqlite_rollback;
	db->close_connection = &sqlite_close_connection;
	db->last_insert_id = &sqlite_last_insert_id;
	db->get_column_int = &sqlite_get_column_int;
	db->get_column_string = &sqlite_get_column_string;
	db->get_column_date = &sqlite_get_column_date;
	db->next_result = &sqlite_next_result;
	db->date_to_string = &sqlite_date_to_string;	
#endif
	
	FILE *config = fopen(config_path,"r");
	if(unlikely(config == NULL)){
		fprintf(stderr,"Cannot open config file:%s\n",config_path);
		exit(1);
	}
	db->conn = db->open_connection(config);
	if(!db->conn){
		return NULL;
	}
	else{
		return db;
	}
}
