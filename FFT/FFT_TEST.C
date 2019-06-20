/***************************************************************************
*	Module:			FFT_TEST.C 	                                            	*
*                                                                         	*
*  Created By:		Roy Chancellor										              	*
*                                                                         	*
*  Date Created:  06-08-1993                                              	*
*  																							  	*
*	Last Modified:	08-10-1994															  	*
*                                                                         	*
*  Version:			1.00                                                    	*
*																								  	*
*	Date Released:	UNRELEASED															  	*
*                                                                         	*
*	Description:	Creates a single column of data which represents the		*
*						discrete time series of the function							*
*					Y=A1úsin(f1út+í1) + A2úsin(f2út+í2) + úúú + Anúsin(fnút+ín)	*
*						where n <= 10.  The file name defaults to "time_ser.dat"	*
*						and is put in the "c:\fft" directory in ASCII format.		*
*                                                                         	*
*	Inputs:			the drive and file name to put data file.		         	*
*                                                                         	*
*	Returns:			1 if all is OK, or 0 if anything fails.	             	*
*                                                                         	*
*	Date				Modification                              Initials      	*
* ----------	-------------------------------------------	---------	  	*
****************************************************************************/
/* Include Files that came with Quick C...											*/
#include <stdio.h>
#include <math.h>
#include <process.h>
#include <conio.h>
#include <stdlib.h>
#include <graph.h>
#include <string.h>

/* Include Files that were custom written for module...							*/
#include <royware.h>

/* Definitions...																				*/
#define MAX_NUM 10

/* Function prototypes...																	*/
int  get_y_info( double [], double [], double [] );
void max_array( double [], double * );

/* GLOBAL varaiables...																		*/
/* N/A																							*/

/***************************************************************************
Function:	main
Purpose:		gets input from user and makes a data file with data for FFT
Arguments:	the data file name
Returns:		1 if all OK; 0 if anything fails.
****************************************************************************/
void main( int argc, char *argv[] )
{
	double 	Y, t, dt, dummy1, dummy2, max_freq;
	double 	amp[MAX_NUM], phase[MAX_NUM], freq[MAX_NUM + 1];
	long int	i, np;
	int    	num_sines, j, pct_comp;
	char		fname[MAXLEN], errmes[MAXLEN], mes[20];
	static 	char default_data_name[] = {"d:\\time_ser.dat"};
	struct	rccoord pos;
	FILE   	*tsf, *fopen();

	/* reset the screen...																	*/
	_clearscreen( _GCLEARSCREEN );

	/* decide if the user entered a custom data file name from the command
		line.  Open the appropriate file for writing...								*/
	if( argc > 1 )		/* then there was at least one command line argument	*/
		strcpy( fname, argv[1] );
	else
		strcpy( fname, default_data_name );

	if ( (tsf = fopen( fname, "w+" )) == NULL )
	{
		sprintf( errmes, "Can't open %s:  ", fname );
		perror( errmes );
		printf( "\n\n Enter the file name as \"x:\\direc\\fname.ext\" " );
		printf( "\n Press any key to end." );
		getch();
		exit( FAIL );
   }  /* end if */

	/* get the following from the user:  the number of sine waves in the
		time series, each amplitude, each frequency, and each phase angle...	*/
	num_sines = get_y_info( amp, freq, phase );

	/* find the maximum frequency in those entered by the user...				*/
	max_array( freq, &max_freq );

	/* get the FFT parameters from the user...										*/
	_clearscreen( _GCLEARSCREEN );
	_settextposition( 10, 2 );
	_outtext( "Enter the \"sample\" period in seconds ====>  " );
	scanf( "%lf", &dt );

	/* get the number of points to make for the FFT (must be a power of 2)	*/
	np = 16385;
	dummy1 = log( np ) / log( 2.0 );
	dummy2 = (int)dummy1;
	while( fmod( dummy1, dummy2 ) != 0.0 )
	{
		_settextposition( 12, 2 );
		_outtext( "How many points? (a power of 2 <= 32768) ===>   " );
		scanf( "%ld", &np );
		dummy1 = log( np ) / log( 2.0 );
		dummy2 = (int)dummy1;
	}  /* end while */

	_clearscreen( _GCLEARSCREEN );
	_settextposition( 10, 5 );
	sprintf( mes, "Writing Data to %s....", fname );
	_outtext( mes );
	pos = _gettextposition();
	_settextposition( pos.row, pos.col + 4 );
	_outtext( "Percent Complete" );
	pct_comp = 0;

	for ( i = 0; i < np; i++ )
	{
		t = i * dt;
		Y = 0;
		for( j = 0; j < num_sines; ++j )
			Y += amp[j] * sin( freq[j] * t + phase[j] );
		fprintf( tsf, "%lf\n", Y );
		if( i % (np / 10) == 0 )
		{
			_settextposition( pos.row, pos.col );
			printf( "%d", pct_comp );
			pct_comp += 10;
		}
   }  /* end for */

	fclose( tsf );
	printf( "\n\n Hit Any Key to End." );
	getch();
}  /* end main */

/***************************************************************************
Function:	get_y_info
Purpose:		to get the amplitude, frequencies, and phase angles of the sine
				waves in the time series function.
Arguments:	amplitude, frequency, phase
Returns:		number of sine waves
****************************************************************************/
int get_y_info( double amp[], double freq[], double phase[] )
{
	int valid = NO, n, i;

	_clearscreen( _GCLEARSCREEN );

	while( !valid )
	{
		printf( "\n\n How Many Sine Waves in The Response ? ( ó%d ):\t",
																						MAX_NUM );
		scanf( "%d", &n );
		if( n <= MAX_NUM && n > 0 )
			valid = YES;
		else
			printf( "\n\n The number of sine waves must satisfy 0 ó n ó %d",
																						MAX_NUM );
	}  /* end while */

	for( i = 0; i < n; ++i )
	{
		printf( "\n Sine Wave #%d", i + 1 );
		printf( "\n\tAmplitude(%d) =\t\t", i + 1 );
		scanf( "%lf", &amp[i] );
		printf( "\n\tFrequency(%d), Hz =\t", i + 1 );
		scanf( "%lf", &freq[i] );
		freq[i] *= 2.0 * PI;
		printf( "\n\tPhase(%d), degrees =\t", i + 1 );
		scanf( "%lf", &phase[i] );
		phase[i] *= PI / 180.0;
		printf( "\n" );
	}  /* end for */
	/* append 999.0 to the end of the frequency array for later use...		*/
	freq[i] = 999.0;

	return( n );
}  /* end get_y_info */

/***************************************************************************
Function:	max_array
Purpose:		finds the highest frequency of the sine waves entered by the
				user.  The 999.0 is appended to the frequency array in the
				previous function.
Arguments:	frequency array, ptr to max frequency
Returns:		void
****************************************************************************/
void max_array( double arr[], double *maxval )
{
	int    i, end_location;

	i = 0;
	while( arr[i] != 999.0 )
		++i;
	end_location = i;
	for( i = 1; i < end_location; ++i )
	{
		if( arr[i] > arr[i - 1] )
			*maxval = arr[i];
	}
}  /* end max_array */
