
#ifndef __EFFECT_H__
#define __EFFECT_H__

#include "../compile.h"
#include <windows.h>

#include "../common/types.h"


#ifdef __cplusplus
extern "C" {
#endif


// effectType_t
typedef enum effectType_e {
	EFFECT_FIG_COROLA,			// figure has a light corola around it
	EFFECT_FIG_MOTIONBLUR,		// motion blur
	EFFECT_GROUND_ILLUMINATE,	// ground is illuminated
	EFFECT_AS_LINEKILL,			// line killing...(probably the best effect in the world)
	EFFECT_GROUND_TOXICBLOCKS,	// toxic blocks

	EFFLIST_SIZE
} effectType_t;



// effect_t
typedef struct effect_s {
	effectType_t	type;			// effect type
	float			R, G, B;		// color
	place_t			place;			// 0 - left window, 1 - right window
	float			v1, v2, v3, v4; // additional data	
	BOOL			init;			// init flag
	int				frame;			// frame number
	BOOL			bEnded;			// TRUE if effect ended, FALSE otherwise;
	int				guid;			// scheduler guid
} effect_t;


typedef void (*effectFunc_t)( effect_t *, BOOL );
typedef void (*paintFunc_t)( const effect_t * );

//effectConfig_t
//
typedef struct effectConfig_s {
	int				flag;		// effect flag (separation flag)
	effectType_t	type;		// effect type
	int				interval;	// -> frame rate
	int				ttl;		// time to live
	int				priority;	// priority (the bigger, the latter the effect will be drawn)
	effectFunc_t	calcFunc;	// effect shader function
	paintFunc_t		paintFunc;	// effect paint function
} effectConfig_t;


//effectList_t
//
typedef struct effectList_s {
	effect_t effect;
	struct effectList_s *pNext;
} effectList_t;




extern effectConfig_t kEffects[ EFFLIST_SIZE ];

void doEffect( effectList_t * );

void pushEffect( effectType_t, int );

effectList_t * attachEffect( effect_t * );

void deattachEffect( effectList_t * );

void calc_eff_GroundIlluminate( effect_t *, BOOL );
void paint_eff_GroundIlluminate( const effect_t * );

void calc_eff_GroundToxicBlocks( effect_t *, BOOL );
void paint_eff_GroundToxicBlocks( const effect_t * );

void calc_eff_AsLineKill( effect_t *, BOOL );
void paint_eff_AsLineKill( const effect_t * );

void paint_Pathcast( place_t );


#ifdef __cplusplus
}
#endif


#endif