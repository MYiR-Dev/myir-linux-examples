/******************************************************************************
* Copyright (C), 2016-2017, Sunny.Guo
* FileName: main.c 
* Author: Sunny.Guo
* Version: 1.0
* Date: 2017年 01月 19日 星期四 10:12:50 CST
* Description: Main entry   
*			   loopback module is from ti-processor-sdk-linux-am437x-evm-02.00.02.11/example-applications
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
#include "jpeg.h"
#include "loopback.h"

extern control_t status;


static const char *		version 		= "1.0";

static const char short_options[] = "h";

static const struct option long_options[] = {
			{"help",	no_argument,		NULL, 'h'},
			{0,		0,					0,  0}
};

static void usage(FILE *fp, int argc, char **argv)
{
	fprintf(fp,
			 "Usage: %s [options]\n\n"
			 "Version %s \n"
			 "Options:\n"
			 "-h | --help		   Print this message\n"
			 "",
			 argv[0], version);
}


void on_capture_clicked()
{
    status.jpeg=true;
}

void  on_switch_2_clicked()
{
    if (status.num_cams==1) status.main_cam=0;
    else if (status.main_cam==0) status.main_cam=1;
    else status.main_cam=0;
}

void  on_pip_clicked()
{
    if (status.num_cams==1) {
        status.pip=false;
    }
    else if (status.pip==true) {
        status.pip=false;
        drm_disable_pip();
    }
    else {
        status.pip=true;
    }
}

void  on_exit_clicked()
{
    status.exit=true;
}

static void *thread_func(void *arg){
    while(status.exit == false) {
        /* Display captured frame and sleep to control FPS */
        process_frame();
        usleep(30000);
    }
    end_streaming();
    exit_devices();
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

			case 'h':
				usage(stdout, argc, argv);
				exit(EXIT_SUCCESS);

			default:
				usage(stderr, argc, argv);
				exit(EXIT_FAILURE);
			}
		}

	pthread_t tid;
	int i;


    if(init_loopback() < 0) {
        /* TODO: make sure exit occurs properly */
    }
	pthread_create(&tid, NULL, thread_func, NULL);
	pthread_join(tid, NULL);
	printf(" Finish \r\n");
	return 0;
	}
