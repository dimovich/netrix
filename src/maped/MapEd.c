
//MapEd - Netrix map editor

#undef __N_FILE__
#define __N_FILE__ TEXT( "MapEd.c" )

#include <windows.h>
#include <commctrl.h>
#include <tchar.h>

#include "../netrixlib/netrixlib.h"

#include "MapEd.h"
#include "resource.h"
#include <afxres.h>

#define MAPX	13
#define MAPY	68

//ID's
#define ID_MAPED_ERASE		(1)
#define ID_MAPED_BLOCK		(2)
#define ID_MAPED_TELEPORT	(3)
#define ID_MAPED_BOMB		(4)
#define ID_MAPED_FLAG		(5)


//control ID's
#define IDC_STATIC_MAP		1018
#define IDC_EDIT_MAPNAME	1019

HWND hDlgMapEd;

//extern system_t k_system;
static int gMap[CYSPACE * CXSPACE] = {0,};	//map array
static HBITMAP hBackBitmap;	//static background
static HBITMAP hMapBitmap;

static HBITMAP hBlockBitmap;	//block bitmap
static HBITMAP hBombBitmap;	//bomb bitmap
static HBITMAP hFlagBitmap;	//flag bitmap
static HBITMAP hTeleBitmap;	//teleport bitmap

static HBRUSH hBrushFig;

static int gxCur, gyCur;			//cursor position
static int gSel;		//selection
static BOOL gSaved;	//for "Save" / "Save As"
static TCHAR gszPath[MAX_PATH];	//for "Save" / "Save As"


/*
=====================
	WinMain
=====================
*/
int WINAPI WinMain( HINSTANCE hInst, HINSTANCE hPrevInst, LPSTR szCmdLine, int nCmdShow ) {
	
	DialogBox( hInst, MAKEINTRESOURCE( IDD_DIALOG_MAPED ),
		NULL, (DLGPROC)DlgMapEdProc );
	
	return 0;
}


