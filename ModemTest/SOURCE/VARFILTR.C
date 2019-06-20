/***************************************************************************
*  Module:        VARFILTR.C                                               *
*                                                                          *
*  Created By:    Roy Chancellor                                           *
*                                                                          *
*  Date Created:  03-30-1994                                               *
*                                                                          *
*  Last Modified: 05-10-1994                                               *
*                                                                          *
*  Version:       1.00                                                     *
*                                                                          *
*  Date Released: 05-10-1994                                               *
*                                                                          *
*  Description:   allows the user to change the DAC values in the modem's  *
*                 variable alias filter table.                             *
*                                                                          *
*  Inputs:        modem status/help file names                             *
*                                                                          *
*  Returns:       none                                                     *
*                                                                          *
*  Date           Modification                                    Initials *
* ----------   -------------------------------------------------  ---------*
* 03-30-1994   Released for use as version 0.00                      RSC   *
* 03-31-1994   Added ability to run mod or demod; ver to 0.01        RSC   *
* 05-10-1994	RELEASED FOR USE ON A CAC AS VERSION 1.00					RSC	*
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

/* Include files custom written for test project...                        */
#include <boolean.h>       /* has values that take on 1 or 0               */
#include <ascii.h>         /* has key press values                         */
#include <utility.h>       /* for calls to getfp, hex2dec_int              */
#include <popwin.h>        /* for calls to popwin                          */
#include <hex2deci.h>      /* for calls to hex2str, etc...                 */
#include <popphelp.h>      /* for calls to popup_help                      */
#include <pc2modem.h>      /* for calls to pc2modem                        */

/* definitions...                                                          */
#define NUMDAC    11
#define FINETOL   0.1
#define PREVIOUS  3
#define STATUS    4
#define SYSFAIL   5
#define DOS       6
#define GOTO      7

/* change the stack size from default of 4K to...                          */
unsigned _stklen = 20000;

/* function prototypes...                                                  */
int      DAC_status( char [][3], char [][3], char [], char [], char [][3],
							char [][7], char [][2] );
int      ask_DAC_correct( char *, int, char *, char *, char * );
int      change_DAC( char *, char *, char *, char *, char * );
float    elem2BP( char * );
POPWIN   *create_menu( char * );

/* GLOBAL variables...                                                     */
FILE  *filterhelp;
char  board_type[2];

/***************************************************************************
Function:      main
Purpose:       starts the program
Arguments:     number of command line arguments, command line arguments
Returns:       none
****************************************************************************/
void main( int argc, char *argv[] )
{
	int      i;
	char     D0_values[NUMDAC][3], D1_values[NUMDAC][3], elements[NUMDAC][3];
	char     sym_rate[NUMDAC][7], range[NUMDAC][2], cmd[200], sta[200];
	char     helpname[100], extension[4];
	struct   text_info orig;

	if( argc < 2 )
	{
		printf( "  \n  \n    SYNTAX:  varfiltr filename.xxx." );
		printf( "\n          where:  filename.xxx is the .CMD/.STA file name" );
		printf( "     \n     \n" );
		exit( FAIL );
	}

	/* get the original screen configuration...                             */
	gettextinfo( &orig );

	/* set up the .cmd and .sta file names...											*/
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
	if( (filterhelp = getfp( helpname, "r" )) == NULL )
		exit( FAIL );

	/* get the power table status from a file...                            */
	switch( DAC_status( D0_values, D1_values, cmd, sta, elements, sym_rate,
							  range ) )
	{
		case FAIL:           /* couldn't open status file                    */
			fclose( filterhelp );
			exit( FAIL );
		case SYSFAIL:
			fclose( filterhelp );
			exit( SYSFAIL );  /* system command failed; can't find out why    */
		default:
			break;
	}  /* end switch */

	/* set the colors for this program...                                   */
	textcolor( WHITE );
	textbackground( LIGHTBLUE );

	/* for each element, decide if the DAC value must be modified.  If it
		needs modification, then modify it in the modem...                   */
	for( i = 0; i < NUMDAC; ++i )
		switch( ask_DAC_correct( elements[i], i, D0_values[i], D1_values[i],
										 range[i] ) )
		{
			case YES:
				break;
			case NO:
				if( change_DAC( elements[i], D0_values[i], D1_values[i],
									 range[i], sym_rate[i] ) == FAIL )
					i = NUMDAC;
				break;
			case -1:		/* memory allocation for popwin failed						*/
				i = NUMDAC;
				break;
			case PREVIOUS:
				if( i > 0 )
					i -= 2;
				else
					i -= 1;
				break;
			case STATUS:
				switch( DAC_status( D0_values, D1_values, cmd, sta, elements,
																			sym_rate, range ) )
				{
					case FAIL:           /* couldn't open status file           */
						fclose( filterhelp );
						exit( FAIL );
					case SYSFAIL:
						fclose( filterhelp );
						exit( SYSFAIL );  /* system command failed;
													can't find out why                  */
					default:
						break;
				}  /* end switch */
				i -= 1;  /* keep the same element number.                      */
				break;
			case DOS:
				if( system( "c:\\modems\\password.exe" ) == -1 )
					printf( "\n\n System FAILED" );
				if( system( "command" ) == -1 )
					printf( "\n\n System FAILED" );
				i -= 1;
				break;
			case ESC:
				i = NUMDAC;
				break;
		}  /* end switch */

	/* return the screen to its original configuration...                   */
	restore_scrn( orig );
	fclose( filterhelp );
}  /* end main */

