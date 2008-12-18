
#ifndef __NXC_H__
#define __NXC_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "interpreter.h"
#include "lexer.h"
#include "parser.h"

//init NetrixC program
//
BOOL NXC_Init( char *szPath, program_t *program, char *name );

//update (re-execute) NetrixC program
//
BOOL NXC_Update( program_t *program );

//destroy NetrixC program
//
void NXC_Destroy( program_t *program );

//register an internal function
//
void NXC_RegisterFunction( char *name, int (*p)() );

#ifdef __cplusplus
}
#endif

#endif