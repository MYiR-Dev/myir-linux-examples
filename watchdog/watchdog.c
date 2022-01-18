/******************************************************************************
* Copyright (C), 2016-2017, Sunny.Guo
* FileName: watchdog.c 
* Author: Sunny.Guo
* Version: 1.0
* Date: 2017年 01月 19日 星期四 10:12:50 CST
* Description: watchdog operation functions
*
* History:        
*  <author>  	<time>   	<version >   	<desc>
*  Sunny.Guo   	22/04/2017      1.0     	create this moudle  
*                                                                          
* Licensed under GPLv2 or later, see file LICENSE in this source tree.
*******************************************************************************/
#include "common.h"
#include "watchdog.h"

// omap watchdog fd
int fd0 = -1;
int fd1 = -1;


/******************************************************************************
  Function:       watchdog_set_timeout
  Description:    set the watchdog timeout
  Input:          index    	--  watchdog index, 0: omap_watchdog  1: myir_wdt
                  timeout	--  timeout
  Output:          
  Return:         int		-- return the  status
  Others:         NONE
*******************************************************************************/
int				watchdog_set_timeout(int index, int timeout)
{
	if(index == 0){
			int data = 0;
			int ret_val;
		
				fd0=open("/dev/watchdog",O_WRONLY);
				if (fd0==-1) {
						dbg_perror("omap watchdog");
						return -1;
				}
		
				ret_val = ioctl (fd0, WDIOC_GETTIMEOUT, &data);
				if (ret_val) {
					dbg_printf ("\nWatchdog Timer : WDIOC_GETTIMEOUT failed");
				}
				else {
					dbg_printf ("\nCurrent timeout value is : %d micro seconds\n", data);
				}
		
				data = timeout;
		
				ret_val = ioctl (fd0, WDIOC_SETTIMEOUT, &data);
				if (ret_val) {
					dbg_printf ("\nWatchdog Timer : WDIOC_SETTIMEOUT failed");
				}
				else {
					dbg_printf ("\nNew timeout value is : %d micro seconds", data);
				}
		
				ret_val = ioctl (fd0, WDIOC_GETTIMEOUT, &data);
				if (ret_val) {
					dbg_printf ("\nWatchdog Timer : WDIOC_GETTIMEOUT failed");
				}
				else {
					dbg_printf ("\nCurrent timeout value is : %d micro seconds\n", data);
				}
		
				while(1){
						if (1 != write(fd0, "\0", 1)){
						dbg_printf("Write failed\n");
						break;
					}
			//		else{
			//			dbg_printf("Write succeeded\n");
			//			}
			
						sleep(timeout);
				}
		
				if (0 != close (fd0))
				dbg_printf("Close failed\n");
			else
				dbg_printf("Close succeeded\n");
		
			return 0;
		}
	else if(index == 1){
//		int ret = 0;
//		char cmdline[128];
//		
//		sprintf(cmdline, "echo %d >  /sys/class/myir_watchdog/wd_period_ms", timeout);
//		ret =  system(cmdline);
//		return	WEXITSTATUS(ret);
//
			int data = 0;
			int ret_val;
		
				fd1=open("/dev/watchdog1",O_WRONLY);
				if (fd1==-1) {
						dbg_perror("omap watchdog");
						return -1;
				}
		
				ret_val = ioctl (fd1, WDIOC_GETTIMEOUT, &data);
				if (ret_val) {
					dbg_printf ("\nWatchdog Timer : WDIOC_GETTIMEOUT failed");
				}
				else {
					dbg_printf ("\nCurrent timeout value is : %d micro seconds\n", data);
				}
		
				data = timeout;
		
				ret_val = ioctl (fd1, WDIOC_SETTIMEOUT, &data);
				if (ret_val) {
					dbg_printf ("\nWatchdog Timer : WDIOC_SETTIMEOUT failed");
				}
				else {
					dbg_printf ("\nNew timeout value is : %d micro seconds", data);
				}
		
				ret_val = ioctl (fd1, WDIOC_GETTIMEOUT, &data);
				if (ret_val) {
					dbg_printf ("\nWatchdog Timer : WDIOC_GETTIMEOUT failed");
				}
				else {
					dbg_printf ("\nCurrent timeout value is : %d micro seconds\n", data);
				}
		
				while(1){
						if (1 != write(fd1, "\0", 1)){
						dbg_printf("Write failed\n");
						break;
					}
			//		else{
			//			dbg_printf("Write succeeded\n");
			//			}
			
						sleep(timeout);
				}
		
				if (0 != close (fd1))
				dbg_printf("Close failed\n");
			else
				dbg_printf("Close succeeded\n");
		}
	else{
		dbg_perror("== watchdog id error\r\n");
		return -1;
		}
}

int watchdog_enable(int index)
{
	int flags = WDIOS_ENABLECARD;
	if(index == 0){
		ioctl(fd0, WDIOC_SETOPTIONS, &flags);
	}
	if(index == 1){
		ioctl(fd1, WDIOC_SETOPTIONS, &flags);
	}
        fprintf(stderr, "Watchdog card disabled.\n");
        fflush(stderr);

}

int watchdog_disable(int index)
{
	int flags = WDIOS_DISABLECARD;
	if(index == 0){
		ioctl(fd0, WDIOC_SETOPTIONS, &flags);
	}
	if(index == 1){
		ioctl(fd1, WDIOC_SETOPTIONS, &flags);
	}
        fprintf(stderr, "Watchdog card disabled.\n");
        fflush(stderr);

}

int watchdog_close(int index)
{
	int ret = 0;
	if(index == 0){
		ret = close(fd0);
		}
	if(index == 1){
		ret = close(fd1);
		}
	
	return ret;
}



