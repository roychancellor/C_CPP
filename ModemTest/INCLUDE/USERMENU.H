/***************************************************************************
Include File:     usermenu.h
Purpose:          defines the coordinates for a usermenu
Written By:       Roy Chancellor
Last Updated:     05-11-1994
Copyright (c), 1994, EF Data Corporation.
****************************************************************************/
#define INPUT_DELAY  350   /* delay for user to enter a second key (ms)    */
#define DEFAULTBLOCK 0     /* value for default block # in hlp file        */
#define MAXOPTS      23    /* maximum number of menu items                 */
#define MAX_PATH     80    /* maximum length of a file path                */

#define MENU_TOP     1     /* row at top of border                         */
#define MENU_BOT     24    /* row at bottom of border                      */
#define MENU_LFT     1     /* column at left of border                     */
#define MENU_RYT     79    /* column at right of border                    */

#define MENU_PROMPT_FORE_COLOR   YELLOW
#define MENU_PROMPT_BACK_COLOR   LIGHTBLUE
#define MENU_UTILITY_FORE_COLOR  YELLOW
#define MENU_UTILITY_BACK_COLOR  LIGHTBLUE
#define MENU_TEXT_COLOR          YELLOW
#define MENU_HIGHLIGHT_COLOR     CYAN
#define MENU_BORDER_FORE_COLOR   CYAN
#define MENU_BORDER_BACK_COLOR   BLACK
#define MENU_TITLE_FORE_COLOR    YELLOW
#define MENU_TITLE_BACK_COLOR    CYAN
#define DEFAULT_FORE_COLOR       WHITE
#define DEFAULT_BACK_COLOR       BLACK

#define FULL   0
#define SUB    1

/* define a type of structure that holds the pertinent info. for a menu... */
typedef struct
{
	int   start_row;
	int   start_col;
	int   delta_row;
	int   hylyt_row;
	int   txtcolor;
	int   backcolor;
	int   borderfore;
	int   borderback;
	int   num_fields;
	int   num_help;
	int   biglen;
	int   bigind;
	int   show_status;
	int   cur_field;
	char  menu_items[MAXOPTS][MAXSTR];
	char  opt_select[MAXOPTS + 1];
	char  test_path[MAXOPTS][MAXSTR];
	char  field_type[MAXOPTS][MAXSTR];
	char  menu_path[MAX_PATH];
	char  help_path[MAX_PATH];
	char  help_items[MAXOPTS][MAXSTR];
} MENU;
int   usermenu( char *, char * );
