CC = $(CROSS_COMPILE)gcc
TARGET = $(notdir $(CURDIR))_test
SRC =  $(shell ls *.c)
OBJS = $(patsubst %.c ,%.o ,$(SRC))
.PHONY: all
all: $(TARGET)
$(TARGET) : $(OBJS)
	$(CC) -o $@ $^
%.o : %.c
	$(CC) -c $< -o $@
clean:
	$(RM) *.o $(TARGET)
