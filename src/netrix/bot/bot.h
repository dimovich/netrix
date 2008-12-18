
#ifndef __BOT_H__
#define __BOT_H__


#include "../../nxc/nxc.h"
#include "../game/game.h"

#ifdef __cplusplus
extern "C" {
#endif


//bot difficulty levels
//
#define BOT_EASY		1	//I Can Win
#define BOT_NORMAL		2	//Hurt Me Plenty
#define BOT_HARD		3	//Hardcore
#define BOT_NIGHTMARE	4	//Nightmare

//movement times
//
#define DELTA_EASY		300
#define DELTA_NORMAL	125
#define DELTA_HARD		75
#define DELTA_NIGHTMARE	50


//bot_t
//
typedef struct bot_s {
	TCHAR		name[NAMESIZE+1];	//bot name
	program_t	program;		//bot program
	int			difficulty;		//bot difficulty
} bot_t;


//botEntry_t
//
typedef struct botEntry_s {
	fileEntry_t		fe;		//file entry
	bot_t			bot;	//bot
	BOOL			loaded;	//TRUE if loaded; FALSE otherwise
} botEntry_t;


//init bot system
//
BOOL botInit();

//destroy bot system
//
void botDestroy();

//load bot
//
BOOL botLoad( game_t *game, botEntry_t *be );

//unload bot
//
void botUnload( botEntry_t *be );

//update bot state (run Update() function)
//
void botUpdate( game_t *game );

//interpolate start-end positions
//and create movement slices
//
void botInterpolate();

//move bot figure
//
void botFrame( game_t *game );

//fast interpolation
//
void botInterpolateFast( game_t *game, figure_t *start, figure_t *end );

#ifdef __cplusplus
}
#endif


#endif