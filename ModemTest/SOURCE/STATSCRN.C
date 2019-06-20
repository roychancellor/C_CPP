/***************************************************************************
*  Module:        STATSCRN.C                                               *
*                                                                          *
*  Created By:    Roy Chancellor                                           *
*                                                                          *
*  Date Created:  01-28-1994                                               *
*                                                                          *
*  Date Modified: 05-10-1994                                               *
*                                                                          *
*  Version:       1.00                                                     *
*                                                                          *
*  Date Released: 05-10-1994                                               *
*                                                                          *
*  Description:   Creates a status screen for modem tests.                 *
*                                                                          *
*  Inputs:        None                                                     *
*                                                                          *
*  Returns:       None                                                     *
*                                                                          *
*  Date           Modification                              Initials       *
*  --------------------------------------------------------------------    *
*  03-09-1994  Released as version 0.00                              RSC   *
*  03-11-1994  added restore_scrn() function; ver to 0.01            RSC   *
*  04-06-1994  modified the part of scrn_from_disk that reads in the RSC   *
*              fields and commands; ver to 0.02                            *
*	05-10-1994	RELEASED FOR USE ON A CAC AS VERSION 1.00					RSC	*
****************************************************************************/
static   char *version = "Version 1.00";

/* Include files that came with Turbo C...                                 */
#include <stdio.h>
#include <stdlib.h>
#include <conio.h>         /* for all screen calls                         */
#include <process.h>       /* for calls to exit                            */
#include <string.h>        /* for strcmp, strcpy, strcat                   */
#include <dos.h>           /* for calls to enable, disable                 */
#include <alloc.h>         /* for calls to calloc                          */
#include <dir.h>           /* for calls to getcwd                          */

/* Include files custom written for the EF Data modem test project...      */
#include <boolean.h>       /* has values that take on 1 or 0               */
#include <ascii.h>         /* has key press values                         */
#include <statscrn.h>      /* defines the status screen coordinates        */
#include <popphelp.h>      /* for calls to popup_help                      */
#include <utility.h>       /* for calls to getfp, str2int                  */
#include <popwin.h>        /* for calls to popwin                          */
#include <monvalue.h>      /* for calls to monitro_values()                */

#define MENUFORE  YELLOW
#define MENUBACK  LIGHTBLUE

/* Function prototypes...                                                  */
static int  scrn_from_disk( void );
static int  process_response( int, PROGSTAT * );
static void opts_to_scrn( void );
static void create_border( char *, char *, char *, char *, char *, char * );
static void cursor_move( int );
static void get_cfg_paths( char *, char * );
static int  modem_status( void );
static void help_prompt( int );
static int  f1tof10( int );

/* GLOBAL variable declarations...                                         */
SCREEN   s[MAXBLOCKS];  /* holds infor for the status screen               */
CFGHELP  cfghlp;        /* holds info for config. and help paths           */
int      blockno = 1;   /* the current block number                        */
FILE     *stathelp;     /* pointer to help file                            */
POPWIN   *stpr;         /* for status screen help prompt                   */

