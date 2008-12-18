
//game engine
//

#undef __N_FILE__
#define __N_FILE__ TEXT( "seq.c" )

#include "../compile.h"
#include <windows.h>
#include <tchar.h>

#include "../../netrixlib/netrixlib.h"

#include "../common/config.h"
#include "sys.h"
#include "scheduler.h"
#include "trigger.h"
#include "seq.h"
#include "func.h"


#define BUFF_SIZE 50


/*
=====================
	seqProc
=====================
*/
BOOL seqProc() {
	switch( k_system.gameType ) {

		//Single
		//
		case GSINGLE:
			//left
			if( k_system.pLeftGame ) {
				seqProcGame( k_system.pLeftGame );
			}
			break;

		//Player vs. Player
		//
		case GVS:
			//left
			if( k_system.pLeftGame ) {
				seqProcGame( k_system.pLeftGame );
			}

			//right
			if( k_system.pRightGame ) {
				seqProcGame( k_system.pRightGame );
			}
			break;

		//Player vs. Computer
		//
		case GBOT:
			//left
			if( k_system.pLeftGame ) {
				seqProcGame( k_system.pLeftGame );
			}
			
			//right
			if( k_system.pRightGame ) {
				seqProcGame( k_system.pRightGame );
			}
			break;
		
		//Computer only
		//
		case GBOTSINGLE:
			//left
			if( k_system.pLeftGame) {
				seqProcGame( k_system.pLeftGame );
			}
			break;
		
		default:
			break;

	} //switch
	
	//game over (Bwahaha!!!)
	if( k_system.flags & SF_GAMEOVER ) {
		gameOverStart();
	}
	
	updateWindow( CGF_DRAWBOTH );
	
	return TRUE;
}


/*
=====================
	seqProcGame
=====================
*/
BOOL seqProcGame( game_t *game ) {
	figure_t * tmpfig = NULL;
	TCHAR buff[BUFF_SIZE] = {0};

	//check game validity
	if( !game ) {
		return FALSE;
	}

	//
	//process internal game msgs
	//

	//skip sequence
	if( FLAG(k_system.flags, SF_GAMEOVER) || FLAG(game->flags, SF_SKIPSEQ) || FLAG(game->flags, SF_GAMEWAIT) ) {
		return TRUE;
	}

	//add toxic blocks
	if( game->flags & SF_ADDBLOCKS ) {
		addGroundBlocks( game );
		game->flags &= ~SF_ADDBLOCKS;
		pushEffect(  EFFECT_GROUND_TOXICBLOCKS, game->place );

		//update bot
		//
		if( game->flags & SF_BOTGAME ) {
			botUpdate( game );
		}
	}

	//spawn new figure
	//	
	if( game->newBlock ) {
		game->flags &= ~SF_SKIPTRIG;
		
		//remove any pending lines
		removeLines( game );

		//generate a new figure
		tmpfig = genFig( game );

		//draw next figure
		drawNFS( game->place );

		//test if possible to place figure on action space
		if( !tryFig( tmpfig, game->AS ) ) {
			game->flags |= SF_GAMELOST;
			k_system.flags |= SF_GAMEOVER;
			return FALSE;
		}

		//write figure to Figure Space
		writeSpace( game->pFig, game->FS );
		
		//setup game
		game->pFig->msg = YMOVE;
		game->newBlock = FALSE;

		//push downward movement slice
		game->pFig->guid[0] = pushTimeSlice( TIME_FIG_YMOVE, TIME_FIG_YMOVE_INTERVAL, 0,
			game->pFig, FIGURE, FALSE );
		
		//update bot
		if( game->flags & SF_BOTGAME ) {
			botUpdate( game );
		}
	}
	//ordinary figure movement
	//
	else {
		//desintegrated figure
		if( game->pFig->desintegrated  ) {
			seqProcGameBlocks( game );
		}
		//normal figure
		else {
			seqProcGameFig( game );
		}
	}

	//check triggers
	checkTriggers( game );

	//check if flag has been captured
	if( game->flags & SF_FLAGCAPTURED ) {
		game->score += 10;
		game->flags &= ~SF_FLAGCAPTURED;
		drawScore( game->place );
	}

	//next sequence a new block should be spawned,
	//so we prepare the ground for it
	if( game->newBlock ) {

		//pop block time slices
		if( game->pFig->desintegrated ) {
			popTimeSlice( game->pFig->guid[1] );
			popTimeSlice( game->pFig->guid[2] );
			game->pFig->desintegrated = FALSE;
		}
		//pop figure time slices
		else {
			if( game->flags & SF_FIGCRUSHED ) {
				game->flags &= ~SF_FIGCRUSHED;
			}
			popTimeSlice( game->pFig->guid[0] );
		}
		
		//pop bot time slices
		if( game->flags & SF_BOTGAME ) {
			popBotSlices( game );
		}

		//push wait time
		pushTimeSlice( TIME_GAME_WAIT, TIME_GAME_WAIT_INTERVAL,
			TIME_GAME_WAIT_INTERVAL, game, GAME, FALSE );
		game->flags |= ( SF_GAMEWAIT | SF_NOMOVE );
		
		//check for complete lines
		checkLines( game );
		if( game->cLines > 0 ) {
			switch( k_system.gameType ) {
				case GSINGLE:
				case GBOTSINGLE:
					drawScore( LEFTGAME );
					break;
				
				case GVS:
				case GBOT:
					drawScore( game->place );
					req( game, SF_ADDBLOCKS );
					break;

				default:
					break;
			}

			//push linekill effect
			if( kcfTable[VAR_EFF_LINEKILL].v.b ) {
				pushEffect( EFFECT_AS_LINEKILL, game->place );
			}
			game->cLines = 0;
		}
	}

	return TRUE;
}


