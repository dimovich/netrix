
#include "compile.h"
#include <windows.h>

#include "libc.h"
#include "const.h"
#include "bot.h"

/*
=====================
	getBotName
=====================
*/
void getBotName( TCHAR *szPath, TCHAR *szName ) {
	HANDLE hFile = INVALID_HANDLE_VALUE;
	TCHAR szTmpName[NAMESIZE];
	int ch;
	BOOL bRes;
	int idx;

	if( (szPath == NULL) || (szName == NULL) ) {
		return;
	}
	
	__try {
		bRes = FALSE;
	
		hFile = N_FOpenR( szPath );
		if( hFile == INVALID_HANDLE_VALUE ) {
			
		}
		
		//try to read name
		//
		idx = 0;
		while( TRUE ) {
			ch = N_FGetc( hFile );
			if( ch == N_EOF ) {
				__leave;
			}
			if( ch == '$' ) {
				break;
			}
		}

		while( TRUE ) {
			ch = N_FGetc( hFile );
			if( (ch == '\r') || (ch == '$') || (ch == N_EOF) || (idx >= NAMESIZE) ) {
				__leave;
			}
			szTmpName[idx++] = ch;
			bRes = TRUE;
		}
	}
	__finally {
		if( hFile != INVALID_HANDLE_VALUE ) {
			N_FClose( hFile );
		}
		
		if( bRes ) {
			szTmpName[idx] = '\0';
			N_Strcpy( szName, szTmpName );
		}
		else {
			N_Strcpy( szName, "Untitled Bot" );
		}
	}
}