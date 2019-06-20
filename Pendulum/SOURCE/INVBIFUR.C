/**************************************************************************
*    MODULE:               INVBIFUR.C                                     *
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
struct axis_data bif_diag;
struct bif bif;
struct calc_params cp;

void main( void )
{
	get_bifur_data( &bif, "\\invbifur.dat" );
	get_pend_data( &p, "\\inverted.dat", 'i' );
	setup_graph( "c:\\pendulum\\ibifmap.dat" );
	calc_bif_points( p, &bif, &cp );
}  /* end main */

/**************************************************************************/

void calc_bif_points( struct pend p, struct bif *bif, struct calc_params *cp )
{
	int         which, n, pts_per_cyc, back_color, pt_color, cnt = 0;
	int         save_flag, show_graphics;
	char        txt[60];
	double      t, dt, dt_poin, t_poin, dt_ph, t_ph;
	double      phi_des, phi_t, phi_next, frac, dphi;
	double      th_new, om_new, th_old, om_old, th_ful, om_ful, th_p1, om_p1;
	double      th_p2, om_p2, th_err, om_err, th_poin, om_poin;
	double      c_I, mg_I, kt_I, tho, wd, dc_I, dtho, dwd, tmp, tmp1;
	static char resp_nam[] = {"\\ibifpts.dat"};
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
		kt_I = p.kt_I;
		mg_I = p.mg_I;

		if( strcmp( bif -> bif_par, "displacement\n" ) == 0 )
		{
			cp -> tho    = bif -> bp_i;
			cp -> dtho   = bif -> delta_bp;
			cp -> thomax = bif -> bp_f;
			cp -> c_I    = p.c_I;
			cp -> dc_I   = 0;
			cp -> c_Imax = p.c_I + 1;
			cp -> wd     = p.wd;
			cp -> dwd    = 0;
			cp -> wdmax  = p.wd + 1;
		}  /* end if */
		else
			if( strcmp( bif -> bif_par, "frequency\n" ) == 0 )
			{
				cp -> wd     = bif -> bp_i;
				cp -> dwd    = bif -> delta_bp;
				cp -> wdmax  = bif -> bp_f;
				cp -> c_I    = p.c_I;
				cp -> dc_I   = 0;
				cp -> c_Imax = p.c_I + 1;
				cp -> tho    = p.tho;
				cp -> dtho   = 0;
				cp -> thomax = p.tho + 1;
			}  /* end if */
		else
			if( strcmp( bif -> bif_par, "damping\n" ) == 0 )
			{
				cp -> c_I    = bif -> bp_i;
				cp -> dc_I   = bif -> delta_bp;
				cp -> c_Imax = bif -> bp_f;
				cp -> tho    = p.tho;
				cp -> dtho   = 0;
				cp -> thomax = p.tho + 1;
				cp -> wd     = p.wd;
				cp -> dwd    = 0;
				cp -> wdmax  = p.wd + 1;
			}  /* end if */
		else
		{
			_clearscreen( _GCLEARSCREEN );
			printf( "\n\n %s is an invalid bifurcation parameter.",
																			bif -> bif_par );
			printf( "\n\n The Valid Ones Are 'AMPLITUDE', 'FREQUENCY', AND" );
			printf( " 'DAMPING'.\n\n Enter a Correct Choice and Restart" );
			getch();
			exit( 11 );
		}  /* end else */

		c_I = cp -> c_I;
		tho = cp -> tho;
		wd   = cp -> wd;
		dc_I = cp -> dc_I;   /* all of this is just to save text space */
		dtho = cp -> dtho;
		dwd  = cp -> dwd;

		phi_des = p.poin_phase;
		dt      = 2.0 * PI / wd / p.pts_per_cycle;

		par_status( c_I, tho, wd, 0 );

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

		while( c_I <= cp -> c_Imax && tho <= cp -> thomax &&
																			wd <= cp -> wdmax )
		{
			th_old = p.theta_0 * PI / 180.0;  /* puts theta into radians   */
			om_old = p.omeg_0;
			t = 0.0;

			_setcolor( pt_color );

			while( t <= p.tmax )
			{
				if( kbhit() != 0 )
					prog_control( vc.adapter, &save_flag, &show_graphics, bifdat,
									  &pt_color, "c:\\pendulum\\ibifmap.dat",
									  FULL_SCREEN );

				runge( th_old, om_old, &th_new, &om_new, t, dt, kt_I,
																tho, c_I, mg_I, wd, 'i' );

				t += dt;
				n  = (int)( wd * t / 2.0 / PI );

				phi_t = wd * t;
				dphi  = wd * dt;

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
						runge( th_new, om_new, &th_ful, &om_ful, t,
											dt_poin, kt_I, tho, c_I, mg_I, wd, 'i' );
				/*  ...take two half steps...                                  */
						runge( th_new, om_new, &th_p1, &om_p1, t,
											dt_ph, kt_I, tho, c_I, mg_I, wd, 'i' );
						runge( th_p1, om_p1, &th_p2, &om_p2, t_ph,
											dt_ph, kt_I, tho, c_I, mg_I, wd, 'i' );

						th_err = fabs( th_p2 - th_ful );
						om_err = fabs( om_p2 - om_ful );
						th_poin = th_p2 + th_err / 15.0;  /*  see Chapra and     */
						om_poin = om_p2 + om_err / 15.0;  /*  Canale, p.613      */

						if( fabs( th_poin ) > PI )
							th_poin -= 2.0 * PI * fabs( th_poin ) / th_poin;

						if( dc_I == 0 && dwd == 0 )
						{
							if( strcmp( bif -> phase_var, "displacement\n" ) == 0 )
							{
								if( save_flag )
									fprintf( bifdat, "%lf     %lf\n", tho, th_poin );
								if( show_graphics )
								{
									_setcolor( pt_color );
									_setpixel_w( tho, th_poin );
								}
							}  /* end if */
							else
								if( strcmp( bif -> phase_var, "velocity\n" ) == 0 )
								{
									if( save_flag )
										fprintf( bifdat, "%lf   %lf\n", tho, om_poin );
									if( show_graphics )
									{
										_setcolor( pt_color );
										_setpixel_w( tho, om_poin );
									}
								}  /* end if */
						}  /* end if */
						else
							if( dc_I == 0 && dtho == 0 )
							{
								if( strcmp( bif -> phase_var, "displacement\n" ) == 0 )
								{
									if( save_flag )
										fprintf( bifdat, "%lf     %lf\n", wd, th_poin );
									if( show_graphics )
									{
										_setcolor( pt_color );
										_setpixel_w( wd, th_poin );
									}
								}  /* end if */
								else
									if( strcmp( bif -> phase_var, "velocity\n" ) == 0 )
									{
										if( save_flag );
											fprintf( bifdat, "%lf  %lf\n", wd, om_poin );
										if( show_graphics )
										{
											_setcolor( pt_color );
											_setpixel_w( wd, om_poin );
										}
									}  /* end if */
							}  /* end else */
						else
							if( dtho == 0 && dwd == 0 )
							{
								if( strcmp( bif -> phase_var, "displacement\n" ) == 0 )
								{
									if( save_flag )
										fprintf( bifdat, "%lf   %lf\n", c_I, th_poin );
									if( show_graphics )
									{
										_setcolor( pt_color );
										_setpixel_w( c_I, th_poin );
									}
								}  /* end if */
								else
									if( strcmp( bif -> phase_var, "velocity\n" ) == 0 )
									{
										if( save_flag )
											fprintf( bifdat, "%lf %lf\n", c_I, om_poin );
										if( show_graphics )
										{
											_setcolor( pt_color );
											_setpixel_w( c_I, om_poin );
										}
									}  /* end if */
							}  /* end if */
					}  /* end if */

					if( fabs( th_new ) > PI )
						th_new -= 2.0 * PI * fabs( th_new ) / th_new;

					th_old = th_new;
					om_old = om_new;

				++cnt;
				if( cnt == 100 )
				{
					switch( vc.adapter )
					{
						case _EGA:
							_settextposition( 24, 22 );
							break;
						case _VGA:
							_settextposition( 30, 22 );
							break;
					}  /* end switch */
					_settextcolor( YELLOW );
					sprintf( txt, "%5.1lf Sec.", t );
					_outtext( txt );
					cnt = 0;
				}  /* end if */
			}  /* end while */

			c_I += dc_I;
			tho += dtho;
			wd += dwd;
			par_status( c_I, tho, wd, 1 );
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
	sprintf( txt, "Finished.  Hit any key to end." );
	_outtext( txt );
	getch();
	_settextposition( 3, 31 );
	_settextcolor( BLACK );
	_outtext( txt );
}  /* end calc_bif_points */

