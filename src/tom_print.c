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

#include<stdio.h>
#include "tom_print.h"

void print_int(void* item)
{
	printf("%d",(int)item);
}

void printChar(void* item)
{
	printf("%c",(int)item);
}

void printString(void* item)
{
	printf("%s\n",(char*)item);
}
void printIntArray(int* item,int length){
	
	int i;
	for(i=0;i<length;i++){
		printf("%d ",item[i]);
	}
	printf("\n");
}
