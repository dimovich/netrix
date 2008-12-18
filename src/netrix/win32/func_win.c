
// Win32 API wrapper code
//

#undef __N_FILE__
#define __N_FILE__ TEXT( "func_win.c" )

#include "../compile.h"
#include <windows.h>
#include <commdlg.h>
#include <commctrl.h>
#include <tchar.h>
#include <stdio.h>

#include "../../netrixlib/netrixlib.h"

#include "../game/sys.h"
#include "../game/replay.h"
#include "../game/func.h"
#include "func_win.h"

#include "../resource.h"


#define ID_LEFTAS 1
#define ID_RIGHTAS 2
#define ID_LEFTNFS 3
#define ID_RIGHTNFS 4
#define ID_LEFTSCORE 5
#define ID_RIGHTSCORE 6


#define AS_CLASS TEXT( "Generic_ASS" )
#define NFS_CLASS TEXT( "Generic_NFS" )


/*
=====================
	OsTypeNT
=====================
*/
BOOL OsTypeNT() {
	OSVERSIONINFOA vi;
	vi.dwOSVersionInfoSize = sizeof( OSVERSIONINFOA );
	GetVersionExA( &vi );
	if( vi.dwPlatformId == VER_PLATFORM_WIN32_NT )
		return TRUE;
	return FALSE;
}


/*
=====================
	AlreadyRunning
=====================
*/
BOOL AlreadyRunning() {
	CreateMutex( NULL, TRUE, TEXT("NETRIX") );
	return (GetLastError() == ERROR_ALREADY_EXISTS);
}


/*
=====================
	initWin32
=====================
*/
BOOL initWin32() {
	INITCOMMONCONTROLSEX iccex;
	
	iccex.dwSize = sizeof(iccex);
	iccex.dwICC = ICC_INTERNET_CLASSES;
	
	InitCommonControlsEx( &iccex );
	
	return TRUE;
}


/*
=====================
	MassEnable
=====================
*/
void MassEnable( HWND hWnd, int idFirst, int idLast, BOOL f ) {
	int i;
	HWND hWndCtl;
	
	//check arguments validity
	if( (idFirst < 0) || (idLast < 0) || (f < 0) ) {
		return;
	}
	
	for( i=idFirst; i<=idLast; i++ ){
		hWndCtl = GetDlgItem( hWnd, i );
		if( hWndCtl != NULL ) {
			EnableWindow( hWndCtl, f );
		}
	}
}


// main window controls

HWND hwndCtlNew;
HWND hwndCtlOptions;
HWND hwndCtlAbout;
HWND hwndCtlLoad;
HWND hwndCtlSave;
HWND hwndCtlPause;
HWND hwndCtlEndgame;
HWND hwndCtlQuit;
HWND hwndCtlMinimize;

WNDPROC OldBtnProc;


