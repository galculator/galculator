	/* TODO
	 * 
	 */

%option noyywrap
%option nounput
%option never-interactive

FUNCTION	a?sinh?|a?cosh?|a?tanh?
NUMBER		(0x|0h|0b|0d|0o)?[-]?([[:xdigit:]])*[.,]?([[:xdigit:]])*
OPERATION	[-]|[+]|"*"|"/"|"&"|and|AND|"|"|or|OR|=
SPACES		[[:space:]]*
ID		[[:alpha:]][[:alnum:]_]*

%x OP

%{
#include <stdlib.h>

#include "calc_basic.h"
%}

	s_cb_token 	parser_token;

%%

	/* operations. without brackets! */

<OP>{OPERATION}	{
			parser_token.operation = *yytext;
			if (*yytext == '=') printf ("[result] = %f\n", 
				alg_add_token (parser_token));
			else alg_add_token (parser_token);
			BEGIN(INITIAL);
		}

	/* opening bracket */

"("		{
			parser_token.number.func = NULL;
			parser_token.operation = *yytext;
			alg_add_token (parser_token);
		}

	/* closing bracket */

<OP>")"		{
			parser_token.operation = *yytext;
			/* closing bracket must be followed by operation,
			 * the value we want to feed with that operation
			 * is alg_add_token's return value
			 */
			parser_token.number.value = alg_add_token (parser_token);
		}
		
	/* numbers */

{NUMBER}	parser_token.number.value = strtod (yytext, NULL); BEGIN(OP);

	/* this __may__ be a constant. check in outer world */

{ID}		{	
			printf ("[C] %s\n", yytext);
			BEGIN(OP);
		}
		
	/* functions (plus following opening bracket) */

{FUNCTION}{SPACES}"("	printf ("[F] %s\n", yytext);

	/* this __may__ be a user defined function. check in outer world */

{ID}{SPACES}"("		printf ("[UF] %s\n", yytext);

	/* ignore spaces */
	
<*>{SPACES}

	/* anything else leads to an error */

<*>.		{
			printf ("[invalid] %s\n", yytext);
			yyterminate();
		}

%%

void flex_parser (char *string)
{
	YY_BUFFER_STATE yybs;
	
	alg_free();
	alg_init(0);
        yybs = yy_scan_string (string);
	BEGIN(INITIAL);
        yylex();
        yy_delete_buffer(yybs);
	return;
}
