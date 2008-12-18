
//Key bindings
//

#undef __N_FILE__
#define __N_FILE__ TEXT( "keys.c" )

#include "../compile.h"
#include <windows.h>

#include "../../netrixlib/netrixlib.h"
#include "../game/game.h"
#include "../game/sys.h"
#include "../game/replay.h"
#include "../game/func.h"
#include "keys.h"


typedef void (*keyFunc_t)( int );


typedef struct keyName_s {
	TCHAR	*name;	//name
	int		vk;		//virtual key code
} keyName_t;


typedef struct keyFuncName_s {
	TCHAR		*name;
	keyFunc_t	func;
} keyFuncName_t;


typedef struct keyBind_s {
	keyFunc_t	func;		//key function
	int			player;		//player id (0-single, 1-player1, 2-player2)
	struct keyBind_s *next;	//next key binding for the same vk
} keyBind_t;

//key bindings
static keyBind_t *g_keyBindings[256] = {0};


//key names
static keyName_t g_keyNames[] = {
	{ "BACK",		VK_BACK },
	{ "TAB",		VK_TAB },
	{ "ENTER",		VK_RETURN },
	{ "SHIFT",		VK_SHIFT },
	{ "SPACE",		VK_SPACE },
	{ "PAGEUP",		VK_PRIOR },
	{ "PAGEDOWN",	VK_NEXT },
	{ "END",		VK_END },
	{ "HOME",		VK_HOME },
	{ "LEFT",		VK_LEFT },
	{ "UP",			VK_UP },
	{ "RIGHT",		VK_RIGHT },
	{ "DOWN",		VK_DOWN },
	{ "INSERT",		VK_INSERT },
	{ "DELETE",		VK_DELETE },
	{ NULL,			0 }};


//insert bind into key binding table
static void InsertBind( keyBind_t *keybind, int vk );


//
//	KEY FUNCTIONS
//

void KEY_P1_Left( int flags ) {
	moveFigLeft( k_system.pLeftGame->pFig );
	if( FLAG( k_system.flags, SF_RECORDING ) ) {
		addReplayFrame( LEFTMOVE, LEFTGAME );
	}
}

void KEY_P1_Right( int flags ) {
	moveFigRight( k_system.pLeftGame->pFig );
	if( FLAG( k_system.flags, SF_RECORDING ) ) {
		addReplayFrame( RIGHTMOVE, LEFTGAME );
	}
}

void KEY_P1_Up( int flags ) {
	rotateFig( k_system.pLeftGame->pFig );
	if( FLAG( k_system.flags, SF_RECORDING ) ) {
		addReplayFrame( ROTATE, LEFTGAME );
	}
}

void KEY_P1_Down( int flags ) {
	moveFigDown( k_system.pLeftGame->pFig );
	if( FLAG( k_system.flags, SF_RECORDING ) ) {
		addReplayFrame( DOWNMOVE, LEFTGAME );
	}
}

void KEY_P1_Crush( int flags ) {
	if( !FLAG( flags, KF_REPEATING ) ) {
		crushFig( k_system.pLeftGame );
		if( FLAG( k_system.flags, SF_RECORDING ) ) {
			addReplayFrame( CRUSH, LEFTGAME );
		}
	}
}

void KEY_P2_Left( int flags ) {
	moveFigLeft( k_system.pRightGame->pFig );
	if( FLAG( k_system.flags, SF_RECORDING ) ) {
		addReplayFrame( LEFTMOVE, RIGHTGAME );
	}
}

void KEY_P2_Right( int flags ) {
	moveFigRight( k_system.pRightGame->pFig );
	if( FLAG( k_system.flags, SF_RECORDING ) ) {
		addReplayFrame( RIGHTMOVE, RIGHTGAME );
	}
}

void KEY_P2_Up( int flags ) {
	rotateFig( k_system.pRightGame->pFig );
	if( FLAG( k_system.flags, SF_RECORDING ) ) {
		addReplayFrame( ROTATE, RIGHTGAME );
	}
}

void KEY_P2_Down( int flags ) {
	moveFigDown( k_system.pRightGame->pFig );
	if( FLAG( k_system.flags, SF_RECORDING ) ) {
		addReplayFrame( DOWNMOVE, RIGHTGAME );
	}
}

void KEY_P2_Crush( int flags ) {
	if( !FLAG( flags, KF_REPEATING ) ) {
		crushFig( k_system.pRightGame );
		if( FLAG( k_system.flags, SF_RECORDING ) ) {
			addReplayFrame( CRUSH, RIGHTGAME );
		}
	}
}


//key functions
static keyFuncName_t g_keyFuncNames[] = {
	//
	//Player-1 (singleplayer)
	//
	{ "LEFT",		KEY_P1_Left },
	{ "RIGHT",		KEY_P1_Right },
	{ "UP",			KEY_P1_Up },
	{ "DOWN",		KEY_P1_Down },
	{ "CRUSH",		KEY_P1_Crush },
	//
	//Player-1 (multiplayer)
	//
	{ "P1_LEFT",	KEY_P1_Left },
	{ "P1_RIGHT",	KEY_P1_Right },
	{ "P1_UP",		KEY_P1_Up },
	{ "P1_DOWN",	KEY_P1_Down },
	{ "P1_CRUSH",	KEY_P1_Crush },
	//
	//Player-2 (multiplayer)
	//
	{ "P2_LEFT",	KEY_P2_Left },
	{ "P2_RIGHT",	KEY_P2_Right },
	{ "P2_UP",		KEY_P2_Up },
	{ "P2_DOWN",	KEY_P2_Down },
	{ "P2_CRUSH",	KEY_P2_Crush },
	{ NULL,			NULL }};


