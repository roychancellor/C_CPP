/***************************************************************************
*  Module:        RXSIGNAL.C                                               *
*                                                                          *
*  Created By:    Roy Chancellor                                           *
*                                                                          *
*  Date Created:  02-22-1994                                               *
*                                                                          *
*  Last Modified: 05-10-1994                                               *
*                                                                          *
*  Version:       1.00                                                     *
*                                                                          *
*  Date Released: 05-10-1994                                               *
*                                                                          *
*  Description:   allows the user to change the I2C Signal values in the   *
*                 modem                                                    *
*                                                                          *
*  Inputs:        modem status file name                                   *
*                                                                          *
*  Returns:       none                                                     *
*                                                                          *
*  Date           Modification                                    Initials *
* ----------   -------------------------------------------------  ---------*
*  03-09-1994  Released as version 0.00                              RSC   *
*  03-11-1994  changed F2-RX... to F8-RX...; ver to 0.01             RSC   *
*  03-16-1994  added option to skip values; ver to 0.02              RSC   *
*  03-23-1994  recompiled with new ver of pc2modem; ver to 0.03      RSC   *
*	05-10-1994	RELEASED FOR USE ON A CAC AS VERSION 1.00					RSC	*
****************************************************************************/
char *version = "(Version 1.00)";

/* Include files that came with Turbo C...                                 */
#include <stdio.h>
#include <stdlib.h>
#include <conio.h>         /* for all screen calls                         */
#include <process.h>       /* for calls to exit                            */
#include <string.h>        /* for strcmp, strcpy, strcat                   */
#include <alloc.h>         /* for calls to calloc                          */

/* Include files written custom for test project...                        */
#include <boolean.h>       /* has values that take on 1 or 0               */
#include <ascii.h>         /* has key press values                         */
#include <utility.h>       /* for calls to getfp, hex2dec_int              */
#include <popwin.h>        /* for calls to popwin                          */
#include <hex2deci.h>      /* for calls to hex2dec, etc...                 */
#include <popphelp.h>      /* for calls to popup_help                      */
#include <pc2modem.h>      /* for calls to pc2modem                        */
#include <monvalue.h>      /* for calls to monitor_values                  */


/* set the stack size...                                                   */
unsigned _stklen = 8000;

/* definitions...                                                          */
#define NUMI2C    8
#define INCRDECR  3
#define CONTINUE  4
#define SKIP      5

/* function prototypes...                                                  */
int      set_attenuator( int );
int      modem_status( int, char [], char [] );
int      decide_I2C_correct( char *, char * );
float    breakpoint2dBm( int );
int      modify_I2C( int, char *, char * );

/* GLOBAL variables...                                                     */
FILE  *rxhelp;

/***************************************************************************
Function:      main
Purpose:       starts the program
Arguments:     number of command line arguments, command line arguments
Returns:       none
****************************************************************************/
void main( int argc, char *argv[] )
{
	int         breakpoint;
	static char I2C_value[3], ADC_value[3], helpname[100], extension[4];
	struct      text_info orig;

	/* open the help file for read...                                       */
	if( argc > 1 )
	{
		strcpy( helpname, argv[1] );
		strcpy( extension, "hlp" );
		change_ext( helpname, extension );
		if( (rxhelp = getfp( helpname, "r" )) == NULL )
			exit( FAIL );
	}  /* end if */

	/* get the original screen configuration...                             */
	gettextinfo( &orig );

	/* set the colors for this program...                                   */
	textcolor( WHITE );
	textbackground( LIGHTBLUE );

	/* for each breakpoint in the I2C signal level table, tell the user to
		adjust the attenuator to the correct value.  Then read the I2C value
		and ADC value from the modem and decide if the I2C value is within
		the proper tolerance of the ADC value.  If it is, then tell the user
		to go to the next attenuator setting.  If it is not, then modify the
		I2C value in the modem and go to the next attenuator setting.        */
	for( breakpoint = 0; breakpoint < NUMI2C; ++breakpoint )
	{
		switch( set_attenuator( breakpoint ) )
		{
			case CONTINUE:
				if( modem_status( breakpoint, I2C_value, ADC_value ) == FAIL )
				{
					fclose( rxhelp );
					exit( FAIL );
				}
				switch( decide_I2C_correct( I2C_value, ADC_value ) )
				{
					case CORRECT:
						break;
					case INCORRECT:
						modify_I2C( breakpoint, I2C_value, ADC_value );
						/* set the colors for this program...                    */
						textcolor( WHITE );
						textbackground( LIGHTBLUE );
						break;
				}  /* end switch */
				break;
			case -1:
				printf( "\n\n Memory allocation for popwin failed.\n\n\n" );
				breakpoint = NUMI2C;
				break;
			case INCRDECR:
				if( breakpoint > 0 )
					breakpoint -= 2;
				else
					breakpoint -= 1;
				break;
			case SKIP:
				break;
			case ESC:
				breakpoint = NUMI2C;
				break;
		}  /* end switch */
	}  /* end for */

	/* return the screen to its original configuration...                   */
	window( orig.winleft, orig.wintop, orig.winright, orig.winbottom );
	textattr( orig.attribute );
	gotoxy( orig.curx, orig.cury );
	fclose( rxhelp );
}  /* end main */

