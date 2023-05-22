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
#include <linux/fb.h>

#include "common.h"
#include "framebuffer.h"

static const char *		version = "1.0";

static char *			dev_name = "/dev/fb0";

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
			 "-d | --device name   framebuffer device name [%s]\n"
			 "-h | --help		   Print this message\n"
			 "",
			 argv[0], version, dev_name);
}

int main(int argc, char **argv)
{
	int ret = 0;
	fb_dev_t *myir_fb = NULL;

	unsigned int w,h;
	point_t p0, p1, p2, p3, p4;
	color_t color;
	
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
	
	myir_fb = malloc(sizeof(fb_dev_t));
	if(myir_fb == NULL){
		dbg_perror("Framebuffer device malloc error, not enough memory!\r\n");
		exit(EXIT_FAILURE);
		}

	ret = fb_init(dev_name, myir_fb);
	if(ret < 0 ){
		dbg_perror("Framebuffer device init failed!\r\n");
		exit(EXIT_FAILURE);
		}
	while(1){	
		sleep(1);
		fill_background(myir_fb, COLOR_RED);
		sleep(1);
		fill_background(myir_fb, COLOR_GREEN);
		sleep(1);
		fill_background(myir_fb, COLOR_BLUE);
		sleep(1);
		fill_background(myir_fb, COLOR_WHITE);
		sleep(1);
		fill_background(myir_fb, COLOR_BLACK);
		sleep(1);

		w = get_fb_width(myir_fb);
		h = get_fb_height(myir_fb);
		
		p0.x = w/2;
		p0.y = h/2;

		p1.x = 0;
		p1.y = 0;

		p2.x = w;
		p2.y = h;

		p3.x = 0;
		p3.y = h;

		p4.x = w;
		p4.y = 0;
	
		fill_square(myir_fb, p1, p0, COLOR_RED);
		fill_square(myir_fb, p2, p0, COLOR_GREEN);
		fill_square(myir_fb, p3, p0, COLOR_BLUE);
		fill_square(myir_fb, p4, p0, COLOR_WHITE);
	
		draw_line(myir_fb, p1, p0, COLOR_YELLOW);
		draw_line(myir_fb, p2, p0, COLOR_YELLOW);
		draw_line(myir_fb, p3, p0, COLOR_YELLOW);
		draw_line(myir_fb, p4, p0, COLOR_YELLOW);
	}
}
