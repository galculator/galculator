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
 * compile with: gcc calc_basic.c `pkg-config --cflags --libs glib-2.0` -Wall -lm
 *
 * for further information see DOC_CALC_TREE
 *
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

char 	*operators=NULL;
char 	*operator_precedence[] = {"=", "+-&|x", "*/<>%", "^", "()", NULL};
char 	*right_associative = "^";

/* current_node is NOT on the stack */

GPtrArray	*sub_tree_stack;
GArray		*rpn_stack;

static int 	debug = 0;

/**
 ** DEBUG CODE
 **/

/*
 * calc_debug_input - This function reads doubles and chars from stdin, until 
 * an operation character is given.
 */

void calc_debug_input ()
{
	char		input[20]="0.";
	s_calc_token	current_token;

	do {
		current_token.number = atof (input);
		scanf ("%s", input);
	} while (strchr (operators, input[0]) == NULL);
	current_token.operator = input[0];
	printf ("\t\tdisplay value: %f\n", calc_tree_add_token (current_token));
}

/*
 * calc_debug_computing - asks the user for help. print the operation to 
 * stdout and get the result from stdin.
 */

void *calc_debug_computing (GNode *node)
{
	double		*first_number, *second_number;
	double 		*result;
	char		*operator;
	
	result = (double *) malloc (sizeof(double));
	first_number = (g_node_nth_child(node, LEFT_CHILD))->data;
	second_number = (g_node_nth_child(node, RIGHT_CHILD))->data;
	operator = node->data;
	printf ("[help me] %f %c %f = ", *first_number, *operator,
		*second_number);
	scanf ("%lf", result);
	return result;
}

/*
 * calc_debug_display_vertices - goes recursivly through the whole tree and 
 * outputs the vertices (floats and operations). The output is a normal 
 * expression string representing the operations stored in the tree.
 */

void calc_debug_display_vertices (GNode *node)
{
	char	*operator;
	double	*number;
	
	if (g_node_nth_child (node, LEFT_CHILD) != NULL ) \
		calc_debug_display_vertices (g_node_nth_child (node, \
		LEFT_CHILD));
	if (G_NODE_IS_LEAF (node)) {
		number = node->data;
		printf ("%f ", *number);
	} else {
		operator = node->data;
		printf ("%c ", *operator);
	}
	
	if (g_node_nth_child (node, RIGHT_CHILD) != NULL ) \
		calc_debug_display_vertices (g_node_nth_child (node, \
		RIGHT_CHILD));
}

/*
 * calc_debug_display_tree - initiates the call of the recursive function 
 * calc_debug_display_vertices.
 */

void calc_debug_display_tree (GNode *root)
{
	putchar ('\t');
	calc_debug_display_vertices (root);
	putchar ('\n');
}

/**
 ** PRECEDENCE HANDLING CODE
 **/

/*
 * get_tree_level - this function is part of the precedence handling code. 
 * Remember the calc_tree documentation data structure, where we stated that 
 * parentheses can be regarded as a new sub tree in the precedence tree.
 * This function looks at the operator and decides, whether to start 
 * (LEVEL_UP) a new sub tree, to close the current sub tree (LEVEL_DOWN) or 
 * to stay where we are (THIS_LEVEL).
 */

int get_tree_level (char operation)
{
	if (operation == '(') return LEVEL_UP;
	else if (operation == ')') return LEVEL_DOWN;
	else return THIS_LEVEL;
}

/*
 * precedence_level - this function is part of the precedence handling code. 
 * The string array operator_precedence	contains all known operators sorted 
 * after their precedence (lowest precedence comes first). This functions
 * simply scans the array and returns the level where the current operator i
 * was found.
 */

int precedence_level (char operator)
{
	int     counter=0;

	while (operator_precedence[counter] != NULL) {
    		if (strchr(operator_precedence[counter], operator) != NULL) \
			return counter;
		counter++;
    	}
	return -1;
}

/*
 * append2node - this function is part of the precedence handling code. When
 * inserting a new operation in a calc_tree, we have to decide either to 
 * append the operation to the current node or to go up until a good place is
 * found. This function makes this decision based on the two operators 
 * regarding precedence_level() and assosiativity.
 */

