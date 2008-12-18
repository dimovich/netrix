
//NetrixC interpreter
//
//Code based on:
//"Mini C++ Interpreter" code (by Herbert Schildt);
//

#undef __N_FILE__
#define __N_FILE__ TEXT( "(nxc) interpreter.c" )

#include "compile.h"
#include <windows.h>
#include <stdarg.h>
#include <tchar.h>
#include <stdio.h>

#include "../netrixlib/netrixlib.h"
#include "lexer.h"
#include "parser.h"
#include "interpreter.h"


//set this to the program to be executed
//
program_t *kProgram;

//last read token
//
itoken_t giToken;

//programmer created function return value
//
value_t gReturnValue;

static BOOL gBreakFound;
static BOOL gReturn;


/*
=====================
	I_Init
=====================
*/
BOOL I_Init( program_t *program ) {
	int id;

	//check arguments validity
	//
	if( program == NULL ) {
		return FALSE;
	}
	
	//set program
	//
	kProgram = program;

	//set globals
	//
	gBreakFound = FALSE;
	gReturn = FALSE;
	gReturnValue.type = K_INT;
	gReturnValue.v.i = 0;
	
	N_Memset( &giToken, 0, sizeof(itoken_t) );
	
	//convert script to internal format
	//
	I_Convert();
	
	//prescan script
	//
	I_Prescan();

	//check if Init() and Update() are present
	//
	if( !I_FindFuncName( "Update", &id ) ) {
		N_Trace( "could not find Update() function" );
		return FALSE;
	}
	
	if( !I_FindFuncName( "Init", &id ) ) {
		N_Trace( "could not find Init() function" );
		return FALSE;
	}
	
	//call Init() function
	//
	kProgram->itokar.idx = kProgram->funcTable[id].idx;
	giToken.subtype = kProgram->funcTable[id].stridx;
	giToken.type = TT_IDENTIFIER;
	I_Call();

	return TRUE;
}


/*
=====================
	I_Update
=====================
*/
void I_Update( program_t *program ) {
	int id;
	
	kProgram = program;
	gBreakFound = FALSE;
	gReturn = FALSE;
	gReturnValue.type = K_INT;
	gReturnValue.v.i = 0;
	N_Memset( &giToken, 0, sizeof(itoken_t) );

	if( !I_FindFuncName( "Update", &id ) ) {
		N_Trace( "could not find Update() function" );
		return;
	}
	
	//call Update() function
	//
	kProgram->itokar.idx = kProgram->funcTable[id].idx;
	giToken.subtype = kProgram->funcTable[id].stridx;
	giToken.type = TT_IDENTIFIER;
	I_Call();
}


#define CHUNK_SIZE 64
/*
=====================
	I_Convert
=====================
*/
void I_Convert() {
	itokar_t *itokar = NULL;
	script_t *script = NULL;
	token_t token = {0};
	int index = 0;
	int size = 0;
	
	//check system
	if( !kProgram ) {
		return;
	}
	
	itokar = &kProgram->itokar;
	script = kProgram->script;
	
	while( L_ExpectAnyToken( script, &token ) ) {
		//increase array
		//
		if( index >= size ) {
			size += CHUNK_SIZE;
			if( itokar->ar ) {
				itokar->ar = N_Realloc( itokar->ar, size*sizeof(itoken_t) );
			}
			else {
				itokar->ar = N_Malloc( size*sizeof(itoken_t) );
			}
		}
		
		//set itoken
		//
		itokar->ar[index].type = token.type;
		itokar->ar[index].p = script->endwhitespace_p;
		
		//set itoken subtype
		//
		switch( token.type ) {
			case TT_IDENTIFIER:
			case TT_STRING:
				itokar->ar[index].subtype = I_AddString( token.string );
				break;
			case TT_NUMBER:
				itokar->ar[index].subtype = I_AddValue( token.subtype,
					token.intvalue, token.floatvalue );
				break;
			case TT_LITERAL:
			case TT_PUNCTUATION:
			case TT_KEYWORD:
				itokar->ar[index].subtype = token.subtype;
				break;
		}

		index++;
	}
	
	itokar->size = index;
	itokar->idx = 0;
}


