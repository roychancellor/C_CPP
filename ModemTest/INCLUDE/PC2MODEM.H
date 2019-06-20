/***************************************************************************
Include File:     pc2modem.h
Purpose:          has all information used by pc2modem.c
Written By:       Roy Chancellor
Last Modified:    05-17-1994
Copyright (c), 1994, EF Data Corporation.
****************************************************************************/
#include <conio.h>

#define CMDWIN_ROWS  10
#define PAUSE        '!'
#define END_TEST     999
#define BREAK_TEST   '#'
#define START_CMD    '<'
#define REMARK		'$'
#define TERMINATOR   (unsigned short)']'
#define TIMEOUT      5
#define COMM_FORE    WHITE
#define COMM_BACK    BLACK
#define BAUD_DEFAULT 9600
#define PORT_DEFAULT 3
#define BASE_ADDR_DEFAULT	0x3E8
#define BAUD_MAX     19200
#define BAUD_MIN     110
#define NUM_RATES    9
#define RESTART      2
#define END_OK       3
#define RXBUFSIZE    (unsigned long)1000
#define TXBUFSIZE    RXBUFSIZE
#define RTSDTR       0
#define RS485		485
#define RS232		232
#define ARGUMENT	3
#define CMDFILE		4
#define RS485_PORT_DEFAULT	3
#define RS485_BAUD_DEFAULT	9600
#define RS485_BASE_ADDR_DEFAULT	0x3E8
#define RS485_IRQ_DEFAULT	5
#define RS232_PORT_DEFAULT	4
#define RS232_BAUD_DEFAULT	9600
#define RS232_BASE_ADDR_DEFAULT	0x2E8
#define RS232_IRQ_DEFAULT	10
#define LOWEST_PORT		1
#define HIGHEST_PORT		4
#define NUM_VALID_IRQ		6   /* 3, 4, 5, 10, 11, 12 */

struct comm_config
{
	int   verbose;
	int   show_stat_of_cmds;
	int   showcmds;
	int   dumpstat;
	int   baud;
	int   port;
	int	base_addr;
	int	irq;
};
int pc2modem( char *, char *, char *, int, int );
int verify_comm_params( struct comm_config * );
