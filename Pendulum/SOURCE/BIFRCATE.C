/**************************************************************************
*    MODULE:               BIFRCATE.C                                     *
*                                                                         *
*    CREATED BY:           ROY CHANCELLOR                                 *
*                                                                         *
*    DATE CREATED:         11-21-92                                       *
*                                                                         *
*    DATE LAST MODIFIED:   09-12-93                                       *
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
#include "c:\royware\pendulum\include\pendsim.h"

void calc_bif_points( struct pend, struct bif, struct calc_params );
void setup_bif_info( struct pend, struct calc_params *, struct bif,
																		double [], double [] );
void par_status( double [], struct sim_control *, int );
void save_and_plot( char [], struct sim_control, FILE *,
												  double [], double [], double, double );

struct pend p;
struct axis_data bif_diag;
struct bif bif;
struct calc_params cp;

void main( int argc, char *argv[] )
/*void main( void )*/
{
	get_bifur_data( &bif, /*"regular"*/ argv[1] );
	get_pend_data( &p, /*"regular"*/ argv[1] );
	calc_bif_points( p, bif, cp );
}  /* end main */

/**************************************************************************/

void calc_bif_points( struct pend p, struct bif bif, struct calc_params cp )
{
	int         which, n, back_color, cnt = 0;
	char        txt[60];
	double      t, dt, dt_poin, t_poin, dt_ph, t_ph;
	double      phi_des, phi_t, phi_next, frac, dphi;
	double      th_new, om_new, th_old, om_old, th_ful, om_ful, th_p1, om_p1;
	double      th_p2, om_p2, th_err, om_err, th_poin, om_poin;
	double      rp[5], drp[5];
	static char path[_MAX_PATH + 20];
	char        direc[_MAX_PATH + 20], buf = 'C';
	struct      videoconfig vc;
	struct      sim_control sc;
	FILE        *bifdat, *fopen();

	switch( p.pend_type )
	{
		case 'i':
			strcpy( p.resp_fname, "\\ibifresp.dat" );
			strcpy( p.grph_fname, "\\ibifmap.dat" );
			break;
		case 'r':
			strcpy( p.resp_fname, "\\rbifresp.dat" );
			strcpy( p.grph_fname, "\\rbifmap.dat" );
			break;
	}  /* end switch */

	_getdcwd( buf - 'A' + 1, path, _MAX_PATH );
	strcpy( direc, path );
	strcat( direc, p.resp_fname );

	setup_graph( strcat( path, p.grph_fname ) );

	_getvideoconfig( &vc );
	back_color = _getcolor();
	if( back_color == BLACK )
		sc.pt_color = YELLOW;
	else
		sc.pt_color = BLACK;

	if( (bifdat = fopen( direc, "w" ) ) == NULL )
	{
		_setvideomode( _DEFAULTMODE );
		printf( "\n\n\n\n\n\n Can't Open %s.  Hit any key to end.", direc );
		getch();
		exit( 11 );
	}
	else
	{
		setup_bif_info( p, &cp, bif, rp, drp );
		rp[SPRING] = p.kt_I;
		rp[MASS]   = p.mg_I;
		phi_des    = p.poin_phase;
		dt         = 2.0 * PI / rp[FREQ] / p.pts_per_cycle;

		par_status( rp, &sc, 0 );

		_settextposition( 3, 31 );
		_settextcolor( CYAN );
		sprintf( txt, "Hit Any Key to Start.");
		_outtext( txt );
		getch();
		_settextposition( 3, 31 );
		_settextcolor( BLACK );
		_outtext( txt );

		sc.save_flag     = YES;
		sc.show_graphics = YES;

		while( rp[DAMP] <= cp.dampmax && rp[AMPL] <= cp.amplmax &&
																	rp[FREQ] <= cp.freqmax )
		{
			th_old = p.theta_0 * PI / 180.0;  /* puts theta into radians   */
			om_old = p.omeg_0;
			t = 0.0;

			_setcolor( sc.pt_color );

			while( t <= p.tmax )
			{
				if( kbhit() != 0 )
					prog_control( vc.adapter, &sc, bifdat, p );

				runge( th_old, om_old, &th_new, &om_new, t, dt, rp, p.pend_type );

				t += dt;
				n  = (int)( rp[FREQ] * t / 2.0 / PI );

				phi_t = rp[FREQ] * t;
				dphi  = rp[FREQ] * dt;

				if( phi_t > 2.0 * PI )
					phi_t -= (2.0 * PI * n);    /* keep it in the proper range */
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
				/*  ...take two half steps...                                  */
					runge( th_new, om_new, &th_ful, &om_ful, t, dt_ph, rp,
																					p.pend_type );
					runge( th_new, om_new, &th_ful, &om_ful, t_ph, dt_ph, rp,
																					p.pend_type );

					th_err  = fabs( th_p2 - th_ful );
					om_err  = fabs( om_p2 - om_ful );
					th_poin = th_p2 + th_err / 15.0;  /*  see Chapra and     */
					om_poin = om_p2 + om_err / 15.0;  /*  Canale, p.613      */

					if( fabs( th_poin ) > PI )
						th_poin -= 2.0 * PI * fabs( th_poin ) / th_poin;

					save_and_plot( bif.phase_var, sc, bifdat, rp, drp,
																			th_poin, om_poin );
				}  /* end if */

				if( fabs( th_new ) > PI )
					th_new -= 2.0 * PI * fabs( th_new ) / th_new;

				th_old = th_new;
				om_old = om_new;

				++cnt;
				if( cnt == 100 )
				{
					_settextposition( sc.time_row, sc.time_col + 20 );
					_settextcolor( YELLOW );
					sprintf( txt, "%.1lf Sec.", t );
					_outtext( txt );
					cnt = 0;
				}  /* end if */
			}  /* end while */

			rp[DAMP] += drp[DAMP];
			rp[AMPL] += drp[AMPL];
			rp[FREQ] += drp[FREQ];
			par_status( rp, &sc, 1 );
		} /* end while */

		fclose( bifdat );
	} /* end else */

	_settextposition( 3, 31 );
	_settextcolor( YELLOW );
	sprintf( txt, "Finished.  Hit any key to end." );
	_outtext( txt );
	getch();
	_settextposition( 3, 31 );
	_settextcolor( BLACK );
	_outtext( txt );
}  /* end calc_bif_points */

