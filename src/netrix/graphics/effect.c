
//Netrix effects
//

#undef __N_FILE__
#define __N_FILE__ TEXT( "effect.c" )

#include "../compile.h"
#include <windows.h>
#include <GL/gl.h>

#include "../../netrixlib/netrixlib.h"

#include "../game/game.h"
#include "../game/sys.h"
#include "../game/seq.h"
#include "../game/func.h"
#include "../game/scheduler.h"
#include "../game/trigger.h"
#include "../common/const.h"
#include "../common/config.h"
#include "ngl.h"
#include "graphics.h"
#include "effect.h"


// general effect configuration
effectConfig_t kEffects[ EFFLIST_SIZE ] = {
	EFF_CONST,	EFFECT_FIG_COROLA,			0,		0,		1,
	NULL,						NULL,
	
	EFF_CONST,	EFFECT_FIG_MOTIONBLUR,		0,		0,		1,
	NULL,						NULL,
	
	EFF_POST,	EFFECT_GROUND_ILLUMINATE,	10,		500,	1,
	calc_eff_GroundIlluminate,	paint_eff_GroundIlluminate,
	
	EFF_POST,	EFFECT_AS_LINEKILL,			120,	480,	2,
	calc_eff_AsLineKill,		paint_eff_AsLineKill,
	
	EFF_POST,	EFFECT_GROUND_TOXICBLOCKS,	10,		600,	2,
	calc_eff_GroundToxicBlocks, paint_eff_GroundToxicBlocks
};


/*
=====================
	doEffect
=====================
*/
void doEffect( effectList_t *eff ) {
	if( (eff != NULL) && (eff->effect.type < EFFLIST_SIZE) ) {
		kEffects[ eff->effect.type ].calcFunc( &eff->effect, FALSE );
	}
}


/*
=====================
	pushEffect
=====================
*/
void pushEffect( effectType_t type, place_t place ) {
	game_t *game = NULL;
	effect_t eff;
	effectList_t *effListNode;
	effectList_t *tmp1, *tmp2;

	effListNode =
	tmp1 =
	tmp2 = NULL;

	//check effect type validity
	if( type >= EFFLIST_SIZE ) {
		return;
	}

	//determine game
	switch( place ) {
		case LEFTGAME:
			game = k_system.pLeftGame;
			break;
		
		case RIGHTGAME:
			game = k_system.pRightGame;
			break;

		default:
			return;
	}

	//check game validity
	if( game == NULL ) {
		return;
	}

	//remove previous illumination effect,
	//so that it doesn't interfere with the current one.
	//
	//FIXME: isn't there a better approach ?
	//
	if( type == EFFECT_GROUND_ILLUMINATE ) {
		tmp1 = game->effectList;
		while( tmp1 ) {
			tmp2 = tmp1->pNext;
			if( tmp1->effect.type == type ) {
				popTimeSlice( tmp1->effect.guid );
			}
			tmp1 = tmp2;
		}
	}

	N_Memset( &eff, 0, sizeof( eff ) );
	eff.type	= type;
	eff.place	= place;
	eff.init	= FALSE;
	eff.bEnded	= FALSE;
	
	effListNode = attachEffect( &eff );

	effListNode->effect.guid = pushTimeSlice( TIME_EFFECT, kEffects[type].interval,
		kEffects[type].ttl, effListNode, EFFECT, TRUE );
}


