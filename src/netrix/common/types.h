
//common game types
//

#ifndef __GTYPES_H__
#define __GTYPES_H__

#include "../compile.h"
#include <windows.h>

//flag options
//
#define SF_GAMELOST		(1<<0)	//the game was lost
#define SF_FIGLANDED	(1<<1)	//figure landed
#define SF_FLAGCAPTURED	(1<<2)	//flag captured
#define SF_FIGCRUSHED	(1<<3)	//figure is crushing
#define SF_TIMEUPDATED	(1<<4)	//time slices updated
#define SF_SKIPSEQ		(1<<5)	//skip seqProcGame
#define SF_RECORDING	(1<<6)	//game is beeing recorded
#define SF_RECORDGAME	(1<<7)	//game should be recorded
#define SF_MAPUSED		(1<<8)	//map was used
#define SF_ADDBLOCKS	(1<<9)	//add toxic blocks
#define SF_GAMEWAIT		(1<<10) //no action is allowed
#define SF_SKIPTRIG		(1<<11)	//skip trigger checking
#define SF_NOMOVE		(1<<12) //no movement is allowed
#define SF_DEMOPLAY		(1<<13) //a demo is played
#define SF_BOTGAME		(1<<14)	//game is controled by a bot
#define SF_BOTFAST		(1<<15)	//fast bot movement
#define SF_GAMEOVER		(1<<16)	//game is over
#define SF_PAUSE		(1<<17) //game is paused

//game outcomes
//
#define GAME_OVER	1
#define GAME_LOOSE	2
#define GAME_WIN	3

//gameType_t
//
typedef enum gameType_e { GNO, GSINGLE, GVS, GBOT, GBOTSINGLE, GNETVS, GNETBOT } gameType_t;

//msg_t
//
typedef enum msg_e { NOMOVE, YMOVE, YMOVEUP, XMOVE, ROTATE, CRUSH, LEFTMOVE, RIGHTMOVE, DOWNMOVE, UPMOVE } msg_t;

//place_t
//
typedef enum place_e { NEUTRAL, LEFTGAME, RIGHTGAME } place_t;

//block types depending on
//position in figure
//
typedef enum blockType_e {
	B_T,	//TOP
	B_TI,	//TOP_INNER
	B_TL,	//TOP_LEFT
	B_TR,	//TOP_RIGHT
	B_L,	//LEFT
	B_R,	//RIGHT
	B_B,	//BOTTOM
	B_BI,	//BOTTOM_INNER
	B_BL,	//BOTTOM_LEFT
	B_BR,	//BOTTOM_RIGHT
	B_I,	//INNER
} blockType_t;


//point_t
//
typedef struct point_s {
	int x;
	int y;
} point_t;


//block_t
//
typedef struct block_s {
	point_t pos;
	point_t prevpos;
	int		xvel;		//x velocity
	int		yvel;		//y velocity
	msg_t	msg;		//movement message
	BOOL	landed;		//TRUE if landed, FALSE otherwise
	int		accel;		//figure acceleration
	blockType_t type;	//block type
} block_t;


//figure_t
//
typedef struct figure_s {
	int			type;		//figure type
	int			state;		//figure rotation position
	int			prevstate;	//previous rotation position

	point_t		pos;		//move point
	point_t		prevpos;	//previous move location

	block_t		*pBlocks;	//array of blocks (used in desintegration animation)
	int			cBlocks;	//block number
	
	msg_t		msg;		//internal state msg
	
	BOOL		desintegrated; //TRUE if figure is desintegrated, FALSE otherwise
	
	void		*parent;	//parent game
	
	DWORD		guid[3];	//scheduler GUID (0-Figure, 1-Block Y, 2-Block X )

} figure_t;


//pathEntry_t
//
typedef struct pathEntry_s {
	TCHAR szPath[MAX_PATH];	//path to resource
} pathEntry_t;

#endif
