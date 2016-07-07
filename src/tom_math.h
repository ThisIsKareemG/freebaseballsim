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
#ifndef _TOM_MATH_H
#define _TOM_MATH_H

#include <math.h>

# define M_E		2.7182818284590452354
# define M_PI		3.14159265358979323846
# define M_2_SQRTPI	1.12837916709551257390
# define M_SQRT2	1.41421356237309504880
#define TM_SQRT_PI 2 * M_2_SQRTPI

int ramdom_normal(double mean, double sd,int lowerBound,int upperBound);
int divide_ceiling(int numerator, int denominator);

#endif /*_TOM_MATH_H */
