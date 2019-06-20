/***************************************************************************
*  Module:        TERMINAL.C                                               *
*                                                                          *
*  Created By:    Roy Chancellor                                           *
*                                                                          *
*  Date Created:  03-21-1994                                               *
*                                                                          *
*  Last Modified: 05-17-1994                                               *
*                                                                          *
*  Version:       1.00                                                     *
*                                                                          *
*  Date Released: 05-10-1994                                               *
*                                                                          *
*  Description:   SIMPLE TERMINAL FOR MODEM COMMUNICATIONS                 *
*                                                                          *
*  Inputs:        NONE                                                     *
*                                                                          *
*  Returns:       1 IF OK; 0 IF ANY FAILS                                  *
*                                                                          *
*  Date           Modification                                    Initials *
* ----------   -------------------------------------------        ---------*
*  05-10-1994  Released for use on a CAC as Version 1.00             RSC   *
****************************************************************************/
static char *version = "Version 1.00";

/* Include Files that came with Turbo C...                                 */
#include <stdio.h>      /* all C programs                                  */
#include <bios.h>
#include <string.h>     /* for strcmp, strlen, etc.                        */
#include <dos.h>			/* for sleep													*/

/* Include files from modem test project...                                */
#include <pc2modem.h>	/* for defines used by pc2modem							*/
#include <popwin.h>     /* for calls to popwin                             */
#include <boolean.h>    /* for values that are 1 or 0                      */
#include <utility.h>    /* for restore_scrn                                */

/* Include files from Silvercom...                                         */
#include <swcasync.h>
#include <swkeys.h>

/* Definitions...                                                          */
#define ROWS		15
#define TERM_FORE	WHITE
#define TERM_BACK	BLACK

/* Function Prototypes...                                                  */
int   terminal( void );

/*#define DEBUG*/
#ifdef DEBUG
/***************************************************************************
Function:      main
Purpose:       calling terminal function for debugging purposes
Arguments:     none
Returns:       stauts of terminal routine
****************************************************************************/
int main( void )
{
	return( terminal() );
}  /* end main */
#endif

/***************************************************************************
Function:      terminal
Purpose:       make PC act like a simple terminal
Arguments:     none
Returns:       1 if ok; 0 if anything fails
****************************************************************************/
int terminal( void )
{
	int      Status, keep_going, i, comport, base_addr, irq_num, baud;
	char     TX_string[TXBUFSIZE], RX_string[RXBUFSIZE], mode[100];
	SWCCB    *pPort;
	POPWIN   *term;
	struct   text_info orig;
	FILE		*tc;

	/* get original screen configuration...                                 */
	gettextinfo( &orig );

	/* get the communications information from the terminal config. file...	*/
	if( (tc = getfp( "terminal.txt", "r" )) == NULL )
		return( FAIL );

	fscanf( tc, "%s", &mode );
	fscanf( tc, "%d", &comport );
	fscanf( tc, "%d", &baud );
	fscanf( tc, "%x", &base_addr );
	fscanf( tc, "%d", &irq_num );
	fclose( tc );

	/* open the communications port...                                      */
	comport -= 1;  /* silvercomm uses this as an index in some arrays
							COM 1 is 0, COM 2 is 1, etc									*/
	usStdUARTBaseIOAddress[comport] = base_addr;		/* address of com board	*/
	usStdIRQNumber[comport] = irq_num;	/* IRQ being used for com board		*/

	pPort=SWCOpenComm( comport, RXBUFSIZE, TXBUFSIZE, RTSDTR, &Status);

	/* If port can not be opened successfully, tell the user why...         */
	if( !pPort )
	{
		printf( "\n\n Error %s returned by SWCOpenComm()\n",
																SWCErrorToText( Status, 0 ) );
		printf( "\n\n Press any key to end..." );
		getch();
		return( FAIL );
	}

	/*  Initialize the UART for communicating with an EF Data modem...      */
	Status = SWCSetUART( pPort, (ULONG)baud, SWPARITYEVEN, 7, 2 );
	if( Status == SWCINVALIDPARAMETER )
	{
		printf( "\n\n There was an invalid parameter passed to the SWCSetUART" );
		printf( " function...\n\n" );
		printf( " Hit any key to end." );
		getch();
		return( FAIL );
	}

	/* make a window for the terminal...                                    */
	textcolor( TERM_FORE );
	textbackground( TERM_BACK );
	if( (term = PWALLOC) == NULL )
	{
		printf( "\n\n Can't allocate memory for terminal window." );
		printf( "\n\n Press any key to end..." );
		getch();
		return( FAIL );
	}
	term -> lft = 5;
	term -> top = 5;
	/* make the window (MAX_WIDTH - left column) X (ROWS)...                */
	for( i = 0; i < (MAX_WIDTH - term -> lft) * ROWS; ++i )
		term -> mes[i] = ' ';
	term -> mes[i] = '\0';
	term = popwin( term, ON );

	window( term -> lft, term -> top, term -> ryt, term -> bot );
	clrscr();
	gotoxy( 1, 1 );
	cprintf( "%s Serial Communications are on.  Enter \"QUIT\" to exit.",
																							mode );
	gotoxy( 1, 2 );
	cprintf( "%s\r\n\r\n", version );

	keep_going = 1;
	while( keep_going )
	{
		cprintf( "\r\n Enter Your Command:    " );
		gets( TX_string );
		if( !strcmp( TX_string, "QUIT" ) || !strcmp( TX_string, "quit" ) )
			keep_going = 0;
		else
		{
			strcat( TX_string, "\r\n" );
			SWCTransmitString( pPort, (unsigned char *)TX_string );
			cprintf( "\r\n You Entered:           %s", TX_string );

			cprintf( "\r\n The Modem Returned:    " );
			while( ( Status =
						SWCReceiveString( pPort, RX_string,
												TXBUFSIZE,    /* buffer size in bytes    */
												TERMINATOR, /* terminating character   */
												TIMEOUT,    /* seconds to timeout      */
												SWFALSE,    /* do not abort w/o carrier*/
												SWTRUE,     /* echo characters when RX */
												0,          /* Not used when echo on   */
												ALTXKEY     /* abort key               */
												 ) ) ==  SWCNOCARRIER );
			switch( Status )
			{
				case SWCMAXLENGTHREACHED:
					printf( "\n\n The maximum number of characters was reached." );
					break;
				case SWCTERMINATORREACHED:
					break;
				case SWCNOCARRIER:
					printf( "\n\n There is no carrier." );
					break;
				case SWCTIMEDOUT:
					gotoxy( 5, wherey() + 2 );
					cprintf( "Timeout!  Check Cable Connections and/or Jumpers." );
					break;
				default:
					printf( "\n\n The status is not defined." );
					break;
			}  /* end switch */

			/* turn all cr into lf...                                         */
			i = 0;
			while( RX_string[i] != TERMINATOR )
				if( RX_string[i] == '\r' )
					RX_string[i++] = '\n';
				else
					++i;
			RX_string[i] = '\0';    /* get rid of the terminating character   */

			/* put the reponse on the screen...                               */
			cprintf( "%s\n", RX_string );
		}  /* end else */
	}  /* end while */

	/*  Close the communications port...                                    */
	SWCCloseComm( pPort, 0 );

	/* turn the terminal window off and restore the screen...               */
	term = popwin( term, OFF );
	restore_scrn( orig );

	return( OK );
}  /* end terminal */