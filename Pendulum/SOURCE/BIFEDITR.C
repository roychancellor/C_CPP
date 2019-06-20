/***************************************************************************
*	Module:			BIFEDITR.C 	                                            	*
*                                                                         	*
*  Created By:		Roy Chancellor										              	*
*                                                                         	*
*  Date Created:  07-30-1993                                              	*
*  																							  	*
*	Last Modified:	06-20-1994															  	*
*                                                                         	*
*  Version:			1.00                                                    	*
*																								  	*
*	Date Released:	UNRELEASED															  	*
*                                                                         	*
*	Description:	This module allows the user to change the simulation		*
*						parameters in the bifurcation diagram simulation			*
*						interactively.  This program is to be linked with the		*
*						menu programs that will make calls to it for	editing		*
*						pendulum system values.												*
*                                                                         	*
*	Inputs:			The full path of the file to be edited.						*
*                                                                         	*
*	Returns:			1 if successful; 0 if anything failed.							*
*                                                                         	*
*	Date				Modification                              Initials      	*
* ----------	-------------------------------------------	---------	  	*
****************************************************************************/
/* Header Files that came with Microsoft Quick C...                        */
#include <stdio.h>      /* all C programs                                  */
#include <conio.h>      /* screen function calls                           */
#include <process.h>    /* exit, etc.                                      */
#include <stdlib.h>     /* standard C library                              */
#include <graph.h>      /* graphics calls                                  */
#include <string.h>     /* string function calls                           */

/* Header files custom written for this software...                        */
#include <royware.h>    /* many often-used defines and such...          	*/
#include "c:\royware\pendulum\include\pendsim.h"
#include "c:\royware\pendulum\include\bifeditr.h"

/*#define DEBUG*/
#ifdef DEBUG
int main( void )
{
	if( bifeditr( "c:\\royware\\pendulum\\data\\regbifur.dat" ) == OK )
		return( OK );
	else
		return( FAIL );
}  /* end main */
#endif

/***************************************************************************
Function:	bifeditr
Purpose:		to edit the bifurcation simulation parameters interactively on
				to computer screen.
Arguments:	the full path of the file holding the parameters.
Returns:		1 if ok; 0 if anything fails (such as file not found)
****************************************************************************/
int bifeditr( char filename[] )
{
	struct text_loc vals[NUM_FIELDS];
	struct bif      bp;

	/* configure the graphics for this editor...										*/
	bif_setup_graphics();
	/* configure the editor on the computer monitor...								*/
	bif_setup_edit_screen();
	/* Get the bifurcation simulation data from the file...						*/
	bif_get_file_info( filename, &bp );
	/*	set all the values in the text_loc structure to default values...		*/
	bif_initialize_vals( vals );
	/* put the bifurcation data on the screen in the proper fields...			*/
	bif_put_info_on_screen( bp, vals );
	/* go the the edit routine an let the user have at it...						*/
	bif_edit_away( &bp, vals );
	/* write the new (or unchanged) information back to the file...			*/
	bif_write_file_info( filename, bp );
	/* put the curson back on the screen...											*/
	_displaycursor( _GCURSORON );

	return( OK );
}  /* end bifeditr */

/***************************************************************************
Function:	bif_setup_graphics
Purpose:		to configure the computer monitor as desired.
Arguments:	none
Returns:		1 if ok; 0 if anything failed.
****************************************************************************/
int bif_setup_graphics( void )
{
	struct videoconfig monitor;

	/* get the PC's video configuration...												*/
	_getvideoconfig( &monitor );
	/* change the text cursor to a double underline style...						*/
	if( _settextcursor( DOUBLE_UNDERLINE ) == -1 )
		return( FAIL );
	/* turn the newly-changed cursor on...												*/
	_displaycursor( _GCURSORON );
	/* clear the screen...																	*/
	_clearscreen( _GCLEARSCREEN );

	/* return OK since nothing failed if it gets here...							*/
	return( OK );
}  /* end bif_setup_graphics */

