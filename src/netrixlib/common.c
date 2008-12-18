
#undef __N_FILE__
#define __N_FILE__ TEXT( "common.c" )


#include "compile.h"
#include <windows.h>
#include <tchar.h>
#include <commctrl.h>
#include <commdlg.h>
#include <stdio.h>

#include "common.h"
#include "libc.h"

#ifdef ENABLE_LOG


#define BUFF_SIZE 200

TCHAR kTraceBuff[TRACE_BUFF_SIZE] = {0};

HANDLE khLog = INVALID_HANDLE_VALUE;

/*
=====================
	_N_Trace
=====================
*/
void _N_Trace( TCHAR *szFormat, ... ) {
	TCHAR buff1[BUFF_SIZE] = {0};
	TCHAR buff2[BUFF_SIZE + TRACE_BUFF_SIZE] = {0};
	va_list va = NULL;

	if( khLog == INVALID_HANDLE_VALUE )
		return;

	//print message
	va_start( va, szFormat );
	_vsntprintf( buff1, BUFF_SIZE, szFormat, va );
	va_end( va );
	
	//combine source code info and message
	N_Sprintf( buff2, BUFF_SIZE + TRACE_BUFF_SIZE,
		TEXT("%s %s\r\n"), kTraceBuff, buff1 );
	
	N_FWrite( khLog, buff2, lstrlen( buff2 ) );
}

#endif //ENABLE_LOG


/*
=====================
	N_InitTrace
=====================
*/
void N_InitTrace() {

#ifdef ENABLE_LOG
	khLog = CreateFile( TEXT( "log.txt" ), GENERIC_WRITE,
		FILE_SHARE_READ, NULL, CREATE_ALWAYS,
		FILE_ATTRIBUTE_NORMAL | FILE_FLAG_WRITE_THROUGH, NULL );
#endif
	
}


/*
=====================
	N_CloseTrace
=====================
*/
void N_CloseTrace() {
#ifdef ENABLE_LOG
	N_FClose( khLog );
#endif
}


/*
=====================
	GetLoadPath
=====================
*/
BOOL GetLoadPath( HWND hWnd, TCHAR *filter, TCHAR *ext, TCHAR *szPath ) {
	OPENFILENAME ofn = {0};
	TCHAR szStartDir[MAX_PATH];
	
	//check argument validity
	//
	if( (!filter) || (!ext) || (!szPath) ) {
		return FALSE;
	}
	
	//save current directory
	//
	GetCurrentDirectory( MAX_PATH, szStartDir );
	
	
	szPath[0] = '\0';
	
	ofn.lStructSize = sizeof( ofn );
	ofn.hwndOwner = hWnd;
	ofn.lpstrFilter = filter;
	ofn.lpstrFile = szPath;
	ofn.nMaxFile = MAX_PATH;
	ofn.lpstrTitle = TEXT( "Load" );
	ofn.lpstrDefExt = ext;
	ofn.Flags = OFN_EXPLORER | OFN_PATHMUSTEXIST;
	
	if( !GetOpenFileName( &ofn ) ) {
		return FALSE;
	}
	
	//restore current directory
	//
	SetCurrentDirectory( szStartDir );

	return TRUE;
}


/*
=====================
	GetSavePath
=====================
*/
BOOL GetSavePath( HWND hWnd, TCHAR *filter, TCHAR *ext, TCHAR *szPath ) {
	OPENFILENAME ofn = {0};
	TCHAR szStartDir[MAX_PATH];
	
	//check argument validity
	//
	if( (!filter) || (!ext) || (!szPath) ) {
		return FALSE;
	}
	
	//save current directory
	//
	GetCurrentDirectory( MAX_PATH, szStartDir );
	

	szPath[0] = '\0';
	
	ofn.lStructSize = sizeof( ofn );
	ofn.hwndOwner = hWnd;
	ofn.lpstrFilter = filter;
	ofn.lpstrFile = szPath;
	ofn.nMaxFile = MAX_PATH;
	ofn.lpstrTitle = TEXT( "Save" );
	ofn.lpstrDefExt = ext;
	ofn.Flags = OFN_EXPLORER | OFN_PATHMUSTEXIST | OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT;
	
	if( !GetSaveFileName( &ofn ) ) {
		return FALSE;
	}

	//restory current directory
	//
	SetCurrentDirectory( szStartDir );

	return TRUE;
}