/***************************************************************************
Function:      statscrn
Purpose:       to guide the status screen program
Arguments:     number of command line arguments, command line argument strings
					argv[0] is used to obtain the executable filename and path.
Returns:       1 if ok; 0 if fail
****************************************************************************/
int statscrn( char *sta, char *txthlp )
{
	int      blk, user_response, statsize, *menubuf;
	struct   text_info orig;
	PROGSTAT pgst;

	/* get the original screen configuration...                             */
	gettextinfo( &orig );

	/* get the required screen configuration and help file paths...         */
	/* sta will have the name of the .sta file in it.                       */
	/* txthlp will have the name of the .txt/.hlp file in it.               */
	get_cfg_paths( sta, txthlp );

	/* take a picture of the menu screen before putting on the status...    */
	statsize = (STAT_RYT - STAT_LFT + 4) * (STAT_BOT - STAT_TOP + 4);
	menubuf = NULL;
	menubuf = calloc( (size_t)statsize, (size_t)sizeof( int ) );
	if( menubuf == NULL )
	{
		printf( "\n\n Couldn't allocate memory for picture of menu." );
		getch();
		return( FAIL );
	}
	if( gettext( STAT_LFT, STAT_TOP, STAT_RYT, STAT_BOT + 1, menubuf ) == FAIL )
	{
		printf( "\n\n Gettext failed...\n" );
		getch();
		restore_scrn( orig );
	}

	/* define a text window for the status screen...                        */
	window( STAT_LFT, STAT_TOP, STAT_RYT, STAT_BOT );

	/* initialize variables...                                              */
	pgst.redraw       = NO;
	pgst.keep_going   = YES;
	pgst.prompt       = OFF;

	/* get the screen configuration from data file...                       */
	if( scrn_from_disk() != OK )
	{
		restore_scrn( orig );
		return( FAIL );
	}
	/* open the screen field help file...                                   */
	if( (stathelp = getfp( cfghlp.help_path, "r" )) == NULL )
	{
		restore_scrn( orig );
		return( FAIL );
	}

	/* define the foreground and background colors for each block...        */
	for( blk = 0; blk < cfghlp.num_blocks; ++blk )
	{
		s[blk].txtcolor   = YELLOW;
		s[blk].backcolor  = CYAN;
		s[blk].statfore   = WHITE;
		s[blk].statback   = LIGHTBLUE;
	}  /* end for */
	cfghlp.borderfore = CYAN;
	cfghlp.borderback = BLACK;

/*******************************MAIN LOOP***********************************/
/* keep showing the screen until the user enters the exit command...       */
	while( pgst.keep_going )
	{
		/* redraw gets set to YES in process_response and must
			be set back to NO before returning to user_response               */
		pgst.redraw = NO;

		/* put screen fields on the screen...                                */
		opts_to_scrn();
		/* put the modem status information on the screen...                 */
		if( modem_status() == FAIL )
		{
			restore_scrn( orig );
			return( FAIL );
		}

		while( pgst.redraw == NO )
		{
			/* wait for the user's response...                                */
			user_response = getch();
			/* process the user's response...                                 */
			process_response( user_response, &pgst );
		}  /* end while */
	}  /* end while */
/*********************END MAIN LOOP*****************************************/

	/* put the menu snapshot on the screen...                               */
	if( puttext( STAT_LFT, STAT_TOP, STAT_RYT, STAT_BOT + 1, menubuf ) == FAIL )
	{
		printf( "\n\n puttext failed...\n" );
		getch();
	}
	free( menubuf );

	/* reset the video screen to its normal attributes...                   */
	help_prompt( OFF );
	restore_scrn( orig );
	fclose( stathelp );
	return( OK );
}  /* end main */

/***************************************************************************
Function:      scrn_from_disk
Purpose:       obtain the screen block configuration files
Arguments:     none
Returns:       1 if file exists; 0 if it does not
****************************************************************************/
static int scrn_from_disk( void )
{
	int      blk, item, delta_col, templen, i, j;
	char     txt[MAXSTR];
	FILE     *cfg;

	/* open the screen block configuration file...                          */
	if( (cfg = getfp( cfghlp.scrn_path, "r" )) == NULL )
		return( FAIL );

	/* read the number of blocks...                                         */
	cfghlp.num_blocks = str2int( fgets( txt, MAXSTR, cfg ) );

	/* read the information for each block...                               */
	for( blk = 0; blk < cfghlp.num_blocks; ++blk )
	{
		/* get the # fields, start row, start col, field col, and delta row  */
		s[blk].num_fields = str2int( fgets( txt, MAXSTR, cfg ) );
		s[blk].start_row  = str2int( fgets( txt, MAXSTR, cfg ) );
		s[blk].start_col  = str2int( fgets( txt, MAXSTR, cfg ) );
		s[blk].field_col  = str2int( fgets( txt, MAXSTR, cfg ) );
		delta_col = s[blk].field_col - s[blk].start_col;
		s[blk].delta_row  = str2int( fgets( txt, MAXSTR, cfg ) );
		s[blk].title_row  = str2int( fgets( txt, MAXSTR, cfg ) );

		/* convert the rows and columns to window RELATIVE...                */
		/* start column...                                                   */
		if( s[blk].start_col > STAT_LFT )
			s[blk].start_col = s[blk].start_col - STAT_LFT + 1;
		else
			s[blk].start_col = 3;
		/* start row...                                                      */
		if( s[blk].start_row > STAT_TOP )
			s[blk].start_row = s[blk].start_row - STAT_TOP + 1;
		else
			s[blk].start_row = 3;
		/* title row...                                                      */
		if( s[blk].title_row > STAT_TOP )
			s[blk].title_row = s[blk].title_row - STAT_TOP + 1;
		else
			s[blk].start_row = 2;
		/* field column...                                                   */
		s[blk].field_col = s[blk].start_col + delta_col;

		/* get the block text strings and associated modem commands...       */
		for( item = 0; item <= s[blk].num_fields; ++item )
		{
			fgets( txt, MAXSTR, cfg );
			i = 0;
			while( txt[i] != '~' )
				s[blk].stat_items[item][i] = txt[i++];
			s[blk].stat_items[item][i] = '\0';
			++i;  /* skip the ~ in the string                                 */
			j = 0;
			while( txt[i] != '\n' )
				s[blk].stat_cmnds[item][j++] = txt[i++];
			s[blk].stat_cmnds[item][j] = '\0';
		}
	}  /* end for */

	/* get the number of user popup help items...                           */
	cfghlp.num_help = str2int( fgets( txt, MAXSTR, cfg ) );

	/* get the help strings and determine the length of the longest one...  */
	cfghlp.biglen = 0;
	cfghlp.bigind = 0;
	for( blk = 0; blk < cfghlp.num_help; ++blk )
	{
		fgets( cfghlp.help_items[blk], MAXSTR, cfg );
		if( (templen = (int)strlen( cfghlp.help_items[blk] )) >=
																				cfghlp.biglen )
		{
			cfghlp.biglen = templen;
			cfghlp.bigind = blk;
		}
		strip_lf( cfghlp.help_items[blk] );
	}  /* end for */

	fclose( cfg );
	return( OK );
}  /* end scrn_from_disk */

