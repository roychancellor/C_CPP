/**************************************************************************
*    MODULE:               DRAWPHAS.C                                     *
*                                                                         *
*    CREATED BY:           ROY CHANCELLOR                                 *
*                                                                         *
*    DATE CREATED:         09-08-93                                       *
*                                                                         *
*    DATE LAST MODIFIED:   09-08-93                                       *
*                                                                         *
*    VERSION:              1.00                                           *
*                                                                         *
*    This program displays a phase plane plot on the screen.  It is found *
*    in pendsim.lib                                                       *
*                                                                         *
**************************************************************************/
#include <stdio.h>
#include <graph.h>
#include <math.h>
#include <royware.h>
#include "c:\roy\quickc\pend\pendsim.h"

void draw_phase( double th_new, double om_new, struct axis_data phs_pln,
					  double th_old, double om_old, struct vport phas )
{
	_setviewport( phas.tlx, phas.tly, phas.brx, phas.bry );
	_setwindow( TRUE, XFAC * phs_pln.lx, YFAC * phs_pln.ly,
													XFAC * phs_pln.hx, YFAC * phs_pln.hy );

	_setcolor( YELLOW );
	if( fabs(th_new - th_old) > PI )
		_setpixel_w( th_new, om_new );
	else
	{
		_moveto_w( th_old, om_old );
		_lineto_w( th_new, om_new );
	}
}  /* end draw_phase */

