
//Netrix scheduler
//
//The heart that pumps all
//bloody actions in the game.
//

#undef __N_FILE__
#define __N_FILE__ TEXT( "scheduler.c" )

#include "../compile.h"
#include <windows.h>

#include "../../netrixlib/netrixlib.h"

#include "scheduler.h"
#include "../bot/bot.h"
#include "../graphics/effect.h"
#include "../win32/func_win.h"
#include "func.h"
#include "seq.h"
#include "sys.h"
#include "replay.h"

extern system_t k_system;

timeSlice_t		*kTimeSlices;


/*
=====================
	parseTimeSlices
=====================
*/
void parseTimeSlices() {
	DWORD curr = 1;
	timeSlice_t *ts, *tmp;
	timeSlice_t slice;
	BOOL skip;	//in case of some events (Ex: effects)
				//there is no need of calling seqProc(...)

	skip = TRUE;
	ts = kTimeSlices;

	while( ts!=NULL ) {
		
		tmp = ts->pNext;

		ts->acum += curr;		

		//set ttl
		if( ts->ttl > 0 ) {
			ts->ttl_acum += curr;
		}

		//execute event
		//
		if( ts->acum >= ts->interval ) {
			
			//reset time accumulator
			ts->acum = 0;
			
			//make a copy of time slice
			slice = *ts;

			switch( slice.objType ) {
				//Block
				//
				case BLOCK:
					doBlockAction( (figure_t *)slice.pObj, slice.action );
					skip = FALSE;
					break;
				//Figure
				//
				case FIGURE:
					doFigAction( (figure_t *)slice.pObj, slice.action );
					skip = FALSE;
					break;
				//Bot
				//
				case BOT:
					botFrame( (game_t *)slice.pObj );
					skip = FALSE;
					break;
				//Effect
				//
				case EFFECT:
					doEffect( (effectList_t *)slice.pObj );
					break;
				//Game
				//
				case GAME:
					doGameAction( (game_t *)slice.pObj, slice.action );
					skip = FALSE;
					break;
				//Replay
				//
				case REPLAY:
					playNextFrame();
					skip = FALSE;
					break;
				//System
				//
				case SYSTEM:
					doSystemAction( slice.action );
					break;
				
				default:
					break;

			} // switch

			//check if event has ended
			if( (slice.ttl > 0) && (slice.ttl_acum >= slice.ttl) ) {
				popTimeSlice( slice.guid );
			}

			//process game sequence
			if( skip == FALSE ) {
				seqProc();
				skip = TRUE;
			}
			
			//all time slices ended in seqProc
			if( kTimeSlices == NULL ) {
				return;
			}
			
			//if timeslice list changed, restart parsing
			if( k_system.flags & SF_TIMEUPDATED ) {
				k_system.flags &= ~SF_TIMEUPDATED;
				return;
			}

		}//if

		//process next slice
		ts = tmp;

	}//while
}


/*
=====================
	pushTimeSlice
=====================
*/
DWORD pushTimeSlice( timeSliceActions_t action, DWORD interval, DWORD ttl, void *pObject, object_t  type, BOOL inst ) {
	timeSlice_t *ts;

	ts = N_Malloc( sizeof( *ts ) );

	//set time slice
	//
	ts->interval = interval;
	ts->action = action;
	ts->pObj = pObject;
	ts->objType = type;
	ts->acum = 0;
	ts->ttl = ttl;
	ts->ttl_acum = 0;
	ts->guid = N_Random( k_system.dwAccumTime );

	//if instantaneous, the event will
	//be procesed as soon as possible
	//	
	if( inst ) {
		ts->acum = interval;
		
		if( ts->ttl > 0 ) {
			ts->ttl_acum = interval;
		}
	}

	if( kTimeSlices == NULL ) {
		ts->pNext = NULL;
		kTimeSlices = ts;
	}
	else {
		ts->pNext = kTimeSlices;
		kTimeSlices = ts;
	}
	
	return ts->guid;
}


/*
=====================
	popTimeSlice
=====================
*/
void popTimeSlice( DWORD guid ) {
	timeSlice_t *ts;
	timeSlice_t *tmp;
	
	tmp =
	ts = kTimeSlices;
	
	while( ts != NULL ) {
		if( ts->guid == guid ) {

			//effects are automatically destroyed
			//
			//FIXME: isn't there a better approach?
			//
			if( ts->objType == EFFECT ) {
				deattachEffect( (effectList_t *)ts->pObj );
			}

			if( ts == kTimeSlices ) {
				kTimeSlices = kTimeSlices->pNext;
			}
			else {
				tmp->pNext = ts->pNext;
			}
			N_Free( ts );
			ts = NULL;
			k_system.flags |= SF_TIMEUPDATED;
			return;
		}
		tmp = ts;
		ts = ts->pNext;
	}
}


/*
=====================
	endAllTimeSlices
=====================
*/
void endAllTimeSlices() {
	timeSlice_t *ts = kTimeSlices;
	timeSlice_t *tmp = ts;
	
	while( ts!=NULL ) {
		tmp = ts->pNext;
		popTimeSlice( ts->guid );
		ts = tmp;
	}
	kTimeSlices = NULL;
}


/*
=====================
	popMovementSlices
=====================
*/
void popMovementSlices( void *pObj ) {
	timeSlice_t *ts = kTimeSlices;
	timeSlice_t *tmp = ts;
	
	while( ts != NULL ) {
		tmp = ts->pNext;
		if( ts->pObj == pObj ) {
			switch( ts->action ) {
				case TIME_FIG_YMOVE:
					popTimeSlice( ts->guid );
					if( ((game_t *)((figure_t *)pObj)->parent) != NULL ) {
						((game_t *)((figure_t *)pObj)->parent)->flags &= ~SF_FIGCRUSHED;
					}
					break;
				default:
					break;
			}
		}
		ts = tmp;
	}
}


/*
=====================
	popBotSlices
=====================
*/
void popBotSlices( void *pObj ) {
	timeSlice_t *ts = kTimeSlices;
	timeSlice_t *tmp = ts;
	
	while( ts != NULL ) {
		tmp = ts->pNext;
		if( ts->pObj == pObj ) {
			switch( ts->action ) {
				case TIME_BOT_FRAME:
					popTimeSlice( ts->guid );
					break;
				default:
					break;
			}
		}
		ts = tmp;
	}
}