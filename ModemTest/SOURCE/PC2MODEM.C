/***************************************************************************
*  Module:        PC2MODEM.C                                               *
*                                                                          *
*  Created By:    Roy Chancellor                                           *
*                                                                          *
*  Date Created:  01-18-1994                                               *
*                                                                          *
*  Last Modified: 05-17-1994                                               *
*                                                                          *
*  Version:       1.00                                                     *
*                                                                          *
*  Date Released: 05-06-1994                                               *
*                                                                          *
*  Description:   Sends and receives data from SDM-100 modem.              *
*                                                                          *
*  Inputs:        command file path (on command line)                      *
*                                                                          *
*  Returns:       1 if successful, 0 if an error occurred                  *
*                                                                          *
*  Date           Modification                                    Initials *
* ----------   -------------------------------------------------- ---------*
* 03-09-1994   Released as version 0.00                              RSC   *
* 03-21-1994   Added error reporting feature; ver to 0.01            RSC   *
* 03-22-1994   Added SILVRCOM routines for comm'cation; ver to 0.5   RSC   *
* 03-28-1994	Fixed problem of putting number in upper-left corner	RSC	*
*					when syntax error occurs.  Ver to 0.51								*
* 03-29-1994	Added feature to overlook comments in command scripts	RSC	*
*					Version to 0.52															*
* 03-31-1994	added free commands for cmds2modem variable that is	RSC	*
*					dynamically allocated.  ver to 0.53									*
* 04-01-1994	fixed the terminate test function (Del key)				RSC	*
*					Version to 0.54.															*
* 04-06-1994	free allocated memory when things fail; ver to 0.55	RSC	*
* 04-15-1994	added a quiet feature for pauses, etc; ver to 0.56		RSC	*
* 05-06-1994	RELEASED FOR USE ON CAC AS VERSION 1.00        			RSC	*
****************************************************************************/
static char *version = "Version 1.00";

/* Include files that are part of Turbo C...                               */
#include <stdio.h>
#include <dir.h>        /* for calls to getcwd and use of MAXPATH          */
#include <conio.h>      /* for screen calls                                */
#include <dos.h>        /* for enable/disable										*/
#include <stdlib.h>     /* for many calls                                  */
#include <string.h>     /* for calls to strcpy, strcat                     */
#include <alloc.h>      /* for calls to calloc                             */

/* Include files custom written for EF Data Test Project...                */
#include <boolean.h>    /* for values that can be 1 or 0                   */
#include <ascii.h>      /* keyboard return values                          */
#include <popwin.h>     /* for making popup messages                       */
#include <utility.h>    /* for calls to getfp, str2int                     */
#include <pc2modem.h>   /* for calls to pc2modem                           */

/* Include files that came with Silvercom...                               */
#include <swcasync.h>
#include <swkeys.h>

/* GLOBAL variable definitions...                                          */
static POPWIN     *cmdwin;
static int        cmdindex;
static int        cmdrow;
static SWCCB      *pPort;
static int			beep_enable;

/* function prototypes...                                                  */
int   communicate( struct comm_config, char *, char [], char [] );
int   end_test( int * );
int   pause_comm( char * );
int   break_test_notify( void );
int   timeout_error( void );
FILE  *dump_status( char [], char [], char [], FILE * );
int   get_comm_config( struct comm_config *, int, int );
int   count_commands( FILE *, int, char *, int *, int * );
void  get_test_type( char *, char *, int );
char  *get_commands( int *, char *, int, char * );
int   cmdstr2cmd( char *, char * );
int   verify_cmd_rsp( char *, char * );
int   put_cmd_onscr( char * );
int   user_termination( void );
int   command_syntax_error( char * );
int   check_baud( int );

/*#define DEBUG*/
#ifdef DEBUG
void main( void )
{
	char all_responses[RXBUFSIZE];

	pc2modem( "d:\\debug.cmd", "", "", CMDFILE, /*RS232*/RS485 );
	/*getch();
	pc2modem("",
		"<1/REM_\r\n<1/MOP_-10.0\r\n$Waiting for you.\n<1/MOP_-5.0\r\n!\r\n",
		all_responses, ARGUMENT, RS485 );*/
}
#endif

