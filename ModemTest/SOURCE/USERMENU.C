/***************************************************************************
*  Module:        USERMENU.C                                               *
*                                                                          *
*  Created By:    Roy Chancellor                                           *
*                                                                          *
*  Date Created:  01-17-1994                                               *
*                                                                          *
*  Date Modified: 05-10-1994                                               *
*                                                                          *
*  Version:       1.00                                                     *
*                                                                          *
*  Date Released: 05-10-1994                                               *
*                                                                          *
*  Description:   Creates a menu of fields on the screen.                  *
*                                                                          *
*  Inputs:        None                                                     *
*                                                                          *
*  Returns:       None                                                     *
*                                                                          *
*  Date           Modification                              Initials       *
*  --------------------------------------------------------------------    *
*  03-08-1994  Released software for use as version 0.00             RSC   *
*  03-11-1994  modified statscrn operation; ver to 0.01              RSC   *
*  03-16-1994  changed F5 to ALT-F5; ver to 0.02                     RSC   *
*  03-23-1994  added terminal f'n to ALT-F6; ver to 0.03             RSC   *
*  03-24-1994  added change comm config. function ALT-F1; ver: 0.04  RSC   *
*  03-29-1994  changed version display to test version, not the      RSC   *
*              version of USERMENU.  Ver. to 0.05.                         *
*  03-29-1994  added restore_scrn after a "NO TEST"; ver to 0.06     RSC   *
*  04-01-1994  added monitor ADC & I2C on F9; ver to 0.07.           RSC   *
*  04-04-1994  added password protection to ALT-F1; ver to 0.08      RSC   *
*  04-06-1994  changed ALT-C & ALT-D in process_response(); changed  RSC   *
*              return type in opts_to_scrn to int; FIXED BUG:  while       *
*              reading in menu options, the title didn't have a '~' on     *
*              it, but the routine had a "while( txt[i] != '~' )" so       *
*              memory locations were getting written over; ver to 0.09     *
*  04-15-1994  Added a comments feature for menu config. files.      RSC   *
*              Put all colors in the include file; ver to 0.10             *
*	04-26-1994	Added a clrscr() before going to comm. config menu;	RSC	*
*					ver to 0.11																	*
*	05-10-1994	RELEASED SOFTWARE FOR USE ON A CAC AS VERSION 1.00		RSC	*
****************************************************************************/
/* Include files that are part of Turbo C...                               */
#include <stdio.h>         /* for all C programs									*/
#include <conio.h>         /* for screen calls										*/
#include <stdlib.h>        /* for calls to many functions						*/
#include <process.h>       /* for calls to exit										*/
#include <string.h>        /* for calls to strcmp, strcpy, strcat				*/
#include <dos.h>           /* for use of stklen										*/
#include <alloc.h>         /* for calls to calloc									*/
#include <dir.h>           /* for calls to getcwd									*/
#include <errno.h>         /* for use of errno variable							*/

/* Include files that are custom written for the EF Data Test Project...   */
#include <boolean.h>       /* for values that take on value of 1 or 0		*/
#include <ascii.h>         /* for key scan codes									*/
#include <popphelp.h>      /* for popup help window coordinates				*/
#include <utility.h>       /* for calls to getfp, str2int, strip_lf			*/
#include <popwin.h>        /* for calls to popwin									*/
#include <pc2modem.h>      /* for calls to pc2modem								*/
#include <statscrn.h>      /* for the status screen window coordinates		*/
#include <subumenu.h>      /* for information about making a sub menu		*/
#include <usermenu.h>      /* for definitions of menu coordinates				*/
#include <monvalue.h>      /* for calls to monitor_values						*/
#include <commcnfg.h>      /* for use of configure_communication()			*/

/* Function prototypes LOCAL to USERMENU...                                */
int         terminal( void );    /* declared in terminal.c                 */
static int  menu_from_disk( void );
static int  process_response( char *, PROGSTAT *, char * );
static int  opts_to_scrn( char * );
static void create_border( char *, char *, char *, char *, char *, char *,
									char * );
static int  opt_to_int( char * );
static int  cursor_move( int );
static void getresp( char * );
static int  get_cfg_paths( char * );
static int  run_prog( char *, char * );
static int  f1tof10( int, char * );
static void bottom_prompt( char * );
static void change_hylyt( int, char * );

/* change the stack size from default of 4K...                             */
extern unsigned _stklen = 10000;

/* GLOBAL variables...                                                     */
static MENU m[2];          /* holds all menu information                   */
									/* type defined in usermenu.h							*/
static FILE *menuhelp[2];  /* file pointers for help files                 */
static int  *menubuf[2];   /* buffer to hold image of menu where status
										goes                                         */
static int  mtf;           /* flag which controls which type of menu is on	*/
									/*	the screen:  FULL or SUB							*/
static int	comm_type[2][MAXOPTS];	/* the communications type:
													RS232 or RS485								*/

