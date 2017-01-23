/******************************************************************************
* Copyright (C), 2016-2017, Sunny.Guo
* FileName: rtc.c 
* Author: Sunny.Guo
* Version: 1.0
* Date: 2017年 01月 19日 星期四 10:12:50 CST
* Description: can operation functions
*
* History:        
*  <author>  	<time>   	<version >   	<desc>
*  Sunny.Guo   	19/01/2017      1.0     	create this moudle  
*                                                                          
* Licensed under GPLv2 or later, see file LICENSE in this source tree.
*******************************************************************************/
#include "common.h"
#include "rtc.h"


/******************************************************************************
  Function:       rtc_init
  Description:    initialize rtc device
  Input:          tty    	--  rtc device name, such as '/dev/rtc'
                   
  Output:          
  Return:         int		--  return the rtc fd
  Others:         NONE
*******************************************************************************/
int				rtc_init(const char * rtc)
{
	int fd;
	fd = open(rtc, O_RDWR);
	if(fd < 0){
		dbg_perror("open rtc");
		}
	return fd;
}


int 			rtc_read_time(int fd, struct rtc_time *rtc_tm)
{
	int ret;

	// read the RTC time/date 
	ret = ioctl(fd, RTC_RD_TIME, rtc_tm);
	if (ret == -1){
		perror("RTC_RD_TIME ioctl");
		return ret;
		}

	return 0;
}

int				rtc_set_time(int fd, int hour, int minute, int second, int year, int month, int day)
{
	int ret;
	struct rtc_time rtc_tm;

	rtc_tm.tm_mday = day;
	rtc_tm.tm_mon = month - 1;
	rtc_tm.tm_year = year - 1900;
	rtc_tm.tm_hour = hour;
	rtc_tm.tm_min = minute;
	rtc_tm.tm_wday = rtc_tm.tm_yday = rtc_tm.tm_isdst = 0;
	rtc_tm.tm_sec = second;
	
	//set the RTC time/date
	ret = ioctl(fd, RTC_SET_TIME, &rtc_tm);
	if (ret == -1) {
		perror("RTC_SET_TIME ioctl");
		return ret;
	}

	return 0;
}

int 			parse_time(char *time, int *hour, int *minute, int *second, int *year, int *month, int *day)
{
	char end = 0;
	
	if(time == NULL){
		return -1;
		}

	int len = strspn(time, "0123456789");
	if(len != 12){
		return -2;
		}

	if(time[len] == '\0' || 
		(time[len] == '.' && isdigit(time[len+1]) && isdigit(time[len+2]))){
		len = sscanf(time, "%2u%2u%2u%2u%4u%c", month, day, hour, minute, year, &end);
//		dbg_printf(" len2 = %d \r\n", len);
		if(len >= 5){
			*month = *month - 1;
			}
		else{
			return -3;
			}

		*second = 0;

		if(end == '.'){
			len =  sscanf(strchr(time, '.')+1, "%u%c", second, &end);
			if(len == 1){
				end = '\0';
				if(*second > 61){
					return -4;
					}
				}
			}
		
		}
//	dbg_printf("date/time is parsed to:  %d-%d-%d, %02d:%02d:%02d.\n\n",
//			*day, *month, *year, *hour, *minute, *second);
	return 0;
}



