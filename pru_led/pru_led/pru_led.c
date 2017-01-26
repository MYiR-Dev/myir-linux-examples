/******************************************************************************
* Copyright (C), 2016-2017, Sunny.Guo
* FileName: pru_led.c 
* Author: Sunny.Guo
* Version: 1.0
* Date: 2017年 01月 19日 星期四 10:12:50 CST
* Description: pru led operation functions
*
* History:        
*  <author>  	<time>   	<version >   	<desc>
*  Sunny.Guo   	19/01/2017      1.0     	create this moudle  
*                                                                          
* Licensed under GPLv2 or later, see file LICENSE in this source tree.
*******************************************************************************/
#include <sys/poll.h>

#include "common.h"
#include "pru_led.h"


int 			pru_led_init(char *dev_name)
{
	int fd = -1;
	
	/* Open the rpmsg_pru character device file */
	fd = open(dev_name, O_RDWR);
	/*
	 * If the RPMsg channel doesn't exist yet the character device
	 * won't either.
	 * Make sure the PRU firmware is loaded and that the rpmsg_pru
	 * module is inserted.
	 */
	if (fd < 0) {
		dbg_printf("Failed to open %s\n", dev_name);
		return -1;
	}
}

/******************************************************************************
  Function:       pru_led_set
  Description:    write the number of led to be lighted on
  Input:          fd    	--  rpmsg dev fd
                  led		--  led status. 0~7 maps to three led's statu. 
  Output:          
  Return:         int		-- return the rpmsg dev fd
  Others:         NONE
*******************************************************************************/
int				pru_led_set(int fd,  char *led)
{
	int ret = 0;
	ret = write(fd, led, strlen(led));
	if (ret > 0){
		dbg_printf("Sent to PRU: %c\n", led);
		}

	return ret;
}

/******************************************************************************
  Function:       pru_rpmsg_read
  Description:    read the msg from pru
  Input:          fd    	--  rpmsg dev fd
                  buf		--  pointer to the read message buffer. 
  Output:          
  Return:         int		-- return the msg len
  Others:         NONE
*******************************************************************************/
int				pru_rpmsg_read(int fd, char *buf)
{
	int ret = 0;
	struct pollfd pollfds[1];

	pollfds[0].fd = fd;
	pollfds[0].events = POLLIN;

	poll(pollfds, 1, -1);

	if(pollfds[0].revents & POLLIN){
		/* Poll until we receive a message from the PRU and then print it */
		ret = read(pollfds[0].fd, buf, MAX_BUFFER_SIZE);
	}

	return ret;
}


