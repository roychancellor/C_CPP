/***************************************************************************
*  Module:        FINEPOWR.C                                               *
*                                                                          *
*  Created By:    Roy Chancellor                                           *
*                                                                          *
*  Date Created:  02-16-1994                                               *
*                                                                          *
*  Last Modified: 05-10-1994                                               *
*                                                                          *
*  Version:       1.00                                                     *
*                                                                          *
*  Date Released: 05-10-1994                                               *
*                                                                          *
*  Description:   allows the user to change the DAC values in the modem    *
*                                                                          *
*  Inputs:        modem status file name                                   *
*                                                                          *
*  Returns:       none                                                     *
*                                                                          *
*  Date           Modification                                    Initials *
* ----------   -------------------------------------------------  ---------*
*  03-08-1994  Released program for use                              RSC   *
*  03-11-1994  added RF_ON command to string; ver to 0.01            RSC   *
*  03-16-1994  change tol. on power to ñ0.1 dBm; ver to 0.02         RSC   *
*  03-28-1994  added goto function.  Changed 'U' to PGUP.  Changed   RSC   *
*              'Y' to PGUP.  Starts at -30 dBm; goes to -5 dBm.            *
*              Changed 'N' to Insert key, 'ESC' in change_DAC to End key.  *
*              Ver to 0.03.                                                *
*  03-29-1994  Made old keys functional again to preserve compat-    RSC   *
*              ibility.  Version to 0.04.                                  *
*  03-31-1994  put TST6 in place of TST5 when modifying the DAC      RSC   *
*              values; use TST5 when End is pressed; ver to 0.05           *
*  04-14-1994  Changed operation so the user only has to press the   RSC   *
*              up/dn arrows to change the DAC values; ver to 0.06          *
*	05-10-1994	RELEASED FOR USE ON A CAC AS VERSION 1.00					RSC	*
****************************************************************************/
static char *version = "(Version 1.00)";

/* Include files that came with Turbo C...                                 */
#include <stdio.h>
#include <stdlib.h>
#include <conio.h>         /* for all screen calls                         */
#include <process.h>       /* for calls to exit                            */
#include <string.h>        /* for strcmp, strcpy, strcat                   */
#include <alloc.h>         /* for calls to calloc                          */
#include <math.h>          /* for atof                                     */
#include <dos.h>           /* for sleep                                    */

/* Include files custom written for test project...                        */
#include <boolean.h>       /* has values that take on 1 or 0               */
#include <ascii.h>         /* has key press values                         */
#include <utility.h>       /* for calls to getfp, hex2dec_int              */
#include <popwin.h>        /* for calls to popwin                          */
#include <hex2deci.h>      /* for calls to hex2str, etc...                 */
#include <popphelp.h>      /* for calls to popup_help                      */
#include <pc2modem.h>      /* for calls to pc2modem                        */

/* definitions...                                                          */
#define NUMDAC    52
#define FINETOL   0.1
#define PREVIOUS  3
#define STATUS    4
#define SYSFAIL   5
#define DOS       6
#define GOTO      7
#define NO_UP     8
#define NO_DOWN   9

/* change the stack size from default of 4K to...                          */
unsigned _stklen = 18000;

/* function prototypes...                                                  */
int      modem_status( char [][3], char [], char [], char [][3] );
int      ask_DAC_correct( char *, char *, int );
int      change_DAC( char *, char *, int );
float    elem2dBm( char * );
int      goto_special_power( void );
int      dBm2elem( float, int );
int      verify_power( float );

/* GLOBAL variables...                                                     */
FILE  *finehelp;

