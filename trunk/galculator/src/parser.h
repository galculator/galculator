/*
 *  parser.h
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

#ifndef _PARSER_H
#define _PARSER_H 1

typedef struct {
	char 	*name;
	double	(*function)(double);
} s_string_func_pair;

typedef struct {
	double		result;
	gboolean	error;
} s_parser_result;

#define OPERATION_CHARS "+-*/^=()%m<>&|x"

s_parser_result parse_string(const char *input_string);

#endif	/* parser.h */