/*
=====================
	I_AddString
=====================
*/
int I_AddString( char *string ) {
	unsigned long hash;
	int i;
	
	//check if enough space
	//
	if( kProgram->stringTableSize >= MAX_STRINGS ) {
		I_ScriptError( "too many strings" );
		return 0;
	}
	
	//calculate hash
	//
	hash = N_CalcHash( string );
	
	//check for duplicate string
	//
	for( i=0; i<kProgram->stringTableSize; i++ ) {
		if( hash == kProgram->stringTable[i].hash ) {
			return i;
		}
	}
	
	//add new string
	//
	kProgram->stringTable[kProgram->stringTableSize].hash = hash;
	N_Strcpy( kProgram->stringTable[kProgram->stringTableSize].string, string );
	
	return kProgram->stringTableSize++;
}


/*
=====================
	I_AddValue
=====================
*/
int I_AddValue( int subtype, long int intvalue, float floatvalue ) {
	value_t value = {K_INT, 0};
	int i;
	
	//check if enough space
	//
	if( kProgram->valueTableSize >= MAX_VALUES ) {
		I_ScriptError( "too many number literals" );
		return 0;
	}
	
	//set value
	//
	if( subtype & TT_FLOAT ) {
		value.type = K_FLOAT;
		value.v.f = floatvalue;
	}
	else if( subtype & TT_INTEGER ) {
		value.type = K_INT;
		value.v.i = intvalue;
	}
	
	//check for duplicate value
	//
	for( i=0; i<kProgram->valueTableSize; i++ ) {
		if( (value.type == kProgram->valueTable[i].type) &&
				(value.v.i == kProgram->valueTable[i].v.i) ) {
			return i;
		}
	}
	
	//add new value
	//
	N_Memcpy( &kProgram->valueTable[kProgram->valueTableSize], &value, sizeof(value_t) );
	
	return kProgram->valueTableSize++;
}


/*
=====================
	I_ScriptError
=====================
*/
void I_ScriptError( char *format, ... ) {
	char buff[1024] = "";
	char *endp = NULL;
	char *p = NULL;
	va_list va;
	int line = 1;

	if( kProgram && kProgram->itokar.ar && kProgram->script ) {

		va_start( va, format );
		_vsntprintf( buff, 1024, format, va );
		va_end( va );
	
		//determine line
		//
		if( kProgram->itokar.idx <= kProgram->itokar.size ) {
			endp = kProgram->itokar.ar[kProgram->itokar.idx-1].p;
			p = kProgram->script->buffer;
			while( p && (p != endp) ) {
				if( *p == '\r' ) {
					line++;
				}
				p++;
			}
		}
	
		//display error message
		//
		N_Trace( "error: file %s, line %d: %s\n",
			kProgram->script->szFileName, line, buff );
	}
}


/*
=====================
	I_SkipUntilToken
=====================
*/
void I_SkipUntilToken( int type, int subtype ) {
	if( kProgram && kProgram->itokar.ar ) {
		do {
			I_ReadToken();
		}while( !(giToken.type == type && giToken.subtype == subtype) );
	}
}


/*
=====================
	I_Interpret
=====================
*/
void I_Interpret() {
	value_t value = {K_FLOAT, 0};
	int block = 0;
	
	//check system
	if( !kProgram || !kProgram->itokar.ar ) {
		return;
	}
	
	do {
		//don't interpret until "break" or "return" is handled
		//
		if( gBreakFound || gReturn ) {
			return;
		}
		
		//read next token
		//
		I_ReadToken();
		
		//check if identifier, "++" or "--"
		//
		if( (giToken.type == TT_IDENTIFIER) || (giToken.type == TT_PUNCTUATION &&
				(giToken.subtype == P_INC || giToken.subtype == P_DEC)) ) {
			I_UnreadLastToken();
			P_EvalExp( &value );
			if( !(giToken.type == TT_PUNCTUATION && giToken.subtype == P_SEMICOLON) ) {
				I_ScriptError( "semicolon expected" );
				return;
			}
		}
		//check if block delimiter
		//
		else if( giToken.type == TT_PUNCTUATION ){
			if( giToken.subtype == P_BRACEOPEN ) {
				block = 1;
				kProgram->nestStack[kProgram->nestStackSize++] = kProgram->varStackSize;
			}
			else if( giToken.subtype == P_BRACECLOSE ) {
				kProgram->varStackSize = kProgram->nestStack[--kProgram->nestStackSize];
				return;
			}
		}
		//process keyword
		//
		else if( giToken.type == TT_KEYWORD ) {
			switch( giToken.subtype ) {
				//declare local variable
				//
				case K_INT:
				case K_FLOAT:
					I_UnreadLastToken();
					I_DeclareLocal();
					break;
				//return from function call
				//
				case K_RETURN:
					I_FunctionReturn();
					return;
				//process an "if" statement
				//
				case K_IF:
					I_ExecuteIf();
					break;
				//process an "else" statement
				//
				case K_ELSE:
					I_FindEob();
					break;
				//process a "while" loop
				//
				case K_WHILE:
					I_ExecuteWhile();
					break;
				//process a "do-while" loop
				//
				case K_DO:
					I_ExecuteDo();
					break;
				//process a "for" loop
				//
				case K_FOR:
					I_ExecuteFor();
					break;
				//handle "break"
				//
				case K_BREAK:
					gBreakFound = TRUE;
					kProgram->varStackSize = kProgram->nestStack[--kProgram->nestStackSize];
					return;
				//handle a "switch" statement
				//
				case K_SWITCH:
					I_ExecuteSwitch();
					break;

				default:
					break;
			}
		}
		//TT_END
		//
		else if( giToken.type == TT_END ) {
			I_ScriptError( "end of script" );
			return;
		}
	}while( (giToken.type != TT_END) && block );
}


