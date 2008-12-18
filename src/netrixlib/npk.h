
#ifndef __NPK_H__
#define __NPK_H__

#include "const.h"
#include "types.h"

#ifdef __cplusplus
extern "C" {
#endif

#define NPKHEADER (('1'<<24) + ('K'<<16) + ('P'<<8) + 'N')


#pragma pack( push, 1 )

// npkHeader_t
typedef struct {
	int		iHeader;		//NPKHEADER
	DWORD	dwChecksum;		//checksum
	DWORD	dwFileNum;		//number of file table entries
} npkHeader_t;


// npkFileEntry_t
typedef struct {
	enum {
		NPKENTRY_BOT,	//bot script
		NPKENTRY_MAP,	//map
	} type;
	DWORD	dwOffset;					//offset of the file in the pack
	DWORD	dwSize;						//file size
	TCHAR	szFileName[FILENAMESIZE];	//name of the file in the pack
	TCHAR	szIName[NAMESIZE];			//resource internal name
} npkFileEntry_t ;


#pragma pack( pop )


//builds a netrix pack file
BOOL npkBuild( TCHAR *, TCHAR * );


//extracts the contents of a netrix pack file
BOOL npkExtract( TCHAR *, TCHAR * );


//verifies checksum for a specified pack file
BOOL npkVerifyChecksum( TCHAR * );


#ifdef __cplusplus
}
#endif

#endif