/*
=====================
	populateGUI
=====================
*/
BOOL populateGUI() {
	HINSTANCE hInst;
	WNDCLASS wnd = {0};
	
	hInst = (HINSTANCE) GetWindowLong( k_system.hwnd, GWL_HINSTANCE );
	
	//register AS and NFS window classes
	
	//ASS'es :)
	wnd.style = CS_OWNDC;
	wnd.hInstance = hInst;
	wnd.lpfnWndProc = WndProcAS;
	wnd.lpszClassName = AS_CLASS;
	
	if( !RegisterClass( &wnd ) ) {
		N_Trace( TEXT( "Could not register window class\n" )
				TEXT( "GetLastError() returned: %d\n" ), GetLastError() );
	}

	//NFS'es
	wnd.lpfnWndProc = WndProcNFS;
	wnd.lpszClassName = NFS_CLASS;

	if( !RegisterClass( &wnd ) ) {
		N_Trace( TEXT( "Could not register window class\n" )
				TEXT( "GetLastError() returned: %d\n" ), GetLastError() );
	}

	//Create left NFS and AS
	//
	populateLeftGUI();

	//Create controls
	//

	// New
	hwndCtlNew = CreateWindow( TEXT( "button" ), TEXT( "New" ),
		WS_CHILDWINDOW | WS_VISIBLE | BS_OWNERDRAW, 120, 70, 70, 25,
		k_system.hwnd, (HMENU) ID_BUTTON_NEW, hInst, NULL );
	
	OldBtnProc = SetWindowLong( hwndCtlNew, GWL_WNDPROC, OwnBtnProc );
	
	// Options
	hwndCtlOptions = CreateWindow( TEXT( "button" ), TEXT( "Options" ),
		WS_CHILDWINDOW | WS_VISIBLE | BS_OWNERDRAW, 200, 70, 70, 25,
		k_system.hwnd, (HMENU) ID_BUTTON_OPTIONS, hInst, NULL );

	SetWindowLong( hwndCtlOptions, GWL_WNDPROC, OwnBtnProc );
	
	// X (quit)
	hwndCtlQuit = CreateWindow( TEXT( "button" ), NULL,
		WS_CHILDWINDOW | WS_VISIBLE | BS_OWNERDRAW, 380, 24, 18, 18,
		k_system.hwnd, (HMENU) ID_BUTTON_QUIT, hInst, NULL );
	
	SetWindowLong( hwndCtlQuit, GWL_WNDPROC, OwnBtnProc );

	// - (minimize)
	hwndCtlMinimize = CreateWindow( TEXT( "button" ), NULL,
		WS_CHILDWINDOW | WS_VISIBLE | BS_OWNERDRAW, 355, 24, 18, 18,
		k_system.hwnd, (HMENU) ID_BUTTON_MINIMIZE, hInst, NULL );
	
	SetWindowLong( hwndCtlMinimize, GWL_WNDPROC, OwnBtnProc );
	
	// End Game
	hwndCtlEndgame = CreateWindow( TEXT( "button" ), TEXT( "End Game" ),
		WS_CHILDWINDOW | BS_OWNERDRAW, 340, 250, 80, 25,
		k_system.hwnd, (HMENU)ID_BUTTON_ENDGAME, hInst, NULL );
	
	SetWindowLong( hwndCtlEndgame, GWL_WNDPROC, OwnBtnProc );
	

	return TRUE;
}


/*
=====================
	populateLeftGUI
=====================
*/
BOOL populateLeftGUI() {
	HINSTANCE hInst;
	
	hInst = (HINSTANCE) GetWindowLong( k_system.hwnd, GWL_HINSTANCE );

	//Left AS
	//
	k_system.hwndLeft = CreateWindow( AS_CLASS, NULL,
		WS_CHILD | WS_VISIBLE | WS_CLIPCHILDREN | WS_CLIPSIBLINGS ,
		LEFTASPOSX, LEFTASPOSY, CXASWINDOW+2, CYASWINDOW+2,
		k_system.hwnd, (HMENU) ID_LEFTAS, hInst, NULL );
	
	if( !k_system.hwndLeft ) {
		N_Trace( TEXT( "Could not create window.\n" )
				TEXT( "GetLastError() returned: %d" ), GetLastError() );
	}


	//Left NFS
	//
	k_system.hwndLeftN = CreateWindow( NFS_CLASS, NULL,
		WS_CHILD | WS_CLIPCHILDREN | WS_CLIPSIBLINGS,
		LEFTNFSPOSX, LEFTNFSPOSY, CXNFSWINDOW, CYNFSWINDOW,
		k_system.hwnd, (HMENU) ID_LEFTNFS, hInst, NULL );
	
	if( !k_system.hwndLeftN ) {
		N_Trace( TEXT( "Could not create window.\n" )
				TEXT( "GetLastError() returned: %d" ), GetLastError() );
	}
	
	//Left Score
	//
	k_system.hWndScoreLeft = CreateWindow( TEXT( "static" ), NULL,
		WS_CHILD, LSCOREX, LSCOREY, SCORECX, SCORECY,
		k_system.hwnd, (HMENU) ID_LEFTSCORE, hInst, NULL );
	if( !k_system.hWndScoreLeft ) {
		N_Trace( TEXT( "Could not create window.\n" )
				TEXT( "GetLastError() returned: %d" ), GetLastError() );
	}
	SetWindowLong( k_system.hWndScoreLeft, GWL_WNDPROC, WndProcScore );

	return TRUE;
}


