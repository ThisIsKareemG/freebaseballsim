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
#include "compiler_macros.h"
#include "tom_objects.h"
#include "tom_dict.h"
#include "tom_array_list.h"
#include "tom_print.h"

Dict createDict(int keySize,int valueSize)
{
	int initialCapacity = 8;
	Dict newDict = (Dict)malloc(sizeof(struct dict));
// 	Array_List keys = Array_List_create(keySize);
// 	Array_List values = Array_List_create(valueSize);
// 	newDict->keys = keys;
// 	newDict->values = values;
	newDict->keys = Array_List_create(keySize);
	newDict->values = Array_List_create(valueSize);
	newDict->capacity = initialCapacity;
	return newDict;
}

int getLength(Dict dict)
{
	return dict->keys->length;
}

/* need to check if key is already in dictionary.
*/
void daddKey(Dict dictionary,void* key,void* value)
{
	Array_List_add(dictionary->keys,key);
	Array_List_add(dictionary->values,value);
}
	
void moveKeyToEnd(Dict dict,void* key)
{
	int value;
	int index = gindexOf(dict->keys,key);
	gremoveItemAt(dict->keys,index);
	value = (int)gremoveItemAt(dict->values,index);
	daddKey(dict,key,(void*)value);
}
void dremoveKeyAt(Dict dict,int index)
{
	gremoveItemAt(dict->keys,index);
	gremoveItemAt(dict->values,index);
}
void* dgetValue(Dict dict,void* key)
{
	int index = gindexOf(dict->keys,key);
	if(index!=-1)
	{	
		return gget(dict->values,index);
	}
	else
	{
//		fprintf(stderr,"Key not in dictionary.\n");
		return NULL;
	}
}	
/*
NOTE: This function only works if the values are all integers.

This function will add a new item to the dictionary if
the item is not in the dictionary.  If the item is in the dictionary,
it will incriment its value.
*/		
void addOrIncriment(Dict dictionary,void* item)
{
	int value;
	int index = gindexOf(dictionary->keys,item);
	if(index>-1)
	{
//		printf("incrimenting value\n");
		value = (int)gget(dictionary->values,index);
		gset(dictionary->values,index,(void*)value+1);
	}
	else
	{
		daddKey(dictionary,item,(void*)1);
	}

}
/*This function prints out a Dict, it takes as its arguments to print(void* item)
functions, one to print the keys, and one to print the values.
*/
void dprint(Dict dictionary,void (*printkey)(void* item),void (*printvalue)(void* item))
{
	int i;
	printf("Length: %d\n",getLength(dictionary));
	printf("Capacity: %d\n",dictionary->capacity);
	for(i=0;i<getLength(dictionary);i++)
	{
		printf("Key %d: <",i);
		printkey(gget(dictionary->keys,i));
		printf(",");
		printvalue(gget(dictionary->values,i));
		printf(">\n");
	}
}
/*only works with values that are numbers.
This function needs to be expanded when I have more time.
*/
void dsort(Dict dict)
{
	int change =1;
	int i=0;
	while(change)
	{
		change = 0;
		
		while(i<getLength(dict)-1)
		{
			if(gget(dict->values,i)<gget(dict->values,i+1))
			{
				moveKeyToEnd(dict,gget(dict->keys,i));
				change =1;
			}
			else
			{
				i++;
			}	
		}
		i=0;
		
	
	}
	//dprint(dict,printChar,printInt);
}

void Dict_set_value(Dict dict,void *key, void *value){
	
	int index_of_key = gindexOf(dict->keys,key);
	gset(dict->values,index_of_key,value);
}
