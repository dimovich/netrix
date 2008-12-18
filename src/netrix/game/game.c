
//high level game functions
//

#undef __N_FILE__
#define __N_FILE__ TEXT( "game.c" )

#include "../compile.h"
#include <windows.h>
#include <commdlg.h>

#include "../../netrixlib/netrixlib.h"

#include "../win32/func_win.h"
#include "sys.h"
#include "func.h"
#include "seq.h"
#include "replay.h"
#include "game.h"

//NGM1
#define GAME_HEADER	( ( '1'<<24) + ('M'<<16) + ('G'<<8) + 'N' )

//game file filter (OFN)
#define GAME_FILTER TEXT( "(*.ngm)\0*.ngm\0" )

//game file extension (OFN)
#define GAME_EXT TEXT( "*.ngm" )

//read/write data in chunks
#define BLOCK_SIZE (2*1024)


static int g_mapID; //map id
extern HWND hwndCtlEndgame;


/*
=====================
	singleGameStart
=====================
*/
BOOL singleGameStart() {
	//left game
	//
	initLeftGame();

	//system
	//
	k_system.cPlayers = 1;
	k_system.gameType = GSINGLE;
	initFigPool();
	unpause();

	//init replay system
	if( k_system.flags & SF_RECORDGAME ) {
		if( !initReplaySystem() ) {
			return FALSE;
		}
	}

	//load map
	//
	if( k_system.idMap >= 0 ) {
		loadMapEntry( &k_system.pMaps[k_system.idMap] );
		g_mapID = k_system.idMap;
		k_system.flags |= SF_MAPUSED;
		setMap( k_system.pLeftGame, g_mapID );
		
		pushTimeSlice( TIME_TRIGGER_RESPAWN, TIME_TRIGGER_RESPAWN_INTERVAL,
			0, k_system.pLeftGame, GAME, FALSE );
	}

	seqProc();

	return TRUE;
}


/*
=====================
	singleGameEnd
=====================
*/
BOOL singleGameEnd() {

	//interactivity stops here
	//
	endAllTimeSlices();

	//end demo
	//
	if( k_system.flags & SF_DEMOPLAY ) {
		endDemo();
	}

	//save replay
	//
	if( k_system.flags & SF_RECORDGAME ) {
		endReplaySystem();
		k_system.flags &= ~SF_RECORDGAME;
	}
	
	//unload map
	//
	if( k_system.flags & SF_MAPUSED ) {
		unloadMapEntry( &k_system.pMaps[g_mapID] );
		k_system.flags &= ~SF_MAPUSED;
		k_system.idMap = -1;
	}

	//destroy system
	//
	destroyFigPool();
	k_system.gameType = GNO;
	k_system.cPlayers = 0;
	
	//destroy left game
	//
	destroyLeftGame();

	updateWindow( CGF_CLEANUP );
	
	return TRUE;
}


/*
=====================
	vsGameStart
=====================
*/
BOOL vsGameStart() {
	//Left Game
	//
	initLeftGame();

	//Right Game
	//
	initRightGame();

	//System
	//
	k_system.cPlayers = 2;
	k_system.gameType = GVS;
	unpause();
	initFigPool();
	
	//init replay system
	//
	if( k_system.flags & SF_RECORDGAME ) {
		if( !initReplaySystem() ) {
			return FALSE;
		}
	}

	//load map
	//
	if( k_system.idMap >= 0 ) {
		loadMapEntry( &k_system.pMaps[k_system.idMap] );
		g_mapID = k_system.idMap;
		k_system.flags |= SF_MAPUSED;
		setMap( k_system.pLeftGame, g_mapID );
		setMap( k_system.pRightGame, g_mapID );
		
		pushTimeSlice( TIME_TRIGGER_RESPAWN, TIME_TRIGGER_RESPAWN_INTERVAL,
			0, k_system.pLeftGame, GAME, FALSE );
		
		pushTimeSlice( TIME_TRIGGER_RESPAWN, TIME_TRIGGER_RESPAWN_INTERVAL,
			0, k_system.pRightGame, GAME, FALSE );
	}

	seqProc();
	
	return TRUE;
}