/***************************************************************************
Function:	bif_setup_edit_screen
Purpose:		to configure the edit screen and put it on the PC monitor
Arguments:	none
Returns:		1 if ok; 0 if anything failed.
****************************************************************************/
int bif_setup_edit_screen( void )
{
	struct text_loc edtr[NUM_HEADINGS];
	int    i;
	char   text[MAXSTR];

	/* define what will be displayed on the screen (would like to put this
		in a file for more general use)...												*/
	strcpy(  edtr[0].txt, "Bifurcation Diagram Parameter Editor\0" );
				edtr[0].row = 2;
				edtr[0].col = 23;
				edtr[0].color   = CYAN;
				edtr[0].frmt    = "%s";
	strcpy(  edtr[1].txt, "State Variable (Y Axis):\0" );
				edtr[1].row = 6;
				edtr[1].col = 4;
				edtr[1].color   = YELLOW;
				edtr[1].frmt    = "%s";
	strcpy(  edtr[2].txt, "Bifurcation Parameter (X Axis):\0" );
				edtr[2].row = 4;
				edtr[2].col = 4;
				edtr[2].color   = YELLOW;
				edtr[2].frmt    = "%s";
	strcpy(  edtr[3].txt, "Increment on Bifurcation Parameter:\0" );
				edtr[3].row = 8;
				edtr[3].col = 4;
				edtr[3].color   = YELLOW;
				edtr[3].frmt    = "%s";
	strcpy(  edtr[4].txt, "Start Value of Bifurcation Parameter:\0" );
				edtr[4].row = 10;
				edtr[4].col = 4;
				edtr[4].color   = YELLOW;
				edtr[4].frmt    = "%s";
	strcpy(  edtr[5].txt, "End Value of Bifurcation Parameter:  \0" );
				edtr[5].row = 12;
				edtr[5].col = 4;
				edtr[5].color   = YELLOW;
				edtr[5].frmt    = "%s";
	strcpy(  edtr[6].txt, "Use [- -] to Move LEFT/RIGHT\0"  );
				edtr[6].row = 18;
				edtr[6].col = 12;
				edtr[6].color   = CYAN;
				edtr[6].frmt    = "%s";
	strcpy(  edtr[7].txt, "Use [- -] to Move UP/DOWN\0" );
				edtr[7].row = 19;
				edtr[7].col = 12;
				edtr[7].color   = CYAN;
				edtr[7].frmt    = "%s";
	strcpy(  edtr[8].txt, "Use [TAB] to Edit Parameter Values\0" );
				edtr[8].row = 20;
				edtr[8].col = 12;
				edtr[8].color   = CYAN;
				edtr[8].frmt    = "%s";
	strcpy(  edtr[9].txt, "Use [BKSP] to Toggle Bifurcation Parameter and State Variable\0" );
				edtr[9].row = 21;
				edtr[9].col = 12;
				edtr[9].color   = CYAN;
				edtr[9].frmt    = "%s";
	strcpy(  edtr[10].txt, "Press [ENTER] When Finished. Info. Will be Saved.\0" );
				edtr[10].row = 22;
				edtr[10].col = 12;
				edtr[10].color   = CYAN;
				edtr[10].frmt    = "%s";

	/* put the information on the screen...											*/
	for( i = 0; i < NUM_HEADINGS; ++i )
	{
		_settextcolor( edtr[i].color );
		_settextposition( edtr[i].row, edtr[i].col );
		sprintf( text, edtr[i].frmt, edtr[i].txt );
		_outtext( text );
		/* these special cases are for putting arrows in the help spaces
			which are array elements 6 and 7 respectively...						*/
		switch( i )
		{
			case 6:
				sprintf( text, "%c %c", 27, 26 );
				_settextposition( edtr[i].row, edtr[i].col + 5 );
				_outtext( text );
				break;
			case 7:
				sprintf( text, "%c %c", 24, 25 );
				_settextposition( edtr[i].row, edtr[i].col + 5 );
				_outtext( text );
				break;
			default:
				break;
		}  /* end switch */
	}  /* end for */

	return( OK );
}  /* end bif_setup_edit_screen */

