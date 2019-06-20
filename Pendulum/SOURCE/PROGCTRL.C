/**************************************************************************
*    MODULE:               PROGCTRL.C                                     *
*                                                                         *
*    CREATED BY:           ROY CHANCELLOR                                 *
*                                                                          *
*    DATE CREATED:         09-08-93                                       *
*                                                                         *
*    DATE LAST MODIFIED:   09-12-93                                       *
*                                                                         *
*    VERSION:              1.00                                           *
*                                                                         *
*    This module performs the program control funtions when a key is      *
*    pressed during simulation operation.  It is located in pendsim.lib   *
*                                                                         *
**************************************************************************/
#include <stdio.h>
#include <graph.h>
#include <royware.h>
#include <conio.h>
#include <process.h>
#include "c:\roy\quickc\pend\pendsim.h"

void prog_control( short adapter, struct sim_control *sc, FILE *fp,
																					struct pend p )
{
	int which, back_color;
	struct axis_data dummy;

	switch( adapter )
	{
		case _EGA:
			_setviewport( 0, 0, 639, 349 );
			break;
		case _VGA:
			_setviewport( 0, 0, 639, 480 );
			break;
		default:
			break;
	}  /* end switch */
	display_message( 10, *sc );
	getch();
	which = getch();
	switch( which )
	{
		case 'g':case 'G':
			display_message( 11, *sc );
			switch( sc -> sim_type )
			{
				case ANIMATION:
					printf( "\a" );
					break;
				case FULL_SCREEN:
					grpeditr( p.grph_fname );
					setup_graph( p.grph_fname );
					back_color = _getcolor();
					if( back_color == BLACK )
						sc -> pt_color = YELLOW;
					else
						sc -> pt_color = BLACK;
					break;
			}  /* end switch */
			break;
		case 'e':case 'E':
			_setvideomode( _DEFAULTMODE );
			fclose( fp );
			exit( 10 );
		case 'r':case 'R':
			display_message( 11, *sc );
			switch( sc -> sim_type )
			{
				case ANIMATION:
					setup_animation( p.pend_fname, 'a', &dummy );
					setup_animation( p.grph_fname, 'p', &dummy );
					break;
				case FULL_SCREEN:
					setup_graph( p.grph_fname );
					back_color = _getcolor();
					if( back_color == BLACK )
						sc -> pt_color = YELLOW;
					else
						sc -> pt_color = BLACK;
					break;
			}  /* end switch */
			break;
		case 's':case 'S':
			display_message( 11, *sc );
			sc -> save_flag = reverse( sc -> save_flag );
			break;
		case 'd':case 'D':
			display_message( 11, *sc );
			sc -> show_graphics = reverse( sc -> show_graphics );
			break;
		default:
			display_message( 11, *sc );
			break;
	}  /* end switch */
}  /* end prog_control */
