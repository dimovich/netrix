
#ifndef __INTERPRETER_H__
#define __INTERPRETER_H__

#include "lexer.h"

#ifdef __cplusplus
extern "C" {
#endif


#define MAX_FUNCTIONS	16		//maximum number of functions
#define MAX_GLOBALS		64		//maximum number of globals
#define MAX_STACK		1024	//maximum number of stack elements
#define MAX_PARAMS		32		//maximum number of function parameters
#define MAX_STRINGS		64		//maximum number of strings
#define MAX_VALUES		256		//maximum number of values


//itoken_t - internal token type
//
typedef struct itoken_s {
	int		type;		//token type
	int		subtype;	//token subtype
	char	*p;			//pointer in script
} itoken_t;


//itokar_t - tokenized script
//
typedef struct itokar_s {
	int			size;		//itoken array size
	int			idx;		//current itoken index
	itoken_t	*ar;		//itoken array
} itokar_t;


//string_t
//
typedef struct string_s {
	char			string[MAX_TOKEN];	//string
	unsigned long	hash;				//string hash
} string_t;


//value_t
//
typedef struct value_s {
	int	type;	//value type: K_INT, K_FLOAT
	union {
		long int	i;
		float		f;
	} v;
} value_t;

//var_t
//
typedef struct var_s {
	int		stridx;		//variable name in string table
	value_t value;		//variable value
} var_t;


//function_t
//
typedef struct function_s {
	int		stridx;				//function name index in string table
	int		rettype;			//return type (K_INT, K_FLOAT)
	int		idx;				//index in itoken array
	var_t	params[MAX_PARAMS];	//parameters (like function prototype)
	int		cparam;				//parameter count
} function_t;


//program_t
//
typedef struct program_s {

	//itoken array (tokenized script)
	//
	itokar_t	itokar;

	//script
	//
	script_t	*script;
	
	//string table
	//
	string_t	stringTable[MAX_STRINGS];
	int			stringTableSize;
	
	//value table
	//
	value_t		valueTable[MAX_VALUES];
	int			valueTableSize;

	//function table
	//
	function_t	funcTable[MAX_FUNCTIONS];
	int			funcTableSize;

	//global variables table
	//
	var_t		globalVars[MAX_GLOBALS];
	int			globalVarsSize;

	//variable stack
	//
	var_t		varStack[MAX_STACK];
	int			varStackSize;

	//stack for function scope
	//
	int			funcCallStack[MAX_STACK];
	int			funcCallStackSize;

	//stack for nested scopes
	//
	int			nestStack[MAX_STACK];
	int			nestStackSize;

} program_t;



//init interpretor
//
BOOL I_Init( program_t *program );

//update (re-execute) program
//
void I_Update( program_t *program );

//convert script to internal format (itoken array)
//which is way _much_ faster (everything is integer)
//
void I_Convert();

//add string to string table
//
int I_AddString( char *string );

//add value to value table
//
int I_AddValue( int subtype, long int intvalue, float floatvalue );

//display a script error
//
void I_ScriptError( char *format, ... );

//skip until a token of given type and subtype
//
void I_SkipUntilToken( int type, int subtype );

//read next token (macro is slightly faster)
//
#define I_ReadToken() {											\
	if( kProgram->itokar.idx < kProgram->itokar.size ) {		\
		giToken = kProgram->itokar.ar[kProgram->itokar.idx++];	\
	} else {													\
		giToken.type = TT_END;									\
	}															\
}

//unread last token (macro is slightly faster)
//
#define I_UnreadLastToken() {			\
	if( kProgram->itokar.idx > 0 ) {	\
		kProgram->itokar.idx--;			\
	}									\
}

//interpret a block or single statement
//
void I_Interpret();

//find the location of all functions and
//store global variables
//
void I_Prescan();

//declare a global variable
//
void I_DeclareGlobal();

//declare a local variable
//
void I_DeclareLocal();

//declare a function
//
void I_DeclareFunction();

//find a function
//
BOOL I_FindFunc( int stridx, int *id );

//find a function (string version)
//
BOOL I_FindFuncName( char *name, int *id );

//call function currently in token
//
void I_Call();

//push the arguments to a function onto the local
//variable stack
//
void I_GetArguments( function_t *func );

//evaluate return expression
//
void I_FunctionReturn();

//assign a value to a variable
//
void I_AssignVariable( int stridx, value_t *value );

//find variable with the given name
//
BOOL I_FindVariable( int stridx, value_t *value );

//promote child to parent (macro is slightly faster)
//
#define I_PromoteValue(p,c) {								\
	if( ((value_t*)p)->type != ((value_t*)c)->type ) {		\
		if( ((value_t*)c)->type == K_INT ) {				\
			((value_t*)c)->type = K_FLOAT;					\
			((value_t*)c)->v.f = (float)((value_t*)c)->v.i;	\
		}													\
		else if( ((value_t*)c)->type == K_FLOAT ) {			\
			((value_t*)c)->type = K_INT;					\
			((value_t*)c)->v.i = (int)((value_t*)c)->v.f;	\
		}													\
	}														\
}


//check if name is a variable
//
BOOL I_IsVariable( int stridx );

//find end-of-block
//
void I_FindEob();

//execute an "if" statement
//
void I_ExecuteIf();

//execute a "switch" statement
//
void I_ExecuteSwitch();

//execute a "while" loop
//
void I_ExecuteWhile();

//execute a "do" loop
//
void I_ExecuteDo();

//execute a "for" loop
//
void I_ExecuteFor();

#ifdef __cplusplus
}
#endif

#endif