/*
=====================
	vsGameEnd
=====================
*/
BOOL vsGameEnd() {

	endAllTimeSlices();
	
	//end demo
	//
	if( k_system.flags & SF_DEMOPLAY ) {
		endDemo();
	}
	
	//save replay
	//
	if( k_system.flags & SF_RECORDGAME ) {
		endReplaySystem();
		k_system.flags &= ~SF_RECORDGAME;
	}
	
	destroyFigPool();	
	k_system.gameType = GNO;
	k_system.cPlayers = 0;

	//unload map
	//
	if( k_system.flags & SF_MAPUSED ) {
		k_system.flags &= ~SF_MAPUSED;
		unloadMapEntry( &k_system.pMaps[g_mapID] );
	}
	
	//destroy games
	//
	destroyLeftGame();
	destroyRightGame();
	
	updateWindow( CGF_CLEANUP );
	
	return TRUE;
}


/*
=====================
	botGameStart
=====================
*/
BOOL botGameStart() {
	//check if there is any valid bot
	if( k_system.cBots <= 0 ) {
		return FALSE;
	}
	
	//init games
	//
	initLeftGame();
	initRightGame();
	k_system.pRightGame->flags |= SF_BOTGAME;
	
	//load bot
	//
	if( !botLoad( k_system.pRightGame, &k_system.pBots[k_system.idBotRight] ) ) {
		return FALSE;
	}
	//set bot difficulty
	k_system.pBots[k_system.idBotRight].bot.difficulty = BOT_NORMAL;
	
	//System
	//
	k_system.cPlayers = 2;
	k_system.gameType = GBOT;
	unpause();
	initFigPool();
	
	//init replay system
	//
	if( k_system.flags & SF_RECORDGAME ) {
		if( !initReplaySystem() ) {
			return FALSE;
		}
	}

	seqProc();
	
	return TRUE;
}


/*
=====================
	botGameEnd
=====================
*/
BOOL botGameEnd() {

	endAllTimeSlices();
	
	//end replay system
	//
	if( k_system.flags & SF_RECORDGAME ) {
		if( !endReplaySystem() ) {
			return FALSE;
		}
		k_system.flags &= ~SF_RECORDGAME;
	}
	
	destroyFigPool();
	k_system.gameType = GNO;
	k_system.cPlayers = 0;

	
	//destroy bot
	//
	botUnload( &k_system.pBots[k_system.idBotRight] );
	
	//destroy games
	//
	destroyRightGame();
	destroyLeftGame();
	
	updateWindow( CGF_CLEANUP );
	
	return TRUE;
}


/*
=====================
	botSingleGameStart
=====================
*/
BOOL botSingleGameStart() {
	//check if any bot present
	//
	if( k_system.cBots <= 0 ) {
		N_Trace( "no valid bot present" );
		return FALSE;
	}
	
	//init game
	//
	initLeftGame();
	k_system.pLeftGame->flags |= SF_BOTGAME;
	
	//load bot
	//
	if( k_system.idBotLeft >= 0 ) {
		if( !botLoad( k_system.pLeftGame, &k_system.pBots[k_system.idBotLeft] ) ) {
			return FALSE;
		}
	}
	else {
		return FALSE;
	}
	
	//set system
	//
	k_system.cPlayers = 1;
	k_system.gameType = GBOTSINGLE;
	k_system.flags |= SF_BOTFAST;
	initFigPool();
	
	unpause();
	
	seqProc();
	
	return TRUE;
}


