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

#ifdef DEBUG
#define PRINT_MESSAGE(cnd,fmt,...) if(cnd){fprintf(stderr,"MESSAGE: " fmt, ## __VA_ARGS__);}
#else
#define PRINT_MESSAGE(cnd,fmt,...)
#endif

#define PRINT_WARNING(fmt,...) fprintf(stderr,"*WARNING: " fmt, ## __VA_ARGS__)

#define PRINT_ERROR(fmt,...) fprintf(stderr,"**ERROR: " fmt, ## __VA_ARGS__)

#ifdef _PRINTRANDOM_
#define PRINT_RANDOM(rand) fprintf(stderr,"Random number generated: %d\n",rand)
#else
#define PRINT_RANDOM(rand)
#endif

#ifdef _PRINTQUERY_
#define PRINT_QUERY(query) fprintf(stderr,"QUERY: %s\n",query)
#else
#define PRINT_QUERY
#endif