/***************************************************************************
Function:      pc2modem
Purpose:       send commands to and receive responses from modem
Arguments:     name of command file, commands to send to modem (if not from
					file), responses buffer (commands not from file), command mode
					(cmdfile or argument), communications mode (RS485 or RS232).
Returns:       1 if OK, 0 if FAIL
****************************************************************************/
int pc2modem( char *cmdfile, char *commands, char *all_responses,
															int cmd_mode, int comm_mode )
{
	FILE     *stat;
	int      keep_going, cmdcnt, numcmds, i;
	int      do_the_rest, return_status, status, com_status, com_verify;
	char     response[RXBUFSIZE], testtype[30], cur_cmd[MAXSTR];
	char     *cmds2modem;
	POPWIN   *onlyn, *ccs, *ttyp;
	struct   text_info orig;
	struct   comm_config cc;

/**************************** PROGRAM SETUP ********************************/
	/* initialize GLOBAL and LOCAL variables...										*/
	cmdindex       = 0;
	cmdcnt         = 0;
	cmdrow         = 1;
	cmds2modem     = NULL;
	keep_going     = YES;
	do_the_rest    = YES;
	return_status  = OK;
	strcpy( response, "" );
	stat				= NULL;

	/* get the original text window information for later restore_screen... */
	gettextinfo( &orig );

	/* set the colors for popup windows...                                  */
	textcolor( COMM_FORE );
	textbackground( COMM_BACK );

	/* get the modem configuration commands and store them in an array...	*/
	cmds2modem = get_commands( &numcmds, commands, cmd_mode, cmdfile );
	if( cmds2modem == NULL )
	{
		restore_scrn( orig );
		return( FAIL );
	}

	/* get the communications configuration information...                  */
	if( get_comm_config( &cc, cmd_mode, comm_mode ) == FAIL )
	{
		free( cmds2modem );
		restore_scrn( orig );
		return( FAIL );
	}  /* end if */

	/* make sure the user entered proper port, baud rate, base address, and
		IRQ number...																			*/
	if( verify_comm_params( &cc ) != OK )
	{
		free( cmds2modem );
		restore_scrn( orig );
		return( FAIL );
	}

	/* user enters ports as 1, 2, 3, etc; Silvercom uses this number as an
		array index, i.e. 0, 1, 2, etc.  Change physical port to index...		*/
	cc.port -= 1;

	/* set the base address of the com board being used (for Silvercom)...	*/
	usStdUARTBaseIOAddress[cc.port] = cc.base_addr;

	/* set the interrupt request line (IRQ) being used (for Silvercom)...	*/
	usStdIRQNumber[cc.port] = cc.irq;

   /* open the communications port and initialize it...                    */
	pPort = SWCOpenComm( cc.port, RXBUFSIZE, TXBUFSIZE, RTSDTR, &status );

	if( !pPort )      /* pPort is 0 if an error occurred                    */
	{
		/* define a window for this message...											*/
		window( 5, 5, 60, 10 );
		clrscr();	/* only clears the text window									*/
		gotoxy( 2, 2 );
		cprintf( "Error Opening Port %d...\n\n %s\n\n", cc.port,
																SWCErrorToText( status, 0 ) );
		gotoxy( 2, 4 );
		cprintf( "Hit any key to end." );
		getch();

		restore_scrn( orig );
		free( cmds2modem );

		return( FAIL );
	}  /* end if */

	/* set the communications UART for the proper parameters...             */
	status = SWCSetUART( pPort, (ULONG)cc.baud, SWPARITYEVEN, 7, 2 );
	if( status == SWCINVALIDPARAMETER )
	{
		/* set a window for this message...												*/
		window( 5, 5, 60, 10 );
		clrscr();
		gotoxy( 2, 2 );
		cprintf( "There was an invalid parameter in the call to SWCSetUART" );
		gotoxy( 2, 4 );
		cprintf( "Press any key to end..." );
		getch();

		SWCCloseComm( pPort, 0 );
		restore_scrn( orig );
		free( cmds2modem );

		return( FAIL );
	}  /* end if */

	/* put a message in a window on the screen telling the user that
		communications are on-line...														*/
	if( cc.verbose )
	{
		if( (onlyn = PWALLOC) == NULL )
		{
			/* set a window for this message...											*/
			window( 5, 5, 60, 10 );
			clrscr();
			gotoxy( 2, 2 );
			cprintf( "Can't allocate memory for the on-line window." );
			gotoxy( 2, 4 );
			cprintf( "Press any key to end." );
			getch();

			free( cmds2modem );
			restore_scrn( orig );
			SWCCloseComm( pPort, 0 );

			return( FAIL );
		}  /* end if */

		onlyn -> top = 6;
		onlyn -> lft = 5;
		sprintf( onlyn -> mes, "Port #%d opened at %u baud...",
																		cc.port + 1, cc.baud );
		strcat( onlyn -> mes,
				  "Modem test program is ON-LINE.\r\nPress DELETE to End Test." );
		onlyn = popwin( onlyn, ON );
	}  /* end if */

	/* configure the modem response file handler...                        */
	if( cc.dumpstat && cmd_mode == CMDFILE )
		if( (stat = dump_status( "", cmdfile, "config", stat )) == NULL )
		{
			free( cmds2modem );
			if( cc.verbose )
			{
				onlyn = popwin( onlyn, OFF );
				free( onlyn );
			}
			restore_scrn( orig );
			SWCCloseComm( pPort, 0 );

			return( FAIL );
		}  /* end if */

	/* tell the user what kind of test:  commands or status...              */
	/* allocate memory for ttyp...                                          */
	if( cmd_mode == CMDFILE )
	{
		if( (ttyp = PWALLOC) == NULL )
		{
			/* define a window for this message...										*/
			window( 5, 5, 60, 10 );
			clrscr();
			gotoxy( 2, 2 );
			cprintf( "Can't allocate memory for test type window." );
			gotoxy( 2, 4 );
			cprintf( "Press any key to end..." );
			getch();

			free( cmds2modem );
         if( cc.verbose )
			{
				onlyn = popwin( onlyn, OFF );
				free( onlyn );
			}
			if( cc.dumpstat && cmd_mode == CMDFILE )
				stat = dump_status( "", cmdfile, "close", stat );
			restore_scrn( orig );
			SWCCloseComm( pPort, 0 );

			return( FAIL );
		}  /* end if */

		/* decide what type of test is being run based on the command file
			name...																				*/
		get_test_type( cmdfile, testtype, cmd_mode );
		if( !strcmp( testtype, "modmstat" ) ||
							!strcmp( testtype, "finepowr" ) ||
														!strcmp( testtype, "rxsignal" ) )
			strcpy( testtype, "Getting Modem Status" );
		else
			strcpy( testtype, "Running Test" );

		ttyp -> top = 3;
		ttyp -> lft = 5;
		sprintf( ttyp -> mes, "%s:  %s...", version, testtype );
		ttyp = popwin( ttyp, ON );
	}  /* end if */

	/* if the program is to display commands, create a window for this...   */
	if( cc.showcmds )
	{
		/* allocate memory for cmdwin...                                     */
		if( (cmdwin = PWALLOC) == NULL )
		{
			/* define a window for this message...										*/
			window( 5, 5, 60, 10 );
			clrscr();
			gotoxy( 2, 2 );
			cprintf( "Can't allocate memory for command window." );
			gotoxy( 2, 4 );
			cprintf( "Press any key to end..." );
			getch();

         free( cmds2modem );
			if( cmd_mode == CMDFILE )
			{
				ttyp = popwin( ttyp, OFF );
				free( ttyp );
			}
			if( cc.verbose )
			{
				onlyn = popwin( onlyn, OFF );
				free( onlyn );
			}
			if( cc.dumpstat && cmd_mode == CMDFILE )
				stat = dump_status( "", cmdfile, "close", stat );
			restore_scrn( orig );
			SWCCloseComm( pPort, 0 );

			return( FAIL );
		}  /* end if */

		cmdwin -> lft = 5;
		cmdwin -> top = 10;

		/* make the window (MAX_WIDTH - left column) X (CMDWIN_ROWS)...      */
		for( i = 0; i < (MAX_WIDTH - cmdwin -> lft) * CMDWIN_ROWS; ++i )
			cmdwin -> mes[i] = ' ';
		cmdwin -> mes[i] = '\0';
		cmdwin = popwin( cmdwin, ON );
	}  /* end if */

	/* put a (C)ommand (C)omplete (S)tatus window on the screen...                */
	/* allocate memory for ccs...                                           */
	if( cc.show_stat_of_cmds )
	{
		if( (ccs = PWALLOC) == NULL )
		{
			/* define a window for this message...										*/
			window( 5, 5, 60, 10 );
			clrscr();
			gotoxy( 2, 2 );
			cprintf( "Can't allocate memory for CCS window." );
			gotoxy( 2, 4 );
			cprintf( "Press any key to end..." );
			getch();

			free( cmds2modem );
			if( cc.showcmds )
			{
				cmdwin = popwin( cmdwin, OFF );
				free( cmdwin );
			}
			if( cmd_mode == CMDFILE )
			{
				ttyp = popwin( ttyp, OFF );
				free( ttyp );
			}
         if( cc.verbose )
			{
				onlyn = popwin( onlyn, OFF );
				free( onlyn );
			}
			if( cc.dumpstat && cmd_mode == CMDFILE )
				stat = dump_status( "", cmdfile, "close", stat );
			restore_scrn( orig );
			SWCCloseComm( pPort, 0 );

			return( FAIL );
		}  /* end if */

		ccs -> top  = 3;
		ccs -> lft  = 47;
		sprintf( ccs -> mes, "%2.2d of %2.2d commands", cmdcnt, numcmds );
		ccs = popwin( ccs, ON );
		/* set window to the Command Complete Status window coords...  		*/
		window( ccs -> lft - ccs -> ofs, ccs -> top,
													ccs -> ryt + ccs -> ofs, ccs -> bot );
	}  /* end if */
/************************** END OF PROGRAM SETUP ***************************/

/************************ MAIN COMMUNICATIONS LOOP *************************/
	while( keep_going )
	{
		do_the_rest = YES;

		/* check for a key hit in the keyboard; shutdown if DELETE is hit...	*/
		if( kbhit() )
			if( getch() == DELETE )
			{
				/* shutdown procedure:  remove windows then close com port...	*/
				if( cc.show_stat_of_cmds )
				{
					ccs = popwin( ccs, OFF );
					free( ccs );
				}
				if( cc.showcmds )
				{
					cmdwin = popwin( cmdwin, OFF );
					free( cmdwin );
				}
				if( cmd_mode == CMDFILE )
				{
					ttyp = popwin( ttyp, OFF );
					free( ttyp );
				}
				if( cc.verbose )
				{
					onlyn = popwin( onlyn, OFF );
					free( onlyn );
				}
				if( cc.dumpstat && cmd_mode == CMDFILE )
					stat = dump_status( "", cmdfile, "close", stat );
				restore_scrn( orig );
				SWCCloseComm( pPort, 0 );

				return( user_termination() /* will be FAIL */ );
			}  /* end if */

		/* TRANSMIT commands to modem/RECEIVE responses from modem...        */
		com_status = communicate( cc, cmds2modem, cur_cmd, response );

		keep_going 		= NO;
		do_the_rest 	= NO;
		return_status	= OK;
		switch( com_status )
		{
			case END_OK:
				break;
			case FAIL:
				return_status  = FAIL;
				break;
			case RESTART:
				keep_going = YES;

				/* reset the command count indices...                          */
				cmdcnt   = 0;
				cmdindex = 0;

				/* reset the status file handler (close, then reopen)...       */
				if( cc.dumpstat && cmd_mode == CMDFILE )
				{
					stat = dump_status( "", cmdfile, "close", stat );
					stat = dump_status( "", cmdfile, "config", stat );
				}

				/* Clear the Command Window...                                 */
				if( cc.showcmds )
				{
					window( cmdwin -> lft - cmdwin -> ofs, cmdwin -> top,
									cmdwin -> ryt + cmdwin -> ofs, cmdwin -> bot );
					clrscr();
				}
				/* set window coordinates to CCS if appropriate...             */
				if( cc.show_stat_of_cmds )
					window( ccs -> lft - ccs -> ofs, ccs -> top,
												ccs -> ryt + ccs -> ofs, ccs -> bot );
				break;
			default:    /* everything is OK...                                */
				do_the_rest    = YES;
				keep_going 		= YES;
				break;
		}  /* end switch */

		if( do_the_rest )
		{
			/* turn interrupts off while accessing screen, etc...             */
			disable();

			/* check for equality of command sent/response received...        */
			com_verify = verify_cmd_rsp( cur_cmd, response );
			if( com_verify == FAIL )
				keep_going = NO;

			/* dump status information to ".sta" file or put current response
				on the end of the the total responses string...						*/
			if( cc.dumpstat && cmd_mode == CMDFILE )
				stat = dump_status( response, cmdfile, "write", stat );
			else if( cmd_mode == ARGUMENT )
				strcat( all_responses, response );

			/* tell the user how many commands are complete...                */
			if( cc.show_stat_of_cmds && keep_going )
			{
				++cmdcnt;
				/* the window will always be set to the CCS window coordinates */
				gotoxy( 1 + ccs -> ofs, 1 );
				cprintf( "%2.2d", cmdcnt );
			}  /* end if */

			/* turn interrupts back on...                                     */
			enable();
		}  /* end if */
	}  /* end while */
/*********************** END OF MAIN COMMUNICATIONS LOOP *******************/

	/* close the status file...                                             */
	if( cc.dumpstat && cmd_mode == CMDFILE )
		stat = dump_status( "", cmdfile, "close", stat );

	/* Remove windows from screen IN THE REVERSE ORDER THEY WERE CREATED... */

	/* remove the Command Complete Status window...                         */
	if( cc.show_stat_of_cmds && cmd_mode == CMDFILE )
	{
		ccs = popwin( ccs, OFF );
		free( ccs );
	}
	/* remove the command window...                                         */
	if( cc.showcmds )
	{
		cmdwin = popwin( cmdwin, OFF );
		free( cmdwin );
	}
	/* remove the test type window...                                       */
	if( cmd_mode == CMDFILE )
	{
		ttyp = popwin( ttyp, OFF );
		free( ttyp );
	}
	/* remove the initial Com Open message...                               */
	if( cc.verbose )
	{
		onlyn = popwin( onlyn, OFF );
		free( onlyn );
	}
	/* set the text window back to the original window coordinates...       */
	restore_scrn( orig );

	/* free the memory allocated for the remote command buffer...				*/
	free( cmds2modem );

	/* close the communications port...                                     */
	SWCCloseComm( pPort, 0 );

	return( return_status );
}  /* end main */