int append2node (char node_operator, char current_operator)
{
	if (precedence_level(current_operator) > precedence_level (node_operator)) \
		return TRUE;
	else if ((precedence_level(current_operator) == precedence_level (node_operator)) \
		&& (strchr (right_associative, current_operator) != NULL)) return TRUE;
	return FALSE;
}

/**
 ** COMPUTING CODE
 **/

/*
 * calc_expr_computing - the real calculator. computes the given expression.
 */

void *calc_expr_computing (double left_hand, char operator, double right_hand)
{
	double 		*result;
	
	result = (double *) malloc (sizeof(double));
	switch (operator) {
	case '+':
		*result = left_hand + right_hand;
		break;
	case '-':
		*result = left_hand - right_hand;
		break;
	case '*':
		*result = left_hand * right_hand;
		break;
	case '/':
		*result = left_hand / right_hand;
		break;
	case '^':
		*result = pow (left_hand, right_hand);
		break;
	case '<':
		*result = ldexp (left_hand, (int) floor(right_hand));		/* left shift x*2^n */
		break;
	case '>':
		*result = ldexp (left_hand, ((int) floor(right_hand))*(-1));
		break;
	case '%':
		*result = fmod (left_hand, right_hand);
		break;
	case '&':
		*result = (long long int) left_hand & (long long int) right_hand;
		break;
	case '|':
		*result = (long long int) left_hand | (long long int) right_hand;
		break;
	case 'x':
		*result = (long long int) left_hand ^ (long long int) right_hand;
		break;
	default: 
		if (debug > 0) fprintf (stderr, _("[%s] %c - unknown operation character. %s\n"), PROG_NAME, operator, BUG_REPORT);
		break;
	}	
	if (debug > 0) fprintf (stderr, _("[%s] computing: %f %c %f = %f\n"), PROG_NAME, left_hand, operator,\
		right_hand, *result);
	return result;
}


/*
 * calc_node_computing - front-end to the real calculator. takes a node as argument, this
 * node has to point to a binary operation, the operands are expected to be 
 * the left and the right child. does all the operations stored in a calc_tree.
 */

void *calc_node_computing (GNode *node)
{
	double		*first_number, *second_number;
	char		*operator;
		
	first_number = (g_node_nth_child(node, LEFT_CHILD))->data;
	second_number = (g_node_nth_child(node, RIGHT_CHILD))->data;
	operator = node->data;
	return calc_expr_computing (*first_number, *operator, *second_number);
}

/**
 ** CALC_TREE CODE
 **/

/*
 * calc_tree_new - the first token starts a new tree.
 */

GNode *calc_tree_new (s_calc_token current_token)
{
	GNode		*root;
	double		*number;
	char		*operator;
	
	number = (double *) malloc (sizeof(char));
	*number = current_token.number;
	operator = (char *) malloc (sizeof(char));
	*operator = current_token.operator;
	root = g_node_new (operator);
	g_node_insert_data (root, LEFT_CHILD, number);
	return root;
}

/*
 * calc_update_tree - this function takes a token as argument, inserts this
 * token with regard to precedence and associativity into the calc_tree and 
 * does all the computing possible at this moment.
 */

