/***************************************************************************
*  Module:        MONVALUE.C                                               *
*                                                                          *
*  Created By:    Roy Chancellor                                           *
*                                                                          *
*  Date Created:  03-10-1994                                               *
*                                                                          *
*  Last Modified: 05-10-1994                                               *
*                                                                          *
*  Version:       1.00                                                     *
*                                                                          *
*  Date Released: 05-17-1994                                               *
*                                                                          *
*  Description:   Monitors the following signals/values on screen:  EBN0   *
*                 RX signal level, ADC & I2C values                        *
*                                                                          *
*  Inputs:        none                                                     *
*                                                                          *
*  Returns:       1 if ok, 0 if any error occurred                         *
*                                                                          *
*  Date           Modification                                    Initials *
* ----------   -------------------------------------------        ---------*
*  03-11-1994  released as version 0.00                              RSC   *
*  03-31-1994  added restore_scrn; changed all communications        RSC   *
*              to SILVERCOMM routines with no calls to pc2modem.           *
*              Version to 0.01.                                            *
*  04-01-1994  added ability to monitor the I2C value and ADC value. RSC   *
*              ver to 0.02.                                                *
*  04-04-1994  changed ADC/I2C values to base 10; ver to 0.03        RSC   *
*	05-10-1994	RELEASED FOR USE ON CAC AS VERSION 1.00					RSC	*
****************************************************************************/
static char *version = "Version 1.00";

/* Include files that came with Turbo C...                                 */
#include <stdio.h>      /* for all C programs                              */
#include <stdlib.h>     /* for standard definitions                        */
#include <string.h>     /* for operations on strings                       */
#include <conio.h>      /* for screen I/O                                  */
#include <dos.h>			/* for calls to sleep										*/

/* custon include files written for project...                             */
#include <boolean.h>    /* values that are 1 or 0                          */
#include <ascii.h>      /* ansi key codes                                  */
#include <pc2modem.h>   /* for defines used in pc2modem                    */
#include <monvalue.h>   /* has anything that must be known by other progs  */
#include <popwin.h>     /* for calls to popwin                             */
#include <hex2deci.h>   /* for calls to hex2dec_int                        */
#include <utility.h>		/* for calls to restore_scrn								*/

/* Include files from SILVERCOMM...                                        */
#include <swcasync.h>
#include <swkeys.h>

#define MAX_RESPLEN  (int)strlen( "sampling" )
#define EBNO         1
#define RXSIGNAL     2
#define ADCI2C       3
#define KEY_HIT      4

/* Function prototypes...                                                  */
void  program_control( int * );
int   show_value( char *, char * );
char  *take_picture( char *, char *, char * );
int   monitor( char *, char [] );
int   timeout_error( void );	/* declared in pc2modem								*/

/* GLOBAL variables...                                                     */
static POPWIN  *monwin;
static SWCCB   *pPort;
static int     exit_key;

/***************************************************************************
Function:      monitor_values
Purpose:       the main routine of the program
Arguments:     value string:  "ebno", "rxsignal", or "ADC_I2C"
Returns:       1 if ok; 0 if any error occurred
****************************************************************************/
int   monitor_values( char *what_value )
{
	int      status, show_stat;
	char     *monbuf;
	struct   text_info orig;
	struct	comm_config cc;		/* defined in pc2modem...						*/

	/* get the original screen configuration...                             */
	gettextinfo( &orig );

	/* take a picture of the screen where the display will go...            */
	monbuf = NULL;
	if( (monbuf = take_picture( what_value, "on", monbuf )) == NULL )
		return( FAIL );

	/* configure the screen for display of ebno...                          */
	if( show_value( what_value, "config" ) == FAIL )
		return( FAIL );

	/* define the communications parameters for this program...					*/
	cc.port      	= RS485_PORT_DEFAULT;
	cc.baud      	= RS485_BAUD_DEFAULT;
	cc.base_addr	= RS485_BASE_ADDR_DEFAULT;
	cc.irq			= RS485_IRQ_DEFAULT;

	/* user enters ports as 1, 2, 3, etc; Silvercom uses this number as an
		array index, i.e. 0, 1, 2, etc.  Change physical port to index...		*/
	cc.port -= 1;

	/* set the base address of the com board being used (for Silvercom)...	*/
	usStdUARTBaseIOAddress[cc.port] = cc.base_addr;

	/* set the interrupt request line (IRQ) being used (for Silvercom)...	*/
	usStdIRQNumber[cc.port] = cc.irq;

	/* open the com port and configure the UART...                          */
	pPort = SWCOpenComm( cc.port, RXBUFSIZE, TXBUFSIZE, RTSDTR, &status );
	if( !pPort )
	{
		/* define a window for this message...											*/
		window( 5, 5, 60, 10 );
		clrscr();
		gotoxy( 2, 2 );
		cprintf( "Error opening comm port #%d.  SWCOpenComm returned %s",
											cc.port + 1, SWCErrorToText( status, 0 ) );
		gotoxy( 2, 4 );
		cprintf( "Hit any key to end..." );
		getch();

		return( FAIL );
	}  /* end if */

	/* set the communications board UART parameters...								*/
	status = SWCSetUART( pPort, (ULONG)cc.baud, SWPARITYEVEN, 7, 2 );
	if( status == SWCINVALIDPARAMETER )
	{
		/* define a window for this message...											*/
		window( 5, 5, 60, 10 );
		clrscr();
		gotoxy( 2, 2 );
		cprintf( "SWCSetUART returned an error:  SWCINVALIDPARAMETER." );
		gotoxy( 2, 4 );
		cprintf( "Press any key to end..." );
		getch();

		SWCCloseComm( pPort, 0 );
		return( FAIL );
	}  /* end if */

/****************************MAIN LOOP**************************************/
	show_stat = OK;
	show_stat = show_value( what_value, "show" );
/***************************************************************************/

	/* return the screen to its original configuration...                   */
	monbuf = take_picture( what_value, "off", monbuf );
	restore_scrn( orig );
	free( monwin );

	/* close the open com port...                                           */
	SWCCloseComm( pPort, 0 );

	return( show_stat );
}  /* end monitor_values */

