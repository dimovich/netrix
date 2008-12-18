
// Netrix Pack Files
//

#undef __N_FILE__
#define __N_FILE__ TEXT( "npk.c" )

#include "compile.h"
#include <windows.h>

#include "npk.h"
#include "libc.h"
#include "crc.h"
#include "compress.h"
#include "map.h"
#include "bot.h"

#define BLOCK_SIZE (4*1024)


/*
=====================
	npkBuild
=====================
*/
BOOL npkBuild( TCHAR *szRootPath, TCHAR *szPackFile ) {
	WIN32_FIND_DATA		FileData;
	npkFileEntry_t		*pFE;	//file entries
	npkFileEntry_t		*fileEntry;
	npkHeader_t			npk;
	HANDLE				hPack;
	HANDLE				hFile;
	HANDLE				hSearch;
	TCHAR				szPath[MAX_PATH] = {0};
	TCHAR				szArchPath[MAX_PATH] = {0};
	TCHAR				szName[NAMESIZE+1];
	DWORD				cnt;
	DWORD				tmp;
	DWORD				tmp2;
	DWORD				crc32;
	DWORD				size;
	BYTE				buff[BLOCK_SIZE];
	BOOL				bRes;
	int					type;
	int					i;
	

	//init

	npk.dwChecksum = 0;
	cnt =
	npk.dwFileNum = 0;
	npk.iHeader = NPKHEADER;
	
	pFE = NULL;
	
	crcInit( &crc32 );
	
	size = 0;

	__try {
		
		N_Sprintf( szPath, MAX_PATH, TEXT( "%s\\*.*" ), szRootPath );

		hSearch = FindFirstFile( szPath, &FileData );
		if( hSearch == INVALID_HANDLE_VALUE )
			__leave;
		
		//process files
		while( TRUE ) {
			//directories are not processed
			if( !( FileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY ) ) {
			
				//only *.map and *.bot files are processed

				if( N_Strstr( FileData.cFileName, TEXT( ".map" ) ) ) {
					type = NPKENTRY_MAP;
				}
				else if( N_Strstr( FileData.cFileName, TEXT( ".c" ) ) ) {
					type = NPKENTRY_BOT;
				}
				else {
					if( !FindNextFile( hSearch, &FileData ) ) {
						__leave;
					}
					continue;
				}
			
				//increase file count
				cnt++;

				//increase memory for pFE
				if( pFE )
					pFE = N_Realloc( pFE, sizeof( npkFileEntry_t ) * cnt );
				else
					pFE = N_Malloc( sizeof( npkFileEntry_t ) );

				//init file entry

				fileEntry = &( pFE[cnt-1] );
				N_Strncpy(fileEntry->szFileName, FileData.cFileName, FILENAMESIZE );
				fileEntry->type = type;

				//get internal resource name
				//
				N_Sprintf( szPath, MAX_PATH, TEXT( "%s\\%s" ), szRootPath, FileData.cFileName );
				switch( type ) {
					case NPKENTRY_MAP:
						getMapName( szPath, szName );
						N_Strncpy( fileEntry->szIName, szName, NAMESIZE );
						break;
					
					case NPKENTRY_BOT:
						getBotName( szPath, szName );
						N_Strncpy( fileEntry->szIName, szName, NAMESIZE );
						break;
					
					default:
						break;
				}
			}
			
			if( !FindNextFile( hSearch, &FileData ) )
				__leave;
		}
	}
	__finally {
		//clean-up
		if( hSearch != INVALID_HANDLE_VALUE ) {
			FindClose( hSearch );
		}
	}


	__try {

		//init file count
		npk.dwFileNum = cnt;

		//open output file
		hPack = N_FOpenC( szPackFile );
		if( hPack == INVALID_HANDLE_VALUE )
			__leave;
		
		//write header
		WriteFile( hPack, &npk, sizeof( npk ), &tmp, NULL );
		if( tmp != sizeof( npk ) ) {
			__leave;
		}
		
		//write file entries
		WriteFile( hPack, pFE, sizeof( *pFE ) * cnt, &tmp, NULL );
		if( tmp != sizeof( *pFE ) * cnt )
			__leave;

		//add file data
		for( i=0; i<(int)cnt; i++ ) {

			//retrieve file offset
			pFE[i].dwOffset = N_FSeek( hPack, 0, FILE_CURRENT );

			//set-up path
			N_Sprintf( szPath, MAX_PATH, TEXT( "%s\\%s" ),
				szRootPath, pFE[i].szFileName );
			N_Sprintf( szArchPath, MAX_PATH, TEXT( "%s\\%s.z" ),
				szRootPath, pFE[i].szFileName );

	
			beginCompress( szPath ); //begin compression
			{
				//open data file
				hFile = N_FOpenR( szArchPath );
				if( hFile == INVALID_HANDLE_VALUE )
					__leave;

				//insert data file into pack file
				while( TRUE ) {
					bRes = ReadFile( hFile, buff, BLOCK_SIZE, &tmp, NULL );
					
					if( bRes && tmp>0 ) {
						//write data to pack file
						WriteFile( hPack, buff, tmp, &tmp2, NULL );
						if( tmp != tmp2 ) {
							__leave;
						}
						
						size += tmp;
						
						//update checksum
						crcUpdate( &crc32, buff, tmp );
					}
					else {
						break;
					}
				}

				//set file size
				pFE[i].dwSize = N_FSeek( hPack, 0, FILE_CURRENT ) - pFE[i].dwOffset;
				
				//close input data file
				N_FClose( hFile );

			}
			endCompress( szPath ); //end compression

		}//cnt

		//update header and file entries

		crcFinish( &crc32 );
		npk.dwChecksum = crc32;

		N_FSeek( hPack, 0, FILE_BEGIN );

		WriteFile( hPack, &npk, sizeof( npk ), &tmp, NULL );
		if( tmp != sizeof( npk ) )
			__leave;
		
		WriteFile( hPack, pFE, sizeof( *pFE ) * npk.dwFileNum, &tmp, NULL );
		if( tmp != sizeof( *pFE ) * npk.dwFileNum )
			__leave;
	}
	
	__finally {
		//clean-up

		if( hPack != INVALID_HANDLE_VALUE )
			N_FClose( hPack );

		if( pFE )
			N_Free( pFE );
	}
	
	return TRUE;
}


