
//NetrixC (recursive descent) expression parser
//
//Code based on:
//"Mini C++ Interpreter" code (by Herbert Schildt);
//

#include "compile.h"
#include <windows.h>

#include "../netrixlib/netrixlib.h"
#include "lexer.h"
#include "interpreter.h"
#include "parser.h"


extern program_t *kProgram;
extern value_t gReturnValue;
extern itoken_t giToken;


//built-in function table
//
static internFunction_t	kInternFunc[MAX_INTERN_FUNCTIONS] = {0};
static int				kInternFuncSize = 0;


/*
=====================
	P_RegisterFunction
=====================
*/
void P_RegisterFunction( char *name, int (*p)() ) {
	int id;
	
	//check arguments
	if( !name || !p ) {
		return;
	}
	
	//check if enough space for one more function
	//
	if( kInternFuncSize >= MAX_INTERN_FUNCTIONS ) {
		return;
	}
	
	id = kInternFuncSize++;
	
	//register internal function
	//
	kInternFunc[id].hash = N_CalcHash( name );
	kInternFunc[id].p = p;
}


/*
=====================
	P_InternFunc
=====================
*/
BOOL P_InternFunc( int stridx, int *id ) {
	int i;
	
	if( id && (stridx<kProgram->stringTableSize) ) {
		for( i=0; i<kInternFuncSize; i++ ) {
			if( kInternFunc[i].hash == kProgram->stringTable[stridx].hash ) {
				*id = i;
				return TRUE;
			}
		}
	}

	return FALSE;
}


/*
=====================
	P_EvalExp
=====================
*/
void P_EvalExp( value_t *value ) {
	//read token
	//
	I_ReadToken();
	if( giToken.type == TT_END ) {
		I_ScriptError( "no expression present" );
		return;
	}
	
	//empty expression
	//
	if( (giToken.type == TT_PUNCTUATION) && (giToken.subtype == P_SEMICOLON) ) {
		value->v.i = 0;
		I_UnreadLastToken(); //unread ";"
		return;
	}
	
	P_EvalExp0( value );
	
	I_UnreadLastToken();
}


/*
=====================
	P_EvalExp0
=====================
*/
void P_EvalExp0( value_t *value ) {
	value_t tmpval = {K_FLOAT, 0};
	itoken_t tmptok;
	BOOL flag = FALSE;
	int op;
	
	//check if token is an identifier
	//
	if( giToken.type == TT_IDENTIFIER ) {
		if( I_FindVariable( giToken.subtype, &tmpval ) ) {

			N_Memcpy( &tmptok, &giToken, sizeof(itoken_t) );
			
			I_ReadToken();
			
			//check if an assignment
			//
			if( giToken.type == TT_PUNCTUATION ) {
			
				//check punctuation type
				switch( giToken.subtype ) {
					case P_ASSIGN:
					case P_MUL_ASSIGN:
					case P_DIV_ASSIGN:
					case P_ADD_ASSIGN:
					case P_SUB_ASSIGN:
						flag = TRUE;
						op = giToken.subtype;
						break;
					default:
						flag = FALSE;
						break;
				}
			
				if( flag ) {
					//read next token
					I_ReadToken();
					//get value to assign
					P_EvalExp0( value );
					
					I_PromoteValue( &tmpval, value );
					
					switch( op ) {
						case P_ASSIGN:
							tmpval = *value;
							break;
						
						case P_MUL_ASSIGN:
							tmpval.v.f *= value->v.f;
							break;

						case P_DIV_ASSIGN:
							tmpval.v.f /= value->v.f;
							break;

						case P_ADD_ASSIGN:
							tmpval.v.f += value->v.f;
							break;

						case P_SUB_ASSIGN:
							tmpval.v.f -= value->v.f;
							break;
						
						default:
							break;
					}

					I_AssignVariable( tmptok.subtype, &tmpval );

					return;
				}
			}

			//not an assignment
			//
			I_UnreadLastToken();
			N_Memcpy( &giToken, &tmptok, sizeof(itoken_t) );
		}
	}
	
	P_EvalExp1( value );
}


