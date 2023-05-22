/******************************************************************************
* Copyright (C), 2016-2017, Sunny.Guo
* FileName: can.h 
* Author: Sunny.Guo
* Version: 1.0
* Date: 2017年 01月 19日 星期四 10:12:50 CST
* Description: can definition         
*
* History:        
*  <author>  	<time>   	<version >   	<desc>
*  Sunny.Guo   	19/01/2017      1.0     	create this moudle  
*                                                                          
* Licensed under GPLv2 or later, see file LICENSE in this source tree.
*******************************************************************************/
#ifndef __CAN_H__

#include <sys/ioctl.h>
#include <sys/mman.h>
#include <sys/types.h>

#include <linux/can.h>
#include <linux/can/raw.h>

#include "common.h"


#define CAN_ENABLE				1
#define CAN_DISABLE				0

#define CAN_DEFAULT_BAUDRATE  	50000
#define CAN_READ_TIMEOUT_USEC	5000

#define DATA_SEPERATOR 			'.'
#define CANID_DELIM 			'#'

struct can_dev
{
	int	fd;
	char can_name[MAX_INPUT];
	int bitrate;
};
typedef struct can_dev can_dev_t;



/******************************************************************************
  Function:       can_init
  Description:    initialize can device
  Input:          can    	--  can device name, such as 'can0', 'can1'
                   
  Output:          
  Return:         int		-- return the can fd
  Others:         NONE
*******************************************************************************/
int				can_init(const char * can);

/******************************************************************************
  Function:       can_setting
  Description:    set the can device's bitrate and status
  Input:          can      --  can name string
  				  bitrate  --  can bitrate
  				  enable   --  can status, 1: enable; 0: disable
                  
  Output:          
  Return:         int 	   --  can setting status 0:success 
  Others:         NONE
*******************************************************************************/
int 			can_setting(const char* can, const int bitrate, int enable);

int				can_read(int fd, struct can_frame *frame);
int				can_write(int fd, struct can_frame *frame, int len);

/******************************************************************************
  Function:       parse_canframe
  Description:    Transfers a valid ASCII string describing CAN frame into struct can_frame.
  Input:          strFrame --  ASCII string decribing CAN frame
  				  frame	   --  a reference to a struct can_frame
                  
  Output:          
  Return:         int 	   --  dlc of the can_frame 
  Others:         The valid ASCII string describing CAN frame is CAN 2.0 frames with string layout <can_id>#{R{len}|data}
				  can_id can have 3 (standard frame format) hexadecimal chars.
				  {data} has 0 to 8 hex-values that can (optionally) be separated by '.'
				  {len} can take values from 0 to 8 and can be omitted if zero
				    
*******************************************************************************/
int				parse_canframe(char* strFrame, struct can_frame *frame);

#endif		
