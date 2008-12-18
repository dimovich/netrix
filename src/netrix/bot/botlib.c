
//NetrixC botlib functions
//

#undef __N_FILE__
#define __N_FILE__ TEXT( "botlib.c" )

#include "../compile.h"
#include <windows.h>

#include "../../nxc/nxc.h"
#include "../../netrixlib/netrixlib.h"
#include "../win32/func_win.h"
#include "../game/game.h"
#include "../game/func.h"
#include "../game/sys.h"
#include "bot.h"
#include "botlib.h"


extern program_t *kProgram;
extern itoken_t	giToken;

extern game_t	*kBotGame;
extern bot_t	*kBot;
extern figure_t	kBotFigure;
extern int		*kBotAS;


//action space stack
//
typedef struct asStack_s {
	int *as;
	struct asStack_s *next;
} asStack_t;


static asStack_t *g_asStack = NULL;

/*
=====================
	B_Message
=====================
*/
int B_Message() {
	value_t value = {K_FLOAT, 0};
	char buff[1024] = {0};
	char string2[MAX_TOKEN];
	char string[MAX_TOKEN];
	char *p;
	char ch;
	int i;

	//read format string
	//
	I_ReadToken();
	N_Strcpy( string, kProgram->stringTable[giToken.subtype].string );
	L_StripDoubleQuotes( string );

	p = string;

	i=0;
	while( (ch = *(p++)) != '\0' ) {
		if( i>=(MAX_TOKEN-1) ) {
			I_ScriptError( "string overflow" );
			break;
		}
		if( ch == '%' ) {
			switch( *p ) {
				case 'd':
					I_ReadToken();	//skip until ","
					P_EvalExp( &value );
					i += N_Sprintf( &buff[i], MAX_TOKEN-i, "%d", (int)value.v.f );
					p++;
					break;
				case 'f':
					I_ReadToken(); //skip until ","
					P_EvalExp( &value );
					i += N_Sprintf( &buff[i], MAX_TOKEN-i, "%f", value.v.f );
					p++;
					break;
				case 's':
					I_ReadToken(); //skip until ","
					I_ReadToken(); //read string
					N_Strcpy( string2, kProgram->stringTable[giToken.subtype].string );
					L_StripDoubleQuotes( string2 );
					i += N_Sprintf( &buff[i], MAX_TOKEN-i, "%s", string2 );
					p++;
					break;
				default:
					I_ScriptError( "unknown format field" );
					break;
			}
		}
		else {
			buff[i++] = ch;
		}
	}
	
	buff[i] = '\0';

	N_Message( "%s", buff );
	
	return 0;
}


/*
=====================
	B_Log
=====================
*/
int B_Log() {
	value_t value = {K_FLOAT, 0};
	char buff[1024] = {0};
	char string2[MAX_TOKEN];
	char string[MAX_TOKEN];
	char *p;
	char ch;
	int i;

	//read format string
	//
	I_ReadToken();
	N_Strcpy( string, kProgram->stringTable[giToken.subtype].string );
	L_StripDoubleQuotes( string );

	p = string;

	i=0;
	while( (ch = *(p++)) != '\0' ) {
		if( i>=(MAX_TOKEN-1) ) {
			I_ScriptError( "string overflow" );
			break;
		}
		if( ch == '%' ) {
			switch( *p ) {
				case 'd':
					I_ReadToken();	//skip until ","
					P_EvalExp( &value );
					i += N_Sprintf( &buff[i], MAX_TOKEN-i, "%d", (int)value.v.f );
					p++;
					break;
				case 'f':
					I_ReadToken(); //skip until ","
					P_EvalExp( &value );
					i += N_Sprintf( &buff[i], MAX_TOKEN-i, "%f", value.v.f );
					p++;
					break;
				case 's':
					I_ReadToken(); //skip until ","
					I_ReadToken(); //read string
					N_Strcpy( string2, kProgram->stringTable[giToken.subtype].string );
					L_StripDoubleQuotes( string2 );
					i += N_Sprintf( &buff[i], MAX_TOKEN-i, "%s", string2 );
					p++;
					break;
				default:
					I_ScriptError( "unknown format field" );
					break;
			}
		}
		else {
			buff[i++] = ch;
		}
	}
	
	buff[i] = '\0';

	N_Trace( "%s", buff );
	
	return 0;
}


