//$Z-Bot$
//
//2-piece dynamic AI

int origX, origY, origState;
int curX, curY, curState;
int nextX, nextY, nextState;
int origNextX, origNextY, origNextState;
int curMerit;
int nextMerit;
int bestMerit;
int bestX, bestY, bestState;
int maxX;
int maxNextX;
int i;


int Init() {
	return 0;
}

int Update() {

	//current figure
	UseFigure( 0 );
	origX 		= GetPosX();
	origY 		= GetPosY();
	origState	= GetState();
	bestX		= origX;
	bestY		= origY;
	bestState	= origState;
	
	//next figure
	UseFigure( 1 );
	origNextX		= GetPosX();
	origNextY		= GetPosY();
	origNextState	= GetState();

	bestMerit	= -10000;
	
	//save current AS
	PushAS();

	UseFigure( 0 );

	//current figure
	//
	for( curState=0; curState<4; curState++ ) {

		if( SetFigure( origX, origY, curState ) ) {

			maxX = PushRight();
			curX = PushLeft();

			for( ; curX<=maxX; curX++ ) {

				SetFigure( curX, origY, curState );
				curY = PushDown();

				WriteFigure();

				curMerit = 4*RowsEliminated();
				curMerit += 5*TouchingEdges();
				curMerit += -7*ShadowedHoles();
				curMerit += -4*PileHeight();

				EliminateRows();

				PushAS();

				UseFigure( 1 );
				
				//next figure
				//
				for( nextState=0; nextState<4; nextState++ ) {
					
					if( SetFigure( origNextX, origNextY, nextState ) ) {
						
						//extremes
						maxNextX = PushRight();
						nextX = PushLeft();
						
						for( ; nextX<=maxNextX; nextX++ ) {
							SetFigure( nextX, origNextY, nextState );
							nextY = PushDown();
							
							WriteFigure();
							
							nextMerit = 4*RowsEliminated();
							nextMerit += 5*TouchingEdges();
							nextMerit += -7*ShadowedHoles();
							nextMerit += -4*PileHeight();
							
							nextMerit += curMerit;
							
							if( nextMerit > bestMerit ) {
								bestMerit = nextMerit;
								
								bestX = curX;
								bestY = curY;
								bestState = curState;
							}
							
							RestoreAS();
						}
					}
				}
				PopAS();
				UseFigure( 0 );
				RestoreAS();
			}
		}
	}
	
	//pop AS
	PopAS();

	SetFigure( bestX, bestY, bestState );
}