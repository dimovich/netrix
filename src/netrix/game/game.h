
#ifndef __GAME_H__
#define __GAME_H__

#include "../compile.h"
#include <windows.h>

#include "../../netrixlib/netrixlib.h"

#include "../common/const.h"
#include "../common/types.h"
#include "../graphics/effect.h"

#ifdef __cplusplus
extern "C" {
#endif


//botFrames_t
//
typedef struct botFrames_s {
	int frames[32][2];	//this should be enough for everyone ([0]-time, [1]-msg)
	int size;
	int idx;
} botFrames_t;


//gameFileHeader_t
//
typedef struct gameFileHeader_s {
	unsigned long	header;		//GAM1
	unsigned long	crc;		//file CRC
	gameType_t		gameType;	//game type
} gameFileHeader_t;


//game_t
//
typedef struct game_s {
	int			*AS;			//action space
	int			*FS;			//figure space
	int			*TS;			//trigger space
	
	DWORD		flags;			//game flags
	
	figure_t	*pFig;			//figure
	figure_t	*pNextFig;		//next figure

	int			score;			//game score
	int			cLines;			//lines previously deleted
	BOOL		abLines[CYSPACE];//lines previously deleted
	BOOL		newBlock;		//TRUE if a new block is needed
	int			toxicBlocks[CXSPACE];	//blocks added in multiplayer game
	
	int			idPlayer;		//player ID, used in figure generation
	place_t		place;			//LEFTGAME xor RIGHTGAME

	effectList_t *effectList;	//effect list
	botFrames_t	botFrames;		//bot movement frames
} game_t;


//sets common init game values
void initGame( game_t * );

//unsets the common game values
void un_initGame( game_t * );

//init left game
//
void initLeftGame();

//destroy left game
//
void destroyLeftGame();

//init right game
//
void initRightGame();

//destroy right game
//
void destroyRightGame();

//ends whatever game it is
void endAllGames();

//single game
BOOL singleGameStart( );
BOOL singleGameEnd( );

//vs. game
//
BOOL vsGameStart();
BOOL vsGameEnd();

//network game
//
BOOL networkGameStart();
BOOL networkGameEnd();

//bot game
//
BOOL botGameStart();
BOOL botGameEnd();

//single bot game
//
BOOL botSingleGameStart();
BOOL botSingleGameEnd();

//bot over network game
//
BOOL botonetGameStart();
BOOL botonetGameEnd();

//start a saved game
//
BOOL startSavedGame();

//save current game
//
BOOL gameSave();

//loads a game from the given path
//
BOOL gameLoad( TCHAR * );

//get saved game type
//
BOOL getSavedGameType( TCHAR *szFile, gameType_t *gameType );

//start game over event
//
void gameOverStart();

//end game over event
//
void gameOverEnd();

#ifdef __cplusplus
}
#endif

#endif