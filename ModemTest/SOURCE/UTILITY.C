/***************************************************************************
*  Module:        UTILITY.C                                                *
*                                                                          *
*  Created By:    Roy Chancellor                                           *
*                                                                          *
*  Date Created:  02-09-1994                                               *
*                                                                          *
*  Last Modified: 05-10-1994                                               *
*                                                                          *
*  Version:       1.00                                                     *
*                                                                          *
*  Date Released: 05-10-1994                                               *
*                                                                          *
*  Description:   has utilities for opening files, converting strings to   *
*                 integers and...                                          *
*                                                                          *
*  Inputs:        several                                                  *
*                                                                          *
*  Returns:       several                                                  *
*                                                                          *
*  Date           Modification                                    Initials *
* ----------   -------------------------------------------        ---------*
*  03-09-1994  Released as version 0.00                              RSC   *
*  03-11-1994  Added restore_scrn function; ver to 0.01              RSC   *
*  03-22-1994  Added beep function; ver to 0.02                      RSC   *
*  03-29-1994  Added get_test_version function. ver to 0.03          RSC   *
*  04-06-1994  Revamped popup menu in getfp(); added fclose() to     RSC   *
*              get_test_version(); version to 0.04.                        *
*	05-10-1994	RELEASED FOR USE ON A CAC AS VERSION 1.00					RSC	*
****************************************************************************/
/* Include files that came with Turbo C...                                 */
#include <stdio.h>      /* for all C programs                              */
#include <alloc.h>      /* for calls to calloc                             */
#include <conio.h>      /* for screen calls                                */
#include <string.h>     /* for calls to strcmp, strcpy, strcat             */
#include <stdlib.h>     /* for call to atoi                                */
#include <dir.h>        /* for definition of MAXFILE                       */
#include <errno.h>      /* for use of errno                                */
#include <dos.h>        /* for sound and delay functions                   */

/* Include files custom written for EF Data test project...                */
#include <popwin.h>     /* for calls to popwin                             */
#include <utility.h>    /* for function prototypes                         */
#include <boolean.h>    /* for values that are 1 or 0                      */


/***************************************************************************
Function:      getfp
Purpose:       to open a file and return a file pointer.
Arguments:     filename, open state
Returns:       FILE pointer to open file; NULL if fails
****************************************************************************/
FILE *getfp( char *filename, char *state )
{
	int      i, maxlen_index;
	char     userstate[MAXSTR];
	char     lyne[4][MAXSTR];
	POPWIN   *cof;
	FILE     *fp;
	struct   text_info orig;

	/* get the original screen configuration...                             */
	gettextinfo( &orig );

	fp = NULL;
	if( (fp = fopen( filename, state )) == NULL )
	{
		/* allocate memory for POPWIN...                                     */
		if( (cof = PWALLOC) == NULL )
		{
			/* define a window for this message...										*/
			window( 5, 5, 60, 12 );
			clrscr();
			gotoxy( 2, 2 );
			cprintf( "Couldn't allocate memory for POPWIN to tell you that:" );
			gotoxy( 2, 4 );
			cprintf( "  %s could not be opened for %s", filename,	userstate );
			gotoxy( 2, 5 );
			perror( "Because" );
			gotoxy( 2, 7 );
			cprintf( "Press any key to end..." );
			getch();
			restore_scrn( orig );

			return( NULL );
		}  /* end if */

		/* convert state to a useful string...                               */
		switch( state[0] )
		{
			case 'r':
				strcpy( userstate, "reading" );
				break;
			case 'w':
				strcpy( userstate, "writing" );
				break;
			case 'a':
				strcpy( userstate, "appending" );
				break;
		}  /* end switch */
		if( state[1] == 'b' )
			strcat( userstate, " binary" );

		/* create popup menu...                                              */
		strcpy( lyne[0], "Could not open file:\n\r\n\r " );
		sprintf( lyne[1], "%s for %s because...\n\r\n\r ", filename, userstate );
		strcpy( lyne[2], strerror( errno ) );
		strcpy( lyne[3], "\r\n\r Press any key to end..." );

		maxlen_index = 0;
		for( i = 1; i < 4; ++i )
			if( (int)strlen( lyne[i] ) >= (int)strlen( lyne[maxlen_index] ) )
				maxlen_index = i;

		strcpy( cof -> cfgstr, lyne[maxlen_index] );
		strcpy( cof -> mes,  lyne[0] );
		for( i = 1; i < 4; ++i )
			strcat( cof -> mes, lyne[i] );

		cof -> lft     = 10;
		cof -> top     = 10;
		cof -> numrows = 7;
		cof = popwin( cof, ON );
		getch();
		cof = popwin( cof, OFF );
		free( cof );

		/* return the screen to its original state...                        */
		restore_scrn( orig );
	}  /* end if */
	return( fp );
}  /* end getfp */