/*
=====================
	botSingleGameEnd
=====================
*/
BOOL botSingleGameEnd() {
	//destroy system
	//
	endAllTimeSlices();
	
	destroyFigPool();
	k_system.gameType = GNO;
	k_system.cPlayers = 0;
	k_system.flags &= ~SF_BOTFAST;

	botUnload( &k_system.pBots[k_system.idBotLeft] );

	destroyLeftGame();
	
	updateWindow( CGF_CLEANUP );
	
	return TRUE;
}


/*
=====================
	startSavedGame
	-----------------
	NOTE: it will be ended by *GameEnd()
=====================
*/
BOOL startSavedGame() {
	TCHAR szPath[MAX_PATH];
	gameType_t gmtype;
	BOOL bRes;
	game_t *game;

	__try {
		bRes = FALSE;
	
		//get saved game path
		if( !GetLoadPath( k_system.hwnd, GAME_FILTER, GAME_EXT, szPath ) ) {
			__leave;
		}
		
		//get game type
		if( !getSavedGameType( szPath, &gmtype ) ) {
			__leave;
		}
		
		endAllGames();
		
		switch( gmtype ) {
			case GVS:
				//init games
				//
				initLeftGame();
				initRightGame();
				
				//system
				//
				k_system.cPlayers = 2;
				break;

			case GSINGLE:
				//init game
				//
				initLeftGame();

				//system
				//
				k_system.cPlayers = 1;
				break;

			default:
				__leave;
		}
		
		gameLoad( szPath );

		bRes = TRUE;
	}
	
	__finally {
	}
	
	if( bRes ) {
		//push necessary time slices
		//and init some interface elements.
		switch( gmtype ) {
		
			case GVS:
				game = k_system.pRightGame;

				if( game->pFig->desintegrated ) {
					//push block movement time slice
					game->pFig->guid[1] = pushTimeSlice( TIME_BLOCK_YMOVE, TIME_BLOCK_YMOVE_INTERVAL,
						0, game->pFig, BLOCK, TRUE );

					game->pFig->guid[2] = pushTimeSlice( TIME_BLOCK_XMOVE, TIME_BLOCK_XMOVE_INTERVAL,
						0, game->pFig, BLOCK, FALSE );

					game->flags |= SF_NOMOVE;
				}
				else {
					game->pFig->guid[0] = pushTimeSlice( TIME_FIG_YMOVE, TIME_FIG_YMOVE_INTERVAL,
						0, game->pFig, FIGURE, FALSE );
				}
				
				pushTimeSlice( TIME_TRIGGER_RESPAWN, TIME_TRIGGER_RESPAWN_INTERVAL,
					0, game, GAME, FALSE );
				
					/*FALLTHRU*/

			case GSINGLE:
				game = k_system.pLeftGame;

				if( game->pFig->desintegrated ) {
					//push block movement time slice
					game->pFig->guid[1] = pushTimeSlice( TIME_BLOCK_YMOVE, TIME_BLOCK_YMOVE_INTERVAL,
						0, game->pFig, BLOCK, TRUE );
	
					game->pFig->guid[2] = pushTimeSlice( TIME_BLOCK_XMOVE, TIME_BLOCK_XMOVE_INTERVAL,
						0, game->pFig, BLOCK, FALSE );
	
					game->flags |= SF_NOMOVE;
				}
				else {
					game->pFig->guid[0] = pushTimeSlice( TIME_FIG_YMOVE, TIME_FIG_YMOVE_INTERVAL,
						0, game->pFig, FIGURE, FALSE );
				}
				
				pushTimeSlice( TIME_TRIGGER_RESPAWN, TIME_TRIGGER_RESPAWN_INTERVAL,
					0, game, GAME, FALSE );
				break;
			
			default:
				break;
		}
	
		unpause();
		initFigPool();
		seqProc();
	}
	
	return bRes;
}


