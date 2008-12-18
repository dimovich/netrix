
//NetrixC lexicographical parser
//
//Code based on:
//Quake3 botlib (by id Software);
//
//Code parts adapted from:
//"Mini C++ Interpreter" code (by Herbert Schildt);
//


#undef __N_FILE__
#define __N_FILE__	TEXT( "(nxc) lexer.c" )

#include "compile.h"
#include <windows.h>
#include <stdarg.h>
#include <stdio.h> //_vsnprintf
#include <stdlib.h>

#include "../netrixlib/netrixlib.h"
#include "lexer.h"


//default keywords
//
static keyword_t default_keywords[] = {
	{ "int",	K_INT,		NULL },
	{ "float",	K_FLOAT,	NULL },
	{ "switch",	K_SWITCH,	NULL },
	{ "case",	K_CASE,		NULL },
	{ "if",		K_IF,		NULL },
	{ "else",	K_ELSE,		NULL },
	{ "for",	K_FOR,		NULL },
	{ "do",		K_DO,		NULL },
	{ "while",	K_WHILE,	NULL },
	{ "break",	K_BREAK,	NULL },
	{ "return", K_RETURN,	NULL },
	{ NULL,		0,			NULL }
};


//default punctuations
//
static punctuation_t default_punctuations[] = {
	//binary operators
	//
	{ ">>=",	P_RSHIFT_ASSIGN,	NULL },
	{ "<<=",	P_LSHIFT_ASSIGN,	NULL },
	{ "...",	P_PARMS,			NULL },
	{ "##",		P_PRECOMPMERGE,		NULL },
	
	//logic operators
	//
	{ "&&",		P_LOGIC_AND,		NULL },
	{ "||",		P_LOGIC_OR,			NULL },
	{ ">=",		P_LOGIC_GEQ,		NULL },
	{ "<=",		P_LOGIC_LEQ,		NULL },
	{ "==",		P_LOGIC_EQ,			NULL },
	{ "!=",		P_LOGIC_UNEQ,		NULL },
	
	//arithmetic operators
	//
	{ "*=",		P_MUL_ASSIGN,		NULL },
	{ "/=",		P_DIV_ASSIGN,		NULL },
	{ "%=",		P_MOD_ASSIGN,		NULL },
	{ "+=",		P_ADD_ASSIGN,		NULL },
	{ "-=",		P_SUB_ASSIGN,		NULL },
	{ "++",		P_INC,				NULL },
	{ "--",		P_DEC,				NULL },
	
	//binary operators
	//
	{ "&=",		P_BIN_AND_ASSIGN,	NULL },
	{ "|=",		P_BIN_OR_ASSIGN,	NULL },
	{ "^=",		P_BIN_XOR_ASSIGN,	NULL },
	{ ">>",		P_RSHIFT,			NULL },
	{ "<<",		P_LSHIFT,			NULL },
	
	//reference operators
	//
	{ "->",		P_POINTERREF,		NULL },
	{ "::",		P_CPP1,				NULL },
	{ ".*",		P_CPP2,				NULL },
	
	//arithmetic operators
	//
	{ "*",		P_MUL,				NULL },
	{ "/",		P_DIV,				NULL },
	{ "%",		P_MOD,				NULL },
	{ "+",		P_ADD,				NULL },
	{ "-",		P_SUB,				NULL },
	{ "=",		P_ASSIGN,			NULL },
	
	//binary operators
	//
	{ "&",		P_BIN_AND,			NULL },
	{ "|",		P_BIN_OR,			NULL },
	{ "^",		P_BIN_XOR,			NULL },
	{ "~",		P_BIN_NOT,			NULL },
	
	//logic operators
	//
	{ "!",		P_LOGIC_NOT,		NULL },
	{ ">",		P_LOGIC_GREATER,	NULL },
	{ "<",		P_LOGIC_LESS,		NULL },
	
	//reference operators
	//
	{ ".",		P_REF,				NULL },
	
	//separators
	//
	{ ",",		P_COMMA,			NULL },
	{ ";",		P_SEMICOLON,		NULL },
	
	//label indication
	//
	{ ":",		P_COLON,			NULL },
	
	//if statement
	//
	{ "?",		P_QUESTIONMARK,		NULL },
	
	//embracements
	//
	{ "(",		P_PARENTHESESOPEN,	NULL },
	{ ")",		P_PARENTHESESCLOSE, NULL },
	{ "{",		P_BRACEOPEN,		NULL },
	{ "}",		P_BRACECLOSE,		NULL },
	{ "[",		P_SQBRACKETOPEN,	NULL },
	{ "]",		P_SQBRACKETCLOSE,	NULL },

	{ "\\",		P_BACKSLASH,		NULL },
	
	//precompiler operator
	//
	{ "#",		P_PRECOMP,			NULL },

	{ "$",		P_DOLLAR,			NULL },
	
	{ NULL,		0,					NULL }
};



