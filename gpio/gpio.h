/******************************************************************************
* Copyright (C), 2016-2017, Sunny.Guo
* FileName: gpio.h 
* Author: Sunny.Guo
* Version: 1.0
* Date: 2017年 01月 19日 星期四 10:12:50 CST
* Description: gpio definition         
*
* History:        
*  <author>  	<time>   	<version >   	<desc>
*  Sunny.Guo   	19/01/2017      1.0     	create this moudle  
*                                                                          
* Licensed under GPLv2 or later, see file LICENSE in this source tree.
*******************************************************************************/
#ifndef __GPIO_H__

#include "common.h"

#define GPIO_SET			1
#define GPIO_GET			0

#define GPIO_IN				0
#define GPIO_OUT			1

int				gpio_export(int number);

int				gpio_unexport(int number);

int 			gpio_set_direction(int number, int dir);

int				gpio_get_direction(int number, char * dir);

/******************************************************************************
  Function:       gpio_set
  Description:    set the gpio level
  Input:          number    	--  gpio number integer value. eg.gpio3_7 = 3*32+7
                  level			--  set gpio level. 1: high; 0: low. 
  Output:          
  Return:         int			--  return the result
  Others:         NONE
*******************************************************************************/
int				gpio_set_level(int number, int level);

int 			gpio_get_level(int number, char* level);

#endif	 // __GPIO_H__	
