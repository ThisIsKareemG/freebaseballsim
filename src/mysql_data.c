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
#include "mysql_data.h"
#include "db_object.h"
#include "baseball_constants.h"
#include "date.h"

const int MAX_STRING_LENGTH = 20;
const char* CONFIG_FILE = "mysql.config";

int mysql_match_config_value(regex_t *expr, const char* line, regmatch_t subexp[2], char* value, int max_len){

	int r;
	int value_len;
	if(!(r=regexec(expr, line, 2, subexp, 0))){
		value_len = subexp[1].rm_eo - subexp[1].rm_so;
		max_len = max_len > value_len ? value_len : max_len;
		strncpy(value,line+subexp[1].rm_so,max_len);
		value[max_len] = '\0';
	}
	return r;
}

MYSQL* MYSQL_open_connection(FILE* config){

	MYSQL *conn;
	int r;
	int check = 0;
	char host[40];
	char user[20];
	char pass[20];
	char db[20];
	int port = 0;
	char err_buf[100];
	char config_line[50];

	regex_t regex_host;
	regex_t regex_user;
	regex_t regex_pass;
	regex_t regex_db;
	regex_t regex_port;
	regex_t comment;

	regmatch_t subexp[2];

	if(r=regcomp(&regex_host,"^host=\\(.\\+\\?\\)\n$",0)){
		regerror(r,&regex_host,err_buf,100);
		fprintf(stderr,err_buf);
	}
	if(r=regcomp(&regex_user,"^username=\\(.\\+\\?\\)\n$",0)){
		regerror(r,&regex_user,err_buf,100);
		fprintf(stderr,err_buf);
	}
	if(r=regcomp(&regex_pass,"^password=\\(.\\+\\?\\)\n$",0)){
		regerror(r,&regex_pass,err_buf,100);
		fprintf(stderr,err_buf);
	}
	if(r=regcomp(&regex_db,"^master=\\(.\\+\\?\\)\n$",0)){
		regerror(r,&regex_db,err_buf,100);
		fprintf(stderr,err_buf);
	}
	if(r=regcomp(&regex_port,"^port=\\(.\\+\\?\\)\n$",0)){
		regerror(r,&regex_port,err_buf,100);
		fprintf(stderr,err_buf);
	}
	if(r=regcomp(&comment,"^#",0)){
		regerror(r,&comment,err_buf,100);
		fprintf(stderr,err_buf);
	}

	while(fgets(config_line,50,config)){
		/*Read the host value.*/
		if(!mysql_match_config_value(&regex_host,config_line,subexp,host,40)){
			check |= 1<<0;
			continue;
		}
		else if(!mysql_match_config_value(&regex_user,config_line,subexp,user,20)){
			check |= 1<<1;
			continue;
		}
		else if(!mysql_match_config_value(&regex_pass,config_line,subexp,pass,20)){
			check |= 1<<2;
			continue;
		}
		else if(!mysql_match_config_value(&regex_db,config_line,subexp,db,20)){
			check |= 1<<3;
			continue;
		}
		else if(!(r=regexec(&regex_port, config_line, 2, subexp, 0))){
			int len = subexp[1].rm_eo - subexp[1].rm_so;
			strncpy(config_line,config_line+subexp[1].rm_so,len);
			config_line[len] = '\0';
			port = atoi(config_line);
		}
		else if(!(r=regexec(&comment,config_line,0,NULL,0))){
			/*Skip the comment.*/
			continue;
		}
		/* benchmark is used by the perl scripts, but not here. */
		else if(strncmp(config_line,"benchmark",9)){
			fprintf(stderr,"Error in config file:\n%s",config_line);
		}
	}
	regfree(&regex_host);
	regfree(&regex_user);
	regfree(&regex_pass);
	regfree(&regex_db);
	regfree(&regex_port);
	regfree(&comment);

	if(check == (1<<4) - 1){
		conn = mysql_init(NULL);
		if(!mysql_real_connect(conn,host,user,pass,db,port,NULL,
			CLIENT_MULTI_RESULTS|CLIENT_MULTI_STATEMENTS)){
			
			fprintf(stderr, "%s\nHost: %s\nUser: %s\nDB: %s\n",
				mysql_error(conn),host,user,db);
			mysql_close(conn);
			return NULL;
		}
	}
	else{
		fprintf(stderr,"Missing a config option err=%d\n",check);
		return NULL;
	}
	return conn;
}

void MYSQL_close_connection(MYSQL* conn){
	mysql_close(conn);
}

int MYSQL_execute_query(MYSQL* conn, const char* query, int query_length){
	#ifdef _PRINTQUERY_
		fprintf(stderr,"Executing query:%s\n",query);
	#endif
	if(mysql_real_query(conn, query, query_length)){
		fprintf(stderr,"%s\n>%s",mysql_error(conn), query);
		return DBERR;		
	}
	#ifdef _PRINTQUERY_
		fprintf(stderr,"Done.\n");
	#endif
	/*We need this while loop in case the query string contains multiple
	queries.*/
	while(!mysql_next_result(conn));
	return DBOK;
}

MYSQL_RES* MYSQL_execute_query_result(MYSQL* conn, const char* query, int query_length){
	if(MYSQL_execute_query(conn,query,query_length)){
		return NULL;
	}
	MYSQL_RES* result = mysql_store_result(conn);
	return result;
}

MYSQL_ROW MYSQL_fetch_row(MYSQL_RES* result){
	return mysql_fetch_row(result);
}

void MYSQL_free_result(MYSQL_RES* result){
	mysql_free_result(result);
}

int MYSQL_last_insert_id(MYSQL* conn){
	return mysql_insert_id(conn);
}

int MYSQL_get_column_int(MYSQL_ROW row, int index){
	return atoi(row[index]);
}

char* MYSQL_get_column_string(MYSQL_ROW row, int index){
	/*TODO: Can this be implemented a little nicer?*/
	char *string = malloc(sizeof(char) * MAX_STRING_LENGTH);
	return strncpy(string,row[index],MAX_STRING_LENGTH);
}

Date MYSQL_get_column_date(MYSQL_ROW row, int index){
	return string_to_Date(MYSQL_get_column_string(row,index), "-", YYYY_MM_DD);
}

int MYSQL_begin_transaction(MYSQL* conn){
	return MYSQL_execute_query(conn,"START TRANSACTION",17);
}

int MYSQL_commit(MYSQL* conn){
	return MYSQL_execute_query(conn,"COMMIT",6);
}

int MYSQL_rollback(MYSQL* conn){
	return MYSQL_execute_query(conn, "ROLLBACK", 8);
}

int MYSQL_next_result(MYSQL* conn){
	return mysql_next_result(conn);
}

char* MYSQL_date_to_string(Date date){
	return Date_to_string(date,"-", YYYY_MM_DD);
}