/*
=====================
	P_EvalExp1
=====================
*/
void P_EvalExp1( value_t *value ) {
	value_t partial_value = {K_FLOAT, 0};
	int subtype;
	int res;
	
	P_EvalExp2( value );
	
	if( giToken.type == TT_PUNCTUATION ) {
		switch( giToken.subtype ) {
			case P_LOGIC_GEQ:
			case P_LOGIC_LEQ:
			case P_LOGIC_EQ:
			case P_LOGIC_UNEQ:
			case P_LOGIC_GREATER:
			case P_LOGIC_LESS:
				break;
			default:
				return;
		}
		subtype = giToken.subtype;

		//get next token and evaluate
		//
		I_ReadToken();
		P_EvalExp2( &partial_value );
	
		//promote
		//
		I_PromoteValue( &partial_value, value );
		
		//evaluate
		//
		switch( subtype ) {
			case P_LOGIC_GEQ:
				res = (value->v.f >= partial_value.v.f);
				break;
			case P_LOGIC_LEQ:
				res = (value->v.f <= partial_value.v.f);
				break;
			case P_LOGIC_EQ:
				res = (value->v.f == partial_value.v.f);
				break;
			case P_LOGIC_UNEQ:
				res = (value->v.f != partial_value.v.f);
				break;
			case P_LOGIC_GREATER:
				res = (value->v.f > partial_value.v.f);
				break;
			case P_LOGIC_LESS:
				res = (value->v.f < partial_value.v.f);
				break;
			default:
				return;
		}
		
		//assign result
		//
		value->v.f = (float)res;
	}
}


/*
=====================
	P_EvalExp2
=====================
*/
void P_EvalExp2( value_t *value ) {
	value_t partial_value = {K_FLOAT, 0};
	int subtype;

	P_EvalExp3( value );
	
	while( (giToken.type == TT_PUNCTUATION) && (giToken.subtype == P_ADD || giToken.subtype == P_SUB) ) {
		
		subtype = giToken.subtype;
		
		I_ReadToken();
		
		//check if next token is a valid punctuation
		//
		if( giToken.type == TT_PUNCTUATION ) {
			switch( giToken.subtype ) {
				case P_PARENTHESESOPEN:
				case P_INC:
				case P_DEC:
				case P_ADD:
				case P_SUB:
					break;
				default:
					I_ScriptError( "(exp2) syntax error" );
					return;
			}
		}
		
		//evaluate next partial expression
		//
		P_EvalExp3( &partial_value );
		
		//promote
		//
		I_PromoteValue( &partial_value, value );
		
		//evaluate expression
		//
		switch( subtype ) {
			case P_ADD:
				value->v.f = value->v.f + partial_value.v.f;
				break;
			case P_SUB:
				value->v.f = value->v.f - partial_value.v.f;
				break;
			default:
				break;
		}
	}
}


/*
=====================
	P_EvalExp3
=====================
*/
void P_EvalExp3( value_t *value ) {
	int subtype;
	value_t partial_value = {K_FLOAT, 0};
	int i;
	
	P_EvalExp4( value );

	while( giToken.type == TT_PUNCTUATION ) {
		switch( giToken.subtype ) {
			case P_MUL:
			case P_DIV:
			case P_MOD:
				break;
			default:
				return;
		}
		subtype = giToken.subtype;
		
		I_ReadToken();
		
		//check if next token is a valid punctuation
		//
		if( giToken.type == TT_PUNCTUATION ) {
			switch( giToken.subtype ) {
				case P_PARENTHESESOPEN:
				case P_INC:
				case P_DEC:
				case P_ADD:
				case P_SUB:
					break;
				default:
					I_ScriptError( "(exp3) syntax error" );
					return;
			}
		}
		
		P_EvalExp4( &partial_value );
		
		//promote
		//
		I_PromoteValue( &partial_value, value );

		switch( subtype ) {
			case P_MUL:
				value->v.f = value->v.f * partial_value.v.f;
				break;
			case P_DIV:
				if( partial_value.v.f == 0 ) {
					I_ScriptError( "division by zero" );
					return;
				}
				value->v.f = value->v.f / partial_value.v.f;
				break;
			case P_MOD:
				//NOTE: mod on float is illegal, but we are fine with it
				//
				if( partial_value.v.f == 0 ) {
					I_ScriptError( "division by zero" );
					return;
				}
				i = (int)(value->v.f/partial_value.v.f);
				value->v.f = value->v.f - (float)i*partial_value.v.f;
				break;
			default:
				break;
		}
	}
}