/*
=====================
	DlgMapEdProc
=====================
*/
BOOL CALLBACK DlgMapEdProc( HWND hWndDlg, UINT msg, WPARAM wParam, LPARAM lParam ) {
	HDC hDC;
	RECT rc;
	PAINTSTRUCT ps;
	static int sel;
	int x, y;
	int xMap, yMap;
	static int width, height;
	static BOOL cursor_visible;

	switch( msg ) {
		
		case WM_INITDIALOG:
		
			//center the dialog box
			//
			GetWindowRect( hWndDlg, &rc );
			width = rc.right - rc.left;
			height = rc.bottom - rc.top;
			rc.left = (GetSystemMetrics( SM_CXSCREEN ) - width) >> 1;
			rc.top = ((GetSystemMetrics( SM_CYSCREEN ) - height) >> 1) - 50;
			MoveWindow( hWndDlg, rc.left, rc.top, width, height, FALSE );

			//create map static window
			CreateWindow( TEXT( "static" ), NULL,
				WS_CHILDWINDOW | WS_VISIBLE | WS_BORDER, MAPX, MAPY, CXSPACE*BLOCKSIZE+2, CYSPACE*BLOCKSIZE+2,
				hWndDlg, (HMENU) IDC_STATIC_MAP, GetModuleHandle( NULL ), NULL );

			//map name edit box
			CreateWindow( TEXT( "edit" ), NULL,
				ES_AUTOHSCROLL | WS_CHILDWINDOW | WS_VISIBLE | WS_BORDER, 88, 36, 200, 20,
				hWndDlg, (HMENU) IDC_EDIT_MAPNAME, GetModuleHandle( NULL ), NULL );

			//for IsDialogMessage(...)
			//in the main game loop
			hDlgMapEd = hWndDlg;

			//init toolbar
			mapEdCreateToolbar( hWndDlg );
			
			//create background bitmap
			createBackBitmap();
			
			//load Bomb, Flag and Teleport bitmaps
			initBitmaps();
			
			//init global variables
			gSaved = FALSE;
			N_Memset( gszPath, 0, MAX_PATH*sizeof(TCHAR) );
			N_Memset( gMap, 0, sizeof( int ) * CXSPACE * CYSPACE );

			//init static variables
			sel = 1;
			cursor_visible = TRUE;
			
			
			//map static control width and height
			width = MAPX + BLOCKSIZE*CXSPACE;
			height = MAPY + BLOCKSIZE*CYSPACE;
			
			hBrushFig = CreateSolidBrush( FIGCOLOR );

			break;

		case WM_COMMAND:
			switch( LOWORD( wParam ) ) {

				case ID_MAPED_ERASE:
					sel = MAPCELL_EMPTY;
					break;

				case ID_MAPED_BLOCK:
					sel = MAPCELL_BLOCK;
					break;

				case ID_MAPED_BOMB:
					sel = MAPCELL_BOMB;
					break;

				case ID_MAPED_TELEPORT:
					sel = MAPCELL_TELEPORT;
					break;

				case ID_MAPED_FLAG:
					sel = MAPCELL_FLAG;
					break;

				case ID_FILE_SAVE:
					mapEdSave( hWndDlg, FALSE );
					break;
				
				case ID_FILE_SAVEAS:
					mapEdSave( hWndDlg, TRUE );
					break;

				case ID_FILE_OPEN:
					mapEdLoad( hWndDlg );
					paintMap( GetDlgItem( hWndDlg, IDC_STATIC_MAP ) );
					break;
				
				case ID_FILE_NEW:
					N_Memset( gMap, 0, sizeof( int ) * CXSPACE * CYSPACE );
					SetDlgItemText( hWndDlg, IDC_EDIT_MAPNAME, TEXT( "" ) );
					InvalidateRect( hWndDlg, NULL, TRUE );
					UpdateWindow( hWndDlg );
					break;

				case ID_FILE_EXIT:
					DestroyWindow( hWndDlg );
					break;
			}
			break;
		
		case WM_PAINT:
			hDC = BeginPaint( hWndDlg, &ps );
			EndPaint( hWndDlg, &ps );

			paintMap( GetDlgItem( hWndDlg, IDC_STATIC_MAP ) );

			break;
		
		//add element
		case WM_LBUTTONDOWN:
		case WM_RBUTTONDOWN:
		case WM_MOUSEMOVE:

			gxCur = ( x = LOWORD( lParam ) );
			gyCur = ( y = HIWORD( lParam ) );
			
			if( x>MAPX && x<width
				&& y>MAPY && y<height ) {
				
				//hide cursor
				if( cursor_visible ) {
					ShowCursor( FALSE );
					cursor_visible = FALSE;
				}
			
				( x -= MAPX );
				( y -= MAPY );
			
				xMap = x / BLOCKSIZE;
				yMap = y / BLOCKSIZE;
				
				switch( msg ) {
					case WM_LBUTTONDOWN:
						gMap[yMap*CXSPACE + xMap] = sel;
						break;
					
					case WM_RBUTTONDOWN:
						gMap[yMap*CXSPACE + xMap] = 0;
						break;
					
					case WM_MOUSEMOVE:
						if( MK_LBUTTON & wParam )
							gMap[yMap*CXSPACE + xMap] = sel;
						else
						if( MK_RBUTTON & wParam )
							gMap[yMap*CXSPACE + xMap] = 0;
						break;
				}
				
				gSel = sel;

				paintMap( GetDlgItem( hWndDlg, IDC_STATIC_MAP ) );
			}

			else {
					//show cursor
					if( !cursor_visible ) {
						ShowCursor( TRUE );
						cursor_visible = TRUE;

						//erase the "tool" cursor
						paintMap( GetDlgItem( hWndDlg, IDC_STATIC_MAP ) );
					}
				}
			
			break;

		
		case WM_CLOSE:
			DestroyWindow( hWndDlg );
			break;
		
		case WM_DESTROY:
			//show cursor
			if( !cursor_visible )
				ShowCursor( TRUE );

			//delete GDI objects

			DeleteObject( hBackBitmap );
			DeleteObject( hBombBitmap );
			DeleteObject( hTeleBitmap );
			DeleteObject( hFlagBitmap );
			DeleteObject( hBlockBitmap );
			DeleteObject( hMapBitmap );
			
			DeleteObject( hBrushFig );
			
			//delete save path
			if( gszPath )
				N_Free( gszPath );

			break;
		
		default:
			return FALSE;
	}
	
	return TRUE;
}


