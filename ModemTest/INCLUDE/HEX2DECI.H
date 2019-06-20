/***************************************************************************
Include File:		hex2deci.h
Purpose:				function prototypes for hex2deci.c
Written By:   		Roy Chancellor
Last Modified:		02-24-1994
Copyright (c), 1994, EF Data Corporation.
****************************************************************************/
void 		add_to_str( char *, int, char * );
int 		hex2dec_char( char, char * );
char 		dec2hex_char( int, char * );
int		hex2dec_int( char *, char * );
void		dec2hex_str( int, char *, char * );