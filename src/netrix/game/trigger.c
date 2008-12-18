
#undef __N_FILE__
#define __N_FILE__ TEXT( "trigger.c" )


#include "../compile.h"
#include <windows.h>

#include "../../netrixlib/netrixlib.h"

#include "func.h"
#include "scheduler.h"
#include "trigger.h"
#include "../common/types.h"
#include "../common/figres.h"


/*
=====================
	checkTriggers
	-----------------
	Checks the map for executable triggers
=====================
*/
void checkTriggers( game_t *game ) {
	trigger_t trigger;

	if( (game != NULL) && (game->pFig != NULL) && !(game->flags & SF_SKIPTRIG) ) {

		if( intersectTrigFig( game, &trigger ) ) {

			//set trigger to respawn
			switch( trigger.type ) {
				case TRIGGER_BOMB:
					SPACE_CELL( game->TS, trigger.y, trigger.x ) = TRIGGER_RESPAWN_BOMB;
					break;
				
				case TRIGGER_TELEPORT:
					SPACE_CELL( game->TS, trigger.y, trigger.x ) = TRIGGER_RESPAWN_TELEPORT;
					break;
				
				case TRIGGER_FLAG:
					SPACE_CELL( game->TS, trigger.y, trigger.x ) = TRIGGER_RESPAWN_FLAG;
					break;
			}

			executeTrigger( game, &trigger );
		}
	}
}


/*
=====================
	intersectTrigFig
=====================
*/
BOOL intersectTrigFig( game_t *game, trigger_t *trigger ) {
	int i, j;
	int x, y;
	figure_t *pFig;

	if( (game == NULL) || (trigger == NULL) ) {
		return FALSE;
	}
	
	if( game->pFig == NULL ) {
		return FALSE;
	}
	
	pFig = game->pFig;
	
	trigger->type = TRIGGER_NO;

	if( pFig != NULL ) {
		x = pFig->pos.x;
		y = pFig->pos.y;
	
		for( i=0; i<CYFIG; i++ ) {
			for( j=0; j<CXFIG; j++ ) {
				if( kFigRes[pFig->type][i][pFig->state*CXFIG + j] == 1 ) {
					
					switch( SPACE_CELL( game->TS, i+y, j+x ) ) {

						case TRIGGER_BOMB:
						case TRIGGER_FLAG:
						case TRIGGER_TELEPORT:

							trigger->type = SPACE_CELL( game->TS, i+y, j+x );
							trigger->x = j + x;
							trigger->y = i + y;
							return TRUE;

						default:
							break;
					}
				}
			}
		}
	}
	
	return FALSE;
}


/*
=====================
	executeTrigger
=====================
*/
void executeTrigger( game_t *game, trigger_t *trigger ) {

	if( (game == NULL) || (trigger == NULL) ) {
		return;
	}

	switch( trigger->type ) {

		case TRIGGER_BOMB:
			if( explodeFig( game ) ) {
				game->flags |= SF_SKIPTRIG;
			}
			break;
		
		case TRIGGER_TELEPORT:
			teleportFig( game );
			break;
		
		case TRIGGER_FLAG:
			game->flags |= SF_FLAGCAPTURED;
			break;
	
		default:
			break;
	}
}


/*
=====================
	explodeFig
=====================
*/
BOOL explodeFig( game_t *game ) {
	int i;
	figure_t *pFig;
	block_t *pBlock;

	if( game == NULL ) {
		return FALSE;
	}

	desintegrateFig( game );

	if( game->pFig->desintegrated == FALSE ) {
		return FALSE;
	}
	
	pFig = game->pFig;

	for( i=0; i<pFig->cBlocks; i++ ) {
		pBlock = &( pFig->pBlocks[i] );
		switch( pBlock->type ) {

			case B_T:
			case B_TI:
				pBlock->xvel = 0;
				pBlock->yvel = -1;
				pBlock->accel = 1;
				break;
			
			case B_TL:
				pBlock->xvel = -1;
				pBlock->yvel = -1;				
				pBlock->accel = 1;
				break;
			
			case B_TR:
				pBlock->xvel = 1;
				pBlock->yvel = -1;				
				pBlock->accel = 1;
				break;

			case B_L:
			case B_BL:
				pBlock->xvel = -1;
				pBlock->yvel = 0;
				pBlock->accel = 1;
				break;
			
			case B_BR:
			case B_R:
				pBlock->xvel = 1;
				pBlock->yvel = 0;
				pBlock->accel = 1;
				break;
			
			case B_I:			
			case B_B:
			case B_BI:
				pBlock->xvel = 0;
				pBlock->yvel = 0;
				pBlock->accel = 1;
				break;

			default:
				return FALSE;
		}
	}
	
	popMovementSlices( game->pFig );
	
	//push block movement time slice
	//
	game->pFig->guid[1] = pushTimeSlice( TIME_BLOCK_YMOVE, TIME_BLOCK_YMOVE_INTERVAL,
		0, game->pFig, BLOCK, TRUE );

	game->pFig->guid[2] = pushTimeSlice( TIME_BLOCK_XMOVE, TIME_BLOCK_XMOVE_INTERVAL,
		0, game->pFig, BLOCK, FALSE );
	
	game->flags |= SF_NOMOVE;
	
	return TRUE;
}



