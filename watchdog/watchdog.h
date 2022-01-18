/******************************************************************************
* Copyright (C), 2016-2017, Sunny.Guo
* FileName: watchdog.h 
* Author: Sunny.Guo
* Version: 1.0
* Date: 2017年 01月 19日 星期四 10:12:50 CST
* Description: watchdog definition         
*
* History:        
*  <author>  	<time>   	<version >   	<desc>
*  Sunny.Guo   	22/04/2017      1.0     	create this moudle  
*                                                                          
* Licensed under GPLv2 or later, see file LICENSE in this source tree.
*******************************************************************************/
#ifndef __WATCHDOG_H__
#include <linux/rtc.h>
#include <sys/ioctl.h>
#include <sys/time.h>
#include <sys/types.h>
#include <errno.h>
#include <asm/types.h>
#include <linux/watchdog.h>
#include <sys/stat.h>
#include <signal.h>

#include "common.h"

/******************************************************************************
  Function:       watchdog_set_timeout
  Description:    set the watchdog timeout
  Input:          index    	--  watchdog index, 0: omap_watchdog  1: myir_wdt
                  timeout	--  timeout
  Output:          
  Return:         int		-- return the  status
  Others:         NONE
*******************************************************************************/
int			watchdog_set_timeout(int index, int timeout);

int 			watchdog_close(int index);
int 			watchdog_enable(int index);
int 			watchdog_disable(int index);

#endif	 // __WATCHDOG_H__	
