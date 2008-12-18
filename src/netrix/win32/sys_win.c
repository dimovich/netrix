
// Entry point
// Win32 message processing
//

#undef __N_FILE__
#define __N_FILE__ TEXT( "sys_win.c" )

#include "../compile.h"
#include <windows.h>
#include <mmsystem.h>
#include <windowsx.h> //message crackers
#include <gl/gl.h>

#include "../game/sys.h"
#include "func_win.h"
#include "sys_win.h"
#include "console_win.h"
#include "../game/func.h"
#include "../game/seq.h"
#include "../game/replay.h"
#include "../common/keys.h"

#include "../resource.h"


static int gMouseX;
static int gMouseY;
static HWND hDlgConsole;

BOOL kbConsoleVisible;

extern HWND hWndDlgRight;



LRESULT CALLBACK WndProc( HWND, UINT, WPARAM, LPARAM );


//message crackers

BOOL Cls_OnCreate(HWND hwnd, LPCREATESTRUCT lpCreateStruct);
void Cls_OnPaint(HWND hwnd);
void Cls_OnKey(HWND hwnd, UINT vk, BOOL fDown, int cRepeat, UINT flags);
void Cls_OnDestroy(HWND hwnd);
void Cls_OnActivate(HWND hwnd, UINT state, HWND hwndActDeact, BOOL fMinimized);
void Cls_OnCommand(HWND hwnd, int id, HWND hwndCtl, UINT codeNotify);
void Cls_OnLButtonDown(HWND hwnd, BOOL fDoubleClick, int x, int y, UINT keyFlags);
BOOL Cls_OnEraseBkgnd(HWND hwnd, HDC hdc);
void Cls_OnDrawItem(HWND hwnd, const DRAWITEMSTRUCT * lpDrawItem);
void Cls_OnMouseMove(HWND hwnd, int x, int y, UINT keyFlags);
void Cls_OnKillFocus(HWND hwnd, HWND hwndNewFocus);
void Cls_OnLButtonUp(HWND hwnd, int x, int y, UINT keyFlags);



/*
=====================
	Cls_OnCreate
=====================
*/
BOOL Cls_OnCreate( HWND hwnd, LPCREATESTRUCT lpCreateStruct ) {
	//set skin
	SetWindowRgn( hwnd, k_system.hSkinRgnLeft, TRUE );
	
	//create console
	hDlgConsole = CreateDialog( GetModuleHandle( NULL ),
		MAKEINTRESOURCE( IDD_DIALOG_CONSOLE ), k_system.hwnd, DlgConsoleProc );
	return TRUE;
}


/*
=====================
	Cls_OnKillFocus
=====================
*/
void Cls_OnKillFocus(HWND hwnd, HWND hwndNewFocus) {
	//when user presses on the Right AS, we don't
	//let Windows kill our focus.
	if( hwndNewFocus == hWndDlgRight
		|| hwndNewFocus == k_system.hwndRight )
		SetFocus( k_system.hwnd );
}


/*
=====================
	Cls_OnMouseMove
=====================
*/
void Cls_OnMouseMove(HWND hwnd, int x, int y, UINT keyFlags) {
	int i;
	RECT rc;

	//Reset hover effect
	//
	//FIXME: Isn't there a better approach ?
	//
	SendMessage( GetDlgItem( hwnd, ID_BUTTON_NEW ), BM_SETSTATE, 0, 0 );
	SendMessage( GetDlgItem( hwnd, ID_BUTTON_OPTIONS ), BM_SETSTATE, 0, 0 );
	SendMessage( GetDlgItem( hwnd, ID_BUTTON_QUIT ), BM_SETSTATE, 0, 0 );
	SendMessage( GetDlgItem( hwnd, ID_BUTTON_MINIMIZE ), BM_SETSTATE, 0, 0 );
	SendMessage( GetDlgItem( hwnd, ID_BUTTON_ENDGAME ), BM_SETSTATE, 0, 0 );
	

	//mass window move
	if( keyFlags & MK_LBUTTON ) {
		if( gMouseX == -1 || gMouseY == -1 ) {
			gMouseX = x;
			gMouseY = y;
			return;
		}
		
		i = 1;
		if( k_system.gameType > GSINGLE )
			i = 2;

		switch( i ) {
			case 2:
				GetWindowRect( hWndDlgRight, &rc );
				MoveWindow( hWndDlgRight, rc.left - (gMouseX - x), rc.top - (gMouseY - y),
					rc.right-rc.left, rc.bottom-rc.top, TRUE );
			case 1:
				GetWindowRect( k_system.hwnd, &rc );
				MoveWindow( k_system.hwnd, rc.left - (gMouseX - x), rc.top - (gMouseY - y),
					rc.right-rc.left, rc.bottom-rc.top, TRUE );
				break;
		}
	}
}


