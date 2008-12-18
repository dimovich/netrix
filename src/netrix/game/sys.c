
//High level system functions
//

#undef __N_FILE__
#define __N_FILE__ TEXT( "sys.c" )


#include "../compile.h"
#include <windows.h>
#include <mmsystem.h>

#include "../../netrixlib/netrixlib.h"

#include "../graphics/graphics.h"
#include "../win32/func_win.h"
#include "../win32/sys_win.h"
#include "../common/config.h"
#include "../common/keys.h"
#include "../game/func.h"
#include "../bot/bot.h"
#include "sys.h"

#include "../resource.h"


#define BLOCK_SIZE (4*1024)


//game "singleton"
//
system_t k_system;


#ifdef ENABLE_LOG
	extern HANDLE khLog; //log file
#endif

#ifdef DEBUG_COMPILE
	extern memory_t *kMemory;
#endif

extern HRGN hSkinRgn;
/*
=====================
	initSystem
=====================
*/
BOOL initSystem() {

	//get game directory path (Ex: D:\Games\Netrix)
	//
	GetCurrentDirectory( MAX_PATH, k_system.szStartDir );

	N_InitTrace();

	//init Win32 system
	//
	initWin32();

	//System
	//
	k_system.pLeftGame	= NULL;
	k_system.pRightGame	= NULL;
	k_system.gameType	= GNO;
	k_system.pause		= FALSE;
	k_system.flags		= 0;
	k_system.dwAccumTime	= 0;
	k_system.dwTime		= 0;
	
	//Maps
	//
	k_system.cMaps	= 0;
	k_system.pMaps	= NULL;
	k_system.idMap	= -1;
	
	//Bots
	//
	k_system.cBots	= 0;
	k_system.pBots	= NULL;
	k_system.idBotLeft	= -1;
	k_system.idBotRight	= -1;
	
	//Paths
	//
	k_system.pPaths	= NULL;
	k_system.cPaths	= 0;
	
	//HWND
	//
	k_system.hwnd		= NULL;
	k_system.hwndLeft	= NULL;
	k_system.hwndRight	= NULL;
	
	k_system.cPlayers	= 0;

	initRandom();

	cfInitTable();

	//resources
	//
	loadResources();
	
	//init bot system
	//
	botInit();

	//Skins
	//
	loadSkin( &k_system.hSkinRgnLeft, &k_system.hSkinBitmapLeft,
		&k_system.cxSkinLeft, &k_system.cySkinLeft, IDR_SKIN_LEFT );

	loadSkin( &k_system.hSkinRgnRight, &k_system.hSkinBitmapRight,
		&k_system.cxSkinRight, &k_system.cySkinRight, IDR_SKIN_RIGHT );

	createWindow();
	
	//GUI
	//
	populateGUI();

	if( !initGraphics( NEUTRAL ) )
		return FALSE;

	if( !initGraphics( LEFTGAME ) )
		return FALSE;

	updateWindow( CGF_DRAWLEFT );
	
	//winmm
	timeBeginPeriod( 1 );
	
	return TRUE;
}


/*
=====================
	destroySystem
=====================
*/
BOOL destroySystem() {
	TCHAR szPath[MAX_PATH] = {0};

	//end all running games
	endAllGames();
	
	botDestroy();
	
	N_CloseTrace();

	//destroy graphics resources
	destroyGraphics( LEFTGAME );
	destroyGraphics( NEUTRAL );

	//winmm
	timeEndPeriod( 1 );
	
	//delete left skin
	if( k_system.hSkinBitmapLeft )
		DeleteObject( k_system.hSkinBitmapLeft );
	
	if( k_system.hSkinBitmapRight )
		DeleteObject( k_system.hSkinBitmapRight );

	if( k_system.pPaths ) {
		N_Free( k_system.pPaths );
	}
	
	if( k_system.pMaps ) {
		N_Free( k_system.pMaps );
	}
	if( k_system.pBots ) {
		N_Free( k_system.pBots );
	}
	
	cfDestroyTable();
	UnbindKeys();
	
#ifdef DEBUG_COMPILE
	N_Sprintf( szPath, MAX_PATH, TEXT( "%s\\leaks.txt" ),
		k_system.szStartDir );
	outputDebugInfo( szPath );
#endif

	return TRUE;
}


