
#ifndef __MAPED_H__
#define __MAPED_H__

#ifdef __cplusplus
extern "C" {
#endif


//MapEd proc
BOOL CALLBACK DlgMapEdProc( HWND, UINT, WPARAM, LPARAM );

//creates MapEd toolbar
void mapEdCreateToolbar( HWND );

//paints the map
void paintMap( HWND );

//creates background bitmap
void createBackBitmap();

//paints background bitmap
void paintBackBitmap( HDC );

//initiates Bomb, Flag and Teleport bitmaps
void initBitmaps();

//saves the edited map
void mapEdSave( HWND, BOOL );

//load the edited map
void mapEdLoad( HWND );

//paints the cursor depending on selected trigger
void paintCursor( HDC );


#ifdef __cplusplus
}
#endif

#endif