GNode *calc_update_tree (GNode *current_node, s_calc_token current_token)
{	
	GNode		*calc_end_node, *sub_root, *new_node;
	char		*current_node_operator, *next_operator, *current_operator;
	double		*current_number;
	
	/* make static copies of the values */
	current_number = (double *) malloc (sizeof(char));
	*current_number = current_token.number;
	current_operator = (char *) malloc (sizeof(char));
	*current_operator = current_token.operator;
	current_node_operator = current_node->data;
	if (!append2node (*current_node_operator, current_token.operator)) {
		g_node_insert_data (current_node, RIGHT_CHILD, current_number);
		new_node = g_node_new (current_operator);
		calc_end_node = current_node;
		if (!G_NODE_IS_ROOT (current_node)) {
			next_operator = (calc_end_node->parent)->data;
			while (!append2node(*next_operator, current_token.operator)) {
			/*while (precedence_level (*next_operator) >= precedence_level (current_token.operator))*/
				calc_end_node = calc_end_node->parent;
				if (G_NODE_IS_ROOT(calc_end_node)) break;
				else next_operator = (calc_end_node->parent)->data;
			}
		}
		/* calc_end_node is the future left child of new_node */
		if (!G_NODE_IS_ROOT(calc_end_node)) {
			sub_root = calc_end_node->parent;
			g_node_unlink (calc_end_node);
			g_node_insert (sub_root, RIGHT_CHILD, new_node);
		}
		g_node_insert (new_node, LEFT_CHILD, calc_end_node);
		/* now, the tree is up2date */
		while (current_node != calc_end_node) {
			current_node->data = calc_node_computing (current_node);
			current_node = current_node->parent;
		}
		current_node->data = calc_node_computing (current_node);
		g_node_destroy (g_node_nth_child (current_node, RIGHT_CHILD));
		g_node_destroy (g_node_nth_child (current_node, LEFT_CHILD));
		current_node = current_node->parent;
	} else {
		/* here, no computation is possible */
		g_node_insert_data (current_node, RIGHT_CHILD, current_operator);
		current_node = g_node_nth_child (current_node, RIGHT_CHILD);
		g_node_insert_data (current_node, LEFT_CHILD, current_number);
	}
	return current_node;
}

/* 
 * calc_tree_add_token - every time the GUI has a valid token, it feeds it to this function
 * (to our data structures). you might assume (and you are right) that this function could be
 * done in a recursive way. but then we have to run it within an infinite loop and no GUI
 * could run beneath (in the same process). 
 * this code handles parenthesis tasks.
 * calc_tree_add_token returns the value one might to put on a display - the left child
 * of the current node.
 * return value is declared static for case LEVEL_UP.
 */
double calc_tree_add_token (s_calc_token current_token)
{
	/* define current_node not null. necessary for first create_new_sub_tree in THIS_LEVLE */
	static GNode		*current_node=(GNode *)NULL+1;
	static double		return_value;
	double				*dummy_double;
	static gboolean		create_new_sub_tree = TRUE;
	
	switch (get_tree_level (current_token.operator)) {
	case THIS_LEVEL:
		/* we have to do nothing to the stack */
		/* if we have to create a new sub tree ...*/
		if (create_new_sub_tree == TRUE) {
			current_node = calc_tree_new (current_token);
			create_new_sub_tree = FALSE;
		} else {
			/* or there is a tree. */
			current_node = calc_update_tree (current_node, current_token);
		}
		if (debug > 0) calc_debug_display_tree (g_node_get_root(current_node));
		/* fetching the return value */
		dummy_double = (g_node_nth_child (current_node, LEFT_CHILD))->data;
		return_value = *dummy_double;
		/* '=' is our "finalizer" */
		if (current_token.operator == '=') {
			/* current_node is root! */
			g_node_destroy (current_node);
			calc_tree_free_stack();
			create_new_sub_tree = TRUE;
		}
		break;
	case LEVEL_UP:
		if (create_new_sub_tree == TRUE) {
			/* If create_new_sub_tree is true, return_value is set to zero to update the display */
			if (sub_tree_stack->len == 0) {
				return_value = 0;
				current_node = NULL;
			}
			/* put a NULL onto the stack. this is for signaling LEVEL_DOWN, that this level has no tree yet. */
			g_ptr_array_add (sub_tree_stack, NULL);
		} else {
			/* save current_node to stack */
			g_ptr_array_add (sub_tree_stack, current_node);
		}
		create_new_sub_tree = TRUE;
		break;
	case LEVEL_DOWN:
		/* ignore if there was no "open parenthesis" */
		if (sub_tree_stack->len < 1) return return_value;

		/* if create_new_sub_tree, there is no operation to be done in this 
		 *	"virtual" subtree (it has never been created). This happens for an
		 *	expression like 1/(5). Just return current_token.number
		 */
		if (create_new_sub_tree == TRUE) {
			create_new_sub_tree = FALSE;
			return current_token.number;
		}

		/* finish the current sub tree (if there is one != NULL).
		 * set current operator to operator at (0,0) - this one has lowest precedence. 
		 * so it becomes new root of sub tree but won't get computed. we are 
		 * only interesting in its left child. current_node is root! 
		 */
		if (current_node != NULL) {
			current_token.operator = operator_precedence[0][0];
			current_node = calc_update_tree (current_node, current_token);
			if (debug > 0) calc_debug_display_tree (g_node_get_root(current_node));
		
			/* now take left child and let calc_tree_add_token remember this value */
			dummy_double = (g_node_nth_child (current_node, LEFT_CHILD))->data;
			return_value = *dummy_double;
			/* actually, we should remember the value of this sub tree. BUT we return it, the return_value is 
			 * put onto the display and current_token.number is set to the current contence of the display
			 * on operation button entry. so we remember the value indeed; but not straightforward.
			 */
			/* destroy sub_tree */
			g_node_destroy (current_node);
		}
		/* get the next current_node (the last item on the stack) */
		current_node = g_ptr_array_index (sub_tree_stack, sub_tree_stack->len - 1);
		/* remove this current_node from the stack */
		g_ptr_array_remove_index (sub_tree_stack, sub_tree_stack->len - 1);
		/* see the comments in case LEVEL_UP */
		if (current_node == NULL) create_new_sub_tree = TRUE;
		else create_new_sub_tree = FALSE;
		break;
	default:
		break;
	}
	return return_value;
}

