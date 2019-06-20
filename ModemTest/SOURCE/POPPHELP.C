/***************************************************************************
*  Module:        POPPHELP.C                                               *
*                                                                          *
*  Created By:    Roy Chancellor                                           *
*                                                                          *
*  Date Created:  02-08-1994                                               *
*                                                                          *
*  Last Modified: 05-10-1994                                               *
*                                                                          *
*  Version:       1.00                                                     *
*                                                                          *
*  Date Released: 05-10-1994                                               *
*                                                                          *
*  Description:   Creates a popup help window on the screen for a USER-    *
*                 MENU-type program.  The program must have a help file    *
*                 associated with it in the form                           *
*                 !field #                                                 *
*                 help text                                                *
*                 !field #                                                 *
*                 help text....                                            *
*                                                                          *
*  Inputs:        FILE pointer to help file                                *
*                                                                          *
*  Returns:       none                                                     *
*                                                                          *
*  Date           Modification                              		Initials *
* ----------   -------------------------------------------  		---------*
*  05-10-1994     RELEASED FOR USE ON A CAC AS VERSION 1.00            RSC *
****************************************************************************/
#include <conio.h>      /* for screen calls                                */
#include <stdio.h>      /* for file access, MAXSTR definition              */
#include <popphelp.h>   /* for popup help window coordinates               */
#include <boolean.h>    /* for ON/OFF definitions                          */
#include <alloc.h>      /* for calloc calls                                */
#include <usermenu.h>   /* for usermenu coordinates                        */
#include <stdlib.h>     /* for call to atoi                                */
#include <string.h>     /* for call to strlen                              */
#include <dos.h>        /* for call to sleep                               */
#include <ascii.h>      /* for ascii key scan codes                        */

/***************************************************************************
Function:      popup_help
Purpose:       to display a popup help window on F1 command from user.
Arguments:     HELPINFO structure
Returns:       1 if ok, 0 if fail
****************************************************************************/
int popup_help( HELPINFO help )
{
	int      *helpbuf = NULL;
	struct   text_info orig;

	/* get the original screen attributes...                                */
	gettextinfo( &orig );

	/* set the foreground and background colors for the window...           */
	textbackground( HELP_BACK );
	textcolor( HELP_FORE );

	/* put the popup window on the screen...                                */
	if( (helpbuf = create_popup( ON, helpbuf )) == NULL )
	{
		clrscr();
		printf( "\n\n Can't allocate memory for popup help window.\n" );
		sleep( 3 );
		clrscr();
		textattr( orig.attribute );
		return( FAIL );
	}

	/* display help for field at current row...                             */
	display_help( help );

	/* take the window off the screen...                                    */
	helpbuf = create_popup( OFF, helpbuf );

	/* reset the screen to its original condition...                        */
	window( orig.winleft, orig.wintop, orig.winright, orig.winbottom );
	gotoxy( orig.curx, orig.cury );
	textattr( orig.attribute );

	return( OK );
}  /* end popup_help */

/***************************************************************************
Function:      create_popup
Purpose:       to make a popup window appear on and disappear from the screen.
Arguments:     ON/OFF, pointer to allocated memory space
Returns:       pointer to allocated memory space
****************************************************************************/
int *create_popup( int state, int *helpbuf )
{
	int      *tempbuf = NULL;
	size_t   buf_size;

	switch( state )
	{
		case ON:
			/* allocate memory for the screen buffer...                       */
			buf_size = (size_t)(HELP_BOT - HELP_TOP) * (HELP_RYT - HELP_LFT) * 2;
			tempbuf = calloc( buf_size, 2 );
			if( tempbuf == NULL )
				return( NULL );
			/* save the part of the screen where the popup window will go...  */
			gettext( HELP_LFT, HELP_TOP, HELP_RYT, HELP_BOT, tempbuf );
			/* make a window for text...                                      */
			window( HELP_LFT, HELP_TOP, HELP_RYT, HELP_BOT );
			/* fill the window with the background color...                   */
			clrscr();
			break;
		case OFF:
			/* reset the text window coordinates and the original colors...   */
			window( MENU_LFT, MENU_TOP, MENU_RYT + 1, MENU_BOT + 1 );
			/* restore the original screen...                                 */
			puttext( HELP_LFT, HELP_TOP, HELP_RYT, HELP_BOT, helpbuf );
			/* free the allocated memory block...                             */
			free( helpbuf );
			break;
	}  /* end switch */
	return( tempbuf );
}  /* end create_popup */

