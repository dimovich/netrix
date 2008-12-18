
//common game functions
//

#undef __N_FILE__
#define __N_FILE__ TEXT( "func.c" )

#include "../compile.h"
#include <windows.h>
#include <mmsystem.h>
#include <commdlg.h>
#include <tchar.h>

#include "../../netrixlib/netrixlib.h"

#include "../common/figres.h"
#include "scheduler.h"
#include "game.h"
#include "sys.h"
#include "replay.h"
#include "func.h"
#include "trigger.h"


//increase figure pool in chunks
#define POOL_CHUNK 10

//plain figure array is way much simpler than
//linked lists or whatever...
//
static figure_t *g_figPool;
static int		g_figPoolSize;

//index of each player's figure
static int		g_figID[MAXPLAYERS];

//replay stuff
extern figure_t *k_replayFig;
extern int		k_replayFigSize;
extern int		k_replayFigID;


/*
=====================
	genFig
=====================
*/
figure_t* genFig( game_t * game ) {
	int i;
	int num;

	//enlarge figure pool
	for( i=0; i<k_system.cPlayers; i++ ) {
		//we need more figs
		if( g_figID[i] >= g_figPoolSize-1 ) {
			enlargeFigPool( POOL_CHUNK );
		}
	}

	//copy figures
	//
	N_Memcpy( game->pFig, &g_figPool[g_figID[game->idPlayer]], sizeof(figure_t) );
	g_figID[game->idPlayer]++;
	N_Memcpy( game->pNextFig, &g_figPool[g_figID[game->idPlayer]], sizeof(figure_t) );
	
	game->pFig->parent = (void *)game;
	game->pNextFig->parent = (void *)game;
	
	//shrink pool
	//
	num = g_figPoolSize;
	for( i=0; i<k_system.cPlayers; i++ ) {
		num = min( num, g_figID[i] );
	}
	if( num >= 5 ) {
		shrinkFigPool( num );
	}

	return game->pFig;
}


/*
=====================
	enlargeFigPool
=====================
*/
void enlargeFigPool( int num ) {
	int i = 0;
	int state = 0;
	int type = 0;
	
	if( num <= 0 ) {
		return;
	}

	//increase figure pool size
	//
	g_figPoolSize += num;

	if( g_figPool != NULL ) { //already allocated
		g_figPool = N_Realloc( g_figPool, sizeof(*g_figPool) * g_figPoolSize );
	}
	else {
		g_figPool = N_Malloc( sizeof(*g_figPool) * g_figPoolSize );
	}

	for( i=g_figPoolSize-num ; i<g_figPoolSize; i++ ) {

		//if demo is playing, get fig data from
		//replay fig pool.
		//
		if( k_system.flags & SF_DEMOPLAY ) {
			if( k_replayFigID >= k_replayFigSize ) {
				N_Trace( TEXT( "Replay figure pool is empty!" ) );
			}
			else {
				state = k_replayFig[k_replayFigID].state;
				type = k_replayFig[k_replayFigID].type;
				k_replayFigID++;
			}
		}
		else {
			state = N_Random( CSTATE );
			type = N_Random( CTYPE );
		}
		
		g_figPool[i].cBlocks = 0;
		g_figPool[i].pBlocks = NULL;
		
		g_figPool[i].prevstate = g_figPool[i].state = state;
		g_figPool[i].type = type;
		
		g_figPool[i].msg = NOMOVE;
		
		g_figPool[i].desintegrated = FALSE;
		
		g_figPool[i].pos.x = g_figPool[i].prevpos.x = STARTPOS;
		g_figPool[i].pos.y = g_figPool[i].prevpos.y = 0;
	}
	
	//in case we are recording the game,
	//copy figures to replay fig pool
	//
	if( k_system.flags & SF_RECORDING ) {
	
		//enlarge fig pool
		//
		k_replayFigSize += num;
		
		if( k_replayFig == NULL ) {
			k_replayFig = N_Malloc( k_replayFigSize*sizeof(figure_t) );
		}
		else {
			k_replayFig = N_Realloc( k_replayFig, k_replayFigSize*sizeof(figure_t) );
		}
		
		//copy figures
		//
		N_Memcpy( &(k_replayFig[k_replayFigSize-num]), &(g_figPool[g_figPoolSize-num]), num*sizeof(figure_t) );
	}

}


