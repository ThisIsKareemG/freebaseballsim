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
#include "linked_list.h"
#include "key_value.h"
#include "hash.h"


Hash Hash_create(int capacity){
	
	Hash hash = malloc(sizeof(struct hash));
	hash->capacity = capacity;
	hash->items = calloc(capacity, sizeof(Key_Value*));
	return hash;
}
/*--------------*/
/*HASH ALGORITHS*/
/*--------------*/

int simple_hash(void* ptr,int capacity){	
	
	return (int)ptr%capacity;
}

void Hash_add_key(Hash hash, void* key, void* value){
	
	int index = simple_hash(key,hash->capacity);
	Key_Value key_value_pair = malloc(sizeof(struct key_value));
	key_value_pair->key = key;
	key_value_pair->value = value;
	if(hash->items[index] == NULL){
		hash->items[index] = Linked_List_create_node(key_value_pair);
	}
	else{
		Linked_List_append(hash->items + index,key_value_pair);
	}	
}

void* Hash_get_value(Hash hash, void* key){
	
	int index = simple_hash(key,hash->capacity);
	struct linked_list *temp = hash->items[index];
	while(temp!=NULL){
		if(key == ((Key_Value)temp->value)->key){
			return ((Key_Value)temp->value)->value;
		}
		temp = temp->next;
	}
	return NULL;
}

void Hash_set_value(Hash hash,void* key, void* value){
	int index = simple_hash(key,hash->capacity);
	struct linked_list *temp = hash->items[index];
	while(temp!=NULL){
		if(key == ((Key_Value)temp->value)->key){
			((Key_Value)temp->value)->value = value;
			return;
		}
		temp = temp->next;
	}
	fprintf(stderr,"Hash_set_value: Key not found in hash.\n");	
}

void Hash_free(Hash hash, void (*item_free)(void *pointer)){
	int i = 0;
	struct linked_list *temp, *prev;
	Key_Value kv;
	for(i=0; i< hash->capacity; i++){
		temp = hash->items[i];
		while(temp){
			kv = temp->value;
			if(item_free){
				item_free(kv->value);
			}
			free(kv);
			prev = temp;
			temp = temp->next;
			free(prev);
		}
	}
	free(hash->items);
	free(hash);
}

void Hash_print(Hash hash, void (*print)(void* value)){
	
	int i;
	for(i=0;i<hash->capacity;i++){
		
		if(hash->items[i] == NULL){
			continue;
		}
		printf("Hash: %d values: ",i);
		Linked_List_print(hash->items[i],print);
		printf("\n");
	}	
	
}
