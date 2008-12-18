
//Code parts written by Saikou (www.subduck.com)


#include <windows.h>
#include <olectl.h>

#include "bmp2img.h"

void bmp2img( const DWORD *data , DWORD **pic, DWORD width, DWORD height, DWORD *size ) {
	DWORD len = width * height;
	DWORD *buff = (DWORD*) malloc( width * height * sizeof( DWORD ) );
	DWORD c;
	DWORD code;
	DWORD _code;
	DWORD j;

	buff[1] = width;
	buff[2] = height;

	code = _code = data[0];
	c = 0;
	j=3;
	for( int i=1; i<len; i++ ) {
		code = data[i];
		c++;
		if( code != _code ) {
			buff[j++] = c;
			buff[j++] = _code;
			c = 0;
			_code = code;
		}
	}
	
	buff[j++] = c;
	buff[j++] = code;
	buff[0] = *size = j;
	
	*pic = (DWORD *)malloc( j * sizeof( DWORD ) );
	
	memcpy( *pic, buff, j*sizeof( DWORD ) );
	
	free( buff );
}


void loadBMP( char *szPath, DWORD **pic, DWORD *width, DWORD *height ) {
	OLECHAR wszPath[MAX_PATH];
	IPicture *pPicture;
	HBITMAP hOldBitmap;
	HRESULT hRes;
	char dir[MAX_PATH];
	
	GetCurrentDirectory( MAX_PATH, dir );
	
	strcat( dir, "\\" );
	strcat( dir, szPath );
	
	MultiByteToWideChar( CP_ACP, 0, dir, -1, wszPath, MAX_PATH );
	
	hRes = OleLoadPicturePath( wszPath, 0, 0, 0, IID_IPicture, (void **)&pPicture );
	
	if( FAILED( hRes ) ) {
		*pic = NULL;
		*height = *width = -1;
		return ;
	}

	long lHeight, lWidth;
	pPicture->get_Width( &lWidth );
	pPicture->get_Height( &lHeight );
	
	HWND hWnd = GetDesktopWindow();
	
	HDC hDCDesk = GetDC( hWnd );
	HDC hDC = CreateCompatibleDC( hDCDesk );
	
	ReleaseDC( hWnd, hDCDesk );
	
	//lWidth * ( 96 / 2540 )
	*width = MulDiv( lWidth, GetDeviceCaps( hDC, LOGPIXELSX ), 2540 );
	*height = MulDiv( lHeight, GetDeviceCaps( hDC, LOGPIXELSY ), 2540 );
	
	BITMAPINFO bi = {0};
	DWORD *pbits = NULL;
	
	bi.bmiHeader.biSize = sizeof( BITMAPINFOHEADER );
	bi.bmiHeader.biBitCount = 32;
	bi.bmiHeader.biWidth = *width;
	bi.bmiHeader.biHeight = *height;
	bi.bmiHeader.biCompression = BI_RGB;
	bi.bmiHeader.biPlanes = 1;
	
	HBITMAP hBitmap = CreateDIBSection( hDC, &bi, DIB_RGB_COLORS, (void **)&pbits, NULL, NULL );
	
	*pic = pbits;
	
	hOldBitmap = (HBITMAP) SelectObject( hDC, hBitmap );
	
	hRes = pPicture->Render( hDC, 0, 0, *width, *height, 0, 0, lWidth, lHeight, 0 );

	hBitmap = (HBITMAP) SelectObject( hDC, hOldBitmap );
	DeleteDC( hDC );
}