/**************************************************************************/

void par_status( double rp[], struct sim_control *sc, int which )
{
	char     txt[70];
	struct   videoconfig vc;

	_getvideoconfig( &vc );

	if( which == 0 )
	{
		switch( vc.adapter )
		{
			case _VGA:
				sc -> amp_row = 29;
				sc -> amp_col = 4;
				sc -> freq_row = 29;
				sc -> freq_col = 35;
				sc -> damp_row = 30;
				sc -> damp_col = 4;
				sc -> time_row = 30;
				sc -> time_col = 35;
				break;
			case _EGA:
				sc -> amp_row = 24;
				sc -> amp_col = 4;
				sc -> freq_row = 24;
				sc -> freq_col = 35;
				sc -> damp_row = 29;
				sc -> damp_col = 4;
				sc -> time_row = 29;
				sc -> time_col = 35;
				break;
		}  /* end switch */
		_settextcolor( CYAN );
		_settextposition( sc -> amp_row, sc -> amp_col );
		sprintf( txt, "Forcing Amplitude:  %.4lf", rp[AMPL] );
		_outtext( txt );
		_settextposition( sc -> freq_row, sc -> freq_col );
		sprintf( txt, "Forcing Frequency:  %.4lf", rp[FREQ] );
		_outtext( txt );
		_settextposition( sc -> damp_row, sc -> damp_col + 3 );
		sprintf( txt, "System Damping:  %.4lf", rp[DAMP] );
		_outtext( txt );
		_settextposition( sc -> time_row, sc -> time_col );
		sprintf( txt, "Simulation Time:" );
		_outtext( txt );
	}  /* end if */
	else
	{
		_settextcolor( CYAN );
		_settextposition( sc -> amp_row, sc -> amp_col + 20 );
		sprintf( txt, "%.4lf", rp[AMPL] );
		_outtext( txt );
		_settextposition( sc -> freq_row, sc -> freq_col + 20 );
		sprintf( txt, "%.4lf", rp[FREQ] );
		_outtext( txt );
		_settextposition( sc -> damp_row, sc -> damp_col + 23 );
		sprintf( txt, "%.4lf", rp[DAMP] );
		_outtext( txt );
	}  /* end else */
}  /* end par_status */

