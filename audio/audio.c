/******************************************************************************
* Copyright (C), 2016-2017, Sunny.Guo
* FileName: audio.c 
* Author: Sunny.Guo
* Version: 1.0
* Date: 2017年 01月 19日 星期四 10:12:50 CST
* Description: audio loopback test. capture audio from line in and play 
*
* History:        
*  <author>  	<time>   	<version >   	<desc>
*  Sunny.Guo   	28/02/2017      1.0     	create this moudle  
*                                                                          
* Licensed under GPLv2 or later, see file LICENSE in this source tree.
*******************************************************************************/
#include "common.h"
#include <alsa/asoundlib.h>

#define ALSA_PCM_NEW_HW_PARAMS_API

static int xrun_recovery(snd_pcm_t *handle, int err)
{
        if (err == -EPIPE) {    					// under-run 
                err = snd_pcm_prepare(handle);
                if (err < 0){
                        dbg_printf("Can't recovery from underrun, prepare failed: %s\n", snd_strerror(err));
                	}
                return 0;
        } else if (err == -ESTRPIPE) {
                while ((err = snd_pcm_resume(handle)) == -EAGAIN)
                        sleep(1);       			// wait until the suspend flag is released  
                if (err < 0) {
                        err = snd_pcm_prepare(handle);
                        if (err < 0)
                                dbg_printf("Can't recovery from suspend, prepare failed: %s\n", snd_strerror(err));
                }
                return 0;
        }
        return err;
}

static snd_pcm_t * init_pcm(int stream_type, int channel, int sample_width, 
							int *sample_rate, snd_pcm_uframes_t * period_size)
{
	snd_pcm_hw_params_t *params;
	snd_pcm_t *handle = NULL;
	unsigned int val;
	int dir, rc;
	snd_pcm_uframes_t frames;

	/* Open PCM device for recording (capture). */
	rc = snd_pcm_open(&handle, "default", stream_type, 0);
	if (rc < 0) {
		fprintf(stderr,"unable to open pcm device: %s/n", snd_strerror(rc));
		goto err_exit;
	}
	
	/* Allocate a hardware parameters object. */
	snd_pcm_hw_params_alloca(&params);
	
	/* Fill it in with default values. */
	snd_pcm_hw_params_any(handle, params);
	
	/* Set the desired hardware parameters. */
	/* Interleaved mode */
	snd_pcm_hw_params_set_access(handle, params, SND_PCM_ACCESS_RW_INTERLEAVED);
	
	/* Set sample format */
	switch(sample_width)
	{
		case 8:
			snd_pcm_hw_params_set_format(handle, params, SND_PCM_FORMAT_U8);
			break ;
		case 16:
			snd_pcm_hw_params_set_format(handle, params, SND_PCM_FORMAT_S16_LE);
			break ;
		case 24:
			snd_pcm_hw_params_set_format(handle, params, SND_PCM_FORMAT_S24_LE);
			break ;
		default:
			dbg_perror("\nUnsupported audio format:");
			goto err_exit;
	}
	
	/* Set channels */
	snd_pcm_hw_params_set_channels(handle, params, channel);
	
	/* sampling rate settings */
	val = *sample_rate;
	snd_pcm_hw_params_set_rate_near(handle, params, sample_rate, &dir);
	printf("rate set to %d, expected %d\n", *sample_rate, val);
	
	/* Set period time */
	frames = *period_size;
	rc = snd_pcm_hw_params_set_period_size_near(handle, params, period_size, &dir);
	if (rc < 0) {
		fprintf(stderr, "Unable to set period size %i : %s\n", frames, snd_strerror(rc));
		goto err_exit;
	}
	if(frames != *period_size)
	{
		dbg_printf("Set period size to %d frames fail, use %d us instead!\r\n", frames, period_size);
	}
	
	/* Write the parameters to the driver */
	rc = snd_pcm_hw_params(handle, params);
	if (rc < 0) {
		fprintf(stderr,"unable to set hw parameters: %s\n",
		snd_strerror(rc));
		goto err_exit;
	}
	
	return handle;
	
err_exit:
	if (handle) {
		snd_pcm_close(handle); 
	}
	return NULL;
}

int main(const int argc, const char *argv[])
{
	int size;
	snd_pcm_t *handle_c = NULL, *handle_p = NULL;
	snd_pcm_uframes_t period_c, period_p;
	int rate_p, rate_c, rc, i = 0;
	char *buffer = NULL;
	
	rate_c = BITRATE;
	
	//set period_size to 128 frames
	period_c = 128;
	
	handle_c = init_pcm(SND_PCM_STREAM_CAPTURE, CHANNELS, BIT_PER_SAMPLE,
						&rate_c, &period_c);
	if (handle_c == NULL) {
		fprintf(stderr, "Init capture failed!\n");
		goto out;
	} else {
		dbg_printf("Init capture successfully, rate: %d, period_size: %d\n",
				rate_c, period_c);
	}
	
	rate_p = rate_c;
	period_p = period_c;
	handle_p = init_pcm(SND_PCM_STREAM_PLAYBACK, CHANNELS, BIT_PER_SAMPLE,
						&rate_p, &period_p);
	if (handle_p == NULL) {
		fprintf(stderr, "Init capture failed!\n");
		goto out;
	}
	
	// if (rate_c != rate_p) {
		// fprintf(stderr, "Capture rate and Playback rate mismatch!! capture:%d, playback:%d\n", 
			// rate_c, rate_p);
		// goto out;
	// }
	
	if (period_c != period_p) {
		fprintf(stderr, "Capture period size and Playback period size mismatch!! capture:%d, playback:%d\n", 
			period_c, period_p);
		goto out;
	}
	
	size = period_c * CHANNELS * BIT_PER_SAMPLE / 8;
	buffer = (char *)malloc(size);
	if (buffer == NULL) {
		fprintf(stderr, "Allocate memory failed!\n");
		goto out;
	}
	memset(buffer, 0, sizeof(buffer));
	dbg_printf("Period size: %d frames, buffer size: %d bytes\n", (int)period_c, size);
	
	for(;;) {
		/* 1. capture a period of data first */
		rc = snd_pcm_readi(handle_c, buffer, period_c); 
		dbg_printf("%4d frames\r",i++); 
		fflush(stdout);
		if (rc == -EPIPE) {
			/* EPIPE means overrun */
			fprintf(stderr, "overrun occurred/n");
			snd_pcm_prepare(handle_c);
		} else if (rc < 0) {
			fprintf(stderr,
			"error from read: %s/n",
			snd_strerror(rc));
		} else if (rc != (int)period_c) {
			fprintf(stderr, "short read, read %d frames/n", rc);
		}
		
		/* 2. write playback stream */
		while((rc = snd_pcm_writei(handle_p, buffer, period_p)) < 0)
		{
			if(rc == -EAGAIN)
			{
				usleep(2000);
				continue;
			}
			else if(xrun_recovery(handle_p, rc) < 0)
			{
				dbg_printf("Recover failed: %s\r\n", snd_strerror(rc));
				goto out;
			}
		}
		if(rc != (int)period_p)
		{
			dbg_printf("short write!! rc:%d exp:%d\r\n", rc, (int)period_p);
		}
	}
	
out:
	if (handle_c) {
		snd_pcm_close(handle_c); 
	}
	if (handle_p) {
		snd_pcm_close(handle_p); 
	}
	if (buffer) {
		free(buffer);
	}
}