/*
=====================
	WndProcAS
=====================
*/
LRESULT CALLBACK WndProcAS( HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam ) {

	switch( msg ) {
		case WM_PAINT:

			if( hWnd == k_system.hwndLeft ) {
				updateWindow( CGF_DRAWLEFT );
			}
			else if( hWnd == k_system.hwndRight ) {
				updateWindow( CGF_DRAWRIGHT );
			}
			
			ValidateRect( hWnd, NULL );
			break;

		default:
			return DefWindowProc( hWnd, msg, wParam, lParam );
	}
	return 0L;
}


/*
=====================
	WndProcNFS
=====================
*/
LRESULT CALLBACK WndProcNFS( HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam ) {

	switch( msg ) {
		case WM_PAINT:

			if( hWnd == k_system.hwndLeftN ) {
				drawNFS( LEFTGAME );
			}
			else if( hWnd == k_system.hwndRightN ) {
				drawNFS( RIGHTGAME );
			}

			ValidateRect( hWnd, NULL );

			break;

		default:
			return DefWindowProc( hWnd, msg, wParam, lParam );
	}
	return 0L;
}


/*
=====================
	WndProcScore
=====================
*/
LRESULT CALLBACK WndProcScore( HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam ) {
	switch( msg ) {
		case WM_PAINT:
			if( hWnd == k_system.hWndScoreLeft ) {
				drawScore( LEFTGAME );
			}
			else if( hWnd == k_system.hWndScoreRight ) {
				drawScore( RIGHTGAME );
			}
			ValidateRect( hWnd, NULL );
			break;
		default:
			return DefWindowProc( hWnd, msg, wParam, lParam );
	}
	
	return 0L;
}


HWND hWndDlgRight;
/*
=====================
	populateRightGUI
=====================
*/
BOOL populateRightGUI() {
	
	hWndDlgRight = CreateDialog( GetModuleHandle( NULL ),
		MAKEINTRESOURCE( IDD_DIALOG_RIGHT ), k_system.hwnd, RightDlgProc );
	
	return TRUE;
}


/*
=====================
	un_populateRightGUI
=====================
*/
BOOL un_populateRightGUI() {
	SendMessage( hWndDlgRight, WM_CLOSE, 0, 0 );
	hWndDlgRight = NULL;
	return TRUE;
}


/*
=====================
	RightDlgProc
=====================
*/
BOOL CALLBACK RightDlgProc( HWND hWndDlg, UINT msg, WPARAM wParam, LPARAM lParam ) {
	int width, height;
	RECT rc;
	int delta;
	int cxScreen;
	PAINTSTRUCT ps;
	HBITMAP hOldBitmap;
	HDC hDCSkin;
	HDC hDC;
	HRGN hRgn;

	switch( msg ) {
		case WM_INITDIALOG:
		
			//Move windows so that they fit
			//corectly on the screen.
			//

			cxScreen = GetSystemMetrics( SM_CXSCREEN );
			
			//Get main window coordinates.
			//
			GetWindowRect( k_system.hwnd, &rc );
			width = rc.right - rc.left;
			height = rc.bottom - rc.top;

			if( rc.right > (cxScreen/2) ) {
				delta = rc.right - (cxScreen/2);
				rc.left -= delta;
				rc.right -= delta;
				MoveWindow( k_system.hwnd, rc.left, rc.top, width, height, TRUE );
			}
			
			//Set dialog position
			//
			rc.left += width - 10;

			//right window
			//
			MoveWindow( hWndDlg, rc.left, rc.top+LEFTASPOSY-50,
				k_system.cxSkinRight, k_system.cySkinRight, TRUE );
			
			//Right AS
			//
			k_system.hwndRight = CreateWindow( AS_CLASS, NULL,
				WS_VISIBLE | WS_CHILDWINDOW | WS_CLIPSIBLINGS | WS_CLIPCHILDREN,
				RIGHTASPOSX, RIGHTASPOSY, CXASWINDOW, CYASWINDOW,
				hWndDlg, (HMENU) ID_RIGHTAS, GetModuleHandle( NULL ), NULL );
			
			//Right NFS
			k_system.hwndRightN = CreateWindow( NFS_CLASS, NULL,
				WS_VISIBLE | WS_CHILD | WS_CLIPCHILDREN | WS_CLIPSIBLINGS,
				RIGHTNFSPOSX, RIGHTNFSPOSY, CXNFSWINDOW, CYNFSWINDOW,
				hWndDlg, (HMENU) ID_RIGHTNFS, GetModuleHandle( NULL ), NULL );
			
			//Right Score
			k_system.hWndScoreRight = CreateWindow( TEXT( "static" ), NULL,
				WS_CHILD, RSCOREX, RSCOREY, SCORECX, SCORECY,
				hWndDlg, (HMENU) ID_RIGHTSCORE, GetModuleHandle( NULL ), NULL );
			SetWindowLong( k_system.hWndScoreRight, GWL_WNDPROC, (LONG)WndProcScore );
			
			//Set window region.
			//NOTE: It will automatically  be destroyed
			//	when window is destroyed.
			//
			hRgn = CreateRectRgn( 0, 0, 0, 0 );
			CombineRgn( hRgn, k_system.hSkinRgnRight, hRgn, RGN_OR );
			SetWindowRgn( hWndDlg, hRgn, TRUE );
			break;

		case WM_SETFOCUS:
			SetFocus( k_system.hwnd );
			break;
		
		case WM_PAINT:
			hDC = BeginPaint( hWndDlg, &ps );
	
			hDCSkin = CreateCompatibleDC( hDC );
			hOldBitmap = (HBITMAP) SelectObject( hDCSkin, k_system.hSkinBitmapRight );
			BitBlt( hDC, 0, 0, k_system.cxSkinRight, k_system.cySkinRight, hDCSkin, 0, 0, SRCCOPY );

			SelectObject( hDCSkin, hOldBitmap );
			EndPaint( hWndDlg, &ps );
			DeleteDC( hDCSkin );
	
			updateWindow( CGF_DRAWRIGHT );
			break;
		
		case WM_CLOSE:
			DestroyWindow( hWndDlg );
			break;

		default:
			return FALSE;
	}
	return TRUE;
}


