
//Netrix replay system
//

#undef __N_FILE__
#define __N_FILE__ TEXT( "replay.c" )

#include "../compile.h"
#include <windows.h>
#include <mmsystem.h>

#include "../../netrixlib/netrixlib.h"

#include "../win32/func_win.h"
#include "../common/config.h"
#include "sys.h"
#include "func.h"
#include "scheduler.h"
#include "replay.h"

#define REPLAYHEADER	( ( '1'<<24) + ('P'<<16) + ('E'<<8) + 'R' )
#define BLOCK_SIZE (4*1024)

//replay header
//
typedef struct repHeader_s {
	DWORD		header;		// == REPLAYHEADER
	DWORD		crc;		//replay file CRC
	gameType_t	gametype;	//game type
	DWORD		mapCRC;		//map CRC (used to identify map)
} repHeader_t;

//replay frame
//
typedef struct repFrame_s {
	DWORD	time;	//frame relative time
	place_t	place;	//can be LEFTGAME or RIGHTGAME
	msg_t	msg;	//action to be taken
} repFrame_t;

//keep track of used figures
//
figure_t	*k_replayFig;
int			k_replayFigSize;
int			k_replayFigID;

static HANDLE		ghRepFile;
static DWORD		gFrameTime;
static gameType_t	gGameType;
static int			gMapID;
static DWORD		gSuffix;
static TCHAR		gszTmpPath[MAX_PATH];
static repFrame_t	gFrame;


/*
=====================
	initReplaySystem
=====================
*/
BOOL initReplaySystem() {
	TCHAR szDemosPath[MAX_PATH];
	TCHAR szRepPath[MAX_PATH];

	//check if already recording
	if( k_system.flags & SF_RECORDING ) {
		return FALSE;
	}
	
	//create "demos" directory
	CreateDirectory( kcfTable[VAR_BASE].v.s, NULL );
	N_Sprintf( szDemosPath, MAX_PATH, TEXT( "%s\\demos" ),
		kcfTable[VAR_BASE].v.s );
	CreateDirectory( szDemosPath, NULL );
	
	//create temporary replay file
	//
	N_Sprintf( szRepPath, MAX_PATH, TEXT( "%s\\demos\\~demo.rep" ),
		kcfTable[VAR_BASE].v.s );
	ghRepFile = N_FOpenC( szRepPath );
	if( ghRepFile == INVALID_HANDLE_VALUE ) {
		return FALSE;
	}
	
	//create replay file suffix
	//
	gSuffix = ((GetTickCount()/100) % 100000);
	N_Message( TEXT( "Saving replay to: %s\\demos\\demo_%i.rep" ),
		kcfTable[VAR_BASE].v.s, gSuffix );
	
	//init globals
	//
	k_replayFigID = 0;
	k_replayFigSize = 0;
	k_replayFig = NULL;
	gFrameTime = k_system.dwAccumTime;
	gGameType = k_system.gameType;
	if( gGameType == GBOT ) {
		gGameType = GVS;
	}

	gMapID = k_system.idMap;

	//init system
	//
	k_system.flags |= SF_RECORDING;
	
	return TRUE;
}


/*
=====================
	addReplayFrame
=====================
*/
void addReplayFrame( msg_t msg, place_t place ) {
	repFrame_t frame;

	//determine frame time
	//
	frame.time = k_system.dwAccumTime - gFrameTime;
	gFrameTime = k_system.dwAccumTime;
	
	//prevent endless time slice
	if( frame.time == 0 ) {
		frame.time = 1;
	}
	
	frame.place = place;
	frame.msg	= msg;
	
	//write frame data
	N_FWrite( ghRepFile, &frame, sizeof(frame) );
}