/*
=====================
	B_Random
=====================
*/
int B_Random() {
	value_t tmpval = {K_FLOAT, 0};
	
	//evaluate argument
	//
	P_EvalExp( &tmpval );

	return N_Random( (int)tmpval.v.f );
}


/*
=====================
	B_Quit
=====================
*/
int B_Quit() {
	PostQuitMessage( 0 );
	return 0;
}


/*
=====================
	B_RowsEliminated
=====================
*/
int B_RowsEliminated() {
	int num;
	int count;
	int i;
	int j;
	
	num = 0;
	
	for( i=0; i<CYSPACE; i++ ) {
		count = 0;
		for( j=0; j<CXSPACE; j++ ) {
			if( SPACE_CELL( kBotAS, i, j ) == MAPCELL_BLOCK ) {
				count++;
			}
		}
		if( count == CXSPACE ) {
			num++;
		}
	}
	
	return num;
}


/*
=====================
	B_OccupiedCells
=====================
*/
int B_OccupiedCells() {
	return getFigBlockNum( &kBotFigure );
}


/*
=====================
	B_ShadowedHoles
=====================
*/
int B_ShadowedHoles() {
	int j, i;
	int tmp;
	int xi, yi;
	int xf, yf;
	int count = 0;
	
	for( j=0; j<CXFIG; j++ ) {
		i = 0;
		tmp = -1;
		while( i<CYFIG ) {
			if( kFigRes[kBotFigure.type][i][kBotFigure.state*CSTATE + j] == 1 ) {
				tmp=i;
			}
			i++;
		}

		if( (tmp < 4) && (tmp > -1) ) {

			xi = ( j + kBotFigure.pos.x );
			yi = ( tmp + kBotFigure.pos.y );
			
			yf = yi+1;
			xf = xi;

			while( (yf < CYSPACE) && (SPACE_CELL( kBotAS, yf, xf ) != MAPCELL_BLOCK) ) {
				yf++;
			}

			if( yf > yi+1 ) {
				count += yf - yi - 1;
			}
		}
	}
	
	return count;
}


/*
=====================
	B_PileHeight
=====================
*/
int B_PileHeight() {
	int top=0;

	//get figure top position
	getFigHeight( &kBotFigure, &top, NULL );

	return (CYSPACE-(kBotFigure.pos.y+top));
}


/*
=====================
	B_WellHeights
	-----------------
	FIXME: rewrite
=====================
*/
int B_WellHeights() {
	int sum;
	int min, mid;
	int h[3], idx;
	int i, j, k;

	sum = 0;

	for( j=0; j<CXSPACE; j+=3 ) {
		N_Memset( h, 0, 3*sizeof(int) );
		idx = 0;
		//determine heights
		//
		for( k=j; k<j+3; k++ ) {
			for( i=0; i<CYSPACE; i++ ) {
				if( SPACE_CELL( kBotAS, i, k ) == MAPCELL_BLOCK ) {
					break;
				}
				h[idx]++;
			}
			idx++;
		}

		//determine minimum
		//
		if( h[0] < h[1] ) {
			if( h[0] < h[2] ) {
				min = h[0];
				h[0] = CYSPACE+1;
			}
			else {
				min = h[2];
				h[2] = CYSPACE+1;
			}
		}
		else {
			if( h[1] < h[2] ) {
				min = h[1];
				h[1] = CYSPACE+1;
			}
			else {
				min = h[2];
				h[2] = CYSPACE+1;
			}
		}
		
		//determine medium
		//
		if( h[0] < h[1] ) {
			if( h[0] < h[2] ) {
				mid = h[0];
			}
			else {
				mid = h[2];
			}
		}
		else {
			if( h[1] < h[2] ) {
				mid = h[1];
			}
			else {
				mid = h[2];
			}
		}
		
		sum += mid - min;
	}

	return sum;
}