/***************************************************************************
Function:      communicate
Purpose:       to send commands to the modem and receive responses.
Arguments:     comm config structure, commands to send, current command,
					modem response.
Returns:       1 if ok; 0 if anything failed.
****************************************************************************/
int communicate( struct comm_config cc, char *cmds2modem, char cur_cmd[],
																				char response[] )
{
	ULONG	chars_TX;
	int  	keep_communicating, status, first_char, i, length, c;
	char	cmd[MAXSTR];

	/* send commands TO modem/receive responses FROM modem...               */
	keep_communicating = YES;
	while( keep_communicating )
	{
		/* get the current command from the string of all commands...			*/
		status = cmdstr2cmd( cmd, cmds2modem );
		if( status == END_OK )
			return( status );

		/* copy the current command into a string used for error check...    */
		strcpy( cur_cmd, cmd );

		/* check the first character of the current command for particular
			cases; react accordingly...													*/
		first_char = cmd[0];
		switch( first_char )
		{
			case REMARK:		/* go to the next command								*/
				break;
			case PAUSE:			/* pause the communications							*/
				if( pause_comm( cmd ) == END_TEST )
					return( END_OK );
				break;
			case BREAK_TEST:	/* break the communications and end (debugging)	*/
				return( break_test_notify() );
			case START_CMD:	/* TRANSMIT the command to the modem				*/
				if( cc.showcmds )
					put_cmd_onscr( cmd );

				/* string to modem...                                          */
				SWCControlRTS( pPort, ON );
				chars_TX = SWCTransmitString( pPort, (unsigned char *)cmd );
				if( chars_TX != (int)strlen( cmd ) )
				{
					/* define a window for this message...								*/
					window( 5, 5, 60, 12 );
					clrscr();
					gotoxy( 2, 2 );
					cprintf( "Error in SWCTransmitString:  didn't send the" );
					cprintf( " correct number of characters." );
					gotoxy( 2, 4 );
					cprintf( "Press any key to end..." );
					getch();

					return( FAIL );
				}  /* end if */

				/* RECEIVE the response from the modem...                      */
				status = SWCReceiveString( pPort,	/* the opened comm port		*/
													response,/* modem's response			*/
												RXBUFSIZE,  /* buffer size in bytes    */
												TERMINATOR, /* terminating character   */
												TIMEOUT,    /* seconds to timeout      */
												SWFALSE,    /* don't abort w/o carrier */
												SWFALSE,    /* don't echo char when RX */
												0,          /* Not used when echo on   */
												ALTXKEY );  /* abort key               */
				switch( status )
				{
					case SWCTERMINATORREACHED:		/* this is what you want		*/
						break;
					case SWCMAXLENGTHREACHED:
						/* define a window for this message...							*/
						window( 5, 5, 60, 15 );
						clrscr();
						gotoxy( 2, 2 );
						cprintf( "The modem's response exceeded the length of" );
						cprintf( " the RX buffer." );
						gotoxy( 2, 3 );
						cprintf( " The System Programmer must increase it." );
						gotoxy( 2, 5 );
						cprintf( "Press any key to end..." );
						getch();

						return( FAIL );
               case SWCTIMEDOUT:			/* VERY IMPORTANT!!!!					*/
						strcpy( response, ">1/NULL\r\n" );
						return( timeout_error() );
					case SWCNOCARRIER:		/* will never happen						*/
						printf( "\n\n There is no carrier\n\n." );
						printf( "\n\n Press any key to end..." );
						getch();
						return( FAIL );
					case -485:  /* termination with ALT-X (doesn't work right)	*/
						/* read the terminating character from the modem...      */
						c = SWCReceiveCharacter( pPort );
						while( c != SWCQUEUEISEMPTY )
							c = SWCReceiveCharacter( pPort );
						return( END_OK );
					default:
						break;
				}  /* end switch */

				/* replace all CR with CRLF and the TERMINATOR with a CRLF...  */
				length = (int)strlen( response );
				for( i = 0; i < length; ++i )
					if( response[i] == '\r' || response[i] == TERMINATOR )
						response[i] = '\n';
				if( cc.showcmds )
					put_cmd_onscr( response );

				return( OK );
			default:    /* FIRST LEVEL of error checking (SYNTAX)...          */
				return( command_syntax_error( cmd ) );
		}  /* end switch */
	}  /* end while */

	return( OK );
}  /* end communicate */