/*
=====================
	I_Prescan
=====================
*/
void I_Prescan() {
	int brace = 0;
	int idx;
	int tmpidx;
	
	//check system
	if( !kProgram || !kProgram->itokar.ar ) {
		return;
	}
	
	//backup
	//
	idx = kProgram->itokar.idx;

	do {
		//bypass code inside functions
		//
		while( brace ) {
			I_ReadToken();
			
			if( giToken.type == TT_END ) {
				I_ScriptError( "unbalanced braces" );
				return;
			}

			if( giToken.type == TT_PUNCTUATION  ) {
				if( giToken.subtype == P_BRACEOPEN ) {
					brace++;
				}
				else if( giToken.subtype == P_BRACECLOSE ) {
					brace--;
				}
			}
		}//while
		
		//save current position
		//
		tmpidx = kProgram->itokar.idx;

		//read next token
		//
		I_ReadToken();
		
		//process token
		//
		if( giToken.type == TT_KEYWORD ) {
			if( giToken.subtype == K_INT || giToken.subtype == K_FLOAT ) {

				//read name
				//
				I_ReadToken();
				
				if( giToken.type == TT_IDENTIFIER ) {
				
					//check punctuation
					//
					I_ReadToken();
					
					//declare a function
					//
					if( (giToken.type == TT_PUNCTUATION) && (giToken.subtype == P_PARENTHESESOPEN) ) {
						kProgram->itokar.idx = tmpidx;
						I_DeclareFunction();
					}
					//global variable
					//
					else {
						kProgram->itokar.idx = tmpidx;
						I_DeclareGlobal();
					}
				}
				//not an identifier
				//
				else {
					I_UnreadLastToken();
				}
			}
		}
		//not a keyword
		//
		else {
			if( giToken.type == TT_PUNCTUATION ) {
				if( giToken.subtype == P_BRACEOPEN ) {
					brace++;
				}
				else if( giToken.subtype == P_BRACECLOSE ) {
					brace--;
				}
			}
		}
	}while( giToken.type != TT_END );
	
	if( brace ) {
		I_ScriptError( "unbalanced braces" );
		return;
	}
	
	//restore script
	//
	kProgram->itokar.idx = idx;
}


/*
=====================
	I_DeclareGlobal
=====================
*/
void I_DeclareGlobal() {
	int i;
	int type;
	var_t var;
	
	//check system
	if( !kProgram || !kProgram->itokar.ar ) {
		return;
	}
	
	//read type
	//
	I_ReadToken();
	type = giToken.subtype;
	
	//process comma-separated list
	//
	do {
		//check if enough space in globals table
		//
		if( kProgram->globalVarsSize >= MAX_GLOBALS ) {
			I_ScriptError( "too many global variables" );
			return;
		}
	
		var.value.type = type;
		var.value.v.i = 0;
		
		//get name
		//
		I_ReadToken();
		
		//check if variable is duplicate
		//
		for( i=0; i<kProgram->globalVarsSize; i++ ) {
			if( giToken.subtype == kProgram->globalVars[i].stridx ) {
				I_ScriptError( "duplicate global variable: %s",
					kProgram->stringTable[giToken.subtype].string );
				return;
			}
		}
	
		var.stridx = giToken.subtype;
		i = kProgram->globalVarsSize++;
		N_Memcpy( &kProgram->globalVars[i], &var, sizeof(var_t) );
		
		//get next variable
		//
		I_ReadToken();

	}while( (giToken.type == TT_PUNCTUATION) && (giToken.subtype == P_COMMA) );
	
	//check for ";"
	//
	if( !( giToken.type == TT_PUNCTUATION && giToken.subtype == P_SEMICOLON  ) ) {
		I_ScriptError( "semicolon expected" );
		return;
	}
}


