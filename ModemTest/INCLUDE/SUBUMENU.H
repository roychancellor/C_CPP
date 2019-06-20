/***************************************************************************
Include File:		subumenu.h
Purpose:				defines the coordinates for a usermenu
Written By:			Roy Chancellor
Last Updated:		02-25-1994
Copyright (c), 1994, EF Data Corporation.
****************************************************************************/
#define INPUT_DELAY	350	/* delay for user to enter a second key (ms)		*/
#define BOTFORE		YELLOW		/* for bottom line prompt/menu				*/
#define BOTBACK		LIGHTBLUE 	/* for bottom prompt/menu        			*/
#define DEFAULTBLOCK	0				/* value for default block # in hlp file	*/
#define MAXOPTS		23		/* maximum number of menu items						*/
#define SUBMENU_TOP		5		/* row at top of border									*/
#define SUBMENU_BOT		18		/* row at bottom of border								*/
#define SUBMENU_LFT		5		/* column at left of border							*/
#define SUBMENU_RYT		70		/* column at right of border							*/
#define MAX_PATH		80		/* maximum length of a file path						*/

/* define a type of structure that holds the pertinent info. for a menu...	*/
typedef struct
{
	int	start_row;
	int	start_col;
	int	delta_row;
	int	hylyt_row;
	int 	txtcolor;
	int	backcolor;
	int	borderfore;
	int	borderback;
	int	num_fields;
	int	num_help;
	int	biglen;
	int	bigind;
	int	show_status;
	char	menu_items[MAXOPTS][MAXSTR];
	char	opt_select[MAXOPTS + 1];
	char  test_path[MAXOPTS][MAXSTR];
	char  field_type[MAXOPTS][MAXSTR];
	char	menu_path[MAX_PATH];
	char 	help_path[MAX_PATH];
	char	help_items[MAXOPTS][MAXSTR];
} SUBMENU;

int subumenu( char * );