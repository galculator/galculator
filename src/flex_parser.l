	/* TODO
	 * 
	 */

%option noyywrap
%option nounput
%option never-interactive

NUMBER		(0x|0h|0b|0d|0o)?[-]?([[:xdigit:]])*[.,]?([[:xdigit:]])*
SPACES		[[:space:]]*
ID		[[:alpha:]][[:alnum:]_]*

%x OP

%{
#include <stdlib.h>

#include "galculator.h"
#include "calc_basic.h"
#include "general_functions.h"
#include "math_functions.h"

int flex_parser_identify_constant (char *string);
void flex_parser_submit_token(char operation);
void flex_parser_submit_func_token (double (*func)(double));
%}

	s_cb_token 	parser_token;
	int 		bracket_counter;
	
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
			alg_add_token (parser_token);
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
			parser_token.number = alg_add_token (parser_token);
			bracket_counter--;
		}
<OP>")!"	{
			parser_token.func = factorial;
			parser_token.operation = *yytext;
			/* closing bracket must be followed by operation,
			 * the value we want to feed with that operation
			 * is alg_add_token's return value
			 */
			parser_token.number = alg_add_token (parser_token);
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
			if (flex_parser_identify_constant(yytext) == FALSE) return FALSE;
			BEGIN(OP);
		}
	/* 
	 * FUNCTIONS
	 */
sin{SPACES}"("		flex_parser_submit_func_token (sin);
asin{SPACES}"("		flex_parser_submit_func_token (asin);
sinh{SPACES}"("		flex_parser_submit_func_token (sinh);
asinh{SPACES}"("	flex_parser_submit_func_token (asinh);
cos{SPACES}"("		flex_parser_submit_func_token (cos);
acos{SPACES}"("		flex_parser_submit_func_token (acos);
cosh{SPACES}"("		flex_parser_submit_func_token (cosh);
acosh{SPACES}"("	flex_parser_submit_func_token (acosh);
tan{SPACES}"("		flex_parser_submit_func_token (tan);
atan{SPACES}"("		flex_parser_submit_func_token (atan);
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
{ID}{SPACES}"("		printf ("[UF] %s\n", yytext);
	/*
	 * ignore spaces
	 */
<*>{SPACES}
	/*
	 * UNKNOWN
	 */
<*>.	return FALSE;

%%

int flex_parser_identify_constant (char *string)
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

void flex_parser_submit_token (char operation)
{
	parser_token.operation = operation;
	parser_token.number = alg_add_token (parser_token);
	BEGIN(INITIAL);
}

void flex_parser_submit_func_token (double (*func)(double))
{
	parser_token.func = func;
	parser_token.operation = '(';
	alg_add_token (parser_token);
	bracket_counter++;
}

double flex_parser (const char *string)
{
	YY_BUFFER_STATE yybs;
	int 		state, string_length;
	char		*my_string;
	
	string_length = strlen(string);
	if (string[string_length-1] != '=') {
		my_string = (char *) malloc ((string_length + 2) * sizeof(char));
		strcpy(my_string, string);
		my_string[string_length] = '=';
		my_string[string_length + 1] = '\0';
	} else {
		my_string = (char *) malloc ((strlen(string) + 1) * sizeof(char));
		strcpy (my_string, string);
	}
	alg_free();
	alg_init(0);
	bracket_counter = 0;
        yybs = yy_scan_string (my_string);
	BEGIN(INITIAL);
        state = yylex();
        yy_delete_buffer(yybs);
	if ((state == TRUE) && (bracket_counter == 0)) 
		printf ("[result] %f\n", parser_token.number);
	else printf ("[ERROR]\n");
	free (my_string);
	return parser_token.number;
}