/***************************************************************************
Function:      pause_comm
Purpose:       to pause the test, display a message to the user, and either
					restart or end.
Arguments:     the remote command (with pause character as first char.)
Returns:       1 or 0
***************************************************************************/
int pause_comm( char *cmd )
{
	int      valid, i, status, allow_esc, offset;
	char     *first   = "Test Paused...\r\n\r\n ";
	char     *last    = "\r\n Press [C] to Continue.";
	POPWIN   *pause;
	struct   text_info orig;

	/* get the original window configuration...                             */
	gettextinfo( &orig );

	/* eliminate the "!" from the pause message...                          */
	offset = 1;
	allow_esc = NO;
	if( cmd[1] == PAUSE )  /* a "!!" was encountered                        */
	{
		allow_esc = YES;
		offset = 2;
	}

	/* this code turns "!text" into "text" and "!!text" into "text"...		*/
	for( i = 0; i < (int)strlen( cmd ) - offset; ++i )
		cmd[i] = cmd[i + offset];
	cmd[i] = '\0';

	/* alert the user...                                                    */
	if( beep_enable )
		printf( "\a" );

	/* create the message...                                                */
	if( (pause = PWALLOC) == NULL )
	{
		/* define a window for this message...											*/
		window( 5, 5, 60, 10 );
		clrscr();
		gotoxy( 2, 2 );
		cprintf( "There is Not enough memory for a PAUSE message" );
		gotoxy( 2, 4 );
		cprintf( "Press any key to end..." );
		getch();

		return( FAIL );
	}  /* end if */

	strcpy( pause -> mes, first );
	strcat( pause -> mes, cmd );
	strcat( pause -> mes, last );
	if( (int)strlen( cmd ) >= (int)strlen( last ) )
		strcpy( pause -> cfgstr, cmd );
	else
		strcpy( pause -> cfgstr, last );
	pause -> lft      = 5;
	pause -> top      = 18;
	pause -> numrows  = 5;
	pause = popwin( pause, ON );

	/* get the user's response and act on it...                             */
	valid = NO;
	while( !valid )
	{
		valid = YES;
		switch( getch() )
		{
			case 'c':case 'C':
				status = OK;
				break;
			case ESC:
				if( allow_esc )
					status = END_TEST;
				else
					valid = NO;
				break;
			default:
				valid = NO;
				break;
		}  /* end switch */
	}  /* end while */
	pause = popwin( pause, OFF );
	free( pause );

	restore_scrn( orig );

	return( status );
}  /* end pause_comm */

