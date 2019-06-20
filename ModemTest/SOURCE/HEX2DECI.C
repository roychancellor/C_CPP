/***************************************************************************
*  Module:        HEX2DECI.C                                               *
*                                                                          *
*  Created By:    Roy Chancellor                                           *
*                                                                          *
*  Date Created:  02-24-1994                                               *
*                                                                          *
*  Last Modified: 05-10-1994                                               *
*                                                                          *
*  Version:       1.00                                                     *
*                                                                          *
*  Date Released: 05-10-1994                                               *
*                                                                          *
*  Description:   converts strings which are hexidecimal values to int-    *
*                 eger values in base 10.  Converts base-10 integers to    *
*                 hexadecimal strings.                                     *
*                                                                          *
*  Inputs:        various                                                  *
*                                                                          *
*  Returns:       none                                                     *
*                                                                          *
*  Date           Modification                                    Initials *
* ----------   -------------------------------------------        ---------*
*  03-12-1994  Released as version 0.00                              RSC   *
*  03-31-1994  Fixed bug in subtracting 1 from 10; ver to 0.01       RSC   *
*  04-01-1994  added a '\0' at the end of str in dec2hex_str.        RSC   *
*              version to 1.00                                             *
*	05-10-1994	RELEASED FOR USE ON CAC AS VERSION 1.00					RSC	*
****************************************************************************/
/* Include files that came with Turbo C...                                 */
#include <string.h>

/* Include files custom written for modem test project...                  */
#include <hex2deci.h>   /* for function prototypes                         */
#include <boolean.h>    /* for values that are 1 or 0                      */

/***************************************************************************
Function:      add_to_str
Purpose:       to add a [type] value to a string which represents a
					[type] value
Arguments:     hex string, value to add, type
Returns:       none
****************************************************************************/
void add_to_str( char *hexstr, int val2add, char *type )
{
	int   intval;

	if( !strcmp( type, "hex" ) )
	{
		/* convert the string to an integer...                               */
		intval = hex2dec_int( hexstr, "hex" );

		/* add the value to intval...                                        */
		intval += val2add;

		/* convert the hex value back to a string...                         */
		dec2hex_str( intval, hexstr, "hex" );
	}  /* end if */
}  /* end add_to_str */

/***************************************************************************
Function:      hex2dec_int
Purpose:       converts a string to its numeric representation
Arguments:     string, type:  "hex" or "int"
Returns:       the integer conversion

NOTE:          see p.221 in Kochan, J., "Programming in C", Revised Ed.
****************************************************************************/
int   hex2dec_int( char *str2conv, char *type )
{
	int   i, intval, base;

	if( !strcmp( type, "hex" ) )
		base = 16;

	intval = 0;
	for( i = 0; i < (int)strlen( str2conv ); ++i )
		intval = intval * base + hex2dec_char( str2conv[i], "hex" );
	return( intval );
}  /* end hex2dec_int */

/***************************************************************************
Function:      hex2dec_char
Purpose:       to convert a [type]char to an integer
Arguments:     character to convert, type:  "hex" or "int"
Returns:       the integer value of the character
****************************************************************************/
int hex2dec_char( char c, char *type )
{
	int   val;

	if( !strcmp( type, "hex" ) )
	{
		switch( c )
		{
			case '1':case '2':case '3':case '4':case '5':case '6':case '7':
			case '8':case '9':case '0':
				val = c - '0';  /* because in ASCII '0' - '9' are sequential   */
				break;
			case 'A':val = 10;break;
			case 'B':val = 11;break;
			case 'C':val = 12;break;
			case 'D':val = 13;break;
			case 'E':val = 14;break;
			case 'F':val = 15;break;
		}  /* end switch */
	}  /* end if */
	return( val );
}  /* end hex2dec_char */

/***************************************************************************
Function:      dec2hex_str
Purpose:       converts an integer into a string
Arguments:     integer to convert, string to put it in, type of integer
Returns:       none
****************************************************************************/
void  dec2hex_str( int intval, char *str, char *type )
{
	int   i, j, base, keep_going, quotient, remainder[8], last, jstart;

	if( !strcmp( type, "hex" ) )
		base = 16;

	i = 0;
	keep_going = YES;
	while( keep_going )
	{
		quotient     = intval / base;
		remainder[i] = intval % base;
		if( quotient == 0 )
			keep_going = NO;
		else
		{
			intval /= base;
			++i;
		}
	}  /* end while */
	last = i;
	if( i == 0 )
	{
		jstart = 1;
		str[0] = '0';
		remainder[1] = remainder[0];
		last = ++i;
	}
	else
	{
		last = i;
		jstart = 0;
	}
	for( j = jstart; j <= last; ++j )
	{
		str[j] = dec2hex_char( remainder[i], "hex" );
		--i;
	}
	str[j] = '\0';
}  /* end dec2hex_str */

/***************************************************************************
Function:      dec2hex_char
Purpose:       to convert a [type]int to a character
Arguments:     integer to convert, type
Returns:       the character value of the integer
****************************************************************************/
char dec2hex_char( int i, char *type )
{
	if( !strcmp( type, "hex" ) )
	{
		switch( i )
		{
			case 0:case 1:case 2:case 3:case 4:case 5:case 6:case 7:case 8:
			case 9:
				return( i + '0' );
			case 10:return( 'A' );
			case 11:return( 'B' );
			case 12:return( 'C' );
			case 13:return( 'D' );
			case 14:return( 'E' );
			case 15:return( 'F' );
			default:return( 'X' );
		}  /* end switch */
	}  /* end if */
	return( OK );
}  /* end dec2hex_char */