/***************************************************************************
Function:	bif_get_file_info
Purpose:		to bet the bifurcation diagram information from the data file.
Arguments:	the file name, ptr to bifurcation diagram parameter structure
Returns:		1 if ok; 0 if anything fails
****************************************************************************/
int bif_get_file_info( char screen_nam[], struct bif *bp )
{
	FILE *sf, *fopen();

	/* open the file and read in the appropriate information...					*/
	if( (sf = fopen( screen_nam, "r" )) != NULL )
	{
		fgets( bp -> bif_par, MAXSTR - 1, sf );
		fgets( bp -> phase_var, MAXSTR - 1, sf );
		fscanf( sf, "%lf", &bp -> delta_bp );
		fscanf( sf, "%lf", &bp -> bp_i );
		fscanf( sf, "%lf", &bp -> bp_f );

		fcloseall();
		flushall();
	}  /* end if */
	else
	{
		_setvideomode( _DEFAULTMODE );
		_clearscreen( _GCLEARSCREEN );
		_settextposition( 10, 10 );
		cprintf( "%s Can Not Be Opened.", screen_nam );
		_settextposition( 12, 10 );
		cprintf( "Check To See If It Is There.  Press any key to end." );
		getch();
		return( FAIL );
	}  /* end else */
}  /* end bif_get_file_info */

/***************************************************************************
Function:	bif_initialize_vals
Purpose:		to set all the values in the screen format structure to defaults.
Arguments:	the screen format structure.
Returns:		none
****************************************************************************/
void bif_initialize_vals( struct text_loc vals[] )
{
	int i;

	for( i = 0; i < NUM_FIELDS; ++i )
	{
		strcpy( vals[i].txt, "initialized\n\0" );
		vals[i].row   = 0;
		vals[i].col   = 0;
		vals[i].val.d = 0.0;
		vals[i].val.i = 0;
		vals[i].frmt  = "formatted";
	}  /* end for */
}  /* end bif_initialize_vals */

/***************************************************************************
Function:	bif_put_info_on_screen
Purpose:		to put the bifurcation diagram simulation data on the screen.
Arguments:	the data structure, the screen format structure
Returns:		1 if ok; 0 if anything fails
****************************************************************************/
int bif_put_info_on_screen( struct bif bp, struct text_loc vals[] )
{
	char   text[MAXSTR];
	int    i/*, num_titles = 1, num_values = 12*/;

	/* define the location of the data fields and what goes in each one...	*/
	strcpy( vals[0].txt, bp.bif_par );
	vals[0].row    = 4;
	vals[0].col    = 43;
	vals[0].frmt   = "%s";
	strcpy( vals[1].txt, bp.phase_var );
	vals[1].row    = 6;
	vals[1].col    = 43;
	vals[1].frmt   = "%s";
	vals[2].val.d  = bp.delta_bp;
	vals[2].row    = 8;
	vals[2].col    = 43;
	vals[2].frmt   = "%.7lf";
	vals[3].val.d  = bp.bp_i;
	vals[3].row    = 10;
	vals[3].col    = 43 ;
	vals[3].frmt   = "%.7lf";
	vals[4].val.d  = bp.bp_f;
	vals[4].row    = 12;
	vals[4].col    = 43;
	vals[4].frmt   = "%.7lf";

	/* define the text color for the fileds and the background color...		*/
	_settextcolor( YELLOW );
	_setbkcolor( DARK_BLUE );

	/* put the values on the screen...													*/
	for( i = 0; i < NUM_FIELDS; ++i )
	{
		_settextposition( vals[i].row, vals[i].col );
		if( !strcmp( vals[i].frmt, "%.3lf" ) ||
			 !strcmp( vals[i].frmt, "%.0lf" ) ||
			 !strcmp( vals[i].frmt, "%.7lf" ) )
			sprintf( text, vals[i].frmt, vals[i].val.d );
		else if( !strcmp( vals[i].frmt, "%d" ) )
			sprintf( text, vals[i].frmt, vals[i].val.i );
		else if( !strcmp( vals[i].frmt, "%s" ) )
			sprintf( text, vals[i].frmt, vals[i].txt );
		_outtext( text );
	}  /* end for */

	return( OK );
}  /* end bif_put_info_on_screen */