/***************************************************************************
Function:      break_test_notify
Purpose:       to notify the user that a break character was found.
Arguments:     none
Returns:       1 if ok; 0 if anything failed
****************************************************************************/
int   break_test_notify( void )
{
	POPWIN   *brk;

	if( (brk = PWALLOC) == NULL )
	{
		/* define a window for this message...											*/
		window( 5, 5, 60, 10 );
		clrscr();
		gotoxy( 2, 2 );
		cprintf( "Can't alloc. memory to tell you that a BREAK was found." );
		gotoxy( 2, 4 );
		cprintf( "Press any key to end..." );
		getch();

		return( FAIL );
	}  /* end if */

	sprintf( brk -> mes, "TEST BREAK..." );
	strcat( brk -> mes, "\n\r Terminating program in 3 seconds." );
	strcpy( brk -> cfgstr, "\n\r Terminating program in 3 seconds." );
	brk -> lft     = 10;
	brk -> top     = 20;
	brk -> numrows = 2;
	brk = popwin( brk, ON );
	sleep( 3 );
	brk = popwin( brk, OFF );
	free( brk );

	return( END_OK );
}  /* end break_test_notify */

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

/***************************************************************************
Function:      dump_status
Purpose:       to update the status screen with current modem configuration
Arguments:     current modem response, cmd file name, what to do, file ptr
Returns:       FILE pointer to open file; NULL if file could no be opened.
****************************************************************************/
FILE *dump_status( char response[], char cmd_file[], char which[], FILE *stat )
{
	char  extension[4], sta_file[MAXSTR];

	if( !strcmp( which, "config" ) )
	{
		/* change the extension on the command file name from .cmd to .sta...*/
		strcpy( extension, "sta" );
		strcpy( sta_file, cmd_file );
		change_ext( sta_file, extension );

		/* open the status file and write the modem status to it...          */
		stat = getfp( sta_file, "w" );
	}  /* end if */
	else if( !strcmp( which, "write" ) )
		fputs( response, stat );
	else if( !strcmp( which, "close" ) )
	{
		if( fclose( stat ) == EOF )
		{
			/* define a window for this message...										*/
			window( 5, 5, 60, 10 );
			clrscr();
			gotoxy( 2, 2 );
			cprintf( "fclose returned EOF in dump_status" );
			gotoxy( 2, 4 );
			cprintf( "Press any key to end..." );
			getch();
		}
	}  /* end else-if */
	return( stat );
}  /* end dump_status */

/***************************************************************************
Function:      get_comm_config
Purpose:       to read the pertinent communications config. from file
Arguments:     comcfg structure, command mode, communications mode
Returns:       1 if ok, 0 if fail
****************************************************************************/
int get_comm_config( struct comm_config *cc, int cmd_mode, int comm_mode )
{
	int	mode_not_found, i, j, file_end, override;
	char  cfgfile[2 * MAXSTR], curdir[MAXSTR], txt[MAXSTR], mode[10];
	char	act_inact[MAXSTR];
	FILE  *commcfg;

	switch( cmd_mode )
	{
		case CMDFILE:
			/* open the communications configuration file (must be in current
				working directory)...                    								*/
			strcpy( cfgfile, getcwd( curdir, MAXPATH ) );
			strip_bkslsh( cfgfile );
			strcat( cfgfile, "\\pc2modem.txt" );
			if( (commcfg = getfp( cfgfile, "r" )) == NULL )
				return( FAIL );

			/* get the override setting.  If override is set to "yes" then the
				active comm mode set in the file will be used regardless of the
				mode passed to pc2modem.  If overrride is set to "no" then the
				desired mode will be found in the file and those settings will
				be used...																		*/
			override = str2int( fgets( txt, MAXSTR, commcfg ) );

			/* get the communications mode from the file...							*/
			mode_not_found = TRUE;
			file_end			= NO;
			while( mode_not_found && !feof( commcfg ) )
			{
				/* look for the mode in curly braces...								*/
				while( *(fgets( txt, MAXSTR, commcfg ) + 0) != '{' &&
																			!feof( commcfg ) );
				if( feof( commcfg ) )
				{
					file_end = YES;
					break;	/* from while													*/
				}

				i = 1;
				j = 0;
				while( txt[i] != '}' )
					mode[j++] = txt[i++];
				mode[j] = '\0';

				switch( override )
				{
					case YES:
                  /* ...once found, read whether the mode is active or inactive.
							An active mode has a "yes" after the '=' and an inactive
							mode has a "no" after the '='.  If two modes are set to
							active, the first mode found active is used.  If no modes
							are set to active, the default of RS-485 is used...			*/

						/* skip the '}=' after the mode number...						*/
						i += 2;

						/* get the active/inactive portion of the string...		*/
						j = 0;
						while( txt[i] != '\n' )
							act_inact[j++] = txt[i++];
						act_inact[j] = '\0';

						if( !strcmp( act_inact, "yes" ) ||
																!strcmp( act_inact, "YES" ) )
							mode_not_found = FALSE;  /* a double negative!!			*/
						break;
					case NO:
						if( atoi( mode ) == comm_mode )
							mode_not_found = FALSE;
						break;
				}  /* end switch */
			}  /* end while */

			if( file_end )		/* an EOF was reached									*/
			{
				window( 2, 2, 60, 12 );
				clrscr();
				gotoxy( 2, 2 );
				cprintf( "There are no active modes set in the config. file" );
				gotoxy( 2, 3 );
				cprintf( "Defaulting to RS-485, COM 3, IRQ 7" );
				gotoxy( 2, 5 );
				cprintf( "Press any key to continue..." );
				getch();
				clrscr();

            cc -> port      	= RS485_PORT_DEFAULT;
				cc -> baud      	= RS485_BAUD_DEFAULT;
				cc -> base_addr	= RS485_BASE_ADDR_DEFAULT;
				cc -> irq			= RS485_IRQ_DEFAULT;
            cc -> verbose           = NO;
				cc -> show_stat_of_cmds = NO;
				cc -> showcmds          = NO;
				cc -> dumpstat          = NO;
				beep_enable					= NO;
				break;  /* from switch */
			}  /* end if */

			/* get the program control and program display information...		*/
			beep_enable = str2int( fgets( txt, MAXSTR, commcfg ) );
			cc -> verbose = str2int( fgets( txt, MAXSTR, commcfg ) );
			cc -> show_stat_of_cmds = str2int( fgets( txt, MAXSTR, commcfg ) );
			cc -> showcmds = str2int( fgets( txt, MAXSTR, commcfg ) );
			cc -> dumpstat = str2int( fgets( txt, MAXSTR, commcfg ) );

			/* get the port, baud rate, base address, and IRQ...					*/
			cc -> port 			= str2int( fgets( txt, MAXSTR, commcfg ) );
			cc -> baud 			= str2int( fgets( txt, MAXSTR, commcfg ) );
			cc -> base_addr 	= str2int( fgets( txt, MAXSTR, commcfg ) );
			cc -> irq 			= str2int( fgets( txt, MAXSTR, commcfg ) );

			/* close the file before returning to calling function...			*/
			if( fclose( commcfg ) == EOF )
			{
				/* define a window for this message...									*/
				window( 5, 5, 60, 10 );
				clrscr();
				gotoxy( 2, 2 );
				cprintf( "fclose returned EOF in comm_config" );
				gotoxy( 2, 4 );
				cprintf( "Press any key to end..." );
				getch();

				return( FAIL );
			}  /* end if */
			commcfg = NULL;
			break;
		case ARGUMENT:
			switch( comm_mode )
			{
				case RS485:
					cc -> port      	= RS485_PORT_DEFAULT;
					cc -> baud      	= RS485_BAUD_DEFAULT;
					cc -> base_addr	= RS485_BASE_ADDR_DEFAULT;
					cc -> irq			= RS485_IRQ_DEFAULT;
					break;
				case RS232:
					cc -> port        = RS232_PORT_DEFAULT;
					cc -> baud        = RS232_BAUD_DEFAULT;
					cc -> base_addr	= RS232_BASE_ADDR_DEFAULT;
					cc -> irq			= RS232_IRQ_DEFAULT;
					break;
			}  /* end switch */
			cc -> verbose           = NO;
			cc -> show_stat_of_cmds = NO;
			cc -> showcmds          = NO;
			cc -> dumpstat          = NO;
			beep_enable					= NO;
			break;
	}  /* end switch */

	return( OK );
}  /* end get_comm_config */

