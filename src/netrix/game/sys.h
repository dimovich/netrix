
#ifndef __SYS_H__
#define __SYS_H__

#include "../compile.h"
#include <windows.h>

#include "game.h"
#include "../common/map.h"
#include "../bot/bot.h"
#include "../graphics/graphics.h"


#ifdef __cplusplus
extern "C" {
#endif


typedef struct system_s {

	game_t		*pLeftGame;		//left game
	game_t		*pRightGame;	//right side game (in multiplayer game type)

	gameType_t	gameType;		//game type
	BOOL		pause;			//pause Flag
	unsigned long	flags;		//system flags
	
	pathEntry_t	*pPaths;		//all paths (packs, single files, etc)
	int			cPaths;			//path count
	
	mapEntry_t	*pMaps;			// Maps
	int			cMaps;			// Map count
	int			idMap;			// current Map

	botEntry_t	*pBots;			//Bots
	int			cBots;			//Bot count
	int			idBotLeft;		//current bot for the left game
	int			idBotRight;		//current bot for the right game

	HWND		hwnd;			// Main window handle
	
	HWND		hwndLeft;		// Left AS (Action Space)
	HWND		hwndRight;		// Right AS ( Bot/Multiplayer/vs ) handle

	HWND		hWndScoreLeft;	//Left score
	HWND		hWndScoreRight;	//Right score
	
	HWND		hwndLeftN;		//Left next figure space
	HWND		hwndRightN;		//Right next figure space

	//Skin
	
	HRGN		hSkinRgnLeft;		//left skin region
	HBITMAP		hSkinBitmapLeft;	//left skin bitmap
	int			cxSkinLeft;			//left skin x dimension
	int			cySkinLeft;			//left skin y dimension
	
	HRGN		hSkinRgnRight;		//right skin region
	HBITMAP		hSkinBitmapRight;	//right skin bitmap
	int			cxSkinRight;		//right skin x dimension
	int			cySkinRight;		//right skin y dimension

	int			cPlayers;			//nbr. of players
	
	TCHAR		szStartDir[MAX_PATH];	//game directory

	DWORD		dwTime;			//current time
	DWORD		dwAccumTime;	//accumulated time

} system_t;

extern system_t k_system;

//ignites all engines
BOOL initSystem();

//shuts down all engines
BOOL destroySystem();

//scans for resources located in base directory
//
//maps, bot scripts, pack files, etc.
void loadResources();

//rescans resources
void reloadResources();

static mapEntry_t * loadMapResource( TCHAR *szPath );
static botEntry_t * loadBotResource( TCHAR *szPath );
static void loadPackResource( TCHAR *szPath );

#ifdef DEBUG_COMPILE
	//output memory leaks
	void outputDebugInfo( TCHAR * Path);
#endif


#ifdef __cplusplus
}
#endif


#endif