/*
 * calc_tree_init - basic intialization
 */

void calc_tree_init (int debug_level)
{
	operators = g_strjoinv("", operator_precedence);
	sub_tree_stack = g_ptr_array_new();
	debug = debug_level;
}

/*
 * calc_tree_free - destroys all calc_trees and reinitializes the tree stack.
 */

void calc_tree_free_stack ()
{
	int				counter;

	/* first stack entry is nonsense (see calc_tree_add_token) */
	for (counter = sub_tree_stack->len - 1; counter > 0; counter --) {
		if (g_ptr_array_index (sub_tree_stack, counter) != NULL) \
			g_node_destroy (g_ptr_array_index (sub_tree_stack, counter));
	}
	g_ptr_array_free (sub_tree_stack, TRUE);
	sub_tree_stack = g_ptr_array_new();
}

void calc_tree_free ()
{
	s_calc_token	fake_token;

	/* this is not the fine way, but as current_node is not on the stack, we have to
	 *	quit and remove the current computation/calc_tree somehow. I do this by adding the
	 *	following token.
	 */
	fake_token.number = 0;
	fake_token.operator = '=';
	calc_tree_add_token (fake_token);
	/* as the token contains a '=', calc_tree_free_stack will be called by calc_tree_add_token*/
}

/*
 * from here on its RPN code
 */


void calc_rpn_init (int debug_level)
{
	rpn_stack = g_array_new (FALSE, FALSE, sizeof(double));
	debug = debug_level;
}

void calc_rpn_stack_add (double number)
{
	rpn_stack = g_array_append_val (rpn_stack, number);
	if (debug > 0) fprintf (stderr, _("[%s] RPN stack size is %i.\n"), PROG_NAME, rpn_stack->len);
}

double calc_rpn_stack_operation (s_calc_token current_token)
{
	double	*return_value;
	double	left_hand;
	
	/* this function only serves binary operations. therefore, we need at least one element on
	 * the stack. if this is not the case, work with 0.
	 */
	if (rpn_stack->len < 1) left_hand = 0;
	else {
		/* retrieve left_hand from stack */
		left_hand = g_array_index (rpn_stack, double, rpn_stack->len - 1);
		rpn_stack = g_array_remove_index (rpn_stack, rpn_stack->len - 1);
	}
	/* compute it */
	return_value = calc_expr_computing (left_hand, current_token.operator, current_token.number);
	if (debug > 0) fprintf (stderr, _("[%s] RPN stack size is %i.\n"), PROG_NAME, rpn_stack->len);
	return *return_value;
}

void calc_rpn_free ()
{
	g_array_free (rpn_stack, TRUE);
	rpn_stack = g_array_new (FALSE, FALSE, sizeof(double));
}

/*
int main (int argc, char *argv[])
{
	calc_tree_init(1);
	while (1) calc_debug_input();
	return 1;
}
*/
