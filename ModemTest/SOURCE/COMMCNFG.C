/***************************************************************************
*  Module:        COMMCNFG.C                                               *
*                                                                          *
*  Created By:    Roy Chancellor                                           *
*                                                                          *
*  Date Created:  03-24-1994                                               *
*                                                                          *
*  Last Modified: 05-10-1994                                               *
*                                                                          *
*  Version:       1.00                                                     *
*                                                                          *
*  Date Released: 05-10-1994                                               *
*                                                                          *
*  Description:   Changes communications parameters on screen              *
*                                                                          *
*  Inputs:        none                                                     *
*                                                                          *
*  Returns:       1 if ok; 0 if anything fails                             *
*                                                                          *
*  Date           Modification                                    Initials *
* ----------   -------------------------------------------        ---------*
*  03-24-1994  Released as version 0.00                              RSC   *
*  04-01-1994  Wasn't making screen right size; ver to 0.01          RSC   *
*  04-04-1994  added a terminate w/o save function; ver to 0.02      RSC   *
*	04-26-1994	added capability for 19,200 baud; ver to 0.03			RSC	*
*	05-10-1994	RELEASED FOR USE ON CAC AS VERSION 1.00					RSC	*
****************************************************************************/
/* Include Files that came with Turbo C...                                 */
#include <stdio.h>      /* all C programs                                  */
#include <conio.h>      /* screen functions                                */
#include <string.h>     /* string functions                                */
#include <alloc.h>      /* calloc                                          */
#include <stdlib.h>     /* for max                                         */

/* Include Files that were custom written for project...                   */
#include <utility.h>    /* getfp, restore_scrn                             */
#include <boolean.h>    /* values that are 1 or 0                          */
#include <ascii.h>      /* key scan codes                                  */
#include <commcnfg.h>   /* function prototypes and defines for commcnfg    */

#define MAXSTR	100

/*#define DEBUG*/
#ifdef DEBUG
int main( void )
{
	return( configure_communications() );
}  /* end main */
#endif

