/**************************************************************************
*    MODULE:                  FFT_PHAS.C                                  *
*                                                                         *
*    CREATED BY:              ROY CHANCELLOR                              *
*                                                                         *
*    DATE VERSION CREATED:    06-11-93                                    *
*                                                                         *
*    DATE LAST MODIFIED:      08-27-93                                    *
*                                                                         *
*    VERSION:                 1.00                                        *
*                                                                         *
*    DESCRIPTION:                                                         *
*                                                                         *
*    This program plots FFT Phase data from a user input file input in    *
*    the FFT.EXE program.                                                 *
*                                                                         *
*    CHANGES:                                                             *
*                                                                         *
**************************************************************************/

#include <stdio.h>
#include <conio.h>
#include <process.h>
#include <stdlib.h>
#include <graph.h>
#include <string.h>
#include <direct.h>
#include <math.h>
#include <royware.h>

void get_and_plot( void );
void get_max_min_time( double *tmin, double *tmax,
						double *ymin, double *ymax, char xtype[], char ytype[] );
void decide_and_chg( double *var1, double *var2, int xax, int yax );
void display_message( int which );

FILE *datfil, *fopen();

/***************************************************************************
***************************************************************************/

void main( void )
{
	get_and_plot();
}

/***************************************************************************
***************************************************************************/

