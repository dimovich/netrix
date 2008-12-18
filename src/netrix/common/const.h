
//Constants
//

#ifndef __GCONST_H__
#define __GCONST_H__


#define CXWINDOW		( 440 )			/* Netrix window width */
#define CYWINDOW		( 515 )			/* Netrix window height */


#define CXASWINDOWSM	( 2*CXASWINDOW/3 )	// Small window width
#define CYASWINDOWSM	( 2*CYASWINDOW/3 )	// Small window height


#define LEFTASPOSX		( 20 )	//left AS x pos
#define LEFTASPOSY		( 120 )	//left AS y pos

#define RIGHTASPOSX		( 20 )	//right AS x pos
#define RIGHTASPOSY		( 50 )	//right AS y pos


#define CXASWINDOW		( BLOCKSIZE * CXSPACE + 10 )			/* AS window width */
#define CYASWINDOW		( BLOCKSIZE * CYSPACE + 10 )			/* AS window height */

#define LEFTNFSPOSX		( LEFTASPOSX + CXASWINDOW + 7 )
#define LEFTNFSPOSY		( LEFTASPOSY + 10 )

#define RIGHTNFSPOSX	( RIGHTASPOSX + CXASWINDOW + 5 )
#define RIGHTNFSPOSY	( RIGHTASPOSY + 10 )

#define CXNFSWINDOW		( BLOCKSIZEN * ( CXFIG ) + 6 )
#define CYNFSWINDOW		( BLOCKSIZEN * ( CYFIG ) + 6 )

#define LSCOREX	(353)
#define LSCOREY (85)

#define RSCOREX (30)
#define RSCOREY (15)

#define SCORECX	(48)
#define SCORECY	(25)

#define MAXPLAYERS		2	// maximum players (local + network, left + right, etc.)

#define TIME_FIG_YMOVE_INTERVAL 640
#define TIME_FIG_CRUSH_INTERVAL 10
#define TIME_FIG_CRUSH_FAST_INTERVAL 3
#define TIME_GAME_WAIT_INTERVAL 1
#define TIME_BLOCK_YMOVE_INTERVAL  100
#define TIME_BLOCK_XMOVE_INTERVAL  150
#define TIME_TRIGGER_RESPAWN_INTERVAL (30*1000) //each 30 seconds triggers are respawned
#define TIME_TELEPORT_INTERVAL 500
#define TIME_SYSTEM_UPDATEWINDOW_INTERVAL (10*1000)
#define TIME_GAME_OVER_INTERVAL (5*1000)

#define NETRIX_VERSION TEXT( "0.2.1_dev" )

#endif
