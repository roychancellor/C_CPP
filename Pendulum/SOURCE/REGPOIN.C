/**************************************************************************
*    MODULE:               REGPOIN.C                                      *
*                                                                         *
*    CREATED BY:           ROY CHANCELLOR                                 *
*                                                                         *
*    DATE LAST MODIFIED:   09-08-93                                       *
*                                                                         *
*    VERSION:              1.00                                           *
*                                                                         *
*    This program makes a Poincare map in real time and writes the data   *
*    to the disk for later use.  The equations of motion are calculated   *
*    using the fourth order Runge-Kutta algorithm.                        *
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

void calc_poin_points( struct pend );

struct pend p;

void main( void )
{
	get_pend_data( &p, "\\pendat.dat", 'r' );
	setup_graph( "C:\\pendulum\\rpoinmap.dat" );
	calc_poin_points( p );
}  /* end main */

/***************************************************************************/

void calc_poin_points( struct pend p )
{
	int         which, n, back_color, pt_color, save_flag, show_graphics, cnt;
	char        txt[60];
	double      t, dt, dt_poin, t_poin, dt_ph, t_ph;
	double      th_err, om_err, th_p1, om_p1, th_p2, om_p2, th_poin, om_poin;
	double      th_ful, om_ful;
	double      phi_des, phi_t, phi_next, frac, dphi;
	double      th_new, om_new, th_old, om_old;
	double      th_nh, om_nh, th_n, om_n, d1, d2, dt_adap, t_adap, delta;
	double      q, g, wd;
	double      x, y;
	static char resp_nam[] = {"\\regpoin.dat"};
	static char path[_MAX_PATH + 20];
	char        direc[_MAX_PATH + 20], buf = 'C';
	struct      videoconfig vc;
	FILE        *poinresp, *fopen();

	_getvideoconfig( &vc );
	back_color = _getcolor();
	if( back_color == BLACK )
		pt_color = YELLOW;
	else
		pt_color = BLACK;

	_getdcwd( buf - 'A' + 1, path, _MAX_PATH );
	strcpy( direc, path );
	strcat( direc, resp_nam );

	if( (poinresp = fopen( direc, "w" ) ) != NULL )
	{
		cnt = 0;
		q = p.q;
		g = p.g;
		wd = p.wd;
		phi_des = p.poin_phase;
		dt = 2.0 * PI / wd / p.pts_per_cycle;

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

		save_flag     = YES;
		show_graphics = YES;

		while( t <= p.tmax )
		{
			if( kbhit() != 0 )
					prog_control( vc.adapter, &save_flag, &show_graphics, poinresp,
									  &pt_color, "c:\\pendulum\\rpoinmap.dat",
									  FULL_SCREEN );

			runge( th_old, om_old, &th_new, &om_new, t, dt, q, g,
																				0, 0, wd, 'r' );
			t += dt;
			n = (int)( wd * t / 2.0 / PI );

			phi_t = wd * t;
			dphi = wd * dt;

			if( phi_t > 2.0 * PI )
				phi_t -= (2.0 * PI * n);    /* keep it in the proper range    */
			phi_next = phi_t + dphi;

			if( phi_t < phi_des && phi_next > phi_des && t > p.tmin )
			{
				frac = (phi_des - phi_t) / dphi;
				t_poin = t + frac * dt;
				dt_poin = t_poin - t;
				dt_ph = dt_poin / 2.0;
				t_ph = t + dt_ph;

			/*  ...take one full step...                                   */
				runge( th_new, om_new, &th_ful, &om_ful, t, dt_poin, q, g,
																				0, 0, wd, 'r' );
			/*  ...take two half steps...                                  */
				runge( th_new, om_new, &th_p1, &om_p1, t, dt_ph, q, g,
																				0, 0, wd, 'r' );
				runge( th_p1, om_p1, &th_p2, &om_p2, t_ph, dt_ph, q, g,
																				0, 0, wd, 'r' );
				th_err = fabs( th_p2 - th_ful );
				om_err = fabs( om_p2 - om_ful );
				th_poin = th_p2 + th_err / 15.0;
				om_poin = om_p2 + om_err / 15.0;  /* see num. anal. book p.613 */

				if( fabs( th_poin ) > PI )
					th_poin -= 2.0 * PI * fabs( th_poin ) / th_poin;

				if( show_graphics )
				{
					_setcolor( pt_color );
					_setpixel_w( th_poin, om_poin );
				}
				if( save_flag )
					fprintf( poinresp, "%lf   %lf   %lf\n", t_poin,
																			th_poin, om_poin );
			}  /* end if */

			if( fabs( th_new ) > PI )
				th_new -= 2.0 * PI * fabs( th_new ) / th_new;

			++cnt;
			if( cnt == 100 )
			{
				_settextposition( 30, 60 );
				_settextcolor( YELLOW );
				sprintf( txt, "Time %5.1lf Sec.", t );
				_outtext( txt );
				cnt = 0;
			}

			th_old = th_new;
			om_old = om_new;
		} /* end while */

		fclose( poinresp );
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
	sprintf( txt, "Finished.  Hit any key to end." );
	_outtext( txt );
	getch();
	_settextposition( 3, 12 );
	_settextcolor( BLACK );
	_outtext( txt );
}  /* calc_poin_points */