/**************************************************************************
Function:      display_help
Purpose:       To find the help text in the help file and display it on the
					screen in the popup window.
Arguments:     HELPINFO structure
Returns:       none
***************************************************************************/
void display_help( HELPINFO help )
{
	int      lyne, linecnt, srchblk, next, width, height, page, numpages;
	int      lines[MAXPAGES], centercol, keep_displaying, i, leaderlen;
	int      keep_getting_help, search4field, txtvalue;
	char     help_text[MAXPAGES][HELP_BOT - HELP_TOP][HELP_RYT - HELP_LFT + 1];
	char     txt[HELP_RYT - HELP_LFT + 1], blkstr[3];
	char     prompt[HELP_RYT - HELP_LFT], title[HELP_RYT - HELP_LFT];
	char     *basicprompt   = "ESC-Clear Help Window";
	char     *pgupdn[]      = { "  PgDn-Next Page", "  PgUp-Previous Page" };
	struct   text_info tmp;

	/* help window height and width...                                      */
	height   = HELP_BOT - HELP_TOP;
	width    = HELP_RYT - HELP_LFT;

	/* convert the block number into a string...                            */
	itoa( help.block, blkstr, 10 );

	/* find the correct block number in the help file...                    */
	/* the format of a block number locator is '~#' where # is the block
		number.                                                              */
	srchblk = NO;
	if( help.block != 0 )  /* only search for ~ in multiple block files     */
		srchblk = YES;
	while( srchblk )
	{
		fgets( txt, width, help.fp );
		if( txt[0] == '~' && txt[1] == blkstr[0] )
			srchblk = NO;
	}  /* end while */

	/* search for the field number; once found read in the text...          */
	/* field number...                                                      */
	search4field   = YES;
	while( search4field )
	{
		fgets( txt, width, help.fp );
		if( txt[0] == '!' )
		{
			txtvalue = 0;
			for( i = 1; i < (int)strlen( txt ) && txt[i] != '\n'; ++i )
				txtvalue = txtvalue * 10 + txt[i] - '0';
			if( txtvalue == help.field )
				search4field = NO;
		}  /* end if */
		if( feof( help.fp ) )
			search4field = NO;
	}  /* end while */

	/* ...read in the help text...                                          */
	linecnt  = 1;
	numpages = 1;
	next     = help.field + 1;
	keep_getting_help = YES;
	fgets( txt, width, help.fp );
	while( keep_getting_help )
	{
		strcpy( help_text[numpages - 1][linecnt - 1], txt );
		lines[numpages - 1] = linecnt;
		if( linecnt++ == height - 2 ) /* 2 counts the title & prompt         */
		{
			++numpages;
			linecnt = 1;
		}
		fgets( txt, width, help.fp );
		if( txt[0] == '!' )
		{
			txtvalue = 0;
			for( i = 1; i < (int)strlen( txt ) && txt[i] != '\n'; ++i )
				txtvalue = txtvalue * 10 + txt[i] - '0';
			if( txtvalue == next )        /* at next field; stop              */
				keep_getting_help = NO;
		}  /* end if */
		else if( txt[0] == '~' )         /* next block; stop                 */
			keep_getting_help = NO;
		if( feof( help.fp ) )            /* end of file; stop                */
			keep_getting_help = NO;
	}  /* end while */

	/* strip the leader from the option text...                             */
	i = 0;
	if( help.opttxt[0] == '[' || help.opttxt[0] == '(' )
	{
		while( help.opttxt[i++] != '.' );
		while( help.opttxt[i++] == '.' );
		leaderlen = i;
		for( i = 0; i < (int)strlen( help.opttxt ) - leaderlen + 1; ++i )
			title[i] = help.opttxt[i + leaderlen - 1];
		title[i] = '\0';
	}  /* end if */
	else
		strcpy( title, help.opttxt );
	centercol = (width - (int)strlen( title )) / 2 + 1;

	/* skip a row and display the contents of the help for field...         */
	/* RECALL:  the gotoxy coordinates are RELATIVE to the window           */
	page = 1;
	keep_displaying = YES;
	while( keep_displaying )
	{
		/* tell the user which field is being displayed...                   */
		gotoxy( centercol, 1 );
		cprintf( title );
		for( lyne = 0; lyne < lines[page - 1]; ++lyne )
		{
			gotoxy( 2, 3 + lyne );
			cprintf( help_text[page - 1][lyne] );
		}
		/* put a prompt at the bottom of the help window...                  */
		gettextinfo( &tmp );
		textcolor( PROMPT_FORE );
		textbackground( PROMPT_BACK );
		strcpy( prompt, basicprompt );
		if( numpages > 1 )
		{
			if( page < numpages )
				strcat( prompt, pgupdn[0] );
			if( page > 1 )
				strcat( prompt, pgupdn[1] );
		}
		gotoxy( (width - (int)strlen( prompt )) / 2 + 2, height + 1 );
		cprintf( prompt );
		textattr( tmp.attribute );
		/* put the cursor at the top-left of the help window...              */
		gotoxy( 1, 1 );
		help_control( &page, numpages, &keep_displaying );
	}  /* end while */
	/* reposition the help file pointer to the beginning of the file...     */
	rewind( help.fp );
}  /* end display_help */

/***************************************************************************
Function:      help_control
Purpose:       to get the user's keystroke and act on it
Arguments:     ptr to current page, number of pages, ptr to keep_displaying
Returns:       -1 to break program, 1 otherwise
****************************************************************************/
int help_control( int *curpage, int numpages, int *keep_displaying )
{
	int valid;

	valid = NO;
	while( !valid )
	{
		valid = YES;
		switch( getch() )
		{
			case EXTENDED_KEY:
				switch( getch() )
				{
					case PGDN:  /* display the next page of help text           */
						if( numpages > 1 && *curpage < numpages )
						{
							++(*curpage);
							clrscr();
						}
						else
							valid = NO;
						break;
					case PGUP:  /* display the previous page of help text       */
						if( numpages > 1 && *curpage > 1 )
						{
							--(*curpage);
							clrscr();
						}
						else
							valid = NO;
						break;
					default:
						valid = NO;
						break;
				}  /* end switch */
				break;
			case ESC:
				*keep_displaying = NO;
				break;
			default:
				valid = NO;
				break;
		}  /* end switch */
	}  /* end while */
	return( OK );
}  /* end get_user_response */