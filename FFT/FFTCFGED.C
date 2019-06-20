/**************************************************************************
*    MODULE:                  FFTCFGED.C                                  *
*                                                                         *
*    CREATED BY:              ROY CHANCELLOR                              *
*                                                                         *
*    DATE VERSION CREATED:    08-25-93                                    *
*                                                                         *
*    DATE LAST MODIFIED:      08-26-93                                    *
*                                                                         *
*    VERSION:                 1.00                                        *
*                                                                         *
*    DESCRIPTION:                                                         *
*                                                                         *
**************************************************************************/

#include <stdio.h>
#include <conio.h>
#include <process.h>
#include <stdlib.h>
#include <graph.h>
#include <string.h>
#include <royware.h>
#include "c:\royware\fft\source\fft.h"

void ffteditr( char [] );
void main( void )
{
	ffteditr( "c:\\fft\\fftsetup.cfg" );
}  /* end main */

#define NUM_VALUES      12
#define NUM_TEXT_LABELS 17
#define NUM_TITLES       6

/* Function Prototypes...                                                  */
void setup_graphics( void );
void setup_edit_screen( void );
void get_file_info( char [], struct fft_cfg * );
void initialize_vals( struct text_loc [] );
void put_info_on_screen( struct fft_cfg, struct text_loc [] );
void edit_away( struct fft_cfg *, struct text_loc [] );
void blanks_to_line( int, int, int );
void write_file_info( char [], struct fft_cfg );

void ffteditr( char filename[] )
{
	struct text_loc   vals[NUM_VALUES];
	struct fft_cfg    info;

	setup_graphics();
	setup_edit_screen();
	get_file_info( filename, &info );
	initialize_vals( vals );
	put_info_on_screen( info, vals );
	edit_away( &info, vals );
	write_file_info( filename, info );
	_setvideomode( _DEFAULTMODE );
}  /* end ffteditr */

/***************************************************************************/

void setup_graphics( void )
{
	struct videoconfig monitor;

	_getvideoconfig( &monitor );
	_settextrows( _MAXTEXTROWS );
	_settextcursor( DOUBLE_UNDERLINE );
	_displaycursor( _GCURSORON );
	_clearscreen( _GCLEARSCREEN );
}  /* end setup_graphics */

/***************************************************************************/

void setup_edit_screen( void )
{
	struct text_loc edtr[NUM_TEXT_LABELS];
	int    i;
	char   text[MAXSTR];

	strcpy( edtr[0].txt, "RoyWare Very Specific Text Editor\0" );
	edtr[0].row = 2;
	edtr[0].col = 24;
	edtr[0].color   = CYAN;
	strcpy( edtr[1].txt, "Time Series File:\0" );
	edtr[1].row = 4;
	edtr[1].col = 4;
	edtr[1].color   = YELLOW;
	strcpy( edtr[2].txt, "Drive:  ( )  File Name:  (            )\0" );
	edtr[2].row = 6;
	edtr[2].col = 7;
	edtr[2].color   = LIGHT_RED;
	strcpy( edtr[3].txt, "Directory:\0" );
	edtr[3].row = 8;
	edtr[3].col = 7;
	edtr[3].color   = LIGHT_RED;
	strcpy( edtr[4].txt, "FFT Output File:\0" );
	edtr[4].row = 10;
	edtr[4].col = 4;
	edtr[4].color   = YELLOW;
	strcpy( edtr[5].txt, "Drive:  ( )  File Name:  (            )\0" );
	edtr[5].row = 12;
	edtr[5].col = 7;
	edtr[5].color   = LIGHT_RED;
	strcpy( edtr[6].txt, "Directory:\0" );
	edtr[6].row = 14;
	edtr[6].col = 7;
	edtr[6].color   = LIGHT_RED;
	strcpy( edtr[7].txt, "Sample Frequency, Hz:\0" );
	edtr[7].row = 16;
	edtr[7].col = 4;
	edtr[7].color   = YELLOW;
	strcpy( edtr[8].txt, "MINimum Frequency in FFT Output File:\0" );
	edtr[8].row = 18;
	edtr[8].col = 4;
	edtr[8].color   = YELLOW;
	strcpy( edtr[9].txt, "MAXimum Frequency in FFT Output File:\0" );
	edtr[9].row = 20;
	edtr[9].col = 4;
	edtr[9].color   = YELLOW;
	strcpy( edtr[10].txt, "# Col in Time File:  ( )  Take FFT of Column:  ( )\0" );
	edtr[10].row = 22;
	edtr[10].col = 4;
	edtr[10].color   = YELLOW;
	strcpy( edtr[11].txt, "Number of Points in Time Series File:  (     )\0" );
	edtr[11].row = 24;
	edtr[11].col = 4;
	edtr[11].color   = YELLOW;
	strcpy( edtr[12].txt, "Use [- -] to Move LEFT/RIGHT\0"  );
	edtr[12].row = 30;
	edtr[12].col = 19;
	edtr[12].color   = CYAN;
	strcpy( edtr[13].txt, "Use [- -] to Move UP/DOWN\0" );
	edtr[13].row = 31;
	edtr[13].col = 19;
	edtr[13].color   = CYAN;
	strcpy( edtr[14].txt, "Use [TAB] to Edit Text\0" );
	edtr[14].row = 32;
	edtr[14].col = 19;
	edtr[14].color   = CYAN;
	strcpy( edtr[15].txt, "Use [BKSP] to Toggle Values\0" );
	edtr[15].row = 33;
	edtr[15].col = 19;
	edtr[15].color   = CYAN;
	strcpy( edtr[16].txt, "Press [ENTER] When Finished\0" );
	edtr[16].row = 34;
	edtr[16].col = 19;
	edtr[16].color   = CYAN;

	for( i = 0; i < NUM_TEXT_LABELS; ++i )
	{
		_settextcolor( edtr[i].color );
		_settextposition( edtr[i].row, edtr[i].col );
		sprintf( text, "%s", edtr[i].txt );
		_outtext( text );
		switch( i )
		{
			case 12:
				_settextcolor( YELLOW );
				sprintf( text, "%c %c", 27, 26 );
				_settextposition( edtr[i].row, edtr[i].col + 5 );
				_outtext( text );
				break;
			case 13:
				_settextcolor( YELLOW );
				sprintf( text, "%c %c", 24, 25 );
				_settextposition( edtr[i].row, edtr[i].col + 5 );
				_outtext( text );
				break;
			default:
				break;
		}  /* end switch */
	}  /* end for */
}  /* end setup_edit_screen */

