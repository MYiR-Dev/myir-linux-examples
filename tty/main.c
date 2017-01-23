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
#include "tty.h"

static const char *		version 		= "1.0";
static char *			dev_name 		= "/dev/ttyO0";
static unsigned int		tty_baudrate 	= TTY_DEFAULT_BAUDRATE;

static const char short_options[] = "d:m:b:w:flh";

static const struct option long_options[] = {
			{"device", 	required_argument, 	NULL, 'd'},
			{"mode", required_argument, NULL, 'm'},
			{"flow", no_argument, NULL, 'f'},
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
			 "-d | --device name   tty device name: %s\n"
			 "-m | --mode mode 	 tty mode. 0: RS232, 1: RS485 default mode: 0\n"
			 "-f | --flow 		flow control\n"
			 "-b | --baudrate baudrate   set baudrate, default baudrate:%d \n"
			 "-l | --loop              operate circularly, default not operate circularly! \r\n"
			 "-w | --write frame 	frame string. such as: 0123456789\r\n"
			 "-h | --help		   Print this message\n"
			 "",
			 argv[0], version, dev_name, tty_baudrate);
}

int main(int argc, char **argv)
{
	int ret = 0;
	dev_name = "/dev/ttyO0";
	char *strWriteFrame = NULL;

	// operate circularly	
	int flgLoop = 0;

	// flow control
	int flgFlow = 0;

	// tty mode
	int ttyMode = TTY_RS232_MODE;

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
				tty_baudrate = atoi(optarg);
				break;

			case 'f':
				flgFlow = 1;
				break;

			case 'm':
				ttyMode = atoi(optarg);
				if((ttyMode != TTY_RS485_MODE)&&(ttyMode!= TTY_RS232_MODE)){
					fprintf(stderr, "\n Wrong TTY Mode, please try:\n"
									"  0  -- RS232\n"
									"  1  -- RS485\n");
					exit(EXIT_FAILURE);
					}
				break;
				
			case 'w':
				strWriteFrame = optarg;
				if((strWriteFrame == NULL)|| (strlen(strWriteFrame)<1)){
					fprintf(stderr, "\n No data to write!\n");
					exit(EXIT_FAILURE);
					}
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
	
	int tty_fd = -1;
	char tty_read_back[TTY_READ_BUFFER_SIZE];
	memset(&tty_read_back, 0, TTY_READ_BUFFER_SIZE);

	tty_fd = tty_init(dev_name);
	if( tty_fd < 0){
		dbg_printf("Initialize tty device %s failed!\r\n", dev_name);
		exit(EXIT_FAILURE);
		}
	
	ret = tty_setting(tty_fd, tty_baudrate, 8, ttyMode, flgFlow, 'n', 1);
	if(ret < 0){
		dbg_printf("setting tty device %s failed!\r\n", dev_name);
		exit(EXIT_FAILURE);
		}

	do{
		if(optWrite == 1){
			int i = 0;
			dbg_printf("SEND:%s\r\n", strWriteFrame);
			sleep(3);

			ret = tty_write(tty_fd , strWriteFrame, strlen(strWriteFrame));
			
			if(ret <= 0){
				dbg_printf("Write tty device %s failed!\r\n", dev_name);
				close(tty_fd);
				exit(EXIT_FAILURE);
				}
				sleep(1);
			}

		ret = tty_read(tty_fd, tty_read_back);
		if(ret > 0 ){
			// got data success
			dbg_printf("RECV:%s, total:%d \n", tty_read_back, ret);
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