/*
=====================
	mapEdCreateToolbar
=====================
*/
#define NUMBUTTONS 7
void mapEdCreateToolbar( HWND hWndDlg ) {
	HWND hWndTB;
	TBADDBITMAP tbab;
	TBBUTTON tbb[NUMBUTTONS];
	HINSTANCE hInst = GetModuleHandle( NULL );
	
	//make sure common controls DLL is loaded
	InitCommonControls();
	
	//create toolbar window
	hWndTB = CreateWindowEx( 0, TOOLBARCLASSNAME, NULL,
		WS_CHILDWINDOW | WS_BORDER,
		0, 0, 0, 0,
		hWndDlg, (HMENU) IDR_TOOLBAR_MAPED, hInst, NULL );
	
	//determine DLL version
	SendMessage( hWndTB, TB_BUTTONSTRUCTSIZE, (WPARAM) sizeof( TBBUTTON ), 0 );
	
	//add bitmap containing button images to the toolbar
	tbab.hInst = hInst;
	tbab.nID = IDR_TOOLBAR_MAPED;
	SendMessage( hWndTB, TB_ADDBITMAP, (WPARAM) NUMBUTTONS, (WPARAM) &tbab );

	tbb[0].iBitmap = 0;
	tbb[0].idCommand = ID_FILE_OPEN;
	tbb[0].fsState = TBSTATE_ENABLED;
	tbb[0].fsStyle = TBSTYLE_BUTTON;
	tbb[0].dwData = 0;
	tbb[0].iString = 0;
	
	tbb[1].iBitmap = 1;
	tbb[1].idCommand = ID_FILE_SAVE;
	tbb[1].fsState = TBSTATE_ENABLED;
	tbb[1].fsStyle = TBSTYLE_BUTTON;
	tbb[1].dwData = 0;
	tbb[1].iString = 0;
	
	tbb[2].iBitmap = 2;
	tbb[2].idCommand = ID_MAPED_ERASE;
	tbb[2].fsState = TBSTATE_ENABLED;
	tbb[2].fsStyle = TBSTYLE_BUTTON;
	tbb[2].dwData = 0;
	tbb[2].iString = 0;

	tbb[3].iBitmap = 3;
	tbb[3].idCommand = ID_MAPED_BLOCK;
	tbb[3].fsState = TBSTATE_ENABLED;
	tbb[3].fsStyle = TBSTYLE_BUTTON;
	tbb[3].dwData = 0;
	tbb[3].iString = 0;
	
	tbb[4].iBitmap = 4;
	tbb[4].idCommand = ID_MAPED_TELEPORT;
	tbb[4].fsState = TBSTATE_ENABLED;
	tbb[4].fsStyle = TBSTYLE_BUTTON;
	tbb[4].dwData = 0;
	tbb[4].iString = 0;
	
	tbb[5].iBitmap = 5;
	tbb[5].idCommand = ID_MAPED_BOMB;
	tbb[5].fsState = TBSTATE_ENABLED;
	tbb[5].fsStyle = TBSTYLE_BUTTON;
	tbb[5].dwData = 0;
	tbb[5].iString = 0;
	
	tbb[6].iBitmap = 6;
	tbb[6].idCommand = ID_MAPED_FLAG;
	tbb[6].fsState = TBSTATE_ENABLED;
	tbb[6].fsStyle = TBSTYLE_BUTTON;
	tbb[6].dwData = 0;
	tbb[6].iString = 0;
	
	SendMessage( hWndTB, TB_ADDBUTTONS, (WPARAM) NUMBUTTONS,
		(LPARAM) &tbb );
	
	ShowWindow( hWndTB, SW_SHOW );
}


