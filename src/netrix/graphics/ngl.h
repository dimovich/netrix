
#ifndef __NGL_H__
#define __NGL_H__


#ifdef __cplusplus
extern "C" {
#endif


//init OpenGL
BOOL NGL_Init( place_t );

//destroy OpenGL
void NGL_Destroy( place_t );

//draws a block (macro is slightly faster)
#define NGL_drawBlock( x, y, cr ) { \
	glColor3f( GetRValue( cr )/255.0f, GetGValue( cr )/255.0f, GetBValue( cr )/255.0f ); \
	glVertex2f( (GLfloat)x+0.5f, (GLfloat)y+0.5f ); \
	glVertex2f( (GLfloat)x+0.5f, (GLfloat)(y+BLOCKSIZE)-0.5f ); \
	glVertex2f( (GLfloat)(x+BLOCKSIZE)-0.5f, (GLfloat)(y+BLOCKSIZE)-0.5f ); \
	glVertex2f( (GLfloat)(x+BLOCKSIZE)-0.5f, (GLfloat)y+0.5f ); }


//draw a polygon (macro is slightly faster)
#define NGL_polygon( x, y, xf, yf ) { \
	glVertex2f( x, y );		\
	glVertex2f( x, yf );	\
	glVertex2f( xf, yf );	\
	glVertex2f( xf, y ); }


//copy backbuffer to frontbuffer (macro is slightly faster)
#define NGL_Swap( x, y, w, h ) { \
	glReadBuffer( GL_BACK ); \
	glDrawBuffer( GL_FRONT ); \
	glCopyTexSubImage2D( GL_TEXTURE_2D, 0, 0, 0, x, y, w, h ); }


#ifdef __cplusplus
}
#endif


#endif