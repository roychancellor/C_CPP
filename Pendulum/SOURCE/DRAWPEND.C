/**************************************************************************
*    MODULE:               DRAWPEND.C                                     *
*                                                                         *
*    CREATED BY:           ROY CHANCELLOR                                 *
*                                                                         *
*    DATE CREATED:         09-08-93                                       *
*                                                                         *
*    DATE LAST MODIFIED:   09-08-93                                       *
*                                                                         *
*    VERSION:              1.00                                           *
*                                                                         *
*    This program displays the pendulum on the screen.  It is found in    *
*    pendsim.lib,                                                         *
*                                                                         *
**************************************************************************/
#include <stdio.h>
#include <graph.h>
#include <royware.h>
#include <math.h>
#include "c:\roy\quickc\pend\pendsim.h"

void draw_pend( double th_new, double th_old, struct axis_data anima,
															  struct vport pend, int which )
{
	double   xtemp, ytemp, x, y;
	double   r = .05;

	_setviewport( pend.tlx, pend.tly, pend.brx, pend.bry );
	_setwindow( TRUE, XFAC * anima.lx, YFAC * anima.ly,
														XFAC * anima.hx, YFAC * anima.hy );
	xtemp = sin( th_old );
	x     = sin( th_new );
	if( which == 'r' )
	{
		ytemp = -cos( th_old );
		y     = -cos( th_new );
	}
	else
	{
		ytemp = cos( th_old );
		y     = cos( th_new );
	}

	_setcolor( BLACK );
	_moveto_w( 0, 0 );
	_lineto_w( xtemp, ytemp );
	_ellipse_w( _GBORDER, xtemp - r, ytemp - r, xtemp + r, ytemp + r );

	_setcolor( YELLOW );
	_moveto_w( 0, 0 );
	_lineto_w( x, y );
	_ellipse_w( _GBORDER, x - r, y - r, x + r, y + r );
}  /* end draw_pend */