/***************************************************************************/

void get_file_info( char screen_nam[], struct fft_cfg *info )
{
	FILE *sf, *fopen();

	if( (sf = fopen( screen_nam, "r" )) != NULL )
	{
		fgets( info -> time_file_path, MAXSTR - 1, sf );
		fgets( info -> fft_file_path, MAXSTR - 1, sf );
		fscanf( sf, "%d", &info -> nocol );
		fscanf( sf, "%d", &info -> fft_col );
		fscanf( sf, "%d", &info -> num_time_pts );
		fscanf( sf, "%lf", &info -> dt );
		fscanf( sf, "%lf", &info -> min_freq_file );
		fscanf( sf, "%lf", &info -> max_freq_file );
		fgets( info -> fft_drv, MAXSTR - 1, sf );  /* dummy read to get '\n'  */
		fgets( info -> fft_drv, MAXSTR - 1, sf );
		fgets( info -> fft_direct, MAXSTR - 1, sf );
		fgets( info -> fft_fname, MAXSTR - 1, sf );
		fgets( info -> time_drv, MAXSTR - 1, sf );
		fgets( info -> time_direct, MAXSTR - 1, sf );
		fgets( info -> time_fname, MAXSTR - 1, sf );

		fcloseall();
		flushall();
	}  /* end if */
	else
	{
		_setvideomode( _DEFAULTMODE );
		printf( "\n\n\n\n\n\n\n" );
		printf( "%s Can Not Be Opened.  Check To See If It Is There.",
																					screen_nam );
		getch();
		exit( 9 );
	}  /* end else */
}  /* end get_file_info */

/***************************************************************************/

void initialize_vals( struct text_loc vals[] )
{
	int i;

	for( i = 0; i < NUM_VALUES; ++i )
	{
		strcpy( vals[i].txt, "initialized\n\0" );
		vals[i].row   = 0;
		vals[i].col   = 0;
		vals[i].val.d = 0.0;
		vals[i].val.i = 0;
		vals[i].frmt  = "formatted";
	}  /* end for */
}  /* end initialize_vals */

/***************************************************************************/

