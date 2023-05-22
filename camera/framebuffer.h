/******************************************************************************
* Copyright (C), 2016-2017, Sunny.Guo
* FileName: framebuffer.h 
* Author: Sunny.Guo
* Version: 1.0
* Date: 2017年 01月 19日 星期四 10:12:50 CST
* Description: framebuffer definition         
*
* History:        
*  <author>  	<time>   	<version >   	<desc>
*  Sunny.Guo   	19/01/2017      1.0     	create this moudle  
*                                                                          
* Licensed under GPLv2 or later, see file LICENSE in this source tree.
*******************************************************************************/
#ifndef __FRAMEBUFFER_H__

#include <linux/fb.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <sys/types.h>

#include "common.h"

#define COLOR_RED			0xff0000
#define COLOR_GREEN			0x00ff00
#define COLOR_BLUE			0x0000ff
#define COLOR_YELLOW		0xffff00

#define COLOR_WHITE			0xffffff
#define COLOR_BLACK			0x000000

struct fb_dev
{
	int	fd;
	unsigned int width;
	unsigned int height;
	void *fb_mem;
	unsigned int bpp;
};
typedef struct fb_dev fb_dev_t;

struct point
{
	unsigned int x;
	unsigned int y;
};
typedef struct point point_t;

struct color
{
	unsigned int red;
	unsigned int green;
	unsigned int blue;
};
typedef struct color color_t;


/******************************************************************************
  Function:       set_color
  Description:    convert a int type color to color_t type
  Input:          color    	--  pointer to a color_t type color
                  c   		--  int type color
  Output:          
  Return:         void
  Others:         NONE
*******************************************************************************/
void			set_color(color_t *color, unsigned int c);

/******************************************************************************
  Function:       get_fb_mem
  Description:    get the framebuffer start memory mapped to userspace
  Input:          fb_dev    --  pointer to a fb_dev_t type framebuffer device handle
                  
  Output:          
  Return:         a void pointer to framebuffer start memory
  Others:         NONE
*******************************************************************************/
void* 			get_fb_mem(fb_dev_t *fb_dev);
int				get_fb_height(fb_dev_t *fb_dev);
int 			get_fb_width(fb_dev_t *fb_dev);
int				get_fb_bpp(fb_dev_t *fb_dev);
int				fb_open(const char *dev_path, fb_dev_t *fb_dev);
int 			fb_init(const char *dev_path, fb_dev_t *fb_dev);
void			fb_close(fb_dev_t *fb_dev);
void 			draw_point(fb_dev_t *fb_dev, point_t p1, unsigned int c);
void			draw_line(fb_dev_t *fb_dev, point_t p1, point_t p2, unsigned int c);
void			draw_square(fb_dev_t *fb_dev, point_t p1, point_t p2, unsigned int c);
void			fill_square(fb_dev_t *fb_dev, point_t p1, point_t p2, unsigned int c);
void 			fill_background(fb_dev_t *fb_dev, unsigned int c);

#endif		// __FRAMEBUFFER_H__