/***************************************************************************
Function:   process_response
Purpose:    act on a user input from the screen of fields.
Arguments:  user_response, ptr to redraw, ptr to keep_going.
Returns:    1 if ok; 0 if fail
****************************************************************************/
static int process_response( int ursp, PROGSTAT *pgst )
{
	int cur_row, index, blk, extkey;

	/* act on the user's response...                                        */
	cur_row  = wherey();
	blk      = blockno - 1;
	pgst -> redraw = YES;
	switch( ursp )
	{
		case SPACE:  /* turn the prompt on/off at the current location...    */
			switch( pgst -> prompt )
			{
				case ON:  /* then turn off...                                  */
					textcolor( s[blk].txtcolor );
					textbackground( BLACK );
					pgst -> prompt = OFF;
					break;
				case OFF:  /* then turn on...                                  */
					textcolor( s[blk].txtcolor );
					textbackground( s[blk].backcolor );
					pgst -> prompt = ON;
					break;
			}  /* end switch */
			index = ( cur_row - s[blk].start_row ) / s[blk].delta_row;
			/* status line...                                                 */
			gotoxy( s[blk].start_col, cur_row );
			cprintf( s[blk].stat_items[index] );
			/* status value...                                                */
			textcolor( s[blk].statfore );
			textbackground( s[blk].statback );
			gotoxy( s[blk].field_col, cur_row );
			cprintf( s[blk].stat_value[index] );
			gotoxy( s[blk].start_col, cur_row );
			pgst -> redraw = NO;
			break;
		case EXTENDED_KEY:
			switch( (extkey = getch()) )
			{
				case UP_ARROW:case DOWN_ARROW:
					cursor_move( extkey );
					pgst -> prompt = ON;
					pgst -> redraw = NO;
					break;
				case F1:case F2:case F3:case F4:case F5:case F6:case F7:case F8:
				case F9:case F10:
					pgst -> redraw = NO;
					if( f1tof10( extkey ) == FAIL )
						return( FAIL );
					break;
				default:
					break;
			}  /* end switch */
			break;
		case ESC:
			pgst -> keep_going = NO;
			break;
		default:
			pgst -> redraw = NO;
			break;
	}  /* end switch */
	return( OK );
}  /* end process_response */

