/**************************************************************************
*    MODULE:               PENDPHAS.C                                     *
*                                                                         *
*    CREATED BY:           ROY CHANCELLOR                                 *
*                                                                         *
*    DATE CREATED:         09-12-93                                       *
*                                                                         *
*    DATE LAST MODIFIED:   09-12-93                                       *
*                                                                         *
*    VERSION:              1.00                                           *
*                                                                         *
*    This program sets up the pendulum side and phase plane side of the   *
*    screen for pendulum animation.                                       *
*                                                                         *
**************************************************************************/

/* Header Files...                                                         */
#include <stdio.h>
#include <math.h>
#include <graph.h>
#include <conio.h>
#include <process.h>
#include <stdlib.h>
#include <string.h>
#include <royware.h>
#include "c:\roy\quickc\pend\pendsim.h"

void setup_animation( char filename[], int which, struct axis_data *win )
{
	struct axis_data xy;

	/* set up the fonted text outputs...                                    */
		afont_stuff();
	/* get the titles, coordinates, and axis types from file...             */
		aget_graph_data( &xy, filename );
		win -> lx = xy.lx;
		win -> ly = xy.ly;
		win -> hx = xy.hx;
		win -> hy = xy.hy;
	/* Setup the colors for background, labels, border, and ticks...        */
		asetup_colors( &xy );
	/* set the videomode and draw the border...                             */
		aset_mode_and_rectangle( xy, which );
	/* draw graph axes based on the various axis type combinations...       */
		if( !strcmp( xy.type_x, "lin\n" ) && !strcmp( xy.type_y, "lin\n" ) )
			adraw_graph_axes( xy, LIN, LIN, which );
	/* otherwise tell the user that the file is invalid...                  */
	else
	{
		_setvideomode( _DEFAULTMODE );
		printf( "\n\n You did not enter a correct graph type in the graph" );
		printf( "\n configuration file.  The allowable values are 'lin' or" );
		printf( " 'log'.\n Hit Any key to end." );
		getch();
	}  /* end else */
	/* Put the titles on the screen...                                      */
		awrite_titles( xy, which );
	/* Set the color to the background color for other programs to read...  */
		_setcolor( xy.back_color );
}  /* end setup_animation */

/***************************************************************************/

void aget_graph_data( struct axis_data *xy, char graph_name[] )
{
	FILE *filename, *fopen();

	if( (filename = fopen( graph_name, "r" )) != NULL )
	{
		fgets( xy -> title, MAXSTR - 1, filename );
		fgets( xy -> yaxis, MAXSTR - 1, filename );
		fgets( xy -> xaxis, MAXSTR - 1, filename );
		fgets( xy -> comments, MAXSTR - 1, filename );
		fscanf( filename, "%lf", &xy -> lx );
		fscanf( filename, "%lf", &xy -> hx );
		fscanf( filename, "%lf", &xy -> ly );
		fscanf( filename, "%lf", &xy -> hy );
		fscanf( filename, "%d", &xy -> ntx );
		fscanf( filename, "%d", &xy -> nty );
		fscanf( filename, "%d", &xy -> nmtx );
		fscanf( filename, "%d", &xy -> nmty );
		fscanf( filename, "%d", &xy -> crosshairs );
		/* get a dummy '\n' from previous fscanf...                          */
		fgets( xy -> type_x, MAXSTR - 1, filename );
		fgets( xy -> type_x, MAXSTR - 1, filename );
		fgets( xy -> type_y, MAXSTR - 1, filename );
		fgets( xy -> quality, MAXSTR - 1, filename );
		fgets( xy -> grid, MAXSTR - 1, filename );

		fclose( filename );
	}  /* end if */
	else
	{
		printf( "\n\n\n\nCan't Open %s.  Hit Any Key To End.", graph_name );
		getch();
		exit( 11 );
	}  /* end else */
}  /* end aget_graph_data */

/***************************************************************************/

void asetup_colors( struct axis_data *xy )
{
	if( !strcmp( xy -> quality, "presentation\n" ) )
	{
		xy -> back_color   = WHITE;
		xy -> border_color = BLACK;
		xy -> text_color   = BLACK;
		xy -> label_color  = BLACK;
	}
	else if( !strcmp( xy -> quality, "screen\n" ) )
	{
		xy -> back_color   = BLACK;
		xy -> border_color = LIGHT_GRAY;
		xy -> text_color   = CYAN;
		xy -> label_color  = YELLOW;
	}
	else
	{
		printf( "\n\n The correct types of graph are PRESENTATION and" );
		printf( " SCREEN.  Hit any key to end." );
		getch();
		exit( 13 );
	}
}  /* end asetup_colors */

