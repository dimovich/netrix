
#ifndef __FUNCWIN_H__
#define __FUNCWIN_H__

#include "../compile.h"
#include <windows.h>


#ifdef __cplusplus
extern "C" {
#endif


//Main Window controls
//
#define ID_BUTTON_NEW		667
#define ID_BUTTON_OPTIONS	668
#define ID_BUTTON_ABOUT		669
#define ID_BUTTON_LOAD		700
#define ID_BUTTON_SAVE		701
#define ID_BUTTON_PAUSE		702
#define ID_BUTTON_ENDGAME	703
#define ID_BUTTON_QUIT		704
#define ID_BUTTON_MINIMIZE	705


//"New Game" dialog box return values
//
#define NEWGAME_SINGLE			(1<<0)
#define NEWGAME_VS				(1<<1)
#define NEWGAME_BOT				(1<<2)
#define NEWGAME_NETVS			(1<<3)
#define NEWGAME_NETBOT			(1<<4)
#define NEWGAME_DIRECT			(1<<5)
#define NEWGAME_THROUGH			(1<<6)
#define NEWGAME_MEDIATED		(1<<7)
#define NEWGAME_RECORD			(1<<8)


//init Win32 system
BOOL initWin32();

//check if we are running on NT
BOOL OsTypeNT();

//check if Netrix is already running
BOOL AlreadyRunning();

//create Left Player's windows
BOOL populateLeftGUI();

//window procedure for ASes
LRESULT CALLBACK WndProcAS( HWND, UINT, WPARAM, LPARAM );

//window procedure for next figure spaces (NFS)
LRESULT CALLBACK WndProcNFS( HWND, UINT, WPARAM, LPARAM );

//window procedure for score window
LRESULT CALLBACK WndProcScore( HWND, UINT, WPARAM, LPARAM );

//create Right Player's windows
BOOL populateRightGUI();

BOOL un_populateRightGUI();

//populates the map combo box with
//available items
BOOL populateMapComboBox( HWND );

//populates the bot combo box with
//available items
BOOL populateBotComboBox( HWND );

//window procedure for right window
BOOL CALLBACK RightDlgProc( HWND, UINT, WPARAM, LPARAM );

//populate GUi
BOOL populateGUI();

//enable/disable windows from first to last
void MassEnable( HWND hWnd, int first, int last,  BOOL flag );

//load skin
BOOL loadSkin( HRGN *, HBITMAP *, int *, int *, int );

//window procedure for options dialog box
BOOL CALLBACK OptionsDlgProc( HWND, UINT, WPARAM, LPARAM );

//window procedure for New Game dialog box
BOOL CALLBACK NewDlgProc( HWND, UINT, WPARAM, LPARAM );

//window procedure for custom buttons
LRESULT CALLBACK OwnBtnProc( HWND, UINT, WPARAM, LPARAM );

//prints a message
void N_Message( TCHAR *, ... );

//perform some system action (message display, etc...)
void doSystemAction( timeSliceActions_t );


#ifdef __cplusplus
}
#endif


#endif