/***************************************************************************
Function:      modem_status
Purpose:       to put the modem status on the screen
Arguments:     breakpoint, I2C_value, ADC_value
Returns:       1 if successful, 0 if fail
****************************************************************************/
int modem_status( int breakpoint, char I2C_value[], char ADC_value[] )
{
	int         i, j, k, resplen;
	static char cmd[5], cmdstr[40], brkpt[3], resp[MAXSTR];

	/* get the current I2C and ADC status...                                */
	strcpy( resp, "" );
	sprintf( cmdstr, "<1/TST0_%2.2d\r\n<1/TST1_\r\n", breakpoint );
	if( pc2modem( "", cmdstr, resp, ARGUMENT, RS485 ) == FAIL )
		return( FAIL );

	/* go through the response character-by-character and get the breakpoint
		number, I2C value, and ADC value...                                  */
	i = 0;
	resplen = (int)strlen( resp ) - 1;
	while( i < resplen - 1 )
	{
		/* get the command portion of the response...                        */
		while( resp[i++] != '>' );
		while( resp[i++] != '/' );
		j = 0;
		while( resp[i] != '_' )
			cmd[j++] = resp[i++];
		cmd[j] = '\0';

		/* decide what command has been found and act accordingly...         */
		if( !strcmp( cmd, "TST0" ) )
		{
			/* get the breakpoint portion of the response...                  */
			++i;  /* skip the '_'.                                            */
			j = 0;
			while( resp[i] != '_' )
				brkpt[j++] = resp[i++];
			brkpt[j] = '\0';

			/* skip the '_' after the breakpoint...                           */
			++i;

			/* get the I2C value from the response string...                  */
			k = 0;
			while( resp[i] != '\n' )
				if( resp[i] != '\r' )
					I2C_value[k++] = resp[i++];
				else
					++i;
			I2C_value[k] = '\0';
		}  /* end if */
		else if( !strcmp( cmd, "TST1" ) )
		{
			/* get the ADC_value from the response...                         */
			++i;  /* skip the '_'.                                            */
			j = 0;
			while( resp[i] != '\n' )
				if( resp[i] != '\r' )
					ADC_value[j++] = resp[i++];
				else
					++i;
			ADC_value[j] = '\0';
		}  /* end else-if */
	}  /* end while */

	return( OK );
}  /* end modem_status */

/***************************************************************************
Function:      decide_I2C_correct
Purpose:       to decide if the I2C value matches the ADC value
Arguments:     I2C value string, ADC value string, index
Returns:       CORRECT if I2C Signal level is in spec, INCORRECT if it is not;
					-1 if calloc fails; ESC to end the program.
****************************************************************************/
int decide_I2C_correct( char *I2C_value, char *ADC_value )
{
	int   I2C, ADC;

	I2C = hex2dec_int( I2C_value, "hex" );
	ADC = hex2dec_int( ADC_value, "hex" );

	if( abs( I2C - ADC ) <= 1 )
		return( CORRECT );
	else
		return( INCORRECT );
}  /* end decide_I2C_correct */

