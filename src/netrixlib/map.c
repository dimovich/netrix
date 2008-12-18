
#undef __N_FILE__
#define __N_FILE__ TEXT( "map.c" )


#include "compile.h"
#include <windows.h>

#include "libc.h"
#include "map.h"
#include "const.h"


/*
=====================
	loadMap
=====================
*/
BOOL loadMap( TCHAR *path, map_t *map ) {
	HANDLE hFile = INVALID_HANDLE_VALUE;
	DWORD tmp;
	BOOL bRes;
	
	//check arguments validity
	if( (path == NULL) || (map == NULL) ) {
		return FALSE;
	}
	
	__try {
		bRes = FALSE;

		hFile = N_FOpenR( path );
		if( hFile == INVALID_HANDLE_VALUE ) {
			__leave;
		}
	
		//read map name

		ReadFile( hFile, map->name, NAMESIZE, &tmp, NULL );
		if( tmp != NAMESIZE ) {
			__leave;
		}


		//read map data
	
		ReadFile( hFile, map->map, sizeof( int ) * CXSPACE * CYSPACE,
			&tmp, NULL );
		if( tmp != sizeof(int) * CXSPACE*CYSPACE ) {
			__leave;
		}
		
		bRes = TRUE;
	}
	
	__finally {
		if( hFile != INVALID_HANDLE_VALUE ) {
			N_FClose( hFile );
		}
	}
	
	return bRes;
}


/*
=====================
	saveMap
=====================
*/
BOOL saveMap( TCHAR *path, map_t *map ) {
	HANDLE hFile = INVALID_HANDLE_VALUE;
	DWORD tmp;
	BOOL bRes;

	__try {
		bRes = FALSE;

		hFile = N_FOpenC( path );
		if( hFile == INVALID_HANDLE_VALUE ) {
			__leave;
		}
		
		//write map name
		WriteFile( hFile, map->name, NAMESIZE, &tmp, NULL );
		if( tmp != NAMESIZE ) {
			__leave;
		}
		
		//write map data
		
		WriteFile( hFile, map->map, sizeof( int ) * CYSPACE*CXSPACE,
			&tmp, NULL );
		if( tmp != sizeof( int ) * CXSPACE*CYSPACE ) {
			__leave;
		}
		
		bRes = TRUE;
	}

	__finally {
		if( hFile != INVALID_HANDLE_VALUE ) {
			N_FClose( hFile );
		}
	}

	return bRes;
}


/*
=====================
	getMapName
=====================
*/
void getMapName( TCHAR *szPath, TCHAR *szName ) {
	HANDLE hFile = INVALID_HANDLE_VALUE;
	DWORD tmp;
	
	//check arguments validity
	if( (szPath == NULL) || (szName == NULL) ) {
		return;
	}
	
	//open file
	//
	hFile = N_FOpenR( szPath );
	if( hFile == INVALID_HANDLE_VALUE ) {
		return;
	}

	//read map name
	//
	ReadFile( hFile, szName, NAMESIZE*sizeof(TCHAR), &tmp, NULL );

	//clean-up
	//
	N_FClose( hFile );
}