/*
=====================
	gameSave
=====================
*/
BOOL gameSave() {
	TCHAR szFile[MAX_PATH];
	TCHAR szTmpFile[MAX_PATH];
	HANDLE hTmpFile;
	HANDLE hFile;
	DWORD crc;
	BYTE buff[BLOCK_SIZE];
	game_t *game = NULL;
	figure_t *fig = NULL;
	int i;
	int k, p;
	int tmp1, tmp2;
	gameFileHeader_t gameFileHeader = {0};
	BOOL success;
	BOOL tmpFile;

	//check if current game is valid
	if( (k_system.gameType < GSINGLE) || (k_system.gameType > GVS) ) {
		return FALSE;
	}

	//get save file name
	if( !GetSavePath( k_system.hwnd, GAME_FILTER, GAME_EXT, szFile ) ) {
		return FALSE;
	}

	//set file names
	N_Sprintf( szTmpFile,		MAX_PATH, TEXT( "%s.tmp" ),		szFile );

	__try {
		success = FALSE;
	
		//create temporary file
		hTmpFile = N_FOpenC( szTmpFile);
		if( hTmpFile == INVALID_HANDLE_VALUE ) {
			__leave;
		}
		//record temporary file creation
		tmpFile = TRUE;
		
		//write game data to temporary file
		//
		
		//system
		N_FWrite( hTmpFile, &( k_system.flags ),	sizeof( k_system.flags ) );
		N_FWrite( hTmpFile, &( k_system.pause ),	sizeof( k_system.pause ) );

		//determine number of running games (1 or 2)
		k = 1;
		if( k_system.gameType == GVS ) {
			k = 2;
		}

		//write game data to temporary file
		for( p=0; p<k; p++ ) {

			if( p == 0 ) {
				game = k_system.pLeftGame;
			}
			else if( p == 1 ) {
				game  = k_system.pRightGame;
			}
			
			//check game validity
			if( !game || !game->pFig) {
				__leave;
			}

			fig = game->pFig;
			
			//save game
			//
			N_FWrite( hTmpFile, &( game->flags ),		sizeof( game->flags ) );
			N_FWrite( hTmpFile, &( game->score ),		sizeof( game->score ) );
			N_FWrite( hTmpFile, &( game->newBlock ),	sizeof( game->newBlock ) );
			
			//save figure
			//
			N_FWrite( hTmpFile, &( fig->msg ),			sizeof( fig->msg ) );
			N_FWrite( hTmpFile, &( fig->desintegrated ), sizeof( fig->desintegrated ) );
			
			//if figure is desintegrated,
			//write block data
			if( fig->desintegrated ) {
				N_FWrite( hTmpFile, &( fig->cBlocks ),	sizeof( fig->cBlocks ) );
				N_FWrite( hTmpFile, fig->pBlocks,		fig->cBlocks*sizeof(block_t) );
			}
			//else write figure information
			else {
				N_FWrite( hTmpFile, &( fig->type ),		sizeof( fig->type ) );
				N_FWrite( hTmpFile, &( fig->state ),	sizeof( fig->state ) );
				N_FWrite( hTmpFile, &( fig->prevstate ), sizeof( fig->prevstate ) );
				N_FWrite( hTmpFile, &( fig->pos ),		sizeof( fig->pos ) );
				N_FWrite( hTmpFile, &( fig->prevpos ),	sizeof( fig->prevpos ) );
			}
			
			//AS
			//
			for( i=0; i<CYSPACE; i++ ) {
				N_FWrite( hTmpFile, &( SPACE_CELL( game->AS, i, 0 ) ),
					sizeof(int)*CXSPACE );
			}
			
			//FS
			//
			for( i=0; i<CYSPACE; i++ ) {
				N_FWrite( hTmpFile, &( SPACE_CELL( game->FS, i, 0 ) ),
					sizeof(int)*CXSPACE );
			}
			
			//TS
			//
			for( i=0; i<CYSPACE; i++ ) {
				N_FWrite( hTmpFile, &( SPACE_CELL( game->TS, i, 0 ) ),
					sizeof(int)*CXSPACE );
			}
		}

		//close temporary file
		N_FClose( hTmpFile );

		//setup game header
		gameFileHeader.header = GAME_HEADER;
		gameFileHeader.crc = 0;
		gameFileHeader.gameType = k_system.gameType;
		
		//open temporary file for reading
		hTmpFile = N_FOpenR( szTmpFile );
		if( hTmpFile == INVALID_HANDLE_VALUE ) {
			__leave;
		}
		
		//open end file	
		hFile = N_FOpenC( szFile );
		if( hFile == INVALID_HANDLE_VALUE ) {
			__leave;
		}
		
		//write header
		N_FWrite( hFile, &gameFileHeader, sizeof(gameFileHeader_t) );
		
		//copy data from temporary file
		crcInit( &crc );
		while( TRUE ) {
			ReadFile(hTmpFile, buff, BLOCK_SIZE, &tmp1, NULL );
			if( tmp1 > 0 ) {
				WriteFile( hFile, buff, tmp1, &tmp2, NULL );
				crcUpdate( &crc, buff, tmp1 );
			}
			else {
				break;
			}
		}
		crcFinish( &crc );
		
		//write crc
		gameFileHeader.crc = crc;
		N_FSeek( hFile, 0, FILE_BEGIN );
		N_FWrite( hFile, &gameFileHeader, sizeof(gameFileHeader_t) );
		
		success = TRUE;
	}
	
	//clean-up
	//
	__finally {
		if( hFile != INVALID_HANDLE_VALUE ) {
			N_FClose( hFile );
		}
		
		if( hTmpFile != INVALID_HANDLE_VALUE ) {
			N_FClose( hTmpFile );
		}
	
		//delete temporary file
		if( tmpFile ) {
			DeleteFile( szTmpFile );
		}
	}
	
	return success;
}


