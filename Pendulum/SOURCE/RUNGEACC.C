/**************************************************************************
*    MODULE:               RUNGEACC.C                                     *
*                                                                         *
*    CREATED BY:           ROY CHANCELLOR                                 *
*                                                                         *
*    DATE CREATED:         07-29-93                                       *
*                                                                         *
*    DATE LAST MODIFIED:   09-12-93                                       *
*                                                                         *
*    VERSION:              1.00                                           *
*                                                                         *
*    This subroutine is compiled and put into "pendsim.lib".  It is called*
*    from the pendulum simulation programs.                               *
*                                                                         *
**************************************************************************/

#include <stdio.h>
#include <math.h>
#include <royware.h>
#include "c:\roy\quickc\pend\pendsim.h"

double accel( double th_old, double om_old, double t, double rp[], int which )
{
	double acc, td;

	switch( which )
	{
		case 'r':
			if( rp[DAMP] != 0 )
				acc = -sin( th_old ) - ( om_old / rp[DAMP] )
														  + rp[AMPL] * cos( rp[FREQ] * t );
			else
				acc = -sin( th_old ) + rp[AMPL] * cos( rp[FREQ] * t );
			break;
		case 'i':
			td  = rp[SPRING] * rp[AMPL] * cos( rp[FREQ] * t );
			acc = -rp[DAMP] * om_old - rp[SPRING] * th_old
															+ rp[MASS] * sin( th_old ) + td;
			break;
	}  /* end switch */
	return( acc );
}  /* end accel */

/**************************************************************************/

void runge( double th_old, double om_old, double *th_new, double *om_new,
				double t, double dt, double rp[], int which )
{
	double th_k1, th_k2, th_k3, th_k4;
	double om_k1, om_k2, om_k3, om_k4;

	th_k1 = om_old * dt;
	om_k1 = accel( th_old, om_old, t, rp, which ) * dt;

	th_k2 = (om_old + om_k1 / 2.0) * dt;
	om_k2 = accel( th_old + th_k1 / 2.0, om_old + om_k1 / 2.0,
															t + dt / 2.0, rp, which ) * dt;

	th_k3 = (om_old + om_k2 / 2.0) * dt;
	om_k3 = accel( th_old + th_k2 / 2.0, om_old + om_k2 / 2.0,
															t + dt / 2.0, rp, which ) * dt;

	th_k4 = (om_old + om_k3) * dt;
	om_k4 = accel( th_old + th_k3, om_old + om_k3, t + dt, rp, which ) * dt;

	*th_new = th_old + (th_k1 + 2.0 * th_k2 + 2.0 * th_k3 + th_k4) / 6.0;
	*om_new = om_old + (om_k1 + 2.0 * om_k2 + 2.0 * om_k3 + om_k4) / 6.0;
}  /* end runge */
