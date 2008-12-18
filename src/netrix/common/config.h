
#ifndef __CONFIG_H__
#define __CONFIG_H__

#ifdef __cplusplus
extern "C" {
#endif


//config commands
//
#define CF_BIND TEXT( "bind" )
#define CF_SET	TEXT( "set" )


//cfVariableList_t
//
typedef enum cfVariableList_e {

//documented

	VAR_FIGCOLOR,				//figure color
	VAR_NEXTFIGCOLOR,			//next figure color
	VAR_SPACECOLOR,				//action space color
	VAR_NEXTSPACECOLOR,			//Next figure space color
	VAR_GROUNDCOLOR,			//ground color
	VAR_SCORESPACECOLOR,		//score background color
	VAR_SCORETEXTCOLOR,			//score text color
	VAR_BASE,					//path to resource
	VAR_PLAYRATE,				//playback speed
	VAR_EFF_ILLUMINATE,			//ground illumination
	VAR_EFF_ILLUMINATE_COLOR,	//ground illumination initial color
	VAR_EFF_PATHCAST,			//path cast effect
	VAR_EFF_PATHCAST_COLOR,		//path cast effect color
	VAR_EFF_LINEKILL,			//linekill effect
	VAR_EFF_LINEKILL_COLOR,		//linekill effect color

	VARLIST_SIZE		//number of variables (always the last)
} cfVariableList_t;

//types that can be contained withing a variant
//
typedef enum cfVariableType_e {
  TYPE_COLOR,
  TYPE_LONG,
  TYPE_BOOL,
  TYPE_CMD,
  TYPE_FLOAT,
  TYPE_STRING
} cfVariableType_t ;


//value types a config variable can be
//
typedef union cfValue_u {
	long	dw;
	TCHAR	*s;
	float	f;
	BOOL	b;
} cfValue_t;


//config variable
typedef struct cfVariable_s {
	int					id;		/* ID */
	TCHAR				*name;	/* name */
	TCHAR				*desc;	/* description */
	cfVariableType_t	type;	/* type */
	cfValue_t			v;		/* value */
	BOOL				modified; //TRUE if modified, FALSE otherwise
} cfVariable_t;


//config table
extern cfVariable_t kcfTable[VARLIST_SIZE];


// loads default values into the config Table
void cfInitTable();

//destroys config table
void cfDestroyTable();

// loads config values from "netrix.cfg"
BOOL cfLoadTable();	

//save config table to "netrix.cfg"
BOOL cfSaveTable();

// returns the id of a specific command
int	cfSeeID( TCHAR * );

// checks the config table for internal corruption
BOOL checkCfTable();

//execute a list of config commands
BOOL cfExecute( char *buffer, int length );

#ifdef __cplusplus
}
#endif

#endif