/*
 *  calc_basic.h - arithmetic precedence handling and computing in basic 
 *			calculator mode.
 *	part of galculator
 *  	(c) 2002-2003 Simon Floery (simon.floery@gmx.at)
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

enum {LEFT_CHILD, RIGHT_CHILD};

enum {THIS_LEVEL, LEVEL_UP, LEVEL_DOWN};

typedef union {
	double		value;
	double		(*function)(double);
} u_number;

typedef struct {
//	u_number	number;
	double		number;
	char		operator;
} s_calc_token;

void calc_tree_init (int debug_level);
void calc_tree_free ();
void calc_tree_free_stack ();
double calc_tree_add_token (s_calc_token current_token);

void calc_rpn_init (int debug_level);
void calc_rpn_free ();
void calc_rpn_stack_add (double number);
double calc_rpn_stack_operation (s_calc_token current_token);

#endif /* calc_basic.h */