/***************************************************************************
Function:      main
Purpose:       starts the program
Arguments:     number of command line arguments, command line arguments
Returns:       none
****************************************************************************/
void main( int argc, char *argv[] )
{
	int      i;
	char     DAC_values[NUMDAC][3], elements[NUMDAC][3], cmd[200], sta[200];
	char     helpname[100], extension[4];
	struct   text_info orig;

	if( argc < 2 )
	{
		printf( "  \n  \n    SYNTAX:  finepowr filename.xxx." );
		printf( "\n          where:  filename.xxx is the .CMD/.STA file name" );
		printf( "     \n     \n" );
		exit( 0 );
	}

	/* get the original screen configuration...                             */
	gettextinfo( &orig );

	/* set up the cmd and sta file names...                                 */
	strcpy( cmd, argv[1] );
	strcpy( sta, argv[1] );
	strcpy( extension, "cmd" );
	change_ext( cmd, extension );
	strcpy( extension, "sta" );
	change_ext( sta, extension );

	/* open the help file...                                                */
	strcpy( helpname, argv[1] );
	strcpy( extension, "hlp" );
	change_ext( helpname, extension );
	if( (finehelp = getfp( helpname, "r" )) == NULL )
		exit( FAIL );

	/* get the power table status from a file...                            */
	switch( modem_status( DAC_values, cmd, sta, elements ) )
	{
		case FAIL:           /* couldn't open status file                    */
			fclose( finehelp );
			exit( FAIL );
		case SYSFAIL:
			fclose( finehelp );
			exit( SYSFAIL );  /* system command failed; can't find out why    */
		default:
			break;
	}  /* end switch */

	/* set the colors for this program...                                   */
	textcolor( WHITE );
	textbackground( LIGHTBLUE );

	/* for each element, decide if the DAC value must be modified.  If it
		does, then modify it in the modem...                                 */
	for( i = NUMDAC - 1; i >= 0; --i )
		switch( ask_DAC_correct( elements[i], DAC_values[i], i ) )
		{
			case YES:
				break;
			case -1:
				printf( "\n\n Memory allocation for popwin failed.\n\n\n" );
				i = -1;
				break;
			case PREVIOUS:
				if( i < NUMDAC - 1 )
					i += 2;
				else
					i += 1;
				break;
			case STATUS:
				switch( modem_status( DAC_values, cmd, sta, elements ) )
				{
					case FAIL:           /* couldn't open status file           */
						fclose( finehelp );
						exit( FAIL );
					case SYSFAIL:
						fclose( finehelp );
						exit( SYSFAIL );  /* system command failed;
													can't find out why                  */
					default:
						break;
				}  /* end switch */
				i += 1;  /* keep the same element number.                      */
				break;
			case GOTO:
				if( (i = goto_special_power()) == FAIL )
					i = -1;
				i += 1;
				break;
			case DOS:
				if( system( "c:\modems\password.exe" ) == -1 )
					printf( "\n\n System FAILED" );
				if( system( "command" ) == -1 )
					printf( "\n\n System FAILED" );
				i += 1;
				break;
			case ESC:case FAIL:
				i = -1;
				break;
		}  /* end switch */

	/* return the screen to its original configuration...                   */
	restore_scrn( orig );
	fclose( finehelp );
}  /* end main */

/***************************************************************************
Function:      modem_status
Purpose:       to put the modem status on the screen
Arguments:     option, DAC_values array, command file name, status file name,
					elements.
Returns:       1 if successful, 0 if fail
****************************************************************************/
int modem_status( char DAC_values[][3], char cmd_path[], char sta_path[],
						char elements[][3] )
{
	int      i, j, k, start_index, check, lyne;
	char     txt[MAXSTR];
	FILE     *sf;

	/* update the power table status...                                     */
	if( pc2modem( cmd_path, "", "", CMDFILE, RS485 ) == FAIL )
		return( FAIL );

	/* open the status file for read...                                     */
	if( (sf = getfp( sta_path, "r" )) == NULL )
		return( FAIL );

	/* go through the file line-by-line and get the element numbers and
		DAC values from each status response line...                         */
	lyne = 0;
	while( !feof( sf ) )
	{
		fgets( txt, MAXSTR, sf );
		if( !strcmp( txt, ">1/REM_\n" ) || !strcmp( txt, "\n" ) )
			check = NO;
		else
			check = YES;
		if( check )
		{
			switch( txt[0] )
			{
				case '>':
					start_index = 3;
					break;
				default:
					start_index = 0;
					break;
			}  /* end switch */

			/* skip the command portion of the status response...             */
			i = start_index;
			while( txt[i++] != '_' );

			/* get the element portion of the response...                     */
			j = 0;
			while( txt[i] != '_' )
				elements[lyne][j++] = txt[i++];
			elements[lyne][j] = '\0';

			/* skip the '_' after the element...                              */
			++i;

			/* get the DAC value from the response string...                  */
			k = 0;
			while( txt[i] != '\n' && txt[i] != '\r' )
				DAC_values[lyne][k++] = txt[i++];
			DAC_values[lyne][k] = '\0';
			++lyne;
		}  /* end if */
	}  /* end while */

	fclose( sf );
	return( OK );
}  /* end modem_status */