/*
=====================
	loadResources
=====================
*/
void loadResources() {

	WIN32_FIND_DATA FileData;

	HANDLE			hSearch;
	TCHAR			szPath[MAX_PATH];
	TCHAR			szFilePath[MAX_PATH];
	
	pathEntry_t		*pathEntry;
	mapEntry_t		*mapEntry;
	botEntry_t		*botEntry;
	

	SetCurrentDirectory( k_system.szStartDir );
	
	__try {
		//set path
		N_Sprintf( szPath, MAX_PATH, TEXT( "%s\\*.*" ),
			kcfTable[VAR_BASE].v.s );
		
		//initiate search
		//
		hSearch = FindFirstFile( szPath, &FileData );
		if( hSearch == INVALID_HANDLE_VALUE )
			__leave;
		
		while( TRUE ) {

			if( !( FileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY ) ) {	
				
				//increase path count
				k_system.cPaths++;
				
				//increase memory
				if( k_system.pPaths )
					k_system.pPaths = N_Realloc( k_system.pPaths, k_system.cPaths * sizeof( pathEntry_t ) );
				else
					k_system.pPaths = N_Malloc( k_system.cPaths * sizeof( pathEntry_t ) );

				//get full file path			
				N_Sprintf( szFilePath, MAX_PATH, TEXT( "%s\\%s" ),
					kcfTable[VAR_BASE].v.s, FileData.cFileName );

				//set-up file path
				pathEntry = &( k_system.pPaths[ k_system.cPaths - 1 ] );
				N_Strncpy( pathEntry->szPath, szFilePath, MAX_PATH );


				//single map file
				//
				if( N_Strstr( FileData.cFileName, TEXT( ".map" ) ) ) {

					//load map
					//
					mapEntry = loadMapResource( szFilePath );
					
					if( mapEntry ) {
						//set-up map entry
						//
						mapEntry->fe.dwFlags |= FILE_SINGLE;
						mapEntry->fe.pathID = k_system.cPaths-1;
						getMapName( szFilePath, mapEntry->map.name );
					}
				}			
				
				//single bot file
				//
				else if( N_Strstr( FileData.cFileName, TEXT( ".c" ) ) ) {
					//load bot
					//
					botEntry = loadBotResource( szFilePath );
					
					if( botEntry ) {
						//set-up bot entry
						//
						botEntry->fe.dwFlags |= FILE_SINGLE;
						botEntry->fe.pathID = k_system.cPaths - 1;
						getBotName( szFilePath, botEntry->bot.name );
					}
				}
				
				//pack file
				//
				else if( N_Strstr( FileData.cFileName, TEXT( ".npk" ) ) )
					loadPackResource( szFilePath );

			} //!directory

			//error check
			if( !FindNextFile( hSearch, &FileData ) )
				__leave;

		} //while( TRUE )
	}
	
	__finally {
		if( hSearch != INVALID_HANDLE_VALUE )
			FindClose( hSearch );
	}
}


/*
=====================
	reloadResources
=====================
*/
void reloadResources() {
	int i;
	
	//check if any resources are still opened
	//
	for( i=0; i<k_system.cMaps; i++ ) {
		if( k_system.pMaps[i].loaded ) {
			N_Message( TEXT( "Close all opened maps and then try again!" ) );
			return;
		}
	}
	
	for( i=0; i<k_system.cBots; i++ ) {
		if( k_system.pBots[i].loaded ) {
			N_Message( TEXT( "Close all opened bots and then try again!" ) );
			return;
		}
	}


	//clean-up old resources
	//

	if( k_system.pBots ) {
		N_Free( k_system.pBots );
		k_system.pBots = NULL;
		k_system.cBots = 0;
	}

	if( k_system.pMaps ) {
		N_Free( k_system.pMaps );
		k_system.pMaps = NULL;
		k_system.cMaps = 0;
	}
	
	if( k_system.pPaths ) {
		N_Free( k_system.pPaths );
		k_system.pPaths = NULL;
		k_system.cPaths = 0;
	}
	
	loadResources();
}


/*
=====================
	loadMapResource
=====================
*/
static mapEntry_t * loadMapResource( TCHAR *szPath ) {
	BYTE buff[BLOCK_SIZE];
	HANDLE hFile;
	DWORD tmp;
	mapEntry_t *mapEntry;
	
	
	hFile = N_FOpenR( szPath );
	if( hFile == INVALID_HANDLE_VALUE ) {
		return NULL;
	}
	
	//increase map count
	//
	k_system.cMaps++;

	//increase memory
	//
	if( k_system.pMaps ) {
		k_system.pMaps = N_Realloc( k_system.pMaps, sizeof( mapEntry_t ) * k_system.cMaps );
	}
	else {
		k_system.pMaps = N_Malloc( sizeof( mapEntry_t ) * k_system.cMaps );
	}

	//obtain map entry pointer
	//
	mapEntry = &( k_system.pMaps[ k_system.cMaps - 1 ] );
	
	//calculate CRC
	//
	crcInit( &(mapEntry->fe.dwCRC) );
	while( ReadFile( hFile, buff, BLOCK_SIZE, &tmp, NULL ) ) {
		if( tmp == 0 ) {
			break;
		}
		crcUpdate( &(mapEntry->fe.dwCRC), buff, tmp );
	}
	crcFinish( &(mapEntry->fe.dwCRC ) );
	
	N_FClose( hFile );
	
	//get map name
	//
	//getMapName( szPath, mapEntry->map.name );
	
	return mapEntry;
}