void get_and_plot( void )
{
	int which, connect = 'c', resp_type, yaxis, time_flag = 1, pts;
	int invalid, which_drive, loglin = 'n', xax = LIN, yax = LIN, stop_prog;
	int cnt;
	double t_old, t_new, phs_old, phs_new, ymin, ymax;
	double tmin, tmax, dt, samp_freq, force_freq, temp;
	char filename[100], txt[100], dum[MAXSTR], xtype[MAXSTR], ytype[MAXSTR];
	static char path[_MAX_PATH + 20];
	static char default_name[] = {"fft_out.dat"};
	char direc[_MAX_PATH + 20];
	char buf;

	invalid = TRUE;
	while( invalid )
	{
		_clearscreen( _GCLEARSCREEN );
		_settextcolor( YELLOW );
		_settextposition( 10, 19 );
		sprintf( txt, "Press The Drive Letter Of The Data: (A//B//C//D)" );
		_outtext( txt );
		which_drive = getch();
		switch( which_drive )
		{
			case 'a':case 'A':
				system( "a:" );
				buf = 'A';
				invalid = FALSE;
				break;
			case 'b':case 'B':
				system( "b:" );
				buf = 'B';
				invalid = FALSE;
				break;
			case 'c':case 'C':
				system( "c:" );
				buf = 'C';
				system( "cd\\" );
				invalid = FALSE;
				break;
			case 'd':case 'D':
				system( "d:" );
				buf = 'D';
				system( "cd\\" );
				invalid = FALSE;
				break;
			default:
				break;
		}  /* end switch */
	}  /* end while */

	_getdcwd( buf - 'A' + 1, path, _MAX_PATH );
	strcpy( direc, path );

	_clearscreen( _GCLEARSCREEN );
	_settextcolor( YELLOW );
	switch( which_drive )
	{
		case 'c':case 'C':case 'd':case 'D':
			_settextposition( 10, 4 );
			sprintf( txt, "Enter Filename (Must Be In %s)---%s",
																path, default_name );
			_outtext( txt );
			_settextcolor( LIGHT_RED );
			_settextposition( 12, 4 );
			sprintf( txt, "Hit Enter to Select the Default or Enter Filename" );
			_outtext( txt );
			_settextposition( 10, 38 );
			_settextcolor( YELLOW );
/*         gets( filename );*/
			gets( filename );
			_settextcolor( YELLOW );
/*        strcat( direc, "\\" );*/
			break;
		case 'a':case 'A':case 'b':case 'B':
			_settextposition( 10, 28 );
			sprintf( txt, "Enter Filename---" );
			_outtext( txt );
			gets( filename );
			break;
		default:
			break;
	}
	if( !strcmp( filename, "\0" ) )
	{
		strcat( direc, "\\" );
		strcat( direc, default_name );
	}
	else
	{
		strcat( direc, "\\" );
		strcat( direc, filename );
	}

	resp_type = 't';
	if( resp_type == 't' || resp_type == 'T' )
		yaxis = '2';

	_clearscreen( _GCLEARSCREEN );
	_settextposition( 10, 20 );
	sprintf( txt, "[C]--Connect Points With a Line  [D]--Dots" );
	_outtext( txt );
	connect = getch();

	cnt = 0;
	stop_prog = YES;
	while( stop_prog )
	{
		stop_prog = NO;
		if( (datfil = fopen( direc, "r" )) == NULL )
		{
			_clearscreen( _GCLEARSCREEN );
			printf( "\n\n %s Was Not In The Current Directory.\n Hit Enter. ",
																						filename );
			getch();
			_setvideomode( _DEFAULTMODE );
			exit( 10 );
		}
		else
		{
			if( cnt == 0 )
				setup_graph( "c:\\fft\\fft_phas.dat" );
			cnt = 1;
			_setcolor( YELLOW );
			get_max_min_time( &tmin, &tmax, &ymin, &ymax, xtype, ytype );

			if( !strcmp( xtype, "lin\n" ) )
				xax = LIN;
			if( !strcmp( xtype, "log\n" ) )
				xax = LOG;
			if( !strcmp( ytype, "lin\n" ) )
				yax = LIN;
			if( !strcmp( ytype, "log\n" ) )
				yax = LOG;

			decide_and_chg( &tmin, &ymin, xax, yax );
			decide_and_chg( &tmax, &ymax, xax, yax );

			fscanf( datfil, "%lf", &t_old );          /* get the first points */
			fscanf( datfil, "%lf", &phs_old );
			fscanf( datfil, "%lf", &phs_old );
			decide_and_chg( &t_old, &phs_old, xax, yax );

			switch( yaxis )
			{
				case '2':
					if( t_old <= tmax && t_old >= tmin && phs_old <= ymax &&
																				phs_old >= ymin )
						_setpixel_w( t_old, phs_old );
					break;
				default:
					break;
			}

			fscanf( datfil, "%lf", &t_new );
			fscanf( datfil, "%lf", &phs_new );

			while( fscanf( datfil, "%lf", &phs_new ) != EOF )
			{
				decide_and_chg( &t_new, &phs_new, xax, yax );
				if( kbhit() != 0 )
				{
					display_message( 0 );
					getch();
					which = getch();
					switch( which )
					{
						case 'e':case 'E':
							fclose( datfil );
							_setvideomode( _DEFAULTMODE );
							system( "c:" );
							system( "cd\\fft" );
							exit( 10 );
							break;
						case 'g':case 'G':
							_setvideomode( _DEFAULTMODE );
							grpeditr( "c:\\fft\\fft_phas.dat" );
							fclose( datfil );
							setup_graph( "c:\\fft\\fft_phas.dat" );
							stop_prog = YES;
							break;
						default:
							display_message( 1 );
							break;
					}  /* end switch */
				}  /* end if */

				if( t_new >= tmin && t_new <= tmax && t_old >= tmin )
				{
					switch( yaxis )
					{
						case '2':
							if( phs_new >= ymin && phs_new <= ymax &&
													phs_old >= ymin && phs_old <= ymax )
							{
								if( connect == 'd' || connect == 'D' )
									_setpixel_w( t_new, phs_new );
								else
									if( connect == 'c' || connect == 'C' )
									{
										_moveto_w( t_old, phs_old );
										_lineto_w( t_new, phs_new );
										phs_old = phs_new;
									}  /* end if */
									t_old = t_new;
							}  /* end if */
							else
								phs_old = phs_new;
							break;
						default:
							break;
					}  /* end switch */
				}  /* end if */
				else
				{
					t_old = t_new;
					phs_old = phs_new;
				}
				fscanf( datfil, "%lf", &t_new );
				fscanf( datfil, "%lf", &phs_new );
			}  /* end while */
		}  /* end else */
	}  /* end while */

	_settextcolor( YELLOW );
	_settextposition( 3, 14 );
	sprintf( txt, "[P]--Plot In Pizazz  [ANY KEY]--End  [L]--Load Pizazz" );
	_outtext( txt );
	which = getch();
	_settextcolor( BLACK );
	_settextposition( 3, 14 );
	_outtext( txt );

	if( which == 'p' || which == 'P' )
		getch();

	if( which == 'l' || which == 'L' )
	{
		system( "pzp.bat");
		getch();
	}

	fclose( datfil );
	_setvideomode( _DEFAULTMODE );
	system( "c:" );
	system( "cd\\fft" );
	exit( 9 );

}  /* end get_and_plot */

