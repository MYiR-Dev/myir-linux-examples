include ../env.mk
TARGET = $(notdir $(CURDIR))

objs := $(patsubst %c, %o, $(shell ls *.c))

all: rs485_read rs485_write rs485_recv_send

rs485_read:
	$(CC) rs485_read.c -o rs485_read
arm_server:
	$(CC) rs485_write.c -o rs485_write

rs485_recv_send:
	$(CC) rs485_recv_send.c -o rs485_recv_send 
clean:
	rm -f rs485_recv_send rs485_read rs485_write *.o *~
