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

#ifndef LINKED_LIST_H
#define LINKED_LIST_H

struct linked_list{
	void *value;
	struct linked_list *next;
	struct linked_list *prev;
};

struct linked_list *Linked_List_create_node(void *value);
void Linked_List_append(struct linked_list **head, void *new_value);
struct linked_list *Linked_List_remove_value(struct linked_list **list,void *value);
void Linked_List_free(struct linked_list *list, void (*value_free)(void *pointer));
void Linked_List_print(struct linked_list *list, void (*print)(void *item));

#endif /* LINKED_LIST_H */
