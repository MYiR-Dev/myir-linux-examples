/******************************************************************************
* Copyright (C), 2016-2017, Sunny.Guo
* FileName: pru_led.h 
* Author: Sunny.Guo
* Version: 1.0
* Date: 2017年 01月 19日 星期四 10:12:50 CST
* Description: pru led definition         
*
* History:        
*  <author>  	<time>   	<version >   	<desc>
*  Sunny.Guo   	19/01/2017      1.0     	create this moudle  
*                                                                          
* Licensed under GPLv2 or later, see file LICENSE in this source tree.
*******************************************************************************/
#ifndef __PRU_LED_H__

#include "common.h"

#define MAX_BUFFER_SIZE				512
#define NUM_MESSAGES				100
#define DEFAULT_DEVICE_NAME			"/dev/rpmsg_pru31"


int 			pru_led_init(char *dev_name);

/******************************************************************************
  Function:       pru_led_set
  Description:    initialize led device and set it on or off
  Input:          fd    	--  led name, such as 'myc\:blue\:cpu0'
                  led		--  led status. 0~7 maps to three led's statu. 
  Output:          
  Return:         int		-- return the led set status
  Others:         NONE
*******************************************************************************/
int				pru_led_set(int fd,  char *led);

int				pru_rpmsg_read(int fd, char *buf);

#endif	 // __PRU_LED_H__	
