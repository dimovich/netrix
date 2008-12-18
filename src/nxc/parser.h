
#ifndef __PARSER_H__
#define __PARSER_H__

#include "interpreter.h"

#ifdef __cplusplus
extern "C" {
#endif


#define MAX_INTERN_FUNCTIONS 32	//maximum number of intern functions


//interFunction_t
//
typedef struct internFunction_s {
	unsigned long hash;	//function name hash
	int (*p)();			//function pointer
} internFunction_t;


//register an internal function
//
void P_RegisterFunction( char *name, int (*p)() );

//get index of internal function
//
BOOL P_InternFunc( int stridx, int *id );

//entry point into parser
//
void P_EvalExp( value_t *value );

//process an assignment expression
//
void P_EvalExp0( value_t *value );

//process relational operators
//
void P_EvalExp1( value_t *value );

//add or substract two terms
//
void P_EvalExp2( value_t *value );

//multiply or divide two factors
//
void P_EvalExp3( value_t *value );

//evaluate unary +, -, ++, or --
//
void P_EvalExp4( value_t *value );

//process parenthesized expression
//
void P_EvalExp5( value_t *value );

//find number, variable or function value
//
void P_Atom( value_t *value );

#ifdef __cplusplus
}
#endif

#endif