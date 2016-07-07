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
#include "tom_objects.h"

Linked_List create_node(void* value){
	
	Linked_List new_node = malloc(sizeof(struct linked_list));
	new_node->value = value;
	new_node->next = NULL;
	new_node->prev = NULL;
	return new_node;
}

Linked_List Linked_List_add(Linked_List head,void* new_value){
			
	printf("Beginning add function.\n");
	if(head == NULL){
		printf("Head is null.\n");
		Linked_List new_node = create_node(new_value);
		printf("Done creating node.\n");
		new_node->value = new_value;
		printf("Done setting value:%d\n",(int)new_node->value);					
		head = new_node;
		printf("Head = new_node.\n");
		return head;
	}
	else if(head->next == NULL){
		Linked_List new_node = create_node(new_value);
		new_node->value = new_value;
		new_node->prev = head;
		head->next = new_node;
		return new_node;		
	}
	else{
		return Linked_List_add(head->next,new_value);
	}
}

void Linked_List_find_head(Linked_List list){
	
	while(list->prev!=NULL){
		list = list->prev;
	}
}

Linked_List Linked_List_circular_add(Linked_List list,void* new_value){
	
	Linked_List new_node = create_node(new_value);
	new_node->value = new_value;
	if(list == NULL){		
		new_node->next = new_node;
		new_node->prev = new_node;
		list = new_node;
		return list;
	}
	else if(list->next == NULL){

		new_node->next =list->next;
		list->next = new_node;
			
		new_node->prev = list;
		new_node->next->prev = new_node;
	
	}
	return list;
}

void Linked_List_remove_value(Linked_List list,void *value){
	
	Linked_List temp = list;
	while(temp!= NULL){
		if (temp->value == value){
			temp->prev->next = temp->next;
			temp->next->prev = temp->prev;
			free(temp);
			return;
		}
	}		
}

void Linked_List_print(Linked_List list,void (*print)(void* item)){
	
	printf("first_value: %d\n",(int)list);
	int starting_value = (int)(list->value);
	printf("s_value: %d\n",starting_value);
	Linked_List temp = list;
	while(1){
		if(temp == NULL || (int)temp->value == starting_value){
			break;
		}
		print(list->value);
		temp = temp->next;
		
	}
}


int main(int argc, char** argv){
	
	Linked_List new_list = calloc(1,sizeof(struct linked_list));
	new_list->value = 5;
	Linked_List
	
}