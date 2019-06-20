/**************************************************************************
*    MODULE:               POINCARE.C                                     *
*                                                                         *
*    CREATED BY:           ROY CHANCELLOR                                 *
*                                                                         *
*    DATE CREATED:         11-21-92                                       *
*                                                                         *
*    DATE LAST MODIFIED:   09-12-93                                       *
*                                                                         *
*    VERSION:              1.00                                           *
*                                                                         *
*    This program makes a Poincar‚ map of the inverted pendulum in real   *
*    time by integrating the equations of motion using the 4th order      *
*    Runge-Kutta algorithm with an adaptive step size.  The data is also  *
*    written to a file.                                                   *
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
#include "c:\roy\quickc\pend\pendsim.h"

void calc_poin_points( struct pend p );

struct pend p;

void main( int argc, char *argv[] )
{
	get_pend_data( &p, argv[1] );
	calc_poin_points( p );
}  /* end main */

/***************************************************************************/

void calc_poin_points( struct pend p )
{
	int         which, n, back_color, cnt = 0;
	char        txt[60];
	double      t, dt, dt_poin, t_poin, dt_ph, t_ph;
	double      th_err, om_err, th_p1, om_p1, th_p2, om_p2, th_poin, om_poin;
	double      th_ful, om_ful;
	double      phi_des, phi_t, phi_next, frac, dphi;
	double      th_new, om_new, th_old, om_old, rp[5];
	double      th_nh, om_nh, th_n, om_n;
	double      x, y;
	static char path[_MAX_PATH + 20];
	char        direc[_MAX_PATH + 20], buf = 'C';
	struct      videoconfig vc;
	struct      sim_control sc;
	FILE        *rspptr, *fopen();

	switch( p.pend_type )
	{
		case 'i':
			strcpy( p.resp_fname, "\\ipoinrsp.dat" );
			strcpy( p.grph_fname, "\\ipoinmap.dat" );
			break;
		case 'r':
			strcpy( p.resp_fname, "\\rpoinrsp.dat" );
			strcpy( p.grph_fname, "\\rpoinmap.dat" );
			break;
	}  /* end switch */

	_getdcwd( buf - 'A' + 1, path, _MAX_PATH );
	strcpy( direc, path );
	strcat( direc, p.resp_fname );

	setup_graph( strcat( path, p.grph_fname ) );
	back_color = _getcolor();
	if( back_color == BLACK )
		sc.pt_color = YELLOW;
	else
		sc.pt_color = BLACK;

	_getvideoconfig( &vc );
	switch( vc.adapter )
	{
		case _EGA:
			sc.time_row = 25;
			break;
		case _VGA:
			sc.time_row = 30;
			break;
	}  /* end switch */
	sc.time_col = 60;
	_settextcolor( YELLOW );
	_settextposition( sc.time_row, sc.time_col - 11 );
	_outtext( "Time, sec:" );

	if( (rspptr = fopen( direc, "w" ) ) == NULL )
	{
		_setvideomode( _DEFAULTMODE );
		printf( "\n\n\n\n\n\n Can't open %s.  Hit any key to end.", direc );
		getch();
		exit( 11 );
	}
	else
	{
		switch( p.pend_type )
		{
			case 'i':
				rp[DAMP]   = p.c_I;
				rp[SPRING] = p.kt_I;
				rp[MASS]   = p.mg_I;
				rp[AMPL]   = p.tho;
				rp[FREQ]   = p.wd;
				break;
			case 'r':
				rp[DAMP]   = p.q;
				rp[SPRING] = 000.000;
				rp[MASS]   = 000.000;
				rp[AMPL]   = p.g;
				rp[FREQ]   = p.wd;
				break;
		}  /* end switch */
		phi_des    = p.poin_phase;
		dt         = 2.0 * PI / p.wd / p.pts_per_cycle;

		th_old = p.theta_0 * PI / 180.0;
		om_old = p.omeg_0;
		t = 0.0;

		_settextposition( 3, 31 );
		_settextcolor( CYAN );
		sprintf( txt, "Hit Any Key to Start.");
		_outtext( txt );
		getch();
		_settextposition( 3, 31 );
		_settextcolor( BLACK );
		_outtext( txt );

		sc.show_graphics = YES;
		sc.save_flag     = NO;

		while( t <= p.tmax )
		{
			if( kbhit() != 0 )
				prog_control( vc.adapter, &sc, rspptr, p );

			runge( th_old, om_old, &th_new, &om_new, t, dt, rp, p.pend_type );

			t += dt;
			n  = (int)( rp[FREQ] * t / 2.0 / PI );

			phi_t = rp[FREQ] * t;
			dphi  = rp[FREQ] * dt;

			if( phi_t > 2.0 * PI )
				phi_t -= (2.0 * PI * n);    /* keep it in the proper range    */
			phi_next = phi_t + dphi;

			if( phi_t < phi_des && phi_next > phi_des && t > p.tmin )
			{
				frac    = (phi_des - phi_t) / dphi;
				t_poin  = t + frac * dt;
				dt_poin = t_poin - t;
				dt_ph   = dt_poin / 2.0;
				t_ph    = t + dt_ph;

			/*  ...take one full step...                                   */
				runge( th_new, om_new, &th_ful, &om_ful, t, dt_poin, rp,
																					p.pend_type );
			/*  ...take  two half steps... */
				runge( th_new, om_new, &th_p1, &om_p1, t, dt_ph, rp,
																					p.pend_type );
				runge( th_p1, om_p1, &th_p2, &om_p2, t_ph, dt_ph, rp,
																					p.pend_type );

				th_err  = fabs( th_p2 - th_ful );
				om_err  = fabs( om_p2 - om_ful );
				th_poin = th_p2 + th_err / 15.0;
				om_poin = om_p2 + om_err / 15.0;  /* see num. anal. book p.613 */

				if( fabs( th_poin ) > PI )
					th_poin -= 2.0 * PI * fabs( th_poin ) / th_poin;

				_setcolor( sc.pt_color );
				if( sc.show_graphics )
					_setpixel_w( th_poin, om_poin );
				if( sc.save_flag )
					fprintf( rspptr, "%lf\t%lf\t%lf\n", t_poin, th_poin, om_poin );
			}  /* end if */

			if( fabs( th_new ) > PI )
				th_new -= 2.0 * PI * fabs( th_new ) / th_new;

			++cnt;
			if( cnt == 100 )
			{
				_settextposition( sc.time_row, sc.time_col );
				_settextcolor( YELLOW );
				sprintf( txt, "%.1lf", t );
				_outtext( txt );
				cnt = 0;
			}

			th_old = th_new;
			om_old = om_new;
		} /* end while */

		fclose( rspptr );
	} /* end if */

	_settextposition( 3, 29 );
	_settextcolor( YELLOW );
	sprintf( txt, "Finished.  Hit any key to end." );
	_outtext( txt );
	getch();
	_settextposition( 3, 29 );
	_settextcolor( BLACK );
	_outtext( txt );
}  /* end calc_poin_points */