/***************************************************************************
Function:      count_commands
Purpose:       to count the number of commands in modem command file
Arguments:     FILE pointer, command mode, commands, ptr to number of pauses,
					ptr to number of remarks
Returns:       total commands
****************************************************************************/
int count_commands( FILE *inz, int cmd_mode, char *commands, int *pauses,
																					int *remarks )
{
	int      numcmds, i, pause, remark;
	char     response[MAXSTR];
	fpos_t   curpos;

	switch( cmd_mode )
	{
		case CMDFILE:
			/* get the current file position and store it in "curpos"...		*/
			fgetpos( inz, &curpos );

			numcmds = 0;
			/* read commands until end-of-file...										*/
			while( strcmp( fgets( response, MAXSTR, inz ), NULL ) != 0 )
			{
				if( strcmp( response, "\x1A" ) != 0 && response[0] != PAUSE &&
																		response[0] != REMARK )
					++numcmds;
				if( response[0] == PAUSE )
					++(*pauses);
				else if( response[0] == REMARK )
					++(*remarks);
			}  /* end while */

			/* set the file pointer back to the start of the file...				*/
			fsetpos( inz, &curpos );
			break;
		case ARGUMENT:
			numcmds = 0;
			for( i = 0; i < (int)strlen( commands ); ++i )
			{
				if( commands[i] == PAUSE )
					pause = YES;
				else if( commands[i] == REMARK )
					remark = YES;
				else
				{
					pause = NO;
					remark = NO;
				}
				if( commands[i] == '\n' && !pause && !remark )
					++numcmds;
			}  /* end for */
			break;
	}  /* end switch */

	return( numcmds );
}  /* end count_commands */

/***************************************************************************
Function:      get_test_type
Purpose:       converts the complete command file path to just a file name
Arguments:     command file path, file name only, cmd_mode
Returns:       none
****************************************************************************/
void get_test_type( char *complete_name, char *fname, int cmd_mode )
{
	int   i, j, start_index, backslash_spot, name_length;

	if( cmd_mode == CMDFILE )
	{
		strcpy( fname, "" );

		start_index 	= 0;
		backslash_spot	= 0;
		name_length 	= (int)strlen( complete_name );

		for( i = 0; i < name_length; ++i )
			switch( complete_name[i] )
			{
				case ':':
					start_index = i + 1;
					break;
				case '\\':
					start_index = i + 1;
					backslash_spot = i;
					break;
				case '.':
					if( backslash_spot != 0 )
						start_index = i - (i - backslash_spot) + 1;
					break;
				default:
					break;
			}  /* end switch */

		i = start_index;
		j = 0;
		while( complete_name[i] != '.' )
			fname[j++] = complete_name[i++];
		fname[j] = '\0';
	}  /* end if */
}  /* end get_test_type */