/*
=====================
	L_Compress
=====================
*/
int L_Compress( char *data_p ) {
	char *in, *out;
	int c;
	BOOL newline = FALSE, whitespace = FALSE;
	
	//check arguments
	//
	if( !data_p ) {
		return 0;
	}
	
	in = out = data_p;
	
	if( in ) {
		while( (c = *in) != 0 ) {
			//skip double slash comments
			//
			if( (c == '/') && (in[1] == '/' ) ) {
				while( *in && (*in != '\n') ) {
					in++;
				}
			}
			//skip /* */ comments
			//
			else if( (c == '/') && (in[1] == '*') ) {
				while( *in && (*in != '*' || in[1] != '/') ) {
					in++;
				}
				if( *in ) {
					in += 2;
				}
			}
			//record when we hit a new line
			//
			else if( (c == '\n') || (c == '\r') ) {
				newline = TRUE;
				in++;
			}
			//record when we hit whitespace
			//
			else if( (c == ' ') || (c == '\t') ) {
				whitespace = TRUE;
				in++;
			}
			//an actual token
			//
			else {
				//if we have a newline, emit it (and it counts as whitespace)
				//
				if( newline ) {
					*out++ = '\n';
					newline = FALSE;
					whitespace = FALSE;
				}
				if( whitespace ) {
					*out++ = ' ';
					whitespace = FALSE;
				}
				
				//copy quoted strings unmolested
				//
				if( c == '"' ) {
					*out++ = c;
					in++;
					while( 1 ) {
						c = *in;
						if( c && (c != '"') ) {
							*out++ = c;
							in++;
						}
						else {
							break;
						}
					}
					if( c == '"' ) {
						*out++ = c;
						in++;
					}
				}
				else {
					*out = c;
					out++;
					in++;
				}
			}
		}//while
	}//if
	
	*out = 0;
	return (int)(out - data_p);
}


/*
=====================
	L_CreatePunctuationTable
=====================
*/
void L_CreatePunctuationTable( script_t *script, punctuation_t *punctuations ) {
	int i;
	punctuation_t *p, *lastp, *newp;
	
	//check arguments validity
	//
	if( (script == NULL) || (punctuations == NULL) ) {
		return;
	}
	
	//allocate memory for the table
	//
	if( !script->punctuationtable ) {
		script->punctuationtable =
			(punctuation_t **) N_Malloc( 256*sizeof(punctuation_t *) );
	}
	
	//add punctuations in the list to the
	//punctuation table
	//
	for( i=0; punctuations[i].p; i++ ) {
		newp = &punctuations[i];
		lastp = NULL;

		//sort the punctuations in this table entry on length
		//(longer punctuations first)
		//
		for( p=script->punctuationtable[(unsigned int)newp->p[0]]; p; p=p->next ) {
			if( N_Strlen( p->p ) < N_Strlen( newp->p ) ) {
				newp->next = p;
				if( lastp ) {
					lastp->next = newp;
				}
				else {
					script->punctuationtable[(unsigned int)newp->p[0]] = newp;
				}
				break;
			}
			lastp = p;
		}

		if( p == NULL ) {
			newp->next = NULL;
			if( lastp ) {
				lastp->next = newp;
			}
			else {
				script->punctuationtable[(unsigned int)newp->p[0]] = newp;
			}
		}
	}
}


/*
=====================
	L_PunctuationFromNum
=====================
*/
char * L_PunctuationFromNum( script_t *script, int num ) {
	int i;
	
	//check arguments
	//
	if( !script || !script->punctuations ) {
		return NULL;
	}
	
	//search punctuation
	//
	for( i=0; script->punctuations[i].p; i++ ) {
		if( script->punctuations[i].n == num ) {
			return script->punctuations[i].p;
		}
	}

	return NULL;
}


/*
=====================
	L_ScriptError
=====================
*/
void L_ScriptError( script_t *script, char *str, ... ) {
	char szBuff[1024];
	va_list va;
	
	//check arguments
	//
	if( !script || !str ) {
		return;
	}
	
	if( script->flags & SCFL_NOERRORS ) {
		return;
	}
	
	va_start( va, str );
	_vsnprintf( szBuff, 1024, str, va );
	va_end( va );
	
	N_Trace( "error: file %s, line %d: %s\n",
		script->szFileName, script->line, szBuff );
}


/*
=====================
	L_ScriptWarning
=====================
*/
void L_ScriptWarning( script_t *script, char *str, ... ) {
	char szBuff[1024];
	va_list va;
	
	if( script->flags & SCFL_NOWARNINGS ) {
		return;
	}
	
	va_start( va, str );
	_vsnprintf( szBuff, 1024, str, va );
	va_end( va );
	
	N_Trace( "warning: file %s, line %d: %s\n",
		script->szFileName, script->line, szBuff );
}


/*
=====================
	L_SetScriptKeywords
=====================
*/
void L_SetScriptKeywords( script_t *script, keyword_t *keywords ) {
	//check arguments validity
	//
	if( script == NULL ) {
		return;
	}
	
	if( keywords == NULL ) {
		script->keywords = default_keywords;
	}
	else {
		script->keywords = keywords;
	}
}


/*
=====================
	L_LookUpName
=====================
*/
int L_LookUpName( script_t *script, token_t *token ) {
	int i;

	//check arguments
	//
	if( !token || !script || !script->keywords ) {
		return K_INVALID;
	}

	//see if token string matches any keyword
	//
	for( i=0; script->keywords[i].p; i++ ) {
		if( !N_Strcmp( token->string, script->keywords[i].p ) ) {
			return script->keywords[i].n;
		}
	}
	
	return K_INVALID;
}


/*
=====================
	L_SetScriptPunctuations
=====================
*/
void L_SetScriptPunctuations( script_t *script, punctuation_t *p ) {
	//check arguments validity
	//
	if( script == NULL ) {
		return;
	}

	if( p ) {
		L_CreatePunctuationTable( script, p );
		script->punctuations = p;
	}
	else {
		L_CreatePunctuationTable( script, default_punctuations );
		script->punctuations = default_punctuations;
	}
}