/*
=====================
	I_DeclareFunction
=====================
*/
void I_DeclareFunction() {
	function_t ft = {0};
	int rettype;
	int count;
	int i;

	//check system
	if( !kProgram || !kProgram->itokar.ar ) {
		return;
	}

	//check if enough space in function table
	//
	if( kProgram->funcTableSize >= MAX_FUNCTIONS ) {
		I_ScriptError( "too many functions" );
		return;
	}
	
	//read return type
	//
	I_ReadToken();
	rettype = giToken.subtype;
	
	//read function name
	//
	I_ReadToken();
	
	//check for duplicate functions
	//
	for( i=0; i<kProgram->funcTableSize; i++ ) {
		if( giToken.subtype == kProgram->funcTable[i].stridx ) {
			I_ScriptError( "duplicate function found: %s",
				 kProgram->stringTable[giToken.subtype].string);
			return;
		}
	}
	
	//set function
	//
	ft.idx = kProgram->itokar.idx;
	ft.rettype = rettype;
	ft.stridx = giToken.subtype;
	i = kProgram->funcTableSize++;
	N_Memcpy( &kProgram->funcTable[i], &ft, sizeof(function_t) );
	
	//read parameter types
	//
	count = 0;
	while( TRUE ) {
		//check if enough space
		//
		if( count >= MAX_PARAMS ) {
			I_ScriptError( "too many parameters" );
			return;
		}
	
		I_ReadToken();
		
		if( (giToken.type == TT_KEYWORD) ) {
			if( giToken.subtype == K_INT || giToken.subtype == K_FLOAT ) {
				//set type
				//
				kProgram->funcTable[i].params[count].value.type = giToken.subtype;

				//set name
				//
				I_ReadToken();
				kProgram->funcTable[i].params[count].stridx = giToken.subtype;
				
				count++;
			}
		}
		
		if( giToken.type == TT_END ) {
			break;
		}
		if( giToken.type == TT_PUNCTUATION && giToken.subtype == P_PARENTHESESCLOSE ) {
			break;
		}
	}
	kProgram->funcTable[i].cparam = count;
}


/*
=====================
	I_FindFunc
=====================
*/
BOOL I_FindFunc( int stridx, int *id ) {
	int i;
	
	if( kProgram ) {
		for( i=0; i<kProgram->funcTableSize; i++ ) {
			if( kProgram->funcTable[i].stridx == stridx ) {
				*id = i;
				return TRUE;
			}
		}
	}
	
	return FALSE;
}


/*
=====================
	I_FindFuncName
=====================
*/
BOOL I_FindFuncName( char *name, int *id ) {
	int i;
	int stridx;
	unsigned long hash;
	
	if( kProgram ) {
		//calculate hash for the name
		//
		hash = N_CalcHash( name );
		
		//search function name
		//
		for( i=0; i<kProgram->funcTableSize; i++ ) {
			stridx = kProgram->funcTable[i].stridx;
			if( hash == kProgram->stringTable[stridx].hash ) {
				*id = i;
				return TRUE;
			}
		}
	}
	
	return FALSE;
}


/*
=====================
	I_DeclareLocal
=====================
*/
void I_DeclareLocal() {
	var_t vt;
	int i;
	int top;
	
	//check system
	if( !kProgram || !kProgram->itokar.ar ) {
		return;
	}
	
	//get type
	//
	I_ReadToken();
	vt.value.type = giToken.subtype;
	vt.value.v.i = 0;
	
	//process comma-separated list
	//
	do {
		//check if enough space
		//
		if( kProgram->varStackSize >= MAX_STACK ) {
			I_ScriptError( "too many local variables" );
			return;
		}
		
		//get name
		//
		I_ReadToken();
		
		//check for duplicate name
		//
		if( kProgram->varStackSize > 0 ) {
			top = kProgram->nestStackSize-1;
			for( i=(kProgram->varStackSize-1); i>= kProgram->nestStack[top]; i-- ) {
				if( giToken.subtype == kProgram->varStack[i].stridx ) {
					I_ScriptError( "duplicate local variable: %s",
						kProgram->stringTable[giToken.subtype].string );
					return;
				}
			}
		}//if
		
		vt.stridx = giToken.subtype;
		i = kProgram->varStackSize++;
		N_Memcpy( &kProgram->varStack[i], &vt, sizeof(var_t) );
		
		I_ReadToken();
		
	}while( (giToken.type == TT_PUNCTUATION) && (giToken.subtype == P_COMMA) );
	
	//check for ";"
	//
	if( ! (giToken.type = TT_PUNCTUATION && giToken.subtype == P_SEMICOLON)  ) {
		I_ScriptError( "semicolon expected" );
		return;
	}
}