/*
=====================
	loadSkin
=====================
*/
BOOL loadSkin( HRGN *phSkinRgn, HBITMAP *phSkin, int *cx, int *cy, int idRes ) {
	HGLOBAL hRes;
	HINSTANCE hInst;
	DWORD *pMem;
	DWORD *texture;
	DWORD width, height;
	HDC hDC, hDCRef;
	HBITMAP hBitmap;
	HBITMAP hOldBitmap;
	int xstart, xindex, yindex;
	HRGN hTmpRgn;
	HRGN skinrgn;
	COLORREF crTrans = RGB( 255*20/100, 255*20/100, 255*40/100 );
	BYTE r, g, b;

	//check arguments validity
	if( (phSkinRgn==NULL) || (phSkin==NULL) || (cx==NULL) || (cy==NULL) ) {
		return FALSE;
	}

	//Load IMG image

	hInst = GetModuleHandle( NULL );
	hRes = LoadResource( hInst,
		FindResource( hInst, MAKEINTRESOURCE( idRes ),
			TEXT( "SKIN" ) ) );
	
	//check resource validity
	if( hRes == NULL ) {
		return FALSE;
	}

	pMem = (DWORD *) LockResource( hRes );

	//Decode IMG to BMP
	decodeIMG( pMem, &texture, &width, &height );

	hDC = GetDC( NULL );
	hDCRef = CreateCompatibleDC( NULL );
	hBitmap = CreateCompatibleBitmap( hDC, width, height );
	SetBitmapBits( hBitmap, width*height*sizeof( DWORD ), (BYTE *) texture );
	
	*phSkin = hBitmap;
	*cx = width;
	*cy = height;


	//Create window region based on skin image.
	//
	//NOTE: color bytes are saved as BGR,
	//		so we have to interchange B and R.
	//

	hOldBitmap = (HBITMAP) SelectObject( hDCRef, hBitmap );
	skinrgn = CreateRectRgn( 0, 0, 0, 0 );
	
	for( yindex = 1; yindex < height-1; yindex++ ) {
		for( xindex = 1; xindex < width-1; xindex++ ) {

			r = GetBValue( texture[yindex*width + xindex] );
			g = GetGValue( texture[yindex*width + xindex] );
			b = GetRValue( texture[yindex*width + xindex] );

			if( RGB( r, g, b ) != crTrans ) { //not a transparent color
				xstart = xindex;

				while( xindex < width ) {

					r = GetBValue( texture[yindex*width + xindex] );
					g = GetGValue( texture[yindex*width + xindex] );
					b = GetRValue( texture[yindex*width + xindex] );

					if( RGB( r, g, b ) == crTrans )
						break;

					xindex++;
				}
				
				hTmpRgn = CreateRectRgn( xstart, yindex, xindex, yindex+1 );
				CombineRgn( skinrgn, skinrgn, hTmpRgn, RGN_OR );
				DeleteObject( hTmpRgn );
			}
		}
	}
	
	*phSkinRgn = skinrgn;

	SelectObject( hDCRef, hOldBitmap );
	ReleaseDC( 0, hDC );
	DeleteDC( hDCRef );
	N_Free( texture );
	return TRUE;
}