/***************************************************************************
Function:      get_commands
Purpose:       gets the modem commands from a file and stores in an array
Arguments:     ptr to total commands, ptr to commands string, command mode,
					command file name
Returns:       ptr to allocated memory if necessary
****************************************************************************/
char *get_commands( int *numcmds, char *commands, int cmd_mode,
						  char *cmdname )
{
	int   i, pauses, remarks;
	long  totbytes, curpos;
	char  txt[MAXSTR], *cmds2modem;
	FILE  *inz;

	totbytes = 0;
	pauses   = 0;
	remarks	= 0;

	switch( cmd_mode )
	{
		case CMDFILE:
			/* open the file with the modem test command sequence...				*/
			inz = NULL;
			if( ( inz = getfp( cmdname, "rb" ) ) == NULL )
				return( NULL );

			/* get the total number of bytes in command file...					*/

			/* get the current file pointer position...								*/
			curpos = ftell( inz );

			/* position the file pointer to the end of the stream...				*/
			fseek( inz, 0L, SEEK_END );

			/* the total number of bytes will be the distance from the start
				of the file; get that distance here...									*/
			totbytes = ftell( inz );

			/* reset the file pointer to the start of the file...					*/
			fseek( inz, curpos, SEEK_SET );

			/* count the number of commands for the program status display...	*/
			*numcmds = count_commands( inz, cmd_mode, commands, &pauses,
												&remarks );

			/* allocate memory for the commands to send to modem...				*/
			cmds2modem = calloc( (size_t)totbytes, (size_t)sizeof( char ) );
			if( cmds2modem == NULL )
			{
				/* define a window for this message...									*/
				window( 5, 5, 60, 10 );
				clrscr();
				gotoxy( 2, 2 );
				cprintf( "Not enough memory to allocate for cmd storage 1" );
				gotoxy( 2, 4 );
				cprintf( "Press any key to end." );
				getch();

				return( NULL );
			}  /* end if */

			/* read in each of the commands and store in the array...			*/
			strcpy( cmds2modem, "" );
			for( i = 0; i < *numcmds + pauses + remarks && !feof( inz ); ++i )
			{
				fgets( txt, MAXSTR, inz );
				if( strcmp( txt, "\n" ) != 0 && strcmp( txt, "\r" ) != 0 )
					strcat( cmds2modem, txt );
			}

			/* close the command file...													*/
			if( fclose( inz ) == EOF )
			{
				/* define a window for this message...									*/
				window( 5, 5, 60, 10 );
				clrscr();
				gotoxy( 2, 2 );
				cprintf( "fclose returned EOF in get_commands" );
				gotoxy( 2, 4 );
				cprintf( "Press any key to end..." );
				getch();

				return( NULL );
			}  /* end if */
			break;
		case ARGUMENT:
			/* count the number of commands for the program status display...	*/
			*numcmds = count_commands( inz, cmd_mode, commands, &pauses,
												&remarks );

			/* allocate memory for the commands to send to modem...				*/
			totbytes    = (int)strlen( commands );
			cmds2modem  = calloc( (size_t)totbytes, (size_t)sizeof( char ) );

			if( cmds2modem == NULL )
			{
				/* define a window for this message...									*/
				window( 5, 5, 60, 10 );
				clrscr();
				gotoxy( 2, 2 );
				cprintf( "Not enough memory to allocate for cmd storage 2" );
				gotoxy( 2, 4 );
				cprintf( "Press any key to end." );
				getch();

				return( NULL );
			}  /* end if */
			strcpy( cmds2modem, commands );
			break;
	}  /* end switch */

	return( cmds2modem );
}  /* end get_commands */

/***************************************************************************
Function:      cmdstr2cmd
Purpose:       to get a single command from the command string
Arguments:     current command, command string with all commands
Returns:       END_OK if at end of command string; 1 if not
****************************************************************************/
int   cmdstr2cmd( char cmd[], char *cmdstr )
{
	int j, length;

	length = (int)strlen( cmdstr );
	if( cmdindex >= length - 1 )
		return( END_OK );
	else
	{
		j = 0;
		while( cmdstr[cmdindex] != '\n' )
			cmd[j++] = cmdstr[cmdindex++];
		cmd[j] = cmdstr[cmdindex++];
		cmd[++j] = '\0';

		return( OK );
	}
}  /* end cmdstr2cmd */

/***************************************************************************
Function:      verify_cmd_rsp
Purpose:       to verify that the command and response are the same
Arguments:     current remote command, current modem response
Returns:       1 if ok, 0 if not the same.
****************************************************************************/
int   verify_cmd_rsp( char *cur_cmd, char *rem_rsp )
{
	int      i, j;
	char     errmes[30], error[5], tmpmes[MAXSTR];
	POPWIN   *em;

	/* SECOND LEVEL of error checking:  look for a '?' in the response as an
		indication of an error...                                            */
	i = 0;
	while( rem_rsp[i++] != '/' );

	if( rem_rsp[i] == '?' )  /* then modem returned an error message        */
	{
		/* create a popup window for displaying the error message...         */
		if( (em = PWALLOC) == NULL )
		{
			/* define a window for this message...										*/
			window( 5, 5, 60, 10 );
			clrscr();
			gotoxy( 2, 2 );
			cprintf( "Not enough memory to make error display window 1." );
			gotoxy( 2, 4 );
			cprintf( "Press any key to end." );
			getch();

			return( FAIL );
		}  /* end if */

		/* get the error message and display it in the popup window...       */
		j = 0;
		while( rem_rsp[i] != '_' )
			error[j++] = rem_rsp[i++];
		error[j] = '\0';

		j = 0;
		++i;     /* skip the '_' in the string                               */
		while( rem_rsp[i] != '\n' && rem_rsp[i] != ']' && rem_rsp[i] != '\r' )
			errmes[j++] = rem_rsp[i++];
		errmes[j] = '\0';

		/* alert the user to the error...												*/
		if( beep_enable )
			printf( "\a" );

		em -> lft      = 10;
		em -> top      = 10;
		em -> numrows  = 7;
		sprintf( em -> mes, "ERROR:  Modem Returned %s Meaning...", error );
		strcpy( em -> cfgstr, em -> mes );
		sprintf( tmpmes, "\n\r\n\r %s", errmes );
		strcat( em -> mes, tmpmes );
		sprintf( tmpmes, "\n\r\n\r Bad Command:  %s", cur_cmd );
		strcat( em -> mes, tmpmes );
		strcat( em -> mes, "\n\r Hit any key to end." );

		em = popwin( em, ON );
		getch();
		em = popwin( em, OFF );
		free( em );

		return( FAIL );
	}  /* end if */

	return( OK );
}  /* end verify_cmd_rsp */

/***************************************************************************
Function:      put_cmd_onscr
Purpose:       to put the command on the screen
Arguments:     the command string
Returns:       1 if ok; 0 if anything failed
****************************************************************************/
int   put_cmd_onscr( char *cmd )
{
	struct   text_info orig;

	/* get the original text window configuration...								*/
	gettextinfo( &orig );

	/* set the window to the command display window coordinates...				*/
	window( cmdwin -> lft - cmdwin -> ofs, cmdwin -> top,
										cmdwin -> ryt + cmdwin -> ofs, cmdwin -> bot );

	/* move to the current command row and put the command on the screen...	*/
	gotoxy( 1, cmdrow );
	cprintf( cmd );

	/* get the current row for the next command to put on the screen...		*/
	cmdrow = wherey();

	/* reset the window to the original window coordinates...					*/
	window( orig.winleft, orig.wintop, orig.winright, orig.winbottom );
	gotoxy( orig.curx, orig.cury );

	return( OK );
}  /* end put_cmd_onscr */

