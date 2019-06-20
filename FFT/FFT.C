/***************************************************************************
*	Module:			FFT.C 	                                            		*
*                                                                         	*
*  Created By:		Roy Chancellor & Stephen Faw					              	*
*                                                                         	*
*  Date Created:  06-10-1993                                              	*
*  																							  	*
*	Last Modified:	10-24-1994															  	*
*                                                                         	*
*  Version:			1.01                                                    	*
*																								  	*
*	Date Released:	08-25-1993															  	*
*                                                                         	*
*	Description:	calculates the FFT of time series data contained in a		*
*						text file using the Sande-Tukey algorithm.  The pseudo-	*
*						code for the algorithm was taken from "Numerical Methods	*
*						for Engineers" by Chapra and Canale, McGraw-Hill, 1988,	*
*						2nd Ed., pp. 424.														*
*                                                                         	*
*	Inputs:			void																		*
*                                                                         	*
*	Returns:			1 if ok, 0 if anything fails				                 	*
*                                                                         	*
*	Date				Modification                              		Initials	*
* ----------	-------------------------------------------			--------	*
*	08-25-1993	Initial Release													RSC	*
*	10-24-1994	Revamped code to update style (ver. to 1.01)				RSC	*
****************************************************************************/
/* Include Files that came with Quick C...											*/
#include <stdio.h>
#include <math.h>
#include <string.h>
#include <stdlib.h>
#include <process.h>
#include <malloc.h>
#include <conio.h>
#include <time.h>

/* Include Files that were custom written for module...							*/
#include <royware.h>
#include "c:\royware\fft\source\fft.h"

#define TWOPI	2.000 * PI

int	str_from_file( char [], FILE * );

/***************************************************************************
Function:		main
Purpose:			to control the fft program
Arguments:		void
Returns:			1 if ok; 0 if anything fails
****************************************************************************/
int	main( void )
{
	struct 	fft_cfg fft;

/* Get all FFT configuration data...                              			*/
	if( !get_config( "c:\\fft\\fftsetup.cfg", &fft ) )
		return( FAIL );
/* Calculate the FFT...                                                    */
	if( !calc_fft( fft ) )
		return( FAIL );

	return( OK );
}  /* end main */

/***************************************************************************
Function:	get_config
Purpose:		to get the fft configuration data from a text file
Arguments:	file name, ptr to config structure
Returns:		1 if ok; 0 if anything fails
****************************************************************************/
int	get_config( char cfg_name[], struct fft_cfg *fft )
{
	char	txt[MAXSTR];
	FILE 	*fp, *fopen();

	if( (fp = fopen( cfg_name, "r" ) ) == NULL )
	{
		/* popup menu in a later version...												*/
		printf( "\n\n\tCan't open %s for %s.", cfg_name, "reading" );
		printf( "\n\n\tThe Reason:  %s", strerror( errno ) );
		printf( "\n\n\tPress any key to end..." );
		getch();
		return( FAIL );
	}  /* end if */

	str_from_file( fft -> time_file_path, fp );
	str_from_file( fft -> fft_file_path, fp );
	str_from_file( txt, fp );
	fft -> nocol = atoi( txt );
	str_from_file( txt, fp );
	fft -> fft_col = atoi( txt );
	str_from_file( txt, fp );
	fft -> num_time_pts = atoi( txt );
	str_from_file( txt, fp );
	fft -> dt = atof( txt );
	str_from_file( txt, fp );
	fft -> min_freq_file = atof( txt );
	str_from_file( txt, fp );
	fft -> max_freq_file = atof( txt );

	fclose( fp );
	return( OK );
}  /* end get_config */