/*
=====================
	attachEffect
=====================
*/
effectList_t * attachEffect( effect_t *eff ) {
	game_t *game = NULL;
	effectList_t *effList;
	effectList_t *tmp;
	effectList_t *tmp2;
	
	//check effect validity
	if( eff == NULL ) {
		return NULL;
	}
	
	//determine game
	switch( eff->place ) {
		case LEFTGAME:
			game = k_system.pLeftGame;
			break;
		
		case RIGHTGAME:
			game = k_system.pRightGame;
			break;

		default:
			return NULL;
	}
	
	//check game validity
	if( game == NULL ) {
		return NULL;
	}
	
	//copy effect to list node
	effList = N_Malloc( sizeof(*effList) );
	N_Memcpy( &effList->effect, eff, sizeof(*eff) );


	//spawn a new effect list
	if( game->effectList == NULL ) {
		effList->pNext = NULL;
		game->effectList = effList;
	}
	else {
		//find a proper place (based on priority);
		//increasing order.
		tmp2 =
		tmp = game->effectList;
		while( tmp ) {
			if( kEffects[ tmp->effect.type ].priority >=
				kEffects[ eff->type ].priority ) {
				break;
			}
			tmp2 = tmp;
			tmp = tmp->pNext;
		}
		
		if( tmp == game->effectList ) {
			effList->pNext = game->effectList;
			game->effectList = effList;
		}
		else {
			tmp2->pNext = effList;
			effList->pNext = tmp;
		}
	}
	
	return effList;
}


/*
=====================
	deattachEffect
=====================
*/
void deattachEffect( effectList_t *eff ) {
	effectList_t *effList, *tmp;
	game_t *game = NULL;
	
	//check arguments
	if( !eff ) {
		return;
	}
	
	//determine game
	switch( eff->effect.place ) {
		case LEFTGAME:
			game = k_system.pLeftGame;
			break;
		
		case RIGHTGAME:
			game = k_system.pRightGame;
			break;

		default:
			return;
	}
	
	//check game validity
	if( !game ) {
		return;
	}
	
	tmp =
	effList = game->effectList;
	while( effList && (effList!=eff) ) {
		tmp = effList;
		effList = effList->pNext;
	}
	
	//no appropriate effect found
	if( effList == NULL ) {
		return;
	}
	
	if( effList == game->effectList ) {
		game->effectList = game->effectList->pNext;
	}
	else {
		tmp->pNext = eff->pNext;
	}

	N_Free( eff );
}


/*
=====================
	calc_eff_GroundIlluminate
=====================
*/
void calc_eff_GroundIlluminate( effect_t *eff, BOOL kill ) {
	static int frames;
	COLORREF cr;
	
	if( eff && !eff->bEnded ) {

		eff->frame++;

		if( eff->init ) {

			if( (eff->frame > frames) || kill ) {
				eff->R = GetRValue( (DWORD) eff->v4 );
				eff->G = GetGValue( (DWORD) eff->v4 );
				eff->B = GetBValue( (DWORD) eff->v4 );
				eff->bEnded = TRUE;
				return;
			}
			
			eff->R += eff->v1;
			eff->G += eff->v2;
			eff->B += eff->v3;

			drawAS( eff->place, CGF_PAINTEFFECTS );
		}
		
		else {
			frames = (int) ( kEffects[EFFECT_GROUND_ILLUMINATE].ttl /
				(1.0f * kEffects[EFFECT_GROUND_ILLUMINATE].interval ) );
			
			cr = kcfTable[VAR_GROUNDCOLOR].v.dw;
			
			eff->R = (float) GetRValue( cr );
			eff->G = (float) GetGValue( cr );
			eff->B = (float) GetBValue( cr );
			
			eff->v4 = (float) cr;
			
			
			cr = kcfTable[VAR_EFF_ILLUMINATE_COLOR].v.dw;

			eff->v1 = ( eff->R - (float)GetRValue( cr ) ) / (float)frames;		
			eff->v2 = ( eff->G - (float)GetGValue( cr ) ) / (float)frames;
			eff->v3 = ( eff->B - (float)GetBValue( cr ) ) / (float)frames;

			eff->R = (float) GetRValue( cr );
			eff->G = (float) GetGValue( cr );
			eff->B = (float) GetBValue( cr );

			eff->init = TRUE;
		}
	}
}