/*
=====================
	endReplaySystem
=====================
*/
BOOL endReplaySystem() {
	repHeader_t repHeader = {0};
	HANDLE hFile;
	HANDLE hTmpFile;
	BYTE buff[BLOCK_SIZE];
	TCHAR szRepTmpPath[MAX_PATH];
	TCHAR szRepPath[MAX_PATH];
	TCHAR szTmpPath[MAX_PATH];
	TCHAR szArchPath[MAX_PATH];
	DWORD crc;
	DWORD tmp, tmp2;
	int i;

	//this will make replay hold till
	//the end.
	//
	addReplayFrame( NOMOVE, LEFTGAME );

	N_FClose( ghRepFile );


	//create paths
	//
	N_Sprintf( szTmpPath, MAX_PATH, TEXT( "%s\\demos\\~demo.rep" ),
		kcfTable[VAR_BASE].v.s );
	N_Sprintf( szRepPath, MAX_PATH, TEXT( "%s\\demos\\demo_%i.rep" ),
		kcfTable[VAR_BASE].v.s, gSuffix );
	N_Sprintf( szRepTmpPath, MAX_PATH, TEXT( "%s.tmp" ), szRepPath );
	N_Sprintf( szArchPath, MAX_PATH, TEXT( "%s.z" ), szRepTmpPath);

	//create temporary replay file
	//
	hFile = N_FOpenC( szRepTmpPath );
	if( hFile == INVALID_HANDLE_VALUE ) {
		return FALSE;
	}
	
	//write figure data
	//
	N_FWrite( hFile, &k_replayFigSize, sizeof(k_replayFigSize) );
	for( i=0; i<k_replayFigSize; i++ ) {
		N_FWrite( hFile, &(k_replayFig[i].type), sizeof(k_replayFig[i].type) );
		N_FWrite( hFile, &(k_replayFig[i].state), sizeof(k_replayFig[i].state) );
	}
	
	//copy replay data from temporary file
	//
	hTmpFile = N_FOpenR( szTmpPath );
	while( ReadFile( hTmpFile, buff, BLOCK_SIZE, &tmp, NULL ) ) {
		if( tmp == 0 ) {
			break;
		}
		WriteFile( hFile, buff, tmp, &tmp2, NULL );
	}
	
	N_FClose( hTmpFile );
	N_FClose( hFile );

	
	//create replay file
	//
	hFile = N_FOpenC( szRepPath );
	
	//write header
	//
	repHeader.header = REPLAYHEADER;
	repHeader.gametype = gGameType;
	if( gMapID >= 0 ) {
		repHeader.mapCRC = k_system.pMaps[gMapID].fe.dwCRC;
	}
	else {
		repHeader.mapCRC = 0;
	}
	N_FWrite( hFile, &repHeader, sizeof(repHeader) );
		
	crcInit( &crc );
	
	//compress
	//
	if( beginCompress( szRepTmpPath ) ) {
		hTmpFile = N_FOpenR( szArchPath );
		
		while( ReadFile( hTmpFile, buff, BLOCK_SIZE, &tmp, NULL ) ) {
			if( tmp == 0 ) {
				break;
			}
			WriteFile( hFile, buff, tmp, &tmp2, NULL );
			crcUpdate( &crc, buff, tmp );
		}
		
		N_FClose( hTmpFile );
		endCompress( szRepTmpPath );
	}
	
	crcFinish( &crc );
	
	//write CRC
	//
	repHeader.crc = crc;
	N_FSeek( hFile, 0, FILE_BEGIN );
	N_FWrite( hFile, &repHeader, sizeof(repHeader) );


	//clean-up
	//

	N_FClose( hFile );
	
	//delete ~demo.rep and demo_%i.rep.tmp
	//
	DeleteFile( szTmpPath );
	DeleteFile( szRepTmpPath );
	
	N_Free( k_replayFig );
	k_replayFig = NULL;
	k_replayFigSize = 0;
	k_replayFigID = 0;

	gFrameTime = 0;
	gGameType = GNO;
	gMapID = -1;
	
	k_system.flags &= ~SF_RECORDING;
	
	return TRUE;
}