/*
=====================
	OptionsDlgProc
=====================
*/
BOOL CALLBACK OptionsDlgProc( HWND hWndDlg, UINT msg, WPARAM wParam, LPARAM lParam ) {

	switch( msg ) {
		case WM_INITDIALOG:
			break;
		
		case WM_COMMAND:
			switch( LOWORD( wParam ) ) {
				
				case IDC_BUTTON_LOADDEMO:
					playDemo();
					EndDialog( hWndDlg, 0 );
					break;

				case IDOK:
					EndDialog( hWndDlg, 0 );
					break;
				
				case IDCANCEL:
					EndDialog( hWndDlg, 1 );
					break;
				
				default:
					break;
			}
			break;
		
		default:
			return FALSE;
	}
	
	return TRUE;
}



/*
=====================
	NewDlgProc
	-----------------
	a pain in the ass...
=====================
*/
BOOL CALLBACK NewDlgProc( HWND hWndDlg, UINT msg, WPARAM wParam, LPARAM lParam ) {
	static UINT type = 0;		//game type flag
	static UINT settings = 0;	//settings flag
	static UINT *res = NULL;	//resultant flag

	switch( msg ) {
		case WM_INITDIALOG:
			CheckRadioButton( hWndDlg, IDC_RADIO_LOCAL, IDC_RADIO_NETWORK,
				IDC_RADIO_LOCAL );
			CheckRadioButton( hWndDlg, IDC_RADIO_SINGLE, IDC_RADIO_BOT,
				IDC_RADIO_SINGLE );
			CheckRadioButton( hWndDlg, IDC_RADIO_NETVS, IDC_RADIO_NETBOT,
				IDC_RADIO_NETVS );

			MassEnable( hWndDlg, IDC_RADIO_NETVS, IDC_RADIO_NETBOT, FALSE );
			MassEnable( hWndDlg, IDC_COMBO_BOT, IDC_CHECK_SERVER, FALSE );
			
			CheckDlgButton( hWndDlg, IDC_CHECK_IPADDRESS, BST_CHECKED );
			
			res = (UINT *)lParam;
			
			type = NEWGAME_SINGLE;
			settings = 0;
			
			populateMapComboBox( GetDlgItem( hWndDlg, IDC_COMBO_MAP ) );
			populateBotComboBox( GetDlgItem( hWndDlg, IDC_COMBO_BOT ) );
			
			break;


		case WM_COMMAND:

			switch( LOWORD( wParam ) ) {	//Dealing with radio checking on/off
				case IDOK:
					k_system.idMap = (int) SendMessage( GetDlgItem( hWndDlg, IDC_COMBO_MAP ),
						CB_GETCURSEL, 0, 0 );
					k_system.idMap--;
					
					k_system.idBotRight = (int) SendMessage( GetDlgItem( hWndDlg, IDC_COMBO_BOT ),
						CB_GETCURSEL, 0, 0 );
					
					if( IsDlgButtonChecked( hWndDlg, IDC_CHECK_RECORD ) ) {
						settings |= NEWGAME_RECORD;
					}

					*res = 0;
					*res = settings | type;
					EndDialog( hWndDlg, 0 );
					break;
				
				case IDCANCEL:
					EndDialog( hWndDlg, 1 );
					break;
				
				case IDC_RADIO_LOCAL:
				case IDC_RADIO_NETWORK:
					CheckRadioButton( hWndDlg, IDC_RADIO_LOCAL, IDC_RADIO_NETWORK,
						LOWORD( wParam ) );
					break;
				
				case IDC_RADIO_SINGLE:
				case IDC_RADIO_VS:
				case IDC_RADIO_BOT:
					CheckRadioButton( hWndDlg, IDC_RADIO_SINGLE, IDC_RADIO_BOT,
						LOWORD( wParam ) );
					break;
				
				case IDC_RADIO_NETVS:
				case IDC_RADIO_NETBOT:
					CheckRadioButton( hWndDlg, IDC_RADIO_NETVS, IDC_RADIO_NETBOT,
						LOWORD( wParam ) );
					break;
				
				default:
					break;
			}
			
			
			switch( LOWORD( wParam ) ) {	//Dealing with window enabling/disabling
											//depending on game type
				case IDC_RADIO_LOCAL:

					//enabling local game types
					MassEnable( hWndDlg, IDC_RADIO_SINGLE, IDC_RADIO_BOT, TRUE );

					//disabling network game types
					MassEnable( hWndDlg, IDC_RADIO_NETVS, IDC_RADIO_NETBOT, FALSE );

					//checking SINGLE as the default local game type
					CheckRadioButton( hWndDlg, IDC_RADIO_SINGLE, IDC_RADIO_BOT,
						IDC_RADIO_SINGLE );

		//fall through
				case IDC_RADIO_SINGLE:

					type = NEWGAME_SINGLE;

		//fall through
				case IDC_RADIO_VS:
	
					if( LOWORD( wParam ) == IDC_RADIO_VS )
						type = NEWGAME_VS;

					//disabling all BOT->SERVER settings
					MassEnable( hWndDlg, IDC_COMBO_BOT, IDC_CHECK_SERVER, FALSE );
					
					//enabling MAP combo box
					EnableWindow( GetDlgItem( hWndDlg, IDC_COMBO_MAP ), TRUE );
					break;

				case IDC_RADIO_BOT:
					
					type = NEWGAME_BOT;
				
					//disabling MAP combo box
					EnableWindow( GetDlgItem( hWndDlg, IDC_COMBO_MAP ), FALSE );
					
					//enabling BOT combo box
					EnableWindow( GetDlgItem( hWndDlg, IDC_COMBO_BOT ), TRUE );
					break;
				

				case IDC_RADIO_NETWORK:
					//enabling network game types
					MassEnable( hWndDlg, IDC_RADIO_NETVS, IDC_RADIO_NETBOT, TRUE );

					//disabling local game types
					MassEnable( hWndDlg, IDC_RADIO_SINGLE, IDC_RADIO_BOT, FALSE );

					//disabling all settings
					MassEnable( hWndDlg, IDC_COMBO_MAP, IDC_CHECK_SERVER, FALSE );

					//enabling IP Address input box
					MassEnable( hWndDlg, IDC_IPADDRESS, IDC_CHECK_IPADDRESS, TRUE );
					
					//checking IP Address checkbox
					CheckDlgButton( hWndDlg, IDC_CHECK_IPADDRESS, TRUE );

					//enabling Server checkbox and unchecking it
					EnableWindow( GetDlgItem( hWndDlg, IDC_CHECK_SERVER ), TRUE );
					if( IsDlgButtonChecked( hWndDlg, IDC_CHECK_SERVER ) )
						CheckDlgButton( hWndDlg, IDC_CHECK_SERVER, 0 );

					//checking NETVS as the default network game type
					CheckRadioButton( hWndDlg, IDC_RADIO_NETVS, IDC_RADIO_NETBOT,
						IDC_RADIO_NETVS );

		//fall through
				case IDC_RADIO_NETVS:
					
					type = NEWGAME_NETVS;
				
					//disable BOT combo box
					EnableWindow( GetDlgItem( hWndDlg, IDC_COMBO_BOT ), FALSE );
					break;
				
				case IDC_RADIO_NETBOT:
				
					type = NEWGAME_NETBOT;

					//enable BOT combo box
					EnableWindow( GetDlgItem( hWndDlg, IDC_COMBO_BOT ), TRUE );
					break;


				case IDC_CHECK_SERVER:
					//enabling/disabling SERVER combo box
					EnableWindow( GetDlgItem( hWndDlg, IDC_COMBO_SERVER ),
						IsDlgButtonChecked( hWndDlg, IDC_CHECK_SERVER ) );
		//fall through
				case IDC_CHECK_IPADDRESS:
					//enabling/disabling IPADDRESS input box
					EnableWindow( GetDlgItem( hWndDlg, IDC_IPADDRESS ),
						IsDlgButtonChecked( hWndDlg, IDC_CHECK_IPADDRESS ) );
					
					//determining network connection type
					if( IsDlgButtonChecked( hWndDlg,  IDC_CHECK_IPADDRESS ) ) {
						if( IsDlgButtonChecked( hWndDlg, IDC_CHECK_SERVER ) )
							settings = NEWGAME_THROUGH;
						else
							settings = NEWGAME_DIRECT;
					}
					else {
						if( IsDlgButtonChecked( hWndDlg, IDC_CHECK_SERVER ) )
							settings = NEWGAME_MEDIATED;
						else
							settings = 0;
					}
					
					break;
				
				default:
					break;
			}
			

			break;
		default:
			return FALSE;
	}
	
	return TRUE;
}