/***************************************************************************
Function:      show_value
Purpose:       to read "value" and put it on the screen
Arguments:     value string:  "ebno", "rxsignal", or "ADC_I2C"; string of
					what to do
Returns:       1 if all ok; 0 if any error occurred
****************************************************************************/
int   show_value( char *what_value, char *task )
{
	int         i, j, keep_searching, difflen, type, keep_going;
	int         num_times;
	static char *prompt     = "\r\n\r\n Press [X] to End...    ";
	static char *ebno       = "EBN0:";
	static char *rxsignal   = "RX Signal Level:";
	static char *adc        = "ADC:";
	static char *i2c        = "I2C:";
	static char *ebnocmd    = "<1/EBN0_\r\n";
	static char *rxcmd      = "<1/RSL_\r\n";
	static char *i2ccmd     = "<1/TST0_00\r\n";
	static char *adccmd     = "<1/TST1_\r\n";
	char        resp[40], val[20], cmd[6], cmdstr[11], valuestr[18], val1[20];
	char        val1str[8], val2str[8], tempresp[40], bp[3];

	if( !strcmp( what_value, "ebno" ) )
	{
		type = EBNO;
		strcpy( cmdstr, ebnocmd );
		strcpy( valuestr, ebno );
	}
	else if( !strcmp( what_value, "rxsignal" ) )
	{
		type = RXSIGNAL;
		strcpy( cmdstr, rxcmd );
		strcpy( valuestr, rxsignal );
	}
	else if( !strcmp( what_value, "ADC_I2C" ) )
	{
		type = ADCI2C;
		strcpy( cmdstr, adccmd );
		strcpy( val1str, adc );
		strcpy( val2str, i2c );
	}

	if( !strcmp( task, "show" ) )
	{
		keep_going  = YES;
		num_times   = 0;
		while( keep_going )
		{
			/* check for a key hit...                                         */
			if( kbhit() )
			{
				exit_key = getch();
				program_control( &keep_going );
				if( keep_going == NO )
					return( OK );
			}

			/* get the value to monitor from the modem...                     */
			strcpy( resp, "" );
			if( monitor( cmdstr, resp ) == FAIL )
				return( FAIL );
			if( type == ADCI2C )
			{
				strcpy( cmdstr, i2ccmd );
				if( monitor( cmdstr, tempresp ) == FAIL )
					return( FAIL );
				strcat( resp, tempresp );
			}  /* end if */

			/* obtain just the value from the response string...              */
			keep_searching = YES;
			i = 0;
			while( keep_searching )
			{
				while( resp[i++] != '>' );
				while( resp[i++] != '/' );
				j = 0;
				while( resp[i] != '_' )
					cmd[j++] = resp[i++];
				cmd[j] = '\0';

				if( !strcmp( cmd, "EBN0" ) || !strcmp( cmd, "RSL" ) )
				{
					while( resp[i++] != '_' );
					j = 0;
					while( resp[i] != '\n' )
						if( resp[i] != '\r' )
							val[j++] = resp[i++];
						else
							++i;
					val[j] = '\0';
					keep_searching = NO;
				}  /* end if */
				else if( !strcmp( cmd, "TST1" ) )
				{
					while( resp[i++] != '_' );
					j = 0;
					while( resp[i] != '\n' )
						if( resp[i] != '\r' )
							val[j++] = resp[i++];
						else
							++i;
					val[j] = '\0';
				}  /* end else-if */
				else if( !strcmp( cmd, "TST0" ) )
				{
					while( resp[i++] != '_' );
					j = 0;
					while( resp[i] != '_' )
						bp[j++] = resp[i++];
					bp[j] = '\0';
					j = 0;
					++i;
					while( resp[i] != '\n' )
						val[j++] = resp[i++];
					val[j] = '\0';
				}  /* end else-if */
				++num_times;
				if( num_times == 1 )
					strcpy( val1, val );
				else
					keep_searching = NO;
			}  /* end while */

			/* put the value on the screen...                                 */
			switch( type )
			{
				case EBNO:case RXSIGNAL:
					difflen = MAX_RESPLEN;
					for( i = 0; i < difflen; ++i )
						strcat( val, " " );
					gotoxy( (int)strlen( valuestr ) + 2, 1 );
					cprintf( val );
					gotoxy( (int)strlen( valuestr ) + 2, 1 );
					break;
				case ADCI2C:
					gotoxy( (int)strlen( val1str ) + 2, 1 );
					cprintf( "%d", hex2dec_int( val1, "hex" ) );
					gotoxy( (int)strlen( val2str ) + 2, 3 );
					cprintf( "%d", hex2dec_int( val, "hex" ) );
					break;
			}  /* end switch */
		}  /* end while */
	}  /* end if */
	else if( !strcmp( task, "config" ) )
	{
		/* create a popup window on the screen...                            */
		if( (monwin = PWALLOC) == NULL )
			return( FAIL );
		textcolor( WHITE );
		textbackground( LIGHTBLUE );
		switch( type )
		{
			case EBNO:
				monwin -> lft        = EBNO_LFT;
				monwin -> top        = EBNO_TOP;
				monwin -> numrows    = 5;
				strcpy( monwin -> mes, ebno );
				strcat( monwin -> mes, prompt );
				strcpy( monwin -> cfgstr, prompt );
				strcat( monwin -> mes, "\n\r\n\r " );
				strcat( monwin -> mes, version );
				break;
			case RXSIGNAL:
				monwin -> lft        = RX_LFT;
				monwin -> top        = RX_TOP;
				monwin -> numrows    = 5;
				strcpy( monwin -> mes, rxsignal );
				strcat( monwin -> mes, prompt );
				strcpy( monwin -> cfgstr, prompt );
				strcat( monwin -> mes, "\n\r\n\r " );
				strcat( monwin -> mes, version );
				break;
			case ADCI2C:
				monwin -> lft        = ADCI2C_LFT;
				monwin -> top        = ADCI2C_TOP;
				monwin -> numrows    = 7;
				strcpy( monwin -> mes, adc );
				strcat( monwin -> mes, "\n\r\n\r " );
				strcat( monwin -> mes, i2c );
				strcat( monwin -> mes, prompt );
				strcpy( monwin -> cfgstr, prompt );
				strcat( monwin -> mes, "\n\r\n\r " );
				strcat( monwin -> mes, version );
				break;
		}  /* end switch */
		monwin = popwin( monwin, ON );
		window( monwin -> lft, monwin -> top, monwin -> ryt + 1,
																			monwin -> bot + 1 );
	}  /* end else-if */
	return( OK );
}  /* end show_ebno */