/***************************************************************************
Function:      ask_DAC_correct
Purpose:       to ask the user if the current DAC values gives the correct
					power setting
Arguments:     element string, element index
Returns:       YES if it power is in spec, NO if it is not; -1 if calloc fails;
					ESC to end the program.
****************************************************************************/
int ask_DAC_correct( char *element, char *DAC_value, int elem_index )
{
	int      valid, elem_int, user_resp;
	float    felem;
	char     highlow[7], *cstr = "<1/REM_\r\n<1/MOP_%4.1f\r\n<1/RF_ON\r\n";
	char     cmdstr[50], resp[30], tmpstr[MAXSTR];
	POPWIN   *adc;
	HELPINFO help;

	/* change the Modulator output power in the modem...                    */
	felem = elem2dBm( element );
	strcpy( cmdstr, "" );
	strcpy( resp, "" );
	sprintf( cmdstr, cstr, felem );
	if( pc2modem( "", cmdstr, resp, ARGUMENT, RS485 ) == FAIL )
		return( ESC );

	/* decide if the range is high or low...                                */
	if( (elem_int = hex2dec_int( element, "hex" )) >= 0 && elem_int <= 25 )
		strcpy( highlow, "(HIGH)" );
	else if( elem_int >= 26 && elem_int <= 51 )
		strcpy( highlow, "(LOW)" );

	/* create a popup window for asking user if correct/not correct...      */
	if( (adc = PWALLOC) == NULL )
		return( -1 );
	adc -> lft = 5;
	adc -> top = 10;
	adc -> numrows = 7;
	sprintf( adc -> mes, "Fine Power Table Adjust %s", version );
	sprintf( tmpstr, "\n\r\n\r Is the power meter at %5.2f ñ %4.2f dBm %s?",
													elem2dBm( element ), FINETOL, highlow );
	strcat( adc -> mes, tmpstr );
	if( elem_index < NUMDAC - 1 )
	{
		strcat( adc -> mes, "\n\r\n\r " );
		strcpy( adc -> cfgstr, "PgDn-Yes  PgUp-Previous  ESC-End" );
		strcat( adc -> cfgstr, "  G-Goto  F1-Help  S-Status" );
		strcat( adc -> mes, adc -> cfgstr );
	}
	else
	{
		strcat( adc -> mes, "\n\r\n\r " );
		strcpy( adc -> cfgstr, "PgDn-Yes  ESC-End" );
		strcat( adc -> cfgstr, "  G-Goto  F1-Help  S-Status     " );
		strcat( adc -> mes, adc -> cfgstr );
	}
	sprintf( tmpstr, "\n\r\n\r Change DAC Value:  %c/%c; Press PGDN When Set",
																							24, 25 );
	strcat( adc -> mes, tmpstr );

	adc = popwin( adc, ON );
	gotoxy( adc -> lft, adc -> top );

	/* get the user's response...                                           */
	valid = NO;
	while( !valid )
	{
		valid = YES;
		switch( getch() )
		{
			case 'g':case 'G':
				adc = popwin( adc, OFF );
				free( adc );
				return( GOTO );
			case 's':case 'S':
				adc = popwin( adc, OFF );
				free( adc );
				return( STATUS );
			case 'y':case 'Y':
				adc = popwin( adc, OFF );
				free( adc );
				return( YES );
			case 'u':case 'U':
				adc = popwin( adc, OFF );
				free( adc );
				return( PREVIOUS );
			case EXTENDED_KEY:
				valid = NO;
				switch( (user_resp = getch()) )
				{
					case F1:
						help.fp        = finehelp;
						help.field     = 1;
						help.numfields = 1;
						help.block     = 0;
						strcpy( help.opttxt, "Help For Fine Power Adjust" );
						popup_help( help );
						break;
					case PGDN:
						adc = popwin( adc, OFF );
						free( adc );
						return( YES );
					case PGUP:
						adc = popwin( adc, OFF );
						free( adc );
						return( PREVIOUS );
					case ALTD:
						adc = popwin( adc, OFF );
						free( adc );
						return( DOS );
					case UP_ARROW:case DOWN_ARROW:
						if( change_DAC( element, DAC_value, user_resp ) == FAIL )
						{
							adc = popwin( adc, OFF );
							free( adc );
							return( FAIL );
						}
						adc = popwin( adc, OFF );
						free( adc );
						return( YES );
					default:
						break;
				}  /* end switch */
				break;
			case ESC:
				adc = popwin( adc, OFF );
				free( adc );
				return( ESC );
			default:
				valid = NO;
				break;
		}  /* end switch */
	}  /* end while */
	return( OK );
}  /* end ask_DAC_correct */

