/**************************************************************************
*    MODULE:               REGBIFUR.C                                     *
*                                                                         *
*    CREATED BY:           ROY CHANCELLOR                                 *
*                                                                         *
*    DATE CREATED:         11-21-92                                       *
*                                                                         *
*    DATE LAST MODIFIED:   09-08-93                                       *
*                                                                         *
*    VERSION:              1.00                                           *
*                                                                         *
*    This program integrates the equations of motion for a damped, driven *
*    pendulum and creates data in a disk file for a bifurcation diagram.  *
*    The control parameters are forcing amplitude, forcing frequency, and *
*    damping value.  The bifurcation variable is either angular position  *
*    or angular velocity.  The bifuracation parameter is at the Poincare  *
*    phase angle.                                                         *
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

void calc_bif_points( struct pend, struct bif *, struct calc_params * );
void par_status( double, double, double, int );

struct pend p;
struct bif bif;
struct calc_params cp;

void main( void )
{
	get_bifur_data( &bif, "\\regbifur.dat" );
	get_pend_data( &p, "\\pendat.dat", 'r' );
	setup_graph( "c:\\pendulum\\rbifmap.dat" );
	calc_bif_points( p, &bif, &cp );
}  /* end main */

/**************************************************************************/

void calc_bif_points( struct pend p, struct bif *bif, struct calc_params *cp )
{
	int         which, n, back_color, pt_color, cnt = 0;
	int         save_flag, show_graphics;
	char        txt[60];
	double      t, dt, dt_poin, t_poin, dt_ph, t_ph;
	double      th_err, om_err, th_p1, om_p1, th_p2, om_p2, th_poin, om_poin;
	double      th_ful, om_ful;
	double      phi_des, phi_t, phi_next, frac, dphi;
	double      th_new, om_new, th_old, om_old;
	double      th_nh, om_nh, th_n, om_n, d1, d2, dt_adap, t_adap, delta;
	double      q, g, par3 = 0, par4 = 0, wd, dq, dg, dwd, tmp, tmp1;
	static char resp_nam[] = {"\\rbifpts.dat"};
	static char path[_MAX_PATH + 20];
	char        direc[_MAX_PATH + 20], buf = 'C';
	struct      videoconfig vc;
	FILE        *bifdat, *fopen();

	_getvideoconfig( &vc );
	back_color = _getcolor();
	if( back_color == BLACK )
		pt_color = YELLOW;
	else
		pt_color = BLACK;

	_getdcwd( buf - 'A' + 1, path, _MAX_PATH );
	strcpy( direc, path );
	strcat( direc, resp_nam );

	if( (bifdat = fopen( direc, "w" ) ) != NULL )
	{
		if( strcmp( bif -> bif_par, "amplitude\n" ) == 0 )
		{
			cp -> g     = bif -> bp_i;
			cp -> dg    = bif -> delta_bp;
			cp -> gmax  = bif -> bp_f;
			cp -> q     = p.q;
			cp -> dq    = 0;
			cp -> qmax  = p.q + 1;
			cp -> wd    = p.wd;
			cp -> dwd   = 0;
			cp -> wdmax = p.wd + 1;
		}  /* end if */
		else
			if( strcmp( bif -> bif_par, "frequency\n" ) == 0 )
			{
				cp -> wd    = bif -> bp_i;
				cp -> dwd   = bif -> delta_bp;
				cp -> wdmax = bif -> bp_f;
				cp -> q     = p.q;
				cp -> dq    = 0;
				cp -> qmax  = p.q + 1;
				cp -> g     = p.g;
				cp -> dg    = 0;
				cp -> gmax  = p.g + 1;
			}  /* end if */
		else
			if( strcmp( bif -> bif_par, "damping\n" ) == 0 )
			{
				cp -> q     = bif -> bp_i;
				cp -> dq    = bif -> delta_bp;
				cp -> qmax  = bif -> bp_f;
				cp -> g     = p.g;
				cp -> dg    = 0;
				cp -> gmax  = p.g + 1;
				cp -> wd    = p.wd;
				cp -> dwd   = 0;
				cp -> wdmax = p.wd + 1;
			}  /* end if */
		else
		{
			_clearscreen( _GCLEARSCREEN );
			printf( "\n\n %s is an invalid bifurcation parameter",
																			bif -> bif_par );
			printf( "\n\n The Valid Ones Are 'AMPLITUDE', 'FREQUENCY', And" );
			printf( " 'DAMPING'.\n\n Enter a Correct Choice and Restart." );
			getch();
			exit( 11 );
		}  /* end else */

		q = cp -> q;
		g = cp -> g;
		wd   = cp -> wd;
		dq   = cp -> dq;   /* all of this is just to save text space */
		dg   = cp -> dg;
		dwd  = cp -> dwd;

		phi_des = p.poin_phase;
		dt      = 2.0 * PI / wd / p.pts_per_cycle;

		par_status( q, g, wd, 0 );

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

		while( q <= cp -> qmax && g <= cp -> gmax && wd <= cp -> wdmax )
		{
			th_old = p.theta_0 * PI / 180.0;  /* puts theta into radians */
			om_old = p.omeg_0;
			t = 0;

			_setcolor( pt_color );

			while( t <= p.tmax )
			{
				if( kbhit() != 0 )
					prog_control( vc.adapter, &save_flag, &show_graphics, bifdat,
									  &pt_color, "c:\\pendulum\\rbifmap.dat",
									  FULL_SCREEN );

				runge( th_old, om_old, &th_new, &om_new, t, dt, q,
																g, par3, par4, wd, 'r' );
				t += dt;
				n = (int)( wd * t / 2.0 / PI );

				phi_t = wd * t;
				dphi = wd * dt;

				if( phi_t > 2.0 * PI )
					phi_t -= (2.0 * PI * n);   /* keep it in the proper range */
				phi_next = phi_t + dphi;

				if( phi_t < phi_des && phi_next > phi_des && t > p.tmin )
				{
					frac    = (phi_des - phi_t) / dphi;
					t_poin  = t + frac * dt;
					dt_poin = t_poin - t;
					dt_ph   = dt_poin / 2.0;
					t_ph    = t + dt_ph;

			/*  ...take one full step...                                   */
					runge( th_new, om_new, &th_ful, &om_ful, t,
											dt_poin, q, g, par3, par4, wd, 'r' );
			/*  ...take two half steps...                                  */
					runge( th_new, om_new, &th_p1, &om_p1, t,
											dt_ph, q, g, par3, par4, wd, 'r' );
					runge( th_p1, om_p1, &th_p2, &om_p2, t_ph,
											dt_ph, q, g, par3, par4, wd, 'r' );
					th_err = fabs( th_p2 - th_ful );
					om_err = fabs( om_p2 - om_ful );
					th_poin = th_p2 + th_err / 15.0;  /*  see Chapra and */
					om_poin = om_p2 + om_err / 15.0;  /*  Canale, p.613  */

					if( fabs( th_poin ) > PI )
						th_poin -= 2.0 * PI * fabs( th_poin ) / th_poin;

					if( dq == 0 && dwd == 0 )
					{
						if( strcmp( bif -> phase_var, "displace\n" ) == 0 )
						{
							fprintf( bifdat, "%lf     %lf\n", g, th_poin );
							_setcolor( pt_color );
							_setpixel_w( g, th_poin );
						}
						else
							if( strcmp( bif -> phase_var, "velocity\n" ) == 0 )
							{
								fprintf( bifdat, "%lf   %lf\n", g, om_poin );
								_setcolor( pt_color );
								_setpixel_w( g, om_poin );
							}
					}  /* end if */
					else
						if( dq == 0 && dg == 0 )
						{
							if( strcmp( bif -> phase_var, "displace\n" ) == 0 )
							{
								fprintf( bifdat, "%lf     %lf\n", wd, th_poin );
								_setcolor( pt_color );
								_setpixel_w( wd, th_poin );
							}
							else
								if( strcmp( bif -> phase_var, "velocity\n" ) == 0 )
								{
									fprintf( bifdat, "%lf   %lf\n", wd, om_poin );
									_setcolor( pt_color );
									_setpixel_w( wd, om_poin );
								}
						}  /* end if */
					else
						if( dg == 0 && dwd == 0 )
						{
							if( strcmp( bif -> phase_var, "displace\n" ) == 0 )
							{
								fprintf( bifdat, "%lf   %lf\n", q, th_poin );
								_setcolor( pt_color );
								_setpixel_w( q, th_poin );
							}
							else
								if( strcmp( bif -> phase_var, "velocity\n" ) == 0 )
								{
									fprintf( bifdat, "%lf   %lf\n", q, om_poin );
									_setcolor( pt_color );
									_setpixel_w( q, om_poin );
								}
						}  /* end if */
				}  /* end if */

				if( fabs( th_new ) > PI )
					th_new -= 2.0 * PI * fabs( th_new ) / th_new;

				th_old = th_new;
				om_old = om_new;

				++cnt;
				if( cnt == 100 )
				{
					_settextcolor( YELLOW );
					_settextposition( 30, 22 );
					sprintf( txt, "%5.1lf Sec.", t );
					_outtext( txt );
					cnt = 0;
				}
			}  /* end while */

			q += dq;
			g += dg;
			wd += dwd;
			par_status( q, g, wd, 1 );
		} /* end while */

		fclose( bifdat );
	} /* end if */
	else
	{
		_setvideomode( _DEFAULTMODE );
		printf( "\n\n\n\n\n\n Can't Open %s.  Hit any key to end.", direc );
		getch();
		exit( 11 );
	}

	_settextposition( 3, 31 );
	_settextcolor( YELLOW );
	sprintf( txt, "Finished.  Hit Any Key to End." );
	_outtext( txt );
	getch();
	_settextposition( 3, 31 );
	_settextcolor( BLACK );
	_outtext( txt );

}  /* end calc_bif_points */

/**************************************************************************/

void par_status( double q, double g, double wd, int which )
{
	char txt[70];

	if( which == 0 )
	{
		_settextcolor( CYAN );
		_settextposition( 29, 15 );
		sprintf( txt, "Amp:  %5.4lf  Freq:  %5.4lf", g, wd );
		_outtext( txt );
		_settextposition( 30, 15 );
		_settextcolor( YELLOW );
		sprintf( txt, "Time:  " );
		_outtext( txt );
		_settextcolor( CYAN );
		_settextposition( 30, 35 );
		sprintf( txt, "Damping:  %5.4lf", q );
		_outtext( txt );
	}
	else
	{
		_settextcolor( CYAN );
		_settextposition( 29, 21 );
		sprintf( txt, "%5.4lf", g );
		_outtext( txt );
		_settextposition( 29, 35 );
		sprintf( txt, "%5.4lf", wd );
		_outtext( txt );
		_settextposition( 30, 45 );
		sprintf( txt, "%5.4lf", q );
		_outtext( txt );
	}
}  /* end par_status */