/***************************************************************************
Function:   opts_to_scrn
Purpose:    put screen fields on screen.
Arguments:  none
Returns:    none
****************************************************************************/
static void opts_to_scrn( void )
{
	int   blk, item, row;

	/* create the border...                                                 */
	textcolor( cfghlp.borderfore );
	textbackground( cfghlp.borderback );
	create_border( "Í", "º", "É", "È", "»", "¼" );
	/*             t/b  l/r   rt   lb   rt   rb                             */

	/* put the block titles on the screen, centered on each half...         */
	for( blk = 0; blk < cfghlp.num_blocks; ++blk )
	{
		/* set the text color for the title...                               */
		textcolor( s[blk].txtcolor );
		textbackground( s[blk].backcolor );
		gotoxy( ((STAT_RYT - STAT_LFT) -
				  (int)strlen( s[blk].stat_items[s[blk].num_fields] )) / 2,
																			s[blk].title_row );
		cprintf( s[blk].stat_items[s[blk].num_fields] );
	}  /* end for */

	/* put help prompt on screen...                                         */
	help_prompt( ON );

	/* place the fields on the screen by indexing through the strings
		in the stat_items character string...                                */
	for( blk = 0; blk < cfghlp.num_blocks; ++blk )
	{
		textcolor( s[blk].txtcolor );
		textbackground( BLACK );
		row = s[blk].start_row;
		for( item = 0; item < s[blk].num_fields; ++item )
		{
			gotoxy( s[blk].start_col, row );
			cprintf( s[blk].stat_items[item] );
			row += s[blk].delta_row;
		}  /* end for */
	}  /* end for */

	/* set the cursor to the first block, first field...                    */
	gotoxy( s[0].start_col, s[0].start_row );
}  /* end opts_to_scrn */

/*****************************************************************************
Function:   create_border
Purpose:    create a border around the perimeter of the screen using the
				character passed in the argspts.
Called:     from opts_to_scrn
Arguments:  horizontal character, vertical character, corner characters
Returns:    none
*****************************************************************************/
static void create_border( char *horizchar, char *vertchar, char *lt, char *lb,
						  char *rt, char *rb )
{
	int   row, col;
	char  lyne[80];

	/* put the corners on first...                                          */
	gotoxy( STAT_RYT - STAT_LFT, STAT_BOT - STAT_TOP + 1 );
	cprintf( rb );
	gotoxy( STAT_LFT - STAT_LFT + 1, STAT_TOP - STAT_TOP + 1 );
	cprintf( lt );
	gotoxy( STAT_LFT - STAT_LFT + 1, STAT_BOT - STAT_TOP + 1 );
	cprintf( lb );
	gotoxy( STAT_RYT - STAT_LFT, STAT_TOP - STAT_TOP + 1 );
	cprintf( rt );

	/* create HORIZONTAL lines at STAT_TOP and STAT_BOT of screen...              */
	strcpy( lyne, "\0" );
	for( col = STAT_LFT - STAT_LFT + 2; col <= STAT_RYT - STAT_LFT - 1; ++col )
		strcat( lyne, horizchar );
	gotoxy( STAT_LFT - STAT_LFT + 2, STAT_TOP - STAT_TOP + 1 );
	cprintf( lyne );
	gotoxy( STAT_LFT - STAT_LFT + 2, STAT_BOT - STAT_TOP + 1 );
	cprintf( lyne );

	/* create VERTICAL lines at STAT_LFT and STAT_RYT of screen...                */
	strcpy( lyne, vertchar );
	for( col = STAT_LFT - STAT_LFT + 2; col <= STAT_RYT - STAT_LFT - 1; ++col )
		strcat( lyne, " " );
	strcat( lyne, vertchar );
	strcat( lyne, "\0" );
	for( row = STAT_TOP - STAT_TOP + 2; row <= STAT_BOT - STAT_TOP; ++row )
	{
		gotoxy( STAT_LFT - STAT_LFT + 1, row );
		cprintf( lyne );
	}
}  /* end create_border */

