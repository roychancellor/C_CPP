/**************************************************************************
*    MODULE:                  INSTALL.C                                   *
*                                                                         *
*    CREATED BY:              ROY CHANCELLOR                              *
*                                                                         *
*    DATE VERSION CREATED:    11/13/92                                    *
*                                                                         *
*    DATE LAST MODIFIED:      07/29/93                                    *
*                                                                         *
*    VERSION:                 1.00                                        *
*                                                                         *
*    DESCRIPTION:  This program installs the pendulum animation system    *
*                  files to either c:\pendulum or to a user input drive   *
*                  and directory.  Pkunzip is used to uncompress .zip     *
*                  files from the floppy disk.                            *
*                                                                         *
*    CHANGES:                                                             *
*                                                                         *
**************************************************************************/

#include <stdio.h>
#include <conio.h>
#include <process.h>
#include <stdlib.h>
#include <stdio.h>
#include <graph.h>
#include <conio.h>
#include <stdlib.h>
#include <process.h>
#include <string.h>

void main( void )
{
	int which;
	char make_dir[20];
	char change_dir[20];
	char cfgfile[35];
	char dir_name[30];
	char letter[3], temp[3];
	char directory[8];
	char string[100];
	struct videoconfig screen;
	FILE *pendcfg, *fopen();

	_getvideoconfig( &screen );

	_clearscreen( _GCLEARSCREEN );

	printf( "\n\n\n\n THIS PROGRAM WILL COPY FILES TO C:\\PENDULUM...\n\n" );
	printf( " IF THIS IS SATISFACTORY ENTER 'Y', OTHERWISE HIT 'N'  " );
	which = getch();

	if( which == 'y' || which == 'Y' )
	{
		_clearscreen( _GCLEARSCREEN );
		printf( "\n\n COPYING FILES TO DISK....\n\n" );
		system( "md c:\\pendulum" );
		system( "c:" );
		system( "cd\\pendulum" );
		system( "copy a:*.*" );
		printf( "\n\n RETRIEVING COMPRESSED FILES...\n\n" );
		system( "pkunzip data.zip" );
		system( "pkunzip execute.zip" );
		if( (pendcfg = fopen( "c:\\pendulum\\pend.cfg", "w" ) ) != NULL )
		{
			fputs( "c:\\pendulum\\", pendcfg );
			fclose( pendcfg );
		}
		system( "erase pkunzip.exe" );
		system( "erase install.exe" );
		system( "erase *.zip" );

		printf( "\n\n\n");
		printf( " INSTALLATION COMPLETE." );
		printf( "\n TYPE 'PENDMENU' AT PROMPT TO START\n\n" );
	}
	else
	{
		_clearscreen( _GCLEARSCREEN );
		printf( "\n\n ENTER THE DESIRED DIRECTORY TO PUT FILES IN" );
		printf( "\n\n       DRIVE LETTER (NO :)---  " );
		gets( temp );
		strcpy( letter, temp );
		printf( "\n          DIRECTORY:  " );
		gets( directory );

		printf( "\n\n COPYING FILES TO DISK....\n\n" );
		strcpy( string, letter );
		strcat( string, ":\\" );
		strcat( string, directory );
		strcpy( make_dir, "md " );
		strcat( make_dir, string );
		system( make_dir );
		strcat( letter, ":" );
		system( letter );
		strcpy( change_dir, "cd\\" );
		strcat( change_dir, directory );
		system( change_dir );
		system( "copy a:*.*" );
		printf( "\n\n RETRIEVING COMPRESSED FILES...\n\n" );
		system( "pkunzip data.zip" );
		system( "pkunzip execute.zip" );

		strcpy( cfgfile, letter );
		strcat( cfgfile, "\\" );
		strcat( cfgfile, directory );
		strcpy( dir_name, cfgfile );
		strcat( dir_name, "\\" );
		strcat( cfgfile, "\\pend.cfg" );

		if( (pendcfg = fopen( cfgfile, "w" ) ) != NULL )
		{
			fputs( dir_name, pendcfg );
			fclose( pendcfg );
		}
		system( "erase pkunzip.exe" );
		system( "erase install.exe" );
		system( "erase *.zip" );

		printf( "\n\n\n");
		printf( " INSTALLATION COMPLETE." );
		printf( "\n TYPE 'PENDMENU' AT PROMPT TO START\n\n" );

	}
}  /* end main */