/*
=====================
	gameLoad
	-----------------
	memory should be already allocated
=====================
*/
BOOL gameLoad( TCHAR *szFile ) {
	HANDLE hFile;
	DWORD crc;
	game_t *game;
	figure_t *fig;
	BOOL sflag;
	int i;
	int p, k;
	BYTE buff[BLOCK_SIZE];
	int tmp;
	gameFileHeader_t gameFileHeader = {0};

	__try {
		sflag = FALSE;

		hFile = N_FOpenR( szFile );
		if( hFile == INVALID_HANDLE_VALUE ) {
			__leave;
		}
		
		//read header
		N_FRead( hFile, &gameFileHeader, sizeof(gameFileHeader_t) );
		
		//check header
		if( gameFileHeader.header != GAME_HEADER ) {
			__leave;
		}

		//check crc
		crcInit( &crc );
		while( TRUE ) {
			ReadFile( hFile, buff, BLOCK_SIZE, &tmp, NULL );
			if( tmp > 0 ) {
				crcUpdate( &crc, buff, tmp );
			}
			else {
				break;
			}
		}
		crcFinish( &crc );
		if( crc != gameFileHeader.crc ) {
			__leave;
		}
		//restore file position
		N_FSeek( hFile, sizeof(gameFileHeader_t), FILE_BEGIN );
				
		//system
		k_system.gameType = gameFileHeader.gameType;
		N_FRead( hFile, &( k_system.flags ), sizeof( k_system.flags ) );
		N_FRead( hFile, &( k_system.pause ), sizeof( k_system.pause ) );
			
		//determine number of games (1 or 2)
		k = 1;
		if( k_system.gameType == GVS ) {
			k = 2;
		}
		
		//read games	
		for( p=0; p<k; p++ ) {

			if( p == 0 ) {
				game = k_system.pLeftGame;
			}
			else if( p == 1 ) {
				game = k_system.pRightGame;
			}

			//check game validity
			if( !game || !game->pFig ) {
				__leave;
			}

			fig = game->pFig;
				
			//game
			N_FRead( hFile, &( game->flags ),	sizeof( game->flags ) );
			N_FRead( hFile, &( game->score ),	sizeof( game->score ) );
			N_FRead( hFile, &( game->newBlock ), sizeof( game->newBlock ) );
				
			//figure
			N_FRead( hFile, &( fig->msg ),		sizeof( fig->msg ) );
			N_FRead( hFile, &( fig->desintegrated ), sizeof( fig->desintegrated ) );

			//figure is desintegrated, so
			//read block data
			//
			if( fig->desintegrated ) {
				N_FRead( hFile, &( fig->cBlocks ), sizeof(fig->cBlocks) );
					
				//free old blocks
				if( fig->pBlocks ) {
					N_Free( fig->pBlocks );
				}
				fig->pBlocks = N_Malloc( fig->cBlocks*sizeof(block_t) );
					
				N_FRead( hFile, fig->pBlocks, fig->cBlocks*sizeof(block_t) );
			}
			//figure is not desintegrated, so
			//read figure data
			else {
				N_FRead( hFile, &( fig->type ),		sizeof( fig->type ) );
				N_FRead( hFile, &( fig->state ),	sizeof( fig->state ) );
				N_FRead( hFile, &( fig->prevstate ), sizeof( fig->prevstate ) );
				N_FRead( hFile, &( fig->pos ),		sizeof( fig->pos ) );
				N_FRead( hFile, &( fig->prevpos ),	sizeof( fig->prevpos ) );
			}

			//AS
			for( i=0; i<CYSPACE; i++ ) {
				N_FRead( hFile, &( SPACE_CELL( game->AS, i, 0 ) ),
					sizeof(int)*CXSPACE );
			}
			
			//FS
			for( i=0; i<CYSPACE; i++ ) {
				N_FRead( hFile, &( SPACE_CELL( game->FS, i, 0 ) ),
					sizeof(int)*CXSPACE );
			}
				
			//TS
			for( i=0; i<CYSPACE; i++ ) {
				N_FRead( hFile, &( SPACE_CELL( game->TS, i, 0 ) ),
					sizeof(int)*CXSPACE );
			}
		}
		
		sflag = TRUE;
	}
	
	__finally {
		if( hFile != INVALID_HANDLE_VALUE ) {
			N_FClose( hFile );
		}
	}
	
	return sflag;
}


