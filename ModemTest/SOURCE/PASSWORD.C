/***************************************************************************
*  Module:        PASSWORD.C                                               *
*                                                                          *
*  Created By:    Roy Chancellor                                           *
*                                                                          *
*  Date Created:  02-25-1994                                               *
*                                                                          *
*  Last Modified: 05-10-1994                                               *
*                                                                          *
*  Version:       1.00                                                     *
*                                                                          *
*  Date Released: 05-10-1994                                               *
*                                                                          *
*  Description:   gets a password from user before continuing              *
*                                                                          *
*  Inputs:        password                                                 *
*                                                                          *
*  Returns:       none                                                     *
*                                                                          *
*  Date           Modification                                    Initials *
* ----------   -------------------------------------------------  ---------*
*  03-16-1994  released as version 0.00                              RSC   *
*  03-31-1994  changed password; made more robust; tries to 5        RSC   *
*              ver to 0.01                                                 *
*  04-02-1994  fixed bug once and for all; ver to 0.02.              RSC   *
*  04-04-1994  clear screen when finished; ver to 0.03.              RSC   *
*	04-21-1994	changed password to "beavis"; ver to 0.04					RSC	*
*	05-10-1994	RELEASED FOR USE ON A CAC AS VERSION 1.00					RSC	*
****************************************************************************/
static char *version = "Version 1.00";

#include <stdio.h>
#include <conio.h>   /* getch()                                            */
#include <errno.h>   /* signal()                                           */
#include <signal.h>  /* signal()                                           */
#include <string.h>  /* strncmp()                                          */
#include <ascii.h>   /* for key scan codes                                 */
#include <dos.h>     /* for sleep                                          */

#define MAXTRIES 5

void cycle( void );

void main( void )
{
	int      password_correct = 0, tries;
	char     password[100];
	struct   text_info orig;

	/* set up a handler for CTRL-BREAK...                                   */
	/* SIGINT is for CTRL-BREAK, CTRL-C                                     */
	signal( SIGINT, cycle );

	/* clear the screen and get the password...                             */
	tries = 1;
	while( tries <= MAXTRIES )
	{
		clrscr();
		gettextinfo( &orig );
		gotoxy( 5, 18 );
		cprintf( "Password Protection %s.", version );
		gotoxy( 5, 20 );
		cprintf( "Enter password ==> " );
		textcolor( orig.attribute >> 4 );
		strcpy( password, "\0" );
		cscanf( "%s", &password );
		strcat( password, "\0" );
		textattr( orig.attribute );
		textcolor( WHITE );

		if( strncmp( password, "beavis", 6 ) != 0 &&
												strncmp( password, "BEAVIS", 6 ) != 0 )
		{
			++tries;
			gotoxy( 5, 22 );
			cprintf( "Incorrect.  You have %d tries left.",
																		MAXTRIES - tries + 1 );
			sleep( 2 );
		}
		else
		{
			password_correct = 1;
			break;
		}
	}  /* end while */

	if( !password_correct )
		cycle();
	clrscr();
}  /* end main */

void cycle( void )
{

	  int a = 0;

	  clrscr();
	  gotoxy( 10, 10 );
	  cprintf( "Too Many Tries, You must now reboot the computer." );

	  while( a != 1 );
}  /* end cycle */