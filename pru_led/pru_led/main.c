/******************************************************************************
* Copyright (C), 2016-2017, Sunny.Guo
* FileName: main.c 
* Author: Sunny.Guo
* Version: 1.0
* Date: 2017年 01月 19日 星期四 10:12:50 CST
* Description: Main entry         
* FunctionList:   
*  <num> <function>		<desc>
*  1	 main			write message with number 0~7 to pru, and read back. when the pru receive the message, 
						it will light the led corresponding to number.
* History:        
*  <author>  	<time>   	<version >   	<desc>
*  Sunny.Guo   	19/01/2017      1.0     	create this moudle  
*                                                                          
* Licensed under GPLv2 or later, see file LICENSE in this source tree.
*******************************************************************************/
#include <getopt.h>
#include <sys/poll.h>

#include "pru_led.h"

static const char *		version 		= "1.0";
static char *			dev_name 		= DEFAULT_DEVICE_NAME;

static const char short_options[] = "d:n:lh";

static const struct option long_options[] = {
			{"device", 	required_argument, 	NULL, 'd'},
			{"number", required_argument, NULL, 'n'},
			{"loop", no_argument, NULL, 'l'},
			{"help",	no_argument,		NULL, 'h'},
			{0,		0,					0,  0}
};

static void usage(FILE *fp, int argc, char **argv)
{
	fprintf(fp,
			 "Usage: %s [options]\n\n"
			 "Version %s \n"
			 "Options:\n"
			 "-d | --device name   pru rpmsg char device name %s\n"
			 "-l | --loop		operate circularly, default not operate circularly! \r\n"
			 "-n | --number num   number send to pru! must be 0~7.\n"
			 "-h | --help		   Print this message\n"
			 "",
			 argv[0], version, dev_name);
}

int main(int argc, char **argv)
{
	int ret = 0;

	// number to display. 
	char * led = NULL;
	
	// operate circularly	
	int flgLoop = 0;
	int loopCount = 1;
	int backCount = 0;
	
	char str[MAX_BUFFER_SIZE] = {'\0'};
	char readBuf[MAX_BUFFER_SIZE]={0};
	int i = 1;

	if(argc < 2){
		usage(stderr, argc, argv);
		exit(EXIT_FAILURE);
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

			case 'n':
				led = optarg;
				break;
				
			case 'l':
				flgLoop = 1;
				loopCount = NUM_MESSAGES;
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
	fd = pru_led_init(dev_name);
	
	/* The RPMsg channel exists and the character device is opened */
	dbg_printf("Opened %s, sending %d messages\n\n", dev_name, loopCount);
	
	
	while(i <= loopCount){
		memset(str, 0 , sizeof(str));
		if(flgLoop == 1){
			sprintf(str,"%d", i%8);
			}
		else{
			sprintf(str,"%s", led);
			}
		
		ret = pru_led_set(fd, str);
		if (ret > 0){
			dbg_printf("%d - Sent to PRU: %s\n", i, str);
			}

		sleep(1);
		/* Poll until we receive a message from the PRU and then print it */
		memset(readBuf, 0 , sizeof(readBuf));
		ret = pru_rpmsg_read(fd, readBuf);
		if (ret > 0){
			++backCount;
			dbg_printf("%d - Received from PRU:%s\n\n", backCount, readBuf);
			}
		
		i++;
		
		}

	/* Received all the messages the example is complete */
	dbg_printf("Received %d messages, closing %s\n", backCount, dev_name);

	/* Close the rpmsg_pru character device file */
	close(fd);

	return 0;

}