/*
=====================
	Cls_OnLButtonDown
=====================
*/
void Cls_OnLButtonDown(HWND hwnd, BOOL fDoubleClick, int x, int y, UINT keyFlags) {
	//On left button press, user can move the window
	
	gMouseX =
	gMouseY = -1;
}


/*
=====================
	Cls_OnLButtonUp
=====================
*/
void Cls_OnLButtonUp(HWND hwnd, int x, int y, UINT keyFlags) {
}


/*
=====================
	Cls_OnDrawItem
=====================
*/
void Cls_OnDrawItem(HWND hwnd, const DRAWITEMSTRUCT * lpDrawItem) {

	FillRect( lpDrawItem->hDC, &lpDrawItem->rcItem,
		(HBRUSH) GetStockObject( LTGRAY_BRUSH ) );
	FrameRect( lpDrawItem->hDC, &lpDrawItem->rcItem,
		(HBRUSH) GetStockObject( BLACK_BRUSH ) );
	
	SetTextColor( lpDrawItem->hDC, RGB( 100, 100, 100 ) );
	SetBkMode( lpDrawItem->hDC, TRANSPARENT );

	switch( lpDrawItem->CtlID ) {
		case ID_BUTTON_NEW:
			DrawText( lpDrawItem->hDC, TEXT( "New" ), -1, &lpDrawItem->rcItem,
				DT_CENTER | DT_VCENTER | DT_SINGLELINE | DT_NOCLIP );
			break;
		
		case ID_BUTTON_OPTIONS:
			DrawText( lpDrawItem->hDC, TEXT( "Options" ), -1, &lpDrawItem->rcItem,
				DT_CENTER | DT_VCENTER | DT_SINGLELINE | DT_NOCLIP);
			break;
		
		case ID_BUTTON_ENDGAME:
			DrawText( lpDrawItem->hDC, TEXT( "End Game" ), -1, &lpDrawItem->rcItem,
				DT_CENTER | DT_VCENTER | DT_SINGLELINE | DT_NOCLIP);
			break;
		
		case ID_BUTTON_QUIT:
			DrawText( lpDrawItem->hDC, TEXT( "x" ), -1, &lpDrawItem->rcItem,
				DT_CENTER | DT_VCENTER | DT_SINGLELINE | DT_NOCLIP);
			break;
		
		case ID_BUTTON_MINIMIZE:
			DrawText( lpDrawItem->hDC, TEXT( "-" ), -1, &lpDrawItem->rcItem,
				DT_CENTER | DT_VCENTER | DT_SINGLELINE | DT_NOCLIP);
			break;
	}
	
	if( lpDrawItem->itemState & ODS_SELECTED )
		InvertRect( lpDrawItem->hDC, &lpDrawItem->rcItem );
}


/*
=====================
	Cls_OnEraseBkgnd
=====================
*/
BOOL Cls_OnEraseBkgnd(HWND hwnd, HDC hdc) {
	return TRUE;
}


