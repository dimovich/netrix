
#ifndef __REPLAY_H__
#define __REPLAY_H__

#include "../common/types.h"

#ifdef __cplusplus
extern "C" {
#endif

//init replay system
BOOL initReplaySystem();

//end replay system
BOOL endReplaySystem();

//adds a new demo frame
void addReplayFrame( msg_t, place_t );

//plays a demo
BOOL playDemo();

//playes the next frame
void playNextFrame();

//ends a demo
void endDemo();


#ifdef __cplusplus
}
#endif


#endif