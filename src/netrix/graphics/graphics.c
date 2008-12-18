
//Graphics engine
//

#undef __N_FILE__
#define __N_FILE__ TEXT( "graphics.c" )


#include "../compile.h"
#include <windows.h>
#include <GL/gl.h>

#include "../../netrixlib/netrixlib.h"

#include "graphics.h"
#include "ngl.h"
#include "../game/sys.h"
#include "../game/func.h"
#include "../game/trigger.h"
#include "../common/config.h"

#include "../resource.h"

HDC kDCLeft;
HGLRC kGLLeft;

HDC kDCLeftN;
HGLRC kGLLeftN;

HDC kDCRight;
HGLRC kGLRight;

HDC kDCRightN;
HGLRC kGLRightN;

//trigger bitmaps
//
static HBITMAP hBitmapBomb;
static HBITMAP hBitmapTeleport;
static HBITMAP hBitmapFlag;

//score
//
static HFONT ghFontScore;
static HBRUSH ghBrushScore;

/*
=====================
	initGraphics
	-----------------
	initializes graphics devices
	and OpenGL interface
=====================
*/
BOOL initGraphics( place_t place ) {
	COLORREF cr;
	BYTE r, g, b;
	LOGFONT lf = {0};
	
	switch( place ) {
		case LEFTGAME: // Left

			//init device contexts
			kDCLeft = GetDC( k_system.hwndLeft );
			kDCLeftN = GetDC( k_system.hwndLeftN );

			//init OpenGL interface
			if( ! NGL_Init( LEFTGAME ) ) {
				return FALSE;
			}

			break;
		
		case RIGHTGAME:	// Right

			//init device context
			kDCRight = GetDC( k_system.hwndRight );
			kDCRightN = GetDC( k_system.hwndRightN );
			
			//init OpenGL interface
			if( ! NGL_Init( RIGHTGAME ) )
				return FALSE;

			break;
		
		case NEUTRAL:

			//check if we need to set the pathcast color
			//in case it wasn't explicitly defined.
			//
			if( !kcfTable[VAR_EFF_PATHCAST_COLOR].modified ) {
				cr = kcfTable[VAR_SPACECOLOR].v.dw;
				r = GetRValue( cr );
				g = GetGValue( cr );
				b = GetBValue( cr );
				r = (r<=245) ? r+10 : r-10;
				g = (g<=245) ? g+10 : g-10;
				b = (b<=245) ? b+10 : b-10;
				kcfTable[VAR_EFF_PATHCAST_COLOR].v.dw = RGB( r, g, b );
			}

			//load trigger bitmaps
			//
			hBitmapBomb = LoadBitmap( GetModuleHandle( NULL ),
				MAKEINTRESOURCE( IDB_BITMAP_BOMB ) );
			
			hBitmapTeleport = LoadBitmap( GetModuleHandle( NULL ),
				MAKEINTRESOURCE( IDB_BITMAP_TELEPORT ) );
			
			hBitmapFlag = LoadBitmap( GetModuleHandle( NULL ),
				MAKEINTRESOURCE( IDB_BITMAP_FLAG ) );
			
			
			//create score font
			//
			lf.lfHeight = 20;
			lf.lfCharSet = DEFAULT_CHARSET;
			N_Strncpy( lf.lfFaceName, TEXT( "Lucida Sans" ), LF_FACESIZE );
			ghFontScore = CreateFontIndirect( &lf );
			
			//score background brush
			//
			ghBrushScore = CreateSolidBrush( kcfTable[VAR_SCORESPACECOLOR].v.dw );

			break;
	}

	return TRUE;
}


/*
=====================
	destroyGraphics
=====================
*/
BOOL destroyGraphics( place_t place ) {

	switch( place ) {
		case LEFTGAME:		
			//destroy OpenGL interface
			NGL_Destroy( LEFTGAME );

			//release device context
			ReleaseDC( k_system.hwndLeft, kDCLeft );

			break;
	
		case RIGHTGAME:
			//destroy OpenGL interface
			NGL_Destroy( RIGHTGAME );
			
			//delete device context
			ReleaseDC( k_system.hwndRight, kDCRight );
			break;
	
		case NEUTRAL:
		
			//trigger bitmaps
			//
			DeleteObject( hBitmapBomb );
			DeleteObject( hBitmapTeleport );
			DeleteObject( hBitmapFlag );
			
			//score font
			//
			DeleteObject( ghFontScore );
			
			//score background brush
			//
			DeleteObject( ghBrushScore );
			break;
	}

	return TRUE;
}


