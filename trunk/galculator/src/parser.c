/*
 *  parser.c - string parser to feed calc_basic.c
 *	part of galculator
 *  	(c) 2002-2004 Simon Floery (chimaira@users.sf.net)
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
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
 
/*
 * compile with:
 * gcc calc_basic.c `pkg-config --cflags --libs glib-2.0` -Wall -lm
 */
 
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <math.h>
#include <glib.h>

#include "parser.h"
#include "calc_basic.h"
#include "galculator.h"

/* is_operation. determines on behalf of current and previous character if 
 * current one is an operation.
 */
 
static gboolean is_operation (char operation_char, char last_char)
{
	/* '-' could also be part of a signed number */
	if (operation_char == '-')
		return (strchr(OPERATION_CHARS, last_char) == NULL) || 
			(last_char == ')');
	else
		return !(strchr(OPERATION_CHARS, operation_char) == NULL);
}

/* remove_whitespaces. remove any whitespaces from input. returns new string
 * length.
 */

int remove_whitespaces (char *string)
{
	int 	string_counter=0, new_string_counter=0;
	
	while (string[string_counter] != '\0') {
		if (!isspace(string[string_counter])) {
			string[new_string_counter] = string[string_counter];
			new_string_counter++;
		}
		string_counter++;
	}
	string[new_string_counter] = '\0';
	return new_string_counter;
}

/* identify_function. tries to match given string to a function
 */

static double (*identify_function (char *string))(double)
{
	int 			counter=0;
	s_string_func_pair 	map[16] = {
		{"sin", sin},
		{"cos", cos},
		{"tan", tan},
		{"ln", log},
		{"log", log10},
		{"sqrt", sqrt},
		{"asin", asin},
		{"acos", acos},
		{"atan", atan},
		{"sinh", sinh},
		{"cosh", cosh},
		{"tanh", tanh},
		{"asinh", asinh},
		{"acosh", acosh},
		{"atanh", atanh},
		{NULL, NULL}};
	
	while ((map[counter].function != NULL) && 
		(strcmp (map[counter].name, string) != 0)) counter++;
	return map[counter].function;
}

/* identify_constant. check, if string is a constant.
 */

static char *identify_constant (char *string)
{
	int 	counter = 0;
	
	while (constant[counter].name != NULL) {
		if (strcmp (constant[counter].name, string) == 0)
			return constant[counter].value;
		counter++;
	}
	return NULL;
}

/* identify_number. number refers here to anything not an operation. number may
 * be an empty string, a function or really a number.
 */

static u_number identify_number (char *string, char current_operation, double default_value)
{
	char 		*endptr;
	char		*constant_string;
	u_number	return_value;
	
	if (strlen(string) == 0) {
		if (current_operation == '(') {
			return_value.func = NULL;
			return return_value;
		}
		return_value.value = default_value;
		return return_value;
	}
	/* now: at first, let's have a look if it's a function */
	if ((return_value.func = identify_function (string)) == NULL) {
		/* then, if it's a constant */
		if ((constant_string = identify_constant (string)) != NULL)
			return_value.value = strtod (constant_string, &endptr);
		/* and last if it is a number */
		else return_value.value = strtod (string, &endptr);
		if ((endptr) && (*endptr != '\0')) 
			fprintf (stderr, "error [id_nr] : %s\n", string);
	}
	return return_value;
}

/* parse_string. use this function to parse an algebraic expression in order
 * to get the result.
 */

double parse_string(const char *input_string)
{
	int 		counter, string_length;
	char 		*number, *string, last_char=0;
	s_cb_token	current_token;
	double 		result=0.;
	
	string = g_strdup_printf ("%s=", input_string);
	string_length = remove_whitespaces (string);
	number = string;
	for (counter = 0; counter < string_length; counter++) {
		/* current character is an operation BUT not a minus sign 
		 * succeeding another operation
		 */
		if (is_operation(string[counter], last_char)) {
			current_token.operation = string[counter];
			last_char = string[counter];
			string[counter] = '\0';
			current_token.number = identify_number (number, 
				current_token.operation, result);
			result = alg_add_token (current_token);
			number = &(string[counter + 1]);
		} else {
			last_char = string[counter];
		}
	}
	g_free (string);
	return result;
}

/*
int main (int argc, char *argv[])
{
	char 	*string2parse;
	
	string2parse = (char *) malloc (256*sizeof(char));
	alg_init (2);
	do {
		fscanf (stdin, "%s", string2parse);
		printf ("---> Result is %f\n", parse_string (string2parse));
	} while (strlen(string2parse) > 1);
	return 0;
}
*/