/**************************************************************************/

void par_status( double c_I, double tho, double wd, int which )
{
	char     txt[70];
	struct   videoconfig vc;

	_getvideoconfig( &vc );

	switch( vc.adapter )
	{
		case _VGA:
			if( which == 0 )
			{
				_settextcolor( CYAN );
				_settextposition( 29, 15 );
				sprintf( txt, "Amp:   %5.4lf          Freq:  %5.4lf", tho, wd );
				_outtext( txt );
				_settextposition( 30, 15 );
				_settextcolor( YELLOW );
				sprintf( txt, "Time:  " );
				_outtext( txt );
				_settextcolor( CYAN );
				_settextposition( 30, 35 );
				sprintf( txt, "Damping:  %5.4lf", c_I );
				_outtext( txt );
			}  /* end if */
			else
			{
				_settextcolor( CYAN );
				_settextposition( 29, 21 );
				sprintf( txt, "%5.4lf", tho );
				_outtext( txt );
				_settextposition( 30, 45 );
				sprintf( txt, "%5.4lf", wd );
				_outtext( txt );
				_settextposition( 30, 45 );
				sprintf( txt, "%5.4lf", c_I );
				_outtext( txt );
			}  /* end if */
			break;
		case _EGA:
			if( which == 0 )
			{
				_settextcolor( CYAN );
				_settextposition( 24, 15 );
				sprintf( txt, "Amp:  %5.4lf  Freq:  %5.4lf", tho, wd );
				_outtext( txt );
				_settextposition( 25, 15 );
				_settextcolor( YELLOW );
				sprintf( txt, "Time:  " );
				_outtext( txt );
				_settextcolor( CYAN );
				_settextposition( 25, 35 );
				sprintf( txt, "Damping:  %5.4lf", c_I );
				_outtext( txt );
			}  /* end if */
			else
			{
				_settextcolor( CYAN );
				_settextposition( 24, 21 );
				sprintf( txt, "%5.4lf", tho );
				_outtext( txt );
				_settextposition( 25, 35 );
				sprintf( txt, "%5.4lf", wd );
				_outtext( txt );
				_settextposition( 25, 45 );
				sprintf( txt, "%5.4lf", c_I );
				_outtext( txt );
			}  /* end if */
			break;
	}  /* end switch */
}  /* end par_status */