/***************************************************************************
Function:      elem2dBm
Purpose:       converts a DAC value to its corresponding dBm power level
Arguments:     ptr to element number string
Returns:       floating point equivalent to DAC value
****************************************************************************/
float elem2dBm( char *element )
{
	int   element_int;
	float fval;
	char  temp[3];

	strcpy( temp, element );
	element_int = hex2dec_int( temp, "hex" );
	if( element_int >= 0 && element_int <= 25 )        /* LOW range         */
		fval = -5.0000 - 0.5000 * (float)element_int;
	else if( element_int > 25 && element_int <= 51 )   /* HIGH Range        */
		fval = -4.5000 - 0.5000 * (float)element_int;
	return( fval );
}  /* end elem2dBm */

/***************************************************************************
Function:      change_DAC
Purpose:       to change the DAC values remotely
Arguments:     current element, current DAC value string, direction (up/down)
Returns:       OK if it was changed; FAIL if it failed.
****************************************************************************/
int change_DAC( char *element, char *DAC_value, int direction )
{
	int      keep_going, valid, int_DAC_value, first_time;
	char     cmd[MAXSTR], resp[MAXSTR], tempcmd[35];
	float    felem;

	first_time = YES;

	/* Continue accepting up & down arrows to change the DAC value until the
		Page Down key is pressed; then return to the main loop...            */
	keep_going = YES;
	while( keep_going )
	{
		/* while the user does not enter a valid response, keep prompting... */
		if( !first_time )
		{
			valid = NO;
			while( !valid )
			{
				/* get the user's response...                                  */
				switch( getch() )
				{
					case EXTENDED_KEY:
						switch( (direction = getch()) )
						{
							case UP_ARROW:case DOWN_ARROW:case PGDN:
								valid = YES;
								break;
							default:
								break;
						}  /* end switch */
						break;
					default:
						valid = NO;
						break;
				}  /* end switch */
			}  /* end while */
		}  /* end if */

		first_time = NO;
		switch( direction )
		{
			case UP_ARROW:case NO_UP:     /* change the DAC value up by 1     */
				int_DAC_value = hex2dec_int( DAC_value, "hex" );
				if( int_DAC_value < 0xFF )
					add_to_str( DAC_value, +0x01, "hex" );
				break;
			case DOWN_ARROW:case NO_DOWN: /* change the DAC value down by 1   */
				int_DAC_value = hex2dec_int( DAC_value, "hex" );
				if( int_DAC_value > 0x00 )
					add_to_str( DAC_value, -0x01, "hex" );
				break;
			case PGDN:                    /* write the DAC value to EEPROM    */
				keep_going = NO;
				felem = elem2dBm( element );
				sprintf( cmd, "<1/REM_\r\n<1/MOP_%4.1f\r\n<1/TST5_%s_%s\r\n",
																	felem, element, DAC_value );
				sprintf( tempcmd, "<1/MOP_%4.1f\r\n<1/RF_ON\r\n", felem );
				strcat( cmd, tempcmd );
				strcpy( resp, "" );
				if( pc2modem( "", cmd, resp, ARGUMENT, RS485 ) == FAIL )
					return( FAIL );
				break;
		}  /* end switch */

		/* Change the DAC Value in the modem, but not in the EEPROM...       */
		if( keep_going )
		{
			felem = elem2dBm( element );
			sprintf( cmd, "<1/REM_\r\n<1/TST6_%s_%s\r\n", element, DAC_value );
			strcpy( resp, "" );
			if( pc2modem( "", cmd, resp, ARGUMENT, RS485 ) == FAIL )
				return( FAIL );
		}  /* end if */
	}  /* end while */

	return( OK );
}  /* end change_DAC */