void put_info_on_screen( struct fft_cfg info, struct text_loc vals[] )
{
	char   text[MAXSTR];
	int    i;

	/* Here are the TITLES that go on the screen...                         */
	strcpy( vals[0].txt, info.time_drv );
	vals[0].row = 6;
	vals[0].col = 16;
	vals[0].frmt    = "%s";
	strcpy( vals[1].txt, info.time_fname );
	vals[1].row = 6;
	vals[1].col = 33;
	strcpy( vals[2].txt, info.time_direct );
	vals[2].row = 8;
	vals[2].col = 19;
	strcpy( vals[3].txt, info.fft_drv );
	vals[3].row = 12;
	vals[3].col = 16;
	vals[3].frmt    = "%s";
	strcpy( vals[4].txt, info.fft_fname );
	vals[4].row = 12;
	vals[4].col = 33;
	strcpy( vals[5].txt, info.fft_direct );
	vals[5].row = 14;
	vals[5].col = 19;
	/* Here are the VALUES that go on the screen...                         */
	vals[6].val.d   = (double)(1.00000000 / info.dt);
	vals[6].row = 16;
	vals[6].col = 27;
	vals[6].frmt    = "%.5lf";
	vals[7].val.d   = info.min_freq_file;
	vals[7].row = 18;
	vals[7].col = 43;
	vals[7].frmt    = "%.5lf";
	vals[8].val.d   = info.max_freq_file;
	vals[8].row = 20;
	vals[8].col = 43;
	vals[8].frmt    = "%.5lf";
	vals[9].val.i   = info.nocol;
	vals[9].row = 22;
	vals[9].col = 26;
	vals[9].frmt    = "%d";
	vals[10].val.i  = info.fft_col;
	vals[10].row = 22;
	vals[10].col = 52;
	vals[10].frmt    = "%d";
	vals[11].val.i   = info.num_time_pts;
	vals[11].row = 24;
	vals[11].col = 44;
	vals[11].frmt    = "%d";

	_settextcolor( YELLOW );
	_setbkcolor( DARK_BLUE );
	for( i = 0; i < NUM_TITLES; ++i )
	{
		_settextposition( vals[i].row, vals[i].col );
		sprintf( text, "%s", vals[i].txt );
		_outtext( text );
	}
	for( i = NUM_TITLES; i < NUM_VALUES; ++i )
	{
		_settextposition( vals[i].row, vals[i].col );
		if( !strcmp( vals[i].frmt, "%.5lf" ) )
			sprintf( text, vals[i].frmt, vals[i].val.d );
		else if( !strcmp( vals[i].frmt, "%d" ) )
			sprintf( text, vals[i].frmt, vals[i].val.i );
		else if( !strcmp( vals[i].frmt, "%s" ) )
			sprintf( text, vals[i].frmt, vals[i].txt );
		_outtext( text );
	}
}  /* end put_info_on_screen */

/***************************************************************************/