/*
=====================
	paint_eff_GroundIlluminate
=====================
*/
void paint_eff_GroundIlluminate( const effect_t *eff ) {
	game_t *game;
	int i, j;
	
	if( eff && !eff->bEnded && eff->init ) {

		switch( eff->place ) {
			case LEFTGAME:
				game = k_system.pLeftGame;
				break;
			
			case RIGHTGAME:
				game = k_system.pRightGame;
				break;
			
			default:
				return;
		}

		glBegin( GL_QUADS );
		for( i=0; i<CYSPACE; i++ ) {
			for( j=0; j<CXSPACE; j++ ) {
				if( SPACE_CELL( game->AS, i, j ) == MAPCELL_BLOCK )
					NGL_drawBlock( j*BLOCKSIZE, i*BLOCKSIZE, RGB( eff->R, eff->G, eff->B ) );
			}
		}
		glEnd();
	}
}



/*
=====================
	calc_eff_GroundToxicBlocks
=====================
*/
void calc_eff_GroundToxicBlocks( effect_t * eff, BOOL kill ) {
	static int frames;
	COLORREF cr;
	
	//check effect validity
	if( eff == NULL ) {
		return;
	}
	
	if( !eff->bEnded ) {
	
		eff->frame++;

		if( eff->init ) {

			if( (eff->frame > frames) || kill ) {
				eff->R = GetRValue( (DWORD) eff->v4 );
				eff->G = GetGValue( (DWORD) eff->v4 );
				eff->B = GetBValue( (DWORD) eff->v4 );
				eff->bEnded = TRUE;
				return;
			}

			eff->R += eff->v1;
			eff->G += eff->v2;
			eff->B += eff->v3;
			
			drawAS( eff->place, CGF_PAINTEFFECTS );

		} //eff->init
		
		else {

			frames = (int) (kEffects[EFFECT_GROUND_ILLUMINATE].ttl /
				(1.0f * kEffects[EFFECT_GROUND_ILLUMINATE].interval ) );
			
			cr = kcfTable[VAR_GROUNDCOLOR].v.dw;
			
			eff->R = (float) GetRValue( cr );
			eff->G = (float) GetGValue( cr );
			eff->B = (float) GetBValue( cr );
			
			eff->v4 = (float) cr;
			
			cr = RGB( 169, 240, 21 );

			eff->v1 = ( eff->R - (float)GetRValue( cr ) ) / (float)frames;		
			eff->v2 = ( eff->G - (float)GetGValue( cr ) ) / (float)frames;
			eff->v3 = ( eff->B - (float)GetBValue( cr ) ) / (float)frames;

			eff->R = (float) GetRValue( cr );
			eff->G = (float) GetGValue( cr );
			eff->B = (float) GetBValue( cr );

			eff->init = TRUE;
		}
	}
}


/*
=====================
	paint_eff_GroundToxicBlocks
=====================
*/
void paint_eff_GroundToxicBlocks( const effect_t *eff ) {
	game_t *game = NULL;
	int i;

	//check effect validity
	if( eff == NULL ) {
		return;
	}

	if( !eff->bEnded && eff->init ) {

		switch( eff->place ) {

			case LEFTGAME:
				game = k_system.pLeftGame;
				break;
			
			case RIGHTGAME:
				game = k_system.pRightGame;
				break;

			default:
				return;
		}

		glBegin( GL_QUADS );
		for( i=0; i<CXSPACE; i++ ) {
			if( game->toxicBlocks[i] > -1 )
				NGL_drawBlock( i*BLOCKSIZE, game->toxicBlocks[i]*BLOCKSIZE,
					RGB( eff->R, eff->G, eff->B ) );
		}
		glEnd();
	}
}


