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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "date.h"

Date create_Date(int month,int day,int year){
    
    Date new_date = malloc(sizeof(struct date));
    new_date->month = month;
    new_date->day = day;
    new_date->year = year;
    
    return new_date;
}

Date string_to_Date(const char* date_string, char* delimiter, enum date_type type){
   
    char* saveptr = malloc(sizeof(char) * 12);
    char* x = malloc(sizeof(char)*12);
    strcpy(x,date_string);
    int token_a, token_b, token_c;
    char* temp = strtok_r(x,delimiter,&saveptr);
    token_a = atoi(temp);
    temp = strtok_r(NULL,delimiter,&saveptr);
    token_b = atoi(temp);
    temp = strtok_r(NULL,delimiter, &saveptr);
    token_c = atoi(temp);
 
    Date new_date;
    
    switch(type){
    
    case MM_DD_YYYY:
        new_date = create_Date(token_a, token_b, token_c);
	break;

    case YYYY_MM_DD:
        new_date = create_Date(token_b, token_c, token_a);
	break;

    case DD_MM_YYYY:
        new_date = create_Date(token_b, token_a, token_b);
	break;

    default:
    	fprintf(stderr,"Unknown date type: %d\n", type);
	break;

    }

    return new_date;
}

char* Date_to_string(Date date, char *delimiter, enum date_type type){
    
    char* date_string = malloc(sizeof(char)*11);
    char* day_string = malloc(sizeof(char)*3);
    char* month_string = malloc(sizeof(char)*3);
    
    if(date->day<10){
	sprintf(day_string,"0%d",date->day);
    }
    else{
	sprintf(day_string,"%d",date->day);
    }
    if(date->month<10){
	sprintf(month_string,"0%d",date->month);
    }
    else{
	sprintf(month_string,"%d",date->month);
    }

    switch(type){

        case MM_DD_YYYY:
            sprintf(date_string,"%s%s%s%s%d",month_string,delimiter,day_string,delimiter,date->year);
	    break;

	case YYYY_MM_DD:
	    sprintf(date_string,"%d%s%s%s%s",date->year,delimiter,month_string,delimiter,day_string);
	    break;

        case DD_MM_YYYY:
	    sprintf(date_string,"%s%s%s%s%d",day_string,delimiter,month_string,delimiter,date->year);

	default:
    	    fprintf(stderr,"Unknown date type: %d\n", type);
	    break;
    }

    return date_string;
}
	
void Date_add_weeks(Date date,int weeks){
    	
    while(weeks>0){
	date->day = date->day+7;
	int num_days_in_month = days_in_month(date->month,date->year);
	if(date->day>num_days_in_month){
	    date->day = date->day%num_days_in_month;
	    date->month = date->month+1;
	    if(date->month>12){
		date->month =1;
		date->year = date->year+1;
	    }
	}
	weeks--;
    }
}

int days_in_month(int month, int year){
    
    switch(month){
	case 1:
	case 3:
	case 5:
	case 7:
	case 8:
        case 10:
	case 12:
	    return 31;
	 
	case 4:
	case 6:
	case 9:
	case 11:
	    return 30;
	    
	case 2:
	    if(year%400 == 0){
		return 29;
	    }
	    else if(year%100 == 0){
		return 28;
	    }
	    else if(year%4 == 0){
		return 29;
	    }
	    else{
		return 28;
	    }
    }
}
	
	
	
