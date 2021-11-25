/******************************************************************************
* Copyright (C), 2016-2017, Sunny.Guo
* FileName: main.c 
* Author: Sunny.Guo
* Version: 1.1
* Date: 2017年 01月 19日 星期四 10:12:50 CST
* Description: Main entry         
*
* History:        
*  <author>  	<time>   	<version >   	<desc>
*  Sunny.Guo   	19/01/2017      1.0     	create this moudle  
*  Sunny.Guo   	25/11/2021      1.1             add eeprom check and byte write options	
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

static const char short_options[] = "d:a:w:r:e:s:c:hqx";

static const struct option long_options[] = {
			{"device", 	required_argument, 	NULL, 'd'},
			{"address", required_argument, NULL, 'a'},
			{"start", required_argument, NULL, 's'},
			{"read",	required_argument,	NULL, 'r'},
			{"erase",	required_argument,	NULL, 'e'},
			{"write",	required_argument,	NULL, 'w'},
			{"quiet",	no_argument,		NULL, 'q'},
			{"hex",		no_argument,		NULL, 'x'},
			{"check",	required_argument,	NULL, 'c'},
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
			 "-r | --read  count    read byte count, count<64 \r\n"
			 "-e | --erase  count   erase byte count, fill with 0x00, count<64 \r\n"
			 "-w | --write frame 	write frame string or hex value(with -x). such as: 0123456789\r\n"
			 "-x | --hex 	        display with hex mode.\n"
			 "-c | --check len	write and check eeprom automatically with length \n"
			 "-q | --quiet 	        quiet mode.\n"
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

    // display mode 0: str   1: hex
    int hexmode = 0;

    // quiet mode 0: noquiet   1: quiet
    int quietmode = 0;

    //  WR check length
    int chkmode = 0;
    int chklen = 1024;

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

            		case 'e':
                		optWrite = 1;
                		count = atoi(optarg);
                		strWriteFrame = (char*)malloc(count);
                		if(strWriteFrame!=NULL){
                    			memset(strWriteFrame, 0, count);
                		}
                		break;

			case 'x':
				hexmode = 1;
				break;

			case 'c':
				chkmode = 1;
				chklen = atoi(optarg);
				if(chklen <= 0){
					fprintf(stderr, "\n wrong length to check!\n");
					exit(EXIT_FAILURE);
				}
				break;

			case 'q':
                		quietmode = 1;
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

	if(chkmode == 1)
	{
		char wb[2] = {0x55,0xAA};
		char rb[2] = {0};

		for(int i=0;i<chklen;i++){
			memset(rb,0,2);
		    for(int j=0;j<2;j++){
          		ret = eeprom_write(fd, eeprom_addr, i, &wb[j],1);
			if(ret <= 0){
				dbg_printf("Write to eeprom failed!\r\n");
				close(fd);
				exit(EXIT_FAILURE);
			}
				printf("w");
				fflush(stdout);
				usleep(20000);
			ret = eeprom_read(fd, eeprom_addr, i, rb+j, 1);
			if(ret < 0){
				dbg_printf("read from eeprom failed!\r\n");
				close(fd);
				exit(EXIT_FAILURE);
			}
				printf("\br");
				fflush(stdout);
				usleep(20000);
		       }

			if((wb[0] == rb[0])&&(wb[1] == rb[1])){
				printf("\b\bc");
				if(i%100==0){printf("\n");}
				fflush(stdout);
			}
			else
			{
				printf("\neeprom check error!\n");
			printf(" wb:%x %x",wb[0],wb[1]);
			printf(" rb:%x %x\n",rb[0],rb[1]);
				close(fd);
				exit(EXIT_FAILURE);

			}
		}
		printf("\neeprom check success!\n");
		close(fd);
		exit(EXIT_SUCCESS);
	}

	do{
		if(optWrite == 1){

		if(hexmode == 1){
			char *end;	
			int value = strtol(strWriteFrame, &end, 16);
			dbg_printf("WRITE:0x%02x\r\n", value);
			if (*end || value < 0 || value > 0xff) {
			                fprintf(stderr, "Error: Data address invalid!\n");
					                
			usage(stdout,argc,argv);
			exit(EXIT_FAILURE);
			}

            		ret = eeprom_write(fd, eeprom_addr, eeprom_offset, &value, 1);
			if(ret <= 0){
				dbg_printf("Write to eeprom failed!\r\n");
				close(fd);
				exit(EXIT_FAILURE);
				}
		}
		else
		{
			dbg_printf("WRITE:%s\r\n", strWriteFrame);
            	ret = eeprom_write(fd, eeprom_addr, eeprom_offset, strWriteFrame, count>0?count:strlen(strWriteFrame));
			if(ret <= 0){
				dbg_printf("Write to eeprom failed!\r\n");
				close(fd);
				exit(EXIT_FAILURE);
				}
            		if(count>0){
                		free(strWriteFrame);
                		strWriteFrame=NULL;
            		}
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
            if(quietmode == 1){
			    dbg_printf("%s", buffer);
                if(hexmode==1){
                    int l = count>0?count:strlen(buffer);
                    int index = 0;
			        dbg_printf("\r\n");
                    for(index=0;index<l;index++){
                        if( (index % 16) == 0  ) 
                                dbg_printf("\r\n %.4x|  ",eeprom_offset+index);
                        else if( (index % 8) == 0  ) 
                                        printf("  ");
                        dbg_printf("%.2x ", buffer[index]);
                    }
			        dbg_printf("\r\n");
                }
            }
            else
            {
			dbg_printf("\nREAD:%s\r\n", buffer);
                dbg_printf("TOTAL %d BYTES.\r\n", count>0?count:strlen(buffer));
                if(hexmode==1){
                    int l = count>0?count:strlen(buffer);
                    int index = 0;
			        dbg_printf("\r\n");
                    for(index=0;index<l;index++){
                        if( (index % 16) == 0  ) 
                                dbg_printf("\r\n %.4x|  ",eeprom_offset+index);
                        else if( (index % 8) == 0  ) 
                                        printf("  ");
                        dbg_printf("%.2x ", buffer[index]);
                    }
			        dbg_printf("\r\n");
                }
            }
			close(fd);
			}
		}while(0);
	}
