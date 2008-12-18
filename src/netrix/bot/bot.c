
//Netrix Bot interface
//

#undef __N_FILE__
#define __N_FILE__ TEXT( "bot.c" )

#include "../compile.h"
#include <windows.h>
#include <stdlib.h>

#include "../../netrixlib/netrixlib.h"
#include "../../nxc/nxc.h"
#include "../game/sys.h"
#include "../game/func.h"
#include "../game/seq.h"
#include "../game/replay.h"
#include "../game/scheduler.h"
#include "../common/npk.h"
#include "../common/keys.h"
#include "bot.h"
#include "botlib.h"


//set this to current bot game (left or right)
//
game_t *kBotGame;

//set this to current bot
//
bot_t *kBot;

//this will hold temporary figure position
//
figure_t kBotFigure;

//Action Space (testing zone)
//
int *kBotAS;

/*
=====================
	botInit
=====================
*/
BOOL botInit() {
	
	//set globals
	//
	kBotGame = NULL;
	kBot = NULL;
	kBotAS = createSpace();
	N_Memset( &kBotFigure, 0, sizeof(figure_t) );

	//register NetrixC botlib functions
	//
	NXC_RegisterFunction( "SetFigure",		B_SetFigure );
	NXC_RegisterFunction( "WriteFigure",	B_WriteFigure );
	NXC_RegisterFunction( "UseFigure",		B_UseFigure );
	NXC_RegisterFunction( "GetState",		B_GetState );
	NXC_RegisterFunction( "GetPosX",		B_GetPosX );
	NXC_RegisterFunction( "GetPosY",		B_GetPosY );
	NXC_RegisterFunction( "GetSpaceX",		B_GetSpaceX );
	NXC_RegisterFunction( "GetSpaceY",		B_GetSpaceY );
	NXC_RegisterFunction( "PushLeft",		B_PushLeft );
	NXC_RegisterFunction( "PushRight",		B_PushRight );
	NXC_RegisterFunction( "PushDown",		B_PushDown );
	NXC_RegisterFunction( "RowsEliminated", B_RowsEliminated );
	NXC_RegisterFunction( "OccupiedCells",	B_OccupiedCells );
	NXC_RegisterFunction( "ShadowedHoles",	B_ShadowedHoles );
	NXC_RegisterFunction( "PileHeight",		B_PileHeight );
	NXC_RegisterFunction( "WellHeights",	B_WellHeights );
	NXC_RegisterFunction( "TouchingEdges",	B_TouchingEdges );
	NXC_RegisterFunction( "PushAS",			B_PushAS );
	NXC_RegisterFunction( "PopAS",			B_PopAS );
	NXC_RegisterFunction( "RestoreAS",		B_RestoreAS );
	NXC_RegisterFunction( "EliminateRows",	B_EliminateRows );
	NXC_RegisterFunction( "Message",		B_Message );
	NXC_RegisterFunction( "Log",			B_Log );
	NXC_RegisterFunction( "Random",			B_Random );
	NXC_RegisterFunction( "Quit",			B_Quit );

	return TRUE;
}


/*
=====================
	botDestroy
=====================
*/
void botDestroy() {
	freeSpace( kBotAS );
}

/*
=====================
	botLoad
=====================
*/
BOOL botLoad( game_t *game, botEntry_t *be ) {
	TCHAR szFileName[MAX_PATH];
	BOOL bRes;

	//set globals
	//
	kBotGame = game;
	kBotFigure = *game->pFig;
	kBot = &be->bot;

	//check if script is packed
	//
	if( be->fe.dwFlags & FILE_PACKED ) {
		if( beginNpkExtract( &(be->fe), szFileName ) ) {
			bRes = NXC_Init( szFileName, &(be->bot.program), be->bot.name );
			endNpkExtract( szFileName );
		}
		else {
			return FALSE;
		}
	}
	//process single script file
	//
	else {
		bRes = NXC_Init( k_system.pPaths[be->fe.pathID].szPath, &(be->bot.program), be->bot.name );
	}
	
	if( !bRes ) {
		return FALSE;
	}
	
	be->loaded = TRUE;
	
	return TRUE;
}


/*
=====================
	botUnload
=====================
*/
void botUnload( botEntry_t *be ) {
	if( be->loaded ) {
		NXC_Destroy( &(be->bot.program) );
		be->loaded = FALSE;
		N_Memset( &(be->bot.program), 0, sizeof(program_t) );
	}
}