/*
=====================
	L_ReadWhiteSpace
=====================
*/
BOOL L_ReadWhiteSpace( script_t *script ) {
	//check arguments validity
	//
	if( !script || !script->script_p ) {
		return FALSE;
	}

	while( TRUE ) {
		//skip white space
		//
		while( *script->script_p <= ' ' ) {
			if( !(*script->script_p) ) {
				return FALSE;
			}
			if( *script->script_p == '\n' ) {
				script->line++;
			}
			script->script_p++;
		}
		
		//skip comments
		//
		if( *script->script_p == '/' ) {
			//comments //
			//
			if( *(script->script_p+1) == '/' ) {
				script->script_p++;
				do {
					script->script_p++;
					if( !(*script->script_p) ) {
						return FALSE;
					}
				}while( *script->script_p != '\n' );

				script->line++;
				script->script_p++;
				if( !(*script->script_p) ) {
					return FALSE;
				}
				continue;
			}
			//comments /*
			//
			else if( *(script->script_p+1) == '*' ) {
				script->script_p++;
				do {
					script->script_p++;
					if( !(*script->script_p) ) {
						return FALSE;
					}
					if( *script->script_p == '\n' ) {
						script->line++;
					}
				}while( !(*script->script_p == '*' && *(script->script_p+1) == '/') );
				
				script->script_p += 2;
				if( !(*script->script_p) ) {
					return FALSE;
				}
				continue;
			}
		}
		break;
	}

	return TRUE;
}


/*
=====================
	L_SkipLine
=====================
*/
BOOL L_SkipLine( script_t *script ) {
	int tmpline;

	if( script == NULL ) {
		return FALSE;
	}
	
	while( TRUE ) {
		
		//check for end of script
		//
		if( !(*script->script_p) ) {
			return FALSE;
		}
		
		//check for /* comments
		//
		if( (*script->script_p == '/') && (*(script->script_p + 1) == '*' ) ) {
	
			tmpline = script->line;
	
			if( !L_ReadWhiteSpace( script ) ) {
				return FALSE;
			}

			//new line was crossed in white space
			//
			if( tmpline > script->line ) {
				return TRUE;
			}
		}
		
		//check for new line
		//
		if( *script->script_p == '\n' ) {
			script->script_p++;
			script->line++;
			break;
		}
		
		script->script_p++;
	}

	return TRUE;
}


/*
=====================
	L_ReadEscapeCharacter
=====================
*/
BOOL L_ReadEscapeCharacter( script_t *script, char *ch ) {
	int c, val, i;
	
	//check arguments
	//
	if( !script || !script->script_p || !ch ) {
		return FALSE;
	}
	
	//step over the leading '\\'
	//
	script->script_p++;
	
	//determine the escape character
	//
	switch( *script->script_p ) {
		case '\\': c = '\\'; break;
		case 'n': c = '\n'; break;
		case 'r': c = '\r'; break;
		case 't': c = '\t'; break;
		case 'v': c = '\v'; break;
		case 'b': c = '\b'; break;
		case 'f': c = '\f'; break;
		case 'a': c = '\a'; break;
		case '\'': c = '\''; break;
		case '\"': c = '\"'; break;
		case '\?': c = '\?'; break;
		case 'x': {
			script->script_p++;
			for( i=0, val=0; ; i++, script->script_p++ ) {
				c = *script->script_p;
				if( c>='0' && c<='9' ) {
					c -= '0';
				}
				else if( c>='A' && c<='Z' ) {
					c = c - 'A' + 10;
				}
				else if( c>='a' && c<='z' ) {
					c = c - 'a' + 10;
				}
				else {
					break;
				}
				val = (val<<4) + c;
			}
			script->script_p--;

			if( val > 0xFF ) {
				L_ScriptWarning( script, "too large value in escape character" );
				val = 0xFF;
			}
			c = val;
		} break;

		//decimal ASCII code
		//
		default: {
			if( *script->script_p < '0' || *script->script_p > '9' ) {
				L_ScriptError( script, "unknown escape character" );
			}
			for( i=0, val=0; ; i++, script->script_p++ ) {
				c = *script->script_p;
				if( c>='0' && c<='9' ) {
					c -= '0';
				}
				else {
					break;
				}
				val = val*10 + c;
			}
			script->script_p--;

			if( val > 0xFF ) {
				L_ScriptWarning( script, "too large value in escape character" );
				val = 0xFF;
			}
			c = val;
		} break;
	}
	
	//step over the escape character or the last digit of the number
	//
	script->script_p++;
	
	//store the escape character
	//
	*ch = c;
	
	return TRUE;
}