/*
=====================
	updateWindow
	-----------------
	update all interface elements
=====================
*/
void updateWindow( DWORD flags ) {

	//clean-up interface
	if( flags & CGF_CLEANUP ) {
		InvalidateRect( k_system.hwnd, NULL, TRUE );
		return;
	}

	//left AS
	if( FLAG( flags, CGF_DRAWLEFT ) ) { 

		drawAS( LEFTGAME, CGF_PAINTALL );
		
		if( FLAG(k_system.flags, SF_GAMEOVER) ) {
			paintGameOver( LEFTGAME );
		}
		else {
			if( FLAG(k_system.flags, SF_PAUSE) && (k_system.gameType > GNO) ) {
				paintPause();
			}
		}
	}

	//right AS
	if( FLAG(flags, CGF_DRAWRIGHT) && ((k_system.gameType == GVS) || (k_system.gameType == GBOT)) ) {

		drawAS( RIGHTGAME, CGF_PAINTALL );
		
		if( FLAG(k_system.flags, SF_GAMEOVER) ) {
			paintGameOver( RIGHTGAME );
		}
	}
}


/*
=====================
	drawAS
	-----------------
	Updates the Action Space
=====================
*/
void drawAS( place_t place, DWORD flags ) {
	COLORREF cr;
	HDC hDC;
	HGLRC hglRC;
	HWND hWnd;
	
	//determine game
	//
	switch( place ) {
		case LEFTGAME:
			hDC = kDCLeft;
			hglRC = kGLLeft;
			hWnd = k_system.hwndLeft;
			break;
		case RIGHTGAME:
			hDC = kDCRight;
			hglRC = kGLRight;
			hWnd = k_system.hwndRight;
			break;
		default:
			return;
	}
	
	//
	//OpenGL
	//
	
	//set current rendering context
	wglMakeCurrent( hDC, hglRC );
	
	//clear background
	//

	if( FLAG( flags, CGF_PAINTALL ) ) {

		glLoadIdentity();
		glOrtho( -5.0f, CXASWINDOW*1.0f-5.0f, CYASWINDOW*1.0f-5.0f, -5.0f, -1.0f, 1.0f );
	
		//clear background
		cr = kcfTable[VAR_SPACECOLOR].v.dw;
		glClearColor( GetRValue( cr )/255.0f, GetGValue( cr )/255.0f, GetBValue( cr )/255.0f, 1.0f );
		glClear( GL_COLOR_BUFFER_BIT );
	}

	//RENDER
	//
	if( k_system.gameType >= GSINGLE ) {

		if( FLAG( flags, CGF_PAINTGROUND ) ) {
			paintGround( place );
		}
		
		if( FLAG( flags, CGF_PAINTFIGURE ) ) {
			paintFigure( place );
		}
		
		if( FLAG( flags, CGF_PAINTEFFECTS ) ) {
			paintEffects( place );
		}
	}
	
	glFlush();

	//swap buffers
	SwapBuffers( hDC );
	
	//
	//GDI
	//

	if( FLAG( flags, CGF_PAINTALL ) ) {
		//paint triggers
		paintTriggers( place );
	}
}


/*
=====================
	paintGround
=====================
*/
void paintGround( place_t place ) {
	game_t *game = NULL;
	int blocksize;
	int i,j;

	blocksize  = BLOCKSIZE;

	//determine game
	switch( place ) {
		case LEFTGAME:
			game = k_system.pLeftGame;
			break;
		
		case RIGHTGAME:
			game = k_system.pRightGame;
			break;

		default:
			return;
	}

	//paint
	if( game && game->AS ) {

		glBegin( GL_QUADS );
		for( i=0; i<CYSPACE; i++ ) {
			for( j=0; j<CXSPACE; j++ ) {
				if( SPACE_CELL( game->AS, i, j ) == MAPCELL_BLOCK ) {
					NGL_drawBlock( j*blocksize, i*blocksize,
						kcfTable[VAR_GROUNDCOLOR].v.dw );
				}
			}
		}
		glEnd();
	}
}