/***************************************************************************
Function:      goto_special_power
Purpose:       to let the user go to a specified power level
Arguments:     none
Returns:       offset from current element index if ok; 0 if anything fails
****************************************************************************/
int   goto_special_power( void )
{
	int      new_elem, power_valid, range, range_valid, prompt_len;
	float    dBm;
	POPWIN   *gsv, *hilo;

	/* create popup window prompting user to enter power to go to...        */
	if( (gsv = PWALLOC) == NULL )
	{
		printf( "\n\n Could not allocate memory for goto power window." );
		sleep( 2 );
		return( FAIL );
	}
	gsv -> lft = 20;
	gsv -> top = 20;
	strcpy( gsv -> mes, "Enter Desired Power (dBm):" );
	prompt_len = (int)strlen( gsv -> mes );
	strcat( gsv -> mes, "         " );

	power_valid = NO;
	while( !power_valid )
	{
		gsv = popwin( gsv, ON );
		gotoxy( gsv -> lft + prompt_len + 2, gsv -> top );
		scanf( "%f", &dBm );
		power_valid = verify_power( dBm );

		if( dBm == -17.5 )
		{
			if( (hilo = PWALLOC) == NULL )
			{
				printf( "\n\n Can't allocate memory for hi lo window." );
				sleep( 2 );
				return( FAIL );
			}
			hilo -> lft = 20;
			hilo -> top = 15;
			strcpy( hilo -> mes, "[H]-High  [L]-Low" );
			hilo = popwin( hilo, ON );
			gotoxy( hilo -> lft + 1, hilo -> top );
			range_valid = NO;
			while( !range_valid )
				switch( (range = getch()) )
				{
					case 'h':case 'H':case 'l':case 'L':
						range_valid = YES;
						break;
					case EXTENDED_KEY:
						getch();
						break;
				}  /* end switch */
			hilo = popwin( hilo, OFF );
			free( hilo );
		}  /* end if */
		if( power_valid )
		{
			if( fabs( dBm ) >= 5.0 && fabs( dBm ) < 17.5 )
				range = 'h';
			else if( fabs( dBm ) > 17.5 && fabs( dBm ) <= 30.0 )
				range = 'l';
			new_elem = dBm2elem( dBm, range );
		}
	}  /* end while */

	/* turn off popup window...                                             */
	gsv = popwin( gsv, OFF );
	free( gsv );

	return( new_elem );
}  /* end goto_special_power */

/***************************************************************************
Function:      dBm2elem
Purpose:       converts a string which represents a dBm value to an element
					index.
Arguments:     dBm (float), range
Returns:       the element
****************************************************************************/
int   dBm2elem( float dBm, int range )
{
	int   element;

	switch( range )
	{
		case 'l':case 'L':
			element = (int)( (dBm + 4.500) / -0.5000 );
			break;
		case 'h':case 'H':
			element = (int)( (dBm + 5.000) / -0.5000 );
			break;
	}  /* end switch */
	return( element );
}  /* end dBm2elem */

/***************************************************************************
Function:      verify_power
Purpose:       to make sure the user entered a valid power
Arguments:     power (float)
Returns:       1 if valid; 0 if invalid
****************************************************************************/
int   verify_power( float dBm )
{
	float i;
	int   status;

	for( i = -5.00; i >= -30.0; i -= .500 )
	{
		if( dBm == i )
		{
			status = OK;
			i = -100;
		}
		else
			status = FAIL;
	}  /* end for */
	return( status );
}  /* end verify_power */