
#ifndef __TRIGGER_H__
#define __TRIGGER_H__


#ifdef __cplusplus
extern "C" {
#endif


#define TRIGGER_NO					(-1)

#define TRIGGER_BOMB				MAPCELL_BOMB
#define TRIGGER_TELEPORT			MAPCELL_TELEPORT
#define TRIGGER_FLAG				MAPCELL_FLAG
#define TRIGGER_RESPAWN_BOMB		5
#define TRIGGER_RESPAWN_TELEPORT	6
#define TRIGGER_RESPAWN_FLAG		7

#define BLOCK_XVEL	(1)
#define BLOCK_YVEL	(1)

//trigger_t
typedef struct {
	int type;
	int x, y;
} trigger_t;


//checks if any triggers should be executed
void checkTriggers( game_t * );

//checks for figure/trigger intersection
BOOL intersectTrigFig( game_t *, trigger_t * );

//executes a specific trigger
void executeTrigger( game_t *, trigger_t * );

//explodes a figure
BOOL explodeFig( game_t * );

//teleports the fig to random place
BOOL teleportFig( game_t * );

//respawn triggers
void triggerRespawn( game_t * );


#ifdef __cplusplus
}
#endif


#endif