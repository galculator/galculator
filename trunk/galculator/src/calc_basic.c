/*
 *  calc_basic.c - arithmetic precedence handling and computing in basic 
 *			calculator mode.
 *	part of galculator
 *  	(c) 2002-2003 Simon Floery (simon.floery@gmx.at)
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
 *
 * this is calc_basic version 2.
 */
 
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <glib.h>
#include <math.h>

#include "calc_basic.h"

/* i18n */

#include <libintl.h>
#define _(String) gettext (String)
#define gettext_noop(String) String
#define N_(String) gettext_noop (String)

char 	*operator_precedence[] = {"=)", "+-&|x", "*/<>%", "^", "(", "%", NULL};
char 	*right_associative = "^";

s_alg_stack	*current_stack=NULL;
GArray		*rpn_stack;
GPtrArray	*stackstack;
int		alg_debug = 0, rpn_debug = 0;

/*
 * GENERAL STUFF
 */

/* id. The identity function. This is used as stack function, if none was given.
 */

double id (double x)
{
	return x;
}

/* debug_input. debug code: enter tokens on stdin.
 */

void debug_input ()
{
	char		input[20];
	s_cb_token	current_token;
	
	scanf ("%s", input);
	current_token.number.value = atof (input);
	scanf ("%s", input);
	current_token.operation = input[0];
	if (current_token.operation == '(') current_token.number.func = id;
	printf ("\t\tdisplay value: %f\n", alg_add_token (current_token));
}

/* reduce. op1 is before op2 in the computation.
 */

static int reduce (char op1, char op2)
{
	int	counter = 0, p1 = 0, p2 = 0;
	
	while (operator_precedence[counter] != NULL) {
		if (strchr (operator_precedence[counter], op1) != NULL)
			p1 = counter;
		if (strchr (operator_precedence[counter], op2) != NULL)
			p2 = counter;
		counter++;
	}
	if (p1 < p2) return FALSE;
	// associativity only makes sense for same operations
	else if ((op1 == op2) && (strchr (right_associative, op1) != NULL)) 
		return FALSE;
	else return TRUE;
}

/* compute_expression. here, the real copmputation is done.
 */

static double compute_expression (double left_hand, 
				char operator, 
				double right_hand)
{
	double	result;
	
	switch (operator) {
	case '+':
		result = left_hand + right_hand;
		break;
	case '-':
		result = left_hand - right_hand;
		break;
	case '*':
		result = left_hand * right_hand;
		break;
	case '/':
		result = left_hand / right_hand;
		break;
	case '^':
		result = pow (left_hand, right_hand);
		break;
	case '<':
		/* left shift x*2^n */
		result = ldexp (left_hand, (int) floor(right_hand));		
		break;
	case '>':
		result = ldexp (left_hand, ((int) floor(right_hand))*(-1));
		break;
	case 'm':
		result = fmod (left_hand, right_hand);
		break;
	case '&':
		result = (long long int)left_hand & (long long int) right_hand;
		break;
	case '|':
		result = (long long int)left_hand | (long long int) right_hand;
		break;
	case 'x':
		result = (long long int)left_hand ^ (long long int) right_hand;
		break;
	case '%':
		result = left_hand * right_hand/100;
		break;
	default: 
		if (alg_debug+rpn_debug > 0) fprintf (stderr, _("[%s] %c - unknown operation \
character. %s\n"), PROG_NAME, operator, BUG_REPORT);
		result = left_hand;
		break;
	}	
	if (alg_debug + rpn_debug > 0) fprintf (stderr, _("[%s] computing: %f %c %f = %f\n"), 
		PROG_NAME, left_hand, operator, right_hand, result);
	return result;
}

/*
 * (PSEUDO)ALGEBRAIC MODE
 */

/* alg_stack_new. we are working with stacks. every bracket layer is a single 
 * stack. this function creates a new stack.
 */

static s_alg_stack *alg_stack_new (s_cb_token this_token)
{
	s_alg_stack	*new_stack;
	
	new_stack = (s_alg_stack *) malloc (sizeof(s_alg_stack));
	new_stack->func = this_token.number.func;
	new_stack->number = NULL;
	new_stack->operation = NULL;
	new_stack->size = 0;
	return new_stack;	
}

/* alg_stack_append. simply appends token (number and operation) to stack.
 * that's all (no computation etc!).
 */

