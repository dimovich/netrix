
//Constants
//

#ifndef __CONST_H__
#define __CONST_H__

#define FIGCOLOR		RGB(90,96,87)		/* figure color*/
#define NEXTFIGCOLOR	RGB(157,218,238)	/* next figure color*/
#define SPACECOLOR		RGB(130,151,165)	/* action space color*/
#define NEXTSPACECOLOR	RGB(90,111,115)
#define GROUNDCOLOR		RGB(70, 76, 67)		/* ground color*/
#define SCORESPACECOLOR	RGB( 180, 180, 180 )//score space color
#define SCORETEXTCOLOR	RGB( 155, 100, 100 )//score text color
#define BASE			TEXT( "base" )		/* default data location*/

#define EFFILLUMINATE		RGB( 255, 255, 255 ) /* Illuminate Effect Initial Color */


	/* Netrix Constants (for now) */

#define BLOCKSIZE		21					/* block size (in pixels) */
#define BLOCKSIZEN		22			//next figure block size
#define CXSPACE			14					/* action space x size (in figure blocks) */
#define CYSPACE			19					/* action space y size (in figure blocks) */
#define CONFIGFILE		TEXT( "netrix.cfg" )/* default netrix configuration file */
#define STARTPOS		5					/* CXSPACE/2-2 */


#define CYFIG			4		/* figure lines */
#define CXFIG			4		/* figure lines */
#define CTYPE			7		/* figure nbr. */
#define CSTATE			4		/* possible figure positions */
#define ASWALL			4		/* action space wall thickness */

#define SPACESIZE		((CYSPACE+2*ASWALL)*(CXSPACE+2*ASWALL))


#define NAMESIZE		25	//name size
#define FILENAMESIZE	25	//file name size

#endif
