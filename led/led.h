/******************************************************************************
* Copyright (C), 2016-2017, Sunny.Guo
* FileName: led.h 
* Author: Sunny.Guo
* Version: 1.0
* Date: 2017年 01月 19日 星期四 10:12:50 CST
* Description: led definition         
*
* History:        
*  <author>  	<time>   	<version >   	<desc>
*  Sunny.Guo   	19/01/2017      1.0     	create this moudle  
*                                                                          
* Licensed under GPLv2 or later, see file LICENSE in this source tree.
*******************************************************************************/
#ifndef __LED_H__

#include "common.h"


/******************************************************************************
  Function:       led_set
  Description:    initialize led device and set it on or off
  Input:          led    	--  led name, such as 'myc\:blue\:cpu0'
                  on		--  led status. 1: on; 0: off. 
  Output:          
  Return:         int		-- return the led set status
  Others:         NONE
*******************************************************************************/
int				led_set(const char * led, int on);

#endif	 // __LED_H__	