/*
=====================
	npkExtract
=====================
*/
BOOL npkExtract( TCHAR *szPack, TCHAR *szRootPath ) {
	npkFileEntry_t	*pFE;
	npkHeader_t		npk;
	HANDLE			hPack;	//pack file
	HANDLE			hFile;	//extracted file
	HANDLE			hUFile;	//uncompressed file
	DWORD			cnt;
	DWORD			tmp;
	TCHAR			szPath[MAX_PATH]; //extracted file path
	TCHAR			szUPath[MAX_PATH]; //uncompressed file path
	BYTE			buff[BLOCK_SIZE];
	BOOL			bRes;
	int				usize;
	int				i;

	
	__try {
	
		bRes = FALSE;
	
		hPack = N_FOpenR( szPack );
		if( hPack == INVALID_HANDLE_VALUE )
			__leave;
		
		//read header
		ReadFile( hPack, &npk, sizeof( npk ), &tmp, NULL );
		if( tmp != sizeof( npk ) )
			__leave;
		
		//check header
		if( npk.iHeader != NPKHEADER )
			__leave;
		
		if( !npkVerifyChecksum( szPack ) )
			__leave;

		//get file nbr.
		cnt = npk.dwFileNum;
		
		pFE = N_Malloc( cnt * sizeof( *pFE ) );
		
		//read file entries
		ReadFile( hPack, pFE, cnt * sizeof( *pFE ), &tmp, NULL );
		if( tmp != cnt * sizeof( *pFE ) )
			__leave;

		//output files
		for( i=0; i<(int)cnt; i++ ) {
			N_Sprintf( szPath, MAX_PATH, TEXT( "%s\\%s" ),
				szRootPath, pFE[i].szFileName );
			
			hFile = N_FOpenC( szPath );
			if( hFile == INVALID_HANDLE_VALUE )
				__leave;

			if( beginDecompress( szPack, pFE[i].dwOffset, szUPath, &usize ) ) {

				hUFile = N_FOpenR( szUPath );
				if( hUFile == INVALID_HANDLE_VALUE )
					__leave;

				//write data to extracted file
				while( usize ) {
					if( usize <= BLOCK_SIZE ) {
						if( N_FRead( hUFile, buff, usize ) ) {
							N_FWrite( hFile, buff, usize );
						}
						usize = 0;
					}
					else {
						if( N_FRead( hUFile, buff, BLOCK_SIZE ) ) {
							N_FWrite( hFile, buff, BLOCK_SIZE );
						}
						usize -= BLOCK_SIZE;
					}
				}
				N_FClose( hUFile );
				endDecompress( szUPath );
			}
			
			N_FClose( hFile );
			N_FSeek( hPack, pFE[i].dwSize, FILE_CURRENT );
		}
		
		bRes = TRUE;
	}
	
	__finally {
		if( hPack != INVALID_HANDLE_VALUE )
			N_FClose( hPack );
		
		if( pFE )
			N_Free( pFE );
	}
	
	return bRes;
}


/*
=====================
	npkVerifyChecksum
=====================
*/
BOOL npkVerifyChecksum( TCHAR *szPack ) {

	npkFileEntry_t	*pFE;
	npkHeader_t		*npk;

	HANDLE		hPack;
	HANDLE		hPackMap;
	BOOL		bRes;
	BYTE		*pData;
	BYTE		*pMap;
	DWORD		crc32;
	DWORD		size;
	int			i;
	
	hPackMap = NULL;
	crc32 = 0;
	size = 0;
	
	__try {
		bRes = FALSE;
		
		//open file
		hPack = N_FOpenR( szPack );
		if( hPack == INVALID_HANDLE_VALUE )
			__leave;
		
		//create file mapping
		hPackMap = CreateFileMapping( hPack, NULL, PAGE_READONLY, 0, 0, NULL );
		if( hPackMap == NULL )
			__leave;

		pData = MapViewOfFile( hPackMap, FILE_MAP_READ, 0, 0, 0 );
		if( pData == NULL )
			__leave;
		
		pMap = pData;
		
		//get header
		npk = (npkHeader_t *) pMap;
		pMap += sizeof( *npk );
		
		//determine data size
		for( i=0; i<(int)npk->dwFileNum; i++ ) {
			pFE = (npkFileEntry_t *) pMap;
			size += pFE->dwSize;
			pMap += sizeof( *pFE );
		}
		
		crc32 = crcBlock( pMap, size );
		
		if( crc32 == npk->dwChecksum )
			bRes = TRUE;
		else
			bRes = FALSE;
	}

	__finally {
		if( hPack != INVALID_HANDLE_VALUE )
			N_FClose( hPack );
		
		if( hPackMap )
			CloseHandle( hPackMap );
		
		if( pData )
			UnmapViewOfFile( pData );
	}
	
	return bRes;
}