/***************************************************************************
Function:      main
Purpose:       to call usermenu
Arguments:     number of command line arguments, command line arguments.
Returns:       none
****************************************************************************/
void main( int argc, char *argv[] )
{
	mtf = FULL;
	if( argc < 2 )
	{
		printf( "\n\n SYNTAX:  usermenu [txt/hlp filename].\n" );
		printf( "\n\n Press any key to end and return to the operating sys." );
		getch();
		exit( FAIL );
	}
	if( usermenu( argv[1], "full" ) == FAIL )
		exit( FAIL );
}  /* end main */

/***************************************************************************
Function:      usermenu
Purpose:       to control all aspects of modem testing
Arguments:     TXT/HLP filename, type of menu:  "full" or "sub"
Returns:       1 if ok; 0 if anything failed
****************************************************************************/
int usermenu( char *argv, char *type )
{
	int      sub_menu_size;
	PROGSTAT pgst;
	char     user_response[3], *sub_menu_buf;
	struct   text_info tmp, orig;

	/* get the required menu configuration and help file paths...           */
	if( get_cfg_paths( argv ) == FAIL )
		return( FAIL );

	/* get the menu configuration from the menu configuration file...			*/
	if( menu_from_disk() == FAIL )
		return( FAIL );

	/* open the menu help file (procedure file)...                     		*/
	menuhelp[mtf] = NULL;
	if( (menuhelp[mtf] = getfp( m[mtf].help_path, "r" )) == NULL )
		return( FAIL );

	/* get original text screen configuration...                            */
	gettextinfo( &orig );

	/* initialize variables...                                              */
	pgst.redraw       = NO;
	pgst.keep_going   = YES;
	pgst.first_time   = YES;

	/* colors for the menu (defined in usermenu.h)...                       */
	m[mtf].hylyt_row  = m[mtf].start_row;
	m[mtf].txtcolor   = MENU_TEXT_COLOR;
	m[mtf].backcolor  = MENU_HIGHLIGHT_COLOR;
	m[mtf].borderfore = MENU_BORDER_FORE_COLOR;
	m[mtf].borderback = MENU_BORDER_BACK_COLOR;

	if( !strcmp( type, "sub" ) )
	{
		/* take a picture of the screen space where the sub menu will go...  */
		sub_menu_size = (SUBMENU_RYT - SUBMENU_LFT + 2) *
														(SUBMENU_BOT - SUBMENU_TOP + 2);
		if( (sub_menu_buf =
				calloc( (size_t)sub_menu_size, (size_t)sizeof( int ) )) == NULL )
		{
			clrscr();
			printf( "\n\n Not enough memory to allocate a buffer for\n" );
			printf( " storing an image of the main menu.  Press any key..." );
			getch();
			return( FAIL );
		}
		if( !gettext( SUBMENU_LFT, SUBMENU_TOP, SUBMENU_RYT + 1,
															SUBMENU_BOT + 1, sub_menu_buf ) )
		{
			clrscr();
			printf( "\n\n Gettext failed when trying to take a picture of the" );
			printf( " screen\n where the SUB menu goes.  Press any key..." );
			getch();
			return( FAIL );
		}
	}  /* end if */

	/* put menu fields on the screen (set the window size first)...			*/
	gettextinfo( &tmp );
	if( !strcmp( type, "full" ) )
		window( 1, 1, tmp.screenwidth, tmp.screenheight );
	else if( !strcmp( type, "sub" ) )
		window( SUBMENU_LFT, SUBMENU_TOP, SUBMENU_RYT + 1, SUBMENU_BOT + 1 );
	if( opts_to_scrn( type ) == FAIL )
		return( FAIL );

	/* take a "picture" of the menu where the status screen goes...         */
	menubuf[mtf] = calloc( (size_t)(STAT_BOT + 1 - STAT_TOP) * 2 *
										(STAT_RYT - STAT_LFT), (size_t)sizeof( int ) );
	if( menubuf[mtf] == NULL )
	{
		clrscr();
		printf( "\n\n Not enough memory to get picture of menu." );
		printf( "\n\n Press any key to end..." );
		getch();
		return( FAIL );
	}
	/* get the original screen configuration...										*/
	gettextinfo( &tmp );

	/* temporarily change the highlighted field to normal...               	*/
	textcolor( MENU_TEXT_COLOR );
	textbackground( MENU_BORDER_BACK_COLOR );
	gotoxy( tmp.curx, tmp.cury );
	cprintf( m[mtf].menu_items[m[mtf].hylyt_row - tmp.cury] );
	gotoxy( tmp.curx, tmp.cury );

	/* take the picture...                                                  */
	if( !gettext( STAT_LFT, STAT_TOP, STAT_RYT, STAT_BOT + 1, menubuf[mtf] ) )
	{
		clrscr();
		printf( "\n\n Gettext failed when trying to take a picture of the" );
		printf( " screen\n where the STATUS screen goes.  Press any key..." );
		getch();
		return( FAIL );
	}

	/* reset the highlighted row...                                         */
	textcolor( MENU_TEXT_COLOR );
	textbackground( MENU_HIGHLIGHT_COLOR );
	gotoxy( tmp.curx, tmp.cury );
	cprintf( m[mtf].menu_items[m[mtf].hylyt_row - tmp.cury] );
	gotoxy( tmp.curx, tmp.cury );

/****************************** MAIN LOOP **********************************/
/* keep showing the menu until the user enters the exit command...         */
	while( pgst.keep_going )
	{
		/* redraw gets set to YES in process_response and must
			be set back to NO before returning to user_response               */
		pgst.redraw = NO;

		if( !pgst.first_time )
			opts_to_scrn( type );
		pgst.first_time = NO;

		while( pgst.redraw == NO )
		{
			/* wait for the user's response...                                */
			getresp( user_response );
			/* process the user's response...                                 */
			if( process_response( user_response, &pgst, type ) == FAIL )
				break;
		}  /* end while */
	}  /* end while */
/*********************END MAIN LOOP*****************************************/

	/* set the text colors to the defaults...                               */
	textcolor( DEFAULT_FORE_COLOR );
	textbackground( DEFAULT_BACK_COLOR );

	/* clear the screen before exiting back to DOS...                       */
	clrscr();
	if( fclose( menuhelp[mtf] ) == EOF )
	{
		printf( "\n  \n fclose returned EOF in usermenu 1\n  \n" );
		getch();
	}
	free( menubuf[mtf] );

	if( !strcmp( type, "sub" ) )
	{
		/* put the main menu text back on the screen...                      */
		puttext( SUBMENU_LFT, SUBMENU_TOP, SUBMENU_RYT + 1, SUBMENU_BOT + 1,
																					sub_menu_buf );
		free( sub_menu_buf );

		/* return the screen to its original configuration...                */
		restore_scrn( orig );
	}  /* end if */

	return( OK );
}  /* end main */