/***************************************************************************
Function:	bif_edit_away
Purpose:		to let the user "edit away" on the parameters
Arguments:	ptr to bifurcation data structure, screen format structure
Returns:		none
****************************************************************************/
void bif_edit_away( struct bif *bp, struct text_loc vals[] )
{
	char   ch, text[MAXSTR];
	int    ppi, oldcolor, temp_val3, temp_val4, i, j, bp_indic = 0;
	long   oldbkcolor;
	double temp_val1, temp_val2;
	struct rccoord curpos, newpos, oldpos;
	struct text_loc str[NUM_FIELDS], tstr[NUM_FIELDS];

	/* put dummy text in the "str" dummy screen format structure...			*/
	for( i = 0; i < NUM_FIELDS; ++i )
		strcpy( str[i].txt, "initialized\0" );
	for( j = 0; j < NUM_FIELDS; ++j )
		strcpy( str[j].txt, vals[j].txt );
	ppi = 0;
	ch = 'g';

	/* set the text foreground and background colors...							*/
	_settextcolor( YELLOW );
	_setbkcolor( DARK_BLUE );

	/* set the current position...														*/
	newpos.row = vals[ppi].row;
	newpos.col = vals[ppi].col;
	oldpos = _settextposition( newpos.row, newpos.col );

	/* while the user does not press enter (CR), keet soliciting
		keystrokes and processing them...												*/
	while( ch != CR )
	{
		switch( (ch = (char)getch()) )
		{
			case EXTENDED_KEY:
				/* then get the other half...                                  */
				switch( getch() )
				{
					case DOWN_ARROW:  /* move the cursor to the next field down	*/
						if( ppi < NUM_FIELDS - 1 )
						{
							++ppi;
							_settextposition( vals[ppi].row, vals[ppi].col );
						}
						break;
					case UP_ARROW:	/* move the cursor to the next field up...	*/
						if( ppi > 0 )
						{
							--ppi;
							_settextposition( vals[ppi].row, vals[ppi].col );
						}
						break;
					default:
						break;
				}  /* end switch */
				break;
			case BACKSPACE: /* this is for toggling on/off types of values    */
				switch( ppi ) /* only do it for "type" fields     					*/
				{
					case 1:
						if( !strcmp( vals[ppi].txt, "displacement\n" ) )
							strcpy( vals[ppi].txt, "velocity\n" );
						else
							strcpy( vals[ppi].txt, "displacement\n" );
						_settextposition( vals[ppi].row, vals[ppi].col );
						sprintf( text, "            " );
						_outtext( text );
						_settextposition( vals[ppi].row, vals[ppi].col );
						sprintf( text, vals[ppi].frmt, vals[ppi].txt );
						_outtext( text );
						/* reset the original position...                        */
						_settextposition( vals[ppi].row, vals[ppi].col );
						break;
					case 0:
						if( bp_indic == 2 )
						{
							strcpy( vals[ppi].txt, "amplitude\n" );
							bp_indic = 0;
						}
						else if( bp_indic == 1 )
						{
							strcpy( vals[ppi].txt, "frequency\n" );
							bp_indic = 2;
						}
						else
						{
							strcpy( vals[ppi].txt, "damping\n" );
							bp_indic = 1;
						}
						_settextposition( vals[ppi].row, vals[ppi].col );
						_outtext( "          " );
						_settextposition( vals[ppi].row, vals[ppi].col );
						sprintf( text, vals[ppi].frmt, vals[ppi].txt );
						_outtext( text );
						/* reset the original position...                        */
						_settextposition( vals[ppi].row, vals[ppi].col );
						break;
					default:
						break;
				}  /* end switch */
				break;
			case TAB:  /* then go to the edit line for user input...          */
				switch( ppi )
				{
					case 2:case 3:case 4:  /* floating point values             */
						/* make the number being edited flash                    */
						oldcolor = _gettextcolor();
						oldbkcolor = _getbkcolor();
						_settextcolor( YELLOW + FLASH_VAL );
						_settextposition( vals[ppi].row, vals[ppi].col );
						sprintf( text, vals[ppi].frmt, vals[ppi].val.d );
						_outtext( text );
						/* here's the edit line...                               */
						_settextcolor( oldcolor );
						_setbkcolor( oldbkcolor );
						bif_blanks_to_line( vals[ppi].row + 1, vals[ppi].col, 7 );
						_settextposition( vals[ppi].row + 1, vals[ppi].col - 3 );
						sprintf( text, "=> " );
						_outtext( text );
						_settextposition( vals[ppi].row + 1, vals[ppi].col );
						temp_val1 = vals[ppi].val.d;
						scanf( "%lf", &temp_val2 );
						vals[ppi].val.d = temp_val2;
						/* blank the newly entered number...                     */
						_settextcolor( BLACK );
						_setbkcolor( BLACK );
						bif_blanks_to_line( vals[ppi].row + 1,
																	vals[ppi].col - 3, 12 );
						/* blank the old number...                               */
						_settextposition( vals[ppi].row, vals[ppi].col );
						sprintf( text, vals[ppi].frmt, temp_val1 );
						_outtext( text );
						/* put the newly entered number in its place...          */
						_settextcolor( oldcolor );
						_setbkcolor( oldbkcolor );
						_settextposition( vals[ppi].row, vals[ppi].col );
						sprintf( text, vals[ppi].frmt, vals[ppi].val.d );
						_outtext( text );
						/* reset the original position...                        */
						_settextposition( vals[ppi].row, vals[ppi].col );
						break;
					default:
						break;
				}  /* end switch */
				break;
			default:
				break;
		}  /* end switch */
	}  /* end while */

	/* put all temporary values back into original structure...             */
	strcpy( bp -> bif_par,   vals[0].txt );
	strcpy( bp -> phase_var, vals[1].txt );
	bp -> delta_bp           = vals[2].val.d;
	bp -> bp_i               = vals[3].val.d;
	bp -> bp_f               = vals[4].val.d;

}  /* end bif_edit_away */

