/**************************************************************************
*    MODULE:               GETBIFDT.C                                     *
*                                                                         *
*    CREATED BY:           ROY CHANCELLOR                                 *
*                                                                         *
*    DATE CREATED:         09-12-93                                       *
*                                                                         *
*    DATE LAST MODIFIED:   09-12-93                                       *
*                                                                         *
*    VERSION:              1.00                                           *
*                                                                         *
*    This module gets the bifurcation simulation data from the appropriate*
*    data file.                                                           *
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

void get_bifur_data( struct bif *bif, char ptype[] )
{
	static char path[_MAX_PATH + 20];
	char        direc[_MAX_PATH + 20], buf = 'C';
	FILE        *bifdat, *fopen();

	_getdcwd( buf - 'A' + 1, path, _MAX_PATH );
	strcpy( direc, path );
	if( !strcmp( ptype, "inverted" ) )
		strcat( direc, "\\invbifur.dat" );
	else if( !strcmp( ptype, "regular" ) )
		strcat( direc, "\\regbifur.dat" );

	if( (bifdat = fopen( direc, "r" )) != NULL )
	{
		fgets( bif -> bif_par, 99, bifdat );
		fgets( bif -> phase_var, 99, bifdat );
		fscanf( bifdat, "%lf", &bif -> delta_bp );
		fscanf( bifdat, "%lf", &bif -> bp_i );
		fscanf( bifdat, "%lf", &bif -> bp_f );

		fclose( bifdat );
	}
	else
	{
		_setvideomode( _DEFAULTMODE );
		printf( "Can't Open %s.  Hit any key to end.", direc );
		getch();
		exit( 11 );
	}
}/* end get_bifur_data */
