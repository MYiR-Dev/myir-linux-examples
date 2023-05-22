CC = $(CROSS_COMPILE)gcc
CFLAGS ?=  -I /home/sunny/sources/myir-buildroot/output/host/usr/arm-myir-linux-gnueabihf/sysroot/usr/include
LDFLAGS ?=  -lpthread -lasound -L /home/sunny/sources/myir-buildroot/output/host/usr/arm-myir-linux-gnueabihf/sysroot/usr/lib
TARGET = $(notdir $(CURDIR))_test
SRC =  $(shell ls *.c)
OBJS = $(patsubst %.c ,%.o ,$(SRC))
.PHONY: all
all: $(TARGET)
$(TARGET) : $(OBJS)
	$(CC) $(LDFLAGS) -o $@ $^
%.o : %.c
	$(CC) $(CFLAGS) -c $< -o $@ 
clean:
	$(RM) *.o $(TARGET)
