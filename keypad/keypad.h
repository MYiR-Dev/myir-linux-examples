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
#ifndef __KEYPAD_H__

#include <linux/input.h>
#include <sys/select.h> 
#include <sys/time.h> 


#include "common.h"

/******************************************************************************
  Function:       keypad_init
  Description:    initialize keypad device
  Input:          keypad    	--  keypad device name, such as '/dev/input/event0'
                   
  Output:          
  Return:         int		-- return the keypad fd
  Others:         NONE
*******************************************************************************/
int				keypad_init(const char * keypad);

int 			keypad_monitor(int fd, struct input_event *ev);

#endif	 // __KEYPAD_H__	
