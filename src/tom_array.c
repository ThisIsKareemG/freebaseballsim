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
#include "compiler_macros.h"

void initialize(void** list,int length)
{
        int i;
        for(i=0;i<length;i++)
        {
                list[i] = 0;
        }
}

void removeItem(void** list,void* item,int length)
{
	int i;
        int j;
        for(i=0;likely(i<length);i++)
        {
                if(list[i] == item)
                {
                        for(j=i;j<length-1;j++)
                        {
                                list[j] = list[j+1];
                        }
                        break;
                }
        }
}
/* Needs to be rewritten to work with all lists.
*/
void* removeItemAt(void** list,int index, int length)
{
	int i;
	void* item = list[index];
	for(i = index;i<(length-1);i++)
	{
		
		list[i] = list[i+1];
	}
	
	return item;
}
/*This function returns the index of an item if
the item is found in the range:  itemIndex <end.
*/
int indexOfRange(void** list, void* item, int end)
{
	int i;
	for(i=0;likely(i<end);i++)
	{	
		if(list[i] == item)
		{
			return i;
		}
	}
	return -1;
}

int indexOf(void** list, void* item,int length)
{
	return indexOfRange(list,item,length);
}
int containsItem(void** list,void* item,int length)
{
	return indexOf(list,item,length)>-1;
}
/*This function returns true if the item is found 
in the range: start >= itemIndex <end and false otherwise.
*/
int containsItemRange(void** list,void* item,int end)
{
	return indexOfRange(list,item,end)>-1;
}


