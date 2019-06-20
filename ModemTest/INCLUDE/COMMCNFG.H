/***************************************************************************
Include File:     commcnfg.h
Purpose:          function prototypes and defines for commcnfg.c
Written By:       Roy Chancellor 
Last Modified:    05-13-1994
Copyright (c), 1994, EF Data Corporation.
****************************************************************************/
#include <conio.h>   /* for color definitions                              */

#define NUM_CFG_ITEMS    21
#define CC_TXT_START_ROW 3
#define CC_TXT_START_COL 15
#define TITLEROW     1
#define PROMPTROW    CC_TXT_START_ROW + NUM_CFG_ITEMS + 1
#define SKIP_ROW     1
#define LEADERLENGTH 4
#define CC_FORE      WHITE
#define CC_BACK      LIGHTBLUE
#define MAX_VALUE_LENGTH	6

/* Function prototypes...                                                  */
void  change_value( char [], int, int );
int   configure_communications( void );