/***************************************************************************
Function:      verify_comm_params
Purpose:       to verify that the user entered proper arguments on the
					command line when running this program.
Arguments:     ptr to comm_config structure
Returns:       1 if OK, 0 if FAIL
****************************************************************************/
int verify_comm_params( struct comm_config *cc )
{
	int	i, std_base_addr[] = {0x3F8, 0x2F8, 0x3E8, 0x2E8}, status;
	int	valid_irq[] = {3, 4, 5, 10, 11, 12};

	if( cc -> port < LOWEST_PORT && cc -> port > HIGHEST_PORT )
	{
		/* define a window for this message...											*/
		window( 5, 5, 60, 10 );
		clrscr();
		gotoxy( 2, 2 );
		cprintf( "Port #%d is invalid.  You must put a valid port in the",
																					cc -> port );
		gotoxy( 2, 3 );
		cprintf( "Communications Configuration File." );
		gotoxy( 2, 5 );
		cprintf( "Press any key to end..." );
		getch();
		return( FAIL );
	}  /* end if */

	if( (cc -> baud > BAUD_MAX || cc -> baud < BAUD_MIN) ||
													(check_baud( cc -> baud ) == FAIL) )
	{
		/* define a window for this message...											*/
		window( 5, 5, 60, 12 );
		clrscr();
		gotoxy( 2, 2 );
		cprintf( "A Baud rate of %d bps is invalid.", cc -> baud );
		gotoxy( 2, 3 );
		cprintf( "Valid baud rates are 110, 150, 300, 600, 1200, 2400," );
		gotoxy( 2, 4 );
		cprintf( " 4800, 9600, or 19200 bps" );
		gotoxy( 2, 6 );
		cprintf( "Press any key to end..." );
		getch();

		return( FAIL );
	}  /* end if */

	/* check the base addresses against the defaults for COM 1 to COM 4...	*/
	for( i = 0; i < HIGHEST_PORT; ++i )
	{
		if( cc -> port == i + 1 )
			if( cc -> base_addr != std_base_addr[i] )
			{
				window( 5, 5, 60, 12 );
				clrscr();
				gotoxy( 2, 2 );
				cprintf( "COM %d must have a base address of %x.",
																			cc -> port,
																			std_base_addr[i] );
				gotoxy( 2, 4 );
				cprintf( "Press any key to end..." );
				getch();

				return( FAIL );
			}  /* end if */
	}  /* end for */

	status = 1;
	for( i = 0; i < NUM_VALID_IRQ; ++i )
	{
		if( cc -> irq == valid_irq[i] )
		{
			status = 1;
			break;
		}
		else
			status = 0;
	}  /* end for */

	if( status == 0 )
	{
		window( 5, 5 ,60, 10 );
		clrscr();
		gotoxy( 2, 2 );
		cprintf( "IRQ %d is invalid.  Valid IRQ's are 3, 4, 5, 10, 11, 12." );
		gotoxy( 2, 4 );
		cprintf( "Press any key to end..." );
		getch();

		return( FAIL );
	}  /* end if */

	return( OK );
}  /* end verify_comm_params */

/***************************************************************************
Function:      check_baud
Purpose:       to check the user entered baud rate for validity
Arguments:     the baud rate
Returns:       1 if valid, 0 if invalid
****************************************************************************/
int check_baud( int baud )
{
	static int valid_bauds[] = { 110, 150, 300, 600, 1200, 2400, 4800, 9600,
										  19200 };
	int i, status;

	for( i = 0; i < NUM_RATES; ++i )
	{
		if( baud == valid_bauds[i] )
		{
			status = OK;
			break;
		}
		else
			status = FAIL;
	}
	return( status );
}  /* end check_baud */

/***************************************************************************
Function:      user_termination
Purpose:       to notify the user that the test is being terminated when DELETE
					is pressed.
Arguments:     none
Returns:       FAIL
****************************************************************************/
int   user_termination( void )
{
	POPWIN   *endtst;

	disable();
	if( (endtst = PWALLOC) == NULL )
	{
		window( 5, 5, 60, 10 );
		clrscr();
		gotoxy( 2, 2 );
		cprintf( "Can't allocate memory to tell you that the test will end." );
		gotoxy( 2, 4 );
		cprintf( "Press any key to end..." );
		getch();

		return( FAIL );
	}  /* end if */

	sprintf( endtst -> mes,
								"DELETE key was pressed.  Test will End in 3 Sec." );
	endtst -> lft = 5;
	endtst -> top = 21;
	endtst = popwin( endtst, ON );
	sleep( 3 );
	endtst = popwin( endtst, OFF );
	free( endtst );
	enable();

	return( FAIL );
}  /* end user_termination */

/***************************************************************************
Function:      command_syntax_error
Purpose:       to notify the user when a command is invalid
Arguments:     command string
Returns:       FAIL always
****************************************************************************/
int   command_syntax_error( char *command )
{
	int      ch;
	char     tmpmes[MAXSTR];
	POPWIN   *cse;

	if( (cse = PWALLOC) == NULL )
	{
		window( 5, 5, 60, 10 );
		clrscr();
		gotoxy( 2, 2 );
		cprintf( "Can't allocate memory for Command Syntax Error window." );
		gotoxy( 2, 4 );
		cprintf( "Press any key to end..." );
		getch();

		return( FAIL );
	}  /* end if */

	cse -> lft     = 5;
	cse -> top     = 5;
	cse -> numrows = 5;
	strcpy( cse -> mes, "SYNTAX ERROR.  The following command is invalid..." );
	strcpy( cse -> cfgstr, cse -> mes );
	sprintf( tmpmes, "\n\r\n\r %s", command );
	strcat( cse -> mes, tmpmes );
	strcat( cse -> mes, "\r\n\r Press [Q] to end." );

	cse = popwin( cse, ON );
	while( (ch = getch()) != 'q' && ch != 'Q' );
	cse = popwin( cse, OFF );
	free( cse );

	return( FAIL );
}  /* end command_syntax_error */