/*
=====================
	Cls_OnCommand
=====================
*/
void Cls_OnCommand(HWND hWnd, int id, HWND hwndCtl, UINT codeNotify) {
	static UINT res;
	int t;

	switch( id ) {
		case ID_BUTTON_NEW:
			t = DialogBoxParam( GetModuleHandle( NULL ), MAKEINTRESOURCE( IDD_DIALOG_NEW ),
				hWnd, NewDlgProc, &res);
			
			if( t==0 ) { //user pressed OK

				endAllGames();

				if( res & NEWGAME_RECORD ) {
					k_system.flags |= SF_RECORDGAME;
				}

				if( res & NEWGAME_SINGLE ) {
					singleGameStart();
				}
				else if( res & NEWGAME_VS ) {
					vsGameStart();
				}
				else if( res & NEWGAME_BOT ) {
					botGameStart();
				}
			}
			break;
		
		case ID_BUTTON_QUIT:
			PostQuitMessage( 0 );
			break;
		
		case ID_BUTTON_MINIMIZE:
			ShowWindow( hWnd, SW_MINIMIZE );
			break;
		
		case ID_BUTTON_OPTIONS:
			DialogBox( GetModuleHandle( NULL ), MAKEINTRESOURCE( IDD_DIALOG_OPTIONS ),
				hWnd, OptionsDlgProc );
			break;
		
		case ID_BUTTON_ENDGAME:
			endAllGames();
			break;
	}
	SetFocus( hWnd );
}


/*
=====================
	Cls_OnPaint
=====================
*/
void Cls_OnPaint( HWND hwnd ) {
	PAINTSTRUCT ps;
	HBITMAP hOldBitmap;
	HDC hDCSkin;
	HDC hDC;
	
	hDC = BeginPaint( hwnd, &ps );

	hDCSkin = CreateCompatibleDC( hDC );
	
	hOldBitmap = (HBITMAP) SelectObject( hDCSkin, k_system.hSkinBitmapLeft );
	BitBlt( hDC, 0, 0, k_system.cxSkinLeft, k_system.cySkinLeft, hDCSkin, 0, 0, SRCCOPY );

	SelectObject( hDCSkin, hOldBitmap );
	DeleteDC( hDCSkin );

	EndPaint( hwnd, &ps );
	
	//when window is cleaned-up, the score is getting screwed up
	drawScore( LEFTGAME );
}


/*
=====================
	Cls_OnKey
	-----------------
	Key processing
=====================
*/
void Cls_OnKey(HWND hwnd, UINT vk, BOOL fDown, int cRepeat, UINT flags) {
	int keyflags;

	//high priority keys
	//
	switch( vk ) {
		//ESC
		case VK_ESCAPE:
				//if any game is running, end it
				if( k_system.gameType > GNO ) {
					endAllGames();
				}
				//no game is running, so quit Netrix
				else {
					PostQuitMessage( 0 );
				}
				break;

		case VK_OEM_3:	// ~ (tilda)
			//hide 
			if( kbConsoleVisible == FALSE ) {
				ShowWindow( hDlgConsole, SW_SHOW );
				kbConsoleVisible = TRUE;
			}
			else {
				//bring console to front
				SetActiveWindow( hDlgConsole );
			}
			break;
	}

	//movement keys
	//
	if( (k_system.gameType >= GSINGLE) && !FLAG(k_system.flags, SF_GAMEOVER) &&
			!FLAG(k_system.flags, SF_PAUSE) && !FLAG( k_system.flags, SF_DEMOPLAY) ) {
		
		if( flags & 0x4000 ) {
			keyflags |= KF_REPEATING;
		}
		
		KeyProc( vk, keyflags );
		seqProc();
	}
		

	//misc keys
	//
	switch( vk ) {

			// Pause

		case VK_PAUSE:
		case 80:	// 'P'
			pause();
			updateWindow( CGF_DRAWLEFT );
			break;
	
/*			// 'S'
		case 83:
			gameSave();
			break;

			// 'L'
		case 76:
			startSavedGame();
			break;
*/
	}
	
}


/*
=====================
	Cls_OnActivate
=====================
*/
void Cls_OnActivate(HWND hwnd, UINT state, HWND hwndActDeact, BOOL fMinimized) {
	if( k_system.gameType > GNO ) {
		if( fMinimized && !k_system.pause )
			pause();
	}

	return;
}


/*
=====================
	Cls_OnDestroy
=====================
*/
void Cls_OnDestroy( HWND hwnd ) {
	PostQuitMessage( 0 );
}


