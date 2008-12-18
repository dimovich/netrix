
#ifndef __KEYS_H__
#define __KEYS_H__

#include "../game/game.h"

#ifdef __cplusplus
extern "C" {
#endif

//key flags
//
#define KF_REPEATING	(1<<0)

//bind a key
BOOL BindKey( TCHAR *key, TCHAR *action );

//unbind keys
void UnbindKeys();

//process key
void KeyProc( int vk, int flags );

//generate key from action
void GenerateKey( TCHAR *action, game_t *game );

#ifdef __cplusplus
}
#endif

#endif