/*
=====================
	I_Call
=====================
*/
void I_Call() {
	int tmpidx;
	int id;
	int i;
	int nestsize;
	int varsize;

	//check system
	if( !kProgram || !kProgram->itokar.ar ) {
		return;
	}

	//call function
	//
	if( I_FindFunc( giToken.subtype, &id ) ) {
	
		//save stack sizes
		//
		nestsize = kProgram->nestStackSize;
		varsize = kProgram->varStackSize;
		
		//save function return type
		//
		i = kProgram->varStackSize++;
		kProgram->varStack[i].value.type = kProgram->funcTable[id].rettype;
		
		//get function arguments
		//
		I_GetArguments( &kProgram->funcTable[id] );
		
		//save stack position
		//
		i = kProgram->funcCallStackSize++;
		kProgram->funcCallStack[i] = varsize;

		//set script pointer
		//
		tmpidx = kProgram->itokar.idx;
		kProgram->itokar.idx = kProgram->funcTable[id].idx;
		I_SkipUntilToken( TT_PUNCTUATION, P_PARENTHESESCLOSE );
	
		//interpret
		//
		I_Interpret();
		
		gReturn = FALSE;

		//restore script state
		//
		kProgram->itokar.idx = tmpidx;

		//restore stack position
		//
		i = --kProgram->funcCallStackSize;
		kProgram->varStackSize = kProgram->funcCallStack[i];
		kProgram->nestStackSize = nestsize;
	}
	//undefined function
	//
	else {
		I_ScriptError( "undefined function: %s",
			kProgram->stringTable[giToken.subtype].string );
		return;
	}
}


/*
=====================
	I_GetArguments
=====================
*/
void I_GetArguments( function_t *func ) {
	value_t value = {K_FLOAT, 0};
	value_t tmpval;
	var_t temp[MAX_PARAMS];
	int count = 0;
	int i = 0;
	
	//check system
	if( !kProgram || !kProgram->itokar.ar ) {
		return;
	}
	
	I_ReadToken();

	if( ! (giToken.type == TT_PUNCTUATION && giToken.subtype == P_PARENTHESESOPEN) ) {
		I_ScriptError( "parentheses expected" );
		return;
	}
	
	//if any parameters, process
	//
	if( func->cparam > 0 ) {
		for( count=0; count<func->cparam; count++ ) {

			tmpval.type =
			value.type = func->params[count].value.type;
			P_EvalExp( &value );
			I_PromoteValue( &tmpval, &value );

			N_Memcpy( &temp[count].value, &value, sizeof(value_t) );
			temp[count].stridx = func->params[count].stridx;

			I_ReadToken();
		}
	}
	//get ")" token
	//
	else {
		I_ReadToken();
	}

	if( !(giToken.type == TT_PUNCTUATION && giToken.subtype == P_PARENTHESESCLOSE) ) {
		I_ScriptError( "parentheses expected" );
		return;
	}

	count--;
	
	//push on local stack in reverse order
	//
	for( ; count>=0; count-- ) {
		i = kProgram->varStackSize++;
		N_Memcpy( &kProgram->varStack[i], &temp[count], sizeof(var_t) );
	}
}


/*
=====================
	I_FunctionReturn
=====================
*/
void I_FunctionReturn() {
	value_t value;
	value_t tmpval;
	int i;

	if( kProgram ) {

		//determine return type
		//	
		i = kProgram->funcCallStack[kProgram->funcCallStackSize - 1];
		tmpval.type =
		value.type = kProgram->varStack[i].value.type;

		value.v.i = 0;

		P_EvalExp( &value );
		I_PromoteValue( &tmpval, &value );
		
		N_Memcpy( &gReturnValue, &value, sizeof(value_t) );
		
		gReturn = TRUE;
	}
}