/*
=====================
	L_ReadString
=====================
*/
BOOL L_ReadString( script_t *script, token_t *token, int quote ) {
	int len, tmpline;
	char *tmpscript_p;
	
	//check arguments validity
	//
	if( (script == NULL) || (token == NULL) ) {
		return FALSE;
	}
	
	//determine string type
	//
	if( quote == '\"' ) {
		token->type = TT_STRING;
	}
	else {
		token->type = TT_LITERAL;
	}
	
	len = 0;
	
	//read leading quote
	//
	token->string[len++] = *script->script_p++;
	
	while( TRUE ) {
		//minus 2 because trailing double quote and
		//zero have to be appended.
		//
		if( len >= MAX_TOKEN-2 ) {
			L_ScriptError( script, "string longer than MAX_TOKEN = %d", MAX_TOKEN );
			return FALSE;
		}

		//process escape character
		//
		if( (*script->script_p == '\\') && !(script->flags & SCFL_NOSTRINGESCAPECHARS) ) {
			if( !L_ReadEscapeCharacter( script, &token->string[len] ) ) {
				token->string[len] = 0;
				return FALSE;
			}
			len++;
		}
		//process trailing quote
		//
		else if( *script->script_p == quote ) {
			//step over the double quote
			//
			script->script_p++;
			
			//if white spaces in a string are not allowed
			//
			if( script->flags & SCFL_NOSTRINGWHITESPACES ) {
				break;
			}
			
			tmpscript_p = script->script_p;
			tmpline = script->line;
			
			//read white space between possible two follwing strings
			//
			if( !L_ReadWhiteSpace( script ) ) {
				script->script_p = tmpscript_p;
				script->line = tmpline;
				break;
			}
			
			//if there's no leading double quote
			//
			if( *script->script_p != quote ) {
				script->script_p = tmpscript_p;
				script->line = tmpline;
				break;
			}
			
			//step over the new leading double quote
			//
			script->script_p++;
		}
		//process string contents
		//
		else {
			if( *script->script_p == '\0' ) {
				token->string[len] = 0;
				L_ScriptError( script, "missing trailing quote" );
				return FALSE;
			}
			
			if( *script->script_p == '\n' ) {
				token->string[len] = 0;
				L_ScriptError( script, "newline inside string %s", token->string );
				return 0;
			}
			
			token->string[len++] = *script->script_p++;
		}
	}
	
	//trailing quote
	//
	token->string[len++] = quote;

	//end string with a zero
	//
	token->string[len] = '\0';
	
	//the subtype is the length of the string
	//
	token->subtype = len;

	return TRUE;
}


/*
=====================
	L_ReadName
=====================
*/
BOOL L_ReadName( script_t *script, token_t *token ) {
	int len = 0;
	char c;	
	
	token->type = TT_NAME;
	
	do {
		token->string[len++] = *script->script_p++;
		if( len >= MAX_TOKEN ) {
			L_ScriptError( script, "name longer than MAX_TOKEN = %d", MAX_TOKEN );
			return FALSE;
		}
		c = *script->script_p;

	}while( (c>='a' && c<='z') || (c>='A' && c<='Z') || (c>='0' && c<='9') || (c=='_') );
	
	token->string[len] = '\0';
	token->subtype = len;
	
	return TRUE;
}


/*
=====================
	L_NumberValue
	-----------------
	FIXME: rewrite using CRL functions
=====================
*/
void L_NumberValue( char *string, int subtype, long int *intvalue, float *floatvalue ) {
	long int dotfount = 0;
	
	//check arguments validity
	//
	if( (string == NULL) || (intvalue == NULL) || (floatvalue == NULL)) {
		return;
	}
	
	*intvalue = 0;
	*floatvalue = 0;
	
	//floating point number
	//
	if( subtype & TT_FLOAT ) {

		*floatvalue = (float)strtod( string, NULL );
		*intvalue = (long int) *floatvalue;
	}
	//decimal number
	//
	else if( subtype & TT_DECIMAL ) {

		*intvalue = strtol( string, NULL, 10 );
		*floatvalue = (float)*intvalue;
	}
	//hex number
	//
	else if( subtype & TT_HEX ) {

		//step over the leading 0x or 0X
		//
		string += 2;

		*intvalue = strtol( string, NULL, 16 );
		*floatvalue = (float)*intvalue;
	}
	//octal number
	//
	else if( subtype & TT_OCTAL ) {

		//step over the first zero
		//
		string++;

		*intvalue = strtol( string, NULL, 8 );
		*floatvalue = (float)*intvalue;
	}
	//binary number
	//
	else if( subtype & TT_BINARY ) {
		
		//step over the leading 0b or 0B
		//
		string += 2;

		*intvalue = strtol( string, NULL, 2 );
		*floatvalue = (float)*intvalue;
	}
}