/***************************************************************************
Function:      program_control
Purpose:       to act on user input
Arguments:     ptr to keep_going flag
Returns:       none
****************************************************************************/
void  program_control( int *kg )
{
	int   valid;

	valid = NO;
	while( !valid )
	{
		valid = YES;
		switch( exit_key )
		{
			case 'x':case 'X':
				*kg = NO;
				break;
			default:
				valid = NO;
				break;
		}  /* end switch */
	}  /* end while */
}  /* end program_control */

/***************************************************************************
Function:      take_picture
Purpose:       takes a picture of main menu where the monitor window goes.
Arguments:     value string:  "ebno" or "rxsignal"; "on"/"off"
Returns:       ptr to buffer if all ok; NULL if anything failed.
****************************************************************************/
char  *take_picture( char *what_value, char *pswitch, char *monbuf )
{
	int   monbuf_size, lft, ryt, top, bot;
	char  *tmpbuf;

	if( !strcmp( what_value, "ebno" ) )
	{
		lft = EBNO_LFT;
		ryt = EBNO_RYT;
		top = EBNO_TOP;
		bot = EBNO_BOT;
	}
	else if( !strcmp( what_value, "rxsignal" ) )
	{
		lft = RX_LFT;
		ryt = RX_RYT;
		top = RX_TOP;
		bot = RX_BOT;
	}
	else if( !strcmp( what_value, "ADC_I2C" ) )
	{
		lft = ADCI2C_LFT;
		ryt = ADCI2C_RYT;
		top = ADCI2C_TOP;
		bot = ADCI2C_BOT;
	}

	if( !strcmp( pswitch, "on" ) )
	{
		monbuf_size = (ryt - lft + 10) * (bot - top + 10) * 2;
		if( (tmpbuf = calloc( (size_t)1, (size_t)monbuf_size )) == NULL )
		{
			printf( "\n\n Could not allocate memory to store main menu image.\n" );
			getch();
			return( NULL );
		}
		if( gettext( lft - 2, top - 2, ryt + 4, bot + 4, tmpbuf ) );
		else
			return( NULL );
	}  /* end if */
	else if( !strcmp( pswitch, "off" ) )
	{
		puttext( lft - 2, top - 2, ryt + 4, bot + 4, monbuf );
		free( monbuf );
		return( monbuf );
	}  /* end else-if */

	return( tmpbuf );
}  /* end take_picture */