/*
=====================
	shrinkFigPool
=====================
*/
void shrinkFigPool( int num ) {
	int i;
	
	if( num <= 0 ) {
		return;
	}
	
	g_figPoolSize -= num;
	
 	for( i=0; i<k_system.cPlayers; i++ ) {
		g_figID[i] -= num;
	}

	N_Memcpy( g_figPool, &(g_figPool[num]), g_figPoolSize*sizeof(figure_t) );
	g_figPool = N_Realloc( g_figPool, g_figPoolSize*sizeof(figure_t) );
}


/*
=====================
	destroyFigPool
=====================
*/
void destroyFigPool() {
	g_figPoolSize = 0;
	N_Free( g_figPool );
	g_figPool = NULL;
}


/*
=====================
	initFigPool
=====================
*/
void initFigPool() {
	int i;

	g_figPool = NULL;
	g_figPoolSize = 0;
	
	for( i=0; i<k_system.cPlayers; i++ ) {
		g_figID[i] = 0;
	}
}


/*
=====================
	tryFig
	-----------------
	try to place a figure on the action space
=====================
*/
BOOL tryFig( const figure_t *fig, const int *as ) {
	int i,j;

	if( (fig!=NULL) && (fig->type<CTYPE) && (fig->state<CSTATE) && (as!=NULL ) ) {
		for( i=0; i<CYFIG; i++ ) {
			for( j=0; j<CXFIG; j++ ) {
				//check if space cell is not already occupied
				if( (SPACE_CELL( as, i+fig->pos.y, j+fig->pos.x ) == MAPCELL_BLOCK )
						&& (kFigRes[fig->type][i][j+fig->state*CSTATE] == MAPCELL_BLOCK) ) {
					return FALSE;
				}
			}
		}
	}

	return TRUE;
}


/*
=====================
	tryBlock
	-----------------
	try to place a block on game spaces
=====================
*/
BOOL tryBlock( const block_t *pBlock, const int *space ) {

	//check arguments validity
	if( (pBlock == NULL) || (space == NULL) ) {
		return FALSE;
	}
	
	//check if space cell is not already occupied
	if( SPACE_CELL( space, pBlock->pos.y, pBlock->pos.x ) == MAPCELL_BLOCK ) {
		return FALSE;
	}
	
	return TRUE;
}


/*
=====================
	doFigAction
=====================
*/
BOOL doFigAction( figure_t * fig, timeSliceActions_t action ) {

	switch( action ) {
		//move figure downwards
		case TIME_FIG_YMOVE:
			moveFigDown( fig );
			break;
		
		default:
			break;
	}
	
	return TRUE;
}


/*
=====================
	doBlockAction
=====================
*/
void doBlockAction( figure_t *pFig, timeSliceActions_t action ) {
	//because of game algorithm, figure/block
	//can move only in one direction at a time,
	//so we compute each dimension separately
	//
	switch( action ) {
		case TIME_BLOCK_YMOVE:
			computeBlockPosY( pFig );
			break;
		
		case TIME_BLOCK_XMOVE:
			computeBlockPosX( pFig );
			break;
		
		default:
			break;
	}
}


/*
=====================
	computeBlockPosY
=====================
*/
void computeBlockPosY( figure_t *pFig ) {
	int i;
	block_t *pBlock;
	
	//check arguments validity
	if( pFig == NULL ) {
		return;
	}

	//process each block	
	for( i=0; i<pFig->cBlocks; i++ ) {
		pBlock = &( pFig->pBlocks[i] );
		
		//save block position
		pBlock->prevpos.y = pBlock->pos.y;

		//move block
		pBlock->pos.y += pBlock->yvel;
		
		//set movement direction msg
		if( pBlock->pos.y > pBlock->prevpos.y ) {
			pBlock->msg = DOWNMOVE;
		}
		else {
			pBlock->msg = UPMOVE;
		}

		//after yvel is >= 1 we do not accelerate anymore
		if( pBlock->yvel < 1) {
			pBlock->yvel += pBlock->accel;
		}
	}
}


