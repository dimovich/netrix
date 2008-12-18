
//Netrix Console
//
//Code parts adapted from:
//Quake3 source code (by id Software);

#undef __N_FILE__
#define __N_FILE__ TEXT( "console_win.c" )

#include "../compile.h"
#include <windows.h>
#include <tchar.h>
#include <stdio.h>

#include "../../netrixlib/netrixlib.h"

#include "console_win.h"
#include "../common/const.h"
#include "../game/sys.h"
#include "../game/game.h"

#include "../resource.h"


#define CON_STRING_SIZE		256
#define CON_NAME_SIZE		30
#define CON_INVALID			-1


//conCmd_t
//
typedef enum conCmd_e {
	CMD_SET,		//set config var
	CMD_SAVECONFIG,	//save config vars
	CMD_ABOUT,		//copyright info
	CMD_LOADRES,	//reload resources
	CMD_TESTBOT,	//test bot script
	CMD_HELP,		//list all commands
	CMD_QUIT,		//quit
	CMDLIST_SIZE
} conCmd_t;


//conVar_t
//
typedef struct conVar_s {
	conCmd_t cmd;
	TCHAR *name;
} conVar_t;


//conData_t
//
typedef struct conData_s {

	HWND hWnd;
	HWND hWndInputLine;
	HWND hWndBuffer;

	WNDPROC InputLineWndProc;

	HBRUSH	hbrBackground;

	TCHAR szCmd[CON_STRING_SIZE];
	TCHAR szRet[CON_STRING_SIZE];

} conData_t;


static conData_t g_conData;

extern BOOL kbConsoleVisible;

//init command list names
static conVar_t g_conVars[CMDLIST_SIZE] = {
	CMD_SET,		TEXT( "set" ),
	CMD_SAVECONFIG, TEXT( "saveconfig" ),
	CMD_ABOUT,		TEXT( "about" ),
	CMD_LOADRES,	TEXT( "loadres" ),
	CMD_TESTBOT,	TEXT( "testbot" ),
	CMD_HELP,		TEXT( "help" ),
	CMD_QUIT,		TEXT( "quit" )
};


//input command procedure
static LRESULT CALLBACK InputLineWndProc( HWND, UINT, WPARAM, LPARAM );

//parse console command
static int ParseCmd( TCHAR *szIn, TCHAR *szOut );

//returns command ID
static conCmd_t SeeCmd( TCHAR *szBuff );

//test a bot
static void TestBot( TCHAR **pszBuff );


/*
=====================
	DlgConsoleProc
=====================
*/
BOOL CALLBACK DlgConsoleProc( HWND hWndDlg, UINT msg, WPARAM wParam, LPARAM lParam) {

	switch( msg ) {
	
		case WM_INITDIALOG:

			//
			//init console
			//

			g_conData.hWnd			= hWndDlg;
			g_conData.hWndInputLine	= GetDlgItem( hWndDlg, IDC_CON_INPUT );
			g_conData.hWndBuffer	= GetDlgItem( hWndDlg, IDC_CON_BUFFER );
			
			g_conData.InputLineWndProc =
				(WNDPROC) SetWindowLong( g_conData.hWndInputLine, GWL_WNDPROC, (LONG)InputLineWndProc );
			
			N_Memset( g_conData.szCmd, 0, sizeof(TCHAR)*CON_STRING_SIZE );
			N_Memset( g_conData.szRet, 0, sizeof(TCHAR)*CON_STRING_SIZE );

			g_conData.hbrBackground = CreateSolidBrush( RGB( 220, 215, 215 ) );
			
			SetFocus( g_conData.hWndInputLine );
			break;


		case WM_SHOWWINDOW:
			SetFocus( g_conData.hWndInputLine );
			break;
		
		case WM_CTLCOLORSTATIC:
			SetBkColor( (HDC) wParam, RGB( 220, 215, 215 ) );
			return ((BOOL)g_conData.hbrBackground);
		
		case WM_KEYDOWN:
			switch( wParam ) {
				case VK_ESCAPE:
					SendMessage( hWndDlg, WM_CLOSE, 0, 0 );
					break;
				default:
					break;
			}
			break;
		
		case WM_CLOSE:
			ShowWindow( hWndDlg, SW_HIDE );
			kbConsoleVisible = FALSE;
			break;
		
		case WM_DESTROY:
			DeleteObject( g_conData.hbrBackground );
			break;
		
		default:
			return FALSE;
	}
	return TRUE;
}


