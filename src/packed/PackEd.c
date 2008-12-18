
//PackEd - Netrix Pack editor
//

#undef __N_FILE__
#define __N_FILE__ TEXT( "PackEd.c" )

#include <windows.h>
#include <shlobj.h>

#include "../netrixlib/netrixlib.h"
#include "PackEd.h"

#include "resource.h"


/*
=====================
	WinMain
=====================
*/
int WINAPI WinMain( HINSTANCE hInst, HINSTANCE hPrevInst, LPSTR szCmdLine, int nCmdShow ) {

	DialogBox( hInst, MAKEINTRESOURCE( IDD_DIALOG_PACKED ),
		NULL, (DLGPROC)DlgPackEdProc );
	
	return 0;
}


/*
=====================
	DlgPackEdProc
=====================
*/
BOOL CALLBACK DlgPackEdProc( HWND hWndDlg, UINT msg, WPARAM wParam, LPARAM lParam ) {
	RECT rc;
	int width, height;

	switch( msg ) {
		case WM_INITDIALOG:
			//center the dialog box
			//
			GetWindowRect( hWndDlg, &rc );
			width = rc.right - rc.left;
			height = rc.bottom - rc.top;
			rc.left = (GetSystemMetrics( SM_CXSCREEN ) - width) >> 1;
			rc.top = ((GetSystemMetrics( SM_CYSCREEN ) - height) >> 1) - 50;
			MoveWindow( hWndDlg, rc.left, rc.top, width, height, TRUE );
			break;
		
		case WM_COMMAND:
			switch( LOWORD( wParam ) ) {
				case IDC_BUTTON_BUILDPACK:
					packEdBuild( hWndDlg );
					break;
				
				case IDC_BUTTON_EXTRACTPACK:
					packEdExtract( hWndDlg );
					break;
			}
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
	packEdBuild
=====================
*/
void packEdBuild( HWND hWnd ) {
	BROWSEINFO bi = {0};
	ITEMIDLIST *pItem;
	TCHAR szDir[MAX_PATH]= {0};
	TCHAR szPack[MAX_PATH];
	
	__try {
	
		bi.hwndOwner = hWnd;
		bi.pidlRoot = NULL;
		bi.pszDisplayName = szDir;
		bi.lpszTitle = TEXT( "Select directory to Pack" );
		bi.ulFlags = 0;
		bi.lParam = 0;
		
		pItem = SHBrowseForFolder( &bi );
		
		if( !pItem )
			__leave;
			
		if( !SHGetPathFromIDList( pItem, szDir ) )
			__leave;
		
		GetSavePath( hWnd, TEXT( "(*.npk)\0*.npk\0" ), TEXT( "*.npk" ), szPack );
		
		npkBuild( szDir, szPack );
	}
	
	__finally {
	}
}


/*
=====================
	packEdExtract
=====================
*/
void packEdExtract( HWND hWnd ) {
	BROWSEINFO bi = {0};
	ITEMIDLIST *pItem;
	TCHAR szDir[MAX_PATH] = {0};
	TCHAR szPack[MAX_PATH];
	
	__try {
		pItem = NULL;
		
		GetLoadPath( hWnd, TEXT( "(*.npk)\0*.npk" ), TEXT( "*.npk" ), szPack );
		
		bi.hwndOwner = hWnd;
		bi.pidlRoot = NULL;
		bi.pszDisplayName = szDir;
		bi.lpszTitle = TEXT( "Select directory to unpack to" );
		bi.ulFlags = 0;
		bi.lParam = 0;
		
		pItem = SHBrowseForFolder( &bi );

		if( !pItem )
			__leave;
		
		if( !SHGetPathFromIDList( pItem, szDir ) )
			__leave;

		npkExtract( szPack, szDir );
	}
	
	__finally {
	}
}