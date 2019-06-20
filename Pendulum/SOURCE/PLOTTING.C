/**************************************************************************
*    MODULE:                  PLOTTING.C                                  *
*                                                                         *
*    CREATED BY:              ROY CHANCELLOR                              *
*                                                                         *
*    DATE VERSION CREATED:    11-23-92                                    *
*                                                                         *
*    DATE LAST MODIFIED:      09-08-93                                    *
*                                                                         *
*    VERSION:                 1.00                                        *
*                                                                         *
*    DESCRIPTION:                                                         *
*                                                                         *
*    This program reads in data from the specified file and plots it in   *
*    cartesian coordinates as (x1,x2).  The data must be in either the    *
*    form "t x1 x2" or "t x1" in the disk file.                           *
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
#include "c:\roy\quickc\pend\pendsim.h"

void get_and_plot( void );
void create_data_entry_screen( void );
void get_configuration_data( int *, int *, int *, char [] );
void get_max_min_time( double *, double *, double *, double * );
void display_message( int which );
void progctrl( int *, int * );

FILE *datfil, *fopen();

/**************************************************************************/

void main( void )
{
	get_and_plot();
}  /* end main */

/**************************************************************************/

void get_and_plot()
{
	int      which, no_cols, connect, resp_type, yaxis, time_flag = 1, fn_flag;
	int      stop_prog = YES, cnt = 0, back_color, pt_color;
	double   t_old, t_new, x_old, y_old, x_new, y_new;
	double   xmin, xmax, ymin, ymax;
	char     filename[100], txt[100], direc[75];

	create_data_entry_screen();
	get_configuration_data( &resp_type, &connect, &yaxis, filename );
	strcpy( direc, filename );
	while( stop_prog )
	{
		stop_prog = NO;
		if( (datfil = fopen( direc, "r" )) == NULL )
		{
			_clearscreen( _GCLEARSCREEN );
			printf( "\n\n %s Was Not Found.  Hit Any Key. ", direc );
			getch();
		}  /* end if */

		switch( resp_type )
		{
			case P:case M:case T:
				no_cols = 3;
				break;
			case B:
				no_cols = 2;
				break;
			default:
				break;
		}  /* end switch */

		if( cnt == 0 )
		{
			setup_graph( "c:\\pendulum\\genlgrap.dat" );
			back_color = _getcolor();
			if( back_color == BLACK )
				pt_color = YELLOW;
			else
				pt_color = BLACK;
			_setcolor( pt_color );
		}  /* end if */

		get_max_min_time( &xmin, &xmax, &ymin, &ymax );

		fscanf( datfil, "%lf", &t_old );
		fscanf( datfil, "%lf", &x_old );           /* get the first points */
		if( no_cols == 3 )
			fscanf( datfil, "%lf", &y_old );

		if( no_cols == 2 && t_old >= xmin && t_old <= xmax &&
															x_old >= ymin && x_old <= ymax )
			_setpixel_w( t_old, x_old );

		while( fscanf( datfil, "%lf", &t_new ) != EOF )
		{
			if( kbhit() != 0 )
			{
				progctrl( &stop_prog, &back_color );
				if( back_color == BLACK )
					pt_color = YELLOW;
				else
					pt_color = BLACK;
				_setcolor( pt_color );
			}  /* end if */
			if( stop_prog )
				break;

			fscanf( datfil, "%lf", &x_new );

			switch( no_cols )
			{
				case 3:
					fscanf( datfil, "%lf", &y_new );
					switch( connect )
					{
						case YES:
							switch( resp_type )
							{
								case P:
									if( x_old >= xmin && x_new <= xmax &&
															y_old >= ymin && y_new <= ymax )
									{
										_moveto_w( x_old, y_old );
										_lineto_w( x_new, y_new );
									}
									x_old = x_new;
									y_old = y_new;
									break;
								case T:
									switch( yaxis )
									{
										case D:
											if( t_old >= xmin && t_new <= xmax &&
															x_old >= ymin && x_new <= ymax )
											{
												_moveto_w( t_old, x_old );
												_lineto_w( t_new, x_new );
											}
											x_old = x_new;
											t_old = t_new;
											break;
										case V:
											if( t_old >= xmin && t_new <= xmax &&
															y_old >= ymin && y_new <= ymax )
											{
												_moveto_w( t_old, y_old );
												_lineto_w( t_new, y_new );
											}
											y_old = y_new;
											t_old = t_new;
											break;
									}  /* end switch */
									break;
							}  /* end switch */
							break;  /* end case YES */
						case NO:
							switch( resp_type )
							{
								case P:case M:
									if( x_new >= xmin && x_new <= xmax &&
															y_new >= ymin && y_new <= ymax )
										_setpixel_w( x_new, y_new );
									break;
								case T:
									switch( yaxis )
									{
										case V:
											if( t_new >= xmin && t_new <= xmax &&
															y_new >= ymin && y_new <= ymax )
												_setpixel_w( t_new, y_new );
											break;
										case D:
											if( t_new >= xmin && t_new <= xmax &&
															x_new >= ymin && x_new <= ymax )
												_setpixel_w( t_new, x_new );
											break;
									}  /* end switch */
									break;
							}  /* end switch */
							break;
					}  /* end switch */
					break;  /* end case 3 */
				case 2:
					if( t_new >= xmin && t_new <= xmax &&
															x_new >= ymin && x_new <= ymax )
						_setpixel_w( t_new, x_new );
					break;
			}  /* end switch */
		}  /* end while */
		++cnt;
	}  /* end while */

	popup_menu( 200, 170, 500, 265, 2, "    [P]--------Plot Screen",
					"    [Any]------End", " ", " ", " ",
					(int)_getbkcolor(), "on" );
	which = getch();
	popup_menu( 200, 170, 500, 265, 2, "    [P]--------Plot Screen",
					"    [Any]------End", " ", " ", " ",
					(int)_getbkcolor(), "off" );
	if( which == 'p' || which == 'P' )
		getch();

	_setvideomode( _DEFAULTMODE );
	exit( 9 );
}  /* end get_and_plot */

