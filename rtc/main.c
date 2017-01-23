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
#include "rtc.h"

static const char *		version 		= "1.0";
static char *			dev_name 		= "/dev/rtc";

static const char short_options[] = "d:w:h";

static const struct option long_options[] = {
			{"device", 	required_argument, 	NULL, 'd'},
			{"write",	required_argument,	NULL, 'w'},
			{"help",	no_argument,		NULL, 'h'},
			{0,		0,					0,  0}
};

static void usage(FILE *fp, int argc, char **argv)
{
	fprintf(fp,
			 "Usage: %s [options]\n\n"
			 "Version %s \n"
			 "Options:\n"
			 "-d | --device name   tty device name: %s\n"
			 "-w | --write time   time string with format MMDDhhmm[CCYY][.ss]. such as: 111817582016.18\r\n"
			 "-h | --help		   Print this message\n"
			 "",
			 argv[0], version, dev_name);
}

int main(int argc, char **argv)
{
	int ret = 0;
	dev_name = "/dev/rtc";
	char *strWriteFrame = NULL;

	// write mode  0: read only; 1: read and write
	int optWrite = 0; 

//	if(argc < 2){
//		usage(stdout, argc, argv);
//		exit(EXIT_SUCCESS);
//		}

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
				
			case 'w':
				strWriteFrame = optarg;
				if((strWriteFrame == NULL)|| (strlen(strWriteFrame)<1)){
					fprintf(stderr, "\n No data to write!\n");
					exit(EXIT_FAILURE);
					}
				optWrite = 1;
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

	fd = rtc_init(dev_name);
	if( fd < 0){
		dbg_printf("Initialize rtc device %s failed!\r\n", dev_name);
		exit(EXIT_FAILURE);
		}
	
	do{
		if(optWrite == 1){
			int hour, minute, second, year, month, day;

			ret = parse_time(strWriteFrame, &hour,&minute,&second,&year,&month,&day);
			if(ret < 0 ){
				fprintf(stderr, "\nWrong time format, please try:\n"
								" Time string with format MMDDhhmm[CCYY][.ss]. such as: 111817582016.18 \n");
				exit(EXIT_FAILURE);
				}
			
			ret = rtc_set_time(fd , hour, minute, second, year, month, day);
			
			if(ret < 0){
				dbg_printf("set time to  device %s failed!\r\n", dev_name);
				close(fd);
				exit(EXIT_FAILURE);
				}
			
			dbg_printf("date/time is updated to:  %d-%d-%d, %02d:%02d:%02d.\n\n",
					day, month, year, hour, minute, second);
			close(fd);
			}
		else if(optWrite == 0){
			struct rtc_time time;
			ret = rtc_read_time(fd, &time);
			if(ret < 0){
				dbg_printf("read time from rtc %s failed!\r\n", dev_name);
				close(fd);
				exit(EXIT_FAILURE);
				}
			
			dbg_printf("Current RTC date/time is %d-%d-%d, %02d:%02d:%02d.\n\n",
					time.tm_mday, time.tm_mon + 1, time.tm_year + 1900,
					time.tm_hour, time.tm_min, time.tm_sec);
			close(fd);
			}
		}while(0);
	}
