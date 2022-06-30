/*****************************************************************************
 * Copyright (c) 2014-2017 MYIR Tech Ltd.
 *        File: led-test.c
 *        Date: 2014/11/3
 *      Author: Kevin Su
 * Description: A demo program to show how to control leds from user-space.
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <signal.h>
#include <linux/input.h>

#define DEBUG	1

#define ERR_MSG(fmt, args...)	fprintf(stderr, fmt, ##args)
#ifdef DEBUG
	#define DBG_MSG(fmt, args...)	fprintf(stdout, fmt, ##args)
#else
	#define DBG_MSG(fmt, args...)
#endif

#define LED1_DIR	"/sys/class/leds/sys_led/"
#define NAME_MAX_LENGTH		64
#define LED_DELAY_US	(500*1000)

#define ARRAY_SIZE(x)	(sizeof(x)/sizeof(x[0]))

#define BITS_MASK(num)	((1<<num) - 1)

typedef struct led_ctrl_s {
	char name[NAME_MAX_LENGTH];
	char brightness[NAME_MAX_LENGTH];
	char trigger[NAME_MAX_LENGTH];
	char trigger_backup[NAME_MAX_LENGTH];
	int state;
	int initialized;
} led_ctrl_t;

static led_ctrl_t leds[] = {
	/* name, brightness, trigger, trigger_str,  state, initialized */
	{"sys_led", LED1_DIR "brightness", LED1_DIR "trigger", "", 0, 0},
};

static int led_set_trigger(led_ctrl_t *led, const char *trigger)
{
	int ret;
	int fd = open(led->trigger, O_WRONLY);
	
	if (fd < 0) {
		ERR_MSG("Open %s failed!\n", led->trigger);
		return -1;
	}
	
	ret = write(fd, trigger, strlen(trigger));
	if (ret != strlen(trigger)) {
		ERR_MSG("Write %s failed!\n", led->trigger);
		close(fd);
		return -1;
	}
	close(fd);
	DBG_MSG("[%8s] Set trigger to '%s'\n",
		led->name,
		trigger);
		
	return 0;
}

static int led_get_trigger(led_ctrl_t *led, char *trigger)
{
	int ret;
	char tmp[128] = {0};
	char *ptr[2] = {NULL};
	int fd = open(led->trigger, O_RDONLY);
	
	if (fd < 0) {
		ERR_MSG("Open %s failed!\n", led->trigger);
		return -1;
	}
	
	/* read back string with format like: "none cpu1 cpu2 [heartbeat] nand" */
	ret = read(fd, tmp, sizeof(tmp));
	if (ret <= 0) {
		ERR_MSG("Read %s failed!\n", led->trigger);
		close(fd);
		return -1;
	}
	close(fd);
	
	/* find trigger from read back string, which is inside "[]" */
	ptr[0] = strchr(tmp, '[');
	if (ptr[0]) {
		ptr[0] += 1;
		ptr[1] = strchr(ptr[0], ']');
		if (ptr[1]) {
			*ptr[1] = '\0';
		} else {
			ERR_MSG("[%s] Can not find trigger in %s!\n", led->name, tmp);
			return -1;
		}
		strcpy(trigger, ptr[0]);
	} else {
		ERR_MSG("[%s] Can not find trigger in %s!\n", led->name, tmp);
		return -1;
	}
	
	DBG_MSG("[%8s] Get trigger: '%s'\n",
		led->name,
		trigger);
		
	return 0;
}

static int led_set_brightness(led_ctrl_t * led, int brightness)
{
	int ret;
	int fd = open(led->brightness, O_WRONLY);
	char br_str[2] = {0};

	br_str[0] = '0' + brightness;
	
	if (fd < 0) {
		ERR_MSG("Open %s failed!\n", led->brightness);
		return -1;
	}
	
	ret = write(fd, br_str, sizeof(br_str));
	if (ret != sizeof(br_str)) {
		ERR_MSG("Write %s failed!\n", led->brightness);
		close(fd);
		return -1;
	}
	close(fd);
	// DBG_MSG("[%s] Set brightness to %s successfully!\n",
		// led->name,
		// br_str);
		
	return 0;
}

static int led_init(void)
{
	int i;
	char tmp[NAME_MAX_LENGTH];
	
	
	for (i=0; i<ARRAY_SIZE(leds); i++) {
		memset(tmp, 0, sizeof(tmp));
		
		/* Backup all led triggers */
		if (led_get_trigger(&leds[i], tmp)) {
			return -1;
		}
		strcpy(leds[i].trigger_backup, tmp);
		
		/* Set all led triggers to 'none' */
		if (led_set_trigger(&leds[i], "none")) {
			return -1;
		}
		
		/* Set all brightness to 0 */
		if (led_set_brightness(&leds[i], 0)) {
			return -1;
		}
		leds[i].state = 0;
		leds[i].initialized = 1;
	}
	return 0;
}

static void led_restore(void)
{
	int i;
	
	/* set all brightness to '0', and restore triggers */
	for (i=0; i<ARRAY_SIZE(leds); i++) {
		if (leds[i].initialized) {
			led_set_brightness(&leds[i], 0);
			led_set_trigger(&leds[i], leds[i].trigger_backup);
		}
	}
}

/* Will be called if SIGINT(Ctrl+C) and SIGTERM(simple kill) signal is received */
static void signal_callback(int num)
{
	led_restore();
	exit(num);
}

int main(int argc, const char *argv[])
{
	unsigned int led_bits = 0x1;
	const int led_num = ARRAY_SIZE(leds);
	int i;
	
	/* Open button device */
	if(led_init()) {
		led_restore();
		return -1;
	}

	/* Register SIGINT(Ctrl+C) and SIGTERM(simple kill) signal and signal handler */
	signal(SIGINT, signal_callback);
	signal(SIGTERM, signal_callback);
	
	for (;;) {
		for (i=0; i<led_num; i++) {
				if (leds[i].state == 0) { /* if already ON, do nothing */
					leds[i].state = 1;
					led_set_brightness(&leds[i], leds[i].state);
				}else if(leds[i].state == 1) { /* if already OFF, do nothing */
					leds[i].state = 0;
					led_set_brightness(&leds[i], leds[i].state);
				}
		}
		usleep(LED_DELAY_US);
		/* do the rotate left */	
		led_bits = ((led_bits << 1) & BITS_MASK(led_num)) 
					| ((led_bits>>(led_num-1)) & 0x1);
	}
	
	led_restore();
	
	return 0;
}
