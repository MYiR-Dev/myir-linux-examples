/******************************************************************************
* Copyright (C), 2016-2017, Sunny.Guo
* FileName: framebuffer.c 
* Author: Sunny.Guo
* Version: 1.0
* Date: 2017年 01月 19日 星期四 10:12:50 CST
* Description: framebuffer functions
*
* History:        
*  <author>  	<time>   	<version >   	<desc>
*  Sunny.Guo   	19/01/2017      1.0     	create this moudle  
*                                                                          
* Licensed under GPLv2 or later, see file LICENSE in this source tree.
*******************************************************************************/

#include "common.h"
#include "framebuffer.h"

void set_color(color_t *color, unsigned int c)
{
	if(color!=NULL){
		memset(color, 0, sizeof(color_t));
		color->red = (c >> 16) & 0xFF;
		color->green =(c >> 8) & 0xFF;
		color->blue = c & 0xFF;
		}
}

void* get_fb_mem(fb_dev_t *fb_dev)
{
	return fb_dev->fb_mem;
}

int get_fb_height(fb_dev_t *fb_dev)
{
	return fb_dev->height;
}

int get_fb_width(fb_dev_t *fb_dev)
{
	return fb_dev->width;
}

int get_fb_bpp(fb_dev_t *fb_dev)
{
	return fb_dev->bpp;
}

int fb_open(const char *dev_path, fb_dev_t *fb_dev)
{
	struct fb_fix_screeninfo fix_info;
	struct fb_var_screeninfo var_info;

	int fb_fd = -1;
	void *start_mem = NULL;
	
	if(fb_dev == NULL){
		return -1;
		}

	fb_fd = open(dev_path, O_RDWR);

	if(fb_fd < 0){
		dbg_printf("Can not open %s\r\n", dev_path);
		return -1;
		}

	fb_dev->fd = fb_fd;
	
    if (ioctl(fb_fd, FBIOGET_FSCREENINFO, &fix_info) < 0){
        dbg_perror("failed to get fb0 info");
        fb_close(fb_dev);
        return -1;
    	}

    if (ioctl(fb_fd, FBIOGET_VSCREENINFO, &var_info) < 0){
        dbg_perror("failed to get fb0 info");
        fb_close(fb_dev);
        return -1;
    	}

    start_mem = mmap(0, fix_info.smem_len, PROT_READ | PROT_WRITE, MAP_SHARED, fb_fd, 0);
    if (start_mem == MAP_FAILED){
        dbg_perror("failed to mmap framebuffer");
        fb_close(fb_dev);
        return -1;
    	}

	fb_dev->width = var_info.xres;
	fb_dev->height = var_info.yres;
	fb_dev->bpp = var_info.bits_per_pixel ;
	fb_dev->fb_mem = start_mem;

	return 0;	
}

int fb_init(const char *dev_path, fb_dev_t *fb_dev)
{
	int ret = 0;
	if(fb_dev != NULL){
		ret = fb_open(dev_path, fb_dev);
		if(ret < 0)
		{
			dbg_printf("Framebuffer init error on %s\r\n", dev_path);
			return -1;
		}
		dbg_printf("xres:%d >>> yres:%d >>> bpp:%d>>>\r\n",get_fb_width(fb_dev), get_fb_height(fb_dev), get_fb_bpp(fb_dev));
		}
	else{
		dbg_printf("Framebuffer init error on %s\r\n", dev_path);
		ret = -1;
		}

	return ret;
}

void fb_close(fb_dev_t *fb_dev)
{
	if(fb_dev == NULL){
		return;
		}
	
	close(fb_dev->fd);
	fb_dev->fd = -1;
}

void draw_point(fb_dev_t *fb_dev, point_t p1, unsigned int c)
{
	unsigned int w, h;
	unsigned int i, j, bpp;
	unsigned short * fb_mem = NULL;
	color_t color;
	set_color(&color, c);
	
	if(fb_dev == NULL){
		return;
		}
	
	w = get_fb_width(fb_dev);
	h = get_fb_height(fb_dev);
	bpp = get_fb_bpp(fb_dev);
	fb_mem = (unsigned short *)get_fb_mem(fb_dev);

	if(p1.x < 0){
		p1.x = 0;
		}
	if(p1.x >= w){
		p1.x = w - 1;
		}
	
	if(p1.y < 0){
		p1.y = 0;
		}
	if(p1.y >= h){
		p1.y = h - 1;
		}
	
	switch(bpp){
		case 16:
			fb_mem[p1.y*w+p1.x] = ((color.red>>3)<<11)|((color.green>>6)<<5)|(color.blue>>3);
			break;
		case 24:
		case 32:
			fb_mem[p1.y*w*2 + p1.x*2] = color.green<< 8| color.blue;
			fb_mem[p1.y*w*2 + p1.x*2+1] = color.red;
			break;
		default:
			fb_mem[p1.y*w*2 + p1.x*2] = color.green<< 8| color.blue;
			fb_mem[p1.y*w*2 + p1.x*2+1] = color.red;
			break;
		}
}

