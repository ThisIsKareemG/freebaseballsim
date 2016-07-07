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
#include <math.h>
#include <limits.h>
#include "tom_math.h"

double calculate_prob(double X, double mean, double sd)
{
	double a = pow(X-mean,2.0);
	double answer = pow(M_E,(a/(-2.0*pow(sd,2.0))));
	return answer;
}

int find_value(double randomNumber,double probArray[], int lowerBound, int upperBound)
{
#ifdef _PRINT_MATH_
	fprintf(stderr,"find_value randomNumber=%lf lower=%d upper=%d\n",randomNumber,lowerBound,upperBound);
#endif
	int index = (lowerBound+upperBound)/2;
#ifdef _PRINT_MATH_
	fprintf(stderr,"probArray[%d] = %lf\n",index,probArray[index]);
#endif
	if(randomNumber< probArray[index])
	{
		if(index == 0 || index == lowerBound){
			return lowerBound;
		}
		if(randomNumber>probArray[index-1]){
#ifdef _PRINT_MATH_
			fprintf(stderr,"find_value ret=%d\n",index);
#endif
			return index;
		}
		else{
			return find_value(randomNumber,probArray,lowerBound,index-1);
		}
	}
	else{
		return find_value(randomNumber,probArray,index+1,upperBound); 
	}
}

int random_normal(double mean, double sd,int lowerBound,int upperBound)
{
	double a = 1/ (sd * M_SQRT2 * TM_SQRT_PI);
	double valueSum = 0.0;
	double *probArray = malloc(upperBound*sizeof(double));
	double lowValue = a * calculate_prob(lowerBound * 1.0, mean, sd);
	int i;	
	for(i = lowerBound;i<upperBound;i++)
	{
		double highValue = a*calculate_prob((i+1)*1.0,mean,sd);
		double averageValue = (lowValue+highValue)/(2.0);	
		valueSum = valueSum+averageValue;		
		probArray[i] = valueSum;
		lowValue = highValue;
	}
	for(i= lowerBound;i<upperBound;i++)
	{
		probArray[i] = probArray[i]/valueSum;
	}
	double randomNumber = rand()*1.0/INT_MAX;
	int ret = find_value(randomNumber,probArray,lowerBound,upperBound);
	free(probArray);
	return ret;
}

int divide_ceiling(int numerator,int denominator){

	if(numerator % denominator == 0){
		return numerator/denominator;
	}
	else{
		return (numerator/denominator)+1;
	}
}

#ifdef _USEGSL_
#include <gsl/gsl_rng.h>
#include <gsl/gsl_randist.h>
int gsl_random_normal(gsl_rng* rng, double mean, double sd, int min, int max){
	
	double value = gsl_ran_gaussian_ziggurat (rng, sd);
	value += mean;
	int ret = (int)value;
	if(ret<=min){
		return min + 1;
	}
	if(ret>=max){
		return max - 1;
	}
	return ret;
}
#endif
