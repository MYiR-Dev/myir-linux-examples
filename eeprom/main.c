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
#include "eeprom.h"

static const char *		version 		= "1.0";
static char *			dev_name 		= "/dev/i2c-0";

static const char short_options[] = "d:a:w:r:s:h";

static const struct option long_options[] = {
			{"device", 	required_argument, 	NULL, 'd'},
			{"address", required_argument, NULL, 'a'},
			{"start", required_argument, NULL, 's'},
			{"read",	required_argument,	NULL, 'r'},
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
			 "-d | --device name   i2c device name: %s\n"
			 "-a | --address addr 	eeprom i2c address, default 0x51\n"
			 "-s | --start addr 	start offset to read/write \n"
			 "-r | --read  count    read byte count \r\n"
			 "-w | --write frame 	write frame string. such as: 0123456789\r\n"
			 "-h | --help		   Print this message\n"
			 "",
			 argv[0], version, dev_name);
}

int main(int argc, char **argv)
{
	int ret = 0;
	char *strWriteFrame = NULL;
	
	// write mode  0: read ; 1: write
	int optWrite = 0; 

	// eeprom i2c addr
	int eeprom_addr = EEPROM_I2C_ADDR;

	// eeprom offset to read/write
	int eeprom_offset = 0;

	// count of bytes to read
	int count = 0;

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
				
			case 'd':
				dev_name = optarg;
				break;

			case 'a':
				eeprom_addr = parse_address(optarg);
				if(eeprom_addr == -1){
					exit(EXIT_FAILURE);
					}
				else if (eeprom_addr < 0x03 || eeprom_addr > 0x77) {
					fprintf(stderr, "Error: Chip address out of range "
					"(0x03-0x77)!\n");
					exit(EXIT_FAILURE);
					}
				break;

			case 's':
				eeprom_offset = parse_address(optarg);
				if(eeprom_offset < 0){
					exit(EXIT_FAILURE);
					}
				break;
				
			case 'w':
				strWriteFrame = optarg;
				if((strWriteFrame == NULL)|| (strlen(strWriteFrame)<1)){
					fprintf(stderr, "\n No data to write!\n");
					exit(EXIT_FAILURE);
					}
				optWrite = 1;
				break;
				
			case 'r':
				optWrite = 0;
				count = atoi(optarg);
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
//	dbg_printf(" dev_name = %s \r\n", dev_name);
//	dbg_printf(" dev_addr = 0x%x\r\n", eeprom_addr);
//	dbg_printf(" dev_offset = 0x%x\r\n", eeprom_offset);
//	dbg_printf(" write = %s\r\n", strWriteFrame);
//	dbg_printf(" read count = %d\r\n", count);
//	dbg_printf(" mode  = 0x%x\r\n", optWrite);

	eeprom_wp(EEPROM_WP_DISABLE);
	
	fd = eeprom_init(dev_name);
	if(fd < 0){
		dbg_perror("Open device");
		dbg_printf("Open:%s failed!\r\n", dev_name);
		exit(EXIT_FAILURE);
		}

	do{
		if(optWrite == 1){
			dbg_printf("WRITE:%s\r\n", strWriteFrame);
			
			ret = eeprom_write(fd, eeprom_addr, eeprom_offset, strWriteFrame, strlen(strWriteFrame));
			if(ret <= 0){
				dbg_printf("Write to eeprom failed!\r\n");
				close(fd);
				exit(EXIT_FAILURE);
				}
			dbg_printf("WRITE SUCCESS!\r\n");
			close(fd);
			}
		else if(optWrite == 0){
//			dbg_printf("eeprom_read\n");
			char buffer[EEPROM_BUFFER_SIZE] = {0};
			
			ret = eeprom_read(fd, eeprom_addr, eeprom_offset, buffer, count);
			if(ret < 0){
				dbg_printf("read from eeprom failed!\r\n");
				close(fd);
				exit(EXIT_FAILURE);
				}
			
			dbg_printf("READ:%s\r\n", buffer);
			dbg_printf("TOTAL %d BYTES.\r\n", strlen(buffer));
			close(fd);
			}
		}while(0);
	}
