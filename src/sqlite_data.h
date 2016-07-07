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

#ifndef _SQLITE_DATA_H
#define _SQLITE_DATA_H

#include <stdio.h>
#include <sqlite3.h>
#include "tom_array_list.h"
#include "date.h"

typedef sqlite3 native_conn;
typedef sqlite3_stmt native_result;
typedef sqlite3_stmt* native_row;

const static char* PERL_DB_TYPE = "sqlite";

native_conn*  sqlite_open_connection(FILE *config);

void sqlite_close_connection(native_conn* conn);

int sqlite_execute_query(native_conn* conn, const char* query, int query_length);

native_result* sqlite_execute_query_result(native_conn* conn, const char* query, int query_length);

native_row sqlite_fetch_row(native_result* result);

void sqlite_free_result(native_result* result);

int sqlite_last_insert_id(native_conn* conn);

int sqlite_get_column_int(native_row row, int index);

char *sqlite_get_column_string(native_row row, int index);

Date sqlite_get_column_date(native_row row, int index);

int sqlite_begin_transaction();

int sqlite_commit();

int sqlite_rollback(sqlite3 *conn);

int sqlite_next_result(native_conn* conn);

char *sqlite_date_to_string(Date date);

#endif /* _SQLITE_DATA_H */
