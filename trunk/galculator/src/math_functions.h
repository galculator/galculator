/*
 *  math_functions.h
 *	part of galculator
 *  	(c) 2002-2003 Simon Floery (chimaira@users.sf.net)
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

#ifndef _MATH_FUNCTIONS_H
#define _MATH_FUNCTIONS_H 1

double pow10y (double y);
double reciprocal (double x);
double idx (double x);
double powx2 (double x);
double factorial (double n);
double cmp (double n);
double rad2deg (double value);
double rad2grad (double value);
double deg2rad (double value);
double grad2rad (double value);
double asinh (double x);
double acosh (double x);
double atanh (double x);

#endif /* math_functions.h */
