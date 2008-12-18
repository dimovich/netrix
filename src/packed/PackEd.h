
#ifndef __PACKED_H__
#define __PACKED_H__

#ifdef __cplusplus
extern "C" {
#endif


BOOL CALLBACK DlgPackEdProc( HWND, UINT, WPARAM, LPARAM );

void packEdBuild( HWND );

void packEdExtract( HWND );


#ifdef __cplusplus
}
#endif

#endif