/*
=====================
	paintBackMap
=====================
*/
void paintBackBitmap( HDC hDCRef) {
	
	HDC hDCBack;
	HDC hDC;
	HWND hWnd;
	HBITMAP hOldBitmap;
	
	//create Device Context
	hWnd = GetDesktopWindow();
	hDC = GetDC( hWnd );
	hDCBack = CreateCompatibleDC( hDC );
	
	hOldBitmap = SelectObject( hDCBack, hBackBitmap );

	//paint background
	BitBlt( hDCRef, 0, 0, CXSPACE*BLOCKSIZE, CYSPACE*BLOCKSIZE,
		hDCBack, 0, 0, SRCCOPY );
	
	//clean-up
	SelectObject( hDCBack, hOldBitmap );
	ReleaseDC( hWnd, hDC );
	DeleteDC( hDCBack );
}


/*
=====================
	createBackBitmap
=====================
*/
void createBackBitmap() {
	HDC hDCt;
	HDC hDC;
	HBITMAP hOldBitmap;
	HPEN hPen;
	RECT rc;
	int i;
	
	hDCt = GetDC( NULL );
	hDC = CreateCompatibleDC( hDCt );
	
	//create bitmap
	hBackBitmap = CreateCompatibleBitmap( hDCt,
		CXSPACE*BLOCKSIZE, CYSPACE*BLOCKSIZE );
	
	//select bitmap into device context
	hOldBitmap = SelectObject( hDC, hBackBitmap );

	//draw background
	SetRect( &rc, 0, 0, CXSPACE*BLOCKSIZE, CYSPACE*BLOCKSIZE );
	FillRect( hDC, &rc, GetStockObject( LTGRAY_BRUSH ) );

	//create pen
	hPen = CreatePen( PS_DOT, 0, RGB( 110, 110, 110 ) );
	SelectObject( hDC, hPen );

	//transparent line inter-spaces
	SetBkMode( hDC, TRANSPARENT );

	//draw intersected lines

	for( i=1; i<CYSPACE; i++ ) {
		MoveToEx( hDC, 0, i*BLOCKSIZE, NULL );
		LineTo( hDC, CXSPACE*BLOCKSIZE, i*BLOCKSIZE );
	}
	
	for( i=1; i<CXSPACE; i++ ) {
		MoveToEx( hDC, i*BLOCKSIZE, 0, NULL );
		LineTo( hDC, i*BLOCKSIZE, CYSPACE*BLOCKSIZE );
	}

	//clean-up
	SelectObject( hDC, hOldBitmap );
	DeleteDC( hDC );
	ReleaseDC( NULL, hDCt );
	
	DeleteObject( hPen );
}


