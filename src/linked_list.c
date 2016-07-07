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
#include "linked_list.h"

struct linked_list *Linked_List_create_node(void *value)
{
	struct linked_list *new_node = malloc(sizeof(struct linked_list));
	new_node->value = value;
	new_node->next = NULL;
	new_node->prev = NULL;
	return new_node;
}

void Linked_List_append(struct linked_list **head, void *new_value)
{
	struct linked_list *temp;
	struct linked_list *new_node = Linked_List_create_node(new_value);
	
	if(*head == NULL){
		*head = new_node;
	}
	else{ 
		temp = *head;
		while(temp->next!=NULL){
			temp = temp->next;
		}
		temp->next = new_node;
	}
}

struct linked_list *Linked_List_remove_value(struct linked_list **list, void *value)
{
	struct linked_list *temp = *list;
	struct linked_list *prev = NULL;
	while(likely(temp!= NULL)){
		if (temp->value == value){
			if(prev == NULL){
				*list = (temp->next);
			}
			else{
				prev->next = temp->next;	
			}
			/*Return removed node.*/
			break;
		}
		prev = temp;
		temp = temp->next;
	}
	return temp;
}

void Linked_List_free(struct linked_list *list, void (*value_free)(void *pointer))
{
	struct linked_list *temp, *prev;
	temp = list;
	while(temp){
		if(value_free){
			value_free(temp->value);
		}
		prev = temp;
		temp = temp->next;
		free(prev);
	}
}

void Linked_List_print(struct linked_list *list, void (*print)(void *item))
{
	struct linked_list *temp = list;
	while(temp!=NULL){
		print(temp->value);
		printf(" -> ");
		temp = temp->next;		
	}
}