/*
=====================
	computeBlockPosX
=====================
*/
void computeBlockPosX( figure_t *pFig ) {
	int i;
	block_t *pBlock;
	
	//check arguments validity
	if( pFig == NULL ) {
		return;
	}
	
	//process each block
	for( i=0; i<pFig->cBlocks; i++ ) {
		pBlock = &( pFig->pBlocks[i] );
		
		//save block position
		pBlock->prevpos.x = pBlock->pos.x;
		
		//move block
		pBlock->pos.x += pBlock->xvel;
		
		//set movement msg
		pBlock->msg = XMOVE;
	}
}


/*
=====================
	doGameAction
=====================
*/
void doGameAction( game_t *game, timeSliceActions_t action ) {

	switch( action ) {
		case TIME_GAME_WAIT:
			game->flags &= ~( SF_GAMEWAIT | SF_NOMOVE );
			break;
		
		case TIME_TRIGGER_RESPAWN:
			triggerRespawn( game );
			break;
		
		case TIME_TELEPORT:
			game->flags &= ~( SF_NOMOVE | SF_GAMEWAIT );
			game->pFig->guid[0] = pushTimeSlice( TIME_FIG_YMOVE, TIME_FIG_YMOVE_INTERVAL,
				0, game->pFig, FIGURE, FALSE );
			break;
		
		case TIME_GAME_OVER:
			gameOverEnd();
			break;
		
		default:
			break;
	}
}


/*
=====================
	crushFig
=====================
*/
void crushFig( game_t *game ) {

	//check game validity
	if( game == NULL ) {
		return;
	}

	//crush figure in case current game, figure and movement are valid
	if( (k_system.gameType > GNO) && (game->pFig!=NULL) && !(game->flags & SF_FIGCRUSHED) ) {
		popTimeSlice( game->pFig->guid[0] );
		game->pFig->guid[0] = pushTimeSlice( TIME_FIG_YMOVE, TIME_FIG_CRUSH_INTERVAL,
			0, game->pFig, FIGURE, TRUE );
		game->flags |= SF_FIGCRUSHED;
	}
}


/*
=====================
	crushFigFast
	-----------------
	this one is used when testing bots
=====================
*/
void crushFigFast( game_t *game ) {

	//check game validity
	if( game == NULL ) {
		return;
	}

	//crush figure in case game, figure and movement are valid
	if( (k_system.gameType > GNO) && (game->pFig!=NULL) && !(game->flags & SF_FIGCRUSHED) ) {
		popTimeSlice( game->pFig->guid[0] );
		game->pFig->guid[0] = pushTimeSlice( TIME_FIG_YMOVE, TIME_FIG_CRUSH_FAST_INTERVAL,
			0, game->pFig, FIGURE, TRUE );
		game->flags |= SF_FIGCRUSHED;
	}
}


/*
=====================
	getFigWidth
=====================
*/
void getFigWidth( const figure_t *fig, int *left, int *right ) {
	int i, j;
	int l, r;
	
	//check arguments validity
	if( !fig ) {
		return;
	}
	
	l = CXFIG-1;
	r = 0;

	//determine left and right extremes
	for( i=0; i<CYFIG; i++ ) {
		for( j=0; j<CXFIG; j++ ) {
			if( kFigRes[fig->type][i][fig->state*CSTATE+j] == 1 ) {
				//compare coordinates
				if( j < l ) { l = j; }
				if( j > r ) { r = j; }
			}
		}
	}

	//we can have NULL arguments
	if( left ) { *left = l; }
	if( right ) {*right = r; }
}


/*
=====================
	getFigHeight
=====================
*/
void getFigHeight( const figure_t *fig, int *top, int *bottom ) {
	int i,j;
	int t, b;
	
	//check arguments validity
	if( !fig || !top || !bottom ) {
		return;
	}
	
	t = CYFIG-1;
	b = 0;
	
	//determine top and bottom extremes
	for( i=0; i<CYFIG; i++ ) {
		for( j=0; j<CXFIG; j++ ) {
			if( kFigRes[fig->type][i][fig->state*CSTATE+j] == 1 ) {
				//compare coordinates
				if( i < t ) { t = i; }
				if( i > b ) { b = i; }
			}
		}
	}
	
	//we can have NULL arguments
	if( top ) { *top = t; }
	if( bottom ) { *bottom = b; }
}


