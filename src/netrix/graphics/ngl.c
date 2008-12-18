
//Netrix OpenGL wrapper code
//

#undef __N_FILE__
#define __N_FILE__ TEXT( "ngl.c" )

#include "../compile.h"
#include <windows.h>
#include <GL/gl.h>
#include <GL/glu.h>

#include "../../netrixlib/netrixlib.h"

#include "../common/const.h"
#include "graphics.h"
#include "ngl.h"

/*
=====================
	NGL_Init
=====================
*/
BOOL NGL_Init( place_t place ) {
	HGLRC hglrc1, hglrc2;
	HDC hDC1, hDC2;
	PIXELFORMATDESCRIPTOR pfd={0};
	int iPixelFormat;
	
	pfd.nSize = sizeof( pfd );
	pfd.nVersion = 1;
	pfd.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
	pfd.iPixelType = PFD_TYPE_RGBA;
	pfd.cColorBits = 24;
	pfd.cDepthBits = 16;
	pfd.iLayerType = PFD_MAIN_PLANE;
	
	switch( place ) {
		case LEFTGAME:
			hDC1 = kDCLeft;
			hDC2 = kDCLeftN;
			break;
		
		case RIGHTGAME:
			hDC1 = kDCRight;
			hDC2 = kDCRightN;
			break;
	}
	
	iPixelFormat = ChoosePixelFormat( hDC1, &pfd );
	SetPixelFormat( hDC1, iPixelFormat, &pfd );
	
	iPixelFormat = ChoosePixelFormat( hDC2, &pfd );
	SetPixelFormat( hDC2, iPixelFormat, &pfd );

	hglrc1 = wglCreateContext( hDC1 );
	hglrc2 = wglCreateContext( hDC2 );
	
	//set-up RCs
	//
	wglMakeCurrent( hDC1, hglrc1 );
	//init
	glOrtho( -5.0f, CXASWINDOW*1.0f-5.0f, CYASWINDOW*1.0f-5.0f, -5.0f, -1.0f, 1.0f );
	//optimizations
	glEnable( GL_CULL_FACE );
	
	switch( place ) {
		case LEFTGAME:
			kGLLeft = hglrc1;
			kGLLeftN = hglrc2;
			break;
		
		case RIGHTGAME:
			kGLRight = hglrc1;
			kGLRightN = hglrc2;
			break;
	}
	
	return TRUE;
}


/*
=====================
	NGL_Destroy
=====================
*/
void NGL_Destroy( place_t place ) {
	HGLRC hglrc1, hglrc2;
	
	switch( place ) {
		case LEFTGAME:
			hglrc1 = kGLLeft;
			hglrc2 = kGLLeftN;
			break;
		
		case RIGHTGAME:
			hglrc1 = kGLRight;
			hglrc2 = kGLRightN;
			break;
	}
	
	wglMakeCurrent( NULL, NULL );
	wglDeleteContext( hglrc1 );
	wglDeleteContext( hglrc2 );
}