/*
=====================
	InputLineWndProc
=====================
*/
static LRESULT CALLBACK InputLineWndProc( HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam ) {
	TCHAR szBuff[256];
	int res;

	switch( msg ) {
		case WM_KILLFOCUS:
			if( ((HWND)wParam == g_conData.hWndBuffer) ||
				((HWND)wParam == g_conData.hWnd ) ) {
				
				SetFocus( hWnd );
			}
			return 0;

		case WM_KEYDOWN:
			switch( wParam ) {
				case VK_ESCAPE: //ESC
					SendMessage( g_conData.hWnd, msg, wParam, lParam );
					return 0;

				case VK_RETURN:
					GetWindowText( hWnd, g_conData.szCmd, CON_STRING_SIZE );
					res = ParseCmd( g_conData.szCmd, g_conData.szRet );
					SetWindowText( hWnd, TEXT( "" ) );
					
					//system command
					if( res ) {
						N_Sprintf( szBuff, 256, TEXT( "\r\n# %s" ), g_conData.szRet );
					}
					//message
					else {
						N_Sprintf( szBuff, 256, TEXT( "\r\n> %s" ), g_conData.szRet );
					}
					SendMessage( g_conData.hWndBuffer, EM_LINESCROLL, 0, 0xffff );
					SendMessage( g_conData.hWndBuffer, EM_SCROLLCARET, 0, 0 );
					SendMessage( g_conData.hWndBuffer, EM_REPLACESEL, 0, (LPARAM)szBuff );
					return 0;

				default:
					break;
			}
			break;
	}

	return CallWindowProc( g_conData.InputLineWndProc, hWnd, msg, wParam, lParam );
}


/*
=====================
	ParseCmd
=====================
*/
static int ParseCmd( TCHAR *szIn, TCHAR *szOut ) {
	conCmd_t command;
	TCHAR szBuff[CON_STRING_SIZE];
	int i;

	if( (szIn == NULL) || (szOut == NULL) ) {
		return 0;
	}
	
	//init "return string"
	N_Strcpy( szOut, TEXT( "invalid command" ) );
	
	if( szIn[0] == '/' ) {
		szIn++;

		//extract useful information from command buffer
		//
		while( *szIn ) {
			N_Memset( szBuff, 0, sizeof(TCHAR)*CON_STRING_SIZE );
			i = 0;
			while( *szIn && (i < CON_STRING_SIZE) ) {
				szBuff[i++] = *(szIn++);
				if( (command = SeeCmd( szBuff )) != CON_INVALID ) {
					break;
				}
			}
			
			switch( command ) {
				case CMD_SET:
					//hacky...
					szIn -= 3;
					if( cfExecute( szIn, N_Strlen( szIn ) ) ) {
						N_Strcpy( szOut, TEXT( "success..." ) );
					}
					else {
						N_Strcpy( szOut, TEXT( "failed..." ) );
					}
					szIn[0] = 0;
					break;
				
				case CMD_SAVECONFIG:
					if( cfSaveTable() ) {
						N_Strcpy( szOut, TEXT( "config saved" ) );
					}
					else {
						N_Strcpy( szOut, TEXT( "could not save config" ) );
					}
					break;
				
				case CMD_QUIT:
					PostQuitMessage( 0 );
					N_Strcpy( szOut, TEXT( "quiting..." ) );
					break;
				
				case CMD_ABOUT:
					N_Sprintf( szOut, CON_STRING_SIZE,
						TEXT( " Netrix \xA9 2005 dimovich\r\n  current version is %s" ), NETRIX_VERSION );
					break;
				
				case CMD_LOADRES:
					reloadResources();
					N_Strcpy( szOut, TEXT( "resources reloaded" ) );
					break;
				
				case CMD_TESTBOT:
					TestBot( &szIn );
					break;

				default:
					break;
			}
		}
	}
	else {
		N_Strncpy( szOut, szIn, CON_STRING_SIZE );
		return 0;
	}
	
	return 1;
}


/*
=====================
	SeeCmd
=====================
*/
static conCmd_t SeeCmd( TCHAR *szCommand ) {
	int i;
	
	for( i=0; i<CMDLIST_SIZE; i++ ) {
		if( !N_Strnicmp( g_conVars[i].name, szCommand, CON_STRING_SIZE ) ) {
			return i;
		}
	}
	
	return CON_INVALID;
}


/*
=====================
	TestBot
=====================
*/
static void TestBot( TCHAR **pszBuff ) {

	TCHAR szBotName[NAMESIZE+1];
	int idx;
	
	while( **pszBuff && **pszBuff == ' ' ) {
		(*pszBuff)++;
	}
	
	if( !**pszBuff ) {
		N_Sprintf( g_conData.szRet, CON_STRING_SIZE, TEXT( "invalid bot name" ) );
		return;
	}
	
	idx = 0;
	while( (idx < NAMESIZE) && **pszBuff && (**pszBuff != ' ') ) {
		szBotName[idx++] = (*(*pszBuff)++);
	}
	szBotName[idx] = '\0';
	
	for( idx=0; idx<k_system.cBots; idx++ ) {
		if( !N_Strcmp( k_system.pBots[idx].bot.name, szBotName ) ) {
			N_Sprintf( g_conData.szRet, CON_STRING_SIZE, TEXT( "testing bot: %s" ), szBotName );
			k_system.idBotLeft = idx;
			endAllGames();
			botSingleGameStart();
			return;
		}
	}
	
	N_Sprintf( g_conData.szRet, CON_STRING_SIZE, TEXT( "bot %s does not exist" ), szBotName );
}