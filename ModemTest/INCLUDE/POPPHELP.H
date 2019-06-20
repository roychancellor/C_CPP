/***************************************************************************
Include File:		popphelp.h
Purpose:				defines coordinates of a popup help window
Written By:			Roy Chancellor
Last Modified:		03-05-1994
Copyright (c), 1994, EF Data Corporation.
****************************************************************************/
/* define a structure that holds help information...								*/
#include <stdio.h>
#define MAXSTR	100
typedef struct
{
	FILE	*fp;
	int	field;
	int	numfields;
	int	block;
	char	opttxt[MAXSTR];
} HELPINFO;

/* NOTE:  inclusion of popphelp.h MUST BE AFTER conio.h so that the colors
			 will be recognized by the compiler!!										*/
/* preprocessor defines...																	*/
#define MAXPAGES		5				/* maximum number of pages						*/
#define HELP_TOP     5				/* row at top of popup help window			*/
#define HELP_BOT     20				/* row at bottom of popup help window		*/
#define HELP_RYT     71				/* column at right of popup help window	*/
#define HELP_LFT     9				/* column at left of popup help window		*/
#define HELP_FORE		WHITE			/* help window foreground color				*/
#define HELP_BACK		LIGHTBLUE	/* help window background color				*/
#define PROMPT_FORE	WHITE	   	/* bottom prompt text color					*/
#define PROMPT_BACK	LIGHTRED		/* bottom prompt background color			*/

/* function prototypes...																	*/
int	popup_help( HELPINFO );
void 	fill_window( int, int );
int	*create_popup( int, int * );
void	display_help( HELPINFO );
int 	help_control( int *, int, int * );