/*
=====================
	loadBotResource
=====================
*/
static botEntry_t *loadBotResource( TCHAR *szPath ) {
	BYTE buff[BLOCK_SIZE];
	HANDLE hFile;
	DWORD tmp;
	//TCHAR szName[NAMESIZE];
	botEntry_t *botEntry;
	
	
	hFile = N_FOpenR( szPath );
	if( hFile == INVALID_HANDLE_VALUE ) {
		return NULL;
	}
	
	//increase map count
	//
	k_system.cBots++;

	//increase memory
	//
	if( k_system.pBots ) {
		k_system.pBots = N_Realloc( k_system.pBots, sizeof(botEntry_t)*k_system.cBots );
	}
	else {
		k_system.pBots = N_Malloc( sizeof(botEntry_t)*k_system.cBots );
	}

	//obtain map entry pointer
	//
	botEntry = &( k_system.pBots[ k_system.cBots - 1 ] );
	
	//calculate CRC
	//
	crcInit( &(botEntry->fe.dwCRC) );
	while( ReadFile( hFile, buff, BLOCK_SIZE, &tmp, NULL ) ) {
		if( tmp == 0 ) {
			break;
		}
		crcUpdate( &(botEntry->fe.dwCRC), buff, tmp );
	}
	crcFinish( &(botEntry->fe.dwCRC ) );
	
	N_FClose( hFile );

	//get bot name
	//
	//getBotName( szPath, botEntry->bot.name );
	
	return botEntry;
}


/*
=====================
	loadPackResource
=====================
*/
static void loadPackResource( TCHAR *szPath ) {
	TCHAR szResPath[MAX_PATH];
	npkFileEntry_t *pFE;
	npkHeader_t npk;
	mapEntry_t *mapEntry;
	botEntry_t *botEntry;
	HANDLE hPack;
	int cnt;
	int i;
	
	__try {
		pFE = NULL;
						
		hPack = N_FOpenR( szPath );
		if( hPack == INVALID_HANDLE_VALUE ) {
			__leave;
		}
						
		//read pack header
		//
		if( ! N_FRead( hPack, &npk, sizeof( npk ) ) ) {
			__leave;
		}

		//check header
		//
		if( npk.iHeader != NPKHEADER ) {
			__leave;
		}
						
		//check CRC
		//
		if( ! npkVerifyChecksum( szPath ) ) {
			__leave;
		}
		
		//read file entry information
		//
		cnt = npk.dwFileNum;
		pFE = N_Malloc( cnt*sizeof(*pFE) );
		if( ! N_FRead( hPack, pFE, cnt*sizeof(*pFE) ) ) {
			__leave;
		}
						
		//process file entries
		for( i=0; i<cnt; i++ ) {
		
			//decompress resource, and process it
			//
			if( beginDecompress( szPath, pFE[i].dwOffset, szResPath, NULL ) ) {
				//map file
				//
				if( pFE[i].type == NPKENTRY_MAP ) {
					//load map
					//
					mapEntry = loadMapResource( szResPath );
					
					if( mapEntry ) {
						//set-up map entry
						//
						mapEntry->fe.dwFlags = FILE_PACKED;
						mapEntry->fe.pathID = k_system.cPaths-1;
						mapEntry->fe.lOffset = pFE[i].dwOffset;
						mapEntry->fe.lSize = pFE[i].dwSize;
						N_Strcpy( mapEntry->map.name, pFE[i].szIName );
					}
				}
				//bot file
				//
				else if( pFE[i].type == NPKENTRY_BOT ) {
					//load bot
					//
					botEntry = loadBotResource( szResPath );
					
					if( botEntry ) {
						//set bot entry
						//
						botEntry->fe.dwFlags = FILE_PACKED;
						botEntry->fe.pathID = k_system.cPaths-1;
						botEntry->fe.lOffset = pFE[i].dwOffset;
						botEntry->fe.lSize = pFE[i].dwSize;
						N_Strcpy( botEntry->bot.name, pFE[i].szIName );
					}
				}
			
				endDecompress( szResPath );
			}

		} //proc files

	} //try
	
	__finally {
		N_FClose( hPack );
		if( pFE ) {
			N_Free( pFE );
		}
	}
}


#ifdef DEBUG_COMPILE
/*
=====================
	outputDebugInfo
=====================
*/
void outputDebugInfo( TCHAR * Path) {
	HANDLE hFile;
	TCHAR buffer[200] = {0,};
	memory_t * mem;
	int num;
	
	hFile = N_FOpenC( Path );
	
	mem = kMemory;
	
	while( mem!=NULL ) {
		N_Sprintf( buffer, 200, TEXT( "%s (%d)\r\n" ), mem->file, mem->line );
		WriteFile( hFile, buffer, N_Strlen( buffer ), &num, NULL );
		N_Memset( buffer, 0, sizeof( buffer ) );
		mem = mem->Next;
	}
	
	N_FClose( hFile );
}

#endif