/*
=====================
	desintegrateFig
=====================
*/
BOOL desintegrateFig( game_t *game ) {
	int num;
	int i;
	figure_t *pFig;
	point_t pt;
	block_t *pBlocks;
	blockType_t type;
	BOOL success;

	//check arguments
	if( !game || !game->pFig) {
		return FALSE;
	}

	//get number of figure blocks
	pFig = game->pFig;
	num = getFigBlockNum( pFig );

	//allocate memory
	pBlocks =
	pFig->pBlocks = N_Malloc( num*sizeof(block_t) );
	pFig->cBlocks = num;

	__try {
		success = FALSE;

		//process blocks
		for( i=0; i<num; i++ ) {
			//get absolute block position
			if( getFigBlockPos( pFig, i, &pt ) ) {
				pBlocks[i].prevpos.x = pBlocks[i].pos.x = pt.x;
				pBlocks[i].prevpos.y = pBlocks[i].pos.y = pt.y;
			} else {
				__leave;
			}
			
			//get block type
			if( getFigBlockType( pFig, i, &type ) ) {
				pBlocks[i].type = type;
			} else {
				__leave;
			}
			
			//setup block
			pBlocks[i].landed = FALSE;
			pBlocks[i].msg = NOMOVE;
		}
		
		success = TRUE;
	}
	__finally {
	}

	if( success ) {
		pFig->desintegrated = TRUE;
	}
	//clean-up
	else {
		if( pFig->pBlocks ) {
			N_Free( pFig->pBlocks );
			pFig->pBlocks = NULL;
			pFig->cBlocks = 0;
		}
	}
	
	return success;
}


/*
=====================
	getFigBlockNum
=====================
*/
int getFigBlockNum( const figure_t *pFig ) {
	int num=0;
	int i, j;
	
	//check arguments
	if( !pFig ) {
		return 0;
	}

	for( i=0; i<CYFIG; i++ ) {
		for( j=0; j<CXFIG; j++ ) {
			if( kFigRes[pFig->type][i][pFig->state*CSTATE+j] == 1 ) {
				num++;
			}
		}
	}
	
	return num;
}


/*
=====================
	getFigBlockPos
=====================
*/
BOOL getFigBlockPos( const figure_t *pFig, int id, point_t *ppt ) {
	int i, j;
	int num=0;

	//check arguments validity
	if( !pFig || !ppt ) {
		return FALSE;
	}
	
	for( i=0; i<CYFIG; i++ ) {
		for( j=0; j<CXFIG; j++ ) {
			if( kFigRes[pFig->type][i][pFig->state*CSTATE+j] == 1 ) {
				//we found our block
				if( id == num ) {
					ppt->x = pFig->pos.x + j;
					ppt->y = pFig->pos.y + i;
					return TRUE;
				}
				num++;
			}
		}
	}
	
	return FALSE;
}


/*
=====================
	getFigBlockType
=====================
*/
BOOL getFigBlockType( const figure_t *pFig, int id, blockType_t *pBlockType ) {
	int num=0;
	int i, j;

	//check arguments
	if( !pFig || !pBlockType ) {
		return FALSE;
	}

	for( i=0; i<CYFIG; i++ ) {
		for( j=0; j<CXFIG; j++ ) {
			if( kFigRes[pFig->type][i][pFig->state*CSTATE+j] == 1 ) {
				//we found our block
				if( id == num ) {
					*pBlockType = kBlockRes[pFig->type][i][pFig->state*CSTATE+j];
					return TRUE;
				}
				num++;
			}
		}	
	}

	return FALSE;
}


/*
=====================
	moveFigLeft
=====================
*/
void moveFigLeft( figure_t *pFig ) {
	if( pFig ) {
		pFig->prevpos.x = pFig->pos.x;
		pFig->pos.x--;
		pFig->msg = XMOVE;
	}
}


/*
=====================
	moveFigRight
=====================
*/
void moveFigRight( figure_t *pFig ) {
	if( pFig ) {
		pFig->prevpos.x = pFig->pos.x;
		pFig->pos.x++;
		pFig->msg = XMOVE;
	}
}


/*
=====================
	moveFigDown
=====================
*/
void moveFigDown( figure_t *pFig ) {
	if( pFig ) {
		pFig->prevpos.y = pFig->pos.y;
		pFig->pos.y++;
		pFig->msg = YMOVE;
	}
}