/*
=====================
	I_AssignVariable
=====================
*/
void I_AssignVariable( int stridx, value_t *value ) {
	int i;
	int top;

	//check system
	if( !kProgram ) {
		return;
	}

	//check if it is a local variable
	//
	if( kProgram->varStackSize > 0 ) {
		top = kProgram->funcCallStack[kProgram->funcCallStackSize - 1];

		for( i = (kProgram->varStackSize - 1); i >= top; i-- ) {
			if( stridx == kProgram->varStack[i].stridx ) {
				I_PromoteValue( &kProgram->varStack[i].value, value );
				switch( kProgram->varStack[i].value.type ) {
					case K_INT:
						kProgram->varStack[i].value.v.i = value->v.i;
						break;
					case K_FLOAT:
						kProgram->varStack[i].value.v.f = value->v.f;
						break;
				}
				return;
			}
		}
	}
	
	//check if it is a global variable
	//
	for( i=0; i<kProgram->globalVarsSize; i++ ) {
		if( stridx == kProgram->globalVars[i].stridx ) {
			I_PromoteValue( &kProgram->globalVars[i].value, value );
			switch( kProgram->globalVars[i].value.type ) {
				case K_INT:
					kProgram->globalVars[i].value.v.i = value->v.i;
					break;
				case K_FLOAT:
					kProgram->globalVars[i].value.v.f = value->v.f;
					break;
			}
			return;
		}
	}
	
	I_ScriptError( "variable %s not found", kProgram->stringTable[stridx].string );
}


/*
=====================
	I_FindVariable
=====================
*/
BOOL I_FindVariable( int stridx, value_t *value ) {
	int i;
	int top;
	value_t partial_value;
	
	//check system
	if( !kProgram ) {
		return FALSE;
	}
	
	//check if it is a local variable
	//
	if( kProgram->varStackSize > 0 ) {
		top = kProgram->funcCallStack[kProgram->funcCallStackSize-1];
		for( i=(kProgram->varStackSize-1); i>=top; i-- ) {
			if( stridx == kProgram->varStack[i].stridx ) {
				//copy value
				//
				N_Memcpy( &partial_value, &kProgram->varStack[i].value, sizeof(value_t) );
				I_PromoteValue( value, &partial_value );
				N_Memcpy( value, &partial_value, sizeof(value_t) );
				return TRUE;
			}
		}
	}
	
	//check if it is a global variable
	//
	for( i=0; i<kProgram->globalVarsSize; i++ ) {
		if( stridx == kProgram->globalVars[i].stridx ) {
			//copy value
			//
			N_Memcpy( &partial_value, &kProgram->globalVars[i].value, sizeof(value_t) );
			I_PromoteValue( value, &partial_value );
			N_Memcpy( value, &partial_value, sizeof(value_t) );
			return TRUE;
		}
	}

	return FALSE;
}


/*
=====================
	I_IsVariable
=====================
*/
BOOL I_IsVariable( int stridx ) {
	int top;
	int i;

	//check local var stack
	//
	if( kProgram->varStackSize > 0 ) {
		top = kProgram->funcCallStack[kProgram->funcCallStackSize-1];

		for( i=(kProgram->varStackSize-1); i>=top; i-- ) {
			if( stridx == kProgram->varStack[i].stridx ) {
				return TRUE;
			}
		}
	}
	
	//check global vars
	//
	for( i=0; i<kProgram->globalVarsSize; i++ ) {
		if( stridx == kProgram->globalVars[i].stridx ) {
			return TRUE;
		}
	}
	
	return FALSE;
}


/*
=====================
	I_FindEob
=====================
*/
void I_FindEob() {
	int brace;
	
	//check system
	if( !kProgram || !kProgram->itokar.ar ) {
		return;
	}
	
	I_ReadToken();
	
	if( !(giToken.type == TT_PUNCTUATION && giToken.subtype == P_BRACEOPEN) ) {
		I_ScriptError( "brace expected" );
		return;
	}
	
	brace = 1;
	
	do {
		I_ReadToken();
		if( giToken.type == TT_PUNCTUATION ) {
			if( giToken.subtype == P_BRACEOPEN ) {
				brace++;
			}
			else if( giToken.subtype == P_BRACECLOSE ) {
				brace--;
			}
		}
	}while( brace && (giToken.type != TT_END) );
	
	if( giToken.type == TT_END ) {
		I_ScriptError( "unbalanced braces" );
		return;
	}
}