/*
=====================
	P_EvalExp4
=====================
*/
void P_EvalExp4( value_t *value ) {
	int subtype;
	int type;
	BOOL flag = FALSE;
	value_t partial_value = {K_FLOAT, 0};
	
	//check token
	//
	if( giToken.type == TT_PUNCTUATION ) {
		switch( giToken.subtype ) {
			case P_INC:
			case P_DEC:
			case P_ADD:
			case P_SUB:
				flag = TRUE;
				break;
			default:
				flag = FALSE;
				break;
		}
	}
	
	type = giToken.type;
	subtype = giToken.subtype;
	
	//perform unary operation
	//
	if( flag ) {
		I_ReadToken();
		
		if( I_FindVariable( giToken.subtype, &partial_value ) ) {
			//evaluate
			//
			switch( subtype ) {
				case P_INC:
					partial_value.v.f += 1;
					break;
				case P_DEC:
					partial_value.v.f -= 1;
					break;				
			}
			I_AssignVariable( giToken.subtype, &partial_value );
		}
	}
	
	P_EvalExp5( value );
	
	if( (type == TT_PUNCTUATION) && (subtype == P_SUB) ) {
		switch( value->type ) {
			case K_INT:
				value->v.i = -(value->v.i);
				break;
			case K_FLOAT:
				value->v.f = -(value->v.f);
				break;
		}
	}
}


/*
=====================
	P_EvalExp5
=====================
*/
void P_EvalExp5( value_t *value ) {
	
	if( (giToken.type == TT_PUNCTUATION) && (giToken.subtype == P_PARENTHESESOPEN) ) {
		I_ReadToken();
		
		//evaluate subexpression
		//
		P_EvalExp0( value );
		
		if( !( giToken.type == TT_PUNCTUATION && giToken.subtype == P_PARENTHESESCLOSE ) ) {
			I_ScriptError( "parentheses expected" );
			return;
		}

		I_ReadToken();
	}
	//get atom value
	//
	else {
		P_Atom( value );
	}
}


/*
=====================
	P_Atom
=====================
*/
void P_Atom( value_t *value ) {
	int i;
	int res;
	int stridx;
	value_t tmpval = {K_FLOAT, 0}, tmpval2 = {K_FLOAT, 0};
	
	//check arguments
	if( !value ) {
		return;
	}
	
	//promote (if not already done so)
	//
	if( value->type != K_FLOAT ) {
		I_PromoteValue( &tmpval, value );
	}
	
	switch( giToken.type ) {
		case TT_IDENTIFIER:
			//check if internal function
			//
			if( P_InternFunc( giToken.subtype, &i ) ) {
				I_ReadToken(); //read "(" token
				res = (*kInternFunc[i].p)();
				I_ReadToken(); //read ")" token
				value->v.f = (float)res;
			}
			//check if programmer created function
			//
			else if( I_FindFunc( giToken.subtype, &i ) ) {
				I_Call();
				N_Memcpy( &tmpval, &gReturnValue, sizeof(value_t) );
				
				//set value
				//
				I_PromoteValue( value, &tmpval );
				value->v.f = tmpval.v.f;
			}
			//variable
			//
			else if( I_FindVariable( giToken.subtype, &tmpval ) ) {

				stridx = giToken.subtype;
				
				//assign value
				//
				value->v.f = tmpval.v.f;
				
				//check for postfix ++ or --
				//
				I_ReadToken();
				if( giToken.type == TT_PUNCTUATION ) {
					switch( giToken.subtype ) {
						case P_INC:
							tmpval.v.f += 1;
							I_AssignVariable( stridx, &tmpval );
							break;
						case P_DEC:
							tmpval.v.f -= 1;
							I_AssignVariable( stridx, &tmpval );
							break;
						default:
							I_UnreadLastToken();
							break;
					}
				}
				else {
					I_UnreadLastToken();
				}
			}
			//unknown identifier
			//
			else {
				I_ScriptError( "unknown identifier %s",
					kProgram->stringTable[giToken.subtype].string );
				return;
			}
			I_ReadToken();
			return;
		
		case TT_NUMBER:
			N_Memcpy( &tmpval, &kProgram->valueTable[giToken.subtype], sizeof(value_t) );
			I_PromoteValue( value, &tmpval );
			value->v.f = tmpval.v.f;
			I_ReadToken();
			return;
		
		case TT_LITERAL:
			value->v.f = (float)giToken.subtype;
			I_ReadToken();
			break;
		
		case TT_PUNCTUATION:
			if( giToken.subtype == P_PARENTHESESCLOSE ) {
				return;
			}
			else {
				I_ScriptError( "(atom1) syntax error" );
				value->v.f = 0.0f;
			}
			break;

		default:
			I_ScriptError( "(atom2) syntax error" );
			value->v.f = 0.0f;
			return;
	}
}