/*
=====================
	initGame
=====================
*/
void initGame( game_t *game ) {
	int i;

	//check arguments
	if( !game ) {
		return;
	}

	game->newBlock = TRUE;
	game->flags = 0;
	game->cLines = 0;
	game->score = 0;
	game->AS = createSpace();
	game->FS = createSpace();
	game->TS = createSpace();

	N_Memset( game->abLines, 0, sizeof(BOOL)*CYSPACE );
		
	for( i=0; i<CXSPACE; i++ ) {
		game->toxicBlocks[i] = -1;
	}

	game->effectList = NULL;
		
	game->pFig = N_Malloc( sizeof( *( game->pFig ) ) );
	game->pNextFig = N_Malloc( sizeof( *( game->pNextFig ) ) );
}


/*
=====================
	destroyGame
=====================
*/
void destroyGame( game_t *game ) {

	if( game != NULL ) {

		freeSpace( game->AS );
		freeSpace( game->FS );
		freeSpace( game->TS );
		
		if( game->pFig->desintegrated ) {
			N_Free( game->pFig->pBlocks );
		}
		
		N_Free( game->pFig );
		N_Free( game->pNextFig );
	}
}


/*
=====================
	initLeftGame
=====================
*/
void initLeftGame() {
	game_t *game;
	
	//setup game
	//
	game =
	k_system.pLeftGame = (game_t *)N_Malloc( sizeof(game_t) );
	initGame( game );
	game->idPlayer = 0;
	game->place = LEFTGAME;
	
	//show windows
	//
	ShowWindow( hwndCtlEndgame, SW_SHOW );
	ShowWindow( k_system.hwndLeftN, SW_SHOW );
	ShowWindow( k_system.hWndScoreLeft, SW_SHOW );
	//resize score window
	MoveWindow( k_system.hWndScoreLeft, LSCOREX, LSCOREY, SCORECX, SCORECY, TRUE );
}