/*
=====================
	I_ExecuteIf
=====================
*/
void I_ExecuteIf() {
	value_t cond = {K_FLOAT, 0};
	
	//check system
	if( !kProgram || !kProgram->itokar.ar ) {
		return;
	}
	
	//get "if" expression
	//
	P_EvalExp( &cond );
	
	//if true, process target of "if"
	//
	if( (int)cond.v.f ) {
		//confirm start of block
		//
		if( !(giToken.type == TT_PUNCTUATION && giToken.subtype == P_BRACEOPEN ) ) {
			I_ScriptError( "brace expected" );
			return;
		}
		//interpret block
		//
		I_Interpret();
	}
	//process "else"
	//
	else {
		//find start "else"
		//
		I_FindEob();

		//confirm "else" presence
		//
		I_ReadToken();
		if( !(giToken.type == TT_KEYWORD && giToken.subtype == K_ELSE ) ) {
			//unread token if no "else" is present.
			//
			I_UnreadLastToken();
			return;
		}

		//confirm start of block
		//
		I_ReadToken();
		if( !(giToken.type == TT_PUNCTUATION && giToken.subtype == P_BRACEOPEN ) ) {
			I_ScriptError( "brace expected" );
			return;
		}
		I_UnreadLastToken();

		//interpret block
		//
		I_Interpret();
	}
}


/*
=====================
	I_ExecuteSwitch
=====================
*/
void I_ExecuteSwitch() {
	value_t sval = {K_FLOAT, 0};
	value_t cval = {K_FLOAT, 0};
	int top;
	int brace;
	
	//evaluate switch expression
	//
	P_EvalExp( &sval );
	
	//check for start of block
	//
	if( !(giToken.type == TT_PUNCTUATION && giToken.subtype == P_BRACEOPEN ) ) {
		I_ScriptError( "brace expected" );
		return;
	}
	
	//record new scope
	//
	top = kProgram->nestStackSize++;
	kProgram->nestStack[top] = kProgram->varStackSize;
	
	//check case statements
	//
	while( TRUE ) {
		brace = 1;
		
		//find a case statement
		//
		do {
			I_ReadToken();
			if( giToken.type == TT_PUNCTUATION ) {
				if( giToken.subtype == P_BRACEOPEN ) {
					brace++;
				}
				else if( giToken.subtype == P_BRACECLOSE ) {
					brace--;
				}
			}
			if( (giToken.type == TT_KEYWORD) && (giToken.subtype == K_CASE) ) {
				break;
			}
		}while( brace && (giToken.type != TT_END) );
		
		//if no matching case found, then skip
		//
		if( !brace ) {
			break;
		}
		
		if( giToken.type == TT_END ) {
			I_ScriptError( "(switch) syntax error" );
			return;
		}
		
		//get value of the case statement
		//
		P_EvalExp( &cval );
		
		//discard ':'
		//
		I_ReadToken();
		if( !(giToken.type == TT_PUNCTUATION && giToken.subtype == P_COLON) ) {
			I_ScriptError( "colon expected" );
			return;
		}
		
		//if values match, then interpret
		//
		if( (int)sval.v.f == (int)cval.v.f ) {
			brace = 1;
			do {
				//Note: I_Interpret() will automatically restore varStack
				//when "}" is read.
				//
				I_Interpret();
				if( (giToken.type == TT_PUNCTUATION) && (giToken.subtype == P_BRACECLOSE) ) {
					brace--;
				}
			}while( !gBreakFound && (giToken.type != TT_END) && brace );
			
			//find end of switch statement
			//
			while( brace && (giToken.type != TT_END) ) {
				I_ReadToken();
				if( giToken.type == TT_PUNCTUATION ) {
					if( giToken.subtype == P_BRACEOPEN ) {
						brace++;
					}
					else if( giToken.subtype == P_BRACECLOSE ) {
						brace--;
					}
				}
			}
			gBreakFound = FALSE;
			break;
		}//if
	}//while
}


/*
=====================
	I_ExecuteWhile
=====================
*/
void I_ExecuteWhile() {
	value_t cond = {K_FLOAT, 0};
	int tmpidx;
	int blockidx;
	
	//save location of top of "while" loop
	//
	I_UnreadLastToken();
	tmpidx = kProgram->itokar.idx;

	I_ReadToken(); //skip "while" token
	P_EvalExp( &cond );
	
	//confirm start of block
	//
	if( !(giToken.type == TT_PUNCTUATION && giToken.subtype == P_BRACEOPEN ) ) {
		I_ScriptError( "brace expected" );
		return;
	}
	
	blockidx = kProgram->itokar.idx;
	
	//interpret
	//
	if( (int)cond.v.f ) {
		I_Interpret();
	}
	else {
		I_FindEob();
		return;
	}
	
	//check for break in loop
	//
	if( gBreakFound ) {
		gBreakFound = FALSE;
		kProgram->itokar.idx = blockidx;
		I_FindEob();
		return;
	}
	
	//loop back to top
	//
	kProgram->itokar.idx = tmpidx;
}