/***************************************************************************
Function:      str2int
Purpose:       converts a string in the form "xxxxxxx=n" to n, where
					xxxxxxx is a string and n is an integer value.
Arguments:     string to convert.
Returns:       the integer to the right of the '='; -1 if value > 32767.
****************************************************************************/
int str2int( char txt[] )
{
	int i = 0, j = 0;
	char  value[10];

	while( txt[i++] != '=' );
	while( txt[i] != '\n' )
		value[j++] = txt[i++];
	value[j] = '\0';
	if( !strcmp( value, "yes" ) || !strcmp( value, "YES" ) )
		return( 1 );
	else if( !strcmp( value, "no" ) || !strcmp( value, "NO" ) )
		return( 0 );
	if( atol( value ) > 32767 )  /* must return a signed int...             */
	{
		clrscr();
		printf( "\n\n The value you are converting, %s, is invalid.", value );
		printf( "\n It must be <= 32767.  Press any key to end..." );
		getch();
		return( -1 );
	}
	else
		return( atoi( value ) );
}  /* end str2int */

/***************************************************************************
Function:      change_ext
Purpose:       converts a filename with any extension to one with newext.
Arguments:     cmd file name.
Returns:       none
****************************************************************************/
void change_ext( char *oldfile, char *newext )
{
	int i, filelen;

	filelen = (int)strlen( oldfile );
	for( i = 0; i < filelen; ++i )
		if( oldfile[i] == '.' )
		{
			oldfile[i + 1] = '\0';
			break;
		}
	if( i == filelen )  /* meaning the file name had no extension           */
		strcat( oldfile, "." );
	strcat( oldfile, newext );
	strcpy( newext, "" );
}  /* end change_ext */

/***************************************************************************
Function:      strip_lf
Purpose:       to strip the \n character from the end of a string
Arguments:     character string to be stripped
Returns:       none
****************************************************************************/
void strip_lf( char *str2strip )
{
	int i = 0;

	while( str2strip[i++] != '\n' && i < MAXSTR );
	str2strip[--i] = '\0';
}  /* end strip_lf */

/***************************************************************************
Function:      strip_bkslsh
Purpose:       to strip a \ from a path string if it is the last character
Arguments:     the path
Returns:       none
****************************************************************************/
void strip_bkslsh( char *path )
{
	int i, spot;

	/* strip the \ from the path if it is the last character, i.e. strip it
		if it is c:\ or d:\, but not if it is c:\sdm100 or d:\temp, etc...   */
	i = 0;
	while( path[i] != '\0' )
		if( path[i++] == '\\' )
			spot = i - 1;
	/* since i will get incremented after a \\ is spotted not on the end of
		the program name.  Thus, simply checking if the spot matches the last
		index will verify that the \\ was on the end of the string.          */
	if( spot == i - 1 )
		path[spot] = '\0';
}  /* end strip_bkslsh */

/***************************************************************************
Function:      restore_scrn
Purpose:       to reset the screen to its original coordinates before exiting
Arguments:     text_info structure
Returns:       none
****************************************************************************/
void  restore_scrn( struct text_info orig )
{
	window( orig.winleft, orig.wintop, orig.winright, orig.winbottom );
	textattr( orig.attribute );
	gotoxy( orig.curx, orig.cury );
}  /* end shutdown */

/***************************************************************************
Function:      beep
Purpose:       makes an audible beep from the computer's speaker.
Arguments:     none
Returns:       none
***************************************************************************/
void beep( void )
{
	printf( "\a" );
/* for( i = 0; i < 2; ++i )
		for( j = 0; j < 32767; ++j );
	printf( "\a" );*/

/* for( a = 1; a--; )
	{*/
		/* numbers in sound argument are the sound frequency                 */
		/* numbers in the delay argument are the delay times in millisec     */
/*    sound( 533 );
		delay( 100 );
		sound( 1600 );
		delay( 100 );
		sound( 4800 );
		delay( 100 );
		sound( 8000 );
		delay( 100 );
	}*/
	/* turn sound off...                                                    */
/* nosound();*/
}  /* end beep */

/***************************************************************************
Function:      get_test_version
Purpose:       to get the official test version number from a file
Arguments:     version string
Returns:       1 if ok; 0 if anything fails
****************************************************************************/
int   get_test_version( char test_version[] )
{
	FILE  *fp;

	if( (fp = getfp( "testvers.ion", "r" )) == NULL )
		return( FAIL );
	if( fgets( test_version, MAXSTR, fp ) == NULL )
	{
		/* define a window for this message...											*/
		window( 5, 5, 60, 10 );
		clrscr();
		gotoxy( 2, 2 );
		cprintf( "fgets returned NULL when reading the version." );
		gotoxy( 2, 4 );
		cprintf( "Press any key to end..." );
		getch();

		fclose( fp );
		return( FAIL );
	}  /* end if */
	fclose( fp );

	return( OK );
}  /* end get_test_version */