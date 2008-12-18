//$T-Bot$
//
//1-piece AI

int origX, origY, origState;
int curX, curY, curState;
int curMerit;
int bestMerit;
int bestX, bestY, bestState;
int maximumX;
int i;


int Init() {
	return 0;
}

int Update() {

	origX 		= GetPosX();
	origY 		= GetPosY();
	origState	= GetState();

	bestX		= origX;
	bestY		= origY;
	bestState	= origState;

	bestMerit	= -10000;

	//use current figure
	UseFigure( 0 );
	
	//save current AS
	PushAS();

	//check for all possible states
	//
	for( curState=0; curState<4; curState++ ) {

		//check if possible to set new state
		//
		if( SetFigure( origX, origY, curState ) ) {

			//extremes
			//
			maximumX = PushRight();
			curX = PushLeft();

			for( ; curX<=maximumX; curX++ ) {
				//reset y dimension
				//
				SetFigure( curX, origY, curState );
				curY = PushDown();

				//write figure to AS
				WriteFigure();

				//calculate merit
				//
				curMerit = 4*RowsEliminated();
				curMerit += 5*TouchingEdges();
				curMerit += -1*OccupiedCells();
				curMerit += -7*ShadowedHoles();
				curMerit += -4*PileHeight();

				//check if it is a better merit
				//
				if( curMerit > bestMerit ) {
					bestMerit = curMerit;
					bestX = curX;
					bestY = curY;
					bestState = curState;
				}
				
				//restore initial AS
				RestoreAS();
			}
		}
	}
	
	//pop AS
	PopAS();

	SetFigure( bestX, bestY, bestState );
}