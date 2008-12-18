
#ifndef __BOTLIB_H__
#define __BOTLIB_H__

#ifdef __cplusplus
extern "C" {
#endif


//display a message
//
int B_Message();

//log a message
//
int B_Log();

//quit game
//
int B_Quit();

//generate a random number
//
int B_Random();

//get number of rows eliminated
//
int B_RowsEliminated();

//get number of occupied cells by current figure
//
int B_OccupiedCells();

//get number of shadowed holes
//
int B_ShadowedHoles();

//get pile height
//
int B_PileHeight();

//get sum of well heights
//
int B_WellHeights();

//get num of touching edges
//
int B_TouchingEdges();

//set figure position
//
int B_SetFigure();

//get figure state
//
int B_GetState();

//get figure x position
//
int B_GetPosX();

//get figure y position
//
int B_GetPosY();

//get game difficulty
//
int B_Difficulty();

//push figure to left action-space boundry
//
int B_PushLeft();

//push figure to right action-space boundry
//
int B_PushRight();

//push figure to down action-space boundry
//
int B_PushDown();

//return space x dimension
//
int B_GetSpaceX();

//return space y dimension
//
int B_GetSpaceY();

//set active figure (current/next)
//
int B_UseFigure();

//push current AS on the stack
//
int B_PushAS();

//pop top-most AS from the stack and
//set it as current
//
int B_PopAS();

//copy top-most AS from stack to current AS
//
int B_RestoreAS();

//write figure to current AS
//
int B_WriteFigure();

//eliminate all complete rows
//
int B_EliminateRows();

#ifdef __cplusplus
}
#endif

#endif