/*
=====================
	L_ReadNumber
=====================
*/
BOOL L_ReadNumber( script_t *script, token_t *token ) {
	int len = 0;
	int i;
	BOOL octal, dot;
	char c;
	
	token->type |= TT_NUMBER;
	
	//check for a hexadecimal number
	//
	if( (*script->script_p == '0') && ((*(script->script_p+1) == 'x') || (*(script->script_p+1) == 'X')) ) {
		token->string[len++] = *script->script_p++;
		token->string[len++] = *script->script_p++;
		
		c = *script->script_p;
		
		//hexadecimal
		//
		while( (c>='0' && c<='9') || (c>='a' && c<='f') || (c>='A' && c<='F')) {
			token->string[len++] = *script->script_p++;
			if( len >= (MAX_TOKEN-1) ) {
				L_ScriptError( script, "hexadecimal number longer than MAX_TOKEN = %d", MAX_TOKEN );
				return 0;
			}
			c = *script->script_p;
		}
		token->subtype |= TT_HEX;
	}
	//check for a binary number
	//
	else if( (*script->script_p == '0') && (*(script->script_p+1) == 'b' || *(script->script_p+1) == 'B') ) {
		token->string[len++] = *script->script_p++;
		token->string[len++] = *script->script_p++;
		
		c = *script->script_p;
		
		while( (c == '0') || (c == '1') ) {
			token->string[len++] = *script->script_p++;

			if( len >= (MAX_TOKEN-1) ) {
				L_ScriptError( script, "binary number longer than MAX_TOKEN = %d", MAX_TOKEN );
				return FALSE;
			}
			c = *script->script_p;
		}
		token->subtype |= TT_BINARY;
	}
	//decimal or octal integer or floating point number
	//
	else {
		octal = FALSE;
		dot = FALSE;
		
		if( *script->script_p == '0' ) {
			octal = TRUE;
		}
		
		while( TRUE ) {
			c = *script->script_p;
			if( c == '.' ) {
				dot = TRUE;
			}
			else if( (c == '8') || (c == '9') ) {
				octal = FALSE;
			}
			else if( (c < '0') || (c > '9') ) {
				break;
			}
			token->string[len++] = *script->script_p++;

			if( len >= (MAX_TOKEN-1) ) {
				L_ScriptError( script, "number longer than MAX_TOKEN = %d", MAX_TOKEN );
				return FALSE;
			}
		}
		if( octal ) {
			token->subtype |= TT_OCTAL;
		} else {
			token->subtype |= TT_DECIMAL;
		}
		
		if( dot ) {
			token->subtype |= TT_FLOAT;
		}
	}
	
	//check suffix
	//
	for( i=0; i<2; i++ ) {
		c = *script->script_p;

		//check for a LONG number
		//
		if( (c == 'l' || c == 'L') && !(token->subtype & TT_LONG) ) {
			script->script_p++;
			token->subtype |= TT_LONG;
		}
		//check for an UNSIGNED number
		//
		else if( (c == 'u' || c == 'U') && !(token->subtype & (TT_UNSIGNED | TT_FLOAT)) ) {
			script->script_p++;
			token->subtype |= TT_UNSIGNED;
		}
	}
	
	token->string[len] = '\0';
	
	//convert string to number value
	//
	L_NumberValue( token->string, token->subtype, &token->intvalue, &token->floatvalue );
	
	if( !(token->subtype & TT_FLOAT) ) {
		token->subtype |= TT_INTEGER;
	}

	return TRUE;
}


/*
=====================
	L_ReadLiteral
=====================
*/
BOOL L_ReadLiteral( script_t *script, token_t *token ) {
	//check arguments validity
	//
	if( (script == NULL) || (token == NULL) ) {
		return FALSE;
	}
	
	token->type = TT_LITERAL;
	
	//first quote
	//
	token->string[0] = *script->script_p++;
	
	//check for end of file
	//
	if( !(*script->script_p) ) {
		L_ScriptError( script, "end of file before trailing \'" );
		return FALSE;
	}
	
	//if it is an escape character
	//
	if( *script->script_p == '\\' ) {
		if( !L_ReadEscapeCharacter( script, &token->string[1] ) ) {
			return FALSE;
		}
	}
	else {
		token->string[1] = *script->script_p++;
	}
	
	//check for trailing quote
	//
	if( *script->script_p != '\'' ) {
		L_ScriptWarning( script, "too many characters in literal, ignored" );

		while( *script->script_p && (*script->script_p != '\'') && (*script->script_p != '\n') ) {
			script->script_p++;
		}
	}
	
	if( *script->script_p == '\'' ) {
		script->script_p++;
	}
	
	//store the trailing quote
	//
	token->string[2] = '\'';
	
	token->string[3] = '\0';
	
	//the subtype is the integer literal value
	//
	token->subtype = token->string[1];
	
	return TRUE;
}


/*
=====================
	L_ReadPunctuation
=====================
*/
BOOL L_ReadPunctuation( script_t *script, token_t *token ) {
	int len;
	char *p;
	punctuation_t *punc;
	
	//check arguments validity
	//
	if( (script == NULL) || (token == NULL) ) {
		return FALSE;
	}
	
	for( punc = script->punctuationtable[(unsigned int)*script->script_p]; punc; punc = punc->next ) {
		p = punc->p;
		len = N_Strlen( p );

		//if the script contains at least as much characters as the punctuation
		//
		if( script->script_p+len <= script->end_p ) {
			//if the script contains the punctuation
			//
			if( !N_Strncmp( script->script_p, p, len ) ) {

				N_Strncpy( token->string, p, MAX_TOKEN );
				script->script_p += len;

				token->type = TT_PUNCTUATION;

				//subtype is the number of punctuation
				//
				token->subtype = punc->n;

				return TRUE;
			}
		}
	}
	
	return FALSE;
}


/*
=====================
	L_ReadPrimitive
=====================
*/
BOOL L_ReadPrimitive( script_t *script, token_t *token ) {
	int len;

	//check arguments validity
	//
	if( (script == NULL) || (token == NULL) ) {
		return FALSE;
	}
	
	len = 0;
	while( (*script->script_p > ' ' ) && (*script->script_p != ';') ) {
		if( len >= (MAX_TOKEN-1) ) {
			L_ScriptError( script, "primitive token longer than MAX_TOKEN = %d", MAX_TOKEN );
			return FALSE;
		}
		token->string[len++] = *script->script_p++;
	}

	token->string[len] = '\0';
	
	//copy the token into the script structure
	//
	N_Memcpy( &script->token, token, sizeof(script->token) );
	
	return TRUE;
}


