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
#include "keypad.h"

static const char *		version 		= "1.0";
static char *			dev_name 		= "/dev/input/event0";

static const char short_options[] = "d:h";

static const struct option long_options[] = {
			{"device", 	required_argument, 	NULL, 'd'},
			{"help",	no_argument,		NULL, 'h'},
			{0,		0,					0,  0}
};

static void usage(FILE *fp, int argc, char **argv)
{
	fprintf(fp,
			 "Usage: %s [options]\n\n"
			 "Version %s \n"
			 "Options:\n"
			 "-d | --device name   keypad input device name: %s\n"
			 "-h | --help		   Print this message\n"
			 "",
			 argv[0], version, dev_name);
}

int main(int argc, char **argv)
{
	int ret = 0;

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

			case 'h':
				usage(stdout, argc, argv);
				exit(EXIT_SUCCESS);

			default:
				usage(stderr, argc, argv);
				exit(EXIT_FAILURE);
			}
		}
	
	int fd = -1;
	struct input_event ev;

	fd = keypad_init(dev_name);
	if( fd < 0){
		dbg_printf("Initialize keypad input device %s failed!\r\n", dev_name);
		exit(EXIT_FAILURE);
		}
	
	do{
			ret = keypad_monitor(fd, &ev);
			if(ret < 0){
				dbg_printf("read keypad input event from  %s failed!\r\n", dev_name);
				close(fd);
				exit(EXIT_FAILURE);
				}
			
			dbg_printf("Event: Code = %d, Type = %d, Value=%d\n",
				ev.code, ev.type, ev.value);
		}while(1);
	}
