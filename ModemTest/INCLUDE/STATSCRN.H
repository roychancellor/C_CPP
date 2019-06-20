/***************************************************************************
Include File:			statscrn.h
Purpose:					defines the coordinates for the modem status screen.
Written By:				Roy Chancellor
Last Modified:			03-05-1994
Copyright (c) EF Data Corporation.
****************************************************************************/
#include <dir.h>				/* for definition of MAXPATH							*/
#define MAXSTR			100
#define STAT_TOP		1		/* row at top of border									*/
#define STAT_BOT		23		/* row at bottom of border								*/
#define STAT_LFT		41		/* column at left of border							*/
#define STAT_RYT		80		/* column at right of border							*/
#define MAXBLOCKS		4		/* maximum number of blocks on screen				*/
#define MAXITEMS		10		/* maximum number of fields per block				*/
#define HELPMENUFORE	YELLOW		/* text color for help menu					*/
#define HELPMENUBACK	LIGHTBLUE	/* background color for help menu			*/

/* define a data type that holds the pertinent info. for a screen...			*/
typedef struct
{
	int	start_row;
	int	start_col;
	int	field_col;
	int	delta_row;
	int	title_row;
	int	txtcolor;
	int	hylytcolor;
	int	backcolor;
	int	statfore;
	int	statback;
	int	num_fields;
	char	stat_items[MAXITEMS][MAXSTR];
	char	stat_cmnds[MAXITEMS][MAXSTR];
	char	stat_value[MAXITEMS][MAXSTR];
} SCREEN;

/* define a data type that holds the filenames for screen config. and
	screen help information (1 file holds for all blocks)...						*/
typedef struct
{
	int   num_blocks;
	int	num_help;
	int	biglen;
	int 	bigind;
	int	borderfore;
	int	borderback;
	char	help_items[MAXITEMS][MAXSTR];
	char 	scrn_path[MAXPATH];
	char 	help_path[MAXPATH];
	char	stat_path[MAXPATH];
} CFGHELP;

int statscrn( char *, char * );