/*
=====================
	playDemo
=====================
*/
BOOL playDemo() {
	repHeader_t repHeader = {0};
	HANDLE hFile = INVALID_HANDLE_VALUE;
	TCHAR szRepPath[MAX_PATH];
	DWORD crc;
	DWORD tmp;
	BYTE buff[BLOCK_SIZE];
	BOOL bRes;
	int mapID;
	int i;
	
	__try {
		bRes = FALSE;
		
		//get replay file path
		//
		if( !GetLoadPath( k_system.hwnd, TEXT( "(*.rep)\0*.rep" ), TEXT( "*.rep" ), szRepPath ) ) {
			__leave;
		}

		//open replay file
		//
		hFile = N_FOpenR( szRepPath );
		if( hFile == INVALID_HANDLE_VALUE ) {
			__leave;
		}
		
		//read header
		//
		N_FRead( hFile, &repHeader, sizeof(repHeader) );
		
		//check header
		//
		if( repHeader.header != REPLAYHEADER ) {
			__leave;
		}
		
		//check map availability
		//
		if( (repHeader.mapCRC != 0) && !seeMapID( repHeader.mapCRC, NULL ) ) {
			__leave;
		}
		
		//check CRC
		//
		crcInit( &crc );
		while( ReadFile( hFile, buff, BLOCK_SIZE, &tmp, NULL ) ) {
			if( tmp == 0 ) {
				break;
			}
			crcUpdate( &crc, buff, tmp );
		}
		crcFinish( &crc );
		if( repHeader.crc != crc ) {
			__leave;
		}

		//restore file position
		//
		N_FSeek( hFile, sizeof(repHeader), FILE_BEGIN );
		
		//end all running games
		//
		endAllGames();

		//decompress replay data
		//		
		if( beginDecompress( szRepPath, sizeof(repHeader), gszTmpPath, NULL ) ) {
		
			ghRepFile = N_FOpenR( gszTmpPath );
			if( ghRepFile == INVALID_HANDLE_VALUE ) {
				__leave;
			}
		
			//read figure data
			//
			N_FRead( ghRepFile, &k_replayFigSize, sizeof(k_replayFigSize) );
			k_replayFig = N_Malloc( k_replayFigSize*sizeof(figure_t) );
			for( i=0; i<k_replayFigSize; i++ ) {
				N_FRead( ghRepFile, &(k_replayFig[i].type), sizeof(k_replayFig[i].type) );
				N_FRead( ghRepFile, &(k_replayFig[i].state), sizeof(k_replayFig[i].state) );
			}
			k_replayFigID = 0;
			
			bRes = TRUE;
		}
	}
	
	__finally {
		
		if( hFile != INVALID_HANDLE_VALUE ) {
			N_FClose( hFile );
		}
	
		//check if failed
		//
		if( bRes == FALSE ) {
			if( k_replayFig ) {
				N_Free( k_replayFig );
				k_replayFig = NULL;
				k_replayFigSize = 0;
			}
		}
	}
	
	//play demo
	//
	if( bRes ) {

		k_system.flags |= SF_DEMOPLAY;
		
		//find map
		//
		if( repHeader.mapCRC == 0 ) {
			k_system.idMap = -1;
		}
		else {
			if( seeMapID( repHeader.mapCRC, &mapID ) ) {
				k_system.idMap = mapID;
			}
			else {
				return FALSE;
			}
		}

		//start the game
		//
		switch( repHeader.gametype ) {
			case GSINGLE:
				singleGameStart();
				break;

			case GVS:
				vsGameStart();
				break;

			default:
				return FALSE;
		}
		
		//schedule frame
		//
		N_FRead( ghRepFile, &gFrame, sizeof(gFrame) );
		pushTimeSlice( TIME_REPLAY_FRAME, gFrame.time, gFrame.time, NULL, REPLAY, FALSE );
		
		N_Message( TEXT( "Replaying: %s" ), szRepPath );
	}
	
	
	return bRes;
}


/*
=====================
	playNextFrame
=====================
*/
void playNextFrame() {
	game_t *game;

	//check replay file
	//
	if( ghRepFile == INVALID_HANDLE_VALUE ) {
		return;
	}
	
	//determine game
	//
	switch( gFrame.place ) {
		case LEFTGAME:
			game = k_system.pLeftGame;
			break;
		
		case RIGHTGAME:
			game = k_system.pRightGame;
			break;
	}
	
	//check game validity
	//
	if( game == NULL ) {
		return;
	}
	
	if( game->pFig == NULL ) {
		return;
	}
	
	//execute frame action
	//
	switch( gFrame.msg ) {

		case LEFTMOVE:
			moveFigLeft( game->pFig );
			break;
		
		case RIGHTMOVE:
			moveFigRight( game->pFig );
			break;
		
		case ROTATE:
			rotateFig( game->pFig );
			break;
		
		case CRUSH:
			crushFig( game );
			break;
		
		case DOWNMOVE:
			moveFigDown( game->pFig );
			break;

		default:
			break;
	}
	
	//read next frame time
	//
	if( !N_FRead( ghRepFile, &gFrame, sizeof(gFrame) ) ) {
		endAllGames();
		return;
	}

	//prevent endless time slice
	//
	if( gFrame.time == 0 ) {
		gFrame.time = 1;
	}

	pushTimeSlice( TIME_REPLAY_FRAME, gFrame.time, gFrame.time, NULL, REPLAY, FALSE );
}


/*
=====================
	endDemo
=====================
*/
void endDemo() {
	k_system.flags &= ~SF_DEMOPLAY;

	N_FClose( ghRepFile );
	ghRepFile = INVALID_HANDLE_VALUE;

	endDecompress( gszTmpPath );

	N_Free( k_replayFig );
	k_replayFig = NULL;
	k_replayFigSize = 0;
	k_replayFigID = 0;
}