/*
=====================
	destroyLeftGame
=====================
*/
void destroyLeftGame() {
	//destroy game
	//
	if( k_system.pLeftGame ) {
		destroyGame( k_system.pLeftGame );
		N_Free( k_system.pLeftGame );
		k_system.pLeftGame = NULL;
	}
	
	//hide windows
	//
	ShowWindow( k_system.hwndLeftN, SW_HIDE );
	ShowWindow( k_system.hWndScoreLeft, SW_HIDE );
	ShowWindow( hwndCtlEndgame, SW_HIDE );
	
	updateWindow( CGF_DRAWLEFT );
}


/*
=====================
	initRightGame
=====================
*/
void initRightGame() {
	game_t *game;
	
	//setup game
	//
	game =
	k_system.pRightGame = (game_t *)N_Malloc( sizeof(game_t) );
	initGame( game );
	game->idPlayer = 1;
	game->place = RIGHTGAME;
	
	//show windows
	//
	populateRightGUI();
	ShowWindow( k_system.hwndRightN, SW_SHOW );
	ShowWindow( k_system.hWndScoreRight, SW_SHOW );
	//resize score window
	MoveWindow( k_system.hWndScoreRight, RSCOREX, RSCOREY, SCORECX, SCORECY, TRUE );

	//init graphics
	//
	initGraphics( RIGHTGAME );
}


/*
=====================
	destroyRightGame
=====================
*/
void destroyRightGame() {
	//destroy game
	//
	if( k_system.pRightGame ) {
		destroyGame( k_system.pRightGame );
		N_Free( k_system.pRightGame );
		k_system.pRightGame = NULL;
	}
	
	//destroy windows
	//
	destroyGraphics( RIGHTGAME );
	un_populateRightGUI();
}


/*
=====================
	endAllGames
=====================
*/
void endAllGames() {
	switch( k_system.gameType ) {

		case GSINGLE:
			singleGameEnd();
			break;
		
		case GVS:
			vsGameEnd();
			break;
		
		case GBOT:
			botGameEnd();
			break;
		
		case GBOTSINGLE:
			botSingleGameEnd();
			break;
		
		default:
			break;
	}
	
	//reset game over flag
	k_system.flags &= ~SF_GAMEOVER;
}


/*
=====================
	getSavedGameType
=====================
*/
BOOL getSavedGameType( TCHAR *szFile, gameType_t *gameType ) {
	HANDLE hFile;
	gameFileHeader_t gameFileHeader = {0};
	BOOL success;

	//check arguments
	if( !szFile || !gameType ) {
		return FALSE;
	}

	__try {
		success = FALSE;
	
		//open file
		hFile = N_FOpenR( szFile );
		if( hFile == INVALID_HANDLE_VALUE ) {
			__leave;
		}
		
		//read header
		N_FRead( hFile, &gameFileHeader, sizeof(gameFileHeader_t) );
		*gameType = gameFileHeader.gameType;
		
		success = TRUE;
	}
	__finally {
		N_FClose( hFile );
	}
	
	return success;
}


/*
=====================
	gameOverStart
=====================
*/
void gameOverStart() {
	endAllTimeSlices();
	pushTimeSlice( TIME_GAME_OVER, TIME_GAME_OVER_INTERVAL,
		TIME_GAME_OVER_INTERVAL, NULL, GAME, FALSE );
}


/*
=====================
	gameOverEnd
=====================
*/
void gameOverEnd() {
	endAllGames();
}