/*
=====================
	BindKey
=====================
*/
BOOL BindKey( TCHAR *key, TCHAR *action ) {
	keyFunc_t func;
	int vk;
	int i;
	int player;
	keyBind_t keybind;

	//check arguments
	if( !key || !action ) {
		return FALSE;
	}
	
	//determine function
	i=0;
	func = NULL;
	player = 0;
	while( g_keyFuncNames[i].name ) {
		if( !N_Strnicmp( g_keyFuncNames[i].name, action, 25 ) ) {

			func = g_keyFuncNames[i].func;
			
			//determine player
			if( action[1] == '2' ) {
				player = 2;
			}
			else if( action[1] == '1' ) {
				player = 1;
			}

			break;
		}
		i++;
	}
	if( !func ) {
		return FALSE;
	}

	keybind.func = func;
	keybind.player = player;

	//single character
	//
	if( key[1] == '\0' ) {
		//windows sends only uppercase vk's
		*key = toupper( *key );
		InsertBind( &keybind, *key );

		//digits are mirrored to numpad
		if( (*key>='0') && (*key<='9') ) {
			InsertBind( &keybind, *key+0x30 );
		}
	}
	//full-mind-blown key
	else {
		//determine key
		i=0;
		vk = 0;
		while( g_keyNames[i].name ) {
			if( !N_Strnicmp( g_keyNames[i].name, key, 25 ) ) {
				vk = g_keyNames[i].vk;
				break;
			}
			i++;
		}
		if( vk == 0 ) {
			return FALSE;
		}

		InsertBind( &keybind, vk );
	}
	
	return TRUE;
}


/*
=====================
	InsertBind
=====================
*/
static void InsertBind( keyBind_t *keybind, int vk ) {
	keyBind_t *tmp;
	
	tmp = g_keyBindings[vk];

	if( !tmp ) {
		g_keyBindings[vk] = N_Malloc( sizeof(keyBind_t) );
		*g_keyBindings[vk] = *keybind;
		g_keyBindings[vk]->next = NULL;
	}
	else {
		while( tmp->next ) {
			tmp = tmp->next;
		}
		tmp->next = N_Malloc( sizeof(keyBind_t) );
		*tmp->next = *keybind;
		tmp->next->next = NULL;
	}
}


/*
=====================
	UnbindKeys
=====================
*/
void UnbindKeys() {
	keyBind_t *tmp1, *tmp2;
	int i;
	
	for( i=0; i<256; i++ ) {
		tmp1 = g_keyBindings[i];
		while( tmp1 ) {
			tmp2 = tmp1;
			tmp1 = tmp1->next;
			N_Free( tmp2 );
		}
		g_keyBindings[i] = NULL;
	}
}


/*
=====================
	KeyProc
=====================
*/
void KeyProc( int vk, int flags ) {
	keyBind_t *tmp;

	tmp = g_keyBindings[vk];

	//
	//GSINGLE, GBOT
	//
	if( (k_system.gameType == GSINGLE) || (k_system.gameType == GBOT) ) {
		while( tmp ) {
			switch( tmp->player ) {
				case 0:
					if( k_system.pLeftGame && !FLAG( k_system.pLeftGame->flags, SF_NOMOVE ) ) {
						((keyFunc_t)tmp->func)( flags );
					}
					break;

				default:
					break;
			}
			tmp = tmp->next;
		}
	}
	//
	//GVS
	//
	else if( k_system.gameType == GVS  ) {
		while( tmp ) {
			switch( tmp->player ) {
				case 1:
					if( k_system.pLeftGame && !FLAG( k_system.pLeftGame->flags, SF_NOMOVE ) ) {
						((keyFunc_t)tmp->func)( flags );
					}
					break;
				case 2:
					if( k_system.pRightGame && !FLAG( k_system.pRightGame->flags, SF_NOMOVE ) ) {
						((keyFunc_t)tmp->func)( flags );
					}
					break;
				default:
					break;
			}
			tmp = tmp->next;
		}
	}
}


/*
=====================
	GenerateKey
=====================
*/
void GenerateKey( TCHAR *action, game_t *game ) {
	TCHAR text[25];
	int i;

	//check arguments
	if( !action || !game ) {
		return;
	}

	//determine game
	if( game->place == LEFTGAME ) {
		N_Sprintf( text, 25, TEXT( "P1_%s" ), action );
	}
	else {
		N_Sprintf( text, 25, TEXT( "P2_%s" ), action );
	}
	
	//determine key function
	i = 0;
	while( g_keyFuncNames[i].name ) {
		if( !N_Strnicmp( g_keyFuncNames[i].name, text, 25 ) ) {
			((keyFunc_t)g_keyFuncNames[i].func)( 0 );
			return;
		}
		i++;
	}
}