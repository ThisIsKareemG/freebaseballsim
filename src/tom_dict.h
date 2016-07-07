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


Dict createDict(int keySize,int valueSize);
int getLength(Dict dict);
void daddKey(Dict dictionary,void* key,void* value);
void* dgetValue(Dict dict,void*key);
void addOrIncriment(Dict dictionary,void* item);
void dprint(Dict dictionary,void (*printkey)(void* item),void (*printvalue)(void* item));
void dsort(Dict dict);
void dremoveKeyAt(Dict dict,int index);
void Dict_set_value(Dict dict,void *key, void *value);
