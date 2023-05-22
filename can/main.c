/******************************************************************************
* Copyright (C), 2016-2017, Sunny.Guo
* FileName: main.c 
* Author: Sunny.Guo
* Version: 1.0
* Date: 2017年 01月 19日 星期四 10:12:50 CST
* Description: Main entry         
*
* History:        
*  <author>  	<time>   	<version >   	<desc>
*  Sunny.Guo   	19/01/2017      1.0     	create this moudle  
*                                                                          
* Licensed under GPLv2 or later, see file LICENSE in this source tree.
*******************************************************************************/
#include <getopt.h>
#include <unistd.h>
#include <fcntl.h>

#include "common.h"
#include "can.h"

static const char *		version 		= "1.0";
static char *			dev_name 		= "can0";
static unsigned int		can_baudrate 	= CAN_DEFAULT_BAUDRATE;

static const char short_options[] = "d:b:w:lh";

static const struct option long_options[] = {
			{"device", 	required_argument, 	NULL, 'd'},
			{"baudrate",	required_argument,	NULL, 'b'},
			{"write",	required_argument,	NULL, 'w'},
			{"help",	no_argument,		NULL, 'h'},
			{"loop",	no_argument,		NULL, 'l'},
			{0,		0,					0,  0}
};

static void usage(FILE *fp, int argc, char **argv)
{
	fprintf(fp,
			 "Usage: %s [options]\n\n"
			 "Version %s \n"
			 "Options:\n"
			 "-d | --device name   can device name: %s\n"
			 "-b | --baudrate baudrate   set baudrate, default baudrate:%d \n"
			 "-l | --loop              operate circularly, default not operate circularly! \r\n"
			 "-w | --write  frame     frame string with format ID#MESSAGE. such as: 123#112233445566\r\n"
			 "-h | --help		   Print this message\n"
			 "",
			 argv[0], version, dev_name, can_baudrate);
}

int main(int argc, char **argv)
{
	int ret = 0;
	char *strWriteFrame = NULL;

	// operate circularly	
	int flgLoop = 0;

	// write mode  0: read only; 1: read and write
	int optWrite = 0; 

	if(argc < 2){
		usage(stdout, argc, argv);
		exit(EXIT_SUCCESS);
		}

	for(;;){
		int opt_idx;
		int opt_nxt;
		opt_nxt = getopt_long(argc, argv, short_options, long_options, NULL);
		
		if(opt_nxt < 0)
			break;

		switch(opt_nxt){
			case 0:
				break;
				
			case 'd':
				dev_name = optarg;
				break;

			case 'b':
				can_baudrate = atoi(optarg);
				break;

			case 'w':
				strWriteFrame = optarg;
				optWrite = 1;
				break;
				
			case 'l':
				flgLoop = 1;
				break;

			case 'h':
				usage(stdout, argc, argv);
				exit(EXIT_SUCCESS);

			default:
				usage(stderr, argc, argv);
				exit(EXIT_FAILURE);
			}
		}
	
	int can_fd = -1;
	struct can_frame  can_read_frame, can_write_frame;

	memset(&can_read_frame, 0, sizeof(struct can_frame));
	memset(&can_write_frame, 0, sizeof(struct can_frame));

	ret = can_setting(dev_name, can_baudrate, CAN_ENABLE);
	if( ret < 0){
		dbg_printf("Set can device %s failed!\r\n", dev_name);
		exit(EXIT_FAILURE);
		}
	
	can_fd = can_init(dev_name);
	if(can_fd <= 0){
		dbg_printf("Initialize can device %s failed!\r\n", dev_name);
		exit(EXIT_FAILURE);
		}

	if((optWrite == 1) &&(strWriteFrame != NULL)){
		ret = parse_canframe(strWriteFrame, &can_write_frame);
		if( ret <= 0){
			dbg_printf("Parse can frame  %s failed!\r\n", strWriteFrame);
			fprintf(stderr, "\nWrong CAN-frame format! Try:\r\n");
			fprintf(stderr, "  <can_id>#{R|data} for CAN 2.0 frames\n");
			fprintf(stderr, "  <can_id> can have 3 (SFF)  hex chars\n");
			fprintf(stderr, "  {data} has 0..8  ASCII hex-values (optionally");
			fprintf(stderr, "  separated by '.')\n");
			fprintf(stderr, "  e.g. 5A1#11.2233.44556677.88 / 123#DEADBEEF / 5AA# / \r\n ");
			can_setting(dev_name, can_baudrate, CAN_DISABLE);
			exit(EXIT_FAILURE);
			}
		}
		
	do{
		if(optWrite == 1){
			int i = 0;
			dbg_printf("====== write  frame: ======\r\n");
			dbg_printf(" frame_id = 0x%x \n", can_write_frame.can_id);
			dbg_printf(" frame_len = %d \n", can_write_frame.can_dlc);
			dbg_printf(" frame_data = ", can_write_frame.can_dlc);
			
			for(i=0; i <can_write_frame.can_dlc; i++){
				dbg_printf("%#x ", can_write_frame.data[i]);
				}
			dbg_printf("\n");
			dbg_printf("=========================== \n");
			sleep(3);

			ret = can_write(can_fd , &can_write_frame, sizeof(can_write_frame));
			
			if(ret <= 0){
				dbg_printf("Write can device %s failed!\r\n", dev_name);
				can_setting(dev_name, can_baudrate, CAN_DISABLE);
				exit(EXIT_FAILURE);
				}
				sleep(1);
			}

		ret = can_read(can_fd,&can_read_frame);
		if(ret > 0 ){
			// Can got data success
			int i = 0;
			dbg_printf("%s %#x [%d] " , dev_name, can_read_frame.can_id, can_read_frame.can_dlc);
			for(i = 0; i < can_read_frame.can_dlc; i++){
				dbg_printf("%#x ", can_read_frame.data[i]);
				}
			dbg_printf("\n");
			}
		else if(ret == 0){
			// Can read data timeout, try again
			//dbg_printf("== select can timout ===\r\n");
			}
		else if(ret < 0){
			// Can read data error!
			// dbg_printf("== read can error ===\r\n");
			}
		
		
		}while(flgLoop == 1);
	}