void draw_line(fb_dev_t *fb_dev, point_t p1, point_t p2, unsigned int c)
{
	int x, y;
	unsigned int w, h;
	unsigned int i, j, bpp;
	unsigned short * fb_mem = NULL;
	color_t color;
	set_color(&color, c);
	
	if(fb_dev == NULL){
		return;
		}
	
	w = get_fb_width(fb_dev);
	h = get_fb_height(fb_dev);
	bpp = get_fb_bpp(fb_dev);
	fb_mem = (unsigned short *)get_fb_mem(fb_dev);

	if(p1.x < 0){
		p1.x = 0;
		}
	if(p1.x >= w){
		p1.x = w - 1;
		}
	if(p2.x < 0){
		p2.x = 0;
		}
	if(p2.x >= w){
		p2.x = w - 1;
		}
	
	if(p1.y < 0){
		p1.y = 0;
		}
	if(p1.y >= h){
		p1.y = h - 1;
		}
	if(p2.y < 0){
		p2.y = 0;
		}
	if(p2.y >= h){
		p2.y = h - 1;
		}
	
	if((p2.x - p1.x) == 0){
		for(y=MIN(p1.y, p2.y);y<=MAX(p1.y, p2.y);y++){
			point_t tmp;
			tmp.x = p1.x;
			tmp.y = y;
	        draw_point(fb_dev, tmp, c);
			}
		}
    else if((p2.y - p1.y) == 0){
		for(x=MIN(p1.x, p2.x);x<=MAX(p1.x, p2.x);x++){
			point_t tmp;
			tmp.x = x;
			tmp.y = p1.y;
	        draw_point(fb_dev, tmp, c);
			}
		}
	else{
		for( x=MIN(p1.x,p2.x); x<=MAX(p1.x,p2.x); x++ ){
			int dy =((int)p2.y-(int)p1.y)*((int)x-(int)p1.x)/((int)p2.x-(int)p1.x);
//			dbg_printf("== dy=%d \r\n", dy);
			point_t tmp;
			tmp.x = x;
			tmp.y = p1.y + dy;
	        draw_point(fb_dev, tmp, c);
	    	}
		}
}

void draw_square(fb_dev_t *fb_dev, point_t p1, point_t p2, unsigned int c)
{
	if(fb_dev == NULL){
		return;
		}
	
	point_t lt;
	point_t lb;
	point_t rt;
	point_t rb;
	
	lt.x = p1.x;
	lt.y = p1.y;
	rt.x = p2.x;
	rt.y = p1.y;
	lb.x = p1.x;
	lb.y = p2.y;
	rb.x = p2.x;
	rb.y = p2.y;
	
	draw_line(fb_dev, lt, rt, c);
	draw_line(fb_dev, rt, rb, c);
	draw_line(fb_dev, rb, lb, c);
	draw_line(fb_dev, lb, lt, c);
}

void fill_square(fb_dev_t *fb_dev, point_t p1, point_t p2, unsigned int c)
{
	unsigned int x, y =0;
	point_t pointl, pointr;
	
	if(fb_dev == NULL){
		return;
		}

	for(y=MIN(p1.y,p2.y); y<=MAX(p1.y,p2.y);y++)
	{
		pointl.x = p1.x;
		pointl.y = y;
		pointr.x = p2.x;
		pointr.y = y;
		draw_line(fb_dev, pointl, pointr, c);
	}
}

void fill_background(fb_dev_t *fb_dev, unsigned int c)
{
	unsigned int w, h;
	unsigned int i, j, bpp;
	unsigned short * fb_mem = NULL;
	color_t color;
	set_color(&color, c);

	if(fb_dev == NULL){
		return;
		}

	w = get_fb_width(fb_dev);
	h = get_fb_height(fb_dev);
	bpp = get_fb_bpp(fb_dev);
	fb_mem = (unsigned short *)get_fb_mem(fb_dev);

	for(i = 0; i < h; i++){
		for(j = 0; j < w; j++){
			switch(bpp){
				case 16:
					fb_mem[i*w+j] = ((color.red>>3)<<11)|((color.green>>6)<<5)|(color.blue>>3);
					break;
				case 24:
				case 32:
					fb_mem[i*w*2 + j*2] = color.green<< 8| color.blue;
					fb_mem[i*w*2 + j*2+1] = color.red;
					break;
				default:
					fb_mem[i*w*2 + j*2] = color.green<< 8| color.blue;
					fb_mem[i*w*2 + j*2+1] = color.red;
					break;
				}
			}
		}
}
