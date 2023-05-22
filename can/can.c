/******************************************************************************
* Copyright (C), 2016-2017, Sunny.Guo
* FileName: can.c 
* Author: Sunny.Guo
* Version: 1.0
* Date: 2017年 01月 19日 星期四 10:12:50 CST
* Description: can operation functions
*
* History:        
*  <author>  	<time>   	<version >   	<desc>
*  Sunny.Guo   	19/01/2017      1.0     	create this moudle  
*                                                                          
* Licensed under GPLv2 or later, see file LICENSE in this source tree.
*******************************************************************************/
#include <sys/socket.h>
#include <sys/types.h>
#include <netdb.h>
#include <netinet/in.h>
#include <net/if.h>

#include "common.h"
#include "can.h"


static unsigned char asc2nibble(char c) {

	if ((c >= '0') && (c <= '9'))
		return c - '0';

	if ((c >= 'A') && (c <= 'F'))
		return c - 'A' + 10;

	if ((c >= 'a') && (c <= 'f'))
		return c - 'a' + 10;

	return 16; /* error */
}


/******************************************************************************
  Function:       can_init
  Description:    initialize can device
  Input:          can    	--  can device name, such as 'can0', 'can1'
                   
  Output:          
  Return:         int		-- return the can fd
  Others:         NONE
*******************************************************************************/
int				can_init(const char * can)
{
	int canfd;
	struct ifreq ifr;
	struct sockaddr_can can_addr;
	int loopback = 0;
	int flags;

	memset(&ifr, 0, sizeof(ifr));
	
	canfd = socket(PF_CAN, SOCK_RAW, CAN_RAW);
	if (canfd < 0) {
		dbg_perror("socket");
		return canfd;
	}
	
	strcpy(ifr.ifr_name, can);

	/* Determine the interface index */
	ioctl(canfd, SIOCGIFINDEX, &ifr);
	can_addr.can_family = AF_CAN;
	can_addr.can_ifindex = ifr.ifr_ifindex;
	
  	flags = fcntl(canfd,F_GETFL,0);
	fcntl(canfd,F_SETFL,O_NONBLOCK|flags);

	/* bin the socket to a CAN interface */
	bind(canfd, (struct sockaddr *)&can_addr, sizeof(can_addr));
	
	/* Set the lookback rules */
	setsockopt(canfd, SOL_CAN_RAW, CAN_RAW_LOOPBACK,
			   &loopback, sizeof(loopback));
	
	return canfd;
}

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
int 			can_setting(const char* can, const int bitrate, int enable)
{
	int ret = 0;
	char cmdline[128] = { '\0' };

	if(enable == 1){
		sprintf(cmdline,
				"ip link set %s down; ip link set %s type can bitrate %ld triple-sampling on; ip link set %s up",
				can, can, bitrate, can);
		}
	else if(enable == 0){
		sprintf(cmdline,
				"ip link set %s down",
				can);
		}

	ret = system(cmdline);
    sleep(3);
	return WEXITSTATUS(ret);
}

int				can_read(int fd, struct can_frame *frame)
{
	struct timeval tv;
	fd_set readfds;
	
	int ret = 0;
	unsigned int timeout = CAN_READ_TIMEOUT_USEC;
	
	tv.tv_sec = 0;
	tv.tv_usec = timeout;

	FD_ZERO(&readfds);
	FD_SET(fd, &readfds);

	if(timeout > 0){
		ret = select(fd+1, &readfds, NULL, NULL, &tv);

		if(ret < 0 ){
			dbg_perror("select can fd\n");
		}
		else if(ret == 0){
			//dbg_printf("select can timeout\n");
			}
		}
	
	if(FD_ISSET(fd, &readfds)){
		ret = read(fd, frame, sizeof(struct can_frame));
		}

	return ret;
}

int				can_write(int fd, struct can_frame *frame, int len)
{
	int ret = 0;

	ret = write(fd, frame, len);

	return ret;
}

int				parse_canframe(char* strFrame, struct can_frame *frame)
	{
		int i, idx, dlen, len;
		int maxdlen = CAN_MAX_DLEN;
		int ret = CAN_MTU;
		unsigned char tmp;
	
		len = strlen(strFrame);

		memset(frame, 0, sizeof(*frame)); /* init CAN FD frame, e.g. LEN = 0 */
	
		if (len < 4)
			return 0;
	
		if (strFrame[3] == CANID_DELIM) { /* 3 digits */
	
			idx = 4;
			for (i=0; i<3; i++){
				if ((tmp = asc2nibble(strFrame[i])) > 0x0F)
					return 0;
				frame->can_id |= (tmp << (2-i)*4);
			}
		} else
			return 0;
	
		if((strFrame[idx] == 'R') || (strFrame[idx] == 'r')){ /* RTR frame */
			frame->can_id |= CAN_RTR_FLAG;
	
			/* check for optional DLC value for CAN 2.0B frames */
			if(strFrame[++idx] && (tmp = asc2nibble(strFrame[idx])) <= CAN_MAX_DLC)
				frame->can_dlc = tmp;
	
			return ret;
		}
	
		for (i=0, dlen=0; i < maxdlen; i++){
	
			if(strFrame[idx] == DATA_SEPERATOR) /* skip (optional) separator */
				idx++;
	
			if(idx >= len) /* end of string => end of data */
				break;
	
			if ((tmp = asc2nibble(strFrame[idx++])) > 0x0F)
				return 0;
			frame->data[i] = (tmp << 4);
			if ((tmp = asc2nibble(strFrame[idx++])) > 0x0F)
				return 0;
			frame->data[i] |= tmp;
			dlen++;
		}
		frame->can_dlc = dlen;
		
// for debug only
//
//		dbg_printf("=== parsed frame===== \n");
//		dbg_printf(" frame_id = 0x%x \n", frame->can_id);
//		dbg_printf(" frame_len = %d \n", frame->can_dlc);
//		
//		for(i=0; i <frame->can_dlc; i++){
//			dbg_printf("%#x ", frame->data[i]);
//			}
//		dbg_printf("\n");
//		dbg_printf("================= \n");
//	
		return ret;
	}


