
#ifndef __SCHEDULER_H__
#define __SCHEDULER_H__

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
	BLOCK,
	FIGURE,
	BOT,
	EFFECT,
	GAME,
	REPLAY,
	SYSTEM
} object_t;

	// TIME Slices

typedef enum timeSliceActions_e {
	TIME_BLOCK_XMOVE,		//block processing	(REPEATING)
	TIME_BLOCK_YMOVE,		//block processing	(REPEATING)
	TIME_FIG_YMOVE,			//figure y move processing	(REPEATING)
	TIME_GAME_WAIT,			//wait time for a new fig to be spawned
	TIME_BOT_FRAME,			//bot frame
	TIME_EFFECT,			//effect (REPEATING)
	TIME_TRIGGER_RESPAWN,	//bombs are respawned (REPEATING)
	TIME_TELEPORT,			//teleport a figure
	TIME_REPLAY_FRAME,		//play next demo frame
	TIME_SYSTEM_UPDATEWINDOW, //update main window
	TIME_GAME_OVER,			//game over
} timeSliceActions_t;


typedef struct _timeSlice_t {
	DWORD					acum;		// accumulator
	DWORD					interval;	// interval
	DWORD					ttl;		// time to live
	DWORD					ttl_acum;	// time to live accumulator
	timeSliceActions_t		action;		// type of the action
	void 					*pObj;		// some OOP style :)
	object_t				objType;	// object type
	DWORD					guid;		// globally unique identifier
	struct _timeSlice_t		*pNext;		// next time slice
} timeSlice_t;



// parses all time slices, and executes the valid ones
void parseTimeSlices();

// creates a new time slice
DWORD pushTimeSlice( timeSliceActions_t action, DWORD interval, DWORD ttl, void *pObj, object_t type, BOOL inst);

// removes a time slice
void popTimeSlice( DWORD guid );

// pops all movement time slices for a specific object
void popMovementSlices( void * );

//pop bot slices for the specific object
//
void popBotSlices( void * );

//removes all events from the scheduler
//
void endAllTimeSlices();

#ifdef __cplusplus
}
#endif

#endif