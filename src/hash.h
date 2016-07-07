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

#ifndef HASH_H
#define HASH_H

#include "linked_list.h"

typedef struct hash{
	struct linked_list **items;
	int capacity;
}*Hash;

Hash Hash_create(int capacity);
void Hash_add_key(Hash hash, void* key, void* value);
void* Hash_get_value(Hash hash, void* key);
void Hash_free(Hash hash, void (*item_free)(void *pointer));
void Hash_print(Hash hash, void (*print)(void* value));

#endif /* HASH_H */