/***************************************************************************
Function:      configure_communications
Purpose:       to configure communications parameters on screen
Arguments:     none
Returns:       1 if ok; 0 if anything fails
****************************************************************************/
int configure_communications( void )
{
	int      winsize, i, j, k, equal_index, row, len, maxlen, difflen;
	int      fieldcol, field, keep_going, status, title_len;
	int      prompt_len, write_changes;
	char     *picture, prompt[MAXSTR];
	char     cfgitems[NUM_CFG_ITEMS][2][MAXSTR], leader[50], tmpstr[10];
	char     *title = "Change Communications Configuration";
	struct   text_info orig;
	FILE     *fp;

	/* get original screen configuration...                                 */
	gettextinfo( &orig );

	/* open communications configuration file...                            */
	if( (fp = getfp( "pc2modem.txt", "r" )) == NULL )
		return( FAIL );

	/* read in the text from the file...                                    */
	for( i = 0; i < NUM_CFG_ITEMS; ++i )
		fgets( cfgitems[i][0], MAXSTR, fp );
	fclose( fp );

	/* find the '=' in each string and put the text to the right of the '='
		in the [1] element of each array...												*/
	maxlen = 0;
	for( i = 0; i < NUM_CFG_ITEMS; ++i )
	{
		j = 0;
		k = 0;
		/* get the index to the '=' character...										*/
		while( cfgitems[i][0][j++] != '=' );

		/* save the index of the '=' character (j is incremented after the
			'=' is found, so subtract 1 from the current value of j)...			*/
		equal_index = j - 1;

		/* read until the end of the line and store everything to the right
			of the '=' character in the [1] element of the array...				*/
		while( cfgitems[i][0][j] != '\n' )
			cfgitems[i][1][k++] = cfgitems[i][0][j++];
		cfgitems[i][1][k] = '\0';

		/* cut the string at the '=' character by putting a null character
			in the string at that point...												*/
		cfgitems[i][0][equal_index] = '\0';

		/* test for the length of the current string against the maximum
			length string found...															*/
		if( (len = (int)strlen( cfgitems[i][0] )) >= maxlen )
			maxlen = len;
	}  /* end for */

	/* set the colors and window...                                         */
	textcolor( CC_FORE );
	textbackground( CC_BACK );

	/* put the text in a FULL SCREEN window...										*/
	title_len	= (int)strlen( title );
	maxlen 	 	= max( title_len, maxlen );
	window( 1, 1, 80, 25 );

	/* take a picture of the screen where the change communication window
		will go...                                                           */
	winsize = 80 * 25 * 2;
	if( (picture = calloc( (size_t)winsize, (size_t)sizeof( int ) )) == NULL )
	{
		window( 2, 2, 60, 10 );
		clrscr();
		gotoxy( 2, 2 );
		cprintf( "Not enough memory for taking a picture where the comm." );
		gotoxy( 2, 3 );
		cprintf( "config window will go." );
		gotoxy( 2, 5 );
		cprintf( "Press any key to end..." );
		getch();

		return( FAIL );
	}  /* end if */

	gettext( 1, 1, 80, 25, picture );
	clrscr();

	/* put a title on the screen...                                         */

	gotoxy( (80 - title_len) / 2, TITLEROW );
	cprintf( title );

	/* put a prompt on the screen...                                        */
	strcpy( prompt,
			  "[X]=YES  [ ]=NO  [ESC]-End/SAVE  [C]-Change  [Q]-Quit/NO SAVE" );
	prompt_len = (int)strlen( prompt );
	gotoxy( (80 - prompt_len) / 2, PROMPTROW );
	cprintf( prompt );

	/* put the strings on the screen.  For a "yes" put an X in the [ ].
		For a "no", leave the [ ] as is...												*/
	row = CC_TXT_START_ROW;
	for( i = 0; i < NUM_CFG_ITEMS; ++i )
	{
		/* put the text on the screen...													*/
		gotoxy( CC_TXT_START_COL, row );
		cprintf( cfgitems[i][0] );

		/* make a leader of '.' from the end of the text string to the start
			of the value field...															*/
		strcpy( leader, "" );
		difflen = maxlen - (int)strlen( cfgitems[i][0] ) + LEADERLENGTH;
		for( j = 0; j < difflen; ++j )
			strcat( leader, "." );

		/* put the field on the screen...												*/
		if( !strcmp( cfgitems[i][1], "yes" ) )
			strcat( leader, "[X]" );
		else if( !strcmp( cfgitems[i][1], "no" ) )
			strcat( leader, "[ ]" );
		else
		{
			sprintf( tmpstr, "[%s]", cfgitems[i][1] );
			strcat( leader, tmpstr );
		}
		cprintf( leader );
		row += SKIP_ROW;
	}  /* end for */

	fieldcol = CC_TXT_START_COL + maxlen + LEADERLENGTH + 1;
	row      = CC_TXT_START_ROW;
	field    = 0;
	gotoxy( fieldcol, CC_TXT_START_ROW );

	/* get the user's response and act on it...                             */
	keep_going = YES;
	while( keep_going )
	{
		write_changes = YES;
		switch( getch() )
		{
			case SPACE:             /* change the value                       */
			case 'c':case 'C':
				change_value( cfgitems[field][1], row, fieldcol );
				break;
			case ESC:               /* exit                                   */
				keep_going = NO;
				break;
			case EXTENDED_KEY:      /* move up/down or help                   */
				switch( getch() )
				{
					case UP_ARROW:    /* move up one                            */
						if( field > 0 )
						{
							--field;
							row -= SKIP_ROW;
							gotoxy( fieldcol, row );
						}
						break;
					case DOWN_ARROW:  /* move down one                          */
						if( field < NUM_CFG_ITEMS - 1 )
						{
							++field;
							row += SKIP_ROW;
							gotoxy( fieldcol, row );
						}
						break;
					case F1:          /* help                                   */
						break;
					default:
						break;
				}  /* end switch */
				break;
			case 'q':case 'Q':
				keep_going     = NO;
				write_changes  = NO;
				break;
			default:
				break;
		}  /* end switch */
	}  /* end while */

	if( write_changes )
	{
		/* open communications configuration file...                         */
		fp = NULL;
		if( (fp = getfp( "pc2modem.txt", "w" )) == NULL )
		{
			restore_scrn( orig );
			return( FAIL );
		}

		/* put the new configuration back in the file...                     */
		j = 0;
		for( k = 0; k < 2; ++k )
		{
			for( i = 0; i < NUM_CFG_ITEMS; ++i )
			{
				if( k == 0 )
				{
					strcat( cfgitems[i][0], "=" );
					while( cfgitems[i][0][j++] != '=' );
					cfgitems[i][0][j] = '\0';
					strcat( cfgitems[i][0], cfgitems[i][1] );
				}  /* end if */
				status = fprintf( fp, "%s\n", cfgitems[i][0] );
				if( status != (int)strlen( cfgitems[i][0] ) + 1 )
					printf( "\n\n Error." );
			}  /* end for */
			rewind( fp );
		}  /* end for */
		fclose( fp );
	}  /* end if */

	/* put the picture back on the screen...                                */
	puttext( 1, 1, 80, 25, picture );
	free( picture );

	restore_scrn( orig );
	return( OK );
}  /* end configure_communications */

/***************************************************************************
Function:      change_value
Purpose:       to change the values of the communication parameters
Arguments:     current value string, current row, current column
Returns:       none
****************************************************************************/
void  change_value( char value[], int row, int col )
{
	int      value_length, i, cr_index;
	struct   text_info now;
	static   char tempvalue[MAXSTR];

	if( !strcmp( value, "yes" ) )
	{
		strcpy( value, "no" );
		gotoxy( col - 1, row );
		cprintf( "[ ]" );
		gotoxy( col, row );
	}
	else if( !strcmp( value, "no" ) )
	{
		strcpy( value, "yes" );
		gotoxy( col - 1, row );
		cprintf( "[X]" );
		gotoxy( col, row );
	}
	else
	{
		/* get current screen attributes; change text color to the background
			color...                                                          */
		gettextinfo( &now );
		textcolor( now.attribute >> 4 );

		/* blank the old value...                                            */
		gotoxy( col, row );
		cprintf( value );

		/* reset the text color and get the new value...                     */
      textattr( now.attribute );
		gotoxy( col, row );

		/* read in the new value...														*/
		if( (value_length = (int)strlen( value )) < MAX_VALUE_LENGTH )
			value_length += (MAX_VALUE_LENGTH - value_length);
		fgets( tempvalue, value_length + 1, stdin );
		fflush( stdin );

		cr_index = 0;
		for( i = 0; i < value_length; ++i )
		{
			value[i] = tempvalue[i];
			if( value[i] == '\n' )
				cr_index = i;
		}  /* end for */

		/* take the '\n' off the value and terminate it with a null char...	*/
		value[cr_index] = '\0';

		/* put the new value on the screen...                                */
		gotoxy( col - 1, row );
		cprintf( "[%s]", value );

		/* reset the cursor position...                                      */
		gotoxy( col, row );
	}  /* end else */
}  /* end change_value */