/*
=====================
	createWindow
	-----------------
	Creates the Netrix main window
	located in k_system.hwnd
=====================
*/
BOOL createWindow() {
	int x, y;
	int width, height;
	WNDCLASS wnd;
	
	wnd.style = CS_HREDRAW | CS_VREDRAW;
	wnd.cbWndExtra = 0;
	wnd.cbClsExtra = 0;
	wnd.hInstance = GetModuleHandle( NULL );
	wnd.lpfnWndProc = WndProc;
	wnd.hCursor = LoadCursor( NULL, IDC_ARROW );
	wnd.hIcon = LoadIcon( GetModuleHandle( NULL ), MAKEINTRESOURCE( IDI_ICON ) );
	wnd.hbrBackground = NULL;//GetStockObject( DKGRAY_BRUSH );
	wnd.lpszClassName = TEXT( "Netrix" );
	wnd.lpszMenuName = NULL;
	
	if( !RegisterClass( &wnd ) ) {
		return FALSE;
	}

	width = k_system.cxSkinLeft;
	height = k_system.cySkinLeft;
	
	// center the window on the screen
	x = ( GetSystemMetrics( SM_CXSCREEN ) - width ) >> 1;
	y = ( ( GetSystemMetrics( SM_CYSCREEN ) - height ) >> 1 ) - 50;
	
	k_system.hwnd = CreateWindow( TEXT( "Netrix" ), TEXT( " - Netrix -" ),
		WS_POPUP | WS_VISIBLE | WS_CLIPCHILDREN, x, y, width, height,
		NULL, NULL, GetModuleHandle( NULL ), NULL );
	
	ShowWindow( k_system.hwnd, SW_SHOWNORMAL );
	UpdateWindow( k_system.hwnd );
	
	
	
	return TRUE;
}


/*
=====================
	WndProc
=====================
*/
LRESULT CALLBACK WndProc( HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam ) {
	switch( msg ) {
		HANDLE_MSG( hwnd,	WM_CREATE,		Cls_OnCreate );
		HANDLE_MSG( hwnd,	WM_PAINT,		Cls_OnPaint );
		HANDLE_MSG( hwnd,	WM_DESTROY,		Cls_OnDestroy );
		HANDLE_MSG( hwnd,	WM_KEYDOWN,		Cls_OnKey );
		HANDLE_MSG( hwnd,	WM_ACTIVATE,	Cls_OnActivate );
		HANDLE_MSG( hwnd,	WM_COMMAND,		Cls_OnCommand );
		HANDLE_MSG( hwnd,	WM_LBUTTONDOWN, Cls_OnLButtonDown );
		HANDLE_MSG( hwnd,	WM_LBUTTONUP,	Cls_OnLButtonUp );
		HANDLE_MSG( hwnd,	WM_ERASEBKGND,	Cls_OnEraseBkgnd );
		HANDLE_MSG( hwnd,	WM_DRAWITEM,	Cls_OnDrawItem );
		HANDLE_MSG( hwnd,	WM_MOUSEMOVE,	Cls_OnMouseMove );
		HANDLE_MSG( hwnd,	WM_KILLFOCUS,	Cls_OnKillFocus );
	}
	
	return DefWindowProc( hwnd, msg, wParam, lParam );
}


/*
=====================
	WinMain
=====================
*/
int WINAPI WinMain( HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR szCmdLine, int iCmdShow ) {
	DWORD oldTime;
	MSG msg;

	//check if already running
	if( AlreadyRunning() ) {
		MessageBox( NULL, TEXT("Netrix is already running!"), TEXT(""), 0 );
		return 1;
	}

	__try {
			
		if( !initSystem() ) {
			__leave;
		}

		while( TRUE ) {
		
			//pump the message loop
			while( PeekMessage( &msg, NULL, 0, 0, PM_NOREMOVE ) ) {
				if( !GetMessage( &msg, NULL, 0, 0 ) ) {
					__leave;
				}
				TranslateMessage( &msg );
				DispatchMessage( &msg );
			}

			Sleep(0);
			
			//process time slices
			//
			if( !k_system.pause ) {
				oldTime = k_system.dwTime;
				k_system.dwTime = timeGetTime();
				if( (k_system.dwTime - oldTime) >= 1 ) {
					k_system.dwAccumTime++;
					parseTimeSlices();
				}
			}
		}
	}

	__finally {
		destroySystem();
		ExitProcess( 0 );
	}
}