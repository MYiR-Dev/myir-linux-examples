/******************************************************************************
* Copyright (C), 2016-2017, Sunny.Guo
* FileName: tty.h 
* Author: Sunny.Guo
* Version: 1.0
* Date: 2017年 01月 19日 星期四 10:12:50 CST
* Description: tty definition         
*
* History:        
*  <author>  	<time>   	<version >   	<desc>
*  Sunny.Guo   	19/01/2017      1.0     	create this moudle  
*                                                                          
* Licensed under GPLv2 or later, see file LICENSE in this source tree.
*******************************************************************************/
#ifndef __TTY_H__

#include <termios.h>
#include <linux/ioctl.h>
#include <linux/serial.h>

#include <sys/ioctl.h>
#include <sys/mman.h>
#include <sys/types.h>

#include "common.h"

#define TTY_DEFAULT_BAUDRATE  	9600
#define TTY_READ_TIMEOUT_USEC	5000
#define TTY_READ_BUFFER_SIZE	1024

#define TTY_RS485_MODE			1
#define TTY_RS232_MODE			0

#ifndef TIOCSRS485
    #define TIOCSRS485     0x542F
#endif

#ifndef TIOCGRS485
    #define TIOCGRS485     0x542E
#endif

struct tty_dev
{
	char tty_dev[MAX_INPUT];
	int bitrate;
	int datasize;
	char par;
	int stop;
	int flow;
	int rs485_mode;
};
typedef struct tty_dev tty_dev_t;


/******************************************************************************
  Function:       tty_init
  Description:    initialize tty device
  Input:          tty    	--  tty device name, such as '/dev/ttyO0', '/dev/ttyO5'
                   
  Output:          
  Return:         int		-- return the tty fd
  Others:         NONE
*******************************************************************************/
int				tty_init(const char * tty);

/******************************************************************************
  Function:       tty_setting
  Description:    set the tty device's mode and bitrate
  Input:          fd       --  tty device fd
  				  bitrate --  tty baudrate
  				  datasize --  number of data bits
  				  mode	   --  tty mode, 1: rs485; 0: rs232
  				  flow	   --  cts/rts flow control
  				  par	   --  odd/even 
  				  stop	   --  number of stop bits
                  
  Output:          
  Return:         int 	   --  tty setting status 0:success 
  Others:         NONE
*******************************************************************************/
int 			tty_setting(int fd, int bitrate, int datasize, int mode, int flow, int par, int stop);

int				tty_read(int fd, char *frame);
int				tty_write(int fd, char *frame, int len);

int 			tty_mode(const int fd,  int mode);

#endif	 // __TTY_H__	
