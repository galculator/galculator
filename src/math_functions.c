/*
 *  math_functions.c - some mathematical functions for the calculator
 *	part of galculator
 *  	(c) 2002-2004 Simon Floery (chimaira@users.sf.net)
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Library General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */
 

#include <math.h>

#include "math_functions.h"
#include "general_functions.h"
#include "galculator.h"

#include <glib.h>		/* for G_PI etc */

double pow10y (double y)
{
	return pow (10, y);
}

double reciprocal (double x)
{
	return 1/x;
}

double idx (double x)
{
	return x;
}

double powx2 (double x)
{
	return pow (x, 2);
}

double factorial (double n)
{
	/* to avoid useless factorial computation of big numbers */
	if (n > 200) return INFINITY;
	if (n > 1) return n*factorial (n-1);
	else return 1;
}

double cmp (double n)
{
	return (double)(~((long long int)n));
}

/*
 * angle base conversions
 */

double rad2deg (double value)
{
	return (value/G_PI)*180;
}

double rad2grad (double value)
{
	return (value/G_PI)*200;
}

double deg2rad (double value)
{
	return (value/180)*G_PI;
}

double grad2rad (double value)
{
	return (value/200)*G_PI;
}

double asinh (double x)
{
	return log (x + sqrt(x*x+1));
}

double acosh (double x)
{
	return log (x + sqrt(x*x-1));
}

double atanh (double x)
{
	return log ((1+x)/(1-x))/2;
}

/* sine wrapper. interprete and convert x according to current_status.angle
 */

double sin_wrapper (double x) 
{
	return sin(x2rad(x));
}

/* arcus sine wrapper. interprete and convert result according to 
 * current_status.angle
 */

double asin_wrapper (double x) 
{
	return rad2x(asin(x));
}

/* cosine wrapper. interprete and convert x according to current_status.angle
 */

double cos_wrapper (double x) 
{
	return cos(x2rad(x));
}

/* arcus cosine wrapper. interprete and convert result according to 
 * current_status.angle
 */

double acos_wrapper (double x) 
{
	return rad2x(acos(x));
}

/* tangens wrapper. interprete and convert x according to current_status.angle
 */

double tan_wrapper (double x) 
{
	return tan(x2rad(x));
}

/* arcus tangens wrapper. interprete and convert result according to 
 * current_status.angle
 */

double atan_wrapper (double x) 
{
	return rad2x(atan(x));
}