void edit_away( struct fft_cfg *info, struct text_loc vals[] )
{
	char   ch, text[MAXSTR];
	int    ppi, oldcolor, i, j, blanks, col_max = vals[9].val.i;
	long   oldbkcolor;
	double temp_val1, temp_val2;
	struct rccoord curpos, newpos, oldpos;
	struct text_loc str[NUM_TITLES], tstr[NUM_TITLES];

	for( i = 0; i < NUM_TITLES; ++i )
		strcpy( str[i].txt, "initialized\0" );
	for( j = 0; j < NUM_TITLES; ++j )
		strcpy( str[j].txt, vals[j].txt );
	ppi = 0;
	ch = 'g';
	_settextcolor( YELLOW );
	_setbkcolor( DARK_BLUE );
	newpos.row = vals[ppi].row;
	newpos.col = vals[ppi].col;
	oldpos = _settextposition( newpos.row, newpos.col );
	while( ch != CR )
	{
		ch = (char)getch();
		switch( ch )
		{
			case EXTENDED_KEY:
				/* then get the other half...                                  */
				ch = (char)getch();
				switch( ch )
				{
					case LEFT_ARROW:
						if( ppi > 0 )
						{
							--ppi;
							_settextposition( vals[ppi].row, vals[ppi].col );
						}
						break;
					case RIGHT_ARROW:
						if( ppi < NUM_VALUES - 1 )
						{
							++ppi;
							_settextposition( vals[ppi].row, vals[ppi].col );
						}
						break;
					case DOWN_ARROW:
						if( ppi < NUM_VALUES - 1 )
						{
							++ppi;
							_settextposition( vals[ppi].row, vals[ppi].col );
						}
						break;
					case UP_ARROW:
						if( ppi > 0 )
						{
							--ppi;
							_settextposition( vals[ppi].row, vals[ppi].col );
						}
						break;
					default:
						break;
				}  /* end switch */
				break;
			case BACKSPACE: /* this is for toggling on/off types of values    */
				switch( ppi )
				{
					case 0:case 3:  /* the drive letters */
						if( !strcmp( vals[ppi].txt, "a\n" ) )
							strcpy( vals[ppi].txt, "b\n" );
						else if( !strcmp( vals[ppi].txt, "b\n" ) )
							strcpy( vals[ppi].txt, "c\n" );
						else if( !strcmp( vals[ppi].txt, "c\n" ) )
							strcpy( vals[ppi].txt, "d\n" );
						else if( !strcmp( vals[ppi].txt, "d\n" ) )
							strcpy( vals[ppi].txt, "a\n" );
						/* change the value...                                   */
						_settextposition( vals[ppi].row, vals[ppi].col );
						sprintf( text, vals[ppi].frmt, vals[ppi].txt );
						_outtext( text );
						/* reset the original position...                        */
						_settextposition( vals[ppi].row, vals[ppi].col );
						break;
					case 9:  /* the number of columns value...                  */
						switch( vals[ppi].val.i )
						{
							case 1:
								vals[ppi].val.i = 2;
								col_max = 2;
								break;
							case 2:
								vals[ppi].val.i = 3;
								col_max = 3;
								break;
							case 3:
								vals[ppi].val.i = 4;
								col_max = 4;
								break;
							case 4:
								vals[ppi].val.i = 1;
								col_max = 1;
								break;
						}  /* end switch */
						if( vals[10].val.i > col_max && col_max != 0 )
						{
							vals[10].val.i = col_max;
							_settextposition( vals[10].row, vals[10].col );
							sprintf( text, vals[10].frmt, vals[10].val.i );
							_outtext( text );
						}
						/* change the value */
						_settextposition( vals[ppi].row, vals[ppi].col );
						sprintf( text, vals[ppi].frmt, vals[ppi].val.i );
						_outtext( text );
						/* reset the original position...                        */
						_settextposition( vals[ppi].row, vals[ppi].col );
						break;
					case 10:  /* the fft column value...                        */
						switch( col_max )
						{
							case 1:
								vals[ppi].val.i = 1;
								break;
							case 2:
								if( vals[ppi].val.i == 1 )
									vals[ppi].val.i = 2;
								else
									vals[ppi].val.i = 1;
								break;
							case 3:
								if( vals[ppi].val.i == 1 )
									vals[ppi].val.i = 2;
								else if( vals[ppi].val.i == 2 )
									vals[ppi].val.i = 3;
								else
									vals[ppi].val.i = 1;
								break;
							case 4:
								if( vals[ppi].val.i == 1 )
									vals[ppi].val.i = 2;
								else if( vals[ppi].val.i == 2 )
									vals[ppi].val.i = 3;
								else if( vals[ppi].val.i == 3 )
									vals[ppi].val.i = 4;
								else
									vals[ppi].val.i = 1;
								break;
						}  /* end switch */
						/* change the value */
						_settextposition( vals[ppi].row, vals[ppi].col );
						sprintf( text, vals[ppi].frmt, vals[ppi].val.i );
						_outtext( text );
						/* reset the original position...                        */
						_settextposition( vals[ppi].row, vals[ppi].col );
						break;
					case 11:
						switch( vals[ppi].val.i )
						{
							case 128:
								vals[ppi].val.i = 256;
								blanks = NO;
								break;
							case 256:
								vals[ppi].val.i = 512;
								blanks = NO;
								break;
							case 512:
								vals[ppi].val.i = 1024;
								blanks = NO;
								break;
							case 1024:
								vals[ppi].val.i = 2048;
								blanks = NO;
								break;
							case 2048:
								vals[ppi].val.i = 4096;
								blanks = NO;
								break;
							case 4096:
								vals[ppi].val.i = 8192;
								break;
								blanks = NO;
							case 8192:
								vals[ppi].val.i = 16384;
								blanks = NO;
								break;
							case 16384:
								vals[ppi].val.i = 128;
								blanks = YES;
								break;
						}  /* end switch */
						/* change the value */
						_settextposition( vals[ppi].row, vals[ppi].col );
						sprintf( text, vals[ppi].frmt, vals[ppi].val.i );
						_outtext( text );
						if( blanks )
							_outtext( "  " );
						/* reset the original position...                        */
						_settextposition( vals[ppi].row, vals[ppi].col );
						break;
					default:
						break;
				}  /* end switch */
				break;
			case TAB:  /* then go to the edit line for user input...          */
				switch( ppi )
				{
					case 1: case 2: case 4: case 5: /* the file names...        */
						/* make the name being edited flash...                   */
						strcpy( tstr[ppi].txt, str[ppi].txt );
						oldcolor   = _gettextcolor();
						oldbkcolor = _getbkcolor();
						_settextcolor( YELLOW + FLASH_VAL );
						_settextposition( vals[ppi].row, vals[ppi].col );
						sprintf( text, "%s", str[ppi].txt );
						_outtext( text );
						/* here's the edit line...                               */
						_settextcolor( oldcolor );
						_setbkcolor( oldbkcolor );
						blanks_to_line( vals[ppi].row + 1, 1, 79 );
						_settextposition( vals[ppi].row + 1, vals[ppi].col - 3 );
						sprintf( text, "=> " );
						_outtext( text );
						gets( str[ppi].txt );
						/* blank the newly entered line...                       */
						_settextcolor( BLACK );
						_setbkcolor( BLACK );
						blanks_to_line( vals[ppi].row + 1, 1, 79 );
						/* blank the old title...                                */
						_settextposition( vals[ppi].row, vals[ppi].col );
						sprintf( text, "%s", tstr[ppi].txt );
						_outtext( text );
						/* put the newly entered line where in its place...      */
						_settextcolor( oldcolor );
						_setbkcolor( oldbkcolor );
						_settextposition( vals[ppi].row, vals[ppi].col );
						sprintf( text, "%s", str[ppi].txt );
						_outtext( text );
						/* reset the original position...                        */
						_settextposition( vals[ppi].row, vals[ppi].col );
						break;
				case 6:case 7:case 8:   /* the frequency stuff...              */
						/* make the number being edited flash                    */
						oldcolor   = _gettextcolor();
						oldbkcolor = _getbkcolor();
						_settextcolor( YELLOW + FLASH_VAL );
						_settextposition( vals[ppi].row, vals[ppi].col );
						sprintf( text, vals[ppi].frmt, vals[ppi].val.d );
						_outtext( text );
						/* here's the edit line...                               */
						_settextcolor( oldcolor );
						_setbkcolor( oldbkcolor );
						blanks_to_line( vals[ppi].row + 1, vals[ppi].col, 7 );
						_settextposition( vals[ppi].row + 1, vals[ppi].col - 3 );
						sprintf( text, "=> " );
						_outtext( text );
						_settextposition( vals[ppi].row + 1, vals[ppi].col );
						temp_val1 = vals[ppi].val.d;
						scanf( "%lf", &temp_val2 );
						vals[ppi].val.d = temp_val2;
						/* blank the newly entered number...                     */
						_settextcolor( BLACK );
						_setbkcolor( BLACK );
						blanks_to_line( vals[ppi].row + 1, vals[ppi].col - 3, 12 );
						/* blank the old number...                               */
						_settextposition( vals[ppi].row, vals[ppi].col );
						sprintf( text, vals[ppi].frmt, temp_val1 );
						_outtext( text );
						/* put the newly entered number in its place...          */
						_settextcolor( oldcolor );
						_setbkcolor( oldbkcolor );
						_settextposition( vals[ppi].row, vals[ppi].col );
						sprintf( text, vals[ppi].frmt, vals[ppi].val.d );
						_outtext( text );
						/* reset the original position...                        */
						_settextposition( vals[ppi].row, vals[ppi].col );
						break;
					default:
						break;
				}  /* end switch */
				break;
			default:
				break;
		}  /* end switch */
	}  /* end while */

	/* copy from the 'str' array back to the 'vals' array...                */
	strcpy( vals[1].txt, str[1].txt );
	strcpy( vals[2].txt, str[2].txt );
	strcpy( vals[4].txt, str[4].txt );
	strcpy( vals[5].txt, str[5].txt );

	/* Write from vals to the info structure...                             */
	strcpy( info -> fft_drv, vals[3].txt );
	strcpy( info -> fft_direct, vals[5].txt );
	strcpy( info -> fft_fname, vals[4].txt );
	strcpy( info -> time_drv, vals[0].txt );
	strcpy( info -> time_direct, vals[2].txt );
	strcpy( info -> time_fname, vals[1].txt );
	info -> dt            = vals[6].val.d;
	info -> min_freq_file = vals[7].val.d;
	info -> max_freq_file = vals[8].val.d;
	info -> nocol         = vals[9].val.i;
	info -> fft_col       = vals[10].val.i;
	info -> num_time_pts  = vals[11].val.i;
	/* construct the full drive, directory, name path for fft and time...   */
	/* first remove the "\n" from everything...                             */
	i = 0;
	while( info -> fft_drv[i] != '\n' )
		++i;
	info -> fft_drv[i] = '\0';
	i = 0;
	while( info -> fft_direct[i] != '\n' )
		++i;
	info -> fft_direct[i] = '\0';
	i = 0;
	while( info -> fft_fname[i] != '\n' )
		++i;
	info -> fft_fname[i] = '\0';
	i = 0;
	while( info -> time_drv[i] != '\n' )
		++i;
	info -> time_drv[i] = '\0';
	i = 0;
	while( info -> time_direct[i] != '\n' )
		++i;
	info -> time_direct[i] = '\0';
	i = 0;
	while( info -> time_fname[i] != '\n' )
		++i;
	info -> time_fname[i] = '\0';
	strcpy( info -> fft_file_path, info -> fft_drv );
	strcat( info -> fft_file_path, ":" );
	strcat( info -> fft_file_path, info -> fft_direct );
	strcat( info -> fft_file_path, info -> fft_fname );
	strcpy( info -> time_file_path, info -> time_drv );
	strcat( info -> time_file_path, ":" );
	strcat( info -> time_file_path, info -> time_direct );
	strcat( info -> time_file_path, info -> time_fname );
}  /* end edit_away */