/*
=====================
	seqProcGameFig
	-----------------
	normal figure processing
=====================
*/
void seqProcGameFig( game_t *game ) {

	//check game validity
	if( game == NULL ) {
		return;
	}

	//check figure validity
	if( game->pFig == NULL ) {
		return;
	}

	//try to place figure on Action Space
	if( tryFig( game->pFig, game->AS ) ) {

		game->pFig->prevpos.x	= game->pFig->pos.x;
		game->pFig->prevpos.y	= game->pFig->pos.y;
		game->pFig->prevstate	= game->pFig->state;
				
	}
	//cannot place figure
	else {
		//check if figure landed
		if( game->pFig->msg == YMOVE) {
			game->newBlock = TRUE;
			game->flags |= SF_FIGLANDED;
			
			//push illuminate effect
			if( kcfTable[VAR_EFF_ILLUMINATE].v.b ) {
				pushEffect( EFFECT_GROUND_ILLUMINATE, game->place );
			}
		}

		//revert position
		game->pFig->pos.x	= game->pFig->prevpos.x;
		game->pFig->pos.y	= game->pFig->prevpos.y;
		game->pFig->state	= game->pFig->prevstate;
	}
	
	//clean figure space
	flushSpace( game->FS );

	//write figure to Action Space
	if( game->flags & SF_FIGLANDED ) {
		writeSpace( game->pFig, game->AS );
		game->flags &= ~SF_FIGLANDED;
	}
	//write figure to Figure Space
	else {
		writeSpace( game->pFig, game->FS );
	}
}


/*
=====================
	seqProcGameBlocks
	-----------------
	desintegrated figure processing
=====================
*/
void seqProcGameBlocks( game_t *game ) {
	int			i;
	figure_t	*pFig;
	block_t		*pBlock;
	int			clanded;
	int			oldx, oldy;
	BOOL		bAS;

	//check game validity
	if( game == NULL ) {
		return;
	}

	//check figure validity
	if( game->pFig == NULL ) {
		return;
	}
	
	pFig = game->pFig;
	
	//check movement
	for( i=0; i<pFig->cBlocks; i++ ) {
		pBlock = &( pFig->pBlocks[i] );
		
		//skip blocks that landed
		if( !pBlock->landed ) {
		
			oldx = pBlock->prevpos.x;
			oldy = pBlock->prevpos.y;

			if( (bAS = tryBlock( pBlock, game->AS )) && (tryBlock( pBlock, game->FS )) ) {
				pBlock->prevpos.x = pBlock->pos.x;
				pBlock->prevpos.y = pBlock->pos.y;
			}
			else {
				if( (bAS == FALSE) && (pBlock->msg == DOWNMOVE) ) {
					pBlock->landed = TRUE;

					//push illuminate effect
					if( kcfTable[VAR_EFF_ILLUMINATE].v.b ) {
						pushEffect( EFFECT_GROUND_ILLUMINATE, game->place );
					}
				}
				
				pBlock->pos.x = pBlock->prevpos.x;
				pBlock->pos.y = pBlock->prevpos.y;
			}
			
			//empty old block cell
			SPACE_CELL( game->FS, oldy, oldx ) = MAPCELL_EMPTY;
			
			//write data to FS or AS
			if( pBlock->landed ) {
				SPACE_CELL( game->AS, pBlock->pos.y, pBlock->pos.x ) = MAPCELL_BLOCK;
			}
			else {
				SPACE_CELL( game->FS, pBlock->pos.y, pBlock->pos.x ) = MAPCELL_BLOCK;
			}
		}
	}
	
	//count landed blocks
	clanded = 0;
	for( i=0; i<pFig->cBlocks; i++ ) {
		if( pFig->pBlocks[i].landed ) {
			clanded++;
		}
	}

	//all blocks landed
	if( clanded == pFig->cBlocks ) {
		game->newBlock = TRUE;
		game->flags &= ~SF_NOMOVE;

		pFig->cBlocks = 0;
		N_Free( pFig->pBlocks );
		pFig->pBlocks = NULL;
	}
}


/*
=====================
	req
	-----------------
	Request sth. from the other player.
	~~~
	The same messages will be sent diferently depending on
	game type (local, network, bot, etc.)
=====================
*/
void req( game_t *game, ULONG flag) {
	game_t *other_game = NULL;
	
	//check game validity
	if( game == NULL ) {
		return;
	}
	
	other_game = (game->place == LEFTGAME) ?
		k_system.pRightGame : k_system.pLeftGame;
	
	if( other_game == NULL ) {
		return;
	}
	
	switch( k_system.gameType ) {
		//Single
		case GSINGLE:
			break;

		//Player vs. Player
		case GVS:
			switch( flag ) {
				case SF_ADDBLOCKS:
					other_game->flags |= SF_ADDBLOCKS;
					break;
				
				default:
					break;
			}
			break;

		//Player vs. Computer
		case GBOT:
			switch( flag ) {
				case SF_ADDBLOCKS:
					other_game->flags |= SF_ADDBLOCKS;
					break;
			}
			break;

		default:
			break;
	}
}