/*
=====================
	I_ExecuteDo
=====================
*/
void I_ExecuteDo() {
	value_t cond = {K_FLOAT, 0};
	int tmpidx;
	int blockidx;
	
	//save location of top of "do" loop
	//
	I_UnreadLastToken();
	tmpidx = kProgram->itokar.idx;

	I_ReadToken(); //skip "do" token
	
	//confirm start of block
	//
	I_ReadToken();
	if( !(giToken.type == TT_PUNCTUATION && giToken.subtype == P_BRACEOPEN) ) {
		I_ScriptError( "brace expected" );
		return;
	}
	I_UnreadLastToken();
	
	blockidx = kProgram->itokar.idx;
	
	//interpret loop
	//
	I_Interpret();
	
	//check for break in loop
	//
	if( gBreakFound ) {
		kProgram->itokar.idx = blockidx;
		I_FindEob();
		
		//find end of while expression
		//
		do {
			I_ReadToken();
			if( (giToken.type == TT_PUNCTUATION) && (giToken.subtype == P_SEMICOLON) ) {
				break;
			}
		}while( giToken.type != TT_END );

		if( giToken.type == TT_END ) {
			I_ScriptError( "(do-while) syntax error" );
			return;
		}

		gBreakFound = FALSE;
		return;
	}

	//check if "while" token is present
	//
	I_ReadToken();
	if( !(giToken.type == TT_KEYWORD && giToken.subtype == K_WHILE ) ) {
		I_ScriptError( "while expected" );
		return;
	}

	//evaluate while expression
	//
	P_EvalExp( &cond );

	//if TRUE loop; otherwise, continue on
	//
	if( (int)cond.v.f ) {
		kProgram->itokar.idx = tmpidx;
	}
}


/*
=====================
	I_ExecuteFor
=====================
*/
void I_ExecuteFor() {
	value_t cond = {K_FLOAT, 0};
	int tmpidx1, tmpidx2;
	int blockidx;
	int paren;
	BOOL init = FALSE;
	
	//skip "("
	//
	I_ReadToken();
	
	//initialization expression
	//
	P_EvalExp( &cond );
	
	if( !(giToken.type == TT_PUNCTUATION && giToken.subtype == P_SEMICOLON ) ) {
		I_ScriptError( "semicolon expected" );
		return;
	}

	//skip ";"
	//
	I_ReadToken();

	tmpidx1 = kProgram->itokar.idx;
	
	while( TRUE ) {
		//evaluate conditional expression
		//
		P_EvalExp( &cond );
		
		//if already initiated jump to block start
		//
		if( init ) {
			kProgram->itokar.idx = blockidx;
		}
		//get info
		//
		else {
			if( !(giToken.type == TT_PUNCTUATION && giToken.subtype == P_SEMICOLON ) ) {
				I_ScriptError( "semicolon expected" );
				return;
			}

			I_ReadToken(); //get past ";"
			tmpidx2 = kProgram->itokar.idx;

			//find start of "for" block
			//
			paren = 1;
			while( paren ) {
				I_ReadToken();
				if( giToken.type == TT_PUNCTUATION ) {
					if( giToken.subtype == P_PARENTHESESOPEN ) {
						paren++;
					}
					else if( giToken.subtype == P_PARENTHESESCLOSE ) {
						paren--;
					}
				}
			}
			
			//confirm start of block
			//
			I_ReadToken();
			if( !(giToken.type == TT_PUNCTUATION && giToken.subtype == P_BRACEOPEN) ) {
				I_ScriptError( "brace expected" );
				return;
			}
			I_UnreadLastToken();
			
			blockidx = kProgram->itokar.idx;
			
			init = TRUE;
		}
		
		//interpret
		//
		if( (int)cond.v.f ) {
			I_Interpret();
		}
		else {
			I_FindEob();
			return;
		}
		
		//check for break in loop
		//
		if( gBreakFound ) {
			kProgram->itokar.idx = blockidx;
			gBreakFound = FALSE;
			I_FindEob();
			return;
		}
		
		//evaluate the increment expression
		//
		kProgram->itokar.idx = tmpidx2;
		P_EvalExp( &cond );
		
		//loop back to top
		//
		kProgram->itokar.idx = tmpidx1;
	}
}