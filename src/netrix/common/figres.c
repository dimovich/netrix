// Default (standard) figures

#undef __N_FILE__
#define __N_FILE__ TEXT( "figres.c" )

#include "../../netrixlib/netrixlib.h"

#include "types.h"
#include "figres.h"

const int kBlockRes[CTYPE][CYFIG][CXFIG*CSTATE] = {
					    {{0,B_T,0,0,	  0,0,B_T,0,          0,B_TL,B_TR,0,  B_TL,B_TI,B_TR,0},
						 {0,B_L,0,0,	  B_TL,B_BI,B_BR,0,   0,0,B_R,0,      B_B,0,0,0},
						 {0,B_BL,B_TR,0,  0,0,0,0,            0,0,B_B,0,      0,0,0,0},
						 {0,0,0,0,		  0,0,0,0,            0,0,0,0,        0,0,0,0}},
						 
						{{0,0,B_T,0,      B_TL,B_TI,B_TR,0,   0,B_TL,B_TR,0,  B_T,0,0,0},
						 {0,0,B_L,0,      0,0,B_B,0,          0,B_L,0,0,      B_BL,B_BI,B_TR,0},
						 {0,B_TL,B_BR,0,  0,0,0,0,            0,B_B,0,0,      0,0,0,0},
						 {0,0,0,0,        0,0,0,0,            0,0,0,0,        0,0,0,0}},

						{{0,B_T,0,0,      B_TL,B_TI,B_BI,B_TR, 0,B_T,0,0,     B_TL,B_BI,B_TI,B_TR},
						 {0,B_L,0,0,      0,0,0,0,             0,B_R,0,0,     0,0,0,0},
						 {0,B_R,0,0,      0,0,0,0,             0,B_L,0,0,     0,0,0,0},
						 {0,B_B,0,0,      0,0,0,0,             0,B_B,0,0,     0,0,0,0}},

						{{0,B_TL,B_TR,0,  0,B_TL,B_TR,0,      0,B_TL,B_TR,0,  0,B_TL,B_TR,0},
						 {0,B_BL,B_BR,0,  0,B_BL,B_BR,0,      0,B_BL,B_BR,0,  0,B_BL,B_BR,0},
						 {0,0,0,0,        0,0,0,0,            0,0,0,0,        0,0,0,0},
						 {0,0,0,0,        0,0,0,0,            0,0,0,0,        0,0,0,0}},

						{{0,B_T,0,0,         0,B_T,0,0,       B_TL,B_TI,B_TR,0,  0,B_T,0,0},
						 {B_TL,B_BI,B_TR,0,  B_L,B_R,0,0,     0,B_B,0,0,         0,B_L,B_TR,0},
						 {0,0,0,0,           0,B_B,0,0,       0,0,0,0,           0,B_B,0,0},
						 {0,0,0,0,           0,0,0,0,         0,0,0,0,           0,0,0,0}},

						{{0,B_T,0,0,      0,0,B_TL,B_TR,     0,B_T,0,0,         0,0,B_TL,B_TR},
						 {0,B_L,B_TR,0,   0,B_TL,B_BR,0,     0,B_R,B_R,0,       0,B_TL,B_BR,0},
						 {0,0,B_B,0,      0,0,0,0,           0,0,B_B,0,         0,0,0,0},
						 {0,0,0,0,        0,0,0,0,           0,0,0,0,           0,0,0,0}},

						{{0,0,B_T,0,      0,B_TL,B_TR,0,     0,0,B_T,0,        0,B_TL,B_TR,0},
						 {0,B_TL,B_R,0,   0,0,B_BL,B_TR,     0,B_TL,B_R,0,     0,0,B_L,B_TR},
						 {0,B_B,0,0,      0,0,0,0,           0,B_B,0,0,        0,0,0,0},
						 {0,0,0,0,        0,0,0,0,           0,0,0,0,          0,0,0,0}}};