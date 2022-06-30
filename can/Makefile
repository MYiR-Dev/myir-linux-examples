CC 		= ${CROSS_COMPILE}gcc
TARGET	= can-test
SRC		= can-test.c
CFLAGS	= -Wall
LDFLAGS	= 

all: $(TARGET)

$(TARGET): $(SRC)
	$(CC) -o $(TARGET) $(CFLAGS) $(LDFLAGS) $<

.PHONY: clean
clean:
	@rm -rf $(TARGET) *.o