/*
=====================
	paintMap
=====================
*/
void paintMap( HWND hCtl ) {
	HBITMAP hOldBitmap;
	HBITMAP hOldBitmap2;
	HBITMAP hBitmap;
	HDC hDC;
	HDC hDCRef;
	HDC hDCSel;
	int i,j;
	int width = CXSPACE*BLOCKSIZE;
	int height = CYSPACE*BLOCKSIZE;
	RECT rc;
	BOOL bSkipDraw;

	// init Device Contexts

	hDC = GetDC( hCtl );
	hDCRef = CreateCompatibleDC( hDC );
	hDCSel = CreateCompatibleDC( hDC );

	hOldBitmap = (HBITMAP) SelectObject( hDCRef, hMapBitmap );
	
	//paint static background
	paintBackBitmap( hDCRef );
	
	//paint triggers

	for( i=0; i<CYSPACE; i++ ) {
		for( j=0; j<CXSPACE; j++ ) {

			if( gMap[i*CXSPACE + j] == 0 )	//empty cell
				continue;
			
			if( gMap[i*CXSPACE + j] == MAPCELL_BLOCK ) {
				rc.left = j*BLOCKSIZE;
				rc.top = i*BLOCKSIZE;
				rc.right = rc.left + BLOCKSIZE;
				rc.bottom = rc.top + BLOCKSIZE;
				
				FillRect( hDCRef, &rc, hBrushFig );
				continue;
			}

			bSkipDraw = FALSE;
			switch( gMap[i*CXSPACE + j] ) {
				case MAPCELL_BLOCK:
					hBitmap = hBlockBitmap;
					break;
				
				case MAPCELL_BOMB:
					hBitmap = hBombBitmap;
					break;
				
				case MAPCELL_TELEPORT:
					hBitmap = hTeleBitmap;
					break;

				case MAPCELL_FLAG:
					hBitmap = hFlagBitmap;
					break;
				
				default:
					bSkipDraw = TRUE;
					break;
			}
			
			if( !bSkipDraw ) {
				hOldBitmap2 = (HBITMAP) SelectObject( hDCSel, hBitmap );
				TransparentBlt( hDCRef, j*BLOCKSIZE, i*BLOCKSIZE, BLOCKSIZE, BLOCKSIZE,
					hDCSel, 0, 0, 20, 20, RGB( 255, 0, 0 ) );
				SelectObject( hDCSel, hOldBitmap2 );
			}
		}
	}
	
	//paint cursor
	paintCursor( hDCRef );

	//paint all stuff to map's static window
	BitBlt( hDC, 0, 0, width, height, hDCRef, 0, 0, SRCCOPY );
	
	SelectObject( hDCRef, hOldBitmap );
	ReleaseDC( hCtl, hDC );
	DeleteDC( hDCRef );
	DeleteDC( hDCSel );
	
	ValidateRect( hCtl, NULL );
}


/*
=====================
	paintCursor
=====================
*/
void paintCursor( HDC hDC ) {
	HDC hDCSel;
	HBITMAP hOldBitmap;
	HBITMAP hBitmap;
	HBITMAP hEraseBitmap;
	RECT rc;
	int x, y;
	BOOL bSkipDraw;

	if( gxCur < MAPX || gxCur > MAPX + CXSPACE*BLOCKSIZE
		|| gyCur < MAPY || gyCur > MAPY + CYSPACE*BLOCKSIZE )

		return;

	__try {

		x = gxCur - MAPX - 5;
		y = gyCur - MAPY;
		
		//create Device Context for selected bitmap
		hDCSel = CreateCompatibleDC( hDC );
		
		//for faster stretch operations
		SetStretchBltMode( hDC, STRETCH_DELETESCANS );
		
		//load erase bitmap
		hEraseBitmap = LoadBitmap( GetModuleHandle( NULL ),
			MAKEINTRESOURCE( IDB_BITMAP_ERASE ) );


		// user selected Block "tool"

		if( gSel == MAPCELL_BLOCK ) {
			rc.left = x;
			rc.top = y;
			rc.right = rc.left + BLOCKSIZE/2;
			rc.bottom = rc.top + BLOCKSIZE/2;

			FillRect( hDC, &rc, hBrushFig );

			__leave;
		}


		// user selected other "tools"

		bSkipDraw = FALSE;
		switch( gSel ) {
			case MAPCELL_EMPTY:
				hBitmap = hEraseBitmap;
				break;
			
			case MAPCELL_BOMB:
				hBitmap = hBombBitmap;
				break;
			
			case MAPCELL_TELEPORT:
				hBitmap = hTeleBitmap;
				break;
			
			case MAPCELL_FLAG:
				hBitmap = hFlagBitmap;
				break;

			default:
				bSkipDraw = TRUE;
				break;
		}

		if( !bSkipDraw ) { //paint selected bitmap

			hOldBitmap = SelectObject( hDCSel, hBitmap );

			if( gSel == MAPCELL_EMPTY )
				TransparentBlt( hDC, x, y, BLOCKSIZE, BLOCKSIZE,
					hDCSel, 0, 0, 20, 20, RGB( 255, 0, 0 ) );
			else
				TransparentBlt( hDC, x, y, 2*BLOCKSIZE/3, 2*BLOCKSIZE/3,
					hDCSel, 0, 0, 20, 20, RGB( 255, 0, 0 ) );

			SelectObject( hDCSel, hOldBitmap );
		}
	}
	
	__finally {	
		DeleteObject( hEraseBitmap );
		DeleteDC( hDCSel );
	}
}