/*
=====================
	botUpdate
=====================
*/
void botUpdate( game_t *game ) {

	if( !game ) {
		return;
	}
	
	//set game
	//
	kBotGame = game;
	
	N_Memcpy( kBotAS, game->AS, sizeof(int)*SPACESIZE );

	//set figure (damn type...)
	//
	kBotFigure.type = kBotGame->pFig->type;
	kBotFigure.state = kBotGame->pFig->state;
	kBotFigure.pos.x = kBotGame->pFig->pos.x;
	kBotFigure.pos.y = kBotGame->pFig->pos.y;

	
	//set bot
	//
	switch( game->place ) {
		case LEFTGAME:
			kBot = &k_system.pBots[k_system.idBotLeft].bot;
			break;
		case RIGHTGAME:
			kBot = &k_system.pBots[k_system.idBotRight].bot;
			break;
		default:
			return;
	}
	
	//run Update() function
	//
	NXC_Update( &kBot->program );
	
	//interpolate bot movement path
	//
	if( k_system.flags & SF_BOTFAST ) {
		botInterpolateFast( kBotGame, kBotGame->pFig, &kBotFigure );
	} else {
		botInterpolate( kBotGame, kBotGame->pFig, &kBotFigure );
	}
}


/*
=====================
	botInterpolate
=====================
*/
void botInterpolate( game_t *game, figure_t *start, figure_t *end ) {
	int startX, startY;
	int endX, endY;
	int size = 0;
	int t;
	int i;
	msg_t msg;
	
	switch( kBot->difficulty ) {
		case BOT_EASY:
			t = DELTA_EASY;
			break;
		case BOT_NORMAL:
			t = DELTA_NORMAL;
			break;
		case BOT_HARD:
			t = DELTA_HARD;
			break;
		case BOT_NIGHTMARE:
			t = DELTA_NIGHTMARE;
			break;
	}
	
	//set positions
	//
	startX = start->pos.x;
	startY = start->pos.y;
	endX = end->pos.x;
	endY = end->pos.y;

	//rotate figure
	//
	i=0;
	while( (i++ < 4) && (start->state != end->state) ) {
		GenerateKey( TEXT( "up" ), game );
	}
	
	//move on X axis
	//
	if( startX < endX ) {
		msg = RIGHTMOVE;
	}
	else {
		msg = LEFTMOVE;
	}
	for( i=0; i<abs(startX-endX); i++ ) {
		game->botFrames.frames[size][0] = t;
		game->botFrames.frames[size][1] = msg;
		size++;
	}
	
	//move on Y axis
	//
	msg = DOWNMOVE;
	for( i=0; i<=abs(startY-endY); i++ ) {
		game->botFrames.frames[size][0] = t;
		game->botFrames.frames[size][1] = msg;
		size++;
	}
	
	game->botFrames.size = size;
	game->botFrames.idx = 0;
	
	popBotSlices( game );
	
	//schedule frame
	//
	pushTimeSlice( TIME_BOT_FRAME, 1, 1, game, BOT, TRUE );
}


/*
=====================
	botFrame
=====================
*/
void botFrame( game_t *game ) {
	int interval;

	//check if all frames were executed
	//
	if( game->botFrames.idx >= game->botFrames.size ) {
		return;
	}
	
	switch( game->botFrames.frames[game->botFrames.idx][1] ) {
		case LEFTMOVE:
			GenerateKey( TEXT( "left" ), game );
			break;
		case RIGHTMOVE:
			GenerateKey( TEXT( "right" ), game );
			break;
		case DOWNMOVE:
			GenerateKey( TEXT( "down" ), game );
			break;
		default:
			return;
	}
	
	game->botFrames.idx++;
	
	//schedule next frame
	//
	interval = game->botFrames.frames[game->botFrames.idx][0];
	pushTimeSlice( TIME_BOT_FRAME, interval, interval, game, BOT, FALSE );
}


/*
=====================
	botInterpolateFast
=====================
*/
void botInterpolateFast( game_t *game, figure_t *start, figure_t *end ) {
	int startX, startY;
	int endX, endY;
	int size = 0;
	int i;
	msg_t msg;
	
	//set positions
	//
	startX = start->pos.x;
	startY = start->pos.y;
	endX = end->pos.x;
	endY = end->pos.y;

	//rotate figure
	//
	i = 0;
	while( (i++ < 4) && (start->state != end->state) ) {
		GenerateKey( TEXT( "up" ), game );
	}
	
	//move on X axis
	//
	if( startX < endX ) {
		msg = RIGHTMOVE;
	}
	else {
		msg = LEFTMOVE;
	}
	for( i=0; i<abs(startX-endX); i++ ) {
		if( msg == LEFTMOVE ) {
			GenerateKey( TEXT( "left" ), game );
		}
		else if( msg == RIGHTMOVE ) {
			GenerateKey( TEXT( "right" ), game );
		}
		seqProc();
	}
	
	//move on Y axis
	//
	for( i=0; i<=abs(startY-endY); i++ ) {
		GenerateKey( TEXT( "down" ), game );
		seqProc();
	}
}