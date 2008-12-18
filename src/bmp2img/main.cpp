
#include <windows.h>
#include <stdio.h>

#include "bmp2img.h"

int main( int argc, char **argv ) {
	DWORD size;
	HANDLE hFile;
	DWORD *pic;
	DWORD width, height;
	DWORD *texture;
	DWORD tmp;

	hFile = NULL;
	pic = NULL;
	texture = NULL;
	__try {

		if( argc != 3 ) {
			printf( "usage: %s <infile> <outfile>\n", argv );
			__leave;
		}

		//load BMP file
		loadBMP( argv[1], &texture, &width, &height );

		//convert BMP file to IMG (Netrix internal image type)
		bmp2img( texture, &pic, width, height, &size );
		
		hFile = CreateFile( argv[2], GENERIC_WRITE, FILE_SHARE_READ, NULL,
			CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL );
		
		if( hFile == INVALID_HANDLE_VALUE )
			__leave;
		
		WriteFile( hFile, pic, size*sizeof( DWORD ), &tmp, NULL );
	}
	__finally {
		if( pic )
			free( pic );
		
		if(hFile != NULL
			|| hFile != INVALID_HANDLE_VALUE )
			CloseHandle( hFile );
	}
	
	return 0;
}