/***************************************************************************
Function:      DAC_status
Purpose:       to put the modem status on the screen
Arguments:     option, D0_values array, , D1 values array, command file name,
					status file name, elements, symbol rate array, range array.
Returns:       1 if successful, 0 if fail
****************************************************************************/
int DAC_status( char D0_values[][3], char D1_values[][3], char cmd_path[],
					 char sta_path[], char elements[][3], char sym_rate[][7],
					 char range[][2] )
{
	int      i, j, k, start_index, check, lyne;
	char     txt[MAXSTR], command[5];
	FILE     *sf;

	/* update the power table status...                                     */
	if( pc2modem( cmd_path, "", "", CMDFILE, RS485 ) == FAIL )
		return( FAIL );

	/* open the status file for read...                                     */
	if( (sf = getfp( sta_path, "r" )) == NULL )
		return( FAIL );

	/* go through the status file line-by-line and get the element numbers
		and DAC values from each status response line...							*/
	lyne = 0;
	while( !feof( sf ) )
	{
		check = YES;
		/* read in a line from the file...												*/
		fgets( txt, MAXSTR, sf );

		/* only check lines that have status information in them...				*/
		if( !strcmp( txt, ">1/REM_\n" ) || !strcmp( txt, "\n" ) )
			check = NO;
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

			/* get the command portion of the status response...              */
			i = start_index;
			j = 0;
			while( txt[i] != '_' )
				command[j++] = txt[i++];
			command[j] = '\0';
			if( strcmp( command, "TST2" ) != 0 )
			{
				window( 5, 5, 60, 10 );
				clrscr();
				gotoxy( 2, 2 );
				cprintf( "The command file used had invalid commands." );
				gotoxy( 2, 4 );
				cprintf( "Press any key to end..." );
				getch();

				return( FAIL );
			}  /* end if */

			/* find out whether the mod or the demod was run...               */
			++i;     /* skip the '_'                                          */
			board_type[0] = txt[i++];     /* skip the '_'                     */
			board_type[1] = '\0';

			/* get the element portion of the response...                     */
			j = 0;
			while( txt[i] != '_' )
				elements[lyne][j++] = txt[i++];
			elements[lyne][j] = '\0';

			/* skip the '_' after the element...                              */
			++i;

			/* get the range (1 or 0)...                                      */
			j = 0;
			while( txt[i] != '_' )
				range[lyne][j++] = txt[i++];
			range[lyne][j] = '\0';

			/* skip the '_' after the range...                                */
			++i;

			/* get the DAC values from the response string...                 */
			/* D0...                                                          */
			k = 0;
			while( txt[i] != '_' )
				D0_values[lyne][k++] = txt[i++];
			D0_values[lyne][k] = '\0';

			/* ...D1...																			*/
			k = 0;
			++i;
			while( txt[i] != '_' )
				D1_values[lyne][k++] = txt[i++];
			D1_values[lyne][k] = '\0';

			/* get the symbol rate from the response...                       */
			++i;
			k = 0;
			while( txt[i] != '\n' && txt[i] != '\r' )
				sym_rate[lyne][k++] = txt[i++];
			sym_rate[lyne][k] = '\0';

			++lyne;
		}  /* end if */
	}  /* end while */

	fclose( sf );
	return( OK );
}  /* end DAC_status */

