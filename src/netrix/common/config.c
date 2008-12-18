
//Netrix config system
//

#undef __N_FILE__
#define __N_FILE__ TEXT( "config.c" )

#include "../compile.h"
#include <windows.h>
#include <tchar.h>
#include <stdlib.h>

#include "../../netrixlib/netrixlib.h"
#include "../../nxc/nxc.h"
#include "config.h"
#include "const.h"


#define VAR_MAXLEN		50
#define INVALID_ID		(-1)
#define MAX_CLAMP		(256) //maximum color amplitude
#define TYPE_BOOL_TRUE	(1)
#define TYPE_BOOL_FALSE	(0)
#define STRING_SIZE		(30) //string literal size


//variable list
//
cfVariable_t kcfTable[VARLIST_SIZE] ={

	VAR_FIGCOLOR,		TEXT( "FigColor" ),			TEXT( "Figure Color" ),
	TYPE_COLOR,			0,	FALSE,
	
	VAR_NEXTFIGCOLOR,	TEXT( "NextFigColor" ),		TEXT( "Next figure color" ),
	TYPE_COLOR,			0,	FALSE,

	VAR_SPACECOLOR,		TEXT( "SpaceColor" ),		TEXT( "Action space color" ),
	TYPE_COLOR,			0,	FALSE,
	
	VAR_NEXTSPACECOLOR,		TEXT( "NextSpaceColor" ), TEXT( "Next figure space color" ),
	TYPE_COLOR,			0,	FALSE,
	
	VAR_GROUNDCOLOR,	TEXT( "GroundColor" ),		TEXT( "Ground color" ),
	TYPE_COLOR,			0,	FALSE,
	
	VAR_SCORESPACECOLOR, TEXT( "ScoreSpaceColor" ),	TEXT( "Score background color" ),
	TYPE_COLOR,			0,	FALSE,
	
	VAR_SCORETEXTCOLOR,	TEXT( "ScoreTextColor" ),	TEXT( "Score text color" ),
	TYPE_COLOR,			0,	FALSE,
	
	VAR_BASE,			TEXT( "Base" ),				TEXT( "Base location" ),
	TYPE_STRING,		0,	FALSE,
	
	VAR_PLAYRATE,		TEXT( "Playrate" ),			TEXT( "Playback rate" ),
	TYPE_FLOAT,			0,	FALSE,
	
	VAR_EFF_ILLUMINATE,	TEXT( "EffIlluminate" ),	TEXT( "Ground Illumination" ),
	TYPE_BOOL,			0,	FALSE,
	
	VAR_EFF_ILLUMINATE_COLOR,	TEXT( "EffIlluminateColor" ), TEXT( "Ground Illumination color" ),
	TYPE_COLOR,			0,	FALSE,
	
	VAR_EFF_PATHCAST,	TEXT( "EffPathcast" ),		TEXT( "Pathcast effect (true/false)" ),
	TYPE_BOOL,			0,	FALSE,
	
	VAR_EFF_PATHCAST_COLOR, TEXT( "EffPathcastColor" ),	TEXT( "Pathcast effect color" ),
	TYPE_COLOR,			0,	FALSE,
	
	VAR_EFF_LINEKILL,	TEXT( "EffLinekill" ),		TEXT( "Linekill effect" ),
	TYPE_BOOL,			0,	FALSE,
	
	VAR_EFF_LINEKILL_COLOR,	TEXT( "EffLinekillColor" ),	TEXT( "Linekill effect color" ),
	TYPE_COLOR,			0,	FALSE,
};


/*
=====================
	cfInitTable
=====================
*/
void cfInitTable() {
	
	//init config table with default values
	//
	kcfTable[VAR_FIGCOLOR].v.dw				= FIGCOLOR;
	kcfTable[VAR_NEXTFIGCOLOR].v.dw			= NEXTFIGCOLOR;
	kcfTable[VAR_SPACECOLOR].v.dw			= SPACECOLOR;
	kcfTable[VAR_NEXTSPACECOLOR].v.dw		= NEXTSPACECOLOR;
	kcfTable[VAR_GROUNDCOLOR].v.dw			= GROUNDCOLOR;
	kcfTable[VAR_SCORESPACECOLOR].v.dw		= SCORESPACECOLOR;
	kcfTable[VAR_SCORETEXTCOLOR].v.dw		= SCORETEXTCOLOR;
	kcfTable[VAR_BASE].v.s					= BASE;
	kcfTable[VAR_PLAYRATE].v.f				= 1;
	kcfTable[VAR_EFF_ILLUMINATE].v.b		= TRUE;
	kcfTable[VAR_EFF_ILLUMINATE_COLOR].v.dw	= EFFILLUMINATE;
	kcfTable[VAR_EFF_PATHCAST].v.b			= TRUE;
	kcfTable[VAR_EFF_PATHCAST_COLOR].v.dw	= RGB( 0, 0, 0 ); //set in initGraphics( NEUTRAL )
	kcfTable[VAR_EFF_LINEKILL].v.b			= TRUE;
	
	//load config file
	//
	cfLoadTable();
}


