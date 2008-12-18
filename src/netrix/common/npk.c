
// Netrix Pack Files
//

#undef __N_FILE__
#define __N_FILE__ TEXT( "npk.c" )

#include "../compile.h"
#include <windows.h>

#include "../../netrixlib/netrixlib.h"

#include "npk.h"
#include "../game/sys.h"


/*
=====================
	beginNpkExtract
=====================
*/
BOOL beginNpkExtract( fileEntry_t *pFE, TCHAR *szPath ) {

	if( (pFE == NULL) || (szPath == NULL) ) {
		return FALSE;
	}

	return( beginDecompress( k_system.pPaths[pFE->pathID].szPath, pFE->lOffset, szPath, NULL ) );
}



/*
=====================
	endNpkExtract
=====================
*/
void endNpkExtract( TCHAR *szPath ) {

	if( szPath == NULL ) {
		return;
	}

	endDecompress( szPath );
}