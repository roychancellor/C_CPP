/***************************************************************************
*  Module:        RSLCOARS.C                                               *
*                                                                          *
*  Created By:    Roy Chancellor                                           *
*                                                                          *
*  Date Created:  04-01-1994                                               *
*                                                                          *
*  Last Modified: 05-17-1994                                               *
*                                                                          *
*  Version:       1.00                                                     *
*                                                                          *
*  Date Released: 05-10-1994                                               *
*                                                                          *
*  Description:   gets I2C & ADC values and makes I2C = ADC + 5            *
*                                                                          *
*  Inputs:        none                                                     *
*                                                                          *
*  Returns:       1 if ok, 0 if any error occurred                         *
*                                                                          *
*  Date           Modification                                    Initials *
* ----------   -------------------------------------------        ---------*
* 05-10-1994   RELEASED FOR USE ON A CAC AS VERSION 1.00             RSC   *
****************************************************************************/
/* Include files that came with Turbo C...                                 */
#include <stdio.h>      /* for all C programs                              */
#include <stdlib.h>     /* for standard definitions                        */
#include <string.h>     /* for operations on strings                       */
#include <conio.h>      /* for screen I/O                                  */
#include <dos.h>        /* for enable/disable                              */

/* custom include files written for project...                             */
#include <boolean.h>    /* values that are 1 or 0                          */
#include <pc2modem.h>   /* for defines used in pc2modem                    */
#include <utility.h>    /* for restore_scrn                                */
#include <popwin.h>     /* for calls to popwin                             */
#include <ascii.h>      /* for ascii key scan codes                        */
#include <hex2deci.h>   /* for calls to hex2dec_int                        */

/* Include files from SILVERCOMM...                                        */
#include <swcasync.h>
#include <swkeys.h>

/* function prototypes...                                                  */
int   modem_communications( char *, char [] );
int   RX_signal_coarse_adjust( void );
int   change_i2c_value( void );
int   timeout_error( void );

/* GLOBAL variables...                                                     */
static SWCCB   *pPort;
static int		beep_enable;

/***************************************************************************
Function:      main
Purpose:       to call the RX_signal_coarse_adjust routine
Arguments:     none
Returns:       1 if ok; 0 if anything fails
****************************************************************************/
int main( void )
{
	int      change_value, valid;
	struct   text_info orig;
	POPWIN   *rsl;

	/* get the original screen configuration...                             */
	gettextinfo( &orig );

	/* ask the user if he/she really wants to set the I2C value...          */
	if( (rsl = PWALLOC) == NULL )
	{
		/* define a window for this message...											*/
		window( 5, 5 ,60, 10 );
		clrscr();
		gotoxy( 2, 2 );
		cprintf( "Not enough memory to make a popup window." );
		gotoxy( 2, 4 );
		cprintf( "Hit a key." );
		getch();

		restore_scrn( orig );
		return( FAIL );
	}  /* end if */

	rsl -> lft = 10;
	rsl -> top = 10;
	rsl -> numrows = 3;
	strcpy( rsl -> mes, "Do you really want to change the I2C value????" );
	strcpy( rsl -> cfgstr, rsl -> mes );
	strcat( rsl -> mes, "\n\r\n\r [Y]-Yes, continue  [N]-No, Don't change" );
	printf( "\a" );
	rsl = popwin( rsl, ON );

	valid = NO;
	while( !valid )
	{
		valid = YES;
		switch( getch() )
		{
			case 'y':case 'Y':
				change_value = YES;
				break;
			case 'n':case 'N':
				change_value = NO;
				break;
			default:
				valid = NO;
		}  /* end switch */
	}  /* end while */

	rsl = popwin( rsl, OFF );
	free( rsl );

	if( change_value )
	{
		if( RX_signal_coarse_adjust() == FAIL )
		{
			restore_scrn( orig );
			return( FAIL );
		}
		return( OK );
	}
	else
	{
		restore_scrn( orig );
		return( OK );
	}
}  /* end main */

/***************************************************************************
Function:      RX_signal_coarse_adjust
Purpose:       the main routine of the program
Arguments:     none
Returns:       1 if ok; 0 if any error occurred
****************************************************************************/
int   RX_signal_coarse_adjust( void )
{
	int      status;
	struct	comm_config cc;	/* defined in pc2modem.h							*/

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

	/* change the I2C value to ADC value + 5...                             */
	status = change_i2c_value();

	/* close the open com port...                                           */
	SWCCloseComm( pPort, 0 );

	return( status );
}  /* end RX_signal_coarse_adjust */

