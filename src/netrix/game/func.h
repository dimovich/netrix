
#ifndef __FUNC_H__
#define __FUNC_H__

#include "../compile.h"
#include <windows.h>

#include "../../netrixlib/netrixlib.h"

#include "../common/types.h"
#include "game.h"
#include "scheduler.h"


#ifdef __cplusplus
extern "C" {
#endif


//get a cell from the space (AS, FS, TS, etc... )
//
#define SPACE_CELL(a,i,j) ((a)[((i)+ASWALL)*(CXSPACE+2*ASWALL) + (ASWALL+(j))])


// generates a figure
figure_t * genFig( game_t * );

//init figure pool
void initFigPool();

//destroy figure pool
void destroyFigPool();

//enlarge figure pool
void enlargeFigPool( int );

//shrink figure pool
void shrinkFigPool( int );

//try to place a figure on game spaces
BOOL tryFig( const figure_t *, const int * );

//try to place a block on game spaces
BOOL tryBlock( const block_t *, const int * );

//movement functions
//
void moveFigLeft( figure_t * );
void moveFigRight( figure_t * );
void moveFigDown( figure_t * );
void rotateFig( figure_t * );

//figure functions
//
void getFigWidth( const figure_t *pFig, int *left, int *right );
void getFigHeight( const figure_t *pFig, int *top, int *bottom );

int getASBlockNum( const game_t *game );

BOOL desintegrateFig( game_t *game );

int getFigBlockNum( const figure_t *pFig );

//get block position in figure
BOOL getFigBlockPos( const figure_t *pFig, int id, point_t *ppt );

//get block type
BOOL getFigBlockType( const figure_t *pFig, int id, blockType_t *pBlockType );

//perform a block action
void doBlockAction( figure_t *, timeSliceActions_t );

//compute block movement
void computeBlockPosY( figure_t * );
void computeBlockPosX( figure_t * );

//peform figure action
BOOL doFigAction( figure_t *, timeSliceActions_t );

//peform game action
void doGameAction( game_t *, timeSliceActions_t );

//when in multiplayer, this will add
//random ground blocks (on top of the ground)
void addGroundBlocks( game_t * );

//determines the number of blocks in a figure;
//used in trigger animation
int getBlockNum( const figure_t * );

//get block position in Action Space;
//used in trigger animation
BOOL posInAS( int, const figure_t *, point_t * );

//fast Y dimension figure movement
void crushFig( game_t *game );

//faster Y dimension figure movement
void crushFigFast( game_t *game );

//checks the action space for complete lines
void checkLines( game_t *game );

//removes completed lines
void removeLines( game_t *game );

//empties the action/figure space
void flushSpace( int *space );

//writes the figure data to action/figure space
void writeSpace( const figure_t *pFig, int *space );

//creates and initializes the action/figure space
int* createSpace();

// frees the alocated action/figure space
void freeSpace( int* );

//pause game
void pause();

//unpause game
void unpause();

//decodes Netrix nativ image files
void decodeIMG( const DWORD *, DWORD **, DWORD *, DWORD * );


#ifdef __cplusplus
}
#endif


#endif