/*
=====================
	initBitmaps
=====================
*/
void initBitmaps() {
	HDC hDC;
	HINSTANCE hInst = GetModuleHandle( NULL );

	//load trigger bitmaps

	hBombBitmap = LoadBitmap( hInst, MAKEINTRESOURCE( IDB_BITMAP_BOMB ) );
	hFlagBitmap = LoadBitmap( hInst, MAKEINTRESOURCE( IDB_BITMAP_FLAG ) );
	hTeleBitmap = LoadBitmap( hInst, MAKEINTRESOURCE( IDB_BITMAP_TELEPORT ) );


	hDC = GetDC( 0 );

	//create Map bitmap

	hMapBitmap = CreateCompatibleBitmap( hDC,
		CXSPACE*BLOCKSIZE, CYSPACE*BLOCKSIZE );

	//clean-up

	ReleaseDC( 0, hDC );
}


/*
=====================
	mapEdSave
=====================
*/
void mapEdSave( HWND hWnd, BOOL saveAgain ) {
	TCHAR szPath[MAX_PATH];
	map_t *map = NULL;
	
	__try {
		//init map
		map = N_Malloc( sizeof( *map ) );
		map->map = gMap;
		N_Memset( map->name, 0, 25 );
		GetWindowText( GetDlgItem( hWnd, IDC_EDIT_MAPNAME ) , map->name, NAMESIZE );

		//get path

		if( saveAgain || !gSaved ) {
			if( !GetSavePath( hWnd, TEXT( "(*.map)\0*.map\0" ), TEXT( "*.map" ), szPath ) ) {
				__leave;
			}

			N_Strncpy( gszPath, szPath, MAX_PATH*sizeof(TCHAR) );
			gSaved = TRUE;
		}

		//save		
		if( !saveMap( gszPath, map ) ) {
			MessageBox( hWnd, TEXT( "Could not save map" ), TEXT( "Saving Error" ), 0 );
			__leave;
		}
	}
	
	__finally {
		//clean-up

		if( map ) {
			N_Free( map );
		}
	}
}


/*
=====================
	mapEdLoad
=====================
*/
void mapEdLoad( HWND hWnd ) {
	TCHAR szPath[MAX_PATH];
	map_t *map = NULL;

	__try {
		//init map

		map = N_Malloc( sizeof( *map ) );
		map->map = gMap;
		N_Memset( map->name, 0, 25 );

		//get path
		if( !GetLoadPath( hWnd, TEXT( "(*.map)\0*.map\0" ), TEXT( "*.map" ), szPath ) ) {
			__leave;
		}

		//load map
		if( !loadMap( szPath, map ) ) {
			MessageBox( hWnd, TEXT( "Could not load map" ), TEXT( "Loading Error" ), 0 );
			__leave;
		}
		
		//set map name edit box
		SetDlgItemText( hWnd, IDC_EDIT_MAPNAME, map->name );

		gSaved = FALSE;
	}

	__finally {
		//clean-up

		if( map )
			N_Free( map );
	}
}