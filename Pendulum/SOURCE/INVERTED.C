/**************************************************************************
*    MODULE:               INVERTED.C                                     *
*                                                                         *
*    CREATED BY:           ROY CHANCELLOR                                 *
*                                                                         *
*    DATE CREATED:         11-12-92                                       *
*                                                                         *
*    DATE LAST MODIFIED:   09-08-93                                       *
*                                                                         *
*    VERSION:              1.00                                           *
*                                                                         *
*    This program animates an inverted pendulum and displays a phase      *
*    plane plot on the screen in real time.  The equations of motion are  *
*    calculated using the fourth order Runge-Kutta algorithm.             *
*                                                                         *
**************************************************************************/

#include <stdio.h>
#include <math.h>
#include <graph.h>
#include <conio.h>
#include <process.h>
#include <stdlib.h>
#include <direct.h>
#include <string.h>
#include <royware.h>
#include "c:\royware\pendulum\include\pendsim.h"

struct pend p;
struct axis_data phs_pln, anima;
struct vport pend, phas;

void main( void )
{
	get_pend_data( &p, "\\inverted.dat", 'i' );
	setup_anima();
	setup_phs_pln();
	get_vport_info( &pend, &phas );
	calc_points( p, pend, phas );
}  /* end main */

/***************************************************************************/

void calc_points( struct pend p, struct vport pend, struct vport phas )
{
	int         which, old_sav_flag, npts = 0, save_flag = FALSE;
	int         show_graphics = YES, cnt = 0, dum_char, dummy;
	char        txt[60];
	double      t, dt;
	double      th_new, om_new, th_old, om_old;
	double      c_I, kt_I, mg_I, tho, wd;
	double      x, y;
	static char resp_nam[] = {"\\ipndresp.dat"};
	static char path[_MAX_PATH + 20];
	struct      videoconfig vc;
	char        direc[_MAX_PATH + 20], buf = 'C';
	FILE        *pendresp, *fopen();

	_getvideoconfig( &vc );

	_getdcwd( buf - 'A' + 1, path, _MAX_PATH );
	strcpy( direc, path );
	strcat( direc, resp_nam );

	if( (pendresp = fopen( direc, "w" ) ) != NULL )
	{
		c_I  = p.c_I;
		kt_I = p.kt_I;
		mg_I = p.mg_I;
		tho  = p.tho;
		wd   = p.wd;
		dt   = 2.000 * PI / p.pts_per_cycle / wd;

		th_old = p.theta_0 * PI / 180.0;
		om_old = p.omeg_0;
		t = 0.0;

		_settextposition( 2, 1 );
		_settextcolor( CYAN );
		sprintf( txt, "Initial Pendulum Position. Hit any key to Start." );
		_outtext( txt );

		draw_pend( th_old, 0, anima, pend, 'i' );
		if( save_flag )
			fprintf( pendresp, "%lf     %lf     %lf\n", t, th_old, om_old );

		dum_char = getch();

		_settextposition( 2, 1 );
		_settextcolor( BLACK );
		_outtext( txt );

		old_sav_flag = save_flag;
		while( t <= p.tmax )
		{
			if( kbhit() != 0 )
				prog_control( vc.adapter, &save_flag, &show_graphics, pendresp,
								  &dummy, " ", ANIMATION );

			runge( th_old, om_old, &th_new, &om_new, t, dt, kt_I,
																tho, c_I, mg_I, wd, 'i' );
			if( fabs( th_new ) > PI )
				th_new -= 2 * PI * fabs( th_new ) / th_new;
			t += dt;

			if( show_graphics )
			{
				draw_pend( th_new, th_old, anima, pend, 'i' );
				draw_phase( th_new, om_new, phs_pln, th_old, om_old, phas );
			}
			++cnt;
			if( cnt == 100 )
			{
				switch( pend.adapt )
				{
					case _EGA:
						_settextposition( 24, 12 );
						break;
					case _VGA:
						_settextposition( 29, 12 );
						break;
				}  /* end switch */
				_settextcolor( YELLOW );
				sprintf( txt, "Time %5.1lf", t );
				_outtext( txt );
				cnt = 0;
			}  /* end if */

			th_old = th_new;
			om_old = om_new;

			if( save_flag )
				fprintf( pendresp, "%lf    %lf    %lf\n", t, th_new, om_new );
		} /* end while */

		fclose( pendresp );
	} /* end if */
	else
	{
		_setvideomode( _DEFAULTMODE );
		printf( "\n\n\n\n\n\n Can't open %s.  Hit any key to end.", direc );
		getch();
		exit( 11 );
	}
	_settextposition( 3, 12 );
	_settextcolor( YELLOW );
	sprintf( txt, "Finished.  Hit any key." );
	_outtext( txt );
	getch();
	_settextposition( 3, 12 );
	_settextcolor( BLACK );
	_outtext( txt );
}  /* end calc_points */
