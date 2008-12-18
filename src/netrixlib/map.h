
#ifndef __MAP_H__
#define __MAP_H__

#include "const.h"

#ifdef __cplusplus
extern "C" {
#endif

//cell types
//
#define MAPCELL_EMPTY		0
#define MAPCELL_BLOCK		1
#define MAPCELL_BOMB		2
#define MAPCELL_TELEPORT	3
#define MAPCELL_FLAG		4

// Map
typedef struct {
	TCHAR		name[NAMESIZE+1];	//map name
	int			*map;				//map array
} map_t;

BOOL loadMap( TCHAR *, map_t * );
// loads a map


BOOL saveMap( TCHAR *, map_t * );
// saves a map

//gets the internal map name
void getMapName( TCHAR *, TCHAR * );


#ifdef __cplusplus
}
#endif


#endif