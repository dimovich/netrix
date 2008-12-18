
//NetrixC interface
//

#undef __N_FILE__
#define __N_FILE__ TEXT( "(nxc) nxc.c" )

#include "compile.h"
#include <windows.h>

#include "../netrixlib/netrixlib.h"
#include "interpreter.h"
#include "lexer.h"
#include "parser.h"
#include "nxc.h"


/*
=====================
	NXC_Init
=====================
*/
BOOL NXC_Init( char *filename, program_t *program, char *name ) {

	//check arguments validity
	//
	if( !program || !filename || !name ) {
		return FALSE;
	}
	
	//load script
	//
	program->script = L_LoadScriptFile( filename, name );

	//init interpretor
	//
	I_Init( program );
	
	return TRUE;
}


/*
=====================
	NXC_Destroy
=====================
*/
void NXC_Destroy( program_t *program ) {
	if( program ) {
		L_FreeScript( program->script );
		N_Free( program->itokar.ar );
	}
}


/*
=====================
	NXC_Update
=====================
*/
BOOL NXC_Update( program_t *program ) {
	if( program ) {
		I_Update( program );
	}
	
	return TRUE;
}


/*
=====================
	NXC_RegisterFunction
=====================
*/
void NXC_RegisterFunction( char *name, int (*p)() ) {
	P_RegisterFunction( name, p );
}