/***************************************************************************
Function:      change_i2c_value
Purpose:       to read I2C value, read ADC value and change I2C to ADC + 5
Arguments:     none
Returns:       1 if all ok; 0 if any error occurred
****************************************************************************/
int   change_i2c_value( void )
{
	int   i, j, i2c_decint, adc_decint;
	char  i2c_cmd[MAXSTR];
	char  *adc_cmd = "<1/TST1_\r\n";
	char  i2c_resp[20], adc_resp[20], ADC[3], I2C[3], bp[3], i2cplus5[8];

	/* get the current I2C value from the modem...									*/
	sprintf( i2c_cmd, "<1/TST0_%s%s%s%s", "00", "\0", "\0", "\r\n" );
	if( modem_communications( i2c_cmd, i2c_resp ) == FAIL )
		return( FAIL );

	if( i2c_resp[0] == '?' )
	{
		printf( "\n\n The modem returned %s.  Hit any key to end", i2c_resp );
		getch();
	}

	/* obtain the I2C number from the modem response...							*/
	i = 0;
	j = 0;

	/* ...breakpoint first...																*/
	while( i2c_resp[i++] != '_' );
	while( i2c_resp[i] != '_' )
		bp[j++] = i2c_resp[i++];
	bp[j] = '\0';

	/* ...then the I2C value...															*/
	j = 0;
	++i;     /* skip the '_'                                                */
	while( i2c_resp[i] != '\n' )
		I2C[j++] = i2c_resp[i++];
	I2C[j] = '\0';

	/* get the current ADC value from the modem...									*/
	if( modem_communications( adc_cmd, adc_resp ) == FAIL )
		return( FAIL );

	if( adc_resp[0] == '?' )
	{
		printf( "\n\n The modem returned %s.  Hit any key to end", adc_resp );
		getch();
	}

	/* obtain the ADC value from the modem response...								*/
	i = 0;
	while( adc_resp[i++] != '_' );
	j = 0;
	while( adc_resp[i] != '\n' )
		ADC[j++] = adc_resp[i++];
	ADC[j] = '\0';

	/* convert the I2C & ADC values from strings which represent hex values
		to base 10 integers having the same value...									*/
	i2c_decint = hex2dec_int( I2C, "hex" );
	adc_decint = hex2dec_int( ADC, "hex" );

	/* change the I2C value in the modem to the ADC value + 5...				*/
	i2c_decint = adc_decint + 5;

	/* convert the base 10 integer back to a string which represents the
		value in hexadecimal...																*/
	dec2hex_str( i2c_decint, i2cplus5, "hex" );

	sprintf( i2c_cmd, "<1/REM_\r\n" );
   if( modem_communications( i2c_cmd, i2c_resp ) == FAIL )
		return( FAIL );

	sprintf( i2c_cmd, "<1/TST0_%s%s%s%s", bp, "_", i2cplus5, "\r\n" );
	if( modem_communications( i2c_cmd, i2c_resp ) == FAIL )
		return( FAIL );

	if( i2c_resp[0] == '?' )
	{
		printf( "\n\n The modem returned %s after change.  Hit any key to end",
																						i2c_resp );
		getch();
		return( FAIL );
	}

	return( OK );
}  /* end change_i2c_value */

/***************************************************************************
Function:      modem_communications
Purpose:       to send command/receive response to/from modem
Arguments:     command, resp
Returns:       1 if ok, 0 if anything fails
****************************************************************************/
int   modem_communications( char *cmd, char resp[] )
{
	ULONG chars_TX;
	int   status, length, i, c;

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

	return( OK );
}  /* end modem_communications */

/**************************************************************************
Function:      timeout_error
Purpose:       to respond to a timeout error
Arguments:     none
Returns:       1 if ok; 0 if anything failed
***************************************************************************/
int timeout_error( void )
{
	int      valid, status;
	POPWIN   *to;

	disable();
	if( beep_enable )
		printf( "\a" );

	if( (to = PWALLOC) == NULL )
	{
		/* define a window for this message...											*/
		window( 5, 5, 60, 10 );
		clrscr();
		gotoxy( 2, 2 );
		cprintf( "Can't allocate memory to say that the modem Timed Out." );
		gotoxy( 2, 4 );
		cprintf( "Press any key to end..." );
		getch();

		return( FAIL );
	}  /* end if */

	strcpy( to -> mes,
					"The computer and the modem are not communicating.\n\r" );
	strcpy( to -> cfgstr, to -> mes );
	strcat( to -> mes, "\n\r (1) Check the cable connection." );
	strcat( to -> mes, "\n\r (2) Check for the correct port on the PC." );
	strcat( to -> mes, "\n\r (3) Check for the correct port on the modem." );
	strcat( to -> mes, "\n\r (4) Check the jumpers on the modem's M&C." );
	strcat( to -> mes, "\n\r     (Must be RS-485 for normal operation)" );
	strcat( to -> mes, "\n\r\n\r [ESC]-QUIT  [R]-Retry" );
	to -> lft      = 10;
	to -> top      = 15;
	to -> numrows  = 9;
	to = popwin( to, ON );

	valid = NO;
	while( !valid )
	{
		valid    = YES;
		status   = OK;
		switch( getch() )
		{
			case ESC:
				status = FAIL;
				break;
			case 'r':case 'R':
				status = RESTART;
				break;
			default:
				valid = NO;
				break;
		}  /* end switch */
	}  /* end while */
	to = popwin( to, OFF );
	free( to );
	enable();

	return( status );
}  /* end timeout_error */