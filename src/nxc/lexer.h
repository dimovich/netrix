
#ifndef __LEXER_H__
#define __LEXER_H__


#ifdef __cplusplus
extern "C" {
#endif


#define MAX_TOKEN 1024


//script flags
//
#define SCFL_NOERRORS				(1<<0)
#define SCFL_NOWARNINGS				(1<<1)
#define SCFL_NOSTRINGWHITESPACES	(1<<2)
#define SCFL_NOSTRINGESCAPECHARS	(1<<3)
#define SCFL_PRIMITIVE				(1<<4)


//token types
//
#define TT_END			0	//end of script
#define TT_STRING		1
#define TT_LITERAL		2
#define TT_NUMBER		3
#define TT_NAME			4
#define TT_PUNCTUATION	5
#define TT_KEYWORD		6
#define TT_IDENTIFIER	7

//string subtype - the length of the string
//literal subtype - the ASCII code of the literal


//keyword subtype
//
#define K_INVALID	0
#define K_INT		1
#define K_FLOAT		2
#define K_SWITCH	3
#define K_CASE		4
#define K_IF		5
#define K_ELSE		6
#define K_FOR		7
#define K_DO		8
#define K_WHILE		9
#define K_BREAK		10
#define K_RETURN	11


//number subtype
//
#define TT_DECIMAL		(1<<3)
#define TT_HEX			(1<<4)
#define TT_OCTAL		(1<<5)
#define TT_BINARY		(1<<6)
#define TT_FLOAT		(1<<7)
#define TT_INTEGER		(1<<8)
#define TT_LONG			(1<<9)
#define TT_UNSIGNED		(1<<10)


//punctuation subtype
//
#define P_RSHIFT_ASSIGN		1
#define P_LSHIFT_ASSIGN		2
#define P_PARMS				3
#define P_PRECOMPMERGE		4

#define P_LOGIC_AND			5
#define P_LOGIC_OR			6
#define P_LOGIC_GEQ			7
#define P_LOGIC_LEQ			8
#define P_LOGIC_EQ			9
#define P_LOGIC_UNEQ		10

#define P_MUL_ASSIGN		11
#define P_DIV_ASSIGN		12
#define P_MOD_ASSIGN		13
#define P_ADD_ASSIGN		14
#define P_SUB_ASSIGN		15
#define P_INC				16
#define P_DEC				17

#define P_BIN_AND_ASSIGN	18
#define P_BIN_OR_ASSIGN		19
#define P_BIN_XOR_ASSIGN	20
#define P_RSHIFT			21
#define P_LSHIFT			22

#define P_POINTERREF		23
#define P_CPP1				24
#define P_CPP2				25

#define P_MUL				26
#define P_DIV				27
#define P_MOD				28
#define P_ADD				29
#define P_SUB				30
#define P_ASSIGN			31

#define P_BIN_AND			32
#define P_BIN_OR			33
#define P_BIN_XOR			34
#define P_BIN_NOT			35

#define P_LOGIC_NOT			36
#define P_LOGIC_GREATER		37
#define P_LOGIC_LESS		38

#define P_REF				39
#define P_COMMA				40
#define P_SEMICOLON			41
#define P_COLON				42
#define P_QUESTIONMARK		43

#define P_PARENTHESESOPEN	44
#define P_PARENTHESESCLOSE	45
#define P_BRACEOPEN			46
#define P_BRACECLOSE		47
#define P_SQBRACKETOPEN		48
#define P_SQBRACKETCLOSE	49
#define P_BACKSLASH			50

#define P_PRECOMP			51
#define P_DOLLAR			52


//punctuation_t
//
typedef struct punctuation_s {
	char	*p;	//punctuation character(s)
	int		n;	//punctuation indication
	struct punctuation_s *next;	//next punctuation
} punctuation_t;


//keyword_t
//
typedef struct keyword_s {
	char	*p;	//keyword character(s)
	int		n;	//keyword indication
	struct keyword_s *next; //next keyword
} keyword_t;


//token_t
//
typedef struct token_s {
	char	string[MAX_TOKEN];	//available token
	int		type;				//last read token type
	int		subtype;			//last reat token subtype
	char	*whitespace_p;		//start of white space before token
	char	*endwhitespace_p;	//end of white space before token
	int		line;				//line the token was on
	int		linescrossed;		//lines crossed in white space
	long int intvalue;			//integer value
	float floatvalue;			//floating point value
	struct token_s *next;		//next token in chain
} token_t;


//script_t
//
typedef struct script_s {
	char	szFileName[MAX_PATH];	//script file name
	char	*buffer;				//buffer containing the script
	char	*script_p;				//current pointer in the script
	char	*end_p;					//pointer to the end of the script
	char	*lastscript_p;			//script pointer before reading token
	char	*whitespace_p;			//begin of the white space
	char	*endwhitespace_p;		//end of white space
	int		length;					//length of the script in bytes
	int		line;					//current line in the script
	int		lastline;				//last line before reading token
	int		tokenavailable;			//
	unsigned long	flags;					//script flags
	token_t	token;					//available token
	punctuation_t *punctuations;	//the punctuations used in the script
	punctuation_t **punctuationtable;
	keyword_t *keywords;			//keywords used in the script
	struct script_s *next;			//next script in a chain
} script_t;


//compress script file
//
int L_Compress( char * data_p );

//build punctuation table
//
void L_CreatePunctuationTable( script_t *script, punctuation_t *punctuations );

//return punctuation string for
//
char * L_PunctuationFromNum( script_t *script, int num );

//set script keywords
//
void L_SetScriptKeywords( script_t *script, keyword_t *keywords );

//see if name is a keyword
//
int L_LookUpName( script_t *script, token_t *token );

//print an error message
//
void L_ScriptError( script_t *script, char *str, ... );

//print a warning message
//
void L_ScriptWarning( script_t *script, char *str, ... );

//set script punctuation
//
void L_SetScriptPunctuations( script_t *script, punctuation_t *p );

//Read spaces, tabs, C-like comments, etc.
//When a new line is found the script line counter is increased.
//
BOOL L_ReadWhiteSpace( script_t *script );

//skip current line
//
BOOL L_SkipLine( script_t *script );

//read an escape character
//
BOOL L_ReadEscapeCharacter( script_t *script, char *ch );

//Reads C-like string. Escape characters are interpretted.
//Quotes are included with the string.
//Reads two strings whith white space between them as one string.
//
BOOL L_ReadString( script_t *script, token_t *token, int quote );

//read a name
//
BOOL L_ReadName( script_t *script, token_t *token );

//convert a string into a number value
//
void L_NumberValue( char *string, int subtype, long int *intvalue, float *floatvalue );

//read a number
//
BOOL L_ReadNumber( script_t *script, token_t *token );

//read a literal
//
BOOL L_ReadLiteral( script_t *script, token_t *token );

//read a primitive token
//
BOOL L_ReadPrimitive( script_t *script, token_t *token );

//read a token
//
BOOL L_ReadToken( script_t *script, token_t *token );

//expect a string token
//
BOOL L_ExpectTokenString( script_t *script, char *string );

//expect a token of a type
//
BOOL L_ExpectTokenType( script_t *script, int type, int subtype, token_t *token );

//expect any token
//
BOOL L_ExpectAnyToken( script_t *script, token_t *token );

//check a token string
//
BOOL L_CheckTokenString( script_t *script, char *string );

//check a token
//
BOOL L_CheckTokenType( script_t *script, int type, int subtype, token_t *token );

//skip tokens until string is found
//
BOOL L_SkipUntilString( script_t *script, char *string );

//unread last token
//
void L_UnreadLastToken( script_t *script );

//unread token
//
void L_UnreadToken( script_t *script, token_t *token );

//get next white space character
//
char L_NextWhiteSpaceChar( script_t *script );

//strip double quotes from a string
//
void L_StripDoubleQuotes( char *string );

//strip single quotes from a string
//
void L_StripSingleQuotes( char *string );

//read a signed float
//
long double L_ReadSignedFloat( script_t *script );

//read a signed integer
//
signed long int L_ReadSignedInt( script_t *script );

//set script flags
//
void L_SetScriptFlags( script_t *script, int flags );

//reset script
//
void L_ResetScript( script_t *script );

//check if reached end of script
//
BOOL L_EndOfScript( script_t *script );

//get number of lines crossed
//
int L_NumLinesCrossed( script_t *script );

//skip script until value
//
BOOL L_ScriptSkipTo( script_t *script, char *value );

//load a script from memory
//
script_t * L_LoadScriptMemory( char *ptr, int length, char *name );

//load a script from file
//
script_t * L_LoadScriptFile( char *filename, char *name );

//free script memory
//
void L_FreeScript( script_t *script );

#ifdef __cplusplus
}
#endif

#endif