/****************************************************************************
Function:      cursor_move
Purpose:       to move the cursor on the screen or initiate help.
Arguments:     the extended key
Returns:       none
****************************************************************************/
static void cursor_move( int extkey )
{
	int cur_row, newrow, index, blk, bot, row;

	blk      = blockno - 1;
	cur_row  = wherey();
	switch( extkey )
	{
		case UP_ARROW:    /* move up one field...                            */
			if( cur_row > s[blk].start_row )
			{
				newrow = cur_row - s[blk].delta_row;
				/* calculate the character string index corresponding to
					the current row...                                          */
				index = ( cur_row - s[blk].start_row ) / s[blk].delta_row;
				/* make the highlighted text on the current row plain...       */
				/* field text...                                               */
				textcolor( s[blk].txtcolor );
				textbackground( BLACK );
				gotoxy( s[blk].start_col, cur_row );
				cprintf( s[blk].stat_items[index] );
				/* field value...                                              */
				textcolor( s[blk].statfore );
				textbackground( s[blk].statback );
				gotoxy( s[blk].field_col, cur_row );
				cprintf( s[blk].stat_value[index] );
				/* make the plain text on the row above highlighted...         */
				/* field text...                                               */
				textcolor( s[blk].txtcolor );
				textbackground( s[blk].backcolor );
				gotoxy( s[blk].start_col, newrow );
				cprintf( s[blk].stat_items[index - 1] );
				/* field value...                                              */
				textcolor( s[blk].statfore );
				textbackground( s[blk].statback );
				gotoxy( s[blk].field_col, newrow );
				cprintf( s[blk].stat_value[index - 1] );
				/* put the cursor at the new row...                            */
				gotoxy( s[blk].start_col, newrow );
			}  /* end if */
			else if( cur_row == s[blk].start_row && blockno > 1 )
			{
				/* make the current highlighted row regular...                 */
				index = ( cur_row - s[blk].start_row ) / s[blk].delta_row;
				/* field text...                                               */
				textcolor( s[blk].txtcolor );
				textbackground( BLACK );
				gotoxy( s[blk].start_col, cur_row );
				cprintf( s[blk].stat_items[index] );
				/* field value...                                              */
				textcolor( s[blk].statfore );
				textbackground( s[blk].statback );
				gotoxy( s[blk].field_col, cur_row );
				cprintf( s[blk].stat_value[index] );
				/* make the bottom row on the previous block highlighted...    */
				if( (--blockno) == 0 )
					blockno = cfghlp.num_blocks;
				blk = blockno - 1;
				/* field text...                                               */
				textcolor( s[blk].txtcolor );
				textbackground( s[blk].backcolor );
				row = s[blk].start_row +
											(s[blk].num_fields - 1) * s[blk].delta_row;
				gotoxy( s[blk].start_col, row );
				cprintf( s[blk].stat_items[s[blk].num_fields - 1] );
				/* field value...                                              */
				textcolor( s[blk].statfore );
				textbackground( s[blk].statback );
				gotoxy( s[blk].field_col, row );
				cprintf( s[blk].stat_value[s[blk].num_fields - 1] );
				gotoxy( s[blk].start_col, row );
			}  /* end else-if */
			break;
		case DOWN_ARROW:
			bot = s[blk].start_row + (s[blk].num_fields - 1) * s[blk].delta_row;
			if( cur_row < bot )
			{
				newrow = cur_row + s[blk].delta_row;
				/* calculate the character string index corresponding to
					the current row...                                          */
				index = ( cur_row - s[blk].start_row ) / s[blk].delta_row;
				/* make the highlighted text on the current row plain...       */
				/* field text...                                               */
				textcolor( s[blk].txtcolor );
				textbackground( BLACK );
				gotoxy( s[blk].start_col, cur_row );
				cprintf( s[blk].stat_items[index] );
				/* field value...                                              */
				textcolor( s[blk].statfore );
				textbackground( s[blk].statback );
				gotoxy( s[blk].field_col, cur_row );
				cprintf( s[blk].stat_value[index] );
				/* make the plain text on the row below highlighted...         */
				/* field text...                                               */
				textcolor( s[blk].txtcolor );
				textbackground( s[blk].backcolor );
				gotoxy( s[blk].start_col, newrow );
				cprintf( s[blk].stat_items[index + 1] );
				/* field value...                                              */
				textcolor( s[blk].statfore );
				textbackground( s[blk].statback );
				gotoxy( s[blk].field_col, newrow );
				cprintf( s[blk].stat_value[index + 1] );
				/* put the cursor at the new row...                            */
				gotoxy( s[blk].start_col, newrow );
			}  /* end if */
			else if( cur_row == bot && blockno < cfghlp.num_blocks )
			{
				/* make the current highlighted row regular...                 */
				index = ( cur_row - s[blk].start_row ) / s[blk].delta_row;
				/* field text...                                               */
				textcolor( s[blk].txtcolor );
				textbackground( BLACK );
				gotoxy( s[blk].start_col, cur_row );
				cprintf( s[blk].stat_items[index] );
				/* field value...                                              */
				textcolor( s[blk].statfore );
				textbackground( s[blk].statback );
				gotoxy( s[blk].field_col, cur_row );
				cprintf( s[blk].stat_value[index] );
				/* make the bottom row on the next block highlighted...        */
				if( (++blockno) > cfghlp.num_blocks )
					blockno = 1;
				blk = blockno - 1;
				/* field text...                                               */
				textcolor( s[blk].txtcolor );
				textbackground( s[blk].backcolor );
				gotoxy( s[blk].start_col, s[blk].start_row );
				cprintf( s[blk].stat_items[0] );
				/* field value...                                              */
				textcolor( s[blk].statfore );
				textbackground( s[blk].statback );
				gotoxy( s[blk].field_col, s[blk].start_row );
				cprintf( s[blk].stat_value[0] );
				gotoxy( s[blk].start_col, s[blk].start_row );
			}  /* end else-if */
			break;
		default:
			break;
	}  /* end switch */
}  /* end cursor_move */