/***************************************************************************
Function:      menu_from_disk
Purpose:       obtain the menu configuration and command file paths
Arguments:     none
Returns:       1 if file exists; 0 if it does not
****************************************************************************/
static int menu_from_disk( void )
{
	int      i, j, k, templen;
	char     txt[MAXSTR], tmp[10];
	FILE     *fp;

	/* Open the menu configuration file for read...                         */
	if( (fp = getfp( m[mtf].menu_path, "r" )) == NULL )
		return( FAIL );

	/* get the number of fields, start row, start column and row step...    */
	m[mtf].num_fields    = str2int( fgets( txt, MAXSTR, fp ) );
	m[mtf].start_row     = str2int( fgets( txt, MAXSTR, fp ) );
	m[mtf].start_col     = str2int( fgets( txt, MAXSTR, fp ) );
	m[mtf].delta_row     = str2int( fgets( txt, MAXSTR, fp ) );

	/* get the menu text strings, field types, and comm. modes...           */
	for( i = 0; i <= m[mtf].num_fields; ++i )
	{
		fgets( txt, MAXSTR, fp );

		/* separate the field type from the menu text...							*/
		j = 0;
		while( txt[j] != '~' )
			m[mtf].menu_items[i][j] = txt[j++];
		m[mtf].menu_items[i][j] = '\0';
		k = 0;

		++j;  /* skip the ~ in the string                                    */

		/* get the field type...															*/
		while( txt[j] != '~' )
			m[mtf].field_type[i][k++] = txt[j++];
		m[mtf].field_type[i][k] = '\0';

		++j;  /* skip the '~' in the string												*/

		/* get the communications mode...												*/
		k = 0;
		while( txt[j] != '\n' )
			tmp[k++] = txt[j++];
		tmp[k] = '\0';
		comm_type[mtf][i] = atoi( tmp );
	}  /* end for */

	/* get the field select string (contains the keys that when pressed
		activate the menu fields)...                                         */
	fgets( m[mtf].opt_select, MAXOPTS, fp );

	/* get the paths for the programs that correspond to menu fields...     */
	for( i = 0; i < m[mtf].num_fields; ++i )
	{
		fgets( m[mtf].test_path[i], MAXSTR, fp );
		strip_lf( m[mtf].test_path[i] );
	}

	/* get the number of popup utility...                           			*/
	fgets( txt, MAXSTR, fp );
	m[mtf].num_help = str2int( txt );

	/* get the help strings and determine the length of the longest one...  */
	m[mtf].biglen = 0;
	m[mtf].bigind = 0;
	for( i = 0; i < m[mtf].num_help; ++i )
	{
		fgets( m[mtf].help_items[i], MAXSTR, fp );
		if( (templen = (int)strlen( m[mtf].help_items[i] )) >= m[mtf].biglen )
		{
			m[mtf].biglen = templen;
			m[mtf].bigind = i;
		}
		strip_lf( m[mtf].help_items[i] );
	}  /* end for */

	if( fclose( fp ) == EOF )
	{
		printf( "\n  \n fclose returned EOF in usermenu 2\n  \n" );
		getch();
	}
	fp = NULL;

	return( OK );
}  /* end menu_from_disk */