/*
=====================
	cfDestroyTable
=====================
*/
void cfDestroyTable() {
	int i;
	
	//free any allocated resources
	//
	for( i=0; i<VARLIST_SIZE; i++ ) {
		if( kcfTable[i].modified && ( kcfTable[i].type == TYPE_STRING ) )
			N_Free( kcfTable[i].v.s );
	}
}

	
/*
=====================
	cfLoadTable
=====================
*/
BOOL cfLoadTable() {
	HANDLE hFile;
	TCHAR *szCfBuff;
	TCHAR *szTmp;
	DWORD size;

	//open config file
	//
	hFile = N_FOpenR( CONFIGFILE );
	if( hFile == INVALID_HANDLE_VALUE ) {
		return TRUE;
	}
	
	//determine file size, alocate memory,
	//and read data
	//
	size = N_FSeek( hFile, 0L, FILE_END );
	szTmp =
	szCfBuff = N_Malloc( size+1*sizeof(TCHAR) );
	N_FSeek( hFile, 0L, FILE_BEGIN );
	N_FRead( hFile, szCfBuff, size );
	szCfBuff[size] = '\0';
	
	//parse config string
	//
	cfExecute( szCfBuff, size );
	
	//clean-up
	//		
	N_FClose( hFile );
	N_Free( szTmp );

	return TRUE;
}


/*
=====================
	cfSaveTable
=====================
*/
BOOL cfSaveTable() {
	int i;
	TCHAR szBuff[256];
	HANDLE hFile;
	
	hFile = N_FOpenC( TEXT( "netrix.cfg" ) );
	if( hFile == INVALID_HANDLE_VALUE ) {
		return FALSE;
	}
	
	N_Sprintf( szBuff, 256, TEXT( "//\n//netrix config\n//\n\n" ) );
	N_FWrite( hFile, szBuff, N_Strlen( szBuff )*sizeof(TCHAR) );
	
	for( i=0; i<VARLIST_SIZE; i++ ) {
		if( kcfTable[i].modified ) {
		
			switch( kcfTable[i].type ) {

				case TYPE_COLOR:
					N_Sprintf( szBuff, 256, TEXT( "%s %d %d %d\r\n" ),
						kcfTable[i].name, GetRValue( kcfTable[i].v.dw ),
						GetGValue( kcfTable[i].v.dw ), GetBValue( kcfTable[i].v.dw ) );
					break;
				
				case TYPE_LONG:
					N_Sprintf( szBuff, 256, TEXT( "%s %d\r\n" ),
						kcfTable[i].name, kcfTable[i].v.dw );
					break;
				
				case TYPE_BOOL:
					N_Sprintf( szBuff, 256, TEXT( "%s %d\r\n" ),
						kcfTable[i].name, kcfTable[i].v.b );
					break;
				
				case TYPE_STRING:
					N_Sprintf( szBuff, 256, TEXT( "%s %s\r\n" ),
						kcfTable[i].name, kcfTable[i].v.s );
					break;

				default:
					break;
			}
			N_FWrite( hFile, szBuff, N_Strlen( szBuff )*sizeof(TCHAR) );
		}
	}
	
	N_FClose( hFile );
	
	return TRUE;
}


/*
=====================
	cfSeeID
	-----------------
	See the variable ID.
=====================
*/
int cfSeeID( TCHAR *var ) {
	int i;
	for( i=0; i<VARLIST_SIZE; i++) {
		if( !N_Strnicmp( var, kcfTable[ i ].name, STRING_SIZE * sizeof(TCHAR) ) ) {
			return i;
		}
	}
	return INVALID_ID;
}