/*
=====================
	OwnBtnProc
	-----------------
	Hover effect for buttons
=====================
*/
LRESULT CALLBACK OwnBtnProc( HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam ) {
	switch( msg ) {
		case WM_MOUSEMOVE:
			SendMessage( hWnd, BM_SETSTATE, 1, 0 );
			return 0;
	}
	return CallWindowProc( OldBtnProc, hWnd, msg, wParam, lParam );
}


/*
=====================
	populateMapComboBox
=====================
*/
BOOL populateMapComboBox( HWND hWndCtl ) {
	int i;
	
	SendMessage( hWndCtl, CB_ADDSTRING, 0, (LPARAM) TEXT( "--- No Map ---" ) );
	
	for( i=0; i<k_system.cMaps; i++ ) {
		SendMessage( hWndCtl, CB_ADDSTRING, i+1, (LPARAM) k_system.pMaps[i].map.name );
	}
	
	SendMessage( hWndCtl, CB_SETCURSEL, 0, 0 );
	
	return TRUE;
}


/*
=====================
	populateBotComboBox
=====================
*/
BOOL populateBotComboBox( HWND hWndCtl ) {
	int i;
	
	if( k_system.cBots <= 0 ) {
		SendMessage( hWndCtl, CB_ADDSTRING, 0, (LPARAM)TEXT( "--- No Bot ---" ) );
	}
	else {
		for( i=0; i<k_system.cBots; i++ ) {
			SendMessage( hWndCtl, CB_ADDSTRING, i, (LPARAM)k_system.pBots[i].bot.name );
		}
	}
	
	SendMessage( hWndCtl, CB_SETCURSEL, 0, 0 );
	
	return TRUE;
}


/*
=====================
	N_Message
=====================
*/
void N_Message( TCHAR * szFormat, ... ) {
	HDC hDC;
	TCHAR szBuff[256];
	va_list v;
	
	va_start( v, szFormat );
	_vsntprintf( szBuff, 256, szFormat, v );
	va_end( v );
	
	hDC = GetDC( k_system.hwnd );
	SetBkMode( hDC, TRANSPARENT );
	TextOut( hDC, 30, LEFTASPOSY + CYASWINDOW+8, szBuff, N_Strlen( szBuff ) );
	ReleaseDC( k_system.hwnd, hDC );
	
	pushTimeSlice( TIME_SYSTEM_UPDATEWINDOW,  TIME_SYSTEM_UPDATEWINDOW_INTERVAL,
		TIME_SYSTEM_UPDATEWINDOW_INTERVAL, NULL, SYSTEM, FALSE );
}


/*
=====================
	doSystemAction
=====================
*/
void doSystemAction( timeSliceActions_t action ) {
	switch( action ) {
		case TIME_SYSTEM_UPDATEWINDOW:
			InvalidateRect( k_system.hwnd, NULL, FALSE );
			break;
	}
}