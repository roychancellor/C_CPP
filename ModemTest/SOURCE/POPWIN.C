/***************************************************************************
*  Module:        POPWIN.C                                                 *
*                                                                          *
*  Created By:    Roy Chancellor                                           *
*                                                                          *
*  Date Created:  02-02-1994                                               *
*                                                                          *
*  Last Modified: 05-10-1994                                               *
*                                                                          *
*  Version:       1.00                                                     *
*                                                                          *
*  Date Released: 05-10-1994                                               *
*                                                                          *
*  Description:   Creates a popup window on the text screen.               *
*                                                                          *
*  Inputs:        None.                                                    *
*                                                                          *
*  Returns:       None.                                                    *
*                                                                          *
*  Date           Modification                              		Initials	*
* ----------   -------------------------------------------  		---------*
*  05-10-1994	RELEASED FOR USE ON A CAC AS VERSION 1.00					RSC	*
****************************************************************************/
#include <stdio.h>
#include <conio.h>      /* for screen calls                                */
#include <alloc.h>      /* for calls to calloc                             */
#include <string.h>     /* for calls to strcpy, strcmp, strlen...          */
#include <boolean.h>    /* for values that are 1 or 0                      */
#include <popwin.h>     /* definitions and function prototypes             */

/* GLOBAL variables...                                                     */
int brcol, brrow, offset;

/***************************************************************************
Function:      popwin
Purpose:       to make a popup window on the screen.
Arguments:     top left corner coordinates, message, ON/OFF
Returns:       OK if memory could be allocated, FAIL if not.
****************************************************************************/
POPWIN *popwin( POPWIN *pw, int state )
{
	int bufsize, num_cols, *scrbuf;

	switch( state )
	{
		case ON:
			/* decide how big to make the TEXT window...                      */
			if( !strcmp( pw -> cfgstr, "" ) )
			{
				pw -> meslen   = (int)strlen( pw -> mes );
				pw -> numrows  = (int)(pw -> meslen / (MAX_WIDTH - pw -> lft) + 1);
			}
			else
				pw -> meslen   = (int)strlen( pw -> cfgstr );

			num_cols = pw -> meslen;
			offset   = 1;
			bufsize  = (pw -> numrows + 2 * ROW_SPACE + offset) *
																	(num_cols + 2 * COL_SPACE);
			brrow    = pw -> top + pw -> numrows - 1;
			brcol    = pw -> lft + num_cols - 1;
			if( pw -> meslen >= (MAX_WIDTH - pw -> lft) )
			{
				if( brrow > MAX_HEIGHT )
					brrow = MAX_HEIGHT - ROW_SPACE;
				brcol    = MAX_WIDTH - COL_SPACE;
				offset   = 0;
			}
			/* allocate memory to store the original screen state...          */
			scrbuf = calloc( (size_t)bufsize, (size_t)sizeof( int ) );
			if( scrbuf == NULL )
				return( 0 );
			/* get the screen text at window location...                      */
			gettext( pw -> lft - COL_SPACE, pw -> top - ROW_SPACE,
						brcol + COL_SPACE, brrow + ROW_SPACE + SCROLL_ROW, scrbuf );
			/* create a window for the border...                              */
			window( pw -> lft - COL_SPACE, pw -> top - ROW_SPACE,
					  brcol + COL_SPACE, brrow + ROW_SPACE + SCROLL_ROW );
			/* make a border around the window...                             */
			border( pw -> lft - COL_SPACE, pw -> top - ROW_SPACE,
					  brcol + COL_SPACE, brrow + ROW_SPACE );
			/* put the message on the screen...                               */
			window( pw -> lft - offset, pw -> top, brcol + offset, brrow );
			gotoxy( 1 + offset, 1 );
			cprintf( pw -> mes );
			gotoxy( 1 + offset, 1 );
			/* reset the window to full screen before returning...            */
			window( 1, 1, 80, 25 );
			/* assign the parts of the POPWIN structure...                    */
			pw -> ofs = offset;
			pw -> ryt = brcol;
			pw -> bot = brrow;
			pw -> buf = scrbuf;
			break;
		case OFF:
			/* put the original text back on the screen...                    */
			puttext( pw -> lft - COL_SPACE, pw -> top - ROW_SPACE,
						pw -> ryt + COL_SPACE, pw -> bot + ROW_SPACE, pw -> buf );
			/* free the allocated memory block...                             */
			free( pw -> buf );
			break;
	}  /* end switch */
	return( pw );
}  /* end popwin */

/***************************************************************************
Function:      border
Purpose:       to make a line border around the popup window.
Arguments:     the coordinates of the upper-left and lower-right corners.
Returns:       none.
****************************************************************************/
void border( int ucol, int urow, int bcol, int brow )
{
	int   row, col;
	char  topbot[81];

	/* fill the window background...                                        */
	for( row = 1; row <= brow - urow; ++row )
	{
		gotoxy( 1, row );
		clreol();
	}
	/* put the corners on the window first...                               */
	gotoxy( bcol - ucol + 1, brow - urow + 1 );  /* lower-right             */
	cprintf( "Ù" );
	gotoxy( 1, 1 );                              /* upper-left              */
	cprintf( "Ú" );
	gotoxy( 1, brow - urow + 1 );                /* lower-left              */
	cprintf( "À" );
	gotoxy( bcol - ucol + 1, 1 );                /* upper-right             */
	cprintf( "¿" );

	/* put the top and bottom lines on the border...                        */
	for( col = 0; col < bcol - ucol - 1; ++col )
		topbot[col] = 'Ä';
	topbot[col] = '\0';
	gotoxy( 2, 1 );  /* relative to window                                  */
	cprintf( topbot );
	gotoxy( 2, brow - urow + 1 );
	cprintf( topbot );

	/* put the left and right lines on the border...                        */
	for( row = 2; row <= (brow - urow); ++row )
	{
		gotoxy( 1, row );
		cprintf( "³" );
		gotoxy( bcol - ucol + 1, row );
		cprintf( "³" );
	}  /* end for */
}  /* end border */