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

#ifndef DATE_H
#define DATE_H

typedef struct date{
	int day;
	int month;
	int year;
	
}*Date;

enum date_type {MM_DD_YYYY, YYYY_MM_DD, DD_MM_YYYY};

Date create_Date(int month,int day,int year);
Date string_to_Date(const char* date_string, char* delimiter, enum  date_type type);
char* Date_to_string(Date date, char* delimiter, enum date_type type);
void Date_add_weeks(Date date,int weeks);
int days_in_month(int month, int year);

#endif /* DATE_H */
