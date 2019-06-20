/***************************************************************************
Include File:		popwin.h
Purpose:				defines coordinates of a popup help window
Written By:			Roy Chancellor
Last Modified:		03-05-1994
Copyright (c), 1994, EF Data Corporation.
****************************************************************************/
#define MAXMESSAGE 	2000
#define MAXSTR			100
/* define a structure that holds information about popup window...			*/
typedef struct
{
	int	*buf;					/* holds screen information							*/
	char	mes[MAXMESSAGE];	/* holds the string to put on the screen			*/
	char	cfgstr[MAXSTR];	/* used to calculate the menu width					*/
	int	lft;					/* left column of the window							*/
	int	top;					/* top row of the window								*/
	int	ryt;					/* right column of the window							*/
	int	bot;					/* bottom row of the window							*/
	int	ofs;					/* offset from the top row								*/
	int	numrows;				/* number of text rows									*/
	int	meslen;				/* message length											*/
} POPWIN;

#define SCROLL_ROW	1		/* how many rows to allow for text scrolling		*/
#define MAX_WIDTH		75		/* maximum column in window width					*/
#define MAX_HEIGHT   22		/* maximum row in window height						*/
#define ROW_SPACE		1		/* how many rows to put between border & text	*/
#define COL_SPACE		2		/* how many columns to put between border/text	*/

/* define a macro for allocating memory for a POPWIN structure...				*/
#include <alloc.h>
#define PWALLOC calloc( (size_t)1, (size_t)sizeof( POPWIN ) )

/* Function Prototypes used in popwin.c...											*/
POPWIN	*popwin( POPWIN *, int );
void 		border( int, int, int, int );