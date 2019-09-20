/******************************************************************************
* Copyright (C), 2016-2017, Sunny.Guo
* FileName: eeprom.h 
* Author: Sunny.Guo
* Version: 1.0
* Date: 2017年 01月 19日 星期四 10:12:50 CST
* Description: eeprom definition         
*
* History:        
*  <author>  	<time>   	<version >   	<desc>
*  Sunny.Guo   	19/01/2017      1.0     	create this moudle  
*                                                                          
* Licensed under GPLv2 or later, see file LICENSE in this source tree.
*******************************************************************************/
#ifndef __EEPROM_H__

#include <linux/ioctl.h>
#include <sys/ioctl.h>
#include <sys/types.h>

#include "common.h"

#define EEPROM_I2C_ADDR			0x50
#define	EEPROM_WP_ENABLE		1
#define EEPROM_WP_DISABLE		0

// gpio3_7 --  3*32+7 = 103
#define EEPROM_WP_GPIO			103
#define EEPROM_BUFFER_SIZE		100

#define I2C_RETRIES 			0x0701
#define I2C_TIMEOUT 			0x0702
#define I2C_RDWR 				0x0707 

#define I2C_M_TEN 				0x0010
#define I2C_M_RD 				0x0001
#define I2C_M_WR				0x0000


struct i2c_msg
{
	unsigned short addr;
	unsigned short flags;
	unsigned short len;
	unsigned char *buf;
};

struct i2c_rdwr_ioctl_data
{
	struct i2c_msg *msgs;
	int nmsgs;
};

/******************************************************************************
  Function:       eeprom_wp
  Description:    set eeprom write protect
  Input:          wp_enable       --  1: write protect enable; 0: write protect disable.
                  
  Output:          
  Return:         int 	   		  --  status 0:success 
  Others:         NONE
*******************************************************************************/
int 			eeprom_wp(int wp_enable);

int 			eeprom_init(char * dev);

int 			eeprom_write(int fd, int addr, int start,  char * data, int len);

int 			eeprom_read(int fd, int addr, int start, char *data, int len);

int 			parse_address(char *address_arg);

#endif	 // __EEPROM_H__	
