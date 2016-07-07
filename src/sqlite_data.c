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
#include <stdio.h>
#include <string.h>
#include <regex.h>
#include <sqlite3.h>
#include "date.h"
#include "compiler_macros.h"
#include "baseball_constants.h"
#include "baseball_macros.h"
#include "tom_array_list.h"
#include "db_object.h"

const char* CONFIG_FILE = "sqlite.config";

void print_error(sqlite3* conn, const char* message){

	fprintf(stderr,"%s %s\n",message,sqlite3_errmsg(conn));
}

sqlite3 *sqlite_open_connection(FILE* config){
	
	char path_to_database[MAX_PATH_LENGTH];
	char err_buf[100];
	char config_line[MAX_PATH_LENGTH];
	int r;
	sqlite3 *conn;
	regex_t master;
	regmatch_t value[2];

	if(r=regcomp(&master,"^master=\\(.\\+\\?\\)\n$",0)){
		regerror(r,&master,err_buf,100);
		fprintf(stderr,err_buf);
		return NULL;
	}
	while(fgets(config_line,MAX_PATH_LENGTH,config)){
		if(!(r=regexec(&master,config_line,2,value,0))){
			strncpy(path_to_database,
					config_line+value[1].rm_so,
					value[1].rm_eo - value[1].rm_so
			);
			path_to_database[value[1].rm_eo - value[1].rm_so] = '\0';
			break;
		}
		else{
			regerror(r,&master,err_buf,100);
			fprintf(stderr,err_buf);
			return NULL;
		}
	}
	regfree(&master);
	if(unlikely(sqlite3_open(path_to_database,&conn)!= SQLITE_OK)){
		print_error(conn,"Sqlite error connecting");
		return NULL;
	}
	return conn;
}

void sqlite_close_connection(sqlite3* conn){

	if(unlikely(sqlite3_close(conn)!=SQLITE_OK)){
		print_error(conn,"Error closing connection.");
	}
}	

int sqlite_execute_query(sqlite3* conn, const char* query,int query_length){

	PRINT_QUERY(query);
	sqlite3_stmt *stmt;
	const char* rest_of_query;
	if(unlikely(sqlite3_prepare_v2(conn,query,query_length*sizeof(char),&stmt,&rest_of_query)!=SQLITE_OK)){
		print_error(conn,"execute_query: sqlite error preparing query:");
		return DBERR;
	}
	if(unlikely(sqlite3_step(stmt) != SQLITE_DONE)){
		print_error(conn,"execute_query: sqlite error steping:");
		return DBERR;
	}
	if(unlikely(sqlite3_finalize(stmt) != SQLITE_OK)){
		print_error(conn,"execute_query: sqlite error finalizing:");
		return DBERR;
	}
	PRINT_QUERY("Done.");
	int len = strlen(rest_of_query);
	if(len>0){
		return sqlite_execute_query(conn,rest_of_query,len);
	}
	return DBOK;
}

sqlite3_stmt *sqlite_execute_query_result(sqlite3* conn, const char* query, int query_length){
	
	PRINT_QUERY(query);
	sqlite3_stmt *stmt;
	if(unlikely(sqlite3_prepare_v2(conn,query,query_length*sizeof(char),&stmt,NULL)!=SQLITE_OK)){
		print_error(conn,"execute_query_result: sqlite error preparing query:");
		return NULL;
	}
	PRINT_QUERY("Done... Returning result.");	
	return stmt;
}

sqlite3_stmt *sqlite_fetch_row(sqlite3_stmt* stmt){
	int rc = sqlite3_step(stmt);
	if(rc == SQLITE_DONE){
		return NULL;
	}
	else{
		return stmt;
	}
}

void sqlite_free_result(sqlite3_stmt* result){
	
	if(unlikely(sqlite3_finalize(result) != SQLITE_OK)){
		fprintf(stderr,"free_result: failed.\n");
	}
}

int sqlite_last_insert_id(sqlite3* conn){

	return sqlite3_last_insert_rowid(conn);
}

int sqlite_get_column_int(sqlite3_stmt* stmt,int index){

	return sqlite3_column_int(stmt,index);
}

char* sqlite_get_column_string(sqlite3_stmt* stmt, int index){

	unsigned const char* temp = sqlite3_column_text(stmt,index);
	char * result = malloc((strlen(temp)+1)*sizeof(char));
	return strcpy(result,temp);
}

Date sqlite_get_column_date(sqlite3_stmt* stmt, int index){
	
	unsigned const char* temp = sqlite3_column_text(stmt,index);
	return string_to_Date(temp,"-", MM_DD_YYYY);
}

int sqlite_begin_transaction(sqlite3* conn){
	return sqlite_execute_query(conn,"Begin Transaction;",18);
}

int sqlite_commit(sqlite3* conn){
	return sqlite_execute_query(conn,"COMMIT;",7);
}

int sqlite_rollback(sqlite3 *conn){
	return sqlite_execute_query(conn, "ROLLBACK;", 9);
}

int sqlite_next_result(sqlite3* conn){
	return 0;
}

char* sqlite_date_to_string(Date date){
	return Date_to_string(date,"-",MM_DD_YYYY);
}

