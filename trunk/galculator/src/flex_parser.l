	/* TODO
	 * 
	 */

%option noyywrap
%option nounput
%option never-interactive

SPACES		[[:space:]]*
ID		[[:alpha:]][[:alnum:]]*

%x OP

%{
#include <stdlib.h>
#include <ctype.h>

#include "galculator.h"
#include "calc_basic.h"
#include "general_functions.h"
#include "math_functions.h"
#include "flex_parser.h"

static int flex_parser_identify_constant (char *string);
static int flex_parser_identify_user_function (char *string);
static void flex_parser_submit_token(char operation);
static void flex_parser_submit_func_token (double (*func)(double));

static GSList		*user_function_stack=NULL, *flex_buffer_stack=NULL;
static ALG_OBJECT	*alg_object_stack=NULL;
static s_cb_token 	parser_token;
static int 		bracket_counter=0;
%}

	
%%
	/* 
	 * OPERATIONS.
	 */
<OP>"-"|"+"|"*"|"/"|"^"|"%"|"&"|"|" flex_parser_submit_token(*yytext);
<OP>"<<"	flex_parser_submit_token('<');
<OP>">>"	flex_parser_submit_token('>');
<OP>and|AND	flex_parser_submit_token('&');
<OP>or|OR	flex_parser_submit_token('|');
<OP>xor|XOR	flex_parser_submit_token('x');
<OP>mod|MOD	flex_parser_submit_token('m');
<OP>"="		flex_parser_submit_token('='); return TRUE;
	/* 
	 * opening bracket - a bit different from an operation 
	 */
"("		{
			parser_token.func = NULL;
			parser_token.operation = *yytext;
			alg_add_token (CURRENT_ALG_OBJECT, parser_token);
			bracket_counter++;
		}
	/* 
	 * closing bracket - a bit different from an operation 
	 */
<OP>")"		{
			parser_token.func = NULL;
			parser_token.operation = *yytext;
			/* closing bracket must be followed by operation,
			 * the value we want to feed with that operation
			 * is alg_add_token's return value
			 */
			parser_token.number = alg_add_token (CURRENT_ALG_OBJECT, parser_token);
			bracket_counter--;
		}
<OP>")!"	{
			parser_token.func = factorial;
			parser_token.operation = *yytext;
			/* closing bracket must be followed by operation,
			 * the value we want to feed with that operation
			 * is alg_add_token's return value
			 */
			parser_token.number = alg_add_token (CURRENT_ALG_OBJECT, parser_token);
			bracket_counter--;
		}
	/* 
	 * FACTORIAL - only after a number, not after a bracket enclosed expression
	 */

<OP>!			parser_token.number = factorial(parser_token.number);

	/* 
	 * NUMBERS
	 */
[-]?([[:digit:]])+[.,]?([[:digit:]])* 	parser_token.number = string2double (yytext, CS_DEC); BEGIN(OP);
[-]?[.,]+([[:digit:]])+			parser_token.number = string2double (yytext, CS_DEC); BEGIN(OP);
(0b)([0,1])*		yytext+=2*sizeof(char); parser_token.number = string2double (yytext, CS_BIN); BEGIN(OP);
(0o)([0-7])*		yytext+=2*sizeof(char); parser_token.number = string2double (yytext, CS_OCT); BEGIN(OP);
(0x)([[:xdigit:]])*	yytext+=2*sizeof(char); parser_token.number = string2double (yytext, CS_HEX); BEGIN(OP);
(0h)([[:xdigit:]])*	yytext+=2*sizeof(char); parser_token.number = string2double (yytext, CS_HEX); BEGIN(OP);
	/* 
	 * CONSTANTS __maybe__ 
	 */
{ID}		{
			if (flex_parser_identify_constant(yytext) == FALSE)  return FALSE;
			BEGIN(OP);
		}
	/* 
	 * FUNCTIONS
	 */
sin{SPACES}"("		flex_parser_submit_func_token (sin_wrapper);
asin{SPACES}"("		flex_parser_submit_func_token (asin_wrapper);
sinh{SPACES}"("		flex_parser_submit_func_token (sinh);
asinh{SPACES}"("	flex_parser_submit_func_token (asinh);
cos{SPACES}"("		flex_parser_submit_func_token (cos_wrapper);
acos{SPACES}"("		flex_parser_submit_func_token (acos_wrapper);
cosh{SPACES}"("		flex_parser_submit_func_token (cosh);
acosh{SPACES}"("	flex_parser_submit_func_token (acosh);
tan{SPACES}"("		flex_parser_submit_func_token (tan_wrapper);
atan{SPACES}"("		flex_parser_submit_func_token (atan_wrapper);
tanh{SPACES}"("		flex_parser_submit_func_token (tanh);
atanh{SPACES}"("	flex_parser_submit_func_token (atanh);
sqrt{SPACES}"("		flex_parser_submit_func_token (sqrt);
ln{SPACES}"("		flex_parser_submit_func_token (log);
log{SPACES}"("		flex_parser_submit_func_token (log10);
!{SPACES}"("		flex_parser_submit_func_token (factorial);
cmp{SPACES}"("		flex_parser_submit_func_token (cmp);
CMP{SPACES}"("		flex_parser_submit_func_token (cmp);
~{SPACES}"("		flex_parser_submit_func_token (cmp);
	/* 
	 * USER DEFINED FUNCTIONS. __maybe__
	 */
{ID}{SPACES}"("		if (flex_parser_identify_user_function (yytext) == FALSE) return FALSE;
	/*
	 * ignore spaces
	 */
