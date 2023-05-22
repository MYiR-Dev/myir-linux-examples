/******************************************************************************
* Copyright (C), 2016-2017, Sunny.Guo
* FileName: gpio.c 
* Author: Sunny.Guo
* Version: 1.0
* Date: 2017年 01月 19日 星期四 10:12:50 CST
* Description: gpio operation functions
*
* History:        
*  <author>  	<time>   	<version >   	<desc>
*  Sunny.Guo   	19/01/2017      1.0     	create this moudle  
*                                                                          
* Licensed under GPLv2 or later, see file LICENSE in this source tree.
*******************************************************************************/
#include "common.h"
#include "gpio.h"

int				gpio_export(int number)
{
	int fd;
	char gpio_str[8] =  { '\0' };
	int  len;
	
	fd = open("/sys/class/gpio/export", O_WRONLY);
	if (fd < 0) {
		dbg_perror("open gpio export");
		return -1;
	}

	sprintf(gpio_str, "%d", number);
	len = strlen(gpio_str);
	if (write(fd, gpio_str, len) < len) {
		dbg_perror("write to export");
		goto _ERR;
	}

	close(fd);
	return 0;

 _ERR:
	close(fd);
	return -1;
}

int				gpio_unexport(int number)
{
   int fd;
   char gpio_str[8] =  { '\0' };
   int	len;
   
   fd = open("/sys/class/gpio/unexport", O_WRONLY);
   if (fd < 0) {
	   dbg_perror("open gpio unexport");
	   return -1;
   }

   sprintf(gpio_str, "%d", number);
   len = strlen(gpio_str);
   if (write(fd, gpio_str, len) < len) {
	   dbg_perror("write to unexport");
	   goto _ERR;
   }

   close(fd);
   return 0;

_ERR:
   close(fd);
   return -1;
}

int				gpio_set_direction(int number, int dir)
{
	int ret = 0;
	int fd = -1;
	char dir_path[MAX_INPUT] = {'\0'};
	char dir_verify[4] = { '\0' };
	char dir_write[4] = {'\0'};
	
	sprintf(dir_path, "/sys/class/gpio/gpio%d/direction", number);
	fd = open(dir_path, O_RDWR);
	if (fd < 0) {
		dbg_perror("open gpio direction");
		return -1;
	}

	if(dir == GPIO_IN){
		strcpy(dir_write, "in");
		}
	else if(dir == GPIO_OUT){
		strcpy(dir_write, "out");
		}
	else{
		return -2;
		}

	ret = write(fd, dir_write, strlen(dir_write));
	if (ret < 0) {
		dbg_perror("write gpio direction");
		goto _ERR;
	}
	
	close(fd);
	
	fd = open(dir_path, O_RDWR);
	if (fd < 0) {
		dbg_perror("open gpio value");
		return -1;
	}

	ret = read(fd, dir_verify, strlen(dir_write));
	if (ret < 0) {
		dbg_perror("read direction back");
		goto _ERR;
	}

//	dbg_printf("==dir_write = %s, dir_verify[0]=%s \r\n", dir_write, dir_verify);
	
	if (strncmp(dir_write, dir_verify, strlen(dir_write)) != 0) {
		dbg_printf("set direction failed\n");
		goto _ERR;
	}

	close(fd);
	return 0;
	
_ERR:
   close(fd);
   return -1;
}


int				gpio_get_direction(int number, char* dir)
{
	int ret = 0;
	int fd = -1;
	char dir_path[MAX_INPUT] = {'\0'};
	char dir_verify[4] = { 0 };

	memset(dir_verify, 0, sizeof(dir_verify));
	
	sprintf(dir_path, "/sys/class/gpio/gpio%d/direction", number);
	fd = open(dir_path, O_RDWR);
	if (fd < 0) {
		dbg_perror("open gpio direction");
		return -1;
	}

	ret = read(fd, dir_verify, 3);
	if (ret < 0) {
		dbg_perror("read direction back");
		goto _ERR;
	}

	memcpy(dir, dir_verify, 3);
	close(fd);
	return 0;
	
_ERR:
   close(fd);
   return -1;
}


/******************************************************************************
  Function:       gpio_set_level
  Description:    set the gpio level
  Input:          number    	--  gpio number integer value. eg.gpio3_7 = 3*32+7
                  level			--  set gpio level. 1: high; 0: low. 
  Output:          
  Return:         int			--  return the result
  Others:         NONE
*******************************************************************************/
int				gpio_set_level(int number, int level)
{
	int ret = 0;
	int  fd;
	char value_path[MAX_INPUT] = { '\0' };
	char value;
	char value_verify;

	sprintf(value_path, "/sys/class/gpio/gpio%d/value", number);
	fd = open(value_path, O_RDWR);
	if (fd < 0) {
		dbg_perror("open gpio value");
		return -1;
	}
	if(level == 1){
		value = '1';
		}
	else if(level == 0){
			value = '0';
		}

	write(fd, &value, 1);
	if (ret < 0) {
		dbg_perror("write gpio level");
		return -1;
	}

	close(fd);
	fd = open(value_path, O_RDWR);
	if (fd < 0) {
		dbg_perror("open gpio value");
		return -1;
	}

	if (read(fd, &value_verify, 1) < 1) {
		dbg_perror("read value back");
		close(fd);
		return -1;
	}

	if ((value - value_verify)!=0) {
		dbg_printf("set value failed\n");
		close(fd);
		return -1;
	}

	close(fd);
	return 0;
}


int 			gpio_get_level(int number, char *level)
{
	int ret = 0;
	int  fd;
	char value_path[MAX_INPUT] = { '\0' };

	sprintf(value_path, "/sys/class/gpio/gpio%d/value", number);
	fd = open(value_path, O_RDWR);
	if (fd < 0) {
		dbg_perror("open gpio value");
		return -1;
	}

	ret = read(fd, level, 1);
	if ( ret < 1) {
		dbg_perror("read value back");
		close(fd);
		return -1;
	}

	return ret;
}