/*
=====================
	paintFigure
=====================
*/
void paintFigure( place_t place ) {
	game_t *game = NULL;
	int blocksize;
	int i, j;
	
	blocksize = BLOCKSIZE;
	
	//determine game
	switch( place ) {
		case LEFTGAME:
			game = k_system.pLeftGame;
			break;
		
		case RIGHTGAME:
			game= k_system.pRightGame;
			break;

		default:
			return;
	}
	
	//paint
	if( game ) {

		glBegin( GL_QUADS );
		for( i=0; i<CYSPACE; i++ ) {
			for( j=0; j<CXSPACE; j++ ) {
				if( (game->FS != NULL) && (SPACE_CELL( game->FS, i, j ) == MAPCELL_BLOCK) ) {
					NGL_drawBlock( j*blocksize, i*blocksize,
						kcfTable[VAR_FIGCOLOR].v.dw );
				}
			}
		}
		glEnd();
	}
	
	//pathcast is a separate issue...
	if( kcfTable[VAR_EFF_PATHCAST].v.b  ) {
		paint_Pathcast( place );
	}
}


/*
=====================
	paintEffects
=====================
*/
void paintEffects( place_t place ) {
	game_t		*game = NULL;
	effectList_t *effList = NULL;

	//determine game	
	switch( place ) {
		case LEFTGAME:
			game = k_system.pLeftGame;
			break;
		
		case RIGHTGAME:
			game = k_system.pRightGame;
			break;

		default:
			return;
	}

	//paint effect
	//
	if( game ) {
		effList = game->effectList;
		while( effList ) {
			if( kEffects[ effList->effect.type ].flag == EFF_POST ) {
				kEffects[ effList->effect.type ].paintFunc( &effList->effect );
			}
			effList = effList->pNext;
		}
	}
}


/*
=====================
	drawNFS
=====================
*/
void drawNFS( place_t place ) {
	game_t *game;
	HDC hDC;
	HGLRC hglRC;
	int i, j;
	COLORREF cr;
	int width, height;
	int left=0, right=0, top=0, bottom=0;
	int offsetx, offsety;
	
	//determine place
	switch( place ) {
		case LEFTGAME:
			game = k_system.pLeftGame;
			hDC = kDCLeftN;
			hglRC = kGLLeftN;
			break;
		
		case RIGHTGAME:
			game = k_system.pRightGame;
			hDC = kDCRightN;
			hglRC = kGLRightN;
			break;
		
		default:
			return;
	}
	
	//check game validity
	if( !game ) {
		return;
	}
	
	wglMakeCurrent( hDC, hglRC );
	
	//clear background
	cr = kcfTable[VAR_NEXTSPACECOLOR].v.dw;
	glClearColor( GetRValue( cr )/255.0f, GetGValue( cr )/255.0f, GetBValue( cr )/255.0f, 1.0f );
	glClear( GL_COLOR_BUFFER_BIT );
	glLoadIdentity();


	if( (game != NULL) && (game->pNextFig != NULL) ) {
	
		//get figure width
		getFigWidth( game->pNextFig, &left, &right );
		width = right-left+1;

		//get figure height
		getFigHeight( game->pNextFig, &top, &bottom );
		height = bottom-top+1;

		offsetx = ( ( CXNFSWINDOW - width*BLOCKSIZEN ) >> 1 );
		offsety = ( ( CYNFSWINDOW - height*BLOCKSIZEN ) >> 1 );

		//kind of SetViewportOrgEx(...)
		glOrtho( -offsetx, CXNFSWINDOW - offsetx, CYNFSWINDOW-offsety, -offsety, -1.0f, 1.0f );
	
		//set color
		cr = kcfTable[VAR_NEXTFIGCOLOR].v.dw;
		glColor3f( GetRValue( cr ) / 255.0f, GetGValue( cr ) / 255.0f,
			GetBValue( cr ) / 255.0f );
	
		glBegin( GL_QUADS );
		for( i=top; i<CYFIG; i++ ) {
			for( j=left; j<CXFIG; j++ ) {
				if( (game->pNextFig->type < CTYPE) && (game->pNextFig->state < CSTATE)
					&& (kFigRes[game->pNextFig->type][i][game->pNextFig->state*CXFIG + j] == 1) ) {
					NGL_polygon( (j-left)*BLOCKSIZEN+0.5f, (i-top)*BLOCKSIZEN+0.5f,
						(j-left+1)*BLOCKSIZEN-0.5f, (i-top+1)*BLOCKSIZEN-0.5f );
				}
			}
		}
		glEnd();
	}
	
	SwapBuffers( hDC );
}