/*
=====================
	rotateFig
=====================
*/
void rotateFig( figure_t *pFig ) {
	if( pFig ) {
		pFig->prevstate = pFig->state;
		if( pFig->state < (CSTATE - 1) ) {
			pFig->state++;
		}
		else {
			pFig->state = 0;
		}
		pFig->msg = ROTATE;
	}
}


/*
=====================
	getASBlockNum
=====================
*/
int getASBlockNum( const game_t *game ) {
	int i;
	int j;
	int num=0;

	//check arguments
	if( !game ) {
		return 0;
	}

	//calculate number of block in Action Space
	for( i=0; i<CYSPACE; i++ ) {
		for( j=0; j<CXSPACE; j++ ) {
			if( SPACE_CELL( game->AS, i, j ) == MAPCELL_BLOCK ) {
				num++;
			}
		}
	}
	
	return num;
}


/*
=====================
	addGroundBlocks
=====================
*/
void addGroundBlocks( game_t *game ) {
	int i;
	int j;
	int grnd_pos;
	int var;
	int bFlag;
	int line;
	
	//check arguments
	if( game == NULL ) {
		return;
	}
	
	//init toxicBlocks array
	for( i=0; i<CXSPACE; i++ ) {
		game->toxicBlocks[i] = -1;
	}
	
	//we will have random blocks; but this
	//randomness must be repeated in demos,
	//so we somehow control the output of
	//random function using this seed
	//
	var = game->score + getASBlockNum( game ) + game->pFig->pos.x;

	//add toxix blocks
	//
	for( i=0; i<CXSPACE; i++ ) {
		var = RANDOM_CONST * var + 1;
		//random locations
		//
		if( (((int)(var >> 16) & 0x7FFF) % 3) < 2 ) {
			//find a proper ground position
			//
			grnd_pos = 0;
			bFlag = FALSE;
			while( grnd_pos <= CYSPACE ) {
				if( SPACE_CELL( game->AS, grnd_pos, i ) == MAPCELL_BLOCK ) {
					//
					if( (SPACE_CELL( game->AS, grnd_pos-1, i ) == MAPCELL_EMPTY) &&
						(SPACE_CELL( game->FS, grnd_pos-1, i ) == MAPCELL_EMPTY) &&
						(SPACE_CELL( game->FS, grnd_pos-2, i ) == MAPCELL_EMPTY) ) {
							bFlag = TRUE;
					}
					break;
				}
				grnd_pos++;
			}

			//add block
			if( bFlag ) {

				SPACE_CELL( game->AS, grnd_pos-1, i ) = MAPCELL_BLOCK;
				game->toxicBlocks[i] = grnd_pos-1;
				
				//if a line is completed by placing this block,
				//we will revert the changes
				//

				line = 0;
				for( j=0; j<CXSPACE; j++ ) {
					if( SPACE_CELL( game->AS, grnd_pos-1, j ) == MAPCELL_BLOCK ) {
						line++;
					}
				}
				
				//revert changes
				if( line == CXSPACE ) {
					SPACE_CELL( game->AS, grnd_pos-1, i ) = MAPCELL_EMPTY;
					game->toxicBlocks[i] = -1;
				}
			}
		}
	}
}


/*
=====================
	flushSpace
	-----------------
	flushes the action/figure space only
	in the "writable" section
=====================
*/
void flushSpace( int *space ) {
	int i;
	
	if( space ) {
		for( i=0; i<CYSPACE; i++ ) {
			N_Memset( &(SPACE_CELL( space, i, 0 )), 0, sizeof(int)*CXSPACE );
		}
	}
}


/*
=====================
	writeSpace
=====================
*/
void writeSpace( const figure_t *pFig, int *space ) {
	int i,j;
	
	//check arguments
	if( !pFig || !space ) {
		return;
	}
	
	//copy figure to space
	for( i=0; i<CYFIG; i++ ) {
		for( j=0; j<CXFIG; j++ ) {
			if( kFigRes[pFig->type][i][j+pFig->state*CSTATE] == 1 ) {
				SPACE_CELL( space, pFig->pos.y+i, pFig->pos.x+j ) = MAPCELL_BLOCK;
			}
		}
	}
}