/***************************************************************************
Function:	bif_write_file_info
Purpose:		to write the bifurcation diagram information to the data file
Arguments:	filename, bifurcation diagram data
Returns:    1 if ok; 0 if anything fails
****************************************************************************/
int bif_write_file_info( char filename[], struct bif bp )
{
	int  i;
	FILE *sf, *fopen();

	/* append a \n on each string that does not already have one...         */
	i = 0;
	while( bp.bif_par[i] != '\0' )
		++i;
	if( bp.bif_par[i - 1] != '\n' )
		strcat( bp.bif_par, "\n" );
	i = 0;
	while( bp.phase_var[i] != '\0' )
		++i;
	if( bp.phase_var[i - 1] != '\n' )
		strcat( bp.phase_var, "\n" );

	if( (sf = fopen( filename, "w" )) != NULL )
	{
		fputs( bp.bif_par, sf );
		fputs( bp.phase_var, sf );
		fprintf( sf, "%lf\n", bp.delta_bp );
		fprintf( sf, "%lf\n", bp.bp_i );
		fprintf( sf, "%lf\n", bp.bp_f );

		fclose( sf );
	}  /* end if */
	else
	{
		_setvideomode( _DEFAULTMODE );
		printf( "\n\n\n\n\n\n\n" );
		printf( "%s Can Not Be Opened.  Hit any key to end.", filename );
		getch();
		return( FAIL );
	}  /* end else */

	return( OK );
}  /* end bif_write_file_info */

/***************************************************************************
Function:	bif_blanks_to_line
Purpose:		to put blanks on a line at the row and column specified
Arguments:	row, column, number of spaces to put
Returns:		none
****************************************************************************/
void bif_blanks_to_line( int row, int col, int num_spcs )
{
   int  i;
	char blanks[MAXSTR];

	strcpy( blanks, " " );
	for( i = 0; i < num_spcs; ++i )
   	strcat( blanks, " " );
   _settextposition( row, col );
   _outtext( blanks );
}  /* end bif_blanks_to_line */