/*
=====================
	paintTriggers
	-----------------
	FIXME: rewrite using OpenGL
	FIXME: what the fuck?
	FIXME: my brain is lame!
=====================
*/
void paintTriggers( place_t place ) {
	HDC hDC;
	HDC hDCRef;
	HWND hWnd;
	HBITMAP hOldBitmap;
	HBITMAP hBitmap;
	int i,j;
	game_t *game;
	BOOL bSkipDraw;

	switch( place ) {
		case LEFTGAME:
			game = k_system.pLeftGame;
			hDC = kDCLeft;
			hWnd = k_system.hwndLeft;
			break;
		
		case RIGHTGAME:
			game = k_system.pRightGame;
			hDC = kDCRight;
			hWnd= k_system.hwndRight;
			break;
		
		default:
			return;
	}

	//check game validity	
	if( game == NULL ) {
		return;
	}

	//check AS validity
	if( game->AS == NULL ) {
		return;
	}
	
	//create bitmap DC
	hDCRef = CreateCompatibleDC( hDC );
	
	//paint
	for( i=0; i<CYSPACE; i++ ) {
		for( j=0; j<CXSPACE; j++ ) {

			bSkipDraw = FALSE;
			switch( SPACE_CELL( game->TS, i, j ) ) {

				case TRIGGER_BOMB:
					hBitmap = hBitmapBomb;
					break;
				
				case TRIGGER_TELEPORT:
					hBitmap = hBitmapTeleport;
					break;
				
				case TRIGGER_FLAG:
					hBitmap = hBitmapFlag;
					break;
				
				default:
					bSkipDraw = TRUE;
					break;
			}
			
			if( !bSkipDraw ) {
				hOldBitmap = SelectObject( hDCRef, hBitmap );
				TransparentBlt( hDC, j*BLOCKSIZE+7, i*BLOCKSIZE+6,
						BLOCKSIZE, BLOCKSIZE, hDCRef, 0, 0, 20, 20, RGB( 255, 0, 0 ) );
				SelectObject( hDCRef, hOldBitmap );
			}
					
		}
	}
	
	//clean-up
	DeleteDC( hDCRef );
}



/*
=====================
	drawScore
=====================
*/
void drawScore( place_t place ) {
	game_t *game;
	HDC hDC;
	HWND hWnd;
	HFONT hOldFont;
	HBITMAP hOldBrush;
	TCHAR szBuff[10];
	RECT rc;
	TEXTMETRIC tm;
	int delta;
	
	//determine place
	//
	switch( place ) {
		case LEFTGAME:
			game = k_system.pLeftGame;
			hWnd = k_system.hWndScoreLeft;
			break;
		
		case RIGHTGAME:
			game = k_system.pRightGame;
			hWnd = k_system.hWndScoreRight;
			break;
		
		default:
			return;
	}
	
	if( !game ) {
		return;
	}
	
	//init
	//
	hDC = GetDC( hWnd );
	hOldFont = (HFONT) SelectObject( hDC, ghFontScore );
	N_Sprintf( szBuff, 10, TEXT( "%d" ), game->score );
	GetClientRect( hWnd, &rc );
	
	//if score cannot fit, extend score area
	//
	GetTextMetrics( hDC, &tm );
	if( tm.tmAveCharWidth*N_Strlen(szBuff)+16 >= rc.right ) {
		delta = tm.tmAveCharWidth*N_Strlen(szBuff)+16 - SCORECX;
		switch( place ) {
			case LEFTGAME:
				//extend window to left
				MoveWindow( hWnd, LSCOREX-delta, LSCOREY, SCORECX+delta, SCORECY, TRUE );
				break;
			case RIGHTGAME:
				//extend window to right
				MoveWindow( hWnd, RSCOREX, RSCOREY, SCORECX+delta, SCORECY, TRUE );
				break;
			default:
				break;
		}
		GetClientRect( hWnd, &rc );
	}
	
	//background
	//
	hOldBrush = SelectObject( hDC, ghBrushScore );
	RoundRect( hDC, -10, 0, rc.right+10, rc.bottom, 20, 20 );
	
	//text
	//
	SetBkMode( hDC, TRANSPARENT );
	SetTextColor( hDC, kcfTable[VAR_SCORETEXTCOLOR].v.dw );
	rc.top+=3;
	DrawText( hDC, szBuff, N_Strlen( szBuff ), &rc, DT_CENTER | DT_NOCLIP );

	//clean-up
	//
	SelectObject( hDC, hOldBrush );
	SelectObject( hDC, hOldFont );
	ReleaseDC( hWnd, hDC );
}


