/******************************************************************************
* Copyright (C), 2016-2017, Sunny.Guo
* FileName: common.h 
* Author: Sunny.Guo
* Version: 1.0
* Date: 2017年 01月 19日 星期四 10:12:50 CST
* Description: Common definition         
*
* History:        
*  <author>  	<time>   	<version >   	<desc>
*  Sunny.Guo   	28/02/2017      1.0     	create this moudle  
*                                                                          
* Licensed under GPLv2 or later, see file LICENSE in this source tree.
*******************************************************************************/

#ifndef __COMMON_H__
#define __COMMON_H__

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <pthread.h>

#define DEBUG 1

#ifdef DEBUG
#define dbg_printf(fmt, args...) printf(fmt, ##args)
#define dbg_perror(msg) (perror(msg))
#else
#define dbg_printf(fmt, args...)
#define dbg_perror(msg)
#endif

#define MIN(x,y)  (((x)<(y))?(x):(y))
#define MAX(x,y)  (((x)>(y))?(x):(y))

#define EXIT_SUCCESS	0
#define EXIT_FAILURE	1

#define MAX_INPUT 		255

#define RECORD_TIME		(10 * 1000000)
#define BITRATE			22000//24000//22000//12000
#define CHANNELS		1
#define	BIT_PER_SAMPLE	16

#define	BUFFER_TIME_US	200000
#define PERIOD_TIME_US	50000

typedef struct WAV_HEADER
{
	char rId[4];    				// "RIFF"
	int rLen;       				// File size - 8
	char wId[4];    				// "WAVE"
	char fId[4];    				// "fmt"
	int fLen;       				// Format block size = 16
	
	short wFormatTag;        		// Format tag = 0x0001(no compressed)
	short wChannels;         		// Channels
	int   nSamplesPersec ;   		// Sample rate
	int   nAvgBytesPerSample;		// = nSamplesPersec * wChannels * wBitsPerSample / 8
	short  wBlockAlign;				// = wChannels * wBitsPerSample / 8
	short wBitsPerSample;
	
	char dId[4];        			// "data"
	int wSampleLength;  			// PCM data length
} wav_header_t;

#endif