/**************************************************************************/

void setup_bif_info( struct pend p, struct calc_params *cp, struct bif bif,
																  double rp[], double drp[] )
{
	if( !strcmp( bif.bif_par, "amplitude\n" ) )
	{
		switch( p.pend_type )
		{
			case 'i':
				cp -> tho     = bif.bp_i;
				cp -> dtho    = bif.delta_bp;
				cp -> amplmax = bif.bp_f;
				cp -> c_I     = p.c_I;
				cp -> dc_I    = 0.0;
				cp -> dampmax = p.c_I + 1;
				cp -> wd      = p.wd;
				cp -> dwd     = 0.0;
				cp -> freqmax = p.wd + 1;
				break;
			case 'r':
				cp -> g       = bif.bp_i;
				cp -> dg      = bif.delta_bp;
				cp -> amplmax = bif.bp_f;
				cp -> q       = p.q;
				cp -> dq      = 0.0;
				cp -> dampmax = p.q + 1;
				cp -> wd      = p.wd;
				cp -> dwd     = 0.0;
				cp -> freqmax = p.wd + 1;
				break;
		}  /* end switch */
	}  /* end if */
	else
		if( !strcmp( bif.bif_par, "frequency\n" ) )
		{
			switch( p.pend_type )
			{
				case 'i':
					cp -> wd      = bif.bp_i;
					cp -> dwd     = bif.delta_bp;
					cp -> freqmax = bif.bp_f;
					cp -> c_I     = p.c_I;
					cp -> dc_I    = 0.0;
					cp -> dampmax = p.c_I + 1;
					cp -> tho     = p.tho;
					cp -> dtho    = 0.0;
					cp -> amplmax = p.tho + 1;
					break;
				case 'r':
					cp -> wd      = bif.bp_i;
					cp -> dwd     = bif.delta_bp;
					cp -> freqmax = bif.bp_f;
					cp -> q       = p.q;
					cp -> dq      = 0.0;
					cp -> dampmax = p.q + 1;
					cp -> g       = p.g;
					cp -> dg      = 0.0;
					cp -> amplmax = p.g + 1;
					break;
			}  /* end switch */
		}  /* end if */
	else
		if( !strcmp( bif.bif_par, "damping\n" ) )
		{
			switch( p.pend_type )
			{
				case 'i':
					cp -> c_I     = bif.bp_i;
					cp -> dc_I    = bif.delta_bp;
					cp -> dampmax = bif.bp_f;
					cp -> tho     = p.tho;
					cp -> dtho    = 0.0;
					cp -> amplmax = p.tho + 1;
					cp -> wd      = p.wd;
					cp -> dwd     = 0.0;
					cp -> freqmax = p.wd + 1;
					break;
				case 'r':
					cp -> q       = bif.bp_i;
					cp -> dq      = bif.delta_bp;
					cp -> dampmax = bif.bp_f;
					cp -> g       = p.g;
					cp -> dg      = 0.0;
					cp -> amplmax = p.g + 1;
					cp -> wd      = p.wd;
					cp -> dwd     = 0.0;
					cp -> freqmax = p.wd + 1;
					break;
			}  /* end switch */
		}  /* end if */
	else
	{
		_clearscreen( _GCLEARSCREEN );
		printf( "\n\n %s is an invalid bifurcation parameter.", bif.bif_par );
		printf( "\n\n The Valid Ones Are 'AMPLITUDE', 'FREQUENCY', AND" );
		printf( " 'DAMPING'.\n\n Enter a Correct Choice and Restart" );
		getch();
		exit( 11 );
	}  /* end else */

	switch( p.pend_type )
	{
		case 'i':
			rp[DAMP]  = cp -> c_I;
			rp[AMPL]  = cp -> tho;
			rp[FREQ]  = cp -> wd;
			drp[DAMP] = cp -> dc_I;
			drp[AMPL] = cp -> dtho;
			drp[FREQ] = cp -> dwd;
			break;
		case 'r':
			rp[DAMP]  = cp -> q;
			rp[AMPL]  = cp -> g;
			rp[FREQ]  = cp -> wd;
			drp[DAMP] = cp -> dq;
			drp[AMPL] = cp -> dg;
			drp[FREQ] = cp -> dwd;
			break;
	}  /* end switch */
}  /* end setup_bif_info */

