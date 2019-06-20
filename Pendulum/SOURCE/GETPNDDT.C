/**************************************************************************
*    MODULE:               GETPNDDT.C                                     *
*                                                                         *
*    CREATED BY:           ROY CHANCELLOR                                 *
*                                                                         *
*    DATE CREATED:         09-12-93                                       *
*                                                                         *
*    DATE LAST MODIFIED:   09-12-93                                       *
*                                                                         *
*    VERSION:              1.00                                           *
*                                                                         *
*    This module reads in the pendulum parameter data from the appropriate*
*    file based on the simtype string passed to it.                       *
*                                                                         *
**************************************************************************/
#include <stdio.h>
#include <direct.h>
#include <conio.h>
#include <stdlib.h>
#include <process.h>
#include <string.h>
#include <graph.h>
#include <royware.h>
#include "c:\roy\quickc\pend\pendsim.h"

void get_pend_data( struct pend *p, char simtype[] )
{
	static char path[_MAX_PATH + 20];
	char        direc[_MAX_PATH + 20], buf = 'C';
	FILE        *pen_dat, *fopen();

	if( !strcmp( simtype, "regular" ) )
	{
		p -> pend_type = 'r';
		strcpy( p -> sim_fname, "\\pendat.dat" );
	}
	else if( !strcmp( simtype, "inverted" ) )
	{
		p -> pend_type = 'i';
		strcpy( p -> sim_fname, "\\inverted.dat" );
	}

	_getdcwd( buf - 'A' + 1, path, _MAX_PATH );
	strcpy( direc, path );
	strcat( direc, p -> sim_fname );

	switch( p -> pend_type )
	{
		case 'r':
			if( (pen_dat = fopen( direc, "r" )) != NULL )
			{
				fscanf( pen_dat, "%d",  &p -> pts_per_cycle );
				fscanf( pen_dat, "%lf", &p -> poin_phase );
				fscanf( pen_dat, "%lf", &p -> wd );
				fscanf( pen_dat, "%lf", &p -> theta_0 );
				fscanf( pen_dat, "%lf", &p -> omeg_0 );
				fscanf( pen_dat, "%lf", &p -> g );
				fscanf( pen_dat, "%lf", &p -> q );
				fscanf( pen_dat, "%lf", &p -> tmin );
				fscanf( pen_dat, "%lf", &p -> tmax );

				fclose( pen_dat );
			}  /* end if */
			else
			{
				_setvideomode( _DEFAULTMODE );
				printf( "Can't Open %s.  Hit any key to end.", direc );
				getch();
				exit( 11 );
			}
			break;
		case 'i':
			if( (pen_dat = fopen( direc, "r" )) != NULL )
			{
				fscanf( pen_dat, "%d",  &p -> pts_per_cycle );
				fscanf( pen_dat, "%lf", &p -> poin_phase );
				fscanf( pen_dat, "%lf", &p -> wd );
				fscanf( pen_dat, "%lf", &p -> theta_0 );
				fscanf( pen_dat, "%lf", &p -> omeg_0 );
				fscanf( pen_dat, "%lf", &p -> tmin );
				fscanf( pen_dat, "%lf", &p -> tmax );
				fscanf( pen_dat, "%lf", &p -> c_I );
				fscanf( pen_dat, "%lf", &p -> kt_I );
				fscanf( pen_dat, "%lf", &p -> mg_I );
				fscanf( pen_dat, "%lf", &p -> tho );

				fclose( pen_dat );
			}  /* end if */
			else
			{
				_setvideomode( _DEFAULTMODE );
				printf( "Can't open %s.  Hit any key to end.", direc );
				getch();
				exit( 11 );
			}
			break;
	}  /* end switch */
}  /* end get_pend_data */