/**************************************************************************/

void display_message( int which )
{
	char        txt[60];
	static char line1[] = {"     [ANY]-----Continue"};
	static char line2[] = {"     [E]-------End"};
	static char line3[] = {"     [G]-------Change Graph"};

	switch( which )
	{
		case 0:
			popup_menu( 200, 170, 500, 295, 3, line1, line2, line3, " ", " ",
																 (int)_getbkcolor(), "on" );
			/*_settextcolor( YELLOW );*/
			break;
		case 1:
			popup_menu( 200, 170, 500, 295, 3, line1, line2, line3, " ", " ",
																 (int)_getbkcolor(), "off" );
			/*_settextcolor( BLACK );*/
			break;
	}  /* end switch */
	/*_settextposition( 3, (80 - (int)strlen( line1 )) / 2 );
	sprintf( txt, line1 );
	_outtext( txt );*/
}  /* end display_message */

/**************************************************************************/

void get_max_min_time( double *xmin, double *xmax,
																double *ymin, double *ymax )
{
	static char data_name[] = {"c:\\pendulum\\genlgrap.dat"};
	char        title[100], yaxis[100], xaxis[100], comments[100];
	double      low_t, high_t, low_y, high_y;
	int         dum2;
	FILE        *fp, *fopen();

	if( (fp = fopen( data_name, "r" )) != NULL )
	{
		fgets( title, MAXSTR - 1, fp );
		fgets( yaxis, MAXSTR - 1, fp );
		fgets( xaxis, MAXSTR - 1, fp );
		fgets( comments, MAXSTR - 1, fp );
		fscanf( fp, "%lf", &low_t );
		fscanf( fp, "%lf", &high_t );
		fscanf( fp, "%lf", &low_y );
		fscanf( fp, "%lf", &high_y );

		fclose( fp );

		*xmin = low_t;
		*xmax = high_t;
		*ymin = low_y;
		*ymax = high_y;
	}  /* end if */
	else
	{
		printf( "\n\n Can't Open %s.  Hit Any Key to End.", data_name );
		getch();
		exit( 11 );
	}
}  /* end get_max_min_time */

/**************************************************************************/