/*
=====================
	createSpace
	-----------------
	Allocates memory for a game space and builds the bounding
	box around it.
	It should be free-ed afterwards with freeSpace()
=====================
*/
int* createSpace() {
	int *space = NULL;
	int i,j;
	
	space = N_Malloc( sizeof(int)*SPACESIZE );

	for( i=0; i<CYSPACE+ASWALL*2; i++ ) {
		for( j=0; j<CXSPACE+ASWALL*2; j++ )
			//empty cell
			if( (i>=ASWALL) && (j>=ASWALL) && (j<CXSPACE+ASWALL) && (i<CYSPACE+ASWALL) ) {
				SPACE_CELL( space, i-ASWALL, j-ASWALL ) = MAPCELL_EMPTY;
			}
			//bounding box
			else {
				SPACE_CELL( space, i-ASWALL, j-ASWALL ) = MAPCELL_BLOCK;
			}
	}

	return space;
}


/*
=====================
	freeSpace
=====================
*/
void freeSpace( int *space ) {
	if( space ) {
		N_Free( space );
	}
}


/*
=====================
	checkLines
	-----------------
	Scans the action space from top to bottom
	for complete lines.
=====================
*/
void checkLines( game_t *game ) {
	int *space = NULL;
	int i,j;
	int sum;
	int index;
	
	//check arguments
	if( !game || !game->AS ) {
		return;
	}
	
	//init variables
	space = game->AS;
	index=0;
	N_Memset( game->abLines, 0, sizeof(BOOL)*CYSPACE );

	//mark completed lines
	for( i=0; i<CYSPACE; i++ ) {
		sum = 0;
		for( j=0; j<CXSPACE; j++ ) {
			if( SPACE_CELL( space, i, j ) == MAPCELL_BLOCK ) {
				sum++;
			}
		}
		
		//line is completed
		if( sum == CXSPACE ) {
			game->abLines[i] = TRUE;
			game->score++;
			index++;
		}
	}

	game->cLines = index;
}


/*
=====================
	removeLines
=====================
*/
void removeLines( game_t *game ) {
	int i, j, k;
	int *space = NULL;

	//check arguments
	if( !game || !game->AS  ) {
		return;
	}
	
	space = game->AS;

	for( i=0; i<CYSPACE; i++ ) {
		//check if line is marked for removal
		if( game->abLines[i] ) {
			for( j=i; j>0; j-- ) {
				for( k=0; k<CXSPACE; k++ ) {
					//copy the upper line into the current one
					SPACE_CELL( space, j, k ) = SPACE_CELL( space, j-1, k );
				}
			}
			//fill the top-most line with empty blocks
			//and move toxic blocks one step lower
			//
			for( j=0; j<CXSPACE; j++ ) {
				SPACE_CELL( space, 0, j ) = 0;
				//move down toxic blocks
				if( (game->toxicBlocks[j] >= 0) && (game->toxicBlocks[j] < CYSPACE-1) ) {
					game->toxicBlocks[j]++;
				}
			}
		}
	}
	
	//reset array of marked lines
	N_Memset( game->abLines, 0, sizeof( BOOL ) * CYSPACE );
}


/*
=====================
	decodeIMG
	-----------------
	Decodes Netrix internal image format
	to BMP format.
	The color order is: BGR
=====================
*/
void decodeIMG( const DWORD *data, DWORD **pic, DWORD *width, DWORD *height ) {
	int s;
	int size;
	DWORD c;
	int i;
	int j;
	
	//check arguments validity
	if( (data==NULL) || (pic==NULL)
		|| (width==NULL) || (height==NULL) ) {
		
		return;
	}
	
	size = data[0];
	
	*width = data[1];
	*height = data[2];

	s = (*height) * (*width);
	*pic = N_Malloc( s * sizeof(DWORD) );
	
	i=3;
	j=0;
	while( i<size ) {
		c = data[i++];
		while( c-->0 ) {
			(*pic)[j++] = data[i];
		}
		i++;
	}
}


/*
=====================
	pause
=====================
*/
void pause() {
	if( k_system.gameType > GNO ) {
		if( k_system.pause ) {
			unpause();
		}
		else {
			k_system.pause = TRUE;
			k_system.flags |= SF_PAUSE;
		}
	}
}


/*
=====================
	unpause
=====================
*/
void unpause() {
	if( k_system.pause ) {
		k_system.pause = FALSE;
		k_system.flags &= ~SF_PAUSE;
	}
}