/***************************************************************************/

void write_file_info( char filename[], struct fft_cfg info )
{
	int  i;
	FILE *sf, *fopen();

	/* append a \n on each string that does not already have one...         */
	i = 0;
	while( info.fft_drv[i] != '\0' )
		++i;
	if( info.fft_drv[i - 1] != '\n' )
		strcat( info.fft_drv, "\n" );

	i = 0;
	while( info.fft_direct[i] != '\0' )
		++i;
	if( info.fft_direct[i - 1] != '\n' )
		strcat( info.fft_direct, "\n" );

	i = 0;
	while( info.fft_fname[i] != '\0' )
		++i;
	if( info.fft_fname[i - 1] != '\n' )
		strcat( info.fft_fname, "\n" );

	i = 0;
	while( info.fft_file_path[i] != '\0' )
		++i;
	if( info.fft_file_path[i - 1] != '\n' )
		strcat( info.fft_file_path, "\n" );

	i = 0;
	while( info.time_drv[i] != '\0' )
		++i;
	if( info.time_drv[i - 1] != '\n' )
		strcat( info.time_drv, "\n" );

	i = 0;
	while( info.time_direct[i] != '\0' )
		++i;
	if( info.time_direct[i - 1] != '\n' )
		strcat( info.time_direct, "\n" );

	i = 0;
	while( info.time_fname[i] != '\0' )
		++i;
	if( info.time_fname[i - 1] != '\n' )
		strcat( info.time_fname, "\n" );

	i = 0;
	while( info.time_file_path[i] != '\0' )
		++i;
	if( info.time_file_path[i - 1] != '\n' )
		strcat( info.time_file_path, "\n" );

	if( (sf = fopen( filename, "w" )) != NULL )
	{
		fputs( info.time_file_path, sf );
		fputs( info.fft_file_path, sf );
		fprintf( sf, "%d\n", info.nocol );
		fprintf( sf, "%d\n", info.fft_col );
		fprintf( sf, "%d\n", info.num_time_pts );
		fprintf( sf, "%.12lf\n", (double)(1.00000000 / info.dt) );
		fprintf( sf, "%lf\n",  info.min_freq_file );
		fprintf( sf, "%lf\n",  info.max_freq_file );
		fputs( info.fft_drv, sf );
		fputs( info.fft_direct, sf );
		fputs( info.fft_fname, sf );
		fputs( info.time_drv, sf );
		fputs( info.time_direct, sf );
		fputs( info.time_fname, sf );

		fclose( sf );
	}  /* end if */
	else
	{
		_setvideomode( _DEFAULTMODE );
		printf( "\n\n\n\n\n\n\n" );
		printf( "%s Can Not Be Opened.  Hit any key to end.", filename );
		getch();
		exit( 9 );
	}
}  /* end write_file_info */

/***************************************************************************/

void blanks_to_line( int row, int col, int num_spcs )
{
	int  i;
	char blanks[MAXSTR];

	strcpy( blanks, " " );
	for( i = 0; i < num_spcs; ++i )
		strcat( blanks, " " );
	_settextposition( row, col );
	_outtext( blanks );
}  /* end blanks_to_line */