void create_data_entry_screen( void )
{
	int         type_choice_loc, connect_choice_loc, row;
	static char file_prompt[] = {"Data File Name:"};
	static char dir_prompt[] = {"File Directory:"};
	static char plot_type_prompt[] = {"Plot Type:"};
	static char yaxis_type_prompt[] = {"Variable on Y axis:"};
	static char type1[] = {"Phase Portrait--------[P]"};
	static char type2[] = {"Poincar‚ Map----------[M]"};
	static char type3[] = {"Bifurcation Diagram---[B]"};
	static char type4[] = {"Time Response---------[T]"};
	static char disp_prompt[] = {"Displacement----------[D]"};
	static char vel_prompt[] =  {"Velocity--------------[V]"};
	static char choice[] = {"Toggle Your Selection: "};
	static char connect_prompt[] = {"Connect Consecutive Points With a Line?"};
	static char yes_prompt[] = {"Yes-------------------[Y]"};
	static char no_prompt[] =  {"No--------------------[N]"};
	char        input_key = 0x0E;
	char        txt[100];

	row = FILE_NAME;
	type_choice_loc = (int)strlen( choice ) + INDENT_COL;
	connect_choice_loc = (int)strlen( choice ) + INDENT_COL;

	_clearscreen( _GCLEARSCREEN );
/* put the title on the screen...*/
	_settextcolor( CYAN );
	_settextposition( 1, 27 );
	sprintf( txt, "Post Processing Data Entry" );
	_outtext( txt );
/* put the file name prompt on the screen...*/
	_settextcolor( LIGHT_RED );
	_settextposition( row, START_COL );
	_outtext( file_prompt );
/* put the directory name prompt on the screen...*/
	row += 2;
	_settextposition( row, START_COL );
	_outtext( dir_prompt );
/* put the plot type prompt and choices on the screen...*/
	row += 2;
	_settextcolor( LIGHT_RED );
	_settextposition( row, START_COL );
	_outtext( plot_type_prompt );
	++row;
	_settextcolor( YELLOW );
	_settextposition( row, INDENT_COL );
	_outtext( type1 );
	++row;
	_settextposition( row, INDENT_COL );
	_outtext( type2 );
	++row;
	_settextposition( row, INDENT_COL );
	_outtext( type3 );
	++row;
	_settextposition( row, INDENT_COL );
	_outtext( type4 );
	++row;
	_settextposition( row, START_COL );
	_outtext( choice );
	_settextposition( row, type_choice_loc - 1 );
	_outtext( "[ ]" );
/* Put the Y axis type prompt on the screen...*/
	row +=2;
	_settextcolor( LIGHT_RED );
	_settextposition( row, START_COL );
	_outtext( yaxis_type_prompt );
	++row;
	_settextcolor( YELLOW );
	_settextposition( row, INDENT_COL );
	_outtext( disp_prompt );
	++row;
	_settextposition( row, INDENT_COL );
	_outtext( vel_prompt );
	++row;
	_settextposition( row, START_COL );
	_outtext( choice );
	_settextposition( row, type_choice_loc - 1 );
	_outtext( "[ ]" );
/* put the connect points prompt on the screen...*/
	row += 2;
	_settextcolor( LIGHT_RED );
	_settextposition( row, START_COL );
	_outtext( connect_prompt );
	++row;
	_settextcolor( YELLOW );
	_settextposition( row, INDENT_COL );
	_outtext( yes_prompt );
	++row;
	_settextposition( row, INDENT_COL );
	_outtext( no_prompt );
	++row;
	_settextposition( row, START_COL );
	_outtext( choice );
	_settextposition( row, connect_choice_loc - 1 );
	_outtext( "[ ]" );
/* put instructions on the screen...*/
	_settextcolor( CYAN );
	_settextposition( 24, 2 );
	sprintf( txt, "%c & %c...Move Between Fields", 24, 25 );
	_outtext( txt );
	_settextposition( 24, 43 );
	sprintf( txt, "[BKSP]....Toggle Choices in a Field" );
	_outtext( txt );
	_settextposition( 25, 2 );
	sprintf( txt, "[TAB]...Edit File or Directory" );
	_outtext( txt );
	_settextposition( 25, 43 );
	sprintf( txt, "[ENTER]...Finshed Making Changes" );
	_outtext( txt );
}  /* end create_data_entry_screen */

/**************************************************************************/

