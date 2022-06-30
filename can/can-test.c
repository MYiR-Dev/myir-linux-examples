/*****************************************************************************
 * Copyright (c) 2014-2017 MYIR Tech Ltd.
 *        File: can-test.c
 *        Date: 2014/11/3
 *      Author: Kevin Su
 * Description: A demo program to show how to transmit/receive data with
 *              socket can interface on CAN bus.
 *				Please note that, this demo needs two boards to run as
 *				transmitter and receiver.
 *				Before run "can-test", we need to config the bitrate with
 *              "ip" command:
 *              # ip link set can0 up type can bitrate 250000
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include <linux/can.h>
#include <linux/can/raw.h>

#define DEBUG	1

#define ERR_MSG(fmt, args...)	fprintf(stderr, fmt, ##args)
#ifdef DEBUG
	#define DBG_MSG(fmt, args...)	fprintf(stdout, fmt, ##args)
#else
	#define DBG_MSG(fmt, args...)
#endif

#ifndef PF_CAN
	#define PF_CAN 29
#endif

#ifndef AF_CAN
	#define AF_CAN PF_CAN
#endif

int main(int argc, char *argv[])
{
	int fd, ret, flag, len;
	char senddata[32] = "test";
	struct sockaddr_can addr;
	struct ifreq ifr;
	struct can_frame frame;
	socklen_t socket_len = sizeof(struct sockaddr_can);

	/* Create a socket with PF_CAN family, SOCK_RAW and CAN_RAW protocol */
	fd = socket(PF_CAN, SOCK_RAW, CAN_RAW); 
	if (fd < 0) {
		ERR_MSG("Open socket failed!\n");
		return fd;
	}

	/* Use can0 */
	strcpy((char *)(ifr.ifr_name), "can0");
	
	/* Get information */
	ret = ioctl(fd, SIOCGIFINDEX, &ifr);
	if (ret != 0) {
		ERR_MSG("SIOCGIFINDEX failed! ret:%d\n", ret);
		close(fd);
		return ret;
	}
	DBG_MSG("can0 can_ifindex = %x\n",ifr.ifr_ifindex);

	/* Disable loopback */
	flag = 0;
    ret = setsockopt(fd, SOL_CAN_RAW, CAN_RAW_LOOPBACK, &flag, sizeof(flag));
	if (ret != 0) {
		ERR_MSG("Set loopback disable failed! ret:%d\n", ret);
		close(fd);
		return ret;
	}
	DBG_MSG("Set can0 loopback disable\n");

	/* Disable receiving own message */
	flag = 0;
    ret = setsockopt(fd, SOL_CAN_RAW, CAN_RAW_RECV_OWN_MSGS, 
               &flag, sizeof(flag));
	if (ret != 0) {
		ERR_MSG("Disable receiving own message failed! ret:%d\n", ret);
		close(fd);
		return ret;
	}
	DBG_MSG("Disable receiving own message\n");

	/* Use AF_CAN protocol family */
	addr.can_family = AF_CAN;
	addr.can_ifindex = ifr.ifr_ifindex;
	
	/* Binding socket */
	ret = bind(fd, (struct sockaddr*)&addr, sizeof(addr));
	if (ret != 0) {
		ERR_MSG("Bind socket failed! ret:%d\n", ret);
		close(fd);
		return ret;
	}
	DBG_MSG("Bind can0 socket\n");

	frame.can_id = 0x123;
	len = strlen(senddata);
	
	while (1) {
		strncpy((char *)frame.data, senddata, len);
		frame.can_dlc = len;
		ret = sendto(fd, &frame, sizeof(struct can_frame), 0, (struct sockaddr*)&addr, sizeof(addr));
		if (ret > 0) {
			DBG_MSG("Send success: [%s], ret=%d\n", senddata, ret);
			ret = recvfrom(fd, &frame, sizeof(struct can_frame), 0, (struct sockaddr *)&addr, &socket_len);
			if (ret > 0) {
				DBG_MSG("Recv message: [%s], ret=%d\n", frame.data, ret);
			}
		}
		usleep(500000);
	}

	return 0;
}