/***************************************************************************
Function:      get_cfg_paths
Purpose:       To convert the screen program name from .exe to .txt and .hlp
					so the rest of the program knows the correct paths.
Arguments:     progname
Returns:       none
****************************************************************************/
static void get_cfg_paths( char *sta, char *txthlp )
{
	char  extension[4];

	/* create the block configuration file names using the program name
		and the number of blocks...                                          */
	strcpy( cfghlp.scrn_path, txthlp );
	strcpy( cfghlp.help_path, txthlp );
	strcpy( cfghlp.stat_path, sta );

	/* convert the screen program name from .xxx to .txt...                 */
	strcpy( extension, "txt" );
	change_ext( cfghlp.scrn_path, extension );

	/* convert the screen program name from .xxx to .hlp...                 */
	strcpy( extension, "hlp" );
	change_ext( cfghlp.help_path, extension );

	/* convert the ".cmd" file name passed from usermenu-type program to a
		status file ".sta" created by pc2modem.exe...                        */
	strcpy( extension, "sta" );
	change_ext( cfghlp.stat_path, extension );
}  /* end get_cfg_paths */

/***************************************************************************
Function:      modem_status
Purpose:       to put the modem status on the screen
Arguments:     none
Returns:       1 if successful, 0 if fail
****************************************************************************/
static int modem_status( void )
{
	int      i, j, found4field, blk, field, start_index, check;
	int      found[MAXBLOCKS][MAXITEMS];
	char     txt[MAXSTR], status[MAXSTR], command[MAXSTR];
	FILE     *sf;
	struct   text_info scrn;

	/* get the text screen information...                                   */
	gettextinfo( &scrn );

	/* open the status file for read...                                     */
	if( (sf = getfp( cfghlp.stat_path, "r" )) == NULL )
		return( FAIL );

	/* initialize the found array.  This array contains 1 for the fields that
		have been found, or a 0 for the ones that have not...                */
	for( i = 0; i < cfghlp.num_blocks; ++i )
		for( j = 0; j < s[i].num_fields; ++j )
			found[i][j] = NO;

	/* go through the file line by line and for each line search for a match
		in command in the screen fields...                                   */
	while( !feof( sf ) )
	{
		fgets( txt, MAXSTR, sf );
		if( !strcmp( txt, ">1/REM_\n" ) || !strcmp( txt, "\n" ) )
			check = NO;
		else
			check = YES;
		if( check )
		{
			switch( txt[0] )
			{
				case '>':
					start_index = 3;
					break;
				case '\n':
					check = NO;
					break;
				default:
					start_index = 0;
					break;
			}  /* end switch */

			/* get the command portion of the status response...              */
			i = start_index;
			while( txt[i] != '_' && txt[i] != '\n' )
				command[i - start_index] = txt[i++];
			command[i - start_index] = '\0';
			++i;  /* to skip the '_' in the modem response                    */

		/* look for the command in the field command strings...              */
			for( blk = 0; blk < cfghlp.num_blocks; ++blk )
			{
				found4field = NO;
				for( field = 0; field < s[blk].num_fields; ++field )
					if( !strcmp( command, s[blk].stat_cmnds[field] ) &&
																			!found[blk][field] )
					{
						/* then get the status portion of the modem response...  */
						j = 0;
						while( txt[i] != '\n' )
							status[j++] = txt[i++];
						status[j] = '\0';
						/* put the status on the screen...                       */
						textcolor( s[blk].statfore );
						textbackground( s[blk].statback );
						gotoxy( s[blk].field_col, s[blk].start_row +
																	field * s[blk].delta_row );
						strcpy( s[blk].stat_value[field], status );
						cprintf( status );
						found[blk][field] = YES;
						found4field = YES;
						break;
					}  /* end if */
				if( found4field )
					break;
			}  /* end for */
		}  /* end if */
	}  /* end while */

	fclose( sf );
	restore_scrn( scrn );
	return( OK );
}  /* end modem_status */

