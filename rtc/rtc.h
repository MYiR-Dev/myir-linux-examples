/******************************************************************************
* Copyright (C), 2016-2017, Sunny.Guo
* FileName: rtc.h 
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
#ifndef __RTC_H__

#include <linux/rtc.h>
#include <sys/ioctl.h>
#include <sys/time.h>
#include <sys/types.h>
#include <ctype.h>

#include "common.h"

/******************************************************************************
  Function:       rtc_init
  Description:    initialize rtc device
  Input:          rtc    	--  rtc device name, such as '/dev/rtc''
                   
  Output:          
  Return:         int		-- return the rtc fd
  Others:         NONE
*******************************************************************************/
int				rtc_init(const char * rtc);

int 			rtc_read_time(int fd, struct rtc_time *rtc_tm);

int				rtc_set_time(int fd, int hour, int minute, int second, int year, int month, int day);

/******************************************************************************
  Function:       parse_time
  Description:    transfer time string to rtc time 
  Input:          time    	--  time string with the format MMDDhhmm[CCYY][.ss]. such as: 111817582016.18
                  hour		--  reference to int hour
                  minute	--  reference to int minute
                  second	--  reference to int second
				  month		--	reference to int month
				  year		--	reference to int year
				  day		--	reference to int day
  Output:          
  Return:         int		-- return the parse status
  Others:         NONE
*******************************************************************************/
int 			parse_time(char *time, int *hour, int *minute, int *second, int *year, int *month, int *day);

#endif	 // __RTC_H__	