/*
=====================
	paintPause
=====================
*/
void paintPause() {
	HDC hDC;
	int x, y;
	static TCHAR szStr[] = TEXT( "  P A U S E D  " );

	hDC = GetDC( k_system.hwndLeft );
				
	SetBkColor( hDC, RGB( 150, 10, 10 ) );
	SetTextColor( hDC, RGB( 200, 200, 200 ) );

	x = (CXASWINDOW - (N_Strlen( szStr )-3)*LOWORD(GetDialogBaseUnits())) / 2 ;
				
	y = CYASWINDOW/2 - HIWORD(GetDialogBaseUnits())*4;
				
	TextOut( hDC, x, y, szStr, N_Strlen( szStr ) );
	ReleaseDC( k_system.hwndLeft, hDC );
}


/*
=====================
	paintGameOver
=====================
*/
void paintGameOver( place_t place ) {
	HDC hDC;
	HWND hWnd;
	game_t *game = NULL;
	int type;
	int x, y;
	static TCHAR szGameOver[] = TEXT( " G A M E   O V E R " );
	static TCHAR szLoose[] = TEXT( " Y O U   L O O S E " );
	static TCHAR szWin[] = TEXT( " Y O U   W I N " );
	
	//determine place
	//
	switch( place ) {
		case LEFTGAME:
			hWnd = k_system.hwndLeft;
			game = k_system.pLeftGame;
			break;
		case RIGHTGAME:
			hWnd = k_system.hwndRight;
			game = k_system.pRightGame;
			break;
		default:
			return;
	}
	
	//check game validity
	if( !game ) {
		return;
	}

	//determine message type
	//
	if( game->flags & SF_GAMELOST ) {
		if( (k_system.gameType == GVS) || (k_system.gameType == GBOT) ) {
			type = GAME_LOOSE;
		}
		else {
			type = GAME_OVER;
		}
	}
	else {
		type = GAME_WIN;
	}


	hDC = GetDC( hWnd );

	//display message
	//
	switch( type ) {
		case GAME_OVER:
			SetBkColor( hDC, RGB( 166, 113, 131 ) );
			SetTextColor( hDC, RGB( 206, 230, 183 ) );
			x = (CXASWINDOW - N_Strlen(szGameOver)*LOWORD(GetDialogBaseUnits()))/2 + 20;
			y = (CYASWINDOW/2) - HIWORD(GetDialogBaseUnits())*4;
			TextOut( hDC, x, y, szGameOver, N_Strlen( szGameOver ) );
			break;

		case GAME_LOOSE:
			SetBkColor( hDC, RGB( 166, 113, 131 ) );
			SetTextColor( hDC, RGB( 206, 230, 183 ) );
			x = (CXASWINDOW - N_Strlen( szLoose )*LOWORD(GetDialogBaseUnits())) / 2 + 20;
			y = (CYASWINDOW/2) - HIWORD(GetDialogBaseUnits())*4;
			TextOut( hDC, x, y, szLoose, N_Strlen( szLoose ) );
			break;
		
		case GAME_WIN:
			SetBkColor( hDC, RGB( 206, 230, 183 ) );
			SetTextColor( hDC, RGB( 176, 113, 131 ) );
			x = (CXASWINDOW - N_Strlen( szWin )*LOWORD(GetDialogBaseUnits())) / 2 + 20;
			y = (CYASWINDOW/2) - HIWORD(GetDialogBaseUnits())*4;
			TextOut( hDC, x, y, szWin, N_Strlen( szWin ) );
			break;
		
		default:
			break;
	}
	
	ReleaseDC( hWnd, hDC );
}