static void alg_stack_append (s_alg_stack *stack, s_cb_token token)
{
	stack->size++;
	stack->number = (double *) realloc (stack->number, 
		stack->size * sizeof(double));
	stack->number[stack->size-1] = token.number.value;
	stack->operation = (char *) realloc (stack->operation,
		stack->size * sizeof(char));
	stack->operation[stack->size-1] = token.operation;
}

/* alg_stack_pool. do as many computation as possible with respect to
 * precedence. here, reduce from above is used.
 */

static double alg_stack_pool (s_alg_stack *stack)
{
	int	index;
	
	index = stack->size - 1;
	while ((index >= 1) && (reduce(stack->operation[index-1], 
		stack->operation[index]))) {
			stack->number[index - 1] = compute_expression (
				stack->number[index - 1],
				stack->operation[index - 1],
				stack->number[index]);
			stack->operation[index - 1] = stack->operation[index];
			index--;
	}
	if (stack->size != (index + 1)) {
		stack->size = index + 1;
		stack->number = (double *) realloc (stack->number,
			sizeof(double) * stack->size);
		stack->operation = (char *) realloc (stack->operation, 
			sizeof(char) * stack->size);
	}
	return stack->number[stack->size-1];
}

/* alg_stack_free. the stack destructor.
 */

static void alg_stack_free (s_alg_stack *stack)
{
	free (stack->number);
	free (stack->operation);
	free (stack);
}

/* alg_add_token. call this from outside. 
 */

double alg_add_token (s_cb_token this_token)
{
	static double	return_value;
	
	switch (this_token.operation) {
	case '(':
		g_ptr_array_add (stackstack, current_stack);
		if (this_token.number.func == NULL)
			this_token.number.func = id;
		current_stack = alg_stack_new (this_token);
		break;
	case ')':
		if (stackstack->len < 1) break;
		alg_stack_append (current_stack, this_token);
		return_value = current_stack->func (
			alg_stack_pool (current_stack));
		alg_stack_free (current_stack);
		current_stack = g_ptr_array_remove_index (stackstack, 
			stackstack->len - 1);
		break;
	default:
		alg_stack_append (current_stack, this_token);
		return_value = alg_stack_pool (current_stack);
		if (this_token.operation == '=') {
			alg_free();
			alg_init(alg_debug);
		}
		break;
	}
	return return_value;
}

/* alg_init. use this from outside to initialize everything
 */

void alg_init (int debug_level)
{
	s_cb_token	token;
	
	alg_debug = debug_level;
	stackstack = g_ptr_array_new ();
	token.number.func = id;
	current_stack = alg_stack_new(token);	
}

/* alg_free. call this from outside to clean up properly
 */

void alg_free ()
{
	if (!current_stack) alg_stack_free (current_stack);
	if (!stackstack) g_ptr_array_free (stackstack, TRUE);
}

/*
 * RPN
 */

void rpn_init (int debug_level)
{
	rpn_stack = g_array_new (FALSE, FALSE, sizeof(double));
	rpn_debug = debug_level;
}

void rpn_stack_push (double number)
{
	rpn_stack = g_array_append_val (rpn_stack, number);
	if (rpn_debug > 0) fprintf (stderr, _("[%s] RPN stack size is %i.\n"), 
		PROG_NAME, rpn_stack->len);
}

double rpn_stack_operation (s_cb_token current_token)
{
	double	return_value;
	double	left_hand;
	
	/* this function only serves binary operations. therefore, we need at 
	 * least one element on the stack. if this is not the case, work with 0.
	 */
	if (rpn_stack->len < 1) left_hand = 0;
	else {
		/* retrieve left_hand from stack */
		left_hand = g_array_index (rpn_stack, double, 
			rpn_stack->len - 1);
		rpn_stack = g_array_remove_index (rpn_stack, 
			rpn_stack->len - 1);
	}
	/* compute it */
	return_value = compute_expression (left_hand, current_token.operation, 
		current_token.number.value);
	if (rpn_debug > 0) fprintf (stderr, _("[%s] RPN stack size is %i.\n"), 
		PROG_NAME, rpn_stack->len);
	return return_value;
}

void rpn_free ()
{
	if (!rpn_stack) g_array_free (rpn_stack, TRUE);
}

/*
int main (int argc, char *argv[])
{
	alg_init(1);
	while (1) debug_input();
	return 1;
}
*/