/***************************************************************************
Function:	calc_fft
Purpose:		to perform the fft calculations on an array of time series data
Arguments:	fft structure
Returns:		1 if ok; 0 if anything failed
****************************************************************************/
int	calc_fft( struct fft_cfg fft )
{
	int      n1, n2, i, j, k, q, power_of_2;
	time_t   time_start, time_end;
	double   angle, argument, cs, sn, xt, yt, el_t;
	double   _huge *re, *im;
	double   amp, freq, phase, resolution, double_powof2;
	FILE     *infil, *outfil, *fopen();

/* open the FFT results file for write...                                  */
	i = 0;
/*	while( fft.fft_file_path[i++] != '\n' );
   fft.fft_file_path[i] = '\0';*/

	if ( (outfil = fopen( fft.fft_file_path, "w" )) == NULL )
	{
		/* popup menu in a later version...												*/
		printf( "\n\n\tCan't open %s for %s.", fft.fft_file_path, "reading" );
		printf( "\n\n\tThe Reason:  %s", strerror( errno ) );
		printf( "\n\n\tPress any key to end..." );
		getch();
		return( FAIL );
	}  /* end if */

/* open the time series data file for read...                              */
	i = 0;
	/*while( fft.time_file_path[i++] != '\n' )
	fft.time_file_path[i] = '\0';*/

	if ( (infil = fopen( fft.time_file_path, "r" )) == NULL )
	{
		/* popup menu in a later version...												*/
		printf( "\n\n\tCan't open %s for %s.", fft.time_file_path, "reading" );
		printf( "\n\n\tThe Reason:  %s", strerror( errno ) );
		printf( "\n\n\tPress any key to end..." );
		getch();
		return( FAIL );
	}  /* end if */

/* dynamically allocate memory for arrays re and im...                     */
	re = (double _huge *)halloc((size_t) fft.num_time_pts + 1,
																	(size_t)sizeof(double));
	im = (double _huge *)halloc((size_t) fft.num_time_pts + 1,
																	(size_t)sizeof(double));
	/* check to see if enough memory was available....                      */
	if( re == NULL || im == NULL )
	{
		printf( "\n\n\tDynamic memory allocation failed.  You will probably" );
		printf( "\n\thave to use fewer points and try again." );
		printf( "\n\n\tPress any key to continue..." );
		getch();
		return( FAIL);
	}  /* end if */

/* calculate the resolution of FFT points...                               */
	resolution = 1.0 / ( fft.num_time_pts * fft.dt );

/* Read in time series data from file...                                   */
	printf( "\n\n\tReading in Time Series Data........." );
	time_start = clock();
	if( !get_data( infil, re, im, fft ) )
		return( FAIL );
	time_end   = clock();
	el_t = (double)(time_end - time_start) / CLOCKS_PER_SEC;
	printf( "Finished!!...in %g Seconds!!", el_t );

/* FFT algorithm...                                                       */
	printf( "\n\n\tStarting FFT Calculations..........." );
	time_start    = clock();
	double_powof2 = log( (double)fft.num_time_pts ) / log( 2.0 );
	power_of_2    = (int)double_powof2;
	n2            = fft.num_time_pts;

	for( k = 1; k <= power_of_2; k++ )
	{
		n1       = n2;
		n2       = n2 / 2;
		angle    = 0.0;
		argument = TWOPI / (double)n1;
		for( j = 0; j <= n2 - 1; j++ )
		{
			cs = cos( angle );
			sn = -sin( angle );
			for( i = j; i <= fft.num_time_pts - 1; i += n1 )
			{
				q      = i + n2;
				xt     = re[i] - re[q];
				re[i] += re[q];
				yt     = im[i] - im[q];
				im[i] += im[q];
				re[q]  = xt * cs - yt * sn;
				im[q]  = yt * cs + xt * sn;
			}  /* end for */
			angle = ( j + 1 ) * argument;
		}  /* end for */
	}  /* end for */

/* end of FFT algorithm...                                                 */
	time_end = clock();
	el_t = (double)(time_end - time_start) / CLOCKS_PER_SEC;
	printf( "Finished!!...in %g Seconds!!", el_t );

/* unscramble coefficients...                                              */
	printf( "\n\n\tUnscrambling Coefficients..........." );
	time_start = clock();
	j = 0;
	for( i = 0; i <= (fft.num_time_pts - 2); i++ )
	{
		if( i < j )
		{
			xt    = re[j];
			re[j] = re[i];
			re[i] = xt;
			yt    = im[j];
			im[j] = im[i];
			im[i] = yt;
		}  /* end if */
		k = fft.num_time_pts / 2;
		while( k < (j + 1) )
		{
			j -= k;
			k /= 2;
		}
		j += k;
	}  /* end for */
	for ( i = 0; i <= ( fft.num_time_pts - 1 ); i++ )
	{
		re[i] /= fft.num_time_pts;
		im[i] /= fft.num_time_pts;
	}
/* end of coefficient unscrambling...                                      */
	time_end = clock();
	el_t = (double)(time_end - time_start) / CLOCKS_PER_SEC;
	printf( "Finished!!...in %g Seconds!!", el_t );

/* calculate freq and amp outputs and write to output file...   				*/
	printf( "\n\n\tWriting Spectral Data to a File....." );
	time_start = clock();
	freq = 0.0;
	for( i = 0; i < fft.num_time_pts / 2; i++ )
	{
		amp = sqrt( re[i] * re[i] + im[i] * im[i] );
		if( im[i] != 0.0 )
			phase = atan( re[i] / im[i] );  /* for C * sine( omega t + phi )	*/
		else
			phase = PI / 2.0;

		if( freq >= fft.min_freq_file )
			fprintf( outfil, "%.8lf  %.8lf  %.8lf\n", freq, amp, phase );
		freq += resolution;
		if( freq >= fft.max_freq_file )
			i = fft.num_time_pts / 2 + 1;	/* end the for loop...					*/
	}  /* end for */
	time_end = clock();
	el_t = (double)(time_end - time_start) / CLOCKS_PER_SEC;
	printf( "Finished!!...in %g Seconds!!", el_t );

/* close all files...                                                      */
	fcloseall();
	printf( "\n\n\tPress Any Key to End..." );
	getch();
	return( OK );
} /* end calc_fft */

/***************************************************************************
Function:	get_data
Purpose:		to get time series data from text file
Arguments:	FILE pointer, real array, imag array, fft config structure
Returns:		1 if ok; 0 if anything failed
****************************************************************************/
int	get_data( FILE *dp, double re[], double im[], struct fft_cfg fft )
{
	int      i, j;
	double   xt, dum[4];

	for( i = 0; i < fft.num_time_pts; ++i )
	{
		for( j = 0; j < fft.nocol; ++j )
			fscanf( dp, "%lf", &dum[j] );
		re[i] = dum[fft.fft_col - 1];
	}  /* end for */

	return( OK );
}  /* end get_data */

/***************************************************************************
Function:   str_from_file
Purpose:		to get a string in the form 'text=string to be gotten'
Arguments:	string to get, open file pointer
Returns:		1 if all ok; 0 if anything failed.
****************************************************************************/
int	str_from_file( char string[], FILE *fp )
{
	int	i, j;
	char	txt[MAXSTR];

	i = 0;
	j = 0;
	fgets( txt, MAXSTR - 1, fp );
	while( txt[i++] != '=' );
	while( txt[i] != '\n' )
		string[j++] = txt[i++];
	string[j] = '\0';

	return( OK );
}  /* end str_from_file */