void get_configuration_data( int *resp_type, int *connect, int *yaxis,
																					char fname[] )
{
	int            pos_indic, row[5], col[5], num_mess = 4, rt_toggle = 0;
	int            cp_toggle = 1;
	int            old_color, rt[4], yax[2], yax_toggle = D, num_fields = 5;
	struct rccoord pos;
	char           input_char, filename[100], directory[100];
	static char    dflt_file_name[] = {"ipndresp.dat"};
	static char    dflt_dir_name[] = {"c:\\pendulum\\"};
	unsigned char  rsp_typ[4] = {'P', 'M', 'B', 'T'};
	char           tmp_str[2];

/* Setup the field positions in screen coordinates...*/
	row[0] = FILE_NAME;       /* file name entry field                      */
	col[0] = 23 + INDENT_COL;
	row[1] = DIRECTORY_NAME;   /* directory name entry field                */
	col[1] = 23 + INDENT_COL;
	row[2] = RESP_TYPE_PROMPT;   /* response type entry field               */
	col[2] = 23 + INDENT_COL;
	row[3] = Y_AXIS_PROMPT;  /* y axis entry field                          */
	col[3] = 23 + INDENT_COL;
	row[4] = CONNECT_POINTS;  /* connect points entry field                 */
	col[4] = 23 + INDENT_COL;

/* Give values to allowable response types...*/
	rt[0] = P;
	rt[1] = M;
	rt[2] = B;
	rt[3] = T;

/* Give values to allowable y axis types...*/
	yax[0] = D;
	yax[1] = V;

/* Default response type and connect points...*/
	*connect = YES;
	*resp_type = T;
	*yaxis = D;

/* Default filename and directory name...*/
	strcpy( filename, dflt_file_name );
	strcpy( directory, dflt_dir_name );
	strcpy( fname, directory );
	strcat( fname, filename );

/* Put all the defaults on the screen...*/
	pos_indic = 0;
	_settextposition( row[pos_indic], col[pos_indic] );
	_outtext( dflt_file_name );
	_settextposition( row[pos_indic], col[pos_indic] );
	pos_indic = 1;
	_settextposition( row[pos_indic], col[pos_indic] );
	_outtext( dflt_dir_name );
	_settextposition( row[pos_indic], col[pos_indic] );
	pos_indic = 2;
	_settextposition( row[pos_indic], col[pos_indic] );
	_outtext( "T" );
	_settextposition( row[pos_indic], col[pos_indic] );
	pos_indic = 3;
	_settextposition( row[pos_indic], col[pos_indic] );
	_outtext( "D" );
	pos_indic = 4;
	_settextposition( row[pos_indic], col[pos_indic] );
	_outtext( "Y" );
	_settextposition( row[pos_indic], col[pos_indic] );

	pos_indic = 0;
	_settextposition( row[pos_indic], col[pos_indic] );

	input_char = 0;
	while( input_char != ENTER_KEY )
	{
		input_char = (char)getch();
		switch( input_char )
		{
			case EXT_ASCII:  /* An extended ASCII key was pressed...          */
				input_char = (char)getch();  /*...so get second part of it     */
				switch( input_char )
				{
					case UP_ARROW:
						pos = _gettextposition();
						if( pos_indic > 0 )
						{
							--pos_indic;
							_settextposition( row[pos_indic], col[pos_indic] );
						}
						else
							printf( "\a" );
						break;
					case DOWN_ARROW:
						pos = _gettextposition();
						if( pos_indic < num_fields - 1 )
						{
							++pos_indic;
							_settextposition( row[pos_indic], col[pos_indic] );
						}
						else
							printf( "\a" );
						break;
					default:
						printf( "\a" );
						break;
				}  /* end switch */
				break;
			case BACKSPACE:
				pos = _gettextposition();
				switch( pos.row )
				{
					case FILE_NAME:       /* at filename prompt */
						printf( "\a" );
						break;
					case DIRECTORY_NAME:  /* at directory prompt */
						printf( "\a" );
						break;
					case RESP_TYPE_PROMPT:  /* at response type prompt */
						if( rt_toggle >= 0 && rt_toggle <= num_mess - 1 )
						{
							_settextcolor( CYAN );
							++rt_toggle;
							if( rt_toggle == num_mess )
								rt_toggle = 0;
							sprintf( tmp_str, "%c", rsp_typ[rt_toggle] );
							_outtext( tmp_str );
							_settextposition( pos.row, pos.col );
							*resp_type = rt[rt_toggle];
						}  /* end if */
						break;
					case CONNECT_POINTS: /* at connect points prompt */
						if( !cp_toggle )
						{
							_outtext( "Y" );
							*connect = YES;
							cp_toggle = 1;
							_settextposition( pos.row, pos.col );
						}
						else if( cp_toggle )
						{
							_outtext( "N" );
							*connect = NO;
							cp_toggle = 0;
							_settextposition( pos.row, pos.col );
						}
						break;
					case Y_AXIS_PROMPT: /* at y axis type prompt */
						if( yax_toggle == V )
						{
							_outtext( "D" );
							*yaxis = D;
							yax_toggle = D;
							_settextposition( pos.row, pos.col );
						}
						else if( yax_toggle == D )
						{
							_outtext( "V" );
							*yaxis = V;
							yax_toggle = V;
							_settextposition( pos.row, pos.col );
						}
						break;
					default: /* don't know where the program is !!! */
						break;
				}  /* end switch */
				break;
			case TAB:
				pos = _gettextposition();
				switch( pos.row )
				{
					case FILE_NAME:
						old_color = _gettextcolor();
						/* get new file name...*/
						_settextcolor( CYAN );
						_settextposition( pos.row + 1, pos.col - 3 );
						_outtext( "->" );
						sprintf( tmp_str, " " );
						_outtext( tmp_str );
						sprintf( tmp_str, "%c", 95 );
						_outtext( tmp_str );
						_settextposition( pos.row + 1, pos.col );
						gets( filename );
						/* blank old filename from original location...*/
						/* and new filename from new position...       */
						_settextcolor( BLACK );
						_settextposition( pos.row, pos.col );
						_outtext( dflt_file_name );
						_settextposition( pos.row + 1, pos.col );
						_outtext( filename );
						_settextposition( pos.row + 1, pos.col - 3 );
						_outtext( "->" );
						strcpy( fname, directory );
						strcat( fname, filename );
						/* put new file name in original location...*/
						_settextcolor( old_color );
						_settextposition( pos.row, pos.col );
						_outtext( filename );
						_settextposition( pos.row, pos.col );
						break;
					case DIRECTORY_NAME:  /* at directory prompt */
						old_color = _gettextcolor();
						/* get new directory name...*/
						_settextcolor( CYAN );
						_settextposition( pos.row + 1, pos.col - 3 );
						_outtext( "->" );
						sprintf( tmp_str, " " );
						_outtext( tmp_str );
						sprintf( tmp_str, "%c", 95 );
						_outtext( tmp_str );
						_settextposition( pos.row + 1, pos.col );
						gets( directory );
						/* blank old directory name from original location...*/
						/* and new directory name from new position...       */
						_settextcolor( BLACK );
						_settextposition( pos.row, pos.col );
						_outtext( dflt_dir_name );
						_settextposition( pos.row + 1, pos.col );
						_outtext( directory );
						_settextposition( pos.row + 1, pos.col - 3 );
						_outtext( "->" );
						strcpy( fname, directory );
						strcat( fname, filename );
						/* put new directory name in original location...*/
						_settextcolor( old_color );
						_settextposition( pos.row, pos.col );
						_outtext( directory );
						_settextposition( pos.row, pos.col );
						break;
					default:
						printf( "\a" );
						break;
				}  /* end switch */
			case '\r':  /* Enter key was pressed:  break so no beep    */
				break;
			default:
				printf( "\a" );
				break;
		}  /* end switch */
	}  /* end while */
}  /* end get_configuration_data */

/**************************************************************************/

void progctrl( int *stop_prog, int *back_color )
{
	int which;

	display_message( 0 );
	getch();
	which = getch();

	switch( which )
	{
		case 'e':case 'E':
			_setvideomode( _DEFAULTMODE );
			exit( 10 );
		case 'g':case 'G':
			display_message( 1 );
			_setvideomode( _DEFAULTMODE );
			grpeditr( "c:\\pendulum\\genlgrap.dat" );
			setup_graph( "c:\\pendulum\\genlgrap.dat" );
			*back_color = _getcolor();
			*stop_prog = YES;
			break;
		default:
			display_message( 1 );
			*stop_prog = NO;
			break;
	}  /* end switch */
}  /* end progctrl */