/***************************************************************************
Function:      ask_DAC_correct
Purpose:       to ask the user if the current DAC values gives the correct
					power setting
Arguments:     element string
Returns:       YES if it power is in spec, NO if it is not; -1 if calloc fails;
					ESC to end the program.
****************************************************************************/
int ask_DAC_correct( char *element, int elem_index, char *D0_value,
							char *D1_value, char *range )
{
	int      valid;
	char     tmpstr[MAXSTR], cmd[MAXSTR], resp[MAXSTR];
	POPWIN   *adc;
	HELPINFO help;

	/* create a popup window for asking user if correct/not correct...      */
	if( (adc = PWALLOC) == NULL )
	{
		window( 5, 5, 60, 10 );
		clrscr();
		gotoxy( 2, 2 );
		cprintf( "Cannot allocate memory for popup window" );
		gotoxy( 2, 3 );
		cprintf( "in ask_DAC_correct" );
		gotoxy( 2, 5 );
		cprintf( "Press any key to end..." );
		getch();

		return( -1 );
	}  /* end if */

	adc -> lft     = 5;
	adc -> top     = 10;
	adc -> numrows = 7;
	if( !strcmp( board_type, "M" ) )
		sprintf( adc -> mes, "%s Variable Alias Filter Adjust", "Modulator" );
	else if( !strcmp( board_type, "D" ) )
		sprintf( adc -> mes, "%s Variable Alias Filter Adjust", "Demodulator" );
	sprintf( tmpstr,
				"\n\r\n\r Is the Response for BP %s Within Specification?",
																						element );
	strcat( adc -> mes, tmpstr );
	if( elem_index > 0 )
	{
		strcat( adc -> mes, "\n\r\n\r " );
		strcpy( adc -> cfgstr, "PGDN-Yes  Insert-No  PGUP-Prev  ESC-End" );
		strcat( adc -> cfgstr, "  F1-Help  S-Status" );
		strcat( adc -> mes, adc -> cfgstr );
	}
	else
	{
		strcat( adc -> mes, "\n\r\n\r " );
		strcpy( adc -> cfgstr, "PGDN-Yes  Insert-No  ESC-End" );
		strcat( adc -> cfgstr, "  F1-Help  S-Status" );
		strcat( adc -> mes, adc -> cfgstr );
	}
	sprintf( tmpstr, "\n\r\n\r %s", version );
	strcat( adc -> mes, tmpstr );

	adc = popwin( adc, ON );
	gotoxy( adc -> lft, adc -> top );

	/* write filter data to hardware so response changes on analyzer...     */
	sprintf( cmd, "<1/REM_\r\n<1/TST4_%s%s_%s_%s_%s\r\n<1/RF_ON\r\n",
				board_type, element, range, D0_value, D1_value );
	strcpy( resp, "" );
	if( pc2modem( "", cmd, resp, ARGUMENT, RS485 ) == FAIL )
		return( FAIL );

	/* get the user's response...                                           */
	valid = NO;
	while( !valid )
	{
		valid = YES;
		switch( getch() )
		{
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
			case 'n':case 'N':
				adc = popwin( adc, OFF );
				free( adc );
				return( NO );
			case EXTENDED_KEY:
				valid = NO;
				switch( getch() )
				{
					case F1:
						help.fp        = filterhelp;
						help.field     = 1;
						help.numfields = 1;
						help.block     = 0;
						strcpy( help.opttxt, "Help For Variable Filter Adjust" );
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
					case INSERT:
						adc = popwin( adc, OFF );
						free( adc );
						return( NO );
					case ALTD:
						adc = popwin( adc, OFF );
						free( adc );
						return( DOS );
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
Function:      change_DAC
Purpose:       to change the DAC values remotely
Arguments:     current element, D0 string, D1 string, range, symbol rate
Returns:       OK if it was changed; FAIL if it failed; -1 if file could not
					be opened; -2 if popwin memory couldn't be allocated.
****************************************************************************/
int change_DAC( char *element, char *D0_value, char *D1_value, char *range,
					 char *sym_rate )
{
	int      keep_going, valid;
	char     cmd[MAXSTR], resp[MAXSTR];
	POPWIN   *menu;
	HELPINFO help;

	/* create the menu...                                             		*/
	if( (menu = create_menu( element )) == NULL )
		return( -2 );

	/* get the user's response and act on it...                             */
	keep_going = YES;
	while( keep_going )
	{
		/* while the user does not enter a valid response, keep prompting... */
		valid = NO;
		while( !valid )
		{
			valid = YES;

			/* get the user's response...                                     */
			switch( getch() )
			{
				case EXTENDED_KEY:
					switch( getch() )
					{
						case UP_ARROW:
							add_to_str( D0_value, +0x01, "hex" );
							break;
						case DOWN_ARROW:
							add_to_str( D0_value, -0x01, "hex" );
							break;
						case PGUP:
							add_to_str( D1_value, +0x01, "hex" );
							break;
						case PGDN:
							add_to_str( D1_value, -0x01, "hex" );
							break;
						case F1:
							help.fp        = filterhelp;
							help.field     = 2;
							help.numfields = 1;
							help.block     = 0;
							strcpy( help.opttxt, "Help For Changing DAC Values" );
							popup_help( help );
							valid = NO;
							break;
						case END:
							keep_going = NO;
							sprintf( cmd,
								"<1/REM_\r\n<1/TST2_%s%s_%s_%s_%s_%s\r\n<1/RF_ON\r\n",
								board_type, element, range, D0_value, D1_value,
								sym_rate );
							strcpy( resp, "" );
							if( pc2modem( "", cmd, resp, ARGUMENT, RS485 ) == FAIL )
							{
                     	menu = popwin( menu, OFF );
								free( menu );
								return( FAIL );
							}
							break;
						default:
							valid = NO;
							break;
					}  /* end switch */
					if( valid && keep_going )
					{
						sprintf( cmd,
								"<1/REM_\r\n<1/TST4_%s%s_%s_%s_%s\r\n<1/RF_ON\r\n",
								board_type, element, range, D0_value, D1_value );
						strcpy( resp, "" );
						if( pc2modem( "", cmd, resp, ARGUMENT, RS485 ) == FAIL )
						{
               		menu = popwin( menu, OFF );
							free( menu );
							return( FAIL );
						}
					}  /* end if */
					break;
				default:
					valid = NO;
					break;
			}  /* end switch */
		}  /* end while */
	}  /* end while */
	menu = popwin( menu, OFF );
	free( menu );

	return( OK );
}  /* end change_DAC */

/***************************************************************************
Function:      create_menu
Purpose:       creates the change DAC value menu
Arguments:     current breakpoint
Returns:       ptr to POPWIN structure
****************************************************************************/
POPWIN   *create_menu( char *breakpoint )
{
	int		maxlen, i;
	char		chgmen[6][MAXSTR];
	char     *rawstr[] = {  "Current Breakpoint:  %s\n\r\n\r ",
									"[%c].......Increase D0 Value by 1\n\r ",
									"[%c].......Decrease D0 Value by 1\n\r\n\r ",
									"[PgUp]....Increase D1 Value by 1\n\r ",
									"[PgDn]....Decrease D1 Value by 1\n\r\n\r ",
									"[End]..Finished  [F1]-Help" };
	POPWIN   *menu;

	if( (menu = PWALLOC) == NULL )
	{
		window( 5, 5, 60, 10 );
		clrscr();
		gotoxy( 2, 2 );
		cprintf( "Couldn't allocate memory for the change value menu" );
		gotoxy( 2, 4 );
		cprintf( "Press any key to end..." );
		getch();

		return( NULL );
	}  /* end if */

	menu -> lft       = 20;
	menu -> top       = 10;
	menu -> numrows   = 9;

	/* make the menu string...                                              */

	/* fill the variables in the menu strings...										*/
	sprintf( chgmen[0], rawstr[0], breakpoint );
	sprintf( chgmen[1], rawstr[1], 24 );
	sprintf( chgmen[2], rawstr[2], 25 );
	strcpy( chgmen[3], rawstr[3] );
	strcpy( chgmen[4], rawstr[4] );
	strcpy( chgmen[5], rawstr[5] );

	/* get the length of the longest string...										*/
	maxlen = (int)strlen( chgmen[0] );
	for( i = 1; i < 6; ++i )
		if( (int)strlen( chgmen[i] ) >= maxlen )
			strcpy( menu -> cfgstr, chgmen[i] );

	/* now concatenate the strings to form the menu string...					*/
	strcpy( menu -> mes, chgmen[0] );
	for( i = 1; i < 6; ++i )
		strcat( menu -> mes, chgmen[i] );

	menu = popwin( menu, ON );
	gotoxy( menu -> lft, menu -> top );
	return( menu );
}  /* end create_menu */