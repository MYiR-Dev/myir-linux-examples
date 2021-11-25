/******************************************************************************
* Copyright (C), 2016-2017, Sunny.Guo
* FileName: eeprom.c 
* Author: Sunny.Guo
* Version: 1.0
* Date: 2017年 01月 19日 星期四 10:12:50 CST
* Description: eeprom operation functions
*
* History:        
*  <author>  	<time>   	<version >   	<desc>
*  Sunny.Guo   	19/01/2017      1.0     	create this moudle  
*                                                                          
* Licensed under GPLv2 or later, see file LICENSE in this source tree.
*******************************************************************************/
#include "common.h"
#include "eeprom.h"
/******************************************************************************
  Function:       eeprom_wp
  Description:    set eeprom write protect
  Input:          wp_enable       --  1: write protect enable; 0: write protect disable.
                  
  Output:          
  Return:         int 	   		  --  status 0:success 
  Others:         NONE
*******************************************************************************/
int 			eeprom_wp(int wp_enable)
{
	int ret = 0;
	char cmdline[256] = { '\0' };
	int  gpio = 103;

	if(wp_enable == EEPROM_WP_ENABLE){
		sprintf(cmdline,
				"echo %d > /sys/class/gpio/unexport; echo %d > /sys/class/gpio/export; echo \"out\" > /sys/class/gpio/gpio%d/direction; echo 1 > /sys/class/gpio/gpio%d/value",
				gpio, gpio, gpio, gpio);
		}
	else if(wp_enable == EEPROM_WP_DISABLE){
		sprintf(cmdline,
				"echo %d > /sys/class/gpio/unexport; echo %d > /sys/class/gpio/export; echo \"out\" > /sys/class/gpio/gpio%d/direction; echo 0 > /sys/class/gpio/gpio%d/value",
				gpio, gpio, gpio, gpio);
		}

	ret = system(cmdline);
	return WEXITSTATUS(ret);
}


int 			eeprom_init(char *dev)
{
	int fd = -1;
	fd = open(dev,O_RDWR);
	if(fd < 0) {
		perror("open error");
		return -1;
	}
	
	ioctl(fd,I2C_TIMEOUT,1);
	ioctl(fd,I2C_RETRIES,2);

	return fd;
}

int 			eeprom_write(int fd, int addr, int start,  char * data, int len)
{
	int ret = 0;
	struct i2c_rdwr_ioctl_data e2prom_data;
	char buffer[EEPROM_BUFFER_SIZE] = {0};
	e2prom_data.nmsgs=1;
	e2prom_data.msgs=(struct i2c_msg*)malloc(e2prom_data.nmsgs*sizeof(struct i2c_msg));
	if(!e2prom_data.msgs) {
		perror("malloc error");
		return -1;
	}
	
	e2prom_data.nmsgs=1;
	e2prom_data.msgs[0].len = len+2;
	e2prom_data.msgs[0].addr = addr;
	e2prom_data.msgs[0].flags = I2C_M_WR; //write
	e2prom_data.msgs[0].buf= buffer;
	
	e2prom_data.msgs[0].buf[0]=(char)((start >>8)&0xff);// e2prom addr[15:8] 
	e2prom_data.msgs[0].buf[1]=(char)((start)&0xff);//e2prom addr[7:0]

	memcpy(&buffer[2], data, len);

	ret=ioctl(fd,I2C_RDWR,(unsigned long)&e2prom_data);
	if(ret<0) {
		perror("ioctl error1");
	}

	return ret;
}

int 			eeprom_read(int fd, int addr, int start, char *data, int len)
{
	int ret = 0;
	struct i2c_rdwr_ioctl_data e2prom_data;
	char buffer[EEPROM_BUFFER_SIZE] = {0};

	e2prom_data.nmsgs=2;
	e2prom_data.msgs=(struct i2c_msg*)malloc(e2prom_data.nmsgs*sizeof(struct i2c_msg));
	if(!e2prom_data.msgs) {
		perror("malloc error");
		return -1;
	}

//	dbg_printf("read address 0x%x at offset 0x%x with %d bytes \r\n", addr, start, len);
	e2prom_data.nmsgs=2;
	e2prom_data.msgs[0].len=2;
	e2prom_data.msgs[0].addr=addr;
	e2prom_data.msgs[0].flags=I2C_M_WR;						// write
	e2prom_data.msgs[0].buf=buffer;
	e2prom_data.msgs[0].buf[0]=(char)((start >>8)&0xff);	// e2prom addr[15:8] 
	e2prom_data.msgs[0].buf[1]=(char)((start)&0xff);		//e2prom addr[7:0]
	e2prom_data.msgs[1].len= len;
	e2prom_data.msgs[1].addr=addr;
	e2prom_data.msgs[1].flags=I2C_M_RD;						// read
	e2prom_data.msgs[1].buf= data;

	ret=ioctl(fd,I2C_RDWR,(unsigned long)&e2prom_data);
	if(ret < 0 ) {
		perror("ioctl error2");
	}

	return ret;
}

int 			parse_address(char *address_arg)
{
	long address;
	char *end;
	address = strtol(address_arg, &end, 0);
	if (*end || !*address_arg) {
		fprintf(stderr, "Error: Chip address is not a number!\n");
		return -1;
		}
	
	return address;
}