/***************************************************************************/

void aset_mode_and_rectangle( struct axis_data xy, int which )
{
	double xrge, yrge, xlo, xhi, ylo, yhi;
	struct videoconfig screen;

	_getvideoconfig( &screen );

/* Setup the appropriate videomode...                                   */
	switch( screen.adapter )
	{
		case _EGA:
			if( which == 'a' )
			{
				_setvideomode( _ERESCOLOR );
				_setviewport( 16, 28, 303, 321 );
			}
			else if( which == 'p' )
				_setviewport( 336, 28, 623, 321 );
			break;
		case _VGA:
			if( which == 'a' )
			{
				_setvideomode( _VRES16COLOR );
				_setviewport( 16, 28, 303, 440 );
			}
			else if( which == 'p' )
				_setviewport( 336, 28, 623, 440 );
			break;
		default:
			_setvideomode( _DEFAULTMODE );
			printf( "\n\n You must have EGA or VGA to use this program." );
			printf( "\n\n Hit any key to end." );
			getch();
			exit( 12 );
	}  /* end switch */

/* decide on how to set the window and draw the rectangle */
/* LINEAR-LINEAR...*/
	if( (!strcmp( xy.type_x, "lin\n" )) &&
													(!strcmp( xy.type_y, "lin\n" )) )
	{
		xrge = ( xy.hx ) - ( xy.lx );
		yrge = ( xy.hy ) - ( xy.ly );
		xlo = ( xy.lx ) - 0.2 * xrge;
		ylo = ( xy.ly ) - 0.3 * yrge;
		xhi = ( xy.hx ) + 0.2 * xrge;
		yhi = ( xy.hy ) + 0.2 * yrge;

		_setwindow( TRUE, xlo, ylo, xhi, yhi );
		_setcolor( xy.back_color );
		_rectangle_w( _GFILLINTERIOR, xlo, ylo, xhi, yhi );
		_setcolor( xy.border_color );
		_rectangle_w( _GBORDER, xy.lx, xy.ly, xy.hx, xy.hy );
	}
}  /* end aset_mode_and_rectangle */

/***************************************************************************/