/*
=====================
	calc_eff_AsLineKill
=====================
*/
void calc_eff_AsLineKill( effect_t *eff, BOOL kill ) {
	static int frames;
	game_t *game = NULL;
	
	//check effect validity
	if( eff == NULL ) {
		return;
	}
	
	switch( eff->place ) {
		case LEFTGAME:
			game = k_system.pLeftGame;
			break;
		
		case RIGHTGAME:
			game = k_system.pRightGame;
			break;

		default:
			return;
	}
	
	//check game validity
	//
	if( game == NULL ) {
		return;
	}
	
	if( !eff->bEnded ) {

		eff->frame++;

		if( eff->init ) {

			if( (eff->frame >= frames) || kill ) {
				game->flags &= ~SF_SKIPSEQ;
				eff->bEnded = TRUE;
				seqProc( game );
				return;
			}

			eff->v1 = (float) ( !( (int)eff->v1 ) );
			drawAS( eff->place, CGF_PAINTEFFECTS );
		}
		else {
			game->flags |= SF_SKIPSEQ;
			eff->v1 = 0.0f;
			eff->init = TRUE;
			frames = (int) ( kEffects[eff->type].ttl/kEffects[eff->type].interval );
		}
	}
}


/*
=====================
	paint_eff_AsLineKill
=====================
*/
void paint_eff_AsLineKill( const effect_t *eff ) {
	COLORREF cr;
	game_t *game = NULL;
	int i, j;

	//check effect validity
	if( eff == NULL ) {
		return;
	}

	if( !eff->bEnded && eff->init ) {

		switch( eff->place ) {

			case LEFTGAME:
				game = k_system.pLeftGame;
				break;
			
			case RIGHTGAME:
				game = k_system.pRightGame;
				break;
			
			default:
				return;
		}

		if( ((int)eff->v1) == 1 ) {
			cr = RGB( 200, 0, 0 );
		}
		else {
			cr = RGB( 100, 0, 0 );
		}

		glBegin( GL_QUADS );
		for( i=0; i<CYSPACE; i++ ) {
			if( game->abLines[i] ) {
				for( j=0; j<CXSPACE; j++ ) {
					NGL_drawBlock( j*BLOCKSIZE, i*BLOCKSIZE, cr );
				}
			}
		}
		glEnd();
	}
}


/*
=====================
	paint_Pathcast
=====================
*/
void paint_Pathcast( place_t place ) {
	int xi, yi;
	int xf, yf;
	int i,j;
	COLORREF cr;
	int tmp;
	game_t *game = NULL;
	figure_t *fig = NULL;
	int blocksize = BLOCKSIZE;

	switch( place ) {
		case LEFTGAME:
			game = k_system.pLeftGame;
			break;
		
		case RIGHTGAME:
			game = k_system.pRightGame;
			break;
		
		default:
			return;
	}
	
	if( game == NULL ) {
		return;
	}

	//validate figure
	//
	if( game->pFig == NULL ) {
		return;
	}

	fig = game->pFig;
	if( fig->desintegrated ) {
		return;
	}
	
	//sometimes the figure has an invalid position
	//and seqProc has not been called yet to fix the problem
	//
	if( (fig->pos.y >= CYSPACE) ) {
		return;
	}
	
	cr = kcfTable[VAR_EFF_PATHCAST_COLOR].v.dw;
	glColor3f( GetRValue(cr)/255.0f, GetGValue(cr)/255.0f, GetBValue(cr)/255.0f );

	//draw pathcast effect
	//
	glBegin( GL_QUADS );
	for( j=0; j<CXFIG; j++ ) {
		i = 0;
		tmp = -1;
		while( i<CYFIG ) {
			if( kFigRes[fig->type][i][fig->state*CSTATE + j] == 1 ) {
				tmp=i;
			}
			i++;
		}
		
		//cast "shadow"
		//
		if( (tmp < 4) && (tmp > -1) ) {

			xi = ( j + fig->pos.x );
			yi = ( tmp + fig->pos.y + 1 );
			
			yf = yi;
			xf = xi;

			while( (yf < CYSPACE) && (SPACE_CELL( game->AS, yf, xf ) != MAPCELL_BLOCK) ) {
				yf++;
			}
			
			if( yf > yi ) {
				NGL_polygon( (GLfloat)( xi*blocksize ), (GLfloat)( yf*blocksize - blocksize/3 ),
					(GLfloat)( (xi+1)*blocksize ), (GLfloat)( yf*blocksize ) );
			}
		}
	}
	glEnd();
}