/*
=====================
	L_ReadToken
=====================
*/
BOOL L_ReadToken( script_t *script, token_t *token ) {
	int t;
	
	//check arguments validity
	//
	if( (script == NULL) || (token == NULL) ) {
		return FALSE;
	}

	//if there is a token available (from L_UnreadToken)
	//
	if( script->tokenavailable ) {
		script->tokenavailable = FALSE;
		N_Memcpy( token, &script->token, sizeof(*token) );
		return TRUE;
	}
	
	//save script state
	//
	script->lastscript_p = script->script_p;
	script->lastline = script->line;
	
	//init token
	//
	N_Memset( token, 0, sizeof(*token) );
	
	//start of the white space
	//
	script->whitespace_p = script->script_p;
	token->whitespace_p = script->script_p;

	//read garbage
	//
	if( !L_ReadWhiteSpace( script ) ) {
		return FALSE;
	}
	
	//end of the white space
	//
	script->endwhitespace_p = script->script_p;
	token->endwhitespace_p = script->script_p;
	
	//line state
	//
	token->line = script->line;
	token->linescrossed = script->line - script->lastline;
	
	//if a string
	//
	if( *script->script_p == '\"' ) {
		if( !L_ReadString( script, token, '\"' ) ) {
			return FALSE;
		}
	}
	//if a literal
	//
	else if( *script->script_p == '\'' ) {
		if( !L_ReadLiteral( script, token ) ) {
			return FALSE;
		}
	}
	//if a number
	//
	else if( (*script->script_p >= '0' && *script->script_p <= '9') ||
			(*script->script_p == '.' &&
			(*(script->script_p+1) >= '0' && *(script->script_p+1) <= '9')) ) {

		if( !L_ReadNumber( script, token ) ) {
			return FALSE;
		}
	}
	//if this is a primitive script
	//
	else if( script->flags & SCFL_PRIMITIVE ) {
		return L_ReadPrimitive( script, token );
	}
	//if there is a name
	//
	else if( (*script->script_p >= 'a' && *script->script_p <= 'z') ||
			(*script->script_p >= 'A' && *script->script_p <= 'Z') ||
			*script->script_p == '_' ) {
		
		if( !L_ReadName( script, token ) ) {
			return FALSE;
		}
		
		//determine if TT_IDENTIFIER or TT_KEYWORD
		//
		t = L_LookUpName( script, token );
		if( t ) {
			token->type = TT_KEYWORD;
			token->subtype = t;
		}
		else {
			token->type = TT_IDENTIFIER;
		}
		
	}
	//check for punctuations
	//
	else if( !L_ReadPunctuation( script, token ) ) {
		L_ScriptError( script, "can't read token" );
		return FALSE;
	}
	
	//copy the token into the script structure
	//
	N_Memcpy( &script->token, token, sizeof(script->token) );
	
	return TRUE;
}


/*
=====================
	L_ExpectTokenString
=====================
*/
BOOL L_ExpectTokenString( script_t *script, char *string ) {
	token_t token;
	
	//check arguments validity
	//
	if( (script == NULL) || (string == NULL) ) {
		return FALSE;
	}
	
	if( !L_ReadToken( script, &token ) ) {
		L_ScriptError( script, "couldn't find expected %s", string );
		return FALSE;
	}
	
	if( N_Strcmp( token.string, string ) ) {
		L_ScriptError( script, "expected %s, found %s", string, token.string );
		return FALSE;
	}
	
	return TRUE;
}


/*
=====================
	L_ExpectTokenType
=====================
*/
BOOL L_ExpectTokenType( script_t *script, int type, int subtype, token_t *token ) {
	char str[MAX_TOKEN];

	//check arguments validity
	//
	if( (script == NULL) || (token == NULL) ) {
		return FALSE;
	}
	
	//try to read a valid token
	//
	while( TRUE ) {
		if( L_ReadToken( script, token ) ) {
			break;
		}
		else {
			//try to skip current line and make another
			//attempt at reading a token
			//
			if( !L_SkipLine( script ) ) {
				token->type = TT_END;
				return FALSE;
			}
		}
	}//while

	//check if token is not what we are looking for
	//
	if( token->type != type ) {
		//format message
		//
		switch( type ) {
			case TT_STRING: N_Strcpy( str, "string" ); break;
			case TT_LITERAL: N_Strcpy( str, "literal" ); break;
			case TT_NUMBER: N_Strcpy( str, "number" ); break;
			case TT_NAME: N_Strcpy( str, "name" ); break;
			case TT_PUNCTUATION: N_Strcpy( str, "punctuation" ); break;
			case TT_KEYWORD: N_Strcpy( str, "keyword" ); break;
			case TT_IDENTIFIER: N_Strcpy( str, "identifier" ); break;
			default: break;
		}
		L_ScriptError( script, "expected %s, found %s", str, token->string );
		return FALSE;
	}
	
	//check number token
	//
	if( token->type == TT_NUMBER ) {
		if( (token->subtype & subtype) != subtype ) {
			//format message
			//
			if( subtype & TT_DECIMAL ) N_Strcpy( str, "decimal" );
			if( subtype & TT_HEX ) N_Strcpy( str, "hex" );
			if( subtype & TT_OCTAL ) N_Strcpy( str, "octal" );
			if( subtype & TT_BINARY ) N_Strcpy( str, "binary" );
			
			if( subtype & TT_LONG ) N_Strcat( str, " long" );
			if( subtype & TT_UNSIGNED ) N_Strcat( str, " unsigned" );
			if( subtype & TT_FLOAT ) N_Strcat( str, " float" );
			if( subtype & TT_INTEGER ) N_Strcat( str, " integer" );
			
			L_ScriptError( script, "expected %s, found %s", str, token->string );

			return FALSE;
		}
	}
	//check punctuation token
	//
	else if( token->type == TT_PUNCTUATION ) {
		if( subtype < 0 ) {
			L_ScriptError( script, "BUG: wrong punctuation subtype" );
			return FALSE;
		}

		if( (token->subtype & subtype) != subtype ) {
			L_ScriptError( script, "expected %s, found %s",
				script->punctuations[subtype].p, token->string );
			return FALSE;
		}
	}
	
	//check keyword token
	//
	else if( token->type == TT_KEYWORD ) {
		if( (token->subtype & subtype) != subtype ) {
			L_ScriptError( script, "expected %s, found %s",
				script->keywords[subtype].p, token->string );
			return FALSE;
		}
	}

	return TRUE;
}