<*>{SPACES}
	/*
	 * UNKNOWN
	 */
<*>.	return FALSE;

%%

static double user_function_handler (double value)
{
	int			index;
	s_flex_parser_result	result;

	/* pop user function index from stack */
	index = GPOINTER_TO_INT (user_function_stack->data);
	user_function_stack = g_slist_delete_link (user_function_stack, user_function_stack);
	result = compute_user_function (user_function[index].expression, 
		user_function[index].variable, 
		get_display_number_string (value, CS_DEC));
	if (!result.error) return result.value;
	else yyterminate();
}

static int flex_parser_identify_user_function (char *string)
{	
	int 	counter;

	/* remove spaces and opening bracket */
	counter = strlen(string) - 2;
	while ((counter > 0) && isspace(string[counter])) counter--;
	string[counter + 1] = '\0';
	counter = 0;
	while (user_function[counter].name != NULL) {
		if (strcmp (user_function[counter].name, string) == 0) {
			user_function_stack = g_slist_prepend (
				user_function_stack, GINT_TO_POINTER (counter));
			flex_parser_submit_func_token(user_function_handler);
			return TRUE;
		}
		counter++;
	}
	return FALSE;
}

static int flex_parser_identify_constant (char *string)
{
	int 	counter = 0;
	
	while (constant[counter].name != NULL) {
		if (strcmp (constant[counter].name, string) == 0) {
			parser_token.number = string2double(constant[counter].value, CS_DEC);
			return TRUE;
		}
		counter++;
	}
	return FALSE;
}

static void flex_parser_submit_token (char operation)
{
	parser_token.operation = operation;
	parser_token.number = alg_add_token (CURRENT_ALG_OBJECT, parser_token);
	BEGIN(INITIAL);
}

static void flex_parser_submit_func_token (double (*func)(double))
{
	parser_token.func = func;
	parser_token.operation = '(';
	alg_add_token (CURRENT_ALG_OBJECT, parser_token);
	bracket_counter++;
}

s_flex_parser_result flex_parser (const char *string)
{
	int 			string_length, bracket_level;
	char			*my_string;
	static int 		recursion_counter = 0;
	s_flex_parser_result	return_result;
	
	recursion_counter++;
	if (recursion_counter > FLEX_PARSER_NR_RECURSIONS)
		fprintf (stderr, _("[%s] flex parser was called more than %i times. Do you \
know what you are doing? If not: %s\n"), PROG_NAME, FLEX_PARSER_NR_RECURSIONS, BUG_REPORT);
	/* create a '=' terminated copy of string */
	string_length = strlen(string);
	if (string[string_length-1] != '=') {
		my_string = (char *) malloc ((string_length + 2) * sizeof(char));
		strcpy(my_string, string);
		my_string[string_length] = '=';
		my_string[string_length + 1] = '\0';
	} else {
		my_string = (char *) malloc ((string_length+ 1) * sizeof(char));
		strcpy (my_string, string);
	}
	/* initialize */
	bracket_level = bracket_counter;
	alg_object_stack = g_slist_prepend (alg_object_stack, alg_init(0));
	/* the flex specific part. put current buffer on stack, create new one */
	flex_buffer_stack = g_slist_prepend (flex_buffer_stack, YY_CURRENT_BUFFER);
        yy_scan_string (my_string);
	BEGIN(INITIAL);
        return_result.error = !yylex();
	/* restore previous buffer state */
        yy_delete_buffer(YY_CURRENT_BUFFER);
	yy_switch_to_buffer (flex_buffer_stack->data);
	flex_buffer_stack = g_slist_delete_link (flex_buffer_stack, flex_buffer_stack);
	free (my_string);
	alg_free (alg_object_stack->data);
	alg_object_stack = g_slist_delete_link (alg_object_stack, alg_object_stack);
	/* after finishing a buffer we insert a number, so state is OP?!?!? */
	BEGIN(OP);
	/* process result */
	if (bracket_counter != bracket_level) {
		return_result.error = TRUE;
		bracket_counter = bracket_level;
	}		
	return_result.value = parser_token.number;
	recursion_counter--;
	return return_result;
}