#define CHOICES 4
#define ATTEMPTS 5
#define QUADRANTS 4

#define MDLX1 (CXSPACE/4)
#define MDLY1 (CYSPACE/4)
#define MDLX2 (3*CXSPACE/4)
#define MDLY2 (CYSPACE/4)
#define MDLX3 (CXSPACE/4)
#define MDLY3 (3*CYSPACE/4)
#define MDLX4 (3*CXSPACE/4)
#define MDLY4 (3*CYSPACE/4)

//We will pre-calculate all (20) possible
//teleport locations.
static point_t gTeleportPos[CHOICES][ATTEMPTS] = {
	{{MDLX1, MDLY1}, {MDLX1, MDLY1-1}, {MDLX1-1, MDLY1},
	{MDLX1, MDLY1+1}, {MDLX1+1, MDLY1}},
	
	{{MDLX2, MDLY2}, {MDLX2, MDLY2-1}, {MDLX2-1, MDLY2},
	{MDLX2, MDLY2+1}, {MDLX2+1, MDLY2}},
	
	{{MDLX3, MDLY3}, {MDLX3, MDLY3-1}, {MDLX3-1, MDLY3},
	{MDLX3, MDLY3+1}, {MDLX3+1, MDLY3}},
	
	{{MDLX4, MDLY4}, {MDLX4, MDLY4-1}, {MDLX4-1, MDLY4},
	{MDLX4, MDLY4+1}, {MDLX4+1, MDLY4}} };

//choice order for teleport quadrant
static int gTeleportChoice[QUADRANTS][CHOICES] = {
	3, 2, 1, 0,
	2, 3, 0, 1,
	1, 0, 3, 2,	
	0, 1, 2, 3 };

/*
=====================
	teleportFig
	-----------------
	Depending on space quadrant the figure is located in,
	we will try to place the figure in 3 different places:
	* X/Y opposed
	* Y opposed
	* X opposed
	 -- --
	|1 |2 |
	 -- --
	|3 |4 |
	 -- --
=====================
*/
BOOL teleportFig( game_t *game ) {
	figure_t fig;
	int attempt, choice;
	int i, j;
	int quadrant;

	if( (game != NULL) && (game->pFig != NULL) ) {

		N_Memcpy( &fig, game->pFig, sizeof(fig) );
		
		attempt =
		choice	= 0;
		
		if( fig.pos.y >= (CYSPACE/2) ) {
			if( fig.pos.x >= (CXSPACE/2) ) { //4
				quadrant = 3;
			}

			else { //3
				quadrant = 2;
			}
		}
		else {
			if( fig.pos.x >= (CXSPACE/2) ) { //2
				quadrant = 1;
			}
			else { //1
				quadrant = 0;
			}
		}
			
		for( i=0; i<CHOICES; i++ ) {
			choice = gTeleportChoice[quadrant][i];

			for( j=0; j<ATTEMPTS; j++ ) {
				fig.pos.x = gTeleportPos[choice][j].x;
				fig.pos.y = gTeleportPos[choice][j].y;

				if( tryFig( &fig, game->AS ) ) {
					game->pFig->pos.x = fig.pos.x;
					game->pFig->pos.y = fig.pos.y;

					popMovementSlices( game->pFig );
					
					pushTimeSlice( TIME_TELEPORT, TIME_TELEPORT_INTERVAL,
						TIME_TELEPORT_INTERVAL, game, GAME, FALSE );
					
					game->flags |= (SF_NOMOVE | SF_GAMEWAIT);

					return TRUE;
				}
			}
		}
	}
	
	return FALSE;
}


/*
=====================
	triggerRespawn
=====================
*/
void triggerRespawn( game_t *game ) {
	int *ts;
	int i, j;
	
	if( (game != NULL) && (game->TS != NULL) ) {

		ts = game->TS;

		for( i=0; i<CYSPACE; i++ ) {
			for( j=0; j<CXSPACE; j++ ) {

				switch( SPACE_CELL( ts, i, j ) ) {

					case TRIGGER_RESPAWN_BOMB:
						SPACE_CELL( ts, i, j ) = TRIGGER_BOMB;
						break;
					
					case TRIGGER_RESPAWN_TELEPORT:
						SPACE_CELL( ts, i, j ) = TRIGGER_TELEPORT;
						break;
					
					case TRIGGER_RESPAWN_FLAG:
						SPACE_CELL( ts, i, j ) = TRIGGER_FLAG;
						break;
					
					default:
						break;
				}
			}
		}
	}
}