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

#ifndef _ARRAY_LIST_H
#define _ARRAY_LIST_H

typedef struct array_list{
	int length;
	int capacity;
	int itemSize;
	void** items;
}*Array_List;

Array_List Array_List_create(int itemSize);
void gincreaseCapacity(Array_List list);
void gset(Array_List list,int index,void* item);
void* gget(Array_List list,int index);
void gremoveItem(Array_List list, void* item);
void* gremoveItemAt(Array_List list,int index);
void Array_List_add(Array_List list, void* item);
int gcontains(Array_List list, void* item);
int gindexOf(Array_List list,void* item);
void** gtoArray(Array_List list);
void gprint(Array_List list, void (*print)(void* item));
void ginsertItem(Array_List list,int index,void* item);
void Array_List_free(Array_List list, int free_items);

#endif /* _ARRAY_LIST_H */