void adraw_graph_axes( struct axis_data xy, int do_xax, int do_yax,
																						int which )
{
	char     title[80], num_lab[10];
	int      cnt, nmtx, nmty, num_times_x, num_times_y;
	int      tick_index, dec_no, num_decades_x, num_decades_y;
	double   delx, dely, xrge, yrge, xpos, ypos, xmpos, ympos;
	double   xlo, xhi, ylo, yhi, low_tick_y, low_tick_x;
	struct   xycoord view, phys;

	if( do_xax == LIN )
	{
		delx = ( xy.hx - xy.lx ) / xy.ntx;
		xrge = ( xy.hx ) - ( xy.lx );
		xhi = xy.hx;
		xlo = xy.lx;
		nmtx = xy.nmtx;
		num_times_x = xy.ntx;
	}
	if( do_yax == LIN )
	{
		dely = ( xy.hy - xy.ly ) / xy.nty;
		yrge = ( xy.hy ) - ( xy.ly );
		yhi = xy.hy;
		ylo = xy.ly;
		nmty = xy.nmty;
		num_times_y = xy.nty;
	}

/************** Make The X AXIS Ticks, Minor ticks, and Number Labels ******/

	if( (do_xax == LIN || do_xax == LOG) && which == 'p' )
	{
		/* make crosshairs if appropriate...                                 */
		if( xy.crosshairs )
		{
			_setcolor( xy.border_color );
			_moveto_w( xlo, 0 );
			_lineto_w( xhi, 0 );
		}
		tick_index = 0;
		dec_no = 1;
		/* MAJOR TICKS HERE...                                               */
		for( cnt = 0; cnt <= num_times_x; ++cnt )
		{
			_setcolor( xy.border_color );
			if( (cnt % xy.ntx) == 0 && do_xax == LOG )
			{
				delx = pow( 10, dec_no ) * xy.lx / xy.ntx;
				low_tick_x = pow( 10, (xlo + dec_no - 1) );
				tick_index = 0;
				xpos = log10( low_tick_x + tick_index * delx );
				sprintf( num_lab, "%.3lf", low_tick_x + tick_index * delx );
				_setcolor( xy.label_color );
				view = _getviewcoord_w( xpos, ylo );
				_moveto( view.xcoord - 16, view.ycoord + 9 );
				_outgtext( num_lab );
				++dec_no;
			}  /* end if */
			else
				++tick_index;

			_setcolor( xy.border_color );
			if( do_xax == LIN )
				xpos = ( xy.lx + cnt * delx );
			if( do_xax == LOG )
				xpos = log10( low_tick_x + tick_index * delx );
			if( xy.crosshairs )
			{
				_moveto_w( xpos, -.025 * yrge );
				_lineto_w( xpos, .025 * yrge );
			}
			if( !strcmp( xy.grid, "off\n" ) )
			{
				_moveto_w( xpos, ylo );
				_lineto_w( xpos, ylo + .025 * yrge );
				_moveto_w( xpos, yhi );
				_lineto_w( xpos, yhi - .025 * yrge );
			}
			else if( !strcmp( xy.grid, "on\n" ) )
			{
				_moveto_w( xpos, ylo );
				_lineto_w( xpos, yhi );
			}

			_setcolor( xy.label_color );
			if( do_xax == LIN )
			{
				sprintf( num_lab, "%.3lf", xy.lx + cnt * delx );
				view = _getviewcoord_w( xpos, ylo );
				_moveto( view.xcoord - 16, view.ycoord + 9 );
				_outgtext( num_lab );
			}
		}  /* end for */

		_setcolor( xy.border_color );
		if( do_xax == LIN )
		{
			/* MINOR TICKS HERE...                                            */
			for( cnt = 0; cnt <= xy.ntx * nmtx; ++cnt )
			{
				xmpos = ( xy.lx + cnt * delx / nmtx );
				if( xy.crosshairs )
				{
					_moveto_w( xmpos, -.01 * yrge );
					_lineto_w( xmpos,  .01 * yrge );
				}
				_moveto_w( xmpos, ylo );
				_lineto_w( xmpos, ylo + .01 * yrge );
				_moveto_w( xmpos, yhi );
				_lineto_w( xmpos, yhi - .01 * yrge );
			}  /* end for */
		}  /* end if */
	}  /* end if */

/************** Make The Y AXIS Ticks, Minor ticks, and Number Labels******/

	if( (do_yax == LIN || do_yax == LOG) && which == 'p' )
	{
		/* Make crosshairs if appropriate...                                 */
		if( xy.crosshairs )
		{
			_setcolor( xy.border_color );
			_moveto_w( 0, ylo );
			_lineto_w( 0, yhi );
		}
		tick_index = 0;
		dec_no = 1;
		/* MAJOR TICKS HERE...                                               */
		for( cnt = 0; cnt <= num_times_y; ++cnt )
		{
			_setcolor( xy.border_color );
			if( (cnt % xy.nty) == 0 && do_yax == LOG )
			{
				dely = pow( 10, dec_no ) * xy.ly / xy.nty;
				low_tick_y = pow( 10, (ylo + dec_no - 1) );
				tick_index = 0;
				ypos = log10( low_tick_y + tick_index * dely );
				sprintf( num_lab, "%.1le", low_tick_y + tick_index * dely );
				_setcolor( xy.label_color );
				view = _getviewcoord_w( xlo, ypos );
				_moveto( view.xcoord - 49, view.ycoord - 5 );
				_outgtext( num_lab );
				++dec_no;
			}  /* end if */
			else
				++tick_index;
			if( do_yax == LIN )
				ypos = ( xy.ly + cnt * dely );
			if( do_yax == LOG )
				ypos = log10( low_tick_y + tick_index * dely );
			_setcolor( xy.border_color );
			if( xy.crosshairs )
			{
				_moveto_w( -.02 * xrge, ypos );
				_lineto_w( .02 * xrge, ypos );
			}
			if( !strcmp( xy.grid, "off\n" ) )
			{
				_moveto_w( xlo, ypos );
				_lineto_w( xlo + .02 * xrge, ypos );
				_moveto_w( xhi, ypos );
				_lineto_w( xhi - .02 * xrge, ypos );
			}
			else if( !strcmp( xy.grid, "on\n" ) )
			{
				_moveto_w( xlo, ypos );
				_lineto_w( xhi, ypos );
			}
			if( do_yax == LIN )
			{
				sprintf( num_lab, "%.3lf", xy.ly + cnt * dely );
				_setcolor( xy.label_color );
				view = _getviewcoord_w( xlo, ypos );
				_moveto( view.xcoord - 39, view.ycoord - 5 );
				_outgtext( num_lab );
			}
		}  /* end for */

		_setcolor( xy.border_color );
		if( do_yax == LIN )
		{
			/*MINOR TICKS HERE...                                             */
			for( cnt = 0; cnt <= xy.nty * nmty; ++cnt )
			{
				ympos = ( xy.ly + cnt * dely / nmty );
				if( xy.crosshairs )
				{
					_moveto_w( -.01 * xrge, ympos );
					_lineto_w( .01 * xrge, ympos );
				}
				_moveto_w( xlo, ympos );
				_lineto_w( xlo + .01 * xrge, ympos );
				_moveto_w( xhi, ympos );
				_lineto_w( xhi - .01 * xrge, ympos );
			} /* end for */
		}  /* end if */
	}  /* end if */

}  /* end adraw_graph_axes */