/*
=====================
	cfExecute
=====================
*/
BOOL cfExecute( char *buffer, int length ) {
	script_t *script;
	token_t token;
	int id;
	BOOL bPerfect, bRes;
	int r, g, b;
	int type;
	TCHAR key[25];
	TCHAR action[25];
	
	
	//check arguments
	if( !buffer ) {
		return FALSE;
	}

	script = L_LoadScriptMemory( buffer, length, "cfg" );
	if( !script ) {
		return FALSE;
	}
	
	script->flags |= SCFL_NOSTRINGWHITESPACES;
	
	bPerfect = TRUE;
	bRes = FALSE;
	while( L_ExpectTokenType( script, TT_IDENTIFIER, 0, &token ) ) {
		//
		//bind
		//
		if( !N_Strnicmp( token.string, CF_BIND, N_Strlen( CF_BIND ) ) ) {
			if( L_ExpectTokenType( script, TT_STRING, 0, &token ) ) {
				L_StripDoubleQuotes( token.string );
				N_Strcpy( key, token.string );
				if( L_ExpectTokenType( script, TT_STRING, 0, &token ) ) {
					L_StripDoubleQuotes( token.string );
					N_Strcpy( action, token.string );
					BindKey( key, action );
				}
			}
		}
		//
		//set
		//
		else if( !N_Strnicmp( token.string, CF_SET, N_Strlen( CF_SET ) ) ) {
	
			if( L_ExpectTokenType( script, TT_IDENTIFIER, 0, &token ) ) {
	
				id = cfSeeID( token.string );
				if( id == INVALID_ID ) {
					bPerfect = FALSE;
					continue;
				}
				type = kcfTable[id].type;
				bRes = FALSE;
				
				switch( type ) {
					//
					//TYPE_COLOR
					//
					case TYPE_COLOR:
						if( !L_ExpectTokenType( script, TT_NUMBER, TT_INTEGER, &token ) ) {
							break;
						}
						r = token.intvalue;
						
						if( !L_ExpectTokenType( script, TT_NUMBER, TT_INTEGER, &token ) ) {
							break;
						}
						g = token.intvalue;
						
						if( !L_ExpectTokenType( script, TT_NUMBER, TT_INTEGER, &token ) ) {
							break;
						}
						b = token.intvalue;

						kcfTable[id].v.dw = RGB( (r+MAX_CLAMP)%MAX_CLAMP,
							(g+MAX_CLAMP)%MAX_CLAMP, (b+MAX_CLAMP)%MAX_CLAMP );

						bRes = TRUE;
						break;
					//
					//TYPE_LONG
					//
					case TYPE_LONG:
						if( !L_ExpectTokenType( script, TT_NUMBER, TT_INTEGER, &token ) ) {
							break;
						}
						kcfTable[id].v.dw = token.intvalue;
						bRes = TRUE;
						break;
					//
					//TYPE_FLOAT
					//
					case TYPE_FLOAT:
						if( !L_ExpectTokenType( script, TT_NUMBER, TT_INTEGER | TT_FLOAT, &token ) ) {
							break;
						}
						kcfTable[id].v.f = token.floatvalue;
						bRes = TRUE;
						break;
					//
					//TYPE_BOOL
					//
					case TYPE_BOOL:
						if( !L_ExpectTokenType( script, TT_NUMBER, TT_INTEGER, &token ) ) {
							break;
						}
						if( token.intvalue ) {
							kcfTable[id].v.b = 1;
						}
						else {
							kcfTable[id].v.b = 0;
						}
						bRes = TRUE;
						break;
					//
					//TYPE_STRING
					//
					case TYPE_STRING:
						if( !L_ExpectTokenType( script, TT_STRING, 0, &token ) ) {
							break;
						}
						L_StripDoubleQuotes( token.string );

						kcfTable[id].v.s = N_Malloc( N_Strlen( token.string ) + 1 );
						N_Strncpy( kcfTable[id].v.s, token.string, N_Strlen( token.string ) );
						bRes = TRUE;
						break;
					
					default:
						break;

				}//switch
				
				if( bRes ) {
					kcfTable[id].modified = TRUE;
				}
				else {
					bPerfect = FALSE;
				}
			}
		}//set
	}//while
	
	L_FreeScript( script );
	
	return bPerfect;
}