/*
=====================
	B_TouchingEdges
=====================
*/
int B_TouchingEdges() {
	int left, right;
	int top, bottom;
	int i, j;
	BOOL flag;
	int edges;
	int *as = kBotAS;
	
	edges = 0;

	//x-dimension
	//
	for( i=0; i<CYFIG; i++ ) {
		left = CXFIG;
		right = 0;
		flag = FALSE;
		for( j=0; j<CXFIG; j++ ) {
			if( kFigRes[kBotFigure.type][i][j+kBotFigure.state*CSTATE] == 1 ) {
				if( j < left ) {
					left = j;
				}
				if( j > right ) {
					right = j;
				}
				flag = TRUE;
			}
		}
		
		if( flag ) {
			//left
			//
			if( SPACE_CELL( as, kBotFigure.pos.y+i, kBotFigure.pos.x+left-1 ) == MAPCELL_BLOCK ) {
				edges++;
			}
			
			//right
			//
			if( SPACE_CELL( as, kBotFigure.pos.y+i, kBotFigure.pos.x+right+1 ) == MAPCELL_BLOCK ) {
				edges++;
			}
		}
	}
	
	//y-dimension
	//
	for( j=0; j<CXFIG; j++ ) {
		top = CYFIG;
		bottom = 0;
		flag = FALSE;
		for( i=0; i<CYFIG; i++ ) {
			if( kFigRes[kBotFigure.type][i][j+kBotFigure.state*CSTATE] == 1 ) {
				if( i < top ) {
					top = i;
				}
				if( i > bottom ) {
					bottom = i;
				}
				flag = TRUE;
			}
		}
		
		if( flag ) {
			//top
			//
			if( SPACE_CELL( as, kBotFigure.pos.y+top-1, kBotFigure.pos.x+j ) == MAPCELL_BLOCK ) {
				edges++;
			}
			
			//bottom
			//
			if( SPACE_CELL( as, kBotFigure.pos.y+bottom+1, kBotFigure.pos.x+j ) == MAPCELL_BLOCK ) {
				edges++;
			}
		}
	}
	
	return edges;
}


/*
=====================
	B_SetFigure
=====================
*/
int B_SetFigure() {
	value_t tmpval = {K_FLOAT, 0};
	figure_t tmpfig;
	
	tmpfig.type = kBotFigure.type;
	
	//read x position
	//
	P_EvalExp( &tmpval );
	tmpfig.pos.x = (int)tmpval.v.f;
	
	I_ReadToken(); //skip ","
	
	//read y position
	//
	P_EvalExp( &tmpval );
	tmpfig.pos.y = (int)tmpval.v.f;
	
	I_ReadToken(); //skip ","
	
	//read rotation
	//
	P_EvalExp( &tmpval );
	tmpfig.state = (int)tmpval.v.f % CSTATE;
	
	//check if figure position is valid
	//
	if( tryFig( &tmpfig, kBotAS ) ) {
		kBotFigure.pos = tmpfig.pos;
		kBotFigure.state = tmpfig.state;
		return 1;
	}

	return 0;
}


/*
=====================
	B_GetState
=====================
*/
int B_GetState() {
	return kBotFigure.state;
}


/*
=====================
	B_GetPosX
=====================
*/
int B_GetPosX() {
	return kBotFigure.pos.x;
}


/*
=====================
	B_GetPosY
=====================
*/
int B_GetPosY() {
	return kBotFigure.pos.y;
}


/*
=====================
	B_Difficulty
=====================
*/
int B_Difficulty() {
	return kBot->difficulty;
}


/*
=====================
	B_PushLeft
=====================
*/
int B_PushLeft() {

	if( tryFig( &kBotFigure, kBotAS ) ) {
		do {
			kBotFigure.pos.x--;
		}while( tryFig( &kBotFigure, kBotAS ) );
		kBotFigure.pos.x++;
	}
	
	return kBotFigure.pos.x;
}


/*
=====================
	B_PushRight
=====================
*/
int B_PushRight() {

	if( tryFig( &kBotFigure, kBotAS ) ) {
		do {
			kBotFigure.pos.x++;
		}while( tryFig( &kBotFigure, kBotAS ) );
		kBotFigure.pos.x--;
	}
	
	return kBotFigure.pos.x;
}