/*
=====================
	L_ExpectAnyToken
=====================
*/
BOOL L_ExpectAnyToken( script_t *script, token_t *token ) {
	//check arguments validity
	//
	if( (script == NULL) || (token == NULL) ) {
		return FALSE;
	}
	
	//try to read a valid token
	//
	while( TRUE ) {
		if( L_ReadToken( script, token ) ) {
			break;
		}
		else {
			//try to skip current line and make another
			//attempt at reading a token
			//
			if( !L_SkipLine( script ) ) {
				token->type = TT_END;
				return FALSE;
			}
		}
	}//while
	
	return TRUE;
}


/*
=====================
	L_CheckTokenString
=====================
*/
BOOL L_CheckTokenString( script_t *script, char *string ) {
	token_t tok;
	
	//check arguments validity
	//
	if( (script == NULL) || (string == NULL) ) {
		return FALSE;
	}
	
	if( !L_ReadToken( script, &tok ) ) {
		return FALSE;
	}
	
	if( !N_Strcmp( tok.string, string ) ) {
		return TRUE;
	}
	
	//token not available
	//
	script->script_p = script->lastscript_p;
	
	return FALSE;
}


/*
=====================
	L_CheckTokenType
=====================
*/
BOOL L_CheckTokenType( script_t *script, int type, int subtype, token_t *token ) {
	token_t tok;
	
	//check arguments validity
	//
	if( (script == NULL) || (token == NULL) ) {
		return FALSE;
	}

	//check if possible to read a token
	//
	if( !L_ReadToken( script, &tok ) ) {
		return FALSE;
	}
	
	//check if types match
	//
	if( (tok.type == type) && ((tok.subtype & subtype) == subtype) ) {
		N_Memcpy( token, &tok, sizeof(*token) );
		return TRUE;
	}
	
	//token not available
	//
	script->script_p = script->lastscript_p;
	return FALSE;
}


/*
=====================
	L_SkipUntilString
=====================
*/
BOOL L_SkipUntilString( script_t *script, char *string ) {
	token_t token;
	
	//check arguments validity
	//
	if( (script == NULL) || (string == NULL) ) {
		return FALSE;
	}
	
	//skip until string is found
	//
	while( L_ReadToken( script, &token ) ) {
		if( !N_Strcmp( token.string, string ) ) {
			return TRUE;
		}
	}
	
	return FALSE;
}


/*
=====================
	L_UnreadLastToken
=====================
*/
void L_UnreadLastToken( script_t *script ) {
	//check arguments validity
	//
	if( script == NULL ) {
		return;
	}

	//unread last token
	//	
	script->tokenavailable = TRUE;
}


/*
=====================
	L_UnreadToken
=====================
*/
void L_UnreadToken( script_t *script, token_t *token ) {
	//check arguments validity
	//
	if( (script == NULL) || (token == NULL) ) {
		return;
	}
	
	//unread token
	//
	N_Memcpy( &script->token, token, sizeof(script->token) );
	script->tokenavailable = TRUE;
}


/*
=====================
	L_NextWhiteSpaceChar
=====================
*/
char L_NextWhiteSpaceChar( script_t *script ) {
	//check arguments
	//
	if( !script || !script->whitespace_p ) {
		return 0;
	}
	
	if( script->whitespace_p != script->endwhitespace_p ) {
		return *script->whitespace_p++;
	}
	
	return 0;
}


/*
=====================
	L_StripDoubleQuotes
=====================
*/
void L_StripDoubleQuotes( char *string ) {
	//check arguments validity
	//
	if( string == NULL ) {
		return;
	}
	
	if( *string == '\"' ) {
		N_Strcpy( string, string+1 );
	}
	
	if( string[N_Strlen(string)-1] == '\"' ) {
		string[N_Strlen(string)-1] = '\0';
	}
}


/*
=====================
	L_StripSingleQuotes
=====================
*/
void L_StripSingleQuotes( char *string ) {
	//check arguments validity
	//
	if( string == NULL ) {
		return;
	}
	
	if( *string == '\'' ) {
		N_Strcpy( string, string+1 );
	}
	
	if( string[N_Strlen(string)-1] == '\'' ) {
		string[N_Strlen(string)-1] = '\0';
	}
}


/*
=====================
	L_ReadSignedFloat
=====================
*/
long double L_ReadSignedFloat( script_t *script ) {
	token_t token;
	long double sign = 1;
	
	//check arguments validity
	//
	if( script == NULL ) {
		return 0;
	}
	
	L_ExpectAnyToken( script, &token );
	if( !N_Strcmp( token.string, "-" ) ) {
		sign = -1;
		L_ExpectTokenType( script, TT_NUMBER, 0, &token );
	}
	else if( token.type != TT_NUMBER ) {
		L_ScriptError( script, "expected float value, found %s", token.string );
	}
	
	return sign * token.floatvalue;
}