/***************************************************************************
Function:   process_response
Purpose:    act on a user input from the menu of fields.
Arguments:  user_response, ptr to PROGSTAT, menu type:  "full" or "sub".
Returns:    0 if run_prog fails or if f1tof10 fails
****************************************************************************/
static int process_response( char user_response[], PROGSTAT *pgst, char *type )
{
	int      extkey, field, change, selected_row;
	char     prog_type[MAXSTR], *errmes;
	struct   text_info orig;

	/* get the current screen information...                                */
	gettextinfo( &orig );

	switch( user_response[0] )
	{
		case ENTER:
			field    = ( orig.cury - m[mtf].start_row ) / m[mtf].delta_row + 1;
			change   = 0;
			m[mtf].cur_field = field + change;
			if( field == m[mtf].num_fields )
				field = ESC;
			break;
		case ESC:
			field = user_response[0];
			break;
		case EXTENDED_KEY:
			field  = user_response[0];
			extkey = user_response[1];
			break;
		/* check for TAB and BACKSPACE keys because they are 9 & 8 respectively
			and will cause the program to run if either is pressed				*/
		case TAB:case BACKSPACE:
			field   = 999;
			break;
		default:                   /* the user pressed a number sequence     */
			field = opt_to_int( user_response );
			/* set the background row to correspond to the field selected...  */
			selected_row = m[mtf].start_row + field - 1;
			change = selected_row - orig.cury + 0;
			m[mtf].cur_field = field + change;
			break;
	}  /* end switch */

	/* now act on the user's response...                                    */
	pgst -> redraw = YES;
	switch( field )
	{
		case 1:case 2:case 3:case 4:case 5:case 6:case 7:case 8:case 9:
		case 10:case 11:case 12:case 13:case 14:case 15:case 16:case 17:
		case 18:case 19:case 20:
			if( field < m[mtf].num_fields )
			{
				/* decide if the option requires a popup menu or not...        */
				if( !strcmp( m[mtf].field_type[field - 1], "popup" ) ||
									!strcmp( m[mtf].field_type[field - 1], "POPUP" ) )
					strcpy( prog_type, "popup" );
				else if( !strcmp( m[mtf].field_type[field - 1], "test" ) ||
										!strcmp( m[mtf].field_type[field - 1], "TEST" ) )
					strcpy( prog_type, "test" );
				else if( !strcmp( m[mtf].field_type[field - 1], "no test" ) ||
									!strcmp( m[mtf].field_type[field - 1], "NO TEST" ) )
				{
					strcpy( prog_type, "no test" );
					gotoxy( m[mtf].start_col, selected_row );
				}
				else if( !strcmp( m[mtf].field_type[field - 1], "execute" ) ||
									!strcmp( m[mtf].field_type[field - 1], "EXECUTE" ) )
				{
					strcpy( prog_type, "execute" );
					gotoxy( m[mtf].start_col, selected_row );
				}

				/* finally run the program, or command file, or popup...       */
				if( run_prog( prog_type, m[mtf].test_path[field - 1] ) == FAIL )
					return( FAIL );

				/* change the highlighted row to the appropriate new row...    */
				gotoxy( orig.curx, orig.cury );
				change_hylyt( change, "relative" );
				pgst -> redraw = NO;
				gotoxy( orig.curx, orig.cury + change );
				textattr( orig.attribute );
			}  /* end if */
			break;
		case EXTENDED_KEY:
			pgst -> redraw = NO;
			switch( extkey )
			{
				case F1:case F2:case F3:case F4:case F5:case ALTF5:case F6:
				case F7:case F8:case F9:case F10:case ALTF6:case ALTF1:
					if( f1tof10( extkey, type ) == FAIL )
						return( FAIL );
					break;
				case UP_ARROW:case DOWN_ARROW:
					m[mtf].hylyt_row = cursor_move( extkey );
					break;
				case ALTD:
					textcolor( DEFAULT_FORE_COLOR );
					textbackground( DEFAULT_BACK_COLOR );
					clrscr();
					if( system( "c:\\modems\\password.exe" ) == -1 )
					{
						errmes = strerror( errno );
						printf( "\n\n Error accessing system at %s:  %s",
																	"ALT-D password", errmes );
						printf( "\n\n Hit any key to end..." );
						getch();
						return( FAIL );
					}  /* end if */
					if( system( "command" ) != 0 )
					{
						errmes = strerror( errno );
						printf( "\n\n Error accessing system at %s:  %s",
																	"ALT-D command", errmes );
						printf( "\n\n Hit any key to end..." );
						getch();
						return( FAIL );
					}  /* end if */
					pgst -> redraw = YES;
					restore_scrn( orig );
					break;
				case ALTC:  /* configure modem parameters...                   */
					pgst -> redraw = YES;
					textcolor( DEFAULT_FORE_COLOR );
					textbackground( DEFAULT_BACK_COLOR );
					clrscr();
					if( system( "c:\\modems\\password.exe" ) == -1 )
					{
						errmes = strerror( errno );
						printf( "\n\n Error accessing system at %s:  %s",
																	"ALT-C password", errmes );
						printf( "\n\n Hit any key to end..." );
						getch();
						return( FAIL );
					}  /* end if */
					if( configure_communications() == FAIL )
						return( FAIL );
					restore_scrn( orig );
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
Purpose:    put menu fields on screen.
Arguments:  the menu type:  "full" or "sub"
Returns:    1 if ok; 0 if anything fails
****************************************************************************/
static int opts_to_scrn( char *type )
{
	int   i, row, col, titlelen;
	char  the_title[81], test_ver[MAXSTR];

	/* set the textcolors to their defaults before clearing the screen...   */
	textcolor( DEFAULT_FORE_COLOR );
	textbackground( DEFAULT_BACK_COLOR );

	/* clear the screen to begin with...                                    */
	clrscr();

	/* create the border...                                                 */
	textcolor( MENU_BORDER_FORE_COLOR );
	textbackground( MENU_BORDER_BACK_COLOR );
	create_border( "Í", "º", "É", "È", "»", "¼", type );
	/*             t/b  l/r   rt   lb   rt   rb                             */

	/* set the text color for the title...                                  */
	textcolor( MENU_TITLE_FORE_COLOR );
	textbackground( MENU_TITLE_BACK_COLOR );

	/* center the title on the screen...                                    */
	strip_lf( m[mtf].menu_items[m[mtf].num_fields] );
	strcpy( the_title, m[mtf].menu_items[m[mtf].num_fields] );
	if( get_test_version( test_ver ) == FAIL )
		return( FAIL );
	strcat( the_title, test_ver );
	titlelen = (int)strlen( the_title );
	if( !strcmp( type, "sub" ) )
	{
		col = (SUBMENU_RYT - SUBMENU_LFT - titlelen) / 2;
		if( col <= 0 )
			col = 1;
		gotoxy( col, 1 );
	}
	else if( !strcmp( type, "full" ) )
		gotoxy( (80 - titlelen) / 2, 1 );
	cprintf( the_title );

	/* put the bottom prompt on the screen...                               */
	bottom_prompt( type );

	/* place the fields on the screen by indexing through the strings
		in the menu_items character string array...                          */
	row = m[mtf].start_row;
	textcolor( MENU_TEXT_COLOR );
	for( i = 0; i < m[mtf].num_fields; ++i )
	{
		gotoxy( m[mtf].start_col, row );
		/* want to make a particular instruction have a different bkgd...    */
		if( row != m[mtf].hylyt_row )
			textbackground( MENU_BORDER_BACK_COLOR );
		else
			textbackground( MENU_HIGHLIGHT_COLOR );
		cprintf( m[mtf].menu_items[i] );
		row += m[mtf].delta_row;
	}  /* end for */

	/* set the cursor to the highlighted row...                             */
	gotoxy( m[mtf].start_col, m[mtf].hylyt_row );

	return( OK );
}  /* end opts_to_scrn */

/*****************************************************************************
Function:   create_border
Purpose:    create a border around the perimeter of the screen using the
				character passed in the arguments.
Called:     from opts_to_scrn
Arguments:  horizontal character, vertical character, corner characters, type
Returns:    none
*****************************************************************************/
static void create_border( char *horizchar, char *vertchar, char *lt, char *lb,
						  char *rt, char *rb, char *type )
{
	int   row, col, colstart, colrange, colincr, rowstart, rowrange, rowincr;
	char  bordstr[MENU_RYT - MENU_LFT + 1];

	if( !strcmp( type, "sub" ) )
	{
		colstart = 1;
		colrange = SUBMENU_RYT - SUBMENU_LFT + 1;
		colincr  = colrange - colstart;
		rowstart = 1;
		rowrange = SUBMENU_BOT - SUBMENU_TOP + 1;
		rowincr  = rowrange - rowstart;
		for( col = 0; col < colrange - 2; ++col )
			bordstr[col] = horizchar[0];
		bordstr[col] = '\0';
	}  /* end if */
	else if( !strcmp( type, "full" ) )
	{
		colstart = MENU_LFT;
		colrange = MENU_RYT;
		colincr  = MENU_RYT - MENU_LFT;
		rowstart = MENU_TOP;
		rowrange = MENU_BOT;
		rowincr  = MENU_BOT - MENU_TOP;
		for( col = 0; col <= colincr - 1; ++col )
			bordstr[col] = horizchar[0];
		bordstr[col] = '\0';
	}
	/* create HORIZONTAL lines at TOP and BOTTOM of screen...               */
	/* NOTE:  only put enough characters to go to 1 space before border end */
	for( row = rowstart; row <= rowrange; row += rowincr )
	{
		gotoxy( colstart + 1, row );
		cprintf( bordstr );
	}

	/* create VERTICAL lines at LEFT and RIGHT of screen...                 */
	for( col = colstart; col <= colrange; col += colincr )
		for( row = colstart + 1; row <= rowincr; ++row )
		{
			gotoxy( col, row );
			cprintf( vertchar );
		}

	/* put the corners on last...                                           */
	gotoxy( colstart, rowstart );
	cprintf( lt );
	gotoxy( colstart, rowrange );
	cprintf( lb );
	gotoxy( colrange, rowstart );
	cprintf( rt );
	gotoxy( colrange, rowrange );
	cprintf( rb );
}  /* end create_border */

/**************************************************************************
Function:      opt_to_int
Purpose:       converts user response from a field string to an integer.
Arguments:     user_response
Returns:       field (int)
***************************************************************************/
static int opt_to_int( char *user_response )
{
	int   i, check_index, field;

	field = user_response[0];

	/* structure of user_response as returned from getresp():
		user_response[0]:  first key hit
		user_response[1]:  second key hit or null character
		user_response[2]:  null character                                    */

	if( user_response[1] == '\0' )  /* only one key was hit                 */
		check_index = 0;
	else if( user_response[0] == '1' || user_response[0] == '2' )
		check_index = 1;
	/* check for each key in option select string...                        */
	for( i = 0; i < 10; ++i )
		if( user_response[check_index] == m[mtf].opt_select[i] )
		{
			field = atoi( user_response );
			break;
		}
	return( field );
}  /* end opt_to_int */

/****************************************************************************
Function:      cursor_move
Purpose:       to move the cursor on the menu
Arguments:     extended key
Returns:       the new row
****************************************************************************/
static int cursor_move( int extkey )
{
	int      newrow, botrow;
	struct   text_info orig;

	/* get original screen configuration...                                 */
	gettextinfo( &orig );

	/* act on the extkey...                                                 */
	switch( extkey )
	{
		case UP_ARROW:
			newrow = orig.cury - m[mtf].delta_row;
			if( orig.cury > m[mtf].start_row )
			{
				change_hylyt( -1, "relative" );
				return( newrow );
			}
			break;
		case DOWN_ARROW:
			botrow = m[mtf].start_row + (m[mtf].num_fields - 1) *
																				m[mtf].delta_row;
			newrow = orig.cury + m[mtf].delta_row;
			if( orig.cury < botrow )
			{
				change_hylyt( +1, "relative" );
				return( newrow );
			}
			break;
	}  /* end switch */
	return( OK );
}  /* end cursor_move */

/***************************************************************************
Function:      getresp
Purpose:       To obtain a user response to menu fields; Handles the case
					when the field is more than one digit.
Arguments:     user_resp (character array)
Returns:       none.
****************************************************************************/
static void getresp( char *user_resp )
{
	/* get the user's keystroke...                                          */
	user_resp[0] = getch();
	switch( user_resp[0] )
	{
		case ESC:
			user_resp[1] = '\0';
			user_resp[2] = '\0';
			break;
		case EXTENDED_KEY:
			user_resp[1] = getch();
			user_resp[2] = '\0';
			break;
		default:
			/* after a small delay, see if the user pressed another key...    */
			delay( INPUT_DELAY );
			if( kbhit() )
				user_resp[1] = getch();
			else
				user_resp[1] = '\0';
			/* terminate the response string with a null character...         */
			user_resp[2] = '\0';
			break;
	}  /* end switch */
}  /* end getresp */

/***************************************************************************
Function:      get_cfg_paths
Purpose:       To convert the menu program name from .exe to .txt and .hlp
					so the rest of the program knows the correct paths.
Arguments:     progname
Returns:       1 if directory exists; 0 if not
****************************************************************************/
static int get_cfg_paths( char *progname )
{
	char  extension[4];

	/* copy the program name to the menu config name and help file name...  */
	strcpy( m[mtf].menu_path, progname );
	strcpy( m[mtf].help_path, progname );

	/* convert the menu program name from .XXX to .TXT...                   */
	strcpy( extension, "txt" );
	change_ext( m[mtf].menu_path, extension );

	/* convert the menu program name from .XXX to .HLP...                   */
	strcpy( extension, "hlp" );
	change_ext( m[mtf].help_path, extension );

	return( OK );
}  /* end get_cfg_paths */

/***************************************************************************
Function:      run_prog
Purpose:       To run the test that the user selected.
Arguments:     program type, required files
Returns:       0 if pc2modem fails.
****************************************************************************/
static int run_prog( char *progtype, char *req_file1 )
{
	int 		field;
	char     curpath[MAXPATH];
	struct   text_info now;
	POPWIN   *nt;

	/* get the current text information...												*/
	gettextinfo( &now );
	field = (now.cury - m[mtf].start_row) / m[mtf].delta_row + 1;

	/* get the current working directory...                                 */
	getcwd( curpath, MAXPATH );
	strip_bkslsh( curpath );

	/* now run the test, run pc2modem, or run statscrn...                   */
	if( !strcmp( progtype, "test" ) )
	{
		/* run pc2modem with the command file corresponding to field...      */
		if( pc2modem( req_file1, "", "", CMDFILE, comm_type[mtf][field - 1] )
																						== FAIL )
			return( FAIL );
	}  /* end if */
	else if( !strcmp( progtype, "popup" ) )
	{
		mtf = SUB;
		if( usermenu( req_file1, "sub" ) == FAIL )
		{
			mtf = FULL;
			return( FAIL );
		}
		mtf = FULL;
	}
	else if( !strcmp( progtype, "no test" ) )
	{
		gettextinfo( &now );
		if( (nt = PWALLOC) == NULL )
			printf( "\n  \n NO TEST\n    \n" );
		nt -> lft = 20;
		nt -> top = 10;
		strcpy( nt -> mes, "NO TEST" );
		nt = popwin( nt, ON );
		delay( 750 );
		nt = popwin( nt, OFF );
		free( nt );
		restore_scrn( now );
	}  /* end else-if */
	else if( !strcmp( progtype, "pc2modem" ) )
	{
		if( pc2modem( req_file1, "", "", CMDFILE, comm_type[mtf][field - 1] )
																						== FAIL )
			return( FAIL );
	}
	else if( !strcmp( progtype, "execute" ) )
	{
		errno = _sys_nerr + 1;
		system( req_file1 );
		if( errno <= _sys_nerr )
		{
			textcolor( DEFAULT_FORE_COLOR );
			textbackground( DEFAULT_BACK_COLOR );
			clrscr();
			gotoxy( 10, 10 );
			perror( "Error Accessing system" );
			gotoxy( 10, 12 );
			printf( "Press any key to end..." );
			getch();
			return( FAIL );
		}
	}  /* end else-if */
	return( OK );
}  /* end run_prog */

/***************************************************************************
Function:      f1tof10
Purpose:       service the user when F1-F10 is hit
Arguments:     the key hit, menu type:  "full" or "sub"
Returns:       0 if run_prog fails
****************************************************************************/
static int f1tof10( int whichkey, char *type )
{
	int      field, ll_len, key, i, j, difflen, stripspot;
	char     user_cmdfile[MAXSTR], *lfcr = "\n\r", *lfcrsp = "\n\r ";
	char		req_file1[20], req_file2[20];
	POPWIN   *ucf, *umen;
	struct   text_info orig;
	HELPINFO help;

	gettextinfo( &orig );
	switch( whichkey )
	{
		case F1:    /* activate popup help windows...                        */
			field = (orig.cury - m[mtf].start_row) / m[mtf].delta_row + 1;
			help.fp        = menuhelp[mtf];
			help.field     = field;
			help.numfields = m[mtf].num_fields;
			help.block     = DEFAULTBLOCK;
			strcpy( help.opttxt, m[mtf].menu_items[field - 1] );
			popup_help( help );
			break;
		case F2:    /* put help/utilities menu on screen...                  */
			/* set the colors...                                              */
			textcolor( MENU_UTILITY_FORE_COLOR );
			textbackground( MENU_UTILITY_BACK_COLOR );
			umen = PWALLOC;
			umen -> top = 10;
			umen -> lft = 10;
			umen -> numrows = m[mtf].num_help;
			strcpy( umen -> cfgstr, m[mtf].help_items[m[mtf].bigind] );
			strcat( umen -> cfgstr, lfcr );
			/* create a single string for the popup window...                 */
			strcpy( umen -> mes, "" );
			for( i = 0; i < m[mtf].num_help; ++i )
			{
				strcat( umen -> mes, m[mtf].help_items[i] );
				/* add spaces to lines to lengthen them to longest...          */
				difflen = m[mtf].biglen - (int)strlen( m[mtf].help_items[i] );
				for( j = 0; j < difflen; ++j )
					strcat( umen -> mes, " " );
				strcat( umen -> mes, lfcrsp );
			}  /* end for */

			/* strip the last cf-lf combination and make it a cr only...      */
			stripspot = (int)( strlen( umen -> mes ) - strlen( lfcrsp ) );
			umen -> mes[stripspot] = '\0';
			strcat( umen -> mes, "\r" );

			/* make the window...                                             */
			umen = popwin( umen, ON );

			/* get the user's selection...                                    */
			key  = getch();

			/* turn the menu off...                                           */
			umen = popwin( umen, OFF );
			free( umen );
			window( orig.winleft, orig.wintop, orig.winright, orig.winbottom );
			gotoxy( orig.curx, orig.cury );
			textattr( orig.attribute );

			/* act on the user's selection...                                 */
			if( key == EXTENDED_KEY )
				if( ( key = getch() ) != F2 )
					if( f1tof10( key, type ) == FAIL )
						return( FAIL );  /* recursive call to f1tof10...         */
			break;
		case F4:    /* run the statscrn routine...                           */
			strcpy( req_file1, "modmstat.cmd" );
			if( run_prog( "pc2modem", req_file1 ) == FAIL )
				return( FAIL );
			strcpy( req_file1, "modmstat.sta" );
			strcpy( req_file2, "statscrn.xxx" );
			if( statscrn( "modmstat.sta", "statscrn.xxx" ) == FAIL )
				return( FAIL );
			gotoxy( orig.curx, orig.cury );
			textattr( orig.attribute );
			break;
		case ALTF6:    /* run the terminal routine...                        */
			if( terminal() == FAIL )
				return( FAIL );
			break;
		case ALTF5: /* run the pc2modem program with a special .cmd file...  */
			/* set the colors for the popup window...                         */
			textcolor( DEFAULT_FORE_COLOR );
			textbackground( DEFAULT_BACK_COLOR );

			/* make the popup window...                                       */
			ucf = PWALLOC;
			ucf -> top = 10;
			ucf -> lft = 10;
			ucf -> numrows = 2;

			/* the whole message...                                           */
			sprintf( ucf -> mes,
						"Enter the command file to run.  Must be in default" );
			strcpy( ucf -> cfgstr, ucf -> mes );
			strcat( ucf -> mes, "\n\r directory as (filename.cmd):  " );
			ucf = popwin( ucf, ON );

			/* calculate the length of the last line of the window text...    */
			ll_len = (int)strlen( ucf -> mes ) - (ucf -> ryt - ucf -> lft);
			/* set the cursor at the end of the text and get the .cmd name... */
			gotoxy( ucf -> lft + ll_len, ucf -> top + ucf -> numrows - 1 );
			gets( user_cmdfile );

			/* remove the popup window...                                     */
			ucf = popwin( ucf, OFF );
			free( ucf );

			/* if the user just pressed enter then don't do anything...       */
			if( strcmp( user_cmdfile, "\0" ) != 0 )
			{
				/* ...otherwise run pc2modem with the user-supplied file...    */
				if( run_prog( "pc2modem", user_cmdfile ) == FAIL )
					return( FAIL );
			}  /* end if */

			/* reset the screen to its original configuration...              */
			restore_scrn( orig );
			break;
		case F7:    /* monitor the Eb/N0...                                  */
			if( monitor_values( "ebno" ) == FAIL )
				return( FAIL );
			break;
		case F8:    /* monitor the RX signal level...                        */
			if( monitor_values( "rxsignal" ) == FAIL )
				return( FAIL );
			break;
		case F9:    /* monitor the ADC & I2C values...                       */
			if( monitor_values( "ADC_I2C" ) == FAIL )
				return( FAIL );
			break;
	}  /* end switch */
	return( OK );
}  /* end f1tof10 */

/***************************************************************************
Function:      bottom_prompt
Purpose:       put the bottom prompt on the menu
Arguments:     menu type:  "full" or "sub"
Returns:       none
****************************************************************************/
static void bottom_prompt( char *type )
{
	int   i, centercol;
	char  botline[MAXSTR];
	char  *prompt[] = {  "4",
								"F1-Procedure  ",
								"F2-Help/Utilities  ",
								"%c/%c-Move  ",
								"ENTER/#-Select" };

	/* create the bottom line text...                                       */
	strcpy( botline, "" );
	if( !strcmp( type, "full" ) )
		for( i = 1; i <= atoi( prompt[0] ); ++i )
			strcat( botline, prompt[i] );

	/* put help prompt at bottom of screen...                               */
	textbackground( MENU_PROMPT_BACK_COLOR );
	textcolor( MENU_PROMPT_FORE_COLOR );

	if( !strcmp( type, "full" ) )
	{
		centercol = (80 - (int)strlen( botline ) - 2) / 2;
		if( centercol <= 0 )
			centercol = 1;
		gotoxy( centercol, 24 );
		cprintf( botline, 24 ,25 );
	}  /* end else-if */
}  /* end bottom_prompt */

/***************************************************************************
Function:      change_hylyt
Purpose:       to place the highlight on a different row
Arguments:     new row/change in row; change type:  relative or absolute
Returns:       none
****************************************************************************/
static void change_hylyt( int change, char *type )
{
	int      i;
	struct   text_info orig;

	/* get the current screen configuration...                              */
	gettextinfo( &orig );

	if( !strcmp( type, "relative" ) )
	{
		/* calculate the character string i corresponding to the
			the current row...                                                */
		i = (orig.cury - m[mtf].start_row) / m[mtf].delta_row;
		/* make the highlighted text on the current row plain...             */
		textcolor( m[mtf].txtcolor );
		textbackground( DEFAULT_BACK_COLOR );
		gotoxy( m[mtf].start_col, orig.cury );
		cprintf( m[mtf].menu_items[i] );
		/* make the plain text on the change row highlighted...              */
		textbackground( m[mtf].backcolor );
		gotoxy( m[mtf].start_col, orig.cury + change );
		cprintf( m[mtf].menu_items[i + change] );
		/* put the cursor at the new row...                                  */
		gotoxy( m[mtf].start_col, orig.cury + change );
		textattr( orig.attribute );
	}  /* end if */
	else if( !strcmp( type, "absolute" ) )
	{
	}
}  /* end change_hylyt */