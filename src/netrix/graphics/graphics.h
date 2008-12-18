// High Level graphics functions.


#ifndef __GRAPHICS_H__
#define __GRAPHICS_H__

#include "../compile.h"
#include <windows.h>

#include "../../netrixlib/netrixlib.h"

#include "../common/types.h"


#ifdef __cplusplus
extern "C" {
#endif


extern HDC kDCLeft;
extern HDC kDCRight;

extern HDC kDCLeftN;
extern HDC kDCRightN;

extern HGLRC kGLLeft;
extern HGLRC kGLRight;

extern HGLRC kGLLeftN;
extern HGLRC kGLRightN;


//paint flags
#define CGF_PAINTGROUND		(1<<0)
#define CGF_PAINTFIGURE		(1<<1)
#define CGF_PAINTEFFECTS	(1<<2)
#define CGF_DRAWLEFT		(1<<3)
#define CGF_DRAWRIGHT		(1<<4)
#define CGF_CLEANUP			(1<<5)
#define CGF_PAINTALL		(CGF_PAINTGROUND | CGF_PAINTFIGURE | CGF_PAINTEFFECTS)
#define CGF_DRAWBOTH		(CGF_DRAWLEFT | CGF_DRAWRIGHT)


//effect types
#define EFF_POST			(1<<0)
#define EFF_CONST			(1<<1)


BOOL initGraphics( place_t );
//init graphics

BOOL destroyGraphics( place_t );
//destroy graphics

//draw next figure space
void drawNFS( place_t );

void drawAS( place_t, DWORD flags );
//draws the action space

void updateWindow( DWORD );
//updates all graphics

void paintGround( place_t );
// paints the ground with all the effects applied

void paintFigure( place_t );
// paints the moving figure with all the effects applied

// paints effects (pathcast, motionblur etc.)
void paintEffects( place_t );

void paintTriggers( place_t );

void drawScore( place_t );

//paint pause message
void paintPause();

//paint game over message
void paintGameOver( place_t place );

#ifdef __cplusplus
}
#endif


#endif