/***************************************************************************
Function:      breakpoint2dBm
Purpose:       converts a I2C value to its corresponding dBm power level
Arguments:     breakpoint
Returns:       floating point equivalent to I2C value
****************************************************************************/
float breakpoint2dBm( int breakpoint )
{
	return( -25.0000 - 5.0000 * (float)breakpoint );
}  /* end breakpoint2dBm */

/***************************************************************************
Function:      modify_I2C
Purpose:       to change the I2C values remotely
Arguments:     breakpoint, I2C value, ADC value, .CMD/.STA file name
Returns:       OK if it was changed; FAIL if file could not be opened.
****************************************************************************/
int modify_I2C( int breakpoint, char *I2C_value, char *ADC_value )
{
	int         I2C, ADC;
	static char cmdstr[MAXSTR], resp[MAXSTR];

	I2C = hex2dec_int( I2C_value, "hex" );
	ADC = hex2dec_int( ADC_value, "hex" );
	add_to_str( I2C_value, ADC - I2C, "hex" );

	sprintf( cmdstr, "<1/REM_\r\n<1/TST0_%2.2d_%s\r\n<1/TST1_\r\n",
																		breakpoint, I2C_value );
	strcpy( resp, "" );
	if( pc2modem( "", cmdstr, resp, ARGUMENT, RS485 ) == FAIL )
		return( FAIL );
	return( OK );
}  /* end modify_I2C */

/***************************************************************************
Function:      set_attenuator
Purpose:       tells the user to set the attenuator to the proper value
Arguments:     breakpoint
Returns:       OK if popwin worked; FAIL if it didn't; program status
****************************************************************************/
int   set_attenuator( int breakpoint )
{
	int      valid;
	POPWIN   *sa;
	HELPINFO help;

	if( (sa = PWALLOC) == NULL )
	{
		clrscr();
		printf( "\n\n Can't allocate memory for POPWIN.\n" );
		return( -1 );
	}
	sa -> lft = 5;
	sa -> top = 11;
	sa -> numrows = 5;
	sprintf( sa -> mes, "Calibrate RX Power Table %s ", version );
	sprintf( sa -> cfgstr,
				"*** Set the attenuator so the input power is %5.3f dBm ***",
																breakpoint2dBm( breakpoint ) );
	strcat( sa -> cfgstr, "       " );
	strcat( sa -> mes, "\r\n\r\n " );
	strcat( sa -> mes, sa -> cfgstr );
	if( breakpoint == 0 )
	{
		strcat( sa -> mes, "\r\n\r\n S-Power Set  ESC-End  F1-Help" );
		strcat( sa -> mes, "  F8-RX Sig. Lev.  K-Skip" );
	}
	else
	{
		strcat( sa -> mes, "\r\n\r\n S-Power Set" );
		strcat( sa -> mes, "  P-Previous  ESC-End  F1-Help" );
		strcat( sa -> mes, "  F8-RX Sig. Lev.  K-Skip" );
	}
	sa = popwin( sa, ON );
	valid = NO;
	while( !valid )
	{
		valid = YES;
		switch( getch() )
		{
			case 's':case 'S':
				sa = popwin( sa, OFF );
				free( sa );
				return( CONTINUE );
			case 'p':case 'P':
				sa = popwin( sa, OFF );
				free( sa );
				return( INCRDECR );
			case 'k':case 'K':
				sa = popwin( sa, OFF );
				free( sa );
				return( SKIP );
			case ESC:
				sa = popwin( sa, OFF );
				free( sa );
				return( ESC );
			case EXTENDED_KEY:
				valid = NO;
				switch( getch() )
				{
					case F1:
						help.fp        = rxhelp;
						help.field     = 1;
						help.numfields = 1;
						help.block     = 0;
						strcpy( help.opttxt, "Help For RX Signal Level" );
						popup_help( help );
						break;
					case F8:
						if( monitor_values( "rxsignal" ) == FAIL )
							return( FAIL );
					default:
						break;
				}  /* end switch */
				break;
			default:
				valid = NO;
				break;
		}  /* end switch */
	}  /* end while */
	return( OK );
}  /* end set_attenuator */