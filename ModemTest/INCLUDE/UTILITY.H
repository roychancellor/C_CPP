/***************************************************************************
Include File:     utility.h
Purpose:          header file for Roy's utilities
Written By:       Roy Chancellor
Last Modified:    03-29-1994
Copyright (c), 1994, EF Data Corporation.
****************************************************************************/
int   str2int( char [] );
FILE  *getfp( char *, char * );
void  change_ext( char *, char * );
void  strip_lf( char * );
void  strip_bkslsh( char * );
#include <conio.h>
void  restore_scrn( struct text_info );
void  beep( void );
int   get_test_version( char [] );

typedef struct
{
	int   keep_going;
	int   redraw;
	int   first_time;
	int   prompt;
} PROGSTAT;
