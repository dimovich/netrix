
#ifndef __COMMON_H__
#define __COMMON_H__

#include "compile.h"
#include <windows.h>

#ifdef __cplusplus
extern "C" {
#endif

#define FLAG( f, v ) (((f) & (v)) == (v))

#ifdef ENABLE_LOG

	#define TRACE_BUFF_SIZE 200

	#define N_Trace N_Sprintf( kTraceBuff, TRACE_BUFF_SIZE,	\
		TEXT( "%s (%d) -" ), __N_FILE__, __LINE__ ), _N_Trace

	extern TCHAR kTraceBuff[TRACE_BUFF_SIZE];

	void _N_Trace( TCHAR *, ... );

#else

	#define N_Trace

#endif

void N_InitTrace();
void N_CloseTrace();

BOOL GetSavePath( HWND, TCHAR*, TCHAR*, TCHAR* );

BOOL GetLoadPath( HWND, TCHAR*, TCHAR*, TCHAR* );


#ifdef __cplusplus
}
#endif


#endif //__COMMON_H__