/***************************************************************************
Include File:		BifEditr.h
Purpose:				Definitions and function prototypes for the bifurcation
						diagram simulation parameter editor.
Written By:       Roy Chancellor 
Last Modified:		6-20-1994
Copyright (c), 1994, Roy Chancellor.
****************************************************************************/
#define NUM_HEADINGS 11
#define NUM_FIELDS   5

int bifeditr( char [] );
int bif_setup_graphics( void );
int bif_setup_edit_screen( void );
int bif_get_file_info( char [], struct bif * );
void bif_initialize_vals( struct text_loc [] );
int bif_put_info_on_screen( struct bif, struct text_loc [] );
void bif_edit_away( struct bif *, struct text_loc [] );
void bif_blanks_to_line( int, int, int );
int bif_write_file_info( char [], struct bif );
