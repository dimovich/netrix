
#undef __N_FILE__
#define __N_FILE__ TEXT( "(netrix) map.c" )


#include "../compile.h"
#include <windows.h>

#include "../../netrixlib/netrixlib.h"

#include "../game/sys.h"
#include "../game/func.h"
#include "../game/game.h"
#include "../game/trigger.h"
#include "const.h"
#include "npk.h"
#include "map.h"


/*
=====================
	loadMapEntry
=====================
*/
BOOL loadMapEntry( mapEntry_t * me ) {
	TCHAR szPath[MAX_PATH];

	//check arguments validity
	if( me == NULL ) {
		return FALSE;
	}

	if( ! me->loaded ) { //load map entry

		me->map.map = N_Malloc( sizeof( int ) * CXSPACE * CYSPACE );
		
		if( me->fe.dwFlags & FILE_PACKED ) { //load packed file
			if( beginNpkExtract( &me->fe, szPath ) ) {
				loadMap( szPath, &me->map );
				endNpkExtract( szPath );
			}
			else
				return FALSE;
		}
		else if( me->fe.dwFlags & FILE_SINGLE ) { //load individual maps
			loadMap( k_system.pPaths[ me->fe.pathID ].szPath, &me->map );
		}
		
		me->loaded = TRUE;
	}

	return TRUE;
}


/*
=====================
	unloadMapEntry
=====================
*/
void unloadMapEntry( mapEntry_t *me ) {

	//check arguments validity
	if( me == NULL ) {
		return;
	}

	if( me->loaded ) {

		//free map
		N_Free( me->map.map );
		
		if( me->fe.dwFlags & FILE_PACKED ) {
			//unload packed file
		}
		else if( me->fe.dwFlags & FILE_SINGLE ) {
			//unload single file
		}
		
		me->loaded = FALSE;
	}
}


/*
=====================
	setMap
=====================
*/
BOOL setMap( game_t *game, int idMap ) {
	int i,j;
	
	//check arguments validity
	if( (game == NULL) || (idMap<0) || (idMap>=k_system.cMaps) ) {
		return FALSE;
	}

	if( (game->AS == NULL) || (k_system.pMaps == NULL) ) {
		return FALSE;
	}

	//set map
	if( k_system.pMaps[idMap].loaded ) {

		for( i=0; i<CYSPACE; i++ ) {
			for( j=0; j<CXSPACE; j++ ) {

				switch( k_system.pMaps[ idMap ].map.map[i*CXSPACE+j] ) {

					case MAPCELL_BLOCK:
						SPACE_CELL( game->AS, i, j ) = MAPCELL_BLOCK;
						break;
					
					case MAPCELL_BOMB:
						SPACE_CELL( game->TS, i, j ) = TRIGGER_BOMB;
						break;
					
					case MAPCELL_TELEPORT:
						SPACE_CELL( game->TS, i, j ) = TRIGGER_TELEPORT;
						break;
					
					case MAPCELL_FLAG:
						SPACE_CELL( game->TS, i, j ) = TRIGGER_FLAG;
						break;

					default:
						break;
				}
			}
		}
	} //map loaded
	
	return TRUE;
}


/*
=====================
	seeMapID
=====================
*/
BOOL seeMapID( DWORD crc, int *mapID ) {
	int i;

	for( i=0; i<k_system.cMaps; i++ ) {
		if( k_system.pMaps[i].fe.dwCRC == crc ) {
			if( mapID ) {
				*mapID = i;
			}
			return TRUE;
		}
	}

	return FALSE;
}