/***************************************************************************
***************************************************************************/

void display_message( int which )
{
	char txt[60];
	static char line1[] = {"[ANY]--Continue   [G]--Change Graph   [E]--End"};

	switch( which )
	{
		case 0:
		{
			_settextcolor( YELLOW );
			break;
		}
		case 1:
		{
			_settextcolor( BLACK );
			break;
		}
	}
	_settextposition( 3, 23 );
	sprintf( txt, line1 );
	_outtext( txt );
}  /* end display_message */

/***************************************************************************
***************************************************************************/

void get_max_min_time( double *tmin, double *tmax,
							double *ymin, double *ymax, char xtype[], char ytype[] )
{
	static char fft_nam[] = {"c:\\fft\\fft_phas.dat"};
	char title[100], yaxis[100], xaxis[100], comments[100], xax[100], yax[100];
	double low_t, high_t, low_y, high_y;
	int dum2;
	FILE *fftdat, *fopen();

	if( (fftdat = fopen( fft_nam, "r" )) != NULL )
	{
		fgets( title, MAXSTR - 1, fftdat );
		fgets( yaxis, MAXSTR - 1, fftdat );
		fgets( xaxis, MAXSTR - 1, fftdat );
		fgets( comments, MAXSTR - 1, fftdat );
		fscanf( fftdat, "%lf", &low_t );
		fscanf( fftdat, "%lf", &high_t );
		fscanf( fftdat, "%lf", &low_y );
		fscanf( fftdat, "%lf", &high_y );
		fscanf( fftdat, "%d", &dum2 );
		fscanf( fftdat, "%d", &dum2 );
		fscanf( fftdat, "%d", &dum2 );
		fscanf( fftdat, "%d", &dum2 );
		fscanf( fftdat, "%d", &dum2 );
		/* get the extra '\n' not read by previous fscanf...                 */
		fgets( xax, MAXSTR - 1, fftdat );
		fgets( xax, MAXSTR - 1, fftdat );
		fgets( yax, MAXSTR - 1, fftdat );

		fclose( fftdat );

		*tmin = low_t;
		*tmax = high_t;
		*ymin = low_y;
		*ymax = high_y;
		strcpy( xtype, xax );
		strcpy( ytype, yax );
	}  /* end if */
	else
	{
		printf( "\n\n CAN'T OPEN %s", fft_nam );
		getch();
		exit( 11 );
	}
}  /* end get_max_min_time */

/***************************************************************************
***************************************************************************/

void decide_and_chg( double *var1, double *var2, int xax, int yax )
{
	if( xax == LIN && yax == LOG && *var2 != 0 )
		*var2 = log10( *var2 );
	if( xax == LOG && yax == LIN && *var1 != 0 )
		*var1 = log10( *var1 );
	if( xax == LOG && yax == LOG && *var1 != 0 && *var2 != 0 )
	{
		*var1 = log10( *var1 );
		*var2 = log10( *var2 );
	}
}  /* end decide_and_chg */
