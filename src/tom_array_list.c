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
#include "compiler_macros.h"
#include "tom_array_list.h"
#include "tom_array.h"
#include "tom_print.h"

/*This function allocates memory for and returns a new Array_List.
NOTE: the suggested item size is 4 or "sizeof(int)" many of the functions 
won't work with items that have a size less than 4, this means don't
try to make a Array_List of characters it won't work.
*/
Array_List Array_List_create(int itemSize)
{
	int initialCapacity = 8;
	Array_List newList = (Array_List)malloc(sizeof(struct array_list));
	newList->length = 0;
	newList->capacity = initialCapacity;
	newList->itemSize = itemSize;
	newList->items = calloc(newList->capacity,itemSize);
	return newList;
}
/* This function doubles the capacity of an existing
Array_List and allocates the appropriate space.
*/
void gincreaseCapacity(Array_List list)
{
	list->capacity = list->capacity << 1;
	list->items = realloc(list->items,list->capacity*sizeof(void*));
}

void gset(Array_List list,int index,void* item)
{
	list->items[index] = item;
}	

void* gget(Array_List list,int index)
{
	return list->items[index];
}	

void gremoveItem(Array_List list, void* item)
{
	removeItem(list->items,item,list->length);
	list->length--;
}

void* gremoveItemAt(Array_List list,int index)
{
	void* item = removeItemAt(list->items,index,list->length);
	list->length--;
	return item;
}
/* This function adds an item to the end of a the list.
*/
void Array_List_add(Array_List list, void* item)
{
	if(list->length == list->capacity){
		gincreaseCapacity(list);
	}
	list->items[list->length] = item;
	list->length++;	
}

int gcontains(Array_List list,void* item)
{
	return containsItem(list->items,item,list->length);
}

int gindexOf(Array_List list,void* item)
{
	return indexOf(list->items,item,list->length);
}
/* This function returns a Array_List's array of items.
 * This is untested.
 */
void** gtoArray(Array_List list)
{
	int i;
	void** array = malloc(list->length*sizeof(int));
	for(i=0;i<list->length;i++)
	{
		
		array[i] = gget(list,i);
	}
	return array;
}
/*
This prints out an entire Array_List.
It takes a function print(void* item) as an argument which prints out
the items of the list.
*/
void gprint(Array_List list,void (*print)(void* item))
{
	int i;
	fprintf(stderr,"Length: %d\n",list->length);
	fprintf(stderr,"Capacity: %d\n",list->capacity);
	for(i=0;i<list->length;i++)
	{
		fprintf(stderr,"Item %d:\n",i);
		print(list->items[i]);
	}

}

void ginsertItem(Array_List list,int index,void* item)
{
	int i;
	if(list->length == list-> capacity)
	{
		gincreaseCapacity(list);
	}
	for(i=list->length-1;i>=index;i--)
	{
		list->items[i+1] = list->items[i];
	}
	list->items[index] = item;
	list->length++;
}

void Array_List_clear(Array_List list,int free_items){
	int i;
	if(free_items){
		for(i=0;i<list->length;i++){
			free(list->items[i]);
		}
	}
	memset(list->items,0,sizeof(void*) * list->length);
	list->length = 0;
}

void Array_List_free(Array_List list,int free_items){
	
	if(free_items){
		int i;
		for(i=0;i<list->length;i++){
			free(list->items[i]);
		}
	}
	free(list->items);
	free(list);
}
		
	
