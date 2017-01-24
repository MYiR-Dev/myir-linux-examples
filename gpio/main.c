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
#include <errno.h>

#include "common.h"
#include "gpio.h"

static const char *		version 		= "1.0";

static const char short_options[] = "n:s:l:gh";

static const struct option long_options[] = {
			{"number", 	required_argument, 	NULL, 'n'},
			{"set", required_argument, NULL, 's'},
			{"get", no_argument, NULL, 'g'},
			{"help",	no_argument,		NULL, 'h'},
			{0,		0,					0,  0}
};

static void usage(FILE *fp, int argc, char **argv)
{
	fprintf(fp,
			 "Usage: %s [options]\n\n"
			 "Version %s \n"
			 "Options:\n"
			 "-n | -- number gpio	 gpio number. \n"
			 "-g | -- get            get gpio level.\n"
			 "-s | -- set  level     set gpio level. 0: low; 1: high\n"
			 "-h | --help		     Print this message\n"
			 "",
			 argv[0], version);
}

int main(int argc, char **argv)
{
	int ret = 0;
	int level = 0;
	int optMode = GPIO_GET;
	int gpio_number = -1;
	
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
				
			case 'n':
				gpio_number = atoi(optarg);
				if(gpio_number <= 0){
					fprintf(stderr, "\nWrong gpio number! please input a integer value of gpio number. eg. gpio3_7 = 32*3+7\n");
					usage(stdout, argc, argv);
					exit(EXIT_SUCCESS);
					}
				break;

			case 'g':
				optMode = GPIO_GET;
				break;
				
			case 's':
				optMode = GPIO_SET;
				if(strlen(optarg) == 1 &&( optarg[0]=='0' || optarg[0]=='1')){
					level = atoi(optarg);
					}
				else{
					fprintf(stderr, "\nWrong level, please try: 0 - low; 1 - high\n");
					usage(stdout, argc, argv);
					exit(EXIT_SUCCESS);
					}
				break;

			case 'h':
				usage(stdout, argc, argv);
				exit(EXIT_SUCCESS);

			default:
				usage(stderr, argc, argv);
				exit(EXIT_FAILURE);
			}
		}
	
	ret = gpio_export(gpio_number);
	if (ret < 0) {
		dbg_perror("export gpio");
		if(errno != EBUSY){
			return -1;
			}
	}

	// read back direction, "in" or "out"
	char	dir_get[4]={'\0'};

	// read back level '0': low; '1': high
	char	clevel = 0;
	
	do{
		if(optMode == GPIO_SET){
			ret = gpio_set_direction(gpio_number, GPIO_OUT);
			if (ret < 0) {
				dbg_perror("set gpio out");
				return -1;
			}
			
			ret = gpio_set_level(gpio_number, level);
			if (ret < 0) {
				dbg_perror("set gpio level");
				return -1;
			}

			memset(dir_get, 0, sizeof(dir_get));
			
			ret = gpio_get_direction(gpio_number, dir_get);
			if (ret < 0) {
				dbg_perror("get gpio direction");
				return -1;
			}
			ret = gpio_get_level(gpio_number, &clevel);
			dbg_printf("==gpio%d_%d  direction is %s\n", gpio_number/32, gpio_number%32, dir_get);
			dbg_printf("==gpio%d_%d  level is %s\n",  gpio_number/32, gpio_number%32, clevel==48?"low":"high");
			dbg_printf("Set gpio%d_%d level %s", gpio_number/32, gpio_number%32, level==0?"low":"high" );
			}
		else{
			ret = gpio_get_direction(gpio_number, dir_get);
			if (ret < 0) {
				dbg_perror("get gpio direction");
				return -1;
			}
			
			ret = gpio_get_level(gpio_number, &clevel);
			if (ret < 0) {
				dbg_perror("get gpio level");
				return -1;
			}
			
			dbg_printf("==gpio%d_%d  direction is %s\n", gpio_number/32, gpio_number%32, dir_get);
			dbg_printf("==gpio%d_%d  level is %s\n",  gpio_number/32, gpio_number%32, clevel==48?"low":"high");
			
			if (clevel == '0'){
				dbg_printf("Get gpio%d_%d level %s", gpio_number/32, gpio_number%32,  "low" );
				}
			else if(clevel == '1'){
				dbg_printf("Get gpio%d_%d level %s", gpio_number/32, gpio_number%32,  "high" );
				}
			}
		if(ret < 0){
			dbg_printf(" failed!\n");
			}
		else{
			dbg_printf(" success!\n");
			}

		// unexport gpio after operation, the gpio direction and level will change after unexport.
		ret = gpio_unexport(gpio_number);
		if (ret < 0) {
			dbg_perror("unexport gpio");
			return -1;
		}
	}while(0);
}
