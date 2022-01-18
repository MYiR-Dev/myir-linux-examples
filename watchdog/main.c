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
*  Sunny.Guo   	22/04/2017      1.0     	create this moudle  
*                                                                          
* Licensed under GPLv2 or later, see file LICENSE in this source tree.
*******************************************************************************/
#include <getopt.h>
#include <unistd.h>
#include <fcntl.h>

#include "common.h"
#include "watchdog.h"

static const char *		version 		= "1.0";

static const char short_options[] = "i:t:hed";

static const struct option long_options[] = {
			{"index", 	required_argument,	NULL, 'i'},
			{"timeout", 	required_argument, 	NULL, 't'},
			{"enable", 	no_argument, 		NULL, 'e'},
			{"disable", 	no_argument, 		NULL, 'd'},
			{"help",	no_argument,		NULL, 'h'},
			{0,		0,					0,  0}
};

static void usage(FILE *fp, int argc, char **argv)
{
	fprintf(fp,
			 "Usage: %s [options]\n\n"
			 "Version %s \n"
			 "Options:\n"
			 "-i | --index  id	  watchdog index (0: omap_watchdog, 1: myir_watchdog). \n"
			 "-t | --timeout  time   watchdog timeout (ms). \n"
			 "-h | --help		   Print this message\n"
			 "",
			 argv[0], version);
}

void catch_int(int signum)
{
	signal(SIGINT, catch_int);

	printf("In signal handler\n");
	if(0 != watchdog_close(0))
		printf("Close failed in signal handler\n");
	else
		printf("Close succeeded in signal handler\n");
	if(0 != watchdog_close(1))
		printf("Close watchdog1 failed in signal handler\n");
	else
		printf("Close watchdog1 succeeded in signal handler\n");
}

int main(int argc, char **argv)
{
	int ret = 0;
	int index = 0;
	int timeout = 200;
	signal(SIGINT, catch_int);

	for(;;){
		int opt_idx;
		int opt_nxt;
		opt_nxt = getopt_long(argc, argv, short_options, long_options, NULL);
		
		if(opt_nxt < 0)
			break;

		switch(opt_nxt){
			case 0:
				break;

			case 'i':
				index = atoi(optarg);
				break;
			case 't':
				timeout = atoi(optarg);
				break;
			case 'e':
				watchdog_enable(index);
				break;
			case 'd':
				watchdog_disable(index);
				exit(EXIT_SUCCESS);
			case 'h':
				usage(stdout, argc, argv);
				exit(EXIT_SUCCESS);

			default:
				usage(stderr, argc, argv);
				exit(EXIT_FAILURE);
			}
		}
	
	do{
 		dbg_printf("Set  watchdog %d timeout  = %d \n", index, timeout );

		watchdog_set_timeout(index, timeout);	
		}while(0);
	}