/*
=====================
	B_PushDown
=====================
*/
int B_PushDown() {

	if( tryFig( &kBotFigure, kBotAS ) ) {
		do {
			kBotFigure.pos.y++;
		}while( tryFig( &kBotFigure, kBotAS ) );
		kBotFigure.pos.y--;
	}
	
	return kBotFigure.pos.y;
}


/*
=====================
	B_GetSpaceX
=====================
*/
int B_GetSpaceX() {
	return CXSPACE;
}


/*
=====================
	B_GetSpaceY
=====================
*/
int B_GetSpaceY() {
	return CYSPACE;
}


/*
=====================
	B_PushAS
=====================
*/
int B_PushAS() {
	asStack_t *item = NULL;
	
	if( kBotAS ) {
		item = N_Malloc( sizeof(asStack_t) );
		item->as = N_Malloc( sizeof(int)*SPACESIZE );
		
		N_Memcpy( item->as, kBotAS, sizeof(int)*SPACESIZE );
		
		item->next = g_asStack;
		g_asStack = item;
	}
	
	return 0;
}


/*
=====================
	B_PopAS
=====================
*/
int B_PopAS() {
	asStack_t *item = NULL;

	if( kBotAS && g_asStack && g_asStack->as ) {
		N_Memcpy( kBotAS, g_asStack->as, sizeof(int)*SPACESIZE );
		item = g_asStack;
		g_asStack = g_asStack->next;
		N_Free( item->as );
		N_Free( item );
	}
	
	return 0;
}


/*
=====================
	B_RestoreAS
=====================
*/
int B_RestoreAS() {
	if( kBotAS && g_asStack && g_asStack->as ) {
		N_Memcpy( kBotAS, g_asStack->as, sizeof(int)*SPACESIZE );
	}
	
	return 0;
}


/*
=====================
	B_UseFigure
=====================
*/
int B_UseFigure() {
	value_t value = {K_FLOAT, 0};

	P_EvalExp( &value );
	
	switch( (int)value.v.f % 2 ) {
		//current figure
		//
		case 0:
		default:
			kBotFigure.type = kBotGame->pFig->type;
			kBotFigure.state = kBotGame->pFig->state;
			kBotFigure.pos.x = kBotGame->pFig->pos.x;
			kBotFigure.pos.y = kBotGame->pFig->pos.y;
			break;
		
		//next figure
		//
		case 1:
			kBotFigure.type = kBotGame->pNextFig->type;
			kBotFigure.state = kBotGame->pNextFig->state;
			kBotFigure.pos.x = kBotGame->pNextFig->pos.x;
			kBotFigure.pos.y = kBotGame->pNextFig->pos.y;
			break;
	}
	
	return 0;
}


/*
=====================
	B_WriteFigure
=====================
*/
int B_WriteFigure() {
	if( kBotAS ) {
		writeSpace( &kBotFigure, kBotAS );
	}
	
	return 0;
}


/*
=====================
	B_EliminateRows
=====================
*/
int B_EliminateRows() {
	int i,j,k;
	int sum;
	int index;
	int lines[CYSPACE] = {0};
	
	if( !kBotAS ) {
		return 0;
	}
	
	//init variables
	index=0;

	//mark completed lines
	for( i=0; i<CYSPACE; i++ ) {
		sum = 0;
		for( j=0; j<CXSPACE; j++ ) {
			if( SPACE_CELL( kBotAS, i, j ) == MAPCELL_BLOCK ) {
				sum++;
			}
		}
		
		//line is completed
		if( sum == CXSPACE ) {
			lines[i] = 1;
			index++;
		}
	}

	for( i=0; i<CYSPACE; i++ ) {
		//check if line is marked for removal
		if( lines[i] ) {
			for( j=i; j>0; j-- ) {
				for( k=0; k<CXSPACE; k++ ) {
					//copy the upper line into the current one
					SPACE_CELL( kBotAS, j, k ) = SPACE_CELL( kBotAS, j-1, k );
				}
			}
			//fill the top-most line with empty blocks
			for( j=0; j<CXSPACE; j++ ) {
				SPACE_CELL( kBotAS, 0, j ) = MAPCELL_EMPTY;
			}
		}
	}
	
	return 0;
}