/***************************************************************************
Function:      monitor
Purpose:       to send command/receive response to/from modem
Arguments:     command, resp
Returns:       1 if ok, 0 if anything fails
****************************************************************************/
int   monitor( char *cmd, char resp[] )
{
	ULONG chars_TX;
	int   status, length, i, c, return_status;

   /* check for a key hit...                                               */
	if( kbhit() )
	{
		exit_key = getch();
		return_status = KEY_HIT;
	}
	else
		return_status = OK;

   /* Assert the RTS before sending commands...										*/
	SWCControlRTS( pPort, ON );
   /* TRANSMIT the command string to modem...                              */
	chars_TX = SWCTransmitString( pPort, (unsigned char *)cmd );
	if( chars_TX != (int)strlen( cmd ) )
	{
		/* define a window for this message...											*/
		window( 5, 5, 60, 10 );
		clrscr();
		gotoxy( 2, 2 );
		cprintf( "Error in SWCTransmitString.  The number of characters" );
		gotoxy( 2, 3 );
		cprintf( "transmitted was not the same as the command length." );
		gotoxy( 2, 5 );
		cprintf( "Press any key to end..." );
		getch();

		return( FAIL );
	}  /* end if */

	/* RECEIVE the response from the modem...                               */
	status = SWCReceiveString( pPort, resp,
										RXBUFSIZE,  /* buffer size in bytes          */
										TERMINATOR, /* terminating character         */
										TIMEOUT,    /* seconds to timeout            */
										SWFALSE,    /* don't abort w/o carrier       */
										SWFALSE,    /* don't echo char when RX       */
										0,          /* Not used when echo on         */
										ALTXKEY );  /* abort key                     */
	switch( status )
	{
		case SWCTERMINATORREACHED:		/* what you want								*/
			break;
		case SWCMAXLENGTHREACHED:
			printf( "\n\n The modem's response exceeded the length of" );
			printf( " the RX buffer.\n\n Programmer must increase it." );
			sleep( 3 );
			return( FAIL );
		case SWCNOCARRIER:		/* won't ever happen									*/
			printf( "\n\n There is no carrier\n\n." );
			sleep( 3 );
			return( FAIL );
		case SWCTIMEDOUT:			/* VERY IMPORTANT!!									*/
			strcpy( resp, ">1/NULL\r\n" );
			return( timeout_error() );
		case -485:  /* termination with ALT-X (doesn't work properly)			*/
			/* read the terminating character from the modem...      */
			c = SWCReceiveCharacter( pPort );
			while( c != SWCQUEUEISEMPTY )
				c = SWCReceiveCharacter( pPort );
			return( FAIL );
		default:
			break;
	}  /* end switch */

	/* replace all CR with CRLF and the TERMINATOR with a CRLF...  */
	length = (int)strlen( resp );
	for( i = 0; i < length; ++i )
		if( resp[i] == '\r' || resp[i] == TERMINATOR )
			resp[i] = '\n';

	return( return_status );
}  /* end monitor */