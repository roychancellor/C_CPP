/**************************************************************************
*    MODULE:               GETVPORT.C                                     *
*                                                                         *
*    CREATED BY:           ROY CHANCELLOR                                 *
*                                                                         *
*    DATE CREATED:         09-08-93                                       *
*                                                                         *
*    DATE LAST MODIFIED:   09-08-93                                       *
*                                                                         *
*    VERSION:              1.00                                           *
*                                                                         *
*    This module gets the viewport information for both pendulum anim-    *
*    ation programs.  It is accessed through pendsim.lib.                 *
*                                                                         *
**************************************************************************/
#include <stdio.h>
#include <graph.h>
#include <royware.h>
#include "c:\roy\quickc\pend\pendsim.h"

void get_vport_info( struct vport *pend, struct vport *phas )
{
	struct videoconfig vc;

	_getvideoconfig( &vc );
	switch( vc.adapter )
	{
		case _EGA:
			pend -> tlx = 16;
			pend -> tly = 28;
			pend -> brx = 303;
			pend -> bry = 321;
			pend -> adapt = vc.adapter;
			phas -> tlx = 336;
			phas -> tly = 28;
			phas -> brx = 623;
			phas -> bry = 321;
			break;
		case _VGA:
			pend -> tlx = 16;
			pend -> tly = 28;
			pend -> brx = 303;
			pend -> bry = 440;
			pend -> adapt = vc.adapter;
			phas -> tlx = 336;
			phas -> tly = 28;
			phas -> brx = 623;
			phas -> bry = 440;
			break;
		default:
			break;
	}  /* end switch */
}  /* end get_vport_info */
