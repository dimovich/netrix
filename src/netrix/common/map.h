
#ifndef __GMAP_H__
#define __GMAP_H__

#ifdef __cplusplus
extern "C" {
#endif

//mapEntry_t
typedef struct {
	fileEntry_t	fe;		//file entry
	map_t		map;	//map
	BOOL		loaded;	//TRUE if already loaded; FALSE otherwise
} mapEntry_t;


//loads a map entry
BOOL loadMapEntry( mapEntry_t * );


//un-loads a map entry
void unloadMapEntry( mapEntry_t * );


//set-ups a map
BOOL setMap( game_t *game, int );

//find map by CRC
BOOL seeMapID( DWORD, int * );

#ifdef __cplusplus
}
#endif

#endif