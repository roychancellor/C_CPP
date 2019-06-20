/**************************************************************************
*    MODULE:               DATFILMU.C                                     *
*                                                                         *
*    CREATED BY:           ROY CHANCELLOR                                 *
*                                                                         *
*    DATE LAST MODIFIED:   09-08-93                                       *
*                                                                         *
*    VERSION:              1.00                                           *
*                                                                         *
*    This program displays a menu of options for editing pendulum anim-   *
*    ation data files.                                                    *
*                                                                         *
**************************************************************************/
#include <stdio.h>
#include <process.h>
#include <stdlib.h>
#include <errno.h>
#include <graph.h>
#include <conio.h>
#include <royware.h>
#include "c:\roy\quickc\pend\pendsim.h"

#define QUIT_PROG 2
#define GO_AGAIN 1
#define START_ROW 7
#define DELTA_ROW 2

struct videoconfig screen;
void showmenu( void );

void main( void )
{
	int which, selection = INVALID, status = GO_AGAIN, row, pt;
   char txt[80];

	while( selection == INVALID )
   {
		_getvideoconfig( &screen );

		switch( screen.adapter )
		{
			case _EGA:
				_setvideomode( _ERESCOLOR );
				_setwindow( TRUE, -10, -10, 10, 10 );
				_setcolor( YELLOW );
				_rectangle_w( _GBORDER, -7, -7, 7, 7 );
				break;
			case _VGA:
				_setvideomode( _VRES16COLOR );
				_setwindow( TRUE, -10, -10, 10, 10 );
				_setcolor( YELLOW );
				_rectangle_w( _GBORDER, -7, -4, 7, 7 );
				break;
			default:
				break;
		}  /* end switch */

		_settextcolor( YELLOW );
		_settextposition( 3, 26 );
		sprintf( txt, "Pendulum Data File Edit Menu" );
		_outtext( txt );

		row = START_ROW;
		_settextcolor( CYAN );
		_settextposition( row,16 );
		sprintf( txt, " Edit Pendulum Parameter Data File-----------[1]" );
		_outtext( txt );
		row += DELTA_ROW;
		_settextposition( row,16 );
		sprintf( txt, " Edit Bifurcation Parameter File-------------[2]" );
		_outtext( txt );
		row += DELTA_ROW;
		_settextposition( row,16 );
		sprintf( txt, " Edit Phase Portrait Coordinates-------------[3]" );
		_outtext( txt );
		row += DELTA_ROW;
		_settextposition( row,16 );
		sprintf( txt, " Edit Bifurcation Diagram Coordinates--------[4]" );
		_outtext( txt );
		row += DELTA_ROW;
		_settextposition( row,16 );
		sprintf( txt, " Edit Poincar‚ Map Coordinates---------------[5]" );
		_outtext( txt );
		row += DELTA_ROW;
		_settextposition( row,16 );
		sprintf( txt, " Edit General Graph Coordinates--------------[6]" );
		_outtext( txt );
		row += DELTA_ROW;
		_settextposition( row,16 );
		_settextcolor( 14 );
		sprintf( txt, " Return to Main Menu-------------------------[7]" );
		_outtext( txt );
		which = getch();

		switch( which )
		{
			case '1':
				showmenu();
				pt = getch();
				if( pt == 'r' || pt == 'R' )
				{
					_setvideomode( _DEFAULTMODE );
					system( "edit pendat.dat" );
				}
				else
					if( pt == 'i' || pt == 'I' )
					{
						_setvideomode( _DEFAULTMODE );
						system( "edit inverted.dat" );
					}
				_setvideomode( _DEFAULTMODE );
				break;
			case '2':
				showmenu();
				pt = getch();
				if( pt == 'r' || pt == 'R' )
				{
					_setvideomode( _DEFAULTMODE );
					bifeditr( "c:\\pendulum\\regbifur.dat" );
				}
				else
					if( pt == 'i' || pt == 'I' )
					{
						_setvideomode( _DEFAULTMODE );
						bifeditr( "c:\\pendulum\\invbifur.dat" );
					}
				_setvideomode( _DEFAULTMODE );
				break;
			case '3':
				_setvideomode( _DEFAULTMODE );
				grpeditr( "c:\\pendulum\\phasanim.dat" );
				break;
			case '4':
				showmenu();
				pt = getch();
				if( pt == 'r' || pt == 'R' )
				{
					_setvideomode( _DEFAULTMODE );
					grpeditr( "c:\\pendulum\\rbifmap.dat" );
				}
				else
					if( pt == 'i' || pt == 'I' )
					{
						_setvideomode( _DEFAULTMODE );
						grpeditr( "c:\\pendulum\\rbifmap.dat" );
					}
				_setvideomode( _DEFAULTMODE );
				break;
			case '5':
				showmenu();
				pt = getch();
				if( pt == 'r' || pt == 'R' )
				{
					_setvideomode( _DEFAULTMODE );
					grpeditr( "c:\\pendulum\\rpoinmap.dat" );
				}
				else
					if( pt == 'i' || pt == 'I' )
					{
						_setvideomode( _DEFAULTMODE );
						grpeditr( "c:\\pendulum\\ipoinmap.dat" );
					}
				break;
			case '6':
				_setvideomode( _DEFAULTMODE );
				grpeditr( "c:\\pendulum\\genlgrap.dat" );
				break;
			case '7':
				selection = VALID;
			default:
				break;
		}  /* end switch */
	}  /* end while */
	_setvideomode( _DEFAULTMODE );
} /* end main */

/***************************************************************************/

void showmenu( void )
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
}  /* end showmenu */