/**************************************************************************/

void awrite_titles( struct axis_data xy, int which )
{
	static unsigned char afont_str_main[] = {"t'tms rmn'h20w12b"};
	static unsigned char afont_str_others[] = {"t'tms rmn'h15w8b"};
	struct videoconfig vc;

	_getvideoconfig( &vc );

/******************Put The Axis Labels On The Screen************************/

	if( _setfont( afont_str_main ) < 0 )
	{
		_setvideomode( _DEFAULTMODE );
		printf( "\n\n Couldn't Set The 20 x 12 Font.  Hit Any Key To end" );
		getch();
		exit( 11 );
	}
	_setcolor( xy.text_color );
	_moveto( (287 - 12 * strlen( xy.title )) / 2, 10 );
	_outgtext( xy.title );

	if( _setfont( afont_str_others ) < 0 )
	{
		_setvideomode( _DEFAULTMODE );
		printf( "\n\n Couldn't Set The 15 x 8 Font.  Hit Any Key To end" );
		getch();
		exit( 11 );
	}
	switch( vc.adapter )
	{
		case _EGA:
			_moveto( (287 - 6 * strlen( xy.xaxis )) / 2, 310 );
			_outgtext( xy.xaxis );
			_moveto( (287 - 6 * strlen( xy.comments )) / 2, 330 );
			_outgtext( xy.comments );
			_setgtextvector( 0, 1 );
			_moveto( 10, (349 + 6 * strlen( xy.yaxis )) / 2 );
			_outgtext( xy.yaxis );
			_setgtextvector( 1, 0 );
			break;
		case _VGA:
			_moveto( (287 - 6 * strlen( xy.xaxis )) / 2, 425 );
			_outgtext( xy.xaxis );
			_moveto( (287 - 6 * strlen( xy.comments )) / 2, 453 );
			_outgtext( xy.comments );
			_setgtextvector( 0, 1 );
			_moveto( 10, (479 + 6 * strlen( xy.yaxis )) / 2 );
			_outgtext( xy.yaxis );
			_setgtextvector( 1, 0 );
			break;
	}  /* end switch */
}  /* end write titles */

/**************************************************************************/

void afont_stuff( void )
{
	static unsigned char afont_str1[] = {"t'tms rmn'h12w6b"};
	static unsigned char afont_str2[] = {"t'tms rmn'h15w8b"};

	if( _registerfonts( FONT_FILE ) < 0 )
	{
		_setvideomode( _DEFAULTMODE );
		printf( "\n\n Couldn't Register The Font.  Hit Any Key To End" );
		getch();
		exit( 10 );
	}
	if( _setfont( afont_str1 ) < 0 )
	{
		_setvideomode( _DEFAULTMODE );
		printf( "\n\n Couldn't Set The 12 x 6 Font.  Hit Any Key To end" );
		getch();
		exit( 11 );
	}
}  /* end afont_stuff */
