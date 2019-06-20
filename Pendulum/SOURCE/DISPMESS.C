/**************************************************************************
*    MODULE:              DISPMESS.C                                      *
*                                                                         *
*    CREATED BY:          Roy Chancellor                                  *
*                                                                         *
*    DATE CREATED:        07-30-93                                        *
*                                                                         *
*    DATE LAST MODIFIED:  09-09-93                                        *
*                                                                         *
*    VERSION:             1.00                                            *
*                                                                         *
*    This program allows the user to change values in a data file from    *
*    screen.  This program is to be linked with the menu programs that    *
*    will make calls to it for editing graph coordinates.                 *
*                                                                         *
**************************************************************************/
#include <stdio.h>
#include <string.h>
#include <graph.h>
#include <royware.h>
#include "c:\royware\pendulum\include\pendsim.h"

void display_message( int which, struct sim_control sc )
{
	/*                      1234567890123456789012345678901234              */
	static char line1[] = {"   [R]---Redraw Screen       [G]----Change Graph"};
	static char line2[] = {"   [E]---End Program         [ANY]--Continue"};
	static char line3[] = {"   [S]---Turn Save Points %s"};
	static char line4[] = {"   [D]---Turn Graphics    %s"};
	char        lyne3[MAXSTR], lyne4[MAXSTR];
	char        *bool_str[2];

	bool_str[1] = "ON";
	bool_str[0] = "OFF";
	sprintf( lyne3, line3, bool_str[reverse( sc.save_flag )] );
	sprintf( lyne4, line4, bool_str[reverse( sc.show_graphics )] );

	switch( which )
	{
		case 10:
			popup_menu( 120, 130, 580, 280, 4, line1, line2, lyne3, lyne4, " ",
																(int)_getbkcolor(), "on" );
			break;
		case 11:
			popup_menu( 120, 130, 580, 280, 4, line1, line2, lyne3, lyne4, " ",
																(int)_getbkcolor(), "off" );
			break;
	}  /* end switch */
}  /* end display_message */
