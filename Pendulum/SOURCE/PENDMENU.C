/**************************************************************************
*    MODULE:               PENDMENU.C                                     *
*                                                                         *
*    CREATED BY:           ROY CHANCELLOR                                 *
*                                                                         *
*    DATE LAST MODIFIED:   09-12-93                                       *
*                                                                         *
*    VERSION:              1.00                                           *
*                                                                         *
*    This program displays a menu of options for pendulum animation using *
*    the programs "REGULAR.C", "INVERTED.C", "PHAS_SCR.C", "POINGRAP",    *
*    and "PEND_SCR.C".                                                    *
*                                                                         *
**************************************************************************/
#include <stdio.h>
#include <process.h>
#include <stdlib.h>
#include <errno.h>
#include <graph.h>
#include <conio.h>
#include <royware.h>

#define QUIT_PROG 2
#define GO_AGAIN 1
#define START_ROW 5
#define DELTA_ROW 2

struct videoconfig screen;
void put_selections( void );

void main( void)
{
	int pt;
	int which, selection = INVALID, row;
   char txt[80];

	while( selection == INVALID )
   {
		_getvideoconfig( &screen );
		_setvideomode( _VRES16COLOR );

		_settextcolor( YELLOW );
		_settextposition( 1, 30 );
		sprintf( txt, "Pendulum Animation Menu" );
		_outtext( txt );

		_setwindow( TRUE, -10, -10, 10, 10 );
		_setcolor( YELLOW );
		_rectangle_w( _GBORDER, -8, -2.3, 8, 8 );

		row = START_ROW;
		_settextcolor( CYAN );
		_settextposition( row,16 );
		sprintf( txt, " Pendulum Animation and Phase Space----------[1]" );
		_outtext( txt );
		row += DELTA_ROW;
		_settextposition( row,16 );
		sprintf( txt, " Bifurcation Diagram-------------------------[2]" );
		_outtext( txt );
		row += DELTA_ROW;
		_settextposition( row,16 );
		sprintf( txt, " Poincar‚ Map--------------------------------[3]" );
		_outtext( txt );
		row += DELTA_ROW;
		_settextposition( row,16 );
		sprintf( txt, " Edit The Input Data Files-------------------[4]" );
		_outtext( txt );
		row += DELTA_ROW;
		_settextposition( row,16 );
		sprintf( txt, " Plot A Data File Stored on Disk-------------[5]" );
		_outtext( txt );
		row += DELTA_ROW;
		_settextposition( row,16 );
		sprintf( txt, " Shell to DOS--------------------------------[6]" );
		_outtext( txt );
		row += DELTA_ROW;
		_settextcolor( LIGHT_RED );
		_settextposition( row,16 );
		sprintf( txt, " Exit Program--------------------------------[7]" );
		_outtext( txt );
		which = getch();

		switch( which )
		{
			case '1':
				put_selections();
				pt = getch();
				if( pt == 'r' || pt == 'R' )
				{
					_setvideomode( _DEFAULTMODE );
					system( "pendanim regular" );
				}
				else
					if( pt == 'i' || pt == 'I' )
					{
						_setvideomode( _DEFAULTMODE );
						system( "pendanim inverted" );
					}
				_setvideomode( _DEFAULTMODE );
				break;
			case '2':
				put_selections();
				pt = getch();
				if( pt == 'r' || pt == 'R' )
				{
					_setvideomode( _DEFAULTMODE );
					system( "bifrcate regular" );
				}
				else
					if( pt == 'i' || pt == 'I' )
					{
						_setvideomode( _DEFAULTMODE );
						system( "bifrcate inverted" );
					}
				_setvideomode( _DEFAULTMODE );
				break;
			case '3':
				put_selections();
				pt = getch();
				if( pt == 'r' || pt == 'R' )
				{
					_setvideomode( _DEFAULTMODE );
					system( "poincare regular" );
				}
				else
					if( pt == 'i' || pt == 'I' )
					{
						_setvideomode( _DEFAULTMODE );
						system( "poincare inverted" );
					}
				_setvideomode( _DEFAULTMODE );
				break;
			case '4':
				_setvideomode( _DEFAULTMODE );
				system( "datfilmu.exe" );
				break;
			case '5':
				_setvideomode( _DEFAULTMODE );
				system( "plotting.exe" );
				break;
			case '6':
				_setvideomode( _DEFAULTMODE );
				system( "command.com" );
				break;
			case '7':
				_setvideomode( _DEFAULTMODE );
				selection = VALID;
				break;
			default:
				break;
		}  /* end switch */
	}  /* end while */
} /* end main */

/***************************************************************************/

void put_selections( void )
{
	char txt[60];

	_clearscreen( _GCLEARSCREEN );
	_settextcolor( YELLOW );
	_settextposition( 12, 29 );
	sprintf( txt, "Input the Pendulum Type" );
	_outtext( txt );
	_settextposition( 14, 27 );
	_settextcolor( CYAN );
	sprintf( txt, "  Regular Pendulum------[R]" );
	_outtext( txt );
	_settextposition( 15, 27 );
	sprintf( txt, "  Inverted Pendulum-----[I]" );
	_outtext( txt );
}  /* end put_selections */
