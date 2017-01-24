/******************************************************************************
* Copyright (C), 2016-2017, Sunny.Guo
* FileName: led.c 
* Author: Sunny.Guo
* Version: 1.0
* Date: 2017年 01月 19日 星期四 10:12:50 CST
* Description: led operation functions
*
* History:        
*  <author>  	<time>   	<version >   	<desc>
*  Sunny.Guo   	19/01/2017      1.0     	create this moudle  
*                                                                          
* Licensed under GPLv2 or later, see file LICENSE in this source tree.
*******************************************************************************/
#include "common.h"
#include "led.h"


/******************************************************************************
  Function:       led_set
  Description:    initialize led device and set it on or off
  Input:          led    	--  led name, such as 'myc\:blue\:cpu0'
                  on		--  led status. 1: on; 0: off. 
  Output:          
  Return:         int		-- return the led set status
  Others:         NONE
*******************************************************************************/
int				led_set(const char * led, int on)
{
	int ret = 0;
	char cmdline[128];

	sprintf(cmdline, "echo %d >  /sys/class/leds/%s/brightness", on, led);
	ret =  system(cmdline);
	return  WEXITSTATUS(ret);

}




