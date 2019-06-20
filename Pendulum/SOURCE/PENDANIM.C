/**************************************************************************
*    MODULE:               PENDANIM.C                                     *
*                                                                         *
*    CREATED BY:           ROY CHANCELLOR                                 *
*                                                                         *
*    DATE CREATED:         11-12-92                                       *
*                                                                         *
*    DATE LAST MODIFIED:   09-12-93                                       *
*                                                                         *
*    VERSION:              2.00                                           *
*                                                                         *
*    This program animates either an inverted or regular pendulum and     *
*    displays a phase plane plot on the screen in real time.              *
*    The equations of motion are calculated using the fourth order        *
*    Runge-Kutta algorithm.                                               *
*                                                                         *
**************************************************************************/

/* Header files...                                                         */
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

/* All function prototypes are in pendsim.h.                               */
/* Global variables...                                                     */
struct pend p;
struct axis_data phs_pln, anima;
struct vport pend, phas;

/*void main( void )*/
void main( int argc, char *argv[] )
{
/* get the system parameters for the pendulum, file names, etc...          */
	get_pend_data( &p, /*"regular"*/ argv[1] );
/* Run the simulation...                                                   */
	calc_points( p, pend, phas );
}  /* end main */

/***************************************************************************/

void calc_points( struct pend p, struct vport pend, struct vport phas )
{
	int         which, npts = 0, cnt = 0, dum_char, dummy;
	char        txt[MAXSTR];
	double      t, dt;
	double      th_new, om_new, th_old, om_old, rp[5];
	double      x, y;
	static char path[_MAX_PATH + 20];
	char        path2[_MAX_PATH + 20];
	struct      videoconfig vc;
	struct      sim_control sc;
	char        direc[_MAX_PATH + 20], buf = 'C';
	FILE        *rspptr, *fopen();

	switch( p.pend_type )
	{
		case 'i':
			strcpy( p.resp_fname, "\\ipndresp.dat" );
			break;
		case 'r':
			strcpy( p.resp_fname, "\\rpndresp.dat" );
			break;
	}  /* end switch */

/* get the current working directory from DOS and create the response
	filename.  All files MUST be in the current working directory.          */
	_getdcwd( buf - 'A' + 1, path, _MAX_PATH );
	strcpy( direc, path );
	strcpy( path2, path );
	strcat( direc, p.resp_fname );

	strcpy( p.grph_fname, path );
	strcat( p.grph_fname, "\\phasanim.dat" );
	strcpy( p.pend_fname, path );
	strcat( p.pend_fname, "\\pendanim.dat" );

/* setup the pendulum animation portion of the screen...                   */
	setup_animation( p.pend_fname, 'a', &anima );
/* setup the phase portrait portion of the screen...                       */
	setup_animation( p.grph_fname, 'p', &phs_pln );
/* setup the information for each of the viewports based on video adapter  */
	get_vport_info( &pend, &phas );

/* open the response data filename for write.  ERASES EXISTING FILES...    */
	if( (rspptr = fopen( direc, "w" ) ) == NULL )
	{
		_setvideomode( _DEFAULTMODE );
		printf( "\n\n\n\n\n\n Can't open %s.  Hit any key to end.", direc );
		getch();
		exit( 11 );
	}
/* if the file was opened successfully then run the simulation...          */
	else
	{
		/* get the current video configuration (interested in video adapter) */
		_getvideoconfig( &vc );

		/* set the placement of the time display on the screen...            */
		switch( vc.adapter )
		{
			case _EGA:
				sc.time_row = 24;
				break;
			case _VGA:
				sc.time_row = 29;
				break;
		}  /* end switch */
		sc.time_col      = 12;
		sc.show_graphics = YES;
		sc.save_flag     = NO;
		sc.sim_type      = ANIMATION;

		/* define the parameters to pass to the RUNGE subroutine...          */
		switch( p.pend_type )
		{
			case 'i':
				rp[FREQ]   = p.wd;
				rp[SPRING] = p.kt_I;
				rp[AMPL]   = p.tho;
				rp[DAMP]   = p.c_I;
				rp[MASS]   = p.mg_I;
				break;
			case 'r':
				rp[FREQ]   = p.wd;
				rp[DAMP]   = p.q;
				rp[AMPL]   = p.g;
				rp[SPRING] = 999.0;
				rp[MASS]   = 999.0;
				break;
		}  /* end switch */

		/* define some other parameters...                                   */
		/* time step in sec. based on the forcing frequency...               */
		dt     = 2.000 * PI / p.pts_per_cycle / rp[FREQ];
		/* initial conditions...                                             */
		th_old = p.theta_0 * PI / 180.0;
		om_old = p.omeg_0;
		/* initial time...                                                   */
		t      = 0.0;

		/* display starting message to the user...                           */
		_settextposition( 2, 1 );
		_settextcolor( CYAN );
		sprintf( txt, "Initial Pendulum Position. Hit any key to Start." );
		_outtext( txt );

		/* show the initial pendulum position on the screen...               */
		if( sc.show_graphics )
			draw_pend( th_old, 0.0, anima, pend, p.pend_type );
		/* write the initial points to the response file if applicable...    */
		if( sc.save_flag )
			fprintf( rspptr, "%lf\t%lf\t%lf\n", t, th_old, om_old );

		/* wait for the user to hit any key...                               */
		dum_char = getch();

		/* erase the starting message from the screen...                     */
		_settextposition( 2, 1 );
		_settextcolor( BLACK );
		_outtext( txt );

		/* run the simulation...                                             */
		while( t <= p.tmax )
		{
			/* if the user hits any key, suspend the program and show menu... */
			if( kbhit() != 0 )
				prog_control( vc.adapter, &sc, rspptr, p );

			/* Here's where the differential equation is integrated...        */
			runge( th_old, om_old, &th_new, &om_new, t, dt, rp, p.pend_type );

			/* keep theta in the range -ã < é < ã...                          */
			if( fabs( th_new ) > PI )
				th_new -= 2.000 * PI * fabs( th_new ) / th_new;

			t += dt;

			/* animate the pendulum and show the phase portrait...            */
			if( sc.show_graphics )
			{
				draw_pend( th_new, th_old, anima, pend, p.pend_type );
				draw_phase( th_new, om_new, phs_pln, th_old, om_old, phas );
			}

			/* write the response points to a file...                         */
			if( sc.save_flag )
				fprintf( rspptr, "%lf\t%lf\t%lf\n", t, th_new, om_new );

			/* display the time every 100 times through the simulation...     */
			++cnt;
			if( cnt == 100 )
			{
				_settextposition( sc.time_row, sc.time_col );
				_settextcolor( YELLOW );
				sprintf( txt, "Time %.1lf", t );
				_outtext( txt );
				cnt = 0;
			}  /* end if */

			th_old = th_new;
			om_old = om_new;
		} /* end while */

		fclose( rspptr );
	} /* end else */
	/* tell the user that the time has run out and the simulation is done   */
	_settextposition( 3, 12 );
	_settextcolor( YELLOW );
	sprintf( txt, "Finished.  Hit any key." );
	_outtext( txt );
	getch();
	_settextposition( 3, 12 );
	_settextcolor( BLACK );
	_outtext( txt );
}  /* end calc_points */