/*
=====================
	L_ReadSignedInt
=====================
*/
signed long int L_ReadSignedInt( script_t *script ) {
	token_t token;
	signed long int sign = 1;

	//check arguments validity
	//
	if( script == NULL ) {
		return 0;
	}
	
	L_ExpectAnyToken( script, &token );
	if( !N_Strcmp( token.string, "-" ) ) {
		sign = -1;
		L_ExpectTokenType( script, TT_NUMBER, TT_INTEGER, &token );
	}
	else if( (token.type != TT_NUMBER) || (token.subtype == TT_FLOAT) ) {
		L_ScriptError( script, "expected integer value, found %s", token.string );
	}
	
	return sign * token.intvalue;
}


/*
=====================
	L_SetScriptFlags
=====================
*/
void L_SetScriptFlags( script_t *script, int flags ) {
	script->flags = flags;
}


/*
=====================
	L_ResetScript
=====================
*/
void L_ResetScript( script_t *script ) {
	//check arguments validity
	//
	if( script == NULL ) {
		return;
	}

	//set script pointer
	//
	script->script_p = 
	script->lastscript_p = script->buffer;
	
	//set white space pointer
	//
	script->whitespace_p =
	script->endwhitespace_p = NULL;
	
	//set token
	//
	script->tokenavailable = FALSE;
	N_Memset( &script->token, 0, sizeof(script->token) );
	
	//line position
	//
	script->line = 
	script->lastline = 1;
}


/*
=====================
	L_EndOfScript
=====================
*/
BOOL L_EndOfScript( script_t *script ) {
	//check arguments validity
	//
	if( !script ) {
		return FALSE;
	}

	return (script->script_p >= script->end_p);
}


/*
=====================
	L_NumLinesCrossed
=====================
*/
int L_NumLinesCrossed( script_t *script ) {
	//check arguments validity
	//
	if( script == NULL ) {
		return 0;
	}

	return script->line - script->lastline;
}


/*
=====================
	L_ScriptSkipTo
=====================
*/
BOOL L_ScriptSkipTo( script_t *script, char *value ) {
	int len;
	
	//check arguments validity
	//
	if( (script == NULL) || (value == NULL) ) {
		return FALSE;
	}
	
	len = N_Strlen( value );
	
	do {
		if( !L_ReadWhiteSpace( script ) ) {
			return FALSE;
		}
		
		if( *script->script_p == *value ) {
			if( !N_Strncmp( script->script_p, value, len ) ) {
				return TRUE;
			}
		}
		script->script_p++;
	}while( TRUE );
}


/*
=====================
	L_LoadScriptMemory
=====================
*/
script_t * L_LoadScriptMemory( char *ptr, int length, char *name ) {
	void *buffer;
	script_t *script;
	
	//check arguments
	//
	if( !ptr || !name ) {
		return NULL;
	}
	
	//allocate memory
	//
	buffer = N_Malloc( sizeof(script_t) + length + 1 );
	script = (script_t *)buffer;

	//set script
	//
	N_Memset( script, 0, sizeof(*script) );
	N_Strncpy( script->szFileName, name, MAX_PATH );
	script->buffer = (char *)buffer + sizeof(script_t);
	script->buffer[length] = 0;
	script->length = length;
	
	//set script pointers
	//
	script->script_p = script->buffer;
	script->lastscript_p = script->buffer;
	script->end_p = &script->buffer[length];
	
	script->tokenavailable = FALSE;
	
	script->line = 1;
	script->lastline = 1;
	
	//set script punctuations and keywords
	//
	L_SetScriptPunctuations( script, NULL );
	L_SetScriptKeywords( script, NULL );
	
	N_Memcpy( script->buffer, ptr, length );
	
	return script;
}


/*
=====================
	L_LoadScriptFile
=====================
*/
script_t * L_LoadScriptFile( char *filename, char *name ) {
	HANDLE hFile;
	DWORD length;
	void *buffer;
	script_t *script;
	
	//check arguments
	//
	if( !filename || !name ) {
		return NULL;
	}
	
	hFile = N_FOpenR( filename );
	if( hFile == INVALID_HANDLE_VALUE ) {
		return NULL;
	}
	
	//allocate memory
	//
	length = GetFileSize( hFile, NULL );
	buffer = N_Malloc( sizeof(script_t) + length + 1 );
	script = (script_t *)buffer;
	
	//set script
	//
	N_Memset( script, 0, sizeof(*script) );
	N_Strncpy( script->szFileName, name, MAX_PATH );
	script->buffer = (char *)buffer + sizeof(script_t);
	script->buffer[length] = 0;
	script->length = length;
	
	//set script pointers
	//
	script->script_p = script->buffer;
	script->lastscript_p = script->buffer;
	script->end_p = &script->buffer[length];
	
	script->tokenavailable = FALSE;
	
	script->line = 1;
	script->lastline = 1;
	
	//set script punctuations and keywords
	//
	L_SetScriptPunctuations( script, NULL );
	L_SetScriptKeywords( script, NULL );
	
	if( !N_FRead( hFile, script->buffer, length ) ) {
		N_Free( buffer );
		script = NULL;
	}
	
	N_FClose( hFile );
	
	return script;
}


/*
=====================
	L_FreeScript
=====================
*/
void L_FreeScript( script_t *script ) {
	//check arguments
	//
	if( !script ) {
		return;
	}

	//free punctuation table
	if( script->punctuationtable ) {
		N_Free( script->punctuationtable );
	}
	
	//free script
	N_Free( script );
}