/***************************************************************************
Function:      help_prompt
Purpose:       puts a popup help prompt on the screen, or removes it
Arguments:     On/Off
Returns:       none
****************************************************************************/
static void help_prompt( int state )
{
	int      i, j, difflen, stripspot;
	char     *lfcr = "\n\r", *lfcrsp = "\n\r ";
	struct   text_info orig;

	/* get the original screen configuration...                             */
	gettextinfo( &orig );
	switch( state )
	{
		case ON:
			/* set the colors...                                              */
			textcolor( MENUFORE );
			textbackground( MENUBACK );
			stpr = PWALLOC;
			stpr -> top = 10;
			stpr -> lft = 5;
			stpr -> numrows = cfghlp.num_help + 1;
			/* NOTE:  the cfgstr is used to fake the popwin function into thinking
				that there is only one text string.  It then makes the window
				only wide enough to accomodate this string.  Then, the number of
				rows is adjusted here to make the window the right size        */
			strcpy( stpr -> cfgstr, cfghlp.help_items[cfghlp.bigind] );
			strcat( stpr -> cfgstr, lfcr );
			/* create a single string for the popup window...                 */
			strcpy( stpr -> mes, "" );
			for( i = 0; i < cfghlp.num_help; ++i )
			{
				strcat( stpr -> mes, cfghlp.help_items[i] );
				/* add spaces to lines to lengthen them to longest...          */
				difflen = cfghlp.biglen - (int)strlen( cfghlp.help_items[i] );
				for( j = 0; j < difflen; ++j )
					strcat( stpr -> mes, " " );
				strcat( stpr -> mes, lfcrsp );
			}
			strcat( stpr -> mes, "         " );
			strcat( stpr -> mes, version );
			strcat( stpr -> mes, lfcrsp );
			/* strip the last cf-lf combination and make it a cr only...      */
			stripspot = (int)( strlen( stpr -> mes) - strlen( lfcrsp ) );
			stpr -> mes[stripspot] = '\0';
			strcat( stpr -> mes, "\r" );
			/* put the help window on the screen...                           */
			stpr = popwin( stpr, ON );
			restore_scrn( orig );
			break;
		case OFF:
			if( stpr != NULL )
			{
				stpr = popwin( stpr, OFF );
				free( stpr );
			}
			break;
	}  /* end switch */
}  /* end help_prompt */

/***************************************************************************
Function:      f1tof10
Purpose:       service user input if F1-F10 is pressed
Arguments:     extended key pressed
Returns:       1 if all ok; 0 if anything failed
****************************************************************************/
static int f1tof10( int extkey )
{
	int      field, cur_row, blk = blockno - 1;
	HELPINFO help;

	cur_row = wherey();
	switch( extkey )
	{
		case F1:    /* activate popup help windows...                        */
			field = (cur_row - s[blk].start_row) / s[blk].delta_row + 1;
			help.block     = blockno;
			help.field     = field;
			help.fp        = stathelp;
			help.numfields = s[blk].num_fields;
			strcpy( help.opttxt, s[blk].stat_items[field - 1] );
			if( popup_help( help ) == FAIL )
				return( FAIL );
			break;
		case F2:    /* refresh the status...                                 */
			if( modem_status() == FAIL )
				return( FAIL );
			break;
		case F7:
			if( monitor_values( "ebno" ) == FAIL )
				return( FAIL );
			break;
		case F8:
			if( monitor_values( "rxsignal" ) == FAIL )
				return( FAIL );
			break;
		default:
			break;
	}  /* end switch */
	return( OK );
}  /* end f1tof10 */