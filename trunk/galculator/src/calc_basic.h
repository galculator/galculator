/*
 *  calc_basic.h - arithmetic precedence handling and computing in basic 
 *			calculator mode.
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
 
#ifndef _CALC_BASIC_H
#define _CALC_BASIC_H 1

#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#ifndef PROG_NAME
	#define PROG_NAME	PACKAGE
#endif

#ifndef BUG_REPORT
	#define BUG_REPORT	"Please submit a bugreport."
#endif

#define RPN_FINITE_STACK	3
#define RPN_INFINITE_STACK	-1

enum {THIS_LEVEL, LEVEL_UP, LEVEL_DOWN};

typedef struct {
	double		num;		/* numerator */
	double		denum;		/* denumerator */
} s_frac;

typedef struct {
	s_frac		real;
	s_frac		imag;
} s_complex;

typedef struct {
	double		number;
	double		(*func)(double);
	char		operation;
} s_cb_token;

typedef struct {
	double		(*func)(double);
	double		*number;
	char		*operation;
	int		size;
} s_alg_stack;

double id (double x);

double alg_add_token (s_cb_token this_token);
void alg_init (int debug_level);
void alg_free ();

void rpn_init (int size, int debug_level);
void rpn_stack_set_array (double *values, int length);
void rpn_stack_push (double number);
double rpn_stack_operation (s_cb_token current_token);
double rpn_stack_rolldown (double x);
double rpn_stack_swapxy (double x);
double *rpn_stack_get (int length);
void rpn_stack_set_size (int size);
void rpn_free ();

#endif /* calc_basic.h */
