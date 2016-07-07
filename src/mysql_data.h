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

#ifndef MYSQL_DATA_H
#define MYSQL_DATA_H

#include <mysql/mysql.h>
#include "date.h"

typedef MYSQL native_conn;
typedef MYSQL_RES native_result;
typedef MYSQL_ROW native_row;

const static char* PERL_DB_TYPE = "mysql";

MYSQL* MYSQL_open_connection(FILE* config);

void MYSQL_close_connection(MYSQL* conn);

int MYSQL_execute_query(MYSQL* conn, const char* query, int query_length);

MYSQL_RES* MYSQL_execute_query_result(MYSQL* conn, const char* query, int query_length);

MYSQL_ROW MYSQL_fetch_row(MYSQL_RES* result);

void MYSQL_free_result(MYSQL_RES* result);

int MYSQL_last_insert_id(MYSQL* conn);

int MYSQL_get_column_int(MYSQL_ROW row, int index);

char* MYSQL_get_column_string(MYSQL_ROW row, int index);

Date MYSQL_get_column_date(MYSQL_ROW row, int index);

int MYSQL_begin_transaction();

int MYSQL_commit();

int MYSQL_rollback(MYSQL *conn);

int MYSQL_next_result(MYSQL* conn);

char* MYSQL_date_to_string(Date date);

#endif /* MYSQL_DATA_H */