/**************************************************************************/

void save_and_plot( char phase_var[], struct sim_control sc, FILE *bifdat,
						double rp[], double drp[], double th_poin, double om_poin )
{
	if( drp[DAMP] == 0.0 && drp[FREQ] == 0.0 )
	{
		if( !strcmp( phase_var, "displacement\n" ) )
		{
			if( sc.save_flag )
				fprintf( bifdat, "%lf\t%lf\n", rp[AMPL], th_poin );
			if( sc.show_graphics )
			{
				_setcolor( sc.pt_color );
				_setpixel_w( rp[AMPL], th_poin );
			}
		}  /* end if */
		else
			if( !strcmp( phase_var, "velocity\n" ) )
			{
				if( sc.save_flag )
					fprintf( bifdat, "%lf\t%lf\n", rp[AMPL], om_poin );
				if( sc.show_graphics )
				{
					_setcolor( sc.pt_color );
					_setpixel_w( rp[AMPL], om_poin );
				}
			}  /* end if */
	}  /* end if */
	else
		if( drp[DAMP] == 0.0 && drp[AMPL] == 0.0 )
		{
			if( !strcmp( phase_var, "displacement\n" ) )
			{
				if( sc.save_flag )
					fprintf( bifdat, "%lf\t%lf\n", rp[FREQ], th_poin );
				if( sc.show_graphics )
				{
					_setcolor( sc.pt_color );
					_setpixel_w( rp[FREQ], th_poin );
				}
			}  /* end if */
			else
				if( !strcmp( phase_var, "velocity\n" ) )
				{
					if( sc.save_flag );
						fprintf( bifdat, "%lf\t%lf\n", rp[FREQ], om_poin );
					if( sc.show_graphics )
					{
						_setcolor( sc.pt_color );
						_setpixel_w( rp[FREQ], om_poin );
					}
				}  /* end if */
		}  /* end else */
	else
		if( drp[AMPL] == 0.0 && drp[FREQ] == 0.0 )
		{
			if( !strcmp( phase_var, "displacement\n" ) )
			{
				if( sc.save_flag )
					fprintf( bifdat, "%lf\t%lf\n", rp[DAMP], th_poin );
				if( sc.show_graphics )
				{
					_setcolor( sc.pt_color );
					_setpixel_w( rp[DAMP], th_poin );
				}
			}  /* end if */
			else
				if( !strcmp( phase_var, "velocity\n" ) )
				{
					if( sc.save_flag )
						fprintf( bifdat, "%lf\t%lf\n", rp[DAMP], om_poin );
					if( sc.show_graphics )
					{
						_setcolor( sc.pt_color );
						_setpixel_w( rp[DAMP], om_poin );
					}
				}  /* end if */
		}  /* end if */
}  /* end save_and_plot */
